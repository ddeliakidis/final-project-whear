#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "config.h"

/* ── Pin definitions (Adafruit Feather HUZZAH32 V2) ──────────────── */
#define PIN_UART_RX  14    /* STM32 PC6 → ESP32 GPIO14 */
#define PIN_UART_TX  32    /* ESP32 GPIO32 → STM32 PC7 */
#define PIN_READY    27    /* ESP32 GPIO27 → STM32 PA8 */

#define UART_BAUD       115200
#define START_MARKER    0xAA
#define END_MARKER      0x55
#define FRAME_TIMEOUT_MS 500

/* ── Tag limits ──────────────────────────────────────────────────── */
#define MAX_TAGS      20
#define MAX_EPC_LEN   12
#define MAX_HEX_LEN   (MAX_EPC_LEN * 2 + 1)
#define MAX_DOC_IDS   (MAX_TAGS * 2)

/* ── Firestore URL ───────────────────────────────────────────────── */
#define FIRESTORE_COLLECTION_URL \
    "https://firestore.googleapis.com/v1/projects/" FIRESTORE_PROJECT \
    "/databases/(default)/documents/" FIRESTORE_COLLECTION

/* UART to STM32 — use UART2 peripheral with custom pins */
HardwareSerial StmSerial(2);

/* ── Tag data ────────────────────────────────────────────────────── */
typedef struct {
    uint8_t epc[MAX_EPC_LEN];
    uint8_t epc_len;
    char    hex_id[MAX_HEX_LEN];
} tag_t;

/* ── Helpers ─────────────────────────────────────────────────────── */

static void bytes_to_hex(const uint8_t *bytes, uint8_t len, char *out) {
    static const char hx[] = "0123456789ABCDEF";
    for (uint8_t i = 0; i < len; i++) {
        out[i * 2]     = hx[bytes[i] >> 4];
        out[i * 2 + 1] = hx[bytes[i] & 0x0F];
    }
    out[len * 2] = '\0';
}

/* Read one byte from STM32 UART with timeout. Returns -1 on timeout. */
static int read_byte_timeout(uint32_t timeout_ms) {
    uint32_t start = millis();
    while ((millis() - start) < timeout_ms) {
        if (StmSerial.available()) return StmSerial.read();
    }
    return -1;
}

/* Read one full tag frame. Blocks until start marker arrives, then
   reads to end marker (or times out). Returns tag count, -1 on error. */
static int read_frame(tag_t *tags) {
    int b;

    /* Scan for start marker (no overall timeout — this is our "idle" state) */
    while (true) {
        if (StmSerial.available()) {
            b = StmSerial.read();
            if (b == START_MARKER) break;
        } else {
            return -1;   /* no data available right now, let caller loop */
        }
    }

    /* Tag count */
    b = read_byte_timeout(FRAME_TIMEOUT_MS);
    if (b < 0 || b > MAX_TAGS) return -1;
    uint8_t count = (uint8_t)b;

    /* Per-tag: length + EPC bytes */
    for (uint8_t i = 0; i < count; i++) {
        b = read_byte_timeout(FRAME_TIMEOUT_MS);
        if (b < 0 || b > MAX_EPC_LEN) return -1;
        uint8_t epc_len = (uint8_t)b;

        tags[i].epc_len = epc_len;
        for (uint8_t j = 0; j < epc_len; j++) {
            b = read_byte_timeout(FRAME_TIMEOUT_MS);
            if (b < 0) return -1;
            tags[i].epc[j] = (uint8_t)b;
        }
        bytes_to_hex(tags[i].epc, epc_len, tags[i].hex_id);
    }

    /* End marker */
    b = read_byte_timeout(FRAME_TIMEOUT_MS);
    if (b != END_MARKER) return -1;

    return count;
}

/* ── Firestore: list existing document IDs ───────────────────────── */

static int list_existing_docs(String *ids, int max_ids) {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.begin(client, FIRESTORE_COLLECTION_URL);
    int code = http.GET();

    if (code != 200) {
        Serial.printf("[firestore] list GET %d\n", code);
        http.end();
        return 0;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;
    if (deserializeJson(doc, payload)) return 0;

    JsonArray documents = doc["documents"];
    int count = 0;
    for (JsonObject d : documents) {
        if (count >= max_ids) break;
        String name = d["name"].as<String>();
        int slash = name.lastIndexOf('/');
        if (slash >= 0) ids[count++] = name.substring(slash + 1);
    }
    return count;
}

/* ── Firestore: PATCH a single document (upsert, no auth needed) ─── */

static bool firestore_patch_doc(const char *doc_id) {
    String url = String(FIRESTORE_COLLECTION_URL) + "/" + doc_id;

    JsonDocument doc;
    doc["fields"]["id"]["stringValue"] = doc_id;

    String body;
    serializeJson(doc, body);

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    int code = http.PATCH(body);
    Serial.printf("[firestore] PATCH %s → %d\n", doc_id, code);
    http.end();

    return code == 200;
}

/* ── Firestore: DELETE a single document ─────────────────────────── */

static bool firestore_delete_doc(const String &doc_id) {
    String url = String(FIRESTORE_COLLECTION_URL) + "/" + doc_id;

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.begin(client, url);
    int code = http.sendRequest("DELETE");
    Serial.printf("[firestore] DELETE %s → %d\n", doc_id.c_str(), code);
    http.end();

    return code == 200;
}

/* ── Firestore: full replace (list → delete stale → patch current) ── */

static void firestore_full_replace(tag_t *tags, uint8_t tag_count) {
    String existing[MAX_DOC_IDS];
    int existing_count = list_existing_docs(existing, MAX_DOC_IDS);
    Serial.printf("[firestore] existing=%d  current=%d\n",
                  existing_count, tag_count);

    for (int i = 0; i < existing_count; i++) {
        bool found = false;
        for (uint8_t j = 0; j < tag_count; j++) {
            if (existing[i] == String(tags[j].hex_id)) {
                found = true;
                break;
            }
        }
        if (!found) firestore_delete_doc(existing[i]);
    }

    for (uint8_t i = 0; i < tag_count; i++)
        firestore_patch_doc(tags[i].hex_id);
}

/* ── Setup ───────────────────────────────────────────────────────── */

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Whear ESP32 WiFi Bridge ===");
    Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());

    /* READY pin LOW until WiFi connected */
    pinMode(PIN_READY, OUTPUT);
    digitalWrite(PIN_READY, LOW);

    /* UART to STM32 */
    StmSerial.begin(UART_BAUD, SERIAL_8N1, PIN_UART_RX, PIN_UART_TX);
    Serial.printf("UART2 ready on RX=%d TX=%d\n", PIN_UART_RX, PIN_UART_TX);

    /* Connect WiFi */
    Serial.printf("Connecting to %s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected  IP: %s\n", WiFi.localIP().toString().c_str());

    digitalWrite(PIN_READY, HIGH);
    Serial.println("READY pin HIGH — waiting for STM32 frames");
}

/* ── Loop ────────────────────────────────────────────────────────── */

void loop() {
    /* Reconnect WiFi if needed */
    if (WiFi.status() != WL_CONNECTED) {
        digitalWrite(PIN_READY, LOW);
        Serial.println("WiFi lost, reconnecting...");
        WiFi.reconnect();
        while (WiFi.status() != WL_CONNECTED) delay(500);
        Serial.println("WiFi reconnected");
        digitalWrite(PIN_READY, HIGH);
    }

    /* Try to read a frame from STM32 */
    tag_t tags[MAX_TAGS];
    int count = read_frame(tags);
    if (count < 0) {
        delay(10);   /* no full frame yet, yield */
        return;
    }

    Serial.printf("[uart] received %d tags\n", count);
    for (int i = 0; i < count; i++)
        Serial.printf("  tag %d: %s\n", i, tags[i].hex_id);

    /* Upload to Firestore */
    digitalWrite(PIN_READY, LOW);
    firestore_full_replace(tags, (uint8_t)count);
    digitalWrite(PIN_READY, HIGH);
}

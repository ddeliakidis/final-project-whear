/*
 * Whear — nRF5340 application core (Zephyr).
 *
 * 1. Configures SPU to grant network core access to RFID UART pins.
 * 2. Releases network core from forced-off (it runs baremetal RFID).
 * 3. Connects to WiFi via nRF7002.
 * 4. Opens a WebSocket connection.
 * 5. Forwards RFID tags received via IPC to the server.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "ipc_app.h"
#include "ipc_shared.h"
#include "wifi_manager.h"
#include "ws_client.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* ── WebSocket server configuration (change to your server) ───────────── */

#define WS_SERVER_ADDR  "192.168.1.100"
#define WS_SERVER_PORT  8080
#define WS_URL_PATH     "/"

/* ── SPU: grant GPIO pins to network core ─────────────────────────────── */

#define NRF_SPU_BASE  0x50003000UL

/* SPU GPIOPORT[port].PERM[pin]
 *   Port 0: offset 0x4C0 + pin * 4
 *   Port 1: offset 0x540 + pin * 4
 *   Write 0 = non-secure (net core can access) */
#define SPU_GPIO_PERM(port, pin) \
    (*(volatile uint32_t *)(NRF_SPU_BASE + 0x4C0 + (port) * 0x80 + (pin) * 4))

static void configure_spu_for_net_core(void)
{
    /* Grant P1.04 (RFID UART TX) and P1.05 (RFID UART RX) */
    SPU_GPIO_PERM(1, 4) = 0;
    SPU_GPIO_PERM(1, 5) = 0;

    /* Grant P0.28 (LED1) for status indication */
    SPU_GPIO_PERM(0, 28) = 0;

    LOG_INF("SPU: granted P1.04, P1.05, P0.28 to network core");
}

/* ── Release network core from forced-off ─────────────────────────────── */

#define RESET_NETWORK_FORCEOFF \
    (*(volatile uint32_t *)(0x50005000UL + 0x614))

static void release_network_core(void)
{
    RESET_NETWORK_FORCEOFF = 0;
    k_msleep(100);
    LOG_INF("Network core released");
}

/* ── Main ─────────────────────────────────────────────────────────────── */

int main(void)
{
    LOG_INF("=== Whear App Core (Zephyr) ===");

    /* Step 1: Grant network core access to GPIO pins */
    configure_spu_for_net_core();

    /* Step 2: Initialize IPC consumer */
    ipc_app_init();

    /* Step 3: Release network core (starts baremetal RFID scanning) */
    release_network_core();

    /* Wait for net core to initialize */
    LOG_INF("Waiting for network core mailbox...");
    while (IPC_MAILBOX->magic != IPC_MAILBOX_MAGIC) {
        k_msleep(10);
    }
    LOG_INF("Network core mailbox ready");

    /* Step 4: Connect to WiFi */
    int ret = wifi_connect();
    if (ret != 0) {
        LOG_ERR("WiFi setup failed — running without network");
        /* Continue anyway to show IPC is working */
    }

    /* Step 5: Connect WebSocket (only if WiFi is up) */
    if (wifi_is_connected()) {
        ret = ws_connect(WS_SERVER_ADDR, WS_SERVER_PORT, WS_URL_PATH);
        if (ret != 0) {
            LOG_ERR("WebSocket connection failed");
        }
    }

    /* Step 6: Main loop — forward tags from IPC to WebSocket */
    LOG_INF("Entering main loop — waiting for RFID tags...");

    while (1) {
        ipc_tag_entry_t tag;

        if (ipc_app_wait_tag(&tag, K_SECONDS(5)) == 0) {
            /* Log the tag */
            LOG_INF("Tag EPC (%u bytes): RSSI=%d dBm",
                    tag.epc_len, (int)tag.rssi);

            /* Send over WebSocket if connected */
            if (ws_is_connected()) {
                ret = ws_send_tag(&tag);
                if (ret < 0) {
                    LOG_WRN("Send failed, reconnecting...");
                    ws_disconnect();
                    k_msleep(1000);
                    ws_connect(WS_SERVER_ADDR, WS_SERVER_PORT, WS_URL_PATH);
                }
            }
        }

        /* Check WiFi health and reconnect if needed */
        if (!wifi_is_connected()) {
            LOG_WRN("WiFi lost, attempting reconnect...");
            wifi_connect();
            if (wifi_is_connected()) {
                ws_connect(WS_SERVER_ADDR, WS_SERVER_PORT, WS_URL_PATH);
            }
        }
    }

    return 0;
}

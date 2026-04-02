/*
 * WebSocket client for sending RFID tag data to a server.
 */

#include "ws_client.h"

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/websocket.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <string.h>

LOG_MODULE_REGISTER(ws_client, LOG_LEVEL_INF);

static int ws_sock = -1;
static int tcp_sock = -1;

static uint8_t temp_recv_buf[512];

/* ── Public API ───────────────────────────────────────────────────────── */

int ws_connect(const char *server_addr, uint16_t port, const char *url_path)
{
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
    };

    /* Resolve server address */
    if (zsock_inet_pton(AF_INET, server_addr, &addr.sin_addr) != 1) {
        LOG_ERR("Invalid server address: %s", server_addr);
        return -1;
    }

    /* Create TCP socket */
    tcp_sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_sock < 0) {
        LOG_ERR("Failed to create TCP socket: %d", errno);
        return -1;
    }

    /* Connect TCP */
    LOG_INF("Connecting to %s:%u ...", server_addr, port);
    int ret = zsock_connect(tcp_sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        LOG_ERR("TCP connect failed: %d", errno);
        zsock_close(tcp_sock);
        tcp_sock = -1;
        return -1;
    }

    /* Upgrade to WebSocket */
    struct websocket_request ws_req = {
        .host = server_addr,
        .url = url_path,
        .tmp_buf = temp_recv_buf,
        .tmp_buf_len = sizeof(temp_recv_buf),
    };

    ws_sock = websocket_connect(tcp_sock, &ws_req, 5000, NULL);
    if (ws_sock < 0) {
        LOG_ERR("WebSocket handshake failed: %d", ws_sock);
        zsock_close(tcp_sock);
        tcp_sock = -1;
        return -1;
    }

    LOG_INF("WebSocket connected");
    return 0;
}

int ws_send_tag(const ipc_tag_entry_t *tag)
{
    if (ws_sock < 0) {
        return -1;
    }

    /* Format tag as JSON */
    char json[256];
    int pos = 0;

    pos += snprintf(json + pos, sizeof(json) - pos, "{\"epc\":\"");
    for (uint8_t i = 0; i < tag->epc_len && i < IPC_MAX_EPC_LEN; i++) {
        pos += snprintf(json + pos, sizeof(json) - pos, "%02X", tag->epc[i]);
    }
    pos += snprintf(json + pos, sizeof(json) - pos,
                    "\",\"rssi\":%d}", (int)tag->rssi);

    /* Send as text frame (timeout in ms, SYS_FOREVER_MS = block forever) */
    int ret = websocket_send_msg(ws_sock,
                                 json, pos,
                                 WEBSOCKET_OPCODE_DATA_TEXT,
                                 true,   /* masked */
                                 true,   /* final */
                                 5000);  /* 5s timeout in ms */
    if (ret < 0) {
        LOG_ERR("WebSocket send failed: %d", ret);
        return ret;
    }

    LOG_DBG("Sent: %s", json);
    return 0;
}

void ws_disconnect(void)
{
    if (ws_sock >= 0) {
        websocket_disconnect(ws_sock);
        ws_sock = -1;
    }
    if (tcp_sock >= 0) {
        zsock_close(tcp_sock);
        tcp_sock = -1;
    }
    LOG_INF("WebSocket disconnected");
}

int ws_is_connected(void)
{
    return (ws_sock >= 0) ? 1 : 0;
}

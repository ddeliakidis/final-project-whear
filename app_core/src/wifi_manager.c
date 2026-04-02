/*
 * WiFi connection manager for nRF7002DK.
 *
 * Uses Zephyr net_mgmt API with static credentials from prj.conf.
 */

#include "wifi_manager.h"

#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(wifi_mgr, LOG_LEVEL_INF);

static K_SEM_DEFINE(wifi_connected_sem, 0, 1);
static K_SEM_DEFINE(ipv4_obtained_sem, 0, 1);

static struct net_mgmt_event_callback wifi_mgmt_cb;
static struct net_mgmt_event_callback ipv4_mgmt_cb;

static volatile int wifi_connected;

/* ── Event handlers ───────────────────────────────────────────────────── */

static void wifi_event_handler(struct net_mgmt_event_callback *cb,
                               uint32_t mgmt_event,
                               struct net_if *iface)
{
    ARG_UNUSED(iface);

    if (mgmt_event == NET_EVENT_WIFI_CONNECT_RESULT) {
        const struct wifi_status *status =
            (const struct wifi_status *)cb->info;

        if (status->status == 0) {
            LOG_INF("WiFi connected");
            wifi_connected = 1;
            k_sem_give(&wifi_connected_sem);
        } else {
            LOG_ERR("WiFi connection failed: %d", status->status);
        }
    } else if (mgmt_event == NET_EVENT_WIFI_DISCONNECT_RESULT) {
        LOG_WRN("WiFi disconnected");
        wifi_connected = 0;
    }
}

static void ipv4_event_handler(struct net_mgmt_event_callback *cb,
                                uint32_t mgmt_event,
                                struct net_if *iface)
{
    ARG_UNUSED(cb);
    ARG_UNUSED(iface);

    if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD) {
        LOG_INF("IPv4 address obtained");
        k_sem_give(&ipv4_obtained_sem);
    }
}

/* ── Public API ───────────────────────────────────────────────────────── */

int wifi_connect(void)
{
    struct net_if *iface = net_if_get_first_wifi();
    if (!iface) {
        LOG_ERR("No WiFi interface found");
        return -1;
    }

    /* Register event callbacks */
    net_mgmt_init_event_callback(&wifi_mgmt_cb, wifi_event_handler,
                                 NET_EVENT_WIFI_CONNECT_RESULT |
                                 NET_EVENT_WIFI_DISCONNECT_RESULT);
    net_mgmt_add_event_callback(&wifi_mgmt_cb);

    net_mgmt_init_event_callback(&ipv4_mgmt_cb, ipv4_event_handler,
                                 NET_EVENT_IPV4_ADDR_ADD);
    net_mgmt_add_event_callback(&ipv4_mgmt_cb);

    /* Connect using stored static credentials */
    LOG_INF("Connecting to WiFi...");
    int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT_STORED, iface, NULL, 0);
    if (ret) {
        LOG_ERR("WiFi connect request failed: %d", ret);
        return ret;
    }

    /* Wait for connection (30s timeout) */
    if (k_sem_take(&wifi_connected_sem, K_SECONDS(30)) != 0) {
        LOG_ERR("WiFi connection timed out");
        return -1;
    }

    /* Wait for DHCP IP (15s timeout) */
    LOG_INF("Waiting for DHCP...");
    if (k_sem_take(&ipv4_obtained_sem, K_SECONDS(15)) != 0) {
        LOG_ERR("DHCP timed out");
        return -1;
    }

    LOG_INF("WiFi ready");
    return 0;
}

int wifi_is_connected(void)
{
    return wifi_connected;
}

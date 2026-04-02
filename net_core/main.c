/*
 * nRF5340 network core — baremetal RFID scanner + IPC.
 *
 * Polls the YRM100 RFID module over UARTE0 (P1.04 TX, P1.05 RX)
 * and forwards detected tags to the application core via shared memory.
 *
 * Debug output: use Segger RTT (J-Link) since UARTE0 is occupied by RFID.
 */

#include "nrf5340_net.h"
#include "yrm100.h"
#include "yrm100_uart_nrf5340_net.h"
#include "ipc_net.h"
#include "ipc_shared.h"
#include <string.h>

/* ── LED (optional, if app core grants the pin via SPU) ──────────────── */

#define LED1_PIN   28      /* P0.28, active low on nRF7002DK */

/* ── RFID state ──────────────────────────────────────────────────────── */

#define MAX_TAGS  100

static yrm100_t      rfid;
static yrm100_tag_t  seen_tags[MAX_TAGS];
static uint8_t       num_unique = 0;

/* ── Tag tracking ────────────────────────────────────────────────────── */

static uint8_t find_or_add_tag(const yrm100_tag_t *tag)
{
    for (uint8_t i = 0; i < num_unique; i++) {
        if (seen_tags[i].epc_len == tag->epc_len &&
            memcmp(seen_tags[i].epc, tag->epc, tag->epc_len) == 0) {
            seen_tags[i].rssi = tag->rssi;
            return 0;   /* duplicate — updated RSSI */
        }
    }
    if (num_unique < MAX_TAGS) {
        memcpy(&seen_tags[num_unique], tag, sizeof(yrm100_tag_t));
        num_unique++;
    }
    return 1;   /* new tag */
}

/* ── Main ────────────────────────────────────────────────────────────── */

int main(void)
{
    /* Optional LED setup (P0.28) — works if app core grants P0.28 via SPU */
    NRF_P0->OUTSET = (1UL << LED1_PIN);
    NRF_P0->PIN_CNF[LED1_PIN] =
        GPIO_PIN_CNF_DIR_OUTPUT | GPIO_PIN_CNF_INPUT_DISCONNECT;

    /* Initialize IPC shared memory */
    ipc_net_init();

    /* Initialize RFID module */
    yrm100_init(&rfid, &yrm100_nrf5340_net_uart, 115200);
    delay_ms(500);

    yrm100_set_region(&rfid, YRM100_REGION_US);
    yrm100_set_tx_power(&rfid, YRM100_POWER_2600);

    /* Signal that we're scanning */
    IPC_MAILBOX->net_status = 1;

    yrm100_start_multi_inventory(&rfid, 0xFFFF);

    while (1) {
        yrm100_tag_t tag;
        yrm100_status_t status = yrm100_poll_inventory(&rfid, &tag);

        if (status == YRM100_OK) {
            NRF_P0->OUT ^= (1UL << LED1_PIN);   /* toggle LED */

            if (find_or_add_tag(&tag)) {
                /* New unique tag — send to app core via IPC */
                ipc_net_send_tag(&tag);
            }
        }
    }

    return 0;
}

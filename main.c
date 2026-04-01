/*
 * YRM100 RFID
 *
 * UARTE0  →  debug console (VCOM via on-board J-Link)
 *            TX P1.01,  RX P1.00
 * UARTE1  →  YRM100 RFID module
 *            TX P1.04,  RX P1.05  (change in yrm100_uart_nrf5340.c)
 * LED1    →  P0.28  (active low on nRF7002DK)
 */

#include "nrf5340.h"
#include "yrm100.h"
#include "yrm100_uart_nrf5340.h"
#include <string.h>


#define LED1_PIN   28      /* P0.28, active low */

#define DBG_TX_PSEL  UARTE_PSEL(1, 1)
#define DBG_RX_PSEL  UARTE_PSEL(1, 0)

/* ── Debug Serail Connection (UARTE0) ────────────────────────────────────────────── */

static uint8_t dbg_tx_buf[128];

static void debug_uart_init(void)
{
    NRF_P1->OUTSET = (1UL << 1);
    NRF_P1->PIN_CNF[1] = GPIO_PIN_CNF_DIR_OUTPUT | GPIO_PIN_CNF_INPUT_DISCONNECT;
    NRF_P1->PIN_CNF[0] = 0;

    NRF_UARTE0->PSEL_TXD  = DBG_TX_PSEL;
    NRF_UARTE0->PSEL_RXD  = DBG_RX_PSEL;
    NRF_UARTE0->PSEL_CTS  = UARTE_PSEL_DISCONNECT;
    NRF_UARTE0->PSEL_RTS  = UARTE_PSEL_DISCONNECT;
    NRF_UARTE0->BAUDRATE   = UARTE_BAUDRATE_115200;
    NRF_UARTE0->CONFIG     = 0;
    NRF_UARTE0->ENABLE     = UARTE_ENABLE_ENABLED;
}

static void debug_tx(const void *src, uint16_t len)
{
    if (len > sizeof(dbg_tx_buf))
        len = sizeof(dbg_tx_buf);
    memcpy(dbg_tx_buf, src, len);

    NRF_UARTE0->TXD_PTR    = (uint32_t)dbg_tx_buf;
    NRF_UARTE0->TXD_MAXCNT = len;
    NRF_UARTE0->EVENTS_ENDTX = 0;
    NRF_UARTE0->TASKS_STARTTX = 1;
    while (!NRF_UARTE0->EVENTS_ENDTX)
        ;
}

static void uart_putc(char c)
{
    debug_tx(&c, 1);
}

static void uart_puts(const char *s)
{
    debug_tx(s, (uint16_t)strlen(s));
}

static void uart_print_hex(uint8_t byte)
{
    const char hex[] = "0123456789ABCDEF";
    uart_putc(hex[byte >> 4]);
    uart_putc(hex[byte & 0x0F]);
}

static void uart_print_u8(uint8_t val)
{
    if (val >= 100) uart_putc('0' + val / 100);
    if (val >= 10)  uart_putc('0' + (val / 10) % 10);
    uart_putc('0' + val % 10);
}

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
            return 0;   /* duplicate */
        }
    }
    if (num_unique < MAX_TAGS) {
        memcpy(&seen_tags[num_unique], tag, sizeof(yrm100_tag_t));
        num_unique++;
    }
    return 1; 
}

static void print_tag(const yrm100_tag_t *tag)
{
    uart_puts("EPC: ");
    for (uint8_t i = 0; i < tag->epc_len; i++) {
        uart_print_hex(tag->epc[i]);
        if (i < tag->epc_len - 1)
            uart_putc(' ');
    }
    uart_puts("  RSSI: ");
    if (tag->rssi < 0) {
        uart_putc('-');
        uart_print_u8((uint8_t)(-(tag->rssi)));
    } else {
        uart_print_u8((uint8_t)tag->rssi);
    }
    uart_puts(" dBm\r\n");
}

/* ── Main ────────────────────────────────────────────────────────────── */

int main(void)
{
    NRF_P0->OUTSET = (1UL << LED1_PIN);
    NRF_P0->PIN_CNF[LED1_PIN] =
        GPIO_PIN_CNF_DIR_OUTPUT | GPIO_PIN_CNF_INPUT_DISCONNECT;

    debug_uart_init();

    yrm100_init(&rfid, &yrm100_nrf5340_uart, 115200);
    delay_ms(500);

    uart_puts("\r\n== YRM100 RFID Scanner (nRF5340) ==\r\n");

    yrm100_set_region(&rfid, YRM100_REGION_US);
    yrm100_set_tx_power(&rfid, YRM100_POWER_2600);

    uart_puts("Starting multi-inventory...\r\n\r\n");
    yrm100_start_multi_inventory(&rfid, 0xFFFF);

    while (1) {
        yrm100_tag_t tag;
        yrm100_status_t status = yrm100_poll_inventory(&rfid, &tag);

        if (status == YRM100_OK) {
            NRF_P0->OUT ^= (1UL << LED1_PIN);   /* toggle LED */

            if (find_or_add_tag(&tag)) {
                uart_puts("[NEW] ");
                print_tag(&tag);
                uart_puts("Total unique: ");
                uart_print_u8(num_unique);
                uart_puts("\r\n");
            }
        }
    }

    return 0;
}

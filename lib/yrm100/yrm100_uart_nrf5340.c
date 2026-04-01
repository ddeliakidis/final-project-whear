/*
 * nRF5340 UARTE1 backend for the YRM100 RFID library.
 *
 * RX: interrupt-driven 1-byte DMA with ENDRX_STARTRX short → ring buffer.
 * TX: blocking DMA (waits for ENDTX).
 *
 * NOTE: nRF EasyDMA can only access Data RAM — all TX buffers must be in
 * RAM (stack or .data/.bss).  The yrm100 library already does this.
 */

#include "yrm100_uart_nrf5340.h"
#include "nrf5340.h"
#include <string.h>

/* ── Pin configuration ───────────────────────────────────────────────── */
/* Change these to match your wiring.  Port 0 or 1, pin 0-31.           */

#ifndef YRM100_UART_PORT
#define YRM100_UART_PORT      1
#endif
#ifndef YRM100_UART_TX_PIN
#define YRM100_UART_TX_PIN    4   /* nRF TX → YRM100 RX */
#endif
#ifndef YRM100_UART_RX_PIN
#define YRM100_UART_RX_PIN    5   /* nRF RX ← YRM100 TX */
#endif

#define YRM100_GPIO  (YRM100_UART_PORT ? NRF_P1 : NRF_P0)
#define YRM100_PSEL_TX  UARTE_PSEL(YRM100_UART_PORT, YRM100_UART_TX_PIN)
#define YRM100_PSEL_RX  UARTE_PSEL(YRM100_UART_PORT, YRM100_UART_RX_PIN)

/* ── Receive ring buffer ─────────────────────────────────────────────── */

#define RX_RING_SIZE  256          /* must be power of 2 */

static volatile uint8_t  rx_ring[RX_RING_SIZE];
static volatile uint16_t rx_head;  /* written by ISR  */
static volatile uint16_t rx_tail;  /* read by app     */
static volatile uint8_t  rx_dma_buf;   /* 1-byte DMA target (must be in RAM) */

/* ── UARTE1 interrupt handler ────────────────────────────────────────── */

void UARTE1_Handler(void)
{
    if (NRF_UARTE1->EVENTS_ENDRX) {
        NRF_UARTE1->EVENTS_ENDRX = 0;
        uint16_t next = (rx_head + 1) & (RX_RING_SIZE - 1);
        if (next != rx_tail) {          /* drop byte on overflow */
            rx_ring[rx_head] = rx_dma_buf;
            rx_head = next;
        }
        /* ENDRX_STARTRX short restarts DMA automatically */
    }
}

/* ── UART callbacks ──────────────────────────────────────────────────── */

static uint32_t baud_to_reg(uint32_t baud)
{
    switch (baud) {
    case 9600:   return UARTE_BAUDRATE_9600;
    case 115200: return UARTE_BAUDRATE_115200;
    default:     return UARTE_BAUDRATE_115200;
    }
}

static void
yrm100_nrf_uart_init(uint32_t baud)
{
    /* GPIO: TX output high (UART idle), RX input */
    YRM100_GPIO->OUTSET = (1UL << YRM100_UART_TX_PIN);
    YRM100_GPIO->PIN_CNF[YRM100_UART_TX_PIN] =
        GPIO_PIN_CNF_DIR_OUTPUT | GPIO_PIN_CNF_INPUT_DISCONNECT;
    YRM100_GPIO->PIN_CNF[YRM100_UART_RX_PIN] = 0;  /* input, connected */

    /* UARTE1 peripheral */
    NRF_UARTE1->PSEL_TXD  = YRM100_PSEL_TX;
    NRF_UARTE1->PSEL_RXD  = YRM100_PSEL_RX;
    NRF_UARTE1->PSEL_CTS  = UARTE_PSEL_DISCONNECT;
    NRF_UARTE1->PSEL_RTS  = UARTE_PSEL_DISCONNECT;
    NRF_UARTE1->BAUDRATE   = baud_to_reg(baud);
    NRF_UARTE1->CONFIG     = 0;   /* 8N1, no hardware flow control */
    NRF_UARTE1->ENABLE     = UARTE_ENABLE_ENABLED;

    /* RX: 1-byte DMA with auto-restart */
    rx_head = 0;
    rx_tail = 0;
    NRF_UARTE1->RXD_PTR    = (uint32_t)&rx_dma_buf;
    NRF_UARTE1->RXD_MAXCNT = 1;
    NRF_UARTE1->SHORTS     = UARTE_SHORTS_ENDRX_STARTRX;
    NRF_UARTE1->EVENTS_ENDRX = 0;
    NRF_UARTE1->INTENSET   = UARTE_INT_ENDRX;
    NVIC_EnableIRQ(UARTE1_IRQn);

    NRF_UARTE1->TASKS_STARTRX = 1;
}

static void
yrm100_nrf_uart_send(const uint8_t *data, uint16_t len)
{
    /* data must already be in RAM (yrm100 lib uses stack buffers → OK) */
    NRF_UARTE1->TXD_PTR    = (uint32_t)data;
    NRF_UARTE1->TXD_MAXCNT = len;
    NRF_UARTE1->EVENTS_ENDTX = 0;
    NRF_UARTE1->TASKS_STARTTX = 1;
    while (!NRF_UARTE1->EVENTS_ENDTX)
        ;
}

static uint8_t
yrm100_nrf_uart_recv_byte(uint16_t timeout_ms)
{
    uint32_t start = millis();
    while (rx_head == rx_tail) {
        if ((millis() - start) >= timeout_ms)
            return 0;   /* timeout */
    }
    uint8_t b = rx_ring[rx_tail];
    rx_tail = (rx_tail + 1) & (RX_RING_SIZE - 1);
    return b;
}

static uint8_t
yrm100_nrf_uart_data_available(void)
{
    return (rx_head != rx_tail) ? 1 : 0;
}

/* ── Ready-made descriptor ───────────────────────────────────────────── */

const yrm100_uart_t yrm100_nrf5340_uart = {
    .uart_init           = yrm100_nrf_uart_init,
    .uart_send           = yrm100_nrf_uart_send,
    .uart_recv_byte      = yrm100_nrf_uart_recv_byte,
    .uart_data_available = yrm100_nrf_uart_data_available,
};

#ifndef NRF5340_H
#define NRF5340_H

/*
 * Minimal baremetal register definitions for the nRF5340 application core.
 * Secure-domain addresses (default after reset, no TF-M / SPM).
 */

#include <stdint.h>

#define SYSTEM_CLOCK_HZ  64000000UL

/* ── GPIO ────────────────────────────────────────────────────────────── */

typedef struct {
    volatile uint32_t RESERVED0;        /* +0x000 */
    volatile uint32_t OUT;              /* +0x004 */
    volatile uint32_t OUTSET;           /* +0x008 */
    volatile uint32_t OUTCLR;           /* +0x00C */
    volatile uint32_t IN;               /* +0x010 */
    volatile uint32_t DIR;              /* +0x014 */
    volatile uint32_t DIRSET;           /* +0x018 */
    volatile uint32_t DIRCLR;           /* +0x01C */
    volatile uint32_t LATCH;            /* +0x020 */
    volatile uint32_t DETECTMODE;       /* +0x024 */
    volatile uint32_t RESERVED1[118];   /* +0x028 .. +0x1FC */
    volatile uint32_t PIN_CNF[32];      /* +0x200 .. +0x27C */
} NRF_GPIO_Type;

#define NRF_P0  ((NRF_GPIO_Type *)0x50842500UL)
#define NRF_P1  ((NRF_GPIO_Type *)0x50842800UL)

/* PIN_CNF bit fields */
#define GPIO_PIN_CNF_DIR_OUTPUT         (1UL << 0)
#define GPIO_PIN_CNF_INPUT_DISCONNECT   (1UL << 1)
#define GPIO_PIN_CNF_PULL_PULLUP        (3UL << 2)

/* ── UARTE (UART with EasyDMA) ───────────────────────────────────────── */

typedef struct {
    volatile uint32_t TASKS_STARTRX;     /* +0x000 */
    volatile uint32_t TASKS_STOPRX;      /* +0x004 */
    volatile uint32_t TASKS_STARTTX;     /* +0x008 */
    volatile uint32_t TASKS_STOPTX;      /* +0x00C */
    volatile uint32_t RESERVED0[7];      /* +0x010 .. +0x028 */
    volatile uint32_t TASKS_FLUSHRX;     /* +0x02C */
    volatile uint32_t RESERVED1[52];     /* +0x030 .. +0x0FC */
    volatile uint32_t EVENTS_CTS;        /* +0x100 */
    volatile uint32_t EVENTS_NCTS;       /* +0x104 */
    volatile uint32_t EVENTS_RXDRDY;     /* +0x108 */
    volatile uint32_t RESERVED2;         /* +0x10C */
    volatile uint32_t EVENTS_ENDRX;      /* +0x110 */
    volatile uint32_t RESERVED3[2];      /* +0x114 .. +0x118 */
    volatile uint32_t EVENTS_TXDRDY;     /* +0x11C */
    volatile uint32_t EVENTS_ENDTX;      /* +0x120 */
    volatile uint32_t EVENTS_ERROR;      /* +0x124 */
    volatile uint32_t RESERVED4[7];      /* +0x128 .. +0x140 */
    volatile uint32_t EVENTS_RXTO;       /* +0x144 */
    volatile uint32_t RESERVED5;         /* +0x148 */
    volatile uint32_t EVENTS_RXSTARTED;  /* +0x14C */
    volatile uint32_t EVENTS_TXSTARTED;  /* +0x150 */
    volatile uint32_t RESERVED6;         /* +0x154 */
    volatile uint32_t EVENTS_TXSTOPPED;  /* +0x158 */
    volatile uint32_t RESERVED7[41];     /* +0x15C .. +0x1FC */
    volatile uint32_t SHORTS;            /* +0x200 */
    volatile uint32_t RESERVED8[63];     /* +0x204 .. +0x2FC */
    volatile uint32_t INTEN;             /* +0x300 */
    volatile uint32_t INTENSET;          /* +0x304 */
    volatile uint32_t INTENCLR;          /* +0x308 */
    volatile uint32_t RESERVED9[93];     /* +0x30C .. +0x47C */
    volatile uint32_t ERRORSRC;          /* +0x480 */
    volatile uint32_t RESERVED10[31];    /* +0x484 .. +0x4FC */
    volatile uint32_t ENABLE;            /* +0x500 */
    volatile uint32_t RESERVED11;        /* +0x504 */
    volatile uint32_t PSEL_RTS;          /* +0x508 */
    volatile uint32_t PSEL_TXD;          /* +0x50C */
    volatile uint32_t PSEL_CTS;          /* +0x510 */
    volatile uint32_t PSEL_RXD;          /* +0x514 */
    volatile uint32_t RESERVED12[3];     /* +0x518 .. +0x520 */
    volatile uint32_t BAUDRATE;          /* +0x524 */
    volatile uint32_t RESERVED13[3];     /* +0x528 .. +0x530 */
    volatile uint32_t RXD_PTR;           /* +0x534 */
    volatile uint32_t RXD_MAXCNT;        /* +0x538 */
    volatile uint32_t RXD_AMOUNT;        /* +0x53C */
    volatile uint32_t RESERVED14;        /* +0x540 */
    volatile uint32_t TXD_PTR;           /* +0x544 */
    volatile uint32_t TXD_MAXCNT;        /* +0x548 */
    volatile uint32_t TXD_AMOUNT;        /* +0x54C */
    volatile uint32_t RESERVED15[7];     /* +0x550 .. +0x568 */
    volatile uint32_t CONFIG;            /* +0x56C */
} NRF_UARTE_Type;

#define NRF_UARTE0  ((NRF_UARTE_Type *)0x50008000UL)
#define NRF_UARTE1  ((NRF_UARTE_Type *)0x50009000UL)

/* UARTE constants */
#define UARTE_ENABLE_ENABLED            8UL
#define UARTE_BAUDRATE_9600             0x00275000UL
#define UARTE_BAUDRATE_115200           0x01D7E000UL
#define UARTE_SHORTS_ENDRX_STARTRX     (1UL << 5)
#define UARTE_INT_ENDRX                (1UL << 4)
#define UARTE_PSEL_DISCONNECT          0xFFFFFFFFUL

/* Pin-select helper: encode port (0/1) and pin (0-31) for PSEL registers */
#define UARTE_PSEL(port, pin)  (((uint32_t)(port) << 5) | (pin))

/* ── CLOCK ───────────────────────────────────────────────────────────── */

#define NRF_CLOCK_BASE                  0x50005000UL
#define CLOCK_TASKS_HFCLKSTART          (*(volatile uint32_t *)(NRF_CLOCK_BASE + 0x000))
#define CLOCK_EVENTS_HFCLKSTARTED       (*(volatile uint32_t *)(NRF_CLOCK_BASE + 0x100))
#define CLOCK_HFCLKSTAT                 (*(volatile uint32_t *)(NRF_CLOCK_BASE + 0x40C))

/* ── Cortex-M33 SysTick ─────────────────────────────────────────────── */

#define SYST_CSR  (*(volatile uint32_t *)0xE000E010UL)  /* Control & Status */
#define SYST_RVR  (*(volatile uint32_t *)0xE000E014UL)  /* Reload Value     */
#define SYST_CVR  (*(volatile uint32_t *)0xE000E018UL)  /* Current Value    */

/* ── Cortex-M33 NVIC ────────────────────────────────────────────────── */

#define NVIC_ISER0  (*(volatile uint32_t *)0xE000E100UL)

/* nRF5340 application core IRQ numbers */
#define UARTE0_IRQn   8
#define UARTE1_IRQn   9

static inline void NVIC_EnableIRQ(uint32_t irqn)
{
    NVIC_ISER0 = (1UL << irqn);
}

/* ── Cortex-M33 SCB ──────────────────────────────────────────────────── */

#define SCB_CPACR  (*(volatile uint32_t *)0xE000ED88UL)

/* ── System helpers (implemented in startup_nrf5340.c) ───────────────── */

uint32_t millis(void);
void     delay_ms(uint32_t ms);

#endif /* NRF5340_H */

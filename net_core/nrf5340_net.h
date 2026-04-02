#ifndef NRF5340_NET_H
#define NRF5340_NET_H

/*
 * Minimal baremetal register definitions for the nRF5340 NETWORK core.
 * Non-secure domain addresses.
 *
 * The struct layouts are identical to the application core — only base
 * addresses and IRQ numbers differ.
 */

#include <stdint.h>

#define SYSTEM_CLOCK_HZ  64000000UL   /* Net core runs at 64 MHz */

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

/* Network core GPIO base addresses */
#define NRF_P0  ((NRF_GPIO_Type *)0x418C0500UL)
#define NRF_P1  ((NRF_GPIO_Type *)0x418C0800UL)

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

/* Network core has only UARTE0 */
#define NRF_UARTE0  ((NRF_UARTE_Type *)0x41013000UL)

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

#define NRF_CLOCK_BASE                  0x41005000UL
#define CLOCK_TASKS_HFCLKSTART          (*(volatile uint32_t *)(NRF_CLOCK_BASE + 0x000))
#define CLOCK_EVENTS_HFCLKSTARTED       (*(volatile uint32_t *)(NRF_CLOCK_BASE + 0x100))
#define CLOCK_HFCLKSTAT                 (*(volatile uint32_t *)(NRF_CLOCK_BASE + 0x40C))

/* ── IPC peripheral ──────────────────────────────────────────────────── */

#define NRF_IPC_BASE  0x4002A000UL

typedef struct {
    volatile uint32_t TASKS_SEND[16];      /* +0x000 .. +0x03C */
    volatile uint32_t RESERVED0[48];       /* +0x040 .. +0x0FC */
    volatile uint32_t EVENTS_RECEIVE[16];  /* +0x100 .. +0x13C */
    volatile uint32_t RESERVED1[48];       /* +0x140 .. +0x1FC */
    volatile uint32_t RESERVED2[64];       /* +0x200 .. +0x2FC */
    volatile uint32_t INTEN;               /* +0x300 */
    volatile uint32_t INTENSET;            /* +0x304 */
    volatile uint32_t INTENCLR;            /* +0x308 */
    volatile uint32_t RESERVED3[61];       /* +0x30C .. +0x3FC */
    volatile uint32_t RESERVED4[64];       /* +0x400 .. +0x4FC */
    volatile uint32_t SEND_CNF[16];        /* +0x500 .. +0x53C */
    volatile uint32_t RESERVED5[16];       /* +0x540 .. +0x57C */
    volatile uint32_t RECEIVE_CNF[16];     /* +0x580 .. +0x5BC */
} NRF_IPC_Type;

#define NRF_IPC  ((NRF_IPC_Type *)NRF_IPC_BASE)

/* ── Cortex-M33 SysTick ─────────────────────────────────────────────── */

#define SYST_CSR  (*(volatile uint32_t *)0xE000E010UL)
#define SYST_RVR  (*(volatile uint32_t *)0xE000E014UL)
#define SYST_CVR  (*(volatile uint32_t *)0xE000E018UL)

/* ── Cortex-M33 NVIC ────────────────────────────────────────────────── */

#define NVIC_ISER0  (*(volatile uint32_t *)0xE000E100UL)

/* nRF5340 network-core IRQ numbers (from PS table 4.3.4) */
#define SERIAL0_IRQn   5     /* UARTE0 / SPIM0 / SPIS0 / TWIM0 / TWIS0 */
#define IPC_IRQn       18

static inline void NVIC_EnableIRQ(uint32_t irqn)
{
    NVIC_ISER0 = (1UL << irqn);
}

/* ── Memory barrier ──────────────────────────────────────────────────── */

static inline void __DMB(void)
{
    __asm volatile ("dmb" ::: "memory");
}

/* ── System helpers (implemented in startup_nrf5340_net.c) ──────────── */

uint32_t millis(void);
void     delay_ms(uint32_t ms);

#endif /* NRF5340_NET_H */

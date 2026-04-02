/*
 * Baremetal startup for nRF5340 network core (Cortex-M33, no FPU).
 *
 * Provides: vector table, Reset_Handler, SysTick 1 ms tick,
 *           millis() / delay_ms() helpers.
 */

#include "nrf5340_net.h"

/* ── Linker-provided symbols ─────────────────────────────────────────── */

extern uint32_t _estack;
extern uint32_t _sidata, _sdata, _edata;
extern uint32_t _sbss, _ebss;

extern int main(void);

/* ── Millisecond tick ────────────────────────────────────────────────── */

static volatile uint32_t systick_count;

void SysTick_Handler(void)
{
    systick_count++;
}

uint32_t millis(void)
{
    return systick_count;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = systick_count;
    while ((systick_count - start) < ms)
        ;
}

/* ── System initialisation ───────────────────────────────────────────── */

static void SystemInit(void)
{
    /* No FPU on the network core — skip CPACR */

    /* Start HFCLK for accurate UART timing */
    CLOCK_EVENTS_HFCLKSTARTED = 0;
    CLOCK_TASKS_HFCLKSTART    = 1;
    while (!CLOCK_EVENTS_HFCLKSTARTED)
        ;

    /* SysTick: 1 ms tick at 64 MHz */
    SYST_RVR = (SYSTEM_CLOCK_HZ / 1000) - 1;   /* 63999 */
    SYST_CVR = 0;
    SYST_CSR = 0x07;   /* enable | tickint | clksource = processor */
}

/* ── Reset handler ───────────────────────────────────────────────────── */

void Reset_Handler(void)
{
    /* Copy .data from flash to RAM */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata)
        *dst++ = *src++;

    /* Zero .bss */
    dst = &_sbss;
    while (dst < &_ebss)
        *dst++ = 0;

    SystemInit();
    main();

    for (;;)
        ;
}

/* ── Default fault / interrupt handler ───────────────────────────────── */

void Default_Handler(void)
{
    for (;;)
        ;
}

/* ── ISRs defined elsewhere ──────────────────────────────────────────── */

void SERIAL0_Handler(void);   /* yrm100_uart_nrf5340_net.c (UARTE0 RX) */
void IPC_Handler(void);       /* ipc_net.c (future: app→net commands)   */

/* ── Vector table (nRF5340 network core, PS table 4.3.4) ────────────── */

#define DH Default_Handler

__attribute__((section(".isr_vector"), used))
void (* const g_pfnVectors[])(void) = {
    /* Cortex-M33 system exceptions */
    (void (*)(void))&_estack,   /*  0  Initial SP                */
    Reset_Handler,               /*  1  Reset                     */
    DH,                          /*  2  NMI                       */
    DH,                          /*  3  HardFault                 */
    DH,                          /*  4  MemManage                 */
    DH,                          /*  5  BusFault                  */
    DH,                          /*  6  UsageFault                */
    DH,                          /*  7  SecureFault               */
    0, 0, 0,                     /*  8-10  Reserved               */
    DH,                          /* 11  SVCall                    */
    DH,                          /* 12  DebugMonitor              */
    0,                           /* 13  Reserved                  */
    DH,                          /* 14  PendSV                    */
    SysTick_Handler,             /* 15  SysTick                   */

    /* nRF5340 network-core external interrupts */
    DH,                          /* IRQ  0  —                     */
    DH,                          /* IRQ  1  —                     */
    DH,                          /* IRQ  2  —                     */
    DH,                          /* IRQ  3  —                     */
    DH,                          /* IRQ  4  —                     */
    SERIAL0_Handler,             /* IRQ  5  UARTE0/SPIM0/…        */
    DH,                          /* IRQ  6  SERIAL1               */
    DH,                          /* IRQ  7  —                     */
    DH,                          /* IRQ  8  TIMER0                */
    DH,                          /* IRQ  9  RTC0                  */
    DH,                          /* IRQ 10  RTC1                  */
    DH,                          /* IRQ 11  —                     */
    DH,                          /* IRQ 12  —                     */
    DH,                          /* IRQ 13  —                     */
    DH,                          /* IRQ 14  —                     */
    DH,                          /* IRQ 15  —                     */
    DH,                          /* IRQ 16  —                     */
    DH,                          /* IRQ 17  —                     */
    IPC_Handler,                 /* IRQ 18  IPC                   */
    DH,                          /* IRQ 19  —                     */
    DH,                          /* IRQ 20  —                     */
    DH,                          /* IRQ 21  —                     */
    DH,                          /* IRQ 22  —                     */
    DH,                          /* IRQ 23  —                     */
    DH,                          /* IRQ 24  —                     */
    DH,                          /* IRQ 25  —                     */
    DH,                          /* IRQ 26  —                     */
    DH,                          /* IRQ 27  —                     */
    DH,                          /* IRQ 28  —                     */
    DH,                          /* IRQ 29  —                     */
    DH,                          /* IRQ 30  —                     */
    DH,                          /* IRQ 31  —                     */
};

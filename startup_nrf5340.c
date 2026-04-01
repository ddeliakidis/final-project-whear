/*
 * Baremetal startup for nRF5340 application core (Cortex-M33).
 *
 * Provides: vector table, Reset_Handler, SysTick 1 ms tick,
 *           millis() / delay_ms() helpers.
 */

#include "nrf5340.h"

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
    /* Enable FPU (CP10 + CP11 full access) */
    SCB_CPACR |= (0xFUL << 20);
    __asm volatile ("dsb");
    __asm volatile ("isb");

    /* Start HFXO (32 MHz crystal → accurate peripheral clocking) */
    CLOCK_EVENTS_HFCLKSTARTED = 0;
    CLOCK_TASKS_HFCLKSTART    = 1;
    while (!CLOCK_EVENTS_HFCLKSTARTED)
        ;

    /* SysTick: 1 ms tick at 64 MHz */
    SYST_RVR = (SYSTEM_CLOCK_HZ / 1000) - 1;
    SYST_CVR = 0;
    SYST_CSR = 0x07;   /* enable | tickint | clksource = processor */
}

/* ── Reset handler ───────────────────────────────────────────────────── */

void Reset_Handler(void)
{
    /* Copy .data from flash → RAM */
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

/* ── UARTE1 ISR — defined in yrm100_uart_nrf5340.c ──────────────────── */

void UARTE1_Handler(void);

/* ── Vector table ────────────────────────────────────────────────────── */

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
    DH,                          /*  7  SecureFault (ARMv8-M)     */
    0, 0, 0,                     /*  8-10  Reserved               */
    DH,                          /* 11  SVCall                    */
    DH,                          /* 12  DebugMonitor              */
    0,                           /* 13  Reserved                  */
    DH,                          /* 14  PendSV                    */
    SysTick_Handler,             /* 15  SysTick                   */

    /* nRF5340 application-core external interrupts */
    DH,                          /* IRQ  0  FPU                   */
    DH,                          /* IRQ  1  CACHE                 */
    DH,                          /* IRQ  2  —                     */
    DH,                          /* IRQ  3  SPU                   */
    DH,                          /* IRQ  4  —                     */
    DH,                          /* IRQ  5  CLOCK_POWER           */
    DH,                          /* IRQ  6  —                     */
    DH,                          /* IRQ  7  —                     */
    DH,                          /* IRQ  8  UARTE0 / SPIM0 / …   */
    UARTE1_Handler,              /* IRQ  9  UARTE1 / SPIM1 / …   */
    DH,                          /* IRQ 10  SPIM4                 */
    DH,                          /* IRQ 11  —                     */
    DH,                          /* IRQ 12  SPIM2/TWIM2/…        */
    DH,                          /* IRQ 13  SPIM3/TWIM3/…        */
    DH,                          /* IRQ 14  GPIOTE0               */
    DH,                          /* IRQ 15  SAADC                 */
    DH,                          /* IRQ 16  TIMER0                */
    DH,                          /* IRQ 17  TIMER1                */
    DH,                          /* IRQ 18  TIMER2                */
    DH,                          /* IRQ 19  —                     */
    DH,                          /* IRQ 20  RTC0                  */
    DH,                          /* IRQ 21  RTC1                  */
    DH,                          /* IRQ 22  —                     */
    DH,                          /* IRQ 23  —                     */
    DH,                          /* IRQ 24  WDT0                  */
    DH,                          /* IRQ 25  —                     */
    DH,                          /* IRQ 26  EGU0                  */
    DH,                          /* IRQ 27  EGU1                  */
    DH,                          /* IRQ 28  —                     */
    DH,                          /* IRQ 29  —                     */
    DH,                          /* IRQ 30  PWM0                  */
    DH,                          /* IRQ 31  PWM1                  */
};

/*
 * Linker script for nRF5340 network core (baremetal).
 *
 * Flash: 256 KB at 0x01000000
 * RAM:    63 KB at 0x21000000  (64 KB minus 1 KB shared)
 * Shared:  1 KB at 0x2100FC00  (IPC mailbox with app core)
 */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x01000000, LENGTH = 256K
    RAM   (rwx) : ORIGIN = 0x21000000, LENGTH = 63K
    SHMEM (rw)  : ORIGIN = 0x2100FC00, LENGTH = 1K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .ARM.exidx :
    {
        *(.ARM.exidx*)
    } > FLASH

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT> FLASH

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM
}


MEMORY
{
    XRAM (rx): ORIGIN = 0x2003F800, LENGTH = 0x800
    RAM (rw):  ORIGIN = 0x2003F000, LENGTH = 0x800
}

SECTIONS
{
    .text :
    {
        *(.text*)
        KEEP(*(.init))
        KEEP(*(.fini))
        *(.rodata*)
    } > XRAM

    .data :
    {
        __data_start__ = .;
        KEEP(*(.functab*))
        *(.data*)
        __data_end__ = .;
    } > RAM

    .bss :
    {
        __bss_start__ = .;
        *(.bss*)
        __bss_end__ = .;
    } > RAM
}

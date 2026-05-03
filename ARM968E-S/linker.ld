ENTRY(_start)

MEMORY
{
    DDR (rwx) : ORIGIN = 0x82000000, LENGTH = 16M
}

SECTIONS
{
    . = ORIGIN(DDR);

    .text : ALIGN(4)
    {
        KEEP(*(.vectors))
        KEEP(*(.text.start))
        *(.text*)
        *(.rodata*)
    } > DDR

    .data : ALIGN(4)
    {
        __data_start__ = .;
        *(.data*)
        __data_end__ = .;
    } > DDR

    .bss (NOLOAD) : ALIGN(4)
    {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        __bss_end__ = .;
    } > DDR

    . = ALIGN(8);
    __image_end__ = .;

    /*
     * Put stack at top of the 16 MB DDR region.
     * Stack grows downward.
     */
    __stack_top__ = ORIGIN(DDR) + LENGTH(DDR);
    __stack_top = __stack_top__;
}

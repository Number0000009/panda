ENTRY(_start)

ARM_LOAD_ADDR = 0x82000000;
STACK_TOP     = 0x8200F000;

SECTIONS {
    . = ARM_LOAD_ADDR;

    .text : ALIGN(4) {
        KEEP(*(.text.start))
        *(.text*)
        *(.rodata*)
    }

    .data : ALIGN(4) {
        *(.data*)
    }

    .bss (NOLOAD) : ALIGN(4) {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    _image_end = .;
    ASSERT(_image_end <= STACK_TOP, "ARM image overlaps stack")

    . = STACK_TOP;
    _stack_top = .;

    /DISCARD/ : {
        *(.comment*)
        *(.note*)
        *(.ARM.attributes*)
        *(.eh_frame*)
        *(.rel*)
        *(.rela*)
    }
}

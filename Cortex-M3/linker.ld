
ENTRY(_start)

SECTIONS
{
  . = 0x82000000;	/* Pandaboard DRAM (initialized by u-boot) */
  .text : {
    *(.text*)
    *(.rodata*)
    *(.data*)
  }
  .bss : {
    *(.bss*)
  }
}

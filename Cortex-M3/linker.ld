
ENTRY(_start)

SECTIONS
{
  /* We keep it position-independent so you can load it anywhere and `go <addr>` */
  . = 0x0;
  .text : {
    *(.text*)
    *(.rodata*)
    *(.data*)
  }
  .bss : {
    *(.bss*)
  }
}

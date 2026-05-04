# PandaBoard OMAP4430 DSP bring-up

A tiny Cortex-A9 bare-metal program that proves the PandaBoard OMAP4430 C64T /
64T DSP can be powered, clocked, reset, mapped through the DSP MMU, and made to
execute a small hand-encoded payload from SDRAM.

The DSP in OMAP4430 is a TI C6000-family core. TI's OMAP4 material usually
calls this core **C64T** or **64T**, and the surrounding DSP subsystem is often
called **Tesla**. Other notes sometimes say C64x/C64x+ or TMS320C64x/C64x+,
but this code targets the OMAP4430 C64T / 64T DSP core and its C6000-style
instruction encoding.

## References

- [TMS320C64x/C64x+ DSP CPU and Instruction Set Reference Guide (SPRU732J)](https://www.ti.com/lit/pdf/spru732)
  covers the C6000 CPU, instruction set, and instruction formats behind the
  hand-encoded DSP payload.
- [OMAP4430 Multimedia Device Silicon Revision 2.x Technical Reference Manual (SWPU231AP)](https://www.ti.com/lit/pdf/swpu231)
  covers the OMAP4430 PRCM, CONTROL, DSP MMU, and UART register blocks.

## Memory layout

```text
Cortex-A9 app / binary load address: 0x82000000
Cortex-A9 stack top:                 0x8200F000

Shared magic word, ARM PA:           0x8200FC00
Shared magic word, DSP VA:           0x2000FC00

DSP payload, ARM PA:                 0x82010000
DSP payload, DSP VA / boot address:  0x20010000

UART3 THR, ARM/DSP VA:               0x48020000
```

The DSP MMU is programmed with two 1 MiB TLB entries:

```text
DSP VA 0x20000000 -> ARM PA 0x82000000
DSP VA 0x48000000 -> ARM PA 0x48000000
```

The first mapping covers the payload and shared magic word. The second mapping
lets the DSP write directly to UART3, so a raw byte from DSP code is visible on
the same serial console as the Cortex-A9 output.

## What the code does

`start.S` is the small Cortex-A9 entry point. It sets the stack, cleans L2 if it
was enabled, disables the Cortex-A9 L1 caches/MMU/branch predictor, clears BSS,
and calls `main()`.

`main.c` then does the DSP bring-up:

1. Clear the shared memory area and the payload page.
2. Copy the hand-encoded C64T DSP payload to ARM physical `0x82010000`.
3. Force the DSP power domain on.
4. Enable the DSP clock domain and DSP module clock.
5. Assert DSP `RST1|RST2`.
6. Release `RST2` while keeping the DSP core held in `RST1`.
7. Reset the DSP MMU.
8. Load the SDRAM and UART3 DSP MMU TLB entries.
9. Write `CONTROL_DSP_BOOTADDR = 0x20010000`.
10. Release DSP `RST1`.
11. Poll ARM physical `0x8200FC00` until the DSP writes the magic value.

The DSP payload itself does this:

```text
B0 = 0x48020000
B1 = '!'
STW B1,*B0        ; print '!' on UART3

B0 = 0x2000FC00
B1 = 0xC0DEF00D
STW B1,*B0        ; write shared magic
LDW *B0,B2        ; mapped readback, drains/confirms the store path

loop:
B0 = loop
B B0
NOP 5             ; 5 branch delay slots
```

## Building

```sh
make
```

## Expected result

```text
DSP magic writer
DSP release, expect !: !
OK magic SHM0=C0DEF00D
DONE
```

The `!` after `expect !:` is printed by the DSP, not by the Cortex-A9.
That byte confirms the DSP executed code and reached the UART3 store. The
`SHM0=C0DEF00D` line confirms the DSP also wrote through the SDRAM mapping.

## Failure output

If the DSP MMU reports a fault before the magic word appears, the Cortex-A9
prints the IRQ status, fault address, fault PC, and fault status. If no magic
word appears and no DSP MMU fault is reported, the Cortex-A9 prints a magic
timeout with the last observed shared word and IRQ status.

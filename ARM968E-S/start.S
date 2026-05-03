.syntax unified
.cpu cortex-a9
.arm

.global _start
.extern main
.extern __bss_start__
.extern __bss_end__
.extern __stack_top__

.section .vectors, "ax"
_start:
    b reset
    b hang
    b hang
    b hang
    b hang
    b hang
    b hang
    b hang

.section .text.start, "ax"
reset:
    cpsid if

    ldr sp, =__stack_top__

    bl zero_bss

    bl main

hang:
    b hang

zero_bss:
    ldr r0, =__bss_start__
    ldr r1, =__bss_end__
    mov r2, #0

1:
    cmp r0, r1
    strlo r2, [r0], #4
    blo 1b

    bx lr

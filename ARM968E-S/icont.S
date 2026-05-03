.syntax unified
.arch armv5te
.arm

.equ DMEM_BASE,      0x00400000
.equ DMEM_TOP,       0x00404000

.equ OFF_MAGIC,      0x00
.equ OFF_COUNTER,    0x04
.equ OFF_HEAD,       0x08
.equ OFF_TAIL,       0x0C

.equ OFF_CFG_MAGIC,  0x20
.equ OFF_CFG_IDCHAR, 0x24

.equ OFF_TXBUF,      0x100
.equ TX_MASK,        0xFF

.section .vectors, "ax"
.global _start

_start:
    b reset
    b hang
    b hang
    b hang
    b hang
    b hang
    b hang
    b hang

.section .text, "ax"

reset:
    ldr sp, =DMEM_TOP
    ldr r7, =DMEM_BASE

    ldr r0, [r7, #OFF_CFG_MAGIC]
    str r0, [r7, #OFF_MAGIC]

    mov r0, #0
    str r0, [r7, #OFF_COUNTER]
    str r0, [r7, #OFF_HEAD]
    str r0, [r7, #OFF_TAIL]

    ldr r4, =msg_icont
    bl puts

    ldr r0, [r7, #OFF_CFG_IDCHAR]
    bl putc

    ldr r4, =msg_boot
    bl puts

main_loop:
    ldr r0, [r7, #OFF_COUNTER]
    add r0, r0, #1
    str r0, [r7, #OFF_COUNTER]

    ldr r4, =msg_icont
    bl puts

    ldr r0, [r7, #OFF_CFG_IDCHAR]
    bl putc

    ldr r4, =msg_alive
    bl puts

    bl delay

    b main_loop

puts:
    stmfd sp!, {r4, lr}

puts_loop:
    ldrb r0, [r4], #1
    cmp r0, #0
    beq puts_done

    bl putc
    b puts_loop

puts_done:
    ldmfd sp!, {r4, pc}

putc:
    ldr r1, [r7, #OFF_HEAD]

putc_wait:
    add r2, r1, #1
    and r2, r2, #TX_MASK

    ldr r3, [r7, #OFF_TAIL]
    cmp r2, r3
    beq putc_wait

    add r3, r7, #OFF_TXBUF
    strb r0, [r3, r1]

    str r2, [r7, #OFF_HEAD]
    bx lr

delay:
    ldr r6, =5000000

1:
    subs r6, r6, #1
    bne 1b

    bx lr

hang:
    b hang

.balign 4
msg_icont:
    .asciz "ICONT"

msg_boot:
    .asciz " boot\n"

msg_alive:
    .asciz " alive\n"

.section .note.GNU-stack, "", %progbits

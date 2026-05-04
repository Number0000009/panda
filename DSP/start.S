.syntax unified
.cpu cortex-a9
.arm

.global _start
.extern main
.extern __bss_start
.extern __bss_end
.extern _stack_top

.equ PL310_BASE,		0x48242000
.equ PL310_CTRL,		0x100
.equ PL310_CACHE_SYNC,		0x730
.equ PL310_CLEAN_INV_WAY,	0x7FC

.section .text.start, "ax"

_start:
	cpsid	if

	ldr	sp, =_stack_top

	bl	clean_l2_if_enabled_no_disable

	bl	disable_l1_caches_and_mmu

	ldr	r0, =__bss_start
	ldr	r1, =__bss_end
	mov	r2, #0
1:
	cmp	r0, r1
	bhs	2f
	str	r2, [r0], #4
	b	1b

2:
	bl	main

3:
	wfi
	b	3b

clean_l2_if_enabled_no_disable:
	push	{r4, lr}
	ldr	r0, =PL310_BASE
	ldr	r1, [r0, #PL310_CTRL]
	tst	r1, #1
	beq	2f

	ldr	r1, =0x0000FFFF
	str	r1, [r0, #PL310_CLEAN_INV_WAY]
1:
	ldr	r2, [r0, #PL310_CLEAN_INV_WAY]
	tst	r2, r1
	bne	1b

	mov	r1, #0
	str	r1, [r0, #PL310_CACHE_SYNC]
2:
	pop	{r4, pc}

disable_l1_caches_and_mmu:
	push	{r4-r11, lr}

	bl	v7_clean_invalidate_l1_dcache_all

	dsb
	isb

	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ invalidate I-cache
	mcr	p15, 0, r0, c8, c7, 0		@ invalidate unified TLB
	dsb
	isb

	mrc	p15, 0, r0, c1, c0, 0		@ SCTLR
	bic	r0, r0, #(1 << 12)		@ I-cache disable
	bic	r0, r0, #(1 << 11)		@ branch predictor disable
	bic	r0, r0, #(1 << 2)		@ D-cache disable
	bic	r0, r0, #(1 << 0)		@ MMU disable
	mcr	p15, 0, r0, c1, c0, 0
	dsb
	isb

	pop	{r4-r11, pc}

v7_clean_invalidate_l1_dcache_all:
	dmb
	mrc	p15, 1, r0, c0, c0, 1		@ CLIDR
	ands	r3, r0, #0x07000000		@ LoC
	mov	r3, r3, lsr #23			@ cache level count * 2
	beq	5f

	mov	r10, #0				@ cache level selector: 0,2,4...
1:
	add	r2, r10, r10, lsr #1		@ 3 * level
	mov	r1, r0, lsr r2
	and	r1, r1, #7			@ cache type
	cmp	r1, #2
	blt	4f

	mcr	p15, 2, r10, c0, c0, 0		@ CSSELR
	isb
	mrc	p15, 1, r1, c0, c0, 0		@ CCSIDR

	and	r2, r1, #7
	add	r2, r2, #4

	ldr	r4, =0x000003FF
	and	r4, r4, r1, lsr #3
	clz	r5, r4

	ldr	r7, =0x00007FFF
	and	r7, r7, r1, lsr #13
2:
	mov	r9, r4
3:
	orr	r11, r10, r9, lsl r5
	orr	r11, r11, r7, lsl r2
	mcr	p15, 0, r11, c7, c14, 2		@ DCCISW
	subs	r9, r9, #1
	bge	3b
	subs	r7, r7, #1
	bge	2b

4:
	add	r10, r10, #2
	cmp	r3, r10
	bgt	1b

5:
	mov	r10, #0
	mcr	p15, 2, r10, c0, c0, 0
	dsb
	isb
	bx	lr

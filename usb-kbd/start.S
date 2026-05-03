.global _start
.extern main
.extern __bss_start
.extern __bss_end

_start:
	cpsid	if				@ disable IRQ and FIQ interrupts

	/* Disable MMU, D-cache, I-cache. */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #(1 << 0)		@ MMU off
	bic	r0, r0, #(1 << 2)		@ D-cache off
	bic	r0, r0, #(1 << 12)		@ I-cache off
	mcr	p15, 0, r0, c1, c0, 0
	dsb
	isb

	ldr	r0, =__bss_start
	ldr	r1, =__bss_end
	mov	r2, #0

clear_bss:
	cmp	r0, r1
	bhs	bss_done
	strb	r2, [r0], #1
	b	clear_bss

bss_done:
	ldr	sp, =0x88000000

	bl	main

halt_loop:
	wfe
	b	halt_loop

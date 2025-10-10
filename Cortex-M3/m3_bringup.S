	.syntax unified
	.cpu cortex-a9
	.arm
	.global _start

_start:
	ldr r0, =0x4A008920		// Set CM_MPU_M3_MPU_M3_CLKCTRL to AUTO mode (MODULEMODE = 0x1)
	mov r1, #1
	str r1, [r0]

	ldr r0, =0x4A008900		// Set CM_MPU_M3_CLKSTCTRL to SW_WKUP (CLKTRCTRL = 0x2)
	mov r1, #2
	str r1, [r0]

	ldr r0, =0x4A307100		// Program the reset timer for PRCM internal deassertion
	ldr r1, =0x00007C00
	str r1, [r0]

	ldr r0, =0x4A306910		// Assert all resets: RST3 | RST2 | RST1
	mov r1, #7
	str r1, [r0]

	ldr r0, =0x4A306910		// Release MMU/cache reset (clear RST3)
	mov r1, #3
	str r1, [r0]

	ldr r0, =0x4A306914		// Wait until RST3ST (bit 2) is latched in RM_MPU_M3_RSTST
wait_rst3:
	ldr r1, [r0]
	tst r1, #4			// bit 2	( (r1 & #4) -> if 1 -> Z is cleared, if 0 -> Z is set => BNE taken if Z=0; BEQ taken if Z=1 )
	beq wait_rst3

	ldr r0, =0x4A306914		// Clear RST3 status
	mov r1, #4
	str r1, [r0]

// Ducati MMU/TLB (0x550820xx)
	ldr r0, =0x55082044		// CNTL: MMU disable
	mov r1, #0
	str r1, [r0]

	ldr r0, =0x55082050		// LOCK: base=0, victim=0
	mov r1, #0
	str r1, [r0]

// 1MiB mapping for M3 SRAM VA 0x00000000 -> PA 0x40300000
	ldr r0, =0x55082058		// CAM: VA 0x00000000, V=1, 1MiB
	mov r1, #4
	str r1, [r0]

	ldr r0, =0x5508205C		// RAM
	ldr r1, =0x40300000		// PA = 0x40300000
	str r1, [r0]

	ldr r0, =0x55082054		// LD_TLB
	mov r1, #1
	str r1, [r0]

	ldr r0, =0x55082050		// LOCK: BASEVALUE=1, CURRENTVICTIM=1
	ldr r1, =0x00000410
	str r1, [r0]

// 4KiB mapping for UART3 at 0x48020000
	ldr r0, =0x55082058		// CAM: V=1, PGSZ=4KB, VATAG=0x48020000
	ldr r1, =0x48020006
	str r1, [r0]

	ldr r0, =0x5508205C		// RAM: PA (ENDIAN=0, ELSZ=8, MIXED=0)
	ldr r1, =0x48020000
	str r1, [r0]

	ldr r0, =0x55082054		// LD_TLB
	mov r1, #1
	str r1, [r0]

	ldr r0, =0x55082050		// LOCK: base=2, vict=2 (protect 0..1)
	ldr r1, =0x00000820
	str r1, [r0]

	ldr r0, =0x55082044		// CNTL: MMU_EN = 1
	mov r1, #2
	str r1, [r0]

// Populate M3 SRAM at VA = 0 (PA = 0x40300000)

	ldr r0, =0x40300000		// Cortex-M3 SP
	ldr r1, =0x00010000		// SP = 0x00010000
	str r1, [r0]

	ldr r0, =0x40300004		// Cortex-M3 PC
	ldr r1, =0x00000101		// PC = 0x00000100 | Thumb (odd address)
	str r1, [r0]

// Copy actual Cortex-M3 program into Cortex-M3 VA=0x100 PA=0x40300100
	ldr r0, =cortex_m3_program
	ldr r1, =0x40300100
	mov r2, #(((cortex_m3_program_end-cortex_m3_program)/2)-1)		// thumb-2 ops can be 2 or 4 bytes long, so let's use 2 bytes copy

memcpy_loop:
	ldrh r3, [r0], #2
	strh r3, [r1], #2
	subs r2, r2, #1
	bne memcpy_loop

	ldr r0, =0x4A306910		// Release M3_0 reset (clear RST1)
	mov r1, #2
	str r1, [r0]

	b .


// Actual Cortex-M3 program
	.cpu cortex-m3
	.thumb
	.syntax unified
cortex_m3_program:
//	ldr r0, =0x48020000		// UART3 base
	movw r0, #0x0000		// literal pools cannot be used at the moment, need more advanced linker script
	movt r0, #0x4802
	mov r1, #'!'
	strb r1, [r0]
	b .
cortex_m3_program_end:

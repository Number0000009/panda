#include <stdint.h>

/*
 * Hardware chain:
 * Cortex-A9 -> OMAP4430 PRCM/CONTROL -> DSP MMU -> Tesla C64T / 64T DSP.
 * DSP VA 0x20000000 maps ARM SDRAM at 0x82000000.
 * DSP VA 0x48000000 maps UART3 registers at 0x48000000.
 *
 * DSP bring-up path:
 * 1. Copy a tiny C64T payload to SDRAM at 0x82010000.
 * 2. Power and clock the DSP domain.
 * 3. Hold the DSP core in RST1 while releasing RST2.
 * 4. Program the DSP MMU with SDRAM and UART3 mappings.
 * 5. Set CONTROL_DSP_BOOTADDR to DSP VA 0x20010000.
 * 6. Release RST1 and wait for the DSP to write 0xC0DEF00D.
 */

#define R32(a) (*(volatile uint32_t *)(uintptr_t)(a))

#define UART3 0x48020000u				// PandaBoard console UART

#define CM_DSP_CLKSTCTRL 0x4A004400u			// DSP clock-domain control
#define CM_DSP_DSP_CLKCTRL 0x4A004420u			// DSP module clock control

#define PM_DSP_PWRSTCTRL 0x4A306400u			// DSP power-state control
#define PM_DSP_PWRSTST 0x4A306404u			// DSP power-state status
#define RM_DSP_RSTCTRL 0x4A306410u			// DSP reset control
#define RM_DSP_RSTST 0x4A306414u			// DSP reset status
#define PRM_RSTTIME 0x4A307100u				// global reset pulse timing

#define CONTROL_DSP_BOOTADDR 0x4A002304u		// DSP boot virtual address

#define DSP_MMU_SYSCONFIG 0x4A066010u			// DSP MMU soft reset
#define DSP_MMU_SYSSTATUS 0x4A066014u			// DSP MMU reset done
#define DSP_MMU_IRQSTATUS 0x4A066018u			// DSP MMU interrupt status
#define DSP_MMU_IRQENABLE 0x4A06601Cu			// DSP MMU interrupt enable
#define DSP_MMU_CNTL 0x4A066044u			// DSP MMU enable/control
#define DSP_MMU_FAULT_AD 0x4A066048u			// faulting DSP virtual address
#define DSP_MMU_TTB 0x4A06604Cu				// table-walk base, unused here
#define DSP_MMU_LOCK 0x4A066050u			// TLB entry select
#define DSP_MMU_LD_TLB 0x4A066054u			// load selected TLB entry
#define DSP_MMU_CAM 0x4A066058u				// TLB virtual-side descriptor
#define DSP_MMU_RAM 0x4A06605Cu				// TLB physical-side descriptor
#define DSP_MMU_GFLUSH 0x4A066060u			// global TLB flush
#define DSP_MMU_FLUSH_ENTRY 0x4A066064u			// selected-entry flush
#define DSP_MMU_FAULT_PC 0x4A066080u			// DSP PC near the fault
#define DSP_MMU_FAULT_STATUS 0x4A066084u		// DSP MMU fault type

#define ARM_SHM 0x8200FC00u				// shared words visible to Cortex-A9
#define ARM_PAYLOAD 0x82010000u				// where Cortex-A9 copies DSP code
#define ARM_PAYLOAD_SIZE 0x1000u			// one payload page is plenty

#define DSP_EXT 0x20000000u				// DSP VA for ARM SDRAM window
#define DSP_BOOT 0x20010000u				// DSP boot VA / payload entry
#define DSP_SHM 0x2000FC00u				// DSP VA of ARM_SHM
#define DSP_UART3 UART3					// DSP VA of UART3 after MMU map

#define A9_STATUS 0x8200FC08u				// final Cortex-A9 result word
#define LAST_STEP 0x8200FC0Cu				// last high-level bring-up step
#define LAST_ADDR 0x8200FC10u				// last hardware register written
#define LAST_VALUE 0x8200FC14u				// value written to LAST_ADDR

#define MAGIC 0xC0DEF00Du				// DSP success value

#define OK_DONE 0xA9000000u				// Cortex-A9 observed DSP success
#define ERR_POWER 0xE0000001u				// DSP power domain did not turn on
#define ERR_CLOCK 0xE0000002u				// DSP module clock did not start
#define ERR_RST2 0xE0000003u				// DSP RST2 release was not observed
#define ERR_MMU_RESET 0xE0000004u			// DSP MMU reset did not complete
#define ERR_RST1 0xE0000005u				// DSP RST1 release was not observed
#define ERR_TIMEOUT 0xE0000006u				// DSP never wrote MAGIC

#define STEP_MAIN 0xA9005000u				// main() entered
#define STEP_POWER 0xA9005001u				// powering DSP domain
#define STEP_CLOCK 0xA9005002u				// enabling DSP clocks
#define STEP_ASSERT_RESET 0xA9005003u			// asserting RST1 and RST2
#define STEP_RELEASE_RST2 0xA9005004u			// releasing only RST2
#define STEP_MMU 0xA9005005u				// programming DSP MMU
#define STEP_BOOTADDR 0xA9005006u			// writing boot address
#define STEP_RELEASE_RST1 0xA9005007u			// releasing DSP core
#define STEP_POLL_MAGIC 0xA9005008u			// polling shared memory

#define POWER_ON 0x3u					// PRM power-state value ON
#define POWER_INTRANSITION (1u << 20)			// PRM power transition busy bit
#define CLK_SW_WKUP 0x2u				// force clock domain wakeup
#define MODULE_AUTO 0x1u				// module mode AUTO/enable
#define IDLEST_DISABLED 0x3u				// module idlest disabled
#define CLK_ACTIVE (1u << 8)				// DSP clock active status bit

#define RST1 (1u << 0)					// DSP core reset
#define RST2 (1u << 1)					// DSP subsystem reset
#define RSTST_ALL 0xFu					// clear all remembered reset status

#define MMU_SOFTRESET 0x2u				// DSP MMU soft reset bit
#define MMU_RESETDONE 0x1u				// DSP MMU reset complete bit
#define MMU_ENABLE 0x2u					// DSP MMU translation enable
#define MMU_IRQ_ALL 0x1Fu				// all DSP MMU fault bits
#define MMU_CAM_PRESERVED (1u << 3)			// keep TLB entry during flushes
#define MMU_CAM_VALID (1u << 2)				// valid TLB CAM entry
#define MMU_RAM_ELSZ_32 (2u << 7)			// 32-bit element size

/*
 * Hand-encoded C64T / 64T B-side payload:
 * 1. STW '!' to UART3 THR, proving peripheral MMU access.
 * 2. STW 0xC0DEF00D to DSP VA 0x2000FC00, proving SDRAM sharing.
 * 3. LDW the same word back to drain/confirm the store path.
 * 4. Branch to itself forever.
 */
static const uint32_t dsp_code[] = {
	0x0000002Au,		// MVKL .S2 0x48020000, B0
	0x0024016Au,		// MVKH .S2 0x48020000, B0
	0x008010AAu,		// MVKL .S2 0x00000021, B1
	0x0080006Au,		// MVKH .S2 0x00000021, B1
	0x008002F6u,		// STW  .D2T2 B1, *B0

	0x007E002Au,		// MVKL .S2 0x2000FC00, B0
	0x0010006Au,		// MVKH .S2 0x2000FC00, B0
	0x00F806AAu,		// MVKL .S2 0xC0DEF00D, B1
	0x00E06F6Au,		// MVKH .S2 0xC0DEF00D, B1
	0x008002F6u,		// STW  .D2T2 B1, *B0
	0x010002E6u,		// LDW  .D2T2 *B0, B2

	0x00001A2Au,		// MVKL .S2 0x20010034, B0
	0x001000EAu,		// MVKH .S2 0x20010034, B0
	0x00000362u,		// B    .S2 B0
	0x00000000u,		// NOP  branch delay slot 1
	0x00000000u,		// NOP  branch delay slot 2
	0x00000000u,		// NOP  branch delay slot 3
	0x00000000u,		// NOP  branch delay slot 4
	0x00000000u,		// NOP  branch delay slot 5
};

static uint32_t r32(uint32_t a)
{
	return R32(a);
}

static void barrier(void)
{
	__asm__ volatile("dsb sy; isb sy" ::: "memory");
}

static void w32(uint32_t a, uint32_t v)
{
	R32(a) = v;
	barrier();
}

static void step(uint32_t s)
{
	w32(LAST_STEP, s);
}

static void w32t(uint32_t a, uint32_t v)
{
	w32(LAST_ADDR, a);				// keep last register write address
	w32(LAST_VALUE, v);				// keep last register write value
	w32(a, v);					// perform the actual hardware write
}

static void delay(uint32_t n)
{
	while (n--)
		__asm__ volatile("nop");
}

static void putc(char c)
{
	if (c == '\n')
		putc('\r');
	while (!(r32(UART3 + 0x14) & 0x20)) {
	}
	w32(UART3, (uint32_t)(uint8_t)c);
}

static void puts(const char *s)
{
	while (*s)
		putc(*s++);
}

static void hex32(uint32_t v)
{
	for (int i = 28; i >= 0; i -= 4)
		putc("0123456789ABCDEF"[(v >> i) & 15]);
}

static void dumpv(const char *s, uint32_t v)
{
	puts(s);
	puts("=");
	hex32(v);
	puts("\n");
}

static int fail(uint32_t status, const char *s)
{
	w32(A9_STATUS, status);				// publish failure before printing
	puts(s);
	return -1;
}

static void halt(void)
{
	for (;;) {
	}
}

static void memzero32(uint32_t a, unsigned words)
{
	volatile uint32_t *p = (volatile uint32_t *)(uintptr_t)a;
	while (words--)
		*p++ = 0;
	barrier();
}

/*
 * Poll a register until at least one bit in the mask becomes set.
 * Used for reset-complete and reset-release handshakes.
 */
static int wait_set(uint32_t a, uint32_t m)
{
	for (uint32_t i = 0; i < 2000000u; i++)
		if (r32(a) & m)				// poll until any masked bit sets
			return 0;
	return -1;
}

static int wait_power_on(void)
{
	for (uint32_t i = 0; i < 2000000u; i++) {
		uint32_t v = r32(PM_DSP_PWRSTST);
		if (!(v & POWER_INTRANSITION) && ((v & 3u) == POWER_ON))
			return 0;
	}
	return -1;
}

static int wait_clock_on(void)
{
	for (uint32_t i = 0; i < 2000000u; i++) {
		uint32_t clkst = r32(CM_DSP_CLKSTCTRL);
		uint32_t clkctrl = r32(CM_DSP_DSP_CLKCTRL);
		uint32_t idlest = (clkctrl >> 16) & 3u;

		if ((clkst & CLK_ACTIVE) && idlest != IDLEST_DISABLED)
			return 0;
	}
	return -1;
}

static void load_payload(void)
{
	volatile uint32_t *dst = (volatile uint32_t *)(uintptr_t)ARM_PAYLOAD;

	memzero32(ARM_SHM, 0x100u / 4u);		// clear shared status/magic area
	memzero32(ARM_PAYLOAD, ARM_PAYLOAD_SIZE / 4u);	// clear stale DSP opcodes

	for (unsigned i = 0; i < sizeof(dsp_code) / sizeof(dsp_code[0]); i++)
		dst[i] = dsp_code[i];			// copy C64T words to SDRAM

	barrier();
}

static void dsp_tlb_1m(uint32_t idx, uint32_t va, uint32_t pa)
{
	uint32_t mask = 0xFFF00000u;
	uint32_t cam = (va & mask) | MMU_CAM_PRESERVED | MMU_CAM_VALID;
	uint32_t ram = (pa & mask) | MMU_RAM_ELSZ_32;

	w32t(DSP_MMU_LOCK, (idx << 10) | (idx << 4));	// select TLB slot
	w32t(DSP_MMU_CAM, cam);				// virtual page descriptor
	w32t(DSP_MMU_RAM, ram);				// physical page descriptor
	w32t(DSP_MMU_FLUSH_ENTRY, 1u);			// discard old matching entry
	w32t(DSP_MMU_LD_TLB, 1u);			// load CAM/RAM into selected slot
}

static int dsp_power_clock(void)
{
	step(STEP_POWER);
	w32t(PM_DSP_PWRSTCTRL, POWER_ON);		// request DSP power domain ON
	if (wait_power_on())
		return fail(ERR_POWER, "FAIL power\n");

	step(STEP_CLOCK);
	w32t(CM_DSP_DSP_CLKCTRL, MODULE_AUTO);		// enable DSP module clock
	w32t(CM_DSP_CLKSTCTRL, CLK_SW_WKUP);		// force clock domain wakeup
	if (wait_clock_on())
		return fail(ERR_CLOCK, "FAIL clock\n");

	return 0;
}

static int dsp_reset_for_mmu(void)
{
	step(STEP_ASSERT_RESET);
	w32t(PRM_RSTTIME, r32(PRM_RSTTIME) | 0x00007C00u);	// lengthen reset pulse
	w32t(RM_DSP_RSTST, RSTST_ALL);				// clear old reset status
	w32t(RM_DSP_RSTCTRL, RST1 | RST2);			// hold core and subsystem reset
	delay(10000u);

	step(STEP_RELEASE_RST2);
	w32t(RM_DSP_RSTST, RSTST_ALL);				// clear status before release
	w32t(RM_DSP_RSTCTRL, RST1);				// release RST2, keep core stopped
	if (wait_set(RM_DSP_RSTST, RST2))
		return fail(ERR_RST2, "FAIL rst2\n");

	w32t(RM_DSP_RSTST, RST2);				// acknowledge observed RST2 release
	return 0;
}

static int dsp_mmu(void)
{
	step(STEP_MMU);
	w32t(DSP_MMU_SYSCONFIG, MMU_SOFTRESET);		// reset DSP MMU
	if (wait_set(DSP_MMU_SYSSTATUS, MMU_RESETDONE))
		return fail(ERR_MMU_RESET, "FAIL mmu reset\n");

	w32t(DSP_MMU_CNTL, 0u);				// disable before changing TLB
	w32t(DSP_MMU_IRQENABLE, 0u);			// no DSP MMU interrupts needed
	w32t(DSP_MMU_IRQSTATUS, MMU_IRQ_ALL);		// clear stale fault bits
	w32t(DSP_MMU_TTB, 0u);				// no page-table walk
	w32t(DSP_MMU_GFLUSH, 1u);			// flush old TLB contents

	dsp_tlb_1m(0u, DSP_EXT, 0x82000000u);		// DSP SDRAM window
	dsp_tlb_1m(1u, DSP_UART3, UART3);		// DSP UART3 window

	w32t(DSP_MMU_LOCK, (2u << 10) | (2u << 4));	// next free slot after ours
	w32t(DSP_MMU_IRQSTATUS, MMU_IRQ_ALL);		// clear faults after TLB load
	w32t(DSP_MMU_CNTL, MMU_ENABLE);			// enable translation
	return 0;
}

static void dsp_stop(void)
{
	w32t(RM_DSP_RSTCTRL, RST1 | RST2);		// stop DSP cleanly after test
}

static int dsp_boot(void)
{
	step(STEP_BOOTADDR);
	w32t(CONTROL_DSP_BOOTADDR, DSP_BOOT);		// DSP virtual fetch address
	barrier();

	step(STEP_RELEASE_RST1);
	w32t(DSP_MMU_IRQSTATUS, MMU_IRQ_ALL);		// clear faults before launch
	w32t(RM_DSP_RSTST, RSTST_ALL);			// clear reset status before launch

	puts("DSP release, expect !: ");
	w32t(RM_DSP_RSTCTRL, 0u);			// release DSP core
	if (wait_set(RM_DSP_RSTST, RST1))
		return fail(ERR_RST1, "FAIL rst1\n");

	step(STEP_POLL_MAGIC);
	for (uint32_t i = 0; i < 30000000u; i++) {
		uint32_t magic = r32(ARM_SHM + 0u);
		uint32_t irq = r32(DSP_MMU_IRQSTATUS) & MMU_IRQ_ALL;

		if (magic == MAGIC) {
			puts("\nOK magic ");
			dumpv("SHM0", magic);
			w32(A9_STATUS, OK_DONE);		// publish success for memory inspection
			dsp_stop();
			return 0;
		}

		if (irq) {
			puts("\nFAIL dsp fault IRQ=");
			hex32(irq);
			puts(" SHM0=");
			hex32(magic);
			puts("\n");
			dumpv("MMU_IRQ", r32(DSP_MMU_IRQSTATUS));
			dumpv("FAULT_AD", r32(DSP_MMU_FAULT_AD));
			dumpv("FAULT_PC", r32(DSP_MMU_FAULT_PC));
			dumpv("FAULT_ST", r32(DSP_MMU_FAULT_STATUS));
			dsp_stop();
			return -1;
		}
	}

	puts("\nFAIL magic timeout SHM0=");
	hex32(r32(ARM_SHM));
	puts(" IRQ=");
	hex32(r32(DSP_MMU_IRQSTATUS));
	puts("\n");
	w32(A9_STATUS, ERR_TIMEOUT);
	dsp_stop();
	return -1;
}

int main(void)
{
	step(STEP_MAIN);
	puts("DSP magic writer\n");

	load_payload();

	if (dsp_power_clock())
		halt();
	if (dsp_reset_for_mmu())
		halt();
	if (dsp_mmu())
		halt();
	if (dsp_boot())
		halt();

	puts("DONE\n");
	halt();
	return 0;
}

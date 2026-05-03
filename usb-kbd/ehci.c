#include "usb.h"
#include "uart.h"

uint32_t opbase;
unsigned ehci_nports = 1;
unsigned root_port = 0;

/*
 * Return the EHCI PORTSC register address for a root port index.
 * EHCI root-port registers are laid out as consecutive 32-bit registers.
 */
static uint32_t port_addr(unsigned port)
{
	return opbase + PORTSC1 + port * 4u;
}

void dump_port(const char *s, unsigned port)
{
	puts(s);
	putc('1' + port);
	puts("=");
	hex32(r32(port_addr(port)));
	puts("\n");
}

/*
 * Wait for the OMAP EHCI ULPI viewport to become idle.
 * The controller clears ULPI_CTRL when the read/write transaction finishes.
 */
static int ulpi_wait_ctrl(void)
{
	for (uint32_t i = 0; i < WAIT_ULPI_LOOPS; i++) {
		if (!(r32(EHCI_BASE + INSNREG05) & ULPI_CTRL))	// wait for ULPI viewport idle
			return 0;
		delay(DELAY_ULPI_POLL);
	}

	return -1;
}

/*
 * Submit one raw ULPI viewport command through the OMAP EHCI wrapper.
 * The caller supplies a readable name for timeout debug output.
 */
static int ulpi_request(uint32_t v, const char *what)
{
	w32(EHCI_BASE + INSNREG05, v);		// issue OMAP ULPI viewport request
	if (ulpi_wait_ctrl()) {
		puts(what);
		puts(" timeout\n");
		return -1;
	}

	return 0;
}

/*
 * Write one ULPI PHY register through the OMAP EHCI viewport.
 * The port number is encoded the way the OMAP wrapper expects it.
 */
static int ulpi_write_omap(unsigned port, uint8_t reg, uint8_t val)
{
	uint32_t req = ULPI_CTRL | (((port + 1u) & 0xfu) << 24) |
		ULPI_OP_WRITE | ((uint32_t)reg << 16) | val;

	return ulpi_request(req, "ULPI write");
}

/*
 * Read one ULPI PHY register through the OMAP EHCI viewport.
 * Returns ULPI_ERROR if the viewport transaction times out.
 */
static uint32_t ulpi_read_omap(unsigned port, uint8_t reg)
{
	uint32_t req = ULPI_CTRL | (((port + 1u) & 0xfu) << 24) | ULPI_OP_READ |
		((uint32_t)reg << 16);

	if (ulpi_request(req, "ULPI read"))
		return ULPI_ERROR;
	return r32(EHCI_BASE + INSNREG05) & 0xff;	// fetch ULPI read data
}

/*
 * Read and print basic ULPI PHY identification/control registers.
 * This is bring-up debug so we can see whether the PHY answers.
 */
static int ulpi_dump(unsigned port)
{
	uint32_t id0 = ulpi_read_omap(port, 0x00);	// read ULPI vendor ID low
	uint32_t id1 = ulpi_read_omap(port, 0x01);	// read ULPI vendor ID high
	uint32_t id2 = ulpi_read_omap(port, 0x02);	// read ULPI product ID low
	uint32_t id3 = ulpi_read_omap(port, 0x03);	// read ULPI product ID high
	uint32_t fc = ulpi_read_omap(port, 0x04);	// read ULPI function control
	uint32_t otg = ulpi_read_omap(port, 0x0A);	// read ULPI OTG control

	if ((id0 | id1 | id2 | id3 | fc | otg) & ULPI_ERROR)
		return fail("ULPI read fail\n");

	dumpv("ULPI_ID ", id0 | (id1 << 8) | (id2 << 16) | (id3 << 24));
	dumpv("ULPI_FC ", fc);
	dumpv("ULPI_OTG", otg);
	return 0;
}

/*
 * Ask the external ULPI PHY to reset and wait for it to clear the reset bit.
 * The function logs PHY state but keeps going if the soft reset is awkward.
 */
static int omap_ehci_soft_phy_reset(unsigned port)
{
	ulpi_dump(port);

	if (ulpi_write_omap(port, 0x05, 0x20)) {	// set ULPI interface reset bit
		puts("ULPI reset timeout\n");
		return 0;
	}

	for (int i = 0; i < ULPI_RESET_POLLS; i++) {
		uint32_t fc = ulpi_read_omap(port, 0x04);	// poll function-control reset bit
		if (fc != ULPI_ERROR && !(fc & 0x20))
			break;
		delay(DELAY_ULPI_RESET_POLL);
	}

	delay(DELAY_SHORT);
	return 0;
}

/*
 * Initialize the EHCI controller after board clocks and pinmux are ready.
 * Resets EHCI, clears schedules, enables the ULPI path, starts the controller, and powers root ports.
 */
int ehci_init(void)
{
	puts("ehci\n");
	dump("CAP     ", EHCI_BASE);
	dump("HCSPARAM", EHCI_BASE + HCS_PARAMS);

	opbase = EHCI_BASE + (r32(EHCI_BASE) & 0xff);	// find operational register base
	ehci_nports = r32(EHCI_BASE + HCS_PARAMS) & 0xf;	// read root port count
	if (ehci_nports == 0 || ehci_nports > 3)
		ehci_nports = 3;

	w32(opbase + USB_CMD, USB_CMD_RESET);		// reset EHCI controller
	delay(DELAY_CLOCK_STABLE);
	if (wait_clr(opbase + USB_CMD, USB_CMD_RESET))
		return -1;

	w32(opbase + USB_INTR, 0);			// mask EHCI interrupts
	w32(opbase + USB_STS, USB_STS_ALL);		// clear stale EHCI status
	w32(EHCI_BASE + INSNREG04, 0x20);		// enable OMAP EHCI ULPI path
	if (omap_ehci_soft_phy_reset(0))
		return -1;

	w32(opbase + PERIODIC_ADDR, 0);			// clear periodic list base
	w32(opbase + ASYNC_ADDR, 0);			// clear async list base

	w32(opbase + USB_CMD, USB_CMD_RUN);		// start EHCI controller
	delay(DELAY_EHCI_SETTLE);

	w32(opbase + CONFIG_FLAG, 1);			// route root ports to EHCI
	delay(DELAY_EHCI_SETTLE);

	for (unsigned p = 0; p < ehci_nports; p++) {
		uint32_t v = r32(port_addr(p));		// read root port status/control
		w32(port_addr(p), (v & ~PORT_CHANGES) | PORT_POWER);	// power root port
	}
	delay(DELAY_EHCI_SETTLE);

	return 0;
}

/*
 * Clear sticky change bits on one EHCI root port.
 * Keeps port power enabled while acknowledging connect/enable/reset changes.
 */
static void port_clear_changes(unsigned port)
{
	uint32_t v = r32(port_addr(port));		// read root port status/control
	v &= ~PORT_RESET;
	v |= PORT_POWER | PORT_CHANGES;
	w32(port_addr(port), v);			// clear root port change bits
	delay(DELAY_SHORT);
}

/*
 * Try to reset one connected EHCI root port.
 * Returns success only when the port comes back enabled after reset.
 */
static int root_port_reset_one(unsigned port)
{
	for (int i = 0; i < ROOT_PORT_TRIES; i++) {
		uint32_t p = r32(port_addr(port));	// read root port connect state

		if (!(p & PORT_CONNECT)) {
			delay(DELAY_ROOT_RETRY);
			continue;
		}

		port_clear_changes(port);
		delay(DELAY_ROOT_DEBOUNCE);

		w32(port_addr(port), PORT_POWER | PORT_RESET);	// assert root port reset
		delay(DELAY_ROOT_RESET);

		w32(port_addr(port), PORT_POWER);	// release root port reset

		for (int j = 0; j < ROOT_RESET_POLLS; j++) {
			delay(DELAY_SHORT);
			if (!(r32(port_addr(port)) & PORT_RESET))	// wait for reset bit to clear
				break;
		}

		delay(DELAY_CLOCK_STABLE);

		if (r32(port_addr(port)) & PORT_ENABLE)	// read final root port enable state
			return 0;
	}

	return -1;
}

/*
 * Search all EHCI root ports for the one that has the onboard hub attached.
 * Stores the successful port index in root_port.
 */
int root_port_reset(void)
{
	for (unsigned p = 0; p < ehci_nports; p++) {
		if (!root_port_reset_one(p)) {
			root_port = p;
			return 0;
		}
	}

	return -1;
}

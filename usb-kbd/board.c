#include "board.h"
#include "usb.h"
#include "gpio.h"
#include "uart.h"

#define ALTCLKSRC_SCRM 0x4A30A110u		// aux clock source
#define AUXCLK1_SCRM 0x4A30A314u		// AUXCLK1 control
#define AUXCLK3_SCRM 0x4A30A31Cu		// AUXCLK3 control

#define CTRL_CORE 0x4A100000u			// core padmux base
#define CTRL_WKUP 0x4A31E000u			// wakeup padmux base

/*
 * Configure the OMAP4 pad multiplexing needed for USB host mode.
 * This selects hub GPIOs, auxiliary clock pads, and the USB1 ULPI pins.
 */
static void padmux(void)
{
	w16(CTRL_CORE + 0x08C, 0x000B);		// mux hub power GPIO pad
	w16(CTRL_CORE + 0x186, 0x0003);		// mux hub reset GPIO pad
	w16(CTRL_CORE + 0x19A, 0x0000);		// mux AUXCLK1 output pad
	w16(CTRL_CORE + 0x19C, 0x011B);		// mux AUXCLK3 request/pad
	w16(CTRL_WKUP + 0x058, 0x0000);		// mux wakeup clock pad

	w16(CTRL_CORE + 0x0C2, 0x010C);		// mux USB1 ULPI CLK pad
	w16(CTRL_CORE + 0x0C4, 0x0004);		// mux USB1 ULPI STP pad
	w16(CTRL_CORE + 0x0C6, 0x010C);		// mux USB1 ULPI DIR pad
	w16(CTRL_CORE + 0x0C8, 0x010C);		// mux USB1 ULPI NXT pad
	w16(CTRL_CORE + 0x0CA, 0x010C);		// mux USB1 ULPI DATA0 pad
	w16(CTRL_CORE + 0x0CC, 0x010C);		// mux USB1 ULPI DATA1 pad
	w16(CTRL_CORE + 0x0CE, 0x010C);		// mux USB1 ULPI DATA2 pad
	w16(CTRL_CORE + 0x0D0, 0x010C);		// mux USB1 ULPI DATA3 pad
	w16(CTRL_CORE + 0x0D2, 0x010C);		// mux USB1 ULPI DATA4 pad
	w16(CTRL_CORE + 0x0D4, 0x010C);		// mux USB1 ULPI DATA5 pad
	w16(CTRL_CORE + 0x0D6, 0x010C);		// mux USB1 ULPI DATA6 pad
	w16(CTRL_CORE + 0x0D8, 0x010C);		// mux USB1 ULPI DATA7 pad
}

/*
 * Bring up the board-side USB hardware before touching EHCI.
 * Enables clocks, powers/resets the LAN9514 hub, resets UHH/TLL, and selects ULPI mode.
 */
void board_usb_init(void)
{
	puts("padmux\n");
	padmux();

	puts("clocks\n");
	w32(CM_WKUP_GPIO1_CLKCTRL, 0x2);	// enable GPIO1 interface clock
	w32(CM_L4PER_GPIO2_CLKCTRL, 0x2);	// enable GPIO2 interface clock
	w32(CM_L4PER_GPIO6_CLKCTRL, 0x2);	// enable GPIO6 interface clock
	w32(ALTCLKSRC_SCRM, (r32(ALTCLKSRC_SCRM) & ~3u) | 0xDu);	// select aux clock sources
	w32(AUXCLK1_SCRM, 0x000F0104);		// set AUXCLK1 source/divider/enable
	w32(AUXCLK3_SCRM, 0x00010100);		// set AUXCLK3 source/divider/enable
	w32(CM_L3INIT_HSUSBHOST_CLKCTRL, 0x01000F02);	// enable USB host clocks
	w32(CM_L3INIT_HSUSBTLL_CLKCTRL, 0x2);	// enable USB TLL module clock
	w32(CM_L3INIT_FSUSB_CLKCTRL, 0x2);	// enable full-speed USB clock
	w32(CM_L3INIT_USBPHY_CLKCTRL, 0x00000301);	// enable USB PHY clocks
	delay(DELAY_CLOCK_STABLE);

	dump("CM_HOST ", CM_L3INIT_HSUSBHOST_CLKCTRL);
	dump("CM_TLL  ", CM_L3INIT_HSUSBTLL_CLKCTRL);

	puts("hub power/reset\n");
	gpio_out(GPIO1, 1, 0);			// drop LAN9514 hub power-enable GPIO
	gpio_out(GPIO2, 30, 0);			// assert LAN9514 hub reset GPIO
	delay(DELAY_HUB_POWER);

	gpio_out(GPIO1, 1, 1);			// enable LAN9514 hub power
	delay(DELAY_HUB_POWER);

	gpio_out(GPIO2, 30, 1);			// release LAN9514 hub reset
	delay(DELAY_HUB_RESET_RELEASE);

	puts("uhh/tll\n");
	w32(UHH + UHH_SYSCONFIG, 0x1);		// soft-reset USB host wrapper
	delay(DELAY_SHORT);
	if (wait_clr(UHH + UHH_SYSCONFIG, 0x1))
		puts("UHH reset timeout\n");

	w32(USBTLL + USBTLL_SYSCONFIG, 0x2);	// soft-reset USB TLL
	if (wait_set(USBTLL + USBTLL_SYSSTATUS, 0x1))
		puts("TLL reset timeout\n");
	w32(USBTLL + USBTLL_SYSCONFIG, 0x10C);	// set TLL idle/wakeup behavior

	w32(UHH + UHH_SYSCONFIG, 0x14);		// set UHH idle/wakeup behavior
	w32(UHH + UHH_HOSTCONFIG, 0x8000001C);	// configure host ports for ULPI PHY
	delay(DELAY_CLOCK_STABLE);

	dump("UHH_HOST", UHH + UHH_HOSTCONFIG);
}

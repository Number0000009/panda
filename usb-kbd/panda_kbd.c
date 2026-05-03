#include "board.h"
#include "hub.h"
#include "keyboard.h"
#include "uart.h"

/*
 * Hardware chain:
 * OMAP4430 EHCI host -> USB1 ULPI PHY -> onboard LAN9514 USB hub
 * -> downstream hub port -> USB keyboard.
 *
 * USB keyboard bring-up path:
 * 1. Configure PandaBoard clocks, padmux, hub power/reset, and USB wrappers.
 * 2. Initialize EHCI, find/reset the root port with the onboard LAN9514 hub.
 * 3. Enumerate the hub, scan its downstream ports, and configure the boot keyboard.
 * 4. Poll the keyboard interrupt endpoint forever and print new key presses.
 */
void main(void)
{
	puts("USB keyboard\n");

	board_usb_init();

	if (ehci_init()) {
		puts("FAIL ehci\n");
		halt();
	}

	if (root_port_reset()) {
		puts("FAIL root port\n");
		for (unsigned p = 0; p < ehci_nports; p++)
			dump_port("PORT ", p);
		halt();
	}

	dump_port("ROOTPORT", root_port);
	delay(DELAY_ROOT_SETTLE);

	if (enum_hub()) {
		puts("FAIL hub enum\n");
		dump_port("PORT ", root_port);
		halt();
	}

	int kbd_port = find_keyboard_port();

	if (kbd_port < 0) {
		puts("FAIL no keyboard port\n");
		halt();
	}

	puts("kbd port ");
	putc('0' + kbd_port);
	puts("\n");

	puts("READY\n");
	for (;;) {
		poll_keyboard();
		delay(DELAY_KEY_POLL);
	}
}

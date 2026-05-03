#include "keyboard.h"
#include "hub.h"
#include "uart.h"

static uint8_t kb_iface = 0;
static uint8_t kb_mps = 8;
static uint8_t kb_addr = 2;
static uint8_t kb_speed = 0;
static uint8_t kb_hub = 1;
static uint8_t kb_hubport = 0;
static uint8_t kb_ep = 1;
static uint8_t kb_ep_mps = 8;
static uint8_t kb_toggle = 0;
static int kb_port = -1;

static volatile uint8_t led_report[1] __attribute__((aligned(64)));

/*
 * Check whether a descriptor at buf[off] is a HID boot keyboard interface.
 * Boot keyboards use class HID, subclass boot, protocol keyboard.
 */
static int is_boot_keyboard_iface(uint16_t off)
{
	return buf[off + 5] == USB_CLASS_HID &&
		buf[off + 6] == USB_SUBCLASS_BOOT &&
		buf[off + 7] == USB_PROTOCOL_KEYBOARD;
}

/*
 * Check whether a descriptor at buf[off] is an interrupt IN endpoint.
 * This is the endpoint we poll for HID boot keyboard reports.
 */
static int is_interrupt_in_endpoint(uint16_t off)
{
	return (buf[off + 2] & USB_ENDPOINT_DIR_IN) &&
		(buf[off + 3] & 3) == USB_ENDPOINT_XFER_INT;
}

/*
 * Try to enumerate a keyboard on one hub downstream port.
 * Sets address/configuration, finds the boot keyboard interface and interrupt endpoint, then enables Num Lock.
 */
static int enum_keyboard(const struct hub_child *dev)
{
	uint8_t ep0_mps = dev->speed == USB_SPEED_HIGH ? 64 : 8;

	puts("device\n");
	if (get_desc(0, dev->speed, ep0_mps, dev->hub, dev->hubport,
			usb_desc(USB_DT_DEVICE, 0), buf, 8))
		return fail("dev8 fail\n");

	if (buf[0] < 8 || buf[1] != USB_DT_DEVICE)
		return fail("bad dev8\n");

	kb_mps = buf[7];
	if (kb_mps == 0 || kb_mps > 64)
		kb_mps = 8;

	dumpv("kbd mps", kb_mps);

	if (set_addr(0, dev->speed, kb_mps, dev->hub, dev->hubport, dev->addr))
		return fail("addr fail\n");

	if (get_desc(dev->addr, dev->speed, kb_mps, dev->hub, dev->hubport,
			usb_desc(USB_DT_DEVICE, 0), buf, 18))
		return fail("dev18 fail\n");

	if (buf[0] != 18 || buf[1] != USB_DT_DEVICE)
		return fail("bad dev18\n");

	dumpv("kbd VIDPID", buf32(8));

	if (get_desc(dev->addr, dev->speed, kb_mps, dev->hub, dev->hubport,
			usb_desc(USB_DT_CONFIG, 0), buf, 255))
		return fail("cfg fail\n");

	if (buf[0] != 9 || buf[1] != USB_DT_CONFIG)
		return fail("bad cfg\n");

	uint8_t cfg = buf[5];
	uint16_t total = buf16(2);
	if (total > sizeof(buf))
		total = sizeof(buf);

	kb_iface = 0xff;
	kb_ep = 0;
	kb_ep_mps = 8;
	uint8_t in_kb_iface = 0;

	for (uint16_t i = 0; i + 2 <= total;) {
		uint8_t l = buf[i];
		uint8_t t = buf[i + 1];

		if (!l)
			break;
		if (i + l > total)
			break;

		if (t == USB_DT_INTERFACE && l >= 9) {
			if (is_boot_keyboard_iface(i)) {
				kb_iface = buf[i + 2];
				in_kb_iface = 1;
			} else {
				in_kb_iface = 0;
			}
		} else if (t == USB_DT_ENDPOINT && in_kb_iface && l >= 7) {
			uint8_t ea = buf[i + 2];
			uint16_t emps = buf16(i + 4);

			if (is_interrupt_in_endpoint(i)) {
				kb_ep = ea & 0xf;
				kb_ep_mps = emps > 8 ? 8 : (uint8_t)emps;
				if (kb_ep_mps == 0)
					kb_ep_mps = 8;
				break;
			}
		}

		i += l;
	}

	if (kb_iface == 0xff || kb_ep == 0)
		return fail("no boot keyboard iface\n");

	dumpv("kbd iface", kb_iface);
	dumpv("kbd ep", kb_ep);

	if (set_config(dev->addr, dev->speed, kb_mps, dev->hub, dev->hubport,
			cfg))
		return fail("set cfg fail\n");

	if (control(dev->addr, dev->speed, kb_mps, dev->hub, dev->hubport,
			USB_RT_HID_OUT, HID_REQ_SET_PROTOCOL, HID_BOOT_PROTOCOL,
			kb_iface, buf, 0))
		puts("kbd boot protocol fail\n");

	if (control(dev->addr, dev->speed, kb_mps, dev->hub, dev->hubport,
			USB_RT_HID_OUT, HID_REQ_SET_IDLE, 0, kb_iface, buf, 0))
		puts("kbd idle fail\n");

	led_report[0] = HID_LED_NUM_LOCK;
	if (control(dev->addr, dev->speed, kb_mps, dev->hub, dev->hubport,
			USB_RT_HID_OUT, HID_REQ_SET_REPORT,
			HID_REPORT_TYPE_OUTPUT << 8, kb_iface, led_report, 1))
		puts("kbd numlock fail\n");

	kb_addr = dev->addr;
	kb_speed = dev->speed;
	kb_hub = dev->hub;
	kb_hubport = dev->hubport;
	kb_port = dev->port;
	kb_toggle = 0;
	return 0;
}

/*
 * Power and scan all LAN9514 downstream ports looking for a boot keyboard.
 * Non-keyboard devices, such as the LAN9514 Ethernet function, are skipped.
 */
int find_keyboard_port(void)
{
	return hub_scan_ports(enum_keyboard);
}

// HID boot keyboard input
static const char keymap[128] = {
	0,	0,	0,	0,	'a',	'b',	'c',	'd',	// 0x00-0x07
	'e',	'f',	'g',	'h',	'i',	'j',	'k',	'l',	// 0x08-0x0f
	'm',	'n',	'o',	'p',	'q',	'r',	's',	't',	// 0x10-0x17
	'u',	'v',	'w',	'x',	'y',	'z',	'1',	'2',	// 0x18-0x1f
	'3',	'4',	'5',	'6',	'7',	'8',	'9',	'0',	// 0x20-0x27
	'\n',	0,	'\b',	'\t',	' ',	'-',	'=',	'[',	// 0x28-0x2f
	']',	'\\',	0,	';',	'\'',	'`',	',',	'.',	// 0x30-0x37
	'/',	0,	0,	0,	0,	0,	0,	0,	// 0x38-0x3f
};

static const char keymap_shift[128] = {
	0,	0,	0,	0,	'A',	'B',	'C',	'D',	// 0x00-0x07
	'E',	'F',	'G',	'H',	'I',	'J',	'K',	'L',	// 0x08-0x0f
	'M',	'N',	'O',	'P',	'Q',	'R',	'S',	'T',	// 0x10-0x17
	'U',	'V',	'W',	'X',	'Y',	'Z',	'!',	'@',	// 0x18-0x1f
	'#',	'$',	'%',	'^',	'&',	'*',	'(',	')',	// 0x20-0x27
	'\n',	0,	'\b',	'\t',	' ',	'_',	'+',	'{',	// 0x28-0x2f
	'}',	'|',	0,	':',	'"',	'~',	'<',	'>',	// 0x30-0x37
	'?',	0,	0,	0,	0,	0,	0,	0,	// 0x38-0x3f
};

static uint8_t old_keys[6];

/*
 * Test whether a key code was present in the previous HID report.
 * This suppresses repeated prints while a key is held down.
 */
static int old_has(uint8_t k)
{
	for (int i = 0; i < 6; i++)
		if (old_keys[i] == k)
			return 1;
	return 0;
}

/*
 * Poll the keyboard once and print newly pressed printable keys.
 * Uses the boot keyboard 8-byte report format: modifiers in byte 0, key codes in bytes 2..7.
 */
void poll_keyboard(void)
{
	memzero(buf, 8);

	if (interrupt_in(kb_addr, kb_ep, kb_speed, kb_ep_mps, kb_hub,
			kb_hubport, &kb_toggle, buf, 8))
		return;

	uint8_t mod = buf[0];
	int shift = (mod & 0x22) != 0;

	for (int i = 2; i < 8; i++) {
		uint8_t k = buf[i];

		if (k && k < sizeof(keymap) && !old_has(k)) {
			char c = shift ? keymap_shift[k] : keymap[k];
			if (c)
				putc(c);
		}
	}

	for (int i = 0; i < 6; i++)
		old_keys[i] = buf[i + 2];
}

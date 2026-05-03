#include "hub.h"
#include "uart.h"

/*
 * Read a downstream hub port status word.
 * The four returned bytes include current status and change bits.
 */
static int hub_get_port(uint8_t hubaddr, uint8_t port, uint32_t *st)
{
	int r = control(hubaddr, USB_SPEED_HIGH, 64, 0, 0, USB_RT_PORT_IN,
		USB_REQ_GET_STATUS, 0, port, buf, 4);

	*st = buf32(0);
	return r;
}

/*
 * Send SET_FEATURE to one downstream hub port.
 * Used here for port power and port reset requests.
 */
static int hub_set_feature(uint8_t hubaddr, uint8_t port, uint16_t feat)
{
	return control(hubaddr, USB_SPEED_HIGH, 64, 0, 0, USB_RT_PORT_OUT,
		USB_REQ_SET_FEATURE, feat, port, buf, 0);
}

/*
 * Send CLEAR_FEATURE to one downstream hub port.
 * Used to acknowledge hub port change bits after reset/connect events.
 */
static int hub_clr_feature(uint8_t hubaddr, uint8_t port, uint16_t feat)
{
	return control(hubaddr, USB_SPEED_HIGH, 64, 0, 0, USB_RT_PORT_OUT,
		USB_REQ_CLEAR_FEATURE, feat, port, buf, 0);
}

static void hub_port_log(const char *s, int port, uint32_t st)
{
	puts(s);
	putc('0' + port);
	puts("=");
	hex32(st);
	puts("\n");
}

/*
 * Decode a hub port status word into the EHCI speed encoding.
 * The hub reports low/high-speed flags; no flag means full speed.
 */
static uint8_t hub_port_speed(uint32_t st)
{
	if (st & USB_PORT_STAT_HIGH_SPEED)
		return USB_SPEED_HIGH;
	if (st & USB_PORT_STAT_LOW_SPEED)
		return USB_SPEED_LOW;
	return USB_SPEED_FULL;
}

/*
 * Fill the EHCI transaction-translator fields for a child device.
 * Full/low-speed devices behind the high-speed hub need TT info; high-speed devices do not.
 */
static void hub_child_init(struct hub_child *dev, uint8_t port, uint8_t speed)
{
	dev->port = port;
	dev->speed = speed;
	dev->addr = 2 + port;
	dev->hub = speed == USB_SPEED_HIGH ? 0 : HUB_ADDR;
	dev->hubport = speed == USB_SPEED_HIGH ? 0 : port;
}

/*
 * Enumerate the onboard LAN9514 hub on the EHCI root port.
 * Reads descriptors, assigns address 1, selects its configuration, and leaves its downstream ports ready.
 */
int enum_hub(void)
{
	puts("hub\n");
	if (get_desc(0, USB_SPEED_HIGH, 64, 0, 0, usb_desc(USB_DT_DEVICE, 0),
			buf, 18))
		return fail("hub dev fail\n");

	if (buf[0] != 18 || buf[1] != USB_DT_DEVICE)
		return fail("bad hub dev desc\n");

	dumpv("hub VIDPID", buf32(8));

	if (set_addr(0, USB_SPEED_HIGH, 64, 0, 0, HUB_ADDR))
		return fail("hub addr fail\n");

	if (get_desc(HUB_ADDR, USB_SPEED_HIGH, 64, 0, 0,
			usb_desc(USB_DT_CONFIG, 0), buf, 64))
		return fail("hub cfg fail\n");

	if (buf[0] != 9 || buf[1] != USB_DT_CONFIG)
		return fail("bad hub cfg desc\n");

	dumpv("hub cfg", buf[5]);

	if (set_config(HUB_ADDR, USB_SPEED_HIGH, 64, 0, 0, buf[5]))
		return fail("hub set cfg fail\n");

	delay(DELAY_HUB_CONFIG);
	return 0;
}

/*
 * Power and scan all LAN9514 downstream ports.
 * Calls probe for each connected and reset child device until one probe accepts it.
 */
int hub_scan_ports(hub_probe_t probe)
{
	for (uint8_t p = 1; p <= HUB_PORTS; p++) {
		hub_set_feature(HUB_ADDR, p, USB_PORT_FEAT_POWER);
		delay(DELAY_HUB_PORT);

		uint32_t st = 0;
		if (hub_get_port(HUB_ADDR, p, &st)) {
			puts("hub get port fail\n");
			continue;
		}

		hub_port_log("port", p, st);

		if (!(st & USB_PORT_STAT_CONNECTION))
			continue;

		hub_set_feature(HUB_ADDR, p, USB_PORT_FEAT_RESET);
		delay(DELAY_HUB_PORT);

		for (int i = 0; i < HUB_RESET_POLLS; i++) {
			hub_get_port(HUB_ADDR, p, &st);
			if (!(st & USB_PORT_STAT_RESET))
				break;
			delay(DELAY_EHCI_SETTLE);
		}

		hub_clr_feature(HUB_ADDR, p, USB_PORT_FEAT_C_CONNECTION);
		hub_clr_feature(HUB_ADDR, p, USB_PORT_FEAT_C_ENABLE);
		hub_clr_feature(HUB_ADDR, p, USB_PORT_FEAT_C_RESET);

		hub_get_port(HUB_ADDR, p, &st);
		hub_port_log("reset", p, st);

		if (!(st & USB_PORT_STAT_ENABLE))
			continue;

		struct hub_child dev;

		hub_child_init(&dev, p, hub_port_speed(st));
		puts("speed=");
		putc('0' + dev.speed);
		puts("\n");

		if (!probe(&dev))
			return p;

		puts("skip port ");
		putc('0' + p);
		puts("\n");
	}

	return -1;
}

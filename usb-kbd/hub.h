#pragma once

#include "usb.h"

#define HUB_ADDR 1				// LAN9514 USB device address
#define HUB_PORTS 4				// LAN9514 downstream ports
#define HUB_RESET_POLLS 40			// hub reset poll count

#define USB_PORT_FEAT_RESET 4			// SET_FEATURE reset selector
#define USB_PORT_FEAT_POWER 8			// SET_FEATURE power selector
#define USB_PORT_FEAT_C_CONNECTION 16		// CLEAR_FEATURE connect-change
#define USB_PORT_FEAT_C_ENABLE 17		// CLEAR_FEATURE enable-change
#define USB_PORT_FEAT_C_RESET 20		// CLEAR_FEATURE reset-change
#define USB_PORT_STAT_CONNECTION BIT(0)		// GET_STATUS connected bit
#define USB_PORT_STAT_ENABLE BIT(1)		// GET_STATUS enabled bit
#define USB_PORT_STAT_RESET BIT(4)		// GET_STATUS reset bit
#define USB_PORT_STAT_LOW_SPEED BIT(9)		// GET_STATUS low-speed bit
#define USB_PORT_STAT_HIGH_SPEED BIT(10)	// GET_STATUS high-speed bit

#define USB_RT_PORT_IN 0xA3			// hub port class IN request
#define USB_RT_PORT_OUT 0x23			// hub port class OUT request

/*
 * Device found on one downstream hub port.
 * port is the physical hub port, addr is the USB address to assign, and
 * hub/hubport are EHCI transaction-translator fields for full/low-speed devices.
 */
struct hub_child {
	uint8_t port;				// LAN9514 downstream port
	uint8_t speed;				// EHCI USB_SPEED_* value
	uint8_t addr;				// address to give the child
	uint8_t hub;				// TT hub address, zero for high-speed
	uint8_t hubport;			// TT port number, zero for high-speed
};

typedef int (*hub_probe_t)(const struct hub_child *dev);

int enum_hub(void);
int hub_scan_ports(hub_probe_t probe);

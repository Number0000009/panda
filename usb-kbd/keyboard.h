#pragma once

#include "usb.h"

#define DELAY_KEY_POLL 3000000u			// keyboard poll delay

#define USB_CLASS_HID 3				// HID interface class
#define USB_SUBCLASS_BOOT 1			// HID boot subclass
#define USB_PROTOCOL_KEYBOARD 1			// HID keyboard protocol

#define HID_REQ_SET_REPORT 9			// HID SET_REPORT request
#define HID_REQ_SET_IDLE 10			// HID SET_IDLE request
#define HID_REQ_SET_PROTOCOL 11			// HID SET_PROTOCOL request
#define HID_REPORT_TYPE_OUTPUT 2		// HID output report type
#define HID_BOOT_PROTOCOL 0			// HID boot protocol
#define HID_LED_NUM_LOCK BIT(0)			// Num Lock LED bit

#define USB_RT_HID_OUT 0x21			// HID iface class OUT request

int find_keyboard_port(void);
void poll_keyboard(void);

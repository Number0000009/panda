#pragma once

#include "helpers.h"

#define WAIT_ULPI_LOOPS 1000000u		// ULPI viewport wait loop
#define WAIT_SCHEDULE_LOOPS 10000000u		// EHCI schedule state wait loop
#define WAIT_TRANSFER_LOOPS 50000000u		// control transfer wait loop
#define WAIT_INTR_LOOPS 1000000u		// interrupt transfer wait loop
#define ULPI_RESET_POLLS 1000			// ULPI reset poll count
#define ROOT_PORT_TRIES 20			// root reset retry count
#define ROOT_RESET_POLLS 100			// root reset poll count

#define DELAY_SHORT 1000000u			// short USB settle delay
#define DELAY_ULPI_POLL 10u			// tiny ULPI poll pause
#define DELAY_ULPI_RESET_POLL 100000u		// ULPI reset poll delay
#define DELAY_ROOT_DEBOUNCE 10000000u		// root connect debounce
#define DELAY_CLOCK_STABLE 30000000u		// clock settle delay
#define DELAY_EHCI_SETTLE 50000000u		// EHCI settle delay
#define DELAY_ROOT_RESET 80000000u		// root reset hold delay
#define DELAY_ROOT_RETRY 100000000u		// root reset retry delay
#define DELAY_HUB_CONFIG 100000000u		// hub config settle delay
#define DELAY_ROOT_SETTLE 200000000u		// root port settle delay
#define DELAY_HUB_POWER 300000000u		// hub port power delay
#define DELAY_HUB_PORT 500000000u		// hub port debounce delay
#define DELAY_HUB_RESET_RELEASE 1500000000u	// hub reset release delay

#define CM_L3INIT_HSUSBHOST_CLKCTRL 0x4A009358u	// USB host clocks
#define CM_L3INIT_HSUSBTLL_CLKCTRL 0x4A009368u	// USB TLL clock
#define CM_L3INIT_FSUSB_CLKCTRL 0x4A0093D0u	// FSUSB clock
#define CM_L3INIT_USBPHY_CLKCTRL 0x4A0093E0u	// USB PHY clock

#define UHH 0x4A064000u				// USB host wrapper base
#define UHH_SYSCONFIG 0x10u			// wrapper sysconfig offset
#define UHH_HOSTCONFIG 0x40u			// wrapper hostconfig offset

#define USBTLL 0x4A062000u			// USB TLL register base
#define USBTLL_SYSCONFIG 0x10u			// TLL sysconfig offset
#define USBTLL_SYSSTATUS 0x14u			// TLL sysstatus offset

#define EHCI_BASE 0x4A064C00u			// EHCI capability base

#define HCS_PARAMS 0x04u			// EHCI capability params
#define USB_CMD 0x00u				// EHCI command offset
#define USB_STS 0x04u				// EHCI status offset
#define USB_INTR 0x08u				// EHCI interrupt offset
#define PERIODIC_ADDR 0x14u			// periodic list base
#define ASYNC_ADDR 0x18u			// async list base
#define CONFIG_FLAG 0x40u			// route ports to EHCI
#define PORTSC1 0x44u				// first port status
#define INSNREG04 0xA0u				// OMAP EHCI insn reg 4
#define INSNREG05 0xA4u				// OMAP ULPI viewport

#define USB_CMD_RUN BIT(0)			// run/stop controller
#define USB_CMD_RESET BIT(1)			// controller reset
#define USB_CMD_PERIODIC BIT(4)			// periodic schedule enable
#define USB_CMD_ASYNC BIT(5)			// async schedule enable
#define USB_STS_PERIODIC BIT(14)		// periodic schedule status
#define USB_STS_ASYNC BIT(15)			// async schedule status
#define USB_STS_ALL 0x3fu			// clearable status bits

#define ULPI_CTRL BIT(31)			// ULPI viewport busy bit
#define ULPI_OP_READ (3u << 22)			// ULPI viewport read op
#define ULPI_OP_WRITE (2u << 22)		// ULPI viewport write op
#define ULPI_ERROR 0x100u			// local ULPI error marker

#define PORT_CONNECT 0x00000001u		// port connected bit
#define PORT_CHANGES 0x0000002Au		// write-1 change bits
#define PORT_ENABLE 0x00000004u			// port enabled bit
#define PORT_RESET 0x00000100u			// port reset bit
#define PORT_POWER 0x00001000u			// port power bit

// USB protocol constants
#define USB_SPEED_FULL 0			// full-speed device
#define USB_SPEED_LOW 1				// low-speed device
#define USB_SPEED_HIGH 2			// high-speed device

#define USB_DIR_OUT 0x00			// setup request OUT
#define USB_DIR_IN 0x80				// setup request IN
#define USB_TYPE_CLASS 0x20			// class request type
#define USB_RECIP_INTERFACE 0x01		// interface recipient
#define USB_RECIP_PORT 0x03			// hub port recipient

#define USB_REQ_GET_STATUS 0			// standard GET_STATUS
#define USB_REQ_CLEAR_FEATURE 1			// standard CLEAR_FEATURE
#define USB_REQ_SET_FEATURE 3			// standard SET_FEATURE
#define USB_REQ_SET_ADDRESS 5			// standard SET_ADDRESS
#define USB_REQ_GET_DESCRIPTOR 6		// standard GET_DESCRIPTOR
#define USB_REQ_SET_CONFIGURATION 9		// standard SET_CONFIGURATION

#define USB_DT_DEVICE 1				// device descriptor type
#define USB_DT_CONFIG 2				// config descriptor type
#define USB_DT_INTERFACE 4			// interface descriptor type
#define USB_DT_ENDPOINT 5			// endpoint descriptor type

#define USB_ENDPOINT_XFER_INT 3			// interrupt endpoint type
#define USB_ENDPOINT_DIR_IN 0x80		// endpoint address IN bit

/*
 * Build the USB GET_DESCRIPTOR wValue field.
 * High byte is descriptor type, low byte is descriptor index.
 */
static inline uint16_t usb_desc(uint8_t type, uint8_t index)
{
	return ((uint16_t)type << 8) | index;
}

extern uint32_t opbase;
extern unsigned ehci_nports;
extern unsigned root_port;

extern volatile uint8_t buf[256];

uint16_t buf16(unsigned off);
uint32_t buf32(unsigned off);

int ehci_init(void);
int root_port_reset(void);
void dump_port(const char *s, unsigned port);

int interrupt_in(uint8_t addr, uint8_t ep, uint8_t speed, uint16_t mps,
	uint8_t hubaddr, uint8_t hubport, uint8_t *toggle, volatile void *data,
	uint32_t len);
int control(uint8_t addr, uint8_t speed, uint8_t mps, uint8_t hubaddr,
	uint8_t hubport, uint8_t bm, uint8_t req, uint16_t val, uint16_t idx,
	volatile void *data, uint16_t len);
int get_desc(uint8_t addr, uint8_t speed, uint8_t mps, uint8_t hub,
	uint8_t port, uint16_t typeidx, volatile void *dst, uint16_t len);
int set_addr(uint8_t oldaddr, uint8_t speed, uint8_t mps, uint8_t hub,
	uint8_t port, uint8_t newaddr);
int set_config(uint8_t addr, uint8_t speed, uint8_t mps, uint8_t hub,
	uint8_t port, uint8_t cfg);

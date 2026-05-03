#include "usb.h"

#define QTD_TERMINATE 1u			// terminate qTD link
#define QTD_ACTIVE BIT(7)			// qTD active bit
#define QTD_ERROR 0x7eu				// qTD error/status bits

#define QH_TERMINATE 1u				// terminate QH link
#define QH_TYPE_QH 2u				// queue-head link type
#define QH_DTC BIT(14)				// data toggle from qTD
#define QH_HEAD BIT(15)				// async head flag
#define QH_CONTROL_EP BIT(27)			// control endpoint flag
#define QH_RELOAD(n) ((uint32_t)(n) << 28)	// nak reload field
#define QH_SMASK(n) (n)				// split start mask
#define QH_CMASK(n) ((uint32_t)(n) << 8)	// split complete mask
#define QH_MULT_ONE BIT(30)			// one transaction per frame

#define PID_OUT 0				// qTD OUT token PID
#define PID_IN 1				// qTD IN token PID
#define PID_SETUP 2				// qTD SETUP token PID

/*
 * EHCI queue element transfer descriptor.
 * One qTD describes one transfer stage that the EHCI controller executes.
 */
struct qtd {
	uint32_t next;				// next qTD physical pointer
	uint32_t altnext;			// alternate qTD pointer on short/error
	uint32_t token;				// status, PID, length, toggle
	uint32_t buf[5];			// data buffer page pointers
};

/*
 * EHCI queue head.
 * A QH describes one endpoint and points the controller at its qTD list.
 */
struct qh {
	uint32_t horiz;				// next QH link in schedule
	uint32_t epchar;			// endpoint address, speed, MPS
	uint32_t epcap;				// hub/TT and split transaction info
	uint32_t current;			// current qTD pointer used by EHCI
	struct qtd overlay;			// live qTD state copied by EHCI
};

// Transfer state
static volatile struct qh qh __attribute__((aligned(64)));
static volatile struct qtd td[3] __attribute__((aligned(64)));
static volatile uint32_t periodic[1024] __attribute__((aligned(4096)));
static volatile uint8_t setup[8] __attribute__((aligned(64)));
volatile uint8_t buf[256] __attribute__((aligned(64)));

/*
 * Read a little-endian 16-bit value from the shared USB buffer.
 * USB descriptors and hub status fields are little-endian byte streams.
 */
uint16_t buf16(unsigned off)
{
	return buf[off] | (buf[off + 1] << 8);
}

/*
 * Read a little-endian 32-bit value from the shared USB buffer.
 * Used for VID/PID debug and hub port status words.
 */
uint32_t buf32(unsigned off)
{
	return buf16(off) | ((uint32_t)buf16(off + 2) << 16);
}

/*
 * Enable an EHCI schedule and wait until hardware reports it active.
 * Used for both async control transfers and periodic interrupt polling.
 */
static void ehci_start_schedule(uint32_t cmd, uint32_t sts)
{
	w32(opbase + USB_STS, USB_STS_ALL);		// clear old schedule status
	w32(opbase + USB_CMD, r32(opbase + USB_CMD) | cmd);	// enable requested schedule

	for (uint32_t i = 0; i < WAIT_SCHEDULE_LOOPS; i++)
		if (r32(opbase + USB_STS) & sts)	// wait until schedule is active
			break;
}

/*
 * Disable an EHCI schedule and wait until hardware reports it inactive.
 * This keeps each transfer self-contained and avoids leaving stale schedules running.
 */
static void ehci_stop_schedule(uint32_t cmd, uint32_t sts)
{
	w32(opbase + USB_CMD, r32(opbase + USB_CMD) & ~cmd);	// disable requested schedule

	for (uint32_t i = 0; i < WAIT_SCHEDULE_LOOPS; i++)
		if (!(r32(opbase + USB_STS) & sts))	// wait until schedule is inactive
			break;
}

/*
 * Fill one EHCI qTD for SETUP, IN, or OUT traffic.
 * Programs the token, toggle, transfer length, and page pointers for the data buffer.
 */
static void fill_td(volatile struct qtd *t, uint32_t next, uint32_t pid,
	volatile void *data, uint32_t len, uint32_t toggle)
{
	uint32_t a = (uint32_t)data;

	t->next = next;
	t->altnext = QTD_TERMINATE;
	t->token = (toggle << 31) | (len << 16) | (3 << 10) | (pid << 8) |
		QTD_ACTIVE;
	t->buf[0] = a;
	t->buf[1] = (a + 0x1000) & ~0xfffu;
	t->buf[2] = (a + 0x2000) & ~0xfffu;
	t->buf[3] = (a + 0x3000) & ~0xfffu;
	t->buf[4] = (a + 0x4000) & ~0xfffu;
}

/*
 * Submit a queue head on the EHCI async schedule and wait for its qTDs to finish.
 * The ctrl flag marks full/low-speed control endpoints that need split handling through the hub.
 */
static int submit_qh(uint8_t addr, uint8_t ep, uint8_t speed, uint16_t mps,
	uint8_t hubaddr, uint8_t hubport, volatile struct qtd *first, int ctrl)
{
	memzero(&qh, sizeof(qh));

	qh.horiz = ((uint32_t)&qh) | QH_TYPE_QH;
	qh.epchar = (uint32_t)addr | ((uint32_t)ep << 8) |
		((uint32_t)speed << 12) | QH_DTC | QH_HEAD |
		((uint32_t)mps << 16) | (ctrl ? QH_CONTROL_EP : 0) |
		QH_RELOAD(15);

	qh.epcap = ((uint32_t)hubaddr << 16) | ((uint32_t)hubport << 23) |
		QH_MULT_ONE;

	qh.current = 0;
	qh.overlay.next = (uint32_t)first;
	qh.overlay.altnext = QTD_TERMINATE;
	qh.overlay.token = 0;

	barrier();

	w32(opbase + ASYNC_ADDR, (uint32_t)&qh);		// publish async queue head
	barrier();

	ehci_start_schedule(USB_CMD_ASYNC, USB_STS_ASYNC);
	delay(DELAY_SHORT);

	for (uint32_t i = 0; i < WAIT_TRANSFER_LOOPS; i++) {
		barrier();
		uint32_t tok0 = td[0].token;
		uint32_t tok1 = td[1].token;
		uint32_t tok2 = td[2].token;

		if ((tok0 | tok1 | tok2) & QTD_ERROR) {
			ehci_stop_schedule(USB_CMD_ASYNC, USB_STS_ASYNC);
			return -1;
		}

		if (!((tok0 | tok1 | tok2) & QTD_ACTIVE)) {
			ehci_stop_schedule(USB_CMD_ASYNC, USB_STS_ASYNC);
			return 0;
		}
	}

	ehci_stop_schedule(USB_CMD_ASYNC, USB_STS_ASYNC);
	return -1;
}

/*
 * Poll one interrupt IN endpoint once.
 * Builds a one-entry periodic schedule and updates the caller's data toggle after a successful packet.
 */
int interrupt_in(uint8_t addr, uint8_t ep, uint8_t speed, uint16_t mps,
	uint8_t hubaddr, uint8_t hubport, uint8_t *toggle, volatile void *data,
	uint32_t len)
{
	memzero(&qh, sizeof(qh));
	memzero(td, sizeof(td));

	fill_td(&td[0], QTD_TERMINATE, PID_IN, data, len, *toggle);
	td[1].next = QTD_TERMINATE;
	td[1].altnext = QTD_TERMINATE;
	td[2].next = QTD_TERMINATE;
	td[2].altnext = QTD_TERMINATE;

	qh.horiz = QH_TERMINATE;
	qh.epchar = (uint32_t)addr | ((uint32_t)ep << 8) |
		((uint32_t)speed << 12) | QH_DTC |
		((uint32_t)mps << 16) | QH_RELOAD(15);

	qh.epcap = QH_SMASK(0x01u) | QH_CMASK(0x1cu) |
		((uint32_t)hubaddr << 16) | ((uint32_t)hubport << 23) |
		QH_MULT_ONE;

	qh.overlay.next = (uint32_t)&td[0];
	qh.overlay.altnext = QTD_TERMINATE;

	for (unsigned i = 0; i < 1024; i++)
		periodic[i] = ((uint32_t)&qh) | QH_TYPE_QH;

	barrier();

	w32(opbase + PERIODIC_ADDR, (uint32_t)periodic);	// publish interrupt schedule
	ehci_start_schedule(USB_CMD_PERIODIC, USB_STS_PERIODIC);

	int ret = 1;

	for (uint32_t i = 0; i < WAIT_INTR_LOOPS; i++) {
		barrier();
		uint32_t tok = td[0].token;

		if (tok & QTD_ERROR) {
			ret = -1;
			break;
		}

		if (!(tok & QTD_ACTIVE)) {
			*toggle ^= 1;
			ret = 0;
			break;
		}
	}

	ehci_stop_schedule(USB_CMD_PERIODIC, USB_STS_PERIODIC);
	return ret;
}

/*
 * Run one USB control transfer on endpoint zero.
 * Builds the SETUP, optional DATA, and STATUS stages, then submits them on the async schedule.
 */
int control(uint8_t addr, uint8_t speed, uint8_t mps, uint8_t hubaddr,
	uint8_t hubport, uint8_t bm, uint8_t req, uint16_t val, uint16_t idx,
	volatile void *data, uint16_t len)
{
	memzero(buf, sizeof(buf));
	memzero(td, sizeof(td));

	setup[0] = bm;
	setup[1] = req;
	setup[2] = val;
	setup[3] = val >> 8;
	setup[4] = idx;
	setup[5] = idx >> 8;
	setup[6] = len;
	setup[7] = len >> 8;

	if (len) {
		fill_td(&td[0], (uint32_t)&td[1], PID_SETUP, setup, 8, 0);
		fill_td(&td[1], (uint32_t)&td[2],
			(bm & USB_DIR_IN) ? PID_IN : PID_OUT, data, len, 1);
		fill_td(&td[2], QTD_TERMINATE,
			(bm & USB_DIR_IN) ? PID_OUT : PID_IN, buf, 0, 1);
	} else {
		fill_td(&td[0], (uint32_t)&td[1], PID_SETUP, setup, 8, 0);
		fill_td(&td[1], QTD_TERMINATE, PID_IN, buf, 0, 1);
		td[2].next = QTD_TERMINATE;
		td[2].altnext = QTD_TERMINATE;
		td[2].token = 0;
	}

	return submit_qh(addr, 0, speed, mps, hubaddr, hubport, &td[0],
		speed != USB_SPEED_HIGH);
}

/*
 * Fetch a USB descriptor with a standard GET_DESCRIPTOR request.
 * The descriptor type/index pair is already packed into typeidx.
 */
int get_desc(uint8_t addr, uint8_t speed, uint8_t mps, uint8_t hub,
	uint8_t port, uint16_t typeidx, volatile void *dst, uint16_t len)
{
	return control(addr, speed, mps, hub, port, USB_DIR_IN,
		USB_REQ_GET_DESCRIPTOR, typeidx, 0, dst, len);
}

/*
 * Assign a USB device address.
 * A short settle delay follows because devices switch to the new address after status stage.
 */
int set_addr(uint8_t oldaddr, uint8_t speed, uint8_t mps, uint8_t hub,
	uint8_t port, uint8_t newaddr)
{
	int r = control(oldaddr, speed, mps, hub, port, USB_DIR_OUT,
		USB_REQ_SET_ADDRESS, newaddr, 0, buf, 0);
	delay(DELAY_EHCI_SETTLE);
	return r;
}

/*
 * Select a USB configuration for an enumerated device.
 * The config value usually comes from byte 5 of the configuration descriptor.
 */
int set_config(uint8_t addr, uint8_t speed, uint8_t mps, uint8_t hub,
	uint8_t port, uint8_t cfg)
{
	return control(addr, speed, mps, hub, port, USB_DIR_OUT,
		USB_REQ_SET_CONFIGURATION, cfg, 0, buf, 0);
}

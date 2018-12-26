#include <cstdint>
#include <cstddef>

#include "cpu.h"
#include "mux.h"
#include "mux_data.h"
#include "uart.h"
#include "mmc.h"
#include "defs.h"	// misc shite

#include "omap4430.h"

//TODO: refactor all this
namespace lowlevel {

#define __arch_getb(a)		(*(volatile uint8_t *)(a))
#define __arch_getw(a)		(*(volatile uint16_t *)(a))
#define __arch_getl(a)		(*(volatile uint32_t *)(a))

#define __arch_putb(v,a)	(*(volatile uint8_t *)(a) = (v))
#define __arch_putw(v,a)	(*(volatile uint16_t *)(a) = (v))
#define __arch_putl(v,a)	(*(volatile uint32_t *)(a) = (v))

#define __raw_writeb(v,a)	__arch_putb(v,a)
#define __raw_writew(v,a)	__arch_putw(v,a)
#define __raw_writel(v,a)	__arch_putl(v,a)

#define __raw_readb(a)		__arch_getb(a)
#define __raw_readw(a)		__arch_getw(a)
#define __raw_readl(a)		__arch_getl(a)


/*****************************************************************
 * sr32 - clear & set a value in a bit range for a 32 bit address
 *****************************************************************/
void sr32(uint32_t addr, uint32_t start_bit, uint32_t num_bits, uint32_t value)
{
	uint32_t tmp, msk = 0;
	msk = 1 << num_bits;
	--msk;
	tmp = __raw_readl(addr) & ~(msk << start_bit);
	tmp |=  value << start_bit;
	__raw_writel(tmp, addr);
}

/*********************************************************************
 * wait_on_value() - common routine to allow waiting for changes in
 *   volatile regs.
 *********************************************************************/
uint32_t wait_on_value(uint32_t read_bit_mask, uint32_t match_value, uint32_t read_addr, uint32_t bound)
{
	uint32_t i = 0, val;
	do {
		++i;
		val = __raw_readl(read_addr) & read_bit_mask;
		if (val == match_value)
			return 1;
		if (i == bound)
			return 0;
	} while (1);
}

uint32_t cortex_a9_rev()
{
	uint32_t i;

	asm ("mrc p15, 0, %0, c0, c0, 0" : "=r" (i));

	return i;
}

uint32_t omap_revision()
{
	switch (cortex_a9_rev()) {

	case 0x410FC091:
		return OMAP4430_ES1_0;

	case 0x411FC092:
		switch ((__raw_readl(OMAP44XX_CTRL_ID_CODE) >> 28) & 0xF) {
		case 0:
		case 1:
		case 2:
			return OMAP4430_ES2_0;
		case 3:
			return OMAP4430_ES2_1;
		default:
			return OMAP4430_ES2_2;
		}
	}

	return OMAP4430_SILICON_ID_INVALID;
}

class l4per {
public:
	auto clock_enable() -> void
	{
		sr32(CM_L4PER_CLKSTCTRL, 0, 32, 0x2);
		sr32(CM_L4PER_DMTIMER10_CLKCTRL, 0, 32, 0x2);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_DMTIMER10_CLKCTRL, LDELAY);
		sr32(CM_L4PER_DMTIMER11_CLKCTRL, 0, 32, 0x2);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_DMTIMER11_CLKCTRL, LDELAY);
		sr32(CM_L4PER_DMTIMER2_CLKCTRL, 0, 32, 0x2);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_DMTIMER2_CLKCTRL, LDELAY);
		sr32(CM_L4PER_DMTIMER3_CLKCTRL, 0, 32, 0x2);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_DMTIMER3_CLKCTRL, LDELAY);
		sr32(CM_L4PER_DMTIMER4_CLKCTRL, 0, 32, 0x2);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_DMTIMER4_CLKCTRL, LDELAY);
		sr32(CM_L4PER_DMTIMER9_CLKCTRL, 0, 32, 0x2);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_DMTIMER9_CLKCTRL, LDELAY);
	}
};

class gpio {
public:
	auto clock_enable() -> void
	{
/*
		sr32(CM_L4PER_GPIO2_CLKCTRL, 0, 32, 0x1);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_GPIO2_CLKCTRL, LDELAY);
		sr32(CM_L4PER_GPIO3_CLKCTRL, 0, 32, 0x1);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_GPIO3_CLKCTRL, LDELAY);
		sr32(CM_L4PER_GPIO4_CLKCTRL, 0, 32, 0x1);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_GPIO4_CLKCTRL, LDELAY);
		sr32(CM_L4PER_GPIO5_CLKCTRL, 0, 32, 0x1);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_GPIO5_CLKCTRL, LDELAY);
		sr32(CM_L4PER_GPIO6_CLKCTRL, 0, 32, 0x1);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_GPIO6_CLKCTRL, LDELAY);
		// TODO: wtf is CM_L4PER_HDQ1W_CLKCTRL?
		sr32(CM_L4PER_HDQ1W_CLKCTRL, 0, 32, 0x2);
*/
	}
};

class clocks {
public:
	// Get the sysclk speed from cm_sys_clksel
	auto get_cm_sysclk() -> bool
	{
		// Set the CM_SYS_CLKSEL in case ROM code has not set
		// TODO: wtf is 0x7?
		__raw_writel(0x7, CM_SYS_CLKSEL);
		return !! __raw_readl(CM_SYS_CLKSEL);
	}
};

class mux {
public:
	auto set_muxconf() -> void
	{
// TODO: use ARRAY_SIZE instead of sizeof(array) / sizeof(array[0])
		for (size_t n = 0; n < sizeof omap4panda_mux / sizeof omap4panda_mux[0]; n++)
			__raw_writew(omap4panda_mux[n].value, omap4panda_mux[n].ads);
	}
};

class uart {
public:
	auto clock_enable() -> void
	{
		sr32(CM_L4PER_UART1_CLKCTRL, 0, 32, 0x2);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_UART1_CLKCTRL, LDELAY);
		sr32(CM_L4PER_UART2_CLKCTRL, 0, 32, 0x2);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_UART2_CLKCTRL, LDELAY);
		sr32(CM_L4PER_UART3_CLKCTRL, 0, 32, 0x2);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_UART3_CLKCTRL, LDELAY);
		sr32(CM_L4PER_UART4_CLKCTRL, 0, 32, 0x2);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_UART4_CLKCTRL, LDELAY);
	}

	// Init UART3 (the one that has DSUB connector outside
	auto init() -> void
	{
		const int divisor = (NS16550_CLK / 16 / 115200);

		__raw_writeb(0, OMAP44XX_UART3 + UART_OFF_IER);
		__raw_writeb(7, OMAP44XX_UART3 + UART_OFF_MDR1);
		__raw_writeb(LCR_BKSE | LCRVAL, OMAP44XX_UART3 + UART_OFF_LCR);
		__raw_writeb(divisor & 0xff, OMAP44XX_UART3 + UART_OFF_DLL);
		__raw_writeb((divisor >> 8) & 0xff, OMAP44XX_UART3 + UART_OFF_DLH);
		__raw_writeb(LCRVAL, OMAP44XX_UART3 + UART_OFF_LCR);
		__raw_writeb(MCRVAL, OMAP44XX_UART3 + UART_OFF_MCR);
		__raw_writeb(FCRVAL, OMAP44XX_UART3 + UART_OFF_FCR);
		__raw_writeb(0, OMAP44XX_UART3 + UART_OFF_MDR1);
	}

	auto puts(const char *string) -> void
	{
		do {
			while(!(__raw_readb(OMAP44XX_UART3 + UART_OFF_LSR) & LSR_THRE)) {
			}
			__raw_writeb(*string, OMAP44XX_UART3 + UART_OFF_THR);
		} while (*string++);
	}
};

class mmc {
public:
	auto clock_enable() -> void
	{
		sr32(CM_L3INIT_HSMMC1_CLKCTRL, 0, 2, 0x2);
		sr32(CM_L3INIT_HSMMC1_CLKCTRL, 24, 1, 0x1);
		sr32(CM_L3INIT_HSMMC2_CLKCTRL, 0, 2, 0x2);
		sr32(CM_L3INIT_HSMMC2_CLKCTRL, 24, 1, 0x1);
		sr32(CM_L4PER_MMCSD3_CLKCTRL, 0, 32, 0x2);
		wait_on_value(BIT18|BIT17|BIT16, 0, CM_L4PER_MMCSD3_CLKCTRL, LDELAY);
		sr32(CM_L4PER_MMCSD4_CLKCTRL, 0, 32, 0x2);
		wait_on_value(BIT18|BIT17|BIT16, 0, CM_L4PER_MMCSD4_CLKCTRL, LDELAY);
		sr32(CM_L4PER_MMCSD5_CLKCTRL, 0, 32, 0x2);
		wait_on_value(BIT17|BIT16, 0, CM_L4PER_MMCSD5_CLKCTRL, LDELAY);
	}

	auto stuff() -> void
	{
/*
		int ret = mmc_init(1);
		if (!ret) {
			puts("mmc_init failed\n\r");
		}

		puts("MMC\n\r");

		unsigned long out = 0;

		ret = mmc_read_sec(0, sizeof(unsigned long), &out);
		if (!ret) {
			puts("mmc_read_sec failed\n\r");
		}

		puts(&((char*)&out)[0]);
*/
	}
};

class wkup {
public:
	auto clock_enable() -> void
	{
		sr32(CM_WKUP_GPIO1_CLKCTRL, 0, 32, 0x1);
		wait_on_value(BIT17|BIT16, 0, CM_WKUP_GPIO1_CLKCTRL, LDELAY);
		sr32(CM_WKUP_TIMER1_CLKCTRL, 0, 32, 0x01000002);
		wait_on_value(BIT17|BIT16, 0, CM_WKUP_TIMER1_CLKCTRL, LDELAY);

		sr32(CM_WKUP_KEYBOARD_CLKCTRL, 0, 32, 0x2);
		wait_on_value(BIT17|BIT16, 0, CM_WKUP_KEYBOARD_CLKCTRL, LDELAY);

		sr32(CM_SDMA_CLKSTCTRL, 0, 32, 0x0);
		sr32(CM_MEMIF_CLKSTCTRL, 0, 32, 0x3);
		sr32(CM_MEMIF_EMIF_1_CLKCTRL, 0, 32, 0x1);
		wait_on_value(BIT17|BIT16, 0, CM_MEMIF_EMIF_1_CLKCTRL, LDELAY);
		sr32(CM_MEMIF_EMIF_2_CLKCTRL, 0, 32, 0x1);
		wait_on_value(BIT17|BIT16, 0, CM_MEMIF_EMIF_2_CLKCTRL, LDELAY);
		sr32(CM_D2D_CLKSTCTRL, 0, 32, 0x3);
		sr32(CM_L3_2_GPMC_CLKCTRL, 0, 32, 0x1);
		wait_on_value(BIT17|BIT16, 0, CM_L3_2_GPMC_CLKCTRL, LDELAY);
		sr32(CM_L3INSTR_L3_3_CLKCTRL, 0, 32, 0x1);
		wait_on_value(BIT17|BIT16, 0, CM_L3INSTR_L3_3_CLKCTRL, LDELAY);
		sr32(CM_L3INSTR_L3_INSTR_CLKCTRL, 0, 32, 0x1);
		wait_on_value(BIT17|BIT16, 0, CM_L3INSTR_L3_INSTR_CLKCTRL, LDELAY);
		sr32(CM_L3INSTR_OCP_WP1_CLKCTRL, 0, 32, 0x1);
		wait_on_value(BIT17|BIT16, 0, CM_L3INSTR_OCP_WP1_CLKCTRL, LDELAY);
	}
};

class leds {
public:
	auto light_up_both() -> void
	{
		uint32_t v = __raw_readl(OMAP44XX_GPIO_BASE1 + __GPIO_OE);

		// set both LED gpio to output
		__raw_writel((v & ~(0x03 << 7)), OMAP44XX_GPIO_BASE1 + __GPIO_OE);

		v = __raw_readl(OMAP44XX_GPIO_BASE1 + __GPIO_DATAOUT);
		__raw_writel((v | (0x03 << 7)), OMAP44XX_GPIO_BASE1 + __GPIO_DATAOUT);
	}
};

};

class omap4430 {
public:
	lowlevel::l4per l4per;
	lowlevel::clocks clocks;
	lowlevel::mux mux;
	lowlevel::gpio gpio;
	lowlevel::uart uart;
	lowlevel::leds leds;
	lowlevel::mmc mmc;
	lowlevel::wkup wkup;
};

class myC {
private:
		int a{0};
		omap4430 soc;
public:
		myC()
		{
			soc.uart.puts("myC()\n\r");
		}

		virtual ~myC()
		{
			soc.uart.puts("~myC()\n\r");
		}

		auto operator new(size_t size) -> void *
		{
// malloc(i);
//			soc.uart.puts("operator new\n\r");

// For now just return the top of L3 OCM SRAM (56KB)
			return reinterpret_cast<void *>(L3_OCM_RAM);
		}

		auto operator delete(void *p) -> void
		{
// free(p);
//			soc.uart.puts("operator delete\n\r");
		}

		auto puts(const char *string) -> void
		{
			soc.uart.puts(string);
		}

		auto get_sysclk() -> bool
		{
			return soc.clocks.get_cm_sysclk();
		}

		auto enable_clocks() -> void
		{
			soc.l4per.clock_enable();
			soc.gpio.clock_enable();
			soc.uart.clock_enable();
			soc.mmc.clock_enable();
			soc.wkup.clock_enable();
		}

		auto set_mux() -> void
		{
			soc.mux.set_muxconf();
		}

		auto lights_up() -> void
		{
			soc.leds.light_up_both();
		}
};

extern "C" void allthecoolstuff()
{

	myC c;

	bool enabled = c.get_sysclk();
// TODO: wtf is this shit?
	if (enabled) {
		c.enable_clocks();
	}

	c.set_mux();

	c.lights_up();

	c.puts("Hi from C++\n\r");

	myC* cc = new myC;
	delete cc;

	c.puts("Done\n\r");
}

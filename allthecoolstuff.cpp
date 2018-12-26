#include <stdint.h>	// TODO: the fuck cstdint doesn't work on Debian?
#include <stddef.h>	// TODO: ditto

#include "lowlevel/l4per.hpp"
#include "lowlevel/clocks.hpp"
#include "lowlevel/mux.hpp"
#include "lowlevel/gpio.hpp"
#include "lowlevel/uart.hpp"
#include "lowlevel/leds.hpp"
#include "lowlevel/mmc.hpp"
#include "lowlevel/wkup.hpp"

#include "cpu.h"

#include "omap4430.h"


//TODO: refactor all this
namespace lowlevel {

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
// TODO: this gets called before soc initialisation
//			soc.uart.puts("myC()\n\r");
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

		auto uart_init() -> void
		{
			soc.uart.init();
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

	c.uart_init();

	c.lights_up();

	c.puts("Hi from C++\n\r");

	myC* cc = new myC;
	delete cc;

	c.puts("Done\n\r");
}

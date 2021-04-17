#include <stdint.h>	// TODO: the fuck cstdint doesn't work on Debian?
#include <stddef.h>	// TODO: ditto

#include "alloc.hpp"

#include "lowlevel/l4per.hpp"
#include "lowlevel/clocks.hpp"
#include "lowlevel/mux.hpp"
#include "lowlevel/gpio.hpp"
#include "lowlevel/uart.hpp"
#include "lowlevel/leds.hpp"
#include "lowlevel/mmc.hpp"
#include "lowlevel/wkup.hpp"

#include "omap4430.h"


static void *__dso_handle = nullptr;
extern "C" auto __aeabi_atexit(void *object, void (*destroyer)(void *), void *dso_handle) -> int
{
	(void)object;
	(void)destroyer;
	(void)dso_handle;
	return 0;
}

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

	auto cortex_a9_rev() -> uint32_t
	{
		uint32_t i;

		asm ("mrc p15, 0, %0, c0, c0, 0" : "=r" (i));

		return i;
	}

	auto omap_revision() -> uint32_t
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

class SoC {
private:
		omap4430 soc;
public:
		SoC()
		{
// TODO: this gets called before soc initialisation
//			soc.uart.puts("soc()\n\r");
		}

		virtual ~SoC()
		{
			soc.uart.puts("~soc()\n\r");
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

static SoC global_soc;
static alloc global_alloc;

auto operator new(size_t size) -> void *
{
	global_soc.puts("operator new\n\r");
	return global_alloc.malloc(size);
}

auto operator delete(void *ptr) -> void
{
	global_soc.puts("operator delete\n\r");
	global_alloc.free(ptr);
}

auto operator delete(void *ptr, size_t sz) -> void
{
	global_soc.puts("operator delete void *, size_t\n\r");
	global_alloc.free(ptr, sz);
}

extern "C" auto allthecoolstuff() -> void
{
	bool enabled = global_soc.get_sysclk();
// TODO: wtf is this shit?
	if (enabled) {
		global_soc.enable_clocks();
	}

	global_soc.set_mux();
	global_soc.uart_init();
	global_soc.lights_up();

	global_soc.puts("Hi from C++\n\r");

	SoC* cc = new SoC;
	delete cc;

	global_soc.puts("Done\n\r");
}

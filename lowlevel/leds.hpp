#ifndef _LEDS_HPP_
#define _LEDS_HPP_

#include "read_write.hpp"

#include <omap4430.h>


//TODO: refactor all this
namespace lowlevel {

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

#endif // _LEDS_HPP_

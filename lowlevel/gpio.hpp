#ifndef _GPIO_HPP_
#define _GPIO_HPP_

#include "read_write.hpp"

#include <cpu.h>
#include <defs.h>	// misc shite


//TODO: refactor all this
namespace lowlevel {

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

};

#endif // _GPIO_HPP_

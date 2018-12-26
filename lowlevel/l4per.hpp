#ifndef _L4PER_HPP_
#define _L4PER_HPP_

#include "read_write.hpp"

#include <cpu.h>
#include <defs.h>	// misc shite


//TODO: refactor all this
namespace lowlevel {

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

};

#endif // _L4PER_HPP_

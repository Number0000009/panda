#ifndef _WKUP_HPP_
#define _WKUP_HPP_

#include "read_write.hpp"

#include <cpu.h>
#include <defs.h>	// misc shite


//TODO: refactor all this
namespace lowlevel {

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

};

#endif // _WKUP_HPP_

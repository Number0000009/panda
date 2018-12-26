#ifndef _MMC_HPP_
#define _MMC_HPP_

#include "read_write.hpp"

#include <cpu.h>
#include <defs.h>	// misc shite


//TODO: refactor all this
namespace lowlevel {

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

};

#endif // _MMC_HPP_

#ifndef _CLOCKS_HPP_
#define _CLOCKS_HPP_

#include "read_write.hpp"

#include <cpu.h>


//TODO: refactor all this
namespace lowlevel {

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

};

#endif // _CLOCKS_HPP_

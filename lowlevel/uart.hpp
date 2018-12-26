#ifndef _UART_HPP_
#define _UART_HPP_

#include "read_write.hpp"

#include <uart.h>
#include <cpu.h>
#include <defs.h>	// misc shite


//TODO: refactor all this
namespace lowlevel {

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
		static const int divisor = (NS16550_CLK / 16 / 115200);

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

};

#endif // _UART_HPP_

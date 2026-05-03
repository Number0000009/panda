#pragma once

#include "helpers.h"

#define CM_WKUP_GPIO1_CLKCTRL 0x4A307838u	// GPIO1 clock control
#define CM_L4PER_GPIO2_CLKCTRL 0x4A009560u	// GPIO2 clock control
#define CM_L4PER_GPIO6_CLKCTRL 0x4A009580u	// GPIO6 clock control

#define GPIO1 0x4A310000u			// GPIO1 register base
#define GPIO2 0x48055000u			// GPIO2 register base
#define GPIO_OE 0x134u				// output-enable offset
#define GPIO_SET 0x194u				// set-output offset
#define GPIO_CLR 0x190u				// clear-output offset

void gpio_out(uint32_t base, unsigned bit, int v);

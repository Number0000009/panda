#include "gpio.h"

/*
 * Configure one GPIO as an output and drive it high or low.
 * PandaBoard hub power and hub reset are controlled this way.
 */
void gpio_out(uint32_t base, unsigned bit, int v)
{
	uint32_t m = 1u << bit;
	w32(base + GPIO_OE, r32(base + GPIO_OE) & ~m);	// make GPIO an output
	if (v)
		w32(base + GPIO_SET, m);		// drive GPIO high
	else
		w32(base + GPIO_CLR, m);		// drive GPIO low
}

#include "helpers.h"
#include "uart.h"

#define WAIT_REG_LOOPS 10000000u		// generic register wait loop

void w32(uint32_t a, uint32_t v)
{
	*(volatile uint32_t *)a = v;
}

uint32_t r32(uint32_t a)
{
	return *(volatile uint32_t *)a;
}

void w16(uint32_t a, uint16_t v)
{
	*(volatile uint16_t *)a = v;
}

void delay(uint32_t n)
{
	while (n--)
		__asm__ volatile("nop");
}

void barrier(void)
{
	__asm__ volatile("dsb; isb" ::: "memory");
}

void dump(const char *s, uint32_t a)
{
	puts(s);
	puts("=");
	hex32(r32(a));
	puts("\n");
}

void dumpv(const char *s, uint32_t v)
{
	puts(s);
	puts("=");
	hex32(v);
	puts("\n");
}

int fail(const char *s)
{
	puts(s);
	return -1;
}

void halt(void)
{
	for (;;) {
	}
}

void memzero(volatile void *p, unsigned n)
{
	volatile uint8_t *x = (volatile uint8_t *)p;
	while (n--)
		*x++ = 0;
}

/*
 * Poll a register until all bits in a mask become clear.
 * Returns 0 on success, -1 if the hardware did not settle in time.
 */
int wait_clr(uint32_t a, uint32_t m)
{
	for (uint32_t i = 0; i < WAIT_REG_LOOPS; i++)
		if (!(r32(a) & m))			// poll until masked bits clear
			return 0;
	return -1;
}

/*
 * Poll a register until at least one bit in a mask becomes set.
 * Used for reset-complete and schedule-status handshakes.
 */
int wait_set(uint32_t a, uint32_t m)
{
	for (uint32_t i = 0; i < WAIT_REG_LOOPS; i++)
		if (r32(a) & m)			// poll until any masked bit sets
			return 0;
	return -1;
}

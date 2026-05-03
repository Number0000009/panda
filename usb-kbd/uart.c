#include "helpers.h"
#include "uart.h"

void putc(char c)
{
	if (c == '\n')
		putc('\r');
	while (!(r32(UART3 + 0x14) & 0x20)) {
	}
	w32(UART3, c);
}

void puts(const char *s)
{
	while (*s)
		putc(*s++);
}

void hex32(uint32_t v)
{
	for (int i = 28; i >= 0; i -= 4)
		putc("0123456789ABCDEF"[(v >> i) & 15]);
}

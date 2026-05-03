#pragma once

#include <stdint.h>

#define UART3 0x48020000u			// UART3 register base

void putc(char c);
void puts(const char *s);
void hex32(uint32_t v);

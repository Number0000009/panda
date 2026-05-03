#pragma once

#include <stdint.h>

#define BIT(n) (1u << (n))			// single-bit mask helper

void w32(uint32_t a, uint32_t v);
uint32_t r32(uint32_t a);
void w16(uint32_t a, uint16_t v);

void delay(uint32_t n);
void barrier(void);
void dump(const char *s, uint32_t a);
void dumpv(const char *s, uint32_t v);
int fail(const char *s);
void halt(void);
void memzero(volatile void *p, unsigned n);
int wait_clr(uint32_t a, uint32_t m);
int wait_set(uint32_t a, uint32_t m);

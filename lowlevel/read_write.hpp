#ifndef _READ_WRITE_HPP_
#define _READ_WRITE_HPP_

#include <stdint.h>	// TODO: the fuck cstdint doesn't work on Debian?
#include <stddef.h>	// TODO: ditto


//TODO: refactor all this
namespace lowlevel {

#define __arch_getb(a)		(*(volatile uint8_t *)(a))
#define __arch_getw(a)		(*(volatile uint16_t *)(a))
#define __arch_getl(a)		(*(volatile uint32_t *)(a))

#define __arch_putb(v,a)	(*(volatile uint8_t *)(a) = (v))
#define __arch_putw(v,a)	(*(volatile uint16_t *)(a) = (v))
#define __arch_putl(v,a)	(*(volatile uint32_t *)(a) = (v))

#define __raw_writeb(v,a)	__arch_putb(v,a)
#define __raw_writew(v,a)	__arch_putw(v,a)
#define __raw_writel(v,a)	__arch_putl(v,a)

#define __raw_readb(a)		__arch_getb(a)
#define __raw_readw(a)		__arch_getw(a)
#define __raw_readl(a)		__arch_getl(a)


// sr32 - clear & set a value in a bit range for a 32 bit address
void sr32(uint32_t addr, uint32_t start_bit, uint32_t num_bits, uint32_t value)
{
	uint32_t msk{(1UL << num_bits) - 1}, tmp{__raw_readl(addr) & ~(msk << start_bit)};
	__raw_writel(tmp | (value << start_bit), addr);
}

// wait_on_value() - common routine to allow waiting for changes in volatile regs
bool wait_on_value(uint32_t read_bit_mask, uint32_t match_value, uint32_t read_addr, uint32_t bound)
{
	uint32_t i{0};

	do {
		uint32_t val{__raw_readl(read_addr) & read_bit_mask};

		if (val == match_value) return false;
		if (++i == bound) return true;
	} while (true);
}

};

#endif // _READ_WRITE_HPP_

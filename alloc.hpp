#ifndef _ALLOC_HPP_
#define _ALLOC_HPP_

#include "cpu.h"


class alloc {
private:
	// L3 OCM SRAM (56KB)
	void *pool_start {reinterpret_cast<void *>(L3_OCM_RAM)};
	void *pool_end {reinterpret_cast<void *>(L3_OCM_RAM_END)};

public:
// TODO: no exceptions, so no std::bad_alloc
	auto malloc(size_t size) -> void *
	{
		return pool_start;
	}

	auto free(void *ptr) -> void
	{
	}

	auto free(void *ptr, size_t sz) -> void
	{
	}
};

#endif // _ALLOC_HPP_

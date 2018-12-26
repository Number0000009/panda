#ifndef _MUX_HPP_
#define _MUX_HPP_

#include "read_write.hpp"

#include <omap4430.h>

#include <mux.h>
#include <mux_data.h>


//TODO: refactor all this
namespace lowlevel {

class mux {
public:
	auto set_muxconf() -> void
	{
// TODO: use ARRAY_SIZE instead of sizeof(array) / sizeof(array[0])
		for (size_t n = 0; n < sizeof omap4panda_mux / sizeof omap4panda_mux[0]; n++)
			__raw_writew(omap4panda_mux[n].value, omap4panda_mux[n].ads);
	}
};

};

#endif // _MUX_HPP_

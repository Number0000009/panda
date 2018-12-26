#include <stddef.h>

#include "omap4430.h"

extern "C" void puts(const char *);

class myC {
private:
		int a{0};
public:
		myC()
		{
			puts("myC()\n\r");
		}

		virtual ~myC()
		{
			puts("~myC()\n\r");
		}

		auto operator new(size_t size) -> void *
		{
// malloc(i);
			puts("operator new\n\r");

// For now just return the top of L3 OCM SRAM (56KB)
			return reinterpret_cast<void *>(L3_OCM_RAM);
		}

		auto operator delete(void *p) -> void
		{
// free(p);
			puts("operator delete\n\r");
		}
};

extern "C" void allthecoolstuff()
{
	puts("Hi from C++\n\r");

	myC c;
	myC* cc = new myC;
	delete cc;

	puts("Done\n\r");
}

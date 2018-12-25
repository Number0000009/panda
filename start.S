#include "omap4430.h"

	.global vectors
vectors:

.space 0x10

	b reset
	b .
	b .
	b .
	b .
	b .
	b .
	b .

	.global end_vectors
end_vectors:

reset:
	ldr	sp, =SAR_RAM1_END-4
	bl main
end:
	wfe
	b end

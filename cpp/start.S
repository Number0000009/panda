SAR1		= 0x4a326000
SAR1_END	= 0x4a326fff

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
	ldr	sp, =SAR1_END-4
	bl main
end:
	wfe
	b end

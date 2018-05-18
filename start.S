L3_OCM_RAM	= 0x40300000
L3_OCM_RAM_END	= 0x4030dfff

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
	b .
	ldr	sp, =SAR1_END+1
	bl main

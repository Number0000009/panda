CROSS ?= arm-none-eabi-

CFLAGS = -mcpu=cortex-a9 -marm -ffreestanding -fno-builtin -nostdlib \
         -O2 -Wall -Wextra

LDFLAGS = -T linker.ld -nostdlib

all: panda_dvi.bin

panda_dvi.elf: start.S panda_dvi.c linker.ld
	$(CROSS)gcc $(CFLAGS) $(LDFLAGS) -o $@ start.S panda_dvi.c

panda_dvi.bin: panda_dvi.elf
	$(CROSS)objcopy -O binary $< $@

clean:
	rm -f panda_dvi.elf panda_dvi.bin

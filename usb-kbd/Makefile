CROSS ?= arm-none-eabi-

CFLAGS = -mcpu=cortex-a9 -marm -ffreestanding -fno-builtin -nostdlib \
         -O2 -Wall -Wextra

LDFLAGS = -T linker.ld -nostdlib

all: panda_kbd.bin

panda_kbd.elf: start.S panda_kbd.c linker.ld
	$(CROSS)gcc $(CFLAGS) $(LDFLAGS) -o $@ start.S panda_kbd.c

panda_kbd.bin: panda_kbd.elf
	$(CROSS)objcopy -O binary $< $@

clean:
	rm -f panda_kbd.elf panda_kbd.bin

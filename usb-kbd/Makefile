CROSS ?= arm-none-eabi-

CFLAGS = -mcpu=cortex-a9 -marm -ffreestanding -fno-builtin -nostdlib \
         -O2 -Wall -Wextra

LDFLAGS = -T linker.ld -nostdlib
SRCS = start.S panda_kbd.c helpers.c uart.c gpio.c board.c ehci.c usb_xfer.c hub.c keyboard.c

all: panda_kbd.bin

panda_kbd.elf: $(SRCS) board.h helpers.h uart.h usb.h hub.h keyboard.h gpio.h linker.ld
	$(CROSS)gcc $(CFLAGS) $(LDFLAGS) -o $@ $(SRCS)

panda_kbd.bin: panda_kbd.elf
	$(CROSS)objcopy -O binary $< $@

clean:
	rm -f panda_kbd.elf panda_kbd.bin

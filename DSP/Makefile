CROSS ?= arm-none-eabi-

CC      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy

CFLAGS := -mcpu=cortex-a9 -marm -ffreestanding -fno-builtin -fno-stack-protector \
          -fno-pic -nostdlib -O2 -Wall -Wextra -Werror
LDFLAGS := -T linker.ld -Wl,--build-id=none -nostdlib

all: dsp_bringup.bin

start.o: start.S
	$(CC) $(CFLAGS) -c -x assembler-with-cpp -o $@ $<

main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $<

dsp_bringup.elf: start.o main.o linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ start.o main.o

dsp_bringup.bin: dsp_bringup.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f *.o *.elf dsp_bringup.bin

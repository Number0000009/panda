CROSS ?= arm-none-eabi-
AS      := $(CROSS)gcc
LD      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy

ASFLAGS := -c -x assembler-with-cpp -mcpu=cortex-a9 -nostdlib -ffreestanding -O2
LDFLAGS := -nostartfiles -nostdlib -Wl,-T,linker.ld -Wl,--build-id=none

all: m3_bringup.bin

m3_bringup.o: m3_bringup.S
	$(AS) $(ASFLAGS) -o $@ $<

m3_bringup.elf: m3_bringup.o linker.ld
	$(LD) $(LDFLAGS) -o $@ m3_bringup.o

m3_bringup.bin: m3_bringup.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f m3_bringup.o m3_bringup.elf m3_bringup.bin

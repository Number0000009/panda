CROSS ?= arm-none-eabi

CC      := $(CROSS)-gcc
OBJCOPY := $(CROSS)-objcopy
OBJDUMP := $(CROSS)-objdump

TARGET := arm968-dmem-debug

A9_CFLAGS := \
	-mcpu=cortex-a9 \
	-marm \
	-O2 \
	-g3 \
	-ffreestanding \
	-fno-builtin \
	-fno-common \
	-ffunction-sections \
	-fdata-sections \
	-Wall \
	-Wextra \
	-nostdlib \
	-nostartfiles

ICONT_CFLAGS := \
	-march=armv5te \
	-marm \
	-O2 \
	-g3 \
	-ffreestanding \
	-fno-builtin \
	-fno-common \
	-nostdlib \
	-nostartfiles

LDFLAGS := \
	-T linker.ld \
	-nostdlib \
	-nostartfiles \
	-Wl,-Map=$(TARGET).map \
	-Wl,--gc-sections

A9_OBJS := start.o main.o icont_bin.o

all: $(TARGET).elf $(TARGET).bin $(TARGET).dis

$(TARGET).elf: $(A9_OBJS) linker.ld
	$(CC) $(A9_CFLAGS) $(LDFLAGS) -o $@ $(A9_OBJS) -lgcc

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).dis: $(TARGET).elf
	$(OBJDUMP) -D $< > $@

start.o: start.S
	$(CC) $(A9_CFLAGS) -c -o $@ $<

main.o: main.c
	$(CC) $(A9_CFLAGS) -c -o $@ $<

icont968.o: icont.S
	$(CC) $(ICONT_CFLAGS) -c -o $@ $<

icont.elf: icont968.o icont_linker.ld
	$(CC) $(ICONT_CFLAGS) -T icont_linker.ld -o $@ icont968.o -lgcc

icont.bin: icont.elf
	$(OBJCOPY) -O binary $< $@

icont_bin.o: icont.bin
	$(OBJCOPY) -I binary -O elf32-littlearm -B arm $< $@

clean:
	rm -f *.o *.elf *.bin *.map *.dis icont968.o

.PHONY: all clean

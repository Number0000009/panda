CROSS ?= arm-none-eabi

CC      := $(CROSS)-gcc
OBJCOPY := $(CROSS)-objcopy
OBJDUMP := $(CROSS)-objdump

TARGET := arm968-test

CFLAGS := \
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

LDFLAGS := \
	-T linker.ld \
	-nostdlib \
	-nostartfiles \
	-Wl,-Map=$(TARGET).map \
	-Wl,--gc-sections

OBJS := start.o main.o

all: $(TARGET).elf $(TARGET).bin $(TARGET).dis

$(TARGET).elf: $(OBJS) linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) -lgcc

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).dis: $(TARGET).elf
	$(OBJDUMP) -D $< > $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.elf *.bin *.map *.dis

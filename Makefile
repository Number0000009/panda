BUILD_DIR := $(shell cd $(BUILD_DIR) && /bin/pwd)

CROSS_COMPILE ?= arm-none-eabi-

AS	:= $(CROSS_COMPILE)as
LD	:= $(CROSS_COMPILE)ld
CC	:= $(CROSS_COMPILE)gcc
CPP	:= $(CROSS_COMPILE)g++
AR	:= $(CROSS_COMPILE)ar
NM	:= $(CROSS_COMPILE)nm
STRIP	:= $(CROSS_COMPILE)strip
OBJCOPY	:= $(CROSS_COMPILE)objcopy
OBJDUMP	:= $(CROSS_COMPILE)objdump
RANLIB	:= $(CROSS_COMPILE)RANLIB

LDSCRIPT = linker.lds
PLATFORM_LDFLAGS =
TEXT_BASE = 0x40304350	# L3_OCM_RAM = 0x40300000 - 0x4030DFFF 56KB

CFLAGS	 = -std=c11 -Wall -Werror -fomit-frame-pointer -fno-common -nostdlib -fno-builtin
CPPFLAGS = -std=c++14 -Wall -Werror -fomit-frame-pointer -fno-common -nostdlib -fno-builtin -fno-exceptions -fno-rtti
INCLUDES = -I .

LDFLAGS = -Bstatic -T $(LDSCRIPT) -Ttext $(TEXT_BASE) $(PLATFORM_LDFLAGS)

START	= start.o
COBJS	= main.o mmc.o
CPPOBJS = allthecoolstuff.o

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c)
CPPSRCS := $(CPPOBJS:.o=.cpp)

START	:= $(addprefix $(obj),$(START))
COBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
CPPOBJS	:= $(addprefix $(obj),$(CPPOBJS))

OUTPUT	:= x-load.bin

all:	$(START) $(CPPOBJS)
	$(LD) $(LDFLAGS) -o $(OUTPUT)
	$(OBJCOPY) -O binary $(OUTPUT)
	./signGP x-load.bin 0x40304350 1
	mv ./x-load.bin.ift ./MLO

$(START):
	$(CC) $(CFLAGS) $(SRCS) -c

$(COBJS):
	$(CC) $(CFLAGS) $(SRCS) -c

$(CPPOBJS):
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(CPPSRCS) -c

clean:
	rm *.o
	rm $(OUTPUT)

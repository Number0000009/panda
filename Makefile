BUILD_DIR := $(shell cd $(BUILD_DIR) && /bin/pwd)

CROSS_COMPILE ?= arm-none-eabi-

AS	:= $(CROSS_COMPILE)as
LD	:= $(CROSS_COMPILE)ld
CC	:= $(CROSS_COMPILE)gcc
CPP	:= $(CC) -E
AR	:= $(CROSS_COMPILE)ar
NM	:= $(CROSS_COMPILE)nm
STRIP	:= $(CROSS_COMPILE)strip
OBJCOPY	:= $(CROSS_COMPILE)objcopy
OBJDUMP	:= $(CROSS_COMPILE)objdump
RANLIB	:= $(CROSS_COMPILE)RANLIB

LDSCRIPT = linker.lds
PLATFORM_LDFLAGS =
TEXT_BASE = 0

CFLAGS	= -Wall -Werror -fomit-frame-pointer -fno-common -nostdlib

LDFLAGS	+= -Bstatic -T $(LDSCRIPT) -Ttext $(TEXT_BASE) $(PLATFORM_LDFLAGS)

START	= start.o
#COBJS	= cpu.o mmc.o sys_info.o
COBJS	=

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
START	:= $(addprefix $(obj),$(START))
OUTPUT	:= MLO

all:	$(START)
	$(LD) $(LDFLAGS) -o $(OUTPUT)
	$(OBJCOPY) -O binary $(OUTPUT)

$(START):
	$(CC) $(CFLAGS) $(SRCS) -c

$(OBJS):
	$(CC) $(CFLAGS) $(SRCS) -c

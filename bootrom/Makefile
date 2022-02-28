#TOOLCHAIN=/data/liup/project/X6/toolchain/gcc-linaro-4.9-2014.11-x86_64_aarch64-elf
TOOLCHAIN=/home/new/liup/tool/gcc-arm-8.2-2018.08-x86_64-aarch64-elf
CROSS = $(TOOLCHAIN)/bin/aarch64-elf-
SYSTEM_DIR=$(TOOLCHAIN)/bin/../lib/gcc/aarch64-elf/8.2.1/include

TARGET := bootrom

CC = $(CROSS)gcc
LD = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy
OBJDUMP = $(CROSS)objdump
 
INCDIRS := ./ \
		    ./include \
		    ./board \
		    ./project \
		    ./bsp \
		    ./bsp/common \
		    ./bsp/int \
		    ./bsp/uart \
		    ./bsp/xload \

SRCDIRS := ./ \
            ./board \
		    ./project \
		    ./bsp \
		    ./bsp/common \
		    ./bsp/int \
		    ./bsp/uart \
		    ./bsp/xload \

INC := $(patsubst %, -I%, $(INCDIRS))
SFILES := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.S))
CFILES := $(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))		 
SOURCES_S := $(notdir $(SFILES))
SOURCES_C := $(notdir $(CFILES))
SOBJS := $(SOURCES_S:.S=.o)
COBJS := $(SOURCES_C:.c=.o)
OBJECTS := $(SOBJS) $(COBJS)

VPATH := $(SRCDIRS)

CFLAGS=-nostdinc $(INC) -isystem $(SYSTEM_DIR) -I./bd_include -D__KERNEL__ -Wall -Wstrict-prototypes -Wno-format-security -fno-builtin -ffreestanding -Os -fno-stack-protector -fstack-usage -Wno-format-nonliteral -Werror=date-time -Werror -D__ARM__ -march=armv8-a -ffunction-sections -fdata-sections -fno-common -ffixed-r9 -fno-common -pipe

$(TARGET).bin : $(OBJECTS) 
	$(LD) --gc-sections -Tbootrom.lds -o $(TARGET).elf $^ -Map=$(TARGET).map 
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
	$(OBJDUMP) -D  -S $(TARGET).elf >$(TARGET).dis
	
.c.o:
	$(CC) $(INC) $(CFLAGS) $(DEBUG) -c -o $@  $<
	
.S.o:
	$(CC) -D__ASSEMBLY__ $(INC) $(CFLAGS) $(DEBUG) -c -o $@  $<

clean:
	rm -rf $(TARGET).elf
	rm -rf $(TARGET).dis
	rm -rf $(TARGET).map
	rm -rf ./*.su
	rm -rf $(TARGET).bin
	rm -rf $(OBJECTS)
coe:
	python $(TARGET).py $(TARGET).bin $(TARGET).coe

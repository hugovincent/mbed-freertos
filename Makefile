# For mbed beta (LPC2368)
# Hugo Vincent, April 24 2010

CC=arm-eabi-gcc
OBJCOPY=arm-eabi-objcopy
LDSCRIPT=lpc2368.ld

LINKER_FLAGS=-mthumb -nostartfiles -Wl,-oRTOSDemo.elf 

DEBUG=
OPTIM=-O2

CFLAGS= $(DEBUG) \
		$(OPTIM) \
		-std=c99 \
		-T$(LDSCRIPT) \
		-I . \
		-I lib/include \
		-I freertos/portable/GCC/ARM7_LPC23xx \
		-I lib/webserver \
		-I lib/uip \
		-D MBED_LPC23xx \
		-D THUMB_INTERWORK \
		-mcpu=arm7tdmi \
		-D PACK_STRUCT_END=__attribute\(\(packed\)\) \
		-D ALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\) \
		-fomit-frame-pointer \
		-mthumb-interwork \
		-fno-dwarf2-cfi-asm \
		-fno-strict-aliasing
		
THUMB_SOURCE= \
		main.c \
		lib/ParTest.c \
		lib/common/BlockQ.c \
		lib/common/blocktim.c \
		lib/common/flash.c \
		lib/common/integer.c \
		lib/common/GenQTest.c \
		lib/common/QPeek.c \
		lib/common/dynamic.c \
		lib/webserver/uIP_Task.c \
		lib/webserver/emac.c \
		lib/webserver/httpd.c \
		lib/webserver/httpd-cgi.c \
		lib/webserver/httpd-fs.c \
		lib/webserver/http-strings.c \
		lib/uip/uip_arp.c \
		lib/uip/psock.c \
		lib/uip/timer.c \
		lib/uip/uip.c \
		freertos/list.c \
		freertos/queue.c \
		freertos/tasks.c \
		freertos/portable/GCC/ARM7_LPC23xx/port.c \
		lib/heap_2.c \
		lib/syscalls.c

ARM_SOURCE= \
		freertos/portable/GCC/ARM7_LPC23xx/portISR.c \
		lib/webserver/EMAC_ISR.c

THUMB_OBJS = $(THUMB_SOURCE:.c=.o)
ARM_OBJS = $(ARM_SOURCE:.c=.o)

all: RTOSDemo.bin

RTOSDemo.bin : RTOSDemo.elf
	$(OBJCOPY) RTOSDemo.elf -O binary RTOSDemo.bin
	 
RTOSDemo.elf : $(THUMB_OBJS) $(ARM_OBJS) boot.s Makefile
	$(CC) $(CFLAGS) $(ARM_OBJS) $(THUMB_OBJS) $(LIBS) boot.s $(LINKER_FLAGS) 

$(THUMB_OBJS) : %.o : %.c Makefile FreeRTOSConfig.h
	$(CC) -c $(CFLAGS) -mthumb $< -o $@

$(ARM_OBJS) : %.o : %.c Makefile FreeRTOSConfig.h
	$(CC) -c $(CFLAGS) $< -o $@

clean :
	rm -f $(THUMB_OBJS) $(ARM_OBJS) RTOSDemo.elf
	

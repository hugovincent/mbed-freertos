# For mbed beta (LPC2368)
# Hugo Vincent, April 24 2010

CC=arm-eabi-gcc
LD=arm-eabi-ld
OBJCOPY=arm-eabi-objcopy
LDSCRIPT=lpc2368.ld

DEBUG=
OPTIM=-Os

BINNAME=RTOSDemo

CFLAGS= $(DEBUG) \
		$(OPTIM) \
		-std=gnu99 \
		-I . \
		-I lib/include \
		-I freertos/portable/GCC/ARM7_LPC23xx \
		-I lib/webserver \
		-I lib/uip \
		-D MBED_LPC23xx \
		-D THUMB_INTERWORK \
		-mcpu=arm7tdmi \
		-fomit-frame-pointer \
		-mthumb-interwork \
		-fno-dwarf2-cfi-asm \
		-fno-strict-aliasing \
		-Wall -Wcast-align -Wimplicit -Wpointer-arith \
		-Wswitch -Wreturn-type -Wshadow -Wunused \
		-ffunction-sections -fdata-sections \
		-mabi=aapcs -mfloat-abi=soft \
		-lm -lstdc++

LINKER_FLAGS= \
		-nostartfiles \
		-T$(LDSCRIPT) \
		-Wl,--gc-sections \
#		-Wl,--print-gc-sections

GAS_FLAGS= \
		-mcpu=arm7tdmi \
		-mthumb-interwork \
		-mabi=aapcs -mfloat-abi=soft \
		-x assembler-with-cpp 

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
#		lib/uart_ISR.c

GAS_SOURCE= \
		crt0.s

THUMB_OBJS = $(THUMB_SOURCE:.c=.o)
ARM_OBJS   = $(ARM_SOURCE:.c=.o)
GAS_OBJS   = $(GAS_SOURCE:.s=.o)

all: $(BINNAME).bin

$(BINNAME).bin : $(BINNAME).elf
	$(OBJCOPY) $(BINNAME).elf -O binary $(BINNAME).bin

# FIXME right now, all debugging/relocation information is thrown away
$(BINNAME).elf : $(THUMB_OBJS) $(ARM_OBJS) $(GAS_OBJS)
	$(CC) $(ARM_OBJS) $(THUMB_OBJS) $(GAS_OBJS) -o $@ $(LINKER_FLAGS)
	arm-eabi-strip -s -R .comment $@
	arm-eabi-size --format=sysv -x $@

$(THUMB_OBJS) : %.o : %.c FreeRTOSConfig.h
	$(CC) -c $(CFLAGS) -mthumb $< -o $@

$(ARM_OBJS) : %.o : %.c FreeRTOSConfig.h
	$(CC) -c $(CFLAGS) $< -o $@

$(GAS_OBJS) : %.o : %.s
	$(CC) -c $(GAS_FLAGS) $< -o $@

clean :
	rm -f $(THUMB_OBJS) $(ARM_OBJS) $(BINNAME).elf
	

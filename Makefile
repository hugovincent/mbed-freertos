# For mbed beta hardware (LPC2368)
# Hugo Vincent, April 25 2010

# Note: after installing an arm-eabi-* toolchain using the instructions at
# http://github.com/hugovincent/arm-eabi-toolchain, run setup-colorgcc.sh in util

TOOLPRE=util/arm-eabi
LDSCRIPT=util/lpc2368.ld

DEBUG=
OPTIM=-Os

BINNAME=RTOSDemo

COMMON_FLAGS = \
		$(DEBUG) \
		$(OPTIM) \
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
		-Wall -Wcast-align -Wimplicit -Wpointer-arith \
		-Wswitch -Wreturn-type -Wshadow -Wunused \
		-fno-strict-aliasing \
		-ffunction-sections -fdata-sections \
		-mabi=aapcs -mapcs-frame -mfloat-abi=soft

CFLAGS = $(COMMON_FLAGS) \
		-Wstrict-prototypes \
		-std=gnu99

CXXFLAGS= $(COMMON_FLAGS) \
		-fno-rtti -fno-exceptions

LINKER_FLAGS= \
		-nostartfiles \
		-T$(LDSCRIPT) \
		-Wl,--gc-sections \
		-lm -lstdc++

ASM_FLAGS= \
		-mcpu=arm7tdmi \
		-mthumb-interwork \
		-mabi=aapcs -mfloat-abi=soft \
		-x assembler-with-cpp 

THUMB_SOURCE= \
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
		lib/uart/uart0.c \
		lib/uart/FractionalBaud.c \
		freertos/list.c \
		freertos/queue.c \
		freertos/tasks.c \
		freertos/portable/GCC/ARM7_LPC23xx/port.c \
		lib/heap_2.c \
		lib/alloc.c \
		lib/syscalls.c

THUMB_CXX_SOURCE= \
		main.cpp \
		CxxTest.cpp

ARM_SOURCE= \
		freertos/portable/GCC/ARM7_LPC23xx/portISR.c \
		lib/webserver/EMAC_ISR.c \
		lib/uart/uart0ISR.c

ASM_SOURCE= \
		util/crt0.s

THUMB_C_OBJS   = $(THUMB_SOURCE:.c=.o)
THUMB_CXX_OBJS = $(THUMB_CXX_SOURCE:.cpp=.o)
ARM_C_OBJS     = $(ARM_SOURCE:.c=.o)
ARM_CXX_OBJS   = $(ARM_CXX_SOURCE:.cpp=.o)

ARM_OBJS   = $(ARM_C_OBJS) $(ARM_CXX_OBJS)
THUMB_OBJS = $(THUMB_C_OBJS) $(THUMB_CXX_OBJS)
ASM_OBJS   = $(ASM_SOURCE:.s=.o)

all: $(BINNAME).bin

$(BINNAME).bin : $(BINNAME).elf
	@echo "  [Converting to binary ] $(BINNAME).bin"
	@cp $(BINNAME).elf $(BINNAME)-stripped.elf
	@$(TOOLPRE)-strip -s -R .comment $(BINNAME)-stripped.elf
	@$(TOOLPRE)-size --format=sysv $(BINNAME)-stripped.elf | grep Total | awk '{print "   -> Total size (bytes):", $$2}'
	@$(TOOLPRE)-objcopy $(BINNAME)-stripped.elf -O binary $(BINNAME).bin
	@rm $(BINNAME)-stripped.elf

$(BINNAME).elf : $(THUMB_OBJS) $(ARM_OBJS) $(ASM_OBJS)
	@echo "  [Linking and stripping] $@"
	@$(TOOLPRE)-gcc $(ARM_OBJS) $(THUMB_OBJS) $(ASM_OBJS) -o $@ $(LINKER_FLAGS)

$(THUMB_C_OBJS) : %.o : %.c FreeRTOSConfig.h
	@echo "  [Compiling   (Thumb/C)] $<"
	@$(TOOLPRE)-gcc -c $(CFLAGS) -mthumb $< -o $@

$(THUMB_CXX_OBJS) : %.o : %.cpp FreeRTOSConfig.h
	@echo "  [Compiling (Thumb/C++)] $<"
	@$(TOOLPRE)-g++ -c $(CXXFLAGS) -mthumb $< -o $@

$(ARM_C_OBJS) : %.o : %.c FreeRTOSConfig.h
	@echo "  [Compiling     (ARM/C)] $<"
	@$(TOOLPRE)-gcc -c $(CFLAGS) $< -o $@

$(ARM_CXX_OBJS) : %.o : %.cpp FreeRTOSConfig.h
	@echo "  [Compiling   (ARM/C++)] $<"
	@$(TOOLPRE)-g++ -c $(CXXFLAGS) $< -o $@

$(ASM_OBJS) : %.o : %.s
	@echo "  [Assembling  (ARM/asm)] $<"
	@$(TOOLPRE)-gcc -c $(ASM_FLAGS) $< -o $@

clean :
	@echo "  [Cleaning...          ]"
	@rm -f $(THUMB_OBJS) $(ARM_OBJS) $(ASM_OBJS) $(BINNAME).elf $(BINNAME).bin
	

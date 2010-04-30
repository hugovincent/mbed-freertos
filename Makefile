# For mbed beta hardware (LPC2368)
# Hugo Vincent, April 25 2010

# Note: after installing an arm-eabi-none* toolchain using the instructions at
# http://github.com/hugovincent/arm-eabi-toolchain, run setup-colorgcc.sh in util/

TOOLPRE=util/arm-none-eabi
LDSCRIPT=util/lpc2368.ld

DEBUG=
OPTIM=-O2

BINNAME=RTOSDemo

COMMON_FLAGS = \
		$(DEBUG) \
		$(OPTIM) \
		-I . \
		-I include \
		-I freertos/include \
		-I freertos/portable/GCC/ARM7_LPC23xx \
		-D MBED_LPC23xx \
		-D THUMB_INTERWORK \
		-D NDEBUG \
		-mcpu=arm7tdmi \
		-fomit-frame-pointer \
		-mthumb-interwork \
		-Wall -Wcast-align -Wimplicit -Wpointer-arith \
		-Wswitch -Wreturn-type -Wshadow -Wunused \
		-fno-strict-aliasing \
		-ffunction-sections -fdata-sections \
		-mfloat-abi=soft -mtp=soft

CFLAGS = $(COMMON_FLAGS) \
		-Wstrict-prototypes \
		-std=gnu99

CXXFLAGS= $(COMMON_FLAGS) \
		-fno-rtti \
		-fno-exceptions \
		-fno-unwind-tables \
		-fno-enforce-eh-specs \
		-fno-use-cxa-get-exception-ptr \
		-fno-stack-protector \
		-Weffc++ \
		-nostdlib -nostartfiles -nodefaultlibs -nostdinc++

LINKER_FLAGS= \
		-nostartfiles \
		-T$(LDSCRIPT) \
		-Wl,--gc-sections \
		-Wl,--Map=$(BINNAME).map \
		-lm

ASM_FLAGS= \
		-mcpu=arm7tdmi \
		-mthumb-interwork \
		-x assembler-with-cpp 

THUMB_SOURCE= \
		example_tasks/BlockQ.c \
		example_tasks/blocktim.c \
		example_tasks/flash.c \
		example_tasks/integer.c \
		example_tasks/GenQTest.c \
		example_tasks/QPeek.c \
		example_tasks/dynamic.c \
		webserver/uIP_Task.c \
		lib/uip/uip_arp.c \
		lib/uip/psock.c \
		lib/uip/timer.c \
		lib/uip/uip.c \
		lib/uip/httpd.c \
		lib/uip/httpd-cgi.c \
		lib/uip/httpd-fs.c \
		lib/uip/http-strings.c \
		hardware/uart/uart.c \
		hardware/uart/uartFractionalBaud.c \
		hardware/gpio/gpio.c \
		hardware/emac/emac.c \
		freertos/list.c \
		freertos/queue.c \
		freertos/tasks.c \
		freertos/portable/GCC/ARM7_LPC23xx/port.c \
		lib/syscalls.c

THUMB_CXX_SOURCE= \
		main.cpp \
		lib/min_c++.cpp \
		CxxTest.cpp

ARM_SOURCE= \
		freertos/portable/GCC/ARM7_LPC23xx/portISR.c \
		hardware/emac/emacISR.c \
		hardware/uart/uartISRs.c

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
	@$(TOOLPRE)-objcopy $(BINNAME).elf -O binary $(BINNAME).bin
	@python util/memory-usage.py $(BINNAME).elf

$(BINNAME).elf : $(THUMB_OBJS) $(ARM_OBJS) $(ASM_OBJS)
	@echo "  [Linking...           ] $@"
	@$(TOOLPRE)-gcc $(ARM_OBJS) $(THUMB_OBJS) $(ASM_OBJS) -o $@ $(LINKER_FLAGS)

$(THUMB_C_OBJS) : %.o : %.c
	@echo "  [Compiling   (Thumb/C)] $<"
	@$(TOOLPRE)-gcc -c $(CFLAGS) -mthumb $< -o $@

$(THUMB_CXX_OBJS) : %.o : %.cpp
	@echo "  [Compiling (Thumb/C++)] $<"
	@$(TOOLPRE)-g++ -c $(CXXFLAGS) -mthumb $< -o $@

$(ARM_C_OBJS) : %.o : %.c
	@echo "  [Compiling     (ARM/C)] $<"
	@$(TOOLPRE)-gcc -c $(CFLAGS) $< -o $@

$(ARM_CXX_OBJS) : %.o : %.cpp
	@echo "  [Compiling   (ARM/C++)] $<"
	@$(TOOLPRE)-g++ -c $(CXXFLAGS) $< -o $@

$(ASM_OBJS) : %.o : %.s
	@echo "  [Assembling  (ARM/asm)] $<"
	@$(TOOLPRE)-gcc -c $(ASM_FLAGS) $< -o $@


.PHONY: disasm clean
disasm:
	@echo "  [Disassembling binary ] $(BINNAME)-disassembled.s"
	@$(TOOLPRE)-objdump --disassemble $(BINNAME).elf > $(BINNAME)-disassembled.s

clean :
	@echo "  [Cleaning...          ]"
	@rm -f $(THUMB_OBJS) $(ARM_OBJS) $(ASM_OBJS) $(BINNAME).elf $(BINNAME).bin $(BINNAME).map $(BINNAME)-disassembled.s
	

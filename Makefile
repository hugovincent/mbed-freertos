# For mbed <http://www.mbed.org>
# Hugo Vincent, April 25 2010
#
# Note: after installing an arm-eabi-none* toolchain using the instructions at
# http://github.com/hugovincent/arm-eabi-toolchain, run setup-colorgcc.sh in util/
#
# Targets you can Make:
#	all		- Build all code and produce a binary suitable for installation.
#	install	- Install to an attached mbed device. Set INSTALL_PATH below to suit.
#	clean	- Delete all temporary build products.
#	dep		- Compute interdependecies between source files. Useful if you're
#			  hacking on the source (otherwise you need to make clean && make).
#	disasm	- Produce a disassembly listing of the whole program.
#

# Set target here according to which type of mbed you have:
# (can be lpc2368 for older mbeds, or lpc1768 for newer ones)
TARGET=lpc2368
LDSCRIPT=hardware/cpu-$(TARGET)/$(TARGET).ld
BINNAME=RTOSDemo

ODIR=.buildtmp
INSTALL_PATH=/Volumes/MBED/
TOOLPRE=util/arm-none-eabi

#------------------------------------------------------------------------------
# Stuff specific to LPC2368 target:
ifeq ($(TARGET), lpc2368)
CPUFLAGS= \
		-mcpu=arm7tdmi -march=armv4t \
		-DTHUMB_INTERWORK -mthumb-interwork
COMMON_FLAGS= \
		-DMBED_LPC23xx -DTARGET_LPC2368 -DPLAT_NAME="\"mbed (LPC2368)\"" \
		-Iinclude/LPC2368
PORT_DIR= \
		ARM7_LPC23xx
ARM_SOURCE= \
		freertos/portable/GCC/$(PORT_DIR)/portISR.c 
endif

#------------------------------------------------------------------------------
# Stuff specific to LPC1768 target:
ifeq ($(TARGET), lpc1768)
CPUFLAGS= \
		-mcpu=cortex-m3 -march=armv7-m \
		-mthumb
COMMON_FLAGS= \
		-DMBED_LPC17xx -DTARGET_LPC1768 -DPLAT_NAME="\"mbed (LPC1768)\"" \
		-Iinclude/LPC1768
PORT_DIR= \
		ARM_CM3
endif

#------------------------------------------------------------------------------
# Compiler, Assembler and Linker Options:

DEBUG=-DNDEBUG=1
OPTIM=-O2

COMMON_FLAGS += \
		$(CPUFLAGS) \
		$(DEBUG) \
		$(OPTIM) \
		-I . \
		-I include \
		-I freertos/include \
		-I freertos/portable/GCC/$(PORT_DIR) \
		-I lib/ustl \
		-fomit-frame-pointer \
		-Wall -Wcast-align -Wimplicit -Wpointer-arith \
		-Wswitch -Wreturn-type -Wshadow -Wunused \
		-fno-strict-aliasing \
		-ffunction-sections -fdata-sections \
		-mfloat-abi=soft -mtp=soft -mabi=aapcs

CFLAGS = $(COMMON_FLAGS) \
		-Wstrict-prototypes \
		-std=gnu99

CXXFLAGS= $(COMMON_FLAGS) \
		-fno-rtti \
		-fno-unwind-tables \
		-fno-enforce-eh-specs \
		-fno-use-cxa-get-exception-ptr \
		-fno-stack-protector \
		-Weffc++

LINKER_FLAGS= \
		-nostartfiles -nostdinc++ \
		-T$(LDSCRIPT) \
		-Wl,--gc-sections \
		-Wl,-Map=$(BINNAME).map \
		-mabi=aapcs \
		-lm -lsupc++

ASM_FLAGS= \
		$(CPUFLAGS) \
		-x assembler-with-cpp 

#------------------------------------------------------------------------------
# Source Code:

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
		hardware/peripherals/uart/uart.c \
		hardware/peripherals/uart/uart_fractional_baud.c \
		hardware/peripherals/gpio/gpio.c \
		hardware/peripherals/emac/emac.c \
		hardware/cpu-$(TARGET)/device_init.c \
		hardware/board-mbed/board_init.c \
		freertos/list.c \
		freertos/queue.c \
		freertos/tasks.c \
		freertos/portable/GCC/$(PORT_DIR)/port.c \
		freertos/portable/MemMang/heap_3.c \
		lib/exception_handlers.c \
		lib/syscalls.c

THUMB_CXX_SOURCE= \
		Main.cpp \
		tests/CxxTest.cpp \
		tests/Tests.cpp \
		hardware/peripherals/wdt/wdt.cpp \
		lib/min_c++.cpp

ARM_SOURCE+= \
		hardware/peripherals/emac/emacISR.c \
		hardware/peripherals/uart/uartISRs.c

ARM_ASM_SOURCE= \
		hardware/cpu-$(TARGET)/crt0.s

# Include uSTL files:
#include lib/ustl/ustl.mk

#------------------------------------------------------------------------------
# Build Rules:

THUMB_C_OBJS   = $(patsubst %.c,$(ODIR)/%.o, $(THUMB_SOURCE))
THUMB_CXX_OBJS = $(patsubst %.cpp,$(ODIR)/%.o, $(THUMB_CXX_SOURCE))
ARM_C_OBJS     = $(patsubst %.c,$(ODIR)/%.o, $(ARM_SOURCE))
ARM_CXX_OBJS   = $(patsubst %.cpp,$(ODIR)/%.o, $(ARM_CXX_SOURCE))
ARM_ASM_OBJS   = $(patsubst %.s,$(ODIR)/%.o, $(ARM_ASM_SOURCE))

ARM_OBJS       = $(ARM_C_OBJS) $(ARM_CXX_OBJS)
THUMB_OBJS     = $(THUMB_C_OBJS) $(THUMB_CXX_OBJS)

all: $(BINNAME).bin

# Binary suitable for installation on mbed
$(BINNAME).bin : $(BINNAME).elf
	@echo "  [Converting to binary ] $(BINNAME).bin"
	@$(TOOLPRE)-objcopy $(BINNAME).elf -O binary $(BINNAME).bin
	@python util/memory-usage.py $(BINNAME).elf
	@echo

# ELF file (intermediate linking product, also used for various checks)
$(BINNAME).elf : $(THUMB_OBJS) $(ARM_OBJS) $(ARM_ASM_OBJS) 
	@echo "  [Linking...           ] $@"
	@$(TOOLPRE)-gcc $(ARM_OBJS) $(THUMB_OBJS) $(ARM_ASM_OBJS) -o $@ $(LINKER_FLAGS)

# Thumb-mode C/C++ Code:
$(THUMB_C_OBJS) : $(ODIR)/%.o : %.c $(ODIR)/exists
	@echo "  [Compiling   (Thumb/C)] $<"
	@$(TOOLPRE)-gcc -c $(CFLAGS) -mthumb $< -o $@

$(THUMB_CXX_OBJS) : $(ODIR)/%.o : %.cpp $(ODIR)/exists
	@echo "  [Compiling (Thumb/C++)] $<"
	@$(TOOLPRE)-g++ -c $(CXXFLAGS) -mthumb $< -o $@

# ARM-mode C/C++ Code:
$(ARM_C_OBJS) : $(ODIR)/%.o : %.c $(ODIR)/exists
	@echo "  [Compiling     (ARM/C)] $<"
	@$(TOOLPRE)-gcc -c $(CFLAGS) $< -o $@

$(ARM_CXX_OBJS) : $(ODIR)/%.o : %.cpp $(ODIR)/exists
	@echo "  [Compiling   (ARM/C++)] $<"
	@$(TOOLPRE)-g++ -c $(CXXFLAGS) $< -o $@

# ARM Assembler Code:
$(ARM_ASM_OBJS) : $(ODIR)/%.o : %.s hardware/cpu-common/crt0.s $(ODIR)/exists
	@echo "  [Assembling  (ARM/asm)] $<"
	@$(TOOLPRE)-gcc -c $(ASM_FLAGS) $< -o $@

# This target ensures the temporary build product directories exist
$(ODIR)/exists:
	@mkdir -p $(ODIR)/hardware/peripherals/uart $(ODIR)/hardware/peripherals/gpio 
	@mkdir -p $(ODIR)/hardware/peripherals/emac $(ODIR)/hardware/peripherals/wdt
	@mkdir -p $(ODIR)/hardware/board-mbed $(ODIR)/freertos/portable/MemMang
	@mkdir -p $(ODIR)/hardware/cpu-$(TARGET) $(ODIR)/tests
	@mkdir -p $(ODIR)/example_tasks $(ODIR)/webserver $(ODIR)/lib/uip $(ODIR)/lib/ustl
	@mkdir -p $(ODIR)/freertos/portable/GCC/$(PORT_DIR)
	@touch $(ODIR)/exists

#------------------------------------------------------------------------------
# Psuedo-targets:

.PHONY: disasm clean install
disasm :
	@echo "  [Disassembling binary ] $(BINNAME)-disassembled.s"
	@$(TOOLPRE)-objdump --disassemble $(BINNAME).elf > $(BINNAME)-disassembled.s

clean:
	@echo "  [Cleaning...          ]"
	@rm -rf $(ODIR) $(BINNAME).elf $(BINNAME).bin $(BINNAME)-disassembled.s

install: $(BINNAME).bin
	@echo "  [Installing to mbed...]"
	@cp $(BINNAME).bin $(INSTALL_PATH)
	@echo "  [Done.                ]"

#------------------------------------------------------------------------------
# Dependency Management (run make dep to generate, otherwise ignored)

DEPS_C         = $(patsubst %.c,$(ODIR)/%.d, $(ARM_SOURCE) $(THUMB_SOURCE))
DEPS_CXX       = $(patsubst %.cpp,$(ODIR)/%.d, $(ARM_CXX_SOURCE) $(THUMB_CXX_SOURCE))

.PHONY: dep
dep : $(DEPS_C) $(DEPS_CXX)

$(DEPS_C) : $(ODIR)/%.d : %.c $(ODIR)/exists
	@echo "  [Finding dependencies ] $<"
	@$(TOOLPRE)-gcc -MM $(CFLAGS) -MT $(patsubst %.c,$(ODIR)/%.o, $<) $< -MF $@

$(DEPS_CXX) : $(ODIR)/%.d : %.cpp $(ODIR)/exists
	@echo "  [Finding dependencies ] $<"
	@$(TOOLPRE)-g++ -MM $(CXXFLAGS) -MT $(patsubst %.cpp,$(ODIR)/%.o, $<) $< -MF $@

-include $(shell find . -name "*.d")


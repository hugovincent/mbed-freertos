# Hugo Vincent, April 25 2010
#
# Note: after installing an arm-eabi-none* toolchain using the instructions at
# http://github.com/hugovincent/arm-eabi-toolchain, run setup-colorgcc.sh in util/
#
# Targets you can Make:
#	all		- Build all code and produce a binary suitable for installation.
#	install	- Install to an attached device. Set PROG_TYPE below to suit.
#	clean	- Delete all temporary build products.
#	dep		- Compute interdependecies between source files. Useful if you're
#			  hacking on the source (otherwise you need to make clean && make).
#	disasm	- Produce a disassembly listing of the whole program.
#

# Set CPU type here (can be lpc2368, lpc1768, lpc2929, or efm32):
TARGET=lpc1768

# Set board type here (can be mbed or efm32-olimex-stk):
BOARD=mbed

# Set programming method here (can be mbed, openocd or serial_isp):
PROG_TYPE=mbed
#ISP_OPT=/dev/tty.usbserial-isp 115200 12000

# Set local options here:
INSTALL_PATH=/Volumes/MBED/
BINNAME=freertos-$(TARGET)
TOOLPRE=util/arm-none-eabi

#------------------------------------------------------------------------------
# Stuff specific to LPC2368 target:
ifeq ($(TARGET), lpc2368)
CPUFLAGS= \
		-mcpu=arm7tdmi-s \
		-mthumb-interwork
COMMON_FLAGS= \
		-DTARGET_LPC23xx \
		-DPLAT_NAME="\"LPC2368\"" \
		-Iinclude/LPC2368
PORT_DIR= \
		ARM7_LPC23xx
ASM_SOURCE= \
		mach/cpu-lpc2368/crt0.s
C_SOURCE= \
		mach/cpu-lpc2368/system_LPC23xx.c \
		mach/cpu-lpc2368/exception_handlers.c
endif

#------------------------------------------------------------------------------
# Stuff specific to LPC1768 target:
ifeq ($(TARGET), lpc1768)
CPUFLAGS= \
		-mcpu=cortex-m3 \
		-mthumb
COMMON_FLAGS= \
		-DTARGET_LPC17xx \
		-DPLAT_NAME="\"LPC1768\"" \
		-DUSE_PROCESS_STACK \
		-Iinclude/LPC1768
PORT_DIR= \
		ARM_CM3
EXTRA_LDFLAGS= \
		-mcpu=cortex-m3 \
		-mthumb
C_SOURCE= \
		mach/cpu-lpc1768/core_cm3.c \
		mach/cpu-lpc1768/system_LPC17xx.c \
		mach/cpu-lpc1768/crt0.c \
		mach/cpu-lpc1768/fault_handlers.c
endif

#------------------------------------------------------------------------------
# Stuff specific to EFM32 targets:
ifeq ($(TARGET), efm32)
CPUFLAGS= \
		-mcpu=cortex-m3 \
		-mthumb
COMMON_FLAGS= \
		-DTARGET_EFM32 \
		-DCORE_HAS_MPU \
		-DEFM32G230F128 \
		-DPLAT_NAME="\"EFM32G230F128\"" \
		-DUSE_PROCESS_STACK \
		-Iinclude/EFM32
PORT_DIR= \
		ARM_CM3_MPU
EXTRA_LDFLAGS= \
		-mcpu=cortex-m3 \
		-mthumb
C_SOURCE= \
		mach/cpu-efm32/core_cm3.c \
		mach/cpu-efm32/system_efm32.c \
		mach/cpu-efm32/crt0.c \
		mach/cpu-efm32/fault_handlers.c
endif

#------------------------------------------------------------------------------
# Stuff specific to LPC2929 target:
ifeq ($(TARGET), lpc2929)
CPUFLAGS= \
		-mcpu=arm968e-s \
		-mthumb-interwork
COMMON_FLAGS= \
		-DTARGET_LPC29xx \
		-DPLAT_NAME="\"LPC2929\"" \
		-Iinclude/LPC2929
PORT_DIR= \
		ARM9_LPC29xx
ASM_SOURCE= \
		mach/cpu-lpc2929/crt0.s
C_SOURCE= \
		mach/cpu-lpc2929/system_LPC29xx.c \
		mach/cpu-lpc2929/exception_handlers.c
endif


#------------------------------------------------------------------------------
# Compiler, Assembler and Linker Options:

DEBUG=-DNDEBUG=1 -gdwarf-2
OPTIM=-O2
ODIR=.buildtmp
LDSCRIPT=$(ODIR)/mach/cpu-$(TARGET)/$(TARGET).ld.S

COMMON_FLAGS += \
		$(CPUFLAGS) \
		$(DEBUG) \
		$(OPTIM) \
		-I . \
		-I include \
		-I kernel/include \
		-I kernel/port/$(PORT_DIR) \
		-Wall -Wimplicit -Wpointer-arith -Wcast-align \
		-Wswitch -Wreturn-type -Wshadow -Wunused -Wstrict-aliasing \
		-fexceptions -fsection-anchors -fomit-frame-pointer \
		-ffunction-sections -fdata-sections \
		-fstrict-aliasing -mlong-calls \
		-mfloat-abi=soft -mtp=soft -mabi=aapcs -fshort-wchar

CFLAGS = $(COMMON_FLAGS) \
		-std=gnu99

CXXFLAGS = $(COMMON_FLAGS) \
		-I lib/ustl/public -nostdinc++ \
		-fno-enforce-eh-specs \
		-fno-use-cxa-get-exception-ptr \
		-fno-stack-protector

LINKER_FLAGS += \
		-T$(LDSCRIPT) $(EXTRA_LDFLAGS) \
		-Wl,--gc-sections -Wl,-O3 \
		-Wl,-Map=$(BINNAME).map \
		-mabi=aapcs -static -nodefaultlibs

EXTRA_CLEAN=$(BINNAME).map $(BINNAME)-disassembled.s

LIBS = \
		-Wl,--start-group -lgcc -lc -lm -lsupc++ -Wl,--end-group

ASM_FLAGS= \
		$(CPUFLAGS) \
		-x assembler-with-cpp

all: $(BINNAME).bin

#------------------------------------------------------------------------------
# Source Code:

# Core Operating System
C_SOURCE+= \
		mach/board-$(BOARD)/board_init.c \
		mach/cpu-$(TARGET)/power_management.c \
		kernel/list.c \
		kernel/queue.c \
		kernel/tasks.c \
		kernel/port/$(PORT_DIR)/port.c \
		kernel/malloc_wrappers.c \
		lib/cmsis_nvic.c \
		lib/debug_support.c \
		lib/device_manager.c \
		lib/freertos_hooks.c \
		lib/semifs.c \
		lib/romfs.c \
		lib/console.c \
		lib/os_init.c \
		lib/task_manager.c
CXX_SOURCE+= \
		Main.cpp

# C/C++ library and operating system calls
include lib/clibrary.mk
#include lib/ustl/ustl.mk
include lib/uip/uip.mk

# Peripheral device drivers
include drivers/drivers.mk

# Example Tasks
include example_tasks/example_tasks.mk

# Applicatinos
include apps/apps.mk

# Tests
CXX_SOURCE+= \
		tests/Cxx_Test.cpp \
		tests/Malloc_Test.cpp \
		tests/FileIO_Test.cpp \
		tests/Debug_Abort_Test.cpp

#------------------------------------------------------------------------------
# Build Rules:

C_OBJS     = $(patsubst %.c,$(ODIR)/%.o, $(C_SOURCE))
CXX_OBJS   = $(patsubst %.cpp,$(ODIR)/%.o, $(CXX_SOURCE))
ASM_OBJS   = $(patsubst %.s,$(ODIR)/%.o, $(ASM_SOURCE))

OBJS       = $(C_OBJS) $(CXX_OBJS) $(ASM_OBJS)

# Binary suitable for installation on mbed
$(BINNAME).bin : $(BINNAME).elf
	@echo "  [Converting to binary    ] $(BINNAME).bin"
	@$(TOOLPRE)-objcopy $(BINNAME).elf -O binary $(BINNAME).bin
	@python util/memory-usage.py $(TARGET) $(BINNAME).elf
	@echo

# ELF file (intermediate linking product, also used for various checks)
$(BINNAME).elf : $(OBJS) $(LDSCRIPT)
	@echo "  [Linking...              ] $@"
	@$(TOOLPRE)-gcc $(LINKER_FLAGS) $(OBJS) $(LIBS) -o $@

# C/C++ Code:
$(C_OBJS) : $(ODIR)/%.o : %.c $(ODIR)/exists
	@echo "  [Compiling  (C)          ] $<"
	@$(TOOLPRE)-gcc -c $(CFLAGS) $< -o $@

$(CXX_OBJS) : $(ODIR)/%.o : %.cpp $(ODIR)/exists
	@echo "  [Compiling (C++)         ] $<"
	@$(TOOLPRE)-g++ -c $(CXXFLAGS) $< -o $@

# ARM Assembler Code:
$(ASM_OBJS) : $(ODIR)/%.o : %.s $(ODIR)/exists
	@echo "  [Assembling (asm)        ] $<"
	@$(TOOLPRE)-gcc -c $(ASM_FLAGS) $< -o $@

# Linker script preprocessing
$(LDSCRIPT) : mach/cpu-$(TARGET)/$(TARGET).ld.S util/arm_common.ld.S util/arm_common_macros.ld.h
	@echo "  [Preprocessing ld script ] $<"
	@$(TOOLPRE)-cpp -P -I. $< $@

# This target ensures the temporary build product directories exist
$(ODIR)/exists:
	@mkdir -p $(ODIR)/drivers/uart $(ODIR)/drivers/gpio $(ODIR)/drivers/rtc
	@mkdir -p $(ODIR)/drivers/emac $(ODIR)/drivers/wdt $(ODIR)/drivers/gpdma
	@mkdir -p $(ODIR)/drivers/xbee
	@mkdir -p $(ODIR)/mach/board-$(BOARD) $(ODIR)/mach/cpu-$(TARGET)
	@mkdir -p $(ODIR)/kernel $(ODIR)/mach/cpu-common $(ODIR)/kernel/port/$(PORT_DIR)
	@mkdir -p $(ODIR)/example_tasks $(ODIR)/apps/webserver $(ODIR)/lib/uip
	@mkdir -p $(ODIR)/lib/ustl $(ODIR)/tests $(ODIR)/lib/syscalls
	@touch $(ODIR)/exists

# RomFS script build rules
lib/romfs_data.h: util/build_romfs.py romfs/*
	@echo "  [Packaging RomFS data    ]"
	@python util/build_romfs.py lib/romfs_data.h romfs/

lib/romfs.c: lib/romfs_data.h

#------------------------------------------------------------------------------
# Psuedo-targets:

.PHONY: disasm clean install
disasm :
	@echo "  [Disassembling binary    ] $(BINNAME)-disassembled.s"
	@$(TOOLPRE)-objdump -d $(BINNAME).elf > $(BINNAME)-disassembled.s

clean:
	@echo "  [Cleaning...             ]"
	@rm -rf $(ODIR) $(BINNAME).elf $(BINNAME).bin
	@rm -rf $(EXTRA_CLEAN)

install: $(BINNAME).bin
ifeq ($(PROG_TYPE), mbed)
	@echo "  [Installing to mbed...   ]"
	@cp $(BINNAME).bin $(INSTALL_PATH)
	@echo "  [Done.                   ]"
else ifeq ($(PROG_TYPE), serial_isp)
	@echo "  [Installing by ISP...    ]"
	@./util/lpc21isp/lpc21isp -wipe -bin $(BINNAME).bin $(ISP_OPT)
	@echo "  [Done.                   ]"
endif

#------------------------------------------------------------------------------
# Dependency Management (run make dep to generate, otherwise ignored)

DEPS_C         = $(patsubst %.c,$(ODIR)/%.d, $(C_SOURCE))
DEPS_CXX       = $(patsubst %.cpp,$(ODIR)/%.d, $(CXX_SOURCE))

.PHONY: dep
dep : $(DEPS_C) $(DEPS_CXX)

$(DEPS_C) : $(ODIR)/%.d : %.c $(ODIR)/exists
	@echo "  [Finding dependencies    ] $<"
	@$(TOOLPRE)-gcc -MM $(CFLAGS) -MT $(patsubst %.c,$(ODIR)/%.o, $<) $< -MF $@

$(DEPS_CXX) : $(ODIR)/%.d : %.cpp $(ODIR)/exists
	@echo "  [Finding dependencies    ] $<"
	@$(TOOLPRE)-g++ -MM $(CXXFLAGS) -MT $(patsubst %.cpp,$(ODIR)/%.o, $<) $< -MF $@

-include $(shell find . -name "*.d")


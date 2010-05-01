@ crt0.s: startup file for NXP LPC230x Family Microprocessors
@
@ Copyright(C) 2006, NXP Semiconductor
@ All rights reserved.
@ Copyright (C) 2010, Hugo Vincent <hugo.vincent@gmail.com>
@
@ History
@     2006.09.01  ver 1.00    Prelimnary version, first Release
@
@ modified by Martin Thomas:
@  - changed comment char to @ for GNU assembler (arm-elf-as)
@    "The line comment character is [...] '@' on the ARM [...]"
@    (GNU-as manual from binutils V 2.17)
@  - ported RV-ASM to GNU-ASM (sections, import/export equ, 
@    :OR:, Labels etc.)
@  - added .data and .bss inits
@  - call main instead for "RV runtime-init" __main
@  - call main with bx so it can be thumb-code
@  - added IRQ-Wrapper for easier porting of IAR, RV etc. ISR-codes.
@  - TargetResetInit can be a thumb-function (using BX instead of BL)
@  - moved TargetResetInit call before the stack-inits and use 
@    a temporary stack-pointer
@
@ modified by Hugo Vincent:
@  - style changes
@  - removed a lot of commented-out stuff that we don't need
@  - removed IRQ-Wrapper (need to use the FreeRTOS macros)
@  

@  ----------------------------------------------------------------------------
@ Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs

.equ Mode_USR,           0x10
.equ Mode_FIQ,           0x11
.equ Mode_IRQ,           0x12
.equ Mode_SVC,           0x13
.equ Mode_ABT,           0x17
.equ Mode_UND,           0x1B
.equ Mode_SYS,           0x1F

.equ I_Bit,              0x80            @ when I bit is set, IRQ is disabled
.equ F_Bit,              0x40            @ when F bit is set, FIQ is disabled

@  ----------------------------------------------------------------------------
@  Stack configuration definitions (sizes in bytes) 

.equ UND_Stack_Size,     0x00000004
.equ SVC_Stack_Size,     0x00000400
.equ ABT_Stack_Size,     0x00000004
.equ FIQ_Stack_Size,     0x00000004
.equ IRQ_Stack_Size,     0x00000200
.equ USR_Stack_Size,     0x00000004

.global Stack_Size_Total
.equ Stack_Size_Total,   (UND_Stack_Size + SVC_Stack_Size + ABT_Stack_Size \
                          + FIQ_Stack_Size + IRQ_Stack_Size + USR_Stack_Size)

@  ----------------------------------------------------------------------------
@  Exception vectors (mapped to 0x0 by linker script)
@  Notes: * Absolute addressing mode must be used.
@         * Dummy Handlers are implemented as infinite loops (can be modified).

.section .vectors, "ax"
.arm
.global Vectors
Vectors:        LDR     pc, Reset_Addr
                LDR     pc, Undef_Addr
                LDR     pc, SWI_Addr
                LDR     pc, PAbt_Addr
                LDR     pc, DAbt_Addr
                NOP                            @ Reserved Vector 
                LDR     pc, [pc, #0xFFFFFF00]  @ Vector from VicVectAddr
                LDR     pc, FIQ_Addr

Reset_Addr:     .word     Reset_Handler
Undef_Addr:     .word     Undef_Handler
SWI_Addr:       .word     SWI_Handler
PAbt_Addr:      .word     PAbt_Handler
DAbt_Addr:      .word     DAbt_Handler
FIQ_Addr:       .word     FIQ_Handler

@  ----------------------------------------------------------------------------
@  Handlers for conditions we don't use (infinite loops)

Undef_Handler:  B       Undef_Handler
PAbt_Handler:   B       PAbt_Handler
DAbt_Handler:   B       DAbt_Handler
FIQ_Handler:    B       FIQ_Handler

@ The SWI is used for YieldProcessor in FreeRTOS.
SWI_Handler:    B       vPortYieldProcessor

@  ----------------------------------------------------------------------------
@  Reset Handler: setup stacks, copy/initialise data, and jump to C main.

.global Reset_handler
Reset_Handler:
                LDR     R0, =__top_of_stack__

@  Enter Undefined Instruction Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_UND | I_Bit | F_Bit
                MOV     SP, R0
                SUB     R0, R0, #UND_Stack_Size

@  Enter Abort Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_ABT | I_Bit | F_Bit
                MOV     SP, R0
                SUB     R0, R0, #ABT_Stack_Size

@  Enter FIQ Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_FIQ | I_Bit | F_Bit
                MOV     SP, R0
                SUB     R0, R0, #FIQ_Stack_Size

@  Enter IRQ Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_IRQ | I_Bit | F_Bit
                MOV     SP, R0
                SUB     R0, R0, #IRQ_Stack_Size

@  Enter Supervisor Mode and set its Stack Pointer
                MSR     CPSR_c, #Mode_SVC | I_Bit | F_Bit
                MOV     SP, R0
                SUB     SL, SP, #SVC_Stack_Size

@  For FreeRTOS, leave in Supervisor Mode with Interrupts disabled. 
@  We don't use User Mode with FreeRTOS so leave it's stack uninitialised.

@  ----------------------------------------------------------------------------
@  Relocate .data section (Copy from ROM to RAM)
                LDR     R1, =__text_end__ 
                LDR     R2, =__data_start__
                LDR     R3, =__data_end__ 
                CMP     R2, R3
                BEQ     DataIsEmpty
LoopReloc:      CMP     R2, R3 
                LDRLO   R0, [R1], #4 
                STRLO   R0, [R2], #4 
                BLO     LoopReloc
DataIsEmpty:
 
@  ----------------------------------------------------------------------------
@  Clear .bss section (Fill with zeros)

                MOV     R0, #0 
                LDR     R1, =__bss_start__ 
                LDR     R2, =__bss_end__ 
                CMP     R1,R2
                BEQ     BSSIsEmpty
LoopFill:       CMP     R1, R2 
                STRLO   R0, [R1], #4 
                BLO     LoopFill
BSSIsEmpty:

@  ----------------------------------------------------------------------------
@  Finally, enter the C code
                .extern main
                LDR R0, =main
                BX      R0

@  ----------------------------------------------------------------------------
@  If C main ever returns, reset the device
				LDR R0,	=Reset_Handler
				BX		R0

.end


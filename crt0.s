@/*****************************************************************************
@*   startup.s: startup file for NXP LPC230x Family Microprocessors
@*
@*   Copyright(C) 2006, NXP Semiconductor
@*   All rights reserved.
@*
@*   History
@*   2006.09.01  ver 1.00    Prelimnary version, first Release
@*
@*****************************************************************************/

@ modified by Martin Thomas:
@  - changed comment char to @ for GNU assembler (arm-elf-as)
@    "The line comment character is [...] ‘@’ on the ARM [...]"
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

@@	PRESERVE8

@/*
@ *  The STARTUP.S code is executed after CPU Reset. This file may be 
@ *  translated with the following SET symbols. In uVision these SET 
@ *  symbols are entered under Options - ASM - Define.
@ *
@ *  REMAP: when set the startup code initializes the register MEMMAP 
@ *  which overwrites the settings of the CPU configuration pins. The 
@ *  startup and interrupt vectors are remapped from:
@ *     0x00000000  default setting (not remapped)
@ *     0x40000000  when RAM_MODE is used
@ *
@ *  RAM_MODE: when set the device is configured for code execution
@ *  from on-chip RAM starting at address 0x40000000. 
@ */


@ Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs

.equ Mode_USR,            0x10
.equ Mode_FIQ,            0x11
.equ Mode_IRQ,            0x12
.equ Mode_SVC,            0x13
.equ Mode_ABT,            0x17
.equ Mode_UND,            0x1B
.equ Mode_SYS,            0x1F

.equ I_Bit,               0x80            @ when I bit is set, IRQ is disabled
.equ F_Bit,               0x40            @ when F bit is set, FIQ is disabled


@// <h> Stack Configuration (Stack Sizes in Bytes)
@//   <o0> Undefined Mode      <0x0-0xFFFFFFFF:8>
@//   <o1> Supervisor Mode     <0x0-0xFFFFFFFF:8>
@//   <o2> Abort Mode          <0x0-0xFFFFFFFF:8>
@//   <o3> Fast Interrupt Mode <0x0-0xFFFFFFFF:8>
@//   <o4> Interrupt Mode      <0x0-0xFFFFFFFF:8>
@//   <o5> User/System Mode    <0x0-0xFFFFFFFF:8>
@// </h>

.equ UND_Stack_Size,     0x00000004
.equ SVC_Stack_Size,     0x00000200
.equ ABT_Stack_Size,     0x00000004
.equ FIQ_Stack_Size,     0x00000004
.equ IRQ_Stack_Size,     0x00000100
.equ USR_Stack_Size,     0x00000004

.equ Stack_Size,        (UND_Stack_Size + SVC_Stack_Size + ABT_Stack_Size + \
                         FIQ_Stack_Size + IRQ_Stack_Size + USR_Stack_Size)

@@                AREA    STACK, NOINIT, READWRITE, ALIGN=3
@@
@@Stack_Mem       SPACE   Stack_Size
@@Stack_Top       EQU     Stack_Mem + Stack_Size

.arm
.section .STACK, "w"
.align 3
Stack_Mem:
.space Stack_Size
.equ Stack_Top, Stack_Mem + Stack_Size


@// <h> Heap Configuration
@//   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF>
@// </h>

@@Heap_Size       EQU     0x00000000
@@                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
@@Heap_Mem        SPACE   Heap_Size

.equ Heap_Size,   0x00000000
.section .HEAP, "w"
.align 3
HeapMem:
.if (Heap_Size>0)
.space Heap_Size
.endif


@ Area Definition and Entry Point
@  Startup Code must be linked first at Address at which it expects to run.

@@                AREA    RESET, CODE, READONLY
@@                ARM
.section .RESET, "ax"
.arm

@ Exception Vectors
@  Mapped to Address 0.
@  Absolute addressing mode must be used.
@  Dummy Handlers are implemented as infinite loops which can be modified.

Vectors:        LDR     PC, Reset_Addr         
                LDR     PC, Undef_Addr
                LDR     PC, SWI_Addr
                LDR     PC, PAbt_Addr
                LDR     PC, DAbt_Addr
                NOP                            @ Reserved Vector 
@@                LDR     PC, IRQ_Addr
                LDR     PC, [PC, #-0x0120]     @ Vector from VicVectAddr
                LDR     PC, FIQ_Addr

Reset_Addr:      .word     Reset_Handler
Undef_Addr:      .word     Undef_Handler
SWI_Addr:        .word     SWI_Handler
PAbt_Addr:       .word     PAbt_Handler
DAbt_Addr:       .word     DAbt_Handler
                 .word     0xB9206E28             @ Reserved Address 
IRQ_Addr:        .word     IRQ_Handler
FIQ_Addr:        .word     FIQ_Handler

IRQ_Handler:	 ldr 	pc, [pc, #0xFFFFFF00]  @ Read VIC

@@ Endless loops:
Undef_Handler:   B       Undef_Handler
SWI_Handler:     B       vPortYieldProcessor
PAbt_Handler:    B       PAbt_Handler
DAbt_Handler:    B       DAbt_Handler
FIQ_Handler:     B       FIQ_Handler


@ Reset Handler
@                EXPORT  Reset_Handler
.global Reset_handler
Reset_Handler:

@  Call low-level init C-function
@                IMPORT TargetResetInit
@                BL   TargetResetInit

@ handled in application
@                 .extern TargetResetInit
@                 ldr     SP, =Stack_Top    @ temporary stack at Stack_Top
@                 LDR R0, =TargetResetInit
@                 MOV LR, PC
@                 BX R0


@  Setup Stack for each mode
                LDR     R0, =Stack_Top

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
@                SUB     R0, R0, #SVC_Stack_Size
@leave in Supervisor Mode with Interrupts disabled

@
@  Enter User Mode and set its Stack Pointer
@                MSR     CPSR_c, #Mode_USR
@                MOV     SP, R0
@                SUB     SL, SP, #USR_Stack_Size

@  Relocate .data section (Copy from ROM to RAM)
                LDR     R1, =_etext 
                LDR     R2, =_data 
                LDR     R3, =_edata 
                CMP     R2, R3
                BEQ     DataIsEmpty
LoopRel:        CMP     R2, R3 
                LDRLO   R0, [R1], #4 
                STRLO   R0, [R2], #4 
                BLO     LoopRel 
DataIsEmpty:
 
@  Clear .bss section (Zero init)
                MOV     R0, #0 
                LDR     R1, =__bss_start__ 
                LDR     R2, =__bss_end__ 
                CMP     R1,R2
                BEQ     BSSIsEmpty
LoopZI:         CMP     R1, R2 
                STRLO   R0, [R1], #4 
                BLO     LoopZI 
BSSIsEmpty:


@  Enter the C code
                .extern main
                LDR R0, =main
                BX      R0


@ User Initial Stack & Heap (not used in GNU port)
@@                AREA    |.text|, CODE, READONLY

@@                IMPORT  __use_two_region_memory
@@                EXPORT  __user_initial_stackheap
@@__user_initial_stackheap

@@                LDR     R0, =  Heap_Mem
@@                LDR     R1, =(Stack_Mem + USR_Stack_Size)
@@                LDR     R2, = (Heap_Mem +      Heap_Size)
@@                LDR     R3, = Stack_Mem
@@                BX      LR


.end


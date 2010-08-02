#include <stdint.h>

#include "cmsis.h"
#include "os_init.h"
#include "exception_handlers.h"

void Reset_Handler(void);
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void DebugMon_Handler(void);

extern void BootInit();

extern void (* const Vectors[])(void);
extern void __top_of_stack__(void);

//*****************************************************************************
#if defined (__cplusplus)
} // extern "C"
#endif

#define HFSR_DEBUGEVT	(1 << 31)
#define HFSR_FORCED		(1 << 30)
#define HFSR_VECTTBL	(1 << 1)

void HardFault_Handler()
{
#if 0
	unsigned int hfsr = SCB->HFSR;
	bool debugevt = hfsr & HFSR_DEBUGEVT ? true : false, // debug event no handled
		forced = hfsr & HFSR_FORCED ? true : false, // configurable fault escalated because not enabled
		vecttbl = hfsr & HFSR_VECTTBL ? true : false; // vector table read exception. Ins addr in PC
#endif
	Exception_HardFault();
}


#define CFSR_BFARVALID		(1 << (7 + 8))
#define CFSR_STKERR			(1 << (4 + 8))
#define CFSR_UNSTKERR		(1 << (3 + 8))
#define CFSR_IMPRECISERR	(1 << (2 + 8))
#define CFSR_PRECISERR		(1 << (1 + 8))
#define CFSR_IBUSERR		(1 << (0 + 8))

void BusFault_Handler()
{
#if 0
	unsigned int cfsr = SCB->CFSR;
	bool bfarValid = cfsr & CFSR_BFARVALID ? true : false,
		stkErr = cfsr & CFSR_STKERR ? true : false, // stacking from exception bus fault
		unstkErr = cfsr & CFSR_UNSTKERR ? true : false, // unstacking from exception bus fault
		impreciseErr = cfsr & CFSR_IMPRECISERR ? true : false, // imprecise data bus err.
		preciseErr = cfsr & CFSR_PRECISERR ? true : false, // precise data bus err
		ibusErr = cfsr & CFSR_IBUSERR ? true : false; // instruction bus error flag caused by prefetch
	unsigned int busFaultAddr = bfarValid ? SCB->BFAR : 0;
#endif
	Exception_DataAbort();
}

#define CFSR_DIVBYZERO	(1 << (9 + 16))
#define CFSR_UNALIGNED	(1 << (8 + 16))
#define CFSR_NOCP		(1 << (3 + 16))
#define CFSR_INVPC		(1 << (2 + 16))
#define CFSR_INVSTATE	(1 << (1 + 16))
#define CFSR_UNDEFINSTR	(1 << (0 + 16))

void UsageFault_Handler()
{
#if 0
	unsigned int cfsr = SCB->CFSR;
	bool divByZero = cfsr & CFSR_DIVBYZERO ? true : false, // only when DIV_0_TRP enabled
		unaligned = cfsr & CFSR_UNALIGNED ? true : false, // unaligned mem access when UNALIGN_TRP enabled
		noCp = cfsr & CFSR_NOCP ? true : false, // no coprocessor
		invPc = cfsr & CFSR_INVPC ? true : false, // attempt to load invalid addr into PC
		invState = cfsr & CFSR_INVSTATE ? true : false, // invalid combination of EPSR for reasons other than undefined instruction
		undefinedStr = cfsr & CFSR_UNDEFINSTR ? true : false; // undefined instruction
#endif
	Exception_UndefinedInstruction();
}

#define CFSR_MMARVALID	(1 << 7)
#define CFSR_MSTKERR	(1 << 4)
#define CFSR_MUNSTKERR	(1 << 3)
#define CFSR_DACCVIOL	(1 << 1)
#define CFSR_IACCVIOL	(1 << 0)

__attribute__ ((section(".vectors")))
void (* const Vectors[])(void) = {
	// Core Level - CM3
	&__top_of_stack__,						// The initial stack pointer
	Reset_Handler,							// The reset handler
	Exception_UnhandledIRQ,					// The NMI handler
	HardFault_Handler,						// The hard fault handler
	Exception_UnhandledIRQ,					// The MPU fault handler
	BusFault_Handler,						// The bus fault handler
	UsageFault_Handler,						// The usage fault handler
	0,										// Reserved
	0,										// Reserved
	0,										// Reserved
	0,										// Reserved
	Exception_UnhandledIRQ,                 // SVCall handler
	Exception_UnhandledIRQ,					// Debug monitor handler
	0,										// Reserved
	Exception_UnhandledIRQ,                 // The PendSV handler
	Exception_UnhandledIRQ,                 // The SysTick handler
};

unsigned long *__ram_vectors_start__;
extern unsigned long __text_start__, __text_end__,
	__vectors_start__, __vectors_end__,
	__data_start__, __data_end__,
	__bss_start__, __bss_end__;

#define VECTORS_LEN_CORE	(sizeof(Vectors) / sizeof(void(*)(void)))
#define VECTORS_LEN_LPC17XX	(35)
#define VECTORS_LEN			(VECTORS_LEN_CORE + VECTORS_LEN_LPC17XX)

__attribute__ ((noreturn)) void Reset_Handler(void)
{
    unsigned long *pulSrc, *pulDest;
	static void (* RamVectors[VECTORS_LEN])(void) __attribute__ ((aligned(0x100), section("privileged_bss")));

    // Copy the data segment initializers from flash to SRAM.
    pulSrc = &__text_end__;
    for (pulDest = &__data_start__; pulDest < &__data_end__; )
        *pulDest++ = *pulSrc++;

    // Zero fill the bss segment.  This is done with inline assembly since this
    // will clear the value of pulDest if it is not kept in a register.
	for (register unsigned long *dst = &__bss_start__; dst < &__bss_end__; dst++)
		*dst = 0;

    // Copy the initial vector table from flash to SRAM and then init the rest
	pulSrc = &__vectors_start__;
    for (pulDest = (unsigned long *)RamVectors; 
			pulDest < (unsigned long *)(RamVectors + VECTORS_LEN_CORE); )
        *pulDest++ = *pulSrc++;
	while (pulDest < (unsigned long *)(RamVectors + VECTORS_LEN))
		*pulDest++ = (unsigned long)&Exception_UnhandledIRQ;
	__ram_vectors_start__ = (unsigned long *)RamVectors;
	SCB->VTOR = (unsigned long)RamVectors;

	Boot_Init();
	
	// If main ever returns, reset the board
	NVIC_SystemReset();
}


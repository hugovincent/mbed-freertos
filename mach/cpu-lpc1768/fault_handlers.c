#include <stdbool.h>
#include "cmsis.h"

void HardFault_Handler()
{
	unsigned int hfsr = SCB->HFSR;

	bool debugevt = hfsr & (0x1<<31);		// Debug event not handled.
	bool forced   = hfsr & (0x1<<30);		// Configurable fault escalated because not enabled.
	bool vecttbl  = hfsr & (0x1<<1);		// Vector table read exception. Instruction addr in PC.

	// FIXME
	while (1);
}


void BusFault_Handler()
{
	unsigned int cfsr = SCB->CFSR;

	bool bfarValid    = cfsr & (0x1<<15);
	bool stkErr       = cfsr & (0x1<<12); 	// stacking from exception bus fault
	bool unstkErr     = cfsr & (0x1<<11);	// unstacking from exception bus fault
	bool impreciseErr = cfsr & (0x1<<10);	// imprecise data bus err.
	bool preciseErr   = cfsr & (0x1<<9);	// precise data bus err
	bool ibusErr      = cfsr & (0x1<<8); 	// instruction bus error flag caused by prefetch

	unsigned int busFaultAddr = bfarValid ? SCB->BFAR : 0;

	// FIXME
	while (1);
}


void UsageFault_Handler()
{
	unsigned int cfsr = SCB->CFSR;

	bool divByZero    = cfsr & (0x1<<25);	// only when DIV_0_TRP enabled
	bool unaligned    = cfsr & (0x1<<24);	// unaligned mem access when UNALIGN_TRP enabled
	bool noCp         = cfsr & (0x1<<19);	// no coprocessor
	bool invPc        = cfsr & (0x1<<18);	// attempt to load invalid addr into PC
	bool invState     = cfsr & (0x1<<17);	// invalid combination of EPSR for reasons other than undefined instruction
	bool undefinedStr = cfsr & (0x1<<16);	// undefined instruction

	// FIXME
	while (1);
}


void NMI_Handler()
{
	// FIXME
	while (1);
}


void SVC_Handler()
{
	// FIXME
	while (1);
}


void DebugMon_Handler()
{
	// FIXME
	while (1);
}


void PendSV_Handler()
{
	// FIXME
	while (1);
}


void SysTick_Handler()
{
	// FIXME
	while (1);
}


void MemManage_Handler()
{
	// FIXME
	while (1);
}

void UnhandledIRQ_Handler()
{
	// FIXME
	while (1);
}

// FIXME
#if 0
void pop_registers_from_fault_stack(unsigned int * hardfault_args)
{
	unsigned int stacked_r0;
	unsigned int stacked_r1;
	unsigned int stacked_r2;
	unsigned int stacked_r3;
	unsigned int stacked_r12;
	unsigned int stacked_lr;
	unsigned int stacked_pc;
	unsigned int stacked_psr;

	stacked_r0 = ((unsigned long) hardfault_args[0]);
	stacked_r1 = ((unsigned long) hardfault_args[1]);
	stacked_r2 = ((unsigned long) hardfault_args[2]);
	stacked_r3 = ((unsigned long) hardfault_args[3]);

	stacked_r12 = ((unsigned long) hardfault_args[4]);
	stacked_lr = ((unsigned long) hardfault_args[5]);
	stacked_pc = ((unsigned long) hardfault_args[6]);
	stacked_psr = ((unsigned long) hardfault_args[7]);

	/* Inspect stacked_pc to locate the offending instruction. */
	for( ;; );
}
#endif

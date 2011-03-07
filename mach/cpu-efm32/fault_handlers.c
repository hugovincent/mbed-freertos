#include <stdio.h>
#include <stdbool.h>
#include "cmsis.h"
#include "console.h"
#include "debug_support.h"
#include "power_management.h"
#include "task_manager.h"

enum ExceptionType
{
	DataAbort = 0,
	PrefetchAbort,
	UndefinedInstruction
};

/* Print as much info as we can about the processor state pre-exception. */
static __attribute__ ((noreturn)) void __print_info(enum ExceptionType type)
{
	// pc is the actual address, and pc_ptr is the actually opcode at that address
	switch (type)
	{
		case DataAbort:
			printf("\n[FreeRTOS] Fatal Error: Data Abort at pc : ");
			//printf("[<%08x>] -> 0x%08x\n", SavedRegs.pc, SavedRegs.pc_ptr);
			break;

		case PrefetchAbort:
			printf("\n[FreeRTOS] Fatal Error: Prefetch Abort at pc : ");
			//printf("[<%08x>]\n", SavedRegs.pc);
			break;

		case UndefinedInstruction:
			printf("\n[FreeRTOS] Fatal Error: Undefined Instruction ");
			//printf("0x%08x at pc : [<%08x>]\n", SavedRegs.pc_ptr, SavedRegs.pc);
			break;
	}

	puts("\nProcesor State:");
	//Debug_PrintSavedRegisterState(&SavedRegs);

	puts("\nBacktrace:");
	//Debug_PrintBacktrace((unsigned int *)SavedRegs.r[11], 0); // r11 is the frame pointer

	// FIXME some FreeRTOS-specific thread information should go here?

	puts("\nHalting.\n");

	// Put processor core into sleep mode to conserve power.
	PowerManagement_PowerDown();
}

void HardFault_Handler()
{
#if 0
	unsigned int hfsr = SCB->HFSR;

	bool debugevt = hfsr & (0x1<<31);		// Debug event not handled.
	bool forced   = hfsr & (0x1<<30);		// Configurable fault escalated because not enabled.
	bool vecttbl  = hfsr & (0x1<<1);		// Vector table read exception. Instruction addr in PC.
#endif

	// FIXME
	Console_SingleMode();
	puts("hard fault");
	PowerManagement_PowerDown();
}


void BusFault_Handler()
{
#if 0
	unsigned int cfsr = SCB->CFSR;

	bool bfarValid    = cfsr & (0x1<<15);
	bool stkErr       = cfsr & (0x1<<12); 	// stacking from exception bus fault
	bool unstkErr     = cfsr & (0x1<<11);	// unstacking from exception bus fault
	bool impreciseErr = cfsr & (0x1<<10);	// imprecise data bus err.
	bool preciseErr   = cfsr & (0x1<<9);	// precise data bus err
	bool ibusErr      = cfsr & (0x1<<8); 	// instruction bus error flag caused by prefetch

	unsigned int busFaultAddr = bfarValid ? SCB->BFAR : 0;
#endif

	// FIXME
	Console_SingleMode();
	puts("bus fault");
	PowerManagement_PowerDown();
}


void UsageFault_Handler()
{
#if 0
	unsigned int cfsr = SCB->CFSR;

	bool divByZero    = cfsr & (0x1<<25);	// only when DIV_0_TRP enabled
	bool unaligned    = cfsr & (0x1<<24);	// unaligned mem access when UNALIGN_TRP enabled
	bool noCp         = cfsr & (0x1<<19);	// no coprocessor
	bool invPc        = cfsr & (0x1<<18);	// attempt to load invalid addr into PC
	bool invState     = cfsr & (0x1<<17);	// invalid combination of EPSR for reasons other than undefined instruction
	bool undefinedStr = cfsr & (0x1<<16);	// undefined instruction
#endif

	// FIXME
	Console_SingleMode();
	puts("usage fault");
	PowerManagement_PowerDown();
}


void NMI_Handler()
{
	// FIXME
	Console_SingleMode();
	puts("nmi");
	PowerManagement_PowerDown();
}


void SVC_Handler()
{
	// FIXME
	Console_SingleMode();
	puts("svc");
	PowerManagement_PowerDown();
}


void DebugMon_Handler()
{
	// FIXME
	Console_SingleMode();
	puts("debugmon");
	PowerManagement_PowerDown();
}


void PendSV_Handler()
{
	// FIXME
	Console_SingleMode();
	puts("pendsv");
	PowerManagement_PowerDown();
}


void SysTick_Handler()
{
	// FIXME
	Console_SingleMode();
	puts("systick");
	PowerManagement_PowerDown();
}


void MemManage_Handler()
{
	// Get the exception stack (bit 2 of the LR determines which stack it's in)
	unsigned int exc_return = (unsigned int)__builtin_return_address(0);
	unsigned int *exc_stack;
	if (exc_return & (0x1<<2))
		exc_stack = (unsigned int *)__get_PSP();
	else
		exc_stack = (unsigned int *)__get_MSP();

	unsigned int pc = exc_stack[6];

	unsigned int cfsr = SCB->CFSR;
	bool mmarValid   = cfsr & (0x1<<7); // fault address valid
#if 0
	bool mstkErr     = cfsr & (0x1<<4);	// stacking from excpetion caused access violation
	bool munstkErr   = cfsr & (0x1<<3);	// as above but unstacking from exception
	bool dataAccViol = cfsr & (0x1<<1);	// data access violation. return PC points to instruction and loads addr in MMAR
	bool instAccViol = cfsr & (0x1<<0);	// instruction access violation. return PC points to instruction (no MMAR)
#endif

	const unsigned int mpuFaultAddr = mmarValid ? SCB->MMFAR : 0;

	if (!TaskManager_HandleFault(TaskManager_MPUFault | ((exc_return & (0x1<<3)) ? TaskManager_FaultFromThread : 0), pc, mpuFaultAddr))
	{
		Console_SingleMode();
		puts("\n[FreeRTOS] Fatal Error: Unhandled MPU protection violation.");
		printf("\tpc : [<%08x>]  bad_access : [<%08x>]\n", pc, mpuFaultAddr);
		puts("\nHalting.");
		PowerManagement_PowerDown();
	}
}

void UnhandledIRQ_Handler()
{
	// FIXME
	Console_SingleMode();
	puts("unhandled IRQ");
	PowerManagement_PowerDown();
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

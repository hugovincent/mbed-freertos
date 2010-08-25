#include <stdint.h>
#include <string.h>

#include "cmsis.h"
#include "os_init.h"

extern void Reset_Handler();
extern void NMI_Handler();
extern void HardFault_Handler();
extern void MemManage_Handler();
extern void BusFault_Handler();
extern void UsageFault_Handler();
extern void SVC_Handler();
extern void DebugMon_Handler();
extern void PendSV_Handler();
extern void SysTick_Handler();
extern void UnhandledIRQ_Handler();

// Symbols defined by the linker:
extern unsigned int __data_init_start__, __stacks_top__,
					__data_start__, __data_end__,
					__bss_start__,  __bss_end__;


/*****************************************************************************/

// Vector table:
void (* const vectors[])(void)
		__attribute__ ((section(".vectors"))) =
{
	(void (*)())&__stacks_top__,	// Initial stack pointer
	Reset_Handler,
	NMI_Handler,
	HardFault_Handler,
	MemManage_Handler,				// MPU faults handler
	BusFault_Handler,
	UsageFault_Handler,
	0, 0, 0, 0, 					// (reserved)
	SVC_Handler,
	DebugMon_Handler,
	0,								// (reserved)
	PendSV_Handler,
	SysTick_Handler,
};


#define VECTORS_LEN_CORE	(sizeof(vectors) / sizeof(void(*)(void)))
#define VECTORS_LEN_LPC17XX	(35)
#define VECTORS_LEN			(VECTORS_LEN_CORE + VECTORS_LEN_LPC17XX)

// Vector table in RAM (after relocation):
void (* __ram_vectors[VECTORS_LEN])(void) __attribute__ ((section(".ram_vectors")));

__attribute__ ((noreturn)) void Reset_Handler(void)
{
	// Copy the data segment initializers from flash to RAM
	unsigned int *src  = &__data_init_start__;
	unsigned int *dest = &__data_start__;
	while (dest < &__data_end__)
		*(dest++) = *(src++);

	// Zero fill the bss segment
	dest = &__bss_start__;
	while (dest < &__bss_end__)
		*(dest++) = 0;

	// Copy the initial vector table from flash to RAM and then fill the
	// remaining vectors with calls to UnhandledIRQ_Handler.
	for (int i = 0; i < VECTORS_LEN_CORE; i++)
		__ram_vectors[i] = vectors[i];
	for (int i = VECTORS_LEN_CORE; i < VECTORS_LEN; i++)
		__ram_vectors[i] = UnhandledIRQ_Handler;

	// Perform the relocation
	SCB->VTOR = (unsigned int)__ram_vectors;

	// Enable Bus and Usage faults (MPU faults enabled when MPU is inited)
	SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk;

	// Enable div-by-0 and unaligned access faults
	SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk | SCB_CCR_UNALIGN_TRP_Msk;

#ifdef USE_PROCESS_STACK
	// Change to process stack. This is only used until scheduler starts.
	// Leave MSP a few in case any interrupts occur during startup
	__set_PSP((uint32_t)(&__stacks_top__ - 16)); 
	__set_CONTROL(2); // PSP + priv mode
#endif

	// Boot the system: hardware initialisation etc., eventually calls main()
	Boot_Init();
}


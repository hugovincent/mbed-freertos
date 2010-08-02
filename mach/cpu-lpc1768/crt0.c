#include <stdint.h>

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

extern void __top_of_stack__();

// Vector table
__attribute__ ((section(".vectors")))
void (* const vectors[])(void) = {
	&__top_of_stack__,						// Initial stack pointer
	Reset_Handler,
	NMI_Handler,
	HardFault_Handler,
	MemManage_Handler,						// MPU faults handler 
	BusFault_Handler,
	UsageFault_Handler,
	0, 0, 0, 0, 							// (reserved)
	SVC_Handler,
	DebugMon_Handler,
	0,										// (reserved)
	PendSV_Handler,
	SysTick_Handler,
};

unsigned int *__ram_vectors_start__;
extern unsigned int __text_start__, __text_end__,
	   __vectors_start__, __vectors_end__,
	   __data_start__, __data_end__,
	   __bss_start__, __bss_end__;

#define VECTORS_LEN_CORE	(sizeof(vectors) / sizeof(void(*)(void)))
#define VECTORS_LEN_LPC17XX	(35)
#define VECTORS_LEN			(VECTORS_LEN_CORE + VECTORS_LEN_LPC17XX)

__attribute__ ((noreturn)) void Reset_Handler(void)
{
	static void (* RamVectors[VECTORS_LEN])(void) 
		__attribute__ ((aligned(0x100), section("privileged_bss")));

	// Copy the data segment initializers from flash to RAM
	unsigned int *src  = &__text_end__;
	unsigned int *dest = &__data_start__;
	while (dest < &__data_end__)
		*dest++ = *src++;

	// Zero fill the bss segment
	dest = &__bss_start__;
	while (dest < &__bss_end__)
		*dest++ = 0;

	// Copy the initial vector table from flash to RAM and then init the rest
	src = &__vectors_start__;
	dest = (unsigned int *)RamVectors; 
	while (dest < (unsigned int *)(RamVectors + VECTORS_LEN_CORE))
		*dest++ = *src++;

	while (dest < (unsigned int *)(RamVectors + VECTORS_LEN))
		*dest++ = (unsigned int)&UnhandledIRQ_Handler;

	__ram_vectors_start__ = (unsigned int *)RamVectors;
	SCB->VTOR = (unsigned int)RamVectors;

	Boot_Init();

	// If main ever returns, reset the board
	NVIC_SystemReset();
}


#include <sys/types.h>
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSconfig.h"
#include "lib/syscalls/heap.h"
#include "power_management.h"
#include "drivers/wdt.h"

#ifdef CORE_HAS_MPU
#include "mpu_manager.h"
#endif

#if configUSE_IDLE_HOOK == 1
void vApplicationIdleHook()
{
#ifdef CORE_HAS_MPU
	MPUManager_Idle();
#endif
	PowerManagement_Idle();
}
#endif

#if configUSE_MALLOC_FAILED_HOOK == 1
__attribute__ ((noreturn)) void vApplicationMallocFailedHook()
{
	puts("\n[FreeRTOS] Fatal Error: memory allocation failed!\n\nHalting.");
	PowerManagement_PowerDown(); // Wait for WDT to reset.
}
#endif

#if configCHECK_FOR_STACK_OVERFLOW > 0
__attribute__ ((noreturn)) void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
	printf("\n[FreeRTOS] Fatal Error: task \"%s\" had a stack overflow!\n\nHalting.\n", pcTaskName);
	PowerManagement_PowerDown(); // Wait for WDT to reset.
}
#endif

__attribute__ ((weak)) size_t xPortGetFreeHeapSize(void)
{
	/* FIXME this isn't really correct, it reports only the RAM which is available to the
	 * system, but which hasn't yet been assigned to Newlibs Malloc implementation. */
	
	/* Initialize on first call */
	if (heap_end == 0)
		heap_end = (void *)&__heap_start__;

	return &__stacks_min__ - (unsigned int *)heap_end;
}

__attribute__ ((weak)) void vPortInitialiseBlocks(void)
{
	// Nothing needed here.
}

#if configGENERATE_RUN_TIME_STATS == 1
#if (defined(TARGET_LPC23xx) || (TARGET_LPC17xx))
/* FIXME this was written for LPC2368 - check it works here too */
/* This uses Timer 1 to record task run-time statistics. Allows FreeRTOS
 * to generate a nice, tabular `top`-style CPU-usage listing. 
 */
void vConfigureTimerForRunTimeStats( void )
{
	// Power up and feed the timer with a clock.
	LPC_SC->PCONP |= 0x1<<2;
	LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & (~(0x3<<4))) | (0x01<<4);

	// Reset Timer 1.
	LPC_TIM1->TCR = 0x1<<1;

	// Prescale to a frequency that is good enough to get a decent resolution,
	// but not too fast so as to overflow all the time.
	LPC_TIM1->PR =  ( SystemCoreClock / 10000UL ) - 1UL;

	// Start the counter, counting up.
	LPC_TIM1->CTCR = 0x0;
	LPC_TIM1->TCR = 0x1<<0;
}
#else
#error "Target not supported"
#endif
#endif


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
/* This uses a timer to measure uptime and task run-time statistics (CPU usage).
 * Timer counts in microseconds. We use Timer1, but any timer would work.
 * Assumes SystemCoreClock is runnning at integer-valued MHz.
 */
#define RUNTIME_TIMER 1
void vConfigureTimerForRunTimeStats( void )
{
	// Power up timer hardware and connect the clock to CCLK
#if   RUNTIME_TIMER == 0
#define theTimer LPC_TIM0
	LPC_SC->PCONP |= 0x1<<1;
	LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & (~(0x3<<2))) | (0x01<<2);
#elif RUNTIME_TIMER == 1
#define theTimer LPC_TIM1
	LPC_SC->PCONP |= 0x1<<2;
	LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & (~(0x3<<4))) | (0x01<<4);
#elif RUNTIME_TIMER == 2
#define theTimer LPC_TIM2
	LPC_SC->PCONP |= 0x1<<22;
	LPC_SC->PCLKSEL1 = (LPC_SC->PCLKSEL1 & (~(0x3<<12))) | (0x01<<12);
#elif RUNTIME_TIMER == 3
#define theTimer LPC_TIM3
	LPC_SC->PCONP |= 0x1<<23;
	LPC_SC->PCLKSEL1 = (LPC_SC->PCLKSEL1 & (~(0x3<<14))) | (0x01<<14);
#endif

	// Stop and reset the timer
	theTimer->TCR = 0x1<<1;

	// set the prescaler to get 1 usec ticks
	theTimer->PR = ( SystemCoreClock / 1000000UL ) - 1UL;

	// Start the timer, counting up.
	theTimer->CTCR = 0x0;
	theTimer->TCR = 0x1<<0;
}
#else
#error "Target not supported"
#endif
#endif


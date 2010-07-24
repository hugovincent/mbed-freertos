#include <sys/types.h>
#include "FreeRTOS.h"
#include "task.h"
#include "lib/syscalls/heap.h"

#if configUSE_IDLE_HOOK == 1
void vApplicationIdleHook()
{
	// Put processor core into Idle Mode to conserve power.
	LPC_SC->PCON |= 0x1;

	// And we're back... let's just NOP for a bit just in case.
	portNOP();
	portNOP();
	portNOP();
	portNOP();
}
#endif

#if configUSE_MALLOC_FAILED_HOOK == 1
void vApplicationMallocFailedHook()
{
	printf("[FreeRTOS] Fatal Error: memory allocation failed!\n");
	while (1); // Wait for WDT to reset.
}
#endif

#if configCHECK_FOR_STACK_OVERFLOW > 0
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
	printf("[FreeRTOS] Fatal Error: task \"%s\" had a stack overflow!\n", pcTaskName);
	while(1); // Wait for WDT to reset.
}
#endif

__attribute__ ((weak)) size_t xPortGetFreeHeapSize(void)
{
	/* FIXME this isn't really correct, it reports only the RAM which is available to the
	 * system, but which hasn't yet been assigned to Newlibs Malloc implementation. */
	
	/* Initialize on first call */
	if (heap_end == 0)
		heap_end = (void *)&__start_of_heap__;

	return &__stack_min__ - (unsigned int *)heap_end;
}

__attribute__ ((weak)) void vPortInitialiseBlocks(void)
{
	// FIXME
}

#include <sys/types.h>
#include "FreeRTOS.h"
#include "task.h"
#include "lib/syscalls/heap.h"
#include "power_management.h"

#if configUSE_IDLE_HOOK == 1
void vApplicationIdleHook()
{
	PowerManagement_Idle();
}
#endif

#if configUSE_MALLOC_FAILED_HOOK == 1
__attribute__ ((noreturn)) void vApplicationMallocFailedHook()
{
	printf("[FreeRTOS] Fatal Error: memory allocation failed!\n");
	PowerManagement_PowerDown(); // Wait for WDT to reset.
}
#endif

#if configCHECK_FOR_STACK_OVERFLOW > 0
__attribute__ ((noreturn)) void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
	printf("[FreeRTOS] Fatal Error: task \"%s\" had a stack overflow!\n", pcTaskName);
	PowerManagement_PowerDown(); // Wait for WDT to reset.
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
	// Nothing needed here.
}

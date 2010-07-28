#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

extern xSemaphoreHandle xEMACSemaphore;

void vEmacISR_Handler(void)
{
	int xHigherPriorityTaskWoken = pdFALSE;

    /* Ensure the uIP task is not blocked as data has arrived. */
    xSemaphoreGiveFromISR(xEMACSemaphore, (portBASE_TYPE *)&xHigherPriorityTaskWoken);

    /* Clear the interrupt. */
    LPC_EMAC->IntClear = 0xffff;
#if defined(MBED_LPC23xx)
    LPC_VIC->Address = 0;
#endif

	if (xHigherPriorityTaskWoken)
    {
    	/* Giving the semaphore woke a task. */
        vPortYieldFromISR();
    }
}

__attribute__ ((naked)) void vEmacISR(void)
{
	/* Save the context of the interrupted task. */
	portSAVE_CONTEXT();

	/* Call the handler to do the work.  This must be a separate
	function to ensure the stack frame is set up correctly. */
	__asm volatile ("bl			vEmacISR_Handler");

	/* Restore the context of whichever task will execute next. */
	portRESTORE_CONTEXT();
}


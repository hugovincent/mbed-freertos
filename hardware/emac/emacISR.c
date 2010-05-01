#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

extern xSemaphoreHandle xEMACSemaphore;

void vEmacISR_Handler ( void )
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    /* Ensure the uIP task is not blocked as data has arrived. */
    xSemaphoreGiveFromISR( xEMACSemaphore, &xHigherPriorityTaskWoken );

    /* Clear the interrupt. */
    MAC_INTCLEAR = 0xffff;
    VICVectAddr = 0;

	if( xHigherPriorityTaskWoken )
    {
    	/* Giving the semaphore woke a task. */
        portYIELD_FROM_ISR();
    }
}

__attribute__ ((naked)) void vEmacISR_Wrapper ( void )
{
	/* Save the context of the interrupted task. */
	portSAVE_CONTEXT();

	/* Call the handler to do the work.  This must be a separate
	function to ensure the stack frame is set up correctly. */
	vEmacISR_Handler();

	/* Restore the context of whichever task will execute next. */
	portRESTORE_CONTEXT();
}

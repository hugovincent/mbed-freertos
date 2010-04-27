#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

extern xSemaphoreHandle xEMACSemaphore;

__attribute__((interrupt ("IRQ") )) void vEmacISR( void )
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


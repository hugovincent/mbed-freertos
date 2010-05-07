#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

extern xSemaphoreHandle xEMACSemaphore;

#if configIRQ_CAN_CONTEXT_SWITCH == 1
void vEmacISR_Handler(void)
#else
__attribute__ ((interrupt ("IRQ"))) void vEmacISR(void)
#endif
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    /* Ensure the uIP task is not blocked as data has arrived. */
    xSemaphoreGiveFromISR( xEMACSemaphore, &xHigherPriorityTaskWoken );

    /* Clear the interrupt. */
    LPC_EMAC->IntClear = 0xffff;
    LPC_VIC->Address = 0;

	if( xHigherPriorityTaskWoken )
    {
    	/* Giving the semaphore woke a task. */
        portYIELD_FROM_ISR();
    }
}

#if configIRQ_CAN_CONTEXT_SWITCH == 1
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
#endif


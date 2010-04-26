// $Id: uart0ISR.c 89 2008-10-10 15:25:48Z jcw $
// $Revision: 89 $
// $Author: jcw $
// $Date: 2008-10-10 11:25:48 -0400 (Fri, 10 Oct 2008) $
// $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uart/uart0ISR.c $
//
// Modified by Hugo Vincent, 25 April 2010.

#include <stdlib.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "uart_defs.h"
#include "uart0.h"
#include "uart0ISR.h"

// Constants to determine the ISR source
#define serSOURCE_THRE				((unsigned portCHAR) 0x02)
#define serSOURCE_RX_TIMEOUT		((unsigned portCHAR) 0x0c)
#define serSOURCE_ERROR				((unsigned portCHAR) 0x06)
#define serSOURCE_RX				((unsigned portCHAR) 0x04)
#define serINTERRUPT_SOURCE_MASK	((unsigned portCHAR) 0x0f)

// Queues used to hold received characters, and characters waiting to be transmitted
static xQueueHandle xRX0Queue; 
static xQueueHandle xTX0Queue; 
static volatile portCHAR lTHREEmpty0;

void uart0ISRCreateQueues (unsigned portBASE_TYPE uxQueueLength, xQueueHandle *pxRX0Queue, xQueueHandle *pxTX0Queue, portCHAR volatile **ppcTHREEmptyFlag)
{
	// Create the queues used to hold Rx and Tx characters
	*pxRX0Queue = xRX0Queue = xQueueCreate(uxQueueLength, (unsigned portBASE_TYPE)sizeof(signed portCHAR));
	*pxTX0Queue = xTX0Queue = xQueueCreate(uxQueueLength + 1, (unsigned portBASE_TYPE)sizeof(signed portCHAR));

	// Initialise the THRE empty flag - and pass back a reference
	lTHREEmpty0 = (portCHAR)pdTRUE;
	*ppcTHREEmptyFlag = &lTHREEmpty0;
}

void uart0ISR_Handler (void)
{
	signed portCHAR cChar;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	switch (U0IIR & serINTERRUPT_SOURCE_MASK)
	{
		// Not handling this, but clear the interrupt
		case serSOURCE_ERROR:	
			{
				cChar = U0RBR; // dummy read to clear interrupt
				cChar = U0LSR;

				// FIXME check lines 121-147 in LPC23xx_24xxSampleSoftware/r6/UART/uart.c
			}
			break;

		// The THRE is empty.  If there is another character in the Tx queue, send it now,
		// otherwise, no more characters, so indicate THRE is available
		case serSOURCE_THRE:	
			{
				if (xQueueReceiveFromISR(xTX0Queue, &cChar, &xHigherPriorityTaskWoken) == pdPASS)
				{
					U0THR = cChar;
				}
				else
				{
					lTHREEmpty0 = pdTRUE;
				}
			}
			break;

		// A character was received.  Place it in the queue of received characters
		case serSOURCE_RX_TIMEOUT:
		case serSOURCE_RX:	
			{
				cChar = U0RBR;
				xQueueSendFromISR(xRX0Queue, &cChar, &xHigherPriorityTaskWoken);
			}
			break;

		default	:
			break;
	}

	/* Clear the interrupt. */
	VICVectAddr = 0;

	if (xHigherPriorityTaskWoken)
	{
		/* Giving the semaphore woke a task. */
		portYIELD_FROM_ISR();
	}
}

void uart0ISR (void)
{
	/* Save the context of the interrupted task. */
	portSAVE_CONTEXT();

	/* Call the handler.  This must be a separate function unless you can
	   guarantee that no stack will be used. */
	__asm volatile ( "bl uart0ISR_Handler" );

	/* Restore the context of whichever task is going to run next. */
	portRESTORE_CONTEXT();
}


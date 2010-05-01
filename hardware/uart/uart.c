// $Id: uart0.c 234 2008-10-28 23:56:14Z jcw $
// $Revision: 234 $
// $Author: jcw $
// $Date: 2008-10-28 19:56:14 -0400 (Tue, 28 Oct 2008) $
// $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uart/uart0.c $
//
// Modified by Hugo Vincent, 25 April 2010.

#include <stdlib.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <hardware/uart.h>
#include "uartISRs.h"

#include "uartFractionalBaud.h"

// Queues used to hold received characters, and characters waiting to be transmitted
static xQueueHandle xRX0Queue; 
static xQueueHandle xTX0Queue; 

// Communication flag between the interrupt service routine and serial API
static volatile portCHAR *pcTHREEmpty0;

signed portBASE_TYPE uart0Init(unsigned portLONG ulWantedBaud, unsigned portBASE_TYPE uxQueueLength)
{
	unsigned portLONG ulDivisor, ulFracDiv;
	static unsigned portLONG sulWantedBaud = 115200;
	static unsigned portBASE_TYPE suxQueueLength = 64;

	if (!ulWantedBaud)
	{
		ulWantedBaud = sulWantedBaud;
	}
	sulWantedBaud = ulWantedBaud;

	if (!uxQueueLength)
	{
		uxQueueLength = suxQueueLength;
	}
	suxQueueLength = uxQueueLength;

	vUart0ISRCreateQueues(suxQueueLength, &xRX0Queue, &xTX0Queue, &pcTHREEmpty0);

	if ((xRX0Queue == serINVALID_QUEUE) || (xTX0Queue == serINVALID_QUEUE) 
			|| (sulWantedBaud == (unsigned portLONG) 0))
	{
		return 0;
	}

	taskENTER_CRITICAL();
	{
		// Setup the pin selection & apply clocks to the UART. Reset PCLK to default CCLK/4.
		PINSEL0 |= 0x00000050;
		PCONP |= 0x1<<3;
		PCLKSEL0 &= ~(0x3<<6);

		// Setup a fractional baud rate
		FindBaudWithFractional(sulWantedBaud, &ulDivisor, &ulFracDiv);
		U0FDR = ulFracDiv;

		// Set the DLAB bit so we can access the divisor
		U0LCR = UART_LCR_DLAB;

		// Setup the divisor
		U0DLL = (unsigned portCHAR)(ulDivisor & (unsigned portLONG)0xff);
		ulDivisor >>= 8;
		U0DLM = (unsigned portCHAR)(ulDivisor & (unsigned portLONG)0xff);

		// Setup transmission format and clear the DLAB bit to enable transmission
		U0LCR = UART_LCR_NOPAR | UART_LCR_1STOP | UART_LCR_8BITS;

		// Turn on the FIFO's and clear the buffers
		U0FCR = UART_FCR_EN | UART_FCR_CLR;

		// Setup the VIC for the UART
		VICIntSelect &= ~VIC_UART0; // normal IRQ (not FIQ)
		VICVectAddr6 = (portLONG)&vUart0ISR_Wrapper;
		VICIntEnable = VIC_UART0;

		// Enable UART0 interrupts
		U0IER |= UART_IER_EI;
	}
	taskEXIT_CRITICAL();

	return 1;
}

signed portBASE_TYPE uart0GetChar(signed portCHAR *pcRxedChar, portTickType xBlockTime)
{
	return xQueueReceive(xRX0Queue, pcRxedChar, xBlockTime) ? pdTRUE : pdFALSE;
}

signed portBASE_TYPE uart0PutChar(signed portCHAR cOutChar, portTickType xBlockTime)
{
	signed portBASE_TYPE xReturn = 0;

	taskENTER_CRITICAL();
	{
		// Is there space to write directly to the UART?
		if (*pcTHREEmpty0 == (portCHAR) pdTRUE)
		{
			*pcTHREEmpty0 = pdFALSE;
			U0THR = cOutChar;
			xReturn = pdPASS;
		}
		else 
		{
			// We cannot write directly to the UART, so queue the character.  Block for a maximum of 
			// xBlockTime if there is no space in the queue.
			xReturn = xQueueSend(xTX0Queue, &cOutChar, xBlockTime);

			// Depending on queue sizing and task prioritisation:  While we were blocked waiting to post 
			// interrupts were not disabled.  It is possible that the serial ISR has emptied the Tx queue, 
			// in which case we need to start the Tx off again.
			if ((*pcTHREEmpty0 == (portCHAR) pdTRUE) && (xReturn == pdPASS))
			{
				xQueueReceive(xTX0Queue, &cOutChar, serNO_BLOCK);
				*pcTHREEmpty0 = pdFALSE;
				U0THR = cOutChar;
			}
		}
	}
	taskEXIT_CRITICAL();

	return xReturn;
}


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

#include "uart_fractional_baud.h"

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
		LPC_PINCON->PINSEL0 |= 0x00000050;
		LPC_SC->PCONP |= 0x1<<3;
		LPC_SC->PCLKSEL0 &= ~(0x3<<6);

		// Setup a fractional baud rate
		FindBaudWithFractional(sulWantedBaud, &ulDivisor, &ulFracDiv);
		LPC_UART0->FDR = ulFracDiv;

		// Set the DLAB bit so we can access the divisor
		LPC_UART0->LCR = UART_LCR_DLAB;

		// Setup the divisor
		LPC_UART0->DLL = (unsigned portCHAR)(ulDivisor & (unsigned portLONG)0xff);
		ulDivisor >>= 8;
		LPC_UART0->DLM = (unsigned portCHAR)(ulDivisor & (unsigned portLONG)0xff);

		// Setup transmission format and clear the DLAB bit to enable transmission
		LPC_UART0->LCR = UART_LCR_NOPAR | UART_LCR_1STOP | UART_LCR_8BITS;

		// Turn on the FIFO's and clear the buffers
		LPC_UART0->FCR = UART_FCR_EN | UART_FCR_CLR;

		// Setup the VIC for the UART
		NVIC_SetVector(UART0_IRQn, (portLONG)vUart0ISR);
		NVIC_EnableIRQ(UART0_IRQn);

		// Enable UART0 interrupts
		LPC_UART0->IER |= UART_IER_EI;
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
			LPC_UART0->THR = cOutChar;
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
				LPC_UART0->THR = cOutChar;
			}
		}
	}
	taskEXIT_CRITICAL();

	return xReturn;
}

/* This is a minimal, blocking putchar function for use when interrupts are
 * disabled, where FreeRTOS API calls can't be made (before scheduler is running
 * for example), or during early boot code.
 */
signed portBASE_TYPE uart0PutChar_debug(signed portCHAR c, portTickType dummy)
{
	/* Avoid compiler warning. */
	(void)dummy;

	taskENTER_CRITICAL();
	{
		/* Send the character. */
		LPC_UART0->THR = c;

		/* Wait for it to send (and FIFO to drain). */
		while (!(LPC_UART0->LSR & UART_LSR_TEMT));

		/* Avoid possibility of an ISR queueing up for execution when interrupts are
		 * re-enabled. */
		NVIC_ClearPendingIRQ(UART0_IRQn);
	}
	taskEXIT_CRITICAL();

	return 0;
}

// $Id: uart0ISR.h 107 2008-10-10 19:54:55Z jcw $
// $Revision: 107 $
// $Author: jcw $
// $Date: 2008-10-10 15:54:55 -0400 (Fri, 10 Oct 2008) $
// $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uart/uart0ISR.h $

#ifndef _UART0ISR_H_
#define _UART0ISR_H_

#include <FreeRTOS.h>
#include <queue.h>

// UART register bits
#define UART_LCR_DLAB		(0x80)
#define UART_LCR_NOPAR		(0x00)
#define UART_LCR_1STOP		(0x00)
#define UART_LCR_8BITS		(0x03)
#define UART_IER_EI			(0x07)
#define UART_FCR_EN			(0x01)
#define UART_FCR_CLR		(0x06)
#define UART_LSR_TEMT		(0x40)

#define serINVALID_QUEUE  ((xQueueHandle) 0)
#define serNO_BLOCK       ((portTickType) 0)

void vUart0ISRCreateQueues (unsigned portBASE_TYPE uxQueueLength, xQueueHandle *pxRX0Queue, xQueueHandle *pxTX0Queue, portCHAR volatile **ppcTHREEmptyFlag);
void vUart0ISR(void);

#endif

// $Id: uart0ISR.h 107 2008-10-10 19:54:55Z jcw $
// $Revision: 107 $
// $Author: jcw $
// $Date: 2008-10-10 15:54:55 -0400 (Fri, 10 Oct 2008) $
// $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uart/uart0ISR.h $

#ifndef _UART0ISR_H_
#define _UART0ISR_H_

#include "FreeRTOS.h"
#include "queue.h"

void uart0ISRCreateQueues (unsigned portBASE_TYPE uxQueueLength, xQueueHandle *pxRX0Queue, xQueueHandle *pxTX0Queue, portCHAR volatile **ppcTHREEmptyFlag);
void uart0ISR (void);

#endif

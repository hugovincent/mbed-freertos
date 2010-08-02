// $Id: uart0.h 110 2008-10-11 00:21:58Z jcw $
// $Revision: 110 $
// $Author: jcw $
// $Date: 2008-10-10 20:21:58 -0400 (Fri, 10 Oct 2008) $
// $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uart/uart0.h $

#ifndef _UART0_H_
#define _UART0_H_

#include "FreeRTOS.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

signed portBASE_TYPE uart0Init (unsigned portLONG ulWantedBaud, unsigned portBASE_TYPE uxQueueLength);
signed portBASE_TYPE uart0GetChar (signed portCHAR *pcRxedChar, portTickType xBlockTime);
signed portBASE_TYPE uart0PutChar (signed portCHAR cOutChar, portTickType xBlockTime);

signed portBASE_TYPE uart0PutChar_debug(signed portCHAR c, portTickType dummy);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

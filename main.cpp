/*
    FreeRTOS V6.0.4 - Copyright (C) 2010 Real Time Engineers Ltd.

    ***************************************************************************
    *                                                                         *
    * If you are:                                                             *
    *                                                                         *
    *    + New to FreeRTOS,                                                   *
    *    + Wanting to learn FreeRTOS or multitasking in general quickly       *
    *    + Looking for basic training,                                        *
    *    + Wanting to improve your FreeRTOS skills and productivity           *
    *                                                                         *
    * then take a look at the FreeRTOS eBook                                  *
    *                                                                         *
    *        "Using the FreeRTOS Real Time Kernel - a Practical Guide"        *
    *                  http://www.FreeRTOS.org/Documentation                  *
    *                                                                         *
    * A pdf reference manual is also available.  Both are usually delivered   *
    * to your inbox within 20 minutes to two hours when purchased between 8am *
    * and 8pm GMT (although please allow up to 24 hours in case of            *
    * exceptional circumstances).  Thank you for your support!                *
    *                                                                         *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    ***NOTE*** The exception to the GPL is included to allow you to distribute
    a combined work that includes FreeRTOS without being obliged to provide the
    source code for proprietary components outside of the FreeRTOS kernel.
    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public 
    License and the FreeRTOS license exception along with FreeRTOS; if not it 
    can be viewed here: http://www.freertos.org/a00114.html and also obtained 
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/

/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the standard demo application tasks.
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "Check" hook -  This only executes every five seconds from the tick hook.
 * Its main function is to check that all the standard demo tasks are still 
 * operational.  Should any unexpected behaviour within a demo task be discovered 
 * the tick hook will write an error to the LCD (via the LCD task).  If all the 
 * demo tasks are executing with their expected behaviour then the check task 
 * writes PASS to the LCD (again via the LCD task), as described above.
 *
 * "uIP" task -  This is the task that handles the uIP stack.  All TCP/IP
 * processing is performed in this task.
 */

extern "C" {

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Demo app includes. */
#include "example_tasks/BlockQ.h"
#include "example_tasks/death.h"
#include "example_tasks/blocktim.h"
#include "example_tasks/flash.h"
#include "example_tasks/GenQTest.h"
#include "example_tasks/QPeek.h"
#include "example_tasks/dynamic.h"

#include "hardware/gpio.h"
#include "hardware/uart.h"

} // end extern "C"

//#include "CxxTest.h"

/* Demo application definitions. */
#define mainCHECK_DELAY						( ( portTickType ) 1000 / portTICK_RATE_MS )
#define mainBASIC_WEB_STACK_SIZE            ( configMINIMAL_STACK_SIZE * 6 )

/* Task priorities. */
#define mainQUEUE_POLL_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainFLASH_PRIORITY                  ( tskIDLE_PRIORITY + 2 )
#define mainCREATOR_TASK_PRIORITY           ( tskIDLE_PRIORITY + 3 )
#define mainGEN_QUEUE_TASK_PRIORITY			( tskIDLE_PRIORITY ) 

/* Constants to setup the PLL for 72 MHz cpu clock and USB support.
 * External crystal = 12 MHz, Fcco = 288 MHz
 */
#define mainPLL_MUL			( ( unsigned portLONG ) 12 )
#define mainPLL_DIV			( ( unsigned portLONG ) 1 )
#define mainCPU_CLK_DIV		( ( unsigned portLONG ) 4 )
#define mainUSB_CLK_DIV		( ( unsigned portLONG ) 6 ) 
#define mainPLL_ENABLE		( ( unsigned portLONG ) 0x0001 )
#define mainPLL_CONNECT		( ( ( unsigned portLONG ) 0x0002 ) | mainPLL_ENABLE )
#define mainPLL_LOCK		( ( unsigned portLONG ) 0x4000000 )
#define mainPLL_CONNECTED	( ( unsigned portLONG ) 0x2000000 )
#define mainOSC_ENABLE		( ( unsigned portLONG ) 0x20 )
#define mainOSC_STAT		( ( unsigned portLONG ) 0x40 )
#define mainOSC_SELECT		( ( unsigned portLONG ) 0x01 )

/* Constants to setup the MAM. */
#define mainMAM_TIM_4		( ( unsigned portCHAR ) 0x04 )
#define mainMAM_MODE_FULL	( ( unsigned portCHAR ) 0x02 )

/* Constants to setup the WDT (4MHz RC clock with fixed divide-by-4 -- 4s timeout) */
#define mainWDT_TIMEOUT		( 1000000 * 4 )

/* The task that handles the uIP stack.  All TCP/IP processing is performed in
 * this task.
 */
extern "C" void vuIP_Task( void *pvParameters );

/* Configure the hardware as required by the demo. */
static void prvSetupHardware( void );

/*-----------------------------------------------------------*/

int main( void )
{
	prvSetupHardware();

	//CxxTest test;
	//test.someMethod();
	
	/* Create the uIP task. This uses the lwIP RTOS abstraction layer. */
    xTaskCreate( vuIP_Task, ( signed portCHAR * ) "uIP", mainBASIC_WEB_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY - 1, NULL );

	/* Start the standard demo tasks. */
	vStartBlockingQueueTasks( mainBLOCK_Q_PRIORITY );
    vCreateBlockTimeTasks();
    vStartLEDFlashTasks( mainFLASH_PRIORITY );
    vStartGenericQueueTasks( mainGEN_QUEUE_TASK_PRIORITY );
    vStartQueuePeekTasks();   
    vStartDynamicPriorityTasks();

	uart0PutChar('b', 0);
	uart0PutChar('o', 0);
	uart0PutChar('o', 0);
	uart0PutChar('t', 0);
	uart0PutChar('\r', 0);
	uart0PutChar('\n', 0);

	/* Start the scheduler. */
	vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle task. */
	return 0; 
}
/*-----------------------------------------------------------*/

static void prvWDT_FeedWatchdog( void )
{
	portENTER_CRITICAL();
	WDFEED = 0xAA; WDFEED = 0x55;
	portEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

extern "C" void vApplicationIdleHook( void )
{
	/* Put processor core into Idle Mode to conserve power */
	PCON |= 0x1; // FIXME this is crashy
}
/*-----------------------------------------------------------*/

extern "C" void vApplicationTickHook( void )
{
static unsigned portLONG ulTicksSinceLastDisplay = 0;

	/* Called from every tick interrupt. Have enough ticks passed to make it
	 * time to perform our health status check again? */
	ulTicksSinceLastDisplay++;
	if( ulTicksSinceLastDisplay >= mainCHECK_DELAY )
	{
		prvWDT_FeedWatchdog();
		ulTicksSinceLastDisplay = 0;

		for (int j = 0; j<20; j++ ) {
		uart0PutChar(' ', 0);
		uart0PutChar('t', 0);
		uart0PutChar('i', 0);
		uart0PutChar('c', 0);
		}
		uart0PutChar('\r', 0);
		uart0PutChar('\n', 0);
#if 0
		/* Has an error been found in any task? */
        if( xAreBlockingQueuesStillRunning() != pdTRUE )
		{
			xMessage.pcMessage = "ERROR - BLOCKQ";
		}

		if( xAreBlockTimeTestTasksStillRunning() != pdTRUE )
		{
			xMessage.pcMessage = "ERROR - BLOCKTIM";
		}

		if( xAreGenericQueueTasksStillRunning() != pdTRUE )
		{
			xMessage.pcMessage = "ERROR - GENQ";
		}
		
		if( xAreQueuePeekTasksStillRunning() != pdTRUE )
		{
			xMessage.pcMessage = "ERROR - PEEKQ";
		}       
		
		if( xAreDynamicPriorityTasksStillRunning() != pdTRUE )
		{
			xMessage.pcMessage = "ERROR - DYNAMIC";
		}
        
        xMessage.xColumn++;
#endif
	}
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	portENTER_CRITICAL();
	
	/* Disable the PLL. */
	PLLCON = 0;
	PLLFEED = 0xAA; PLLFEED = 0x55;
	
	/* Turn on the oscillator clock source and wait for it to start. */
	SCS |= mainOSC_ENABLE;
	while( !( SCS & mainOSC_STAT ) );
	CLKSRCSEL = mainOSC_SELECT; 
	
	/* Setup the PLL to multiply the XTAL input (12 MHz) by 6. */
	PLLCFG = ( (mainPLL_MUL - 1) | ((mainPLL_DIV - 1) << 16) );
	PLLFEED = 0xAA; PLLFEED = 0x55;

	/* Turn on and wait for the PLL to lock. */
	PLLCON = mainPLL_ENABLE;
	PLLFEED = 0xAA; PLLFEED = 0x55;
	while( !( PLLSTAT & mainPLL_LOCK ) );

	/* Set clock dividors for CPU and USB blocks. */
	CCLKCFG = (mainCPU_CLK_DIV - 1);	
	USBCLKCFG = (mainUSB_CLK_DIV - 1);
	
	/* Connect the PLL and wait for it to connect. */
	PLLCON = mainPLL_CONNECT;
	PLLFEED = 0xAA;
	PLLFEED = 0x55;
	while( !( PLLSTAT & mainPLL_CONNECTED ) ); 
	
	/* Setup and turn on the MAM.  Four cycle access is used due to the fast
	 * PLL used.  It is possible faster overall performance could be obtained by
	 * tuning the MAM and PLL settings.
	 */
	MAMCR = 0;
	MAMTIM = mainMAM_TIM_4;
	MAMCR = mainMAM_MODE_FULL;
	
	/* Setup the watchdog timer (4 second timeout). */
	// FIXME check/clear reset-reason for WDT reset
	WDMOD = 0x03; // enable and reset
	WDTC = mainWDT_TIMEOUT;
	WDFEED = 0xAA; WDFEED = 0x55;

	portEXIT_CRITICAL();

	/* Setup the led's on the mbed board. */
	vGpioInitialise();

	/* Setup the debug UART (talks to the PC through the mbed's second microcontroller). */
	uart0Init(115200, 128);
}


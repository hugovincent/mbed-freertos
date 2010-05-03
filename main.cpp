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

// Scheduler includes.
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Demo app includes.
#include "example_tasks/BlockQ.h"
#include "example_tasks/death.h"
#include "example_tasks/blocktim.h"
#include "example_tasks/flash.h"
#include "example_tasks/GenQTest.h"
#include "example_tasks/QPeek.h"
#include "example_tasks/dynamic.h"

#include "hardware/gpio.h"
#include "hardware/uart.h"

#include <string.h>
#include <stdlib.h>

// Demo application definitions.
#define mainCHECK_DELAY						( ( portTickType ) 1000 / portTICK_RATE_MS )
#define mainBASIC_WEB_STACK_SIZE            ( configMINIMAL_STACK_SIZE * 8 )

// Task priorities.
#define mainQUEUE_POLL_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainFLASH_PRIORITY                  ( tskIDLE_PRIORITY + 2 )
#define mainCREATOR_TASK_PRIORITY           ( tskIDLE_PRIORITY + 3 )
#define mainGEN_QUEUE_TASK_PRIORITY			( tskIDLE_PRIORITY )

// Constants to setup the WDT (4MHz RC clock with fixed divide-by-4 -- 3s timeout)
#define mainWDT_TIMEOUT		( 1000000 * 3 )

// The task that handles the uIP stack.  All TCP/IP processing is
// performed in this task.
extern "C" void vuIP_Task( void *pvParameters );

// This initialises the stdio layer in Newlib.
extern "C" void initialise_stdio (void);

// Configure the hardware as required by the demo.
static void prvSetupHardware( void );

static void prvWDT_FeedWatchdog( void )
{
	taskENTER_CRITICAL();
	{
		WDFEED = 0xAA; WDFEED = 0x55;
	}
	taskEXIT_CRITICAL();
}

#include <Tests.cpp>

#include "CxxTest.h"
CxxTest cxxTest;

int main( void )
{
	prvSetupHardware();

	// Start the standard demo tasks.
	vStartLEDFlashTasks( mainFLASH_PRIORITY );
	vStartBlockingQueueTasks( mainBLOCK_Q_PRIORITY );
	vCreateBlockTimeTasks();
	vStartGenericQueueTasks( mainGEN_QUEUE_TASK_PRIORITY );
	vStartQueuePeekTasks();
	vStartDynamicPriorityTasks();

	// Create the uIP task. This uses the lwIP RTOS abstraction layer.
//	xTaskCreate( vuIP_Task, ( signed portCHAR * ) "uIP", 
//			mainBASIC_WEB_STACK_SIZE, NULL, mainQUEUE_POLL_PRIORITY, NULL );

	cxxTest.someMethod();

	uart0PutChar('b', 0);
	uart0PutChar('o', 0);
	uart0PutChar('o', 0);
	uart0PutChar('t', 0);
	uart0PutChar('\r', 0);
	uart0PutChar('\n', 0);

	// Start the scheduler.
	printf("FreeRTOS Kernel, v" tskKERNEL_VERSION_NUMBER " for " PLAT_NAME \
			" booted, starting scheduler.");
	vTaskStartScheduler();
	// Will only get here if there was insufficient memory to create the idle task.

	while (1);	// Wait for WDT to reset.
}

extern "C" void vApplicationIdleHook( void )
{
	// Put processor core into Idle Mode to conserve power.
	PCON |= 0x1;

	// And we're back... let's just NOP for a bit just in case.
	portNOP();
	portNOP();
	portNOP();
	portNOP();
}

extern "C" void vApplicationMallocFailedHook( void )
{
	printf("[FreeRTOS] Error: memory allocation failed!\n");
	while (1); // Wait for WDT to reset.
}

extern "C" void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
	printf("[FreeRTOS] Error: task \"%s\" had a stack overflow!\n", pcTaskName);
	while(1); // Wait for WDT to reset.
}

extern "C" void vApplicationTickHook( void )
{
	static unsigned portLONG ulTicksSinceLastDisplay = 0;

	// Called from every tick interrupt. Have enough ticks passed to make it
	// time to perform our health status check again?
	ulTicksSinceLastDisplay++;
	if( ulTicksSinceLastDisplay >= mainCHECK_DELAY )
	{
		prvWDT_FeedWatchdog();
		ulTicksSinceLastDisplay = 0;

		printf(" tic\n");

		// Has an error been found in any task?
		if( xAreBlockingQueuesStillRunning() != pdTRUE )
		{
			//	xMessage.pcMessage = "ERROR - BLOCKQ";
		}

		if( xAreBlockTimeTestTasksStillRunning() != pdTRUE )
		{
			//	xMessage.pcMessage = "ERROR - BLOCKTIM";
		}

		if( xAreGenericQueueTasksStillRunning() != pdTRUE )
		{
			//	xMessage.pcMessage = "ERROR - GENQ";
		}

		if( xAreQueuePeekTasksStillRunning() != pdTRUE )
		{
			//	xMessage.pcMessage = "ERROR - PEEKQ";
		}

		if( xAreDynamicPriorityTasksStillRunning() != pdTRUE )
		{
			//	xMessage.pcMessage = "ERROR - DYNAMIC";
		}
	}
}

static void prvSetupHardware( void )
{
#if 0
	// Setup the watchdog timer (3 second timeout).
	// FIXME check/clear reset-reason for WDT reset
	WDMOD = 0x03; // enable and reset
	WDTC = mainWDT_TIMEOUT;
	WDFEED = 0xAA; WDFEED = 0x55;
#endif

	// Setup the led's on the mbed board.
	vGpioInitialise();

	// Setup the debug UART (talks to the PC through the mbed's second 
	// microcontroller) and connect it to stdio.
	uart0Init(115200, 128);
	initialise_stdio();
}


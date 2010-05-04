/*
    FreeRTOS V6.0.4 - Copyright (C) 2010 Real Time Engineers Ltd.

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

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

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
#include "webserver.h"

// Demo application definitions.
#define mainCHECK_DELAY					((portTickType)5000 / portTICK_RATE_MS)
#define mainBLOCK_Q_PRIORITY			(tskIDLE_PRIORITY + 2)
#define mainFLASH_PRIORITY              (tskIDLE_PRIORITY + 2)
#define mainGEN_QUEUE_TASK_PRIORITY		(tskIDLE_PRIORITY)

int main()
{
	// Start the standard demo tasks.
	vStartLEDFlashTasks( mainFLASH_PRIORITY );
	vStartBlockingQueueTasks( mainBLOCK_Q_PRIORITY );
	vCreateBlockTimeTasks();
	vStartGenericQueueTasks( mainGEN_QUEUE_TASK_PRIORITY );
	vStartQueuePeekTasks();
	vStartDynamicPriorityTasks();

	//vStartWebserverTask();

	printf("FreeRTOS Kernel " tskKERNEL_VERSION_NUMBER " for " PLAT_NAME
			" booted, starting scheduler.\n");

	// Start the scheduler.
	vTaskStartScheduler();
	// Will only get here if there was insufficient memory to create the idle task.

	while (1);	// Wait for WDT to reset.
}

//-----------------------------------------------------------------------------
// FreeRTOS Callback Hooks:
extern "C"
{
	void vApplicationIdleHook()
	{
		// Put processor core into Idle Mode to conserve power.
		LPC_SC->PCON |= 0x1;

		// And we're back... let's just NOP for a bit just in case.
		portNOP();
		portNOP();
		portNOP();
		portNOP();
	}

	void vApplicationMallocFailedHook()
	{
		printf("[FreeRTOS] Error: memory allocation failed!\n");
		while (1); // Wait for WDT to reset.
	}

	void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
	{
		printf("[FreeRTOS] Error: task \"%s\" had a stack overflow!\n", pcTaskName);
		while(1); // Wait for WDT to reset.
	}

	void vApplicationTickHook()
	{
		static unsigned portLONG ulTicksSinceLastDisplay = 0;

		// Called from every tick interrupt. Have enough ticks passed to make it
		// time to perform our health status check again?
		ulTicksSinceLastDisplay++;
		if( ulTicksSinceLastDisplay >= mainCHECK_DELAY )
		{
			//WDT_FeedWatchdog();
			ulTicksSinceLastDisplay = 0;

			// Has an error been found in any task?
			int allGood = 1;
			if( xAreBlockingQueuesStillRunning() != pdTRUE )
			{
				printf("ERROR - BLOCKQ\n");
				allGood = 0;
			}

			if( xAreBlockTimeTestTasksStillRunning() != pdTRUE )
			{
				printf("ERROR - BLOCKTIM\n");
				allGood = 0;
			}

			if( xAreGenericQueueTasksStillRunning() != pdTRUE )
			{
				printf("ERROR - GENQ\n");
				allGood = 0;
			}

			if( xAreQueuePeekTasksStillRunning() != pdTRUE )
			{
				printf("ERROR - PEEKQ\n");
				allGood = 0;
			}

			if( xAreDynamicPriorityTasksStillRunning() != pdTRUE )
			{
				printf("ERROR - DYNAMIC\n");
				allGood = 0;
			}

			if (allGood == 1)
			{
				printf("All Good.\n");
			}
		}
	}
}


/*
    FreeRTOS V6.0.4 - Copyright (C) 2010 Real Time Engineers Ltd.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
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
#include "example_tasks/webserver_task.h"

#include "hardware/uart.h"
#include "hardware/wdt.h"
#include "power_management.h"

// Demo application definitions.
#define mainCHECK_DELAY					((portTickType)5000 / portTICK_RATE_MS)
#define mainBLOCK_Q_PRIORITY			(tskIDLE_PRIORITY + 2)
#define mainFLASH_PRIORITY              (tskIDLE_PRIORITY + 2)
#define mainGEN_QUEUE_TASK_PRIORITY		(tskIDLE_PRIORITY)


void SimplePrint(const char *str)
{
	while (*str)
	{
		/* Make line endings behave like normal serial terminals. */
		if (*str == '\n')
		{
			uart0PutChar('\r', 0);
		}
		uart0PutChar(*str++, 0);
	}
}

#ifdef CORE_HAS_MPU
void xBadTask(void *params)
{
	extern unsigned long __privileged_data_end__[];
	extern unsigned long __privileged_functions_end__[];
	extern unsigned long __SRAM_segment_end__[];
	extern unsigned long __FLASH_segment_end__[];
	volatile unsigned long *pul;
	volatile unsigned long ulReadData;
	printf("Misbehaving task started...\n");
	vTaskDelay(150);


	portSWITCH_TO_USER_MODE();
	//LPC_GPIO1->FIOCLR = ( 1UL << 23UL );

	/*taskENTER_CRITICAL();
	  taskEXIT_CRITICAL();*/

	pul = __privileged_data_end__ + 1;
	ulReadData = *pul;
	pul = __SRAM_segment_end__ - 1;
	ulReadData = *pul;

	pul = __privileged_functions_end__ + 1;
	ulReadData = *pul;
	pul = __FLASH_segment_end__ - 1;
	ulReadData = *pul;
	printf("Misbehaving task writing LEDs...\n");
	vTaskDelay(150);
	//volatile int v = LPC_UART0->DLM;
	//LPC_GPIO1->FIOCLR = ( 1UL << 23UL );

	printf("Misbehaving task finished...\n");

	for (;;)
	{
		vTaskDelay(1000);
		printf("Alive\n");
	}
}
#endif

#if 1 /* Temporary testing of semihosted mbed filesystem */
#include "device_manager.h"
#include <fcntl.h>
#include <errno.h>
extern "C" {
	extern struct FileLikeObj SemiFS_FLO;
	struct FileLikeObj *semi = &SemiFS_FLO;
}
#endif

int main()
{
	// Start the standard demo tasks.
	vStartLEDFlashTasks(mainFLASH_PRIORITY | portPRIVILEGE_BIT);
	/*
	vStartBlockingQueueTasks(mainBLOCK_Q_PRIORITY);
	vCreateBlockTimeTasks();
	vStartGenericQueueTasks(mainGEN_QUEUE_TASK_PRIORITY);
	vStartQueuePeekTasks();
	vStartDynamicPriorityTasks();
	vStartWebserverTask();
	*/

#ifdef CORE_HAS_MPU
	xTaskCreate(xBadTask, (signed char *)"Bad", configMINIMAL_STACK_SIZE + 800, (void *)NULL, tskIDLE_PRIORITY | portPRIVILEGE_BIT, NULL);
#endif

#if 1 /* Temporary testing of semihosted mbed filesystem */
	printf("Trying to open a file...\n");
	int fd = semi->open_("test.txt", O_CREAT | O_APPEND, 0x755);
	if (fd != -1)
	{
		char str[] = "hello world";
		printf("Trying to write %d bytes to /semifs/test.txt (fd=%d)...\n", strlen(str), fd);
		ssize_t nwritten = semi->write_(fd, str, strlen(str));
		if (nwritten != (ssize_t)strlen(str))
			printf("Failed to write (%d) %s\n", nwritten, strerror(errno));
		semi->close_(fd);
	}
	else
		printf("Failed to open /semifs/test.txt for writing\n");
#endif

	printf("Starting scheduler.\n");

	// Start the scheduler.
	vTaskStartScheduler();

	// Will only get here if there was insufficient memory to create the idle task.
	// Wait for WDT to reset.
	PowerManagement_PowerDown();
}

#if configUSE_TICK_HOOK == 1
extern "C" void vApplicationTickHook()
{
	static unsigned portLONG ulTicksSinceLastDisplay = 0;

	// Called from every tick interrupt. Have enough ticks passed to make it
	// time to perform our health status check again?
	ulTicksSinceLastDisplay++;
	if( ulTicksSinceLastDisplay >= mainCHECK_DELAY )
	{
		ulTicksSinceLastDisplay = 0;

		WDT_Feed();

#if configGENERATE_RUN_TIME_STATS == 1
		int8_t *taskListBuffer = (int8_t *)malloc(40 * uxTaskGetNumberOfTasks());
		if (taskListBuffer != NULL)
		{
			vTaskGetRunTimeStats((int8_t *)taskListBuffer);
			puts((const char *)taskListBuffer);
			free(taskListBuffer);
		}
#endif

		// Has an error been found in any task?
		int allGood = 1;
#if 0
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
#endif
		if (allGood == 1)
		{
			printf("All Good.\n");
		}
	}
}
#endif

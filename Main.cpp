/*
    FreeRTOS V6.1.0 - Copyright (C) 2010 Real Time Engineers Ltd.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
*/

#include <string.h>
#include <stdlib.h>
//#include "drivers/wdt.h"
//#include "drivers/uart.h"
//#include "drivers/dma_memcpy.h"
#include "power_management.h"
#include "console.h"

// Scheduler includes:
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Demo app includes:
#include "example_tasks/BlockQ.h"
#include "example_tasks/death.h"
#include "example_tasks/blocktim.h"
#include "example_tasks/flash.h"
#include "example_tasks/GenQTest.h"
#include "example_tasks/QPeek.h"
#include "example_tasks/dynamic.h"
#include "example_tasks/webserver_task.h"

// Demo application definitions.
#define mainCHECK_DELAY					((portTickType)5000 / portTICK_RATE_MS)
#define mainBLOCK_Q_PRIORITY			(tskIDLE_PRIORITY + 2)
#define mainFLASH_PRIORITY              (tskIDLE_PRIORITY + 2)
#define mainGEN_QUEUE_TASK_PRIORITY		(tskIDLE_PRIORITY)

#ifdef CORE_HAS_MPU
void xBadTask(void *params)
{
	// The privileged bit is set when this function is started, so it initially
	// runs in privileged mode. We first turn on an LED (which requires
	// privileges), lower our privileges to user mode, and again try to access
	// the LED. LED access happens through raw access to the registers, rather
	// than through the GPIO driver, because the driver might be privileged.
	
	printf("Bad task: started in privileged mode. Turning LED on...\n");
	//LPC_GPIO1->FIOSET = ( 1UL << 23UL );
	vTaskDelay(150);

	// Now switch to user mode and try again
	portSWITCH_TO_USER_MODE();
	printf("Bad task: switched to user mode. Turning LED off...\n");
	//LPC_GPIO1->FIOCLR = ( 1UL << 23UL );

	printf("Bad task: finished.\n");

	for (;;)
	{
		vTaskDelay(1000);
		printf("Bad task: still alive\n");
	}
}
#endif

/*
void simpleSerialTask(void *p)
{
	(void)p;
	int num = 0;
	for (;;)
	{
		char buff[10];
		int numWaiting = uart0->BytesWaiting();
		if (num != numWaiting)
		{
			num = numWaiting;
			printf("Num in buff: %d\n", num);
		}
		if (num > 2)
		{
			num = uart0->Read(buff, num);
			buff[num] = '\0';
			//uart0->write(buff, num);
			printf("\"%s\"\n", buff);
		}
		vTaskDelay(500);
	}
}
*/

#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

extern "C" void handler(int sig)
{
	printf("signal %d!\n", sig);
}

int main()
{
#if 1
	signal(SIGRTMIN+1, &handler);
	raise(SIGRTMIN+1);
#endif

	// Start the standard demo tasks.
	vStartLEDFlashTasks(mainFLASH_PRIORITY | portPRIVILEGE_BIT);
#if 0
	vStartBlockingQueueTasks(mainBLOCK_Q_PRIORITY);
	vCreateBlockTimeTasks();
	vStartGenericQueueTasks(mainGEN_QUEUE_TASK_PRIORITY);
	vStartQueuePeekTasks();
	vStartDynamicPriorityTasks();
#endif
#if 0
	vStartWebserverTask();
#endif

#if 0
	char *name = "testfile";
	int len = strlen(name);
	DIR *dirp = opendir("/sdcard");
	struct dirent *dp;
	while ((dp = readdir(dirp)) != NULL)
		if (dp->d_namlen == len && !strcmp(dp->d_name, name)) {
			(void)closedir(dirp);
			printf("found\n");
		}
	(void)closedir(dirp);
	printf("not_found\n");
#endif

#ifdef CORE_HAS_MPU
	xTaskCreate(xBadTask, (signed char *)"BadTask", configMINIMAL_STACK_SIZE, (void *)NULL, tskIDLE_PRIORITY | portPRIVILEGE_BIT, NULL);
#endif
	//xTaskCreate(simpleSerialTask, (signed char *)"Ser", configMINIMAL_STACK_SIZE, (void *)NULL, tskIDLE_PRIORITY | portPRIVILEGE_BIT, NULL);

	printf("Starting scheduler.\n");
	fflush(stdout);

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

		//WDT_Feed();

#if configGENERATE_RUN_TIME_STATS == 1
		unsigned long long uptime_usec = ullTaskGetSchedulerUptime();

#if 1
		struct timeval tp;
		int t = gettimeofday(&tp, NULL);
		printf("timeofday = %ld seconds %ld microseconds (code %d)\n", (long)tp.tv_sec, (long)tp.tv_usec, t);
#endif

		printf("Uptime: %u.%06u seconds\n", (unsigned int)(uptime_usec / 1000000), (unsigned int)(uptime_usec % 1000000));

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
		fflush(stdout);
	}
}
#endif


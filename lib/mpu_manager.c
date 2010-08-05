/* Robert Turner <rwt33@uclive.ac.nz>, July 23, 2010. */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "mpu_manager.h"

struct MPU_TaskKiller_t
{
	xTaskHandle handle;
};

static xQueueHandle xKillQueue;
#define KILL_QUEUE_LEN	(3)


void MPUManager_Idle(void)
{	
	struct MPU_TaskKiller_t task;
	if (xQueueReceive(xKillQueue, &task, 0))
	{
		xTaskHandle handle = task.handle;
		if (handle)
		{
			// Do some reporting etc here
			printf("[FreeRTOS] Killed task %d due to MPU protection violation.\n",
					(int)handle);
			vTaskDelete(handle);
		}
	}
}


void MPUManager_Init(void)
{
	xKillQueue = xQueueCreate(KILL_QUEUE_LEN, sizeof(struct MPU_TaskKiller_t));
}


bool MPUManager_HandleFault(uint32_t pc, uint32_t faultAddr) PRIVILEGED_FUNCTION
{
	// FIXME check it was application code (not 
	// ISR or library code) that faulted:
	bool will_handle = true;

	if (will_handle)
	{
		puts("Access Violation. (attempting to handle).");
		printf("\tpc : [<%08x>]  bad_access : [<%08x>]\n", pc, faultAddr);

		// Get task handle
		xTaskHandle handle = xTaskGetCurrentTaskHandle();
		puts("got task handle");
		
		// Suspend task
		vTaskSuspend(handle);
		puts("suspended task");

		// Add task to kill queue
		struct MPU_TaskKiller_t task = {
			.handle = handle,
		};
		signed portBASE_TYPE yield;
		xQueueSendFromISR(xKillQueue, &task, &yield);
		puts("added to kill queue");
		if (yield)
			portYIELD_WITHIN_API();
	}
	return will_handle;
}


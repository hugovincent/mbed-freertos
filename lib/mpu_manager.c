/* Robert Turner <rwt33@uclive.ac.nz>, July 23, 2010. */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "mpu_manager.h"

struct MPU_TaskKiller_t
{
	xTaskHandle handle;
	unsigned int pc;
	unsigned int faultAddr;
	// FIXME mode stuff could be captured too
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
			vTaskDelete(handle);

			// Do some reporting etc here
			const signed char * const name = pcTaskGetName(handle);

			printf("[FreeRTOS] Killed task \"%s\" due to MPU protection violation.\n", name);
			printf("\tpc : [<%08x>]  bad_access : [<%08x>]\n", (unsigned int)task.pc,
				(unsigned int)task.faultAddr);
			fflush(stdout);
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
		// Get task handle
		xTaskHandle handle = xTaskGetCurrentTaskHandleFromISR();

		// Suspend task
		vTaskSuspendFromISR(handle);

		// Add task to kill queue
		struct MPU_TaskKiller_t task = {
			.handle = handle,
			.pc = pc,
			.faultAddr = faultAddr,
		};
		signed portBASE_TYPE yield;
		xQueueSendFromISR(xKillQueue, &task, &yield);
		if (yield)
			portYIELD_WITHIN_API();
	}
	return will_handle;
}


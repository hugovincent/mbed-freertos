/* Task manager: responsible for cleanly killing and tearing down errant
 * tasks. TODO: will later also be responsible for doing task startup/init.
 * TODO: integrate with signal handling (SIGSEGV etc.)
 *
 * Robert Turner <rwt33@uclive.ac.nz>, July 23, 2010.
 * Hugo Vincent <hugo.vincent@gmail.com>, August 14, 2010.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "task_manager.h"

struct TaskKillInfo_t
{
	TaskManager_FaultType_t type;
	xTaskHandle handle;
	unsigned int pc;
	unsigned int faultAddr;
	// FIXME mode stuff could be captured too
};

static xQueueHandle xKillQueue;
#define KILL_QUEUE_LEN	(3)


void TaskManager_Idle(void)
{
	struct TaskKillInfo_t task;
	if (xQueueReceive(xKillQueue, &task, 0))
	{
		xTaskHandle handle = task.handle;
		if (handle)
		{
			vTaskDelete(handle);

			// Do some reporting etc here
			const signed char * const name = pcTaskGetName(handle);

			switch (task.type)
			{
				case TaskManager_MPUFault:
					printf("[FreeRTOS] Killed task \"%s\" due to MPU protection violation.\n", name);
					printf("\tpc : [<%08x>]  bad_access : [<%08x>]\n", (unsigned int)task.pc,
						(unsigned int)task.faultAddr);
					break;
				default:
					printf("[FreeRTOS] Killed task \"%s\" due to FIXME?\n", name);
					break;
			}
			fflush(stdout);
		}
	}
}


void TaskManager_Init(void)
{
	xKillQueue = xQueueCreate(KILL_QUEUE_LEN, sizeof(struct TaskKillInfo_t));
}


bool TaskManager_HandleFault(TaskManager_FaultType_t type, uint32_t pc, uint32_t faultAddr) PRIVILEGED_FUNCTION
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
		struct TaskKillInfo_t task = {
			.type = type,
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


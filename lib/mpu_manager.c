/* Robert Turner <rwt33@uclive.ac.nz>, July 23, 2010. */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "mpu_manager.h"

static void MPUFault_Handler(void);

static xQueueHandle xKillQueue;

typedef struct 
{
	xTaskHandle handle;
} taskKiller_t;

#define KILL_QUEUE_LEN	(3)

void Mpu_Idle(void)
{	
	taskKiller_t task;
	if (xQueueReceive(xKillQueue, &task, 0))
	{
		xTaskHandle handle = task.handle;
		if (handle)
		{
			// Do some reporting etc here
			vTaskDelete(handle);
		}
	}
}

void MpuManager_Init(void)
{
	xKillQueue = xQueueCreate(KILL_QUEUE_LEN, sizeof(taskKiller_t));
	NVIC_SetVector(MemoryManagement_IRQn, (unsigned int)MPUFault_Handler);
}


static inline void mpuFaultCleanup(void)
{
	signed portBASE_TYPE yield;
	taskKiller_t task;
	xTaskHandle handle = xTaskGetCurrentTaskHandleFromISR();
	//vTaskSuspendFromISR(handle);
	vTaskSuspend(handle);
	task.handle = handle;
	xQueueSendFromISR(xKillQueue, &task, &yield);
	if (yield)
		portYIELD_WITHIN_API();
}


void MPUFault_Handler(void)
{
#if 0
	unsigned int cfsr = SCB->CFSR;
	bool mmarValid = cfsr & CFSR_MMARVALID ? true : false,
		mstkErr = cfsr & CFSR_MSTKERR ? true : false, // stacking from excpetion caused access violation
		munstkErr = cfsr & CFSR_MUNSTKERR ? true : false, // as above but unstacking from exception
		dataAccViol = cfsr & CFSR_DACCVIOL ? true : false, // data access violation. return PC points to instruction and loads addr in MMAR
		instAccViol = cfsr & CFSR_IACCVIOL ? true : false; //instruction access violation. return PC points to instruction (no MMAR)
	unsigned int mpuFaultAddr = mmarValid ? SCB->MMFAR : 0;
#endif
	mpuFaultCleanup();
}



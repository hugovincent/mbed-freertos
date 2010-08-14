#ifndef Task_Manager_H
#define Task_Manager_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	TaskManager_MPUFault,
	// FIXME
} TaskManager_FaultType_t;

void TaskManager_Init();
void TaskManager_Idle();
bool TaskManager_HandleFault(TaskManager_FaultType_t type, uint32_t pc, uint32_t faultAddr);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifdef Task_Manager_H


#ifndef MPU_Manager_H
#define MPU_Manager_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void MPUManager_Init();
void MPUManager_Idle();
bool MPUManager_HandleFault(uint32_t pc, uint32_t faultAddr);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifdef MPU_Manager_H


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

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdio.h>
#include <cmsis.h>

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE. 
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION					1
#define configUSE_IDLE_HOOK						1
#define configUSE_TICK_HOOK						1
#define configCPU_CLOCK_HZ						( ( unsigned portLONG ) 72000000 )
#define configTICK_RATE_HZ						( ( portTickType ) 100 )
#define configMAX_PRIORITIES					( ( unsigned portBASE_TYPE ) 5 )
#define configMINIMAL_STACK_SIZE				( 120 )
#define configUSE_TRACE_FACILITY				0
#define configUSE_16_BIT_TICKS					0
#define configIDLE_SHOULD_YIELD					1
#define configUSE_MUTEXES						1

#ifdef MBED_LPC17xx
#define configMAX_SYSCALL_INTERRUPT_PRIORITY	1 // FIXME!
#endif

/* Debugging/testing options */
#define configCHECK_FOR_STACK_OVERFLOW			2
#define configGENERATE_RUN_TIME_STATS			0
#define configUSE_MALLOC_FAILED_HOOK			1

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES					0
#define configMAX_CO_ROUTINE_PRIORITIES			2

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet				1
#define INCLUDE_uxTaskPriorityGet				1
#define INCLUDE_vTaskDelete						1
#define INCLUDE_vTaskCleanUpResources			1
#define INCLUDE_vTaskSuspend         			1
#define INCLUDE_vTaskDelayUntil					1
#define INCLUDE_vTaskDelay						1
#define INCLUDE_xTaskGetCurrentTaskHandle		1
#define INCLUDE_uxTaskGetStackHighWaterMark		1
#define INCLUDE_xTaskGetSchedulerState			1

#define PACK_STRUCT_END							__attribute((packed))
#define ALIGN_STRUCT_END						__attribute((aligned(4)))

#endif /* FREERTOS_CONFIG_H */


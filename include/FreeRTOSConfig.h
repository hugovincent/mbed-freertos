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

#ifdef __cplusplus
extern "C" {
#endif

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
#define configCPU_CLOCK_HZ						( SystemCoreClock  )
#define configTICK_RATE_HZ						( ( portTickType ) 1000 )
#define configMAX_PRIORITIES					( ( unsigned portBASE_TYPE ) 5 )
#define configMINIMAL_STACK_SIZE				( 120 )
#define configUSE_TRACE_FACILITY				0
#define configUSE_CO_ROUTINES					0
#define configUSE_16_BIT_TICKS					0
#define configIDLE_SHOULD_YIELD					1
#define configUSE_MUTEXES						1
#define configIDLE_STACK_SIZE					( 800 )


#if defined(TARGET_LPC17xx)
#ifdef __NVIC_PRIO_BITS
	#define configPRIO_BITS       		__NVIC_PRIO_BITS
#else
	#define configPRIO_BITS       		5        /* 32 priority levels */
#endif
/* LOW NUMBERS ARE HIGHER PRIORITY! */
#define configKERNEL_INTERRUPT_PRIORITY 		( 31 << (8 - configPRIO_BITS) )
/* This is max any driver ISR should be: */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( 5 << (8 - configPRIO_BITS) )
/* This is the prio that SVC calls should be. Need to have higher prio than sys calls so that they're never disabled */
#define configSVC_INTERRUPT_PRIORITY			( 4 << (8 - configPRIO_BITS) )
#elif defined(TARGET_LPC23xx)
// Additions specific to this distribution of FreeRTOS
#define configIRQ_CAN_CONTEXT_SWITCH			0
#endif

/* Debugging/testing options */
#define configCHECK_FOR_STACK_OVERFLOW			2
#define configGENERATE_RUN_TIME_STATS			1
#define configUSE_MALLOC_FAILED_HOOK			1

/* MAC address configuration. 
 * For my mbed it is 00:02:f7:f0:33:eb. You should set this to your MAC address. */
#define configMAC_ADDR0		0x00
#define configMAC_ADDR1		0x02
#define configMAC_ADDR2		0xf7
#define configMAC_ADDR3		0xf0
#define configMAC_ADDR4		0x33
#define configMAC_ADDR5		0xeb

/* IP address configuration. */
#define configIP_ADDR0		192
#define configIP_ADDR1		168
#define configIP_ADDR2		2
#define configIP_ADDR3		5

/* Netmask configuration. */
#define configNET_MASK0		255
#define configNET_MASK1		255
#define configNET_MASK2		255
#define configNET_MASK3		0

#define configEMAC_INTERRUPT_PRIORITY       5

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet				1
#define INCLUDE_uxTaskPriorityGet				1
#define INCLUDE_vTaskDelete						1
#define INCLUDE_vTaskCleanUpResources			1
#define INCLUDE_vTaskSuspend         			1
#define INCLUDE_vTaskDelayUntil					1
#define INCLUDE_vTaskDelay						1
#define INCLUDE_pcTaskGetName					1
#define INCLUDE_xTaskGetCurrentTaskHandle		1
#define INCLUDE_uxTaskGetStackHighWaterMark		1
#define INCLUDE_xTaskGetSchedulerState			1

#define PACK_STRUCT_END							__attribute((packed))
#define ALIGN_STRUCT_END						__attribute((aligned(4)))

/* Optional run-time statistics output (like `top`). Uses Timer 1. */
#if configGENERATE_RUN_TIME_STATS == 1
extern void vConfigureTimerForRunTimeStats( void );
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()	vConfigureTimerForRunTimeStats()
#define portGET_RUN_TIME_COUNTER_VALUE()			(LPC_TIM1->TC)
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* FREERTOS_CONFIG_H */


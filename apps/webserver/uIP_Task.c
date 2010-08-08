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

/* Standard includes. */
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* uip includes. */
#include "uip/uip.h"
#include "uip/uip_arp.h"
#include "uip/timer.h"
#include "uip/clock-arch.h"
#include "httpd.h"

/* Demo includes. */
#include "debug_support.h"
#include "drivers/emac/EthDev_LPC17xx.h"
#include "drivers/emac/EthDev.h"
#include "drivers/gpio.h"

/*-----------------------------------------------------------*/

/* How long to wait before attempting to connect the MAC again. */
#define uipINIT_WAIT    ( 100 / portTICK_RATE_MS )

/* Shortcut to the header within the Rx buffer. */
#define xHeader ((struct uip_eth_hdr * restrict) &uip_buf[ 0 ])

/* Standard constant. */
#define uipTOTAL_FRAME_HEADER_SIZE	54

/*-----------------------------------------------------------*/

/*
 * Setup the MAC address in the MAC itself, and in the uIP stack.
 */
static void prvSetMACAddress( void );

/*
 * Port functions required by the uIP stack.
 */
void clock_init( void );
clock_time_t clock_time( void );


/* The semaphore used by the ISR to wake the uIP task. */
xSemaphoreHandle xEMACSemaphore = NULL;


void clock_init(void)
{
	/* This is done when the scheduler starts. */
}

clock_time_t clock_time( void )
{
	return xTaskGetTickCount();
}

void vuIP_Task( void *pvParameters )
{
portBASE_TYPE i;
uip_ipaddr_t xIPAddr;
struct timer periodic_timer, arp_timer;
extern void ( vEMAC_ISR_Wrapper )( void );

	( void ) pvParameters;

	/* Initialise the MAC. */
	while( lEMACInit() != pdPASS )
    {
        vTaskDelay( uipINIT_WAIT );
    }

	/* Initialise the uIP stack. */
	timer_set( &periodic_timer, configTICK_RATE_HZ / 2 );
	timer_set( &arp_timer, configTICK_RATE_HZ * 10 );
	uip_init();
	uip_ipaddr( xIPAddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3 );
	uip_sethostaddr( xIPAddr );
	uip_ipaddr( xIPAddr, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3 );
	uip_setnetmask( xIPAddr );
	httpd_init();

	printf("Initialized uIP stack and httpd (IP address %d.%d.%d.%d, port 80)\n",
			uip_hostaddr[0] & 0xFF, uip_hostaddr[0] >> 8,
			uip_hostaddr[1] & 0xFF, uip_hostaddr[1] >> 8);

	/* Create the semaphore used to wake the uIP task. */
	vSemaphoreCreateBinary( xEMACSemaphore );

	portENTER_CRITICAL();
	{
		LPC_EMAC->IntEnable = ( INT_RX_DONE | INT_TX_DONE );

		/* Set the interrupt priority to the max permissible to cause some
		interrupt nesting. */
		NVIC_SetPriority( ENET_IRQn, configEMAC_INTERRUPT_PRIORITY );

		extern void vEMAC_ISR();
		NVIC_SetVector(ENET_IRQn, (portLONG)vEMAC_ISR);

		/* Enable the interrupt. */
		NVIC_EnableIRQ( ENET_IRQn );
		prvSetMACAddress();
	}
	portEXIT_CRITICAL();

	for( ;; )
	{
		/* Is there received data ready to be processed? */
		uip_len = ulGetEMACRxData();

		if( ( uip_len > 0 ) && ( uip_buf != NULL ) )
		{
			/* Standard uIP loop taken from the uIP manual. */
			if( xHeader->type == htons( UIP_ETHTYPE_IP ) )
			{
				printf("got IP packet\n");
				uip_arp_ipin();
				uip_input();

				/* If the above function invocation resulted in data that 
				   should be sent out on the network, the global variable 
				   uip_len is set to a value > 0. */
				if( uip_len > 0 )
				{
					uip_arp_out();
					vSendEMACTxData( uip_len );
				}
			}
			else if( xHeader->type == htons( UIP_ETHTYPE_ARP ) )
			{
				uip_arp_arpin();

				/* If the above function invocation resulted in data that 
				   should be sent out on the network, the global variable 
				   uip_len is set to a value > 0. */
				if( uip_len > 0 )
				{
					vSendEMACTxData( uip_len );
				}
			}
			else
				printf("got unknown eth packet (header type = %d)\n", xHeader->type);
		}
		else
		{
			if( timer_expired( &periodic_timer ) && ( uip_buf != NULL ) )
			{
				timer_reset( &periodic_timer );
				for( i = 0; i < UIP_CONNS; i++ )
				{
					uip_periodic( i );

					/* If the above function invocation resulted in data that 
					   should be sent out on the network, the global variable 
					   uip_len is set to a value > 0. */
					if( uip_len > 0 )
					{
						uip_arp_out();
						vSendEMACTxData( uip_len );
					}
				}

				/* Call the ARP timer function every 10 seconds. */
				if( timer_expired( &arp_timer ) )
				{
					timer_reset( &arp_timer );
					uip_arp_timer();
				}
			}
			else
			{
				/* We did not receive a packet, and there was no periodic
				   processing to perform.  Block for a fixed period.  If a packet
				   is received during this period we will be woken by the ISR
				   giving us the Semaphore. */
				xSemaphoreTake( xEMACSemaphore, configTICK_RATE_HZ / 2 );			
			}
		}
	}
}

static void prvSetMACAddress( void )
{
struct uip_eth_addr xAddr;

	/* Extract mbed officially allocated ethernet MAC address
	 * (code snipped supplied by Simon Ford @ mbed). 
	 */
	char uid_string[36];
	uint32_t args[2];
	args[0] = (uint32_t)&uid_string;
	args[1] = 33;
	SemihostCall(Semihost_USR_UID, &args);

	/* Configure the MAC address in the uIP stack. */
	int tmp; // (sscanf seems to choke with alignment issues when we assign
			 // directly within the sscanf call, so we have to do it like this)
	sscanf(&uid_string[20], "%2x", &tmp); xAddr.addr[0] = tmp;
	sscanf(&uid_string[22], "%2x", &tmp); xAddr.addr[1] = tmp;
	sscanf(&uid_string[24], "%2x", &tmp); xAddr.addr[2] = tmp;
	sscanf(&uid_string[26], "%2x", &tmp); xAddr.addr[3] = tmp;
	sscanf(&uid_string[28], "%2x", &tmp); xAddr.addr[4] = tmp;
	sscanf(&uid_string[30], "%2x", &tmp); xAddr.addr[5] = tmp;
	uip_setethaddr( xAddr );

	printf("Ethernet hardware initialized (mbed MAC address %02x:%02x:%02x:%02x:%02x:%02x).\n", 
			xAddr.addr[0], xAddr.addr[1], xAddr.addr[2],
			xAddr.addr[3], xAddr.addr[4], xAddr.addr[5]);
}

void vApplicationProcessFormInput( char *pcInputString )
{
char *c;

	/* Process the form input sent by the IO page of the served HTML. */
	c = strstr( pcInputString, "?" );
    if( c )
    {
		/* Turn LED's on or off in accordance with the check box status. */
		if( strstr( c, "LED0=1" ) != NULL )
			GPIO_PinSet(0, 4);
		else
			GPIO_PinClear(0, 4);
	}
}

void vStartWebserverTask()
{
	// Create the uIP task. This uses the lwIP RTOS abstraction layer.
	xTaskCreate( vuIP_Task, ( signed portCHAR * ) "httpd",
			configMINIMAL_STACK_SIZE * 6, NULL, (tskIDLE_PRIORITY + 2) | portPRIVILEGE_BIT, NULL );
}


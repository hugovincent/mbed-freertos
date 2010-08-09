/* Board-specific hardware initialisation. Must set up the console serial 
 * port. 
 *
 * Hugo Vincent, 2 May 2010.
 */

#include "FreeRTOS.h"

#include "os_init.h"
#include "semifs.h"
#include "console.h"

#include "drivers/gpio.h"
#include "drivers/wdt.h"
#include "drivers/uart.h"

extern void vPortSVCHandler( void );
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);


void Board_EarlyInit( void )
{
	// Disable TPIU.
	LPC_PINCON->PINSEL10 = 0;

#if defined(TARGET_LPC17xx)

	// Set system call and system timer tick interrupts (priorities for these are set by kernel)
	NVIC_SetVector(SVCall_IRQn, (unsigned int)vPortSVCHandler);
	NVIC_SetVector(PendSV_IRQn, (unsigned int)xPortPendSVHandler);
	NVIC_SetVector(SysTick_IRQn, (unsigned int)xPortSysTickHandler);

#elif defined(TARGET_LPC23xx)

	// FIXME

#endif

	// FIXME This is where pinmux configuration should get done

	uart0 = UART_Init(/* which: */ 0, /* Tx buffer size: */ 128, 
			/* Rx buffer size: */ 128, /* use DMA: */ true);
	UART_SetBaud(uart0, 115200);

	WDT_Init(/* timeout in seconds: */ 6);
}

void Board_LateInit()
{
	GPIO_Init();
	//RTC_Init();

	SemiFS_Init();
}


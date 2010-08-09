#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "console.h"

#include "drivers/uart.h"

// Any system requires at least one UART that console goes on. We call this uart0
UART *uart0;

static xSemaphoreHandle uart0mutex;

void Console_EarlyInit()
{
	// FIXME:
	extern void initialise_stdio();
	initialise_stdio();

	uart0mutex = xSemaphoreCreateMutex();
}

void Console_LateInit()
{
	// FIXME register with device manager
}

void Console_SingleMode()
{
	portDISABLE_INTERRUPTS();
	// FIXME switch over stdio output
}

/* ------------------------------------------------------------------------- */
// FIXME:

int uart0write(const char *buff, size_t len)
{
	bool running = xTaskGetSchedulerState() == taskSCHEDULER_RUNNING;
	if (running)
	{
		while (!xSemaphoreTake(uart0mutex, portMAX_DELAY))
			;
	}
	char *tmp = (char *)buff;
	while (tmp < ((char *)buff + len))
	{
		tmp += UART_Write(uart0, tmp, ((char *)buff + len) - tmp);
	}
	if (running)
		xSemaphoreGive(uart0mutex);
	return len;
}

int uart0writeDebug(const char *buff, size_t len)
{
	return UART_WriteUnbuffered(uart0, buff, len);
}


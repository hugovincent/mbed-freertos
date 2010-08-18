#include <string.h>
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

int uart0write_(const char *buf, size_t len)
{
#if defined(TARGET_LPC23xx)
	// DMA currently doesn't work on the LPC23xx
	UART_WriteUnbuffered(uart0, buf, len);
#else
	const char *tmp = buf;
	do
	{
		tmp += UART_Write(uart0, tmp, (buf + len) - tmp);
	}
	while (tmp < (buf + len));
#endif
	return len;
}

int uart0write(const char *buf, size_t len)
{
	bool running = (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING);
	if (running)
	{
		while (!xSemaphoreTake(uart0mutex, portMAX_DELAY))
			;
	}

#ifndef configCONSOLE_NO_CRLF
	// Newline -> CRLF replacement (zero copy):
	char *nl = memchr(buf, '\n', len);
	if (nl != NULL)
	{
		char *tmp = (char *)buf;
		do
		{
			tmp += uart0write_(tmp, nl - tmp) + 1;
			uart0write_("\r\n", 2);
			if (tmp >= (buf + len))
				break;
			nl = memchr(tmp, '\n', buf + len - tmp);
		} while (nl != NULL);
		if (tmp < buf + len)
			uart0write_(tmp, buf + len - tmp);
	}
	else
#endif
	{
		uart0write_(buf, len);
	}

	if (running)
		xSemaphoreGive(uart0mutex);
	return len;
}

int uart0writeDebug(const char *buf, size_t len)
{
	return UART_WriteUnbuffered(uart0, buf, len);
}


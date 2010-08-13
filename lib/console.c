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
	char *tmp = (char *)buf;
	while (tmp < ((char *)buf + len))
	{
#ifdef TARGET_LPC23xx
		// DMA currently doesn't work on the LPC23xx
		tmp += UART_WriteUnbuffered(uart0, tmp, ((char *)buf + len) - tmp);
#else
		tmp += UART_Write(uart0, tmp, ((char *)buf + len) - tmp);
#endif
	}
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

	// FIXME CR stripping
	// Newline -> CRLF replacement (zero copy):
	char *nl = strchr(buf, '\n');
	if (nl != NULL)
	{
		char *tmp = (char *)buf;
		int printed = 0;
		while (nl != NULL)
		{
			printed += uart0write_(tmp, nl - tmp) + 1;
			uart0write_("\r\n", 2);
			tmp = nl + 1;
			if (tmp > (buf + len))
				break;
			nl = strchr(tmp, '\n');
		}
		if (len - printed > 0)
			uart0write_(tmp, len - printed);
	}
	else
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


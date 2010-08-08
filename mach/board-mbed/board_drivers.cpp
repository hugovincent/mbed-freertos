
#include "drivers/drivers.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

extern "C" void DRIVER_Init();

Uart *uart0;

static xSemaphoreHandle uart0mutex;

void DRIVER_Init()
{
	Uart::init();
	Gpdma::init();
	Gpdma *dmaTx = Gpdma::getChannel();
	Gpdma *dmaRx = Gpdma::getChannel();
	uart0 = new Uart(/* UART: */ 0, /* Tx buffer size: */ 128, /* Rx buffer size: */ 128, dmaTx, dmaRx);
	uart0->setBaud(115200);
	uart0mutex = xSemaphoreCreateMutex();
}

extern "C"
{
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
			//tmp += u0->writeUnbuffered(tmp, ((char *)buff + len) - tmp);
			tmp += uart0->write(tmp, ((char *)buff + len) - tmp);
		}
		if (running)
			xSemaphoreGive(uart0mutex);
		return len;
	}

	int uart0writeDebug(const char *buff, size_t len)
	{
		return uart0->writeUnbuffered(buff, len);
	}
}


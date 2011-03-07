#include <drivers/uart.h>
#include <math.h>
#include <string.h>

#if defined(TARGET_LPC17xx)

// UART register bits
#define UART_LCR_DLAB		(0x80)
#define UART_LCR_NOPAR		(0x00)
#define UART_LCR_1STOP		(0x00)
#define UART_LCR_8BITS		(0x03)
#define UART_IER_EI			(0x07)
#define UART_FCR_EN			(0x01)
#define UART_FCR_CLR		(0x06)
#define UART_FCR_DMA		(1UL<<3)
#define UART_LSR_TEMT		(0x40)

	UART::UART(int deviceNum, size_t txFifoLen, size_t rxFifoLen, bool useDMA)
: m_DevNum(deviceNum), m_Base(0), m_RxDMA(0), m_TxDMA(0)
{
	switch (m_DevNum)
	{
		case 0:
			LPC_SC->PCONP |= 0x1<<3;
			LPC_SC->PCLKSEL0 &= ~(0x3<<6);
			m_Base = (LPC_UART_TypeDef*)LPC_UART0_BASE;
			break;
		case 1:
			LPC_SC->PCONP |= 0x1<<4;
			LPC_SC->PCLKSEL0 &= ~(0x3<<8);
			m_Base = (LPC_UART_TypeDef*)LPC_UART1_BASE;
			break;
		case 2:
			LPC_SC->PCONP |= 0x1<<24;
			LPC_SC->PCLKSEL1 &= ~(0x3<<16);
			m_Base = (LPC_UART_TypeDef*)LPC_UART2_BASE;
			break;
		case 3:
			LPC_SC->PCONP |= 0x1<<25;
			LPC_SC->PCLKSEL1 &= ~(0x3<<18);
			m_Base = (LPC_UART_TypeDef*)LPC_UART3_BASE;
			break;
		default:
			m_DevNum = -1;
	}

	if (useDMA)
	{
		GPDMA::init(); // FIXME
		GPDMA *dmaTx = GPDMA::getChannel();
		GPDMA *dmaRx = GPDMA::getChannel();
		m_TxDMA = new DmaM2P<char>(GPDMA::UART0Tx_Mat00, (char*)&(m_Base->THR), txFifoLen, dmaTx);
		m_RxDMA = new DmaP2M<char>(GPDMA::UART0Rx_Mat01, (char*)&(m_Base->RBR), rxFifoLen, dmaRx);
	}

	// Turn on the FIFO's and clear the buffers
	m_Base->FCR = UART_FCR_EN | UART_FCR_CLR;// | ((m_TxDMA || m_RxDMA) ? UART_FCR_DMA : 0);

	if (m_RxDMA || m_TxDMA)
		m_Base->FCR |= UART_FCR_DMA;
}

const UART::FractionalBaudEntry UART::FractionalBaudTable[] =
{
	{1.000,0,1},
	{1.067,1,15},
	{1.071,1,14},
	{1.077,1,13},
	{1.083,1,12},
	{1.091,1,11},
	{1.100,1,10},
	{1.111,1,9},
	{1.125,1,8},
	{1.133,2,15},
	{1.143,1,7},
	{1.154,2,13},
	{1.167,1,6},
	{1.182,2,11},
	{1.200,1,5},
	{1.214,3,14},
	{1.222,2,9},
	{1.231,3,13},
	{1.250,1,4},
	{1.267,4,15},
	{1.273,3,11},
	{1.286,2,7},
	{1.300,3,10},
	{1.308,4,13},
	{1.333,1,3},
	{1.357,5,14},
	{1.364,4,11},
	{1.375,3,8},
	{1.385,5,13},
	{1.400,2,5},
	{1.417,5,12},
	{1.429,3,7},
	{1.444,4,9},
	{1.455,5,11},
	{1.462,6,13},
	{1.467,7,15},
	{1.500,1,2},
	{1.533,8,15},
	{1.538,7,13},
	{1.545,6,11},
	{1.556,5,9},
	{1.571,4,7},
	{1.583,7,12},
	{1.600,3,5},
	{1.615,8,13},
	{1.625,5,8},
	{1.636,7,11},
	{1.643,9,14},
	{1.667,2,3},
	{1.692,9,13},
	{1.700,7,10},
	{1.714,5,7},
	{1.727,8,11},
	{1.733,11,15},
	{1.750,3,4},
	{1.769,10,13},
	{1.778,7,9},
	{1.786,11,14},
	{1.800,4,5},
	{1.818,9,11},
	{1.833,5,6},
	{1.846,11,13},
	{1.857,6,7},
	{1.867,13,15},
	{1.875,7,8},
	{1.889,8,9},
	{1.900,9,10},
	{1.909,10,11},
	{1.917,11,12},
	{1.923,12,13},
	{1.929,13,14},
	{1.933,14,15}
};

void UART::FindBaudWithFractional(uint32_t wantedBaud, uint32_t *divisor, uint32_t *fracDiv)
{
	float FRest = 1.5;
	int divAddVal = 0, mulVal = 1;

	// Setup the baud rate:  Calculate the divisor value.
	// Note: PCLK is CCLK/4, so the 16 in the equations becomes 64.
	*divisor = SystemCoreClock / (wantedBaud * 64);

	// Check for integer divisor, otherwise compute fractional divisors
	if (SystemCoreClock % (wantedBaud * 64) != 0)
	{
		*divisor = (uint32_t)floorf(SystemCoreClock / (wantedBaud * 64 * FRest));
		FRest = SystemCoreClock / (64 * wantedBaud * (float)(*divisor));
		if (FRest > 1.1 && FRest < 1.9)
		{
			for (unsigned char j = 0; j < 71; j++)
			{
				if (FractionalBaudTable[j].FRest > FRest
						&& FRest < FractionalBaudTable[j+1].FRest)
				{
					mulVal = FractionalBaudTable[j].mulVal;
					divAddVal = FractionalBaudTable[j].divAddVal;
					break;
				}
			}
		}
	}
	*fracDiv = (divAddVal & 0x0F) | ((mulVal & 0x0F) << 4);
}

void UART::SetBaud(uint32_t baud)
{
	uint32_t ulDivisor, ulFracDiv;

	// Setup a fractional baud rate
	FindBaudWithFractional(baud, &ulDivisor, &ulFracDiv);
	m_Base->FDR = ulFracDiv;

	// Set the DLAB bit so we can access the divisor
	m_Base->LCR = UART_LCR_DLAB;

	// Setup the divisor
	m_Base->DLL = (unsigned char)(ulDivisor & (uint32_t)0xff);
	ulDivisor >>= 8;
	m_Base->DLM = (unsigned char)(ulDivisor & (uint32_t)0xff);

	// Setup transmission format and clear the DLAB bit to enable transmission
	m_Base->LCR = UART_LCR_NOPAR | UART_LCR_1STOP | UART_LCR_8BITS;

	if (m_RxDMA)
		m_RxDMA->startReading();
}

int UART::Write(const char * buf, size_t len)
{
	if (len < 1)
		return 0;
	if (!m_TxDMA)
		return WriteUnbuffered(buf, len);

	return m_TxDMA->write(buf, len);
}

int UART::WriteUnbuffered(const char * buf, size_t len)
{
	if (len < 1)
		return 0;
	for (size_t i = 0; i < len; i++)
	{
		while (!(m_Base->LSR & (1 << 5)))
			;
		m_Base->THR = *(buf++);
	}
	return len;
}

int UART::Read(char * buf, size_t len)
{
	size_t num;
	if (len < 1)
		return 0;
	if (!m_RxDMA)
	{
		for (num = 0; num < len; num++)
		{
			if (LPC_UART0->LSR & 0x1) // Receive data ready
			{
				*buf++ = LPC_UART0->RBR & 0xFF;
			}
			else
				break;
		}
		return num;
	}
	return m_RxDMA->read(buf, len);
}

#elif defined(TARGET_EFM32)

#endif

/* ------------------------------------------------------------------------- */
// C Wrappers:

UART *UART_Init(int which, int txBufSize, int rxBufSize, bool useDma)
{
	return new UART(which, txBufSize, rxBufSize, useDma);
}

void UART_Deinit(UART *uart)
{
	delete uart;
}

void UART_SetBaud(UART *uart, int baud)
{
	uart->SetBaud(baud);
}

int UART_Read(UART *uart, char *buf, size_t len)
{
	return uart->Read(buf, len);
}

int UART_Write(UART *uart, const char *buf, size_t len)
{
	return uart->Write(buf, len);
}

int UART_WriteUnbuffered(UART *uart, const char *buf, size_t len)
{
	return uart->WriteUnbuffered(buf, len);
}


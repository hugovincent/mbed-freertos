/* DMA-driven UART driver for LPC17xx (should also work on LPC23xx but untested).
 *
 * Rob Turner, August 2010.
 */

#ifndef UART_h
#define UART_h

#include <stdlib.h>
#include <stdbool.h>

#include <cmsis.h>
#include <drivers/gpdma.h>

#ifdef __cplusplus

class UART
{
public:
	UART(int deviceNum, size_t txFifoLen, size_t rxFifoLen, bool useDMA);
	~UART();

	void SetBaud(uint32_t baud);

	int Read(char * buf, size_t len);
	int Write(const char * buf, size_t len);
	int WriteUnbuffered(const char * buf, size_t len);

	size_t BytesWaiting() { return m_RxDMA ? m_RxDMA->numWaiting() : 0; }

protected:

	inline size_t GetTxBuffLen() { return m_TxDMA ? m_TxDMA->getBuffLen() : 0; }
	inline size_t GetRxBuffLen() { return m_RxDMA ? m_RxDMA->getBuffLen() : 0; }

	struct FractionalBaudEntry {
		float FRest;
		uint8_t divAddVal, mulVal;
	};

	static const FractionalBaudEntry FractionalBaudTable[72];
	static void FindBaudWithFractional(uint32_t wantedBaud, uint32_t *divisor, uint32_t *fracDiv);

	int m_DevNum;
	LPC_UART_TypeDef *m_Base;
	DmaP2M<char> *m_RxDMA;
	DmaM2P<char> *m_TxDMA;
};

extern "C" {

#else // ifdef __cplusplus

#include <stdbool.h>
typedef struct UART UART;

#endif // ifdef __cplusplus

UART *UART_Init(int which, int txBufSize, int rxBufSize, bool useDma);
void UART_Deinit(UART *uart);
void UART_SetBaud(UART *uart, int baud);
int UART_Read(UART *uart, char *buf, size_t len);
int UART_Write(UART *uart, const char *buf, size_t len);
int UART_WriteUnbuffered(UART *uart, const char *buf, size_t len);

#ifdef __cplusplus
} // extern "C"
#endif // ifdef __cpluspls

#endif // UART_h


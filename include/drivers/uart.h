

#ifndef _H_UART_
#define _H_UART_

#include <stdlib.h>
#include <lpc17xx.h>
#include <drivers/gpdma.h>

class Uart
{
public:
	Uart(int deviceNum, size_t txFifoLen, size_t rxFifoLen, Gpdma *txDma = 0, Gpdma *rxDma = 0);

	static void init();

	void setBaud(uint32_t baud);

	int read(char * buf, size_t len);
	size_t numWaiting() { return _rxDma ? _rxDma->numWaiting() : 0; }
	int write(const char * buf, size_t len);
	int writeUnbuffered(const char * buf, size_t len);

	inline DmaM2P<char> * getTxDma() { return _txDma; }
	inline DmaP2M<char> * getDmaRx() { return _rxDma; }

	inline size_t getTxBuffLen() { return _txDma ? _txDma->getBuffLen() : 0; }
	inline size_t getRxBuffLen() { return _rxDma ? _rxDma->getBuffLen() : 0; }

protected:

	typedef struct {
		float FRest;
		uint8_t divAddVal, mulVal;
	} FracBaudLine_t;

	static FracBaudLine_t FractionalBaudTable[72];
	static void FindBaudWithFractional(uint32_t wantedBaud, uint32_t *divisor, uint32_t *fracDiv);

	int _devNum;
	LPC_UART_TypeDef *_base;
	DmaP2M<char> *_rxDma;
	DmaM2P<char> *_txDma;
};

#endif // _H_UART_


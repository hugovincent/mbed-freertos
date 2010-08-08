

// 


#ifndef _H_GPDMA_
#define _H_GPDMA_

#include <stdint.h>
#include <lpc17xx.h>
#include <stdlib.h>

class Gpdma
{
public:
	typedef struct 
	{
		uint32_t SrcAddr;
		uint32_t DestAddr;
		uint32_t NextLLI;
		uint32_t Control;
	} LLI_t;

	typedef enum
	{
		SSP0Tx,
		SSP0Rx,
		SSP1Tx,
		SSP1Rx,
		ADC,
		I2S0,
		I2S1,
		DAC,
		UART0Tx_Mat00,
		UART0Rx_Mat01,
		UART1Tx_Mat10,
		UART1Rx_Mat11,
		UART2Tx_Mat20,
		UART2Rx_Mat21,
		UART3Tx_Mat30,
		UART3Rx_Mat31	
	} PeripheralId_t;

	typedef enum
	{
		_1,
		_4,
		_8,
		_16,
		_32,
		_64,
		_128,
		_256
	} BurstSize_t;

	typedef enum
	{
		Byte,
		HalfWord,
		Word
	} TransferWidth_t;

	typedef enum
	{
		MemToMem,
		MemToPeriph,
		PeriphToMem,
		PeriphToPeriph
	} TransferType_t;

public:

/* Get a DMA channel
	 *	The lowest channel number has the highest priority
	 *	Either get a particular channel or get the next free one.
	 *	It is the responsibility of the requester to check if someone else is using the channel (check if active)
	 */
	static Gpdma * getChannel(int channel = -1);

	static void init();

	static inline void burstRequest(PeripheralId_t peripheral) { LPC_GPDMA->DMACSoftBReq = (1 << (unsigned long)peripheral); }
	static inline void singleRequest(PeripheralId_t peripheral) { LPC_GPDMA->DMACSoftSReq = (1 << (unsigned long)peripheral); }
	static inline void lastBurstRequest(PeripheralId_t peripheral) { LPC_GPDMA->DMACSoftLBReq = (1 << (unsigned long)peripheral); }
	static inline void lastSingleRequest(PeripheralId_t peripheral) { LPC_GPDMA->DMACSoftLSReq = (1 << (unsigned long)peripheral); }

	inline void reset();

	inline void setSrcAddress(void * addr) { chanReg()->DMACCSrcAddr = (unsigned int)addr; }
	inline void setDestAddress(void * addr) { chanReg()->DMACCDestAddr = (unsigned int)addr; }
	inline void setLLIAddress(LLI_t * lli) { chanReg()->DMACCLLI = (unsigned int)lli; }

	inline void * getSrcAddress() { return (void *)chanReg()->DMACCSrcAddr; }
	inline void * getDestAddress() { return (void *)chanReg()->DMACCDestAddr; }
	inline LLI_t * getLLIAddress() { return (LLI_t *)chanReg()->DMACCLLI; }

	static inline unsigned int createControlWord(int transferSize, 
		BurstSize_t srcBurstSize, 
		BurstSize_t destBurstSize,
		TransferWidth_t srcTransferWidth,
		TransferWidth_t destTransferWidth,
		bool srcIncrement = false,
		bool destIncrement = false,
		bool terminalCntIntEn = false);

	inline void setControl(unsigned int controlWord) { chanReg()->DMACCControl = controlWord; }
	inline unsigned int getControl() { return chanReg()->DMACCControl; }
	inline int getTransfersToGo() { return chanReg()->DMACCControl & 0xFFF; } // THIS IS ALWAYS DELAYED. IT DOESN'T GET UPDATED TILL THE PERIPHERAL ACKNOWLEDGES
	static inline int getTransfersToGoFromLLI(LLI_t * lli) { return lli->Control & 0xFFF; }

	inline void setConfig(bool channelEnabled,
		PeripheralId_t srcPeripheral,
		PeripheralId_t destPeripheral,
		TransferType_t transferType,
		bool interruptErrorMask = false,
		bool interruptTerminalCountMask = false,
		bool halt = false);

	inline bool getEnabled() { return (LPC_GPDMA->DMACEnbldChns & (1 << _devChannel)) ? true : false; }

	inline bool getActive() { return (chanReg()->DMACCConfig & (1UL << 17UL)) ? true : false; }
	inline void setHalt() { chanReg()->DMACCConfig |= (1UL << 18UL); }
	inline void clearHalt() { chanReg()->DMACCConfig &= ~(1UL << 18UL); }
	inline void clearEnable() { chanReg()->DMACCConfig &= ~((1UL << 0UL)); } // clear enable// and halt bits
	inline void clearEnableAndHalt() { chanReg()->DMACCConfig &= ~((1UL << 0UL) | (1UL << 18UL)); } // clear enable// and halt bits

	static inline void setSync(PeripheralId_t peripheral) { LPC_GPDMA->DMACSync |= (1 << (unsigned long)peripheral); }

	static inline void setPeripheralType(PeripheralId_t peripheral, bool isTimerMatchRatherThanUart = false);

	inline int getChannelNum() { return _devChannel; }

	static const int NumChannels = 8;
	static const size_t MaxTransferSize = 0xFFF;

protected:
	inline LPC_GPDMACH_TypeDef * chanReg() { return (LPC_GPDMACH_TypeDef *) (LPC_GPDMACH0_BASE + (0x20 * _devChannel)); }

	Gpdma(int channel)
		: _devChannel(channel)
	{ }
	~Gpdma() { }

	int _devChannel;
	static void dmaIsr();
	static Gpdma * _channels[NumChannels];
};


template <typename T>
class DmaM2P
{
public:
	DmaM2P(Gpdma::PeripheralId_t peripheral, T *periphDest, size_t fifoLen, Gpdma * dma = 0);
	~DmaM2P();

	int write(const T * buf, size_t len);

	inline size_t getBuffLen() { return _buffLen; }
	inline Gpdma::PeripheralId_t getPeripheral() { return _peripheral; }
	inline Gpdma * getDma() { return _dma; }
protected:
	Gpdma::PeripheralId_t _peripheral;
	T *_periphDest;
	size_t _buffLen;
	Gpdma *_dma;
	T *_buff, *_buffEnd;
	Gpdma::LLI_t _LLI;
};

template <typename T>
class DmaP2M
{
public:
	DmaP2M(Gpdma::PeripheralId_t peripheral, T *periphSrc, size_t fifoLen, Gpdma * dma = 0);
	~DmaP2M();

	size_t read(T * buf, size_t len);
	size_t numWaiting();
	//size_t numInBuff();
	void startReading();

	inline size_t getBuffLen() { return _buffLen; }
	inline Gpdma::PeripheralId_t getPeripheral() { return _peripheral; }
	inline Gpdma * getDma() { return _dma; }
protected:
	Gpdma::PeripheralId_t _peripheral;
	T *_periphSrc;
	size_t _buffLen;
	Gpdma *_dma;
	T *_buff, *_buffStart;
	Gpdma::LLI_t _LLI;
};

/**** Gpdma Implementation ****/
inline void Gpdma::reset()
{
	chanReg()->DMACCConfig = 0;
	setSrcAddress(0);
	setDestAddress(0);
	setLLIAddress(0);
	setControl(0);
}

inline unsigned int Gpdma::createControlWord(int transferSize, 
	BurstSize_t srcBurstSize, 
	BurstSize_t destBurstSize,
	TransferWidth_t srcTransferWidth,
	TransferWidth_t destTransferWidth,
	bool srcIncrement,
	bool destIncrement,
	bool terminalCntIntEn)
{
	return (transferSize & 0xFFF) | ((srcBurstSize & 7) << 12) | ((destBurstSize & 7) << 15) | ((srcTransferWidth & 7) << 18) | ((destTransferWidth & 7) << 21) |
		((srcIncrement ? 1 : 0) << 26) | ((destIncrement ? 1 : 0) << 27) | ((terminalCntIntEn ? 1 : 0) << 31);
}

inline void Gpdma::setConfig(bool channelEnabled,
	PeripheralId_t srcPeripheral,
	PeripheralId_t destPeripheral,
	TransferType_t transferType,
	bool interruptErrorMask,
	bool interruptTerminalCountMask,
	bool halt)
{
	chanReg()->DMACCConfig = (channelEnabled ? 1 : 0) | (srcPeripheral << 1) | (destPeripheral << 6) | ((transferType & 7) << 11) |
		((interruptErrorMask ? 1 : 0) << 14) | ((interruptTerminalCountMask ? 1 : 0) << 15) | ((halt ? 1 : 0) << 18);
}

inline void Gpdma::setPeripheralType(PeripheralId_t peripheral, bool isTimerMatchRatherThanUart)
{
	if (peripheral >= UART0Tx_Mat00 && peripheral <= UART3Rx_Mat31)
	{
		if (isTimerMatchRatherThanUart)
			LPC_SC->DMAREQSEL |= (1 << (peripheral - UART0Tx_Mat00));
		else
			LPC_SC->DMAREQSEL &= ~(1 << (peripheral - UART0Tx_Mat00));
	}	
}

/**** DmaM2P Implementation ****/
template<typename T> DmaM2P<T>::DmaM2P(Gpdma::PeripheralId_t peripheral, T *periphDest, size_t fifoLen, Gpdma * dma)
	: _peripheral(peripheral), _periphDest(periphDest), _buffLen(fifoLen), _dma(dma)
{
	if (!_dma)
		_dma = Gpdma::getChannel();
	if (_buffLen > Gpdma::MaxTransferSize)
		_buffLen = Gpdma::MaxTransferSize;
	else if (_buffLen < 1)
		_buffLen = 1;
	size_t tmp = _buffLen % 4;
	if (tmp)
		_buffLen += 4 - tmp;
	_buff = (T *)malloc(_buffLen * sizeof(T));
	_dma->reset();
	_LLI.SrcAddr = (unsigned long)_buff;
	_LLI.DestAddr = (unsigned long)_periphDest;
	_LLI.NextLLI = 0;
}

template<typename T> DmaM2P<T>::~DmaM2P()
{
	if (_buff)
		free(_buff);
}

template<typename T> int DmaM2P<T>::write(const T * buf, size_t len)
{
	bool wasActive = _dma->getEnabled() || _dma->getActive();
	T *src = _buff;
	bool usingLli = false;
	size_t numToGo = 0, numInLli = 0;
	Gpdma::TransferWidth_t txWidth = (sizeof(T) == 1) ? Gpdma::Byte : ((sizeof(T) == 2) ? Gpdma::HalfWord : Gpdma::Word);
	unsigned int controlWord = Gpdma::createControlWord(0, Gpdma::_1, Gpdma::_1, txWidth, txWidth, true);
	
	if (wasActive)
	{
		_dma->clearEnable();
		while (_dma->getActive())
			;

		numToGo = _dma->getTransfersToGo();
		if (numToGo < 1)
			wasActive = false;
		else
		{
			usingLli = _dma->getLLIAddress() != 0;
			if (usingLli)
			{
				numInLli = Gpdma::getTransfersToGoFromLLI(&_LLI);
				src = (_buff + _buffLen) - numToGo;
			}
			else
				src = _buffEnd - numToGo;
		}
	}

	size_t buffSpace = _buffLen - (numToGo + numInLli);
	if (buffSpace < len)
		len = buffSpace;

	if (wasActive)
	{
		if (usingLli)
		{
			memcpy(_buffEnd, buf, len);
			_LLI.Control = controlWord + numInLli + len; // can do this as transfer size is in bottom 3 nibbles
			_buffEnd += len;
			_dma->setLLIAddress(&_LLI);
			_dma->setControl(controlWord + numToGo);
		}
		else
		{
			size_t spaceAtTop = (_buff + _buffLen) - _buffEnd;
			if (len > spaceAtTop)
			{
				// Setup an LLI
				memcpy(_buffEnd, buf, spaceAtTop);
				_dma->setControl(controlWord + numToGo + spaceAtTop);
				numInLli = len - spaceAtTop;
				memcpy(_buff, buf + spaceAtTop, numInLli);
				_LLI.Control = controlWord + numInLli;
				_LLI.SrcAddr = (uint32_t)_buff;
				_LLI.NextLLI = 0;
				_buffEnd = _buff + numInLli;
				_dma->setLLIAddress(&_LLI);
			}
			else // Don't need LLI, add to end of existing transfer
			{
				memcpy(_buffEnd, buf, len);
				_buffEnd += len;
				_dma->setControl(controlWord + numToGo + len);
				_dma->setLLIAddress(0);
			}
		}
		_dma->setSrcAddress(src);
	}
	else // wasn't active
	{
		_buffEnd = _buff + len;
		memcpy(_buff, buf, len);
		_dma->setSrcAddress(_buff);
		_dma->setControl(controlWord + len);
		_dma->setLLIAddress(0);
	}
	_dma->setDestAddress(_periphDest);
	_dma->setConfig(true, (Gpdma::PeripheralId_t)0, _peripheral, Gpdma::MemToPeriph);
	return len;
}

/**** DmaP2M Implementation ****/
template<typename T> DmaP2M<T>::DmaP2M(Gpdma::PeripheralId_t peripheral, T *periphSrc, size_t fifoLen, Gpdma * dma)
	: _peripheral(peripheral), _periphSrc(periphSrc), _buffLen(fifoLen), _dma(dma)
{
	if (!_dma)
		_dma = Gpdma::getChannel();
	if (_buffLen > Gpdma::MaxTransferSize)
		_buffLen = Gpdma::MaxTransferSize;
	else if (_buffLen < 1)
		_buffLen = 1;
	size_t tmp = _buffLen % 4;
	if (tmp)
		_buffLen += 4 - tmp;
	_buff = (T *)malloc(_buffLen * sizeof(T));
	_dma->reset();
	_LLI.SrcAddr = (unsigned long)_periphSrc;
	_LLI.DestAddr = (unsigned long)_buff;
	_LLI.NextLLI = 0;
}

template<typename T> DmaP2M<T>::~DmaP2M()
{
	if (_buff)
		free(_buff);
}

template<typename T> void DmaP2M<T>::startReading()
{
	_buffStart = _buff;
	_dma->setSrcAddress(_periphSrc);
	_dma->setDestAddress(_buff);
	_dma->setLLIAddress(0);
	Gpdma::TransferWidth_t txWidth = (sizeof(T) == 1) ? Gpdma::Byte : ((sizeof(T) == 2) ? Gpdma::HalfWord : Gpdma::Word);
	_dma->setControl(Gpdma::createControlWord(_buffLen, Gpdma::_1, Gpdma::_1, txWidth, txWidth, false, true));
	_dma->setConfig(true, _peripheral, (Gpdma::PeripheralId_t)0, Gpdma::PeriphToMem);
}

template<typename T> size_t DmaP2M<T>::numWaiting()
{
	if (_dma->getEnabled())
	{
		_dma->setHalt();
		while (_dma->getActive())
			;
		_dma->clearEnable();
		while (_dma->getEnabled())
			;
		_dma->clearHalt();
	}
	else
		return _buffLen;

	size_t numToGo = _dma->getTransfersToGo();
	bool usingLli = _dma->getLLIAddress() != 0;
	size_t numInBuff = _buffLen - numToGo - (usingLli ? Gpdma::getTransfersToGoFromLLI(&_LLI) : 0);

	_dma->setLLIAddress(usingLli ? &_LLI : 0);
	_dma->setSrcAddress(_periphSrc);
	Gpdma::TransferWidth_t txWidth = (sizeof(T) == 1) ? Gpdma::Byte : ((sizeof(T) == 2) ? Gpdma::HalfWord : Gpdma::Word);
	_dma->setControl(Gpdma::createControlWord(numToGo, Gpdma::_1, Gpdma::_1, txWidth, txWidth, false, true));
	_dma->setDestAddress((usingLli || (_buff == _buffStart)) ? ((_buff + _buffLen) - numToGo) : (_buffStart - numToGo));
	_dma->setConfig(true, _peripheral, (Gpdma::PeripheralId_t)0, Gpdma::PeriphToMem);
	return numInBuff;
}

template<typename T> size_t DmaP2M<T>::read(T * buf, size_t len)
{
	Gpdma::TransferWidth_t txWidth = (sizeof(T) == 1) ? Gpdma::Byte : ((sizeof(T) == 2) ? Gpdma::HalfWord : Gpdma::Word);

	if (_dma->getEnabled())
	{
		_dma->setHalt();
		while (_dma->getActive())
			;
		_dma->clearEnable();
		while (_dma->getEnabled())
			;
		_dma->clearHalt();
	}

	size_t numToGo = _dma->getTransfersToGo(), numToGoInLli = 0;
	bool usingLli = _dma->getLLIAddress() != 0;
	if (usingLli)
		numToGoInLli = Gpdma::getTransfersToGoFromLLI(&_LLI);
	_dma->setDestAddress((usingLli || (_buff == _buffStart)) ? ((_buff + _buffLen) - numToGo) : (_buffStart - numToGo));
	size_t numInBuff = _buffLen - numToGo - numToGoInLli;
	if (len > numInBuff)
		len = numInBuff;
	if (usingLli)
	{
		memcpy(buf, _buffStart, len);
		_buffStart += len;
		_LLI.Control = Gpdma::createControlWord(numToGoInLli + len, Gpdma::_1, Gpdma::_1, txWidth, txWidth, false, true);
		_dma->setLLIAddress(&_LLI);
		// numToGo = numToGo
	}
	else
	{
		size_t numAtTop = (_buff == _buffStart) ? 0 : ((_buff + _buffLen) - _buffStart);
		if (len > numAtTop)
		{
			// Setup LLI
			memcpy(buf, _buffStart, numAtTop);
			size_t numAtBottom = len - numAtTop;
			memcpy(buf + numAtTop, _buff, numAtBottom);
			_buffStart = _buff + numAtBottom;
			numToGo += numAtTop;
			_LLI.Control = Gpdma::createControlWord(numAtBottom, Gpdma::_1, Gpdma::_1, txWidth, txWidth, false, true);
			_dma->setLLIAddress(&_LLI);
		}
		else if (len < numAtTop)
		{
			memcpy(buf, _buffStart, len);
			_buffStart += len;
			_dma->setLLIAddress(0);
			numToGo += len;
		}
		else // if (len == numAtTop)
		{
			memcpy(buf, _buffStart, len);
			numToGo += len;
			_buffStart = _buff;
			_dma->setLLIAddress(0);
		}
	}
	_dma->setSrcAddress(_periphSrc);
	_dma->setControl(Gpdma::createControlWord(numToGo, Gpdma::_1, Gpdma::_1, txWidth, txWidth, false, true));
	_dma->setConfig(true, _peripheral, (Gpdma::PeripheralId_t)0, Gpdma::PeriphToMem);
	
	return len;
}

#endif // _H_GPDMA_


#ifndef	GPDMA_h
#define GPDMA_h

#include <stdint.h>
#include <cmsis.h>
#include <stdlib.h>

#ifdef __cplusplus

class GPDMA
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
		 * The lowest channel number has the highest priority
		 * Either get a particular channel or get the next free one.
		 * It is the responsibility of the requester to check if someone else is using the channel (check if active)
		 */
		static GPDMA * getChannel(int channel = -1);

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

		GPDMA(int channel)
			: _devChannel(channel)
		{ }
		~GPDMA() { }

		int _devChannel;
		static void dmaIsr();
		static GPDMA * _channels[NumChannels];
};

template <typename T>
	class DmaM2P
	{
		public:
			DmaM2P(GPDMA::PeripheralId_t peripheral, T *periphDest, size_t fifoLen, GPDMA * dma = 0);
			~DmaM2P();

			int write(const T * buf, size_t len);

			inline size_t getBuffLen() { return _buffLen; }
			inline GPDMA::PeripheralId_t getPeripheral() { return _peripheral; }
			inline GPDMA * getDma() { return _dma; }
		protected:
			GPDMA::PeripheralId_t _peripheral;
			T *_periphDest;
			size_t _buffLen;
			GPDMA *_dma;
			T *_buff, *_buffEnd;
			GPDMA::LLI_t _LLI;
	};

template <typename T>
	class DmaP2M
	{
		public:
			DmaP2M(GPDMA::PeripheralId_t peripheral, T *periphSrc, size_t fifoLen, GPDMA * dma = 0);
			~DmaP2M();

			size_t read(T * buf, size_t len);
			size_t numWaiting();
			//size_t numInBuff();
			void startReading();

			inline size_t getBuffLen() { return _buffLen; }
			inline GPDMA::PeripheralId_t getPeripheral() { return _peripheral; }
			inline GPDMA * getDma() { return _dma; }
		protected:
			GPDMA::PeripheralId_t _peripheral;
			T *_periphSrc;
			size_t _buffLen;
			GPDMA *_dma;
			T *_buff, *_buffStart;
			GPDMA::LLI_t _LLI;
	};

/**** GPDMA Implementation ****/
inline void GPDMA::reset()
{
	chanReg()->DMACCConfig = 0;
	setSrcAddress(0);
	setDestAddress(0);
	setLLIAddress(0);
	setControl(0);
}

inline unsigned int GPDMA::createControlWord(int transferSize,
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

inline void GPDMA::setConfig(bool channelEnabled,
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

inline void GPDMA::setPeripheralType(PeripheralId_t peripheral, bool isTimerMatchRatherThanUart)
{
#ifdef TARGET_LPC17xx
	if (peripheral >= UART0Tx_Mat00 && peripheral <= UART3Rx_Mat31)
	{
		if (isTimerMatchRatherThanUart)
			LPC_SC->DMAREQSEL |= (1 << (peripheral - UART0Tx_Mat00));
		else
			LPC_SC->DMAREQSEL &= ~(1 << (peripheral - UART0Tx_Mat00));
	}
#endif
}

/**** DmaM2P Implementation ****/
template<typename T>
	DmaM2P<T>::DmaM2P(GPDMA::PeripheralId_t peripheral, T *periphDest, size_t fifoLen, GPDMA * dma)
		: _peripheral(peripheral), _periphDest(periphDest), _buffLen(fifoLen), _dma(dma)
	{
		if (!_dma)
			_dma = GPDMA::getChannel();
		if (_buffLen > GPDMA::MaxTransferSize)
			_buffLen = GPDMA::MaxTransferSize;
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

template<typename T>
	DmaM2P<T>::~DmaM2P()
	{
		if (_buff)
			free(_buff);
	}

template<typename T>
	int DmaM2P<T>::write(const T * buf, size_t len)
	{
		bool wasActive = _dma->getEnabled() || _dma->getActive();
		T *src = _buff;
		bool usingLli = false;
		size_t numToGo = 0, numInLli = 0;
		GPDMA::TransferWidth_t txWidth = (sizeof(T) == 1) ? GPDMA::Byte : ((sizeof(T) == 2) ? GPDMA::HalfWord : GPDMA::Word);
		unsigned int controlWord = GPDMA::createControlWord(0, GPDMA::_1, GPDMA::_1, txWidth, txWidth, true);

		if (wasActive)
		{
			_dma->clearEnable();
			while (_dma->getActive())
				;

			numToGo = _dma->getTransfersToGo();
			usingLli = _dma->getLLIAddress() != 0;
			if (numToGo < 1 && !usingLli)
				wasActive = false;
			else
			{
				if (usingLli)
				{
					numInLli = GPDMA::getTransfersToGoFromLLI(&_LLI);
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
				_buffEnd += len;
				if (numToGo)
				{
					_LLI.Control = controlWord + numInLli + len; // can do this as transfer size is in bottom 3 nibbles
					_dma->setLLIAddress(&_LLI);
					_dma->setControl(controlWord + numToGo);
				}
				else
				{
					_dma->setLLIAddress(0);
					_dma->setControl(controlWord + numInLli + len);
					src = _buff;
				}
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
		_dma->setConfig(true, (GPDMA::PeripheralId_t)0, _peripheral, GPDMA::MemToPeriph);
		return len;
	}

/**** DmaP2M Implementation ****/
template<typename T>
	DmaP2M<T>::DmaP2M(GPDMA::PeripheralId_t peripheral, T *periphSrc, size_t fifoLen, GPDMA * dma)
		: _peripheral(peripheral), _periphSrc(periphSrc), _buffLen(fifoLen), _dma(dma)
	{
		if (!_dma)
			_dma = GPDMA::getChannel();
		if (_buffLen > GPDMA::MaxTransferSize)
			_buffLen = GPDMA::MaxTransferSize;
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

template<typename T>
	DmaP2M<T>::~DmaP2M()
	{
		if (_buff)
			free(_buff);
	}

template<typename T> void
	DmaP2M<T>::startReading()
	{
		_buffStart = _buff;
		_dma->setSrcAddress(_periphSrc);
		_dma->setDestAddress(_buff);
		_dma->setLLIAddress(0);
		GPDMA::TransferWidth_t txWidth = (sizeof(T) == 1) ? GPDMA::Byte : ((sizeof(T) == 2) ? GPDMA::HalfWord : GPDMA::Word);
		_dma->setControl(GPDMA::createControlWord(_buffLen, GPDMA::_1, GPDMA::_1, txWidth, txWidth, false, true));
		_dma->setConfig(true, _peripheral, (GPDMA::PeripheralId_t)0, GPDMA::PeriphToMem);
	}

template<typename T>
	size_t DmaP2M<T>::numWaiting()
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
		size_t numInBuff = _buffLen - numToGo - (usingLli ? GPDMA::getTransfersToGoFromLLI(&_LLI) : 0);

		_dma->setLLIAddress(usingLli ? &_LLI : 0);
		_dma->setSrcAddress(_periphSrc);
		GPDMA::TransferWidth_t txWidth = (sizeof(T) == 1) ? GPDMA::Byte : ((sizeof(T) == 2) ? GPDMA::HalfWord : GPDMA::Word);
		_dma->setControl(GPDMA::createControlWord(numToGo, GPDMA::_1, GPDMA::_1, txWidth, txWidth, false, true));
		_dma->setDestAddress((usingLli || (_buff == _buffStart)) ? ((_buff + _buffLen) - numToGo) : (_buffStart - numToGo));
		_dma->setConfig(true, _peripheral, (GPDMA::PeripheralId_t)0, GPDMA::PeriphToMem);
		return numInBuff;
	}

template<typename T>
	size_t DmaP2M<T>::read(T * buf, size_t len)
	{
		GPDMA::TransferWidth_t txWidth = (sizeof(T) == 1) ? GPDMA::Byte : ((sizeof(T) == 2) ? GPDMA::HalfWord : GPDMA::Word);

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
			numToGoInLli = GPDMA::getTransfersToGoFromLLI(&_LLI);
		_dma->setDestAddress((usingLli || (_buff == _buffStart)) ? ((_buff + _buffLen) - numToGo) : (_buffStart - numToGo));
		size_t numInBuff = _buffLen - numToGo - numToGoInLli;
		if (len > numInBuff)
			len = numInBuff;
		if (usingLli)
		{
			memcpy(buf, _buffStart, len);
			_buffStart += len;
			_LLI.Control = GPDMA::createControlWord(numToGoInLli + len, GPDMA::_1, GPDMA::_1, txWidth, txWidth, false, true);
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
				_LLI.Control = GPDMA::createControlWord(numAtBottom, GPDMA::_1, GPDMA::_1, txWidth, txWidth, false, true);
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
		_dma->setControl(GPDMA::createControlWord(numToGo, GPDMA::_1, GPDMA::_1, txWidth, txWidth, false, true));
		_dma->setConfig(true, _peripheral, (GPDMA::PeripheralId_t)0, GPDMA::PeriphToMem);

		return len;
	}

#endif // ifdef __cplusplus

#endif // ifndef GPDMA_h


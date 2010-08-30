#include <drivers/gpdma.h>
#include <cmsis_nvic.h>

typedef struct
{
	uint32_t offset[8];
} devChanOffset_t;

GPDMA * GPDMA::_channels[];

void GPDMA::init()
{
	LPC_SC->PCONP |= (1UL << 29UL); // DMA power
	NVIC_SetVector(DMA_IRQn, (unsigned long)&dmaIsr);
	NVIC_EnableIRQ(DMA_IRQn);
	LPC_GPDMA->DMACConfig = 1; // Enable DMA controller
	while (!(LPC_GPDMA->DMACConfig & 1))
		;
}

GPDMA * GPDMA::getChannel(int channel)
{
	if (channel >= NumChannels || channel < -1)
		return 0;

	GPDMA *c;
	if (-1 == channel)
	{
		// Find a free one
		for (channel = 0; (c = _channels[channel]); ++channel)
			;
	}
	else
		c = _channels[channel];

	if (0 == c)
	{
		c = new GPDMA(channel);
		_channels[channel] = c;
	}

	// Clear pending ints
	LPC_GPDMA->DMACIntTCClear |= (1 << channel);
	LPC_GPDMA->DMACIntErrClr |= (1 << channel);

	c->setLLIAddress(0);

	return c;
}

void GPDMA::dmaIsr()
{
	//uint8_t channelActive = LPC_GPDMA->DMACIntStat & 0xFF;
	uint8_t channelTC = LPC_GPDMA->DMACIntTCStat & 0xFF;
	uint8_t channelErr = LPC_GPDMA->DMACIntErrStat & 0xFF;
	for (int i = 0; i < NumChannels; ++i)
	{
		if (channelTC & 1)
		{
			if (_channels[i] == 0)
			{
				// TODO disable interrupts, channel's not used!
			}
			else
			{
				// TODO Service int
			}

			LPC_GPDMA->DMACIntTCClear |= (1 << i);
		}
		if (channelErr & 1)
		{
			if (_channels[i] == 0)
			{
				// TODO disable interrupts, channel's not used!
			}
			else
			{
				// TODO Service int
			}

			LPC_GPDMA->DMACIntErrClr |= (1 << i);
		}
		channelTC >>= 1;
		channelErr >>= 1;
		if (!channelTC && !channelErr)
			break;
	}
}

/**** DmaM2M Implementation ****/
DmaM2M::DmaM2M(GPDMA * dma)
	: _dma(dma)
{
	if (!_dma)
		_dma = GPDMA::getChannel();
	_dma->reset();
}

DmaM2M::~DmaM2M()
{
}

void DmaM2M::transferForwards(void *dest, const void *src, size_t num, GPDMA::TransferWidth_t transferWidth)
{
	_dma->setDestAddress(dest);
	_dma->setSrcAddress((void *)src);
	_dma->setLLIAddress(0);
	_dma->setControl(GPDMA::createControlWord(num, GPDMA::_1, GPDMA::_1, transferWidth, transferWidth, true, true));
	_dma->setConfig(true, (GPDMA::PeripheralId_t)0, (GPDMA::PeripheralId_t)0, GPDMA::MemToMem);
}


/* ------------------------------------------------------------------------- */
// C Wrappers:

DmaM2M *DmaM2M_Init(GPDMA *dma)
{
	return new DmaM2M(dma);
}

void DmaM2M_Transfer(DmaM2M *dma, void *dest, const void *src, size_t num, GPDMA_TransferWidth_t transferWidth)
{ 
	dma->transfer(dest, src, num, (GPDMA::TransferWidth_t)transferWidth); 
}

bool DmaM2M_TransferInProgress(DmaM2M *dma)
{
	return dma->transferInProgress();
}


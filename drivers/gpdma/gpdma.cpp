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


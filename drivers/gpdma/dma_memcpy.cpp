
#include "drivers/dma_memcpy.h"

// Use lowest priority dma channel
static DmaM2M dmaChan(GPDMA::getChannel(GPDMA::NumChannels - 1));


void dma_memcpy_byte(char *dst, const char *src, size_t numBYTES)
{
	dmaChan.transferForwards(dst, src, numBYTES, GPDMA::Byte);
	while (dmaChan.transferInProgress())
		;
}

void dma_memcpy_word(int *dst, const int *src, size_t numBYTES)
{
	dmaChan.transferForwards(dst, src, ((unsigned int)numBYTES) >> 2, GPDMA::Word);
	while (dmaChan.transferInProgress())
		;
}

void dma_memmove_word(int *dst, const int *src, size_t numBYTES)
{
	unsigned int numWords = (unsigned int)numBYTES >> 2;
	if (src < dst && dst < src + numWords)
	{
		// Have to copy backwards and DMA doesn't have decrementing. Use newlib impl
		src += numWords;
		dst += numWords;
		while (numWords--)
		{
			*--dst = *--src;
		}
	}
	else
	{
		dmaChan.transferForwards(dst, src, numWords, GPDMA::Word);
		while (dmaChan.transferInProgress())
			;
	}
}

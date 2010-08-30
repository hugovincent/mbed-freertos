
/* memcpy routines using a dma channel. For transfers of more than a 
 *	5-or-so words then it's much faster than regular memcpy.
 *
 * WARNING: These are NOT thread safe. A single dma channel instance is used
 */

#ifndef DMA_MEMCPY_h
#define DMA_MEMCPY_h

#include "gpdma.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Note that the size is in number of BYTES in every case to provide compatibility with memcpy */
void dma_memcpy_byte(char *dst, const char *src, size_t numBYTES);
void dma_memcpy_word(int *dst, const int *src, size_t numBYTES);

void dma_memmove_word(int *dst, const int *src, size_t numBYTES);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // DMA_MEMCPY_h

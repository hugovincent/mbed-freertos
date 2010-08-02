/* Based on scripts found in the LPC yahoo-group and the Keil examples 
 * modified by Martin Thomas, Kaiserslautern, Germany
 * modified by Hugo Vincent <http://hugovincent.com>
 */

/* Memory Definitions */
MEMORY
{
	Flash (rx)  : ORIGIN = 0x00000000, LENGTH = 512k
	Ram (rwx)   : ORIGIN = 0x10000000, LENGTH = 32k
	UsbRam (rw) : ORIGIN = 0x2007C000, LENGTH = 16k
	EthRam (rw) : ORIGIN = 0x20080000, LENGTH = 16k
	CanRam (rw) : ORIGIN = 0x40038000, LENGTH = 2k
}

__FLASH_segment_start__ = ORIGIN( Flash );
__FLASH_segment_end__ = __FLASH_segment_start__ + LENGTH( Flash );
__SRAM_segment_start__ = ORIGIN( Ram );
__SRAM_segment_end__ = __SRAM_segment_start__ + LENGTH( Ram );

Stack_Size_Total = 768;

INCLUDE "mach/cpu-common/common.ld"

SECTIONS
{
	/* C data can be defined as being in special purpose rams using
	 * __attribute__ ((section ("ethram"))) for example. */
	.usbram (NOLOAD):
	{
		*( .usbram )
		*( .usbram.* )
	} > UsbRam
	.ethram (NOLOAD):
	{
		*( .ethram )
		*( .ethram.* )
	} > EthRam
	.canram (NOLOAD):
	{
		*( .canram )
		*( .canram.* )
	} > CanRam
}


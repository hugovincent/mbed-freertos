/* Based on scripts found in the LPC yahoo-group and the Keil examples 
 * modified by Martin Thomas, Kaiserslautern, Germany
 * modified by Hugo Vincent <http://hugovincent.com>
 */

/* Memory Definitions */
MEMORY
{
	Flash (rx)  : ORIGIN = 0x00000000, LENGTH = 512k
	Ram (rw)    : ORIGIN = 0x40000000, LENGTH = 32k
	UsbRam (rw) : ORIGIN = 0x7FD00000, LENGTH = 8k
	EthRam (rw) : ORIGIN = 0x7FE00000, LENGTH = 16k
	BatRam (rw) : ORIGIN = 0xE0084000, LENGTH = 2k
}

INCLUDE "hardware/cpu-common/common.ld"

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
	.batram (NOLOAD):
	{
		*( .batram )
		*( .batram.* )
	} > BatRam
}


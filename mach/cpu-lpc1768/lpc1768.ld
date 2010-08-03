/* Memory Definitions */
MEMORY
{
	Flash  (rx)  : ORIGIN = 0x00000000, LENGTH = 512k
	Ram    (rwx) : ORIGIN = 0x10000000, LENGTH = 32k
	UsbRam (rw)  : ORIGIN = 0x2007C000, LENGTH = 16k
	EthRam (rw)  : ORIGIN = 0x20080000, LENGTH = 16k
	CanRam (rw)  : ORIGIN = 0x40038000, LENGTH = 2k
}

Stack_Size_Total = 768;

INCLUDE "mach/arm_common.ld"

SECTIONS
{
	/* C data can be defined as being in special purpose RAMs using
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


/* Memory Definitions */
MEMORY
{
	Flash  (rx)  : ORIGIN = 0x00000000, LENGTH = 512k
	Ram    (rwx) : ORIGIN = 0x40000000, LENGTH = 32k
	UsbRam (rw)  : ORIGIN = 0x7FD00000, LENGTH = 8k
	EthRam (rw)  : ORIGIN = 0x7FE00000, LENGTH = 16k
	CanRam (rw)  : ORIGIN = 0xE0038000, LENGTH = 2k
	BatRam (rw)  : ORIGIN = 0xE0084000, LENGTH = 2k
}

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
	.batram (NOLOAD):
	{
		*( .batram )
		*( .batram.* )
	} > BatRam
}


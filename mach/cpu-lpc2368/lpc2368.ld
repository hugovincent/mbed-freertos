/* Memory Definitions: */
MEMORY
{
	Flash  (rx)  : ORIGIN = 0x00000000, LENGTH = 512k
	Ram    (rwx) : ORIGIN = 0x40000000, LENGTH = 32k
	UsbRam (rw)  : ORIGIN = 0x7FD00000, LENGTH = 8k
	EthRam (rw)  : ORIGIN = 0x7FE00000, LENGTH = 16k
	CanRam (rw)  : ORIGIN = 0xE0038000, LENGTH = 2k
	BatRam (rw)  : ORIGIN = 0xE0084000, LENGTH = 2k
}

/* Stack sizes: */
UND_Stack_Size = 16;
SVC_Stack_Size = 512;
ABT_Stack_Size = 16;
FIQ_Stack_Size = 16;
IRQ_Stack_Size = 256;
Stack_Size_Total = UND_Stack_Size + SVC_Stack_Size + ABT_Stack_Size + FIQ_Stack_Size + IRQ_Stack_Size;

INCLUDE "mach/arm_common.ld"

/* Stack tops for each mode: */
__und_stack_top__ = __stacks_top__;
__abt_stack_top__ = __und_stack_top__ - UND_Stack_Size ; 
__fiq_stack_top__ = __abt_stack_top__ - ABT_Stack_Size ;
__irq_stack_top__ = __fiq_stack_top__ - FIQ_Stack_Size ;
__svc_stack_top__ = __irq_stack_top__ - IRQ_Stack_Size ;

/* Special-purpose RAM sections: */
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


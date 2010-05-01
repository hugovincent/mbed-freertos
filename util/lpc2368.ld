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

/* Section Definitions */
SECTIONS
{
	/* ------------------------- START OF ROM CODE ------------------------- */

	/* first section is .text which is used for code */
	.text : AT ( 0x0 )
	{
		*util/crt0.o ( .vectors )			/* Exception vectors from crt0.s */
		*util/crt0.o ( .text )				/* remaining boot code */
		*( .text )
		*( .text.* )
		*( .gnu.linkonce.t.* )
		*( .glue_7 )
		*( .glue_7t )
		*( .gcc_except_table )
		*( .rodata )						/* read-only data (constants) */
		*( .rodata* )
		*( .gnu.linkonce.r.* )
	} > Flash


	/* .ctors .dtors and are used for c++ constructors/destructors */
	.ctors :
	{
		__ctors_start__ = . ;
		KEEP(*(SORT( .ctors.* )))
		KEEP(*( .ctors ))
		__ctors_end__ = . ;
	} > Flash
	.dtors :
	{
		__dtors_start__ = . ; 
		KEEP(*(SORT( .dtors.* )))
		KEEP(*( .dtors ))
		__dtors_end__ = . ;
	} > Flash

	/* .ARM.exidx and .ARM.extab are used for exception handling */
	.ARM.extab : 
	{
		*( .ARM.extab* )
		*( .gnu.linkonce.armextab.* )
	} > Flash
	__exidx_start = . ;
	.ARM.exidx :
	{
		*( .ARM.exidx* )
		*( .gnu.linkonce.armexidx.* )
	} > Flash
	__exidx_end = . ;

	. = ALIGN(8);
	__text_end__ = . ;

	/* .data section -- used for initialized data */
	.data : AT ( __text_end__ ) 
	{
		__data_start__ = . ;
		SORT(CONSTRUCTORS)
		*( .data )
		*( .data.* )
		*( .gnu.linkonce.d* )
		__data_end__ = . ;
	} > Ram

	/* -------------------------- END OF ROM CODE -------------------------- */

	/* .bss section -- used for uninitialized data */
	.bss (NOLOAD) :
	{
		__bss_start__ = . ;
		*( .bss )
		*( .bss.* )
		*( .gnu.linkonce.b* )
		*( COMMON )
		. = ALIGN(8);
		__bss_end__ = . ;
	} > Ram

	/* Stack and Heap setup */
	__top_of_stack__ = ALIGN(ORIGIN(Ram) + LENGTH(Ram), 8);
	__stack_min__ = ALIGN(__top_of_stack__ - Stack_Size_Total, 8);
	. = ALIGN(8);
	__start_of_heap__ = . ;

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

	/* -------------------------- DEBUG STUFF (not in binary) -------------- */
	/* Stabs debugging sections.  */
	.stab          0 : { *(.stab) }
	.stabstr       0 : { *(.stabstr) }
	.stab.excl     0 : { *(.stab.excl) }
	.stab.exclstr  0 : { *(.stab.exclstr) }
	.stab.index    0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment       0 : { *(.comment) }
	/* DWARF debug sections.
	   Symbols in the DWARF debugging sections are relative to the beginning
	   of the section so we begin them at 0.  */
	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
}

ENTRY( Vectors )

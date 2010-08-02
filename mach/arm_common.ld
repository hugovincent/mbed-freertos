/* Based on scripts found in the LPC yahoo-group and the Keil examples 
 * modified by Martin Thomas, Kaiserslautern, Germany
 * modified by Hugo Vincent <http://hugovincent.com>
 */

/* Section Definitions */
SECTIONS
{
	/* ------------------------- START OF ROM CODE ------------------------- */

	/* first section is .text which is used for code */
	__text_start__ = . ;
	.text : AT ( 0x0 )
	{
		__privileged_functions_start__ = .;

		__vectors_start__ = . ;
		KEEP(*( .vectors ))
		__vectors_end__ = .;

		*(privileged_functions .privileged_functions)

		/* Align to nearest power of two */
		prvFLen_ = . - __privileged_functions_start__;
		prvFLen2_ = prvFLen_ - 1;
		prvFLen2_ |= prvFLen2_ >> 1;
		prvFLen2_ |= prvFLen2_ >> 2;
		prvFLen2_ |= prvFLen2_ >> 4;
		prvFLen2_ |= prvFLen2_ >> 8;
		prvFLen2_ |= prvFLen2_ >> 16;
		. = ALIGN(MAX(prvFLen2_ + 1, 32));			/* Min MPU size is 32 bytes */

		__privileged_functions_end__ = .;

		*( .text .text.* )
		*( .gnu.linkonce.t.* )
		*( .glue_7 .glue_7t )
		*( .gcc_except_table )
		*( .rodata .rodata* )
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

	__text_end__ = . ;

	/* -------------------------- END OF ROM CODE -------------------------- */

	/* .bss section -- used for uninitialized data */
	.bss (NOLOAD) :
	{
		. = ALIGN(8);
		__bss_start__ = . ;

		__privileged_data_start__ = .;

		KEEP(*crt0.o( privileged_bss .privileged_bss ))
		KEEP(*( privileged_bss .privileged_bss ))

		prvDLen_ = . - __privileged_data_start__;
		prvDLen2_ = prvDLen_ - 1;
		prvDLen2_ |= prvDLen2_ >> 1;
		prvDLen2_ |= prvDLen2_ >> 2;
		prvDLen2_ |= prvDLen2_ >> 4;
		prvDLen2_ |= prvDLen2_ >> 8;
		prvDLen2_ |= prvDLen2_ >> 16;
		. = ALIGN(MAX(prvDLen2_ + 1, 32));

		__privileged_data_end__ = .;

		*(.shbss)
		*(.bss .bss.* .gnu.linkonce.b.*)
		*( .gnu.linkonce.b* )
		*( COMMON )
		. = ALIGN(8);
		*(.ram.b .bss.ram)
		__bss_end__ = . ;
	} > Ram

	/* .data section -- used for initialized data */
	.data : AT ( __text_end__ ) 
	{
		. = ALIGN(8);
		__data_start__ = . ;
		SORT(CONSTRUCTORS)
		*( .data .data.* )
		*( .gnu.linkonce.d* )
		. = ALIGN(8);
		__data_end__ = . ;
	} > Ram

	/* Heap starts here and grows up in memory */
	. = ALIGN(8);
	__start_of_heap__ = . ;

	/* Stack starts at top of memory and grows downward. __stack_min__ is used by the
	 * malloc implementation to ensure heap never collides with stack (assuming stack
	 * never grows beyond Stack_Size_Total in length) */
	__top_of_stack__ = ALIGN(ORIGIN(Ram) + LENGTH(Ram), 8);
	__stack_min__ = ALIGN(__top_of_stack__ - Stack_Size_Total, 8);

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

ENTRY( vectors )


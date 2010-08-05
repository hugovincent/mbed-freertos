/* Based on GNU toolchain standard linker script (arm-none-eabi/lib/ldscripts/armelf.x)
 *
 * Hugo Vincent, 4 August 2010.
 */

OUTPUT_FORMAT( "elf32-littlearm", "elf32-bigarm", "elf32-littlearm" )
OUTPUT_ARCH( arm )
ENTRY( vectors )

/* C-accessible symbols for memory address ranges: */
__FLASH_segment_start__ = ORIGIN( Flash );
__FLASH_segment_end__   = ORIGIN( Flash ) + LENGTH( Flash );
__SRAM_segment_start__  = ORIGIN( Ram );
__SRAM_segment_end__    = ORIGIN( Ram ) + LENGTH( Ram );

/* Stacks (full descending) at top of RAM, grows downward:
 *
 * __stack_min__ is used by the malloc implementation to ensure heap never collides
 * with stack (assuming stack never grows beyond Stack_Size_Total in length) */
__stacks_top__ = __SRAM_segment_end__;
__stacks_min__ = __SRAM_segment_end__ - Stack_Size_Total;

SECTIONS
{
	/* first section is .text which is used for code */
	.text :
	{
		__text_start__ = . ;
		__privileged_code_start__ = . ;

		KEEP( *( .vectors ) )

		*( .privileged_code )

		/* Privileged code section has to be power-of-two sized. */
		priv_code_len  = . - __privileged_code_start__ - 1 ;
		priv_code_len |= priv_code_len >>  1 ;
		priv_code_len |= priv_code_len >>  2 ;
		priv_code_len |= priv_code_len >>  4 ;
		priv_code_len |= priv_code_len >>  8 ;
		priv_code_len |= priv_code_len >> 16 ;
		. = ALIGN( MAX( priv_code_len + 1, 32 ) ) ;

		__privileged_code_end__ = .;

		*( .text .text.* .gnu.linkonce.t.* )
		*( .plt )
		*( .gnu.warning )
		*( .glue_7t ) *( .glue_7 ) *( .vfp11_veneer )

		*( .rodata .rodata.* .gnu.linkonce.r.* )

		*(.ARM.extab* .gnu.linkonce.armextab.*)
		*(.gcc_except_table)
		*(.eh_frame_hdr)
		*(.eh_frame)

		. = ALIGN( 4 ) ;
		KEEP( *( .init ) )

		. = ALIGN( 4 ) ;
		__preinit_array_start = . ;
		KEEP( *( .preinit_array ) )
		__preinit_array_end = . ;

		. = ALIGN( 4 ) ;
		__init_array_start = . ;
		KEEP( *( SORT( .init_array.* ) ) )
		KEEP( *( .init_array ) )
		__init_array_end = . ;

		. = ALIGN( 4 ) ;
		KEEP( *crtbegin.o( .ctors ) )
		KEEP( *( EXCLUDE_FILE( *crtend.o ) .ctors ) )
		KEEP( *( SORT( .ctors.* ) ) )
		KEEP( *crtend.o( .ctors ) )

		. = ALIGN( 4 ) ;
		KEEP( *( .fini ) )

		. = ALIGN( 4 ) ;
		__fini_array_start = . ;
		KEEP( *( .fini_array ) )
		KEEP( *( SORT( .fini_array.* ) ) )
		__fini_array_end = . ;

		KEEP( *crtbegin.o( .dtors ) )
		KEEP( *( EXCLUDE_FILE( *crtend.o ) .dtors ) )
		KEEP( *( SORT( .dtors.* ) ) )
		KEEP( *crtend.o( .dtors ) )

	} >Flash

	/* .ARM.exidx is used for exception handling. Also it's sorted so there
	 * are some section & alignment schenanigans here... */
	__exidx_start = . ;
	.ARM.exidx :
	{
		*( .ARM.exidx* .gnu.linkonce.armexidx.* )
	} >Flash
	__exidx_end = . ;
	.text.align :
	{
		. = ALIGN( 8 ) ;
	} >Flash
	__text_end__ = . ;

	/* .data section -- used for initialized data */
	.data :
	{
		__data_start__ = . ;
		KEEP( *( .jcr ) )
		*( .got.plt ) *( .got )
		*( .shdata )
		*( .data .data.* .gnu.linkonce.d.* )
		*( .ram )
		. = ALIGN( 8 ) ;
		__data_end__ = . ;
	} >Ram AT>Flash

	/* .bss section -- used for uninitialized data */
	.bss :
	{
		__bss_start__ = . ;

		. = ALIGN( 32 ) ;
		__privileged_bss_start__ = . ;

		*crt0.o( .privileged_bss )
		*( .privileged_bss )

		/* Privileged bss section has to be power-of-two sized. */
		priv_bss_len  = . - __privileged_bss_start__  - 1 ;
		priv_bss_len |= priv_bss_len >>  1 ;
		priv_bss_len |= priv_bss_len >>  2 ;
		priv_bss_len |= priv_bss_len >>  4 ;
		priv_bss_len |= priv_bss_len >>  8 ;
		priv_bss_len |= priv_bss_len >> 16 ;
		. = ALIGN( MAX( priv_bss_len + 1, 32 ) ) ;

		__privileged_bss_end__ = . ;

		*( .shbss )
		*( .bss .bss.* .gnu.linkonce.b.* )
		*( COMMON )
		*( .ram.b )
		. = ALIGN( 8 ) ;
		__bss_end__ = . ;
	} >Ram AT>Flash

	/* Heap starts here and grows up in memory */
	. = ALIGN( 8 ) ;
	__heap_start__ = . ;
	.heap :
	{
		*( .heap )
	} >Ram
	.stack __stacks_min__ :
	{
		*( .stack )
	} >Ram


	.stab          0 (NOLOAD) : { *(.stab) }
	.stabstr       0 (NOLOAD) : { *(.stabstr) }

	/* -------------------------- DEBUG STUFF (not in binary) -------------- */
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
	/* DWARF 3 */
	.debug_pubtypes 0 : { *(.debug_pubtypes) }
	.debug_ranges   0 : { *(.debug_ranges) }

	.note.gnu.arm.ident 0 : { KEEP( *( .note.gnu.arm.ident ) ) }
	.ARM.attributes     0 : { KEEP( *( .ARM.attributes ) ) KEEP( *( .gnu.attributes ) ) }
	/DISCARD/             : { *( .note.GNU-stack ) }
}


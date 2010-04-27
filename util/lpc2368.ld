/* Based on scripts found in the LPC yahoo-group and the Keil examples 
 * modified by Martin Thomas, Kaiserslautern, Germany
 * modified by Hugo Vincent <http://hugovincent.com>
 */

/* Memory Definitions */
MEMORY
{
	ROM (rx)	: ORIGIN = 0x00000000, LENGTH = (512k-4k)
	RAM (rw)	: ORIGIN = 0x40000000, LENGTH = 32k
	USBRAM (rw) : ORIGIN = 0x7FD00000, LENGTH = 8k
	ETHRAM (rw) : ORIGIN = 0x7FE00000, LENGTH = 16k
	BATRAM (rw) : ORIGIN = 0xE0084000, LENGTH = 2k
}

/* Section Definitions */
SECTIONS
{
	/* first section is .text which is used for code */
	.text :
	{
		KEEP(*(.RESET))				/* Startup code from .init-section */
		*(.text .text.*)			/* remaining code */
		*(.gnu.linkonce.t.*)
		*(.glue_7)
		*(.glue_7t)
		*(.gcc_except_table)
		*(.rodata)					/* read-only data (constants) */
		*(.rodata*)
		*(.gnu.linkonce.r.*)
		. = ALIGN(4);
	} > ROM

	. = ALIGN(4);

	/* .ctors .dtors are used for c++ constructors/destructors */
	.ctors :
	{
	  PROVIDE(__ctors_start__ = . );
	  KEEP(*(SORT(.ctors.*)))
	  KEEP(*(.ctors))
	  . = ALIGN(4);
	  PROVIDE(__ctors_end__ = . );
	  . = ALIGN(4);
	} > ROM

	.dtors :
	{
		PROVIDE(__dtors_start__ = . ); 
		KEEP(*(SORT(.dtors.*)))
		KEEP(*(.dtors))
		. = ALIGN(4);
		PROVIDE(__dtors_end__ = . );
		. = ALIGN(4);
	} > ROM

	/* for exception handling/unwind - some Newlib functions use this. */
	.ARM.extab : 
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > ROM

	__exidx_start = .;
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > ROM
	__exidx_end = .;

	. = ALIGN(4);

	_etext = . ;
	PROVIDE (etext = . );

	/* .data section which is used for initialized data */
	.data : AT (_etext)
	{
		_data = . ;
		KEEP(*(.vectmapped)) /* mthomas: when code in ROM and vectors remapped */
		. = ALIGN(4);
		*(.fastrun) /* mthomas: for "RAMFUNC" */
		. = ALIGN(4);
		SORT(CONSTRUCTORS)
		. = ALIGN(4);
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		. = ALIGN(4);
	} > RAM

	. = ALIGN(4);
	_edata = . ;
	PROVIDE (edata = . );

	/* .bss section which is used for uninitialized data */
	.bss (NOLOAD) :
	{
		__bss_start__ = . ;
		*(.bss) 
		*(.bss.*) /* mthomas 7/2006 */
		*(.gnu.linkonce.b*)
		*(COMMON)
		. = ALIGN(4);
	} > RAM

	. = ALIGN(4);
	__bss_end__ = . ;
	PROVIDE (__bss_end = . );

	.stack ALIGN(256) :
	{
		*(.stack)
		*(.STACK)
		PROVIDE (_stack = . );
		. = ALIGN(4);
	} > RAM

	_end = . ;
	PROVIDE (end = . );

	.heap :
	{
		*(.HEAP)
		. = ALIGN(4);
	} > RAM

	.usbram (NOLOAD):
	{
		__usbram_beg__ = . ;
		*(.usbram)
		__usbram_end__ = . ;
	} > USBRAM

	.ethram (NOLOAD):
	{
		__ethram_beg__ = . ;
		*(.ethram)
		__ethram_end__ = . ;
	} > ETHRAM

	.batram (NOLOAD):
	{
		__batram_beg__ = . ;
		*(.batram)
		__batram_end__ = . ;
	} > BATRAM

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


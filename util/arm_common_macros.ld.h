/* Common fragments and macros used in linker scripts.
 *
 * Hugo Vincnet, 5 August 2010.
 */

/* MPU regions have to be power-of-two sized, and have to be aligned to their
 * size. This pads so that the size from start_addr to where this macro is
 * instantiated is power-of-two sized. */
#define MPU_REGION_SIZE(start_addr) 										\
		start_addr##_len  = . - start_addr - 1 ;							\
		start_addr##_len |= start_addr##_len >>  1 ;						\
		start_addr##_len |= start_addr##_len >>  2 ;						\
		start_addr##_len |= start_addr##_len >>  4 ;						\
		start_addr##_len |= start_addr##_len >>  8 ;						\
		start_addr##_len |= start_addr##_len >> 16 ;						\
		. = ALIGN( MAX( start_addr##_len + 1, 32 ) ) ;						\

#define MPU_REGION_ALIGN(size) FIXME

#define DEBUG_STUFF()														\
	.stab    0 (NOLOAD) : { *(.stab) }										\
	.stabstr 0 (NOLOAD) : { *(.stabstr) }									\
	/* DWARF debug sections. */												\
	/* Symbols in the DWARF debugging sections are relative to the 	*/		\
	/* beginning of the section so we begin them at 0.  			*/		\
	/* DWARF 1 */															\
	.debug           0 : { *(.debug) }										\
	.line            0 : { *(.line) }										\
	/* GNU DWARF 1 extensions */											\
	.debug_srcinfo   0 : { *(.debug_srcinfo) }								\
	.debug_sfnames   0 : { *(.debug_sfnames) }								\
	/* DWARF 1.1 and DWARF 2 */												\
	.debug_aranges   0 : { *(.debug_aranges) }								\
	.debug_pubnames  0 : { *(.debug_pubnames) }								\
	/* DWARF 2 */															\
	.debug_info      0 : { *(.debug_info .gnu.linkonce.wi.*) }				\
	.debug_abbrev    0 : { *(.debug_abbrev) }								\
	.debug_line      0 : { *(.debug_line) }									\
	.debug_frame     0 : { *(.debug_frame) }								\
	.debug_str       0 : { *(.debug_str) }									\
	.debug_loc       0 : { *(.debug_loc) }									\
	.debug_macinfo   0 : { *(.debug_macinfo) }								\
	/* SGI/MIPS DWARF 2 extensions */										\
	.debug_weaknames 0 : { *(.debug_weaknames) }							\
	.debug_funcnames 0 : { *(.debug_funcnames) }							\
	.debug_typenames 0 : { *(.debug_typenames) }							\
	.debug_varnames  0 : { *(.debug_varnames) }								\
	/* DWARF 3 */															\
	.debug_pubtypes  0 : { *(.debug_pubtypes) }								\
	.debug_ranges    0 : { *(.debug_ranges) }								\
																			\
	.note.gnu.arm.ident 0 : { KEEP( *( .note.gnu.arm.ident ) ) }			\
	.ARM.attributes     0 : { 												\
		KEEP( *( .ARM.attributes ) ) 										\
		KEEP( *( .gnu.attributes ) )										\
	}																		\
	/DISCARD/             : { *( .note.GNU-stack ) }						\

#define TEXT()																\
	*( .text .text.* .gnu.linkonce.t.* )									\
	*( .plt )																\
	*( .gnu.warning )														\
	*( .glue_7t ) *( .glue_7 ) *( .vfp11_veneer )							\

#define RODATA()															\
	*( .rodata .rodata.* .gnu.linkonce.r.* )								\

#define EXCEPTION_HANDLING_TABLES()											\
	*(.ARM.extab* .gnu.linkonce.armextab.*)									\
	*(.gcc_except_table)													\
	*(.eh_frame_hdr)														\
	*(.eh_frame)															\

#define INIT_ARRAY()														\
	. = ALIGN( 4 ) ;														\
	KEEP( *( .init ) )														\
	. = ALIGN( 4 ) ;														\
	__preinit_array_start = . ;												\
	KEEP( *( .preinit_array ) )												\
	__preinit_array_end = . ;												\
	. = ALIGN( 4 ) ;														\
	__init_array_start = . ;												\
	KEEP( *( SORT( .init_array.* ) ) )										\
	KEEP( *( .init_array ) )												\
	__init_array_end = . ;													\

#define CONSTRUCTORS()														\
	. = ALIGN( 4 ) ;														\
	KEEP( *crtbegin.o( .ctors ) )											\
	KEEP( *( EXCLUDE_FILE( *crtend.o ) .ctors ) )							\
	KEEP( *( SORT( .ctors.* ) ) )											\
	KEEP( *crtend.o( .ctors ) )												\

#define FINI_ARRAY()														\
	. = ALIGN( 4 ) ;														\
	KEEP( *( .fini ) )														\
	. = ALIGN( 4 ) ;														\
	__fini_array_start = . ;												\
	KEEP( *( .fini_array ) )												\
	KEEP( *( SORT( .fini_array.* ) ) )										\
	__fini_array_end = . ;													\

#define DESTRUCTORS()														\
	KEEP( *crtbegin.o( .dtors ) )											\
	KEEP( *( EXCLUDE_FILE( *crtend.o ) .dtors ) )							\
	KEEP( *( SORT( .dtors.* ) ) )											\
	KEEP( *crtend.o( .dtors ) )												\

#define EXCEPTION_INDEX_SECTION()											\
	__exidx_start = . ;														\
	.ARM.exidx : {															\
		*( .ARM.exidx* .gnu.linkonce.armexidx.* )							\
	} >Flash																\
	__exidx_end = . ;														\

#define BSS()																\
	*( .shbss )																\
	*( .bss .bss.* .gnu.linkonce.b.* )										\
	*( COMMON )																\
	*( .ram.b )																\
	. = ALIGN( 8 ) ;														\

#define DATA()																\
	KEEP( *( .jcr ) )														\
	*( .got.plt ) *( .got )													\
	*( .shdata )															\
	*( .data .data.* .gnu.linkonce.d.* )									\
	*( .ram )																\
	. = ALIGN( 8 ) ;														\


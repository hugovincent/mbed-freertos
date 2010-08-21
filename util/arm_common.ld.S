/* Based on GNU toolchain standard linker script (arm-none-eabi/lib/ldscripts/armelf.x)
 *
 * Hugo Vincent, 4 August 2010.
 */

#include "util/arm_common_macros.ld.h"

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
	__text_start__ = . ;
	.text : {
		__privileged_code_start__ = . ;

		KEEP( *( .vectors ) )
		*( .privileged_code )
#ifdef HAVE_MPU
		MPU_REGION_SIZE(__privileged_code_start__)
#endif
		__privileged_code_end__ = .;

		TEXT()
		RODATA()
		EXCEPTION_HANDLING_TABLES()
		INIT_ARRAY()
		CONSTRUCTORS()
		FINI_ARRAY()
		DESTRUCTORS()

	} >Flash
	EXCEPTION_INDEX_SECTION()
	.text.align : { . = ALIGN( 8 ) ; } >Flash /* Alignment schenanigans */
	__text_end__ = . ;

	/* .bss section -- used for uninitialized data */
	/* Located at the start of RAM */
	.bss (NOLOAD) : {
		__bss_start__ = . ;

		__privileged_bss_start__ = . ;
		/* It is easiest to put RAM vectors here to meet the elaborate
		 * alignment requirements placed upon them. */
		*crt0.o( .ram_vectors )
		*crt0.o( .privileged_bss )
		*( .privileged_bss )
#ifdef HAVE_MPU
		MPU_REGION_SIZE(__privileged_bss_start__)
#endif
		__privileged_bss_end__ = . ;

		BSS()

		__bss_end__ = . ;
	} >Ram AT>Flash

	/* .data section -- used for initialized data */
	.data : {
		__data_start__ = . ;
		DATA()
		__data_end__ = . ;
	} >Ram AT>Flash

	__data_init_start__ = LOADADDR( .data ) ;

	/* Heap starts here and grows up in memory */
	. = ALIGN( 8 ) ;
	__heap_start__ = . ;

	DEBUG_STUFF()
}


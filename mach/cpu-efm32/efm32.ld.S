/* Memory Definitions: */
MEMORY
{
  Flash (rx) : ORIGIN = 0x00000800, LENGTH = 126K /* We use the bootloader, so first 2K are reserved */
  Ram (rwx) : ORIGIN = 0x20000000, LENGTH = 16K
}

/* Stack sizes: */
C_Stack_Size = 768;
Stack_Size_Total = C_Stack_Size;  /*FIXME other stacks? */

/* Define that we have and want to use an MPU */
#define HAVE_MPU

#include "util/arm_common.ld.S"


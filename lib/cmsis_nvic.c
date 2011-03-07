#include "cmsis_nvic.h"
#include "FreeRTOS.h"

#if defined(TARGET_LPC17xx) || defined(TARGET_EFM32)

PRIVILEGED_FUNCTION void NVIC_SetVector(IRQn_Type IRQn, uint32_t vect)
{
	extern void (* __ram_vectors[])(void);
	void (*vector)(void) = (void (*)(void))vect;

	if (IRQn >= 0) // stupid negative ARMv7 vector defs
	{
		__ram_vectors[16 + IRQn] = vector;
	}
	else
	{
		switch (IRQn)
		{
			case NonMaskableInt_IRQn:
				__ram_vectors[2] = vector;
				break;
			case MemoryManagement_IRQn:
				__ram_vectors[4] = vector;
				break;
			case BusFault_IRQn:
				__ram_vectors[5] = vector;
				break;
			case UsageFault_IRQn:
				__ram_vectors[6] = vector;
				break;
			case SVCall_IRQn:
				__ram_vectors[11] = vector;
				break;
			case DebugMonitor_IRQn:
				__ram_vectors[12] = vector;
				break;
			case PendSV_IRQn:
				__ram_vectors[14] = vector;
				break;
			case SysTick_IRQn:
				__ram_vectors[15] = vector;
				break;
			default:
				break;
		}
	}
}

#elif defined(TARGET_LPC23xx)

/* static inline function in LPC2368/core_arm7.h */

#else
#error "CMSIS Target not recognised"
#endif


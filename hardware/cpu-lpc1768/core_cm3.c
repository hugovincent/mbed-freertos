#include <cmsis.h>

void NVIC_SetVector(IRQn_Type IRQn, uint32_t vector)
{
	extern long *__ram_vectors_start__;

	if (IRQn >= 0) // stupid negative ARMv7 vector defs
	{
		*(__ram_vectors_start__ + 16 + IRQn) = vector;
	}
	else
	{
		switch (IRQn)
		{
			case NonMaskableInt_IRQn:
				*(__ram_vectors_start__ + 2) = vector;
				break;
			case MemoryManagement_IRQn:
				*(__ram_vectors_start__ + 4) = vector;
				break;
			case BusFault_IRQn:
				*(__ram_vectors_start__ + 5) = vector;
				break;
			case UsageFault_IRQn:
				*(__ram_vectors_start__ + 6) = vector;
				break;
			case SVCall_IRQn:
				*(__ram_vectors_start__ + 11) = vector;
				break;
			case DebugMonitor_IRQn:
				*(__ram_vectors_start__ + 12) = vector;
				break;
			case PendSV_IRQn:
				*(__ram_vectors_start__ + 14) = vector;
				break;
			case SysTick_IRQn:
				*(__ram_vectors_start__ + 15) = vector;
				break;
			default:
				break;
		}
	}
}


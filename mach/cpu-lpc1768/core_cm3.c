#include <cmsis.h>

void NVIC_SetVector(IRQn_Type IRQn, uint32_t vector)
{
	extern uint32_t __ram_vectors[];

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


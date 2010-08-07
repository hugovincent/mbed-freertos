#include <stdlib.h>
#include <cmsis.h>
#include "drivers/gpio.h"

static inline LPC_GPIO_TypeDef *block(int which)
{
	switch (which)
	{
    	case 0:
			return LPC_GPIO0;
		case 1:
			return LPC_GPIO1;
		case 2:
			return LPC_GPIO2;
		case 3:
			return LPC_GPIO3;
		case 4:
			return LPC_GPIO4;
		default:
			return NULL;
	}
}

void GPIO_Init()
{
	// Enable GPIO peripheral power for GPIO0 and GPIO1
	LPC_SC->PCONP |= 0x00008000;

	// We have four LEDs on P1.18, P1.20, P1,21, and P1,23.
	LPC_GPIO1->FIODIR |= (0x1 << 18) | (0x1 << 20) | (0x1 << 21) | (0x1 << 23);

	// And two ethernet LEDs on P0.4 and P0.5
	LPC_GPIO0->FIODIR |= (0x1 << 4) | (0x1 << 5);
}

void GPIO_Write(int which, unsigned int set_bitmap, unsigned int clear_bitmap)
{
	block(which)->FIOSET = set_bitmap;
	block(which)->FIOCLR = clear_bitmap;
}

void GPIO_Toggle(int which, unsigned int bitmap)
{
	unsigned int oldval = GPIO_Read(which);
	GPIO_Write(which, ~oldval & bitmap, oldval & bitmap);
}

unsigned int GPIO_Read(int which)
{
	return block(which)->FIOPIN;
}




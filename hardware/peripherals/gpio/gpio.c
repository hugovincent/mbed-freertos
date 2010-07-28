/*
    FreeRTOS V6.0.4 - Copyright (C) 2010 Real Time Engineers Ltd.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    ***NOTE*** The exception to the GPL is included to allow you to distribute
    a combined work that includes FreeRTOS without being obliged to provide the
    source code for proprietary components outside of the FreeRTOS kernel.
    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public 
    License and the FreeRTOS license exception along with FreeRTOS; if not it 
    can be viewed here: http://www.freertos.org/a00114.html and also obtained 
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.
*/

/* FreeRTOS.org includes. */
#include <FreeRTOS.h>

/* Demo application includes. */
#include "hardware/gpio.h"

void vGpioInitialise()
{
	// We have four LEDs on P1.18, P1.20, P1,21, and P1,23.
	LPC_PINCON->PINSEL10 = 0x0;
	LPC_GPIO1->FIODIR |= (0x1 << 18) | (0x1 << 20) | (0x1 << 21) | (0x1 << 23);
}

void vGpioSet(unsigned portBASE_TYPE uxPin, signed portBASE_TYPE xValue)
{
	/* Set of clear the output. */
	if (!xValue)
	{
		LPC_GPIO1->FIOCLR = 0x1 << uxPin;
	}
	else
	{
		LPC_GPIO1->FIOSET = 0x1 << uxPin;
	}
}

void vGpioToggle(unsigned portBASE_TYPE uxPin)
{
	/* If this bit is already set, clear it, and visa versa. */
	portLONG ulCurrentState = LPC_GPIO1->FIOPIN;
	if (ulCurrentState & (0x1 << uxPin))
	{
		LPC_GPIO1->FIOCLR = 0x1 << uxPin;
	}
	else
	{
		LPC_GPIO1->FIOSET = 0x1 << uxPin;
	}
}

unsigned portBASE_TYPE uxGpioGet(unsigned portBASE_TYPE uxPin)
{
    return !(LPC_GPIO1->FIOPIN & (0x1 << uxPin)) ? 1 : 0;
}



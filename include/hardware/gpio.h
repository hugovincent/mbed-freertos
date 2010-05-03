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

#ifndef GPIO_H
#define GPIO_H

#include <FreeRTOS.h>

#ifdef __cplusplus
extern "C" {
#endif

// FIXME register pins, set direction, read pins, register interrupt

void vGpioInitialise(void);
void vGpioSet(unsigned portBASE_TYPE uxPin, signed portBASE_TYPE xValue);
void vGpioToggle(unsigned portBASE_TYPE uxPin);
unsigned portBASE_TYPE uxGpioGet(unsigned portBASE_TYPE uxPin);

#ifdef __cplusplus
} // extern "C"
#endif

#endif


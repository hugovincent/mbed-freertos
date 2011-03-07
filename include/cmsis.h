/* mbed Microcontroller Library - CMSIS
 * Copyright (C) 2009 ARM Limited. All rights reserved.
 * 
 * A generic CMSIS include header, pulling in the appropriate
 * target specific CMSIS files 
 */

#ifndef MBED_CMSIS_H
#define MBED_CMSIS_H

#if defined(TARGET_LPC17xx)
#include "LPC17xx.h"
#elif defined(TARGET_LPC23xx)
#include "LPC23xx.h"
#elif defined(TARGET_LPC29xx)
#include "LPC29xx.h"
#elif defined(TARGET_EFM32)
#include "efm32.h"
#else
#error "CMSIS Target not recognised"
#endif

#include "cmsis_nvic.h"

#endif

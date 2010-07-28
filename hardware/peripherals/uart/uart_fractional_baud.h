#ifndef FractionalBaud_h
#define FractionalBaud_h

#include "FreeRTOS.h"

void FindBaudWithFractional(unsigned portLONG ulWantedBaud, 
		unsigned portLONG *ulDivisor, unsigned portLONG *ulFracDiv);

#endif // ifndef FractionaBaud_h

/* Fractional baud rate generation for LPC23xx (and proably other LPCxxxx) UARTs.
 *
 * Hugo Vincent, April 28 2010.
 */

#include "cmsis.h"
#include <math.h>

#include "uart_fractional_baud.h"

struct FracBaudLine {
	float FRest;
	uint8_t divAddVal, mulVal;
};

static struct FracBaudLine FractionalBaudTable[72] = 
{
	{1.000,0,1}, 
	{1.067,1,15}, 
	{1.071,1,14}, 
	{1.077,1,13}, 
	{1.083,1,12}, 
	{1.091,1,11}, 
	{1.100,1,10}, 
	{1.111,1,9}, 
	{1.125,1,8}, 
	{1.133,2,15}, 
	{1.143,1,7}, 
	{1.154,2,13}, 
	{1.167,1,6}, 
	{1.182,2,11}, 
	{1.200,1,5}, 
	{1.214,3,14}, 
	{1.222,2,9}, 
	{1.231,3,13}, 
	{1.250,1,4}, 
	{1.267,4,15}, 
	{1.273,3,11}, 
	{1.286,2,7}, 
	{1.300,3,10}, 
	{1.308,4,13}, 
	{1.333,1,3}, 
	{1.357,5,14}, 
	{1.364,4,11}, 
	{1.375,3,8}, 
	{1.385,5,13}, 
	{1.400,2,5}, 
	{1.417,5,12}, 
	{1.429,3,7}, 
	{1.444,4,9}, 
	{1.455,5,11}, 
	{1.462,6,13}, 
	{1.467,7,15}, 
	{1.500,1,2}, 
	{1.533,8,15}, 
	{1.538,7,13}, 
	{1.545,6,11}, 
	{1.556,5,9}, 
	{1.571,4,7}, 
	{1.583,7,12}, 
	{1.600,3,5}, 
	{1.615,8,13}, 
	{1.625,5,8}, 
	{1.636,7,11}, 
	{1.643,9,14}, 
	{1.667,2,3}, 
	{1.692,9,13}, 
	{1.700,7,10}, 
	{1.714,5,7}, 
	{1.727,8,11}, 
	{1.733,11,15}, 
	{1.750,3,4}, 
	{1.769,10,13}, 
	{1.778,7,9}, 
	{1.786,11,14}, 
	{1.800,4,5}, 
	{1.818,9,11}, 
	{1.833,5,6}, 
	{1.846,11,13}, 
	{1.857,6,7}, 
	{1.867,13,15}, 
	{1.875,7,8}, 
	{1.889,8,9}, 
	{1.900,9,10}, 
	{1.909,10,11}, 
	{1.917,11,12}, 
	{1.923,12,13}, 
	{1.929,13,14}, 
	{1.933,14,15}
};

// Setup a fractional baud rate (this based on figure 83 in the UM --hugo 24/5/10)
void FindBaudWithFractional(uint32_t wantedBaud, uint32_t *divisor, uint32_t *fracDiv)
{
	float FRest = 1.5;
	int divAddVal = 0, mulVal = 1;

	// Setup the baud rate:  Calculate the divisor value.
	// Note: PCLK is CCLK/4, so the 16 in the equations becomes 64.
	*divisor = SystemCoreClock / (wantedBaud * 64);

	// Check for integer divisor, otherwise compute fractional divisors
	if (SystemCoreClock % (wantedBaud * 64) != 0)
	{
		*divisor = (unsigned portLONG)floorf(SystemCoreClock 
				/ (wantedBaud * 64 * FRest));
		FRest = SystemCoreClock / (64 * wantedBaud * (float)(*divisor));
		if (FRest > 1.1 && FRest < 1.9)
		{
			for (unsigned char j = 0; j < 71; j++)
			{
				if (FractionalBaudTable[j].FRest > FRest 
						&& FRest < FractionalBaudTable[j+1].FRest)
				{
					mulVal = FractionalBaudTable[j].mulVal;
					divAddVal = FractionalBaudTable[j].divAddVal;
					break;
				}
			}
		}
	}
	*fracDiv = (divAddVal & 0x0F) | ((mulVal & 0x0F) << 4);
}

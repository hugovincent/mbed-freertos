extern "C" {
#include "FreeRTOS.h"
#include "partest.h"
#include "CxxTest.h"
}

int CxxTest::someMethod() {
	vParTestToggleLED( 18 );
	return 0;
}


extern "C" {
#include <FreeRTOS.h>
#include "hardware/gpio.h"
#include "CxxTest.h"
}

#include <string>

int CxxTest::someMethod() {

	// FIXME this isn't a very good test cos it gets optimized out... FAIL!
	std::string *t = new std::string();
	*t = "Hello world";
	delete t;

	vGpioToggle( 18 );
	return 0;
}


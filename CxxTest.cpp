extern "C" {
#include <FreeRTOS.h>
#include "hardware/gpio.h"
#include "CxxTest.h"
}

//#include <string>

int CxxTest::someMethod() {

#if 0
	std::string *t = new std::string();
	*t = "Hello world";
	delete t;
	char *v = new char[100];
	v = "Hello world";
	delete[] v;
#endif

	vGpioToggle( 18 );
	return 0;
}


#include <FreeRTOS.h>
#include "drivers/gpio.h"
#include <string>

class CxxTest {
public:
	CxxTest() {}
	~CxxTest() {}

	int someMethod();
};

int CxxTest::someMethod() {

#if 0
	std::string *t = new std::string();
	*t = "Hello world";
	delete t;
#endif

	vGpioToggle( 18 );
	return 0;
}


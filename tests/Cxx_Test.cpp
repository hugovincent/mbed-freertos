#include <FreeRTOS.h>
#include "drivers/gpio.h"
//#include <string>

class CxxTest {
public:
	CxxTest() : have_initted(true) { printf("in the constructor\n"); }
	~CxxTest() { have_initted = false; printf("destructing\n"); }

	int someMethod();
private:
	bool have_initted;
};

static CxxTest obj;

int CxxTest::someMethod() {

/*	std::string *t = new std::string();
	*t = "Hello world";
	delete t;
*/
	GPIO_PinWrite(1, 18, have_initted);
	return 0;
}

void test_cxx() 
{
	obj.someMethod();
}

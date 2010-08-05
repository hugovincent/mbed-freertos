#include <FreeRTOS.h>
//#include <string>

class CxxTest {
public:
	CxxTest() : have_initted(true) { /*printf("in the constructor\n");*/ }
	~CxxTest() { /*have_initted = false; printf("in the destructor\n");*/ }

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
	if (have_initted)
		printf("in someMethod() - initted correctly\n");

	return 0;
}

void test_cxx() 
{
	obj.someMethod();
}

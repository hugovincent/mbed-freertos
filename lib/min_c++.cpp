/* Minimal memory handling for C++. Avoids the standard implementation which pull in
 * a bunch of exception handling code...
 *
 * Based on:
 * http://www.embedded.com/columns/technicalinsights/201001729?_requestid=139460
 */

#include <stdlib.h>

void *operator new(size_t size) throw()
{
	return malloc(size);
}

void operator delete(void *p) throw()
{
	free(p);
}

void *operator new[](size_t size)
{
	return malloc(size);
}

void operator delete[](void *p)
{
	free(p);
}

/* FIXME this is fine for now (no global destructors) but maybe we want to do something
 * else here, e.g. tie in with FreeRTOS task teardown.
 */
extern "C" int __aeabi_atexit(void *object, void (*destructor)(void *), void *dso_handle)
{
	return 0;
}

/* This calls calls any static constructors before starting main.
 * As main never returns (running bare metal), we don't worry about static destructors.
 */
void __cxx_main(void)
{
	extern unsigned long __ctors_start__, __ctors_end__;
	extern int main();

	// call all the static constructors in the list.
	for(unsigned long *constructor(&__ctors_start__); constructor < &__ctors_end__; ++constructor)
	{
		((void (*) (void)) (*constructor)) ();
	}

	// call proper main function
	main();
}


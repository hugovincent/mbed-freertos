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

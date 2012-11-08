#include "exceptions.h"
#include "return.h"

extern "C" void __cxa_rethrow();
extern "C" void* __cxa_begin_catch(void*);
extern "C" void __cxa_end_catch();

void objc_exception_rethrow()
{
	__cxa_rethrow();
}

void* objc_begin_catch(void* p)
{
	void *rv = returnReturn();
	void* cpp = __cxa_begin_catch(p);
	if (cpp)
		return cpp;
	else
		return rv;
}

void objc_end_catch()
{
	__cxa_end_catch();
}


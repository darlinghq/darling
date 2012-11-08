#include "exceptions.h"
#include "return.h"

extern "C" void __cxa_rethrow();
extern "C" void* __cxa_begin_catch(void*);
extern "C" void __cxa_end_catch();

static __thread bool m_cxx = false;

void objc_exception_rethrow()
{
	__cxa_rethrow();
}

void* objc_begin_catch(void* p)
{
	void *rv = returnReturn();
	void* cpp = __cxa_begin_catch(p);
	if (cpp)
	{
		m_cxx = true;
		return cpp;
	}
	else
	{
		m_cxx = false;
		return rv;
	}
}

void objc_end_catch()
{
	if (m_cxx)
		__cxa_end_catch();
}


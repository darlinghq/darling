#include "misc.h"
#include <cstdio>

char* mach_error_string(kern_return_t rv)
{
	static __thread char buf[100];
	snprintf(buf, sizeof(buf), "Return value %d", rv);
	return buf;
}

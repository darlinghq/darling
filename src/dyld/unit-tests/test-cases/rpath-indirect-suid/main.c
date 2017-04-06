#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "test.h"

extern void foo();

int main()
{
	foo();
#if DEFAULT_FAIL
	FAIL("rpath-indirect-setuid");
#else
	PASS("rpath-indirect-setuid");
#endif
	return EXIT_SUCCESS;
}

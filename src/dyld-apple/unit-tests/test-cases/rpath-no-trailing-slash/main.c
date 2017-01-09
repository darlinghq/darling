#include <stdio.h>
#include <stdlib.h>
#include <mach-o/dyld.h>

#include "test.h"

extern void foo();

int main()
{
	foo();
	PASS("rpath-no-trailing-slash");
	return EXIT_SUCCESS;
}

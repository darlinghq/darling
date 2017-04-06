
#include <stdio.h>
#include <stdlib.h>
#include <mach-o/dyld.h>

#include "test.h"

extern int foo();

int main()
{
	if ( foo() )
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

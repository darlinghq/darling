#include <stdio.h>
#include <stdlib.h>
#include <mach-o/dyld.h>

#include "test.h"

extern void foo();

int main()
{
	foo();
	PASS("loader_path-symlink");
	return EXIT_SUCCESS;
}

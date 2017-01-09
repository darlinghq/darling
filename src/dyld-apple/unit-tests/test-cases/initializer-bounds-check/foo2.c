#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

__attribute__((constructor))
void secondInit()
{
	FAIL("initializer-bounds-check, second init called");
	exit(0);
}


#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <dlfcn.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

// libfoo.dylib has a weak global variable that foo() returns

extern int foo();

int main()
{
	if ( foo() == 10 )
		PASS("non-lazy-weak");
	else
		FAIL("non-lazy-weak");
	return EXIT_SUCCESS;
}



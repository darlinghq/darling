#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <dlfcn.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

// libfoo.dylib is built from multiple source files
// It internally uses non-lazy pointer to compute
// the result for foo()


extern int foo();

int main()
{
	if ( foo() == 10 )
		PASS("non-lazy-slide");
	else
		FAIL("non-lazy-slide");
	return EXIT_SUCCESS;
}



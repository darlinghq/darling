#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <dlfcn.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

extern int foo();
extern int bar();

int (*pbar)() = &bar;


int main()
{
	if ( foo() != 10 )
		FAIL("re-export-symbol: foo() returned wrong value");
	if ( bar() != 10 )
		FAIL("re-export-symbol: bar() returned wrong value");
	if ( (*pbar)() != 10 )
		FAIL("re-export-symbol: (*pbar)() returned wrong value");
	PASS("re-export-symbol");
	return 0;
}

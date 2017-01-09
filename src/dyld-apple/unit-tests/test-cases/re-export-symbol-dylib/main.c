
#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <dlfcn.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()



extern int foo();
extern int bar();
extern int baz();
extern int frob();

int (*pfoo)() = &foo;
int (*pbar)() = &bar;
int (*pbaz)() = &baz;
int (*pfrob)() = &frob;


int main()
{
	if ( foo() != 1 )
		FAIL("re-export-symbol-dylib: foo() returned wrong value");
	else if ( bar() != 2 )
		FAIL("re-export-symbol-dylib: bar() returned wrong value");
	else if ( baz() != 3 )
		FAIL("re-export-symbol-dylib: baz() returned wrong value");
	else if ( frob() != 4 )
		FAIL("re-export-symbol-dylib: frob() returned wrong value");
    
	else if ( (*pfoo)() != 1 )
		FAIL("re-export-symbol-dylib: (*pfoo)() returned wrong value");
	else if ( (*pbar)() != 2 )
		FAIL("re-export-symbol-dylib: (*pbar)() returned wrong value");
	else if ( (*pbaz)() != 3 )
		FAIL("re-export-symbol-dylib: (*pbaz)() returned wrong value");
	else if ( (*pfrob)() != 4 )
		FAIL("re-export-symbol-dylib: (*pfrob)() returned wrong value");
    else 
        PASS("re-export-symbol-dylib");
	return 0;
}


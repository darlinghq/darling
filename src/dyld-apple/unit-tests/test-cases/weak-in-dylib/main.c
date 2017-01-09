
#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

// libfoo.dylib has a weak foo[]
extern int foo[];

int* pfoo3 = &foo[3];

int main()
{
	if ( *pfoo3 != 4 )
		FAIL("weak-in-dylib, pfoo3=%d", *pfoo3);
	else if ( foo[2] != 3 ) 
		FAIL("weak-in-dylib, foo[2]=%d", foo[2]);
	else
		PASS("weak-in-dyliby");
	return 0;
}


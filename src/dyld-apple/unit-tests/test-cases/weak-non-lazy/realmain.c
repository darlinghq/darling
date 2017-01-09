

#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

// getfoo() is in libfoo.dylib and uses a non-lazy pointer to reference foo
// libfoo.dylib also has weak foo[]
// getfoo() should be rebound to point into the foo[] in main
int foo[] = { 5, 6, 7, 8 };
extern int* getfoo();

// libfoo.dylib has a weak bar[]
// libbar.dylib has a strong bar[]
// at build time linker only sees weak bar in libfoo.dylib
// at runtime, dyld uses strong bar in libbar.dylib
extern int bar[];


int __attribute__((weak)) stuff[] = { 1, 2, 3, 4, 5 };


void realmain()
{
	if ( getfoo()[2] != 7 )
		FAIL("weak-non-lazy, getfoo()[2]=%d", getfoo()[2]);
	else if ( bar[1] != 21 ) 
		FAIL("weak-non-lazy, bar[1]=%d", bar[1]);
	else if ( stuff[3] != 4 ) 
		FAIL("weak-external-reloc, pstuffr=%d", stuff[3]);
	else
		PASS("weak-non-lazy");
}



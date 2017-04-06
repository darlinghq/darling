

#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

// pfoo is in libfoo.dylib
// libfoo.dylib also has weak foo[]
// pfoo should be rebound to point into the foo[] in main
int foo[] = { 5, 6, 7, 8 };
extern int* pfoo;

// libfoo.dylib has a weak bar[]
// libbar.dylib has a strong bar[]
// at build  time linker  only sees weak bar in libfoo.dylib
// at runtime, dyld uses strong bar in libbar.dylib
extern int bar[];
int* pbar = &bar[1];

// there is only one stuff, but it is weak
// so we are testing the case when stuff is not overridden
int __attribute__((weak)) stuff[] = { 1, 2, 3, 4, 5 };
int* pstuff = &stuff[3];


void realmain()
{
	if ( *pfoo != 7 )
		FAIL("weak-external-reloc, pfoo=%d", *pfoo);
	else if ( *pbar != 21 ) 
		FAIL("weak-external-reloc, pbar=%d", *pbar);
	else if ( *pstuff != 4 ) 
		FAIL("weak-external-reloc, pstuffr=%d", *pstuff);
	else
		PASS("weak-external-reloc");
}



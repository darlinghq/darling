

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


// libfoo.dylib has a weak frob[]
// libbar.dylib has a weak frob[] and a funtion that refrences frob[]
// the function should use libfoo's frob[] even if libfoo is flat
extern int getfrob();

int main()
{
	if ( *pfoo != 7 )
		FAIL("weak-external-reloc-flat, pfoo=%d", *pfoo);
	else if ( *pbar != 21 ) 
		FAIL("weak-external-reloc-flat, pbar=%d", *pbar);
	else if ( getfrob() != 22 ) 
		FAIL("weak-external-reloc-flat, frob()=%d", getfrob());
	else
		PASS("weak-external-reloc-flat");
	
	return EXIT_SUCCESS;
}




// 
// TEST-OPTIONS: -arch i386 -fexceptions -fomit-frame-pointer -Wl,-no_compact_unwind
// TEST-OPTIONS: -arch i386 -fexceptions -Wl,-no_compact_unwind
// TEST-OPTIONS: -arch x86_64 -fomit-frame-pointer -Wl,-no_compact_unwind
// TEST-OPTIONS: -arch x86_64 -Wl,-no_compact_unwind
// TEST-OPTIONS: -arch ppc -fexceptions  
//
// Basic _Unwind_FindEnclosingFunction test
//
// _Unwind_FindEnclosingFunction() is supposed to return the start of a 
// function given an address in the function.  It uses FDE info.
//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "unwind.h"

int bar(int x)
{
	// dummy call to force dwarf info generation
	_Unwind_GetIP(NULL);
	return x + 20;
}

int foo(int x)
{
	// dummy call to force dwarf info generation
	_Unwind_GetIP(NULL);
	return x + 10;
}


int main()
{
	uint8_t* addrInBar = (uint8_t*)bar + 4;
	uint8_t* addrInFoo = (uint8_t*)foo + 4;
	if ( _Unwind_FindEnclosingFunction(addrInBar) != &bar ) 
		return 1;
	if ( _Unwind_FindEnclosingFunction(addrInFoo) != &foo )
		return 1;
	
	return 0;
}



// 
// TEST-OPTIONS:  -arch i386 -fexceptions
// TEST-OPTIONS:  -arch i386 -fexceptions -Wl,-no_compact_unwind
// TEST-OPTIONS:  -arch x86_64
// TEST-OPTIONS:  -arch x86_64 -Wl,-no_compact_unwind
// TEST-OPTIONS:  -arch ppc -fexceptions
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "libunwind.h"

int main()
{
	// verify unw_getcontext() returns no UNW_ESUCCESS
	unw_context_t uc;
	if ( unw_getcontext(&uc) == UNW_ESUCCESS )
		return 0;
	else
		return 1;

}


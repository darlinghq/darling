
// 
// TEST-OPTIONS:   -arch i386 -fexceptions
// TEST-OPTIONS:   -arch i386 -fexceptions -fomit-frame-pointer
// TEST-OPTIONS:   -arch i386 -fexceptions -Wl,-no_compact_unwind
// TEST-OPTIONS:   -arch x86_64
// TEST-OPTIONS:   -arch x86_64 -fomit-frame-pointer
// TEST-OPTIONS:   -arch x86_64 -Wl,-no_compact_unwind
// TEST-OPTIONS:   -arch ppc -fexceptions


//
// Tests _Unwind_RaiseException() returns _URC_END_OF_STACK if there is nothing to catch the exception
//

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "unwind.h"


struct _Unwind_Exception ex;

int main()
{
    ex.exception_class = ((uint64_t)'mymy' << 32) | (uint64_t)'exex'; 
	ex.exception_cleanup = NULL;
	ex.private_1 = 0;
	ex.private_2 = 0;

	_Unwind_Reason_Code result = _Unwind_RaiseException(&ex);
	
	if ( result == _URC_END_OF_STACK )
		return 0;
	else
		return 1;
}


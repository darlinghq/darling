
// 
// TEST-OPTIONS:  -arch i386 
// TEST-OPTIONS:  -arch i386  -Wl,-no_compact_unwind
// TEST-OPTIONS:  -arch x86_64
// TEST-OPTIONS:  -arch x86_64 -Wl,-no_compact_unwind
// TEST-OPTIONS:  -arch ppc 
//
// Tests _Unwind_ForcedUnwind()
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "unwind.h"



static bool exceptionCaught = false;


static _Unwind_Reason_Code stop_func(int version, _Unwind_Action actions, uint64_t exceptionClass,
									_Unwind_Exception* exceptionObject, _Unwind_Context* context, void* stop_parameter)
{
	if ( actions & _UA_END_OF_STACK ) {
		if ( exceptionCaught )
			exit(0);
		else
			exit(1);
	}
	return _URC_NO_REASON;
}


void test()
{
	try {
		_Unwind_Exception* except_obj = (_Unwind_Exception*)calloc(sizeof(_Unwind_Exception), 1);
		_Unwind_ForcedUnwind(except_obj, stop_func, NULL);
	}
	catch(...) {
		// forced unwindind should run this catch clause
		exceptionCaught = true;
		throw;
	}
}

int main()
{
	test();
	return 1;
}


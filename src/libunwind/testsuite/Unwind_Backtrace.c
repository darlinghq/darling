
// 
// TEST-OPTIONS:  backtrace2.c -arch i386 -fexceptions
// TEST-OPTIONS:  backtrace2.c -arch i386 -fexceptions -fomit-frame-pointer
// TEST-OPTIONS:  backtrace2.c -arch i386 -fexceptions -Wl,-no_compact_unwind
// TEST-OPTIONS:  backtrace2.c -arch x86_64
// TEST-OPTIONS:  backtrace2.c -arch x86_64 -fomit-frame-pointer
// TEST-OPTIONS:  backtrace2.c -arch x86_64 -Wl,-no_compact_unwind
// TEST-OPTIONS:  backtrace2.c -arch ppc -fexceptions
//
// Tests _Unwind_Backtrace()
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "unwind.h"


// put intermediate function into another file so compiler does not optimze away 
extern int foo();

static const char* expected[] = { "bar", "foo", "main" };

static int step = 0;
static _Unwind_Reason_Code handler(struct _Unwind_Context* context, void* ref)
{
	if ( step > 2 )
		return _URC_NORMAL_STOP;
	struct dl_info dyldInfo;
	if ( dladdr((void*)_Unwind_GetIP(context), &dyldInfo) ) {
		if ( strcmp(dyldInfo.dli_sname, expected[step]) != 0 ) {
			fprintf(stderr, "unexpected frame %s\n", dyldInfo.dli_sname);
			exit(1);
		}
		++step;
	}
	else {
		exit(1);
	}
	return _URC_NO_REASON;
}



int bar()
{
	_Unwind_Backtrace(&handler, NULL);
	return (step == 2);
}


int main()
{
	return foo();
}


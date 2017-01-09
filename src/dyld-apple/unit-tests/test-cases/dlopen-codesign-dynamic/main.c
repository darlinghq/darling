#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <mach/mach.h>
#include <System/sys/codesign.h>

#include "test.h"


int main()
{
#if ENFORCE
	uint32_t flags = CS_ENFORCEMENT | CS_KILL;
	if ( csops(0, CS_OPS_SET_STATUS, &flags, sizeof(flags)) != 0 ) {
		FAIL("dlopen-codesign-dynamic: csops() failed");
		return EXIT_SUCCESS;
	}
	
	void* handle = dlopen("libfoo.dylib", RTLD_LAZY);
	if ( handle != NULL ) {
		FAIL("dlopen-codesign-dynamic: load of libfoo.dylib should have failed");
		return EXIT_SUCCESS;
	}
	const char* msg = dlerror();
	if ( strstr(msg, "signature") == NULL ) {
		FAIL("dlopen-codesign-dynamic: load of libfoo.dylib failed, but message was wrong: %s", msg);
		return EXIT_SUCCESS;
	}

#else
	void* handle = dlopen("libfoo.dylib", RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("dlopen-codesign-dynamic: load of libfoo.dylib failed");
		return EXIT_SUCCESS;
	}

#endif
	
	PASS("dlopen-codesign-dynamic");
	return EXIT_SUCCESS;
}

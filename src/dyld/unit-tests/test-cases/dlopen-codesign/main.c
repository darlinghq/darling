#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

  
#include "test.h"


int main()
{
	void* handle = dlopen("libfoo.dylib", RTLD_LAZY);
	if ( handle != NULL ) {
		FAIL("dlopen-codesign: load of libfoo.dylib should have failed");
		return EXIT_SUCCESS;
	}
	const char* msg = dlerror();
	if ( strstr(msg, "signature") == NULL ) {
		FAIL("dlopen-codesign: load of libfoo.dylib failed, but message was wrong: %s", msg);
		return EXIT_SUCCESS;
	}
	
	PASS("dlopen-codesign");
	return EXIT_SUCCESS;
}


#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "test.h"

void bar()
{
	void* handle = dlopen("libfoo.dylib", RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("rpath-dlopen-indirect: %s", dlerror());
		exit(EXIT_SUCCESS);
	}
	
	void* pFoo = dlsym(handle, "foo");
	if ( pFoo == NULL ) {
		FAIL("rpath-dlopen-indirect: %s", dlerror());
		exit(EXIT_SUCCESS);
	}
}



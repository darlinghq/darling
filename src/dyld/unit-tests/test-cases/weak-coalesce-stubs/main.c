#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h> // exit(), EXIT_SUCCESS

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

//<rdar://problem/7886402> Loading MH_DYLIB_STUB causing coalescable miscount

int main()
{
	// try to load stub many times
	for (int i=0; i < 10; ++i) {
		void* handle = dlopen("libstub.dylib", RTLD_LAZY);
		if ( handle != NULL ) {
			FAIL("weak-coalesce-stubs: load of libstub.dylib unexpectedly succeeded");
		}
	}
	// try to load real dylib
	dlopen("libbar.dylib", RTLD_LAZY);
	PASS("weak-coalesce-stubs");
	return 0;
}

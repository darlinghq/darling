#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "test.h"

extern void compress();

int main(int argc, const char* argv[])
{
#if !SHOULD_FAIL
	Dl_info info;
	if ( dladdr(&compress, &info) ) {
		//fprintf(stderr, "_compress found in %s\n", info.dli_fname);
		if ( strstr(info.dli_fname, argv[1]) != 0 ) {
			PASS("env-DYLD_FALLBACK_LIBRARY_PATH");
			exit(0);
		}
	}
	FAIL("env-DYLD_FALLBACK_LIBRARY_PATH");
#endif
	return EXIT_SUCCESS;
}

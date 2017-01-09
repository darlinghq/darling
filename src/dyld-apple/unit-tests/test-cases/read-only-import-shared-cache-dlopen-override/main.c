#include <dlfcn.h>

#include "test.h"

int main()
{
	// dynamically load libz.dylib which imports _malloc from libSystem
	dlopen("/usr/lib/libz.dylib", RTLD_LAZY);


	PASS("read-only-import-shared-cache-dlopen-override");
	return 0;
}	


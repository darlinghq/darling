
#include <dlfcn.h>

int main()
{
	// dynamically load libfoo.dylib which depends on libstdc++.dylib 
	// being re-bound to libfoo's operator new.
	dlopen("libfoo.dylib", RTLD_LAZY);
	return 0;
}	



#include <dlfcn.h>


void myinit() __attribute__((constructor));

void myinit()
{
    // call dlopen() in initializer
    void* handle = dlopen(RUN_DIR "/libbar.dylib", RTLD_LAZY);
}

int foo()
{
	return 0;
}


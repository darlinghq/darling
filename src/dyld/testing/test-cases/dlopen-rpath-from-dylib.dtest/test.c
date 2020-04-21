
#include <stdio.h>
#include <dlfcn.h>
#include <stdbool.h>

bool test_dlopen()
{
    void* handle = dlopen("@rpath/libbar.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("[FAIL] dlopen-rpath-from-dylib: dlopen(@rpath/libbar.dylib) failed: %s\n", dlerror());
        return false;
    }

    dlclose(handle);

    return true;
}



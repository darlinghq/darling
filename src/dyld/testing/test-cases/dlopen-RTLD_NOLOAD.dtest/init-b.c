
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <dlfcn.h>

bool doneInitB = false;
bool inInitB = false;


__attribute__((constructor))
void initB()
{
    inInitB = true;

    // "upward" link to libInitA.dylib
    void* handle = dlopen(RUN_DIR "/libInitA.dylib", RTLD_NOLOAD);
    if ( handle == NULL ) {
        printf("[FAIL] dlopen-RTLD_NOLOAD-in-initializer: dlopen(libInitA.dylib, RTLD_NOLOAD) failed but it should have worked: %s\n", dlerror());
        return;
    }
    inInitB = false;

    doneInitB = true;
}

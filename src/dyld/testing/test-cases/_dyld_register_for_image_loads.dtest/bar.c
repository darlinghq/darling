
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

__attribute__((constructor))
void bar() {
    void* handle = dlopen(RUN_DIR "/libbaz.dylib", RTLD_FIRST);
    if ( handle == NULL ) {
        printf("[FAIL] dlopen(\"%s\") failed with: %s\n", RUN_DIR "/libbaz.dylib", dlerror());
        exit(0);
    }
}

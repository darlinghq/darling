
// BUILD:  $CC foo.c -dynamiclib  -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-race.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlopen-race.exe

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>



int main()
{
    printf("[BEGIN] dlopen-read\n");

    __block bool allGood = true;
    dispatch_apply(6, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(size_t index) {
        for (int i=0; i < 500; ++i) {
            void* handle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_LAZY);
            if ( handle == NULL ) {
                printf("[FAIL] dlopen-read: %s\n", dlerror());
                exit(0);
            }
            dlclose(handle);
        }
    });

    printf("[PASS] dlopen-read\n");
	return 0;
}


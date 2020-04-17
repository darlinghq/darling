
// BUILD:  $CC foo.c -dynamiclib -install_name $RUN_DIR/libtlv.dylib -o $BUILD_DIR/libtlv.dylib
// BUILD:  $CC main.c  -DRUN_DIR="$RUN_DIR"  -o $BUILD_DIR/thread-local-cleanup.exe

// RUN:  ./thread-local-cleanup.exe

#include <stdio.h>
#include <dlfcn.h>





int main()
{
    printf("[BEGIN] thread-local-cleanup\n");

    for (int i=0; i < 1000; ++i) {
        void* handle = dlopen(RUN_DIR "/libtlv.dylib", RTLD_FIRST);
        if ( handle == NULL ) {
            printf("[FAIL] thread-local-cleanup: iteration %d %s\n", i, dlerror());
            return 0;
        }

        int result = dlclose(handle);
        if ( result != 0 ) {
            printf("[FAIL] thread-local-cleanup: iteration %d %s\n", i, dlerror());
            return 0;
        }
    }
    
    printf("[PASS] thread-local-cleanup\n");

	return 0;
}


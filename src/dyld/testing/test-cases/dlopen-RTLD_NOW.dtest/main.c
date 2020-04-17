
// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/libbar.dylib         -install_name $RUN_DIR/libbar.dylib
// BUILD:  $CC bar.c -dynamiclib -o $TEMP_DIR/libbar-present.dylib  -install_name $RUN_DIR/libbar.dylib -DHAS_SYMBOL=1
// BUILD:  $CC foo.c -dynamiclib -Os -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib $TEMP_DIR/libbar-present.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-RTLD_NOW.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlopen-RTLD_NOW.exe

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/getsect.h>

#if __LP64__
extern struct mach_header_64 __dso_handle;
#else
extern struct mach_header __dso_handle;
#endif

int main()
{
    printf("[BEGIN] dlopen-RTLD_NOW\n");

    ///
    /// This tests that RTLD_NOW on dlopen() will return NULL because call from libfoo to libbar could not be bound
    ///
    void* handle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_NOW);
    if ( handle != NULL ) {
        printf("[FAIL] dlopen-RTLD_NOW: dlopen(libfoo.dylib, RTLD_NOW) should have failed\n");
        return 0;
    }


#if __arm64e__
    // arm64e always uses chained binds which does not support lazy binding
    bool supportsLazyBinding = false;
#else
    // other architectures may or may not use lazy binding
    unsigned long sectSize = 0;
    bool supportsLazyBinding = (getsectiondata(&__dso_handle, "__DATA", "__la_symbol_ptr", &sectSize) != NULL);
  #if __ARM_ARCH_7K__
    // armv7 has two names for lazy pointers section
    if ( !supportsLazyBinding )
        supportsLazyBinding = (getsectiondata(&__dso_handle, "__DATA", "__lazy_symbol", &sectSize) != NULL);
  #endif
#endif

    ///
    /// This tests that RTLD_LAZY on dlopen() will succeed if libfoo.dylib
    ///
    handle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_LAZY);
    if ( supportsLazyBinding ) {
        if ( handle == NULL ) {
            printf("[FAIL] dlopen-RTLD_NOW: dlopen(libfoo.dylib, RTLD_LAZY) should have succeeded: %s\n", dlerror());
            return 0;
        }
    }
    else {
        if ( handle != NULL ) {
            printf("[FAIL] dlopen-RTLD_NOW: dlopen(libfoo.dylib, RTLD_LAZY) should have failed becuase a symbol was missing\n");
            return 0;
        }
    }


    printf("[PASS] dlopen-RTLD_NOW\n");
    return 0;
}


// BUILD:  $CC foo.c -dynamiclib  -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC main.c -DRUN_DIR="$RUN_DIR" $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/dlopen-RTLD_NOLOAD-basic.exe
// BUILD:  cd $BUILD_DIR && ln -s libfoo.dylib libfoo-sym.dylib

// RUN:  ./dlopen-RTLD_NOLOAD-basic.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>


int main()
{
    printf("[BEGIN] dlopen-RTLD_NOLOAD-basic\n");

    ///
    /// This tests that RTLD_NOLOAD finds existing dylib statically linked
    ///
    void* handle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_NOLOAD);
    if ( handle == NULL ) {
        printf("[FAIL] dlopen-RTLD_NOLOAD-basic: dlopen(libfoo.dylib, RTLD_NOLOAD) failed but it should have worked: %s\n", dlerror());
        return 0;
    }
    void* sym = dlsym(handle, "foo");
    if ( sym == NULL ) {
        printf("[FAIL] dlopen-RTLD_NOLOAD-basic: dlsym(handle, \"foo\") failed but it should have worked: %s\n", dlerror());
        return 0;
    }

    ///
    /// This tests that RTLD_NOLOAD verifies that non-existant dylib returns NULL
    ///
    void* handle2 = dlopen(RUN_DIR "/libfobbulate.dylib", RTLD_NOLOAD);
    if ( handle2 != NULL ) {
        printf("[FAIL] dlopen-RTLD_NOLOAD-basic: dlopen(libfobbulate.dylib, RTLD_NOLOAD) succeeded but it should have failed\n");
        return 0;
    }


    ///
    /// This tests that RTLD_NOLOAD finds symlink to existing dylib
    ///
    void* handle3 = dlopen(RUN_DIR "/libfoo-sym.dylib", RTLD_NOLOAD);
    if ( handle3 == NULL ) {
        printf("[FAIL] dlopen-RTLD_NOLOAD-basic: dlopen(libfoo-sym.dylib, RTLD_NOLOAD) failed but it should have worked: %s\n", dlerror());
        return 0;
    }


    ///
    /// This tests that RTLD_NOLOAD of something in the dyld cache that is not yet loaded returns NULL
    ///
    void* handle4 = dlopen("/usr/lib/libz.1.dylib", RTLD_NOLOAD);
    if ( handle4 != NULL ) {
        printf("[FAIL] dlopen-RTLD_NOLOAD-basic: dlopen(libz.dylib, RTLD_NOLOAD) worked but it should have failed\n");
        return 0;
    }


    printf("[PASS] dlopen-RTLD_NOLOAD-basic\n");
    return 0;
}

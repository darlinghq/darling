
// BUILD:  $CC foo.c -dynamiclib  -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC bar.c -dynamiclib  -install_name $RUN_DIR/libbar.dylib -o $BUILD_DIR/libbar.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-RTLD_NODELETE.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlopen-RTLD_NODELETE.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>


int main()
{
    printf("[BEGIN] dlopen-RTLD_NODELETE\n");

    ///
    /// This tests that RTLD_NODELETE on first dlopen() blocks dlclose() from unloading image
    ///
    void* handle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_NODELETE);
    if ( handle == NULL ) {
        printf("[FAIL] dlopen-RTLD_NODELETE: dlopen(libfoo.dylib, RTLD_NODELETE) failed but it should have worked: %s\n", dlerror());
        return 0;
    }
    int* fooSym = (int*)dlsym(handle, "foo");
    if ( fooSym == NULL ) {
        printf("[FAIL] dlopen-RTLD_NODELETE: dlsym(handle, \"foo\") failed but it should have worked: %s\n", dlerror());
        return 0;
    }
    int fooValue = *fooSym;
    dlclose(handle);
    Dl_info info;
    if ( dladdr(fooSym, &info) != 0 ) {
        printf("[FAIL] dlopen-RTLD_NODELETE: dladdr(fooSym, xx) succeeded as if libfoo.dylib was not unloaded\n");
        return 0;
    }
    // dereference foo pointer.  If RTLD_NODELETE worked, this will not crash
    if ( *fooSym != fooValue ) {
        printf("[FAIL] dlopen-RTLD_NODELETE: value at fooSym changed\n");
        return 0;
    }

    ///
    /// This tests that RTLD_NODELETE on later dlopen() blocks dlclose() from unloading image
    ///
    void* handle2 = dlopen(RUN_DIR "/libbar.dylib", RTLD_GLOBAL);
    if ( handle2 == NULL ) {
        printf("[FAIL] dlopen-RTLD_NODELETE: dlopen(libfoo.dylib, RTLD_GLOBAL) failed but it should have worked: %s\n", dlerror());
        return 0;
    }
    int* barSym = (int*)dlsym(handle2, "bar");
    if ( barSym == NULL ) {
        printf("[FAIL] dlopen-RTLD_NODELETE: dlsym(handle, \"bar\") failed but it should have worked: %s\n", dlerror());
        return 0;
    }
    int barValue = *barSym;
    void* handle3 = dlopen(RUN_DIR "/libbar.dylib", RTLD_NODELETE);
    if ( handle3 == NULL ) {
        printf("[FAIL] dlopen-RTLD_NODELETE: dlopen(libfoo.dylib, RTLD_NODELETE) failed but it should have worked: %s\n", dlerror());
        return 0;
    }
    dlclose(handle2);
    dlclose(handle3);
    if ( dladdr(barSym, &info) != 0 ) {
        printf("[FAIL] dlopen-RTLD_NODELETE: dladdr(barSym, xx) succeeded as if libbar.dylib was not unloaded\n");
        return 0;
    }
    // dereference foo pointer.  If RTLD_NODELETE worked, this will not crash
    if ( *barSym != barValue ) {
        printf("[FAIL] dlopen-RTLD_NODELETE: value at barSym changed\n");
        return 0;
    }


    printf("[PASS] dlopen-RTLD_NODELETE\n");
    return 0;
}

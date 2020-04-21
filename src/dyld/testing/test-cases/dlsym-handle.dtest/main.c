
// BUILD:  $CC base.c -dynamiclib  -install_name $RUN_DIR/libbase.dylib  -o $BUILD_DIR/libbase.dylib
// BUILD:  $CC foo.c  -dynamiclib $BUILD_DIR/libbase.dylib -install_name $RUN_DIR/libfoo.dylib  -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC bar.c  -dynamiclib $BUILD_DIR/libbase.dylib -install_name $RUN_DIR/libbar.dylib  -o $BUILD_DIR/libbar.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlsym-handle.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlsym-handle.exe

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld_priv.h>


// verify RTLD_DEFAULT search order

int mainSymbol = 4;

int main()
{
    printf("[BEGIN] dlsym-handle\n");

    void* fooHandle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_LAZY);
    if ( fooHandle == NULL ) {
        printf("[FAIL]  dlsym-handle: libfoo.dylib could not be loaded, %s\n", dlerror());
        return 0;
    }

    void* barHandle = dlopen(RUN_DIR "/libbar.dylib", RTLD_LAZY);
    if ( barHandle == NULL ) {
        printf("[FAIL]  dlsym-handle: libbar.dylib could not be loaded, %s\n", dlerror());
        return 0;
    }

    // verify fooHandle does not find mainSymbol
    if ( dlsym(fooHandle, "mainSymbol") != NULL ) {
        printf("[FAIL]  dlsym-handle: mainSymbol was found with fooHandle\n");
        return 0;
    }

    // verify fooHandle can find foo
    if ( dlsym(fooHandle, "foo") == NULL ) {
        printf("[FAIL]  dlsym-handle: foo not found with fooHandle\n");
        return 0;
    }

    // verify fooHandle can find base
    if ( dlsym(fooHandle, "base") == NULL ) {
        printf("[FAIL]  dlsym-handle: base not found with fooHandle\n");
        return 0;
    }

    // verify fooHandle cannot find bar
    if ( dlsym(fooHandle, "bar") != NULL ) {
        printf("[FAIL]  dlsym-handle: bar found with fooHandle\n");
        return 0;
    }

    // verify barHandle can find bar
    if ( dlsym(barHandle, "bar") == NULL ) {
        printf("[FAIL]  dlsym-handle: bar not found with barHandle\n");
        return 0;
    }

    // verify barHandle can find base
    if ( dlsym(barHandle, "base") == NULL ) {
        printf("[FAIL]  dlsym-handle: base not found with barHandle\n");
        return 0;
    }

    // verify barHandle cannot find foo
    if ( dlsym(barHandle, "foo") != NULL ) {
        printf("[FAIL]  dlsym-handle: foo found with barHandle\n");
        return 0;
    }

    // verify renamed and re-exported symbols work
    if ( dlsym(RTLD_DEFAULT, "strcmp") == NULL ) {
        printf("[FAIL]  dlsym-handle: strcmp not found\n");
        return 0;
    }
    
    // verify bad handle errors
    if ( dlsym((void*)0xdeadbeef, "malloc") != NULL ) {
        printf("[FAIL]  dlsym-handle: malloc found with bad handle\n");
        return 0;
    }
    else {
        const char* message = dlerror();
        if ( strstr(message, "invalid") == NULL ) {
            printf("[FAIL]  dlsym-handle: invalid handle error message missing 'invalid'\n");
            return 0;
        }
    }

    printf("[PASS]  dlsym-handle\n");
	return 0;
}


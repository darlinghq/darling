
// BUILD:  $CC base.c  -dynamiclib -install_name $RUN_DIR/libbase.dylib  -o $BUILD_DIR/libbase.dylib
// BUILD:  $CC foo.c   -dynamiclib -install_name $RUN_DIR/libdynamic.dylib  -o $BUILD_DIR/libdynamic.dylib $BUILD_DIR/libbase.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/static-terminators.exe -DRUN_DIR="$RUN_DIR" $BUILD_DIR/libbase.dylib

// RUN:  ./static-terminators.exe

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld_priv.h>


// verify all static terminators run in proper order


extern void mainTerminated();

static __attribute__((destructor))
void myTerm()
{
    //fprintf(stderr, "main's static terminator\n");
    mainTerminated();
}


int main()
{
    printf("[BEGIN] static-terminators\n");

    // load dylib
    void* handle = dlopen(RUN_DIR "/libdynamic.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("[FAIL]  dlclose-static-terminator: libdynamic.dylib could not be loaded, %s\n", dlerror());
        return 0;
    }

    // PASS is printed in libbase.dylib terminator

	return 0;
}


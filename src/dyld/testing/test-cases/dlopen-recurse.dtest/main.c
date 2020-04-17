
// BUILD:  $CC bar.c -dynamiclib  -install_name $RUN_DIR/libbar.dylib -o $BUILD_DIR/libbar.dylib
// BUILD:  $CC foo.c -dynamiclib  -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib -DRUN_DIR="$RUN_DIR"
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-recurse.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlopen-recurse.exe

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>



int main()
{
    printf("[BEGIN] dlopen-recurse\n");

    // libfoo's initializer calls dlopen().  If that hangs, we have a locking bug
    void* handle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_LAZY);
    dlclose(handle);

    printf("[PASS] dlopen-recurse\n");
	return 0;
}


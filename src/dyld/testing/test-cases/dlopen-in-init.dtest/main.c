

// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-in-init.exe $BUILD_DIR/libfoo.dylib

// RUN:  ./dlopen-in-init.exe

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>


__attribute__((constructor))
void myinit()
{

}


int main()
{
    printf("[BEGIN] dlopen-in-init\n");

    // The test is for libdyld.dylib to not crash when libfoo.dylib dlopen() stuff in its initializer

    printf("[PASS]  dlopen-in-init\n");
	return 0;
}


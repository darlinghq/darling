
// This tests that our header such as dlfcn.h pass unix conformance.

// BUILD_ONLY: MacOSX

// BUILD:  $CC main.c -o $BUILD_DIR/unix-conformance.exe -D_XOPEN_SOURCE=600
// BUILD:  $CC main.c -o $TEMP_DIR/scratch.exe -D_XOPEN_SOURCE=600 -D_POSIX_C_SOURCE=200112

// RUN:  ./unix-conformance.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <dlfcn.h> 

int main()
{
    printf("[BEGIN] unix-conformance.dtest\n");

    printf("[PASS] unix-conformance.dtest\n");
    return 0;
}


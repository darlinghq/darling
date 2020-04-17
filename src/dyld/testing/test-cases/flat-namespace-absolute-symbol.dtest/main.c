// BUILD_ONLY: MacOSX

// BUILD:  $CC foo.s -dynamiclib -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CC main.c $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/flat-namespace.exe -flat_namespace

// RUN:    ./flat-namespace.exe



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int myAbs1;
int* ptr = &myAbs1;

int main()
{
    printf("[BEGIN] flat-namespace-absolute-symbol\n");

    if ( ptr != 0 ) {
        printf("[FAIL] absolute symbol not bound to zero with flat lookup\n");
        return 0;
    }

    printf("[PASS] flat-namespace-absolute-symbol\n");
	return 0;
}

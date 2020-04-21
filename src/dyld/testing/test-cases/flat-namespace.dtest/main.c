// BUILD_ONLY: MacOSX

// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CC main.c $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/flat-namespace.exe -flat_namespace

// RUN:    ./flat-namespace.exe



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    printf("[BEGIN] flat-namespace\n");

    // check that the malloc in libfoo.dylib was used by looking at the content the allocated buffer
    // <rdar://problem/31921090> strncmp is tricky for flat namespace because it is re-exporte and renamed
    char* p1 = malloc(10);
    if ( strncmp(p1, "##########", 10) != 0 ) {
        printf("[FAIL] malloc() from main executable not interposed\n");
        return 0;
    }

    printf("[PASS] flat-namespace\n");
	return 0;
}

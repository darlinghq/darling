

// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CC present.c $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/dylib-static-present.exe
// BUILD:  $CC foo.c -dynamiclib -o $TEMP_DIR/libfoo2.dylib -install_name $RUN_DIR/libfoomissing.dylib
// BUILD:  $CC missing.c $TEMP_DIR/libfoo2.dylib -o $BUILD_DIR/dylib-static-missing.exe

// RUN:  ./dylib-static-present.exe
// RUN:  NOCR_TEST_NAME="dylib-static-link missing" $REQUIRE_CRASH ./dylib-static-missing.exe


#include <stdio.h>

extern int foo;


int main()
{
    printf("[BEGIN] dylib-static-link present\n");
    if ( foo == 42 )
        printf("[PASS] dylib-static-link present\n");
    else
        printf("[FAIL] dylib-static-link present, wrong value\n");

	return 0;
}





// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/libbar.dylib -install_name $RUN_DIR/libbar.dylib
// BUILD:  $CC foo.c -dynamiclib $BUILD_DIR/libbar.dylib -sub_library libbar -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dylib-re-export.exe $BUILD_DIR/libfoo.dylib -L$BUILD_DIR

// RUN:  ./dylib-re-export.exe


#include <stdio.h>

extern int bar();


int main()
{
    printf("[BEGIN] dylib-re-export\n");
    if ( bar() == 42 )
        printf("[PASS] dylib-re-export\n");
    else
        printf("[FAIL] dylib-re-export, wrong value\n");

	return 0;
}





// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libfoo.dylib -install_name libfoo.dylib
// BUILD:  $CC main.c $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/cwd-load.exe
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/cwd-load.exe

// RUN:  ./cwd-load.exe

// libfoo.dylib is loaded from the current directory (not an absolute path)


#include <stdio.h>

extern int foo;


int main()
{
    printf("[BEGIN] cwd-relative-load\n");
    if ( foo == 42 )
        printf("[PASS] cwd-relative-load\n");
    else
        printf("[FAIL] cwd-relative-load, wrong value\n");

	return 0;
}



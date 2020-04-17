// BUILD_ONLY:      MacOSX
// BUILD_MIN_OS:    10.5
// BUILD:           $CC bar.c -dynamiclib -install_name $RUN_DIR/libbar.dylib -o $BUILD_DIR/libbar.dylib -nostdlib -ldylib1.o
// BUILD:           $CC foo.c -dynamiclib $BUILD_DIR/libbar.dylib -sub_library libbar -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib -nostdlib -ldylib1.o
// BUILD:           $CC main.c -o $BUILD_DIR/dylib-re-export.exe $BUILD_DIR/libfoo.dylib -L$BUILD_DIR -nostdlib -lSystem -lcrt1.10.5.o

// RUN:  ./dylib-re-export.exe


#include <stdio.h>

extern int bar();


int main()
{
    printf("[BEGIN] dylib-re-export-old-format\n");
    if ( bar() == 42 )
        printf("[PASS] dylib-re-export-old-format\n");
    else
        printf("[FAIL] dylib-re-export-old-format, wrong value\n");

	return 0;
}



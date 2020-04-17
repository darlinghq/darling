// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CC present.c $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/dylib-static-weak-present.exe
// BUILD:  $CC foo.c -dynamiclib -o $TEMP_DIR/libfoo2.dylib -install_name $RUN_DIR/libfoomissing.dylib
// BUILD:  $CC missing.c $TEMP_DIR/libfoo2.dylib -o $BUILD_DIR/dylib-static-weak-missing.exe

// RUN:  ./dylib-static-weak-present.exe
// RUN:  ./dylib-static-weak-missing.exe


#include <stddef.h>
#include <stdio.h>

extern int foo __attribute__((weak_import));


int main()
{
    printf("[BEGIN] dylib-static-weak-link present\n");
    // dylib will be found at runtime, so &foo should never be NULL
    if ( &foo != NULL ) {
       if ( foo == 42 )
            printf("[PASS] dylib-static-weak-link present\n");
        else
            printf("[FAIL] dylib-static-weak-link present, wrong value\n");
    }
    else {
        printf("[FAIL] dylib-static-weak-link present, &foo == NULL\n");
    }

	return 0;
}



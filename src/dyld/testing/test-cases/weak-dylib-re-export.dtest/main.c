

// BUILD:  $CC bar.c -dynamiclib -o $TEMP_DIR/libbar.dylib -install_name $RUN_DIR/libbar.dylib
// BUILD:  $CC foo.c -dynamiclib -L$TEMP_DIR -weak-lbar -Wl,-reexported_symbols_list,symbols.txt -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dylib-re-export.exe $BUILD_DIR/libfoo.dylib -L$BUILD_DIR

// RUN:  ./dylib-re-export.exe


#include <stdio.h>

__attribute__((weak_import))
extern int bar();

int main()
{
    printf("[BEGIN] dylib-re-export\n");
    if ( &bar == 0 )
        printf("[PASS] dylib-re-export\n");
    else
        printf("[FAIL] dylib-re-export, wrong value\n");

	return 0;
}



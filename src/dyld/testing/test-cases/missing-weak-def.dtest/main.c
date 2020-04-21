
// BUILD:  $CC bar.c -dynamiclib -install_name $RUN_DIR/libbar.dylib -o $TEMP_DIR/libbar.dylib
// BUILD:  $CC bar-empty.c -dynamiclib -install_name $BUILD_DIR/libbar.dylib -o $BUILD_DIR/libbar.dylib
// BUILD:  $CC main.c $TEMP_DIR/libbar.dylib -o $BUILD_DIR/missing-weak-def.exe

// RUN:  ./missing-weak-def.exe

// bar is a weak_import weak bind and the libbar we have at runtime is missing that symbol


#include <stdio.h>
#include <stdlib.h>

__attribute__((weak))
__attribute__((weak_import))
int bar();

int main()
{
    printf("[BEGIN] missing-weak-def\n");

    if (&bar) {
    	printf("[FAIL] missing-weak-def\n");
    	return 0;
    }

    printf("[PASS] missing-weak-def\n");
	return 0;
}


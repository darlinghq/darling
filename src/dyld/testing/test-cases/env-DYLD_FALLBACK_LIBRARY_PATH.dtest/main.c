
// BUILD:  mkdir -p $BUILD_DIR/fallback
// BUILD:  $CC foo.c -dynamiclib -o $TEMP_DIR/libfoo.dylib           -install_name $RUN_DIR/libfoo.dylib -DVALUE=1
// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/fallback/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib -DVALUE=42
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_FALLBACK_LIBRARY_PATH.exe $TEMP_DIR/libfoo.dylib
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/env-DYLD_FALLBACK_LIBRARY_PATH.exe

// RUN:  DYLD_FALLBACK_LIBRARY_PATH=$RUN_DIR/fallback/ ./env-DYLD_FALLBACK_LIBRARY_PATH.exe

#include <stdio.h>

extern int foo();

int main()
{
     printf("[BEGIN] env-DYLD_FALLBACK_LIBRARY_PATH\n");

	if ( foo() == 42 )
        printf("[PASS] env-DYLD_FALLBACK_LIBRARY_PATH\n");
    else
        printf("[FAIL] env-DYLD_FALLBACK_LIBRARY_PATH\n");

	return 0;
}


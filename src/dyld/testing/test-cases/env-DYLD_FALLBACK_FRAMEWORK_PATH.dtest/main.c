
// BUILD:  mkdir -p $TEMP_DIR/Foo.framework $BUILD_DIR/FallbackFrameworks/Foo.framework
// BUILD:  $CC foo.c -dynamiclib -o $TEMP_DIR/Foo.framework/Foo                     -install_name $RUN_DIR/Foo.framework/Foo -DVALUE=1
// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/FallbackFrameworks/Foo.framework/Foo -install_name $RUN_DIR/Foo.framework/Foo -DVALUE=42
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_FALLBACK_FRAMEWORK_PATH.exe $TEMP_DIR/Foo.framework/Foo
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/env-DYLD_FALLBACK_FRAMEWORK_PATH.exe

// RUN:  DYLD_FALLBACK_FRAMEWORK_PATH=$RUN_DIR/FallbackFrameworks/ ./env-DYLD_FALLBACK_FRAMEWORK_PATH.exe

#include <stdio.h>
#include <stdlib.h>

extern int foo();

int main()
{
    printf("[BEGIN] env-DYLD_FALLBACK_FRAMEWORK_PATH\n");

	if ( foo() == 42 )
        printf("[PASS] env-DYLD_FALLBACK_FRAMEWORK_PATH\n");
    else
        printf("[FAIL] env-DYLD_FALLBACK_FRAMEWORK_PATH\n");

	return 0;
}


// BUILD:  $CC main.c -DRUN_DIR=$RUN_DIR -o $BUILD_DIR/insert_target.exe
// BUILD:  $CC foo.c -dynamiclib -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC interposer.c -dynamiclib $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libinterposer.dylib -o $BUILD_DIR/libinterposer.dylib
// BUILD:  $CC main.c $BUILD_DIR/libinterposer.dylib $BUILD_DIR/libfoo.dylib -DRUN_DIR=$RUN_DIR -o $BUILD_DIR/interpose_target.exe
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/insert_target.exe

// RUN:  ./insert_target.exe
// RUN:  DYLD_INSERT_LIBRARIES=$RUN_DIR/libfoo.dylib  ./insert_target.exe true
// RUN:  ./interpose_target.exe true

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <mach-o/dyld_priv.h>

int main(int argc, const char* argv[])
{
    printf("[BEGIN] dyld_has_inserted_or_interposing_libraries\n");

    bool actual = dyld_has_inserted_or_interposing_libraries();
    bool expected = (argc == 2) && (strcmp(argv[1], "true") == 0);

    const char* result = (actual == expected) ? "PASS" : "FAIL";
    printf("[%s]  dyld_has_inserted_or_interposing_libraries\n", result);

	return 0;
}


// BUILD:  $CC base.c -dynamiclib -install_name $RUN_DIR/libbase.dylib -o $BUILD_DIR/libbase.dylib
// BUILD:  $CC foo3.c -dynamiclib -install_name $RUN_DIR/libfoo3.dylib -o $BUILD_DIR/libfoo3.dylib $BUILD_DIR/libbase.dylib
// BUILD:  $CC foo2.c -dynamiclib -install_name $RUN_DIR/libfoo2.dylib -o $BUILD_DIR/libfoo2.dylib $BUILD_DIR/libfoo3.dylib $BUILD_DIR/libbase.dylib
// BUILD:  $CC foo1.c -dynamiclib -install_name $RUN_DIR/libfoo1.dylib -o $BUILD_DIR/libfoo1.dylib $BUILD_DIR/libfoo2.dylib $BUILD_DIR/libbase.dylib
// BUILD:  $CC main.c  $BUILD_DIR/libfoo1.dylib $BUILD_DIR/libbase.dylib -o $BUILD_DIR/weak-coalesce.exe

// RUN:  ./weak-coalesce.exe


#include <stdio.h>
#include <stdlib.h>

#include "base.h"

int main()
{
    printf("[BEGIN] weak-coalesce\n");

	baseVerifyCoal1("in main", &coal1);
	baseVerifyCoal2("in main", &coal2);

	baseCheck();
	return 0;
}


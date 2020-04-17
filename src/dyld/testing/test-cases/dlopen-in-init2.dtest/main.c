

// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/libbar.dylib -install_name $RUN_DIR/libbar.dylib
// BUILD:  $CC baz.c -dynamiclib -o $BUILD_DIR/libbaz.dylib -install_name $RUN_DIR/libbaz.dylib
// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib $BUILD_DIR/libbaz.dylib -DRUN_DIR="$RUN_DIR"
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-in-init2.exe $BUILD_DIR/libfoo.dylib $BUILD_DIR/libbar.dylib

// RUN:  ./dlopen-in-init2.exe

// This test uses dlopen to jump ahead in the initializer graph
// The static linkages here should result in initializers being run in the order libbaz, libfoo, libbar
// However, a dlopen of libbar inside libfoo's static initializer means we need to skip ahead and initialize libbar to satisfy that dlopen
// This means that the closure needs to have "initializer-order" on libfoo and not just the top level executable image.
// It also means that dlopen needs to check we have actually initialized libbar instead of just bumping its ref-count.

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>


extern int foo();
extern int bar();

int main() {
	printf("[BEGIN] dlopen-in-init2\n");
	if ( foo() != 0 )
		return 0;
	if ( bar() != 0 )
		return 0;
	printf("[PASS]  dlopen-in-init2\n");
	return 0;
}


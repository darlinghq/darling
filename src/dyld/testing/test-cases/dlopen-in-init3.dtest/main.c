

// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/libbar.dylib -install_name $RUN_DIR/libbar.dylib -DRUN_DIR="$RUN_DIR"
// BUILD:  $CC baz.c -dynamiclib -o $BUILD_DIR/libbaz.dylib -install_name $RUN_DIR/libbaz.dylib
// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib $BUILD_DIR/libbar.dylib $BUILD_DIR/libbaz.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-in-init3.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./dlopen-in-init3.exe

// This test uses dlopen to jump ahead in the initializer graph
// main doesn't directly link any of the libraries here, but dlopen's libfoo which links libbar and libbar.
// We should run initializers in the order libbar, libbaz, libfoo.
// However, libbar has a static init with a dlopen of libbaz and so libbaz needs to be initialized by libbar instead of by libfoo

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>

int main() {
	printf("[BEGIN] dlopen-in-init3\n");
	void* fooHandle = dlopen(RUN_DIR "/libfoo.dylib", 0);
	if ( fooHandle == NULL ) {
		printf("[FAIL]  dlopen-in-init3, dlopen libfoo.dylib: %s\n", dlerror());
		return 0;
	}
	void* fooSymbol = dlsym(RTLD_DEFAULT, "foo");
    if ( fooSymbol == NULL ) {
		printf("[FAIL]  dlopen-in-init3, dlsym libfoo.dylib\n");
        return 0;
    }
	if ( ((int(*)())fooSymbol)() != 0 )
		return 0;
	printf("[PASS]  dlopen-in-init3\n");
	return 0;
}


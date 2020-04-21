
// BOOT_ARGS: dyld_flags=2

// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/librpathabs.dylib -install_name @rpath/librpathabs.dylib
// BUILD:  $CC foo.c -dynamiclib $BUILD_DIR/librpathabs.dylib -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib -rpath $RUN_DIR
// BUILD:  $CC main.c $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/rpath-absolute.exe

// RUN:  DYLD_AMFI_FAKE=0 ./rpath-absolute.exe

// main prog links with libfoo.dylib which uses an absolute LC_RPRATH and @rpath to find librpathabs.dylib
// must be able to run with full restrictions

#include <stdio.h>

extern int foo;


int main()
{
    printf("[BEGIN] rpath-absolute.exe\n");
    printf("[PASS]  rpath-absolute.exe\n");
	return 0;
}



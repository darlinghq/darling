
// BOOT_ARGS: dyld_flags=2

// BUILD:  $CC main.cpp -std=c++11 -o $BUILD_DIR/double_insert_main.exe
// BUILD:  $CC foo.c -dynamiclib -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC bar.c -dynamiclib -install_name $RUN_DIR/libbar.dylib -o $BUILD_DIR/libbar.dylib
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/double_insert_main.exe

// Load foo and bar at the same time to ensure that paths separated by ':'' are working

// RUN:  DYLD_INSERT_LIBRARIES="$RUN_DIR/libfoo.dylib:$RUN_DIR/libbar.dylib" ./double_insert_main.exe

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <mach-o/dyld_priv.h>

bool gFoundLibrary = false;
const char* gLibraryName = NULL;

bool wasImageLoaded(const char* libraryName) {
	gFoundLibrary = false;
	gLibraryName = libraryName;
	_dyld_register_for_image_loads([](const mach_header* mh, const char* path, bool unloadable) {
		if ( strstr(path, gLibraryName) != NULL ) {
			gFoundLibrary = true;
		}
	});
    if (!gFoundLibrary)
        printf("[FAIL] dyld-insert-library-double: expected insert to pass for '%s'\n", libraryName);
	return gFoundLibrary;
}

int main()
{
    printf("[BEGIN] dyld-insert-library-double\n");

    if (!wasImageLoaded("libfoo.dylib") || !wasImageLoaded("libbar.dylib")) {
        return 0;
    }

    printf("[PASS]  dyld-insert-library-double\n");

	return 0;
}

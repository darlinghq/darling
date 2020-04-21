
// BUILD:  mkdir -p $BUILD_DIR/dir1 $BUILD_DIR/dir2
// BUILD:  $CC sub1.c -dynamiclib -install_name @rpath/librpathstatic.dylib -o $BUILD_DIR/dir1/librpathstatic.dylib
// BUILD:  $CC sub2.c -dynamiclib -install_name @rpath/libdynamic.dylib     -o $BUILD_DIR/dir2/libdynamic.dylib $BUILD_DIR/dir1/librpathstatic.dylib
// BUILD:  $CC foo.c  -dynamiclib -install_name $RUN_DIR/libstatic.dylib    -o $BUILD_DIR/libstatic.dylib -rpath @loader_path/dir1 $BUILD_DIR/dir1/librpathstatic.dylib
// BUILD:  $CC main.c $BUILD_DIR/libstatic.dylib -DRUN_DIR="$RUN_DIR"       -o $BUILD_DIR/dlopen-rpath-prev.exe

// RUN:  ./dlopen-rpath-prev.exe

// main links with libstatic.dylib which uses rpath to link with dir1/librpathstatic.dylib
// main dlopen()s libdynamic.dylib which links with dir1/librpathstatic.dylib, but has no rpath for it and depends on it being already loaded

#include <stdio.h>
#include <dlfcn.h>

int main()
{
    printf("[BEGIN] dlopen-rpath-prev\n");

	void* handle = dlopen(RUN_DIR "/dir2/libdynamic.dylib", RTLD_LAZY);
	if ( handle == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL]  dlopen-rpath-prev\n");
		return 0;
	}

    printf("[PASS]  dlopen-rpath-prev\n");
	return 0;
}


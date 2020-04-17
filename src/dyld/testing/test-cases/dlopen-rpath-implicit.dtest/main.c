
// BUILD:  mkdir -p $BUILD_DIR/dir1
// BUILD:  $CC foo.c -dynamiclib -install_name @rpath/libimplicitrpath.dylib -o $BUILD_DIR/dir1/libimplicitrpath.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-rpath-implicit.exe -rpath @loader_path/dir1

// RUN:  ./dlopen-rpath-implicit.exe

#include <stdio.h>
#include <dlfcn.h>


/// test that a leaf name passed to dlopen() searches the rpath

int main()
{
    printf("[BEGIN] dlopen-rpath-implicit\n");

    void* handle = dlopen("libimplicitrpath.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("[FAIL] dlopen-rpath-implicit: dlopen(libimplicitrpath.dylib) failed: %s\n", dlerror());
        return 0;
    }

    dlclose(handle);

    printf("[PASS] dlopen-rpath-implicit\n");

	return 0;
}


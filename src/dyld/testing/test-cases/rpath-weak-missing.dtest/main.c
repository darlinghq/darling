
// BOOT_ARGS: dyld_flags=2

// BUILD:  $CC foo.c -dynamiclib -o $TEMP_DIR/libmissing.dylib -install_name @rpath/libmissing.dylib
// BUILD:  $CC foo.c -dynamiclib -Wl,-weak_library,$TEMP_DIR/libmissing.dylib -o $BUILD_DIR/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib -rpath $RUN_DIR
// BUILD:  $CC main.c -o $BUILD_DIR/rpath-weak-missing.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./rpath-weak-missing.exe
// RUN:  DYLD_AMFI_FAKE=0 ./rpath-weak-missing.exe

// main prog dlopen()s libfoo.dylib which weak links to @rpath/libmissing.dylib

#include <stdio.h>
#include <dlfcn.h>


int main()
{
    printf("[BEGIN] rpath-weak-missing\n");

    void* handle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("[FAIL]  rpath-weak-missing dlopen(\"%s/libfoo.dylib\") - %s\n", RUN_DIR, dlerror());
        return 0;
    }

    printf("[PASS]  rpath-weak-missing\n");
	return 0;
}



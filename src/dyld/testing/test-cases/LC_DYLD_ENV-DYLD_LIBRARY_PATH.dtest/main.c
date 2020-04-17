
// BUILD:  mkdir -p $BUILD_DIR/hideyhole
// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/hideyhole/libfoo1.dylib -install_name /bad/path/libfoo1.dylib
// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/hideyhole/libfoo2.dylib -install_name /bad/path2/libfoo2.dylib
// BUILD:  $CC main.c            -o $BUILD_DIR/LC_DYLD_ENV-DYLD_LIBRARY_PATH-main1.exe $BUILD_DIR/hideyhole/libfoo1.dylib -Wl,-dyld_env,DYLD_LIBRARY_PATH=@executable_path/hideyhole
// BUILD:  $CC main.c            -o $BUILD_DIR/LC_DYLD_ENV-DYLD_LIBRARY_PATH-main2.exe $BUILD_DIR/hideyhole/libfoo1.dylib -Wl,-dyld_env,DYLD_LIBRARY_PATH=@loader_path/hideyhole
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/LC_DYLD_ENV-DYLD_LIBRARY_PATH-main1.exe
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/LC_DYLD_ENV-DYLD_LIBRARY_PATH-main2.exe

// RUN:  ./LC_DYLD_ENV-DYLD_LIBRARY_PATH-main1.exe
// RUN:  ./LC_DYLD_ENV-DYLD_LIBRARY_PATH-main2.exe

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

/// Test that main executable's LC_DYLD_ENVIRONMENT can set DYLD_LIBRARY_PATH with @executable_path or @loader_path relative paths

extern char* __progname;

int main()
{
    printf("[BEGIN] LC_DYLD_ENV-DYLD_LIBRARY_PATH %s\n", __progname);

    void*h = dlopen("/other/path/libfoo2.dylib", 0);

	if ( h != NULL )
        printf("[PASS] LC_DYLD_ENV-DYLD_LIBRARY_PATH %s\n", __progname);
    else
        printf("[FAIL] LC_DYLD_ENV-DYLD_LIBRARY_PATH %s\n", __progname);

	return 0;
}


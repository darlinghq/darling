
// BUILD:  mkdir -p $BUILD_DIR/Bar.framework
// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libfoo.dylib            -install_name $RUN_DIR/libfoo.dylib -DVALUE=1
// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/libfoo_other.dylib      -install_name $RUN_DIR/libfoo.dylib -DVALUE=42
// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/Bar.framework/Bar       -install_name $RUN_DIR/Bar.framework/Bar  -DVALUE=1
// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/Bar.framework/Bar_alt   -install_name $RUN_DIR/Bar.framework/Bar  -DVALUE=42
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_IMAGE_SUFFIX.exe $BUILD_DIR/libfoo.dylib $BUILD_DIR/Bar.framework/Bar
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/env-DYLD_IMAGE_SUFFIX.exe
// BUILD:  $CC main.c            -o $BUILD_DIR/env-DYLD_IMAGE_SUFFIX-dynamic.exe -DRUN_DIR="$RUN_DIR"
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/env-DYLD_IMAGE_SUFFIX-dynamic.exe

// RUN:                                 ./env-DYLD_IMAGE_SUFFIX.exe
// RUN:  DYLD_IMAGE_SUFFIX=_other       ./env-DYLD_IMAGE_SUFFIX.exe
// RUN:  DYLD_IMAGE_SUFFIX=_alt         ./env-DYLD_IMAGE_SUFFIX.exe
// RUN:  DYLD_IMAGE_SUFFIX=_alt:_other  ./env-DYLD_IMAGE_SUFFIX.exe
// RUN:                                 ./env-DYLD_IMAGE_SUFFIX-dynamic.exe
// RUN:  DYLD_IMAGE_SUFFIX=_other       ./env-DYLD_IMAGE_SUFFIX-dynamic.exe
// RUN:  DYLD_IMAGE_SUFFIX=_alt         ./env-DYLD_IMAGE_SUFFIX-dynamic.exe
// RUN:  DYLD_IMAGE_SUFFIX=_alt:_other  ./env-DYLD_IMAGE_SUFFIX-dynamic.exe




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

extern int foo();
extern int bar();

typedef int (*IntProc)();

int main()
{
    const char* suffix = getenv("DYLD_IMAGE_SUFFIX");
    if ( suffix == NULL )
        suffix = "";
    printf("[BEGIN] env-DYLD_IMAGE_SUFFIX-%s\n", suffix);

    const int expectedFoo = (strstr(suffix, "_other") != NULL) ? 42 : 1;
    const int expectedBar = (strstr(suffix, "_alt") != NULL) ? 42 : 1;;

#ifdef RUN_DIR
    void* fooHandle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_LAZY);
    if ( fooHandle == NULL ) {
        printf("[FAIL]  env-DYLD_IMAGE_SUFFIX-%s, libfoo.dylib could not be loaded, %s\n", suffix, dlerror());
        return 0;
    }
    void* barHandle = dlopen(RUN_DIR "/Bar.framework/Bar", RTLD_LAZY);
    if ( barHandle == NULL ) {
        printf("[FAIL]  env-DYLD_IMAGE_SUFFIX-%s, Bar.framework/Bar could not be loaded, %s\n", suffix, dlerror());
        return 0;
    }
    IntProc fooProc = (IntProc)dlsym(fooHandle, "foo");
    if ( fooProc == NULL ) {
        printf("[FAIL]  env-DYLD_IMAGE_SUFFIX-%s, symbol 'foo' not found %s\n", suffix, dlerror());
        return 0;
    }
    IntProc barProc = (IntProc)dlsym(barHandle, "bar");
    if ( barProc == NULL ) {
        printf("[FAIL]  env-DYLD_IMAGE_SUFFIX-%s, symbol 'bar' not found %s\n", suffix, dlerror());
        return 0;
    }
    int fooValue = (*fooProc)();
    int barValue = (*barProc)();
#else
    int fooValue = foo();
    int barValue = bar();
#endif
	if ( fooValue != expectedFoo )
        printf("[FAIL]  env-DYLD_IMAGE_SUFFIX-%s, foo()=%d expected=%d\n", suffix, fooValue, expectedFoo);
    else if ( barValue != expectedBar )
        printf("[FAIL]  env-DYLD_IMAGE_SUFFIX-%s, bar()=%d expected=%d\n", suffix, barValue, expectedBar);
    else
        printf("[PASS]  env-DYLD_IMAGE_SUFFIX-%s\n", suffix);

	return 0;
}


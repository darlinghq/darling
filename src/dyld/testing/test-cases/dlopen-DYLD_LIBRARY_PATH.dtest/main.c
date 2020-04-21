
// BUILD:  mkdir -p $BUILD_DIR/door1 $BUILD_DIR/door2
// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/door1/libbar.dylib -install_name $RUN_DIR/libbar.dylib -DVALUE=3
// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/door2/libbar.dylib -install_name $RUN_DIR/libbar.dylib -DVALUE=17
// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/door1/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib -DVALUE=10  $BUILD_DIR/door1/libbar.dylib
// BUILD:  $CC foo.c -dynamiclib -o $BUILD_DIR/door2/libfoo.dylib -install_name $RUN_DIR/libfoo.dylib -DVALUE=25  $BUILD_DIR/door2/libbar.dylib
// BUILD:  $CC main.c            -o $BUILD_DIR/dlopen-DYLD_LIBRARY_PATH.exe 
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/dlopen-DYLD_LIBRARY_PATH.exe

// RUN:  DYLD_LIBRARY_PATH=$RUN_DIR/door1/                  ./dlopen-DYLD_LIBRARY_PATH.exe  13
// RUN:  DYLD_LIBRARY_PATH=$RUN_DIR/door2                   ./dlopen-DYLD_LIBRARY_PATH.exe  42
// RUN:  DYLD_LIBRARY_PATH=$RUN_DIR/door3/:$RUN_DIR/door2/  ./dlopen-DYLD_LIBRARY_PATH.exe  42

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>

// Program dlopen()s libfoo.dylib which was linked against libbar.dylib
// Neither have valid paths and must be found via DYLD_LIBRARY_PATH
// This test direct and indirect loading.

int main(int argc, const char* argv[])
{
    const char* env = getenv("DYLD_LIBRARY_PATH");
    if ( env == NULL ) {
        printf("[BEGIN] dlopen-DYLD_LIBRARY_PATH\n");
        printf("[FAIL] dlopen-DYLD_LIBRARY_PATH, env not set\n");
		return 0;
    }
    const char* valueStr = argv[1];
    if ( valueStr == NULL ) {
        printf("[BEGIN] dlopen-DYLD_LIBRARY_PATH\n");
        printf("[FAIL] dlopen-DYLD_LIBRARY_PATH, arg1 value not set\n");
		return 0;
    }
    char* end;
    long value = strtol(valueStr, &end, 0);

    printf("[BEGIN] dlopen-DYLD_LIBRARY_PATH %s\n", env);

	void* handle = dlopen("/bogus/libfoo.dylib", RTLD_LAZY);
	if ( handle == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL] dlopen-DYLD_LIBRARY_PATH %s\n", env);
		return 0;
	}
	
    typedef int (*FooProc)();

	FooProc sym = (FooProc)dlsym(handle, "foo");
	if ( sym == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL] dlopen-DYLD_LIBRARY_PATH %s\n", env);
		return 0;
	}

    int result = (*sym)();
    if ( result != value ) {
        printf("result=%d, expected %ld (str=%s)\n", result, value, valueStr);
        printf("[FAIL] dlopen-DYLD_LIBRARY_PATH %s\n", env);
		return 0;
	}

	int r = dlclose(handle);
	if ( r != 0 ) {
        printf("dlclose() returned %d\n", r);
        printf("[FAIL] dlopen-DYLD_LIBRARY_PATH %s\n", env);
		return 0;
	}

	void* handle2 = dlopen("/junk/libfoo.dylib", RTLD_LAZY);
	if ( handle2 == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL] dlopen-DYLD_LIBRARY_PATH %s\n", env);
		return 0;
	}



    printf("[PASS] dlopen-DYLD_LIBRARY_PATH %s\n", env);

	return 0;
}


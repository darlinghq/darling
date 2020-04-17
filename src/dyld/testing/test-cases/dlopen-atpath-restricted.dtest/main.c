// BUILD_ONLY: MacOSX

// BUILD:  mkdir -p $BUILD_DIR/test1
// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/test1/libtest1.dylib -install_name @rpath/libtest1.dylib
// BUILD:  $CC foo.c -bundle -o $BUILD_DIR/test1.bundle -Wl,-rpath,@loader_path/test1/ $BUILD_DIR/test1/libtest1.dylib

// BUILD:  mkdir -p $BUILD_DIR/test2
// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/test2/libtest2.dylib -install_name @loader_path/test2/libtest2.dylib
// BUILD:  $CC foo.c -bundle -o $BUILD_DIR/test2.bundle $BUILD_DIR/test2/libtest2.dylib

// BUILD:  mkdir -p $BUILD_DIR/test3
// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/test3/libtest3.dylib -install_name @rpath/libtest3.dylib
// BUILD:  $CC foo.c -bundle -o $BUILD_DIR/test3.bundle -Wl,-rpath,$RUN_DIR/test3  $BUILD_DIR/test3/libtest3.dylib 

// BUILD:  mkdir -p $BUILD_DIR/test4
// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/test4/libtest4.dylib -install_name @rpath/libtest4.dylib
// BUILD:  $CC foo.c -bundle -o $BUILD_DIR/test4.bundle -Wl,-rpath,@executable_path/test4/ $BUILD_DIR/test4/libtest4.dylib

// BUILD:  mkdir -p $BUILD_DIR/test5
// BUILD:  $CC bar.c -dynamiclib -o $BUILD_DIR/test5/libtest5.dylib -install_name @executable_path/test5/libtest5.dylib
// BUILD:  $CC foo.c -bundle -o $BUILD_DIR/test5.bundle $BUILD_DIR/test5/libtest5.dylib



// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-restricted.exe -DRUN_DIR="$RUN_DIR" -sectcreate __RESTRICT __restrict /dev/null

// RUN:  ./dlopen-restricted.exe

#include <stdio.h>
#include <dlfcn.h>


int main(int argc, const char* argv[])
{
    printf("[BEGIN] dlopen-restricted\n");

    // test1: LC_RPATH not in main executable uses @loader_path
    void* handle = dlopen(RUN_DIR "/test1.bundle", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL]  dlopen-restricted test1.bundle\n");
        return 0;
    }

    // test2: @loader_path not in main executable
    handle = dlopen(RUN_DIR "/test2.bundle", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL]  dlopen-restricted test2.bundle\n");
        return 0;
    }

    // test3: LC_RPATH not in main executable uses absolute path
    handle = dlopen(RUN_DIR "/test3.bundle", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL]  dlopen-restricted test3.bundle\n");
        return 0;
    }

    // test4: [SHOULD FAIL] LC_RPATH not in main executable uses @executable_path
    handle = dlopen(RUN_DIR "/test4.bundle", RTLD_LAZY);
    if ( handle != NULL ) {
        printf("[FAIL]  dlopen-restricted test4.bundle dlopen() should not work\n");
        return 0;
    }

    // test5: [SHOULD FAIL] @executable_path in LC_LOAD_DYLIB
    handle = dlopen(RUN_DIR "/test5.bundle", RTLD_LAZY);
    if ( handle != NULL ) {
        printf("[FAIL]  dlopen-restricted test5.bundle dlopen() should not work\n");
        return 0;
    }



    printf("[PASS]  dlopen-restricted\n");
	return 0;
}


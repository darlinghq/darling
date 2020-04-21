
// BUILD:  $CC foo.c -dynamiclib  -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC bar.c -dynamiclib  -install_name $RUN_DIR/libbar.dylib -o $BUILD_DIR/libbar.dylib
// BUILD:  $CC main.c -DRUN_DIR="$RUN_DIR" $BUILD_DIR/libfoo.dylib    -o $BUILD_DIR/dyld_immutable_test.exe

// RUN:  ./dyld_immutable_test.exe

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_priv.h>

#if __has_feature(ptrauth_calls)
    #include <ptrauth.h>
#endif

static const void* stripPointer(const void* ptr)
{
#if __has_feature(ptrauth_calls)
    return __builtin_ptrauth_strip(ptr, ptrauth_key_asia);
#else
    return ptr;
#endif
}


typedef const char* (*BarProc)(void);

extern uint32_t _cpu_capabilities;
extern const char* foo();

const char* myStr = "myStr";

int myInt;


int main()
{
    printf("[BEGIN] _dyld_is_memory_immutable\n");

    if ( !_dyld_is_memory_immutable(myStr, 6) ) {
		printf("[FAIL] _dyld_is_memory_immutable() returned false for string in main executable\n");
        return 0;
    }

    if ( _dyld_is_memory_immutable(strdup("hello"), 6) ) {
		printf("[FAIL] _dyld_is_memory_immutable() returned true for result from strdup()\n");
        return 0;
    }

    if ( _dyld_is_memory_immutable(&myInt, 4) ) {
		printf("[FAIL] _dyld_is_memory_immutable() returned true for global variabe in main executable\n");
        return 0;
    }

    if ( !_dyld_is_memory_immutable(foo(), 4) ) {
		printf("[FAIL] _dyld_is_memory_immutable() returned false for string in statically linked dylib\n");
        return 0;
    }

    if ( !_dyld_is_memory_immutable(stripPointer((void*)&strcpy), 4) ) {
		printf("[FAIL] _dyld_is_memory_immutable() returned false for strcpy function in dyld shared cache\n");
        return 0;
    }

    if ( _dyld_is_memory_immutable(&_cpu_capabilities, 4) ) {
		printf("[FAIL] _dyld_is_memory_immutable() returned true for global variable in shared cache\n");
        return 0;
    }

	void* handle = dlopen(RUN_DIR "/libbar.dylib", RTLD_FIRST);
    if ( handle == NULL ) {
		printf("[FAIL] dlopen(libbar.dylib) failed");
        return 0;
    }

    BarProc proc = dlsym(handle, "bar");
    if ( proc == NULL ) {
		printf("[FAIL] dlsym(bar) failed\n");
        return 0;
    }
    const char* barStr = (*proc)();
    if ( _dyld_is_memory_immutable(barStr, 4) ) {
		printf("[FAIL] _dyld_is_memory_immutable() returned true for string in unloadable dylib\n");
        return 0;
    }


    printf("[PASS] _dyld_is_memory_immutable\n");
    return 0;
}


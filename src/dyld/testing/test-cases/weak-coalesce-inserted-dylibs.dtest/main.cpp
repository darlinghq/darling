
// BUILD:  $CXX foo.cpp -dynamiclib -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib -fno-exceptions
// BUILD:  $CXX bar.cpp -dynamiclib -install_name $RUN_DIR/libbar.dylib -o $BUILD_DIR/libbar.dylib -fno-exceptions
// BUILD:  $CXX main.cpp  $BUILD_DIR/libfoo.dylib -o $BUILD_DIR/weak-coalesce-inserted-dylibs.exe -fno-exceptions
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/weak-coalesce-inserted-dylibs.exe

// RUN: DYLD_INSERT_LIBRARIES=libbar.dylib ./weak-coalesce-inserted-dylibs.exe


#include <stdio.h>
#include <stdlib.h>

extern void foo();

// We have our own copy of operator new.  Make sure that we call our version, but that foo calls the one from the inserted bar dylib
static bool calledMainNew = false;
static bool calledMainDelete = false;
static bool enableTracking = false;

void* operator new(size_t size)
{
    if (enableTracking)
        calledMainNew = true;
    void* ptr = malloc(size);
    return ptr;
}

void operator delete(void* ptr)
{
    if (enableTracking)
        calledMainDelete = true;
    free(ptr);
}

int main()
{
    printf("[BEGIN] weak-coalesce-inserted-dylibs\n");

    // First make sure we do use our versions of new and delete.
    enableTracking = true;

    int* v = new int(1);
    if (!calledMainNew) {
        printf("[FAIL] weak-coalesce-inserted-dylibs, didn't call executable operator new\n");
        return 1;
    }

    delete v;
    if (!calledMainDelete) {
        printf("[FAIL] weak-coalesce-inserted-dylibs, didn't call executable operator delete\n");
        return 1;
    }

    // Now make foo do the same and make sure we got the new/delete from bar
    calledMainNew = false;
    calledMainDelete = false;
    foo();

    if (calledMainNew) {
        printf("[FAIL] weak-coalesce-inserted-dylibs, didn't call bar operator new\n");
        return 1;
    }

    if (calledMainDelete) {
        printf("[FAIL] weak-coalesce-inserted-dylibs, didn't call bar operator delete\n");
        return 1;
    }

    printf("[PASS] weak-coalesce-inserted-dylibs\n");

    return 0;
}


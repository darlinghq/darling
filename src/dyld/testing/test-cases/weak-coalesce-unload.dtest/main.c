
// BUILD:  $CC foo1.c -dynamiclib -install_name $RUN_DIR/libfoo1.dylib -o $BUILD_DIR/libfoo1.dylib
// BUILD:  $CC foo2.c -dynamiclib -install_name $RUN_DIR/libfoo2.dylib -o $BUILD_DIR/libfoo2.dylib
// BUILD:  $CC foo3.c -dynamiclib -install_name $RUN_DIR/libfoo3.dylib -o $BUILD_DIR/libfoo3.dylib
// BUILD:  $CC main.c -o $BUILD_DIR/weak-coalesce-unload.exe -DRUN_DIR="$RUN_DIR"

// RUN:  ./weak-coalesce-unload.exe


#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

extern int foo();
extern void* fooPtr();

int main()
{
    printf("[BEGIN] weak-coalesce-unload\n");

	// dlopen foo1 which defines "foo"
	void* handle1 = dlopen(RUN_DIR "/libfoo1.dylib", RTLD_FIRST);
    if ( handle1 == NULL ) {
        printf("[FAIL] dlopen(\"%s\") failed with: %s\n", RUN_DIR "/libfoo1.dylib", dlerror());
        return 0;
    }

    const void* symFoo1 = dlsym(handle1, "foo");
    if ( symFoo1 == NULL ) {
        printf("[FAIL] dlsym(handle1, foo) failed\n");
        return 0;
    }

    const void* symFooPtr1 = dlsym(handle1, "fooPtr");
    if ( symFooPtr1 == NULL ) {
        printf("[FAIL] dlsym(handle1, fooPtr) failed\n");
        return 0;
    }
    void* fooptr1 = ((__typeof(&fooPtr))symFooPtr1)();

    int close1 = dlclose(handle1);
    if ( close1 != 0 ) {
        printf("[FAIL] dlclose(handle1) failed with: %s\n", dlerror());
        return 0;
    }

    // Now dlopen foo2 and get the value it finds for foo
    void* handle2 = dlopen(RUN_DIR "/libfoo2.dylib", RTLD_FIRST);
    if ( handle2 == NULL ) {
        printf("[FAIL] dlopen(\"%s\") failed with: %s\n", RUN_DIR "/libfoo2.dylib", dlerror());
        return 0;
    }

    const void* symFoo2 = dlsym(handle2, "foo");
    if ( symFoo2 == NULL ) {
        printf("[FAIL] dlsym(handle2, foo) failed\n");
        return 0;
    }

    const void* symFooPtr2 = dlsym(handle2, "fooPtr");
    if ( symFooPtr2 == NULL ) {
        printf("[FAIL] dlsym(handle2, fooPtr) failed\n");
        return 0;
    }
    void* fooptr2 = ((__typeof(&fooPtr))symFooPtr2)();

    // Don't close foo2, but instead open foo3
    void* handle3 = dlopen(RUN_DIR "/libfoo3.dylib", RTLD_FIRST);
    if ( handle3 == NULL ) {
        printf("[FAIL] dlopen(\"%s\") failed with: %s\n", RUN_DIR "/libfoo3.dylib", dlerror());
        return 0;
    }

    const void* symFoo3 = dlsym(handle3, "foo");
    if ( symFoo3 == NULL ) {
        printf("[FAIL] dlsym(handle3, foo) failed\n");
        return 0;
    }

    const void* symFooPtr3 = dlsym(handle3, "fooPtr");
    if ( symFooPtr3 == NULL ) {
        printf("[FAIL] dlsym(handle3, fooPtr) failed\n");
        return 0;
    }
    void* fooptr3 = ((__typeof(&fooPtr))symFooPtr3)();

    // No-one should point to libfoo1.dylib
    if ( symFoo1 == symFoo2 ) {
    	printf("[FAIL] foo1 == foo2\n");
        return 0;
    }
    if ( symFoo1 == symFoo3 ) {
    	printf("[FAIL] foo1 == foo3\n");
        return 0;
    }

    // foo2 and foo3 should be different
    if ( symFoo2 == symFoo3 ) {
    	printf("[FAIL] foo2 != foo3\n");
        return 0;
    }

    // But their coalesced values should be the same
    if ( fooptr1 == fooptr2 ) {
    	printf("[FAIL] fooptr1 == fooptr2\n");
        return 0;
    }
    if ( fooptr2 != fooptr3 ) {
    	printf("[FAIL] fooptr2 != fooptr3\n");
        return 0;
    }

    // foo should return the value from foo2, not the value from foo3
    // Also calling this would explode if we somehow pointed at foo1
    if ( ((__typeof(&foo))fooptr2)() != 2 ) {
    	printf("[FAIL] foo2 != 2\n");
        return 0;	
    }
    if ( ((__typeof(&foo))fooptr3)() != 2 ) {
    	printf("[FAIL] foo3 != 2\n");
        return 0;	
    }

    printf("[PASS] weak-coalesce-unload\n");
	return 0;
}



// BUILD:  $CC foo.c -dynamiclib -Wl,-U,_gInitialisersCalled                                         -install_name $RUN_DIR/libfoo.dylib -o $BUILD_DIR/libfoo.dylib
// BUILD:  $CC bar.c -dynamiclib -Wl,-U,_gInitialisersCalled $BUILD_DIR/libfoo.dylib -flat_namespace -install_name $RUN_DIR/libbar.dylib -o $BUILD_DIR/libbar.dylib -Wl,-w
// BUILD:  $CC main.c -DRUN_DIR="$RUN_DIR"                                                                                               -o $BUILD_DIR/dlopen-flat.exe

// RUN:  ./dlopen-flat.exe

#include <stdio.h>
#include <dlfcn.h>

int gInitialisersCalled = 0;

int main() {
	printf("[BEGIN] dlopen-flat\n");
	int result;

	// Foo exports foo()
	void* fooHandle = 0;
	{
		fooHandle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_LAZY);
		if (!fooHandle) {
			printf("dlopen failed with error: %s\n", dlerror());
			return 1;
		}
		if (gInitialisersCalled != 1) {
	        printf("gInitialisersCalled != 1\n");
	        printf("[FAIL] dlopen-flat\n");
			return 1;
		}
	}
	// Now unload foo which should do something.
	result = dlclose(fooHandle);
	if (result != 0) {
        printf("dlclose() returned %c\n", result);
        printf("[FAIL] dlopen-flat\n");
		return 1;
	}

	// Open foo again which should do something.
	{
		fooHandle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_LAZY);
		if (!fooHandle) {
			printf("dlopen failed with error: %s\n", dlerror());
			return 1;
		}
		if (gInitialisersCalled != 2) {
	        printf("gInitialisersCalled != 2\n");
	        printf("[FAIL] dlopen-flat\n");
			return 1;
		}
	}

	// Bar is going to resolve foo()
	void* barHandle = 0;
	{
		barHandle = dlopen(RUN_DIR "/libbar.dylib", RTLD_LAZY);
		if (!barHandle) {
			printf("dlopen failed with error: %s\n", dlerror());
			return 1;
		}
		if (gInitialisersCalled != 3) {
	        printf("gInitialisersCalled != 3\n");
	        printf("[FAIL] dlopen-flat\n");
			return 1;
		}
	}
	// Now unload foo which shouldn't do anything.
	result = dlclose(fooHandle);
	if (result != 0) {
        printf("dlclose() returned %c\n", result);
        printf("[FAIL] dlopen-flat\n");
		return 1;
	}

	// Open foo again which shouldn't do anything.
	{
		fooHandle = dlopen(RUN_DIR "/libfoo.dylib", RTLD_LAZY);
		if (!fooHandle) {
			printf("dlopen failed with error: %s\n", dlerror());
			return 1;
		}
		if (gInitialisersCalled != 3) {
	        printf("gInitialisersCalled != 3\n");
	        printf("[FAIL] dlopen-flat\n");
			return 1;
		}
	}

    printf("[PASS] dlopen-flat\n");
	return 0;
}


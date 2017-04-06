
// BUILD:  $CC main.c            -o $BUILD_DIR/dlopen-framework-fallback.exe

// RUN:  ./dlopen-framework-fallback.exe

#include <stdio.h>
#include <dlfcn.h>



int main()
{
    printf("[BEGIN] dlopen-framework-fallback\n");

    // Verify dyld will fallback and look for framework in /System/Library/Frameworks/
	void* handle = dlopen("/System/Library/BadPath/CoreFoundation.framework/CoreFoundation", RTLD_LAZY);
	if ( handle == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL] dlopen-framework-fallback\n");
		return 0;
	}

	// validate handle works to find symbols
	void* sym = dlsym(handle, "CFRetain");
	if ( sym == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL] dlopen-framework-fallback\n");
		return 0;
	}

    printf("[PASS] dlopen-framework-fallback\n");

	return 0;
}


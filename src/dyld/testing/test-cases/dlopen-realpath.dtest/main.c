
// BUILD:  $CC main.c -o $BUILD_DIR/dlopen-realpath.exe
// BUILD:  cd $BUILD_DIR && ln -s ./IOKit.framework/IOKit IOKit && ln -s /System/Library/Frameworks/IOKit.framework IOKit.framework

// RUN:  DYLD_FALLBACK_LIBRARY_PATH=/baz  ./dlopen-realpath.exe

#include <stdio.h>
#include <dlfcn.h>


static void tryImage(const char* path)
{
    printf("[BEGIN] dlopen-realpath %s\n", path);
	void* handle = dlopen(path, RTLD_LAZY);
	if ( handle == NULL ) {
        printf("dlerror(): %s\n", dlerror());
        printf("[FAIL] dlopen-realpath %s\n", path);
		return;
	}

	int result = dlclose(handle);
	if ( result != 0 ) {
        printf("dlclose(%p): %s\n", handle, dlerror());
        printf("[FAIL] dlopen-realpath %s\n", path);
		return;
	}

    printf("[PASS] dlopen-realpath %s\n", path);
}



int main()
{
	tryImage("./IOKit.framework/IOKit");
	tryImage("./././IOKit/../IOKit.framework/IOKit");
    tryImage("./IOKit");

    // Also try libSystem which has an alias in the OS to /usr/lib/libSystem.B.dylib
    tryImage("/usr/lib/libSystem.dylib");

    // Also try using non-canonical path
    // This requires DYLD_FALLBACK_LIBRARY_PATH to be disabled, otherwise it is found that way
    tryImage("//usr/lib/libSystem.dylib");
    tryImage("/usr/./lib/libSystem.dylib");

	return 0;
}


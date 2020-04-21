
// BUILD:  mkdir -p $BUILD_DIR/override
// BUILD:  mkdir -p $BUILD_DIR/re-export-override
// BUILD:  $CC myzlib.c -dynamiclib -o $BUILD_DIR/override/libz.1.dylib -install_name /usr/lib/libz.1.dylib -compatibility_version 1.0
// BUILD:  $CC main.c  -o $BUILD_DIR/dyld_shared_cache_some_image_overridden.exe -lz
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/dyld_shared_cache_some_image_overridden.exe
// BUILD:  $CC main.c  -o $BUILD_DIR/dyld_shared_cache_some_image_overridden-no-lz.exe -DNO_LZ
// BUILD:  $DYLD_ENV_VARS_ENABLE $BUILD_DIR/dyld_shared_cache_some_image_overridden-no-lz.exe

// RUN:  ./dyld_shared_cache_some_image_overridden.exe
// RUN:  DYLD_LIBRARY_PATH=$RUN_DIR/override/ ./dyld_shared_cache_some_image_overridden.exe
// RUN:  ./dyld_shared_cache_some_image_overridden-no-lz.exe
// RUN:  DYLD_LIBRARY_PATH=$RUN_DIR/override/ ./dyld_shared_cache_some_image_overridden-no-lz.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <stdbool.h>

#include <dlfcn.h>
#include <mach-o/dyld_priv.h>

// The test here is to override libz.1.dylib which is in the dyld cache with our own implementation.
// We then ensure that dyld_shared_cache_some_image_overridden returns the correct value to match whether we took a root

extern const char* zlibVersion();

int main()
{
    // If we aren't using a shared cache, eg, have DYLD_SHARED_REGION=avoid, then just assume we work
    uuid_t currentCacheUUID;
    if ( !_dyld_get_shared_cache_uuid(currentCacheUUID) ) {
        printf("[BEGIN] dyld_shared_cache_some_image_overridden\n");
        if (dyld_shared_cache_some_image_overridden())
            printf("[FAIL] dyld_shared_cache_some_image_overridden\n");
        else
            printf("[PASS] dyld_shared_cache_some_image_overridden\n");
        return 0;
    }

#if NO_LZ
	// This run doesn't link lz so instead dlopen's it
    bool expectMyDylib = (getenv("DYLD_LIBRARY_PATH") != NULL);

    printf("[BEGIN] dyld_shared_cache_some_image_overridden, %s\n", expectMyDylib ? "my" : "os");

    void* handle = dlopen("/usr/lib/libz.1.dylib", RTLD_NOLOAD);
    if ( handle != NULL ) {
        // Uh oh.  Someone else has started linking libz so we can't use it as our root any more
        printf("[FAIL] dyld_shared_cache_some_image_overridden, libz is hard linked now.  Update test to use a new dylib\n");
        return 0;
    }

    bool launchedWithOverriddenBinary = dyld_shared_cache_some_image_overridden();

    // Now dlopen libz
    handle = dlopen("/usr/lib/libz.1.dylib", RTLD_LAZY);
    if ( handle == NULL ) {
        printf("[FAIL]  dyld_shared_cache_some_image_overridden: /usr/lib/libz.1.dylib could not be loaded, %s\n", dlerror());
        return 0;
    }

    // verify handle has the version symbol
    __typeof(&zlibVersion) versionSymbol = (__typeof(&zlibVersion))dlsym(handle, "zlibVersion");
    if ( versionSymbol == NULL ) {
        printf("[FAIL]  dyld_shared_cache_some_image_overridden: zlibVersion was not found\n");
        return 0;
    }

    bool usingMyDylib = (strcmp(versionSymbol(), "my") == 0);

	if ( usingMyDylib != expectMyDylib ) {
		// Not using the right dylib
        printf("[FAIL] dyld_shared_cache_some_image_overridden, %s\n", expectMyDylib ? "my" : "os");
        return 0;
    }

    // Using the right dylib, so now see if we returned the correct value for dyld_shared_cache_some_image_overridden
    if (usingMyDylib) {
    	if (!dyld_shared_cache_some_image_overridden()) {
    		printf("[FAIL] dyld_shared_cache_some_image_overridden, my dylib but not some dylib overridden\n");
        	return 0;
    	}
    } else if (!launchedWithOverriddenBinary) {
        // We didn't have a root when we launched, so now we can make sure we do have a root after the dlopen
        // Assume we aren't testing against a root of libz in the system itself...
        if (dyld_shared_cache_some_image_overridden()) {
            printf("[FAIL] dyld_shared_cache_some_image_overridden, system dylib was overridden\n");
            return 0;
        }
    } else {
    	// We can't actually be sure of the result here.  There may be other roots on the system so call the API to
    	// make sure it doesn't crash, but don't actually check it.
        dyld_shared_cache_some_image_overridden();
    }
    

    printf("[PASS] dyld_shared_cache_some_image_overridden, %s\n", expectMyDylib ? "my" : "os");

#else
    // This run links libz directly
    bool expectMyDylib = (getenv("DYLD_LIBRARY_PATH") != NULL);

    printf("[BEGIN] dyld_shared_cache_some_image_overridden, %s\n", expectMyDylib ? "my" : "os");

    bool usingMyDylib = (strcmp(zlibVersion(), "my") == 0);

	if ( usingMyDylib != expectMyDylib ) {
		// Not using the right dylib
        printf("[FAIL] dyld_shared_cache_some_image_overridden, %s\n", expectMyDylib ? "my" : "os");
        return 0;
    }

    // Using the right dylib, so now see if we returned the correct value for dyld_shared_cache_some_image_overridden
    if (usingMyDylib) {
    	if (!dyld_shared_cache_some_image_overridden()) {
    		printf("[FAIL] dyld_shared_cache_some_image_overridden, my dylib but not some dylib overridden\n");
        	return 0;
    	}
    } else {
    	// We can't actually be sure of the result here.  There may be other roots on the system so call the API to
    	// make sure it doesn't crash, but don't actually check it.
        dyld_shared_cache_some_image_overridden();
    }
    
    printf("[PASS] dyld_shared_cache_some_image_overridden, %s\n", expectMyDylib ? "my" : "os");
#endif

	return 0;
}


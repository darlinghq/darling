
// BUILD:  $CC main.c -o $BUILD_DIR/_dyld_get_objc_selector-shared-cache.exe

// RUN:  ./_dyld_get_objc_selector-shared-cache.exe

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_priv.h>


int main()
{
    printf("[BEGIN] _dyld_get_objc_selector-shared-cache\n");

    size_t cacheLen;
    uintptr_t cacheStart = (uintptr_t)_dyld_get_shared_cache_range(&cacheLen);

    const char* selName = _dyld_get_objc_selector("retain");

    if ( cacheStart != 0 ) {
        // We have a shared cache, so the selector should be there
        if ( selName == NULL ) {
            printf("[FAIL] _dyld_get_objc_selector() returned null for selector in shared cache\n");
            return 0;
        }

        if ( ((uintptr_t)selName < cacheStart) || ((uintptr_t)selName >= (cacheStart + cacheLen)) ) {
            printf("[FAIL] _dyld_get_objc_selector() pointer outside of shared cache range\n");
            return 0;
        }
    } else {
        // No shared cache, so the selector should not be found.
        // FIXME: This assumption may be false once the selectors are in the closure.
        if ( selName != NULL ) {
            printf("[FAIL] _dyld_get_objc_selector() returned non-null for selector without shared cache\n");
            return 0;
        }
    }

    printf("[PASS] _dyld_get_objc_selector-shared-cache\n");
    return 0;
}


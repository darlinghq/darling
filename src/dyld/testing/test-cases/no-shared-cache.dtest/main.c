// BUILD_ONLY: MacOSX

// BUILD:  $CC main.c  -framework AppKit         -o $BUILD_DIR/no_shared_cache.exe

// RUN:  DYLD_SHARED_REGION=avoid ./no_shared_cache.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <mach-o/dyld_priv.h>
#include <dlfcn.h>


// This program links with AppKit which in dyld3 mode stress tests building closures when there is no dyld shared cache

int main()
{
    printf("[BEGIN] no-shared-cache\n");

    size_t cacheLen;
    const void* cacheStart = _dyld_get_shared_cache_range(&cacheLen);

    if ( cacheStart != NULL ) {
        printf("[FAIL] no-shared-cache: _dyld_get_shared_cache_range() returned %p even though we are not using a dyld cache\n", cacheStart);
        return 0;
    }

    printf("[PASS] no-shared-cache\n");
    return 0;
}


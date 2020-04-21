
// BUILD:  $CC main.c            -o $BUILD_DIR/shared_cache_range.exe

// RUN:  ./shared_cache_range.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <mach-o/dyld_priv.h>
#include <dlfcn.h>

#if __has_feature(ptrauth_calls)
    #include <ptrauth.h>
#endif

static const void *stripPointer(const void *ptr) {
#if __has_feature(ptrauth_calls)
    return __builtin_ptrauth_strip(ptr, ptrauth_key_asia);
#else
    return ptr;
#endif
}


int main()
{
    printf("[BEGIN] shared_cache_range\n");

    // see if image containing malloc is in the dyld cache
    Dl_info info;
    if ( dladdr(&malloc, &info) == 0 ) {
        printf("[FAIL] shared_cache_range: dladdr(&malloc, xx) failed");
        return 0;
    }
    const struct mach_header* mh = (struct mach_header*)info.dli_fbase;
    printf("image with malloc=%p\n", mh);
    if ( mh == NULL ) {
        printf("[FAIL] shared_cache_range: dladdr(&malloc, xx) => dli_fbase==NULL");
        return 0;
    }
    bool haveSharedCache = (mh->flags & 0x80000000);
    printf("haveSharedCache=%d\n", haveSharedCache);

    size_t cacheLen;
    const void* cacheStart = _dyld_get_shared_cache_range(&cacheLen);

    if ( haveSharedCache ) {
        if ( cacheStart == NULL ) {
            printf("[FAIL] _dyld_get_shared_cache_range() returned NULL even though we have a cache\n");
            return 0;
        }
        printf("shared cache start=%p, len=0x%0lX\n", cacheStart, cacheLen);
        const void* cacheEnd = (char*)cacheStart + cacheLen;

        // verify malloc is in shared cache
        if ( (stripPointer((void*)&malloc) < cacheStart) || (stripPointer((void*)&malloc) > cacheEnd) ) {
            printf("[FAIL] shared_cache_range: malloc is outside range of cache\n");
            return 0;
        }
    }
    else {
        if ( cacheStart != NULL ) {
            printf("[FAIL] _dyld_get_shared_cache_range() returned non-NULL even though we don't have a cache\n");
            return 0;
        }
    }

    printf("[PASS] shared_cache_range\n");
    return 0;
}


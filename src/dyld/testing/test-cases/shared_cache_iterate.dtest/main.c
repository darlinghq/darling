
// BUILD:  $CC main.c -o $BUILD_DIR/shared_cache_iterate.exe

// RUN:  ./shared_cache_iterate.exe


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach-o/dyld_priv.h>
#include <dlfcn.h>
#include <sys/syslimits.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#if __has_feature(ptrauth_calls)
    #include <ptrauth.h>
#endif



struct dyld_cache_header
{
    char      magic[16];
    uint64_t  other[9];
    uuid_t    uuid;
};

static void forEachCacheInDir(const char* dirPath, void (^handler)(const uuid_t uuid))
{
    DIR* dirp = opendir(dirPath);
    if ( dirp != NULL) {
        struct dirent entry;
        struct dirent* entp = NULL;
        char cachePath[PATH_MAX];
        while ( readdir_r(dirp, &entry, &entp) == 0 ) {
            if ( entp == NULL )
                break;
            if ( entp->d_type != DT_REG )
                continue;
            if ( strlcpy(cachePath, dirPath, PATH_MAX) >= PATH_MAX )
                continue;
            if ( strlcat(cachePath, "/", PATH_MAX) >= PATH_MAX )
                continue;
            if ( strlcat(cachePath, entp->d_name, PATH_MAX) >= PATH_MAX )
                continue;
            int fd = open(cachePath, O_RDONLY);
            if ( fd < 0 )
                continue;
            struct dyld_cache_header cacheHeader;
            ssize_t amount = pread(fd, &cacheHeader, sizeof(cacheHeader), 0);
            close(fd);
            if ( amount != sizeof(cacheHeader) )
                continue;
            if ( memcmp(cacheHeader.magic, "dyld_v", 6) == 0 )
                handler(cacheHeader.uuid);
        }
        closedir(dirp);
    }
}


int main()
{
    printf("[BEGIN] shared_cache_iterate\n");

    size_t cacheLen;
    const void* cacheStart = _dyld_get_shared_cache_range(&cacheLen);
    uuid_t currentCacheUUID;
    if ( _dyld_get_shared_cache_uuid(currentCacheUUID) ) {
        const uint8_t* currentCacheUUIDptr = currentCacheUUID;
        // have dyld cache
        __block unsigned count      = 0;
        __block bool     badVersion = false;
        // iterate current cache
        int result = dyld_shared_cache_iterate_text(currentCacheUUID, ^(const dyld_shared_cache_dylib_text_info* info) {
            if ( info->version != 2 )
                badVersion = true;
            ++count;
        });
        if ( result != 0 ) {
            printf("[FAIL] shared_cache_iterate dyld_shared_cache_iterate_text() returned non-zero: %d\n", result);
            return 0;
        }
        if ( count < 100 ) {
            printf("[FAIL] shared_cache_iterate dyld_shared_cache_iterate_text() iterated over less than 100 images: %d\n", count);
            return 0;
        }
        if ( badVersion ) {
            printf("[FAIL] shared_cache_iterate dyld_shared_cache_iterate_text() some dyld_shared_cache_dylib_text_info was not 2\n");
            return 0;
        }

        // iterate current cache
        count = 0;
        const char* extraSearchDirsStorage[] = { "/tmp/", NULL };
        const char** extraSearchDirs = extraSearchDirsStorage;
        result = dyld_shared_cache_find_iterate_text(currentCacheUUID, extraSearchDirs, ^(const dyld_shared_cache_dylib_text_info* info) {
            if ( info->version != 2 )
                badVersion = true;
            ++count;
        });
        if ( result != 0 ) {
            printf("[FAIL] shared_cache_iterate dyld_shared_cache_find_iterate_text() returned non-zero: %d\n", result);
            return 0;
        }
        if ( count < 100 ) {
            printf("[FAIL] shared_cache_iterate dyld_shared_cache_find_iterate_text() iterated over less than 100 images: %d\n", count);
            return 0;
        }
        if ( badVersion ) {
            printf("[FAIL] shared_cache_iterate dyld_shared_cache_find_iterate_text() some dyld_shared_cache_dylib_text_info was not 2\n");
            return 0;
        }

        // look for non-existent cache
        count = 0;
        uuid_t badCacheUUID;
        bzero(&badCacheUUID, sizeof(uuid_t));
        badCacheUUID[1] = 0x33;
        result = dyld_shared_cache_find_iterate_text(badCacheUUID, extraSearchDirs, ^(const dyld_shared_cache_dylib_text_info* info) {
            ++count;
        });
        if ( result == 0 ) {
            printf("[FAIL] shared_cache_iterate dyld_shared_cache_find_iterate_text() expected result to be nonzero: %d\n", result);
            return 0;
        }
        if ( count != 0 ) {
            printf("[FAIL] shared_cache_iterate dyld_shared_cache_find_iterate_text() expected iteration count is zero: %d\n", count);
            return 0;
        }

        // find other cache
        const char* curCachePath = dyld_shared_cache_file_path();
        if ( curCachePath == NULL ) {
            printf("[FAIL] shared_cache_iterate dyld_shared_cache_file_path() returned NULL\n");
            return 0;
        }
        char cacheDir[PATH_MAX];
        strlcpy(cacheDir, curCachePath, PATH_MAX);
        char* end = strrchr(cacheDir, '/');
        if ( end == NULL ) {
            printf("[FAIL] shared_cache_iterate cache path has no '/'\n");
            return 0;
        }
        *end = '\0';
        forEachCacheInDir(cacheDir, ^(const uuid_t uuid) {
            if ( uuid_compare(uuid, currentCacheUUIDptr) != 0 ) {
                count = 0;
                int res = dyld_shared_cache_find_iterate_text(uuid, extraSearchDirs, ^(const dyld_shared_cache_dylib_text_info* info) {
                    ++count;
                });
                if ( res != 0 ) {
                    printf("[FAIL] shared_cache_iterate dyld_shared_cache_find_iterate_text() expected result to be nonzero: %d\n", result);
                    exit(0);
                }
                if ( count < 100 ) {
                    printf("[FAIL] shared_cache_iterate dyld_shared_cache_find_iterate_text() iterated over less than 100 images: %d\n", count);
                    exit(0);
                }
            }
        });
    }
    else {
        printf("no dyld cache\n");
    }

    printf("[PASS] shared_cache_iterate\n");
    return 0;
}



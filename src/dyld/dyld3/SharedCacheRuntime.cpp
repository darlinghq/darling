/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */



#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/syslog.h>
#include <sys/sysctl.h>
#include <sys/mman.h>
#include <mach/mach.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach-o/ldsyms.h>
#include <mach/shared_region.h>
#include <mach/mach.h>
#include <Availability.h>
#include <TargetConditionals.h>

#include "dyld_cache_format.h"
#include "SharedCacheRuntime.h"
#include "Loading.h"
#include "BootArgs.h"

#define ENABLE_DYLIBS_TO_OVERRIDE_CACHE_SIZE 1024

// should be in mach/shared_region.h
extern "C" int __shared_region_check_np(uint64_t* startaddress);
extern "C" int __shared_region_map_and_slide_np(int fd, uint32_t count, const shared_file_mapping_np mappings[], long slide, const dyld_cache_slide_info2* slideInfo, size_t slideInfoSize);


namespace dyld {
    extern int  my_stat(const char* path, struct stat* buf);
    extern int  my_open(const char* path, int flag, int other);
    extern void log(const char*, ...);
}


namespace dyld3 {


struct CacheInfo
{
    int                     fd;
    shared_file_mapping_np  mappings[3];
    uint64_t                slideInfoAddressUnslid;
    size_t                  slideInfoSize;
    uint64_t                sharedRegionStart;
    uint64_t                sharedRegionSize;
    uint64_t                maxSlide;
};




#if __i386__
    #define ARCH_NAME            "i386"
    #define ARCH_CACHE_MAGIC     "dyld_v1    i386"
#elif __x86_64__
    #define ARCH_NAME            "x86_64"
    #define ARCH_CACHE_MAGIC     "dyld_v1  x86_64"
    #define ARCH_NAME_H          "x86_64h"
    #define ARCH_CACHE_MAGIC_H   "dyld_v1 x86_64h"
#elif __ARM_ARCH_7K__
    #define ARCH_NAME            "armv7k"
    #define ARCH_CACHE_MAGIC     "dyld_v1  armv7k"
#elif __ARM_ARCH_7A__
    #define ARCH_NAME            "armv7"
    #define ARCH_CACHE_MAGIC     "dyld_v1   armv7"
#elif __ARM_ARCH_7S__
    #define ARCH_NAME            "armv7s"
    #define ARCH_CACHE_MAGIC     "dyld_v1  armv7s"
#elif __arm64e__
    #define ARCH_NAME            "arm64e"
    #define ARCH_CACHE_MAGIC     "dyld_v1  arm64e"
#elif __arm64__
    #if __LP64__
        #define ARCH_NAME            "arm64"
        #define ARCH_CACHE_MAGIC     "dyld_v1   arm64"
    #else
        #define ARCH_NAME            "arm64_32"
        #define ARCH_CACHE_MAGIC     "dyld_v1arm64_32"
    #endif
#endif


#if !TARGET_OS_SIMULATOR
static void rebaseChainV2(uint8_t* pageContent, uint16_t startOffset, uintptr_t slideAmount, const dyld_cache_slide_info2* slideInfo)
{
    const uintptr_t   deltaMask    = (uintptr_t)(slideInfo->delta_mask);
    const uintptr_t   valueMask    = ~deltaMask;
    const uintptr_t   valueAdd     = (uintptr_t)(slideInfo->value_add);
    const unsigned    deltaShift   = __builtin_ctzll(deltaMask) - 2;

    uint32_t pageOffset = startOffset;
    uint32_t delta = 1;
    while ( delta != 0 ) {
        uint8_t* loc = pageContent + pageOffset;
        uintptr_t rawValue = *((uintptr_t*)loc);
        delta = (uint32_t)((rawValue & deltaMask) >> deltaShift);
        uintptr_t value = (rawValue & valueMask);
        if ( value != 0 ) {
            value += valueAdd;
            value += slideAmount;
        }
        *((uintptr_t*)loc) = value;
        //dyld::log("         pageOffset=0x%03X, loc=%p, org value=0x%08llX, new value=0x%08llX, delta=0x%X\n", pageOffset, loc, (uint64_t)rawValue, (uint64_t)value, delta);
        pageOffset += delta;
    }
}
#endif

#if !__LP64__ && !TARGET_OS_SIMULATOR
static void rebaseChainV4(uint8_t* pageContent, uint16_t startOffset, uintptr_t slideAmount, const dyld_cache_slide_info4* slideInfo)
{
    const uintptr_t   deltaMask    = (uintptr_t)(slideInfo->delta_mask);
    const uintptr_t   valueMask    = ~deltaMask;
    const uintptr_t   valueAdd     = (uintptr_t)(slideInfo->value_add);
    const unsigned    deltaShift   = __builtin_ctzll(deltaMask) - 2;

    uint32_t pageOffset = startOffset;
    uint32_t delta = 1;
    while ( delta != 0 ) {
        uint8_t* loc = pageContent + pageOffset;
        uintptr_t rawValue = *((uintptr_t*)loc);
        delta = (uint32_t)((rawValue & deltaMask) >> deltaShift);
        uintptr_t value = (rawValue & valueMask);
        if ( (value & 0xFFFF8000) == 0 ) {
           // small positive non-pointer, use as-is
        }
        else if ( (value & 0x3FFF8000) == 0x3FFF8000 ) {
           // small negative non-pointer
           value |= 0xC0000000;
        }
        else {
            value += valueAdd;
            value += slideAmount;
        }
        *((uintptr_t*)loc) = value;
        //dyld::log("         pageOffset=0x%03X, loc=%p, org value=0x%08llX, new value=0x%08llX, delta=0x%X\n", pageOffset, loc, (uint64_t)rawValue, (uint64_t)value, delta);
        pageOffset += delta;
    }
}
#endif

static void getCachePath(const SharedCacheOptions& options, size_t pathBufferSize, char pathBuffer[])
{
    // set cache dir
    if ( options.cacheDirOverride != nullptr ) {
        strlcpy(pathBuffer, options.cacheDirOverride, pathBufferSize);
    }
    else {
#if __IPHONE_OS_VERSION_MIN_REQUIRED
        strlcpy(pathBuffer, IPHONE_DYLD_SHARED_CACHE_DIR, sizeof(IPHONE_DYLD_SHARED_CACHE_DIR));
#else
        strlcpy(pathBuffer, MACOSX_DYLD_SHARED_CACHE_DIR, sizeof(MACOSX_DYLD_SHARED_CACHE_DIR));
#endif
    }

    // append file component of cache file
    if ( pathBuffer[strlen(pathBuffer)-1] != '/' )
        strlcat(pathBuffer, "/", pathBufferSize);
#if __x86_64__ && !__IPHONE_OS_VERSION_MIN_REQUIRED
    if ( options.useHaswell ) {
        size_t len = strlen(pathBuffer);
        struct stat haswellStatBuf;
        strlcat(pathBuffer, DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME_H, pathBufferSize);
        if ( dyld::my_stat(pathBuffer, &haswellStatBuf) == 0 )
            return;
        // no haswell cache file, use regular x86_64 cache
        pathBuffer[len] = '\0';
    }
#endif

    strlcat(pathBuffer, DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME, pathBufferSize);

#if __IPHONE_OS_VERSION_MIN_REQUIRED && !TARGET_OS_SIMULATOR
    // use .development cache if it exists
    struct stat enableStatBuf;
    struct stat devCacheStatBuf;
    struct stat optCacheStatBuf;
    bool developmentDevice = dyld3::internalInstall();
    bool enableFileExists = (dyld::my_stat(IPHONE_DYLD_SHARED_CACHE_DIR "enable-dylibs-to-override-cache", &enableStatBuf) == 0);
    bool devCacheExists = (dyld::my_stat(IPHONE_DYLD_SHARED_CACHE_DIR DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME DYLD_SHARED_CACHE_DEVELOPMENT_EXT, &devCacheStatBuf) == 0);
    bool optCacheExists = (dyld::my_stat(IPHONE_DYLD_SHARED_CACHE_DIR DYLD_SHARED_CACHE_BASE_NAME ARCH_NAME, &optCacheStatBuf) == 0);
    if ( !BootArgs::forceCustomerCache() && developmentDevice && ((enableFileExists && (enableStatBuf.st_size < ENABLE_DYLIBS_TO_OVERRIDE_CACHE_SIZE) && devCacheExists) || !optCacheExists) )
        strlcat(pathBuffer, DYLD_SHARED_CACHE_DEVELOPMENT_EXT, pathBufferSize);
#endif

}


int openSharedCacheFile(const SharedCacheOptions& options, SharedCacheLoadInfo* results)
{
    getCachePath(options, sizeof(results->path), results->path);
    return dyld::my_open(results->path, O_RDONLY, 0);
}

static bool validMagic(const SharedCacheOptions& options, const DyldSharedCache* cache)
{
    if ( strcmp(cache->header.magic, ARCH_CACHE_MAGIC) == 0 )
        return true;

#if __x86_64__
    if ( options.useHaswell ) {
        if ( strcmp(cache->header.magic, ARCH_CACHE_MAGIC_H) == 0 )
            return true;
    }
#endif
    return false;
}


static bool validPlatform(const SharedCacheOptions& options, const DyldSharedCache* cache)
{
    // grandfather in old cache that does not have platform in header
    if ( cache->header.mappingOffset < 0xE0 )
        return true;

    if ( cache->header.platform != (uint32_t)MachOFile::currentPlatform() )
        return false;

#if TARGET_OS_SIMULATOR
    if ( cache->header.simulator == 0 )
        return false;
#else
    if ( cache->header.simulator != 0 )
        return false;
#endif

    return true;
}

#if !TARGET_OS_SIMULATOR
static void verboseSharedCacheMappings(const shared_file_mapping_np mappings[3])
{
    for (int i=0; i < 3; ++i) {
        dyld::log("        0x%08llX->0x%08llX init=%x, max=%x %s%s%s\n",
            mappings[i].sfm_address, mappings[i].sfm_address+mappings[i].sfm_size-1,
            mappings[i].sfm_init_prot, mappings[i].sfm_init_prot,
            ((mappings[i].sfm_init_prot & VM_PROT_READ) ? "read " : ""),
            ((mappings[i].sfm_init_prot & VM_PROT_WRITE) ? "write " : ""),
            ((mappings[i].sfm_init_prot & VM_PROT_EXECUTE) ? "execute " : ""));
    }
}
#endif

static bool preflightCacheFile(const SharedCacheOptions& options, SharedCacheLoadInfo* results, CacheInfo* info)
{
    
    // find and open shared cache file
    int fd = openSharedCacheFile(options, results);
    if ( fd == -1 ) {
        results->errorMessage = "shared cache file open() failed";
        return false;
    }

    struct stat cacheStatBuf;
    if ( dyld::my_stat(results->path, &cacheStatBuf) != 0 ) {
        results->errorMessage = "shared cache file stat() failed";
        ::close(fd);
        return false;
    }
    size_t cacheFileLength = (size_t)(cacheStatBuf.st_size);

    // sanity check header and mappings
    uint8_t firstPage[0x4000];
    if ( ::pread(fd, firstPage, sizeof(firstPage), 0) != sizeof(firstPage) ) {
        results->errorMessage = "shared cache file pread() failed";
        ::close(fd);
        return false;
    }
    const DyldSharedCache* cache = (DyldSharedCache*)firstPage;
    if ( !validMagic(options, cache) ) {
        results->errorMessage = "shared cache file has wrong magic";
        ::close(fd);
        return false;
    }
    if ( !validPlatform(options, cache) ) {
        results->errorMessage = "shared cache file is for a different platform";
        ::close(fd);
        return false;
    }
    if ( (cache->header.mappingCount != 3) || (cache->header.mappingOffset > 0x138) ) {
        results->errorMessage = "shared cache file mappings are invalid";
        ::close(fd);
        return false;
    }
    const dyld_cache_mapping_info* const fileMappings = (dyld_cache_mapping_info*)&firstPage[cache->header.mappingOffset];
    if (  (fileMappings[0].fileOffset != 0)
      || ((fileMappings[0].address + fileMappings[0].size) > fileMappings[1].address)
      || ((fileMappings[1].address + fileMappings[1].size) > fileMappings[2].address)
      || ((fileMappings[0].fileOffset + fileMappings[0].size) != fileMappings[1].fileOffset)
      || ((fileMappings[1].fileOffset + fileMappings[1].size) != fileMappings[2].fileOffset)
      || ((cache->header.codeSignatureOffset + cache->header.codeSignatureSize) != cacheFileLength)
      || (fileMappings[0].maxProt != (VM_PROT_READ|VM_PROT_EXECUTE))
      || (fileMappings[1].maxProt != (VM_PROT_READ|VM_PROT_WRITE))
      || (fileMappings[2].maxProt != VM_PROT_READ) ) {
        results->errorMessage = "shared cache file mappings are invalid";
        ::close(fd);
        return false;
    }

    if ( cache->header.mappingOffset >= 0xF8 ) {
        if ( (fileMappings[0].address != cache->header.sharedRegionStart) || ((fileMappings[2].address + fileMappings[2].size) > (cache->header.sharedRegionStart+cache->header.sharedRegionSize)) ) {
            results->errorMessage = "shared cache file mapping addressses invalid";
            ::close(fd);
            return false;
        }
    }
    else {
        if ( (fileMappings[0].address != SHARED_REGION_BASE) || ((fileMappings[2].address + fileMappings[2].size) > (SHARED_REGION_BASE+SHARED_REGION_SIZE)) ) {
            results->errorMessage = "shared cache file mapping addressses invalid";
            ::close(fd);
            return false;
        }
    }

    // register code signature of cache file
    fsignatures_t siginfo;
    siginfo.fs_file_start = 0;  // cache always starts at beginning of file
    siginfo.fs_blob_start = (void*)cache->header.codeSignatureOffset;
    siginfo.fs_blob_size  = (size_t)(cache->header.codeSignatureSize);
    int result = fcntl(fd, F_ADDFILESIGS_RETURN, &siginfo);
    if ( result == -1 ) {
        results->errorMessage = "code signature registration for shared cache failed";
        ::close(fd);
        return false;
    }

    // <rdar://problem/23188073> validate code signature covers entire shared cache
    uint64_t codeSignedLength = siginfo.fs_file_start;
    if ( codeSignedLength < cache->header.codeSignatureOffset ) {
        results->errorMessage = "code signature does not cover entire shared cache file";
        ::close(fd);
        return false;
    }
    void* mappedData = ::mmap(NULL, sizeof(firstPage), PROT_READ|PROT_EXEC, MAP_PRIVATE, fd, 0);
    if ( mappedData == MAP_FAILED ) {
        results->errorMessage = "first page of shared cache not mmap()able";
        ::close(fd);
        return false;
    }
    if ( memcmp(mappedData, firstPage, sizeof(firstPage)) != 0 ) {
        results->errorMessage = "first page of mmap()ed shared cache not valid";
        ::close(fd);
        return false;
    }
    ::munmap(mappedData, sizeof(firstPage));

    // fill out results
    info->fd = fd;
    for (int i=0; i < 3; ++i) {
        info->mappings[i].sfm_address       = fileMappings[i].address;
        info->mappings[i].sfm_size          = fileMappings[i].size;
        info->mappings[i].sfm_file_offset   = fileMappings[i].fileOffset;
        info->mappings[i].sfm_max_prot      = fileMappings[i].maxProt;
        info->mappings[i].sfm_init_prot     = fileMappings[i].initProt;
    }
    info->mappings[1].sfm_max_prot  |= VM_PROT_SLIDE;
    info->mappings[1].sfm_init_prot |= VM_PROT_SLIDE;
    info->slideInfoAddressUnslid  = fileMappings[2].address + cache->header.slideInfoOffset - fileMappings[2].fileOffset;
    info->slideInfoSize           = (long)cache->header.slideInfoSize;
    if ( cache->header.mappingOffset >= 0xf8 ) {
        info->sharedRegionStart = cache->header.sharedRegionStart;
        info->sharedRegionSize  = cache->header.sharedRegionSize;
        info->maxSlide          = cache->header.maxSlide;
    }
    else {
        info->sharedRegionStart = SHARED_REGION_BASE;
        info->sharedRegionSize  = SHARED_REGION_SIZE;
        info->maxSlide          = SHARED_REGION_SIZE - (fileMappings[2].address + fileMappings[2].size - fileMappings[0].address);
    }
    return true;
}


#if !TARGET_OS_SIMULATOR

// update all __DATA pages with slide info
static bool rebaseDataPages(bool isVerbose, CacheInfo& info, SharedCacheLoadInfo* results)
{
    uint64_t dataPagesStart = info.mappings[1].sfm_address;
    const dyld_cache_slide_info* slideInfo = nullptr;
    if ( info.slideInfoSize != 0 ) {
        slideInfo = (dyld_cache_slide_info*)(info.slideInfoAddressUnslid + results->slide);
    }
    const dyld_cache_slide_info* slideInfoHeader = (dyld_cache_slide_info*)slideInfo;
    if ( slideInfoHeader != nullptr ) {
        if ( slideInfoHeader->version == 2 ) {
            const dyld_cache_slide_info2* slideHeader = (dyld_cache_slide_info2*)slideInfo;
            const uint32_t  page_size = slideHeader->page_size;
            const uint16_t* page_starts = (uint16_t*)((long)(slideInfo) + slideHeader->page_starts_offset);
            const uint16_t* page_extras = (uint16_t*)((long)(slideInfo) + slideHeader->page_extras_offset);
            for (int i=0; i < slideHeader->page_starts_count; ++i) {
                uint8_t* page = (uint8_t*)(long)(dataPagesStart + (page_size*i));
                uint16_t pageEntry = page_starts[i];
                //dyld::log("page[%d]: page_starts[i]=0x%04X\n", i, pageEntry);
                if ( pageEntry == DYLD_CACHE_SLIDE_PAGE_ATTR_NO_REBASE )
                    continue;
                if ( pageEntry & DYLD_CACHE_SLIDE_PAGE_ATTR_EXTRA ) {
                    uint16_t chainIndex = (pageEntry & 0x3FFF);
                    bool done = false;
                    while ( !done ) {
                        uint16_t pInfo = page_extras[chainIndex];
                        uint16_t pageStartOffset = (pInfo & 0x3FFF)*4;
                        //dyld::log("     chain[%d] pageOffset=0x%03X\n", chainIndex, pageStartOffset);
                        rebaseChainV2(page, pageStartOffset, results->slide, slideHeader);
                        done = (pInfo & DYLD_CACHE_SLIDE_PAGE_ATTR_END);
                        ++chainIndex;
                    }
                }
                else {
                    uint32_t pageOffset = pageEntry * 4;
                    //dyld::log("     start pageOffset=0x%03X\n", pageOffset);
                    rebaseChainV2(page, pageOffset, results->slide, slideHeader);
                }
            }
        }
#if __LP64__
        else if ( slideInfoHeader->version == 3 ) {
             const dyld_cache_slide_info3* slideHeader = (dyld_cache_slide_info3*)slideInfo;
             const uint32_t                pageSize    = slideHeader->page_size;
             for (int i=0; i < slideHeader->page_starts_count; ++i) {
                 uint8_t* page = (uint8_t*)(dataPagesStart + (pageSize*i));
                 uint64_t delta = slideHeader->page_starts[i];
                 if ( delta == DYLD_CACHE_SLIDE_V3_PAGE_ATTR_NO_REBASE )
                     continue;
                 delta = delta/sizeof(uint64_t); // initial offset is byte based
                 dyld_cache_slide_pointer3* loc = (dyld_cache_slide_pointer3*)page;
                 do {
                     loc += delta;
                     delta = loc->plain.offsetToNextPointer;
                     if ( loc->auth.authenticated ) {
#if __has_feature(ptrauth_calls)
                        uint64_t target = info.sharedRegionStart + loc->auth.offsetFromSharedCacheBase + results->slide;
                        MachOLoaded::ChainedFixupPointerOnDisk ptr;
                        ptr.raw64 = *((uint64_t*)loc);
                        loc->raw = ptr.arm64e.signPointer(loc, target);
#else
                        results->errorMessage = "invalid pointer kind in cache file";
                        return false;
#endif
                     }
                     else {
                        MachOLoaded::ChainedFixupPointerOnDisk ptr;
                        ptr.raw64 = *((uint64_t*)loc);
                        loc->raw = ptr.arm64e.unpackTarget() + results->slide;
                     }
                } while (delta != 0);
            }
        }
#else
        else if ( slideInfoHeader->version == 4 ) {
            const dyld_cache_slide_info4* slideHeader = (dyld_cache_slide_info4*)slideInfo;
            const uint32_t  page_size = slideHeader->page_size;
            const uint16_t* page_starts = (uint16_t*)((long)(slideInfo) + slideHeader->page_starts_offset);
            const uint16_t* page_extras = (uint16_t*)((long)(slideInfo) + slideHeader->page_extras_offset);
            for (int i=0; i < slideHeader->page_starts_count; ++i) {
                uint8_t* page = (uint8_t*)(long)(dataPagesStart + (page_size*i));
                uint16_t pageEntry = page_starts[i];
                //dyld::log("page[%d]: page_starts[i]=0x%04X\n", i, pageEntry);
                if ( pageEntry == DYLD_CACHE_SLIDE4_PAGE_NO_REBASE )
                    continue;
                if ( pageEntry & DYLD_CACHE_SLIDE4_PAGE_USE_EXTRA ) {
                    uint16_t chainIndex = (pageEntry & DYLD_CACHE_SLIDE4_PAGE_INDEX);
                    bool done = false;
                    while ( !done ) {
                        uint16_t pInfo = page_extras[chainIndex];
                        uint16_t pageStartOffset = (pInfo & DYLD_CACHE_SLIDE4_PAGE_INDEX)*4;
                        //dyld::log("     chain[%d] pageOffset=0x%03X\n", chainIndex, pageStartOffset);
                        rebaseChainV4(page, pageStartOffset, results->slide, slideHeader);
                        done = (pInfo & DYLD_CACHE_SLIDE4_PAGE_EXTRA_END);
                        ++chainIndex;
                    }
                }
                else {
                    uint32_t pageOffset = pageEntry * 4;
                    //dyld::log("     start pageOffset=0x%03X\n", pageOffset);
                    rebaseChainV4(page, pageOffset, results->slide, slideHeader);
                }
            }
        }
#endif // LP64
        else {
            results->errorMessage = "invalid slide info in cache file";
            return false;
        }
    }
    return true;
}

static bool reuseExistingCache(const SharedCacheOptions& options, SharedCacheLoadInfo* results)
{
    uint64_t cacheBaseAddress;
#if __i386__
    if ( syscall(294, &cacheBaseAddress) == 0 ) {
#else
    if ( __shared_region_check_np(&cacheBaseAddress) == 0 ) {
#endif
        const DyldSharedCache* existingCache = (DyldSharedCache*)cacheBaseAddress;
        if ( validMagic(options, existingCache) ) {
            const dyld_cache_mapping_info* const fileMappings = (dyld_cache_mapping_info*)(cacheBaseAddress + existingCache->header.mappingOffset);
            results->loadAddress = existingCache;
            results->slide = (long)(cacheBaseAddress - fileMappings[0].address);
            // we don't know the path this cache was previously loaded from, assume default
            getCachePath(options, sizeof(results->path), results->path);
            if ( options.verbose ) {
                const shared_file_mapping_np* const mappings = (shared_file_mapping_np*)(cacheBaseAddress + existingCache->header.mappingOffset);
                dyld::log("re-using existing shared cache (%s):\n", results->path);
                shared_file_mapping_np slidMappings[3];
                for (int i=0; i < 3; ++i) {
                    slidMappings[i] = mappings[i];
                    slidMappings[i].sfm_address += results->slide;
                }
                verboseSharedCacheMappings(slidMappings);
            }
        }
        else {
            results->errorMessage = "existing shared cache in memory is not compatible";
        }
        return true;
    }
    return false;
}

static long pickCacheASLR(CacheInfo& info)
{
    // choose new random slide
#if __IPHONE_OS_VERSION_MIN_REQUIRED
    // <rdar://problem/20848977> change shared cache slide for 32-bit arm to always be 16k aligned
    long slide = ((arc4random() % info.maxSlide) & (-16384));
#else
    long slide = ((arc4random() % info.maxSlide) & (-4096));
#endif

    // <rdar://problem/32031197> respect -disable_aslr boot-arg
    if ( BootArgs::contains("-disable_aslr") )
        slide = 0;

    // update mappings
    for (uint32_t i=0; i < 3; ++i) {
        info.mappings[i].sfm_address += slide;
    }
    
    return slide;
}

static bool mapCacheSystemWide(const SharedCacheOptions& options, SharedCacheLoadInfo* results)
{
    CacheInfo info;
    if ( !preflightCacheFile(options, results, &info) )
        return false;

    const dyld_cache_slide_info2* slideInfo = nullptr;
    if ( info.slideInfoSize != 0 ) {
        results->slide = pickCacheASLR(info);
        slideInfo = (dyld_cache_slide_info2*)(info.slideInfoAddressUnslid + results->slide);
    }

    int result = __shared_region_map_and_slide_np(info.fd, 3, info.mappings, results->slide, slideInfo, info.slideInfoSize);
    ::close(info.fd);
    if ( result == 0 ) {
        results->loadAddress = (const DyldSharedCache*)(info.mappings[0].sfm_address);
    }
    else {
        // could be another process beat us to it
        if ( reuseExistingCache(options, results) )
            return true;
        // if cache does not exist, then really is an error
        if ( results->errorMessage == nullptr )
            results->errorMessage = "syscall to map cache into shared region failed";
        return false;
    }

    if ( options.verbose ) {
        dyld::log("mapped dyld cache file system wide: %s\n", results->path);
        verboseSharedCacheMappings(info.mappings);
    }
    return true;
}
#endif // TARGET_OS_SIMULATOR

static bool mapCachePrivate(const SharedCacheOptions& options, SharedCacheLoadInfo* results)
{
    // open and validate cache file
    CacheInfo info;
    if ( !preflightCacheFile(options, results, &info) )
        return false;

    // compute ALSR slide
    results->slide = 0;
#if !TARGET_OS_SIMULATOR // simulator caches do not support sliding
    if ( info.slideInfoSize != 0 ) {
        results->slide = pickCacheASLR(info);
    }
#endif
    results->loadAddress = (const DyldSharedCache*)(info.mappings[0].sfm_address);

    // deallocate any existing system wide shared cache
    deallocateExistingSharedCache();

#if TARGET_OS_SIMULATOR && TARGET_OS_WATCH
    // <rdar://problem/50887685> watchOS 32-bit cache does not overlap macOS dyld cache address range
    // mmap() of a file needs a vm_allocation behind it, so make one
    vm_address_t loadAddress = 0x40000000;
    ::vm_allocate(mach_task_self(), &loadAddress, 0x40000000, VM_FLAGS_FIXED);
#endif

    // map cache just for this process with mmap()
    for (int i=0; i < 3; ++i) {
        void* mmapAddress = (void*)(uintptr_t)(info.mappings[i].sfm_address);
        size_t size = (size_t)(info.mappings[i].sfm_size);
        //dyld::log("dyld: mapping address %p with size 0x%08lX\n", mmapAddress, size);
        int protection = 0;
        if ( info.mappings[i].sfm_init_prot & VM_PROT_EXECUTE )
            protection   |= PROT_EXEC;
        if ( info.mappings[i].sfm_init_prot & VM_PROT_READ )
            protection   |= PROT_READ;
        if ( info.mappings[i].sfm_init_prot & VM_PROT_WRITE )
            protection   |= PROT_WRITE;
        off_t offset = info.mappings[i].sfm_file_offset;
        if ( ::mmap(mmapAddress, size, protection, MAP_FIXED | MAP_PRIVATE, info.fd, offset) != mmapAddress ) {
            // failed to map some chunk of this shared cache file
            // clear shared region
            ::mmap((void*)((long)SHARED_REGION_BASE), SHARED_REGION_SIZE, PROT_NONE, MAP_FIXED | MAP_PRIVATE| MAP_ANON, 0, 0);
            // return failure
            results->loadAddress        = nullptr;
            results->errorMessage       = "could not mmap() part of dyld cache";
            ::close(info.fd);
            return false;
        }
    }
    ::close(info.fd);

#if TARGET_OS_SIMULATOR // simulator caches do not support sliding
    return true;
#else
    bool success = rebaseDataPages(options.verbose, info, results);

    if ( options.verbose ) {
        dyld::log("mapped dyld cache file private to process (%s):\n", results->path);
        verboseSharedCacheMappings(info.mappings);
    }
    return success;
#endif
}



bool loadDyldCache(const SharedCacheOptions& options, SharedCacheLoadInfo* results)
{
    results->loadAddress        = 0;
    results->slide              = 0;
    results->errorMessage       = nullptr;

#if TARGET_OS_SIMULATOR
    // simulator only supports mmap()ing cache privately into process
    return mapCachePrivate(options, results);
#else
    if ( options.forcePrivate ) {
        // mmap cache into this process only
        return mapCachePrivate(options, results);
    }
    else {
        // fast path: when cache is already mapped into shared region
        bool hasError = false;
        if ( reuseExistingCache(options, results) ) {
            hasError = (results->errorMessage != nullptr);
        } else {
            // slow path: this is first process to load cache
            hasError = mapCacheSystemWide(options, results);
        }
        return hasError;
    }
#endif
}


bool findInSharedCacheImage(const SharedCacheLoadInfo& loadInfo, const char* dylibPathToFind, SharedCacheFindDylibResults* results)
{
    if ( loadInfo.loadAddress == nullptr )
        return false;

    if ( loadInfo.loadAddress->header.formatVersion != dyld3::closure::kFormatVersion ) {
        // support for older cache with a different Image* format
#if __IPHONE_OS_VERSION_MIN_REQUIRED
        uint64_t hash = 0;
        for (const char* s=dylibPathToFind; *s != '\0'; ++s)
                hash += hash*4 + *s;
#endif
        const dyld_cache_image_info* const start = (dyld_cache_image_info*)((uint8_t*)loadInfo.loadAddress + loadInfo.loadAddress->header.imagesOffset);
        const dyld_cache_image_info* const end = &start[loadInfo.loadAddress->header.imagesCount];
        for (const dyld_cache_image_info* p = start; p != end; ++p) {
#if __IPHONE_OS_VERSION_MIN_REQUIRED
            // on iOS, inode is used to hold hash of path
            if ( (p->modTime == 0) && (p->inode != hash) )
                continue;
#endif
            const char* aPath = (char*)loadInfo.loadAddress + p->pathFileOffset;
            if ( strcmp(aPath, dylibPathToFind) == 0 ) {
                results->mhInCache    = (const mach_header*)(p->address+loadInfo.slide);
                results->pathInCache  = aPath;
                results->slideInCache = loadInfo.slide;
                results->image        = nullptr;
                return true;
            }
        }
        return false;
    }

    const dyld3::closure::ImageArray* images = loadInfo.loadAddress->cachedDylibsImageArray();
    results->image = nullptr;
    uint32_t imageIndex;
    if ( loadInfo.loadAddress->hasImagePath(dylibPathToFind, imageIndex) ) {
        results->image = images->imageForNum(imageIndex+1);
    }
 #if __MAC_OS_X_VERSION_MIN_REQUIRED
    else {
        // <rdar://problem/32740215> handle symlink to cached dylib
        if ( loadInfo.loadAddress->header.dylibsExpectedOnDisk ) {
            struct stat statBuf;
            if ( dyld::my_stat(dylibPathToFind, &statBuf) == 0 ) {
                // on macOS we store the inode and mtime of each dylib in the cache in the dyld_cache_image_info array
                const dyld_cache_image_info* const start = (dyld_cache_image_info*)((uint8_t*)loadInfo.loadAddress + loadInfo.loadAddress->header.imagesOffset);
                const dyld_cache_image_info* const end = &start[loadInfo.loadAddress->header.imagesCount];
                for (const dyld_cache_image_info* p = start; p != end; ++p) {
                    if ( (p->inode == statBuf.st_ino) && (p->modTime == statBuf.st_mtime) ) {
                        imageIndex = (uint32_t)(p - start);
                        results->image = images->imageForNum(imageIndex+1);
                        break;
                    }
                }
            }
        }
        else {
            char resolvedPath[PATH_MAX];
            if ( realpath(dylibPathToFind, resolvedPath) != nullptr ) {
                if ( loadInfo.loadAddress->hasImagePath(resolvedPath, imageIndex) ) {
                    results->image = images->imageForNum(imageIndex+1);
                }
            }
        }
    }
#endif
    if ( results->image == nullptr )
        return false;

    results->mhInCache    = (const mach_header*)((uintptr_t)loadInfo.loadAddress + results->image->cacheOffset());
    results->pathInCache  = results->image->path();
    results->slideInCache = loadInfo.slide;
    return true;
}


bool pathIsInSharedCacheImage(const SharedCacheLoadInfo& loadInfo, const char* dylibPathToFind)
{
    if ( (loadInfo.loadAddress == nullptr) )
        return false;

    uint32_t imageIndex;
    return loadInfo.loadAddress->hasImagePath(dylibPathToFind, imageIndex);
}

void deallocateExistingSharedCache()
{
#if TARGET_OS_SIMULATOR
    // dyld deallocated macOS shared cache before jumping into dyld_sim
#else
    // <rdar://problem/5077374> remove the shared region sub-map
    uint64_t existingCacheAddress = 0;
    if ( __shared_region_check_np(&existingCacheAddress) == 0 ) {
        ::mmap((void*)((long)SHARED_REGION_BASE), SHARED_REGION_SIZE, PROT_NONE, MAP_FIXED | MAP_PRIVATE| MAP_ANON, 0, 0);
    }
#endif

}

} // namespace dyld3


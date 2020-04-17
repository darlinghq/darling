/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2009-2012 Apple Inc. All rights reserved.
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

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/syslimits.h>
#include <mach-o/arch.h>
#include <mach-o/loader.h>
#include <mach-o/dyld_priv.h>
#include <bootstrap.h>
#include <mach/mach.h>
#include <dispatch/dispatch.h>

#include <TargetConditionals.h>

#include <map>
#include <vector>
#include <iostream>

#include "DyldSharedCache.h"
#include "Trie.hpp"

#include "objc-shared-cache.h"

#if TARGET_OS_OSX
#define DSC_BUNDLE_REL_PATH "../../lib/dsc_extractor.bundle"
#else
#define DSC_BUNDLE_REL_PATH "../lib/dsc_extractor.bundle"
#endif

// mmap() an shared cache file read/only but laid out like it would be at runtime
static const DyldSharedCache* mapCacheFile(const char* path)
{
    struct stat statbuf;
    if ( ::stat(path, &statbuf) ) {
        fprintf(stderr, "Error: stat failed for dyld shared cache at %s\n", path);
        return nullptr;
    }
    
    int cache_fd = ::open(path, O_RDONLY);
    if (cache_fd < 0) {
        fprintf(stderr, "Error: failed to open shared cache file at %s\n", path);
        return nullptr;
    }
    
    uint8_t  firstPage[4096];
    if ( ::pread(cache_fd, firstPage, 4096, 0) != 4096 ) {
        fprintf(stderr, "Error: failed to read shared cache file at %s\n", path);
        return nullptr;
    }
    const dyld_cache_header*       header   = (dyld_cache_header*)firstPage;
    const dyld_cache_mapping_info* mappings = (dyld_cache_mapping_info*)(firstPage + header->mappingOffset);
    
    size_t vmSize = (size_t)(mappings[2].address + mappings[2].size - mappings[0].address);
    vm_address_t result;
    kern_return_t r = ::vm_allocate(mach_task_self(), &result, vmSize, VM_FLAGS_ANYWHERE);
    if ( r != KERN_SUCCESS ) {
        fprintf(stderr, "Error: failed to allocate space to load shared cache file at %s\n", path);
        return nullptr;
    }
    for (int i=0; i < 3; ++i) {
        void* mapped_cache = ::mmap((void*)(result + mappings[i].address - mappings[0].address), (size_t)mappings[i].size,
                                    PROT_READ, MAP_FIXED | MAP_PRIVATE, cache_fd, mappings[i].fileOffset);
        if (mapped_cache == MAP_FAILED) {
            fprintf(stderr, "Error: mmap() for shared cache at %s failed, errno=%d\n", path, errno);
            return nullptr;
        }
    }
    ::close(cache_fd);
    
    return (DyldSharedCache*)result;
}

enum Mode {
    modeNone,
    modeList,
    modeMap,
    modeDependencies,
    modeSlideInfo,
    modeVerboseSlideInfo,
    modeAcceleratorInfo,
    modeTextInfo,
    modeLinkEdit,
    modeLocalSymbols,
    modeJSONMap,
    modeJSONDependents,
    modeSectionSizes,
    modeStrings,
    modeInfo,
    modeSize,
    modeObjCProtocols,
    modeExtract
};

struct Options {
    Mode            mode;
    const char*     dependentsOfPath;
    const char*     extractionDir;
    bool            printUUIDs;
    bool            printVMAddrs;
    bool            printDylibVersions;
    bool            printInodes;
};


void usage() {
    fprintf(stderr, "Usage: dyld_shared_cache_util -list [ -uuid ] [-vmaddr] | -dependents <dylib-path> [ -versions ] | -linkedit | -map | -slide_info | -verbose_slide_info | -info | -extract <dylib-dir>  [ shared-cache-file ] \n");
}

static void checkMode(Mode mode) {
    if ( mode != modeNone ) {
        fprintf(stderr, "Error: select one of: -list, -dependents, -info, -slide_info, -verbose_slide_info, -linkedit, -map, -extract, or -size\n");
        usage();
        exit(1);
    }
}

static bool isAlias(const char* path, const DyldSharedCache* dyldCache) {
    const dyld_cache_header* header = &dyldCache->header;
    const dyld_cache_mapping_info* mappings = (const dyld_cache_mapping_info*)((char*)dyldCache + header->mappingOffset);
    const dyld_cache_mapping_info* textMapping = &mappings[0];
    // paths for aliases are store between cache header and first segment
    return path < (char*)textMapping;
}

int main (int argc, const char* argv[]) {

    const char* sharedCachePath = nullptr;

    Options options;
    options.mode = modeNone;
    options.printUUIDs = false;
    options.printVMAddrs = false;
    options.printDylibVersions = false;
    options.printInodes = false;
    options.dependentsOfPath = NULL;
    options.extractionDir = NULL;

    bool printStrings = false;
    bool printExports = false;

    for (uint32_t i = 1; i < argc; i++) {
        const char* opt = argv[i];
        if (opt[0] == '-') {
            if (strcmp(opt, "-list") == 0) {
                checkMode(options.mode);
                options.mode = modeList;
            }
            else if (strcmp(opt, "-dependents") == 0) {
                checkMode(options.mode);
                options.mode = modeDependencies;
                options.dependentsOfPath = argv[++i];
                if ( i >= argc ) {
                    fprintf(stderr, "Error: option -depdendents requires an argument\n");
                    usage();
                    exit(1);
                }
            }
            else if (strcmp(opt, "-linkedit") == 0) {
                checkMode(options.mode);
                options.mode = modeLinkEdit;
            }
            else if (strcmp(opt, "-info") == 0) {
                checkMode(options.mode);
                options.mode = modeInfo;
            }
            else if (strcmp(opt, "-slide_info") == 0) {
                checkMode(options.mode);
                options.mode = modeSlideInfo;
            }
            else if (strcmp(opt, "-verbose_slide_info") == 0) {
                checkMode(options.mode);
                options.mode = modeVerboseSlideInfo;
            }
            else if (strcmp(opt, "-accelerator_info") == 0) {
                checkMode(options.mode);
                options.mode = modeAcceleratorInfo;
            }
            else if (strcmp(opt, "-text_info") == 0) {
                checkMode(options.mode);
                options.mode = modeTextInfo;
            }
            else if (strcmp(opt, "-local_symbols") == 0) {
                checkMode(options.mode);
                options.mode = modeLocalSymbols;
            }
            else if (strcmp(opt, "-strings") == 0) {
                if (options.mode != modeStrings)
                    checkMode(options.mode);
                options.mode = modeStrings;
                printStrings = true;
            }
            else if (strcmp(opt, "-sections") == 0) {
                checkMode(options.mode);
                options.mode = modeSectionSizes;
            }
            else if (strcmp(opt, "-exports") == 0) {
                if (options.mode != modeStrings)
                    checkMode(options.mode);
                options.mode = modeStrings;
                printExports = true;
            }
            else if (strcmp(opt, "-map") == 0) {
                checkMode(options.mode);
                options.mode = modeMap;
            }
            else if (strcmp(opt, "-json-map") == 0) {
                checkMode(options.mode);
                options.mode = modeJSONMap;
            }
            else if (strcmp(opt, "-json-dependents") == 0) {
                checkMode(options.mode);
                options.mode = modeJSONDependents;
            }
            else if (strcmp(opt, "-size") == 0) {
                checkMode(options.mode);
                options.mode = modeSize;
            }
            else if (strcmp(opt, "-objc-protocols") == 0) {
                checkMode(options.mode);
                options.mode = modeObjCProtocols;
            }
            else if (strcmp(opt, "-extract") == 0) {
                checkMode(options.mode);
                options.mode = modeExtract;
                options.extractionDir = argv[++i];
                if ( i >= argc ) {
                    fprintf(stderr, "Error: option -extract requires a directory argument\n");
                    usage();
                    exit(1);
                }
            }
            else if (strcmp(opt, "-uuid") == 0) {
                options.printUUIDs = true;
            }
            else if (strcmp(opt, "-inode") == 0) {
                options.printInodes = true;
            }
            else if (strcmp(opt, "-versions") == 0) {
                options.printDylibVersions = true;
            }
            else if (strcmp(opt, "-vmaddr") == 0) {
                options.printVMAddrs = true;
            }
            else {
                fprintf(stderr, "Error: unrecognized option %s\n", opt);
                usage();
                exit(1);
            }
        }
        else {
            sharedCachePath = opt;
        }
    }

    if ( options.mode == modeNone ) {
        fprintf(stderr, "Error: select one of -list, -dependents, -info, -linkedit, or -map\n");
        usage();
        exit(1);
    }

    if ( options.mode != modeSlideInfo && options.mode != modeVerboseSlideInfo ) {
        if ( options.printUUIDs && (options.mode != modeList) )
            fprintf(stderr, "Warning: -uuid option ignored outside of -list mode\n");

        if ( options.printVMAddrs && (options.mode != modeList) )
            fprintf(stderr, "Warning: -vmaddr option ignored outside of -list mode\n");

        if ( options.printDylibVersions && (options.mode != modeDependencies) )
            fprintf(stderr, "Warning: -versions option ignored outside of -dependents mode\n");

        if ( (options.mode == modeDependencies) && (options.dependentsOfPath == NULL) ) {
            fprintf(stderr, "Error: -dependents given, but no dylib path specified\n");
            usage();
            exit(1);
        }
    }

    const DyldSharedCache* dyldCache = nullptr;
    if ( sharedCachePath != nullptr ) {
        dyldCache = mapCacheFile(sharedCachePath);
        // mapCacheFile prints an error if something goes wrong, so just return in that case.
        if ( dyldCache == nullptr )
            return 1;
    }
    else {
#if !defined(__MAC_OS_X_VERSION_MIN_REQUIRED) || (__MAC_OS_X_VERSION_MIN_REQUIRED >= 101300)
        size_t cacheLength;
        dyldCache = (DyldSharedCache*)_dyld_get_shared_cache_range(&cacheLength);
#endif
        if (dyldCache == nullptr) {
            fprintf(stderr, "Could not get in-memory shared cache\n");
            return 1;
        }
    }

    if ( options.mode == modeSlideInfo || options.mode == modeVerboseSlideInfo ) {
        const dyld_cache_header* header = &dyldCache->header;
        if ( header->slideInfoOffset == 0 ) {
            fprintf(stderr, "Error: dyld shared cache does not contain slide info\n");
            exit(1);
        }
        const dyld_cache_mapping_info* mappings = (const dyld_cache_mapping_info*)((char*)dyldCache + header->mappingOffset);
        const dyld_cache_mapping_info* dataMapping = &mappings[1];
        uint64_t dataStartAddress = dataMapping->address;
        uint64_t dataSize = dataMapping->size;

        const dyld_cache_slide_info* slideInfoHeader = dyldCache->slideInfo();
        printf("slide info version=%d\n", slideInfoHeader->version);
        if ( slideInfoHeader->version == 1 ) {
            printf("toc_count=%d, data page count=%lld\n", slideInfoHeader->toc_count, dataSize/4096);
            const dyld_cache_slide_info_entry* entries = (dyld_cache_slide_info_entry*)((char*)slideInfoHeader + slideInfoHeader->entries_offset);
            const uint16_t* tocs = (uint16_t*)((char*)slideInfoHeader + slideInfoHeader->toc_offset);
            for(int i=0; i < slideInfoHeader->toc_count; ++i) {
                printf("0x%08llX: [% 5d,% 5d] ", dataStartAddress + i*4096, i, tocs[i]);
                const dyld_cache_slide_info_entry* entry = &entries[tocs[i]];
                for(int j=0; j < slideInfoHeader->entries_size; ++j)
                    printf("%02X", entry->bits[j]);
                printf("\n");
            }
        }
        else if ( slideInfoHeader->version == 2 ) {
            const dyld_cache_slide_info2* slideInfo = (dyld_cache_slide_info2*)(slideInfoHeader);
            printf("page_size=%d\n", slideInfo->page_size);
            printf("delta_mask=0x%016llX\n", slideInfo->delta_mask);
            printf("value_add=0x%016llX\n", slideInfo->value_add);
            printf("page_starts_count=%d, page_extras_count=%d\n", slideInfo->page_starts_count, slideInfo->page_extras_count);
            const uint16_t* starts = (uint16_t* )((char*)slideInfo + slideInfo->page_starts_offset);
            const uint16_t* extras = (uint16_t* )((char*)slideInfo + slideInfo->page_extras_offset);
            for (int i=0; i < slideInfo->page_starts_count; ++i) {
                const uint16_t start = starts[i];
                auto rebaseChain = [&](uint8_t* pageContent, uint16_t startOffset)
                {
                    uintptr_t slideAmount = 0;
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
                        printf("    [% 5d + 0x%04llX]: 0x%016llX = 0x%016llX\n", i, (uint64_t)(pageOffset), (uint64_t)rawValue, (uint64_t)value);
                        pageOffset += delta;
                    }
                };
                const uint8_t* dataPagesStart = dyldCache->dataRegionStart();
                if ( start == DYLD_CACHE_SLIDE_PAGE_ATTR_NO_REBASE ) {
                    printf("page[% 5d]: no rebasing\n", i);
                }
                else if ( start & DYLD_CACHE_SLIDE_PAGE_ATTR_EXTRA ) {
                    printf("page[% 5d]: ", i);
                    int j=(start & 0x3FFF);
                    bool done = false;
                    do {
                        uint16_t aStart = extras[j];
                        printf("start=0x%04X ", aStart & 0x3FFF);
                        if ( options.mode == modeVerboseSlideInfo ) {
                            uint8_t* page = (uint8_t*)(long)(dataPagesStart + (slideInfo->page_size*i));
                            uint16_t pageStartOffset = (aStart & 0x3FFF)*4;
                            rebaseChain(page, pageStartOffset);
                        }
                        done = (extras[j] & DYLD_CACHE_SLIDE_PAGE_ATTR_END);
                        ++j;
                    } while ( !done );
                    printf("\n");
                }
                else {
                    printf("page[% 5d]: start=0x%04X\n", i, starts[i]);
                    if ( options.mode == modeVerboseSlideInfo ) {
                        uint8_t* page = (uint8_t*)(long)(dataPagesStart + (slideInfo->page_size*i));
                        uint16_t pageStartOffset = start*4;
                        rebaseChain(page, pageStartOffset);
                    }
                }
            }
        }
        else if ( slideInfoHeader->version == 3 ) {
            const dyld_cache_slide_info3* slideInfo = (dyld_cache_slide_info3*)(slideInfoHeader);
            printf("page_size=%d\n", slideInfo->page_size);
            printf("page_starts_count=%d\n", slideInfo->page_starts_count);
            printf("auth_value_add=0x%016llX\n", slideInfo->auth_value_add);
            const uintptr_t authValueAdd = (uintptr_t)(slideInfo->auth_value_add);
            const uint8_t* dataSegmentStart = dyldCache->dataRegionStart();
            for (int i=0; i < slideInfo->page_starts_count; ++i) {
                uint16_t delta = slideInfo->page_starts[i];
                if ( delta == DYLD_CACHE_SLIDE_V3_PAGE_ATTR_NO_REBASE ) {
                    printf("page[% 5d]: no rebasing\n", i);
                    continue;
                }

                printf("page[% 5d]: start=0x%04X\n", i, delta);
                if ( options.mode != modeVerboseSlideInfo )
                    continue;

                delta = delta/sizeof(uint64_t); // initial offset is byte based
                const uint8_t* pageStart = dataSegmentStart + (i * slideInfo->page_size);
                const dyld_cache_slide_pointer3* loc = (dyld_cache_slide_pointer3*)pageStart;
                do {
                    loc += delta;
                    delta = loc->plain.offsetToNextPointer;
                    dyld3::MachOLoaded::ChainedFixupPointerOnDisk ptr;
                    ptr.raw64 = *((uint64_t*)loc);
                    if ( loc->auth.authenticated ) {
                        uint64_t target = authValueAdd + loc->auth.offsetFromSharedCacheBase;
                        uint64_t targetValue = ptr.arm64e.signPointer((void*)loc, target);
                        printf("    [% 5d + 0x%04llX]: 0x%016llX (JOP: diversity %d, address %s, %s)\n",
                               i, (uint64_t)((const uint8_t*)loc - pageStart), targetValue,
                               ptr.arm64e.authBind.diversity, ptr.arm64e.authBind.addrDiv ? "true" : "false",
                               ptr.arm64e.keyName());
                    }
                    else {
                        uint64_t targetValue = ptr.arm64e.unpackTarget();
                        printf("    [% 5d + 0x%04llX]: 0x%016llX\n", i, (uint64_t)((const uint8_t*)loc - pageStart), targetValue);
                    }
                } while (delta != 0);
            }
        }
        else if ( slideInfoHeader->version == 4 ) {
            const dyld_cache_slide_info4* slideInfo = (dyld_cache_slide_info4*)(slideInfoHeader);
            printf("page_size=%d\n", slideInfo->page_size);
            printf("delta_mask=0x%016llX\n", slideInfo->delta_mask);
            printf("value_add=0x%016llX\n", slideInfo->value_add);
            printf("page_starts_count=%d, page_extras_count=%d\n", slideInfo->page_starts_count, slideInfo->page_extras_count);
            const uint16_t* starts = (uint16_t* )((char*)slideInfo + slideInfo->page_starts_offset);
            const uint16_t* extras = (uint16_t* )((char*)slideInfo + slideInfo->page_extras_offset);
            for (int i=0; i < slideInfo->page_starts_count; ++i) {
                const uint16_t start = starts[i];
                auto rebaseChainV4 = [&](uint8_t* pageContent, uint16_t startOffset)
                {
                    uintptr_t slideAmount = 0;
                    const uintptr_t   deltaMask    = (uintptr_t)(slideInfo->delta_mask);
                    const uintptr_t   valueMask    = ~deltaMask;
                    const uintptr_t   valueAdd     = (uintptr_t)(slideInfo->value_add);
                    const unsigned    deltaShift   = __builtin_ctzll(deltaMask) - 2;

                    uint32_t pageOffset = startOffset;
                    uint32_t delta = 1;
                    while ( delta != 0 ) {
                        uint8_t* loc = pageContent + pageOffset;
                        uint32_t rawValue = *((uint32_t*)loc);
                        delta = (uint32_t)((rawValue & deltaMask) >> deltaShift);
                        uintptr_t value = (rawValue & valueMask);
                        if ( (value & 0xFFFF8000) == 0 ) {
                            // small positive non-pointer, use as-is
                        }
                        else if ( (value & 0x3FFF8000) == 0x3FFF8000 ) {
                            // small negative non-pointer
                            value |= 0xC0000000;
                        }
                        else  {
                            value += valueAdd;
                            value += slideAmount;
                        }
                        printf("    [% 5d + 0x%04X]: 0x%08X\n", i, pageOffset, rawValue);
                        pageOffset += delta;
                    }
                };
                const uint8_t* dataPagesStart = dyldCache->dataRegionStart();
                if ( start == DYLD_CACHE_SLIDE4_PAGE_NO_REBASE ) {
                    printf("page[% 5d]: no rebasing\n", i);
                }
                else if ( start & DYLD_CACHE_SLIDE4_PAGE_USE_EXTRA ) {
                    printf("page[% 5d]: ", i);
                    int j=(start & DYLD_CACHE_SLIDE4_PAGE_INDEX);
                    bool done = false;
                    do {
                        uint16_t aStart = extras[j];
                        printf("start=0x%04X ", aStart & DYLD_CACHE_SLIDE4_PAGE_INDEX);
                        if ( options.mode == modeVerboseSlideInfo ) {
                            uint8_t* page = (uint8_t*)(long)(dataPagesStart + (slideInfo->page_size*i));
                            uint16_t pageStartOffset = (aStart & DYLD_CACHE_SLIDE4_PAGE_INDEX)*4;
                            rebaseChainV4(page, pageStartOffset);
                        }
                        done = (extras[j] & DYLD_CACHE_SLIDE4_PAGE_EXTRA_END);
                        ++j;
                    } while ( !done );
                    printf("\n");
                }
                else {
                    printf("page[% 5d]: start=0x%04X\n", i, starts[i]);
                    if ( options.mode == modeVerboseSlideInfo ) {
                        uint8_t* page = (uint8_t*)(long)(dataPagesStart + (slideInfo->page_size*i));
                        uint16_t pageStartOffset = start*4;
                        rebaseChainV4(page, pageStartOffset);
                    }
                }
            }
        }
    }
    else if ( options.mode == modeInfo ) {
        const dyld_cache_header* header = &dyldCache->header;
        printf("uuid: ");
        if ( header->mappingOffset >= 0x68 ) {
            const uint8_t* uuid = header->uuid;
            printf("%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                   uuid[0],  uuid[1],  uuid[2],  uuid[3],
                   uuid[4],  uuid[5],  uuid[6],  uuid[7],
                   uuid[8],  uuid[9],  uuid[10], uuid[11],
                   uuid[12], uuid[13], uuid[14], uuid[15]);
        }
        else {
            printf("n/a\n");
        }
        if ( header->mappingOffset >= 0xE0 ) {
            // HACK until this uses new header
            uint32_t platform = *((uint32_t*)(((char*)header) + 0xD8));
            uint32_t bitfield = *((uint32_t*)(((char*)header) + 0xDC));
            uint32_t simulator = bitfield & 0x200;
            uint32_t locallyBuiltCache = bitfield & 0x400;
            switch (platform) {
                case 1:
                    printf("platform: macOS\n");
                    break;
                case 2:
                    if ( simulator )
                        printf("platform: iOS simulator\n");
                    else
                        printf("platform: iOS\n");
                    break;
                case 3:
                    if ( simulator )
                        printf("platform: tvOS simulator\n");
                    else
                        printf("platform: tvOS\n");
                    break;
                case 4:
                    if ( simulator )
                        printf("platform: watchOS simulator\n");
                    else
                        printf("platform: watchOS\n");
                    break;
                case 5:
                    printf("platform: bridgeOS\n");
                    break;
                default:
                    printf("platform: 0x%08X 0x%08X\n", platform, simulator);
            }
            printf("built by: %s\n", locallyBuiltCache ? "local machine" : "B&I");
        }
        printf("cache type: %s\n", header->cacheType ? "production" : "development");
        printf("image count: %u\n", header->imagesCount);
        if ( (header->mappingOffset >= 0x78) && (header->branchPoolsOffset != 0) ) {
            printf("branch pool count:  %u\n", header->branchPoolsCount);
        }
        if ( header->slideInfoSize > 0 ) {
            uint32_t pageSize            = 0x4000; // fix me for intel
            uint32_t possibleSlideValues = (uint32_t)(header->maxSlide/pageSize);
            uint32_t entropyBits         = 32 - __builtin_clz(possibleSlideValues - 1);
            printf("ASLR entropy: %u-bits\n", entropyBits);
        }
        printf("mappings:\n");
        const dyld_cache_mapping_info* mappings = (const dyld_cache_mapping_info*)((char*)dyldCache + header->mappingOffset);
        for (uint32_t i=0; i < header->mappingCount; ++i) {
            if ( mappings[i].initProt & VM_PROT_EXECUTE )
                printf("    __TEXT      %3lluMB,  file offset: 0x%08llX -> 0x%08llX,  address: 0x%08llX -> 0x%08llX\n",
                       mappings[i].size/(1024*1024), mappings[i].fileOffset, mappings[i].fileOffset + mappings[i].size,
                       mappings[i].address, mappings[i].address + mappings[i].size);
            else if ( mappings[i].initProt & VM_PROT_WRITE )
                printf("    __DATA      %3lluMB,  file offset: 0x%08llX -> 0x%08llX,  address: 0x%08llX -> 0x%08llX\n",
                       mappings[i].size/(1024*1024), mappings[i].fileOffset, mappings[i].fileOffset + mappings[i].size,
                       mappings[i].address, mappings[i].address + mappings[i].size);
            else if ( mappings[i].initProt & VM_PROT_READ )
                printf("    __LINKEDIT  %3lluMB,  file offset: 0x%08llX -> 0x%08llX,  address: 0x%08llX -> 0x%08llX\n",
                       mappings[i].size/(1024*1024), mappings[i].fileOffset, mappings[i].fileOffset + mappings[i].size,
                       mappings[i].address, mappings[i].address + mappings[i].size);
        }
        if ( header->codeSignatureOffset != 0 ) {
            uint64_t size = header->codeSignatureSize;
            uint64_t csAddr = mappings[header->mappingCount-1].address + mappings[header->mappingCount-1].size;
            if ( size != 0 )
                printf("    code sign   %3lluMB,  file offset: 0x%08llX -> 0x%08llX,  address: 0x%08llX -> 0x%08llX\n",
                       size/(1024*1024), header->codeSignatureOffset, header->codeSignatureOffset + size, csAddr, csAddr + size);
        }
        printf("slide info:    %4lluKB,  file offset: 0x%08llX -> 0x%08llX\n",
               header->slideInfoSize/1024, header->slideInfoOffset, header->slideInfoOffset + header->slideInfoSize);
        if ( header->localSymbolsOffset != 0 )
            printf("local symbols:  %3lluMB,  file offset: 0x%08llX -> 0x%08llX\n",
                   header->localSymbolsSize/(1024*1024), header->localSymbolsOffset, header->localSymbolsOffset + header->localSymbolsSize);
        if ( (header->mappingOffset >= 0x78) && (header->accelerateInfoSize != 0) )
            printf("accelerate tab: %3lluKB,                                          address: 0x%08llX -> 0x%08llX\n",
                   header->accelerateInfoSize/1024, header->accelerateInfoAddr, header->accelerateInfoAddr + header->accelerateInfoSize);
    }
    else if ( options.mode == modeAcceleratorInfo ) {
        const dyld_cache_header* header = &dyldCache->header;
        if ( (header->mappingOffset < sizeof(dyld_cache_header)) || (header->accelerateInfoSize == 0) ) {
            printf("no accelerator info\n");
        }
        else {
            const dyld_cache_mapping_info* mappings = (const dyld_cache_mapping_info*)((char*)dyldCache + header->mappingOffset);
            uint64_t aiAddr = header->accelerateInfoAddr;
            const dyld_cache_accelerator_info* accelInfo = NULL;
            for (uint32_t i=0; i < header->mappingCount; ++i) {
                if ( (mappings[i].address <= aiAddr) && (aiAddr < mappings[i].address+mappings[i].size) ) {
                    uint64_t offset = aiAddr - mappings[i].address + mappings[i].fileOffset;
                    accelInfo = (dyld_cache_accelerator_info*)((uint8_t*)dyldCache + offset);
                }
            }
            if ( accelInfo == NULL ) {
                printf("accelerator info not in any mapped range\n");
            }
            else {
                const dyld_cache_image_info* images = (dyld_cache_image_info*)((char*)dyldCache + header->imagesOffset);
                const dyld_cache_image_info_extra* imagesExtra = (dyld_cache_image_info_extra*)((char*)accelInfo + accelInfo->imagesExtrasOffset);
                const uint16_t* dependencyArray = (uint16_t*)((char*)accelInfo + accelInfo->depListOffset);
                const uint16_t* reExportArray = (uint16_t*)((char*)accelInfo + accelInfo->reExportListOffset);
                printf("extra image info (count=%u):\n", accelInfo->imageExtrasCount);
                for (uint32_t i=0; i < accelInfo->imageExtrasCount; ++i) {
                    printf("  image[%3u] %s:\n", i, (char*)dyldCache +images[i].pathFileOffset);
                    printf("     exports trie: addr=0x%llX, size=0x%08X\n", imagesExtra[i].exportsTrieAddr, imagesExtra[i].exportsTrieSize);
                    if ( imagesExtra[i].weakBindingsSize )
                        printf("     weak bind info: addr=0x%llX, size=0x%08X\n", imagesExtra[i].weakBindingsAddr, imagesExtra[i].weakBindingsSize);
                    printf("     dependents: ");
                    for (uint32_t d=imagesExtra[i].dependentsStartArrayIndex; dependencyArray[d] != 0xFFFF; ++d) {
                        uint16_t depIndex = dependencyArray[d];
                        if ( depIndex & 0x8000 )
                            printf(" up(%d) ", depIndex & 0x7FFF);
                        else
                            printf(" %d ", depIndex);
                    }
                    printf("\n");
                    printf("     re-exports: ");
                    for (uint32_t r=imagesExtra[i].reExportsStartArrayIndex; reExportArray[r] != 0xFFFF; ++r)
                        printf(" %d ", reExportArray[r]);
                    printf("\n");
                }
                printf("libdyld.dylib:\n");
                printf("   __dyld section address: 0x%llX\n", accelInfo->dyldSectionAddr);
                printf("initializers (count=%u):\n", accelInfo->initializersCount);
                const dyld_cache_accelerator_initializer* initializers = (dyld_cache_accelerator_initializer*)((char*)accelInfo + accelInfo->initializersOffset);
                for (uint32_t i=0; i < accelInfo->initializersCount; ++i) {
                    printf("  image[%3u] 0x%llX\n", initializers[i].imageIndex, mappings[0].address + initializers[i].functionOffset);
                }
                printf("DOF sections (count=%u):\n", accelInfo->dofSectionsCount);
                const dyld_cache_accelerator_dof* dofs = (dyld_cache_accelerator_dof*)((char*)accelInfo + accelInfo->dofSectionsOffset);
                for (uint32_t i=0; i < accelInfo->dofSectionsCount; ++i) {
                    printf("  image[%3u] 0x%llX -> 0x%llX\n", dofs[i].imageIndex, dofs[i].sectionAddress, dofs[i].sectionAddress+dofs[i].sectionSize);
                }
                printf("bottom up order (count=%u):\n", accelInfo->imageExtrasCount);
                const uint16_t* bottomUpArray = (uint16_t*)((char*)accelInfo + accelInfo->bottomUpListOffset);
                for (uint32_t i=0; i < accelInfo->imageExtrasCount; ++i) {
                    unsigned imageIndex = bottomUpArray[i];
                    if ( imageIndex < accelInfo->imageExtrasCount )
                        printf("  image[%3u] %s\n", imageIndex, (char*)dyldCache + images[imageIndex].pathFileOffset);
                    else
                        printf("  image[%3u] BAD INDEX\n", imageIndex);
                }
                printf("range table (count=%u):\n", accelInfo->rangeTableCount);
                const dyld_cache_range_entry* rangeTable = (dyld_cache_range_entry*)((char*)accelInfo + accelInfo->rangeTableOffset);
                for (uint32_t i=0; i < accelInfo->rangeTableCount; ++i) {
                    const dyld_cache_range_entry& entry = rangeTable[i];
                    printf("  0x%llX -> 0x%llX %s\n", entry.startAddress, entry.startAddress + entry.size, (char*)dyldCache + images[entry.imageIndex].pathFileOffset);
                }
                printf("dylib trie (size=%u):\n", accelInfo->dylibTrieSize);
                const uint8_t* dylibTrieStart = (uint8_t*)accelInfo + accelInfo->dylibTrieOffset;
                const uint8_t* dylibTrieEnd = dylibTrieStart + accelInfo->dylibTrieSize;
                std::vector<DylibIndexTrie::Entry> dylibEntries;
                if ( !Trie<DylibIndex>::parseTrie(dylibTrieStart, dylibTrieEnd, dylibEntries) )
                    printf("  malformed dylibs trie\n");
                for (const DylibIndexTrie::Entry& x : dylibEntries) {
                    printf("  image[%3u] %s\n", x.info.index, x.name.c_str());
                }
            }
        }
    }
    else if ( options.mode == modeTextInfo ) {
        const dyld_cache_header* header = &dyldCache->header;
        if ( (header->mappingOffset < sizeof(dyld_cache_header)) || (header->imagesTextCount == 0) ) {
            printf("no text info\n");
        }
        else {
            const dyld_cache_image_text_info* imagesText = (dyld_cache_image_text_info*)((char*)dyldCache + header->imagesTextOffset);
            const dyld_cache_image_text_info* imagesTextEnd = &imagesText[header->imagesTextCount];
            printf("dylib text infos (count=%llu):\n", header->imagesTextCount);
            for (const dyld_cache_image_text_info* p=imagesText; p < imagesTextEnd; ++p) {
                printf("   0x%09llX -> 0x%09llX  <", p->loadAddress, p->loadAddress + p->textSegmentSize);
                for (int i=0; i<16; ++i) {
                    switch (i) {
                        case 4:
                        case 6:
                        case 8:
                        case 10:
                            printf("-");
                            break;
                    }
                    printf("%02X", p->uuid[i]);
                }
                printf(">  %s\n", (char*)dyldCache + p->pathOffset);
            }
        }
    }
    else if ( options.mode == modeLocalSymbols ) {
        const dyld_cache_header* header = &dyldCache->header;
        if ( header->localSymbolsOffset == 0 ) {
            fprintf(stderr, "Error: dyld shared cache does not contain local symbols info\n");
            exit(1);
        }
        const bool is64 = (strstr((char*)dyldCache, "64") != NULL);
        const dyld_cache_image_info* imageInfos = (dyld_cache_image_info*)((char*)dyldCache + header->imagesOffset);
        const dyld_cache_local_symbols_info* localsInfo = (dyld_cache_local_symbols_info*)((char*)dyldCache + header->localSymbolsOffset);
        const uint32_t nlistFileOffset = (uint32_t)(header->localSymbolsOffset + localsInfo->nlistOffset);
        const uint32_t nlistCount = localsInfo->nlistCount;
        const uint32_t nlistByteSize = is64 ? nlistCount*16 : nlistCount*12;
        const uint32_t stringsFileOffset = (uint32_t)(header->localSymbolsOffset + localsInfo->stringsOffset);
        const uint32_t stringsSize = localsInfo->stringsSize;
        const uint32_t entriesCount = localsInfo->entriesCount;
        const dyld_cache_local_symbols_entry* entries = (dyld_cache_local_symbols_entry*)((char*)localsInfo + localsInfo->entriesOffset);
        printf("local symbols nlist array:  %3uMB,  file offset: 0x%08X -> 0x%08X\n", nlistByteSize/(1024*1024), nlistFileOffset, nlistFileOffset+nlistByteSize);
        printf("local symbols string pool:  %3uMB,  file offset: 0x%08X -> 0x%08X\n", stringsSize/(1024*1024), stringsFileOffset, stringsFileOffset+stringsSize);
        printf("local symbols by dylib (count=%d):\n", entriesCount);
        //const char* stringPool = (char*)dyldCache + stringsFileOffset;
        for (int i=0; i < entriesCount; ++i) {
            const char* imageName = (char*)dyldCache + imageInfos[i].pathFileOffset;
            printf("   nlistStartIndex=%5d, nlistCount=%5d, image=%s\n", entries[i].nlistStartIndex, entries[i].nlistCount, imageName);
#if 0
            if ( is64 ) {
                const nlist_64* symTab = (nlist_64*)((char*)dyldCache + nlistFileOffset);
                for (int e=0; e < entries[i].nlistCount(); ++e) {
                    const nlist_64* entry = &symTab[entries[i].nlistStartIndex()+e];
                    printf("     nlist[%d].str=%d, %s\n", e, entry->n_un.n_strx, &stringPool[entry->n_un.n_strx]);
                    printf("     nlist[%d].value=0x%0llX\n", e, entry->n_value);
                }
            }
#endif
        }
    }
    else if ( options.mode == modeJSONMap ) {
        std::string buffer = dyldCache->generateJSONMap("unknown");
        printf("%s\n", buffer.c_str());
    }
    else if ( options.mode == modeJSONDependents ) {
        std::cout <<  dyldCache->generateJSONDependents();
    }
    else if ( options.mode == modeStrings ) {
        if (printStrings) {
            dyldCache->forEachImage(^(const mach_header *mh, const char *installName) {
                const dyld3::MachOAnalyzer* ma = (dyld3::MachOAnalyzer*)mh;
                int64_t slide = ma->getSlide();
                ma->forEachSection(^(const dyld3::MachOAnalyzer::SectionInfo& info, bool malformedSectionRange, bool& stop) {
                    if ( ( (info.sectFlags & SECTION_TYPE) == S_CSTRING_LITERALS ) ) {
                        if ( malformedSectionRange ) {
                            stop = true;
                            return;
                        }
                        const uint8_t* content = (uint8_t*)(info.sectAddr + slide);
                        const char* s   = (char*)content;
                        const char* end = s + info.sectSize;
                        while ( s < end ) {
                            printf("%s: %s\n", ma->installName(), s);
                            while (*s != '\0' )
                                ++s;
                            ++s;
                        }
                    }
                });
            });
        }

        if (printExports) {
            dyldCache->forEachImage(^(const mach_header *mh, const char *installName) {
                const dyld3::MachOAnalyzer* ma = (dyld3::MachOAnalyzer*)mh;
                uint32_t exportTrieRuntimeOffset;
                uint32_t exportTrieSize;
                if ( ma->hasExportTrie(exportTrieRuntimeOffset, exportTrieSize) ) {
                    const uint8_t* start = (uint8_t*)mh + exportTrieRuntimeOffset;
                    const uint8_t* end = start + exportTrieSize;
                    std::vector<ExportInfoTrie::Entry> exports;
                    if ( !ExportInfoTrie::parseTrie(start, end, exports) ) {
                        return;
                    }

                    for (const ExportInfoTrie::Entry& entry: exports) {
                        printf("%s: %s\n", ma->installName(), entry.name.c_str());
                    }
                }
            });
        }
    }
    else if ( options.mode == modeSectionSizes ) {
        __block std::map<std::string, uint64_t> sectionSizes;
        dyldCache->forEachImage(^(const mach_header *mh, const char *installName) {
            const dyld3::MachOAnalyzer* ma = (const dyld3::MachOAnalyzer*)mh;
            ma->forEachSection(^(const dyld3::MachOAnalyzer::SectionInfo &sectInfo, bool malformedSectionRange, bool &stop) {
                std::string section = std::string(sectInfo.segInfo.segName) + " " + sectInfo.sectName;
                sectionSizes[section] += sectInfo.sectSize;
            });
        });
        for (const auto& keyAndValue : sectionSizes) {
            printf("%lld %s\n", keyAndValue.second, keyAndValue.first.c_str());
        }
    }
    else if ( options.mode == modeObjCProtocols ) {
        if ( dyldCache->objcOpt() == nullptr ) {
            fprintf(stderr, "Error: could not get optimized objc\n");
            return 1;
        }
        objc_opt::objc_protocolopt2_t* protocols = dyldCache->objcOpt()->protocolopt2();
        if ( protocols == nullptr ) {
            fprintf(stderr, "Error: could not get optimized objc protocols\n");
            return 1;
        }

        for (uint64_t index = 0; index != protocols->capacity; ++index) {
            const objc_opt::objc_classheader_t& clshi = protocols->classOffsets()[index];
            if ( clshi.clsOffset == 0 ) {
                fprintf(stderr, "[% 5lld]\n", index);
                continue;
            }
            const char* name = (const char*)(((const uint8_t*)protocols) + protocols->offsets()[index]);
            if ( !clshi.isDuplicate() ) {
                fprintf(stderr, "[% 5lld] -> (% 8d, % 8d) = %s\n", index, clshi.clsOffset, clshi.hiOffset, name);
                continue;
            }

            // class appears in more than one header
            uint32_t count = clshi.duplicateCount();
            fprintf(stderr, "[% 5lld] -> duplicates [% 5d..% 5d] = %s\n",
                    index, clshi.duplicateIndex(), clshi.duplicateIndex() + clshi.duplicateCount() - 1, name);

            const objc_opt::objc_classheader_t *list = &protocols->duplicateOffsets()[clshi.duplicateIndex()];
            for (uint32_t i = 0; i < count; i++) {
                fprintf(stderr, "  - [% 5lld] -> (% 8d, % 8d)\n", (uint64_t)(clshi.duplicateIndex() + i), list[i].clsOffset, list[i].hiOffset);
            }
        }
    }
    else if ( options.mode == modeExtract ) {
        char pathBuffer[PATH_MAX];
        uint32_t bufferSize = PATH_MAX;
        if ( _NSGetExecutablePath(pathBuffer, &bufferSize) != 0 ) {
            fprintf(stderr, "Error: could not get path of program\n");
            return 1;
        }
        char* last = strrchr(pathBuffer, '/');
        // The bundle is at a different location on device.  Its /usr/lib/dsc_extractor.bundle in the SDK
        // but /usr/local/lib/dsc_extractor.bundle on device.
        strcpy(last+1, DSC_BUNDLE_REL_PATH);
        void* handle = dlopen(pathBuffer, RTLD_LAZY);
        if ( handle == NULL ) {
            fprintf(stderr, "Error: dsc_extractor.bundle could not be loaded at %s\n", pathBuffer);
            return 1;
        }

        typedef int (*extractor_proc)(const char* shared_cache_file_path, const char* extraction_root_path,
                                      void (^progress)(unsigned current, unsigned total));

        extractor_proc proc = (extractor_proc)dlsym(handle, "dyld_shared_cache_extract_dylibs_progress");
        if ( proc == NULL ) {
            fprintf(stderr, "Error: dsc_extractor.bundle did not have dyld_shared_cache_extract_dylibs_progress symbol\n");
            return 1;
        }

        int result = (*proc)(sharedCachePath, options.extractionDir, ^(unsigned c, unsigned total) { } );
        return result;
    }
    else {
        switch ( options.mode ) {
            case modeList: {
                if (options.printInodes) {
                    dyldCache->forEachImageEntry(^(const char* path, uint64_t mTime, uint64_t inode) {
                        printf("0x%08llX 0x%08llX ", inode, mTime);
                        if ( isAlias(path, dyldCache) )
                            printf("[alias] %s\n", path);
                        else
                            printf("%s\n", path);
                    });
                } else {
                    dyldCache->forEachImageTextSegment(^(uint64_t loadAddressUnslid, uint64_t textSegmentSize, const unsigned char *dylibUUID, const char *installName, bool &stop) {
                        if ( options.printVMAddrs )
                            printf("0x%08llX ", loadAddressUnslid);
                        if ( options.printUUIDs ) {
                            const uint8_t* uuid = (uint8_t*)dylibUUID;
                            printf("<%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X> ",
                                   uuid[0],  uuid[1],  uuid[2],  uuid[3],
                                   uuid[4],  uuid[5],  uuid[6],  uuid[7],
                                   uuid[8],  uuid[9],  uuid[10], uuid[11],
                                   uuid[12], uuid[13], uuid[14], uuid[15]);
                        }
                        if ( isAlias(installName, dyldCache) )
                            printf("[alias] %s\n", installName);
                        else
                            printf("%s\n", installName);
                    });
                }
                break;
            }
            case modeMap: {
                __block std::map<uint64_t, const char*> dataSegNames;
                __block std::map<uint64_t, uint64_t>    dataSegEnds;
                dyldCache->forEachImage(^(const mach_header* mh, const char* installName) {
                    dyld3::MachOFile* mf = (dyld3::MachOFile*)mh;
                    mf->forEachSegment(^(const dyld3::MachOAnalyzer::SegmentInfo &info, bool &stop) {
                        if ( isAlias(installName, dyldCache) )
                            return;
                        printf("0x%08llX - 0x%08llX %s %s\n", info.vmAddr, info.vmAddr + info.vmSize, info.segName, installName);
                        if ( strncmp(info.segName, "__DATA", 6) == 0 ) {
                            dataSegNames[info.vmAddr] = installName;
                            dataSegEnds[info.vmAddr] = info.vmAddr + info.vmSize;
                        }
                    });
                });
                // <rdar://problem/51084507> Enhance dyld_shared_cache_util to show where section alignment added padding
                uint64_t lastEnd = 0;
                for (const auto& entry : dataSegEnds) {
                    uint64_t padding = entry.first - lastEnd;
                    if ( (padding > 32) && (lastEnd != 0) ) {
                        printf("0x%08llX - 0x%08llX PADDING %lluKB\n", lastEnd, entry.first, padding/1024);
                    }
                    lastEnd = entry.second;
                }
                break;
            }
            case modeDependencies: {
                __block bool dependentTargetFound = false;
                dyldCache->forEachImage(^(const mach_header* mh, const char* installName) {
                    if ( strcmp(options.dependentsOfPath, installName) != 0 )
                        return;
                    dependentTargetFound = true;

                    auto printDep = [&options](const char *loadPath, uint32_t compatVersion, uint32_t curVersion) {
                        if ( options.printDylibVersions ) {
                            uint32_t compat_vers = compatVersion;
                            uint32_t current_vers = curVersion;
                            printf("\t%s", loadPath);
                            if ( compat_vers != 0xFFFFFFFF ) {
                                printf("(compatibility version %u.%u.%u, current version %u.%u.%u)\n",
                                       (compat_vers >> 16),
                                       (compat_vers >> 8) & 0xff,
                                       (compat_vers) & 0xff,
                                       (current_vers >> 16),
                                       (current_vers >> 8) & 0xff,
                                       (current_vers) & 0xff);
                            }
                            else {
                                printf("\n");
                            }
                        }
                        else {
                            printf("\t%s\n", loadPath);
                        }
                    };

                    dyld3::MachOFile* mf = (dyld3::MachOFile*)mh;

                    // First print out our dylib and version.
                    const char* dylibInstallName;
                    uint32_t currentVersion;
                    uint32_t compatVersion;
                    if ( mf->getDylibInstallName(&dylibInstallName, &compatVersion, &currentVersion) ) {
                        printDep(dylibInstallName, compatVersion, currentVersion);
                    }

                    // Then the dependent dylibs.
                    mf->forEachDependentDylib(^(const char *loadPath, bool isWeak, bool isReExport, bool isUpward, uint32_t compatVersion, uint32_t curVersion, bool &stop) {
                        printDep(loadPath, compatVersion, curVersion);
                    });
                });
                if (options.dependentsOfPath && !dependentTargetFound) {
                    fprintf(stderr, "Error: could not find '%s' in the shared cache at\n  %s\n", options.dependentsOfPath, sharedCachePath);
                    exit(1);
                }
                break;
            }
            case modeLinkEdit: {
                std::map<uint32_t, const char*> pageToContent;
                auto add_linkedit = [&pageToContent](uint32_t pageStart, uint32_t pageEnd, const char* message) {
                    for (uint32_t p = pageStart; p <= pageEnd; p += 4096) {
                        std::map<uint32_t, const char*>::iterator pos = pageToContent.find(p);
                        if ( pos == pageToContent.end() ) {
                            pageToContent[p] = strdup(message);
                        }
                        else {
                            const char* oldMessage = pos->second;
                            char* newMesssage;
                            asprintf(&newMesssage, "%s, %s", oldMessage, message);
                            pageToContent[p] = newMesssage;
                            ::free((void*)oldMessage);
                        }
                    }
                };

                dyldCache->forEachImage(^(const mach_header* mh, const char* installName) {
                    // Filter out symlinks.
                    if (isAlias(installName, dyldCache))
                        return;
                    dyld3::MachOAnalyzer* ma = (dyld3::MachOAnalyzer*)mh;
                    Diagnostics diag;
                    dyld3::MachOAnalyzer::LinkEditInfo leInfo;
                    ma->getLinkEditPointers(diag, leInfo);

                    if (diag.hasError())
                        return;

                    char message[1000];
                    const char* shortName = strrchr(installName, '/') + 1;
                    // add export trie info
                    if ( leInfo.dyldInfo->export_size != 0 ) {
                        //printf("export_off=0x%X\n", leInfo.dyldInfo->export_off());
                        uint32_t exportPageOffsetStart = leInfo.dyldInfo->export_off & (-4096);
                        uint32_t exportPageOffsetEnd = (leInfo.dyldInfo->export_off + leInfo.dyldInfo->export_size) & (-4096);
                        sprintf(message, "exports from %s", shortName);
                        add_linkedit(exportPageOffsetStart, exportPageOffsetEnd, message);
                    }
                    // add binding info
                    if ( leInfo.dyldInfo->bind_size != 0 ) {
                        uint32_t bindPageOffsetStart = leInfo.dyldInfo->bind_off & (-4096);
                        uint32_t bindPageOffsetEnd = (leInfo.dyldInfo->bind_off + leInfo.dyldInfo->bind_size) & (-4096);
                        sprintf(message, "bindings from %s", shortName);
                        add_linkedit(bindPageOffsetStart, bindPageOffsetEnd, message);
                    }
                    // add lazy binding info
                    if ( leInfo.dyldInfo->lazy_bind_size != 0 ) {
                        uint32_t lazybindPageOffsetStart = leInfo.dyldInfo->lazy_bind_off & (-4096);
                        uint32_t lazybindPageOffsetEnd = (leInfo.dyldInfo->lazy_bind_off + leInfo.dyldInfo->lazy_bind_size) & (-4096);
                        sprintf(message, "lazy bindings from %s", shortName);
                        add_linkedit(lazybindPageOffsetStart, lazybindPageOffsetEnd, message);
                    }
                    // add weak binding info
                    if ( leInfo.dyldInfo->weak_bind_size != 0 ) {
                        uint32_t weakbindPageOffsetStart = leInfo.dyldInfo->weak_bind_off & (-4096);
                        uint32_t weakbindPageOffsetEnd = (leInfo.dyldInfo->weak_bind_off + leInfo.dyldInfo->weak_bind_size) & (-4096);
                        sprintf(message, "weak bindings from %s", shortName);
                        add_linkedit(weakbindPageOffsetStart, weakbindPageOffsetEnd, message);
                    }
                });

                for (std::map<uint32_t, const char*>::iterator it = pageToContent.begin(); it != pageToContent.end(); ++it) {
                    printf("0x%08X %s\n", it->first, it->second);
                }
                break;
            }
            case modeSize: {
                struct TextInfo {
                    uint64_t    textSize;
                    const char* path;
                };
                __block std::vector<TextInfo> textSegments;
                dyldCache->forEachImage(^(const mach_header* mh, const char* installName) {
                    // Filter out symlinks.
                    if (isAlias(installName, dyldCache))
                        return;

                    dyld3::MachOAnalyzer* ma = (dyld3::MachOAnalyzer*)mh;
                    ma->forEachSegment(^(const dyld3::MachOAnalyzer::SegmentInfo &info, bool &stop) {
                        if ( strcmp(info.segName, "__TEXT") != 0 )
                            return;
                        textSegments.push_back({ info.fileSize, installName });
                    });
                });
                std::sort(textSegments.begin(), textSegments.end(), [](const TextInfo& left, const TextInfo& right) {
                    return (left.textSize > right.textSize);
                });
                for (std::vector<TextInfo>::iterator it = textSegments.begin(); it != textSegments.end(); ++it) {
                    printf(" 0x%08llX  %s\n", it->textSize, it->path);
                }
                break;
            }
            case modeNone:
            case modeInfo:
            case modeSlideInfo:
            case modeVerboseSlideInfo:
            case modeAcceleratorInfo:
            case modeTextInfo:
            case modeLocalSymbols:
            case modeJSONMap:
            case modeJSONDependents:
            case modeSectionSizes:
            case modeStrings:
            case modeObjCProtocols:
            case modeExtract:
                break;
        }
    }
    return 0;
}


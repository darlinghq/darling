/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2011 Apple Inc. All rights reserved.
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/syslimits.h>
#include <libkern/OSByteOrder.h>
#include <mach-o/arch.h>
#include <mach-o/loader.h>
#include <Availability.h>

#include "CodeSigningTypes.h"
#include <CommonCrypto/CommonHMAC.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonDigestSPI.h>

#define NO_ULEB
#include "Architectures.hpp"
#include "MachOFileAbstraction.hpp"
#include "CacheFileAbstraction.hpp"

#include "dsc_iterator.h"
#include "dsc_extractor.h"
#include "MachOTrie.hpp"
#include "SupportedArchs.h"
#include "DyldSharedCache.h"

#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <dispatch/dispatch.h>

struct seg_info
{
    seg_info(const char* n, uint64_t o, uint64_t s)
    : segName(n), offset(o), sizem(s) { }
    const char* segName;
    uint64_t    offset;
    uint64_t    sizem;
};

class CStringHash {
public:
    size_t operator()(const char* __s) const {
        size_t __h = 0;
        for ( ; *__s; ++__s)
            __h = 5 * __h + *__s;
        return __h;
    };
};
class CStringEquals {
public:
    bool operator()(const char* left, const char* right) const { return (strcmp(left, right) == 0); }
};
typedef std::unordered_map<const char*, std::vector<seg_info>, CStringHash, CStringEquals> NameToSegments;

// Filter to find individual symbol re-exports in trie
class NotReExportSymbol {
public:
    NotReExportSymbol(const std::set<int> &rd) :_reexportDeps(rd) {}
    bool operator()(const mach_o::trie::Entry &entry) const {
        bool result = isSymbolReExport(entry);
        if (result) {
            // <rdar://problem/17671438> Xcode 6 leaks in dyld_shared_cache_extract_dylibs
            ::free((void*)entry.name);
            const_cast<mach_o::trie::Entry*>(&entry)->name = NULL;
        }
        return result;
    }
private:
    bool isSymbolReExport(const mach_o::trie::Entry &entry) const {
        if ( (entry.flags & EXPORT_SYMBOL_FLAGS_KIND_MASK) != EXPORT_SYMBOL_FLAGS_KIND_REGULAR )
            return true;
        if ( (entry.flags & EXPORT_SYMBOL_FLAGS_REEXPORT) == 0 )
            return true;
        // If the symbol comes from a dylib that is re-exported, this is not an individual symbol re-export
        if ( _reexportDeps.count((int)entry.other) != 0 )
            return true;
        return false;
    }
    const std::set<int> &_reexportDeps;
};

template <typename P>
struct LoadCommandInfo {
};

template <typename A>
class LinkeditOptimizer {
    typedef typename A::P P;
    typedef typename A::P::E E;
    typedef typename A::P::uint_t pint_t;

private:
    macho_segment_command<P>* linkEditSegCmd = NULL;
    macho_symtab_command<P>* symtab = NULL;
    macho_dysymtab_command<P>*    dynamicSymTab = NULL;
    macho_linkedit_data_command<P>*    functionStarts = NULL;
    macho_linkedit_data_command<P>*    dataInCode = NULL;
    uint32_t exportsTrieOffset = 0;
    uint32_t exportsTrieSize = 0;
    std::set<int> reexportDeps;

public:

    void optimize_loadcommands(macho_header<typename A::P>* mh)
    {
        typedef typename A::P P;
        typedef typename A::P::E E;
        typedef typename A::P::uint_t pint_t;

        // update header flags
        mh->set_flags(mh->flags() & 0x7FFFFFFF); // remove in-cache bit

        // update load commands
        uint64_t cumulativeFileSize = 0;
        const unsigned origLoadCommandsSize = mh->sizeofcmds();
        unsigned bytesRemaining = origLoadCommandsSize;
        unsigned removedCount = 0;
        const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)mh + sizeof(macho_header<P>));
        const uint32_t cmdCount = mh->ncmds();
        const macho_load_command<P>* cmd = cmds;
        int depIndex = 0;
        for (uint32_t i = 0; i < cmdCount; ++i) {
            bool remove = false;
            switch ( cmd->cmd() ) {
                case macho_segment_command<P>::CMD:
                {
                    // update segment/section file offsets
                    macho_segment_command<P>* segCmd = (macho_segment_command<P>*)cmd;
                    segCmd->set_fileoff(cumulativeFileSize);
                    segCmd->set_filesize(segCmd->vmsize());
                    macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
                    macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
                    for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
                        if ( sect->offset() != 0 )
                            sect->set_offset((uint32_t)(cumulativeFileSize+sect->addr()-segCmd->vmaddr()));
                    }
                    if ( strcmp(segCmd->segname(), "__LINKEDIT") == 0 ) {
                        linkEditSegCmd = segCmd;
                    }
                    cumulativeFileSize += segCmd->filesize();
                    break;
                }
                case LC_DYLD_INFO_ONLY:
                {
                    // zero out all dyld info
                    macho_dyld_info_command<P>* dyldInfo = (macho_dyld_info_command<P>*)cmd;
                    exportsTrieOffset = dyldInfo->export_off();
                    exportsTrieSize = dyldInfo->export_size();
                    dyldInfo->set_rebase_off(0);
                    dyldInfo->set_rebase_size(0);
                    dyldInfo->set_bind_off(0);
                    dyldInfo->set_bind_size(0);
                    dyldInfo->set_weak_bind_off(0);
                    dyldInfo->set_weak_bind_size(0);
                    dyldInfo->set_lazy_bind_off(0);
                    dyldInfo->set_lazy_bind_size(0);
                    dyldInfo->set_export_off(0);
                    dyldInfo->set_export_size(0);
                }
                    break;
                case LC_SYMTAB:
                    symtab = (macho_symtab_command<P>*)cmd;
                    break;
                case LC_DYSYMTAB:
                    dynamicSymTab = (macho_dysymtab_command<P>*)cmd;
                    break;
                case LC_FUNCTION_STARTS:
                    functionStarts = (macho_linkedit_data_command<P>*)cmd;
                    break;
                case LC_DATA_IN_CODE:
                    dataInCode = (macho_linkedit_data_command<P>*)cmd;
                    break;
                case LC_LOAD_DYLIB:
                case LC_LOAD_WEAK_DYLIB:
                case LC_REEXPORT_DYLIB:
                case LC_LOAD_UPWARD_DYLIB:
                    ++depIndex;
                    if ( cmd->cmd() == LC_REEXPORT_DYLIB ) {
                        reexportDeps.insert(depIndex);
                    }
                    break;
                case LC_SEGMENT_SPLIT_INFO:
                    // <rdar://problem/23212513> dylibs iOS 9 dyld caches have bogus LC_SEGMENT_SPLIT_INFO
                    remove = true;
                    break;
            }
            uint32_t cmdSize = cmd->cmdsize();
            macho_load_command<P>* nextCmd = (macho_load_command<P>*)(((uint8_t*)cmd)+cmdSize);
            if ( remove ) {
                ::memmove((void*)cmd, (void*)nextCmd, bytesRemaining);
                ++removedCount;
            }
            else {
                bytesRemaining -= cmdSize;
                cmd = nextCmd;
            }
        }
        // zero out stuff removed
        ::bzero((void*)cmd, bytesRemaining);
        // update header
        mh->set_ncmds(cmdCount - removedCount);
        mh->set_sizeofcmds(origLoadCommandsSize - bytesRemaining);
    }

    int optimize_linkedit(std::vector<uint8_t> &new_linkedit_data, uint64_t textOffsetInCache, const void* mapped_cache)
    {
        typedef typename A::P P;
        typedef typename A::P::E E;
        typedef typename A::P::uint_t pint_t;

        // rebuild symbol table
        if ( linkEditSegCmd == NULL ) {
            fprintf(stderr, "__LINKEDIT not found\n");
            return -1;
        }
        if ( symtab == NULL ) {
            fprintf(stderr, "LC_SYMTAB not found\n");
            return -1;
        }
        if ( dynamicSymTab == NULL ) {
            fprintf(stderr, "LC_DYSYMTAB not found\n");
            return -1;
        }

        const uint64_t newFunctionStartsOffset = new_linkedit_data.size();
        uint32_t functionStartsSize = 0;
        if ( functionStarts != NULL ) {
            // copy function starts from original cache file to new mapped dylib file
            functionStartsSize = functionStarts->datasize();
            new_linkedit_data.insert(new_linkedit_data.end(),
                                     (char*)mapped_cache + functionStarts->dataoff(),
                                     (char*)mapped_cache + functionStarts->dataoff() + functionStartsSize);
        }

        // pointer align
        while ((linkEditSegCmd->fileoff() + new_linkedit_data.size()) % sizeof(pint_t))
            new_linkedit_data.push_back(0);

        const uint64_t newDataInCodeOffset = new_linkedit_data.size();
        uint32_t dataInCodeSize = 0;
        if ( dataInCode != NULL ) {
            // copy data-in-code info from original cache file to new mapped dylib file
            dataInCodeSize = dataInCode->datasize();
            new_linkedit_data.insert(new_linkedit_data.end(),
                                     (char*)mapped_cache + dataInCode->dataoff(),
                                     (char*)mapped_cache + dataInCode->dataoff() + dataInCodeSize);
        }

        std::vector<mach_o::trie::Entry> exports;
        if ( exportsTrieSize != 0 ) {
            const uint8_t* exportsStart = ((uint8_t*)mapped_cache) + exportsTrieOffset;
            const uint8_t* exportsEnd = &exportsStart[exportsTrieSize];
            mach_o::trie::parseTrie(exportsStart, exportsEnd, exports);
            exports.erase(std::remove_if(exports.begin(), exports.end(), NotReExportSymbol(reexportDeps)), exports.end());
        }

        // look for local symbol info in unmapped part of shared cache
        dyldCacheHeader<E>* header = (dyldCacheHeader<E>*)mapped_cache;
        macho_nlist<P>* localNlists = NULL;
        uint32_t localNlistCount = 0;
        const char* localStrings = NULL;
        const char* localStringsEnd = NULL;
        if ( header->mappingOffset() > offsetof(dyld_cache_header,localSymbolsSize) ) {
            dyldCacheLocalSymbolsInfo<E>* localInfo = (dyldCacheLocalSymbolsInfo<E>*)(((uint8_t*)mapped_cache) + header->localSymbolsOffset());
            dyldCacheLocalSymbolEntry<E>* entries = (dyldCacheLocalSymbolEntry<E>*)(((uint8_t*)mapped_cache) + header->localSymbolsOffset() + localInfo->entriesOffset());
            macho_nlist<P>* allLocalNlists = (macho_nlist<P>*)(((uint8_t*)localInfo) + localInfo->nlistOffset());
            const uint32_t entriesCount = localInfo->entriesCount();
            for (uint32_t i=0; i < entriesCount; ++i) {
                if ( entries[i].dylibOffset() == textOffsetInCache ) {
                    uint32_t localNlistStart = entries[i].nlistStartIndex();
                    localNlistCount = entries[i].nlistCount();
                    localNlists = &allLocalNlists[localNlistStart];
                    localStrings = ((char*)localInfo) + localInfo->stringsOffset();
                    localStringsEnd = &localStrings[localInfo->stringsSize()];
                    break;
                }
            }
        }
        // compute number of symbols in new symbol table
        const macho_nlist<P>* const mergedSymTabStart = (macho_nlist<P>*)(((uint8_t*)mapped_cache) + symtab->symoff());
        const macho_nlist<P>* const mergedSymTabend = &mergedSymTabStart[symtab->nsyms()];
        uint32_t newSymCount = symtab->nsyms();
        if ( localNlists != NULL ) {
            newSymCount = localNlistCount;
            for (const macho_nlist<P>* s = mergedSymTabStart; s != mergedSymTabend; ++s) {
                // skip any locals in cache
                if ( (s->n_type() & (N_TYPE|N_EXT)) == N_SECT )
                    continue;
                ++newSymCount;
            }
        }

        // add room for N_INDR symbols for re-exported symbols
        newSymCount += exports.size();

        // copy symbol entries and strings from original cache file to new mapped dylib file
        const char* mergedStringPoolStart = (char*)mapped_cache + symtab->stroff();
        const char* mergedStringPoolEnd = &mergedStringPoolStart[symtab->strsize()];

        // First count how many entries we need
        std::vector<macho_nlist<P>> newSymTab;
        newSymTab.reserve(newSymCount);
        std::vector<char> newSymNames;

        // first pool entry is always empty string
        newSymNames.push_back('\0');

        for (const macho_nlist<P>* s = mergedSymTabStart; s != mergedSymTabend; ++s) {
            // if we have better local symbol info, skip any locals here
            if ( (localNlists != NULL) && ((s->n_type() & (N_TYPE|N_EXT)) == N_SECT) )
                continue;
            macho_nlist<P> t = *s;
            t.set_n_strx((uint32_t)newSymNames.size());
            const char* symName = &mergedStringPoolStart[s->n_strx()];
            if ( symName > mergedStringPoolEnd )
                symName = "<corrupt symbol name>";
            newSymNames.insert(newSymNames.end(),
                               symName,
                               symName + (strlen(symName) + 1));
            newSymTab.push_back(t);
        }
        // <rdar://problem/16529213> recreate N_INDR symbols in extracted dylibs for debugger
        for (std::vector<mach_o::trie::Entry>::iterator it = exports.begin(); it != exports.end(); ++it) {
            macho_nlist<P> t;
            memset(&t, 0, sizeof(t));
            t.set_n_strx((uint32_t)newSymNames.size());
            t.set_n_type(N_INDR | N_EXT);
            t.set_n_sect(0);
            t.set_n_desc(0);
            newSymNames.insert(newSymNames.end(),
                               it->name,
                               it->name + (strlen(it->name) + 1));
            const char* importName = it->importName;
            if ( *importName == '\0' )
                importName = it->name;
            t.set_n_value(newSymNames.size());
            newSymNames.insert(newSymNames.end(),
                               importName,
                               importName + (strlen(importName) + 1));
            newSymTab.push_back(t);
        }
        if ( localNlists != NULL ) {
            // update load command to reflect new count of locals
            dynamicSymTab->set_ilocalsym((uint32_t)newSymTab.size());
            dynamicSymTab->set_nlocalsym(localNlistCount);
            // copy local symbols
            for (uint32_t i=0; i < localNlistCount; ++i) {
                const char* localName = &localStrings[localNlists[i].n_strx()];
                if ( localName > localStringsEnd )
                    localName = "<corrupt local symbol name>";
                macho_nlist<P> t = localNlists[i];
                t.set_n_strx((uint32_t)newSymNames.size());
                newSymNames.insert(newSymNames.end(),
                                   localName,
                                   localName + (strlen(localName) + 1));
                newSymTab.push_back(t);
            }
        }

        if ( newSymCount != newSymTab.size() ) {
            fprintf(stderr, "symbol count miscalculation\n");
            return -1;
        }

        //const uint64_t newStringPoolOffset = newIndSymTabOffset + dynamicSymTab->nindirectsyms()*sizeof(uint32_t);
        //macho_nlist<P>* const newSymTabStart = (macho_nlist<P>*)(((uint8_t*)mh) + newSymTabOffset);
        //char* const newStringPoolStart = (char*)mh + newStringPoolOffset;

        // pointer align
        while ((linkEditSegCmd->fileoff() + new_linkedit_data.size()) % sizeof(pint_t))
            new_linkedit_data.push_back(0);

        const uint64_t newSymTabOffset = new_linkedit_data.size();

        // Copy sym tab
        for (macho_nlist<P>& sym : newSymTab) {
            uint8_t symData[sizeof(macho_nlist<P>)];
            memcpy(&symData, &sym, sizeof(sym));
            new_linkedit_data.insert(new_linkedit_data.end(), &symData[0], &symData[sizeof(macho_nlist<P>)]);
        }

        const uint64_t newIndSymTabOffset = new_linkedit_data.size();

        // Copy indirect symbol table
        const uint32_t* mergedIndSymTab = (uint32_t*)((char*)mapped_cache + dynamicSymTab->indirectsymoff());
        new_linkedit_data.insert(new_linkedit_data.end(),
                                 (char*)mergedIndSymTab,
                                 (char*)(mergedIndSymTab + dynamicSymTab->nindirectsyms()));

        const uint64_t newStringPoolOffset = new_linkedit_data.size();

        // pointer align string pool size
        while (newSymNames.size() % sizeof(pint_t))
            newSymNames.push_back('\0');

        new_linkedit_data.insert(new_linkedit_data.end(), newSymNames.begin(), newSymNames.end());

        // update load commands
        if ( functionStarts != NULL ) {
            functionStarts->set_dataoff((uint32_t)(newFunctionStartsOffset + linkEditSegCmd->fileoff()));
            functionStarts->set_datasize(functionStartsSize);
        }
        if ( dataInCode != NULL ) {
            dataInCode->set_dataoff((uint32_t)(newDataInCodeOffset + linkEditSegCmd->fileoff()));
            dataInCode->set_datasize(dataInCodeSize);
        }

        symtab->set_nsyms(newSymCount);
        symtab->set_symoff((uint32_t)(newSymTabOffset + linkEditSegCmd->fileoff()));
        symtab->set_stroff((uint32_t)(newStringPoolOffset + linkEditSegCmd->fileoff()));
        symtab->set_strsize((uint32_t)newSymNames.size());
        dynamicSymTab->set_extreloff(0);
        dynamicSymTab->set_nextrel(0);
        dynamicSymTab->set_locreloff(0);
        dynamicSymTab->set_nlocrel(0);
        dynamicSymTab->set_indirectsymoff((uint32_t)(newIndSymTabOffset + linkEditSegCmd->fileoff()));
        linkEditSegCmd->set_filesize(symtab->stroff()+symtab->strsize() - linkEditSegCmd->fileoff());
        linkEditSegCmd->set_vmsize( (linkEditSegCmd->filesize()+4095) & (-4096) );

        // <rdar://problem/17671438> Xcode 6 leaks in dyld_shared_cache_extract_dylibs
        for (std::vector<mach_o::trie::Entry>::iterator it = exports.begin(); it != exports.end(); ++it) {
            ::free((void*)(it->name));
        }


        return 0;
    }

};

static void make_dirs(const char* file_path)
{
    //printf("make_dirs(%s)\n", file_path);
    char dirs[strlen(file_path)+1];
    strcpy(dirs, file_path);
    char* lastSlash = strrchr(dirs, '/');
    if ( lastSlash == NULL )
        return;
    lastSlash[1] = '\0';
    struct stat stat_buf;
    if ( stat(dirs, &stat_buf) != 0 ) {
        char* afterSlash = &dirs[1];
        char* slash;
        while ( (slash = strchr(afterSlash, '/')) != NULL ) {
            *slash = '\0';
            ::mkdir(dirs, S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH);
            //printf("mkdir(%s)\n", dirs);
            *slash = '/';
            afterSlash = slash+1;
        }
    }
}



template <typename A>
void dylib_maker(const void* mapped_cache, std::vector<uint8_t> &dylib_data, const std::vector<seg_info>& segments) {
    typedef typename A::P P;

    size_t  additionalSize  = 0;
    for(std::vector<seg_info>::const_iterator it=segments.begin(); it != segments.end(); ++it) {
        if ( strcmp(it->segName, "__LINKEDIT") != 0 )
            additionalSize += it->sizem;
    }

    std::vector<uint8_t> new_dylib_data;
    new_dylib_data.reserve(additionalSize);

    // Write regular segments into the buffer
    uint64_t                textOffsetInCache    = 0;
    for( std::vector<seg_info>::const_iterator it=segments.begin(); it != segments.end(); ++it) {

        if(strcmp(it->segName, "__TEXT") == 0 )
            textOffsetInCache = it->offset;

        //printf("segName=%s, offset=0x%llX, size=0x%0llX\n", it->segName, it->offset, it->sizem);
        // Copy all but the __LINKEDIT.  It will be copied later during the optimizer in to a temporary buffer but it would
        // not be efficient to copy it all now for each dylib.
        if (strcmp(it->segName, "__LINKEDIT") == 0 )
            continue;
        std::copy(((uint8_t*)mapped_cache)+it->offset, ((uint8_t*)mapped_cache)+it->offset+it->sizem, std::back_inserter(new_dylib_data));
    }

    // optimize linkedit
    std::vector<uint8_t> new_linkedit_data;
    new_linkedit_data.reserve(1 << 20);

    LinkeditOptimizer<A> linkeditOptimizer;
    macho_header<P>* mh = (macho_header<P>*)&new_dylib_data.front();
    linkeditOptimizer.optimize_loadcommands(mh);
    linkeditOptimizer.optimize_linkedit(new_linkedit_data, textOffsetInCache, mapped_cache);

    new_dylib_data.insert(new_dylib_data.end(), new_linkedit_data.begin(), new_linkedit_data.end());

    // Page align file
    while (new_dylib_data.size() % 4096)
        new_dylib_data.push_back(0);

    dylib_data.insert(dylib_data.end(), new_dylib_data.begin(), new_dylib_data.end());
}

typedef __typeof(dylib_maker<x86>) dylib_maker_func;
typedef void (^progress_block)(unsigned current, unsigned total);

class SharedCacheExtractor;
struct SharedCacheDylibExtractor {
    SharedCacheDylibExtractor(const char* name, std::vector<seg_info> segInfo)
        : name(name), segInfo(segInfo) { }

    void extractCache(SharedCacheExtractor& context);

    const char*                     name;
    const std::vector<seg_info>     segInfo;
    int                             result = 0;
};

struct SharedCacheExtractor {
    SharedCacheExtractor(const NameToSegments& map,
                         const char* extraction_root_path,
                         dylib_maker_func* dylib_create_func,
                         void* mapped_cache,
                         progress_block progress)
        : map(map), extraction_root_path(extraction_root_path),
          dylib_create_func(dylib_create_func), mapped_cache(mapped_cache),
          progress(progress) {

      extractors.reserve(map.size());
      for (auto it : map)
          extractors.emplace_back(it.first, it.second);

        // Limit the number of open files.  16 seems to give better performance than higher numbers.
        sema = dispatch_semaphore_create(16);
    }
    int extractCaches();

    static void extractCache(void *ctx, size_t i);

    const NameToSegments&                   map;
    std::vector<SharedCacheDylibExtractor>  extractors;
    dispatch_semaphore_t                    sema;
    const char*                             extraction_root_path;
    dylib_maker_func*                       dylib_create_func;
    void*                                   mapped_cache;
    progress_block                          progress;
    std::atomic_int                         count = { 0 };
};

int SharedCacheExtractor::extractCaches() {
    dispatch_queue_t process_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0);
    dispatch_apply_f(map.size(), process_queue,
                     this, extractCache);

    int result = 0;
    for (const SharedCacheDylibExtractor& extractor : extractors) {
        if (extractor.result != 0) {
            result = extractor.result;
            break;
        }
    }
    return result;
}

void SharedCacheExtractor::extractCache(void *ctx, size_t i) {
    SharedCacheExtractor& context = *(SharedCacheExtractor*)ctx;
    dispatch_semaphore_wait(context.sema, DISPATCH_TIME_FOREVER);
    context.extractors[i].extractCache(context);
    dispatch_semaphore_signal(context.sema);
}

void SharedCacheDylibExtractor::extractCache(SharedCacheExtractor &context) {

    char    dylib_path[PATH_MAX];
    strcpy(dylib_path, context.extraction_root_path);
    strcat(dylib_path, "/");
    strcat(dylib_path, name);

    //printf("%s with %lu segments\n", dylib_path, it->second.size());
    // make sure all directories in this path exist
    make_dirs(dylib_path);

    // open file, create if does not already exist
    int fd = ::open(dylib_path, O_CREAT | O_TRUNC | O_EXLOCK | O_RDWR, 0644);
    if ( fd == -1 ) {
        fprintf(stderr, "can't open or create dylib file %s, errnor=%d\n", dylib_path, errno);
        result = -1;
        return;
    }

    std::vector<uint8_t> vec;
    context.dylib_create_func(context.mapped_cache, vec, segInfo);
    context.progress(context.count++, (unsigned)context.map.size());

    // Write file data
    if( write(fd, &vec.front(), vec.size()) == -1) {
        fprintf(stderr, "error writing, errnor=%d\n", errno);
        result = -1;
    }

    close(fd);
}

static int sharedCacheIsValid(const void* mapped_cache, uint64_t size) {
    // First check that the size is good.
    // Note the shared cache may not have a codeSignatureSize value set so we need to first make
    // sure we have space for the CS_SuperBlob, then later crack that to check for the size of the rest.
    const DyldSharedCache* dyldSharedCache = (DyldSharedCache*)mapped_cache;
    uint64_t requiredSizeForCSSuperBlob = dyldSharedCache->header.codeSignatureOffset + sizeof(CS_SuperBlob);
    const dyld_cache_mapping_info* mappings = (dyld_cache_mapping_info*)((uint8_t*)mapped_cache + dyldSharedCache->header.mappingOffset);
    if ( requiredSizeForCSSuperBlob > size ) {
        fprintf(stderr, "Error: dyld shared cache size 0x%08llx is less than required size of 0x%08llx.\n", size, requiredSizeForCSSuperBlob);
        return -1;
    }

    // Now see if the code signatures are valid as that tells us the pages aren't corrupt.
    // First find all of the regions of the shared cache we computed cd hashes
    std::vector<std::pair<uint64_t, uint64_t>> sharedCacheRegions;
    sharedCacheRegions.emplace_back(std::make_pair(mappings[0].fileOffset, mappings[0].fileOffset + mappings[0].size));
    sharedCacheRegions.emplace_back(std::make_pair(mappings[1].fileOffset, mappings[1].fileOffset + mappings[1].size));
    sharedCacheRegions.emplace_back(std::make_pair(mappings[2].fileOffset, mappings[2].fileOffset + mappings[2].size));
    if (dyldSharedCache->header.localSymbolsSize)
        sharedCacheRegions.emplace_back(std::make_pair(dyldSharedCache->header.localSymbolsOffset, dyldSharedCache->header.localSymbolsOffset + dyldSharedCache->header.localSymbolsSize));
    size_t inBbufferSize = 0;
    for (auto& sharedCacheRegion : sharedCacheRegions)
        inBbufferSize += (sharedCacheRegion.second - sharedCacheRegion.first);

    // Now take the cd hash from the cache itself and validate the regions we found.
    uint8_t* codeSignatureRegion = (uint8_t*)mapped_cache + dyldSharedCache->header.codeSignatureOffset;
    CS_SuperBlob* sb = reinterpret_cast<CS_SuperBlob*>(codeSignatureRegion);
    if (sb->magic != htonl(CSMAGIC_EMBEDDED_SIGNATURE)) {
        fprintf(stderr, "Error: dyld shared cache code signature magic is incorrect.\n");
        return -1;
    }

    size_t sbSize = ntohl(sb->length);
    uint64_t requiredSizeForCS = dyldSharedCache->header.codeSignatureOffset + sbSize;
    if ( requiredSizeForCS > size ) {
        fprintf(stderr, "Error: dyld shared cache size 0x%08llx is less than required size of 0x%08llx.\n", size, requiredSizeForCS);
        return -1;
    }

    // Find the offset to the code directory.
    CS_CodeDirectory* cd = nullptr;
    for (unsigned i =0; i != sb->count; ++i) {
        if (ntohl(sb->index[i].type) == CSSLOT_CODEDIRECTORY) {
            cd = (CS_CodeDirectory*)(codeSignatureRegion + ntohl(sb->index[i].offset));
            break;
        }
    }

    if (!cd) {
        fprintf(stderr, "Error: dyld shared cache code signature directory is missing.\n");
        return -1;
    }

    if ( (uint8_t*)cd > (codeSignatureRegion + sbSize) ) {
        fprintf(stderr, "Error: dyld shared cache code signature directory is out of bounds.\n");
        return -1;
    }

    if ( cd->magic != htonl(CSMAGIC_CODEDIRECTORY) ) {
        fprintf(stderr, "Error: dyld shared cache code signature directory magic is incorrect.\n");
        return -1;
    }

    uint32_t pageSize = 1 << cd->pageSize;
    uint32_t slotCountFromRegions = (uint32_t)((inBbufferSize + pageSize - 1) / pageSize);
    if ( ntohl(cd->nCodeSlots) < slotCountFromRegions ) {
        fprintf(stderr, "Error: dyld shared cache code signature directory num slots is incorrect.\n");
        return -1;
    }

    uint32_t dscDigestFormat = kCCDigestNone;
    switch (cd->hashType) {
        case CS_HASHTYPE_SHA1:
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            dscDigestFormat = kCCDigestSHA1;
#pragma clang diagnostic pop
            break;
        case CS_HASHTYPE_SHA256:
            dscDigestFormat = kCCDigestSHA256;
            break;
        default:
            break;
    }

    if (dscDigestFormat != kCCDigestNone) {
        const uint64_t csPageSize = 1 << cd->pageSize;
        size_t   hashOffset = ntohl(cd->hashOffset);
        uint8_t* hashSlot = (uint8_t*)cd + hashOffset;
        uint8_t cdHashBuffer[cd->hashSize];

        // Skip local symbols for now as those aren't being codesign correctly right now.
        size_t inBbufferSize = 0;
        for (auto& sharedCacheRegion : sharedCacheRegions) {
            if (sharedCacheRegion.first == dyldSharedCache->header.localSymbolsOffset)
                continue;
            inBbufferSize += (sharedCacheRegion.second - sharedCacheRegion.first);
        }
        uint32_t slotCountToProcess = (uint32_t)((inBbufferSize + pageSize - 1) / pageSize);

        for (unsigned i = 0; i != slotCountToProcess; ++i) {
            // Skip data pages as those may have been slid by ASLR in the extracted file
            uint64_t fileOffset = i * csPageSize;
            if ( (fileOffset >= mappings[1].fileOffset) && (fileOffset < (mappings[1].fileOffset + mappings[1].size)) )
                continue;

            CCDigest(dscDigestFormat, (uint8_t*)mapped_cache + fileOffset, (size_t)csPageSize, cdHashBuffer);
            uint8_t* cacheCdHashBuffer = hashSlot + (i * cd->hashSize);
            if (memcmp(cdHashBuffer, cacheCdHashBuffer, cd->hashSize) != 0)  {
                fprintf(stderr, "Error: dyld shared cache code signature for page %d is incorrect.\n", i);
                return -1;
            }
        }
    }
    return 0;
}

int dyld_shared_cache_extract_dylibs_progress(const char* shared_cache_file_path, const char* extraction_root_path,
                                              progress_block progress)
{
    struct stat statbuf;
    if (stat(shared_cache_file_path, &statbuf)) {
        fprintf(stderr, "Error: stat failed for dyld shared cache at %s\n", shared_cache_file_path);
        return -1;
    }

    int cache_fd = open(shared_cache_file_path, O_RDONLY);
    if (cache_fd < 0) {
        fprintf(stderr, "Error: failed to open shared cache file at %s\n", shared_cache_file_path);
        return -1;
    }

    void* mapped_cache = mmap(NULL, (size_t)statbuf.st_size, PROT_READ, MAP_PRIVATE, cache_fd, 0);
    if (mapped_cache == MAP_FAILED) {
        fprintf(stderr, "Error: mmap() for shared cache at %s failed, errno=%d\n", shared_cache_file_path, errno);
        return -1;
    }

    close(cache_fd);

    // instantiate arch specific dylib maker
    dylib_maker_func* dylib_create_func = nullptr;
    if ( strcmp((char*)mapped_cache, "dyld_v1    i386") == 0 )
        dylib_create_func = dylib_maker<x86>;
    else if ( strcmp((char*)mapped_cache, "dyld_v1  x86_64") == 0 )
        dylib_create_func = dylib_maker<x86_64>;
    else if ( strcmp((char*)mapped_cache, "dyld_v1 x86_64h") == 0 )
        dylib_create_func = dylib_maker<x86_64>;
    else if ( strcmp((char*)mapped_cache, "dyld_v1   armv5") == 0 )
        dylib_create_func = dylib_maker<arm>;
    else if ( strcmp((char*)mapped_cache, "dyld_v1   armv6") == 0 )
        dylib_create_func = dylib_maker<arm>;
    else if ( strcmp((char*)mapped_cache, "dyld_v1   armv7") == 0 )
        dylib_create_func = dylib_maker<arm>;
    else if ( strncmp((char*)mapped_cache, "dyld_v1  armv7", 14) == 0 )
        dylib_create_func = dylib_maker<arm>;
    else if ( strcmp((char*)mapped_cache, "dyld_v1   arm64") == 0 )
        dylib_create_func = dylib_maker<arm64>;
#if SUPPORT_ARCH_arm64e
    else if ( strcmp((char*)mapped_cache, "dyld_v1  arm64e") == 0 )
        dylib_create_func = dylib_maker<arm64>;
#endif
#if SUPPORT_ARCH_arm64_32
    else if ( strcmp((char*)mapped_cache, "dyld_v1arm64_32") == 0 )
        dylib_create_func = dylib_maker<arm64_32>;
#endif
    else {
        fprintf(stderr, "Error: unrecognized dyld shared cache magic.\n");
        munmap(mapped_cache, (size_t)statbuf.st_size);
        return -1;
    }

    // Verify that the cache isn't corrupt.
    if (int result = sharedCacheIsValid(mapped_cache, (uint64_t)statbuf.st_size)) {
        munmap(mapped_cache, (size_t)statbuf.st_size);
        return result;
    }

    // iterate through all images in cache and build map of dylibs and segments
    __block NameToSegments  map;
    int                     result = 0;

    result = dyld_shared_cache_iterate(mapped_cache, (uint32_t)statbuf.st_size, ^(const dyld_shared_cache_dylib_info* dylibInfo, const dyld_shared_cache_segment_info* segInfo) {
        map[dylibInfo->path].push_back(seg_info(segInfo->name, segInfo->fileOffset, segInfo->fileSize));
    });

    if(result != 0) {
        fprintf(stderr, "Error: dyld_shared_cache_iterate_segments_with_slide failed.\n");
        munmap(mapped_cache, (size_t)statbuf.st_size);
        return result;
    }

    // for each dylib instantiate a dylib file
    SharedCacheExtractor extractor(map, extraction_root_path, dylib_create_func, mapped_cache, progress);
    result = extractor.extractCaches();

    munmap(mapped_cache, (size_t)statbuf.st_size);
    return result;
}



int dyld_shared_cache_extract_dylibs(const char* shared_cache_file_path, const char* extraction_root_path)
{
    return dyld_shared_cache_extract_dylibs_progress(shared_cache_file_path, extraction_root_path,
                                                     ^(unsigned , unsigned) {} );
}


#if 0
// test program
#include <stdio.h>
#include <stddef.h>
#include <dlfcn.h>


typedef int (*extractor_proc)(const char* shared_cache_file_path, const char* extraction_root_path,
                              void (^progress)(unsigned current, unsigned total));

int main(int argc, const char* argv[])
{
    if ( argc != 3 ) {
        fprintf(stderr, "usage: dsc_extractor <path-to-cache-file> <path-to-device-dir>\n");
        return 1;
    }

    //void* handle = dlopen("/Volumes/my/src/dyld/build/Debug/dsc_extractor.bundle", RTLD_LAZY);
    void* handle = dlopen("/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/usr/lib/dsc_extractor.bundle", RTLD_LAZY);
    if ( handle == NULL ) {
        fprintf(stderr, "dsc_extractor.bundle could not be loaded\n");
        return 1;
    }

    extractor_proc proc = (extractor_proc)dlsym(handle, "dyld_shared_cache_extract_dylibs_progress");
    if ( proc == NULL ) {
        fprintf(stderr, "dsc_extractor.bundle did not have dyld_shared_cache_extract_dylibs_progress symbol\n");
        return 1;
    }

    int result = (*proc)(argv[1], argv[2], ^(unsigned c, unsigned total) { printf("%d/%d\n", c, total); } );
    fprintf(stderr, "dyld_shared_cache_extract_dylibs_progress() => %d\n", result);
    return 0;
}


#endif





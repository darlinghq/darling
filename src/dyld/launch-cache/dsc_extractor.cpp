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
#include <mach-o/fat.h>
#include <mach-o/arch.h>
#include <mach-o/loader.h>
#include <Availability.h>

#define NO_ULEB 
#include "Architectures.hpp"
#include "MachOFileAbstraction.hpp"
#include "CacheFileAbstraction.hpp"

#include "dsc_iterator.h"
#include "dsc_extractor.h"
#include "MachOTrie.hpp"

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
	uint64_t	offset;
	uint64_t	sizem;
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


template <typename A>
int optimize_linkedit(macho_header<typename A::P>* mh, uint64_t textOffsetInCache, const void* mapped_cache, uint64_t* newSize) 
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
	macho_segment_command<P>* linkEditSegCmd = NULL;
	macho_symtab_command<P>* symtab = NULL;
	macho_dysymtab_command<P>*	dynamicSymTab = NULL;
	macho_linkedit_data_command<P>*	functionStarts = NULL;
	macho_linkedit_data_command<P>*	dataInCode = NULL;
	uint32_t exportsTrieOffset = 0;
	uint32_t exportsTrieSize = 0;
	std::set<int> reexportDeps;
	int depIndex = 0;
	for (uint32_t i = 0; i < cmdCount; ++i) {
	    bool remove = false;
		switch ( cmd->cmd() ) {
		case macho_segment_command<P>::CMD:
			{
			// update segment/section file offsets
			macho_segment_command<P>* segCmd = (macho_segment_command<P>*)cmd;
			segCmd->set_fileoff(cumulativeFileSize);
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
			}
			break;
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

	const uint64_t newFunctionStartsOffset = linkEditSegCmd->fileoff();
	uint32_t functionStartsSize = 0;
	if ( functionStarts != NULL ) {
		// copy function starts from original cache file to new mapped dylib file
		functionStartsSize = functionStarts->datasize();
		memcpy((char*)mh + newFunctionStartsOffset, (char*)mapped_cache + functionStarts->dataoff(), functionStartsSize);
	}
	const uint64_t newDataInCodeOffset = (newFunctionStartsOffset + functionStartsSize + sizeof(pint_t) - 1) & (-sizeof(pint_t)); // pointer align
	uint32_t dataInCodeSize = 0;
	if ( dataInCode != NULL ) {
		// copy data-in-code info from original cache file to new mapped dylib file
		dataInCodeSize = dataInCode->datasize();
		memcpy((char*)mh + newDataInCodeOffset, (char*)mapped_cache + dataInCode->dataoff(), dataInCodeSize);
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
	const uint64_t newSymTabOffset = (newDataInCodeOffset + dataInCodeSize + sizeof(pint_t) - 1) & (-sizeof(pint_t)); // pointer align
	const uint64_t newIndSymTabOffset = newSymTabOffset + newSymCount*sizeof(macho_nlist<P>);
	const uint64_t newStringPoolOffset = newIndSymTabOffset + dynamicSymTab->nindirectsyms()*sizeof(uint32_t);
	macho_nlist<P>* const newSymTabStart = (macho_nlist<P>*)(((uint8_t*)mh) + newSymTabOffset);
	char* const newStringPoolStart = (char*)mh + newStringPoolOffset;
	const uint32_t* mergedIndSymTab = (uint32_t*)((char*)mapped_cache + dynamicSymTab->indirectsymoff());
	const char* mergedStringPoolStart = (char*)mapped_cache + symtab->stroff();
	const char* mergedStringPoolEnd = &mergedStringPoolStart[symtab->strsize()];
	macho_nlist<P>* t = newSymTabStart;
	int poolOffset = 0;
	uint32_t symbolsCopied = 0;
	newStringPoolStart[poolOffset++] = '\0'; // first pool entry is always empty string
	for (const macho_nlist<P>* s = mergedSymTabStart; s != mergedSymTabend; ++s) {
		// if we have better local symbol info, skip any locals here
		if ( (localNlists != NULL) && ((s->n_type() & (N_TYPE|N_EXT)) == N_SECT) ) 
			continue;
		*t = *s;
		t->set_n_strx(poolOffset);
		const char* symName = &mergedStringPoolStart[s->n_strx()];
		if ( symName > mergedStringPoolEnd )
			symName = "<corrupt symbol name>";
		strcpy(&newStringPoolStart[poolOffset], symName);
		poolOffset += (strlen(symName) + 1);
		++t;
		++symbolsCopied;
	}
	// <rdar://problem/16529213> recreate N_INDR symbols in extracted dylibs for debugger
	for (std::vector<mach_o::trie::Entry>::iterator it = exports.begin(); it != exports.end(); ++it) {
		strcpy(&newStringPoolStart[poolOffset], it->name);
		t->set_n_strx(poolOffset);
		poolOffset += (strlen(it->name) + 1);
		t->set_n_type(N_INDR | N_EXT);
		t->set_n_sect(0);
		t->set_n_desc(0);
		const char* importName = it->importName;
		if ( *importName == '\0' )
			importName = it->name;
		strcpy(&newStringPoolStart[poolOffset], importName);
		t->set_n_value(poolOffset);
		poolOffset += (strlen(importName) + 1);
		++t;
		++symbolsCopied;
	}
	if ( localNlists != NULL ) {
		// update load command to reflect new count of locals
		dynamicSymTab->set_ilocalsym(symbolsCopied);
		dynamicSymTab->set_nlocalsym(localNlistCount);
		// copy local symbols
		for (uint32_t i=0; i < localNlistCount; ++i) {
			const char* localName = &localStrings[localNlists[i].n_strx()];
			if ( localName > localStringsEnd )
				localName = "<corrupt local symbol name>";
			*t = localNlists[i];
			t->set_n_strx(poolOffset);
			strcpy(&newStringPoolStart[poolOffset], localName);
			poolOffset += (strlen(localName) + 1);
			++t;
			++symbolsCopied;
		}
	}
	
	if ( newSymCount != symbolsCopied ) {
		fprintf(stderr, "symbol count miscalculation\n");
		return -1;
	}
	
	// pointer align string pool size
	while ( (poolOffset % sizeof(pint_t)) != 0 )
		++poolOffset; 
	// copy indirect symbol table
	uint32_t* newIndSymTab = (uint32_t*)((char*)mh + newIndSymTabOffset);
	memcpy(newIndSymTab, mergedIndSymTab, dynamicSymTab->nindirectsyms()*sizeof(uint32_t));
	
	// update load commands
	if ( functionStarts != NULL ) {
		functionStarts->set_dataoff((uint32_t)newFunctionStartsOffset);
		functionStarts->set_datasize(functionStartsSize);
	}
	if ( dataInCode != NULL ) {
		dataInCode->set_dataoff((uint32_t)newDataInCodeOffset);
		dataInCode->set_datasize(dataInCodeSize);
	}
	symtab->set_nsyms(symbolsCopied);
	symtab->set_symoff((uint32_t)newSymTabOffset);
	symtab->set_stroff((uint32_t)newStringPoolOffset);
	symtab->set_strsize(poolOffset);
	dynamicSymTab->set_extreloff(0);
	dynamicSymTab->set_nextrel(0);
	dynamicSymTab->set_locreloff(0);
	dynamicSymTab->set_nlocrel(0);
	dynamicSymTab->set_indirectsymoff((uint32_t)newIndSymTabOffset);
	linkEditSegCmd->set_filesize(symtab->stroff()+symtab->strsize() - linkEditSegCmd->fileoff());
	linkEditSegCmd->set_vmsize( (linkEditSegCmd->filesize()+4095) & (-4096) );
	
	// return new size
	*newSize = (symtab->stroff()+symtab->strsize()+4095) & (-4096);
	
	// <rdar://problem/17671438> Xcode 6 leaks in dyld_shared_cache_extract_dylibs
	for (std::vector<mach_o::trie::Entry>::iterator it = exports.begin(); it != exports.end(); ++it) {
		::free((void*)(it->name));
	}
	
	
	return 0;
}



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
size_t dylib_maker(const void* mapped_cache, std::vector<uint8_t> &dylib_data, const std::vector<seg_info>& segments) {		
	typedef typename A::P P;
    
    size_t  additionalSize  = 0;
	for(std::vector<seg_info>::const_iterator it=segments.begin(); it != segments.end(); ++it) {
		additionalSize                          += it->sizem;
	}
    
    dylib_data.reserve(dylib_data.size() + additionalSize);
    
    uint32_t                nfat_archs          = 0;
	uint32_t                offsetInFatFile     = 4096;
    uint8_t                 *base_ptr           = &dylib_data.front();
	    
#define FH reinterpret_cast<fat_header*>(base_ptr)
#define FA reinterpret_cast<fat_arch*>(base_ptr + (8 + (nfat_archs - 1) * sizeof(fat_arch)))
    
    if(dylib_data.size() >= 4096 && OSSwapBigToHostInt32(FH->magic) == FAT_MAGIC) {
		// have fat header, append new arch to end
        nfat_archs                              = OSSwapBigToHostInt32(FH->nfat_arch);
		offsetInFatFile                         = OSSwapBigToHostInt32(FA->offset) + OSSwapBigToHostInt32(FA->size);
    }
    
    dylib_data.resize(offsetInFatFile);
    base_ptr                                    = &dylib_data.front();
    
    FH->magic                                   = OSSwapHostToBigInt32(FAT_MAGIC);
    FH->nfat_arch                               = OSSwapHostToBigInt32(++nfat_archs);
    
    FA->cputype                                 = 0; // filled in later
    FA->cpusubtype                              = 0; // filled in later
    FA->offset                                  = OSSwapHostToBigInt32(offsetInFatFile);
    FA->size                                    = 0; // filled in later
    FA->align                                   = OSSwapHostToBigInt32(12);
    
	// Write regular segments into the buffer
	uint64_t                totalSize           = 0;
    uint64_t				textOffsetInCache	= 0;
	for( std::vector<seg_info>::const_iterator it=segments.begin(); it != segments.end(); ++it) {
        
        if(strcmp(it->segName, "__TEXT") == 0 ) {
			textOffsetInCache					= it->offset;
            const macho_header<P>   *textMH     = reinterpret_cast<macho_header<P>*>((uint8_t*)mapped_cache+textOffsetInCache);
            FA->cputype                         = OSSwapHostToBigInt32(textMH->cputype()); 
            FA->cpusubtype                      = OSSwapHostToBigInt32(textMH->cpusubtype());
            
            // if this cputype/subtype already exist in fat header, then return immediately
            for(uint32_t i=0; i < nfat_archs-1; ++i) {
                fat_arch            *afa        = reinterpret_cast<fat_arch*>(base_ptr+8)+i;
                
                if(   afa->cputype == FA->cputype
                   && afa->cpusubtype == FA->cpusubtype) {
                    //fprintf(stderr, "arch already exists in fat dylib\n");
                    dylib_data.resize(offsetInFatFile);
                    return offsetInFatFile;
                }
            }
		}
        
		//printf("segName=%s, offset=0x%llX, size=0x%0llX\n", it->segName, it->offset, it->sizem);
        std::copy(((uint8_t*)mapped_cache)+it->offset, ((uint8_t*)mapped_cache)+it->offset+it->sizem, std::back_inserter(dylib_data));
        base_ptr                                = &dylib_data.front();
        totalSize                               += it->sizem;
	}
    
	FA->size                                    = OSSwapHostToBigInt32(totalSize); 
    
	// optimize linkedit
	uint64_t                newSize             = dylib_data.size();
	optimize_linkedit<A>(((macho_header<P>*)(base_ptr+offsetInFatFile)), textOffsetInCache, mapped_cache, &newSize);
	
	// update fat header with new file size
    dylib_data.resize(offsetInFatFile+newSize);
    base_ptr                                    = &dylib_data.front();
	FA->size                                    = OSSwapHostToBigInt32(newSize);
#undef FH
#undef FA
	return offsetInFatFile;
} 


int dyld_shared_cache_extract_dylibs_progress(const char* shared_cache_file_path, const char* extraction_root_path,
													void (^progress)(unsigned current, unsigned total))
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
	
	void* mapped_cache = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, cache_fd, 0);
	if (mapped_cache == MAP_FAILED) {
		fprintf(stderr, "Error: mmap() for shared cache at %s failed, errno=%d\n", shared_cache_file_path, errno);
		return -1;
	}
    
    close(cache_fd);

	// instantiate arch specific dylib maker
    size_t (*dylib_create_func)(const void*, std::vector<uint8_t>&, const std::vector<seg_info>&) = NULL;
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
	else {
		fprintf(stderr, "Error: unrecognized dyld shared cache magic.\n");
        munmap(mapped_cache, statbuf.st_size);
		return -1;
	}

	// iterate through all images in cache and build map of dylibs and segments
	__block NameToSegments  map;
	__block int				result              = dyld_shared_cache_iterate(mapped_cache, (uint32_t)statbuf.st_size, ^(const dyld_shared_cache_dylib_info* dylibInfo, const dyld_shared_cache_segment_info* segInfo) {
        map[dylibInfo->path].push_back(seg_info(segInfo->name, segInfo->fileOffset, segInfo->fileSize));
    });

    if(result != 0) {
		fprintf(stderr, "Error: dyld_shared_cache_iterate_segments_with_slide failed.\n");
        munmap(mapped_cache, statbuf.st_size);
		return result;
    }

	// for each dylib instantiate a dylib file
    dispatch_group_t        group               = dispatch_group_create();
    dispatch_semaphore_t    sema                = dispatch_semaphore_create(2);
    dispatch_queue_t        process_queue       = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0);
    dispatch_queue_t        writer_queue        = dispatch_queue_create("dyld writer queue", 0);
    
	__block unsigned        count               = 0;
    
	for ( NameToSegments::iterator it = map.begin(); it != map.end(); ++it) {
		dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
        dispatch_group_async(group, process_queue, ^{
            
            char    dylib_path[PATH_MAX];
            strcpy(dylib_path, extraction_root_path);
            strcat(dylib_path, "/");
            strcat(dylib_path, it->first);
            
            //printf("%s with %lu segments\n", dylib_path, it->second.size());
            // make sure all directories in this path exist
            make_dirs(dylib_path);
            
            // open file, create if does not already exist
            int fd = ::open(dylib_path, O_CREAT | O_EXLOCK | O_RDWR, 0644);
            if ( fd == -1 ) {
                fprintf(stderr, "can't open or create dylib file %s, errnor=%d\n", dylib_path, errno);
                result    = -1;
                return;
            }
            
            struct stat statbuf;
            if (fstat(fd, &statbuf)) {
                fprintf(stderr, "Error: stat failed for dyld file %s, errnor=%d\n", dylib_path, errno);
                close(fd);
                result    = -1;
                return;
            }
            
            std::vector<uint8_t> *vec   = new std::vector<uint8_t>(statbuf.st_size);
            if(pread(fd, &vec->front(), vec->size(), 0) != (long)vec->size()) {
                fprintf(stderr, "can't read dylib file %s, errnor=%d\n", dylib_path, errno);
                close(fd);
                result    = -1;
                return;
            }
            
            const size_t    offset  = dylib_create_func(mapped_cache, *vec, it->second);
            
            dispatch_group_async(group, writer_queue, ^{
                progress(count++, (unsigned)map.size());
                
                if(offset != vec->size()) {
                    //Write out the first page, and everything after offset
                    if(   pwrite(fd, &vec->front(), 4096, 0) == -1 
                       || pwrite(fd, &vec->front() + offset, vec->size() - offset, offset) == -1) {
                        fprintf(stderr, "error writing, errnor=%d\n", errno);
                        result    = -1;
                    }
                }
                
                delete vec;
                close(fd);
                dispatch_semaphore_signal(sema);
            });
        });
	}
    
    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    dispatch_release(group);
    dispatch_release(writer_queue);
    
    munmap(mapped_cache, statbuf.st_size);
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

 
 


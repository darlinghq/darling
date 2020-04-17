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

#include <stdlib.h>
#include <stdio.h>
#include <Availability.h>


#include "dsc_iterator.h"
#include "dyld_cache_format.h"
#define NO_ULEB
#include "Architectures.hpp"
#include "MachOFileAbstraction.hpp"
#include "CacheFileAbstraction.hpp"
#include "SupportedArchs.h"

namespace dyld {


	// convert an address in the shared region where the cache would normally be mapped, into an address where the cache is currently mapped
	template <typename E>
	const uint8_t* mappedAddress(const uint8_t* cache, const uint8_t* cacheEnd, uint64_t addr)
	{
		const dyldCacheHeader<E>* header = (dyldCacheHeader<E>*)cache;
		const dyldCacheFileMapping<E>* mappings = (dyldCacheFileMapping<E>*)&cache[header->mappingOffset()];
		for (uint32_t i=0; i < header->mappingCount(); ++i) {
			if ( (mappings[i].address() <= addr) &&  (addr < (mappings[i].address() + mappings[i].size())) ) {
				uint64_t cacheOffset = mappings[i].file_offset() + addr - mappings[i].address();
				const uint8_t* result =  &cache[cacheOffset];
				if ( result < cacheEnd )
					return result;
				else 
					return NULL;
			}
		}
		return NULL;
	}

	// call the callback block on each segment in this image						  	  
	template <typename A>
	int walkSegments(const uint8_t* cache, const uint8_t* cacheEnd, const uint8_t* firstSeg, const char* dylibPath, uint64_t inode,uint64_t modTime, const uint8_t* machHeader, uint64_t cache_unslid_base_address,
											void (^callback)(const dyld_shared_cache_dylib_info* dylibInfo, const dyld_shared_cache_segment_info* segInfo)) 
	{
		typedef typename A::P		P;	
		typedef typename A::P::E	E;	
		dyld_shared_cache_dylib_info	dylibInfo;
		dyld_shared_cache_segment_info	segInfo;
		dylibInfo.version = 2;
		dylibInfo.isAlias = (dylibPath < (char*)firstSeg); // paths for aliases are store between cache header and first segment
		dylibInfo.machHeader = machHeader;
		dylibInfo.path = dylibPath;
		dylibInfo.inode = inode;
		dylibInfo.modTime = modTime;
		const macho_header<P>* mh = (const macho_header<P>*)machHeader;
		const macho_load_command<P>* const cmds = (macho_load_command<P>*)(machHeader + sizeof(macho_header<P>));
		if ( (machHeader+ mh->sizeofcmds()) > cacheEnd )
			return -1;
		const uint32_t cmd_count = mh->ncmds();
		const macho_load_command<P>* cmd = cmds;
		// scan for LC_UUID
		dylibInfo.uuid = NULL;
		for (uint32_t i = 0; i < cmd_count; ++i) {
			if ( cmd->cmd() == LC_UUID ) {
				const uuid_command* uc = (const uuid_command*)cmd;
				dylibInfo.uuid = &uc->uuid;
				break;
			}
			cmd = (const macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
		}
		// callback for each LC_SEGMENT
		cmd = cmds;
		for (uint32_t i = 0; i < cmd_count; ++i) {
			if ( cmd->cmd() == macho_segment_command<P>::CMD ) {
				macho_segment_command<P>* segCmd = (macho_segment_command<P>*)cmd;
				uint64_t fileOffset = segCmd->fileoff();
				// work around until <rdar://problem/7022345> is fixed
				if ( fileOffset == 0 ) {
					fileOffset = (machHeader - cache);
				}
				uint64_t sizem = segCmd->vmsize();
				if ( strcmp(segCmd->segname(), "__LINKEDIT") == 0 ) {
					// clip LINKEDIT size if bigger than cache file
					if ( (fileOffset+sizem) > (uint64_t)(cacheEnd-cache) )
						sizem = (cacheEnd-cache)-fileOffset;
				}
				segInfo.version = 2;
				segInfo.name = segCmd->segname();
				segInfo.fileOffset = fileOffset;
				segInfo.fileSize = sizem;
				if ( segCmd->filesize() > segCmd->vmsize() )
					return -1;
				segInfo.address = segCmd->vmaddr();
                segInfo.addressOffset = segInfo.address - cache_unslid_base_address;
				callback(&dylibInfo, &segInfo);
			}
			cmd = (const macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
		}
		return 0;
	}
			
							  
	// call walkSegments on each image in the cache							  	  
	template <typename A>
	int walkImages(const uint8_t* cache, uint32_t size,	void (^callback)(const dyld_shared_cache_dylib_info* dylibInfo, const dyld_shared_cache_segment_info* segInfo)) 
	{
		// Sanity check there is at least a header
		if ( (size > 0) && (size < 0x7000) )
			return -1;
		typedef typename A::P::E			E;	
		typedef typename A::P				P;	
		const dyldCacheHeader<E>*      header   = (dyldCacheHeader<E>*)cache;
		const dyldCacheImageInfo<E>*   dylibs   = (dyldCacheImageInfo<E>*)&cache[header->imagesOffset()];
		const dyldCacheFileMapping<E>* mappings = (dyldCacheFileMapping<E>*)&cache[header->mappingOffset()];
        uint64_t unslid_base_address = mappings[0].address();
		uint64_t greatestMappingOffset = 0;
		for (uint32_t i=0; i < header->mappingCount(); ++i) {
			if ( (size != 0) && (mappings[i].file_offset() > size) )
				return -1;
			uint64_t endOffset = mappings[i].file_offset()+mappings[i].size();
			if ( (size != 0) && (endOffset > size) )
				return -1;
			if ( endOffset > greatestMappingOffset ) 
				greatestMappingOffset = endOffset;
		}
		const uint8_t* cacheEnd = &cache[size];
		if ( size == 0 ) {
			// Zero size means old API is being used, assume all mapped
			cacheEnd = &cache[greatestMappingOffset];
		}
		else {
			// verifiy mappings are not bigger than size
			if ( size < greatestMappingOffset ) 
				return -1;
		}
		// verify all image infos are mapped
		if ( (const uint8_t*)&dylibs[header->imagesCount()] > cacheEnd )
			return -1;
		const uint8_t* firstSeg = NULL;
		for (uint32_t i=0; i < header->imagesCount(); ++i) {
			const char* dylibPath  = (char*)cache + dylibs[i].pathFileOffset();
			uint64_t inode = dylibs[i].inode();
			uint64_t modTime = dylibs[i].modTime();
			if ( (const uint8_t*)dylibPath > cacheEnd )
				return -1;
			const uint8_t* machHeader = mappedAddress<E>(cache, cacheEnd, dylibs[i].address());
			if ( machHeader == NULL )
				return -1;
			if ( machHeader > cacheEnd )
				return -1;
			if ( firstSeg == NULL )
				firstSeg = machHeader;
			int result = walkSegments<A>(cache, cacheEnd, firstSeg, dylibPath, inode, modTime, machHeader, unslid_base_address, callback);
			if ( result != 0 )
				return result;
		}
		return 0;
	}

}


// Given a pointer to an in-memory copy of a dyld shared cache file,
// this routine will call the callback block once for each segment
// in each dylib in the shared cache file.  
// Returns -1 if there was an error, otherwise 0.
extern int dyld_shared_cache_iterate(const void* shared_cache_file, uint32_t shared_cache_size, 
									void (^callback)(const dyld_shared_cache_dylib_info* dylibInfo, const dyld_shared_cache_segment_info* segInfo)) {
	const uint8_t* cache = (uint8_t*)shared_cache_file;
		 if ( strcmp((char*)cache, "dyld_v1    i386") == 0 ) 
			return dyld::walkImages<x86>(cache, shared_cache_size, callback);
	else if ( strcmp((char*)cache, "dyld_v1  x86_64") == 0 ) 
			return dyld::walkImages<x86_64>(cache, shared_cache_size, callback);
	else if ( strcmp((char*)cache, "dyld_v1 x86_64h") == 0 ) 
			return dyld::walkImages<x86_64>(cache, shared_cache_size, callback);
	else if ( strcmp((char*)cache, "dyld_v1   armv5") == 0 ) 
			return dyld::walkImages<arm>(cache, shared_cache_size, callback);
	else if ( strcmp((char*)cache, "dyld_v1   armv6") == 0 ) 
			return dyld::walkImages<arm>(cache, shared_cache_size, callback);
	else if ( strcmp((char*)cache, "dyld_v1   armv7") == 0 ) 
			return dyld::walkImages<arm>(cache, shared_cache_size, callback);
	else if ( strncmp((char*)cache, "dyld_v1  armv7", 14) == 0 ) 
			return dyld::walkImages<arm>(cache, shared_cache_size, callback);
	else if ( strcmp((char*)cache, "dyld_v1   arm64") == 0 ) 
			return dyld::walkImages<arm64>(cache, shared_cache_size, callback);
#if SUPPORT_ARCH_arm64_32
	else if ( strcmp((char*)cache, "dyld_v1arm64_32") == 0 ) 
			return dyld::walkImages<arm64_32>(cache, shared_cache_size, callback);
#endif
#if SUPPORT_ARCH_arm64e
	else if ( strcmp((char*)cache, "dyld_v1  arm64e") == 0 ) 
			return dyld::walkImages<arm64>(cache, shared_cache_size, callback);
#endif
	else
		return -1;
}


// implement old version by calling new version
int dyld_shared_cache_iterate_segments_with_slide(const void* shared_cache_file, dyld_shared_cache_iterator_slide_t callback)
{
	return dyld_shared_cache_iterate(shared_cache_file, 0, ^(const dyld_shared_cache_dylib_info* dylibInfo, const dyld_shared_cache_segment_info* segInfo) {
														callback(dylibInfo->path, segInfo->name, segInfo->fileOffset, segInfo->fileSize, segInfo->address, 0);
	});
}

// implement non-block version by calling block version
int dyld_shared_cache_iterate_segments_with_slide_nb(const void* shared_cache_file, dyld_shared_cache_iterator_slide_nb_t func, void* userData)
{
	return dyld_shared_cache_iterate_segments_with_slide(shared_cache_file, ^(const char* dylibName, const char* segName, 
													uint64_t offset, uint64_t size, uint64_t mappedddress, uint64_t slide) {
														(*func)(dylibName, segName, offset, size, mappedddress, slide, userData);
	});
}


// implement non-slide version by wrapping slide version in block
int dyld_shared_cache_iterate_segments(const void* shared_cache_file, dyld_shared_cache_iterator_t callback)
{
	dyld_shared_cache_iterator_slide_t wrapper_cb = ^(const char* dylibName, const char* segName, uint64_t offset, 
														uint64_t size, uint64_t mappedddress, uint64_t slide) {
														callback(dylibName, segName, offset, size, mappedddress);
													};
	return dyld_shared_cache_iterate_segments_with_slide(shared_cache_file, wrapper_cb);
}

// implement non-slide,non-block version by wrapping slide version in block
int dyld_shared_cache_iterate_segments_nb(const void* shared_cache_file, dyld_shared_cache_iterator_nb_t func, void* userData)
{
	return dyld_shared_cache_iterate_segments_with_slide(shared_cache_file, ^(const char* dylibName, const char* segName, 
																			  uint64_t offset, uint64_t size, uint64_t mappedddress, uint64_t slide) {
		(*func)(dylibName, segName, offset, size, mappedddress, userData);
	});
}


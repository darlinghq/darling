/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2006-2015 Apple Inc. All rights reserved.
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
#ifndef __DYLD_CACHE_FORMAT__
#define __DYLD_CACHE_FORMAT__

#include <sys/types.h>
#include <stdint.h>
#include <mach/shared_region.h>
#include <uuid/uuid.h>


struct dyld_cache_header
{
	char		magic[16];				// e.g. "dyld_v0    i386"
	uint32_t	mappingOffset;			// file offset to first dyld_cache_mapping_info
	uint32_t	mappingCount;			// number of dyld_cache_mapping_info entries
	uint32_t	imagesOffset;			// file offset to first dyld_cache_image_info
	uint32_t	imagesCount;			// number of dyld_cache_image_info entries
	uint64_t	dyldBaseAddress;		// base address of dyld when cache was built
	uint64_t	codeSignatureOffset;	// file offset of code signature blob
	uint64_t	codeSignatureSize;		// size of code signature blob (zero means to end of file)
	uint64_t	slideInfoOffset;		// file offset of kernel slid info
	uint64_t	slideInfoSize;			// size of kernel slid info
	uint64_t	localSymbolsOffset;		// file offset of where local symbols are stored
	uint64_t	localSymbolsSize;		// size of local symbols information
	uint8_t		uuid[16];				// unique value for each shared cache file
	uint64_t	cacheType;				// 0 for development, 1 for production
	uint32_t	branchPoolsOffset;		// file offset to table of uint64_t pool addresses
	uint32_t	branchPoolsCount;	    // number of uint64_t entries
	uint64_t	accelerateInfoAddr;		// (unslid) address of optimization info
	uint64_t	accelerateInfoSize;		// size of optimization info
	uint64_t	imagesTextOffset;		// file offset to first dyld_cache_image_text_info
	uint64_t	imagesTextCount;		// number of dyld_cache_image_text_info entries
};


struct dyld_cache_mapping_info {
	uint64_t	address;
	uint64_t	size;
	uint64_t	fileOffset;
	uint32_t	maxProt;
	uint32_t	initProt;
};

struct dyld_cache_image_info
{
	uint64_t	address;
	uint64_t	modTime;
	uint64_t	inode;
	uint32_t	pathFileOffset;
	uint32_t	pad;
};

struct dyld_cache_image_info_extra
{
	uint64_t	exportsTrieAddr;		// address of trie in unslid cache
	uint64_t	weakBindingsAddr;
	uint32_t	exportsTrieSize;
	uint32_t	weakBindingsSize;
	uint32_t	dependentsStartArrayIndex;
	uint32_t	reExportsStartArrayIndex;
};


struct dyld_cache_accelerator_info
{
	uint32_t	version;				// currently 1
	uint32_t	imageExtrasCount;		// does not include aliases
	uint32_t	imagesExtrasOffset;		// offset into this chunk of first dyld_cache_image_info_extra
	uint32_t	bottomUpListOffset;		// offset into this chunk to start of 16-bit array of sorted image indexes
	uint32_t	dylibTrieOffset;		// offset into this chunk to start of trie containing all dylib paths
	uint32_t	dylibTrieSize;			// size of trie containing all dylib paths
	uint32_t	initializersOffset;		// offset into this chunk to start of initializers list
	uint32_t	initializersCount;		// size of initializers list
	uint32_t	dofSectionsOffset;		// offset into this chunk to start of DOF sections list
	uint32_t	dofSectionsCount;		// size of initializers list
	uint32_t	reExportListOffset;		// offset into this chunk to start of 16-bit array of re-exports
	uint32_t	reExportCount;			// size of re-exports
	uint32_t	depListOffset;			// offset into this chunk to start of 16-bit array of dependencies (0x8000 bit set if upward)
	uint32_t	depListCount;			// size of dependencies
	uint32_t	rangeTableOffset;		// offset into this chunk to start of ss
	uint32_t	rangeTableCount;		// size of dependencies
	uint64_t	dyldSectionAddr;		// address of libdyld's __dyld section in unslid cache
};

struct dyld_cache_accelerator_initializer
{
	uint32_t	functionOffset;			// address offset from start of cache mapping
	uint32_t	imageIndex;
};

struct dyld_cache_range_entry
{
	uint64_t	startAddress;			// unslid address of start of region
	uint32_t	size;
	uint32_t	imageIndex;
};

struct dyld_cache_accelerator_dof
{
	uint64_t	sectionAddress;			// unslid address of start of region
	uint32_t	sectionSize;
	uint32_t	imageIndex;
};

struct dyld_cache_image_text_info
{
	uuid_t		uuid;
	uint64_t	loadAddress;			// unslid address of start of __TEXT
	uint32_t	textSegmentSize;
	uint32_t	pathOffset;				// offset from start of cache file
};

// The rebasing info is to allow the kernel to lazily rebase DATA pages of the
// dyld shared cache.  Rebasing is adding the slide to interior pointers.
struct dyld_cache_slide_info
{
	uint32_t	version;		// currently 1
	uint32_t	toc_offset;
	uint32_t	toc_count;
	uint32_t	entries_offset;
	uint32_t	entries_count;
	uint32_t	entries_size;  // currently 128 
	// uint16_t toc[toc_count];
	// entrybitmap entries[entries_count];
};


// The version 2 of the slide info uses a different compression scheme. Since
// only interior pointers (pointers that point within the cache) are rebased
// (slid), we know the possible range of the pointers and thus know there are
// unused bits in each pointer.  We use those bits to form a linked list of
// locations needing rebasing in each page.
//
// Definitions:
//
//  pageIndex = (pageAddress - startOfAllDataAddress)/info->page_size
//  pageStarts[] = info + info->page_starts_offset
//  pageExtras[] = info + info->page_extras_offset
//  valueMask = ~(info->delta_mask)
//  deltaShift = __builtin_ctzll(info->delta_mask) - 2
//
// There are three cases:
//
// 1) pageStarts[pageIndex] == DYLD_CACHE_SLIDE_PAGE_ATTR_NO_REBASE
//    The page contains no values that need rebasing.
//
// 2) (pageStarts[pageIndex] & DYLD_CACHE_SLIDE_PAGE_ATTR_EXTRA) == 0
//    All rebase locations are in one linked list. The offset of the first
//    rebase location in the page is pageStarts[pageIndex] * 4.
//
// 3) pageStarts[pageIndex] & DYLD_CACHE_SLIDE_PAGE_ATTR_EXTRA
//	  Multiple linked lists are needed for all rebase locations in a page.
//    The pagesExtras array contains 2 or more entries each of which is the
//    start of a new linked list in the page. The first is at:
//       extrasStartIndex = (pageStarts[pageIndex] & 0x3FFF)
//	  The next is at extrasStartIndex+1.  The last is denoted by
//    having the high bit (DYLD_CACHE_SLIDE_PAGE_ATTR_END) of the pageExtras[]
//    set.
//
// For 64-bit architectures, there is always enough free bits to encode all
// possible deltas.  The info->delta_mask field shows where the delta is located
// in the pointer.  That value must be masked off (valueMask) before the slide
// is added to the pointer.
//
// For 32-bit architectures, there are only three bits free (the three most
// significant bits). To extract the delta, you must first subtract value_add
// from the pointer value, then AND with delta_mask, then shift by deltaShift.
// That still leaves a maximum delta to the next rebase location of 28 bytes.
// To reduce the number or chains needed, an optimization was added.  Turns
// out zero is common in the DATA region.  A zero can be turned into a
// non-rebasing entry in the linked list.  The can be done because nothing
// in the shared cache should point out of its dylib to the start of the shared
// cache.
//
// The code for processing a linked list (chain) is:
//   
//	uint32_t delta = 1;
//	while ( delta != 0 ) {
//		uint8_t* loc = pageStart + pageOffset;
//		uintptr_t rawValue = *((uintptr_t*)loc);
//		delta = ((rawValue & deltaMask) >> deltaShift);
//		uintptr_t newValue = (rawValue & valueMask);
//		if ( newValue != 0 ) {
//			newValue += valueAdd;
//			newValue += slideAmount;
//		}
//		*((uintptr_t*)loc) = newValue;
//		pageOffset += delta;
//	}
//
//
struct dyld_cache_slide_info2
{
	uint32_t	version;		// currently 2
	uint32_t	page_size;		// currently 4096 (may also be 16384)
	uint32_t	page_starts_offset;
	uint32_t	page_starts_count;
	uint32_t	page_extras_offset;
	uint32_t	page_extras_count;
	uint64_t	delta_mask;		// which (contiguous) set of bits contains the delta to the next rebase location
	uint64_t	value_add;
	//uint16_t	page_starts[page_starts_count];
	//uint16_t	page_extras[page_extras_count];
};
#define DYLD_CACHE_SLIDE_PAGE_ATTRS				0xC000	// high bits of uint16_t are flags
#define DYLD_CACHE_SLIDE_PAGE_ATTR_EXTRA		0x8000  // index is into extras array (not starts array)
#define DYLD_CACHE_SLIDE_PAGE_ATTR_NO_REBASE	0x4000  // page has no rebasing
#define DYLD_CACHE_SLIDE_PAGE_ATTR_END			0x8000  // last chain entry for page



// The version 3 of the slide info uses a different compression scheme. Since
// only interior pointers (pointers that point within the cache) are rebased
// (slid), we know the possible range of the pointers and thus know there are
// unused bits in each pointer.  We use those bits to form a linked list of
// locations needing rebasing in each page.
//
// Definitions:
//
//  pageIndex = (pageAddress - startOfAllDataAddress)/info->page_size
//  pageStarts[] = info + info->page_starts_offset
//
// There are two cases:
//
// 1) pageStarts[pageIndex] == DYLD_CACHE_SLIDE_V3_PAGE_ATTR_NO_REBASE
//    The page contains no values that need rebasing.
//
// 2) otherwise...
//    All rebase locations are in one linked list. The offset of the first
//    rebase location in the page is pageStarts[pageIndex].
//
// A pointer is one of :
// {
//     uint64_t pointerValue : 51;
//     uint64_t offsetToNextPointer : 11;
//     uint64_t isBind : 1 = 0;
//     uint64_t authenticated : 1 = 0;
// }
// {
//     uint32_t offsetFromSharedCacheBase;
//     uint16_t diversityData;
//     uint16_t hasAddressDiversity : 1;
//     uint16_t key : 2;
//     uint16_t offsetToNextPointer : 11;
//     uint16_t isBind : 1;
//     uint16_t authenticated : 1 = 1;
// }
//
// The code for processing a linked list (chain) is:
//
//    uint32_t delta = pageStarts[pageIndex];
//    uint8_t* loc = pageStart;
//    do {
//        loc += delta;
//        uintptr_t rawValue = *((uintptr_t*)loc);
//        delta = ( (value & 0x3FF8000000000000) >> 51) * sizeof(uint64_t);
//        if (extraBindData.isAuthenticated) {
//            newValue = ( value & 0xFFFFFFFF )  + results->slide + auth_value_add;
//            newValue = sign_using_the_various_bits(newValue);
//        } else {
//            uint64_t top8Bits = value & 0x0007F80000000000ULL;
//            uint64_t bottom43Bits = value & 0x000007FFFFFFFFFFULL;
//            uint64_t targetValue = ( top8Bits << 13 ) | bottom43Bits;
//            newValue = targetValue + results->slide;
//        }
//        *((uintptr_t*)loc) = newValue;
//    } while (delta != 0 )
//
//
struct dyld_cache_slide_info3
{
    uint32_t    version;            // currently 3
    uint32_t    page_size;          // currently 4096 (may also be 16384)
    uint32_t    page_starts_count;
    uint64_t    auth_value_add;
    uint16_t    page_starts[/* page_starts_count */];
};

#define DYLD_CACHE_SLIDE_V3_PAGE_ATTR_NO_REBASE    0xFFFF    // page has no rebasing


struct dyld_cache_local_symbols_info
{
	uint32_t	nlistOffset;		// offset into this chunk of nlist entries
	uint32_t	nlistCount;			// count of nlist entries
	uint32_t	stringsOffset;		// offset into this chunk of string pool
	uint32_t	stringsSize;		// byte count of string pool
	uint32_t	entriesOffset;		// offset into this chunk of array of dyld_cache_local_symbols_entry 
	uint32_t	entriesCount;		// number of elements in dyld_cache_local_symbols_entry array
};

struct dyld_cache_local_symbols_entry
{
	uint32_t	dylibOffset;		// offset in cache file of start of dylib
	uint32_t	nlistStartIndex;	// start index of locals for this dylib
	uint32_t	nlistCount;			// number of local symbols for this dylib
};

struct dyld_cache_image_patches
{
    uint32_t    patchExportsStartIndex;
    uint32_t    patchExportsCount;
};

struct dyld_cache_patchable_export
{
    uint32_t    cacheOffsetOfImpl;
    uint32_t    patchLocationsStartIndex;
    uint32_t    patchLocationsCount;
    uint32_t    exportNameOffset;
};

struct dyld_cache_patchable_location
{
    uint64_t    cacheOffset             : 32,
                addend                  : 12,    // +/- 2048
                authenticated           : 1,
                usesAddressDiversity    : 1,
                key                     : 2,
                discriminator           : 16;

    dyld_cache_patchable_location(size_t cacheOffset, uint64_t addend);
    //dyld_cache_patchable_location(size_t cacheOffset, uint64_t addend, dyld3::MachOLoaded::ChainedFixupPointerOnDisk authInfo);

    uint64_t getAddend() const {
        uint64_t unsingedAddend = addend;
        int64_t signedAddend = (int64_t)unsingedAddend;
        signedAddend = (signedAddend << 52) >> 52;
        return (uint64_t)signedAddend;
    }

    const char* keyName() const;
    bool operator==(const dyld_cache_patchable_location& other) const {
        return this->cacheOffset == other.cacheOffset;
    }
};



#define MACOSX_DYLD_SHARED_CACHE_DIR	"/private/var/db/dyld/"
#define IPHONE_DYLD_SHARED_CACHE_DIR	"/System/Library/Caches/com.apple.dyld/"
#define DYLD_SHARED_CACHE_BASE_NAME		"dyld_shared_cache_"
#define DYLD_SHARED_CACHE_DEVELOPMENT_EXT ".development"

static const uint64_t kDyldSharedCacheTypeDevelopment = 0;
static const uint64_t kDyldSharedCacheTypeProduction = 1;




#endif // __DYLD_CACHE_FORMAT__



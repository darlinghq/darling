/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
#ifndef __DYLD_CACHE_ABSTRACTION__
#define __DYLD_CACHE_ABSTRACTION__

#include "dyld_cache_format.h"

#include "FileAbstraction.hpp"
#include "Architectures.hpp"

template <typename E>
class dyldCacheHeader {
public:
	const char*		magic() const							INLINE { return fields.magic; }
	void			set_magic(const char* value)			INLINE { memcpy(fields.magic, value, 16); }

	uint32_t		mappingOffset() const					INLINE { return E::get32(fields.mappingOffset); }
	void			set_mappingOffset(uint32_t value)		INLINE { E::set32(fields.mappingOffset, value); }

	uint32_t		mappingCount() const					INLINE { return E::get32(fields.mappingCount); }
	void			set_mappingCount(uint32_t value)		INLINE { E::set32(fields.mappingCount, value); }

	uint32_t		imagesOffset() const					INLINE { return E::get32(fields.imagesOffset); }
	void			set_imagesOffset(uint32_t value)		INLINE { E::set32(fields.imagesOffset, value); }

	uint32_t		imagesCount() const						INLINE { return E::get32(fields.imagesCount); }
	void			set_imagesCount(uint32_t value)			INLINE { E::set32(fields.imagesCount, value); }

	uint64_t		dyldBaseAddress() const					INLINE { return E::get64(fields.dyldBaseAddress); }
	void			set_dyldBaseAddress(uint64_t value)		INLINE { E::set64(fields.dyldBaseAddress, value); }

	uint64_t		codeSignatureOffset() const				INLINE { return E::get64(fields.codeSignatureOffset); }
	void			set_codeSignatureOffset(uint64_t value)	INLINE { E::set64(fields.codeSignatureOffset, value); }

	uint64_t		codeSignatureSize() const				INLINE { return E::get64(fields.codeSignatureSize); }
	void			set_codeSignatureSize(uint64_t value)	INLINE { E::set64(fields.codeSignatureSize, value); }

	uint64_t		slideInfoOffset() const					INLINE { return E::get64(fields.slideInfoOffset); }
	void			set_slideInfoOffset(uint64_t value)		INLINE { E::set64(fields.slideInfoOffset, value); }

	uint64_t		slideInfoSize() const					INLINE { return E::get64(fields.slideInfoSize); }
	void			set_slideInfoSize(uint64_t value)		INLINE { E::set64(fields.slideInfoSize, value); }

	uint64_t		localSymbolsOffset() const				INLINE { return E::get64(fields.localSymbolsOffset); }
	void			set_localSymbolsOffset(uint64_t value)	INLINE { E::set64(fields.localSymbolsOffset, value); }

	uint64_t		localSymbolsSize() const				INLINE { return E::get64(fields.localSymbolsSize); }
	void			set_localSymbolsSize(uint64_t value)	INLINE { E::set64(fields.localSymbolsSize, value); }

	const uint8_t*	uuid() const							INLINE { return fields.uuid; }
	void			set_uuid(const uint8_t value[16])		INLINE { memcpy(fields.uuid, value, 16); }
	
	uint64_t		cacheType() const						INLINE { return E::get64(fields.cacheType); }
	void			set_cacheType(uint64_t value)			INLINE { E::set64(fields.cacheType, value); }

	uint32_t		branchPoolsOffset() const				INLINE { return E::get32(fields.branchPoolsOffset); }
	void			set_branchPoolsOffset(uint32_t value)	INLINE { E::set32(fields.branchPoolsOffset, value); }

	uint32_t		branchPoolsCount() const				INLINE { return E::get32(fields.branchPoolsCount); }
	void			set_branchPoolsCount(uint32_t value)	INLINE { E::set32(fields.branchPoolsCount, value); }

	uint64_t		accelerateInfoAddr() const				INLINE { return E::get64(fields.accelerateInfoAddr); }
	void			set_accelerateInfoAddr(uint64_t value)	INLINE { E::set64(fields.accelerateInfoAddr, value); }

	uint64_t		accelerateInfoSize() const				INLINE { return E::get64(fields.accelerateInfoSize); }
	void			set_accelerateInfoSize(uint64_t value)	INLINE { E::set64(fields.accelerateInfoSize, value); }

	uint64_t		imagesTextOffset() const				INLINE { return E::get64(fields.imagesTextOffset); }
	void			set_imagesTextOffset(uint64_t value)	INLINE { E::set64(fields.imagesTextOffset, value); }

	uint64_t		imagesTextCount() const					INLINE { return E::get64(fields.imagesTextCount); }
	void			set_imagesTextCount(uint64_t value)		INLINE { E::set64(fields.imagesTextCount, value); }

private:
	dyld_cache_header			fields;
};



template <typename E>
class dyldCacheFileMapping {
public:		
	uint64_t		address() const								INLINE { return E::get64(fields.address); }
	void			set_address(uint64_t value)					INLINE { E::set64(fields.address, value); }

	uint64_t		size() const								INLINE { return E::get64(fields.size); }
	void			set_size(uint64_t value)					INLINE { E::set64(fields.size, value); }

	uint64_t		file_offset() const							INLINE { return E::get64(fields.fileOffset); }
	void			set_file_offset(uint64_t value)				INLINE { E::set64(fields.fileOffset, value); }

	uint32_t		max_prot() const							INLINE { return E::get32(fields.maxProt); }
	void			set_max_prot(uint32_t value)				INLINE { E::set32((uint32_t&)fields.maxProt, value); }

	uint32_t		init_prot() const							INLINE { return E::get32(fields.initProt); }
	void			set_init_prot(uint32_t value)				INLINE { E::set32((uint32_t&)fields.initProt, value); }

private:
	dyld_cache_mapping_info			fields;
};


template <typename E>
class dyldCacheImageInfo {
public:		
	uint64_t		address() const								INLINE { return E::get64(fields.address); }
	void			set_address(uint64_t value)					INLINE { E::set64(fields.address, value); }

	uint64_t		modTime() const								INLINE { return E::get64(fields.modTime); }
	void			set_modTime(uint64_t value)					INLINE { E::set64(fields.modTime, value); }

	uint64_t		inode() const								INLINE { return E::get64(fields.inode); }
	void			set_inode(uint64_t value)					INLINE { E::set64(fields.inode, value); }

	uint32_t		pathFileOffset() const						INLINE { return E::get32(fields.pathFileOffset); }
	void			set_pathFileOffset(uint32_t value)			INLINE { E::set32(fields.pathFileOffset, value); fields.pad=0; }

private:
	dyld_cache_image_info			fields;
};

template <typename E>
class dyldCacheImageTextInfo {
public:		
	const uint8_t*	uuid() const								INLINE { return fields.uuid; }
	void			set_uuid(const uint8_t value[16])			INLINE { memcpy(fields.uuid, value, 16); }

	uint64_t		loadAddress() const							INLINE { return E::get64(fields.loadAddress); }
	void			set_loadAddress(uint64_t value)				INLINE { E::set64(fields.loadAddress, value); }

	uint32_t		textSegmentSize() const						INLINE { return E::get32(fields.textSegmentSize); }
	void			set_textSegmentSize(uint32_t value)			INLINE { E::set32(fields.textSegmentSize, value); }

	uint32_t		pathOffset() const							INLINE { return E::get32(fields.pathOffset); }
	void			set_pathOffset(uint32_t value)				INLINE { E::set32(fields.pathOffset, value); }

private:
	dyld_cache_image_text_info			fields;
};



template <typename E>
class dyldCacheImageInfoExtra {
public:		
	uint64_t		exportsTrieAddr() const							INLINE { return E::get64(fields.exportsTrieAddr); }
	void			set_exportsTrieAddr(uint64_t value)				INLINE { E::set64(fields.exportsTrieAddr, value); }

	uint64_t		weakBindingsAddr() const						INLINE { return E::get64(fields.weakBindingsAddr); }
	void			set_weakBindingsAddr(uint64_t value)			INLINE { E::set64(fields.weakBindingsAddr, value); }

	uint32_t		exportsTrieSize() const							INLINE { return E::get32(fields.exportsTrieSize); }
	void			set_exportsTrieSize(uint32_t value)				INLINE { E::set32(fields.exportsTrieSize, value); }

	uint32_t		weakBindingsSize() const						INLINE { return E::get32(fields.weakBindingsSize); }
	void			set_weakBindingsSize(uint32_t value)			INLINE { E::set32(fields.weakBindingsSize, value); }

	uint32_t		dependentsStartArrayIndex() const				INLINE { return E::get32(fields.dependentsStartArrayIndex); }
	void			set_dependentsStartArrayIndex(uint32_t value)	INLINE { E::set32(fields.dependentsStartArrayIndex, value); }

	uint32_t		reExportsStartArrayIndex() const				INLINE { return E::get32(fields.reExportsStartArrayIndex); }
	void			set_reExportsStartArrayIndex(uint32_t value)	INLINE { E::set32(fields.reExportsStartArrayIndex, value); }

private:
	dyld_cache_image_info_extra			fields;
};


template <typename E>
class dyldCacheAcceleratorInfo {
public:		
	uint32_t		version() const								INLINE { return E::get32(fields.version); }
	void			set_version(uint32_t value)					INLINE { E::set32(fields.version, value); }

	uint32_t		imageExtrasCount() const					INLINE { return E::get32(fields.imageExtrasCount); }
	void			set_imageExtrasCount(uint32_t value)		INLINE { E::set32(fields.imageExtrasCount, value); }

	uint32_t		imagesExtrasOffset() const					INLINE { return E::get32(fields.imagesExtrasOffset); }
	void			set_imagesExtrasOffset(uint32_t value)		INLINE { E::set32(fields.imagesExtrasOffset, value); }

	uint32_t		bottomUpListOffset() const					INLINE { return E::get32(fields.bottomUpListOffset); }
	void			set_bottomUpListOffset(uint32_t value)		INLINE { E::set32(fields.bottomUpListOffset, value); }

	uint32_t		dylibTrieOffset() const						INLINE { return E::get32(fields.dylibTrieOffset); }
	void			set_dylibTrieOffset(uint32_t value)			INLINE { E::set32(fields.dylibTrieOffset, value); }

	uint32_t		dylibTrieSize() const						INLINE { return E::get32(fields.dylibTrieSize); }
	void			set_dylibTrieSize(uint32_t value)			INLINE { E::set32(fields.dylibTrieSize, value); }

	uint32_t		initializersOffset() const					INLINE { return E::get32(fields.initializersOffset); }
	void			set_initializersOffset(uint32_t value)		INLINE { E::set32(fields.initializersOffset, value); }

	uint32_t		initializersCount() const					INLINE { return E::get32(fields.initializersCount); }
	void			set_initializersCount(uint32_t value)		INLINE { E::set32(fields.initializersCount, value); }

	uint32_t		dofSectionsOffset() const					INLINE { return E::get32(fields.dofSectionsOffset); }
	void			set_dofSectionsOffset(uint32_t value)		INLINE { E::set32(fields.dofSectionsOffset, value); }

	uint32_t		dofSectionsCount() const					INLINE { return E::get32(fields.dofSectionsCount); }
	void			set_dofSectionsCount(uint32_t value)		INLINE { E::set32(fields.dofSectionsCount, value); }

	uint32_t		reExportListOffset() const					INLINE { return E::get32(fields.reExportListOffset); }
	void			set_reExportListOffset(uint32_t value)		INLINE { E::set32(fields.reExportListOffset, value); }

	uint32_t		reExportCount() const						INLINE { return E::get32(fields.reExportCount); }
	void			set_reExportCount(uint32_t value)			INLINE { E::set32(fields.reExportCount, value); }

	uint32_t		depListOffset() const						INLINE { return E::get32(fields.depListOffset); }
	void			set_depListOffset(uint32_t value)			INLINE { E::set32(fields.depListOffset, value); }

	uint32_t		depListCount() const						INLINE { return E::get32(fields.depListCount); }
	void			set_depListCount(uint32_t value)			INLINE { E::set32(fields.depListCount, value); }

	uint32_t		rangeTableOffset() const					INLINE { return E::get32(fields.rangeTableOffset); }
	void			set_rangeTableOffset(uint32_t value)		INLINE { E::set32(fields.rangeTableOffset, value); }

	uint32_t		rangeTableCount() const						INLINE { return E::get32(fields.rangeTableCount); }
	void			set_rangeTableCount(uint32_t value)			INLINE { E::set32(fields.rangeTableCount, value); }

	uint64_t		dyldSectionAddr() const						INLINE { return E::get64(fields.dyldSectionAddr); }
	void			set_dyldSectionAddr(uint64_t value)			INLINE { E::set64(fields.dyldSectionAddr, value); }


private:
	dyld_cache_accelerator_info			fields;
};


template <typename E>
class dyldCacheAcceleratorInitializer {
public:		
	uint32_t		functionOffset() const						INLINE { return E::get32(fields.functionOffset); }
	void			set_functionOffset(uint32_t value)			INLINE { E::set32(fields.functionOffset, value); }

	uint32_t		imageIndex() const							INLINE { return E::get32(fields.imageIndex); }
	void			set_imageIndex(uint32_t value)				INLINE { E::set32(fields.imageIndex, value); }

private:
	dyld_cache_accelerator_initializer			fields;
};


template <typename E>
class dyldCacheAcceleratorRangeEntry {
public:		
	uint64_t		startAddress() const						INLINE { return E::get64(fields.startAddress); }
	void			set_startAddress(uint64_t value)			INLINE { E::set64(fields.startAddress, value); }

	uint32_t		size() const								INLINE { return E::get32(fields.size); }
	void			set_size(uint32_t value)					INLINE { E::set32(fields.size, value); }

	uint32_t		imageIndex() const							INLINE { return E::get32(fields.imageIndex); }
	void			set_imageIndex(uint32_t value)				INLINE { E::set32(fields.imageIndex, value); }

private:
	dyld_cache_range_entry			fields;
};

template <typename E>
class dyldCacheAcceleratorDOFEntry {
public:		
	uint64_t		sectionAddress() const						INLINE { return E::get64(fields.sectionAddress); }
	void			set_sectionAddress(uint64_t value)			INLINE { E::set64(fields.sectionAddress, value); }

	uint32_t		sectionSize() const							INLINE { return E::get32(fields.sectionSize); }
	void			set_sectionSize(uint32_t value)				INLINE { E::set32(fields.sectionSize, value); }

	uint32_t		imageIndex() const							INLINE { return E::get32(fields.imageIndex); }
	void			set_imageIndex(uint32_t value)				INLINE { E::set32(fields.imageIndex, value); }

private:
	dyld_cache_accelerator_dof			fields;
};


template <typename E>
class dyldCacheSlideInfo {
public:		
	uint32_t		version() const								INLINE { return E::get32(fields.version); }
	void			set_version(uint32_t value)					INLINE { E::set32(fields.version, value); }

	uint32_t		toc_offset() const							INLINE { return E::get32(fields.toc_offset); }
	void			set_toc_offset(uint32_t value)				INLINE { E::set32(fields.toc_offset, value); }

	uint32_t		toc_count() const							INLINE { return E::get32(fields.toc_count); }
	void			set_toc_count(uint32_t value)				INLINE { E::set32(fields.toc_count, value); }

	uint32_t		entries_offset() const						INLINE { return E::get32(fields.entries_offset); }
	void			set_entries_offset(uint32_t value)			INLINE { E::set32(fields.entries_offset, value); }
	
	uint32_t		entries_count() const						INLINE { return E::get32(fields.entries_count); }
	void			set_entries_count(uint32_t value)			INLINE { E::set32(fields.entries_count, value); }

	uint32_t		entries_size() const						INLINE { return E::get32(fields.entries_size); }
	void			set_entries_size(uint32_t value)			INLINE { E::set32(fields.entries_size, value); }

	uint16_t		toc(unsigned index) const					INLINE { return E::get16(((uint16_t*)(((uint8_t*)this)+E::get16(fields.toc_offset)))[index]); }
	void			set_toc(unsigned index, uint16_t value) 	INLINE { return E::set16(((uint16_t*)(((uint8_t*)this)+E::get16(fields.toc_offset)))[index], value); }

private:
	dyld_cache_slide_info			fields;
};


struct dyldCacheSlideInfoEntry {
	uint8_t  bits[4096/(8*4)]; // 128-byte bitmap
};


template <typename E>
class dyldCacheSlideInfo2 {
public:		
	uint32_t		version() const								INLINE { return E::get32(fields.version); }
	void			set_version(uint32_t value)					INLINE { E::set32(fields.version, value); }

	uint32_t		page_starts_offset() const					INLINE { return E::get32(fields.page_starts_offset); }
	void			set_page_starts_offset(uint32_t value)		INLINE { E::set32(fields.page_starts_offset, value); }

	uint32_t		page_starts_count() const					INLINE { return E::get32(fields.page_starts_count); }
	void			set_page_starts_count(uint32_t value)		INLINE { E::set32(fields.page_starts_count, value); }

	uint32_t		page_extras_offset() const					INLINE { return E::get32(fields.page_extras_offset); }
	void			set_page_extras_offset(uint32_t value)		INLINE { E::set32(fields.page_extras_offset, value); }

	uint32_t		page_extras_count() const					INLINE { return E::get32(fields.page_extras_count); }
	void			set_page_extras_count(uint32_t value)		INLINE { E::set32(fields.page_extras_count, value); }

	uint32_t		page_size() const							INLINE { return E::get32(fields.page_size); }
	void			set_page_size(uint32_t value)				INLINE { E::set32(fields.page_size, value); }

	uint64_t		delta_mask() const							INLINE { return E::get64(fields.delta_mask); }
	void			set_delta_mask(uint64_t value)				INLINE { E::set64(fields.delta_mask, value); }

	uint64_t		value_add() const							INLINE { return E::get64(fields.value_add); }
	void			set_value_add(uint64_t value)				INLINE { E::set64(fields.value_add, value); }

	uint16_t		page_starts(unsigned index) const				INLINE { return E::get16(((uint16_t*)(((uint8_t*)this)+E::get16(fields.page_starts_offset)))[index]); }
	void			set_page_starts(unsigned index, uint16_t value) INLINE { return E::set16(((uint16_t*)(((uint8_t*)this)+E::get16(fields.page_starts_offset)))[index], value); }

	uint16_t		page_extras(unsigned index) const				INLINE { return E::get16(((uint16_t*)(((uint8_t*)this)+E::get16(fields.page_extras_offset)))[index]); }
	void			set_page_extras(unsigned index, uint16_t value) INLINE { return E::set16(((uint16_t*)(((uint8_t*)this)+E::get16(fields.page_extras_offset)))[index], value); }


private:
	dyld_cache_slide_info2			fields;
};


template <typename E>
class dyldCacheSlideInfo3 {
public:
    uint32_t        version() const                                INLINE { return E::get32(fields.version); }
    void            set_version(uint32_t value)                    INLINE { E::set32(fields.version, value); }

    uint32_t        page_starts_count() const                    INLINE { return E::get32(fields.page_starts_count); }
    void            set_page_starts_count(uint32_t value)        INLINE { E::set32(fields.page_starts_count, value); }

    uint32_t        page_size() const                            INLINE { return E::get32(fields.page_size); }
    void            set_page_size(uint32_t value)                INLINE { E::set32(fields.page_size, value); }

    uint64_t        auth_value_add() const                       INLINE { return E::get64(fields.auth_value_add); }
    void            set_auth_value_add(uint64_t value)           INLINE { E::set64(fields.auth_value_add, value); }

    uint16_t        page_starts(unsigned index) const                INLINE { return E::get16(fields.page_starts[index]); }
    void            set_page_starts(unsigned index, uint16_t value) INLINE { E::set16(fields.page_starts[index], value); }


private:
    dyld_cache_slide_info3            fields;
};



template <typename E>
class dyldCacheLocalSymbolsInfo {
public:		
	uint32_t		nlistOffset() const							INLINE { return E::get32(fields.nlistOffset); }
	void			set_nlistOffset(uint32_t value)				INLINE { E::set32(fields.nlistOffset, value); }

	uint32_t		nlistCount() const							INLINE { return E::get32(fields.nlistCount); }
	void			set_nlistCount(uint32_t value)				INLINE { E::set32(fields.nlistCount, value); }

	uint32_t		stringsOffset() const						INLINE { return E::get32(fields.stringsOffset); }
	void			set_stringsOffset(uint32_t value)			INLINE { E::set32(fields.stringsOffset, value); }
	
	uint32_t		stringsSize() const							INLINE { return E::get32(fields.stringsSize); }
	void			set_stringsSize(uint32_t value)				INLINE { E::set32(fields.stringsSize, value); }

	uint32_t		entriesOffset() const						INLINE { return E::get32(fields.entriesOffset); }
	void			set_entriesOffset(uint32_t value)			INLINE { E::set32(fields.entriesOffset, value); }

	uint32_t		entriesCount() const						INLINE { return E::get32(fields.entriesCount); }
	void			set_entriesCount(uint32_t value)			INLINE { E::set32(fields.entriesCount, value); }

private:
	dyld_cache_local_symbols_info			fields;
};


template <typename E>
class dyldCacheLocalSymbolEntry {
public:		
	uint32_t		dylibOffset() const							INLINE { return E::get32(fields.dylibOffset); }
	void			set_dylibOffset(uint32_t value)				INLINE { E::set32(fields.dylibOffset, value); }

	uint32_t		nlistStartIndex() const						INLINE { return E::get32(fields.nlistStartIndex); }
	void			set_nlistStartIndex(uint32_t value)			INLINE { E::set32(fields.nlistStartIndex, value); }

	uint32_t		nlistCount() const							INLINE { return E::get32(fields.nlistCount); }
	void			set_nlistCount(uint32_t value)				INLINE { E::set32(fields.nlistCount, value); }
	
private:
	dyld_cache_local_symbols_entry			fields;
};




#endif // __DYLD_CACHE_ABSTRACTION__



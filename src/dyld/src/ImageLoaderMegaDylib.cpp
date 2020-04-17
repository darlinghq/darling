/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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


#if __arm__ || __arm64__
  #include <System/sys/mman.h>
#else
  #include <sys/mman.h>
#endif
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h> 
#include <sys/param.h>
#include <mach/mach.h>
#include <mach/thread_status.h>
#include <mach-o/loader.h> 
#include <mach-o/dyld_images.h>
#include <libkern/OSAtomic.h>

#include "ImageLoaderMegaDylib.h"
#include "ImageLoaderMachO.h"
#include "dyldLibSystemInterface.h"
#include "Tracing.h"
#include "dyld2.h"

// from dyld_gdb.cpp 
extern void addImagesToAllImages(uint32_t infoCount, const dyld_image_info info[]);

extern "C" int _dyld_func_lookup(const char* name, void** address);


#ifndef EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE
	#define EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE			0x02
#endif

// relocation_info.r_length field has value 3 for 64-bit executables and value 2 for 32-bit executables
#if __LP64__
	#define RELOC_SIZE 3
	#define LC_SEGMENT_COMMAND		LC_SEGMENT_64
	#define LC_ROUTINES_COMMAND		LC_ROUTINES_64
	struct macho_segment_command	: public segment_command_64  {};
	struct macho_section			: public section_64  {};	
	struct macho_routines_command	: public routines_command_64  {};	
#else
	#define RELOC_SIZE 2
	#define LC_SEGMENT_COMMAND		LC_SEGMENT
	#define LC_ROUTINES_COMMAND		LC_ROUTINES
	struct macho_segment_command	: public segment_command {};
	struct macho_section			: public section  {};	
	struct macho_routines_command	: public routines_command  {};	
#endif



#if SUPPORT_ACCELERATE_TABLES
 

ImageLoaderMegaDylib* ImageLoaderMegaDylib::makeImageLoaderMegaDylib(const dyld_cache_header* header, long slide, const macho_header* mainMH, const LinkContext& context)
{
	return new ImageLoaderMegaDylib(header, slide, mainMH, context);
}

struct DATAdyld {
	void*			dyldLazyBinder;		// filled in at launch by dyld to point into dyld to &stub_binding_helper
	void*			dyldFuncLookup;		// filled in at launch by dyld to point into dyld to &_dyld_func_lookup
	ProgramVars		vars;
};




ImageLoaderMegaDylib::ImageLoaderMegaDylib(const dyld_cache_header* header, long slide, const macho_header* mainMH, const LinkContext& context)
	: ImageLoader("dyld shared cache", 0), _header(header), _linkEditBias(NULL), _slide(slide),
	 _lockArray(NULL), _lockArrayInUseCount(0)
{
	pthread_mutex_init(&_lockArrayGuard, NULL);
	const dyld_cache_mapping_info* mappings = (const dyld_cache_mapping_info*)((uint8_t*)_header + _header->mappingOffset);
	const dyld_cache_mapping_info* lastMapping = &mappings[_header->mappingCount-1];
	const dyld_cache_accelerator_info* accHeader = (dyld_cache_accelerator_info*)(_header->accelerateInfoAddr + slide);
	for (const dyld_cache_mapping_info* m=mappings; m <= lastMapping; ++m) {
		if ( m->initProt == VM_PROT_READ ) {
			_linkEditBias = (uint8_t*)(m->address - m->fileOffset) + _slide;
		}
	}

	_endOfCacheInMemory = (void*)(lastMapping->address + lastMapping->size + slide);
	_images = (const dyld_cache_image_info*)((uint8_t*)_header + _header->imagesOffset);
	_imageExtras = (dyld_cache_image_info_extra*)((char*)accHeader + accHeader->imagesExtrasOffset);
	_initializers = (dyld_cache_accelerator_initializer*)((char*)accHeader + accHeader->initializersOffset);
	_reExportsArray = (uint16_t*)((char*)accHeader + accHeader->reExportListOffset);
	_dependenciesArray = (uint16_t*)((char*)accHeader + accHeader->depListOffset);
	_bottomUpArray = (uint16_t*)((char*)accHeader + accHeader->bottomUpListOffset);
	_rangeTable = (dyld_cache_range_entry*)((char*)accHeader + accHeader->rangeTableOffset);
	_rangeTableCount = accHeader->rangeTableCount;
	_imageCount = accHeader->imageExtrasCount;
	_stateFlags = (uint8_t*)calloc(_imageCount, 1);
	_initializerCount = accHeader->initializersCount;
	_dylibsTrieStart = (uint8_t*)accHeader + accHeader->dylibTrieOffset;
	_dylibsTrieEnd = _dylibsTrieStart + accHeader->dylibTrieSize;
	_imageTextInfo = (dyld_cache_image_text_info*)((uint8_t*)_header + _header->imagesTextOffset);
	DATAdyld* dyldSection = (DATAdyld*)(accHeader->dyldSectionAddr + slide);
	dyldSection->dyldLazyBinder = NULL; // not used by libdyld.dylib
	dyldSection->dyldFuncLookup = (void*)&_dyld_func_lookup;
	dyldSection->vars.mh = mainMH;
	context.setNewProgramVars(dyldSection->vars);
}


void ImageLoaderMegaDylib::unreachable() const
{
	abort();
}

ImageLoaderMegaDylib::~ImageLoaderMegaDylib()
{
}

const char*	ImageLoaderMegaDylib::getInstallPath() const {
	unreachable();
}

const macho_header* ImageLoaderMegaDylib::getIndexedMachHeader(unsigned index) const
{
	if ( index > _header->imagesCount )
		dyld::throwf("cache image index out of range (%u), max=%u", index, _header->imagesCount - 1);
	return (const macho_header*)(_images[index].address + _slide);
}

const char* ImageLoaderMegaDylib::getIndexedPath(unsigned index) const
{
	if ( index > _header->imagesCount )
		dyld::throwf("cache image index out of range (%u), max=%u", index, _header->imagesCount - 1);
	return (char*)_header + _images[index].pathFileOffset;
}

const char* ImageLoaderMegaDylib::getIndexedShortName(unsigned index) const
{
	const char* path = getIndexedPath(index);
	const char* lastSlash = strrchr(path, '/');
	if ( lastSlash == NULL )
		return path;
	else
		return lastSlash+1;
}

void ImageLoaderMegaDylib::getDylibUUID(unsigned int index, uuid_t uuid) const
{
	if ( index > _header->imagesCount )
		dyld::throwf("cache image index out of range (%u), max=%u", index, _header->imagesCount - 1);
	memcpy(uuid, _imageTextInfo[index].uuid, 16);
}

void ImageLoaderMegaDylib::printSegments(const macho_header* mh) const
{
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)((uint8_t*)mh + sizeof(macho_header));
	const struct load_command* cmd = cmds;
	const macho_segment_command* seg;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
				seg = (macho_segment_command*)cmd;
				dyld::log("%18s at 0x%08lX->0x%08lX\n", seg->segname, (long)(seg->vmaddr + _slide), (long)(seg->vmaddr + seg->vmsize + _slide));
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
}

bool ImageLoaderMegaDylib::hasDylib(const char* path, unsigned* index) const
{
	const uint8_t* imageNode = ImageLoader::trieWalk(_dylibsTrieStart, _dylibsTrieEnd, path);
	if ( imageNode == NULL ) {
  #if __MAC_OS_X_VERSION_MIN_REQUIRED
		// not all symlinks are recorded as aliases in accelerator tables
		if ( (strncmp(path, "/usr/lib/", 9) == 0) || (strncmp(path, "/System/Library/", 16) == 0) ) {
			char resolvedPath[PATH_MAX];
			if ( realpath(path, resolvedPath) != NULL ) {
				imageNode = ImageLoader::trieWalk(_dylibsTrieStart, _dylibsTrieEnd, resolvedPath);
			}
		}
		if ( imageNode == NULL )
			return false;
  #else
		return false;
  #endif
	}
	*index = (unsigned)read_uleb128(imageNode, _dylibsTrieEnd);
	return true;
}

bool ImageLoaderMegaDylib::addressInCache(const void* address, const mach_header** mh, const char** path, unsigned* index)
{
	// quick out of bounds check
#if __x86_64__
	if ( (uintptr_t)address < 0x7FFF70000000LL )
		return false;
#else
	if ( address < (void*)_header )
		return false;
#endif
	if ( address > _endOfCacheInMemory )
		return false;

	uint64_t unslidAddress = (uint64_t)address - _slide;
	// linear search for now
	const dyld_cache_range_entry* rangeTableEnd = &_rangeTable[_rangeTableCount];
	for (const dyld_cache_range_entry* r = _rangeTable; r < rangeTableEnd; ++r) {
		if ( (r->startAddress <= unslidAddress) && (unslidAddress < r->startAddress+r->size) ) {
			*index = r->imageIndex;
			*mh = (mach_header*)getIndexedMachHeader(r->imageIndex);
			*path = getIndexedPath(r->imageIndex);
			return true;
		}
	}

	return false;
}


bool ImageLoaderMegaDylib::findUnwindSections(const void* address, dyld_unwind_sections* info)
{
	const char* path;
	unsigned index;
	if ( addressInCache(address, &info->mh, &path, &index) ) {
		info->dwarf_section = NULL;
		info->dwarf_section_length = 0;
		ImageLoaderMachO::findSection(info->mh, "__TEXT", "__eh_frame", (void**)&info->dwarf_section, &info->dwarf_section_length);

		info->compact_unwind_section = NULL;
		info->compact_unwind_section_length = 0;
		ImageLoaderMachO::findSection(info->mh, "__TEXT", "__unwind_info", (void**)&info->compact_unwind_section, &info->compact_unwind_section_length);

		return true;
	}
	return false;
}


unsigned ImageLoaderMegaDylib::findImageIndex(const LinkContext& context, const char* path) const
{
	unsigned index;
	if ( hasDylib(path, &index) )
		return index;

	if ( strncmp(path, "@rpath/libswift", 15) == 0 ) {
		// <rdar://problem/51352017> a stable swift app built to run on pre-iOS-12.2 will use @rpath to reference swift dylibs in OS
		const char* trailingPath = &path[7];
		char possiblePath[strlen(trailingPath)+16];
		strcpy(possiblePath, "/usr/lib/swift/");
		strcat(possiblePath, trailingPath);
		if ( hasDylib(possiblePath, &index) )
			return index;
	}
	else if ( strncmp(path, "@rpath/", 7) == 0 ) {
		// <rdar://problem/26934069> Somehow we found the dylib in the cache, but it is not this literal string, try simple expansions of @rpath
		std::vector<const char*> rpathsFromMainExecutable;
		context.mainExecutable->getRPaths(context, rpathsFromMainExecutable);
		rpathsFromMainExecutable.push_back("/System/Library/Frameworks/");
		rpathsFromMainExecutable.push_back("/usr/lib/swift/");
		const char* trailingPath = &path[7];
		for (const char* anRPath : rpathsFromMainExecutable) {
			if ( anRPath[0] != '/' )
				continue;
			char possiblePath[strlen(anRPath) + strlen(trailingPath)+2];
			strcpy(possiblePath, anRPath);
			if ( possiblePath[strlen(possiblePath)-1] != '/' )
				strcat(possiblePath, "/");
			strcat(possiblePath, trailingPath);
			if ( hasDylib(possiblePath, &index) ) {
				return index;
			}
		}
	}
    else {
        // handle symlinks embedded in load commands
        char resolvedPath[PATH_MAX];
        realpath(path, resolvedPath);
        int realpathErrno = errno;
        // If realpath() resolves to a path which does not exist on disk, errno is set to ENOENT
        if ( (realpathErrno == ENOENT) || (realpathErrno == 0) ) {
            if ( strcmp(resolvedPath, path) != 0 )
                return findImageIndex(context, resolvedPath);
        }
    }

	dyld::throwf("no cache image with name (%s)", path);
}

void ImageLoaderMegaDylib::initializeCoalIterator(CoalIterator& it, unsigned int loadOrder, unsigned imageIndex)
{
	it.image = this;
	it.symbolName = " ";
	it.loadOrder = loadOrder;
	it.weakSymbol = false;
	it.symbolMatches = false;
	it.done = false;
	it.curIndex = 0;
	it.endIndex = _imageExtras[imageIndex].weakBindingsSize;
	it.address = 0;
	it.type = 0;
	it.addend = 0;
	it.imageIndex = imageIndex;
}

bool ImageLoaderMegaDylib::incrementCoalIterator(CoalIterator& it)
{
	if ( it.done )
		return false;
		
	if ( _imageExtras[it.imageIndex].weakBindingsSize == 0 ) {
		/// hmmm, ld set MH_WEAK_DEFINES or MH_BINDS_TO_WEAK, but there is no weak binding info
		it.done = true;
		it.symbolName = "~~~";
		return true;
	}
	const uint8_t* start = (uint8_t*)(_imageExtras[it.imageIndex].weakBindingsAddr + _slide);
	const uint8_t* end = (uint8_t*)(_imageExtras[it.imageIndex].weakBindingsAddr + _slide + _imageExtras[it.imageIndex].weakBindingsSize);
	const uint8_t* p = start + it.curIndex;
	uintptr_t count;
	uintptr_t skip;
	uint64_t segOffset;
	unsigned segIndex;
	const mach_header* mh;
	while ( p < end ) {
		uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
		uint8_t opcode = *p & BIND_OPCODE_MASK;
		++p;
		switch (opcode) {
			case BIND_OPCODE_DONE:
				it.done = true;
				it.curIndex = p - start;
				it.symbolName = "~~~"; // sorts to end
				return true;
			case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
				it.symbolName = (char*)p;
				it.weakSymbol = ((immediate & BIND_SYMBOL_FLAGS_NON_WEAK_DEFINITION) == 0);
				it.symbolMatches = false;
				while (*p != '\0')
					++p;
				++p;
				it.curIndex = p - start;
				return false;
			case BIND_OPCODE_SET_TYPE_IMM:
				it.type = immediate;
				break;
			case BIND_OPCODE_SET_ADDEND_SLEB:
				it.addend = read_sleb128(p, end);
				break;
			case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
				segIndex = immediate;
				segOffset = read_uleb128(p, end);
				mh = (mach_header*)getIndexedMachHeader((unsigned)it.imageIndex);
				if ( uintptr_t segPrefAddress = ImageLoaderMachO::segPreferredAddress(mh, segIndex) )
					it.address = segPrefAddress + (uintptr_t)segOffset + _slide;
				else
					dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has segment %d which is too large", segIndex);
				break;
			case BIND_OPCODE_ADD_ADDR_ULEB:
				it.address += read_uleb128(p, end);
				break;
			case BIND_OPCODE_DO_BIND:
				it.address += sizeof(intptr_t);
				break;
			case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
				it.address += read_uleb128(p, end) + sizeof(intptr_t);
				break;
			case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
				it.address += immediate*sizeof(intptr_t) + sizeof(intptr_t);
				break;
			case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
				count = read_uleb128(p, end);
				skip = read_uleb128(p, end);
				for (uint32_t i=0; i < count; ++i) {
					it.address += skip + sizeof(intptr_t);
				}
				break;
			default:
				dyld::throwf("bad weak bind opcode '%d' found after processing %d bytes in '%s'", *p, (int)(p-start), this->getPath());
		}
	}
	/// hmmm, BIND_OPCODE_DONE is missing...
	it.done = true;
	it.symbolName = "~~~";
	//dyld::log("missing BIND_OPCODE_DONE for image %s\n", this->getPath());
	return true;
}

uintptr_t ImageLoaderMegaDylib::getAddressCoalIterator(CoalIterator& it, const LinkContext& context)
{
	//dyld::log("looking for %s in %s\n", it.symbolName, this->getPath());
	uintptr_t address;
	if ( findInChainedTries(context, it.symbolName, (unsigned)it.imageIndex, NULL, false, &address) ) {
		return address;
	}
	return 0;
}

void ImageLoaderMegaDylib::updateUsesCoalIterator(CoalIterator& it, uintptr_t value, ImageLoader* targetImage, unsigned targetIndex, const LinkContext& context)
{

	const uint8_t* start = (uint8_t*)(_imageExtras[it.imageIndex].weakBindingsAddr + _slide);
	const uint8_t* end = (uint8_t*)(_imageExtras[it.imageIndex].weakBindingsAddr + _slide + _imageExtras[it.imageIndex].weakBindingsSize);
	const uint8_t* p = start + it.curIndex;

	uint8_t type = it.type;
	uintptr_t address = it.address;
	const char* symbolName = it.symbolName;
	intptr_t addend = it.addend;
	uint64_t segOffset;
	unsigned segIndex;
	const mach_header* mh;
	uintptr_t count;
	uintptr_t skip;
	bool done = false;
	bool boundSomething = false;
	const char* targetImagePath = targetImage ? targetImage->getIndexedPath(targetIndex) : NULL;
	while ( !done && (p < end) ) {
		uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
		uint8_t opcode = *p & BIND_OPCODE_MASK;
		++p;
		switch (opcode) {
			case BIND_OPCODE_DONE:
				done = true;
				break;
			case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
				done = true;
				break;
			case BIND_OPCODE_SET_TYPE_IMM:
				type = immediate;
				break;
			case BIND_OPCODE_SET_ADDEND_SLEB:
				addend = read_sleb128(p, end);
				break;
			case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
				segIndex = immediate;
				segOffset = read_uleb128(p, end);
				mh = (mach_header*)getIndexedMachHeader((unsigned)it.imageIndex);
				if ( uintptr_t segPrefAddress = ImageLoaderMachO::segPreferredAddress(mh, segIndex) )
					address = segPrefAddress + (uintptr_t)segOffset + _slide;
				else
					dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has segment %d which is too large", segIndex);
				break;
			case BIND_OPCODE_ADD_ADDR_ULEB:
				address += read_uleb128(p, end);
				break;
			case BIND_OPCODE_DO_BIND:
				ImageLoaderMachO::bindLocation(context, 0, address, value, type, symbolName, addend, getIndexedPath((unsigned)it.imageIndex), targetImagePath, "weak ", NULL, _slide);
				boundSomething = true;
				address += sizeof(intptr_t);
				break;
			case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
				ImageLoaderMachO::bindLocation(context, 0, address, value, type, symbolName, addend, getIndexedPath((unsigned)it.imageIndex), targetImagePath, "weak ", NULL, _slide);
				boundSomething = true;
				address += read_uleb128(p, end) + sizeof(intptr_t);
				break;
			case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
				ImageLoaderMachO::bindLocation(context, 0, address, value, type, symbolName, addend, getIndexedPath((unsigned)it.imageIndex), targetImagePath, "weak ", NULL, _slide);
				boundSomething = true;
				address += immediate*sizeof(intptr_t) + sizeof(intptr_t);
				break;
			case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
				count = read_uleb128(p, end);
				skip = read_uleb128(p, end);
				for (uint32_t i=0; i < count; ++i) {
					ImageLoaderMachO::bindLocation(context, 0, address, value, type, symbolName, addend, getIndexedPath((unsigned)it.imageIndex), targetImagePath, "weak ", NULL, _slide);
					boundSomething = true;
					address += skip + sizeof(intptr_t);
				}
				break;
			default:
				dyld::throwf("bad bind opcode %d in weak binding info", *p);
		}
	}
	// C++ weak coalescing cannot be tracked by reference counting.  Error on side of never unloading.
	if ( boundSomething && (targetImage != this) )
		context.addDynamicReference(this, targetImage);
}


void ImageLoaderMegaDylib::appendImagesNeedingCoalescing(ImageLoader* images[], unsigned imageIndexes[], unsigned& count)
{
	for (unsigned i=0; i < _imageCount; ++i) {
		uint16_t index = _bottomUpArray[i];
		if ( _stateFlags[index] == kStateUnused )
			continue;
		if ( _imageExtras[index].weakBindingsSize == 0 )
			continue;
		images[count] = this;
		imageIndexes[count] = index;
		++count;
	}
}


bool ImageLoaderMegaDylib::weakSymbolsBound(unsigned index) const
{
	return ( _stateFlags[index] >= kStateFlagWeakBound );
}

void ImageLoaderMegaDylib::setWeakSymbolsBound(unsigned index)
{
	if ( _stateFlags[index] == kStateFlagBound )
		_stateFlags[index] = kStateFlagWeakBound;
}


void ImageLoaderMegaDylib::recursiveMarkLoaded(const LinkContext& context, unsigned imageIndex)
{
	if ( _stateFlags[imageIndex] != kStateUnused )
		return;

	const macho_header* mh = getIndexedMachHeader(imageIndex);
	const char* path = getIndexedPath(imageIndex);

	if ( context.verboseLoading )
		dyld::log("dyld: loaded: %s\n", path);
	if ( context.verboseMapping ) {
		dyld::log("dyld: Using shared cached for %s\n", path);
		printSegments(mh);
	}
	
	// change state to "loaded" before recursing to break cycles
	_stateFlags[imageIndex] = kStateLoaded;
	++fgImagesUsedFromSharedCache;

	dyld_image_info debuggerInfo;
	debuggerInfo.imageLoadAddress = (mach_header*)mh;
	debuggerInfo.imageFilePath = path;
	debuggerInfo.imageFileModDate = 0;
	addImagesToAllImages(1, &debuggerInfo);

	if ( _imageExtras[imageIndex].weakBindingsSize != 0 ) {
		++fgImagesRequiringCoalescing;
		++fgImagesHasWeakDefinitions;
	}

	unsigned startArrayIndex = _imageExtras[imageIndex].dependentsStartArrayIndex;
	for (int i=startArrayIndex; _dependenciesArray[i] != 0xFFFF; ++i) {
		unsigned subDep = (_dependenciesArray[i] & 0x7FFF); // mask off upward bit
		recursiveMarkLoaded(context, subDep);
	}
}

void ImageLoaderMegaDylib::recursiveLoadLibraries(const LinkContext& context, bool preflightOnly, const RPathChain& loaderRPaths, const char* loadPath)
{
	unsigned index = findImageIndex(context, loadPath);
	recursiveMarkLoaded(context, index);
}

unsigned int ImageLoaderMegaDylib::recursiveUpdateDepth(unsigned int maxDepth)
{
	setDepth(maxDepth);
	return maxDepth;
}


const ImageLoader::Symbol* ImageLoaderMegaDylib::findExportedSymbol(const char* name, bool searchReExports, const char* thisPath, const ImageLoader** foundIn) const
{
	unsigned index;
	if ( !hasDylib(thisPath, &index) )
		return NULL;
	const uint8_t* exportNode;
	const uint8_t* exportTrieEnd;
	unsigned foundinIndex;
	// <rdar://problem/22068598> always search re-exports
	// the point of searchReExports was to break cycles in dylibs, we don't have cycles in cache, so ok to search deep
	searchReExports = true;
	if ( searchReExports ) {
		if ( !exportTrieHasNodeRecursive(name, index, &exportNode, &exportTrieEnd, &foundinIndex) )
			return NULL;
	}
	else {
		if ( !exportTrieHasNode(name, index, &exportNode, &exportTrieEnd) )
			return NULL;
	}
	*foundIn = this;
	return (ImageLoader::Symbol*)exportNode;
}

bool ImageLoaderMegaDylib::exportTrieHasNode(const char* symbolName, unsigned index,
											 const uint8_t** exportNode, const uint8_t** exportTrieEnd) const
{
	const uint8_t* start = (uint8_t*)(_imageExtras[index].exportsTrieAddr + _slide);
	const uint32_t size = _imageExtras[index].exportsTrieSize;
	if ( size == 0 )
		return false;
	const uint8_t* end = start + size;
	const uint8_t* node = ImageLoader::trieWalk(start, end, symbolName);
	if ( node == NULL )
		return false;
	*exportNode = node;
	*exportTrieEnd = end;
	return true;
}

bool ImageLoaderMegaDylib::exportTrieHasNodeRecursive(const char* symbolName, unsigned index,
														const uint8_t** exportNode, const uint8_t** exportTrieEnd,
														unsigned* foundinIndex) const
{
	// look in trie for image index
	if ( exportTrieHasNode(symbolName, index, exportNode, exportTrieEnd) ) {
		*foundinIndex = index;
		return true;
	}
	// recursively look in all re-exported tries
	unsigned startArrayIndex = _imageExtras[index].reExportsStartArrayIndex;
	for (int i=startArrayIndex; _reExportsArray[i] != 0xFFFF; ++i) {
		unsigned reExIndex = _reExportsArray[i];
		if ( exportTrieHasNodeRecursive(symbolName, reExIndex, exportNode, exportTrieEnd, foundinIndex) )
			return true;
	}
	return false;
}

bool ImageLoaderMegaDylib::findExportedSymbolAddress(const LinkContext& context, const char* symbolName,
													const ImageLoader* requestorImage, int requestorOrdinalOfDef,
													bool runResolver, const ImageLoader** foundIn, uintptr_t* address) const
{
	const char* definedImagePath = requestorImage->libPath(requestorOrdinalOfDef-1);
	unsigned index = findImageIndex(context, definedImagePath);
	*foundIn = this;
	return findInChainedTries(context, symbolName, index, requestorImage, runResolver, address);
}

uintptr_t ImageLoaderMegaDylib::getExportedSymbolAddress(const Symbol* sym, const LinkContext& context,
														 const ImageLoader* requestor, bool runResolver, const char* symbolName) const
{
	// scan for with trie contains this node
	const uint8_t* exportTrieEnd = NULL;
	unsigned imageIndex = 0xFFFF;
	const macho_header* mh = NULL;
	uint64_t unslidTrieNode = ((uintptr_t)sym) - _slide;
	for (unsigned i=0; i < _imageCount; ++i) {
		uint64_t start = _imageExtras[i].exportsTrieAddr;
		uint64_t end   = _imageExtras[i].exportsTrieAddr +  _imageExtras[i].exportsTrieSize;
		if ( (start < unslidTrieNode) && (unslidTrieNode < end) ) {
			exportTrieEnd = (uint8_t*)(end + _slide);
			imageIndex = i;
			mh = (macho_header*)(_images[imageIndex].address + _slide);
			break;
		}
	}

	if ( mh == NULL )
		dyld::throwf("getExportedSymbolAddress(Symbol=%p) not in a cache trie", sym);

	const uint8_t* exportNode = (const uint8_t*)sym;
	uintptr_t address;
	processExportNode(context, symbolName ? symbolName : "unknown", imageIndex, exportNode, exportTrieEnd, requestor, runResolver, &address);
	return address;
}

void ImageLoaderMegaDylib::processExportNode(const LinkContext& context, const char* symbolName, unsigned definedImageIndex,
											const uint8_t* exportNode, const uint8_t* exportTrieEnd,
											  const ImageLoader* requestorImage, bool runResolver, uintptr_t* address) const
{
	const macho_header* mh = getIndexedMachHeader(definedImageIndex);
	uintptr_t flags = read_uleb128(exportNode, exportTrieEnd);
	uintptr_t rawAddress;
	switch ( flags & EXPORT_SYMBOL_FLAGS_KIND_MASK ) {
		case EXPORT_SYMBOL_FLAGS_KIND_REGULAR:
			if ( runResolver && (flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER) ) {
				// this node has a stub and resolver, run the resolver to get target address
				uintptr_t stub = read_uleb128(exportNode, exportTrieEnd) + (uintptr_t)mh; // skip over stub
				// <rdar://problem/10657737> interposing dylibs have the stub address as their replacee
				uintptr_t interposedStub = interposedAddress(context, stub, requestorImage);
				if ( interposedStub != stub ) {
					*address = interposedStub;
					return;
				}
				// stub was not interposed, so run resolver
				typedef uintptr_t (*ResolverProc)(void);
				ResolverProc resolver = (ResolverProc)(read_uleb128(exportNode, exportTrieEnd) + (uintptr_t)mh);
				*address = (*resolver)();
				if ( context.verboseBind )
					dyld::log("dyld: resolver at %p returned 0x%08lX\n", resolver, *address);
				return;
			}
			if ( flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
				// re-export from another dylib, lookup there
				const uintptr_t ordinal = read_uleb128(exportNode, exportTrieEnd);
				const char* importedName = (char*)exportNode;
				if ( importedName[0] == '\0' )
					importedName = symbolName;
				unsigned startArrayIndex = _imageExtras[definedImageIndex].dependentsStartArrayIndex;
				unsigned reExImageIndex = _dependenciesArray[startArrayIndex + ordinal-1] & 0x7FFF;
				if ( findInChainedTries(context, importedName, reExImageIndex, requestorImage, runResolver, address) )
					return;
				dyld::throwf("re-exported symbol '%s' not found for image %s expected re-exported in %s, node=%p",
							symbolName, getIndexedShortName(definedImageIndex), getIndexedShortName(reExImageIndex), exportNode);
			}
			rawAddress = read_uleb128(exportNode, exportTrieEnd) + (uintptr_t)mh;
			*address = interposedAddress(context, rawAddress, requestorImage);
			return;
		case EXPORT_SYMBOL_FLAGS_KIND_THREAD_LOCAL:
			if ( flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER )
				dyld::throwf("unsupported exported symbol kind. flags=%lu at node=%p", flags, exportNode);
			*address = read_uleb128(exportNode, exportTrieEnd) + (uintptr_t)mh;
			return;
		case EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE:
			if ( flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER )
				dyld::throwf("unsupported exported symbol kind. flags=%lu at node=%p", flags, exportNode);
			*address = read_uleb128(exportNode, exportTrieEnd);
			return;
		default:
			dyld::throwf("unsupported exported symbol kind. flags=%lu at node=%p", flags, exportNode);
	}
	//dyld::throwf("unsupported exported symbol node=%p", exportNode);
}

bool ImageLoaderMegaDylib::findInChainedTries(const LinkContext& context, const char* symbolName, unsigned definedImageIndex,
											  const ImageLoader* requestorImage, bool runResolver, uintptr_t* address) const
{
	//dyld::log("findInChainedTries(sym=%s, index=%u, path=%s)\n", symbolName, definedImageIndex, getIndexedPath(definedImageIndex));
	const uint8_t* exportNode;
	const uint8_t* exportTrieEnd;
	unsigned foundinIndex;
	if ( !exportTrieHasNodeRecursive(symbolName, definedImageIndex, &exportNode, &exportTrieEnd, &foundinIndex) )
		return false;

	processExportNode(context, symbolName, foundinIndex, exportNode, exportTrieEnd, requestorImage, runResolver, address);
	return true;
}


bool ImageLoaderMegaDylib::findInChainedTriesAndDependentsExcept(const LinkContext& context, const char* symbolName, unsigned imageIndex,
																const ImageLoader* requestorImage, bool runResolver, bool alreadyVisited[], uintptr_t* address) const
{
	//dyld::log("findInChainedTriesAndDependentsExcept(sym=%s, index=%u, path=%s)\n", symbolName, imageIndex, getIndexedPath(imageIndex));
	if ( alreadyVisited[imageIndex] )
		return false;
	alreadyVisited[imageIndex] = true;

	if ( findInChainedTries(context, symbolName, imageIndex, requestorImage, runResolver, address) )
		return true;

	unsigned startArrayIndex = _imageExtras[imageIndex].dependentsStartArrayIndex;
	for (int i=startArrayIndex; _dependenciesArray[i] != 0xFFFF; ++i) {
		// ignore upward links
		if ( (_dependenciesArray[i] & 0x8000) == 0 ) {
			unsigned depIndex = _dependenciesArray[i] & 0x7FFF;
			if ( _stateFlags[depIndex] != kStateFlagInitialized )
				continue;
			if ( findInChainedTriesAndDependentsExcept(context, symbolName, depIndex, requestorImage, runResolver, alreadyVisited, address) )
				return true;
		}
	}
	return false;
}

bool ImageLoaderMegaDylib::findInChainedTriesAndDependents(const LinkContext& context, const char* symbolName, unsigned definedImageIndex,
															const ImageLoader* requestorImage, bool runResolver, uintptr_t* address) const
{
	//dyld::log("findInChainedTriesAndDependents(sym=%s, index=%u, path=%s)\n", symbolName, definedImageIndex, getIndexedPath(definedImageIndex));
	if ( findInChainedTries(context, symbolName, definedImageIndex, requestorImage, runResolver, address) )
		return true;

	bool alreadyVisited[_header->imagesCount];
	bzero(alreadyVisited, sizeof(alreadyVisited));
	return findInChainedTriesAndDependentsExcept(context, symbolName, definedImageIndex, requestorImage, runResolver, alreadyVisited, address);
}


bool ImageLoaderMegaDylib::flatFindSymbol(const char* name, bool onlyInCoalesced, const ImageLoader::Symbol** sym, const ImageLoader** image, ImageLoader::CoalesceNotifier notifier)
{
	bool found = false;
	// check export trie of all in-use images
	for (unsigned i=0; i < _imageCount ; ++i) {
		uint16_t imageIndex = _bottomUpArray[i];
		if ( _stateFlags[imageIndex] == kStateUnused )
			continue;
		const macho_header* mh = getIndexedMachHeader(imageIndex);
		if ( onlyInCoalesced && (mh->flags & MH_WEAK_DEFINES) == 0 )
			continue;
		const uint8_t* exportNode;
		const uint8_t* exportTrieEnd;
		if ( exportTrieHasNode(name, imageIndex, &exportNode, &exportTrieEnd) ) {
			if ( notifier )
				notifier((Symbol*)exportNode, this, (mach_header*)getIndexedMachHeader(imageIndex));
			if ( !found ) {
				*sym = (Symbol*)exportNode;
				*image = this;
				found = true;
			}
			if ( !onlyInCoalesced )
				return true;
		}
	}
	return found;
}


void ImageLoaderMegaDylib::markAllbound(const LinkContext& context)
{
	for (unsigned i=0; i < _imageCount; ++i) {
		uint16_t imageIndex = _bottomUpArray[i];
		if ( _stateFlags[imageIndex] == kStateLoaded ) {
			_stateFlags[imageIndex] = kStateFlagBound;
			context.notifySingleFromCache(dyld_image_state_bound, (mach_header*)getIndexedMachHeader(imageIndex), getIndexedPath(imageIndex));
		}
	}
}


void ImageLoaderMegaDylib::recursiveSpinLockAcquire(unsigned int imageIndex, mach_port_t thisThread)
{
	pthread_mutex_lock(&_lockArrayGuard);
	if ( _lockArray == NULL )
		_lockArray = (recursive_lock*)calloc(_imageCount, sizeof(recursive_lock));
	_lockArrayInUseCount++;
	pthread_mutex_unlock(&_lockArrayGuard);

	recursive_lock* imagesLock = &_lockArray[imageIndex];
	while ( !OSAtomicCompareAndSwap32Barrier(0, thisThread, (int*)&imagesLock->thread) ) {
		if ( imagesLock->thread == thisThread )
			break;
	}
	imagesLock->count++;
}

void ImageLoaderMegaDylib::recursiveSpinLockRelease(unsigned int imageIndex, mach_port_t thisThread)
{
	recursive_lock* imagesLock = &_lockArray[imageIndex];
	if ( --imagesLock->count == 0 )
		imagesLock->thread = 0;

	pthread_mutex_lock(&_lockArrayGuard);
	_lockArrayInUseCount--;
	if ( _lockArrayInUseCount == 0 ) {
		free((void*)_lockArray);
		_lockArray = NULL;
	}
	pthread_mutex_unlock(&_lockArrayGuard);
}


void ImageLoaderMegaDylib::recursiveInitialization(const LinkContext& context, mach_port_t thisThread, unsigned int imageIndex,
													InitializerTimingList& timingInfo, UpwardIndexes& upInits)
{
	// Don't do any locking until libSystem.dylib is initialized, so we can malloc() the lock array
	bool useLock = dyld::gProcessInfo->libSystemInitialized;
	if ( useLock )
		recursiveSpinLockAcquire(imageIndex, thisThread);

	// only run initializers if currently in bound state
	if ( (_stateFlags[imageIndex] == kStateFlagBound) || (_stateFlags[imageIndex] == kStateFlagWeakBound) ) {

		// Each image in cache has its own lock.  We only set the state to Initialized if we hold the lock for the image.
		_stateFlags[imageIndex] = kStateFlagInitialized;

		// first recursively init all dependents
		unsigned startArrayIndex = _imageExtras[imageIndex].dependentsStartArrayIndex;
		for (int i=startArrayIndex; _dependenciesArray[i] != 0xFFFF; ++i) {
			unsigned subDepIndex = _dependenciesArray[i];
			// ignore upward links
			if ( (subDepIndex & 0x8000) == 0 )
				recursiveInitialization(context, thisThread, subDepIndex, timingInfo, upInits);
            else
                upInits.images[upInits.count++] = (subDepIndex & 0x7FFF);
		}

		// notify objc about this image
		context.notifySingleFromCache(dyld_image_state_dependents_initialized, (mach_header*)getIndexedMachHeader(imageIndex), getIndexedPath(imageIndex));

		// run all initializers for imageIndex
		const dyld_cache_accelerator_initializer* pInitStart = _initializers;
		const dyld_cache_accelerator_initializer* pInitEnd = &pInitStart[_initializerCount];
		bool ranSomeInitializers = false;
		uint64_t t1 = mach_absolute_time();
		for (const dyld_cache_accelerator_initializer* p=pInitStart; p < pInitEnd; ++p) {
			if ( p->imageIndex == imageIndex ) {
				Initializer func = (Initializer)(p->functionOffset + (uintptr_t)_header);
				if ( context.verboseInit )
					dyld::log("dyld: calling initializer function %p in %s\n", func, getIndexedPath(imageIndex));
				bool haveLibSystemHelpersBefore = (dyld::gLibSystemHelpers != NULL);
				{
					dyld3::ScopedTimer timer(DBG_DYLD_TIMING_STATIC_INITIALIZER, (uint64_t)getIndexedMachHeader(imageIndex), (uint64_t)func, 0);
					func(context.argc, context.argv, context.envp, context.apple, &context.programVars);
				};
				bool haveLibSystemHelpersAfter = (dyld::gLibSystemHelpers != NULL);
				ranSomeInitializers = true;
				if ( !haveLibSystemHelpersBefore && haveLibSystemHelpersAfter ) {
					// now safe to use malloc() and other calls in libSystem.dylib
					dyld::gProcessInfo->libSystemInitialized = true;
				}
			}
		}
		if ( ranSomeInitializers ) {
			uint64_t t2 = mach_absolute_time();
			const char* shortName = strrchr(getIndexedPath(imageIndex), '/');
			if ( shortName == NULL )
				shortName = getIndexedPath(imageIndex);
			else
				++shortName;
			timingInfo.images[timingInfo.count].shortName = shortName;
			timingInfo.images[timingInfo.count].initTime = (t2-t1);
			timingInfo.count++;
		}
	}

	// only unlock if this frame locked (note: libSystemInitialized changes after libSystem's initializer is run)
	if ( useLock )
		recursiveSpinLockRelease(imageIndex, thisThread);
}


void ImageLoaderMegaDylib::recursiveInitialization(const LinkContext& context, mach_port_t thisThread, const char* pathToInitialize,
													InitializerTimingList& timingInfo, UninitedUpwards&)
{
	UpwardIndexes upsBuffer[256];
	UpwardIndexes& ups = upsBuffer[0];
	ups.count = 0;
	unsigned imageIndex = findImageIndex(context, pathToInitialize);
	this->recursiveInitialization(context, thisThread, imageIndex, timingInfo, ups);
	for (int i=0; i < ups.count; ++i) {
		UpwardIndexes upsBuffer2[256];
		UpwardIndexes& ignoreUp = upsBuffer2[0];
		ignoreUp.count = 0;
		this->recursiveInitialization(context, thisThread, ups.images[i], timingInfo, ignoreUp);
	}
}

void ImageLoaderMegaDylib::recursiveBind(const LinkContext& context, bool forceLazysBound, bool neverUnload)
{
	markAllbound(context);
}

uint8_t ImageLoaderMegaDylib::dyldStateToCacheState(dyld_image_states state) {
	switch (state) {
		case dyld_image_state_mapped:
		case dyld_image_state_dependents_mapped:
			return kStateLoaded;
		case dyld_image_state_bound:
			return kStateFlagBound;
		case dyld_image_state_initialized:
			return kStateFlagInitialized;
		case dyld_image_state_rebased:
		case dyld_image_state_dependents_initialized:
		case dyld_image_state_terminated:
			return kStateUnused;
	}
	return kStateUnused;
}

void ImageLoaderMegaDylib::recursiveApplyInterposing(const LinkContext& context)
{
	if ( context.verboseInterposing )
		dyld::log("dyld: interposing %lu tuples onto shared cache\n", fgInterposingTuples.size());


}

unsigned ImageLoaderMegaDylib::appendImagesToNotify(dyld_image_states state, bool orLater, dyld_image_info* infos)
{
	uint8_t targetCacheState = dyldStateToCacheState(state);
	if ( targetCacheState == kStateUnused )
		return 0;
	unsigned usedCount = 0;
	for (int i=_imageCount-1; i >= 0; --i) {
		uint16_t index = _bottomUpArray[i];
		uint8_t imageState = _stateFlags[index];
		if ( imageState == kStateFlagWeakBound )
			imageState = kStateFlagBound;
		if ( (imageState == targetCacheState) || (orLater && (imageState > targetCacheState)) ) {
			infos[usedCount].imageLoadAddress	= (mach_header*)getIndexedMachHeader(index);
			infos[usedCount].imageFilePath		= getIndexedPath(index);
			infos[usedCount].imageFileModDate	= 0;
			++usedCount;
		}
	}
	return usedCount;
}


bool ImageLoaderMegaDylib::dlopenFromCache(const LinkContext& context, const char* path, int mode, void** handle)
{
	unsigned imageIndex;
	if ( !hasDylib(path, &imageIndex) ) {
		return false;
	}

	// RTLD_NOLOAD means return handle if already loaded, but don't now load it
	if ( mode & RTLD_NOLOAD ) {
		dyld::gLibSystemHelpers->releaseGlobalDyldLock();
		if ( _stateFlags[imageIndex] == kStateUnused ) {
			*handle = NULL;
			return true;
		}
	}
	else {
		this->recursiveMarkLoaded(context, imageIndex);
		context.notifyBatch(dyld_image_state_dependents_mapped, false);
		this->markAllbound(context);
		context.notifyBatch(dyld_image_state_bound, false);

		this->weakBind(context);

		// <rdar://problem/25069046> Release dyld global lock before running initializers in dlopen() with customer cache
		dyld::gLibSystemHelpers->releaseGlobalDyldLock();

		InitializerTimingList timingInfo[_initializerCount];
		timingInfo[0].count = 0;
		mach_port_t thisThread  = mach_thread_self();
		UpwardIndexes upsBuffer[256];  // room for 511 dangling upward links
		UpwardIndexes& ups = upsBuffer[0];
		ups.count = 0;
		this->recursiveInitialization(context, thisThread, imageIndex, timingInfo[0], ups);
		// make sure any upward linked dylibs were initialized
		for (int i=0; i < ups.count; ++i) {
			UpwardIndexes upsBuffer2[256];
			UpwardIndexes& ignoreUp = upsBuffer2[0];
			ignoreUp.count = 0;
			this->recursiveInitialization(context, thisThread, ups.images[i], timingInfo[0], ignoreUp);
		}
		mach_port_deallocate(mach_task_self(), thisThread);
		context.notifyBatch(dyld_image_state_initialized, false);
	}

	*handle = makeCacheHandle(imageIndex, mode);
	return true;
}

bool ImageLoaderMegaDylib::makeCacheHandle(const LinkContext& context, unsigned cacheIndex, int mode, void** result)
{
	if ( cacheIndex >= _imageCount )
		return false;

	*result = makeCacheHandle(cacheIndex, mode);
	return true;
}

void* ImageLoaderMegaDylib::makeCacheHandle(unsigned index, int mode)
{
	uint8_t flags = ((mode & RTLD_FIRST) ? 1 : 0);

#if __LP64__
	return (void*)(uintptr_t)( 0xFFEEDDCC00000000LL | (index << 8) | flags);
#else
	return (void*)(uintptr_t)( 0xF8000000 | (index << 8) | flags);
#endif
}

bool ImageLoaderMegaDylib::isCacheHandle(void* handle, unsigned* index, uint8_t* flags)
{
#if __LP64__
	if ( (((uintptr_t)handle) >> 32) == 0xFFEEDDCC ) {
		if ( index )
			*index = (((uintptr_t)handle) >> 8) & 0xFFFF;
		if ( flags )
			*flags = ((uintptr_t)handle) & 0xFF;
		return true;
	}
#else
	if ( (((uintptr_t)handle) >> 24) == 0xF8 ) {
		if ( index )
			*index = (((uintptr_t)handle) >> 8) & 0xFFFF;
		if ( flags )
			*flags = ((uintptr_t)handle) & 0xFF;
		return true;
	}
#endif
	return false;
}


void* ImageLoaderMegaDylib::dlsymFromCache(const LinkContext& context, void* handle, const char* symbolName, unsigned imageIndex)
{
	unsigned indexInHandle;
	uint8_t flags;
	uintptr_t symAddress;
	if ( handle == RTLD_SELF ) {
		if ( findInChainedTriesAndDependents(context, symbolName, imageIndex, NULL, true, &symAddress) )
			return (void*)symAddress;
	}
	else if ( handle == RTLD_NEXT ) {
		// FIXME: really need to not look in imageIndex, but look in others.
		if ( findInChainedTriesAndDependents(context, symbolName, imageIndex, NULL, true, &symAddress) )
			return (void*)symAddress;
	}
	else if ( isCacheHandle(handle, &indexInHandle, &flags) ) {
		bool searchOnlyFirst = (flags & 1);  // RTLD_FIRST
		// normal dlsym(handle,) semantics is that the handle is just the first place to search. RTLD_FIRST disables that
		if ( searchOnlyFirst ) {
			if ( findInChainedTries(context, symbolName, indexInHandle, NULL, true, &symAddress) )
				return (void*)symAddress;
		}
		else {
			if ( findInChainedTriesAndDependents(context, symbolName, indexInHandle, NULL, true, &symAddress) )
				return (void*)symAddress;
		}
	}

	return NULL;
}

bool ImageLoaderMegaDylib::dladdrFromCache(const void* address, Dl_info* info)
{
	const mach_header* mh;
	unsigned index;
	if ( !addressInCache(address, &mh, &info->dli_fname, &index) )
		return false;

	info->dli_fbase = (void*)mh;
	if ( address == mh ) {
		// special case lookup of header
		info->dli_sname = "__dso_handle";
		info->dli_saddr = info->dli_fbase;
		return true;
	}

	// find closest symbol in the image
	info->dli_sname = ImageLoaderMachO::findClosestSymbol(mh, address, (const void**)&info->dli_saddr);

	// never return the mach_header symbol
	if ( info->dli_saddr == info->dli_fbase ) {
		info->dli_sname = NULL;
		info->dli_saddr = NULL;
		return true;
	}

	// strip off leading underscore
	if ( info->dli_sname != NULL ) {
		if ( info->dli_sname[0] == '_' )
			info->dli_sname = info->dli_sname +1;
	}
	return true;
}


uintptr_t ImageLoaderMegaDylib::bindLazy(uintptr_t lazyBindingInfoOffset, const LinkContext& context, const mach_header* mh, unsigned imageIndex)
{
	const dyld_info_command* dyldInfoCmd = ImageLoaderMachO::findDyldInfoLoadCommand(mh);
	if ( dyldInfoCmd == NULL )
		return 0;

	const uint8_t* const lazyInfoStart = &_linkEditBias[dyldInfoCmd->lazy_bind_off];
	const uint8_t* const lazyInfoEnd = &lazyInfoStart[dyldInfoCmd->lazy_bind_size];
	uint32_t lbOffset = (uint32_t)lazyBindingInfoOffset;
	uint8_t segIndex;
	uintptr_t segOffset;
	int libraryOrdinal;
	const char* symbolName;
	bool doneAfterBind;
	if ( ImageLoaderMachO::getLazyBindingInfo(lbOffset, lazyInfoStart, lazyInfoEnd, &segIndex, &segOffset, &libraryOrdinal, &symbolName, &doneAfterBind) ) {
		//const char* thisPath = getIndexedPath(imageIndex);
		//dyld::log("%s needs symbol '%s' from ordinal=%d\n", thisPath, symbolName, libraryOrdinal);
		unsigned startDepArrayIndex = _imageExtras[imageIndex].dependentsStartArrayIndex;
		unsigned targetIndex;
		if ( libraryOrdinal == BIND_SPECIAL_DYLIB_SELF )
			targetIndex = imageIndex;
		else
			targetIndex = _dependenciesArray[startDepArrayIndex+libraryOrdinal-1] & 0x7FFF;
		//const char* targetPath = getIndexedPath(targetIndex);
		//dyld::log("%s needs symbol '%s' from %s\n", thisPath, symbolName, targetPath);
		uintptr_t targetAddress;
		if ( findInChainedTries(context, symbolName, targetIndex, this, true, &targetAddress) ) {
			if ( uintptr_t segPrefAddress = ImageLoaderMachO::segPreferredAddress(mh, segIndex) ) {
				uintptr_t* lp = (uintptr_t*)(segPrefAddress + segOffset + _slide);
				//dyld::log("    storing 0x%0lX to lp %p\n", targetAddress, lp);
				*lp = targetAddress;
				return targetAddress;
			}
		}
	}

	return 0;
}


#endif // SUPPORT_ACCELERATE_TABLES




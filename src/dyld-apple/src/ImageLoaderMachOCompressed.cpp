/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h> 
#include <sys/param.h>
#include <mach/mach.h>
#include <mach/thread_status.h>
#include <mach-o/loader.h> 
#include "ImageLoaderMachOCompressed.h"
#include "mach-o/dyld_images.h"

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


// create image for main executable
ImageLoaderMachOCompressed* ImageLoaderMachOCompressed::instantiateMainExecutable(const macho_header* mh, uintptr_t slide, const char* path, 
																		unsigned int segCount, unsigned int libCount, const LinkContext& context)
{
	ImageLoaderMachOCompressed* image = ImageLoaderMachOCompressed::instantiateStart(mh, path, segCount, libCount);

	// set slide for PIE programs
	image->setSlide(slide);

	// for PIE record end of program, to know where to start loading dylibs
	if ( slide != 0 )
		fgNextPIEDylibAddress = (uintptr_t)image->getEnd();

	image->disableCoverageCheck();
	image->instantiateFinish(context);
	image->setMapped(context);

	if ( context.verboseMapping ) {
		dyld::log("dyld: Main executable mapped %s\n", path);
		for(unsigned int i=0, e=image->segmentCount(); i < e; ++i) {
			const char* name = image->segName(i);
			if ( (strcmp(name, "__PAGEZERO") == 0) || (strcmp(name, "__UNIXSTACK") == 0)  )
				dyld::log("%18s at 0x%08lX->0x%08lX\n", name, image->segPreferredLoadAddress(i), image->segPreferredLoadAddress(i)+image->segSize(i));
			else
				dyld::log("%18s at 0x%08lX->0x%08lX\n", name, image->segActualLoadAddress(i), image->segActualEndAddress(i));
		}
	}

	return image;
}

// create image by mapping in a mach-o file
ImageLoaderMachOCompressed* ImageLoaderMachOCompressed::instantiateFromFile(const char* path, int fd, const uint8_t* fileData, size_t lenFileData,
															uint64_t offsetInFat, uint64_t lenInFat, const struct stat& info, 
															unsigned int segCount, unsigned int libCount, 
															const struct linkedit_data_command* codeSigCmd, 
															const struct encryption_info_command* encryptCmd, 
															const LinkContext& context)
{
	ImageLoaderMachOCompressed* image = ImageLoaderMachOCompressed::instantiateStart((macho_header*)fileData, path, segCount, libCount);

	try {
		// record info about file  
		image->setFileInfo(info.st_dev, info.st_ino, info.st_mtime);

		// if this image is code signed, let kernel validate signature before mapping any pages from image
		image->loadCodeSignature(codeSigCmd, fd, offsetInFat, context);
		
		// Validate that first data we read with pread actually matches with code signature
		image->validateFirstPages(codeSigCmd, fd, fileData, lenFileData, offsetInFat, context);

		// mmap segments
		image->mapSegments(fd, offsetInFat, lenInFat, info.st_size, context);

		// if framework is FairPlay encrypted, register with kernel
		image->registerEncryption(encryptCmd, context);
		
		// probe to see if code signed correctly
		image->crashIfInvalidCodeSignature();

		// finish construction
		image->instantiateFinish(context);
		
		// if path happens to be same as in LC_DYLIB_ID load command use that, otherwise malloc a copy of the path
		const char* installName = image->getInstallPath();
		if ( (installName != NULL) && (strcmp(installName, path) == 0) && (path[0] == '/') )
			image->setPathUnowned(installName);
#if __MAC_OS_X_VERSION_MIN_REQUIRED
		// <rdar://problem/6563887> app crashes when libSystem cannot be found
		else if ( (installName != NULL) && (strcmp(path, "/usr/lib/libgcc_s.1.dylib") == 0) && (strcmp(installName, "/usr/lib/libSystem.B.dylib") == 0) )
			image->setPathUnowned("/usr/lib/libSystem.B.dylib");
#endif
		else if ( (path[0] != '/') || (strstr(path, "../") != NULL) ) {
			// rdar://problem/10733082 Fix up @rpath based paths during introspection
			// rdar://problem/5135363 turn relative paths into absolute paths so gdb, Symbolication can later find them
			char realPath[MAXPATHLEN];
			if ( fcntl(fd, F_GETPATH, realPath) == 0 ) 
				image->setPaths(path, realPath);
			else
				image->setPath(path);
		}
		else 
			image->setPath(path);

		// make sure path is stable before recording in dyld_all_image_infos
		image->setMapped(context);

		// pre-fetch content of __DATA and __LINKEDIT segment for faster launches
		// don't do this on prebound images or if prefetching is disabled
        if ( !context.preFetchDisabled && !image->isPrebindable()) {
			image->preFetchDATA(fd, offsetInFat, context);
			image->markSequentialLINKEDIT(context);
		}
	}
	catch (...) {
		// ImageLoader::setMapped() can throw an exception to block loading of image
		// <rdar://problem/6169686> Leaked fSegmentsArray and image segments during failed dlopen_preflight
		delete image;
		throw;
	}
	
	return image;
}

// create image by using cached mach-o file
ImageLoaderMachOCompressed* ImageLoaderMachOCompressed::instantiateFromCache(const macho_header* mh, const char* path, long slide,
																		const struct stat& info, unsigned int segCount,
																		unsigned int libCount, const LinkContext& context)
{
	ImageLoaderMachOCompressed* image = ImageLoaderMachOCompressed::instantiateStart(mh, path, segCount, libCount);
	try {
		// record info about file  
		image->setFileInfo(info.st_dev, info.st_ino, info.st_mtime);

		// remember this is from shared cache and cannot be unloaded
		image->fInSharedCache = true;
		image->setNeverUnload();
		image->setSlide(slide);
		image->disableCoverageCheck();

		// segments already mapped in cache
		if ( context.verboseMapping ) {
			dyld::log("dyld: Using shared cached for %s\n", path);
			for(unsigned int i=0; i < image->fSegmentsCount; ++i) {
				dyld::log("%18s at 0x%08lX->0x%08lX\n", image->segName(i), image->segActualLoadAddress(i), image->segActualEndAddress(i));
			}
		}

		image->instantiateFinish(context);
		image->setMapped(context);
	}
	catch (...) {
		// ImageLoader::setMapped() can throw an exception to block loading of image
		// <rdar://problem/6169686> Leaked fSegmentsArray and image segments during failed dlopen_preflight
		delete image;
		throw;
	}
	
	return image;
}

// create image by copying an in-memory mach-o file
ImageLoaderMachOCompressed* ImageLoaderMachOCompressed::instantiateFromMemory(const char* moduleName, const macho_header* mh, uint64_t len, 
															unsigned int segCount, unsigned int libCount, const LinkContext& context)
{
	ImageLoaderMachOCompressed* image = ImageLoaderMachOCompressed::instantiateStart(mh, moduleName, segCount, libCount);
	try {
		// map segments 
		if ( mh->filetype == MH_EXECUTE ) 
			throw "can't load another MH_EXECUTE";
		
		// vmcopy segments
		image->mapSegments((const void*)mh, len, context);
		
		// for compatibility, never unload dylibs loaded from memory
		image->setNeverUnload();

		image->disableCoverageCheck();

		// bundle loads need path copied
		if ( moduleName != NULL ) 
			image->setPath(moduleName);

		image->instantiateFinish(context);
		image->setMapped(context);
	}
	catch (...) {
		// ImageLoader::setMapped() can throw an exception to block loading of image
		// <rdar://problem/6169686> Leaked fSegmentsArray and image segments during failed dlopen_preflight
		delete image;
		throw;
	}
	
	return image;
}


ImageLoaderMachOCompressed::ImageLoaderMachOCompressed(const macho_header* mh, const char* path, unsigned int segCount, 
																		uint32_t segOffsets[], unsigned int libCount)
 : ImageLoaderMachO(mh, path, segCount, segOffsets, libCount), fDyldInfo(NULL)
{
}

ImageLoaderMachOCompressed::~ImageLoaderMachOCompressed()
{
	// don't do clean up in ~ImageLoaderMachO() because virtual call to segmentCommandOffsets() won't work
	destroy();
}



// construct ImageLoaderMachOCompressed using "placement new" with SegmentMachO objects array at end
ImageLoaderMachOCompressed* ImageLoaderMachOCompressed::instantiateStart(const macho_header* mh, const char* path, 
																			unsigned int segCount, unsigned int libCount)
{
	size_t size = sizeof(ImageLoaderMachOCompressed) + segCount * sizeof(uint32_t) + libCount * sizeof(ImageLoader*);
	ImageLoaderMachOCompressed* allocatedSpace = static_cast<ImageLoaderMachOCompressed*>(malloc(size));
	if ( allocatedSpace == NULL )
		throw "malloc failed";
	uint32_t* segOffsets = ((uint32_t*)(((uint8_t*)allocatedSpace) + sizeof(ImageLoaderMachOCompressed)));
	bzero(&segOffsets[segCount], libCount*sizeof(void*));	// zero out lib array
	return new (allocatedSpace) ImageLoaderMachOCompressed(mh, path, segCount, segOffsets, libCount);
}


// common code to finish initializing object
void ImageLoaderMachOCompressed::instantiateFinish(const LinkContext& context)
{
	// now that segments are mapped in, get real fMachOData, fLinkEditBase, and fSlide
	this->parseLoadCmds(context);
}

uint32_t* ImageLoaderMachOCompressed::segmentCommandOffsets() const
{
	return ((uint32_t*)(((uint8_t*)this) + sizeof(ImageLoaderMachOCompressed)));
}


ImageLoader* ImageLoaderMachOCompressed::libImage(unsigned int libIndex) const
{
	const uintptr_t* images = ((uintptr_t*)(((uint8_t*)this) + sizeof(ImageLoaderMachOCompressed) + fSegmentsCount*sizeof(uint32_t)));
	// mask off low bits
	return (ImageLoader*)(images[libIndex] & (-4));
}

bool ImageLoaderMachOCompressed::libReExported(unsigned int libIndex) const
{
	const uintptr_t* images = ((uintptr_t*)(((uint8_t*)this) + sizeof(ImageLoaderMachOCompressed) + fSegmentsCount*sizeof(uint32_t)));
	// re-export flag is low bit
	return ((images[libIndex] & 1) != 0);
}	

bool ImageLoaderMachOCompressed::libIsUpward(unsigned int libIndex) const
{
	const uintptr_t* images = ((uintptr_t*)(((uint8_t*)this) + sizeof(ImageLoaderMachOCompressed) + fSegmentsCount*sizeof(uint32_t)));
	// re-export flag is second bit
	return ((images[libIndex] & 2) != 0);
}	


void ImageLoaderMachOCompressed::setLibImage(unsigned int libIndex, ImageLoader* image, bool reExported, bool upward)
{
	uintptr_t* images = ((uintptr_t*)(((uint8_t*)this) + sizeof(ImageLoaderMachOCompressed) + fSegmentsCount*sizeof(uint32_t)));
	uintptr_t value = (uintptr_t)image;
	if ( reExported ) 
		value |= 1;
	if ( upward ) 
		value |= 2;
	images[libIndex] = value;
}


void ImageLoaderMachOCompressed::markFreeLINKEDIT(const LinkContext& context)
{
	// mark that we are done with rebase and bind info 
	markLINKEDIT(context, MADV_FREE);
}

void ImageLoaderMachOCompressed::markSequentialLINKEDIT(const LinkContext& context)
{
	// mark the rebase and bind info and using sequential access
	markLINKEDIT(context, MADV_SEQUENTIAL);
}

void ImageLoaderMachOCompressed::markLINKEDIT(const LinkContext& context, int advise)
{
	// if not loaded at preferred address, mark rebase info 
	uintptr_t start = 0;
	if ( (fSlide != 0) && (fDyldInfo->rebase_size != 0) ) 
		start = (uintptr_t)fLinkEditBase + fDyldInfo->rebase_off;
	else if ( fDyldInfo->bind_off != 0 )
		start = (uintptr_t)fLinkEditBase + fDyldInfo->bind_off;
	else
		return; // no binding info to prefetch
		
	// end is at end of bind info
	uintptr_t end = 0;
	if ( fDyldInfo->bind_off != 0 )
		end = (uintptr_t)fLinkEditBase + fDyldInfo->bind_off + fDyldInfo->bind_size;
	else if ( fDyldInfo->rebase_off != 0 )
		end = (uintptr_t)fLinkEditBase + fDyldInfo->rebase_off + fDyldInfo->rebase_size;
	else
		return;
		
			
	// round to whole pages
	start = dyld_page_trunc(start);
	end = dyld_page_round(end);

	// do nothing if only one page of rebase/bind info
	if ( (end-start) <= dyld_page_size )
		return;
	
	// tell kernel about our access to these pages
	madvise((void*)start, end-start, advise);
	if ( context.verboseMapping ) {
		const char* adstr = "sequential";
		if ( advise == MADV_FREE )
			adstr = "free";
		dyld::log("%18s %s 0x%0lX -> 0x%0lX for %s\n", "__LINKEDIT", adstr, start, end-1, this->getPath());
	}
}



void ImageLoaderMachOCompressed::rebaseAt(const LinkContext& context, uintptr_t addr, uintptr_t slide, uint8_t type)
{
	if ( context.verboseRebase ) {
		dyld::log("dyld: rebase: %s:*0x%08lX += 0x%08lX\n", this->getShortName(), (uintptr_t)addr, slide);
	}
	//dyld::log("0x%08lX type=%d\n", addr, type);
	uintptr_t* locationToFix = (uintptr_t*)addr;
	switch (type) {
		case REBASE_TYPE_POINTER:
			*locationToFix += slide;
			break;
		case REBASE_TYPE_TEXT_ABSOLUTE32:
			*locationToFix += slide;
			break;
		default:
			dyld::throwf("bad rebase type %d", type);
	}
}

void ImageLoaderMachOCompressed::throwBadRebaseAddress(uintptr_t address, uintptr_t segmentEndAddress, int segmentIndex, 
										const uint8_t* startOpcodes, const uint8_t* endOpcodes, const uint8_t* pos)
{
	dyld::throwf("malformed rebase opcodes (%ld/%ld): address 0x%08lX is outside of segment %s (0x%08lX -> 0x%08lX)",
		(intptr_t)(pos-startOpcodes), (intptr_t)(endOpcodes-startOpcodes), address, segName(segmentIndex), 
		segActualLoadAddress(segmentIndex), segmentEndAddress); 
}

void ImageLoaderMachOCompressed::rebase(const LinkContext& context, uintptr_t slide)
{
	CRSetCrashLogMessage2(this->getPath());
	const uint8_t* const start = fLinkEditBase + fDyldInfo->rebase_off;
	const uint8_t* const end = &start[fDyldInfo->rebase_size];
	const uint8_t* p = start;

	try {
		uint8_t type = 0;
		int segmentIndex = 0;
		uintptr_t address = segActualLoadAddress(0);
		uintptr_t segmentStartAddress = segActualLoadAddress(0);
		uintptr_t segmentEndAddress = segActualEndAddress(0);
		uintptr_t count;
		uintptr_t skip;
		bool done = false;
		while ( !done && (p < end) ) {
			uint8_t immediate = *p & REBASE_IMMEDIATE_MASK;
			uint8_t opcode = *p & REBASE_OPCODE_MASK;
			++p;
			switch (opcode) {
				case REBASE_OPCODE_DONE:
					done = true;
					break;
				case REBASE_OPCODE_SET_TYPE_IMM:
					type = immediate;
					break;
				case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
					segmentIndex = immediate;
					if ( segmentIndex >= fSegmentsCount )
						dyld::throwf("REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has segment %d which is too large (0..%d)",
								segmentIndex, fSegmentsCount-1);
			#if TEXT_RELOC_SUPPORT
					if ( !segWriteable(segmentIndex) && !segHasRebaseFixUps(segmentIndex) && !segHasBindFixUps(segmentIndex) )
			#else
					if ( !segWriteable(segmentIndex) )
			#endif
						dyld::throwf("REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has segment %d which is not a writable segment (%s)",
								segmentIndex, segName(segmentIndex));
					segmentStartAddress = segActualLoadAddress(segmentIndex);
					segmentEndAddress = segActualEndAddress(segmentIndex);
					address = segmentStartAddress + read_uleb128(p, end);
					break;
				case REBASE_OPCODE_ADD_ADDR_ULEB:
					address += read_uleb128(p, end);
					break;
				case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
					address += immediate*sizeof(uintptr_t);
					break;
				case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
					for (int i=0; i < immediate; ++i) {
						if ( (address < segmentStartAddress) || (address >= segmentEndAddress) )
							throwBadRebaseAddress(address, segmentEndAddress, segmentIndex, start, end, p);
						rebaseAt(context, address, slide, type);
						address += sizeof(uintptr_t);
					}
					fgTotalRebaseFixups += immediate;
					break;
				case REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
					count = read_uleb128(p, end);
					for (uint32_t i=0; i < count; ++i) {
						if ( (address < segmentStartAddress) || (address >= segmentEndAddress) )
							throwBadRebaseAddress(address, segmentEndAddress, segmentIndex, start, end, p);
						rebaseAt(context, address, slide, type);
						address += sizeof(uintptr_t);
					}
					fgTotalRebaseFixups += count;
					break;
				case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
					if ( (address < segmentStartAddress) || (address >= segmentEndAddress) )
						throwBadRebaseAddress(address, segmentEndAddress, segmentIndex, start, end, p);
					rebaseAt(context, address, slide, type);
					address += read_uleb128(p, end) + sizeof(uintptr_t);
					++fgTotalRebaseFixups;
					break;
				case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
					count = read_uleb128(p, end);
					skip = read_uleb128(p, end);
					for (uint32_t i=0; i < count; ++i) {
						if ( (address < segmentStartAddress) || (address >= segmentEndAddress) )
							throwBadRebaseAddress(address, segmentEndAddress, segmentIndex, start, end, p);
						rebaseAt(context, address, slide, type);
						address += skip + sizeof(uintptr_t);
					}
					fgTotalRebaseFixups += count;
					break;
				default:
					dyld::throwf("bad rebase opcode %d", *p);
			}
		}
	}
	catch (const char* msg) {
		const char* newMsg = dyld::mkstringf("%s in %s", msg, this->getPath());
		free((void*)msg);
		throw newMsg;
	}
	CRSetCrashLogMessage2(NULL);
}

const ImageLoader::Symbol* ImageLoaderMachOCompressed::findShallowExportedSymbol(const char* symbol, const ImageLoader** foundIn) const
{
	//dyld::log("Compressed::findExportedSymbol(%s) in %s\n", symbol, this->getShortName());
	if ( fDyldInfo->export_size == 0 )
		return NULL;
#if LOG_BINDINGS
	dyld::logBindings("%s: %s\n", this->getShortName(), symbol);
#endif
	++ImageLoaderMachO::fgSymbolTrieSearchs;
	const uint8_t* start = &fLinkEditBase[fDyldInfo->export_off];
	const uint8_t* end = &start[fDyldInfo->export_size];
	const uint8_t* foundNodeStart = this->trieWalk(start, end, symbol); 
	if ( foundNodeStart != NULL ) {
		const uint8_t* p = foundNodeStart;
		const uintptr_t flags = read_uleb128(p, end);
		// found match, return pointer to terminal part of node
		if ( flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
			// re-export from another dylib, lookup there
			const uintptr_t ordinal = read_uleb128(p, end);
			const char* importedName = (char*)p;
			if ( importedName[0] == '\0' )
				importedName = symbol;
			if ( (ordinal > 0) && (ordinal <= libraryCount()) ) {
				const ImageLoader* reexportedFrom = libImage((unsigned int)ordinal-1);
				//dyld::log("Compressed::findExportedSymbol(), %s -> %s/%s\n", symbol, reexportedFrom->getShortName(), importedName);
				const char* reExportLibPath = libPath((unsigned int)ordinal-1);
				return reexportedFrom->findExportedSymbol(importedName, true, reExportLibPath, foundIn);
			}
			else {
				//dyld::throwf("bad mach-o binary, library ordinal (%u) invalid (max %u) for re-exported symbol %s in %s",
				//	ordinal, libraryCount(), symbol, this->getPath());
			}
		}
		else {
			//dyld::log("findExportedSymbol(%s) in %s found match, returning %p\n", symbol, this->getShortName(), p);
			if ( foundIn != NULL )
				*foundIn = (ImageLoader*)this;		
			// return pointer to terminal part of node
			return (Symbol*)foundNodeStart;
		}
	}
	return NULL;
}


bool ImageLoaderMachOCompressed::containsSymbol(const void* addr) const
{
	const uint8_t* start = &fLinkEditBase[fDyldInfo->export_off];
	const uint8_t* end = &start[fDyldInfo->export_size];
	return ( (start <= addr) && (addr < end) );
}


uintptr_t ImageLoaderMachOCompressed::exportedSymbolAddress(const LinkContext& context, const Symbol* symbol, const ImageLoader* requestor, bool runResolver) const
{
	const uint8_t* exportNode = (uint8_t*)symbol;
	const uint8_t* exportTrieStart = fLinkEditBase + fDyldInfo->export_off;
	const uint8_t* exportTrieEnd = exportTrieStart + fDyldInfo->export_size;
	if ( (exportNode < exportTrieStart) || (exportNode > exportTrieEnd) )
		throw "symbol is not in trie";
	//dyld::log("exportedSymbolAddress(): node=%p, nodeOffset=0x%04X in %s\n", symbol, (int)((uint8_t*)symbol - exportTrieStart), this->getShortName());
	uintptr_t flags = read_uleb128(exportNode, exportTrieEnd);
	switch ( flags & EXPORT_SYMBOL_FLAGS_KIND_MASK ) {
		case EXPORT_SYMBOL_FLAGS_KIND_REGULAR:
			if ( runResolver && (flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER) ) {
				// this node has a stub and resolver, run the resolver to get target address
				uintptr_t stub = read_uleb128(exportNode, exportTrieEnd) + (uintptr_t)fMachOData; // skip over stub
				// <rdar://problem/10657737> interposing dylibs have the stub address as their replacee
				uintptr_t interposedStub = interposedAddress(context, stub, requestor);
				if ( interposedStub != stub )
					return interposedStub;
				// stub was not interposed, so run resolver
				typedef uintptr_t (*ResolverProc)(void);
				ResolverProc resolver = (ResolverProc)(read_uleb128(exportNode, exportTrieEnd) + (uintptr_t)fMachOData);
				uintptr_t result = (*resolver)();
				if ( context.verboseBind )
					dyld::log("dyld: resolver at %p returned 0x%08lX\n", resolver, result);
				return result;
			}
			return read_uleb128(exportNode, exportTrieEnd) + (uintptr_t)fMachOData;
		case EXPORT_SYMBOL_FLAGS_KIND_THREAD_LOCAL:
			if ( flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER )
				dyld::throwf("unsupported exported symbol kind. flags=%lu at node=%p", flags, symbol);
			return read_uleb128(exportNode, exportTrieEnd) + (uintptr_t)fMachOData;
		case EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE:
			if ( flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER )
				dyld::throwf("unsupported exported symbol kind. flags=%lu at node=%p", flags, symbol);
			return read_uleb128(exportNode, exportTrieEnd);
		default:
			dyld::throwf("unsupported exported symbol kind. flags=%lu at node=%p", flags, symbol);
	}
}

bool ImageLoaderMachOCompressed::exportedSymbolIsWeakDefintion(const Symbol* symbol) const
{
	const uint8_t* exportNode = (uint8_t*)symbol;
	const uint8_t* exportTrieStart = fLinkEditBase + fDyldInfo->export_off;
	const uint8_t* exportTrieEnd = exportTrieStart + fDyldInfo->export_size;
	if ( (exportNode < exportTrieStart) || (exportNode > exportTrieEnd) )
		throw "symbol is not in trie";
	uintptr_t flags = read_uleb128(exportNode, exportTrieEnd);
	return ( flags & EXPORT_SYMBOL_FLAGS_WEAK_DEFINITION );
}


const char* ImageLoaderMachOCompressed::exportedSymbolName(const Symbol* symbol) const
{
	throw "NSNameOfSymbol() not supported with compressed LINKEDIT";
}

unsigned int ImageLoaderMachOCompressed::exportedSymbolCount() const
{
	throw "NSSymbolDefinitionCountInObjectFileImage() not supported with compressed LINKEDIT";
}

const ImageLoader::Symbol* ImageLoaderMachOCompressed::exportedSymbolIndexed(unsigned int index) const
{
	throw "NSSymbolDefinitionNameInObjectFileImage() not supported with compressed LINKEDIT";
}

unsigned int ImageLoaderMachOCompressed::importedSymbolCount() const
{
	throw "NSSymbolReferenceCountInObjectFileImage() not supported with compressed LINKEDIT";
}

const ImageLoader::Symbol* ImageLoaderMachOCompressed::importedSymbolIndexed(unsigned int index) const
{
	throw "NSSymbolReferenceCountInObjectFileImage() not supported with compressed LINKEDIT";
}

const char* ImageLoaderMachOCompressed::importedSymbolName(const Symbol* symbol) const
{
	throw "NSSymbolReferenceNameInObjectFileImage() not supported with compressed LINKEDIT";
}



uintptr_t ImageLoaderMachOCompressed::resolveFlat(const LinkContext& context, const char* symbolName, bool weak_import, 
													bool runResolver, const ImageLoader** foundIn)
{
	const Symbol* sym;
	if ( context.flatExportFinder(symbolName, &sym, foundIn) ) {
		if ( *foundIn != this )
			context.addDynamicReference(this, const_cast<ImageLoader*>(*foundIn));
		return (*foundIn)->getExportedSymbolAddress(sym, context, this, runResolver);
	}
	// if a bundle is loaded privately the above will not find its exports
	if ( this->isBundle() && this->hasHiddenExports() ) {
		// look in self for needed symbol
		sym = this->findShallowExportedSymbol(symbolName, foundIn);
		if ( sym != NULL )
			return (*foundIn)->getExportedSymbolAddress(sym, context, this, runResolver);
	}
	if ( weak_import ) {
		// definition can't be found anywhere, ok because it is weak, just return 0
		return 0;
	}
	throwSymbolNotFound(context, symbolName, this->getPath(), "", "flat namespace");
}


uintptr_t ImageLoaderMachOCompressed::resolveTwolevel(const LinkContext& context, const char* symbolName, const ImageLoader* definedInImage,
													  const ImageLoader* requestorImage, unsigned requestorOrdinalOfDef, bool weak_import, bool runResolver,
													  const ImageLoader** foundIn)
{
	// two level lookup
	uintptr_t address;
	if ( definedInImage->findExportedSymbolAddress(context, symbolName, requestorImage, requestorOrdinalOfDef, runResolver, foundIn, &address) )
		return address;

	if ( weak_import ) {
		// definition can't be found anywhere, ok because it is weak, just return 0
		return 0;
	}

	// nowhere to be found, check if maybe this image is too new for this OS
	char versMismatch[256];
	versMismatch[0] = '\0';
	uint32_t imageMinOS = this->minOSVersion();
	// dyld is always built for the current OS, so we can get the current OS version
	// from the load command in dyld itself.
	extern const mach_header __dso_handle;
	uint32_t dyldMinOS = ImageLoaderMachO::minOSVersion(&__dso_handle);
	if ( imageMinOS > dyldMinOS ) {
#if __MAC_OS_X_VERSION_MIN_REQUIRED
		const char* msg = dyld::mkstringf(" (which was built for Mac OS X %d.%d)", imageMinOS >> 16, (imageMinOS >> 8) & 0xFF);
#else
		const char* msg = dyld::mkstringf(" (which was built for iOS %d.%d)", imageMinOS >> 16, (imageMinOS >> 8) & 0xFF);
#endif
		strcpy(versMismatch, msg);
		::free((void*)msg);
	}
	throwSymbolNotFound(context, symbolName, this->getPath(), versMismatch, definedInImage->getPath());
}


uintptr_t ImageLoaderMachOCompressed::resolve(const LinkContext& context, const char* symbolName, 
													uint8_t symboFlags, long libraryOrdinal, const ImageLoader** targetImage,
													LastLookup* last, bool runResolver)
{
	*targetImage = NULL;
	
	// only clients that benefit from caching last lookup pass in a LastLookup struct
	if ( last != NULL ) {
		if ( (last->ordinal == libraryOrdinal)
			&& (last->flags == symboFlags)
			&& (last->name == symbolName) ) {
				*targetImage = last->foundIn;
				return last->result;
			}
	}
	
	bool weak_import = (symboFlags & BIND_SYMBOL_FLAGS_WEAK_IMPORT);
	uintptr_t symbolAddress;
	if ( context.bindFlat || (libraryOrdinal == BIND_SPECIAL_DYLIB_FLAT_LOOKUP) ) {
		symbolAddress = this->resolveFlat(context, symbolName, weak_import, runResolver, targetImage);
	}
	else {
		if ( libraryOrdinal == BIND_SPECIAL_DYLIB_MAIN_EXECUTABLE ) {
			*targetImage = context.mainExecutable;
		}
		else if ( libraryOrdinal == BIND_SPECIAL_DYLIB_SELF ) {
			*targetImage = this;
		}
		else if ( libraryOrdinal <= 0 ) {
			dyld::throwf("bad mach-o binary, unknown special library ordinal (%ld) too big for symbol %s in %s",
				libraryOrdinal, symbolName, this->getPath());
		}
		else if ( (unsigned)libraryOrdinal <= libraryCount() ) {
			*targetImage = libImage((unsigned int)libraryOrdinal-1);
		}
		else {
			dyld::throwf("bad mach-o binary, library ordinal (%ld) too big (max %u) for symbol %s in %s",
				libraryOrdinal, libraryCount(), symbolName, this->getPath());
		}
		if ( *targetImage == NULL ) {
			if ( weak_import ) {
				// if target library not loaded and reference is weak or library is weak return 0
				symbolAddress = 0;
			}
			else {
				dyld::throwf("can't resolve symbol %s in %s because dependent dylib #%ld could not be loaded",
					symbolName, this->getPath(), libraryOrdinal);
			}
		}
		else {
			symbolAddress = resolveTwolevel(context, symbolName, *targetImage, this, (unsigned)libraryOrdinal, weak_import, runResolver, targetImage);
		}
	}

	// save off lookup results if client wants 
	if ( last != NULL ) {
		last->ordinal	= libraryOrdinal;
		last->flags		= symboFlags;
		last->name		= symbolName;
		last->foundIn	= *targetImage;
		last->result	= symbolAddress;
	}
	
	return symbolAddress;
}

uintptr_t ImageLoaderMachOCompressed::bindAt(const LinkContext& context, uintptr_t addr, uint8_t type, const char* symbolName, 
								uint8_t symbolFlags, intptr_t addend, long libraryOrdinal, const char* msg,
								LastLookup* last, bool runResolver)
{
	const ImageLoader*	targetImage;
	uintptr_t			symbolAddress;
	
	// resolve symbol
	symbolAddress = this->resolve(context, symbolName, symbolFlags, libraryOrdinal, &targetImage, last, runResolver);

	// do actual update
	return this->bindLocation(context, addr, symbolAddress, type, symbolName, addend, this->getPath(), targetImage ? targetImage->getPath() : NULL, msg);
}


void ImageLoaderMachOCompressed::throwBadBindingAddress(uintptr_t address, uintptr_t segmentEndAddress, int segmentIndex, 
										const uint8_t* startOpcodes, const uint8_t* endOpcodes, const uint8_t* pos)
{
	dyld::throwf("malformed binding opcodes (%ld/%ld): address 0x%08lX is outside segment %s (0x%08lX -> 0x%08lX)",
		(intptr_t)(pos-startOpcodes), (intptr_t)(endOpcodes-startOpcodes), address, segName(segmentIndex), 
		segActualLoadAddress(segmentIndex), segmentEndAddress); 
}


void ImageLoaderMachOCompressed::doBind(const LinkContext& context, bool forceLazysBound)
{
	CRSetCrashLogMessage2(this->getPath());

	// if prebound and loaded at prebound address, and all libraries are same as when this was prebound, then no need to bind
	// note: flat-namespace binaries need to have imports rebound (even if correctly prebound)
	if ( this->usablePrebinding(context) ) {
		// don't need to bind
	}
	else {
		uint64_t t0 = mach_absolute_time();

	#if TEXT_RELOC_SUPPORT
		// if there are __TEXT fixups, temporarily make __TEXT writable
		if ( fTextSegmentBinds ) 
			this->makeTextSegmentWritable(context, true);
	#endif
	
		// run through all binding opcodes
		eachBind(context, &ImageLoaderMachOCompressed::bindAt);
			
	#if TEXT_RELOC_SUPPORT
		// if there were __TEXT fixups, restore write protection
		if ( fTextSegmentBinds ) 
			this->makeTextSegmentWritable(context, false);
	#endif	
	
		// if this image is in the shared cache, but depends on something no longer in the shared cache,
		// there is no way to reset the lazy pointers, so force bind them now
		if ( forceLazysBound || fInSharedCache ) 
			this->doBindJustLazies(context);
            
		// this image is in cache, but something below it is not.  If
        // this image has lazy pointer to a resolver function, then
        // the stub may have been altered to point to a shared lazy pointer.
		if ( fInSharedCache ) 
			this->updateOptimizedLazyPointers(context);
	
		// tell kernel we are done with chunks of LINKEDIT
		if ( !context.preFetchDisabled ) 
			this->markFreeLINKEDIT(context);

		uint64_t t1 = mach_absolute_time();
		ImageLoader::fgTotalRebindCacheTime += (t1-t0);
	}
	
	// set up dyld entry points in image
	// do last so flat main executables will have __dyld or __program_vars set up
	this->setupLazyPointerHandler(context);
	CRSetCrashLogMessage2(NULL);
}


void ImageLoaderMachOCompressed::doBindJustLazies(const LinkContext& context)
{
	eachLazyBind(context, &ImageLoaderMachOCompressed::bindAt);
}

void ImageLoaderMachOCompressed::eachBind(const LinkContext& context, bind_handler handler)
{
	try {
		uint8_t type = 0;
		int segmentIndex = -1;
		uintptr_t address = segActualLoadAddress(0);
		uintptr_t segmentStartAddress = segActualLoadAddress(0);
		uintptr_t segmentEndAddress = segActualEndAddress(0);
		const char* symbolName = NULL;
		uint8_t symboFlags = 0;
		bool libraryOrdinalSet = false;
		long libraryOrdinal = 0;
		intptr_t addend = 0;
		uintptr_t count;
		uintptr_t skip;
		uintptr_t segOffset;
		LastLookup last = { 0, 0, NULL, 0, NULL };
		const uint8_t* const start = fLinkEditBase + fDyldInfo->bind_off;
		const uint8_t* const end = &start[fDyldInfo->bind_size];
		const uint8_t* p = start;
		bool done = false;
		while ( !done && (p < end) ) {
			uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
			uint8_t opcode = *p & BIND_OPCODE_MASK;
			++p;
			switch (opcode) {
				case BIND_OPCODE_DONE:
					done = true;
					break;
				case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
					libraryOrdinal = immediate;
					libraryOrdinalSet = true;
					break;
				case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
					libraryOrdinal = read_uleb128(p, end);
					libraryOrdinalSet = true;
					break;
				case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
					// the special ordinals are negative numbers
					if ( immediate == 0 )
						libraryOrdinal = 0;
					else {
						int8_t signExtended = BIND_OPCODE_MASK | immediate;
						libraryOrdinal = signExtended;
					}
					libraryOrdinalSet = true;
					break;
				case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
					symbolName = (char*)p;
					symboFlags = immediate;
					while (*p != '\0')
						++p;
					++p;
					break;
				case BIND_OPCODE_SET_TYPE_IMM:
					type = immediate;
					break;
				case BIND_OPCODE_SET_ADDEND_SLEB:
					addend = read_sleb128(p, end);
					break;
				case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
					segmentIndex = immediate;
					if ( (segmentIndex >= fSegmentsCount) || (segmentIndex < 0) )
						dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has segment %d which is out of range (0..%d)",
								segmentIndex, fSegmentsCount-1);
			#if TEXT_RELOC_SUPPORT
					if ( !segWriteable(segmentIndex) && !segHasRebaseFixUps(segmentIndex) && !segHasBindFixUps(segmentIndex) )
			#else
					if ( !segWriteable(segmentIndex) )
			#endif
						dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has segment %d which is not writable", segmentIndex);
					segOffset = read_uleb128(p, end);
					if ( segOffset > segSize(segmentIndex) )
						dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has offset 0x%08lX beyond segment size (0x%08lX)", segOffset, segSize(segmentIndex));
					segmentStartAddress = segActualLoadAddress(segmentIndex);
					address = segmentStartAddress + segOffset;
					segmentEndAddress = segActualEndAddress(segmentIndex);
					break;
				case BIND_OPCODE_ADD_ADDR_ULEB:
					address += read_uleb128(p, end);
					break;
				case BIND_OPCODE_DO_BIND:
					if ( (address < segmentStartAddress) || (address >= segmentEndAddress) )
						throwBadBindingAddress(address, segmentEndAddress, segmentIndex, start, end, p);
					if ( symbolName  == NULL )
						dyld::throwf("BIND_OPCODE_DO_BIND missing preceding BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM");
					if ( segmentIndex == -1 )
						dyld::throwf("BIND_OPCODE_DO_BIND missing preceding BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB");
					if ( !libraryOrdinalSet )
						dyld::throwf("BIND_OPCODE_DO_BIND missing preceding BIND_OPCODE_SET_DYLIB_ORDINAL*");
					(this->*handler)(context, address, type, symbolName, symboFlags, addend, libraryOrdinal, "", &last, false);
					address += sizeof(intptr_t);
					break;
				case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
					if ( (address < segmentStartAddress) || (address >= segmentEndAddress) )
						throwBadBindingAddress(address, segmentEndAddress, segmentIndex, start, end, p);
					if ( symbolName  == NULL )
						dyld::throwf("BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB missing preceding BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM");
					if ( segmentIndex == -1 )
						dyld::throwf("BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB missing preceding BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB");
					if ( !libraryOrdinalSet )
						dyld::throwf("BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB missing preceding BIND_OPCODE_SET_DYLIB_ORDINAL*");
					(this->*handler)(context, address, type, symbolName, symboFlags, addend, libraryOrdinal, "", &last, false);
					address += read_uleb128(p, end) + sizeof(intptr_t);
					break;
				case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
					if ( (address < segmentStartAddress) || (address >= segmentEndAddress) )
						throwBadBindingAddress(address, segmentEndAddress, segmentIndex, start, end, p);
					if ( symbolName  == NULL )
						dyld::throwf("BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED missing preceding BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM");
					if ( segmentIndex == -1 )
						dyld::throwf("BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED missing preceding BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB");
					if ( !libraryOrdinalSet )
						dyld::throwf("BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED missing preceding BIND_OPCODE_SET_DYLIB_ORDINAL*");
					(this->*handler)(context, address, type, symbolName, symboFlags, addend, libraryOrdinal, "", &last, false);
					address += immediate*sizeof(intptr_t) + sizeof(intptr_t);
					break;
				case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
					if ( symbolName  == NULL )
						dyld::throwf("BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB missing preceding BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM");
					if ( segmentIndex == -1 )
						dyld::throwf("BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB missing preceding BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB");
					count = read_uleb128(p, end);
					if ( !libraryOrdinalSet )
						dyld::throwf("BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB missing preceding BIND_OPCODE_SET_DYLIB_ORDINAL*");
					skip = read_uleb128(p, end);
					for (uint32_t i=0; i < count; ++i) {
						if ( (address < segmentStartAddress) || (address >= segmentEndAddress) )
							throwBadBindingAddress(address, segmentEndAddress, segmentIndex, start, end, p);
						(this->*handler)(context, address, type, symbolName, symboFlags, addend, libraryOrdinal, "", &last, false);
						address += skip + sizeof(intptr_t);
					}
					break;
				default:
					dyld::throwf("bad bind opcode %d in bind info", *p);
			}
		}
	}
	catch (const char* msg) {
		const char* newMsg = dyld::mkstringf("%s in %s", msg, this->getPath());
		free((void*)msg);
		throw newMsg;
	}
}

void ImageLoaderMachOCompressed::eachLazyBind(const LinkContext& context, bind_handler handler)
{
	try {
		uint8_t type = BIND_TYPE_POINTER;
		int segmentIndex = -1;
		uintptr_t address = segActualLoadAddress(0);
		uintptr_t segmentStartAddress = segActualLoadAddress(0);
		uintptr_t segmentEndAddress = segActualEndAddress(0);
		uintptr_t segOffset;
		const char* symbolName = NULL;
		uint8_t symboFlags = 0;
		long libraryOrdinal = 0;
		intptr_t addend = 0;
		const uint8_t* const start = fLinkEditBase + fDyldInfo->lazy_bind_off;
		const uint8_t* const end = &start[fDyldInfo->lazy_bind_size];
		const uint8_t* p = start;
		bool done = false;
		while ( !done && (p < end) ) {
			uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
			uint8_t opcode = *p & BIND_OPCODE_MASK;
			++p;
			switch (opcode) {
				case BIND_OPCODE_DONE:
					// there is BIND_OPCODE_DONE at end of each lazy bind, don't stop until end of whole sequence
					break;
				case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
					libraryOrdinal = immediate;
					break;
				case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
					libraryOrdinal = read_uleb128(p, end);
					break;
				case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
					// the special ordinals are negative numbers
					if ( immediate == 0 )
						libraryOrdinal = 0;
					else {
						int8_t signExtended = BIND_OPCODE_MASK | immediate;
						libraryOrdinal = signExtended;
					}
					break;
				case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
					symbolName = (char*)p;
					symboFlags = immediate;
					while (*p != '\0')
						++p;
					++p;
					break;
				case BIND_OPCODE_SET_TYPE_IMM:
					type = immediate;
					break;
				case BIND_OPCODE_SET_ADDEND_SLEB:
					addend = read_sleb128(p, end);
					break;
				case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
					segmentIndex = immediate;
					if ( (segmentIndex >= fSegmentsCount) || (segmentIndex < 0) )
						dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has segment %d which is out of range (0..%d)",
								segmentIndex, fSegmentsCount-1);
					if ( !segWriteable(segmentIndex) )
						dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has segment %d which is not writable", segmentIndex);
					segOffset = read_uleb128(p, end);
					if ( segOffset > segSize(segmentIndex) )
						dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has offset 0x%08lX beyond segment size (0x%08lX)", segOffset, segSize(segmentIndex));
					segmentStartAddress = segActualLoadAddress(segmentIndex);
					segmentEndAddress = segActualEndAddress(segmentIndex);
					address = segmentStartAddress + segOffset;
					break;
				case BIND_OPCODE_ADD_ADDR_ULEB:
					address += read_uleb128(p, end);
					break;
				case BIND_OPCODE_DO_BIND:
					if ( segmentIndex == -1 )
						dyld::throwf("BIND_OPCODE_DO_BIND missing preceding BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB");
					if ( (address < segmentStartAddress) || (address >= segmentEndAddress) )
						throwBadBindingAddress(address, segmentEndAddress, segmentIndex, start, end, p);
					if ( symbolName  == NULL )
						dyld::throwf("BIND_OPCODE_DO_BIND missing preceding BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM");
					(this->*handler)(context, address, type, symbolName, symboFlags, addend, libraryOrdinal, "forced lazy ", NULL, false);
					address += sizeof(intptr_t);
					break;
				case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
				case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
				case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
				default:
					dyld::throwf("bad lazy bind opcode %d", *p);
			}
		}
	}  

	catch (const char* msg) {
		const char* newMsg = dyld::mkstringf("%s in %s", msg, this->getPath());
		free((void*)msg);
		throw newMsg;
	}
}

// A program built targeting 10.5 will have hybrid stubs.  When used with weak symbols
// the classic lazy loader is used even when running on 10.6
uintptr_t ImageLoaderMachOCompressed::doBindLazySymbol(uintptr_t* lazyPointer, const LinkContext& context)
{
	// only works with compressed LINKEDIT if classic symbol table is also present
	const macho_nlist* symbolTable = NULL;
	const char* symbolTableStrings = NULL;
	const dysymtab_command* dynSymbolTable = NULL;
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SYMTAB:
				{
					const struct symtab_command* symtab = (struct symtab_command*)cmd;
					symbolTableStrings = (const char*)&fLinkEditBase[symtab->stroff];
					symbolTable = (macho_nlist*)(&fLinkEditBase[symtab->symoff]);
				}
				break;
			case LC_DYSYMTAB:
				dynSymbolTable = (struct dysymtab_command*)cmd;
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	// no symbol table => no lookup by address
	if ( (symbolTable == NULL) || (dynSymbolTable == NULL) )
		dyld::throwf("classic lazy binding used with compressed LINKEDIT at %p in image %s", lazyPointer, this->getPath());

	// scan for all lazy-pointer sections
	const bool twoLevel = this->usesTwoLevelNameSpace();
	const uint32_t* const indirectTable = (uint32_t*)&fLinkEditBase[dynSymbolTable->indirectsymoff];
	cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
				{
					const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
					const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
					const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
					for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
						const uint8_t type = sect->flags & SECTION_TYPE;
						uint32_t symbolIndex = INDIRECT_SYMBOL_LOCAL;
						if ( type == S_LAZY_SYMBOL_POINTERS ) {
							const size_t pointerCount = sect->size / sizeof(uintptr_t);
							uintptr_t* const symbolPointers = (uintptr_t*)(sect->addr + fSlide);
							if ( (lazyPointer >= symbolPointers) && (lazyPointer < &symbolPointers[pointerCount]) ) {
								const uint32_t indirectTableOffset = sect->reserved1;
								const size_t lazyIndex = lazyPointer - symbolPointers;
								symbolIndex = indirectTable[indirectTableOffset + lazyIndex];
							}
						}
						if ( (symbolIndex != INDIRECT_SYMBOL_ABS) && (symbolIndex != INDIRECT_SYMBOL_LOCAL) ) {
							const macho_nlist* symbol = &symbolTable[symbolIndex];
							const char* symbolName = &symbolTableStrings[symbol->n_un.n_strx];
							int libraryOrdinal = GET_LIBRARY_ORDINAL(symbol->n_desc);
							if ( !twoLevel || context.bindFlat ) 
								libraryOrdinal = BIND_SPECIAL_DYLIB_FLAT_LOOKUP;
							uintptr_t ptrToBind = (uintptr_t)lazyPointer;
							uintptr_t symbolAddr = bindAt(context, ptrToBind, BIND_TYPE_POINTER, symbolName, 0, 0, libraryOrdinal, "lazy ", NULL);
							++fgTotalLazyBindFixups;
							return symbolAddr;
						}
					}
				}
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	dyld::throwf("lazy pointer not found at address %p in image %s", lazyPointer, this->getPath());
}



uintptr_t ImageLoaderMachOCompressed::doBindFastLazySymbol(uint32_t lazyBindingInfoOffset, const LinkContext& context,
															void (*lock)(), void (*unlock)())
{
	// <rdar://problem/8663923> race condition with flat-namespace lazy binding
	if ( this->usesTwoLevelNameSpace() ) {
		// two-level namespace lookup does not require lock because dependents can't be unloaded before this image
	}
	else {
		// acquire dyld global lock
		if ( lock != NULL )
			lock();
	}
	
	const uint8_t* const start = fLinkEditBase + fDyldInfo->lazy_bind_off;
	const uint8_t* const end = &start[fDyldInfo->lazy_bind_size];
	uint8_t segIndex;
	uintptr_t segOffset;
	int libraryOrdinal;
	const char* symbolName;
	bool doneAfterBind;
	uintptr_t result;
	do {
		if ( ! getLazyBindingInfo(lazyBindingInfoOffset, start, end, &segIndex, &segOffset, &libraryOrdinal, &symbolName, &doneAfterBind) )
			dyld::throwf("bad lazy bind info");

		if ( segIndex >= fSegmentsCount )
			dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has segment %d which is too large (0..%d)", 
							segIndex, fSegmentsCount-1);
		if ( segOffset > segSize(segIndex) )
			dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has offset 0x%08lX beyond segment size (0x%08lX)", segOffset, segSize(segIndex));
		uintptr_t address = segActualLoadAddress(segIndex) + segOffset;
		result = this->bindAt(context, address, BIND_TYPE_POINTER, symbolName, 0, 0, libraryOrdinal, "lazy ", NULL, true);
		// <rdar://problem/24140465> Some old apps had multiple lazy symbols bound at once
	} while (!doneAfterBind && !context.strictMachORequired);

	if ( !this->usesTwoLevelNameSpace() ) {
		// release dyld global lock
		if ( unlock != NULL )
			unlock();
	}
	return result;
}

void ImageLoaderMachOCompressed::initializeCoalIterator(CoalIterator& it, unsigned int loadOrder, unsigned)
{
	it.image = this;
	it.symbolName = " ";
	it.loadOrder = loadOrder;
	it.weakSymbol = false;
	it.symbolMatches = false;
	it.done = false;
	it.curIndex = 0;
	it.endIndex = this->fDyldInfo->weak_bind_size;
	it.address = 0;
	it.type = 0;
	it.addend = 0;
}


bool ImageLoaderMachOCompressed::incrementCoalIterator(CoalIterator& it)
{
	if ( it.done )
		return false;
		
	if ( this->fDyldInfo->weak_bind_size == 0 ) {
		/// hmmm, ld set MH_WEAK_DEFINES or MH_BINDS_TO_WEAK, but there is no weak binding info
		it.done = true;
		it.symbolName = "~~~";
		return true;
	}
	const uint8_t* start = fLinkEditBase + fDyldInfo->weak_bind_off;
	const uint8_t* p = start + it.curIndex;
	const uint8_t* end = fLinkEditBase + fDyldInfo->weak_bind_off + this->fDyldInfo->weak_bind_size;
	uintptr_t count;
	uintptr_t skip;
	uintptr_t segOffset;
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
				if ( immediate >= fSegmentsCount )
					dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has segment %d which is too large (0..%d)",
							immediate, fSegmentsCount-1);
		#if __arm__
				// <rdar://problem/23138428> iOS app compatibility
				if ( !segWriteable(immediate) && it.image->isPositionIndependentExecutable() )
		#elif TEXT_RELOC_SUPPORT
				// <rdar://problem/23479396&23590867> i386 OS X app compatibility
				if ( !segWriteable(immediate) && !segHasRebaseFixUps(immediate) && !segHasBindFixUps(immediate)
					&& (!it.image->isExecutable() || it.image->isPositionIndependentExecutable()) )
		#else
				if ( !segWriteable(immediate) )
        #endif
					dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB targets segment %s which is not writable", segName(immediate));
				segOffset = read_uleb128(p, end);
				if ( segOffset > segSize(immediate) )
					dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has offset 0x%08lX beyond segment size (0x%08lX)", segOffset, segSize(immediate));
				it.address = segActualLoadAddress(immediate) + segOffset;
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

uintptr_t ImageLoaderMachOCompressed::getAddressCoalIterator(CoalIterator& it, const LinkContext& context)
{
	//dyld::log("looking for %s in %s\n", it.symbolName, this->getPath());
	const ImageLoader* foundIn = NULL;
	const ImageLoader::Symbol* sym = this->findShallowExportedSymbol(it.symbolName, &foundIn);
	if ( sym != NULL ) {
		//dyld::log("sym=%p, foundIn=%p\n", sym, foundIn);
		return foundIn->getExportedSymbolAddress(sym, context, this);
	}
	return 0;
}


void ImageLoaderMachOCompressed::updateUsesCoalIterator(CoalIterator& it, uintptr_t value, ImageLoader* targetImage, unsigned targetIndex, const LinkContext& context)
{
	// <rdar://problem/6570879> weak binding done too early with inserted libraries
	if ( this->getState() < dyld_image_state_bound  )
		return;

	const uint8_t* start = fLinkEditBase + fDyldInfo->weak_bind_off;
	const uint8_t* p = start + it.curIndex;
	const uint8_t* end = fLinkEditBase + fDyldInfo->weak_bind_off + this->fDyldInfo->weak_bind_size;

	uint8_t type = it.type;
	uintptr_t address = it.address;
	const char* symbolName = it.symbolName;
	intptr_t addend = it.addend;
	uintptr_t count;
	uintptr_t skip;
	uintptr_t segOffset;
	bool done = false;
	bool boundSomething = false;
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
				if ( immediate >= fSegmentsCount )
					dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has segment %d which is too large (0..%d)",
							immediate, fSegmentsCount-1);
		#if __arm__
				// <rdar://problem/23138428> iOS app compatibility
				if ( !segWriteable(immediate) && it.image->isPositionIndependentExecutable() )
		#elif TEXT_RELOC_SUPPORT
				// <rdar://problem/23479396&23590867> i386 OS X app compatibility
				if ( !segWriteable(immediate) && !segHasRebaseFixUps(immediate) && !segHasBindFixUps(immediate)
					&& (!it.image->isExecutable() || it.image->isPositionIndependentExecutable()) )
		#else
				if ( !segWriteable(immediate) )
        #endif
					dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB targets segment %s which is not writable", segName(immediate));
				segOffset = read_uleb128(p, end);
				if ( segOffset > segSize(immediate) )
					dyld::throwf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB has offset 0x%08lX beyond segment size (0x%08lX)", segOffset, segSize(immediate));
				address = segActualLoadAddress(immediate) + segOffset;
				break;
			case BIND_OPCODE_ADD_ADDR_ULEB:
				address += read_uleb128(p, end);
				break;
			case BIND_OPCODE_DO_BIND:
				bindLocation(context, address, value, type, symbolName, addend, this->getPath(), targetImage ? targetImage->getPath() : NULL, "weak ");
				boundSomething = true;
				address += sizeof(intptr_t);
				break;
			case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
				bindLocation(context, address, value, type, symbolName, addend, this->getPath(), targetImage ? targetImage->getPath() : NULL, "weak ");
				boundSomething = true;
				address += read_uleb128(p, end) + sizeof(intptr_t);
				break;
			case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
				bindLocation(context, address, value, type, symbolName, addend, this->getPath(), targetImage ? targetImage->getPath() : NULL, "weak ");
				boundSomething = true;
				address += immediate*sizeof(intptr_t) + sizeof(intptr_t);
				break;
			case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
				count = read_uleb128(p, end);
				skip = read_uleb128(p, end);
				for (uint32_t i=0; i < count; ++i) {
					bindLocation(context, address, value, type, symbolName, addend, this->getPath(), targetImage ? targetImage->getPath() : NULL, "weak ");
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

uintptr_t ImageLoaderMachOCompressed::interposeAt(const LinkContext& context, uintptr_t addr, uint8_t type, const char*, 
												uint8_t, intptr_t, long, const char*, LastLookup*, bool runResolver)
{
	if ( type == BIND_TYPE_POINTER ) {
		uintptr_t* fixupLocation = (uintptr_t*)addr;
		uintptr_t curValue = *fixupLocation;
		uintptr_t newValue = interposedAddress(context, curValue, this);
		if ( newValue != curValue) {
			*fixupLocation = newValue;
		}
	}
	return 0;
}

void ImageLoaderMachOCompressed::doInterpose(const LinkContext& context)
{
	if ( context.verboseInterposing )
		dyld::log("dyld: interposing %lu tuples onto image: %s\n", fgInterposingTuples.size(), this->getPath());

	// update prebound symbols
	eachBind(context, &ImageLoaderMachOCompressed::interposeAt);
	eachLazyBind(context, &ImageLoaderMachOCompressed::interposeAt);
}


uintptr_t ImageLoaderMachOCompressed::dynamicInterposeAt(const LinkContext& context, uintptr_t addr, uint8_t type, const char* symbolName, 
												uint8_t, intptr_t, long, const char*, LastLookup*, bool runResolver)
{
	if ( type == BIND_TYPE_POINTER ) {
		uintptr_t* fixupLocation = (uintptr_t*)addr;
		uintptr_t value = *fixupLocation;
		// don't apply interposing to table entries.
		if ( (context.dynamicInterposeArray <= (void*)addr) && ((void*)addr < &context.dynamicInterposeArray[context.dynamicInterposeCount]) )
			return 0;
		for(size_t i=0; i < context.dynamicInterposeCount; ++i) {
			if ( value == (uintptr_t)context.dynamicInterposeArray[i].replacee ) {
				if ( context.verboseInterposing ) {
					dyld::log("dyld: dynamic interposing: at %p replace %p with %p in %s\n", 
						fixupLocation, context.dynamicInterposeArray[i].replacee, context.dynamicInterposeArray[i].replacement, this->getPath());
				}
				*fixupLocation = (uintptr_t)context.dynamicInterposeArray[i].replacement;
			}
		}
	}
	return 0;
}

void ImageLoaderMachOCompressed::dynamicInterpose(const LinkContext& context)
{
	if ( context.verboseInterposing )
		dyld::log("dyld: dynamic interposing %lu tuples onto image: %s\n", context.dynamicInterposeCount, this->getPath());

	// update already bound references to symbols
	eachBind(context, &ImageLoaderMachOCompressed::dynamicInterposeAt);
	eachLazyBind(context, &ImageLoaderMachOCompressed::dynamicInterposeAt);
}

const char* ImageLoaderMachOCompressed::findClosestSymbol(const void* addr, const void** closestAddr) const
{
	return ImageLoaderMachO::findClosestSymbol((mach_header*)fMachOData, addr, closestAddr);
}


#if PREBOUND_IMAGE_SUPPORT
void ImageLoaderMachOCompressed::resetPreboundLazyPointers(const LinkContext& context)
{
	// no way to back off a prebound compress image
}
#endif


#if __arm__ || __x86_64__
void ImageLoaderMachOCompressed::updateAlternateLazyPointer(uint8_t* stub, void** originalLazyPointerAddr, const LinkContext& context)
{
#if __arm__ 
	uint32_t* instructions = (uint32_t*)stub;
    // sanity check this is a stub we understand
	if ( (instructions[0] != 0xe59fc004) || (instructions[1] != 0xe08fc00c) || (instructions[2] != 0xe59cf000) )
		return;
    
	void** lazyPointerAddr = (void**)(instructions[3] + (stub + 12));
#endif
#if __x86_64__
    // sanity check this is a stub we understand
	if ( (stub[0] != 0xFF) || (stub[1] != 0x25) )
		return;
    int32_t ripOffset = *((int32_t*)(&stub[2]));
	void** lazyPointerAddr = (void**)(ripOffset + stub + 6);
#endif

   // if stub does not use original lazy pointer (meaning it was optimized by update_dyld_shared_cache)
    if ( lazyPointerAddr != originalLazyPointerAddr ) {
		// <rdar://problem/12928448> only de-optimization lazy pointers if they are part of shared cache not loaded (because overridden)
		const ImageLoader* lazyPointerImage = context.findImageContainingAddress(lazyPointerAddr);
		if ( lazyPointerImage != NULL )
			return;
		
        // copy newly re-bound lazy pointer value to shared lazy pointer
        *lazyPointerAddr = *originalLazyPointerAddr;
		
		if ( context.verboseBind )
			dyld::log("dyld: alter bind: %s: *0x%08lX = 0x%08lX \n",
					  this->getShortName(), (long)lazyPointerAddr, (long)*originalLazyPointerAddr);
    }
}
#endif


// <rdar://problem/8890875> overriding shared cache dylibs with resolvers fails
void ImageLoaderMachOCompressed::updateOptimizedLazyPointers(const LinkContext& context)
{
#if __arm__ || __x86_64__
	// find stubs and indirect symbol table
	const struct macho_section* stubsSection = NULL;
	const dysymtab_command* dynSymbolTable = NULL;
	const macho_header* mh = (macho_header*)fMachOData;
	const uint32_t cmd_count = mh->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		if (cmd->cmd == LC_SEGMENT_COMMAND) {
			const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
			const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
			const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
			for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
				const uint8_t type = sect->flags & SECTION_TYPE;
				if ( type == S_SYMBOL_STUBS ) 
					stubsSection = sect;
			}
		}
		else if ( cmd->cmd == LC_DYSYMTAB ) {
			dynSymbolTable = (struct dysymtab_command*)cmd;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	if ( dynSymbolTable == NULL )
		return;
	const uint32_t* const indirectTable = (uint32_t*)&fLinkEditBase[dynSymbolTable->indirectsymoff];
	if ( stubsSection == NULL )
		return;
	const uint32_t stubsSize = stubsSection->reserved2;
	const uint32_t stubsCount = (uint32_t)(stubsSection->size / stubsSize);
	const uint32_t stubsIndirectTableOffset = stubsSection->reserved1;
	if ( (stubsIndirectTableOffset+stubsCount) > dynSymbolTable->nindirectsyms )
		return;
	uint8_t* const stubsAddr = (uint8_t*)(stubsSection->addr + this->fSlide);

	// for each lazy pointer section
	cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		if (cmd->cmd == LC_SEGMENT_COMMAND) {
			const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
			const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
			const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
			for (const struct macho_section* lazyPointerSection=sectionsStart; lazyPointerSection < sectionsEnd; ++lazyPointerSection) {
				const uint8_t type = lazyPointerSection->flags & SECTION_TYPE;
				if ( type != S_LAZY_SYMBOL_POINTERS )
					continue;
				const uint32_t lazyPointersCount = (uint32_t)(lazyPointerSection->size / sizeof(void*));
				const uint32_t lazyPointersIndirectTableOffset = lazyPointerSection->reserved1;
				if ( (lazyPointersIndirectTableOffset+lazyPointersCount) > dynSymbolTable->nindirectsyms )
					continue;
				void** const lazyPointersAddr = (void**)(lazyPointerSection->addr + this->fSlide);
				// for each lazy pointer
				for(uint32_t lpIndex=0; lpIndex < lazyPointersCount; ++lpIndex) {
					const uint32_t lpSymbolIndex = indirectTable[lazyPointersIndirectTableOffset+lpIndex];
					// find matching stub and validate it uses this lazy pointer
					for(uint32_t stubIndex=0; stubIndex < stubsCount; ++stubIndex) {
						if ( indirectTable[stubsIndirectTableOffset+stubIndex] == lpSymbolIndex ) {
							this->updateAlternateLazyPointer(stubsAddr+stubIndex*stubsSize, &lazyPointersAddr[lpIndex], context);
							break;
						}
					}
				}

			}
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}

#endif
}


void ImageLoaderMachOCompressed::registerEncryption(const encryption_info_command* encryptCmd, const LinkContext& context)
{
#if __arm__ || __arm64__
	if ( encryptCmd == NULL )
		return;
	const mach_header* mh = NULL;
	for(unsigned int i=0; i < fSegmentsCount; ++i) {
		if ( (segFileOffset(i) == 0) && (segFileSize(i) != 0) ) {
			mh = (mach_header*)segActualLoadAddress(i);
			break;
		}
	}
	void* start = ((uint8_t*)mh) + encryptCmd->cryptoff;
	size_t len = encryptCmd->cryptsize;
	uint32_t cputype = mh->cputype;
	uint32_t cpusubtype = mh->cpusubtype;
	uint32_t cryptid = encryptCmd->cryptid;
	if (context.verboseMapping) {
		 dyld::log("                      0x%08lX->0x%08lX configured for FairPlay decryption\n", (long)start, (long)start+len);
	}
	int result = mremap_encrypted(start, len, cryptid, cputype, cpusubtype);
	if ( result != 0 ) {
		dyld::throwf("mremap_encrypted() => %d, errno=%d for %s\n", result, errno, this->getPath());
	}
#endif
}




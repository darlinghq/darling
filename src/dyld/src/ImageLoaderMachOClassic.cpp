/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2004-2010 Apple Inc. All rights reserved.
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

// work around until conformance work is complete rdar://problem/4508801
#define __srr0	srr0 
#define __eip	eip 
#define __rip	rip 


#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/stat.h> 
#include <sys/mman.h>
#include <mach/mach.h>
#include <mach/thread_status.h>
#include <mach-o/loader.h> 
#include <mach-o/reloc.h> 
#include <mach-o/nlist.h> 
#include <sys/sysctl.h>
#include <libkern/OSAtomic.h>
#include <libkern/OSCacheControl.h>
#include <mach-o/dyld_images.h>

#if __x86_64__
	#include <mach-o/x86_64/reloc.h>
#endif
#if __arm__
	#include <mach-o/arm/reloc.h>
#endif

#include "ImageLoaderMachOClassic.h"

// in dyldStartup.s
extern "C" void stub_binding_helper_i386_old();


#if __x86_64__
	#define POINTER_RELOC X86_64_RELOC_UNSIGNED
#else
	#define POINTER_RELOC GENERIC_RELOC_VANILLA
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
ImageLoaderMachOClassic* ImageLoaderMachOClassic::instantiateMainExecutable(const macho_header* mh, uintptr_t slide, const char* path, 
																		unsigned int segCount, unsigned int libCount, const LinkContext& context)
{
	ImageLoaderMachOClassic* image = ImageLoaderMachOClassic::instantiateStart(mh, path, segCount, libCount);

	// set slide for PIE programs
	image->setSlide(slide);

	// for PIE record end of program, to know where to start loading dylibs
	if ( slide != 0 )
		fgNextPIEDylibAddress = (uintptr_t)image->getEnd();

	image->disableCoverageCheck();
	image->instantiateFinish(context);
	image->setMapped(context);

#if __i386__
	// kernel may have mapped in __IMPORT segment read-only, we need it read/write to do binding
	if ( image->fReadOnlyImportSegment ) {
		for(unsigned int i=0; i < image->fSegmentsCount; ++i) {
			if ( image->segIsReadOnlyImport(i) )
				image->segMakeWritable(i, context);
		}
	}
#endif

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
ImageLoaderMachOClassic* ImageLoaderMachOClassic::instantiateFromFile(const char* path, int fd, const uint8_t* fileData, size_t lenFileData,
															uint64_t offsetInFat, uint64_t lenInFat, const struct stat& info, 
															unsigned int segCount, unsigned int libCount, 
															const struct linkedit_data_command* codeSigCmd, const LinkContext& context)
{
	ImageLoaderMachOClassic* image = ImageLoaderMachOClassic::instantiateStart((macho_header*)fileData, path, segCount, libCount);
	try {
		// record info about file  
		image->setFileInfo(info.st_dev, info.st_ino, info.st_mtime);

		// if this image is code signed, let kernel validate signature before mapping any pages from image
		image->loadCodeSignature(codeSigCmd, fd, offsetInFat, context);
		
		// Validate that first data we read with pread actually matches with code signature
		image->validateFirstPages(codeSigCmd, fd, fileData, lenFileData, offsetInFat, context);

		// mmap segments
		image->mapSegmentsClassic(fd, offsetInFat, lenInFat, info.st_size, context);

		// finish up
		image->instantiateFinish(context);

		// if path happens to be same as in LC_DYLIB_ID load command use that, otherwise malloc a copy of the path
		const char* installName = image->getInstallPath();
		if ( (installName != NULL) && (strcmp(installName, path) == 0) && (path[0] == '/') )
			image->setPathUnowned(installName);
		else if ( (path[0] != '/') || (strstr(path, "../") != NULL) ) {
			// rdar://problem/10733082 Fix up @path based paths during introspection
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
ImageLoaderMachOClassic* ImageLoaderMachOClassic::instantiateFromCache(const macho_header* mh, const char* path, long slide, const struct stat& info,
																unsigned int segCount, unsigned int libCount, const LinkContext& context)
{
	ImageLoaderMachOClassic* image = ImageLoaderMachOClassic::instantiateStart(mh, path, segCount, libCount);
	try {
		// record info about file  
		image->setFileInfo(info.st_dev, info.st_ino, info.st_mtime);

		// remember this is from shared cache and cannot be unloaded
		image->fInSharedCache = true;
		image->setNeverUnload();
		image->disableCoverageCheck();

		// segments already mapped in cache
		if ( context.verboseMapping ) {
			dyld::log("dyld: Using shared cached for %s\n", path);
			for(unsigned int i=0, e=image->segmentCount(); i < e; ++i) {
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
ImageLoaderMachOClassic* ImageLoaderMachOClassic::instantiateFromMemory(const char* moduleName, const macho_header* mh, uint64_t len, 
															unsigned int segCount, unsigned int libCount, const LinkContext& context)
{
	ImageLoaderMachOClassic* image = ImageLoaderMachOClassic::instantiateStart(mh, moduleName, segCount, libCount);
	try {
		// map segments 
		if ( mh->filetype == MH_EXECUTE ) 
			throw "can't load another MH_EXECUTE";
		
		// vmcopy segments
		image->ImageLoaderMachO::mapSegments((const void*)mh, len, context);

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


ImageLoaderMachOClassic::ImageLoaderMachOClassic(const macho_header* mh, const char* path, 
													unsigned int segCount, uint32_t segOffsets[], unsigned int libCount)
 : ImageLoaderMachO(mh, path, segCount, segOffsets, libCount), fStrings(NULL), fSymbolTable(NULL), fDynamicInfo(NULL)
{
}

// construct ImageLoaderMachOClassic using "placement new" with SegmentMachO objects array at end
ImageLoaderMachOClassic* ImageLoaderMachOClassic::instantiateStart(const macho_header* mh, const char* path,
																		unsigned int segCount, unsigned int libCount)
{
	size_t size = sizeof(ImageLoaderMachOClassic) + segCount * sizeof(uint32_t) + libCount * sizeof(ImageLoader*);
	ImageLoaderMachOClassic* allocatedSpace = static_cast<ImageLoaderMachOClassic*>(malloc(size));
	if ( allocatedSpace == NULL )
		throw "malloc failed";
	uint32_t* segOffsets = ((uint32_t*)(((uint8_t*)allocatedSpace) + sizeof(ImageLoaderMachOClassic)));
	bzero(&segOffsets[segCount], libCount*sizeof(void*));	// zero out lib array
	return new (allocatedSpace) ImageLoaderMachOClassic(mh, path, segCount, segOffsets, libCount);
}



// common code to finish initializing object
void ImageLoaderMachOClassic::instantiateFinish(const LinkContext& context)
{
	// now that segments are mapped in, get real fMachOData, fLinkEditBase, and fSlide
	this->parseLoadCmds(context);
}

ImageLoaderMachOClassic::~ImageLoaderMachOClassic()
{
	// don't do clean up in ~ImageLoaderMachO() because virtual call to segmentCommandOffsets() won't work
	destroy();
}

uint32_t* ImageLoaderMachOClassic::segmentCommandOffsets() const
{
	return ((uint32_t*)(((uint8_t*)this) + sizeof(ImageLoaderMachOClassic)));
}


ImageLoader* ImageLoaderMachOClassic::libImage(unsigned int libIndex) const
{
	const uintptr_t* images = ((uintptr_t*)(((uint8_t*)this) + sizeof(ImageLoaderMachOClassic) + fSegmentsCount*sizeof(uint32_t)));
	// mask off low bits
	return (ImageLoader*)(images[libIndex] & (-4));
}

bool ImageLoaderMachOClassic::libReExported(unsigned int libIndex) const
{
	const uintptr_t* images = ((uintptr_t*)(((uint8_t*)this) + sizeof(ImageLoaderMachOClassic) + fSegmentsCount*sizeof(uint32_t)));
	// re-export flag is low bit
	return ((images[libIndex] & 1) != 0);
}	

bool ImageLoaderMachOClassic::libIsUpward(unsigned int libIndex) const
{
	const uintptr_t* images = ((uintptr_t*)(((uint8_t*)this) + sizeof(ImageLoaderMachOClassic) + fSegmentsCount*sizeof(uint32_t)));
	// upward flag is second bit
	return ((images[libIndex] & 2) != 0);
}	


void ImageLoaderMachOClassic::setLibImage(unsigned int libIndex, ImageLoader* image, bool reExported, bool upward)
{
	uintptr_t* images = ((uintptr_t*)(((uint8_t*)this) + sizeof(ImageLoaderMachOClassic) + fSegmentsCount*sizeof(uint32_t)));
	uintptr_t value = (uintptr_t)image;
	if ( reExported ) 
		value |= 1;
	if ( upward ) 
		value |= 2;
	images[libIndex] = value;
}


void ImageLoaderMachOClassic::setSymbolTableInfo(const macho_nlist* symbols, const char* strings, const dysymtab_command* dynSym)
{
	fSymbolTable = symbols;
	fStrings = strings;
	fDynamicInfo = dynSym;
}



#if SPLIT_SEG_DYLIB_SUPPORT	
unsigned int
ImageLoaderMachOClassic::getExtraZeroFillEntriesCount()
{
	// calculate mapping entries
	unsigned int extraZeroFillEntries = 0;
	for(unsigned int i=0; i < fSegmentsCount; ++i) {
		if ( segHasTrailingZeroFill(i) )
			++extraZeroFillEntries;
	}
	
	return extraZeroFillEntries;
}

void
ImageLoaderMachOClassic::initMappingTable(uint64_t offsetInFat,
								   shared_file_mapping_np *mappingTable)
{
	for(unsigned int i=0,entryIndex=0; i < fSegmentsCount; ++i, ++entryIndex) {
		shared_file_mapping_np* entry = &mappingTable[entryIndex];
		entry->sfm_address			= segActualLoadAddress(i);
		entry->sfm_size				= segFileSize(i);
		entry->sfm_file_offset		= segFileOffset(i) + offsetInFat;
		entry->sfm_init_prot		= VM_PROT_NONE;
		if ( !segUnaccessible(i) ) {
			if ( segExecutable(i) )
				entry->sfm_init_prot   |= VM_PROT_EXECUTE;
			if ( segReadable(i) )
				entry->sfm_init_prot   |= VM_PROT_READ;
			if ( segWriteable(i) )
				entry->sfm_init_prot   |= VM_PROT_WRITE | VM_PROT_COW;
		}
		entry->sfm_max_prot			= entry->sfm_init_prot;
		if ( segHasTrailingZeroFill(i) ) {
			shared_file_mapping_np* zfentry = &mappingTable[++entryIndex];
			zfentry->sfm_address		= entry->sfm_address + segFileSize(i);
			zfentry->sfm_size			= segSize(i) - segFileSize(i);
			zfentry->sfm_file_offset	= 0;
			zfentry->sfm_init_prot		= entry->sfm_init_prot | VM_PROT_COW | VM_PROT_ZF;
			zfentry->sfm_max_prot		= zfentry->sfm_init_prot;
		}
	}
}

int
ImageLoaderMachOClassic::mapSplitSegDylibOutsideSharedRegion(int fd,
													uint64_t offsetInFat,
													uint64_t lenInFat,
													uint64_t fileLen,
													const LinkContext& context)
{
	uintptr_t nextAltLoadAddress = 0;
	const unsigned int segmentCount = fSegmentsCount;
	const unsigned int extraZeroFillEntries = getExtraZeroFillEntriesCount();
	const unsigned int regionCount = segmentCount+extraZeroFillEntries;
	shared_file_mapping_np regions[regionCount];
	initMappingTable(offsetInFat, regions);
	int r = -1;
	// find space somewhere to allocate split seg
	bool foundRoom = false;
	while ( ! foundRoom ) {
		foundRoom = true;
		for(unsigned int i=0; i < regionCount; ++i) {
			vm_address_t addr = (vm_address_t)(nextAltLoadAddress + regions[i].sfm_address - regions[0].sfm_address);
			vm_size_t size = (vm_size_t)regions[i].sfm_size ;
			r = vm_allocate(mach_task_self(), &addr, size, false /*only this range*/);
			if ( 0 != r ) {
				// no room here, deallocate what has succeeded so far
				for(unsigned int j=0; j < i; ++j) {
					addr = (vm_address_t)(nextAltLoadAddress + regions[j].sfm_address - regions[0].sfm_address);
					size = (vm_size_t)(regions[j].sfm_size);
					(void)vm_deallocate(mach_task_self(), addr, size);
				}
				nextAltLoadAddress += 0x00100000;  // skip ahead 1MB and try again
				// skip over shared region
				if ( (SHARED_REGION_BASE <= nextAltLoadAddress) && (nextAltLoadAddress < (SHARED_REGION_BASE + SHARED_REGION_SIZE)) )
					nextAltLoadAddress = (SHARED_REGION_BASE + SHARED_REGION_SIZE);
				if ( nextAltLoadAddress > 0xFF000000 )
					throw "can't map split seg anywhere";
				foundRoom = false;
				break;
			}
		}
	}
	
	// map in each region
	uintptr_t slide = (uintptr_t)(nextAltLoadAddress - regions[0].sfm_address);
	this->setSlide(slide);
	for(unsigned int i=0; i < regionCount; ++i) {
		if ( ((regions[i].sfm_init_prot & VM_PROT_ZF) != 0) || (regions[i].sfm_size == 0) ) {
			// nothing to mmap for zero-fills areas, they are just vm_allocated 
		}
		else {
			void* mmapAddress = (void*)(uintptr_t)(regions[i].sfm_address + slide);
			size_t size = (size_t)regions[i].sfm_size;
			int protection = 0;
			if ( regions[i].sfm_init_prot & VM_PROT_EXECUTE )
				protection   |= PROT_EXEC;
			if ( regions[i].sfm_init_prot & VM_PROT_READ )
				protection   |= PROT_READ;
			if ( regions[i].sfm_init_prot & VM_PROT_WRITE )
				protection   |= PROT_WRITE;
			off_t offset = regions[i].sfm_file_offset;
			//dyld::log("mmap(%p, 0x%08lX, %s\n", mmapAddress, size, fPath);
			mmapAddress = mmap(mmapAddress, size, protection, MAP_FIXED | MAP_PRIVATE, fd, offset);
			if ( mmapAddress == ((void*)(-1)) )
				throw "mmap error";
		}
	}
	
	// logging
	if ( context.verboseMapping ) {
		dyld::log("dyld: Mapping split-seg outside shared region, slid by 0x%08lX %s\n", this->fSlide, this->getPath());
		for(unsigned int segIndex=0,entryIndex=0; segIndex < segmentCount; ++segIndex, ++entryIndex){
			const shared_file_mapping_np* entry = &regions[entryIndex];
			if ( (entry->sfm_init_prot & VM_PROT_ZF) == 0 ) 
				dyld::log("%18s at 0x%08lX->0x%08lX\n",
						segName(segIndex), segActualLoadAddress(segIndex), segActualEndAddress(segIndex)-1);
			if ( entryIndex < (regionCount-1) ) {
				const shared_file_mapping_np* nextEntry = &regions[entryIndex+1];
				if ( (nextEntry->sfm_init_prot & VM_PROT_ZF) != 0 ) {
					uint64_t segOffset = nextEntry->sfm_address - entry->sfm_address;
					dyld::log("%18s at 0x%08lX->0x%08lX (zerofill)\n",
							segName(segIndex), (uintptr_t)(segActualLoadAddress(segIndex) + segOffset), (uintptr_t)(segActualLoadAddress(segIndex) + segOffset + nextEntry->sfm_size - 1));
					++entryIndex;
				}
			}
		}
	}
	
	return r;
}
#endif // SPLIT_SEG_DYLIB_SUPPORT


void ImageLoaderMachOClassic::mapSegmentsClassic(int fd, uint64_t offsetInFat, uint64_t lenInFat, uint64_t fileLen, const LinkContext& context)
{
	// non-split segment libraries handled by super class
	if ( !fIsSplitSeg )
		return ImageLoaderMachO::mapSegments(fd, offsetInFat, lenInFat, fileLen, context);

#if SPLIT_SEG_SHARED_REGION_SUPPORT	
	// don't map split-seg dylibs into shared region if shared cache is in use
	if ( ! context.dyldLoadedAtSameAddressNeededBySharedCache ) {
		// try to map into shared region at preferred address
		if ( mapSplitSegDylibInfoSharedRegion(fd, offsetInFat, lenInFat, fileLen, context) == 0) 
			return;
	}
	// if there is a problem, fall into case where we map file somewhere outside the shared region
#endif

#if SPLIT_SEG_DYLIB_SUPPORT
	// support old split-seg dylibs by mapping them where ever we find space
	if ( mapSplitSegDylibOutsideSharedRegion(fd, offsetInFat, lenInFat, fileLen, context) != 0 ) 
#endif
		throw "mapping error";
}


#if SPLIT_SEG_SHARED_REGION_SUPPORT	
static int _shared_region_map_np(int fd, uint32_t count, const shared_file_mapping_np mappings[])
{
	return syscall(295, fd, count, mappings);
}

int
ImageLoaderMachOClassic::mapSplitSegDylibInfoSharedRegion(int fd,
                                         uint64_t offsetInFat,
                                         uint64_t lenInFat,
                                         uint64_t fileLen,
                                         const LinkContext& context)
{
	// build table of segments to map
	const unsigned int segmentCount = fSegmentsCount;
	const unsigned int extraZeroFillEntries = getExtraZeroFillEntriesCount();
	const unsigned int mappingTableCount = segmentCount+extraZeroFillEntries;
	shared_file_mapping_np mappingTable[mappingTableCount];
	initMappingTable(offsetInFat, mappingTable);

	// try to map it in shared
	int r = _shared_region_map_np(fd, mappingTableCount, mappingTable);
	if ( 0 == r ) {
		this->setNeverUnload();
		if ( context.verboseMapping ) {
			dyld::log("dyld: Mapping split-seg shared %s\n", this->getPath());
			for(unsigned int segIndex=0,entryIndex=0; segIndex < segmentCount; ++segIndex, ++entryIndex){
				const shared_file_mapping_np* entry = &mappingTable[entryIndex];
				if ( (entry->sfm_init_prot & VM_PROT_ZF) == 0 ) 
					dyld::log("%18s at 0x%08lX->0x%08lX\n",
							  segName(segIndex), segActualLoadAddress(segIndex), segActualEndAddress(segIndex)-1);
				if ( entryIndex < (mappingTableCount-1) ) {
					const shared_file_mapping_np* nextEntry = &mappingTable[entryIndex+1];
					if ( (nextEntry->sfm_init_prot & VM_PROT_ZF) != 0 ) {
						uint64_t segOffset = nextEntry->sfm_address - entry->sfm_address;
						dyld::log("%18s at 0x%08lX->0x%08lX\n",
								  segName(segIndex), (uintptr_t)(segActualLoadAddress(segIndex) + segOffset), 
								  (uintptr_t)(segActualLoadAddress(segIndex) + segOffset + nextEntry->sfm_size - 1));
						++entryIndex;
					}
				}
			}
		}
	}
	return r;
}

#endif // SPLIT_SEG_SHARED_REGION_SUPPORT	

// test if this image is re-exported through parent (the image that loaded this one)
bool ImageLoaderMachOClassic::isSubframeworkOf(const LinkContext& context, const ImageLoader* parent) const
{
	if ( fInUmbrella ) {
		const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
		const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
		const struct load_command* cmd = cmds;
		for (uint32_t i = 0; i < cmd_count; ++i) {
			if (cmd->cmd == LC_SUB_FRAMEWORK) {
				const struct sub_framework_command* subf = (struct sub_framework_command*)cmd;
				const char* exportThruName = (char*)cmd + subf->umbrella.offset;
				// need to match LC_SUB_FRAMEWORK string against the leaf name of the install location of parent...
				const char* parentInstallPath = parent->getInstallPath();
				if ( parentInstallPath != NULL ) {
					const char* lastSlash = strrchr(parentInstallPath, '/');
					if ( lastSlash != NULL ) {
						if ( strcmp(&lastSlash[1], exportThruName) == 0 )
							return true;
						if ( context.imageSuffix != NULL ) {
							// when DYLD_IMAGE_SUFFIX is used, lastSlash string needs imageSuffix removed from end
							for(const char* const* suffix = context.imageSuffix; *suffix != NULL; ++suffix) {
								char reexportAndSuffix[strlen(*suffix)+strlen(exportThruName)+1];
								strcpy(reexportAndSuffix, exportThruName);
								strcat(reexportAndSuffix, *suffix);
								if ( strcmp(&lastSlash[1], reexportAndSuffix) == 0 )
									return true;
							}
						}
					}
				}
			}
			cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
		}
	}
	return false;
}

// test if child is re-exported 
bool ImageLoaderMachOClassic::hasSubLibrary(const LinkContext& context, const ImageLoader* child) const
{
	if ( fHasSubLibraries ) {
		// need to match LC_SUB_LIBRARY string against the leaf name (without extension) of the install location of child...
		const char* childInstallPath = child->getInstallPath();
		if ( childInstallPath != NULL ) {
			const char* lastSlash = strrchr(childInstallPath, '/');
			if ( lastSlash != NULL ) {
				const char* firstDot = strchr(lastSlash, '.');
				size_t len;
				if ( firstDot == NULL )
					len = strlen(lastSlash);
				else
					len = firstDot-lastSlash-1;
				char childLeafName[len+1];
				strncpy(childLeafName, &lastSlash[1], len);
				childLeafName[len] = '\0';
				const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
				const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
				const struct load_command* cmd = cmds;
				for (uint32_t i = 0; i < cmd_count; ++i) {
					switch (cmd->cmd) {
						case LC_SUB_LIBRARY:
							{
								const struct sub_library_command* lib = (struct sub_library_command*)cmd;
								const char* aSubLibName = (char*)cmd + lib->sub_library.offset;
								if ( strcmp(aSubLibName, childLeafName) == 0 )
									return true;
								if ( context.imageSuffix != NULL ) {
									// when DYLD_IMAGE_SUFFIX is used, childLeafName string needs imageSuffix removed from end
									for(const char* const* suffix = context.imageSuffix; *suffix != NULL; ++suffix) {
										char aSubLibNameAndSuffix[strlen(*suffix)+strlen(aSubLibName)+1];
										strcpy(aSubLibNameAndSuffix, aSubLibName);
										strcat(aSubLibNameAndSuffix, *suffix);
										if ( strcmp(aSubLibNameAndSuffix, childLeafName) == 0 )
											return true;
									}
								}
							}
							break;
					}
					cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
				}
			}
		}
	}
	if ( fHasSubUmbrella ) {
		// need to match LC_SUB_UMBRELLA string against the leaf name of install location of child...
		const char* childInstallPath = child->getInstallPath();
		if ( childInstallPath != NULL ) {
			const char* lastSlash = strrchr(childInstallPath, '/');
			if ( lastSlash != NULL ) {
				const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
				const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
				const struct load_command* cmd = cmds;
				for (uint32_t i = 0; i < cmd_count; ++i) {
					switch (cmd->cmd) {
						case LC_SUB_UMBRELLA:
							{
								const struct sub_umbrella_command* um = (struct sub_umbrella_command*)cmd;
								const char* aSubUmbrellaName = (char*)cmd + um->sub_umbrella.offset;
								if ( strcmp(aSubUmbrellaName, &lastSlash[1]) == 0 )
									return true;
								if ( context.imageSuffix != NULL ) {
									// when DYLD_IMAGE_SUFFIX is used, lastSlash string needs imageSuffix removed from end
									for(const char* const* suffix = context.imageSuffix; *suffix != NULL; ++suffix) {
										char umbrellaAndSuffix[strlen(*suffix)+strlen(aSubUmbrellaName)+1];
										strcpy(umbrellaAndSuffix, aSubUmbrellaName);
										strcat(umbrellaAndSuffix, *suffix);
										if ( strcmp(umbrellaAndSuffix, &lastSlash[1]) == 0 )
											return true;
									}
								}
							}
							break;
					}
					cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
				}
			}
		}
	}
	return false;
}


uintptr_t ImageLoaderMachOClassic::getFirstWritableSegmentAddress()
{
	// in split segment libraries r_address is offset from first writable segment
	for(unsigned int i=0; i < fSegmentsCount; ++i) {
		if ( segWriteable(i) ) 
			return segActualLoadAddress(i);
	}
	throw "no writable segment";
}

uintptr_t ImageLoaderMachOClassic::getRelocBase()
{
	// r_address is either an offset from the first segment address
	// or from the first writable segment address
#if __x86_64__
	return getFirstWritableSegmentAddress();
#else
	if ( fIsSplitSeg )
		return getFirstWritableSegmentAddress();
	else
		return segActualLoadAddress(0);
#endif
}


#if PREBOUND_IMAGE_SUPPORT
void ImageLoaderMachOClassic::resetPreboundLazyPointers(const LinkContext& context)
{
	// loop through all local (internal) relocation records looking for pre-bound-lazy-pointer values
	const uintptr_t relocBase = this->getRelocBase();
    const uintptr_t slide = this->fSlide;
	const relocation_info* const relocsStart = (struct relocation_info*)(&fLinkEditBase[fDynamicInfo->locreloff]);
	const relocation_info* const relocsEnd = &relocsStart[fDynamicInfo->nlocrel];
	for (const relocation_info* reloc=relocsStart; reloc < relocsEnd; ++reloc) {
		if ( (reloc->r_address & R_SCATTERED) != 0 ) {
			const struct scattered_relocation_info* sreloc = (struct scattered_relocation_info*)reloc;
			if (sreloc->r_length == RELOC_SIZE) {
				uintptr_t* locationToFix = (uintptr_t*)(sreloc->r_address + relocBase);
				switch(sreloc->r_type) {
		#if __i386__
					case GENERIC_RELOC_PB_LA_PTR:
						*locationToFix = sreloc->r_value + slide;
						break;
		#endif
		#if __arm__
					case ARM_RELOC_PB_LA_PTR:
						*locationToFix = sreloc->r_value + slide;
						break;
		#endif
				}
			}
		}
	}
}
#endif




void ImageLoaderMachOClassic::rebase(const LinkContext& context, uintptr_t slide)
{
	CRSetCrashLogMessage2(this->getPath());
	const uintptr_t relocBase = this->getRelocBase();

	// loop through all local (internal) relocation records
	const relocation_info* const relocsStart = (struct relocation_info*)(&fLinkEditBase[fDynamicInfo->locreloff]);
	const relocation_info* const relocsEnd = &relocsStart[fDynamicInfo->nlocrel];
	for (const relocation_info* reloc=relocsStart; reloc < relocsEnd; ++reloc) {
		uintptr_t rebaseAddr;
		try {
	#if LINKEDIT_USAGE_DEBUG
			noteAccessedLinkEditAddress(reloc);
	#endif
		#if __x86_64__
			// only one kind of local relocation supported for x86_64
			if ( reloc->r_length != 3 ) 
				throw "bad local relocation length";
			if ( reloc->r_type != X86_64_RELOC_UNSIGNED ) 
				throw "unknown local relocation type";
			if ( reloc->r_pcrel != 0 ) 
				throw "bad local relocation pc_rel";
			if ( reloc->r_extern != 0 ) 
				throw "extern relocation found with local relocations";
			rebaseAddr = reloc->r_address + relocBase;
			if ( ! this->containsAddress((void*)rebaseAddr) )
				dyld::throwf("local reloc %p not in mapped image\n", (void*)rebaseAddr);
			*((uintptr_t*)rebaseAddr) += slide;
			if ( context.verboseRebase )
				dyld::log("dyld: rebase: %s:*0x%08lX += 0x%08lX\n", this->getShortName(), rebaseAddr, slide);
		#else	
			if ( (reloc->r_address & R_SCATTERED) == 0 ) {
				if ( reloc->r_symbolnum == R_ABS ) {
					// ignore absolute relocations
				}
				else if (reloc->r_length == RELOC_SIZE) {
					switch(reloc->r_type) {
						case GENERIC_RELOC_VANILLA:
							rebaseAddr = reloc->r_address + relocBase;
							if ( ! this->containsAddress((void*)rebaseAddr) )
								dyld::throwf("local reloc %p not in mapped image\n", (void*)rebaseAddr);
							*((uintptr_t*)rebaseAddr) += slide;
							if ( context.verboseRebase )
								dyld::log("dyld: rebase: %s:*0x%08lX += 0x%08lX\n", this->getShortName(), rebaseAddr, slide);
							break;
						default:
							throw "unknown local relocation type";
					}
				}
				else {
					throw "bad local relocation length";
				}
			}
			else {
				const struct scattered_relocation_info* sreloc = (struct scattered_relocation_info*)reloc;
				if (sreloc->r_length == RELOC_SIZE) {
					uintptr_t* locationToFix = (uintptr_t*)(sreloc->r_address + relocBase);
					switch(sreloc->r_type) {
						case GENERIC_RELOC_VANILLA:
							if ( ! this->containsAddress((void*)locationToFix) ) 
								dyld::throwf("local scattered reloc %p not in mapped image\n", locationToFix);
							*locationToFix += slide;
							if ( context.verboseRebase )
								dyld::log("dyld: rebase: %s:*0x%08lX += 0x%08lX\n", this->getShortName(), (uintptr_t)locationToFix, slide);
							break;
			#if __i386__
						case GENERIC_RELOC_PB_LA_PTR:
							// do nothing
							break;
			#elif __arm__
						case ARM_RELOC_PB_LA_PTR:
							// do nothing
							break;
			#endif
						default:
							throw "unknown local scattered relocation type";
					}
				}
				else {
					throw "bad local scattered relocation length";
				}
			}
		#endif // x86_64
		}
		catch (const char* msg) {
			const uint8_t* r = (uint8_t*)reloc;
			dyld::throwf("%s in %s. reloc record at %p: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
				msg, this->getPath(), reloc, r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7]);
		}
	}
	
	// update stats
	fgTotalRebaseFixups += fDynamicInfo->nlocrel;
	CRSetCrashLogMessage2(NULL);
}



const struct macho_nlist* ImageLoaderMachOClassic::binarySearchWithToc(const char* key, const char stringPool[], const struct macho_nlist symbols[], 
												const struct dylib_table_of_contents toc[], uint32_t symbolCount, uint32_t hintIndex) const
{
	int32_t high = symbolCount-1;
	int32_t mid = hintIndex;
	
	// handle out of range hint
	if ( mid >= (int32_t)symbolCount )
		mid = symbolCount/2;
	++ImageLoaderMachO::fgSymbolTableBinarySearchs;
	++fgTotalBindImageSearches;	

	//dyld::log("dyld: binarySearchWithToc for %s in %s\n", key, this->getShortName());

	for (int32_t low = 0; low <= high; mid = (low+high)/2) {
		const uint32_t index = toc[mid].symbol_index;
		const struct macho_nlist* pivot = &symbols[index];
		const char* pivotStr = &stringPool[pivot->n_un.n_strx];
#if LINKEDIT_USAGE_DEBUG
		noteAccessedLinkEditAddress(&toc[mid]);
		noteAccessedLinkEditAddress(pivot);
		noteAccessedLinkEditAddress(pivotStr);
#endif
		int cmp = strcmp(key, pivotStr);
		if ( cmp == 0 )
			return pivot;
		if ( cmp > 0 ) {
			// key > pivot 
			low = mid + 1;
		}
		else {
			// key < pivot 
			high = mid - 1;
		}
	}
	return NULL;
}

const struct macho_nlist* ImageLoaderMachOClassic::binarySearch(const char* key, const char stringPool[], const struct macho_nlist symbols[], uint32_t symbolCount) const
{
	// update stats
	++fgTotalBindImageSearches;	
	++ImageLoaderMachO::fgSymbolTableBinarySearchs;
	
	//dyld::log("dyld: binarySearch for %s in %s, stringpool=%p, symbols=%p, symbolCount=%u\n", 
	//				key, this->getShortName(), stringPool, symbols, symbolCount);

	const struct macho_nlist* base = symbols;
	for (uint32_t n = symbolCount; n > 0; n /= 2) {
		const struct macho_nlist* pivot = &base[n/2];
		const char* pivotStr = &stringPool[pivot->n_un.n_strx];
#if LINKEDIT_USAGE_DEBUG
		noteAccessedLinkEditAddress(pivot);
		noteAccessedLinkEditAddress(pivotStr);
#endif
		int cmp = strcmp(key, pivotStr);
		if ( cmp == 0 )
			return pivot;
		if ( cmp > 0 ) {
			// key > pivot 
			// move base to symbol after pivot
			base = &pivot[1];
			--n; 
		}
		else {
			// key < pivot 
			// keep same base
		}
	}
	return NULL;
}


const ImageLoader::Symbol* ImageLoaderMachOClassic::findShallowExportedSymbol(const char* name, const ImageLoader** foundIn) const
{
	const struct macho_nlist* sym = NULL;
	if ( fDynamicInfo->tocoff == 0 )
		sym = binarySearch(name, fStrings, &fSymbolTable[fDynamicInfo->iextdefsym], fDynamicInfo->nextdefsym);
	else 
		sym = binarySearchWithToc(name, fStrings, fSymbolTable, (dylib_table_of_contents*)&fLinkEditBase[fDynamicInfo->tocoff], 
										fDynamicInfo->ntoc, fDynamicInfo->nextdefsym);
	if ( sym != NULL ) {
		if ( foundIn != NULL )
			*foundIn = (ImageLoader*)this;		
		return (const Symbol*)sym;
	}
	return NULL;
}



bool ImageLoaderMachOClassic::containsSymbol(const void* addr) const
{
	return ( (fSymbolTable <= addr) && (addr < fStrings) );
}


uintptr_t ImageLoaderMachOClassic::exportedSymbolAddress(const LinkContext& context, const Symbol* symbol, const ImageLoader* requestor, bool runResolver) const
{
	const struct macho_nlist* sym = (macho_nlist*)symbol;
	uintptr_t result = sym->n_value + fSlide;
	#if __arm__
		// processor assumes code address with low bit set is thumb
		if (sym->n_desc & N_ARM_THUMB_DEF)
			result |= 1;
	#endif
	return result;
}

bool ImageLoaderMachOClassic::exportedSymbolIsWeakDefintion(const Symbol* symbol) const
{
	const struct macho_nlist* nlistSym = (const struct macho_nlist*)symbol;
	return ( (nlistSym->n_desc & N_WEAK_DEF) != 0 );
}

const char* ImageLoaderMachOClassic::exportedSymbolName(const Symbol* symbol) const
{
	const struct macho_nlist* nlistSym = (const struct macho_nlist*)symbol;
	return &fStrings[nlistSym->n_un.n_strx];
}

unsigned int ImageLoaderMachOClassic::exportedSymbolCount() const
{
	return fDynamicInfo->nextdefsym;
}

const ImageLoader::Symbol* ImageLoaderMachOClassic::exportedSymbolIndexed(unsigned int index) const
{
	if ( index < fDynamicInfo->nextdefsym ) {
		const struct macho_nlist* sym = &fSymbolTable[fDynamicInfo->iextdefsym + index];
		return (const ImageLoader::Symbol*)sym;
	}
	return NULL;
}

unsigned int ImageLoaderMachOClassic::importedSymbolCount() const
{
	return fDynamicInfo->nundefsym;
}

const ImageLoader::Symbol* ImageLoaderMachOClassic::importedSymbolIndexed(unsigned int index) const
{
	if ( index < fDynamicInfo->nundefsym ) {
		const struct macho_nlist* sym = &fSymbolTable[fDynamicInfo->iundefsym + index];
		return (const ImageLoader::Symbol*)sym;
	}
	return NULL;
}	

const char* ImageLoaderMachOClassic::importedSymbolName(const Symbol* symbol) const
{
	const struct macho_nlist* nlistSym = (const struct macho_nlist*)symbol;
	return &fStrings[nlistSym->n_un.n_strx];
}



bool ImageLoaderMachOClassic::symbolIsWeakDefinition(const struct macho_nlist* symbol)
{
	// if a define and weak ==> coalesced 
	if ( ((symbol->n_type & N_TYPE) == N_SECT) && ((symbol->n_desc & N_WEAK_DEF) != 0) ) 
		return true;
	
	// regular symbol
	return false;
}

bool ImageLoaderMachOClassic::symbolIsWeakReference(const struct macho_nlist* symbol)
{
	// if an undefine and not referencing a weak symbol ==> coalesced
	if ( ((symbol->n_type & N_TYPE) != N_SECT) && ((symbol->n_desc & N_REF_TO_WEAK) != 0) )
		return true;
	
	// regular symbol
	return false;
}

uintptr_t ImageLoaderMachOClassic::getSymbolAddress(const macho_nlist* sym, const LinkContext& context, bool runResolver) const
{
	return ImageLoaderMachO::getSymbolAddress((Symbol*)sym, this, context, runResolver);
}

uintptr_t ImageLoaderMachOClassic::resolveUndefined(const LinkContext& context, const struct macho_nlist* undefinedSymbol, 
										bool twoLevel, bool dontCoalesce, bool runResolver, const ImageLoader** foundIn)
{
	++fgTotalBindSymbolsResolved;
	const char* symbolName = &fStrings[undefinedSymbol->n_un.n_strx];

#if LINKEDIT_USAGE_DEBUG
	noteAccessedLinkEditAddress(undefinedSymbol);
	noteAccessedLinkEditAddress(symbolName);
#endif
	if ( context.bindFlat || !twoLevel ) {
		// flat lookup
		if ( ((undefinedSymbol->n_type & N_PEXT) != 0) && ((undefinedSymbol->n_type & N_TYPE) == N_SECT) ) {
			// is a multi-module private_extern internal reference that the linker did not optimize away
			uintptr_t addr = this->getSymbolAddress(undefinedSymbol, context, false);
			*foundIn = this;
			return addr;
		}
		const Symbol* sym;
		if ( context.flatExportFinder(symbolName, &sym, foundIn) ) {
			if ( *foundIn != this )
				context.addDynamicReference(this, const_cast<ImageLoader*>(*foundIn));
			return (*foundIn)->getExportedSymbolAddress(sym, context, this);
		}
		// if a bundle is loaded privately the above will not find its exports
		if ( this->isBundle() && this->hasHiddenExports() ) {
			// look in self for needed symbol
			sym = this->findShallowExportedSymbol(symbolName, foundIn);
			if ( sym != NULL )
				return (*foundIn)->getExportedSymbolAddress(sym, context, this);
		}
		if ( (undefinedSymbol->n_desc & N_WEAK_REF) != 0 ) {
			// definition can't be found anywhere
			// if reference is weak_import, then it is ok, just return 0
			return 0;
		}
		throwSymbolNotFound(context, symbolName, this->getPath(), "", "flat namespace");
	}
	else {
		// symbol requires searching images with coalesced symbols (not done during prebinding)
		if ( !context.prebinding && !dontCoalesce && (symbolIsWeakReference(undefinedSymbol) || symbolIsWeakDefinition(undefinedSymbol)) ) {
			const Symbol* sym;
			if ( context.coalescedExportFinder(symbolName, &sym, foundIn, nullptr) ) {
				if ( *foundIn != this )
					context.addDynamicReference(this, const_cast<ImageLoader*>(*foundIn));
				return (*foundIn)->getExportedSymbolAddress(sym, context, this);
			}
			//throwSymbolNotFound(context, symbolName, this->getPath(), "coalesced namespace");
			//dyld::log("dyld: coalesced symbol %s not found in any coalesced image, falling back to two-level lookup", symbolName);
		}
		
		// if this is a real definition (not an undefined symbol) there is no ordinal
		if ( (undefinedSymbol->n_type & N_TYPE) == N_SECT ) {
			// static linker should never generate this case, but if it does, do something sane
			uintptr_t addr = this->getSymbolAddress(undefinedSymbol, context, false);
			*foundIn = this;
			return addr;
		}

		// two level lookup
		ImageLoader* target = NULL;
		uint8_t ord = GET_LIBRARY_ORDINAL(undefinedSymbol->n_desc);
		if ( ord == EXECUTABLE_ORDINAL ) {
			target = context.mainExecutable;
		}
		else if ( ord == SELF_LIBRARY_ORDINAL ) {
			target = this;
		}
		else if ( ord == DYNAMIC_LOOKUP_ORDINAL ) {
			// rnielsen: HACKHACK
			// flat lookup
			const Symbol* sym;
			if ( context.flatExportFinder(symbolName, &sym, foundIn) )
				return (*foundIn)->getExportedSymbolAddress(sym, context, this);
			// no image has exports this symbol
			// report error
			context.undefinedHandler(symbolName);
			// try looking again
			if ( context.flatExportFinder(symbolName, &sym, foundIn) )
				return (*foundIn)->getExportedSymbolAddress(sym, context, this);
			
			throwSymbolNotFound(context, symbolName, this->getPath(), "", "dynamic lookup");
		}
		else if ( ord <= libraryCount() ) {
			target = libImage(ord-1);
			if ( target == NULL ) {
				// if target library not loaded and reference is weak or library is weak return 0
				return 0;
			}
		}
		else {
			dyld::throwf("bad mach-o binary, library ordinal (%u) too big (max %u) for symbol %s in %s",
				ord, libraryCount(), symbolName, this->getPath());
		}
		
		if ( target == NULL ) {
			//dyld::log("resolveUndefined(%s) in %s\n", symbolName, this->getPath());
			throw "symbol not found";
		}

		uintptr_t address;
		if ( target->findExportedSymbolAddress(context, symbolName, this, ord, runResolver, foundIn, &address) )
			return address;

		if ( (undefinedSymbol->n_type & N_PEXT) != 0 ) {
			// don't know why the static linker did not eliminate the internal reference to a private extern definition
			*foundIn = this;
			return this->getSymbolAddress(undefinedSymbol, context, false);
		}
		else if ( (undefinedSymbol->n_desc & N_WEAK_REF) != 0 ) {
			// if definition not found and reference is weak return 0
			return 0;
		}
		
		// nowhere to be found
		throwSymbolNotFound(context, symbolName, this->getPath(), "", target->getPath());
	}
}



// returns if 'addr' is within the address range of section 'sectionIndex'
// fSlide is not used.  'addr' is assumed to be a prebound address in this image 
bool ImageLoaderMachOClassic::isAddrInSection(uintptr_t addr, uint8_t sectionIndex)
{
	uint8_t currentSectionIndex = 1;
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		if ( cmd->cmd == LC_SEGMENT_COMMAND ) {
			const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
			if ( (currentSectionIndex <= sectionIndex) && (sectionIndex < currentSectionIndex+seg->nsects) ) {
				// 'sectionIndex' is in this segment, get section info
				const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
				const struct macho_section* const section = &sectionsStart[sectionIndex-currentSectionIndex];
				return ( (section->addr <= addr) && (addr < section->addr+section->size) );
			}
			else {
				// 'sectionIndex' not in this segment, skip to next segment
				currentSectionIndex += seg->nsects;
			}
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	
	return false;
}

void ImageLoaderMachOClassic::doBindExternalRelocations(const LinkContext& context)
{
	const uintptr_t relocBase = this->getRelocBase();
	const bool twoLevel = this->usesTwoLevelNameSpace();
	const bool prebound = this->isPrebindable();
	
#if TEXT_RELOC_SUPPORT
	// if there are __TEXT fixups, temporarily make __TEXT writable
	if ( fTextSegmentBinds ) 
		this->makeTextSegmentWritable(context, true);
#endif
	// cache last lookup
	const struct macho_nlist*	lastUndefinedSymbol = NULL;
	uintptr_t					symbolAddr = 0;
	const ImageLoader*			image = NULL;
	
	// loop through all external relocation records and bind each
	const relocation_info* const relocsStart = (struct relocation_info*)(&fLinkEditBase[fDynamicInfo->extreloff]);
	const relocation_info* const relocsEnd = &relocsStart[fDynamicInfo->nextrel];
	for (const relocation_info* reloc=relocsStart; reloc < relocsEnd; ++reloc) {
		if (reloc->r_length == RELOC_SIZE) {
			switch(reloc->r_type) {
				case POINTER_RELOC:
					{
						const struct macho_nlist* undefinedSymbol = &fSymbolTable[reloc->r_symbolnum];
						uintptr_t* location = ((uintptr_t*)(reloc->r_address + relocBase));
						if ( ! this->containsAddress((void*)location) )
							dyld::throwf("external reloc %p not in mapped image %s\n", (void*)location, this->getPath());
						uintptr_t value = *location;
						bool symbolAddrCached = true;
					#if __i386__
						if ( reloc->r_pcrel ) {
							value += (uintptr_t)location + 4 - fSlide;
						}
					#endif
						if ( prebound ) {
							// we are doing relocations, so prebinding was not usable
							// in a prebound executable, the n_value field of an undefined symbol is set to the address where the symbol was found when prebound
							// so, subtracting that gives the initial displacement which we need to add to the newly found symbol address
							// if mach-o relocation structs had an "addend" field this complication would not be necessary.
							if ( ((undefinedSymbol->n_type & N_TYPE) == N_SECT) && ((undefinedSymbol->n_desc & N_WEAK_DEF) != 0) ) {
								// weak symbols need special casing, since *location may have been prebound to a definition in another image.
								// If *location is currently prebound to somewhere in the same section as the weak definition, we assume 
								// that we can subtract off the weak symbol address to get the addend.
								// If prebound elsewhere, we've lost the addend and have to assume it is zero.
								// The prebinding to elsewhere only happens with 10.4+ update_prebinding which only operates on a small set of Apple dylibs
								if ( (value == undefinedSymbol->n_value) || this->isAddrInSection(value, undefinedSymbol->n_sect) ) {
									value -= undefinedSymbol->n_value;
                        #if __arm__
                                    // if weak and thumb subtract off extra thumb bit
                                    if ( (undefinedSymbol->n_desc & N_ARM_THUMB_DEF) != 0 )
                                        value -= 1;
                        #endif
								}
								else
									value = 0;
							} 
					#if __arm__
							else if ( ((undefinedSymbol->n_type & N_TYPE) == N_SECT) && ((undefinedSymbol->n_desc & N_ARM_THUMB_DEF) != 0) ) {
								// it was prebound to a defined symbol for thumb code in the same linkage unit
								// we need to subtract off one to get real addend
								value -= (undefinedSymbol->n_value+1);
							}
					#endif
							else {
								// is undefined or non-weak symbol, so do subtraction to get addend
								value -= undefinedSymbol->n_value;
							}
						}
						// if undefinedSymbol is same as last time, then symbolAddr and image will resolve to the same too
						if ( undefinedSymbol != lastUndefinedSymbol ) {
							bool dontCoalesce = true;
							if ( symbolIsWeakReference(undefinedSymbol) ) { 
								// when weakbind() is run on a classic mach-o encoding, it won't try
								// to coalesce N_REF_TO_WEAK symbols because they are not in the sorted
								// range of global symbols.  To handle that case we do the coalesing now.
								dontCoalesce = false;
							}
							symbolAddr = this->resolveUndefined(context, undefinedSymbol, twoLevel, dontCoalesce, false, &image);
							lastUndefinedSymbol = undefinedSymbol;
							symbolAddrCached = false;
						}
						if ( context.verboseBind && (undefinedSymbol != NULL) ) {
							const char *path = NULL;
							if ( image != NULL ) {
								path = image->getShortName();
							}
							const char* cachedString = "(cached)";
							if ( !symbolAddrCached ) 
								cachedString = "";
							if ( value == 0 ) {
								dyld::log("dyld: bind: %s:0x%08lX = %s:%s, *0x%08lX = 0x%08lX%s\n",
										this->getShortName(), (uintptr_t)location,
										path, &fStrings[undefinedSymbol->n_un.n_strx], (uintptr_t)location, symbolAddr, cachedString);
							}
							else {
								dyld::log("dyld: bind: %s:0x%08lX = %s:%s, *0x%08lX = 0x%08lX%s + %ld\n",
										this->getShortName(), (uintptr_t)location,
										path, &fStrings[undefinedSymbol->n_un.n_strx], (uintptr_t)location, symbolAddr, cachedString, value);
							}
						}
						value += symbolAddr;
					#if __i386__
						if ( reloc->r_pcrel ) {
							*location = value - ((uintptr_t)location + 4);
						}
						else {
							// don't dirty page if prebound value was correct
							if ( !prebound || (*location != value) )
								*location = value; 
						}
					#else
						// don't dirty page if prebound value was correct
						if ( !prebound || (*location != value) )
							*location = value; 
					#endif
						// update stats
						++fgTotalBindFixups;
					}
					break;
				default:
					throw "unknown external relocation type";
			}
		}
		else {
			throw "bad external relocation length";
		}
	}
	
#if TEXT_RELOC_SUPPORT
	// if there were __TEXT fixups, restore write protection
	if ( fTextSegmentBinds ) {
		this->makeTextSegmentWritable(context, true);
	}
#endif
}



uintptr_t ImageLoaderMachOClassic::bindIndirectSymbol(uintptr_t* ptrToBind, const struct macho_section* sect, const char* symbolName, uintptr_t targetAddr, const ImageLoader* targetImage, const LinkContext& context)
{
	if ( context.verboseBind ) {
		const char* path = NULL;
		if ( targetImage != NULL )
			path = targetImage->getShortName();
		dyld::log("dyld: bind indirect sym: %s:%s$%s = %s:%s, *0x%08lx = 0x%08lx\n",
				this->getShortName(), symbolName, (((sect->flags & SECTION_TYPE)==S_NON_LAZY_SYMBOL_POINTERS) ? "non_lazy_ptr" : "lazy_ptr"),
				((path != NULL) ? path : "<weak_import-not-found>"), symbolName, (uintptr_t)ptrToBind, targetAddr);
	}
	if ( context.bindingHandler != NULL ) {
		const char* path = NULL;
		if ( targetImage != NULL )
			path = targetImage->getShortName();
		targetAddr = (uintptr_t)context.bindingHandler(path, symbolName, (void *)targetAddr);
	}
#if __i386__
	// i386 has special self-modifying stubs that change from "CALL rel32" to "JMP rel32"
	if ( ((sect->flags & SECTION_TYPE) == S_SYMBOL_STUBS) && ((sect->flags & S_ATTR_SELF_MODIFYING_CODE) != 0) && (sect->reserved2 == 5) ) {
		uint32_t rel32 = targetAddr - (((uint32_t)ptrToBind)+5);
		// re-write instruction in a thread-safe manner
		// use 8-byte compare-and-swap to alter 5-byte jump table entries
		// loop is required in case the extra three bytes that cover the next entry are altered by another thread
		bool done = false;
		while ( !done ) {
			volatile int64_t* jumpPtr = (int64_t*)ptrToBind;
			int pad = 0;
			// By default the three extra bytes swapped follow the 5-byte JMP.
			// But, if the 5-byte jump is up against the end of the __IMPORT segment
			// We don't want to access bytes off the end of the segment, so we shift
			// the extra bytes to precede the 5-byte JMP.
			if ( (((uint32_t)ptrToBind + 8) & 0x00000FFC) == 0x00000000 ) {
				jumpPtr = (int64_t*)((uint32_t)ptrToBind - 3);
				pad = 3;
			}
			int64_t oldEntry = *jumpPtr;
			union {
				int64_t int64;
				uint8_t bytes[8];
			} newEntry;
			newEntry.int64 = oldEntry;
			newEntry.bytes[pad+0] = 0xE9; // JMP rel32
			newEntry.bytes[pad+1] = rel32 & 0xFF;
			newEntry.bytes[pad+2] = (rel32 >> 8) & 0xFF;
			newEntry.bytes[pad+3] = (rel32 >> 16) & 0xFF;
			newEntry.bytes[pad+4] = (rel32 >> 24) & 0xFF;
			done = OSAtomicCompareAndSwap64Barrier(oldEntry, newEntry.int64, (int64_t*)jumpPtr);
		}
	}
	else
#endif
	*ptrToBind = targetAddr;
	return targetAddr;
}

uintptr_t ImageLoaderMachOClassic::doBindFastLazySymbol(uint32_t lazyBindingInfoOffset, const LinkContext& context, void (*lock)(), void (*unlock)())
{
	throw "compressed LINKEDIT lazy binder called with classic LINKEDIT";
}

uintptr_t ImageLoaderMachOClassic::doBindLazySymbol(uintptr_t* lazyPointer, const LinkContext& context)
{
	// scan for all lazy-pointer sections
	const bool twoLevel = this->usesTwoLevelNameSpace();
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	const uint32_t* const indirectTable = (uint32_t*)&fLinkEditBase[fDynamicInfo->indirectsymoff];
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
					#if __i386__
						else if ( (type == S_SYMBOL_STUBS) && (sect->flags & S_ATTR_SELF_MODIFYING_CODE) && (sect->reserved2 == 5) ) {
							// 5 bytes stubs on i386 are new "fast stubs"
							uint8_t* const jmpTableBase = (uint8_t*)(sect->addr + fSlide);
							uint8_t* const jmpTableEnd = jmpTableBase + sect->size;
							// initial CALL instruction in jump table leaves pointer to next entry, so back up
							uint8_t* const jmpTableEntryToPatch = ((uint8_t*)lazyPointer) - 5;  
							lazyPointer = (uintptr_t*)jmpTableEntryToPatch; 
							if ( (jmpTableEntryToPatch >= jmpTableBase) && (jmpTableEntryToPatch < jmpTableEnd) ) {
								const uint32_t indirectTableOffset = sect->reserved1;
								const uint32_t entryIndex = (jmpTableEntryToPatch - jmpTableBase)/5;
								symbolIndex = indirectTable[indirectTableOffset + entryIndex];
							}
						}
					#endif
						if ( symbolIndex != INDIRECT_SYMBOL_ABS && symbolIndex != INDIRECT_SYMBOL_LOCAL ) {
							const char* symbolName = &fStrings[fSymbolTable[symbolIndex].n_un.n_strx];
							const ImageLoader* image = NULL;
							uintptr_t symbolAddr = this->resolveUndefined(context, &fSymbolTable[symbolIndex], twoLevel, false, true, &image);
							symbolAddr = this->bindIndirectSymbol(lazyPointer, sect, symbolName, symbolAddr, image,  context);
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



void ImageLoaderMachOClassic::initializeCoalIterator(CoalIterator& it, unsigned int loadOrder, unsigned)
{
	it.image = this;
	it.symbolName = " ";
	it.loadOrder = loadOrder;
	it.weakSymbol = false;
	it.symbolMatches = false;
	it.done = false;
	it.type = 0; 
	if ( fDynamicInfo->tocoff != 0 ) {
		it.curIndex = 0;
		it.endIndex = fDynamicInfo->ntoc;
	}
	else {
		it.curIndex = 0;
		it.endIndex = fDynamicInfo->nextdefsym;
	}
}


bool ImageLoaderMachOClassic::incrementCoalIterator(CoalIterator& it)
{
	if ( it.done )
		return false;
		
	if ( fDynamicInfo->tocoff != 0 ) {
		if ( it.curIndex >= fDynamicInfo->ntoc ) {
			it.done = true;
			it.symbolName = "~~~";
			return true;
		}
		else {
			const dylib_table_of_contents* toc = (dylib_table_of_contents*)&fLinkEditBase[fDynamicInfo->tocoff];
			const uint32_t index = toc[it.curIndex].symbol_index;
			const struct macho_nlist* sym = &fSymbolTable[index];
			const char* symStr = &fStrings[sym->n_un.n_strx];
			it.symbolName = symStr;
			it.weakSymbol = (sym->n_desc & N_WEAK_DEF);
			it.symbolMatches = false;
			it.type = 0; // clear flag that says we applied updates for this symbol
			//dyld::log("incrementCoalIterator() curIndex=%ld, symbolName=%s in %s\n", it.curIndex, symStr, this->getPath());
			it.curIndex++;
			return false;
		}
	}
	else {
		if ( it.curIndex >= fDynamicInfo->nextdefsym ) {
			it.done = true;
			it.symbolName = "~~~";
			return true;
		}
		else {
			const struct macho_nlist* sym = &fSymbolTable[fDynamicInfo->iextdefsym+it.curIndex];
			const char* symStr = &fStrings[sym->n_un.n_strx];
			it.symbolName = symStr;
			it.weakSymbol = (sym->n_desc & N_WEAK_DEF);
			it.symbolMatches = false;
			it.type = 0; // clear flag that says we applied updates for this symbol
			//dyld::log("incrementCoalIterator() curIndex=%ld, symbolName=%s in %s\n", it.curIndex, symStr, this->getPath());
			it.curIndex++;
			return false;
		}
	}	
	
	return false;
}

uintptr_t ImageLoaderMachOClassic::getAddressCoalIterator(CoalIterator& it, const LinkContext& context)
{
	uint32_t symbol_index = 0;
	if ( fDynamicInfo->tocoff != 0 ) {
		const dylib_table_of_contents* toc = (dylib_table_of_contents*)&fLinkEditBase[fDynamicInfo->tocoff];
		symbol_index = toc[it.curIndex-1].symbol_index;
	}
	else {
		symbol_index = fDynamicInfo->iextdefsym + (uint32_t)it.curIndex - 1;
	}	
	const struct macho_nlist* sym = &fSymbolTable[symbol_index];
	//dyld::log("getAddressCoalIterator() => 0x%llX, %s symbol_index=%d, in %s\n", (uint64_t)(sym->n_value + fSlide), &fStrings[sym->n_un.n_strx], symbol_index, this->getPath());
#if __arm__
		// processor assumes code address with low bit set is thumb
		if (sym->n_desc & N_ARM_THUMB_DEF)
			return (sym->n_value | 1) + fSlide ;
		else
			return sym->n_value + fSlide;
#else
	return sym->n_value + fSlide;
#endif
}


void ImageLoaderMachOClassic::updateUsesCoalIterator(CoalIterator& it, uintptr_t value, ImageLoader* targetImage, unsigned targetIndex, const LinkContext& context)
{
	// flat_namespace images with classic LINKEDIT do not need late coalescing.
	// They still need to be iterated becuase they may implement
	// something needed by other coalescing images.
	// But they need no updating because during the bind phase every symbol lookup is a full scan.
	if ( !this->usesTwoLevelNameSpace() )
		return;
		
	// <rdar://problem/6570879> weak binding done too early with inserted libraries
	if ( this->getState() < dyld_image_state_bound  )
		return;

	uint32_t symbol_index = 0;
	if ( fDynamicInfo->tocoff != 0 ) {
		const dylib_table_of_contents* toc = (dylib_table_of_contents*)&fLinkEditBase[fDynamicInfo->tocoff];
		symbol_index = toc[it.curIndex-1].symbol_index;
	}
	else {
		symbol_index = fDynamicInfo->iextdefsym + (uint32_t)it.curIndex - 1;
	}	

	// if this image's copy of the symbol is not a weak definition nor a weak reference then nothing to coalesce here
	if ( !symbolIsWeakReference(&fSymbolTable[symbol_index]) && !symbolIsWeakDefinition(&fSymbolTable[symbol_index]) ) {
		return;
	}

	// <rdar://problem/6555720> malformed dylib with duplicate weak symbols causes re-binding
	if ( it.type ) 
		return;

	bool boundSomething = false;
	// scan external relocations for uses of symbol_index
	const uintptr_t relocBase = this->getRelocBase();
	const bool prebound = this->isPrebindable();
	const relocation_info* const relocsStart = (struct relocation_info*)(&fLinkEditBase[fDynamicInfo->extreloff]);
	const relocation_info* const relocsEnd = &relocsStart[fDynamicInfo->nextrel];
	for (const relocation_info* reloc=relocsStart; reloc < relocsEnd; ++reloc) {
		if ( reloc->r_symbolnum == symbol_index ) {
			//dyld::log("found external reloc using symbol_index=%d in %s\n",symbol_index, this->getPath());
			const struct macho_nlist* undefinedSymbol = &fSymbolTable[reloc->r_symbolnum];
			const char* symbolName = &fStrings[undefinedSymbol->n_un.n_strx];
			uintptr_t* location = ((uintptr_t*)(reloc->r_address + relocBase));
			const uintptr_t initialValue = *location;
			uintptr_t addend = 0;
			if ( prebound ) {
				// we are doing relocations, so prebinding was not usable
				// in a prebound executable, the n_value field of an undefined symbol is set to the address where the symbol was found when prebound
				// so, subtracting that gives the initial displacement which we need to add to the newly found symbol address
				// if mach-o relocation structs had an "addend" field this complication would not be necessary.
				if ( ((undefinedSymbol->n_type & N_TYPE) == N_SECT) && ((undefinedSymbol->n_desc & N_WEAK_DEF) != 0) ) {
					// weak symbols need special casing, since *location may have been prebound to a definition in another image.
					// If *location is currently prebound to somewhere in the same section as the weak definition, we assume 
					// that we can subtract off the weak symbol address to get the addend.
					// If prebound elsewhere, we've lost the addend and have to assume it is zero.
					// The prebinding to elsewhere only happens with 10.4+ update_prebinding which only operates on a small set of Apple dylibs
					if ( (initialValue == undefinedSymbol->n_value) || this->isAddrInSection(initialValue, undefinedSymbol->n_sect) ) {
						addend = initialValue - undefinedSymbol->n_value;
			#if __arm__
						// if weak and thumb subtract off extra thumb bit
						if ( (undefinedSymbol->n_desc & N_ARM_THUMB_DEF) != 0 )
							addend &= -2;
			#endif
					}
				} 
		#if __arm__
				else if ( ((undefinedSymbol->n_type & N_TYPE) == N_SECT) && ((undefinedSymbol->n_desc & N_ARM_THUMB_DEF) != 0) ) {
					// it was prebound to a defined symbol for thumb code in the same linkage unit
					// we need to subtract off one to get real addend
					addend = initialValue - (undefinedSymbol->n_value+1);
				}
		#endif
				else {
					// is undefined or non-weak symbol, so do subtraction to get addend
					addend = initialValue - undefinedSymbol->n_value;
				}
			}
			else {
				// non-prebound case
				if ( ((undefinedSymbol->n_type & N_TYPE) == N_SECT) && ((undefinedSymbol->n_desc & N_WEAK_DEF) != 0) ) {
					// if target is weak-def in same linkage unit, then bind phase has already set initialValue 
					// to be definition address plus addend
					//dyld::log("weak def, initialValue=0x%lX, undefAddr=0x%lX\n", initialValue, undefinedSymbol->n_value+fSlide);
					addend = initialValue - (undefinedSymbol->n_value + fSlide);
		#if __arm__
					// if weak and thumb subtract off extra thumb bit
					if ( (undefinedSymbol->n_desc & N_ARM_THUMB_DEF) != 0 )
						addend &= -2;
		#endif
				}
				else {
					// nothing fixed up yet, addend is just initial value
					//dyld::log("addend=0x%lX\n", initialValue);
					addend = initialValue;
				}
			}
			
			uint8_t type = BIND_TYPE_POINTER;
		#if __i386__
			if ( reloc->r_pcrel ) 
				type = BIND_TYPE_TEXT_PCREL32;
		#endif
			this->bindLocation(context, this->imageBaseAddress(), (uintptr_t)location, value, type, symbolName, addend, this->getPath(), targetImage ? targetImage->getPath() : NULL, "weak ", NULL, fSlide);
			boundSomething = true;
		}
	}

	// scan lazy and non-lazy pointers for uses of symbol_index
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	const uint32_t* const indirectTable = (uint32_t*)&fLinkEditBase[fDynamicInfo->indirectsymoff];
	for (uint32_t i = 0; i < cmd_count; ++i) {
		if ( cmd->cmd == LC_SEGMENT_COMMAND ) {
			const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
			const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
			const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
			for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
				uint32_t elementSize = sizeof(uintptr_t);
				switch ( sect->flags & SECTION_TYPE ) {
				#if __i386__
					case S_SYMBOL_STUBS:
						if ( ((sect->flags & S_ATTR_SELF_MODIFYING_CODE) ==0) || (sect->reserved2 != 5) )
							continue;
						elementSize = 5;
                        [[clang::fallthrough]];
				#endif
					case S_NON_LAZY_SYMBOL_POINTERS:
					case S_LAZY_SYMBOL_POINTERS:
					{
						size_t elementCount = sect->size / elementSize;
						const uint32_t indirectTableOffset = sect->reserved1;
						uint8_t* ptrToBind = (uint8_t*)(sect->addr + fSlide);
						//dyld::log(" scanning section %s of %s starting at %p\n", sect->sectname, this->getShortName(), ptrToBind);
						for (size_t j=0; j < elementCount; ++j, ptrToBind += elementSize) {
							if ( indirectTable[indirectTableOffset + j] == symbol_index ) {
								//dyld::log("  found symbol index match at %d/%d, ptrToBind=%p\n", j, elementCount, ptrToBind);
								// update pointer
								this->bindIndirectSymbol((uintptr_t*)ptrToBind, sect, it.symbolName, value, targetImage, context);
								boundSomething = true;
							}
						}
					}
					break;
				}
			}
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	if ( boundSomething && (targetImage != this) ) {
		context.addDynamicReference(this, targetImage);
	}
	
	// mark that this symbol has already been bound, so we don't try to bind again
	it.type = 1;
}


void ImageLoaderMachOClassic::bindIndirectSymbolPointers(const LinkContext& context, bool bindNonLazys, bool bindLazys)
{
	// scan for all non-lazy-pointer sections 
	const bool twoLevel = this->usesTwoLevelNameSpace();
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	const uint32_t* const indirectTable = (uint32_t*)&fLinkEditBase[fDynamicInfo->indirectsymoff];
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
				{
					const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
					const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
					const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
					for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
						bool isLazySymbol = false;
						const uint8_t type = sect->flags & SECTION_TYPE;
						uint32_t elementSize = sizeof(uintptr_t);
						size_t elementCount = sect->size / elementSize;
						if ( type == S_NON_LAZY_SYMBOL_POINTERS ) {
							if ( ! bindNonLazys )
								continue;
						}
						else if ( type == S_LAZY_SYMBOL_POINTERS ) {
							// process each symbol pointer in this section
							fgTotalPossibleLazyBindFixups += elementCount;
							isLazySymbol = true;
							if ( ! bindLazys )
								continue;
						}
				#if __i386__
						else if ( (type == S_SYMBOL_STUBS) && (sect->flags & S_ATTR_SELF_MODIFYING_CODE) && (sect->reserved2 == 5) ) {
							// process each jmp entry in this section
							elementCount = sect->size / 5;
							elementSize = 5;
							fgTotalPossibleLazyBindFixups += elementCount;
							isLazySymbol = true;
							if ( ! bindLazys )
								continue;
						}
				#endif
						else {
							continue;
						}
						const uint32_t indirectTableOffset = sect->reserved1;
						uint8_t* ptrToBind = (uint8_t*)(sect->addr + fSlide);
						for (size_t j=0; j < elementCount; ++j, ptrToBind += elementSize) {
				#if LINKEDIT_USAGE_DEBUG
							noteAccessedLinkEditAddress(&indirectTable[indirectTableOffset + j]);
				#endif
							uint32_t symbolIndex = indirectTable[indirectTableOffset + j];
							if ( symbolIndex == INDIRECT_SYMBOL_LOCAL) {
								*((uintptr_t*)ptrToBind) += this->fSlide;
							}
							else if ( symbolIndex == INDIRECT_SYMBOL_ABS) {
								// do nothing since already has absolute address
							}
							else {
								const struct macho_nlist* sym = &fSymbolTable[symbolIndex];
								if ( symbolIndex == 0 ) {
									// This could be rdar://problem/3534709 
									if ( ((const macho_header*)fMachOData)->filetype == MH_EXECUTE ) {
										static bool alreadyWarned = false;
										if ( (sym->n_type & N_TYPE) != N_UNDF ) {
											// The indirect table parallels the (non)lazy pointer sections.  For
											// instance, to find info about the fifth lazy pointer you look at the
											// fifth entry in the indirect table.  (try otool -Iv on a file).
											// The entry in the indirect table contains an index into the symbol table.

											// The bug in ld caused the entry in the indirect table to be zero
											// (instead of a magic value that means a local symbol).  So, if the
											// symbolIndex == 0, we may be encountering the bug, or 0 may be a valid
											// symbol table index. The check I put in place is to see if the zero'th
											// symbol table entry is an import entry (usually it is a local symbol
											// definition).
											if ( context.verboseWarnings && !alreadyWarned ) {
												dyld::log("dyld: malformed executable '%s', skipping indirect symbol to %s\n",
														this->getPath(), &fStrings[sym->n_un.n_strx]);
												alreadyWarned = true;
											}
											continue;
										}
									}
								}
								const ImageLoader* image = NULL;
								// let weak definitions resolve to themselves, later coalescing may overwrite them
								bool dontCoalesce = true;
								if ( bindLazys && isLazySymbol ) {
									// if this is something normally lazy bound, but we are forcing
									// it to be bound now, do coalescing
									dontCoalesce = false;
								}
								if ( symbolIsWeakReference(sym) ) { 
									// when weakbind() is run on a classic mach-o encoding, it won't try
									// to coalesce N_REF_TO_WEAK symbols because they are not in the sorted
									// range of global symbols.  To handle that case we do the coalesing now.
									dontCoalesce = false;
								}
								uintptr_t symbolAddr = resolveUndefined(context, sym, twoLevel, dontCoalesce, false, &image);
								// update pointer
								this->bindIndirectSymbol((uintptr_t*)ptrToBind, sect, &fStrings[sym->n_un.n_strx], symbolAddr, image,  context);
								// update stats
								++fgTotalBindFixups;
							}
						}
					}
				}
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
}



#if __i386__
void ImageLoaderMachOClassic::initializeLazyStubs(const LinkContext& context)
{
	if ( ! this->usablePrebinding(context) ) {
		// reset all "fast" stubs
		const macho_header* mh = (macho_header*)fMachOData;
		const uint32_t cmd_count = mh->ncmds;
		const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
		const struct load_command* cmd = cmds;
		for (uint32_t i = 0; i < cmd_count; ++i) {
			switch (cmd->cmd) {
				case LC_SEGMENT_COMMAND:
				{
					const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
					const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
					const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
					for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
						const uint8_t type = sect->flags & SECTION_TYPE;
						if ( (type == S_SYMBOL_STUBS) && (sect->flags & S_ATTR_SELF_MODIFYING_CODE) && (sect->reserved2 == 5) ) {
							// reset each jmp entry in this section
							const uint32_t indirectTableOffset = sect->reserved1;
							const uint32_t* const indirectTable = (uint32_t*)&fLinkEditBase[fDynamicInfo->indirectsymoff];
							uint8_t* start = (uint8_t*)(sect->addr + this->fSlide);
							uint8_t* end = start + sect->size;
							uintptr_t dyldHandler = (uintptr_t)&stub_binding_helper_i386_old;
							uint32_t entryIndex = 0;
							for (uint8_t* entry = start; entry < end; entry += 5, ++entryIndex) {
								bool installLazyHandler = true;
								// jump table entries that cross a (64-byte) cache line boundary have the potential to cause crashes
								// if the instruction is updated by one thread while being executed by another
								if ( ((uint32_t)entry & 0xFFFFFFC0) != ((uint32_t)entry+4 & 0xFFFFFFC0) ) {
									// need to bind this now to avoid a potential problem if bound lazily
									uint32_t symbolIndex = indirectTable[indirectTableOffset + entryIndex];
									// the latest linker marks 64-byte crossing stubs with INDIRECT_SYMBOL_ABS so they are not used
									if ( symbolIndex != INDIRECT_SYMBOL_ABS ) {
										const char* symbolName = &fStrings[fSymbolTable[symbolIndex].n_un.n_strx];
										const ImageLoader* image = NULL;
										try {
											uintptr_t symbolAddr = this->resolveUndefined(context, &fSymbolTable[symbolIndex], this->usesTwoLevelNameSpace(), false, false, &image);
											symbolAddr = this->bindIndirectSymbol((uintptr_t*)entry, sect, symbolName, symbolAddr, image, context);
											++fgTotalBindFixups;
											uint32_t rel32 = symbolAddr - (((uint32_t)entry)+5);
											entry[0] = 0xE9; // JMP rel32
											entry[1] = rel32 & 0xFF;
											entry[2] = (rel32 >> 8) & 0xFF;
											entry[3] = (rel32 >> 16) & 0xFF;
											entry[4] = (rel32 >> 24) & 0xFF;
											installLazyHandler = false;
										} 
										catch (const char* msg) {
											// ignore errors when binding symbols early
											// maybe the function is never called, and therefore erroring out now would be a regression
										}
									}
								}
								if ( installLazyHandler ) {
									uint32_t rel32 = dyldHandler - (((uint32_t)entry)+5);
									entry[0] = 0xE8; // CALL rel32
									entry[1] = rel32 & 0xFF;
									entry[2] = (rel32 >> 8) & 0xFF;
									entry[3] = (rel32 >> 16) & 0xFF;
									entry[4] = (rel32 >> 24) & 0xFF;
								}
							}
						}
					}
				}
			}
			cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
		}
	}
}
#endif // __i386__


void ImageLoaderMachOClassic::doBind(const LinkContext& context, bool forceLazysBound)
{
	CRSetCrashLogMessage2(this->getPath());
#if __i386__
	this->initializeLazyStubs(context);
#endif

	// if prebound and loaded at prebound address, and all libraries are same as when this was prebound, then no need to bind
	// note: flat-namespace binaries need to have imports rebound (even if correctly prebound)
	if ( this->usablePrebinding(context) ) {
		// binding already up to date
	}
	else {
		// no valid prebinding, so bind symbols.
		// values bound by name are stored two different ways in classic mach-o:
		
	#if TEXT_RELOC_SUPPORT
		// if there are __TEXT fixups, temporarily make __TEXT writable
		if ( fTextSegmentBinds ) 
			this->makeTextSegmentWritable(context, true);
	#endif

		// 1) external relocations are used for data initialized to external symbols
		this->doBindExternalRelocations(context);
		
		// 2) "indirect symbols" are used for code references to external symbols
		// if this image is in the shared cache, there is no way to reset the lazy pointers, so bind them now
		this->bindIndirectSymbolPointers(context, true, forceLazysBound || fInSharedCache);

	#if TEXT_RELOC_SUPPORT
		// if there were __TEXT fixups, restore write protection
		if ( fTextSegmentBinds ) 
			this->makeTextSegmentWritable(context, false);
	#endif	
	}
	
	// set up dyld entry points in image
	this->setupLazyPointerHandler(context);
	
	CRSetCrashLogMessage2(NULL);
}

void ImageLoaderMachOClassic::doBindJustLazies(const LinkContext& context)
{
	// some API called requested that all lazy pointers in this image be force bound
	this->bindIndirectSymbolPointers(context, false, true);
}

void ImageLoaderMachOClassic::doInterpose(const LinkContext& context)
{
	if ( context.verboseInterposing )
		dyld::log("dyld: interposing %lu tuples onto: %s\n", fgInterposingTuples.size(), this->getPath());

	// scan indirect symbols
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
				{
					const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
					const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
					const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
					for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
						const uint8_t type = sect->flags & SECTION_TYPE;
						if ( (type == S_NON_LAZY_SYMBOL_POINTERS) || (type == S_LAZY_SYMBOL_POINTERS) ) {
							const size_t pointerCount = sect->size / sizeof(uintptr_t);
							uintptr_t* const symbolPointers = (uintptr_t*)(sect->addr + fSlide);
							for (size_t pointerIndex=0; pointerIndex < pointerCount; ++pointerIndex) {
								uintptr_t newValue = interposedAddress(context, symbolPointers[pointerIndex], this);
								if ( newValue != symbolPointers[pointerIndex] )
									symbolPointers[pointerIndex] = newValue;
							}
						}
				#if __i386__
						// i386 has special self-modifying stubs that might be prebound to "JMP rel32" that need checking
						else if ( (type == S_SYMBOL_STUBS) && ((sect->flags & S_ATTR_SELF_MODIFYING_CODE) != 0) && (sect->reserved2 == 5) ) {
							// check each jmp entry in this section
							uint8_t* start = (uint8_t*)(sect->addr + this->fSlide);
							uint8_t* end = start + sect->size;
							for (uint8_t* entry = start; entry < end; entry += 5) {
								if ( entry[0] == 0xE9 ) { // 0xE9 == JMP 
									uint32_t rel32 = *((uint32_t*)&entry[1]); // assume unaligned load of uint32_t is ok
									uint32_t target = (uint32_t)&entry[5] + rel32;
									uint32_t newTarget = interposedAddress(context, target, this);
									if ( newTarget != target ) {
										uint32_t newRel32 = newTarget - (uint32_t)&entry[5];
										*((uint32_t*)&entry[1]) = newRel32; // assume unaligned store of uint32_t is ok
									}
								}
							}
						}
				#endif
					}
				}
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	
	// scan external relocations 
	const uintptr_t relocBase = this->getRelocBase();
	const relocation_info* const relocsStart = (struct relocation_info*)(&fLinkEditBase[fDynamicInfo->extreloff]);
	const relocation_info* const relocsEnd = &relocsStart[fDynamicInfo->nextrel];
	for (const relocation_info* reloc=relocsStart; reloc < relocsEnd; ++reloc) {
		if (reloc->r_length == RELOC_SIZE) {
			switch(reloc->r_type) {
				case POINTER_RELOC:
					{
						uintptr_t* location = ((uintptr_t*)(reloc->r_address + relocBase));
						uintptr_t value = *location;
						uintptr_t newValue = interposedAddress(context, value, this);
						if ( newValue != value )
							*location = newValue;
					}
					break;
			}
		}
	}
}

void ImageLoaderMachOClassic::dynamicInterpose(const LinkContext& context) 
{
	if ( context.verboseInterposing )
		dyld::log("dyld: dynamic interposing %lu tuples onto image: %s\n", context.dynamicInterposeCount, this->getPath());

	// scan indirect symbols
	const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
	const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
	const struct load_command* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
				{
					const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
					const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
					const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
					for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
						const uint8_t type = sect->flags & SECTION_TYPE;
						if ( (type == S_NON_LAZY_SYMBOL_POINTERS) || (type == S_LAZY_SYMBOL_POINTERS) ) {
							const size_t pointerCount = sect->size / sizeof(uintptr_t);
							uintptr_t* const symbolPointers = (uintptr_t*)(sect->addr + fSlide);
							for (size_t pointerIndex=0; pointerIndex < pointerCount; ++pointerIndex) {
								for(size_t j=0; j < context.dynamicInterposeCount; ++j) {
									// replace all references to 'replacee' with 'replacement'
									if ( symbolPointers[pointerIndex] == (uintptr_t)context.dynamicInterposeArray[j].replacee ) {
										if ( context.verboseInterposing ) {
											dyld::log("dyld: dynamic interposing: at %p replace %p with %p in %s\n", 
												&symbolPointers[pointerIndex], context.dynamicInterposeArray[j].replacee, context.dynamicInterposeArray[j].replacement, this->getPath());
										}
										symbolPointers[pointerIndex] = (uintptr_t)context.dynamicInterposeArray[j].replacement;
									}
								}
							}
						}
					}
				}
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	
	// scan external relocations 
	const uintptr_t relocBase = this->getRelocBase();
	const relocation_info* const relocsStart = (struct relocation_info*)(&fLinkEditBase[fDynamicInfo->extreloff]);
	const relocation_info* const relocsEnd = &relocsStart[fDynamicInfo->nextrel];
	for (const relocation_info* reloc=relocsStart; reloc < relocsEnd; ++reloc) {
		if (reloc->r_length == RELOC_SIZE) {
			switch(reloc->r_type) {
				case POINTER_RELOC:
					{
						uintptr_t* location = ((uintptr_t*)(reloc->r_address + relocBase));
						for(size_t i=0; i < context.dynamicInterposeCount; ++i) {
							// replace all references to 'replacee' with 'replacement'
							if ( *location == (uintptr_t)context.dynamicInterposeArray[i].replacee ) {
								if ( context.verboseInterposing ) {
									dyld::log("dyld: dynamic interposing: at %p replace %p with %p in %s\n", 
										location, context.dynamicInterposeArray[i].replacee, context.dynamicInterposeArray[i].replacement, this->getPath());
								}
								*location = (uintptr_t)context.dynamicInterposeArray[i].replacement;
							}
						}
					}
					break;
			}
		}
	}
}


const char* ImageLoaderMachOClassic::findClosestSymbol(const void* addr, const void** closestAddr) const
{
	uintptr_t targetAddress = (uintptr_t)addr - fSlide;
	const struct macho_nlist* bestSymbol = NULL;
	// first walk all global symbols
	const struct macho_nlist* const globalsStart = &fSymbolTable[fDynamicInfo->iextdefsym];
	const struct macho_nlist* const globalsEnd= &globalsStart[fDynamicInfo->nextdefsym];
	for (const struct macho_nlist* s = globalsStart; s < globalsEnd; ++s) {
 		if ( (s->n_type & N_TYPE) == N_SECT ) {
			if ( bestSymbol == NULL ) {
				if ( s->n_value <= targetAddress )
					bestSymbol = s;
			}
			else if ( (s->n_value <= targetAddress) && (bestSymbol->n_value < s->n_value) ) {
				bestSymbol = s;
			}
		}
	}
	// next walk all local symbols
	const struct macho_nlist* const localsStart = &fSymbolTable[fDynamicInfo->ilocalsym];
	const struct macho_nlist* const localsEnd= &localsStart[fDynamicInfo->nlocalsym];
	for (const struct macho_nlist* s = localsStart; s < localsEnd; ++s) {
 		if ( ((s->n_type & N_TYPE) == N_SECT) && ((s->n_type & N_STAB) == 0) ) {
			if ( bestSymbol == NULL ) {
				if ( s->n_value <= targetAddress )
					bestSymbol = s;
			}
			else if ( (s->n_value <= targetAddress) && (bestSymbol->n_value < s->n_value) ) {
				bestSymbol = s;
			}
		}
	}
	if ( bestSymbol != NULL ) {
#if __arm__
		if (bestSymbol->n_desc & N_ARM_THUMB_DEF)
			*closestAddr = (void*)((bestSymbol->n_value | 1) + fSlide);
		else
			*closestAddr = (void*)(bestSymbol->n_value + fSlide);
#else
		*closestAddr = (void*)(bestSymbol->n_value + fSlide);
#endif
		return &fStrings[bestSymbol->n_un.n_strx];
	}
	return NULL;
}



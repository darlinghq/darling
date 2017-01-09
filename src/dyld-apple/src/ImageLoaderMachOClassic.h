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


#ifndef __IMAGELOADER_MACHO_CLASSIC__
#define __IMAGELOADER_MACHO_CLASSIC__

#include <stdint.h> 

#include "ImageLoaderMachO.h"


//
// ImageLoaderMachOClassic is the concrete subclass of ImageLoader which loads mach-o files 
// that use the traditional LINKEDIT format.  
//
class ImageLoaderMachOClassic : public ImageLoaderMachO {
public:
	static ImageLoaderMachOClassic*		instantiateMainExecutable(const macho_header* mh, uintptr_t slide, const char* path, 
																	unsigned int segCount, unsigned int libCount, const LinkContext& context);
	static ImageLoaderMachOClassic*		instantiateFromFile(const char* path, int fd, const uint8_t* fileData, size_t lenFileData, 
															uint64_t offsetInFat, uint64_t lenInFat, const struct stat& info, 
															unsigned int segCount, unsigned int libCount, 
															const struct linkedit_data_command* codeSigCmd, const LinkContext& context);
	static ImageLoaderMachOClassic*		instantiateFromCache(const macho_header* mh, const char* path, long slide, const struct stat& info,
																unsigned int segCount, unsigned int libCount, const LinkContext& context);
	static ImageLoaderMachOClassic*		instantiateFromMemory(const char* moduleName, const macho_header* mh, uint64_t len, 
															unsigned int segCount, unsigned int libCount, const LinkContext& context);

	virtual								~ImageLoaderMachOClassic();

	virtual ImageLoader*				libImage(unsigned int) const;
	virtual bool						libReExported(unsigned int) const;
	virtual bool						libIsUpward(unsigned int) const;
	virtual void						setLibImage(unsigned int, ImageLoader*, bool, bool);
	virtual void						doBind(const LinkContext& context, bool forceLazysBound);
	virtual void						doBindJustLazies(const LinkContext& context);
	virtual uintptr_t					doBindLazySymbol(uintptr_t* lazyPointer, const LinkContext& context);
	virtual uintptr_t					doBindFastLazySymbol(uint32_t lazyBindingInfoOffset, const LinkContext& context, void (*lock)(), void (*unlock)());
	virtual const char*					findClosestSymbol(const void* addr, const void** closestAddr) const;
	virtual	void						initializeCoalIterator(CoalIterator&, unsigned int loadOrder, unsigned);
	virtual	bool						incrementCoalIterator(CoalIterator&);
	virtual	uintptr_t					getAddressCoalIterator(CoalIterator&, const LinkContext& contex);
	virtual	void						updateUsesCoalIterator(CoalIterator&, uintptr_t newAddr, ImageLoader* target, unsigned targetIndex, const LinkContext& context);

protected:
	virtual void						doInterpose(const LinkContext& context);
	virtual void						dynamicInterpose(const LinkContext& context);
	virtual void						setDyldInfo(const dyld_info_command*) {}
	virtual void						setSymbolTableInfo(const macho_nlist*, const char*, const dysymtab_command*);
	virtual	bool						isSubframeworkOf(const LinkContext& context, const ImageLoader* image) const;
	virtual	bool						hasSubLibrary(const LinkContext& context, const ImageLoader* child) const;
	virtual uint32_t*					segmentCommandOffsets() const;
	virtual	void						rebase(const LinkContext& context, uintptr_t slide);
	virtual const ImageLoader::Symbol*	findShallowExportedSymbol(const char* name, const ImageLoader** foundIn) const;
	virtual bool						containsSymbol(const void* addr) const;
	virtual uintptr_t					exportedSymbolAddress(const LinkContext& context, const Symbol* symbol, const ImageLoader* requestor, bool runResolver) const;
	virtual bool						exportedSymbolIsWeakDefintion(const Symbol* symbol) const;
	virtual const char*					exportedSymbolName(const Symbol* symbol) const;
	virtual unsigned int				exportedSymbolCount() const;
	virtual const ImageLoader::Symbol*	exportedSymbolIndexed(unsigned int) const;
	virtual unsigned int				importedSymbolCount() const;
	virtual const ImageLoader::Symbol*	importedSymbolIndexed(unsigned int) const;
	virtual const char*					importedSymbolName(const Symbol* symbol) const;
#if PREBOUND_IMAGE_SUPPORT
	virtual void						resetPreboundLazyPointers(const LinkContext& context);
#endif


private:
										ImageLoaderMachOClassic(const macho_header* mh, const char* path, unsigned int segCount, 
																		uint32_t segOffsets[], unsigned int libCount);
	static ImageLoaderMachOClassic*		instantiateStart(const macho_header* mh, const char* path, unsigned int segCount, unsigned int libCount);
	void								instantiateFinish(const LinkContext& context);
	uintptr_t							getRelocBase();
	void								mapSegmentsClassic(int fd, uint64_t offsetInFat, uint64_t lenInFat, uint64_t fileLen, const LinkContext& context);
	uintptr_t							getFirstWritableSegmentAddress();
	const struct macho_nlist*			binarySearch(const char* key, const char stringPool[], const struct macho_nlist symbols[], uint32_t symbolCount) const;
	const struct macho_nlist*			binarySearchWithToc(const char* key, const char stringPool[], const struct macho_nlist symbols[], 
															const struct dylib_table_of_contents toc[], uint32_t symbolCount, uint32_t hintIndex) const;
	static	bool						symbolIsWeakReference(const struct macho_nlist* symbol); 
	static	bool						symbolIsWeakDefinition(const struct macho_nlist* symbol); 
	uintptr_t							resolveUndefined(const LinkContext& context, const struct macho_nlist* symbol, bool twoLevel, 
															bool dontCoalesce, bool runResolver, const ImageLoader **foundIn);
	uintptr_t							getSymbolAddress(const macho_nlist*, const LinkContext& context, bool runResolver) const;
	bool								isAddrInSection(uintptr_t addr, uint8_t sectionIndex);
	void								doBindExternalRelocations(const LinkContext& context);
	uintptr_t							bindIndirectSymbol(uintptr_t* ptrToBind, const struct macho_section* sect, 
															const char* symbolName, uintptr_t targetAddr, 
															const ImageLoader* targetImage, const LinkContext& context);
	void								bindIndirectSymbolPointers(const LinkContext& context, bool bindNonLazys, bool bindLazys);
	void								initializeLazyStubs(const LinkContext& context);
	void								prefetchLINKEDIT(const LinkContext& context);
#if SPLIT_SEG_DYLIB_SUPPORT	
	unsigned int						getExtraZeroFillEntriesCount();
	void								initMappingTable(uint64_t offsetInFat, shared_file_mapping_np *mappingTable);
#endif
	int									mapSplitSegDylibOutsideSharedRegion(int fd, uint64_t offsetInFat, uint64_t lenInFat, uint64_t fileLen, const LinkContext& context);
#if SPLIT_SEG_SHARED_REGION_SUPPORT
	int									mapSplitSegDylibInfoSharedRegion(int fd, uint64_t offsetInFat, uint64_t lenInFat, uint64_t fileLen, const LinkContext& context);
#endif

			
	const char*								fStrings;
	const struct macho_nlist*				fSymbolTable;
	const struct dysymtab_command*			fDynamicInfo;

};




#endif // __IMAGELOADER_MACHO_CLASSIC__





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


#ifndef __IMAGELOADER_MACHO_COMPRESSED__
#define __IMAGELOADER_MACHO_COMPRESSED__

#include <stdint.h> 

#include "ImageLoaderMachO.h"


//
// ImageLoaderMachOCompressed is the concrete subclass of ImageLoader which loads mach-o files 
// that use the compressed LINKEDIT format.  
//
class ImageLoaderMachOCompressed : public ImageLoaderMachO {
public:
	static ImageLoaderMachOCompressed*	instantiateMainExecutable(const macho_header* mh, uintptr_t slide, const char* path, 
																	unsigned int segCount, unsigned int libCount, const LinkContext& context);
	static ImageLoaderMachOCompressed*	instantiateFromFile(const char* path, int fd, const uint8_t *fileData, size_t lenFileData,
															uint64_t offsetInFat, uint64_t lenInFat, const struct stat& info, 
															unsigned int segCount, unsigned int libCount, 
															const struct linkedit_data_command* codeSigCmd, 
															const struct encryption_info_command* encryptCmd,
															const LinkContext& context);
	static ImageLoaderMachOCompressed*	instantiateFromCache(const macho_header* mh, const char* path, long slide, const struct stat& info,
																unsigned int segCount, unsigned int libCount, const LinkContext& context);
	static ImageLoaderMachOCompressed*	instantiateFromMemory(const char* moduleName, const macho_header* mh, uint64_t len, 
															unsigned int segCount, unsigned int libCount, const LinkContext& context);


	virtual								~ImageLoaderMachOCompressed();

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
	virtual void						registerInterposing(const LinkContext& context);
	virtual bool						usesChainedFixups() const;
	virtual void						makeDataReadOnly() const;

protected:
	virtual void						doInterpose(const LinkContext& context);
	virtual void						dynamicInterpose(const LinkContext& context);
	virtual void						setDyldInfo(const dyld_info_command* dyldInfo) { fDyldInfo = dyldInfo; }
	virtual void						setChainedFixups(const linkedit_data_command* fixups) { fChainedFixups = fixups; }
	virtual void						setExportsTrie(const linkedit_data_command* trie) { fExportsTrie = trie; }
	virtual void						setSymbolTableInfo(const macho_nlist*, const char*, const dysymtab_command*) {}
	virtual	bool						isSubframeworkOf(const LinkContext& context, const ImageLoader* image) const { return false; }
	virtual	bool						hasSubLibrary(const LinkContext& context, const ImageLoader* child) const { return false; }
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
	virtual uintptr_t					resolveWeak(const LinkContext& context, const char* symbolName, bool weak_import, bool runResolver,
													const ImageLoader** foundIn);

		
private:
	struct LastLookup { long ordinal; uint8_t flags; const char* name; uintptr_t result; const ImageLoader* foundIn; };


	typedef uintptr_t                   (^bind_handler)(const LinkContext& context, ImageLoaderMachOCompressed* image, uintptr_t addr, uint8_t type,
														const char* symbolName, uint8_t symboFlags, intptr_t addend, long libraryOrdinal,
														ExtraBindData *extraBindData,
														const char* msg, LastLookup* last, bool runResolver);

	void								eachLazyBind(const LinkContext& context, bind_handler);
	void								eachBind(const LinkContext& context, bind_handler);


										ImageLoaderMachOCompressed(const macho_header* mh, const char* path, unsigned int segCount,
																	uint32_t segOffsets[], unsigned int libCount);
	static ImageLoaderMachOCompressed*	instantiateStart(const macho_header* mh, const char* path, unsigned int segCount, unsigned int libCount);
	void								instantiateFinish(const LinkContext& context);

	void								rebaseAt(const LinkContext& context, uintptr_t addr, uintptr_t slide, uint8_t type);
	void								throwBadRebaseAddress(uintptr_t address, uintptr_t segmentEndAddress, int segmentIndex, 
												const uint8_t* startOpcodes, const uint8_t* endOpcodes, const uint8_t* pos);
	static uintptr_t					bindAt(const LinkContext& context, ImageLoaderMachOCompressed* image, uintptr_t addr, uint8_t type, const char* symbolName,
                                               uint8_t symboFlags, intptr_t addend, long libraryOrdinal,
                                               ExtraBindData *extraBindData,
                                               const char* msg,
												LastLookup* last, bool runResolver=false);
	void								bindCompressed(const LinkContext& context);
	void								throwBadBindingAddress(uintptr_t address, uintptr_t segmentEndAddress, int segmentIndex, 
												const uint8_t* startOpcodes, const uint8_t* endOpcodes, const uint8_t* pos);
	uintptr_t							resolve(const LinkContext& context, const char* symbolName, 
												uint8_t symboFlags, long libraryOrdinal, const ImageLoader** targetImage, 
												LastLookup* last = NULL, bool runResolver=false);
	uintptr_t							resolveFlat(const LinkContext& context, const char* symbolName, bool weak_import, bool runResolver,
													const ImageLoader** foundIn);
	uintptr_t							resolveCoalesced(const LinkContext& context, const char* symbolName, const ImageLoader** foundIn);
	uintptr_t							resolveTwolevel(const LinkContext& context, const char* symbolName, const ImageLoader* definedInImage,
													  const ImageLoader* requestorImage, unsigned requestorOrdinalOfDef, bool weak_import, bool runResolver,
													  const ImageLoader** foundInn);
	static uintptr_t					interposeAt(const LinkContext& context, ImageLoaderMachOCompressed* image, uintptr_t addr, uint8_t type, const char*, 
                                                    uint8_t, intptr_t, long,
                                                    ExtraBindData *extraBindData,
                                                    const char*, LastLookup*, bool runResolver);
	static uintptr_t					dynamicInterposeAt(const LinkContext& context, ImageLoaderMachOCompressed* image, uintptr_t addr, uint8_t type, const char*, 
                                                           uint8_t, intptr_t, long,
                                                           ExtraBindData *extraBindData,
                                                           const char*, LastLookup*, bool runResolver);
    void                                updateOptimizedLazyPointers(const LinkContext& context);
    void                                updateAlternateLazyPointer(uint8_t* stub, void** originalLazyPointerAddr, const LinkContext& context);
	void								registerEncryption(const struct encryption_info_command* encryptCmd, const LinkContext& context);
	void								doApplyFixups(const LinkContext& context, const dyld_chained_fixups_header* fixupsHeader);

	const struct dyld_info_command*			fDyldInfo;
	const struct linkedit_data_command*		fChainedFixups;
	const struct linkedit_data_command*		fExportsTrie;
};



#endif // __IMAGELOADER_MACHO_COMPRESSED__





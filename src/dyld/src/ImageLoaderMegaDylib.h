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


#ifndef __IMAGELOADER_MEGADYLIB__
#define __IMAGELOADER_MEGADYLIB__

#include <stdint.h> 
#include <pthread.h>
#include <uuid/uuid.h>

#include "ImageLoaderMachO.h"
#include "dyld_cache_format.h"


//
// ImageLoaderMegaDylib is the concrete subclass of ImageLoader which represents
// all dylibs in the shared cache.
//
class ImageLoaderMegaDylib : public ImageLoader {
public:
	static ImageLoaderMegaDylib*		makeImageLoaderMegaDylib(const dyld_cache_header*, long slide, const macho_header* mainMH, const LinkContext&);


	virtual								~ImageLoaderMegaDylib();

	void								appendImagesNeedingCoalescing(ImageLoader* images[], unsigned imageIndex[], unsigned& count);
	virtual	void						initializeCoalIterator(CoalIterator&, unsigned int loadOrder, unsigned imageIndex);
	virtual	bool						incrementCoalIterator(CoalIterator&);
	virtual	uintptr_t					getAddressCoalIterator(CoalIterator&, const LinkContext& contex);
	virtual	void						updateUsesCoalIterator(CoalIterator&, uintptr_t newAddr, ImageLoader* target, unsigned imageIndex, const LinkContext& context);

	virtual const char*					getIndexedPath(unsigned index) const;
	virtual const char*					getIndexedShortName(unsigned) const;
	virtual const char*					getInstallPath() const;
	virtual bool						inSharedCache() const { return true; }
	virtual bool						containsSymbol(const void* addr) const { unreachable(); }
	virtual void*						getEntryFromLC_MAIN() const { unreachable(); }
	virtual void*						getEntryFromLC_UNIXTHREAD() const { unreachable(); }
	virtual const struct mach_header*   machHeader() const { unreachable(); }
	virtual uintptr_t					getSlide() const { return _slide; }
	virtual const void*					getEnd() const { unreachable(); }
	virtual bool						hasCoalescedExports() const { unreachable(); }
	virtual bool						findExportedSymbolAddress(const LinkContext& context, const char* symbolName,
																const ImageLoader* requestorImage, int requestorOrdinalOfDef,
																bool runResolver, const ImageLoader** foundIn, uintptr_t* address) const;
	virtual const Symbol*				findExportedSymbol(const char* name, bool searchReExports, const char* thisPath, const ImageLoader** foundIn) const;
	virtual uintptr_t					getExportedSymbolAddress(const Symbol* sym, const LinkContext& context,
																 const ImageLoader* requestor, bool runResolver, const char* symbolName) const;
	virtual DefinitionFlags				getExportedSymbolInfo(const Symbol* sym) const { unreachable(); }
	virtual const char*					getExportedSymbolName(const Symbol* sym) const { unreachable(); }
	virtual uint32_t					getExportedSymbolCount() const { unreachable(); }
	virtual const Symbol*				getIndexedExportedSymbol(uint32_t index) const { unreachable(); }

	virtual uint32_t					getImportedSymbolCount() const { unreachable(); }
	virtual const Symbol*				getIndexedImportedSymbol(uint32_t index) const { unreachable(); }
	virtual ReferenceFlags				getImportedSymbolInfo(const Symbol* sym) const { unreachable(); }
	virtual const char*					getImportedSymbolName(const Symbol* sym) const { unreachable(); }
	virtual const char*					findClosestSymbol(const void* addr, const void** closestAddr) const { unreachable(); }
	virtual bool						isBundle() const { return false; }
	virtual bool						isDylib() const { return true;  }
	virtual bool						isExecutable() const { unreachable();  }
	virtual bool						isPositionIndependentExecutable() const { unreachable();  }
	virtual bool						forceFlat() const { unreachable();  }
	virtual uintptr_t					doBindLazySymbol(uintptr_t* lazyPointer, const LinkContext& context) { unreachable(); }
	virtual uintptr_t					doBindFastLazySymbol(uint32_t lazyBindingInfoOffset, const LinkContext& context,
															void (*lock)(), void (*unlock)()) { unreachable(); }
	virtual void						doTermination(const LinkContext& context) { unreachable(); }
	virtual bool						needsInitialization() { unreachable(); }
	virtual bool						getSectionContent(const char* segmentName, const char* sectionName, void** start, size_t* length) { unreachable(); }
	virtual void						getUnwindInfo(dyld_unwind_sections* info) { unreachable(); }
    virtual bool                        findSection(const void* imageInterior, const char** segmentName, const char** sectionName, size_t* sectionOffset) { unreachable(); }
    virtual const struct macho_section* findSection(const void* imageInterior) const { unreachable(); }
	virtual bool						isPrebindable() const { unreachable();  }
	virtual bool						usablePrebinding(const LinkContext& context) const { unreachable(); }
	virtual	void						getRPaths(const LinkContext& context, std::vector<const char*>&) const { }
	virtual bool						participatesInCoalescing() const { unreachable(); }
	virtual	bool						getUUID(uuid_t) const { unreachable(); }
	virtual void						dynamicInterpose(const LinkContext& context) { unreachable(); }
	void								addDynamicInterposingTuples(const struct dyld_interpose_tuple array[], size_t count) { unreachable(); }
	virtual unsigned int				segmentCount() const { unreachable(); }
	virtual const char*					segName(unsigned int) const { unreachable(); }
	virtual uintptr_t					segSize(unsigned int) const { unreachable(); }
	virtual uintptr_t					segFileSize(unsigned int) const { unreachable(); }
	virtual bool						segHasTrailingZeroFill(unsigned int) { unreachable(); }
	virtual uintptr_t					segFileOffset(unsigned int) const { unreachable(); }
	virtual bool						segReadable(unsigned int) const { unreachable(); }
	virtual bool						segWriteable(unsigned int) const { unreachable(); }
	virtual bool						segExecutable(unsigned int) const { unreachable(); }
	virtual bool						segUnaccessible(unsigned int) const { unreachable(); }
	virtual bool						segHasPreferredLoadAddress(unsigned int) const { unreachable(); }
	virtual uintptr_t					segPreferredLoadAddress(unsigned int) const { unreachable(); }
	virtual uintptr_t					segActualLoadAddress(unsigned int) const { unreachable(); }
	virtual uintptr_t					segActualEndAddress(unsigned int) const { unreachable(); }

	
										// info from LC_VERSION_MIN_MACOSX or LC_VERSION_MIN_IPHONEOS
	virtual uint32_t					sdkVersion() const { unreachable(); }
	virtual uint32_t					minOSVersion() const { unreachable(); }
	
										// if the image contains interposing functions, register them
	virtual void						registerInterposing(const LinkContext& context) { unreachable(); }

	virtual ImageLoader*				libImage(unsigned int) const { unreachable(); }
	virtual bool						libReExported(unsigned int) const { unreachable(); }
	virtual bool						libIsUpward(unsigned int) const { unreachable(); }
	virtual void						setLibImage(unsigned int, ImageLoader*, bool, bool) { unreachable(); }
	virtual const char*					libPath(unsigned int) const { unreachable(); }

	unsigned							appendImagesToNotify(dyld_image_states state, bool orLater, dyld_image_info* infos);
	const char*							notify(dyld_image_states state, bool orLater, dyld_image_state_change_handler);
	bool								dlopenFromCache(const LinkContext& context, const char* path, int mode, void** handle);
	bool								makeCacheHandle(const LinkContext& context, unsigned cacheIndex, int mode, void** result);
	void*								dlsymFromCache(const LinkContext& context, void* handle, const char* symName, unsigned index);
	bool								isCacheHandle(void* handle, unsigned* index, uint8_t* flags);
	bool								hasDylib(const char* path, unsigned* index) const;
	bool								addressInCache(const void* address, const mach_header** mh, const char** path, unsigned* index);
	bool								findUnwindSections(const void* addr, dyld_unwind_sections* info);
	bool								dladdrFromCache(const void* address, Dl_info* info);
	uintptr_t							bindLazy(uintptr_t lazyBindingInfoOffset, const LinkContext& context, const mach_header* mh, unsigned index);
	bool								flatFindSymbol(const char* name, bool onlyInCoalesced, const ImageLoader::Symbol** sym, const ImageLoader** image, ImageLoader::CoalesceNotifier);
	void								getDylibUUID(unsigned int index, uuid_t) const;

protected:
	virtual void						setDyldInfo(const dyld_info_command* dyldInfo) { unreachable(); }
	virtual void						setChainedFixups(const linkedit_data_command* fixups) { unreachable(); }
	virtual void						setExportsTrie(const linkedit_data_command*) { unreachable(); }
	virtual void						setSymbolTableInfo(const macho_nlist*, const char*, const dysymtab_command*) { unreachable(); }
	virtual uint32_t*					segmentCommandOffsets() const { unreachable(); }
	virtual	void						rebase(const LinkContext& context, uintptr_t slide) { unreachable(); }
	virtual uintptr_t					exportedSymbolAddress(const LinkContext& context, const Symbol* symbol, const ImageLoader* requestor, bool runResolver) const { unreachable(); }
	virtual bool						exportedSymbolIsWeakDefintion(const Symbol* symbol) const { unreachable(); }
	virtual const char*					exportedSymbolName(const Symbol* symbol) const { unreachable(); }
	virtual unsigned int				exportedSymbolCount() const { unreachable(); }
	virtual const ImageLoader::Symbol*	exportedSymbolIndexed(unsigned int) const { unreachable(); }
	virtual unsigned int				importedSymbolCount() const { unreachable(); }
	virtual const ImageLoader::Symbol*	importedSymbolIndexed(unsigned int) const { unreachable(); }
	virtual const char*					importedSymbolName(const Symbol* symbol) const { unreachable(); }
#if PREBOUND_IMAGE_SUPPORT
	virtual void						resetPreboundLazyPointers(const LinkContext& context) { unreachable(); }
#endif

	virtual void						recursiveLoadLibraries(const LinkContext& context, bool preflightOnly, const RPathChain& loaderRPaths, const char* loadPath);
	virtual unsigned 					recursiveUpdateDepth(unsigned int maxDepth);
	virtual void						recursiveRebase(const LinkContext& context) {  }
	virtual void						recursiveBind(const LinkContext& context, bool forceLazysBound, bool neverUnload);
	virtual void						recursiveApplyInterposing(const LinkContext& context);
	virtual void						recursiveMakeDataReadOnly(const LinkContext& context) {}
	virtual void						recursiveGetDOFSections(const LinkContext& context, std::vector<DOFInfo>& dofs) { }
	virtual void						recursiveInitialization(const LinkContext& context, mach_port_t this_thread, const char* pathToInitialize,
																ImageLoader::InitializerTimingList&, ImageLoader::UninitedUpwards&);

	virtual void						doGetDependentLibraries(DependentLibraryInfo libs[]) { unreachable(); }
	virtual LibraryInfo					doGetLibraryInfo(const LibraryInfo& requestorInfo) { return requestorInfo; }
	virtual void						doRebase(const LinkContext& context) { unreachable(); }
	virtual void						doBind(const LinkContext& context, bool forceLazysBound) { unreachable(); }
	virtual void						doBindJustLazies(const LinkContext& context) { unreachable(); }
	virtual void						doGetDOFSections(const LinkContext& context, std::vector<DOFInfo>& dofs) { unreachable(); }
	virtual void						doInterpose(const LinkContext& context) { unreachable(); }
	virtual bool						doInitialization(const LinkContext& context) { unreachable(); }
	virtual bool						needsTermination() { unreachable(); }
	virtual bool						segmentsMustSlideTogether() const { unreachable(); }
	virtual bool						segmentsCanSlide() const { unreachable(); }
	virtual void						setSlide(intptr_t slide) { unreachable(); }
			bool						allDependentLibrariesAsWhenPreBound() const { unreachable(); }
	virtual	bool						isSubframeworkOf(const LinkContext& context, const ImageLoader* image) const { return false; }
	virtual	bool						hasSubLibrary(const LinkContext& context, const ImageLoader* child) const { return false; }
	virtual bool						weakSymbolsBound(unsigned index) const;
	virtual void						setWeakSymbolsBound(unsigned index);

private:
										ImageLoaderMegaDylib(const dyld_cache_header*, long slide, const macho_header* mainMH, const LinkContext&);

    struct UpwardIndexes
	{
		uint16_t	 count;
		uint16_t     images[1];
	};

	const macho_header*					getIndexedMachHeader(unsigned index) const;
	const uint8_t*						getIndexedTrie(unsigned index, uint32_t& trieSize) const;
	unsigned							findImageIndex(const LinkContext& context, const char* path) const;
	void								recursiveMarkLoaded(const LinkContext& context, unsigned imageIndex);
	void								markAllbound(const LinkContext& context);
	bool								findInChainedTries(const LinkContext& context, const char* symbolName, unsigned definedImageIndex,
															const ImageLoader* requestorImage, bool runResolver, uintptr_t* address) const;
	bool								findInChainedTriesAndDependents(const LinkContext& context, const char* symbolName, unsigned definedImageIndex,
																		const ImageLoader* requestorImage, bool runResolver, uintptr_t* address) const;
	bool								findInChainedTriesAndDependentsExcept(const LinkContext& context, const char* symbolName, unsigned imageIndex,
																			const ImageLoader* requestorImage, bool runResolver, bool alreadyVisited[], uintptr_t* address) const;
	bool								exportTrieHasNodeRecursive(const char* symbolName, unsigned index,
																	const uint8_t** exportNode, const uint8_t** exportTrieEnd,
																	unsigned* foundinIndex) const;
	bool								exportTrieHasNode(const char* symbolName, unsigned index,
														  const uint8_t** exportNode, const uint8_t** exportTrieEnd) const;

	void								initAllLoaded(const LinkContext& context, InitializerTimingList& timingInfo);
	void								printSegments(const macho_header* mh) const;
	void*								makeCacheHandle(unsigned index, int mode);
	uint8_t								flagsFromCacheHandle(void* handle);
	void								processExportNode(const LinkContext& context, const char* symbolName, unsigned definedImageIndex,
															const uint8_t* exportNode, const uint8_t* exportTrieEnd,
															const ImageLoader* requestorImage, bool runResolver, uintptr_t* address) const;
	static uint8_t						dyldStateToCacheState(dyld_image_states state);
	void								recursiveInitialization(const LinkContext& context, mach_port_t this_thread, unsigned int imageIndex,
																InitializerTimingList& timingInfo, UpwardIndexes&);
	void								recursiveSpinLockAcquire(unsigned int imageIndex, mach_port_t thisThread);
	void								recursiveSpinLockRelease(unsigned int imageIndex, mach_port_t thisThread);

	__attribute__((noreturn))
	void								unreachable() const;

	enum { kStateUnused=0, kStateLoaded=1, kStateFlagBound=2, kStateFlagWeakBound=3, kStateFlagInitialized=4 };


	const dyld_cache_header*					_header;
	const void*									_endOfCacheInMemory;
	const uint8_t*								_linkEditBias;
	const dyld_cache_image_info*				_images;
	const dyld_cache_image_info_extra*			_imageExtras;
	long										_slide;
	const dyld_cache_accelerator_initializer*	_initializers;
	const dyld_cache_range_entry*				_rangeTable;
	const uint16_t*								_reExportsArray;
	const uint16_t*								_dependenciesArray;
	const uint16_t*								_bottomUpArray;
	const uint8_t*								_dylibsTrieStart;
	const uint8_t*								_dylibsTrieEnd;
	const dyld_cache_image_text_info*			_imageTextInfo;
	uint8_t*									_stateFlags;
	uint32_t									_imageCount;
	uint32_t									_initializerCount;
	uint32_t									_rangeTableCount;
	pthread_mutex_t								_lockArrayGuard;
	ImageLoader::recursive_lock*				_lockArray;
	unsigned int								_lockArrayInUseCount;
};



#endif // __IMAGELOADER_MEGADYLIB__





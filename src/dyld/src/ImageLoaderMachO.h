/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2004-2009 Apple Inc. All rights reserved.
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


#ifndef __IMAGELOADERMACHO__
#define __IMAGELOADERMACHO__

#include <stdint.h> 
#include <mach-o/loader.h> 
#include <mach-o/nlist.h>
#include <uuid/uuid.h>
#include <mach-o/dyld_images.h>

#if __has_feature(ptrauth_calls)
#include <ptrauth.h>
#endif

#include "ImageLoader.h"

#define BIND_TYPE_THREADED_BIND 100


#define BIND_TYPE_THREADED_REBASE 102


//
// ImageLoaderMachO is a subclass of ImageLoader which loads mach-o format files.
//
//
class ImageLoaderMachO : public ImageLoader {
public:
	static ImageLoader*					instantiateMainExecutable(const macho_header* mh, uintptr_t slide, const char* path, const LinkContext& context);
	static ImageLoader*					instantiateFromFile(const char* path, int fd, const uint8_t firstPages[], size_t firstPagesSize, uint64_t offsetInFat,
															uint64_t lenInFat, const struct stat& info, const LinkContext& context);
	static ImageLoader*					instantiateFromCache(const macho_header* mh, const char* path, long slide, const struct stat& info, const LinkContext& context);
	static ImageLoader*					instantiateFromMemory(const char* moduleName, const macho_header* mh, uint64_t len, const LinkContext& context);


	bool								inSharedCache() const { return fInSharedCache; }
	void								disableCoverageCheck() { fCoveredCodeLength = UINT64_MAX; }

	const char*							getInstallPath() const;
	virtual void*						getEntryFromLC_UNIXTHREAD() const;
	virtual void*						getEntryFromLC_MAIN() const;
	virtual const struct mach_header*   machHeader() const;
	virtual uintptr_t					getSlide() const;
	virtual const void*					getEnd() const;
	virtual bool						hasCoalescedExports() const;
	virtual const Symbol*				findExportedSymbol(const char* name, bool searchReExports, const char* thisPath, const ImageLoader** foundIn) const;
	virtual uintptr_t					getExportedSymbolAddress(const Symbol* sym, const LinkContext& context, 
																const ImageLoader* requestor, bool runResolver, const char*) const;
	virtual DefinitionFlags				getExportedSymbolInfo(const Symbol* sym) const;
	virtual const char*					getExportedSymbolName(const Symbol* sym) const;
	virtual uint32_t					getExportedSymbolCount() const;
	virtual const Symbol*				getIndexedExportedSymbol(uint32_t index) const;
	virtual uint32_t					getImportedSymbolCount() const;
	virtual const Symbol*				getIndexedImportedSymbol(uint32_t index) const;
	virtual ReferenceFlags				getImportedSymbolInfo(const Symbol* sym) const;
	virtual const char*					getImportedSymbolName(const Symbol* sym) const;
	virtual bool						isBundle() const;
	virtual bool						isDylib() const;
	virtual bool						isExecutable() const;
	virtual bool						isPositionIndependentExecutable() const;
	virtual bool						forceFlat() const;
	virtual bool						participatesInCoalescing() const;
	virtual const char*					findClosestSymbol(const void* addr, const void** closestAddr) const = 0;
	virtual	void						initializeCoalIterator(CoalIterator&, unsigned int loadOrder, unsigned) = 0;
	virtual	bool						incrementCoalIterator(CoalIterator&) = 0;
	virtual	uintptr_t					getAddressCoalIterator(CoalIterator&, const LinkContext& contex) = 0;
	virtual	void						updateUsesCoalIterator(CoalIterator&, uintptr_t newAddr, ImageLoader* target, unsigned targetIndex, const LinkContext& context) = 0;
	virtual uintptr_t					doBindLazySymbol(uintptr_t* lazyPointer, const LinkContext& context) = 0;
	virtual uintptr_t					doBindFastLazySymbol(uint32_t lazyBindingInfoOffset, const LinkContext& context, void (*lock)(), void (*unlock)()) = 0;
	virtual void						doTermination(const LinkContext& context);
	virtual bool						needsInitialization();
	virtual bool						getSectionContent(const char* segmentName, const char* sectionName, void** start, size_t* length);
	virtual void						getUnwindInfo(dyld_unwind_sections* info);
    virtual const struct macho_section* findSection(const void* imageInterior) const;
	virtual bool						findSection(const void* imageInterior, const char** segmentName, const char** sectionName, size_t* sectionOffset);
	virtual bool						usablePrebinding(const LinkContext& context) const;
	virtual unsigned int				segmentCount() const;
	virtual const char*					segName(unsigned int) const;
	virtual uintptr_t					segSize(unsigned int) const;
	virtual uintptr_t					segFileSize(unsigned int) const;
	virtual bool						segHasTrailingZeroFill(unsigned int);
	virtual uintptr_t					segFileOffset(unsigned int) const;
	virtual bool						segReadable(unsigned int) const;
	virtual bool						segWriteable(unsigned int) const;
	virtual bool						segExecutable(unsigned int) const;
	virtual bool						segUnaccessible(unsigned int) const;
	virtual bool						segHasPreferredLoadAddress(unsigned int) const;
	virtual uintptr_t					segActualLoadAddress(unsigned int) const;
	virtual uintptr_t					segPreferredLoadAddress(unsigned int) const;
	virtual uintptr_t					segActualEndAddress(unsigned int) const;
	virtual void						registerInterposing(const LinkContext& context);
	virtual uint32_t					sdkVersion() const;
	virtual uint32_t					minOSVersion() const;
	virtual const char*					libPath(unsigned int) const;
	virtual	bool						notifyObjC() const { return fNotifyObjC; }
	virtual bool						overridesCachedDylib(uint32_t& num) const { num = fOverrideOfCacheImageNum; return (num != 0); }
	virtual void						setOverridesCachedDylib(uint32_t num) { fOverrideOfCacheImageNum = num; }


	static void							printStatisticsDetails(unsigned int imageCount, const InitializerTimingList&);
	static uint32_t						minOSVersion(const mach_header*);
	static uint32_t						sdkVersion(const mach_header* mh);
	static intptr_t						computeSlide(const mach_header* mh);
	static bool							findSection(const mach_header* mh, const char* segmentName, const char* sectionName, void** sectAddress, uintptr_t* sectSize);
	static const dyld_info_command*		findDyldInfoLoadCommand(const mach_header* mh);
	static const char*					findClosestSymbol(const mach_header* mh, const void* addr, const void** closestAddr);
	static bool							getLazyBindingInfo(uint32_t& lazyBindingInfoOffset, const uint8_t* lazyInfoStart, const uint8_t* lazyInfoEnd,
														  uint8_t* segIndex, uintptr_t* segOffset, int* ordinal, const char** symbolName, bool* doneAfterBind);
	static uintptr_t					segPreferredAddress(const mach_header* mh, unsigned segIndex);

    uintptr_t                           imageBaseAddress() const;

    struct ExtraBindData {
        ExtraBindData() = default;
        explicit ExtraBindData(uint64_t d) : data(d) { }

        union {
            uint64_t data = 0;
        };
        bool operator==(const ExtraBindData& other) const {
            return this->data == other.data;
        }
        bool operator!=(const ExtraBindData& other) const {
            return !(*this == other);
        }
        bool operator<(const ExtraBindData& other) const {
            return data < other.data;
        }

    };

	static uintptr_t					bindLocation(const LinkContext& context, uintptr_t baseVMAddress,
                                                     uintptr_t location, uintptr_t value,
                                                     uint8_t type, const char* symbolName,
                                                     intptr_t addend, const char* inPath, const char* toPath, const char* msg,
                                                     ExtraBindData *extraBindData,
                                                     uintptr_t fSlide);
	virtual	void						rebase(const LinkContext& context, uintptr_t slide) = 0;



protected:
						ImageLoaderMachO(const ImageLoaderMachO&);
						ImageLoaderMachO(const macho_header* mh, const char* path, unsigned int segCount,
																	uint32_t segOffsets[], unsigned int libCount);
	virtual				~ImageLoaderMachO() {}

	void				operator=(const ImageLoaderMachO&);

	virtual void						setDyldInfo(const struct dyld_info_command*) = 0;
	virtual void						setChainedFixups(const linkedit_data_command*) = 0;
	virtual void						setExportsTrie(const linkedit_data_command*) = 0;
	virtual void						setSymbolTableInfo(const macho_nlist*, const char*, const dysymtab_command*) = 0;
	virtual	bool						isSubframeworkOf(const LinkContext& context, const ImageLoader* image) const = 0;
	virtual	bool						hasSubLibrary(const LinkContext& context, const ImageLoader* child) const = 0;
	virtual uint32_t*					segmentCommandOffsets() const = 0;
	virtual const ImageLoader::Symbol*	findShallowExportedSymbol(const char* name, const ImageLoader** foundIn) const = 0;
	virtual bool						containsSymbol(const void* addr) const = 0;
	virtual uintptr_t					exportedSymbolAddress(const LinkContext& context, const Symbol* symbol, const ImageLoader* requestor, bool runResolver) const = 0;
	virtual bool						exportedSymbolIsWeakDefintion(const Symbol* symbol) const = 0;
	virtual const char*					exportedSymbolName(const Symbol* symbol) const = 0;
	virtual unsigned int				exportedSymbolCount() const = 0;
	virtual const ImageLoader::Symbol*	exportedSymbolIndexed(unsigned int) const = 0;
	virtual unsigned int				importedSymbolCount() const = 0;
	virtual const ImageLoader::Symbol*	importedSymbolIndexed(unsigned int) const = 0;
	virtual const char*					importedSymbolName(const Symbol* symbol) const = 0;
#if PREBOUND_IMAGE_SUPPORT
	virtual void						resetPreboundLazyPointers(const LinkContext& context) = 0;
#endif


	virtual void		doGetDependentLibraries(DependentLibraryInfo libs[]);
	virtual LibraryInfo doGetLibraryInfo(const LibraryInfo& requestorInfo);
	virtual	void		getRPaths(const LinkContext& context, std::vector<const char*>&) const;
	virtual	bool		getUUID(uuid_t) const;
	virtual void		doRebase(const LinkContext& context);
	virtual void		doBind(const LinkContext& context, bool forceLazysBound) = 0;
	virtual void		doBindJustLazies(const LinkContext& context) = 0;
	virtual bool		doInitialization(const LinkContext& context);
	virtual void		doGetDOFSections(const LinkContext& context, std::vector<ImageLoader::DOFInfo>& dofs);
	virtual bool		needsTermination();
	virtual bool		segmentsMustSlideTogether() const;	
	virtual bool		segmentsCanSlide() const;			
	virtual void		setSlide(intptr_t slide);		
	virtual bool		usesTwoLevelNameSpace() const;
	virtual bool		isPrebindable() const;

protected:

			void		destroy();
	static void			sniffLoadCommands(const macho_header* mh, const char* path, bool inCache, bool* compressed,
											unsigned int* segCount, unsigned int* libCount, const LinkContext& context,
											const linkedit_data_command** codeSigCmd,
											const encryption_info_command** encryptCmd);
	static bool			needsAddedLibSystemDepency(unsigned int libCount, const macho_header* mh);
			void		loadCodeSignature(const struct linkedit_data_command* codeSigCmd, int fd, uint64_t offsetInFatFile, const LinkContext& context);
			void		validateFirstPages(const struct linkedit_data_command* codeSigCmd, int fd, const uint8_t *fileData, size_t lenFileData, off_t offsetInFat, const LinkContext& context);
			const struct macho_segment_command* segLoadCommand(unsigned int segIndex) const;
			void		parseLoadCmds(const ImageLoader::LinkContext& context);
			int 		crashIfInvalidCodeSignature();
			bool		segHasRebaseFixUps(unsigned int) const;
			bool		segHasBindFixUps(unsigned int) const;
			void		segProtect(unsigned int segIndex, const ImageLoader::LinkContext& context);
			void		segMakeWritable(unsigned int segIndex, const ImageLoader::LinkContext& context);
#if __i386__
			bool		segIsReadOnlyImport(unsigned int) const;
#endif
			bool		segIsReadOnlyData(unsigned int) const;
			intptr_t	assignSegmentAddresses(const LinkContext& context);
			uintptr_t	reserveAnAddressRange(size_t length, const ImageLoader::LinkContext& context);
			bool		reserveAddressRange(uintptr_t start, size_t length);
			void		mapSegments(int fd, uint64_t offsetInFat, uint64_t lenInFat, uint64_t fileLen, const LinkContext& context);
			void		mapSegments(const void* memoryImage, uint64_t imageLen, const LinkContext& context);
			void		UnmapSegments();
			void		__attribute__((noreturn)) throwSymbolNotFound(const LinkContext& context, const char* symbol, 
																	const char* referencedFrom, const char* fromVersMismatch,
																	const char* expectedIn);
			void		doImageInit(const LinkContext& context);
			void		doModInitFunctions(const LinkContext& context);
			void		setupLazyPointerHandler(const LinkContext& context);
			void		lookupProgramVars(const LinkContext& context) const;

			void		makeTextSegmentWritable(const LinkContext& context, bool writeable);


			void		doInterpose(const LinkContext& context) = 0;
			bool		hasReferencesToWeakSymbols() const;
			uintptr_t	getSymbolAddress(const Symbol* sym, const ImageLoader* requestor, 
										const LinkContext& context, bool runResolver) const;
			
	static uintptr_t			bindLazySymbol(const mach_header*, uintptr_t* lazyPointer);
protected:
	uint64_t								fCoveredCodeLength;
	const uint8_t*							fMachOData;
	const uint8_t*							fLinkEditBase; // add any internal "offset" to this to get mapped address
	uintptr_t								fSlide;
	uint32_t								fEHFrameSectionOffset;
	uint32_t								fUnwindInfoSectionOffset;
	uint32_t								fDylibIDOffset;
	uint32_t								fSegmentsCount : 8,
											fIsSplitSeg : 1,
											fInSharedCache : 1,
#if TEXT_RELOC_SUPPORT
											fTextSegmentRebases : 1, 
											fTextSegmentBinds : 1,
#else
											fReadOnlyDataSegment : 1,
#endif
#if __i386__
											fReadOnlyImportSegment : 1,
#endif
											fHasSubLibraries : 1,
											fHasSubUmbrella : 1,
											fInUmbrella : 1,
											fHasDOFSections : 1,
											fHasDashInit : 1,
											fHasInitializers : 1,
											fHasTerminators : 1,
											fNotifyObjC : 1,
											fRetainForObjC : 1,
											fRegisteredAsRequiresCoalescing : 1, 	// <rdar://problem/7886402> Loading MH_DYLIB_STUB causing coalescable miscount
											fOverrideOfCacheImageNum : 12;

											
	static uint32_t					fgSymbolTableBinarySearchs;
};


#endif // __IMAGELOADERMACHO__





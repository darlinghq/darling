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


#ifndef __IMAGELOADER__
#define __IMAGELOADER__

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <mach/mach_time.h> // struct mach_timebase_info
#include <mach/mach_init.h> // struct mach_thread_self
#include <mach/shared_region.h>
#include <mach-o/loader.h> 
#include <mach-o/nlist.h> 
#include <mach-o/dyld_images.h>
#include <mach-o/dyld_priv.h>
#include <stdint.h>
#include <stdlib.h>
#include <TargetConditionals.h>
#include <vector>
#include <new>
#include <uuid/uuid.h>

#if !TARGET_OS_DRIVERKIT && (BUILDING_LIBDYLD || BUILDING_DYLD)
  #include <CrashReporterClient.h>
#else
  #define CRSetCrashLogMessage(x)
  #define CRSetCrashLogMessage2(x)
#endif

#include "DyldSharedCache.h"

#include "Map.h"

#if __arm__
 #include <mach/vm_page_size.h>
#endif


#ifndef SHARED_REGION_BASE_ARM64
	#define SHARED_REGION_BASE_ARM64 0x7FFF80000000LL
#endif

#ifndef SHARED_REGION_SIZE_ARM64
	#define SHARED_REGION_SIZE_ARM64 0x10000000LL
#endif


#define LOG_BINDINGS 0


#if __IPHONE_OS_VERSION_MIN_REQUIRED          
	#define SPLIT_SEG_SHARED_REGION_SUPPORT 0
	#define SPLIT_SEG_DYLIB_SUPPORT			0
	#define PREBOUND_IMAGE_SUPPORT			__arm__
	#define TEXT_RELOC_SUPPORT				__i386__
	#define SUPPORT_OLD_CRT_INITIALIZATION	0
	#define SUPPORT_LC_DYLD_ENVIRONMENT		1
	#define SUPPORT_VERSIONED_PATHS			0
	#define SUPPORT_CLASSIC_MACHO			__arm__
	#define SUPPORT_ZERO_COST_EXCEPTIONS	(!__USING_SJLJ_EXCEPTIONS__)
	#define INITIAL_IMAGE_COUNT				150
	#define SUPPORT_ACCELERATE_TABLES		!TARGET_OS_SIMULATOR
	#define SUPPORT_ROOT_PATH				TARGET_OS_SIMULATOR
#else
	#define SPLIT_SEG_SHARED_REGION_SUPPORT 0
	#define SPLIT_SEG_DYLIB_SUPPORT			__i386__
	#define PREBOUND_IMAGE_SUPPORT			__i386__
	#define TEXT_RELOC_SUPPORT				__i386__
	#define SUPPORT_OLD_CRT_INITIALIZATION	__i386__
	#define SUPPORT_LC_DYLD_ENVIRONMENT		(__i386__ || __x86_64__)
	#define SUPPORT_VERSIONED_PATHS			1
	#define SUPPORT_CLASSIC_MACHO			1
	#define SUPPORT_ZERO_COST_EXCEPTIONS	1
	#define INITIAL_IMAGE_COUNT				200
	#define SUPPORT_ACCELERATE_TABLES		0
	#define SUPPORT_ROOT_PATH				1
#endif

#define MAX_MACH_O_HEADER_AND_LOAD_COMMANDS_SIZE (32*1024)


// <rdar://problem/13590567> optimize away dyld's initializers
#define VECTOR_NEVER_DESTRUCTED(type) \
	namespace std { \
		template <> \
		__vector_base<type, std::allocator<type> >::~__vector_base() { } \
	}
#define VECTOR_NEVER_DESTRUCTED_EXTERN(type) \
       namespace std { \
               template <> \
               __vector_base<type, std::allocator<type> >::~__vector_base(); \
       }
#define VECTOR_NEVER_DESTRUCTED_IMPL(type) \
       namespace std { \
               template <> \
               __vector_base<type, std::allocator<type> >::~__vector_base() { } \
       }

// utilities
namespace dyld {
	extern __attribute__((noreturn)) void throwf(const char* format, ...)  __attribute__((format(printf, 1, 2)));
	extern void log(const char* format, ...)  __attribute__((format(printf, 1, 2)));
	extern void warn(const char* format, ...)  __attribute__((format(printf, 1, 2)));
	extern const char* mkstringf(const char* format, ...)  __attribute__((format(printf, 1, 2)));
#if LOG_BINDINGS
	extern void logBindings(const char* format, ...)  __attribute__((format(printf, 1, 2)));
#endif
}
extern "C" 	int   vm_alloc(vm_address_t* addr, vm_size_t size, uint32_t flags);
extern "C" 	void* xmmap(void* addr, size_t len, int prot, int flags, int fd, off_t offset);


#if __LP64__
	struct macho_header				: public mach_header_64  {};
	struct macho_nlist				: public nlist_64  {};	
#else
	struct macho_header				: public mach_header  {};
	struct macho_nlist				: public nlist  {};	
#endif


#if __arm64__
	#define dyld_page_trunc(__addr)     (__addr & (-16384))
	#define dyld_page_round(__addr)     ((__addr + 16383) & (-16384))
	#define dyld_page_size              16384
#elif __arm__
	#define dyld_page_trunc(__addr)     trunc_page_kernel(__addr)
	#define dyld_page_round(__addr)     round_page_kernel(__addr)
	#define dyld_page_size              vm_kernel_page_size
#else
	#define dyld_page_trunc(__addr)     (__addr & (-4096))
	#define dyld_page_round(__addr)     ((__addr + 4095) & (-4096))
	#define dyld_page_size              4096
#endif


#define DYLD_PACKED_VERSION(major, minor, tiny) ((((major) & 0xffff) << 16) | (((minor) & 0xff) << 8) | ((tiny) & 0xff))

struct ProgramVars
{
	const void*		mh;
	int*			NXArgcPtr;
	const char***	NXArgvPtr;
	const char***	environPtr;
	const char**	__prognamePtr;
};



enum dyld_image_states
{
	dyld_image_state_mapped					= 10,		// No batch notification for this
	dyld_image_state_dependents_mapped		= 20,		// Only batch notification for this
	dyld_image_state_rebased				= 30,
	dyld_image_state_bound					= 40,
	dyld_image_state_dependents_initialized	= 45,		// Only single notification for this
	dyld_image_state_initialized			= 50,
	dyld_image_state_terminated				= 60		// Only single notification for this
};
typedef const char* (*dyld_image_state_change_handler)(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[]);

//
// ImageLoader is an abstract base class.  To support loading a particular executable
// file format, you make a concrete subclass of ImageLoader.
//
// For each executable file (dynamic shared object) in use, an ImageLoader is instantiated.
//
// The ImageLoader base class does the work of linking together images, but it knows nothing
// about any particular file format.
//
//
class ImageLoader {
public:

	typedef uint32_t DefinitionFlags;
	static const DefinitionFlags kNoDefinitionOptions = 0;
	static const DefinitionFlags kWeakDefinition = 1;
	
	typedef uint32_t ReferenceFlags;
	static const ReferenceFlags kNoReferenceOptions = 0;
	static const ReferenceFlags kWeakReference = 1;
	static const ReferenceFlags kTentativeDefinition = 2;
	
	enum PrebindMode { kUseAllPrebinding, kUseSplitSegPrebinding, kUseAllButAppPredbinding, kUseNoPrebinding };
	enum BindingOptions { kBindingNone, kBindingLazyPointers, kBindingNeverSetLazyPointers };
	enum SharedRegionMode { kUseSharedRegion, kUsePrivateSharedRegion, kDontUseSharedRegion, kSharedRegionIsSharedCache };
	
	struct Symbol;  // abstact symbol

	struct MappedRegion {
		uintptr_t	address;
		size_t		size;
	};

	struct RPathChain {
		RPathChain(const RPathChain* n, std::vector<const char*>* p) : next(n), paths(p) {};
		const RPathChain*			next;
		std::vector<const char*>*	paths;
	};

	struct DOFInfo {
		void*				dof;
		const mach_header*	imageHeader;
		const char*			imageShortName;
	};

	struct DynamicReference {
		ImageLoader* from;
		ImageLoader* to;
	};
	
	struct InitializerTimingList
	{
		uintptr_t	count;
		struct {
			const char*		shortName;
			uint64_t		initTime;
		}			images[1];

		void addTime(const char* name, uint64_t time);
	};

	typedef void (^CoalesceNotifier)(const Symbol* implSym, const ImageLoader* implIn, const mach_header* implMh);

    struct HashCString {
        static size_t hash(const char* v);
    };

    struct EqualCString {
        static bool equal(const char* s1, const char* s2);
    };
	
	struct LinkContext {
		ImageLoader*	(*loadLibrary)(const char* libraryName, bool search, const char* origin, const RPathChain* rpaths, unsigned& cacheIndex);
		void			(*terminationRecorder)(ImageLoader* image);
		bool			(*flatExportFinder)(const char* name, const Symbol** sym, const ImageLoader** image);
		bool			(*coalescedExportFinder)(const char* name, const Symbol** sym, const ImageLoader** image, CoalesceNotifier);
		unsigned int	(*getCoalescedImages)(ImageLoader* images[], unsigned imageIndex[]);
		void			(*undefinedHandler)(const char* name);
		MappedRegion*	(*getAllMappedRegions)(MappedRegion*);
		void *			(*bindingHandler)(const char *, const char *, void *);
		void			(*notifySingle)(dyld_image_states, const ImageLoader* image, InitializerTimingList*);
		void			(*notifyBatch)(dyld_image_states state, bool preflightOnly);
		void			(*removeImage)(ImageLoader* image);
		void			(*registerDOFs)(const std::vector<DOFInfo>& dofs);
		void			(*clearAllDepths)();
		void			(*printAllDepths)();
		unsigned int	(*imageCount)();
		void			(*setNewProgramVars)(const ProgramVars&);
		bool			(*inSharedCache)(const char* path);
		void			(*setErrorStrings)(unsigned errorCode, const char* errorClientOfDylibPath,
										const char* errorTargetDylibPath, const char* errorSymbol);
		ImageLoader*	(*findImageContainingAddress)(const void* addr);
		void			(*addDynamicReference)(ImageLoader* from, ImageLoader* to);
#if SUPPORT_ACCELERATE_TABLES
		void			(*notifySingleFromCache)(dyld_image_states, const mach_header* mh, const char* path);
		dyld_image_state_change_handler (*getPreInitNotifyHandler)(unsigned index);
		dyld_image_state_change_handler (*getBoundBatchHandler)(unsigned index);
#endif
		
#if SUPPORT_OLD_CRT_INITIALIZATION
		void			(*setRunInitialzersOldWay)();
#endif
		BindingOptions	bindingOptions;
		int				argc;
		const char**	argv;
		const char**	envp;
		const char**	apple;
		const char*		progname;
		ProgramVars		programVars;
		ImageLoader*	mainExecutable;
		const char* const * imageSuffix;
#if SUPPORT_ROOT_PATH
		const char**	rootPaths;
#endif
		const DyldSharedCache*  dyldCache;
		const dyld_interpose_tuple*	dynamicInterposeArray;
		size_t			dynamicInterposeCount;
		PrebindMode		prebindUsage;
		SharedRegionMode sharedRegionMode;
		mutable dyld3::Map<const char*, std::pair<const ImageLoader*, uintptr_t>, HashCString, EqualCString> weakDefMap;
		mutable bool	weakDefMapInitialized = false;
		mutable bool	weakDefMapProcessedLaunchDefs = false;
		mutable bool	useNewWeakBind = false;
		bool			dyldLoadedAtSameAddressNeededBySharedCache;
		bool			strictMachORequired;
		bool			allowAtPaths;
		bool			allowEnvVarsPrint;
		bool			allowEnvVarsPath;
		bool			allowEnvVarsSharedCache;
		bool			allowClassicFallbackPaths;
		bool			allowInsertFailures;
		bool			mainExecutableCodeSigned;
		bool			prebinding;
		bool			bindFlat;
		bool			linkingMainExecutable;
		bool			startedInitializingMainExecutable;
#if __MAC_OS_X_VERSION_MIN_REQUIRED
		bool			iOSonMac;
		bool			driverKit;
#endif
		bool			verboseOpts;
		bool			verboseEnv;
		bool			verboseLoading;
		bool			verboseMapping;
		bool			verboseRebase;
		bool			verboseBind;
		bool			verboseWeakBind;
		bool			verboseInit;
		bool			verboseDOF;
		bool			verbosePrebinding;
		bool			verboseCoreSymbolication;
		bool			verboseWarnings;
		bool			verboseRPaths;
		bool			verboseInterposing;
		bool			verboseCodeSignatures;
	};
	
	struct CoalIterator
	{
		ImageLoader*	image;
		const char*		symbolName;
		unsigned int	loadOrder;
		bool			weakSymbol;
		bool			symbolMatches;
		bool			done;
		// the following are private to the ImageLoader subclass
		uintptr_t		curIndex;
		uintptr_t		endIndex;
		uintptr_t		address;
		uintptr_t		type;
		uintptr_t		addend;
		uintptr_t		imageIndex;
	};
	
	virtual	void			initializeCoalIterator(CoalIterator&, unsigned int loadOrder, unsigned imageIndex) = 0;
	virtual	bool			incrementCoalIterator(CoalIterator&) = 0;
	virtual	uintptr_t		getAddressCoalIterator(CoalIterator&, const LinkContext& context) = 0;
	virtual	void			updateUsesCoalIterator(CoalIterator&, uintptr_t newAddr, ImageLoader* target, unsigned targetIndex, const LinkContext& context) = 0;
	
	struct UninitedUpwards
	{
		uintptr_t	 						 count;
		std::pair<ImageLoader*, const char*> imagesAndPaths[1];
	};

	
										// constructor is protected, but anyone can delete an image
	virtual								~ImageLoader();
	
										// link() takes a newly instantiated ImageLoader and does all 
										// fixups needed to make it usable by the process
	void								link(const LinkContext& context, bool forceLazysBound, bool preflight, bool neverUnload, const RPathChain& loaderRPaths, const char* imagePath);
	
										// runInitializers() is normally called in link() but the main executable must 
										// run crt code before initializers
	void								runInitializers(const LinkContext& context, InitializerTimingList& timingInfo);
	
										// called after link() forces all lazy pointers to be bound
	void								bindAllLazyPointers(const LinkContext& context, bool recursive);
	
										// used by dyld to see if a requested library is already loaded (might be symlink)
	bool								statMatch(const struct stat& stat_buf) const;

										// get short name of this image
	const char*							getShortName() const;

										// returns leaf name
	static const char*					shortName(const char* fullName);

										// get path used to load this image, not necessarily the "real" path
	const char*							getPath() const { return fPath; }

	uint32_t							getPathHash() const { return fPathHash; }

										// get the "real" path for this image (e.g. no @rpath)
	const char*							getRealPath() const;

										// get path this image is intended to be placed on disk or NULL if no preferred install location
	virtual const char*					getInstallPath() const = 0;

										// image was loaded with NSADDIMAGE_OPTION_MATCH_FILENAME_BY_INSTALLNAME and all clients are looking for install path
	bool								matchInstallPath() const;
	void								setMatchInstallPath(bool);
	
										// mark that this image's exported symbols should be ignored when linking other images (e.g. RTLD_LOCAL)
	void								setHideExports(bool hide = true);
	
										// check if this image's exported symbols should be ignored when linking other images 
	bool								hasHiddenExports() const;
	
										// checks if this image is already linked into the process
	bool								isLinked() const;
	
										// even if image is deleted, leave segments mapped in
	void								setLeaveMapped();
	
										// even if image is deleted, leave segments mapped in
	bool								leaveMapped() { return fLeaveMapped; }

										// image resides in dyld shared cache
	virtual bool						inSharedCache() const { return false; };

										// checks if the specifed address is within one of this image's segments
	virtual bool						containsAddress(const void* addr) const;

										// checks if the specifed symbol is within this image's symbol table
	virtual bool						containsSymbol(const void* addr) const = 0;

										// checks if the specifed address range overlaps any of this image's segments
	virtual bool						overlapsWithAddressRange(const void* start, const void* end) const;

										// adds to list of ranges of memory mapped in
	void								getMappedRegions(MappedRegion*& region) const;

										// st_mtime from stat() on file
	time_t								lastModified() const;

										// only valid for main executables, returns a pointer its entry point from LC_MAIN
	virtual void*						getEntryFromLC_MAIN() const = 0;
	
										// only valid for main executables, returns a pointer its main from LC_UNIXTHREAD
	virtual void*						getEntryFromLC_UNIXTHREAD() const = 0;
	
										// dyld API's require each image to have an associated mach_header
	virtual const struct mach_header*   machHeader() const = 0;
	
										// dyld API's require each image to have a slide (actual load address minus preferred load address)
	virtual uintptr_t					getSlide() const = 0;
	
										// last address mapped by image
	virtual const void*					getEnd() const = 0;
	
										// image has exports that participate in runtime coalescing
	virtual bool						hasCoalescedExports() const = 0;

										// search symbol table of definitions in this image for requested name
	virtual bool						findExportedSymbolAddress(const LinkContext& context, const char* symbolName,
																const ImageLoader* requestorImage, int requestorOrdinalOfDef,
																bool runResolver, const ImageLoader** foundIn, uintptr_t* address) const;

										// search symbol table of definitions in this image for requested name
	virtual const Symbol*				findExportedSymbol(const char* name, bool searchReExports, const char* thisPath, const ImageLoader** foundIn) const = 0;
	
										// search symbol table of definitions in this image for requested name
	virtual const Symbol*				findExportedSymbol(const char* name, bool searchReExports, const ImageLoader** foundIn) const {
											return findExportedSymbol(name, searchReExports, this->getPath(), foundIn);
										}
	
										// gets address of implementation (code) of the specified exported symbol
	virtual uintptr_t					getExportedSymbolAddress(const Symbol* sym, const LinkContext& context, 
													const ImageLoader* requestor=NULL, bool runResolver=false, const char* symbolName=NULL) const = 0;
	
										// gets attributes of the specified exported symbol
	virtual DefinitionFlags				getExportedSymbolInfo(const Symbol* sym) const = 0;
	
										// gets name of the specified exported symbol
	virtual const char*					getExportedSymbolName(const Symbol* sym) const = 0;
	
										// gets how many symbols are exported by this image
	virtual uint32_t					getExportedSymbolCount() const = 0;
			
										// gets the i'th exported symbol
	virtual const Symbol*				getIndexedExportedSymbol(uint32_t index) const = 0;
			
										// find exported symbol as if imported by this image
										// used by RTLD_NEXT
	virtual const Symbol*				findExportedSymbolInDependentImages(const char* name, const LinkContext& context, const ImageLoader** foundIn) const;
	
										// find exported symbol as if imported by this image
										// used by RTLD_SELF
	virtual const Symbol*				findExportedSymbolInImageOrDependentImages(const char* name, const LinkContext& context, const ImageLoader** foundIn) const;
	
										// gets how many symbols are imported by this image
	virtual uint32_t					getImportedSymbolCount() const = 0;
	
										// gets the i'th imported symbol
	virtual const Symbol*				getIndexedImportedSymbol(uint32_t index) const = 0;
			
										// gets attributes of the specified imported symbol
	virtual ReferenceFlags				getImportedSymbolInfo(const Symbol* sym) const = 0;
			
										// gets name of the specified imported symbol
	virtual const char*					getImportedSymbolName(const Symbol* sym) const = 0;
			
										// find the closest symbol before addr
	virtual const char*					findClosestSymbol(const void* addr, const void** closestAddr) const = 0;
	
										// for use with accelerator tables
	virtual const char*					getIndexedPath(unsigned) const { return getPath(); }
	virtual const char*					getIndexedShortName(unsigned) const { return getShortName(); }

										// checks if this image is a bundle and can be loaded but not linked
	virtual bool						isBundle() const = 0;
	
										// checks if this image is a dylib 
	virtual bool						isDylib() const = 0;
	
										// checks if this image is a main executable 
	virtual bool						isExecutable() const = 0;
	
										// checks if this image is a main executable 
	virtual bool						isPositionIndependentExecutable() const = 0;
	
										// only for main executable
	virtual bool						forceFlat() const = 0;
	
										// called at runtime when a lazily bound function is first called
	virtual uintptr_t					doBindLazySymbol(uintptr_t* lazyPointer, const LinkContext& context) = 0;
	
										// called at runtime when a fast lazily bound function is first called
	virtual uintptr_t					doBindFastLazySymbol(uint32_t lazyBindingInfoOffset, const LinkContext& context,
															void (*lock)(), void (*unlock)()) = 0;

										// calls termination routines (e.g. C++ static destructors for image)
	virtual void						doTermination(const LinkContext& context) = 0;
					
										// return if this image has initialization routines
	virtual bool						needsInitialization() = 0;
			
										// return if this image has specified section and set start and length
	virtual bool						getSectionContent(const char* segmentName, const char* sectionName, void** start, size_t* length) = 0;

										// fills in info about __eh_frame and __unwind_info sections
	virtual void						getUnwindInfo(dyld_unwind_sections* info) = 0;

										// given a pointer into an image, find which segment and section it is in
	virtual const struct macho_section* findSection(const void* imageInterior) const = 0;

										// given a pointer into an image, find which segment and section it is in
	virtual bool						findSection(const void* imageInterior, const char** segmentName, const char** sectionName, size_t* sectionOffset) = 0;
	
										// the image supports being prebound
	virtual bool						isPrebindable() const = 0;
	
										// the image is prebindable and its prebinding is valid
	virtual bool						usablePrebinding(const LinkContext& context) const = 0;
	
										// add all RPATH paths this image contains
	virtual	void						getRPaths(const LinkContext& context, std::vector<const char*>&) const = 0;
	
										// image has or uses weak definitions that need runtime coalescing
	virtual bool						participatesInCoalescing() const = 0;
		
										// if image has a UUID, copy into parameter and return true
	virtual	bool						getUUID(uuid_t) const = 0;

										// dynamic interpose values onto this image
	virtual void						dynamicInterpose(const LinkContext& context) = 0;

										// record interposing for any late binding
	void								addDynamicInterposingTuples(const struct dyld_interpose_tuple array[], size_t count);
		
	virtual const char*					libPath(unsigned int) const = 0;

										// Image has objc sections, so information objc about when it comes and goes
	virtual	bool						notifyObjC() const { return false; }

	virtual bool						overridesCachedDylib(uint32_t& num) const { return false; }
	virtual void						setOverridesCachedDylib(uint32_t num) { }


//
// A segment is a chunk of an executable file that is mapped into memory.  
//
	virtual unsigned int				segmentCount() const = 0;
	virtual const char*					segName(unsigned int) const = 0;
	virtual uintptr_t					segSize(unsigned int) const = 0;
	virtual uintptr_t					segFileSize(unsigned int) const = 0;
	virtual bool						segHasTrailingZeroFill(unsigned int) = 0;
	virtual uintptr_t					segFileOffset(unsigned int) const = 0;
	virtual bool						segReadable(unsigned int) const = 0;
	virtual bool						segWriteable(unsigned int) const = 0;
	virtual bool						segExecutable(unsigned int) const = 0;
	virtual bool						segUnaccessible(unsigned int) const = 0;
	virtual bool						segHasPreferredLoadAddress(unsigned int) const = 0;
	virtual uintptr_t					segPreferredLoadAddress(unsigned int) const = 0;
	virtual uintptr_t					segActualLoadAddress(unsigned int) const = 0;
	virtual uintptr_t					segActualEndAddress(unsigned int) const = 0;

	
										// info from LC_VERSION_MIN_MACOSX or LC_VERSION_MIN_IPHONEOS
	virtual uint32_t					sdkVersion() const = 0;
	virtual uint32_t					minOSVersion() const = 0;
	
										// if the image contains interposing functions, register them
	virtual void						registerInterposing(const LinkContext& context) = 0;

	virtual bool						usesChainedFixups() const { return false; }

	virtual void						makeDataReadOnly() const {}
	
										// when resolving symbols look in subImage if symbol can't be found
	void								reExport(ImageLoader* subImage);

	virtual void						recursiveBind(const LinkContext& context, bool forceLazysBound, bool neverUnload);
	void								recursiveBindWithAccounting(const LinkContext& context, bool forceLazysBound, bool neverUnload);
	void								recursiveRebaseWithAccounting(const LinkContext& context);
	void								weakBind(const LinkContext& context);

	void								applyInterposing(const LinkContext& context);

	dyld_image_states					getState() { return (dyld_image_states)fState; }

	ino_t								getInode() const { return fInode; }
    dev_t                               getDevice() const { return fDevice; }

										// used to sort images bottom-up
	int									compare(const ImageLoader* right) const;
	
	void								incrementDlopenReferenceCount() { ++fDlopenReferenceCount; }

	bool								decrementDlopenReferenceCount();
	
	void								printReferenceCounts();

	uint32_t							dlopenCount() const { return fDlopenReferenceCount; }

	void								setCanUnload() { fNeverUnload = false; fLeaveMapped = false; }

	bool								neverUnload() const { return fNeverUnload; }

	void								setNeverUnload() { fNeverUnload = true; fLeaveMapped = true; }
	void								setNeverUnloadRecursive();
	
	bool								isReferencedDownward() { return fIsReferencedDownward; }

	virtual void						recursiveMakeDataReadOnly(const LinkContext& context);

	virtual uintptr_t					resolveWeak(const LinkContext& context, const char* symbolName, bool weak_import, bool runResolver,
													const ImageLoader** foundIn) { return 0; } 

										// triggered by DYLD_PRINT_STATISTICS to write info on work done and how fast
	static void							printStatistics(unsigned int imageCount, const InitializerTimingList& timingInfo);
	static void							printStatisticsDetails(unsigned int imageCount, const InitializerTimingList& timingInfo);

										// used with DYLD_IMAGE_SUFFIX
	static void							addSuffix(const char* path, const char* suffix, char* result);
	
	static uint32_t						hash(const char*);
	
	static const uint8_t*				trieWalk(const uint8_t* start, const uint8_t* end, const char* stringToFind);

										// used instead of directly deleting image
	static void							deleteImage(ImageLoader*);

	static bool							haveInterposingTuples() { return !fgInterposingTuples.empty(); }
	static void							clearInterposingTuples() { fgInterposingTuples.clear(); }

	static void							applyInterposingToDyldCache(const LinkContext& context);

			bool						dependsOn(ImageLoader* image);
			
 			void						setPath(const char* path);
			void						setPaths(const char* path, const char* realPath);
			void						setPathUnowned(const char* path);
						
			void						clearDepth() { fDepth = 0; }
			int							getDepth() { return fDepth; }
			
			void						setBeingRemoved() { fBeingRemoved = true; }
			bool						isBeingRemoved() const { return fBeingRemoved; }
			
			void						markNotUsed() { fMarkedInUse = false; }
			void						markedUsedRecursive(const std::vector<DynamicReference>&);
			bool						isMarkedInUse() const	{ return fMarkedInUse; }

			void						setAddFuncNotified() { fAddFuncNotified = true; }
			bool						addFuncNotified() const { return fAddFuncNotified; }
	
			void						setObjCMappedNotified() { fObjCMappedNotified = true; }
			bool						objCMappedNotified() const { return fObjCMappedNotified; }

	struct InterposeTuple { 
		uintptr_t		replacement; 
		ImageLoader*	neverImage;			// don't apply replacement to this image
		ImageLoader*	onlyImage;			// only apply replacement to this image
		uintptr_t		replacee; 
	};

	static uintptr_t read_uleb128(const uint8_t*& p, const uint8_t* end);
	static intptr_t read_sleb128(const uint8_t*& p, const uint8_t* end);

	void			vmAccountingSetSuspended(const LinkContext& context, bool suspend);

protected:			
	// abstract base class so all constructors protected
					ImageLoader(const char* path, unsigned int libCount); 
					ImageLoader(const ImageLoader&);
	void			operator=(const ImageLoader&);
	void			operator delete(void* image) throw() { ::free(image); }
	

	struct LibraryInfo {
		uint32_t		checksum;
		uint32_t		minVersion;
		uint32_t		maxVersion;
	};

	struct DependentLibrary {
		ImageLoader*	image;
		uint32_t		required : 1,
						checksumMatches : 1,
						isReExported : 1,
						isSubFramework : 1;
	};
	
	struct DependentLibraryInfo {
		const char*			name;
		LibraryInfo			info;
		bool				required;
		bool				reExported;
		bool				upward;
	};


	typedef void (*Initializer)(int argc, const char* argv[], const char* envp[], const char* apple[], const ProgramVars* vars);
	typedef void (*Terminator)(void);
	


	unsigned int			libraryCount() const { return fLibraryCount; }
	virtual ImageLoader*	libImage(unsigned int) const = 0;
	virtual bool			libReExported(unsigned int) const = 0;
	virtual bool			libIsUpward(unsigned int) const = 0;
	virtual void			setLibImage(unsigned int, ImageLoader*, bool, bool) = 0;

						// To link() an image, its dependent libraries are loaded, it is rebased, bound, and initialized.
						// These methods do the above, exactly once, and it the right order
	virtual void		recursiveLoadLibraries(const LinkContext& context, bool preflightOnly, const RPathChain& loaderRPaths, const char* loadPath);
	virtual unsigned 	recursiveUpdateDepth(unsigned int maxDepth);
	virtual void		recursiveRebase(const LinkContext& context);
	virtual void		recursiveApplyInterposing(const LinkContext& context);
	virtual void		recursiveGetDOFSections(const LinkContext& context, std::vector<DOFInfo>& dofs);
	virtual void		recursiveInitialization(const LinkContext& context, mach_port_t this_thread, const char* pathToInitialize,
												ImageLoader::InitializerTimingList&, ImageLoader::UninitedUpwards&);

								// fill in information about dependent libraries (array length is fLibraryCount)
	virtual void				doGetDependentLibraries(DependentLibraryInfo libs[]) = 0;
	
								// called on images that are libraries, returns info about itself
	virtual LibraryInfo			doGetLibraryInfo(const LibraryInfo& requestorInfo) = 0;
	
								// do any fix ups in this image that depend only on the load address of the image
	virtual void				doRebase(const LinkContext& context) = 0;
	
								// do any symbolic fix ups in this image
	virtual void				doBind(const LinkContext& context, bool forceLazysBound) = 0;
	
								// called later via API to force all lazy pointer to be bound
	virtual void				doBindJustLazies(const LinkContext& context) = 0;
	
								// if image has any dtrace DOF sections, append them to list to be registered
	virtual void				doGetDOFSections(const LinkContext& context, std::vector<DOFInfo>& dofs) = 0;
	
								// do interpose
	virtual void				doInterpose(const LinkContext& context) = 0;

								// run any initialization routines in this image
	virtual bool				doInitialization(const LinkContext& context) = 0;
	
								// return if this image has termination routines
	virtual bool				needsTermination() = 0;
	
								// support for runtimes in which segments don't have to maintain their relative positions
	virtual bool				segmentsMustSlideTogether() const = 0;	
	
								// built with PIC code and can load at any address
	virtual bool				segmentsCanSlide() const = 0;		

								// set how much all segments slide
	virtual void				setSlide(intptr_t slide) = 0;		

								// returns if all dependent libraries checksum's were as expected and none slide
			bool				allDependentLibrariesAsWhenPreBound() const;

								// in mach-o a child tells it parent to re-export, instead of the other way around...
	virtual	bool				isSubframeworkOf(const LinkContext& context, const ImageLoader* image) const = 0;

								// in mach-o a parent library knows name of sub libraries it re-exports..
	virtual	bool				hasSubLibrary(const LinkContext& context, const ImageLoader* child) const  = 0;

	virtual bool				weakSymbolsBound(unsigned index) const { return fWeakSymbolsBound; }
	virtual void				setWeakSymbolsBound(unsigned index) { fWeakSymbolsBound = true; }

								// set fState to dyld_image_state_memory_mapped
	void						setMapped(const LinkContext& context);
		
	void						setFileInfo(dev_t device, ino_t inode, time_t modDate);

	void						setDepth(uint16_t depth) { fDepth = depth; }

	static uintptr_t			interposedAddress(const LinkContext& context, uintptr_t address, const ImageLoader* notInImage, const ImageLoader* onlyInImage=NULL);
	
	static uintptr_t			fgNextPIEDylibAddress;
	static uint32_t				fgImagesWithUsedPrebinding;
	static uint32_t				fgImagesUsedFromSharedCache;
	static uint32_t				fgImagesHasWeakDefinitions;
	static uint32_t				fgImagesRequiringCoalescing;
	static uint32_t				fgTotalRebaseFixups;
	static uint32_t				fgTotalBindFixups;
	static uint32_t				fgTotalBindSymbolsResolved;
	static uint32_t				fgTotalBindImageSearches;
	static uint32_t				fgTotalLazyBindFixups;
	static uint32_t				fgTotalPossibleLazyBindFixups;
	static uint32_t				fgTotalSegmentsMapped;
	static uint32_t				fgSymbolTrieSearchs;
	static uint64_t				fgTotalBytesMapped;
	static uint64_t				fgTotalLoadLibrariesTime;
public:
	static uint64_t				fgTotalObjCSetupTime;
	static uint64_t				fgTotalDebuggerPausedTime;
	static uint64_t				fgTotalRebindCacheTime;
	static uint64_t				fgTotalRebaseTime;
	static uint64_t				fgTotalBindTime;
	static uint64_t				fgTotalWeakBindTime;
	static uint64_t				fgTotalDOF;
	static uint64_t				fgTotalInitTime;

protected:
	static std::vector<InterposeTuple>	fgInterposingTuples;
	
	const char*					fPath;
	const char*					fRealPath;
	dev_t						fDevice;
	ino_t						fInode;
	time_t						fLastModified;
	uint32_t					fPathHash;
	uint32_t					fDlopenReferenceCount;	// count of how many dlopens have been done on this image

	struct recursive_lock {
						recursive_lock(mach_port_t t) : thread(t), count(0) {}
		mach_port_t		thread;
		int				count;
	};
	void						recursiveSpinLock(recursive_lock&);
	void						recursiveSpinUnLock();

private:
	const ImageLoader::Symbol*	findExportedSymbolInDependentImagesExcept(const char* name, const ImageLoader** dsiStart, 
										const ImageLoader**& dsiCur, const ImageLoader** dsiEnd, const ImageLoader** foundIn) const;

	void						processInitializers(const LinkContext& context, mach_port_t this_thread,
													InitializerTimingList& timingInfo, ImageLoader::UninitedUpwards& ups);

	void						weakBindOld(const LinkContext& context);


	recursive_lock*				fInitializerRecursiveLock;
	union {
		struct {
			uint16_t					fLoadOrder;
			uint16_t					fDepth : 15,
										fObjCMappedNotified : 1;
			uint32_t					fState : 8,
										fLibraryCount : 9,
										fMadeReadOnly : 1,
										fAllLibraryChecksumsAndLoadAddressesMatch : 1,
										fLeaveMapped : 1,		// when unloaded, leave image mapped in cause some other code may have pointers into it
										fNeverUnload : 1,		// image was statically loaded by main executable
										fHideSymbols : 1,		// ignore this image's exported symbols when linking other images
										fMatchByInstallName : 1,// look at image's install-path not its load path
										fInterposed : 1,
										fRegisteredDOF : 1,
										fAllLazyPointersBound : 1,
										fMarkedInUse : 1,
										fBeingRemoved : 1,
										fAddFuncNotified : 1,
										fPathOwnedByImage : 1,
										fIsReferencedDownward : 1,
										fWeakSymbolsBound : 1;
		};
		uint64_t 						sizeOfData;
	};
	static_assert(sizeof(sizeOfData) == 8, "Bad data size");

	static uint16_t				fgLoadOrdinal;
};


VECTOR_NEVER_DESTRUCTED_EXTERN(ImageLoader::InterposeTuple);


#endif


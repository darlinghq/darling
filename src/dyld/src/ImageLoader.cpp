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

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <mach/mach.h>
#include <mach-o/fat.h> 
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/sysctl.h>
#include <libkern/OSAtomic.h>
#include <string_view>

#include <atomic>

#include "Tracing.h"

#include "ImageLoader.h"


uint32_t								ImageLoader::fgImagesUsedFromSharedCache = 0;
uint32_t								ImageLoader::fgImagesWithUsedPrebinding = 0;
uint32_t								ImageLoader::fgImagesRequiringCoalescing = 0;
uint32_t								ImageLoader::fgImagesHasWeakDefinitions = 0;
uint32_t								ImageLoader::fgTotalRebaseFixups = 0;
uint32_t								ImageLoader::fgTotalBindFixups = 0;
uint32_t								ImageLoader::fgTotalBindSymbolsResolved = 0;
uint32_t								ImageLoader::fgTotalBindImageSearches = 0;
uint32_t								ImageLoader::fgTotalLazyBindFixups = 0;
uint32_t								ImageLoader::fgTotalPossibleLazyBindFixups = 0;
uint32_t								ImageLoader::fgTotalSegmentsMapped = 0;
uint64_t								ImageLoader::fgTotalBytesMapped = 0;
uint64_t								ImageLoader::fgTotalLoadLibrariesTime;
uint64_t								ImageLoader::fgTotalObjCSetupTime = 0;
uint64_t								ImageLoader::fgTotalDebuggerPausedTime = 0;
uint64_t								ImageLoader::fgTotalRebindCacheTime = 0;
uint64_t								ImageLoader::fgTotalRebaseTime;
uint64_t								ImageLoader::fgTotalBindTime;
uint64_t								ImageLoader::fgTotalWeakBindTime;
uint64_t								ImageLoader::fgTotalDOF;
uint64_t								ImageLoader::fgTotalInitTime;
uint16_t								ImageLoader::fgLoadOrdinal = 0;
uint32_t								ImageLoader::fgSymbolTrieSearchs = 0;
std::vector<ImageLoader::InterposeTuple>ImageLoader::fgInterposingTuples;
uintptr_t								ImageLoader::fgNextPIEDylibAddress = 0;



ImageLoader::ImageLoader(const char* path, unsigned int libCount)
	: fPath(path), fRealPath(NULL), fDevice(0), fInode(0), fLastModified(0),
	fPathHash(0), fDlopenReferenceCount(0), fInitializerRecursiveLock(NULL), 
	fLoadOrder(fgLoadOrdinal++), fDepth(0), fObjCMappedNotified(false), fState(0), fLibraryCount(libCount),
	fMadeReadOnly(false), fAllLibraryChecksumsAndLoadAddressesMatch(false), fLeaveMapped(false), fNeverUnload(false),
	fHideSymbols(false), fMatchByInstallName(false),
	fInterposed(false), fRegisteredDOF(false), fAllLazyPointersBound(false), 
    fBeingRemoved(false), fAddFuncNotified(false),
	fPathOwnedByImage(false), fIsReferencedDownward(false), 
	fWeakSymbolsBound(false)
{
	if ( fPath != NULL )
		fPathHash = hash(fPath);
	if ( libCount > 512 )
		dyld::throwf("too many dependent dylibs in %s", path);
}


void ImageLoader::deleteImage(ImageLoader* image)
{
	delete image;
}


ImageLoader::~ImageLoader()
{
	if ( fRealPath != NULL ) 
		delete [] fRealPath;
	if ( fPathOwnedByImage && (fPath != NULL) ) 
		delete [] fPath;
}

void ImageLoader::setFileInfo(dev_t device, ino_t inode, time_t modDate)
{
	fDevice = device;
	fInode = inode;
	fLastModified = modDate;
}

void ImageLoader::setMapped(const LinkContext& context)
{
	fState = dyld_image_state_mapped;
	context.notifySingle(dyld_image_state_mapped, this, NULL);  // note: can throw exception
}

int ImageLoader::compare(const ImageLoader* right) const
{
	if ( this->fDepth == right->fDepth ) {
		if ( this->fLoadOrder == right->fLoadOrder )
			return 0;
		else if ( this->fLoadOrder < right->fLoadOrder )
			return -1;
		else
			return 1;
	}
	else {
		if ( this->fDepth < right->fDepth )
			return -1;
		else
			return 1;
	}
}

void ImageLoader::setPath(const char* path)
{
	if ( fPathOwnedByImage && (fPath != NULL) )
		delete [] fPath;
	fPath = new char[strlen(path)+1];
	strcpy((char*)fPath, path);
	fPathOwnedByImage = true;  // delete fPath when this image is destructed
	fPathHash = hash(fPath);
	if ( fRealPath != NULL ) {
		delete [] fRealPath;
		fRealPath = NULL;
	}
}

void ImageLoader::setPathUnowned(const char* path)
{
	if ( fPathOwnedByImage && (fPath != NULL) ) {
		delete [] fPath;
	}
	fPath = path;
	fPathOwnedByImage = false;
	fPathHash = hash(fPath);
}

void ImageLoader::setPaths(const char* path, const char* realPath)
{
	this->setPath(path);
	fRealPath = new char[strlen(realPath)+1];
	strcpy((char*)fRealPath, realPath);
}


const char* ImageLoader::getRealPath() const
{
	if ( fRealPath != NULL )
		return fRealPath;
	else
		return fPath;
}

uint32_t ImageLoader::hash(const char* path)
{
	// this does not need to be a great hash
	// it is just used to reduce the number of strcmp() calls
	// of existing images when loading a new image
	uint32_t h = 0;
	for (const char* s=path; *s != '\0'; ++s)
		h = h*5 + *s;
	return h;
}

bool ImageLoader::matchInstallPath() const
{
	return fMatchByInstallName;
}

void ImageLoader::setMatchInstallPath(bool match)
{
	fMatchByInstallName = match;
}

bool ImageLoader::statMatch(const struct stat& stat_buf) const
{
	return ( (this->fDevice == stat_buf.st_dev) && (this->fInode == stat_buf.st_ino) );	
}

const char* ImageLoader::shortName(const char* fullName)
{
	// try to return leaf name
	if ( fullName != NULL ) {
		const char* s = strrchr(fullName, '/');
		if ( s != NULL ) 
			return &s[1];
	}
	return fullName;
}

const char* ImageLoader::getShortName() const
{
	return shortName(fPath);
}

void ImageLoader::setLeaveMapped()
{
	fLeaveMapped = true;
}

void ImageLoader::setHideExports(bool hide)
{
	fHideSymbols = hide;
}

bool ImageLoader::hasHiddenExports() const
{
	return fHideSymbols;
}

bool ImageLoader::isLinked() const
{
	return (fState >= dyld_image_state_bound);
}

time_t ImageLoader::lastModified() const
{
	return fLastModified;
}

bool ImageLoader::containsAddress(const void* addr) const
{
	for(unsigned int i=0, e=segmentCount(); i < e; ++i) {
		const uint8_t* start = (const uint8_t*)segActualLoadAddress(i);
		const uint8_t* end = (const uint8_t*)segActualEndAddress(i);
		if ( (start <= addr) && (addr < end) && !segUnaccessible(i) )
			return true;
	}
	return false;
}

bool ImageLoader::overlapsWithAddressRange(const void* start, const void* end) const
{
	for(unsigned int i=0, e=segmentCount(); i < e; ++i) {
		const uint8_t* segStart = (const uint8_t*)segActualLoadAddress(i);
		const uint8_t* segEnd = (const uint8_t*)segActualEndAddress(i);
		if ( strcmp(segName(i), "__UNIXSTACK") == 0 ) {
			// __UNIXSTACK never slides.  This is the only place that cares
			// and checking for that segment name in segActualLoadAddress()
			// is too expensive.
			segStart -= getSlide();
			segEnd -= getSlide();
		}
		if ( (start <= segStart) && (segStart < end) )
			return true;
		if ( (start <= segEnd) && (segEnd < end) )
			return true;
		if ( (segStart < start) && (end < segEnd) )
			return true;
	}
	return false;
}

void ImageLoader::getMappedRegions(MappedRegion*& regions) const
{
	for(unsigned int i=0, e=segmentCount(); i < e; ++i) {
		MappedRegion region;
		region.address = segActualLoadAddress(i);
		region.size = segSize(i);
		*regions++ = region;
	}
}



bool ImageLoader::dependsOn(ImageLoader* image) {
	for(unsigned int i=0; i < libraryCount(); ++i) {
		if ( libImage(i) == image )
			return true;
	}
	return false;
}


static bool notInImgageList(const ImageLoader* image, const ImageLoader** dsiStart, const ImageLoader** dsiCur)
{
	for (const ImageLoader** p = dsiStart; p < dsiCur; ++p)
		if ( *p == image )
			return false;
	return true;
}

bool ImageLoader::findExportedSymbolAddress(const LinkContext& context, const char* symbolName,
											const ImageLoader* requestorImage, int requestorOrdinalOfDef,
											bool runResolver, const ImageLoader** foundIn, uintptr_t* address) const
{
	const Symbol* sym = this->findExportedSymbol(symbolName, true, foundIn);
	if ( sym != NULL ) {
		*address = (*foundIn)->getExportedSymbolAddress(sym, context, requestorImage, runResolver);
		return true;
	}
	return false;
}


// private method that handles circular dependencies by only search any image once
const ImageLoader::Symbol* ImageLoader::findExportedSymbolInDependentImagesExcept(const char* name, 
			const ImageLoader** dsiStart, const ImageLoader**& dsiCur, const ImageLoader** dsiEnd, const ImageLoader** foundIn) const
{
	const ImageLoader::Symbol* sym;
	// search self
	if ( notInImgageList(this, dsiStart, dsiCur) ) {
		sym = this->findExportedSymbol(name, false, this->getPath(), foundIn);
		if ( sym != NULL )
			return sym;
		*dsiCur++ = this;
	}

	// search directly dependent libraries
	for(unsigned int i=0; i < libraryCount(); ++i) {
		ImageLoader* dependentImage = libImage(i);
		if ( (dependentImage != NULL) && notInImgageList(dependentImage, dsiStart, dsiCur) ) {
			sym = dependentImage->findExportedSymbol(name, false, libPath(i), foundIn);
			if ( sym != NULL )
				return sym;
		}
	}
	
	// search indirectly dependent libraries
	for(unsigned int i=0; i < libraryCount(); ++i) {
		ImageLoader* dependentImage = libImage(i);
		if ( (dependentImage != NULL) && notInImgageList(dependentImage, dsiStart, dsiCur) ) {
			*dsiCur++ = dependentImage; 
			sym = dependentImage->findExportedSymbolInDependentImagesExcept(name, dsiStart, dsiCur, dsiEnd, foundIn);
			if ( sym != NULL )
				return sym;
		}
	}

	return NULL;
}


const ImageLoader::Symbol* ImageLoader::findExportedSymbolInDependentImages(const char* name, const LinkContext& context, const ImageLoader** foundIn) const
{
	unsigned int imageCount = context.imageCount()+2;
	const ImageLoader* dontSearchImages[imageCount];
	dontSearchImages[0] = this; // don't search this image
	const ImageLoader** cur = &dontSearchImages[1];
	return this->findExportedSymbolInDependentImagesExcept(name, &dontSearchImages[0], cur, &dontSearchImages[imageCount], foundIn);
}

const ImageLoader::Symbol* ImageLoader::findExportedSymbolInImageOrDependentImages(const char* name, const LinkContext& context, const ImageLoader** foundIn) const
{
	unsigned int imageCount = context.imageCount()+2;
	const ImageLoader* dontSearchImages[imageCount];
	const ImageLoader** cur = &dontSearchImages[0];
	return this->findExportedSymbolInDependentImagesExcept(name, &dontSearchImages[0], cur, &dontSearchImages[imageCount], foundIn);
}

// this is called by initializeMainExecutable() to interpose on the initial set of images
void ImageLoader::applyInterposing(const LinkContext& context)
{
	dyld3::ScopedTimer timer(DBG_DYLD_TIMING_APPLY_INTERPOSING, 0, 0, 0);
	if ( fgInterposingTuples.size() != 0 )
		this->recursiveApplyInterposing(context);
}


uintptr_t ImageLoader::interposedAddress(const LinkContext& context, uintptr_t address, const ImageLoader* inImage, const ImageLoader* onlyInImage)
{
	//dyld::log("interposedAddress(0x%08llX), tupleCount=%lu\n", (uint64_t)address, fgInterposingTuples.size());
	for (std::vector<InterposeTuple>::iterator it=fgInterposingTuples.begin(); it != fgInterposingTuples.end(); it++) {
		//dyld::log("    interposedAddress: replacee=0x%08llX, replacement=0x%08llX, neverImage=%p, onlyImage=%p, inImage=%p\n", 
		//				(uint64_t)it->replacee, (uint64_t)it->replacement,  it->neverImage, it->onlyImage, inImage);
		// replace all references to 'replacee' with 'replacement'
		if ( (address == it->replacee) && (inImage != it->neverImage) && ((it->onlyImage == NULL) || (inImage == it->onlyImage)) ) {
			if ( context.verboseInterposing ) {
				dyld::log("dyld interposing: replace 0x%lX with 0x%lX\n", it->replacee, it->replacement);
			}
			return it->replacement;
		}
	}
	return address;
}

void ImageLoader::applyInterposingToDyldCache(const LinkContext& context) {
	if (!context.dyldCache)
		return;
#if !__arm64e__ // until arm64e cache builder sets builtFromChainedFixups
	if (!context.dyldCache->header.builtFromChainedFixups)
		return;
#endif
	if (fgInterposingTuples.empty())
		return;
	// For each of the interposed addresses, see if any of them are in the shared cache.  If so, find
	// that image and apply its patch table to all uses.
	uintptr_t cacheStart = (uintptr_t)context.dyldCache;
	for (std::vector<InterposeTuple>::iterator it=fgInterposingTuples.begin(); it != fgInterposingTuples.end(); it++) {
		if ( context.verboseInterposing )
			dyld::log("dyld: interpose: Trying to interpose address 0x%08llx\n", (uint64_t)it->replacee);
		uint32_t imageIndex;
		uint32_t cacheOffsetOfReplacee = (uint32_t)(it->replacee - cacheStart);
		if (!context.dyldCache->addressInText(cacheOffsetOfReplacee, &imageIndex))
			continue;
		dyld3::closure::ImageNum imageInCache = imageIndex+1;
		if ( context.verboseInterposing )
			dyld::log("dyld: interpose: Found shared cache image %d for 0x%08llx\n", imageInCache, (uint64_t)it->replacee);
		context.dyldCache->forEachPatchableExport(imageIndex, ^(uint32_t cacheOffsetOfImpl, const char* exportName) {
			// Skip patching anything other than this symbol
			if (cacheOffsetOfImpl != cacheOffsetOfReplacee)
				return;
			if ( context.verboseInterposing ) {
				const dyld3::closure::Image* image = context.dyldCache->cachedDylibsImageArray()->imageForNum(imageInCache);
				dyld::log("dyld: interpose: Patching uses of symbol %s in shared cache binary at %s\n", exportName, image->path());
			}
			uintptr_t newLoc = it->replacement;
			context.dyldCache->forEachPatchableUseOfExport(imageIndex, cacheOffsetOfImpl, ^(dyld_cache_patchable_location patchLocation) {
				uintptr_t* loc = (uintptr_t*)(cacheStart+patchLocation.cacheOffset);
#if __has_feature(ptrauth_calls)
				if ( patchLocation.authenticated ) {
					dyld3::MachOLoaded::ChainedFixupPointerOnDisk ptr = *(dyld3::MachOLoaded::ChainedFixupPointerOnDisk*)loc;
					ptr.arm64e.authRebase.auth      = true;
					ptr.arm64e.authRebase.addrDiv   = patchLocation.usesAddressDiversity;
					ptr.arm64e.authRebase.diversity = patchLocation.discriminator;
					ptr.arm64e.authRebase.key       = patchLocation.key;
					*loc = ptr.arm64e.signPointer(loc, newLoc + DyldSharedCache::getAddend(patchLocation));
					if ( context.verboseInterposing )
						dyld::log("dyld: interpose: *%p = %p (JOP: diversity 0x%04X, addr-div=%d, key=%s)\n",
								  loc, (void*)*loc, patchLocation.discriminator, patchLocation.usesAddressDiversity, DyldSharedCache::keyName(patchLocation));
					return;
				}
#endif
				if ( context.verboseInterposing )
					dyld::log("dyld: interpose: *%p = 0x%0llX (dyld cache patch) to %s\n", loc, newLoc + DyldSharedCache::getAddend(patchLocation), exportName);
				*loc = newLoc + (uintptr_t)DyldSharedCache::getAddend(patchLocation);
			});
		});
	}
}

void ImageLoader::addDynamicInterposingTuples(const struct dyld_interpose_tuple array[], size_t count)
{
	for(size_t i=0; i < count; ++i) {
		ImageLoader::InterposeTuple tuple;
		tuple.replacement		= (uintptr_t)array[i].replacement;
		tuple.neverImage		= NULL;
		tuple.onlyImage		    = this;
		tuple.replacee			= (uintptr_t)array[i].replacee;
		// chain to any existing interpositions
		for (std::vector<InterposeTuple>::iterator it=fgInterposingTuples.begin(); it != fgInterposingTuples.end(); it++) {
			if ( (it->replacee == tuple.replacee) && (it->onlyImage == this) ) {
				tuple.replacee = it->replacement;
			}
		}
		ImageLoader::fgInterposingTuples.push_back(tuple);
	}
}

// <rdar://problem/29099600> dyld should tell the kernel when it is doing root fix-ups
void ImageLoader::vmAccountingSetSuspended(const LinkContext& context, bool suspend)
{
#if __arm__ || __arm64__
	static bool sVmAccountingSuspended = false;
	if ( suspend == sVmAccountingSuspended )
		return;
    if ( context.verboseBind )
        dyld::log("set vm.footprint_suspend=%d\n", suspend);
    int newValue = suspend ? 1 : 0;
    int oldValue = 0;
    size_t newlen = sizeof(newValue);
    size_t oldlen = sizeof(oldValue);
    int ret = sysctlbyname("vm.footprint_suspend", &oldValue, &oldlen, &newValue, newlen);
    if ( context.verboseBind && (ret != 0) )
		dyld::log("vm.footprint_suspend => %d, errno=%d\n", ret, errno);
	sVmAccountingSuspended = suspend;
#endif
}


void ImageLoader::link(const LinkContext& context, bool forceLazysBound, bool preflightOnly, bool neverUnload, const RPathChain& loaderRPaths, const char* imagePath)
{
	//dyld::log("ImageLoader::link(%s) refCount=%d, neverUnload=%d\n", imagePath, fDlopenReferenceCount, fNeverUnload);
	
	// clear error strings
	(*context.setErrorStrings)(0, NULL, NULL, NULL);

	uint64_t t0 = mach_absolute_time();
	this->recursiveLoadLibraries(context, preflightOnly, loaderRPaths, imagePath);
	context.notifyBatch(dyld_image_state_dependents_mapped, preflightOnly);

	// we only do the loading step for preflights
	if ( preflightOnly )
		return;

	uint64_t t1 = mach_absolute_time();
	context.clearAllDepths();
	this->recursiveUpdateDepth(context.imageCount());

	__block uint64_t t2, t3, t4, t5;
	{
		dyld3::ScopedTimer(DBG_DYLD_TIMING_APPLY_FIXUPS, 0, 0, 0);
		t2 = mach_absolute_time();
		this->recursiveRebaseWithAccounting(context);
		context.notifyBatch(dyld_image_state_rebased, false);

		t3 = mach_absolute_time();
		if ( !context.linkingMainExecutable )
			this->recursiveBindWithAccounting(context, forceLazysBound, neverUnload);

		t4 = mach_absolute_time();
		if ( !context.linkingMainExecutable )
			this->weakBind(context);
		t5 = mach_absolute_time();
	}

	// interpose any dynamically loaded images
	if ( !context.linkingMainExecutable && (fgInterposingTuples.size() != 0) ) {
		dyld3::ScopedTimer timer(DBG_DYLD_TIMING_APPLY_INTERPOSING, 0, 0, 0);
		this->recursiveApplyInterposing(context);
	}

	// now that all fixups are done, make __DATA_CONST segments read-only
	if ( !context.linkingMainExecutable )
		this->recursiveMakeDataReadOnly(context);

    if ( !context.linkingMainExecutable )
        context.notifyBatch(dyld_image_state_bound, false);
	uint64_t t6 = mach_absolute_time();

	if ( context.registerDOFs != NULL ) {
		std::vector<DOFInfo> dofs;
		this->recursiveGetDOFSections(context, dofs);
		context.registerDOFs(dofs);
	}
	uint64_t t7 = mach_absolute_time();

	// clear error strings
	(*context.setErrorStrings)(0, NULL, NULL, NULL);

	fgTotalLoadLibrariesTime += t1 - t0;
	fgTotalRebaseTime += t3 - t2;
	fgTotalBindTime += t4 - t3;
	fgTotalWeakBindTime += t5 - t4;
	fgTotalDOF += t7 - t6;
	
	// done with initial dylib loads
	fgNextPIEDylibAddress = 0;
}


void ImageLoader::printReferenceCounts()
{
	dyld::log("      dlopen=%d for %s\n", fDlopenReferenceCount, getPath() );
}


bool ImageLoader::decrementDlopenReferenceCount() 
{
	if ( fDlopenReferenceCount == 0 )
		return true;
	--fDlopenReferenceCount;
	return false;
}


// <rdar://problem/14412057> upward dylib initializers can be run too soon
// To handle dangling dylibs which are upward linked but not downward, all upward linked dylibs
// have their initialization postponed until after the recursion through downward dylibs
// has completed.
void ImageLoader::processInitializers(const LinkContext& context, mach_port_t thisThread,
									 InitializerTimingList& timingInfo, ImageLoader::UninitedUpwards& images)
{
	uint32_t maxImageCount = context.imageCount()+2;
	ImageLoader::UninitedUpwards upsBuffer[maxImageCount];
	ImageLoader::UninitedUpwards& ups = upsBuffer[0];
	ups.count = 0;
	// Calling recursive init on all images in images list, building a new list of
	// uninitialized upward dependencies.
	for (uintptr_t i=0; i < images.count; ++i) {
		images.imagesAndPaths[i].first->recursiveInitialization(context, thisThread, images.imagesAndPaths[i].second, timingInfo, ups);
	}
	// If any upward dependencies remain, init them.
	if ( ups.count > 0 )
		processInitializers(context, thisThread, timingInfo, ups);
}


void ImageLoader::runInitializers(const LinkContext& context, InitializerTimingList& timingInfo)
{
	uint64_t t1 = mach_absolute_time();
	mach_port_t thisThread = mach_thread_self();
	ImageLoader::UninitedUpwards up;
	up.count = 1;
	up.imagesAndPaths[0] = { this, this->getPath() };
	processInitializers(context, thisThread, timingInfo, up);
	context.notifyBatch(dyld_image_state_initialized, false);
	mach_port_deallocate(mach_task_self(), thisThread);
	uint64_t t2 = mach_absolute_time();
	fgTotalInitTime += (t2 - t1);
}


void ImageLoader::bindAllLazyPointers(const LinkContext& context, bool recursive)
{
	if ( ! fAllLazyPointersBound ) {
		fAllLazyPointersBound = true;

		if ( recursive ) {
			// bind lower level libraries first
			for(unsigned int i=0; i < libraryCount(); ++i) {
				ImageLoader* dependentImage = libImage(i);
				if ( dependentImage != NULL )
					dependentImage->bindAllLazyPointers(context, recursive);
			}
		}
		// bind lazies in this image
		this->doBindJustLazies(context);
	}
}


bool ImageLoader::allDependentLibrariesAsWhenPreBound() const
{
	return fAllLibraryChecksumsAndLoadAddressesMatch;
}


void ImageLoader::markedUsedRecursive(const std::vector<DynamicReference>& dynamicReferences)
{
	// already visited here
	if ( fMarkedInUse )
		return;
	fMarkedInUse = true;
	
	// clear mark on all statically dependent dylibs
	for(unsigned int i=0; i < libraryCount(); ++i) {
		ImageLoader* dependentImage = libImage(i);
		if ( dependentImage != NULL ) {
			dependentImage->markedUsedRecursive(dynamicReferences);
		}
	}
	
	// clear mark on all dynamically dependent dylibs
	for (std::vector<ImageLoader::DynamicReference>::const_iterator it=dynamicReferences.begin(); it != dynamicReferences.end(); ++it) {
		if ( it->from == this )
			it->to->markedUsedRecursive(dynamicReferences);
	}
	
}

unsigned int ImageLoader::recursiveUpdateDepth(unsigned int maxDepth)
{
	// the purpose of this phase is to make the images sortable such that 
	// in a sort list of images, every image that an image depends on
	// occurs in the list before it.
	if ( fDepth == 0 ) {
		// break cycles
		fDepth = maxDepth;
		
		// get depth of dependents
		unsigned int minDependentDepth = maxDepth;
		for(unsigned int i=0; i < libraryCount(); ++i) {
			ImageLoader* dependentImage = libImage(i);
			if ( (dependentImage != NULL) && !libIsUpward(i) ) {
				unsigned int d = dependentImage->recursiveUpdateDepth(maxDepth);
				if ( d < minDependentDepth )
					minDependentDepth = d;
			}
		}
	
		// make me less deep then all my dependents
		fDepth = minDependentDepth - 1;
	}
	
	return fDepth;
}


void ImageLoader::recursiveLoadLibraries(const LinkContext& context, bool preflightOnly, const RPathChain& loaderRPaths, const char* loadPath)
{
	if ( fState < dyld_image_state_dependents_mapped ) {
		// break cycles
		fState = dyld_image_state_dependents_mapped;
		
		// get list of libraries this image needs
		DependentLibraryInfo libraryInfos[fLibraryCount]; 
		this->doGetDependentLibraries(libraryInfos);
		
		// get list of rpaths that this image adds
		std::vector<const char*> rpathsFromThisImage;
		this->getRPaths(context, rpathsFromThisImage);
		const RPathChain thisRPaths(&loaderRPaths, &rpathsFromThisImage);
		
		// try to load each
		bool canUsePrelinkingInfo = true; 
		for(unsigned int i=0; i < fLibraryCount; ++i){
			ImageLoader* dependentLib;
			bool depLibReExported = false;
			DependentLibraryInfo& requiredLibInfo = libraryInfos[i];
			if ( preflightOnly && context.inSharedCache(requiredLibInfo.name) ) {
				// <rdar://problem/5910137> dlopen_preflight() on image in shared cache leaves it loaded but not objc initialized
				// in preflight mode, don't even load dylib that are in the shared cache because they will never be unloaded
				setLibImage(i, NULL, false, false);
				continue;
			}
			try {
				unsigned cacheIndex;
				dependentLib = context.loadLibrary(requiredLibInfo.name, true, this->getPath(), &thisRPaths, cacheIndex);
				if ( dependentLib == this ) {
					// found circular reference, perhaps DYLD_LIBARY_PATH is causing this rdar://problem/3684168 
					dependentLib = context.loadLibrary(requiredLibInfo.name, false, NULL, NULL, cacheIndex);
					if ( dependentLib != this )
						dyld::warn("DYLD_ setting caused circular dependency in %s\n", this->getPath());
				}
				if ( fNeverUnload )
					dependentLib->setNeverUnload();
				if ( requiredLibInfo.upward ) {
				}
				else { 
					dependentLib->fIsReferencedDownward = true;
				}
				LibraryInfo actualInfo = dependentLib->doGetLibraryInfo(requiredLibInfo.info);
				depLibReExported = requiredLibInfo.reExported;
				if ( ! depLibReExported ) {
					// for pre-10.5 binaries that did not use LC_REEXPORT_DYLIB
					depLibReExported = dependentLib->isSubframeworkOf(context, this) || this->hasSubLibrary(context, dependentLib);
				}
				// check found library version is compatible
				// <rdar://problem/89200806> 0xFFFFFFFF is wildcard that matches any version
				if ( (requiredLibInfo.info.minVersion != 0xFFFFFFFF) && (actualInfo.minVersion < requiredLibInfo.info.minVersion)
						&& ((dyld3::MachOFile*)(dependentLib->machHeader()))->enforceCompatVersion() ) {
					// record values for possible use by CrashReporter or Finder
					dyld::throwf("Incompatible library version: %s requires version %d.%d.%d or later, but %s provides version %d.%d.%d",
							this->getShortName(), requiredLibInfo.info.minVersion >> 16, (requiredLibInfo.info.minVersion >> 8) & 0xff, requiredLibInfo.info.minVersion & 0xff,
							dependentLib->getShortName(), actualInfo.minVersion >> 16, (actualInfo.minVersion >> 8) & 0xff, actualInfo.minVersion & 0xff);
				}
				// prebinding for this image disabled if any dependent library changed
				//if ( !depLibCheckSumsMatch )
				//	canUsePrelinkingInfo = false;
				// prebinding for this image disabled unless both this and dependent are in the shared cache
				if ( !dependentLib->inSharedCache() || !this->inSharedCache() )
					canUsePrelinkingInfo = false;
					
				//if ( context.verbosePrebinding ) {
				//	if ( !requiredLib.checksumMatches )
				//		fprintf(stderr, "dyld: checksum mismatch, (%u v %u) for %s referencing %s\n", 
				//			requiredLibInfo.info.checksum, actualInfo.checksum, this->getPath(), 	dependentLib->getPath());		
				//	if ( dependentLib->getSlide() != 0 )
				//		fprintf(stderr, "dyld: dependent library slid for %s referencing %s\n", this->getPath(), dependentLib->getPath());		
				//}
			}
			catch (const char* msg) {
				//if ( context.verbosePrebinding )
				//	fprintf(stderr, "dyld: exception during processing for %s referencing %s\n", this->getPath(), dependentLib->getPath());		
				if ( requiredLibInfo.required ) {
					fState = dyld_image_state_mapped;
					// record values for possible use by CrashReporter or Finder
					if ( strstr(msg, "Incompatible library version") != NULL )
						(*context.setErrorStrings)(DYLD_EXIT_REASON_DYLIB_WRONG_VERSION, this->getPath(), requiredLibInfo.name, NULL);
					else if ( strstr(msg, "architecture") != NULL )
						(*context.setErrorStrings)(DYLD_EXIT_REASON_DYLIB_WRONG_ARCH, this->getPath(), requiredLibInfo.name, NULL);
					else if ( strstr(msg, "file system sandbox") != NULL )
						(*context.setErrorStrings)(DYLD_EXIT_REASON_FILE_SYSTEM_SANDBOX, this->getPath(), requiredLibInfo.name, NULL);
					else if ( strstr(msg, "code signature") != NULL )
						(*context.setErrorStrings)(DYLD_EXIT_REASON_CODE_SIGNATURE, this->getPath(), requiredLibInfo.name, NULL);
					else if ( strstr(msg, "malformed") != NULL )
						(*context.setErrorStrings)(DYLD_EXIT_REASON_MALFORMED_MACHO, this->getPath(), requiredLibInfo.name, NULL);
					else
						(*context.setErrorStrings)(DYLD_EXIT_REASON_DYLIB_MISSING, this->getPath(), requiredLibInfo.name, NULL);
					const char* newMsg = dyld::mkstringf("Library not loaded: %s\n  Referenced from: %s\n  Reason: %s", requiredLibInfo.name, this->getRealPath(), msg);
					free((void*)msg); 	// our free() will do nothing if msg is a string literal
					throw newMsg;
				}
				free((void*)msg); 	// our free() will do nothing if msg is a string literal
				// ok if weak library not found
				dependentLib = NULL;
				canUsePrelinkingInfo = false;  // this disables all prebinding, we may want to just slam import vectors for this lib to zero
			}
			setLibImage(i, dependentLib, depLibReExported, requiredLibInfo.upward);
		}
		fAllLibraryChecksumsAndLoadAddressesMatch = canUsePrelinkingInfo;

		// tell each to load its dependents
		for(unsigned int i=0; i < libraryCount(); ++i) {
			ImageLoader* dependentImage = libImage(i);
			if ( dependentImage != NULL ) {	
				dependentImage->recursiveLoadLibraries(context, preflightOnly, thisRPaths, libraryInfos[i].name);
			}
		}
		
		// do deep prebind check
		if ( fAllLibraryChecksumsAndLoadAddressesMatch ) {
			for(unsigned int i=0; i < libraryCount(); ++i){
				ImageLoader* dependentImage = libImage(i);
				if ( dependentImage != NULL ) {	
					if ( !dependentImage->allDependentLibrariesAsWhenPreBound() )
						fAllLibraryChecksumsAndLoadAddressesMatch = false;
				}
			}
		}
		
		// free rpaths (getRPaths() malloc'ed each string)
		for(std::vector<const char*>::iterator it=rpathsFromThisImage.begin(); it != rpathsFromThisImage.end(); ++it) {
			const char* str = *it;
			free((void*)str);
		}
		
	}
}


void ImageLoader::recursiveRebaseWithAccounting(const LinkContext& context)
{
	this->recursiveRebase(context);
	vmAccountingSetSuspended(context, false);
}

void ImageLoader::recursiveRebase(const LinkContext& context)
{ 
	if ( fState < dyld_image_state_rebased ) {
		// break cycles
		fState = dyld_image_state_rebased;
		
		try {
			// rebase lower level libraries first
			for(unsigned int i=0; i < libraryCount(); ++i) {
				ImageLoader* dependentImage = libImage(i);
				if ( dependentImage != NULL )
					dependentImage->recursiveRebase(context);
			}
				
			// rebase this image
			doRebase(context);
			
			// notify
			context.notifySingle(dyld_image_state_rebased, this, NULL);
		}
		catch (const char* msg) {
			// this image is not rebased
			fState = dyld_image_state_dependents_mapped;
            CRSetCrashLogMessage2(NULL);
			throw;
		}
	}
}

void ImageLoader::recursiveApplyInterposing(const LinkContext& context)
{ 
	if ( ! fInterposed ) {
		// break cycles
		fInterposed = true;
		
		try {
			// interpose lower level libraries first
			for(unsigned int i=0; i < libraryCount(); ++i) {
				ImageLoader* dependentImage = libImage(i);
				if ( dependentImage != NULL )
					dependentImage->recursiveApplyInterposing(context);
			}
				
			// interpose this image
			doInterpose(context);
		}
		catch (const char* msg) {
			// this image is not interposed
			fInterposed = false;
			throw;
		}
	}
}

void ImageLoader::recursiveMakeDataReadOnly(const LinkContext& context)
{
	if ( ! fMadeReadOnly ) {
		// break cycles
		fMadeReadOnly = true;

		try {
			// handle lower level libraries first
			for(unsigned int i=0; i < libraryCount(); ++i) {
				ImageLoader* dependentImage = libImage(i);
				if ( dependentImage != NULL )
					dependentImage->recursiveMakeDataReadOnly(context);
			}

			// if this image has __DATA_CONST, make that segment read-only
			makeDataReadOnly();
		}
		catch (const char* msg) {
			fMadeReadOnly = false;
			throw;
		}
	}
}


void ImageLoader::recursiveBindWithAccounting(const LinkContext& context, bool forceLazysBound, bool neverUnload)
{
	this->recursiveBind(context, forceLazysBound, neverUnload);
	vmAccountingSetSuspended(context, false);
}

void ImageLoader::recursiveBind(const LinkContext& context, bool forceLazysBound, bool neverUnload)
{
	// Normally just non-lazy pointers are bound immediately.
	// The exceptions are:
	//   1) DYLD_BIND_AT_LAUNCH will cause lazy pointers to be bound immediately
	//   2) some API's (e.g. RTLD_NOW) can cause lazy pointers to be bound immediately
	if ( fState < dyld_image_state_bound ) {
		// break cycles
		fState = dyld_image_state_bound;
	
		try {
			// bind lower level libraries first
			for(unsigned int i=0; i < libraryCount(); ++i) {
				ImageLoader* dependentImage = libImage(i);
				if ( dependentImage != NULL )
					dependentImage->recursiveBind(context, forceLazysBound, neverUnload);
			}
			// bind this image
			this->doBind(context, forceLazysBound);	
			// mark if lazys are also bound
			if ( forceLazysBound || this->usablePrebinding(context) )
				fAllLazyPointersBound = true;
			// mark as never-unload if requested
			if ( neverUnload )
				this->setNeverUnload();
				
			context.notifySingle(dyld_image_state_bound, this, NULL);
		}
		catch (const char* msg) {
			// restore state
			fState = dyld_image_state_rebased;
            CRSetCrashLogMessage2(NULL);
			throw;
		}
	}
}



// These are mangled symbols for all the variants of operator new and delete
// which a main executable can define (non-weak) and override the
// weak-def implementation in the OS.
static const char* const sTreatAsWeak[] = {
    "__Znwm", "__ZnwmRKSt9nothrow_t",
    "__Znam", "__ZnamRKSt9nothrow_t",
    "__ZdlPv", "__ZdlPvRKSt9nothrow_t", "__ZdlPvm",
    "__ZdaPv", "__ZdaPvRKSt9nothrow_t", "__ZdaPvm",
    "__ZnwmSt11align_val_t", "__ZnwmSt11align_val_tRKSt9nothrow_t",
    "__ZnamSt11align_val_t", "__ZnamSt11align_val_tRKSt9nothrow_t",
    "__ZdlPvSt11align_val_t", "__ZdlPvSt11align_val_tRKSt9nothrow_t", "__ZdlPvmSt11align_val_t",
    "__ZdaPvSt11align_val_t", "__ZdaPvSt11align_val_tRKSt9nothrow_t", "__ZdaPvmSt11align_val_t"
};

size_t ImageLoader::HashCString::hash(const char* v) {
    // FIXME: Use hash<string_view> when it has the correct visibility markup
    return std::hash<std::string_view>{}(v);
}

bool ImageLoader::EqualCString::equal(const char* s1, const char* s2) {
    return strcmp(s1, s2) == 0;
}

void ImageLoader::weakBind(const LinkContext& context)
{

	if (!context.useNewWeakBind) {
		weakBindOld(context);
		return;
	}

	if ( context.verboseWeakBind )
		dyld::log("dyld: weak bind start:\n");
	uint64_t t1 = mach_absolute_time();

	// get set of ImageLoaders that participate in coalecsing
	ImageLoader* imagesNeedingCoalescing[fgImagesRequiringCoalescing];
	unsigned imageIndexes[fgImagesRequiringCoalescing];
	int count = context.getCoalescedImages(imagesNeedingCoalescing, imageIndexes);
	
	// count how many have not already had weakbinding done
	int countNotYetWeakBound = 0;
	int countOfImagesWithWeakDefinitionsNotInSharedCache = 0;
	for(int i=0; i < count; ++i) {
		if ( ! imagesNeedingCoalescing[i]->weakSymbolsBound(imageIndexes[i]) )
			++countNotYetWeakBound;
		if ( ! imagesNeedingCoalescing[i]->inSharedCache() )
			++countOfImagesWithWeakDefinitionsNotInSharedCache;
	}

	// don't need to do any coalescing if only one image has overrides, or all have already been done
	if ( (countOfImagesWithWeakDefinitionsNotInSharedCache > 0) && (countNotYetWeakBound > 0) ) {
		if (!context.weakDefMapInitialized) {
			// Initialize the weak def map as the link context doesn't run static initializers
			new (&context.weakDefMap) dyld3::Map<const char*, std::pair<const ImageLoader*, uintptr_t>, ImageLoader::HashCString, ImageLoader::EqualCString>();
			context.weakDefMapInitialized = true;
		}
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	  // only do alternate algorithm for dlopen(). Use traditional algorithm for launch
	  if ( !context.linkingMainExecutable ) {
		  // Don't take the memory hit of weak defs on the launch path until we hit a dlopen with more weak symbols to bind
		  if (!context.weakDefMapProcessedLaunchDefs) {
			  context.weakDefMapProcessedLaunchDefs = true;

			  // Walk the nlist for all binaries from launch and fill in the map with any other weak defs
			  for (int i=0; i < count; ++i) {
				  const ImageLoader* image = imagesNeedingCoalescing[i];
				  // skip images without defs.  We've processed launch time refs already
				  if ( !image->hasCoalescedExports() )
					  continue;
				  // Only process binaries which have had their weak symbols bound, ie, not the new ones we are processing now
				  // from this dlopen
				  if ( !image->weakSymbolsBound(imageIndexes[i]) )
					  continue;

				  Diagnostics diag;
				  const dyld3::MachOAnalyzer* ma = (const dyld3::MachOAnalyzer*)image->machHeader();
				  ma->forEachWeakDef(diag, ^(const char *symbolName, uintptr_t imageOffset, bool isFromExportTrie) {
					  uintptr_t targetAddr = (uintptr_t)ma + imageOffset;
					  if ( isFromExportTrie ) {
						  // Avoid duplicating the string if we already have the symbol name
						  if ( context.weakDefMap.find(symbolName) != context.weakDefMap.end() )
							  return;
						  symbolName = strdup(symbolName);
					  }
					  context.weakDefMap.insert({ symbolName, { image, targetAddr } });
				  });
			  }
		  }

		  // Walk the nlist for all binaries in dlopen and fill in the map with any other weak defs
		  for (int i=0; i < count; ++i) {
			  const ImageLoader* image = imagesNeedingCoalescing[i];
			  if ( image->weakSymbolsBound(imageIndexes[i]) )
				  continue;
			  // skip images without defs.  We'll process refs later
			  if ( !image->hasCoalescedExports() )
				  continue;
			  Diagnostics diag;
			  const dyld3::MachOAnalyzer* ma = (const dyld3::MachOAnalyzer*)image->machHeader();
			  ma->forEachWeakDef(diag, ^(const char *symbolName, uintptr_t imageOffset, bool isFromExportTrie) {
				  uintptr_t targetAddr = (uintptr_t)ma + imageOffset;
				  if ( isFromExportTrie ) {
					  // Avoid duplicating the string if we already have the symbol name
					  if ( context.weakDefMap.find(symbolName) != context.weakDefMap.end() )
						  return;
					  symbolName = strdup(symbolName);
				  }
				  context.weakDefMap.insert({ symbolName, { image, targetAddr } });
			  });
		  }
		// for all images that need weak binding
		for (int i=0; i < count; ++i) {
			ImageLoader* imageBeingFixedUp = imagesNeedingCoalescing[i];
			if ( imageBeingFixedUp->weakSymbolsBound(imageIndexes[i]) )
				continue; // weak binding already completed
			bool imageBeingFixedUpInCache = imageBeingFixedUp->inSharedCache();

			if ( context.verboseWeakBind )
				dyld::log("dyld: checking for weak symbols in %s\n", imageBeingFixedUp->getPath());
			// for all symbols that need weak binding in this image
			ImageLoader::CoalIterator coalIterator;
			imageBeingFixedUp->initializeCoalIterator(coalIterator, i, imageIndexes[i]);
			while ( !imageBeingFixedUp->incrementCoalIterator(coalIterator) ) {
				const char*         nameToCoalesce = coalIterator.symbolName;
				uintptr_t           targetAddr     = 0;
				const ImageLoader*  targetImage;
				// Seatch the map for a previous definition to use
				auto weakDefIt = context.weakDefMap.find(nameToCoalesce);
				if ( (weakDefIt != context.weakDefMap.end()) && (weakDefIt->second.first != nullptr) ) {
					// Found a previous defition
					targetImage = weakDefIt->second.first;
					targetAddr = weakDefIt->second.second;
				} else {
					// scan all images looking for definition to use
					for (int j=0; j < count; ++j) {
						const ImageLoader* anImage = imagesNeedingCoalescing[j];
						bool anImageInCache = anImage->inSharedCache();
						// <rdar://problem/47986398> Don't look at images in dyld cache because cache is
						//  already coalesced.  Only images outside cache can potentially override something in cache.
						if ( anImageInCache && imageBeingFixedUpInCache )
							continue;

						//dyld::log("looking for %s in %s\n", nameToCoalesce, anImage->getPath());
						const ImageLoader* foundIn;
						const Symbol* sym = anImage->findExportedSymbol(nameToCoalesce, false, &foundIn);
						if ( sym != NULL ) {
							targetAddr = foundIn->getExportedSymbolAddress(sym, context);
							targetImage = foundIn;
							if ( context.verboseWeakBind )
								dyld::log("dyld:   found weak %s at 0x%lX in %s\n", nameToCoalesce, targetAddr, foundIn->getPath());
							break;
						}
					}
				}
				if ( (targetAddr != 0) && (coalIterator.image != targetImage) ) {
					coalIterator.image->updateUsesCoalIterator(coalIterator, targetAddr, (ImageLoader*)targetImage, 0, context);
					if (weakDefIt == context.weakDefMap.end()) {
						if (targetImage->neverUnload()) {
							// Add never unload defs to the map for next time
							context.weakDefMap.insert({ nameToCoalesce, { targetImage, targetAddr } });
							if ( context.verboseWeakBind ) {
								dyld::log("dyld: weak binding adding %s to map\n", nameToCoalesce);
							}
						} else {
							// Add a placeholder for unloadable symbols which makes us fall back to the regular search
							context.weakDefMap.insert({ nameToCoalesce, { targetImage, targetAddr } });
							if ( context.verboseWeakBind ) {
								dyld::log("dyld: weak binding adding unloadable placeholder %s to map\n", nameToCoalesce);
							}
						}
					}
					if ( context.verboseWeakBind )
						dyld::log("dyld:     adjusting uses of %s in %s to use definition from %s\n", nameToCoalesce, coalIterator.image->getPath(), targetImage->getPath());
				}
			}
			imageBeingFixedUp->setWeakSymbolsBound(imageIndexes[i]);
		}
	  }
	  else
#endif // __MAC_OS_X_VERSION_MIN_REQUIRED
	  {
		// make symbol iterators for each
		ImageLoader::CoalIterator iterators[count];
		ImageLoader::CoalIterator* sortedIts[count];
		for(int i=0; i < count; ++i) {
			imagesNeedingCoalescing[i]->initializeCoalIterator(iterators[i], i, imageIndexes[i]);
			sortedIts[i] = &iterators[i];
			if ( context.verboseWeakBind )
				dyld::log("dyld: weak bind load order %d/%d for %s\n", i, count, imagesNeedingCoalescing[i]->getIndexedPath(imageIndexes[i]));
		}
		
		// walk all symbols keeping iterators in sync by 
		// only ever incrementing the iterator with the lowest symbol 
		int doneCount = 0;
		while ( doneCount != count ) {
			//for(int i=0; i < count; ++i)
			//	dyld::log("sym[%d]=%s ", sortedIts[i]->loadOrder, sortedIts[i]->symbolName);
			//dyld::log("\n");
			// increment iterator with lowest symbol
			if ( sortedIts[0]->image->incrementCoalIterator(*sortedIts[0]) )
				++doneCount; 
			// re-sort iterators
			for(int i=1; i < count; ++i) {
				int result = strcmp(sortedIts[i-1]->symbolName, sortedIts[i]->symbolName);
				if ( result == 0 )
					sortedIts[i-1]->symbolMatches = true;
				if ( result > 0 ) {
					// new one is bigger then next, so swap
					ImageLoader::CoalIterator* temp = sortedIts[i-1];
					sortedIts[i-1] = sortedIts[i];
					sortedIts[i] = temp;
				}
				if ( result < 0 )
					break;
			}
			// process all matching symbols just before incrementing the lowest one that matches
			if ( sortedIts[0]->symbolMatches && !sortedIts[0]->done ) {
				const char* nameToCoalesce = sortedIts[0]->symbolName;
				// pick first symbol in load order (and non-weak overrides weak)
				uintptr_t targetAddr = 0;
				ImageLoader* targetImage = NULL;
				unsigned targetImageIndex = 0;
				for(int i=0; i < count; ++i) {
					if ( strcmp(iterators[i].symbolName, nameToCoalesce) == 0 ) {
						if ( context.verboseWeakBind )
							dyld::log("dyld: weak bind, found %s weak=%d in %s \n", nameToCoalesce, iterators[i].weakSymbol, iterators[i].image->getIndexedPath((unsigned)iterators[i].imageIndex));
						if ( iterators[i].weakSymbol ) {
							if ( targetAddr == 0 ) {
								targetAddr = iterators[i].image->getAddressCoalIterator(iterators[i], context);
								if ( targetAddr != 0 ) {
									targetImage = iterators[i].image;
									targetImageIndex = (unsigned)iterators[i].imageIndex;
								}
							}
						}
						else {
							targetAddr = iterators[i].image->getAddressCoalIterator(iterators[i], context);
							if ( targetAddr != 0 ) {
								targetImage = iterators[i].image;
								targetImageIndex = (unsigned)iterators[i].imageIndex;
								// strong implementation found, stop searching
								break;
							}
						}
					}
				}
				// tell each to bind to this symbol (unless already bound)
				if ( targetAddr != 0 ) {
					if ( context.verboseWeakBind ) {
						dyld::log("dyld: weak binding all uses of %s to copy from %s\n",
									nameToCoalesce, targetImage->getIndexedShortName(targetImageIndex));
					}
					for(int i=0; i < count; ++i) {
						if ( strcmp(iterators[i].symbolName, nameToCoalesce) == 0 ) {
							if ( context.verboseWeakBind ) {
								dyld::log("dyld: weak bind, setting all uses of %s in %s to 0x%lX from %s\n",
											nameToCoalesce, iterators[i].image->getIndexedShortName((unsigned)iterators[i].imageIndex),
											targetAddr, targetImage->getIndexedShortName(targetImageIndex));
							}
							if ( ! iterators[i].image->weakSymbolsBound(imageIndexes[i]) )
								iterators[i].image->updateUsesCoalIterator(iterators[i], targetAddr, targetImage, targetImageIndex, context);
							iterators[i].symbolMatches = false; 
						}
					}
					if (targetImage->neverUnload()) {
						// Add never unload defs to the map for next time
						context.weakDefMap.insert({ nameToCoalesce, { targetImage, targetAddr } });
						if ( context.verboseWeakBind ) {
							dyld::log("dyld: weak binding adding %s to map\n",
										nameToCoalesce);
						}
					}
				}

			}
		}

		for (int i=0; i < count; ++i) {
			if ( imagesNeedingCoalescing[i]->weakSymbolsBound(imageIndexes[i]) )
				continue;	// skip images already processed

			if ( imagesNeedingCoalescing[i]->usesChainedFixups() ) {
				// during binding of references to weak-def symbols, the dyld cache was patched
				// but if main executable has non-weak override of operator new or delete it needs is handled here
				for (const char* weakSymbolName : sTreatAsWeak) {
					const ImageLoader* dummy;
					imagesNeedingCoalescing[i]->resolveWeak(context, weakSymbolName, true, false, &dummy);
				}
			}
#if __arm64e__
			else {
				// support traditional arm64 app on an arm64e device
				// look for weak def symbols in this image which may override the cache
				ImageLoader::CoalIterator coaler;
				imagesNeedingCoalescing[i]->initializeCoalIterator(coaler, i, 0);
				imagesNeedingCoalescing[i]->incrementCoalIterator(coaler);
				while ( !coaler.done ) {
					const ImageLoader* dummy;
					// a side effect of resolveWeak() is to patch cache
					imagesNeedingCoalescing[i]->resolveWeak(context, coaler.symbolName, true, false, &dummy);
					imagesNeedingCoalescing[i]->incrementCoalIterator(coaler);
				}
			}
#endif
		}

		// mark all as having all weak symbols bound
		for(int i=0; i < count; ++i) {
			imagesNeedingCoalescing[i]->setWeakSymbolsBound(imageIndexes[i]);
		}
	  }
	}

	uint64_t t2 = mach_absolute_time();
	fgTotalWeakBindTime += t2  - t1;
	
	if ( context.verboseWeakBind )
		dyld::log("dyld: weak bind end\n");
}


void ImageLoader::weakBindOld(const LinkContext& context)
{
	if ( context.verboseWeakBind )
		dyld::log("dyld: weak bind start:\n");
	uint64_t t1 = mach_absolute_time();
	// get set of ImageLoaders that participate in coalecsing
	ImageLoader* imagesNeedingCoalescing[fgImagesRequiringCoalescing];
	unsigned imageIndexes[fgImagesRequiringCoalescing];
	int count = context.getCoalescedImages(imagesNeedingCoalescing, imageIndexes);

	// count how many have not already had weakbinding done
	int countNotYetWeakBound = 0;
	int countOfImagesWithWeakDefinitionsNotInSharedCache = 0;
	for(int i=0; i < count; ++i) {
		if ( ! imagesNeedingCoalescing[i]->weakSymbolsBound(imageIndexes[i]) )
			++countNotYetWeakBound;
		if ( ! imagesNeedingCoalescing[i]->inSharedCache() )
			++countOfImagesWithWeakDefinitionsNotInSharedCache;
	}

	// don't need to do any coalescing if only one image has overrides, or all have already been done
	if ( (countOfImagesWithWeakDefinitionsNotInSharedCache > 0) && (countNotYetWeakBound > 0) ) {
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	  // only do alternate algorithm for dlopen(). Use traditional algorithm for launch
	  if ( !context.linkingMainExecutable ) {
		// for all images that need weak binding
		for (int i=0; i < count; ++i) {
			ImageLoader* imageBeingFixedUp = imagesNeedingCoalescing[i];
			if ( imageBeingFixedUp->weakSymbolsBound(imageIndexes[i]) )
				continue; // weak binding already completed
			bool imageBeingFixedUpInCache = imageBeingFixedUp->inSharedCache();

			if ( context.verboseWeakBind )
				dyld::log("dyld: checking for weak symbols in %s\n", imageBeingFixedUp->getPath());
			// for all symbols that need weak binding in this image
			ImageLoader::CoalIterator coalIterator;
			imageBeingFixedUp->initializeCoalIterator(coalIterator, i, imageIndexes[i]);
			while ( !imageBeingFixedUp->incrementCoalIterator(coalIterator) ) {
				const char*         nameToCoalesce = coalIterator.symbolName;
				uintptr_t           targetAddr     = 0;
				const ImageLoader*  targetImage;
				// scan all images looking for definition to use
				for (int j=0; j < count; ++j) {
					const ImageLoader* anImage = imagesNeedingCoalescing[j];
					bool anImageInCache = anImage->inSharedCache();
					// <rdar://problem/47986398> Don't look at images in dyld cache because cache is
					//  already coalesced.  Only images outside cache can potentially override something in cache.
					if ( anImageInCache && imageBeingFixedUpInCache )
						continue;

					//dyld::log("looking for %s in %s\n", nameToCoalesce, anImage->getPath());
					const ImageLoader* foundIn;
					const Symbol* sym = anImage->findExportedSymbol(nameToCoalesce, false, &foundIn);
					if ( sym != NULL ) {
						if ( (foundIn->getExportedSymbolInfo(sym) & ImageLoader::kWeakDefinition) == 0 ) {
							// found non-weak def, use it and stop looking
							targetAddr = foundIn->getExportedSymbolAddress(sym, context);
							targetImage = foundIn;
							if ( context.verboseWeakBind )
								dyld::log("dyld:   found strong %s at 0x%lX in %s\n", nameToCoalesce, targetAddr, foundIn->getPath());
							break;
						}
						else {
							// found weak-def, only use if no weak found yet
							if ( targetAddr == 0 ) {
								targetAddr = foundIn->getExportedSymbolAddress(sym, context);
								targetImage = foundIn;
								if ( context.verboseWeakBind )
									dyld::log("dyld:   found weak %s at 0x%lX in %s\n", nameToCoalesce, targetAddr, foundIn->getPath());
							}
						}
					}
				}
				if ( (targetAddr != 0) && (coalIterator.image != targetImage) ) {
					coalIterator.image->updateUsesCoalIterator(coalIterator, targetAddr, (ImageLoader*)targetImage, 0, context);
					if ( context.verboseWeakBind )
						dyld::log("dyld:     adjusting uses of %s in %s to use definition from %s\n", nameToCoalesce, coalIterator.image->getPath(), targetImage->getPath());
				}
			}
			imageBeingFixedUp->setWeakSymbolsBound(imageIndexes[i]);
		}
	  }
	  else
#endif // __MAC_OS_X_VERSION_MIN_REQUIRED
	  {
		// make symbol iterators for each
		ImageLoader::CoalIterator iterators[count];
		ImageLoader::CoalIterator* sortedIts[count];
		for(int i=0; i < count; ++i) {
			imagesNeedingCoalescing[i]->initializeCoalIterator(iterators[i], i, imageIndexes[i]);
			sortedIts[i] = &iterators[i];
			if ( context.verboseWeakBind )
				dyld::log("dyld: weak bind load order %d/%d for %s\n", i, count, imagesNeedingCoalescing[i]->getIndexedPath(imageIndexes[i]));
		}

		// walk all symbols keeping iterators in sync by
		// only ever incrementing the iterator with the lowest symbol
		int doneCount = 0;
		while ( doneCount != count ) {
			//for(int i=0; i < count; ++i)
			//	dyld::log("sym[%d]=%s ", sortedIts[i]->loadOrder, sortedIts[i]->symbolName);
			//dyld::log("\n");
			// increment iterator with lowest symbol
			if ( sortedIts[0]->image->incrementCoalIterator(*sortedIts[0]) )
				++doneCount;
			// re-sort iterators
			for(int i=1; i < count; ++i) {
				int result = strcmp(sortedIts[i-1]->symbolName, sortedIts[i]->symbolName);
				if ( result == 0 )
					sortedIts[i-1]->symbolMatches = true;
				if ( result > 0 ) {
					// new one is bigger then next, so swap
					ImageLoader::CoalIterator* temp = sortedIts[i-1];
					sortedIts[i-1] = sortedIts[i];
					sortedIts[i] = temp;
				}
				if ( result < 0 )
					break;
			}
			// process all matching symbols just before incrementing the lowest one that matches
			if ( sortedIts[0]->symbolMatches && !sortedIts[0]->done ) {
				const char* nameToCoalesce = sortedIts[0]->symbolName;
				// pick first symbol in load order (and non-weak overrides weak)
				uintptr_t targetAddr = 0;
				ImageLoader* targetImage = NULL;
				unsigned targetImageIndex = 0;
				for(int i=0; i < count; ++i) {
					if ( strcmp(iterators[i].symbolName, nameToCoalesce) == 0 ) {
						if ( context.verboseWeakBind )
							dyld::log("dyld: weak bind, found %s weak=%d in %s \n", nameToCoalesce, iterators[i].weakSymbol, iterators[i].image->getIndexedPath((unsigned)iterators[i].imageIndex));
						if ( iterators[i].weakSymbol ) {
							if ( targetAddr == 0 ) {
								targetAddr = iterators[i].image->getAddressCoalIterator(iterators[i], context);
								if ( targetAddr != 0 ) {
									targetImage = iterators[i].image;
									targetImageIndex = (unsigned)iterators[i].imageIndex;
								}
							}
						}
						else {
							targetAddr = iterators[i].image->getAddressCoalIterator(iterators[i], context);
							if ( targetAddr != 0 ) {
								targetImage = iterators[i].image;
								targetImageIndex = (unsigned)iterators[i].imageIndex;
								// strong implementation found, stop searching
								break;
							}
						}
					}
				}
				// tell each to bind to this symbol (unless already bound)
				if ( targetAddr != 0 ) {
					if ( context.verboseWeakBind ) {
						dyld::log("dyld: weak binding all uses of %s to copy from %s\n",
									nameToCoalesce, targetImage->getIndexedShortName(targetImageIndex));
					}
					for(int i=0; i < count; ++i) {
						if ( strcmp(iterators[i].symbolName, nameToCoalesce) == 0 ) {
							if ( context.verboseWeakBind ) {
								dyld::log("dyld: weak bind, setting all uses of %s in %s to 0x%lX from %s\n",
											nameToCoalesce, iterators[i].image->getIndexedShortName((unsigned)iterators[i].imageIndex),
											targetAddr, targetImage->getIndexedShortName(targetImageIndex));
							}
							if ( ! iterators[i].image->weakSymbolsBound(imageIndexes[i]) )
								iterators[i].image->updateUsesCoalIterator(iterators[i], targetAddr, targetImage, targetImageIndex, context);
							iterators[i].symbolMatches = false;
						}
					}
				}

			}
		}

		for (int i=0; i < count; ++i) {
			if ( imagesNeedingCoalescing[i]->weakSymbolsBound(imageIndexes[i]) )
				continue;	// skip images already processed

			if ( imagesNeedingCoalescing[i]->usesChainedFixups() ) {
				// during binding of references to weak-def symbols, the dyld cache was patched
				// but if main executable has non-weak override of operator new or delete it needs is handled here
				for (const char* weakSymbolName : sTreatAsWeak) {
					const ImageLoader* dummy;
					imagesNeedingCoalescing[i]->resolveWeak(context, weakSymbolName, true, false, &dummy);
				}
			}
#if __arm64e__
			else {
				// support traditional arm64 app on an arm64e device
				// look for weak def symbols in this image which may override the cache
				ImageLoader::CoalIterator coaler;
				imagesNeedingCoalescing[i]->initializeCoalIterator(coaler, i, 0);
				imagesNeedingCoalescing[i]->incrementCoalIterator(coaler);
				while ( !coaler.done ) {
					const ImageLoader* dummy;
					// a side effect of resolveWeak() is to patch cache
					imagesNeedingCoalescing[i]->resolveWeak(context, coaler.symbolName, true, false, &dummy);
					imagesNeedingCoalescing[i]->incrementCoalIterator(coaler);
				}
			}
#endif
		}

		// mark all as having all weak symbols bound
		for(int i=0; i < count; ++i) {
			imagesNeedingCoalescing[i]->setWeakSymbolsBound(imageIndexes[i]);
		}
	  }
	}

	uint64_t t2 = mach_absolute_time();
	fgTotalWeakBindTime += t2  - t1;

	if ( context.verboseWeakBind )
		dyld::log("dyld: weak bind end\n");
}



void ImageLoader::recursiveGetDOFSections(const LinkContext& context, std::vector<DOFInfo>& dofs)
{
	if ( ! fRegisteredDOF ) {
		// break cycles
		fRegisteredDOF = true;
		
		// gather lower level libraries first
		for(unsigned int i=0; i < libraryCount(); ++i) {
			ImageLoader* dependentImage = libImage(i);
			if ( dependentImage != NULL )
				dependentImage->recursiveGetDOFSections(context, dofs);
		}
		this->doGetDOFSections(context, dofs);
	}
}

void ImageLoader::setNeverUnloadRecursive() {
	if ( ! fNeverUnload ) {
		// break cycles
		fNeverUnload = true;
		
		// gather lower level libraries first
		for(unsigned int i=0; i < libraryCount(); ++i) {
			ImageLoader* dependentImage = libImage(i);
			if ( dependentImage != NULL )
				dependentImage->setNeverUnloadRecursive();
		}
	}
}

void ImageLoader::recursiveSpinLock(recursive_lock& rlock)
{
	// try to set image's ivar fInitializerRecursiveLock to point to this lock_info
	// keep trying until success (spin)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	while ( ! OSAtomicCompareAndSwapPtrBarrier(NULL, &rlock, (void**)&fInitializerRecursiveLock) ) {
		// if fInitializerRecursiveLock already points to a different lock_info, if it is for
		// the same thread we are on, the increment the lock count, otherwise continue to spin
		if ( (fInitializerRecursiveLock != NULL) && (fInitializerRecursiveLock->thread == rlock.thread) )
			break;
	}
#pragma clang diagnostic pop
	++(fInitializerRecursiveLock->count);
}

void ImageLoader::recursiveSpinUnLock()
{
	if ( --(fInitializerRecursiveLock->count) == 0 )
		fInitializerRecursiveLock = NULL;
}

void ImageLoader::InitializerTimingList::addTime(const char* name, uint64_t time)
{
	for (int i=0; i < count; ++i) {
		if ( strcmp(images[i].shortName, name) == 0 ) {
			images[i].initTime += time;
			return;
		}
	}
	images[count].initTime = time;
	images[count].shortName = name;
	++count;
}

void ImageLoader::recursiveInitialization(const LinkContext& context, mach_port_t this_thread, const char* pathToInitialize,
										  InitializerTimingList& timingInfo, UninitedUpwards& uninitUps)
{
	recursive_lock lock_info(this_thread);
	recursiveSpinLock(lock_info);

	if ( fState < dyld_image_state_dependents_initialized-1 ) {
		uint8_t oldState = fState;
		// break cycles
		fState = dyld_image_state_dependents_initialized-1;
		try {
			// initialize lower level libraries first
			for(unsigned int i=0; i < libraryCount(); ++i) {
				ImageLoader* dependentImage = libImage(i);
				if ( dependentImage != NULL ) {
					// don't try to initialize stuff "above" me yet
					if ( libIsUpward(i) ) {
						uninitUps.imagesAndPaths[uninitUps.count] = { dependentImage, libPath(i) };
						uninitUps.count++;
					}
					else if ( dependentImage->fDepth >= fDepth ) {
						dependentImage->recursiveInitialization(context, this_thread, libPath(i), timingInfo, uninitUps);
					}
                }
			}
			
			// record termination order
			if ( this->needsTermination() )
				context.terminationRecorder(this);

			// let objc know we are about to initialize this image
			uint64_t t1 = mach_absolute_time();
			fState = dyld_image_state_dependents_initialized;
			oldState = fState;
			context.notifySingle(dyld_image_state_dependents_initialized, this, &timingInfo);
			
			// initialize this image
			bool hasInitializers = this->doInitialization(context);

			// let anyone know we finished initializing this image
			fState = dyld_image_state_initialized;
			oldState = fState;
			context.notifySingle(dyld_image_state_initialized, this, NULL);
			
			if ( hasInitializers ) {
				uint64_t t2 = mach_absolute_time();
				timingInfo.addTime(this->getShortName(), t2-t1);
			}
		}
		catch (const char* msg) {
			// this image is not initialized
			fState = oldState;
			recursiveSpinUnLock();
			throw;
		}
	}
	
	recursiveSpinUnLock();
}


static void printTime(const char* msg, uint64_t partTime, uint64_t totalTime)
{
	static uint64_t sUnitsPerSecond = 0;
	if ( sUnitsPerSecond == 0 ) {
		struct mach_timebase_info timeBaseInfo;
		if ( mach_timebase_info(&timeBaseInfo) != KERN_SUCCESS )
			return;
		sUnitsPerSecond = 1000000000ULL * timeBaseInfo.denom / timeBaseInfo.numer;
	}
	if ( partTime < sUnitsPerSecond ) {
		uint32_t milliSecondsTimesHundred = (uint32_t)((partTime*100000)/sUnitsPerSecond);
		uint32_t milliSeconds = (uint32_t)(milliSecondsTimesHundred/100);
		uint32_t percentTimesTen = (uint32_t)((partTime*1000)/totalTime);
		uint32_t percent = percentTimesTen/10;
		if ( milliSeconds >= 100 )
			dyld::log("%s: %u.%02u milliseconds (%u.%u%%)\n", msg, milliSeconds, milliSecondsTimesHundred-milliSeconds*100, percent, percentTimesTen-percent*10);
		else if ( milliSeconds >= 10 )
			dyld::log("%s:  %u.%02u milliseconds (%u.%u%%)\n", msg, milliSeconds, milliSecondsTimesHundred-milliSeconds*100, percent, percentTimesTen-percent*10);
		else
			dyld::log("%s:   %u.%02u milliseconds (%u.%u%%)\n", msg, milliSeconds, milliSecondsTimesHundred-milliSeconds*100, percent, percentTimesTen-percent*10);
	}
	else {
		uint32_t secondsTimeTen = (uint32_t)((partTime*10)/sUnitsPerSecond);
		uint32_t seconds = secondsTimeTen/10;
		uint32_t percentTimesTen = (uint32_t)((partTime*1000)/totalTime);
		uint32_t percent = percentTimesTen/10;
		dyld::log("%s: %u.%u seconds (%u.%u%%)\n", msg, seconds, secondsTimeTen-seconds*10, percent, percentTimesTen-percent*10);
	}
}

static char* commatize(uint64_t in, char* out)
{
	uint64_t div10 = in / 10;
	uint8_t delta = in - div10*10;
	char* s = &out[32];
	int digitCount = 1;
	*s = '\0';
	*(--s) = '0' + delta;
	in = div10;
	while ( in != 0 ) {
		if ( (digitCount % 3) == 0 )
			*(--s) = ',';
		div10 = in / 10;
		delta = in - div10*10;
		*(--s) = '0' + delta;
		in = div10;
		++digitCount;
	}
	return s;
}


void ImageLoader::printStatistics(unsigned int imageCount, const InitializerTimingList& timingInfo)
{
	uint64_t totalTime = fgTotalLoadLibrariesTime  + fgTotalRebaseTime + fgTotalBindTime + fgTotalWeakBindTime + fgTotalDOF + fgTotalInitTime;

	uint64_t totalDyldTime = totalTime - fgTotalDebuggerPausedTime - fgTotalRebindCacheTime;
	printTime("Total pre-main time", totalDyldTime, totalDyldTime);
	printTime("         dylib loading time", fgTotalLoadLibrariesTime-fgTotalDebuggerPausedTime, totalDyldTime);
	printTime("        rebase/binding time", fgTotalRebaseTime+fgTotalBindTime+fgTotalWeakBindTime-fgTotalRebindCacheTime, totalDyldTime);
	printTime("            ObjC setup time", fgTotalObjCSetupTime, totalDyldTime);
	printTime("           initializer time", fgTotalInitTime-fgTotalObjCSetupTime, totalDyldTime);
	dyld::log("           slowest intializers :\n");
	for (uintptr_t i=0; i < timingInfo.count; ++i) {
		uint64_t t = timingInfo.images[i].initTime;
		if ( t*50 < totalDyldTime )
			continue;
		dyld::log("%30s ", timingInfo.images[i].shortName);
		if ( strncmp(timingInfo.images[i].shortName, "libSystem.", 10) == 0 )
			t -= fgTotalObjCSetupTime;
			printTime("", t, totalDyldTime);
	}
	dyld::log("\n");
}

void ImageLoader::printStatisticsDetails(unsigned int imageCount, const InitializerTimingList& timingInfo)
{
	uint64_t totalTime = fgTotalLoadLibrariesTime  + fgTotalRebaseTime + fgTotalBindTime + fgTotalWeakBindTime + fgTotalDOF + fgTotalInitTime;
	char commaNum1[40];
	char commaNum2[40];

	printTime("  total time", totalTime, totalTime);
	dyld::log("  total images loaded:  %d (%u from dyld shared cache)\n", imageCount, fgImagesUsedFromSharedCache);
	dyld::log("  total segments mapped: %u, into %llu pages\n", fgTotalSegmentsMapped, fgTotalBytesMapped/4096);
	printTime("  total images loading time", fgTotalLoadLibrariesTime, totalTime);
	printTime("  total load time in ObjC", fgTotalObjCSetupTime, totalTime);
	printTime("  total debugger pause time", fgTotalDebuggerPausedTime, totalTime);
	printTime("  total dtrace DOF registration time", fgTotalDOF, totalTime);
	dyld::log("  total rebase fixups:  %s\n", commatize(fgTotalRebaseFixups, commaNum1));
	printTime("  total rebase fixups time", fgTotalRebaseTime, totalTime);
	dyld::log("  total binding fixups: %s\n", commatize(fgTotalBindFixups, commaNum1));
	if ( fgTotalBindSymbolsResolved != 0 ) {
		uint32_t avgTimesTen = (fgTotalBindImageSearches * 10) / fgTotalBindSymbolsResolved;
		uint32_t avgInt = fgTotalBindImageSearches / fgTotalBindSymbolsResolved;
		uint32_t avgTenths = avgTimesTen - (avgInt*10);
		dyld::log("total binding symbol lookups: %s, average images searched per symbol: %u.%u\n", 
				commatize(fgTotalBindSymbolsResolved, commaNum1), avgInt, avgTenths);
	}
	printTime("  total binding fixups time", fgTotalBindTime, totalTime);
	printTime("  total weak binding fixups time", fgTotalWeakBindTime, totalTime);
	printTime("  total redo shared cached bindings time", fgTotalRebindCacheTime, totalTime);
	dyld::log("  total bindings lazily fixed up: %s of %s\n", commatize(fgTotalLazyBindFixups, commaNum1), commatize(fgTotalPossibleLazyBindFixups, commaNum2));
	printTime("  total time in initializers and ObjC +load", fgTotalInitTime-fgTotalObjCSetupTime, totalTime);
	for (uintptr_t i=0; i < timingInfo.count; ++i) {
		uint64_t t = timingInfo.images[i].initTime;
		if ( t*1000 < totalTime )
			continue;
		dyld::log("%42s ", timingInfo.images[i].shortName);
		if ( strncmp(timingInfo.images[i].shortName, "libSystem.", 10) == 0 )
			t -= fgTotalObjCSetupTime;
		printTime("", t, totalTime);
	}
	
}


//
// copy path and add suffix to result
//
//  /path/foo.dylib		_debug   =>   /path/foo_debug.dylib	
//  foo.dylib			_debug   =>   foo_debug.dylib
//  foo     			_debug   =>   foo_debug
//  /path/bar			_debug   =>   /path/bar_debug  
//  /path/bar.A.dylib   _debug   =>   /path/bar.A_debug.dylib
//
void ImageLoader::addSuffix(const char* path, const char* suffix, char* result)
{
	strcpy(result, path);
	
	char* start = strrchr(result, '/');
	if ( start != NULL )
		start++;
	else
		start = result;
		
	char* dot = strrchr(start, '.');
	if ( dot != NULL ) {
		strcpy(dot, suffix);
		strcat(&dot[strlen(suffix)], &path[dot-result]);
	}
	else {
		strcat(result, suffix);
	}
}


//
// This function is the hotspot of symbol lookup.  It was pulled out of findExportedSymbol()
// to enable it to be re-written in assembler if needed.
//
const uint8_t* ImageLoader::trieWalk(const uint8_t* start, const uint8_t* end, const char* s)
{
	//dyld::log("trieWalk(%p, %p, %s)\n", start, end, s);
	++fgSymbolTrieSearchs;
	const uint8_t* p = start;
	while ( p != NULL ) {
		uintptr_t terminalSize = *p++;
		if ( terminalSize > 127 ) {
			// except for re-export-with-rename, all terminal sizes fit in one byte
			--p;
			terminalSize = read_uleb128(p, end);
		}
		if ( (*s == '\0') && (terminalSize != 0) ) {
			//dyld::log("trieWalk(%p) returning %p\n", start, p);
			return p;
		}
		const uint8_t* children = p + terminalSize;
		if ( children > end ) {
			dyld::log("trieWalk() malformed trie node, terminalSize=0x%lx extends past end of trie\n", terminalSize);
			return NULL;
		}
		//dyld::log("trieWalk(%p) sym=%s, terminalSize=%lu, children=%p\n", start, s, terminalSize, children);
		uint8_t childrenRemaining = *children++;
		p = children;
		uintptr_t nodeOffset = 0;
		for (; childrenRemaining > 0; --childrenRemaining) {
			const char* ss = s;
			//dyld::log("trieWalk(%p) child str=%s\n", start, (char*)p);
			bool wrongEdge = false;
			// scan whole edge to get to next edge
			// if edge is longer than target symbol name, don't read past end of symbol name
			char c = *p;
			while ( c != '\0' ) {
				if ( !wrongEdge ) {
					if ( c != *ss )
						wrongEdge = true;
					++ss;
				}
				++p;
				c = *p;
			}
			if ( wrongEdge ) {
				// advance to next child
				++p; // skip over zero terminator
				// skip over uleb128 until last byte is found
				while ( (*p & 0x80) != 0 )
					++p;
				++p; // skip over last byte of uleb128
				if ( p > end ) {
					dyld::log("trieWalk() malformed trie node, child node extends past end of trie\n");
					return NULL;
				}
			}
			else {
 				// the symbol so far matches this edge (child)
				// so advance to the child's node
				++p;
				nodeOffset = read_uleb128(p, end);
				if ( (nodeOffset == 0) || ( &start[nodeOffset] > end) ) {
					dyld::log("trieWalk() malformed trie child, nodeOffset=0x%lx out of range\n", nodeOffset);
					return NULL;
				}
				s = ss;
				//dyld::log("trieWalk() found matching edge advancing to node 0x%lx\n", nodeOffset);
				break;
			}
		}
		if ( nodeOffset != 0 )
			p = &start[nodeOffset];
		else
			p = NULL;
	}
	//dyld::log("trieWalk(%p) return NULL\n", start);
	return NULL;
}


	
uintptr_t ImageLoader::read_uleb128(const uint8_t*& p, const uint8_t* end)
{
	uint64_t result = 0;
	int		 bit = 0;
	do {
		if (p == end)
			dyld::throwf("malformed uleb128");

		uint64_t slice = *p & 0x7f;

		if (bit > 63)
			dyld::throwf("uleb128 too big for uint64, bit=%d, result=0x%0llX", bit, result);
		else {
			result |= (slice << bit);
			bit += 7;
		}
	} while (*p++ & 0x80);
	return (uintptr_t)result;
}


intptr_t ImageLoader::read_sleb128(const uint8_t*& p, const uint8_t* end)
{
	int64_t result = 0;
	int bit = 0;
	uint8_t byte;
	do {
		if (p == end)
			throw "malformed sleb128";
		byte = *p++;
		result |= (((int64_t)(byte & 0x7f)) << bit);
		bit += 7;
	} while (byte & 0x80);
	// sign extend negative numbers
	if ( (byte & 0x40) != 0 )
		result |= (~0ULL) << bit;
	return (intptr_t)result;
}


VECTOR_NEVER_DESTRUCTED_IMPL(ImageLoader::InterposeTuple);
//VECTOR_NEVER_DESTRUCTED_IMPL(ImagePair);




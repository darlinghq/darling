/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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


#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <_simple.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <TargetConditionals.h>
#include <malloc/malloc.h>
#include <mach-o/dyld_priv.h>
#include <mach-o/dyld_images.h>

#include <algorithm>

#include "dlfcn.h"

#include "AllImages.h"
#include "Loading.h"
#include "Logging.h"
#include "Diagnostics.h"
#include "DyldSharedCache.h"
#include "APIs.h"


namespace dyld3 {

// from APIs.cpp
void                    parseDlHandle(void* h, const MachOLoaded** mh, bool* dontContinue);


// only in macOS and deprecated 
#if __MAC_OS_X_VERSION_MIN_REQUIRED

// macOS needs to support an old API that only works with fileype==MH_BUNDLE.
// In this deprecated API (unlike dlopen), loading and linking are separate steps.
// NSCreateObjectFileImageFrom*() just maps in the bundle mach-o file.
// NSLinkModule() does the load of dependent modules and rebasing/binding.
// To unload one of these, you must call NSUnLinkModule() and NSDestroyObjectFileImage() in any order!
//

NSObjectFileImageReturnCode NSCreateObjectFileImageFromFile(const char* path, NSObjectFileImage* ofi)
{
    log_apis("NSCreateObjectFileImageFromFile(\"%s\", %p)\n", path, ofi);

    // verify path exists
     struct stat statbuf;
    if ( ::stat(path, &statbuf) == -1 )
        return NSObjectFileImageFailure;

    // create ofi that just contains path. NSLinkModule does all the work
    OFIInfo result;
    result.path        = strdup(path);
    result.memSource   = nullptr;
    result.memLength   = 0;
    result.loadAddress = nullptr;
    result.imageNum    = 0;
    *ofi = gAllImages.addNSObjectFileImage(result);

    log_apis("NSCreateObjectFileImageFromFile() => %p\n", *ofi);

    return NSObjectFileImageSuccess;
}

NSObjectFileImageReturnCode NSCreateObjectFileImageFromMemory(const void* memImage, size_t memImageSize, NSObjectFileImage *ofi)
{
    log_apis("NSCreateObjectFileImageFromMemory(%p, 0x%0lX, %p)\n", memImage, memImageSize, ofi);

    // sanity check the buffer is a mach-o file
    __block Diagnostics diag;

	// check if it is current arch mach-o or fat with slice for current arch
    bool usable = false;
    const MachOFile* mf = (MachOFile*)memImage;
    if ( mf->hasMachOMagic() && mf->isMachO(diag, memImageSize) ) {
        usable = (gAllImages.archs().grade(mf->cputype, mf->cpusubtype) != 0);
    }
    else if ( const FatFile* ff = FatFile::isFatFile(memImage) ) {
        uint64_t sliceOffset;
        uint64_t sliceLen;
        bool     missingSlice;
        if ( ff->isFatFileWithSlice(diag, memImageSize, gAllImages.archs(), sliceOffset, sliceLen, missingSlice) ) {
            mf = (MachOFile*)((long)memImage+sliceOffset);
            if ( mf->isMachO(diag, sliceLen) ) {
                usable = true;
            }
        }
    }
    if ( usable ) {
        if ( !mf->supportsPlatform(Platform::macOS) )
            usable = false;
    }
    if ( !usable ) {
        log_apis("NSCreateObjectFileImageFromMemory() not mach-o\n");
        return NSObjectFileImageFailure;
    }

    // this API can only be used with bundles
    if ( !mf->isBundle() ) {
        log_apis("NSCreateObjectFileImageFromMemory() not a bundle\n");
        return NSObjectFileImageInappropriateFile;
    }

    // allocate ofi that just lists the memory range
    OFIInfo result;
    result.path        = nullptr;
    result.memSource   = memImage;
    result.memLength   = memImageSize;
    result.loadAddress = nullptr;
    result.imageNum    = 0;
    *ofi = gAllImages.addNSObjectFileImage(result);

    log_apis("NSCreateObjectFileImageFromMemory() => %p\n", *ofi);

    return NSObjectFileImageSuccess;
}

NSModule NSLinkModule(NSObjectFileImage ofi, const char* moduleName, uint32_t options)
{
    DYLD_LOAD_LOCK_THIS_BLOCK
    log_apis("NSLinkModule(%p, \"%s\", 0x%08X)\n", ofi, moduleName, options);

    __block const char* path = nullptr;
    bool foundImage = gAllImages.forNSObjectFileImage(ofi, ^(OFIInfo &image) {
        // if this is memory based image, write to temp file, then use file based loading
        if ( image.memSource != nullptr ) {
            // make temp file with content of memory buffer
            image.path = nullptr;
            char tempFileName[PATH_MAX];
            const char* tmpDir = getenv("TMPDIR");
            if ( (tmpDir != nullptr) && (strlen(tmpDir) > 2) ) {
                strlcpy(tempFileName, tmpDir, PATH_MAX);
                if ( tmpDir[strlen(tmpDir)-1] != '/' )
                    strlcat(tempFileName, "/", PATH_MAX);
            }
            else
                strlcpy(tempFileName,"/tmp/", PATH_MAX);
            strlcat(tempFileName, "NSCreateObjectFileImageFromMemory-XXXXXXXX", PATH_MAX);
            int fd = ::mkstemp(tempFileName);
            if ( fd != -1 ) {
                ssize_t writtenSize = ::pwrite(fd, image.memSource, image.memLength, 0);
                if ( writtenSize == image.memLength ) {
                    image.path = strdup(tempFileName);
                }
                else {
                    log_apis("NSLinkModule() => NULL (could not save memory image to temp file)\n");
                }
                ::close(fd);
            }
        }
        path = image.path;
    });

    if (!foundImage) {
        // ofi is invalid if not in list
        log_apis("NSLinkModule() => NULL (invalid NSObjectFileImage)\n");
        return nullptr;
    }

    if (!path)
        return nullptr;

    // dlopen the binary outside of the read lock as we don't want to risk deadlock
    Diagnostics diag;
    void* callerAddress = __builtin_return_address(1); // note layers: 1: real client, 0: libSystem glue
    const MachOLoaded* loadAddress = gAllImages.dlopen(diag, path, false, false, false, false, true, callerAddress);
    if ( diag.hasError() ) {
        log_apis("   NSLinkModule: failed: %s\n", diag.errorMessage());
        return nullptr;
    }

    // Now update the load address of this object
    gAllImages.forNSObjectFileImage(ofi, ^(OFIInfo &image) {
        image.loadAddress = loadAddress;

        // if memory based load, delete temp file
        if ( image.memSource != nullptr ) {
            log_apis("   NSLinkModule: delete temp file: %s\n", image.path);
            ::unlink(image.path);
        }
    });

    log_apis("NSLinkModule() => %p\n", loadAddress);
    return (NSModule)loadAddress;
}

// NSUnLinkModule unmaps the image, but does not release the NSObjectFileImage
bool NSUnLinkModule(NSModule module, uint32_t options)
{
    DYLD_LOAD_LOCK_THIS_BLOCK
    log_apis("NSUnLinkModule(%p, 0x%08X)\n", module, options);

    __block const mach_header* mh = nullptr;
    gAllImages.infoForImageMappedAt(module, ^(const LoadedImage& foundImage, uint8_t permissions) {
        mh = foundImage.loadedAddress();
    });

    if ( mh != nullptr )
        gAllImages.decRefCount(mh); // removes image if reference count went to zero

    log_apis("NSUnLinkModule() => %d\n", mh != nullptr);

    return mh != nullptr;
}

// NSDestroyObjectFileImage releases the NSObjectFileImage, but the mapped image may remain in use
bool NSDestroyObjectFileImage(NSObjectFileImage imageHandle)
{
    log_apis("NSDestroyObjectFileImage(%p)\n", imageHandle);

    __block const void* memSource = nullptr;
    __block size_t      memLength = 0;
    __block const char* path      = nullptr;
    bool foundImage = gAllImages.forNSObjectFileImage(imageHandle, ^(OFIInfo &image) {
        // keep copy of info
        memSource = image.memSource;
        memLength = image.memLength;
        path      = image.path;
    });

    if (!foundImage)
        return false;

    // remove from list
    gAllImages.removeNSObjectFileImage(imageHandle);

    // if object was created from a memory, release that memory
    // NOTE: this is the way dyld has always done this. NSCreateObjectFileImageFromMemory() hands ownership of the memory to dyld
    if ( memSource != nullptr ) {
        // we don't know if memory came from malloc or vm_allocate, so ask malloc
        if ( malloc_size(memSource) != 0 )
            free((void*)(memSource));
        else
            vm_deallocate(mach_task_self(), (vm_address_t)memSource, memLength);
    }
    free((void*)path);

    return true;
}

uint32_t NSSymbolDefinitionCountInObjectFileImage(NSObjectFileImage objectFileImage)
{
    halt("NSSymbolDefinitionCountInObjectFileImage() is obsolete");
}

const char* NSSymbolDefinitionNameInObjectFileImage(NSObjectFileImage objectFileImage, uint32_t ordinal)
{
    halt("NSSymbolDefinitionNameInObjectFileImage() is obsolete");
}

uint32_t NSSymbolReferenceCountInObjectFileImage(NSObjectFileImage objectFileImage)
{
    halt("NSSymbolReferenceCountInObjectFileImage() is obsolete");
}

const char* NSSymbolReferenceNameInObjectFileImage(NSObjectFileImage objectFileImage, uint32_t ordinal, bool *tentative_definition)
{
    halt("NSSymbolReferenceNameInObjectFileImage() is obsolete");
}

bool NSIsSymbolDefinedInObjectFileImage(NSObjectFileImage imageHandle, const char* symbolName)
{
    log_apis("NSIsSymbolDefinedInObjectFileImage(%p, %s)\n", imageHandle, symbolName);

    __block bool hasSymbol = false;
    bool foundImage = gAllImages.forNSObjectFileImage(imageHandle, ^(OFIInfo &image) {
        void* addr;
        bool resultPointsToInstructions = false;
        hasSymbol = image.loadAddress->hasExportedSymbol(symbolName, nullptr, &addr, &resultPointsToInstructions);
    });

    // ofi is invalid if not in list
    if (!foundImage)
        return false;

    return hasSymbol;
}

void* NSGetSectionDataInObjectFileImage(NSObjectFileImage imageHandle, const char* segmentName, const char* sectionName, size_t* size)
{
    __block const void* result = nullptr;
    bool foundImage = gAllImages.forNSObjectFileImage(imageHandle, ^(OFIInfo &image) {
        uint64_t sz;
        result = image.loadAddress->findSectionContent(segmentName, sectionName, sz);
        *size = (size_t)sz;
    });

    // ofi is invalid if not in list
    if (!foundImage)
        return nullptr;

	return (void*)result;
}

const char* NSNameOfModule(NSModule m)
{
    log_apis("NSNameOfModule(%p)\n", m);

    __block const char* result = nullptr;
    gAllImages.infoForImageMappedAt(m, ^(const LoadedImage& foundImage, uint8_t permissions) {
        result = gAllImages.imagePath(foundImage.image());
    });

    return result;
}

const char* NSLibraryNameForModule(NSModule m)
{
    log_apis("NSLibraryNameForModule(%p)\n", m);

     __block const char* result = nullptr;
    gAllImages.infoForImageMappedAt(m, ^(const LoadedImage& foundImage, uint8_t permissions) {
        result = gAllImages.imagePath(foundImage.image());
    });
    return result;
 }


static bool flatFindSymbol(const char* symbolName, void** symbolAddress, const mach_header** foundInImageAtLoadAddress)
{
    __block bool result = false;
    gAllImages.forEachImage(^(const LoadedImage& loadedImage, bool& stop) {
        bool resultPointsToInstructions = false;
        if ( loadedImage.loadedAddress()->hasExportedSymbol(symbolName, nullptr, symbolAddress, &resultPointsToInstructions) ) {
            *foundInImageAtLoadAddress = loadedImage.loadedAddress();
            stop = true;
            result = true;
        }
    });
    return result;
}

bool NSIsSymbolNameDefined(const char* symbolName)
{
    log_apis("NSIsSymbolNameDefined(%s)\n", symbolName);

    const mach_header* foundInImageAtLoadAddress;
    void* address;
    return flatFindSymbol(symbolName, &address, &foundInImageAtLoadAddress);
}

bool NSIsSymbolNameDefinedWithHint(const char* symbolName, const char* libraryNameHint)
{
    log_apis("NSIsSymbolNameDefinedWithHint(%s, %s)\n", symbolName, libraryNameHint);

    const mach_header* foundInImageAtLoadAddress;
    void* address;
    return flatFindSymbol(symbolName, &address, &foundInImageAtLoadAddress);
}

bool NSIsSymbolNameDefinedInImage(const struct mach_header* mh, const char* symbolName)
{
    log_apis("NSIsSymbolNameDefinedInImage(%p, %s)\n", mh, symbolName);

    void* addr;
    bool resultPointsToInstructions = false;
    return ((MachOLoaded*)mh)->hasExportedSymbol(symbolName, nullptr, &addr, &resultPointsToInstructions);
}

NSSymbol NSLookupAndBindSymbol(const char* symbolName)
{
    log_apis("NSLookupAndBindSymbol(%s)\n", symbolName);

    const mach_header* foundInImageAtLoadAddress;
    void* symbolAddress;
    if ( flatFindSymbol(symbolName, &symbolAddress, &foundInImageAtLoadAddress) ) {
        return (NSSymbol)symbolAddress;
    }
    return nullptr;
}

NSSymbol NSLookupAndBindSymbolWithHint(const char* symbolName, const char* libraryNameHint)
{
    log_apis("NSLookupAndBindSymbolWithHint(%s, %s)\n", symbolName, libraryNameHint);

    const mach_header* foundInImageAtLoadAddress;
    void* symbolAddress;
    if ( flatFindSymbol(symbolName, &symbolAddress, &foundInImageAtLoadAddress) ) {
        return (NSSymbol)symbolAddress;
    }
    return nullptr;
}

NSSymbol NSLookupSymbolInModule(NSModule module, const char* symbolName)
{
    log_apis("NSLookupSymbolInModule(%p. %s)\n", module, symbolName);

    const MachOLoaded* mh = (const MachOLoaded*)module;
    void* addr;
    bool resultPointsToInstructions = false;
    if ( mh->hasExportedSymbol(symbolName, nullptr, &addr, &resultPointsToInstructions) ) {
        return (NSSymbol)addr;
    }
    return nullptr;
}

NSSymbol NSLookupSymbolInImage(const mach_header* mh, const char* symbolName, uint32_t options)
{
    log_apis("NSLookupSymbolInImage(%p, \"%s\", 0x%08X)\n", mh, symbolName, options);

    void* addr;
    bool resultPointsToInstructions = false;
	if ( ((MachOLoaded*)mh)->hasExportedSymbol(symbolName, nullptr, &addr, &resultPointsToInstructions) ) {
        log_apis("   NSLookupSymbolInImage() => %p\n", addr);
        return (NSSymbol)addr;
	}
    if ( options & NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR ) {
        log_apis("   NSLookupSymbolInImage() => NULL\n");
        return nullptr;
    }
    // FIXME: abort();
    return nullptr;
}

const char* NSNameOfSymbol(NSSymbol symbol)
{
    halt("NSNameOfSymbol() is obsolete");
}

void* NSAddressOfSymbol(NSSymbol symbol)
{
    log_apis("NSAddressOfSymbol(%p)\n", symbol);

    // in dyld 1.0, NSSymbol was a pointer to the nlist entry in the symbol table
    return (void*)symbol;
}

NSModule NSModuleForSymbol(NSSymbol symbol)
{
    log_apis("NSModuleForSymbol(%p)\n", symbol);

    __block NSModule result = nullptr;
    gAllImages.infoForImageMappedAt(symbol, ^(const LoadedImage& foundImage, uint8_t permissions) {
        result = (NSModule)foundImage.loadedAddress();
    });

    return result;
}

void NSLinkEditError(NSLinkEditErrors *c, int *errorNumber, const char** fileName, const char** errorString)
{
    log_apis("NSLinkEditError(%p, %p, %p, %p)\n", c, errorNumber, fileName, errorString);
    *c = NSLinkEditOtherError;
    *errorNumber = 0;
    *fileName = NULL;
    *errorString = NULL;
}

bool NSAddLibrary(const char* pathName)
{
    log_apis("NSAddLibrary(%s)\n", pathName);

    void* callerAddress = __builtin_return_address(1); // note layers: 1: real client, 0: libSystem glue
    return ( dlopen_internal(pathName, 0, callerAddress) != nullptr);
}

bool NSAddLibraryWithSearching(const char* pathName)
{
    log_apis("NSAddLibraryWithSearching(%s)\n", pathName);

    void* callerAddress = __builtin_return_address(1); // note layers: 1: real client, 0: libSystem glue
    return ( dlopen_internal(pathName, 0, callerAddress) != nullptr);
}

const mach_header* NSAddImage(const char* imageName, uint32_t options)
{
    log_apis("NSAddImage(\"%s\", 0x%08X)\n", imageName, options);

    // Note: this is a quick and dirty implementation that just uses dlopen() and ignores some option flags
    uint32_t dloptions = 0;
    if ( (options & NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED) != 0 )
        dloptions |= RTLD_NOLOAD;

    void* callerAddress = __builtin_return_address(1); // note layers: 1: real client, 0: libSystem glue
    void* h = dlopen_internal(imageName, dloptions, callerAddress);
    if ( h != nullptr ) {
        const MachOLoaded* mh;
        bool dontContinue;
        parseDlHandle(h, &mh, &dontContinue);
        return mh;
    }

    if ( (options & (NSADDIMAGE_OPTION_RETURN_ON_ERROR|NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED)) == 0 ) {
        halt("NSAddImage() image not found");
    }
    return nullptr;
}

void NSInstallLinkEditErrorHandlers(const NSLinkEditErrorHandlers *handlers)
{
    halt("NSInstallLinkEditErrorHandlers() is obsolete");
}

bool _dyld_present(void)
{
    log_apis("_dyld_present()\n");

    return true;
}

bool _dyld_launched_prebound(void)  
{
    halt("_dyld_launched_prebound() is obsolete");
}

bool _dyld_all_twolevel_modules_prebound(void)
{
    halt("_dyld_all_twolevel_modules_prebound() is obsolete");
}

bool _dyld_bind_fully_image_containing_address(const void* address)
{
    log_apis("_dyld_bind_fully_image_containing_address(%p)\n", address);

    // in dyld3, everything is always fully bound
    return true;
}

bool _dyld_image_containing_address(const void* address)
{
    log_apis("_dyld_image_containing_address(%p)\n", address);

    return (dyld_image_header_containing_address(address) != nullptr);
}

void _dyld_lookup_and_bind(const char* symbolName, void **address, NSModule* module)
{
    log_apis("_dyld_lookup_and_bind(%s, %p, %p)\n", symbolName, address, module);

    const mach_header* foundInImageAtLoadAddress;
    if ( flatFindSymbol(symbolName, address, &foundInImageAtLoadAddress) ) {
        *module = (NSModule)foundInImageAtLoadAddress;
        return;
    }

    *address = 0;
    *module = 0;
}

void _dyld_lookup_and_bind_with_hint(const char* symbolName, const char* libraryNameHint, void** address, NSModule* module)
{
    log_apis("_dyld_lookup_and_bind_with_hint(%s, %s, %p, %p)\n", symbolName, libraryNameHint, address, module);

    const mach_header* foundInImageAtLoadAddress;
    if ( flatFindSymbol(symbolName, address, &foundInImageAtLoadAddress) ) {
        *module = (NSModule)foundInImageAtLoadAddress;
        return;
    }

    *address = 0;
    *module = 0;
}


void _dyld_lookup_and_bind_fully(const char* symbolName, void** address, NSModule* module)
{
    log_apis("_dyld_lookup_and_bind_fully(%s, %p, %p)\n", symbolName, address, module);

    const mach_header* foundInImageAtLoadAddress;
    if ( flatFindSymbol(symbolName, address, &foundInImageAtLoadAddress) ) {
        *module = (NSModule)foundInImageAtLoadAddress;
        return;
    }

    *address = 0;
    *module = 0;
}

const struct mach_header* _dyld_get_image_header_containing_address(const void* address)
{
    log_apis("_dyld_get_image_header_containing_address(%p)\n", address);

    return dyld_image_header_containing_address(address);
}

#endif


} // namespace dyld3


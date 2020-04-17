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


#include <stdint.h>
#include <assert.h>
#include <uuid/uuid.h>
#include <unistd.h>
#include <limits.h>
#include <mach-o/dyld_priv.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <System/machine/cpu_capabilities.h>

extern "C" {
  #include <corecrypto/ccdigest.h>
  #include <corecrypto/ccsha2.h>
}

#include "Closure.h"
#include "MachOFile.h"
#include "MachOLoaded.h"
#include "StringUtils.h"

#include "objc-shared-cache.h"


namespace dyld3 {
namespace closure {


////////////////////////////  TypedBytes ////////////////////////////////////////

const void* TypedBytes::payload() const
{
    return (uint8_t*)this + sizeof(TypedBytes);
}

void* TypedBytes::payload()
{
    return (uint8_t*)this + sizeof(TypedBytes);
}


////////////////////////////  ContainerTypedBytes ////////////////////////////////////////

const TypedBytes* ContainerTypedBytes::first() const
{
    return (TypedBytes*)payload();
}

const TypedBytes* ContainerTypedBytes::next(const TypedBytes* p) const
{
    assert((p->payloadLength & 0x3) == 0);
    return (TypedBytes*)((uint8_t*)(p->payload()) + p->payloadLength);
}

void ContainerTypedBytes::forEachAttribute(void (^handler)(const TypedBytes* typedBytes, bool& stop)) const
{
    assert(((long)this & 0x3) == 0);
    const TypedBytes* end = next(this);
    bool stop = false;
    for (const TypedBytes* p = first(); p < end && !stop; p = next(p)) {
        handler(p, stop);
    }
}

void ContainerTypedBytes::forEachAttributePayload(Type requestedType, void (^handler)(const void* payload, uint32_t size, bool& stop)) const
{
    forEachAttribute(^(const TypedBytes* typedBytes, bool& stop) {
        if ( (Type)(typedBytes->type) != requestedType )
            return;
        handler(typedBytes->payload(), typedBytes->payloadLength, stop);
    });
}

const void* ContainerTypedBytes::findAttributePayload(Type requestedType, uint32_t* payloadSize) const
{
    assert(((long)this & 0x3) == 0);
    if ( payloadSize != nullptr )
        *payloadSize = 0;
    const TypedBytes* end = next(this);
    bool stop = false;
    for (const TypedBytes* p = first(); p < end && !stop; p = next(p)) {
        if ( (Type)(p->type) == requestedType ) {
            if ( payloadSize != nullptr )
                *payloadSize = p->payloadLength;
            return p->payload();
        }
    }
    return nullptr;
}


////////////////////////////  Image ////////////////////////////////////////

const Image::Flags& Image::getFlags() const
{
    return *(Flags*)((uint8_t*)this + 2*sizeof(TypedBytes));
}

bool Image::isInvalid() const
{
    return getFlags().isInvalid;
}

size_t Image::size() const
{
    return sizeof(TypedBytes) + this->payloadLength;
}

ImageNum Image::imageNum() const
{
    return getFlags().imageNum;
}

// returns true iff 'num' is this image's ImageNum, or this image overrides that imageNum (in dyld cache)
bool Image::representsImageNum(ImageNum num) const
{
    const Flags& flags = getFlags();
    if ( flags.imageNum == num )
        return true;
    if ( !flags.isDylib )
        return false;
    if ( flags.inDyldCache )
        return false;
    ImageNum cacheImageNum;
    if ( isOverrideOfDyldCacheImage(cacheImageNum) )
        return (cacheImageNum == num);
    return false;
}

uint32_t Image::maxLoadCount() const
{
    return getFlags().maxLoadCount;
}

bool Image::isBundle() const
{
    return getFlags().isBundle;
}

bool Image::isDylib() const
{
    return getFlags().isDylib;
}

bool Image::isExecutable() const
{
    return getFlags().isExecutable;
}

bool Image::hasObjC() const
{
    return getFlags().hasObjC;
}

bool Image::is64() const
{
    return getFlags().is64;
}

bool Image::hasWeakDefs() const
{
    return getFlags().hasWeakDefs;
}

bool Image::mayHavePlusLoads() const
{
    return getFlags().mayHavePlusLoads;
}

bool Image::neverUnload() const
{
    return getFlags().neverUnload;
}

bool Image::overridableDylib() const
{
    return getFlags().overridableDylib;
}

bool Image::inDyldCache() const
{
    return getFlags().inDyldCache;
}

const char* Image::path() const
{
    // might be multiple pathWithHash enties, first is canonical name
    const PathAndHash* result = (PathAndHash*)findAttributePayload(Type::pathWithHash);
    assert(result && "Image missing pathWithHash");
    return result->path;
}

const char* Image::leafName() const
{
    uint32_t size;
    // might be multiple pathWithHash enties, first is canonical name
    const PathAndHash* result = (PathAndHash*)findAttributePayload(Type::pathWithHash, &size);
    assert(result && "Image missing pathWithHash");
    for (const char* p=(char*)result + size; p > result->path; --p) {
        if ( *p == '/' )
            return p+1;
    }
    return result->path;
}

bool Image::hasFileModTimeAndInode(uint64_t& inode, uint64_t& mTime) const
{
    uint32_t size;
    const FileInfo* info = (FileInfo*)(findAttributePayload(Type::fileInodeAndTime, &size));
    if ( info != nullptr ) {
        assert(size == sizeof(FileInfo));
        inode = info->inode;
        mTime = info->modTime;
        return true;
    }
    return false;
}

void Image::forEachCDHash(void (^handler)(const uint8_t cdHash[20], bool& stop)) const
{
    forEachAttribute(^(const TypedBytes* typedBytes, bool& stopLoop) {
        if ( (Type)(typedBytes->type) != Type::cdHash )
            return;
        assert(typedBytes->payloadLength == 20);
        const uint8_t* bytes = (const uint8_t*)typedBytes->payload();
        handler(bytes, stopLoop);
    });
}

bool Image::getUuid(uuid_t uuid) const
{
    uint32_t size;
    const uint8_t* bytes = (uint8_t*)(findAttributePayload(Type::uuid, &size));
    if ( bytes == nullptr )
        return false;
    assert(size == 16);
    memcpy(uuid, bytes, 16);
    return true;
}

bool Image::hasCodeSignature(uint32_t& sigFileOffset, uint32_t& sigSize) const
{
    uint32_t sz;
    const Image::CodeSignatureLocation* sigInfo = (Image::CodeSignatureLocation*)(findAttributePayload(Type::codeSignLoc, &sz));
    if ( sigInfo != nullptr ) {
        assert(sz == sizeof(Image::CodeSignatureLocation));
        sigFileOffset = sigInfo->fileOffset;
        sigSize       = sigInfo->fileSize;
        return true;
    }
    return false;
}

bool Image::isFairPlayEncrypted(uint32_t& textOffset, uint32_t& size) const
{
    uint32_t sz;
    const Image::FairPlayRange* fpInfo = (Image::FairPlayRange*)(findAttributePayload(Type::fairPlayLoc, &sz));
    if ( fpInfo != nullptr ) {
        assert(sz == sizeof(Image::FairPlayRange));
        textOffset = fpInfo->rangeStart;
        size       = fpInfo->rangeLength;
        return true;
    }
    return false;
}

const Array<Image::LinkedImage> Image::dependentsArray() const
{
    uint32_t size;
    LinkedImage* dependents = (LinkedImage*)findAttributePayload(Type::dependents, &size);
    assert((size % sizeof(LinkedImage)) == 0);
    uintptr_t count = size / sizeof(LinkedImage);
    return Array<Image::LinkedImage>(dependents, count, count);
}

void Image::forEachDependentImage(void (^handler)(uint32_t dependentIndex, LinkKind kind, ImageNum imageNum, bool& stop)) const
{
    uint32_t size;
    const LinkedImage* dependents = (LinkedImage*)findAttributePayload(Type::dependents, &size);
    assert((size % sizeof(LinkedImage)) == 0);
    const uint32_t count = size / sizeof(LinkedImage);
    bool stop = false;
    for (uint32_t i=0; (i < count) && !stop; ++i) {
        LinkKind kind     = dependents[i].kind();
        ImageNum imageNum = dependents[i].imageNum();
        // ignore missing weak links
        if ( (imageNum == kMissingWeakLinkedImage) && (kind == LinkKind::weak) )
            continue;
        handler(i, kind, imageNum, stop);
    }
}

ImageNum Image::dependentImageNum(uint32_t depIndex) const
{
    uint32_t size;
    const LinkedImage* dependents = (LinkedImage*)findAttributePayload(Type::dependents, &size);
    assert((size % sizeof(LinkedImage)) == 0);
    const uint32_t count = size / sizeof(LinkedImage);
    assert(depIndex < count);
    return dependents[depIndex].imageNum();
}


uint32_t Image::hashFunction(const char* str)
{
    uint32_t h = 0;
    for (const char* s=str; *s != '\0'; ++s)
        h = h*5 + *s;
    return h;
}

void Image::forEachAlias(void (^handler)(const char* aliasPath, bool& stop)) const
{
    __block bool foundFirst = false;
    forEachAttribute(^(const TypedBytes* typedBytes, bool& stopLoop) {
        if ( (Type)(typedBytes->type) != Type::pathWithHash )
            return;
        if ( foundFirst ) {
            const PathAndHash* aliasInfo = (PathAndHash*)typedBytes->payload();
            handler(aliasInfo->path, stopLoop);
        }
        else {
            foundFirst = true;
        }
    });
}

bool Image::hasPathWithHash(const char* path, uint32_t hash) const
{
    __block bool found = false;
    forEachAttribute(^(const TypedBytes* typedBytes, bool& stop) {
        if ( (Type)(typedBytes->type) != Type::pathWithHash )
            return;
        const PathAndHash* pathInfo = (PathAndHash*)typedBytes->payload();
        if ( (pathInfo->hash == hash) && (strcmp(path, pathInfo->path) == 0) ) {
            stop = true;
            found = true;
        }
    });
    return found;
}

void Image::forEachDiskSegment(void (^handler)(uint32_t segIndex, uint32_t fileOffset, uint32_t fileSize, int64_t vmOffset, uint64_t vmSize,
                                               uint8_t permissions, bool laterReadOnly, bool& stop)) const
{
    uint32_t size;
    const DiskSegment* segments = (DiskSegment*)findAttributePayload(Type::diskSegment, &size);
    assert(segments != nullptr);
    assert((size % sizeof(DiskSegment)) == 0);
    const uint32_t  count        = size / sizeof(DiskSegment);
    const uint32_t  pageSz       = pageSize();
    uint32_t        segIndex     = 0;
    uint32_t        fileOffset   = 0;
    int64_t         vmOffset     = 0;
    // decrement vmOffset by all segments before TEXT (e.g. PAGEZERO)
    for (uint32_t i=0; i < count; ++i) {
        const DiskSegment* seg = &segments[i];
        if ( seg->filePageCount != 0 ) {
            break;
        }
        vmOffset -= (uint64_t)seg->vmPageCount * pageSz;
    }
    // walk each segment and call handler
    bool stop = false;
    for (uint32_t i=0; i < count && !stop; ++i) {
        const DiskSegment* seg = &segments[i];
        uint64_t vmSize   = (uint64_t)seg->vmPageCount * pageSz;
        uint32_t fileSize = seg->filePageCount * pageSz;
        if ( !seg->paddingNotSeg ) {
            uint8_t perms   = seg->permissions;
            bool    laterRO = false;
            // read-only data segments are encoded as .w. , initially make them r/w
            if ( perms == Image::DiskSegment::kReadOnlyDataPermissions ) {
                perms   = VM_PROT_READ|VM_PROT_WRITE;
                laterRO = true;
            }
            handler(segIndex, ( fileSize == 0) ? 0 : fileOffset, fileSize, vmOffset, vmSize, perms, laterRO, stop);
            ++segIndex;
        }
        vmOffset   += vmSize;
        fileOffset += fileSize;
    }
}

uint32_t Image::pageSize() const
{
    if ( getFlags().has16KBpages )
        return 0x4000;
    else
        return 0x1000;
}

uint32_t Image::cacheOffset() const
{
    uint32_t size;
    const DyldCacheSegment* segments = (DyldCacheSegment*)findAttributePayload(Type::cacheSegment, &size);
    assert(segments != nullptr);
    assert((size % sizeof(DyldCacheSegment)) == 0);
    return segments[0].cacheOffset;
}

void Image::forEachCacheSegment(void (^handler)(uint32_t segIndex, uint64_t vmOffset, uint64_t vmSize, uint8_t permissions, bool& stop)) const
{
    uint32_t size;
    const DyldCacheSegment* segments = (DyldCacheSegment*)findAttributePayload(Type::cacheSegment, &size);
    assert(segments != nullptr);
    assert((size % sizeof(DyldCacheSegment)) == 0);
    const uint32_t  count = size / sizeof(DyldCacheSegment);
    bool stop = false;
    for (uint32_t i=0; i < count; ++i) {
        uint64_t vmOffset    = segments[i].cacheOffset - segments[0].cacheOffset;
        uint64_t vmSize      = segments[i].size;
        uint8_t  permissions = segments[i].permissions;
        handler(i, vmOffset, vmSize, permissions, stop);
        if ( stop )
            break;
    }
}

uint64_t Image::textSize() const
{
    __block uint64_t result = 0;
    if ( inDyldCache() ) {
        forEachCacheSegment(^(uint32_t segIndex, uint64_t vmOffset, uint64_t vmSize, uint8_t permissions, bool& stop) {
            result = vmSize;
            stop = true;
        });
    }
    else {
        forEachDiskSegment(^(uint32_t segIndex, uint32_t fileOffset, uint32_t fileSize, int64_t vmOffset, uint64_t vmSize, uint8_t permissions, bool laterReadOnly, bool& stop) {
            if ( permissions != 0) {
                result = vmSize;
                stop = true;
            }
        });
    }
    return result;
}

bool Image::containsAddress(const void* addr, const void* imageLoadAddress, uint8_t* permsResult) const
{
    __block bool  result     = false;
    uint64_t      targetAddr = (uint64_t)addr;
    uint64_t      imageStart = (uint64_t)imageLoadAddress;
    if ( inDyldCache() ) {
        forEachCacheSegment(^(uint32_t segIndex, uint64_t vmOffset, uint64_t vmSize, uint8_t permissions, bool& stop) {
            if ( (targetAddr >= imageStart+vmOffset) && (targetAddr < imageStart+vmOffset+vmSize) ) {
                result = true;
                if ( permsResult )
                    *permsResult = permissions;
                stop = true;
            }
        });
    }
    else {
        forEachDiskSegment(^(uint32_t segIndex, uint32_t fileOffset, uint32_t fileSize, int64_t vmOffset, uint64_t vmSize, uint8_t permissions, bool laterReadOnly, bool& stop) {
            if ( (targetAddr >= imageStart+vmOffset) && (targetAddr < imageStart+vmOffset+vmSize) ) {
                result = true;
                if ( permsResult )
                    *permsResult = permissions;
                stop = true;
            }
        });
    }
    return result;
}

uint64_t Image::vmSizeToMap() const
{
    uint32_t size;
    const Image::MappingInfo* info = (Image::MappingInfo*)(findAttributePayload(Type::mappingInfo, &size));
    assert(info != nullptr);
    assert(size == sizeof(Image::MappingInfo));
    return info->totalVmPages * pageSize();
}

uint64_t Image::sliceOffsetInFile() const
{
    uint32_t size;
    const Image::MappingInfo* info = (Image::MappingInfo*)(findAttributePayload(Type::mappingInfo, &size));
    assert(info != nullptr);
    assert(size == sizeof(Image::MappingInfo));
    return info->sliceOffsetIn4K * 0x1000;
}

void Image::forEachInitializer(const void* imageLoadAddress, void (^handler)(const void* initializer)) const
{
    uint32_t size;
    const uint32_t* inits = (uint32_t*)findAttributePayload(Type::initOffsets, &size);
    if ( inits != nullptr ) {
        assert((size % sizeof(uint32_t)) == 0);
        const uint32_t count = size / sizeof(uint32_t);
        for (uint32_t i=0; i < count; ++i) {
            uint32_t offset = inits[i];
            const void* init = (void*)((uint8_t*)imageLoadAddress + offset);
            handler(init);
        }
        return;
    }
    const Image::InitializerSectionRange* range = (Image::InitializerSectionRange*)findAttributePayload(Type::initsSection, &size);
    if ( range != nullptr ) {
        const uint32_t pointerSize = is64() ? 8 : 4;
        const uint32_t* start32 = (uint32_t*)((uint8_t*)imageLoadAddress + range->sectionOffset);
        const uint64_t* start64 = (uint64_t*)((uint8_t*)imageLoadAddress + range->sectionOffset);
        const uint32_t count = range->sectionSize / pointerSize;
        for (uint32_t i=0; i < count; ++i) {
            if ( pointerSize == 8 )
                handler((void*)(long)(start64[i]));
            else
                handler((void*)(long)(start32[i]));
        }
    }
}

bool Image::forEachInitializerSection(void (^handler)(uint32_t sectionOffset, uint32_t sectionSize)) const
{
    __block bool result = false;
    forEachAttributePayload(Type::initsSection, ^(const void* payload, uint32_t size, bool& stop) {
        const Image::InitializerSectionRange* range = (Image::InitializerSectionRange*)payload;
        assert((size % sizeof(Image::InitializerSectionRange)) == 0);
        handler(range->sectionOffset, range->sectionSize);
        result = true;
    });
    return result;
}

bool Image::hasInitializers() const
{
    uint32_t size;
    return ( findAttributePayload(Type::initOffsets, &size) != nullptr );
}

bool Image::hasTerminators() const
{
    return getFlags().hasTerminators;
}

bool Image::hasReadOnlyData() const
{
    return getFlags().hasReadOnlyData;
}

bool Image::hasChainedFixups() const
{
    return getFlags().hasChainedFixups;
}

bool Image::hasPrecomputedObjC() const
{
    return getFlags().hasPrecomputedObjC;
}

void Image::forEachTerminator(const void* imageLoadAddress, void (^handler)(const void* terminator)) const
{
    uint32_t size;
    const uint32_t* terms = (uint32_t*)findAttributePayload(Type::termOffsets, &size);
    if ( terms != nullptr ) {
        assert((size % sizeof(uint32_t)) == 0);
        const uint32_t count = size / sizeof(uint32_t);
        for (uint32_t i=0; i < count; ++i) {
            uint32_t offset = terms[i];
            const void* term = (void*)((uint8_t*)imageLoadAddress + offset);
            handler(term);
        }
    }
}

void Image::forEachDOF(const void* imageLoadAddress, void (^handler)(const void* dofSection)) const
{
    uint32_t size;
    const uint32_t* dofs = (uint32_t*)findAttributePayload(Type::dofOffsets, &size);
    if ( dofs != nullptr ) {
        assert((size % sizeof(uint32_t)) == 0);
        const uint32_t count = size / sizeof(uint32_t);
        for (uint32_t i=0; i < count; ++i) {
            uint32_t offset = dofs[i];
            const void* sect = (void*)((uint8_t*)imageLoadAddress + offset);
            handler(sect);
        }
    }
}

void Image::forEachFixup(void (^rebase)(uint64_t imageOffsetToRebase, bool& stop),
                         void (^bind)(uint64_t imageOffsetToBind, ResolvedSymbolTarget bindTarget, bool& stop),
                         void (^chainedFixups)(uint64_t imageOffsetToStarts, const Array<ResolvedSymbolTarget>& targets, bool& stop),
                         void (^fixupObjCImageInfo)(uint64_t imageOffsetToFixup),
                         void (^fixupObjCProtocol)(uint64_t imageOffsetToBind, ResolvedSymbolTarget bindTarget, bool& stop),
                         void (^fixupObjCSelRef)(uint64_t imageOffsetToFixup, uint32_t selectorIndex, bool inSharedCache, bool& stop),
                         void (^fixupObjCStableSwift)(uint64_t imageOffsetToFixup, bool& stop),
                         void (^fixupObjCMethodList)(uint64_t imageOffsetToFixup, bool& stop)) const
{
    const uint32_t pointerSize = is64() ? 8 : 4;
	uint64_t curRebaseOffset = 0;
	bool stop = false;
    for (const Image::RebasePattern& rebasePat : rebaseFixups()) {
        //fprintf(stderr, " repeat=0x%04X, contig=%d, skip=%d\n", rebasePat.repeatCount, rebasePat.contigCount, rebasePat.skipCount);
        if ( rebasePat.contigCount == 0 ) {
            // note: contigCount==0 means this just advances location
            if ( (rebasePat.repeatCount == 0) && (rebasePat.skipCount == 0) ) {
                // all zeros is special pattern that means reset to rebase offset to zero
                curRebaseOffset = 0;
            }
            else {
                curRebaseOffset += rebasePat.repeatCount * rebasePat.skipCount;
            }
        }
        else {
            for (int r=0; r < rebasePat.repeatCount && !stop; ++r) {
                for (int i=0; i < rebasePat.contigCount && !stop; ++i) {
                    //fprintf(stderr, "  0x%08llX\n", curRebaseOffset);
                    rebase(curRebaseOffset, stop);
                    curRebaseOffset += pointerSize;
                }
                curRebaseOffset += pointerSize * rebasePat.skipCount;
            }
        }
        if ( stop )
            break;
    }
    if ( stop )
        return;

    for (const Image::BindPattern& bindPat : bindFixups()) {
        uint64_t curBindOffset = bindPat.startVmOffset;
        for (uint16_t i=0; i < bindPat.repeatCount; ++i) {
            bind(curBindOffset, bindPat.target, stop);
            curBindOffset += (pointerSize * (1 + bindPat.skipCount));
            if ( stop )
                break;
        }
        if ( stop )
            break;
    }

    if (hasChainedFixups())
        chainedFixups(chainedStartsOffset(), chainedTargets(), stop);

    if ( hasPrecomputedObjC() ) {
        ResolvedSymbolTarget objcProtocolClassTarget;
        uint64_t objcImageInfoVMOffset = 0;
        Array<ProtocolISAFixup> protocolISAFixups;
        Array<Image::SelectorReferenceFixup> selRefFixupEntries;
        Array<Image::ClassStableSwiftFixup> classStableSwiftFixups;
        Array<Image::MethodListFixup> methodListFixups;
        objcFixups(objcProtocolClassTarget, objcImageInfoVMOffset, protocolISAFixups,
                   selRefFixupEntries, classStableSwiftFixups, methodListFixups);

        // Set the objc image info bit to tell libobjc we are optimized
        fixupObjCImageInfo(objcImageInfoVMOffset);


        // First bind all the protocols to the same Protocol class in libobjc
        for (const Image::ProtocolISAFixup& bindPat : protocolISAFixups) {
            uint64_t curBindOffset = bindPat.startVmOffset;
            for (uint16_t i=0; i < bindPat.repeatCount; ++i) {
                fixupObjCProtocol(curBindOffset, objcProtocolClassTarget, stop);
                curBindOffset += (pointerSize * (1 + bindPat.skipCount));
                if ( stop )
                    break;
            }
            if ( stop )
                break;
        }

        for (uintptr_t i = 0, e = selRefFixupEntries.count(); i != e; ++i) {
            Image::SelectorReferenceFixup fixupEntry = selRefFixupEntries[i];
            // Start a new chain
            uint64_t curFixupOffset = fixupEntry.chainStartVMOffset;
            // Now walk the chain until we get a 'next' of 0
            while (i != e) {
                fixupEntry = selRefFixupEntries[++i];
                fixupObjCSelRef(curFixupOffset, fixupEntry.chainEntry.index, fixupEntry.chainEntry.inSharedCache, stop);
                if ( stop )
                    break;
                if ( fixupEntry.chainEntry.next == 0 )
                    break;
                curFixupOffset += (4 * fixupEntry.chainEntry.next);
            }
        }

        // Set classes to have stable Swift
        for (const Image::ClassStableSwiftFixup& bindPat : classStableSwiftFixups) {
            uint64_t curBindOffset = bindPat.startVmOffset;
            for (uint16_t i=0; i < bindPat.repeatCount; ++i) {
                fixupObjCStableSwift(curBindOffset, stop);
                curBindOffset += (pointerSize * (1 + bindPat.skipCount));
                if ( stop )
                    break;
            }
            if ( stop )
                break;
        }

        // Set method lists to be fixed up
        for (const Image::MethodListFixup& bindPat : methodListFixups) {
            uint64_t curBindOffset = bindPat.startVmOffset;
            for (uint16_t i=0; i < bindPat.repeatCount; ++i) {
                fixupObjCMethodList(curBindOffset, stop);
                curBindOffset += (pointerSize * (1 + bindPat.skipCount));
                if ( stop )
                    break;
            }
            if ( stop )
                break;
        }
    }
}

void Image::forEachTextReloc(void (^rebase)(uint32_t imageOffsetToRebase, bool& stop),
                             void (^bind)(uint32_t imageOffsetToBind, ResolvedSymbolTarget bindTarget, bool& stop)) const
{
    bool stop = false;
    const Array<Image::TextFixupPattern> f = textFixups();
    for (const Image::TextFixupPattern& pat : f) {
        uint32_t curOffset = pat.startVmOffset;
        for (uint16_t i=0; i < pat.repeatCount; ++i) {
            if ( pat.target.raw == 0 )
                rebase(curOffset, stop);
            else
                bind(curOffset, pat.target, stop);
            curOffset += pat.skipCount;
        }
    }
}

const Array<Image::RebasePattern> Image::rebaseFixups() const
{
    uint32_t rebaseFixupsSize;
    Image::RebasePattern* rebaseFixupsContent = (RebasePattern*)findAttributePayload(Type::rebaseFixups, &rebaseFixupsSize);
    uint32_t rebaseCount = rebaseFixupsSize/sizeof(RebasePattern);
    return Array<RebasePattern>(rebaseFixupsContent, rebaseCount, rebaseCount);
}

const Array<Image::BindPattern> Image::bindFixups() const
{
    uint32_t bindFixupsSize;
    BindPattern* bindFixupsContent = (BindPattern*)findAttributePayload(Type::bindFixups, &bindFixupsSize);
    uint32_t bindCount = bindFixupsSize/sizeof(BindPattern);
    return Array<BindPattern>(bindFixupsContent, bindCount, bindCount);
}

uint64_t Image::chainedStartsOffset() const
{
    uint32_t size;
    uint64_t* startsOffset = (uint64_t*)findAttributePayload(Type::chainedStartsOffset, &size);
    if ( startsOffset == nullptr )
        return 0; // means no pre-computed offset to starts table
    assert(size == sizeof(uint64_t));
    return *startsOffset;
}
    
void Image::objcFixups(ResolvedSymbolTarget& objcProtocolClassTarget,
                       uint64_t& objcImageInfoVMOffset,
                       Array<ProtocolISAFixup>& protocolISAFixups,
                       Array<SelectorReferenceFixup>& selRefFixups,
                       Array<ClassStableSwiftFixup>& classStableSwiftFixups,
                       Array<MethodListFixup>& methodListFixups) const
{
    // The layout here is:
    //   ResolvedSymbolTarget
    //   uint64_t vmOffset to objc_imageinfo
    //   uint32_t protocol count
    //   uint32_t selector reference count
    //   array of ProtocolISAFixup
    //   array of SelectorReferenceFixup
    //   optional uint32_t stable swift fixup count
    //   optional uint32_t method list fixup count
    //   optional array of ClassStableSwiftFixup
    //   optional array of MethodListFixup

    if (!hasPrecomputedObjC())
        return;

    uint32_t contentSize;
    const uint8_t* fixupsContent = (uint8_t*)findAttributePayload(Type::objcFixups, &contentSize);
    const uint8_t* fixupsContentEnd = fixupsContent + contentSize;

    // Get the statically sized data
    uint32_t protocolFixupCount = 0;
    uint32_t selRefFixupCount = 0;
    memcpy(&objcProtocolClassTarget, fixupsContent, sizeof(ResolvedSymbolTarget));
    fixupsContent += sizeof(ResolvedSymbolTarget);
    memcpy(&objcImageInfoVMOffset, fixupsContent, sizeof(uint64_t));
    fixupsContent += sizeof(uint64_t);
    memcpy(&protocolFixupCount, fixupsContent, sizeof(uint32_t));
    fixupsContent += sizeof(uint32_t);
    memcpy(&selRefFixupCount, fixupsContent, sizeof(uint32_t));
    fixupsContent += sizeof(uint32_t);

    // Get the protocol fixups
    if ( protocolFixupCount != 0) {
        protocolISAFixups = Array<ProtocolISAFixup>((ProtocolISAFixup*)fixupsContent, protocolFixupCount, protocolFixupCount);
        fixupsContent += (sizeof(ProtocolISAFixup) * protocolFixupCount);
    }

    // Get the selector reference fixups
    if ( selRefFixupCount != 0) {
        selRefFixups = Array<SelectorReferenceFixup>((SelectorReferenceFixup*)fixupsContent, selRefFixupCount, selRefFixupCount);
        fixupsContent += (sizeof(SelectorReferenceFixup) * selRefFixupCount);
    }

    // Old closures end here, but newer ones might have additional fixups
    if (fixupsContent == fixupsContentEnd)
        return;

    uint32_t stableSwiftFixupCount = 0;
    uint32_t methodListFixupCount = 0;
    memcpy(&stableSwiftFixupCount, fixupsContent, sizeof(uint32_t));
    fixupsContent += sizeof(uint32_t);
    memcpy(&methodListFixupCount, fixupsContent, sizeof(uint32_t));
    fixupsContent += sizeof(uint32_t);

    // Get the stable swift fixups
    if ( stableSwiftFixupCount != 0) {
        classStableSwiftFixups = Array<ClassStableSwiftFixup>((ClassStableSwiftFixup*)fixupsContent, stableSwiftFixupCount, stableSwiftFixupCount);
        fixupsContent += (sizeof(ClassStableSwiftFixup) * stableSwiftFixupCount);
    }

    // Get the method list fixups
    if ( methodListFixupCount != 0) {
        methodListFixups = Array<MethodListFixup>((MethodListFixup*)fixupsContent, methodListFixupCount, methodListFixupCount);
        fixupsContent += (sizeof(MethodListFixup) * methodListFixupCount);
    }
}

const Array<Image::ResolvedSymbolTarget> Image::chainedTargets() const
{
    uint32_t size;
    ResolvedSymbolTarget* targetsContent = (ResolvedSymbolTarget*)findAttributePayload(Type::chainedFixupsTargets, &size);
    uint32_t count = size/sizeof(ResolvedSymbolTarget);
    return Array<ResolvedSymbolTarget>(targetsContent, count, count);
}

const Array<Image::TextFixupPattern> Image::textFixups() const
{
    uint32_t fixupsSize;
    TextFixupPattern* fixupsContent = (TextFixupPattern*)findAttributePayload(Type::textFixups, &fixupsSize);
    uint32_t count = fixupsSize/sizeof(TextFixupPattern);
    return Array<TextFixupPattern>(fixupsContent, count, count);
}

bool Image::isOverrideOfDyldCacheImage(ImageNum& imageNum) const
{
	uint32_t size;
	const uint32_t* content = (uint32_t*)findAttributePayload(Type::imageOverride, &size);
	if ( content != nullptr ) {
        assert(size == sizeof(uint32_t));
        imageNum = *content;
        return true;
    }
    return false;
}

void Image::forEachImageToInitBefore(void (^handler)(ImageNum imageToInit, bool& stop)) const
{
    uint32_t size;
    const ImageNum* initBefores = (ImageNum*)findAttributePayload(Type::initBefores, &size);
    if ( initBefores != nullptr ) {
        assert((size % sizeof(ImageNum)) == 0);
        const uint32_t count = size / sizeof(ImageNum);
        bool stop = false;
        for (uint32_t i=0; (i < count) && !stop; ++i) {
            handler(initBefores[i], stop);
        }
    }
}

////////////////////////////  ImageArray ////////////////////////////////////////

size_t ImageArray::size() const
{
    return sizeof(TypedBytes) + this->payloadLength;
}

size_t ImageArray::startImageNum() const
{
    return firstImageNum;
}

uint32_t ImageArray::imageCount() const
{
    return count;
}

void ImageArray::forEachImage(void (^callback)(const Image* image, bool& stop)) const
{
    bool stop = false;
    for (uint32_t i=0; i < count && !stop; ++i) {
        const Image* image = (Image*)((uint8_t*)payload() + offsets[i]);
        callback(image, stop);
        if (stop)
            break;
    }
}

bool ImageArray::hasPath(const char* path, ImageNum& num) const
{
    const uint32_t hash = Image::hashFunction(path);
    __block bool found = false;
    forEachImage(^(const Image* image, bool& stop) {
        if ( image->hasPathWithHash(path, hash) ) {
            num   = image->imageNum();
            found = true;
            stop  = true;
        }
    });
    return found;
}

const Image* ImageArray::imageForNum(ImageNum num) const
{
    if (hasRoots) {
        __block const Image* foundImage = nullptr;
        forEachImage(^(const Image *image, bool &stop) {
            if (image->imageNum() == num) {
                foundImage = image;
                stop = true;
            }
        });
        return foundImage;
    }
    if ( num < firstImageNum )
        return nullptr;

    uint32_t index = num - firstImageNum;
    if ( index >= count )
        return nullptr;

    return (Image*)((uint8_t*)payload() + offsets[index]);
}

const Image* ImageArray::findImage(const Array<const ImageArray*> imagesArrays, ImageNum imageNum)
{
   for (const ImageArray* ia : imagesArrays) {
        if ( const Image* result = ia->imageForNum(imageNum) )
            return result;
    }
    return nullptr;
}

void ImageArray::deallocate() const
{
    ::vm_deallocate(mach_task_self(), (long)this, size());
}

////////////////////////////  Closure ////////////////////////////////////////

size_t Closure::size() const
{
    return sizeof(TypedBytes) + this->payloadLength;
}

const ImageArray* Closure::images() const
{
    __block const TypedBytes* result = nullptr;
    forEachAttribute(^(const TypedBytes* typedBytes, bool& stop) {
        if ( (Type)(typedBytes->type) == Type::imageArray ) {
            result = typedBytes;
            stop = true;
        }
    });

    return (ImageArray*)result;
}

ImageNum Closure::topImage() const
{
    uint32_t size;
    const ImageNum* top = (ImageNum*)findAttributePayload(Type::topImage, &size);
    assert(top != nullptr);
    assert(size == sizeof(ImageNum));
    return *top;
}

void Closure::forEachPatchEntry(void (^handler)(const PatchEntry& entry)) const
{
	forEachAttributePayload(Type::cacheOverrides, ^(const void* payload, uint32_t size, bool& stop) {
        assert((size % sizeof(Closure::PatchEntry)) == 0);
        const PatchEntry* patches    = (PatchEntry*)payload;
        const PatchEntry* patchesEnd = (PatchEntry*)((char*)payload + size);
        for (const PatchEntry* p=patches; p < patchesEnd; ++p)
            handler(*p);
	});
}

void Closure::forEachWarning(Closure::Warning::Type type, void (^handler)(const char* warning, bool& stop)) const
{
    forEachAttributePayload(Type::warning, ^(const void* payload, uint32_t size, bool& stop) {
        const Closure::Warning* warning = (const Closure::Warning*)payload;
        if ( warning->type != type )
            return;
        handler(warning->message, stop);
    });
}

void Closure::deallocate() const
{
    ::vm_deallocate(mach_task_self(), (long)this, size());
}

////////////////////////////  LaunchClosure ////////////////////////////////////////

void LaunchClosure::forEachMustBeMissingFile(void (^handler)(const char* path, bool& stop)) const
{
    uint32_t size;
    const char* paths = (const char*)findAttributePayload(Type::missingFiles, &size);
    bool stop = false;
    for (const char* s=paths; s < &paths[size]; ++s) {
        if ( *s != '\0' )
            handler(s, stop);
        if ( stop )
            break;
        s += strlen(s);
    }
}

void LaunchClosure::forEachSkipIfExistsFile(void (^handler)(const SkippedFile& file, bool& stop)) const
{
    uint32_t size;
    const uint64_t* files = (const uint64_t*)findAttributePayload(Type::existingFiles, &size);
    if (files == nullptr)
        return;

    // The first entry is the length of the array
    uint64_t fileCount = *files++;

    // Followed by count number of mod times and inodes
    const char* paths = (const char*)(files + (2 * fileCount));
    bool stop = false;
    for (const char* s=paths; s < &paths[size]; ++s) {
        if ( *s != '\0' ) {
            uint64_t inode = *files++;
            uint64_t mtime = *files++;
            SkippedFile skippedFile = { s, inode, mtime };
            handler(skippedFile, stop);
        }
        if ( stop )
            break;
        s += strlen(s);
    }
}

bool LaunchClosure::builtAgainstDyldCache(uuid_t cacheUUID) const
{
    uint32_t size;
    const uint8_t* uuidBytes = (uint8_t*)findAttributePayload(Type::dyldCacheUUID, &size);
    if ( uuidBytes == nullptr )
        return false;
    assert(size == sizeof(uuid_t));
    memcpy(cacheUUID, uuidBytes, sizeof(uuid_t));
    return true;
}

const char* LaunchClosure::bootUUID() const
{
    uint32_t size;
    return (char*)findAttributePayload(Type::bootUUID, &size);
}

void LaunchClosure::forEachEnvVar(void (^handler)(const char* keyEqualValue, bool& stop)) const
{
    forEachAttributePayload(Type::envVar, ^(const void* payload, uint32_t size, bool& stop) {
        handler((char*)payload, stop);
    });
}

ImageNum LaunchClosure::libSystemImageNum() const
{
    uint32_t size;
    const ImageNum* num = (ImageNum*)findAttributePayload(Type::libSystemNum, &size);
    assert(num != nullptr);
    assert(size == sizeof(ImageNum));
    return *num;
}

void LaunchClosure::libDyldEntry(Image::ResolvedSymbolTarget& loc) const
{
    uint32_t size;
    const Image::ResolvedSymbolTarget* data = (Image::ResolvedSymbolTarget*)findAttributePayload(Type::libDyldEntry, &size);
    assert(data != nullptr);
    assert(size == sizeof(Image::ResolvedSymbolTarget));
    loc = *data;
}

bool LaunchClosure::mainEntry(Image::ResolvedSymbolTarget& mainLoc) const
{
    uint32_t size;
    const Image::ResolvedSymbolTarget* data = (Image::ResolvedSymbolTarget*)findAttributePayload(Type::mainEntry, &size);
    if ( data == nullptr )
        return false;
    assert(size == sizeof(Image::ResolvedSymbolTarget));
    mainLoc = *data;
    return true;
}

bool LaunchClosure::startEntry(Image::ResolvedSymbolTarget& startLoc) const
{
    uint32_t size;
    const Image::ResolvedSymbolTarget* data = (Image::ResolvedSymbolTarget*)findAttributePayload(Type::startEntry, &size);
    if ( data == nullptr )
        return false;
    assert(size == sizeof(Image::ResolvedSymbolTarget));
    startLoc = *data;
    return true;
}

const LaunchClosure::Flags& LaunchClosure::getFlags() const
{
    uint32_t size;
    const Flags* flags = (Flags*)findAttributePayload(Type::closureFlags, &size);
    assert(flags != nullptr && "Closure missing Flags");
    return *flags;
}

uint32_t LaunchClosure::initialLoadCount() const
{
    return getFlags().initImageCount;
}

bool LaunchClosure::usedAtPaths() const
{
    return getFlags().usedAtPaths;
}

bool LaunchClosure::usedFallbackPaths() const
{
	return getFlags().usedFallbackPaths;
}

bool LaunchClosure::hasInsertedLibraries() const
{
    return getFlags().hasInsertedLibraries;
}

bool LaunchClosure::hasInterposings() const
{
    __block bool result = false;

    forEachInterposingTuple(^(const InterposingTuple&, bool& stop) {
        result = true;
        stop = true;
    });

    return result;
}

void LaunchClosure::forEachInterposingTuple(void (^handler)(const InterposingTuple& tuple, bool& stop)) const
{
	forEachAttributePayload(Type::interposeTuples, ^(const void* payload, uint32_t size, bool& stop) {
        assert((size % sizeof(InterposingTuple)) == 0);
        uintptr_t count = size / sizeof(InterposingTuple);
        const InterposingTuple* tuples = (InterposingTuple*)payload;
        for (uint32_t i=0; i < count && !stop; ++i) {
            handler(tuples[i], stop);
        }
	});
}
bool LaunchClosure::selectorHashTable(Array<Image::ObjCSelectorImage>& imageNums,
                                      const closure::ObjCSelectorOpt*& hashTable) const {
    uint32_t payloadSize = 0;
    const uint8_t* buffer = (const uint8_t*)findAttributePayload(Type::selectorTable, &payloadSize);
    if (buffer == nullptr)
        return false;

    // Get count
    uint32_t count = 0;
    memcpy(&count, buffer, sizeof(uint32_t));
    buffer += sizeof(uint32_t);

    // Get image nums
    imageNums = Array<Image::ObjCSelectorImage>((Image::ObjCSelectorImage*)buffer, count, count);
    buffer += sizeof(Image::ObjCSelectorImage) * count;

    // Get hash table
    hashTable = (const closure::ObjCSelectorOpt*)buffer;

    return true;
}

bool LaunchClosure::classAndProtocolHashTables(Array<Image::ObjCClassImage>& imageNums,
                                               const ObjCClassOpt*& classHashTable,
                                               const ObjCClassOpt*& protocolHashTable) const {
    // The layout here is:
    //   uint32_t offset to class table (note this is 0 if there are no classes)
    //   uint32_t offset to protocol table (note this is 0 if there are no protocols)
    //   uint32_t num images
    //   ObjCClassImage[num images]
    //   class hash table
    //   [ padding to 4-byte alignment if needed
    //   protocol hash table
    //   [ padding to 4-byte alignment if needed

    uint32_t payloadSize = 0;
    const uint8_t* buffer = (const uint8_t*)findAttributePayload(Type::classTable, &payloadSize);
    if (buffer == nullptr)
        return false;

    uint32_t headerSize = sizeof(uint32_t) * 3;

    uint32_t offsetToClassTable = 0;
    uint32_t offsetToProtocolTable = 0;
    uint32_t numImages = 0;
    
    // Get the header
    memcpy(&offsetToClassTable,     buffer + 0, sizeof(uint32_t));
    memcpy(&offsetToProtocolTable,  buffer + 4, sizeof(uint32_t));
    memcpy(&numImages,              buffer + 8, sizeof(uint32_t));

    // Get the image nums
    imageNums = Array<Image::ObjCClassImage>((Image::ObjCClassImage*)(buffer + headerSize), numImages, numImages);

    // Get the class hash table if there is one
    if ( offsetToClassTable != 0 )
        classHashTable = (const ObjCClassOpt*)(buffer + offsetToClassTable);

    // Write out out the protocol hash table if there is one
    if ( offsetToProtocolTable != 0 )
        protocolHashTable = (const ObjCClassOpt*)(buffer + offsetToProtocolTable);
    
    return true;
}

void LaunchClosure::duplicateClassesHashTable(const ObjCClassDuplicatesOpt*& duplicateClassesHashTable) const {
    uint32_t payloadSize = 0;
    const uint8_t* buffer = (const uint8_t*)findAttributePayload(Type::duplicateClassesTable, &payloadSize);
    if (buffer == nullptr)
        return;

    duplicateClassesHashTable = (const ObjCClassDuplicatesOpt*)buffer;
}

static void putHexNibble(uint8_t value, char*& p)
{
    if ( value < 10 )
        *p++ = '0' + value;
    else
        *p++ = 'A' + value - 10;
}

static void putHexByte(uint8_t value, char*& p)
{
    value &= 0xFF;
    putHexNibble(value >> 4,   p);
    putHexNibble(value & 0x0F, p);
}

static bool hashBootAndFileInfo(const char* mainExecutablePath, char hashString[32])
{
    struct stat statbuf;
    if ( ::stat(mainExecutablePath, &statbuf) != 0)
        return false;
#if !DARLING && !TARGET_OS_DRIVERKIT // Temp until core crypto is available
    const struct ccdigest_info* di = ccsha256_di();
    ccdigest_di_decl(di, hashTemp); // defines hashTemp array in stack
    ccdigest_init(di, hashTemp);

    // put boot time into hash
    const uint64_t* bootTime = ((uint64_t*)_COMM_PAGE_BOOTTIME_USEC);
    ccdigest_update(di, hashTemp, sizeof(uint64_t), bootTime);

    // put inode of executable into hash
    ccdigest_update(di, hashTemp, sizeof(statbuf.st_ino), &statbuf.st_ino);

    // put mod-time of executable into hash
    ccdigest_update(di, hashTemp, sizeof(statbuf.st_mtime), &statbuf.st_mtime);

    // complete hash computation and append as hex string
    uint8_t hashBits[32];
    ccdigest_final(di, hashTemp, hashBits);
    char* s = hashString;
    for (size_t i=0; i < sizeof(hashBits); ++i)
        putHexByte(hashBits[i], s);
    *s = '\0';
#endif
    return true;
}

bool LaunchClosure::buildClosureCachePath(const char* mainExecutablePath, char closurePath[], const char* tempDir,
                                          bool makeDirsIfMissing)
{
    // <rdar://problem/47688842> dyld3 should only save closures to disk for containerized apps
    if ( strstr(tempDir, "/Containers/Data/") == nullptr )
        return false;

    strlcpy(closurePath, tempDir, PATH_MAX);
    strlcat(closurePath, "/com.apple.dyld/", PATH_MAX);
    
    // make sure dyld sub-dir exists
    if ( makeDirsIfMissing ) {
        struct stat statbuf;
        if ( ::stat(closurePath, &statbuf) != 0 ) {
            if ( ::mkdir(closurePath, S_IRWXU) != 0 )
                return false;
        }
    }
    
    const char* leafName = strrchr(mainExecutablePath, '/');
    if ( leafName == nullptr )
        leafName = mainExecutablePath;
    else
        ++leafName;
    strlcat(closurePath, leafName, PATH_MAX);
    strlcat(closurePath, "-", PATH_MAX);

    if ( !hashBootAndFileInfo(mainExecutablePath, &closurePath[strlen(closurePath)]) )
        return false;

    strlcat(closurePath, ".closure", PATH_MAX);
    return true;
}

////////////////////////////  ObjCStringTable ////////////////////////////////////////
    
uint32_t ObjCStringTable::hash(const char *key, size_t keylen) const
{
    uint64_t val = objc_opt::lookup8((uint8_t*)key, keylen, salt);
    uint32_t index = (uint32_t)((shift == 64) ? 0 : (val>>shift)) ^ scramble[tab[val&mask]];
    return index;
}

const char* ObjCStringTable::getString(const char* selName, const Array<uintptr_t>& baseAddresses) const {
    StringTarget target = getPotentialTarget(selName);
    if (target == sentinelTarget)
        return nullptr;

    dyld3::closure::Image::ObjCImageOffset imageAndOffset;
    imageAndOffset.raw = target;

    uintptr_t sectionBaseAddress = baseAddresses[imageAndOffset.imageIndex];

    const char* value = (const char*)(sectionBaseAddress + imageAndOffset.imageOffset);
    if (!strcmp(selName, value))
        return value;
    return nullptr;
}

////////////////////////////  ObjCSelectorOpt ////////////////////////////////////////
bool ObjCSelectorOpt::getStringLocation(uint32_t index, const Array<closure::Image::ObjCSelectorImage>& selImages,
                                        ImageNum& imageNum, uint64_t& vmOffset) const {
    if ( index >= capacity )
        return false;

    StringTarget target = targets()[index];
    if ( target == indexNotFound )
        return false;

    dyld3::closure::Image::ObjCImageOffset imageAndOffset;
    imageAndOffset.raw = target;

    imageNum = selImages[imageAndOffset.imageIndex].imageNum;
    vmOffset = selImages[imageAndOffset.imageIndex].offset + imageAndOffset.imageOffset;
    return true;;
}

void ObjCSelectorOpt::forEachString(const Array<Image::ObjCSelectorImage>& selectorImages,
                                    void (^callback)(uint64_t selVMOffset, ImageNum imageNum)) const {
    dyld3::Array<StringTarget> stringTargets = targets();
    for (unsigned i = 0; i != capacity; ++i) {
        dyld3::closure::Image::ObjCImageOffset imageAndOffset;
        imageAndOffset.raw = stringTargets[i];

        if (imageAndOffset.raw == sentinelTarget)
            continue;

        callback(selectorImages[imageAndOffset.imageIndex].offset + imageAndOffset.imageOffset,
                 selectorImages[imageAndOffset.imageIndex].imageNum);
    }
}

////////////////////////////  ObjCClassOpt ////////////////////////////////////////

void ObjCClassOpt::forEachClass(const char* className, const Array<std::pair<uintptr_t, uintptr_t>>& nameAndDataBaseAddresses,
                                void (^callback)(void* classPtr, bool isLoaded, bool* stop)) const {
    uint32_t index = getIndex(className);
    if ( index == closure::ObjCStringTable::indexNotFound )
        return;

    StringTarget target = targets()[index];
    if ( target == sentinelTarget )
        return;

    // We have a potential target.  First check if the name is an exact match given the hash matched
    closure::Image::ObjCImageOffset classNameImageAndOffset;
    classNameImageAndOffset.raw = target;

    uintptr_t nameBaseAddress = 0;
    uintptr_t dataBaseAddress = 0;
    std::tie(nameBaseAddress, dataBaseAddress) = nameAndDataBaseAddresses[classNameImageAndOffset.imageIndex];

    const char* value = (const char*)(nameBaseAddress + classNameImageAndOffset.imageOffset);
    if ( strcmp(className, value) != 0 )
        return;

    // The name matched so now call the handler on all the classes for this name
    Array<closure::ObjCClassOpt::ClassTarget> classOffsetsArray = classOffsets();
    Array<closure::ObjCClassOpt::ClassTarget> duplicatesArray = duplicateOffsets(duplicateCount());

    const closure::ObjCClassOpt::ClassTarget& classOffset = classOffsetsArray[index];
    if (classOffset.classData.isDuplicate == 0) {
        // This class has a single implementation
        void* classImpl = (void*)(dataBaseAddress + classOffset.classData.imageOffset);
        bool stop = false;
        callback(classImpl, true, &stop);
    } else {
        // This class has mulitple implementations
        uint32_t duplicateCount = classOffset.duplicateData.count;
        uint32_t duplicateStartIndex = classOffset.duplicateData.index;
        for (uint32_t dupeIndex = 0; dupeIndex != duplicateCount; ++dupeIndex) {
            closure::ObjCClassOpt::ClassTarget& duplicateClass = duplicatesArray[duplicateStartIndex + dupeIndex];

            std::tie(nameBaseAddress, dataBaseAddress) = nameAndDataBaseAddresses[duplicateClass.classData.imageIndex];
            void* classImpl = (void*)(dataBaseAddress + duplicateClass.classData.imageOffset);
            bool            stop                = false;
            callback(classImpl, true, &stop);
            if (stop)
                break;
        }
    }
}

void ObjCClassOpt::forEachClass(const Array<Image::ObjCClassImage>& classImages,
                                void (^nameCallback)(uint64_t classNameVMOffset, ImageNum imageNum),
                                void (^implCallback)(uint64_t classVMOffset, ImageNum imageNum)) const {

    dyld3::Array<StringTarget> stringTargets = targets();
    dyld3::Array<ObjCClassOpt::ClassTarget> classOffsetsArray = classOffsets();
    dyld3::Array<ObjCClassOpt::ClassTarget> duplicatesArray = duplicateOffsets(duplicateCount());
    for (unsigned i = 0; i != capacity; ++i) {
        dyld3::closure::Image::ObjCImageOffset classNameImageAndOffset;
        classNameImageAndOffset.raw = stringTargets[i];

        if (classNameImageAndOffset.raw == sentinelTarget)
            continue;

        nameCallback(classImages[classNameImageAndOffset.imageIndex].offsetOfClassNames + classNameImageAndOffset.imageOffset,
                     classImages[classNameImageAndOffset.imageIndex].imageNum);

        // Walk each class for this key
        const ObjCClassOpt::ClassTarget& classOffset = classOffsetsArray[i];
        if (classOffset.classData.isDuplicate == 0) {
            // This class has a single implementation
            implCallback(classImages[classOffset.classData.imageIndex].offsetOfClasses + classOffset.classData.imageOffset,
                         classImages[classOffset.classData.imageIndex].imageNum);
        } else {
            // This class has mulitple implementations
            uint32_t duplicateCount = classOffset.duplicateData.count;
            uint32_t duplicateStartIndex = classOffset.duplicateData.index;
            for (uint32_t dupeIndex = 0; dupeIndex != duplicateCount; ++dupeIndex) {
                ObjCClassOpt::ClassTarget& duplicateClass = duplicatesArray[duplicateStartIndex + dupeIndex];
                implCallback(classImages[duplicateClass.classData.imageIndex].offsetOfClasses + duplicateClass.classData.imageOffset,
                             classImages[duplicateClass.classData.imageIndex].imageNum);
            }
        }
    }
}

////////////////////////////  ObjCClassDuplicatesOpt ////////////////////////////////////////

bool ObjCClassDuplicatesOpt::getClassLocation(const char* className, const objc_opt::objc_opt_t* objCOpt, void*& classImpl) const {
    uint32_t potentialTarget = getPotentialTarget(className);
    if (potentialTarget == sentinelTarget)
        return false;

    objc_opt::objc_clsopt_t* clsOpt = objCOpt->clsopt();

    Image::ObjCDuplicateClass duplicateClass;
    duplicateClass.raw = potentialTarget;

    const char* sharedCacheClassName = clsOpt->getClassNameForIndex(duplicateClass.sharedCacheClassOptIndex);
    if (strcmp(className, sharedCacheClassName) != 0)
        return false;

    classImpl = clsOpt->getClassForIndex(duplicateClass.sharedCacheClassOptIndex, duplicateClass.sharedCacheClassDuplicateIndex);
    return true;
}

void ObjCClassDuplicatesOpt::forEachClass(void (^callback)(Image::ObjCDuplicateClass duplicateClass)) const {
    dyld3::Array<StringTarget> stringTargets = targets();
    for (unsigned i = 0; i != capacity; ++i) {
        StringTarget target = stringTargets[i];
        if ( target == sentinelTarget )
            continue;
        Image::ObjCDuplicateClass duplicateClass;
        duplicateClass.raw = (uint32_t)target;
        callback(duplicateClass);
    }
}


} // namespace closure
} // namespace dyld3




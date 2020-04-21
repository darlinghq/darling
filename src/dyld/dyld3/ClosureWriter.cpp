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
#include <mach/vm_page_size.h>

#include "ClosureWriter.h"
#include "MachOFile.h"


namespace dyld3 {
namespace closure {


////////////////////////////  ContainerTypedBytesWriter ////////////////////////////////////////


void ContainerTypedBytesWriter::setContainerType(TypedBytes::Type containerType)
{
    assert(_vmAllocationStart == 0);
    _vmAllocationSize = 1024 * 1024;
    vm_address_t allocationAddr;
    ::vm_allocate(mach_task_self(), &allocationAddr, _vmAllocationSize, VM_FLAGS_ANYWHERE);
    assert(allocationAddr != 0);
    _vmAllocationStart = (void*)allocationAddr;
    _containerTypedBytes =  (TypedBytes*)_vmAllocationStart;
    _containerTypedBytes->type = containerType;
    _containerTypedBytes->payloadLength = 0;
    _end = (uint8_t*)_vmAllocationStart + sizeof(TypedBytes);
}

void* ContainerTypedBytesWriter::append(TypedBytes::Type t, const void* payload, uint32_t payloadSize)
{
    assert((payloadSize & 0x3) == 0);
    if ( (uint8_t*)_end + payloadSize >= (uint8_t*)_vmAllocationStart + _vmAllocationSize ) {
        // if current buffer too small, grow it
        size_t growth = _vmAllocationSize;
        if ( growth < payloadSize )
            growth = _vmAllocationSize*((payloadSize/_vmAllocationSize)+1);
        vm_address_t newAllocationAddr;
        size_t newAllocationSize = _vmAllocationSize+growth;
        ::vm_allocate(mach_task_self(), &newAllocationAddr, newAllocationSize, VM_FLAGS_ANYWHERE);
	    assert(newAllocationAddr != 0);
        size_t currentInUse = (char*)_end - (char*)_vmAllocationStart;
        memcpy((void*)newAllocationAddr, _vmAllocationStart, currentInUse);
        ::vm_deallocate(mach_task_self(), (vm_address_t)_vmAllocationStart, _vmAllocationSize);
        _end                 = (void*)(newAllocationAddr + currentInUse);
        _vmAllocationStart   = (void*)newAllocationAddr;
        _containerTypedBytes = (TypedBytes*)_vmAllocationStart;
        _vmAllocationSize    = newAllocationSize;
    }
    assert( (uint8_t*)_end + payloadSize < (uint8_t*)_vmAllocationStart + _vmAllocationSize);
    TypedBytes* tb = (TypedBytes*)_end;
    tb->type   = t;
    tb->payloadLength = payloadSize;
    if ( payload != nullptr )
        ::memcpy(tb->payload(), payload, payloadSize);
    _end = (uint8_t*)_end + sizeof(TypedBytes) + payloadSize;
    assert((_containerTypedBytes->payloadLength + sizeof(TypedBytes) + payloadSize) < (16 * 1024 * 1024));
    _containerTypedBytes->payloadLength += sizeof(TypedBytes) + payloadSize;
    return tb->payload();
}

const void* ContainerTypedBytesWriter::finalizeContainer()
{
    // trim vm allocation down to just what is needed
    uintptr_t bufferStart = (uintptr_t)_vmAllocationStart;
    uintptr_t used = round_page((uintptr_t)_end - bufferStart);
    if ( used < _vmAllocationSize ) {
        uintptr_t deallocStart = bufferStart + used;
        ::vm_deallocate(mach_task_self(), deallocStart, _vmAllocationSize - used);
        _end = nullptr;
        _vmAllocationSize = used;
    }
    // mark vm region read-only
    ::vm_protect(mach_task_self(), bufferStart, used, false, VM_PROT_READ);
    return (void*)_vmAllocationStart;
}

const void* ContainerTypedBytesWriter::currentTypedBytes()
{
    return (void*)_vmAllocationStart;
}

void ContainerTypedBytesWriter::deallocate()
{
    ::vm_deallocate(mach_task_self(), (long)_vmAllocationStart, _vmAllocationSize);
}

////////////////////////////  ImageWriter ////////////////////////////////////////


const Image* ImageWriter::finalize()
{
    return (Image*)finalizeContainer();
}

const Image* ImageWriter::currentImage()
{
    return (Image*)currentTypedBytes();
}

void ImageWriter::addPath(const char* path)
{
    uint32_t roundedPathLen = ((uint32_t)strlen(path) + 1 + 3) & (-4);
    Image::PathAndHash* ph = (Image::PathAndHash*)append(TypedBytes::Type::pathWithHash, nullptr, sizeof(Image::PathAndHash)+roundedPathLen);
    ph->hash = Image::hashFunction(path);
    strcpy(ph->path, path);
}

Image::Flags& ImageWriter::getFlags()
{
    if ( _flagsOffset == -1 ) {
        setContainerType(TypedBytes::Type::image);
        Image::Flags flags;
        ::bzero(&flags, sizeof(flags));
        uint8_t* p = (uint8_t*)append(TypedBytes::Type::imageFlags, &flags, sizeof(flags));
        _flagsOffset = (int)(p - (uint8_t*)currentTypedBytes());
    }
    return *((Image::Flags*)((uint8_t*)currentTypedBytes() + _flagsOffset));
}

void ImageWriter::setImageNum(ImageNum num)
{
   getFlags().imageNum = num;
}

void ImageWriter::setHasObjC(bool value)
{
    getFlags().hasObjC = value;
}

void ImageWriter::setIs64(bool value)
{
    getFlags().is64 = value;
}

void ImageWriter::setHasPlusLoads(bool value)
{
    getFlags().mayHavePlusLoads = value;
}

void ImageWriter::setIsBundle(bool value)
{
    getFlags().isBundle = value;
}

void ImageWriter::setIsDylib(bool value)
{
    getFlags().isDylib = value;
}

void ImageWriter::setIsExecutable(bool value)
{
    getFlags().isExecutable = value;
}

void ImageWriter::setHasWeakDefs(bool value)
{
    getFlags().hasWeakDefs = value;
}

void ImageWriter::setUses16KPages(bool value)
{
    getFlags().has16KBpages = value;
}

void ImageWriter::setOverridableDylib(bool value)
{
    getFlags().overridableDylib = value;
}

void ImageWriter::setInvalid()
{
    getFlags().isInvalid = true;
}

void ImageWriter::setInDyldCache(bool value)
{
    getFlags().inDyldCache = value;
}

void ImageWriter::setHasPrecomputedObjC(bool value)
{
    getFlags().hasPrecomputedObjC = value;
}

void ImageWriter::setNeverUnload(bool value)
{
    getFlags().neverUnload = value;
}

void ImageWriter::setUUID(const uuid_t uuid)
{
    append(TypedBytes::Type::uuid, uuid, sizeof(uuid_t));
}

void ImageWriter::addCDHash(const uint8_t cdHash[20])
{
    append(TypedBytes::Type::cdHash, cdHash, 20);
}

void ImageWriter::setDependents(const Array<Image::LinkedImage>& deps)
{
    append(TypedBytes::Type::dependents, deps.begin(), (uint32_t)deps.count()*sizeof(Image::LinkedImage));
}

void ImageWriter::setDofOffsets(const Array<uint32_t>& dofSectionOffsets)
{
    append(TypedBytes::Type::dofOffsets, &dofSectionOffsets[0], (uint32_t)dofSectionOffsets.count()*sizeof(uint32_t));
}

void ImageWriter::setInitOffsets(const uint32_t initOffsets[], uint32_t count)
{
    append(TypedBytes::Type::initOffsets, initOffsets, count*sizeof(uint32_t));
}

void ImageWriter::setTermOffsets(const uint32_t termOffsets[], uint32_t count)
{
    getFlags().hasTerminators = true;
    append(TypedBytes::Type::termOffsets, termOffsets, count*sizeof(uint32_t));
}

void ImageWriter::setInitSectRange(uint32_t sectionOffset, uint32_t sectionSize)
{
    Image::InitializerSectionRange range = { sectionOffset, sectionSize };
    append(TypedBytes::Type::initsSection, &range, sizeof(Image::InitializerSectionRange));
}

void ImageWriter::setDiskSegments(const Image::DiskSegment segs[], uint32_t count)
{
    append(TypedBytes::Type::diskSegment, segs, count*sizeof(Image::DiskSegment));
    for (uint32_t i=0; i < count; ++i) {
        if ( segs[i].permissions == Image::DiskSegment::kReadOnlyDataPermissions )
            getFlags().hasReadOnlyData = true;
    }
}

void ImageWriter::setCachedSegments(const Image::DyldCacheSegment segs[], uint32_t count)
{
    append(TypedBytes::Type::cacheSegment, segs, count*sizeof(Image::DyldCacheSegment));
}

void ImageWriter::setCodeSignatureLocation(uint32_t fileOffset, uint32_t size)
{
    Image::CodeSignatureLocation loc;
    loc.fileOffset = fileOffset;
    loc.fileSize   = size;
    append(TypedBytes::Type::codeSignLoc, &loc, sizeof(loc));
}

void ImageWriter::setFairPlayEncryptionRange(uint32_t fileOffset, uint32_t size)
{
    Image::FairPlayRange loc;
    loc.rangeStart = fileOffset;
    loc.rangeLength = size;
    append(TypedBytes::Type::fairPlayLoc, &loc, sizeof(loc));
}

void ImageWriter::setMappingInfo(uint64_t sliceOffset, uint64_t vmSize)
{
    const uint32_t pageSize = getFlags().has16KBpages ? 0x4000 : 0x1000;
    Image::MappingInfo info;
    info.sliceOffsetIn4K = (uint32_t)(sliceOffset / 0x1000);
    info.totalVmPages    = (uint32_t)(vmSize / pageSize);
    append(TypedBytes::Type::mappingInfo, &info, sizeof(info));
}

void ImageWriter::setFileInfo(uint64_t inode, uint64_t mTime)
{
    Image::FileInfo info = { inode, mTime };
    append(TypedBytes::Type::fileInodeAndTime, &info, sizeof(info));
}

void ImageWriter::setRebaseInfo(const Array<Image::RebasePattern>& fixups)
{
    append(TypedBytes::Type::rebaseFixups, fixups.begin(), (uint32_t)fixups.count()*sizeof(Image::RebasePattern));
}

void ImageWriter::setTextRebaseInfo(const Array<Image::TextFixupPattern>& fixups)
{
    append(TypedBytes::Type::textFixups, fixups.begin(), (uint32_t)fixups.count()*sizeof(Image::TextFixupPattern));
}

void ImageWriter::setBindInfo(const Array<Image::BindPattern>& fixups)
{
    append(TypedBytes::Type::bindFixups, fixups.begin(), (uint32_t)fixups.count()*sizeof(Image::BindPattern));
}

void ImageWriter::setChainedFixups(uint64_t runtimeStartsStructOffset, const Array<Image::ResolvedSymbolTarget>& targets)
{
    getFlags().hasChainedFixups = true;
    append(TypedBytes::Type::chainedStartsOffset, &runtimeStartsStructOffset, sizeof(uint64_t));
    append(TypedBytes::Type::chainedFixupsTargets, targets.begin(), (uint32_t)targets.count()*sizeof(Image::ResolvedSymbolTarget));
}

void ImageWriter::setObjCFixupInfo(const Image::ResolvedSymbolTarget& objcProtocolClassTarget,
                                   uint64_t objcImageInfoVMOffset,
                                   const Array<Image::ProtocolISAFixup>& protocolISAFixups,
                                   const Array<Image::SelectorReferenceFixup>& selRefFixups,
                                   const Array<Image::ClassStableSwiftFixup>& classStableSwiftFixups,
                                   const Array<Image::MethodListFixup>& methodListFixups)
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

    uint64_t headerSize = sizeof(Image::ResolvedSymbolTarget) + sizeof(uint64_t) + (sizeof(uint32_t) * 4);
    uint64_t protocolsSize = (sizeof(Image::ProtocolISAFixup) * protocolISAFixups.count());
    uint64_t selRefsSize = (sizeof(Image::SelectorReferenceFixup) * selRefFixups.count());
    uint64_t stableSwiftSize = (sizeof(Image::ClassStableSwiftFixup) * classStableSwiftFixups.count());
    uint64_t methodListSize = (sizeof(Image::MethodListFixup) * methodListFixups.count());

    uint64_t totalSize = headerSize + protocolsSize + selRefsSize + stableSwiftSize + methodListSize;
    assert( (totalSize & 3) == 0);
    uint8_t* buffer = (uint8_t*)append(TypedBytes::Type::objcFixups, nullptr, (uint32_t)totalSize);

    // Set the statically sized data
    uint32_t protocolFixupCount = (uint32_t)protocolISAFixups.count();
    uint32_t selRefFixupCount = (uint32_t)selRefFixups.count();
    memcpy(buffer, &objcProtocolClassTarget, sizeof(Image::ResolvedSymbolTarget));
    buffer += sizeof(Image::ResolvedSymbolTarget);
    memcpy(buffer, &objcImageInfoVMOffset, sizeof(uint64_t));
    buffer += sizeof(uint64_t);
    memcpy(buffer, &protocolFixupCount, sizeof(uint32_t));
    buffer += sizeof(uint32_t);
    memcpy(buffer, &selRefFixupCount, sizeof(uint32_t));
    buffer += sizeof(uint32_t);

    // Set the protocol fixups
    if ( protocolFixupCount != 0 ) {
        memcpy(buffer, protocolISAFixups.begin(), (size_t)protocolsSize);
        buffer += protocolsSize;
    }

    // Set the selector reference fixups
    if ( selRefFixupCount != 0 ) {
        memcpy(buffer, selRefFixups.begin(), (size_t)selRefsSize);
        buffer += selRefsSize;
    }

    // New closures get additional fixups.  These are ignored by old dyld's
    uint32_t stableSwiftFixupCount = (uint32_t)classStableSwiftFixups.count();
    uint32_t methodListFixupCount = (uint32_t)methodListFixups.count();
    memcpy(buffer, &stableSwiftFixupCount, sizeof(uint32_t));
    buffer += sizeof(uint32_t);
    memcpy(buffer, &methodListFixupCount, sizeof(uint32_t));
    buffer += sizeof(uint32_t);

    // Set the stable swift fixups
    if ( stableSwiftFixupCount != 0 ) {
        memcpy(buffer, classStableSwiftFixups.begin(), (size_t)stableSwiftSize);
        buffer += stableSwiftSize;
    }

    // Set the method list fixups
    if ( methodListFixupCount != 0 ) {
        memcpy(buffer, methodListFixups.begin(), (size_t)methodListSize);
        buffer += methodListSize;
    }
}

void ImageWriter::setAsOverrideOf(ImageNum imageNum)
{
    uint32_t temp = imageNum;
    append(TypedBytes::Type::imageOverride, &temp, sizeof(temp));
}

void ImageWriter::setInitsOrder(const ImageNum images[], uint32_t count)
{
    append(TypedBytes::Type::initBefores, images, count*sizeof(ImageNum));
}


////////////////////////////  ImageArrayWriter ////////////////////////////////////////


ImageArrayWriter::ImageArrayWriter(ImageNum startImageNum, unsigned count, bool hasRoots) : _index(0)
{
    setContainerType(TypedBytes::Type::imageArray);
    _end = (void*)((uint8_t*)_end + sizeof(ImageArray) - sizeof(TypedBytes) + sizeof(uint32_t)*count);
    _containerTypedBytes->payloadLength = sizeof(ImageArray) - sizeof(TypedBytes) + sizeof(uint32_t)*count;
    ImageArray* ia = (ImageArray*)_containerTypedBytes;
    ia->firstImageNum   = startImageNum;
    ia->count           = count;
    ia->hasRoots        = hasRoots;
}

void ImageArrayWriter::appendImage(const Image* image)
{
    ImageArray* ia = (ImageArray*)_containerTypedBytes;
    ia->offsets[_index++] = _containerTypedBytes->payloadLength;
    append(TypedBytes::Type::image, image->payload(), image->payloadLength);
}

const ImageArray* ImageArrayWriter::finalize()
{
    return (ImageArray*)finalizeContainer();
}


////////////////////////////  ClosureWriter ////////////////////////////////////////

void ClosureWriter::setTopImageNum(ImageNum imageNum)
{
    append(TypedBytes::Type::topImage, &imageNum, sizeof(ImageNum));
}

void ClosureWriter::addCachePatches(const Array<Closure::PatchEntry>& patches)
{
    append(TypedBytes::Type::cacheOverrides, patches.begin(), (uint32_t)patches.count()*sizeof(Closure::PatchEntry));
}

void ClosureWriter::applyInterposing(const LaunchClosure* launchClosure)
{
    const Closure*       currentClosure = (Closure*)currentTypedBytes();
	const ImageArray*    images         = currentClosure->images();
	launchClosure->forEachInterposingTuple(^(const InterposingTuple& tuple, bool&) {
        images->forEachImage(^(const dyld3::closure::Image* image, bool&) {
            for (const Image::BindPattern& bindPat : image->bindFixups()) {
                if ( (bindPat.target == tuple.stockImplementation) && (tuple.newImplementation.image.imageNum != image->imageNum()) ) {
                    Image::BindPattern* writePat = const_cast<Image::BindPattern*>(&bindPat);
                    writePat->target = tuple.newImplementation;
                }
            }

            // Chained fixups may also be interposed.  We can't change elements in the chain, but we can change
            // the target list.
            for (const Image::ResolvedSymbolTarget& symbolTarget : image->chainedTargets()) {
                if ( (symbolTarget == tuple.stockImplementation) && (tuple.newImplementation.image.imageNum != image->imageNum()) ) {
                    Image::ResolvedSymbolTarget* writeTarget = const_cast<Image::ResolvedSymbolTarget*>(&symbolTarget);
                    *writeTarget = tuple.newImplementation;
                }
            }
        });
    });
}

void ClosureWriter::addWarning(Closure::Warning::Type warningType, const char* warning)
{
    uint32_t roundedMessageLen = ((uint32_t)strlen(warning) + 1 + 3) & (-4);
    Closure::Warning* ph = (Closure::Warning*)append(TypedBytes::Type::warning, nullptr, sizeof(Closure::Warning)+roundedMessageLen);
    ph->type = warningType;
    strcpy(ph->message, warning);
}


////////////////////////////  LaunchClosureWriter ////////////////////////////////////////

LaunchClosureWriter::LaunchClosureWriter(const ImageArray* images)
{
    setContainerType(TypedBytes::Type::launchClosure);
    append(TypedBytes::Type::imageArray, images->payload(), images->payloadLength);
}

const LaunchClosure* LaunchClosureWriter::finalize()
{
    return (LaunchClosure*)finalizeContainer();
}

void LaunchClosureWriter::setLibSystemImageNum(ImageNum imageNum)
{
    append(TypedBytes::Type::libSystemNum, &imageNum, sizeof(ImageNum));
}

void LaunchClosureWriter::setLibDyldEntry(Image::ResolvedSymbolTarget entry)
{
    append(TypedBytes::Type::libDyldEntry, &entry, sizeof(entry));
}

void LaunchClosureWriter::setMainEntry(Image::ResolvedSymbolTarget main)
{
    append(TypedBytes::Type::mainEntry, &main, sizeof(main));
}

void LaunchClosureWriter::setStartEntry(Image::ResolvedSymbolTarget start)
{
    append(TypedBytes::Type::startEntry, &start, sizeof(start));
}

void LaunchClosureWriter::setUsedFallbackPaths(bool value)
{
    getFlags().usedFallbackPaths = value;
}

void LaunchClosureWriter::setUsedAtPaths(bool value)
{
    getFlags().usedAtPaths = value;
}

void LaunchClosureWriter::setHasInsertedLibraries(bool value)
{
    getFlags().hasInsertedLibraries = value;
}

void LaunchClosureWriter::setInitImageCount(uint32_t count)
{
    getFlags().initImageCount = count;
}

LaunchClosure::Flags& LaunchClosureWriter::getFlags()
{
    if ( _flagsOffset == -1 ) {
        LaunchClosure::Flags flags;
        ::bzero(&flags, sizeof(flags));
        uint8_t* p = (uint8_t*)append(TypedBytes::Type::closureFlags, &flags, sizeof(flags));
        _flagsOffset = (int)(p - (uint8_t*)currentTypedBytes());
    }
    return *((LaunchClosure::Flags*)((uint8_t*)currentTypedBytes() + _flagsOffset));
}

void LaunchClosureWriter::setMustBeMissingFiles(const Array<const char*>& paths)
{
    uint32_t totalSize = 0;
    for (const char* s : paths)
        totalSize += (strlen(s) +1);
    totalSize = (totalSize + 3) & (-4); // align

    char* buffer = (char*)append(TypedBytes::Type::missingFiles, nullptr, totalSize);
    char* t = buffer;
    for (const char* path : paths) {
        for (const char* s=path; *s != '\0'; ++s)
            *t++ = *s;
        *t++ = '\0';
    }
    while (t < &buffer[totalSize])
        *t++ = '\0';
}

void LaunchClosureWriter::setMustExistFiles(const Array<LaunchClosure::SkippedFile>& files)
{
    // Start the structure with a count
    uint32_t totalSize = sizeof(uint64_t);

    // Then make space for the array of mod times and inode numbers
    totalSize += files.count() * sizeof(uint64_t) * 2;

    // Then the array of paths on the end
    for (const LaunchClosure::SkippedFile& file : files)
        totalSize += (strlen(file.path) + 1);
    totalSize = (totalSize + 3) & (-4); // align

    char* buffer = (char*)append(TypedBytes::Type::existingFiles, nullptr, totalSize);

    // Set the size
    uint64_t* bufferPtr = (uint64_t*)buffer;
    *bufferPtr++ = (uint64_t)files.count();

    // And the array of mod times and inode numbers
    for (const LaunchClosure::SkippedFile& file : files) {
        *bufferPtr++ = file.inode;
        *bufferPtr++ = file.mtime;
    }

    char* t = (char*)bufferPtr;
    for (const LaunchClosure::SkippedFile& file : files) {
        for (const char* s=file.path; *s != '\0'; ++s)
            *t++ = *s;
        *t++ = '\0';
    }
    while (t < &buffer[totalSize])
        *t++ = '\0';
}

void LaunchClosureWriter::addEnvVar(const char* envVar)
{
    unsigned len = (unsigned)strlen(envVar);
    char temp[len+8];
    strcpy(temp, envVar);
    unsigned paddedSize = len+1;
    while ( (paddedSize % 4) != 0 )
        temp[paddedSize++] = '\0';
    append(TypedBytes::Type::envVar, temp, paddedSize);
}

void LaunchClosureWriter::addInterposingTuples(const Array<InterposingTuple>& tuples)
{
    append(TypedBytes::Type::interposeTuples, tuples.begin(), (uint32_t)tuples.count()*sizeof(InterposingTuple));
}

void LaunchClosureWriter::setDyldCacheUUID(const uuid_t uuid)
{
    append(TypedBytes::Type::dyldCacheUUID, uuid, sizeof(uuid_t));
}

void LaunchClosureWriter::setBootUUID(const char* uuid)
{
    unsigned len = (unsigned)strlen(uuid);
    char temp[len+8];
    strcpy(temp, uuid);
    unsigned paddedSize = len+1;
    while ( (paddedSize % 4) != 0 )
        temp[paddedSize++] = '\0';
    append(TypedBytes::Type::bootUUID, temp, paddedSize);
}

void LaunchClosureWriter::setObjCSelectorInfo(const Array<uint8_t>& hashTable, const Array<Image::ObjCSelectorImage>& hashTableImages) {
    uint32_t count = (uint32_t)hashTableImages.count();
    uint32_t totalSize = (uint32_t)(sizeof(count) + (sizeof(Image::ObjCSelectorImage) * count) + hashTable.count());
    totalSize = (totalSize + 3) & (-4); // align
    uint8_t* buffer = (uint8_t*)append(TypedBytes::Type::selectorTable, nullptr, totalSize);

    // Write out out the image count
    memcpy(buffer, &count, sizeof(count));
    buffer += sizeof(count);

    // Write out out the image nums
    memcpy(buffer, hashTableImages.begin(), sizeof(Image::ObjCSelectorImage) * count);
    buffer += sizeof(Image::ObjCSelectorImage) * count;

    // Write out out the image count
    memcpy(buffer, hashTable.begin(), hashTable.count());
}

void LaunchClosureWriter::setObjCClassAndProtocolInfo(const Array<uint8_t>& classHashTable, const Array<uint8_t>& protocolHashTable,
                                                      const Array<Image::ObjCClassImage>& hashTableImages) {
    // The layout here is:
    //   uint32_t offset to class table (note this is 0 if there are no classes)
    //   uint32_t offset to protocol table (note this is 0 if there are no protocols)
    //   uint32_t num images
    //   ObjCClassImage[num images]
    //   class hash table
    //   [ padding to 4-byte alignment if needed
    //   protocol hash table
    //   [ padding to 4-byte alignment if needed

    uint32_t numImages = (uint32_t)hashTableImages.count();

    uint32_t headerSize = sizeof(uint32_t) * 3;
    uint32_t imagesSize = (sizeof(Image::ObjCClassImage) * numImages);
    uint32_t classTableSize = ((uint32_t)classHashTable.count() + 3) & (-4); // pad to 4-byte multiple
    uint32_t protocolTableSize = ((uint32_t)protocolHashTable.count() + 3) & (-4); // pad to 4-byte multiple
    uint32_t offsetToClassTable = (classTableSize == 0) ? 0 : (headerSize + imagesSize);
    uint32_t offsetToProtocolTable = (protocolTableSize == 0) ? 0 : (headerSize + imagesSize + classTableSize);

    uint32_t totalSize = headerSize + imagesSize + classTableSize + protocolTableSize;
    assert( (totalSize & 3) == 0);
    uint8_t* buffer = (uint8_t*)append(TypedBytes::Type::classTable, nullptr, totalSize);

    // Write out out the header
    memcpy(buffer + 0, &offsetToClassTable, sizeof(uint32_t));
    memcpy(buffer + 4, &offsetToProtocolTable, sizeof(uint32_t));
    memcpy(buffer + 8, &numImages, sizeof(uint32_t));

    // Write out out the image nums
    memcpy(buffer + headerSize, hashTableImages.begin(), imagesSize);

    // Write out out the class hash table
    if ( offsetToClassTable != 0 )
        memcpy(buffer + offsetToClassTable, classHashTable.begin(), classHashTable.count());

    // Write out out the protocol hash table
    if ( offsetToProtocolTable != 0 )
        memcpy(buffer + offsetToProtocolTable, protocolHashTable.begin(), protocolHashTable.count());
}

void LaunchClosureWriter::setObjCDuplicateClassesInfo(const Array<uint8_t>& hashTable) {
    uint32_t totalSize = (uint32_t)hashTable.count();
    totalSize = (totalSize + 3) & (-4); // align
    uint8_t* buffer = (uint8_t*)append(TypedBytes::Type::duplicateClassesTable, nullptr, totalSize);

    // Write out out the hash table
    memcpy(buffer, hashTable.begin(), hashTable.count());
}

////////////////////////////  DlopenClosureWriter ////////////////////////////////////////

DlopenClosureWriter::DlopenClosureWriter(const ImageArray* images)
{
    setContainerType(TypedBytes::Type::dlopenClosure);
    append(TypedBytes::Type::imageArray, images->payload(), images->payloadLength);
}

const DlopenClosure* DlopenClosureWriter::finalize()
{
    return (DlopenClosure*)finalizeContainer();
}


} // namespace closure
} // namespace dyld3




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


#ifndef ClosureWriter_h
#define ClosureWriter_h


#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <uuid/uuid.h>
#include <mach/mach.h>
#include <mach-o/loader.h>
#include <uuid/uuid.h>

#include "Closure.h"



namespace dyld3 {

namespace closure {


class VIS_HIDDEN ContainerTypedBytesWriter
{
public:
    void        deallocate();

protected:
    void        setContainerType(TypedBytes::Type containerType);
    void*       append(TypedBytes::Type t, const void* payload, uint32_t payloadSize);

    const void* currentTypedBytes();
    const void* finalizeContainer();

    void*       _vmAllocationStart      = nullptr;
    size_t      _vmAllocationSize       = 0;
    TypedBytes* _containerTypedBytes    = nullptr;
    void*       _end                    = nullptr;
};


class VIS_HIDDEN ImageWriter : public ContainerTypedBytesWriter
{
public:

    void        setImageNum(ImageNum num);
    void        addPath(const char* path); // first is canonical, others are aliases
    void        setInvalid();
    void        setInDyldCache(bool);
    void        setHasPrecomputedObjC(bool);
    void        setIs64(bool);
    void        setHasObjC(bool);
    void        setHasPlusLoads(bool);
    void        setIsBundle(bool);
    void        setIsDylib(bool);
    void        setIsExecutable(bool);
    void        setIsRestricted(bool);
    void        setHasWeakDefs(bool);
    void        setUses16KPages(bool);
    void        setOverridableDylib(bool);
    void        setNeverUnload(bool);
    void        setHasTerminators(bool);
    void        setUUID(const uuid_t uuid);
    void        addCDHash(const uint8_t cdHash[20]);
    void        setDependents(const Array<Image::LinkedImage>& deps);
    void        setDofOffsets(const Array<uint32_t>& dofSectionOffsets);
    void        setInitOffsets(const uint32_t initOffsets[], uint32_t count);
    void        setInitSectRange(uint32_t sectionOffset, uint32_t sectionSize);
    void        setTermOffsets(const uint32_t termOffsets[], uint32_t count);
    void        setDiskSegments(const Image::DiskSegment segs[], uint32_t count);
    void        setCachedSegments(const Image::DyldCacheSegment segs[], uint32_t count);
    void        setCodeSignatureLocation(uint32_t fileOffset, uint32_t size);
    void        setFairPlayEncryptionRange(uint32_t fileOffset, uint32_t size);
    void        setMappingInfo(uint64_t sliceOffset, uint64_t vmSize);
    void        setFileInfo(uint64_t inode, uint64_t modTime);
    void        setRebaseInfo(const Array<Image::RebasePattern>&);
    void        setTextRebaseInfo(const Array<Image::TextFixupPattern>&);
    void        setBindInfo(const Array<Image::BindPattern>&);
    void        setObjCFixupInfo(const Image::ResolvedSymbolTarget& objcProtocolClassTarget,
                                 uint64_t objcImageInfoVMOffset,
                                 const Array<Image::ProtocolISAFixup>& protocolISAFixups,
                                 const Array<Image::SelectorReferenceFixup>& selRefFixups,
                                 const Array<Image::ClassStableSwiftFixup>& classStableSwiftFixups,
                                 const Array<Image::MethodListFixup>& methodListFixups);
    void        setAsOverrideOf(ImageNum);
    void        setInitsOrder(const ImageNum images[], uint32_t count);
    void        setChainedFixups(uint64_t runtimeStartsStructOffset, const Array<Image::ResolvedSymbolTarget>& targets);

    const Image* currentImage();

    const Image* finalize();

private:
    Image::Flags& getFlags();

    int   _flagsOffset = -1;
};


class VIS_HIDDEN ImageArrayWriter : public ContainerTypedBytesWriter
{
public:
                        ImageArrayWriter(ImageNum startImageNum, unsigned count, bool hasRoots);

    void                appendImage(const Image*);
    const ImageArray*   finalize();
private:
    unsigned            _index;
};

class VIS_HIDDEN ClosureWriter : public ContainerTypedBytesWriter
{
public:
    void                    setTopImageNum(ImageNum imageNum);
    void                    addCachePatches(const Array<Closure::PatchEntry>&);
    void                    applyInterposing(const LaunchClosure* launchClosure);
    void                    addWarning(Closure::Warning::Type type, const char* warning);
};

class VIS_HIDDEN LaunchClosureWriter : public ClosureWriter
{
    friend class            ClosureBuilder;
public:
                            LaunchClosureWriter(const ImageArray* images);

    const LaunchClosure*    finalize();
    void                    setLibSystemImageNum(ImageNum imageNum);
    void                    setInitImageCount(uint32_t count);
    void                    setLibDyldEntry(Image::ResolvedSymbolTarget dyldEntry);
    void                    setMainEntry(Image::ResolvedSymbolTarget main);
    void                    setStartEntry(Image::ResolvedSymbolTarget start);
    void                    setUsedFallbackPaths(bool);
    void                    setUsedAtPaths(bool);
    void                    setHasInsertedLibraries(bool);
    void                    setMustBeMissingFiles(const Array<const char*>& paths);
    void                    setMustExistFiles(const Array<LaunchClosure::SkippedFile>& files);
    void                    addInterposingTuples(const Array<InterposingTuple>& tuples);
    void                    setDyldCacheUUID(const uuid_t);
    void                    setBootUUID(const char* uuid);
    void                    addEnvVar(const char* envVar);
    void                    setObjCSelectorInfo(const Array<uint8_t>& hashTable, const Array<Image::ObjCSelectorImage>& hashTableImages);
    void                    setObjCClassAndProtocolInfo(const Array<uint8_t>& classHashTable, const Array<uint8_t>& protocolHashTable,
                                                        const Array<Image::ObjCClassImage>& hashTableImages);
    void                    setObjCDuplicateClassesInfo(const Array<uint8_t>& hashTable);

private:
    LaunchClosure::Flags&   getFlags();

    int                      _flagsOffset = -1;
};


class VIS_HIDDEN DlopenClosureWriter : public ClosureWriter
{
public:
                             DlopenClosureWriter(const ImageArray* images);

    const DlopenClosure*     finalize();

};


} //  namespace closure
} //  namespace dyld3


#endif // ClosureWriter_h


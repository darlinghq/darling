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


#ifndef Closures_h
#define Closures_h


#include <stdint.h>
#include <assert.h>
#include <uuid/uuid.h>
#include <mach/mach.h>
#include <mach-o/loader.h>

#include "Diagnostics.h"
#include "Array.h"
#include "MachOLoaded.h"
#include "SupportedArchs.h"

namespace objc_opt {
struct objc_opt_t;
}

namespace dyld3 {
namespace closure {



// bump this number each time binary format changes
enum  { kFormatVersion = 10 };


typedef uint32_t ImageNum;

const ImageNum kFirstDyldCacheImageNum      = 0x00000001;
const ImageNum kLastDyldCacheImageNum       = 0x00000FFF;
const ImageNum kFirstOtherOSImageNum        = 0x00001001;
const ImageNum kLastOtherOSImageNum         = 0x00001FFF;
const ImageNum kFirstLaunchClosureImageNum  = 0x00002000;
const ImageNum kMissingWeakLinkedImage      = 0x0FFFFFFF;

class ObjCSelectorOpt;
class ObjCClassOpt;
class ObjCClassDuplicatesOpt;

//
//  Generic typed range of bytes (similar to load commands)
//  Must be 4-byte aligned
//
struct VIS_HIDDEN TypedBytes
{
    enum class Type : uint32_t {
        // containers which have an overall length and TypedBytes inside their content
        launchClosure    =  1, // contains TypedBytes of closure attributes including imageArray
        imageArray       =  2, // sizeof(ImageArray) + sizeof(uint32_t)*count + size of all images
        image            =  3, // contains TypedBytes of image attributes
        dlopenClosure    =  4, // contains TypedBytes of closure attributes including imageArray

        // attributes for Images
        imageFlags       =  7, // sizeof(Image::Flags)
        pathWithHash     =  8, // len = uint32_t + length path + 1, use multiple entries for aliases
        fileInodeAndTime =  9, // sizeof(FileInfo)
        cdHash           = 10, // 20, use multiple entries on watchOS for all hashes
        uuid             = 11, // 16
        mappingInfo      = 12, // sizeof(MappingInfo)
        diskSegment      = 13, // sizeof(DiskSegment) * count
        cacheSegment     = 14, // sizeof(DyldCacheSegment) * count
        dependents       = 15, // sizeof(LinkedImage) * count
        initOffsets      = 16, // sizeof(uint32_t) * count
        dofOffsets       = 17, // sizeof(uint32_t) * count
        codeSignLoc      = 18, // sizeof(CodeSignatureLocation)
        fairPlayLoc      = 19, // sizeof(FairPlayRange)
        rebaseFixups     = 20, // sizeof(RebasePattern) * count
        bindFixups       = 21, // sizeof(BindPattern) * count
        cachePatchInfo   = 22, // deprecated
        textFixups       = 23, // sizeof(TextFixupPattern) * count
        imageOverride    = 24, // sizeof(ImageNum)
        initBefores      = 25, // sizeof(ImageNum) * count
        initsSection     = 26, // sizeof(InitializerSectionRange)
        chainedFixupsTargets = 27, // sizeof(ResolvedSymbolTarget) * count
        termOffsets      = 28, // sizeof(uint32_t) * count
        chainedStartsOffset = 29, // sizeof(uint64_t)
        objcFixups       = 30,   // sizeof(ResolvedSymbolTarget) + (sizeof(uint32_t) * 2) + (sizeof(ProtocolISAFixup) * count) + (sizeof(SelectorReferenceFixup) * count)

        // attributes for Closures (launch or dlopen)
        closureFlags            = 32,  // sizeof(Closure::Flags)
        dyldCacheUUID           = 33,  // 16
        missingFiles            = 34,
        envVar                  = 35,  // "DYLD_BLAH=stuff"
        topImage                = 36,  // sizeof(ImageNum)
        libDyldEntry            = 37,  // sizeof(ResolvedSymbolTarget)
        libSystemNum            = 38,  // sizeof(ImageNum)
        bootUUID                = 39,  // c-string 40
        mainEntry               = 40,  // sizeof(ResolvedSymbolTarget)
        startEntry              = 41,  // sizeof(ResolvedSymbolTarget)     // used by programs built with crt1.o
        cacheOverrides          = 42,  // sizeof(PatchEntry) * count       // used if process uses interposing or roots (cached dylib overrides)
        interposeTuples         = 43,  // sizeof(InterposingTuple) * count
        existingFiles           = 44,  // uint64_t + (SkippedFiles * count)
        selectorTable           = 45,  // uint32_t + (sizeof(ObjCSelectorImage) * count) + hashTable size
        classTable              = 46,  // (3 * uint32_t) + (sizeof(ObjCClassImage) * count) + classHashTable size + protocolHashTable size
        warning                 = 47,  // len = uint32_t + length path + 1, use one entry per warning
        duplicateClassesTable   = 48, // duplicateClassesHashTable
   };

    Type         type          : 8;
    uint32_t     payloadLength : 24;

    const void*     payload() const;
    void*           payload();
};

static_assert(sizeof(TypedBytes) == 4, "Wrong size for TypedBytes");


//
//  A TypedBytes which is a bag of other TypedBytes
//
struct VIS_HIDDEN ContainerTypedBytes : TypedBytes
{
    void                forEachAttribute(void (^callback)(const TypedBytes* typedBytes, bool& stop)) const;
    void                forEachAttributePayload(Type requestedType, void (^handler)(const void* payload, uint32_t size, bool& stop)) const;
    const void*         findAttributePayload(Type requestedType, uint32_t* payloadSize=nullptr) const;
private:
    const TypedBytes*   first() const;
    const TypedBytes*   next(const TypedBytes*) const;
};


//
//  Information about a mach-o file
//
struct VIS_HIDDEN Image : ContainerTypedBytes
{
    enum class LinkKind { regular=0, weak=1, upward=2, reExport=3 };

    size_t              size() const;
    ImageNum            imageNum() const;
    bool                representsImageNum(ImageNum num) const;  // imageNum() or isOverrideOfDyldCacheImage()
    uint32_t            maxLoadCount() const;
    const char*         path() const;
    const char*         leafName() const;
    bool                getUuid(uuid_t) const;
    bool                isInvalid() const;
    bool                inDyldCache() const;
    bool                hasObjC() const;
    bool                hasInitializers() const;
    bool                hasPrecomputedObjC() const;
    bool                hasTerminators() const;
    bool                hasReadOnlyData() const;
    bool                hasChainedFixups() const;
    bool                isBundle() const;
    bool                isDylib() const;
    bool                isExecutable() const;
    bool                hasWeakDefs() const;
    bool                mayHavePlusLoads() const;
    bool                is64() const;
    bool                neverUnload() const;
    bool                cwdMustBeThisDir() const;
    bool                isPlatformBinary() const;
    bool                overridableDylib() const;
    bool                hasFileModTimeAndInode(uint64_t& inode, uint64_t& mTime) const;
    void                forEachCDHash(void (^handler)(const uint8_t cdHash[20], bool& stop)) const;
    void                forEachAlias(void (^handler)(const char* aliasPath, bool& stop)) const;
    void                forEachDependentImage(void (^handler)(uint32_t dependentIndex, LinkKind kind, ImageNum imageNum, bool& stop)) const;
    ImageNum            dependentImageNum(uint32_t depIndex) const;
    bool                containsAddress(const void* addr, const void* imageLoadAddress, uint8_t* permissions=nullptr) const;
    bool                forEachInitializerSection(void (^handler)(uint32_t sectionOffset, uint32_t sectionSize)) const;
    void                forEachInitializer(const void* imageLoadAddress, void (^handler)(const void* initializer)) const;
    void                forEachTerminator(const void* imageLoadAddress, void (^handler)(const void* terminator)) const;
    void                forEachImageToInitBefore(void (^handler)(ImageNum imageToInit, bool& stop)) const;
    void                forEachDOF(const void* imageLoadAddress, void (^handler)(const void* initializer)) const;
    bool                hasPathWithHash(const char* path, uint32_t hash) const;
    bool                isOverrideOfDyldCacheImage(ImageNum& cacheImageNum) const;
    uint64_t            textSize() const;

	union ResolvedSymbolTarget
    {
        enum Kinds { kindRebase, kindSharedCache, kindImage, kindAbsolute };

        struct Rebase {
            uint64_t    kind            :  2,       // kindRebase
                        unused          : 62;       // all zeros
        };
        struct SharedCache {
            uint64_t    kind            :  2;       // kindSharedCache
            int64_t     offset          : 62;
        };
        struct Image {
            uint64_t    kind            :  2,       // kindImage
                        imageNum        : 22;       // ImageNum
            int64_t     offset          : 40;
        };
        struct Absolute {
            uint64_t    kind            :  2,       // kindAbsolute
                        value           : 62;       // sign extended
        };
        Rebase          rebase;
        SharedCache     sharedCache;
        Image           image;
        Absolute        absolute;
        uint64_t        raw;

        bool operator==(const ResolvedSymbolTarget& rhs) const {
            return (raw == rhs.raw);
        }
        bool operator!=(const ResolvedSymbolTarget& rhs) const {
            return (raw != rhs.raw);
        }
     };
     static_assert(sizeof(ResolvedSymbolTarget) == 8);


    // ObjC optimisations
    struct ObjCImageOffset {
        union {
            uint32_t raw = 0;
            struct {
                uint32_t imageIndex  : 8;
                uint32_t imageOffset : 24;
            };
        };

        enum : uint32_t {
            // The unused value so that we have a sentinel in our hash table
            sentinelValue = 0xFFFFFFFF,

            // The maximum image index
            maximumImageIndex = (1U << 8) - 1,

            // The maximum offset from the start of the strings section
            maximumOffset = (1U << 24) - 1
        };
    };
    
    struct ObjCClassNameImageOffset {
        union {
            uint32_t raw = 0;
            struct {
                uint32_t classNameImageIndex  : 8;
                uint32_t classNameImageOffset : 24;
            };
        };
        
        enum : uint32_t {
            // The unused value so that we have a sentinel in our hash table
            sentinelValue = 0xFFFFFFFF,

            // The maximum image index
            maximumImageIndex = (1U << 8) - 1,

            // The maximum offset from the start of the strings section
            maximumOffset = (1U << 24) - 1
        };
    };
    
    struct ObjCClassImageOffset {
        union {
            uint32_t raw = 0;
            struct {
                uint32_t imageIndex         : 8;
                uint32_t imageOffset        : 23;
                uint32_t isDuplicate        : 1; // == 0
            } classData;
            struct {
                uint32_t count              : 8;
                uint32_t index              : 23;
                uint32_t isDuplicate        : 1; // == 1
            } duplicateData;
        };
        
        enum : uint32_t {
            // The unused value so that we have a sentinel in our hash table
            sentinelValue = 0xFFFFFFFF,

            // The maximum image index
            maximumImageIndex = (1U << 8) - 1,

            // The maximum offset from the start of the class data section
            maximumOffset = (1U << 23) - 1
        };
    };

    static_assert(sizeof(ObjCClassImageOffset) == 4, "Invalid size");

    static_assert(ObjCClassNameImageOffset::maximumImageIndex == ObjCClassImageOffset::maximumImageIndex , "Invalid indices");

    struct ObjCDuplicateClass {
        union {
            uint32_t raw = 0;
            struct {
                uint32_t sharedCacheClassOptIndex : 20;
                uint32_t sharedCacheClassDuplicateIndex : 12;
            };
        };

        enum : uint32_t {
            // The unused value so that we have a sentinel in our hash table
            sentinelValue = 0xFFFFFFFF
        };
    };

    static_assert(sizeof(ObjCDuplicateClass) == 4, "Invalid size");

    struct ObjCSelectorImage {
        ImageNum imageNum;
        uint32_t offset;
    };
    
    struct ObjCClassImage {
        ImageNum imageNum;
        uint32_t offsetOfClassNames;
        uint32_t offsetOfClasses;
    };

    typedef MachOLoaded::ChainedFixupPointerOnDisk ChainedFixupPointerOnDisk;

    // the following are only valid if inDyldCache() returns true
    uint32_t            cacheOffset() const;
    uint32_t            patchStartIndex() const;
    uint32_t            patchCount() const;
    void                forEachCacheSegment(void (^handler)(uint32_t segIndex, uint64_t vmOffset, uint64_t vmSize, uint8_t permissions, bool& stop)) const;


    // the following are only valid if inDyldCache() returns false
    uint64_t            vmSizeToMap() const;
    uint64_t            sliceOffsetInFile() const;
    bool                hasCodeSignature(uint32_t& fileOffset, uint32_t& size) const;
    bool                isFairPlayEncrypted(uint32_t& textOffset, uint32_t& size) const;
    void                forEachDiskSegment(void (^handler)(uint32_t segIndex, uint32_t fileOffset, uint32_t fileSize, int64_t vmOffset, uint64_t vmSize,
                                                          uint8_t permissions, bool laterReadOnly, bool& stop)) const;
    void                forEachFixup(void (^rebase)(uint64_t imageOffsetToRebase, bool& stop),
                                     void (^bind)(uint64_t imageOffsetToBind, ResolvedSymbolTarget bindTarget, bool& stop),
                                     void (^chainedFixups)(uint64_t imageOffsetToStarts, const Array<ResolvedSymbolTarget>& targets, bool& stop),
                                     void (^fixupObjCImageInfo)(uint64_t imageOffsetToFixup),
                                     void (^fixupObjCProtocol)(uint64_t imageOffsetToBind, ResolvedSymbolTarget bindTarget, bool& stop),
                                     void (^fixupObjCSelRef)(uint64_t imageOffsetToFixup, uint32_t selectorIndex, bool inSharedCache, bool& stop),
                                     void (^fixupObjCStableSwift)(uint64_t imageOffsetToFixup, bool& stop),
                                     void (^fixupObjCMethodList)(uint64_t imageOffsetToFixup, bool& stop)) const;
    void                forEachTextReloc(void (^rebase)(uint32_t imageOffsetToRebase, bool& stop),
                                         void (^bind)(uint32_t imageOffsetToBind, ResolvedSymbolTarget bindTarget, bool& stop)) const;

 	static_assert(sizeof(ResolvedSymbolTarget) == 8, "Overflow in size of SymbolTargetLocation");

    static uint32_t     hashFunction(const char*);

private:
    friend struct Closure;
    friend class ImageWriter;
    friend class ClosureBuilder;
    friend class ClosureWriter;
    friend class LaunchClosureWriter;

    uint32_t             pageSize() const;

    struct Flags
    {
        uint64_t        imageNum                     : 16,
                        maxLoadCount                 : 12,
                        isInvalid                    : 1,       // an error occurred creating the info for this image
                        has16KBpages                 : 1,
                        is64                         : 1,
                        hasObjC                      : 1,
                        mayHavePlusLoads             : 1,
                        isEncrypted                  : 1,       // image is DSMOS or FairPlay encrypted
                        hasWeakDefs                  : 1,
                        neverUnload                  : 1,
                        cwdSameAsThis                : 1,       // dylibs use file system relative paths, cwd must be main's dir
                        isPlatformBinary             : 1,       // part of OS - can be loaded into LV process
                        isBundle                     : 1,
                        isDylib                      : 1,
                        isExecutable                 : 1,
                        overridableDylib             : 1,       // only applicable to cached dylibs
                        inDyldCache                  : 1,
                        hasTerminators               : 1,
                        hasReadOnlyData              : 1,
                        hasChainedFixups             : 1,
                        hasPrecomputedObjC           : 1,
                        padding                      : 17;
    };

    static_assert(sizeof(Flags) == sizeof(uint64_t), "Flags overflow");

    const Flags&        getFlags() const;

    struct PathAndHash
    {
        uint32_t    hash;
        char        path[];
    };

    // In disk based images, all segments are multiples of page size
    // This struct just tracks the size (disk and vm) of each segment.
    // This is compact for most every image which have contiguous segments.
    // If the image does not have contiguous segments (rare), an extra
    // DiskSegment is inserted with the paddingNotSeg bit set.
    struct DiskSegment
    {
        uint64_t    filePageCount   : 30,
                    vmPageCount     : 30,
                    permissions     : 3,
                    paddingNotSeg   : 1;

        // We only have three bits for permissions, and need a way to
        // encode segments that are initially r/w then made read-only
        // after fixups are done.  Previously, the only valid patterns
        // were: ---, r--, rw-, r-x.  We now add -w- to mean r/w -> r/o.
        enum { kReadOnlyDataPermissions = VM_PROT_WRITE };
    };


    // In cache DATA_DIRTY is not page aligned or sized
    // This struct allows segments with any alignment and up to 256MB in size
    struct DyldCacheSegment
    {
        uint64_t    cacheOffset : 32,
                    size        : 28,
                    permissions : 4;
    };

    struct CodeSignatureLocation
    {
        uint32_t     fileOffset;
        uint32_t     fileSize;
    };

    struct FileInfo
    {
        uint64_t     inode;
        uint64_t     modTime;
    };

    struct FairPlayRange
    {
        uint32_t rangeStart;    // The byte offset of the start of the range.
        uint32_t rangeLength;   // How long is the fairplay range in bytes
    };

    struct MappingInfo
    {
        uint32_t     totalVmPages;
        uint32_t     sliceOffsetIn4K;
    };

    struct InitializerSectionRange
    {
        uint32_t     sectionOffset;
        uint32_t     sectionSize;
    };

    struct LinkedImage {

        LinkedImage() : imgNum(0), linkKind(0) {
        }
        LinkedImage(LinkKind k, ImageNum num) : imgNum(num), linkKind((uint32_t)k) {
            assert((num & 0xC0000000) == 0);
        }

        LinkKind    kind()  const      { return (LinkKind)linkKind; }
        ImageNum    imageNum() const   { return imgNum; }
        void        clearKind()        { linkKind = 0; }

        bool operator==(const LinkedImage& rhs) const {
            return (linkKind == rhs.linkKind) && (imgNum == rhs.imgNum);
        }
        bool operator!=(const LinkedImage& rhs) const {
            return (linkKind != rhs.linkKind) || (imgNum != rhs.imgNum);
        }
	private:
        uint32_t     imgNum         :  30,
                     linkKind       :   2;     // LinkKind
    };

    const Array<LinkedImage> dependentsArray() const;

    struct RebasePattern
    {
        uint32_t    repeatCount   : 20,     
                    contigCount   :  8, // how many contiguous pointers neeed rebasing
                    skipCount     :  4; // how many pointers to skip between contig groups
        // If contigCount == 0, then there are no rebases for this entry,
        // instead it advances the rebase location by repeatCount*skipCount.
        // If all fields are zero, then the rebase position is reset to the start.
        // This is to support old binaries with some non-monotonically-increasing rebases.
    };
    const Array<RebasePattern> rebaseFixups() const;

    struct BindPattern
    {
        Image::ResolvedSymbolTarget     target;
        uint64_t                        startVmOffset : 40, // max 1TB offset
                                        skipCount     :  8, 
                                        repeatCount   : 16;
    };
    const Array<BindPattern> bindFixups() const;

    // An optimzied selector reference bind will either point to the shared cache
    // or a binary optimized in our launch closure.  We can use the index in to each
    // of their respective selector hash tables as the target or the bind.
    union SelectorReferenceFixup
    {
        uint32_t chainStartVMOffset;
        struct {
            uint32_t    index         : 24, // max 16m entries in the table
                        next          : 7,  // (next * 4) -> offset to next fixup
                        inSharedCache : 1;  // 0 -> in the closure.  1 -> in the cache
        } chainEntry;
    };

    struct ProtocolISAFixup
    {
        uint64_t                        startVmOffset : 40, // max 1TB offset
                                        skipCount     :  8,
                                        repeatCount   : 16;
    };

    struct ClassStableSwiftFixup
    {
        uint64_t                        startVmOffset : 40, // max 1TB offset
                                        skipCount     :  8,
                                        repeatCount   : 16;
    };

    struct MethodListFixup
    {
        uint64_t                        startVmOffset : 40, // max 1TB offset
                                        skipCount     :  8,
                                        repeatCount   : 16;
    };

    void objcFixups(ResolvedSymbolTarget& objcProtocolClassTarget,
                    uint64_t& objcImageInfoVMOffset,
                    Array<ProtocolISAFixup>& protocolISAFixups,
                    Array<SelectorReferenceFixup>& selRefFixups,
                    Array<ClassStableSwiftFixup>& classStableSwiftFixups,
                    Array<MethodListFixup>& methodListFixups) const;

    struct TextFixupPattern
    {
        Image::ResolvedSymbolTarget     target;
        uint32_t                        startVmOffset;
        uint16_t                        repeatCount;
        uint16_t                        skipCount;
    };
    const Array<TextFixupPattern> textFixups() const;

    // for use with chained fixups
    uint64_t                                  chainedStartsOffset() const;
    const Array<Image::ResolvedSymbolTarget>  chainedTargets() const;

};

/*
     Dyld cache patching notes:

     The dyld cache needs to be patched to support interposing and dylib "roots".

     For cached dylibs overrides:
         Closure build time:
             1) LoadedImages will contain the new dylib, so all symbol look ups
                will naturally find new impl.  Only dyld cache needs special help.
             2) LoadedImages entry will have flag for images that override cache.
             3) When setting Closure attributes, if flag is set, builder will
                iterate PatchableExport entries in Image* from cache and create
                a PatchEntry for each.
         Runtime:
             1) [lib]dyld will iterate PatchEntry in closure and patch cache

     For interposing:
         Closure build time:
             1) After Images built, if __interpose section(s) exist, builder will
                build InterposingTuple entries for closure
             2) For being-built closure and launch closure, apply any InterposingTuple
                to modify Image fixups before Image finalized.
             3) Builder will find PatchableExport entry that matchs stock Impl
                and add PatchEntry to closure for it.
         Runtime:
             1) When closure is loaded (launch or dlopen) PatchEntries are
                applied (exactly once) to whole cache.
             2) For each DlopenClosure loaded, any InterposeTuples in *launch* closure
                are applied to all new images in new DlopenClosure.

     For weak-def coalesing:
          Closure build time:
             1) weak_bind entries are turned into -3 ordinal lookup which search through images
                in load order for first def (like flat). This fixups up all images not in cache.
             2) When processing -3 ordinals, it continues past first found and if any images
                past it are in dyld cache and export that same symbol, a PatchEntry is added to
                closure to fix up all cached uses of that symbol.
             3) If a weak_bind has strong bit set (no fixup, just def), all images from the dyld
                cache are checked to see if the export that symbol, if so, a PatchEntry is added
                to the closure.
          Runtime:
             1) When closure is loaded (launch or dlopen) PatchEntries are
                applied (exactly once) to whole cache.

*/


//
// An array (accessible by index) list of Images
//
struct VIS_HIDDEN ImageArray : public TypedBytes
{
    size_t              size() const;
    size_t              startImageNum() const;
    uint32_t            imageCount() const;
    void                forEachImage(void (^callback)(const Image* image, bool& stop)) const;
    bool                hasPath(const char* path, ImageNum& num) const;
    const Image*        imageForNum(ImageNum) const;
    void                deallocate() const;

    static const Image* findImage(const Array<const ImageArray*> imagesArrays, ImageNum imageNum);

private:
    friend class ImageArrayWriter;
    
    uint32_t        firstImageNum;
    uint32_t        count       : 31;
    uint32_t        hasRoots    : 1; // True if this ImageArray contains roots with imageNum's below firstImageNum
    uint32_t        offsets[];
    // Image data
};


struct InterposingTuple
{
    Image::ResolvedSymbolTarget    stockImplementation;
    Image::ResolvedSymbolTarget    newImplementation;
};


//
// Describes how dyld should load a set of mach-o files
//
struct VIS_HIDDEN Closure : public ContainerTypedBytes
{
    size_t              size() const;
    const ImageArray*   images() const;
    ImageNum            topImage() const;
    void                deallocate() const;

    friend class ClosureWriter;

    struct PatchEntry
    {
        ImageNum                    overriddenDylibInCache;
        uint32_t                    exportCacheOffset;
        Image::ResolvedSymbolTarget replacement;
    };

    struct Warning
    {
        enum Type : uint32_t {
            duplicateObjCClass = 0
        };

        Type        type;
        char        message[];
    };

    void                forEachPatchEntry(void (^handler)(const PatchEntry& entry)) const;
    void                forEachWarning(Warning::Type type, void (^handler)(const char* warning, bool& stop)) const;
};


//
// Describes how dyld should launch a main executable
//
struct VIS_HIDDEN LaunchClosure : public Closure
{
    // Represents a file on disk we tried to load but skipped as it wasn't valid for our use
    struct SkippedFile
    {
        const char* path;
        uint64_t    inode;
        uint64_t    mtime;
    };

    bool                builtAgainstDyldCache(uuid_t cacheUUID) const;
    const char*         bootUUID() const;
    void                forEachMustBeMissingFile(void (^handler)(const char* path, bool& stop)) const;
    void                forEachSkipIfExistsFile(void (^handler)(const SkippedFile& file, bool& stop)) const;
    void                forEachEnvVar(void (^handler)(const char* keyEqualValue, bool& stop)) const;
    ImageNum            libSystemImageNum() const;
    void                libDyldEntry(Image::ResolvedSymbolTarget& loc) const;
    bool                mainEntry(Image::ResolvedSymbolTarget& mainLoc) const;
    bool                startEntry(Image::ResolvedSymbolTarget& startLoc) const;
    uint32_t            initialLoadCount() const;
    void                forEachInterposingTuple(void (^handler)(const InterposingTuple& tuple, bool& stop)) const;
    bool                usedAtPaths() const;
    bool                usedFallbackPaths() const;
    bool                selectorHashTable(Array<Image::ObjCSelectorImage>& imageNums,
                                          const ObjCSelectorOpt*& hashTable) const;
    bool                classAndProtocolHashTables(Array<Image::ObjCClassImage>& imageNums,
                                                   const ObjCClassOpt*& classHashTable,
                                                   const ObjCClassOpt*& protocolHashTable) const;
    void                duplicateClassesHashTable(const ObjCClassDuplicatesOpt*& duplicateClassesHashTable) const;
    bool                hasInsertedLibraries() const;
    bool                hasInterposings() const;
    
    static bool         buildClosureCachePath(const char* mainExecutablePath, char closurePath[], const char* tempDir,
                                              bool makeDirsIfMissing);

private:
    friend class LaunchClosureWriter;

    struct Flags
    {
        uint32_t        usedAtPaths              :  1,
                        usedFallbackPaths        :  1,
                        initImageCount           : 16,
                        hasInsertedLibraries     :  1,
                        padding                  : 13;
    };
    const Flags&        getFlags() const;
};



//
// Describes how dyld should dlopen() a mach-o file
//
struct VIS_HIDDEN DlopenClosure : public Closure
{

};

// Precomputed perfect hash table of strings.
// Base class for closure precomputed selector table and class table.
class VIS_HIDDEN ObjCStringTable {
public:
    typedef uint32_t StringTarget;

private:
    typedef uint8_t StringHashCheckByte;

protected:

    uint32_t capacity;
    uint32_t occupied;
    uint32_t shift;
    uint32_t mask;
    StringTarget sentinelTarget;
    uint32_t roundedTabSize;
    uint64_t salt;

    uint32_t scramble[256];
    uint8_t tab[0];                   /* tab[mask+1] (always power-of-2). Rounded up to roundedTabSize */
    // uint8_t checkbytes[capacity];  /* check byte for each string */
    // int32_t offsets[capacity];     /* offsets from &capacity to cstrings */

    StringHashCheckByte* checkBytesOffset() {
        return &tab[roundedTabSize];
    }

    const StringHashCheckByte* checkBytesOffset() const {
        return &tab[roundedTabSize];
    }

    StringTarget* targetsOffset() {
        return (StringTarget*)(checkBytesOffset() + capacity);
    }

    const StringTarget* targetsOffset() const {
        return (StringTarget*)(checkBytesOffset() + capacity);
    }

    dyld3::Array<StringHashCheckByte> checkBytes() {
        return dyld3::Array<StringHashCheckByte>((StringHashCheckByte *)checkBytesOffset(), capacity, capacity);
    }
    const dyld3::Array<StringHashCheckByte> checkBytes() const {
        return dyld3::Array<StringHashCheckByte>((StringHashCheckByte *)checkBytesOffset(), capacity, capacity);
    }

    dyld3::Array<StringTarget> targets() {
        return dyld3::Array<StringTarget>((StringTarget *)targetsOffset(), capacity, capacity);
    }
    const dyld3::Array<StringTarget> targets() const {
        return dyld3::Array<StringTarget>((StringTarget *)targetsOffset(), capacity, capacity);
    }

    uint32_t hash(const char *key, size_t keylen) const;

    uint32_t hash(const char *key) const
    {
        return hash(key, strlen(key));
    }

    // The check bytes areused to reject strings that aren't in the table
    // without paging in the table's cstring data. This checkbyte calculation
    // catches 4785/4815 rejects when launching Safari; a perfect checkbyte
    // would catch 4796/4815.
    StringHashCheckByte checkbyte(const char *key, size_t keylen) const
    {
        return ((key[0] & 0x7) << 5) | ((uint8_t)keylen & 0x1f);
    }

    StringHashCheckByte checkbyte(const char *key) const
    {
        return checkbyte(key, strlen(key));
    }

    StringTarget getPotentialTarget(const char *key) const
    {
        uint32_t index = getIndex(key);
        if (index == indexNotFound)
            return sentinelTarget;
        return targets()[index];

    }

public:

    enum : uint32_t {
        indexNotFound = ~0U
    };

    uint32_t getIndex(const char *key) const
    {
        size_t keylen = strlen(key);
        uint32_t h = hash(key, keylen);

        // Use check byte to reject without paging in the table's cstrings
        StringHashCheckByte h_check = checkBytes()[h];
        StringHashCheckByte key_check = checkbyte(key, keylen);
        if (h_check != key_check)
            return indexNotFound;
        return h;
    }

    template<typename PerfectHashT>
    static size_t size(const PerfectHashT& phash) {
        // Round tab[] to at least 4 in length to ensure the uint32_t's after are aligned
        uint32_t roundedTabSize = std::max(phash.mask+1, 4U);
        size_t tableSize = 0;
        tableSize += sizeof(ObjCStringTable);
        tableSize += roundedTabSize;
        tableSize += phash.capacity * sizeof(StringHashCheckByte);
        tableSize += phash.capacity * sizeof(StringTarget);
        return tableSize;
    }
    
    // Get a string if it has an entry in the table
    const char* getString(const char* selName, const Array<uintptr_t>& baseAddresses) const;

    void forEachString(const Array<Image::ObjCSelectorImage>& selectorImages,
                       void (^callback)(uint64_t selVMOffset, ImageNum imageNum)) const;

    template<typename PerfectHashT, typename ImageOffsetT>
    void write(const PerfectHashT& phash, const Array<std::pair<const char*, ImageOffsetT>>& strings);
};

class VIS_HIDDEN ObjCSelectorOpt : public ObjCStringTable {
public:
    // Get a string if it has an entry in the table
    // Returns true if an entry is found and sets the imageNum and vmOffset.
    bool getStringLocation(uint32_t index, const Array<closure::Image::ObjCSelectorImage>& selImages,
                           ImageNum& imageNum, uint64_t& vmOffset) const;

    void forEachString(const Array<Image::ObjCSelectorImage>& selectorImages,
                       void (^callback)(uint64_t selVMOffset, ImageNum imageNum)) const;
};

class VIS_HIDDEN ObjCClassOpt : public ObjCStringTable {
private:
    // ...ObjCStringTable fields...
    // ClassTarget classOffsets[capacity]; /* offsets from &capacity to class_t and header_info */
    // DuplicateCount duplicateCount;
    // ClassTarget duplicateOffsets[duplicatedClasses];
    
    typedef uint32_t DuplicateCount;
    typedef Image::ObjCClassImageOffset ClassTarget;

    ClassTarget *classOffsetsStart() { return (ClassTarget *)&targetsOffset()[capacity]; }
    const ClassTarget *classOffsetsStart() const { return (const ClassTarget *)&targetsOffset()[capacity]; }
    
    dyld3::Array<ClassTarget> classOffsets() {
        return dyld3::Array<ClassTarget>((ClassTarget *)classOffsetsStart(), capacity, capacity);
    }
    const dyld3::Array<ClassTarget> classOffsets() const {
        return dyld3::Array<ClassTarget>((ClassTarget *)classOffsetsStart(), capacity, capacity);
    }
    
    DuplicateCount& duplicateCount() { return *(DuplicateCount *)&classOffsetsStart()[capacity]; }
    const DuplicateCount& duplicateCount() const { return *(const DuplicateCount *)&classOffsetsStart()[capacity]; }

    ClassTarget *duplicateOffsetsStart() { return (ClassTarget *)(&duplicateCount()+1); }
    const ClassTarget *duplicateOffsetsStart() const { return (const ClassTarget *)(&duplicateCount()+1); }
    
    dyld3::Array<ClassTarget> duplicateOffsets(uint32_t preCalculatedDuplicateCount) {
        return dyld3::Array<ClassTarget>((ClassTarget *)duplicateOffsetsStart(), preCalculatedDuplicateCount, duplicateCount());
    }
    const dyld3::Array<ClassTarget> duplicateOffsets(uint32_t preCalculatedDuplicateCount) const {
        return dyld3::Array<ClassTarget>((ClassTarget *)duplicateOffsetsStart(), preCalculatedDuplicateCount, duplicateCount());
    }

public:
    
    template<typename PerfectHashT>
    static size_t size(PerfectHashT& phash, uint32_t duplicateCount)
    {
        size_t tableSize = 0;
        tableSize += ObjCStringTable::size(phash);
        tableSize += phash.capacity * sizeof(ClassTarget);
        tableSize += sizeof(DuplicateCount);
        tableSize += duplicateCount * sizeof(ClassTarget);
        return tableSize;
    }

    void forEachClass(const char* className,
                      const Array<std::pair<uintptr_t, uintptr_t>>& nameAndDataBaseAddresses,
                      void (^callback)(void* classPtr, bool isLoaded, bool* stop)) const;

    void forEachClass(const Array<Image::ObjCClassImage>& classImages,
                      void (^nameCallback)(uint64_t classNameVMOffset, ImageNum imageNum),
                      void (^implCallback)(uint64_t classVMOffset, ImageNum imageNum)) const;
    
    template<typename PerfectHashT, typename ImageOffsetT, typename ClassesMapT>
    void write(const PerfectHashT& phash, const Array<std::pair<const char*, ImageOffsetT>>& strings,
               const ClassesMapT& classes, uint32_t preCalculatedDuplicateCount);
};

class VIS_HIDDEN ObjCClassDuplicatesOpt : public ObjCStringTable {
public:
    // Get a class if it has an entry in the table
    bool getClassLocation(const char* className, const objc_opt::objc_opt_t* objCOpt, void*& classImpl) const;

    void forEachClass(void (^callback)(Image::ObjCDuplicateClass duplicateClass)) const;
};

} //  namespace closure
} //  namespace dyld3


#endif // Closures_h



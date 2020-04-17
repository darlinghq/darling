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


#ifndef CacheBuilder_h
#define CacheBuilder_h

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "ClosureFileSystem.h"
#include "DyldSharedCache.h"
#include "Diagnostics.h"
#include "MachOAnalyzer.h"



template <typename P> class LinkeditOptimizer;


class CacheBuilder {
public:
    CacheBuilder(const DyldSharedCache::CreateOptions& options, const dyld3::closure::FileSystem& fileSystem);

    struct InputFile {
        enum State {
            Unset,
            MustBeIncluded,
            MustBeIncludedForDependent,
            MustBeExcludedIfUnused
        };
        InputFile(const char* path, State state) : path(path), state(state) { }
        const char*     path;
        State           state = Unset;
        Diagnostics     diag;

        bool mustBeIncluded() const {
            return (state == MustBeIncluded) || (state == MustBeIncludedForDependent);
        }
    };

    // Contains a MachO which has been loaded from the file system and may potentially need to be unloaded later.
    struct LoadedMachO {
        DyldSharedCache::MappedMachO    mappedFile;
        dyld3::closure::LoadedFileInfo  loadedFileInfo;
        InputFile*                      inputFile;
    };

    void                                        build(std::vector<InputFile>& inputFiles,
                                                      std::vector<DyldSharedCache::FileAlias>& aliases);
    void                                        build(const std::vector<LoadedMachO>& dylibs,
                                                      const std::vector<LoadedMachO>& otherOsDylibsInput,
                                                      const std::vector<LoadedMachO>& osExecutables,
                                                      std::vector<DyldSharedCache::FileAlias>& aliases);
    void                                        build(const std::vector<DyldSharedCache::MappedMachO>&  dylibsToCache,
                                                      const std::vector<DyldSharedCache::MappedMachO>&  otherOsDylibs,
                                                      const std::vector<DyldSharedCache::MappedMachO>&  osExecutables,
                                                      std::vector<DyldSharedCache::FileAlias>& aliases);
    void                                        writeFile(const std::string& path);
    void                                        writeBuffer(uint8_t*& buffer, uint64_t& size);
    void                                        writeMapFile(const std::string& path);
    std::string                                 getMapFileBuffer(const std::string& cacheDisposition) const;
    void                                        deleteBuffer();
    std::string                                 errorMessage();
    const std::set<std::string>                 warnings();
    const std::set<const dyld3::MachOAnalyzer*> evictions();
    const bool                                  agileSignature();
    const std::string                           cdHashFirst();
    const std::string                           cdHashSecond();
    const std::string                           uuid() const;

    void forEachCacheDylib(void (^callback)(const std::string& path));

    struct SegmentMappingInfo {
        const void*     srcSegment;
        const char*     segName;
        void*           dstSegment;
        uint64_t        dstCacheUnslidAddress;
        uint32_t        dstCacheFileOffset;
        uint32_t        dstCacheSegmentSize;
        uint32_t        dstCacheFileSize;
        uint32_t        copySegmentSize;
        uint32_t        srcSegmentIndex;
    };

    struct DylibTextCoalescer {

        typedef std::map<uint32_t, uint32_t> DylibSectionOffsetToCacheSectionOffset;

        DylibSectionOffsetToCacheSectionOffset objcClassNames;
        DylibSectionOffsetToCacheSectionOffset objcMethNames;
        DylibSectionOffsetToCacheSectionOffset objcMethTypes;

        bool sectionWasCoalesced(std::string_view sectionName) const;
        DylibSectionOffsetToCacheSectionOffset& getSectionCoalescer(std::string_view sectionName);
        const DylibSectionOffsetToCacheSectionOffset& getSectionCoalescer(std::string_view sectionName) const;
    };

    struct CacheCoalescedText {
        static const char* SupportedSections[3];
        struct StringSection {
            // Map from class name strings to offsets in to the class names buffer
            std::map<std::string_view, uint32_t> stringsToOffsets;
            uint8_t*                             bufferAddr       = nullptr;
            uint32_t                             bufferSize       = 0;
            uint64_t                             bufferVMAddr     = 0;

            // Note this is for debugging only
            uint64_t                             savedSpace       = 0;
        };

        StringSection objcClassNames;
        StringSection objcMethNames;
        StringSection objcMethTypes;

        void parseCoalescableText(const dyld3::MachOAnalyzer* ma,
                                   DylibTextCoalescer& textCoalescer);
        void clear();

        StringSection& getSectionData(std::string_view sectionName);
        const StringSection& getSectionData(std::string_view sectionName) const;
    };

    class ASLR_Tracker
    {
    public:
                ~ASLR_Tracker();

        void        setDataRegion(const void* rwRegionStart, size_t rwRegionSize);
        void        add(void* p);
        void        remove(void* p);
        bool        has(void* p);
        const bool* bitmap()        { return _bitmap; }
        unsigned    dataPageCount() { return _pageCount; }
        void        disable() { _enabled = false; };

    private:

        uint8_t*     _regionStart    = nullptr;
        uint8_t*     _endStart       = nullptr;
        bool*        _bitmap         = nullptr;
        unsigned     _pageCount      = 0;
        unsigned     _pageSize       = 4096;
        bool         _enabled        = true;
    };

    typedef std::map<uint64_t, std::set<void*>> LOH_Tracker;

    struct Region
    {
        uint8_t*    buffer                 = nullptr;
        uint64_t    bufferSize             = 0;
        uint64_t    sizeInUse              = 0;
        uint64_t    unslidLoadAddress      = 0;
        uint64_t    cacheFileOffset        = 0;
    };

private:
    template <typename P>
    friend class LinkeditOptimizer;
    
    struct ArchLayout
    {
        uint64_t    sharedMemoryStart;
        uint64_t    sharedMemorySize;
        uint64_t    textAndDataMaxSize;
        uint64_t    sharedRegionPadding;
        uint64_t    pointerDeltaMask;
        const char* archName;
        uint16_t    csPageSize;
        uint8_t     sharedRegionAlignP2;
        uint8_t     slideInfoBytesPerPage;
        bool        sharedRegionsAreDiscontiguous;
        bool        is64;
        bool        useValueAdd;
    };

    static const ArchLayout  _s_archLayout[];
    static const char* const _s_neverStubEliminateDylibs[];
    static const char* const _s_neverStubEliminateSymbols[];

    struct UnmappedRegion
    {
        uint8_t*    buffer                 = nullptr;
        uint64_t    bufferSize             = 0;
        uint64_t    sizeInUse              = 0;
    };

    struct DylibInfo
    {
        const LoadedMachO*              input;
        std::string                     runtimePath;
        std::vector<SegmentMappingInfo> cacheLocation;
        DylibTextCoalescer              textCoalescer;
    };

    void        makeSortedDylibs(const std::vector<LoadedMachO>& dylibs, const std::unordered_map<std::string, unsigned> sortOrder);
    void        processSelectorStrings(const std::vector<LoadedMachO>& executables);
    void        parseCoalescableSegments();
    void        assignSegmentAddresses();

    uint64_t    cacheOverflowAmount();
    size_t      evictLeafDylibs(uint64_t reductionTarget, std::vector<const LoadedMachO*>& overflowDylibs);

    void        fipsSign();
    void        codeSign();
    uint64_t    pathHash(const char* path);
    void        writeCacheHeader();
    void        copyRawSegments();
    void        adjustAllImagesForNewSegmentLocations();
    void        writeSlideInfoV1();
    void        writeSlideInfoV3(const bool bitmap[], unsigned dataPageCoun);
    uint16_t    pageStartV3(uint8_t* pageContent, uint32_t pageSize, const bool bitmap[]);
    void        findDylibAndSegment(const void* contentPtr, std::string& dylibName, std::string& segName);
    void        addImageArray();
    void        buildImageArray(std::vector<DyldSharedCache::FileAlias>& aliases);
    void        addOtherImageArray(const std::vector<LoadedMachO>&, std::vector<const LoadedMachO*>& overflowDylibs);
    void        addClosures(const std::vector<LoadedMachO>&);
    void        markPaddingInaccessible();

    bool        writeCache(void (^cacheSizeCallback)(uint64_t size), bool (^copyCallback)(const uint8_t* src, uint64_t size, uint64_t dstOffset));

    template <typename P> void writeSlideInfoV2(const bool bitmap[], unsigned dataPageCount);
    template <typename P> bool makeRebaseChainV2(uint8_t* pageContent, uint16_t lastLocationOffset, uint16_t newOffset, const struct dyld_cache_slide_info2* info);
    template <typename P> void addPageStartsV2(uint8_t* pageContent, const bool bitmap[], const struct dyld_cache_slide_info2* info,
                                             std::vector<uint16_t>& pageStarts, std::vector<uint16_t>& pageExtras);

    template <typename P> void writeSlideInfoV4(const bool bitmap[], unsigned dataPageCount);
    template <typename P> bool makeRebaseChainV4(uint8_t* pageContent, uint16_t lastLocationOffset, uint16_t newOffset, const struct dyld_cache_slide_info4* info);
    template <typename P> void addPageStartsV4(uint8_t* pageContent, const bool bitmap[], const struct dyld_cache_slide_info4* info,
                                             std::vector<uint16_t>& pageStarts, std::vector<uint16_t>& pageExtras);

    // implemented in AdjustDylibSegemnts.cpp
    void        adjustDylibSegments(const DylibInfo& dylib, Diagnostics& diag) const;

    // implemented in OptimizerLinkedit.cpp
    void        optimizeLinkedit();

    // implemented in OptimizerObjC.cpp
    void        optimizeObjC();
    uint32_t    computeReadOnlyObjC(uint32_t selRefCount, uint32_t classDefCount, uint32_t protocolDefCount);
    uint32_t    computeReadWriteObjC(uint32_t imageCount, uint32_t protocolDefCount);

    // implemented in OptimizerBranches.cpp
    void        optimizeAwayStubs();

    typedef std::unordered_map<std::string, const dyld3::MachOAnalyzer*> InstallNameToMA;

    typedef uint64_t                                                CacheOffset;

    const DyldSharedCache::CreateOptions&       _options;
    const dyld3::closure::FileSystem&           _fileSystem;
    Region                                      _readExecuteRegion;
    Region                                      _readWriteRegion;
    Region                                      _readOnlyRegion;
    UnmappedRegion                              _localSymbolsRegion;
    UnmappedRegion                              _codeSignatureRegion;
    vm_address_t                                _fullAllocatedBuffer;
    uint64_t                                    _nonLinkEditReadOnlySize;
    Diagnostics                                 _diagnostics;
    std::set<const dyld3::MachOAnalyzer*>       _evictions;
    const ArchLayout*                           _archLayout;
    uint32_t                                    _aliasCount;
    uint64_t                                    _slideInfoFileOffset;
    uint64_t                                    _slideInfoBufferSizeAllocated;
    uint8_t*                                    _objcReadOnlyBuffer;
    uint64_t                                    _objcReadOnlyBufferSizeUsed;
    uint64_t                                    _objcReadOnlyBufferSizeAllocated;
    uint8_t*                                    _objcReadWriteBuffer;
    uint64_t                                    _objcReadWriteBufferSizeAllocated;
    uint64_t                                    _allocatedBufferSize;
    CacheCoalescedText                          _coalescedText;
    uint64_t                                    _selectorStringsFromExecutables;
    std::vector<DylibInfo>                      _sortedDylibs;
    InstallNameToMA                             _installNameToCacheDylib;
    std::unordered_map<std::string, uint32_t>   _dataDirtySegsOrder;
    // Note this is mutable as the only parallel writes to it are done atomically to the bitmap
    mutable ASLR_Tracker                        _aslrTracker;
    std::map<void*, std::string>                _missingWeakImports;
    mutable LOH_Tracker                         _lohTracker;
    const dyld3::closure::ImageArray*           _imageArray;
    uint32_t                                    _sharedStringsPoolVmOffset;
    uint8_t                                     _cdHashFirst[20];
    uint8_t                                     _cdHashSecond[20];
    std::unordered_map<const dyld3::MachOLoaded*, std::set<CacheOffset>>        _dylibToItsExports;
    std::set<std::pair<const dyld3::MachOLoaded*, CacheOffset>>                 _dylibWeakExports;
    std::unordered_map<CacheOffset, std::vector<dyld_cache_patchable_location>> _exportsToUses;
    std::unordered_map<CacheOffset, std::string>                                _exportsToName;
};




inline uint64_t align(uint64_t addr, uint8_t p2)
{
    uint64_t mask = (1 << p2);
    return (addr + mask - 1) & (-mask);
}



#endif /* CacheBuilder_h */

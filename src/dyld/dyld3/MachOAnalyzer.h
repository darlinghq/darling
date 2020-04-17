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

#ifndef MachOAnalyzer_h
#define MachOAnalyzer_h


#include "MachOLoaded.h"
#include "Array.h"
#include "ClosureFileSystem.h"


namespace dyld3 {

// Extra functionality on loaded mach-o files only used during closure building
struct VIS_HIDDEN MachOAnalyzer : public MachOLoaded
{
    // protected members of subclass promoted to public here
    using MachOLoaded::SegmentInfo;
    using MachOLoaded::SectionInfo;
    using MachOLoaded::forEachSegment;
    using MachOLoaded::forEachSection;
    using MachOLoaded::forEachDependentDylib;
    using MachOLoaded::getDylibInstallName;
    using MachOLoaded::FoundSymbol;
    using MachOLoaded::findExportedSymbol;

    static closure::LoadedFileInfo load(Diagnostics& diag, const closure::FileSystem& fileSystem,
                                        const char* logicalPath, const GradedArchs& archs, Platform platform, char realerPath[MAXPATHLEN]);
    static const MachOAnalyzer*  validMainExecutable(Diagnostics& diag, const mach_header* mh, const char* path, uint64_t sliceLength,
                                                     const GradedArchs& archs, Platform platform);

    typedef void (^ExportsCallback)(const char* symbolName, uint64_t imageOffset, uint64_t flags,
                                    uint64_t other, const char* importName, bool& stop);
    bool                validMachOForArchAndPlatform(Diagnostics& diag, size_t mappedSize, const char* path, const GradedArchs& archs, Platform platform) const;
    uint64_t            mappedSize() const;
    bool                hasObjC() const;
    bool                hasPlusLoadMethod(Diagnostics& diag) const;
    uint64_t            preferredLoadAddress() const;
    void                forEachLocalSymbol(Diagnostics& diag, void (^callback)(const char* symbolName, uint64_t n_value, uint8_t n_type, uint8_t n_sect, uint16_t n_desc, bool& stop)) const;
    void                forEachRPath(void (^callback)(const char* rPath, bool& stop)) const;

    bool                isEncrypted() const;
    void                forEachCDHash(void (^handler)(const uint8_t cdHash[20])) const;
    bool                hasCodeSignature(uint32_t& fileOffset, uint32_t& size) const;
    bool                usesLibraryValidation() const;
    bool                isRestricted() const;
    bool                getEntry(uint32_t& offset, bool& usesCRT) const;
    bool                isSlideable() const;
    bool                hasInitializer(Diagnostics& diag, bool contentRebased, const void* dyldCache=nullptr) const;
    void                forEachInitializerPointerSection(Diagnostics& diag, void (^callback)(uint32_t sectionOffset, uint32_t sectionSize, const uint8_t* content, bool& stop)) const;
    void                forEachInitializer(Diagnostics& diag, bool contentRebased, void (^callback)(uint32_t offset), const void* dyldCache=nullptr) const;
    bool                hasTerminators(Diagnostics& diag, bool contentRebased) const;
    void                forEachTerminator(Diagnostics& diag, bool contentRebased, void (^callback)(uint32_t offset)) const;
    void                forEachDOFSection(Diagnostics& diag, void (^callback)(uint32_t offset)) const;
    uint32_t            segmentCount() const;
    void                forEachExportedSymbol(Diagnostics& diag, ExportsCallback callback) const;
    void                forEachWeakDef(Diagnostics& diag, void (^callback)(bool strongDef, uint32_t dataSegIndex, uint64_t dataSegOffset,
                                                    uint64_t addend, const char* symbolName, bool& stop)) const;
    void                forEachIndirectPointer(Diagnostics& diag, void (^handler)(uint64_t pointerAddress, bool bind, int bindLibOrdinal,
                                                                                  const char* bindSymbolName, bool bindWeakImport, bool bindLazy, bool selfModifyingStub, bool& stop)) const;
    void                forEachInterposingSection(Diagnostics& diag, void (^handler)(uint64_t vmOffset, uint64_t vmSize, bool& stop)) const;
    const void*         content(uint64_t vmOffset);
    void                forEachLocalReloc(void (^handler)(uint64_t runtimeOffset, bool& stop)) const;
    void                forEachExternalReloc(void (^handler)(uint64_t runtimeOffset, int libOrdinal, const char* symbolName, bool& stop)) const;

    const void*         getRebaseOpcodes(uint32_t& size) const;
    const void*         getBindOpcodes(uint32_t& size) const;
    const void*         getLazyBindOpcodes(uint32_t& size) const;
    const void*         getSplitSeg(uint32_t& size) const;
    uint64_t            segAndOffsetToRuntimeOffset(uint8_t segIndex, uint64_t segOffset) const;
    bool                hasLazyPointers(uint32_t& runtimeOffset, uint32_t& size) const;
    void                forEachRebase(Diagnostics& diag, bool ignoreLazyPointer, void (^callback)(uint64_t runtimeOffset, bool& stop)) const;
    void                forEachTextRebase(Diagnostics& diag, void (^callback)(uint64_t runtimeOffset, bool& stop)) const;
    void                forEachBind(Diagnostics& diag, void (^callback)(uint64_t runtimeOffset, int libOrdinal, const char* symbolName,
                                                                        bool weakImport, bool lazyBind, uint64_t addend, bool& stop),
                                                       void (^strongHandler)(const char* symbolName),
                                                       void (^missingLazyBindHandler)()) const;
    void                forEachChainedFixupTarget(Diagnostics& diag, void (^callback)(int libOrdinal, const char* symbolName, uint64_t addend, bool weakImport, bool& stop)) const;
    bool                canHavePrecomputedDlopenClosure(const char* path, void (^failureReason)(const char*)) const;
    void                forEachRebase(Diagnostics& diag, void (^handler)(const char* opcodeName, const LinkEditInfo& leInfo, const SegmentInfo segments[],
                                                                             bool segIndexSet, uint32_t pointerSize, uint8_t segmentIndex, uint64_t segmentOffset, uint8_t type, bool& stop)) const;
    void                forEachBind(Diagnostics& diag, void (^handler)(const char* opcodeName, const LinkEditInfo& leInfo, const SegmentInfo segments[],
                                                                       bool segIndexSet,  bool libraryOrdinalSet, uint32_t dylibCount, int libOrdinal,
                                                                       uint32_t pointerSize, uint8_t segmentIndex, uint64_t segmentOffset,
                                                                       uint8_t type, const char* symbolName, bool weakImport, bool lazyBind, uint64_t addend, bool& stop),
                                                       void (^strongHandler)(const char* symbolName),
                                                       void (^missingLazyBindHandler)()) const;
    bool                canBePlacedInDyldCache(const char* path, void (^failureReason)(const char*)) const;
    uint32_t            loadCommandsFreeSpace() const;
 
#if DEBUG
    void                validateDyldCacheDylib(Diagnostics& diag, const char* path) const;
#endif
    void                withChainStarts(Diagnostics& diag, uint64_t startsStructOffsetHint, void (^callback)(const dyld_chained_starts_in_image*)) const;
    uint64_t            chainStartsOffset() const;
    uint16_t            chainedPointerFormat() const;
    bool                hasUnalignedPointerFixups() const;

    const MachOAnalyzer*    remapIfZeroFill(Diagnostics& diag, const closure::FileSystem& fileSystem, closure::LoadedFileInfo& info) const;

    struct ObjCInfo {
        uint32_t    selRefCount;
        uint32_t    classDefCount;
        uint32_t    protocolDefCount;
    };
    ObjCInfo            getObjCInfo() const;

    // This optionally caches a list of sections for lookup
    struct SectionCache {
    private:
        SectionInfo buffer[2];

    public:
        SectionCache(const MachOAnalyzer* ma) : ma(ma) { }

        bool findSectionForVMAddr(uint64_t vmAddr, bool (^sectionHandler)(const SectionInfo& sectionInfo));

        const MachOAnalyzer*                  ma           = nullptr;
        dyld3::OverflowSafeArray<SectionInfo> sectionInfos = { buffer, sizeof(buffer) / sizeof(buffer[0]) };
    };

    // Caches data useful for converting from raw data to VM addresses
    struct VMAddrConverter {
        uint64_t preferredLoadAddress   = 0;
        intptr_t slide                  = 0;
        uint16_t chainedPointerFormat   = 0;
        bool contentRebased             = false;
    };

    struct ObjCClassInfo {
        // These fields are all present on the objc_class_t struct
        uint64_t isaVMAddr                                  = 0;
        uint64_t superclassVMAddr                           = 0;
        //uint64_t methodCacheBuckets;
        //uint64_t methodCacheProperties;
        uint64_t dataVMAddr                                 = 0;

        // This field is only present if this is a Swift object, ie, has the Swift
        // fast bits set
        uint32_t swiftClassFlags                            = 0;

        // These are taken from the low bits of the dataVMAddr value
        bool     isSwiftLegacy                              = false;
        bool     isSwiftStable                              = false;

        // Cache the data to convert vmAddr's
        MachOAnalyzer::VMAddrConverter  vmAddrConverter;

        // These are from the class_ro_t which data points to
        enum class ReadOnlyDataField {
            name,
            baseMethods
        };

        uint64_t getReadOnlyDataField(ReadOnlyDataField field, uint32_t pointerSize) const;
        uint64_t nameVMAddr(uint32_t pointerSize) const {
            return getReadOnlyDataField(ReadOnlyDataField::name, pointerSize);
        }
        uint64_t baseMethodsVMAddr(uint32_t pointerSize) const {
            return getReadOnlyDataField(ReadOnlyDataField::baseMethods, pointerSize);
        }

        // These are embedded in the Mach-O itself by the compiler
        enum FastDataBits {
            FAST_IS_SWIFT_LEGACY    = 0x1,
            FAST_IS_SWIFT_STABLE    = 0x2
        };

        // These are embedded by the Swift compiler in the swiftClassFlags field
        enum SwiftClassFlags {
            isSwiftPreStableABI     = 0x1
        };

        // Note this is taken from the objc runtime
        bool isUnfixedBackwardDeployingStableSwift() const {
            // Only classes marked as Swift legacy need apply.
            if (!isSwiftLegacy) return false;

            // Check the true legacy vs stable distinguisher.
            // The low bit of Swift's ClassFlags is SET for true legacy
            // and UNSET for stable pretending to be legacy.
            bool isActuallySwiftLegacy = (swiftClassFlags & isSwiftPreStableABI) != 0;
            return !isActuallySwiftLegacy;
        }
    };

    struct ObjCImageInfo {
        uint32_t version;
        uint32_t flags;

        // FIXME: Put this somewhere objc can see it.
        enum : uint32_t {
            dyldPreoptimized = 1 << 7
        };
    };

    struct ObjCMethod {
        uint64_t nameVMAddr;   // & SEL
        uint64_t typesVMAddr;  // & const char *
        uint64_t impVMAddr;    // & IMP

        // We also need to know where the reference to the nameVMAddr was
        // This is so that we know how to rebind that location
        uint64_t nameLocationVMAddr;
    };

    struct ObjCCategory {
        uint64_t nameVMAddr;
        uint64_t clsVMAddr;
        uint64_t instanceMethodsVMAddr;
        uint64_t classMethodsVMAddr;
        uint64_t protocolsVMAddr;
        uint64_t instancePropertiesVMAddr;
    };

    struct ObjCProtocol {
        uint64_t isaVMAddr;
        uint64_t nameVMAddr;
        //uint64_t protocolsVMAddr;
        uint64_t instanceMethodsVMAddr;
        uint64_t classMethodsVMAddr;
        uint64_t optionalInstanceMethodsVMAddr;
        uint64_t optionalClassMethodsVMAddr;
        //uint64_t instancePropertiesVMAddr;
        //uint32_t size;
        //uint32_t flags;
        // Fields below this point are not always present on disk.
        //uint64_t extendedMethodTypesVMAddr;
        //uint64_t demangledNameVMAddr;
        //uint64_t classPropertiesVMAddr;

        // Note this isn't in a protocol, but we use it in dyld to track if the protocol
        // is large enough to avoid a reallocation in objc.
        bool requiresObjCReallocation;
    };

    enum class PrintableStringResult {
        CanPrint,
        FairPlayEncrypted,
        ProtectedSection,
        UnknownSection
    };

    const char* getPrintableString(uint64_t stringVMAddr, PrintableStringResult& result,
                                   SectionCache* sectionCache = nullptr,
                                   bool (^sectionHandler)(const SectionInfo& sectionInfo) = nullptr) const;
    
    void forEachObjCClass(Diagnostics& diag, bool contentRebased,
                          void (^handler)(Diagnostics& diag, uint64_t classVMAddr,
                                          uint64_t classSuperclassVMAddr, uint64_t classDataVMAddr,
                                          const ObjCClassInfo& objcClass, bool isMetaClass)) const;

    void forEachObjCCategory(Diagnostics& diag, bool contentRebased,
                             void (^handler)(Diagnostics& diag, uint64_t categoryVMAddr,
                                             const dyld3::MachOAnalyzer::ObjCCategory& objcCategory)) const;

    void forEachObjCProtocol(Diagnostics& diag, bool contentRebased,
                             void (^handler)(Diagnostics& diag, uint64_t protocolVMAddr,
                                             const dyld3::MachOAnalyzer::ObjCProtocol& objCProtocol)) const;

    // Walk a method list starting from its vmAddr.
    // Note, classes, categories, protocols, etc, all share the same method list struture so can all use this.
    void forEachObjCMethod(uint64_t methodListVMAddr, bool contentRebased,
                           void (^handler)(uint64_t methodVMAddr, const ObjCMethod& method)) const;

    void forEachObjCSelectorReference(Diagnostics& diag, bool contentRebased,
                                      void (^handler)(uint64_t selRefVMAddr, uint64_t selRefTargetVMAddr)) const;

    void forEachObjCMethodName(void (^handler)(const char* methodName)) const;

    bool hasObjCMessageReferences() const;

    const ObjCImageInfo* objcImageInfo() const;

    void forEachWeakDef(Diagnostics& diag, void (^handler)(const char* symbolName, uintptr_t imageOffset, bool isFromExportTrie)) const;
    
private:

    struct SegmentStuff
    {
        uint64_t    fileOffset;
        uint64_t    fileSize;
        uint64_t    writable          :  1,
                    executable        :  1,
                    textRelocsAllowed :  1,  // segment supports text relocs (i386 only)
                    segSize           : 61;
	};

    enum class Malformed { linkeditOrder, linkeditAlignment, linkeditPermissions, dyldInfoAndlocalRelocs, segmentOrder, textPermissions, executableData, codeSigAlignment };
    bool                    enforceFormat(Malformed) const;

    const uint8_t*          getContentForVMAddr(const LayoutInfo& info, uint64_t vmAddr) const;
    bool                    validLoadCommands(Diagnostics& diag, const char* path, size_t fileLen) const;
    bool                    validEmbeddedPaths(Diagnostics& diag, Platform platform, const char* path) const;
    bool                    validSegments(Diagnostics& diag, const char* path, size_t fileLen) const;
    bool                    validLinkedit(Diagnostics& diag, const char* path) const;
    bool                    validLinkeditLayout(Diagnostics& diag, const char* path) const;
    bool                    validRebaseInfo(Diagnostics& diag, const char* path) const;
    bool                    validBindInfo(Diagnostics& diag, const char* path) const;
    bool                    validMain(Diagnostics& diag, const char* path) const;
    bool                    validChainedFixupsInfo(Diagnostics& diag, const char* path) const;

    bool                    invalidRebaseState(Diagnostics& diag, const char* opcodeName, const char* path, const LinkEditInfo& leInfo, const SegmentInfo segments[],
                                              bool segIndexSet, uint32_t pointerSize, uint8_t segmentIndex, uint64_t segmentOffset, uint8_t type) const;
    bool                    invalidBindState(Diagnostics& diag, const char* opcodeName, const char* path, const LinkEditInfo& leInfo, const SegmentInfo segments[],
                                              bool segIndexSet,  bool libraryOrdinalSet, uint32_t dylibCount, int libOrdinal, uint32_t pointerSize,
                                              uint8_t segmentIndex, uint64_t segmentOffset, uint8_t type, const char* symbolName) const;
    bool                    doLocalReloc(Diagnostics& diag, uint32_t r_address, bool& stop, void (^callback)(uint32_t dataSegIndex, uint64_t dataSegOffset, uint8_t type, bool& stop)) const;
    uint8_t                 relocPointerType() const;
    int                     libOrdinalFromDesc(uint16_t n_desc) const;
    bool                    doExternalReloc(Diagnostics& diag, uint32_t r_address, uint32_t r_symbolnum, LinkEditInfo& leInfo, bool& stop,
                                            void (^callback)(uint32_t dataSegIndex, uint64_t dataSegOffset, uint8_t type, int libOrdinal,
                                                             uint64_t addend, const char* symbolName, bool weakImport, bool lazy, bool& stop)) const;

    void                    getAllSegmentsInfos(Diagnostics& diag, SegmentInfo segments[]) const;
    bool                    segmentHasTextRelocs(uint32_t segIndex) const;
    uint64_t                relocBaseAddress(const SegmentInfo segmentsInfos[], uint32_t segCount) const;
    bool                    segIndexAndOffsetForAddress(uint64_t addr, const SegmentInfo segmentsInfos[], uint32_t segCount, uint32_t& segIndex, uint64_t& segOffset) const;
    void                    parseOrgArm64eChainedFixups(Diagnostics& diag, void (^targetCount)(uint32_t totalTargets, bool& stop),
                                                                           void (^addTarget)(const LinkEditInfo& leInfo, const SegmentInfo segments[], bool libraryOrdinalSet, uint32_t dylibCount, int libOrdinal, uint8_t type, const char* symbolName, uint64_t addend, bool weakImport, bool& stop),
                                                                           void (^addChainStart)(const LinkEditInfo& leInfo, const SegmentInfo segments[], uint8_t segmentIndex, bool segIndexSet, uint64_t segmentOffset, uint16_t format, bool& stop)) const;
    bool                    contentIsRegularStub(const uint8_t* helperContent) const;
    uint64_t                entryAddrFromThreadCmd(const thread_command* cmd) const;
    void                    recurseTrie(Diagnostics& diag, const uint8_t* const start, const uint8_t* p, const uint8_t* const end,
                                        OverflowSafeArray<char>& cummulativeString, int curStrOffset, bool& stop, MachOAnalyzer::ExportsCallback callback) const;
    void                    analyzeSegmentsLayout(uint64_t& vmSpace, bool& hasZeroFill) const;

};

} // namespace dyld3

#endif /* MachOAnalyzer_h */

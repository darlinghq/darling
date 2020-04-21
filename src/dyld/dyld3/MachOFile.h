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


#ifndef MachOFile_h
#define MachOFile_h

#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <uuid/uuid.h>

#include "Diagnostics.h"
#include "SupportedArchs.h"
#include <mach-o/fixup-chains.h>
#include <mach-o/loader.h>

// needed until dyld builds with a newer SDK
#ifndef CPU_SUBTYPE_ARM64E
  #define CPU_SUBTYPE_ARM64E 2
#endif
#ifndef CPU_TYPE_ARM64_32
  #define CPU_TYPE_ARM64_32 0x0200000C
#endif
#ifndef CPU_SUBTYPE_ARM64_32_V8
  #define CPU_SUBTYPE_ARM64_32_V8 1
#endif
#ifndef BIND_OPCODE_THREADED
    #define BIND_OPCODE_THREADED    0xD0
#endif
#ifndef BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB
    #define BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB    0x00
#endif
#ifndef BIND_SUBOPCODE_THREADED_APPLY
    #define BIND_SUBOPCODE_THREADED_APPLY                               0x01
#endif
#ifndef BIND_SPECIAL_DYLIB_WEAK_LOOKUP
  #define BIND_SPECIAL_DYLIB_WEAK_LOOKUP                (-3)
#endif
#ifndef EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE
  #define EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE             0x02
#endif
#ifndef SG_READ_ONLY
  #define SG_READ_ONLY            0x10
#endif

#ifndef LC_DYLD_EXPORTS_TRIE
  #define LC_DYLD_EXPORTS_TRIE    0x80000033
#endif
#ifndef LC_DYLD_CHAINED_FIXUPS
  #define LC_DYLD_CHAINED_FIXUPS  0x80000034
#endif

#ifndef S_INIT_FUNC_OFFSETS
  #define S_INIT_FUNC_OFFSETS       0x16
#endif

namespace dyld3 {



/// Returns true if (addLHS + addRHS) > b, or if the add overflowed
template<typename T>
inline bool greaterThanAddOrOverflow(uint32_t addLHS, uint32_t addRHS, T b) {
    return (addLHS > b) || (addRHS > (b-addLHS));
}

/// Returns true if (addLHS + addRHS) > b, or if the add overflowed
template<typename T>
inline bool greaterThanAddOrOverflow(uint64_t addLHS, uint64_t addRHS, T b) {
    return (addLHS > b) || (addRHS > (b-addLHS));
}


// Note, this should match PLATFORM_* values in <mach-o/loader.h>
enum class Platform {
    unknown             = 0,
    macOS               = 1,    // PLATFORM_MACOS
    iOS                 = 2,    // PLATFORM_IOS
    tvOS                = 3,    // PLATFORM_TVOS
    watchOS             = 4,    // PLATFORM_WATCHOS
    bridgeOS            = 5,    // PLATFORM_BRIDGEOS
    iOSMac              = 6,    // PLATFORM_IOSMAC
    iOS_simulator       = 7,    // PLATFORM_IOSSIMULATOR
    tvOS_simulator      = 8,    // PLATFORM_TVOSSIMULATOR
    watchOS_simulator   = 9,    // PLATFORM_WATCHOSSIMULATOR
    driverKit           = 10,   // PLATFORM_DRIVERKIT
};

struct MachOFile; // forward ref

// A prioritized list of architectures
class VIS_HIDDEN GradedArchs {
public:
    // never construct new ones - just use existing static instances
    GradedArchs()                   = delete;
    GradedArchs(const GradedArchs&) = delete;

    static const GradedArchs&  forCurrentOS(const MachOFile* mainExecutable);
    static const GradedArchs&  forName(const char* archName, bool forMainExecutable = false);

    int                     grade(uint32_t cputype, uint32_t cpusubtype) const;
    const char*             name() const;

    // pre-built lists for existing hardware
    static const GradedArchs i386;            // 32-bit Mac
    static const GradedArchs x86_64;          // older Mac
    static const GradedArchs x86_64h;         // haswell Mac
    static const GradedArchs arm64;           // A11 or earlier iPhone or iPad
#if SUPPORT_ARCH_arm64e
    static const GradedArchs arm64e;          // A12 or later iPhone or iPad
    static const GradedArchs arm64e_compat;   // A12 running arm64 main executable
#endif
    static const GradedArchs armv7k;          // watch thru series 3
    static const GradedArchs armv7s;          // deprecated
    static const GradedArchs armv7;           // deprecated
#if SUPPORT_ARCH_arm64_32
    static const GradedArchs arm64_32;        // watch series 4 and later
#endif

// private:
// should be private, but compiler won't statically initialize static members above
    struct CpuGrade { uint32_t type; uint32_t subtype; uint32_t grade; };
    const CpuGrade     _orderedCpuTypes[3];  // zero terminated
};


// A file read/mapped into memory
struct VIS_HIDDEN FatFile : fat_header
{
    static const FatFile*  isFatFile(const void* fileContent);
    void                   forEachSlice(Diagnostics& diag, uint64_t fileLen, void (^callback)(uint32_t sliceCpuType, uint32_t sliceCpuSubType, const void* sliceStart, uint64_t sliceSize, bool& stop)) const;
    bool                   isFatFileWithSlice(Diagnostics& diag, uint64_t fileLen, const GradedArchs& archs, uint64_t& sliceOffset, uint64_t& sliceLen, bool& missingSlice) const;

private:
    bool                   isValidSlice(Diagnostics& diag, uint64_t fileLen, uint32_t sliceIndex,
                                        uint32_t sliceCpuType, uint32_t sliceCpuSubType, uint64_t sliceOffset, uint64_t sliceLen) const;
};


// A mach-o file read/mapped into memory
// Only info from mach_header or load commands is accessible (no LINKEDIT info)
struct VIS_HIDDEN MachOFile : mach_header
{
    static const char*      archName(uint32_t cputype, uint32_t cpusubtype);
    static const char*      platformName(Platform platform);
    static uint32_t         cpuTypeFromArchName(const char* archName);
    static uint32_t         cpuSubtypeFromArchName(const char* archName);
    static void             packedVersionToString(uint32_t packedVersion, char versionString[32]);
    static const char*      currentArchName();
    static Platform         currentPlatform();
    static uint64_t         read_uleb128(Diagnostics& diag, const uint8_t*& p, const uint8_t* end);
    static int64_t          read_sleb128(Diagnostics& diag, const uint8_t*& p, const uint8_t* end);
    static bool             isSimulatorPlatform(Platform platform);
    static bool             isSharedCacheEligiblePath(const char* path);

    bool            hasMachOMagic() const;
    bool            isMachO(Diagnostics& diag, uint64_t fileSize) const;
    bool            isDylib() const;
    bool            isBundle() const;
    bool            isMainExecutable() const;
    bool            isDynamicExecutable() const;
    bool            isStaticExecutable() const;
    bool            isPreload() const;
    bool            isPIE() const;
    bool            isArch(const char* archName) const;
    const char*     archName() const;
    bool            is64() const;
    size_t          machHeaderSize() const;
    uint32_t        pointerSize() const;
    bool            uses16KPages() const;
    bool            supportsPlatform(Platform) const;
    bool            isZippered() const;
    bool            inDyldCache() const;
    bool            isSimulatorBinary() const;
    bool            getUuid(uuid_t uuid) const;
    bool            hasWeakDefs() const;
    bool            hasThreadLocalVariables() const;
    bool            getDylibInstallName(const char** installName, uint32_t* compatVersion, uint32_t* currentVersion) const;
    void            forEachSupportedPlatform(void (^callback)(Platform platform, uint32_t minOS, uint32_t sdk)) const;
    const char*     installName() const;  // returns nullptr is no install name
    void            forEachDependentDylib(void (^callback)(const char* loadPath, bool isWeak, bool isReExport, bool isUpward, uint32_t compatVersion, uint32_t curVersion, bool& stop)) const;
    bool            canBePlacedInDyldCache(const char* path, void (^failureReason)(const char*)) const;
    bool            canBeFairPlayEncrypted() const;
    bool            isFairPlayEncrypted(uint32_t& textOffset, uint32_t& size) const;
    bool            allowsAlternatePlatform() const;
    bool            hasChainedFixups() const;
    void            forDyldEnv(void (^callback)(const char* envVar, bool& stop)) const;
    bool            enforceCompatVersion() const;

    struct SegmentInfo
    {
        uint64_t    fileOffset;
        uint64_t    fileSize;
        uint64_t    vmAddr;
        uint64_t    vmSize;
        uint64_t    sizeOfSections;
        const char* segName;
        uint32_t    loadCommandOffset;
        uint32_t    protections;
        uint32_t    textRelocs    :  1,  // segment has text relocs (i386 only)
                    readOnlyData  :  1,
                    isProtected   :  1,  // segment is protected
                    segIndex      : 13,
                    p2align       : 16;
        bool        readable() const   { return protections & VM_PROT_READ; }
        bool        writable() const   { return protections & VM_PROT_WRITE; }
        bool        executable() const { return protections & VM_PROT_EXECUTE; }
     };

    struct SectionInfo
    {
        SegmentInfo segInfo;
        uint64_t    sectAddr;
        uint64_t    sectSize;
        const char* sectName;
        uint32_t    sectFileOffset;
        uint32_t    sectFlags;
        uint32_t    sectAlignP2;
        uint32_t    reserved1;
        uint32_t    reserved2;
    };

    void            forEachSegment(void (^callback)(const SegmentInfo& info, bool& stop)) const;
    void            forEachSection(void (^callback)(const SectionInfo& sectInfo, bool malformedSectionRange, bool& stop)) const;

protected:
    bool            hasMachOBigEndianMagic() const;
    void            forEachLoadCommand(Diagnostics& diag, void (^callback)(const load_command* cmd, bool& stop)) const;
    bool            hasLoadCommand(uint32_t) const;

    const encryption_info_command* findFairPlayEncryptionLoadCommand() const;

    struct ArchInfo
    {
        const char* name;
        uint32_t    cputype;
        uint32_t    cpusubtype;
    };
    static const ArchInfo       _s_archInfos[];

    struct PlatformInfo
    {
        const char* name;
        Platform    platform;
        uint32_t    loadCommand;
    };
    static const PlatformInfo   _s_platformInfos[];
};


} // namespace dyld3

#endif /* MachOFile_h */

/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2008-2010 Apple Inc. All rights reserved.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <vector>
#include <tuple>
#include <set>
#include <unordered_set>
#include <string>

#include "Array.h"
#include "MachOFile.h"
#include "MachOLoaded.h"
#include "MachOAnalyzer.h"
#include "ClosureFileSystemPhysical.h"

static bool printSharedRegion = false;
static bool printFunctionStarts = false;
static bool printDataCode = false;


static void versionToString(uint32_t value, char buffer[32])
{
    if ( value == 0 )
        strcpy(buffer, "n/a");
    else if ( value & 0xFF )
        sprintf(buffer, "%d.%d.%d", value >> 16, (value >> 8) & 0xFF, value & 0xFF);
    else
        sprintf(buffer, "%d.%d", value >> 16, (value >> 8) & 0xFF);
}

static void printPlatforms(const dyld3::MachOAnalyzer* ma)
{
    printf("    -platform:\n");
    printf("        platform     minOS      sdk\n");
    ma->forEachSupportedPlatform(^(dyld3::Platform platform, uint32_t minOS, uint32_t sdk) {
        char osVers[32];
        char sdkVers[32];
        versionToString(minOS, osVers);
        versionToString(sdk,   sdkVers);
       printf(" %15s     %-7s   %-7s\n", dyld3::MachOFile::platformName(platform), osVers, sdkVers);
    });
}

static void printSegments(const dyld3::MachOAnalyzer* ma)
{
    printf("    -segments:\n");
    printf("        load-offset   segment section        sect-size  seg-size perm\n");
    __block const char* lastSegName = "";
    __block uint64_t    firstSegVmAddr = 0;
    ma->forEachSection(^(const dyld3::MachOFile::SectionInfo& sectInfo, bool malformedSectionRange, bool& stop) {
        if ( lastSegName[0] == '\0' )
            firstSegVmAddr = sectInfo.segInfo.vmAddr;
        if ( strcmp(lastSegName, sectInfo.segInfo.segName) != 0 ) {
            char r = (sectInfo.segInfo.protections & VM_PROT_READ)    ? 'r' : '.';
            char w = (sectInfo.segInfo.protections & VM_PROT_WRITE)   ? 'w' : '.';
            char x = (sectInfo.segInfo.protections & VM_PROT_EXECUTE) ? 'x' : '.';
            printf("        0x%08llX    %-12s                      %6lluKB %c%c%c\n", sectInfo.segInfo.vmAddr - firstSegVmAddr, sectInfo.segInfo.segName, sectInfo.segInfo.vmSize/1024, r, w, x);
            lastSegName = sectInfo.segInfo.segName;
        }
            printf("        0x%08llX             %-16s %6llu\n", sectInfo.sectAddr-firstSegVmAddr, sectInfo.sectName, sectInfo.sectSize);

    });

 }


static void printDependents(const dyld3::MachOAnalyzer* ma)
{
    printf("    -dependents:\n");
    printf("        attributes     load path\n");
    ma->forEachDependentDylib(^(const char* loadPath, bool isWeak, bool isReExport, bool isUpward, uint32_t compatVersion, uint32_t curVersion, bool &stop) {
        const char* attribute = "";
        if ( isWeak )
            attribute = "weak_import";
        else if ( isReExport )
            attribute = "re-export";
        else if ( isUpward )
            attribute = "upward";
        printf("        %-12s   %s\n", attribute, loadPath);
    });
}

static const char* rebaseTypeName(uint8_t type)
{
    switch (type ){
        case REBASE_TYPE_POINTER:
            return "rebase pointer";
        case REBASE_TYPE_TEXT_ABSOLUTE32:
            return "rebase text abs32";
        case REBASE_TYPE_TEXT_PCREL32:
            return "rebase text rel32";
    }
    return "!!unknown!!";
}

static const char* bindTypeName(uint8_t type)
{
    switch (type ){
        case BIND_TYPE_POINTER:
            return "bind pointer";
        case BIND_TYPE_TEXT_ABSOLUTE32:
            return "bind text abs32";
        case BIND_TYPE_TEXT_PCREL32:
            return "bind text rel32";
    }
    return "!!unknown!!";
}

static const char* pointerFormat(uint16_t format)
{
    switch (format) {
        case DYLD_CHAINED_PTR_ARM64E:
            return "authenticated arm64e";
        case DYLD_CHAINED_PTR_64:
            return "generic 64-bit";
        case DYLD_CHAINED_PTR_32:
            return "generic 32-bit";
        case DYLD_CHAINED_PTR_32_CACHE:
            return "32-bit for dyld cache";
    }
    return "unknown";
}

static void printChains(const dyld3::MachOAnalyzer* ma)
{
    Diagnostics diag;
    ma->withChainStarts(diag, 0, ^(const dyld_chained_starts_in_image* starts) {
        for (int i=0; i < starts->seg_count; ++i) {
            if ( starts->seg_info_offset[i] == 0 )
                continue;
            const dyld_chained_starts_in_segment* seg = (dyld_chained_starts_in_segment*)((uint8_t*)starts + starts->seg_info_offset[i]);
            if ( seg->page_count == 0 )
                continue;
            printf("seg[%d]:\n", i);
            printf("  page_size:       0x%04X\n",     seg->page_size);
            printf("  pointer_format:  %d (%s)\n",    seg->pointer_format, pointerFormat(seg->pointer_format));
            printf("  segment_offset:  0x%08llX\n",   seg->segment_offset);
            printf("  max_pointer:     0x%08X\n",     seg->max_valid_pointer);
            printf("  pages:         %d\n",           seg->page_count);
            for (int p=0; p < seg->page_count; ++p) {
                printf("    start[% 2d]:  0x%04X\n",   p, seg->page_start[p]);
            }
        }
   });
}

struct FixupInfo
{
    const char* segName;
    std::string sectName;
    uint64_t    address;
    const char* type;
    uint64_t    targetValue;
    const char* targetDylib;
    const char* targetSymbolName;
    uint64_t    targetAddend;
    bool        targetWeakImport;
};



static const char* ordinalName(const dyld3::MachOAnalyzer* ma, int libraryOrdinal)
{
    static int          sLastOrdinal = -100;
    static const char*  sLastString = nullptr;
    if ( libraryOrdinal > 0 ) {
        if ( libraryOrdinal != sLastOrdinal ) {
            const char* path = ma->dependentDylibLoadPath(libraryOrdinal-1);
            if ( path == nullptr )
                return "ordinal-too-large";
            const char* leafName = path;
            if ( const char* lastSlash = strrchr(path, '/') )
                leafName = lastSlash+1;
            sLastOrdinal = libraryOrdinal;
            sLastString = leafName;
        }
        return sLastString;
    }
    else {
        switch ( libraryOrdinal) {
            case BIND_SPECIAL_DYLIB_SELF:
                return "this-image";
            case BIND_SPECIAL_DYLIB_MAIN_EXECUTABLE:
                return "main-executable";
            case BIND_SPECIAL_DYLIB_FLAT_LOOKUP:
                return "flat-namespace";
            case BIND_SPECIAL_DYLIB_WEAK_LOOKUP:
                return "weak-coalesce";
        }
    }
    return "unknown-ordinal";
}


class SectionFinder
{
public:
                     SectionFinder(const dyld3::MachOAnalyzer* ma);
    const char*      segmentName(uint64_t vmOffset) const;
    const char*      sectionName(uint64_t vmOffset) const;
    uint64_t         baseAddress() const { return _baseAddress; }

private:
    void             updateLastSection(uint64_t vmOffset) const;

    const dyld3::MachOAnalyzer*           _ma;
    uint64_t                              _baseAddress;
    mutable dyld3::MachOFile::SectionInfo _lastSection;
    mutable char                          _lastSectName[20];
};

SectionFinder::SectionFinder(const dyld3::MachOAnalyzer* ma)
    : _ma(ma)
{
    _baseAddress = ma->preferredLoadAddress();
    _lastSection.sectAddr = 0;
    _lastSection.sectSize = 0;
}

void SectionFinder::updateLastSection(uint64_t vmOffset) const
{
    uint64_t vmAddr = _baseAddress + vmOffset;
    if ( (vmAddr < _lastSection.sectAddr) || (vmAddr >= _lastSection.sectAddr+_lastSection.sectSize) ) {
        _ma->forEachSection(^(const dyld3::MachOFile::SectionInfo& sectInfo, bool malformedSectionRange, bool& sectStop) {
            if ( (sectInfo.sectAddr <= vmAddr) && (vmAddr < sectInfo.sectAddr+sectInfo.sectSize) ) {
                _lastSection = sectInfo;
                strcpy(_lastSectName, _lastSection.sectName);
                sectStop = true;
            }
        });
    }
}

const char* SectionFinder::segmentName(uint64_t vmOffset) const
{
    updateLastSection(vmOffset);
    return _lastSection.segInfo.segName;
}

const char* SectionFinder::sectionName(uint64_t vmOffset) const
{
    updateLastSection(vmOffset);
    return _lastSectName;
}



static void printPreloadChainedFixups(const dyld3::MachOAnalyzer* ma)
{
    printf("        segment     section           address       type            (dvrsty   addr  key)  target\n");
    SectionFinder namer(ma);
    uint64_t sectionSize;
    const dyld_chained_starts_offsets* startsSection = (dyld_chained_starts_offsets*)ma->findSectionContent("__TEXT", "__chain_starts", sectionSize);
    if ( startsSection != nullptr ) {
        switch (startsSection->pointer_format) {
            case DYLD_CHAINED_PTR_32_FIRMWARE:
                for (uint32_t startIndex=0; startIndex < startsSection->starts_count; ++startIndex) {
                    const dyld_chained_ptr_32_firmware_rebase* p = (dyld_chained_ptr_32_firmware_rebase*)(((uint8_t*)ma)+ startsSection->chain_starts[startIndex]);
                    bool done = false;
                    while (!done) {
                        uint64_t vmOffset = ((uint8_t*)p - (uint8_t*)ma);
                        printf("        %-12s %-16s 0x%08llX  %16s                        0x%08X\n",
                                namer.segmentName(vmOffset), namer.sectionName(vmOffset), namer.baseAddress()+vmOffset,
                                "rebase pointer", p->target);
                        done = (p->next == 0);
                        p += p->next;
                    }
                }
        }

    }
}



struct FixupTarget
{
    uint64_t    value;
    const char* dylib;
    const char* symbolName;
    uint64_t    addend;
    bool        weakImport;
};


static void printChainedFixups(const dyld3::MachOAnalyzer* ma)
{
    // build array of targets
    __block Diagnostics diag;
    __block std::vector<FixupTarget> targets;
    ma->forEachChainedFixupTarget(diag, ^(int libOrdinal, const char* symbolName, uint64_t addend, bool weakImport, bool& stop) {
        FixupTarget target;
        target.value      = 0;
        target.dylib      = ordinalName(ma, libOrdinal);
        target.symbolName = symbolName;
        target.addend     = addend;
        target.weakImport = weakImport;
        targets.push_back(target);
    });
    if ( diag.hasError() )
        return;

    uint64_t baseAddress = ma->preferredLoadAddress();

    printf("        segment     section           address       type             (dvrsty  addr  key)  target\n");
    SectionFinder namer(ma);
    ma->withChainStarts(diag, 0, ^(const dyld_chained_starts_in_image* starts) {
        ma->forEachFixupInAllChains(diag, starts, false, ^(dyld3::MachOLoaded::ChainedFixupPointerOnDisk* fixupLoc, const dyld_chained_starts_in_segment* segInfo, bool& stop) {
            uint64_t vmOffset = (uint8_t*)fixupLoc - (uint8_t*)ma;
            switch (segInfo->pointer_format) {
                case DYLD_CHAINED_PTR_ARM64E:
                   if ( fixupLoc->arm64e.authRebase.auth ) {
                        if ( fixupLoc->arm64e.authBind.bind ) {
                            const FixupTarget& bindTarget = targets[fixupLoc->arm64e.authBind.ordinal];
                            if ( bindTarget.addend )
                                printf("        %-12s %-16s 0x%08llX  %16s  (0x%04X   %d     %s)   %s/%s + 0x%llX\n",
                                       namer.segmentName(vmOffset), namer.sectionName(vmOffset), vmOffset+namer.baseAddress(), "bind authptr",
                                       fixupLoc->arm64e.authBind.diversity, fixupLoc->arm64e.authBind.addrDiv,
                                       fixupLoc->arm64e.keyName(), bindTarget.dylib, bindTarget.symbolName, bindTarget.addend);
                            else
                                printf("        %-12s %-16s 0x%08llX  %16s  (0x%04X   %d     %s)   %s/%s\n",
                                       namer.segmentName(vmOffset), namer.sectionName(vmOffset), vmOffset+namer.baseAddress(), "bind authptr",
                                       fixupLoc->arm64e.authBind.diversity, fixupLoc->arm64e.authBind.addrDiv,
                                       fixupLoc->arm64e.keyName(), bindTarget.dylib, bindTarget.symbolName);
                         }
                        else {
                            uint64_t targetAddr = fixupLoc->arm64e.authRebase.target + baseAddress;
                            printf("        %-12s %-16s 0x%08llX  %16s  (0x%04X   %d     %s)   0x%08llX\n",
                                   namer.segmentName(vmOffset), namer.sectionName(vmOffset), vmOffset+namer.baseAddress(), "rebase authptr",
                                   fixupLoc->arm64e.authBind.diversity, fixupLoc->arm64e.authBind.addrDiv,
                                   fixupLoc->arm64e.keyName(), targetAddr);
                        }
                    }
                    else {
                        if ( fixupLoc->arm64e.rebase.bind ) {
                            const FixupTarget& bindTarget = targets[fixupLoc->arm64e.bind.ordinal];
                            uint64_t fullAddend = bindTarget.addend + fixupLoc->arm64e.signExtendedAddend();
                            if ( fullAddend )
                                printf("        %-12s %-16s 0x%08llX  %16s                        %s/%s + 0x%llX\n",
                                        namer.segmentName(vmOffset), namer.sectionName(vmOffset), vmOffset+namer.baseAddress(),
                                        "bind pointer", bindTarget.dylib, bindTarget.symbolName, fullAddend);
                            else
                                printf("        %-12s %-16s 0x%08llX  %16s                        %s/%s\n",
                                        namer.segmentName(vmOffset), namer.sectionName(vmOffset), vmOffset+namer.baseAddress(),
                                        "bind pointer", bindTarget.dylib, bindTarget.symbolName);
                        }
                        else {
                            uint64_t targetAddr = fixupLoc->arm64e.unpackTarget();
                            printf("        %-12s %-16s 0x%08llX  %16s                        0x%08llX\n",
                                    namer.segmentName(vmOffset), namer.sectionName(vmOffset), vmOffset+namer.baseAddress(),
                                    "rebase pointer", targetAddr);
                        }
                    }
                    break;
                case DYLD_CHAINED_PTR_64:
                    if ( fixupLoc->generic64.rebase.bind ) {
                        const FixupTarget& bindTarget = targets[fixupLoc->generic64.bind.ordinal];
                        uint64_t fullAddend = bindTarget.addend + fixupLoc->generic64.signExtendedAddend();
                        if ( fullAddend )
                            printf("        %-12s %-16s 0x%08llX  %16s                        %s/%s + 0x%llX\n",
                                   namer.segmentName(vmOffset), namer.sectionName(vmOffset), vmOffset+namer.baseAddress(),
                                    "bind pointer", bindTarget.dylib, bindTarget.symbolName, fullAddend);
                        else
                            printf("        %-12s %-16s 0x%08llX  %16s                        %s/%s\n",
                                    namer.segmentName(vmOffset), namer.sectionName(vmOffset), vmOffset+namer.baseAddress(),
                                    "bind pointer", bindTarget.dylib, bindTarget.symbolName);
                    }
                    else {
                        uint64_t targetAddr = fixupLoc->generic64.unpackedTarget();
                        printf("        %-12s %-16s 0x%08llX  %16s                        0x%08llX\n",
                                namer.segmentName(vmOffset), namer.sectionName(vmOffset), vmOffset+namer.baseAddress(),
                                "rebase pointer", targetAddr);
                    }
                    break;
                case DYLD_CHAINED_PTR_32:
                    if ( fixupLoc->generic32.rebase.bind ) {
                        const FixupTarget& bindTarget = targets[fixupLoc->generic32.bind.ordinal];
                        uint32_t fullAddend = (uint32_t)bindTarget.addend + fixupLoc->generic32.bind.addend;
                        if ( fullAddend )
                            printf("        %-12s %-16s 0x%08llX  %16s                        %s/%s + 0x%X\n",
                                    namer.segmentName(vmOffset), namer.sectionName(vmOffset), vmOffset+namer.baseAddress(),
                                    "bind pointer", bindTarget.dylib, bindTarget.symbolName, fullAddend);
                        else
                            printf("        %-12s %-16s 0x%08llX  %16s                        %s/%s\n",
                                    namer.segmentName(vmOffset), namer.sectionName(vmOffset), vmOffset+namer.baseAddress(),
                                    "bind pointer", bindTarget.dylib, bindTarget.symbolName);
                    }
                    else {
                        uint32_t targetAddr = fixupLoc->generic32.rebase.target;
                        printf("        %-12s %-16s 0x%08llX  %16s                        0x%08X\n",
                                namer.segmentName(vmOffset), namer.sectionName(vmOffset), vmOffset+namer.baseAddress(),
                                "rebase pointer", targetAddr);
                    }
                    break;
                default:
                    fprintf(stderr, "unknown pointer type %d\n", segInfo->pointer_format);
                    break;
            }
         });
    });
}

static void printOpcodeFixups(const dyld3::MachOAnalyzer* ma)
{
    Diagnostics diag;
    __block std::vector<FixupInfo> fixups;
    SectionFinder namer(ma);
    ma->forEachRebase(diag, ^(const char* opcodeName, const dyld3::MachOLoaded::LinkEditInfo& leInfo, const dyld3::MachOFile::SegmentInfo segments[],
                              bool segIndexSet, uint32_t pointerSize, uint8_t segIndex, uint64_t segOffset, uint8_t type, bool& stop) {
        const dyld3::MachOFile::SegmentInfo& segment = segments[segIndex];
        uint64_t locVmAddr = segment.vmAddr + segOffset;
        uint64_t runtimeOffset = locVmAddr - namer.baseAddress();
        const uint8_t* loc = ((uint8_t*)ma + runtimeOffset);
        uint64_t value = (pointerSize == 8) ? *((uint64_t*)(loc)) : *((uint32_t*)(loc));
        FixupInfo fixup;
        fixup.segName           = namer.segmentName(runtimeOffset);
        fixup.sectName          = namer.sectionName(runtimeOffset);
        fixup.address           = locVmAddr;
        fixup.type              = rebaseTypeName(type);
        fixup.targetValue       = value;
        fixup.targetDylib       = nullptr;
        fixup.targetSymbolName  = nullptr;
        fixup.targetAddend      = 0;
        fixup.targetWeakImport  = false;
        fixups.push_back(fixup);
    });

    ma->forEachBind(diag, ^(const char* opcodeName, const dyld3::MachOLoaded::LinkEditInfo& leInfo, const dyld3::MachOFile::SegmentInfo segments[],
                            bool segIndexSet,  bool libraryOrdinalSet, uint32_t dylibCount, int libOrdinal,
                            uint32_t pointerSize, uint8_t segIndex, uint64_t segOffset,
                            uint8_t type, const char* symbolName, bool weakImport, bool lazyBind, uint64_t addend, bool& stop) {
        const dyld3::MachOFile::SegmentInfo& segment = segments[segIndex];
        uint64_t locVmAddr = segment.vmAddr + segOffset;
        uint64_t runtimeOffset = locVmAddr - namer.baseAddress();
        FixupInfo fixup;
        fixup.segName           = namer.segmentName(runtimeOffset);
        fixup.sectName          = namer.sectionName(runtimeOffset);
        fixup.address           = locVmAddr;
        fixup.type              = bindTypeName(type);
        fixup.targetValue       = 0;
        fixup.targetDylib       = ordinalName(ma, libOrdinal);
        fixup.targetSymbolName  = symbolName;
        fixup.targetAddend      = addend;
        fixup.targetWeakImport  = weakImport;
        fixups.push_back(fixup);
    },^(const char* symbolName) {
    },^() { });


    std::sort(fixups.begin(), fixups.end(), [](const FixupInfo& l, const FixupInfo& r) {
        if ( &l == &r )
            return false;
        if ( l.address == r.address )
            return (l.targetSymbolName == nullptr);
        return ( l.address < r.address );
    });

    printf("        segment      section          address         type          target\n");
    for (const FixupInfo& fixup : fixups) {
        if ( fixup.targetSymbolName == nullptr )
            printf("        %-12s %-16s 0x%08llX  %16s  0x%08llX\n", fixup.segName, fixup.sectName.c_str(), fixup.address, fixup.type, fixup.targetValue);
        else if ( fixup.targetAddend != 0 )
            printf("        %-12s %-16s 0x%08llX  %16s  %s/%s + 0x%llX\n", fixup.segName, fixup.sectName.c_str(), fixup.address, fixup.type, fixup.targetDylib, fixup.targetSymbolName, fixup.targetAddend);
        else if ( fixup.targetWeakImport )
            printf("        %-12s %-16s 0x%08llX  %16s  %s/%s [weak-import]\n", fixup.segName, fixup.sectName.c_str(), fixup.address, fixup.type, fixup.targetDylib, fixup.targetSymbolName);
        else
            printf("        %-12s %-16s 0x%08llX  %16s  %s/%s\n", fixup.segName, fixup.sectName.c_str(), fixup.address, fixup.type, fixup.targetDylib, fixup.targetSymbolName);
   }


}


static void printFixups(const dyld3::MachOAnalyzer* ma)
{
    printf("    -fixups:\n");
    if ( ma->isPreload() || ma->isStaticExecutable() ) {
        printPreloadChainedFixups(ma);
    }
    else if ( ma->hasChainedFixups() ) {
        printChainedFixups(ma);
    }
    else {
        printOpcodeFixups(ma);
    }
 }


static void printExports(const dyld3::MachOAnalyzer* ma)
{
    printf("    -exports:\n");
    printf("        offset      symbol\n");
    Diagnostics diag;
    ma->forEachExportedSymbol(diag, ^(const char* symbolName, uint64_t imageOffset, uint64_t flags, uint64_t other, const char* importName, bool& stop) {
        //printf("0x%08llX %s\n", imageOffset, symbolName);
        const bool reExport     = (flags & EXPORT_SYMBOL_FLAGS_REEXPORT);
        const bool weakDef      = (flags & EXPORT_SYMBOL_FLAGS_WEAK_DEFINITION);
        const bool resolver     = (flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER);
        const bool threadLocal  = ((flags & EXPORT_SYMBOL_FLAGS_KIND_MASK) == EXPORT_SYMBOL_FLAGS_KIND_THREAD_LOCAL);
        const bool abs          = ((flags & EXPORT_SYMBOL_FLAGS_KIND_MASK) == EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE);
        if ( reExport )
            printf("        [re-export] ");
        else
            printf("        0x%08llX  ", imageOffset);
        printf("%s", symbolName);
        if ( weakDef || threadLocal || resolver || abs ) {
            bool needComma = false;
            printf(" [");
            if ( weakDef ) {
                printf("weak_def");
                needComma = true;
            }
            if ( threadLocal ) {
                if ( needComma )
                    printf(", ");
                printf("per-thread");
                needComma = true;
            }
            if ( abs ) {
                if ( needComma )
                    printf(", ");
                printf("absolute");
                needComma = true;
            }
            if ( resolver ) {
                if ( needComma )
                    printf(", ");
                printf("resolver=0x%08llX", other);
                needComma = true;
            }
            printf("]");
        }
        if ( reExport ) {
            if ( importName[0] == '\0' )
                printf(" (from %s)", ordinalName(ma, (int)other));
            else
                printf(" (%s from %s)", importName, ordinalName(ma, (int)other));
        }
        printf("\n");
    });

}

static void printObjC(const dyld3::MachOAnalyzer* ma)
{
    Diagnostics diag;
    const bool contentRebased = false;
    const uint32_t pointerSize = ma->pointerSize();

    auto printMethod = ^(uint64_t methodVMAddr, const dyld3::MachOAnalyzer::ObjCMethod& method) {
        const char* type = "method";
        dyld3::MachOAnalyzer::PrintableStringResult methodNameResult;
        const char* methodName = ma->getPrintableString(method.nameVMAddr, methodNameResult);
        switch (methodNameResult) {
            case dyld3::MachOAnalyzer::PrintableStringResult::CanPrint:
                // The string is already valid
                break;
            case dyld3::MachOAnalyzer::PrintableStringResult::FairPlayEncrypted:
                methodName = "### fairplay encrypted";
                break;
            case dyld3::MachOAnalyzer::PrintableStringResult::ProtectedSection:
                methodName = "### protected section";
                break;
            case dyld3::MachOAnalyzer::PrintableStringResult::UnknownSection:
                methodName = "### unknown section";
                break;
        }
        printf("        %10s   0x%08llX                 %s\n",
               type, methodVMAddr, methodName);
    };

    printf("    -objc:\n");
    printf("              type       vmaddr   data-vmaddr   name\n");
    auto printClass = ^(Diagnostics& diag, uint64_t classVMAddr,
                        uint64_t classSuperclassVMAddr, uint64_t classDataVMAddr,
                        const dyld3::MachOAnalyzer::ObjCClassInfo& objcClass, bool isMetaClass) {
        const char* type = "class";
        if (isMetaClass)
            type = "meta-class";
        dyld3::MachOAnalyzer::PrintableStringResult classNameResult;
        const char* className = ma->getPrintableString(objcClass.nameVMAddr(pointerSize), classNameResult);
        switch (classNameResult) {
            case dyld3::MachOAnalyzer::PrintableStringResult::CanPrint:
                // The string is already valid
                break;
            case dyld3::MachOAnalyzer::PrintableStringResult::FairPlayEncrypted:
                className = "### fairplay encrypted";
                break;
            case dyld3::MachOAnalyzer::PrintableStringResult::ProtectedSection:
                className = "### protected section";
                break;
            case dyld3::MachOAnalyzer::PrintableStringResult::UnknownSection:
                className = "### unknown section";
                break;
        }
        printf("        %10s   0x%08llX    0x%08llX   %s\n",
               type, classVMAddr, objcClass.dataVMAddr, className);

        // Now print the methods on this class
        ma->forEachObjCMethod(objcClass.baseMethodsVMAddr(pointerSize), contentRebased,
                              printMethod);
    };
    auto printCategory = ^(Diagnostics& diag, uint64_t categoryVMAddr,
                           const dyld3::MachOAnalyzer::ObjCCategory& objcCategory) {
        const char* type = "category";
        dyld3::MachOAnalyzer::PrintableStringResult categoryNameResult;
        const char* categoryName = ma->getPrintableString(objcCategory.nameVMAddr, categoryNameResult);
        switch (categoryNameResult) {
            case dyld3::MachOAnalyzer::PrintableStringResult::CanPrint:
                // The string is already valid
                break;
            case dyld3::MachOAnalyzer::PrintableStringResult::FairPlayEncrypted:
                categoryName = "### fairplay encrypted";
                break;
            case dyld3::MachOAnalyzer::PrintableStringResult::ProtectedSection:
                categoryName = "### protected section";
                break;
            case dyld3::MachOAnalyzer::PrintableStringResult::UnknownSection:
                categoryName = "### unknown section";
                break;
        }
        printf("        %10s   0x%08llX                 %s\n",
               type, categoryVMAddr, categoryName);

        // Now print the methods on this category
        ma->forEachObjCMethod(objcCategory.instanceMethodsVMAddr, contentRebased,
                              printMethod);
        ma->forEachObjCMethod(objcCategory.classMethodsVMAddr, contentRebased,
                              printMethod);
    };
    auto printProtocol = ^(Diagnostics& diag, uint64_t protocolVMAddr,
                           const dyld3::MachOAnalyzer::ObjCProtocol& objCProtocol) {
        const char* type = "protocol";
        dyld3::MachOAnalyzer::PrintableStringResult protocolNameResult;
        const char* protocolName = ma->getPrintableString(objCProtocol.nameVMAddr, protocolNameResult);
        switch (protocolNameResult) {
            case dyld3::MachOAnalyzer::PrintableStringResult::CanPrint:
                // The string is already valid
                break;
            case dyld3::MachOAnalyzer::PrintableStringResult::FairPlayEncrypted:
                protocolName = "### fairplay encrypted";
                break;
            case dyld3::MachOAnalyzer::PrintableStringResult::ProtectedSection:
                protocolName = "### protected section";
                break;
            case dyld3::MachOAnalyzer::PrintableStringResult::UnknownSection:
                protocolName = "### unknown section";
                break;
        }
        printf("        %10s   0x%08llX                 %s\n",
               type, protocolVMAddr, protocolName);

        // Now print the methods on this protocol
        ma->forEachObjCMethod(objCProtocol.instanceMethodsVMAddr, contentRebased,
                              printMethod);
        ma->forEachObjCMethod(objCProtocol.classMethodsVMAddr, contentRebased,
                              printMethod);
        ma->forEachObjCMethod(objCProtocol.optionalInstanceMethodsVMAddr, contentRebased,
                              printMethod);
        ma->forEachObjCMethod(objCProtocol.optionalClassMethodsVMAddr, contentRebased,
                              printMethod);
    };
    ma->forEachObjCClass(diag, contentRebased, printClass);
    ma->forEachObjCCategory(diag, contentRebased, printCategory);
    ma->forEachObjCProtocol(diag, contentRebased, printProtocol);
}

static void usage()
{
	fprintf(stderr, "Usage: dyldinfo [-arch <arch>]* <options>* <mach-o file>+\n"
            "\t-segments         print segments\n"
			"\t-dependents       print dependent dylibs\n"
            "\t-fixups           print locations dyld will rebase/bind\n"
            "\t-exports          print addresses of all symbols this file exports\n"
            "\t-objc             print objc classes, categories, etc\n"
		);
}

static bool inStringVector(const std::vector<const char*>& vect, const char* target)
{
    for (const char* str : vect) {
        if ( strcmp(str, target) == 0 )
            return true;
    }
    return false;
}


int main(int argc, const char* argv[])
{
	if ( argc == 1 ) {
		usage();
		return 0;
	}

    std::vector<const char*> files;
    std::vector<const char*> cmdLineArchs;
    for (int i=1; i < argc; ++i) {
        const char* arg = argv[i];
        if ( arg[0] == '-' ) {
            if ( strcmp(arg, "-arch") == 0 ) {
                if ( ++i < argc ) {
                    cmdLineArchs.push_back(argv[i]);
                }
                else {
                    fprintf(stderr, "-arch missing architecture name");
                    return 1;
                }
            }
        }
        else {
            files.push_back(arg);
        }
    }
    if ( files.size() == 0 ) {
        usage();
        return 0;
    }

    for (const char* path : files) {
        Diagnostics diag;
        dyld3::closure::FileSystemPhysical fileSystem;
        dyld3::closure::LoadedFileInfo info;
        char realerPath[MAXPATHLEN];
        __block bool printedError = false;
        if (!fileSystem.loadFile(path, info, realerPath, ^(const char* format, ...) {
            fprintf(stderr, "dyldinfo: ");
            va_list list;
            va_start(list, format);
            vfprintf(stderr, format, list);
            va_end(list);
            printedError = true;
        })) {
            if (!printedError )
                fprintf(stderr, "dyldinfo: %s: file not found\n", path);
            return 1;
        }
        __block std::vector<const char*> archesForFile;
        __block dyld3::Platform platform = dyld3::Platform::unknown;
        if ( dyld3::FatFile::isFatFile(info.fileContent) ) {
            const dyld3::FatFile* ff = (dyld3::FatFile*)info.fileContent;
            ff->forEachSlice(diag, info.fileContentLen, ^(uint32_t sliceCpuType, uint32_t sliceCpuSubType, const void* sliceStart, uint64_t sliceSize, bool& stop) {
                const char* sliceArchName = dyld3::MachOFile::archName(sliceCpuType, sliceCpuSubType);
                if ( cmdLineArchs.empty() || inStringVector(cmdLineArchs, sliceArchName) ) {
                    archesForFile.push_back(sliceArchName);
                    const dyld3::MachOFile* mf = (dyld3::MachOFile*)sliceStart;
                    mf->forEachSupportedPlatform(^(dyld3::Platform plat, uint32_t minOS, uint32_t sdk) {
                        if ( platform == dyld3::Platform::unknown)
                            platform = plat;
                    });
                }
             });
        }
        else {
            const dyld3::MachOFile* mo = (dyld3::MachOFile*)info.fileContent;
            if ( mo->isMachO(diag, info.sliceLen) ) {
                archesForFile.push_back(mo->archName());
                mo->forEachSupportedPlatform(^(dyld3::Platform plat, uint32_t minOS, uint32_t sdk) {
                    if ( platform == dyld3::Platform::unknown)
                        platform = plat;
                });
            }
            else {
                fprintf(stderr, "dyldinfo: %s: %s\n", path, diag.errorMessage());
                return 1;
            }
        }
        if ( archesForFile.empty() ) {
            fprintf(stderr, "dyldinfo: %s: does not contain specified arch(s)\n", path);
            return 1;
        }
        char loadedPath[MAXPATHLEN];
        for (const char* sliceArch : archesForFile) {
            info = dyld3::MachOAnalyzer::load(diag, fileSystem, path, dyld3::GradedArchs::forName(sliceArch), platform, loadedPath);
            if ( diag.hasError() ) {
                fprintf(stderr, "dyldinfo: %s\n", diag.errorMessage());
                return 1;
            }
            const dyld3::MachOAnalyzer* ma = (dyld3::MachOAnalyzer*)info.fileContent;
            printf("%s [%s]:\n", path, sliceArch);

            bool somethingPrinted = false;
            for (int i=1; i < argc; ++i) {
                const char* arg = argv[i];
                if ( arg[0] != '-' )
                    continue;
                if ( strcmp(arg, "-arch") == 0 ) {
                    // handled previously
                    ++i;
                }
                else if ( strcmp(arg, "-platform") == 0 ) {
                    printPlatforms(ma);
                    somethingPrinted = true;
                }
                else if ( strcmp(arg, "-segments") == 0 ) {
                    printSegments(ma);
                    somethingPrinted = true;
                }
                else if ( strcmp(arg, "-dependents") == 0 ) {
                    printDependents(ma);
                     somethingPrinted = true;
                }
                else if ( strcmp(arg, "-fixups") == 0 ) {
                    printFixups(ma);
                    somethingPrinted = true;
                }
                else if ( strcmp(arg, "-exports") == 0 ) {
                    printExports(ma);
                     somethingPrinted = true;
                }
                else if ( strcmp(arg, "-fixup_chains") == 0 ) {
                    printChains(ma);
                    somethingPrinted = true;
                }
                else if ( strcmp(arg, "-opcodes") == 0 ) {
                }
                else if ( strcmp(arg, "-shared_region") == 0 ) {
                }
                else if ( strcmp(arg, "-function_starts") == 0 ) {
                }
                else if ( strcmp(arg, "-data_in_code") == 0 ) {
                }
                else if ( strcmp(arg, "-objc") == 0 ) {
                    printObjC(ma);
                    somethingPrinted = true;
                }
                else {
                    fprintf(stderr, "unknown option: %s\n", arg);
                    return 1;
                }
            }
            if ( !somethingPrinted ) {
                printPlatforms(ma);
                printSegments(ma);
                printDependents(ma);
                printFixups(ma);
                printExports(ma);
                printObjC(ma);
            }

        }
    }

	return 0;
}




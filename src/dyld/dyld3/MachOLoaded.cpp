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


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <mach/mach.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <mach-o/reloc.h>
#include <mach-o/nlist.h>
extern "C" {
  #include <corecrypto/ccdigest.h>
  #include <corecrypto/ccsha1.h>
  #include <corecrypto/ccsha2.h>
}

#include "MachOFile.h"
#include "MachOLoaded.h"
#include "CodeSigningTypes.h"



namespace dyld3 {


void MachOLoaded::getLinkEditLoadCommands(Diagnostics& diag, LinkEditInfo& result) const
{
    result.dyldInfo       = nullptr;
    result.exportsTrie    = nullptr;
    result.chainedFixups  = nullptr;
    result.symTab         = nullptr;
    result.dynSymTab      = nullptr;
    result.splitSegInfo   = nullptr;
    result.functionStarts = nullptr;
    result.dataInCode     = nullptr;
    result.codeSig        = nullptr;
    __block bool hasUUID    = false;
    __block bool hasMinVersion = false;
    __block bool hasEncrypt = false;
    forEachLoadCommand(diag, ^(const load_command* cmd, bool& stop) {
        switch ( cmd->cmd ) {
            case LC_DYLD_INFO:
            case LC_DYLD_INFO_ONLY:
                if ( cmd->cmdsize != sizeof(dyld_info_command) )
                    diag.error("LC_DYLD_INFO load command size wrong");
                else if ( result.dyldInfo != nullptr )
                    diag.error("multiple LC_DYLD_INFO load commands");
                result.dyldInfo = (dyld_info_command*)cmd;
                break;
            case LC_DYLD_EXPORTS_TRIE:
                if ( cmd->cmdsize != sizeof(linkedit_data_command) )
                    diag.error("LC_DYLD_EXPORTS_TRIE load command size wrong");
                else if ( result.exportsTrie != nullptr )
                    diag.error("multiple LC_DYLD_EXPORTS_TRIE load commands");
                result.exportsTrie = (linkedit_data_command*)cmd;
                break;
            case LC_DYLD_CHAINED_FIXUPS:
                if ( cmd->cmdsize != sizeof(linkedit_data_command) )
                    diag.error("LC_DYLD_CHAINED_FIXUPS load command size wrong");
                else if ( result.chainedFixups != nullptr )
                    diag.error("multiple LC_DYLD_CHAINED_FIXUPS load commands");
                result.chainedFixups = (linkedit_data_command*)cmd;
                break;
            case LC_SYMTAB:
                if ( cmd->cmdsize != sizeof(symtab_command) )
                    diag.error("LC_SYMTAB load command size wrong");
                else if ( result.symTab != nullptr )
                    diag.error("multiple LC_SYMTAB load commands");
                result.symTab = (symtab_command*)cmd;
                break;
            case LC_DYSYMTAB:
                if ( cmd->cmdsize != sizeof(dysymtab_command) )
                    diag.error("LC_DYSYMTAB load command size wrong");
                else if ( result.dynSymTab != nullptr )
                    diag.error("multiple LC_DYSYMTAB load commands");
                result.dynSymTab = (dysymtab_command*)cmd;
                break;
            case LC_SEGMENT_SPLIT_INFO:
                if ( cmd->cmdsize != sizeof(linkedit_data_command) )
                    diag.error("LC_SEGMENT_SPLIT_INFO load command size wrong");
                else if ( result.splitSegInfo != nullptr )
                    diag.error("multiple LC_SEGMENT_SPLIT_INFO load commands");
                result.splitSegInfo = (linkedit_data_command*)cmd;
                break;
            case LC_FUNCTION_STARTS:
                if ( cmd->cmdsize != sizeof(linkedit_data_command) )
                    diag.error("LC_FUNCTION_STARTS load command size wrong");
                else if ( result.functionStarts != nullptr )
                    diag.error("multiple LC_FUNCTION_STARTS load commands");
                result.functionStarts = (linkedit_data_command*)cmd;
                break;
            case LC_DATA_IN_CODE:
                if ( cmd->cmdsize != sizeof(linkedit_data_command) )
                    diag.error("LC_DATA_IN_CODE load command size wrong");
                else if ( result.dataInCode != nullptr )
                    diag.error("multiple LC_DATA_IN_CODE load commands");
                result.dataInCode = (linkedit_data_command*)cmd;
                break;
            case LC_CODE_SIGNATURE:
                if ( cmd->cmdsize != sizeof(linkedit_data_command) )
                    diag.error("LC_CODE_SIGNATURE load command size wrong");
                else if ( result.codeSig != nullptr )
                     diag.error("multiple LC_CODE_SIGNATURE load commands");
                result.codeSig = (linkedit_data_command*)cmd;
                break;
            case LC_UUID:
                if ( cmd->cmdsize != sizeof(uuid_command) )
                    diag.error("LC_UUID load command size wrong");
                else if ( hasUUID )
                     diag.error("multiple LC_UUID load commands");
                hasUUID = true;
                break;
            case LC_VERSION_MIN_IPHONEOS:
            case LC_VERSION_MIN_MACOSX:
            case LC_VERSION_MIN_TVOS:
            case LC_VERSION_MIN_WATCHOS:
                if ( cmd->cmdsize != sizeof(version_min_command) )
                    diag.error("LC_VERSION_* load command size wrong");
                 else if ( hasMinVersion )
                     diag.error("multiple LC_VERSION_MIN_* load commands");
                hasMinVersion = true;
                break;
            case LC_BUILD_VERSION:
                if ( cmd->cmdsize != (sizeof(build_version_command) + ((build_version_command*)cmd)->ntools * sizeof(build_tool_version)) )
                    diag.error("LC_BUILD_VERSION load command size wrong");
                break;
            case LC_ENCRYPTION_INFO:
                if ( cmd->cmdsize != sizeof(encryption_info_command) )
                    diag.error("LC_ENCRYPTION_INFO load command size wrong");
                else if ( hasEncrypt )
                    diag.error("multiple LC_ENCRYPTION_INFO load commands");
                else if ( is64() )
                    diag.error("LC_ENCRYPTION_INFO found in 64-bit mach-o");
                hasEncrypt = true;
                break;
            case LC_ENCRYPTION_INFO_64:
                if ( cmd->cmdsize != sizeof(encryption_info_command_64) )
                    diag.error("LC_ENCRYPTION_INFO_64 load command size wrong");
                else if ( hasEncrypt )
                     diag.error("multiple LC_ENCRYPTION_INFO_64 load commands");
                else if ( !is64() )
                      diag.error("LC_ENCRYPTION_INFO_64 found in 32-bit mach-o");
                hasEncrypt = true;
                break;
        }
    });
    if ( diag.noError() && (result.dynSymTab != nullptr) && (result.symTab == nullptr) )
        diag.error("LC_DYSYMTAB but no LC_SYMTAB load command");
}

void MachOLoaded::getLinkEditPointers(Diagnostics& diag, LinkEditInfo& result) const
{
    getLinkEditLoadCommands(diag, result);
    if ( diag.noError() )
        getLayoutInfo(result.layout);
}

const uint8_t* MachOLoaded::getExportsTrie(const LinkEditInfo& leInfo, uint64_t& trieSize) const
{
    if ( leInfo.exportsTrie != nullptr) {
        trieSize = leInfo.exportsTrie->datasize;
        uint64_t offsetInLinkEdit = leInfo.exportsTrie->dataoff - leInfo.layout.linkeditFileOffset;
        return (uint8_t*)this + (leInfo.layout.linkeditUnslidVMAddr - leInfo.layout.textUnslidVMAddr) + offsetInLinkEdit;
    }
    else if ( leInfo.dyldInfo != nullptr ) {
        trieSize = leInfo.dyldInfo->export_size;
        uint64_t offsetInLinkEdit = leInfo.dyldInfo->export_off - leInfo.layout.linkeditFileOffset;
        return (uint8_t*)this + (leInfo.layout.linkeditUnslidVMAddr - leInfo.layout.textUnslidVMAddr) + offsetInLinkEdit;
    }
    trieSize = 0;
    return nullptr;
}


void MachOLoaded::getLayoutInfo(LayoutInfo& result) const
{
    forEachSegment(^(const SegmentInfo& info, bool& stop) {
        if ( strcmp(info.segName, "__TEXT") == 0 ) {
            result.textUnslidVMAddr = (uintptr_t)info.vmAddr;
            result.slide = (uintptr_t)(((uint64_t)this) - info.vmAddr);
        }
        else if ( strcmp(info.segName, "__LINKEDIT") == 0 ) {
            result.linkeditUnslidVMAddr = (uintptr_t)info.vmAddr;
            result.linkeditFileOffset   = (uint32_t)info.fileOffset;
            result.linkeditFileSize     = (uint32_t)info.fileSize;
            result.linkeditSegIndex     = info.segIndex;
        }
    });
}

bool MachOLoaded::hasExportTrie(uint32_t& runtimeOffset, uint32_t& size) const
{
    runtimeOffset = 0;
    size = 0;
    Diagnostics diag;
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    diag.assertNoError();   // any malformations in the file should have been caught by earlier validate() call
    if ( diag.hasError() )
        return false;
    uint64_t trieSize;
    if ( const uint8_t* trie = getExportsTrie(leInfo, trieSize) ) {
        runtimeOffset = (uint32_t)(trie - (uint8_t*)this);
        size = (uint32_t)trieSize;
        return true;
    }
    return false;
}


#if BUILDING_LIBDYLD
// this is only used by dlsym() at runtime.  All other binding is done when the closure is built.
bool MachOLoaded::hasExportedSymbol(const char* symbolName, DependentToMachOLoaded finder, void** result,
                                    bool* resultPointsToInstructions) const
{
    typedef void* (*ResolverFunc)(void);
    ResolverFunc resolver;
    Diagnostics diag;
    FoundSymbol foundInfo;
    if ( findExportedSymbol(diag, symbolName, false, foundInfo, finder) ) {
        switch ( foundInfo.kind ) {
            case FoundSymbol::Kind::headerOffset: {
                *result = (uint8_t*)foundInfo.foundInDylib + foundInfo.value;
                *resultPointsToInstructions = false;
                int64_t slide = foundInfo.foundInDylib->getSlide();
                foundInfo.foundInDylib->forEachSection(^(const SectionInfo& sectInfo, bool malformedSectionRange, bool& stop) {
                    uint64_t sectStartAddr = sectInfo.sectAddr + slide;
                    uint64_t sectEndAddr = sectStartAddr + sectInfo.sectSize;
                    if ( ((uint64_t)*result >= sectStartAddr) && ((uint64_t)*result < sectEndAddr) ) {
                        *resultPointsToInstructions = (sectInfo.sectFlags & S_ATTR_PURE_INSTRUCTIONS) || (sectInfo.sectFlags & S_ATTR_SOME_INSTRUCTIONS);
                        stop = true;
                    }
                });
                break;
            }
            case FoundSymbol::Kind::absolute:
                *result = (void*)(long)foundInfo.value;
                *resultPointsToInstructions = false;
                break;
            case FoundSymbol::Kind::resolverOffset:
                // foundInfo.value contains "stub".
                // in dlsym() we want to call resolver function to get final function address
                resolver = (ResolverFunc)((uint8_t*)foundInfo.foundInDylib + foundInfo.resolverFuncOffset);
                *result = (*resolver)();
                // FIXME: Set this properly
                *resultPointsToInstructions = true;
                break;
        }
        return true;
    }
    return false;
}
#endif // BUILDING_LIBDYLD

bool MachOLoaded::findExportedSymbol(Diagnostics& diag, const char* symbolName, bool weakImport, FoundSymbol& foundInfo, DependentToMachOLoaded findDependent) const
{
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() )
        return false;
    uint64_t trieSize;
    if ( const uint8_t* trieStart = getExportsTrie(leInfo, trieSize) ) {
        const uint8_t* trieEnd   = trieStart + trieSize;
        const uint8_t* node      = trieWalk(diag, trieStart, trieEnd, symbolName);
        if ( node == nullptr ) {
            // symbol not exported from this image. Seach any re-exported dylibs
            __block unsigned        depIndex = 0;
            __block bool            foundInReExportedDylib = false;
            forEachDependentDylib(^(const char* loadPath, bool isWeak, bool isReExport, bool isUpward, uint32_t compatVersion, uint32_t curVersion, bool& stop) {
                if ( isReExport && findDependent ) {
                    if ( const MachOLoaded* depMH = findDependent(this, depIndex) ) {
                       if ( depMH->findExportedSymbol(diag, symbolName, weakImport, foundInfo, findDependent) ) {
                            stop = true;
                            foundInReExportedDylib = true;
                        }
                    }
                }
                ++depIndex;
            });
            return foundInReExportedDylib;
        }
        const uint8_t* p = node;
        const uint64_t flags = read_uleb128(diag, p, trieEnd);
        if ( flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
            if ( !findDependent )
                return false;
            // re-export from another dylib, lookup there
            const uint64_t ordinal = read_uleb128(diag, p, trieEnd);
            const char* importedName = (char*)p;
            if ( importedName[0] == '\0' )
                importedName = symbolName;
            if ( (ordinal == 0) || (ordinal > dependentDylibCount()) ) {
                diag.error("re-export ordinal %lld out of range for %s", ordinal, symbolName);
                return false;
            }
            uint32_t depIndex = (uint32_t)(ordinal-1);
            if ( const MachOLoaded* depMH = findDependent(this, depIndex) ) {
                return depMH->findExportedSymbol(diag, importedName, weakImport, foundInfo, findDependent);
            }
            else if (weakImport) {
                return false;
            }
            else {
                diag.error("dependent dylib %lld not found for re-exported symbol %s", ordinal, symbolName);
                return false;
            }
        }
        foundInfo.kind               = FoundSymbol::Kind::headerOffset;
        foundInfo.isThreadLocal      = false;
        foundInfo.isWeakDef          = false;
        foundInfo.foundInDylib       = this;
        foundInfo.value              = read_uleb128(diag, p, trieEnd);
        foundInfo.resolverFuncOffset = 0;
        foundInfo.foundSymbolName    = symbolName;
        if ( diag.hasError() )
            return false;
        switch ( flags & EXPORT_SYMBOL_FLAGS_KIND_MASK ) {
            case EXPORT_SYMBOL_FLAGS_KIND_REGULAR:
                if ( flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER ) {
                    foundInfo.kind = FoundSymbol::Kind::headerOffset;
                    foundInfo.resolverFuncOffset = (uint32_t)read_uleb128(diag, p, trieEnd);
                }
                else {
                    foundInfo.kind = FoundSymbol::Kind::headerOffset;
                }
                if ( flags & EXPORT_SYMBOL_FLAGS_WEAK_DEFINITION )
                    foundInfo.isWeakDef = true;
                break;
            case EXPORT_SYMBOL_FLAGS_KIND_THREAD_LOCAL:
                foundInfo.isThreadLocal = true;
                break;
            case EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE:
                foundInfo.kind = FoundSymbol::Kind::absolute;
                break;
            default:
                diag.error("unsupported exported symbol kind. flags=%llu at node offset=0x%0lX", flags, (long)(node-trieStart));
                return false;
        }
        return true;
    }
    else {
        // this is an old binary (before macOS 10.6), scan the symbol table
        foundInfo.foundInDylib = nullptr;
        forEachGlobalSymbol(diag, ^(const char* aSymbolName, uint64_t n_value, uint8_t n_type, uint8_t n_sect, uint16_t n_desc, bool& stop) {
            if ( strcmp(aSymbolName, symbolName) == 0 ) {
                foundInfo.kind               = FoundSymbol::Kind::headerOffset;
                foundInfo.isThreadLocal      = false;
                foundInfo.foundInDylib       = this;
                foundInfo.value              = n_value - leInfo.layout.textUnslidVMAddr;
                foundInfo.resolverFuncOffset = 0;
                foundInfo.foundSymbolName    = symbolName;
                stop = true;
            }
        });
        if ( foundInfo.foundInDylib == nullptr ) {
            // symbol not exported from this image. Search any re-exported dylibs
            __block unsigned depIndex = 0;
            forEachDependentDylib(^(const char* loadPath, bool isWeak, bool isReExport, bool isUpward, uint32_t compatVersion, uint32_t curVersion, bool& stop) {
                if ( isReExport && findDependent ) {
                    if ( const MachOLoaded* depMH = findDependent(this, depIndex) ) {
                        if ( depMH->findExportedSymbol(diag, symbolName, weakImport, foundInfo, findDependent) ) {
                            stop = true;
                        }
                    }
                }
                ++depIndex;
            });
        }
        return (foundInfo.foundInDylib != nullptr);
    }
}

intptr_t MachOLoaded::getSlide() const
{
    Diagnostics diag;
    __block intptr_t slide = 0;
    forEachLoadCommand(diag, ^(const load_command* cmd, bool& stop) {
        if ( cmd->cmd == LC_SEGMENT_64 ) {
            const segment_command_64* seg = (segment_command_64*)cmd;
            if ( strcmp(seg->segname, "__TEXT") == 0 ) {
                slide = (uintptr_t)(((uint64_t)this) - seg->vmaddr);
                stop = true;
            }
        }
        else if ( cmd->cmd == LC_SEGMENT ) {
            const segment_command* seg = (segment_command*)cmd;
            if ( strcmp(seg->segname, "__TEXT") == 0 ) {
                slide = (uintptr_t)(((uint64_t)this) - seg->vmaddr);
                stop = true;
            }
        }
    });
    diag.assertNoError();   // any malformations in the file should have been caught by earlier validate() call
    return slide;
}

const uint8_t* MachOLoaded::getLinkEditContent(const LayoutInfo& info, uint32_t fileOffset) const
{
    uint32_t offsetInLinkedit   = fileOffset - info.linkeditFileOffset;
    uintptr_t linkeditStartAddr = info.linkeditUnslidVMAddr + info.slide;
    return (uint8_t*)(linkeditStartAddr + offsetInLinkedit);
}


void MachOLoaded::forEachGlobalSymbol(Diagnostics& diag, void (^callback)(const char* symbolName, uint64_t n_value, uint8_t n_type, uint8_t n_sect, uint16_t n_desc, bool& stop)) const
{
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() )
        return;

    const bool is64Bit = is64();
    if ( leInfo.symTab != nullptr ) {
        uint32_t globalsStartIndex = 0;
        uint32_t globalsCount      = leInfo.symTab->nsyms;
        if ( leInfo.dynSymTab != nullptr ) {
            globalsStartIndex = leInfo.dynSymTab->iextdefsym;
            globalsCount      = leInfo.dynSymTab->nextdefsym;
        }
        uint32_t               maxStringOffset  = leInfo.symTab->strsize;
        const char*            stringPool       =             (char*)getLinkEditContent(leInfo.layout, leInfo.symTab->stroff);
        const struct nlist*    symbols          = (struct nlist*)   (getLinkEditContent(leInfo.layout, leInfo.symTab->symoff));
        const struct nlist_64* symbols64        = (struct nlist_64*)symbols;
        bool                   stop             = false;
        for (uint32_t i=0; (i < globalsCount) && !stop; ++i) {
            if ( is64Bit ) {
                const struct nlist_64& sym = symbols64[globalsStartIndex+i];
                if ( sym.n_un.n_strx > maxStringOffset )
                    continue;
                if ( (sym.n_type & N_EXT) && ((sym.n_type & N_TYPE) == N_SECT) && ((sym.n_type & N_STAB) == 0) )
                    callback(&stringPool[sym.n_un.n_strx], sym.n_value, sym.n_type, sym.n_sect, sym.n_desc, stop);
            }
            else {
                const struct nlist& sym = symbols[globalsStartIndex+i];
                if ( sym.n_un.n_strx > maxStringOffset )
                    continue;
                if ( (sym.n_type & N_EXT) && ((sym.n_type & N_TYPE) == N_SECT) && ((sym.n_type & N_STAB) == 0) )
                    callback(&stringPool[sym.n_un.n_strx], sym.n_value, sym.n_type, sym.n_sect, sym.n_desc, stop);
            }
        }
    }
}

void MachOLoaded::forEachLocalSymbol(Diagnostics& diag, void (^callback)(const char* symbolName, uint64_t n_value, uint8_t n_type, uint8_t n_sect, uint16_t n_desc, bool& stop)) const
{
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() )
        return;

    const bool is64Bit = is64();
    if ( leInfo.symTab != nullptr ) {
        uint32_t localsStartIndex = 0;
        uint32_t localsCount      = leInfo.symTab->nsyms;
        if ( leInfo.dynSymTab != nullptr ) {
            localsStartIndex = leInfo.dynSymTab->ilocalsym;
            localsCount      = leInfo.dynSymTab->nlocalsym;
        }
        uint32_t               maxStringOffset  = leInfo.symTab->strsize;
        const char*            stringPool       =             (char*)getLinkEditContent(leInfo.layout, leInfo.symTab->stroff);
        const struct nlist*    symbols          = (struct nlist*)   (getLinkEditContent(leInfo.layout, leInfo.symTab->symoff));
        const struct nlist_64* symbols64        = (struct nlist_64*)(getLinkEditContent(leInfo.layout, leInfo.symTab->symoff));
        bool                   stop             = false;
        for (uint32_t i=0; (i < localsCount) && !stop; ++i) {
            if ( is64Bit ) {
                const struct nlist_64& sym = symbols64[localsStartIndex+i];
                if ( sym.n_un.n_strx > maxStringOffset )
                    continue;
                if ( ((sym.n_type & N_EXT) == 0) && ((sym.n_type & N_TYPE) == N_SECT) && ((sym.n_type & N_STAB) == 0) )
                    callback(&stringPool[sym.n_un.n_strx], sym.n_value, sym.n_type, sym.n_sect, sym.n_desc, stop);
            }
            else {
                const struct nlist& sym = symbols[localsStartIndex+i];
                if ( sym.n_un.n_strx > maxStringOffset )
                    continue;
                if ( ((sym.n_type & N_EXT) == 0) && ((sym.n_type & N_TYPE) == N_SECT) && ((sym.n_type & N_STAB) == 0) )
                    callback(&stringPool[sym.n_un.n_strx], sym.n_value, sym.n_type, sym.n_sect, sym.n_desc, stop);
            }
        }
    }
}

uint32_t MachOLoaded::dependentDylibCount() const
{
    __block uint32_t count = 0;
    forEachDependentDylib(^(const char* loadPath, bool isWeak, bool isReExport, bool isUpward, uint32_t compatVersion, uint32_t curVersion, bool& stop) {
        ++count;
    });
    return count;
}

const char* MachOLoaded::dependentDylibLoadPath(uint32_t depIndex) const
{
    __block const char* foundLoadPath = nullptr;
    __block uint32_t curDepIndex = 0;
    forEachDependentDylib(^(const char* loadPath, bool isWeak, bool isReExport, bool isUpward, uint32_t compatVersion, uint32_t curVersion, bool& stop) {
        if ( curDepIndex == depIndex ) {
            foundLoadPath = loadPath;
            stop = true;
        }
        ++curDepIndex;
    });
    return foundLoadPath;
}

const char* MachOLoaded::segmentName(uint32_t targetSegIndex) const
{
    __block const char* result = nullptr;
	forEachSegment(^(const SegmentInfo& info, bool& stop) {
        if ( targetSegIndex == info.segIndex ) {
            result = info.segName;
            stop = true;
        }
    });
    return result;
}

bool MachOLoaded::findClosestFunctionStart(uint64_t address, uint64_t* functionStartAddress) const
{
    Diagnostics diag;
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() )
        return false;
    if ( leInfo.functionStarts == nullptr )
        return false;

    const uint8_t* starts    = getLinkEditContent(leInfo.layout, leInfo.functionStarts->dataoff);
    const uint8_t* startsEnd = starts + leInfo.functionStarts->datasize;

    uint64_t lastAddr    = (uint64_t)(long)this;
    uint64_t runningAddr = lastAddr;
    while (diag.noError()) {
        uint64_t value = read_uleb128(diag, starts, startsEnd);
        if ( value == 0 )
            break;
        lastAddr = runningAddr;
        runningAddr += value;
        //fprintf(stderr, "  addr=0x%08llX\n", runningAddr);
        if ( runningAddr > address ) {
            *functionStartAddress = lastAddr;
            return true;
        }
    };

    return false;
}

bool MachOLoaded::findClosestSymbol(uint64_t address, const char** symbolName, uint64_t* symbolAddr) const
{
    Diagnostics diag;
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() )
        return false;
    if ( (leInfo.symTab == nullptr) || (leInfo.dynSymTab == nullptr) )
        return false;
    uint64_t targetUnslidAddress = address - leInfo.layout.slide;

    uint32_t               maxStringOffset  = leInfo.symTab->strsize;
    const char*            stringPool       =             (char*)getLinkEditContent(leInfo.layout, leInfo.symTab->stroff);
    const struct nlist*    symbols          = (struct nlist*)   (getLinkEditContent(leInfo.layout, leInfo.symTab->symoff));
    if ( is64() ) {
        const struct nlist_64* symbols64  = (struct nlist_64*)symbols;
        const struct nlist_64* bestSymbol = nullptr;
        // first walk all global symbols
        const struct nlist_64* const globalsStart = &symbols64[leInfo.dynSymTab->iextdefsym];
        const struct nlist_64* const globalsEnd   = &globalsStart[leInfo.dynSymTab->nextdefsym];
        for (const struct nlist_64* s = globalsStart; s < globalsEnd; ++s) {
            if ( (s->n_type & N_TYPE) == N_SECT ) {
                if ( bestSymbol == nullptr ) {
                    if ( s->n_value <= targetUnslidAddress )
                        bestSymbol = s;
                }
                else if ( (s->n_value <= targetUnslidAddress) && (bestSymbol->n_value < s->n_value) ) {
                    bestSymbol = s;
                }
            }
        }
        // next walk all local symbols
        const struct nlist_64* const localsStart = &symbols64[leInfo.dynSymTab->ilocalsym];
        const struct nlist_64* const localsEnd   = &localsStart[leInfo.dynSymTab->nlocalsym];
        for (const struct nlist_64* s = localsStart; s < localsEnd; ++s) {
             if ( ((s->n_type & N_TYPE) == N_SECT) && ((s->n_type & N_STAB) == 0) ) {
                if ( bestSymbol == nullptr ) {
                    if ( s->n_value <= targetUnslidAddress )
                        bestSymbol = s;
                }
                else if ( (s->n_value <= targetUnslidAddress) && (bestSymbol->n_value < s->n_value) ) {
                    bestSymbol = s;
                }
            }
        }
        if ( bestSymbol != NULL ) {
            *symbolAddr = bestSymbol->n_value + leInfo.layout.slide;
            if ( bestSymbol->n_un.n_strx < maxStringOffset )
                *symbolName = &stringPool[bestSymbol->n_un.n_strx];
            return true;
        }
    }
    else {
       const struct nlist* bestSymbol = nullptr;
        // first walk all global symbols
        const struct nlist* const globalsStart = &symbols[leInfo.dynSymTab->iextdefsym];
        const struct nlist* const globalsEnd   = &globalsStart[leInfo.dynSymTab->nextdefsym];
        for (const struct nlist* s = globalsStart; s < globalsEnd; ++s) {
            if ( (s->n_type & N_TYPE) == N_SECT ) {
                if ( bestSymbol == nullptr ) {
                    if ( s->n_value <= targetUnslidAddress )
                        bestSymbol = s;
                }
                else if ( (s->n_value <= targetUnslidAddress) && (bestSymbol->n_value < s->n_value) ) {
                    bestSymbol = s;
                }
            }
        }
        // next walk all local symbols
        const struct nlist* const localsStart = &symbols[leInfo.dynSymTab->ilocalsym];
        const struct nlist* const localsEnd   = &localsStart[leInfo.dynSymTab->nlocalsym];
        for (const struct nlist* s = localsStart; s < localsEnd; ++s) {
             if ( ((s->n_type & N_TYPE) == N_SECT) && ((s->n_type & N_STAB) == 0) ) {
                if ( bestSymbol == nullptr ) {
                    if ( s->n_value <= targetUnslidAddress )
                        bestSymbol = s;
                }
                else if ( (s->n_value <= targetUnslidAddress) && (bestSymbol->n_value < s->n_value) ) {
                    bestSymbol = s;
                }
            }
        }
        if ( bestSymbol != nullptr ) {
#if __arm__
            if ( bestSymbol->n_desc & N_ARM_THUMB_DEF )
                *symbolAddr = (bestSymbol->n_value | 1) + leInfo.layout.slide;
            else
                *symbolAddr = bestSymbol->n_value + leInfo.layout.slide;
#else
            *symbolAddr = bestSymbol->n_value + leInfo.layout.slide;
#endif
            if ( bestSymbol->n_un.n_strx < maxStringOffset )
                *symbolName = &stringPool[bestSymbol->n_un.n_strx];
            return true;
        }
    }

    return false;
}

const void* MachOLoaded::findSectionContent(const char* segName, const char* sectName, uint64_t& size) const
{
    __block const void* result = nullptr;
    forEachSection(^(const SectionInfo& sectInfo, bool malformedSectionRange, bool& stop) {
        if ( (strcmp(sectInfo.sectName, sectName) == 0) && (strcmp(sectInfo.segInfo.segName, segName) == 0) ) {
            size = sectInfo.sectSize;
            result = (void*)(sectInfo.sectAddr + getSlide());
        }
    });
    return result;
}


bool MachOLoaded::intersectsRange(uintptr_t start, uintptr_t length) const
{
    __block bool result = false;
    uintptr_t slide = getSlide();
    forEachSegment(^(const SegmentInfo& info, bool& stop) {
        if ( (info.vmAddr+info.vmSize+slide >= start) && (info.vmAddr+slide < start+length) )
            result = true;
    });
    return result;
}

const uint8_t* MachOLoaded::trieWalk(Diagnostics& diag, const uint8_t* start, const uint8_t* end, const char* symbol)
{
    uint32_t visitedNodeOffsets[128];
    int visitedNodeOffsetCount = 0;
    visitedNodeOffsets[visitedNodeOffsetCount++] = 0;
    const uint8_t* p = start;
    while ( p < end ) {
        uint64_t terminalSize = *p++;
        if ( terminalSize > 127 ) {
            // except for re-export-with-rename, all terminal sizes fit in one byte
            --p;
            terminalSize = read_uleb128(diag, p, end);
            if ( diag.hasError() )
                return nullptr;
        }
        if ( (*symbol == '\0') && (terminalSize != 0) ) {
            return p;
        }
        const uint8_t* children = p + terminalSize;
        if ( children > end ) {
            //diag.error("malformed trie node, terminalSize=0x%llX extends past end of trie\n", terminalSize);
            return nullptr;
        }
        uint8_t childrenRemaining = *children++;
        p = children;
        uint64_t nodeOffset = 0;
        for (; childrenRemaining > 0; --childrenRemaining) {
            const char* ss = symbol;
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
                    diag.error("malformed trie node, child node extends past end of trie\n");
                    return nullptr;
                }
            }
            else {
                 // the symbol so far matches this edge (child)
                // so advance to the child's node
                ++p;
                nodeOffset = read_uleb128(diag, p, end);
                if ( diag.hasError() )
                    return nullptr;
                if ( (nodeOffset == 0) || ( &start[nodeOffset] > end) ) {
                    diag.error("malformed trie child, nodeOffset=0x%llX out of range\n", nodeOffset);
                    return nullptr;
                }
                symbol = ss;
                break;
            }
        }
        if ( nodeOffset != 0 ) {
            if ( nodeOffset > (uint64_t)(end-start) ) {
                diag.error("malformed trie child, nodeOffset=0x%llX out of range\n", nodeOffset);
               return nullptr;
            }
            for (int i=0; i < visitedNodeOffsetCount; ++i) {
                if ( visitedNodeOffsets[i] == nodeOffset ) {
                    diag.error("malformed trie child, cycle to nodeOffset=0x%llX\n", nodeOffset);
                    return nullptr;
                }
            }
            visitedNodeOffsets[visitedNodeOffsetCount++] = (uint32_t)nodeOffset;
            if ( visitedNodeOffsetCount >= 128 ) {
                diag.error("malformed trie too deep\n");
                return nullptr;
            }
            p = &start[nodeOffset];
        }
        else
            p = end;
    }
    return nullptr;
}

void MachOLoaded::forEachCDHashOfCodeSignature(const void* codeSigStart, size_t codeSignLen,
                                               void (^callback)(const uint8_t cdHash[20])) const
{
#ifndef DARLING
    forEachCodeDirectoryBlob(codeSigStart, codeSignLen, ^(const void *cdBuffer) {
        const CS_CodeDirectory* cd = (const CS_CodeDirectory*)cdBuffer;
        uint32_t cdLength = htonl(cd->length);
        uint8_t cdHash[20];
        if ( cd->hashType == CS_HASHTYPE_SHA384 ) {
            uint8_t digest[CCSHA384_OUTPUT_SIZE];
            const struct ccdigest_info* di = ccsha384_di();
            ccdigest_di_decl(di, tempBuf); // declares tempBuf array in stack
            ccdigest_init(di, tempBuf);
            ccdigest_update(di, tempBuf, cdLength, cd);
            ccdigest_final(di, tempBuf, digest);
            ccdigest_di_clear(di, tempBuf);
            // cd-hash of sigs that use SHA384 is the first 20 bytes of the SHA384 of the code digest
            memcpy(cdHash, digest, 20);
            callback(cdHash);
            return;
        }
        else if ( (cd->hashType == CS_HASHTYPE_SHA256) || (cd->hashType == CS_HASHTYPE_SHA256_TRUNCATED) ) {
            uint8_t digest[CCSHA256_OUTPUT_SIZE];
            const struct ccdigest_info* di = ccsha256_di();
            ccdigest_di_decl(di, tempBuf); // declares tempBuf array in stack
            ccdigest_init(di, tempBuf);
            ccdigest_update(di, tempBuf, cdLength, cd);
            ccdigest_final(di, tempBuf, digest);
            ccdigest_di_clear(di, tempBuf);
            // cd-hash of sigs that use SHA256 is the first 20 bytes of the SHA256 of the code digest
            memcpy(cdHash, digest, 20);
            callback(cdHash);
            return;
        }
        else if ( cd->hashType == CS_HASHTYPE_SHA1 ) {
            // compute hash directly into return buffer
            const struct ccdigest_info* di = ccsha1_di();
            ccdigest_di_decl(di, tempBuf); // declares tempBuf array in stack
            ccdigest_init(di, tempBuf);
            ccdigest_update(di, tempBuf, cdLength, cd);
            ccdigest_final(di, tempBuf, cdHash);
            ccdigest_di_clear(di, tempBuf);
            callback(cdHash);
            return;
        }
    });
#endif
}


// Note, this has to match the kernel
static const uint32_t hashPriorities[] = {
    CS_HASHTYPE_SHA1,
    CS_HASHTYPE_SHA256_TRUNCATED,
    CS_HASHTYPE_SHA256,
    CS_HASHTYPE_SHA384,
};

static unsigned int hash_rank(const CS_CodeDirectory *cd)
{
    uint32_t type = cd->hashType;
    for (uint32_t n = 0; n < sizeof(hashPriorities) / sizeof(hashPriorities[0]); ++n) {
        if (hashPriorities[n] == type)
            return n + 1;
    }

    /* not supported */
    return 0;
}

// Note, this does NOT match the kernel.
// On watchOS, in main executables, we will record all cd hashes then make sure
// one of the ones we record matches the kernel.
// This list is only for dylibs where we embed the cd hash in the closure instead of the
// mod time and inode
// This is sorted so that we choose sha1 first when checking dylibs
static const uint32_t hashPriorities_watchOS_dylibs[] = {
    CS_HASHTYPE_SHA256_TRUNCATED,
    CS_HASHTYPE_SHA256,
    CS_HASHTYPE_SHA384,
    CS_HASHTYPE_SHA1
};

static unsigned int hash_rank_watchOS_dylibs(const CS_CodeDirectory *cd)
{
    uint32_t type = cd->hashType;
    for (uint32_t n = 0; n < sizeof(hashPriorities_watchOS_dylibs) / sizeof(hashPriorities_watchOS_dylibs[0]); ++n) {
        if (hashPriorities_watchOS_dylibs[n] == type)
            return n + 1;
    }

    /* not supported */
    return 0;
}

// This calls the callback for all code directories required for a given platform/binary combination.
// On watchOS main executables this is all cd hashes.
// On watchOS dylibs this is only the single cd hash we need (by rank defined by dyld, not the kernel).
// On all other platforms this always returns a single best cd hash (ranked to match the kernel).
// Note the callback parameter is really a CS_CodeDirectory.
void MachOLoaded::forEachCodeDirectoryBlob(const void* codeSigStart, size_t codeSignLen,
                                           void (^callback)(const void* cd)) const
{
    // verify min length of overall code signature
    if ( codeSignLen < sizeof(CS_SuperBlob) )
        return;

    // verify magic at start
    const CS_SuperBlob* codeSuperBlob = (CS_SuperBlob*)codeSigStart;
    if ( codeSuperBlob->magic != htonl(CSMAGIC_EMBEDDED_SIGNATURE) )
        return;

    // verify count of sub-blobs not too large
    uint32_t subBlobCount = htonl(codeSuperBlob->count);
    if ( (codeSignLen-sizeof(CS_SuperBlob))/sizeof(CS_BlobIndex) < subBlobCount )
        return;

    // Note: The kernel sometimes chooses sha1 on watchOS, and sometimes sha256.
    // Embed all of them so that we just need to match any of them
    const bool isWatchOS = this->supportsPlatform(Platform::watchOS);
    const bool isMainExecutable = this->isMainExecutable();
    auto hashRankFn = isWatchOS ? &hash_rank_watchOS_dylibs : &hash_rank;

    // walk each sub blob, looking at ones with type CSSLOT_CODEDIRECTORY
    const CS_CodeDirectory* bestCd = nullptr;
    for (uint32_t i=0; i < subBlobCount; ++i) {
        if ( codeSuperBlob->index[i].type == htonl(CSSLOT_CODEDIRECTORY) ) {
            // Ok, this is the regular code directory
        } else if ( codeSuperBlob->index[i].type >= htonl(CSSLOT_ALTERNATE_CODEDIRECTORIES) && codeSuperBlob->index[i].type <= htonl(CSSLOT_ALTERNATE_CODEDIRECTORY_LIMIT)) {
            // Ok, this is the alternative code directory
        } else {
            continue;
        }
        uint32_t cdOffset = htonl(codeSuperBlob->index[i].offset);
        // verify offset is not out of range
        if ( cdOffset > (codeSignLen - sizeof(CS_CodeDirectory)) )
            continue;
        const CS_CodeDirectory* cd = (CS_CodeDirectory*)((uint8_t*)codeSuperBlob + cdOffset);
        uint32_t cdLength = htonl(cd->length);
        // verify code directory length not out of range
        if ( cdLength > (codeSignLen - cdOffset) )
            continue;

        // The watch main executable wants to know about all cd hashes
        if ( isWatchOS && isMainExecutable ) {
            callback(cd);
            continue;
        }

        if ( cd->magic == htonl(CSMAGIC_CODEDIRECTORY) ) {
            if ( !bestCd || (hashRankFn(cd) > hashRankFn(bestCd)) )
                bestCd = cd;
        }
    }

    // Note this callback won't happen on watchOS as that one was done in the loop
    if ( bestCd != nullptr )
        callback(bestCd);
}


uint64_t MachOLoaded::ChainedFixupPointerOnDisk::Arm64e::unpackTarget() const
{
    assert(this->authBind.bind == 0);
    assert(this->authBind.auth == 0);
    return ((uint64_t)(this->rebase.high8) << 56) | (this->rebase.target);
}

uint64_t MachOLoaded::ChainedFixupPointerOnDisk::Arm64e::signExtendedAddend() const
{
    assert(this->authBind.bind == 1);
    assert(this->authBind.auth == 0);
    uint64_t addend19     = this->bind.addend;
    if ( addend19 & 0x40000 )
        return addend19 | 0xFFFFFFFFFFFC0000ULL;
    else
        return addend19;
}

const char* MachOLoaded::ChainedFixupPointerOnDisk::Arm64e::keyName() const
{
    static const char* names[] = {
        "IA", "IB", "DA", "DB"
    };
    assert(this->authBind.auth == 1);
    uint8_t keyBits = this->authBind.key;
    assert(keyBits < 4);
    return names[keyBits];
}

uint64_t MachOLoaded::ChainedFixupPointerOnDisk::Arm64e::signPointer(void* loc, uint64_t target) const
{
    assert(this->authBind.auth == 1);
#if __has_feature(ptrauth_calls)
    uint64_t discriminator = authBind.diversity;
    if ( authBind.addrDiv )
        discriminator = __builtin_ptrauth_blend_discriminator(loc, discriminator);
    switch ( authBind.key ) {
        case 0: // IA
            return (uint64_t)__builtin_ptrauth_sign_unauthenticated((void*)target, 0, discriminator);
        case 1: // IB
            return (uint64_t)__builtin_ptrauth_sign_unauthenticated((void*)target, 1, discriminator);
        case 2: // DA
            return (uint64_t)__builtin_ptrauth_sign_unauthenticated((void*)target, 2, discriminator);
        case 3: // DB
            return (uint64_t)__builtin_ptrauth_sign_unauthenticated((void*)target, 3, discriminator);
    }
#endif
    return target;
}

uint64_t MachOLoaded::ChainedFixupPointerOnDisk::Generic64::unpackedTarget() const
{
    return (((uint64_t)this->rebase.high8) << 56) | (uint64_t)(this->rebase.target);
}

uint64_t MachOLoaded::ChainedFixupPointerOnDisk::Generic64::signExtendedAddend() const
{
    uint64_t addend27     = this->bind.addend;
    uint64_t top8Bits     = addend27 & 0x00007F80000ULL;
    uint64_t bottom19Bits = addend27 & 0x0000007FFFFULL;
    uint64_t newValue     = (top8Bits << 13) | (((uint64_t)(bottom19Bits << 37) >> 37) & 0x00FFFFFFFFFFFFFF);
    return newValue;
}

bool MachOLoaded::ChainedFixupPointerOnDisk::isRebase(uint16_t pointerFormat, uint64_t preferedLoadAddress, uint64_t& targetRuntimeOffset) const
{
    switch (pointerFormat) {
       case DYLD_CHAINED_PTR_ARM64E:
            if ( this->arm64e.bind.bind )
                return false;
            if ( this->arm64e.authRebase.auth ) {
                targetRuntimeOffset = this->arm64e.authRebase.target;
                return true;
            }
            else {
                targetRuntimeOffset = this->arm64e.unpackTarget() - preferedLoadAddress;
                return true;
            }
            break;
        case DYLD_CHAINED_PTR_64:
            if ( this->generic64.bind.bind )
                return false;
            targetRuntimeOffset = this->generic64.unpackedTarget() - preferedLoadAddress;
            return true;
            break;
        case DYLD_CHAINED_PTR_32:
            if ( this->generic32.bind.bind )
                return false;
            targetRuntimeOffset = this->generic32.rebase.target - preferedLoadAddress;
            return true;
            break;
        default:
            break;
    }
    assert(0 && "unsupported pointer chain format");
}

bool MachOLoaded::ChainedFixupPointerOnDisk::isBind(uint16_t pointerFormat, uint32_t& bindOrdinal) const
{
    switch (pointerFormat) {
        case DYLD_CHAINED_PTR_ARM64E:
            if ( !this->arm64e.authBind.bind )
                return false;
            if ( this->arm64e.authBind.auth ) {
                bindOrdinal = this->arm64e.authBind.ordinal;
                return true;
            }
            else {
                bindOrdinal = this->arm64e.bind.ordinal;
                return true;
            }
            break;
        case DYLD_CHAINED_PTR_64:
            if ( !this->generic64.bind.bind )
                return false;
            bindOrdinal = this->generic64.bind.ordinal;
            return true;
            break;
        case DYLD_CHAINED_PTR_32:
            if ( !this->generic32.bind.bind )
                return false;
            bindOrdinal = this->generic32.bind.ordinal;
            return true;
            break;
        default:
            break;
    }
    assert(0 && "unsupported pointer chain format");
}

#if BUILDING_DYLD || BUILDING_LIBDYLD
void MachOLoaded::fixupAllChainedFixups(Diagnostics& diag, const dyld_chained_starts_in_image* starts, uintptr_t slide,
                                        Array<const void*> bindTargets, void (^logFixup)(void* loc, void* newValue)) const
{
    forEachFixupInAllChains(diag, starts, true, ^(ChainedFixupPointerOnDisk* fixupLoc, const dyld_chained_starts_in_segment* segInfo, bool& stop) {
        void* newValue;
        switch (segInfo->pointer_format) {
#if __LP64__
  #if  __has_feature(ptrauth_calls)
           case DYLD_CHAINED_PTR_ARM64E:
               if ( fixupLoc->arm64e.authRebase.auth ) {
                    if ( fixupLoc->arm64e.authBind.bind ) {
                        if ( fixupLoc->arm64e.authBind.ordinal >= bindTargets.count() ) {
                            diag.error("out of range bind ordinal %d (max %lu)", fixupLoc->arm64e.authBind.ordinal, bindTargets.count());
                            stop = true;
                            break;
                        }
                        else {
                            // authenticated bind
                            newValue = (void*)(bindTargets[fixupLoc->arm64e.bind.ordinal]);
                            if (newValue != 0)  // Don't sign missing weak imports
                                newValue = (void*)fixupLoc->arm64e.signPointer(fixupLoc, (uintptr_t)newValue);
                        }
                    }
                    else {
                        // authenticated rebase
                        newValue = (void*)fixupLoc->arm64e.signPointer(fixupLoc, (uintptr_t)this + fixupLoc->arm64e.authRebase.target);
                    }
                }
                else {
                    if ( fixupLoc->arm64e.bind.bind ) {
                        if ( fixupLoc->arm64e.bind.ordinal >= bindTargets.count() ) {
                            diag.error("out of range bind ordinal %d (max %lu)", fixupLoc->arm64e.bind.ordinal, bindTargets.count());
                            stop = true;
                            break;
                        }
                        else {
                            // plain bind
                            newValue = (void*)((long)bindTargets[fixupLoc->arm64e.bind.ordinal] + fixupLoc->arm64e.signExtendedAddend());
                        }
                    }
                    else {
                        // plain rebase
                        newValue = (void*)(fixupLoc->arm64e.unpackTarget()+slide);
                   }
                }
                if ( logFixup )
                    logFixup(fixupLoc, newValue);
                fixupLoc->raw64 = (uintptr_t)newValue;
                break;
  #endif
            case DYLD_CHAINED_PTR_64:
                if ( fixupLoc->generic64.bind.bind ) {
                    if ( fixupLoc->generic64.bind.ordinal >= bindTargets.count() ) {
                        diag.error("out of range bind ordinal %d (max %lu)", fixupLoc->generic64.bind.ordinal, bindTargets.count());
                        stop = true;
                        break;
                    }
                    else {
                        newValue = (void*)((long)bindTargets[fixupLoc->generic64.bind.ordinal] + fixupLoc->generic64.signExtendedAddend());
                    }
                }
                else {
                    newValue = (void*)(fixupLoc->generic64.unpackedTarget()+slide);
                }
                if ( logFixup )
                    logFixup(fixupLoc, newValue);
                fixupLoc->raw64 = (uintptr_t)newValue;
               break;
#else
            case DYLD_CHAINED_PTR_32:
                if ( fixupLoc->generic32.bind.bind ) {
                    if ( fixupLoc->generic32.bind.ordinal >= bindTargets.count() ) {
                        diag.error("out of range bind ordinal %d (max %lu)", fixupLoc->generic32.bind.ordinal, bindTargets.count());
                        stop = true;
                        break;
                    }
                    else {
                        newValue = (void*)((long)bindTargets[fixupLoc->generic32.bind.ordinal] + fixupLoc->generic32.bind.addend);
                    }
                }
                else {
                    if ( fixupLoc->generic32.rebase.target > segInfo->max_valid_pointer ) {
                        // handle non-pointers in chain
                        uint32_t bias = (0x04000000 + segInfo->max_valid_pointer)/2;
                        newValue = (void*)(fixupLoc->generic32.rebase.target - bias);
                    }
                    else {
                        newValue = (void*)(fixupLoc->generic32.rebase.target + slide);
                    }
                }
                if ( logFixup )
                    logFixup(fixupLoc, newValue);
                fixupLoc->raw32 = (uint32_t)(uintptr_t)newValue;
               break;
#endif // __LP64__
            default:
                diag.error("unsupported pointer chain format: 0x%04X", segInfo->pointer_format);
                stop = true;
                break;
        }
    });
}
#endif

bool MachOLoaded::walkChain(Diagnostics& diag, const dyld_chained_starts_in_segment* segInfo, uint32_t pageIndex, uint16_t offsetInPage,
                            bool notifyNonPointers, void (^handler)(ChainedFixupPointerOnDisk* fixupLocation, const dyld_chained_starts_in_segment* segInfo, bool& stop)) const
{
    bool                       stop = false;
    uint8_t*                   pageContentStart = (uint8_t*)this + segInfo->segment_offset + (pageIndex * segInfo->page_size);
    ChainedFixupPointerOnDisk* chain = (ChainedFixupPointerOnDisk*)(pageContentStart+offsetInPage);
    bool                       chainEnd = false;
    while (!stop && !chainEnd) {
        // copy chain content, in case handler modifies location to final value
        ChainedFixupPointerOnDisk chainContent = *chain;
        handler(chain, segInfo, stop);
        if ( !stop ) {
            switch (segInfo->pointer_format) {
                case DYLD_CHAINED_PTR_ARM64E:
                    if ( chainContent.arm64e.rebase.next == 0 )
                        chainEnd = true;
                    else
                        chain = (ChainedFixupPointerOnDisk*)((uint8_t*)chain + chainContent.arm64e.rebase.next*8);
                    break;
                case DYLD_CHAINED_PTR_64:
                    if ( chainContent.generic64.rebase.next == 0 )
                        chainEnd = true;
                    else
                        chain = (ChainedFixupPointerOnDisk*)((uint8_t*)chain + chainContent.generic64.rebase.next*4);
                    break;
                case DYLD_CHAINED_PTR_32:
                    if ( chainContent.generic32.rebase.next == 0 )
                        chainEnd = true;
                    else {
                        chain = (ChainedFixupPointerOnDisk*)((uint8_t*)chain + chainContent.generic32.rebase.next*4);
                        if ( !notifyNonPointers ) {
                            while ( (chain->generic32.rebase.bind == 0) && (chain->generic32.rebase.target > segInfo->max_valid_pointer) ) {
                                // not a real pointer, but a non-pointer co-opted into chain
                                chain = (ChainedFixupPointerOnDisk*)((uint8_t*)chain + chain->generic32.rebase.next*4);
                            }
                        }
                    }
                    break;
                default:
                    diag.error("unknown pointer format 0x%04X", segInfo->pointer_format);
                    stop = true;
            }
        }
    }
    return stop;
}

void MachOLoaded::forEachFixupInAllChains(Diagnostics& diag, const dyld_chained_starts_in_image* starts, bool notifyNonPointers,
                                          void (^handler)(ChainedFixupPointerOnDisk* fixupLocation, const dyld_chained_starts_in_segment* segInfo, bool& stop)) const
{
    bool stopped = false;
    for (uint32_t segIndex=0; segIndex < starts->seg_count && !stopped; ++segIndex) {
        if ( starts->seg_info_offset[segIndex] == 0 )
            continue;
        const dyld_chained_starts_in_segment* segInfo = (dyld_chained_starts_in_segment*)((uint8_t*)starts + starts->seg_info_offset[segIndex]);
        for (uint32_t pageIndex=0; pageIndex < segInfo->page_count && !stopped; ++pageIndex) {
            uint16_t offsetInPage = segInfo->page_start[pageIndex];
            if ( offsetInPage == DYLD_CHAINED_PTR_START_NONE )
                continue;
            if ( offsetInPage & DYLD_CHAINED_PTR_START_MULTI ) {
                // 32-bit chains which may need multiple starts per page
                uint32_t overflowIndex = offsetInPage & ~DYLD_CHAINED_PTR_START_MULTI;
                bool chainEnd = false;
                while (!stopped && !chainEnd) {
                    chainEnd = (segInfo->page_start[overflowIndex] & DYLD_CHAINED_PTR_START_LAST);
                    offsetInPage = (segInfo->page_start[overflowIndex] & ~DYLD_CHAINED_PTR_START_LAST);
                    if ( walkChain(diag, segInfo, pageIndex, offsetInPage, notifyNonPointers, handler) )
                        stopped = true;
                    ++overflowIndex;
                }
            }
            else {
                // one chain per page
                walkChain(diag, segInfo, pageIndex, offsetInPage, notifyNonPointers, handler);
            }
        }
    }
}


} // namespace dyld3


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

#ifndef MachOLoaded_h
#define MachOLoaded_h

#include <stdint.h>

#include "Array.h"
#include "MachOFile.h"


class CacheBuilder;

namespace dyld3 {


// A mach-o mapped into memory with zero-fill expansion
// Can be used in dyld at runtime or during closure building
struct VIS_HIDDEN MachOLoaded : public MachOFile
{
	typedef const MachOLoaded* (^DependentToMachOLoaded)(const MachOLoaded* image, uint32_t depIndex);

    // for dlsym()
	bool                hasExportedSymbol(const char* symbolName, DependentToMachOLoaded finder, void** result,
                                          bool* resultPointsToInstructions) const;

    // for DYLD_PRINT_SEGMENTS
    const char*         segmentName(uint32_t segIndex) const;

    // used to see if main executable overlaps shared region
    bool                intersectsRange(uintptr_t start, uintptr_t length) const;

    // for _dyld_get_image_slide()
    intptr_t            getSlide() const;

    // for dladdr()
    bool                findClosestSymbol(uint64_t unSlidAddr, const char** symbolName, uint64_t* symbolUnslidAddr) const;

    // for _dyld_find_unwind_sections()
    const void*         findSectionContent(const char* segName, const char* sectName, uint64_t& size) const;

    // used at runtime to validate loaded image matches closure
    void                forEachCDHashOfCodeSignature(const void* codeSigStart, size_t codeSignLen,
                                                     void (^callback)(const uint8_t cdHash[20])) const;

    // used by DyldSharedCache to find closure
    static const uint8_t*   trieWalk(Diagnostics& diag, const uint8_t* start, const uint8_t* end, const char* symbol);

    // used by cache builder during error handling in chain bind processing
    const char*             dependentDylibLoadPath(uint32_t depIndex) const;

    // used by closure builder to find the offset and size of the trie.
    bool                    hasExportTrie(uint32_t& runtimeOffset, uint32_t& size) const;

    // used by dyld/libdyld to apply fixups
#if BUILDING_DYLD || BUILDING_LIBDYLD
    void                    fixupAllChainedFixups(Diagnostics& diag, const dyld_chained_starts_in_image* starts, uintptr_t slide,
                                                  Array<const void*> bindTargets, void (^fixupLogger)(void* loc, void* newValue)) const;
#endif



    // For use with new rebase/bind scheme were each fixup location on disk contains info on what
    // fix up it needs plus the offset to the next fixup.
    union ChainedFixupPointerOnDisk
    {
        union Arm64e {
            dyld_chained_ptr_arm64e_auth_rebase authRebase;
            dyld_chained_ptr_arm64e_auth_bind   authBind;
            dyld_chained_ptr_arm64e_rebase      rebase;
            dyld_chained_ptr_arm64e_bind        bind;

            uint64_t        signExtendedAddend() const;
            uint64_t        unpackTarget() const;
            const char*     keyName() const;
            uint64_t        signPointer(void* loc, uint64_t target) const;
        };

        union Generic64 {
            dyld_chained_ptr_64_rebase rebase;
            dyld_chained_ptr_64_bind   bind;
            
            uint64_t        signExtendedAddend() const;
            uint64_t        unpackedTarget() const;
        };

        union Generic32 {
            dyld_chained_ptr_32_rebase rebase;
            dyld_chained_ptr_32_bind   bind;

            uint64_t        signExtendedAddend() const;
        };

        typedef dyld_chained_ptr_32_cache_rebase Cache32;

        uint64_t            raw64;
        Arm64e              arm64e;
        Generic64           generic64;

        uint32_t            raw32;
        Generic32           generic32;
        Cache32             cache32;


        bool                isRebase(uint16_t pointerFormat, uint64_t preferedLoadAddress, uint64_t& targetRuntimeOffset) const;
        bool                isBind(uint16_t pointerFormat, uint32_t& bindOrdinal) const;
    };


     struct LayoutInfo {
        uintptr_t    slide;
        uintptr_t    textUnslidVMAddr;
        uintptr_t    linkeditUnslidVMAddr;
        uint32_t     linkeditFileOffset;
        uint32_t     linkeditFileSize;
        uint32_t     linkeditSegIndex;
    };

    struct LinkEditInfo
    {
        const dyld_info_command*        dyldInfo;
        const linkedit_data_command*    exportsTrie;
        const linkedit_data_command*    chainedFixups;
        const symtab_command*           symTab;
        const dysymtab_command*         dynSymTab;
        const linkedit_data_command*    splitSegInfo;
        const linkedit_data_command*    functionStarts;
        const linkedit_data_command*    dataInCode;
        const linkedit_data_command*    codeSig;
        LayoutInfo                      layout;
    };
    void                    getLinkEditPointers(Diagnostics& diag, LinkEditInfo&) const;

    // use by dyldinfo
    void                    forEachFixupInAllChains(Diagnostics& diag, const dyld_chained_starts_in_image* starts, bool notifyNonPointers,
                                                    void (^callback)(ChainedFixupPointerOnDisk* fixupLocation, const dyld_chained_starts_in_segment* segInfo, bool& stop)) const;
protected:
    friend CacheBuilder;

    struct FoundSymbol {
        enum class Kind { headerOffset, absolute, resolverOffset };
        Kind                kind;
        bool                isThreadLocal;
        bool                isWeakDef;
        const MachOLoaded*  foundInDylib;
        uint64_t            value;
        uint32_t            resolverFuncOffset;
        const char*         foundSymbolName;
    };


protected:
    friend CacheBuilder;

    bool                    findExportedSymbol(Diagnostics& diag, const char* symbolName, bool weakImport, FoundSymbol& foundInfo, DependentToMachOLoaded finder) const;

    void                    getLinkEditLoadCommands(Diagnostics& diag, LinkEditInfo& result) const;
    void                    getLayoutInfo(LayoutInfo&) const;
    const uint8_t*          getLinkEditContent(const LayoutInfo& info, uint32_t fileOffset) const;
    const uint8_t*          getExportsTrie(const LinkEditInfo& info, uint64_t& trieSize) const;
    void                    forEachGlobalSymbol(Diagnostics& diag, void (^callback)(const char* symbolName, uint64_t n_value, uint8_t n_type, uint8_t n_sect, uint16_t n_desc, bool& stop)) const;
    void                    forEachLocalSymbol(Diagnostics& diag, void (^callback)(const char* symbolName, uint64_t n_value, uint8_t n_type, uint8_t n_sect, uint16_t n_desc, bool& stop)) const;
    uint32_t                dependentDylibCount() const;
    bool                    findClosestFunctionStart(uint64_t address, uint64_t* functionStartAddress) const;

    // This calls the callback for all code directories required for a given platform/binary combination.
    // On watchOS main executables this is all cd hashes.
    // On watchOS dylibs this is only the single cd hash we need (by rank defined by dyld, not the kernel).
    // On all other platforms this always returns a single best cd hash (ranked to match the kernel).
    // Note the callback parameter is really a CS_CodeDirectory.
    void                    forEachCodeDirectoryBlob(const void* codeSigStart, size_t codeSignLen, void (^callback)(const void* cd)) const;
    bool                    walkChain(Diagnostics& diag, const dyld_chained_starts_in_segment* segInfo, uint32_t pageIndex, uint16_t offsetInPage,
                                      bool notifyNonPointers, void (^handler)(ChainedFixupPointerOnDisk* fixupLocation, const dyld_chained_starts_in_segment* segInfo, bool& stop)) const;


};

} // namespace dyld3

#endif /* MachOLoaded_h */

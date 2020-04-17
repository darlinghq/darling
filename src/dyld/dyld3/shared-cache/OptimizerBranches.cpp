/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- 
 *
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <CommonCrypto/CommonDigest.h>

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "StringUtils.h"
#include "Trie.hpp"
#include "MachOFileAbstraction.hpp"
#include "MachOAnalyzer.h"
#include "Diagnostics.h"
#include "DyldSharedCache.h"
#include "CacheBuilder.h"

static const bool verbose = false;




template <typename P>
class StubOptimizer {
public:
                            StubOptimizer(const DyldSharedCache* cache, macho_header<P>* mh, Diagnostics& diags);
    void                    buildStubMap(const std::unordered_set<std::string>& neverStubEliminate);
    void                    optimizeStubs();
    void                    optimizeCallSites(std::unordered_map<uint64_t, uint64_t>& targetAddrToOptStubAddr);
    const char*             installName() { return _installName; }
    const uint8_t*          exportsTrie() {
        if ( _dyldInfo != nullptr )
            return &_linkeditBias[_dyldInfo->export_off()];
        else
            return &_linkeditBias[_exportTrie->dataoff()];
    }
    uint32_t                exportsTrieSize() {
        if ( _dyldInfo != nullptr )
            return _dyldInfo->export_size();
        else
            return _exportTrie->datasize();
    }

    uint32_t                _stubCount                      = 0;
    uint32_t                _stubOptimizedCount             = 0;
    uint32_t                _stubsLeftInterposable          = 0;
    uint32_t                _branchToStubCount              = 0;
    uint32_t                _branchOptimizedToDirectCount   = 0;
    uint32_t                _branchToOptimizedStubCount     = 0;
    uint32_t                _branchToReUsedOptimizedStubCount = 0;

private:
    Diagnostics             _diagnostics;

    typedef std::function<bool(uint8_t callSiteKind, uint64_t callSiteAddr, uint64_t stubAddr, uint32_t& instruction)> CallSiteHandler;
    typedef typename P::uint_t pint_t;
    typedef typename P::E E;

    void                    forEachCallSiteToAStub(CallSiteHandler);
    void                    optimizeArm64CallSites(std::unordered_map<uint64_t, uint64_t>& targetAddrToOptStubAddr);
    void                    optimizeArm64Stubs();
#if SUPPORT_ARCH_arm64e
    void                    optimizeArm64eStubs();
#endif
#if SUPPORT_ARCH_arm64_32
    void                    optimizeArm64_32Stubs();
#endif
    void                    optimizeArmCallSites(std::unordered_map<uint64_t, uint64_t>& targetAddrToOptStubAddr);
    void                    optimizeArmStubs();
    uint64_t                lazyPointerAddrFromArm64Stub(const uint8_t* stubInstructions, uint64_t stubVMAddr);
#if SUPPORT_ARCH_arm64e
    uint64_t                lazyPointerAddrFromArm64eStub(const uint8_t* stubInstructions, uint64_t stubVMAddr);
#endif
#if SUPPORT_ARCH_arm64_32
    uint64_t                lazyPointerAddrFromArm64_32Stub(const uint8_t* stubInstructions, uint64_t stubVMAddr);
#endif
    uint32_t                lazyPointerAddrFromArmStub(const uint8_t* stubInstructions, uint32_t stubVMAddr);
    int32_t                 getDisplacementFromThumbBranch(uint32_t instruction, uint32_t instrAddr);
    uint32_t                setDisplacementInThumbBranch(uint32_t instruction,  uint32_t instrAddr,
                                                         int32_t displacement, bool targetIsThumb);


    struct AddressAndName { pint_t targetVMAddr; const char* targetName; };
    typedef std::unordered_map<pint_t, AddressAndName> StubVMAddrToTarget;

    static const int64_t b128MegLimit = 0x07FFFFFF;
    static const int64_t b16MegLimit  = 0x00FFFFFF;



    macho_header<P>*                        _mh;
    int64_t                                 _cacheSlide          = 0;
    uint64_t                                _cacheUnslideAddr    = 0;
    bool                                    _chainedFixups       = false;
    uint32_t                                _linkeditSize        = 0;
    uint64_t                                _linkeditAddr        = 0;
    const uint8_t*                          _linkeditBias        = nullptr;
    const char*                             _installName         = nullptr;
    const macho_symtab_command<P>*          _symTabCmd           = nullptr;
    const macho_dysymtab_command<P>*        _dynSymTabCmd        = nullptr;
    const macho_dyld_info_command<P>*       _dyldInfo            = nullptr;
    const macho_linkedit_data_command<P>*   _exportTrie          = nullptr;
    macho_linkedit_data_command<P>*         _splitSegInfoCmd     = nullptr;
    const macho_section<P>*                 _textSection         = nullptr;
    const macho_section<P>*                 _stubSection         = nullptr;
    uint32_t                                _textSectionIndex    = 0;
    uint32_t                                _stubSectionIndex    = 0;
    pint_t                                  _textSegStartAddr    = 0;
    std::vector<macho_segment_command<P>*>  _segCmds;
    std::unordered_map<pint_t, pint_t>      _stubAddrToLPAddr;
    std::unordered_map<pint_t, pint_t>      _lpAddrToTargetAddr;
    std::unordered_map<pint_t, const char*> _targetAddrToName;
    std::unordered_set<uint64_t>            _stubsToOptimize;
};


template <typename P>
StubOptimizer<P>::StubOptimizer(const DyldSharedCache* cache, macho_header<P>* mh, Diagnostics& diags)
: _mh(mh), _diagnostics(diags)
{
    _cacheSlide = (long)cache - cache->unslidLoadAddress();
    _cacheUnslideAddr = cache->unslidLoadAddress();
#if SUPPORT_ARCH_arm64e
    _chainedFixups = (strcmp(cache->archName(), "arm64e") == 0);
#else
    _chainedFixups = false;
#endif
    const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)mh + sizeof(macho_header<P>));
    const uint32_t cmd_count = mh->ncmds();
    macho_segment_command<P>* segCmd;
    uint32_t sectionIndex = 0;
    const macho_load_command<P>* cmd = cmds;
    for (uint32_t i = 0; i < cmd_count; ++i) {
        switch (cmd->cmd()) {
            case LC_ID_DYLIB:
                _installName = ((macho_dylib_command<P>*)cmd)->name();
                break;
            case LC_SYMTAB:
                _symTabCmd = (macho_symtab_command<P>*)cmd;
                break;
            case LC_DYSYMTAB:
                _dynSymTabCmd = (macho_dysymtab_command<P>*)cmd;
                break;
            case LC_SEGMENT_SPLIT_INFO:
                _splitSegInfoCmd = (macho_linkedit_data_command<P>*)cmd;
                break;
            case LC_DYLD_INFO:
            case LC_DYLD_INFO_ONLY:
                _dyldInfo = (macho_dyld_info_command<P>*)cmd;
                break;
            case LC_DYLD_EXPORTS_TRIE:
                _exportTrie = (macho_linkedit_data_command<P>*)cmd;
                break;
            case macho_segment_command<P>::CMD:
                segCmd =( macho_segment_command<P>*)cmd;
                _segCmds.push_back(segCmd);
                if ( strcmp(segCmd->segname(), "__LINKEDIT") == 0 ) {
                    _linkeditBias        = (uint8_t*)(segCmd->vmaddr() + _cacheSlide - segCmd->fileoff());
                    _linkeditSize        = (uint32_t)segCmd->vmsize();
                    _linkeditAddr        = segCmd->vmaddr();
                }
                else if ( strcmp(segCmd->segname(), "__TEXT") == 0 ) {
                    _textSegStartAddr = (pint_t)segCmd->vmaddr();
                    const macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
                    const macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
                    for (const macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
                        ++sectionIndex;
                        if ( strcmp(sect->sectname(), "__text") == 0 ) {
                            _textSection = sect;
                            _textSectionIndex = sectionIndex;
                        }
                        else if ( ((sect->flags() & SECTION_TYPE) == S_SYMBOL_STUBS) && (sect->size() != 0) ) {
                            _stubSection = sect;
                            _stubSectionIndex = sectionIndex;
                        }
                    }
                }
                break;
        }
        cmd = (const macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
    }
}



template <typename P>
uint32_t StubOptimizer<P>::lazyPointerAddrFromArmStub(const uint8_t* stubInstructions, uint32_t stubVMAddr)
{
    uint32_t stubInstr1 = E::get32(*(uint32_t*)stubInstructions);
    uint32_t stubInstr2 = E::get32(*(uint32_t*)(stubInstructions+4));
    uint32_t stubInstr3 = E::get32(*(uint32_t*)(stubInstructions+8));
    int32_t  stubData   = E::get32(*(uint32_t*)(stubInstructions+12));
    if ( stubInstr1 != 0xe59fc004 ) {
        _diagnostics.warning("first instruction of stub (0x%08X) is not 'ldr ip, pc + 12' for stub at addr 0x%0llX in %s",
                stubInstr1, (uint64_t)stubVMAddr, _installName);
        return 0;
    }
    if ( stubInstr2 != 0xe08fc00c ) {
        _diagnostics.warning("second instruction of stub (0x%08X) is not 'add ip, pc, ip' for stub at addr 0x%0llX in %s",
                stubInstr1, (uint64_t)stubVMAddr, _installName);
        return 0;
    }
    if ( stubInstr3 != 0xe59cf000 ) {
        _diagnostics.warning("third instruction of stub (0x%08X) is not 'ldr pc, [ip]' for stub at addr 0x%0llX in %s",
                stubInstr1, (uint64_t)stubVMAddr, _installName);
        return 0;
    }
    return stubVMAddr + 12 + stubData;
}


template <typename P>
uint64_t StubOptimizer<P>::lazyPointerAddrFromArm64Stub(const uint8_t* stubInstructions, uint64_t stubVMAddr)
{
    uint32_t stubInstr1 = E::get32(*(uint32_t*)stubInstructions);
    if ( (stubInstr1 & 0x9F00001F) != 0x90000010 ) {
        _diagnostics.warning("first instruction of stub (0x%08X) is not ADRP for stub at addr 0x%0llX in %s",
                stubInstr1, (uint64_t)stubVMAddr, _installName);
        return 0;
    }
    int32_t adrpValue = ((stubInstr1 & 0x00FFFFE0) >> 3) | ((stubInstr1 & 0x60000000) >> 29);
    if ( stubInstr1 & 0x00800000 )
        adrpValue |= 0xFFF00000;
    uint32_t stubInstr2 = E::get32(*(uint32_t*)(stubInstructions + 4));
    if ( (stubInstr2 & 0xFFC003FF) != 0xF9400210 ) {
        _diagnostics.warning("second instruction of stub (0x%08X) is not LDR for stub at addr 0x%0llX in %s",
                stubInstr2, (uint64_t)stubVMAddr, _installName);
        return 0;
    }
    uint32_t ldrValue = ((stubInstr2 >> 10) & 0x00000FFF);
    return (stubVMAddr & (-4096)) + adrpValue*4096 + ldrValue*8;
}

#if SUPPORT_ARCH_arm64_32
template <typename P>
uint64_t StubOptimizer<P>::lazyPointerAddrFromArm64_32Stub(const uint8_t* stubInstructions, uint64_t stubVMAddr)
{
    uint32_t stubInstr1 = E::get32(*(uint32_t*)stubInstructions);
    if ( (stubInstr1 & 0x9F00001F) != 0x90000010 ) {
        _diagnostics.warning("first instruction of stub (0x%08X) is not ADRP for stub at addr 0x%0llX in %s",
                stubInstr1, (uint64_t)stubVMAddr, _installName);
        return 0;
    }
    int32_t adrpValue = ((stubInstr1 & 0x00FFFFE0) >> 3) | ((stubInstr1 & 0x60000000) >> 29);
    if ( stubInstr1 & 0x00800000 )
        adrpValue |= 0xFFF00000;
    uint32_t stubInstr2 = E::get32(*(uint32_t*)(stubInstructions + 4));
    if ( (stubInstr2 & 0xFFC003FF) != 0xB9400210 ) {
        _diagnostics.warning("second instruction of stub (0x%08X) is not LDR for stub at addr 0x%0llX in %s",
                stubInstr2, (uint64_t)stubVMAddr, _installName);
        return 0;
    }
    uint32_t ldrValue = ((stubInstr2 >> 10) & 0x00000FFF);
    return (stubVMAddr & (-4096)) + adrpValue*4096 + ldrValue*4; // LDR Wn has a scale factor of 4

}
#endif


#if SUPPORT_ARCH_arm64e
template <typename P>
uint64_t StubOptimizer<P>::lazyPointerAddrFromArm64eStub(const uint8_t* stubInstructions, uint64_t stubVMAddr)
{
    uint32_t stubInstr1 = E::get32(*(uint32_t*)stubInstructions);
    // ADRP  X17, dyld_mageLoaderCache@page
    if ( (stubInstr1 & 0x9F00001F) != 0x90000011 ) {
        _diagnostics.warning("first instruction of stub (0x%08X) is not ADRP for stub at addr 0x%0llX in %s",
                stubInstr1, (uint64_t)stubVMAddr, _installName);
        return 0;
    }
    int32_t adrpValue = ((stubInstr1 & 0x00FFFFE0) >> 3) | ((stubInstr1 & 0x60000000) >> 29);
    if ( stubInstr1 & 0x00800000 )
        adrpValue |= 0xFFF00000;

    // ADD     X17, X17, dyld_mageLoaderCache@pageoff
    uint32_t stubInstr2 = E::get32(*(uint32_t*)(stubInstructions + 4));
    if ( (stubInstr2 & 0xFFC003FF) != 0x91000231 ) {
        _diagnostics.warning("second instruction of stub (0x%08X) is not ADD for stub at addr 0x%0llX in %s",
                             stubInstr2, (uint64_t)stubVMAddr, _installName);
        return 0;
    }
    uint32_t addValue = ((stubInstr2 & 0x003FFC00) >> 10);

    // LDR   X16, [X17]
    uint32_t stubInstr3 = E::get32(*(uint32_t*)(stubInstructions + 8));
    if ( stubInstr3 != 0xF9400230 ) {
        _diagnostics.warning("second instruction of stub (0x%08X) is not LDR for stub at addr 0x%0llX in %s",
                stubInstr2, (uint64_t)stubVMAddr, _installName);
        return 0;
    }
    return (stubVMAddr & (-4096)) + adrpValue*4096 + addValue;
}
#endif


template <typename P>
void StubOptimizer<P>::buildStubMap(const std::unordered_set<std::string>& neverStubEliminate)
{
    // find all stubs and lazy pointers
    const macho_nlist<P>* symbolTable = (const macho_nlist<P>*)(&_linkeditBias[_symTabCmd->symoff()]);
    const char* symbolStrings = (char*)(&_linkeditBias[_symTabCmd->stroff()]);
    const uint32_t* const indirectTable = (uint32_t*)(&_linkeditBias[_dynSymTabCmd->indirectsymoff()]);
    const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)_mh + sizeof(macho_header<P>));
    const uint32_t cmd_count = _mh->ncmds();
    const macho_load_command<P>* cmd = cmds;
    for (uint32_t i = 0; i < cmd_count; ++i) {
        if ( cmd->cmd() == macho_segment_command<P>::CMD ) {
            macho_segment_command<P>* seg = (macho_segment_command<P>*)cmd;
            macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)seg + sizeof(macho_segment_command<P>));
            macho_section<P>* const sectionsEnd = &sectionsStart[seg->nsects()];
            for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
                if ( sect->size() == 0 ) 
                    continue;
                unsigned sectionType = (sect->flags() & SECTION_TYPE);
                const uint32_t indirectTableOffset = sect->reserved1();
                if ( sectionType == S_SYMBOL_STUBS ) {
                    const uint32_t stubSize = sect->reserved2();
                    _stubCount = (uint32_t)(sect->size() / stubSize);
                    pint_t stubVMAddr = (pint_t)sect->addr();
                    for (uint32_t j=0; j < _stubCount; ++j, stubVMAddr += stubSize) {
                        uint32_t symbolIndex = E::get32(indirectTable[indirectTableOffset + j]); 
                        switch ( symbolIndex ) {
                            case INDIRECT_SYMBOL_ABS:
                            case INDIRECT_SYMBOL_LOCAL:
                            case INDIRECT_SYMBOL_ABS | INDIRECT_SYMBOL_LOCAL:
                                break;
                            default:
                                if ( symbolIndex >= _symTabCmd->nsyms() ) {
                                    _diagnostics.warning("symbol index out of range (%d of %d) for stub at addr 0x%0llX in %s",
                                        symbolIndex, _symTabCmd->nsyms(), (uint64_t)stubVMAddr, _installName);
                                    continue;
                                }
                                const macho_nlist<P>* sym = &symbolTable[symbolIndex];
                                uint32_t stringOffset = sym->n_strx();
                                if ( stringOffset > _symTabCmd->strsize() ) {
                                    _diagnostics.warning("symbol string offset out of range (%u of %u) for stub at addr 0x%0llX in %s",
                                        stringOffset, sym->n_strx(), (uint64_t)stubVMAddr, _installName);
                                    continue;
                                }
                                const char* symName = &symbolStrings[stringOffset];
                                if ( neverStubEliminate.count(symName) ) {
                                    //fprintf(stderr, "stubVMAddr=0x%llX, not bypassing stub to %s in %s because target is interposable\n", (uint64_t)stubVMAddr, symName, _installName);
                                    _stubsLeftInterposable++;
                                    continue;
                                }
                                const uint8_t* stubInstrs = (uint8_t*)(long)stubVMAddr + _cacheSlide;
                                pint_t targetLPAddr = 0;
                                switch ( _mh->cputype() ) {
                                    case CPU_TYPE_ARM64:
                                    case CPU_TYPE_ARM64_32:
#if SUPPORT_ARCH_arm64e
                                        if (_mh->cpusubtype() == CPU_SUBTYPE_ARM64E)
                                            targetLPAddr = (pint_t)lazyPointerAddrFromArm64eStub(stubInstrs, stubVMAddr);
                                        else
#endif
#if SUPPORT_ARCH_arm64_32
                                        if (_mh->cputype() == CPU_TYPE_ARM64_32)
                                            targetLPAddr = (pint_t)lazyPointerAddrFromArm64_32Stub(stubInstrs, stubVMAddr);
                                        else
#endif
                                            targetLPAddr = (pint_t)lazyPointerAddrFromArm64Stub(stubInstrs, stubVMAddr);
                                        break;
                                    case CPU_TYPE_ARM:
                                        targetLPAddr = (pint_t)lazyPointerAddrFromArmStub(stubInstrs, (uint32_t)stubVMAddr);
                                        break;
                                }
                                if ( targetLPAddr != 0 )
                                    _stubAddrToLPAddr[stubVMAddr] = targetLPAddr;
                                break;
                        }
                    }
                }
                else if ( (sectionType == S_LAZY_SYMBOL_POINTERS) || (sectionType == S_NON_LAZY_SYMBOL_POINTERS) ) {
                    pint_t lpVMAddr;
                    pint_t* lpContent = (pint_t*)(sect->addr() + _cacheSlide);
                    uint32_t elementCount = (uint32_t)(sect->size() / sizeof(pint_t));
                    uint64_t textSegStartAddr = _segCmds[0]->vmaddr();
                    uint64_t textSegEndAddr   = _segCmds[0]->vmaddr() + _segCmds[0]->vmsize();
                    pint_t lpValue;
                     for (uint32_t j=0; j < elementCount; ++j) {
                        uint32_t symbolIndex = E::get32(indirectTable[indirectTableOffset + j]); 
                        switch ( symbolIndex ) {
                            case INDIRECT_SYMBOL_ABS:
                            case INDIRECT_SYMBOL_LOCAL:
                            case INDIRECT_SYMBOL_LOCAL|INDIRECT_SYMBOL_ABS:
                                break;
                            default:
                                lpValue = (pint_t)P::getP(lpContent[j]);

                                // Fixup threaded rebase/bind
                                if ( _chainedFixups ) {
                                    dyld3::MachOLoaded::ChainedFixupPointerOnDisk ptr;
                                    ptr.raw64 = lpValue;
                                    assert(ptr.arm64e.authRebase.bind == 0);
                                    if ( ptr.arm64e.authRebase.auth ) {
                                        lpValue = (pint_t)(_cacheUnslideAddr + ptr.arm64e.authRebase.target);
                                    }
                                    else {
                                        lpValue = (pint_t)ptr.arm64e.unpackTarget();
                                    }
                                }

                                lpVMAddr = (pint_t)sect->addr() + j * sizeof(pint_t);
                                if ( symbolIndex >= _symTabCmd->nsyms() ) {
                                    _diagnostics.warning("symbol index out of range (%d of %d) for lazy pointer at addr 0x%0llX in %s",
                                            symbolIndex, _symTabCmd->nsyms(), (uint64_t)lpVMAddr, _installName);
                                    continue;
                                }
                                const macho_nlist<P>* sym = &symbolTable[symbolIndex];
                                uint32_t stringOffset = sym->n_strx();
                                if ( stringOffset > _symTabCmd->strsize() ) {
                                    _diagnostics.warning("symbol string offset out of range (%u of %u) for lazy pointer at addr 0x%0llX in %s",
                                            stringOffset, sym->n_strx(), (uint64_t)lpVMAddr, _installName);
                                    continue;
                                }
                                const char* symName = &symbolStrings[stringOffset];
                                if ( (lpValue > textSegStartAddr) && (lpValue< textSegEndAddr) ) {
                                    //fprintf(stderr, "skipping lazy pointer at 0x%0lX to %s in %s because target is within dylib\n", (long)lpVMAddr, symName, _installName);
                                }
                                else if ( (sizeof(pint_t) == 8) && ((lpValue % 4) != 0) ) {
                                    _diagnostics.warning("lazy pointer at 0x%0llX does not point to 4-byte aligned address(0x%0llX) in %s",
                                                (uint64_t)lpVMAddr, (uint64_t)lpValue, _installName);
                                }
                                else {
                                   _lpAddrToTargetAddr[lpVMAddr] = lpValue;
                                   _targetAddrToName[lpValue] = symName;
                                }
                                 break;
                        }
                    }
                }
            }
        }
        cmd = (const macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
    }
}


template <typename P>
void StubOptimizer<P>::forEachCallSiteToAStub(CallSiteHandler handler)
{
    if (_diagnostics.hasError())
        return;
    const uint8_t* infoStart = &_linkeditBias[_splitSegInfoCmd->dataoff()];
    const uint8_t* infoEnd = &infoStart[_splitSegInfoCmd->datasize()];
    if ( *infoStart++ != DYLD_CACHE_ADJ_V2_FORMAT ) {
        _diagnostics.error("malformed split seg info in %s", _installName);
        return;
    }

    uint8_t* textSectionContent = (uint8_t*)(_textSection->addr() + _cacheSlide);

    // Whole         :== <count> FromToSection+
    // FromToSection :== <from-sect-index> <to-sect-index> <count> ToOffset+
    // ToOffset         :== <to-sect-offset-delta> <count> FromOffset+
    // FromOffset     :== <kind> <count> <from-sect-offset-delta>
    const uint8_t* p = infoStart;
    uint64_t sectionCount = read_uleb128(p, infoEnd);
    for (uint64_t i=0; i < sectionCount; ++i) {
        uint64_t fromSectionIndex = read_uleb128(p, infoEnd);
        uint64_t toSectionIndex = read_uleb128(p, infoEnd);
        uint64_t toOffsetCount = read_uleb128(p, infoEnd);
        uint64_t toSectionOffset = 0;
        for (uint64_t j=0; j < toOffsetCount; ++j) {
            uint64_t toSectionDelta = read_uleb128(p, infoEnd);
            uint64_t fromOffsetCount = read_uleb128(p, infoEnd);
            toSectionOffset += toSectionDelta;
            for (uint64_t k=0; k < fromOffsetCount; ++k) {
                uint64_t kind = read_uleb128(p, infoEnd);
                if ( kind > 13 ) {
                    _diagnostics.error("bad kind (%llu) value in %s\n", kind, _installName);
                }
                uint64_t fromSectDeltaCount = read_uleb128(p, infoEnd);
                uint64_t fromSectionOffset = 0;
                for (uint64_t l=0; l < fromSectDeltaCount; ++l) {
                    uint64_t delta = read_uleb128(p, infoEnd);
                    fromSectionOffset += delta;
                    if ( (fromSectionIndex == _textSectionIndex) && (toSectionIndex == _stubSectionIndex) ) {
                        uint32_t* instrPtr = (uint32_t*)(textSectionContent + fromSectionOffset);
                        uint64_t instrAddr = _textSection->addr() + fromSectionOffset;
                        uint64_t stubAddr = _stubSection->addr() + toSectionOffset;
                        uint32_t instruction = E::get32(*instrPtr);
                        _branchToStubCount++;
                        if ( handler(kind, instrAddr, stubAddr, instruction) ) {
                            E::set32(*instrPtr, instruction);
                        }
                    }
                }
            }
        }
    }
}


/// Extract displacement from a thumb b/bl/blx instruction.
template <typename P>
int32_t StubOptimizer<P>::getDisplacementFromThumbBranch(uint32_t instruction, uint32_t instrAddr)
{
    bool is_blx    = ((instruction & 0xD000F800) == 0xC000F000);
    uint32_t s     = (instruction >> 10) & 0x1;
    uint32_t j1    = (instruction >> 29) & 0x1;
    uint32_t j2    = (instruction >> 27) & 0x1;
    uint32_t imm10 = instruction & 0x3FF;
    uint32_t imm11 = (instruction >> 16) & 0x7FF;
    uint32_t i1    = (j1 == s);
    uint32_t i2    = (j2 == s);
    uint32_t dis   = (s << 24) | (i1 << 23) | (i2 << 22) | (imm10 << 12) | (imm11 << 1);
    int32_t  sdis  = dis;
    int32_t result = s ? (sdis | 0xFE000000) : sdis;
    if ( is_blx && (instrAddr & 0x2) ) {
        // The thumb blx instruction always has low bit of imm11 as zero.  The way
        // a 2-byte aligned blx can branch to a 4-byte aligned ARM target is that
        // the blx instruction always 4-byte aligns the pc before adding the
        // displacement from the blx.  We must emulate that when decoding this.
        result -= 2;
    }
    return result;
}

/// Update a thumb b/bl/blx instruction, switching bl <-> blx as needed.
template <typename P>
uint32_t StubOptimizer<P>::setDisplacementInThumbBranch(uint32_t instruction,  uint32_t instrAddr,
                                                        int32_t displacement, bool targetIsThumb) {
    if ( (displacement > 16777214) || (displacement < (-16777216)) ) {
        _diagnostics.error("thumb branch out of range at 0x%0X in %s", instrAddr, _installName);
        return 0;
    }
    bool is_bl = ((instruction & 0xD000F800) == 0xD000F000);
    bool is_blx = ((instruction & 0xD000F800) == 0xC000F000);
    bool is_b = ((instruction & 0xD000F800) == 0x9000F000);
    uint32_t newInstruction = (instruction & 0xD000F800);
    if (is_bl || is_blx) {
        if (targetIsThumb) {
            newInstruction = 0xD000F000; // Use bl
        }
        else {
            newInstruction = 0xC000F000; // Use blx
            // See note in getDisplacementFromThumbBranch() about blx.
            if (instrAddr & 0x2)
                displacement += 2;
        }
    }
    else if (is_b) {
        if ( !targetIsThumb ) {
            _diagnostics.error("no pc-rel thumb branch instruction that switches to arm mode at 0x%0X in %s", instrAddr, _installName);
            return 0;
        }
    }
    else {
        _diagnostics.error("not b/bl/blx at 0x%0X in %s", instrAddr, _installName);
        return 0;
    }
    uint32_t s = (uint32_t)(displacement >> 24) & 0x1;
    uint32_t i1 = (uint32_t)(displacement >> 23) & 0x1;
    uint32_t i2 = (uint32_t)(displacement >> 22) & 0x1;
    uint32_t imm10 = (uint32_t)(displacement >> 12) & 0x3FF;
    uint32_t imm11 = (uint32_t)(displacement >> 1) & 0x7FF;
    uint32_t j1 = (i1 == s);
    uint32_t j2 = (i2 == s);
    uint32_t nextDisp = (j1 << 13) | (j2 << 11) | imm11;
    uint32_t firstDisp = (s << 10) | imm10;
    newInstruction |= (nextDisp << 16) | firstDisp;
    return newInstruction;
}


template <typename P>
void StubOptimizer<P>::optimizeArmCallSites(std::unordered_map<uint64_t, uint64_t>& targetAddrToOptStubAddr)
{
    forEachCallSiteToAStub([&](uint8_t kind, uint64_t callSiteAddr, uint64_t stubAddr, uint32_t& instruction) -> bool {
        if ( kind == DYLD_CACHE_ADJ_V2_THUMB_BR22 ) {
            bool is_bl = ((instruction & 0xD000F800) == 0xD000F000);
            bool is_blx = ((instruction & 0xD000F800) == 0xC000F000);
            bool is_b = ((instruction & 0xD000F800) == 0x9000F000);
            if ( !is_bl && !is_blx && !is_b ){
                _diagnostics.warning("non-branch instruction at 0x%0llX in %s", callSiteAddr, _installName);
                return false;
            }
            int32_t brDelta = getDisplacementFromThumbBranch(instruction, (uint32_t)callSiteAddr);
            pint_t targetAddr = (pint_t)callSiteAddr + 4 + brDelta;
            if ( targetAddr != stubAddr ) {
                _diagnostics.warning("stub target mismatch at callsite 0x%0llX in %s", callSiteAddr, _installName);
                return false;
            }
            // ignore branch if not to a known stub
            const auto& pos = _stubAddrToLPAddr.find(targetAddr);
            if ( pos == _stubAddrToLPAddr.end() )
                return false;

            // ignore branch if lazy pointer is not known (resolver or interposable)
            uint64_t lpAddr = pos->second;
            const auto& pos2 = _lpAddrToTargetAddr.find((pint_t)lpAddr);
            if ( pos2 == _lpAddrToTargetAddr.end() )
                return false;

            uint64_t finalTargetAddr = pos2->second;
            int64_t deltaToFinalTarget = finalTargetAddr - (callSiteAddr + 4);
            // if final target within range, change to branch there directly
            if ( (deltaToFinalTarget > -b16MegLimit) && (deltaToFinalTarget < b16MegLimit) ) {
                bool targetIsThumb = (finalTargetAddr & 1);
                instruction = setDisplacementInThumbBranch(instruction, (uint32_t)callSiteAddr, (int32_t)deltaToFinalTarget, targetIsThumb);
                if (_diagnostics.hasError())
                    return false;
                _branchOptimizedToDirectCount++;
                return true;
            }

            // try to re-use an existing optimized stub
            const auto& pos3 = targetAddrToOptStubAddr.find(finalTargetAddr);
            if ( pos3 != targetAddrToOptStubAddr.end() ) {
                uint64_t existingStub = pos3->second;
                if ( existingStub != stubAddr ) {
                    int64_t deltaToOptStub = existingStub - (callSiteAddr + 4);
                    if ( (deltaToOptStub > -b16MegLimit) && (deltaToOptStub < b16MegLimit) ) {
                        bool targetIsThumb = (existingStub & 1);
                        instruction = setDisplacementInThumbBranch(instruction, (uint32_t)callSiteAddr, (int32_t)deltaToOptStub, targetIsThumb);
                        if (_diagnostics.hasError())
                            return false;
                        _branchToReUsedOptimizedStubCount++;
                        return true;
                    }
                }
            }

            // leave as BL to stub, but optimize the stub
            _stubsToOptimize.insert(stubAddr);
            targetAddrToOptStubAddr[finalTargetAddr] = stubAddr;
            _branchToOptimizedStubCount++;
            return false;
        }
        else if ( kind == DYLD_CACHE_ADJ_V2_ARM_BR24 ) {
            // too few of these to be worth trying to optimize
        }

        return false;
    });
    if (_diagnostics.hasError())
        return;
}


template <typename P>
void StubOptimizer<P>::optimizeArmStubs()
{
    for (const auto& stubEntry : _stubAddrToLPAddr) {
        pint_t stubVMAddr = stubEntry.first;
        pint_t lpVMAddr   = stubEntry.second;
        const auto& pos = _lpAddrToTargetAddr.find(lpVMAddr);
        if ( pos == _lpAddrToTargetAddr.end() )
            return;
        pint_t targetVMAddr = pos->second;

        int32_t delta = (int32_t)(targetVMAddr - (stubVMAddr + 12));
        uint32_t* stubInstructions = (uint32_t*)((uint8_t*)(long)stubVMAddr + _cacheSlide);
        assert(stubInstructions[0] == 0xe59fc004);
        stubInstructions[0] = 0xe59fc000;  //      ldr    ip, L0
        stubInstructions[1] = 0xe08ff00c;  //      add    pc, pc, ip
        stubInstructions[2] = delta;       // L0:  .long  xxxx
        stubInstructions[3] = 0xe7ffdefe;  //      trap
        _stubOptimizedCount++;
    }
}



template <typename P>
void StubOptimizer<P>::optimizeArm64Stubs()
{
    for (const uint64_t stubVMAddr : _stubsToOptimize ) {
        pint_t lpVMAddr   = _stubAddrToLPAddr[(pint_t)stubVMAddr];
        const auto& pos = _lpAddrToTargetAddr.find(lpVMAddr);
        if ( pos == _lpAddrToTargetAddr.end() )
            return;
        pint_t targetVMAddr = pos->second;

        int64_t adrpDelta = (targetVMAddr & -4096) - (stubVMAddr & -4096);
        // Note: ADRP/ADD can only span +/-4GB
        uint32_t* stubInstructions = (uint32_t*)((uint8_t*)(long)stubVMAddr + _cacheSlide);
        bool rightInstr1 = ((stubInstructions[0] & 0x9F00001F) == 0x90000010);   // ADRP X16, lp@page
        bool rightInstr2 = ((stubInstructions[1] & 0xFFC003FF) == 0xF9400210);   // LDR X16, [X16, lp@pageoff]
        bool rightInstr3 = (stubInstructions[2] == 0xD61F0200);                  // BR X16

        if ( rightInstr1 && rightInstr2 && rightInstr3 ) {
            uint32_t immhi   = (adrpDelta >> 9) & (0x00FFFFE0);
            uint32_t immlo   = (adrpDelta << 17) & (0x60000000);
            uint32_t newADRP = (0x90000010) | immlo | immhi;
            uint32_t off12   = (targetVMAddr & 0xFFF);
            uint32_t newADD  = (0x91000210) | (off12 << 10);

            stubInstructions[0] = newADRP;     //      ADRP   X16, target@page
            stubInstructions[1] = newADD;      //      ADD    X16, X16, target@pageoff
            stubInstructions[2] = 0xD61F0200;  //      BR     X16
            _stubOptimizedCount++;
        }
    }
}

#if SUPPORT_ARCH_arm64e
template <typename P>
void StubOptimizer<P>::optimizeArm64eStubs()
{
    for (const uint64_t stubVMAddr : _stubsToOptimize ) {
        pint_t lpVMAddr   = _stubAddrToLPAddr[(pint_t)stubVMAddr];
        const auto& pos = _lpAddrToTargetAddr.find(lpVMAddr);
        if ( pos == _lpAddrToTargetAddr.end() )
            return;
        pint_t targetVMAddr = pos->second;

        int64_t adrpDelta = (targetVMAddr & -4096) - (stubVMAddr & -4096);
        // Note: ADRP/ADD can only span +/-4GB
        uint32_t* stubInstructions = (uint32_t*)((uint8_t*)(long)stubVMAddr + _cacheSlide);
        bool rightInstr1 = ((stubInstructions[0] & 0x9F00001F) == 0x90000011);   // ADRP  X17, lp@page
        bool rightInstr2 = ((stubInstructions[1] & 0xFFC003FF) == 0x91000231);   // ADD   X17, [X17, lp@pageoff]
        bool rightInstr3 = (stubInstructions[2] == 0xF9400230);                  // LDR   X16, [X17]
        bool rightInstr4 = (stubInstructions[3] == 0xD71F0A11);                  // BRAA  X16, X17

        if ( rightInstr1 && rightInstr2 && rightInstr3 && rightInstr4) {
            uint32_t immhi   = (adrpDelta >> 9) & (0x00FFFFE0);
            uint32_t immlo   = (adrpDelta << 17) & (0x60000000);
            uint32_t newADRP = (0x90000010) | immlo | immhi;
            uint32_t off12   = (targetVMAddr & 0xFFF);
            uint32_t newADD  = (0x91000210) | (off12 << 10);

            stubInstructions[0] = newADRP;     //      ADRP   X16, target@page
            stubInstructions[1] = newADD;      //      ADD    X16, X16, target@pageoff
            stubInstructions[2] = 0xD61F0200;  //      BR     X16
            stubInstructions[3] = 0xD4200020;  //      TRAP
            _stubOptimizedCount++;
        }
    }
}
#endif

#if SUPPORT_ARCH_arm64_32
template <typename P>
void StubOptimizer<P>::optimizeArm64_32Stubs()
{
    for (const uint64_t stubVMAddr : _stubsToOptimize ) {
        pint_t lpVMAddr   = _stubAddrToLPAddr[(pint_t)stubVMAddr];
        const auto& pos = _lpAddrToTargetAddr.find(lpVMAddr);
        if ( pos == _lpAddrToTargetAddr.end() )
            return;
        pint_t targetVMAddr = pos->second;

        int64_t adrpDelta = (targetVMAddr & -4096) - (stubVMAddr & -4096);
        uint32_t* stubInstructions = (uint32_t*)((uint8_t*)(long)stubVMAddr + _cacheSlide);
        bool rightInstr1 = ((stubInstructions[0] & 0x9F00001F) == 0x90000010);   // ADRP X16, lp@page
        bool rightInstr2 = ((stubInstructions[1] & 0xFFC003FF) == 0xB9400210);   // LDR  W16, [X16, lp@pageoff]
        bool rightInstr3 = (stubInstructions[2] == 0xD61F0200);                  // BR   X16

        if ( rightInstr1 && rightInstr2 && rightInstr3 ) {
            uint32_t immhi   = (adrpDelta >> 9) & (0x00FFFFE0);
            uint32_t immlo   = (adrpDelta << 17) & (0x60000000);
            uint32_t newADRP = (0x90000010) | immlo | immhi;
            uint32_t off12   = (targetVMAddr & 0xFFF);
            uint32_t newADD  = (0x91000210) | (off12 << 10);

            stubInstructions[0] = newADRP;     //      ADRP   X16, target@page
            stubInstructions[1] = newADD;      //      ADD    X16, X16, target@pageoff
            stubInstructions[2] = 0xD61F0200;  //      BR     X16
            _stubOptimizedCount++;
        }
    }
}
#endif


template <typename P>
void StubOptimizer<P>::optimizeArm64CallSites(std::unordered_map<uint64_t, uint64_t>& targetAddrToOptStubAddr)
{
    forEachCallSiteToAStub([&](uint8_t kind, uint64_t callSiteAddr, uint64_t stubAddr, uint32_t& instruction) -> bool {
        if ( kind != DYLD_CACHE_ADJ_V2_ARM64_BR26 )
            return false;
        // skip all but BL or B
        if ( (instruction & 0x7C000000) != 0x14000000 )
            return false;
        // compute target of branch instruction
        int32_t brDelta = (instruction & 0x03FFFFFF) << 2;
        if ( brDelta & 0x08000000 )
            brDelta |= 0xF0000000;
        uint64_t targetAddr = callSiteAddr + (int64_t)brDelta;
        if ( targetAddr != stubAddr ) {
            _diagnostics.warning("stub target mismatch");
            return false;
        }
        // ignore branch if not to a known stub
        const auto& pos = _stubAddrToLPAddr.find((pint_t)targetAddr);
        if ( pos == _stubAddrToLPAddr.end() )
            return false;

        // ignore branch if lazy pointer is not known (resolver or interposable)
        uint64_t lpAddr = pos->second;
        const auto& pos2 = _lpAddrToTargetAddr.find((pint_t)lpAddr);
        if ( pos2 == _lpAddrToTargetAddr.end() )
            return false;

        uint64_t finalTargetAddr = pos2->second;
        int64_t deltaToFinalTarget = finalTargetAddr - callSiteAddr;
        // if final target within range, change to branch there directly
        if ( (deltaToFinalTarget > -b128MegLimit) && (deltaToFinalTarget < b128MegLimit) ) {
            instruction= (instruction & 0xFC000000) | ((deltaToFinalTarget >> 2) & 0x03FFFFFF);
            _branchOptimizedToDirectCount++;
            return true;
        }

        // try to re-use an existing optimized stub
        const auto& pos3 = targetAddrToOptStubAddr.find((pint_t)finalTargetAddr);
        if ( pos3 != targetAddrToOptStubAddr.end() ) {
            uint64_t existingStub = pos3->second;
            if ( existingStub != stubAddr ) {
                int64_t deltaToOptStub = existingStub - callSiteAddr;
                if ( (deltaToOptStub > -b128MegLimit) && (deltaToOptStub < b128MegLimit) ) {
                    instruction = (instruction & 0xFC000000) | ((deltaToOptStub >> 2) & 0x03FFFFFF);
                    _branchToReUsedOptimizedStubCount++;
                    return true;
                }
            }
        }

        // leave as BL to stub, but optimize the stub
        _stubsToOptimize.insert(stubAddr);
        targetAddrToOptStubAddr[(pint_t)finalTargetAddr] = (pint_t)stubAddr;
        _branchToOptimizedStubCount++;
        return false;
    });
    if (_diagnostics.hasError())
        return;
}


template <typename P>
void StubOptimizer<P>::optimizeCallSites(std::unordered_map<uint64_t, uint64_t>& targetAddrToOptStubAddr)
{
    if ( _textSection == NULL )
        return;
    if ( _stubSection == NULL )
        return;


    switch ( _mh->cputype() ) {
        case CPU_TYPE_ARM64:
            optimizeArm64CallSites(targetAddrToOptStubAddr);
#if SUPPORT_ARCH_arm64e
            if (_mh->cpusubtype() == CPU_SUBTYPE_ARM64E)
                optimizeArm64eStubs();
            else
#endif
                optimizeArm64Stubs();
            break;
#if SUPPORT_ARCH_arm64_32
        case CPU_TYPE_ARM64_32:
            optimizeArm64CallSites(targetAddrToOptStubAddr);
            optimizeArm64_32Stubs();
            break;
#endif
        case CPU_TYPE_ARM:
            optimizeArmCallSites(targetAddrToOptStubAddr);
            optimizeArmStubs();
            break;
    }
    if ( verbose ) {
        _diagnostics.verbose("dylib has %6u BLs to %4u stubs. Changed %5u, %5u, %5u BLs to use direct branch, optimized stub, neighbor's optimized stub. "
                             "%5u stubs left interposable, %4u stubs optimized. path=%s\n",
                            _branchToStubCount, _stubCount, _branchOptimizedToDirectCount, _branchToOptimizedStubCount, _branchToReUsedOptimizedStubCount,
                            _stubsLeftInterposable, _stubOptimizedCount, _installName);
    }

}

template <typename P>
void bypassStubs(DyldSharedCache* cache, const std::string& archName, std::unordered_map<uint64_t, uint64_t>& targetAddrToOptStubAddr,
                 const char* const neverStubEliminateDylibs[], const char* const neverStubEliminateSymbols[],
                 Diagnostics& diags)
{
    diags.verbose("Stub elimination optimization:\n");

    // construct a StubOptimizer for each image
    __block std::vector<StubOptimizer<P>*> optimizers;
    cache->forEachImage(^(const mach_header* mh, const char* installName) {
        optimizers.push_back(new StubOptimizer<P>(cache, (macho_header<P>*)mh, diags));
    });

    // build set of functions to never stub-eliminate because tools may need to override them
    std::unordered_set<std::string> neverStubEliminate;
    for (const char* const* p=neverStubEliminateSymbols; *p != nullptr; ++p) {
        neverStubEliminate.insert(*p);
    }
    for (const char* const* d=neverStubEliminateDylibs; *d != nullptr; ++d) {
        for (StubOptimizer<P>* op : optimizers) {
            if ( strcmp(op->installName(), *d) == 0 ) {
                // add all exports
                const uint8_t* exportsStart = op->exportsTrie();
                const uint8_t* exportsEnd = exportsStart + op->exportsTrieSize();
                std::vector<ExportInfoTrie::Entry> exports;
                if ( !ExportInfoTrie::parseTrie(exportsStart, exportsEnd, exports) ) {
                    diags.error("malformed exports trie in %s", *d);
                    return;
                }
                for(const ExportInfoTrie::Entry& entry : exports) {
                    neverStubEliminate.insert(entry.name);
               }
            }
        }
    }

    // build maps of stubs-to-lp and lp-to-target
    for (StubOptimizer<P>* op : optimizers)
        op->buildStubMap(neverStubEliminate);

    // optimize call sites to by-pass stubs or jump through island
    for (StubOptimizer<P>* op : optimizers)
        op->optimizeCallSites(targetAddrToOptStubAddr);

    // write total optimization info
    uint32_t callSiteCount = 0;
    uint32_t callSiteDirectOptCount = 0;
    for (StubOptimizer<P>* op : optimizers) {
        callSiteCount           += op->_branchToStubCount;
        callSiteDirectOptCount  += op->_branchOptimizedToDirectCount;
    }
    diags.verbose("  cache contains %u call sites of which %u were direct bound\n", callSiteCount, callSiteDirectOptCount);

    // clean up
    for (StubOptimizer<P>* op : optimizers)
        delete op;
}

void CacheBuilder::optimizeAwayStubs()
{
    std::unordered_map<uint64_t, uint64_t> targetAddrToOptStubAddr;

    DyldSharedCache* dyldCache = (DyldSharedCache*)_readExecuteRegion.buffer;
    std::string archName = dyldCache->archName();
#if SUPPORT_ARCH_arm64_32
    if ( startsWith(archName, "arm64_32") )
        bypassStubs<Pointer32<LittleEndian> >(dyldCache, archName, targetAddrToOptStubAddr, _s_neverStubEliminateDylibs, _s_neverStubEliminateSymbols, _diagnostics);
    else
#endif
    if ( startsWith(archName, "arm64") )
        bypassStubs<Pointer64<LittleEndian> >(dyldCache, archName, targetAddrToOptStubAddr, _s_neverStubEliminateDylibs, _s_neverStubEliminateSymbols, _diagnostics);
    else if ( archName == "armv7k" )
        bypassStubs<Pointer32<LittleEndian>>(dyldCache, archName, targetAddrToOptStubAddr, _s_neverStubEliminateDylibs, _s_neverStubEliminateSymbols, _diagnostics);
    // no stub optimization done for other arches
}

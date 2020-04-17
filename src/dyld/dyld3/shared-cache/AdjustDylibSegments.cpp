/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- 
 *
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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


#include <dirent.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <assert.h>

#include <fstream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "CacheBuilder.h"
#include "Diagnostics.h"
#include "DyldSharedCache.h"
#include "Trie.hpp"
#include "MachOFileAbstraction.hpp"
#include "MachOLoaded.h"
#include "MachOAnalyzer.h"


#ifndef EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE
    #define EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE 0x02
#endif

namespace {

template <typename P>
class Adjustor {
public:
                    Adjustor(DyldSharedCache* cacheBuffer, macho_header<P>* mh, const std::vector<CacheBuilder::SegmentMappingInfo>& mappingInfo, Diagnostics& diag);
    void            adjustImageForNewSegmentLocations(CacheBuilder::ASLR_Tracker& aslrTracker,
                                                      CacheBuilder::LOH_Tracker& lohTracker,
                                                      const CacheBuilder::CacheCoalescedText& coalescedText,
                                                      const CacheBuilder::DylibTextCoalescer& textCoalescer);
 
private:
    void            adjustReferencesUsingInfoV2(CacheBuilder::ASLR_Tracker& aslrTracker, CacheBuilder::LOH_Tracker& lohTracker,
                                                const CacheBuilder::CacheCoalescedText& coalescedText,
                                                const CacheBuilder::DylibTextCoalescer& textCoalescer);
    void            adjustReference(uint32_t kind, uint8_t* mappedAddr, uint64_t fromNewAddress, uint64_t toNewAddress, int64_t adjust, int64_t targetSlide,
                                    uint64_t imageStartAddress, uint64_t imageEndAddress,
                                    CacheBuilder::ASLR_Tracker& aslrTracker, CacheBuilder::LOH_Tracker* lohTracker,
                                    uint32_t*& lastMappedAddr32, uint32_t& lastKind, uint64_t& lastToNewAddress);
    void            adjustDataPointers(CacheBuilder::ASLR_Tracker& aslrTracker);
    void            slidePointer(int segIndex, uint64_t segOffset, uint8_t type, CacheBuilder::ASLR_Tracker& aslrTracker);
    void            adjustSymbolTable();
    void            adjustChainedFixups();
    void            adjustExportsTrie(std::vector<uint8_t>& newTrieBytes);
    void            rebuildLinkEdit();
    void            adjustCode();
    void            adjustInstruction(uint8_t kind, uint8_t* textLoc, uint64_t codeToDataDelta);
    void            rebuildLinkEditAndLoadCommands(const CacheBuilder::DylibTextCoalescer& textCoalescer);
    uint64_t        slideForOrigAddress(uint64_t addr);

    typedef typename P::uint_t pint_t;
    typedef typename P::E E;

    DyldSharedCache*                                        _cacheBuffer;
    macho_header<P>*                                        _mh;
    Diagnostics&                                            _diagnostics;
    const uint8_t*                                          _linkeditBias       = nullptr;
    unsigned                                                _linkeditSegIndex   = 0;
    bool                                                    _maskPointers       = false;
    bool                                                    _splitSegInfoV2     = false;
    const char*                                             _installName        = nullptr;
    macho_symtab_command<P>*                                _symTabCmd          = nullptr;
    macho_dysymtab_command<P>*                              _dynSymTabCmd       = nullptr;
    macho_dyld_info_command<P>*                             _dyldInfo           = nullptr;
    macho_linkedit_data_command<P>*                         _splitSegInfoCmd    = nullptr;
    macho_linkedit_data_command<P>*                         _functionStartsCmd  = nullptr;
    macho_linkedit_data_command<P>*                         _dataInCodeCmd      = nullptr;
    macho_linkedit_data_command<P>*                         _exportTrieCmd      = nullptr;
    macho_linkedit_data_command<P>*                         _chainedFixupsCmd   = nullptr;
    std::vector<uint64_t>                                   _segOrigStartAddresses;
    std::vector<uint64_t>                                   _segSlides;
    std::vector<macho_segment_command<P>*>                  _segCmds;
    const std::vector<CacheBuilder::SegmentMappingInfo>&    _mappingInfo;
};

template <typename P>
Adjustor<P>::Adjustor(DyldSharedCache* cacheBuffer, macho_header<P>* mh, const std::vector<CacheBuilder::SegmentMappingInfo>& mappingInfo, Diagnostics& diag)
    : _cacheBuffer(cacheBuffer), _mh(mh), _diagnostics(diag), _mappingInfo(mappingInfo)
{
    assert((mh->magic() == MH_MAGIC) || (mh->magic() == MH_MAGIC_64));
    macho_segment_command<P>* segCmd;
    const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)mh + sizeof(macho_header<P>));
    const uint32_t cmd_count = mh->ncmds();
    const macho_load_command<P>* cmd = cmds;
    unsigned segIndex = 0;
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
            case LC_DYLD_INFO:
            case LC_DYLD_INFO_ONLY:
                _dyldInfo = (macho_dyld_info_command<P>*)cmd;
                break;
            case LC_SEGMENT_SPLIT_INFO:
                _splitSegInfoCmd = (macho_linkedit_data_command<P>*)cmd;
                break;
            case LC_FUNCTION_STARTS:
                _functionStartsCmd = (macho_linkedit_data_command<P>*)cmd;
                break;
            case LC_DATA_IN_CODE:
                _dataInCodeCmd = (macho_linkedit_data_command<P>*)cmd;
                break;
            case LC_DYLD_CHAINED_FIXUPS:
                _chainedFixupsCmd = (macho_linkedit_data_command<P>*)cmd;
                break;
            case LC_DYLD_EXPORTS_TRIE:
                _exportTrieCmd = (macho_linkedit_data_command<P>*)cmd;
                break;
            case macho_segment_command<P>::CMD:
                segCmd = (macho_segment_command<P>*)cmd;
                _segCmds.push_back(segCmd);
                _segOrigStartAddresses.push_back(segCmd->vmaddr());
                _segSlides.push_back(_mappingInfo[segIndex].dstCacheUnslidAddress - segCmd->vmaddr());
                if ( strcmp(segCmd->segname(), "__LINKEDIT") == 0 ) {
                    _linkeditBias = (uint8_t*)_mappingInfo[segIndex].dstSegment - segCmd->fileoff();
                    _linkeditSegIndex = segIndex;
                }
                ++segIndex;
                break;
        }
        cmd = (const macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
    }
    _maskPointers = (P::E::get32(mh->cputype()) == CPU_TYPE_ARM64) || (P::E::get32(mh->cputype()) == CPU_TYPE_ARM64_32);
    if ( _splitSegInfoCmd != NULL ) {
        const uint8_t* infoStart = &_linkeditBias[_splitSegInfoCmd->dataoff()];
        _splitSegInfoV2 = (*infoStart == DYLD_CACHE_ADJ_V2_FORMAT);
    }
    else {
        _diagnostics.error("missing LC_SEGMENT_SPLIT_INFO in %s", _installName);
    }
}

template <typename P>
void Adjustor<P>::adjustImageForNewSegmentLocations(CacheBuilder::ASLR_Tracker& aslrTracker,
                                                    CacheBuilder::LOH_Tracker& lohTracker,
                                                    const CacheBuilder::CacheCoalescedText& coalescedText,
                                                    const CacheBuilder::DylibTextCoalescer& textCoalescer)
{
    if ( _diagnostics.hasError() )
        return;
    if ( _splitSegInfoV2 ) {
        adjustReferencesUsingInfoV2(aslrTracker, lohTracker, coalescedText, textCoalescer);
    }
    else {
        adjustDataPointers(aslrTracker);
        adjustCode();
    }
    if ( _diagnostics.hasError() )
        return;
    adjustSymbolTable();
    if ( _diagnostics.hasError() )
        return;
    adjustChainedFixups();
    if ( _diagnostics.hasError() )
        return;
    rebuildLinkEditAndLoadCommands(textCoalescer);

#if DEBUG
    Diagnostics  diag;
    ((dyld3::MachOAnalyzer*)_mh)->validateDyldCacheDylib(diag, _installName);
    if ( diag.hasError() ) {
        fprintf(stderr, "%s\n", diag.errorMessage().c_str());
    }
#endif
}

template <typename P>
uint64_t Adjustor<P>::slideForOrigAddress(uint64_t addr)
{
    for (unsigned i=0; i < _segOrigStartAddresses.size(); ++i) {
        if ( (_segOrigStartAddresses[i] <= addr) && (addr < (_segOrigStartAddresses[i]+_segCmds[i]->vmsize())) )
            return _segSlides[i];
    }
    // On arm64, high nibble of pointers can have extra bits
    if ( _maskPointers && (addr & 0xF000000000000000) ) {
        return slideForOrigAddress(addr & 0x0FFFFFFFFFFFFFFF);
    }
    _diagnostics.error("slide not known for dylib address 0x%llX in %s", addr, _installName);
    return 0;
}

template <typename P>
void Adjustor<P>::rebuildLinkEditAndLoadCommands(const CacheBuilder::DylibTextCoalescer& textCoalescer)
{
    // Exports trie is only data structure in LINKEDIT that might grow
    std::vector<uint8_t> newTrieBytes;
    adjustExportsTrie(newTrieBytes);

    // Remove: code signature, rebase info, code-sign-dirs, split seg info
    uint32_t chainedFixupsOffset = 0;
    uint32_t chainedFixupsSize   = _chainedFixupsCmd ? _chainedFixupsCmd->datasize() : 0;
    uint32_t bindOffset          = chainedFixupsOffset + chainedFixupsSize;
    uint32_t bindSize            = _dyldInfo ? _dyldInfo->bind_size() : 0;
    uint32_t weakBindOffset      = bindOffset + bindSize;
    uint32_t weakBindSize        = _dyldInfo ? _dyldInfo->weak_bind_size() : 0;
    uint32_t lazyBindOffset      = weakBindOffset + weakBindSize;
    uint32_t lazyBindSize        = _dyldInfo ? _dyldInfo->lazy_bind_size() : 0;
    uint32_t exportOffset        = lazyBindOffset + lazyBindSize;
    uint32_t exportSize          = (uint32_t)newTrieBytes.size();
    uint32_t splitSegInfoOffset  = exportOffset + exportSize;
    uint32_t splitSegInfosSize   = (_splitSegInfoCmd ? _splitSegInfoCmd->datasize() : 0);
    uint32_t funcStartsOffset    = splitSegInfoOffset + splitSegInfosSize;
    uint32_t funcStartsSize      = (_functionStartsCmd ? _functionStartsCmd->datasize() : 0);
    uint32_t dataInCodeOffset    = funcStartsOffset + funcStartsSize;
    uint32_t dataInCodeSize      = (_dataInCodeCmd ? _dataInCodeCmd->datasize() : 0);
    uint32_t symbolTableOffset   = dataInCodeOffset + dataInCodeSize;
    uint32_t symbolTableSize     = _symTabCmd->nsyms() * sizeof(macho_nlist<P>);
    uint32_t indirectTableOffset = symbolTableOffset + symbolTableSize;
    uint32_t indirectTableSize   = _dynSymTabCmd->nindirectsyms() * sizeof(uint32_t);
    uint32_t symbolStringsOffset = indirectTableOffset + indirectTableSize;
    uint32_t symbolStringsSize   = _symTabCmd->strsize();
    uint32_t newLinkEditSize     = symbolStringsOffset + symbolStringsSize;

    size_t linkeditBufferSize = align(_segCmds[_linkeditSegIndex]->vmsize(), 12);
    if ( linkeditBufferSize < newLinkEditSize ) {
        _diagnostics.error("LINKEDIT overflow in %s", _installName);
        return;
    }

    uint8_t* newLinkeditBufer = (uint8_t*)::calloc(linkeditBufferSize, 1);
    if ( chainedFixupsSize )
        memcpy(&newLinkeditBufer[chainedFixupsOffset], &_linkeditBias[_chainedFixupsCmd->dataoff()], chainedFixupsSize);
    if ( bindSize )
        memcpy(&newLinkeditBufer[bindOffset], &_linkeditBias[_dyldInfo->bind_off()], bindSize);
    if ( lazyBindSize )
        memcpy(&newLinkeditBufer[lazyBindOffset], &_linkeditBias[_dyldInfo->lazy_bind_off()], lazyBindSize);
    if ( weakBindSize )
        memcpy(&newLinkeditBufer[weakBindOffset], &_linkeditBias[_dyldInfo->weak_bind_off()], weakBindSize);
    if ( exportSize )
        memcpy(&newLinkeditBufer[exportOffset], &newTrieBytes[0], exportSize);
    if ( splitSegInfosSize )
        memcpy(&newLinkeditBufer[splitSegInfoOffset], &_linkeditBias[_splitSegInfoCmd->dataoff()], splitSegInfosSize);
    if ( funcStartsSize )
        memcpy(&newLinkeditBufer[funcStartsOffset], &_linkeditBias[_functionStartsCmd->dataoff()], funcStartsSize);
    if ( dataInCodeSize )
        memcpy(&newLinkeditBufer[dataInCodeOffset], &_linkeditBias[_dataInCodeCmd->dataoff()], dataInCodeSize);
    if ( symbolTableSize )
        memcpy(&newLinkeditBufer[symbolTableOffset], &_linkeditBias[_symTabCmd->symoff()], symbolTableSize);
    if ( indirectTableSize )
        memcpy(&newLinkeditBufer[indirectTableOffset], &_linkeditBias[_dynSymTabCmd->indirectsymoff()], indirectTableSize);
    if ( symbolStringsSize )
        memcpy(&newLinkeditBufer[symbolStringsOffset], &_linkeditBias[_symTabCmd->stroff()], symbolStringsSize);

    memcpy(_mappingInfo[_linkeditSegIndex].dstSegment, newLinkeditBufer, newLinkEditSize);
    ::bzero(((uint8_t*)_mappingInfo[_linkeditSegIndex].dstSegment)+newLinkEditSize, linkeditBufferSize-newLinkEditSize);
    ::free(newLinkeditBufer);
    uint32_t linkeditStartOffset = (uint32_t)_mappingInfo[_linkeditSegIndex].dstCacheFileOffset;

    // updates load commands and removed ones no longer needed
    macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)_mh + sizeof(macho_header<P>));
    uint32_t cmd_count = _mh->ncmds();
    const macho_load_command<P>* cmd = cmds;
    const unsigned origLoadCommandsSize = _mh->sizeofcmds();
    unsigned bytesRemaining = origLoadCommandsSize;
    unsigned removedCount = 0;
    unsigned segIndex = 0;
    for (uint32_t i = 0; i < cmd_count; ++i) {
        macho_symtab_command<P>*           symTabCmd;
        macho_dysymtab_command<P>*         dynSymTabCmd;
        macho_dyld_info_command<P>*        dyldInfo;
        macho_linkedit_data_command<P>*    functionStartsCmd;
        macho_linkedit_data_command<P>*    dataInCodeCmd;
        macho_linkedit_data_command<P>*    chainedFixupsCmd;
        macho_linkedit_data_command<P>*    exportTrieCmd;
        macho_linkedit_data_command<P>*    splitSegInfoCmd;
        macho_segment_command<P>*          segCmd;
        macho_routines_command<P>*         routinesCmd;
        macho_dylib_command<P>*            dylibIDCmd;
        uint32_t cmdSize = cmd->cmdsize();
        int32_t segFileOffsetDelta;
        bool remove = false;
        switch ( cmd->cmd() ) {
            case LC_ID_DYLIB:
                dylibIDCmd = (macho_dylib_command<P>*)cmd;
                dylibIDCmd->set_timestamp(2); // match what static linker sets in LC_LOAD_DYLIB
                break;
            case LC_SYMTAB:
                symTabCmd = (macho_symtab_command<P>*)cmd;
                symTabCmd->set_symoff(linkeditStartOffset+symbolTableOffset);
                symTabCmd->set_stroff(linkeditStartOffset+symbolStringsOffset);
              break;
            case LC_DYSYMTAB:
                dynSymTabCmd = (macho_dysymtab_command<P>*)cmd;
                dynSymTabCmd->set_indirectsymoff(linkeditStartOffset+indirectTableOffset);
                break;
            case LC_DYLD_INFO:
            case LC_DYLD_INFO_ONLY:
                dyldInfo = (macho_dyld_info_command<P>*)cmd;
                dyldInfo->set_rebase_off(0);
                dyldInfo->set_rebase_size(0);
                dyldInfo->set_bind_off(bindSize ? linkeditStartOffset+bindOffset : 0);
                dyldInfo->set_bind_size(bindSize);
                dyldInfo->set_weak_bind_off(weakBindSize ? linkeditStartOffset+weakBindOffset : 0);
                dyldInfo->set_weak_bind_size(weakBindSize);
                dyldInfo->set_lazy_bind_off(lazyBindSize ? linkeditStartOffset+lazyBindOffset : 0);
                dyldInfo->set_lazy_bind_size(lazyBindSize);
                dyldInfo->set_export_off(exportSize ? linkeditStartOffset+exportOffset : 0);
                dyldInfo->set_export_size(exportSize);
                break;
            case LC_FUNCTION_STARTS:
                functionStartsCmd = (macho_linkedit_data_command<P>*)cmd;
                functionStartsCmd->set_dataoff(linkeditStartOffset+funcStartsOffset);
               break;
            case LC_DATA_IN_CODE:
                dataInCodeCmd = (macho_linkedit_data_command<P>*)cmd;
                dataInCodeCmd->set_dataoff(linkeditStartOffset+dataInCodeOffset);
                break;
            case LC_DYLD_CHAINED_FIXUPS:
                chainedFixupsCmd = (macho_linkedit_data_command<P>*)cmd;
                chainedFixupsCmd->set_dataoff(chainedFixupsSize ? linkeditStartOffset+chainedFixupsOffset : 0);
                chainedFixupsCmd->set_datasize(chainedFixupsSize);
                break;
            case LC_DYLD_EXPORTS_TRIE:
                exportTrieCmd = (macho_linkedit_data_command<P>*)cmd;
                exportTrieCmd->set_dataoff(exportSize ? linkeditStartOffset+exportOffset : 0);
                exportTrieCmd->set_datasize(exportSize);
                break;
            case macho_routines_command<P>::CMD:
                routinesCmd = (macho_routines_command<P>*)cmd;
                routinesCmd->set_init_address(routinesCmd->init_address()+slideForOrigAddress(routinesCmd->init_address()));
                break;
            case macho_segment_command<P>::CMD:
                segCmd = (macho_segment_command<P>*)cmd;
                segFileOffsetDelta = (int32_t)(_mappingInfo[segIndex].dstCacheFileOffset - segCmd->fileoff());
                segCmd->set_vmaddr(_mappingInfo[segIndex].dstCacheUnslidAddress);
                segCmd->set_vmsize(_mappingInfo[segIndex].dstCacheSegmentSize);
                segCmd->set_fileoff(_mappingInfo[segIndex].dstCacheFileOffset);
                segCmd->set_filesize(_mappingInfo[segIndex].dstCacheFileSize);
                if ( strcmp(segCmd->segname(), "__LINKEDIT") == 0 )
                    segCmd->set_vmsize(linkeditBufferSize);
                if ( segCmd->nsects() > 0 ) {
                    macho_section<P>* const sectionsStart = (macho_section<P>*)((uint8_t*)segCmd + sizeof(macho_segment_command<P>));
                    macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];

                    for (macho_section<P>*  sect=sectionsStart; sect < sectionsEnd; ++sect) {
                        if ( (strcmp(segCmd->segname(), "__TEXT") == 0) && textCoalescer.sectionWasCoalesced(sect->sectname())) {
                            // Put coalesced sections at the end of the segment
                            sect->set_addr(segCmd->vmaddr() + segCmd->filesize());
                            sect->set_offset(0);
                            sect->set_size(0);
                        } else {
                            sect->set_addr(sect->addr() + _segSlides[segIndex]);
                            if ( sect->offset() != 0 )
                                sect->set_offset(sect->offset() + segFileOffsetDelta);
                        }
                    }
                }
                ++segIndex;
                break;
             case LC_RPATH:
                _diagnostics.warning("dyld shared cache does not support LC_RPATH found in %s", _installName);
                remove = true;
                break;
            case LC_SEGMENT_SPLIT_INFO:
                splitSegInfoCmd = (macho_linkedit_data_command<P>*)cmd;
                splitSegInfoCmd->set_dataoff(linkeditStartOffset+splitSegInfoOffset);
                break;
            case LC_CODE_SIGNATURE:
            case LC_DYLIB_CODE_SIGN_DRS:
                remove = true;
                break;
            default:
                break;
        }
        macho_load_command<P>* nextCmd = (macho_load_command<P>*)(((uint8_t*)cmd)+cmdSize);
        if ( remove ) {
            ::memmove((void*)cmd, (void*)nextCmd, bytesRemaining);
            ++removedCount;
        }
        else {
            bytesRemaining -= cmdSize;
            cmd = nextCmd;
        }
    }
    // zero out stuff removed
    ::bzero((void*)cmd, bytesRemaining);
    // update header
    _mh->set_ncmds(cmd_count-removedCount);
    _mh->set_sizeofcmds(origLoadCommandsSize-bytesRemaining);
    _mh->set_flags(_mh->flags() | 0x80000000);
}


template <typename P>
void Adjustor<P>::adjustSymbolTable()
{
    macho_nlist<P>*  symbolTable = (macho_nlist<P>*)&_linkeditBias[_symTabCmd->symoff()];

    // adjust global symbol table entries
    macho_nlist<P>* lastExport = &symbolTable[_dynSymTabCmd->iextdefsym()+_dynSymTabCmd->nextdefsym()];
    for (macho_nlist<P>* entry = &symbolTable[_dynSymTabCmd->iextdefsym()]; entry < lastExport; ++entry) {
        if ( (entry->n_type() & N_TYPE) == N_SECT )
            entry->set_n_value(entry->n_value() + slideForOrigAddress(entry->n_value()));
    }

    // adjust local symbol table entries
    macho_nlist<P>*  lastLocal = &symbolTable[_dynSymTabCmd->ilocalsym()+_dynSymTabCmd->nlocalsym()];
    for (macho_nlist<P>* entry = &symbolTable[_dynSymTabCmd->ilocalsym()]; entry < lastLocal; ++entry) {
        if ( (entry->n_sect() != NO_SECT) && ((entry->n_type() & N_STAB) == 0) )
            entry->set_n_value(entry->n_value() + slideForOrigAddress(entry->n_value()));
    }
}


template <typename P>
void Adjustor<P>::adjustChainedFixups()
{
    if ( _chainedFixupsCmd == nullptr )
        return;

    // Pass a start hint in to withChainStarts which takes account of the LINKEDIT shifting but we haven't
    // yet updated that LC_SEGMENT to point to the new data
    const dyld_chained_fixups_header* header = (dyld_chained_fixups_header*)&_linkeditBias[_chainedFixupsCmd->dataoff()];
    uint64_t startsOffset = ((uint64_t)header + header->starts_offset) - (uint64_t)_mh;

    // segment_offset in dyld_chained_starts_in_segment is wrong.  We need to move it to the new segment offset
    ((dyld3::MachOAnalyzer*)_mh)->withChainStarts(_diagnostics, startsOffset, ^(const dyld_chained_starts_in_image* starts) {
        for (uint32_t segIndex=0; segIndex < starts->seg_count; ++segIndex) {
            if ( starts->seg_info_offset[segIndex] == 0 )
                continue;
            dyld_chained_starts_in_segment* segInfo = (dyld_chained_starts_in_segment*)((uint8_t*)starts + starts->seg_info_offset[segIndex]);
            segInfo->segment_offset = (uint64_t)_mappingInfo[segIndex].dstSegment - (uint64_t)_mh;
        }
    });
}

template <typename P>
void Adjustor<P>::slidePointer(int segIndex, uint64_t segOffset, uint8_t type, CacheBuilder::ASLR_Tracker& aslrTracker)
{
    pint_t*   mappedAddrP  = (pint_t*)((uint8_t*)_mappingInfo[segIndex].dstSegment + segOffset);
    uint32_t* mappedAddr32 = (uint32_t*)mappedAddrP;
    pint_t    valueP;
    uint32_t  value32;
    switch ( type ) {
        case REBASE_TYPE_POINTER:
            valueP = (pint_t)P::getP(*mappedAddrP);
            P::setP(*mappedAddrP, valueP + slideForOrigAddress(valueP));
            aslrTracker.add(mappedAddrP);
            break;
        
        case REBASE_TYPE_TEXT_ABSOLUTE32:
            value32 = P::E::get32(*mappedAddr32);
            P::E::set32(*mappedAddr32, value32 + (uint32_t)slideForOrigAddress(value32));
            break;

        case REBASE_TYPE_TEXT_PCREL32:
            // general text relocs not support
        default:
            _diagnostics.error("unknown rebase type 0x%02X in %s", type, _installName);
    }
}


static bool isThumbMovw(uint32_t instruction)
{
    return ( (instruction & 0x8000FBF0) == 0x0000F240 );
}

static bool isThumbMovt(uint32_t instruction)
{
    return ( (instruction & 0x8000FBF0) == 0x0000F2C0 );
}

static uint16_t getThumbWord(uint32_t instruction)
{
    uint32_t i = ((instruction & 0x00000400) >> 10);
    uint32_t imm4 = (instruction & 0x0000000F);
    uint32_t imm3 = ((instruction & 0x70000000) >> 28);
    uint32_t imm8 = ((instruction & 0x00FF0000) >> 16);
    return ((imm4 << 12) | (i << 11) | (imm3 << 8) | imm8);
}

static uint32_t setThumbWord(uint32_t instruction, uint16_t word) {
    uint32_t imm4 = (word & 0xF000) >> 12;
    uint32_t i =    (word & 0x0800) >> 11;
    uint32_t imm3 = (word & 0x0700) >> 8;
    uint32_t imm8 =  word & 0x00FF;
    return (instruction & 0x8F00FBF0) | imm4 | (i << 10) | (imm3 << 28) | (imm8 << 16);
}

static bool isArmMovw(uint32_t instruction)
{
    return (instruction & 0x0FF00000) == 0x03000000;
}

static bool isArmMovt(uint32_t instruction)
{
    return (instruction & 0x0FF00000) == 0x03400000;
}

static uint16_t getArmWord(uint32_t instruction)
{
    uint32_t imm4 = ((instruction & 0x000F0000) >> 16);
    uint32_t imm12 = (instruction & 0x00000FFF);
    return (imm4 << 12) | imm12;
}

static uint32_t setArmWord(uint32_t instruction, uint16_t word) {
    uint32_t imm4 = (word & 0xF000) >> 12;
    uint32_t imm12 = word & 0x0FFF;
    return (instruction & 0xFFF0F000) | (imm4 << 16) | imm12;
}

template <typename P>
void Adjustor<P>::adjustReference(uint32_t kind, uint8_t* mappedAddr, uint64_t fromNewAddress, uint64_t toNewAddress,
                                  int64_t adjust, int64_t targetSlide, uint64_t imageStartAddress, uint64_t imageEndAddress,
                                  CacheBuilder::ASLR_Tracker& aslrTracker, CacheBuilder::LOH_Tracker* lohTracker,
                                  uint32_t*& lastMappedAddr32, uint32_t& lastKind, uint64_t& lastToNewAddress)
{
    uint64_t value64;
    uint64_t* mappedAddr64 = 0;
    uint32_t value32;
    uint32_t* mappedAddr32 = 0;
    uint32_t instruction;
    dyld3::MachOLoaded::ChainedFixupPointerOnDisk chainPtr;
    int64_t offsetAdjust;
    int64_t delta;
    switch ( kind ) {
        case DYLD_CACHE_ADJ_V2_DELTA_32:
            mappedAddr32 = (uint32_t*)mappedAddr;
            value32 = P::E::get32(*mappedAddr32);
            delta = (int32_t)value32;
            delta += adjust;
            if ( (delta > 0x80000000) || (-delta > 0x80000000) ) {
                _diagnostics.error("DYLD_CACHE_ADJ_V2_DELTA_32 can't be adjust by 0x%016llX in %s", adjust, _installName);
                return;
            }
            P::E::set32(*mappedAddr32, (int32_t)delta);
            break;
        case DYLD_CACHE_ADJ_V2_POINTER_32:
            mappedAddr32 = (uint32_t*)mappedAddr;
            if ( toNewAddress != (uint64_t)(E::get32(*mappedAddr32) + targetSlide) ) {
                _diagnostics.error("bad DYLD_CACHE_ADJ_V2_POINTER_32 value not as expected at address 0x%llX in %s", fromNewAddress, _installName);
                return;
            }
            E::set32(*mappedAddr32, (uint32_t)toNewAddress);
            aslrTracker.add(mappedAddr32);
            break;
        case DYLD_CACHE_ADJ_V2_POINTER_64:
            mappedAddr64 = (uint64_t*)mappedAddr;
            if ( toNewAddress != (E::get64(*mappedAddr64) + targetSlide) ) {
                _diagnostics.error("bad DYLD_CACHE_ADJ_V2_POINTER_64 value not as expected at address 0x%llX in %s", fromNewAddress, _installName);
                return;
            }
            E::set64(*mappedAddr64, toNewAddress);
            aslrTracker.add(mappedAddr64);
            break;
        case DYLD_CACHE_ADJ_V2_THREADED_POINTER_64:
            mappedAddr64 = (uint64_t*)mappedAddr;
            chainPtr.raw64 = E::get64(*mappedAddr64);
            // ignore binds, fix up rebases to have new targets
            if ( chainPtr.arm64e.authRebase.bind == 0 ) {
                if ( chainPtr.arm64e.authRebase.auth ) {
                    // auth pointer target is offset in dyld cache
                    chainPtr.arm64e.authRebase.target += (((dyld3::MachOAnalyzer*)_mh)->preferredLoadAddress() + targetSlide - _cacheBuffer->header.sharedRegionStart);
                }
                else {
                    // plain pointer target is unslid address of target
                    chainPtr.arm64e.rebase.target += targetSlide;
                }
                // Note, the pointer remains a chain with just the target of the rebase adjusted to the new target location
                E::set64(*mappedAddr64, chainPtr.raw64);
            }
            break;
       case DYLD_CACHE_ADJ_V2_DELTA_64:
            mappedAddr64 = (uint64_t*)mappedAddr;
            value64 = P::E::get64(*mappedAddr64);
            E::set64(*mappedAddr64, value64 + adjust);
            break;
        case DYLD_CACHE_ADJ_V2_IMAGE_OFF_32:
            if ( adjust == 0 )
                break;
            mappedAddr32 = (uint32_t*)mappedAddr;
            value32 = P::E::get32(*mappedAddr32);
            value64 = toNewAddress - imageStartAddress;
            if ( value64 > imageEndAddress ) {
                _diagnostics.error("DYLD_CACHE_ADJ_V2_IMAGE_OFF_32 can't be adjust to 0x%016llX in %s", toNewAddress, _installName);
                return;
            }
            P::E::set32(*mappedAddr32, (uint32_t)value64);
            break;
        case DYLD_CACHE_ADJ_V2_ARM64_ADRP:
            mappedAddr32 = (uint32_t*)mappedAddr;
            if (lohTracker)
                (*lohTracker)[toNewAddress].insert(mappedAddr);
            instruction = P::E::get32(*mappedAddr32);
            if ( (instruction & 0x9F000000) == 0x90000000 ) {
                int64_t pageDistance = ((toNewAddress & ~0xFFF) - (fromNewAddress & ~0xFFF));
                int64_t newPage21 = pageDistance >> 12;
                if ( (newPage21 > 2097151) || (newPage21 < -2097151) ) {
                    _diagnostics.error("DYLD_CACHE_ADJ_V2_ARM64_ADRP can't be adjusted that far in %s", _installName);
                    return;
                }
                instruction = (instruction & 0x9F00001F) | ((newPage21 << 29) & 0x60000000) | ((newPage21 << 3) & 0x00FFFFE0);
                P::E::set32(*mappedAddr32, instruction);
            }
            else {
                // ADRP instructions are sometimes optimized to other instructions (e.g. ADR) after the split-seg-info is generated
            }
            break;
        case DYLD_CACHE_ADJ_V2_ARM64_OFF12:
            mappedAddr32 = (uint32_t*)mappedAddr;
            if (lohTracker)
                (*lohTracker)[toNewAddress].insert(mappedAddr);
            instruction = P::E::get32(*mappedAddr32);
            offsetAdjust = (adjust & 0xFFF);
            if ( offsetAdjust == 0 )
                break;
            if ( (instruction & 0x3B000000) == 0x39000000 ) {
                // LDR/STR imm12
                if ( offsetAdjust != 0 ) {
                    uint32_t encodedAddend = ((instruction & 0x003FFC00) >> 10);
                    uint32_t newAddend = 0;
                    switch ( instruction & 0xC0000000 ) {
                        case 0x00000000:
                            if ( (instruction & 0x04800000) == 0x04800000 ) {
                                if ( offsetAdjust & 0xF ) {
                                    _diagnostics.error("can't adjust off12 scale=16 instruction by %lld bytes at mapped address=%p in %s", offsetAdjust, mappedAddr, _installName);
                                    return;
                                }
                                if ( encodedAddend*16 >= 4096 ) {
                                    _diagnostics.error("off12 scale=16 instruction points outside its page at mapped address=%p in %s", mappedAddr, _installName);
                                }
                                newAddend = (encodedAddend + offsetAdjust/16) % 256;
                            }
                            else {
                                // scale=1
                                newAddend = (encodedAddend + (int32_t)offsetAdjust) % 4096;
                            }
                            break;
                        case 0x40000000:
                            if ( offsetAdjust & 1 ) {
                                _diagnostics.error("can't adjust off12 scale=2 instruction by %lld bytes at mapped address=%p in %s", offsetAdjust, mappedAddr, _installName);
                                return;
                            }
                            if ( encodedAddend*2 >= 4096 ) {
                                _diagnostics.error("off12 scale=2 instruction points outside its page at mapped address=%p in %s", mappedAddr, _installName);
                                return;
                            }
                            newAddend = (encodedAddend + offsetAdjust/2) % 2048;
                            break;
                        case 0x80000000:
                            if ( offsetAdjust & 3 ) {
                                _diagnostics.error("can't adjust off12 scale=4 instruction by %lld bytes at mapped address=%p in %s", offsetAdjust, mappedAddr, _installName);
                                return;
                            }
                            if ( encodedAddend*4 >= 4096 ) {
                                _diagnostics.error("off12 scale=4 instruction points outside its page at mapped address=%p in %s", mappedAddr, _installName);
                                return;
                            }
                            newAddend = (encodedAddend + offsetAdjust/4) % 1024;
                            break;
                        case 0xC0000000:
                            if ( offsetAdjust & 7 ) {
                                _diagnostics.error("can't adjust off12 scale=8 instruction by %lld bytes at mapped address=%p in %s", offsetAdjust, mappedAddr, _installName);
                                return;
                            }
                            if ( encodedAddend*8 >= 4096 ) {
                                _diagnostics.error("off12 scale=8 instruction points outside its page at mapped address=%p in %s", mappedAddr, _installName);
                                return;
                            }
                            newAddend = (encodedAddend + offsetAdjust/8) % 512;
                            break;
                    }
                    uint32_t newInstruction = (instruction & 0xFFC003FF) | (newAddend << 10);
                    P::E::set32(*mappedAddr32, newInstruction);
                }
            }
            else if ( (instruction & 0xFFC00000) == 0x91000000 ) {
                // ADD imm12
                if ( instruction & 0x00C00000 ) {
                    _diagnostics.error("ADD off12 uses shift at mapped address=%p in %s", mappedAddr, _installName);
                    return;
                }
                uint32_t encodedAddend = ((instruction & 0x003FFC00) >> 10);
                uint32_t newAddend = (encodedAddend + offsetAdjust) & 0xFFF;
                uint32_t newInstruction = (instruction & 0xFFC003FF) | (newAddend << 10);
                P::E::set32(*mappedAddr32, newInstruction);
            }
            else if ( instruction != 0xD503201F ) {
                // ignore imm12 instructions optimized into a NOP, but warn about others
                _diagnostics.error("unknown off12 instruction 0x%08X at 0x%0llX in %s", instruction, fromNewAddress, _installName);
                return;
            }
            break;
        case DYLD_CACHE_ADJ_V2_THUMB_MOVW_MOVT:
            mappedAddr32 = (uint32_t*)mappedAddr;
            // to update a movw/movt pair we need to extract the 32-bit they will make,
            // add the adjust and write back the new movw/movt pair.
            if ( lastKind == kind ) {
                if ( lastToNewAddress == toNewAddress ) {
                    uint32_t instruction1 = P::E::get32(*lastMappedAddr32);
                    uint32_t instruction2 = P::E::get32(*mappedAddr32);
                    if ( isThumbMovw(instruction1) && isThumbMovt(instruction2) ) {
                        uint16_t high = getThumbWord(instruction2);
                        uint16_t low  = getThumbWord(instruction1);
                        uint32_t full = high << 16 | low;
                        full += adjust;
                        instruction1 = setThumbWord(instruction1, full & 0xFFFF);
                        instruction2 = setThumbWord(instruction2, full >> 16);
                    }
                    else if ( isThumbMovt(instruction1) && isThumbMovw(instruction2) ) {
                        uint16_t high = getThumbWord(instruction1);
                        uint16_t low  = getThumbWord(instruction2);
                        uint32_t full = high << 16 | low;
                        full += adjust;
                        instruction2 = setThumbWord(instruction2, full & 0xFFFF);
                        instruction1 = setThumbWord(instruction1, full >> 16);
                    }
                    else {
                        _diagnostics.error("two DYLD_CACHE_ADJ_V2_THUMB_MOVW_MOVT in a row but not paried in %s", _installName);
                        return;
                    }
                    P::E::set32(*lastMappedAddr32, instruction1);
                    P::E::set32(*mappedAddr32, instruction2);
                    kind = 0;
                }
                else {
                    _diagnostics.error("two DYLD_CACHE_ADJ_V2_THUMB_MOVW_MOVT in a row but target different addresses in %s", _installName);
                    return;
                }
            }
            break;
        case DYLD_CACHE_ADJ_V2_ARM_MOVW_MOVT:
            mappedAddr32 = (uint32_t*)mappedAddr;
            // to update a movw/movt pair we need to extract the 32-bit they will make,
            // add the adjust and write back the new movw/movt pair.
            if ( lastKind == kind ) {
                if ( lastToNewAddress == toNewAddress ) {
                    uint32_t instruction1 = P::E::get32(*lastMappedAddr32);
                    uint32_t instruction2 = P::E::get32(*mappedAddr32);
                    if ( isArmMovw(instruction1) && isArmMovt(instruction2) ) {
                        uint16_t high = getArmWord(instruction2);
                        uint16_t low  = getArmWord(instruction1);
                        uint32_t full = high << 16 | low;
                        full += adjust;
                        instruction1 = setArmWord(instruction1, full & 0xFFFF);
                        instruction2 = setArmWord(instruction2, full >> 16);
                    }
                    else if ( isArmMovt(instruction1) && isArmMovw(instruction2) ) {
                        uint16_t high = getArmWord(instruction1);
                        uint16_t low  = getArmWord(instruction2);
                        uint32_t full = high << 16 | low;
                        full += adjust;
                        instruction2 = setArmWord(instruction2, full & 0xFFFF);
                        instruction1 = setArmWord(instruction1, full >> 16);
                    }
                    else {
                        _diagnostics.error("two DYLD_CACHE_ADJ_V2_ARM_MOVW_MOVT in a row but not paired in %s", _installName);
                        return;
                    }
                    P::E::set32(*lastMappedAddr32, instruction1);
                    P::E::set32(*mappedAddr32, instruction2);
                    kind = 0;
                }
                else {
                    _diagnostics.error("two DYLD_CACHE_ADJ_V2_ARM_MOVW_MOVT in a row but target different addresses in %s", _installName);
                    return;
                }
            }
            break;
        case DYLD_CACHE_ADJ_V2_ARM64_BR26:
        case DYLD_CACHE_ADJ_V2_THUMB_BR22:
        case DYLD_CACHE_ADJ_V2_ARM_BR24:
            // nothing to do with calls to stubs
            break;
        default:
            _diagnostics.error("unknown split seg kind=%d in %s", kind, _installName);
            return;
    }
    lastKind = kind;
    lastToNewAddress = toNewAddress;
    lastMappedAddr32 = mappedAddr32;
}

template <typename P>
void Adjustor<P>::adjustReferencesUsingInfoV2(CacheBuilder::ASLR_Tracker& aslrTracker,
                                              CacheBuilder::LOH_Tracker& lohTracker,
                                              const CacheBuilder::CacheCoalescedText& coalescedText,
                                              const CacheBuilder::DylibTextCoalescer& textCoalescer)
{
    static const bool log = false;

    const uint8_t* infoStart = &_linkeditBias[_splitSegInfoCmd->dataoff()];
    const uint8_t* infoEnd = &infoStart[_splitSegInfoCmd->datasize()];
    if ( *infoStart++ != DYLD_CACHE_ADJ_V2_FORMAT ) {
        _diagnostics.error("malformed split seg info in %s", _installName);
        return;
    }
    // build section arrays of slide and mapped address for each section
    std::vector<uint64_t> sectionSlides;
    std::vector<uint64_t> sectionNewAddress;
    std::vector<uint8_t*> sectionMappedAddress;
    sectionSlides.reserve(16);
    sectionNewAddress.reserve(16);
    sectionMappedAddress.reserve(16);
    // section index 0 refers to mach_header
    sectionMappedAddress.push_back((uint8_t*)_mappingInfo[0].dstSegment);
    sectionSlides.push_back(_segSlides[0]);
    sectionNewAddress.push_back(_mappingInfo[0].dstCacheUnslidAddress);
    // section 1 and later refer to real sections
    unsigned sectionIndex = 0;
    unsigned objcSelRefsSectionIndex = ~0U;
    std::map<uint64_t, std::string_view> coalescedSectionNames;
    std::map<uint64_t, uint64_t> coalescedSectionOriginalVMAddrs;
    for (unsigned segmentIndex=0; segmentIndex < _segCmds.size(); ++segmentIndex) {
        macho_segment_command<P>* segCmd = _segCmds[segmentIndex];
        macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
        macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];

        for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
            if ( (strcmp(segCmd->segname(), "__TEXT") == 0) && textCoalescer.sectionWasCoalesced(sect->sectname())) {
                // If we coalesced the segment then the sections aren't really there to be fixed up
                sectionMappedAddress.push_back(nullptr);
                sectionSlides.push_back(0);
                sectionNewAddress.push_back(0);
                if (log) {
                    fprintf(stderr, " %s/%s, sectIndex=%d, mapped at=%p\n",
                            sect->segname(), sect->sectname(), sectionIndex, sectionMappedAddress.back());
                }
                ++sectionIndex;
                std::string_view sectionName = sect->sectname();
                if (sectionName.size() > 16)
                    sectionName = sectionName.substr(0, 16);
                coalescedSectionNames[sectionIndex] = sectionName;
                coalescedSectionOriginalVMAddrs[sectionIndex] = sect->addr();
            } else {
                sectionMappedAddress.push_back((uint8_t*)_mappingInfo[segmentIndex].dstSegment + sect->addr() - segCmd->vmaddr());
                sectionSlides.push_back(_segSlides[segmentIndex]);
                sectionNewAddress.push_back(_mappingInfo[segmentIndex].dstCacheUnslidAddress + sect->addr() - segCmd->vmaddr());
                if (log) {
                    fprintf(stderr, " %s/%s, sectIndex=%d, mapped at=%p\n",
                            sect->segname(), sect->sectname(), sectionIndex, sectionMappedAddress.back());
                }
                ++sectionIndex;
                if (!strcmp(sect->segname(), "__DATA") && !strcmp(sect->sectname(), "__objc_selrefs"))
                    objcSelRefsSectionIndex = sectionIndex;
            }
        }
    }

    // Whole         :== <count> FromToSection+
    // FromToSection :== <from-sect-index> <to-sect-index> <count> ToOffset+
    // ToOffset         :== <to-sect-offset-delta> <count> FromOffset+
    // FromOffset     :== <kind> <count> <from-sect-offset-delta>
    const uint8_t* p = infoStart;
    uint64_t sectionCount = read_uleb128(p, infoEnd);
    for (uint64_t i=0; i < sectionCount; ++i) {
        uint32_t* lastMappedAddr32 = NULL;
        uint32_t lastKind = 0;
        uint64_t lastToNewAddress = 0;
        uint64_t fromSectionIndex = read_uleb128(p, infoEnd);
        uint64_t toSectionIndex = read_uleb128(p, infoEnd);
        uint64_t toOffsetCount = read_uleb128(p, infoEnd);
        uint64_t fromSectionSlide = sectionSlides[fromSectionIndex];
        uint64_t fromSectionNewAddress = sectionNewAddress[fromSectionIndex];
        uint8_t* fromSectionMappedAddress = sectionMappedAddress[fromSectionIndex];
        uint64_t toSectionSlide = sectionSlides[toSectionIndex];
        uint64_t toSectionNewAddress = sectionNewAddress[toSectionIndex];
        CacheBuilder::LOH_Tracker* lohTrackerPtr = (toSectionIndex == objcSelRefsSectionIndex) ? &lohTracker : nullptr;
        if (log) printf(" from sect=%lld (mapped=%p), to sect=%lld (new addr=0x%llX):\n", fromSectionIndex, fromSectionMappedAddress, toSectionIndex, toSectionNewAddress);
        uint64_t toSectionOffset = 0;

        // We don't support updating split seg from a coalesced segment
        if (coalescedSectionNames.find(fromSectionIndex) != coalescedSectionNames.end()) {
            _diagnostics.error("split seg from coalesced segment in %s", _installName);
            return;
        }
        for (uint64_t j=0; j < toOffsetCount; ++j) {
            uint64_t toSectionDelta = read_uleb128(p, infoEnd);
            uint64_t fromOffsetCount = read_uleb128(p, infoEnd);
            toSectionOffset += toSectionDelta;
            for (uint64_t k=0; k < fromOffsetCount; ++k) {
                uint64_t kind = read_uleb128(p, infoEnd);
                if ( kind > 13 ) {
                    _diagnostics.error("unknown split seg info v2 kind value (%llu) in %s", kind, _installName);
                    return;
                }
                uint64_t fromSectDeltaCount = read_uleb128(p, infoEnd);
                uint64_t fromSectionOffset = 0;
                for (uint64_t l=0; l < fromSectDeltaCount; ++l) {
                    uint64_t delta = read_uleb128(p, infoEnd);
                    fromSectionOffset += delta;
                    //if (log) printf("   kind=%lld, from offset=0x%0llX, to offset=0x%0llX, adjust=0x%llX, targetSlide=0x%llX\n", kind, fromSectionOffset, toSectionOffset, deltaAdjust, toSectionSlide);

                    uint8_t*  fromMappedAddr = fromSectionMappedAddress + fromSectionOffset;
                    uint64_t toNewAddress = toSectionNewAddress + toSectionOffset;
                    uint64_t fromNewAddress = fromSectionNewAddress + fromSectionOffset;
                    uint64_t imageStartAddress = sectionNewAddress.front();
                    uint64_t imageEndAddress = sectionNewAddress.back();
                    if ( toSectionIndex != 255 ) {
                        auto textCoalIt = coalescedSectionNames.find(toSectionIndex);
                        if (textCoalIt != coalescedSectionNames.end() ) {
                            //printf("Section name: %s\n", textCoalIt->second.data());
                            const CacheBuilder::DylibTextCoalescer::DylibSectionOffsetToCacheSectionOffset& offsetMap = textCoalescer.getSectionCoalescer(textCoalIt->second);
                            auto offsetIt = offsetMap.find((uint32_t)toSectionOffset);
                            assert(offsetIt != offsetMap.end());
                            uint64_t baseVMAddr = coalescedText.getSectionData(textCoalIt->second).bufferVMAddr;
                            toNewAddress = baseVMAddr + offsetIt->second;

                            // The 'to' section is gone, but we still need the 'to' slide.  Instead of a section slide, compute the slide
                            // for this individual atom
                            uint64_t toAtomOriginalVMAddr = coalescedSectionOriginalVMAddrs[toSectionIndex] + toSectionOffset;
                            uint64_t toAtomSlide = toNewAddress - toAtomOriginalVMAddr;
                            int64_t deltaAdjust = toAtomSlide - fromSectionSlide;
                            adjustReference((uint32_t)kind, fromMappedAddr, fromNewAddress, toNewAddress, deltaAdjust, toAtomSlide,
                                            imageStartAddress, imageEndAddress, aslrTracker, lohTrackerPtr, lastMappedAddr32, lastKind, lastToNewAddress);

                        } else {
                            int64_t deltaAdjust = toSectionSlide - fromSectionSlide;
                            adjustReference((uint32_t)kind, fromMappedAddr, fromNewAddress, toNewAddress, deltaAdjust, toSectionSlide,
                                            imageStartAddress, imageEndAddress, aslrTracker, lohTrackerPtr, lastMappedAddr32, lastKind, lastToNewAddress);
                        }
                    }
                    if ( _diagnostics.hasError() )
                        return;
                }
            }
        }
    }

}

template <typename P>
void Adjustor<P>::adjustDataPointers(CacheBuilder::ASLR_Tracker& aslrTracker)
{
    const uint8_t* p = &_linkeditBias[_dyldInfo->rebase_off()];
    const uint8_t* end = &p[_dyldInfo->rebase_size()];
    
    uint8_t type = 0;
    int segIndex = 0;
    uint64_t segOffset = 0;
    uint64_t count;
    uint64_t skip;
    bool done = false;
    while ( !done && (p < end) ) {
        uint8_t immediate = *p & REBASE_IMMEDIATE_MASK;
        uint8_t opcode = *p & REBASE_OPCODE_MASK;
        ++p;
        switch (opcode) {
            case REBASE_OPCODE_DONE:
                done = true;
                break;
            case REBASE_OPCODE_SET_TYPE_IMM:
                type = immediate;
                break;
            case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                segIndex = immediate;
                segOffset = read_uleb128(p, end);
                break;
            case REBASE_OPCODE_ADD_ADDR_ULEB:
                segOffset += read_uleb128(p, end);
                break;
            case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
                segOffset += immediate*sizeof(pint_t);
                break;
            case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
                for (int i=0; i < immediate; ++i) {
                    slidePointer(segIndex, segOffset, type, aslrTracker);
                    segOffset += sizeof(pint_t);
                }
                break;
            case REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
                count = read_uleb128(p, end);
                for (uint32_t i=0; i < count; ++i) {
                    slidePointer(segIndex, segOffset, type, aslrTracker);
                    segOffset += sizeof(pint_t);
                }
                break;
            case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
                slidePointer(segIndex, segOffset, type, aslrTracker);
                segOffset += read_uleb128(p, end) + sizeof(pint_t);
                break;
            case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
                count = read_uleb128(p, end);
                skip = read_uleb128(p, end);
                for (uint32_t i=0; i < count; ++i) {
                    slidePointer(segIndex, segOffset, type, aslrTracker);
                    segOffset += skip + sizeof(pint_t);
                }
                break;
            default:
                _diagnostics.error("unknown rebase opcode 0x%02X in %s", opcode, _installName);
                done = true;
                break;
        }
    }
}


template <typename P>
void Adjustor<P>::adjustInstruction(uint8_t kind, uint8_t* textLoc, uint64_t codeToDataDelta)
{
    uint32_t* fixupLoc32 = (uint32_t*)textLoc;
    uint64_t* fixupLoc64 = (uint64_t*)textLoc;
    uint32_t instruction;
    uint32_t value32;
    uint64_t value64;

    switch (kind) {
    case 1:    // 32-bit pointer (including x86_64 RIP-rel)
        value32 = P::E::get32(*fixupLoc32);
        value32 += codeToDataDelta;
        P::E::set32(*fixupLoc32, value32);
        break;
    case 2: // 64-bit pointer
        value64 =  P::E::get64(*fixupLoc64);
        value64 += codeToDataDelta;
        P::E::set64(*fixupLoc64, value64);
        break;
    case 4:    // only used for i386, a reference to something in the IMPORT segment
        break;
    case 5: // used by thumb2 movw
        instruction = P::E::get32(*fixupLoc32);
        // slide is always a multiple of 4096, so only top 4 bits of lo16 will ever need adjusting
        value32 = (instruction & 0x0000000F) + ((uint32_t)codeToDataDelta >> 12);
        instruction = (instruction & 0xFFFFFFF0) | (value32 & 0x0000000F);
        P::E::set32(*fixupLoc32, instruction);
        break;
    case 6: // used by ARM movw
        instruction = P::E::get32(*fixupLoc32);
        // slide is always a multiple of 4096, so only top 4 bits of lo16 will ever need adjusting
        value32 = ((instruction & 0x000F0000) >> 16) + ((uint32_t)codeToDataDelta >> 12);
        instruction = (instruction & 0xFFF0FFFF) | ((value32 <<16) & 0x000F0000);
        P::E::set32(*fixupLoc32, instruction);
        break;
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x1C:
    case 0x1D:
    case 0x1E:
    case 0x1F:
        // used by thumb2 movt (low nibble of kind is high 4-bits of paired movw)
        {
            instruction = P::E::get32(*fixupLoc32);
            assert((instruction & 0x8000FBF0) == 0x0000F2C0);
            // extract 16-bit value from instruction
            uint32_t i     = ((instruction & 0x00000400) >> 10);
            uint32_t imm4  =  (instruction & 0x0000000F);
            uint32_t imm3  = ((instruction & 0x70000000) >> 28);
            uint32_t imm8  = ((instruction & 0x00FF0000) >> 16);
            uint32_t imm16 = (imm4 << 12) | (i << 11) | (imm3 << 8) | imm8;
            // combine with codeToDataDelta and kind nibble
            uint32_t targetValue = (imm16 << 16) | ((kind & 0xF) << 12);
            uint32_t newTargetValue = targetValue + (uint32_t)codeToDataDelta;
            // construct new bits slices
            uint32_t imm4_    = (newTargetValue & 0xF0000000) >> 28;
            uint32_t i_       = (newTargetValue & 0x08000000) >> 27;
            uint32_t imm3_    = (newTargetValue & 0x07000000) >> 24;
            uint32_t imm8_    = (newTargetValue & 0x00FF0000) >> 16;
            // update instruction to match codeToDataDelta 
            uint32_t newInstruction = (instruction & 0x8F00FBF0) | imm4_ | (i_ << 10) | (imm3_ << 28) | (imm8_ << 16);
            P::E::set32(*fixupLoc32, newInstruction);
        }
        break;
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
    case 0x26:
    case 0x27:
    case 0x28:
    case 0x29:
    case 0x2A:
    case 0x2B:
    case 0x2C:
    case 0x2D:
    case 0x2E:
    case 0x2F:
        // used by arm movt (low nibble of kind is high 4-bits of paired movw)
        {
            instruction = P::E::get32(*fixupLoc32);
            // extract 16-bit value from instruction
            uint32_t imm4 = ((instruction & 0x000F0000) >> 16);
            uint32_t imm12 = (instruction & 0x00000FFF);
            uint32_t imm16 = (imm4 << 12) | imm12;
            // combine with codeToDataDelta and kind nibble
            uint32_t targetValue = (imm16 << 16) | ((kind & 0xF) << 12);
            uint32_t newTargetValue = targetValue + (uint32_t)codeToDataDelta;
            // construct new bits slices
            uint32_t imm4_  = (newTargetValue & 0xF0000000) >> 28;
            uint32_t imm12_ = (newTargetValue & 0x0FFF0000) >> 16;
            // update instruction to match codeToDataDelta 
            uint32_t newInstruction = (instruction & 0xFFF0F000) | (imm4_ << 16) | imm12_;
            P::E::set32(*fixupLoc32, newInstruction);
        }
        break;
    case 3: // used for arm64 ADRP
        instruction = P::E::get32(*fixupLoc32);
        if ( (instruction & 0x9F000000) == 0x90000000 ) {
            // codeToDataDelta is always a multiple of 4096, so only top 4 bits of lo16 will ever need adjusting
            value64 = ((instruction & 0x60000000) >> 17) | ((instruction & 0x00FFFFE0) << 9);
            value64 += codeToDataDelta;
            instruction = (instruction & 0x9F00001F) | ((value64 << 17) & 0x60000000) | ((value64 >> 9) & 0x00FFFFE0);
            P::E::set32(*fixupLoc32, instruction);
        }
        break;
    default:
        break;
    }
}

template <typename P>
void Adjustor<P>::adjustCode()
{
    // find compressed info on how code needs to be updated
    const uint8_t* infoStart = &_linkeditBias[_splitSegInfoCmd->dataoff()];
    const uint8_t* infoEnd = &infoStart[_splitSegInfoCmd->datasize()];;

    // This encoding only works if all data segments slide by the same amount
    uint64_t codeToDataDelta = _segSlides[1] - _segSlides[0];

    // compressed data is:  [ <kind> [uleb128-delta]+ <0> ] + <0>
    for (const uint8_t* p = infoStart; (*p != 0) && (p < infoEnd);) {
        uint8_t kind = *p++;
        uint8_t* textLoc = (uint8_t*)_mappingInfo[0].dstSegment;
        while (uint64_t delta = read_uleb128(p, infoEnd)) {
            textLoc += delta;
            adjustInstruction(kind, textLoc, codeToDataDelta);
        }
    }
}


template <typename P>
void Adjustor<P>::adjustExportsTrie(std::vector<uint8_t>& newTrieBytes)
{
    // if no export info, nothing to adjust
    uint32_t exportOffset   = 0;
    uint32_t exportSize     = 0;
    if ( _dyldInfo != nullptr ) {
        exportOffset = _dyldInfo->export_off();
        exportSize = _dyldInfo->export_size();
    } else {
        exportOffset = _exportTrieCmd->dataoff();
        exportSize = _exportTrieCmd->datasize();
    }

    if ( exportSize == 0 )
        return;

    // since export info addresses are offsets from mach_header, everything in __TEXT is fine
    // only __DATA addresses need to be updated
    const uint8_t* start = &_linkeditBias[exportOffset];
    const uint8_t* end = &start[exportSize];
    std::vector<ExportInfoTrie::Entry> originalExports;
    if ( !ExportInfoTrie::parseTrie(start, end, originalExports) ) {
        _diagnostics.error("malformed exports trie in %s", _installName);
        return;
    }

    std::vector<ExportInfoTrie::Entry> newExports;
    newExports.reserve(originalExports.size());
    uint64_t baseAddress = _segOrigStartAddresses[0];
    uint64_t baseAddressSlide = slideForOrigAddress(baseAddress);
    for (auto& entry:  originalExports) {
        // remove symbols used by the static linker only
        if (   (strncmp(entry.name.c_str(), "$ld$", 4) == 0)
            || (strncmp(entry.name.c_str(), ".objc_class_name",16) == 0)
            || (strncmp(entry.name.c_str(), ".objc_category_name",19) == 0) ) {
            continue;
        }
        // adjust symbols in slid segments
        if ( (entry.info.flags & EXPORT_SYMBOL_FLAGS_KIND_MASK) != EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE )
            entry.info.address += (slideForOrigAddress(entry.info.address + baseAddress) - baseAddressSlide);
        newExports.push_back(entry);
    }

    // rebuild export trie
    newTrieBytes.reserve(exportSize);
    
    ExportInfoTrie(newExports).emit(newTrieBytes);
    // align
    while ( (newTrieBytes.size() % sizeof(pint_t)) != 0 )
        newTrieBytes.push_back(0);
}


} // anonymous namespace

void CacheBuilder::adjustDylibSegments(const DylibInfo& dylib, Diagnostics& diag) const
{
    DyldSharedCache* cache = (DyldSharedCache*)_readExecuteRegion.buffer;
    if ( _archLayout->is64 ) {
        Adjustor<Pointer64<LittleEndian>> adjustor64(cache, (macho_header<Pointer64<LittleEndian>>*)dylib.cacheLocation[0].dstSegment, dylib.cacheLocation, diag);
        adjustor64.adjustImageForNewSegmentLocations(_aslrTracker, _lohTracker, _coalescedText, dylib.textCoalescer);
    }
    else {
        Adjustor<Pointer32<LittleEndian>> adjustor32(cache, (macho_header<Pointer32<LittleEndian>>*)dylib.cacheLocation[0].dstSegment, dylib.cacheLocation, diag);
        adjustor32.adjustImageForNewSegmentLocations(_aslrTracker, _lohTracker, _coalescedText, dylib.textCoalescer);
    }
}







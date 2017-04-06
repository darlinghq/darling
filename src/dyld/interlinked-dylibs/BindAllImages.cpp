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

#include "mega-dylib-utils.h"
#include "MachOFileAbstraction.hpp"
#include "Trie.hpp"
#include "Logging.h"

#include <dirent.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <assert.h>

#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "dyld_cache_config.h"

#if !NEW_CACHE_FILE_FORMAT
    #include "CacheFileAbstraction.hpp"
#endif

#ifndef EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE
    #define EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE            0x02
#endif


namespace {

template <typename P>
class BindInfo {
public:
    BindInfo(void* cacheBuffer, macho_header<P>* mh);

    void setReExports(const std::unordered_map<std::string, BindInfo<P>*>& dylibPathToBindInfo);
    void setDependentDylibs(const std::unordered_map<std::string, BindInfo<P>*>& dylibPathToBindInfo);
    void bind(const std::unordered_map<std::string, BindInfo<P>*>& dylibPathToBindInfo, std::vector<void*>& pointersForASLR);

    static void bindAllImagesInCache(void* cacheBuffer, const std::unordered_map<std::string, void*>& dylibPathToMachHeader, std::vector<void*>& pointersForASLR);

    void addExportsToGlobalMap(std::unordered_map<std::string, BindInfo<P>*>& reverseMap);

private:
    typedef typename P::uint_t pint_t;
    typedef typename P::E E;

    struct SymbolInfo {
                SymbolInfo() { }
        pint_t  address          = 0;
        bool    isResolver       = false;
        bool    isAbsolute       = false;
        bool    isSymbolReExport = false;
        bool    isThreadLocal    = false;
        int     reExportDylibIndex = 0;
        std::string reExportName;
   };

    void bindImmediates(const std::unordered_map<std::string, BindInfo<P>*>& dylibPathToBindInfo, std::vector<void*>& pointersForASLR);
    void bindLazyPointers(const std::unordered_map<std::string, BindInfo<P>*>& dylibPathToBindInfo, std::vector<void*>& pointersForASLR);

    void bindLocation(uint8_t segmentIndex, uint64_t segmentOffset, uint8_t type, int libraryOrdinal,
                      int64_t addend, const char* symbolName, bool lazyPointer, bool weakImport,
                      const std::unordered_map<std::string, BindInfo<P>*>& dylibPathToBindInfo, std::vector<void*>& pointersForASLR);

    bool findExportedSymbolAddress(const char* symbolName, const std::unordered_map<std::string, BindInfo<P>*>& dylibPathToBindInfo,
                                   pint_t* address, BindInfo<P>** foundIn, bool* isResolverSymbol, bool* isAbsolute);
    pint_t  findBlessedLazyPointerFor(const std::string& resolverSymbolName);
    void    switchStubToUseSharedLazyPointer(const std::string& resolverSymbolName, pint_t lpVMAddr);
    void    switchArmStubsLazyPointer(uint8_t* stubMappedAddress, pint_t stubVMAddress, uint32_t stubSize, pint_t lpVMAddr);
    void    switchArm64StubsLazyPointer(uint8_t* stubMappedAddress, pint_t stubVMAddress, uint32_t stubSize, pint_t lpVMAddr);

    typedef std::unordered_map<std::string, std::unordered_set<BindInfo<P>*>> ResolverClientsMap;
    typedef std::unordered_map<std::string, pint_t> ResolverToBlessedLazyPointerMap;

    void*                                       _cacheBuffer;
    macho_header<P>*                            _mh;
    const uint8_t*                              _linkeditBias;
    const char*                                 _installName;
    const macho_symtab_command<P>*              _symTabCmd;
    const macho_dysymtab_command<P>*            _dynSymTabCmd;
    const macho_dyld_info_command<P>*           _dyldInfo;
    std::vector<std::string>                    _dependentPaths;
    std::vector<uint64_t>                       _segSizes;
    std::vector<uint64_t>                       _segCacheOffsets;
    std::vector<const macho_segment_command<P>*>_segCmds;
    std::unordered_map<std::string, SymbolInfo> _exports;
    std::vector<std::string>                    _reExportedDylibNames;
    std::vector<BindInfo<P>*>                   _reExportedDylibs;
    std::vector<BindInfo<P>*>                   _dependentDylibs;
    pint_t                                      _baseAddress;
    ResolverClientsMap                          _resolverClients;
    ResolverToBlessedLazyPointerMap             _resolverBlessedMap;
};


template <typename P>
BindInfo<P>::BindInfo(void* cacheBuffer, macho_header<P>* mh)
    : _cacheBuffer(cacheBuffer), _mh(mh), _linkeditBias((uint8_t*)cacheBuffer), _symTabCmd(nullptr), _dynSymTabCmd(nullptr), _dyldInfo(nullptr), _baseAddress(0)
{
    macho_segment_command<P>* segCmd;
    macho_dylib_command<P>* dylibCmd;
    const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)mh + sizeof(macho_header<P>));
    const uint32_t cmd_count = mh->ncmds();
    unsigned segIndex = 0;
    const macho_load_command<P>* cmd = cmds;
    for (uint32_t i = 0; i < cmd_count; ++i) {
        switch (cmd->cmd()) {
            case LC_ID_DYLIB:
                dylibCmd = (macho_dylib_command<P>*)cmd;
                _installName = dylibCmd->name();
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
            case LC_REEXPORT_DYLIB:
                dylibCmd = (macho_dylib_command<P>*)cmd;
                _dependentPaths.push_back(dylibCmd->name());
                _reExportedDylibNames.push_back(dylibCmd->name());
                break;
            case LC_LOAD_DYLIB:
            case LC_LOAD_WEAK_DYLIB:
            case LC_LOAD_UPWARD_DYLIB:
                dylibCmd = (macho_dylib_command<P>*)cmd;
                _dependentPaths.push_back(dylibCmd->name());
                break;
           case macho_segment_command<P>::CMD:
                segCmd = (macho_segment_command<P>*)cmd;
                _segCmds.push_back(segCmd);
                _segSizes.push_back(segCmd->vmsize());
                _segCacheOffsets.push_back(segCmd->fileoff());
                if ( segIndex == 0 )
                    _baseAddress = (pint_t)segCmd->vmaddr();
                ++segIndex;
                break;
        }
        cmd = (const macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
    }

    // if no export info, no _exports map to build
    if ( _dyldInfo->export_size() == 0 )
        return;
 
    std::vector<ExportInfoTrie::Entry> exports;
    const uint8_t* exportsStart = &_linkeditBias[_dyldInfo->export_off()];
    const uint8_t* exportsEnd = &exportsStart[_dyldInfo->export_size()];
    if ( !ExportInfoTrie::parseTrie(exportsStart, exportsEnd, exports) ) {
        terminate("malformed exports trie in %s", _installName);
    }

    for(const ExportInfoTrie::Entry& entry : exports) {
        _exports[entry.name].address = (pint_t)entry.info.address + _baseAddress;
        switch ( entry.info.flags & EXPORT_SYMBOL_FLAGS_KIND_MASK ) {
            case EXPORT_SYMBOL_FLAGS_KIND_REGULAR:
                if ( (entry.info.flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER) ) {
                    _exports[entry.name].isResolver = true;
                }
                if ( entry.info.flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
                    SymbolInfo& info = _exports[entry.name];
                    info.isSymbolReExport = true;
                    info.reExportDylibIndex = (int)entry.info.other;
                    if ( !entry.info.importName.empty())
                        info.reExportName = entry.info.importName;
                }
                break;
            case EXPORT_SYMBOL_FLAGS_KIND_THREAD_LOCAL:
                _exports[entry.name].isThreadLocal = true;
                break;
            case EXPORT_SYMBOL_FLAGS_KIND_ABSOLUTE:
                _exports[entry.name].isAbsolute = true;
                _exports[entry.name].address = (pint_t)entry.info.address;
                break;
            default:
                terminate("non-regular symbol binding not supported for %s in %s", entry.name.c_str(), _installName);
                break;
        }
    }

}

template <typename P>
void BindInfo<P>::bind(const std::unordered_map<std::string, BindInfo<P>*>& dylibPathToBindInfo, std::vector<void*>& pointersForASLR)
{
    bindImmediates(dylibPathToBindInfo, pointersForASLR);
    bindLazyPointers(dylibPathToBindInfo, pointersForASLR);
    // weak bind info is processed at launch time
}


template <typename P>
void BindInfo<P>::setReExports(const std::unordered_map<std::string, BindInfo<P>*>& dylibPathToBindInfo)
{
    for (const std::string& depName : _reExportedDylibNames) {
        auto pos = dylibPathToBindInfo.find(depName);
        if ( pos == dylibPathToBindInfo.end() ) {
            terminate("can't find re-exported dylib '%s' needed by '%s'", depName.c_str(), _installName);
        }
        _reExportedDylibs.push_back(pos->second);
    }
}

template <typename P>
void BindInfo<P>::setDependentDylibs(const std::unordered_map<std::string, BindInfo<P>*>& dylibPathToBindInfo)
{
    for (const std::string& depName : _dependentPaths) {
        auto pos = dylibPathToBindInfo.find(depName);
        if ( pos == dylibPathToBindInfo.end() ) {
            terminate("can't find dependent dylib '%s' needed by '%s'", depName.c_str(), _installName);
        }
        _dependentDylibs.push_back(pos->second);
    }
}


template <typename P>
void BindInfo<P>::bindImmediates(const std::unordered_map<std::string, BindInfo<P>*>& dylibPathToBindInfo, std::vector<void*>& pointersForASLR)
{
    const uint8_t* p = &_linkeditBias[_dyldInfo->bind_off()];
    const uint8_t* end = &p[_dyldInfo->bind_size()];
    
    uint8_t type = 0;
    uint64_t segmentOffset = 0;
    uint8_t segmentIndex = 0;
    const char* symbolName = NULL;
    int libraryOrdinal = 0;
    int64_t addend = 0;
    uint64_t count;
    uint64_t skip;
    bool weakImport = false;
    bool done = false;
    while ( !done && (p < end) ) {
        uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
        uint8_t opcode = *p & BIND_OPCODE_MASK;
        ++p;
        switch (opcode) {
            case BIND_OPCODE_DONE:
                done = true;
                break;
            case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
                libraryOrdinal = immediate;
                break;
            case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
                libraryOrdinal = (int)read_uleb128(p, end);
                break;
            case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
                // the special ordinals are negative numbers
                if ( immediate == 0 )
                    libraryOrdinal = 0;
                else {
                    int8_t signExtended = BIND_OPCODE_MASK | immediate;
                    libraryOrdinal = signExtended;
                }
                break;
            case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
                weakImport = ( (immediate & BIND_SYMBOL_FLAGS_WEAK_IMPORT) != 0 );
                symbolName = (char*)p;
                while (*p != '\0')
                    ++p;
                ++p;
                break;
            case BIND_OPCODE_SET_TYPE_IMM:
                type = immediate;
                break;
            case BIND_OPCODE_SET_ADDEND_SLEB:
                addend = read_sleb128(p, end);
                break;
            case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                segmentIndex = immediate;
                segmentOffset = read_uleb128(p, end);
                break;
            case BIND_OPCODE_ADD_ADDR_ULEB:
                segmentOffset += read_uleb128(p, end);
                break;
            case BIND_OPCODE_DO_BIND:
                bindLocation(segmentIndex, segmentOffset, type, libraryOrdinal, addend, symbolName, false, weakImport, dylibPathToBindInfo, pointersForASLR);
                segmentOffset += sizeof(pint_t);
                break;
            case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
                bindLocation(segmentIndex, segmentOffset, type, libraryOrdinal, addend, symbolName, false, weakImport, dylibPathToBindInfo, pointersForASLR);
                segmentOffset += read_uleb128(p, end) + sizeof(pint_t);
                break;
            case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
                bindLocation(segmentIndex, segmentOffset, type, libraryOrdinal, addend, symbolName, false, weakImport, dylibPathToBindInfo, pointersForASLR);
                segmentOffset += immediate*sizeof(pint_t) + sizeof(pint_t);
                break;
            case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
                count = read_uleb128(p, end);
                skip = read_uleb128(p, end);
                for (uint32_t i=0; i < count; ++i) {
                    bindLocation(segmentIndex, segmentOffset, type, libraryOrdinal, addend, symbolName, false, weakImport, dylibPathToBindInfo, pointersForASLR);
                    segmentOffset += skip + sizeof(pint_t);
                }
                break;
            default:
                terminate("bad bind opcode 0x%02X in %s", *p, _installName);
        }
    }

}

template <typename P>
void BindInfo<P>::bindLazyPointers(const std::unordered_map<std::string, BindInfo<P>*>& dylibPathToBindInfo, std::vector<void*>& pointersForASLR)
{
    const uint8_t* p = &_linkeditBias[_dyldInfo->lazy_bind_off()];
    const uint8_t* end = &p[_dyldInfo->lazy_bind_size()];

    uint8_t type = BIND_TYPE_POINTER;
    uint64_t segmentOffset = 0;
    uint8_t segmentIndex = 0;
    const char* symbolName = NULL;
    int libraryOrdinal = 0;
    int64_t addend = 0;
    bool weakImport = false;
    while ( p < end ) {
        uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
        uint8_t opcode = *p & BIND_OPCODE_MASK;
        ++p;
        switch (opcode) {
            case BIND_OPCODE_DONE:
                // this opcode marks the end of each lazy pointer binding
                break;
            case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
                libraryOrdinal = immediate;
                break;
            case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
                libraryOrdinal = (int)read_uleb128(p, end);
                break;
            case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
                // the special ordinals are negative numbers
                if ( immediate == 0 )
                    libraryOrdinal = 0;
                else {
                    int8_t signExtended = BIND_OPCODE_MASK | immediate;
                    libraryOrdinal = signExtended;
                }
                break;
            case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
                weakImport = ( (immediate & BIND_SYMBOL_FLAGS_WEAK_IMPORT) != 0 );
                symbolName = (char*)p;
                while (*p != '\0')
                    ++p;
                ++p;
                break;
            case BIND_OPCODE_SET_ADDEND_SLEB:
                addend = read_sleb128(p, end);
                break;
            case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                segmentIndex = immediate;
                segmentOffset = read_uleb128(p, end);
                break;
            case BIND_OPCODE_DO_BIND:
                bindLocation(segmentIndex, segmentOffset, type, libraryOrdinal, addend, symbolName, true, weakImport, dylibPathToBindInfo, pointersForASLR);
                segmentOffset += sizeof(pint_t);
                break;
            case BIND_OPCODE_SET_TYPE_IMM:
            case BIND_OPCODE_ADD_ADDR_ULEB:
            case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
            case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
            case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
            default:
                terminate("bad lazy bind opcode 0x%02X in %s", opcode, _installName);
        }
    }

}


template <typename P>
void BindInfo<P>::bindLocation(uint8_t segmentIndex, uint64_t segmentOffset, uint8_t type, int libraryOrdinal,
                               int64_t addend, const char* symbolName, bool lazyPointer, bool weakImport,
                               const std::unordered_map<std::string, BindInfo<P>*>& dylibPathToBindInfo, std::vector<void*>& pointersForASLR)
{
    //printf("bindLocation: seg=%d, segOffset=0x%08llX, type=%d, lib=%d, addend=%lld, symbol=%s\n", segmentIndex, segmentOffset, type, libraryOrdinal, addend, symbolName);
    if ( segmentIndex > _segSizes.size() )
        terminate("bad segment index in bind info in %s", _installName);

    if ( segmentOffset > _segSizes[segmentIndex] )
        terminate("bad segment offset in bind info in %s", _installName);

    BindInfo<P>* targetBinder = nullptr;
    std::string depName;
    switch ( libraryOrdinal ) {
        case BIND_SPECIAL_DYLIB_FLAT_LOOKUP:
            terminate("dynamic lookup linkage not allowed in dyld shared cache in %s", _installName);
            break;

        case BIND_SPECIAL_DYLIB_MAIN_EXECUTABLE:
            terminate("linkage to main executable not allowed in dyld shared cache in %s", _installName);
            break;

        case BIND_SPECIAL_DYLIB_SELF:
            targetBinder = this;
            break;

        default:
            if ( libraryOrdinal < 0 )
                terminate("bad mach-o binary, special library ordinal not allowd in dyld shared cache in %s", _installName);
            if ( (unsigned)libraryOrdinal > _dependentPaths.size() )
                terminate("bad mach-o binary, library ordinal too big in %s", _installName);
            depName = _dependentPaths[libraryOrdinal-1];
            auto pos = dylibPathToBindInfo.find(depName);
            if ( pos != dylibPathToBindInfo.end() )
                targetBinder = pos->second;
            break;
    }

    pint_t targetSymbolAddress;
    bool isResolverSymbol = false;
    bool isAbsolute = false;
    BindInfo<P>* foundIn;
    if ( weakImport && (targetBinder == nullptr) ) {
        targetSymbolAddress = 0;
        foundIn = nullptr;
    }
    else {
        if (targetBinder == nullptr)
            terminate("could not bind symbol '%s'  used in '%s' because installname '%s' not found", symbolName, _installName, depName.c_str());
        if ( ! targetBinder->findExportedSymbolAddress(symbolName, dylibPathToBindInfo, &targetSymbolAddress, &foundIn, &isResolverSymbol, &isAbsolute) )
            terminate("could not bind symbol '%s' used in: %s expected in: %s", symbolName, _installName, targetBinder->_installName);
    }

    //if ( isResolverSymbol )
    //    fprintf(stderr, "found resolver based symbol '%s' in %s\n", symbolName, targetBinder->_installName);
    // don't bind lazy pointers to resolvers in shared cache
    if ( lazyPointer && isResolverSymbol ) {
        // instead find common lazy pointer that can be re-used by all clients
        pint_t lpVMAddr = targetBinder->findBlessedLazyPointerFor(symbolName);
        // switch stub to use shared lazy pointer to reduce dirty pages
        this->switchStubToUseSharedLazyPointer(symbolName, lpVMAddr);
        return;
    }


    // do actual update
    uint8_t*  mappedAddr = (uint8_t*)_cacheBuffer + _segCacheOffsets[segmentIndex] + segmentOffset;
    pint_t*   mappedAddrP = (pint_t*)mappedAddr;
    pint_t    newValue = (pint_t)(targetSymbolAddress + addend);
    switch ( type ) {
        case BIND_TYPE_POINTER:
            // only write new value if it will change it
            // this reduces pages dirtied
            if ( P::getP(*mappedAddrP) != newValue )
                P::setP(*mappedAddrP, newValue);
            break;
        
        case BIND_TYPE_TEXT_ABSOLUTE32:
        case BIND_TYPE_TEXT_PCREL32:
            terminate("text relocs not supported for shared cache binding in %s", _installName);
            break;
        
        default:
            terminate("bad bind type (%d) in %s", type, _installName);
    }
    if ( !isAbsolute )
        pointersForASLR.push_back(mappedAddr);
}


template <typename P>
bool BindInfo<P>::findExportedSymbolAddress(const char* symbolName, const std::unordered_map<std::string, BindInfo<P>*>& dylibPathToBindInfo,
                                            pint_t* address, BindInfo<P>** foundIn, bool* isResolverSymbol, bool* isAbsolute)
{
    auto pos = _exports.find(symbolName);
    if ( pos != _exports.end() ) {
        if ( pos->second.isSymbolReExport ) {
            const char* importName = symbolName;
            if ( !pos->second.reExportName.empty() )
                importName = pos->second.reExportName.c_str();
            std::string& depPath = _dependentPaths[pos->second.reExportDylibIndex-1];
            auto pos2 = dylibPathToBindInfo.find(depPath);
            if ( pos2 != dylibPathToBindInfo.end() ) {
                BindInfo<P>* reExportFrom = pos2->second;
                return reExportFrom->findExportedSymbolAddress(importName, dylibPathToBindInfo, address, foundIn, isResolverSymbol, isAbsolute);
            }
            else {
                verboseLog("findExportedSymbolAddress(%s) => ???\n", symbolName);
            }
        }
        *address = pos->second.address;
        *foundIn = this;
        *isResolverSymbol = pos->second.isResolver;
        *isAbsolute = pos->second.isAbsolute;
        //verboseLog("findExportedSymbolAddress(%s) => 0x0%llX\n", symbolName, (uint64_t)*address);
        return true;
    }

    for (BindInfo<P>* dep : _reExportedDylibs) {
        if ( dep->findExportedSymbolAddress(symbolName, dylibPathToBindInfo, address, foundIn, isResolverSymbol, isAbsolute) )
            return true;
    }
    return false;
}

template <typename P>
void BindInfo<P>::addExportsToGlobalMap(std::unordered_map<std::string, BindInfo<P>*>& reverseMap)
{
    for (const auto& expEntry : _exports) {
        const std::string& symName = expEntry.first;
        auto pos = reverseMap.find(symName);
        if ( pos == reverseMap.end() ) {
            reverseMap[symName] = this;
        }
        else {
            BindInfo<P>* other = pos->second;
            if ( expEntry.second.isSymbolReExport )
                continue;
            if ( other->_exports[symName].isSymbolReExport )
                continue;
            //warning("symbol '%s' exported from %s and %s\n", symName.c_str(), this->_installName, other->_installName);
        }
    }
}

template <typename P>
typename P::uint_t BindInfo<P>::findBlessedLazyPointerFor(const std::string& resolverSymbolName)
{
    static const bool log = false;

    // check if this has already been looked up
    auto pos1 = _resolverBlessedMap.find(resolverSymbolName);
    if ( pos1 != _resolverBlessedMap.end() ) {
        return pos1->second;
    }

    // if this symbol is re-exported from another dylib, look there
    bool thisDylibImplementsResolver = false;
    auto pos = _exports.find(resolverSymbolName);
    if ( pos != _exports.end() ) {
        const SymbolInfo& info = pos->second;
        if ( info.isSymbolReExport ) {
            std::string reImportName = resolverSymbolName;
            if ( !info.reExportName.empty() )
                reImportName = info.reExportName;
            if ( info.reExportDylibIndex > _dependentDylibs.size() ) {
                warning("dylib index for re-exported symbol %s too large (%d) in %s", resolverSymbolName.c_str(), info.reExportDylibIndex, _installName);
            }
            else {
                BindInfo<P>* reExportedFrom = _dependentDylibs[info.reExportDylibIndex-1];
                if ( log ) verboseLog( "following re-export of %s in %s, to %s in %s", resolverSymbolName.c_str(), _installName, reImportName.c_str(), reExportedFrom->_installName);
                pint_t lp = reExportedFrom->findBlessedLazyPointerFor(reImportName);
                if ( lp != 0 ) {
                    _resolverBlessedMap[resolverSymbolName] = lp;
                    return lp;
                }
            }
        }
        if ( info.isResolver )
            thisDylibImplementsResolver = true;
    }

    // lookup in lazy pointer section
    if ( thisDylibImplementsResolver ) {
        const uint32_t* const indirectTable = (uint32_t*)&_linkeditBias[_dynSymTabCmd->indirectsymoff()];
        const macho_nlist<P>* const symbolTable = (macho_nlist<P>*)(&_linkeditBias[_symTabCmd->symoff()]);
        const char* symbolStringPool = (char*)(&_linkeditBias[_symTabCmd->stroff()]);

        for (const macho_segment_command<P>* seg : _segCmds) {
            const macho_section<P>* const sectionsStart = (macho_section<P>*)((uint8_t*)seg + sizeof(macho_segment_command<P>));
            const macho_section<P>* const sectionsEnd = &sectionsStart[seg->nsects()];
            for (const macho_section<P>* sect=sectionsStart; sect < sectionsEnd; ++sect) {
                uint8_t sectionType = sect->flags() & SECTION_TYPE;
                if ( sectionType == S_LAZY_SYMBOL_POINTERS) {
                    uint32_t elementCount = (uint32_t)(sect->size() / sizeof(pint_t));
                    const uint32_t indirectTableOffset = sect->reserved1();
                    pint_t vmlocation = (pint_t)sect->addr();
                    for (uint32_t j=0; j < elementCount; ++j, vmlocation += sizeof(pint_t)) {
                        uint32_t symbolIndex = E::get32(indirectTable[indirectTableOffset + j]); 
                        switch ( symbolIndex ) {
                            case INDIRECT_SYMBOL_ABS:
                            case INDIRECT_SYMBOL_LOCAL:
                                break;
                            default:
                                const macho_nlist<P>* aSymbol = &symbolTable[symbolIndex];
                                const char* aName = &symbolStringPool[aSymbol->n_strx()];
                                if ( resolverSymbolName ==  aName) {
                                    if ( log ) verboseLog("found shared lazy pointer at 0x%llX for %s in %s in %s", (uint64_t)vmlocation, aName, sect->sectname(), _installName);
                                    _resolverBlessedMap[resolverSymbolName] = vmlocation;
                                    return vmlocation;
                                }
                                break;
                        }
                    }
                }
            }
        }
    }

    if ( log ) verboseLog( "not found shared lazy pointer for %s in %s, checking re-export dylibs", resolverSymbolName.c_str(), _installName);
    for (BindInfo<P>* reExportedDylib : _reExportedDylibs ) {
        pint_t result = reExportedDylib->findBlessedLazyPointerFor(resolverSymbolName);
        if ( result != 0 ) {
            _resolverBlessedMap[resolverSymbolName] = result;
           return result;
        }
    }

    if ( log ) verboseLog( "NOT found shared lazy pointer for %s in %s", resolverSymbolName.c_str(), _installName);
    return 0;
}

template <typename P>
void BindInfo<P>::switchStubToUseSharedLazyPointer(const std::string& resolverSymbolName, pint_t lpVMAddr)
{
    // find named stub
    const uint32_t* const indirectTable = (uint32_t*)&_linkeditBias[_dynSymTabCmd->indirectsymoff()];
    const macho_nlist<P>* const symbolTable = (macho_nlist<P>*)(&_linkeditBias[_symTabCmd->symoff()]);
    const char* symbolStringPool = (char*)(&_linkeditBias[_symTabCmd->stroff()]);
    for (const macho_segment_command<P>* seg : _segCmds) {
        macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)seg + sizeof(macho_segment_command<P>));
        macho_section<P>* const sectionsEnd = &sectionsStart[seg->nsects()];
        for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
            if ( ((sect->flags() & SECTION_TYPE) == S_SYMBOL_STUBS) && (sect->size() != 0) ) {
                pint_t stubsVMStart = (pint_t)sect->addr();
                uint8_t* stubsMappingStart = ((uint8_t*)_cacheBuffer) + sect->offset();
               const uint32_t indirectTableOffset = sect->reserved1();
                const uint32_t stubSize = sect->reserved2();
                uint32_t elementCount = (uint32_t)(sect->size() / stubSize);
                pint_t stubVMAddr = stubsVMStart;
                uint8_t* stubMappedAddr = stubsMappingStart;
                for (uint32_t j=0; j < elementCount; ++j, stubMappedAddr += stubSize, stubVMAddr += stubSize) {
                    uint32_t symbolIndex = E::get32(indirectTable[indirectTableOffset + j]); 
                    switch ( symbolIndex ) {
                        case INDIRECT_SYMBOL_ABS:
                        case INDIRECT_SYMBOL_LOCAL:
                            break;
                        default:
                            {
                                const macho_nlist<P>* aSymbol = &symbolTable[symbolIndex];
                                const char* stubName = &symbolStringPool[aSymbol->n_strx()];
                                if ( resolverSymbolName == stubName ) {
                                    switch (_mh->cputype()) {
                                        case CPU_TYPE_ARM:
                                            switchArmStubsLazyPointer(stubMappedAddr, stubVMAddr, stubSize, lpVMAddr);
                                            break;
                                        default:
                                            //warning("shared resolver lazy pointer to %s not implemented for this arch", resolverSymbolName.c_str());
                                            break;
                                    }
                                }
                            }
                            break;
                    }
                }
            }
        }
    }
}

template <typename P>
void BindInfo<P>::switchArmStubsLazyPointer(uint8_t* stubMappedAddress, pint_t stubVMAddress, uint32_t stubSize, pint_t lpVMAddr)
{
    if ( stubSize != 16 ) {
        warning("could not optimize ARM stub to resolver function in %s because it is wrong size\n", _installName);
        return;
    }
    uint32_t* instructions = (uint32_t*)stubMappedAddress;
    if (   (E::get32(instructions[0]) != 0xe59fc004)
        || (E::get32(instructions[1]) != 0xe08fc00c)
        || (E::get32(instructions[2]) != 0xe59cf000)
        ) {
        warning("could not optimize ARM stub to resolver function in %s because instructions are not as expected", _installName);
        return;
    }
    // last .long in stub is:  lazyPtr - (stub+8)
    // alter to point to more optimal lazy pointer
    uint32_t betterOffset = (uint32_t)(lpVMAddr  - (stubVMAddress + 12));
    E::set32(instructions[3], betterOffset);
}


template <typename P>
void BindInfo<P>::bindAllImagesInCache(void* cacheBuffer, const std::unordered_map<std::string, void*>& dylibPathToMachHeader, std::vector<void*>& pointersForASLR)
{
    // build BindInfo object for each dylib
    std::unordered_map<macho_header<P>*, BindInfo<P>*> headersToBindInfo;
    std::unordered_map<std::string, BindInfo<P>*> dylibPathToBindInfo;
    for (const auto& entry: dylibPathToMachHeader) {
        macho_header<P>* mh = (macho_header<P>*)entry.second;
        if ( headersToBindInfo.count(mh) == 0 )
            headersToBindInfo[mh] = new BindInfo<P>(cacheBuffer, mh);
        dylibPathToBindInfo[entry.first] = headersToBindInfo[mh];
   }

    // chain re-exported dylibs
    for (const auto& entry: headersToBindInfo) {
        entry.second->setDependentDylibs(dylibPathToBindInfo);
        entry.second->setReExports(dylibPathToBindInfo);
    }

    // bind each dylib
    for (const auto& entry: headersToBindInfo) {
        entry.second->bind(dylibPathToBindInfo, pointersForASLR);
    }

    // look for exported symbol collisions
    std::unordered_map<std::string, BindInfo<P>*> reverseMap;
    for (const auto& entry: headersToBindInfo) {
        entry.second->addExportsToGlobalMap(reverseMap);
    }

    // clean up
    for (const auto& entry: headersToBindInfo) {
        delete entry.second;
    }
}


} // anonymous namespace


void SharedCache::bindAllImagesInCache(const std::unordered_map<std::string, void*>& dylibPathToMachHeader, std::vector<void*>& pointersForASLR)
{
    switch ( _arch.arch ) {
        case CPU_TYPE_ARM:
        case CPU_TYPE_I386:
            BindInfo<Pointer32<LittleEndian>>::bindAllImagesInCache(_buffer.get(), dylibPathToMachHeader, pointersForASLR);
            break;
        case CPU_TYPE_X86_64:
        case CPU_TYPE_ARM64:
             BindInfo<Pointer64<LittleEndian>>::bindAllImagesInCache(_buffer.get(), dylibPathToMachHeader, pointersForASLR);
            break;
        default:
            terminate("unsupported arch 0x%08X", _arch.arch);
    }
}




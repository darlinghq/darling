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
#include "Trie.hpp"

#if !NEW_CACHE_FILE_FORMAT
    #include "CacheFileAbstraction.hpp"
#endif

#define ALIGN_AS_TYPE(value, type) \
        ((value + alignof(type) - 1) & (-alignof(type)))

namespace {

template <typename P>
class SortedStringPool
{
public:
    // add a string and symbol table entry index to be updated later
    void add(uint32_t symbolIndex, const char* symbolName) {
        _map[symbolName].push_back(symbolIndex);
    }

    // copy sorted strings to buffer and update all symbol's string offsets
    uint32_t copyPoolAndUpdateOffsets(char* dstStringPool, macho_nlist<P>* symbolTable) {
        // make sorted list of strings
        std::vector<std::string> allStrings;
        allStrings.reserve(_map.size());
        for (auto& entry : _map) {
            allStrings.push_back(entry.first);
        }
        std::sort(allStrings.begin(), allStrings.end());
        // walk sorted list of strings
        dstStringPool[0] = '\0'; // tradition for start of pool to be empty string
        uint32_t poolOffset = 1;
        for (const std::string& symName : allStrings) {
            // append string to pool
            strcpy(&dstStringPool[poolOffset], symName.c_str());
            //  set each string offset of each symbol using it
            for (uint32_t symbolIndex : _map[symName]) {
                symbolTable[symbolIndex].set_n_strx(poolOffset);
            }
            poolOffset += symName.size() + 1;
        }
        // return size of pool
        return poolOffset;
    }

private:
    std::unordered_map<std::string, std::vector<uint32_t>> _map;
};




struct LocalSymbolInfo
{
	uint32_t	dylibOffset;
	uint32_t	nlistStartIndex;
	uint32_t	nlistCount;
};


template <typename P>
class LinkeditOptimizer {
public:
                    LinkeditOptimizer(void* cacheBuffer, macho_header<P>* mh);

    uint32_t        linkeditSize() { return _linkeditSize; }
    uint32_t        linkeditOffset() { return _linkeditCacheOffset; }
    uint64_t        linkeditAddr() { return _linkeditAddr; }
    const char*     installName() { return _installName; }
    void            copyWeakBindingInfo(uint8_t* newLinkEditContent, uint32_t& offset);
    void            copyLazyBindingInfo(uint8_t* newLinkEditContent, uint32_t& offset);
    void            copyBindingInfo(uint8_t* newLinkEditContent, uint32_t& offset);
    void            copyExportInfo(uint8_t* newLinkEditContent, uint32_t& offset);
    void            copyExportedSymbols(uint8_t* newLinkEditContent, SortedStringPool<P>& stringPool, uint32_t& offset, uint32_t& symbolIndex);
    void            copyImportedSymbols(uint8_t* newLinkEditContent, SortedStringPool<P>& stringPool, uint32_t& offset, uint32_t& symbolIndex);
    void            copyLocalSymbols(uint8_t* newLinkEditContent, SortedStringPool<P>& stringPool, uint32_t& offset, uint32_t& symbolIndex,
                                     bool redact, std::vector<LocalSymbolInfo>& localSymbolInfos,
                                     std::vector<macho_nlist<P>>& unmappedLocalSymbols, SortedStringPool<P>& localSymbolsStringPool);
    void            copyFunctionStarts(uint8_t* newLinkEditContent, uint32_t& offset);
    void            copyDataInCode(uint8_t* newLinkEditContent, uint32_t& offset);
    void            copyIndirectSymbolTable(uint8_t* newLinkEditContent, uint32_t& offset);
    void            updateLoadCommands(uint32_t linkeditStartOffset, uint64_t mergedLinkeditAddr, uint64_t newLinkeditSize,
                                       uint32_t sharedSymbolTableStartOffset, uint32_t sharedSymbolTableCount,
                                       uint32_t sharedSymbolStringsOffset, uint32_t sharedSymbolStringsSize);

    macho_header<P>*                        machHeader() { return _mh; }
    const std::vector<const char*>          getDownwardDependents() { return _downDependentPaths; }
    const std::vector<const char*>          getAllDependents() { return _allDependentPaths; }
    const std::vector<const char*>          getReExportPaths() { return _reExportPaths; }
    const std::vector<uint64_t>             initializerAddresses() { return _initializerAddresses; }
    const std::vector<macho_section<P>*>    dofSections() { return _dofSections; }
    uint32_t                                exportsTrieLinkEditOffset() { return _newExportInfoOffset; }
    uint32_t                                exportsTrieLinkEditSize() { return _exportInfoSize; }
    uint32_t                                weakBindingLinkEditOffset() { return _newWeakBindingInfoOffset; }
    uint32_t                                weakBindingLinkEditSize() { return _newWeakBindingSize; }
    uint64_t                                dyldSectionAddress() { return _dyldSectionAddr; }
    const std::vector<macho_segment_command<P>*>&  segCmds() { return _segCmds; }


private:

    typedef typename P::uint_t pint_t;
    typedef typename P::E E;

    macho_header<P>*                        _mh;
    void*                                   _cacheBuffer;
    uint32_t                                _linkeditSize        = 0;
    uint32_t                                _linkeditCacheOffset = 0;
    uint64_t                                _linkeditAddr        = 0;
    const uint8_t*                          _linkeditBias       = nullptr;
    const char*                             _installName        = nullptr;
    macho_symtab_command<P>*                _symTabCmd          = nullptr;
    macho_dysymtab_command<P>*              _dynSymTabCmd       = nullptr;
    macho_dyld_info_command<P>*             _dyldInfo           = nullptr;
    macho_linkedit_data_command<P>*         _functionStartsCmd  = nullptr;
    macho_linkedit_data_command<P>*         _dataInCodeCmd      = nullptr;
    std::vector<macho_segment_command<P>*>  _segCmds;
    std::unordered_map<uint32_t,uint32_t>   _oldToNewSymbolIndexes;
    std::vector<const char*>                _reExportPaths;
    std::vector<const char*>                _downDependentPaths;
    std::vector<const char*>                _allDependentPaths;
    std::vector<uint64_t>                   _initializerAddresses;
    std::vector<macho_section<P>*>          _dofSections;
    uint32_t                                _newWeakBindingInfoOffset       = 0;
    uint32_t                                _newLazyBindingInfoOffset       = 0;
    uint32_t                                _newBindingInfoOffset           = 0;
    uint32_t                                _newExportInfoOffset            = 0;
    uint32_t                                _exportInfoSize                 = 0;
    uint32_t                                _newWeakBindingSize             = 0;
    uint32_t                                _newExportedSymbolsStartIndex   = 0;
    uint32_t                                _newExportedSymbolCount         = 0;
    uint32_t                                _newImportedSymbolsStartIndex   = 0;
    uint32_t                                _newImportedSymbolCount         = 0;
    uint32_t                                _newLocalSymbolsStartIndex      = 0;
    uint32_t                                _newLocalSymbolCount            = 0;
    uint32_t                                _newFunctionStartsOffset        = 0;
    uint32_t                                _newDataInCodeOffset            = 0;
    uint32_t                                _newIndirectSymbolTableOffset   = 0;
    uint64_t                                _dyldSectionAddr                = 0;
};



template <typename P>
class AcceleratorTables {
public:
                AcceleratorTables(void* cacheBuffer, uint64_t linkeditStartAddr, const std::vector<LinkeditOptimizer<P>*>& optimizers);

    uint32_t    totalSize() const;
    void        copyTo(uint8_t* buffer);

private:
    typedef typename P::E  E;

    struct NodeChain;

    struct DepNode {
        std::vector<DepNode*>   _dependents;
        unsigned                _depth;
        const char*             _installName;

                                DepNode() : _depth(0), _installName(nullptr) { }
        void                    computeDepth();
        static void             verifyUnreachable(DepNode* target, NodeChain& chain, std::unordered_set<DepNode*>& visitedNodes, const std::vector<DepNode*>& from);
    };

    struct NodeChain {
        NodeChain*   prev;
        DepNode*     node;
    };

    std::unordered_map<macho_header<P>*, DepNode>                   _depDAG;
    std::vector<dyldCacheImageInfoExtra<E>>                         _extraInfo;
    std::vector<uint8_t>                                            _trieBytes;
    std::vector<uint16_t>                                           _reExportArray;
    std::vector<uint16_t>                                           _dependencyArray;
    std::vector<uint16_t>                                           _bottomUpArray;
    std::vector<dyldCacheAcceleratorInitializer<E>>                 _initializers;
    std::vector<dyldCacheAcceleratorDOFEntry<E>>                    _dofSections;
    std::vector<dyldCacheAcceleratorRangeEntry<E>>                  _rangeTable;
    std::unordered_map<macho_header<P>*, uint32_t>                  _machHeaderToImageIndex;
    std::unordered_map<std::string, macho_header<P>*>               _dylibPathToMachHeader;
    std::unordered_map<macho_header<P>*, LinkeditOptimizer<P>*>     _machHeaderToOptimizer;
    dyldCacheAcceleratorInfo<E>                                     _acceleratorInfoHeader;
};


template <typename P>
void AcceleratorTables<P>::AcceleratorTables::DepNode::verifyUnreachable(AcceleratorTables<P>::DepNode* target, struct AcceleratorTables<P>::NodeChain& chain,
                                                                        std::unordered_set<DepNode*>& visitedNodes, const std::vector<AcceleratorTables<P>::DepNode*>& from) {
    for (DepNode* node : from) {
        bool foundCycle = (node == target);
        for (NodeChain* c = &chain; c->prev != nullptr; c = c->prev) {
            if ( c->node == node )
                foundCycle = true;
        }
        if ( foundCycle ) {
            NodeChain* chp = &chain;
            std::string msg = std::string("found cycle for ") + target->_installName;
            while (chp != nullptr) {
                msg = msg + "\n  " + chp->node->_installName;
                chp = chp->prev;
            }
            warning("%s", msg.c_str());
            return;
        }

        if ( visitedNodes.count(node) )
            continue;
        NodeChain nextChain;
        nextChain.prev = &chain;
        nextChain.node = node;
        verifyUnreachable(target, nextChain, visitedNodes, node->_dependents);
        visitedNodes.insert(node);
    }
}

const uint16_t kBranchIslandDylibIndex = 0x7FFF;

template <typename P>
AcceleratorTables<P>::AcceleratorTables(void* cacheBuffer, uint64_t linkeditStartAddr, const std::vector<LinkeditOptimizer<P>*>& optimizers)
{
    // build table mapping tables to map between mach_header, index, and optimizer
    for ( LinkeditOptimizer<P>* op : optimizers ) {
        _machHeaderToOptimizer[op->machHeader()] = op;
    }
    uint64_t cacheStartAddress;
#if NEW_CACHE_FILE_FORMAT
    #error new format support not implemented
#else
    typedef typename P::E  E;
    const dyldCacheHeader<E>* header = (dyldCacheHeader<E>*)cacheBuffer;
    const dyldCacheFileMapping<E>* mappings = (dyldCacheFileMapping<E>*)((uint8_t*)cacheBuffer + header->mappingOffset());
    cacheStartAddress = mappings[0].address();
    const dyldCacheImageInfo<E>* images = (dyldCacheImageInfo<E>*)((uint8_t*)cacheBuffer + header->imagesOffset());
    const unsigned imageCount = header->imagesCount();
    for (unsigned i=0; i < imageCount; ++i) {
        uint64_t segCacheFileOffset = images[i].address() - cacheStartAddress;
        macho_header<P>* mhMapped = (macho_header<P>*)((uint8_t*)cacheBuffer+segCacheFileOffset);
        const char* path = (char*)cacheBuffer + images[i].pathFileOffset();
        _dylibPathToMachHeader[path] = mhMapped;
        // don't add alias entries (path offset in pool near start of cache) to header->index map
        if ( images[i].pathFileOffset() > segCacheFileOffset )
            _machHeaderToImageIndex[mhMapped] = i;
    }
#endif

    // build DAG of image dependencies
    for (LinkeditOptimizer<P>* op : optimizers) {
        _depDAG[op->machHeader()]._installName = op->installName();
    }
    for (LinkeditOptimizer<P>* op : optimizers) {
        DepNode& node = _depDAG[op->machHeader()];
        for (const char* depPath : op->getDownwardDependents()) {
            macho_header<P>* depMH = _dylibPathToMachHeader[depPath];
            assert(depMH != NULL);
            DepNode* depNode = &_depDAG[depMH];
            node._dependents.push_back(depNode);
        }
    }

    // check for cycles in DAG
    for (auto& entry : _depDAG) {
        DepNode* node = &entry.second;
        NodeChain chain;
        chain.prev = nullptr;
        chain.node = node;
        std::unordered_set<DepNode*> visitedNodes;
        DepNode::verifyUnreachable(node, chain, visitedNodes, node->_dependents);
    }

    // compute depth for each DAG node
    for (auto& entry : _depDAG) {
        entry.second.computeDepth();
    }

    // build sorted (bottom up) list of images
    std::vector<macho_header<P>*> sortedMachHeaders;
    sortedMachHeaders.reserve(optimizers.size());
    for (LinkeditOptimizer<P>* op : optimizers) {
        if ( strcmp(op->installName(), "dyld_shared_cache_branch_islands") != 0 )
            sortedMachHeaders.push_back(op->machHeader());
        else
            _machHeaderToImageIndex[op->machHeader()] = kBranchIslandDylibIndex;
    }
    std::sort(sortedMachHeaders.begin(), sortedMachHeaders.end(),
              [&](macho_header<P>* lmh, macho_header<P>* rmh) -> bool {
                if ( _depDAG[lmh]._depth != _depDAG[rmh]._depth )
                    return (_depDAG[lmh]._depth < _depDAG[rmh]._depth);
                else
                    return (lmh < rmh);
              });

    // build zeroed array of extra infos
    dyldCacheImageInfoExtra<E> emptyExtra;
    emptyExtra.set_exportsTrieAddr(0);
    emptyExtra.set_weakBindingsAddr(0);
    emptyExtra.set_exportsTrieSize(0);
    emptyExtra.set_weakBindingsSize(0);
    emptyExtra.set_dependentsStartArrayIndex(0);
    emptyExtra.set_reExportsStartArrayIndex(0);
    _extraInfo.insert(_extraInfo.begin(), sortedMachHeaders.size(), emptyExtra);

    //for ( macho_header<P>* mh : sortedMachHeaders ) {
    //    fprintf(stderr, "depth: %3d mh: %p  path: %s\n", _depDAG[mh]._depth, mh, _machHeaderToOptimizer[mh]->installName());
    //}

    // build dependency table
    _dependencyArray.push_back(0xFFFF); // reserve 0 slot to be "no-dependencies"
    for (macho_header<P>* mh : sortedMachHeaders) {
        LinkeditOptimizer<P>* op = _machHeaderToOptimizer[mh];
        unsigned index = _machHeaderToImageIndex[mh];
        auto depPaths = op->getAllDependents();
        if ( depPaths.empty() ) {
            _extraInfo[index].set_dependentsStartArrayIndex(0);
        }
        else {
            _extraInfo[index].set_dependentsStartArrayIndex((uint32_t)_dependencyArray.size());
            auto downPaths = op->getDownwardDependents();
            for (const char* depPath : depPaths) {
                macho_header<P>* depMH = _dylibPathToMachHeader[depPath];
                uint16_t depIndex = _machHeaderToImageIndex[depMH];
                if ( std::find(downPaths.begin(), downPaths.end(), depPath) == downPaths.end())
                    depIndex |= 0x8000;
                _dependencyArray.push_back(depIndex);
            }
            _dependencyArray.push_back(0xFFFF); // mark end of list
       }
    }

    // build re-exports table
    _reExportArray.push_back(0xFFFF); // reserve 0 slot to be "no-re-exports"
    for (macho_header<P>* mh : sortedMachHeaders) {
        LinkeditOptimizer<P>* op = _machHeaderToOptimizer[mh];
        unsigned index = _machHeaderToImageIndex[mh];
        auto reExPaths = op->getReExportPaths();
        if ( reExPaths.empty() ) {
            _extraInfo[index].set_reExportsStartArrayIndex(0);
        }
        else {
            _extraInfo[index].set_reExportsStartArrayIndex((uint32_t)_reExportArray.size());
            for (const char* reExPath : reExPaths) {
                macho_header<P>* reExMH = _dylibPathToMachHeader[reExPath];
                uint32_t reExIndex = _machHeaderToImageIndex[reExMH];
                _reExportArray.push_back(reExIndex);
            }
            _reExportArray.push_back(0xFFFF); // mark end of list
       }
    }

    // build ordered list of initializers
    for (macho_header<P>* mh : sortedMachHeaders) {
        LinkeditOptimizer<P>* op = _machHeaderToOptimizer[mh];
        unsigned index = _machHeaderToImageIndex[mh];
        _bottomUpArray.push_back(index);
        for (uint64_t initializer : op->initializerAddresses()) {
            //fprintf(stderr, "0x%08llX %s\n", initializer, op->installName());
            dyldCacheAcceleratorInitializer<E> entry;
            entry.set_functionOffset((uint32_t)(initializer-cacheStartAddress));
            entry.set_imageIndex(_machHeaderToImageIndex[mh]);
            _initializers.push_back(entry);
        }
    }

    // build ordered list of DOF sections
    for (macho_header<P>* mh : sortedMachHeaders) {
        LinkeditOptimizer<P>* op = _machHeaderToOptimizer[mh];
        assert(op != NULL);
        unsigned imageIndex = _machHeaderToImageIndex[mh];
        for (auto& sect : op->dofSections()) {
            //fprintf(stderr, "0x%08llX %s\n", initializer, op->installName());
            dyldCacheAcceleratorDOFEntry<E> entry;
            entry.set_sectionAddress(sect->addr());
            entry.set_sectionSize((uint32_t)sect->size());
            entry.set_imageIndex(imageIndex);
            _dofSections.push_back(entry);
        }
    }

    // register exports trie and weak binding info in each dylib with image extra info
    for (macho_header<P>* mh : sortedMachHeaders) {
        LinkeditOptimizer<P>* op = _machHeaderToOptimizer[mh];
        unsigned index = _machHeaderToImageIndex[mh];
        _extraInfo[index].set_exportsTrieAddr(op->exportsTrieLinkEditOffset() + linkeditStartAddr);
        _extraInfo[index].set_exportsTrieSize(op->exportsTrieLinkEditSize());
        _extraInfo[index].set_weakBindingsAddr(op->weakBindingLinkEditOffset() + linkeditStartAddr);
        _extraInfo[index].set_weakBindingsSize(op->weakBindingLinkEditSize());
    }

    // record location of __DATA/__dyld section in libdyld.dylib
    macho_header<P>* libdyldMH = _dylibPathToMachHeader["/usr/lib/system/libdyld.dylib"];
    LinkeditOptimizer<P>* libdyldOp = _machHeaderToOptimizer[libdyldMH];
    uint64_t dyldSectionAddr = libdyldOp->dyldSectionAddress();

    // build range table for fast address->image lookups
    for (macho_header<P>* mh : sortedMachHeaders) {
        LinkeditOptimizer<P>* op = _machHeaderToOptimizer[mh];
        unsigned imageIndex = _machHeaderToImageIndex[mh];
        for ( const macho_segment_command<P>* segCmd : op->segCmds() ) {
            if ( strcmp(segCmd->segname(), "__LINKEDIT") == 0 )
                continue;
            dyldCacheAcceleratorRangeEntry<E> entry;
            entry.set_startAddress(segCmd->vmaddr());
            entry.set_size((uint32_t)segCmd->vmsize());
            entry.set_imageIndex(imageIndex);
            _rangeTable.push_back(entry);
        }
    }
    std::sort(_rangeTable.begin(), _rangeTable.end(),
              [&](const dyldCacheAcceleratorRangeEntry<E>& lRange, const dyldCacheAcceleratorRangeEntry<E>& rRange) -> bool {
                return (lRange.startAddress() < rRange.startAddress());
              });

    // build trie that maps install names to image index
    std::vector<DylibIndexTrie::Entry> dylibEntrys;
    for (auto &x : _dylibPathToMachHeader) {
        const std::string& path = x.first;
        unsigned index = _machHeaderToImageIndex[x.second];
        dylibEntrys.push_back(DylibIndexTrie::Entry(path, DylibIndex(index)));
    }
    DylibIndexTrie dylibsTrie(dylibEntrys);
    dylibsTrie.emit(_trieBytes);
    while ( (_trieBytes.size() % 4) != 0 )
        _trieBytes.push_back(0);

    // fill out header
    dyldCacheAcceleratorInfo<E>& h = _acceleratorInfoHeader;
    h.set_version(1);
    h.set_imageExtrasCount((uint32_t)_extraInfo.size());
    h.set_imagesExtrasOffset(ALIGN_AS_TYPE(sizeof(dyld_cache_accelerator_info), dyld_cache_image_info_extra));
    h.set_bottomUpListOffset(h.imagesExtrasOffset() + h.imageExtrasCount()*sizeof(dyld_cache_image_info_extra));
    h.set_dylibTrieOffset(h.bottomUpListOffset() + h.imageExtrasCount()*sizeof(uint16_t));
    h.set_dylibTrieSize((uint32_t)_trieBytes.size());
    h.set_initializersOffset(ALIGN_AS_TYPE(h.dylibTrieOffset() + h.dylibTrieSize(), dyld_cache_accelerator_initializer));
    h.set_initializersCount((uint32_t)_initializers.size());
    h.set_dofSectionsOffset(ALIGN_AS_TYPE(h.initializersOffset() + h.initializersCount()*sizeof(dyld_cache_accelerator_initializer), dyld_cache_accelerator_initializer));
    h.set_dofSectionsCount((uint32_t)_dofSections.size());
    h.set_reExportListOffset(ALIGN_AS_TYPE(h.dofSectionsOffset() + h.dofSectionsCount()*sizeof(dyld_cache_accelerator_dof), dyld_cache_accelerator_dof));
    h.set_reExportCount((uint32_t)_reExportArray.size());
    h.set_depListOffset(ALIGN_AS_TYPE(h.reExportListOffset() + h.reExportCount()*sizeof(uint16_t), uint16_t));
    h.set_depListCount((uint32_t)_dependencyArray.size());
    h.set_rangeTableOffset(ALIGN_AS_TYPE(h.depListOffset() + h.depListCount()*sizeof(uint16_t), dyld_cache_range_entry));
    h.set_rangeTableCount((uint32_t)_rangeTable.size());
    h.set_dyldSectionAddr(dyldSectionAddr);
}


template <typename P>
void AcceleratorTables<P>::DepNode::computeDepth()
{
    if ( _depth != 0 )
        return;
    _depth = 1;
    for (DepNode* node : _dependents) {
        node->computeDepth();
        if ( node->_depth >= _depth )
            _depth = node->_depth + 1;
    }
}

template <typename P>
uint32_t AcceleratorTables<P>::totalSize() const
{
    return (uint32_t)align(_acceleratorInfoHeader.rangeTableOffset() + _acceleratorInfoHeader.rangeTableCount()*sizeof(dyld_cache_range_entry), 14);
}

template <typename P>
void AcceleratorTables<P>::copyTo(uint8_t* buffer)
{
    memcpy(buffer, &_acceleratorInfoHeader, sizeof(dyld_cache_accelerator_info));
    memcpy(&buffer[_acceleratorInfoHeader.imagesExtrasOffset()], &_extraInfo[0],       _extraInfo.size()*sizeof(dyld_cache_image_info_extra));
    memcpy(&buffer[_acceleratorInfoHeader.bottomUpListOffset()], &_bottomUpArray[0],   _bottomUpArray.size()*sizeof(uint16_t));
    memcpy(&buffer[_acceleratorInfoHeader.initializersOffset()], &_initializers[0],    _initializers.size()*sizeof(dyld_cache_accelerator_initializer));
    memcpy(&buffer[_acceleratorInfoHeader.reExportListOffset()], &_reExportArray[0],   _reExportArray.size()*sizeof(uint16_t));
    memcpy(&buffer[_acceleratorInfoHeader.dofSectionsOffset()],  &_dofSections[0],     _dofSections.size()*sizeof(dyld_cache_accelerator_dof));
    memcpy(&buffer[_acceleratorInfoHeader.depListOffset()],      &_dependencyArray[0], _dependencyArray.size()*sizeof(uint16_t));
    memcpy(&buffer[_acceleratorInfoHeader.rangeTableOffset()],   &_rangeTable[0],      _rangeTable.size()*sizeof(dyld_cache_range_entry));
    memcpy(&buffer[_acceleratorInfoHeader.dylibTrieOffset()],    &_trieBytes[0],       _trieBytes.size());
}



template <typename P>
LinkeditOptimizer<P>::LinkeditOptimizer(void* cacheBuffer, macho_header<P>* mh)
: _mh(mh), _cacheBuffer(cacheBuffer)
{
    _linkeditBias = (uint8_t*)cacheBuffer;
    const unsigned origLoadCommandsSize = mh->sizeofcmds();
    unsigned bytesRemaining = origLoadCommandsSize;
    unsigned removedCount = 0;
    const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)mh + sizeof(macho_header<P>));
    const uint32_t cmdCount = mh->ncmds();
    const macho_load_command<P>* cmd = cmds;
    const macho_dylib_command<P>* dylibCmd;
    const macho_routines_command<P>* routinesCmd;
    macho_segment_command<P>* segCmd;
    for (uint32_t i = 0; i < cmdCount; ++i) {
        bool remove = false;
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
                _exportInfoSize = _dyldInfo->export_size();
                break;
            case LC_FUNCTION_STARTS:
                _functionStartsCmd = (macho_linkedit_data_command<P>*)cmd;
                break;
            case LC_DATA_IN_CODE:
                _dataInCodeCmd = (macho_linkedit_data_command<P>*)cmd;
                break;
            case LC_ROUTINES:
            case LC_ROUTINES_64:
                routinesCmd = (macho_routines_command<P>*)cmd;
                _initializerAddresses.push_back(routinesCmd->init_address());
                break;
            case LC_REEXPORT_DYLIB:
            case LC_LOAD_DYLIB:
            case LC_LOAD_WEAK_DYLIB:
            case LC_LOAD_UPWARD_DYLIB:
                dylibCmd = (macho_dylib_command<P>*)cmd;
                _allDependentPaths.push_back(dylibCmd->name());
                if ( cmd->cmd() != LC_LOAD_UPWARD_DYLIB )
                    _downDependentPaths.push_back(dylibCmd->name());
                if ( cmd->cmd() == LC_REEXPORT_DYLIB )
                    _reExportPaths.push_back(dylibCmd->name());
               break;
            case macho_segment_command<P>::CMD:
                segCmd = (macho_segment_command<P>*)cmd;
                _segCmds.push_back(segCmd);
                if ( strcmp(segCmd->segname(), "__LINKEDIT") == 0 ) {
                    _linkeditSize        = (uint32_t)segCmd->vmsize();
                    _linkeditCacheOffset = (uint32_t)segCmd->fileoff();
                    _linkeditAddr        = segCmd->vmaddr();
                }
                else if ( segCmd->nsects() > 0 ) {
                    macho_section<P>* const sectionsStart = (macho_section<P>*)((uint8_t*)segCmd + sizeof(macho_segment_command<P>));
                    macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
                    for (macho_section<P>* sect=sectionsStart; sect < sectionsEnd; ++sect) {
                        const uint8_t type = sect->flags() & SECTION_TYPE;
                        if ( type == S_MOD_INIT_FUNC_POINTERS ) {
                            const pint_t* inits = (pint_t*)((char*)cacheBuffer + sect->offset());
                            const size_t count = sect->size() / sizeof(pint_t);
                            for (size_t j=0; j < count; ++j) {
                                uint64_t func = P::getP(inits[j]);
                                _initializerAddresses.push_back(func);
                            }
                        }
  						else if ( type == S_DTRACE_DOF ) {
                            _dofSections.push_back(sect);
                        }
                        else if ( (strcmp(sect->sectname(), "__dyld") == 0) && (strncmp(sect->segname(), "__DATA", 6) == 0) ) {
                            _dyldSectionAddr = sect->addr();
                        }
                    }
                }
                break;
            case LC_SEGMENT_SPLIT_INFO:
                remove = true;
                break;
        }
        uint32_t cmdSize = cmd->cmdsize();
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
    mh->set_ncmds(cmdCount - removedCount);
    mh->set_sizeofcmds(origLoadCommandsSize - bytesRemaining);
}

/*
static void dumpLoadCommands(const uint8_t* mheader)
{
	const mach_header* const mh = (mach_header*)mheader;
	const uint32_t cmd_count = mh->ncmds;
	bool is64 = (mh->magic == MH_MAGIC_64);
	const load_command* cmds = (load_command*)(mheader + (is64 ? sizeof(mach_header_64) : sizeof(mach_header)));
	const load_command* cmd = cmds;
	const segment_command* segCmd;
	const segment_command_64* seg64Cmd;
	const symtab_command* symTab;
	const linkedit_data_command* leData;
    const uint8_t* linkEditBias = NULL;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT:
				segCmd = (const segment_command*)cmd;
				printf("LC_SEGMENT\n");
				printf("  segname  = %s\n",     segCmd->segname);
				printf("  vmaddr   = 0x%08X\n", segCmd->vmaddr);
				printf("  vmsize   = 0x%08X\n", segCmd->vmsize);
				printf("  fileoff  = 0x%08X\n", segCmd->fileoff);
				printf("  filesize = 0x%08X\n", segCmd->filesize);
                if ( strcmp(segCmd->segname, "__TEXT") == 0 ) {
                    linkEditBias = mheader - segCmd->fileoff;
                }
 				break;
			case LC_SEGMENT_64:
				seg64Cmd = (const segment_command_64*)cmd;
				printf("LC_SEGMENT_64\n");
				printf("  segname  = %s\n",        seg64Cmd->segname);
				printf("  vmaddr   = 0x%09llX\n",  seg64Cmd->vmaddr);
				printf("  vmsize   = 0x%09llX\n",  seg64Cmd->vmsize);
				printf("  fileoff  = 0x%09llX\n",  seg64Cmd->fileoff);
				printf("  filesize = 0x%09llX\n",  seg64Cmd->filesize);
                if ( strcmp(seg64Cmd->segname, "__TEXT") == 0 ) {
                    linkEditBias = mheader - seg64Cmd->fileoff;
                }
				break;
			case LC_SYMTAB:
				symTab = (const symtab_command*)cmd;
				printf("LC_SYMTAB\n");
				printf("  symoff   = 0x%08X\n", symTab->symoff);
				printf("  nsyms    = 0x%08X\n", symTab->nsyms);
				printf("  stroff   = 0x%08X\n", symTab->stroff);
				printf("  strsize  = 0x%08X\n", symTab->strsize);
                {
                const char* strPool = (char*)&linkEditBias[symTab->stroff];
                const nlist_64* sym0 = (nlist_64*)(&linkEditBias[symTab->symoff]);
                printf("    sym[0].n_strx = 0x%08X (%s)\n", sym0->n_un.n_strx, &strPool[sym0->n_un.n_strx]);
                printf("    sym[0].n_type = 0x%02X\n", sym0->n_type);
                printf("    sym[0].n_sect = 0x%02X\n", sym0->n_sect);
                printf("    sym[0].n_desc = 0x%04X\n", sym0->n_desc);
                printf("    sym[0].n_value = 0x%llX\n", sym0->n_value);
                const nlist_64* sym1 = (nlist_64*)(&linkEditBias[symTab->symoff+16]);
                printf("    sym[1].n_strx = 0x%08X (%s)\n", sym1->n_un.n_strx, &strPool[sym1->n_un.n_strx]);
                printf("    sym[1].n_type = 0x%02X\n", sym1->n_type);
                printf("    sym[1].n_sect = 0x%02X\n", sym1->n_sect);
                printf("    sym[1].n_desc = 0x%04X\n", sym1->n_desc);
                printf("    sym[1].n_value = 0x%llX\n", sym1->n_value);
                }
 				break;
			case LC_FUNCTION_STARTS:
				leData = (const linkedit_data_command*)cmd;
				printf("LC_FUNCTION_STARTS\n");
				printf("  dataoff  = 0x%08X\n", leData->dataoff);
				printf("  datasize = 0x%08X\n", leData->datasize);
			default:
				//printf("0x%08X\n", cmd->cmd);
				break;
		}
		cmd = (const load_command*)(((uint8_t*)cmd)+cmd->cmdsize);
	}
}
*/

template <typename P>
void LinkeditOptimizer<P>::updateLoadCommands(uint32_t mergedLinkeditStartOffset, uint64_t mergedLinkeditAddr, uint64_t newLinkeditSize,
                                              uint32_t sharedSymbolTableStartOffset, uint32_t sharedSymbolTableCount,
                                              uint32_t sharedSymbolStringsOffset, uint32_t sharedSymbolStringsSize)
{
    // update __LINKEDIT segment in all dylibs to overlap the same shared region
    for (macho_segment_command<P>* segCmd : _segCmds) {
        if ( strcmp(segCmd->segname(), "__LINKEDIT") == 0 ) {
            segCmd->set_vmaddr(mergedLinkeditAddr);
            segCmd->set_vmsize(newLinkeditSize);
            segCmd->set_fileoff(mergedLinkeditStartOffset);
            segCmd->set_filesize(newLinkeditSize);
        }
        else if ( strcmp(segCmd->segname(), "__TEXT") == 0 ) {
            // HACK until lldb fixed in: <rdar://problem/20357466> DynamicLoaderMacOSXDYLD fixes for Monarch dyld shared cache
            segCmd->set_fileoff(0);

        }
   }

    // update symbol table to point to shared symbol table
    _symTabCmd->set_symoff(mergedLinkeditStartOffset + sharedSymbolTableStartOffset + _newLocalSymbolsStartIndex*sizeof(macho_nlist<P>));
    _symTabCmd->set_nsyms(_newLocalSymbolCount+_newExportedSymbolCount+_newImportedSymbolCount);
    _symTabCmd->set_stroff(mergedLinkeditStartOffset + sharedSymbolStringsOffset);
    _symTabCmd->set_strsize(sharedSymbolStringsSize);

    // update dynamic symbol table to have proper offsets into shared symbol table
    _dynSymTabCmd->set_ilocalsym(0);
    _dynSymTabCmd->set_nlocalsym(_newLocalSymbolCount);
    _dynSymTabCmd->set_iextdefsym(_newExportedSymbolsStartIndex-_newLocalSymbolsStartIndex);
    _dynSymTabCmd->set_nextdefsym(_newExportedSymbolCount);
    _dynSymTabCmd->set_iundefsym(_newImportedSymbolsStartIndex-_newLocalSymbolsStartIndex);
    _dynSymTabCmd->set_nundefsym(_newImportedSymbolCount);
    _dynSymTabCmd->set_tocoff(0);
    _dynSymTabCmd->set_ntoc(0);
    _dynSymTabCmd->set_modtaboff(0);
    _dynSymTabCmd->set_nmodtab(0);
    _dynSymTabCmd->set_indirectsymoff(mergedLinkeditStartOffset + _newIndirectSymbolTableOffset);
    _dynSymTabCmd->set_extreloff(0);
    _dynSymTabCmd->set_locreloff(0);
    _dynSymTabCmd->set_nlocrel(0);

    // update dyld info
    if ( _dyldInfo != nullptr ) {
        _dyldInfo->set_rebase_off(0);
        _dyldInfo->set_rebase_size(0);
        _dyldInfo->set_bind_off(_dyldInfo->bind_size() ?  mergedLinkeditStartOffset + _newBindingInfoOffset : 0);
        _dyldInfo->set_weak_bind_off(_dyldInfo->weak_bind_size() ?  mergedLinkeditStartOffset + _newWeakBindingInfoOffset : 0 );
        _dyldInfo->set_lazy_bind_off(_dyldInfo->lazy_bind_size() ?  mergedLinkeditStartOffset + _newLazyBindingInfoOffset : 0 );
        _dyldInfo->set_export_off(mergedLinkeditStartOffset + _newExportInfoOffset);
    }

    // update function-starts
    if ( _functionStartsCmd != nullptr )
        _functionStartsCmd->set_dataoff(mergedLinkeditStartOffset+_newFunctionStartsOffset);

    // update data-in-code
    if ( _dataInCodeCmd != nullptr )
        _dataInCodeCmd->set_dataoff(mergedLinkeditStartOffset+_newDataInCodeOffset);
}

template <typename P>
void LinkeditOptimizer<P>::copyWeakBindingInfo(uint8_t* newLinkEditContent, uint32_t& offset)
{
    if ( _dyldInfo == nullptr )
        return;
    unsigned size = _dyldInfo->weak_bind_size();
    if ( size != 0 ) {
        ::memcpy(&newLinkEditContent[offset], &_linkeditBias[_dyldInfo->weak_bind_off()], size);
        _newWeakBindingInfoOffset = offset;
        _newWeakBindingSize = size;
        offset += size;
    }
}


template <typename P>
void LinkeditOptimizer<P>::copyLazyBindingInfo(uint8_t* newLinkEditContent, uint32_t& offset)
{
    if ( _dyldInfo == nullptr )
        return;
    unsigned size = _dyldInfo->lazy_bind_size();
    if ( size != 0 ) {
        ::memcpy(&newLinkEditContent[offset], &_linkeditBias[_dyldInfo->lazy_bind_off()], size);
        _newLazyBindingInfoOffset = offset;
        offset += size;
    }
}

template <typename P>
void LinkeditOptimizer<P>::copyBindingInfo(uint8_t* newLinkEditContent, uint32_t& offset)
{
    if ( _dyldInfo == nullptr )
        return;
    unsigned size = _dyldInfo->bind_size();
    if ( size != 0 ) {
        ::memcpy(&newLinkEditContent[offset], &_linkeditBias[_dyldInfo->bind_off()], size);
        _newBindingInfoOffset = offset;
        offset += size;
    }
}

template <typename P>
void LinkeditOptimizer<P>::copyExportInfo(uint8_t* newLinkEditContent, uint32_t& offset)
{
    if ( _dyldInfo == nullptr )
        return;
    unsigned size = _dyldInfo->export_size();
    if ( size != 0 ) {
        ::memcpy(&newLinkEditContent[offset], &_linkeditBias[_dyldInfo->export_off()], size);
        _newExportInfoOffset = offset;
        offset += size;
    }
}


template <typename P>
void LinkeditOptimizer<P>::copyFunctionStarts(uint8_t* newLinkEditContent, uint32_t& offset)
{
    if ( _functionStartsCmd == nullptr )
        return;
    unsigned size = _functionStartsCmd->datasize();
    ::memcpy(&newLinkEditContent[offset], &_linkeditBias[_functionStartsCmd->dataoff()], size);
    _newFunctionStartsOffset = offset;
    offset += size;
}

template <typename P>
void LinkeditOptimizer<P>::copyDataInCode(uint8_t* newLinkEditContent, uint32_t& offset)
{
    if ( _dataInCodeCmd == nullptr )
        return;
    unsigned size = _dataInCodeCmd->datasize();
    ::memcpy(&newLinkEditContent[offset], &_linkeditBias[_dataInCodeCmd->dataoff()], size);
    _newDataInCodeOffset = offset;
    offset += size;
}


template <typename P>
void LinkeditOptimizer<P>::copyLocalSymbols(uint8_t* newLinkEditContent, SortedStringPool<P>& stringPool, uint32_t& offset, uint32_t& symbolIndex,
                                            bool redact, std::vector<LocalSymbolInfo>& localSymbolInfos,
                                            std::vector<macho_nlist<P>>& unmappedLocalSymbols, SortedStringPool<P>& localSymbolsStringPool)
{
    LocalSymbolInfo localInfo;
	localInfo.dylibOffset = (uint32_t)(((uint8_t*)_mh) - (uint8_t*)_cacheBuffer);
	localInfo.nlistStartIndex = (uint32_t)unmappedLocalSymbols.size();
	localInfo.nlistCount = 0;
    _newLocalSymbolsStartIndex = symbolIndex;
    const char* strings = (char*)&_linkeditBias[_symTabCmd->stroff()];
    const macho_nlist<P>* const symbolTable = (macho_nlist<P>*)(&_linkeditBias[_symTabCmd->symoff()]);
    const macho_nlist<P>* const firstExport = &symbolTable[_dynSymTabCmd->ilocalsym()];
    const macho_nlist<P>* const lastExport  = &symbolTable[_dynSymTabCmd->ilocalsym()+_dynSymTabCmd->nlocalsym()];
    for (const macho_nlist<P>* entry = firstExport; entry < lastExport; ++entry) {
        if ( (entry->n_type() & N_TYPE) != N_SECT)
            continue;
         if ( (entry->n_type() & N_STAB) != 0)
            continue;
        const char* name = &strings[entry->n_strx()];
        macho_nlist<P>* newSymbolEntry = (macho_nlist<P>*)&newLinkEditContent[offset];
        *newSymbolEntry = *entry;
        if ( redact ) {
            // if removing local symbols, change __text symbols to "<redacted>" so backtraces don't have bogus names
            if ( entry->n_sect() == 1 ) {
                stringPool.add(symbolIndex, "<redacted>");
                ++symbolIndex;
                offset += sizeof(macho_nlist<P>);
            }
            // copy local symbol to unmmapped locals area
            localSymbolsStringPool.add((uint32_t)unmappedLocalSymbols.size(), name);
            unmappedLocalSymbols.push_back(*entry);
            unmappedLocalSymbols.back().set_n_strx(0);
        }
        else {
            stringPool.add(symbolIndex, name);
            ++symbolIndex;
            offset += sizeof(macho_nlist<P>);
       }
    }
    _newLocalSymbolCount = symbolIndex - _newLocalSymbolsStartIndex;
	localInfo.nlistCount = (uint32_t)unmappedLocalSymbols.size() - localInfo.nlistStartIndex;
	localSymbolInfos.push_back(localInfo);
}


template <typename P>
void LinkeditOptimizer<P>::copyExportedSymbols(uint8_t* newLinkEditContent, SortedStringPool<P>& stringPool, uint32_t& offset, uint32_t& symbolIndex)
{
    _newExportedSymbolsStartIndex = symbolIndex;
    const char* strings = (char*)&_linkeditBias[_symTabCmd->stroff()];
    const macho_nlist<P>* const symbolTable = (macho_nlist<P>*)(&_linkeditBias[_symTabCmd->symoff()]);
    const macho_nlist<P>* const firstExport = &symbolTable[_dynSymTabCmd->iextdefsym()];
    const macho_nlist<P>* const lastExport  = &symbolTable[_dynSymTabCmd->iextdefsym()+_dynSymTabCmd->nextdefsym()];
    uint32_t oldSymbolIndex = _dynSymTabCmd->iextdefsym();
    for (const macho_nlist<P>* entry = firstExport; entry < lastExport; ++entry, ++oldSymbolIndex) {
        if ( (entry->n_type() & N_TYPE) != N_SECT)
            continue;
        const char* name = &strings[entry->n_strx()];
        if ( strncmp(name, ".objc_", 6) == 0 )
            continue;
        if ( strncmp(name, "$ld$", 4) == 0 )
            continue;
        macho_nlist<P>* newSymbolEntry = (macho_nlist<P>*)&newLinkEditContent[offset];
        *newSymbolEntry = *entry;
        newSymbolEntry->set_n_strx(0);
        stringPool.add(symbolIndex, name);
        _oldToNewSymbolIndexes[oldSymbolIndex] = symbolIndex - _newLocalSymbolsStartIndex;
        ++symbolIndex;
        offset += sizeof(macho_nlist<P>);
    }
    _newExportedSymbolCount = symbolIndex - _newExportedSymbolsStartIndex;
}

template <typename P>
void LinkeditOptimizer<P>::copyImportedSymbols(uint8_t* newLinkEditContent, SortedStringPool<P>& stringPool, uint32_t& offset, uint32_t& symbolIndex)
{
    _newImportedSymbolsStartIndex = symbolIndex;
    const char* strings = (char*)&_linkeditBias[_symTabCmd->stroff()];
    const macho_nlist<P>* const symbolTable = (macho_nlist<P>*)(&_linkeditBias[_symTabCmd->symoff()]);
    const macho_nlist<P>* const firstImport = &symbolTable[_dynSymTabCmd->iundefsym()];
    const macho_nlist<P>* const lastImport  = &symbolTable[_dynSymTabCmd->iundefsym()+_dynSymTabCmd->nundefsym()];
    uint32_t oldSymbolIndex = _dynSymTabCmd->iundefsym();
    for (const macho_nlist<P>* entry = firstImport; entry < lastImport; ++entry, ++oldSymbolIndex) {
        if ( (entry->n_type() & N_TYPE) != N_UNDF)
            continue;
        const char* name = &strings[entry->n_strx()];
        macho_nlist<P>* newSymbolEntry = (macho_nlist<P>*)&newLinkEditContent[offset];
        *newSymbolEntry = *entry;
        newSymbolEntry->set_n_strx(0);
        stringPool.add(symbolIndex, name);
        _oldToNewSymbolIndexes[oldSymbolIndex] = symbolIndex - _newLocalSymbolsStartIndex;
        ++symbolIndex;
        offset += sizeof(macho_nlist<P>);
    }
    _newImportedSymbolCount = symbolIndex - _newImportedSymbolsStartIndex;
}

template <typename P>
void LinkeditOptimizer<P>::copyIndirectSymbolTable(uint8_t* newLinkEditContent, uint32_t& offset)
{
    _newIndirectSymbolTableOffset = offset;
     const uint32_t* const indirectTable = (uint32_t*)&_linkeditBias[_dynSymTabCmd->indirectsymoff()];
    uint32_t* newIndirectTable = (uint32_t*)&newLinkEditContent[offset];
    for (int i=0; i < _dynSymTabCmd->nindirectsyms(); ++i) {
        uint32_t symbolIndex = E::get32(indirectTable[i]);
        if ( (symbolIndex == INDIRECT_SYMBOL_ABS) || (symbolIndex == INDIRECT_SYMBOL_LOCAL) )
            E::set32(newIndirectTable[i], symbolIndex);
        else
            E::set32(newIndirectTable[i], _oldToNewSymbolIndexes[symbolIndex]);
        offset += sizeof(uint32_t);
    }
}

template <typename P>
uint64_t mergeLinkedits(SharedCache& cache, bool dontMapLocalSymbols, bool addAcceleratorTables, std::vector<LinkeditOptimizer<P>*>& optimizers)
{
    // allocate space for new linkedit data
    uint32_t linkeditStartOffset = 0xFFFFFFFF;
    uint32_t linkeditEndOffset = 0;
    uint64_t linkeditStartAddr = 0;
    for (LinkeditOptimizer<P>* op : optimizers) {
        uint32_t leOffset = op->linkeditOffset();
        if ( leOffset < linkeditStartOffset ) {
            linkeditStartOffset = leOffset;
            linkeditStartAddr = op->linkeditAddr();
        }
        uint32_t leEndOffset = op->linkeditOffset() + op->linkeditSize();
        if ( leEndOffset > linkeditEndOffset )
            linkeditEndOffset = leEndOffset;
    }
    uint64_t totalUnoptLinkeditsSize = linkeditEndOffset - linkeditStartOffset;
    uint8_t* newLinkEdit = (uint8_t*)calloc(totalUnoptLinkeditsSize, 1);
    SortedStringPool<P> stringPool;
    uint32_t offset = 0;

    verboseLog("Merged LINKEDIT:");

    // copy weak binding info
    uint32_t startWeakBindInfosOffset = offset;
    for (LinkeditOptimizer<P>* op : optimizers) {
        op->copyWeakBindingInfo(newLinkEdit, offset);
    }
    verboseLog("  weak bindings size:      %5uKB", (offset-startWeakBindInfosOffset)/1024);

    // copy export info
    uint32_t startExportInfosOffset = offset;
    for (LinkeditOptimizer<P>* op : optimizers) {
        op->copyExportInfo(newLinkEdit, offset);
    }
    verboseLog("  exports info size:       %5uKB", (offset-startExportInfosOffset)/1024);

    // in theory, an optimized cache can drop the binding info
    if ( true ) {
        // copy binding info
        uint32_t startBindingsInfosOffset = offset;
        for (LinkeditOptimizer<P>* op : optimizers) {
            op->copyBindingInfo(newLinkEdit, offset);
        }
        verboseLog("  bindings size:           %5uKB", (offset-startBindingsInfosOffset)/1024);

       // copy lazy binding info
        uint32_t startLazyBindingsInfosOffset = offset;
        for (LinkeditOptimizer<P>* op : optimizers) {
            op->copyLazyBindingInfo(newLinkEdit, offset);
        }
        verboseLog("  lazy bindings size:      %5uKB", (offset-startLazyBindingsInfosOffset)/1024);
    }

    // copy symbol table entries
    std::vector<macho_nlist<P>> unmappedLocalSymbols;
    if ( dontMapLocalSymbols )
        unmappedLocalSymbols.reserve(0x01000000);
    std::vector<LocalSymbolInfo> localSymbolInfos;
        localSymbolInfos.reserve(optimizers.size());
    SortedStringPool<P> localSymbolsStringPool;
    uint32_t symbolIndex = 0;
    const uint32_t sharedSymbolTableStartOffset = offset;
    uint32_t sharedSymbolTableExportsCount = 0;
    uint32_t sharedSymbolTableImportsCount = 0;
    for (LinkeditOptimizer<P>* op : optimizers) {
        op->copyLocalSymbols(newLinkEdit, stringPool, offset, symbolIndex, dontMapLocalSymbols,
                             localSymbolInfos, unmappedLocalSymbols, localSymbolsStringPool);
        uint32_t x = symbolIndex;
        op->copyExportedSymbols(newLinkEdit, stringPool, offset, symbolIndex);
        sharedSymbolTableExportsCount += (symbolIndex-x);
        uint32_t y = symbolIndex;
        op->copyImportedSymbols(newLinkEdit, stringPool, offset, symbolIndex);
        sharedSymbolTableImportsCount += (symbolIndex-y);
    }
    uint32_t sharedSymbolTableCount = symbolIndex;
    const uint32_t sharedSymbolTableEndOffset = offset;

    // copy function starts
    uint32_t startFunctionStartsOffset = offset;
    for (LinkeditOptimizer<P>* op : optimizers) {
        op->copyFunctionStarts(newLinkEdit, offset);
    }
    verboseLog("  function starts size:    %5uKB", (offset-startFunctionStartsOffset)/1024);

    // copy data-in-code info
    uint32_t startDataInCodeOffset = offset;
    for (LinkeditOptimizer<P>* op : optimizers) {
        op->copyDataInCode(newLinkEdit, offset);
    }
    verboseLog("  data in code size:       %5uB", offset-startDataInCodeOffset);

    // copy indirect symbol tables
    for (LinkeditOptimizer<P>* op : optimizers) {
        op->copyIndirectSymbolTable(newLinkEdit, offset);
    }
    // if indirect table has odd number of entries, end will not be 8-byte aligned
    if ( (offset % sizeof(typename P::uint_t)) != 0 )
        offset += 4;

    // copy string pool
    uint32_t sharedSymbolStringsOffset = offset;
    uint32_t sharedSymbolStringsSize = stringPool.copyPoolAndUpdateOffsets((char*)&newLinkEdit[sharedSymbolStringsOffset], (macho_nlist<P>*)&newLinkEdit[sharedSymbolTableStartOffset]);
    offset += sharedSymbolStringsSize;
    uint32_t newLinkeditUnalignedSize = offset;
    uint64_t newLinkeditEnd = align(linkeditStartOffset+newLinkeditUnalignedSize, 14);
    verboseLog("  symbol table size:       %5uKB (%d exports, %d imports)", (sharedSymbolTableEndOffset-sharedSymbolTableStartOffset)/1024, sharedSymbolTableExportsCount, sharedSymbolTableImportsCount);
    verboseLog("  symbol string pool size: %5uKB", sharedSymbolStringsSize/1024);

    // overwrite mapped LINKEDIT area in cache with new merged LINKEDIT content
    verboseLog("LINKEDITS optimized from %uMB to %uMB", (uint32_t)totalUnoptLinkeditsSize/(1024*1024), (uint32_t)newLinkeditUnalignedSize/(1024*1024));
    ::memcpy((char*)cache.buffer().get()+linkeditStartOffset, newLinkEdit, newLinkeditUnalignedSize);
    ::bzero((char*)cache.buffer().get()+linkeditStartOffset+newLinkeditUnalignedSize, totalUnoptLinkeditsSize-newLinkeditUnalignedSize);
    ::free(newLinkEdit);

    // If making cache for customers, add extra accelerator tables for dyld
    if ( addAcceleratorTables ) {
        AcceleratorTables<P> tables(cache.buffer().get(), linkeditStartAddr, optimizers);
        uint32_t tablesSize = tables.totalSize();
        if ( tablesSize < (totalUnoptLinkeditsSize-newLinkeditUnalignedSize) ) {
            tables.copyTo((uint8_t*)cache.buffer().get()+newLinkeditEnd);
            newLinkeditEnd += tablesSize;
            uint64_t accelInfoAddr = align(linkeditStartAddr + newLinkeditUnalignedSize, 14);
            cache.setAcceleratorInfoRange(accelInfoAddr, tablesSize);
        }
        else {
            warning("not enough room to add dyld accelerator tables");
        }
    }

    // update mapping to reduce linkedit size
    cache.setLinkeditsMappingEndFileOffset(newLinkeditEnd);

	// overwrite end of un-opt linkedits to create a new unmapped region for local symbols
    uint64_t newFileSize = newLinkeditEnd;
    if ( dontMapLocalSymbols ) {
        typedef typename P::E   E;
        uint32_t localSymbolsOffset = (uint32_t)align(newFileSize, 14);
        uint32_t spaceAtEnd = linkeditEndOffset - (uint32_t)newLinkeditEnd;
        dyldCacheLocalSymbolsInfo<E>* infoHeader = (dyldCacheLocalSymbolsInfo<E>*)((uint8_t*)cache.buffer().get()+localSymbolsOffset);
        const uint32_t entriesOffset = sizeof(dyldCacheLocalSymbolsInfo<E>);
        const uint32_t entriesCount  = (uint32_t)localSymbolInfos.size();
        const uint32_t nlistOffset   = (uint32_t)align(entriesOffset + entriesCount * sizeof(dyldCacheLocalSymbolEntry<E>), 4); // 16-byte align start
        const uint32_t nlistCount    = (uint32_t)unmappedLocalSymbols.size();
        const uint32_t stringsOffset = nlistOffset + nlistCount * sizeof(macho_nlist<P>);
        // copy info for each dylib
        dyldCacheLocalSymbolEntry<E>* entries = (dyldCacheLocalSymbolEntry<E>*)(((uint8_t*)infoHeader)+entriesOffset);
        for (int i=0; i < entriesCount; ++i) {
            entries[i].set_dylibOffset(localSymbolInfos[i].dylibOffset);
            entries[i].set_nlistStartIndex(localSymbolInfos[i].nlistStartIndex);
            entries[i].set_nlistCount(localSymbolInfos[i].nlistCount);
        }
        // copy nlists
        macho_nlist<P>* newLocalsSymbolTable = (macho_nlist<P>*)(((uint8_t*)infoHeader)+nlistOffset);
        ::memcpy(newLocalsSymbolTable, &unmappedLocalSymbols[0], nlistCount*sizeof(macho_nlist<P>));
        // copy string pool
        const uint32_t stringsSize = localSymbolsStringPool.copyPoolAndUpdateOffsets(((char*)infoHeader)+stringsOffset, newLocalsSymbolTable);
        const uint32_t localsRegionSize  = (uint32_t)align(stringsOffset+stringsSize, 14);
        if ( localsRegionSize > spaceAtEnd )
            terminate("not enough room to store local symbols");
        // fill in local symbols info
        infoHeader->set_nlistOffset(nlistOffset);
        infoHeader->set_nlistCount(nlistCount);
        infoHeader->set_stringsOffset(stringsOffset);
        infoHeader->set_stringsSize(stringsSize);
        infoHeader->set_entriesOffset(entriesOffset);
        infoHeader->set_entriesCount(entriesCount);
        // update cache size
        newFileSize += localsRegionSize;
        verboseLog("Unmapped local symbol info: %uMB",  localsRegionSize/(1024*1024));
        cache.setUnmappedLocalsRange(localSymbolsOffset, localsRegionSize);
    }

    // update all load commands to new merged layout
    for (LinkeditOptimizer<P>* op : optimizers) {
        op->updateLoadCommands(linkeditStartOffset, linkeditStartAddr, newLinkeditEnd-linkeditStartOffset,
                               sharedSymbolTableStartOffset, sharedSymbolTableCount,
                               sharedSymbolStringsOffset, sharedSymbolStringsSize);
    }

    return newFileSize;
}

} // anonymous namespace

template <typename P>
void SharedCache::optimizeLinkedit(bool dontMapLocalSymbols, bool addAcceleratorTables, const std::vector<uint64_t>& branchPoolOffsets)
{
    // construct a LinkeditOptimizer for each image
    std::vector<LinkeditOptimizer<P>*> optimizers;
    forEachImage([&](const void* mh, const char*, time_t, ino_t, const std::vector<MachOProxy::Segment>&) {
        optimizers.push_back(new LinkeditOptimizer<P>(_buffer.get(), (macho_header<P>*)mh));
    });
    // add optimizer for each branch pool
    for (uint64_t poolOffset : branchPoolOffsets) {
        macho_header<P>* mh =  (macho_header<P>*)((char*)_buffer.get() + poolOffset);
        optimizers.push_back(new LinkeditOptimizer<P>(_buffer.get(), mh));
    }

    // merge linkedit info
    _fileSize = mergeLinkedits(*this, dontMapLocalSymbols, addAcceleratorTables, optimizers);

    // delete optimizers
    for (LinkeditOptimizer<P>* op : optimizers)
        delete op;
}

void SharedCache::optimizeLinkedit(bool dontMapLocalSymbols, bool addAcceleratorTables, const std::vector<uint64_t>& branchPoolOffsets)
{
    switch ( _arch.arch ) {
        case CPU_TYPE_ARM:
        case CPU_TYPE_I386:
            optimizeLinkedit<Pointer32<LittleEndian>>(dontMapLocalSymbols, addAcceleratorTables, branchPoolOffsets);
            break;
        case CPU_TYPE_X86_64:
        case CPU_TYPE_ARM64:
            optimizeLinkedit<Pointer64<LittleEndian>>(dontMapLocalSymbols, addAcceleratorTables, branchPoolOffsets);
            break;
        default:
            terminate("unsupported arch 0x%08X", _arch.arch);
    }
}




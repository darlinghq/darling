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
#include "FileAbstraction.hpp"
#include "Logging.h"

#include "dyld_cache_config.h"

#import "Trie.hpp"

#include <dirent.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <assert.h>
#include <unistd.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonDigestSPI.h>

#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "OptimizerBranches.h"

#include "CacheFileAbstraction.hpp"
#include "CodeSigningTypes.h"

namespace {

uint64_t sharedRegionStartExecutableAddress(ArchPair arch)
{
    switch (arch.arch) {
        case CPU_TYPE_ARM:
            return ARM_SHARED_REGION_START;
        case CPU_TYPE_I386:
            return SHARED_REGION_BASE_I386;
        case CPU_TYPE_X86_64:
            return SHARED_REGION_BASE_X86_64;
        case CPU_TYPE_ARM64:
            return ARM64_SHARED_REGION_START;
        default:
            terminate("unsupported arch 0x%08X", arch.arch);
    }
}

uint64_t sharedRegionStartWriteableAddress(ArchPair arch, uint64_t textEndAddress)
{
    switch (arch.arch) {
        case CPU_TYPE_I386:
        case CPU_TYPE_X86_64:
            // more efficient if code and data never in same 2MB chunk
            return textEndAddress + 0x04000000;
        case CPU_TYPE_ARM:
            return textEndAddress;
        case CPU_TYPE_ARM64:
            return textEndAddress + 32*1024*1024; // <rdar://problem/18564532> Add 32MB padding before arm64 dyld shared cache R/W region
        default:
            terminate("unsupported arch 0x%08X", arch.arch);
    }
}

uint64_t sharedRegionStartReadOnlyAddress(ArchPair arch, uint64_t dataEndAddress, uint64_t textEndAddress)
{
    switch (arch.arch) {
        case CPU_TYPE_I386:
        case CPU_TYPE_X86_64:
            // more efficient if code and data never in same 2MB chunk
            return dataEndAddress + 0x04000000;
        case CPU_TYPE_ARM:
            return dataEndAddress;
        case CPU_TYPE_ARM64:
            return dataEndAddress + 32*1024*1024; // <rdar://problem/18564532> Add 32MB padding before arm64 dyld shared cache R/W region
        default:
            terminate("unsupported arch 0x%08X", arch.arch);
    }
}

} // anon namespace

uint8_t sharedRegionRegionAlignment(ArchPair arch) {
    switch (arch.arch) {
            return ARM_SHARED_REGION_SIZE;
        case CPU_TYPE_I386:
        case CPU_TYPE_X86_64:
            return 12; // 4KB
        case CPU_TYPE_ARM:
        case CPU_TYPE_ARM64:
            return 14; // 16KB
        default:
            terminate("unsupported arch 0x%08X", arch.arch);
    }
}

uint64_t sharedRegionRegionSize(ArchPair arch) {
    switch ( arch.arch ) {
        case CPU_TYPE_I386:
            return SHARED_REGION_SIZE_I386;
        case CPU_TYPE_X86_64:
            return SHARED_REGION_SIZE_X86_64;
        case CPU_TYPE_ARM:
            return ARM_SHARED_REGION_SIZE;
        case CPU_TYPE_ARM64:
            return ARM64_SHARED_REGION_SIZE;
        default:
            terminate("unsupported arch 0x%08X", arch.arch);
    }
}

static const std::tuple<const char* const, const char* const, const ArchPair> gArchitectures[] = {
    {"i386", nullptr, ArchPair( CPU_TYPE_I386, CPU_SUBTYPE_I386_ALL )},
    {"x86_64", nullptr, ArchPair( CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_ALL )},
    {"x86_64h", "x86_64", ArchPair( CPU_TYPE_X86_64, CPU_SUBTYPE_X86_64_H )},
    {"armv4t", nullptr, ArchPair( CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V4T )},
    {"armv5", nullptr, ArchPair( CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V5TEJ )},
    {"armv6", nullptr, ArchPair( CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V6 )},
    {"armv7", nullptr, ArchPair( CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V7 )},
    {"armv7f", nullptr, ArchPair( CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V7F )},
    {"armv7k", nullptr, ArchPair( CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V7K )},
    {"armv7s", "armv7", ArchPair( CPU_TYPE_ARM, CPU_SUBTYPE_ARM_V7S )},
    {"arm64", nullptr, ArchPair( CPU_TYPE_ARM64, CPU_SUBTYPE_ARM64_ALL )},
};

ArchPair archForString(const std::string& archStr) {
    for (auto& a : gArchitectures) {
        if ( std::get<0>( a ) == archStr ) return std::get<2>( a );
    }
    terminate("unknown architecture %s", archStr.c_str());
}

std::string stringForArch(ArchPair arch, bool allowUnknown) {
    for (auto& a : gArchitectures) {
        // FIXME LIB64 is set on some binaries and not other
        if ( std::get<2>( a ).arch == arch.arch && std::get<2>( a ).subtype == ( arch.subtype & ~CPU_SUBTYPE_MASK ) )
            return std::get<0>( a );
    }
    
    auto unknownString = 
        "unrecognized cpu type " + std::to_string(arch.arch) + 
        " subtype " + std::to_string(arch.subtype);
    if (allowUnknown) return unknownString;
    else terminate("%s", unknownString.c_str());
}

std::string fallbackArchStringForArchString( const std::string& archStr ) {
    for ( auto& a : gArchitectures ) {
        if ( std::get<0>( a ) == archStr && std::get<1>( a ) != nullptr ) {
            return std::get<1>( a );
        }
    }
    
    return "";
}

SharedCache::SharedCache(Manifest& manifest,
                         const std::string& configuration, const std::string& architecture) :
        _manifest(manifest), _arch(archForString(architecture)),
        _archManifest(manifest.configurations.find(configuration)->second.architectures.find(architecture)->second), _buffer(nullptr),
        _fileSize(0), _vmSize(0), _aliasCount(0), _slideInfoFileOffset(0), _slideInfoBufferSize(0) {
    auto maxCacheVMSize = sharedRegionRegionSize(_arch);

    for ( auto& includedDylib : _archManifest.results.dylibs ) {
        if (includedDylib.second.included) {
            //assert(manifest.dylibs.count(includedDylib.first) > 0);
            //assert(manifest.dylibs.find(includedDylib.first)->second.proxies.count(architecture) > 0);
            MachOProxy* proxy = _manifest.dylibProxy( includedDylib.first, architecture );
            assert(proxy != nullptr);
            _dylibs.push_back(proxy);
        }
    }

    // <rdar://problem/21317611> error out instead of crash if cache has no dylibs
    if ( _dylibs.size() < 30 ) // FIXME: plist should specify required vs optional dylibs
        terminate("missing required minimum set of dylibs");

    for (auto &dylib : _dylibs) {
        _segmentMap[dylib].reserve(dylib->segments.size());
        for (const auto& seg : dylib->segments)
            _segmentMap[dylib].push_back(&seg);
        _aliasCount += dylib->installNameAliases.size();
    }

    sortDylibs(_manifest.dylibOrderFile);
    if ( !_manifest.dirtyDataOrderFile.empty() )
        loadDirtyDataOrderFile(_manifest.dirtyDataOrderFile);

    assignSegmentAddresses();
    if ( _vmSize > maxCacheVMSize )
        verboseLog("%s cache overflow.  %lluMB (max %lluMB)", archName().c_str(), _vmSize/1024/1024, maxCacheVMSize/1024/1024);
    while (_vmSize > maxCacheVMSize) {
        auto evictedDylib = manifest.removeLargestLeafDylib( configuration, architecture );
        _dylibs.erase( std::remove( _dylibs.begin(), _dylibs.end(), evictedDylib ), _dylibs.end() );
        _aliasCount -= evictedDylib->installNameAliases.size();
        assignSegmentAddresses();
    }
}

// There is an order file specifying the order in which dylibs are laid out in
// general, as well as an order file specifying the order in which __DATA_DIRTY
// segments are laid out in particular.
//
// The syntax is one dylib (install name) per line.  Blank lines are ignored.
// Comments start with the # character.

static std::unordered_map<std::string, uint32_t> loadOrderFile(const std::string& orderFile) {
    std::unordered_map<std::string, uint32_t> order;

    std::ifstream myfile(orderFile);
    if ( myfile.is_open() ) {
        uint32_t count = 0;
        std::string line;
        while ( std::getline(myfile, line) ) {
            size_t pos = line.find('#');
            if ( pos != std::string::npos )
                line.resize(pos);
            while ( !line.empty() && isspace(line.back()) ) {
                line.pop_back();
            }
            if ( !line.empty() )
                order[line] = count++;
        }
        myfile.close();
    } else {
        warning("could not load orderfile '%s'", orderFile.c_str());
    }

    return order;
}

void SharedCache::loadDirtyDataOrderFile(const std::string& dirtyDataOrderFile) {
    _dataDirtySegsOrder = loadOrderFile(dirtyDataOrderFile);
}

void SharedCache::sortDylibs(const std::string& dylibOrderFile) {
    std::unordered_map<std::string, uint32_t> dylibOrder;
    if ( !dylibOrderFile.empty() )
        dylibOrder = loadOrderFile(dylibOrderFile);

    std::sort(_dylibs.begin(), _dylibs.end(), [&](const MachOProxy* a,
                                                  const MachOProxy* b) {
        const std::string& pathA = a->installName;
        const std::string& pathB = b->installName;

        const auto& orderA = dylibOrder.find(pathA);
        const auto& orderB = dylibOrder.find(pathB);
        bool foundA = (orderA != dylibOrder.end());
        bool foundB = (orderB != dylibOrder.end());

        // Order all dylibs specified in the order file first, in the order specified in
        // the file, followed by any other dylibs in lexicographic order.
        if ( foundA && foundB )
            return orderA->second < orderB->second;
        else if ( foundA )
            return true;
        else if ( foundB )
             return false;
        else
             return pathA < pathB;
    });
}

void SharedCache::buildUnoptimizedCache(void) {
    _buffer = std::shared_ptr<void>(calloc(_fileSize, 1), free);
    writeCacheHeader();
    writeCacheSegments();
    rebaseAll();
    bindAll();
}

template <typename P>
void SharedCache::buildForDevelopment(const std::string& cachePath) {
    typedef typename P::E   E;
    std::vector<uint64_t> emptyBranchPoolOffsets;
    buildUnoptimizedCache();
    optimizeObjC(false/*not production*/);
    if (_manifest.platform == "osx") {
        optimizeLinkedit(false, false, emptyBranchPoolOffsets);
    } else {
        optimizeLinkedit(true, false, emptyBranchPoolOffsets);
    }
    writeSlideInfoV2();

    dyldCacheHeader<E>* header = (dyldCacheHeader<E>*)_buffer.get();
    header->set_cacheType(kDyldSharedCacheTypeDevelopment);
    recomputeCacheUUID();

    // Calculate the VMSize of the resulting cache
    uint64_t endAddr = 0;

    forEachRegion([&] (void* content, uint64_t vmAddr, uint64_t size, uint32_t permissions) {
        if (vmAddr+size > endAddr)
            endAddr = vmAddr+size;
    });
    _vmSize = endAddr - sharedRegionStartExecutableAddress(_arch);

    if (_manifest.platform == "osx") {
        appendCodeSignature("release");
    } else {
        appendCodeSignature("development");
    }
}

template <typename P>
void SharedCache::buildForProduction(const std::string& cachePath) {
    typedef typename P::E   E;
    buildUnoptimizedCache();
    optimizeObjC(true/*production*/);
    uint64_t cacheStartAddress = sharedRegionStartExecutableAddress(_arch);

    dyldCacheHeader<E>* header = (dyldCacheHeader<E>*)_buffer.get();
    header->set_cacheType(kDyldSharedCacheTypeProduction);

    // build vector of branch pool addresss
    std::vector<uint64_t> branchPoolStartAddrs;
    std::vector<uint64_t> branchPoolOffsets;
    const uint64_t* p = (uint64_t*)((uint8_t*)_buffer.get() + header->branchPoolsOffset());
    for (int i=0; i < header->branchPoolsCount(); ++i) {
        uint64_t poolAddr = LittleEndian::get64(p[i]);
        branchPoolStartAddrs.push_back(poolAddr);
        branchPoolOffsets.push_back(poolAddr - cacheStartAddress);
    }

    bypassStubs(branchPoolStartAddrs);
    optimizeLinkedit(true, true, branchPoolOffsets);
    writeSlideInfoV2();

    recomputeCacheUUID();

    // Calculate the VMSize of the resulting cache
    uint64_t endAddr = 0;

    forEachRegion([&] (void* content, uint64_t vmAddr, uint64_t size, uint32_t permissions) {
        if (vmAddr+size > endAddr)
            endAddr = vmAddr+size;
    });
    _vmSize = endAddr - cacheStartAddress;

    appendCodeSignature("release");
}

bool SharedCache::writeCacheMapFile(const std::string& mapPath) {
    FILE* fmap = ::fopen(mapPath.c_str(), "w");
    if ( fmap == NULL )
        return false;

    std::vector<uint64_t> regionStartAddresses;
    std::vector<uint64_t> regionSizes;
    std::vector<uint64_t> regionFileOffsets;

    forEachRegion([&] (void* content, uint64_t vmAddr, uint64_t size, uint32_t permissions) {
        regionStartAddresses.push_back(vmAddr);
        regionSizes.push_back(size);
        regionFileOffsets.push_back((uint8_t*)content - (uint8_t*)_buffer.get());
        const char* prot = "RW";
        if ( permissions == (VM_PROT_EXECUTE|VM_PROT_READ) )
            prot = "EX";
        else if ( permissions == VM_PROT_READ )
            prot = "RO";
        if ( size > 1024*1024 )
            fprintf(fmap, "mapping  %s %4lluMB 0x%0llX -> 0x%0llX\n", prot, size/(1024*1024), vmAddr, vmAddr+size);
        else
            fprintf(fmap, "mapping  %s %4lluKB 0x%0llX -> 0x%0llX\n", prot, size/1024,        vmAddr, vmAddr+size);
    });

    // TODO:  add linkedit breakdown
    fprintf(fmap, "\n\n");

    std::unordered_set<const void*> seenHeaders;
    forEachImage([&](const void* machHeader, const char* installName, time_t mtime,
                                                        ino_t inode, const std::vector<MachOProxy::Segment>& segments) {
        if ( !seenHeaders.count(machHeader) ) {
            seenHeaders.insert(machHeader);

            fprintf(fmap, "%s\n", installName);
            for (const MachOProxy::Segment& seg : segments) {
                uint64_t vmAddr = 0;
                for (int i=0; i < regionSizes.size(); ++i) {
                    if ( (seg.fileOffset >= regionFileOffsets[i]) && (seg.fileOffset < (regionFileOffsets[i]+regionSizes[i])) ) {
                        vmAddr = regionStartAddresses[i] + seg.fileOffset - regionFileOffsets[i];
                    }
                }
                fprintf(fmap, "\t%16s 0x%08llX -> 0x%08llX\n", seg.name.c_str(), vmAddr, vmAddr+seg.size);
            }
        }
    });


    ::fclose(fmap);
    return true;
}

template <typename P>
std::vector<MachOProxy::Segment> getSegments(const void* cacheBuffer, const void* machHeader)
{
    std::vector<MachOProxy::Segment> result;
    macho_header<P>* mh = (macho_header<P>*)machHeader;
    const uint32_t cmd_count = mh->ncmds();
    const macho_load_command<P>* cmd = (macho_load_command<P>*)((uint8_t*)mh + sizeof(macho_header<P>));
    for (uint32_t i = 0; i < cmd_count; ++i) {
        if ( cmd->cmd() != macho_segment_command<P>::CMD )
            continue;
        macho_segment_command<P>* segCmd = (macho_segment_command<P>*)cmd;
        MachOProxy::Segment seg;
        seg.name        = segCmd->segname();
        seg.size        = segCmd->vmsize();
        seg.diskSize    = (uint32_t)segCmd->filesize();
        seg.fileOffset  = (uint32_t)segCmd->fileoff();
        seg.protection  = segCmd->initprot();
        // HACK until lldb fixed in <rdar://problem/20357466>
        if ( (seg.fileOffset == 0) && (strcmp(segCmd->segname(), "__TEXT") == 0) )
            seg.fileOffset = (uint32_t)((char*)machHeader - (char*)cacheBuffer);
        if ( segCmd->nsects() > 0 ) {
            seg.p2align  = 0;
            const macho_section<P>* const sectionsStart = (macho_section<P>*)((uint8_t*)segCmd + sizeof(macho_segment_command<P>));
            const macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
            for (const macho_section<P>*  sect=sectionsStart; sect < sectionsEnd; ++sect) {
                if ( sect->align() > seg.p2align )
                    seg.p2align = sect->align();
            }
        }
        else {
            seg.p2align = 12;
        }
        result.push_back(seg);
        cmd = (const macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
    }
    return result;
}

template <typename P>
void SharedCache::forEachImage(DylibHandler handler)
{
#if NEW_CACHE_FILE_FORMAT
    terminate("forEachImage() not implemented");
#else
    typedef typename P::E   E;
    const dyldCacheHeader<E>*      header   = (dyldCacheHeader<E>*)_buffer.get();
    const dyldCacheImageInfo<E>*   dylibs   = (dyldCacheImageInfo<E>*)((char*)_buffer.get() + header->imagesOffset());
    const dyldCacheFileMapping<E>* mappings = (dyldCacheFileMapping<E>*)((char*)_buffer.get() + header->mappingOffset());
    if ( mappings[0].file_offset() != 0 )
        terminate("malformed cache file");
    uint64_t firstImageOffset = 0;
    uint64_t firstRegionAddress = mappings[0].address();
    const void* cacheEnd = (char*)_buffer.get() + _fileSize;
    if ( (const void*)&dylibs[header->imagesCount()] > cacheEnd )
        return;
    for (uint32_t i=0; i < header->imagesCount(); ++i) {
        const char* dylibPath  = (char*)_buffer.get() + dylibs[i].pathFileOffset();
        if ( dylibPath > cacheEnd )
            return;
        uint64_t offset = dylibs[i].address() - firstRegionAddress;
        if ( firstImageOffset == 0 )
            firstImageOffset = offset;
        // skip over aliases
        if ( dylibs[i].pathFileOffset() < firstImageOffset)
            continue;
        const void* mh = (char*)_buffer.get() + offset;
        time_t inode = dylibs[i].inode();
        ino_t modTime = dylibs[i].modTime();
        handler(mh, dylibPath, modTime, inode, getSegments<P>(_buffer.get(), mh));
    }
#endif
}


template <typename P>
void SharedCache::recomputeCacheUUID(void)
{
    uint8_t* uuidLoc = nullptr;
#if NEW_CACHE_FILE_FORMAT
    const macho_header<P>* mh = (macho_header<P>*)cacheBuffer;
    const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)mh + sizeof(macho_header<P>));
    const uint32_t cmd_count = mh->ncmds();
    const macho_load_command<P>* cmd = cmds;
    for (uint32_t i = 0; i < cmd_count; ++i) {
        if ( cmd->cmd() == LC_UUID ) {
            const macho_uuid_command<P>* uuidCmd = (macho_uuid_command<P>*)cmd;
            uuidLoc = const_cast<uint8_t*>(uuidCmd->uuid());
            break;
        }
        cmd = (const macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
    }
#else
    dyldCacheHeader<P>* header = (dyldCacheHeader<P>*)_buffer.get();
    uuidLoc = const_cast<uint8_t*>(header->uuid());
#endif

    // Clear existing UUID, then MD5 whole cache buffer.
    bzero(uuidLoc, 16);
    CC_MD5(_buffer.get(), (unsigned)_fileSize, uuidLoc);
    // <rdar://problem/6723729> uuids should conform to RFC 4122 UUID version 4 & UUID version 5 formats
    uuidLoc[6] = ( uuidLoc[6] & 0x0F ) | ( 3 << 4 );
    uuidLoc[8] = ( uuidLoc[8] & 0x3F ) | 0x80;
}

template <typename P>
void SharedCache::setLinkeditsMappingEndFileOffset(uint64_t newFileSize)
{
#if NEW_CACHE_FILE_FORMAT
    terminate("setLinkeditsMappingEndFileOffset() not implemented");
#else
    typedef typename P::E   E;
    dyldCacheHeader<E>*      header   = (dyldCacheHeader<E>*)_buffer.get();
    dyldCacheFileMapping<E>* mappings = (dyldCacheFileMapping<E>*)((char*)_buffer.get() + header->mappingOffset());
    uint64_t newReadOnlySize = newFileSize - mappings[2].file_offset();
    mappings[2].set_size(newReadOnlySize);
    header->set_codeSignatureOffset(newFileSize);
    _readOnlyRegion.size = (newReadOnlySize);
#endif
}

template <typename P>
void SharedCache::setUnmappedLocalsRange(uint64_t localSymbolsOffset, uint32_t unmappedSize)
{
#if NEW_CACHE_FILE_FORMAT
    terminate("setUnmappedLocalsRange() not implemented");
#else
    typedef typename P::E   E;
    dyldCacheHeader<E>* header = (dyldCacheHeader<E>*)_buffer.get();
    header->set_localSymbolsOffset(localSymbolsOffset);
    header->set_localSymbolsSize(unmappedSize);
    // move start of code signature to new end of file
    header->set_codeSignatureOffset(localSymbolsOffset+unmappedSize);
#endif
}

template <typename P>
void SharedCache::setAcceleratorInfoRange(uint64_t accelInfoAddr, uint32_t accelInfoSize)
{
#if NEW_CACHE_FILE_FORMAT
    terminate("setUnmappedLocalsRange() not implemented");
#else
    typedef typename P::E   E;
    dyldCacheHeader<E>* header = (dyldCacheHeader<E>*)_buffer.get();
    header->set_accelerateInfoAddr(accelInfoAddr);
    header->set_accelerateInfoSize(accelInfoSize);
#endif
}

template <typename P>
void SharedCache::forEachRegion(RegionHandler handler)
{
#if NEW_CACHE_FILE_FORMAT
    const macho_header<P>* mh = (macho_header<P>*)cacheBuffer;
    const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)mh + sizeof(macho_header<P>));
    const uint32_t cmd_count = mh->ncmds();
    const macho_load_command<P>* cmd = cmds;
    for (uint32_t i = 0; i < cmd_count; ++i) {
        if ( cmd->cmd() == macho_segment_command<P>::CMD ) {
            const macho_segment_command<P>* segCmd = (macho_segment_command<P>*)cmd;
            handler((char*)cacheBuffer + segCmd->fileoff(), segCmd->vmaddr(), segCmd->vmsize(), segCmd->initprot());
        }
        cmd = (const macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
    }
#else
    typedef typename P::E   E;
    const dyldCacheHeader<E>*      header   = (dyldCacheHeader<E>*)_buffer.get();
    const dyldCacheFileMapping<E>* mappings = (dyldCacheFileMapping<E>*)((char*)_buffer.get() + header->mappingOffset());
    const dyldCacheFileMapping<E>* mappingsEnd = &mappings[header->mappingCount()];
    for (const dyldCacheFileMapping<E>* m=mappings; m < mappingsEnd; ++m) {
        handler((char*)_buffer.get() + m->file_offset(), m->address(), m->size(), m->init_prot());
    }
#endif
}

std::shared_ptr<void> SharedCache::buffer(void) const {
    return _buffer;
}

std::string SharedCache::archName() {
    return stringForArch(_arch);
}

void SharedCache::assignSegmentAddresses()
{
    _branchPoolStarts.clear();
    uint64_t addr = sharedRegionStartExecutableAddress(_arch);

    // assign TEXT segment addresses
    _textRegion.address = addr;
    _textRegion.fileOffset = 0;
    _textRegion.prot = VM_PROT_READ | VM_PROT_EXECUTE;
#if NEW_CACHE_FILE_FORMAT
    addr += 0x4000; // header
#else
    addr += 0x28000; // header
#endif
    uint64_t brPoolTextSize = branchPoolTextSize(_arch);
    uint64_t brPoolLinkEditSize = branchPoolLinkEditSize(_arch);
    uint64_t brRearch = branchReach(_arch);
    uint64_t lastPoolAddress = addr;
    for (auto& dylib : _dylibs) {
        for (auto& seg : _segmentMap[dylib]) {
            if ( seg.base->protection != (VM_PROT_READ | VM_PROT_EXECUTE) )
                continue;
            // Insert branch island pools every 128MB for arm64
            if ( (brPoolTextSize != 0) && ((addr + seg.base->size - lastPoolAddress) > brRearch) ) {
                _branchPoolStarts.push_back(addr);
                //verboseLog("adding branch pool at 0x%lX\n", addr);
                lastPoolAddress = addr;
                addr += brPoolTextSize;
            }
            // Keep __TEXT segments 4K or more aligned
            uint64_t startAlignPad = align(addr, std::max(seg.base->p2align, (uint8_t)12)) - addr;
            addr += startAlignPad;
            seg.address = addr;
            seg.cacheFileOffset = addr - _textRegion.address + _textRegion.fileOffset;
            seg.cacheSegSize = align(seg.base->sizeOfSections, 12);
            addr += align(seg.base->sizeOfSections, 12);
        }
    }
    // align TEXT region end
    uint64_t endTextAddress = align(addr, sharedRegionRegionAlignment(_arch));
    _textRegion.size = endTextAddress - _textRegion.address;

    std::unordered_map<const SegmentInfo*, std::string> dataDirtySegPaths;

    // co-locate similar __DATA* segments
    std::vector<SegmentInfo*> dataSegs;
    std::vector<SegmentInfo*> dataConstSegs;
    std::vector<SegmentInfo*> dataDirtySegs;
    for (auto& dylib : _dylibs) {
        for (auto& seg : _segmentMap[dylib]) {
            if ( seg.base->protection == (VM_PROT_READ | VM_PROT_WRITE) ) {
                if ( seg.base->name == "__DATA_CONST" ) {
                    dataConstSegs.push_back(&seg);
                }
                else if ( seg.base->name == "__DATA_DIRTY" ) {
                    dataDirtySegs.push_back(&seg);
                    dataDirtySegPaths[&seg] = dylib->installName;
                }
                else {
                    dataSegs.push_back(&seg);
                }
            }
        }
    }

    // assign __DATA* addresses
    addr = sharedRegionStartWriteableAddress(_arch, endTextAddress);
    _dataRegion.address = addr;
    _dataRegion.fileOffset = _textRegion.fileOffset + _textRegion.size;
    _dataRegion.prot = VM_PROT_READ | VM_PROT_WRITE;

    // layout all __DATA_CONST segments
    for (SegmentInfo* seg : dataConstSegs) {
        // Keep __DATA_CONST segments 4K or more aligned
        uint64_t startAlignPad = align(addr, std::max(seg->base->p2align, (uint8_t)12)) - addr;
        addr += startAlignPad;
        seg->address = addr;
        seg->cacheFileOffset = addr - _dataRegion.address + _dataRegion.fileOffset;
        seg->cacheSegSize = seg->base->sizeOfSections;
        addr += seg->base->sizeOfSections;
    }

    // layout all __DATA segments
    for (SegmentInfo* seg : dataSegs) {
        // Keep __DATA segments 4K or more aligned
        uint64_t startAlignPad = align(addr, std::max(seg->base->p2align, (uint8_t)12)) - addr;
        addr += startAlignPad;
        seg->address = addr;
        seg->cacheFileOffset = addr - _dataRegion.address + _dataRegion.fileOffset;
        seg->cacheSegSize = seg->base->sizeOfSections;
        addr += seg->base->sizeOfSections;
    }

    // layout all __DATA_DIRTY segments
    addr = align(addr, 12);
    std::sort(dataDirtySegs.begin(), dataDirtySegs.end(), [&](const SegmentInfo *a, const SegmentInfo *b) {
        const std::string& pathA = dataDirtySegPaths[a];
        const std::string& pathB = dataDirtySegPaths[b];

        const auto& orderA = _dataDirtySegsOrder.find(pathA);
        const auto& orderB = _dataDirtySegsOrder.find(pathB);
        bool foundA = (orderA != _dataDirtySegsOrder.end());
        bool foundB = (orderB != _dataDirtySegsOrder.end());

        // Order all __DATA_DIRTY segments specified in the order file first, in
        // the order specified in the file, followed by any other __DATA_DIRTY
        // segments in lexicographic order.
        if ( foundA && foundB )
            return orderA->second < orderB->second;
        else if ( foundA )
            return true;
        else if ( foundB )
             return false;
        else
             return pathA < pathB;
    });
    for (SegmentInfo* seg : dataDirtySegs) {
        // Pack __DATA_DIRTY segments
        uint64_t startAlignPad = align(addr, seg->base->p2align) - addr;
        addr += startAlignPad;
        seg->address = addr;
        seg->cacheFileOffset = addr - _dataRegion.address + _dataRegion.fileOffset;
        seg->cacheSegSize = seg->base->sizeOfSections;
        addr += seg->base->sizeOfSections;
    }

    // align DATA region end
    uint64_t endDataAddress = align(addr, sharedRegionRegionAlignment(_arch));
    _dataRegion.size = endDataAddress - _dataRegion.address;

    // start read-only region
    addr = sharedRegionStartReadOnlyAddress(_arch, endDataAddress, endTextAddress);
    _readOnlyRegion.address = addr;
    _readOnlyRegion.fileOffset = _dataRegion.fileOffset + _dataRegion.size;
    _readOnlyRegion.prot = VM_PROT_READ;

    // reserve space for kernel ASLR slide info at start of r/o region
    _slideInfoBufferSize = align((_dataRegion.size/4096) * 130, 12); // bitmap entry + toc entry
    _slideInfoFileOffset = _readOnlyRegion.fileOffset;
    addr += _slideInfoBufferSize;

    // layout all read-only (but not LINKEDIT) segments
    for (auto& dylib : _dylibs) {
        for (auto& seg : _segmentMap[dylib]) {
            if ( seg.base->protection != VM_PROT_READ )
                continue;
            if ( seg.base->name == "__LINKEDIT" )
                continue;
            // Keep segments 4K or more aligned
            addr = align(addr, std::min(seg.base->p2align, (uint8_t)12));
            seg.address = addr;
            seg.cacheFileOffset = addr - _readOnlyRegion.address + _readOnlyRegion.fileOffset;;
            seg.cacheSegSize = seg.base->size;
            addr += seg.base->size;
            //verboseLog("read-only offset=0x%08X, for path=%s\n", seg.cacheFileOffset, ex->proxy->installName.c_str());
        }
    }
    // layout all LINKEDIT segments (after other read-only segments)
    for (auto& dylib : _dylibs) {
        for (auto& seg : _segmentMap[dylib]) {
            if ( seg.base->protection != VM_PROT_READ )
                continue;
            if ( seg.base->name != "__LINKEDIT" )
                continue;
            // Keep LINKEDIT segments 4K aligned
            addr = align(addr, 12);
            seg.address = addr;
            seg.cacheFileOffset = addr - _readOnlyRegion.address + _readOnlyRegion.fileOffset;;
            seg.cacheSegSize = seg.base->diskSize;
            addr += seg.base->size;
            //verboseLog("linkedit offset=0x%08X, for path=%s\n", seg.cacheFileOffset, ex->proxy->installName.c_str());
        }
    }
    // add room for branch pool linkedits
    _branchPoolsLinkEditStartAddr = addr;
    addr += (_branchPoolStarts.size() * brPoolLinkEditSize);

    // align r/o region end
    uint64_t endReadOnlyAddress = align(addr, sharedRegionRegionAlignment(_arch));
    _readOnlyRegion.size = endReadOnlyAddress - _readOnlyRegion.address;
    _fileSize = _readOnlyRegion.fileOffset + _readOnlyRegion.size;
    // assume LINKEDIT optimzation reduces LINKEDITs to %40 of original size
    _vmSize = _readOnlyRegion.address+(_readOnlyRegion.size * 2/5) - _textRegion.address;
}

uint64_t SharedCache::pathHash(const char* path)
{
    uint64_t sum = 0;
    for (const char* s=path; *s != '\0'; ++s)
        sum += sum*4 + *s;
    return sum;
}



void SharedCache::findDylibAndSegment(const void* contentPtr, std::string& dylibName, std::string& segName)
{
    uint64_t fileOffset = (uint8_t*)contentPtr - (uint8_t*)_buffer.get();
    for (const auto& entry : _segmentMap ) {
        const MachOProxy* dylib = entry.first;
        for (const SegmentInfo& segInfo : entry.second) {
            //fprintf(stderr, "   cacheFileOffset=0x%08llX, end=0x%08llX\n", segInfo.cacheFileOffset, segInfo.cacheFileOffset+segInfo.base->size);
            if ( (segInfo.cacheFileOffset <= fileOffset) && (fileOffset < segInfo.cacheFileOffset+segInfo.base->size) ) {
                dylibName = dylib->installName;
                segName = segInfo.base->name;
                return;
            }
        }
    }
    dylibName = "???";
    segName = "???";
}


template <typename P>
bool SharedCache::makeRebaseChain(uint8_t* pageContent, uint16_t lastLocationOffset, uint16_t offset, const dyldCacheSlideInfo2<typename P::E>* info)
{
    typedef typename P::uint_t     pint_t;

    const pint_t   deltaMask    = (pint_t)(info->delta_mask());
    const pint_t   valueMask    = ~deltaMask;
    const pint_t   valueAdd     = (pint_t)(info->value_add());
    const unsigned deltaShift   = __builtin_ctzll(deltaMask) - 2;
    const uint32_t maxDelta     = (uint32_t)(deltaMask >> deltaShift);

    pint_t* lastLoc = (pint_t*)&pageContent[lastLocationOffset+0];
    pint_t lastValue = (pint_t)P::getP(*lastLoc);
    if ( (lastValue - valueAdd) & deltaMask ) {
        std::string dylibName;
        std::string segName;
        findDylibAndSegment((void*)pageContent, dylibName, segName);
        terminate("rebase pointer does not point within cache. lastOffset=0x%04X, seg=%s, dylib=%s\n",
                    lastLocationOffset, segName.c_str(), dylibName.c_str());
    }
    if ( offset <= (lastLocationOffset+maxDelta) ) {
        // previous location in range, make link from it
        // encode this location into last value
        pint_t delta = offset - lastLocationOffset;
        pint_t newLastValue = ((lastValue - valueAdd) & valueMask) | (delta << deltaShift);
        //warning("  add chain: delta = %d, lastOffset=0x%03X, offset=0x%03X, org value=0x%08lX, new value=0x%08lX",
        //                    offset - lastLocationOffset, lastLocationOffset, offset, (long)lastValue, (long)newLastValue);
        P::setP(*lastLoc, newLastValue);
        return true;
    }
    //warning("  too big delta = %d, lastOffset=0x%03X, offset=0x%03X", offset - lastLocationOffset, lastLocationOffset, offset);

    // distance between rebase locations is too far
    // see if we can make a chain from non-rebase locations
    uint16_t nonRebaseLocationOffsets[1024];
    unsigned nrIndex = 0;
    for (uint16_t i = lastLocationOffset; i < offset-maxDelta; ) {
        nonRebaseLocationOffsets[nrIndex] = 0;
        for (int j=maxDelta; j > 0; j -= 4) {
            pint_t value = (pint_t)P::getP(*(pint_t*)&pageContent[i+j]);
            if ( value == 0 ) {
                // Steal values of 0 to be used in the rebase chain
                nonRebaseLocationOffsets[nrIndex] = i+j;
                break;
            }
        }
        if ( nonRebaseLocationOffsets[nrIndex] == 0 ) {
            lastValue = (pint_t)P::getP(*lastLoc);
            pint_t newValue = ((lastValue - valueAdd) & valueMask);
            //warning("   no way to make non-rebase delta chain, terminate off=0x%03X, old value=0x%08lX, new value=0x%08lX", lastLocationOffset, (long)value, (long)newValue);
            P::setP(*lastLoc, newValue);
            return false;
        }
        i = nonRebaseLocationOffsets[nrIndex];
        ++nrIndex;
    }

    // we can make chain. go back and add each non-rebase location to chain
    uint16_t prevOffset = lastLocationOffset;
    pint_t* prevLoc = (pint_t*)&pageContent[prevOffset];
    for (int n=0; n < nrIndex; ++n) {
        uint16_t nOffset = nonRebaseLocationOffsets[n];
        assert(nOffset != 0);
        pint_t* nLoc = (pint_t*)&pageContent[nOffset];
        uint32_t delta2 = nOffset - prevOffset;
        pint_t value = (pint_t)P::getP(*prevLoc);
        pint_t newValue;
        if ( value == 0 )
            newValue = (delta2 << deltaShift);
        else
            newValue = ((value - valueAdd) & valueMask) | (delta2 << deltaShift);
        //warning("    non-rebase delta = %d, to off=0x%03X, old value=0x%08lX, new value=0x%08lX", delta2, nOffset, (long)value, (long)newValue);
        P::setP(*prevLoc, newValue);
        prevOffset = nOffset;
        prevLoc = nLoc;
    }
    uint32_t delta3 = offset - prevOffset;
    pint_t value = (pint_t)P::getP(*prevLoc);
    pint_t newValue;
    if ( value == 0 )
        newValue = (delta3 << deltaShift);
    else
        newValue = ((value - valueAdd) & valueMask) | (delta3 << deltaShift);
    //warning("    non-rebase delta = %d, to off=0x%03X, old value=0x%08lX, new value=0x%08lX", delta3, offset, (long)value, (long)newValue);
    P::setP(*prevLoc, newValue);

    return true;
}


template <typename P>
void SharedCache::addPageStarts(uint8_t* pageContent, const bool bitmap[], const dyldCacheSlideInfo2<typename P::E>* info,
                                std::vector<uint16_t>& pageStarts, std::vector<uint16_t>& pageExtras)
{
    typedef typename P::uint_t     pint_t;

    const pint_t   deltaMask    = (pint_t)(info->delta_mask());
    const pint_t   valueMask    = ~deltaMask;
    const uint32_t pageSize     = info->page_size();
    const pint_t   valueAdd     = (pint_t)(info->value_add());

    uint16_t startValue = DYLD_CACHE_SLIDE_PAGE_ATTR_NO_REBASE;
    uint16_t lastLocationOffset = 0xFFFF;
    for(int i=0; i < pageSize/4; ++i) {
        unsigned offset = i*4;
        if ( bitmap[i] ) {
            if ( startValue == DYLD_CACHE_SLIDE_PAGE_ATTR_NO_REBASE ) {
                // found first rebase location in page
                startValue = i;
            }
            else if ( !makeRebaseChain<P>(pageContent, lastLocationOffset, offset, info) ) {
                // can't record all rebasings in one chain
                if ( (startValue & DYLD_CACHE_SLIDE_PAGE_ATTR_EXTRA) == 0 ) {
                    // switch page_start to "extras" which is a list of chain starts
                    unsigned indexInExtras = (unsigned)pageExtras.size();
                    if ( indexInExtras > 0x3FFF )
                        terminate("rebase overflow in page extras");
                    pageExtras.push_back(startValue);
                    startValue = indexInExtras | DYLD_CACHE_SLIDE_PAGE_ATTR_EXTRA;
                }
                pageExtras.push_back(i);
            }
            lastLocationOffset = offset;
        }
    }
    if ( lastLocationOffset != 0xFFFF ) {
        // mark end of chain
        pint_t* lastLoc = (pint_t*)&pageContent[lastLocationOffset];
        pint_t lastValue = (pint_t)P::getP(*lastLoc);
        pint_t newValue = ((lastValue - valueAdd) & valueMask);
        P::setP(*lastLoc, newValue);
    }
    if ( startValue & DYLD_CACHE_SLIDE_PAGE_ATTR_EXTRA ) {
        // add end bit to extras
        pageExtras.back() |= DYLD_CACHE_SLIDE_PAGE_ATTR_END;
    }
    pageStarts.push_back(startValue);
}

template <typename P>
void SharedCache::writeSlideInfoV2(uint64_t deltaMask, uint64_t valueAdd)
{
    // i386 cache does not support sliding because stubs use absolute addressing (text relocs)
    if (_arch.arch == CPU_TYPE_I386 ) {
        dyldCacheHeader<LittleEndian>* header = (dyldCacheHeader<LittleEndian>*)_buffer.get();
        header->set_slideInfoSize(0);
        return;
    }

    typedef typename P::E         E;
    const uint32_t pageSize = 4096;

    // build one 1024/4096 bool bitmap per page (4KB/16KB) of DATA
    uint8_t* const dataStart = (uint8_t*)_buffer.get() + _dataRegion.fileOffset;
    uint8_t* const dataEnd   = dataStart + _dataRegion.size;
    unsigned pageCount = (unsigned)(_dataRegion.size+pageSize-1)/pageSize;
    const long bitmapSize = pageCount*(pageSize/4)*sizeof(bool);
    bool* bitmap = (bool*)calloc(bitmapSize, 1);
    for (void* p : _pointersForASLR) {
        if ( (p < dataStart) || ( p > dataEnd) )
            terminate("DATA pointer for sliding, out of range\n");
        long byteOffset = (long)((uint8_t*)p - dataStart);
        if ( (byteOffset % 4) != 0 )
            terminate("pointer not 4-byte aligned in DATA offset 0x%08lX\n", byteOffset);
        long boolIndex = byteOffset / 4;
        // work around <rdar://24941083> by ignoring pointers to be slid that are NULL on disk
        if ( *(typename P::uint_t*)p == 0 ) {
            std::string dylibName;
            std::string segName;
            findDylibAndSegment(p, dylibName, segName);
            warning("NULL pointer asked to be slid in %s of %s", segName.c_str(), dylibName.c_str());
            continue;
        }
        bitmap[boolIndex] = true;
    }

    // fill in fixed info
    dyldCacheSlideInfo2<E>* info = (dyldCacheSlideInfo2<E>*)((uint8_t*)_buffer.get() + _slideInfoFileOffset);
    info->set_version(2);
    info->set_page_size(pageSize);
    info->set_delta_mask(deltaMask);
    info->set_value_add(valueAdd);

    // set page starts and extras for each page
    std::vector<uint16_t> pageStarts;
    std::vector<uint16_t> pageExtras;
    pageStarts.reserve(pageCount);
    uint8_t* pageContent = dataStart;;
    const bool* bitmapForPage = bitmap;
    for (unsigned i=0; i < pageCount; ++i) {
        //warning("page[%d]", i);
        addPageStarts<P>(pageContent, bitmapForPage, info, pageStarts, pageExtras);
        pageContent += pageSize;
        bitmapForPage += (sizeof(bool)*(pageSize/4));
    }
    free((void*)bitmap);

    // fill in computed info
    info->set_page_starts_offset(sizeof(dyldCacheSlideInfo2<E>));
    info->set_page_starts_count((unsigned)pageStarts.size());
    info->set_page_extras_offset((unsigned)(sizeof(dyldCacheSlideInfo2<E>)+pageStarts.size()*sizeof(uint16_t)));
    info->set_page_extras_count((unsigned)pageExtras.size());
    for (unsigned i=0; i < pageStarts.size(); ++i)
        info->set_page_starts(i, pageStarts[i]);
    for (unsigned i=0; i < pageExtras.size(); ++i)
        info->set_page_extras(i, pageExtras[i]);
    //warning("pageCount=%u, page_starts_count=%lu, page_extras_count=%lu", pageCount, pageStarts.size(), pageExtras.size());
    _slideInfoBufferSize = align(info->page_extras_offset() + pageExtras.size()*sizeof(uint16_t), 12);

#if NEW_CACHE_FILE_FORMAT

#else
    unsigned long slideInfoPageSize = align(_slideInfoBufferSize, sharedRegionRegionAlignment(_arch));
    dyldCacheHeader<E>* header = (dyldCacheHeader<E>*)_buffer.get();
    header->set_slideInfoSize(slideInfoPageSize);
#endif
}

void SharedCache::writeSlideInfoV2(void)
{
     switch (_arch.arch) {
        case CPU_TYPE_ARM:
            // linked list based slide info needs high 3 bits of pointer, won't work with > 512MB of pointable content
            if ( (_textRegion.size + _dataRegion.size) > 512*1024*1024 ) {
                warning("cache TEXT+DATA > 512MB, using larger slide info format");
                writeSlideInfo<LittleEndian>();
            }
            else {
                writeSlideInfoV2<Pointer32<LittleEndian>>(0xE0000000, ARM_SHARED_REGION_START);
            }
            break;
        case CPU_TYPE_I386:
            writeSlideInfoV2<Pointer32<LittleEndian>>(0xE0000000, 0x90000000);
            break;
        case CPU_TYPE_X86_64:
            writeSlideInfoV2<Pointer64<LittleEndian>>(0xFFFF000000000000, 0);
            break;
        case CPU_TYPE_ARM64:
            writeSlideInfoV2<Pointer64<LittleEndian>>(0x00FFFF0000000000, 0);
            break;
        default:
            warning("unsupported arch 0x%08X", _arch.arch);
            return;
    }
}



template <typename E>
void SharedCache::writeSlideInfo(void)
{
    // i386 cache does not support sliding because stubs use absolute addressing (text relocs)
    if (_arch.arch == CPU_TYPE_I386 ) {
        dyldCacheHeader<E>* header = (dyldCacheHeader<E>*)_buffer.get();
        header->set_slideInfoSize(0);
        return;
    }

    // build one 128-byte bitmap per page (4096) of DATA
    uint8_t* const dataStart = (uint8_t*)_buffer.get() + _dataRegion.fileOffset;
    uint8_t* const dataEnd   = dataStart + _dataRegion.size;
    const long bitmapSize = (dataEnd - dataStart)/(4*8);
    uint8_t* bitmap = (uint8_t*)calloc(bitmapSize, 1);
    for (void* p : _pointersForASLR) {
        if ( (p < dataStart) || ( p > dataEnd) )
            terminate("DATA pointer for sliding, out of range\n");
        long offset = (long)((uint8_t*)p - dataStart);
        if ( (offset % 4) != 0 )
            terminate("pointer not 4-byte aligned in DATA offset 0x%08lX\n", offset);
        long byteIndex = offset / (4*8);
        long bitInByte =  (offset % 32) >> 2;
        bitmap[byteIndex] |= (1 << bitInByte);
    }

    // allocate worst case size block of all slide info
    const unsigned entry_size = 4096/(8*4); // 8 bits per byte, possible pointer every 4 bytes.
    const unsigned toc_count = (unsigned)bitmapSize/entry_size;
    dyldCacheSlideInfo<E>* slideInfo = (dyldCacheSlideInfo<E>*)((uint8_t*)_buffer.get() + _slideInfoFileOffset);
    slideInfo->set_version(1);
    slideInfo->set_toc_offset(sizeof(dyldCacheSlideInfo<E>));
    slideInfo->set_toc_count(toc_count);
    slideInfo->set_entries_offset((slideInfo->toc_offset()+2*toc_count+127)&(-128));
    slideInfo->set_entries_count(0);
    slideInfo->set_entries_size(entry_size);
    // append each unique entry
    const dyldCacheSlideInfoEntry* bitmapAsEntries = (dyldCacheSlideInfoEntry*)bitmap;
    dyldCacheSlideInfoEntry* const entriesInSlidInfo = (dyldCacheSlideInfoEntry*)((char*)slideInfo+slideInfo->entries_offset());
    int entry_count = 0;
    for (int i=0; i < toc_count; ++i) {
        const dyldCacheSlideInfoEntry* thisEntry = &bitmapAsEntries[i];
        // see if it is same as one already added
        bool found = false;
        for (int j=0; j < entry_count; ++j) {
            if ( memcmp(thisEntry, &entriesInSlidInfo[j], entry_size) == 0 ) {
                slideInfo->set_toc(i, j);
                found = true;
                break;
            }
        }
        if ( !found ) {
            // append to end
            memcpy(&entriesInSlidInfo[entry_count], thisEntry, entry_size);
            slideInfo->set_toc(i, entry_count++);
        }
    }
    slideInfo->set_entries_count(entry_count);
    ::free((void*)bitmap);

#if NEW_CACHE_FILE_FORMAT

#else
    unsigned long slideInfoPageSize = align(slideInfo->entries_offset() + entry_count*entry_size, sharedRegionRegionAlignment(_arch));
    dyldCacheHeader<E>* header = (dyldCacheHeader<E>*)_buffer.get();
    header->set_slideInfoSize(slideInfoPageSize);
#endif
}

template <typename P>
void SharedCache::writeCacheHeader(void)
{
#if NEW_CACHE_FILE_FORMAT
    macho_header<P>* mh = (macho_header<P>*)cacheBuffer;
    mh->set_magic((sizeof(typename P::uint_t) == 8) ? MH_MAGIC_64 : MH_MAGIC);
    mh->set_cputype(arch.arch);
    mh->set_cpusubtype(arch.subtype);
    mh->set_filetype(MH_DYLIB);
    mh->set_ncmds(0);
    mh->set_sizeofcmds(0);
    mh->set_flags(0);

    uint8_t* cmd = (uint8_t*)cacheBuffer + sizeof(macho_header<P>);

    // write LC_SEGMENT for each region
    macho_segment_command<P>* rxSegCmd = (macho_segment_command<P>*)cmd;
    rxSegCmd->set_cmd(macho_segment_command<P>::CMD);
    rxSegCmd->set_cmdsize(sizeof(macho_segment_command<P>));
    rxSegCmd->set_segname("R.X");
    rxSegCmd->set_vmaddr(_textRegion.address);
    rxSegCmd->set_vmsize(_textRegion.size);
    rxSegCmd->set_fileoff(_textRegion.fileOffset);
    rxSegCmd->set_filesize(_textRegion.size);
    rxSegCmd->set_maxprot(VM_PROT_READ | VM_PROT_EXECUTE);
    rxSegCmd->set_initprot(VM_PROT_READ | VM_PROT_EXECUTE);
    rxSegCmd->set_nsects(0);
    rxSegCmd->set_flags(0);
    mh->set_ncmds(mh->ncmds()+1);
    mh->set_sizeofcmds(mh->sizeofcmds()+rxSegCmd->cmdsize());
    cmd += rxSegCmd->cmdsize();

    macho_segment_command<P>* rwSegCmd = (macho_segment_command<P>*)cmd;
    rwSegCmd->set_cmd(macho_segment_command<P>::CMD);
    rwSegCmd->set_cmdsize(sizeof(macho_segment_command<P>));
    rwSegCmd->set_segname("RW.");
    rwSegCmd->set_vmaddr(_dataRegion.address);
    rwSegCmd->set_vmsize(_dataRegion.size);
    rwSegCmd->set_fileoff(_dataRegion.fileOffset);
    rwSegCmd->set_filesize(_dataRegion.size);
    rwSegCmd->set_maxprot(VM_PROT_READ | VM_PROT_WRITE);
    rwSegCmd->set_initprot(VM_PROT_READ | VM_PROT_WRITE);
    rwSegCmd->set_nsects(0);
    rwSegCmd->set_flags(0);
    mh->set_ncmds(mh->ncmds()+1);
    mh->set_sizeofcmds(mh->sizeofcmds()+rwSegCmd->cmdsize());
    cmd += rwSegCmd->cmdsize();

    macho_segment_command<P>* roSegCmd = (macho_segment_command<P>*)cmd;
    roSegCmd->set_cmd(macho_segment_command<P>::CMD);
    roSegCmd->set_cmdsize(sizeof(macho_segment_command<P>));
    roSegCmd->set_segname("R..");
    roSegCmd->set_vmaddr(_readOnlyRegion.address);
    roSegCmd->set_vmsize(_readOnlyRegion.size);
    roSegCmd->set_fileoff(_readOnlyRegion.fileOffset);
    roSegCmd->set_filesize(_readOnlyRegion.size);
    roSegCmd->set_maxprot(VM_PROT_READ);
    roSegCmd->set_initprot(VM_PROT_READ);
    roSegCmd->set_nsects(0);
    roSegCmd->set_flags(0);
    mh->set_ncmds(mh->ncmds()+1);
    mh->set_sizeofcmds(mh->sizeofcmds()+roSegCmd->cmdsize());
    cmd += roSegCmd->cmdsize();

    // Add LC_ID_DYLIB
    macho_dylib_command<P>* dylibIdCmd = (macho_dylib_command<P>*)cmd;
    const char* installName = "/System/Library/Frameworks/OS.framework/OS"; // FIXME
    uint32_t sz = (uint32_t)align(sizeof(macho_dylib_command<P>) + strlen(installName) + 1, 3);
    dylibIdCmd->set_cmd(LC_ID_DYLIB);
    dylibIdCmd->set_cmdsize(sz);
    dylibIdCmd->set_name_offset();
    dylibIdCmd->set_timestamp(1);
    dylibIdCmd->set_current_version(0x10000);
    dylibIdCmd->set_compatibility_version(0x10000);
    strcpy((char*)&cmd[sizeof(macho_dylib_command<P>)], installName);
    mh->set_ncmds(mh->ncmds()+1);
    mh->set_sizeofcmds(mh->sizeofcmds()+sz);
    cmd += dylibIdCmd->cmdsize();

    // Add LC_UUID
    macho_uuid_command<P>* uuidCmd = (macho_uuid_command<P>*)cmd;
    uint8_t zeros[16];
    bzero(zeros, 16);
    uuidCmd->set_cmd(LC_UUID);
    uuidCmd->set_cmdsize(sizeof(macho_uuid_command<P>));
    uuidCmd->set_uuid(zeros);
    cmd += uuidCmd->cmdsize();

    // Build dylib trie
    std::vector<mach_o::trie::Entry> dylibTrieEntires;
    int pathLengths = 0;
    for (Extra* ex : _sortedDylibs) {
        mach_o::trie::Entry entry;
        entry.name = ex->proxy->installName.c_str();
        entry.address = ex->segments[0].address;
        entry.flags = 0;
        entry.other = 0;
        entry.importName = NULL;
        dylibTrieEntires.push_back(entry);
        pathLengths += (strlen(entry.name) + 1);
        for (const std::string& alias : ex->proxy->installNameAliases) {
            mach_o::trie::Entry aliasEntry;
            aliasEntry.name = alias.c_str();
            aliasEntry.address = ex->segments[0].address;
            aliasEntry.flags = 0;
            aliasEntry.other = 0;
            aliasEntry.importName = NULL;
            dylibTrieEntires.push_back(aliasEntry);
            pathLengths += (strlen(aliasEntry.name) + 1);
        }
    }
    std::vector<uint8_t> dylibTrieBytes;
    dylibTrieBytes.reserve(4096);
    mach_o::trie::makeTrie(dylibTrieEntires, dylibTrieBytes);
    fprintf(stderr, "dylib trie size = %lu bytes, for %lu entries, pathLength=%d\n", dylibTrieBytes.size(), dylibTrieEntires.size(), pathLengths);



    // Build SPI trie (optimized cache only)



    // add LC_CODE_SIGNATURE
    macho_linkedit_data_command<P>* codeSigCmd = (macho_linkedit_data_command<P>*)cmd;
    codeSigCmd->set_cmd(LC_CODE_SIGNATURE);
    codeSigCmd->set_cmdsize(sizeof(macho_linkedit_data_command<P>));
    codeSigCmd->set_dataoff((uint32_t)(_readOnlyRegion.fileOffset + _readOnlyRegion.size));
    codeSigCmd->set_datasize(0);    // FIXME
    mh->set_ncmds(mh->ncmds()+1);
    mh->set_sizeofcmds(mh->sizeofcmds()+codeSigCmd->cmdsize());
    cmd += codeSigCmd->cmdsize();

#else
    typedef typename P::E   E;
    // fill in header
    uint8_t* buffer = (uint8_t*)_buffer.get();
    dyldCacheHeader<E>* header = (dyldCacheHeader<E>*)_buffer.get();;
    // "dyld_v1" + spaces + archName(), with enough spaces to pad to 15 bytes
    std::string magic = "dyld_v1";
    magic.append(15 - magic.length() - archName().length(), ' ');
    magic.append(archName());
    assert(magic.length() == 15);
    header->set_magic(magic.c_str());
    header->set_mappingOffset(sizeof(dyldCacheHeader<E>));
    header->set_mappingCount(3);
    header->set_imagesOffset((uint32_t)(header->mappingOffset() + 3*sizeof(dyldCacheFileMapping<E>) + sizeof(uint64_t)*_branchPoolStarts.size()));
    header->set_imagesCount((uint32_t)_dylibs.size() + _aliasCount);
    header->set_dyldBaseAddress(0);
    header->set_codeSignatureOffset(_fileSize);
    header->set_codeSignatureSize(0);
    header->set_slideInfoOffset(_slideInfoFileOffset);
    header->set_slideInfoSize(_slideInfoBufferSize);
    header->set_localSymbolsOffset(0);
    header->set_localSymbolsSize(0);
    header->set_cacheType(kDyldSharedCacheTypeDevelopment);
    header->set_accelerateInfoAddr(0);
    header->set_accelerateInfoSize(0);
    static const uint8_t zero_uuid[16] = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
    header->set_uuid(zero_uuid); // overwritten later by recomputeCacheUUID()
    header->set_branchPoolsOffset(header->mappingOffset() + 3*sizeof(dyldCacheFileMapping<E>));
    header->set_branchPoolsCount((uint32_t)_branchPoolStarts.size());
    header->set_imagesTextOffset(0);
    header->set_imagesTextCount(_dylibs.size());

    // fill in mappings
    dyldCacheFileMapping<E>* mappings = (dyldCacheFileMapping<E>*)&buffer[header->mappingOffset()];
    mappings[0].set_address(_textRegion.address);
    mappings[0].set_size(_textRegion.size);
    mappings[0].set_file_offset(_textRegion.fileOffset);
    mappings[0].set_max_prot(_textRegion.prot);
    mappings[0].set_init_prot(_textRegion.prot);

    mappings[1].set_address(_dataRegion.address);
    mappings[1].set_size(_dataRegion.size);
    mappings[1].set_file_offset(_dataRegion.fileOffset);
    mappings[1].set_max_prot(_dataRegion.prot);
    mappings[1].set_init_prot(_dataRegion.prot);

    mappings[2].set_address(_readOnlyRegion.address);
    mappings[2].set_size(_readOnlyRegion.size);
    mappings[2].set_file_offset(_readOnlyRegion.fileOffset);
    mappings[2].set_max_prot(_readOnlyRegion.prot);
    mappings[2].set_init_prot(_readOnlyRegion.prot);

    // fill in branch pool addresses
    uint64_t* p = (uint64_t*)&buffer[header->branchPoolsOffset()];
    for (uint64_t pool : _branchPoolStarts) {
        E::set64(*p, pool);
        ++p;
    }

    // fill in image table
    dyldCacheImageInfo<E>* images = (dyldCacheImageInfo<E>*)&buffer[header->imagesOffset()];
    for (auto& dylib : _dylibs) {
        auto textSeg = _segmentMap[dylib][0];
        images->set_address(textSeg.address);
        if (_manifest.platform == "osx") {
            images->set_modTime(dylib->lastModTime);
            images->set_inode(dylib->inode);
        } else {
            images->set_modTime(0);
            images->set_inode(pathHash(dylib->installName.c_str()));
        }
        images->set_pathFileOffset((uint32_t)textSeg.cacheFileOffset + dylib->installNameOffsetInTEXT);
        ++images;
    }
    // append aliases image records and strings
    uint32_t offset = header->imagesOffset() + header->imagesCount()*sizeof(dyld_cache_image_info);
    for (auto &dylib : _dylibs) {
        if (!dylib->installNameAliases.empty()) {
            for (const std::string& alias : dylib->installNameAliases) {
                images->set_address(_segmentMap[dylib][0].address);
                if (_manifest.platform == "osx") {
                    images->set_modTime(dylib->lastModTime);
                    images->set_inode(dylib->inode);
                } else {
                    images->set_modTime(0);
                    images->set_inode(pathHash(alias.c_str()));
                }
                images->set_pathFileOffset(offset);
                ::strcpy((char*)&buffer[offset], alias.c_str());
                offset += alias.size() + 1;
                ++images;
            }
        }
    }

    // calculate start of text image array and trailing string pool
    offset = (offset + 15) & (-16);
    header->set_imagesTextOffset(offset);
    dyldCacheImageTextInfo<E>* textImages = (dyldCacheImageTextInfo<E>*)&buffer[header->imagesTextOffset()];
    uint32_t stringOffset = offset + (uint32_t)(sizeof(dyldCacheImageTextInfo<E>) * _dylibs.size());

    // write text image array and image names pool at same time
    for (auto& dylib : _dylibs) {
        textImages->set_uuid(dylib->uuid);
        textImages->set_loadAddress(_segmentMap[dylib][0].address);
        textImages->set_textSegmentSize((uint32_t)dylib->segments[0].size);
        textImages->set_pathOffset(stringOffset);
        ::strcpy((char*)&buffer[stringOffset], dylib->installName.c_str());
        stringOffset += dylib->installName.size()+1;
        ++textImages;
    }

    assert(stringOffset < 0x28000);
#endif
}

void SharedCache::rebase(MachOProxy* dylib)
{
    std::vector<uint64_t> segNewStartAddresses;
    std::vector<uint64_t> segCacheFileOffsets;
    std::vector<uint64_t> segCacheFileSizes;
    for (auto& seg : _segmentMap[dylib]) {
        segNewStartAddresses.push_back(seg.address);
        segCacheFileOffsets.push_back(seg.cacheFileOffset);
        segCacheFileSizes.push_back(seg.cacheSegSize);
    }
    adjustImageForNewSegmentLocations(segNewStartAddresses, segCacheFileOffsets, segCacheFileSizes, _pointersForASLR);
}


void SharedCache::rebaseAll(void)
{
    for (auto& dylib : _dylibs)
        rebase(dylib);
}

void SharedCache::bindAll(void)
{
    std::unordered_map<std::string, void*> dylibPathToMachHeader;
    for (auto& dylib : _dylibs) {
        void* mh = (uint8_t*)_buffer.get() + _segmentMap[dylib][0].cacheFileOffset;
        dylibPathToMachHeader[dylib->installName] = mh;
        for (const std::string& path : dylib->installNameAliases) {
            if (path != dylib->installName) {
                dylibPathToMachHeader[path] = mh;
            }
        }
    }

    bindAllImagesInCache(dylibPathToMachHeader, _pointersForASLR);
}

void SharedCache::writeCacheSegments(void)
{
    uint8_t* cacheBytes = (uint8_t*)_buffer.get();
    for (auto& dylib : _dylibs) {
        struct stat stat_buf;
        const uint8_t* srcDylib;
        bool rootless;

        std::tie(srcDylib, stat_buf, rootless) = fileCache.cacheLoad(dylib->path);
        for (auto& seg : _segmentMap[dylib]) {
            uint32_t segFileOffset = dylib->fatFileOffset + seg.base->fileOffset;
            uint64_t copySize = std::min(seg.cacheSegSize, (uint64_t)seg.base->diskSize);
            verboseLog("copy segment %12s (0x%08llX bytes) to %p (logical addr 0x%llX) for %s", seg.base->name.c_str(), copySize, &cacheBytes[seg.cacheFileOffset], seg.address, dylib->installName.c_str());
            ::memcpy(&cacheBytes[seg.cacheFileOffset], &srcDylib[segFileOffset], copySize);
        }
    }
}



void SharedCache::appendCodeSignature(const std::string& suffix)
{
    // select which codesigning hash
    uint8_t  dscHashType     = CS_HASHTYPE_SHA1;
    uint8_t  dscHashSize     = CS_HASH_SIZE_SHA1;
    uint32_t dscDigestFormat = kCCDigestSHA1;
    if ( _manifest.platform == "osx" ) {
        dscHashType     = CS_HASHTYPE_SHA256;
        dscHashSize     = CS_HASH_SIZE_SHA256;
        dscDigestFormat = kCCDigestSHA256;
    }

    std::string cacheIdentifier = "com.apple.dyld.cache." + archName() + "." + suffix;
    // get pointers into shared cache buffer
    size_t          inBbufferSize = _fileSize;
    const uint8_t*  inBuffer = (uint8_t*)_buffer.get();
    uint8_t*        csBuffer = (uint8_t*)_buffer.get()+inBbufferSize;

    // layout code signature contents
    size_t   idSize     = cacheIdentifier.size()+1; // +1 for terminating 0
    uint32_t slotCount  = (uint32_t)((inBbufferSize + CS_PAGE_SIZE - 1) / CS_PAGE_SIZE);
    uint32_t xSlotCount = CSSLOT_REQUIREMENTS;
    size_t   scatOffset = sizeof(CS_CodeDirectory);
    size_t   scatSize   = 4*sizeof(CS_Scatter);  // only 3 used??
    size_t   idOffset   = scatOffset+scatSize;
    size_t   hashOffset = idOffset+idSize + dscHashSize*xSlotCount;
    size_t   cdSize     = hashOffset + (slotCount * dscHashSize);
    size_t   reqsSize   = 12;
    size_t   cmsSize    = sizeof(CS_Blob);
    size_t   cdOffset   = sizeof(CS_SuperBlob) + 3*sizeof(CS_BlobIndex);
    size_t   reqsOffset = cdOffset + cdSize;
    size_t   cmsOffset  = reqsOffset + reqsSize;
    size_t   sbSize     = cmsOffset + cmsSize;
    size_t   sigSize    = align(sbSize, 14);       // keep whole cache 16KB aligned

    // create overall code signature which is a superblob
    CS_SuperBlob* sb = reinterpret_cast<CS_SuperBlob*>(csBuffer);
    sb->magic           = htonl(CSMAGIC_EMBEDDED_SIGNATURE);
    sb->length          = htonl(sbSize);
    sb->count           = htonl(3);
    sb->index[0].type   = htonl(CSSLOT_CODEDIRECTORY);
    sb->index[0].offset = htonl(cdOffset);
    sb->index[1].type   = htonl(CSSLOT_REQUIREMENTS);
    sb->index[1].offset = htonl(reqsOffset);
    sb->index[2].type   = htonl(CSSLOT_CMS_SIGNATURE);
    sb->index[2].offset = htonl(cmsOffset);

    // initialize fixed fields of Code Directory
    CS_CodeDirectory* cd = (CS_CodeDirectory*)(((char*)sb)+cdOffset);
    cd->magic           = htonl(CSMAGIC_CODEDIRECTORY);
    cd->length          = htonl(cdSize);
    cd->version         = htonl(0x20100);
    cd->flags           = htonl(kSecCodeSignatureAdhoc);
    cd->hashOffset      = htonl(hashOffset);
    cd->identOffset     = htonl(idOffset);
    cd->nSpecialSlots   = htonl(xSlotCount);
    cd->nCodeSlots      = htonl(slotCount);
    cd->codeLimit       = htonl(inBbufferSize);
    cd->hashSize        = dscHashSize;
    cd->hashType        = dscHashType;
    cd->platform        = 0;                            // not platform binary
    cd->pageSize        = __builtin_ctz(CS_PAGE_SIZE);  // log2(CS_PAGE_SIZE);
    cd->spare2          = 0;                            // unused (must be zero)
    cd->scatterOffset   = htonl(scatOffset);

    // initialize dynamic fields of Code Directory
    strcpy((char*)cd + idOffset, cacheIdentifier.c_str());

    // add scatter info
    CS_Scatter* scatter = reinterpret_cast<CS_Scatter*>((char*)cd+scatOffset);
    scatter[0].count        = htonl(_textRegion.size/CS_PAGE_SIZE);
    scatter[0].base         = htonl(_textRegion.fileOffset/CS_PAGE_SIZE);
    scatter[0].targetOffset = htonll(_textRegion.address);
    scatter[0].spare        = 0;
    scatter[1].count        = htonl(_dataRegion.size/CS_PAGE_SIZE);
    scatter[1].base         = htonl(_dataRegion.fileOffset/CS_PAGE_SIZE);
    scatter[1].targetOffset = htonll(_dataRegion.address);
    scatter[1].spare        = 0;
    scatter[2].count        = htonl(_readOnlyRegion.size/CS_PAGE_SIZE);
    scatter[2].base         = htonl(_readOnlyRegion.fileOffset/CS_PAGE_SIZE);
    scatter[2].targetOffset = htonll(_readOnlyRegion.address);
    scatter[2].spare        = 0;

    // fill in empty requirements
    CS_RequirementsBlob* reqs = (CS_RequirementsBlob*)(((char*)sb)+reqsOffset);
    reqs->magic  = htonl(CSMAGIC_REQUIREMENTS);
    reqs->length = htonl(sizeof(CS_RequirementsBlob));
    reqs->data   = 0;

    // fill in empty CMS blob for ad-hoc signing
    CS_Blob* cms = (CS_Blob*)(((char*)sb)+cmsOffset);
    cms->magic  = htonl(CSMAGIC_BLOBWRAPPER);
    cms->length = htonl(sizeof(CS_Blob));

    // add special slot hashes
    uint8_t* hashSlot = (uint8_t*)cd + hashOffset;
    uint8_t* reqsHashSlot = &hashSlot[-CSSLOT_REQUIREMENTS*dscHashSize];
    CCDigest(dscDigestFormat, (uint8_t*)reqs, sizeof(CS_RequirementsBlob), reqsHashSlot);

    // alter header of cache to record size and location of code signature
    // do this *before* hashing each page
    dyldCacheHeader<LittleEndian>* header = (dyldCacheHeader<LittleEndian>*)inBuffer;
    header->set_codeSignatureOffset(inBbufferSize);
    header->set_codeSignatureSize(sigSize);

    // compute hashes
    const uint8_t* code = inBuffer;
    for (uint32_t i=0; i < slotCount; ++i) {
        CCDigest(dscDigestFormat, code, CS_PAGE_SIZE, hashSlot);
        hashSlot += dscHashSize;
        code += CS_PAGE_SIZE;
    }

    // hash of entire code directory (cdHash) uses same has hash as each page
    uint8_t fullCdHash[dscHashSize];
    CCDigest(dscDigestFormat, (const uint8_t*)cd, cdSize, fullCdHash);
    // Note: cdHash is defined as first 20 bytes of hash
    memcpy(_cdHash, fullCdHash, 20);

    // increase file size to include newly append code signature
    _fileSize += sigSize;
}

std::string SharedCache::cdHashString()
{
	char buff[48];
	for (int i = 0; i < sizeof(_cdHash); ++i)
		sprintf(&buff[2*i], "%2.2x", _cdHash[i]);
	return buff;
}


#pragma mark -
#pragma mark Template dispatchers

#define TEMPLATE_DISPATCHER_BODY(method,...)                    \
    switch( _arch.arch ) {                                      \
        case CPU_TYPE_ARM:                                      \
        case CPU_TYPE_I386:                                     \
            method<Pointer32<LittleEndian>>(__VA_ARGS__);       \
            break;                                              \
        case CPU_TYPE_X86_64:                                   \
        case CPU_TYPE_ARM64:                                    \
            method<Pointer64<LittleEndian>>(__VA_ARGS__);       \
            break;                                              \
        default:                                                \
            terminate("unsupported arch 0x%08X", _arch.arch);   \
    }

void SharedCache::writeCacheHeader() {
    TEMPLATE_DISPATCHER_BODY(writeCacheHeader)
};

void SharedCache::buildForDevelopment(const std::string& cachePath) {
    TEMPLATE_DISPATCHER_BODY(buildForDevelopment, cachePath)
};

void SharedCache::buildForProduction(const std::string& cachePath) {
    TEMPLATE_DISPATCHER_BODY(buildForProduction, cachePath)
};

void SharedCache::setLinkeditsMappingEndFileOffset(uint64_t newFileSize) {
    TEMPLATE_DISPATCHER_BODY(setLinkeditsMappingEndFileOffset, newFileSize)
}

void SharedCache::setUnmappedLocalsRange(uint64_t localSymbolsOffset, uint32_t unmappedSize) {
    TEMPLATE_DISPATCHER_BODY(setUnmappedLocalsRange, localSymbolsOffset, unmappedSize)
}

void SharedCache::setAcceleratorInfoRange(uint64_t accelInfoAddr, uint32_t accelInfoSize) {
    TEMPLATE_DISPATCHER_BODY(setAcceleratorInfoRange, accelInfoAddr, accelInfoSize)
}

void SharedCache::recomputeCacheUUID(void) {
    TEMPLATE_DISPATCHER_BODY(recomputeCacheUUID)
}

void SharedCache::forEachImage(DylibHandler handler) {
    TEMPLATE_DISPATCHER_BODY(forEachImage, handler)
}

void SharedCache::forEachRegion(RegionHandler handler) {
    TEMPLATE_DISPATCHER_BODY(forEachRegion, handler)
}

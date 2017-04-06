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

#ifndef __MEGA_DYLIB_UTILS_H__
#define __MEGA_DYLIB_UTILS_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <dispatch/dispatch.h>

#include <memory>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

#include "CacheFileAbstraction.hpp"

#include "MachOFileAbstraction.hpp"

#include "Manifest.h"
#include "MachOProxy.h"

struct SharedCache;

struct FileCache {
    FileCache(void);
    std::tuple<uint8_t *, struct stat, bool> cacheLoad(const std::string path);
    void preflightCache(const std::string& path);
    void preflightCache(const std::unordered_set<std::string> &paths);
private:
    void fill(const std::string& path);

    std::unordered_map<std::string, std::tuple<uint8_t *, struct stat, bool>> entries;
    dispatch_queue_t cache_queue;
};

extern FileCache fileCache;

typedef std::pair<Manifest*,std::set<std::pair<std::string, std::string>>> WarningTargets;

inline uint64_t align(uint64_t addr, uint8_t p2)
{
    uint64_t mask = (1 << p2);
    return (addr + mask - 1) & (-mask);
}


uint64_t sharedRegionRegionSize(ArchPair arch);
uint8_t sharedRegionRegionAlignment(ArchPair arch);
ArchPair archForString(const std::string& archStr);
std::string stringForArch(ArchPair arch, bool allowUnknown = false);
std::string fallbackArchStringForArchString( const std::string& archStr );

struct SharedCache {
    struct SegmentInfo {
        SegmentInfo(const MachOProxy::Segment* seg)
        : base(seg), address(0), cacheFileOffset(0), cacheSegSize(0) { }

        const MachOProxy::Segment*  base;
        uint64_t                    address;
        uint64_t                    cacheFileOffset;
        uint64_t                    cacheSegSize;
    };

    struct Region {
        uint64_t    address;
        uint64_t    size;
        uint64_t    fileOffset;
        uint32_t    prot;
    };

    SharedCache(Manifest& manifest,
                const std::string& configuration, const std::string& architecture);

    // We do not need an explicit destructor despite our move/copy constructor because the resource the are dealing with is a
    // std::unique<void> which has a registered destructor

    //FIXME reminants of merging the two classes, unifyu these
    uint64_t vmSize() const { return _vmSize; }
    uint64_t fileSize() const { return _fileSize; }
    const uint8_t* cdHash() { return _cdHash; }
    std::string cdHashString();

    //FIXME: This should be private, but we can't do that until we move write out into the class after the next refactor
    std::shared_ptr<void> buffer() const;

    void buildForDevelopment(const std::string& cachePath);
    void buildForProduction(const std::string& cachePath);
    bool writeCacheMapFile(const std::string&  mapPath);

    typedef std::function<void(const void* machHeader, const char* installName, time_t lastModTime, ino_t inode,
                               const std::vector<MachOProxy::Segment>& segments)> DylibHandler;
    // Calls lambda once per image in the cache
    void forEachImage(DylibHandler handler);

    typedef std::function<void(void* content, uint64_t vmAddr, uint64_t size, uint32_t permissions)> RegionHandler;
    // Calls lambda once per region in the cache
    void forEachRegion(RegionHandler handler);

    void setLinkeditsMappingEndFileOffset(uint64_t newFileSize);
    void setAcceleratorInfoRange(uint64_t accelInfoAddr, uint32_t accelInfoSize);
    void setUnmappedLocalsRange(uint64_t localSymbolsOffset, uint32_t unmappedSize);
    void recomputeCacheUUID(void);

private:
    void loadDirtyDataOrderFile(const std::string& dirtyDataOrderFile);
    void sortDylibs(const std::string& dylibOrderFile);
    void assignSegmentAddresses();
    void bypassStubs(const std::vector<uint64_t>& branchPoolStartAddrs);
    void optimizeLinkedit(bool dontMapLocalSymbols, bool addAcceleratorTables, const std::vector<uint64_t>& branchPoolOffsets);

    // Once all a dylib's segments are copied into a cache, this function will adjust the contents of
    // the TEXT, DATA, and LINKEDIT segments in the cache to be correct for their new addresses.
    void bindAllImagesInCache(const std::unordered_map<std::string, void*>& dylibPathToMachHeader, std::vector<void*>& pointersForASLR);

    // After adjustImageForNewSegmentLocations() is called to rebase all segments, this function can be called to
    // bind all symbols to their new addresses
    void adjustImageForNewSegmentLocations(const std::vector<uint64_t>& segNewStartAddresses,
                                           const std::vector<uint64_t>& segCacheFileOffsets,
                                           const std::vector<uint64_t>& segCacheSizes, std::vector<void*>& pointersForASLR);

    uint64_t    pathHash(const char* path);
    void        writeCacheHeader(void);
    void        writeCacheSegments(void);
    void        rebaseAll(void);
    void        rebase(MachOProxy* dylib);
    void        bindAll(void);
    void        optimizeObjC(bool forProduction);
    void        writeSlideInfoV2(void);

    void buildUnoptimizedCache();
    void appendCodeSignature(const std::string& suffix);
    template <typename P> void buildForDevelopment(const std::string& cachePath);
    template <typename P> void buildForProduction(const std::string& cachePath);
    template <typename P> void forEachImage(DylibHandler handler);
    template <typename P> void forEachRegion(RegionHandler handler);
    template <typename P> void setLinkeditsMappingEndFileOffset(uint64_t newFileSize);
    template <typename P> void setAcceleratorInfoRange(uint64_t accelInfoAddr, uint32_t accelInfoSize);
    template <typename P> void setUnmappedLocalsRange(uint64_t localSymbolsOffset, uint32_t unmappedSize);
    template <typename P> void recomputeCacheUUID(void);
    template <typename P> void bypassStubs(const std::vector<uint64_t>& branchPoolStartAddrs);
    template <typename P> void optimizeLinkedit(bool dontMapLocalSymbols, bool addAcceleratorTables, const std::vector<uint64_t>& branchPoolOffsets);
    template <typename P> void writeCacheHeader(void);
    template <typename E> void writeSlideInfo(void);
    template <typename P> void writeSlideInfoV2(uint64_t deltaMask, uint64_t valueAdd);

    template <typename P> void findImplicitAliases(std::shared_ptr<MachOProxy> dylib);
    template <typename P> void addPageStarts(uint8_t* pageContent, const bool bitmap[], const dyldCacheSlideInfo2<typename P::E>* info,
                                             std::vector<uint16_t>& pageStarts, std::vector<uint16_t>& pageExtras);
    template <typename P> bool makeRebaseChain(uint8_t* pageContent, uint16_t lastLocationOffset, uint16_t newOffset, const dyldCacheSlideInfo2<typename P::E>* info);
                            void findDylibAndSegment(const void* contentPtr, std::string& dylibName, std::string& segName);

    ArchPair _arch;
    std::vector<MachOProxy *> _dylibs;
    std::shared_ptr<void> _buffer;
    std::unordered_map<const MachOProxy *, std::vector<SegmentInfo>>    _segmentMap;

    std::string                                 archName();

    Manifest&                                   _manifest;
    const Manifest::Architecture&               _archManifest;
    uint32_t                                    _aliasCount;
    Region                                      _textRegion;
    Region                                      _dataRegion;
    Region                                      _readOnlyRegion;
    uint64_t                                    _slideInfoFileOffset;
    uint64_t                                    _slideInfoBufferSize;
    uint64_t                                    _fileSize;
    uint64_t                                    _vmSize;
    std::unordered_map<std::string, uint32_t>   _dataDirtySegsOrder;
    std::vector<void*>                          _pointersForASLR;
    std::vector<uint64_t>                       _branchPoolStarts;
    uint64_t                                    _branchPoolsLinkEditStartAddr;
    uint8_t                                     _cdHash[20];
};

std::string normalize_absolute_file_path(const std::string &path);
std::string baspath(const std::string& path);
std::string dirpath(const std::string& path);

std::string toolDir();
bool isProtectedBySIP(const std::string& path, int fd=-1);


template <class Set1, class Set2>
inline bool is_disjoint(const Set1& set1, const Set2& set2)
{
    if (set1.empty() || set2.empty())
        return true;

    typename Set1::const_iterator it1 = set1.begin(), it1End = set1.end();
    typename Set2::const_iterator it2 = set2.begin(), it2End = set2.end();

    if (*it1 > *set2.rbegin() || *it2 > *set1.rbegin())
        return true;

    while (it1 != it1End && it2 != it2End) {
        if (*it1 == *it2)
            return false;
        if (*it1 < *it2) {
            it1++;
        } else {
            it2++;
        }
    }

    return true;
}

inline bool has_prefix(const std::string& str, const std::string& prefix)
{
    return std::mismatch(prefix.begin(), prefix.end(), str.begin()).first == prefix.end();
}


#define NEW_CACHE_FILE_FORMAT 0

#endif // __MEGA_DYLIB_UTILS_H__






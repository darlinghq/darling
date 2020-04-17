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


#ifndef Manifest_h
#define Manifest_h

#include <map>
#include <set>
#include <memory>
#include <string>
#include <vector>

#include <unordered_map>
#include <unordered_set>

#include <assert.h>
#include <uuid/uuid.h>

#import <Foundation/Foundation.h>

#include "DyldSharedCache.h"
#include "Diagnostics.h"
#include "MachOAnalyzer.h"
#include "ClosureFileSystemPhysical.h"

extern std::string toolDir();

namespace dyld3 {

struct VIS_HIDDEN UUID {
    UUID() {}
    UUID(const UUID& other) { uuid_copy(_bytes, other._bytes); }
    UUID(const uuid_t other) { uuid_copy(&_bytes[0], other); }
    UUID(const dyld3::MachOAnalyzer* ml) { ml->getUuid(_bytes); }
    bool operator<(const UUID& other) const { return uuid_compare(_bytes, other._bytes) < 0; }
    bool operator==(const UUID& other) const { return uuid_compare(_bytes, other._bytes) == 0; }
    bool operator!=(const UUID& other) const { return !(*this == other); }

    size_t hash() const
    {
        size_t retval = 0;
        for (size_t i = 0; i < (16 / sizeof(size_t)); ++i) {
            retval ^= ((size_t*)(&_bytes[0]))[i];
        }
        return retval;
    }
    const unsigned char* get() const { return _bytes; };

private:
    uuid_t _bytes;
};

struct BuildQueueEntry {
    DyldSharedCache::CreateOptions            options;
    dyld3::closure::FileSystemPhysical        fileSystem;
    std::vector<DyldSharedCache::MappedMachO> dylibsForCache;
    std::vector<DyldSharedCache::MappedMachO> otherDylibsAndBundles;
    std::vector<DyldSharedCache::MappedMachO> mainExecutables;
    std::string                               outputPath;
    std::set<std::string>                     configNames;
};

struct Manifest {
    struct UUIDInfo {
        const MachOAnalyzer* mh;
        uint64_t sliceFileOffset;
        std::size_t size;
        std::string runtimePath;
        std::string buildPath;
        std::string installName;
        std::string arch;
        UUID uuid;
        UUIDInfo(const MachOAnalyzer* M, std::size_t S, uint64_t SO, UUID U, std::string A, std::string RP, std::string BP, std::string IN)
            : mh(M), size(S), arch(A), uuid(U), runtimePath(RP), buildPath(BP), installName(IN), sliceFileOffset(SO) {}
        UUIDInfo() : UUIDInfo(nullptr, 0, 0, UUID(), "", "", "", "") {}
    };

    struct Project {
        std::vector<std::string> sources;
    };


    struct SegmentInfo {
        std::string name;
        uint64_t    startAddr;
        uint64_t    endAddr;
    };

    struct CacheInfo {
        std::vector<SegmentInfo> regions;
        std::string              cdHash;
    };

    struct CacheImageInfo {
        bool                     included;
        std::string              exclusionInfo;
        UUID                     uuid;
        std::string              installname;
        std::vector<SegmentInfo> segments;
        CacheImageInfo(void)
            : included(true)
        {
        }
    };

    struct Results {
        std::string failure;
        std::map<UUID, CacheImageInfo> dylibs;
        std::map<UUID, CacheImageInfo> bundles;
        std::map<UUID, CacheImageInfo> executables;

        std::set<std::string> warnings;
        CacheInfo             developmentCache;
        CacheInfo             productionCache;
        CacheImageInfo& dylibForInstallname(const std::string& installname);
        void exclude(const dyld3::MachOAnalyzer* ml, const std::string& reason);
        void exclude(Manifest& manifest, const UUID& uuid, const std::string& reason);
    };

    struct Architecture {
        mutable Results     results;

        bool operator==(const Architecture& O) const;
        bool operator!=(const Architecture& other) const;
    };

    struct Configuration {
        std::string                         platformName;
        std::string                         device;
        std::string                         disposition;
        std::string                         metabomTag;
        std::set<std::string>               metabomTags;
        std::set<std::string>               metabomExcludeTags;
        std::set<std::string>               metabomRestrictTags;
        std::set<std::string>               restrictedInstallnames;
        std::map<std::string, Architecture> architectures;

        bool operator==(const Configuration& O) const;
        bool operator!=(const Configuration& other) const;
        const Architecture& architecture(const std::string& architecture) const;
        void forEachArchitecture(std::function<void(const std::string& archName)> lambda) const;
    };

    const std::map<std::string, Project>& projects();
    const Configuration& configuration(const std::string& configuration) const;
    void forEachConfiguration(std::function<void(const std::string& configName)> lambda) const;

    void addProjectSource(const std::string& project, const std::string& source, bool first = false);

    const std::string projectPath(const std::string& projectName);
    const bool        empty(void);
    const std::string dylibOrderFile() const;
    void setDylibOrderFile(const std::string& dylibOrderFile);

    const std::string dirtyDataOrderFile() const;
    void setDirtyDataOrderFile(const std::string& dirtyDataOrderFile);

    const std::string metabomFile() const;
    void setMetabomFile(const std::string& metabomFile);

    const Platform platform() const;
    void setPlatform(const Platform platform);

    const std::string& build() const;
    void setBuild(const std::string& build);
    const uint32_t version() const;
    void setVersion(const uint32_t manifestVersion);
    bool normalized;

    Manifest(Diagnostics& D, const std::string& path, bool populateIt = true);
    void populate(const std::set<std::string>& overlays);

    BuildQueueEntry makeQueueEntry(const std::string& outputPath, const std::set<std::string>& configs, const std::string& arch, bool optimizeStubs, const std::string& prefix,
                                   bool isLocallyBuiltCache, bool skipWrites, bool verbose);

    void write(const std::string& path);
    void writeJSON(const std::string& path);
    void        canonicalize(void);
    void        calculateClosure();
    const MachOAnalyzer* machOForUUID(const UUID& uuid) const;
    const std::string buildPathForUUID(const UUID& uuid);
    const std::string runtimePathForUUID(const UUID& uuid);
    const std::string& installNameForUUID(const UUID& uuid);
    DyldSharedCache::MappedMachO machoForPathAndArch(const std::string& path, const std::string& arch) const;
    void remove(const std::string& config, const std::string& arch);
    void runConcurrently(dispatch_queue_t queue, dispatch_semaphore_t concurrencyLimitingSemaphore, std::function<void(const std::string configuration, const std::string architecture)> lambda);
    bool filterForConfig(const std::string& configName);
    std::set<std::string> resultsForConfiguration(const std::string& configName);

    // These are used by MRM to support having the Manifest give us a list of files/symlinks from the BOM but we use MRM for the actual cache generation
    void forEachMachO(std::string configuration, std::function<void(const std::string &buildPath, const std::string &runtimePath, const std::string &arch, bool shouldBeExcludedIfLeaf)> lambda);

    void forEachSymlink(std::string configuration, std::function<void(const std::string &fromPath, const std::string &toPath)> lambda);

private:
    NSDictionary*    _manifestDict;
    Diagnostics&      _diags;
    std::map<UUID, UUIDInfo> _uuidMap;
    std::map<std::pair<std::string, std::string>, UUID> _installNameMap;
    std::vector<std::pair<std::string, std::string>>    _symlinks;
    static dispatch_queue_t _identifierQueue;
    uint32_t                _manifestVersion;
    std::string             _build;
    std::string             _dylibOrderFile;
    std::string             _dirtyDataOrderFile;
    std::string             _metabomFile;
    Platform                _platform;
    std::map<std::string, Project>               _projects;
    std::map<std::string, Configuration>         _configurations;
    std::map<std::string, std::set<std::string>> _metabomTagMap;
    std::map<std::string, std::set<std::string>> _metabomSymlinkTagMap;
    std::map<std::string, std::set<std::string>> _metabomExcludeTagMap;
    std::map<std::string, std::set<std::string>> _metabomRestrictedTagMap;

    std::vector<DyldSharedCache::MappedMachO> dylibsForCache(const std::string& configuration, const std::string& architecture);
    std::vector<DyldSharedCache::MappedMachO> otherDylibsAndBundles(const std::string& configuration, const std::string& architecture);
    std::vector<DyldSharedCache::MappedMachO> mainExecutables(const std::string& configuration, const std::string& architecture);

    const UUIDInfo& infoForUUID(const UUID& uuid) const;
    const UUIDInfo infoForInstallNameAndarch(const std::string& installName, const std::string arch) const;
    void insert(std::vector<DyldSharedCache::MappedMachO>& mappedMachOs, const CacheImageInfo& imageInfo);
    bool loadParser(const void* p, size_t sliceLength, uint64_t sliceOffset, const std::string& runtimePath, const std::string& buildPath, const std::set<std::string>& architectures);
    bool loadParsers(const std::string& pathToMachO, const std::string& runtimePath, const std::set<std::string>& architectures);
    void dedupeDispositions();
    void calculateClosure(const std::string& configuration, const std::string& architecture);
    void canonicalizeDylib(const std::string& installname);
    template <typename P>
    void canonicalizeDylib(const std::string& installname, const uint8_t* p);
    void addImplicitAliases(void);
};
}

namespace std {
template <>
struct hash<dyld3::UUID> {
    size_t operator()(const dyld3::UUID& x) const
    {
        return x.hash();
    }
};
}

#endif /* Manifest_h */

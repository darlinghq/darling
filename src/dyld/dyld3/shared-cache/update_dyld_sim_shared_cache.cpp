/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
 *
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach-o/dyld.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <dirent.h>
#include <rootless.h>
#include <dscsym.h>
#include <dispatch/dispatch.h>
#include <pthread/pthread.h>
#include <CoreFoundation/CoreFoundation.h>

#include <algorithm>
#include <vector>
#include <unordered_set>
#include <unordered_set>
#include <iostream>
#include <fstream>

#include "FileUtils.h"
#include "StringUtils.h"
#include "DyldSharedCache.h"
#include "MachOFile.h"
#include "MachOAnalyzer.h"
#include "ClosureFileSystemPhysical.h"

struct MappedMachOsByCategory
{
    const dyld3::GradedArchs&                   archs;
    std::vector<DyldSharedCache::MappedMachO>   dylibsForCache;
    std::vector<DyldSharedCache::MappedMachO>   otherDylibsAndBundles;
    std::vector<DyldSharedCache::MappedMachO>   mainExecutables;
};

static const char* sAllowedPrefixes[] = {
    "/usr",
    "/System",
// don't look at main executables until simulator supports dyld3
//    "/bin",
//    "/sbin",
};

static const char* sDontUsePrefixes[] = {
    "/usr/share",
    "/usr/local",
};

static const char* sMacOSHostLibs[] = {
    "/usr/lib/system/libsystem_kernel.dylib",
    "/usr/lib/system/libsystem_platform.dylib",
    "/usr/lib/system/libsystem_pthread.dylib",
};

static const char* sMacOSBinaries[] = {
    "/sbin/launchd_sim_trampoline",
    "/usr/sbin/iokitsimd",
    "/usr/lib/system/host/liblaunch_sim.dylib",
};

static bool verbose = false;


static bool addIfMachO(const dyld3::closure::FileSystem& fileSystem, const std::string& runtimePath, const struct stat& statBuf, std::vector<MappedMachOsByCategory>& files, dyld3::Platform platform)
{
    // don't precompute closure info for any debug or profile dylibs
    if ( endsWith(runtimePath, "_profile.dylib") || endsWith(runtimePath, "_debug.dylib") || endsWith(runtimePath, "_asan.dylib") || endsWith(runtimePath, "_profile") || endsWith(runtimePath, "_debug") )
        return false;
    if ( startsWith(runtimePath, "/usr/lib/system/introspection/") )
        return false;

    bool result = false;
    for (MappedMachOsByCategory& file : files) {
        Diagnostics diag;
        char realerPath[MAXPATHLEN];
        dyld3::closure::LoadedFileInfo loadedFileInfo = dyld3::MachOAnalyzer::load(diag, fileSystem, runtimePath.c_str(), file.archs, platform, realerPath);
        const dyld3::MachOAnalyzer* ma = (const dyld3::MachOAnalyzer*)loadedFileInfo.fileContent;

        if ( ma != nullptr ) {
            bool            sipProtected    = false; // isProtectedBySIP(fd);
            bool            issetuid        = false;
            const uint64_t  sliceLen        = loadedFileInfo.sliceLen;
            if ( ma->isDynamicExecutable() ) {
        #if 0 // dyld3 not enabled for simulator yet, so don't collect main executables
                issetuid = (statBuf.st_mode & (S_ISUID|S_ISGID));
                file.mainExecutables.emplace_back(runtimePath, ma, sliceLen, issetuid, sipProtected, loadedFileInfo.sliceOffset, statBuf.st_mtime, statBuf.st_ino);
                result = true;
        #endif
            }
            else if ( ma->canBePlacedInDyldCache(runtimePath.c_str(), ^(const char* msg) {}) ) {
                file.dylibsForCache.emplace_back(runtimePath, ma, sliceLen, issetuid, sipProtected, loadedFileInfo.sliceOffset, statBuf.st_mtime, statBuf.st_ino);
                result = true;
           }
        }
    }
    
    return result;
}

static void findAllFiles(const dyld3::closure::FileSystem& fileSystem, const std::vector<std::string>& pathPrefixes, std::vector<MappedMachOsByCategory>& files, dyld3::Platform platform)
{
    std::unordered_set<std::string> skipDirs;
    for (const char* s : sDontUsePrefixes)
        skipDirs.insert(s);
    
    __block std::unordered_set<std::string> alreadyUsed;
    bool multiplePrefixes = (pathPrefixes.size() > 1);
    for (const std::string& prefix : pathPrefixes) {
        // get all files from overlay for this search dir
        for (const char* searchDir : sAllowedPrefixes ) {
            iterateDirectoryTree(prefix, searchDir, ^(const std::string& dirPath) { return (skipDirs.count(dirPath) != 0); }, ^(const std::string& path, const struct stat& statBuf) {
                // ignore files that don't have 'x' bit set (all runnable mach-o files do)
                const bool hasXBit = ((statBuf.st_mode & S_IXOTH) == S_IXOTH);
                if ( !hasXBit && !endsWith(path, ".dylib") )
                    return;

                // ignore files too small (must have at least a page of TEXT and LINKEDIT)
                if ( statBuf.st_size < 0x2000 )
                    return;

                // don't add paths already found using previous prefix
                if ( multiplePrefixes && (alreadyUsed.count(path) != 0) )
                    return;
                
                // don't add binaries built for the host Mac OS platform
                for (std::string s : sMacOSBinaries) {
                    if (s.compare(path) == 0)
                        return;
                }
                // if the file is mach-o, add to list
                if ( addIfMachO(fileSystem, path, statBuf, files, platform) ) {
                    if ( multiplePrefixes )
                        alreadyUsed.insert(path);
                }
            });
        }
    }
}

static void addMacOSHostLibs(std::vector<MappedMachOsByCategory>& allFileSets, dyld3::Platform platform)
{
    dyld3::closure::FileSystemPhysical fileSystem;
    for (const char* path : sMacOSHostLibs) {
        struct stat statBuf;
        if ( stat(path, &statBuf) == 0 ) {
            addIfMachO(fileSystem, path, statBuf, allFileSets, dyld3::Platform::macOS);
        }
    }
}

static void addMacOSBinaries(const dyld3::closure::FileSystem& fileSystem, const std::vector<std::string>& pathPrefixes, std::vector<MappedMachOsByCategory>& files)
{
    for (const std::string& prefix : pathPrefixes) {
        for (std::string path : sMacOSBinaries) {
            std::string fullPath = prefix + path;
            struct stat statBuf;
            if ( stat(fullPath.c_str(), &statBuf) == 0 ) {
                addIfMachO(fileSystem, path, statBuf, files, dyld3::Platform::macOS);
            }
        }
    }
}

static bool dontCache(const std::string& simRuntimeRootPath, const std::string& archName,
                      const std::unordered_set<std::string>& pathsWithDuplicateInstallName,
                      const DyldSharedCache::MappedMachO& aFile, bool warn,
                      const std::unordered_set<std::string>& skipDylibs)
{
    if ( skipDylibs.count(aFile.runtimePath) )
        return true;
    if ( startsWith(aFile.runtimePath, "/usr/lib/system/introspection/") )
        return true;
    if ( startsWith(aFile.runtimePath, "/usr/local/") )
        return true;
    
    // anything inside a .app bundle is specific to app, so should not be in shared cache
    if ( aFile.runtimePath.find(".app/") != std::string::npos )
        return true;
    
    if ( archName == "i386" ) {
        if ( startsWith(aFile.runtimePath, "/System/Library/CoreServices/") )
            return true;
        if ( startsWith(aFile.runtimePath, "/System/Library/Extensions/") )
            return true;
    }
    
    if ( aFile.runtimePath.find("//") != std::string::npos ) {
        if (warn) fprintf(stderr, "update_dyld_sim_shared_cache: warning: %s use of bad install name %s\n", archName.c_str(), aFile.runtimePath.c_str());
        return true;
    }
    
    const char* installName = aFile.mh->installName();
    if ( (pathsWithDuplicateInstallName.count(aFile.runtimePath) != 0) && (aFile.runtimePath != installName) ) {
        // <rdar://problem/46431467> if a dylib moves and a symlink is installed into its place, bom iterator will see both and issue a warning
        struct stat statBuf;
        bool isSymLink = ( (lstat(aFile.runtimePath.c_str(), &statBuf) == 0) && S_ISLNK(statBuf.st_mode) );
        if (!isSymLink && warn) fprintf(stderr, "update_dyld_sim_shared_cache: warning: %s skipping because of duplicate install name %s\n", archName.c_str(), aFile.runtimePath.c_str());
        return true;
    }
    
    if ( aFile.runtimePath != installName ) {
        // see if install name is a symlink to actual path
        std::string fullInstall = simRuntimeRootPath + installName;
        char resolvedPath[PATH_MAX];
        if ( realpath(fullInstall.c_str(), resolvedPath) != NULL ) {
            std::string resolvedSymlink = resolvedPath;
            if ( !simRuntimeRootPath.empty() ) {
                resolvedSymlink = resolvedSymlink.substr(simRuntimeRootPath.size());
            }
            if ( aFile.runtimePath == resolvedSymlink ) {
                return false;
            }
        }
        // <rdar://problem/38000411> also if runtime path is a symlink to install name
        std::string fullRuntime = simRuntimeRootPath + aFile.runtimePath;
        if ( realpath(fullRuntime.c_str(), resolvedPath) != NULL ) {
            std::string resolvedSymlink = resolvedPath;
            if ( !simRuntimeRootPath.empty() ) {
                resolvedSymlink = resolvedSymlink.substr(simRuntimeRootPath.size());
            }
            if ( resolvedSymlink == installName ) {
                return false;
            }
        }
        if (warn) fprintf(stderr, "update_dyld_sim_shared_cache: warning: %s skipping because of bad install name %s\n", archName.c_str(), aFile.runtimePath.c_str());
        return true;
    }
    return false;
}

static void pruneCachedDylibs(const std::string& volumePrefix, const std::unordered_set<std::string>& skipDylibs, MappedMachOsByCategory& fileSet)
{
    std::unordered_set<std::string> pathsWithDuplicateInstallName;
    
    std::unordered_map<std::string, std::string> installNameToFirstPath;
    for (DyldSharedCache::MappedMachO& aFile : fileSet.dylibsForCache) {
        const char* installName = aFile.mh->installName();
        auto pos = installNameToFirstPath.find(installName);
        if ( pos == installNameToFirstPath.end() ) {
            installNameToFirstPath[installName] = aFile.runtimePath;
        }
        else {
            pathsWithDuplicateInstallName.insert(aFile.runtimePath);
            pathsWithDuplicateInstallName.insert(installNameToFirstPath[installName]);
        }
    }
    
    for (DyldSharedCache::MappedMachO& aFile : fileSet.dylibsForCache) {
        if ( dontCache(volumePrefix, fileSet.archs.name(), pathsWithDuplicateInstallName, aFile, true, skipDylibs) ){
            // <rdar://problem/46423929> don't build dlopen closures for symlinks to something in the dyld cache
            if ( pathsWithDuplicateInstallName.count(aFile.runtimePath) == 0 )
                fileSet.otherDylibsAndBundles.push_back(aFile);
        }
    }
    fileSet.dylibsForCache.erase(std::remove_if(fileSet.dylibsForCache.begin(), fileSet.dylibsForCache.end(),
                                                [&](const DyldSharedCache::MappedMachO& aFile) { return dontCache(volumePrefix, fileSet.archs.name(), pathsWithDuplicateInstallName, aFile, false, skipDylibs); }),
                                 fileSet.dylibsForCache.end());
}

static void pruneOtherDylibs(const std::string& volumePrefix, MappedMachOsByCategory& fileSet)
{
    // other OS dylibs should not contain dylibs that are embedded in some .app bundle
    fileSet.otherDylibsAndBundles.erase(std::remove_if(fileSet.otherDylibsAndBundles.begin(), fileSet.otherDylibsAndBundles.end(),
                                                       [&](const DyldSharedCache::MappedMachO& aFile) { return (aFile.runtimePath.find(".app/") != std::string::npos); }),
                                        fileSet.otherDylibsAndBundles.end());
}


static bool existingCacheUpToDate(const std::string& existingCache, const std::vector<DyldSharedCache::MappedMachO>& currentDylibs)
{
    // if no existing cache, it is not up-to-date
    int fd = ::open(existingCache.c_str(), O_RDONLY);
    if ( fd < 0 )
        return false;
    struct stat statbuf;
    if ( ::fstat(fd, &statbuf) == -1 ) {
        ::close(fd);
        return false;
    }
    
    // build map of found dylibs
    std::unordered_map<std::string, const DyldSharedCache::MappedMachO*> currentDylibMap;
    for (const DyldSharedCache::MappedMachO& aFile : currentDylibs) {
        //fprintf(stderr, "0x%0llX 0x%0llX  %s\n", aFile.inode, aFile.modTime, aFile.runtimePath.c_str());
        currentDylibMap[aFile.runtimePath] = &aFile;
    }
    
    // make sure all dylibs in existing cache have same mtime and inode as found dylib
    __block bool foundMismatch = false;
    const uint64_t cacheMapLen = statbuf.st_size;
    void *p = ::mmap(NULL, cacheMapLen, PROT_READ, MAP_PRIVATE, fd, 0);
    if ( p != MAP_FAILED ) {
        const DyldSharedCache* cache = (DyldSharedCache*)p;
        cache->forEachImageEntry(^(const char* installName, uint64_t mTime, uint64_t inode) {
            bool foundMatch = false;
            auto pos = currentDylibMap.find(installName);
            if ( pos != currentDylibMap.end() ) {
                const DyldSharedCache::MappedMachO* foundDylib = pos->second;
                if ( (foundDylib->inode == inode) && (foundDylib->modTime == mTime) ) {
                    foundMatch = true;
                }
            }
            if ( !foundMatch ) {
                // use slow path and look for any dylib with a matching inode and mtime
                bool foundSlow = false;
                for (const DyldSharedCache::MappedMachO& aFile : currentDylibs) {
                    if ( (aFile.inode == inode) && (aFile.modTime == mTime) ) {
                        foundSlow = true;
                        break;
                    }
                }
                if ( !foundSlow ) {
                    foundMismatch = true;
                    if ( verbose )
                        fprintf(stderr, "rebuilding dyld cache because dylib changed: %s\n", installName);
                }
            }
         });
        ::munmap(p, cacheMapLen);
    }

    ::close(fd);

    return !foundMismatch;
}


inline uint32_t absolutetime_to_milliseconds(uint64_t abstime)
{
    return (uint32_t)(abstime/1000/1000);
}


#define TERMINATE_IF_LAST_ARG( s )      \
    do {                                \
        if ( i == argc - 1 ) {          \
            fprintf(stderr, s );        \
            return 1;                   \
        }                               \
    } while ( 0 )

int main(int argc, const char* argv[], const char* envp[])
{
    std::string                     rootPath;
    bool                            force = false;
    bool                            dylibsRemoved = false;
    std::string                     cacheDir;
    std::string                     dylibOrderFile;
    std::string                     dirtyDataOrderFile;
    dyld3::Platform                 platform = dyld3::Platform::iOS_simulator;
    std::unordered_set<std::string> skipDylibs;
    
    // parse command line options
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (strcmp(arg, "-debug") == 0) {
            verbose = true;
        }
        else if (strcmp(arg, "-verbose") == 0) {
            verbose = true;
        }
        else if ((strcmp(arg, "-root") == 0) || (strcmp(arg, "--root") == 0)) {
            TERMINATE_IF_LAST_ARG("-root missing path argument\n");
            rootPath = argv[++i];
        }
        else if (strcmp(arg, "-cache_dir") == 0) {
            TERMINATE_IF_LAST_ARG("-cache_dir missing path argument\n");
            cacheDir = argv[++i];
        }
        else if (strcmp(arg, "-iOS") == 0) {
            platform = dyld3::Platform::iOS_simulator;
        }
        else if (strcmp(arg, "-watchOS") == 0) {
            platform = dyld3::Platform::watchOS_simulator;
        }
        else if (strcmp(arg, "-tvOS") == 0) {
            platform = dyld3::Platform::tvOS_simulator;
        }
        else if (strcmp(arg, "-dylibs_removed_in_mastering") == 0) {
            dylibsRemoved = true;
        }
        else if (strcmp(arg, "-dylib_order_file") == 0) {
            TERMINATE_IF_LAST_ARG("-dylib_order_file missing path argument\n");
            dylibOrderFile = argv[++i];
        }
        else if (strcmp(arg, "-dirty_data_order_file") == 0) {
            TERMINATE_IF_LAST_ARG("-dirty_data_order_file missing path argument\n");
            dirtyDataOrderFile = argv[++i];
        }
        else if (strcmp(arg, "-force") == 0) {
            force = true;
        }
        else if (strcmp(arg, "-skip") == 0) {
            TERMINATE_IF_LAST_ARG("-skip missing argument\n");
            skipDylibs.insert(argv[++i]);
        }
        else {
            //usage();
            fprintf(stderr, "update_dyld_sim_shared_cache: unknown option: %s\n", arg);
            return 1;
        }
    }
    
    if ( rootPath.empty()) {
        fprintf(stderr, "-root should be specified\n");
        return 1;
    }
    if (cacheDir.empty()) {
        fprintf(stderr, "-cache_dir should be specified\n");
        return 1;
    }
    // canonicalize rootPath
    char resolvedPath[PATH_MAX];
    if ( realpath(rootPath.c_str(), resolvedPath) != NULL ) {
        rootPath = resolvedPath;
    }

    // Find the boot volume so that we can ensure all overlays are on the same volume
    struct stat rootStatBuf;
    if ( stat(rootPath.c_str(), &rootStatBuf) != 0 ) {
        fprintf(stderr, "update_dyld_sim_shared_cache: error: could not stat root file system because '%s'\n", strerror(errno));
        return 1;
    }

    std::vector<std::string> pathPrefixes;

    pathPrefixes.push_back(rootPath);
    
    // build FileSystem object
    const char* fsRoot = rootPath.empty() ? nullptr : rootPath.c_str();
    dyld3::closure::FileSystemPhysical fileSystem(fsRoot, nullptr);

    
    int err = mkpath_np(cacheDir.c_str(), S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH);
    if ( (err != 0) && (err != EEXIST) ) {
        fprintf(stderr, "update_dyld_sim_shared_cache: could not access cache dir: mkpath_np(%s) failed errno=%d\n", cacheDir.c_str(), err);
        return 1;
    }

    uint64_t t1 = mach_absolute_time();

    __block std::vector<MappedMachOsByCategory> allFileSets;
    switch ( platform ) {
        case dyld3::Platform::iOS_simulator:
            allFileSets.push_back({dyld3::GradedArchs::x86_64});
            break;
        case dyld3::Platform::watchOS_simulator:
            allFileSets.push_back({dyld3::GradedArchs::i386});
            break;
        case dyld3::Platform::tvOS_simulator:
            allFileSets.push_back({dyld3::GradedArchs::x86_64});
            break;
        default:
            assert(0 && "invalid platform");
            break;
    }
    findAllFiles(fileSystem, pathPrefixes, allFileSets, platform);
    addMacOSHostLibs(allFileSets, platform);
    addMacOSBinaries(fileSystem, pathPrefixes, allFileSets);

    // nothing in OS uses i386 dylibs, so only dylibs used by third party apps need to be in cache
    for (MappedMachOsByCategory& fileSet : allFileSets) {
        pruneCachedDylibs(rootPath, skipDylibs, fileSet);
        pruneOtherDylibs(rootPath, fileSet);
    }

    uint64_t t2 = mach_absolute_time();
    if ( verbose ) {
        fprintf(stderr, "time to scan file system and construct lists of mach-o files: %ums\n", absolutetime_to_milliseconds(t2-t1));
    }
    
    // build caches in parallel on machines with at leat 4GB of RAM
    uint64_t memSize = 0;
    size_t sz = sizeof(memSize);;
    bool buildInParallel = false;
    if ( sysctlbyname("hw.memsize", &memSize, &sz, NULL, 0) == 0 ) {
        if ( memSize >= 0x100000000ULL )
            buildInParallel = true;
    }
    dispatch_queue_t dqueue = buildInParallel ? dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0)
    : dispatch_queue_create("serial-queue", DISPATCH_QUEUE_SERIAL);
    
    // build all caches
    __block bool cacheBuildFailure = false;
    __block bool wroteSomeCacheFile = false;
    dispatch_apply(allFileSets.size(), dqueue, ^(size_t index) {
        MappedMachOsByCategory& fileSet = allFileSets[index];
        const std::string outFile = cacheDir + "/dyld_sim_shared_cache_" + fileSet.archs.name();
        
        DyldSharedCache::MappedMachO (^loader)(const std::string&) = ^DyldSharedCache::MappedMachO(const std::string& runtimePath) {
            if ( skipDylibs.count(runtimePath) )
                return DyldSharedCache::MappedMachO();

            for (const std::string& prefix : pathPrefixes) {
                std::string fullPath = prefix + runtimePath;
                struct stat statBuf;
                if ( stat(fullPath.c_str(), &statBuf) == 0 ) {
                    char truePath[PATH_MAX];
                    if ( realpath(fullPath.c_str(), truePath) != NULL ) {
                        std::string resolvedSymlink = truePath;
                        if ( !rootPath.empty() ) {
                            resolvedSymlink = resolvedSymlink.substr(rootPath.size());
                        }
                        if ( (runtimePath != resolvedSymlink) && !contains(runtimePath, "InputContext") ) {  //HACK remove InputContext when fixed
                            // path requested is a symlink path, check if real path already loaded
                            for (const DyldSharedCache::MappedMachO& aDylibMapping : fileSet.dylibsForCache) {
                                if ( aDylibMapping.runtimePath == resolvedSymlink ) {
                                    if ( verbose )
                                        fprintf(stderr, "verifySelfContained, redirect %s to %s\n", runtimePath.c_str(), aDylibMapping.runtimePath.c_str());
                                    return aDylibMapping;
                                }
                            }
                        }
                    }
                    
                    std::vector<MappedMachOsByCategory> mappedFiles;
                    mappedFiles.push_back({fileSet.archs});
                    if ( addIfMachO(fileSystem, runtimePath, statBuf, mappedFiles, platform) ) {
                        if ( !mappedFiles.back().dylibsForCache.empty() ) {
                            if ( verbose )
                                fprintf(stderr, "verifySelfContained, add %s\n", mappedFiles.back().dylibsForCache.back().runtimePath.c_str());
                            return mappedFiles.back().dylibsForCache.back();
                        }
                    }
                }
            }
            return DyldSharedCache::MappedMachO();
        };
        size_t startCount = fileSet.dylibsForCache.size();
        std::vector<std::pair<DyldSharedCache::MappedMachO, std::set<std::string>>> excludes;
        std::unordered_set<std::string> badZippered;
        DyldSharedCache::verifySelfContained(fileSet.dylibsForCache, badZippered, loader, excludes);
        for (size_t i=startCount; i < fileSet.dylibsForCache.size(); ++i) {
            fprintf(stderr, "update_dyld_sim_shared_cache: warning: %s not in initial scan, but adding required dylib %s\n", fileSet.archs.name(), fileSet.dylibsForCache[i].runtimePath.c_str());
        }
        for (auto& exclude : excludes) {
            std::string reasons = "(\"";
            for (auto i = exclude.second.begin(); i != exclude.second.end(); ++i) {
                reasons += *i;
                if (i != --exclude.second.end()) {
                    reasons += "\", \"";
                }
            }
            reasons += "\")";
            fprintf(stderr, "update_dyld_shared_cache: warning: %s rejected from cached dylibs: %s (%s)\n", fileSet.archs.name(), exclude.first.runtimePath.c_str(), reasons.c_str());
            fileSet.otherDylibsAndBundles.push_back(exclude.first);
        }
        
        // check if cache is already up to date
        if ( !force ) {
            if ( existingCacheUpToDate(outFile, fileSet.dylibsForCache) )
                return;
        }
        
        
        // build cache new cache file
        DyldSharedCache::CreateOptions options;
        options.outputFilePath               = outFile;
        options.outputMapFilePath            = cacheDir + "/dyld_sim_shared_cache_" + fileSet.archs.name() + ".map";
        options.archs                        = &fileSet.archs;
        options.platform                     = platform;
        options.excludeLocalSymbols          = false;
        options.optimizeStubs                = false;
        options.optimizeObjC                 = true;
        options.codeSigningDigestMode        = DyldSharedCache::SHA256only;
        options.dylibsRemovedDuringMastering = dylibsRemoved;
        options.inodesAreSameAsRuntime       = true;
        options.cacheSupportsASLR            = false;
        options.forSimulator                 = true;
        options.isLocallyBuiltCache          = true;
        options.verbose                      = verbose;
        options.evictLeafDylibsOnOverflow    = true;
        options.dylibOrdering                = parseOrderFile(dylibOrderFile);
        options.dirtyDataSegmentOrdering     = parseOrderFile(dirtyDataOrderFile);
        DyldSharedCache::CreateResults results = DyldSharedCache::create(options, fileSystem, fileSet.dylibsForCache, fileSet.otherDylibsAndBundles, fileSet.mainExecutables);
        
        // print any warnings
        for (const std::string& warn : results.warnings) {
            fprintf(stderr, "update_dyld_sim_shared_cache: warning: %s %s\n", fileSet.archs.name(), warn.c_str());
        }
        if ( results.errorMessage.empty() ) {
            wroteSomeCacheFile = true;
        }
        else {
            fprintf(stderr, "update_dyld_sim_shared_cache: %s\n", results.errorMessage.c_str());
            cacheBuildFailure = true;
        }
    });
    
    // we could unmap all input files, but tool is about to quit

    return (cacheBuildFailure ? 1 : 0);
}


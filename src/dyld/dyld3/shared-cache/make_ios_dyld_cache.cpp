/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
 *
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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

#include <algorithm>
#include <vector>
#include <unordered_set>
#include <unordered_set>
#include <iostream>
#include <fstream>

#include "MachOFile.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "DyldSharedCache.h"



struct MappedMachOsByCategory
{
    std::string                                 archName;
    std::vector<DyldSharedCache::MappedMachO>   dylibsForCache;
    std::vector<DyldSharedCache::MappedMachO>   otherDylibsAndBundles;
    std::vector<DyldSharedCache::MappedMachO>   mainExecutables;
};

static bool verbose = false;


static bool addIfMachO(const std::string& buildRootPath, const std::string& runtimePath, const struct stat& statBuf, dyld3::Platform platform, std::vector<MappedMachOsByCategory>& files)
{
    // read start of file to determine if it is mach-o or a fat file
    std::string fullPath = buildRootPath + runtimePath;
    int fd = ::open(fullPath.c_str(), O_RDONLY);
    if ( fd < 0 )
        return false;
    bool result = false;
    const void* wholeFile = ::mmap(NULL, (size_t)statBuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if ( wholeFile != MAP_FAILED ) {
        Diagnostics diag;
        bool usedWholeFile = false;
        for (MappedMachOsByCategory& file : files) {
            uint64_t sliceOffset = 0;
            uint64_t sliceLength = statBuf.st_size;
            bool fatButMissingSlice;
            const void* slice = MAP_FAILED;
            const dyld3::FatFile* fh = (dyld3::FatFile*)wholeFile;
            const dyld3::MachOFile* mh = (dyld3::MachOFile*)wholeFile;
            if ( fh->isFatFileWithSlice(diag, statBuf.st_size, file.archName.c_str(), sliceOffset, sliceLength, fatButMissingSlice) ) {
                slice = ::mmap(NULL, sliceLength, PROT_READ, MAP_PRIVATE, fd, sliceOffset);
                if ( slice != MAP_FAILED ) {
                    //fprintf(stderr, "mapped slice at %p size=0x%0lX, offset=0x%0lX for %s\n", p, len, offset, fullPath.c_str());
                    mh = (dyld3::MachOFile*)slice;
                    if ( !mh->isMachO(diag, sliceLength) ) {
                        ::munmap((void*)slice, sliceLength);
                        slice = MAP_FAILED;
                    }
                }
            }
            else if ( !fatButMissingSlice && mh->isMachO(diag, sliceLength) ) {
                slice           = wholeFile;
                sliceLength     = statBuf.st_size;
                sliceOffset     = 0;
                usedWholeFile   = true;
                //fprintf(stderr, "mapped whole file at %p size=0x%0lX for %s\n", p, len, inputPath.c_str());
            }
            if ( slice != MAP_FAILED ) {
                mh = (dyld3::MachOFile*)slice;
                if ( mh->platform() != platform ) {
                    fprintf(stderr, "skipped wrong platform binary: %s\n", fullPath.c_str());
                    result = false;
                }
                else {
                    bool sip = true; // assume anything found in the simulator runtime is a platform binary
                    if ( mh->isDynamicExecutable() ) {
                        bool issetuid = (statBuf.st_mode & (S_ISUID|S_ISGID));
                        file.mainExecutables.emplace_back(runtimePath, mh, sliceLength, issetuid, sip, sliceOffset, statBuf.st_mtime, statBuf.st_ino);
                    }
                    else {
                        if ( parser.canBePlacedInDyldCache(runtimePath) ) {
                            file.dylibsForCache.emplace_back(runtimePath, mh, sliceLength, false, sip, sliceOffset, statBuf.st_mtime, statBuf.st_ino);
                        }
                    }
                    result = true;
                }
            }
        }
        if ( !usedWholeFile )
            ::munmap((void*)wholeFile, statBuf.st_size);
    }
    ::close(fd);
    return result;
}


static bool parsePathsFile(const std::string& filePath, std::vector<std::string>& paths) {
    std::ifstream myfile( filePath );
    if ( myfile.is_open() ) {
    std::string line;
        while ( std::getline(myfile, line) ) {
            size_t pos = line.find('#');
            if ( pos != std::string::npos )
                line.resize(pos);
            while ( line.size() != 0 && isspace(line.back()) ) {
                line.pop_back();
            }
            if ( !line.empty() )
                paths.push_back(line);
        }
        myfile.close();
        return true;
    }
    return false;
}


static void mapAllFiles(const std::string& dylibsRootDir, const std::vector<std::string>& paths, dyld3::Platform platform, std::vector<MappedMachOsByCategory>& files)
{
    for (const std::string& runtimePath : paths) {
        std::string fullPath = dylibsRootDir + runtimePath;
        struct stat statBuf;
        if ( (stat(fullPath.c_str(), &statBuf) != 0) || !addIfMachO(dylibsRootDir, runtimePath, statBuf, platform, files) )
             fprintf(stderr, "could not load: %s\n", fullPath.c_str());
    }
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

int main(int argc, const char* argv[])
{
    std::string                     rootPath;
    std::string                     dylibListFile;
    bool                            force = false;
    std::string                     cacheDir;
    std::string                     dylibsList;
    std::unordered_set<std::string> archStrs;

    dyld3::Platform platform = dyld3::Platform::iOS;

    // parse command line options
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (strcmp(arg, "-debug") == 0) {
            verbose = true;
        }
        else if (strcmp(arg, "-verbose") == 0) {
            verbose = true;
        }
        else if (strcmp(arg, "-tvOS") == 0) {
            platform = dyld3::Platform::tvOS;
        }
        else if (strcmp(arg, "-iOS") == 0) {
            platform = dyld3::Platform::iOS;
        }
        else if (strcmp(arg, "-watchOS") == 0) {
            platform = dyld3::Platform::watchOS;
        }
        else if ( strcmp(arg, "-root") == 0 ) {
            TERMINATE_IF_LAST_ARG("-root missing path argument\n");
            rootPath = argv[++i];
        }
        else if ( strcmp(arg, "-dylibs_list") == 0 ) {
            TERMINATE_IF_LAST_ARG("-dylibs_list missing path argument\n");
            dylibsList = argv[++i];
        }
        else if (strcmp(arg, "-cache_dir") == 0) {
            TERMINATE_IF_LAST_ARG("-cache_dir missing path argument\n");
            cacheDir = argv[++i];
        }
        else if (strcmp(arg, "-arch") == 0) {
            TERMINATE_IF_LAST_ARG("-arch missing argument\n");
            archStrs.insert(argv[++i]);
        }
        else if (strcmp(arg, "-force") == 0) {
            force = true;
        }
        else {
            //usage();
            fprintf(stderr, "update_dyld_sim_shared_cache: unknown option: %s\n", arg);
            return 1;
        }
    }

    if ( cacheDir.empty() ) {
        fprintf(stderr, "missing -cache_dir <path> option to specify directory in which to write cache file(s)\n");
        return 1;
    }

    if ( rootPath.empty() ) {
        fprintf(stderr, "missing -runtime_dir <path> option to specify directory which is root of simulator runtime)\n");
        return 1;
    }
    else {
        // canonicalize rootPath
        char resolvedPath[PATH_MAX];
        if ( realpath(rootPath.c_str(), resolvedPath) != NULL ) {
            rootPath = resolvedPath;
        }
        if ( rootPath.back() != '/' )
            rootPath = rootPath + "/";
    }

    int err = mkpath_np(cacheDir.c_str(), S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH);
    if ( (err != 0) && (err != EEXIST) ) {
        fprintf(stderr, "mkpath_np fail: %d", err);
        return 1;
    }

    if ( archStrs.empty() ) {
        switch ( platform ) {
            case dyld3::Platform::iOS:
            case dyld3::Platform::tvOS:
                archStrs.insert("arm64");
                break;
            case dyld3::Platform::watchOS:
                archStrs.insert("armv7k");
                archStrs.insert("arm64_32");
                break;
             case dyld3::Platform::unknown:
             case dyld3::Platform::macOS:
                assert(0 && "macOS not support with this tool");
                break;
        }
    }

    uint64_t t1 = mach_absolute_time();

    // find all mach-o files for requested architectures
    std::vector<MappedMachOsByCategory> allFileSets;
    if ( archStrs.count("arm64") )
        allFileSets.push_back({"arm64"});
    if ( archStrs.count("arm64_32") )
        allFileSets.push_back({"arm64_32"});
    if ( archStrs.count("armv7k") )
        allFileSets.push_back({"armv7k"});
    std::vector<std::string> paths;
    parsePathsFile(dylibsList, paths);
    mapAllFiles(rootPath, paths, platform, allFileSets);

    uint64_t t2 = mach_absolute_time();

    fprintf(stderr, "time to scan file system and construct lists of mach-o files: %ums\n", absolutetime_to_milliseconds(t2-t1));

    // build all caches in parallel
    __block bool cacheBuildFailure = false;
    dispatch_apply(allFileSets.size(), dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(size_t index) {
        const MappedMachOsByCategory& fileSet = allFileSets[index];
        const std::string outFile = cacheDir + "/dyld_shared_cache_" + fileSet.archName;

        fprintf(stderr, "make %s cache with %lu dylibs, %lu other dylibs, %lu programs\n", fileSet.archName.c_str(), fileSet.dylibsForCache.size(), fileSet.otherDylibsAndBundles.size(), fileSet.mainExecutables.size());

        // build cache new cache file
        DyldSharedCache::CreateOptions options;
        options.archName                     = fileSet.archName;
        options.platform                     = platform;
        options.excludeLocalSymbols          = true;
        options.optimizeStubs                = false;
        options.optimizeObjC                 = true;
        options.codeSigningDigestMode = (platform() == dyld3::Platform::watchOS) ?
                                        DyldSharedCache::Agile : DyldSharedCache::SHA256only;
        options.dylibsRemovedDuringMastering = true;
        options.inodesAreSameAsRuntime       = false;
        options.cacheSupportsASLR            = true;
        options.forSimulator                 = false;
        options.isLocallyBuiltCache          = true;
        options.verbose                      = verbose;
        options.evictLeafDylibsOnOverflow    = false;
        DyldSharedCache::CreateResults results = DyldSharedCache::create(options, fileSet.dylibsForCache, fileSet.otherDylibsAndBundles, fileSet.mainExecutables);

        // print any warnings
        for (const std::string& warn : results.warnings) {
            fprintf(stderr, "update_dyld_sim_shared_cache: warning: %s %s\n", fileSet.archName.c_str(), warn.c_str());
        }
        if ( !results.errorMessage.empty() ) {
            // print error (if one)
            fprintf(stderr, "update_dyld_sim_shared_cache: %s\n", results.errorMessage.c_str());
            cacheBuildFailure = true;
        }
        else {
            // save new cache file to disk and write new .map file
            assert(results.cacheContent != nullptr);
            if ( !safeSave(results.cacheContent, results.cacheLength, outFile) )
                cacheBuildFailure = true;
            if ( !cacheBuildFailure ) {
                std::string mapStr = results.cacheContent->mapFile();
                std::string outFileMap = cacheDir + "/dyld_shared_cache_" + fileSet.archName + ".map";
                safeSave(mapStr.c_str(), mapStr.size(), outFileMap);
            }
            // free created cache buffer
            vm_deallocate(mach_task_self(), (vm_address_t)results.cacheContent, results.cacheLength);
        }
    });

    // we could unmap all input files, but tool is about to quit

    return (cacheBuildFailure ? 1 : 0);
}


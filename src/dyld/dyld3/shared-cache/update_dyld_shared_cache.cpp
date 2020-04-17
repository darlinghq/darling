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
#include <Bom/Bom.h>
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
    std::unordered_set<std::string>             badZippered;
};

static const char* sAllowedPrefixes[] = {
    "/bin/",
    "/sbin/",
    "/usr/",
    "/System/",
    "/Library/Apple/System/",
    "/Library/Apple/usr/",
    "/System/Applications/App Store.app/",
    "/System/Applications/Automator.app/",
    "/System/Applications/Calculator.app/",
    "/System/Applications/Calendar.app/",
    "/System/Applications/Chess.app/",
    "/System/Applications/Contacts.app/",
    "/System/Applications/Dashboard.app/",
    "/System/Applications/Dictionary.app/",
    "/System/Applications/FaceTime.app/",
    "/System/Applications/Font Book.app/",
    "/System/Applications/Image Capture.app/",
    "/System/Applications/Launchpad.app/",
    "/System/Applications/Mail.app/",
    "/System/Applications/Maps.app/",
    "/System/Applications/Messages.app/",
    "/System/Applications/Mission Control.app/",
    "/System/Applications/Notes.app/",
    "/System/Applications/Photo Booth.app/",
    "/System/Applications/Preview.app/",
    "/System/Applications/QuickTime Player.app/",
    "/System/Applications/Reminders.app/",
    "/Applications/Safari.app/",
    "/System/Applications/Siri.app/",
    "/System/Applications/Stickies.app/",
    "/System/Applications/System Preferences.app/",
    "/System/Applications/TextEdit.app/",
    "/System/Applications/Time Machine.app/",
    "/System/Applications/iBooks.app/",
    "/System/Applications/iTunes.app/",
    "/System/Applications/Utilities/Activity Monitor.app",
    "/System/Applications/Utilities/AirPort Utility.app",
    "/System/Applications/Utilities/Audio MIDI Setup.app",
    "/System/Applications/Utilities/Bluetooth File Exchange.app",
    "/System/Applications/Utilities/Boot Camp Assistant.app",
    "/System/Applications/Utilities/ColorSync Utility.app",
    "/System/Applications/Utilities/Console.app",
    "/System/Applications/Utilities/Digital Color Meter.app",
    "/System/Applications/Utilities/Disk Utility.app",
    "/System/Applications/Utilities/Grab.app",
    "/System/Applications/Utilities/Grapher.app",
    "/System/Applications/Utilities/Keychain Access.app",
    "/System/Applications/Utilities/Migration Assistant.app",
    "/System/Applications/Utilities/Script Editor.app",
    "/System/Applications/Utilities/System Information.app",
    "/System/Applications/Utilities/Terminal.app",
    "/System/Applications/Utilities/VoiceOver Utility.app",
    "/Library/CoreMediaIO/Plug-Ins/DAL/"                // temp until plugins moved or closured working
};

static const char* sDontUsePrefixes[] = {
    "/usr/share",
    "/usr/local/",
    "/System/Library/Assets",
    "/System/Library/StagedFrameworks",
    "/Library/Apple/System/Library/StagedFrameworks",
    "/System/Library/Kernels/",
    "/bin/zsh",                             // until <rdar://31026756> is fixed
    "/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/Metadata.framework/Versions/A/Support/mdworker", // these load third party plugins
    "/usr/bin/mdimport", // these load third party plugins
};


static bool verbose = false;




static bool addIfMachO(const dyld3::closure::FileSystem& fileSystem, const std::string& runtimePath, const struct stat& statBuf,
                       bool requireSIP, dev_t rootFS, std::vector<MappedMachOsByCategory>& files)
{
    // don't precompute closure info for any debug or profile dylibs
    if ( endsWith(runtimePath, "_profile.dylib") || endsWith(runtimePath, "_debug.dylib") || endsWith(runtimePath, "_profile") || endsWith(runtimePath, "_debug") )
        return false;
    if ( startsWith(runtimePath, "/usr/lib/system/introspection/") )
        return false;

#if !BUILDING_UPDATE_OTHER_DYLD_CACHE_BUILDER
    // Only use files on the same volume as the boot volume
    if (statBuf.st_dev != rootFS) {
        if ( verbose )
            fprintf(stderr, "update_dyld_shared_cache: warning: skipping overlay file '%s' which is not on the root volume\n", runtimePath.c_str());
       return false;
    }
#endif

    auto warningHandler = ^(const char* msg) {
        if ( verbose )
            fprintf(stderr, "update_dyld_shared_cache: warning: cannot build dlopen closure for '%s' because %s\n", runtimePath.c_str(), msg);
    };

    bool result = false;
    for (MappedMachOsByCategory& file : files) {
        Diagnostics diag;
        char realerPath[MAXPATHLEN];
        dyld3::closure::LoadedFileInfo loadedFileInfo = dyld3::MachOAnalyzer::load(diag, fileSystem, runtimePath.c_str(), file.archs, dyld3::Platform::macOS, realerPath);
        if (diag.hasError() ) {
            // Try again with iOSMac
            diag.clearError();
            loadedFileInfo = dyld3::MachOAnalyzer::load(diag, fileSystem, runtimePath.c_str(), file.archs, dyld3::Platform::iOSMac, realerPath);
        }
        const dyld3::MachOAnalyzer* ma = (const dyld3::MachOAnalyzer*)loadedFileInfo.fileContent;
        if ( ma != nullptr ) {
            bool           issetuid       = false;
            const uint64_t sliceLen       = loadedFileInfo.sliceLen;
            const bool     isSipProtected = loadedFileInfo.isSipProtected;
            if ( ma->isDynamicExecutable() ) {
                // When SIP enabled, only build closures for SIP protected programs
                if ( !requireSIP || isSipProtected ) {
                    //fprintf(stderr, "requireSIP=%d, sipProtected=%d, path=%s\n", requireSIP, sipProtected, fullPath.c_str());
                    issetuid = (statBuf.st_mode & (S_ISUID|S_ISGID));
                    file.mainExecutables.emplace_back(runtimePath, ma, sliceLen, issetuid, isSipProtected, loadedFileInfo.sliceOffset, statBuf.st_mtime, statBuf.st_ino);
                }
            }
            else if ( ma->canBePlacedInDyldCache(runtimePath.c_str(), ^(const char* msg) {
                if (verbose)
                    fprintf(stderr, "update_dyld_shared_cache: warning dylib located at '%s' cannot be placed in cache because: %s\n", runtimePath.c_str(), msg);
            }) ) {
                // when SIP is enabled, only dylib protected by SIP can go in cache
                if ( !requireSIP || isSipProtected )
                    file.dylibsForCache.emplace_back(runtimePath, ma, sliceLen, issetuid, isSipProtected, loadedFileInfo.sliceOffset, statBuf.st_mtime, statBuf.st_ino);
                else if ( ma->canHavePrecomputedDlopenClosure(runtimePath.c_str(), warningHandler) )
                    file.otherDylibsAndBundles.emplace_back(runtimePath, ma, sliceLen, issetuid, isSipProtected, loadedFileInfo.sliceOffset, statBuf.st_mtime, statBuf.st_ino);
            }
            else {
                if ( ma->isDylib() ) {
                    std::string installName = ma->installName();
                    if ( startsWith(installName, "@") && !contains(runtimePath, ".app/") && !contains(runtimePath, ".xpc/") ) {
                        if ( dyld3::MachOFile::isSharedCacheEligiblePath(runtimePath.c_str()) )
                            fprintf(stderr, "update_dyld_shared_cache: warning @rpath install name for system framework: %s\n", runtimePath.c_str());
                    }
                }
                if ( ma->canHavePrecomputedDlopenClosure(runtimePath.c_str(), warningHandler) ) {
                    // Only add a dlopen closure for objc trampolines.  The rest should have been shared cache eligible.
                    bool addClosure = false;
                    if ( ma->isDylib() ) {
                        std::string installName = ma->installName();
                        addClosure = installName == "/usr/lib/libobjc-trampolines.dylib";
                    } else {
                        addClosure = true;
                    }
                    if (addClosure)
                        file.otherDylibsAndBundles.emplace_back(runtimePath, ma, sliceLen, issetuid, isSipProtected, loadedFileInfo.sliceOffset, statBuf.st_mtime, statBuf.st_ino);
                }
            }
            result = true;
        }
    }

    return result;
}

static void findAllFiles(const dyld3::closure::FileSystem& fileSystem, const std::vector<std::string>& pathPrefixes,
                         bool requireSIP, dev_t rootFS, std::vector<MappedMachOsByCategory>& files)
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

                // ignore files too small
                if ( statBuf.st_size < 0x3000 )
                    return;

                // don't add paths already found using previous prefix
                if ( multiplePrefixes && (alreadyUsed.count(path) != 0) )
                    return;

                // if the file is mach-o, add to list
                if ( addIfMachO(fileSystem, path, statBuf, requireSIP, rootFS, files) ) {
                    if ( multiplePrefixes )
                        alreadyUsed.insert(path);
                }
            });
        }
    }
}

static const char* sReceiptLocations[] = {
    "/System/Library/Receipts",
    "/Library/Apple/System/Library/Receipts"
};

static void findOSFilesViaBOMS(const dyld3::closure::FileSystem& fileSystem, const std::vector<std::string>& pathPrefixes,
                               bool requireSIP, dev_t rootFS, std::vector<MappedMachOsByCategory>& files)
{
    __block std::unordered_set<std::string> runtimePathsFound;
    __block bool foundUsableBom = false;
    for (const std::string& prefix : pathPrefixes) {
        for (const char* dirToIterate : sReceiptLocations ) {
            iterateDirectoryTree(prefix, dirToIterate, ^(const std::string&) { return false; }, ^(const std::string& path, const struct stat& statBuf) {
                if ( !contains(path, "com.apple.pkg.") )
                    return;
                if ( !endsWith(path, ".bom") )
                    return;
                std::string fullPath = prefix + path;
                BOMBom bom = BOMBomOpenWithSys(fullPath.c_str(), false, NULL);
                if ( bom == nullptr )
                    return;
                BOMFSObject rootFso = BOMBomGetRootFSObject(bom);
                if ( rootFso == nullptr ) {
                    BOMBomFree(bom);
                    return;
                }
                BOMBomEnumerator e = BOMBomEnumeratorNew(bom, rootFso);
                if ( e == nullptr ) {
                    fprintf(stderr, "Can't get enumerator for BOM root FSObject\n");
                    return;
                }
                BOMFSObjectFree(rootFso);
                //fprintf(stderr, "using BOM %s\n", path.c_str());
                foundUsableBom = true;
                while (BOMFSObject fso = BOMBomEnumeratorNext(e)) {
                    if ( BOMFSObjectIsBinaryObject(fso) ) {
                        const char* runPath = BOMFSObjectPathName(fso);
                        if ( (runPath[0] == '.') && (runPath[1] == '/') )
                            ++runPath;
                        // <rdar://problem/48748330> update_dyld_shared_cache needs to fold away /S/L/Templates/Data
                        if (strncmp(runPath, "/System/Library/Templates/Data/", 31) == 0 )
                            runPath = &runPath[30];
                        if ( runtimePathsFound.count(runPath) == 0 ) {
                            // only add files from sAllowedPrefixes and not in sDontUsePrefixes
                            bool inSearchDir = false;
                            for (const char* searchDir : sAllowedPrefixes ) {
                                if ( strncmp(searchDir, runPath, strlen(searchDir)) == 0 )  {
                                    inSearchDir = true;
                                    break;
                                }
                            }
                            if ( inSearchDir ) {
                                bool inSkipDir = false;
                                for (const char* skipDir : sDontUsePrefixes) {
                                    if ( strncmp(skipDir, runPath, strlen(skipDir)) == 0 )  {
                                        inSkipDir = true;
                                        break;
                                    }
                                }
                                if ( !inSkipDir ) {
                                    for (const std::string& prefix2 : pathPrefixes) {
                                        struct stat statBuf2;
                                        std::string fullPath2 = prefix2 + runPath;
                                        if ( stat(fullPath2.c_str(), &statBuf2) == 0 ) {
                                            if ( addIfMachO(fileSystem, runPath, statBuf2, requireSIP, rootFS, files) ) {
                                                runtimePathsFound.insert(runPath);
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    BOMFSObjectFree(fso);
                }

                BOMBomEnumeratorFree(e);
                BOMBomFree(bom);
            });
        }
    }

    if (!foundUsableBom)
        fprintf(stderr, "update_dyld_shared_cache: warning: No usable BOM files were found in '/System/Library/Receipts'\n");
}


static bool dontCache(const std::string& volumePrefix, const std::string& archName,
                      const std::unordered_set<std::string>& pathsWithDuplicateInstallName,
                      const std::unordered_set<std::string>& badZippered,
                      const DyldSharedCache::MappedMachO& aFile, bool warn,
                      const std::unordered_set<std::string>& skipDylibs)
{
    if ( skipDylibs.count(aFile.runtimePath) )
        return true;
    if ( startsWith(aFile.runtimePath, "/usr/lib/system/introspection/") )
        return true;
    if ( startsWith(aFile.runtimePath, "/System/Library/QuickTime/") )
        return true;
    if ( startsWith(aFile.runtimePath, "/System/Library/Tcl/") )
        return true;
    if ( startsWith(aFile.runtimePath, "/System/Library/Perl/") )
        return true;
    if ( startsWith(aFile.runtimePath, "/System/Library/MonitorPanels/") )
        return true;
    if ( startsWith(aFile.runtimePath, "/System/Library/Accessibility/") )
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
        if (warn) fprintf(stderr, "update_dyld_shared_cache: warning: %s skipping because of bad install name %s\n", archName.c_str(), aFile.runtimePath.c_str());
        return true;
    }

    const char* installName = aFile.mh->installName();
    if ( (pathsWithDuplicateInstallName.count(aFile.runtimePath) != 0) && (aFile.runtimePath != installName) ) {
        // <rdar://problem/46431467> if a dylib moves and a symlink is installed into its place, bom iterator will see both and issue a warning
        struct stat statBuf;
        bool isSymLink = ( (lstat(aFile.runtimePath.c_str(), &statBuf) == 0) && S_ISLNK(statBuf.st_mode) );
        if (!isSymLink && warn) fprintf(stderr, "update_dyld_shared_cache: warning: %s skipping because of duplicate install name %s\n", archName.c_str(), aFile.runtimePath.c_str());
        return true;
    }

    if (badZippered.count(aFile.runtimePath)) {
        return true;
    }

    if ( aFile.runtimePath != installName ) {
        // see if install name is a symlink to actual path
        std::string fullInstall = volumePrefix + installName;
        char resolvedPath[PATH_MAX];
        if ( realpath(fullInstall.c_str(), resolvedPath) != NULL ) {
            std::string resolvedSymlink = resolvedPath;
            if ( !volumePrefix.empty() ) {
                resolvedSymlink = resolvedSymlink.substr(volumePrefix.size());
            }
            if ( aFile.runtimePath == resolvedSymlink ) {
                return false;
            }
        }
        // <rdar://problem/38000411> also if runtime path is a symlink to install name
        std::string fullRuntime = volumePrefix + aFile.runtimePath;
        if ( realpath(fullRuntime.c_str(), resolvedPath) != NULL ) {
            std::string resolvedSymlink = resolvedPath;
            if ( !volumePrefix.empty() ) {
                resolvedSymlink = resolvedSymlink.substr(volumePrefix.size());
            }
            if ( resolvedSymlink == installName ) {
                return false;
            }
        }
        if (warn) fprintf(stderr, "update_dyld_shared_cache: warning: %s skipping because of bad install name %s\n", archName.c_str(), aFile.runtimePath.c_str());
        return true;
    }
    return false;
}

static void pruneCachedDylibs(const std::string& volumePrefix, const std::unordered_set<std::string>& skipDylibs,
                              MappedMachOsByCategory& fileSet, bool warn)
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

    std::unordered_map<std::string, std::string> macOSPathToTwinPath;
    for (const auto& entry : installNameToFirstPath) {
        if ( startsWith(entry.first, "/System/iOSSupport/") ) {
            std::string tail = entry.first.substr(18);
            if ( installNameToFirstPath.count(tail) != 0 ) {
                macOSPathToTwinPath.insert({ tail, entry.first });
            }
        }
    }

    for (DyldSharedCache::MappedMachO& aFile : fileSet.dylibsForCache) {
        if ( aFile.mh->isZippered() ) {
            aFile.mh->forEachDependentDylib(^(const char* loadPath, bool isWeak, bool isReExport, bool isUpward, uint32_t compatVersion, uint32_t curVersion, bool& stop) {
                auto macOSAndTwinPath = macOSPathToTwinPath.find(loadPath);
                if ( macOSAndTwinPath != macOSPathToTwinPath.end() ) {
                    if ( warn ) {
                        fprintf(stderr, "update_dyld_shared_cache: warning: evicting UIKitForMac binary: %s as it is linked by zippered binary %s\n",
                                macOSAndTwinPath->second.c_str(), aFile.runtimePath.c_str());
                    }
                    fileSet.badZippered.insert(macOSAndTwinPath->second);
                }
            });
        }
    }

    for (DyldSharedCache::MappedMachO& aFile : fileSet.dylibsForCache) {
        if ( dontCache(volumePrefix, fileSet.archs.name(), pathsWithDuplicateInstallName, fileSet.badZippered, aFile, true, skipDylibs) ){
            // <rdar://problem/46423929> don't build dlopen closures for symlinks to something in the dyld cache
            if ( pathsWithDuplicateInstallName.count(aFile.runtimePath) == 0 )
                fileSet.otherDylibsAndBundles.push_back(aFile);
        }
    }
    fileSet.dylibsForCache.erase(std::remove_if(fileSet.dylibsForCache.begin(), fileSet.dylibsForCache.end(),
        [&](const DyldSharedCache::MappedMachO& aFile) { return dontCache(volumePrefix, fileSet.archs.name(), pathsWithDuplicateInstallName, fileSet.badZippered, aFile, false, skipDylibs); }),
        fileSet.dylibsForCache.end());
}

static void pruneOtherDylibs(const std::string& volumePrefix, MappedMachOsByCategory& fileSet)
{
    // other OS dylibs should not contain dylibs that are embedded in some .app bundle
    fileSet.otherDylibsAndBundles.erase(std::remove_if(fileSet.otherDylibsAndBundles.begin(), fileSet.otherDylibsAndBundles.end(),
                                                       [&](const DyldSharedCache::MappedMachO& aFile) { return (aFile.runtimePath.find(".app/") != std::string::npos); }),
                                        fileSet.otherDylibsAndBundles.end());
}
static void pruneExecutables(const std::string& volumePrefix, MappedMachOsByCategory& fileSet)
{
    // don't build closures for xcode shims in /usr/bin (e.g. /usr/bin/clang) which re-exec themselves to a tool inside Xcode.app
    fileSet.mainExecutables.erase(std::remove_if(fileSet.mainExecutables.begin(), fileSet.mainExecutables.end(),
        [&](const DyldSharedCache::MappedMachO& aFile) {
            if ( !startsWith(aFile.runtimePath, "/usr/bin/") )
                return false;
            __block bool isXcodeShim = false;
            aFile.mh->forEachDependentDylib(^(const char* loadPath, bool, bool, bool, uint32_t, uint32_t, bool &stop) {
                if ( strcmp(loadPath, "/usr/lib/libxcselect.dylib") == 0 )
                    isXcodeShim = true;
            });
            return isXcodeShim;
        }), fileSet.mainExecutables.end());
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

static bool runningOnHaswell()
{
    // check system is capable of running x86_64h code
    struct host_basic_info  info;
    mach_msg_type_number_t  count    = HOST_BASIC_INFO_COUNT;
    mach_port_t             hostPort = mach_host_self();
    kern_return_t           result   = host_info(hostPort, HOST_BASIC_INFO, (host_info_t)&info, &count);
    mach_port_deallocate(mach_task_self(), hostPort);

    return ( (result == KERN_SUCCESS) && (info.cpu_subtype == CPU_SUBTYPE_X86_64_H) );
}

#if !BUILDING_UPDATE_OTHER_DYLD_CACHE_BUILDER
static std::string currentToolRealPath()
{
    char curToolPath[PATH_MAX];
    uint32_t curToolPathsize = PATH_MAX;
    int result = _NSGetExecutablePath(curToolPath, &curToolPathsize);
    if ( result == 0 ) {
        char resolvedCurToolPath[PATH_MAX];
        if ( realpath(curToolPath, resolvedCurToolPath) != NULL )
            return resolvedCurToolPath;
        else
            return curToolPath;
    }
    return "/usr/bin/update_dyld_shared_cache";
}
#endif

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
    std::string                     overlayPath;
    std::string                     dylibListFile;
    bool                            universal = false;
    bool                            force = false;
    bool                            searchDisk = false;
    bool                            dylibsRemoved = false;
    std::string                     cacheDir;
    std::unordered_set<std::string> archStrs;
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
        else if (strcmp(arg, "-dont_map_local_symbols") == 0) {
            //We are going to ignore this
        }
        else if (strcmp(arg, "-dylib_list") == 0) {
            TERMINATE_IF_LAST_ARG("-dylib_list missing argument");
            dylibListFile = argv[++i];
        }
        else if ((strcmp(arg, "-root") == 0) || (strcmp(arg, "--root") == 0)) {
            TERMINATE_IF_LAST_ARG("-root missing path argument\n");
            rootPath = argv[++i];
        }
        else if (strcmp(arg, "-overlay") == 0) {
            TERMINATE_IF_LAST_ARG("-overlay missing path argument\n");
            overlayPath = argv[++i];
        }
        else if (strcmp(arg, "-cache_dir") == 0) {
            TERMINATE_IF_LAST_ARG("-cache_dir missing path argument\n");
            cacheDir = argv[++i];
        }
        else if (strcmp(arg, "-arch") == 0) {
            TERMINATE_IF_LAST_ARG("-arch missing argument\n");
            archStrs.insert(argv[++i]);
        }
        else if (strcmp(arg, "-search_disk") == 0) {
            searchDisk = true;
        }
        else if (strcmp(arg, "-dylibs_removed_in_mastering") == 0) {
            dylibsRemoved = true;
        }
        else if (strcmp(arg, "-force") == 0) {
            force = true;
        }
        else if (strcmp(arg, "-sort_by_name") == 0) {
            //No-op, we always do this now
        }
        else if (strcmp(arg, "-universal_boot") == 0) {
            universal = true;
        }
        else if (strcmp(arg, "-skip") == 0) {
            TERMINATE_IF_LAST_ARG("-skip missing argument\n");
            skipDylibs.insert(argv[++i]);
        }
        else {
            //usage();
            fprintf(stderr, "update_dyld_shared_cache: unknown option: %s\n", arg);
            return 1;
        }
    }

    if ( !rootPath.empty() & !overlayPath.empty() ) {
        fprintf(stderr, "-root and -overlay cannot be used together\n");
        return 1;
    }
    // canonicalize rootPath
    if ( !rootPath.empty() ) {
        char resolvedPath[PATH_MAX];
        if ( realpath(rootPath.c_str(), resolvedPath) != NULL ) {
            rootPath = resolvedPath;
        }
        // <rdar://problem/33223984> when building closures for boot volume, pathPrefixes should be empty
        if ( rootPath == "/" ) {
            rootPath = "";
        }
    }
    // canonicalize overlayPath
    if ( !overlayPath.empty() ) {
        char resolvedPath[PATH_MAX];
        if ( realpath(overlayPath.c_str(), resolvedPath) != NULL ) {
            overlayPath = resolvedPath;
        }
    }

#if !BUILDING_UPDATE_OTHER_DYLD_CACHE_BUILDER
    // <rdar://problem/36362221> update_dyld_shared_cache -root should re-exec() itself to a newer version
    std::string newTool;
    if ( !rootPath.empty() )
        newTool = rootPath + "/usr/bin/update_dyld_shared_cache_root_mode";
    else if ( !overlayPath.empty() )
        newTool = overlayPath + "/usr/bin/update_dyld_shared_cache";
    if ( !newTool.empty() ) {
        struct stat newToolStatBuf;
        if ( stat(newTool.c_str(), &newToolStatBuf) == 0 ) {
            // don't re-exec if we are already running that tool
            if ( newTool != currentToolRealPath() ) {
                argv[0] = newTool.c_str();
                execve(newTool.c_str(), (char**)argv, (char**)envp);
                fprintf(stderr, "update_dyld_shared_cache: error: could not find '%s/usr/bin/update_dyld_shared_cache_root_mode' in target volume\n", rootPath.c_str());
                return 1;
            }
        }
        if ( !rootPath.empty() ) {
            // could be old macOS dmg, try old tool name
            newTool = rootPath + "/usr/bin/update_dyld_shared_cache";
            if ( stat(newTool.c_str(), &newToolStatBuf) == 0 ) {
                // don't re-exec if we are already running that tool
                if ( newTool != currentToolRealPath() ) {
                    argv[0] = newTool.c_str();
                    execve(newTool.c_str(), (char**)argv, (char**)envp);
                }
            }
            fprintf(stderr, "update_dyld_shared_cache: error: could not find '%s/usr/bin/update_dyld_shared_cache_root_mode' in target volume\n", rootPath.c_str());
            return 1;
        }
    }
#else
    if ( rootPath.empty() ) {
        fprintf(stderr, "update_dyld_shared_cache_root_mode: error: -root option missing\n");
        return 1;
    }
#endif

    // Find the boot volume so that we can ensure all overlays are on the same volume
    struct stat rootStatBuf;
    if ( stat(rootPath == "" ? "/" : rootPath.c_str(), &rootStatBuf) != 0 ) {
        fprintf(stderr, "update_dyld_shared_cache: error: could not stat root file system because '%s'\n", strerror(errno));
        return 1;
    }
    dev_t rootFS = rootStatBuf.st_dev;


    //
    // pathPrefixes for three modes:
    //   1) no options: { "" }           // search only boot volume
    //   2) -overlay:   { overlay, "" }  // search overlay, then boot volume
    //   3) -root:      { root }         // search only -root volume
    //
    std::vector<std::string> pathPrefixes;
    if ( !overlayPath.empty() ) {
        // Only add the overlay path if it exists, and is the same volume as the root
        struct stat overlayStatBuf;
        if ( stat(overlayPath.c_str(), &overlayStatBuf) != 0 ) {
            fprintf(stderr, "update_dyld_shared_cache: warning: ignoring overlay dir '%s' because '%s'\n", overlayPath.c_str(), strerror(errno));
            overlayPath.clear();
        }
        else {
            char resolvedOverlayPath[PATH_MAX];
            if ( realpath(overlayPath.c_str(), resolvedOverlayPath) != NULL ) {
                overlayPath = resolvedOverlayPath;
            }
            else {
                fprintf(stderr, "update_dyld_shared_cache: warning: ignoring overlay dir '%s' because realpath() failed\n", overlayPath.c_str());
                overlayPath.clear();
            }
        }
        if ( !overlayPath.empty() )
            pathPrefixes.push_back(overlayPath);
    }
    pathPrefixes.push_back(rootPath);

    // build FileSystem object
    const char* fsRoot    = rootPath.empty()    ? nullptr : rootPath.c_str();
    const char* fsOverlay = overlayPath.empty() ? nullptr : overlayPath.c_str();
    dyld3::closure::FileSystemPhysical fileSystem(fsRoot, fsOverlay);

    // normalize output directory
    if ( cacheDir.empty() ) {
        // if -cache_dir is not specified, then write() will eventually fail if we are not running as root
        if ( geteuid() != 0 ) {
            fprintf(stderr, "update_dyld_shared_cache: must be run as root (sudo)\n");
            return 1;
        }

        // write cache file into -root or -overlay directory, if used
        if ( rootPath != "/" )
            cacheDir = rootPath +  MACOSX_DYLD_SHARED_CACHE_DIR;
        else if ( !overlayPath.empty()  )
            cacheDir = overlayPath +  MACOSX_DYLD_SHARED_CACHE_DIR;
        else
            cacheDir = MACOSX_DYLD_SHARED_CACHE_DIR;
    }
    int err = mkpath_np(cacheDir.c_str(), S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH);
    if ( (err != 0) && (err != EEXIST) ) {
        fprintf(stderr, "update_dyld_shared_cache: could not access cache dir: mkpath_np(%s) failed errno=%d\n", cacheDir.c_str(), err);
        return 1;
    }
    // make sure cacheDir is always a real path, so it can be checked later to see if it changed
    char resolvedCachePath[PATH_MAX];
    ::realpath(cacheDir.c_str(), resolvedCachePath);
    cacheDir = resolvedCachePath;

#if BUILDING_UPDATE_OTHER_DYLD_CACHE_BUILDER
    bool requireDylibsBeRootlessProtected = false;
#else
    bool requireDylibsBeRootlessProtected = isProtectedBySIPExceptDyld(cacheDir);
    if ( requireDylibsBeRootlessProtected && !overlayPath.empty() && !isProtectedBySIP(overlayPath.c_str()) ) {
        fprintf(stderr, "update_dyld_shared_cache: warning: ignoring overlay dir '%s' because it is not SIP protected\n", overlayPath.c_str());
        overlayPath.clear();
        pathPrefixes.clear();
        pathPrefixes.push_back(rootPath);
    }
#endif

    if ( archStrs.empty() ) {
        // <rdar://44190126> check if OS has enough i386 to make a shared cache
        char realerPath[MAXPATHLEN];
        Diagnostics testDiag;
        const char* foundationPath = "/System/Library/Frameworks/Foundation.framework/Versions/C/Foundation";
        dyld3::closure::LoadedFileInfo foundationInfo = dyld3::MachOAnalyzer::load(testDiag, fileSystem, foundationPath, dyld3::GradedArchs::i386, dyld3::Platform::macOS, realerPath);
        bool foundationHas32bit = (foundationInfo.fileContent != NULL);
        if ( foundationHas32bit )
            fileSystem.unloadFile(foundationInfo);

        if ( universal ) {
            // <rdar://problem/26182089> -universal_boot should make all possible dyld caches
            if ( foundationHas32bit )
                archStrs.insert("i386");
            archStrs.insert("x86_64");
            archStrs.insert("x86_64h");
        }
        else {
            // just make caches for this machine
            if ( foundationHas32bit )
                archStrs.insert("i386");
            archStrs.insert(runningOnHaswell() ? "x86_64h" : "x86_64");
        }
    }

    uint64_t t1 = mach_absolute_time();

    // find all mach-o files for requested architectures
    __block std::vector<MappedMachOsByCategory> allFileSets;
    if ( archStrs.count("x86_64") )
        allFileSets.push_back({dyld3::GradedArchs::x86_64});
    if ( archStrs.count("x86_64h") )
        allFileSets.push_back({dyld3::GradedArchs::x86_64h});
    if ( archStrs.count("i386") )
        allFileSets.push_back({dyld3::GradedArchs::i386});
    if ( searchDisk )
        findAllFiles(fileSystem, pathPrefixes, requireDylibsBeRootlessProtected, rootFS, allFileSets);
    else {
        std::unordered_set<std::string> runtimePathsFound;
        findOSFilesViaBOMS(fileSystem, pathPrefixes, requireDylibsBeRootlessProtected, rootFS, allFileSets);
    }

    // nothing in OS uses i386 dylibs, so only dylibs used by third party apps need to be in cache
    for (MappedMachOsByCategory& fileSet : allFileSets) {
        pruneCachedDylibs(rootPath, skipDylibs, fileSet, verbose);
        pruneOtherDylibs(rootPath, fileSet);
        pruneExecutables(rootPath, fileSet);
   }

    uint64_t t2 = mach_absolute_time();
    if ( verbose ) {
        if ( searchDisk )
            fprintf(stderr, "time to scan file system and construct lists of mach-o files: %ums\n", absolutetime_to_milliseconds(t2-t1));
        else
            fprintf(stderr, "time to read BOM and construct lists of mach-o files: %ums\n", absolutetime_to_milliseconds(t2-t1));
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
        const std::string outFile = cacheDir + "/dyld_shared_cache_" + fileSet.archs.name();

        DyldSharedCache::MappedMachO (^loader)(const std::string&) = ^DyldSharedCache::MappedMachO(const std::string& runtimePath) {
            if ( skipDylibs.count(runtimePath) )
                return DyldSharedCache::MappedMachO();
            if (fileSet.badZippered.count(runtimePath)) {
                return DyldSharedCache::MappedMachO();
            }
            for (const std::string& prefix : pathPrefixes) {
                std::string fullPath = prefix + runtimePath;
                struct stat statBuf;
                if ( stat(fullPath.c_str(), &statBuf) == 0 ) {
                    char resolvedPath[PATH_MAX];
                    if ( realpath(fullPath.c_str(), resolvedPath) != NULL ) {
                        std::string resolvedSymlink = resolvedPath;
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
                   if ( addIfMachO(fileSystem, runtimePath, statBuf, requireDylibsBeRootlessProtected, rootFS, mappedFiles) ) {
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
        DyldSharedCache::verifySelfContained(fileSet.dylibsForCache, fileSet.badZippered, loader, excludes);
        for (size_t i=startCount; i < fileSet.dylibsForCache.size(); ++i) {
            fprintf(stderr, "update_dyld_shared_cache: warning: %s not in .bom, but adding required dylib %s\n", fileSet.archs.name(), fileSet.dylibsForCache[i].runtimePath.c_str());
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

         // add any extra dylibs needed which were not in .bom
        fprintf(stderr, "update_dyld_shared_cache: %s incorporating %lu OS dylibs, tracking %lu others, building closures for %lu executables\n",
                        fileSet.archs.name(), fileSet.dylibsForCache.size(), fileSet.otherDylibsAndBundles.size(), fileSet.mainExecutables.size());
        //for (const DyldSharedCache::MappedMachO& aFile : fileSet.otherDylibsAndBundles) {
        //    fprintf(stderr, "  %s\n", aFile.runtimePath.c_str());
        //}


        // build cache new cache file
        DyldSharedCache::CreateOptions options;
        options.outputFilePath               = outFile;
        options.outputMapFilePath            = cacheDir + "/dyld_shared_cache_" + fileSet.archs.name() + ".map";
        options.archs                        = &fileSet.archs;
        options.platform                     = dyld3::Platform::macOS;
        options.excludeLocalSymbols          = false;
        options.optimizeStubs                = false;
        options.optimizeObjC                 = true;
        options.codeSigningDigestMode        = DyldSharedCache::SHA256only;
        options.dylibsRemovedDuringMastering = dylibsRemoved;
        options.inodesAreSameAsRuntime       = true;
        options.cacheSupportsASLR            = (&fileSet.archs != &dyld3::GradedArchs::i386);
        options.forSimulator                 = false;
        options.isLocallyBuiltCache          = true;
        options.verbose                      = verbose;
        options.evictLeafDylibsOnOverflow    = true;
        DyldSharedCache::CreateResults results = DyldSharedCache::create(options, fileSystem, fileSet.dylibsForCache, fileSet.otherDylibsAndBundles, fileSet.mainExecutables);

        // print any warnings
        for (const std::string& warn : results.warnings) {
            fprintf(stderr, "update_dyld_shared_cache: warning: %s %s\n", fileSet.archs.name(), warn.c_str());
        }
        if ( results.errorMessage.empty() ) {
            wroteSomeCacheFile = true;
        }
        else {
            fprintf(stderr, "update_dyld_shared_cache: %s\n", results.errorMessage.c_str());
            cacheBuildFailure = true;
        }
    });


    // Save off spintrace data
    if ( wroteSomeCacheFile ) {
        void* h = dlopen("/usr/lib/libdscsym.dylib", 0);
        if ( h != nullptr ) {
            typedef int (*dscym_func)(const char*);
            dscym_func func = (dscym_func)dlsym(h, "dscsym_save_dscsyms_for_current_caches");
            std::string nuggetRoot = rootPath;
            if ( nuggetRoot.empty() )
                 nuggetRoot = overlayPath;
            if ( nuggetRoot.empty() )
                 nuggetRoot = "/";
            (*func)(nuggetRoot.c_str());
        }
    }


    // we could unmap all input files, but tool is about to quit

    return (cacheBuildFailure ? 1 : 0);
}


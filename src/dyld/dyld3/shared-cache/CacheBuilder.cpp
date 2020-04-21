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


#include <unistd.h>
#include <dirent.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach/mach_vm.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach/shared_region.h>
#include <assert.h>
#include <CommonCrypto/CommonHMAC.h>
#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonDigestSPI.h>
#include <pthread/pthread.h>
#include <apfs/apfs_fsctl.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "MachOFileAbstraction.hpp"
#include "CodeSigningTypes.h"
#include "DyldSharedCache.h"
#include "CacheBuilder.h"
#include "FileAbstraction.hpp"
#include "Trie.hpp"
#include "FileUtils.h"
#include "Diagnostics.h"
#include "ClosureBuilder.h"
#include "Closure.h"
#include "ClosureFileSystemNull.h"
#include "StringUtils.h"

#if __has_include("dyld_cache_config.h")
    #include "dyld_cache_config.h"
#else
    #define ARM_SHARED_REGION_START      0x1A000000ULL
    #define ARM_SHARED_REGION_SIZE       0x26000000ULL
    #define ARM64_SHARED_REGION_START   0x180000000ULL
    #define ARM64_SHARED_REGION_SIZE     0x100000000ULL
#endif

#ifndef ARM64_32_SHARED_REGION_START
    #define ARM64_32_SHARED_REGION_START 0x1A000000ULL
    #define ARM64_32_SHARED_REGION_SIZE  0x26000000ULL
#endif

#if ARM_SHARED_REGION_SIZE > 0x26000000ULL
  #define  ARMV7K_CHAIN_BITS    0xC0000000
  #define  ARMV7K_MAX           0x0
#else
  #define  ARMV7K_CHAIN_BITS    0xE0000000
  #define  ARMV7K_MAX           0x20000000
#endif

const CacheBuilder::ArchLayout CacheBuilder::_s_archLayout[] = {
    { 0x7FFF20000000ULL,            0xEFE00000ULL,              0x0,         0x40000000, 0x00FFFF0000000000, "x86_64",   CS_PAGE_SIZE_4K,  12, 2, true,  true,  true  },
    { 0x7FFF20000000ULL,            0xEFE00000ULL,              0x0,         0x40000000, 0x00FFFF0000000000, "x86_64h",  CS_PAGE_SIZE_4K,  12, 2, true,  true,  true  },
    { SHARED_REGION_BASE_I386,      SHARED_REGION_SIZE_I386,    0x0,         0x00200000,                0x0, "i386",     CS_PAGE_SIZE_4K,  12, 0, false, false, true  },
    { ARM64_SHARED_REGION_START,    ARM64_SHARED_REGION_SIZE,   0x0,         0x02000000, 0x00FFFF0000000000, "arm64",    CS_PAGE_SIZE_4K,  14, 2, false, true,  false },
#if SUPPORT_ARCH_arm64e
    { ARM64_SHARED_REGION_START,    ARM64_SHARED_REGION_SIZE,   0x0,         0x02000000, 0x00FFFF0000000000, "arm64e",   CS_PAGE_SIZE_16K, 14, 2, false, true,  false },
#endif
#if SUPPORT_ARCH_arm64_32
    { ARM64_32_SHARED_REGION_START, ARM64_32_SHARED_REGION_SIZE,0x0,         0x02000000,         0xC0000000, "arm64_32", CS_PAGE_SIZE_16K, 14, 6, false, false, true  },
#endif
    { ARM_SHARED_REGION_START,      ARM_SHARED_REGION_SIZE,     0x0,         0x02000000,         0xE0000000, "armv7s",   CS_PAGE_SIZE_4K,  14, 4, false, false, true  },
    { ARM_SHARED_REGION_START,      ARM_SHARED_REGION_SIZE,     ARMV7K_MAX,  0x00400000,  ARMV7K_CHAIN_BITS, "armv7k",   CS_PAGE_SIZE_4K,  14, 4, false, false, true  },
    { 0x40000000,                   0x40000000,                 0x0,         0x02000000,                0x0, "sim-x86",  CS_PAGE_SIZE_4K,  14, 0, false, false, true  }
};


// These are dylibs that may be interposed, so stubs calling into them should never be bypassed
const char* const CacheBuilder::_s_neverStubEliminateDylibs[] = {
    "/usr/lib/system/libdispatch.dylib",
    nullptr
};

// These are functions that are interposed by Instruments.app or ASan
const char* const CacheBuilder::_s_neverStubEliminateSymbols[] = {
    "___bzero",
    "___cxa_atexit",
    "___cxa_throw",
    "__longjmp",
    "__objc_autoreleasePoolPop",
    "_accept",
    "_access",
    "_asctime",
    "_asctime_r",
    "_asprintf",
    "_atoi",
    "_atol",
    "_atoll",
    "_calloc",
    "_chmod",
    "_chown",
    "_close",
    "_confstr",
    "_ctime",
    "_ctime_r",
    "_dispatch_after",
    "_dispatch_after_f",
    "_dispatch_async",
    "_dispatch_async_f",
    "_dispatch_barrier_async_f",
    "_dispatch_group_async",
    "_dispatch_group_async_f",
    "_dispatch_source_set_cancel_handler",
    "_dispatch_source_set_event_handler",
    "_dispatch_sync_f",
    "_dlclose",
    "_dlopen",
    "_dup",
    "_dup2",
    "_endgrent",
    "_endpwent",
    "_ether_aton",
    "_ether_hostton",
    "_ether_line",
    "_ether_ntoa",
    "_ether_ntohost",
    "_fchmod",
    "_fchown",
    "_fclose",
    "_fdopen",
    "_fflush",
    "_fopen",
    "_fork",
    "_fprintf",
    "_free",
    "_freopen",
    "_frexp",
    "_frexpf",
    "_frexpl",
    "_fscanf",
    "_fstat",
    "_fstatfs",
    "_fstatfs64",
    "_fsync",
    "_ftime",
    "_getaddrinfo",
    "_getattrlist",
    "_getcwd",
    "_getgrent",
    "_getgrgid",
    "_getgrgid_r",
    "_getgrnam",
    "_getgrnam_r",
    "_getgroups",
    "_gethostbyaddr",
    "_gethostbyname",
    "_gethostbyname2",
    "_gethostent",
    "_getifaddrs",
    "_getitimer",
    "_getnameinfo",
    "_getpass",
    "_getpeername",
    "_getpwent",
    "_getpwnam",
    "_getpwnam_r",
    "_getpwuid",
    "_getpwuid_r",
    "_getsockname",
    "_getsockopt",
    "_gmtime",
    "_gmtime_r",
    "_if_indextoname",
    "_if_nametoindex",
    "_index",
    "_inet_aton",
    "_inet_ntop",
    "_inet_pton",
    "_initgroups",
    "_ioctl",
    "_lchown",
    "_lgamma",
    "_lgammaf",
    "_lgammal",
    "_link",
    "_listxattr",
    "_localtime",
    "_localtime_r",
    "_longjmp",
    "_lseek",
    "_lstat",
    "_malloc",
    "_malloc_create_zone",
    "_malloc_default_purgeable_zone",
    "_malloc_default_zone",
    "_malloc_good_size",
    "_malloc_make_nonpurgeable",
    "_malloc_make_purgeable",
    "_malloc_set_zone_name",
    "_mbsnrtowcs",
    "_mbsrtowcs",
    "_mbstowcs",
    "_memchr",
    "_memcmp",
    "_memcpy",
    "_memmove",
    "_memset",
    "_mktime",
    "_mlock",
    "_mlockall",
    "_modf",
    "_modff",
    "_modfl",
    "_munlock",
    "_munlockall",
    "_objc_autoreleasePoolPop",
    "_objc_setProperty",
    "_objc_setProperty_atomic",
    "_objc_setProperty_atomic_copy",
    "_objc_setProperty_nonatomic",
    "_objc_setProperty_nonatomic_copy",
    "_objc_storeStrong",
    "_open",
    "_opendir",
    "_poll",
    "_posix_memalign",
    "_pread",
    "_printf",
    "_pthread_attr_getdetachstate",
    "_pthread_attr_getguardsize",
    "_pthread_attr_getinheritsched",
    "_pthread_attr_getschedparam",
    "_pthread_attr_getschedpolicy",
    "_pthread_attr_getscope",
    "_pthread_attr_getstack",
    "_pthread_attr_getstacksize",
    "_pthread_condattr_getpshared",
    "_pthread_create",
    "_pthread_getschedparam",
    "_pthread_join",
    "_pthread_mutex_lock",
    "_pthread_mutex_unlock",
    "_pthread_mutexattr_getprioceiling",
    "_pthread_mutexattr_getprotocol",
    "_pthread_mutexattr_getpshared",
    "_pthread_mutexattr_gettype",
    "_pthread_rwlockattr_getpshared",
    "_pwrite",
    "_rand_r",
    "_read",
    "_readdir",
    "_readdir_r",
    "_readv",
    "_readv$UNIX2003",
    "_realloc",
    "_realpath",
    "_recv",
    "_recvfrom",
    "_recvmsg",
    "_remquo",
    "_remquof",
    "_remquol",
    "_scanf",
    "_send",
    "_sendmsg",
    "_sendto",
    "_setattrlist",
    "_setgrent",
    "_setitimer",
    "_setlocale",
    "_setpwent",
    "_shm_open",
    "_shm_unlink",
    "_sigaction",
    "_sigemptyset",
    "_sigfillset",
    "_siglongjmp",
    "_signal",
    "_sigpending",
    "_sigprocmask",
    "_sigwait",
    "_snprintf",
    "_sprintf",
    "_sscanf",
    "_stat",
    "_statfs",
    "_statfs64",
    "_strcasecmp",
    "_strcat",
    "_strchr",
    "_strcmp",
    "_strcpy",
    "_strdup",
    "_strerror",
    "_strerror_r",
    "_strlen",
    "_strncasecmp",
    "_strncat",
    "_strncmp",
    "_strncpy",
    "_strptime",
    "_strtoimax",
    "_strtol",
    "_strtoll",
    "_strtoumax",
    "_tempnam",
    "_time",
    "_times",
    "_tmpnam",
    "_tsearch",
    "_unlink",
    "_valloc",
    "_vasprintf",
    "_vfprintf",
    "_vfscanf",
    "_vprintf",
    "_vscanf",
    "_vsnprintf",
    "_vsprintf",
    "_vsscanf",
    "_wait",
    "_wait$UNIX2003",
    "_wait3",
    "_wait4",
    "_waitid",
    "_waitid$UNIX2003",
    "_waitpid",
    "_waitpid$UNIX2003",
    "_wcslen",
    "_wcsnrtombs",
    "_wcsrtombs",
    "_wcstombs",
    "_wordexp",
    "_write",
    "_writev",
    "_writev$UNIX2003",
    // <rdar://problem/22050956> always use stubs for C++ symbols that can be overridden
    "__ZdaPv",
    "__ZdlPv",
    "__Znam",
    "__Znwm",

    nullptr
};


CacheBuilder::CacheBuilder(const DyldSharedCache::CreateOptions& options, const dyld3::closure::FileSystem& fileSystem)
    : _options(options)
    , _fileSystem(fileSystem)
    , _fullAllocatedBuffer(0)
    , _diagnostics(options.loggingPrefix, options.verbose)
    , _archLayout(nullptr)
    , _aliasCount(0)
    , _slideInfoFileOffset(0)
    , _slideInfoBufferSizeAllocated(0)
    , _allocatedBufferSize(0)
    , _selectorStringsFromExecutables(0)
{

    std::string targetArch = options.archs->name();
    if ( options.forSimulator && (options.archs == &dyld3::GradedArchs::i386) )
        targetArch = "sim-x86";

    for (const ArchLayout& layout : _s_archLayout) {
        if ( layout.archName == targetArch ) {
            _archLayout = &layout;
            break;
        }
    }

    if (!_archLayout) {
        _diagnostics.error("Tool was built without support for: '%s'", targetArch.c_str());
    }
}


std::string CacheBuilder::errorMessage()
{
    return _diagnostics.errorMessage();
}

const std::set<std::string> CacheBuilder::warnings()
{
    return _diagnostics.warnings();
}

const std::set<const dyld3::MachOAnalyzer*> CacheBuilder::evictions()
{
    return _evictions;
}

void CacheBuilder::deleteBuffer()
{
    // Cache buffer
    vm_deallocate(mach_task_self(), _fullAllocatedBuffer, _archLayout->sharedMemorySize);
    _fullAllocatedBuffer = 0;
    _allocatedBufferSize = 0;
    // Local symbols buffer
    if ( _localSymbolsRegion.bufferSize != 0 ) {
        vm_deallocate(mach_task_self(), (vm_address_t)_localSymbolsRegion.buffer, _localSymbolsRegion.bufferSize);
        _localSymbolsRegion.buffer = 0;
        _localSymbolsRegion.bufferSize = 0;
    }
    // Code singatures
    vm_deallocate(mach_task_self(), (vm_address_t)_codeSignatureRegion.buffer, _codeSignatureRegion.bufferSize);
    _codeSignatureRegion.buffer = 0;
    _codeSignatureRegion.bufferSize = 0;
}


void CacheBuilder::makeSortedDylibs(const std::vector<LoadedMachO>& dylibs, const std::unordered_map<std::string, unsigned> sortOrder)
{
    for (const LoadedMachO& dylib : dylibs) {
        _sortedDylibs.push_back({ &dylib, dylib.mappedFile.runtimePath, {} });
    }

    std::sort(_sortedDylibs.begin(), _sortedDylibs.end(), [&](const DylibInfo& a, const DylibInfo& b) {
        const auto& orderA = sortOrder.find(a.input->mappedFile.runtimePath);
        const auto& orderB = sortOrder.find(b.input->mappedFile.runtimePath);
        bool foundA = (orderA != sortOrder.end());
        bool foundB = (orderB != sortOrder.end());

        // Order all __DATA_DIRTY segments specified in the order file first, in
        // the order specified in the file, followed by any other __DATA_DIRTY
        // segments in lexicographic order.
        if ( foundA && foundB )
            return orderA->second < orderB->second;
        else if ( foundA )
            return true;
        else if ( foundB )
             return false;
        
        // Sort mac before iOSMac
        bool isIOSMacA = strncmp(a.input->mappedFile.runtimePath.c_str(), "/System/iOSSupport/", 19) == 0;
        bool isIOSMacB = strncmp(b.input->mappedFile.runtimePath.c_str(), "/System/iOSSupport/", 19) == 0;
        if (isIOSMacA != isIOSMacB)
            return !isIOSMacA;
        
        // Finally sort by path
        return a.input->mappedFile.runtimePath < b.input->mappedFile.runtimePath;
    });
}


inline uint32_t absolutetime_to_milliseconds(uint64_t abstime)
{
    return (uint32_t)(abstime/1000/1000);
}

struct DylibAndSize
{
    const CacheBuilder::LoadedMachO*    input;
    const char*                         installName;
    uint64_t                            size;
};

uint64_t CacheBuilder::cacheOverflowAmount()
{
    if ( _archLayout->sharedRegionsAreDiscontiguous ) {
        // for macOS x86_64 cache, need to check each region for overflow
        if ( _readExecuteRegion.sizeInUse > 0x60000000 )
            return (_readExecuteRegion.sizeInUse - 0x60000000);

        if ( _readWriteRegion.sizeInUse > 0x40000000 )
            return (_readWriteRegion.sizeInUse - 0x40000000);

        if ( _readOnlyRegion.sizeInUse > 0x3FE00000 )
            return (_readOnlyRegion.sizeInUse - 0x3FE00000);
    }
    else if ( _archLayout->textAndDataMaxSize != 0 ) {
        // for armv7k, limit is 512MB of TEX+DATA
        uint64_t totalTextAndData = _readWriteRegion.unslidLoadAddress + _readWriteRegion.sizeInUse - _readExecuteRegion.unslidLoadAddress;
        if ( totalTextAndData < _archLayout->textAndDataMaxSize )
            return 0;
        else
            return totalTextAndData - _archLayout->textAndDataMaxSize;
    }
    else {
        bool alreadyOptimized = (_readOnlyRegion.sizeInUse != _readOnlyRegion.bufferSize);
        uint64_t vmSize = _readOnlyRegion.unslidLoadAddress - _readExecuteRegion.unslidLoadAddress;
        if ( alreadyOptimized )
            vmSize += _readOnlyRegion.sizeInUse;
        else if ( _options.excludeLocalSymbols )
            vmSize += (_readOnlyRegion.sizeInUse * 37/100); // assume locals removal and LINKEDIT optimzation reduces LINKEDITs %37 of original size
        else
            vmSize += (_readOnlyRegion.sizeInUse * 80/100); // assume LINKEDIT optimzation reduces LINKEDITs to %80 of original size
        if ( vmSize > _archLayout->sharedMemorySize )
            return vmSize - _archLayout->sharedMemorySize;
    }
    // fits in shared region
    return 0;
}

size_t CacheBuilder::evictLeafDylibs(uint64_t reductionTarget, std::vector<const LoadedMachO*>& overflowDylibs)
{
    // build a reverse map of all dylib dependencies
    __block std::map<std::string, std::set<std::string>> references;
    std::map<std::string, std::set<std::string>>* referencesPtr = &references;
    for (const DylibInfo& dylib : _sortedDylibs) {
        // Esnure we have an entry (even if it is empty)
        if (references.count(dylib.input->mappedFile.mh->installName()) == 0) {
            references[dylib.input->mappedFile.mh->installName()] = std::set<std::string>();
        };
        dylib.input->mappedFile.mh->forEachDependentDylib(^(const char* loadPath, bool isWeak, bool isReExport, bool isUpward, uint32_t compatVersion, uint32_t curVersion, bool &stop) {
            references[loadPath].insert(dylib.input->mappedFile.mh->installName());
        });
    }

    // Find the sizes of all the dylibs
    std::vector<DylibAndSize> dylibsToSort;
    std::vector<DylibAndSize> sortedDylibs;
    for (const DylibInfo& dylib : _sortedDylibs) {
        const char* installName = dylib.input->mappedFile.mh->installName();
        __block uint64_t segsSize = 0;
        dylib.input->mappedFile.mh->forEachSegment(^(const dyld3::MachOFile::SegmentInfo& info, bool& stop) {
            if ( strcmp(info.segName, "__LINKEDIT") != 0 )
                segsSize += info.vmSize;
        });
        dylibsToSort.push_back({ dylib.input, installName, segsSize });
    }

    // Build an ordered list of what to remove. At each step we do following
    // 1) Find all dylibs that nothing else depends on
    // 2a) If any of those dylibs are not in the order select the largest one of them
    // 2b) If all the leaf dylibs are in the order file select the last dylib that appears last in the order file
    // 3) Remove all entries to the removed file from the reverse dependency map
    // 4) Go back to one and repeat until there are no more evictable dylibs
    // This results in us always choosing the locally optimal selection, and then taking into account how that impacts
    // the dependency graph for subsequent selections
    
    bool candidateFound = true;
    while (candidateFound) {
        candidateFound = false;
        DylibAndSize candidate;
        uint64_t candidateOrder = 0;
        for(const auto& dylib : dylibsToSort) {
            const auto &i = referencesPtr->find(dylib.installName);
            assert(i != referencesPtr->end());
            if (!i->second.empty()) {
                continue;
            }
            const auto& j = _options.dylibOrdering.find(dylib.input->mappedFile.runtimePath);
            uint64_t order = 0;
            if (j != _options.dylibOrdering.end()) {
                order = j->second;
            } else {
                // Not in the order file, set order sot it goes to the front of the list
                order = UINT64_MAX;
            }
            if (order > candidateOrder ||
                (order == UINT64_MAX && candidate.size < dylib.size)) {
                    // The new file is either a lower priority in the order file
                    // or the same priority as the candidate but larger
                    candidate = dylib;
                    candidateOrder = order;
                    candidateFound = true;
            }
        }
        if (candidateFound) {
            sortedDylibs.push_back(candidate);
            referencesPtr->erase(candidate.installName);
            for (auto& dependent : references) {
                (void)dependent.second.erase(candidate.installName);
            }
            auto j = std::find_if(dylibsToSort.begin(), dylibsToSort.end(), [&candidate](const DylibAndSize& dylib) {
                return (strcmp(candidate.installName, dylib.installName) == 0);
            });
            if (j != dylibsToSort.end()) {
                dylibsToSort.erase(j);
            }
        }
    }

     // build set of dylibs that if removed will allow cache to build
    for (DylibAndSize& dylib : sortedDylibs) {
        if ( _options.verbose )
            _diagnostics.warning("to prevent cache overflow, not caching %s", dylib.installName);
        _evictions.insert(dylib.input->mappedFile.mh);
        // Track the evicted dylibs so we can try build "other" dlopen closures for them.
        overflowDylibs.push_back(dylib.input);
        if ( dylib.size > reductionTarget )
            break;
        reductionTarget -= dylib.size;
    }

    // prune _sortedDylibs
    _sortedDylibs.erase(std::remove_if(_sortedDylibs.begin(), _sortedDylibs.end(), [&](const DylibInfo& dylib) {
        return (_evictions.count(dylib.input->mappedFile.mh) != 0);
    }),_sortedDylibs.end());

    return _evictions.size();
}

// Handles building a list of input files to the CacheBuilder itself.
class CacheInputBuilder {
public:
    CacheInputBuilder(const dyld3::closure::FileSystem& fileSystem,
                      const dyld3::GradedArchs& archs, dyld3::Platform reqPlatform)
    : fileSystem(fileSystem), reqArchs(archs), reqPlatform(reqPlatform) { }

    // Loads and maps any MachOs in the given list of files.
    void loadMachOs(std::vector<CacheBuilder::InputFile>& inputFiles,
                    std::vector<CacheBuilder::LoadedMachO>& dylibsToCache,
                    std::vector<CacheBuilder::LoadedMachO>& otherDylibs,
                    std::vector<CacheBuilder::LoadedMachO>& executables,
                    std::vector<CacheBuilder::LoadedMachO>& couldNotLoadFiles) {

        std::map<std::string, uint64_t> dylibInstallNameMap;
        for (CacheBuilder::InputFile& inputFile : inputFiles) {
            char realerPath[MAXPATHLEN];
            dyld3::closure::LoadedFileInfo loadedFileInfo = dyld3::MachOAnalyzer::load(inputFile.diag, fileSystem, inputFile.path, reqArchs, reqPlatform, realerPath);
            const dyld3::MachOAnalyzer* ma = (const dyld3::MachOAnalyzer*)loadedFileInfo.fileContent;
            if (ma == nullptr) {
                couldNotLoadFiles.emplace_back((CacheBuilder::LoadedMachO){ DyldSharedCache::MappedMachO(), loadedFileInfo, &inputFile });
                continue;
            }

            DyldSharedCache::MappedMachO mappedFile(inputFile.path, ma, loadedFileInfo.sliceLen, false, false,
                                                    loadedFileInfo.sliceOffset, loadedFileInfo.mtime, loadedFileInfo.inode);

            // The file can be loaded with the given slice, but we may still want to exlude it from the cache.
            if (ma->isDylib()) {
                std::string installName = ma->installName();

                // Let the platform exclude the file before we do anything else.
                if (platformExcludesInstallName(installName)) {
                    inputFile.diag.verbose("Platform excluded file\n");
                    fileSystem.unloadFile(loadedFileInfo);
                    continue;
                }

                if (!ma->canBePlacedInDyldCache(inputFile.path, ^(const char* msg) {
                    inputFile.diag.warning("Dylib located at '%s' cannot be placed in cache because: %s", inputFile.path, msg);
                })) {
                    // TODO: Add exclusion lists here?
                    // Probably not as we already applied the dylib exclusion list.
                    if (!ma->canHavePrecomputedDlopenClosure(inputFile.path, ^(const char* msg) {
                        inputFile.diag.verbose("Dylib located at '%s' cannot prebuild dlopen closure in cache because: %s", inputFile.path, msg);
                    }) ) {
                        fileSystem.unloadFile(loadedFileInfo);
                        continue;
                    }
                    otherDylibs.emplace_back((CacheBuilder::LoadedMachO){ mappedFile, loadedFileInfo, &inputFile });
                    continue;
                }

                // Otherwise see if we have another file with this install name
                auto iteratorAndInserted = dylibInstallNameMap.insert(std::make_pair(installName, dylibsToCache.size()));
                if (iteratorAndInserted.second) {
                    // We inserted the dylib so we haven't seen another with this name.
                    if (installName[0] != '@' && installName != inputFile.path) {
                        inputFile.diag.warning("Dylib located at '%s' has installname '%s'", inputFile.path, installName.c_str());
                    }

                    dylibsToCache.emplace_back((CacheBuilder::LoadedMachO){ mappedFile, loadedFileInfo, &inputFile });
                } else {
                    // We didn't insert this one so we've seen it before.
                    CacheBuilder::LoadedMachO& previousLoadedMachO = dylibsToCache[iteratorAndInserted.first->second];
                    inputFile.diag.warning("Multiple dylibs claim installname '%s' ('%s' and '%s')", installName.c_str(), inputFile.path, previousLoadedMachO.mappedFile.runtimePath.c_str());

                    // This is the "Good" one, overwrite
                    if (inputFile.path == installName) {
                        // Unload the old one
                        fileSystem.unloadFile(previousLoadedMachO.loadedFileInfo);

                        // And replace with this one.
                        previousLoadedMachO.mappedFile = mappedFile;
                        previousLoadedMachO.loadedFileInfo = loadedFileInfo;
                    }
                }
            } else if (ma->isBundle()) {
                // TODO: Add exclusion lists here?
                if (!ma->canHavePrecomputedDlopenClosure(inputFile.path, ^(const char* msg) {
                    inputFile.diag.verbose("Dylib located at '%s' cannot prebuild dlopen closure in cache because: %s", inputFile.path, msg);
                }) ) {
                    fileSystem.unloadFile(loadedFileInfo);
                    continue;
                }
                otherDylibs.emplace_back((CacheBuilder::LoadedMachO){ mappedFile, loadedFileInfo, &inputFile });
            } else if (ma->isDynamicExecutable()) {
                if (platformExcludesExecutablePath_macOS(inputFile.path)) {
                    inputFile.diag.verbose("Platform excluded file\n");
                    fileSystem.unloadFile(loadedFileInfo);
                    continue;
                }
                executables.emplace_back((CacheBuilder::LoadedMachO){ mappedFile, loadedFileInfo, &inputFile });
            } else {
                inputFile.diag.verbose("Unsupported mach file type\n");
                fileSystem.unloadFile(loadedFileInfo);
            }
        }
    }

private:



    static bool platformExcludesInstallName_macOS(const std::string& installName) {
        return false;
    }

    static bool platformExcludesInstallName_iOS(const std::string& installName) {
        if ( installName == "/System/Library/Caches/com.apple.xpc/sdk.dylib" )
            return true;
        if ( installName == "/System/Library/Caches/com.apple.xpcd/xpcd_cache.dylib" )
            return true;
        return false;
    }

    static bool platformExcludesInstallName_tvOS(const std::string& installName) {
        return platformExcludesInstallName_iOS(installName);
    }

    static bool platformExcludesInstallName_watchOS(const std::string& installName) {
        return platformExcludesInstallName_iOS(installName);
    }

    static bool platformExcludesInstallName_bridgeOS(const std::string& installName) {
        return platformExcludesInstallName_iOS(installName);
    }

    // Returns true if the current platform requires that this install name be excluded from the shared cache
    // Note that this overrides any exclusion from anywhere else.
    bool platformExcludesInstallName(const std::string& installName) {
        switch (reqPlatform) {
            case dyld3::Platform::unknown:
                return false;
            case dyld3::Platform::macOS:
                return platformExcludesInstallName_macOS(installName);
            case dyld3::Platform::iOS:
                return platformExcludesInstallName_iOS(installName);
            case dyld3::Platform::tvOS:
                return platformExcludesInstallName_tvOS(installName);
            case dyld3::Platform::watchOS:
                return platformExcludesInstallName_watchOS(installName);
            case dyld3::Platform::bridgeOS:
                return platformExcludesInstallName_bridgeOS(installName);
            case dyld3::Platform::iOSMac:
                return false;
            case dyld3::Platform::iOS_simulator:
                return false;
            case dyld3::Platform::tvOS_simulator:
                return false;
            case dyld3::Platform::watchOS_simulator:
                return false;
            case dyld3::Platform::driverKit:
                return false;
        }
    }




    static bool platformExcludesExecutablePath_macOS(const std::string& path) {
        return false;
    }

    static bool platformExcludesExecutablePath_iOS(const std::string& path) {
        //HACK exclude all launchd and installd variants until we can do something about xpcd_cache.dylib and friends
        if (path == "/sbin/launchd"
            || path == "/usr/local/sbin/launchd.debug"
            || path == "/usr/local/sbin/launchd.development"
            || path == "/usr/libexec/installd") {
            return true;
        }
        return false;
    }

    static bool platformExcludesExecutablePath_tvOS(const std::string& path) {
        return platformExcludesExecutablePath_iOS(path);
    }

    static bool platformExcludesExecutablePath_watchOS(const std::string& path) {
        return platformExcludesExecutablePath_iOS(path);
    }

    static bool platformExcludesExecutablePath_bridgeOS(const std::string& path) {
        return platformExcludesExecutablePath_iOS(path);
    }

    // Returns true if the current platform requires that this path be excluded from the shared cache
    // Note that this overrides any exclusion from anywhere else.
    bool platformExcludesExecutablePath(const std::string& path) {
        switch (reqPlatform) {
            case dyld3::Platform::unknown:
                return false;
            case dyld3::Platform::macOS:
                return platformExcludesExecutablePath_macOS(path);
            case dyld3::Platform::iOS:
                return platformExcludesExecutablePath_iOS(path);
            case dyld3::Platform::tvOS:
                return platformExcludesExecutablePath_tvOS(path);
            case dyld3::Platform::watchOS:
                return platformExcludesExecutablePath_watchOS(path);
            case dyld3::Platform::bridgeOS:
                return platformExcludesExecutablePath_bridgeOS(path);
            case dyld3::Platform::iOSMac:
                return false;
            case dyld3::Platform::iOS_simulator:
                return false;
            case dyld3::Platform::tvOS_simulator:
                return false;
            case dyld3::Platform::watchOS_simulator:
                return false;
            case dyld3::Platform::driverKit:
                return false;
        }
    }

    const dyld3::closure::FileSystem&                   fileSystem;
    const dyld3::GradedArchs&                           reqArchs;
    dyld3::Platform                                     reqPlatform;
};

static void verifySelfContained(std::vector<CacheBuilder::LoadedMachO>& dylibsToCache,
                                std::vector<CacheBuilder::LoadedMachO>& otherDylibs,
                                std::vector<CacheBuilder::LoadedMachO>& couldNotLoadFiles)
{
    // build map of dylibs
    __block std::map<std::string, const CacheBuilder::LoadedMachO*> knownDylibs;
    __block std::map<std::string, const CacheBuilder::LoadedMachO*> allDylibs;
    for (const CacheBuilder::LoadedMachO& dylib : dylibsToCache) {
        knownDylibs.insert({ dylib.mappedFile.runtimePath, &dylib });
        allDylibs.insert({ dylib.mappedFile.runtimePath, &dylib });
        if (const char* installName = dylib.mappedFile.mh->installName()) {
            knownDylibs.insert({ installName, &dylib });
            allDylibs.insert({ installName, &dylib });
        }
    }

    for (const CacheBuilder::LoadedMachO& dylib : otherDylibs) {
        allDylibs.insert({ dylib.mappedFile.runtimePath, &dylib });
        if (const char* installName = dylib.mappedFile.mh->installName())
            allDylibs.insert({ installName, &dylib });
    }

    for (const CacheBuilder::LoadedMachO& dylib : couldNotLoadFiles) {
        allDylibs.insert({ dylib.inputFile->path, &dylib });
    }

    // check all dependencies to assure every dylib in cache only depends on other dylibs in cache
    __block std::map<std::string, std::set<std::string>> badDylibs;
    __block bool doAgain = true;
    while ( doAgain ) {
        doAgain = false;
        // scan dylib list making sure all dependents are in dylib list
        for (const CacheBuilder::LoadedMachO& dylib : dylibsToCache) {
            if ( badDylibs.count(dylib.mappedFile.runtimePath) != 0 )
                continue;
            dylib.mappedFile.mh->forEachDependentDylib(^(const char* loadPath, bool isWeak, bool isReExport, bool isUpward, uint32_t compatVersion, uint32_t curVersion, bool& stop) {
                if (isWeak)
                    return;
                if ( knownDylibs.count(loadPath) == 0 ) {
                    badDylibs[dylib.mappedFile.runtimePath].insert(std::string("Could not find dependency '") + loadPath + "'");
                    knownDylibs.erase(dylib.mappedFile.runtimePath);
                    knownDylibs.erase(dylib.mappedFile.mh->installName());
                    doAgain = true;
                }
            });
        }
    }

    // Now walk the dylibs which depend on missing dylibs and see if any of them are required binaries.
    for (auto badDylibsIterator : badDylibs) {
        const std::string& dylibRuntimePath = badDylibsIterator.first;
        auto requiredDylibIterator = allDylibs.find(dylibRuntimePath);
        if (requiredDylibIterator == allDylibs.end())
            continue;
        if (!requiredDylibIterator->second->inputFile->mustBeIncluded())
            continue;
        // This dylib is required so mark all dependencies as requried too
        __block std::vector<const CacheBuilder::LoadedMachO*> worklist;
        worklist.push_back(requiredDylibIterator->second);
        while (!worklist.empty()) {
            const CacheBuilder::LoadedMachO* dylib = worklist.back();
            worklist.pop_back();
            if (!dylib->mappedFile.mh)
                continue;
            dylib->mappedFile.mh->forEachDependentDylib(^(const char* loadPath, bool isWeak, bool isReExport, bool isUpward, uint32_t compatVersion, uint32_t curVersion, bool& stop) {
                if (isWeak)
                    return;
                auto dylibIterator = allDylibs.find(loadPath);
                if (dylibIterator != allDylibs.end()) {
                    if (dylibIterator->second->inputFile->state == CacheBuilder::InputFile::Unset) {
                        dylibIterator->second->inputFile->state = CacheBuilder::InputFile::MustBeIncludedForDependent;
                        worklist.push_back(dylibIterator->second);
                    }
                }
            });
        }
    }

    // FIXME: Make this an option we can pass in
    const bool evictLeafDylibs = true;
    if (evictLeafDylibs) {
        doAgain = true;
        while ( doAgain ) {
            doAgain = false;

            // build count of how many references there are to each dylib
            __block std::set<std::string> referencedDylibs;
            for (const CacheBuilder::LoadedMachO& dylib : dylibsToCache) {
                if ( badDylibs.count(dylib.mappedFile.runtimePath) != 0 )
                    continue;
                dylib.mappedFile.mh->forEachDependentDylib(^(const char* loadPath, bool isWeak, bool isReExport, bool isUpward, uint32_t compatVersion, uint32_t curVersion, bool &stop) {
                    referencedDylibs.insert(loadPath);
                });
            }

            // find all dylibs not referenced
            for (const CacheBuilder::LoadedMachO& dylib : dylibsToCache) {
                if ( badDylibs.count(dylib.mappedFile.runtimePath) != 0 )
                    continue;
                const char* installName = dylib.mappedFile.mh->installName();
                if ( (referencedDylibs.count(installName) == 0) && (dylib.inputFile->state == CacheBuilder::InputFile::MustBeExcludedIfUnused) ) {
                    badDylibs[dylib.mappedFile.runtimePath].insert(std::string("It has been explicitly excluded as it is unused"));
                    doAgain = true;
                }
            }
        }
    }

    // Move bad dylibs from dylibs to cache to other dylibs.
    for (const CacheBuilder::LoadedMachO& dylib : dylibsToCache) {
        auto i = badDylibs.find(dylib.mappedFile.runtimePath);
        if ( i != badDylibs.end()) {
            otherDylibs.push_back(dylib);
            for (const std::string& reason : i->second )
                otherDylibs.back().inputFile->diag.warning("Dylib located at '%s' not placed in shared cache because: %s", dylib.mappedFile.runtimePath.c_str(), reason.c_str());
        }
    }

    const auto& badDylibsLambdaRef = badDylibs;
    dylibsToCache.erase(std::remove_if(dylibsToCache.begin(), dylibsToCache.end(), [&](const CacheBuilder::LoadedMachO& dylib) {
        if (badDylibsLambdaRef.find(dylib.mappedFile.runtimePath) != badDylibsLambdaRef.end())
            return true;
        return false;
    }), dylibsToCache.end());
}

// This is the new build API which takes the raw files (which could be FAT) and tries to build a cache from them.
// We should remove the other build() method, or make it private so that this can wrap it.
void CacheBuilder::build(std::vector<CacheBuilder::InputFile>& inputFiles,
                         std::vector<DyldSharedCache::FileAlias>& aliases) {
    // First filter down to files which are actually MachO's
    CacheInputBuilder cacheInputBuilder(_fileSystem, *_options.archs, _options.platform);

    std::vector<LoadedMachO> dylibsToCache;
    std::vector<LoadedMachO> otherDylibs;
    std::vector<LoadedMachO> executables;
    std::vector<LoadedMachO> couldNotLoadFiles;
    cacheInputBuilder.loadMachOs(inputFiles, dylibsToCache, otherDylibs, executables, couldNotLoadFiles);

    verifySelfContained(dylibsToCache, otherDylibs, couldNotLoadFiles);

    // Check for required binaries before we try to build the cache
    if (!_diagnostics.hasError()) {
        // If we succeeded in building, then now see if there was a missing required file, and if so why its missing.
        std::string errorString;
        for (const LoadedMachO& dylib : otherDylibs) {
            if (dylib.inputFile->mustBeIncluded()) {
                // An error loading a required file must be propagated up to the top level diagnostic handler.
                bool gotWarning = false;
                for (const std::string& warning : dylib.inputFile->diag.warnings()) {
                    gotWarning = true;
                    std::string message = warning;
                    if (message.back() == '\n')
                        message.pop_back();
                    if (!errorString.empty())
                        errorString += "ERROR: ";
                    errorString += "Required binary was not included in the shared cache '" + std::string(dylib.inputFile->path) + "' because: " + message + "\n";
                }
                if (!gotWarning) {
                    if (!errorString.empty())
                        errorString += "ERROR: ";
                    errorString += "Required binary was not included in the shared cache '" + std::string(dylib.inputFile->path) + "' because: 'unknown error.  Please report to dyld'\n";
                }
            }
        }
        for (const LoadedMachO& dylib : couldNotLoadFiles) {
            if (dylib.inputFile->mustBeIncluded()) {
                if (dylib.inputFile->diag.hasError()) {
                    if (!errorString.empty())
                        errorString += "ERROR: ";
                    errorString += "Required binary was not included in the shared cache '" + std::string(dylib.inputFile->path) + "' because: " + dylib.inputFile->diag.errorMessage() + "\n";
                } else {
                    if (!errorString.empty())
                        errorString += "ERROR: ";
                    errorString += "Required binary was not included in the shared cache '" + std::string(dylib.inputFile->path) + "' because: 'unknown error.  Please report to dyld'\n";

                }
            }
        }
        if (!errorString.empty()) {
            _diagnostics.error("%s", errorString.c_str());
        }
    }

    if (!_diagnostics.hasError())
        build(dylibsToCache, otherDylibs, executables, aliases);

    if (!_diagnostics.hasError()) {
        // If we succeeded in building, then now see if there was a missing required file, and if so why its missing.
        std::string errorString;
        for (CacheBuilder::InputFile& inputFile : inputFiles) {
            if (inputFile.mustBeIncluded() && inputFile.diag.hasError()) {
                // An error loading a required file must be propagated up to the top level diagnostic handler.
                std::string message = inputFile.diag.errorMessage();
                if (message.back() == '\n')
                    message.pop_back();
                errorString += "Required binary was not included in the shared cache '" + std::string(inputFile.path) + "' because: " + message + "\n";
            }
        }
        if (!errorString.empty()) {
            _diagnostics.error("%s", errorString.c_str());
        }
    }

    // Add all the warnings from the input files to the top level warnings on the main diagnostics object.
    for (CacheBuilder::InputFile& inputFile : inputFiles) {
        for (const std::string& warning : inputFile.diag.warnings())
            _diagnostics.warning("%s", warning.c_str());
    }

    // Clean up the loaded files
    for (LoadedMachO& loadedMachO : dylibsToCache)
        _fileSystem.unloadFile(loadedMachO.loadedFileInfo);
    for (LoadedMachO& loadedMachO : otherDylibs)
        _fileSystem.unloadFile(loadedMachO.loadedFileInfo);
    for (LoadedMachO& loadedMachO : executables)
        _fileSystem.unloadFile(loadedMachO.loadedFileInfo);
}

void CacheBuilder::build(const std::vector<DyldSharedCache::MappedMachO>& dylibs,
                         const std::vector<DyldSharedCache::MappedMachO>& otherOsDylibsInput,
                         const std::vector<DyldSharedCache::MappedMachO>& osExecutables,
                         std::vector<DyldSharedCache::FileAlias>& aliases) {

    std::vector<LoadedMachO> dylibsToCache;
    std::vector<LoadedMachO> otherDylibs;
    std::vector<LoadedMachO> executables;

    for (const DyldSharedCache::MappedMachO& mappedMachO : dylibs) {
        dyld3::closure::LoadedFileInfo loadedFileInfo;
        loadedFileInfo.fileContent      = mappedMachO.mh;
        loadedFileInfo.fileContentLen   = mappedMachO.length;
        loadedFileInfo.sliceOffset      = mappedMachO.sliceFileOffset;
        loadedFileInfo.sliceLen         = mappedMachO.length;
        loadedFileInfo.inode            = mappedMachO.inode;
        loadedFileInfo.mtime            = mappedMachO.modTime;
        loadedFileInfo.path             = mappedMachO.runtimePath.c_str();
        dylibsToCache.emplace_back((LoadedMachO){ mappedMachO, loadedFileInfo, nullptr });
    }

    for (const DyldSharedCache::MappedMachO& mappedMachO : otherOsDylibsInput) {
        dyld3::closure::LoadedFileInfo loadedFileInfo;
        loadedFileInfo.fileContent      = mappedMachO.mh;
        loadedFileInfo.fileContentLen   = mappedMachO.length;
        loadedFileInfo.sliceOffset      = mappedMachO.sliceFileOffset;
        loadedFileInfo.sliceLen         = mappedMachO.length;
        loadedFileInfo.inode            = mappedMachO.inode;
        loadedFileInfo.mtime            = mappedMachO.modTime;
        loadedFileInfo.path             = mappedMachO.runtimePath.c_str();
        otherDylibs.emplace_back((LoadedMachO){ mappedMachO, loadedFileInfo, nullptr });
    }

    for (const DyldSharedCache::MappedMachO& mappedMachO : osExecutables) {
        dyld3::closure::LoadedFileInfo loadedFileInfo;
        loadedFileInfo.fileContent      = mappedMachO.mh;
        loadedFileInfo.fileContentLen   = mappedMachO.length;
        loadedFileInfo.sliceOffset      = mappedMachO.sliceFileOffset;
        loadedFileInfo.sliceLen         = mappedMachO.length;
        loadedFileInfo.inode            = mappedMachO.inode;
        loadedFileInfo.mtime            = mappedMachO.modTime;
        loadedFileInfo.path             = mappedMachO.runtimePath.c_str();
        executables.emplace_back((LoadedMachO){ mappedMachO, loadedFileInfo, nullptr });
    }

    build(dylibsToCache, otherDylibs, executables, aliases);
}

void CacheBuilder::build(const std::vector<LoadedMachO>& dylibs,
                         const std::vector<LoadedMachO>& otherOsDylibsInput,
                         const std::vector<LoadedMachO>& osExecutables,
                         std::vector<DyldSharedCache::FileAlias>& aliases)
{
    // <rdar://problem/21317611> error out instead of crash if cache has no dylibs
    // FIXME: plist should specify required vs optional dylibs
    if ( dylibs.size() < 30 ) {
        _diagnostics.error("missing required minimum set of dylibs");
        return;
    }
    uint64_t t1 = mach_absolute_time();

    // make copy of dylib list and sort
    makeSortedDylibs(dylibs, _options.dylibOrdering);

    // allocate space used by largest possible cache plus room for LINKEDITS before optimization
    _allocatedBufferSize = _archLayout->sharedMemorySize * 1.50;
    if ( vm_allocate(mach_task_self(), &_fullAllocatedBuffer, _allocatedBufferSize, VM_FLAGS_ANYWHERE) != 0 ) {
        _diagnostics.error("could not allocate buffer");
        return;
    }

    // assign addresses for each segment of each dylib in new cache
    parseCoalescableSegments();
    processSelectorStrings(osExecutables);
    assignSegmentAddresses();
    std::vector<const LoadedMachO*> overflowDylibs;
    while ( cacheOverflowAmount() != 0 ) {
        if ( !_options.evictLeafDylibsOnOverflow ) {
            _diagnostics.error("cache overflow by %lluMB", cacheOverflowAmount() / 1024 / 1024);
            return;
        }
        size_t evictionCount = evictLeafDylibs(cacheOverflowAmount(), overflowDylibs);
        // re-layout cache
        for (DylibInfo& dylib : _sortedDylibs)
            dylib.cacheLocation.clear();
        _coalescedText.clear();
        parseCoalescableSegments();
        processSelectorStrings(osExecutables);
        assignSegmentAddresses();

        _diagnostics.verbose("cache overflow, evicted %lu leaf dylibs\n", evictionCount);
    }
    markPaddingInaccessible();

     // copy all segments into cache
    uint64_t t2 = mach_absolute_time();
    writeCacheHeader();
    copyRawSegments();

    // rebase all dylibs for new location in cache
    uint64_t t3 = mach_absolute_time();
    _aslrTracker.setDataRegion(_readWriteRegion.buffer, _readWriteRegion.sizeInUse);
    if ( !_options.cacheSupportsASLR )
        _aslrTracker.disable();
    adjustAllImagesForNewSegmentLocations();
    if ( _diagnostics.hasError() )
        return;

    // build ImageArray for dyld3, which has side effect of binding all cached dylibs
    uint64_t t4 = mach_absolute_time();
    buildImageArray(aliases);
    if ( _diagnostics.hasError() )
        return;

    // optimize ObjC
    uint64_t t5 = mach_absolute_time();
    DyldSharedCache* dyldCache = (DyldSharedCache*)_readExecuteRegion.buffer;
    if ( _options.optimizeObjC )
        optimizeObjC();
    if ( _diagnostics.hasError() )
        return;


    // optimize away stubs
    uint64_t t6 = mach_absolute_time();
    if ( _options.optimizeStubs )
         optimizeAwayStubs();


    // FIPS seal corecrypto, This must be done after stub elimination (so that __TEXT,__text is not changed after sealing)
    fipsSign();

    // merge and compact LINKEDIT segments
    uint64_t t7 = mach_absolute_time();
    optimizeLinkedit();

    // copy ImageArray to end of read-only region
    addImageArray();
    if ( _diagnostics.hasError() )
        return;
    uint64_t t8 = mach_absolute_time();

    // don't add dyld3 closures to simulator cache
    if ( !dyld3::MachOFile::isSimulatorPlatform(_options.platform) ) {
        // compute and add dlopen closures for all other dylibs
        addOtherImageArray(otherOsDylibsInput, overflowDylibs);
        if ( _diagnostics.hasError() )
            return;

        // compute and add launch closures to end of read-only region
        addClosures(osExecutables);
        if ( _diagnostics.hasError() )
            return;
    }

    // update final readOnly region size
    dyld_cache_mapping_info* mappings = (dyld_cache_mapping_info*)(_readExecuteRegion.buffer + dyldCache->header.mappingOffset);
    mappings[2].size = _readOnlyRegion.sizeInUse;
    if ( _options.excludeLocalSymbols )
        dyldCache->header.localSymbolsOffset = _readOnlyRegion.cacheFileOffset + _readOnlyRegion.sizeInUse;

    // record max slide now that final size is established
    if ( _archLayout->sharedRegionsAreDiscontiguous ) {
        // special case x86_64 which has three non-contiguous chunks each in their own 1GB regions
        uint64_t maxSlide0 = 0x60000000 - _readExecuteRegion.sizeInUse; // TEXT region has 1.5GB region
        uint64_t maxSlide1 = 0x40000000 - _readWriteRegion.sizeInUse;
        uint64_t maxSlide2 = 0x3FE00000 - _readOnlyRegion.sizeInUse;
        dyldCache->header.maxSlide = std::min(std::min(maxSlide0, maxSlide1), maxSlide2);
    }
    else {
        // <rdar://problem/49852839> branch predictor on arm64 currently only looks at low 32-bits, so don't slide cache more than 2GB
        if ( (_archLayout->sharedMemorySize == 0x100000000) && (_readExecuteRegion.sizeInUse < 0x80000000) )
            dyldCache->header.maxSlide = 0x80000000 - _readExecuteRegion.sizeInUse;
        else
            dyldCache->header.maxSlide = (_archLayout->sharedMemoryStart + _archLayout->sharedMemorySize) - (_readOnlyRegion.unslidLoadAddress + _readOnlyRegion.sizeInUse);
    }

    uint64_t t9 = mach_absolute_time();
    
    // fill in slide info at start of region[2]
    // do this last because it modifies pointers in DATA segments
    if ( _options.cacheSupportsASLR ) {
#if SUPPORT_ARCH_arm64e
        if ( strcmp(_archLayout->archName, "arm64e") == 0 )
            writeSlideInfoV3(_aslrTracker.bitmap(), _aslrTracker.dataPageCount());
        else
#endif
        if ( _archLayout->is64 )
            writeSlideInfoV2<Pointer64<LittleEndian>>(_aslrTracker.bitmap(), _aslrTracker.dataPageCount());
#if SUPPORT_ARCH_arm64_32 || SUPPORT_ARCH_armv7k
        else if ( _archLayout->pointerDeltaMask == 0xC0000000 )
            writeSlideInfoV4<Pointer32<LittleEndian>>(_aslrTracker.bitmap(), _aslrTracker.dataPageCount());
#endif
        else
            writeSlideInfoV2<Pointer32<LittleEndian>>(_aslrTracker.bitmap(), _aslrTracker.dataPageCount());
    }

    uint64_t t10 = mach_absolute_time();

    // last sanity check on size
    if ( cacheOverflowAmount() != 0 ) {
        _diagnostics.error("cache overflow after optimizations 0x%llX -> 0x%llX", _readExecuteRegion.unslidLoadAddress, _readOnlyRegion.unslidLoadAddress + _readOnlyRegion.sizeInUse);
        return;
    }

    // codesignature is part of file, but is not mapped
    codeSign();
    if ( _diagnostics.hasError() )
        return;

    uint64_t t11 = mach_absolute_time();

    if ( _options.verbose ) {
        fprintf(stderr, "time to layout cache: %ums\n", absolutetime_to_milliseconds(t2-t1));
        fprintf(stderr, "time to copy cached dylibs into buffer: %ums\n", absolutetime_to_milliseconds(t3-t2));
        fprintf(stderr, "time to adjust segments for new split locations: %ums\n", absolutetime_to_milliseconds(t4-t3));
        fprintf(stderr, "time to bind all images: %ums\n", absolutetime_to_milliseconds(t5-t4));
        fprintf(stderr, "time to optimize Objective-C: %ums\n", absolutetime_to_milliseconds(t6-t5));
        fprintf(stderr, "time to do stub elimination: %ums\n", absolutetime_to_milliseconds(t7-t6));
        fprintf(stderr, "time to optimize LINKEDITs: %ums\n", absolutetime_to_milliseconds(t8-t7));
        fprintf(stderr, "time to build %lu closures: %ums\n", osExecutables.size(), absolutetime_to_milliseconds(t9-t8));
        fprintf(stderr, "time to compute slide info: %ums\n", absolutetime_to_milliseconds(t10-t9));
        fprintf(stderr, "time to compute UUID and codesign cache file: %ums\n", absolutetime_to_milliseconds(t11-t10));
    }

    return;
}


void CacheBuilder::writeCacheHeader()
{
    // "dyld_v1" + spaces + archName(), with enough spaces to pad to 15 bytes
    std::string magic = "dyld_v1";
    magic.append(15 - magic.length() - strlen(_options.archs->name()), ' ');
    magic.append(_options.archs->name());
    assert(magic.length() == 15);

    // fill in header
    dyld_cache_header* dyldCacheHeader = (dyld_cache_header*)_readExecuteRegion.buffer;
    memcpy(dyldCacheHeader->magic, magic.c_str(), 16);
    dyldCacheHeader->mappingOffset        = sizeof(dyld_cache_header);
    dyldCacheHeader->mappingCount         = 3;
    dyldCacheHeader->imagesOffset         = (uint32_t)(dyldCacheHeader->mappingOffset + 3*sizeof(dyld_cache_mapping_info));
    dyldCacheHeader->imagesCount          = (uint32_t)_sortedDylibs.size() + _aliasCount;
    dyldCacheHeader->dyldBaseAddress      = 0;
    dyldCacheHeader->codeSignatureOffset  = 0;
    dyldCacheHeader->codeSignatureSize    = 0;
    dyldCacheHeader->slideInfoOffset      = _slideInfoFileOffset;
    dyldCacheHeader->slideInfoSize        = _slideInfoBufferSizeAllocated;
    dyldCacheHeader->localSymbolsOffset   = 0;
    dyldCacheHeader->localSymbolsSize     = 0;
    dyldCacheHeader->cacheType            = _options.optimizeStubs ? kDyldSharedCacheTypeProduction : kDyldSharedCacheTypeDevelopment;
    dyldCacheHeader->accelerateInfoAddr   = 0;
    dyldCacheHeader->accelerateInfoSize   = 0;
    bzero(dyldCacheHeader->uuid, 16);// overwritten later by recomputeCacheUUID()
    dyldCacheHeader->branchPoolsOffset    = 0;
    dyldCacheHeader->branchPoolsCount     = 0;
    dyldCacheHeader->imagesTextOffset     = dyldCacheHeader->imagesOffset + sizeof(dyld_cache_image_info)*dyldCacheHeader->imagesCount;
    dyldCacheHeader->imagesTextCount      = _sortedDylibs.size();
    dyldCacheHeader->patchInfoAddr        = 0;
    dyldCacheHeader->patchInfoSize        = 0;
    dyldCacheHeader->otherImageGroupAddrUnused  = 0;
    dyldCacheHeader->otherImageGroupSizeUnused  = 0;
    dyldCacheHeader->progClosuresAddr     = 0;
    dyldCacheHeader->progClosuresSize     = 0;
    dyldCacheHeader->progClosuresTrieAddr = 0;
    dyldCacheHeader->progClosuresTrieSize = 0;
    dyldCacheHeader->platform             = (uint8_t)_options.platform;
    dyldCacheHeader->formatVersion        = dyld3::closure::kFormatVersion;
    dyldCacheHeader->dylibsExpectedOnDisk = !_options.dylibsRemovedDuringMastering;
    dyldCacheHeader->simulator            = _options.forSimulator;
    dyldCacheHeader->locallyBuiltCache    = _options.isLocallyBuiltCache;
    dyldCacheHeader->builtFromChainedFixups= (strcmp(_options.archs->name(), "arm64e") == 0); // FIXME
    dyldCacheHeader->formatVersion        = dyld3::closure::kFormatVersion;
    dyldCacheHeader->sharedRegionStart    = _archLayout->sharedMemoryStart;
    dyldCacheHeader->sharedRegionSize     = _archLayout->sharedMemorySize;

   // fill in mappings
    dyld_cache_mapping_info* mappings = (dyld_cache_mapping_info*)(_readExecuteRegion.buffer + dyldCacheHeader->mappingOffset);
    mappings[0].address    = _readExecuteRegion.unslidLoadAddress;
    mappings[0].fileOffset = 0;
    mappings[0].size       = _readExecuteRegion.sizeInUse;
    mappings[0].maxProt    = VM_PROT_READ | VM_PROT_EXECUTE;
    mappings[0].initProt   = VM_PROT_READ | VM_PROT_EXECUTE;
    mappings[1].address    = _readWriteRegion.unslidLoadAddress;
    mappings[1].fileOffset = _readExecuteRegion.sizeInUse;
    mappings[1].size       = _readWriteRegion.sizeInUse;
    mappings[1].maxProt    = VM_PROT_READ | VM_PROT_WRITE;
    mappings[1].initProt   = VM_PROT_READ | VM_PROT_WRITE;
    mappings[2].address    = _readOnlyRegion.unslidLoadAddress;
    mappings[2].fileOffset = _readExecuteRegion.sizeInUse + _readWriteRegion.sizeInUse;
    mappings[2].size       = _readOnlyRegion.sizeInUse;
    mappings[2].maxProt    = VM_PROT_READ;
    mappings[2].initProt   = VM_PROT_READ;

    // fill in image table
    dyld_cache_image_info* images = (dyld_cache_image_info*)(_readExecuteRegion.buffer + dyldCacheHeader->imagesOffset);
    for (const DylibInfo& dylib : _sortedDylibs) {
        const char* installName = dylib.input->mappedFile.mh->installName();
        images->address = dylib.cacheLocation[0].dstCacheUnslidAddress;
        if ( _options.dylibsRemovedDuringMastering ) {
            images->modTime = 0;
            images->inode   = pathHash(installName);
        }
        else {
            images->modTime = dylib.input->mappedFile.modTime;
            images->inode   = dylib.input->mappedFile.inode;
        }
        uint32_t installNameOffsetInTEXT =  (uint32_t)(installName - (char*)dylib.input->mappedFile.mh);
        images->pathFileOffset = (uint32_t)dylib.cacheLocation[0].dstCacheFileOffset + installNameOffsetInTEXT;
        ++images;
    }
    // append aliases image records and strings
/*
    for (auto &dylib : _dylibs) {
        if (!dylib->installNameAliases.empty()) {
            for (const std::string& alias : dylib->installNameAliases) {
                images->set_address(_segmentMap[dylib][0].address);
                if (_manifest.platform() == "osx") {
                    images->modTime = dylib->lastModTime;
                    images->inode = dylib->inode;
                }
                else {
                    images->modTime = 0;
                    images->inode = pathHash(alias.c_str());
                }
                images->pathFileOffset = offset;
                //fprintf(stderr, "adding alias %s for %s\n", alias.c_str(), dylib->installName.c_str());
                ::strcpy((char*)&_buffer[offset], alias.c_str());
                offset += alias.size() + 1;
                ++images;
            }
        }
    }
*/
    // calculate start of text image array and trailing string pool
    dyld_cache_image_text_info* textImages = (dyld_cache_image_text_info*)(_readExecuteRegion.buffer + dyldCacheHeader->imagesTextOffset);
    uint32_t stringOffset = (uint32_t)(dyldCacheHeader->imagesTextOffset + sizeof(dyld_cache_image_text_info) * _sortedDylibs.size());

    // write text image array and image names pool at same time
    for (const DylibInfo& dylib : _sortedDylibs) {
        dylib.input->mappedFile.mh->getUuid(textImages->uuid);
        textImages->loadAddress     = dylib.cacheLocation[0].dstCacheUnslidAddress;
        textImages->textSegmentSize = (uint32_t)dylib.cacheLocation[0].dstCacheSegmentSize;
        textImages->pathOffset      = stringOffset;
        const char* installName = dylib.input->mappedFile.mh->installName();
        ::strcpy((char*)_readExecuteRegion.buffer + stringOffset, installName);
        stringOffset += (uint32_t)strlen(installName)+1;
        ++textImages;
    }

    // make sure header did not overflow into first mapped image
    const dyld_cache_image_info* firstImage = (dyld_cache_image_info*)(_readExecuteRegion.buffer + dyldCacheHeader->imagesOffset);
    assert(stringOffset <= (firstImage->address - mappings[0].address));
}

void CacheBuilder::copyRawSegments()
{
    const bool log = false;
    dispatch_apply(_sortedDylibs.size(), DISPATCH_APPLY_AUTO, ^(size_t index) {
        const DylibInfo& dylib = _sortedDylibs[index];
        for (const SegmentMappingInfo& info : dylib.cacheLocation) {
            if (log) fprintf(stderr, "copy %s segment %s (0x%08X bytes) from %p to %p (logical addr 0x%llX) for %s\n",
                             _options.archs->name(), info.segName, info.copySegmentSize, info.srcSegment, info.dstSegment, info.dstCacheUnslidAddress, dylib.input->mappedFile.runtimePath.c_str());
            ::memcpy(info.dstSegment, info.srcSegment, info.copySegmentSize);
        }
    });

    // Copy the coalesced sections
    const uint64_t numCoalescedSections = sizeof(CacheCoalescedText::SupportedSections) / sizeof(*CacheCoalescedText::SupportedSections);
    dispatch_apply(numCoalescedSections, DISPATCH_APPLY_AUTO, ^(size_t index) {
        const CacheCoalescedText::StringSection& cacheStringSection = _coalescedText.getSectionData(CacheCoalescedText::SupportedSections[index]);
        if (log) fprintf(stderr, "copy %s __TEXT_COAL section %s (0x%08X bytes) to %p (logical addr 0x%llX)\n",
                         _options.archs->name(), CacheCoalescedText::SupportedSections[index],
                         cacheStringSection.bufferSize, cacheStringSection.bufferAddr, cacheStringSection.bufferVMAddr);
        for (const auto& stringAndOffset : cacheStringSection.stringsToOffsets)
            ::memcpy(cacheStringSection.bufferAddr + stringAndOffset.second, stringAndOffset.first.data(), stringAndOffset.first.size() + 1);
    });
}

void CacheBuilder::adjustAllImagesForNewSegmentLocations()
{
    __block std::vector<Diagnostics> diags;
    diags.resize(_sortedDylibs.size());

    if (_options.platform == dyld3::Platform::macOS) {
        dispatch_apply(_sortedDylibs.size(), DISPATCH_APPLY_AUTO, ^(size_t index) {
            const DylibInfo& dylib = _sortedDylibs[index];
            adjustDylibSegments(dylib, diags[index]);
        });
    } else {
        // Note this has to be done in serial because the LOH Tracker isn't thread safe
        for (size_t index = 0; index != _sortedDylibs.size(); ++index) {
            const DylibInfo& dylib = _sortedDylibs[index];
            adjustDylibSegments(dylib, diags[index]);
        }
    }

    for (const Diagnostics& diag : diags) {
        if ( diag.hasError() ) {
            _diagnostics.error("%s", diag.errorMessage().c_str());
            break;
        }
    }
}

void CacheBuilder::processSelectorStrings(const std::vector<LoadedMachO>& executables) {
    const bool log = false;

    _selectorStringsFromExecutables = 0;

    // Don't do this optimisation on watchOS where the shared cache is too small
    if (_options.platform == dyld3::Platform::watchOS)
        return;

    // Get the method name coalesced section as that is where we need to put these strings
    CacheBuilder::CacheCoalescedText::StringSection& cacheStringSection = _coalescedText.getSectionData("__objc_methname");
    for (const LoadedMachO& executable : executables) {
        const dyld3::MachOAnalyzer* ma = (const dyld3::MachOAnalyzer*)executable.loadedFileInfo.fileContent;

        uint64_t sizeBeforeProcessing = cacheStringSection.bufferSize;

        ma->forEachObjCMethodName(^(const char *methodName) {
            std::string_view str = methodName;
            auto itAndInserted = cacheStringSection.stringsToOffsets.insert({ str, cacheStringSection.bufferSize });
            if (itAndInserted.second) {
                // If we inserted the string then we need to include it in the total
                cacheStringSection.bufferSize += str.size() + 1;
                // if (log) printf("Selector: %s -> %s\n", ma->installName(), methodName);
                ++_selectorStringsFromExecutables;
            }
        });

        uint64_t sizeAfterProcessing = cacheStringSection.bufferSize;
        if ( log && (sizeBeforeProcessing != sizeAfterProcessing) ) {
            printf("Pulled in % 6lld bytes of selectors from %s\n",
                   sizeAfterProcessing - sizeBeforeProcessing, executable.loadedFileInfo.path);
        }
    }

    _diagnostics.verbose("Pulled in %lld selector strings from executables\n", _selectorStringsFromExecutables);
}

void CacheBuilder::parseCoalescableSegments() {
    const bool log = false;

    for (DylibInfo& dylib : _sortedDylibs)
        _coalescedText.parseCoalescableText(dylib.input->mappedFile.mh, dylib.textCoalescer);

    if (log) {
        for (const char* section : CacheCoalescedText::SupportedSections) {
            CacheCoalescedText::StringSection& sectionData = _coalescedText.getSectionData(section);
            printf("Coalesced %s from % 10lld -> % 10d, saving % 10lld bytes\n", section,
                   sectionData.bufferSize + sectionData.savedSpace, sectionData.bufferSize, sectionData.savedSpace);
        }
    }
}

void CacheBuilder::assignSegmentAddresses()
{
    // calculate size of header info and where first dylib's mach_header should start
    size_t startOffset = sizeof(dyld_cache_header) + 3*sizeof(dyld_cache_mapping_info);
    startOffset += sizeof(dyld_cache_image_info) * _sortedDylibs.size();
    startOffset += sizeof(dyld_cache_image_text_info) * _sortedDylibs.size();
    for (const DylibInfo& dylib : _sortedDylibs) {
        startOffset += (strlen(dylib.input->mappedFile.mh->installName()) + 1);
    }
    //fprintf(stderr, "%s total header size = 0x%08lX\n", _options.archName.c_str(), startOffset);
    startOffset = align(startOffset, 12);

    // assign TEXT segment addresses
    _readExecuteRegion.buffer               = (uint8_t*)_fullAllocatedBuffer;
    _readExecuteRegion.bufferSize           = 0;
    _readExecuteRegion.sizeInUse            = 0;
    _readExecuteRegion.unslidLoadAddress    = _archLayout->sharedMemoryStart;
    _readExecuteRegion.cacheFileOffset      = 0;
    __block uint64_t addr = _readExecuteRegion.unslidLoadAddress + startOffset; // header
    for (DylibInfo& dylib : _sortedDylibs) {
        __block uint64_t textSegVmAddr = 0;
        dylib.input->mappedFile.mh->forEachSegment(^(const dyld3::MachOFile::SegmentInfo& segInfo, bool& stop) {
            if ( strcmp(segInfo.segName, "__TEXT") == 0 )
                textSegVmAddr = segInfo.vmAddr;
            if ( segInfo.protections != (VM_PROT_READ | VM_PROT_EXECUTE) )
                return;
            // We may have coalesced the sections at the end of this segment.  In that case, shrink the segment to remove them.
            __block size_t sizeOfSections = 0;
            __block bool foundCoalescedSection = false;
            dylib.input->mappedFile.mh->forEachSection(^(const dyld3::MachOAnalyzer::SectionInfo &sectInfo, bool malformedSectionRange, bool &stopSection) {
                if (strcmp(sectInfo.segInfo.segName, segInfo.segName) != 0)
                    return;
                if ( dylib.textCoalescer.sectionWasCoalesced(sectInfo.sectName)) {
                    foundCoalescedSection = true;
                } else {
                    sizeOfSections = sectInfo.sectAddr + sectInfo.sectSize - segInfo.vmAddr;
                }
            });
            if (!foundCoalescedSection)
                sizeOfSections = segInfo.sizeOfSections;

            // Keep __TEXT segments 4K or more aligned
            addr = align(addr, std::max((int)segInfo.p2align, (int)12));
            uint64_t offsetInRegion = addr - _readExecuteRegion.unslidLoadAddress;
            SegmentMappingInfo loc;
            loc.srcSegment             = (uint8_t*)dylib.input->mappedFile.mh + segInfo.vmAddr - textSegVmAddr;
            loc.segName                = segInfo.segName;
            loc.dstSegment             = _readExecuteRegion.buffer + offsetInRegion;
            loc.dstCacheUnslidAddress  = addr;
            loc.dstCacheFileOffset     = (uint32_t)offsetInRegion;
            loc.dstCacheSegmentSize    = (uint32_t)align(sizeOfSections, 12);
            loc.dstCacheFileSize       = (uint32_t)align(sizeOfSections, 12);
            loc.copySegmentSize        = (uint32_t)sizeOfSections;
            loc.srcSegmentIndex        = segInfo.segIndex;
            dylib.cacheLocation.push_back(loc);
            addr += loc.dstCacheSegmentSize;
        });
    }
    // move read-only segments to end of TEXT
    if ( _archLayout->textAndDataMaxSize != 0 ) {
       for (DylibInfo& dylib : _sortedDylibs) {
            __block uint64_t textSegVmAddr = 0;
            dylib.input->mappedFile.mh->forEachSegment(^(const dyld3::MachOFile::SegmentInfo& segInfo, bool& stop) {
                if ( strcmp(segInfo.segName, "__TEXT") == 0 )
                    textSegVmAddr = segInfo.vmAddr;
                if ( segInfo.protections != VM_PROT_READ )
                    return;
                if ( strcmp(segInfo.segName, "__LINKEDIT") == 0 )
                    return;

                // Keep segments segments 4K or more aligned
                addr = align(addr, std::max((int)segInfo.p2align, (int)12));
                uint64_t offsetInRegion = addr - _readExecuteRegion.unslidLoadAddress;
                SegmentMappingInfo loc;
                loc.srcSegment             = (uint8_t*)dylib.input->mappedFile.mh + segInfo.vmAddr - textSegVmAddr;
                loc.segName                = segInfo.segName;
                loc.dstSegment             = _readExecuteRegion.buffer + offsetInRegion;
                loc.dstCacheUnslidAddress  = addr;
                loc.dstCacheFileOffset     = (uint32_t)(_readExecuteRegion.cacheFileOffset + offsetInRegion);
                loc.dstCacheSegmentSize    = (uint32_t)align(segInfo.sizeOfSections, 12);
                loc.dstCacheFileSize       = (uint32_t)segInfo.sizeOfSections;
                loc.copySegmentSize        = (uint32_t)segInfo.sizeOfSections;
                loc.srcSegmentIndex        = segInfo.segIndex;
                dylib.cacheLocation.push_back(loc);
                addr += loc.dstCacheSegmentSize;
            });
        }
    }

    // reserve space for objc optimization tables and deduped strings
    uint64_t objcReadOnlyBufferVMAddr = addr;
    _objcReadOnlyBuffer = _readExecuteRegion.buffer + (addr - _readExecuteRegion.unslidLoadAddress);

    // First the strings as we'll fill in the objc tables later in the optimizer
    for (const char* section: CacheCoalescedText::SupportedSections) {
        CacheCoalescedText::StringSection& cacheStringSection = _coalescedText.getSectionData(section);
        cacheStringSection.bufferAddr = _readExecuteRegion.buffer + (addr - _readExecuteRegion.unslidLoadAddress);
        cacheStringSection.bufferVMAddr = addr;
        addr += cacheStringSection.bufferSize;
    }

    addr = align(addr, 14);
    _objcReadOnlyBufferSizeUsed = addr - objcReadOnlyBufferVMAddr;

    uint32_t totalSelectorRefCount = (uint32_t)_selectorStringsFromExecutables;
    uint32_t totalClassDefCount    = 0;
    uint32_t totalProtocolDefCount = 0;
    for (DylibInfo& dylib : _sortedDylibs) {
        dyld3::MachOAnalyzer::ObjCInfo info = dylib.input->mappedFile.mh->getObjCInfo();
        totalSelectorRefCount   += info.selRefCount;
        totalClassDefCount      += info.classDefCount;
        totalProtocolDefCount   += info.protocolDefCount;
    }

    // now that shared cache coalesces all selector strings, use that better count
    uint32_t coalescedSelectorCount = (uint32_t)_coalescedText.objcMethNames.stringsToOffsets.size();
    if ( coalescedSelectorCount > totalSelectorRefCount )
        totalSelectorRefCount = coalescedSelectorCount;
    addr += align(computeReadOnlyObjC(totalSelectorRefCount, totalClassDefCount, totalProtocolDefCount), 14);
    _objcReadOnlyBufferSizeAllocated = addr - objcReadOnlyBufferVMAddr;


    // align TEXT region end
    uint64_t endTextAddress = align(addr, _archLayout->sharedRegionAlignP2);
    _readExecuteRegion.bufferSize = endTextAddress - _readExecuteRegion.unslidLoadAddress;
    _readExecuteRegion.sizeInUse  = _readExecuteRegion.bufferSize;


    // assign __DATA* addresses
    if ( _archLayout->sharedRegionsAreDiscontiguous )
        addr = _archLayout->sharedMemoryStart + 0x60000000;
    else
        addr = align((addr + _archLayout->sharedRegionPadding), _archLayout->sharedRegionAlignP2);
    _readWriteRegion.buffer               = (uint8_t*)_fullAllocatedBuffer + addr - _archLayout->sharedMemoryStart;
    _readWriteRegion.bufferSize           = 0;
    _readWriteRegion.sizeInUse            = 0;
    _readWriteRegion.unslidLoadAddress    = addr;
    _readWriteRegion.cacheFileOffset      = _readExecuteRegion.sizeInUse;

    // layout all __DATA_CONST segments
    __block int dataConstSegmentCount = 0;
    for (DylibInfo& dylib : _sortedDylibs) {
        __block uint64_t textSegVmAddr = 0;
       dylib.input->mappedFile.mh->forEachSegment(^(const dyld3::MachOFile::SegmentInfo& segInfo, bool& stop) {
           if ( _options.platform == dyld3::Platform::watchOS_simulator)
               return;
            if ( strcmp(segInfo.segName, "__TEXT") == 0 )
                textSegVmAddr = segInfo.vmAddr;
            if ( segInfo.protections != (VM_PROT_READ | VM_PROT_WRITE) )
                return;
            if ( strcmp(segInfo.segName, "__DATA_CONST") != 0 )
                return;
            ++dataConstSegmentCount;
            // Pack __DATA_CONST segments
            addr = align(addr, segInfo.p2align);
            size_t copySize = std::min((size_t)segInfo.fileSize, (size_t)segInfo.sizeOfSections);
            uint64_t offsetInRegion = addr - _readWriteRegion.unslidLoadAddress;
            SegmentMappingInfo loc;
            loc.srcSegment             = (uint8_t*)dylib.input->mappedFile.mh + segInfo.vmAddr - textSegVmAddr;
            loc.segName                = segInfo.segName;
            loc.dstSegment             = _readWriteRegion.buffer + offsetInRegion;
            loc.dstCacheUnslidAddress  = addr;
            loc.dstCacheFileOffset     = (uint32_t)(_readWriteRegion.cacheFileOffset + offsetInRegion);
            loc.dstCacheSegmentSize    = (uint32_t)segInfo.sizeOfSections;
            loc.dstCacheFileSize       = (uint32_t)copySize;
            loc.copySegmentSize        = (uint32_t)copySize;
            loc.srcSegmentIndex        = segInfo.segIndex;
            dylib.cacheLocation.push_back(loc);
            addr += loc.dstCacheSegmentSize;
        });
    }

    // layout all __DATA segments (and other r/w non-dirty, non-const) segments
    for (DylibInfo& dylib : _sortedDylibs) {
        __block uint64_t textSegVmAddr = 0;
       dylib.input->mappedFile.mh->forEachSegment(^(const dyld3::MachOFile::SegmentInfo& segInfo, bool& stop) {
            if ( strcmp(segInfo.segName, "__TEXT") == 0 )
                textSegVmAddr = segInfo.vmAddr;
            if ( segInfo.protections != (VM_PROT_READ | VM_PROT_WRITE) )
                return;
            if ( _options.platform != dyld3::Platform::watchOS_simulator) {
                if ( strcmp(segInfo.segName, "__DATA_CONST") == 0 )
                    return;
                if ( strcmp(segInfo.segName, "__DATA_DIRTY") == 0 )
                    return;
            }
            bool forcePageAlignedData = false;
            if (_options.platform == dyld3::Platform::macOS) {
                forcePageAlignedData = dylib.input->mappedFile.mh->hasUnalignedPointerFixups();
                //if ( forcePageAlignedData )
                //    warning("unaligned pointer in %s\n", dylib.input->mappedFile.runtimePath.c_str());
            }
            if ( (dataConstSegmentCount > 10) && !forcePageAlignedData ) {
                // Pack __DATA segments only if we also have __DATA_CONST segments
                addr = align(addr, segInfo.p2align);
            }
            else {
                // Keep __DATA segments 4K or more aligned
                addr = align(addr, std::max((int)segInfo.p2align, (int)12));
            }
            size_t copySize = std::min((size_t)segInfo.fileSize, (size_t)segInfo.sizeOfSections);
            uint64_t offsetInRegion = addr - _readWriteRegion.unslidLoadAddress;
            SegmentMappingInfo loc;
            loc.srcSegment             = (uint8_t*)dylib.input->mappedFile.mh + segInfo.vmAddr - textSegVmAddr;
            loc.segName                = segInfo.segName;
            loc.dstSegment             = _readWriteRegion.buffer + offsetInRegion;
            loc.dstCacheUnslidAddress  = addr;
            loc.dstCacheFileOffset     = (uint32_t)(_readWriteRegion.cacheFileOffset + offsetInRegion);
            loc.dstCacheSegmentSize    = (uint32_t)segInfo.sizeOfSections;
            loc.dstCacheFileSize       = (uint32_t)copySize;
            loc.copySegmentSize        = (uint32_t)copySize;
            loc.srcSegmentIndex        = segInfo.segIndex;
            dylib.cacheLocation.push_back(loc);
            addr += loc.dstCacheSegmentSize;
        });
    }

    // layout all __DATA_DIRTY segments, sorted (FIXME)
	const size_t dylibCount = _sortedDylibs.size();
    uint32_t dirtyDataSortIndexes[dylibCount];
    for (size_t i=0; i < dylibCount; ++i)
        dirtyDataSortIndexes[i] = (uint32_t)i;
    std::sort(&dirtyDataSortIndexes[0], &dirtyDataSortIndexes[dylibCount], [&](const uint32_t& a, const uint32_t& b) {
        const auto& orderA = _options.dirtyDataSegmentOrdering.find(_sortedDylibs[a].input->mappedFile.runtimePath);
        const auto& orderB = _options.dirtyDataSegmentOrdering.find(_sortedDylibs[b].input->mappedFile.runtimePath);
        bool foundA = (orderA != _options.dirtyDataSegmentOrdering.end());
        bool foundB = (orderB != _options.dirtyDataSegmentOrdering.end());

        // Order all __DATA_DIRTY segments specified in the order file first, in the order specified in the file,
        // followed by any other __DATA_DIRTY segments in lexicographic order.
        if ( foundA && foundB )
            return orderA->second < orderB->second;
        else if ( foundA )
            return true;
        else if ( foundB )
             return false;
        else
             return _sortedDylibs[a].input->mappedFile.runtimePath < _sortedDylibs[b].input->mappedFile.runtimePath;
    });
    addr = align(addr, 12);
    for (size_t i=0; i < dylibCount; ++i) {
        DylibInfo& dylib  = _sortedDylibs[dirtyDataSortIndexes[i]];
        __block uint64_t textSegVmAddr = 0;
        dylib.input->mappedFile.mh->forEachSegment(^(const dyld3::MachOFile::SegmentInfo& segInfo, bool& stop) {
            if ( _options.platform == dyld3::Platform::watchOS_simulator)
                return;
            if ( strcmp(segInfo.segName, "__TEXT") == 0 )
                textSegVmAddr = segInfo.vmAddr;
            if ( segInfo.protections != (VM_PROT_READ | VM_PROT_WRITE) )
                return;
            if ( strcmp(segInfo.segName, "__DATA_DIRTY") != 0 )
                return;
            // Pack __DATA_DIRTY segments
            addr = align(addr, segInfo.p2align);
            size_t copySize = std::min((size_t)segInfo.fileSize, (size_t)segInfo.sizeOfSections);
            uint64_t offsetInRegion = addr - _readWriteRegion.unslidLoadAddress;
            SegmentMappingInfo loc;
            loc.srcSegment             = (uint8_t*)dylib.input->mappedFile.mh + segInfo.vmAddr - textSegVmAddr;
            loc.segName                = segInfo.segName;
            loc.dstSegment             = _readWriteRegion.buffer + offsetInRegion;
            loc.dstCacheUnslidAddress  = addr;
            loc.dstCacheFileOffset     = (uint32_t)(_readWriteRegion.cacheFileOffset + offsetInRegion);
            loc.dstCacheSegmentSize    = (uint32_t)segInfo.sizeOfSections;
            loc.dstCacheFileSize       = (uint32_t)copySize;
            loc.copySegmentSize        = (uint32_t)copySize;
            loc.srcSegmentIndex        = segInfo.segIndex;
            dylib.cacheLocation.push_back(loc);
            addr += loc.dstCacheSegmentSize;
        });
    }

    // reserve space for objc r/w optimization tables
    _objcReadWriteBufferSizeAllocated = align(computeReadWriteObjC((uint32_t)_sortedDylibs.size(), totalProtocolDefCount), 14);
    addr = align(addr, 4); // objc r/w section contains pointer and must be at least pointer align
    _objcReadWriteBuffer = _readWriteRegion.buffer + (addr - _readWriteRegion.unslidLoadAddress);
    addr += _objcReadWriteBufferSizeAllocated;

    // align DATA region end
    uint64_t endDataAddress = align(addr, _archLayout->sharedRegionAlignP2);
    _readWriteRegion.bufferSize   = endDataAddress - _readWriteRegion.unslidLoadAddress;
    _readWriteRegion.sizeInUse    = _readWriteRegion.bufferSize;

    // start read-only region
    if ( _archLayout->sharedRegionsAreDiscontiguous )
        addr = _archLayout->sharedMemoryStart + 0xA0000000;
    else
        addr = align((addr + _archLayout->sharedRegionPadding), _archLayout->sharedRegionAlignP2);
    _readOnlyRegion.buffer               = (uint8_t*)_fullAllocatedBuffer + addr - _archLayout->sharedMemoryStart;
    _readOnlyRegion.bufferSize           = 0;
    _readOnlyRegion.sizeInUse            = 0;
    _readOnlyRegion.unslidLoadAddress    = addr;
    _readOnlyRegion.cacheFileOffset      = _readWriteRegion.cacheFileOffset + _readWriteRegion.sizeInUse;

    // reserve space for kernel ASLR slide info at start of r/o region
    if ( _options.cacheSupportsASLR ) {
        size_t slideInfoSize = sizeof(dyld_cache_slide_info);
        slideInfoSize = std::max(slideInfoSize, sizeof(dyld_cache_slide_info2));
        slideInfoSize = std::max(slideInfoSize, sizeof(dyld_cache_slide_info3));
        slideInfoSize = std::max(slideInfoSize, sizeof(dyld_cache_slide_info4));
        _slideInfoBufferSizeAllocated = align(slideInfoSize + (_readWriteRegion.sizeInUse/4096) * _archLayout->slideInfoBytesPerPage, _archLayout->sharedRegionAlignP2);
        _slideInfoFileOffset = _readOnlyRegion.cacheFileOffset;
        addr += _slideInfoBufferSizeAllocated;
    }

    // layout all read-only (but not LINKEDIT) segments
    if ( _archLayout->textAndDataMaxSize == 0 ) {
        for (DylibInfo& dylib : _sortedDylibs) {
            __block uint64_t textSegVmAddr = 0;
            dylib.input->mappedFile.mh->forEachSegment(^(const dyld3::MachOFile::SegmentInfo& segInfo, bool& stop) {
                if ( strcmp(segInfo.segName, "__TEXT") == 0 )
                    textSegVmAddr = segInfo.vmAddr;
                if ( segInfo.protections != VM_PROT_READ )
                    return;
                if ( strcmp(segInfo.segName, "__LINKEDIT") == 0 )
                    return;

                // Keep segments segments 4K or more aligned
                addr = align(addr, std::max((int)segInfo.p2align, (int)12));
                uint64_t offsetInRegion = addr - _readOnlyRegion.unslidLoadAddress;
                SegmentMappingInfo loc;
                loc.srcSegment             = (uint8_t*)dylib.input->mappedFile.mh + segInfo.vmAddr - textSegVmAddr;
                loc.segName                = segInfo.segName;
                loc.dstSegment             = _readOnlyRegion.buffer + offsetInRegion;
                loc.dstCacheUnslidAddress  = addr;
                loc.dstCacheFileOffset     = (uint32_t)(_readOnlyRegion.cacheFileOffset + offsetInRegion);
                loc.dstCacheSegmentSize    = (uint32_t)align(segInfo.sizeOfSections, 12);
                loc.dstCacheFileSize       = (uint32_t)segInfo.sizeOfSections;
                loc.copySegmentSize        = (uint32_t)segInfo.sizeOfSections;
                loc.srcSegmentIndex        = segInfo.segIndex;
                dylib.cacheLocation.push_back(loc);
                addr += loc.dstCacheSegmentSize;
            });
        }
    }
    // layout all LINKEDIT segments (after other read-only segments), aligned to 16KB
    addr = align(addr, 14);
    _nonLinkEditReadOnlySize =  addr - _readOnlyRegion.unslidLoadAddress;
    for (DylibInfo& dylib : _sortedDylibs) {
        __block uint64_t textSegVmAddr = 0;
        dylib.input->mappedFile.mh->forEachSegment(^(const dyld3::MachOFile::SegmentInfo& segInfo, bool& stop) {
            if ( strcmp(segInfo.segName, "__TEXT") == 0 )
                textSegVmAddr = segInfo.vmAddr;
            if ( segInfo.protections != VM_PROT_READ )
                return;
            if ( strcmp(segInfo.segName, "__LINKEDIT") != 0 )
                return;
            // Keep segments segments 4K or more aligned
            addr = align(addr, std::max((int)segInfo.p2align, (int)12));
            size_t copySize = std::min((size_t)segInfo.fileSize, (size_t)segInfo.sizeOfSections);
            uint64_t offsetInRegion = addr - _readOnlyRegion.unslidLoadAddress;
            SegmentMappingInfo loc;
            loc.srcSegment             = (uint8_t*)dylib.input->mappedFile.mh + segInfo.vmAddr - textSegVmAddr;
            loc.segName                = segInfo.segName;
            loc.dstSegment             = _readOnlyRegion.buffer + offsetInRegion;
            loc.dstCacheUnslidAddress  = addr;
            loc.dstCacheFileOffset     = (uint32_t)(_readOnlyRegion.cacheFileOffset + offsetInRegion);
            loc.dstCacheSegmentSize    = (uint32_t)align(segInfo.sizeOfSections, 12);
            loc.dstCacheFileSize       = (uint32_t)copySize;
            loc.copySegmentSize        = (uint32_t)copySize;
            loc.srcSegmentIndex        = segInfo.segIndex;
            dylib.cacheLocation.push_back(loc);
            addr += loc.dstCacheSegmentSize;
        });
    }

    // align r/o region end
    uint64_t endReadOnlyAddress = align(addr, _archLayout->sharedRegionAlignP2);
    _readOnlyRegion.bufferSize  = endReadOnlyAddress - _readOnlyRegion.unslidLoadAddress;
    _readOnlyRegion.sizeInUse   = _readOnlyRegion.bufferSize;

    //fprintf(stderr, "RX region=%p -> %p, logical addr=0x%llX\n", _readExecuteRegion.buffer, _readExecuteRegion.buffer+_readExecuteRegion.bufferSize, _readExecuteRegion.unslidLoadAddress);
    //fprintf(stderr, "RW region=%p -> %p, logical addr=0x%llX\n", _readWriteRegion.buffer,   _readWriteRegion.buffer+_readWriteRegion.bufferSize, _readWriteRegion.unslidLoadAddress);
    //fprintf(stderr, "RO region=%p -> %p, logical addr=0x%llX\n", _readOnlyRegion.buffer,    _readOnlyRegion.buffer+_readOnlyRegion.bufferSize, _readOnlyRegion.unslidLoadAddress);

    // sort SegmentMappingInfo for each image to be in the same order as original segments
    for (DylibInfo& dylib : _sortedDylibs) {
        std::sort(dylib.cacheLocation.begin(), dylib.cacheLocation.end(), [&](const SegmentMappingInfo& a, const SegmentMappingInfo& b) {
            return a.srcSegmentIndex < b.srcSegmentIndex;
        });
    }
}

void CacheBuilder::markPaddingInaccessible()
{
    // region between RX and RW
    uint8_t* startPad1 = _readExecuteRegion.buffer+_readExecuteRegion.sizeInUse;
    uint8_t* endPad1   = _readWriteRegion.buffer;
    ::vm_protect(mach_task_self(), (vm_address_t)startPad1, endPad1-startPad1, false, 0);

    // region between RW and RO
    uint8_t* startPad2 = _readWriteRegion.buffer+_readWriteRegion.sizeInUse;
    uint8_t* endPad2   = _readOnlyRegion.buffer;
    ::vm_protect(mach_task_self(), (vm_address_t)startPad2, endPad2-startPad2, false, 0);
}


uint64_t CacheBuilder::pathHash(const char* path)
{
    uint64_t sum = 0;
    for (const char* s=path; *s != '\0'; ++s)
        sum += sum*4 + *s;
    return sum;
}


void CacheBuilder::findDylibAndSegment(const void* contentPtr, std::string& foundDylibName, std::string& foundSegName)
{
    foundDylibName = "???";
    foundSegName   = "???";
    uint64_t unslidVmAddr = ((uint8_t*)contentPtr - _readExecuteRegion.buffer) + _readExecuteRegion.unslidLoadAddress;
    const DyldSharedCache* cache = (DyldSharedCache*)_readExecuteRegion.buffer;
    cache->forEachImage(^(const mach_header* mh, const char* installName) {
        ((dyld3::MachOLoaded*)mh)->forEachSegment(^(const dyld3::MachOFile::SegmentInfo& info, bool &stop) {
            if ( (unslidVmAddr >= info.vmAddr) && (unslidVmAddr < (info.vmAddr+info.vmSize)) ) {
                foundDylibName = installName;
                foundSegName   = info.segName;
                stop           = true;
            }
        });
    });
}


template <typename P>
bool CacheBuilder::makeRebaseChainV2(uint8_t* pageContent, uint16_t lastLocationOffset, uint16_t offset, const dyld_cache_slide_info2* info)
{
    typedef typename P::uint_t     pint_t;

    const pint_t   deltaMask    = (pint_t)(info->delta_mask);
    const pint_t   valueMask    = ~deltaMask;
    const pint_t   valueAdd     = (pint_t)(info->value_add);
    const unsigned deltaShift   = __builtin_ctzll(deltaMask) - 2;
    const uint32_t maxDelta     = (uint32_t)(deltaMask >> deltaShift);

    pint_t* lastLoc = (pint_t*)&pageContent[lastLocationOffset+0];
    pint_t lastValue = (pint_t)P::getP(*lastLoc);
    if ( (lastValue - valueAdd) & deltaMask ) {
        std::string dylibName;
        std::string segName;
        findDylibAndSegment((void*)pageContent, dylibName, segName);
        _diagnostics.error("rebase pointer (0x%0lX) does not point within cache. lastOffset=0x%04X, seg=%s, dylib=%s\n",
                            (long)lastValue, lastLocationOffset, segName.c_str(), dylibName.c_str());
        return false;
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
    //fprintf(stderr, "  too big delta = %d, lastOffset=0x%03X, offset=0x%03X\n", offset - lastLocationOffset, lastLocationOffset, offset);

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
    for (unsigned n=0; n < nrIndex; ++n) {
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
void CacheBuilder::addPageStartsV2(uint8_t* pageContent, const bool bitmap[], const dyld_cache_slide_info2* info,
                                std::vector<uint16_t>& pageStarts, std::vector<uint16_t>& pageExtras)
{
    typedef typename P::uint_t     pint_t;

    const pint_t   deltaMask    = (pint_t)(info->delta_mask);
    const pint_t   valueMask    = ~deltaMask;
    const uint32_t pageSize     = info->page_size;
    const pint_t   valueAdd     = (pint_t)(info->value_add);

    uint16_t startValue = DYLD_CACHE_SLIDE_PAGE_ATTR_NO_REBASE;
    uint16_t lastLocationOffset = 0xFFFF;
    for(uint32_t i=0; i < pageSize/4; ++i) {
        unsigned offset = i*4;
        if ( bitmap[i] ) {
            if ( startValue == DYLD_CACHE_SLIDE_PAGE_ATTR_NO_REBASE ) {
                // found first rebase location in page
                startValue = i;
            }
            else if ( !makeRebaseChainV2<P>(pageContent, lastLocationOffset, offset, info) ) {
                // can't record all rebasings in one chain
                if ( (startValue & DYLD_CACHE_SLIDE_PAGE_ATTR_EXTRA) == 0 ) {
                    // switch page_start to "extras" which is a list of chain starts
                    unsigned indexInExtras = (unsigned)pageExtras.size();
                    if ( indexInExtras > 0x3FFF ) {
                        _diagnostics.error("rebase overflow in v2 page extras");
                        return;
                    }
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
void CacheBuilder::writeSlideInfoV2(const bool bitmap[], unsigned dataPageCount)
{
    typedef typename P::uint_t    pint_t;
    typedef typename P::E         E;
    const uint32_t pageSize = 4096;

    // fill in fixed info
    assert(_slideInfoFileOffset != 0);
    dyld_cache_slide_info2* info = (dyld_cache_slide_info2*)_readOnlyRegion.buffer;
    info->version    = 2;
    info->page_size  = pageSize;
    info->delta_mask = _archLayout->pointerDeltaMask;
    info->value_add  = _archLayout->useValueAdd ? _archLayout->sharedMemoryStart : 0;

    // set page starts and extras for each page
    std::vector<uint16_t> pageStarts;
    std::vector<uint16_t> pageExtras;
    pageStarts.reserve(dataPageCount);
    uint8_t* pageContent = _readWriteRegion.buffer;
    const bool* bitmapForPage = bitmap;
    for (unsigned i=0; i < dataPageCount; ++i) {
        //warning("page[%d]", i);
        addPageStartsV2<P>(pageContent, bitmapForPage, info, pageStarts, pageExtras);
        if ( _diagnostics.hasError() ) {
            return;
        }
        pageContent += pageSize;
        bitmapForPage += (sizeof(bool)*(pageSize/4));
    }

    // fill in computed info
    info->page_starts_offset = sizeof(dyld_cache_slide_info2);
    info->page_starts_count  = (unsigned)pageStarts.size();
    info->page_extras_offset = (unsigned)(sizeof(dyld_cache_slide_info2)+pageStarts.size()*sizeof(uint16_t));
    info->page_extras_count  = (unsigned)pageExtras.size();
    uint16_t* pageStartsBuffer = (uint16_t*)((char*)info + info->page_starts_offset);
    uint16_t* pageExtrasBuffer = (uint16_t*)((char*)info + info->page_extras_offset);
    for (unsigned i=0; i < pageStarts.size(); ++i)
        pageStartsBuffer[i] = pageStarts[i];
    for (unsigned i=0; i < pageExtras.size(); ++i)
        pageExtrasBuffer[i] = pageExtras[i];
    // update header with final size
    uint64_t slideInfoSize = align(info->page_extras_offset + pageExtras.size()*sizeof(uint16_t), _archLayout->sharedRegionAlignP2);
    if ( slideInfoSize > _slideInfoBufferSizeAllocated ) {
        _diagnostics.error("kernel slide info overflow buffer");
    }
    ((dyld_cache_header*)_readExecuteRegion.buffer)->slideInfoSize = slideInfoSize;
    //fprintf(stderr, "pageCount=%u, page_starts_count=%lu, page_extras_count=%lu\n", dataPageCount, pageStarts.size(), pageExtras.size());
}

#if SUPPORT_ARCH_arm64_32 || SUPPORT_ARCH_armv7k
// fits in to int16_t
static bool smallValue(uint64_t value)
{
    uint32_t high = (value & 0xFFFF8000);
    return (high == 0) || (high == 0xFFFF8000);
}

template <typename P>
bool CacheBuilder::makeRebaseChainV4(uint8_t* pageContent, uint16_t lastLocationOffset, uint16_t offset, const dyld_cache_slide_info4* info)
{
    typedef typename P::uint_t     pint_t;

    const pint_t   deltaMask    = (pint_t)(info->delta_mask);
    const pint_t   valueMask    = ~deltaMask;
    const pint_t   valueAdd     = (pint_t)(info->value_add);
    const unsigned deltaShift   = __builtin_ctzll(deltaMask) - 2;
    const uint32_t maxDelta     = (uint32_t)(deltaMask >> deltaShift);

    pint_t* lastLoc = (pint_t*)&pageContent[lastLocationOffset+0];
    pint_t lastValue = (pint_t)P::getP(*lastLoc);
    if ( (lastValue - valueAdd) & deltaMask ) {
        std::string dylibName;
        std::string segName;
        findDylibAndSegment((void*)pageContent, dylibName, segName);
        _diagnostics.error("rebase pointer does not point within cache. lastOffset=0x%04X, seg=%s, dylib=%s\n",
                            lastLocationOffset, segName.c_str(), dylibName.c_str());
        return false;
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
    //fprintf(stderr, "  too big delta = %d, lastOffset=0x%03X, offset=0x%03X\n", offset - lastLocationOffset, lastLocationOffset, offset);

    // distance between rebase locations is too far
    // see if we can make a chain from non-rebase locations
    uint16_t nonRebaseLocationOffsets[1024];
    unsigned nrIndex = 0;
    for (uint16_t i = lastLocationOffset; i < offset-maxDelta; ) {
        nonRebaseLocationOffsets[nrIndex] = 0;
        for (int j=maxDelta; j > 0; j -= 4) {
            pint_t value = (pint_t)P::getP(*(pint_t*)&pageContent[i+j]);
            if ( smallValue(value) ) {
                // Steal values of 0 to be used in the rebase chain
                nonRebaseLocationOffsets[nrIndex] = i+j;
                break;
            }
        }
        if ( nonRebaseLocationOffsets[nrIndex] == 0 ) {
            lastValue = (pint_t)P::getP(*lastLoc);
            pint_t newValue = ((lastValue - valueAdd) & valueMask);
            //fprintf(stderr, "   no way to make non-rebase delta chain, terminate off=0x%03X, old value=0x%08lX, new value=0x%08lX\n",
            //                lastLocationOffset, (long)lastValue, (long)newValue);
            P::setP(*lastLoc, newValue);
            return false;
        }
        i = nonRebaseLocationOffsets[nrIndex];
        ++nrIndex;
    }

    // we can make chain. go back and add each non-rebase location to chain
    uint16_t prevOffset = lastLocationOffset;
    pint_t* prevLoc = (pint_t*)&pageContent[prevOffset];
    for (unsigned n=0; n < nrIndex; ++n) {
        uint16_t nOffset = nonRebaseLocationOffsets[n];
        assert(nOffset != 0);
        pint_t* nLoc = (pint_t*)&pageContent[nOffset];
        uint32_t delta2 = nOffset - prevOffset;
        pint_t value = (pint_t)P::getP(*prevLoc);
        pint_t newValue;
        if ( smallValue(value) )
            newValue = (value & valueMask) | (delta2 << deltaShift);
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
    if ( smallValue(value) )
        newValue = (value & valueMask) | (delta3 << deltaShift);
    else
        newValue = ((value - valueAdd) & valueMask) | (delta3 << deltaShift);
    //warning("    non-rebase delta = %d, to off=0x%03X, old value=0x%08lX, new value=0x%08lX", delta3, offset, (long)value, (long)newValue);
    P::setP(*prevLoc, newValue);

    return true;
}


template <typename P>
void CacheBuilder::addPageStartsV4(uint8_t* pageContent, const bool bitmap[], const dyld_cache_slide_info4* info,
                                std::vector<uint16_t>& pageStarts, std::vector<uint16_t>& pageExtras)
{
    typedef typename P::uint_t     pint_t;

    const pint_t   deltaMask    = (pint_t)(info->delta_mask);
    const pint_t   valueMask    = ~deltaMask;
    const uint32_t pageSize     = info->page_size;
    const pint_t   valueAdd     = (pint_t)(info->value_add);

    uint16_t startValue = DYLD_CACHE_SLIDE4_PAGE_NO_REBASE;
    uint16_t lastLocationOffset = 0xFFFF;
    for(uint32_t i=0; i < pageSize/4; ++i) {
        unsigned offset = i*4;
        if ( bitmap[i] ) {
            if ( startValue == DYLD_CACHE_SLIDE4_PAGE_NO_REBASE ) {
                // found first rebase location in page
                startValue = i;
            }
            else if ( !makeRebaseChainV4<P>(pageContent, lastLocationOffset, offset, info) ) {
                // can't record all rebasings in one chain
                if ( (startValue & DYLD_CACHE_SLIDE4_PAGE_USE_EXTRA) == 0 ) {
                    // switch page_start to "extras" which is a list of chain starts
                    unsigned indexInExtras = (unsigned)pageExtras.size();
                    if ( indexInExtras >= DYLD_CACHE_SLIDE4_PAGE_INDEX ) {
                        _diagnostics.error("rebase overflow in v4 page extras");
                        return;
                    }
                    pageExtras.push_back(startValue);
                    startValue = indexInExtras | DYLD_CACHE_SLIDE4_PAGE_USE_EXTRA;
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
    if ( startValue & DYLD_CACHE_SLIDE4_PAGE_USE_EXTRA ) {
        // add end bit to extras
        pageExtras.back() |= DYLD_CACHE_SLIDE4_PAGE_EXTRA_END;
    }
    pageStarts.push_back(startValue);
}



template <typename P>
void CacheBuilder::writeSlideInfoV4(const bool bitmap[], unsigned dataPageCount)
{
    typedef typename P::uint_t    pint_t;
    typedef typename P::E         E;
    const uint32_t pageSize = 4096;

    // fill in fixed info
    assert(_slideInfoFileOffset != 0);
    dyld_cache_slide_info4* info = (dyld_cache_slide_info4*)_readOnlyRegion.buffer;
    info->version    = 4;
    info->page_size  = pageSize;
    info->delta_mask = _archLayout->pointerDeltaMask;
    info->value_add  = info->value_add  = _archLayout->useValueAdd ? _archLayout->sharedMemoryStart : 0;

    // set page starts and extras for each page
    std::vector<uint16_t> pageStarts;
    std::vector<uint16_t> pageExtras;
    pageStarts.reserve(dataPageCount);
    uint8_t* pageContent = _readWriteRegion.buffer;
    const bool* bitmapForPage = bitmap;
    for (unsigned i=0; i < dataPageCount; ++i) {
        addPageStartsV4<P>(pageContent, bitmapForPage, info, pageStarts, pageExtras);
        if ( _diagnostics.hasError() ) {
            return;
        }
        pageContent += pageSize;
        bitmapForPage += (sizeof(bool)*(pageSize/4));
    }
    // fill in computed info
    info->page_starts_offset = sizeof(dyld_cache_slide_info4);
    info->page_starts_count  = (unsigned)pageStarts.size();
    info->page_extras_offset = (unsigned)(sizeof(dyld_cache_slide_info4)+pageStarts.size()*sizeof(uint16_t));
    info->page_extras_count  = (unsigned)pageExtras.size();
    uint16_t* pageStartsBuffer = (uint16_t*)((char*)info + info->page_starts_offset);
    uint16_t* pageExtrasBuffer = (uint16_t*)((char*)info + info->page_extras_offset);
    for (unsigned i=0; i < pageStarts.size(); ++i)
        pageStartsBuffer[i] = pageStarts[i];
    for (unsigned i=0; i < pageExtras.size(); ++i)
        pageExtrasBuffer[i] = pageExtras[i];
    // update header with final size
    uint64_t slideInfoSize = align(info->page_extras_offset + pageExtras.size()*sizeof(uint16_t), _archLayout->sharedRegionAlignP2);
    if ( slideInfoSize > _slideInfoBufferSizeAllocated ) {
        _diagnostics.error("kernel slide info v4 overflow buffer");
    }
    ((dyld_cache_header*)_readExecuteRegion.buffer)->slideInfoSize = slideInfoSize;
    //fprintf(stderr, "pageCount=%u, page_starts_count=%lu, page_extras_count=%lu\n", dataPageCount, pageStarts.size(), pageExtras.size());
}
#endif

/*
void CacheBuilder::writeSlideInfoV1()
{
    // build one 128-byte bitmap per page (4096) of DATA
    uint8_t* const dataStart = (uint8_t*)_buffer.get() + regions[1].fileOffset;
    uint8_t* const dataEnd   = dataStart + regions[1].size;
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
    dyld_cache_slide_info* slideInfo = (dyld_cache_slide_info*)((uint8_t*)_buffer + _slideInfoFileOffset);
    slideInfo->version          = 1;
    slideInfo->toc_offset       = sizeof(dyld_cache_slide_info);
    slideInfo->toc_count        = toc_count;
    slideInfo->entries_offset   = (slideInfo->toc_offset+2*toc_count+127)&(-128);
    slideInfo->entries_count    = 0;
    slideInfo->entries_size     = entry_size;
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
    slideInfo->entries_count  = entry_count;
    ::free((void*)bitmap);

    _buffer.header->slideInfoSize = align(slideInfo->entries_offset + entry_count*entry_size, _archLayout->sharedRegionAlignP2);
}

*/



uint16_t CacheBuilder::pageStartV3(uint8_t* pageContent, uint32_t pageSize, const bool bitmap[])
{
    const int maxPerPage = pageSize / 4;
    uint16_t result = DYLD_CACHE_SLIDE_V3_PAGE_ATTR_NO_REBASE;
    dyld3::MachOLoaded::ChainedFixupPointerOnDisk* lastLoc = nullptr;
    for (int i=0; i < maxPerPage; ++i) {
        if ( bitmap[i] ) {
            if ( result == DYLD_CACHE_SLIDE_V3_PAGE_ATTR_NO_REBASE ) {
                // found first rebase location in page
                result = i * 4;
            }
            dyld3::MachOLoaded::ChainedFixupPointerOnDisk* loc = (dyld3::MachOLoaded::ChainedFixupPointerOnDisk*)(pageContent + i*4);;
            if ( lastLoc != nullptr ) {
                // update chain (original chain may be wrong because of segment packing)
                lastLoc->arm64e.rebase.next = loc - lastLoc;
            }
            lastLoc = loc;
        }
    }
    if ( lastLoc != nullptr ) {
        // mark last one as end of chain
        lastLoc->arm64e.rebase.next = 0;
	}
    return result;
}


void CacheBuilder::writeSlideInfoV3(const bool bitmap[], unsigned dataPageCount)
{
	const uint32_t pageSize = 4096;

    // fill in fixed info
    assert(_slideInfoFileOffset != 0);
    dyld_cache_slide_info3* info = (dyld_cache_slide_info3*)_readOnlyRegion.buffer;
    info->version           = 3;
    info->page_size         = pageSize;
    info->page_starts_count = dataPageCount;
    info->auth_value_add    = _archLayout->sharedMemoryStart;
    
    // fill in per-page starts
    uint8_t* pageContent = _readWriteRegion.buffer;
    const bool* bitmapForPage = bitmap;
    for (unsigned i=0; i < dataPageCount; ++i) {
        info->page_starts[i] = pageStartV3(pageContent, pageSize, bitmapForPage);
        pageContent += pageSize;
        bitmapForPage += (sizeof(bool)*(pageSize/4));
    }

    // update header with final size
    dyld_cache_header* dyldCacheHeader = (dyld_cache_header*)_readExecuteRegion.buffer;
    dyldCacheHeader->slideInfoSize = align(__offsetof(dyld_cache_slide_info3, page_starts[dataPageCount]), _archLayout->sharedRegionAlignP2);
    if ( dyldCacheHeader->slideInfoSize > _slideInfoBufferSizeAllocated ) {
        _diagnostics.error("kernel slide info overflow buffer");
    }
}


void CacheBuilder::fipsSign()
{
    // find libcorecrypto.dylib in cache being built
    DyldSharedCache* dyldCache = (DyldSharedCache*)_readExecuteRegion.buffer;
    __block const dyld3::MachOLoaded* ml = nullptr;
    dyldCache->forEachImage(^(const mach_header* mh, const char* installName) {
        if ( strcmp(installName, "/usr/lib/system/libcorecrypto.dylib") == 0 )
            ml = (dyld3::MachOLoaded*)mh;
    });
    if ( ml == nullptr ) {
        _diagnostics.warning("Could not find libcorecrypto.dylib, skipping FIPS sealing");
        return;
    }

    // find location in libcorecrypto.dylib to store hash of __text section
    uint64_t hashStoreSize;
    const void* hashStoreLocation = ml->findSectionContent("__TEXT", "__fips_hmacs", hashStoreSize);
    if ( hashStoreLocation == nullptr ) {
        _diagnostics.warning("Could not find __TEXT/__fips_hmacs section in libcorecrypto.dylib, skipping FIPS sealing");
        return;
    }
    if ( hashStoreSize != 32 ) {
        _diagnostics.warning("__TEXT/__fips_hmacs section in libcorecrypto.dylib is not 32 bytes in size, skipping FIPS sealing");
        return;
    }

    // compute hmac hash of __text section
    uint64_t textSize;
    const void* textLocation = ml->findSectionContent("__TEXT", "__text", textSize);
    if ( textLocation == nullptr ) {
        _diagnostics.warning("Could not find __TEXT/__text section in libcorecrypto.dylib, skipping FIPS sealing");
        return;
    }
    unsigned char hmac_key = 0;
    CCHmac(kCCHmacAlgSHA256, &hmac_key, 1, textLocation, textSize, (void*)hashStoreLocation); // store hash directly into hashStoreLocation
}

void CacheBuilder::codeSign()
{
    uint8_t  dscHashType;
    uint8_t  dscHashSize;
    uint32_t dscDigestFormat;
    bool agile = false;

    // select which codesigning hash
    switch (_options.codeSigningDigestMode) {
        case DyldSharedCache::Agile:
            agile = true;
            // Fall through to SHA1, because the main code directory remains SHA1 for compatibility.
            [[clang::fallthrough]];
        case DyldSharedCache::SHA1only:
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
            dscHashType     = CS_HASHTYPE_SHA1;
            dscHashSize     = CS_HASH_SIZE_SHA1;
            dscDigestFormat = kCCDigestSHA1;
#pragma clang diagnostic pop
            break;
        case DyldSharedCache::SHA256only:
            dscHashType     = CS_HASHTYPE_SHA256;
            dscHashSize     = CS_HASH_SIZE_SHA256;
            dscDigestFormat = kCCDigestSHA256;
            break;
        default:
            _diagnostics.error("codeSigningDigestMode has unknown, unexpected value %d, bailing out.",
                               _options.codeSigningDigestMode);
            return;
    }

    std::string cacheIdentifier = "com.apple.dyld.cache.";
    cacheIdentifier +=  _options.archs->name();
    if ( _options.dylibsRemovedDuringMastering ) {
        if ( _options.optimizeStubs  )
            cacheIdentifier +=  ".release";
        else
            cacheIdentifier += ".development";
    }
    // get pointers into shared cache buffer
    size_t          inBbufferSize = _readExecuteRegion.sizeInUse+_readWriteRegion.sizeInUse+_readOnlyRegion.sizeInUse+_localSymbolsRegion.sizeInUse;

    const uint16_t pageSize = _archLayout->csPageSize;

    // layout code signature contents
    uint32_t blobCount     = agile ? 4 : 3;
    size_t   idSize        = cacheIdentifier.size()+1; // +1 for terminating 0
    uint32_t slotCount     = (uint32_t)((inBbufferSize + pageSize - 1) / pageSize);
    uint32_t xSlotCount    = CSSLOT_REQUIREMENTS;
    size_t   idOffset      = offsetof(CS_CodeDirectory, end_withExecSeg);
    size_t   hashOffset    = idOffset+idSize + dscHashSize*xSlotCount;
    size_t   hash256Offset = idOffset+idSize + CS_HASH_SIZE_SHA256*xSlotCount;
    size_t   cdSize        = hashOffset + (slotCount * dscHashSize);
    size_t   cd256Size     = agile ? hash256Offset + (slotCount * CS_HASH_SIZE_SHA256) : 0;
    size_t   reqsSize      = 12;
    size_t   cmsSize       = sizeof(CS_Blob);
    size_t   cdOffset      = sizeof(CS_SuperBlob) + blobCount*sizeof(CS_BlobIndex);
    size_t   cd256Offset   = cdOffset + cdSize;
    size_t   reqsOffset    = cd256Offset + cd256Size; // equals cdOffset + cdSize if not agile
    size_t   cmsOffset     = reqsOffset + reqsSize;
    size_t   sbSize        = cmsOffset + cmsSize;
    size_t   sigSize       = align(sbSize, 14);       // keep whole cache 16KB aligned

    // allocate space for blob
    vm_address_t codeSigAlloc;
    if ( vm_allocate(mach_task_self(), &codeSigAlloc, sigSize, VM_FLAGS_ANYWHERE) != 0 ) {
        _diagnostics.error("could not allocate code signature buffer");
        return;
    }
    _codeSignatureRegion.buffer     = (uint8_t*)codeSigAlloc;
    _codeSignatureRegion.bufferSize = sigSize;
    _codeSignatureRegion.sizeInUse  = sigSize;

    // create overall code signature which is a superblob
    CS_SuperBlob* sb = reinterpret_cast<CS_SuperBlob*>(_codeSignatureRegion.buffer);
    sb->magic           = htonl(CSMAGIC_EMBEDDED_SIGNATURE);
    sb->length          = htonl(sbSize);
    sb->count           = htonl(blobCount);
    sb->index[0].type   = htonl(CSSLOT_CODEDIRECTORY);
    sb->index[0].offset = htonl(cdOffset);
    sb->index[1].type   = htonl(CSSLOT_REQUIREMENTS);
    sb->index[1].offset = htonl(reqsOffset);
    sb->index[2].type   = htonl(CSSLOT_CMS_SIGNATURE);
    sb->index[2].offset = htonl(cmsOffset);
    if ( agile ) {
        sb->index[3].type = htonl(CSSLOT_ALTERNATE_CODEDIRECTORIES + 0);
        sb->index[3].offset = htonl(cd256Offset);
    }

    // fill in empty requirements
    CS_RequirementsBlob* reqs = (CS_RequirementsBlob*)(((char*)sb)+reqsOffset);
    reqs->magic  = htonl(CSMAGIC_REQUIREMENTS);
    reqs->length = htonl(sizeof(CS_RequirementsBlob));
    reqs->data   = 0;

    // initialize fixed fields of Code Directory
    CS_CodeDirectory* cd = (CS_CodeDirectory*)(((char*)sb)+cdOffset);
    cd->magic           = htonl(CSMAGIC_CODEDIRECTORY);
    cd->length          = htonl(cdSize);
    cd->version         = htonl(0x20400);               // supports exec segment
    cd->flags           = htonl(kSecCodeSignatureAdhoc);
    cd->hashOffset      = htonl(hashOffset);
    cd->identOffset     = htonl(idOffset);
    cd->nSpecialSlots   = htonl(xSlotCount);
    cd->nCodeSlots      = htonl(slotCount);
    cd->codeLimit       = htonl(inBbufferSize);
    cd->hashSize        = dscHashSize;
    cd->hashType        = dscHashType;
    cd->platform        = 0;                            // not platform binary
    cd->pageSize        = __builtin_ctz(pageSize);      // log2(CS_PAGE_SIZE);
    cd->spare2          = 0;                            // unused (must be zero)
    cd->scatterOffset   = 0;                            // not supported anymore
    cd->teamOffset      = 0;                            // no team ID
    cd->spare3          = 0;                            // unused (must be zero)
    cd->codeLimit64     = 0;                            // falls back to codeLimit

    // executable segment info
    cd->execSegBase     = htonll(_readExecuteRegion.cacheFileOffset); // base of TEXT segment
    cd->execSegLimit    = htonll(_readExecuteRegion.sizeInUse);       // size of TEXT segment
    cd->execSegFlags    = 0;                                          // not a main binary

    // initialize dynamic fields of Code Directory
    strcpy((char*)cd + idOffset, cacheIdentifier.c_str());

    // add special slot hashes
    uint8_t* hashSlot = (uint8_t*)cd + hashOffset;
    uint8_t* reqsHashSlot = &hashSlot[-CSSLOT_REQUIREMENTS*dscHashSize];
    CCDigest(dscDigestFormat, (uint8_t*)reqs, sizeof(CS_RequirementsBlob), reqsHashSlot);

    CS_CodeDirectory* cd256;
    uint8_t* hash256Slot;
    uint8_t* reqsHash256Slot;
    if ( agile ) {
        // Note that the assumption here is that the size up to the hashes is the same as for
        // sha1 code directory, and that they come last, after everything else.

        cd256 = (CS_CodeDirectory*)(((char*)sb)+cd256Offset);
        cd256->magic           = htonl(CSMAGIC_CODEDIRECTORY);
        cd256->length          = htonl(cd256Size);
        cd256->version         = htonl(0x20400);               // supports exec segment
        cd256->flags           = htonl(kSecCodeSignatureAdhoc);
        cd256->hashOffset      = htonl(hash256Offset);
        cd256->identOffset     = htonl(idOffset);
        cd256->nSpecialSlots   = htonl(xSlotCount);
        cd256->nCodeSlots      = htonl(slotCount);
        cd256->codeLimit       = htonl(inBbufferSize);
        cd256->hashSize        = CS_HASH_SIZE_SHA256;
        cd256->hashType        = CS_HASHTYPE_SHA256;
        cd256->platform        = 0;                            // not platform binary
        cd256->pageSize        = __builtin_ctz(pageSize);      // log2(CS_PAGE_SIZE);
        cd256->spare2          = 0;                            // unused (must be zero)
        cd256->scatterOffset   = 0;                            // not supported anymore
        cd256->teamOffset      = 0;                            // no team ID
        cd256->spare3          = 0;                            // unused (must be zero)
        cd256->codeLimit64     = 0;                            // falls back to codeLimit

        // executable segment info
        cd256->execSegBase     = cd->execSegBase;
        cd256->execSegLimit    = cd->execSegLimit;
        cd256->execSegFlags    = cd->execSegFlags;

        // initialize dynamic fields of Code Directory
        strcpy((char*)cd256 + idOffset, cacheIdentifier.c_str());

        // add special slot hashes
        hash256Slot = (uint8_t*)cd256 + hash256Offset;
        reqsHash256Slot = &hash256Slot[-CSSLOT_REQUIREMENTS*CS_HASH_SIZE_SHA256];
        CCDigest(kCCDigestSHA256, (uint8_t*)reqs, sizeof(CS_RequirementsBlob), reqsHash256Slot);
    }
    else {
        cd256 = NULL;
        hash256Slot = NULL;
        reqsHash256Slot = NULL;
    }

    // fill in empty CMS blob for ad-hoc signing
    CS_Blob* cms = (CS_Blob*)(((char*)sb)+cmsOffset);
    cms->magic  = htonl(CSMAGIC_BLOBWRAPPER);
    cms->length = htonl(sizeof(CS_Blob));


    // alter header of cache to record size and location of code signature
    // do this *before* hashing each page
    dyld_cache_header* cache = (dyld_cache_header*)_readExecuteRegion.buffer;
    cache->codeSignatureOffset  = inBbufferSize;
    cache->codeSignatureSize    = sigSize;

    const uint32_t rwSlotStart = (uint32_t)(_readExecuteRegion.sizeInUse / pageSize);
    const uint32_t roSlotStart = (uint32_t)(rwSlotStart + _readWriteRegion.sizeInUse / pageSize);
    const uint32_t localsSlotStart = (uint32_t)(roSlotStart + _readOnlyRegion.sizeInUse / pageSize);
    auto codeSignPage = ^(size_t i) {
        const uint8_t* code = nullptr;
        // move to correct region
        if ( i < rwSlotStart )
            code = _readExecuteRegion.buffer + (i * pageSize);
        else if ( i >= rwSlotStart && i < roSlotStart )
            code = _readWriteRegion.buffer + ((i - rwSlotStart) * pageSize);
        else if ( i >= roSlotStart && i < localsSlotStart )
            code = _readOnlyRegion.buffer + ((i - roSlotStart) * pageSize);
        else
            code = _localSymbolsRegion.buffer + ((i - localsSlotStart) * pageSize);

        CCDigest(dscDigestFormat, code, pageSize, hashSlot + (i * dscHashSize));

        if ( agile ) {
            CCDigest(kCCDigestSHA256, code, pageSize, hash256Slot + (i * CS_HASH_SIZE_SHA256));
        }
    };

    // compute hashes
    dispatch_apply(slotCount, DISPATCH_APPLY_AUTO, ^(size_t i) {
        codeSignPage(i);
    });

    // Now that we have a code signature, compute a cache UUID by hashing the code signature blob
    {
        uint8_t* uuidLoc = cache->uuid;
        assert(uuid_is_null(uuidLoc));
        static_assert(offsetof(dyld_cache_header, uuid) / CS_PAGE_SIZE_4K == 0, "uuid is expected in the first page of the cache");
        uint8_t fullDigest[CC_SHA256_DIGEST_LENGTH];
        CC_SHA256((const void*)cd, (unsigned)cdSize, fullDigest);
        memcpy(uuidLoc, fullDigest, 16);
        // <rdar://problem/6723729> uuids should conform to RFC 4122 UUID version 4 & UUID version 5 formats
        uuidLoc[6] = ( uuidLoc[6] & 0x0F ) | ( 3 << 4 );
        uuidLoc[8] = ( uuidLoc[8] & 0x3F ) | 0x80;

        // Now codesign page 0 again, because we modified it by setting uuid in header
        codeSignPage(0);
    }

    // hash of entire code directory (cdHash) uses same hash as each page
    uint8_t fullCdHash[dscHashSize];
    CCDigest(dscDigestFormat, (const uint8_t*)cd, cdSize, fullCdHash);
    // Note: cdHash is defined as first 20 bytes of hash
    memcpy(_cdHashFirst, fullCdHash, 20);
    if ( agile ) {
        uint8_t fullCdHash256[CS_HASH_SIZE_SHA256];
        CCDigest(kCCDigestSHA256, (const uint8_t*)cd256, cd256Size, fullCdHash256);
        // Note: cdHash is defined as first 20 bytes of hash, even for sha256
        memcpy(_cdHashSecond, fullCdHash256, 20);
    }
    else {
        memset(_cdHashSecond, 0, 20);
    }
}

const bool CacheBuilder::agileSignature()
{
    return _options.codeSigningDigestMode == DyldSharedCache::Agile;
}

static const std::string cdHash(uint8_t hash[20])
{
    char buff[48];
    for (int i = 0; i < 20; ++i)
        sprintf(&buff[2*i], "%2.2x", hash[i]);
    return buff;
}

const std::string CacheBuilder::cdHashFirst()
{
    return cdHash(_cdHashFirst);
}

const std::string CacheBuilder::cdHashSecond()
{
    return cdHash(_cdHashSecond);
}

const std::string CacheBuilder::uuid() const
{
    dyld_cache_header* cache = (dyld_cache_header*)_readExecuteRegion.buffer;
    uuid_string_t uuidStr;
    uuid_unparse(cache->uuid, uuidStr);
    return uuidStr;
}

static dyld_cache_patchable_location makePatchLocation(size_t cacheOff, uint64_t ad) {
    int64_t signedAddend = (int64_t)ad;
    assert(((signedAddend << 52) >> 52) == signedAddend);
    dyld_cache_patchable_location patch;
    patch.cacheOffset           = cacheOff;
    patch.addend                = ad;
    patch.authenticated         = 0;
    patch.usesAddressDiversity  = 0;
    patch.key                   = 0;
    patch.discriminator         = 0;
    return patch;
}

static dyld_cache_patchable_location makePatchLocation(size_t cacheOff, uint64_t ad,
                                                       dyld3::MachOLoaded::ChainedFixupPointerOnDisk authInfo) {
    int64_t signedAddend = (int64_t)ad;
    assert(((signedAddend << 52) >> 52) == signedAddend);
    dyld_cache_patchable_location patch;
    patch.cacheOffset           = cacheOff;
    patch.addend                = ad;
    patch.authenticated         = authInfo.arm64e.authBind.auth;
    patch.usesAddressDiversity  = authInfo.arm64e.authBind.addrDiv;
    patch.key                   = authInfo.arm64e.authBind.key;
    patch.discriminator         = authInfo.arm64e.authBind.diversity;
    return patch;
}


void CacheBuilder::buildImageArray(std::vector<DyldSharedCache::FileAlias>& aliases)
{
    typedef dyld3::closure::ClosureBuilder::CachedDylibInfo         CachedDylibInfo;

    // convert STL data structures to simple arrays to passe to makeDyldCacheImageArray()
    __block std::vector<CachedDylibInfo> dylibInfos;
    __block std::unordered_map<dyld3::closure::ImageNum, const dyld3::MachOLoaded*> imageNumToML;
    DyldSharedCache* cache = (DyldSharedCache*)_readExecuteRegion.buffer;
    cache->forEachImage(^(const mach_header* mh, const char* installName) {
        uint64_t mtime;
        uint64_t inode;
        cache->getIndexedImageEntry((uint32_t)dylibInfos.size(), mtime, inode);
        CachedDylibInfo entry;
        entry.fileInfo.fileContent  = mh;
        entry.fileInfo.path         = installName;
        entry.fileInfo.sliceOffset  = 0;
        entry.fileInfo.inode        = inode;
        entry.fileInfo.mtime        = mtime;
        dylibInfos.push_back(entry);
        imageNumToML[(dyld3::closure::ImageNum)(dylibInfos.size())] = (dyld3::MachOLoaded*)mh;
    });

    // Convert symlinks from STL to simple char pointers.
    std::vector<dyld3::closure::ClosureBuilder::CachedDylibAlias> dylibAliases;
    dylibAliases.reserve(aliases.size());
    for (const auto& alias : aliases)
        dylibAliases.push_back({ alias.realPath.c_str(), alias.aliasPath.c_str() });

    dyld3::closure::ClosureBuilder::CacheDylibsBindingHandlers handlers;

    handlers.chainedBind = ^(dyld3::closure::ImageNum, const dyld3::MachOLoaded* imageLoadAddress,
                             const  dyld_chained_starts_in_image* starts,
                             const dyld3::Array<dyld3::closure::Image::ResolvedSymbolTarget>& targets,
                             const dyld3::Array<dyld3::closure::ClosureBuilder::ResolvedTargetInfo>& targetInfos) {
        imageLoadAddress->forEachFixupInAllChains(_diagnostics, starts, false, ^(dyld3::MachOLoaded::ChainedFixupPointerOnDisk* fixupLoc, const dyld_chained_starts_in_segment* segInfo, bool& stop) {
            uint64_t                                                  offsetInCache;
            dyld3::closure::Image::ResolvedSymbolTarget               target;
            const dyld3::closure::ClosureBuilder::ResolvedTargetInfo* targetInfo;
            switch (segInfo->pointer_format) {
                case DYLD_CHAINED_PTR_ARM64E:
                    if ( fixupLoc->arm64e.bind.bind ) {
                        target     = targets[fixupLoc->arm64e.bind.ordinal];
                        targetInfo = &targetInfos[fixupLoc->arm64e.bind.ordinal];
                        switch ( target.sharedCache.kind ) {
                            case dyld3::closure::Image::ResolvedSymbolTarget::kindSharedCache:
                                offsetInCache = target.sharedCache.offset - targetInfo->addend;
                                _dylibToItsExports[targetInfo->foundInDylib].insert(offsetInCache);
                                _exportsToName[offsetInCache] = targetInfo->foundSymbolName;
                                if ( fixupLoc->arm64e.authBind.auth ) {
                                    // turn this auth bind into an auth rebase into the cache
                                    fixupLoc->arm64e.authRebase.bind   = 0;
                                    fixupLoc->arm64e.authRebase.target = target.sharedCache.offset;
                                    _exportsToUses[offsetInCache].push_back(makePatchLocation((uint8_t*)fixupLoc - _readExecuteRegion.buffer, targetInfo->addend, *fixupLoc));
                                }
                                else {
                                    // turn this plain bind into an plain rebase into the cache
                                    fixupLoc->arm64e.rebase.bind   = 0;
                                    fixupLoc->arm64e.rebase.target = _archLayout->sharedMemoryStart + target.sharedCache.offset;
                                    _exportsToUses[offsetInCache].push_back(makePatchLocation((uint8_t*)fixupLoc - _readExecuteRegion.buffer, targetInfo->addend));
                                }
                                _aslrTracker.add(fixupLoc);
                                break;
                            case dyld3::closure::Image::ResolvedSymbolTarget::kindAbsolute:
                                fixupLoc->raw64 = target.absolute.value;
                                // don't record absolute targets for ASLR
                                // HACK: Split seg may have added a target.  Remove it
                                _aslrTracker.remove(fixupLoc);
                                if ( (targetInfo->libOrdinal > 0) && (targetInfo->libOrdinal <= (int)(imageLoadAddress->dependentDylibCount())) ) {
                                    _missingWeakImports[fixupLoc] = imageLoadAddress->dependentDylibLoadPath(targetInfo->libOrdinal - 1);
                                }
                               break;
                            default:
                                assert(0 && "unsupported ResolvedSymbolTarget kind in dyld cache");
                        }
                    } else {
                        _aslrTracker.add(fixupLoc);
                    }
                    break;
                case DYLD_CHAINED_PTR_64:
                    if ( fixupLoc->generic64.bind.bind ) {
                        target     = targets[fixupLoc->generic64.bind.ordinal];
                        targetInfo = &targetInfos[fixupLoc->generic64.bind.ordinal];
                        switch ( target.sharedCache.kind ) {
                            case dyld3::closure::Image::ResolvedSymbolTarget::kindSharedCache:
                                offsetInCache = target.sharedCache.offset - targetInfo->addend;
                                _dylibToItsExports[targetInfo->foundInDylib].insert(offsetInCache);
                                _exportsToName[offsetInCache] = targetInfo->foundSymbolName;
                                // turn this bind into a rebase into the cache
                                fixupLoc->generic64.rebase.bind     = 0;
                                fixupLoc->generic64.rebase.next     = 0; // rechained later
                                fixupLoc->generic64.rebase.reserved = 0;
                                fixupLoc->generic64.rebase.high8    = 0;
                                fixupLoc->generic64.rebase.target   = target.sharedCache.offset;
                                _exportsToUses[offsetInCache].push_back(makePatchLocation((uint8_t*)fixupLoc - _readExecuteRegion.buffer, targetInfo->addend));
                                _aslrTracker.add(fixupLoc);
                                break;
                            case dyld3::closure::Image::ResolvedSymbolTarget::kindAbsolute:
                                fixupLoc->raw64 = target.absolute.value;
                                 // don't record absolute targets for ASLR
                                if ( (targetInfo->libOrdinal > 0) && (targetInfo->libOrdinal <= (int)(imageLoadAddress->dependentDylibCount())) ) {
                                    _missingWeakImports[fixupLoc] = imageLoadAddress->dependentDylibLoadPath(targetInfo->libOrdinal - 1);
                                }
                                break;
                            default:
                                assert(0 && "unsupported ResolvedSymbolTarget kind in dyld cache");
                        }
                    }
                    break;
                case DYLD_CHAINED_PTR_32:
                    if ( fixupLoc->generic32.bind.bind ) {
                        target     = targets[fixupLoc->generic32.bind.ordinal];
                        targetInfo = &targetInfos[fixupLoc->generic32.bind.ordinal];
                        switch ( target.sharedCache.kind ) {
                            case dyld3::closure::Image::ResolvedSymbolTarget::kindSharedCache:
                                offsetInCache = target.sharedCache.offset - targetInfo->addend;
                                _dylibToItsExports[targetInfo->foundInDylib].insert(offsetInCache);
                                _exportsToName[offsetInCache] = targetInfo->foundSymbolName;
                                // turn this bind into a rebase into the cache
                                fixupLoc->cache32.next   = 0; // rechained later
                                fixupLoc->cache32.target = (uint32_t)(target.sharedCache.offset);
                                _exportsToUses[offsetInCache].push_back(makePatchLocation((uint8_t*)fixupLoc - _readExecuteRegion.buffer, targetInfo->addend));
                                _aslrTracker.add(fixupLoc);
                                break;
                            case dyld3::closure::Image::ResolvedSymbolTarget::kindAbsolute:
                                fixupLoc->raw32 = (uint32_t)target.absolute.value;
                                 // don't record absolute targets for ASLR
                                if ( (targetInfo->libOrdinal > 0) && (targetInfo->libOrdinal <= (int)(imageLoadAddress->dependentDylibCount())) ) {
                                    _missingWeakImports[fixupLoc] = imageLoadAddress->dependentDylibLoadPath(targetInfo->libOrdinal - 1);
                                }
                                break;
                            default:
                                assert(0 && "unsupported ResolvedSymbolTarget kind in dyld cache");
                        }
                    }
                   break;
                default:
                    assert(0 && "unsupported chained bind type");
            }

        });
    };

    handlers.rebase = ^(dyld3::closure::ImageNum imageNum, const dyld3::MachOLoaded* imageToFix, uint32_t runtimeOffset) {
        // record location in aslr tracker so kernel can slide this on page-in
        uint8_t* fixupLoc = (uint8_t*)imageToFix+runtimeOffset;
        _aslrTracker.add(fixupLoc);
    };

    handlers.bind = ^(dyld3::closure::ImageNum imageNum, const dyld3::MachOLoaded* mh,
                      uint32_t runtimeOffset, dyld3::closure::Image::ResolvedSymbolTarget target,
                      const dyld3::closure::ClosureBuilder::ResolvedTargetInfo& targetInfo) {
        uint8_t* fixupLoc = (uint8_t*)mh+runtimeOffset;

        // binder is called a second time for weak_bind info, which we ignore when building cache
        const bool weakDefUseAlreadySet = targetInfo.weakBindCoalese && _aslrTracker.has(fixupLoc);

        // do actual bind that sets pointer in image to unslid target address
        uint64_t offsetInCache;
        switch ( target.sharedCache.kind ) {
            case dyld3::closure::Image::ResolvedSymbolTarget::kindSharedCache:
                offsetInCache = target.sharedCache.offset - targetInfo.addend;
                _dylibToItsExports[targetInfo.foundInDylib].insert(offsetInCache);
                if (targetInfo.isWeakDef)
                    _dylibWeakExports.insert({ targetInfo.foundInDylib, offsetInCache });
                _exportsToUses[offsetInCache].push_back(makePatchLocation(fixupLoc - _readExecuteRegion.buffer, targetInfo.addend));
                _exportsToName[offsetInCache] = targetInfo.foundSymbolName;
                if ( !weakDefUseAlreadySet ) {
                    if ( _archLayout->is64 )
                        *((uint64_t*)fixupLoc) = _archLayout->sharedMemoryStart + target.sharedCache.offset;
                    else
                        *((uint32_t*)fixupLoc) = (uint32_t)(_archLayout->sharedMemoryStart + target.sharedCache.offset);
                    // record location in aslr tracker so kernel can slide this on page-in
                    _aslrTracker.add(fixupLoc);
                }
                break;
            case dyld3::closure::Image::ResolvedSymbolTarget::kindAbsolute:
                 if ( _archLayout->is64 )
                    *((uint64_t*)fixupLoc) = target.absolute.value;
                else
                    *((uint32_t*)fixupLoc) = (uint32_t)(target.absolute.value);
                // don't record absolute targets for ASLR
                // HACK: Split seg may have added a target.  Remove it
                _aslrTracker.remove(fixupLoc);
                if ( (targetInfo.libOrdinal > 0) && (targetInfo.libOrdinal <= (int)(mh->dependentDylibCount())) ) {
                    _missingWeakImports[fixupLoc] = mh->dependentDylibLoadPath(targetInfo.libOrdinal - 1);
                }
                break;
            default:
                assert(0 && "unsupported ResolvedSymbolTarget kind in dyld cache");
        }
    };

    // build ImageArray for all dylibs in dyld cache
    dyld3::closure::PathOverrides pathOverrides;
    dyld3::closure::ClosureBuilder cb(dyld3::closure::kFirstDyldCacheImageNum, _fileSystem, cache, false, *_options.archs, pathOverrides,
                                      dyld3::closure::ClosureBuilder::AtPath::none, false, nullptr, _options.platform, &handlers);
    dyld3::Array<CachedDylibInfo> dylibs(&dylibInfos[0], dylibInfos.size(), dylibInfos.size());
    const dyld3::Array<dyld3::closure::ClosureBuilder::CachedDylibAlias> aliasesArray(dylibAliases.data(), dylibAliases.size(), dylibAliases.size());
    _imageArray = cb.makeDyldCacheImageArray(_options.optimizeStubs, dylibs, aliasesArray);
    if ( cb.diagnostics().hasError() ) {
        _diagnostics.error("%s", cb.diagnostics().errorMessage().c_str());
        return;
    }
}

static bool operator==(const dyld_cache_patchable_location& a, const dyld_cache_patchable_location& b) {
    return a.cacheOffset == b.cacheOffset;
}

void CacheBuilder::addImageArray()
{
    // build trie of dylib paths
    __block std::vector<DylibIndexTrie::Entry> dylibEntrys;
    _imageArray->forEachImage(^(const dyld3::closure::Image* image, bool& stop) {
        dylibEntrys.push_back(DylibIndexTrie::Entry(image->path(), DylibIndex(image->imageNum()-1)));
        image->forEachAlias(^(const char *aliasPath, bool &innerStop) {
            dylibEntrys.push_back(DylibIndexTrie::Entry(aliasPath, DylibIndex(image->imageNum()-1)));
        });
    });
    DylibIndexTrie dylibsTrie(dylibEntrys);
    std::vector<uint8_t> trieBytes;
    dylibsTrie.emit(trieBytes);
    while ( (trieBytes.size() % 4) != 0 )
        trieBytes.push_back(0);

    // build set of functions to never stub-eliminate because tools may need to override them
    std::unordered_set<std::string> alwaysGeneratePatch;
    for (const char* const* p=_s_neverStubEliminateSymbols; *p != nullptr; ++p)
        alwaysGeneratePatch.insert(*p);

    // Add the patches for the image array.
    __block uint64_t numPatchImages             = _imageArray->size();
    __block uint64_t numPatchExports            = 0;
    __block uint64_t numPatchLocations          = 0;
    __block uint64_t numPatchExportNameBytes    = 0;

    auto needsPatch = [&](bool dylibNeedsPatching, const dyld3::MachOLoaded* mh,
                          CacheOffset offset) -> bool {
        if (dylibNeedsPatching)
            return true;
        if (_dylibWeakExports.find({ mh, offset }) != _dylibWeakExports.end())
            return true;
        const std::string& exportName = _exportsToName[offset];
        return alwaysGeneratePatch.find(exportName) != alwaysGeneratePatch.end();
    };

    std::set<std::string> alwaysPatchDylibs;
    for (const char* const* d= _s_neverStubEliminateDylibs; *d != nullptr; ++d)
        alwaysPatchDylibs.insert(*d);

    // First calculate how much space we need
    const DyldSharedCache* cache = (DyldSharedCache*)_readExecuteRegion.buffer;
    cache->forEachImage(^(const mach_header* mh, const char* installName) {
        const dyld3::MachOLoaded* ml = (const dyld3::MachOLoaded*)mh;
        const std::set<CacheOffset>& dylibExports = _dylibToItsExports[ml];

        // On a customer cache, only store patch locations for interposable dylibs and weak binding
        bool dylibNeedsPatching = !_options.optimizeStubs || alwaysPatchDylibs.count(installName);

        uint64_t numDylibExports = 0;
        for (CacheOffset exportCacheOffset : dylibExports) {
            if (!needsPatch(dylibNeedsPatching, ml, exportCacheOffset))
                continue;
            std::vector<dyld_cache_patchable_location>& uses = _exportsToUses[exportCacheOffset];
            uses.erase(std::unique(uses.begin(), uses.end()), uses.end());
            numPatchLocations += uses.size();

            std::string exportName = _exportsToName[exportCacheOffset];
            numPatchExportNameBytes += exportName.size() + 1;
        }
        numPatchExports += numDylibExports;
    });

    // Now reserve the space
    __block std::vector<dyld_cache_image_patches>       patchImages;
    __block std::vector<dyld_cache_patchable_export>    patchExports;
    __block std::vector<dyld_cache_patchable_location>  patchLocations;
    __block std::vector<char>                           patchExportNames;

    patchImages.reserve(numPatchImages);
    patchExports.reserve(numPatchExports);
    patchLocations.reserve(numPatchLocations);
    patchExportNames.reserve(numPatchExportNameBytes);

    // And now fill it with the patch data
    cache->forEachImage(^(const mach_header* mh, const char* installName) {
        const dyld3::MachOLoaded* ml = (const dyld3::MachOLoaded*)mh;
        const std::set<CacheOffset>& dylibExports = _dylibToItsExports[ml];

        // On a customer cache, only store patch locations for interposable dylibs and weak binding
        bool dylibNeedsPatching = !_options.optimizeStubs || alwaysPatchDylibs.count(installName);

        // Add the patch image which points in to the exports
        dyld_cache_image_patches patchImage;
        patchImage.patchExportsStartIndex   = (uint32_t)patchExports.size();
        patchImage.patchExportsCount        = 0;

        // Then add each export which points to a list of locations and a name
        for (CacheOffset exportCacheOffset : dylibExports) {
            if (!needsPatch(dylibNeedsPatching, ml, exportCacheOffset))
                continue;
            ++patchImage.patchExportsCount;
            std::vector<dyld_cache_patchable_location>& uses = _exportsToUses[exportCacheOffset];

            dyld_cache_patchable_export cacheExport;
            cacheExport.cacheOffsetOfImpl           = (uint32_t)exportCacheOffset;
            cacheExport.patchLocationsStartIndex    = (uint32_t)patchLocations.size();
            cacheExport.patchLocationsCount         = (uint32_t)uses.size();
            cacheExport.exportNameOffset            = (uint32_t)patchExportNames.size();
            patchExports.push_back(cacheExport);

            // Now add the list of locations.
            patchLocations.insert(patchLocations.end(), uses.begin(), uses.end());

            // And add the export name
            const std::string& exportName = _exportsToName[exportCacheOffset];
            patchExportNames.insert(patchExportNames.end(), &exportName[0], &exportName[0] + exportName.size() + 1);
        }
        patchImages.push_back(patchImage);
    });

    while ( (patchExportNames.size() % 4) != 0 )
        patchExportNames.push_back('\0');

    uint64_t patchInfoSize = sizeof(dyld_cache_patch_info);
    patchInfoSize += sizeof(dyld_cache_image_patches) * patchImages.size();
    patchInfoSize += sizeof(dyld_cache_patchable_export) * patchExports.size();
    patchInfoSize += sizeof(dyld_cache_patchable_location) * patchLocations.size();
    patchInfoSize += patchExportNames.size();

    // check for fit
    uint64_t imageArraySize = _imageArray->size();
    size_t freeSpace = _readOnlyRegion.bufferSize - _readOnlyRegion.sizeInUse;
    if ( (imageArraySize+trieBytes.size()+patchInfoSize) > freeSpace ) {
        _diagnostics.error("cache buffer too small to hold ImageArray and Trie (buffer size=%lldMB, imageArray size=%lldMB, trie size=%luKB, patch size=%lluKB, free space=%ldMB)",
                            _allocatedBufferSize/1024/1024, imageArraySize/1024/1024, trieBytes.size()/1024, patchInfoSize/1024, freeSpace/1024/1024);
        return;
    }

    // copy into cache and update header
    DyldSharedCache* dyldCache = (DyldSharedCache*)_readExecuteRegion.buffer;
    dyldCache->header.dylibsImageArrayAddr = _readOnlyRegion.unslidLoadAddress + _readOnlyRegion.sizeInUse;
    dyldCache->header.dylibsImageArraySize = imageArraySize;
    dyldCache->header.dylibsTrieAddr       = dyldCache->header.dylibsImageArrayAddr + imageArraySize;
    dyldCache->header.dylibsTrieSize       = trieBytes.size();
    ::memcpy(_readOnlyRegion.buffer + _readOnlyRegion.sizeInUse, _imageArray, imageArraySize);
    ::memcpy(_readOnlyRegion.buffer + _readOnlyRegion.sizeInUse + imageArraySize, &trieBytes[0], trieBytes.size());

    // Also write out the patch info
    dyldCache->header.patchInfoAddr = dyldCache->header.dylibsTrieAddr + dyldCache->header.dylibsTrieSize;
    dyldCache->header.patchInfoSize = patchInfoSize;
    dyld_cache_patch_info patchInfo;
    patchInfo.patchTableArrayAddr = dyldCache->header.patchInfoAddr + sizeof(dyld_cache_patch_info);
    patchInfo.patchTableArrayCount = patchImages.size();
    patchInfo.patchExportArrayAddr = patchInfo.patchTableArrayAddr + (patchInfo.patchTableArrayCount * sizeof(dyld_cache_image_patches));
    patchInfo.patchExportArrayCount = patchExports.size();
    patchInfo.patchLocationArrayAddr = patchInfo.patchExportArrayAddr + (patchInfo.patchExportArrayCount * sizeof(dyld_cache_patchable_export));
    patchInfo.patchLocationArrayCount = patchLocations.size();
    patchInfo.patchExportNamesAddr = patchInfo.patchLocationArrayAddr + (patchInfo.patchLocationArrayCount * sizeof(dyld_cache_patchable_location));
    patchInfo.patchExportNamesSize = patchExportNames.size();
    ::memcpy(_readOnlyRegion.buffer + dyldCache->header.patchInfoAddr - _readOnlyRegion.unslidLoadAddress,
             &patchInfo, sizeof(dyld_cache_patch_info));
    ::memcpy(_readOnlyRegion.buffer + patchInfo.patchTableArrayAddr - _readOnlyRegion.unslidLoadAddress,
             &patchImages[0], sizeof(patchImages[0]) * patchImages.size());
    ::memcpy(_readOnlyRegion.buffer + patchInfo.patchExportArrayAddr - _readOnlyRegion.unslidLoadAddress,
             &patchExports[0], sizeof(patchExports[0]) * patchExports.size());
    ::memcpy(_readOnlyRegion.buffer + patchInfo.patchLocationArrayAddr - _readOnlyRegion.unslidLoadAddress,
             &patchLocations[0], sizeof(patchLocations[0]) * patchLocations.size());
    ::memcpy(_readOnlyRegion.buffer + patchInfo.patchExportNamesAddr - _readOnlyRegion.unslidLoadAddress,
             &patchExportNames[0], patchExportNames.size());

    _readOnlyRegion.sizeInUse += align(imageArraySize+trieBytes.size()+patchInfoSize,14);

    // Free the underlying image array buffer
    _imageArray->deallocate();
}

void CacheBuilder::addOtherImageArray(const std::vector<LoadedMachO>& otherDylibsAndBundles, std::vector<const LoadedMachO*>& overflowDylibs)
{
    DyldSharedCache* cache = (DyldSharedCache*)_readExecuteRegion.buffer;
    dyld3::closure::PathOverrides pathOverrides;
    dyld3::closure::FileSystemNull nullFileSystem;
    dyld3::closure::ClosureBuilder cb(dyld3::closure::kFirstOtherOSImageNum, nullFileSystem, cache, false, *_options.archs, pathOverrides,
                                      dyld3::closure::ClosureBuilder::AtPath::none, false, nullptr, _options.platform);

    // make ImageArray for other dylibs and bundles
    STACK_ALLOC_ARRAY(dyld3::closure::LoadedFileInfo, others, otherDylibsAndBundles.size() + overflowDylibs.size());
    for (const LoadedMachO& other : otherDylibsAndBundles) {
        if ( !contains(other.loadedFileInfo.path, "staged_system_apps/") )
            others.push_back(other.loadedFileInfo);
    }

    for (const LoadedMachO* dylib : overflowDylibs) {
        if (dylib->mappedFile.mh->canHavePrecomputedDlopenClosure(dylib->mappedFile.runtimePath.c_str(), ^(const char*) {}) )
            others.push_back(dylib->loadedFileInfo);
    }

    // Sort the others array by name so that it is deterministic
    std::sort(others.begin(), others.end(),
              [](const dyld3::closure::LoadedFileInfo& a, const dyld3::closure::LoadedFileInfo& b) {
                  // Sort mac before iOSMac
                  bool isIOSMacA = strncmp(a.path, "/System/iOSSupport/", 19) == 0;
                  bool isIOSMacB = strncmp(b.path, "/System/iOSSupport/", 19) == 0;
                  if (isIOSMacA != isIOSMacB)
                      return !isIOSMacA;
                  return strcmp(a.path, b.path) < 0;
              });

    const dyld3::closure::ImageArray* otherImageArray = cb.makeOtherDylibsImageArray(others, (uint32_t)_sortedDylibs.size());

    // build trie of paths
    __block std::vector<DylibIndexTrie::Entry> otherEntrys;
    otherImageArray->forEachImage(^(const dyld3::closure::Image* image, bool& stop) {
        if ( !image->isInvalid() )
            otherEntrys.push_back(DylibIndexTrie::Entry(image->path(), DylibIndex(image->imageNum())));
    });
    DylibIndexTrie dylibsTrie(otherEntrys);
    std::vector<uint8_t> trieBytes;
    dylibsTrie.emit(trieBytes);
    while ( (trieBytes.size() % 4) != 0 )
        trieBytes.push_back(0);

    // check for fit
    uint64_t imageArraySize = otherImageArray->size();
    size_t freeSpace = _readOnlyRegion.bufferSize - _readOnlyRegion.sizeInUse;
    if ( imageArraySize+trieBytes.size() > freeSpace ) {
        _diagnostics.error("cache buffer too small to hold ImageArray and Trie (buffer size=%lldMB, imageArray size=%lldMB, trie size=%luKB, free space=%ldMB)",
                           _allocatedBufferSize/1024/1024, imageArraySize/1024/1024, trieBytes.size()/1024, freeSpace/1024/1024);
        return;
    }

    // copy into cache and update header
    DyldSharedCache* dyldCache = (DyldSharedCache*)_readExecuteRegion.buffer;
    dyldCache->header.otherImageArrayAddr = _readOnlyRegion.unslidLoadAddress + _readOnlyRegion.sizeInUse;
    dyldCache->header.otherImageArraySize = imageArraySize;
    dyldCache->header.otherTrieAddr       = dyldCache->header.otherImageArrayAddr + imageArraySize;
    dyldCache->header.otherTrieSize       = trieBytes.size();
    ::memcpy(_readOnlyRegion.buffer + _readOnlyRegion.sizeInUse, otherImageArray, imageArraySize);
    ::memcpy(_readOnlyRegion.buffer + _readOnlyRegion.sizeInUse + imageArraySize, &trieBytes[0], trieBytes.size());
    _readOnlyRegion.sizeInUse += align(imageArraySize+trieBytes.size(),14);

    // Free the underlying buffer
    otherImageArray->deallocate();
}


void CacheBuilder::addClosures(const std::vector<LoadedMachO>& osExecutables)
{
    const DyldSharedCache* dyldCache = (DyldSharedCache*)_readExecuteRegion.buffer;

    __block std::vector<Diagnostics> osExecutablesDiags;
    __block std::vector<const dyld3::closure::LaunchClosure*> osExecutablesClosures;
    osExecutablesDiags.resize(osExecutables.size());
    osExecutablesClosures.resize(osExecutables.size());

    dispatch_apply(osExecutables.size(), DISPATCH_APPLY_AUTO, ^(size_t index) {
        const LoadedMachO& loadedMachO = osExecutables[index];
        // don't pre-build closures for staged apps into dyld cache, since they won't run from that location
        if ( startsWith(loadedMachO.mappedFile.runtimePath, "/private/var/staged_system_apps/") ) {
            return;
        }
        dyld3::closure::PathOverrides pathOverrides;
        dyld3::closure::ClosureBuilder builder(dyld3::closure::kFirstLaunchClosureImageNum, _fileSystem, dyldCache, false, *_options.archs, pathOverrides,
                                               dyld3::closure::ClosureBuilder::AtPath::all, false, nullptr, _options.platform, nullptr);
        bool issetuid = false;
        if ( this->_options.platform == dyld3::Platform::macOS || dyld3::MachOFile::isSimulatorPlatform(this->_options.platform) )
            _fileSystem.fileExists(loadedMachO.loadedFileInfo.path, nullptr, nullptr, &issetuid, nullptr);
        const dyld3::closure::LaunchClosure* mainClosure = builder.makeLaunchClosure(loadedMachO.loadedFileInfo, issetuid);
        if ( builder.diagnostics().hasError() ) {
           osExecutablesDiags[index].error("%s", builder.diagnostics().errorMessage().c_str());
        }
        else {
            assert(mainClosure != nullptr);
            osExecutablesClosures[index] = mainClosure;
        }
    });

    std::map<std::string, const dyld3::closure::LaunchClosure*> closures;
    for (uint64_t i = 0, e = osExecutables.size(); i != e; ++i) {
        const LoadedMachO& loadedMachO = osExecutables[i];
        const Diagnostics& diag = osExecutablesDiags[i];
        if (diag.hasError()) {
            if ( _options.verbose ) {
                _diagnostics.warning("building closure for '%s': %s", loadedMachO.mappedFile.runtimePath.c_str(), diag.errorMessage().c_str());
                for (const std::string& warn : diag.warnings() )
                    _diagnostics.warning("%s", warn.c_str());
            }
            if ( loadedMachO.inputFile && (loadedMachO.inputFile->mustBeIncluded()) ) {
                loadedMachO.inputFile->diag.error("%s", diag.errorMessage().c_str());
            }
        } else {
            // Note, a closure could be null here if it has a path we skip.
            if (osExecutablesClosures[i] != nullptr)
                closures[loadedMachO.mappedFile.runtimePath] = osExecutablesClosures[i];
        }
    }

    osExecutablesDiags.clear();
    osExecutablesClosures.clear();

    // preflight space needed
    size_t closuresSpace = 0;
    for (const auto& entry : closures) {
        closuresSpace += entry.second->size();
    }
    size_t freeSpace = _readOnlyRegion.bufferSize - _readOnlyRegion.sizeInUse;
    if ( closuresSpace > freeSpace ) {
        _diagnostics.error("cache buffer too small to hold all closures (buffer size=%lldMB, closures size=%ldMB, free space=%ldMB)",
                            _allocatedBufferSize/1024/1024, closuresSpace/1024/1024, freeSpace/1024/1024);
        return;
    }
    DyldSharedCache* cache = (DyldSharedCache*)_readExecuteRegion.buffer;
    cache->header.progClosuresAddr = _readOnlyRegion.unslidLoadAddress + _readOnlyRegion.sizeInUse;
    uint8_t* closuresBase = _readOnlyRegion.buffer + _readOnlyRegion.sizeInUse;
    std::vector<DylibIndexTrie::Entry> closureEntrys;
    uint32_t currentClosureOffset = 0;
    for (const auto& entry : closures) {
        const dyld3::closure::LaunchClosure* closure = entry.second;
        closureEntrys.push_back(DylibIndexTrie::Entry(entry.first, DylibIndex(currentClosureOffset)));
        size_t size = closure->size();
        assert((size % 4) == 0);
        memcpy(closuresBase+currentClosureOffset, closure, size);
        currentClosureOffset += size;
        freeSpace -= size;
        closure->deallocate();
    }
    cache->header.progClosuresSize = currentClosureOffset;
    _readOnlyRegion.sizeInUse += currentClosureOffset;
    freeSpace = _readOnlyRegion.bufferSize - _readOnlyRegion.sizeInUse;
    // build trie of indexes into closures list
    DylibIndexTrie closureTrie(closureEntrys);
    std::vector<uint8_t> trieBytes;
    closureTrie.emit(trieBytes);
    while ( (trieBytes.size() % 8) != 0 )
        trieBytes.push_back(0);
    if ( trieBytes.size() > freeSpace ) {
        _diagnostics.error("cache buffer too small to hold all closures trie (buffer size=%lldMB, trie size=%ldMB, free space=%ldMB)",
                            _allocatedBufferSize/1024/1024, trieBytes.size()/1024/1024, freeSpace/1024/1024);
        return;
    }
    memcpy(_readOnlyRegion.buffer + _readOnlyRegion.sizeInUse, &trieBytes[0], trieBytes.size());
    cache->header.progClosuresTrieAddr = _readOnlyRegion.unslidLoadAddress + _readOnlyRegion.sizeInUse;
    cache->header.progClosuresTrieSize = trieBytes.size();
    _readOnlyRegion.sizeInUse += trieBytes.size();
    _readOnlyRegion.sizeInUse = align(_readOnlyRegion.sizeInUse, 14);
}


bool CacheBuilder::writeCache(void (^cacheSizeCallback)(uint64_t size), bool (^copyCallback)(const uint8_t* src, uint64_t size, uint64_t dstOffset))
{
    const dyld_cache_header*       cacheHeader = (dyld_cache_header*)_readExecuteRegion.buffer;
    const dyld_cache_mapping_info* mappings = (dyld_cache_mapping_info*)(_readExecuteRegion.buffer + cacheHeader->mappingOffset);
    assert(_readExecuteRegion.sizeInUse       == mappings[0].size);
    assert(_readWriteRegion.sizeInUse         == mappings[1].size);
    assert(_readOnlyRegion.sizeInUse          == mappings[2].size);
    assert(_readExecuteRegion.cacheFileOffset == mappings[0].fileOffset);
    assert(_readWriteRegion.cacheFileOffset   == mappings[1].fileOffset);
    assert(_readOnlyRegion.cacheFileOffset    == mappings[2].fileOffset);
    assert(_codeSignatureRegion.sizeInUse     == cacheHeader->codeSignatureSize);
    assert(cacheHeader->codeSignatureOffset   == mappings[2].fileOffset+_readOnlyRegion.sizeInUse+_localSymbolsRegion.sizeInUse);
    cacheSizeCallback(_readExecuteRegion.sizeInUse+_readWriteRegion.sizeInUse+_readOnlyRegion.sizeInUse+_localSymbolsRegion.sizeInUse+_codeSignatureRegion.sizeInUse);
    bool fullyWritten = copyCallback(_readExecuteRegion.buffer, _readExecuteRegion.sizeInUse, mappings[0].fileOffset);
    fullyWritten &= copyCallback(_readWriteRegion.buffer, _readWriteRegion.sizeInUse, mappings[1].fileOffset);
    fullyWritten &= copyCallback(_readOnlyRegion.buffer, _readOnlyRegion.sizeInUse, mappings[2].fileOffset);
    if ( _localSymbolsRegion.sizeInUse != 0 ) {
        assert(cacheHeader->localSymbolsOffset == mappings[2].fileOffset+_readOnlyRegion.sizeInUse);
        fullyWritten &= copyCallback(_localSymbolsRegion.buffer, _localSymbolsRegion.sizeInUse, cacheHeader->localSymbolsOffset);
    }
    fullyWritten &= copyCallback(_codeSignatureRegion.buffer, _codeSignatureRegion.sizeInUse, cacheHeader->codeSignatureOffset);
    return fullyWritten;
}


void CacheBuilder::writeFile(const std::string& path)
{
    std::string pathTemplate = path + "-XXXXXX";
    size_t templateLen = strlen(pathTemplate.c_str())+2;
    BLOCK_ACCCESSIBLE_ARRAY(char, pathTemplateSpace, templateLen);
    strlcpy(pathTemplateSpace, pathTemplate.c_str(), templateLen);
    int fd = mkstemp(pathTemplateSpace);
    if ( fd != -1 ) {
        auto cacheSizeCallback = ^(uint64_t size) {
            // if making macOS dyld cache for current OS into standard location
            if ( (_options.platform == dyld3::Platform::macOS) && startsWith(path, MACOSX_DYLD_SHARED_CACHE_DIR) ) {
                // <rdar://48687550> pin cache file to SSD on fusion drives
                apfs_data_pin_location_t where = APFS_PIN_DATA_TO_MAIN;
                ::fsctl(pathTemplateSpace, APFSIOC_PIN_DATA, &where, 0);
            }
            // set final cache file size (may help defragment file)
            ::ftruncate(fd, size);
        };
        auto copyCallback = ^(const uint8_t* src, uint64_t size, uint64_t dstOffset) {
            uint64_t writtenSize = pwrite(fd, src, size, dstOffset);
            return writtenSize == size;
        };
        // <rdar://problem/55370916> TOCTOU: verify path is still a realpath (not changed)
        char tempPath[MAXPATHLEN];
        if ( ::fcntl(fd, F_GETPATH, tempPath) == 0 ) {
            size_t tempPathLen = strlen(tempPath);
            if ( tempPathLen > 7 )
                tempPath[tempPathLen-7] = '\0'; // remove trailing -xxxxxx
            if ( path != tempPath ) {
                _diagnostics.error("output file path changed from: '%s' to: '%s'", path.c_str(), tempPath);
                ::close(fd);
                return;
            }
        }
        else {
            _diagnostics.error("unable to fcntl(fd, F_GETPATH) on output file");
            ::close(fd);
            return;
        }
        bool fullyWritten = writeCache(cacheSizeCallback, copyCallback);
        if ( fullyWritten ) {
            ::fchmod(fd, S_IRUSR|S_IRGRP|S_IROTH); // mkstemp() makes file "rw-------", switch it to "r--r--r--"
            // <rdar://problem/55370916> TOCTOU: verify path is still a realpath (not changed)
            char resolvedPath[PATH_MAX];
            ::realpath(path.c_str(), resolvedPath);
            // Note: if the target cache file does not already exist, realpath() will return NULL, but still fill in the path buffer
            if ( path != resolvedPath ) {
                _diagnostics.error("output file path changed from: '%s' to: '%s'", path.c_str(), resolvedPath);
                return;
            }
            if ( ::rename(pathTemplateSpace, path.c_str()) == 0) {
                ::close(fd);
                return; // success
            }
        }
        else {
            _diagnostics.error("could not write file %s", pathTemplateSpace);
        }
        ::close(fd);
        ::unlink(pathTemplateSpace);
    }
    else {
        _diagnostics.error("could not open file %s", pathTemplateSpace);
    }
}

void CacheBuilder::writeBuffer(uint8_t*& buffer, uint64_t& bufferSize) {
    auto cacheSizeCallback = ^(uint64_t size) {
        buffer = (uint8_t*)malloc(size);
        bufferSize = size;
    };
    auto copyCallback = ^(const uint8_t* src, uint64_t size, uint64_t dstOffset) {
        memcpy(buffer + dstOffset, src, size);
        return true;
    };
    bool fullyWritten = writeCache(cacheSizeCallback, copyCallback);
    assert(fullyWritten);
}

void CacheBuilder::writeMapFile(const std::string& path)
{
    const DyldSharedCache* cache = (DyldSharedCache*)_readExecuteRegion.buffer;
    std::string mapContent = cache->mapFile();
    safeSave(mapContent.c_str(), mapContent.size(), path);
}

std::string CacheBuilder::getMapFileBuffer(const std::string& cacheDisposition) const
{
    const DyldSharedCache* cache = (DyldSharedCache*)_readExecuteRegion.buffer;
    return cache->generateJSONMap(cacheDisposition.c_str());
}


void CacheBuilder::forEachCacheDylib(void (^callback)(const std::string& path)) {
    for (const DylibInfo& dylibInfo : _sortedDylibs)
        callback(dylibInfo.runtimePath);
}


CacheBuilder::ASLR_Tracker::~ASLR_Tracker()
{
    if ( _bitmap != nullptr )
        ::free(_bitmap);
}

void CacheBuilder::ASLR_Tracker::setDataRegion(const void* rwRegionStart, size_t rwRegionSize)
{
    _pageCount   = (unsigned)(rwRegionSize+_pageSize-1)/_pageSize;
    _regionStart = (uint8_t*)rwRegionStart;
    _endStart    = (uint8_t*)rwRegionStart + rwRegionSize;
    _bitmap      = (bool*)calloc(_pageCount*(_pageSize/4)*sizeof(bool), 1);
}

void CacheBuilder::ASLR_Tracker::add(void* loc)
{
    if (!_enabled)
        return;
    uint8_t* p = (uint8_t*)loc;
    assert(p >= _regionStart);
    assert(p < _endStart);
    _bitmap[(p-_regionStart)/4] = true;
}

void CacheBuilder::ASLR_Tracker::remove(void* loc)
{
    if (!_enabled)
        return;
    uint8_t* p = (uint8_t*)loc;
    assert(p >= _regionStart);
    assert(p < _endStart);
    _bitmap[(p-_regionStart)/4] = false;
}

bool CacheBuilder::ASLR_Tracker::has(void* loc)
{
    if (!_enabled)
        return true;
    uint8_t* p = (uint8_t*)loc;
    assert(p >= _regionStart);
    assert(p < _endStart);
    return _bitmap[(p-_regionStart)/4];
}


////////////////////////////  DylibTextCoalescer ////////////////////////////////////

bool CacheBuilder::DylibTextCoalescer::sectionWasCoalesced(std::string_view sectionName) const {
    if (sectionName.size() > 16)
        sectionName = sectionName.substr(0, 16);
    std::map<std::string_view, const DylibSectionOffsetToCacheSectionOffset*> supportedSections = {
        { "__objc_classname", &objcClassNames },
        { "__objc_methname", &objcMethNames },
        { "__objc_methtype", &objcMethTypes }
    };
    auto it = supportedSections.find(sectionName);
    if (it == supportedSections.end())
        return false;
    return !it->second->empty();
}

CacheBuilder::DylibTextCoalescer::DylibSectionOffsetToCacheSectionOffset& CacheBuilder::DylibTextCoalescer::getSectionCoalescer(std::string_view sectionName) {
    if (sectionName.size() > 16)
        sectionName = sectionName.substr(0, 16);
    std::map<std::string_view, DylibSectionOffsetToCacheSectionOffset*> supportedSections = {
        { "__objc_classname", &objcClassNames },
        { "__objc_methname", &objcMethNames },
        { "__objc_methtype", &objcMethTypes }
    };
    auto it = supportedSections.find(sectionName);
    assert(it != supportedSections.end());
    return *it->second;
}

const CacheBuilder::DylibTextCoalescer::DylibSectionOffsetToCacheSectionOffset& CacheBuilder::DylibTextCoalescer::getSectionCoalescer(std::string_view sectionName) const {
    if (sectionName.size() > 16)
        sectionName = sectionName.substr(0, 16);
    std::map<std::string_view, const DylibSectionOffsetToCacheSectionOffset*> supportedSections = {
        { "__objc_classname", &objcClassNames },
        { "__objc_methname", &objcMethNames },
        { "__objc_methtype", &objcMethTypes }
    };
    auto it = supportedSections.find(sectionName);
    assert(it != supportedSections.end());
    return *it->second;
}

////////////////////////////  CacheCoalescedText ////////////////////////////////////
const char* CacheBuilder::CacheCoalescedText::SupportedSections[] = {
    "__objc_classname",
    "__objc_methname",
    "__objc_methtype",
};

void CacheBuilder::CacheCoalescedText::parseCoalescableText(const dyld3::MachOAnalyzer *ma,
                                                            DylibTextCoalescer& textCoalescer) {
    static const bool log = false;

    // We can only remove sections if we know we have split seg v2 to point to it
    // Otherwise, a PC relative load in the __TEXT segment wouldn't know how to point to the new strings
    // which are no longer in the same segment
    uint32_t splitSegSize = 0;
    const void* splitSegStart = ma->getSplitSeg(splitSegSize);
    if (!splitSegStart)
        return;

    if ((*(const uint8_t*)splitSegStart) != DYLD_CACHE_ADJ_V2_FORMAT)
        return;

    // We can only remove sections from the end of a segment, so cache them all and walk backwards.
    __block std::vector<std::pair<std::string, dyld3::MachOAnalyzer::SectionInfo>> textSectionInfos;
    ma->forEachSection(^(const dyld3::MachOAnalyzer::SectionInfo &sectInfo, bool malformedSectionRange, bool &stop) {
        if (strcmp(sectInfo.segInfo.segName, "__TEXT") != 0)
            return;
        assert(!malformedSectionRange);
        textSectionInfos.push_back({ sectInfo.sectName, sectInfo });
    });

    const std::set<std::string_view> supportedSections(std::begin(SupportedSections), std::end(SupportedSections));
    int64_t slide = ma->getSlide();

    for (auto sectionInfoIt = textSectionInfos.rbegin(); sectionInfoIt != textSectionInfos.rend(); ++sectionInfoIt) {
        const std::string& sectionName = sectionInfoIt->first;
        const dyld3::MachOAnalyzer::SectionInfo& sectInfo = sectionInfoIt->second;

        // If we find a section we can't handle then stop here.  Hopefully we coalesced some from the end.
        if (supportedSections.find(sectionName) == supportedSections.end())
            break;

        StringSection& cacheStringSection = getSectionData(sectionName);

        DylibTextCoalescer::DylibSectionOffsetToCacheSectionOffset& sectionStringData = textCoalescer.getSectionCoalescer(sectionName);

        // Walk the strings in this section
        const uint8_t* content = (uint8_t*)(sectInfo.sectAddr + slide);
        const char* s   = (char*)content;
        const char* end = s + sectInfo.sectSize;
        while ( s < end ) {
            std::string_view str = s;
            auto itAndInserted = cacheStringSection.stringsToOffsets.insert({ str, cacheStringSection.bufferSize });
            if (itAndInserted.second) {
                // If we inserted the string then we need to include it in the total
                cacheStringSection.bufferSize += str.size() + 1;
                if (log)
                    printf("Selector: %s -> %s\n", ma->installName(), s);
            } else {
                // Debugging only.  If we didn't include the string then we saved that many bytes
                cacheStringSection.savedSpace += str.size() + 1;
            }

            // Now keep track of this offset in our source dylib as pointing to this offset
            uint32_t sourceSectionOffset = (uint32_t)((uint64_t)s - (uint64_t)content);
            uint32_t cacheSectionOffset = itAndInserted.first->second;
            sectionStringData[sourceSectionOffset] = cacheSectionOffset;
            s += str.size() + 1;
        }
    }
}

void CacheBuilder::CacheCoalescedText::clear() {
    *this = CacheBuilder::CacheCoalescedText();
}


CacheBuilder::CacheCoalescedText::StringSection& CacheBuilder::CacheCoalescedText::getSectionData(std::string_view sectionName) {
    if (sectionName.size() > 16)
        sectionName = sectionName.substr(0, 16);
    std::map<std::string_view, StringSection*> supportedSections = {
        { "__objc_classname", &objcClassNames },
        { "__objc_methname", &objcMethNames },
        { "__objc_methtype", &objcMethTypes }
    };
    auto it = supportedSections.find(sectionName);
    assert(it != supportedSections.end());
    return *it->second;
}


const CacheBuilder::CacheCoalescedText::StringSection& CacheBuilder::CacheCoalescedText::getSectionData(std::string_view sectionName) const {
    if (sectionName.size() > 16)
        sectionName = sectionName.substr(0, 16);
    std::map<std::string_view, const StringSection*> supportedSections = {
        { "__objc_classname", &objcClassNames },
        { "__objc_methname", &objcMethNames },
        { "__objc_methtype", &objcMethTypes }
    };
    auto it = supportedSections.find(sectionName);
    assert(it != supportedSections.end());
    return *it->second;
}

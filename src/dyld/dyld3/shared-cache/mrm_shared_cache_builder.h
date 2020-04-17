/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
 *
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

#ifndef mrm_shared_cache_builder_hpp
#define mrm_shared_cache_builder_hpp

#include <Availability.h>

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Note, this should match PLATFORM_* values in <mach-o/loader.h>
enum Platform {
    unknown             = 0,
    macOS               = 1,    // PLATFORM_MACOS
    iOS                 = 2,    // PLATFORM_IOS
    tvOS                = 3,    // PLATFORM_TVOS
    watchOS             = 4,    // PLATFORM_WATCHOS
    bridgeOS            = 5,    // PLATFORM_BRIDGEOS
    iOSMac              = 6,    // PLATFORM_IOSMAC
    iOS_simulator       = 7,    // PLATFORM_IOSIMULATOR
    tvOS_simulator      = 8,    // PLATFORM_TVOSSIMULATOR
    watchOS_simulator   = 9     // PLATFORM_WATCHOSSIMULATOR
};

enum Disposition
{
    Unknown                 = 0,
    InternalDevelopment     = 1,
    Customer                = 2,
    InternalMinDevelopment  = 3
};

enum FileFlags
{
    // Note these are for macho inputs
    NoFlags                                     = 0,
    MustBeInCache                               = 1,
    ShouldBeExcludedFromCacheIfUnusedLeaf       = 2,
    RequiredClosure                             = 3,

    // These are for the order files
    DylibOrderFile                              = 100,
    DirtyDataOrderFile                          = 101
};

struct BuildOptions_v1
{
    uint64_t                                    version;                        // Future proofing, set to 1
    const char *                                updateName;                     // BuildTrain+UpdateNumber
    const char *                                deviceName;
    enum Disposition                            disposition;                    // Internal, Customer, etc.
    enum Platform                               platform;                       // Enum: unknown, macOS, iOS, ...
    const char **                               archs;
    uint64_t                                    numArchs;
    bool                                        verboseDiagnostics;
    bool                                        isLocallyBuiltCache;
};

enum FileBehavior
{
    AddFile                                     = 0,        // New file: uid, gid, mode, data, cdhash fields must be set
    ChangeFile                                  = 1,        // Change the data of file: data, size, and cdhash fields must be set
};

struct FileResult
{
    uint64_t                                    version;            // Future proofing, set to 1
    const char*                                 path;
    enum FileBehavior                           behavior;
    const uint8_t*                              data;               // Owned by the cache builder.  Destroyed by destroySharedCacheBuilder
    uint64_t                                    size;
    // CDHash, must be set for new or modified files
    const char*                                 hashArch;
    const char*                                 hashType;
    const char*                                 hash;
};


struct CacheResult
{
    uint64_t                                    version;            // Future proofing, set to 1
    const char*                                 loggingPrefix;      // needed?
    const char*                                 deviceConfiguration;
    const char **                               warnings;           // should this be per-result?
    uint64_t                                    numWarnings;
    const char **                               errors;             // should this be per-result?
    uint64_t                                    numErrors;
    const char*                                 uuidString;
    const char*                                 mapJSON;
};

struct SharedCacheBuilder;

__API_AVAILABLE(macos(10.12))
void getVersion(uint32_t *major, uint32_t *minor);

__API_AVAILABLE(macos(10.12))
struct SharedCacheBuilder* createSharedCacheBuilder(const struct BuildOptions_v1* options);

// Add a file.  Returns true on success.
__API_AVAILABLE(macos(10.12))
bool addFile(struct SharedCacheBuilder* builder, const char* path, uint8_t* data, uint64_t size, enum FileFlags fileFlags);

__API_AVAILABLE(macos(10.12))
bool addSymlink(struct SharedCacheBuilder* builder, const char* fromPath, const char* toPath);

__API_AVAILABLE(macos(10.12))
bool runSharedCacheBuilder(struct SharedCacheBuilder* builder);

__API_AVAILABLE(macos(10.12))
const char* const* getErrors(const struct SharedCacheBuilder* builder, uint64_t* errorCount);

__API_AVAILABLE(macos(10.12))
const struct FileResult* const* getFileResults(struct SharedCacheBuilder* builder, uint64_t* resultCount);

__API_AVAILABLE(macos(10.12))
const struct CacheResult* const* getCacheResults(struct SharedCacheBuilder* builder, uint64_t* resultCount);

__API_AVAILABLE(macos(10.12))
const char* const* getFilesToRemove(const struct SharedCacheBuilder* builder, uint64_t* fileCount);

__API_AVAILABLE(macos(10.12))
void destroySharedCacheBuilder(struct SharedCacheBuilder* builder);

#ifdef __cplusplus
}
#endif

#endif /* mrm_shared_cache_builder_hpp */

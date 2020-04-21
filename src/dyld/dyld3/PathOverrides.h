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



#ifndef __DYLD_PATH_OVERRIDES_H__
#define __DYLD_PATH_OVERRIDES_H__

#include <stdint.h>

#include "Logging.h"
#include "MachOFile.h"


namespace dyld3 {
namespace closure {


class VIS_HIDDEN PathPool
{
public:
    static PathPool*    allocate();
    static void         deallocate(PathPool* pool);
    const char*         add(const char* path);
    void                forEachPath(void (^handler)(const char* path));

private:
    enum { kAllocationSize = 32*1024 };

    PathPool*       _next;
    char*           _current;
    size_t          _bytesFree;
    char            _buffer[];
};


class VIS_HIDDEN PathOverrides
{
public:
#if !BUILDING_LIBDYLD
    // libdyld is never unloaded
                                    ~PathOverrides();
#endif
    enum class FallbackPathMode { classic, restricted, none };

    void                            setFallbackPathHandling(FallbackPathMode mode);
    void                            setEnvVars(const char* envp[], const dyld3::MachOFile* mainExe, const char* mainExePath);
    void                            setMainExecutable(const dyld3::MachOFile* mainExe, const char* mainExePath);
    void                            forEachPathVariant(const char* requestedPath, bool pathIsInDyldCacheWhichCannotBeOverridden, void (^handler)(const char* possiblePath, bool isFallbackPath, bool& stop),
                                                       Platform plat=MachOFile::currentPlatform()) const;

    void                            forEachEnvVar(void (^handler)(const char* envVar)) const;
    void                            forEachExecutableEnvVar(void (^handler)(const char* envVar)) const;
    void                            forEachInsertedDylib(void (^handler)(const char* dylibPath, bool &stop)) const;

private:
    void                            setString(const char*& var, const char* value);
    const char*                     addString(const char* str);
    static void                     forEachInColonList(const char* list1, const char* list2, void (^callback)(const char* path, bool& stop));
    void                            addEnvVar(const char* keyEqualsValue, bool forExecutable);
    const char*                     getFrameworkPartialPath(const char* path) const;
    static const char*              getLibraryLeafName(const char* path);
    void                            handleListEnvVar(const char* key, const char** list, void (^handler)(const char* envVar)) const;
    void                            handleEnvVar(const char* key, const char* value, void (^handler)(const char* envVar)) const;
    void                            forEachDylibFallback(Platform platform, void (^handler)(const char* fallbackDir, bool& stop)) const;
    void                            forEachFrameworkFallback(Platform platform, void (^handler)(const char* fallbackDir, bool& stop)) const;
    void                            forEachImageSuffix(const char* path, bool isFallbackPath, bool pathIsInDyldCacheWhichCannotBeOverridden, bool& stop, void (^handler)(const char* possiblePath, bool isFallbackPath, bool& stop)) const;
    void                            addSuffix(const char* path, const char* suffix, char* result) const;

    PathPool*                       _pathPool                    = nullptr;
    const char*                     _dylibPathOverridesEnv       = nullptr;
    const char*                     _frameworkPathOverridesEnv   = nullptr;
    const char*                     _dylibPathFallbacksEnv       = nullptr;
    const char*                     _frameworkPathFallbacksEnv   = nullptr;
    const char*                     _dylibPathOverridesExeLC     = nullptr;
    const char*                     _frameworkPathOverridesExeLC = nullptr;
    const char*                     _dylibPathFallbacksExeLC     = nullptr;
    const char*                     _frameworkPathFallbacksExeLC = nullptr;
    const char*                     _insertedDylibs              = nullptr;
    const char*                     _imageSuffix                 = nullptr;
    const char*                     _rootPath                    = nullptr;  // simulator only
    FallbackPathMode                _fallbackPathMode            = FallbackPathMode::classic;
};

#if BUILDING_LIBDYLD
extern PathOverrides   gPathOverrides;
#endif


} // namespace closure
} // namespace dyld3

#endif // __DYLD_PATH_OVERRIDES_H__



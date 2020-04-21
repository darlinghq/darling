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



#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <uuid/uuid.h>
#include <mach/mach.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <limits.h>
#include <sys/errno.h>
#include <unistd.h>

#include <initializer_list>

#include "PathOverrides.h"



namespace dyld3 {
namespace closure {

#if BUILDING_LIBDYLD
PathOverrides   gPathOverrides;
#endif


// based on ANSI-C strstr()
static const char* strrstr(const char* str, const char* sub) 
{
    const size_t sublen = strlen(sub);
    for(const char* p = &str[strlen(str)]; p != str; --p) {
        if ( strncmp(p, sub, sublen) == 0 )
            return p;
    }
    return NULL;
}

    
void PathOverrides::setFallbackPathHandling(FallbackPathMode mode)
{
    _fallbackPathMode = mode;
}

void PathOverrides::setEnvVars(const char* envp[], const MachOFile* mainExe, const char* mainExePath)
{
    for (const char** p = envp; *p != NULL; p++) {
        addEnvVar(*p, false);
    }
    if ( mainExe != nullptr )
        setMainExecutable(mainExe, mainExePath);
}

void PathOverrides::setMainExecutable(const dyld3::MachOFile* mainExe, const char* mainExePath)
{
    assert(mainExe != nullptr);
    assert(mainExe->isMainExecutable());
    // process any LC_DYLD_ENVIRONMENT load commands in main executable
	mainExe->forDyldEnv(^(const char* envVar, bool& stop) {
        addEnvVar(envVar, true);
	});
}


#if !BUILDING_LIBDYLD
// libdyld is never unloaded
PathOverrides::~PathOverrides()
{
}
#endif

void PathOverrides::forEachInsertedDylib(void (^handler)(const char* dylibPath, bool &stop)) const
{
    if ( _insertedDylibs != nullptr ) {
        forEachInColonList(_insertedDylibs, nullptr, ^(const char* path, bool &stop) {
            handler(path, stop);
        });
    }
}

void PathOverrides::handleEnvVar(const char* key, const char* value, void (^handler)(const char* envVar)) const
{
    if ( value == nullptr )
        return;
    size_t allocSize = strlen(key) + strlen(value) + 2;
    char buffer[allocSize];
    strlcpy(buffer, key, allocSize);
    strlcat(buffer, "=", allocSize);
    strlcat(buffer, value, allocSize);
    handler(buffer);
}

// Note, this method only returns variables set on the environment, not those from the load command
void PathOverrides::forEachEnvVar(void (^handler)(const char* envVar)) const
{
    handleEnvVar("DYLD_LIBRARY_PATH",            _dylibPathOverridesEnv,        handler);
    handleEnvVar("DYLD_FRAMEWORK_PATH",          _frameworkPathOverridesEnv,    handler);
    handleEnvVar("DYLD_FALLBACK_FRAMEWORK_PATH", _frameworkPathFallbacksEnv,    handler);
    handleEnvVar("DYLD_FALLBACK_LIBRARY_PATH",   _dylibPathFallbacksEnv,        handler);
    handleEnvVar("DYLD_INSERT_LIBRARIES",        _insertedDylibs,               handler);
    handleEnvVar("DYLD_IMAGE_SUFFIX",            _imageSuffix,                  handler);
    handleEnvVar("DYLD_ROOT_PATH",               _rootPath,                     handler);
}

// Note, this method only returns variables set on the executable load command, not those from the environment
void PathOverrides::forEachExecutableEnvVar(void (^handler)(const char* envVar)) const
{
    handleEnvVar("DYLD_LIBRARY_PATH",            _dylibPathOverridesExeLC,        handler);
    handleEnvVar("DYLD_FRAMEWORK_PATH",          _frameworkPathOverridesExeLC,    handler);
    handleEnvVar("DYLD_FALLBACK_FRAMEWORK_PATH", _frameworkPathFallbacksExeLC,    handler);
    handleEnvVar("DYLD_FALLBACK_LIBRARY_PATH",   _dylibPathFallbacksExeLC,        handler);
}

const char* PathOverrides::addString(const char* str)
{
    if ( _pathPool == nullptr )
        _pathPool = PathPool::allocate();
    return _pathPool->add(str);
}

void PathOverrides::setString(const char*& var, const char* value)
{
    if ( var == nullptr ) {
        var = addString(value);
        return;
    }
    // string already in use, build new appended string
    char tmp[strlen(var)+strlen(value)+2];
    strcpy(tmp, var);
    strcat(tmp, ":");
    strcat(tmp, value);
    var = addString(tmp);
}

void PathOverrides::addEnvVar(const char* keyEqualsValue, bool forExecutable)
{
    // We have to make a copy of the env vars because the dyld
    // semantics is that the env vars are only looked at once
    // at launch (using setenv() at runtime does not change dyld behavior).
    const char* equals = strchr(keyEqualsValue, '=');
    if ( equals != NULL ) {
        if ( strncmp(keyEqualsValue, "DYLD_LIBRARY_PATH", 17) == 0 ) {
            setString(forExecutable ? _dylibPathOverridesExeLC : _dylibPathOverridesEnv, &keyEqualsValue[18]);
        }
        else if ( strncmp(keyEqualsValue, "DYLD_FRAMEWORK_PATH", 19) == 0 ) {
            setString(forExecutable ? _frameworkPathOverridesExeLC : _frameworkPathOverridesEnv, &keyEqualsValue[20]);
        }
        else if ( strncmp(keyEqualsValue, "DYLD_FALLBACK_FRAMEWORK_PATH", 28) == 0 ) {
            setString(forExecutable ? _frameworkPathFallbacksExeLC : _frameworkPathFallbacksEnv, &keyEqualsValue[29]);
        }
        else if ( strncmp(keyEqualsValue, "DYLD_FALLBACK_LIBRARY_PATH", 26) == 0 ) {
            setString(forExecutable ? _dylibPathFallbacksExeLC : _dylibPathFallbacksEnv, &keyEqualsValue[27]);
        }
        else if ( strncmp(keyEqualsValue, "DYLD_INSERT_LIBRARIES", 21) == 0 ) {
            setString(_insertedDylibs, &keyEqualsValue[22]);
        }
        else if ( strncmp(keyEqualsValue, "DYLD_IMAGE_SUFFIX", 17) == 0 ) {
            setString(_imageSuffix, &keyEqualsValue[18]);
        }
        else if ( strncmp(keyEqualsValue, "DYLD_ROOT_PATH", 14) == 0 ) {
            setString(_rootPath, &keyEqualsValue[15]);
        }
    }
}

void PathOverrides::forEachInColonList(const char* list1, const char* list2, void (^handler)(const char* path, bool& stop))
{
    for (const char* list : { list1, list2 }) {
        if (list == nullptr)
            continue;
        char buffer[strlen(list)+1];
        const char* t = list;
        bool stop = false;
        for (const char* s=list; *s != '\0'; ++s) {
            if (*s != ':')
                continue;
            size_t len = s - t;
            memcpy(buffer, t, len);
            buffer[len] = '\0';
            handler(buffer, stop);
            if ( stop )
                return;
            t = s+1;
        }
        handler(t, stop);
        if (stop)
            return;
    }
}

void PathOverrides::forEachDylibFallback(Platform platform, void (^handler)(const char* fallbackDir, bool& stop)) const
{
    __block bool stop = false;
    if ( (_dylibPathFallbacksEnv != nullptr) || (_dylibPathFallbacksExeLC != nullptr) ) {
        forEachInColonList(_dylibPathFallbacksEnv, _dylibPathFallbacksExeLC, ^(const char* pth, bool& innerStop) {
            handler(pth, innerStop);
            if ( innerStop )
                stop = true;
        });
    }
    else {
        switch ( platform ) {
            case Platform::macOS:
                switch ( _fallbackPathMode ) {
                    case FallbackPathMode::classic:
                        // "$HOME/lib"
                        handler("/usr/local/lib", stop);
                        if ( stop )
                            break;
                        [[clang::fallthrough]];
                    case FallbackPathMode::restricted:
                        handler("/usr/lib", stop);
                        break;
                    case FallbackPathMode::none:
                        break;
                }
                break;
            case Platform::iOS:
            case Platform::watchOS:
            case Platform::tvOS:
            case Platform::bridgeOS:
            case Platform::driverKit:
            case Platform::unknown:
                if ( _fallbackPathMode != FallbackPathMode::none ) {
                    handler("/usr/local/lib", stop);
                    if ( stop )
                        break;
                }
                // fall into /usr/lib case
                [[clang::fallthrough]];
            case Platform::iOSMac:
            case Platform::iOS_simulator:
            case Platform::watchOS_simulator:
            case Platform::tvOS_simulator:
                if ( _fallbackPathMode != FallbackPathMode::none )
                    handler("/usr/lib", stop);
                break;
        }
    }
}

void PathOverrides::forEachFrameworkFallback(Platform platform, void (^handler)(const char* fallbackDir, bool& stop)) const
{
    __block bool stop = false;
    if ( (_frameworkPathFallbacksEnv != nullptr) || (_frameworkPathFallbacksExeLC != nullptr) ) {
        forEachInColonList(_frameworkPathFallbacksEnv, _frameworkPathFallbacksExeLC, ^(const char* pth, bool& innerStop) {
            handler(pth, innerStop);
            if ( innerStop )
                stop = true;
        });
    }
    else {
        switch ( platform ) {
            case Platform::macOS:
                switch ( _fallbackPathMode ) {
                    case FallbackPathMode::classic:
                        // "$HOME/Library/Frameworks"
                        handler("/Library/Frameworks", stop);
                        if ( stop )
                            break;
                        // "/Network/Library/Frameworks"
                        // fall thru
                        [[clang::fallthrough]];
                    case FallbackPathMode::restricted:
                        handler("/System/Library/Frameworks", stop);
                        break;
                    case FallbackPathMode::none:
                        break;
                }
                break;
            case Platform::iOS:
            case Platform::watchOS:
            case Platform::tvOS:
            case Platform::bridgeOS:
            case Platform::iOSMac:
            case Platform::iOS_simulator:
            case Platform::watchOS_simulator:
            case Platform::tvOS_simulator:
            case Platform::driverKit:
            case Platform::unknown:
                if ( _fallbackPathMode != FallbackPathMode::none )
                    handler("/System/Library/Frameworks", stop);
                break;
        }
    }
}


//
// copy path and add suffix to result
//
//  /path/foo.dylib      _debug   =>   /path/foo_debug.dylib
//  foo.dylib            _debug   =>   foo_debug.dylib
//  foo                  _debug   =>   foo_debug
//  /path/bar            _debug   =>   /path/bar_debug
//  /path/bar.A.dylib    _debug   =>   /path/bar.A_debug.dylib
//
void PathOverrides::addSuffix(const char* path, const char* suffix, char* result) const
{
    strcpy(result, path);

    // find last slash
    char* start = strrchr(result, '/');
    if ( start != NULL )
        start++;
    else
        start = result;

    // find last dot after last slash
    char* dot = strrchr(start, '.');
    if ( dot != NULL ) {
        strcpy(dot, suffix);
        strcat(&dot[strlen(suffix)], &path[dot-result]);
    }
    else {
        strcat(result, suffix);
    }
}

void PathOverrides::forEachImageSuffix(const char* path, bool isFallbackPath, bool pathIsInDyldCacheWhichCannotBeOverridden, bool& stop, void (^handler)(const char* possiblePath, bool isFallbackPath, bool& stop)) const
{
    if ( (_imageSuffix == nullptr) || pathIsInDyldCacheWhichCannotBeOverridden ) {
        handler(path, isFallbackPath, stop);
    }
    else {
        forEachInColonList(_imageSuffix, nullptr, ^(const char* suffix, bool& innerStop) {
            char npath[strlen(path)+strlen(suffix)+8];
            addSuffix(path, suffix, npath);
            handler(npath, isFallbackPath, innerStop);
            if ( innerStop )
                stop = true;
        });
        if ( !stop )
            handler(path, isFallbackPath, stop);
    }
}

void PathOverrides::forEachPathVariant(const char* initialPath, bool pathIsInDyldCacheWhichCannotBeOverridden, void (^handler)(const char* possiblePath, bool isFallbackPath, bool& stop), Platform platform) const
{
    __block bool stop = false;


    if ( !pathIsInDyldCacheWhichCannotBeOverridden ) {
        // check for overrides
        const char* frameworkPartialPath = getFrameworkPartialPath(initialPath);
        if ( frameworkPartialPath != nullptr ) {
            const size_t frameworkPartialPathLen = strlen(frameworkPartialPath);
            // look at each DYLD_FRAMEWORK_PATH directory
            if ( (_frameworkPathOverridesEnv != nullptr) || (_frameworkPathOverridesExeLC != nullptr) ) {
                forEachInColonList(_frameworkPathOverridesEnv, _frameworkPathOverridesExeLC, ^(const char* frDir, bool &innerStop) {
                    char npath[strlen(frDir)+frameworkPartialPathLen+8];
                    strcpy(npath, frDir);
                    strcat(npath, "/");
                    strcat(npath, frameworkPartialPath);
                    forEachImageSuffix(npath, false, pathIsInDyldCacheWhichCannotBeOverridden, innerStop, handler);
                    if ( innerStop )
                        stop = true;
                });
            }
        }
        else {
            const char* libraryLeafName = getLibraryLeafName(initialPath);
            const size_t libraryLeafNameLen = strlen(libraryLeafName);
            // look at each DYLD_LIBRARY_PATH directory
            if ( (_dylibPathOverridesEnv != nullptr) || (_dylibPathOverridesExeLC != nullptr) ) {
                forEachInColonList(_dylibPathOverridesEnv, _dylibPathOverridesExeLC, ^(const char* libDir, bool &innerStop) {
                    char npath[strlen(libDir)+libraryLeafNameLen+8];
                    strcpy(npath, libDir);
                    strcat(npath, "/");
                    strcat(npath, libraryLeafName);
                    forEachImageSuffix(npath, false, pathIsInDyldCacheWhichCannotBeOverridden, innerStop, handler);
                    if ( innerStop )
                        stop = true;
                });
            }
        }
        if ( stop )
            return;
    }

    // try original path
    forEachImageSuffix(initialPath, false, pathIsInDyldCacheWhichCannotBeOverridden, stop, handler);
    if ( stop )
        return;

    // check fallback paths
    if ( const char* frameworkPartialPath = getFrameworkPartialPath(initialPath) ) {
        const size_t frameworkPartialPathLen = strlen(frameworkPartialPath);
        // look at each DYLD_FALLBACK_FRAMEWORK_PATH directory
        bool usesDefaultFallbackPaths = (_frameworkPathFallbacksEnv == nullptr) && (_frameworkPathFallbacksExeLC == nullptr);
        forEachFrameworkFallback(platform, ^(const char* dir, bool& innerStop) {
            char npath[strlen(dir)+frameworkPartialPathLen+8];
            strcpy(npath, dir);
            strcat(npath, "/");
            strcat(npath, frameworkPartialPath);
            forEachImageSuffix(npath, usesDefaultFallbackPaths, pathIsInDyldCacheWhichCannotBeOverridden, innerStop, handler);
            if ( innerStop )
                stop = true;
        });

    }
   else {
        const char* libraryLeafName = getLibraryLeafName(initialPath);
        const size_t libraryLeafNameLen = strlen(libraryLeafName);
        // look at each DYLD_FALLBACK_LIBRARY_PATH directory
        bool usesDefaultFallbackPaths = (_dylibPathFallbacksEnv == nullptr) && (_dylibPathFallbacksExeLC == nullptr);
        forEachDylibFallback(platform, ^(const char* dir, bool& innerStop) {
            char libpath[strlen(dir)+libraryLeafNameLen+8];
            strcpy(libpath, dir);
            strcat(libpath, "/");
            strcat(libpath, libraryLeafName);
            forEachImageSuffix(libpath, usesDefaultFallbackPaths, pathIsInDyldCacheWhichCannotBeOverridden, innerStop, handler);
            if ( innerStop )
                stop = true;
        });
    }
}


//
// Find framework path
//
//  /path/foo.framework/foo                             =>   foo.framework/foo    
//  /path/foo.framework/Versions/A/foo                  =>   foo.framework/Versions/A/foo
//  /path/foo.framework/Frameworks/bar.framework/bar    =>   bar.framework/bar
//  /path/foo.framework/Libraries/bar.dylb              =>   NULL
//  /path/foo.framework/bar                             =>   NULL
//
// Returns nullptr if not a framework path
//
const char* PathOverrides::getFrameworkPartialPath(const char* path) const
{
    const char* dirDot = strrstr(path, ".framework/");
    if ( dirDot != nullptr ) {
        const char* dirStart = dirDot;
        for ( ; dirStart >= path; --dirStart) {
            if ( (*dirStart == '/') || (dirStart == path) ) {
                const char* frameworkStart = &dirStart[1];
                if ( dirStart == path )
                    --frameworkStart;
                size_t len = dirDot - frameworkStart;
                char framework[len+1];
                strncpy(framework, frameworkStart, len);
                framework[len] = '\0';
                const char* leaf = strrchr(path, '/');
                if ( leaf != nullptr ) {
                    if ( strcmp(framework, &leaf[1]) == 0 ) {
                        return frameworkStart;
                    }
                    if (  _imageSuffix != nullptr ) {
                        // some debug frameworks have install names that end in _debug
                        if ( strncmp(framework, &leaf[1], len) == 0 ) {
                            if ( strcmp( _imageSuffix, &leaf[len+1]) == 0 )
                                return frameworkStart;
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}


const char* PathOverrides::getLibraryLeafName(const char* path)
{
    const char* start = strrchr(path, '/');
    if ( start != nullptr )
        return &start[1];
    else
        return path;
}



////////////////////////////  PathPool ////////////////////////////////////////


PathPool* PathPool::allocate()
{
    vm_address_t addr;
    ::vm_allocate(mach_task_self(), &addr, kAllocationSize, VM_FLAGS_ANYWHERE);
    PathPool* p = (PathPool*)addr;
    p->_next      = nullptr;
    p->_current   = &(p->_buffer[0]);
    p->_bytesFree = kAllocationSize - sizeof(PathPool);
    return p;
}

void PathPool::deallocate(PathPool* pool) {
    do {
        PathPool* next = pool->_next;
        ::vm_deallocate(mach_task_self(), (vm_address_t)pool, kAllocationSize);
        pool = next;
    } while (pool);
}

const char* PathPool::add(const char* path)
{
    size_t len = strlen(path) + 1;
    if ( len < _bytesFree ) {
        char* result = _current;
        strcpy(_current, path);
        _current += len;
        _bytesFree -= len;
        return result;
    }
    if ( _next == nullptr )
        _next = allocate();
    return _next->add(path);
}

void PathPool::forEachPath(void (^handler)(const char* path))
{
    for (const char* s = _buffer; s < _current; ++s) {
        handler(s);
        s += strlen(s);
    }

    if ( _next != nullptr )
        _next->forEachPath(handler);
}



} // namespace closure
} // namespace dyld3






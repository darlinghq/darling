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


#include <string.h>
#include <stdint.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <libc_private.h>
#include <TargetConditionals.h>
#include <_simple.h>
#include <mach-o/dyld_priv.h>
#include <mach-o/dyld_images.h>
#include <crt_externs.h> // FIXME: Remove once we move off of _NSGetMainExecutable()
#include <os/once_private.h>

#include <array>
#include <algorithm>

#include "dlfcn.h"

#include "AllImages.h"
#include "Loading.h"
#include "Logging.h"
#include "Diagnostics.h"
#include "DyldSharedCache.h"
#include "PathOverrides.h"
#include "APIs.h"
#include "Closure.h"
#include "MachOLoaded.h"
#include "ClosureBuilder.h"
#include "ClosureFileSystemPhysical.h"

#if __has_feature(ptrauth_calls)
#include <ptrauth.h>
#endif


namespace dyld {
    extern dyld_all_image_infos dyld_all_image_infos;
}


namespace dyld3 {


static const void *stripPointer(const void *ptr) {
#if __has_feature(ptrauth_calls)
    return __builtin_ptrauth_strip(ptr, ptrauth_key_asia);
#else
    return ptr;
#endif
}

pthread_mutex_t RecursiveAutoLock::_sMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;

// forward declaration
static void dyld_get_image_versions_internal(const struct mach_header* mh, void (^callback)(dyld_platform_t platform, uint32_t sdk_version, uint32_t min_version));


uint32_t _dyld_image_count(void)
{
    log_apis("_dyld_image_count()\n");

    return gAllImages.count();
}

const mach_header* _dyld_get_image_header(uint32_t imageIndex)
{
    log_apis("_dyld_get_image_header(%d)\n", imageIndex);
    return gAllImages.imageLoadAddressByIndex(imageIndex);
}

intptr_t _dyld_get_image_slide(const mach_header* mh)
{
    log_apis("_dyld_get_image_slide(%p)\n", mh);

    const MachOLoaded* mf = (MachOLoaded*)mh;
    if ( !mf->hasMachOMagic() )
        return 0;

    return mf->getSlide();
}

intptr_t _dyld_get_image_vmaddr_slide(uint32_t imageIndex)
{
    log_apis("_dyld_get_image_vmaddr_slide(%d)\n", imageIndex);

    const mach_header* mh = gAllImages.imageLoadAddressByIndex(imageIndex);
    if ( mh != nullptr )
        return dyld3::_dyld_get_image_slide(mh);
    return 0;
}

const char* _dyld_get_image_name(uint32_t imageIndex)
{
    log_apis("_dyld_get_image_name(%d)\n", imageIndex);
    return gAllImages.imagePathByIndex(imageIndex);
}


static bool nameMatch(const char* installName, const char* libraryName)
{
    const char* leafName = strrchr(installName, '/');
    if ( leafName == NULL )
        leafName = installName;
    else
        leafName++;

    // -framework case is exact match of leaf name
    if ( strcmp(leafName, libraryName) == 0 )
        return true;

    // -lxxx case: leafName must match "lib" <libraryName> ["." ?] ".dylib"
    size_t leafNameLen = strlen(leafName);
    size_t libraryNameLen = strlen(libraryName);
    if ( leafNameLen < (libraryNameLen+9) )
        return false;
    if ( strncmp(leafName, "lib", 3) != 0 )
        return false;
    if ( strcmp(&leafName[leafNameLen-6], ".dylib") != 0 )
        return false;
    if ( strncmp(&leafName[3], libraryName, libraryNameLen) != 0 )
        return false;
    return (leafName[libraryNameLen+3] == '.');
}


//
// BETTER, USE: dyld_get_program_sdk_version()
//
// Scans the main executable and returns the version of the specified dylib the program was built against.
//
// The library to find is the leaf name that would have been passed to linker tool
// (e.g. -lfoo or -framework foo would use "foo").
//
// Returns -1 if the main executable did not link against the specified library, or is malformed.
//
int32_t NSVersionOfLinkTimeLibrary(const char* libraryName)
{
    log_apis("NSVersionOfLinkTimeLibrary(\"%s\")\n", libraryName);

    __block int32_t result = -1;
    gAllImages.mainExecutable()->forEachDependentDylib(^(const char* loadPath, bool, bool, bool, uint32_t compatVersion, uint32_t currentVersion, bool& stop) {
        if ( nameMatch(loadPath, libraryName) )
            result = currentVersion;
    });
    log_apis("   NSVersionOfLinkTimeLibrary() => 0x%08X\n", result);
    return result;
}


//
// Searches loaded images for the requested dylib and returns its current version.
//
// The library to find is the leaf name that would have been passed to linker tool
// (e.g. -lfoo or -framework foo would use "foo").
//
// If the specified library is not loaded, -1 is returned.
//
int32_t NSVersionOfRunTimeLibrary(const char* libraryName)
{
    log_apis("NSVersionOfRunTimeLibrary(\"%s\")\n", libraryName);
    __block int32_t result = -1;
    gAllImages.forEachImage(^(const dyld3::LoadedImage& loadedImage, bool &stop) {
        const char* installName;
        uint32_t currentVersion;
        uint32_t compatVersion;
        if ( loadedImage.loadedAddress()->getDylibInstallName(&installName, &compatVersion, &currentVersion) && nameMatch(installName, libraryName) ) {
            result = currentVersion;
            stop = true;
        }
    });
    log_apis("   NSVersionOfRunTimeLibrary() => 0x%08X\n", result);
    return result;
}


uint32_t dyld_get_program_sdk_watch_os_version()
{
    log_apis("dyld_get_program_sdk_watch_os_version()\n");

    __block uint32_t retval = 0;
    __block bool versionFound = false;
    dyld3::dyld_get_image_versions(gAllImages.mainExecutable(), ^(dyld_platform_t platform, uint32_t sdk_version, uint32_t min_version) {
        if (versionFound) return;

        if (dyld_get_base_platform(platform) == PLATFORM_WATCHOS) {
            versionFound = true;
            retval = sdk_version;
        }
    });

    return retval;
}

uint32_t dyld_get_program_min_watch_os_version()
{
    log_apis("dyld_get_program_min_watch_os_version()\n");

    __block uint32_t retval = 0;
    __block bool versionFound = false;
    dyld3::dyld_get_image_versions(gAllImages.mainExecutable(), ^(dyld_platform_t platform, uint32_t sdk_version, uint32_t min_version) {
        if (versionFound) return;

        if (dyld_get_base_platform(platform) == PLATFORM_WATCHOS) {
            versionFound = true;
            retval = min_version;
        }
    });

    return retval;
}

uint32_t dyld_get_program_sdk_bridge_os_version()
{
   log_apis("dyld_get_program_sdk_bridge_os_version()\n");

    __block uint32_t retval = 0;
    __block bool versionFound = false;
    dyld3::dyld_get_image_versions(gAllImages.mainExecutable(), ^(dyld_platform_t platform, uint32_t sdk_version, uint32_t min_version) {
        if (versionFound) return;

        if (dyld_get_base_platform(platform) == PLATFORM_BRIDGEOS) {
            versionFound = true;
            retval = sdk_version;
        }
    });

    return retval;
}

uint32_t dyld_get_program_min_bridge_os_version()
{
    log_apis("dyld_get_program_min_bridge_os_version()\n");

    __block uint32_t retval = 0;
    __block bool versionFound = false;
    dyld3::dyld_get_image_versions(gAllImages.mainExecutable(), ^(dyld_platform_t platform, uint32_t sdk_version, uint32_t min_version) {
        if (versionFound) return;

        if (dyld_get_base_platform(platform) == PLATFORM_BRIDGEOS) {
            versionFound = true;
            retval = min_version;
        }
    });

    return retval;
 }

//
// Returns the sdk version (encode as nibble XXXX.YY.ZZ) that the
// specified binary was built against.
//
// First looks for LC_VERSION_MIN_* in binary and if sdk field is
// not zero, return that value.
// Otherwise, looks for the libSystem.B.dylib the binary linked
// against and uses a table to convert that to an sdk version.
//
uint32_t dyld_get_sdk_version(const mach_header* mh)
{
    log_apis("dyld_get_sdk_version(%p)\n", mh);
    __block bool versionFound = false;
    __block uint32_t retval = 0;
    dyld3::dyld_get_image_versions(mh, ^(dyld_platform_t platform, uint32_t sdk_version, uint32_t min_version) {
        if (versionFound) return;

        if (platform == ::dyld_get_active_platform()) {
            versionFound = true;
            switch (dyld3::dyld_get_base_platform(platform)) {
                case PLATFORM_BRIDGEOS: retval = sdk_version + 0x00090000; return;
                case PLATFORM_WATCHOS:  retval = sdk_version + 0x00070000; return;
                default: retval = sdk_version; return;
            }
        }
    });

    return retval;
}

uint32_t dyld_get_program_sdk_version()
{
	log_apis("dyld_get_program_sdk_version()\n");
    return dyld3::dyld_get_sdk_version(gAllImages.mainExecutable());
}

uint32_t dyld_get_min_os_version(const mach_header* mh)
{
    log_apis("dyld_get_min_os_version(%p)\n", mh);
    __block bool versionFound = false;
    __block uint32_t retval = 0;
    dyld3::dyld_get_image_versions(mh, ^(dyld_platform_t platform, uint32_t sdk_version, uint32_t min_version) {
        if (versionFound) return;

        if (platform == ::dyld_get_active_platform()) {
            versionFound = true;
            switch (dyld3::dyld_get_base_platform(platform)) {
                case PLATFORM_BRIDGEOS: retval = min_version + 0x00090000; return;
                case PLATFORM_WATCHOS:  retval = min_version + 0x00070000; return;
                default: retval = min_version; return;
            }
        }
    });

    return retval;
}

dyld_platform_t dyld_get_active_platform(void) {
    return gAllImages.platform();
}

dyld_platform_t dyld_get_base_platform(dyld_platform_t platform) {
    switch (platform) {
#ifdef PLATFORM_IOSMAC
        case PLATFORM_IOSMAC:               return PLATFORM_IOS;
#endif
        case PLATFORM_IOSSIMULATOR:         return PLATFORM_IOS;
        case PLATFORM_WATCHOSSIMULATOR:     return PLATFORM_WATCHOS;
        case PLATFORM_TVOSSIMULATOR:        return PLATFORM_TVOS;
        default:                            return platform;
    }
}

bool dyld_is_simulator_platform(dyld_platform_t platform) {
    switch(platform) {
        case PLATFORM_IOSSIMULATOR:
        case PLATFORM_WATCHOSSIMULATOR:
        case PLATFORM_TVOSSIMULATOR:
            return true;
        default:
            return false;
    }
}

bool dyld_sdk_at_least(const struct mach_header* mh, dyld_build_version_t version) {
    __block bool retval = false;
    dyld3::dyld_get_image_versions(mh, ^(dyld_platform_t platform, uint32_t sdk_version, uint32_t min_version) {
        if (dyld3::dyld_get_base_platform(platform) == version.platform && sdk_version >= version.version) {
            retval = true;
        }
    });
    return retval;
}

bool dyld_minos_at_least(const struct mach_header* mh, dyld_build_version_t version) {
    __block bool retval = false;
    dyld3::dyld_get_image_versions(mh, ^(dyld_platform_t platform, uint32_t sdk_version, uint32_t min_version) {
        if (dyld3::dyld_get_base_platform(platform) == version.platform && min_version >= version.version) {
            retval = true;
        }
    });
    return retval;
}

bool dyld_program_sdk_at_least(dyld_build_version_t version) {
    return dyld3::dyld_sdk_at_least(gAllImages.mainExecutable(), version);
}

bool dyld_program_minos_at_least(dyld_build_version_t version) {
    return dyld3::dyld_minos_at_least(gAllImages.mainExecutable(), version);
}

#if TARGET_OS_OSX || TARGET_OS_IOS
static
uint32_t linkedDylibVersion(const mach_header* mh, const char *installname) {
    __block uint32_t retval = 0;
    ((MachOLoaded*)mh)->forEachDependentDylib(^(const char* loadPath, bool, bool, bool, uint32_t compatVersion, uint32_t currentVersion, bool& stop) {
        if (strcmp(loadPath, installname) == 0) {
            retval = currentVersion;
            stop = true;
        }
    });
    return retval;
}
#endif


#define PACKED_VERSION(major, minor, tiny) ((((major) & 0xffff) << 16) | (((minor) & 0xff) << 8) | ((tiny) & 0xff))

static uint32_t deriveVersionFromDylibs(const struct mach_header* mh) {
    // This is a binary without a version load command, we need to infer things
    struct DylibToOSMapping {
        uint32_t dylibVersion;
        uint32_t osVersion;
    };
    uint32_t linkedVersion = 0;
#if TARGET_OS_OSX
    linkedVersion = linkedDylibVersion(mh, "/usr/lib/libSystem.B.dylib");
    static const DylibToOSMapping versionMapping[] = {
        { PACKED_VERSION(88,1,3),   0x000A0400 },
        { PACKED_VERSION(111,0,0),  0x000A0500 },
        { PACKED_VERSION(123,0,0),  0x000A0600 },
        { PACKED_VERSION(159,0,0),  0x000A0700 },
        { PACKED_VERSION(169,3,0),  0x000A0800 },
        { PACKED_VERSION(1197,0,0), 0x000A0900 },
        { PACKED_VERSION(0,0,0),    0x000A0900 }
        // We don't need to expand this table because all recent
        // binaries have LC_VERSION_MIN_ load command.
    };
#elif TARGET_OS_IOS
        linkedVersion = linkedDylibVersion(mh, "/System/Library/Frameworks/Foundation.framework/Foundation");
        static const DylibToOSMapping versionMapping[] = {
            { PACKED_VERSION(678,24,0), 0x00020000 },
            { PACKED_VERSION(678,26,0), 0x00020100 },
            { PACKED_VERSION(678,29,0), 0x00020200 },
            { PACKED_VERSION(678,47,0), 0x00030000 },
            { PACKED_VERSION(678,51,0), 0x00030100 },
            { PACKED_VERSION(678,60,0), 0x00030200 },
            { PACKED_VERSION(751,32,0), 0x00040000 },
            { PACKED_VERSION(751,37,0), 0x00040100 },
            { PACKED_VERSION(751,49,0), 0x00040200 },
            { PACKED_VERSION(751,58,0), 0x00040300 },
            { PACKED_VERSION(881,0,0),  0x00050000 },
            { PACKED_VERSION(890,1,0),  0x00050100 },
            { PACKED_VERSION(992,0,0),  0x00060000 },
            { PACKED_VERSION(993,0,0),  0x00060100 },
            { PACKED_VERSION(1038,14,0),0x00070000 },
            { PACKED_VERSION(0,0,0),    0x00070000 }
            // We don't need to expand this table because all recent
            // binaries have LC_VERSION_MIN_ load command.
    };
#else
    static const DylibToOSMapping versionMapping[] = {};
#endif
    if ( linkedVersion != 0 ) {
        uint32_t lastOsVersion = 0;
        for (const DylibToOSMapping* p=versionMapping; ; ++p) {
            if ( p->dylibVersion == 0 ) {
                return p->osVersion;
            }
            if ( linkedVersion < p->dylibVersion ) {
                return lastOsVersion;
            }
            lastOsVersion = p->osVersion;
        }
    }
    return 0;
}

// assumes mh has already been validated
static void dyld_get_image_versions_internal(const struct mach_header* mh, void (^callback)(dyld_platform_t platform, uint32_t sdk_version, uint32_t min_version))
{
    const MachOFile* mf = (MachOFile*)mh;
    __block bool lcFound = false;
    mf->forEachSupportedPlatform(^(dyld3::Platform platform, uint32_t minOS, uint32_t sdk) {
        lcFound = true;
        // If SDK field is empty then derive the value from library linkages
        if (sdk == 0) {
            sdk = deriveVersionFromDylibs(mh);
        }
        if (platform == dyld3::Platform::iOSMac) {
            sdk = 0x000A0F00;
        }
        callback((const dyld_platform_t)platform, sdk, minOS);
    });

    // No load command was found, so again, fallback to deriving it from library linkages
    if (!lcFound) {
#if TARGET_OS_IOS
#if __x86_64__ || __x86__
        dyld_platform_t platform = PLATFORM_IOSSIMULATOR;
#else
        dyld_platform_t platform = PLATFORM_IOS;
#endif
#elif TARGET_OS_OSX
        dyld_platform_t platform = PLATFORM_MACOS;
#else
        dyld_platform_t platform = 0;
#endif
        uint32_t derivedVersion = deriveVersionFromDylibs(mh);
        if ( platform != 0 && derivedVersion != 0 ) {
            callback(platform, derivedVersion, 0);
        }
    }
}

void dyld_get_image_versions(const struct mach_header* mh, void (^callback)(dyld_platform_t platform, uint32_t sdk_version, uint32_t min_version))
{
    Diagnostics diag;
    const MachOFile* mf = (MachOFile*)mh;
    static dyld_platform_t mainExecutablePlatform = 0;
    static uint32_t mainExecutableSDKVersion = 0;
    static uint32_t mainExecutableMinOSVersion = 0;

    // FIXME: Once dyld2 is gone gAllImages.mainExecutable() will be valid in all cases
    // and we can stop calling _NSGetMachExecuteHeader()
    if (mh == (const struct mach_header*)_NSGetMachExecuteHeader()) {
        // Cache the main executable and short circuit parsing the
        if (mainExecutablePlatform == 0) {
            dyld_get_image_versions_internal(mh, ^(dyld_platform_t platform, uint32_t sdk_version, uint32_t min_version) {
#if 0
                //FIXME: Reenable this once Libc supports dynamic platforms.
                if (platform == PLATFORM_MACOS && dyld_get_active_platform() == PLATFORM_IOSMAC) {
                    //FIXME: This version should be generated at link time
                    mainExecutablePlatform = PLATFORM_IOSMAC;
                    mainExecutableSDKVersion = 0x000D0000;
                    mainExecutableMinOSVersion = 0x000D0000;
                } else {
                    mainExecutablePlatform = platform;
                    mainExecutableSDKVersion = sdk_version;
                    mainExecutableMinOSVersion = min_version;
                }
#else
                mainExecutablePlatform = platform;
                mainExecutableSDKVersion = sdk_version;
                mainExecutableMinOSVersion = min_version;
#endif
                //FIXME: Assert if more than one command?
            });
        }
        return callback(mainExecutablePlatform, mainExecutableSDKVersion, mainExecutableMinOSVersion);
    }
#if TARGET_OS_EMBEDDED
    // If we are on embedded AND in the shared cache then the versions should be the same as libdyld
    if (mf->inDyldCache()) {
        static dyld_platform_t libDyldPlatform = 0;
        static uint32_t libDyldSDKVersion = 0;
        static uint32_t libDyldMinOSVersion = 0;
        if (libDyldPlatform == 0) {
            dyld_get_image_versions_internal(mh, ^(dyld_platform_t platform, uint32_t sdk_version, uint32_t min_version) {
                libDyldPlatform = platform;
                libDyldSDKVersion = sdk_version;
                libDyldMinOSVersion = min_version;
                //FIXME: Assert if more than one command?
            });
        }
        return callback(libDyldPlatform, libDyldSDKVersion, libDyldMinOSVersion);
    }
#endif
    if ( mf->isMachO(diag, mh->sizeofcmds + sizeof(mach_header_64)) )
        dyld_get_image_versions_internal(mh, callback);
}

uint32_t dyld_get_program_min_os_version()
{
    log_apis("dyld_get_program_min_os_version()\n");
    return dyld3::dyld_get_min_os_version(gAllImages.mainExecutable());
}

bool _dyld_get_image_uuid(const mach_header* mh, uuid_t uuid)
{
    log_apis("_dyld_get_image_uuid(%p, %p)\n", mh, uuid);

    const MachOFile* mf = (MachOFile*)mh;
    if ( !mf->hasMachOMagic() )
        return false;

    return mf->getUuid(uuid);
}

//
// _NSGetExecutablePath() copies the path of the main executable into the buffer. The bufsize parameter
// should initially be the size of the buffer.  The function returns 0 if the path was successfully copied,
// and *bufsize is left unchanged. It returns -1 if the buffer is not large enough, and *bufsize is set
// to the size required.
//
int _NSGetExecutablePath(char* buf, uint32_t* bufsize)
{
    log_apis("_NSGetExecutablePath(%p, %p)\n", buf, bufsize);

    const closure::Image* mainImage = gAllImages.mainExecutableImage();
    const char* path = gAllImages.imagePath(mainImage);
    size_t pathSize = strlen(path) + 1;
    if ( *bufsize >= pathSize ) {
        strcpy(buf, path);
        return 0;
    }
    *bufsize = (uint32_t)pathSize;
    return -1;
}

void _dyld_register_func_for_add_image(void (*func)(const mach_header *mh, intptr_t vmaddr_slide))
{
    log_apis("_dyld_register_func_for_add_image(%p)\n", func);

    gAllImages.addLoadNotifier(func);
}

void _dyld_register_func_for_remove_image(void (*func)(const mach_header *mh, intptr_t vmaddr_slide))
{
    log_apis("_dyld_register_func_for_remove_image(%p)\n", func);

    gAllImages.addUnloadNotifier(func);
}

void _dyld_objc_notify_register(_dyld_objc_notify_mapped    mapped,
                                _dyld_objc_notify_init      init,
                                _dyld_objc_notify_unmapped  unmapped)
{
    log_apis("_dyld_objc_notify_register(%p, %p, %p)\n", mapped, init, unmapped);

    gAllImages.setObjCNotifiers(mapped, init, unmapped);
}


const mach_header* dyld_image_header_containing_address(const void* addr)
{
    log_apis("dyld_image_header_containing_address(%p)\n", addr);

    addr = stripPointer(addr);

    const MachOLoaded* ml;
    if ( gAllImages.infoForImageMappedAt(addr, &ml, nullptr, nullptr) )
        return ml;

    return nullptr;
}


const char* dyld_image_path_containing_address(const void* addr)
{
    log_apis("dyld_image_path_containing_address(%p)\n", addr);

    addr = stripPointer(addr);
    const char* result = gAllImages.pathForImageMappedAt(addr);

    log_apis("   dyld_image_path_containing_address() => %s\n", result);
    return result;
}

    

bool _dyld_is_memory_immutable(const void* addr, size_t length)
{
    return gAllImages.immutableMemory(addr, length);
}

int dladdr(const void* addr, Dl_info* info)
{
    log_apis("dladdr(%p, %p)\n", addr, info);

    // <rdar://problem/42171466> calling dladdr(xx,NULL) crashes
    if ( info == NULL )
        return 0; // failure

    addr = stripPointer(addr);

    __block int         result = 0;
    const MachOLoaded*  ml     = nullptr;
    const char*         path   = nullptr;
    if ( gAllImages.infoForImageMappedAt(addr, &ml, nullptr, &path) ) {
        info->dli_fname = path;
        info->dli_fbase = (void*)ml;

        uint64_t symbolAddr;
        if ( addr == info->dli_fbase ) {
            // special case lookup of header
            info->dli_sname = "__dso_handle";
            info->dli_saddr = info->dli_fbase;
        }
        else if ( ml->findClosestSymbol((long)addr, &(info->dli_sname), &symbolAddr) ) {
            info->dli_saddr = (void*)(long)symbolAddr;
            // never return the mach_header symbol
            if ( info->dli_saddr == info->dli_fbase ) {
                info->dli_sname = nullptr;
                info->dli_saddr = nullptr;
            }
            // strip off leading underscore
            else if ( (info->dli_sname != nullptr) && (info->dli_sname[0] == '_') ) {
                info->dli_sname = info->dli_sname + 1;
            }
        }
        else {
            info->dli_sname = nullptr;
            info->dli_saddr = nullptr;
        }
        result = 1;
    }

    if ( result == 0 )
        log_apis("   dladdr() => 0\n");
    else
        log_apis("   dladdr() => 1, { \"%s\", %p, \"%s\", %p }\n", info->dli_fname, info->dli_fbase, info->dli_sname, info->dli_saddr);
    return result;
}

#if !TARGET_OS_DRIVERKIT

struct PerThreadErrorMessage
{
    size_t      sizeAllocated;
    bool        valid;
    char        message[1];
};

static void dlerror_perThreadKey_once(void* ctx)
{
    pthread_key_t* dlerrorPThreadKeyPtr = (pthread_key_t*)ctx;
    pthread_key_create(dlerrorPThreadKeyPtr, &free);
}

static pthread_key_t dlerror_perThreadKey()
{
    static os_once_t  onceToken;
    static pthread_key_t    dlerrorPThreadKey;
    os_once(&onceToken, &dlerrorPThreadKey, dlerror_perThreadKey_once);
    return dlerrorPThreadKey;
}

static void clearErrorString()
{
    PerThreadErrorMessage* errorBuffer = (PerThreadErrorMessage*)pthread_getspecific(dlerror_perThreadKey());
    if ( errorBuffer != nullptr )
        errorBuffer->valid = false;
}

__attribute__((format(printf, 1, 2)))
static void setErrorString(const char* format, ...)
{
    _SIMPLE_STRING buf = _simple_salloc();
    if ( buf != nullptr ) {
        va_list    list;
        va_start(list, format);
        _simple_vsprintf(buf, format, list);
        va_end(list);
        size_t strLen = strlen(_simple_string(buf)) + 1;
        size_t sizeNeeded = sizeof(PerThreadErrorMessage) + strLen;
        PerThreadErrorMessage* errorBuffer = (PerThreadErrorMessage*)pthread_getspecific(dlerror_perThreadKey());
        if ( errorBuffer != nullptr ) {
            if ( errorBuffer->sizeAllocated < sizeNeeded ) {
                free(errorBuffer);
                errorBuffer = nullptr;
            }
        }
        if ( errorBuffer == nullptr ) {
            size_t allocSize = std::max(sizeNeeded, (size_t)256);
            PerThreadErrorMessage* p = (PerThreadErrorMessage*)malloc(allocSize);
            p->sizeAllocated = allocSize;
            p->valid = false;
            pthread_setspecific(dlerror_perThreadKey(), p);
            errorBuffer = p;
        }
        strcpy(errorBuffer->message, _simple_string(buf));
        errorBuffer->valid = true;
        _simple_sfree(buf);
    }
}

char* dlerror()
{
    log_apis("dlerror()\n");

    PerThreadErrorMessage* errorBuffer = (PerThreadErrorMessage*)pthread_getspecific(dlerror_perThreadKey());
    if ( errorBuffer != nullptr ) {
        if ( errorBuffer->valid ) {
            // you can only call dlerror() once, then the message is cleared
            errorBuffer->valid = false;
            return errorBuffer->message;
        }
    }
    return nullptr;
}

#if __arm64__
    #define CURRENT_CPU_TYPE CPU_TYPE_ARM64
#elif __arm__
    #define CURRENT_CPU_TYPE CPU_TYPE_ARM
#endif


static void* makeDlHandle(const mach_header* mh, bool dontContinue)
{
    uintptr_t flags = (dontContinue ? 1 : 0);
    return (void*)((((uintptr_t)mh) >> 5) | flags);
}

VIS_HIDDEN
void parseDlHandle(void* h, const MachOLoaded** mh, bool* dontContinue)
{
    *dontContinue = (((uintptr_t)h) & 1);
    *mh           = (const MachOLoaded*)((((uintptr_t)h) & (-2)) << 5);
}

int dlclose(void* handle)
{
    DYLD_LOAD_LOCK_THIS_BLOCK
    log_apis("dlclose(%p)\n", handle);

    // silently accept magic handles for main executable
    if ( handle == RTLD_MAIN_ONLY )
        return 0;
    if ( handle == RTLD_DEFAULT )
        return 0;
    
    const MachOLoaded*  mh;
    bool                dontContinue;
    parseDlHandle(handle, &mh, &dontContinue);

    __block bool unloadable = false;
    __block bool validHandle = false;
    gAllImages.infoForImageMappedAt(mh, ^(const LoadedImage& foundImage, uint8_t permissions) {
        validHandle = true;
        if ( !foundImage.image()->neverUnload() )
            unloadable = true;
    });
    if ( unloadable ) {
        gAllImages.decRefCount(mh);  // removes image if reference count went to zero
    }

    if ( validHandle ) {
        clearErrorString();
        return 0;
    }
    else {
        setErrorString("invalid handle passed to dlclose()");
        return -1;
    }
}


void* dlopen_internal(const char* path, int mode, void* callerAddress)
{    
    DYLD_LOAD_LOCK_THIS_BLOCK
    log_apis("dlopen(\"%s\", 0x%08X)\n", ((path==NULL) ? "NULL" : path), mode);

    clearErrorString();

    // passing NULL for path means return magic object
    if ( path == NULL ) {
        // RTLD_FIRST means any dlsym() calls on the handle should only search that handle and not subsequent images
        if ( (mode & RTLD_FIRST) != 0 )
            return RTLD_MAIN_ONLY;
        else
            return RTLD_DEFAULT;
    }

    const char* leafName = strrchr(path, '/');
    if ( leafName != nullptr )
        ++leafName;
    else
        leafName = path;

#if __IPHONE_OS_VERSION_MIN_REQUIRED
    // <rdar://problem/40235395> dyld3: dlopen() not working with non-canonical paths
    char canonicalPath[PATH_MAX];
    if ( leafName != path ) {
        // make path canonical if it contains a // or ./
        if ( (strstr(path, "//") != NULL) || (strstr(path, "./") != NULL) ) {
            const char* lastSlash = strrchr(path, '/');
            char dirPath[PATH_MAX];
            if ( strlcpy(dirPath, path, sizeof(dirPath)) < sizeof(dirPath) ) {
                dirPath[lastSlash-path] = '\0';
                if ( realpath(dirPath, canonicalPath) ) {
                    strlcat(canonicalPath, "/", sizeof(canonicalPath));
                    if ( strlcat(canonicalPath, lastSlash+1, sizeof(canonicalPath)) < sizeof(canonicalPath) ) {
                        // if all fit in buffer, use new canonical path
                        path = canonicalPath;
                    }
                }
            }
        }
    }
#endif

    // RTLD_FIRST means when dlsym() is called with handle, only search the image and not those loaded after it
    const bool firstOnly = (mode & RTLD_FIRST);

    // RTLD_LOCAL means when flat searches of all images (e.g. RTLD_DEFAULT) is done, this image should be skipped. But dlsym(handle, xx) can find symbols
    const bool rtldLocal = (mode & RTLD_LOCAL);

    // RTLD_NODELETE means don't unmap image during dlclose(). Leave the memory mapped, but orphan (leak) it.
    // Note: this is a weird state and it slightly different semantics that other OSs
    const bool rtldNoDelete = (mode & RTLD_NODELETE);

    // RTLD_NOLOAD means do nothing if image not already loaded
    const bool rtldNoLoad = (mode & RTLD_NOLOAD);

    // RTLD_NOW means force lazy symbols bound and fail dlopen() if some cannot be bound
    const bool rtldNow = (mode & RTLD_NOW);

    // try to load image from specified path
    Diagnostics diag;
    const mach_header* topLoadAddress = gAllImages.dlopen(diag, path, rtldNoLoad, rtldLocal, rtldNoDelete, rtldNow, false, callerAddress);
    if ( diag.hasError() ) {
        setErrorString("dlopen(%s, 0x%04X): %s", path, mode, diag.errorMessage());
        log_apis("   dlopen: closure creation error: %s\n", diag.errorMessage());
        return nullptr;
    }
    if ( topLoadAddress == nullptr ) {
        log_apis("   dlopen(%s) => NULL\n", leafName);
        return nullptr;
    }
    void* result = makeDlHandle(topLoadAddress, firstOnly);
    log_apis("   dlopen(%s) => %p\n", leafName, result);
    return result;

}

bool dlopen_preflight_internal(const char* path)
{
    DYLD_LOAD_LOCK_THIS_BLOCK
    log_apis("dlopen_preflight(%s)\n", path);

    // check if path is in dyld shared cache
    if ( gAllImages.dyldCacheHasPath(path) )
        return true;

    // check if file is loadable
    Diagnostics diag;
    closure::FileSystemPhysical fileSystem;
    char realerPath[MAXPATHLEN];
    closure::LoadedFileInfo loadedFileInfo = MachOAnalyzer::load(diag, fileSystem, path, gAllImages.archs(), (Platform)gAllImages.platform(), realerPath);
    if ( loadedFileInfo.fileContent != nullptr ) {
        fileSystem.unloadFile(loadedFileInfo);
        return true;
    }

    // FIXME: may be symlink to something in dyld cache

    return false;
}

static void* dlsym_search(const char* symName, const LoadedImage& start, bool searchStartImage, MachOLoaded::DependentToMachOLoaded reExportHelper,
                          bool* resultPointsToInstructions)
{
    MachOLoaded::DependentToMachOLoaded finder = ^(const MachOLoaded* mh, uint32_t depIndex) {
        return gAllImages.findDependent(mh, depIndex);
    };
    //fprintf(stderr, "dlsym_search: %s, start=%s\n", symName, start.image()->path());

    // walk all dependents of 'start' in order looking for symbol
    __block void* result = nullptr;
    gAllImages.visitDependentsTopDown(start, ^(const LoadedImage& aLoadedImage, bool& stop) {
        //fprintf(stderr, "    search: %s\n", aLoadedImage.image()->path());
        if ( !searchStartImage && aLoadedImage.image() == start.image() )
            return;
        if ( aLoadedImage.loadedAddress()->hasExportedSymbol(symName, finder, &result, resultPointsToInstructions) ) {
            result = gAllImages.interposeValue(result);
            stop = true;
        }
    });

    return result;
}


void* dlsym_internal(void* handle, const char* symbolName, void* callerAddress)
{
    log_apis("dlsym(%p, \"%s\")\n", handle, symbolName);

    clearErrorString();

    MachOLoaded::DependentToMachOLoaded finder = ^(const MachOLoaded* mh, uint32_t depIndex) {
        return gAllImages.findDependent(mh, depIndex);
    };

    // dlsym() assumes symbolName passed in is same as in C source code
    // dyld assumes all symbol names have an underscore prefix
    BLOCK_ACCCESSIBLE_ARRAY(char, underscoredName, strlen(symbolName)+2);
    underscoredName[0] = '_';
    strcpy(&underscoredName[1], symbolName);

    __block void* result = nullptr;
    __block bool resultPointsToInstructions = false;
    if ( handle == RTLD_DEFAULT ) {
        // magic "search all in load order" handle
        gAllImages.forEachImage(^(const LoadedImage& loadedImage, bool& stop) {
            if ( loadedImage.hideFromFlatSearch() )
                return;
            if ( loadedImage.loadedAddress()->hasExportedSymbol(underscoredName, finder, &result, &resultPointsToInstructions) ) {
                stop = true;
            }
        });
        if ( result != nullptr ) {
            result = gAllImages.interposeValue(result);
#if __has_feature(ptrauth_calls)
            if (resultPointsToInstructions)
                result = __builtin_ptrauth_sign_unauthenticated(result, ptrauth_key_asia, 0);
#endif
            log_apis("   dlsym() => %p\n", result);
            return result;
        }
        setErrorString("dlsym(RTLD_DEFAULT, %s): symbol not found", symbolName);
        log_apis("   dlsym() => NULL\n");
        return nullptr;
    }
    else if ( handle == RTLD_MAIN_ONLY ) {
        // magic "search only main executable" handle
        if ( gAllImages.mainExecutable()->hasExportedSymbol(underscoredName, finder, &result, &resultPointsToInstructions) ) {
            result = gAllImages.interposeValue(result);
            log_apis("   dlsym() => %p\n", result);
#if __has_feature(ptrauth_calls)
            if (resultPointsToInstructions)
                result = __builtin_ptrauth_sign_unauthenticated(result, ptrauth_key_asia, 0);
#endif
            return result;
        }
        setErrorString("dlsym(RTLD_MAIN_ONLY, %s): symbol not found", symbolName);
        log_apis("   dlsym() => NULL\n");
        return nullptr;
    }
    // rest of cases search in dependency order
    if ( handle == RTLD_NEXT ) {
        // magic "search what I would see" handle
        __block bool foundCaller = false;
        gAllImages.infoForImageMappedAt(callerAddress, ^(const LoadedImage& foundImage, uint8_t permissions) {
            foundCaller = true;
            result = dlsym_search(underscoredName, foundImage, false, finder, &resultPointsToInstructions);
        });
        if ( !foundCaller ) {
            setErrorString("dlsym(RTLD_NEXT, %s): called by unknown image (caller=%p)", symbolName, callerAddress);
            return nullptr;
        }
    }
    else if ( handle == RTLD_SELF ) {
        // magic "search me, then what I would see" handle
        __block bool foundCaller = false;
        gAllImages.infoForImageMappedAt(callerAddress, ^(const LoadedImage& foundImage, uint8_t permissions) {
            foundCaller = true;
            result = dlsym_search(underscoredName, foundImage, true, finder, &resultPointsToInstructions);
        });
        if ( !foundCaller ) {
            setErrorString("dlsym(RTLD_SELF, %s): called by unknown image (caller=%p)", symbolName, callerAddress);
            return nullptr;
        }
    }
    else {
        // handle value was something returned by dlopen()
        const MachOLoaded*  mh;
        bool                dontContinue;
        parseDlHandle(handle, &mh, &dontContinue);

        __block bool foundCaller = false;
        gAllImages.infoForImageWithLoadAddress(mh, ^(const LoadedImage& foundImage) {
            foundCaller = true;
            if ( dontContinue ) {
                // RTLD_FIRST only searches one place
                // we go through infoForImageWithLoadAddress() to validate the handle
                if (mh->hasExportedSymbol(underscoredName, finder, &result, &resultPointsToInstructions))
                    result = gAllImages.interposeValue(result);
            }
            else {
                result = dlsym_search(underscoredName, foundImage, true, finder, &resultPointsToInstructions);
            }
        });
        if ( !foundCaller ) {
            setErrorString("dlsym(%p, %s): invalid handle", handle, symbolName);
            log_apis("   dlsym() => NULL\n");
            return nullptr;
        }
    }

    if ( result != nullptr ) {
#if __has_feature(ptrauth_calls)
        if (resultPointsToInstructions)
            result = __builtin_ptrauth_sign_unauthenticated(result, ptrauth_key_asia, 0);
#endif
        log_apis("   dlsym() => %p\n", result);
        return result;
    }

    setErrorString("dlsym(%p, %s): symbol not found", handle, symbolName);
    log_apis("   dlsym() => NULL\n");
    return nullptr;
}
#endif // !TARGET_OS_DRIVERKIT


const struct dyld_all_image_infos* _dyld_get_all_image_infos()
{
    return gAllImages.oldAllImageInfo();
}

bool dyld_shared_cache_some_image_overridden()
{
    log_apis("dyld_shared_cache_some_image_overridden()\n");

    return gAllImages.hasCacheOverrides();
}

bool _dyld_get_shared_cache_uuid(uuid_t uuid)
{
    log_apis("_dyld_get_shared_cache_uuid()\n");

    const DyldSharedCache* sharedCache = (DyldSharedCache*)gAllImages.cacheLoadAddress();
    if ( sharedCache == nullptr )
        return false;

    if ( gAllImages.oldAllImageInfo() != nullptr ) {
        memcpy(uuid, gAllImages.oldAllImageInfo()->sharedCacheUUID, sizeof(uuid_t));
        return true;
    }
    return false;
}

const void* _dyld_get_shared_cache_range(size_t* mappedSize)
{
    log_apis("_dyld_get_shared_cache_range()\n");

    const DyldSharedCache* sharedCache = (DyldSharedCache*)gAllImages.cacheLoadAddress();
    if ( sharedCache != nullptr ) {
        *mappedSize = (size_t)sharedCache->mappedSize();
        return sharedCache;
    }
    *mappedSize = 0;
    return NULL;
}

bool _dyld_shared_cache_optimized()
{
    const DyldSharedCache* sharedCache = (DyldSharedCache*)gAllImages.cacheLoadAddress();
    if ( sharedCache != nullptr ) {
        return (sharedCache->header.cacheType == kDyldSharedCacheTypeProduction);
    }
    return false;
}

bool _dyld_shared_cache_is_locally_built()
{
    const DyldSharedCache* sharedCache = (DyldSharedCache*)gAllImages.cacheLoadAddress();
    if ( sharedCache != nullptr ) {
        return (sharedCache->header.locallyBuiltCache == 1);
    }
    return false;
}

void _dyld_images_for_addresses(unsigned count, const void* addresses[], dyld_image_uuid_offset infos[])
{
    log_apis("_dyld_images_for_addresses(%u, %p, %p)\n", count, addresses, infos);

    // in stack crawls, common for contiguous fames to be in same image, so cache
    // last lookup and check if next addresss in in there before doing full search
    const MachOLoaded*    ml       = nullptr;
    uint64_t              textSize = 0;
    const void*           end      = (void*)ml;
    for (unsigned i=0; i < count; ++i) {
        const void* addr = stripPointer(addresses[i]);
        bzero(&infos[i], sizeof(dyld_image_uuid_offset));
        if ( (ml == nullptr) || (addr < (void*)ml) || (addr > end) ) {
            if ( gAllImages.infoForImageMappedAt(addr, &ml, &textSize, nullptr) ) {
                end = (void*)((uint8_t*)ml + textSize);
            }
            else {
                ml       = nullptr;
                textSize = 0;
            }
        }
        if ( ml != nullptr ) {
            infos[i].image         = ml;
            infos[i].offsetInImage = (uintptr_t)addr - (uintptr_t)ml;
            ml->getUuid(infos[i].uuid);
        }
    }
}

void _dyld_register_for_image_loads(void (*func)(const mach_header* mh, const char* path, bool unloadable))
{
    gAllImages.addLoadNotifier(func);
}

void _dyld_register_for_bulk_image_loads(void (*func)(unsigned imageCount, const struct mach_header* mhs[], const char* paths[]))
{
    gAllImages.addBulkLoadNotifier(func);
}

bool _dyld_find_unwind_sections(void* addr, dyld_unwind_sections* info)
{
    log_apis("_dyld_find_unwind_sections(%p, %p)\n", addr, info);
    addr = (void*)stripPointer(addr);

    const MachOLoaded* ml = nullptr;
    if ( gAllImages.infoForImageMappedAt(addr, &ml, nullptr, nullptr) ) {
        info->mh                            = ml;
        info->dwarf_section                 = nullptr;
        info->dwarf_section_length          = 0;
        info->compact_unwind_section        = nullptr;
        info->compact_unwind_section_length = 0;

        uint64_t size;
        if ( const void* content = ml->findSectionContent("__TEXT", "__eh_frame", size) ) {
            info->dwarf_section                 = content;
            info->dwarf_section_length          = (uintptr_t)size;
        }
        if ( const void* content = ml->findSectionContent("__TEXT", "__unwind_info", size) ) {
            info->compact_unwind_section        = content;
            info->compact_unwind_section_length = (uintptr_t)size;
        }
        return true;
    }

    return false;
}


bool dyld_process_is_restricted()
{
    log_apis("dyld_process_is_restricted()\n");
    return gAllImages.isRestricted();
}


const char* dyld_shared_cache_file_path()
{
    log_apis("dyld_shared_cache_file_path()\n");

    return gAllImages.dyldCachePath();
}


bool dyld_has_inserted_or_interposing_libraries()
{
   log_apis("dyld_has_inserted_or_interposing_libraries()\n");

   return gAllImages.hasInsertedOrInterposingLibraries();
}


void dyld_dynamic_interpose(const mach_header* mh, const dyld_interpose_tuple array[], size_t count)
{
    log_apis("dyld_dynamic_interpose(%p, %p, %lu)\n", mh, array, count);
    // FIXME
}


static void* mapStartOfCache(const char* path, size_t length)
{
    struct stat statbuf;
    if ( ::stat(path, &statbuf) == -1 )
        return NULL;

    if ( statbuf.st_size < length )
        return NULL;

    int cache_fd = ::open(path, O_RDONLY);
    if ( cache_fd < 0 )
        return NULL;

    void* result = ::mmap(NULL, length, PROT_READ, MAP_PRIVATE, cache_fd, 0);
    close(cache_fd);

    if ( result == MAP_FAILED )
        return NULL;

    return result;
}

static const DyldSharedCache* findCacheInDirAndMap(const uuid_t cacheUuid, const char* dirPath, size_t& sizeMapped)
{
    DIR* dirp = ::opendir(dirPath);
    if ( dirp != NULL) {
        dirent entry;
        dirent* entp = NULL;
        char cachePath[PATH_MAX];
        while ( ::readdir_r(dirp, &entry, &entp) == 0 ) {
            if ( entp == NULL )
                break;
            if ( entp->d_type != DT_REG ) 
                continue;
            if ( strlcpy(cachePath, dirPath, PATH_MAX) >= PATH_MAX )
                continue;
            if ( strlcat(cachePath, "/", PATH_MAX) >= PATH_MAX )
                continue;
            if ( strlcat(cachePath, entp->d_name, PATH_MAX) >= PATH_MAX )
                continue;
            if ( const DyldSharedCache* cache = (DyldSharedCache*)mapStartOfCache(cachePath, 0x00100000) ) {
                uuid_t foundUuid;
                cache->getUUID(foundUuid);
                if ( ::memcmp(foundUuid, cacheUuid, 16) != 0 ) {
                    // wrong uuid, unmap and keep looking
                    ::munmap((void*)cache, 0x00100000);
                }
                else {
                    // found cache
                    closedir(dirp);
                    sizeMapped = 0x00100000;
                    return cache;
                }
            }
        }
        closedir(dirp);
    }
    return nullptr;
}

int dyld_shared_cache_find_iterate_text(const uuid_t cacheUuid, const char* extraSearchDirs[], void (^callback)(const dyld_shared_cache_dylib_text_info* info))
{
    log_apis("dyld_shared_cache_find_iterate_text()\n");

    // see if requested cache is the active one in this process
    size_t sizeMapped = 0;
    const DyldSharedCache* sharedCache = (DyldSharedCache*)gAllImages.cacheLoadAddress();
    if ( sharedCache != nullptr ) {
        uuid_t runningUuid;
        sharedCache->getUUID(runningUuid);
        if ( ::memcmp(runningUuid, cacheUuid, 16) != 0 )
            sharedCache = nullptr;
    }
    if ( sharedCache == nullptr ) {
         // if not, look in default location for cache files
    #if    __IPHONE_OS_VERSION_MIN_REQUIRED
        const char* defaultSearchDir = IPHONE_DYLD_SHARED_CACHE_DIR;
    #else
        const char* defaultSearchDir = MACOSX_DYLD_SHARED_CACHE_DIR;
    #endif
        sharedCache = findCacheInDirAndMap(cacheUuid, defaultSearchDir, sizeMapped);
        // if not there, look in extra search locations
        if ( sharedCache == nullptr ) {
            for (const char** p = extraSearchDirs; *p != nullptr; ++p) {
                sharedCache = findCacheInDirAndMap(cacheUuid, *p, sizeMapped);
                if ( sharedCache != nullptr )
                    break;
            }
        }
    }
    if ( sharedCache == nullptr )
        return -1;

    // get base address of cache
    __block uint64_t cacheUnslidBaseAddress = 0;
    sharedCache->forEachRegion(^(const void *content, uint64_t vmAddr, uint64_t size, uint32_t permissions) {
        if ( cacheUnslidBaseAddress == 0 )
            cacheUnslidBaseAddress = vmAddr;
    });

    // iterate all images
    sharedCache->forEachImageTextSegment(^(uint64_t loadAddressUnslid, uint64_t textSegmentSize, const uuid_t dylibUUID, const char* installName, bool& stop) {
        dyld_shared_cache_dylib_text_info dylibTextInfo;
        dylibTextInfo.version              = 2;
        dylibTextInfo.loadAddressUnslid    = loadAddressUnslid;
        dylibTextInfo.textSegmentSize      = textSegmentSize;
        dylibTextInfo.path                 = installName;
        ::memcpy(dylibTextInfo.dylibUuid, dylibUUID, 16);
        dylibTextInfo.textSegmentOffset    = loadAddressUnslid - cacheUnslidBaseAddress;
        callback(&dylibTextInfo);
    });

    if ( sizeMapped != 0 )
        ::munmap((void*)sharedCache, sizeMapped);

    return 0;
}

int dyld_shared_cache_iterate_text(const uuid_t cacheUuid, void (^callback)(const dyld_shared_cache_dylib_text_info* info))
{
    log_apis("dyld_shared_cache_iterate_text()\n");

    const char* extraSearchDirs[] = { NULL };
    return dyld3::dyld_shared_cache_find_iterate_text(cacheUuid, extraSearchDirs, callback);
}
    
bool dyld_need_closure(const char* execPath, const char* tempDir)
{
    log_apis("dyld_need_closure()\n");

    // We don't need to build a closure if the shared cache has it already
    const DyldSharedCache* sharedCache = (DyldSharedCache*)gAllImages.cacheLoadAddress();
    if ( sharedCache != nullptr ) {
        if ( sharedCache->findClosure(execPath) != nullptr )
            return false;
    }


    char closurePath[PATH_MAX];
    if ( dyld3::closure::LaunchClosure::buildClosureCachePath(execPath, closurePath, tempDir, false) ) {
        struct stat statbuf;
        return (::stat(closurePath, &statbuf) != 0);
    }

    // Not containerized so no point in building a closure.
    return false;
}

void _dyld_missing_symbol_abort()
{
    // We don't know the name of the lazy symbol that is missing.
    // dyld3 binds all such missing symbols to this one handler.
    // We need the crash log to contain the backtrace so someone can
    // figure out the symbol.
    abort_report_np("missing lazy symbol called");
}

const char* _dyld_get_objc_selector(const char* selName)
{
    log_apis("dyld_get_objc_selector()\n");
    return gAllImages.getObjCSelector(selName);
}

void _dyld_for_each_objc_class(const char* className,
                               void (^callback)(void* classPtr, bool isLoaded, bool* stop)) {
    log_apis("_dyld_for_each_objc_class()\n");
    gAllImages.forEachObjCClass(className, callback);
}

void _dyld_for_each_objc_protocol(const char* protocolName,
                                  void (^callback)(void* protocolPtr, bool isLoaded, bool* stop)) {
    log_apis("_dyld_for_each_objc_protocol()\n");
    gAllImages.forEachObjCProtocol(protocolName, callback);
}

#if !TARGET_OS_DRIVERKIT
struct dyld_func {
    const char*  name;
    void*        implementation;
};

static const struct dyld_func dyld_funcs[] = {
    {"__dyld_dlsym",                    (void*)dlsym }, // needs to go through generic function to get caller address
    {"__dyld_dlopen",                   (void*)dlopen },// needs to go through generic function to get caller address
    {"__dyld_dladdr",                   (void*)dyld3::dladdr },
    {"__dyld_image_count",              (void*)dyld3::_dyld_image_count },
    {"__dyld_get_image_name",           (void*)dyld3::_dyld_get_image_name },
    {"__dyld_get_image_header",         (void*)dyld3::_dyld_get_image_header },
    {"__dyld_get_image_vmaddr_slide",   (void*)dyld3::_dyld_get_image_vmaddr_slide },
};
#endif

int compatFuncLookup(const char* name, void** address)
{
#if !TARGET_OS_DRIVERKIT
    for (const dyld_func* p = dyld_funcs; p->name != NULL; ++p) {
        if ( strcmp(p->name, name) == 0 ) {
            *address = p->implementation;
            return true;
        }
    }
    *address = 0;
#endif
    return false;
}



} // namespace dyld3


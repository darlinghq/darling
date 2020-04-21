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

#include <stdarg.h>
#include <mach-o/dyld_priv.h>
#include <mach-o/dyld_images.h>

#include "libdyldEntryVector.h"
#include "AllImages.h"
#include "Array.h"
#include "Loading.h"
#include "Logging.h"
#include "PathOverrides.h"
#include "StartGlue.h"
#include "dyld_process_info_internal.h"

extern "C" char start;

VIS_HIDDEN const char** appleParams;

extern bool gUseDyld3;

namespace dyld3 {


AllImages::ProgramVars sVars;
static void (*sChildForkFunction)();

static const char* leafName(const char* argv0)
{
    if ( argv0 == nullptr )
       return "";

    if ( const char* lastSlash = strrchr(argv0, '/') )
        return lastSlash+1;
    else
        return argv0;
}

static void entry_setVars(const mach_header* mainMH, int argc, const char* argv[], const char* envp[], const char* apple[])
{
    NXArgc       = argc;
    NXArgv       = argv;
    environ      = (char**)envp;
    appleParams  = apple;
    __progname   = leafName(argv[0]);

    sVars.mh            = mainMH;
    sVars.NXArgcPtr     = &NXArgc;
    sVars.NXArgvPtr     = &NXArgv;
    sVars.environPtr    = (const char***)&environ;
    sVars.__prognamePtr = &__progname;
    gAllImages.setProgramVars(&sVars);

    gUseDyld3 = true;

    setLoggingFromEnvs(envp);
}

static void entry_setHaltFunction(void (*func)(const char* message) __attribute__((noreturn)) )
{
    setHaltFunction(func);
}

static void entry_setLogFunction(void (*logFunction)(const char* format, va_list list))
{
    setLoggingFunction(logFunction);
}

static void entry_setOldAllImageInfo(dyld_all_image_infos* old)
{
    gAllImages.setOldAllImageInfo(old);
}

static void entry_setNotifyMonitoringDyldMain(void (*notifyMonitoringDyldMain)()) {
#if !TARGET_OS_DRIVERKIT
    setNotifyMonitoringDyldMain(notifyMonitoringDyldMain);
#endif
}

static void entry_setNotifyMonitoringDyld(void (*notifyMonitoringDyld)(bool unloading,unsigned imageCount,
                                                                               const struct mach_header* loadAddresses[],
                                                                               const char* imagePaths[])) {
#if !TARGET_OS_DRIVERKIT
    setNotifyMonitoringDyld(notifyMonitoringDyld);
#endif
}

static void entry_setInitialImageList(const closure::LaunchClosure* closure,
                                const DyldSharedCache* dyldCacheLoadAddress, const char* dyldCachePath,
                                const Array<LoadedImage>& initialImages, LoadedImage& libSystem)
{
    gAllImages.init(closure, dyldCacheLoadAddress, dyldCachePath, initialImages);
    gAllImages.applyInterposingToDyldCache(closure);

    // run initializer for libSytem.B.dylib
    // this calls back into _dyld_initializer which calls gAllIimages.addImages()
    gAllImages.runLibSystemInitializer(libSystem);

    // now that malloc is available, parse DYLD_ env vars
    closure::gPathOverrides.setEnvVars((const char**)environ, gAllImages.mainExecutable(), gAllImages.mainExecutableImage()->path());
}

static void entry_runInitialzersBottomUp(const mach_header* mainExecutableImageLoadAddress)
{
    gAllImages.runStartupInitialzers();
#if !TARGET_OS_DRIVERKIT
    gAllImages.notifyMonitorMain();
#endif
}

static void entry_setChildForkFunction(void (*func)() )
{
    sChildForkFunction = func;
}

static void entry_setRestrictions(bool allowAtPaths, bool allowEnvPaths, bool allowFallbackPaths)
{
    gAllImages.setRestrictions(allowAtPaths, allowEnvPaths);
    closure::gPathOverrides.setFallbackPathHandling(allowFallbackPaths ?
                                                    dyld3::closure::PathOverrides::FallbackPathMode::classic :
                                                    dyld3::closure::PathOverrides::FallbackPathMode::restricted);
}

static void entry_setHasCacheOverrides(bool someCacheImageOverriden)
{
    gAllImages.setHasCacheOverrides(someCacheImageOverriden);
}

static_assert((closure::kFormatVersion & LibDyldEntryVector::kBinaryFormatVersionMask) == closure::kFormatVersion, "binary format version overflow");

const LibDyldEntryVector entryVectorForDyld = {
    LibDyldEntryVector::kCurrentVectorVersion,
    closure::kFormatVersion,
    &entry_setVars,
    &entry_setHaltFunction,
    &entry_setOldAllImageInfo,
    &entry_setInitialImageList,
    &entry_runInitialzersBottomUp,
    (__typeof(LibDyldEntryVector::startFunc))address_of_start,
    &entry_setChildForkFunction,
    &entry_setLogFunction,
    &entry_setRestrictions,
    &entry_setNotifyMonitoringDyldMain,
    &entry_setNotifyMonitoringDyld,
    &entry_setHasCacheOverrides
};

VIS_HIDDEN void _dyld_atfork_prepare()
{
    gAllImages.takeLockBeforeFork();
}

VIS_HIDDEN void _dyld_atfork_parent()
{
    gAllImages.releaseLockInForkParent();
}

VIS_HIDDEN void _dyld_fork_child()
{
    // Note the child fork function updates the data structures inside dyld
    (*sChildForkFunction)();

    // And we then need to update the structures for dyld3 in libdyld
    gAllImages.resetLockInForkChild();
}


} // namespace dyld3


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



#ifndef __DYLD_ENTRY_VECTOR_H__
#define __DYLD_ENTRY_VECTOR_H__

#include <mach-o/loader.h>
#include <Availability.h>

#include "Loading.h"

struct dyld_all_image_infos;
class DyldSharedCache;

namespace dyld3 {


struct LibDyldEntryVector
{
    enum { kCurrentVectorVersion = 7 };
    // The 32-bit caches steal bits to make rebase chains, so use 32-bits for the binary format version storage, but mask only some to actually use
    enum { kBinaryFormatVersionMask = 0x00FFFFFF };

    uint32_t    vectorVersion;              // should be kCurrentVectorVersion
    uint32_t    binaryFormatVersion;        // should be dyld3::closure::kFormatVersion
    void        (*setVars)(const mach_header* mainMH, int argc, const char* argv[], const char* envp[], const char* apple[]);
    void        (*setHaltFunction)(void (*func)(const char* message) __attribute__((noreturn)) );
    void        (*setOldAllImageInfo)(dyld_all_image_infos*);
    void        (*setInitialImageList)(const closure::LaunchClosure* closure,
                                        const DyldSharedCache* dyldCacheLoadAddress, const char* dyldCachePath,
                                        const Array<LoadedImage>& initialImages, LoadedImage& libSystem);
    void        (*runInitialzersBottomUp)(const mach_header* topImageLoadAddress);
    void        (*startFunc)();
    // added in version 3
    void        (*setChildForkFunction)(void (*func)());
    // added in version 4
    void        (*setLogFunction)(void (*logFunction)(const char* format, va_list list));
    // added in version 5
    void        (*setRestrictions)(bool allowAtPaths, bool allowEnvVars, bool allowFallbackPaths);
    // added in version 6
    void        (*setNotifyMonitoringDyldMain)(void (*notifyMonitoringDyldMain)());
    void        (*setNotifyMonitoringDyld)(void (*notifyMonitoringDyldMain)(bool unloading, unsigned imageCount,
                                                                            const struct mach_header* loadAddresses[],
                                                                            const char* imagePaths[]));
    // added in version 7
    void        (*setHasCacheOverrides)(bool someCacheImageOverriden);
};

extern const LibDyldEntryVector entryVectorForDyld;

extern int compatFuncLookup(const char* name, void** address) __API_AVAILABLE(ios(13.0));

} // namespace dyld3


#endif // __DYLD_ENTRY_VECTOR_H__





/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _LIBKERN_OSKEXTLIB_H
#define _LIBKERN_OSKEXTLIB_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <stdint.h>
#include <mach/kmod.h>
#include <mach/vm_types.h>
#include <uuid/uuid.h>

#ifdef KERNEL
#include <libkern/OSTypes.h>
#include <libkern/OSReturn.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#include <libkern/OSReturn.h>
#endif /* KERNEL */

/*!
 * @header
 *
 * Declares functions, basic return values, and other constants
 * related to kernel extensions (kexts).
 */

#if PRAGMA_MARK
#pragma mark -
/********************************************************************/
#pragma mark OSReturn Values for Kernel Extensions
/********************************************************************/
#endif
/*!
 * @group OSReturn Values for Kernel Extensions
 * Many kext-related functions return these values,
 * as well as those defined under
 * <code>@link //apple_ref/c/tdef/OSReturn OSReturn@/link</code>
 * and other variants of <code>kern_return_t</code>.
 */

#ifdef XNU_KERNEL_PRIVATE
/*********************************************************************
* Check libsyscall/mach/err_libkern.sub when editing or adding
* result codes!
*********************************************************************/
#endif /* XNU_KERNEL_PRIVATE */

#define sub_libkern_kext           err_sub(2)
#define libkern_kext_err(code)     (sys_libkern|sub_libkern_kext|(code))


/*!
 * @define   kOSKextReturnInternalError
 * @abstract An internal error in the kext library.
 *           Contrast with <code>@link //apple_ref/c/econst/OSReturnError
 *           OSReturnError@/link</code>.
 */
#define kOSKextReturnInternalError                   libkern_kext_err(0x1)

/*!
 * @define   kOSKextReturnNoMemory
 * @abstract Memory allocation failed.
 */
#define kOSKextReturnNoMemory                        libkern_kext_err(0x2)

/*!
 * @define   kOSKextReturnNoResources
 * @abstract Some resource other than memory (such as available load tags)
 *           is exhausted.
 */
#define kOSKextReturnNoResources                     libkern_kext_err(0x3)

/*!
 * @define   kOSKextReturnNotPrivileged
 * @abstract The caller lacks privileges to perform the requested operation.
 */
#define kOSKextReturnNotPrivileged                   libkern_kext_err(0x4)

/*!
 * @define   kOSKextReturnInvalidArgument
 * @abstract Invalid argument.
 */
#define kOSKextReturnInvalidArgument                 libkern_kext_err(0x5)

/*!
 * @define   kOSKextReturnNotFound
 * @abstract Search item not found.
 */
#define kOSKextReturnNotFound                        libkern_kext_err(0x6)

/*!
 * @define   kOSKextReturnBadData
 * @abstract Malformed data (not used for XML).
 */
#define kOSKextReturnBadData                         libkern_kext_err(0x7)

/*!
 * @define   kOSKextReturnSerialization
 * @abstract Error converting or (un)serializing URL, string, or XML.
 */
#define kOSKextReturnSerialization                   libkern_kext_err(0x8)

/*!
 * @define   kOSKextReturnUnsupported
 * @abstract Operation is no longer or not yet supported.
 */
#define kOSKextReturnUnsupported                     libkern_kext_err(0x9)

/*!
 * @define   kOSKextReturnDisabled
 * @abstract Operation is currently disabled.
 */
#define kOSKextReturnDisabled                        libkern_kext_err(0xa)

/*!
 * @define   kOSKextReturnNotAKext
 * @abstract Bundle is not a kernel extension.
 */
#define kOSKextReturnNotAKext                        libkern_kext_err(0xb)

/*!
 * @define   kOSKextReturnValidation
 * @abstract Validation failures encountered; check diagnostics for details.
 */
#define kOSKextReturnValidation                      libkern_kext_err(0xc)

/*!
 * @define   kOSKextReturnAuthentication
 * @abstract Authetication failures encountered; check diagnostics for details.
 */
#define kOSKextReturnAuthentication                  libkern_kext_err(0xd)

/*!
 * @define   kOSKextReturnDependencies
 * @abstract Dependency resolution failures encountered; check diagnostics for details.
 */
#define kOSKextReturnDependencies                    libkern_kext_err(0xe)

/*!
 * @define   kOSKextReturnArchNotFound
 * @abstract Kext does not contain code for the requested architecture.
 */
#define kOSKextReturnArchNotFound                    libkern_kext_err(0xf)

/*!
 * @define   kOSKextReturnCache
 * @abstract An error occurred processing a system kext cache.
 */
#define kOSKextReturnCache                           libkern_kext_err(0x10)

/*!
 * @define   kOSKextReturnDeferred
 * @abstract Operation has been posted asynchronously to user space (kernel only).
 */
#define kOSKextReturnDeferred                        libkern_kext_err(0x11)

/*!
 * @define   kOSKextReturnBootLevel
 * @abstract Kext not loadable or operation not allowed at current boot level.
 */
#define kOSKextReturnBootLevel                       libkern_kext_err(0x12)

/*!
 * @define   kOSKextReturnNotLoadable
 * @abstract Kext cannot be loaded; check diagnostics for details.
 */
#define kOSKextReturnNotLoadable                     libkern_kext_err(0x13)

/*!
 * @define   kOSKextReturnLoadedVersionDiffers
 * @abstract A different version (or executable UUID, or executable by checksum)
 *           of the requested kext is already loaded.
 */
#define kOSKextReturnLoadedVersionDiffers            libkern_kext_err(0x14)

/*!
 * @define   kOSKextReturnDependencyLoadError
 * @abstract A load error occurred on a dependency of the kext being loaded.
 */
#define kOSKextReturnDependencyLoadError             libkern_kext_err(0x15)

/*!
 * @define   kOSKextReturnLinkError
 * @abstract A link failure occured with this kext or a dependency.
 */
#define kOSKextReturnLinkError                       libkern_kext_err(0x16)

/*!
 * @define   kOSKextReturnStartStopError
 * @abstract The kext start or stop routine returned an error.
 */
#define kOSKextReturnStartStopError                  libkern_kext_err(0x17)

/*!
 * @define   kOSKextReturnInUse
 * @abstract The kext is currently in use or has outstanding references,
 *           and cannot be unloaded.
 */
#define kOSKextReturnInUse                           libkern_kext_err(0x18)

/*!
 * @define   kOSKextReturnTimeout
 * @abstract A kext request has timed out.
 */
#define kOSKextReturnTimeout                         libkern_kext_err(0x19)

/*!
 * @define   kOSKextReturnStopping
 * @abstract The kext is in the process of stopping; requests cannot be made.
 */
#define kOSKextReturnStopping                        libkern_kext_err(0x1a)

/*!
 * @define   kOSKextReturnSystemPolicy
 * @abstract The kext was prevented from loading due to system policy.
 */
#define kOSKextReturnSystemPolicy                    libkern_kext_err(0x1b)

#if PRAGMA_MARK
#pragma mark -
/********************************************************************/
#pragma mark Kext/OSBundle Property List Keys
/********************************************************************/
#endif
/*!
 * @group Kext Property List Keys
 * These constants cover CFBundle properties defined for kernel extensions.
 * Because they are used in the kernel, if you want to use one with
 * CFBundle APIs you'll need to wrap it in a <code>CFSTR()</code> macro.
 */

#ifdef KERNEL
/* Define C-string versions of the CFBundle keys for use in the kernel.
 */
#define kCFBundleIdentifierKey                  "CFBundleIdentifier"
#define kCFBundleIdentifierKernelKey            "CFBundleIdentifierKernel"
#define kCFBundleVersionKey                     "CFBundleVersion"
#define kCFBundleNameKey                        "CFBundleName"
#define kCFBundleExecutableKey                  "CFBundleExecutable"
#define kCFBundlePackageTypeKey                 "CFBundlePackageType"
#define kCFBundleDriverKitUUIDKey               "CFBundleDriverKitUUID"
#endif /* KERNEL */

/*!
 * @define   kOSBundleCompatibleVersionKey
 * @abstract A string giving the backwards-compatible version of a library kext
 *           in extended Mac OS 'vers' format (####.##.##s{1-255} where 's'
 *           is a build stage 'd', 'a', 'b', 'f' or 'fc').
 */
#define kOSBundleCompatibleVersionKey           "OSBundleCompatibleVersion"

/*!
 * @define   kOSBundleEnableKextLoggingKey
 * @abstract Set to true to have the kernel kext logging spec applied
 *           to the kext.
 *           See <code>@link //apple_ref/c/econst/OSKextLogSpec
 *           OSKextLogSpec@/link</code>.
 */
#define kOSBundleEnableKextLoggingKey           "OSBundleEnableKextLogging"

/*!
 * @define   kOSBundleIsInterfaceKey
 * @abstract A boolean value indicating whether the kext executable
 *           contains only symbol references.
 */
#define kOSBundleIsInterfaceKey                 "OSBundleIsInterface"

/*!
 * @define   kOSBundleLibrariesKey
 * @abstract A dictionary listing link dependencies for this kext.
 *           Keys are bundle identifiers, values are version strings.
 */
#define kOSBundleLibrariesKey                   "OSBundleLibraries"

/*!
 * @define   kOSBundleRequiredKey
 * @abstract A string indicating in which kinds of startup this kext
 *           may need to load during early startup (before
 *           <code>@link //apple_ref/doc/man/8/kextd kextcache(8)@/link</code>).
 * @discussion
 * The value is one of:
 * <ul>
 * <li>@link kOSBundleRequiredRoot "OSBundleRequiredRoot"@/link</li>
 * <li>@link kOSBundleRequiredLocalRoot "OSBundleRequiredLocalRoot"@/link</li>
 * <li>@link kOSBundleRequiredNetworkRoot "OSBundleRequiredNetworkRoot"@/link</li>
 * <li>@link kOSBundleRequiredSafeBoot "OSBundleRequiredSafeBoot"@/link</li>
 * <li>@link kOSBundleRequiredConsole "OSBundleRequiredConsole"@/link</li>
 * </ul>
 *
 * Use this property judiciously.
 * Every kext that declares a value other than "OSBundleRequiredSafeBoot"
 * increases startup time, as the booter must read it into memory,
 * or startup kext caches must include it.
 */
#define kOSBundleRequiredKey                    "OSBundleRequired"

/*!
 * @define   kOSBundleAllowUserLoadKey
 * @abstract A boolean value indicating whether
 *           <code>@link //apple_ref/doc/man/8/kextd kextcache(8)@/link</code>
 *           will honor a non-root process's request to load a kext.
 * @discussion
 * See <code>@link //apple_ref/doc/compositePage/c/func/KextManagerLoadKextWithURL
 * KextManagerLoadKextWithURL@/link</code>
 * and <code>@link //apple_ref/doc/compositePage/c/func/KextManagerLoadKextWithIdentifier
 * KextManagerLoadKextWithIdentifier@/link</code>.
 */
#define kOSBundleAllowUserLoadKey               "OSBundleAllowUserLoad"

/*!
 * @define   kOSKernelResourceKey
 * @abstract A boolean value indicating whether the kext represents a built-in
 *           component of the kernel.
 */
#define kOSKernelResourceKey                    "OSKernelResource"

/*!
 * @define   kOSKextVariantOverrideKey
 * @abstract A dictionary with target names as key and a target-specific variant
 *           name as value.
 */
#define kOSKextVariantOverrideKey               "OSKextVariantOverride"

/*!
 * @define   kIOKitPersonalitiesKey
 * @abstract A dictionary of dictionaries used in matching for I/O Kit drivers.
 */
#define kIOKitPersonalitiesKey                  "IOKitPersonalities"

/*
 * @define   kIOPersonalityPublisherKey
 * @abstract Used in personalities sent to the I/O Kit,
 *           contains the CFBundleIdentifier of the kext
 *           that the personality originated in.
 */
#define kIOPersonalityPublisherKey              "IOPersonalityPublisher"

#if CONFIG_KEC_FIPS
/*
 * @define   kAppleTextHashesKey
 * @abstract A dictionary conataining hashes for corecrypto kext.
 */
#define kAppleTextHashesKey                     "AppleTextHashes"
#endif



#if PRAGMA_MARK
/********************************************************************/
#pragma mark Kext/OSBundle Property Deprecated Keys
/********************************************************************/
#endif
/*
 * @define   kOSBundleDebugLevelKey
 * @abstract
 * Deprecated (used on some releases of Mac OS X prior to 10.6 Snow Leopard).
 * Value is an integer from 1-6, corresponding to the verbose levels
 * of kext tools on those releases.
 * On 10.6 Snow Leopard, use <code>@link OSKextEnableKextLogging
 * OSKextEnableKextLogging@/link</code>.
 */
#define kOSBundleDebugLevelKey                  "OSBundleDebugLevel"

/*!
 * @define   kOSBundleSharedExecutableIdentifierKey
 * @abstract Deprecated (used on some releases of Mac OS X
 *           prior to 10.6 Snow Leopard).
 *           Value is the bundle identifier of the pseudokext
 *           that contains an executable shared by this kext.
 */
#define kOSBundleSharedExecutableIdentifierKey  "OSBundleSharedExecutableIdentifier"


#if PRAGMA_MARK
/********************************************************************/
#pragma mark Kext/OSBundle Property List Values
/********************************************************************/
#endif

/*!
 * @group Kext Property List Values
 * These constants encompass established values
 * for kernel extension bundle properties.
 */

/*!
 * @define   kOSKextKernelIdentifier
 * @abstract
 * This is the CFBundleIdentifier user for the kernel itself.
 */
#define kOSKextKernelIdentifier                 "__kernel__"

/*!
 * @define  kOSKextBundlePackageTypeKext
 * @abstract
 * The bundle type value for Kernel Extensions.
 */
#define kOSKextBundlePackageTypeKext        "KEXT"

/*!
 * @define  kOSKextBundlePackageTypeDriverKit
 * @abstract
 * The bundle type value for Driver Extensions.
 */
#define kOSKextBundlePackageTypeDriverKit   "DEXT"

/*!
 * @define   kOSBundleRequiredRoot
 * @abstract
 * This <code>@link kOSBundleRequiredKey OSBundleRequired@/link</code>
 * value indicates that the kext may be needed to mount the root filesystem
 * whether starting from a local or a network volume.
 */
#define kOSBundleRequiredRoot                   "Root"

/*!
 * @define   kOSBundleRequiredLocalRoot
 * @abstract
 * This <code>@link kOSBundleRequiredKey OSBundleRequired@/link</code>
 * value indicates that the kext may be needed to mount the root filesystem
 * when starting from a local disk.
 */
#define kOSBundleRequiredLocalRoot              "Local-Root"

/*!
 * @define   kOSBundleRequiredNetworkRoot
 * @abstract
 * This <code>@link kOSBundleRequiredKey OSBundleRequired@/link</code>
 * value indicates that the kext may be needed to mount the root filesystem
 * when starting over a network connection.
 */
#define kOSBundleRequiredNetworkRoot            "Network-Root"

/*!
 * @define   kOSBundleRequiredSafeBoot
 * @abstract
 * This <code>@link kOSBundleRequiredKey OSBundleRequired@/link</code>
 * value indicates that the kext can be loaded during a safe startup.
 * This value does not normally cause the kext to be read by the booter
 * or included in startup kext caches.
 */
#define kOSBundleRequiredSafeBoot               "Safe Boot"

/*!
 * @define   kOSBundleRequiredConsole
 * @abstract
 * This <code>@link kOSBundleRequiredKey OSBundleRequired@/link</code>
 * value indicates that the kext may be needed for console access
 * (specifically in a single-user startup when
 * <code>@link //apple_ref/doc/man/8/kextd kextd(8)@/link</code>.
 * does not run)
 * and should be loaded during early startup.
 */
#define kOSBundleRequiredConsole                "Console"

/*!
 * @define   kOSBundleRequiredDriverKit
 * @abstract
 * This <code>@link kOSBundleRequiredKey OSBundleRequired@/link</code>
 * value indicates that the driver extension's (DriverKit driver's)
 * personalities must be present in the kernel at early boot (specifically
 * before <code>@link //apple_ref/doc/man/8/kextd kextd(8)@/link</code> starts)
 * in order to compete with kexts built into the prelinkedkernel. Note that
 * kextd is still required to launch the user space driver binary. The IOKit
 * matching will happen during early boot, and the actual driver launch
 * will happen after kextd starts.
 */
#define kOSBundleRequiredDriverKit              "DriverKit"

#if PRAGMA_MARK
#pragma mark -
/********************************************************************/
#pragma mark Kext Information
/********************************************************************/
#endif
/*!
 * @group Kext Information
 * Types, constants, and macros providing a kext with information
 * about itself.
 */

#ifdef KERNEL
/*!
 * @typedef OSKextLoadTag
 *
 * @abstract
 * A unique identifier assigned to a loaded instanace of a kext.
 *
 * @discussion
 * If a kext is unloaded and later reloaded, the new instance
 * has a different load tag.
 *
 * A kext can get its own load tag in the <code>kmod_info_t</code>
 * structure passed into its module start routine, as the
 * <code>id</code> field (cast to this type).
 * You can use the load tag with the functions
 * <code>@link OSKextRetainKextWithLoadTag
 * OSKextRetainKextWithLoadTag@/link</code> and
 * <code>@link OSKextReleaseKextWithLoadTag
 * OSKextReleaseKextWithLoadTag@/link</code>.
 */
#else
/*!
 * @typedef OSKextLoadTag
 *
 * @abstract
 * A unique identifier assigned to a loaded instanace of a kext.
 *
 * @discussion
 * If a kext is unloaded and later reloaded, the new instance
 * has a different load tag.
 *
 * A kext can get its own load tag in the <code>kmod_info_t</code>
 * structure passed into its module start routine, as the
 * <code>id</code> field (cast to this type).
 */
#endif
typedef uint32_t  OSKextLoadTag;

/*!
 * @define kOSKextInvalidLoadTag
 *
 * @abstract
 * A load tag value that will never be used for a loaded kext;
 * indicates kext not found.
 */
#define  kOSKextInvalidLoadTag  ((OSKextLoadTag)(-1))

#ifdef KERNEL

/* Make these visible to kexts only and *not* the kernel.
 */
#if !XNU_KERNEL_PRIVATE

/*!
 * @function OSKextGetCurrentLoadTag
 *
 * @abstract
 * Returns the run-time load tag for the calling kext as an
 * <code>@link OSKextLoadTag OSKextLoadTag@/link</code>.
 *
 * @result
 * The run-time load tag for the calling kext as an
 * <code>@link OSKextLoadTag@/link</code>.
 *
 * @discussion
 * The load tag identifies this loaded instance of the kext to the kernel
 * and to kernel functions that operate on kexts.
 */
OSKextLoadTag OSKextGetCurrentLoadTag(void);

/*!
 * @function OSKextGetCurrentIdentifier
 *
 * @abstract
 * Returns the CFBundleIdentifier for the calling kext as a C string.
 *
 * @result
 * The CFBundleIdentifier for the calling kext as a C string.
 */
const char * OSKextGetCurrentIdentifier(void);

/*!
 * @function OSKextGetCurrentVersionString
 *
 * @abstract
 * Returns the CFBundleVersion for the calling kext as a C string.
 *
 * @result
 * The CFBundleVersion for the calling kext as a C string.
 */
const char * OSKextGetCurrentVersionString(void);

#endif /* !XNU_KERNEL_PRIVATE */

#if PRAGMA_MARK
#pragma mark -
/********************************************************************/
#pragma mark Kext Loading C Functions
/********************************************************************/
#endif
/*!
 * @group Kext Loading C Functions
 * Functions for loading and tracking kexts in the kernel.
 */

/*!
 * @function OSKextLoadKextWithIdentifier
 *
 * @abstract
 * Request that a kext be loaded.
 *
 * @param  kextIdentifier  The bundle identifier of the kext to be loaded.
 *
 * @result
 * <code>@link //apple_ref/c/macro/kOSReturnSuccess kOSReturnSuccess@/link</code>
 * if the kext was loaded (or was already loaded).
 * <code>@link //apple_ref/c/macro/kOSKextReturnDeferred kOSKextReturnDeferred@/link</code>
 * if the kext was not found and a request
 * was queued to <code>@link //apple_ref/doc/man/8/kextd kextd(8)@/link</code>.
 * Other return values indicate a failure to load the kext.
 *
 * @discussion
 * If a kext is already in the kernel but not loaded, it is loaded immediately.
 * If it isn't found, an asynchronous load request is
 * made to <code>@link //apple_ref/doc/man/8/kextd kextd(8)@/link</code>
 * and  <code>@link //apple_ref/c/macro/kOSKextReturnDeferred kOSKextReturnDeferred@/link</code> is returned.
 * There is no general notification or callback mechanism for load requests.
 */
OSReturn OSKextLoadKextWithIdentifier(const char * kextIdentifier);


/*!
 * @function OSKextRetainKextWithLoadTag
 *
 * @abstract
 * Retain a loaded kext based on its load tag,
 * and enable autounload for that kext.
 *
 * @param  loadTag   The load tag of the kext to be retained.
 *                   See <code>@link OSKextGetCurrentLoadTag@/link</code>.
 *
 * @result
 * <code>@link //apple_ref/c/macro/kOSReturnSuccess kOSReturnSuccess@/link</code>
 * if the kext was retained.
 * <code>@link //apple_ref/c/macro/kOSKextReturnNotFound kOSKextReturnNotFound@/link</code>
 * if the kext was not found.
 * <code>@link //apple_ref/c/macro/kOSKextReturnInvalidArgument
 * kOSKextReturnInvalidArgument@/link</code>
 * if <code>loadTag</code> is
 * <code>@link kOSKextInvalidLoadTag kOSKextInvalidLoadTag@/link</code>.
 *
 * @discussion
 * Retaining a kext prevents it from being unloaded,
 * either explicitly or automatically, and enables autounload for the kext.
 * When autounload is enabled, then shortly after the kext's last reference
 * is dropped, it will be unloaded if there are no outstanding references to it
 * and there are no instances of its Libkern C++ subclasses (if any).
 *
 * Kexts that define subclasses of
 * <code>@link //apple_ref/doc/class/IOService IOService@/link</code>
 * have autounload enabled automatically.
 * Other kexts can use the reference count to manage automatic unload
 * without having to define and create Libkern C++ objects.
 * For example, a filesystem kext can retain itself whenever a new mount
 * is created, and release itself when a mount is removed.
 * When the last mount is removed, the kext will be unloaded after a brief delay.
 *
 * A kext can get its own load tag using the
 * <code>@link OSKextGetCurrentLoadTag@/link</code>.
 *
 * Kexts should not retain and release other kexts; linkage references
 * are accounted for internally.
 */
OSReturn OSKextRetainKextWithLoadTag(OSKextLoadTag loadTag);


/*!
 * @function OSKextReleaseKextWithLoadTag
 *
 * @abstract
 * Release a loaded kext based on its load tag.
 *
 * @param  loadTag   The load tag of the kext to be released.
 *                   See <code>@link OSKextGetCurrentLoadTag@/link</code>.
 *
 * @result
 * <code>@link //apple_ref/c/macro/kOSReturnSuccess kOSReturnSuccess@/link</code>
 * if the kext was released.
 * <code>@link //apple_ref/c/macro/kOSKextReturnNotFound
 * kOSKextReturnNotFound@/link</code>
 * if the kext was not found.
 * <code>@link //apple_ref/c/macro/kOSKextReturnInvalidArgument
 * kOSKextReturnInvalidArgument@/link</code>
 * if <code>loadTag</code> is
 * <code>@link kOSKextInvalidLoadTag kOSKextInvalidLoadTag@/link</code>.
 *
 * @discussion
 * The kext should have been retained previously via
 * <code>@link OSKextRetainKextWithLoadTag@/link</code>.
 *
 * This function schedules an autounload scan for all kexts.
 * When that scan occurs, if a kext has autounload enabled,
 * it will be unloaded if there are no outstanding references to it
 * and there are no instances of its Libkern C++ classes (if any).
 *
 * Kexts that define subclasses of
 * <code>@link //apple_ref/doc/class/IOService IOService@/link</code>
 * have autounload enabled automatically.
 * Other kexts can use the reference count to manage automatic unload
 * without having to define and create Libkern C++ objects.
 * For example, a filesystem kext can be retained whenever a new mount
 * is created, and released when a mount is removed.
 * When the last mount is removed, the kext will be unloaded after a brief delay.
 *
 * While the autounload scan takes place after a delay of at least a minute,
 * a kext that manages its own reference counts for autounload should
 * be prepared to have its module stop function called even while the function
 * calling this function is still running.
 *
 * A kext can get its own load tag using the
 * <code>@link OSKextGetCurrentLoadTag@/link</code>.
 *
 * Kexts should not retain and release other kexts; linkage references
 * are accounted for internally.
 */
OSReturn OSKextReleaseKextWithLoadTag(OSKextLoadTag loadTag);

#if PRAGMA_MARK
/********************************************************************/
#pragma mark -
#pragma mark Kext Requests
/********************************************************************/
#endif
/*!
 * @group Kext Requests to User Space
 * Functions for making requests to kextd in user space.
 */

/*!
 * @typedef OSKextRequestTag
 *
 * @abstract
 * Identifies a kext request made to user space.
 */
typedef uint32_t OSKextRequestTag;

/*!
 * @define kOSKextRequestTagInvalid
 *
 * @abstract
 * A request tag value that will never be used for a kext request;
 * indicates failure to create/queue the request.
 */
#define kOSKextRequestTagInvalid  ((OSKextRequestTag)-1)

/*!
 * @typedef OSKextRequestResourceCallback
 *
 * @abstract
 * Invoked to provide results for a kext resource request.
 *
 * @param  requestTag          The tag of the request that the callback pertains to.
 * @param  result              The result of the request:
 *                             <code>@link kOSReturnSuccess
 *                             kOSReturnSuccess@/link</code>
 *                             if the request was fulfilled;
 *                             <code>@link kOSKextReturnTimeout
 *                             kOSKextReturnTimeout@/link</code>
 *                             if the request has timed out;
 *                             <code>@link kOSKextReturnStopping
 *                             kOSKextReturnStopping@/link</code>
 *                             if the kext containing the callback
 *                             address for the kext is being unloaded;
 *                             or other values on error.
 * @param  resourceData        A pointer to the requested resource data.
 *                             Owned by the system; the kext should make a copy
 *                             if it needs to keep the data past the callback.
 * @param  resourceDataLength  The length of <code>resourceData</code>.
 * @param  context             The context pointer originally passed to
 *                             <code>@link OSKextRequestResource
 *                             OSKextRequestResource@/link</code>.
 */
typedef void (* OSKextRequestResourceCallback)(
	OSKextRequestTag                requestTag,
	OSReturn                        result,
	const void                    * resourceData,
	uint32_t                        resourceDataLength,
	void                          * context);

/*!
 * @function OSKextRequestResource
 *
 * @abstract
 * Requests data from a nonlocalized resource file in a kext bundle on disk.
 *
 * @param  kextIdentifier  The CFBundleIdentifier of the kext
 *                         from which to read the file.
 * @param  resourceName    The name of the resource file to read.
 * @param  callback        A pointer to a callback function; the address
 *                         must be within a currently-loaded kext.
 * @param  context         A pointer to arbitrary run-time data
 *                         that will be passed to the callback
 *                         when it is invoked. May be <code>NULL</code>.
 * @param  requestTagOut   If non-<code>NULL</code>,
 *                         filled on success with a tag identifying the
 *                         pending request
 *                         (or on failure with <code>@link kOSKextRequestTagInvalid
 *                         kOSKextRequestTagInvalid@/link</code>;
 *                         can be used with
 *                         <code>@link OSKextCancelRequest
 *                         OSKextCancelRequest@/link</code>.
 *
 * @result
 * <code>@link kOSReturnSuccess kOSReturnSuccess@/link</code>
 * if the request is successfully queued.
 * <code>@link kOSKextReturnInvalidArgument kOSKextReturnInvalidArgument@/link</code>
 * if <code>kextIdentifier</code> or <code>resourceName</code> or if
 * <code>callback</code> is not an address within a loaded kext executable.
 * <code>@link kOSKextReturnStopping kOSKextReturnStopping@/link</code>
 * if an unload attempt is being made
 * on the kext containing <code>callback</code>.
 * Other <code>OSKextReturn...</code> errors are possible.
 *
 * @discussion
 * This function queues an asynchronous request to the user-space kext daemon
 * <code>@link //apple_ref/doc/man/8/kextd kextd(8)@/link</code>;
 * requests for resources early in system startup
 * will not be fulfilled until that daemon starts.
 * Requests made by a kext while that kext is loading
 * (specifically in the kext's module start routine)
 * will not be fulfilled until after the start routine returns and
 * the kext is completely loaded.
 * Kexts requesting resources should be sure to perform appropriate locking
 * in the callback function.
 *
 * Kext resources are stored in the kext's on-disk bundle under the
 * Resources subdirectory.
 * See {@linkdoc //apple_ref/doc/uid/10000123i Bundle Programming Guide}
 * for an overview of bundle structure.
 * The localization context of the kext daemon
 * (namely that of the superuser)
 * will be used in retrieving resources;
 * kext resources intended for use in the kernel
 * should generally not be localized.
 *
 * <code>callback</code> is guaranteed to be invoked except when:
 * <ul>
 * <li>@link OSKextCancelRequest <code>OSKextCancelRequest</code>@/link
 *     is used to cancel the request.
 *     In this case the kext gets the <code>context</code> pointer
 *     and can clean it up.</li>
 * <li>The request is made during a kext's module start routine
 *     and the start routine returns an error.
 *     In this case, callbacks cannot be safely invoked, so
 *     the kext should clean up all request contexts
 *     when returning the error from the start routine.</li>
 * </ul>
 *
 * Kexts with pending requests are not subject to autounload,
 * but requests are subject to timeout after a few minutes.
 * If that amount of time passes with no response from user space,
 * <code>callback</code> is invoked with a result of.
 * <code>@link kOSKextReturnTimeout kOSKextReturnTimeout@/link</code>.
 *
 * Kexts that are explicitly unloaded have all pending request callbacks
 * invoked with a result of
 * <code>@link kOSKextReturnStopping kOSKextReturnStopping@/link</code>.
 * The kext must handle these callbacks,
 * even if its stop routine will prevent unloading.
 * If the kext does prevent unloading, it can reissue resource requests
 * outside of the stop function.
 */
OSReturn OSKextRequestResource(
	const char                    * kextIdentifier,
	const char                    * resourceName,
	OSKextRequestResourceCallback   callback,
	void                          * context,
	OSKextRequestTag              * requestTagOut);

/*!
 * @function OSKextCancelRequest
 *
 * @abstract
 * Cancels a pending user-space kext request without invoking the callback.
 *
 * @param  requestTag  A tag identifying a pending request.
 * @param  contextOut  If non-<code>NULL</code>, filled with the context pointer
 *                     originally passed with the request.
 *
 * @result
 * <code>@link kOSReturnSuccess kOSReturnSuccess@/link</code>
 * if the request is successfully canceled.
 * <code>@link kOSKextReturnNotFound kOSKextReturnNotFound@/link</code>
 * if <code>requestTag</code> does not identify any pending request.
 * Other <code>OSKextReturn...</code> errors are possible.
 *
 * @discussion
 * This function cancels a pending request if it exists,
 * so that its callback will not be invoked.
 * It returns in <code>contextOut</code>
 * the context pointer used to create the request
 * so that any resources allocated for the request can be cleaned up.
 *
 * Kexts do not need to cancel outstanding requests
 * in their module stop functions;
 * when a kext is unloaded, all pending request callbacks
 * are invoked with a result of
 * <code>@link kOSKextReturnTimeout kOSKextReturnTimeout@/link</code>
 * before the stop function is called.
 */
OSReturn OSKextCancelRequest(
	OSKextRequestTag    requestTag,
	void             ** contextOut);


/*!
 * @function OSKextGrabPgoData
 *
 * @abstract
 * Grab a LLVM profile data buffer from a loaded kext.
 *
 * @param   uuid             the uuid identifying the kext to retrieve data from
 * @param   pSize            pointer of where to store the size of the buffer.   May be NULL.
 * @param   pBuffer          pointer to the output buffer.   May be NULL.
 * @param   bufferSize       size of the buffer pointed to by pBuffer
 * @param   wait_for_unload  (boolean) sleep until the kext is unloaded
 * @param   metadata         (boolean) include metadata footer
 *
 * @result
 * 0 on success
 * ENOTSUP if the kext does not have profile data to retrieve.
 * ENOTSUP if no kext with the given UUID is found
 * ERRORS  if the provided buffer is too small
 * EIO     internal error, such as if __llvm_profile_write_buffer_internal fails
 */
int
OSKextGrabPgoData(uuid_t uuid,
    uint64_t *pSize,
    char *pBuffer,
    uint64_t bufferSize,
    int wait_for_unload,
    int metadata);

/*!
 * @function OSKextResetPgoCountersLock
 *
 * @abstract
 * Call this function before trapping into the debugger to call OSKextResetPgoCounters.
 */
void
OSKextResetPgoCountersLock(void);

/*!
 * @function OSKextResetPgoCountersUnlock
 *
 * @abstract
 * Call this function after trapping into the debugger to call OSKextResetPgoCounters.
 */
void
OSKextResetPgoCountersUnlock(void);

/*!
 * @function OSKextResetPgoCounters
 *
 * @abstract Reset the PGO counters for all kexts.  Call only from debugger
 * context, while holding OSKextResetPgoCountersLock().
 */
void
OSKextResetPgoCounters(void);


#if PRAGMA_MARK
#pragma mark -
/********************************************************************/
#pragma mark Weak linking
/********************************************************************/
#endif
/*!
 * @group Weak Linking
 * Support for weak references to symbols in kexts.
 */

/*!
 * @var gOSKextUnresolved
 *
 * @abstract
 * The value to which a kext's unresolved, weakly-referenced symbols are bound.
 *
 * @discussion
 * A kext must test a weak symbol before using it.  A weak symbol
 * is only safe to use if it is not equal to <code>gOSKextUnresolved</code>.
 *
 * Example for a weak symbol named <code>foo</code>:
 * <pre>
 * @textblock
 *      if (&foo != gOSKextUnresolved) {
 *          foo();
 *      } else {
 *          printf("foo() is not supported\n");
 *      }
 * @/textblock
 * </pre>
 */
extern const void * const gOSKextUnresolved;

/*!
 * @define OSKextSymbolIsResolved
 *
 * @abstract
 * Checks whether a weakly-referenced symbol has been resolved.
 *
 * @param weak_sym   The weak symbol to be tested for resolution.
 *
 * @result
 * <code>TRUE</code> if weak_sym is resolved, or <code>FALSE</code>
 * if weak_sym is unresolved.
 *
 * @discussion
 * This is a convenience macro for testing if weak symbols are resolved.
 *
 * Example for a weak symbol named <code>foo</code>:
 * <pre>
 * @textblock
 *      if (OSKextSymbolIsResolved(foo)) {
 *          foo();
 *      } else {
 *          printf("foo() is not resolved\n");
 *      }
 * @/textblock
 * </pre>
 */
#define OSKextSymbolIsResolved(weak_sym)        \
    (&(weak_sym) != gOSKextUnresolved)


#if CONFIG_KEC_FIPS

#if PRAGMA_MARK
#pragma mark -
/********************************************************************/
#pragma mark Kernel External Components for FIPS compliance
/********************************************************************/
#endif

// Kernel External Components for FIPS compliance (KEC_FIPS)
// WARNING - ath_hash is owned by the kernel, do not free
typedef struct AppleTEXTHash {
	int                 ath_version;// version of this structure (value is 1 or 2)
	int             ath_length; // length of hash data
	void *          ath_hash;   // hash extracted from AppleTextHashes dict
} AppleTEXTHash_t;
#endif // CONFIG_KEC_FIPS

#endif /* KERNEL */

__END_DECLS

#endif /* _LIBKERN_OSKEXTLIB_H */

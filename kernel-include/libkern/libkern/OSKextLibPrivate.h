/*
 * Copyright (c) 1998-2000 Apple Inc. All rights reserved.
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

#ifndef _LIBKERN_OSKEXTLIBPRIVATE_H
#define _LIBKERN_OSKEXTLIBPRIVATE_H


#include <sys/cdefs.h>
#include <uuid/uuid.h>

__BEGIN_DECLS
#ifdef KERNEL
#include <libkern/OSTypes.h>
#include <mach/kmod.h>
#include <mach/vm_types.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#include <mach/kmod.h>
#endif /* KERNEL */
__END_DECLS

#include <libkern/OSReturn.h>

__BEGIN_DECLS

#if PRAGMA_MARK
#pragma mark -
/********************************************************************/
#pragma mark Misc Constants
/********************************************************************/
#endif

typedef uint8_t OSKextExcludeLevel;
#define kOSKextExcludeNone  (0)
#define kOSKextExcludeKext  (1)
#define kOSKextExcludeAll   (2)

#define kOSKextManagementEntitlement "com.apple.private.security.kext-management"

#if PRAGMA_MARK
#pragma mark -
/********************************************************************/
#pragma mark Kext/OSBundle Property List Keys
/********************************************************************/
#endif

/*!
 * @define   kOSBundleHelperKey
 * @abstract Used by loginwindow.
 */
#define kOSBundleHelperKey      "OSBundleHelper"

/*!
 * @define   kOSBundleDeveloperOnlyKey
 * @abstract A boolean value indicating whether the kext should only load on
 *           Developer devices.
 */
#define kOSBundleDeveloperOnlyKey               "OSBundleDeveloperOnly"

/*!
 * @define   kOSBundleRamDiskOnlyKey
 * @abstract A boolean value indicating whether the kext should only load when
 *           booted from a ram disk.
 */
#define kOSBundleRamDiskOnlyKey         "OSBundleRamDiskOnly"


/*!
 * @define   kAppleSecurityExtensionKey
 * @abstract A boolean value indicating whether the kext registers
 *           MACF hooks.
 */
#define kAppleSecurityExtensionKey      "AppleSecurityExtension"

/*!
 * @define   kAppleKernelExternalComponentKey
 * @abstract A boolean value indicating whether the kext is vending kernel
 *           KPI, and needs special loading behavior.
 */
#define kAppleKernelExternalComponentKey        "AppleKernelExternalComponent"

// properties found in the registry root
#define kOSKernelCPUTypeKey             "OSKernelCPUType"
#define kOSKernelCPUSubtypeKey          "OSKernelCPUSubtype"
#define kOSStartupMkextCRC              "OSStartupMkextCRC"             /* value is 32-bit OSData */
#define kOSPrelinkKextCountKey          "OSPrelinkKextCount"            /* value is 32-bit OSNumber */
#define kOSPrelinkPersonalityCountKey   "OSPrelinkPersonalityCount"     /* value is 32-bit OSNumber */

#if PRAGMA_MARK
#pragma mark -
/********************************************************************/
#pragma mark Load Info Keys
/********************************************************************/
#endif
/*********************************************************************
* In addition to the keys defined here, you will find:
*   kCFBundleIdentifierKey
*   kCFBundleVersionKey
*   kOSBundleCompatibleVersionKey
*   kOSBundleIsInterfaceKey
*   kOSKernelResourceKey
*********************************************************************/
#define kOSBundleMachOHeadersKey                "OSBundleMachOHeaders"
#define kOSBundleLogStringsKey                  "OSBundleLogStrings"
#define kOSBundleCPUTypeKey                     "OSBundleCPUType"
#define kOSBundleCPUSubtypeKey                  "OSBundleCPUSubtype"
#define kOSBundlePathKey                        "OSBundlePath"
#define kOSBundleExecutablePathKey              "OSBundleExecutablePath"
#define kOSBundleUUIDKey                        "OSBundleUUID"
#define kOSBundleTextUUIDKey                    "OSBundleTextUUID"
#define kOSBundleStartedKey                     "OSBundleStarted"
#define kOSBundlePrelinkedKey                   "OSBundlePrelinked"
#define kOSBundleLoadTagKey                     "OSBundleLoadTag"
#define kOSBundleLoadAddressKey                 "OSBundleLoadAddress"
#define kOSBundleLoadSizeKey                    "OSBundleLoadSize"
#define kOSBundleExecLoadAddressKey             "OSBundleExecLoadAddress"
#define kOSBundleExecLoadSizeKey                "OSBundleExecLoadSize"
#define kOSBundleWiredSizeKey                   "OSBundleWiredSize"
#define kOSBundleDependenciesKey                "OSBundleDependencies"
#define kOSBundleRetainCountKey                 "OSBundleRetainCount"
#define kOSBundleCacheLoadAddressKey            "OSBundleCacheLoadAddress"
// Kernel TEXT encompasses kexts
#define kOSBundleKextsInKernelTextKey           "OSBundleKextsInKernelText"
// OSKextCopyLoadedKextInfo includes non-started kexts when present:
#define kOSBundleAllPrelinkedKey                "OSBundleAllPrelinked"

/* Dictionary of metaclass info keyed by classname.
 */
#define kOSBundleClassesKey                     "OSBundleClasses"

/* These are contained in kOSBundleClassesKey. kOSMetaClassSuperclassNameKey
 * may be absent (for the root class).
 */
#define kOSMetaClassNameKey                     "OSMetaClassName"
#define kOSMetaClassSuperclassNameKey           "OSMetaClassSuperclassName"
#define kOSMetaClassTrackingCountKey            "OSMetaClassTrackingCount"

#if PRAGMA_MARK
#pragma mark -
/********************************************************************/
#pragma mark Kext Log Specification
/********************************************************************/
#endif
/*!
 * @group Kext Log Specification
 * Logging levels & flags for kernel extensions.
 * See <code>@link //apple_ref/c/tdef/OSKextLogSpec OSKextLogSpec@/link</code>
 * for an overview.
 */

/*!
 * @typedef  OSKextLogSpec
 * @abstract Describes what a log message applies to,
 *           or a filter that determines which log messages are displayed.
 *
 * @discussion
 * A kext log specification is a 32-bit value used as a desription of
 * what a given log message applies to, or as a filter
 * indicating which log messages are desired and which are not.
 * A log specification has three parts (described in detail shortly):
 * <ul>
 *   <li>A <b>level</b> from 0-7 in the lowest-order nibble (0x7).</li>
 *   <li>A flag bit in the lowest-order nibble (0x8) indicating whether
 *       log messages tied to individual kexts are always printed (1)
 *       or printed only if the kext has an
 *       @link //apple_ref/c/macro/kOSBundleEnableKextLoggingKey
 *       OSBundleEnableKextLogging@/link set to <code>true</code>.
 *   <li>A set of <b>activity flags</b> in the remaining nibbles (0xFFFFFFF0),
 *       which describe areas of activity related to kernel extensions.</li>
 * </ul>
 *
 * You can specify a log spec to most of the kext tools with the -v option
 * and a hex number (rather than the escalating decimal levels 0-6).
 * You can also specify a log spec to the kernel with the "kextlog" boot arg
 * or "debug.kextlog" sysctl.
 *
 * <b>Log Levels</b>
 *
 * The log level spans a range from silent (no log messages)
 * to debuging information:
 *
 * <ol start="0">
 * <li>Silent - Not applicable to messages; as a filter, do not print any log messages.</li>
 * <li>Errors - Log message is an error.
 * <li>Warnings - Log message is a warning.
 * <li>Basic information - Log message is basic success/failure.</li>
 * <li>Progress - Provides high-level information about stages in processing.</li>
 * <li>Step - Provides low-level information about complex operations,
 *          typically about individual kexts.</li>
 * <li>Detail - Provides very low-level information about parts of kexts,
 *          including individual Libkern classes and operations on bundle files.</li>
 * <li>Debug - Very verbose logging about internal activities.</li>
 * </ol>
 *
 * Log messages at
 * <code>@link kOSKextLogErrorLevel kOSKextLogErrorLevel@/link</code> or
 * <code>@link kOSKextLogWarningLevel kOSKextLogWarningLevel@/link</code>
 * ignore activity flags and the
 * @link //apple_ref/c/macro/kOSBundleEnableKextLoggingKey
 * OSBundleEnableKextLogging@/link property;
 * that is, only the filter level is checked for messages at these levels.
 * Log messages at levels above
 * <code>@link kOSKextLogWarningLevel kOSKextLogWarningLevel@/link</code>
 * are filtered according both to the activity flags in the current filter
 * and to whether the log message is associated with a kext or not.
 * Normally log messages associated with kexts are not printed
 * unless the kext has a
 * @link //apple_ref/c/macro/kOSBundleEnableKextLoggingKey
 * OSBundleEnableKextLogging@/link property set to <code>true</code>.
 * If you set the high-order bit of the log level (that is, add 8 to the level),
 * then all kext-specific log messages matching the activity flags are printed.
 * This can be very verbose.
 *
 * <b>Activity Flags</b>
 *
 * Each flag governs a category of activity,
 * such as loading, IPC, or archives; by combining them with bitwise OR,
 * you can choose which messages you wish to see (or when logging messages,
 * which bit flags select your message).
 *
 * <b>Byte 1:</b> <code>0xF0</code> - Basic activities
 * (<code>@link kOSKextLogGeneralFlag kOSKextLogGeneralFlag@/link</code>,
 * <code>@link kOSKextLogLoadFlag kOSKextLogLoadFlag@/link</code>, and
 * <code>@link kOSKextLogArchiveFlag kOSKextLogArchiveFlag@/link</code>).
 *
 * <b>Byte 2:</b> <code>0xF00</code> - Reserved.
 *
 * <b>Byte 4:</b> <code>0xF000</code> - Kext diagnostics
 * (<code>@link kOSKextLogValidationFlag kOSKextLogValidationFlag@/link</code>,
 * <code>@link kOSKextLogAuthenticationFlag kOSKextLogAuthenticationFlag@/link</code>, and
 * <code>@link kOSKextLogDependenciesFlag kOSKextLogDependenciesFlag@/link</code>).
 *
 * <b>Byte 5:</b> <code>0xF00000</code> - Kext access & bookkeeping
 * (<code>@link kOSKextLogDirectoryScanFlag kOSKextLogDirectoryScanFlag@/link</code>,
 * <code>@link kOSKextLogFileAccessFlag kOSKextLogFileAccessFlag@/link</code>,
 * <code>@link kOSKextLogKextBookkeepingFlag kOSKextLogKextBookkeepingFlag@/link </code>).
 *
 * <b>Byte 6:</b> <code>0xF000000</code> - Linking & patching
 * (<code>@link kOSKextLogLinkFlag kOSKextLogLinkFlag@/link</code> and
 * <code>@link kOSKextLogPatchFlag kOSKextLogPatchFlag@/link</code>).
 *
 * <b>Byte 7:</b> <code>0xF0000000</code> - Reserved.
 */
typedef uint32_t OSKextLogSpec;

#if PRAGMA_MARK
/********************************************************************/
#pragma mark Masks
/********************************************************************/
#endif
/*!
 * @define   kOSKextLogLevelMask
 * @abstract Masks the bottom 3 bits of an
 *           <code>@link OSKextLogSpec OSKextLogSpec@/link</code> to extract
 *           the raw level.
 */
#define kOSKextLogLevelMask              ((OSKextLogSpec) 0x00000007)

/*!
 * @define   kOSKextLogKextOrGlobalMask
 * @abstract Determines whether per-kext log messages are output.
 *
 * @discussion
 * In filter specifications, if unset (the usual default),
 * then log messages associated with a kext are only output
 * if the kext has an
 * @link //apple_ref/c/macro/kOSBundleEnableKextLoggingKey
 * OSBundleEnableKextLogging@/link
 * property set to <code>true</code>.
 * If set, then all log messages associated with kexts
 * are output.
 *
 * In message specifications, if set it indicates that the message is either
 * not associated with a kext, or is associated with a kext that has an
 * @link //apple_ref/c/macro/kOSBundleEnableKextLoggingKey
 * OSBundleEnableKextLogging@/link
 * property set to <code>true</code>.
 */
#define kOSKextLogKextOrGlobalMask       ((OSKextLogSpec) 0x00000008)


/*!
 * @define   kOSKextLogFlagsMask
 * @abstract Masks the flag bits of an
 *           <code>@link OSKextLogSpec OSKextLogSpec@/link</code>.
 */
#define kOSKextLogFlagsMask              ((OSKextLogSpec) 0x0ffffff0)

/*!
 * @define   kOSKextLogFlagsMask
 * @abstract Masks the flag bits of an
 *           <code>@link OSKextLogSpec OSKextLogSpec@/link</code>
 *           to which command-line <code>-v</code> levels apply.
 */
#define kOSKextLogVerboseFlagsMask       ((OSKextLogSpec) 0x00000ff0)

/*!
 * @define   kOSKextLogConfigMask
 * @abstract Masks the config bits of an
 *           <code>@link OSKextLogSpec OSKextLogSpec@/link</code>.
 */
#define kOSKextLogConfigMask             ((OSKextLogSpec) 0xf0000000)

#if PRAGMA_MARK
/********************************************************************/
#pragma mark 0xF - Log Level
/********************************************************************/
#endif

/*!
 * @define   kOSKextLogExplicitLevel
 * @abstract Used when logging a message to overrides the current log filter,
 *           even if it's set to silent for log messages.
 *           This is essentially a pass-through for
 *           unconditional print messages to go
 *           through the logging engine.
 */
#define kOSKextLogExplicitLevel          ((OSKextLogSpec)        0x0)

/*!
 * @define   kOSKextLogErrorLevel
 * @abstract Log messages concerning error conditions in any category.
 */
#define kOSKextLogErrorLevel            ((OSKextLogSpec)        0x1)


/*!
 * @define   kOSKextLogWarningLevel
 * @abstract Log messages concerning warning conditions in any category,
 *           which indicate potential error conditions,
 *           and notices, which may explain unexpected but correct behavior.
 */
#define kOSKextLogWarningLevel          ((OSKextLogSpec)        0x2)


/*!
 * @define   kOSKextLogBasicLevel
 * @abstract Log messages concerning top-level outcome in any category
 *           (kext load/unload, kext cache creation/extration w/# kexts).
 */
#define kOSKextLogBasicLevel           ((OSKextLogSpec)        0x3)


/*!
 * @define   kOSKextLogProgressLevel
 * @abstract Log messages concerning high-level progress in any category,
 *           such as sending a load request to the kernel,
 *           allocation/link/map/start (load operation),
 *           stop/unmap (unload operation), kext added/extracted (archive).
 */
#define kOSKextLogProgressLevel          ((OSKextLogSpec)        0x4)


/*!
 * @define   kOSKextLogStepLevel
 * @abstract Log messages concerning major steps in any category,
 *           such as sending personalities to the IOCatalogue when loading,
 *           detailed IPC with the kernel, or filtering of kexts for an archive.
 */
#define kOSKextLogStepLevel             ((OSKextLogSpec)        0x5)


/*!
 * @define   kOSKextLogDetailLevel
 * @abstract Log messages concerning specific details in any category,
 *           such as classes being registered/unregistered or
 *           operations on indivdual files in a kext.
 */
#define kOSKextLogDetailLevel           ((OSKextLogSpec)        0x6)


/*!
 * @define   kOSKextLogDebugLevel
 * @abstract Log messages concerning very low-level actions that are
 *           useful mainly for debugging the kext system itself.
 */
#define kOSKextLogDebugLevel             ((OSKextLogSpec)        0x7)


#if PRAGMA_MARK
/********************************************************************/
#pragma mark 0xF0 - General Activity, Load, Kernel IPC, Personalities
/********************************************************************/
#endif

/*!
 * @define   kOSKextLogGeneralFlag
 * @abstract Log messages about general activity in the kext system.
 */
#define kOSKextLogGeneralFlag            ((OSKextLogSpec)       0x10)

/*!
 * @define   kOSKextLogLoadFlag
 * @abstract Log messages regarding kernel extension load, start/stop, or unload activity
 *           in the kernel.
 */
#define kOSKextLogLoadFlag               ((OSKextLogSpec)       0x20)

/*!
 * @define   kOSKextLogIPCFlag
 * @abstract Log messages about any interaction between kernel and user space
 *           regarding kernel extensions.
 */
#define kOSKextLogIPCFlag                ((OSKextLogSpec)       0x40)

/*!
 * @define   kOSKextLogArchiveFlag
 * @abstract Log messages about creating or processing a kext startup cache file
 *           (mkext or prelinked kernel).
 */
#define kOSKextLogArchiveFlag           ((OSKextLogSpec)       0x80)


#if PRAGMA_MARK
/********************************************************************/
#pragma mark 0xF00 - Reserved Verbose Area
/********************************************************************/
#endif
// reserved slot for group               ((OSKextLogSpec)      0x100)
// reserved slot for group               ((OSKextLogSpec)      0x200)
// reserved slot for group               ((OSKextLogSpec)      0x400)
// reserved slot for group               ((OSKextLogSpec)      0x800)

#if PRAGMA_MARK
/********************************************************************/
#pragma mark 0xF000 - Kext diagnostic activity
/********************************************************************/
#endif

/*!
 * @define   kOSKextLogValidationFlag
 * @abstract Log messages when validating kernel extensions.
 */
#define kOSKextLogValidationFlag         ((OSKextLogSpec)     0x1000)

/*!
 * @define   kOSKextLogAuthenticationFlag
 * @abstract Log messages when autnenticating kernel extension files.
 *           Irrelevant in the kernel.
 */
#define kOSKextLogAuthenticationFlag     ((OSKextLogSpec)     0x2000)

/*!
 * @define   kOSKextLogDependenciesFlag
 * @abstract Log messages when resolving dependencies for a kernel extension.
 */
#define kOSKextLogDependenciesFlag       ((OSKextLogSpec)     0x4000)

// reserved slot for group               ((OSKextLogSpec)     0x8000)

#if PRAGMA_MARK
/********************************************************************/
#pragma mark 0xF0000 - Archives, caches, directory scan, file access
/********************************************************************/
#endif

/*!
 * @define   kOSKextLogDirectoryScanFlag
 * @abstract Log messages when scanning directories for kernel extensions.
 *           In the kernel logs every booter kext entry processed.
 */
#define kOSKextLogDirectoryScanFlag      ((OSKextLogSpec)    0x10000)

/*!
 * @define   kOSKextLogFileAccessFlag
 * @abstract Log messages when performing any filesystem access (very verbose).
 *           Irrelevant in the kernel.
 */
#define kOSKextLogFileAccessFlag         ((OSKextLogSpec)    0x20000)

/*!
 * @define   kOSKextLogKextBookkeepingFlag
 * @abstract Log messages about internal tracking of kexts. Can be very verbose.
 */
#define kOSKextLogKextBookkeepingFlag    ((OSKextLogSpec)    0x40000)

// reserved slot for group               ((OSKextLogSpec)    0x80000)

#if PRAGMA_MARK
/********************************************************************/
#pragma mark 0xF00000 - Linking & Patching
/********************************************************************/
#endif

/*!
 * @define   kOSKextLogLinkFlag
 * @abstract Log messages about linking.
 */
#define kOSKextLogLinkFlag               ((OSKextLogSpec)   0x100000)

/*!
 * @define   kOSKextLogPatchFlag
 * @abstract Log messages about patching.
 */
#define kOSKextLogPatchFlag              ((OSKextLogSpec)   0x200000)

// reserved slot for group               ((OSKextLogSpec)   0x400000)
// reserved slot for group               ((OSKextLogSpec)   0x800000)

#if PRAGMA_MARK
/********************************************************************/
#pragma mark 0xF000000 - Reserved
/********************************************************************/
#endif

// reserved slot for grouping            ((OSKextLogSpec)  0x1000000)
// reserved slot for grouping            ((OSKextLogSpec)  0x2000000)
// reserved slot for grouping            ((OSKextLogSpec)  0x4000000)
// reserved slot for grouping            ((OSKextLogSpec)  0x8000000)


#if PRAGMA_MARK
/********************************************************************/
#pragma mark 0xF0000000 - Config Flags
/********************************************************************/
#endif

// reserved slot for grouping            ((OSKextLogSpec) 0x10000000)
// reserved slot for grouping            ((OSKextLogSpec) 0x20000000)
// reserved slot for grouping            ((OSKextLogSpec) 0x40000000)

#if PRAGMA_MARK
/********************************************************************/
#pragma mark Predefined Specifications
/********************************************************************/
#endif

/*!
 * @define   kOSKextLogSilentFilter
 * @abstract For use in filter specs:
 *           Ignore all log messages with a log level greater than
 *           <code>@link kOSKextLogExplicitLevel kOSKextLogExplicitLevel@/link</code>.
 */
#define kOSKextLogSilentFilter           ((OSKextLogSpec)        0x0)

/*!
 * @define   kOSKextLogShowAllFilter
 * @abstract For use in filter specs:
 *           Print all log messages not associated with a kext or
 *           associated with a kext that has
 *           @link //apple_ref/c/macro/kOSBundleEnableKextLoggingKey
 *           OSBundleEnableKextLogging@/link
 *           set to <code>true</code>.
 */
#define kOSKextLogShowAllFilter          ((OSKextLogSpec) 0x0ffffff7)

/*!
 * @define   kOSKextLogShowAllKextsFilter
 * @abstract For use in filter specs:
 *           Print all log messages has
 *           @link //apple_ref/c/macro/kOSBundleEnableKextLoggingKey
 *           OSBundleEnableKextLogging@/link
 *           set to <code>true</code>.
 */
#define kOSKextLogShowAllKextsFilter     ((OSKextLogSpec) \
	                                   (kOSKextLogShowAllFilter | \
	                                    kOSKextLogKextOrGlobalMask))

#if PRAGMA_MARK
#pragma mark -
/********************************************************************/
#pragma mark Kext Version String Processing
/********************************************************************/
#endif
/*!
 * @group Kext Version String Processing
 * Functions for working with kext versions and compatible versions.
 */

/*!
 * @typedef   OSKextVersion
 * @abstract  An encoded kext version that can be compared arithmetically.
 *
 * @discussion
 * A value of zero (<code>@link kOSKextVersionUndefined kOSKextVersionUndefined@/link</code>)
 * is <i>not</i> equivalent to a version string of "0.0",
 * and typically means there is no version specified
 * (for example, that there is no CFBundleVersion property at all).
 * Values below zero are invalid.
 *
 * The encoding used is subject to change,
 * and should never be saved to permanent storage.
 * Always use proper version strings in files and interprocess communication.
 */
typedef int64_t OSKextVersion;

/*!
 * @define    kOSKextVersionMaxLength
 * @abstract  The length of a string buffer
 *            guaranteed to be able to hold a kext version.
 *
 * @discussion
 * Kext versions use an extended Mac OS 'vers' format with double the number
 * of digits before the build stage: ####.##.##s{1-255} where 's'
 * is a build stage 'd', 'a', 'b', 'f' or 'fc'.
 */
#define kOSKextVersionMaxLength (20)
// with a few bytes to spare including a nul byte
// xx-review: Should we make this much bigger in case we ever need longer strings?

/*!
 * @define    kOSKextVersionUndefined
 * @abstract  The undefined version.
 *
 * @discussion
 * This value of <code>@link OSKextVersion OSKextVersion@/link</code> represents the
 * lack of a version
 * (for example, that there is no CFBundleVersion property at all).
 */
#define kOSKextVersionUndefined  (0)

/*!
 * @function OSKextParseVersionString
 *
 * @abstract
 * Parses a kext version string into an <code>@link OSKextVersion OSKextVersion@/link</code>.
 *
 * @param  versionString  The kext version string to parse.
 *
 * @result
 * An encoded kext version that can be compared numerically
 * against other encoded kext versions,
 * <0 if <code>versionString</code> is <code>NULL</code>, empty,
 * or cannot be parsed.
 *
 * @discussion
 * Kext versions use an extended Mac OS 'vers' format with double the number
 * of digits before the build stage: ####.##.##s{1-255} where 's'
 * is a build stage 'd', 'a', 'b', 'f' or 'fc'.
 */
OSKextVersion OSKextParseVersionString(const char * versionString);


/*!
 * @function OSKextVersionGetString
 *
 * @abstract
 * Formats an encoded <code>@link OSKextVersion OSKextVersion@/link</code> into a string
 * representation.
 *
 * @param  aVersion
 *         The encoded version to format.
 * @param  buffer
 *         A C string buffer of at least
 *         <code>@link kOSKextVersionMaxLength kOSKextVersionMaxLength@/link</code> bytes.
 * @param  bufferSize  The size in bytes of <code>buffer</code>.
 *
 * @result
 * <code>TRUE</code> if the encoded version is formatted successfully.
 * <code>FALSE</code> if <code>buffer</code> is <code>NULL</code> or
 * <code>bufferSize</code> is less than
 * <code>@link kOSKextVersionMaxLength kOSKextVersionMaxLength@/link</code>.
 *
 * @discussion
 * The return value strictly indicates whether <code>buffer</code>
 * is large enough to hold the result.
 * If <code>aVersion</code> is 0, the resulting string is "(missing)".
 * If <code>aVersion</code> is less than 0
 * or is not a valid kext version encoding,
 * the resulting string is "(invalid)".
 */
Boolean OSKextVersionGetString(
	OSKextVersion   aVersion,
	char          * buffer,
	uint32_t        bufferSize);


#ifdef KERNEL


#if PRAGMA_MARK
/********************************************************************/
#pragma mark -
#pragma mark Weak linking
/********************************************************************/
#endif
#ifdef XNU_KERNEL_PRIVATE
void kext_weak_symbol_referenced(void) __abortlike;
#endif /* XNU_KERNEL_PRIVATE */

#if PRAGMA_MARK
#pragma mark -
/********************************************************************/
#pragma mark Miscellaneous Kernel-Only Kext Functions
/********************************************************************/
#endif

/*!
 * @function kext_get_vm_map
 * @abstract Returns the vm_map from which the kext was allocated.
 *
 * @param info The kmod_info_t structure of the kext.
 * @result The vm_map from which the kext was allocated.  This function
 * cannot return <code>NULL</code>.
 */
vm_map_t kext_get_vm_map(kmod_info_t * info);

#ifdef XNU_KERNEL_PRIVATE

#if CONFIG_DTRACE
/*!
 * @function OSKextRegisterKextsWithDTrace
 * @abstract
 * DTrace calls this once when it has started up so that the kext system
 * will register any already-loaded kexts with it.
 */
void OSKextRegisterKextsWithDTrace(void);

#endif /* CONFIG_DTRACE */

/*!
 * @function kext_dump_panic_lists
 * @abstract Prints compacted lists of last unloaded & all loaded kexts
 *           during a panic.
 *
 * @param printf_func The printf-style function to use for output.
 */
void kext_dump_panic_lists(int (*printf_func)(const char *fmt, ...));

#endif /* XNU_KERNEL_PRIVATE */

#ifdef XNU_KERNEL_PRIVATE

#if PRAGMA_MARK
#pragma mark -
/********************************************************************/
#pragma mark Kext Loading C Functions
/********************************************************************/
#endif
/*!
 * @function OSKextGetLoadTagForBundleIdentifier
 * @abstract Look up the load tag for a kext.
 *
 * @param  kextIdentifier  The bundle identifier of the kext to look up.
 * @result
 * The load tag of the requested kext, or
 * <code>@link //apple_ref/c/macro/kOSKextInvalidLoadTag kOSKextInvalidLoadTag@/link</code>
 * if the kext was not found.
 *
 * @discussion
 * A load tag uniquely identifies a loaded kext.
 * It can be found as the <code>id</code> field of a loaded kext's
 * <code>kmod_info_t</code> struct.
 *
 * Note that a load tag represents a specific loaded instance of a kext.
 * If that kext is unloaded, the load tag is no longer a valid reference.
 * If the same kext is later reloaded, it will have a new load tag.
 *
 * You can use the load tag to adjust a kext's reference count
 * via
 * <code>@link //apple_ref/c/func/OSKextRetainKextWithLoadTag
 * OSKextRetainKextWithLoadTag@/link</code>
 * and
 * <code>@link //apple_ref/c/func/OSKextReleaseKextWithLoadTag
 * OSKextReleaseKextWithLoadTag@/link</code>,
 * so that the kext is automatically unloaded when no references remain,
 * or to unload the kext immediately
 * with <code>@link //apple_ref/c/func/OSKextUnloadKextWithLoadTag OSKextUnloadKextWithLoadTag@/link</code>.
 *
 * Those functions are intended for use with non-IOKit kexts
 * (specifically, kexts that define no subclasses of
 * <code>@link //apple_ref/doc/class/IOServiceIOService@/link</code>).
 * Pure IOKit kexts are managed via instance counts
 * of their libkern C++ object classes;
 * using those functions on them will only interfere with that mechanism.
 * If you have a hybrid kext with both IOService subclasses and non-IOKit code,
 * however, you may want to use reference counting for the non-IOKit portions:
 * that way the kext will only unload automaticaly
 * when there are no C++ objects and the kext reference count is zero.
 */
uint32_t OSKextGetLoadTagForBundleIdentifier(
	const char * kextIdentifier);


/*!
 * @function OSKextUnloadKextWithLoadTag
 * @abstract Stop and unload a kext based on its load tag.
 *
 * @param  loadTag   The load tag of the kext to unload.
 * @result
 * <code>@link //apple_ref/c/macro/kOSReturnSuccess kOSReturnSuccess@/link</code>
 * if the kext was found and unloaded.
 * <code>@link //apple_ref/c/macro/kOSKextReturnNotFound
 * kOSKextReturnNotFound@/link</code>
 * if the kext was not found.
 * <code>@link //apple_ref/c/macro/kOSKextReturnInUse
 * kOSKextReturnInUse@/link</code>
 * if the kext has outstanding references
 * or if there are instances of its libkern C++ subclasses.
 * Other return values indicate a failure to unload the kext,
 * typically because the module stop routine failed.
 *
 * @discussion
 * A panic will occur if a kext calls this function to unload itself.
 * The safest way for a kext to unload itself is to call
 * <code>@link //apple_ref/c/func/OSKextRetainKextWithLoadTag
 * OSKextRetainKextWithLoadTag@/link</code>
 * with its own load tag
 * (the <code>id</code> field of its <code>kmod_info_t</code> struct),
 * followed by
 * <code>@link //apple_ref/c/func/OSKextReleaseKextWithLoadTag
 * OSKextReleaseKextWithLoadTag@/link</code>;
 * this will schedule the kext for unload on a separate thread.
 *
 * This function can be used when reference-based autounloading is not
 * appropriate.
 * If a kernel system or kext is already monitoring
 * the need for a kext,
 * it can simply call this function when it's known that the kext is not needed.
 */
OSReturn OSKextUnloadKextWithLoadTag(uint32_t loadTag);

#endif /* XNU_KERNEL_PRIVATE */

#endif /* KERNEL */

#if PRAGMA_MARK
#pragma mark -
/********************************************************************/
#pragma mark Loaded Kext Summary
/********************************************************************/
#endif

/*!
 * @define kOSKextLoadedKextSummaryVersion
 * @abstract The current version of the loaded kext summary headers.
 */
#define kOSKextLoadedKextSummaryVersion 2

/*!
 * @typedef OSKextLoadedKextSummary
 * @abstract A structure that describes a loaded kext.
 *
 * @field name The kext's bundle identifier.
 * @field uuid The kext's UUID;
 * @field address The kext's load address.
 * @field size The kext's load size.
 * @field version The binary format (OSKextVersion) version of the kext.
 * @field loadTag The kext's load tag.
 * @field flags Internal tracking flags.
 * @field reference_list who this refs (links on).
 *
 * @discussion
 * The OSKextLoadedKextSummary structure contains a basic set of information
 * about the kext to facilitate kext debugging and panic debug log output.
 */
typedef struct _loaded_kext_summary {
	char        name[KMOD_MAX_NAME];
	uuid_t      uuid;
	uint64_t    address;
	uint64_t    size;
	uint64_t    version;
	uint32_t    loadTag;
	uint32_t    flags;
	uint64_t    reference_list;
} OSKextLoadedKextSummary;

/*!
 * @typedef OSKextLoadedKextSummaryHeader
 * @abstract A structure that describes the set of loaded kext summaries.
 *
 * @field version The version of the loaded kext summary structures.
 * @field entry_size The size of each entry in summaries.
 * @field numSummaries The number of OSKextLoadedKextSummary structures
 *        following the header.
 * @field summaries A convenience pointer to the array of summaries following
 *        the header.
 *
 * @discussion
 * The OSKextLoadedKextSummaryHeader describes the set of loaded kext summaries
 * available for use by the debugger or panic log routine.
 * The array of summaries contains one OSKextLoadedKextSummary for every kext
 * that declares an executable and is not an interface to the kernel.
 */
typedef struct _loaded_kext_summary_header {
	uint32_t version;
	uint32_t entry_size;
	uint32_t numSummaries;
	uint32_t reserved; /* explicit alignment for gdb  */
	OSKextLoadedKextSummary summaries[0];
} OSKextLoadedKextSummaryHeader;

/*!
 * @var gLoadedKextSummaries
 * @abstract The global pointer to the current set of loaded kext summaries.
 */
extern OSKextLoadedKextSummaryHeader * gLoadedKextSummaries;

/*!
 * @var gLoadedKextSummariesTimestamp
 *
 * @abstract This will be set to mach_absolute_time() around updates to
 * gLoadedKextSummaries.  Ie. immediately before gLoadedKextSummaries is set to
 * zero, and immediately after it is set to a new value.
 */
extern uint64_t gLoadedKextSummariesTimestamp;

/*!
 * @function OSKextLoadedKextSummariesUpdated
 * @abstract Called when gLoadedKextSummaries has been updated.
 *
 * @discussion
 * gLoadedKextSummaries is updated when a kext is loaded or unloaded.
 * When the update is complete, OSKextLoadedKextSummariesUpdated is called.
 * gdb can set a breakpoint on this function to detect kext loads and unloads.
 */
void OSKextLoadedKextSummariesUpdated(void);

#ifdef XNU_KERNEL_PRIVATE

extern const vm_allocation_site_t * OSKextGetAllocationSiteForCaller(uintptr_t address);
extern uint32_t                     OSKextGetKmodIDForSite(const vm_allocation_site_t * site,
    char * name, vm_size_t namelen);
extern void                         OSKextFreeSite(vm_allocation_site_t * site);

#if CONFIG_IMAGEBOOT
extern int OSKextGetUUIDForName(const char *, uuid_t);
#endif

extern vm_tag_t gIOSurfaceTag;

extern void *OSKextKextForAddress(const void *addr);

#endif /* XNU_KERNEL_PRIVATE */

__END_DECLS

#endif /* ! _LIBKERN_OSKEXTLIBPRIVATE_H */

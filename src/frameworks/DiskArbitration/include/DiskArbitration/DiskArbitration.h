/*
 * Copyright (c) 1998-2015 Apple Inc. All rights reserved.
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

#ifndef __DISKARBITRATION_DISKARBITRATION__
#define __DISKARBITRATION_DISKARBITRATION__

#include <CoreFoundation/CoreFoundation.h>

#include <DiskArbitration/DADisk.h>
#include <DiskArbitration/DADissenter.h>
#include <DiskArbitration/DASession.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED

/*!
 * @enum       DADiskMountOptions
 * @abstract   Options for DADiskMount().
 * @constant   kDADiskMountOptionWhole Mount the volumes tied to the whole disk object.
 */

enum
{
    kDADiskMountOptionDefault = 0x00000000,
    kDADiskMountOptionWhole   = 0x00000001
};

typedef UInt32 DADiskMountOptions;

/*!
 * @enum       DADiskRenameOptions
 * @abstract   Options for DADiskRename().
 */

enum
{
    kDADiskRenameOptionDefault = 0x00000000
};

typedef UInt32 DADiskRenameOptions;

/*!
 * @enum       DADiskUnmountOptions
 * @abstract   Options for DADiskUnmount().
 * @constant   kDADiskUnmountOptionForce Unmount the volume even if files are still active.
 * @constant   kDADiskUnmountOptionWhole Unmount the volumes tied to the whole disk object.
 */

enum
{
    kDADiskUnmountOptionDefault = 0x00000000,
    kDADiskUnmountOptionForce   = 0x00080000,
    kDADiskUnmountOptionWhole   = 0x00000001
};

typedef UInt32 DADiskUnmountOptions;

/*!
 * @enum       DADiskEjectOptions
 * @abstract   Options for DADiskEject().
 */

enum
{
    kDADiskEjectOptionDefault = 0x00000000
};

typedef UInt32 DADiskEjectOptions;

/*!
 * @enum       DADiskClaimOptions
 * @abstract   Options for DADiskClaim().
 */

enum
{
    kDADiskClaimOptionDefault = 0x00000000
};

typedef UInt32 DADiskClaimOptions;

/*!
 * @enum       DADiskOptions
 * @abstract   Options for DADiskGetOptions() and DADiskSetOptions().
 */

enum
{
    kDADiskOptionDefault = 0x00000000
};

typedef UInt32 DADiskOptions;

/*!
 * @constant   kDADiskDescriptionMatchMediaUnformatted
 * Predefined CFDictionary object containing a set of disk description keys and values
 * appropriate for matching unformatted media using DARegister*Callback().
 */

extern CFDictionaryRef kDADiskDescriptionMatchMediaUnformatted;

/*!
 * @constant   kDADiskDescriptionMatchMediaWhole
 * Predefined CFDictionary object containing a set of disk description keys and values
 * appropriate for matching whole media using DARegister*Callback().
 */

extern CFDictionaryRef kDADiskDescriptionMatchMediaWhole;

/*!
 * @constant   kDADiskDescriptionMatchVolumeMountable
 * Predefined CFDictionary object containing a set of disk description keys and values
 * appropriate for matching mountable volumes using DARegister*Callback().
 */

extern CFDictionaryRef kDADiskDescriptionMatchVolumeMountable;

/*!
 * @constant   kDADiskDescriptionMatchVolumeUnrecognized
 * Predefined CFDictionary object containing a set of disk description keys and values
 * appropriate for matching unrecognized volumes using DARegister*Callback().
 */

extern CFDictionaryRef kDADiskDescriptionMatchVolumeUnrecognized;

/*!
 * @constant   kDADiskDescriptionWatchVolumeName
 * Predefined CFArray object containing a set of disk description keys appropriate for
 * watching volume name changes using DARegisterDiskDescriptionChangedCallback().
 */

extern CFArrayRef kDADiskDescriptionWatchVolumeName;

/*!
 * @constant   kDADiskDescriptionWatchVolumePath
 * Predefined CFArray object containing a set of disk description keys appropriate for
 * watching volume mount changes using DARegisterDiskDescriptionChangedCallback().
 */

extern CFArrayRef kDADiskDescriptionWatchVolumePath;

#ifndef __DISKARBITRATIOND__

/*!
 * @typedef    DADiskAppearedCallback
 * @abstract   Type of the callback function used by DARegisterDiskAppearedCallback().
 * @param      disk    A disk object.
 * @param      context The user-defined context parameter given to the registration function.
 */

typedef void ( *DADiskAppearedCallback )( DADiskRef disk, void * __nullable context );

/*!
 * @function   DARegisterDiskAppearedCallback
 * @abstract   Registers a callback function to be called whenever a disk has appeared.
 * @param      session  The session object.
 * @param      match    The disk description keys to match.  Pass NULL for all disk objects.
 * @param      callback The callback function to call when a disk has appeared.
 * @param      context  The user-defined context parameter to pass to the callback function.
 */

extern void DARegisterDiskAppearedCallback( DASessionRef               session,
                                            CFDictionaryRef __nullable match,
                                            DADiskAppearedCallback     callback,
                                            void * __nullable          context );

/*!
 * @typedef    DADiskDescriptionChangedCallback
 * @abstract   Type of the callback function used by DARegisterDiskDescriptionChangedCallback().
 * @param      disk    A disk object.
 * @param      keys    A list of changed keys.
 * @param      context The user-defined context parameter given to the registration function.
 */

typedef void ( *DADiskDescriptionChangedCallback )( DADiskRef disk, CFArrayRef keys, void * __nullable context );

/*!
 * @function   DARegisterDiskDescriptionChangedCallback
 * @abstract   Registers a callback function to be called whenever a disk description has changed.
 * @param      session  The session object.
 * @param      match    The disk description keys to match.  Pass NULL for all disk objects.
 * @param      watch    The disk description keys to watch.  Pass NULL for all keys.
 * @param      callback The callback function to call when a watched key changes.
 * @param      context  The user-defined context parameter to pass to the callback function.
 */

extern void DARegisterDiskDescriptionChangedCallback( DASessionRef                     session,
                                                      CFDictionaryRef __nullable       match,
                                                      CFArrayRef __nullable            watch,
                                                      DADiskDescriptionChangedCallback callback,
                                                      void * __nullable                context );

/*!
 * @typedef    DADiskDisappearedCallback
 * @abstract   Type of the callback function used by DARegisterDiskDisappearedCallback().
 * @param      disk    A disk object.
 * @param      context The user-defined context parameter given to the registration function.
 */

typedef void ( *DADiskDisappearedCallback )( DADiskRef disk, void * __nullable context );

/*!
 * @function   DARegisterDiskDisappearedCallback
 * @abstract   Registers a callback function to be called whenever a disk has disappeared.
 * @param      session  The session object.
 * @param      match    The disk description keys to match.  Pass NULL for all disk objects.
 * @param      callback The callback function to call when a disk has disappeared.
 * @param      context  The user-defined context parameter to pass to the callback function.
 */

extern void DARegisterDiskDisappearedCallback( DASessionRef               session,
                                               CFDictionaryRef __nullable match,
                                               DADiskDisappearedCallback  callback,
                                               void * __nullable          context );

/*!
 * @typedef    DADiskMountCallback
 * @abstract   Type of the callback function used by DADiskMount().
 * @param      disk      The disk object.
 * @param      dissenter A dissenter object on failure or NULL on success.
 * @param      context   The user-defined context parameter given to the mount function.
 * @discussion
 * If the disk is already mounted, then status code in the dissenter object will be set to kDAReturnBusy
 */

typedef void ( *DADiskMountCallback )( DADiskRef disk, DADissenterRef __nullable dissenter, void * __nullable context );

/*!
 * @function   DADiskMount
 * @abstract   Mounts the volume at the specified disk object.
 * @param      disk     The disk object.
 * @param      path     The mount path.  Pass NULL for a "standard" mount path.
 * @param      options  The mount options.
 * @param      callback The callback function to call once the mount completes.
 * @param      context  The user-defined context parameter to pass to the callback function.
 */

extern void DADiskMount( DADiskRef                      disk,
                         CFURLRef __nullable            path,
                         DADiskMountOptions             options,
                         DADiskMountCallback __nullable callback,
                         void * __nullable              context );

/*!
 * @function   DADiskMountWithArguments
 * @abstract   Mounts the volume at the specified disk object, with the specified mount options.
 * @param      disk      The disk object.
 * @param      path      The mount path.  Pass NULL for a "standard" mount path.
 * @param      options   The mount options.
 * @param      callback  The callback function to call once the mount completes.
 * @param      context   The user-defined context parameter to pass to the callback function.
 * @param      arguments The null-terminated list of mount options to pass to /sbin/mount -o.
 */

extern void DADiskMountWithArguments( DADiskRef                      disk,
                                      CFURLRef __nullable            path,
                                      DADiskMountOptions             options,
                                      DADiskMountCallback __nullable callback,
                                      void * __nullable              context,
                                      CFStringRef __nullable         arguments[_Nullable] );

/*!
 * @typedef    DADiskMountApprovalCallback
 * @abstract   Type of the callback function used by DARegisterDiskMountApprovalCallback().
 * @param      disk    A disk object.
 * @param      context The user-defined context parameter given to the registration function.
 * @result     A dissenter reference.  Pass NULL to approve.
 * @discussion
 * The caller of this callback receives a reference to the returned object.  The
 * caller also implicitly retains the object and is responsible for releasing it
 * with CFRelease().
 */

typedef DADissenterRef __nullable ( *DADiskMountApprovalCallback )( DADiskRef disk, void * __nullable context );

/*!
 * @function   DARegisterDiskMountApprovalCallback
 * @abstract   Registers a callback function to be called whenever a volume is to be mounted.
 * @param      session  The session object.
 * @param      match    The disk description keys to match.  Pass NULL for all disk objects.
 * @param      callback The callback function to call when a volume is to be mounted.
 * @param      context  The user-defined context parameter to pass to the callback function.
 */

extern void DARegisterDiskMountApprovalCallback( DASessionRef                session,
                                                 CFDictionaryRef __nullable  match,
                                                 DADiskMountApprovalCallback callback,
                                                 void * __nullable           context );

/*!
 * @typedef    DADiskRenameCallback
 * @abstract   Type of the callback function used by DADiskRename().
 * @param      disk      The disk object.
 * @param      dissenter A dissenter object on failure or NULL on success.
 * @param      context   The user-defined context parameter given to the rename function.
 */

typedef void ( *DADiskRenameCallback )( DADiskRef disk, DADissenterRef __nullable dissenter, void * __nullable context );

/*!
 * @function   DADiskRename
 * @abstract   Renames the volume at the specified disk object.
 * @param      disk     The disk object.
 * @param      options  The rename options.
 * @param      callback The callback function to call once the rename completes.
 * @param      context  The user-defined context parameter to pass to the callback function.
 */

extern void DADiskRename( DADiskRef                       disk,
                          CFStringRef                     name,
                          DADiskRenameOptions             options,
                          DADiskRenameCallback __nullable callback,
                          void * __nullable               context );

/*!
 * @typedef    DADiskUnmountCallback
 * @abstract   Type of the callback function used by DADiskUnmount().
 * @param      disk      The disk object.
 * @param      dissenter A dissenter object on failure or NULL on success.
 * @param      context   The user-defined context parameter given to the unmount function.
 */

typedef void ( *DADiskUnmountCallback )( DADiskRef disk, DADissenterRef __nullable dissenter, void * __nullable context );

/*!
 * @function   DADiskUnmount
 * @abstract   Unmounts the volume at the specified disk object.
 * @param      disk     The disk object.
 * @param      options  The unmount options.
 * @param      callback The callback function to call once the unmount completes.
 * @param      context  The user-defined context parameter to pass to the callback function.
 */

extern void DADiskUnmount( DADiskRef                        disk,
                           DADiskUnmountOptions             options,
                           DADiskUnmountCallback __nullable callback,
                           void * __nullable                context );

/*!
 * @typedef    DADiskUnmountApprovalCallback
 * @abstract   Type of the callback function used by DARegisterDiskUnmountApprovalCallback().
 * @param      disk    A disk object.
 * @param      context The user-defined context parameter given to the registration function.
 * @result     A dissenter reference.  Pass NULL to approve.
 * @discussion
 * The caller of this callback receives a reference to the returned object.  The
 * caller also implicitly retains the object and is responsible for releasing it
 * with CFRelease().
 */

typedef DADissenterRef __nullable ( *DADiskUnmountApprovalCallback )( DADiskRef disk, void * __nullable context );

/*!
 * @function   DARegisterDiskUnmountApprovalCallback
 * @abstract   Registers a callback function to be called whenever a volume is to be unmounted.
 * @param      session  The session object.
 * @param      match    The disk description keys to match.  Pass NULL for all disk objects.
 * @param      callback The callback function to call when a volume is to be unmounted.
 * @param      context  The user-defined context parameter to pass to the callback function.
 */

extern void DARegisterDiskUnmountApprovalCallback( DASessionRef                  session,
                                                   CFDictionaryRef __nullable    match,
                                                   DADiskUnmountApprovalCallback callback,
                                                   void * __nullable             context );

/*!
 * @typedef    DADiskEjectCallback
 * @abstract   Type of the callback function used by DADiskEject().
 * @param      disk      The disk object.
 * @param      dissenter A dissenter object on failure or NULL on success.
 * @param      context   The user-defined context parameter given to the eject function.
 */

typedef void ( *DADiskEjectCallback )( DADiskRef disk, DADissenterRef __nullable dissenter, void * __nullable context );

/*!
 * @function   DADiskEject
 * @abstract   Ejects the specified disk object.
 * @param      disk     The disk object.
 * @param      options  The eject options.
 * @param      callback The callback function to call once the ejection completes.
 * @param      context  The user-defined context parameter to pass to the callback function.
 */

extern void DADiskEject( DADiskRef                      disk,
                         DADiskEjectOptions             options,
                         DADiskEjectCallback __nullable callback,
                         void * __nullable              context );

/*!
 * @typedef    DADiskEjectApprovalCallback
 * @abstract   Type of the callback function used by DARegisterDiskEjectApprovalCallback().
 * @param      disk    A disk object.
 * @param      context The user-defined context parameter given to the registration function.
 * @result     A dissenter reference.  Pass NULL to approve.
 * @discussion
 * The caller of this callback receives a reference to the returned object.  The
 * caller also implicitly retains the object and is responsible for releasing it
 * with CFRelease().
 */

typedef DADissenterRef __nullable ( *DADiskEjectApprovalCallback )( DADiskRef disk, void * __nullable context );

/*!
 * @function   DARegisterDiskEjectApprovalCallback
 * @abstract   Registers a callback function to be called whenever a volume is to be ejected.
 * @param      session  The session object.
 * @param      match    The disk description keys to match.  Pass NULL for all disk objects.
 * @param      callback The callback function to call when a volume is to be ejected.
 * @param      context  The user-defined context parameter to pass to the callback function.
 */

extern void DARegisterDiskEjectApprovalCallback( DASessionRef                session,
                                                 CFDictionaryRef __nullable  match,
                                                 DADiskEjectApprovalCallback callback,
                                                 void * __nullable           context );

/*!
 * @typedef    DADiskClaimCallback
 * @abstract   Type of the callback function used by DADiskClaim().
 * @param      disk      The disk object.
 * @param      dissenter A dissenter object on failure or NULL on success.
 * @param      context   The user-defined context parameter given to the claim function.
 */

typedef void ( *DADiskClaimCallback )( DADiskRef disk, DADissenterRef __nullable dissenter, void * __nullable context );

/*!
 * @typedef    DADiskClaimReleaseCallback
 * @abstract   Type of the callback function used by DADiskClaim().
 * @param      disk    The disk object.
 * @param      context The user-defined context parameter given to the claim function.
 * @result     A dissenter reference.  Pass NULL to release claim.
 * @discussion
 * The caller of this callback receives a reference to the returned object.  The
 * caller also implicitly retains the object and is responsible for releasing it
 * with CFRelease().
 */

typedef DADissenterRef __nullable ( *DADiskClaimReleaseCallback )( DADiskRef disk, void * __nullable context );

/*!
 * @function   DADiskClaim
 * @abstract   Claims the specified disk object for exclusive use.
 * @param      disk            The disk object.
 * @param      options         The claim options.
 * @param      release         The callback function to call when the claim is to be released.
 * @param      releaseContext  The user-defined context parameter to pass to the callback function.
 * @param      callback        The callback function to call once the claim completes.
 * @param      callbackContext The user-defined context parameter to pass to the callback function.
 */

extern void DADiskClaim( DADiskRef                             disk,
                         DADiskClaimOptions                    options,
                         DADiskClaimReleaseCallback __nullable release,
                         void * __nullable                     releaseContext,
                         DADiskClaimCallback __nullable        callback,
                         void * __nullable                     callbackContext );

/*!
 * @function   DADiskIsClaimed
 * @abstract   Reports whether or not the disk is claimed.
 * @param      disk The disk object.
 * @result     TRUE if the disk is claimed, otherwise FALSE.
 */

extern Boolean DADiskIsClaimed( DADiskRef disk );

/*!
 * @function   DADiskUnclaim
 * @abstract   Unclaims the specified disk object.
 * @param      disk The disk object.
 */

extern void DADiskUnclaim( DADiskRef disk );

/*!
 * @typedef    DADiskPeekCallback
 * @abstract   Type of the callback function used by DARegisterDiskPeekCallback().
 * @param      disk    A disk object.
 * @param      context The user-defined context parameter given to the registration function.
 * @discussion
 * The peek callback functions are called in a specific order, from lowest order to highest
 * order.  DADiskClaim() could be used here to claim the disk object and DADiskSetOptions()
 * could be used here to set up options on the disk object.
 */

typedef void ( *DADiskPeekCallback )( DADiskRef disk, void * __nullable context );

/*!
 * @function   DARegisterDiskPeekCallback
 * @abstract   Registers a callback function to be called whenever a disk has been probed.
 * @param      session  The session object.
 * @param      match    The disk description keys to match.  Pass NULL for all disk objects.
 * @param      order    The callback order, from lowest to highest.  Pass 0 for the default.
 * @param      callback The callback function to call when a disk has been probed.
 * @param      context  The user-defined context parameter to pass to the callback function.
 */

extern void DARegisterDiskPeekCallback( DASessionRef               session,
                                        CFDictionaryRef __nullable match,
                                        CFIndex                    order,
                                        DADiskPeekCallback         callback,
                                        void * __nullable          context );

/*!
 * @function   DADiskGetOptions
 * @abstract   Obtains the options for the specified disk.
 * @param      disk The disk object for which to obtain the options.
 * @result     The options.
 */

extern DADiskOptions DADiskGetOptions( DADiskRef disk );

/*!
 * @function   DADiskSetOptions
 * @abstract   Sets the options for the specified disk.
 * @param      disk    The disk object for which to set the options.
 * @param      options The options to set or clear.
 * @param      value   Pass TRUE to set options; otherwise pass FALSE to clear options.
 * @result     A result code.
 */

extern DAReturn DADiskSetOptions( DADiskRef disk, DADiskOptions options, Boolean value );

/*!
 * @function   DAUnregisterCallback
 * @abstract   Unregisters a registered callback function.
 * @param      session  The session object.
 * @param      callback The registered callback function.
 * @param      context  The user-defined context parameter.
 */

extern void DAUnregisterCallback( DASessionRef session, void * callback, void * __nullable context );

/*
 * @function   DAUnregisterApprovalCallback
 * @abstract   Unregisters a registered callback function.
 * @param      session  The session object.
 * @param      callback The registered callback function.
 * @param      context  The user-defined context parameter.
 */

extern void DAUnregisterApprovalCallback( DASessionRef session, void * callback, void * __nullable context ) CF_SWIFT_UNAVAILABLE( "Use DAUnregisterCallback instead" );

#endif /* !__DISKARBITRATIOND__ */

CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !__DISKARBITRATION_DISKARBITRATION__ */

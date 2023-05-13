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

#ifndef __DISKARBITRATION_DADISK__
#define __DISKARBITRATION_DADISK__

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

#include <DiskArbitration/DASession.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED

extern const CFStringRef kDADiskDescriptionVolumeKindKey;      /* ( CFString     ) */
extern const CFStringRef kDADiskDescriptionVolumeMountableKey; /* ( CFBoolean    ) */
extern const CFStringRef kDADiskDescriptionVolumeNameKey;      /* ( CFString     ) */
extern const CFStringRef kDADiskDescriptionVolumeNetworkKey;   /* ( CFBoolean    ) */
extern const CFStringRef kDADiskDescriptionVolumePathKey;      /* ( CFURL        ) */
extern const CFStringRef kDADiskDescriptionVolumeTypeKey;      /* ( CFString     ) */
extern const CFStringRef kDADiskDescriptionVolumeUUIDKey;      /* ( CFUUID       ) */

extern const CFStringRef kDADiskDescriptionMediaBlockSizeKey;  /* ( CFNumber     ) */
extern const CFStringRef kDADiskDescriptionMediaBSDMajorKey;   /* ( CFNumber     ) */
extern const CFStringRef kDADiskDescriptionMediaBSDMinorKey;   /* ( CFNumber     ) */
extern const CFStringRef kDADiskDescriptionMediaBSDNameKey;    /* ( CFString     ) */
extern const CFStringRef kDADiskDescriptionMediaBSDUnitKey;    /* ( CFNumber     ) */
extern const CFStringRef kDADiskDescriptionMediaContentKey;    /* ( CFString     ) */
extern const CFStringRef kDADiskDescriptionMediaEjectableKey;  /* ( CFBoolean    ) */
extern const CFStringRef kDADiskDescriptionMediaIconKey;       /* ( CFDictionary ) */
extern const CFStringRef kDADiskDescriptionMediaKindKey;       /* ( CFString     ) */
extern const CFStringRef kDADiskDescriptionMediaLeafKey;       /* ( CFBoolean    ) */
extern const CFStringRef kDADiskDescriptionMediaNameKey;       /* ( CFString     ) */
extern const CFStringRef kDADiskDescriptionMediaPathKey;       /* ( CFString     ) */
extern const CFStringRef kDADiskDescriptionMediaRemovableKey;  /* ( CFBoolean    ) */
extern const CFStringRef kDADiskDescriptionMediaSizeKey;       /* ( CFNumber     ) */
extern const CFStringRef kDADiskDescriptionMediaTypeKey;       /* ( CFString     ) */
extern const CFStringRef kDADiskDescriptionMediaUUIDKey;       /* ( CFUUID       ) */
extern const CFStringRef kDADiskDescriptionMediaWholeKey;      /* ( CFBoolean    ) */
extern const CFStringRef kDADiskDescriptionMediaWritableKey;   /* ( CFBoolean    ) */
extern const CFStringRef kDADiskDescriptionMediaEncryptedKey;  /* ( CFBoolean    ) */
extern const CFStringRef kDADiskDescriptionMediaEncryptionDetailKey; /* ( CFNumber ) */

extern const CFStringRef kDADiskDescriptionDeviceGUIDKey;      /* ( CFData       ) */
extern const CFStringRef kDADiskDescriptionDeviceInternalKey;  /* ( CFBoolean    ) */
extern const CFStringRef kDADiskDescriptionDeviceModelKey;     /* ( CFString     ) */
extern const CFStringRef kDADiskDescriptionDevicePathKey;      /* ( CFString     ) */
extern const CFStringRef kDADiskDescriptionDeviceProtocolKey;  /* ( CFString     ) */
extern const CFStringRef kDADiskDescriptionDeviceRevisionKey;  /* ( CFString     ) */
extern const CFStringRef kDADiskDescriptionDeviceUnitKey;      /* ( CFNumber     ) */
extern const CFStringRef kDADiskDescriptionDeviceVendorKey;    /* ( CFString     ) */
extern const CFStringRef kDADiskDescriptionDeviceTDMLockedKey; /* ( CFBoolean    ) */

extern const CFStringRef kDADiskDescriptionBusNameKey;         /* ( CFString     ) */
extern const CFStringRef kDADiskDescriptionBusPathKey;         /* ( CFString     ) */

#ifndef __DISKARBITRATIOND__

/*!
 * @typedef    DADiskRef
 * Type of a reference to DADisk instances.
 */

typedef struct CF_BRIDGED_TYPE( id ) __DADisk * DADiskRef;

/*!
 * @function   DADiskGetTypeID
 * @abstract   Returns the type identifier of all DADisk instances.
 */

extern CFTypeID DADiskGetTypeID( void );

/*!
 * @function   DADiskCreateFromBSDName
 * @abstract   Creates a new disk object.
 * @param      allocator The allocator object to be used to allocate memory.
 * @param      session   The DASession in which to contact Disk Arbitration.
 * @param      name      The BSD device name.
 * @result     A reference to a new DADisk.
 * @discussion
 * The caller of this function receives a reference to the returned object.  The
 * caller also implicitly retains the object and is responsible for releasing it
 * with CFRelease().
 */

extern DADiskRef __nullable DADiskCreateFromBSDName( CFAllocatorRef __nullable allocator, DASessionRef session, const char * name );

/*!
 * @function   DADiskCreateFromIOMedia
 * @abstract   Creates a new disk object.
 * @param      allocator The allocator object to be used to allocate memory.
 * @param      session   The DASession in which to contact Disk Arbitration.
 * @param      media     The I/O Kit media object.
 * @result     A reference to a new DADisk.
 * @discussion
 * The caller of this function receives a reference to the returned object.  The
 * caller also implicitly retains the object and is responsible for releasing it
 * with CFRelease().
 */

extern DADiskRef __nullable DADiskCreateFromIOMedia( CFAllocatorRef __nullable allocator, DASessionRef session, io_service_t media );

/*!
 * @function   DADiskCreateFromVolumePath
 * @abstract   Creates a new disk object.
 * @param      allocator The allocator object to be used to allocate memory.
 * @param      session   The DASession in which to contact Disk Arbitration.
 * @param      path      The BSD mount point.
 * @result     A reference to a new DADisk.
 * @discussion
 * The caller of this function receives a reference to the returned object.  The
 * caller also implicitly retains the object and is responsible for releasing it
 * with CFRelease().
 */

extern DADiskRef __nullable DADiskCreateFromVolumePath( CFAllocatorRef __nullable allocator, DASessionRef session, CFURLRef path );

/*!
 * @function   DADiskGetBSDName
 * @abstract   Obtains the BSD device name for the specified disk.
 * @param      disk The DADisk for which to obtain the BSD device name.
 * @result     The disk's BSD device name.
 * @discussion
 * The BSD device name can be used with opendev() to open the BSD device.
 */

extern const char * __nullable DADiskGetBSDName( DADiskRef disk );

/*!
 * @function   DADiskCopyIOMedia
 * @abstract   Obtains the I/O Kit media object for the specified disk.
 * @param      disk The DADisk for which to obtain the I/O Kit media object.
 * @result     The disk's I/O Kit media object.
 * @discussion
 * The caller of this function receives a reference to the returned object.  The
 * caller also implicitly retains the object and is responsible for releasing it
 * with IOObjectRelease().
 */

extern io_service_t DADiskCopyIOMedia( DADiskRef disk );

/*!
 * @function   DADiskCopyDescription
 * @abstract   Obtains the Disk Arbitration description of the specified disk.
 * @param      disk The DADisk for which to obtain the Disk Arbitration description.
 * @result     The disk's Disk Arbitration description.
 * @discussion
 * This function will contact Disk Arbitration to acquire the latest description
 * of the specified disk, unless this function is called on a disk object passed
 * within the context of a registered callback, in which case the description is
 * current as of that callback event.
 *
 * The caller of this function receives a reference to the returned object.  The
 * caller also implicitly retains the object and is responsible for releasing it
 * with CFRelease().
 */

extern CFDictionaryRef __nullable DADiskCopyDescription( DADiskRef disk );

/*!
 * @function   DADiskCopyWholeDisk
 * @abstract   Obtain the associated whole disk object for the specified disk.
 * @param      disk The disk object.
 * @result     The disk's associated whole disk object.
 * @discussion
 * The caller of this function receives a reference to the returned object.  The
 * caller also implicitly retains the object and is responsible for releasing it
 * with CFRelease().
 */

extern DADiskRef __nullable DADiskCopyWholeDisk( DADiskRef disk );

#endif /* !__DISKARBITRATIOND__ */

CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !__DISKARBITRATION_DADISK__ */

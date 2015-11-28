/*
 * Copyright (c) 2002, 2004, 2005 Apple Computer, Inc. All rights reserved.
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


#ifndef _DEVICEONHOLD_H
#define _DEVICEONHOLD_H

#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>


/*!
	@header DeviceOnHold
 */


/*!
	@enum
	@discussion Returned status codes from DeviceOnHoldGetStatus()

	@constant kDeviceIdle
	@constant kDeviceConnecting
	@constant kDeviceDataConnectionActive
	@constant kDeviceDataConnectionOnHold
	@constant kDeviceDisconnecting
*/
enum
{
	kDeviceIdle			= 0,
	kDeviceConnecting,
	kDeviceDataConnectionActive,
	kDeviceDataConnectionOnHold,
	kDeviceDisconnecting
};


/*!
	@typedef DeviceOnHoldRef
	@discussion This is the handle to a specific device
		which is used by the DeviceOnHold APIs.
 */
typedef const struct __DeviceOnHoldRef * DeviceOnHoldRef;


__BEGIN_DECLS


/*!
	@function IsDeviceOnHoldSupported
	@discussion Determines whether a device has the capability to have
		a connection placed "on hold".

	This function determines whether the device specified supports
	the "on hold" capabality.

	@param devname A CFStringRef that represents the device being queried.
	@param options A CFDictionaryRef of various options for the device.
	@result TRUE if device supports "on hold".

 */
Boolean
IsDeviceOnHoldSupported			(
					CFStringRef		devname,     // e.g. "modem"
					CFDictionaryRef		options
					);

/*!
	@function DeviceOnHoldCreate
	@discussion Creates a DeviceOnHoldRef for the specified device supports.

	This function creates a DeviceOnHoldRef handle which will be used
		in all subsequent calls to the "on hold" API's.

	@param allocator A CFAllocatorRef.
	@param devname A CFStringRef that represents the device being queried.
	@param options A CFDictionaryRef of various options for the device.
	@result DeviceOnHoldRef to pass to subsequent device hold api's.

 */
DeviceOnHoldRef
DeviceOnHoldCreate			(
					CFAllocatorRef		allocator,
					CFStringRef		devname,     // e.g. "modem"
					CFDictionaryRef		options
					);


/*!
	@function DeviceOnHoldGetStatus
	@discussion Returns the "on hold" status of the device.

	@param deviceRef A DeviceOnHoldRef.
	@result The status of device.

 */
int32_t
DeviceOnHoldGetStatus			(
					DeviceOnHoldRef		deviceRef
					);


/*!
	@function DeviceOnHoldSuspend
	@discussion Tells the device to go "on hold".

	This function will signal the deviceRef to suspend operations

	@param deviceRef A DeviceOnHoldRef.
	@result Boolean if call succeeded.

 */
Boolean
DeviceOnHoldSuspend			(
					DeviceOnHoldRef		deviceRef
					);

/*!
	@function DeviceOnHoldResume
	@discussion Tells the device to resume.

	This function will signal the deviceRef to resume operations

	@param deviceRef A DeviceOnHoldRef.
	@result Boolean if call succeeded.

 */
Boolean
DeviceOnHoldResume			(
					DeviceOnHoldRef		deviceRef
					);

__END_DECLS

#endif	/* _DEVICEONHOLD_H */


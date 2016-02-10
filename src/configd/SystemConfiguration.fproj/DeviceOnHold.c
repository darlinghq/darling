/*
 * Copyright (c) 2002-2005 Apple Computer, Inc. All rights reserved.
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

/*
 * Modification History
 *
 * May 29, 2002		Roger Smith <rsmith@apple.com>
 * - initial revision
 */

#include <sys/types.h>
#include <mach/mach.h>
#include <pthread.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCValidation.h>
#include <SystemConfiguration/SCPrivate.h>

#include <IOKit/IOKitLib.h>
#include "dy_framework.h"

#include "moh_msg.h"
#include "moh.h"
#include "DeviceOnHold.h"


#define kIODeviceSupportsHoldKey	"V92Modem"


typedef struct {

	/* base CFType information */
	CFRuntimeBase	cfBase;

	/* device name (e.g. "modem") */
	CFStringRef	name;

	int		sock;

} DeviceOnHoldPrivate, *DeviceOnHoldPrivateRef;


static CFStringRef
__DeviceOnHoldCopyDescription(CFTypeRef cf)
{
	CFAllocatorRef		allocator	= CFGetAllocator(cf);
	CFMutableStringRef	result;

	result = CFStringCreateMutable(allocator, 0);
	CFStringAppendFormat(result, NULL, CFSTR("<DeviceOnHold %p [%p]> {\n"), cf, allocator);
	CFStringAppendFormat(result, NULL, CFSTR("}"));

	return result;
}


static void
__DeviceOnHoldDeallocate(CFTypeRef cf)
{
	DeviceOnHoldPrivateRef	DeviceOnHoldPrivate	= (DeviceOnHoldPrivateRef)cf;

	/* release resources */
	if (DeviceOnHoldPrivate->name)		CFRelease(DeviceOnHoldPrivate->name);
	if (DeviceOnHoldPrivate->sock != -1) {

	}

	return;
}


static CFTypeID __kDeviceOnHoldTypeID = _kCFRuntimeNotATypeID;


static const CFRuntimeClass __DeviceOnHoldClass = {
	0,				// version
	"DeviceOnHold",			// className
	NULL,				// init
	NULL,				// copy
	__DeviceOnHoldDeallocate,	// dealloc
	NULL,				// equal
	NULL,				// hash
	NULL,				// copyFormattingDesc
	__DeviceOnHoldCopyDescription	// copyDebugDesc
};


static pthread_once_t initialized	= PTHREAD_ONCE_INIT;

static void
__DeviceOnHoldInitialize(void)
{
	__kDeviceOnHoldTypeID = _CFRuntimeRegisterClass(&__DeviceOnHoldClass);
	return;
}


static DeviceOnHoldPrivateRef
__DeviceOnHoldCreatePrivate(CFAllocatorRef	allocator)
{
	DeviceOnHoldPrivateRef	devicePrivate;
	uint32_t		size;

	/* initialize runtime */
	pthread_once(&initialized, __DeviceOnHoldInitialize);

	/* allocate session */
	size          = sizeof(DeviceOnHoldPrivate) - sizeof(CFRuntimeBase);
	devicePrivate = (DeviceOnHoldPrivateRef)_CFRuntimeCreateInstance(allocator,
									 __kDeviceOnHoldTypeID,
									 size,
									 NULL);
	if (!devicePrivate) {
		return NULL;
	}

	devicePrivate->name	= NULL;
	devicePrivate->sock	= -1;

	return devicePrivate;
}


CFTypeID
DeviceOnHoldGetTypeID(void) {
	pthread_once(&initialized, __DeviceOnHoldInitialize);	/* initialize runtime */
	return __kDeviceOnHoldTypeID;
}


/*
 * TBD: We determine whether a device supports on hold capability by looking at
 * the numeric property DeviceSupportsHold (1 - yes, 0 or no property - no). For
 * the Apple Dash II internal modem we also use the property V92Modem to track
 * this same capability.
 */

Boolean
IsDeviceOnHoldSupported(CFStringRef	deviceName,	// "modem"
			CFDictionaryRef	options)
{
	CFMutableDictionaryRef	deviceToMatch;
	uint32_t		deviceSupportsHoldValue;
	kern_return_t		kr;
	static mach_port_t	masterPort			= MACH_PORT_NULL;
	io_iterator_t		matchingServices;
	CFNumberRef		num;
	CFMutableDictionaryRef	properties;
	Boolean			result				= FALSE;
	io_service_t 		service;

	if (CFStringCompare(deviceName, CFSTR("modem"), 0) == kCFCompareEqualTo) {
		if (masterPort == MACH_PORT_NULL) {
			kr = IOMasterPort(MACH_PORT_NULL, &masterPort);
			if (kr != KERN_SUCCESS) {
				return FALSE;
			}
		}

		deviceToMatch = IOServiceMatching("InternalModemSupport");
		if (deviceToMatch == NULL) {
			return FALSE;
		}

		kr = IOServiceGetMatchingServices(masterPort, deviceToMatch, &matchingServices);
		if (kr != KERN_SUCCESS) {
			return FALSE;
		}

		for ( ; (service = IOIteratorNext(matchingServices)) ; IOObjectRelease(service)) {
			io_string_t	path;

			kr = IORegistryEntryGetPath(service, kIOServicePlane, path);
			assert( kr == KERN_SUCCESS );

			// grab a copy of the properties
			kr = IORegistryEntryCreateCFProperties(service, &properties, kCFAllocatorDefault, kNilOptions);
			assert( kr == KERN_SUCCESS );

			num = CFDictionaryGetValue(properties, CFSTR(kIODeviceSupportsHoldKey));
			if (isA_CFNumber(num)) {
				CFNumberGetValue(num, kCFNumberSInt32Type, &deviceSupportsHoldValue);
				if (deviceSupportsHoldValue == 1) {
					result = TRUE;
				}
			}

			CFRelease(properties);
		}

		IOObjectRelease(matchingServices);
	}

	// Note: The issue for the general case is how to go from the SystemConfiguration
	//       dynamic store to the actual driver. The devicesupportshold property is not
	//       copied the either of the setup/state descriptions so the caller would need
	//       to know the exact driver they are searching for.

	return result;
}


DeviceOnHoldRef
DeviceOnHoldCreate(CFAllocatorRef	allocator,
		   CFStringRef		deviceName,	// "modem"
		   CFDictionaryRef	options)
{
	DeviceOnHoldPrivateRef	devicePrivate;
	int			status;

	if (CFStringCompare(deviceName, CFSTR("modem"), 0) != kCFCompareEqualTo) {
		return NULL;
	}

	devicePrivate = __DeviceOnHoldCreatePrivate(allocator);
	if (!devicePrivate) {
		return NULL;
	}

	status = MOHInit(&devicePrivate->sock, deviceName);
	if (status != 0) {
		CFRelease(devicePrivate);
		return NULL;
	}

	devicePrivate->name = CFStringCreateCopy(NULL, deviceName);

	return (DeviceOnHoldRef)devicePrivate;
}



int32_t
DeviceOnHoldGetStatus(DeviceOnHoldRef	device)
{
	DeviceOnHoldPrivateRef	devicePrivate	= (DeviceOnHoldPrivateRef)device;
	int			err;
	uint32_t		link		= 1;
	void			*replyBuf;
	size_t			replyBufLen;
	int32_t			result		= -1;

	if (!device) {
		return -1;
	}

	if (devicePrivate->sock == -1) {
		return -1;
	}

	err = MOHExec(devicePrivate->sock,
		      link,
		      MOH_SESSION_GET_STATUS,
		      NULL,
		      0,
		      &replyBuf,
		      &replyBufLen);

	if (err != 0) {
		return -1;
	}

	if (replyBufLen == sizeof(result)) {
		result = *(int32_t *)replyBuf;
	}

	if (replyBuf)	CFAllocatorDeallocate(NULL, replyBuf);
	return result;
}


Boolean
DeviceOnHoldSuspend(DeviceOnHoldRef	device)
{
	DeviceOnHoldPrivateRef	devicePrivate	= (DeviceOnHoldPrivateRef)device;
	int			err;
	uint32_t		link		= 1;
	void			*replyBuf;
	size_t			replyBufLen;
	Boolean			result		= FALSE;

	if (!device) {
		return FALSE;
	}

	if (devicePrivate->sock == -1) {
		return FALSE;
	}

	err = MOHExec(devicePrivate->sock,
		      link,
		      MOH_PUT_SESSION_ON_HOLD,
		      NULL,
		      0,
		      &replyBuf,
		      &replyBufLen);

	if (err != 0) {
		return -1;
	}

	if (replyBufLen == sizeof(result)) {
		result = (*(int32_t *)replyBuf) ? TRUE : FALSE;
	}

	if (replyBuf)	CFAllocatorDeallocate(NULL, replyBuf);
	return result;
}


Boolean
DeviceOnHoldResume(DeviceOnHoldRef	device)
{
	DeviceOnHoldPrivateRef	devicePrivate	= (DeviceOnHoldPrivateRef)device;
	int			err;
	uint32_t		link		= 1;
	void			*replyBuf;
	size_t			replyBufLen;
	Boolean			result		= FALSE;

	if (!device) {
		return FALSE;
	}

	if (devicePrivate->sock == -1) {
		return FALSE;
	}

	err = MOHExec(devicePrivate->sock,
		      link,
		      MOH_RESUME_SESSION_ON_HOLD,NULL,
		      0,
		      &replyBuf,
		      &replyBufLen);

	if (err != 0) {
		return -1;
	}

	if (replyBufLen == sizeof(result)) {
		result = (*(int32_t *)replyBuf) ? TRUE : FALSE;
	}

	if (replyBuf)	CFAllocatorDeallocate(NULL, replyBuf);
	return result;
}

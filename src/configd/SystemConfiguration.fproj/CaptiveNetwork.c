/*
 * Copyright (c) 2009, 2010, 2012, 2013, 2015, 2018, 2019 Apple Inc. All rights reserved.
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


#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <SystemConfiguration/CaptiveNetwork.h>
#include <SystemConfiguration/SCPrivate.h>

#pragma mark -
#pragma mark CaptiveNetwork.framework APIs (exported through the SystemConfiguration.framework)

const CFStringRef kCNNetworkInfoKeySSIDData    = CFSTR("SSIDDATA");
const CFStringRef kCNNetworkInfoKeySSID        = CFSTR("SSID");
const CFStringRef kCNNetworkInfoKeyBSSID       = CFSTR("BSSID");

static void *
__loadCaptiveNetwork(void) {
	static void		*image	= NULL;
	static dispatch_once_t	once;

	dispatch_once(&once, ^{
		image = _SC_dlopen("/System/Library/PrivateFrameworks/CaptiveNetwork.framework/CaptiveNetwork");
	});

	return image;
}

Boolean
CNSetSupportedSSIDs(CFArrayRef ssidArray)
{
	static typeof (CNSetSupportedSSIDs) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadCaptiveNetwork();
		if (image) dyfunc = dlsym(image, "__CNSetSupportedSSIDs");
	}
	return dyfunc ? dyfunc(ssidArray) : FALSE;
}

Boolean
CNMarkPortalOnline(CFStringRef interfaceName)
{
	static typeof (CNMarkPortalOnline) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadCaptiveNetwork();
		if (image) dyfunc = dlsym(image, "__CNMarkPortalOnline");
	}
	return dyfunc ? dyfunc(interfaceName) : FALSE;
}

Boolean
CNMarkPortalOffline(CFStringRef interfaceName)
{
	static typeof (CNMarkPortalOffline) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadCaptiveNetwork();
		if (image) dyfunc = dlsym(image, "__CNMarkPortalOffline");
	}
	return dyfunc ? dyfunc(interfaceName) : FALSE;
}

CFArrayRef
CNCopySupportedInterfaces(void)
{
	static typeof (CNCopySupportedInterfaces) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadCaptiveNetwork();
		if (image) dyfunc = dlsym(image, "__CNCopySupportedInterfaces");
	}
	return dyfunc ? dyfunc() : NULL;
}

CFDictionaryRef
CNCopyCurrentNetworkInfo(CFStringRef	interfaceName)
{
#if	TARGET_OS_IPHONE && !TARGET_OS_IOSMAC
	static typeof (CNCopyCurrentNetworkInfo) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadCaptiveNetwork();
		if (image) dyfunc = dlsym(image, "__CNCopyCurrentNetworkInfo");
	}
	return dyfunc ? dyfunc(interfaceName) : NULL;
#else	// TARGET_OS_IPHONE && !TARGET_OS_IOSMAC
#pragma unused(interfaceName)
	return NULL;
#endif	// TARGET_OS_IPHONE && !TARGET_OS_IOSMAC
}


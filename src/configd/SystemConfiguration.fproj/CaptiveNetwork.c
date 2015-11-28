/*
 * Copyright (c) 2009, 2010, 2012, 2013 Apple Inc. All rights reserved.
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


#pragma mark -
#pragma mark CaptiveNetwork.framework APIs (exported through the SystemConfiguration.framework)


#if	TARGET_OS_IPHONE
const CFStringRef kCNNetworkInfoKeySSIDData    = CFSTR("SSIDDATA");
const CFStringRef kCNNetworkInfoKeySSID        = CFSTR("SSID");
const CFStringRef kCNNetworkInfoKeyBSSID       = CFSTR("BSSID");
#endif	// TARGET_OS_IPHONE


static void *
__loadCaptiveNetwork(void) {
	static void *image = NULL;
	if (NULL == image) {
		const char	*framework		= "/System/Library/PrivateFrameworks/CaptiveNetwork.framework/CaptiveNetwork";
		struct stat	statbuf;
		const char	*suffix			= getenv("DYLD_IMAGE_SUFFIX");
		char		path[MAXPATHLEN];

		strlcpy(path, framework, sizeof(path));
		if (suffix) strlcat(path, suffix, sizeof(path));
		if (0 <= stat(path, &statbuf)) {
			image = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
		} else {
			image = dlopen(framework, RTLD_LAZY | RTLD_LOCAL);
		}
	}
	return (void *)image;
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


#if	TARGET_OS_IPHONE
CFDictionaryRef
CNCopyCurrentNetworkInfo(CFStringRef	interfaceName)
{
	static typeof (CNCopyCurrentNetworkInfo) *dyfunc = NULL;
	if (!dyfunc) {
		void *image = __loadCaptiveNetwork();
		if (image) dyfunc = dlsym(image, "__CNCopyCurrentNetworkInfo");
	}
	return dyfunc ? dyfunc(interfaceName) : NULL;
}
#endif	// TARGET_OS_IPHONE

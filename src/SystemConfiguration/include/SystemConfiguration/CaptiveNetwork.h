/*
 * Copyright (c) 2009-2016 Apple Inc. All rights reserved.
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

#ifndef _CAPTIVENETWORK_H
#define _CAPTIVENETWORK_H

#include <Availability.h>
#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_ASSUME_NONNULL_BEGIN

/*!
	@header CaptiveNetwork
	@discussion The CaptiveNetwork API allows applications to interact
		with Captive Network Support. Captive Network Support is a
		system component responsible for detecting and help users
		navigate networks that require interaction before providing
		internet access. The most common Captive Networks are WiFi
		Hotspots in places like airports, restaurants, and hotels.
		Captive Network Support will attempt to authenticate if
		possible or drop a user in to a web sheet if authentication
		is not possible. In the web sheet the user has an opportunity
		to authenticate or disassociate from the network.

		The following APIs are designed for third party applications
		that may handle authentication on these networks on behalf of
		the user.

		These APIs are treated as advisory only.
		There is no guarantee or contract that the operating system
		will take the intended action.
 */

__BEGIN_DECLS

/*!
	@function CNSetSupportedSSIDs
	@discussion Provides Captive Network Support with an updated list of
		SSIDs that this application will perform authentication on.

		Captive Network Support suppresses showing the Web Sheet
		for a captive Wi-Fi network if that network's SSID is in the
		specified list.

		On iOS, the registrations persist until the application is
		removed from the device.

		On MacOSX, the registrations persist as long as the application
		is running.

	@param ssidArray A CFArray of CFStrings of the SSIDs.
	@result Returns TRUE if the operation succeeded, FALSE otherwise.
 */
Boolean
CNSetSupportedSSIDs	(CFArrayRef	ssidArray)
    __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_8, __MAC_NA,
				       __IPHONE_4_0, __IPHONE_9_0,
				       "Replaced by <NetworkExtension/NEHotspotHelper.h>");

/*!
	@function CNMarkPortalOnline
	@discussion Tells Captive Network Support that your application has
		authenticated the device to the network. Captive Network Support
		will notify the rest of the system that WiFi is now a viable
		interface.
	@param interfaceName Name of the interface that is now online.
	@result Returns TRUE if the operation succeeded, FALSE otherwise.
 */
Boolean
CNMarkPortalOnline	(CFStringRef	interfaceName)
    __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_8, __MAC_NA,
				       __IPHONE_4_0, __IPHONE_9_0,
				       "Replaced by <NetworkExtension/NEHotspotHelper.h>");

/*!
	@function CNMarkPortalOffline
	@discussion Tells Captive Network Support that the device is not
		authenticated on the given network interface.
	@param interfaceName Name of the interface that is still captive.
	@result Returns TRUE if the operation succeeded, FALSE otherwise.
 */
Boolean
CNMarkPortalOffline	(CFStringRef	interfaceName)
    __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_8, __MAC_NA,
				       __IPHONE_4_0, __IPHONE_9_0,
				       "Replaced by <NetworkExtension/NEHotspotHelper.h>");

/*!
 @function CNCopySupportedInterfaces
 @discussion copies a list of all interfaces CaptiveNetworkSupport is monitoring.
 @result An array of CFStringRef- BSD interface names.
	 Returns NULL if an error was encountered.
	 You MUST release the returned value.
 */
CFArrayRef __nullable
CNCopySupportedInterfaces	(void)			__OSX_AVAILABLE_STARTING(__MAC_10_8,__IPHONE_4_1);

/*!
 @constant kCNNetworkInfoKeySSIDData
 @discussion NetworkInfo Dictionary key for SSID in CFData format
 */
extern const CFStringRef kCNNetworkInfoKeySSIDData	__OSX_AVAILABLE_STARTING(__MAC_NA,__IPHONE_4_1);

/*!
 @constant kCNNetworkInfoKeySSID
 @discussion NetworkInfo Dictionary key for SSID in CFString format
 */
extern const CFStringRef kCNNetworkInfoKeySSID		__OSX_AVAILABLE_STARTING(__MAC_NA,__IPHONE_4_1);

/*!
 @constant kCNNetworkInfoKeyBSSID
 @discussion NetworkInfo Dictionary key for BSSID in CFString format
 */
extern const CFStringRef kCNNetworkInfoKeyBSSID		__OSX_AVAILABLE_STARTING(__MAC_NA,__IPHONE_4_1);

/*!
 @function CNCopyCurrentNetworkInfo
 @discussion Returns the Network Info for the specified interface.
	For example, Network Info dictionary will contain the following
	keys, and values:
	<pre>
	@textblock
	Keys                      : Values
	=======================================
	kCNNetworkInfoKeySSIDData : CFDataRef
	kCNNetworkInfoKeySSID     : CFStringRef
	kCNNetworkInfoKeyBSSID    : CFStringRef
	@/textblock
	</pre>
 @param interfaceName Name of the interface you are interested in
 @result Network Info dictionary associated with the interface.
	 Returns NULL if an error was encountered.
	 You MUST release the returned value.
 */
CFDictionaryRef __nullable
CNCopyCurrentNetworkInfo	(CFStringRef interfaceName)	__OSX_AVAILABLE_STARTING(__MAC_NA,__IPHONE_4_1);

__END_DECLS

CF_ASSUME_NONNULL_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif	/* _CAPTIVENETWORK_H */

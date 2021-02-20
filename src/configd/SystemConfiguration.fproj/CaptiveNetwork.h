/*
 * Copyright (c) 2009-2016, 2018, 2019 Apple Inc. All rights reserved.
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

#include <os/availability.h>
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
    API_AVAILABLE(macos(10.8))
    API_DEPRECATED_WITH_REPLACEMENT("Replaced by <NetworkExtension/NEHotspotHelper.h>", ios(4.0,9.0))
    SPI_AVAILABLE(tvos(9.0), watchos(1.0), bridgeos(1.0));

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
    API_AVAILABLE(macos(10.8))
    API_DEPRECATED_WITH_REPLACEMENT("Replaced by <NetworkExtension/NEHotspotHelper.h>", ios(4.0,9.0))
    SPI_AVAILABLE(tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function CNMarkPortalOffline
	@discussion Tells Captive Network Support that the device is not
		authenticated on the given network interface.
	@param interfaceName Name of the interface that is still captive.
	@result Returns TRUE if the operation succeeded, FALSE otherwise.
 */
Boolean
CNMarkPortalOffline	(CFStringRef	interfaceName)
    API_AVAILABLE(macos(10.8))
    API_DEPRECATED_WITH_REPLACEMENT("Replaced by <NetworkExtension/NEHotspotHelper.h>", ios(4.0,9.0))
    SPI_AVAILABLE(tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
 @function CNCopySupportedInterfaces
 @discussion copies a list of all interfaces CaptiveNetworkSupport is monitoring.
 @result An array of CFStringRef- BSD interface names.
	 Returns NULL if an error was encountered.
	 You MUST release the returned value.
 */
CFArrayRef __nullable
CNCopySupportedInterfaces	(void)
    API_AVAILABLE(macos(10.8), ios(4.1))
    SPI_AVAILABLE(tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
 @constant kCNNetworkInfoKeySSIDData
 @discussion NetworkInfo Dictionary key for SSID in CFData format
 */
extern const CFStringRef kCNNetworkInfoKeySSIDData
    API_AVAILABLE(ios(4.1))
    SPI_AVAILABLE(macos(10.6), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
 @constant kCNNetworkInfoKeySSID
 @discussion NetworkInfo Dictionary key for SSID in CFString format
 */
extern const CFStringRef kCNNetworkInfoKeySSID
    API_AVAILABLE(ios(4.1))
    SPI_AVAILABLE(macos(10.6), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
 @constant kCNNetworkInfoKeyBSSID
 @discussion NetworkInfo Dictionary key for BSSID in CFString format
 */
extern const CFStringRef kCNNetworkInfoKeyBSSID
    API_AVAILABLE(ios(4.1))
    SPI_AVAILABLE(macos(10.6), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
 @function CNCopyCurrentNetworkInfo
 @discussion Returns the network information for the specified interface when the requesting application meets one of following 3 requirements -.
	1. application is using CoreLocation API and has user's authorization to access location.
	2. application has used NEHotspotConfiguration API to configure the current Wi-Fi network.
	3. application has active VPN configurations installed.

	- An application that is linked against iOS 12.0 SDK and above must have the "com.apple.developer.networking.wifi-info" entitlement.
	- An application will receive a pseudo network information if it is linked against an SDK before iOS 13.0, and if it fails to meet any of the
	  above 3 requirements.
	- An application will receive NULL if it is linked against iOS 13.0 SDK or above, and if it fails to meet any of the above 3 requirements..

	Network Information dictionary will contain the following keys, and values:
	<pre>
	@textblock
	Keys                      : Values
	=======================================
	kCNNetworkInfoKeySSIDData : CFDataRef
	kCNNetworkInfoKeySSID     : CFStringRef
	kCNNetworkInfoKeyBSSID    : CFStringRef
	@/textblock
	</pre>

	Pseudo network information will contain "Wi-Fi" SSID and "00:00:00:00:00:00" BSSID. For China region, the SSID will be "WLAN".
 @param interfaceName Name of the interface you are interested in
 @result Network Information dictionary associated with the interface.
	 Returns NULL if an error was encountered.
	 You MUST release the returned value.
 */
CFDictionaryRef __nullable
CNCopyCurrentNetworkInfo	(CFStringRef	interfaceName)
    API_AVAILABLE(ios(4.1))
    SPI_AVAILABLE(macos(10.6), tvos(9.0), watchos(1.0), bridgeos(1.0));

__END_DECLS

CF_ASSUME_NONNULL_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif	/* _CAPTIVENETWORK_H */

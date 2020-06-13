/*
 * Copyright (c) 2004-2011, 2015, 2016, 2018 Apple Inc. All rights reserved.
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

#ifndef _SCNETWORKCONFIGURATION_H
#define _SCNETWORKCONFIGURATION_H

#include <os/availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_ASSUME_NONNULL_BEGIN

/*!
	@header SCNetworkConfiguration
	@discussion The SCNetworkConfiguration API provides access to the
		stored network configuration.  The functions include
		providing access to the network capable devices on the
		system, the network sets, network services, and network
		protocols.

		Note: When using the SCNetworkConfiguraiton APIs you must
		keep in mind that in order for any of your changes to be
		committed to permanent storage a call must be made to the
		SCPreferencesCommitChanges function.
 */


/*!
	@group Interface configuration
 */

#pragma mark -
#pragma mark SCNetworkInterface configuration (typedefs, consts)

/*!
	@typedef SCNetworkInterfaceRef
	@discussion This is the type of a reference to an object that represents
		a network interface.
 */
typedef const struct CF_BRIDGED_TYPE(id) __SCNetworkInterface * SCNetworkInterfaceRef;

/*!
	@const kSCNetworkInterfaceType6to4
 */
extern const CFStringRef kSCNetworkInterfaceType6to4						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkInterfaceTypeBluetooth
 */
extern const CFStringRef kSCNetworkInterfaceTypeBluetooth					API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkInterfaceTypeBond
 */
extern const CFStringRef kSCNetworkInterfaceTypeBond						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkInterfaceTypeEthernet
 */
extern const CFStringRef kSCNetworkInterfaceTypeEthernet					API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkInterfaceTypeFireWire
 */
extern const CFStringRef kSCNetworkInterfaceTypeFireWire					API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkInterfaceTypeIEEE80211
 */
extern const CFStringRef kSCNetworkInterfaceTypeIEEE80211					API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));	// IEEE 802.11, AirPort

/*!
	@const kSCNetworkInterfaceTypeIPSec
 */
extern const CFStringRef kSCNetworkInterfaceTypeIPSec						API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkInterfaceTypeIrDA
 */
extern const CFStringRef kSCNetworkInterfaceTypeIrDA						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkInterfaceTypeL2TP
 */
extern const CFStringRef kSCNetworkInterfaceTypeL2TP						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkInterfaceTypeModem
 */
extern const CFStringRef kSCNetworkInterfaceTypeModem						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkInterfaceTypePPP
 */
extern const CFStringRef kSCNetworkInterfaceTypePPP						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkInterfaceTypePPTP
 */
extern const CFStringRef kSCNetworkInterfaceTypePPTP						API_DEPRECATED("No longer supported", macos(10.4,10.12))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));


/*!
	@const kSCNetworkInterfaceTypeSerial
 */
extern const CFStringRef kSCNetworkInterfaceTypeSerial						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkInterfaceTypeVLAN
 */
extern const CFStringRef kSCNetworkInterfaceTypeVLAN						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkInterfaceTypeWWAN
 */
extern const CFStringRef kSCNetworkInterfaceTypeWWAN						API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/* special network interfaces (and types) */

/*!
	@const kSCNetworkInterfaceTypeIPv4
 */
extern const CFStringRef kSCNetworkInterfaceTypeIPv4						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkInterfaceIPv4
	@discussion A network interface that can used for layering other
		interfaces (e.g. 6to4, IPSec, PPTP, L2TP) over an existing
		IPv4 network.
 */
extern const SCNetworkInterfaceRef kSCNetworkInterfaceIPv4					API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@group Interface configuration (Bond)
 */

#pragma mark -

/*!
	@typedef SCBondInterfaceRef
	@discussion This is the type of a reference to an object that represents
		an Ethernet Bond interface.
 */
typedef SCNetworkInterfaceRef SCBondInterfaceRef;

/*!
	@typedef SCBondStatusRef
	@discussion This is the type of a reference to an object that represents
		the status of an Ethernet Bond interface.
 */
typedef const struct CF_BRIDGED_TYPE(id) __SCBondStatus *		SCBondStatusRef;

/*!
	@enum Ethernet Bond Aggregation Status (kSCBondStatusDeviceAggregationStatus) codes
	@discussion Returned status codes.
	@constant kSCBondStatusOK		Enabled, active, running, ...
	@constant kSCBondStatusLinkInvalid	The link state was not valid (i.e. down, half-duplex, wrong speed)
	@constant kSCBondStatusNoPartner	The port on the switch that the device is connected doesn't seem to have 802.3ad Link Aggregation enabled
	@constant kSCBondStatusNotInActiveGroup	We're talking to a partner, but the link aggregation group is different from the one that's active
	@constant kSCBondStatusUnknown		Non-specific failure
 */
enum {
	kSCBondStatusOK			= 0,	/* enabled, active, running, ... */
	kSCBondStatusLinkInvalid	= 1,	/* The link state was not valid (i.e. down, half-duplex, wrong speed) */
	kSCBondStatusNoPartner		= 2,	/* The port on the switch that the device is connected doesn't seem to have 802.3ad Link Aggregation enabled */
	kSCBondStatusNotInActiveGroup	= 3,	/* We're talking to a partner, but the link aggregation group is different from the one that's active */
	kSCBondStatusUnknown		= 999	/* Non-specific failure */
};

/*!
	@const kSCBondStatusDeviceAggregationStatus
 */
extern const CFStringRef kSCBondStatusDeviceAggregationStatus	/* CFNumber */			API_AVAILABLE(macos(10.4)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@const kSCBondStatusDeviceCollecting
 */
extern const CFStringRef kSCBondStatusDeviceCollecting		/* CFNumber (0 or 1) */		API_AVAILABLE(macos(10.4)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@const kSCBondStatusDeviceDistributing
 */
extern const CFStringRef kSCBondStatusDeviceDistributing	/* CFNumber (0 or 1) */		API_AVAILABLE(macos(10.4)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@group Interface configuration (VLAN)
 */

#pragma mark -

/*!
	@typedef SCVLANInterfaceRef
	@discussion This is the type of a reference to an object that represents
		a Virtual LAN (VLAN) interface.
 */
typedef SCNetworkInterfaceRef SCVLANInterfaceRef;


/*!
	@group Protocol configuration
 */

#pragma mark -
#pragma mark SCNetworkProtocol configuration (typedefs, consts)

/*!
	@typedef SCNetworkProtocolRef
	@discussion This is the type of a reference to an object that represents
		a network protocol.
 */
typedef const struct CF_BRIDGED_TYPE(id) __SCNetworkProtocol * SCNetworkProtocolRef;

/* network "protocol" types */

/*!
	@const kSCNetworkProtocolTypeDNS
 */
extern const CFStringRef kSCNetworkProtocolTypeDNS						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkProtocolTypeIPv4
 */
extern const CFStringRef kSCNetworkProtocolTypeIPv4						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkProtocolTypeIPv6
 */
extern const CFStringRef kSCNetworkProtocolTypeIPv6						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkProtocolTypeProxies
 */
extern const CFStringRef kSCNetworkProtocolTypeProxies						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkProtocolTypeSMB
 */
extern const CFStringRef kSCNetworkProtocolTypeSMB						API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@group Service configuration
 */

#pragma mark -
#pragma mark SCNetworkService configuration (typedefs, consts)

/*!
	@typedef SCNetworkServiceRef
	@discussion This is the type of a reference to an object that represents
		a network service.
 */
typedef const struct CF_BRIDGED_TYPE(id) __SCNetworkService * SCNetworkServiceRef;


/*!
	@group Set configuration
 */

#pragma mark -
#pragma mark SCNetworkSet configuration (typedefs, consts)

/*!
	@typedef SCNetworkSetRef
	@discussion This is the type of a reference to an object that represents
		a network set.
 */
typedef const struct CF_BRIDGED_TYPE(id) __SCNetworkSet	* SCNetworkSetRef;


__BEGIN_DECLS


/* --------------------------------------------------------------------------------
 * INTERFACES
 * -------------------------------------------------------------------------------- */

/*!
	@group Interface configuration
 */

#pragma mark -
#pragma mark SCNetworkInterface configuration (APIs)

/*!
	@function SCNetworkInterfaceGetTypeID
	@discussion Returns the type identifier of all SCNetworkInterface instances.
 */
CFTypeID
SCNetworkInterfaceGetTypeID			(void)						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceCopyAll
	@discussion Returns all network capable interfaces on the system.
	@result The list of interfaces on the system.
		You must release the returned value.
 */
CFArrayRef /* of SCNetworkInterfaceRef's */
SCNetworkInterfaceCopyAll			(void)						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceGetSupportedInterfaceTypes
	@discussion Identify all of the network interface types (e.g. PPP) that
		can be layered on top of this interface.
	@param interface The network interface.
	@result The list of SCNetworkInterface types supported by the interface;
		NULL if no interface types are supported.
 */
CFArrayRef /* of kSCNetworkInterfaceTypeXXX CFStringRef's */ __nullable
SCNetworkInterfaceGetSupportedInterfaceTypes	(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceGetSupportedProtocolTypes
	@discussion Identify all of the network protocol types (e.g. IPv4, IPv6) that
		can be layered on top of this interface.
	@param interface The network interface.
	@result The list of SCNetworkProtocol types supported by the interface;
		NULL if no protocol types are supported.
 */
CFArrayRef /* of kSCNetworkProtocolTypeXXX CFStringRef's */ __nullable
SCNetworkInterfaceGetSupportedProtocolTypes	(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceCreateWithInterface
	@discussion Create a new network interface layered on top of another.  For
		example, this function would be used to create a "PPP" interface
		on top of a "modem".
	@param interface The network interface.
	@param interfaceType The type of SCNetworkInterface to be layered on
		top of the provided interface.
	@result A reference to the new SCNetworkInterface.
		You must release the returned value.
 */
SCNetworkInterfaceRef __nullable
SCNetworkInterfaceCreateWithInterface		(SCNetworkInterfaceRef		interface,
						 CFStringRef			interfaceType)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceGetBSDName
	@discussion Returns the BSD interface (en0) or device name (modem)
		for the interface.
	@param interface The network interface.
	@result The BSD name associated with the interface (e.g. "en0");
		NULL if no BSD name is available.
 */
CFStringRef __nullable
SCNetworkInterfaceGetBSDName			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceGetConfiguration
	@discussion Returns the configuration settings associated with a interface.
	@param interface The network interface.
	@result The configuration settings associated with the interface;
		NULL if no configuration settings are associated with the interface
		or an error was encountered.
 */
CFDictionaryRef __nullable
SCNetworkInterfaceGetConfiguration		(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceGetExtendedConfiguration
	@discussion Returns the configuration settings associated with a interface.
	@param interface The network interface.
	@param extendedType A string representing the type of extended information (e.g. EAPOL).
	@result The configuration settings associated with the interface;
		NULL if no configuration settings are associated with the interface
		or an error was encountered.
 */
CFDictionaryRef __nullable
SCNetworkInterfaceGetExtendedConfiguration	(SCNetworkInterfaceRef		interface,
						 CFStringRef			extendedType)	API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceGetHardwareAddressString
	@discussion Returns a displayable link layer address for the interface.
	@param interface The network interface.
	@result A string representing the hardware (MAC) address for the interface.
 */
CFStringRef __nullable
SCNetworkInterfaceGetHardwareAddressString	(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceGetInterface
	@discussion For layered network interfaces, return the underlying interface.
	@param interface The network interface.
	@result The underlying network interface;
		NULL if this is a leaf interface.
 */
SCNetworkInterfaceRef __nullable
SCNetworkInterfaceGetInterface			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceGetInterfaceType
	@discussion Returns the associated network interface type.
	@param interface The network interface.
	@result The interface type.
 */
CFStringRef __nullable
SCNetworkInterfaceGetInterfaceType		(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceGetLocalizedDisplayName
	@discussion Returns the localized name (e.g. "Ethernet", "FireWire") for
		the interface.
	@param interface The network interface.
	@result A localized, display name for the interface;
		NULL if no name is available.
 */
CFStringRef __nullable
SCNetworkInterfaceGetLocalizedDisplayName	(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceSetConfiguration
	@discussion Stores the configuration settings for the interface.
	@param interface The network interface.
	@param config The configuration settings to associate with this interface.
	@result TRUE if the configuration was stored; FALSE if an error was encountered.
 */
Boolean
SCNetworkInterfaceSetConfiguration		(SCNetworkInterfaceRef		interface,
						 CFDictionaryRef __nullable	config)		API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceSetExtendedConfiguration
	@discussion Stores the configuration settings for the interface.
	@param interface The network interface.
	@param config The configuration settings to associate with this interface.
	@result TRUE if the configuration was stored; FALSE if an error was encountered.
 */
Boolean
SCNetworkInterfaceSetExtendedConfiguration	(SCNetworkInterfaceRef		interface,
						 CFStringRef			extendedType,
						 CFDictionaryRef __nullable	config)		API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

#pragma mark -

/*!
	@function SCNetworkInterfaceCopyMediaOptions
	@discussion For the specified network interface, returns information
		about the currently requested media options, the active media
		options, and the media options which are available.
	@param interface The desired network interface.
	@param current A pointer to memory that will be filled with a CFDictionaryRef
		representing the currently requested media options (subtype, options).
		If NULL, the current options will not be returned.
	@param active A pointer to memory that will be filled with a CFDictionaryRef
		representing the active media options (subtype, options).
		If NULL, the active options will not be returned.
	@param available A pointer to memory that will be filled with a CFArrayRef
		representing the possible media options (subtype, options).
		If NULL, the available options will not be returned.
	@param filter A boolean indicating whether the available options should be
		filtered to exclude those options which would not normally be
		requested by a user/admin (e.g. hw-loopback).
	@result TRUE if requested information has been returned.
 */
Boolean
SCNetworkInterfaceCopyMediaOptions		(SCNetworkInterfaceRef						interface,
						 CFDictionaryRef		__nullable	* __nullable	current,
						 CFDictionaryRef		__nullable	* __nullable	active,
						 CFArrayRef			__nullable	* __nullable	available,
						 Boolean					filter)		API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceCopyMediaSubTypes
	@discussion For the provided interface configuration options, return a list
		of available media subtypes.
	@param available The available options as returned by the
		SCNetworkInterfaceCopyMediaOptions function.
	@result An array of available media subtypes CFString's (e.g. 10BaseT/UTP,
		100baseTX, etc).  NULL if no subtypes are available.
 */
CFArrayRef __nullable
SCNetworkInterfaceCopyMediaSubTypes		(CFArrayRef			available)	API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceCopyMediaSubTypeOptions
	@discussion For the provided interface configuration options and specific
		subtype, return a list of available media options.
	@param available The available options as returned by the
		SCNetworkInterfaceCopyMediaOptions function.
	@param subType The subtype
	@result An array of available media options.  Each of the available options
		is returned as an array of CFString's (e.g. <half-duplex>,
		<full-duplex,flow-control>).  NULL if no options are available.
 */
CFArrayRef __nullable
SCNetworkInterfaceCopyMediaSubTypeOptions	(CFArrayRef			available,
						 CFStringRef			subType)	API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceCopyMTU
	@discussion For the specified network interface, returns information
		about the currently MTU setting and the range of allowable
		values.
	@param interface The desired network interface.
	@param mtu_cur A pointer to memory that will be filled with the current
		MTU setting for the interface.
	@param mtu_min A pointer to memory that will be filled with the minimum
		MTU setting for the interface.  If negative, the minimum setting
		could not be determined.
	@param mtu_max A pointer to memory that will be filled with the maximum
		MTU setting for the interface.  If negative, the maximum setting
		could not be determined.
	@result TRUE if requested information has been returned.
 */
Boolean
SCNetworkInterfaceCopyMTU			(SCNetworkInterfaceRef			interface,
						 int			* __nullable	mtu_cur,
						 int			* __nullable	mtu_min,
						 int			* __nullable	mtu_max)	API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceSetMediaOptions
	@discussion For the specified network interface, sets the requested
		media subtype and options.
	@param interface The desired network interface.
	@param subtype The desired media subtype (e.g. "autoselect", "100baseTX", ...).
		If NULL, no specific media subtype will be requested.
	@param options The desired media options (e.g. "half-duplex", "full-duplex", ...).
		If NULL, no specific media options will be requested.
	@result TRUE if the configuration was updated; FALSE if an error was encountered.
 */
Boolean
SCNetworkInterfaceSetMediaOptions		(SCNetworkInterfaceRef			interface,
						 CFStringRef		__nullable	subtype,
						 CFArrayRef		__nullable	options)	API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceSetMTU
	@discussion For the specified network interface, sets the
		requested MTU setting.
	@param interface The desired network interface.
	@param mtu The desired MTU setting for the interface.
		If zero, the interface will use the default MTU setting.
	@result TRUE if the configuration was updated; FALSE if an error was encountered.
 */
Boolean
SCNetworkInterfaceSetMTU			(SCNetworkInterfaceRef		interface,
						 int				mtu)		API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceForceConfigurationRefresh
	@discussion Sends a notification to interested network configuration
		agents to immediately retry their configuration. For example,
		calling this function will cause the DHCP client to contact
		the DHCP server immediately rather than waiting until its
		timeout has expired.  The utility of this function is to
		allow the caller to give a hint to the system that the
		network infrastructure or configuration has changed.

		Note: This function requires root (euid==0) privilege or,
		alternatively, you may pass an SCNetworkInterface which
		is derived from a sequence of calls to :

			SCPreferencesCreateWithAuthorization
			SCNetworkSetCopy...
			SCNetworkServiceGetInterface
	@param interface The desired network interface.
	@result Returns TRUE if the notification was sent; FALSE otherwise.
 */
Boolean
SCNetworkInterfaceForceConfigurationRefresh	(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@group Interface configuration (Bond)
 */

#pragma mark -
#pragma mark SCBondInterface configuration (APIs)

/*!
	@function SCBondInterfaceCopyAll
	@discussion Returns all Ethernet Bond interfaces on the system.
	@param prefs The "preferences" session.
	@result The list of Ethernet Bond interfaces on the system.
		You must release the returned value.
 */
CFArrayRef /* of SCBondInterfaceRef's */
SCBondInterfaceCopyAll				(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@function SCBondInterfaceCopyAvailableMemberInterfaces
	@discussion Returns all network capable devices on the system
		that can be added to an Ethernet Bond interface.
	@param prefs The "preferences" session.
	@result The list of interfaces.
		You must release the returned value.
 */
CFArrayRef /* of SCNetworkInterfaceRef's */
SCBondInterfaceCopyAvailableMemberInterfaces	(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@function SCBondInterfaceCreate
	@discussion Create a new SCBondInterface interface.
	@param prefs The "preferences" session.
	@result A reference to the new SCBondInterface.
		You must release the returned value.
 */
SCBondInterfaceRef __nullable
SCBondInterfaceCreate				(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@function SCBondInterfaceRemove
	@discussion Removes the SCBondInterface from the configuration.
	@param bond The SCBondInterface interface.
	@result TRUE if the interface was removed; FALSE if an error was encountered.
 */
Boolean
SCBondInterfaceRemove				(SCBondInterfaceRef		bond)		API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@function SCBondInterfaceGetMemberInterfaces
	@discussion Returns the member interfaces for the specified Ethernet Bond interface.
	@param bond The SCBondInterface interface.
	@result The list of interfaces.
 */
CFArrayRef /* of SCNetworkInterfaceRef's */ __nullable
SCBondInterfaceGetMemberInterfaces		(SCBondInterfaceRef		bond)		API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@function SCBondInterfaceGetOptions
	@discussion Returns the configuration settings associated with a Ethernet Bond interface.
	@param bond The SCBondInterface interface.
	@result The configuration settings associated with the Ethernet Bond interface;
		NULL if no changes to the default configuration have been saved.
 */
CFDictionaryRef __nullable
SCBondInterfaceGetOptions			(SCBondInterfaceRef		bond)		API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@function SCBondInterfaceSetMemberInterfaces
	@discussion Sets the member interfaces for the specified Ethernet Bond interface.
	@param bond The SCBondInterface interface.
	@param members The desired member interfaces.
	@result TRUE if the configuration was stored; FALSE if an error was encountered.
 */
Boolean
SCBondInterfaceSetMemberInterfaces		(SCBondInterfaceRef		bond,
						 CFArrayRef			members) /* of SCNetworkInterfaceRef's */
												API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@function SCBondInterfaceSetLocalizedDisplayName
	@discussion Sets the localized display name for the specified Ethernet Bond interface.
	@param bond The SCBondInterface interface.
	@param newName The new display name.
	@result TRUE if the configuration was stored; FALSE if an error was encountered.
 */
Boolean
SCBondInterfaceSetLocalizedDisplayName		(SCBondInterfaceRef		bond,
						 CFStringRef			newName)	API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@function SCBondInterfaceSetOptions
	@discussion Sets the configuration settings for the specified Ethernet Bond interface.
	@param bond The SCBondInterface interface.
	@param newOptions The new configuration settings.
	@result TRUE if the configuration was stored; FALSE if an error was encountered.
 */
Boolean
SCBondInterfaceSetOptions			(SCBondInterfaceRef		bond,
						 CFDictionaryRef		newOptions)	API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

#pragma mark -

/*!
	@function SCBondInterfaceCopyStatus
	@discussion Returns the status of the specified Ethernet Bond interface.
	@param bond The SCBondInterface interface.
	@result The status associated with the interface.
		You must release the returned value.
 */
SCBondStatusRef __nullable
SCBondInterfaceCopyStatus			(SCBondInterfaceRef	bond)			API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@function SCBondStatusGetTypeID
	@discussion Returns the type identifier of all SCBondStatus instances.
 */
CFTypeID
SCBondStatusGetTypeID				(void)						API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@function SCBondStatusGetMemberInterfaces
	@discussion Returns the member interfaces that are represented with the
		Ethernet Bond interface.
	@param bondStatus The Ethernet Bond status.
	@result The list of interfaces.
 */
CFArrayRef __nullable /* of SCNetworkInterfaceRef's */
SCBondStatusGetMemberInterfaces			(SCBondStatusRef	bondStatus)		API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@function SCBondStatusGetInterfaceStatus
	@discussion Returns the status of a specific member interface of an
		Ethernet Bond or the status of the bond as a whole.
	@param bondStatus The Ethernet Bond status.
	@param interface The specific member interface; NULL if you want the
		status of the Ethernet Bond.
	@result The interface status.

	Note: at present, no information about the status of the Ethernet
	      Bond is returned.  As long as one member interface is active
	      then the bond should be operational.
 */
CFDictionaryRef __nullable
SCBondStatusGetInterfaceStatus			(SCBondStatusRef			bondStatus,
						 SCNetworkInterfaceRef	__nullable	interface)	API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@group Interface configuration (VLAN)
 */

#pragma mark -
#pragma mark SCVLANInterface configuration (APIs)

/*!
	@function SCVLANInterfaceCopyAll
	@discussion Returns all VLAN interfaces on the system.
	@result The list of VLAN interfaces on the system.
		You must release the returned value.
 */
CFArrayRef /* of SCVLANInterfaceRef's */
SCVLANInterfaceCopyAll				(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCVLANInterfaceCopyAvailablePhysicalInterfaces
	@discussion Returns the network capable devices on the system
		that can be associated with a VLAN interface.
	@result The list of interfaces.
		You must release the returned value.
 */
CFArrayRef /* of SCNetworkInterfaceRef's */
SCVLANInterfaceCopyAvailablePhysicalInterfaces	(void)						API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCVLANInterfaceCreate
	@discussion Create a new SCVLANInterface interface.
	@param prefs The "preferences" session.
	@param physical The physical interface to associate with the VLAN.
	@param tag The tag to associate with the VLAN.
	@result A reference to the new SCVLANInterface.
		You must release the returned value.

	Note: the tag must be in the range (1 <= tag <= 4094)
 */
SCVLANInterfaceRef __nullable
SCVLANInterfaceCreate				(SCPreferencesRef		prefs,
						 SCNetworkInterfaceRef		physical,
						 CFNumberRef			tag)		API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCVLANInterfaceRemove
	@discussion Removes the SCVLANInterface from the configuration.
	@param vlan The SCVLANInterface interface.
	@result TRUE if the interface was removed; FALSE if an error was encountered.
 */
Boolean
SCVLANInterfaceRemove				(SCVLANInterfaceRef		vlan)		API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCVLANInterfaceGetPhysicalInterface
	@discussion Returns the physical interface for the specified VLAN interface.
	@param vlan The SCVLANInterface interface.
	@result The list of interfaces.
 */
SCNetworkInterfaceRef __nullable
SCVLANInterfaceGetPhysicalInterface		(SCVLANInterfaceRef		vlan)		API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCVLANInterfaceGetTag
	@discussion Returns the tag for the specified VLAN interface.
	@param vlan The SCVLANInterface interface.
	@result The tag.
 */
CFNumberRef __nullable
SCVLANInterfaceGetTag				(SCVLANInterfaceRef		vlan)		API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCVLANInterfaceGetOptions
	@discussion Returns the configuration settings associated with the VLAN interface.
	@param vlan The SCVLANInterface interface.
	@result The configuration settings associated with the VLAN interface;
		NULL if no changes to the default configuration have been saved.
 */
CFDictionaryRef __nullable
SCVLANInterfaceGetOptions			(SCVLANInterfaceRef		vlan)		API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCVLANInterfaceSetPhysicalInterfaceAndTag
	@discussion Updates the specified VLAN interface.
	@param vlan The SCVLANInterface interface.
	@param physical The physical interface to associate with the VLAN.
	@param tag The tag to associate with the VLAN.
	@result TRUE if the configuration was stored; FALSE if an error was encountered.

	Note: the tag must be in the range (1 <= tag <= 4094)
 */
Boolean
SCVLANInterfaceSetPhysicalInterfaceAndTag	(SCVLANInterfaceRef		vlan,
						 SCNetworkInterfaceRef		physical,
						 CFNumberRef			tag)		API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCVLANInterfaceSetLocalizedDisplayName
	@discussion Sets the localized display name for the specified VLAN interface.
	@param vlan The SCVLANInterface interface.
	@param newName The new display name.
	@result TRUE if the configuration was stored; FALSE if an error was encountered.
 */
Boolean
SCVLANInterfaceSetLocalizedDisplayName		(SCVLANInterfaceRef		vlan,
						 CFStringRef			newName)	API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCVLANInterfaceSetOptions
	@discussion Sets the configuration settings for the specified VLAN interface.
	@param vlan The SCVLANInterface interface.
	@param newOptions The new configuration settings.
	@result TRUE if the configuration was stored; FALSE if an error was encountered.
 */
Boolean
SCVLANInterfaceSetOptions			(SCVLANInterfaceRef		vlan,
						 CFDictionaryRef		newOptions)	API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));


/* --------------------------------------------------------------------------------
 * PROTOCOLS
 * -------------------------------------------------------------------------------- */

/*!
	@group Protocol configuration
 */

#pragma mark -
#pragma mark SCNetworkProtocol configuration (APIs)

/*!
	@function SCNetworkProtocolGetTypeID
	@discussion Returns the type identifier of all SCNetworkProtocol instances.
 */
CFTypeID
SCNetworkProtocolGetTypeID			(void)						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkProtocolGetConfiguration
	@discussion Returns the configuration settings associated with the protocol.
	@param protocol The network protocol.
	@result The configuration settings associated with the protocol;
		NULL if no configuration settings are associated with the protocol
		or an error was encountered.
 */
CFDictionaryRef __nullable
SCNetworkProtocolGetConfiguration		(SCNetworkProtocolRef		protocol)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkProtocolGetEnabled
	@discussion Returns whether this protocol has been enabled.
	@param protocol The network protocol.
	@result TRUE if the protocol is enabled.
 */
Boolean
SCNetworkProtocolGetEnabled			(SCNetworkProtocolRef		protocol)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkProtocolGetProtocolType
	@discussion Returns the associated network protocol type.
	@param protocol The network protocol.
	@result The protocol type.
 */
CFStringRef __nullable
SCNetworkProtocolGetProtocolType		(SCNetworkProtocolRef		protocol)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkProtocolSetConfiguration
	@discussion Stores the configuration settings for the protocol.
	@param protocol The network protocol.
	@param config The configuration settings to associate with this protocol.
	@result TRUE if the configuration was stored; FALSE if an error was encountered.
 */
Boolean
SCNetworkProtocolSetConfiguration		(SCNetworkProtocolRef		protocol,
						 CFDictionaryRef __nullable	config)		API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkProtocolSetEnabled
	@discussion Enables or disables the protocol.
	@param protocol The network protocol.
	@param enabled TRUE if the protocol should be enabled.
	@result TRUE if the enabled status was saved; FALSE if an error was encountered.
 */
Boolean
SCNetworkProtocolSetEnabled			(SCNetworkProtocolRef		protocol,
						 Boolean			enabled)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/* --------------------------------------------------------------------------------
 * SERVICES
 * -------------------------------------------------------------------------------- */

/*!
	@group Service configuration
 */

#pragma mark -
#pragma mark SCNetworkService configuration (APIs)

/*!
	@function SCNetworkServiceGetTypeID
	@discussion Returns the type identifier of all SCNetworkService instances.
 */
CFTypeID
SCNetworkServiceGetTypeID			(void)						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkServiceAddProtocolType
	@discussion Adds a network protocol of the specified type to the
		service.  The protocal configuration is set to default values
		that are appropriate for the interface associated with the
		service.
	@param service The network service.
	@param protocolType The type of SCNetworkProtocol to be added to the service.
	@result TRUE if the protocol was added to the service; FALSE if the
		protocol was already present or an error was encountered.
 */
Boolean
SCNetworkServiceAddProtocolType			(SCNetworkServiceRef		service,
						 CFStringRef			protocolType)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkServiceCopyAll
	@discussion Returns all available network services for the specified preferences.
	@param prefs The "preferences" session.
	@result The list of SCNetworkService services associated with the preferences.
		You must release the returned value.
 */
CFArrayRef /* of SCNetworkServiceRef's */ __nullable
SCNetworkServiceCopyAll				(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkServiceCopyProtocols
	@discussion Returns all network protocols associated with the service.
	@param service The network service.
	@result The list of SCNetworkProtocol protocols associated with the service.
		You must release the returned value.
 */
CFArrayRef /* of SCNetworkProtocolRef's */ __nullable
SCNetworkServiceCopyProtocols			(SCNetworkServiceRef		service)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkServiceCreate
	@discussion Create a new network service for the specified interface in the
		configuration.
	@param prefs The "preferences" session.
	@result A reference to the new SCNetworkService.
		You must release the returned value.
 */
SCNetworkServiceRef __nullable
SCNetworkServiceCreate				(SCPreferencesRef		prefs,
						 SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkServiceCopy
	@discussion Returns the network service with the specified identifier.
	@param prefs The "preferences" session.
	@param serviceID The unique identifier for the service.
	@result A reference to the SCNetworkService from the associated preferences;
		NULL if the serviceID does not exist in the preferences or if an
		error was encountered.
		You must release the returned value.
 */
SCNetworkServiceRef __nullable
SCNetworkServiceCopy				(SCPreferencesRef		prefs,
						 CFStringRef			serviceID)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkServiceEstablishDefaultConfiguration
	@discussion Establishes the "default" configuration for a network
		service.  This configuration includes the addition of
		network protocols for the service (with "default"
		configuration options).
	@param service The network service.
	@result TRUE if the configuration was updated; FALSE if an error was encountered.
*/
Boolean
SCNetworkServiceEstablishDefaultConfiguration	(SCNetworkServiceRef		service)	API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkServiceGetEnabled
	@discussion Returns whether this service has been enabled.
	@param service The network service.
	@result TRUE if the service is enabled.
 */
Boolean
SCNetworkServiceGetEnabled			(SCNetworkServiceRef		service)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkServiceGetInterface
	@discussion Returns the network interface associated with the service.
	@param service The network service.
	@result A reference to the SCNetworkInterface associated with the service;
		NULL if an error was encountered.
 */
SCNetworkInterfaceRef __nullable
SCNetworkServiceGetInterface			(SCNetworkServiceRef		service)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkServiceGetName
	@discussion Returns the [user specified] name associated with the service.
	@param service The network service.
	@result The [user specified] name.
 */
CFStringRef __nullable
SCNetworkServiceGetName				(SCNetworkServiceRef		service)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkServiceCopyProtocol
	@discussion Returns the network protocol of the specified type for
		the service.
	@param service The network service.
	@result A reference to the SCNetworkProtocol associated with the service;
		NULL if this protocol has not been added or if an error was encountered.
		You must release the returned value.
 */
SCNetworkProtocolRef __nullable
SCNetworkServiceCopyProtocol			(SCNetworkServiceRef		service,
						 CFStringRef			protocolType)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkServiceGetServiceID
	@discussion Returns the identifier for the service.
	@param service The network service.
	@result The service identifier.
 */
CFStringRef __nullable
SCNetworkServiceGetServiceID			(SCNetworkServiceRef		service)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkServiceRemove
	@discussion Removes the network service from the configuration.
	@param service The network service.
	@result TRUE if the service was removed; FALSE if an error was encountered.
 */
Boolean
SCNetworkServiceRemove				(SCNetworkServiceRef		service)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkServiceRemoveProtocolType
	@discussion Removes the network protocol of the specified type from the service.
	@param service The network service.
	@param protocolType The type of SCNetworkProtocol to be removed from the service.
	@result TRUE if the protocol was removed to the service; FALSE if the
		protocol was not configured or an error was encountered.
 */
Boolean
SCNetworkServiceRemoveProtocolType		(SCNetworkServiceRef		service,
						 CFStringRef			protocolType)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkServiceSetEnabled
	@discussion Enables or disables the service.
	@param service The network service.
	@param enabled TRUE if the service should be enabled.
	@result TRUE if the enabled status was saved; FALSE if an error was encountered.
 */
Boolean
SCNetworkServiceSetEnabled			(SCNetworkServiceRef		service,
						 Boolean			enabled)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkServiceSetName
	@discussion Stores the [user specified] name for the service.
	@param service The network service.
	@param name The [user defined] name to associate with the service.
	@result TRUE if the name was saved; FALSE if an error was encountered.

	Note: although not technically required, the [user specified] names
	for all services within any given set should be unique.  As such, an
	error will be returned if you attemp to name two services with the
	same string.
 */
Boolean
SCNetworkServiceSetName				(SCNetworkServiceRef		service,
						 CFStringRef __nullable		name)		API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));


/* --------------------------------------------------------------------------------
 * SETS
 * -------------------------------------------------------------------------------- */

/*!
	@group Set configuration
 */

#pragma mark -
#pragma mark SCNetworkSet configuration (APIs)

/*!
	@function SCNetworkSetGetTypeID
	@discussion Returns the type identifier of all SCNetworkSet instances.
 */
CFTypeID
SCNetworkSetGetTypeID				(void)						API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkSetAddService
	@discussion Adds the network service to the set.
	@param set The network set.
	@param service The service to be added.
	@result TRUE if the service was added to the set; FALSE if the
		service was already present or an error was encountered.

	Note: prior to Mac OS X 10.5, the Network Preferences UI
	did not support having a single service being a member of
	more than one set.  An error will be returned if you attempt
	to add a service to more than one set on a pre-10.5 system.
 */
Boolean
SCNetworkSetAddService				(SCNetworkSetRef		set,
						 SCNetworkServiceRef		service)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkSetContainsInterface
	@discussion Checks if an interface is represented by at least one
		network service in the specified set.
	@param set The network set.
	@param interface The network interface.
	@result TRUE if the interface is represented in the set; FALSE if not.
 */
Boolean
SCNetworkSetContainsInterface			(SCNetworkSetRef		set,
						 SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.5))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkSetCopyAll
	@discussion Returns all available sets for the specified preferences.
	@param prefs The "preferences" session.
	@result The list of SCNetworkSet sets associated with the preferences.
		You must release the returned value.
 */
CFArrayRef /* of SCNetworkSetRef's */ __nullable
SCNetworkSetCopyAll				(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkSetCopyCurrent
	@discussion Returns the "current" set.
	@param prefs The "preferences" session.
	@result The current set; NULL if no current set has been defined.
 */
SCNetworkSetRef __nullable
SCNetworkSetCopyCurrent				(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkSetCopyServices
	@discussion Returns all network services associated with the set.
	@param set The network set.
	@result The list of SCNetworkService services associated with the set.
		You must release the returned value.
 */
CFArrayRef /* of SCNetworkServiceRef's */ __nullable
SCNetworkSetCopyServices			(SCNetworkSetRef		set)		API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkSetCreate
	@discussion Create a new set in the configuration.
	@param prefs The "preferences" session.
	@result A reference to the new SCNetworkSet.
		You must release the returned value.
 */
SCNetworkSetRef __nullable
SCNetworkSetCreate				(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkSetCopy
	@discussion Returns the set with the specified identifier.
	@param prefs The "preferences" session.
	@param setID The unique identifier for the set.
	@result A reference to the SCNetworkSet from the associated preferences;
		NULL if the setID does not exist in the preferences or if an
		error was encountered.
		You must release the returned value.
 */
SCNetworkSetRef __nullable
SCNetworkSetCopy				(SCPreferencesRef		prefs,
						 CFStringRef			setID)		API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkSetGetName
	@discussion Returns the [user specified] name associated with the set.
	@param set The network set.
	@result The [user specified] name.
 */
CFStringRef __nullable
SCNetworkSetGetName				(SCNetworkSetRef		set)		API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkSetGetSetID
	@discussion Returns the identifier for the set.
	@param set The network set.
	@result The set identifier.
 */
CFStringRef __nullable
SCNetworkSetGetSetID				(SCNetworkSetRef		set)		API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkSetGetServiceOrder
	@discussion Returns the [user specified] ordering of network services
		within the set.
	@param set The network set.
	@result The ordered list of CFStringRef service identifiers associated
		with the set;
		NULL if no service order has been specified or if an error
		was encountered.
 */
CFArrayRef /* of serviceID CFStringRef's */ __nullable
SCNetworkSetGetServiceOrder			(SCNetworkSetRef		set)		API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkSetRemove
	@discussion Removes the set from the configuration.
	@param set The network set.
	@result TRUE if the set was removed; FALSE if an error was encountered.
 */
Boolean
SCNetworkSetRemove				(SCNetworkSetRef		set)		API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkSetRemoveService
	@discussion Removes the network service from the set.
	@param set The network set.
	@param service The service to be removed.
	@result TRUE if the service was removed from the set; FALSE if the
		service was not already present or an error was encountered.
 */
Boolean
SCNetworkSetRemoveService			(SCNetworkSetRef		set,
						 SCNetworkServiceRef		service)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkSetSetCurrent
	@discussion Specifies the set that should be the "current" set.
	@param set The network set.
	@result TRUE if the current set was updated;
		FALSE if an error was encountered.
 */
Boolean
SCNetworkSetSetCurrent				(SCNetworkSetRef		set)		API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkSetSetName
	@discussion Stores the [user specified] name for the set.
	@param set The network set.
	@param name The [user defined] name to associate with the set.
	@result TRUE if the name was saved; FALSE if an error was encountered.

	Note: although not technically required, the [user specified] names
	for all set should be unique.  As such, an error will be returned if
	you attemp to name two sets with the same string.
 */
Boolean
SCNetworkSetSetName				(SCNetworkSetRef		set,
						 CFStringRef __nullable		name)		API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkSetSetServiceOrder
	@discussion Stores the [user specified] ordering of network services for the set.
	@param set The network set.
	@param newOrder The ordered list of CFStringRef service identifiers for the set.
	@result TRUE if the new service order was saved; FALSE if an error was encountered.
 */
Boolean
SCNetworkSetSetServiceOrder			(SCNetworkSetRef		set,
						 CFArrayRef			newOrder)	API_AVAILABLE(macos(10.4))
												SPI_AVAILABLE(ios(2.0), tvos(9.0), watchos(1.0), bridgeos(1.0));	/* serviceID CFStringRef's */


__END_DECLS

CF_ASSUME_NONNULL_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif	/* _SCNETWORKCONFIGURATION_H */

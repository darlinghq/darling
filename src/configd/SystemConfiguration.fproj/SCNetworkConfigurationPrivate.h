/*
 * Copyright (c) 2005-2018 Apple Inc. All rights reserved.
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

#ifndef _SCNETWORKCONFIGURATIONPRIVATE_H
#define _SCNETWORKCONFIGURATIONPRIVATE_H

#include <os/availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCValidation.h>
#include <IOKit/IOKitLib.h>

/*!
	@header SCNetworkConfigurationPrivate
 */

__BEGIN_DECLS


/*!
	@group Interface configuration
 */

#pragma mark -
#pragma mark SCNetworkInterface configuration (typedefs, consts, enums)

/*!
	@const kSCNetworkInterfaceTypeBridge
 */
extern const CFStringRef kSCNetworkInterfaceTypeBridge						API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));


/*!
	@const kSCNetworkInterfaceTypeLoopback
 */
extern const CFStringRef kSCNetworkInterfaceTypeLoopback					API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkInterfaceLoopback
	@discussion A network interface representing the loopback
		interface (lo0).
 */
extern const SCNetworkInterfaceRef kSCNetworkInterfaceLoopback					API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@const kSCNetworkInterfaceTypeVPN
 */
extern const CFStringRef kSCNetworkInterfaceTypeVPN						API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@group Interface configuration (Bridge)
 */

/*!
	@typedef SCBridgeInterfaceRef
	@discussion This is the type of a reference to an object that represents
		a bridge interface.
 */
typedef SCNetworkInterfaceRef SCBridgeInterfaceRef;

typedef CF_ENUM(uint32_t, SCNetworkServicePrimaryRank) {
	kSCNetworkServicePrimaryRankDefault	= 0,
	kSCNetworkServicePrimaryRankFirst	= 1,
	kSCNetworkServicePrimaryRankLast	= 2,
	kSCNetworkServicePrimaryRankNever	= 3,
	kSCNetworkServicePrimaryRankScoped	= 4
};

#pragma mark -
#pragma mark SCNetworkInterface configuration (SPI)

/*!
	@group Interface configuration
 */

static __inline__ CFTypeRef
isA_SCNetworkInterface(CFTypeRef obj)
{
	return (isA_CFType(obj, SCNetworkInterfaceGetTypeID()));
}

static __inline__ CFTypeRef
isA_SCBondInterface(CFTypeRef obj)
{
	CFStringRef	interfaceType;

	if (!isA_SCNetworkInterface(obj)) {
		// if not an SCNetworkInterface
		return NULL;
	}

	interfaceType = SCNetworkInterfaceGetInterfaceType((SCNetworkInterfaceRef)obj);
	if (!CFEqual(interfaceType, kSCNetworkInterfaceTypeBond)) {
		// if not a Bond
		return NULL;
	}

	return obj;
}

static __inline__ CFTypeRef
isA_SCBridgeInterface(CFTypeRef obj)
{
	CFStringRef	interfaceType;

	if (!isA_SCNetworkInterface(obj)) {
		// if not an SCNetworkInterface
		return NULL;
	}

	interfaceType = SCNetworkInterfaceGetInterfaceType((SCNetworkInterfaceRef)obj);
	if (!CFEqual(interfaceType, kSCNetworkInterfaceTypeBridge)) {
		// if not a bridge
		return NULL;
	}

	return obj;
}

static __inline__ CFTypeRef
isA_SCVLANInterface(CFTypeRef obj)
{
	CFStringRef	interfaceType;

	if (!isA_SCNetworkInterface(obj)) {
		// if not an SCNetworkInterface
		return NULL;
	}

	interfaceType = SCNetworkInterfaceGetInterfaceType((SCNetworkInterfaceRef)obj);
	if (!CFEqual(interfaceType, kSCNetworkInterfaceTypeVLAN)) {
		// if not a VLAN
		return NULL;
	}

	return obj;
}

/*!
	@function _SCNetworkInterfaceCompare
	@discussion Compares two SCNetworkInterface objects.
	@param val1 The SCNetworkInterface object.
	@param val2 The SCNetworkInterface object.
	@param context Not used.
	@result A comparison result.
 */
CFComparisonResult
_SCNetworkInterfaceCompare				(const void			*val1,
							 const void			*val2,
							 void				*context)	API_AVAILABLE(macos(10.5), ios(2.0));

/*!
	@function _SCNetworkInterfaceCopyActive
	@discussion Creates an SCNetworkInterface and associated with interface name
		and SCDynamicStoreRef
	@param store The SCDynamicStoreRef
	@param bsdName The interface name
	@result the SCNetworkInterface
 */
SCNetworkInterfaceRef
_SCNetworkInterfaceCopyActive				(SCDynamicStoreRef		store,
							 CFStringRef			bsdName)	API_AVAILABLE(macos(10.8), ios(5.0));

/*!
	@function _SCNetworkInterfaceCopyAllWithPreferences
		Returns all network capable interfaces on the system.
	@param prefs The "preferences" session.
	@result The list of interfaces on the system.
		You must release the returned value.
 */
CFArrayRef /* of SCNetworkInterfaceRef's */
_SCNetworkInterfaceCopyAllWithPreferences		(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(5.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function _SCNetworkInterfaceCopyBTPANInterface
	@discussion Returns the SCNetworkInterface associated with the BT-PAN interface
	@result The BT-PAN interface; NULL if the interface is not (yet) known.
 */
SCNetworkInterfaceRef
_SCNetworkInterfaceCopyBTPANInterface			(void)						API_AVAILABLE(macos(10.9)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@function _SCNetworkInterfaceCopySlashDevPath
	@discussion Returns the /dev pathname for the interface.
	@param interface The network interface.
	@result The /dev pathname associated with the interface (e.g. "/dev/modem");
		NULL if no path is available.
 */
CFStringRef
_SCNetworkInterfaceCopySlashDevPath			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.6), ios(3.0));

#define kIncludeNoVirtualInterfaces	0x0
#define kIncludeVLANInterfaces		0x1
#define kIncludeBondInterfaces		0x2
#define kIncludeBridgeInterfaces	0x4
#define kIncludeAllVirtualInterfaces	0xffffffff

/*!
	@function _SCNetworkInterfaceCreateWithBSDName
	@discussion Create a new network interface associated with the provided
		BSD interface name.  This API supports Ethernet, FireWire, and
		IEEE 802.11 interfaces.
	@param bsdName The BSD interface name.
	@param flags Indicates whether virtual (Bond, Bridge, VLAN)
		network interfaces should be included.
	@result A reference to the new SCNetworkInterface.
		You must release the returned value.
 */
SCNetworkInterfaceRef
_SCNetworkInterfaceCreateWithBSDName			(CFAllocatorRef			allocator,
							 CFStringRef			bsdName,
							 UInt32				flags)		API_AVAILABLE(macos(10.5), ios(2.0));

/*!
	@function _SCNetworkInterfaceCreateWithEntity
	@discussion Create a new network interface associated with the provided
		SCDynamicStore service entity dictionary.
	@param interface_entity The entity dictionary.
	@param service The network service.
	@result A reference to the new SCNetworkInterface.
		You must release the returned value.
 */
SCNetworkInterfaceRef
_SCNetworkInterfaceCreateWithEntity			(CFAllocatorRef			allocator,
							 CFDictionaryRef		interface_entity,
							 SCNetworkServiceRef		service)	API_AVAILABLE(macos(10.5), ios(2.0));

/*!
	@function _SCNetworkInterfaceCreateWithIONetworkInterfaceObject
	@discussion Create a new network interface associated with the provided
		IORegistry "IONetworkInterface" object.
	@param if_obj The IONetworkInterface object.
	@result A reference to the new SCNetworkInterface.
		You must release the returned value.
 */
SCNetworkInterfaceRef
_SCNetworkInterfaceCreateWithIONetworkInterfaceObject	(io_object_t			if_obj)		API_AVAILABLE(macos(10.5), ios(2.0));

/*!
	@function SCNetworkInterfaceGetPrimaryRank
	@discussion We allow caller to retrieve the rank on an interface.
	@param interface The interface to get the rank
	@result SCNetworkServicePrimaryRank
 */
SCNetworkServicePrimaryRank
SCNetworkInterfaceGetPrimaryRank			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.8), ios(5.0));

/*!
	@function SCNetworkInterfaceSetPrimaryRank
	@discussion We allow caller to set an assertion on an interface.
		The rank assertion lives as long as the SCNetworkInterfaceRef
		remains valid.
	@param interface The interface to set the rank assertion
	@param newRank The new rank to be set
	@result TRUE if operation is successful; FALSE if an error was encountered.
 */
Boolean
SCNetworkInterfaceSetPrimaryRank			(SCNetworkInterfaceRef		interface,
							 SCNetworkServicePrimaryRank	newRank)	API_AVAILABLE(macos(10.8), ios(5.0));

/**
 ** SCNetworkInterfaceAdvisory
 **/

typedef CF_ENUM(uint32_t, SCNetworkInterfaceAdvisory) {
	kSCNetworkInterfaceAdvisoryNone = 0,
	kSCNetworkInterfaceAdvisoryLinkLayerIssue = 1,
	kSCNetworkInterfaceAdvisoryUplinkIssue = 2,
};

/*!
	@function SCNetworkInterfaceSetAdvisory
	@discussion Advise the system of some condition on the network interface
	that warrants changing how the interface is used for IP networking,
	and to clear a previously set advisory.
	Calling this function requires root or having the boolean entitlement
	"com.apple.SystemConfiguration.SCNetworkInterfaceSetAdvisory" = true.
	@param interface The interface to assert the advisory on.
	@param advisory The advisory to indicate on the interface, use
	kSCNetworkInterfaceAdvisoryNone to clear advisory.
	@param reason A string indicating the reason for setting the advisory,
	used to aid debugging.
	@result TRUE if the advisory change was successful; FALSE otherwise.
*/
Boolean
SCNetworkInterfaceSetAdvisory(SCNetworkInterfaceRef interface,
			      SCNetworkInterfaceAdvisory advisory,
			      CFStringRef reason)
	API_AVAILABLE(macos(10.14), ios(12.0));

/*!
	@function SCNetworkInterfaceAdvisoryIsSet
	@discussion Find out if there is an advisory set on the interface.
	@param interface The interface to check for an advisory.
	@result TRUE if an advisory is set; FALSE otherwise.
*/
Boolean
SCNetworkInterfaceAdvisoryIsSet(SCNetworkInterfaceRef interface)
	API_AVAILABLE(macos(10.14), ios(12.0));

/*!
	@function SCNetworkInterfaceCopyAdvisoryNotificationKey
	@discussion Get the SCDynamicStore notication key for advisory changes
	made on the interface.
	@param interface The interface for which to get the notification key.
	@result Key used to receive advisory change notifications on the
	interface.
*/
CFStringRef
SCNetworkInterfaceCopyAdvisoryNotificationKey(SCNetworkInterfaceRef interface)
	API_AVAILABLE(macos(10.14), ios(12.0));


#define	kSCNetworkInterfaceConfigurationActionKey		CFSTR("New Interface Detected Action")
#define	kSCNetworkInterfaceConfigurationActionValueNone		CFSTR("None")
#define	kSCNetworkInterfaceConfigurationActionValuePrompt	CFSTR("Prompt")
#define	kSCNetworkInterfaceConfigurationActionValueConfigure	CFSTR("Configure")

#define	kSCNetworkInterfaceNetworkConfigurationOverridesKey	CFSTR("NetworkConfigurationOverrides")
#define	kSCNetworkInterfaceHiddenConfigurationKey		CFSTR("HiddenConfiguration")
#define	kSCNetworkInterfaceHiddenPortKey			CFSTR("HiddenPort") /* for serial ports */
#define	kSCNetworkInterfaceHiddenInterfaceKey			CFSTR("HiddenInterface") /* for network interfaces */

// IORegistry property to indicate that a [WWAN] interface is not yet ready
#define	kSCNetworkInterfaceInitializingKey			CFSTR("Initializing")

// IORegistry property to indicate that the attached host should be trusted before use
#define	kSCNetworkInterfaceTrustRequiredKey			CFSTR("TrustRequired")

/*!
	@function _SCNetworkInterfaceCopyInterfaceInfo
	@discussion Returns interface details
	@param interface The network interface.
	@result A dictionary with details about the network interface.
 */
CFDictionaryRef
_SCNetworkInterfaceCopyInterfaceInfo			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.6), ios(3.0));

/*!
	@function _SCNetworkInterfaceGetConfigurationAction
	@discussion Returns a user-notification / auto-configuration action for the interface.
	@param interface The network interface.
	@result The user-notification / auto-configuration action;
		NULL if the default action should be used.
 */
CFStringRef
_SCNetworkInterfaceGetConfigurationAction		(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.6), ios(2.0));

/*!
	@function _SCNetworkInterfaceGetFamilyType
	@discussion Returns the family type for the interface.
	@param interface The network interface.
	@result The family type (ift_family) associated with the interface;
		NULL if no family type is available.
 */
CFNumberRef
_SCNetworkInterfaceGetFamilyType			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.12), ios(10.0));

/*!
	@function _SCNetworkInterfaceGetFamilySubType
	@discussion Returns the family subtype for the interface.
	@param interface The network interface.
	@result The family subtype (ift_subfamily) associated with the interface;
		NULL if no family subtype is available.
 */
CFNumberRef
_SCNetworkInterfaceGetFamilySubType			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.12), ios(10.0));

/*!
	@function _SCNetworkInterfaceGetHardwareAddress
	@discussion Returns a link layer address for the interface.
	@param interface The network interface.
	@result The hardware (MAC) address for the interface.
		NULL if no hardware address is available.
 */
CFDataRef
_SCNetworkInterfaceGetHardwareAddress			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.5), ios(2.0));

/*!
	@function _SCNetworkInterfaceGetIOInterfaceNamePrefix
	@discussion Returns the IOInterfaceNamePrefix for the interface.
	@param interface The network interface.
	@result The IOInterfaceNamePrefix associated with the interface;
		NULL if no IOInterfaceNamePrefix is available.
 */
CFStringRef
_SCNetworkInterfaceGetIOInterfaceNamePrefix		(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.8), ios(6.0));

/*!
	@function _SCNetworkInterfaceGetIOInterfaceType
	@discussion Returns the IOInterfaceType for the interface.
	@param interface The network interface.
	@result The IOInterfaceType associated with the interface
 */
CFNumberRef
_SCNetworkInterfaceGetIOInterfaceType			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.5), ios(2.0));

/*!
	@function _SCNetworkInterfaceGetIOInterfaceUnit
	@discussion Returns the IOInterfaceUnit for the interface.
	@param interface The network interface.
	@result The IOInterfaceUnit associated with the interface;
		NULL if no IOInterfaceUnit is available.
 */
CFNumberRef
_SCNetworkInterfaceGetIOInterfaceUnit			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.5), ios(2.0));

/*!
	@function _SCNetworkInterfaceGetIOPath
	@discussion Returns the IOPath for the interface.
	@param interface The network interface.
	@result The IOPath associated with the interface;
		NULL if no IOPath is available.
 */
CFStringRef
_SCNetworkInterfaceGetIOPath				(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.5), ios(2.0));

/*!
	@function _SCNetworkInterfaceGetIORegistryEntryID
	@discussion Returns the IORegistry entry ID for the interface.
	@param interface The network interface.
	@result The IORegistry entry ID associated with the interface;
		Zero if no entry ID is available.
 */
uint64_t
_SCNetworkInterfaceGetIORegistryEntryID			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.8), ios(5.0));

/*!
	@function _SCNetworkInterfaceIsApplePreconfigured
	@discussion Identifies if a network interface is internal/pre-configured.
	@param interface The network interface.
	@result TRUE if the interface is an internal/pre-configured.
 */
Boolean
_SCNetworkInterfaceIsApplePreconfigured			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.12), ios(10.0));

/*!
	@function _SCNetworkInterfaceIsBluetoothPAN
	@discussion Identifies if a network interface is a Bluetooth PAN (GN) device.
	@param interface The network interface.
	@result TRUE if the interface is a Bluetooth PAN device.
 */
Boolean
_SCNetworkInterfaceIsBluetoothPAN			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.6), ios(3.0));

/*!
	@function _SCNetworkInterfaceIsBluetoothPAN_NAP
	@discussion Identifies if a network interface is a Bluetooth PAN-NAP device.
	@param interface The network interface.
	@result TRUE if the interface is a Bluetooth PAN-NAP device.
 */
Boolean
_SCNetworkInterfaceIsBluetoothPAN_NAP			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.7), ios(5.0));

/*!
	@function _SCNetworkInterfaceIsBluetoothP2P
	@discussion Identifies if a network interface is a Bluetooth P2P (PAN-U) device.
	@param interface The network interface.
	@result TRUE if the interface is a Bluetooth P2P device.
 */
Boolean
_SCNetworkInterfaceIsBluetoothP2P			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.7), ios(5.0));

/*!
	@function _SCNetworkInterfaceIsBuiltin
	@discussion Identifies if a network interface is "built-in".
	@param interface The network interface.
	@result TRUE if the interface is "built-in".
 */
Boolean
_SCNetworkInterfaceIsBuiltin				(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.5), ios(2.0));

/*!
	@function _SCNetworkInterfaceIsCarPlay
	@discussion Identifies if a network interface is a CarPlay device.
	@param interface The network interface.
	@result TRUE if the interface is a CarPlay device.
 */
Boolean
_SCNetworkInterfaceIsCarPlay				(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.15), ios(13.0));

/*!
	@function _SCNetworkInterfaceIsHiddenConfiguration
	@discussion Identifies if the configuration of a network interface should be
		hidden from any user interface (e.g. the "Network" pref pane).
	@param interface The network interface.
	@result TRUE if the interface configuration should be hidden.
 */
Boolean
_SCNetworkInterfaceIsHiddenConfiguration		(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.7), ios(4.0));

/*!
	@function _SCNetworkInterfaceIsTethered
	@discussion Identifies if a network interface is an Apple tethered device (e.g. an iPhone).
	@param interface The network interface.
	@result TRUE if the interface is a tethered device.
 */
Boolean
_SCNetworkInterfaceIsTethered				(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.6), ios(3.0));

/*!
	@function _SCNetworkInterfaceIsThunderbolt
	@discussion Identifies if a network interface is a Thunderbolt device
	@param interface The network interface.
	@result TRUE if the interface is a Thunderbolt device.
 */
Boolean
_SCNetworkInterfaceIsThunderbolt			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.9), ios(7.0));

/*!
	@function _SCNetworkInterfaceIsTrustRequired
	@discussion Identifies if a network interface requires that the
		associated host be trusted.
	@param interface The network interface.
	@result TRUE if the interface requires trust.
 */
Boolean
_SCNetworkInterfaceIsTrustRequired			(SCNetworkInterfaceRef		interface)	SPI_AVAILABLE(macos(10.14)) API_AVAILABLE(ios(12.0));

/*!
	@function _SCNetworkInterfaceIsPhysicalEthernet
	@discussion Indicates whether a network interface is a real ethernet interface i.e. one with an ethernet PHY.
	@param interface The network interface.
	@result TRUE if the interface is a real ethernet interface.
 */
Boolean
_SCNetworkInterfaceIsPhysicalEthernet			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.7), ios(5.0));

/*!
	@function _SCNetworkInterfaceForceConfigurationRefresh
	@discussion Forces a configuration refresh of the
		specified interface.
	@param ifName Network interface name.
	@result TRUE if the refresh was successfully posted.
 */
Boolean
_SCNetworkInterfaceForceConfigurationRefresh		(CFStringRef			ifName)		API_AVAILABLE(macos(10.5), ios(2.0));

/*!
	@function SCNetworkInterfaceCopyCapability
	@discussion For the specified network interface, returns information
		about the currently requested capabilities, the active capabilities,
		and the capabilities which are available.
	@param interface The desired network interface.
	@param capability The desired capability;
		NULL to return a CFDictionary of all capabilities.
	@result a CFTypeRef representing the current value of requested
		capability;
		NULL if the capability is not available for this
		interface or if an error was encountered.
		You must release the returned value.
 */
CFTypeRef
SCNetworkInterfaceCopyCapability			(SCNetworkInterfaceRef		interface,
							 CFStringRef			capability)	API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(5.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCNetworkInterfaceSetCapability
	@discussion For the specified network interface, sets the requested
		capabilities.
	@param interface The desired network interface.
	@param capability The desired capability.
	@param newValue The new requested setting for the capability;
		NULL to restore the default setting.
	@result TRUE if the configuration was updated; FALSE if an error was encountered.
 */
Boolean
SCNetworkInterfaceSetCapability				(SCNetworkInterfaceRef		interface,
							 CFStringRef			capability,
							 CFTypeRef			newValue)	API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(5.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

Boolean
__SCNetworkInterfaceSetDisableUntilNeededValue		(SCNetworkInterfaceRef		interface,
							 CFTypeRef			disable)	API_AVAILABLE(macos(10.11)) SPI_AVAILABLE(ios(9.0), tvos(9.0), watchos(2.0), bridgeos(2.0));


int
__SCNetworkInterfaceCreateCapabilities			(SCNetworkInterfaceRef		interface,
							 int				capability_base,
							 CFDictionaryRef		capability_options)	API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(5.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

int
__SCNetworkInterfaceCreateMediaOptions			(SCNetworkInterfaceRef		interface,
							 CFDictionaryRef		media_options)		API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(5.0), tvos(9.0), watchos(1.0), bridgeos(1.0));


#pragma mark -
#pragma mark SCBondInterface configuration (SPIs)

/*!
	@function _SCBondInterfaceCopyActive
	@discussion Returns all Ethernet Bond interfaces on the system.
	@result The list of SCBondInterface interfaces on the system.
		You must release the returned value.
 */
CFArrayRef
_SCBondInterfaceCopyActive				(void)						API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@function _SCBondInterfaceUpdateConfiguration
	@discussion Updates the bond interface configuration.
	@param prefs The "preferences" session.
	@result TRUE if the bond interface configuration was updated.; FALSE if the
		an error was encountered.
 */
Boolean
_SCBondInterfaceUpdateConfiguration			(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@function SCBondInterfaceGetMode
	@discussion Return the mode for the given bond interface.
	@param bond The bond interface to get the mode from.
	@result A CFNumberRef containing the mode (IF_BOND_MODE_{LACP,STATIC}).
 */
CFNumberRef
SCBondInterfaceGetMode					(SCBondInterfaceRef		bond)		API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

/*!
	@function SCBondInterfaceSetMode
	@discussion Set the mode on the bond interface.
	@param bond The bond interface on which to adjust the mode.
	@param mode The mode value (0=IF_BOND_MODE_LACP,1=IF_BOND_MODE_STATIC)
	@result TRUE if operation succeeded.
 */
Boolean
SCBondInterfaceSetMode					(SCBondInterfaceRef		bond,
							 CFNumberRef			mode)		API_AVAILABLE(macos(10.5)) API_UNAVAILABLE(ios, tvos, watchos, bridgeos);

#pragma mark -
#pragma mark SCBridgeInterface configuration (SPIs)

/*!
	@function SCBridgeInterfaceCopyAll
	@discussion Returns all bridge interfaces on the system.
	@param prefs The "preferences" session.
	@result The list of bridge interfaces on the system.
		You must release the returned value.
 */
CFArrayRef /* of SCBridgeInterfaceRef's */
SCBridgeInterfaceCopyAll				(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCBridgeInterfaceCopyAvailableMemberInterfaces
	@discussion Returns all network capable devices on the system
		that can be added to an bridge interface.
	@param prefs The "preferences" session.
	@result The list of interfaces.
		You must release the returned value.
 */
CFArrayRef /* of SCNetworkInterfaceRef's */
SCBridgeInterfaceCopyAvailableMemberInterfaces		(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCBridgeInterfaceCreate
	@discussion Create a new SCBridgeInterface interface.
	@param prefs The "preferences" session.
	@result A reference to the new SCBridgeInterface.
		You must release the returned value.
 */
SCBridgeInterfaceRef
SCBridgeInterfaceCreate					(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCBridgeInterfaceRemove
	@discussion Removes the SCBridgeInterface from the configuration.
	@param bridge The SCBridgeInterface interface.
	@result TRUE if the interface was removed; FALSE if an error was encountered.
 */
Boolean
SCBridgeInterfaceRemove					(SCBridgeInterfaceRef		bridge)		API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCBridgeInterfaceGetMemberInterfaces
	@discussion Returns the member interfaces for the specified bridge interface.
	@param bridge The SCBridgeInterface interface.
	@result The list of interfaces.
 */
CFArrayRef /* of SCNetworkInterfaceRef's */
SCBridgeInterfaceGetMemberInterfaces			(SCBridgeInterfaceRef		bridge)		API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCBridgeInterfaceGetOptions
	@discussion Returns the configuration settings associated with a bridge interface.
	@param bridge The SCBridgeInterface interface.
	@result The configuration settings associated with the bridge interface;
		NULL if no changes to the default configuration have been saved.
 */
CFDictionaryRef
SCBridgeInterfaceGetOptions				(SCBridgeInterfaceRef		bridge)		API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCBridgeInterfaceSetMemberInterfaces
	@discussion Sets the member interfaces for the specified bridge interface.
	@param bridge The SCBridgeInterface interface.
	@param members The desired member interfaces.
	@result TRUE if the configuration was stored; FALSE if an error was encountered.
 */
Boolean
SCBridgeInterfaceSetMemberInterfaces			(SCBridgeInterfaceRef		bridge,
							 CFArrayRef			members) /* of SCNetworkInterfaceRef's */
													API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCBridgeInterfaceSetLocalizedDisplayName
	@discussion Sets the localized display name for the specified bridge interface.
	@param bridge The SCBridgeInterface interface.
	@param newName The new display name.
	@result TRUE if the configuration was stored; FALSE if an error was encountered.
 */
Boolean
SCBridgeInterfaceSetLocalizedDisplayName		(SCBridgeInterfaceRef		bridge,
							 CFStringRef			newName)	API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function SCBridgeInterfaceSetOptions
	@discussion Sets the configuration settings for the specified bridge interface.
	@param bridge The SCBridgeInterface interface.
	@param newOptions The new configuration settings.
	@result TRUE if the configuration was stored; FALSE if an error was encountered.
 */
Boolean
SCBridgeInterfaceSetOptions				(SCBridgeInterfaceRef		bridge,
							 CFDictionaryRef		newOptions)	API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

#pragma mark -

/*!
	@function _SCBridgeInterfaceCopyActive
	@discussion Returns all bridge interfaces on the system.
	@result The list of SCBridgeInterface interfaces on the system.
		You must release the returned value.
 */
CFArrayRef
_SCBridgeInterfaceCopyActive				(void)						API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function _SCBridgeInterfaceUpdateConfiguration
	@discussion Updates the bridge interface configuration.
	@param prefs The "preferences" session.
	@result TRUE if the bridge interface configuration was updated.; FALSE if the
		an error was encountered.
 */
Boolean
_SCBridgeInterfaceUpdateConfiguration			(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));


#pragma mark -
#pragma mark SCVLANInterface configuration (SPIs)

/*!
	@function _SCVLANInterfaceCopyActive
	@discussion Returns all VLAN interfaces on the system.
	@result The list of SCVLANInterface interfaces on the system.
		You must release the returned value.
 */
CFArrayRef
_SCVLANInterfaceCopyActive				(void)						API_AVAILABLE(macos(10.5)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function _SCVLANInterfaceUpdateConfiguration
	@discussion Updates the VLAN interface configuration.
	@param prefs The "preferences" session.
	@result TRUE if the VLAN interface configuration was updated.; FALSE if the
		an error was encountered.
 */
Boolean
_SCVLANInterfaceUpdateConfiguration			(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.5)) SPI_AVAILABLE(ios(4.0), tvos(9.0), watchos(1.0), bridgeos(1.0));


#pragma mark -
#pragma mark SCNetworkInterface Password SPIs


enum {
	kSCNetworkInterfacePasswordTypePPP		= 1,
	kSCNetworkInterfacePasswordTypeIPSecSharedSecret,
	kSCNetworkInterfacePasswordTypeEAPOL,
	kSCNetworkInterfacePasswordTypeIPSecXAuth,
	kSCNetworkInterfacePasswordTypeVPN,
};
typedef uint32_t	SCNetworkInterfacePasswordType;

Boolean
SCNetworkInterfaceCheckPassword				(SCNetworkInterfaceRef		interface,
							 SCNetworkInterfacePasswordType	passwordType)	API_AVAILABLE(macos(10.5), ios(2.0));

CFDataRef
SCNetworkInterfaceCopyPassword				(SCNetworkInterfaceRef		interface,
							 SCNetworkInterfacePasswordType	passwordType)	API_AVAILABLE(macos(10.5), ios(2.0));

Boolean
SCNetworkInterfaceRemovePassword			(SCNetworkInterfaceRef		interface,
							 SCNetworkInterfacePasswordType	passwordType)	API_AVAILABLE(macos(10.5), ios(2.0));

Boolean
SCNetworkInterfaceSetPassword				(SCNetworkInterfaceRef		interface,
							 SCNetworkInterfacePasswordType	passwordType,
							 CFDataRef			password,
							 CFDictionaryRef		options)	API_AVAILABLE(macos(10.5), ios(2.0));


Boolean
SCNetworkInterfaceGetDisableUntilNeeded			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.11), ios(9.0));

Boolean
SCNetworkInterfaceSetDisableUntilNeeded			(SCNetworkInterfaceRef		interface,
							 Boolean			disable)	API_AVAILABLE(macos(10.11), ios(9.0));


CFDictionaryRef
SCNetworkInterfaceGetQoSMarkingPolicy			(SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.13), ios(10.0));

Boolean
SCNetworkInterfaceSetQoSMarkingPolicy			(SCNetworkInterfaceRef		interface,
							 CFDictionaryRef		policy)		API_AVAILABLE(macos(10.13), ios(10.0));


#pragma mark -
#pragma mark SCNetworkProtocol configuration (SPI)


/*!
	@group Protocol configuration
 */


static __inline__ CFTypeRef
isA_SCNetworkProtocol(CFTypeRef obj)
{
	return (isA_CFType(obj, SCNetworkProtocolGetTypeID()));
}

/*!
	@function _SCNetworkProtocolCompare
	@discussion Compares two SCNetworkProtocol objects.
	@param val1 The SCNetworkProtocol object.
	@param val2 The SCNetworkProtocol object.
	@param context Not used.
	@result A comparison result.
 */
CFComparisonResult
_SCNetworkProtocolCompare				(const void			*val1,
							 const void			*val2,
							 void				*context)	API_AVAILABLE(macos(10.13), ios(11.0));


#pragma mark -
#pragma mark SCNetworkService configuration (SPI)


/*!
	@group Service configuration
 */


static __inline__ CFTypeRef
isA_SCNetworkService(CFTypeRef obj)
{
	return (isA_CFType(obj, SCNetworkServiceGetTypeID()));
}

/*!
	@function _SCNetworkServiceCompare
	@discussion Compares two SCNetworkService objects.
	@param val1 The SCNetworkService object.
	@param val2 The SCNetworkService object.
	@param context The service order (from SCNetworkSetGetServiceOrder).
	@result A comparison result.
 */
CFComparisonResult
_SCNetworkServiceCompare				(const void			*val1,
							 const void			*val2,
							 void				*context)	API_AVAILABLE(macos(10.7), ios(4.0));

/*!
	@function _SCNetworkServiceCopyActive
	@discussion Returns the network service with the specified identifier.

	Note: The service returned by this SPI differs from the SCNetworkServiceCopy
	      API in that queries and operations interact with the "active" service
	      represented in the SCDynamicStore.  Only a limited subset of the
	      SCNetworkService APIs are supported.
	@param store The dynamic store session.
	@param serviceID The unique identifier for the service.
	@result A reference to the SCNetworkService represented in the SCDynamicStore;
		NULL if the serviceID does not exist in the SCDynamicStore or if an
		error was encountered.
		You must release the returned value.
 */
SCNetworkServiceRef
_SCNetworkServiceCopyActive				(SCDynamicStoreRef		store,
							 CFStringRef			serviceID)	API_AVAILABLE(macos(10.6), ios(2.1));

/*!
	@function SCNetworkServiceGetPrimaryRank
	@discussion Returns the primary service rank associated with a service.
	@param service The network service.
	@result The primary service rank associated with the specified application;
		kSCNetworkServicePrimaryRankDefault if no rank is associated with the
		application or an error was encountered.
 */
SCNetworkServicePrimaryRank
SCNetworkServiceGetPrimaryRank				(SCNetworkServiceRef		service)	API_AVAILABLE(macos(10.6), ios(2.0));

/*!
	@function SCNetworkServiceSetPrimaryRank
	@discussion Updates the the primary service rank associated with a service.
	@param service The network service.
	@param newRank The new primary service rank; kSCNetworkServicePrimaryRankDefault
		if the default service rank should be used.
	@result TRUE if the rank was stored; FALSE if an error was encountered.

	Notes : The kSCNetworkServicePrimaryRankFirst and kSCNetworkServicePrimaryRankLast
		values can only valid as a transient setting.
 */
Boolean
SCNetworkServiceSetPrimaryRank				(SCNetworkServiceRef		service,
							 SCNetworkServicePrimaryRank	newRank)	API_AVAILABLE(macos(10.6), ios(2.0));

/*!
	@function _SCNetworkServiceIsVPN
	@discussion Identifies a VPN service.
	@param service The network service.
	@result TRUE if the service is a VPN.
 */
Boolean
_SCNetworkServiceIsVPN					(SCNetworkServiceRef		service)	API_AVAILABLE(macos(10.7), ios(4.0));

/*!
	@function SCNetworkServiceSetExternalID
	@discussion Set the external identifier for a network service.
	@param service A reference to the network service.
	@param identifierDomain A service can have multiple external identifiers. This string specifies which external identifier to set.
	@param identifier The new external identifier to assign to the network service.
	@result Returns TRUE if the external identifier was set successfully, FALSE if an error occurred.
 */
Boolean
SCNetworkServiceSetExternalID				(SCNetworkServiceRef		service,
							 CFStringRef			identifierDomain,
							 CFStringRef identifier);

/*!
	@function SCNetworkServiceCopyExternalID
	@discussion Copy the external identifier for a network service.
	@param service The network service.
	@param identifierDomain A service can have multiple external identifiers. This string specifies which external identifier to copy.
	@result Returns the service's external identifier, or NULL if the service does not have an external identifier in the given domain.
*/
CFStringRef
SCNetworkServiceCopyExternalID				(SCNetworkServiceRef		service,
							 CFStringRef			identifierDomain);

/*!
	@function _SCNetworkServiceSetServiceID
	@discussion Sets serviceID of the service to a different value provided.
	@param service The network service
	@param newServiceID The new service ID
	@result TRUE if new service ID is set successfully.
 */
Boolean
_SCNetworkServiceSetServiceID				(SCNetworkServiceRef		service,
							 CFStringRef			newServiceID)	API_AVAILABLE(macos(10.10), ios(8.0));

#pragma mark -
#pragma mark SCNetworkSet configuration (SPI)


/*!
	@group Set configuration
 */


static __inline__ CFTypeRef
isA_SCNetworkSet(CFTypeRef obj)
{
	return (isA_CFType(obj, SCNetworkSetGetTypeID()));
}


/*!
	@function _SCNetworkSetCompare
	@discussion Compares two SCNetworkSet objects.
	@param val1 The SCNetworkSet object.
	@param val2 The SCNetworkSet object.
	@param context Not used.
	@result A comparison result.
 */
CFComparisonResult
_SCNetworkSetCompare					(const void			*val1,
							 const void			*val2,
							 void				*context)	API_AVAILABLE(macos(10.13), ios(11.0));

/*!
	@function SCNetworkSetCopyAvailableInterfaces
	@discussion Returns all available interfaces for the set.
		The interfaces excludes those of bond and bridge members.
	@param set The network set.
	@result The list of SCNetworkInterfaces.
		You must release the returned value.
 */
CFArrayRef
SCNetworkSetCopyAvailableInterfaces			(SCNetworkSetRef		set)		API_AVAILABLE(macos(10.9)) SPI_AVAILABLE(ios(7.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function _SCNetworkSetCreateDefault
	@discussion Create a new [default] set in the configuration.
	@param prefs The "preferences" session.
	@result A reference to the new SCNetworkSet.
		You must release the returned value.
 */
SCNetworkSetRef
_SCNetworkSetCreateDefault				(SCPreferencesRef		prefs)		API_AVAILABLE(macos(10.12)) SPI_AVAILABLE(ios(10.0), tvos(10.0), watchos(3.0), bridgeos(3.0));

/*!
	@function SCNetworkSetEstablishDefaultConfiguration
	@discussion Updates a network set by adding services for
		any network interface that is not currently
		represented.
		If the provided set contains one (or more) services, new
		services will only be added for those interfaces that are
		not represented in *any* set.
		Otherwise, new services will be added for those interfaces
		that are not represented in the provided set.
		The new services are established with "default" configuration
		options.
	@param set The network set.
	@result TRUE if the configuration was updated; FALSE if no
		changes were required or if an error was encountered.
*/
Boolean
SCNetworkSetEstablishDefaultConfiguration		(SCNetworkSetRef		set)		API_AVAILABLE(macos(10.5), ios(2.0));

/*!
	@function SCNetworkSetEstablishDefaultInterfaceConfiguration
	@discussion Updates a network set by adding services for
		the specified network interface if is not currently
		represented.
		If the provided set contains one (or more) services, new
		services will only be added for interfaces that are not
		represented in *any* set.
		Otherwise, new services will be added for interfaces that
		are not represented in the provided set.
		The new services are established with "default" configuration
		options.
	@param set The network set.
	@param interface The network interface.
	@result TRUE if the configuration was updated; FALSE if no
		changes were required or if an error was encountered.
 */
Boolean
SCNetworkSetEstablishDefaultInterfaceConfiguration	(SCNetworkSetRef		set,
							 SCNetworkInterfaceRef		interface)	API_AVAILABLE(macos(10.5), ios(2.0));

/*!
	@function SCNetworkSetCopySelectedVPNService
	@discussion On the iPhone we only allow a single VPN network service
		to be selected at any given time.  This API will identify
		the selected VPN service.
	@param set The network set.
	@result The selected VPN service; NULL if no service has been
		selected.
		You must release the returned value.
 */
SCNetworkServiceRef
SCNetworkSetCopySelectedVPNService			(SCNetworkSetRef		set)		API_AVAILABLE(macos(10.7), ios(4.0));

/*!
	@function SCNetworkSetSetSelectedVPNService
	@discussion On the iPhone we only allow a single VPN network service
		to be selected at any given time.  This API should be used to
		select a VPN service.
	@param set The network set.
	@param service The VPN service to be selected.
	@result TRUE if the name was saved; FALSE if an error was encountered.
 */
Boolean
SCNetworkSetSetSelectedVPNService			(SCNetworkSetRef		set,
							 SCNetworkServiceRef		service)	API_AVAILABLE(macos(10.7), ios(4.0));

Boolean
_SCNetworkSetSetSetID					(SCNetworkSetRef		set,
							 CFStringRef			setID)		API_AVAILABLE(macos(10.10), ios(8.0));

/*!
	@group VPN Service configuration
 */

#pragma mark -
#pragma mark VPN Service configuration

typedef SCNetworkServiceRef VPNServiceRef;

/*!
	@function VPNServiceCopyAllMatchingExternalID
	@discussion Copy the VPN services with the given external identifier.
	@param prefs A reference to the prefs where the VPN services are stored.
	@param identifierDomain A service can have multiple external identifiers. This string specifies which one to match with the given identifier.
	@param identifier The external identifier of the desired services.
	@result A array of references to the VPN services with the given identifier, or NULL if no such service exists
 */
CFArrayRef
VPNServiceCopyAllMatchingExternalID		(SCPreferencesRef		prefs,
						 CFStringRef			identifierDomain,
						 CFStringRef identifier)		API_AVAILABLE(macos(10.9)) SPI_AVAILABLE(ios(7.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function VPNServiceCopyAll
	@discussion Copy all VPN services.
	@param prefs A reference to the prefs where the VPN services are stored.
	@result An array containing VPNServiceRefs for all the VPN services.
 */
CFArrayRef
VPNServiceCopyAll				(SCPreferencesRef		prefs)	API_AVAILABLE(macos(10.9)) SPI_AVAILABLE(ios(7.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function VPNServiceCopyAppRuleIDs
	@discussion Copy all the app rule identifiers for a VPN service.
	@param service A reference to the VPN service.
	@result An array of CFStringRefs, each string containing the identifier of a app rule in the given service.
 */
CFArrayRef
VPNServiceCopyAppRuleIDs			(VPNServiceRef			service)	API_AVAILABLE(macos(10.9)) SPI_AVAILABLE(ios(7.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function VPNServiceSetAppRule
	@discussion Add or modify an app rule in a VPN service. The ruleSettings dictionary must contain one of the following keys:
		<pre>kSCValNetVPNAppRuleExecutableMatch</pre>
		<pre>kSCValNetVPNAppRuleAccountIdentifierMatch</pre>
	The ruleSettings dictionary may also contain the following keys:
		<pre>kSCValNetVPNAppRuleDNSDomainMatch</pre>
	See SCSchemaDefinitionsPrivate.h for more details.
	@param service A reference to the VPN service.
	@param ruleIdentifier The identifier of the new app rule.
	@param ruleSettings The settings for the new app rule. See the dictionary keys defined above.
	@result TRUE if the app rule was set successfully, FALSE if an error occurred.
 */
Boolean
VPNServiceSetAppRule				(VPNServiceRef			service,
						 CFStringRef			ruleIdentifier,
						 CFDictionaryRef		ruleSettings)	API_AVAILABLE(macos(10.9)) SPI_AVAILABLE(ios(7.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function VPNServiceCopyAppRule
	@discussion Copy the settings for a app rule in a VPN service.
	@param service The app tunnel service.
	@param ruleIdentifier The ID of the app rule.
	@result The rule settings, or NULL if the app rule could not be found.
 */
CFDictionaryRef
VPNServiceCopyAppRule				(VPNServiceRef			service,
						 CFStringRef			ruleIdentifier)	API_AVAILABLE(macos(10.9)) SPI_AVAILABLE(ios(7.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function VPNServiceRemoveAppRule
	@discussion Remove an app rule from a VPN service.
	@param service The VPN service.
	@param ruleIdentifier The ID of the app rule to remove.
	@result Returns TRUE if the app rule was removed successfully; FALSE otherwise.
 */
Boolean
VPNServiceRemoveAppRule				(VPNServiceRef			service,
						 CFStringRef			ruleIdentifier)	API_AVAILABLE(macos(10.9)) SPI_AVAILABLE(ios(7.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@function VPNServiceIsManagedAppVPN
	@discussion Check to see if a VPN service is a managed App VPN service
	@param service The VPN servie.
	@result Returns TRUE if the service is a managed App VPN service; FALSE otherwise.
*/
Boolean
VPNServiceIsManagedAppVPN			(VPNServiceRef			service)	API_AVAILABLE(macos(10.9)) SPI_AVAILABLE(ios(7.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

/*!
	@group	Migration SPI
 */
#pragma mark -
#pragma mark Migration SPI

extern const CFStringRef kSCNetworkConfigurationRepair			/* CFBoolean */		API_AVAILABLE(macos(10.10), ios(8.0));

extern const CFStringRef kSCNetworkConfigurationMigrationActionKey	/* CFNumber */		API_AVAILABLE(macos(10.10), ios(8.0));

typedef CF_ENUM(uint32_t, SCNetworkConfigurationMigrationAction) {
	kSCNetworkConfigurationMigrationAction_CleanInstall	= 0,
	kSCNetworkConfigurationMigrationAction_Upgrade		= 1,
	kSCNetworkConfigurationMigrationAction_Restore		= 2,
};

/*!
	@function _SCNetworkConfigurationCopyMigrationPaths
	@result	Returns an array of paths that we would need from the source
 */
CFArrayRef
_SCNetworkConfigurationCopyMigrationPaths(CFDictionaryRef options)				API_AVAILABLE(macos(10.10), ios(8.0));

/*!
	@function _SCNetworkConfigurationPerformMigration
	@discussion Updates the network configuration of the target system with
		configurations from previous system. Both sourceDir and targetDir
		cannot be NULL, since NULL indicates API to look at the local system
	@param sourceDir A reference which points to the root of a directory populated
		with the list of requested directories/path from the "source" volume. Passing NULL
		will indicate that sourceDir should point to local system
	@param currentDir A reference which points to the root of a directory populated
		with the list of requested directories/path from the "destination" volume. Passing
		NULL will indicate that currentDir should point to local system.
	@param targetDir A reference which points to the root of a directory that we
		will populate (update) with new configuration. Passing NULL will mean that we want to
		migrate to the currentDir. If not NULL, then this path should exist.
	@param options Argument which will tell us what action we are supposed to take
		(clean-install, upgrade, migrate/restore settings from another system, ...)
	@result Returns array which would consist of those paths that should be moved
		from the "targetDir" directory to destination volume. You must release the returned value.
 */

CF_RETURNS_RETAINED
CFArrayRef
_SCNetworkConfigurationPerformMigration		(CFURLRef			sourceDir,
						 CFURLRef			currentDir,
						 CFURLRef			targetDir,
						 CFDictionaryRef		options)	API_AVAILABLE(macos(10.10), ios(8.0));


/*!
	@function _SCNetworkConfigurationCheckValidity
	@discussion Verifies whether the configuration files present in the specified
		directory have valid mappings or not
	@param configDir A reference which points to the directory where the configuration
		files are present
	@result TRUE if valid configurations are found

 */

Boolean
_SCNetworkConfigurationCheckValidity		(CFURLRef			configDir,
						 CFDictionaryRef		options)	API_AVAILABLE(macos(10.10), ios(8.0));


/*!
 @function	_SCNetworkConfigurationCheckValidityWithPreferences
 @discussion	Validates the specified preferences.plist against NetworkInterfaces.plist
 @param prefs	the preferences ref pointing to the said preferences.plist
 @param ni_prefs	the preferences ref pointing to the said NetworkInterfaces.plist
 @result	TRUE if the configurations are valid against each other

 */

Boolean
_SCNetworkConfigurationCheckValidityWithPreferences
						(SCPreferencesRef		prefs,
						 SCPreferencesRef		ni_prefs,
						 CFDictionaryRef		options)	API_AVAILABLE(macos(10.11), ios(9.0));


/*!
 @function _SCNetworkMigrationAreConfigurationsIdentical
 @discussion Compares the migration output between network configurations
		with the expected output.
 @param configurationURL A URL pointing to the top-level directory of the
		configuration to compare. This directory is expected to have
		a Library/Preferences/SystemConfiguration subdirectoy.
 @param expectedConfigurationURL A URL pointing to the top-level directory of
		the expected configuration. This directory is expected to have
		a Library/Preferences/SystemConfiguration subdirectoy.
 @result TRUE if configurations match with the expected configurations

 */

Boolean
_SCNetworkMigrationAreConfigurationsIdentical	(CFURLRef			configurationURL,
						 CFURLRef			expectedConfigurationURL)	API_AVAILABLE(macos(10.10), ios(8.0));

/*!
 @function	_SCNetworkConfigurationCopyMigrationRemovePaths
 @discussion	List of paths to files which we want to be removed from the target filesystem after migration
 @param targetPaths	the CFArray returned by _SCNetworkConfigurationPerformMigration
 @param targetDir	the CFURL passed to _SCNetworkConfigurationPerformMigration
 @result	An array of CFURL's; NULL if no paths need to be removed from the target filesystem

*/

CFArrayRef	// of CFURLRef's
_SCNetworkConfigurationCopyMigrationRemovePaths	(CFArrayRef	targetPaths,
						 CFURLRef	targetDir)				API_AVAILABLE(macos(10.10), ios(8.0));

__END_DECLS

#endif	/* _SCNETWORKCONFIGURATIONPRIVATE_H */

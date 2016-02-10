/*
 * Copyright (c) 2005-2014 Apple Inc. All rights reserved.
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

#include <Availability.h>
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
extern const CFStringRef kSCNetworkInterfaceTypeBridge						__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0/*SPI*/);


/*!
	@const kSCNetworkInterfaceTypeLoopback
 */
extern const CFStringRef kSCNetworkInterfaceTypeLoopback					__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0/*SPI*/);

/*!
	@const kSCNetworkInterfaceLoopback
	@discussion A network interface representing the loopback
		interface (lo0).
 */
extern const SCNetworkInterfaceRef kSCNetworkInterfaceLoopback					__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0/*SPI*/);

/*!
	@const kSCNetworkInterfaceTypeVPN
 */
extern const CFStringRef kSCNetworkInterfaceTypeVPN						__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0/*SPI*/);

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
							 void				*context)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

/*!
	@function _SCNetworkInterfaceCopyActive
	@discussion Creates an SCNetworkInterface and associated with interface name
		and SCDynamicStoreRef
	@param the interface name
	@param the SCDynamicStoreRef
	@result the SCNetworkInterface
 */
SCNetworkInterfaceRef
_SCNetworkInterfaceCopyActive				(SCDynamicStoreRef		store,
							 CFStringRef			bsdName)	__OSX_AVAILABLE_STARTING(__MAC_10_8,__IPHONE_5_0);

/*!
	@function _SCNetworkInterfaceCopyAllWithPreferences
		Returns all network capable interfaces on the system.
	@param prefs The "preferences" session.
	@result The list of interfaces on the system.
		You must release the returned value.
 */
CFArrayRef /* of SCNetworkInterfaceRef's */
_SCNetworkInterfaceCopyAllWithPreferences		(SCPreferencesRef		prefs)		__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_5_0/*SPI*/);

/*!
	@function _SCNetworkInterfaceCopyBTPANInterface
	@discussion Returns the SCNetworkInterface associated with the BT-PAN interface
	@result The BT-PAN interface; NULL if the interface is not (yet) known.
 */
SCNetworkInterfaceRef
_SCNetworkInterfaceCopyBTPANInterface			(void)						__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_NA);

/*!
	@function _SCNetworkInterfaceCopySlashDevPath
	@discussion Returns the /dev pathname for the interface.
	@param interface The network interface.
	@result The /dev pathname associated with the interface (e.g. "/dev/modem");
		NULL if no path is available.
 */
CFStringRef
_SCNetworkInterfaceCopySlashDevPath			(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_3_0);

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
							 UInt32				flags)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

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
							 SCNetworkServiceRef		service)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

/*!
	@function _SCNetworkInterfaceCreateWithIONetworkInterfaceObject
	@discussion Create a new network interface associated with the provided
		IORegistry "IONetworkInterface" object.
	@param if_obj The IONetworkInterface object.
	@result A reference to the new SCNetworkInterface.
		You must release the returned value.
 */
SCNetworkInterfaceRef
_SCNetworkInterfaceCreateWithIONetworkInterfaceObject	(io_object_t			if_obj)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

/*!
	@function SCNetworkInterfaceGetPrimaryRank
	@discussion We allow caller to retrieve the rank on an interface.
	@param the interface to get the rank
	@result SCNetworkServicePrimaryRank
 */
SCNetworkServicePrimaryRank
SCNetworkInterfaceGetPrimaryRank			(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_8,__IPHONE_5_0);

/*!
	@function SCNetworkInterfaceSetPrimaryRank
	@discussion We allow caller to set an assertion on an interface.
		The rank assertion lives as long as the SCNetworkInterfaceRef
		remains valid.
	@param the interface to set the rank assertion
	@param the new rank to be set
	@result TRUE if operation is successful; FALSE if an error was encountered.
 */
Boolean
SCNetworkInterfaceSetPrimaryRank			(SCNetworkInterfaceRef		interface,
							 SCNetworkServicePrimaryRank	newRank)	__OSX_AVAILABLE_STARTING(__MAC_10_8,__IPHONE_5_0);

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

/*!
	@function _SCNetworkInterfaceCopyInterfaceInfo
	@discussion Returns interface details
	@param interface The network interface.
	@result A dictionary with details about the network interface.
 */
CFDictionaryRef
_SCNetworkInterfaceCopyInterfaceInfo			(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_3_0);

/*!
	@function _SCNetworkInterfaceGetConfigurationAction
	@discussion Returns a user-notification / auto-configuration action for the interface.
	@param interface The network interface.
	@result The user-notification / auto-configuration action;
		NULL if the default action should be used.
 */
CFStringRef
_SCNetworkInterfaceGetConfigurationAction		(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_2_0);

/*!
	@function _SCNetworkInterfaceGetHardwareAddress
	@discussion Returns a link layer address for the interface.
	@param interface The network interface.
	@result The hardware (MAC) address for the interface.
		NULL if no hardware address is available.
 */
CFDataRef
_SCNetworkInterfaceGetHardwareAddress			(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

/*!
	@function _SCNetworkInterfaceGetIOInterfaceNamePrefix
	@discussion Returns the IOInterfaceNamePrefix for the interface.
	@param interface The network interface.
	@result The IOInterfaceNamePrefix associated with the interface;
		NULL if no IOInterfaceNamePrefix is available.
 */
CFStringRef
_SCNetworkInterfaceGetIOInterfaceNamePrefix		(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_8,__IPHONE_6_0);

/*!
	@function _SCNetworkInterfaceGetIOInterfaceType
	@discussion Returns the IOInterfaceType for the interface.
	@param interface The network interface.
	@result The IOInterfaceType associated with the interface
 */
CFNumberRef
_SCNetworkInterfaceGetIOInterfaceType			(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

/*!
	@function _SCNetworkInterfaceGetIOInterfaceUnit
	@discussion Returns the IOInterfaceUnit for the interface.
	@param interface The network interface.
	@result The IOInterfaceUnit associated with the interface;
		NULL if no IOInterfaceUnit is available.
 */
CFNumberRef
_SCNetworkInterfaceGetIOInterfaceUnit			(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

/*!
	@function _SCNetworkInterfaceGetIOPath
	@discussion Returns the IOPath for the interface.
	@param interface The network interface.
	@result The IOPath associated with the interface;
		NULL if no IOPath is available.
 */
CFStringRef
_SCNetworkInterfaceGetIOPath				(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

/*!
	@function _SCNetworkInterfaceGetIORegistryEntryID
	@discussion Returns the IORegistry entry ID for the interface.
	@param interface The network interface.
	@result The IORegistry entry ID associated with the interface;
		Zero if no entry ID is available.
 */
uint64_t
_SCNetworkInterfaceGetIORegistryEntryID			(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_8,__IPHONE_5_0);

/*!
	@function _SCNetworkInterfaceIsBluetoothPAN
	@discussion Identifies if a network interface is a Bluetooth PAN (GN) device.
	@param interface The network interface.
	@result TRUE if the interface is a Bluetooth PAN device.
 */
Boolean
_SCNetworkInterfaceIsBluetoothPAN			(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_3_0);

/*!
	@function _SCNetworkInterfaceIsBluetoothPAN_NAP
	@discussion Identifies if a network interface is a Bluetooth PAN-NAP device.
	@param interface The network interface.
	@result TRUE if the interface is a Bluetooth PAN-NAP device.
 */
Boolean
_SCNetworkInterfaceIsBluetoothPAN_NAP			(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_5_0);

/*!
	@function _SCNetworkInterfaceIsBluetoothP2P
	@discussion Identifies if a network interface is a Bluetooth P2P (PAN-U) device.
	@param interface The network interface.
	@result TRUE if the interface is a Bluetooth P2P device.
 */
Boolean
_SCNetworkInterfaceIsBluetoothP2P			(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_5_0);

/*!
	@function _SCNetworkInterfaceIsBuiltin
	@discussion Identifies if a network interface is "built-in".
	@param interface The network interface.
	@result TRUE if the interface is "built-in".
 */
Boolean
_SCNetworkInterfaceIsBuiltin				(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

/*!
	@function _SCNetworkInterfaceIsHiddenConfiguration
	@discussion Identifies if the configuration of a network interface should be
		hidden from any user interface (e.g. the "Network" pref pane).
	@param interface The network interface.
	@result TRUE if the interface configuration should be hidden.
 */
Boolean
_SCNetworkInterfaceIsHiddenConfiguration		(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0);

/*!
	@function _SCNetworkInterfaceIsModemV92
	@discussion Identifies if a modem network interface supports
		v.92 (hold).
	@param interface The network interface.
	@result TRUE if the interface is "v.92" modem.
 */
Boolean
_SCNetworkInterfaceIsModemV92				(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

/*!
	@function _SCNetworkInterfaceIsTethered
	@discussion Identifies if a network interface is an Apple tethered device (e.g. an iPhone).
	@param interface The network interface.
	@result TRUE if the interface is a tethered device.
 */
Boolean
_SCNetworkInterfaceIsTethered				(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_3_0);

/*!
	@function _SCNetworkInterfaceIsThunderbolt
	@discussion Identifies if a network interface is a Thunderbolt device
	@param interface The network interface.
	@result TRUE if the interface is a Thunderbolt device.
 */
Boolean
_SCNetworkInterfaceIsThunderbolt			(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

/*!
	@function _SCNetworkInterfaceIsPhysicalEthernet
	@discussion Indicates whether a network interface is a real ethernet interface i.e. one with an ethernet PHY.
	@param interface The network interface.
	@result TRUE if the interface is a real ethernet interface.
 */
Boolean
_SCNetworkInterfaceIsPhysicalEthernet			(SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_5_0);

/*!
	@function _SCNetworkInterfaceForceConfigurationRefresh
	@discussion Forces a configuration refresh of the
		specified interface.
	@param ifName Network interface name.
	@result TRUE if the refresh was successfully posted.
 */
Boolean
_SCNetworkInterfaceForceConfigurationRefresh		(CFStringRef			ifName)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

/*!
	@function SCNetworkInterfaceCopyCapability
	@discussion For the specified network interface, returns information
		about the currently requested capabilities, the active capabilities,
		and the capabilities which are available.
	@param interface The desired network interface.
	@param capability The desired capability.
	@result a CFTypeRef representing the current value of requested
		capability;
		NULL if the capability is not available for this
		interface or if an error was encountered.
		You must release the returned value.
 */
CFTypeRef
SCNetworkInterfaceCopyCapability			(SCNetworkInterfaceRef		interface,
							 CFStringRef			capability)	__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_5_0/*SPI*/);

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
							 CFTypeRef			newValue)	__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_5_0/*SPI*/);

#pragma mark -
#pragma mark SCBondInterface configuration (SPIs)

/*!
	@function _SCBondInterfaceCopyActive
	@discussion Returns all Ethernet Bond interfaces on the system.
	@result The list of SCBondInterface interfaces on the system.
		You must release the returned value.
 */
CFArrayRef
_SCBondInterfaceCopyActive				(void)						__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_4_0/*SPI*/);

/*!
	@function _SCBondInterfaceUpdateConfiguration
	@discussion Updates the bond interface configuration.
	@param prefs The "preferences" session.
	@result TRUE if the bond interface configuration was updated.; FALSE if the
		an error was encountered.
 */
Boolean
_SCBondInterfaceUpdateConfiguration			(SCPreferencesRef		prefs)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_4_0/*SPI*/);

/*!
	@function SCBondInterfaceGetMode
	@discussion Return the mode for the given bond interface.
	@param bond The bond interface to get the mode from.
	@result A CFNumberRef containing the mode (IF_BOND_MODE_{LACP,STATIC}).
 */
CFNumberRef
SCBondInterfaceGetMode					(SCBondInterfaceRef		bond)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_4_0/*SPI*/);

/*!
	@function SCBondInterfaceSetMode
	@discussion Set the mode on the bond interface.
	@param bond The bond interface on which to adjust the mode.
	@param mode The mode value (0=IF_BOND_MODE_LACP,1=IF_BOND_MODE_STATIC)
	@result TRUE if operation succeeded.
 */
Boolean
SCBondInterfaceSetMode					(SCBondInterfaceRef		bond,
							 CFNumberRef			mode)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_4_0/*SPI*/);

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
SCBridgeInterfaceCopyAll				(SCPreferencesRef		prefs)		__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0/*SPI*/);

/*!
	@function SCBridgeInterfaceCopyAvailableMemberInterfaces
	@discussion Returns all network capable devices on the system
		that can be added to an bridge interface.
	@param prefs The "preferences" session.
	@result The list of interfaces.
		You must release the returned value.
 */
CFArrayRef /* of SCNetworkInterfaceRef's */
SCBridgeInterfaceCopyAvailableMemberInterfaces		(SCPreferencesRef		prefs)		__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0/*SPI*/);

/*!
	@function SCBridgeInterfaceCreate
	@discussion Create a new SCBridgeInterface interface.
	@param prefs The "preferences" session.
	@result A reference to the new SCBridgeInterface.
		You must release the returned value.
 */
SCBridgeInterfaceRef
SCBridgeInterfaceCreate					(SCPreferencesRef		prefs)		__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0/*SPI*/);

/*!
	@function SCBridgeInterfaceRemove
	@discussion Removes the SCBridgeInterface from the configuration.
	@param bridge The SCBridgeInterface interface.
	@result TRUE if the interface was removed; FALSE if an error was encountered.
 */
Boolean
SCBridgeInterfaceRemove					(SCBridgeInterfaceRef		bridge)		__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0/*SPI*/);

/*!
	@function SCBridgeInterfaceGetMemberInterfaces
	@discussion Returns the member interfaces for the specified bridge interface.
	@param bridge The SCBridgeInterface interface.
	@result The list of interfaces.
 */
CFArrayRef /* of SCNetworkInterfaceRef's */
SCBridgeInterfaceGetMemberInterfaces			(SCBridgeInterfaceRef		bridge)		__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0/*SPI*/);

/*!
	@function SCBridgeInterfaceGetOptions
	@discussion Returns the configuration settings associated with a bridge interface.
	@param bridge The SCBridgeInterface interface.
	@result The configuration settings associated with the bridge interface;
		NULL if no changes to the default configuration have been saved.
 */
CFDictionaryRef
SCBridgeInterfaceGetOptions				(SCBridgeInterfaceRef		bridge)		__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0/*SPI*/);

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
													__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0/*SPI*/);

/*!
	@function SCBridgeInterfaceSetLocalizedDisplayName
	@discussion Sets the localized display name for the specified bridge interface.
	@param bridge The SCBridgeInterface interface.
	@param newName The new display name.
	@result TRUE if the configuration was stored; FALSE if an error was encountered.
 */
Boolean
SCBridgeInterfaceSetLocalizedDisplayName		(SCBridgeInterfaceRef		bridge,
							 CFStringRef			newName)	__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0/*SPI*/);

/*!
	@function SCBridgeInterfaceSetOptions
	@discussion Sets the configuration settings for the specified bridge interface.
	@param bridge The SCBridgeInterface interface.
	@param newOptions The new configuration settings.
	@result TRUE if the configuration was stored; FALSE if an error was encountered.
 */
Boolean
SCBridgeInterfaceSetOptions				(SCBridgeInterfaceRef		bridge,
							 CFDictionaryRef		newOptions)	__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0/*SPI*/);

#pragma mark -

/*!
	@function _SCBridgeInterfaceCopyActive
	@discussion Returns all bridge interfaces on the system.
	@result The list of SCBridgeInterface interfaces on the system.
		You must release the returned value.
 */
CFArrayRef
_SCBridgeInterfaceCopyActive				(void)						__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0/*SPI*/);

/*!
	@function _SCBridgeInterfaceUpdateConfiguration
	@discussion Updates the bridge interface configuration.
	@param prefs The "preferences" session.
	@result TRUE if the bridge interface configuration was updated.; FALSE if the
		an error was encountered.
 */
Boolean
_SCBridgeInterfaceUpdateConfiguration			(SCPreferencesRef		prefs)		__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0/*SPI*/);


#pragma mark -
#pragma mark SCVLANInterface configuration (SPIs)

/*!
	@function _SCVLANInterfaceCopyActive
	@discussion Returns all VLAN interfaces on the system.
	@result The list of SCVLANInterface interfaces on the system.
		You must release the returned value.
 */
CFArrayRef
_SCVLANInterfaceCopyActive				(void)						__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_4_0/*SPI*/);

/*!
	@function _SCVLANInterfaceUpdateConfiguration
	@discussion Updates the VLAN interface configuration.
	@param prefs The "preferences" session.
	@result TRUE if the VLAN interface configuration was updated.; FALSE if the
		an error was encountered.
 */
Boolean
_SCVLANInterfaceUpdateConfiguration			(SCPreferencesRef		prefs)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_4_0/*SPI*/);


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
							 SCNetworkInterfacePasswordType	passwordType)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

CFDataRef
SCNetworkInterfaceCopyPassword				(SCNetworkInterfaceRef		interface,
							 SCNetworkInterfacePasswordType	passwordType)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

Boolean
SCNetworkInterfaceRemovePassword			(SCNetworkInterfaceRef		interface,
							 SCNetworkInterfacePasswordType	passwordType)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

Boolean
SCNetworkInterfaceSetPassword				(SCNetworkInterfaceRef		interface,
							 SCNetworkInterfacePasswordType	passwordType,
							 CFDataRef			password,
							 CFDictionaryRef		options)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);


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
							 void				*context)	__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0);

/*!
	@function _SCNetworkServiceCopyActive
	@discussion Returns the network service with the specified identifier.

	Note: The service returned by this SPI differs from the SCNetworkServiceCopy
	      API in that queries and operations interact with the "active" service
	      represented in the SCDynamicStore.  Only a limited subset of the
	      SCNetworkService APIs are supported.
	@param prefs The dynamic store session.
	@param serviceID The unique identifier for the service.
	@result A reference to the SCNetworkService represented in the SCDynamicStore;
		NULL if the serviceID does not exist in the SCDynamicStore or if an
		error was encountered.
		You must release the returned value.
 */
SCNetworkServiceRef
_SCNetworkServiceCopyActive				(SCDynamicStoreRef		store,
							 CFStringRef			serviceID)	__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_2_1);

/*!
	@function SCNetworkServiceGetPrimaryRank
	@discussion Returns the primary service rank associated with a service.
	@param service The network service.
	@result The primary service rank associated with the specified application;
		kSCNetworkServicePrimaryRankDefault if no rank is associated with the
		application or an error was encountered.
 */
SCNetworkServicePrimaryRank
SCNetworkServiceGetPrimaryRank				(SCNetworkServiceRef		service)	__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_2_0);

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
							 SCNetworkServicePrimaryRank	newRank)	__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_2_0);

/*!
	@function _SCNetworkServiceIsVPN
	@discussion Identifies a VPN service.
	@param service The network service.
	@result TRUE if the service is a VPN.
 */
Boolean
_SCNetworkServiceIsVPN					(SCNetworkServiceRef		service)	__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0);

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
							 CFStringRef			newServiceID)	__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0);

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
	@function SCNetworkSetCopyAvailableInterfaces
	@discussion Returns all available interfaces for the set.
		The interfaces excludes those of bond and bridge members.
	@param set The network set.
	@result The list of SCNetworkInterfaces.
		You must release the returned value.
 */
CFArrayRef
SCNetworkSetCopyAvailableInterfaces			(SCNetworkSetRef		set)		__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0/*SPI*/);

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
SCNetworkSetEstablishDefaultConfiguration		(SCNetworkSetRef		set)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

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
							 SCNetworkInterfaceRef		interface)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

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
SCNetworkSetCopySelectedVPNService			(SCNetworkSetRef		set)		__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0);

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
							 SCNetworkServiceRef		service)	__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0);

Boolean
_SCNetworkSetSetSetID					(SCNetworkSetRef		set,
							 CFStringRef			setID)		__OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_8_0);

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
						 CFStringRef identifier)		__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0/*SPI*/);

/*!
	@function VPNServiceCopyAll
	@discussion Copy all VPN services.
	@param prefs A reference to the prefs where the VPN services are stored.
	@result An array containing VPNServiceRefs for all the VPN services.
 */
CFArrayRef
VPNServiceCopyAll				(SCPreferencesRef		prefs)	__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0/*SPI*/);

/*!
	@function VPNServiceCopyAppRuleIDs
	@discussion Copy all the app rule identifiers for a VPN service.
	@param service A reference to the VPN service.
	@result An array of CFStringRefs, each string containing the identifier of a app rule in the given service.
 */
CFArrayRef
VPNServiceCopyAppRuleIDs			(VPNServiceRef			service)	__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0/*SPI*/);

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
						 CFDictionaryRef		ruleSettings)	__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0/*SPI*/);

/*!
	@function VPNServiceCopyAppRule
	@discussion Copy the settings for a app rule in a VPN service.
	@param service The app tunnel service.
	@param ruleIdentifier The ID of the app rule.
	@result The rule settings, or NULL if the app rule could not be found.
 */
CFDictionaryRef
VPNServiceCopyAppRule				(VPNServiceRef			service,
						 CFStringRef			ruleIdentifier)	__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0/*SPI*/);

/*!
	@function VPNServiceRemoveAppRule
	@discussion Remove an app rule from a VPN service.
	@param service The VPN service.
	@param ruleIdentifier The ID of the app rule to remove.
	@result Returns TRUE if the app rule was removed successfully; FALSE otherwise.
 */
Boolean
VPNServiceRemoveAppRule				(VPNServiceRef			service,
						 CFStringRef			ruleIdentifier)	__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0/*SPI*/);

/*!
	@function VPNServiceIsManagedAppVPN
	@discussion Check to see if a VPN service is a managed App VPN service
	@param service The VPN servie.
	@result Returns TRUE if the service is a managed App VPN service; FALSE otherwise.
*/
Boolean
VPNServiceIsManagedAppVPN			(VPNServiceRef			service)	__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0/*SPI*/);

/*!
	@group	Migration SPI
 */
#pragma mark -
#pragma mark Migration SPI

extern const CFStringRef kSCNetworkConfigurationRepair			/* CFBoolean */		__OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_8_0);

extern const CFStringRef kSCNetworkConfigurationMigrationActionKey	/* CFNumber */		__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0);

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
_SCNetworkConfigurationCopyMigrationPaths(CFDictionaryRef options)				__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0);

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
_SCNetworkConfigurationPerformMigration(CFURLRef sourceDir,
					CFURLRef currentDir,
					CFURLRef targetDir,
					CFDictionaryRef options)				__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0);


/*!
	@function _SCNetworkConfigurationCheckValidity
	@discussion Verifies whether the configuration files present in the specified
		directory have valid mappings or not
	@param configDir A reference which points to the directory where the configuration
		files are present
	@result TRUE if valid configurations are found

 */

Boolean
_SCNetworkConfigurationCheckValidity(CFURLRef configDir,
				     CFDictionaryRef options)					__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0);


/*!
 @function _SCNetworkMigrationAreConfigurationsIdentical
 @discussion Compares the migration output between network configurations
		with the expected output.
 @param configPref Preferences pointing toward preferences.plist file to
		be compared with expected file.
 @param configNetworkInterfacePref Preferences pointing toward NetworkInterfaces.plist
		file to be compared with expected file.
 @param expectedConfigPref Preferences pointing toward preferences.plist file
		which is the expected result.
 @param expectedNetworkInterfacePref Preferences pointing toward NetworkInterfaces.plist
		file which is the expected file.
 @result TRUE if configurations match with the expected configurations

 */

Boolean
_SCNetworkMigrationAreConfigurationsIdentical (CFURLRef configurationURL,
					       CFURLRef expectedConfigurationURL)
							__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0);

/*!
 @function	_SCNetworkConfigurationCopyMigrationRemovePaths
 @discussion	List of paths to files which we want to be removed from the target filesystem after migration
 @param targetPaths	the CFArray returned by _SCNetworkConfigurationPerformMigration
 @param targetDir	the CFURL passed to _SCNetworkConfigurationPerformMigration
 @result	An array of CFURL's; NULL if no paths need to be removed from the target filesystem
 
*/

CFArrayRef	// of CFURLRef's
_SCNetworkConfigurationCopyMigrationRemovePaths	(CFArrayRef	targetPaths,
                                                 CFURLRef	targetDir)				__OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_8_0);

__END_DECLS
#endif	/* _SCNETWORKCONFIGURATIONPRIVATE_H */

/*
 * Copyright (c) 2000-2002, 2004, 2005, 2008 Apple Inc. All rights reserved.
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

#ifndef _SCDYNAMICSTOREKEY_H
#ifdef	USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS
#include <SystemConfiguration/_SCDynamicStoreKey.h>
#else	/* USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS */
#define _SCDYNAMICSTOREKEY_H

#include <Availability.h>
#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>


/*!
	@header SCDynamicStoreKey
	@discussion The SCDynamicStoreKey API provides convenience functions
		that an application can use to create a correctly formatted
		dynamic store key for accessing specific items in the dynamic
		store.  An application can then use the resulting string in
		any function that requires a dynamic store key.
 */


__BEGIN_DECLS

/*
 * SCDynamicStoreKeyCreate
 * - convenience routines that create a CFString key for an item in the store
 */

/*!
	@function SCDynamicStoreKeyCreate
	@discussion Creates a dynamic store key using the given format.
	@param allocator The CFAllocator that should be used to allocate
		memory for this key.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@param fmt A CFStringRef describing the format for this key.
	@result Returns a string containing the formatted key.
 */
CFStringRef
SCDynamicStoreKeyCreate				(
						CFAllocatorRef	allocator,
						CFStringRef	fmt,
						...
						)		__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreKeyCreateNetworkGlobalEntity
	@discussion Creates a dynamic store key that can be used to access
		a specific global (as opposed to a per-service or per-interface)
		network configuration entity.
	@param allocator The CFAllocator that should be used to allocate
		memory for this key.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@param domain A string specifying the desired domain, such as the
		requested configuration (kSCDynamicStoreDomainSetup) or the
		actual state (kSCDynamicStoreDomainState).
	@param entity A string containing the specific global entity, such
		as IPv4 (kSCEntNetIPv4) or DNS (kSCEntNetDNS).
	@result Returns a string containing the formatted key.

 */
CFStringRef
SCDynamicStoreKeyCreateNetworkGlobalEntity	(
						CFAllocatorRef	allocator,
						CFStringRef	domain,
						CFStringRef	entity
						)		__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreKeyCreateNetworkInterface
	@discussion Creates a dynamic store key that can be used to access
		the network interface configuration information stored in
		the dynamic store.
	@param allocator The CFAllocator that should be used to allocate
		memory for this key.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@param domain A string specifying the desired domain, such as the
		requested configuration (kSCDynamicStoreDomainSetup) or the
		actual state (kSCDynamicStoreDomainState).
	@result Returns a string containing the formatted key.

 */
CFStringRef
SCDynamicStoreKeyCreateNetworkInterface		(
						CFAllocatorRef	allocator,
						CFStringRef	domain
						)		__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreKeyCreateNetworkInterfaceEntity
	@discussion Creates a dynamic store key that can be used to access
		the per-interface network configuration information stored in
		the dynamic store.
	@param allocator The CFAllocator that should be used to allocate
		memory for this key.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@param domain A string specifying the desired domain, such as the
		requested configuration (kSCDynamicStoreDomainSetup) or the
		actual state (kSCDynamicStoreDomainState).
	@param ifname A string containing the interface name or a regular
		expression pattern.
	@param entity A string containing the specific global entity, such
		as IPv4 (kSCEntNetIPv4) or DNS (kSCEntNetDNS).
	@result Returns a string containing the formatted key.

 */
CFStringRef
SCDynamicStoreKeyCreateNetworkInterfaceEntity	(
						CFAllocatorRef	allocator,
						CFStringRef	domain,
						CFStringRef	ifname,
						CFStringRef	entity
						)		__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreKeyCreateNetworkServiceEntity
	@discussion Creates a dynamic store key that can be used to access
		the per-service network configuration information stored in
		the dynamic store.
	@param allocator The CFAllocator that should be used to allocate
		memory for this key.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@param domain A string specifying the desired domain, such as the
		requested configuration (kSCDynamicStoreDomainSetup) or the
		actual state (kSCDynamicStoreDomainState).
	@param serviceID A string containing the service ID or a regular
		expression pattern.
	@param entity A string containing the specific global entity, such
		as IPv4 (kSCEntNetIPv4) or DNS (kSCEntNetDNS).
	@result Returns a string containing the formatted key.


 */
CFStringRef
SCDynamicStoreKeyCreateNetworkServiceEntity	(
						CFAllocatorRef	allocator,
						CFStringRef	domain,
						CFStringRef	serviceID,
						CFStringRef	entity
						)		__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreKeyCreateComputerName
	@discussion Creates a key that can be used in conjuntion with
		SCDynamicStoreSetNotificationKeys function to receive
		notifications when the current computer name changes.
	@param allocator The CFAllocator that should be used to allocate
		memory for this key.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@result Returns a notification string for the current computer or
		host name.
*/
CFStringRef
SCDynamicStoreKeyCreateComputerName		(
						CFAllocatorRef		allocator
						)		__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreKeyCreateConsoleUser
	@discussion Creates a key that can be used in conjunction with
		SCDynamicStoreSetNotificationKeys function to receive
		notifications when the current console user changes.
	@param allocator The CFAllocator that should be used to allocate
		memory for this key.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@result Returns a notification string for the current console user.
*/
CFStringRef
SCDynamicStoreKeyCreateConsoleUser		(
						CFAllocatorRef		allocator
						)		__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_NA);

/*!
	@function SCDynamicStoreKeyCreateHostNames
	@discussion Creates a key that can be used in conjunction with the
		SCDynamicStoreSetNotificationKeys function to receive
		notifications when the HostNames entity changes.  The
		HostNames entity includes the local host name.
	@param allocator The CFAllocator that should be used to allocate
		memory for this key.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@result Returns a notification string for the HostNames entity.
*/
CFStringRef
SCDynamicStoreKeyCreateHostNames		(
						CFAllocatorRef		allocator
						)		__OSX_AVAILABLE_STARTING(__MAC_10_2,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreKeyCreateLocation
	@discussion Creates a key that can be used in conjunction with the
		SCDynamicStoreSetNotificationKeys function to receive
		notifications when the location identifier changes.
	@param allocator The CFAllocator that should be used to allocate
		memory for this key.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@result Returns a notification string for the current location
		identifier.
*/
CFStringRef
SCDynamicStoreKeyCreateLocation			(
						CFAllocatorRef		allocator
						)		__OSX_AVAILABLE_STARTING(__MAC_10_2,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreKeyCreateProxies
	@discussion Creates a key that can be used in conjunction with
		the SCDynamicStoreSetNotificationKeys function to receive
		notifications when the current network proxy settings
		(such as HTTP or FTP) are changed.
	@param allocator The CFAllocator that should be used to allocate
		memory for this key.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@result Returns a notification string for the current proxy settings.
*/
CFStringRef
SCDynamicStoreKeyCreateProxies			(
						CFAllocatorRef		allocator
						)		__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

__END_DECLS

#endif	/* USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS */
#endif	/* _SCDYNAMICSTOREKEY_H */

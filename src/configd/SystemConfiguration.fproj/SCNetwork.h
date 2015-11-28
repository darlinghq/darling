/*
 * Copyright (c) 2000, 2001, 2003-2009 Apple Inc. All rights reserved.
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

#ifndef _SCNETWORK_H
#define _SCNETWORK_H

#include <Availability.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <CoreFoundation/CoreFoundation.h>


/*!
	@header SCNetwork
	@discussion The SCNetwork API contains functions an application can
		use to determine remote host reachability and notify the
		system of configuration changes.

		The two SCNetworkCheckReachability functions allow an
		application to determine the status of the system's current
		network configuration and the reachability of a target host
		or address.

		"Reachability" reflects whether a data packet, sent by an
		application into the network stack, can leave the local
		computer.  Note that reachability does <i>not</i> guarantee
		that the data packet will actually be received by the host.
 */

/*!
	@enum SCNetworkConnectionFlags
	@discussion Flags that indicate whether the specified network
		nodename or address is reachable, whether a connection is
		required, and whether some user intervention may be required
		when establishing a connection.

		Note: the SCNetworkConnection flags have been deprecated
		in favor of the newer SCNetworkReachability flags defined
		in SCNetworkReachability.h.
	@constant kSCNetworkFlagsTransientConnection
		This flag indicates that the specified nodename or address can
		be reached via a transient connection, such as PPP.
	@constant kSCNetworkFlagsReachable
		This flag indicates that the specified nodename or address can
		be reached using the current network configuration.
	@constant kSCNetworkFlagsConnectionRequired
		This flag indicates that the specified nodename or address can
		be reached using the current network configuration, but a
		connection must first be established.

		As an example, this status would be returned for a dialup
		connection that was not currently active, but could handle
		network traffic for the target system.
	@constant kSCNetworkFlagsConnectionAutomatic
		This flag indicates that the specified nodename or address can
		be reached using the current network configuration, but a
		connection must first be established.  Any traffic directed
		to the specified name or address will initiate the connection.
	@constant kSCNetworkFlagsInterventionRequired
		This flag indicates that the specified nodename or address can
		be reached using the current network configuration, but a
		connection must first be established.  In addition, some
		form of user intervention will be required to establish this
		connection, such as providing a password, an authentication
		token, etc.

		Note: At the present time, this flag will only be returned
		in the case where you have a dial-on-traffic configuration
		(ConnectionAutomatic), where an attempt to connect has
		already been made, and where some error (e.g. no dial tone,
		no answer, bad password, ...) was encountered during the
		automatic connection attempt.  In this case the PPP controller
		will stop attempting to establish a connection until the user
		has intervened.
	@constant kSCNetworkFlagsIsLocalAddress
		This flag indicates that the specified nodename or address
		is one associated with a network interface on the current
		system.
	@constant kSCNetworkFlagsIsDirect
		This flag indicates that network traffic to the specified
		nodename or address will not go through a gateway, but is
		routed directly to one of the interfaces in the system.
 */
enum {
	kSCNetworkFlagsTransientConnection	= 1<<0,
	kSCNetworkFlagsReachable		= 1<<1,
	kSCNetworkFlagsConnectionRequired	= 1<<2,
	kSCNetworkFlagsConnectionAutomatic	= 1<<3,
	kSCNetworkFlagsInterventionRequired	= 1<<4,
	kSCNetworkFlagsIsLocalAddress		= 1<<16,
	kSCNetworkFlagsIsDirect			= 1<<17,
};
typedef	uint32_t	SCNetworkConnectionFlags;

__BEGIN_DECLS

/*!
	@function SCNetworkCheckReachabilityByAddress
	@discussion Determines if the given network address is
		reachable using the current network configuration.

		Note: this API has been deprecated but you can
		      get equivalent results with :
<pre>
	SCNetworkReachabiltyRef   target;
	SCNetworkReachabiltyFlags flags = 0;
	Boolean                   ok;

	target = SCNetworkReachabilityCreateWithAddress(NULL, address);
	ok = SCNetworkReachabilityGetFlags(target, &flags);
	CFRelease(target);
</pre>
	@param address The network address of the desired host.
	@param addrlen The length, in bytes, of the address.
	@param flags A pointer to memory that will be filled with a
		set of SCNetworkConnectionFlags detailing the reachability
		of the specified address.
	@result Returns TRUE if the network connection flags are valid;
		FALSE if the status could not be determined.
 */
Boolean
SCNetworkCheckReachabilityByAddress	(
					const struct sockaddr		*address,
					socklen_t			addrlen,
					SCNetworkConnectionFlags	*flags
					)				__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_1,__MAC_10_6,__IPHONE_NA,__IPHONE_NA);

/*!
	@function SCNetworkCheckReachabilityByName
	@discussion Determines if the given network host or node name is
		reachable using the current network configuration.

		Note: this API has been deprecated but you can
		      get equivalent results with :
<pre>
	SCNetworkReachabilityRef   target;
	SCNetworkReachabilityFlags flags = 0;
	Boolean                   ok;

	target = SCNetworkReachabilityCreateWithName(NULL, name);
	ok = SCNetworkReachabilityGetFlags(target, &flags);
	CFRelease(target);
</pre>
	@param nodename The node name of the desired host. This name would
		be the same as that passed to the gethostbyname(3) or
		getaddrinfo(3) functions.
	@param flags A pointer to memory that will be filled with a
		set of SCNetworkConnectionFlags detailing the reachability
		of the specified node name.
	@result Returns TRUE if the network connection flags are valid;
		FALSE if the status could not be determined.
 */
Boolean
SCNetworkCheckReachabilityByName	(
					const char			*nodename,
					SCNetworkConnectionFlags	*flags
					)				__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_1,__MAC_10_6,__IPHONE_NA,__IPHONE_NA);

/*!
	@function SCNetworkInterfaceRefreshConfiguration
	@discussion Sends a notification to interested configuration agents
		to have them immediately retry their configuration over a
		particular network interface.

		Note: This function must be invoked by root (uid == 0).
	@param ifName The BSD name of the network interface, such as
		CFSTR("en0").
	@result Returns TRUE if the notification was sent; FALSE otherwise.
	@deprecated in version 10.4. Replaced with SCNetworkInterfaceForceConfigurationRefresh.
 */
Boolean
SCNetworkInterfaceRefreshConfiguration	(
					CFStringRef			ifName
					)				__OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_1,__MAC_10_5,__IPHONE_NA,__IPHONE_NA);

__END_DECLS

#endif /* _SCNETWORK_H */

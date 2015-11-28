/*
 * Copyright (c) 2003-2005, 2008-2010 Apple Inc. All rights reserved.
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

#ifndef _SCNETWORKREACHABILITY_H
#define _SCNETWORKREACHABILITY_H

#include <Availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCNetwork.h>
#include <dispatch/dispatch.h>

/*!
	@header SCNetworkReachability
	@discussion The SCNetworkReachability API allows an application to
		determine the status of a system's current network
		configuration and the reachability of a target host.
		In addition, reachability can be monitored with notifications
		that are sent when the status has changed.

		"Reachability" reflects whether a data packet, sent by
		an application into the network stack, can leave the local
		computer.
		Note that reachability does <i>not</i> guarantee that the data
		packet will actually be received by the host.
 */

/*!
	@typedef SCNetworkReachabilityRef
	@discussion This is the handle to a network address or name.
 */
typedef const struct __SCNetworkReachability * SCNetworkReachabilityRef;


/*!
	@typedef SCNetworkReachabilityContext
	Structure containing user-specified data and callbacks for SCNetworkReachability.
	@field version The version number of the structure type being passed
		in as a parameter to the SCDynamicStore creation function.
		This structure is version 0.
	@field info A C pointer to a user-specified block of data.
	@field retain The callback used to add a retain for the info field.
		If this parameter is not a pointer to a function of the correct
		prototype, the behavior is undefined.  The value may be NULL.
	@field release The calllback used to remove a retain previously added
		for the info field.  If this parameter is not a pointer to a
		function of the correct prototype, the behavior is undefined.
		The value may be NULL.
	@field copyDescription The callback used to provide a description of
		the info field.
 */
typedef struct {
	CFIndex		version;
	void *		info;
	const void	*(*retain)(const void *info);
	void		(*release)(const void *info);
	CFStringRef	(*copyDescription)(const void *info);
} SCNetworkReachabilityContext;

/*!
	@enum SCNetworkReachabilityFlags
	@discussion Flags that indicate whether the specified network
		nodename or address is reachable, whether a connection is
		required, and whether some user intervention may be required
		when establishing a connection.
	@constant kSCNetworkReachabilityFlagsTransientConnection
		This flag indicates that the specified nodename or address can
		be reached via a transient connection, such as PPP.
	@constant kSCNetworkReachabilityFlagsReachable
		This flag indicates that the specified nodename or address can
		be reached using the current network configuration.
	@constant kSCNetworkReachabilityFlagsConnectionRequired
		This flag indicates that the specified nodename or address can
		be reached using the current network configuration, but a
		connection must first be established.

		As an example, this status would be returned for a dialup
		connection that was not currently active, but could handle
		network traffic for the target system.
	@constant kSCNetworkReachabilityFlagsConnectionOnTraffic
		This flag indicates that the specified nodename or address can
		be reached using the current network configuration, but a
		connection must first be established.  Any traffic directed
		to the specified name or address will initiate the connection.

		Note: this flag was previously named kSCNetworkReachabilityFlagsConnectionAutomatic
	@constant kSCNetworkReachabilityFlagsInterventionRequired
		This flag indicates that the specified nodename or address can
		be reached using the current network configuration, but a
		connection must first be established.  In addition, some
		form of user intervention will be required to establish this
		connection, such as providing a password, an authentication
		token, etc.

		Note: At the present time, this flag will only be returned
		in the case where you have a dial-on-traffic configuration
		(ConnectionOnTraffic), where an attempt to connect has
		already been made, and where some error (e.g. no dial tone,
		no answer, bad password, ...) was encountered during the
		automatic connection attempt.  In this case the PPP controller
		will stop attempting to establish a connection until the user
		has intervened.
	@constant kSCNetworkReachabilityFlagsConnectionOnDemand
		This flag indicates that the specified nodename or address can
		be reached using the current network configuration, but a
		connection must first be established.
		The connection will be established "On Demand" by the
		CFSocketStream APIs.
		Other APIs will not establish the connection.
	@constant kSCNetworkReachabilityFlagsIsLocalAddress
		This flag indicates that the specified nodename or address
		is one associated with a network interface on the current
		system.
	@constant kSCNetworkReachabilityFlagsIsDirect
		This flag indicates that network traffic to the specified
		nodename or address will not go through a gateway, but is
		routed directly to one of the interfaces in the system.
#if	TARGET_OS_IPHONE
	@constant kSCNetworkReachabilityFlagsIsWWAN
		This flag indicates that the specified nodename or address can
		be reached via an EDGE, GPRS, or other "cell" connection.
#endif	// TARGET_OS_IPHONE
 */
enum {
	kSCNetworkReachabilityFlagsTransientConnection	= 1<<0,
	kSCNetworkReachabilityFlagsReachable		= 1<<1,
	kSCNetworkReachabilityFlagsConnectionRequired	= 1<<2,
	kSCNetworkReachabilityFlagsConnectionOnTraffic	= 1<<3,
	kSCNetworkReachabilityFlagsInterventionRequired	= 1<<4,
	kSCNetworkReachabilityFlagsConnectionOnDemand	= 1<<5,	// __OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_3_0)
	kSCNetworkReachabilityFlagsIsLocalAddress	= 1<<16,
	kSCNetworkReachabilityFlagsIsDirect		= 1<<17,
#if	TARGET_OS_IPHONE
	kSCNetworkReachabilityFlagsIsWWAN		= 1<<18,
#endif	// TARGET_OS_IPHONE

	kSCNetworkReachabilityFlagsConnectionAutomatic	= kSCNetworkReachabilityFlagsConnectionOnTraffic
};
typedef	uint32_t	SCNetworkReachabilityFlags;

/*!
	@typedef SCNetworkReachabilityCallBack
	@discussion Type of the callback function used when the
		reachability of a network address or name changes.
	@param target The SCNetworkReachability reference being monitored
		for changes.
	@param flags The new SCNetworkReachabilityFlags representing the
		reachability status of the network address/name.
	@param info A C pointer to a user-specified block of data.
 */
typedef void (*SCNetworkReachabilityCallBack)	(
						SCNetworkReachabilityRef	target,
						SCNetworkReachabilityFlags	flags,
						void				*info
						);

__BEGIN_DECLS

/*!
	@function SCNetworkReachabilityCreateWithAddress
	@discussion Creates a reference to the specified network
		address.  This reference can be used later to monitor the
		reachability of the target host.
	@param address The address of the desired host.
	@result Returns a reference to the new immutable SCNetworkReachabilityRef.

		 You must release the returned value.
 */
SCNetworkReachabilityRef
SCNetworkReachabilityCreateWithAddress		(
						CFAllocatorRef			allocator,
						const struct sockaddr		*address
						)				__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0);

/*!
	@function SCNetworkReachabilityCreateWithAddressPair
	@discussion Creates a reference to the specified network
		address.  This reference can be used later to monitor the
		reachability of the target host.
	@param localAddress The local address associated with a network
		connection.  If NULL, only the remote address is of interest.
	@param remoteAddress The remote address associated with a network
		connection.  If NULL, only the local address is of interest.
	@result Returns a reference to the new immutable SCNetworkReachabilityRef.

		 You must release the returned value.
 */
SCNetworkReachabilityRef
SCNetworkReachabilityCreateWithAddressPair	(
						CFAllocatorRef			allocator,
						const struct sockaddr		*localAddress,
						const struct sockaddr		*remoteAddress
						)				__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0);

/*!
	@function SCNetworkReachabilityCreateWithName
	@discussion Creates a reference to the specified network host or node
		name.  This reference can be used later to monitor the
		reachability of the target host.
	@param nodename The node name of the desired host.
		This name would be the same as that passed to the
		gethostbyname(3) or getaddrinfo(3) functions.
	@result Returns a reference to the new immutable SCNetworkReachabilityRef.

		You must release the returned value.
 */
SCNetworkReachabilityRef
SCNetworkReachabilityCreateWithName		(
						CFAllocatorRef			allocator,
						const char			*nodename
						)				__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0);

/*!
	@function SCNetworkReachabilityGetTypeID
	@discussion Returns the type identifier of all SCNetworkReachability
		instances.
 */
CFTypeID
SCNetworkReachabilityGetTypeID			(void)				__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0);


/*!
	@function SCNetworkReachabilityGetFlags
	@discussion Determines if the given target is reachable using the
		current network configuration.
	@param target The network reference associated with the address or name
		to be checked for reachability.
	@param flags A pointer to memory that will be filled with the
		SCNetworkReachabilityFlags detailing the reachability
		of the specified target.
	@result Returns TRUE if the network connection flags are valid;
		FALSE if the status could not be determined.
 */
Boolean
SCNetworkReachabilityGetFlags			(
						SCNetworkReachabilityRef	target,
						SCNetworkReachabilityFlags	*flags
						)				__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0);

/*!
	@function SCNetworkReachabilitySetCallback
	@discussion Assigns a client to a target, which receives callbacks
		when the reachability of the target changes.
	@param target The network reference associated with the address or
		name to be checked for reachability.
	@param callout The function to be called when the reachability of the
		target changes.  If NULL, the current client for the target
		is removed.
	@param context The SCNetworkReachabilityContext associated with
		the callout.  The value may be NULL.
	@result Returns TRUE if the notification client was successfully set.
 */
Boolean
SCNetworkReachabilitySetCallback		(
						SCNetworkReachabilityRef	target,
						SCNetworkReachabilityCallBack	callout,
						SCNetworkReachabilityContext	*context
						)				__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0);

/*!
	@function SCNetworkReachabilityScheduleWithRunLoop
	@discussion Schedules the given target with the given run loop and mode.
	@param target The address or name that is set up for asynchronous
		notifications.  Must be non-NULL.
	@param runLoop A reference to a run loop on which the target should
		be scheduled.  Must be non-NULL.
	@param runLoopMode The mode on which to schedule the target.
		Must be non-NULL.
	@result Returns TRUE if the target is scheduled successfully;
		FALSE otherwise.
 */
Boolean
SCNetworkReachabilityScheduleWithRunLoop	(
						SCNetworkReachabilityRef	target,
						CFRunLoopRef			runLoop,
						CFStringRef			runLoopMode
						)				__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0);

/*!
	@function SCNetworkReachabilityUnscheduleFromRunLoop
	@discussion Unschedules the given target from the given run loop
		and mode.
	@param target The address or name that is set up for asynchronous
		notifications.  Must be non-NULL.
	@param runLoop A reference to a run loop from which the target
		should be unscheduled.  Must be non-NULL.
	@param runLoopMode The mode on which to unschedule the target.
		Must be non-NULL.
	@result Returns TRUE if the target is unscheduled successfully;
		FALSE otherwise.
 */
Boolean
SCNetworkReachabilityUnscheduleFromRunLoop	(
						SCNetworkReachabilityRef	target,
						CFRunLoopRef			runLoop,
						CFStringRef			runLoopMode
						)				__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0);

/*!
	@function SCNetworkReachabilitySetDispatchQueue
	@discussion Schedules callbacks for the given target on the given
		dispatch queue.
	@param target The address or name that is set up for asynchronous
		notifications.  Must be non-NULL.
	@param queue A libdispatch queue to run the callback on. Pass NULL to disable notifications, and release queue.
	@result Returns TRUE if the target is unscheduled successfully;
		FALSE otherwise.
 */
Boolean
SCNetworkReachabilitySetDispatchQueue		(
						SCNetworkReachabilityRef	target,
						dispatch_queue_t		queue
						)				__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0);

__END_DECLS

#endif /* _SCNETWORKREACHABILITY_H */

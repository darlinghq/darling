/*
 * Copyright (c) 2003-2006, 2008-2010 Apple Inc. All rights reserved.
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

#ifndef _SCNETWORKCONNECTION_H
#ifdef	USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS
#include <SystemConfiguration/_SCNetworkConnection.h>
#else	/* USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS */
#define _SCNETWORKCONNECTION_H

#include <Availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <dispatch/dispatch.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>


/*!
	@header SCNetworkConnection
	@discussion The SCNetworkConnection API contains functions that allow
		an application to control connection-oriented services defined
		in the system and get connection-status information.

		The functions in the SCNetworkConnection API allow you to
		control and get information about existing services only.
		If you need to create, change, or remove services, you
		should use the SCNetworkConfiguration API instead.

		Note: Currently, only PPP services can be controlled.
 */


/*!
	@typedef SCNetworkConnectionRef
	@discussion This is the handle to manage a connection-oriented service.
 */
typedef const struct __SCNetworkConnection * SCNetworkConnectionRef;


/*!
	@typedef SCNetworkConnectionContext
	@discussion Structure containing user-specified data and callbacks
		for a SCNetworkConnection.
	@field version The version number of the structure type being passed
		in as a parameter to the SCNetworkConnectionCreateWithServiceID
		function.  This structure is version 0.
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
	CFIndex         version;
	void *          info;
	const void      *(*retain)(const void *info);
	void            (*release)(const void *info);
	CFStringRef     (*copyDescription)(const void *info);
} SCNetworkConnectionContext;



/*!
	@enum SCNetworkConnectionStatus
	@discussion Status of the network connection.
		This status is intended to be generic and high level.
		An extended status, specific to the type of network
		connection is also available for applications that
		need additonal information.
	@constant kSCNetworkConnectionInvalid
		The network connection refers to an invalid service.
	@constant kSCNetworkConnectionDisconnected
		The network connection is disconnected.
	@constant kSCNetworkConnectionConnecting
		The network connection is connecting.
	@constant kSCNetworkConnectionConnected
		The network connection is connected.
	@constant kSCNetworkConnectionDisconnecting
		The network connection is disconnecting.
 */
enum {
	kSCNetworkConnectionInvalid		=  -1,
	kSCNetworkConnectionDisconnected	=  0,
	kSCNetworkConnectionConnecting		=  1,
	kSCNetworkConnectionConnected		=  2,
	kSCNetworkConnectionDisconnecting	=  3
};
typedef int32_t SCNetworkConnectionStatus;


/*!
	@enum SCNetworkConnectionPPPStatus
	@discussion PPP-specific status of the network connection.
		This status is returned as part of the extended information
		for a PPP service.
		Note: additional status might be returned in the future.
		Your application should be prepared to receive an unknown value.
	@constant kSCNetworkConnectionPPPDisconnected
		PPP is disconnected.
	@constant kSCNetworkConnectionPPPInitializing
		PPP is initializing.
	@constant kSCNetworkConnectionPPPConnectingLink
		PPP is connecting the lower connection layer (for example,
		the modem is dialing out).
	@constant kSCNetworkConnectionPPPDialOnTraffic
		PPP is waiting for networking traffic to automatically
		establish the connection.
	@constant kSCNetworkConnectionPPPNegotiatingLink
		The PPP lower layer is connected and PPP is negotiating the
		link layer (LCP protocol).
	@constant kSCNetworkConnectionPPPAuthenticating
		PPP is authenticating to the server (PAP, CHAP, MS-CHAP or
		EAP protocols).
	@constant kSCNetworkConnectionPPPWaitingForCallBack
		PPP is waiting for the server to call back.
	@constant kSCNetworkConnectionPPPNegotiatingNetwork
		PPP is now authenticated and negotiating the networking
		layer (IPCP or IPv6CP protocols)
	@constant kSCNetworkConnectionPPPConnected
		PPP is now fully connected for at least one networking layer.
		Additional networking protocol might still be negotiating.
	@constant kSCNetworkConnectionPPPTerminating
		PPP networking and link protocols are terminating.
	@constant kSCNetworkConnectionPPPDisconnectingLink
		PPP is disconnecting the lower level (for example, the modem
		is hanging up).
	@constant kSCNetworkConnectionPPPHoldingLinkOff
		PPP is disconnected and maintaining the link temporarily off.
	@constant kSCNetworkConnectionPPPSuspended
		PPP is suspended as a result of the suspend command (for
		example, when a V.92 Modem is On Hold).
	@constant kSCNetworkConnectionPPPWaitingForRedial
		PPP has found a busy server and is waiting for redial.
 */
enum {
	kSCNetworkConnectionPPPDisconnected		=  0,
	kSCNetworkConnectionPPPInitializing		=  1,
	kSCNetworkConnectionPPPConnectingLink		=  2,
	kSCNetworkConnectionPPPDialOnTraffic		=  3,
	kSCNetworkConnectionPPPNegotiatingLink		=  4,
	kSCNetworkConnectionPPPAuthenticating		=  5,
	kSCNetworkConnectionPPPWaitingForCallBack	=  6,
	kSCNetworkConnectionPPPNegotiatingNetwork	=  7,
	kSCNetworkConnectionPPPConnected		=  8,
	kSCNetworkConnectionPPPTerminating		=  9,
	kSCNetworkConnectionPPPDisconnectingLink	=  10,
	kSCNetworkConnectionPPPHoldingLinkOff		=  11,
	kSCNetworkConnectionPPPSuspended		=  12,
	kSCNetworkConnectionPPPWaitingForRedial		=  13
};
typedef int32_t SCNetworkConnectionPPPStatus;


/*!
	@typedef SCNetworkConnectionCallBack
	@discussion Type of the callback function used when a
		status event is delivered.
	@param status The connection status.
	@param connection The connection reference.
	@param info Application-specific information.
 */
typedef void (*SCNetworkConnectionCallBack)	(
						SCNetworkConnectionRef          connection,
						SCNetworkConnectionStatus       status,
						void                            *info
						);



/*
    Keys for the statistics dictionary
*/

#define kSCNetworkConnectionBytesIn		CFSTR("BytesIn")		/* CFNumber */
#define kSCNetworkConnectionBytesOut		CFSTR("BytesOut")		/* CFNumber */
#define kSCNetworkConnectionPacketsIn		CFSTR("PacketsIn")		/* CFNumber */
#define kSCNetworkConnectionPacketsOut		CFSTR("PacketsOut")		/* CFNumber */
#define kSCNetworkConnectionErrorsIn		CFSTR("ErrorsIn")		/* CFNumber */
#define kSCNetworkConnectionErrorsOut		CFSTR("ErrorsOut")		/* CFNumber */


/*
 Keys for the SCNetworkConnectionCopyUserPreferences() "selectionOptions"
 dictionary
 */

/*!
	@define kSCNetworkConnectionSelectionOptionOnDemandHostName
	@discussion A host name that will be used to select the
		"best" SCNetworkConnection.
 */
#define kSCNetworkConnectionSelectionOptionOnDemandHostName	CFSTR("OnDemandHostName")	// CFString
												// __OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_3_0/*SPI*/)

/*!
	@define kSCNetworkConnectionSelectionOptionOnDemandRetry
	@discussion A boolean value used to indicate whether a DNS query has
		already been issued for the specified OnDemand host name.
 */
#define kSCNetworkConnectionSelectionOptionOnDemandRetry	CFSTR("OnDemandRetry")		// CFBoolean
												// __OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_3_0/*SPI*/)

__BEGIN_DECLS

/*!
	@function SCNetworkConnectionGetTypeID
	@discussion Returns the type identifier of all SCNetworkConnection
		instances.
 */
CFTypeID
SCNetworkConnectionGetTypeID			(void)			__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0/*SPI*/);


/*!
	@function SCNetworkConnectionCopyUserPreferences
	@discussion Provides the default service ID and a dictionary of user
		options for the connection.  Applications can use the
		returned serviceID and userOptions values to open a
		connection on the fly.
	@param selectionOptions Currently unimplemented. Pass NULL for this
		version.
	@param serviceID Reference to the default serviceID for starting
		connections, this value will be returned by the function.
	@param userOptions Reference to default userOptions for starting
		connections, this will be returned by the function.
	@result Returns TRUE if there is a valid service to dial;
		FALSE if the function was unable to retrieve a service to dial.
 */
Boolean
SCNetworkConnectionCopyUserPreferences		(
						CFDictionaryRef			selectionOptions,
						CFStringRef			*serviceID,
						CFDictionaryRef			*userOptions
						)			__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0/*SPI*/);


/*!
	@function SCNetworkConnectionCreateWithServiceID
	@discussion Creates a new connection reference to use for getting
		the status or for connecting or disconnecting the associated
		service.
	@param allocator The CFAllocator that should be used to allocate
		memory for the connection structure.  This parameter may be
		NULL in which case the current default CFAllocator is used.
		If this reference is not a valid CFAllocator, the behavior
		is undefined.
	@param serviceID A string that defines the service identifier
		of the connection.  Service identifiers uniquely identify
		services in the system configuration database.
	@param callout The function to be called when the status
		of the connection changes.  If this parameter is NULL, the
		application will not receive notifications of status change
		and will need to poll for updates.
	@param context The SCNetworkConnectionContext associated with the
		callout.
	@result Returns a reference to the new SCNetworkConnection.
 */
SCNetworkConnectionRef
SCNetworkConnectionCreateWithServiceID		(
						CFAllocatorRef			allocator,
						CFStringRef			serviceID,
						SCNetworkConnectionCallBack	callout,
						SCNetworkConnectionContext	*context
						)			__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0/*SPI*/);


/*!
	@function SCNetworkConnectionCopyServiceID
	@discussion Returns the service ID associated with the SCNetworkConnection.
	@param connection The SCNetworkConnection to obtain status from.
	@result Returns the service ID associated with the SCNetworkConnection.
 */
CFStringRef
SCNetworkConnectionCopyServiceID		(
						SCNetworkConnectionRef		connection
						)			__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0/*SPI*/);


/*!
	@function SCNetworkConnectionGetStatus
	@discussion Returns the status of the SCNetworkConnection.
		A status is one of the following values:
<pre>
@textblock
&#32
	kSCNetworkConnectionInvalid
	kSCNetworkConnectionDisconnected
	kSCNetworkConnectionConnecting
	kSCNetworkConnectionDisconnecting
	kSCNetworkConnectionConnected
@/textblock
</pre>
	@param connection The SCNetworkConnection to obtain status from.
	@result Returns the status value.
*/
SCNetworkConnectionStatus
SCNetworkConnectionGetStatus			(
						SCNetworkConnectionRef		connection
						)			__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0/*SPI*/);


/*!
	@function SCNetworkConnectionCopyExtendedStatus
	@discussion Returns the extended status of the connection.
		An extended status dictionary contains specific dictionaries
		describing the status for each subcomponent of the service.

		For example, a status dictionary will contain the following
		sub-dictionaries, keys, and values:
<pre>
@textblock
&#32
	IPv4  : Addresses      : the assigned IP address.
&#32
	PPP   : Status         : the PPP-specific status of type
				 SCNetworkConnectionPPPStatus.
&#32
		LastCause      : Available when the status is "Disconnected"
				 and contains the last error associated with
				 connecting or disconnecting.
&#32
		ConnectTime    : the time when the connection was
				 established.
&#32
	Modem : ConnectSpeed   : the speed of the modem connection
				 in bits/second.
&#32
	IPSec : Status         : the IPSec-specific status of type
				 SCNetworkConnectionIPSecStatus
&#32
		ConnectTime    : the time when the connection was
				 established.

@/textblock
</pre>
		Other dictionaries could be present for PPPoE, PPTP, and L2TP.

		The status dictionary may be extended in the future to contain
		additional information.
	@param connection The SCNetworkConnection to obtain status from.
	@result Returns the status dictionary.
		If NULL is returned, the error can be retrieved using the SCError function.
 */
CFDictionaryRef
SCNetworkConnectionCopyExtendedStatus		(
						SCNetworkConnectionRef		connection
						)			__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0/*SPI*/);


/*!
	@function SCNetworkConnectionCopyStatistics
	@discussion Returns the statistics of the SCNetworkConnection.
		A statistic dictionary contains specific dictionaries
		with statistics for each subcomponent of the service.

		For example, a statistics dictionary will contain the following
		sub-dictionaries, keys, and values:
<pre>
@textblock
&#32
	PPP : BytesIn    :
	PPP : BytesOut   : Contains the number of bytes going up into
			   (or coming out of) the network stack for
			   any networking protocol without the PPP
			   headers and trailers.
&#32
	PPP : PacketsIn  :
	PPP : PacketsOut : Contains the number of packets going up into
			   (or coming out of) the network stack for
			   any networking protocol without the PPP
			   headers and trailers.
&#32
	PPP : ErrorsIn   :
	PPP : ErrorsOut  : Contains the number of errors going up into
			   (or coming out of) the network stack for
			   any networking protocol without the PPP
			   headers and trailers.
@/textblock
</pre>
		The statistics dictionary may be extended in the future to
		contain additional information.
	@param connection The SCNetworkConnection to obtained statistics from.
	@result Returns the statistics dictionary.
		If NULL is returned, the error can be retrieved using the SCError function.
 */
CFDictionaryRef
SCNetworkConnectionCopyStatistics		(
						SCNetworkConnectionRef		connection
						)			__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0/*SPI*/);


/*!
	@function SCNetworkConnectionStart
	@discussion Starts the connection for the SCNetworkConnection.
		The connection process is asynchronous and the function will
		return immediately.  The connection status can be obtained
		by polling or by callback.  The connection is made with the
		default settings from the administrator.  Some of the settings
		can be overridden for the duration of the connection.  These
		are specified in an options dictionary.  The options dictionary
		uses the same format as a network service defined in the system
		configuration preferences schema.

		Note: Starting and stopping of connections is implicitly
		arbitrated.  Calling SCNetworkConnectionStart on a connection
		already started will indicate that the application has
		interest in the connection and it shouldn't be stopped by
		anyone else.
	@param connection The SCNetworkConnection to start.
	@param userOptions The options dictionary to start the connection with.
		If userOptions is NULL, the default settings will be used.
		If userOptions are specified, they must be in the same format
		as network services stored in the system configuration
		preferences schema.  The options will override the default
		settings defined for the service.

		For security reasons, not all options can be overridden; the
		appropriate merging of all settings will be done before the
		connection is established, and inappropriate options will be
		ignored.
	@param linger This parameter indicates whether or not the connection
		can stay around when the application no longer has interest
		in it.  A typical application should pass FALSE, and the
		connection will be automatically stopped when the reference
		is released or if the application quits.  If the application
		passes TRUE, the application can release the reference or
		exit and the connection will be maintained until a timeout
		event, until a specific stop request occurs, or until an
		error is encountered.
	@result Returns TRUE if the connection was correctly started (the
		actual connection is not established yet, and the connection
		status needs to be periodically checked); FALSE if the
		connection request was not started.  The error must be
		retrieved from the SCError function.
 */
Boolean
SCNetworkConnectionStart			(
						SCNetworkConnectionRef		connection,
						CFDictionaryRef			userOptions,
						Boolean				linger
						)			__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0/*SPI*/);


/*!
	@function SCNetworkConnectionStop
	@discussion Stops the connection for the SCNetworkConnection.
		The disconnection process is asynchronous and the function
		will return immediately.  The connection status can be
		obtained by polling or by callback.  This function performs
		an arbitrated stop of the connection.  If several applications
		have marked their interest in the connection, by calling
		SCNetworkConnectionStart, the call will succeed but the
		actual connection will be maintained until the last interested
		application calls SCNetworkConnectionStop.

		In certain cases, you might want to stop the connection anyway.
		In these cases, you set the forceDisconnect argument to TRUE.
	@param connection The SCNetworkConnection to stop.
	@result Returns TRUE if the disconnection request succeeded;
		FALSE if the disconnection request failed.
		The error must be retrieved from the SCError function.
 */
Boolean
SCNetworkConnectionStop				(
						SCNetworkConnectionRef		connection,
						Boolean				forceDisconnect
						)			__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0/*SPI*/);


/*!
	@function SCNetworkConnectionCopyUserOptions
	@discussion Copies the user options used to start the connection.
		This is a mechanism a client can use to retrieve the user options
		previously passed to the SCNetworkConnectionStart function.
	@param connection The SCNetworkConnection to obtain options from.
	@result Returns the service dictionary containing the connection options.
		The dictionary can be empty if no user options were used.
		If NULL is returned, the error can be retrieved using the SCError function.
 */
CFDictionaryRef
SCNetworkConnectionCopyUserOptions		(
						SCNetworkConnectionRef		connection
						)			__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0/*SPI*/);


/*!
	@function SCNetworkConnectionScheduleWithRunLoop
	@discussion Schedules a connection with the run loop.
	@param connection The SCNetworkConnection to schedule.
	@param runLoop The run loop to schedule with.
	@param runLoopMode The run loop mode.
	@result Returns TRUE if the connection is scheduled successfully;
		FALSE if the scheduling failed.
		The error can be retrieved using the SCError function.
 */
Boolean
SCNetworkConnectionScheduleWithRunLoop		(
						SCNetworkConnectionRef		connection,
						CFRunLoopRef			runLoop,
						CFStringRef			runLoopMode
						)			__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0/*SPI*/);


/*!
	@function SCNetworkConnectionUnscheduleFromRunLoop
	@discussion Unschedules a connection from the run loop.
	@param connection The SCNetworkConnection to unschedule.
	@param runLoop The run loop to unschedule from.
	@param runLoopMode The run loop mode.
	@result Returns TRUE if the connection is unscheduled successfully;
		FALSE if the unscheduling failed.
		The error can be retrieved using the SCError function.
 */
Boolean
SCNetworkConnectionUnscheduleFromRunLoop	(
						SCNetworkConnectionRef		connection,
						CFRunLoopRef			runLoop,
						CFStringRef			runLoopMode
						)			__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0/*SPI*/);


/*!
	 @function SCNetworkConnectionSetDispatchQueue
	 @discussion Caller provides a dispatch queue on which the callback contained in connection will run.
	 @param connection The SCNetworkConnection to notify.
	 @param queue The libdispatch queue to run the callback on.
		Pass NULL to disable notifications, and release queue.
	 @result Returns TRUE if the notifications have been enabled/disabled as desired;
		 FALSE if not.
		 The error can be retrieved using the SCError function.
 */
Boolean
SCNetworkConnectionSetDispatchQueue		(
						 SCNetworkConnectionRef		connection,
						 dispatch_queue_t		queue
						 )			__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0/*SPI*/);

__END_DECLS

#endif	/* USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS */
#endif /* _SCNETWORKCONNECTION_H */

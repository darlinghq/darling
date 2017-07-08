/*
 * Copyright (c) 2000-2004, 2006, 2008-2010, 2012, 2015 Apple Inc. All rights reserved.
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

#ifndef _SYSTEMCONFIGURATION_H
#define _SYSTEMCONFIGURATION_H

#include <Availability.h>
#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>


/*!
	@header SystemConfiguration
	@discussion The System Configuration framework provides access to the
		data used to configure a running system.  The APIs provided
		by this framework communicate with the configd daemon.

		The configd daemon manages a dynamic store reflecting the
		desired configuration settings as well as the current state
		of the system.  The daemon provides a notification mechanism
		for processes that need to be aware of changes made to the
		data.  Lastly, the daemon loads a number of bundles (or
		plug-ins) that monitor low-level kernel events and, through
		a set of policy modules, keep the state data up to date.
 */

/*!
	@enum Error codes
	@discussion Returned error codes.
	@constant kSCStatusOK			Success
	@constant kSCStatusFailed		Non-specific Failure
	@constant kSCStatusInvalidArgument	Invalid argument
	@constant kSCStatusAccessError		Permission denied
	@constant kSCStatusNoKey		No such key
	@constant kSCStatusKeyExists		Data associated with key already defined
	@constant kSCStatusLocked		Lock already held
	@constant kSCStatusNeedLock		Lock required for this operation
	@constant kSCStatusNoStoreSession	Configuration daemon session not active
	@constant kSCStatusNoStoreServer	Configuration daemon not (or no longer) available
	@constant kSCStatusNotifierActive	Notifier is currently active
	@constant kSCStatusNoPrefsSession	Preferences session not active
	@constant kSCStatusPrefsBusy		Preferences update currently in progress
	@constant kSCStatusNoConfigFile		Configuration file not found
	@constant kSCStatusNoLink		No such link
	@constant kSCStatusStale		Write attempted on stale version of object
	@constant kSCStatusMaxLink		Maximum link count exceeded
	@constant kSCStatusReachabilityUnknown
		A determination could not be made regarding the reachability
		of the specified nodename or address.
	@constant kSCStatusConnectionNoService	Network service for connection not available
*/
enum {
	/*
	 * Generic error codes
	 */
	kSCStatusOK				= 0,	/* Success */
	kSCStatusFailed				= 1001,	/* Non-specific failure */
	kSCStatusInvalidArgument		= 1002,	/* Invalid argument */
	kSCStatusAccessError			= 1003,	/* Permission denied
							   - must be root to obtain lock
							   - could not create access/create preferences
							 */
	kSCStatusNoKey				= 1004,	/* No such key */
	kSCStatusKeyExists			= 1005,	/* Key already defined */
	kSCStatusLocked				= 1006,	/* Lock already held */
	kSCStatusNeedLock			= 1007,	/* Lock required for this operation */
	/*
	 * SCDynamicStore error codes
	 */
	kSCStatusNoStoreSession			= 2001,	/* Configuration daemon session not active */
	kSCStatusNoStoreServer			= 2002,	/* Configuration daemon not (no longer) available */
	kSCStatusNotifierActive			= 2003,	/* Notifier is currently active */
	/*
	 * SCPreferences error codes
	 */
	kSCStatusNoPrefsSession			= 3001,	/* Preference session not active */
	kSCStatusPrefsBusy			= 3002,	/* Preferences update currently in progress */
	kSCStatusNoConfigFile			= 3003,	/* Configuration file not found */
	kSCStatusNoLink				= 3004,	/* No such link */
	kSCStatusStale				= 3005,	/* Write attempted on stale version of object */
	kSCStatusMaxLink			= 3006,	/* Maximum link count exceeded */
	/*
	 * SCNetwork error codes
	 */
	kSCStatusReachabilityUnknown		= 4001,	/* Network reachability cannot be determined */
	/*
	 * SCNetworkConnection error codes
	 */
	kSCStatusConnectionNoService		= 5001,	/* Network service for connection not available
							   __OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_2_0)
							 */
	kSCStatusConnectionIgnore               = 5002, /* Network connection information not available at this time
							   __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
							 */
};


/* SCDynamicStore APIs */
#include <SystemConfiguration/SCDynamicStore.h>
#include <SystemConfiguration/SCDynamicStoreKey.h>
#include <SystemConfiguration/SCDynamicStoreCopySpecific.h>

/* SCPreferences APIs */
#include <SystemConfiguration/SCPreferences.h>
#include <SystemConfiguration/SCPreferencesPath.h>
#include <SystemConfiguration/SCPreferencesSetSpecific.h>

/* Schema Definitions (for SCDynamicStore and SCPreferences) */
#include <SystemConfiguration/SCSchemaDefinitions.h>

/* SCNetworkConfiguration APIs */
#include <SystemConfiguration/SCNetworkConfiguration.h>

/* SCNetworkReachability and SCNetworkConnection APIs */
#include <SystemConfiguration/SCNetwork.h>
#include <SystemConfiguration/SCNetworkReachability.h>
#include <SystemConfiguration/SCNetworkConnection.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_ASSUME_NONNULL_BEGIN

/*!
	@const kCFErrorDomainSystemConfiguration
	@discussion CFError domain associated with errors reported by
		the SystemConfiguration.framework.
 */
extern const CFStringRef	kCFErrorDomainSystemConfiguration	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

__BEGIN_DECLS

/*!
	@function SCCopyLastError
	@discussion Returns the most recent status or error code generated
		as the result of calling a System Configuration framework API.
	@result Returns the last error encountered.
 */
CFErrorRef	SCCopyLastError		(void)				__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

/*!
	@function SCError
	@discussion Returns the most recent status or error code generated
		as the result of calling a System Configuration framework API.
	@result Returns the last error encountered.
 */
int		SCError			(void)				__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0);

/*!
	@function SCErrorString
	@discussion Returns a pointer to the message string
		associated with the specified status or error.
	@param status The SCDynamicStoreStatus to be returned.
	@result Returns a pointer to the error message string.
 */
const char *	SCErrorString		(int	status)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0);

__END_DECLS

CF_ASSUME_NONNULL_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* _SYSTEMCONFIGURATION_H */

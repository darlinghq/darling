/*
 * Copyright (c) 2006, 2008, 2009, 2011-2014 Apple Inc. All rights reserved.
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

#ifndef _SCNETWORKCONNECTIONPRIVATE_H
#define _SCNETWORKCONNECTIONPRIVATE_H

#include <Availability.h>
#include <sys/cdefs.h>
#if !TARGET_IPHONE_SIMULATOR
#include <ne_session.h>
#endif
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCNetworkConfigurationPrivate.h>


typedef const struct __SCUserPreferencesRef * SCUserPreferencesRef;


__BEGIN_DECLS


#pragma mark -
#pragma mark SCNetworkConnection SPIs

CFArrayRef /* of SCNetworkServiceRef's */
SCNetworkConnectionCopyAvailableServices	(SCNetworkSetRef		set)			__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

SCNetworkConnectionRef
SCNetworkConnectionCreateWithService		(CFAllocatorRef			allocator,
						 SCNetworkServiceRef		service,
						 SCNetworkConnectionCallBack	callout,
						 SCNetworkConnectionContext	*context)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

SCNetworkServiceRef
SCNetworkConnectionGetService			(SCNetworkConnectionRef		connection)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

CFArrayRef /* of SCUserPreferencesRef's */
SCNetworkConnectionCopyAllUserPreferences	(SCNetworkConnectionRef		connection)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

SCUserPreferencesRef
SCNetworkConnectionCopyCurrentUserPreferences	(SCNetworkConnectionRef		connection)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

SCUserPreferencesRef
SCNetworkConnectionCreateUserPreferences	(SCNetworkConnectionRef		connection)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

Boolean
SCNetworkConnectionSuspend			(SCNetworkConnectionRef		connection)		__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0);

Boolean
SCNetworkConnectionResume			(SCNetworkConnectionRef		connection)		__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_2_0);

Boolean
SCNetworkConnectionRefreshOnDemandState		(SCNetworkConnectionRef		connection)		__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

Boolean
SCNetworkConnectionSetClientInfo		(SCNetworkConnectionRef		connection,
						 mach_port_t			client_audit_session,
						 uid_t				client_uid,
						 gid_t				client_gid,
						 pid_t				client_pid)		__OSX_AVAILABLE_STARTING(__MAC_10_8,__IPHONE_5_0);

/*!
 @function SCNetworkConnectionCreate
 @discussion Create a network connection that is not associated with any
     network service.
 @param allocator The CFAllocator that should be used to allocate
     memory for the connection structure.  This parameter may be
     NULL in which case the current default CFAllocator is used.
     If this reference is not a valid CFAllocator, the behavior
     is undefined.
 @param callout The function to be called when the status
     of the connection changes.  If this parameter is NULL, the
     application will not receive notifications of status change
     and will need to poll for updates.
 @param context The SCNetworkConnectionContext associated with the
     callout.
 @return The new SCNetworkConnection object.
 */
SCNetworkConnectionRef
SCNetworkConnectionCreate			(CFAllocatorRef			allocator,
						 SCNetworkConnectionCallBack	callout,
						 SCNetworkConnectionContext	*context)		__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

/*!
 @function SCNetworkConnectionSetClientAuditInfo
 @discussion Set audit information for the process that is initiating the network traffic
      that will be transmitted over this network connection.
 @param connection The SCNetworkConnection object.
 @param client_audit_token The audit token of the initiator process.
 @param audit_session The audit session mach port of the initiator process.
 @param bootstrap_port The bootstrap port of the initiator process.
 @param pid The PID of the initiator process.
 @param uuid The Mach-O UUID of the initiator process.
 @param bundle_id The CFBundleIdentifier of the initiator process.
 @return TRUE if the audit token was set successfully, FALSE if an error occurred.
 */
Boolean
SCNetworkConnectionSetClientAuditInfo		(SCNetworkConnectionRef		connection,
						 audit_token_t			client_audit_token,
						 mach_port_t			audit_session,
						 mach_port_t			bootstrap_port,
						 pid_t				pid,
						 const uuid_t			uuid,
						 const char			*bundle_id)		__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

/*!
 @defined kSCNetworkConnectionSelectionOptionNoUserPrefs
 @abstract Indicates whether or not SCNetworkConnectionSelectServiceWithOptions
     should consult the user preferences to find a network service.
 */
#define kSCNetworkConnectionSelectionOptionNoUserPrefs		CFSTR("NoUserPrefs")	/* CFBoolean */

/*!
 @define kSCNetworkConnectionSelectionOptionOnDemandTrafficClass
 @abstract The traffic class that is attempting to trigger OnDemand.
 */
#define kSCNetworkConnectionSelectionOptionOnDemandTrafficClass	CFSTR("OnDemandTrafficClass")	// CFNumber
												// __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0/*SPI*/)
/*!
 @define kSCNetworkConnectionSelectionOptionOnDemandAccountIdentifier
 @abstract The account identifier that is attempting to trigger OnDemand.
 */
#define kSCNetworkConnectionSelectionOptionOnDemandAccountIdentifier	CFSTR("OnDemandAccountIdentifier")	/* CFString */

/*!
 @define kSCNetworkConnectionSelectionOptionOnDemandMatchInfo
 @abstract A dictionary containing information about the On Demand trigger that matched
 */
#define kSCNetworkConnectionSelectionOptionOnDemandMatchInfo	CFSTR("OnDemandMatchInfo")	/* CFDictionary */

/*!
 @define kSCNetworkConnectionOnDemandMatchInfoVPNType
 @abstract The type of VPN connection associated with the matching trigger.
 */
#define kSCNetworkConnectionOnDemandMatchInfoVPNType		CFSTR("OnDemandMatchInfoVPNType")	/* CFNumber containing a SCNetworkConnectionType */

/*!
 @define kSCNetworkConnectionOnDemandMatchInfoDomain
 @abstract The specific DNS domain in the trigger's match domains that matched the on demand hostname.
 */
#define kSCNetworkConnectionOnDemandMatchInfoDomain		CFSTR("OnDemandMatchInfoDomain")	/* CFString */

/*!
 @define kSCNetworkConnectionOnDemandMatchInfoAppRuleID
 @abstract The identifier of the app rule in the matching trigger that matched the calling app.
 */
#define kSCNetworkConnectionOnDemandMatchInfoAppRuleID		CFSTR("OnDemandMatchInfoAppRuleID")	/* CFString */

/*
 @define kSCNetworkConnectionOnDemandMatchInfoOnRetry
 @abstract A flag indicating if the on demand hostname matched a domain in the "on retry" match domains.
 */
#define kSCNetworkConnectionOnDemandMatchInfoOnRetry		CFSTR("OnDemandMatchInfoOnRetry")	/* CFString */


/*!
 @function SCNetworkConnectionSelectServiceWithOptions
 @discussion Associate a network connection with a network service based on some
      selection options. This function can only be called on connections that are
      not already associated with a network service.
 @param connection The SCNetworkConnection object.
 @param selectionOptions A dictionary containing some options to be used to
      select the appropriate service.
 @return TRUE if an appropriate network service was found and was associated with
      the connection, FALSE otherwise.
 */
Boolean
SCNetworkConnectionSelectServiceWithOptions	(SCNetworkConnectionRef		connection,
						 CFDictionaryRef		selectionOptions)	__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

/*!
 @function SCNetworkConnectionOnDemandShouldRetryOnFailure
 @discussion After SCNetworkConnectionSelectServiceWithOptions returns FALSE, use
	this function to determine if an On Demand service was indeed matched for
	On Retry behavior (try resolving/connecting, and start VPN on failure).
 @param connection The SCNetworkConnection object.
 @return TRUE if the selected On Demand connection should be retried on DNS or connection
	failure, FALSE otherwise.
 */
Boolean
SCNetworkConnectionOnDemandShouldRetryOnFailure	(SCNetworkConnectionRef		connection)		__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

/*!
 @function SCNetworkConnectionCanTunnelAddress
 @discussion This function should be called on a connection object selected with
	SCNetworkConnectionSelectServiceWithOptions. If the address belongs to
	the connection (the tunnel can be used for the address), the function
	will return TRUE. If startImmediately is set, the address should not be trusted
	and the VPN should be brought up before attempting to connect.
 @param connection The SCNetworkConnection object.
 @param address The address structure to check.
 @param startImmediately On return, TRUE indicates that the address is not trusted
	and the VPN should be started immediately.
 @return TRUE if the selected VPN connection's tunnel routes match the address.
 */
Boolean
SCNetworkConnectionCanTunnelAddress		(SCNetworkConnectionRef		connection,
						 const struct sockaddr		*address,
						 Boolean			*startImmediately)	__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

/*!
 @function SCNetworkConnectionIsOnDemandSuspended
 @discussion Indicates whether the On Demand connection is suspended or not. Call
	SCNetworkConnectionSelectServiceWithOptions before calling this function.
 @param connection The SCNetworkConnection object.
 @return TRUE if the On Demand connection is suspended, FALSE otherwise.
 */
Boolean
SCNetworkConnectionIsOnDemandSuspended		(SCNetworkConnectionRef		connection)		__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

/*!
 @function SCNetworkConnectionCopyOnDemandInfo
 @discussion Indicates whether or not the caller should start the network connection,
      assuming that the caller has network traffic that needs to use the network connection.
 @param connection The SCNetworkConnection object.
 @param onDemandRemoteAddress On return, contains the address of the server providing the
      network connection. Ownership follows the "Create" rule.
 @param onDemandConnectionStatus On return, contains the current status of the network
      connection.
 @return TRUE if the caller should start the connection upon traffic demand, FALSE otherwise.
 */
Boolean
SCNetworkConnectionCopyOnDemandInfo		(SCNetworkConnectionRef		connection,
						 CFStringRef			*onDemandRemoteAddress,
						 SCNetworkConnectionStatus	*onDemandConnectionStatus)	__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

/*!
 @function SCNetworkConnectionTriggerOnDemandIfNeeded
 @discussion Trigger a VPN On Demand connection based on a hostname. This function combines the
	functionality of calling SCNetworkConnectionCreate, SCNetworkConnectionSelectServiceWithOptions,
	and SCNetworkConnectionStart. The function blocks until the connection is established,
	fails, or the timer runs out. Since it blocks, this function should not generally be called on
	the main runloop. NOTE: This function should only be called from process running
	in the user's context to ensure that the user's keychain is available.
 @param hostName The hostname that needs to be resolved and accessed.
 @param afterDNSFail Pass FALSE if this call is made before trying to resolve the hostname, and
	TRUE if the hostname resolution already failed.
 @param timeout Number of seconds to wait for a connection. Passing 0 sets a timeout of forever.
 @param trafficClass Numeric value of a traffic class. Pass 0 for default traffic class, 'Best Effort'.
 @return TRUE if the connection was established or was not needed, FALSE otherwise.
 */
Boolean
SCNetworkConnectionTriggerOnDemandIfNeeded	(CFStringRef			hostName,
						 Boolean			afterDNSFail,
						 int				timeout,
						 int				trafficClass)			__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

/*!
 @function SCNetworkConnectionGetReachabilityInfo
 @discussion Get the reachability info of the connection server.
 @param connection The SCNetworkConnection object.
 @param reach_flags On return, contains the reachability flags of the connection server.
 @param reach_if_index On return, contains the index of the interface through which
      the connection server is reachable.
 @return TRUE if the reachability information was available for the connection
      server, FALSE otherwise.
 */
Boolean
SCNetworkConnectionGetReachabilityInfo		(SCNetworkConnectionRef		connection,
						 SCNetworkReachabilityFlags	*reach_flags,
						 unsigned int			*reach_if_index)	__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);


/*!
 @enum SCNetworkConnectionType
 @discussion Type of the network connection.
 @constant kSCNetworkConnectionTypeUnknown
     The type of the network connection has not yet been determined.
 @constant kSCNetworkConnectionTypePPP
     The network connection is a Point To Point Protocol connection.
 @constant kSCNetworkConnectionTypeIPLayerVPN
     The network connection is a IP-layer Virtual Private Network connection.
 @constant kSCNetworkConnectionTypeAppLayerVPN
     The network connection is an Application-layer Virtual Private Network connection.
 */
enum {
	kSCNetworkConnectionTypeUnknown 	= 0,
	kSCNetworkConnectionTypePPP		= 1,
	kSCNetworkConnectionTypeIPLayerVPN	= 2,
	kSCNetworkConnectionTypeAppLayerVPN	= 3,
};

typedef int SCNetworkConnectionType;

/*!
 @function SCNetworkConnectionGetType
 @discussion Get the type of the network connection.
 @param connection The SCNetworkConnection object.
 @return The type of the network connection.
 */
SCNetworkConnectionType
SCNetworkConnectionGetType			(SCNetworkConnectionRef		connection)		__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

/*!
 @defined kSCNetworkConnectionFlowPropertyHostName
 @abstract The host name of the flow's destination server.
 */
#define kSCNetworkConnectionFlowPropertyHostName	CFSTR("HostName")	/* CFString */

/*!
 @defined kSCNetworkConnectionFlowPropertyHostAddress
 @abstract The address of the flow's destination server.
 */
#define kSCNetworkConnectionFlowPropertyHostAddress	CFSTR("HostAddress")	/* CFData containing a struct sockaddr */

/*!
 @defined kSCNetworkConnectionFlowPropertyHostPort
 @abstract The port of the flow's destination server.
 */
#define kSCNetworkConnectionFlowPropertyHostPort	CFSTR("HostPort")	/* CFNumber */

/*!
 @function SCNetworkConnectionCopyFlowDivertToken
 @discussion Copy a token that should be used to activate flow divert on
      a socket, causing data on the socket to be diverted through the
      Application-layer VPN provided by the given network connection.
 @param connection The SCNetworkConnection object.
 @param flowProperties A dictionary containing properties of the socket
      to be diverted.
 @return The flow divert token, or NULL if the flow properties are not valid
      or the connection does not provide Application-Layer VPN services.
 */
CFDataRef
SCNetworkConnectionCopyFlowDivertToken		(SCNetworkConnectionRef		connection,
						 CFDictionaryRef		flowProperties)		__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

#define kSCNetworkConnectionAppPropertyRuleID		CFSTR("RuleID")
#define kSCNetworkConnectionAppPropertyCodeDirHash	CFSTR("CodeDirHash")
#define kSCNetworkConnectionAppPropertySigningID	CFSTR("SigningID")
#define kSCNetworkConnectionAppPropertyAuditToken	CFSTR("AuditToken")
#define kSCNetworkConnectionAppPropertyPID		CFSTR("ProcessID")
#define kSCNetworkConnectionAppPropertyUUID		CFSTR("UUID")

int
SCNetworkConnectionGetServiceIdentifier		(SCNetworkConnectionRef		connection)		__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

#pragma mark -
#pragma mark SCNetworkConnection "VPN on Demand" SPIs


/* VPN On Demand
 *
 * in the SCDynamicStore we will have :
 *
 *   <key>State:/Network/Global/OnDemand</key>
 *   <dict>
 *     <key>Triggers</key>
 *     <array>
 *       <dict>
 *         <key>ServiceID</key>
 *         <string>A740678C-1983-492B-BF64-B825AAE7101E</string>
 *         <key>Status</key>
 *         <integer>8</integer>
 *         <key>RemoteAddress</key>
 *         <string>vpn.mycompany.com</string>
 *         <key>OnDemandMatchDomainsAlways</key>
 *         <array>
 *           <string>internal.mycompany.com</string>
 *         </array>
 *         <key>OnDemandMatchDomainsOnRetry</key>
 *         <array>
 *           <string>mycompany.com</string>
 *         </array>
 *         <key>kSCNetworkConnectionOnDemandMatchDomainsNever</key>
 *         <array>
 *           <string>external.mycompany.com</string>
 *         </array>
 *         <key>OnDemandMatchAppEnabled</key>
 *         <integer>0</integer>
 *       </dict>
 *     </array>
 *   </dict>
 */

// notify(3) key
#define kSCNETWORKCONNECTION_ONDEMAND_NOTIFY_KEY		"com.apple.system.SCNetworkConnectionOnDemand"

// SCNetworkConnection status
// (included in the dictionary returned by SCNetworkConnectionCopyExtendedStatus)
#define kSCNetworkConnectionStatus				CFSTR("Status")			/* CFNumber */

// a CFArray[CFDictionary] of VPN on Demand "trigger" configurations
#define kSCNetworkConnectionOnDemandTriggers			CFSTR("Triggers")		/* CFArray[CFDictionary] */

// VPN service ID
#define kSCNetworkConnectionOnDemandServiceID			CFSTR("ServiceID")		/* CFString */

// VPN service status (idle, connecting, connected, disconnecting)
#define kSCNetworkConnectionOnDemandStatus			CFSTR("Status")			/* CFNumber */

// VPN server address
#define kSCNetworkConnectionOnDemandRemoteAddress		CFSTR("RemoteAddress")		/* CFString */

// Reachability flags for the VPN server
#define kSCNetworkConnectionOnDemandReachFlags			CFSTR("ReachFlags")		/* CFNumber */

// Reachability interface index for the VPN server
#define kSCNetworkConnectionOnDemandReachInterfaceIndex		CFSTR("ReachInterfaceIndex")	/* CFNumber */

// Network detection has detected DNS Redirecting (like OpenDNS)
#define kSCNetworkConnectionOnDemandDNSRedirectDetected		CFSTR("DNSRedirectDetected")	/* CFBoolean */

// a dictionary of address information for known lying results
#define kSCNetworkConnectionOnDemandDNSRedirectedAddresses	CFSTR("DNSRedirectedAddresses")	/* CFDictionary[CFDictionary] */

// A dictionary of routes tunneled over this service in the past
#define kSCNetworkConnectionOnDemandTunneledNetworks		CFSTR("TunneledNetworks")	/* CFDictionary[CFDictionary] */

// An array of plugin PIDs
#define kSCNetworkConnectionOnDemandPluginPIDs			CFSTR("PluginPIDs")		/* CFArray[CFNumber] */

// A dictionary of results, keyed by probe string
#define kSCNetworkConnectionOnDemandProbeResults		CFSTR("ProbeResults")		/* CFDictionary[CFBoolean] */

/* The following generic NetworkInfo keys are currently used in the dynamic store as follows:

 kSCNetworkConnectionOnDemandTunneledNetworks (CFDictionary)
	- kSCNetworkConnectionNetworkInfoIPv4 (CFDictionary)
		- kSCNetworkConnectionNetworkInfoIncludedRoutes (CFDictionary)
			- kSCNetworkConnectionNetworkInfoAddresses (CFData)
			- kSCNetworkConnectionNetworkInfoMasks (CFData)
		- kSCNetworkConnectionNetworkInfoExcludedRoutes (CFDictionary)
			- kSCNetworkConnectionNetworkInfoAddresses (CFData)
			- kSCNetworkConnectionNetworkInfoMasks (CFData)
	- kSCNetworkConnectionNetworkInfoIPv6 (CFDictionary)
		[Same as for IPv4]

 kSCNetworkConnectionOnDemandDNSRedirectedAddresses (CFDictionary)
	- kSCNetworkConnectionNetworkInfoIPv4 (CFDictionary)
		- kSCNetworkConnectionNetworkInfoAddresses (CFData)
	- kSCNetworkConnectionNetworkInfoIPv6 (CFDictionary)
		- kSCNetworkConnectionNetworkInfoAddresses (CFData)
 */
#define kSCNetworkConnectionNetworkInfoIPv4			CFSTR("IPv4")			/* CFDictionary[CFType] */
#define kSCNetworkConnectionNetworkInfoIPv6			CFSTR("IPv6")			/* CFDictionary[CFType] */
#define kSCNetworkConnectionNetworkInfoIncludedRoutes		CFSTR("IncludedRoutes")		/* CFDictionary[CFData] */
#define kSCNetworkConnectionNetworkInfoExcludedRoutes		CFSTR("ExcludedRoutes")		/* CFDictionary[CFData] */
#define kSCNetworkConnectionNetworkInfoAddresses		CFSTR("Addresses")		/* CFData */
#define kSCNetworkConnectionNetworkInfoMasks			CFSTR("Masks")			/* CFData */

// a CFArray[CFString] representing those domain (or host) names that, if
// matched to a target hostname, should result in our first establishing
// the VPN connection before any DNS queries are issued.
#define kSCNetworkConnectionOnDemandMatchDomainsAlways		CFSTR("OnDemandMatchDomainsAlways")	/* CFArray[CFString] */

// a CFArray[CFString] representing those domain (or host) names that, if
// matched to a target hostname, should result in a DNS query regardless of
// whether the VPN connection has been established.  If the DNS query returns
// an [EAI_NONAME] error then we should establish the VPN connection and
// re-issue / retry the query.
#define kSCNetworkConnectionOnDemandMatchDomainsOnRetry		CFSTR("OnDemandMatchDomainsOnRetry")	/* CFArray[CFString] */

// a CFArray[CFString] representing those domain (or host) names that should
// be excluded from those that would be used to establish tje VPN connection.
#define kSCNetworkConnectionOnDemandMatchDomainsNever		CFSTR("OnDemandMatchDomainsNever")	/* CFArray[CFString] */

// A CFNumber (0 or 1) indicating whether or not the App Layer rules should be
// used to decide whether or not to establish the tunnel connection.
#define kSCNetworkConnectionOnDemandMatchAppEnabled		CFSTR("OnDemandMatchAppEnabled")	/* CFNumber */

Boolean
__SCNetworkConnectionCopyOnDemandInfoWithName	(SCDynamicStoreRef		*storeP,
						 CFStringRef			nodeName,
						 Boolean			onDemandRetry,
						 CFStringRef			*connectionServiceID,
						 SCNetworkConnectionStatus	*connectionStatus,
						 CFStringRef			*vpnRemoteAddress)	__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_2_0);

#if !TARGET_IPHONE_SIMULATOR
SCNetworkConnectionStatus
SCNetworkConnectionGetStatusFromNEStatus	(ne_session_status_t status)				__OSX_AVAILABLE_STARTING(__MAC_10_10,__IPHONE_8_0);
#endif /* !TARGET_IPHONE_SIMULATOR */

#pragma mark -
#pragma mark SCUserPreferences SPIs


Boolean
SCUserPreferencesRemove				(SCUserPreferencesRef		userPreferences)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

Boolean
SCUserPreferencesSetCurrent			(SCUserPreferencesRef		userPreferences)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

CFStringRef
SCUserPreferencesCopyName			(SCUserPreferencesRef		userPreferences)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

CFTypeID
SCUserPreferencesGetTypeID			(void)							__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

CFStringRef
SCUserPreferencesGetUniqueID			(SCUserPreferencesRef		userPreferences)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

Boolean
SCUserPreferencesIsForced			(SCUserPreferencesRef		userPreferences)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

Boolean
SCUserPreferencesSetName			(SCUserPreferencesRef		userPreferences,
						 CFStringRef			newName)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

Boolean
SCNetworkConnectionStartWithUserPreferences	(SCNetworkConnectionRef		connection,
						 SCUserPreferencesRef		userPreferences,
						 Boolean			linger)			__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

CFDictionaryRef
SCUserPreferencesCopyInterfaceConfiguration	(SCUserPreferencesRef		userPreferences,
						 SCNetworkInterfaceRef		interface)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

Boolean
SCUserPreferencesSetInterfaceConfiguration	(SCUserPreferencesRef		userPreferences,
						 SCNetworkInterfaceRef		interface,
						 CFDictionaryRef		newOptions)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

CFDictionaryRef
SCUserPreferencesCopyExtendedInterfaceConfiguration
						(SCUserPreferencesRef		userPreferences,
						 SCNetworkInterfaceRef		interface,
						 CFStringRef			extendedType)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

Boolean
SCUserPreferencesSetExtendedInterfaceConfiguration
						(SCUserPreferencesRef		userPreferences,
						 SCNetworkInterfaceRef		interface,
						 CFStringRef			extendedType,
						 CFDictionaryRef		newOptions)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);


#pragma mark -
#pragma mark SCUserPreferences + SCNetworkInterface Password SPIs


Boolean
SCUserPreferencesCheckInterfacePassword		(SCUserPreferencesRef		userPreferences,
						 SCNetworkInterfaceRef		interface,
						 SCNetworkInterfacePasswordType	passwordType)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

CFDataRef
SCUserPreferencesCopyInterfacePassword		(SCUserPreferencesRef		userPreferences,
						 SCNetworkInterfaceRef		interface,
						 SCNetworkInterfacePasswordType	passwordType)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

Boolean
SCUserPreferencesRemoveInterfacePassword	(SCUserPreferencesRef		userPreferences,
						 SCNetworkInterfaceRef		interface,
						 SCNetworkInterfacePasswordType	passwordType)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

Boolean
SCUserPreferencesSetInterfacePassword		(SCUserPreferencesRef		userPreferences,
						 SCNetworkInterfaceRef		interface,
						 SCNetworkInterfacePasswordType	passwordType,
						 CFDataRef			password,
						 CFDictionaryRef		options)		__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0);

__END_DECLS

#endif /* _SCNETWORKCONNECTIONPRIVATE_H */

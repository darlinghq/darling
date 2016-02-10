/*
 * Copyright (c) 2003-2014 Apple Inc. All rights reserved.
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

#ifndef _SCNETWORKREACHABILITYINTERNAL_H
#define _SCNETWORKREACHABILITYINTERNAL_H

#include <Availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include <dispatch/dispatch.h>

#include <dns_sd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <net/if.h>
#include <xpc/xpc.h>




#pragma mark -
#pragma mark SCNetworkReachability


#define kSCNetworkReachabilityFlagsFirstResolvePending	(1<<31)



#define kSCNetworkReachabilityFlagsMask			0x00ffffff	// top 8-bits reserved for implementation


typedef	enum {
	NO	= 0,
	YES,
	UNKNOWN
} lazyBoolean;


typedef enum {
	// by-address SCNetworkReachability targets
	reachabilityTypeAddress,
	reachabilityTypeAddressPair,
	// by-name SCNetworkReachability targets
	reachabilityTypeName,
	reachabilityTypePTR
} ReachabilityAddressType;

#define isReachabilityTypeAddress(type)		(type < reachabilityTypeName)
#define isReachabilityTypeName(type)		(type >= reachabilityTypeName)


typedef struct {
	uint64_t			cycle;
	SCNetworkReachabilityFlags	flags;
	unsigned int			if_index;
	char				if_name[IFNAMSIZ];
	Boolean				sleeping;
} ReachabilityInfo;


typedef struct {

	/* base CFType information */
	CFRuntimeBase			cfBase;

	/* lock */
	pthread_mutex_t			lock;

	/* logging */
	Boolean				quiet;

	/* address type */
	ReachabilityAddressType		type;

	/* target host name */
	const char			*name;
	Boolean				needResolve;
	CFArrayRef			resolvedAddresses;	/* CFArray[CFData] or CFArray[CFString] */
	int				resolvedError;
	SCNetworkReachabilityFlags	resolverFlags;

	/* [scoped routing] interface constraints */
	unsigned int			if_index;
	char				if_name[IFNAMSIZ];

	/* local & remote addresses */
	struct sockaddr			*localAddress;
	struct sockaddr			*remoteAddress;

	/* current reachability flags */
	uint64_t			cycle;
	ReachabilityInfo		info;
	ReachabilityInfo		last_notify;

	/* run loop source, callout, context, rl scheduling info */
	Boolean				scheduled;
	CFRunLoopSourceRef		rls;
	SCNetworkReachabilityCallBack	rlsFunction;
	SCNetworkReachabilityContext	rlsContext;
	CFMutableArrayRef		rlList;
	unsigned int			pending;		// 0 == no notifications queued, else # to be delivered

	dispatch_group_t		dispatchGroup;
	dispatch_queue_t		dispatchQueue;		// SCNetworkReachabilitySetDispatchQueue

	/* [async] DNS query info */
	Boolean				haveDNS;
	struct timeval			dnsQueryStart;
	struct timeval			dnsQueryEnd;

	/* [async] processing info */
	struct timeval			last_dns;
	struct timeval			last_network;
#if	!TARGET_OS_IPHONE
	struct timeval			last_power;
#endif	// !TARGET_OS_IPHONE
	struct timeval			last_push;

	/* on demand info */
	Boolean				onDemandBypass;
	CFStringRef			onDemandName;
	CFStringRef			onDemandRemoteAddress;
	SCNetworkReachabilityRef	onDemandServer;
	CFStringRef			onDemandServiceID;


	union {
		uint32_t		dnsFlags;
		struct {
			Boolean		dnsActive     :1;	// if DNSServiceGetAddrInfo active

			Boolean		dnsHaveError  :1;	// error during query
			Boolean		dnsHaveV4     :1;	// have IPv4 (A) reply
			Boolean		dnsHaveV6     :1;	// have IPv6 (AAAA) reply
			Boolean		dnsHavePTR    :1;	// have PTR reply
			Boolean		dnsHaveTimeout:1;	// no replies (A, AAAA, or PTR)
		};
	};
	CFArrayRef			dnsAddresses;		// CFArray[CFData]
	Boolean				dnsBlocked;		// if DNS query blocked
	int				dnsError;
	int				dnsFailures;		// # of unexpected DNSServiceXXX errors
	int				dnsGeneration;
	DNSServiceRef			dnsTarget;
	Boolean				dnsNoAddressesSinceLastTimeout;

	/* SCNetworkReachability server "client" info */
	Boolean				serverActive;
	Boolean				serverBypass;
	Boolean				serverScheduled;
	ReachabilityInfo		serverInfo;

	/* SCNetworkReachability server "server" info */
	CFDataRef			serverDigest;
	dispatch_group_t		serverGroup;
	Boolean				serverInfoValid;
	unsigned int			serverSyncQueryActive;	// 0 == no [sync] query active, else # waiting on group
	dispatch_queue_t		serverQueue;
	unsigned int			serverReferences;	// how many [client] targets
	CFMutableDictionaryRef		serverWatchers;		// [client_id/target_id] watchers

	Boolean				useNEVPN;
	uid_t				uid;
	void				*nePolicyResult;
	Boolean				serverBypassForVPN;	// if serverBypassForVPN, only use client mode

	Boolean				resolverBypass;		// set this flag to bypass resolving the name



	/* logging */
	char				log_prefix[32];

} SCNetworkReachabilityPrivate, *SCNetworkReachabilityPrivateRef;



// ------------------------------------------------------------

#pragma mark -
#pragma mark [XPC] Reachability Server


#define	REACH_SERVER_VERSION		20110323

#if	!TARGET_IPHONE_SIMULATOR
#define	REACH_SERVICE_NAME		"com.apple.SystemConfiguration.SCNetworkReachability"
#else	// !TARGET_IPHONE_SIMULATOR
#define	REACH_SERVICE_NAME		"com.apple.SystemConfiguration.SCNetworkReachability_sim"
#endif	// !TARGET_IPHONE_SIMULATOR

// ------------------------------------------------------------


#pragma mark -
#pragma mark [XPC] Reachability Server (client->server request)


#define	REACH_CLIENT_PROC_NAME		"proc_name"		// string
#define	REACH_CLIENT_TARGET_ID		"target_id"		// uint64

#define	REACH_REQUEST			"request_op"		// int64

enum {
	REACH_REQUEST_CREATE		= 0x0001,
	REACH_REQUEST_REMOVE,
	REACH_REQUEST_SCHEDULE,
	REACH_REQUEST_STATUS,
	REACH_REQUEST_UNSCHEDULE,
	REACH_REQUEST_SNAPSHOT		= 0x0101,
};

#define	REACH_TARGET_NAME		"name"			// string

#define	REACH_TARGET_LOCAL_ADDR		"local_address"		// data (struct sockaddr)
#define	REACH_TARGET_REMOTE_ADDR	"remote_address"	// data (struct sockaddr)

#define	REACH_TARGET_PTR_ADDR		"ptr_address"		// data (struct sockaddr)

#define	REACH_TARGET_IF_INDEX		"if_index"		// int64
#define	REACH_TARGET_IF_NAME		"if_name"		// string
#define	REACH_TARGET_ONDEMAND_BYPASS	"ondemand_bypass"	// bool
#define REACH_TARGET_RESOLVER_BYPASS	"resolver_bypass"	// bool


#define REACH_REQUEST_REPLY		"reply"			// int64
#define REACH_REQUEST_REPLY_DETAIL	"reply_detail"		// string

enum {
	REACH_REQUEST_REPLY_OK		= 0x0000,
	REACH_REQUEST_REPLY_FAILED,
	REACH_REQUEST_REPLY_UNKNOWN,
};


// ------------------------------------------------------------


#pragma mark -
#pragma mark [XPC] Reachability Server (server->client request)


#define	MESSAGE_NOTIFY			"notify_op"		// int64

enum {
	MESSAGE_REACHABILITY_STATUS	= 0x1001,
};

#define REACH_STATUS_CYCLE		"cycle"			// uint64
#define REACH_STATUS_DNS_FLAGS		"dns_flags"		// uint64
#define REACH_STATUS_FLAGS		"flags"			// uint64
#define REACH_STATUS_IF_INDEX		"if_index"		// uint64
#define REACH_STATUS_IF_NAME		"if_name"		// data (char if_name[IFNAMSIZ])
#define REACH_STATUS_RESOLVED_ADDRESSES	"resolved_addresses"	// array[data]
#define REACH_STATUS_RESOLVED_ERROR	"resolved_error"	// int64
#define REACH_STATUS_SLEEPING		"sleeping"		// bool


// ------------------------------------------------------------


__BEGIN_DECLS

CFStringRef
_SCNetworkReachabilityCopyTargetDescription	(SCNetworkReachabilityRef	target);

CFStringRef
_SCNetworkReachabilityCopyTargetFlags		(SCNetworkReachabilityRef	target);

void
__SCNetworkReachabilityUpdate			(SCNetworkReachabilityRef	target);

void
__SCNetworkReachabilityUpdateConcurrent		(SCNetworkReachabilityRef	target);

dispatch_queue_t
__SCNetworkReachability_concurrent_queue	(void);

#pragma mark -
#pragma mark [XPC] Reachability Server (client APIs)

Boolean
_SCNetworkReachabilityServer_snapshot		(void);

Boolean
__SCNetworkReachabilityServer_targetAdd		(SCNetworkReachabilityRef	target);

void
__SCNetworkReachabilityServer_targetRemove	(SCNetworkReachabilityRef	target);

Boolean
__SCNetworkReachabilityServer_targetSchedule	(SCNetworkReachabilityRef	target);

Boolean
__SCNetworkReachabilityServer_targetStatus	(SCNetworkReachabilityRef	target);

Boolean
__SCNetworkReachabilityServer_targetUnschedule	(SCNetworkReachabilityRef	target);


Boolean
__SC_checkResolverReachabilityInternal		(SCDynamicStoreRef		*storeP,
						 SCNetworkReachabilityFlags	*flags,
						 Boolean			*haveDNS,
						 const char			*nodename,
						 uint32_t			*resolver_if_index,
						 int				*dns_config_index);

static __inline__ void
__SCNetworkReachabilityPrintFlags(SCNetworkReachabilityFlags flags)
{
	if (flags != 0) {
		if (flags & kSCNetworkReachabilityFlagsReachable) {
			SCPrint(TRUE, stdout, CFSTR("Reachable"));
			flags &= ~kSCNetworkReachabilityFlagsReachable;
			SCPrint(flags != 0, stdout, CFSTR(","));
		}
		if (flags & kSCNetworkReachabilityFlagsTransientConnection) {
			SCPrint(TRUE, stdout, CFSTR("Transient Connection"));
			flags &= ~kSCNetworkReachabilityFlagsTransientConnection;
			SCPrint(flags != 0, stdout, CFSTR(","));
		}
		if (flags & kSCNetworkReachabilityFlagsConnectionRequired) {
			SCPrint(TRUE, stdout, CFSTR("Connection Required"));
			flags &= ~kSCNetworkReachabilityFlagsConnectionRequired;
			SCPrint(flags != 0, stdout, CFSTR(","));
		}
		if (flags & kSCNetworkReachabilityFlagsConnectionOnTraffic) {
			SCPrint(TRUE, stdout, CFSTR("Automatic Connection On Traffic"));
			flags &= ~kSCNetworkReachabilityFlagsConnectionOnTraffic;
			SCPrint(flags != 0, stdout, CFSTR(","));
		}
		if (flags & kSCNetworkReachabilityFlagsConnectionOnDemand) {
			SCPrint(TRUE, stdout, CFSTR("Automatic Connection On Demand"));
			flags &= ~kSCNetworkReachabilityFlagsConnectionOnDemand;
			SCPrint(flags != 0, stdout, CFSTR(","));
		}
		if (flags & kSCNetworkReachabilityFlagsInterventionRequired) {
			SCPrint(TRUE, stdout, CFSTR("Intervention Required"));
			flags &= ~kSCNetworkReachabilityFlagsInterventionRequired;
			SCPrint(flags != 0, stdout, CFSTR(","));
		}
		if (flags & kSCNetworkReachabilityFlagsIsLocalAddress) {
			SCPrint(TRUE, stdout, CFSTR("Local Address"));
			flags &= ~kSCNetworkReachabilityFlagsIsLocalAddress;
			SCPrint(flags != 0, stdout, CFSTR(","));
		}
		if (flags & kSCNetworkReachabilityFlagsIsDirect) {
			SCPrint(TRUE, stdout, CFSTR("Directly Reachable Address"));
			flags &= ~kSCNetworkReachabilityFlagsIsDirect;
			SCPrint(flags != 0, stdout, CFSTR(","));
		}
#if	TARGET_OS_IPHONE
		if (flags & kSCNetworkReachabilityFlagsIsWWAN) {
			SCPrint(TRUE, stdout, CFSTR("WWAN"));
			flags &= ~kSCNetworkReachabilityFlagsIsWWAN;
			SCPrint(flags != 0, stdout, CFSTR(","));
		}
#endif	// TARGET_OS_IPHONE
		if (flags != 0) {
			SCPrint(TRUE, stdout, CFSTR("0x%08x"), flags);
		}
	} else {
		SCPrint(TRUE, stdout, CFSTR("Not Reachable"));
	}

	return;
}


__END_DECLS

#endif	// _SCNETWORKREACHABILITYINTERNAL_H

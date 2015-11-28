/*
 * Copyright (c) 2000-2014 Apple Inc.  All Rights Reserved.
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

/*
 * ip_plugin.c
 * - decides which interface will be made the "primary" interface,
 *   that is, the one with the default route assigned
 */

/*
 * Modification History
 *
 * July 19, 2000	Dieter Siegmund (dieter@apple.com)
 * - initial revision
 *
 * November 15, 2000	Dieter Siegmund (dieter@apple.com)
 * - changed to use new configuration model
 *
 * March 19, 2001	Dieter Siegmund (dieter@apple.com)
 * - use service state instead of interface state
 *
 * July 16, 2001	Allan Nathanson (ajn@apple.com)
 * - update to public SystemConfiguration.framework APIs
 *
 * August 28, 2001	Dieter Siegmund (dieter@apple.com)
 * - specify the interface name when installing the default route
 * - this ensures that default traffic goes to the highest priority
 *   service when multiple interfaces are configured to be on the same subnet
 *
 * September 16, 2002	Dieter Siegmund (dieter@apple.com)
 * - don't elect a link-local service to be primary unless it's the only
 *   one that's available
 *
 * July 16, 2003	Dieter Siegmund (dieter@apple.com)
 * - modifications to support IPv6
 * - don't elect a service to be primary if it doesn't have a default route
 *
 * July 29, 2003	Dieter Siegmund (dieter@apple.com)
 * - support installing a default route to a router that's not on our subnet
 *
 * March 22, 2004	Allan Nathanson (ajn@apple.com)
 * - create expanded DNS configuration
 *
 * June 20, 2006	Allan Nathanson (ajn@apple.com)
 * - add SMB configuration
 *
 * December 5, 2007	Dieter Siegmund (dieter@apple.com)
 * - added support for multiple scoped routes
 *
 * November 13, 2013	Dieter Siegmund (dieter@apple.com)
 * - added generic IPv4 routing support
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/route.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <netinet/icmp6.h>
#include <netinet6/in6_var.h>
#include <netinet6/nd6.h>
#include <arpa/inet.h>
#include <sys/sysctl.h>
#include <limits.h>
#include <notify.h>
#include <mach/mach_time.h>
#include <dispatch/dispatch.h>
#include <CommonCrypto/CommonDigest.h>

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCDynamicStoreCopyDHCPInfo.h>
#include <SystemConfiguration/SCValidation.h>
#include <SystemConfiguration/scprefs_observer.h>
#include <SystemConfiguration/SCPrivate.h>	/* for SCLog() */
#include "SCNetworkReachabilityInternal.h"
#include "SCNetworkSignaturePrivate.h"
#include <dnsinfo.h>
#include "dnsinfo_server.h"

#include <ppp/PPPControllerPriv.h>

#include <dns_sd.h>
#ifndef	kDNSServiceCompMulticastDNS
#define	kDNSServiceCompMulticastDNS	"MulticastDNS"
#endif
#ifndef	kDNSServiceCompPrivateDNS
#define	kDNSServiceCompPrivateDNS	"PrivateDNS"
#endif
#include <network_information.h>
#include "network_information_priv.h"
#include "network_information_server.h"
#include <ppp/ppp_msg.h>
#include "ip_plugin.h"
#if	!TARGET_IPHONE_SIMULATOR
#include "set-hostname.h"
#endif	/* !TARGET_IPHONE_SIMULATOR */

#include "dns-configuration.h"
#include "proxy-configuration.h"

#if	!TARGET_OS_IPHONE
#include "smb-configuration.h"
#endif	/* !TARGET_OS_IPHONE */

#define kLoopbackInterface	"lo0"
#define EROUTENOTAPPLIED	1001

typedef CF_ENUM(uint8_t, ProtocolFlags) {
    kProtocolFlagsNone	= 0x0,
    kProtocolFlagsIPv4	= 0x1,
    kProtocolFlagsIPv6	= 0x2
};

enum {
    kDebugFlag1		= 0x00000001,
    kDebugFlag2		= 0x00000002,
    kDebugFlag4		= 0x00000004,
    kDebugFlag8		= 0x00000008,
    kDebugFlagDefault	= kDebugFlag1,
    kDebugFlagAll	= 0xffffffff
};

typedef unsigned int	IFIndex;

#ifndef TEST_ROUTELIST

#define ROUTELIST_DEBUG(flag, fmt, ...)

static struct if_nameindex *	S_if_nameindex_cache;

__private_extern__ IFIndex
my_if_nametoindex(const char * ifname)
{
    IFIndex		idx = 0;
    struct if_nameindex *	scan;

    if (S_if_nameindex_cache == NULL) {
	return (if_nametoindex(ifname));
    }
    for (scan = S_if_nameindex_cache;
	 scan->if_index != 0 && scan->if_name != NULL;
	 scan++) {
	if (strcmp(scan->if_name, ifname) == 0) {
	    idx = scan->if_index;
	    break;
	}
    }
    return (idx);
}

__private_extern__ const char *
my_if_indextoname(IFIndex idx, char if_name[IFNAMSIZ])
{
    const char *		name = NULL;
    struct if_nameindex *	scan;

    if (S_if_nameindex_cache == NULL) {
	return (if_indextoname(idx, if_name));
    }
    for (scan = S_if_nameindex_cache;
	 scan->if_index != 0 && scan->if_name != NULL;
	 scan++) {
	if (scan->if_index == idx) {
	    name = if_name;
	    strlcpy(if_name, scan->if_name, IFNAMSIZ);
	    break;
	}
    }
    return (name);
}

static void
my_if_freenameindex(void)
{
    if (S_if_nameindex_cache != NULL) {
	if_freenameindex(S_if_nameindex_cache);
	S_if_nameindex_cache = NULL;
    }
    return;
}

static void
my_if_nameindex(void)
{
    my_if_freenameindex();
    S_if_nameindex_cache = if_nameindex();
    return;
}


#else /* TEST_ROUTELIST */

#define ROUTELIST_DEBUG(flags, format, ...)	{ if (((S_IPMonitor_debug & (flags)) != 0)) printf((format), ## __VA_ARGS__ ); }


static const char * * 	list;
static int		list_count;
static int		list_size;

__private_extern__ IFIndex
my_if_nametoindex(const char * ifname)
{
    IFIndex		ret;

    if (list == NULL) {
	list_size = 4;
	list_count = 2;
	list = (const char * *)malloc(sizeof(*list) * list_size);
	list[0] = strdup("");
	list[1] = strdup(kLoopbackInterface);
    }
    else {
	int	i;

	for (i = 1; i < list_count; i++) {
	    if (strcmp(list[i], ifname) == 0) {
		ret = i;
		goto done;
	    }
	}
    }
    if (list_count == list_size) {
	list_size += 2;
	list = (const char * *)realloc(list, sizeof(*list) * list_size);
    }
    list[list_count] = strdup(ifname);
    ret = list_count;
    list_count++;
 done:
    return (ret);
}

__private_extern__ const char *
my_if_indextoname(IFIndex idx, char if_name[IFNAMSIZ])
{
    const char *	name = NULL;

    if (idx < list_count) {
	name = if_name;
	strlcpy(if_name, list[idx], IFNAMSIZ);
    }
    return (name);
}

static void
my_if_nameindex(void)
{
}

static void
my_if_freenameindex(void)
{
}

#endif /* TEST_ROUTELIST */

static const char *
my_if_indextoname2(IFIndex ifindex, char ifname[IFNAMSIZ])
{
    if (ifindex == 0) {
	return (NULL);
    }
    if (my_if_indextoname(ifindex, ifname) == NULL) {
	snprintf(ifname, IFNAMSIZ, "[%d]", ifindex);
    }
    return (ifname);
}


static IFIndex
lo0_ifindex(void)
{
    static IFIndex		idx;

    if (idx == 0) {
	idx = my_if_nametoindex(kLoopbackInterface);
    }
    return (idx);
}


/*
 * Property: kServiceOptionRankAssertion
 * Purpose:
 *   Key used in the service options dictionary to hold the RankAssertion
 *   derived from the kSCPropNetServicePrimaryRank string.
 */
#define kServiceOptionRankAssertion	CFSTR("RankAssertion")	/* number */

/*
 * Property: kIPIsCoupled
 * Purpose:
 *   Used to indicate that the IPv4 and IPv6 services are coupled.
 *   Neither the IPv4 part nor the IPv6 part of a coupled service
 *   may become primary if IPv4 or IPv6 is primary for another interface.
 *
 *   For example, if the service over en3 is "coupled" and has IPv6,
 *   and en0 is primary for just IPv4, IPv6 over en3 is not eligible
 *   to become primary for IPv6.
 */
#define kIPIsCoupled	CFSTR("IPIsCoupled")

#define PPP_PREFIX	"ppp"

#define IP_FORMAT	"%d.%d.%d.%d"
#define IP_CH(ip)	((u_char *)(ip))
#define IP_LIST(ip)	IP_CH(ip)[0],IP_CH(ip)[1],IP_CH(ip)[2],IP_CH(ip)[3]

static SCLoggerRef	S_IPMonitor_logger;

static Boolean	S_bundle_logging_verbose;

/*
 * IPv4 Route management
 */

typedef CF_ENUM(uint16_t, RouteFlags) {
    kRouteFlagsIsScoped		= 0x0001,
    kRouteFlagsHasGateway	= 0x0002,
    kRouteFlagsIsHost		= 0x0004,
    kRouteFlagsIsNULL		= 0x0008,
    kRouteFlagsKernelManaged	= 0x0010
};

typedef CF_ENUM(uint16_t, ControlFlags) {
    kControlFlagsProcessed	= 0x0001,
    kControlFlagsAdded		= 0x0002,
};

#define ROUTE_COMMON				\
    int			prefix_length;		\
    IFIndex		ifindex;		\
    IFIndex		exclude_ifindex;	\
    Rank		rank;			\
    RouteFlags		flags;			\
    ControlFlags	control_flags;

typedef struct {
    ROUTE_COMMON
} Route, * RouteRef;

typedef struct {
    ROUTE_COMMON
    struct in_addr	dest;
    struct in_addr	mask;
    struct in_addr	gateway;
    struct in_addr	ifa;
} IPv4Route, * IPv4RouteRef;

typedef struct {
    ROUTE_COMMON
    struct in6_addr	dest;
    struct in6_addr	gateway;
    struct in6_addr	ifa;
} IPv6Route, * IPv6RouteRef;

typedef CF_ENUM(uint16_t, RouteListFlags) {
    kRouteListFlagsExcludeNWI	= 0x0001,
    kRouteListFlagsHasDefault	= 0x0002
};

#define ROUTELIST_COMMON			\
    int			count;			\
    int			size;			\
    RouteListFlags	flags;

typedef struct {
    ROUTELIST_COMMON
} RouteListCommon, * RouteListRef;

typedef struct {
    ROUTELIST_COMMON
    IPv4Route		list[1];	/* variable length */
} IPv4RouteList, * IPv4RouteListRef;

typedef struct {
    ROUTELIST_COMMON
    IPv6Route		list[1];	/* variable length */
} IPv6RouteList, * IPv6RouteListRef;

typedef union {
    void *		ptr;
    RouteListRef	common;
    IPv4RouteListRef	v4;
    IPv6RouteListRef	v6;
} RouteListUnion;

typedef enum {
    kRouteCommandAdd,
    kRouteCommandRemove
} RouteCommand;

/*
 * Election Information
 * - information about the current best services
 */
typedef union {
    struct in_addr	v4;
    struct in6_addr	v6;
} in_addr;

typedef union {
    struct sockaddr_in 	v4;
    struct sockaddr_in6 v6;
} in_sockaddr;

typedef struct Candidate {
    CFStringRef			serviceID;
    CFStringRef			if_name;
    Rank			rank;
    boolean_t			ip_is_coupled;
    SCNetworkReachabilityFlags	reachability_flags;
    in_addr			addr;
    in_sockaddr			vpn_server_addr;
    CFStringRef			signature;
} Candidate, * CandidateRef;

typedef struct ElectionResults {
    int				af;
    int				count;
    int				size;
    Candidate			candidates[1];
} ElectionResults, * ElectionResultsRef;

static __inline__ size_t
ElectionResultsComputeSize(unsigned int n)
{
    return (offsetof(ElectionResults, candidates[n]));
}

/*
 * Type: Rank
 * Purpose:
 *   A 32-bit value to encode the relative rank of a service.
 *
 *   The top 8 bits are used to hold the rank assertion (first, default, last,
 *   never, scoped);
 *
 *   The bottom 24 bits are used to store the service index (i.e. the
 *   position within the service order array).
 */
#define RANK_ASSERTION_MAKE(r)		((Rank)(r) << 24)
#define kRankAssertionFirst		RANK_ASSERTION_MAKE(0)
#define kRankAssertionDefault		RANK_ASSERTION_MAKE(1)
#define kRankAssertionLast		RANK_ASSERTION_MAKE(2)
#define kRankAssertionNever		RANK_ASSERTION_MAKE(3)
#define kRankAssertionScoped		RANK_ASSERTION_MAKE(4)
#define kRankAssertionMask		RANK_ASSERTION_MAKE(0xff)
#define RANK_ASSERTION_MASK(r)		((Rank)(r) & kRankAssertionMask)
#define RANK_ASSERTION_GET(r)		((Rank)(r) >> 24)
#define RANK_INDEX_MAKE(r)		((Rank)(r))
#define kRankIndexMask			RANK_INDEX_MAKE(0xffffff)
#define RANK_INDEX_MASK(r)		((Rank)(r) & kRankIndexMask)

static __inline__ Rank
RankMake(uint32_t service_index, Rank primary_rank)
{
    return (RANK_INDEX_MASK(service_index) | RANK_ASSERTION_MASK(primary_rank));
}

static Rank
InterfaceRankGetRankAssertion(CFNumberRef rank_cf, Boolean * ret_is_set)
{
    SCNetworkServicePrimaryRank if_rank;
    Boolean			is_set = FALSE;
    Rank			rank = kRankAssertionDefault;

    if (rank_cf != NULL
	&& CFNumberGetValue(rank_cf, kCFNumberSInt32Type, &if_rank)
	&& if_rank != kSCNetworkServicePrimaryRankDefault) {
	if (if_rank == kSCNetworkServicePrimaryRankFirst) {
	    rank = kRankAssertionFirst;
	}
	else {
	    rank = RANK_ASSERTION_MAKE(if_rank);
	}
	is_set = TRUE;
    }
    if (ret_is_set != NULL) {
	*ret_is_set = is_set;
    }
    return (rank);
}

static Rank
PrimaryRankGetRankAssertion(CFStringRef rank_str, Boolean * is_set)
{
    int				i;
    struct {
	const CFStringRef *	name;
	Rank			rank_assertion;
    } values[] = {
	{ &kSCValNetServicePrimaryRankFirst, kRankAssertionFirst },
	{ &kSCValNetServicePrimaryRankLast, kRankAssertionLast },
	{ &kSCValNetServicePrimaryRankNever, kRankAssertionNever },
	{ &kSCValNetServicePrimaryRankScoped, kRankAssertionScoped }
    };

    if (rank_str != NULL) {
	for (i = 0; i < countof(values); i++) {
	    if (CFEqual(rank_str, *(values[i].name))) {
		if (is_set != NULL) {
		    *is_set = TRUE;
		}
		return (values[i].rank_assertion);
	    }
	}
    }
    if (is_set != NULL) {
	*is_set = FALSE;
    }
    return (kRankAssertionDefault);
}

/* SCDynamicStore session */
static SCDynamicStoreRef	S_session = NULL;

/* debug output flags */
static uint32_t			S_IPMonitor_debug = 0;
static Boolean			S_IPMonitor_verbose = FALSE;

/* are we netbooted?  If so, don't touch the default route */
static boolean_t		S_netboot = FALSE;

/* is scoped routing enabled? */
static boolean_t		S_scopedroute = FALSE;
static boolean_t		S_scopedroute_v6 = FALSE;

/* dictionary to hold per-service state: key is the serviceID */
static CFMutableDictionaryRef	S_service_state_dict = NULL;
static CFMutableDictionaryRef	S_ipv4_service_rank_dict = NULL;
static CFMutableDictionaryRef	S_ipv6_service_rank_dict = NULL;

/* dictionary to hold per-interface rank information */
static CFDictionaryRef		S_if_rank_dict;

/* if set, a PPP interface overrides the primary */
static boolean_t		S_ppp_override_primary = FALSE;

/* the current primary serviceID's */
static CFStringRef		S_primary_ipv4 = NULL;
static CFStringRef		S_primary_ipv6 = NULL;
static CFStringRef		S_primary_dns = NULL;
static CFStringRef		S_primary_proxies = NULL;

/* the current election results */
static ElectionResultsRef	S_ipv4_results;
static ElectionResultsRef	S_ipv6_results;

static CFStringRef		S_state_global_ipv4 = NULL;
static CFStringRef		S_state_global_ipv6 = NULL;
static CFStringRef		S_state_global_dns = NULL;
static CFStringRef		S_state_global_proxies = NULL;
static CFStringRef		S_state_service_prefix = NULL;
static CFStringRef		S_setup_global_ipv4 = NULL;
static CFStringRef		S_setup_service_prefix = NULL;

static CFStringRef		S_multicast_resolvers = NULL;
static CFStringRef		S_private_resolvers = NULL;

#if	!TARGET_IPHONE_SIMULATOR
static IPv4RouteListRef		S_ipv4_routelist = NULL;
static IPv6RouteListRef		S_ipv6_routelist = NULL;

#endif	/* !TARGET_IPHONE_SIMULATOR */

static boolean_t		S_append_state = FALSE;

static CFDictionaryRef		S_dns_dict = NULL;

static Boolean			S_dnsinfo_synced = TRUE;

static nwi_state_t		S_nwi_state = NULL;
static Boolean			S_nwi_synced = TRUE;

static CFDictionaryRef		S_proxies_dict = NULL;

// Note: access should be gated with __network_change_queue()
static uint32_t			S_network_change_needed = 0;
#define	NETWORK_CHANGE_NET	1<<0
#define	NETWORK_CHANGE_DNS	1<<1
#define	NETWORK_CHANGE_PROXY	1<<2
#if	!TARGET_OS_IPHONE
#define	NETWORK_CHANGE_SMB	1<<3
#endif	/* !TARGET_OS_IPHONE */
static struct timeval		S_network_change_start;
static Boolean			S_network_change_timeout = FALSE;
static dispatch_source_t	S_network_change_timer = NULL;

#if	!TARGET_OS_IPHONE
static CFStringRef		S_primary_smb = NULL;
static CFStringRef		S_state_global_smb = NULL;
static CFDictionaryRef		S_smb_dict = NULL;
#endif	/* !TARGET_OS_IPHONE */

#if	!TARGET_OS_IPHONE
#define VAR_RUN_RESOLV_CONF	"/var/run/resolv.conf"
#endif	/* !TARGET_OS_IPHONE */

#ifndef KERN_NETBOOT
#define KERN_NETBOOT		40	/* int: are we netbooted? 1=yes,0=no */
#endif /* KERN_NETBOOT */

/**
 ** entityType*, GetEntityChanges*
 ** - definitions for the entity types we handle
 **/
typedef enum {
    kEntityTypeIPv4	= 0,
    kEntityTypeIPv6,
    kEntityTypeDNS,
    kEntityTypeProxies,
#if	!TARGET_OS_IPHONE
    kEntityTypeSMB,
#endif	/* !TARGET_OS_IPHONE */
    ENTITY_TYPES_COUNT,
    kEntityTypeTransientStatus,
    kEntityTypeServiceOptions	= 31
} EntityType;

static const CFStringRef *entityTypeNames[ENTITY_TYPES_COUNT] = {
    &kSCEntNetIPv4,	/* 0 */
    &kSCEntNetIPv6,	/* 1 */
    &kSCEntNetDNS,	/* 2 */
    &kSCEntNetProxies,	/* 3 */
#if	!TARGET_OS_IPHONE
    &kSCEntNetSMB,	/* 4 */
#endif	/* !TARGET_OS_IPHONE */
};

static Boolean
S_dict_get_boolean(CFDictionaryRef dict, CFStringRef key, Boolean def_value);

static __inline__ char
ipvx_char(int af)
{
    return ((af == AF_INET) ? '4' : '6');
}

static __inline__ char
ipvx_other_char(int af)
{
    return ((af == AF_INET) ? '6' : '4');
}

/*
 * IPv4/IPv6 Service Dict keys: kIPDictRoutes, IPDictService
 *
 * The IPv4/IPv6 service dictionary contains two sub-dictionaries:
 *   	Routes		CFData containing IPv4RouteList/IPv6RouteList
 *      Service		dictionary containing kSCEntNetIPv[46] service entity
 */
#define kIPDictRoutes 		CFSTR("Routes")		/* data */
#define	kIPDictService		CFSTR("Service")	/* dict */

static CFDictionaryRef
ipdict_create(CFDictionaryRef dict, CFDataRef routes_data)
{
    CFStringRef	keys[2];
    CFTypeRef	values[2];

    keys[0] = kIPDictService;
    values[0] = dict;
    keys[1] = kIPDictRoutes;
    values[1] = routes_data;
    return (CFDictionaryCreate(NULL,
			       (const void * *)keys,
			       values,
			       countof(keys),
			       &kCFTypeDictionaryKeyCallBacks,
			       &kCFTypeDictionaryValueCallBacks));
}

static void *
ipdict_get_routelist(CFDictionaryRef dict)
{
    void * 	routes_list = NULL;

    if (dict != NULL) {
	CFDataRef 	routes;

	routes = CFDictionaryGetValue(dict, kIPDictRoutes);
	if (routes != NULL) {
	    routes_list = (void *)CFDataGetBytePtr(routes);
	}
    }
    return (routes_list);
}

static CFDictionaryRef
ipdict_get_service(CFDictionaryRef dict)
{
    CFDictionaryRef 	ip_dict = NULL;

    if (dict != NULL) {
	ip_dict = CFDictionaryGetValue(dict, kIPDictService);
    }
    return (ip_dict);
}

static CFStringRef
ipdict_get_ifname(CFDictionaryRef dict)
{
    CFStringRef		ifname = NULL;
    CFDictionaryRef 	ip_dict;

    ip_dict = ipdict_get_service(dict);
    if (ip_dict != NULL) {
	ifname = CFDictionaryGetValue(ip_dict, kSCPropInterfaceName);
    }
    return (ifname);
}

typedef boolean_t GetEntityChangesFunc(CFStringRef serviceID,
				       CFDictionaryRef state_dict,
				       CFDictionaryRef setup_dict,
				       CFDictionaryRef info);
typedef GetEntityChangesFunc * GetEntityChangesFuncRef;

static GetEntityChangesFunc get_ipv4_changes;
static GetEntityChangesFunc get_ipv6_changes;
static GetEntityChangesFunc get_dns_changes;
static GetEntityChangesFunc get_proxies_changes;
#if	!TARGET_OS_IPHONE
static GetEntityChangesFunc get_smb_changes;
#endif	/* !TARGET_OS_IPHONE */

static void
my_CFRelease(void * t);

static void
my_CFArrayAppendUniqueValue(CFMutableArrayRef arr, CFTypeRef new);

static void
my_CFArrayRemoveValue(CFMutableArrayRef arr, CFStringRef key);

static const GetEntityChangesFuncRef entityChangeFunc[ENTITY_TYPES_COUNT] = {
    get_ipv4_changes,	/* 0 */
    get_ipv6_changes,	/* 1 */
    get_dns_changes,	/* 2 */
    get_proxies_changes,/* 3 */
#if	!TARGET_OS_IPHONE
    get_smb_changes,	/* 4 */
#endif	/* !TARGET_OS_IPHONE */
};

/**
 ** keyChangeList
 ** - mechanism to do an atomic update of the SCDynamicStore
 **   when the content needs to be changed across multiple functions
 **/
typedef struct {
    CFMutableArrayRef		notify;
    CFMutableArrayRef		remove;
    CFMutableDictionaryRef	set;
} keyChangeList, * keyChangeListRef;

static void
keyChangeListInit(keyChangeListRef keys)
{
    keys->notify = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    keys->remove = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    keys->set = CFDictionaryCreateMutable(NULL, 0,
					  &kCFTypeDictionaryKeyCallBacks,
					  &kCFTypeDictionaryValueCallBacks);
    return;
}

static void
keyChangeListFree(keyChangeListRef keys)
{
    my_CFRelease(&keys->notify);
    my_CFRelease(&keys->remove);
    my_CFRelease(&keys->set);
    return;
}

static Boolean
keyChangeListActive(keyChangeListRef keys)
{
    return ((CFDictionaryGetCount(keys->set) > 0) ||
	    (CFArrayGetCount(keys->remove) > 0) ||
	    (CFArrayGetCount(keys->notify) > 0));
}

static void
keyChangeListNotifyKey(keyChangeListRef keys, CFStringRef key)
{
    my_CFArrayAppendUniqueValue(keys->notify, key);
    return;
}

static void
keyChangeListRemoveValue(keyChangeListRef keys, CFStringRef key)
{
    my_CFArrayAppendUniqueValue(keys->remove, key);
    CFDictionaryRemoveValue(keys->set, key);
    return;
}

static void
keyChangeListSetValue(keyChangeListRef keys, CFStringRef key, CFTypeRef value)
{
    my_CFArrayRemoveValue(keys->remove, key);
    CFDictionarySetValue(keys->set, key, value);
    return;
}

static void
keyChangeListApplyToStore(keyChangeListRef keys, SCDynamicStoreRef session)
{
    CFArrayRef		notify = keys->notify;
    CFArrayRef		remove = keys->remove;
    CFDictionaryRef	set = keys->set;

    if (CFArrayGetCount(notify) == 0) {
	notify = NULL;
    }
    if (CFArrayGetCount(remove) == 0) {
	remove = NULL;
    }
    if (CFDictionaryGetCount(set) == 0) {
	set = NULL;
    }
    if (set == NULL && remove == NULL && notify == NULL) {
	return;
    }
    if (S_IPMonitor_debug & kDebugFlag1) {
	if (set != NULL) {
	    my_log(LOG_DEBUG, "IPMonitor: Setting:\n%@", set);
	}
	if (remove != NULL) {
	    my_log(LOG_DEBUG, "IPMonitor: Removing:\n%@", remove);
	}
	if (notify != NULL) {
	    my_log(LOG_DEBUG, "IPMonitor: Notifying:\n%@", notify);
	}
    }
    (void)SCDynamicStoreSetMultiple(session, set, remove, notify);

    return;
}

static void
S_nwi_ifstate_dump(nwi_ifstate_t ifstate, int i)
{
    const char *		addr_str;
    void *			address;
    char 			ntopbuf[INET6_ADDRSTRLEN];
    char 			vpn_ntopbuf[INET6_ADDRSTRLEN];
    const struct sockaddr * 	vpn_addr;

    address = nwi_ifstate_get_address(ifstate);
    addr_str = inet_ntop(ifstate->af, address, ntopbuf, sizeof(ntopbuf));
    vpn_addr = nwi_ifstate_get_vpn_server(ifstate);
    if (vpn_addr != NULL) {
	_SC_sockaddr_to_string(nwi_ifstate_get_vpn_server(ifstate),
			       vpn_ntopbuf,
			       sizeof(vpn_ntopbuf));
    }
    my_log(LOG_DEBUG,
	   "    [%d]: %s%s%s%s rank 0x%x iaddr %s%s%s reach_flags 0x%x",
	   i, ifstate->ifname,
	   ifstate->diff_str != NULL ? ifstate->diff_str : "",
	   (ifstate->flags & NWI_IFSTATE_FLAGS_HAS_DNS) != 0
	   ? " dns" : "",
	   (ifstate->flags & NWI_IFSTATE_FLAGS_NOT_IN_LIST) != 0
	   ? " never" : "",
	   ifstate->rank,
	   addr_str,
	   (vpn_addr != NULL) ? " vpn_server_addr: " : "",
	   (vpn_addr != NULL) ? vpn_ntopbuf : "",
	   ifstate->reach_flags);
    return;
}

static void
S_nwi_state_dump(nwi_state_t state)
{
    int			i;
    nwi_ifstate_t 	scan;

    if (state == NULL) {
	my_log(LOG_DEBUG, "nwi_state = <none>");
	return;
    }
    my_log(LOG_DEBUG,
	   "nwi_state = { "
	   "gen=%llu size=%u #v4=%u #v6=%u "
	   "reach_flags=(v4=0x%x, v6=0x%x) }",
	   state->generation_count,
	   state->size,
	   state->ipv4_count,
	   state->ipv6_count,
	   nwi_state_get_reachability_flags(state, AF_INET),
	   nwi_state_get_reachability_flags(state, AF_INET6));
    if (state->ipv4_count) {
	my_log(LOG_DEBUG, "IPv4:");
	for (i = 0, scan = state->nwi_ifstates;
	     i < state->ipv4_count; i++, scan++) {
	    S_nwi_ifstate_dump(scan, i);
	}
    }
    if (state->ipv6_count) {
	my_log(LOG_DEBUG, "IPv6:");
	for (i = 0, scan = state->nwi_ifstates + state->ipv6_start;
	     i < state->ipv6_count; i++, scan++) {
	    S_nwi_ifstate_dump(scan, i);
	}
    }
    return;
}

static boolean_t
S_is_network_boot()
{
    int mib[2];
    size_t len;
    int netboot = 0;

    mib[0] = CTL_KERN;
    mib[1] = KERN_NETBOOT;
    len = sizeof(netboot);
    sysctl(mib, 2, &netboot, &len, NULL, 0);
    return (netboot);
}

static int	rtm_seq = 0;

#if	!TARGET_IPHONE_SIMULATOR
static int
open_routing_socket(void)
{
    int sockfd;

    if ((sockfd = socket(PF_ROUTE, SOCK_RAW, PF_ROUTE)) == -1) {
	my_log(LOG_NOTICE,
	       "IPMonitor: open_routing_socket: socket failed, %s",
	       strerror(errno));
    }
    return (sockfd);
}

static __inline__ int
inet6_dgram_socket()
{
    return (socket(AF_INET6, SOCK_DGRAM, 0));
}

static int
siocdradd_in6(int s, int if_index, const struct in6_addr * addr, u_char flags)
{
    struct in6_defrouter	dr;
    struct sockaddr_in6 *	sin6;

    bzero(&dr, sizeof(dr));
    sin6 = &dr.rtaddr;
    sin6->sin6_len = sizeof(struct sockaddr_in6);
    sin6->sin6_family = AF_INET6;
    sin6->sin6_addr = *addr;
    dr.flags = flags;
    dr.if_index = if_index;
    return (ioctl(s, SIOCDRADD_IN6, &dr));
}

static int
siocdrdel_in6(int s, int if_index, const struct in6_addr * addr)
{
    struct in6_defrouter	dr;
    struct sockaddr_in6 *	sin6;

    bzero(&dr, sizeof(dr));
    sin6 = &dr.rtaddr;
    sin6->sin6_len = sizeof(struct sockaddr_in6);
    sin6->sin6_family = AF_INET6;
    sin6->sin6_addr = *addr;
    dr.if_index = if_index;
    return (ioctl(s, SIOCDRDEL_IN6, &dr));
}

#endif	/* !TARGET_IPHONE_SIMULATOR */

static boolean_t
S_is_scoped_routing_enabled()
{
    int	    scopedroute	= 0;
    size_t  len		= sizeof(scopedroute);

    if ((sysctlbyname("net.inet.ip.scopedroute",
		      &scopedroute, &len,
		      NULL, 0) == -1)
	&& (errno != ENOENT)) {
	my_log(LOG_ERR, "sysctlbyname() failed: %s", strerror(errno));
    }
    return (scopedroute);
}

static boolean_t
S_is_scoped_v6_routing_enabled()
{
    int	    scopedroute_v6	= 0;
    size_t  len			= sizeof(scopedroute_v6);

    if ((sysctlbyname("net.inet6.ip6.scopedroute",
		      &scopedroute_v6, &len,
		      NULL, 0) == -1)
	&& (errno != ENOENT)) {
	my_log(LOG_ERR, "sysctlbyname() failed: %s", strerror(errno));
    }
    return (scopedroute_v6);
}

static void
my_CFArrayAppendUniqueValue(CFMutableArrayRef arr, CFTypeRef new)
{
    CFIndex	n = CFArrayGetCount(arr);

    if (CFArrayContainsValue(arr, CFRangeMake(0, n), new)) {
	return;
    }
    CFArrayAppendValue(arr, new);
    return;
}

static void
my_CFArrayRemoveValue(CFMutableArrayRef arr, CFStringRef key)
{
    CFIndex	i;

    i = CFArrayGetFirstIndexOfValue(arr,
				    CFRangeMake(0, CFArrayGetCount(arr)),
				    key);
    if (i != kCFNotFound) {
	CFArrayRemoveValueAtIndex(arr, i);
    }
    return;
}

static CFArrayRef
my_CFArrayCreateCombinedArray(CFArrayRef array1, CFArrayRef array2)
{
    CFMutableArrayRef	combined;

    combined = CFArrayCreateMutableCopy(NULL, 0, array1);
    CFArrayAppendArray(combined,
		       array2,
		       CFRangeMake(0, CFArrayGetCount(array2)));
    return (combined);
}

static void
my_CFRelease(void * t)
{
    void * * obj = (void * *)t;

    if (obj && *obj) {
	CFRelease(*obj);
	*obj = NULL;
    }
    return;
}

static CFDictionaryRef
my_CFDictionaryGetDictionary(CFDictionaryRef dict, CFStringRef key)
{
    if (isA_CFDictionary(dict) == NULL) {
	return (NULL);
    }
    return (isA_CFDictionary(CFDictionaryGetValue(dict, key)));
}

static CFArrayRef
my_CFDictionaryGetArray(CFDictionaryRef dict, CFStringRef key)
{
    if (isA_CFDictionary(dict) == NULL) {
	return (NULL);
    }
    return (isA_CFArray(CFDictionaryGetValue(dict, key)));
}

static boolean_t
cfstring_to_ipvx(int family, CFStringRef str, void * addr, int addr_size)
{
    char        buf[128];

    if (isA_CFString(str) == NULL) {
	goto done;
    }

    switch (family) {
    case AF_INET:
	if (addr_size < sizeof(struct in_addr)) {
	    goto done;
	}
	break;
    case AF_INET6:
	if (addr_size < sizeof(struct in6_addr)) {
	    goto done;
	}
	break;
    default:
	goto done;
    }
    (void)_SC_cfstring_to_cstring(str, buf, sizeof(buf), kCFStringEncodingASCII);
    if (inet_pton(family, buf, addr) == 1) {
	return (TRUE);
    }
 done:
    bzero(addr, addr_size);
    return (FALSE);
}

__private_extern__
boolean_t
cfstring_to_ip(CFStringRef str, struct in_addr * ip_p)
{
    return (cfstring_to_ipvx(AF_INET, str, ip_p, sizeof(*ip_p)));
}

__private_extern__
boolean_t
cfstring_to_ip6(CFStringRef str, struct in6_addr * ip6_p)
{
    return (cfstring_to_ipvx(AF_INET6, str, ip6_p, sizeof(*ip6_p)));
}

static boolean_t
cfnumber_to_int(CFNumberRef num, int * int_val)
{
    if (isA_CFNumber(num) == NULL) {
	return (FALSE);
    }
    return (CFNumberGetValue(num, kCFNumberIntType, int_val));
}

static CF_RETURNS_RETAINED CFStringRef
setup_service_key(CFStringRef serviceID, CFStringRef entity)
{
    return (SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							kSCDynamicStoreDomainSetup,
							serviceID,
							entity));
}

static CF_RETURNS_RETAINED CFStringRef
state_service_key(CFStringRef serviceID, CFStringRef entity)
{
    return (SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							kSCDynamicStoreDomainState,
							serviceID,
							entity));
}

static CFStringRef
interface_entity_key_copy(CFStringRef ifname, CFStringRef entity)
{
    return (SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
							  kSCDynamicStoreDomainState,
							  ifname,
							  entity));
}

static CFDictionaryRef
get_service_setup_entity(CFDictionaryRef services_info, CFStringRef serviceID,
			 CFStringRef entity)
{
    CFStringRef		setup_key;
    CFDictionaryRef	setup_dict;

    setup_key = setup_service_key(serviceID, entity);
    setup_dict = my_CFDictionaryGetDictionary(services_info, setup_key);
    my_CFRelease(&setup_key);
    return (setup_dict);
}

static CFDictionaryRef
get_service_state_entity(CFDictionaryRef services_info, CFStringRef serviceID,
			 CFStringRef entity)
{
    CFStringRef		state_key;
    CFDictionaryRef	state_dict;

    state_key = state_service_key(serviceID, entity);
    state_dict = my_CFDictionaryGetDictionary(services_info, state_key);
    my_CFRelease(&state_key);
    return (state_dict);
}

static boolean_t
dict_get_first_ip(CFDictionaryRef dict, CFStringRef prop, struct in_addr * ip_p)
{
    CFArrayRef		ip_list;

    ip_list = CFDictionaryGetValue(dict, prop);
    if (isA_CFArray(ip_list) != NULL
	&& CFArrayGetCount(ip_list) > 0
	&& cfstring_to_ip(CFArrayGetValueAtIndex(ip_list, 0), ip_p)) {
	return (TRUE);
    }
    return (FALSE);
}

static boolean_t
dict_get_first_ipv6(CFDictionaryRef dict, CFStringRef prop,
		    struct in6_addr * ip_p)
{
    CFArrayRef		ip_list;

    ip_list = CFDictionaryGetValue(dict, prop);
    if (isA_CFArray(ip_list) != NULL
	&& CFArrayGetCount(ip_list) > 0
	&& cfstring_to_ip6(CFArrayGetValueAtIndex(ip_list, 0), ip_p)) {
	return (TRUE);
    }
    return (FALSE);
}

static boolean_t
dict_get_first_int(CFDictionaryRef dict, CFStringRef prop,
		   int * val)
{
    CFArrayRef		list;

    list = CFDictionaryGetValue(dict, prop);
    if (isA_CFArray(list) != NULL
	&& CFArrayGetCount(list) > 0
	&& cfnumber_to_int(CFArrayGetValueAtIndex(list, 0), val)) {
	return (TRUE);
    }
    return (FALSE);
}

static boolean_t
dict_get_ip(CFDictionaryRef dict, CFStringRef prop, struct in_addr * ip_p)
{
    CFStringRef		val;

    val = CFDictionaryGetValue(dict, prop);
    return (cfstring_to_ip(val, ip_p));
}

static boolean_t
dict_get_ipv6(CFDictionaryRef dict, CFStringRef prop, struct in6_addr * ip_p)
{
    CFStringRef		val;

    val = CFDictionaryGetValue(dict, prop);
    return (cfstring_to_ip6(val, ip_p));
}

static boolean_t
dict_get_int(CFDictionaryRef dict, CFStringRef prop, int * intval)
{
    CFNumberRef		val;

    val = CFDictionaryGetValue(dict, prop);
    return (cfnumber_to_int(val, intval));
}

static boolean_t
get_override_primary(CFDictionaryRef dict)
{
    CFTypeRef	override;

    override = CFDictionaryGetValue(dict, kSCPropNetOverridePrimary);
    if (isA_CFNumber(override) != NULL) {
	int	val = 0;

	CFNumberGetValue((CFNumberRef)override, kCFNumberIntType, &val);
	if (val != 0) {
	    return (TRUE);
	}
    }
    else if (isA_CFBoolean(override) != NULL) {
	if (CFBooleanGetValue(override)) {
	    return (TRUE);
	}
    }
    return (FALSE);
}

/**
 ** Route*
 **/

typedef size_t
(*RouteListComputeSize)(CFIndex n);

typedef boolean_t
(*RouteIsEqual)(RouteRef a, RouteRef b);

typedef int
(*RouteApply)(RouteRef route, int cmd, int sockfd);

typedef const void *
(*RouteGateway)(RouteRef route);

typedef void
(*RouteSetGateway)(RouteRef route, const void * address);

typedef const void *
(*RouteDestination)(RouteRef route);

typedef boolean_t
(*RouteSameSubnet)(RouteRef route, const void * address);

typedef CFStringRef
(*RouteCopyDescription)(RouteRef route);

typedef void
(*RouteLog)(int priority, RouteRef route, const char * msg);

typedef struct {
    RouteListComputeSize	list_compute_size;

    RouteIsEqual		route_equal;
    RouteApply			route_apply;
    RouteGateway		route_gateway;
    RouteSetGateway		route_set_gateway;
    RouteDestination		route_destination;
    RouteSameSubnet		route_same_subnet;
    RouteLog			route_log;
    RouteCopyDescription	route_copy_description;

    int				element_size;
    int				address_size;
    int				all_bits_set;
} RouteListInfo;

typedef const RouteListInfo * RouteListInfoRef;

typedef struct {
    RouteListInfoRef	info;
    RouteListRef 	old_routes;
    RouteListRef 	new_routes;
    int			sockfd;
    int			depth;
} RouteListApplyContext, * RouteListApplyContextRef;


static int
RouteAddressCompare(RouteListInfoRef info,
		    const void * addr1,
		    const void * addr2)
{
    return (memcmp(addr1, addr2, info->address_size));
}

static int
RouteCompare(RouteListInfoRef info,
	     RouteRef a, Rank a_rank,
	     RouteRef b, Rank b_rank, boolean_t * same_dest)
{
    int				cmp;
    RouteDestination		route_destination;
    RouteCopyDescription	route_copy_description;

    *same_dest = FALSE;
    route_destination = info->route_destination;
    route_copy_description = info->route_copy_description;
    cmp = RouteAddressCompare(info,
			      (*route_destination)(a),
			      (*route_destination)(b));
    if (cmp == 0) {
	cmp = a->prefix_length - b->prefix_length;
	if (cmp == 0) {
	    int		index_cmp = a->ifindex - b->ifindex;

	    if (index_cmp == 0) {
		cmp = 0;
	    }
	    else if ((a->ifindex == 0 || b->ifindex == 0)
		     && (a->flags & kRouteFlagsIsScoped) == 0
		     && (b->flags & kRouteFlagsIsScoped) == 0) {
		/*
		 * Either of the routes specifies no interface and neither
		 * route is scoped. Claim they are equal to eliminate the
		 * duplicate route.
		 */
		cmp = 0;
	    }
	    else {
		*same_dest = TRUE;
		cmp = RankCompare(a_rank, b_rank);
		if (cmp == 0) {
		    cmp = index_cmp;
		}
	    }
	}
    }
    if ((S_IPMonitor_debug & kDebugFlag8) != 0) {
	CFStringRef	a_str;
	CFStringRef	b_str;
	char		ch;

	if (cmp < 0) {
	    ch = '<';
	}
	else if (cmp == 0) {
	    ch = '=';
	}
	else {
	    ch = '>';
	}
	a_str = (*route_copy_description)(a);
	b_str = (*route_copy_description)(b);
	my_log(LOG_DEBUG, "%@ rank 0x%x %c %@ rank 0x%x",
	       a_str, a_rank, ch, b_str, b_rank);
	CFRelease(a_str);
	CFRelease(b_str);
    }
    return (cmp);
}

static RouteRef
RouteListGetRouteAtIndexSimple(RouteListInfoRef info, RouteListRef routes,
			       CFIndex where)
{
    return ((void *)routes + (*info->list_compute_size)(where));
}

static RouteRef
RouteListGetRouteAtIndex(RouteListInfoRef info, RouteListRef routes,
			 CFIndex where)
{
    if (routes->count == 0
	|| where >= routes->count) {
	return (NULL);
    }
    return (RouteListGetRouteAtIndexSimple(info, routes, where));
}

static RouteRef
RouteListGetFirstRoute(RouteListInfoRef info, RouteListRef routes)
{
    return (RouteListGetRouteAtIndexSimple(info, routes, 0));
}

#if	!TARGET_IPHONE_SIMULATOR
static CFIndex
RouteListRouteIndex(RouteListInfoRef info, RouteListRef routes,
		    RouteRef route)
{
    return (((void *)route
	     - (void *)RouteListGetFirstRoute(info, routes))
	    / info->element_size);
}
#endif /* !TARGET_IPHONE_SIMULATOR */

static RouteRef
RouteGetNextRoute(RouteListInfoRef info, RouteRef route)
{
    return ((RouteRef)(((void *)route) + info->element_size));
}

static RouteRef
RouteListAddRouteAtIndex(RouteListInfoRef info, RouteListRef routes,
			 RouteRef this_route, CFIndex where)
{
    RouteRef	insert_route;

    if (where == kCFNotFound) {
	/* add it to the end */
	insert_route
	    = RouteListGetRouteAtIndexSimple(info, routes, routes->count);
    }
    else {
	/* make space at [where] */
	insert_route = RouteListGetRouteAtIndexSimple(info, routes, where);
	bcopy(insert_route,
	      (void *)insert_route + info->element_size,
	      info->element_size * (routes->count - where));
    }
    /* copy the route */
    bcopy(this_route, insert_route, info->element_size);
    routes->count++;
    return (insert_route);
}

static void
RouteListRemoveRouteAtIndex(RouteListInfoRef info, RouteListRef routes,
			    CFIndex where)
{
    if (routes->count == 0
	|| where >= routes->count) {
	return;
    }
    routes->count--;
    if (where == routes->count) {
	/* last slot, decrementing gets rid of it */
    }
    else {
	RouteRef	remove_route;

	remove_route = RouteListGetRouteAtIndexSimple(info, routes, where);
	bcopy((void *)remove_route + info->element_size,
	      remove_route,
	      info->element_size * (routes->count - where));
    }
    return;
}

/*
 * Function: RouteListAddRoute
 *
 * Purpose:
 *   Add the given route to the list of routes, eliminating lower-ranked
 *   duplicates on the same interface, and marking any lower ranked duplicates
 *   on other interfaces with kRouteFlagsIsScoped.
 *
 *   This routine assumes that if routes is not NULL, it is malloc'd memory.
 *
 * Returns:
 *   Route list updated with the given route, possibly a different pointer,
 *   due to using realloc'd memory.
 */

typedef enum {
    kScopeNone	= 0,
    kScopeThis	= 1,
    kScopeNext	= 2
} Scope;

static RouteListRef
RouteListAddRoute(RouteListInfoRef info,
		  RouteListRef routes, int init_size,
		  RouteRef this_route, Rank this_rank)
{
    CFIndex		i;
    RouteRef		first_scan = NULL;
    RouteFlags		flags;
    RouteRef		scan;
    Scope		scope_which = kScopeNone;
    CFIndex		where = kCFNotFound;

    if (routes == NULL) {
	size_t	alloc_size = (*info->list_compute_size)(init_size);

	routes = (RouteListRef)malloc(alloc_size);
	bzero(routes, sizeof(*routes));
	routes->size = init_size;
    }
    for (i = 0, scan = RouteListGetFirstRoute(info, routes);
	 i < routes->count;
	 i++, scan = RouteGetNextRoute(info, scan)) {
	int		cmp;
	boolean_t	same_dest;

	cmp = RouteCompare(info, this_route, this_rank, scan, scan->rank,
			   &same_dest);
	if (same_dest == TRUE && first_scan == NULL) {
	    first_scan = scan;
	}
	if (cmp < 0) {
	    if (where == kCFNotFound) {
		if (same_dest == TRUE
		    && (first_scan->flags & kRouteFlagsIsScoped) == 0) {
		    if ((scan->flags & kRouteFlagsIsScoped) != 0) {
			ROUTELIST_DEBUG(kDebugFlag8,
					"Hit 1: set scope on self\n");
			scope_which = kScopeThis;
		    }
		    else {
			ROUTELIST_DEBUG(kDebugFlag8,
					"Hit 2: set scope on next\n");
			scope_which = kScopeNext;
		    }
		}
		/* remember our insertion point, but keep going to find a dup */
		where = i;
	    }
	}
	else if (cmp == 0) {
	    /* exact match */
	    /* exact match */
	    if (where != kCFNotFound
		&& scan->ifindex == this_route->ifindex
		&& scan->exclude_ifindex == 0
		&& this_route->exclude_ifindex == 0) {
		/* this route is a duplicate */
		ROUTELIST_DEBUG(kDebugFlag8, "Hit 3: removing [%ld]\n", i);
		RouteListRemoveRouteAtIndex(info, routes, i);
		break;
	    }
	    /*
	     * this_route is "better" than scan if this_route is not excluded
	     * and scan is excluded or this_route sorts ahead of scan
	     */
	    if (this_route->exclude_ifindex == 0
		&& (scan->exclude_ifindex != 0 || this_rank < scan->rank)) {
		IFIndex		ifindex = 0;
		boolean_t	is_scoped = FALSE;

		if (scan->flags & kRouteFlagsIsScoped) {
		    is_scoped = TRUE;
		}
		if (this_rank < scan->rank) {
		    ROUTELIST_DEBUG(kDebugFlag8,
				    "Hit 4a: replacing [%ld]"
				    " rank 0x%x < 0x%x\n",
				    i, this_rank, scan->rank);
		}
		else {
		    ROUTELIST_DEBUG(kDebugFlag8,
				    "Hit 4b: replacing [%ld] excluded route\n",
				    i);
		}
		if (scan->ifindex != 0) {
		    ifindex = scan->ifindex;
		}
		else if (this_route->ifindex != 0) {
		    ifindex = this_route->ifindex;
		}
		bcopy(this_route, scan, info->element_size);
		scan->rank = this_rank;
		scan->ifindex = ifindex;
		scan->exclude_ifindex = 0;
		if (is_scoped) {
		    /* preserve whether route was scoped */
		    ROUTELIST_DEBUG(kDebugFlag8, "Hit 5: preserved scope\n");
		    scan->flags |= kRouteFlagsIsScoped;
		}
	    }
	    /* we're done */
	    goto done;
	}
	else {
	    if (same_dest == TRUE) {
		if (scope_which == kScopeNone) {
		    ROUTELIST_DEBUG(kDebugFlag8, "Hit 6: set scope on self\n");
		    scope_which = kScopeThis;
		}
	    }
#ifdef TEST_ROUTELIST
	    else if (where != kCFNotFound) {
		/* not possible because we maintain a sorted list */
		fprintf(stderr,
			"Hit 7: moved past routes - can't happen\n");
		exit(2);
		break;
	    }
#endif /* TEST_ROUTELIST */
	}
    }

    if (routes->size == routes->count) {
	int		how_many;
	RouteListRef	new_routes;
	int		old_size;

	/* double the size */
	old_size = routes->size;
	how_many = old_size * 2;
	new_routes = (RouteListRef)
	    reallocf(routes, (*info->list_compute_size)(how_many));
	if (new_routes == NULL) {
	    /* no memory */
	    routes = NULL;
	    goto done;
	}
	ROUTELIST_DEBUG(kDebugFlag8, "increasing size from %d to %d\n",
			old_size, how_many);
	new_routes->size = how_many;
	routes = new_routes;
    }

    /* add/insert the new route */
    this_route = RouteListAddRouteAtIndex(info, routes, this_route, where);
    this_route->rank = this_rank;
    flags = 0;
    if (RANK_ASSERTION_MASK(this_rank) == kRankAssertionNever) {
	flags |= kRouteFlagsIsScoped;
    }
    switch (scope_which) {
    case kScopeThis:
	flags |= kRouteFlagsIsScoped;
	break;
    case kScopeNext:
	this_route = RouteListGetRouteAtIndex(info, routes, where + 1);
	flags |= kRouteFlagsIsScoped;
	break;
    default:
    case kScopeNone:
	break;
    }
    if (this_route != NULL && flags != 0) {
	this_route->flags |= flags;
    }

 done:
    return (routes);
}

/*
 * Function: RouteListAddRouteList
 * Purpose:
 *   Invoke RouteListAddRoute for each route in the given list
 *   'service_routes' combining them into a combined list 'routes'.
 *
 * Returns:
 *   See RouteListAddRoute for more information.
 */
static RouteListRef
RouteListAddRouteList(RouteListInfoRef info,
		      RouteListRef routes, int init_size,
		      RouteListRef service_routes, Rank rank)
{
    int		i;
    RouteRef	scan;

    for (i = 0, scan = RouteListGetFirstRoute(info, service_routes);
	 i < service_routes->count;
	 i++, scan = RouteGetNextRoute(info, scan)) {
	Rank	this_rank;

	if (i == 0
	    && (service_routes->flags & kRouteListFlagsHasDefault) != 0) {
	    /* only apply rank to first element of the list (default route) */
	    this_rank = rank;
	}
	else {
	    this_rank = RANK_INDEX_MASK(rank) | RANK_ASSERTION_MASK(scan->rank);
	}
	routes = RouteListAddRoute(info, routes, init_size, scan, this_rank);
    }
    return (routes);
}

static void
RouteAddInterfaceToDescription(RouteRef r, CFMutableStringRef str)
{
    char	if_name[IFNAMSIZ];

    if (my_if_indextoname2(r->ifindex, if_name) != NULL) {
	CFStringAppendFormat(str, NULL,
			     CFSTR(" Ifp %s"),
			     if_name);
    }
    if (my_if_indextoname2(r->exclude_ifindex, if_name) != NULL) {
	CFStringAppendFormat(str, NULL,
			     CFSTR(" !Ifp %s"),
			     if_name);
    }
    return;
}

static void
RouteAddFlagsToDescription(RouteRef r, CFMutableStringRef str)
{
    if ((r->flags & kRouteFlagsIsNULL) != 0) {
	CFStringAppend(str, CFSTR(" [null]"));
    }
    else {
	Rank 	rank_assertion = RANK_ASSERTION_MASK(r->rank);

	switch (rank_assertion) {
	case kRankAssertionFirst:
	    CFStringAppend(str, CFSTR(" [first]"));
	    break;
	case kRankAssertionLast:
	    CFStringAppend(str, CFSTR(" [last]"));
	    break;
	case kRankAssertionNever:
	    CFStringAppend(str, CFSTR(" [never]"));
	    break;
	default:
	    break;
	}
	if ((r->flags & kRouteFlagsKernelManaged) != 0) {
	    CFStringAppend(str, CFSTR(" [kern]"));
	}
	if ((r->flags & kRouteFlagsIsScoped) != 0) {
	    CFStringAppend(str, CFSTR(" [SCOPED]"));
	}
    }
    return;
}

#if	!TARGET_IPHONE_SIMULATOR
static RouteRef
RouteListFindRoute(RouteListInfoRef info, RouteListRef routes, RouteRef route)
{
    int		i;
    RouteRef	match = NULL;
    RouteRef	scan;

    for (i = 0, scan = RouteListGetFirstRoute(info, routes);
	 i < routes->count;
	 i++, scan = RouteGetNextRoute(info, scan)) {
	if ((*info->route_equal)(scan, route)) {
	    match = scan;
	    break;
	}

    }
    return (match);
}

typedef enum {
    kRouteLookupFlagsNone = 0x0,
    kRouteLookupFlagsExcludeInterface = 0x1
} RouteLookupFlags;

static RouteRef
RouteListLookup(RouteListInfoRef info,
		RouteListRef routes,
		const void * address,
		int n_bits,
		IFIndex ifindex,
		RouteLookupFlags lookup_flags)
{
    RouteRef	best_match = NULL;
    RouteRef	candidate;
    int		i;

    for (i = 0, candidate = RouteListGetFirstRoute(info, routes);
	 i < routes->count;
	 i++, candidate = RouteGetNextRoute(info, candidate)) {
	if (candidate->ifindex == 0 || candidate->exclude_ifindex != 0) {
	    /* ignore exclude routes */
	    continue;
	}
	if ((lookup_flags & kRouteLookupFlagsExcludeInterface) != 0) {
	    /* exclude interfaces with the same interface index */
	    if (ifindex == candidate->ifindex) {
		continue;
	    }
	}
	else if (ifindex != candidate->ifindex) {
	    continue;
	}
	if ((candidate->flags & kRouteFlagsHasGateway) != 0
	    && RouteAddressCompare(info,
				   (*info->route_gateway)(candidate),
				   address) == 0) {
	    /* skip route whose gateway is the address we're looking for */
	    continue;
	}
	if ((candidate->flags & kRouteFlagsIsHost) != 0) {
	    /* if host route and we're looking for an exact match */
	    if (n_bits == info->all_bits_set
		&& RouteAddressCompare(info,
				       (*info->route_destination)(candidate),
				       address) == 0) {
		/* found exact match */
		best_match = candidate;
		break;
	    }
	    /* skip it */
	    continue;
	}
	/* verify that address is on the same subnet */
	if ((*info->route_same_subnet)(candidate, address) == FALSE) {
	    /* different subnet */
	    continue;
	}

	if (candidate->prefix_length == n_bits) {
	    /* exact match */
	    best_match = candidate;
	    break;
	}
	if (candidate->prefix_length > n_bits) {
	    /* matched too many bits */
	    continue;
	}
	if (best_match == NULL
	    || candidate->prefix_length > best_match->prefix_length) {
	    best_match = candidate;
	}
    }
    return (best_match);
}


/*
 * Function: RouteProcess
 * Purpose:
 *   Function to process adding or removing the specified route.
 *   In the case of adding, that may involve first processing the gateway
 *   route (recursively).
 */
static boolean_t
RouteProcess(RouteRef route,
	     RouteCommand cmd,
	     RouteListApplyContextRef context)
{
    RouteLog		route_log = context->info->route_log;
    RouteApply		route_apply = context->info->route_apply;
    RouteGateway	route_gateway = context->info->route_gateway;
    int			retval;

    switch (cmd) {
    case kRouteCommandAdd:
	if ((route->control_flags & kControlFlagsProcessed) != 0) {
	    return ((route->control_flags & kControlFlagsAdded) != 0);
	}
	route->control_flags |= kControlFlagsProcessed;
	if ((route->flags & kRouteFlagsHasGateway) != 0) {
	    boolean_t		added;
	    RouteRef		gateway_route;

	    gateway_route
		= RouteListLookup(context->info,
				  context->new_routes,
				  (*route_gateway)(route),
				  context->info->all_bits_set,
				  route->ifindex,
				  kRouteLookupFlagsNone);
	    if (gateway_route == NULL) {
		(*route_log)(LOG_NOTICE, route,
			     "IPMonitor RouteProcess: no gateway route");
	    }
	    else {
#define MAX_RECURSE_DEPTH	10
		/* avoid infinite recursion */
		if (context->depth == MAX_RECURSE_DEPTH) {
		    (*route_log)(LOG_NOTICE, route,
				 "IPMonitor RouteProcess: "
				 "routing loop detected, not adding");
		    return (FALSE);
		}
		/* recurse to add gateway route */
		context->depth++;
		added = RouteProcess(gateway_route,
				     kRouteCommandAdd,
				     context);
		context->depth--;
		if (added == FALSE) {
		    (*route_log)(LOG_NOTICE, route,
				 "IPMonitor RouteProcess: failed to add");
		    return (FALSE);
		}
	    }
	}
	retval = (*route_apply)(route, RTM_ADD, context->sockfd);
	if (retval == EEXIST) {
	    /* delete and add again */
	    (void)(*route_apply)(route, RTM_DELETE, context->sockfd);
	    retval = (*route_apply)(route, RTM_ADD, context->sockfd);
	}
	switch (retval) {
	default:
	    my_log(LOG_NOTICE,
		   "IPMonitor RouteProcess failed to add route, %s:",
		   strerror(retval));
	    (*route_log)(LOG_NOTICE, route, NULL);
	    break;
	case 0:
	case EROUTENOTAPPLIED:
	    if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
		char		buf[64];
		const char *	str;

		str = (retval == EROUTENOTAPPLIED) ? "!" : "";
		snprintf(buf, sizeof(buf), "%sAdd new[%ld]",
			 str,
			 RouteListRouteIndex(context->info,
					     context->new_routes,
					     route));
		(*route_log)(LOG_DEBUG, route, buf);
	    }
	    route->control_flags |= kControlFlagsAdded;
	    break;
	}
	break;
    case kRouteCommandRemove:
	retval = (*route_apply)(route, RTM_DELETE, context->sockfd);
	switch (retval) {
	case 0:
	case ESRCH:
	case EROUTENOTAPPLIED:
	    if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
		char		buf[64];
		const char *	str;

		str = (retval == EROUTENOTAPPLIED) ? "!" : "";
		snprintf(buf, sizeof(buf), "%sRemove old[%ld]%s",
			 str,
			 RouteListRouteIndex(context->info,
					     context->old_routes,
					     route),
			 (retval == ESRCH) ? "(ESRCH)" : "");
		(*route_log)(LOG_DEBUG, route, buf);
	    }
	    break;
	default:
	    my_log(LOG_NOTICE,
		   "IPMonitor RouteProcess failed to remove"
		   " route, %s", strerror(retval));
	    (*route_log)(LOG_NOTICE, route, NULL);
	    break;
	}
	break;
    default:
	break;
    }
    return (TRUE);
}

static void
RouteListApply(RouteListInfoRef info,
	       RouteListRef old_routes, RouteListRef new_routes,
	       int sockfd)
{
    RouteListApplyContext	context;
    int				i;
    RouteRef			scan;

    if (old_routes == new_routes && old_routes == NULL) {
	/* both old and new are NULL, so there's nothing to do */
	return;
    }
    bzero(&context, sizeof(context));
    context.old_routes = old_routes;
    context.new_routes = new_routes;
    context.sockfd = sockfd;
    context.info = info;
    if (old_routes != NULL) {
	for (i = 0, scan = RouteListGetFirstRoute(info, old_routes);
	     i < old_routes->count;
	     i++, scan = RouteGetNextRoute(info, scan)) {
	    RouteRef	new_route = NULL;

	    if (new_routes != NULL) {
		new_route = RouteListFindRoute(info, new_routes, scan);
	    }
	    if (new_route == NULL) {
		if ((scan->control_flags & kControlFlagsAdded) != 0) {
		    RouteProcess(scan, kRouteCommandRemove, &context);
		}
	    }
	}
    }
    if (new_routes != NULL) {
	if (old_routes != NULL) {
	    /* preserve the control flags from any old routes */
	    for (i = 0, scan = RouteListGetFirstRoute(info, new_routes);
		 i < new_routes->count;
		 i++, scan = RouteGetNextRoute(info, scan)) {
		RouteRef	old_route = NULL;

		old_route = RouteListFindRoute(info, old_routes, scan);
		if (old_route != NULL) {
		    /* preserve the control state in the new route */
		    scan->control_flags = old_route->control_flags;
		}
	    }
	}
	/* add any routes that need to be added */
	for (i = 0, scan = RouteListGetFirstRoute(info, new_routes);
	     i < new_routes->count;
	     i++, scan = RouteGetNextRoute(info, scan)) {
	    if ((scan->control_flags & kControlFlagsProcessed) != 0) {
		continue;
	    }
	    RouteProcess(scan, kRouteCommandAdd, &context);
	}
    }
    return;
}
/*
 * Function: RouteListFinalize
 * Purpose:
 *   Look for excluded routes. If the excluded route does not have an assigned
 *   interface, search for a route that *does not* go over the excluded
 *   interface.
 *
 *   If the excluded route does have an assigned interface, search for a route
 *   that *does* go over the assigned interface.
 *
 *   Set the gateway on the excluded route to match the gateway of the found
 *   route.
 */
static void
RouteListFinalize(RouteListInfoRef info, RouteListRef routes)
{
    int			i;
    RouteRef		scan;

    if (routes == NULL) {
	return;
    }
    for (i = 0, scan = RouteListGetFirstRoute(info, routes);
	 i < routes->count;
	 i++, scan = RouteGetNextRoute(info, scan)) {
	RouteRef		route;
	IFIndex			ifindex;
	RouteLookupFlags	flags;

	if (scan->exclude_ifindex == 0) {
	    continue;
	}
	if (scan->ifindex == 0) {
	    ifindex = scan->exclude_ifindex;
	    flags = kRouteLookupFlagsExcludeInterface;
	}
	else {
	    ifindex = scan->ifindex;
	    flags = kRouteLookupFlagsNone;
	}
	route = RouteListLookup(info, routes,
				(*info->route_destination)(scan),
				scan->prefix_length, ifindex, flags);
	if (route == NULL) {
	    (*info->route_log)(LOG_NOTICE, (RouteRef)scan,
			       "IPMonitor: can't resolve excluded route");
	}
	else {
	    if ((S_IPMonitor_debug & kDebugFlag8) != 0) {
		(*info->route_log)(LOG_DEBUG, (RouteRef)scan, "Excluded route");
		(*info->route_log)(LOG_DEBUG, (RouteRef)route, "Resolved to");
	    }
	    scan->ifindex = route->ifindex;
	    if ((route->flags & kRouteFlagsHasGateway) != 0) {
		(*info->route_set_gateway)(scan, (*info->route_gateway)(route));
		scan->flags |= kRouteFlagsHasGateway;
		if (scan->prefix_length == info->all_bits_set) {
		    scan->flags |= kRouteFlagsIsHost;
		}
	    }
	    else {
		/* routes directly to interface */
		scan->flags &= ~(kRouteFlagsHasGateway | kRouteFlagsIsHost);
	    }
	}
    }
    return;
}
#endif /* !TARGET_IPHONE_SIMULATOR */

/**
 ** IPv4Route*
 **/

#define IPV4_ROUTE_ALL_BITS_SET		32

static __inline__ struct in_addr
subnet_addr(struct in_addr addr, struct in_addr mask)
{
    struct in_addr	net;

    net.s_addr = addr.s_addr & mask.s_addr;
    return (net);
}

static void
IPv4RouteCopyDescriptionWithString(IPv4RouteRef r, CFMutableStringRef str)
{
    if ((r->flags & kRouteFlagsIsHost) != 0) {
	CFStringAppendFormat(str, NULL,
			     CFSTR("Host " IP_FORMAT),
			     IP_LIST(&r->dest));
    }
    else {
	CFStringAppendFormat(str, NULL,
			     CFSTR("Net " IP_FORMAT),
			     IP_LIST(&r->dest));
	CFStringAppendFormat(str, NULL, CFSTR("/%d"),
			     r->prefix_length);
    }
    if ((r->flags & kRouteFlagsHasGateway) != 0) {
	CFStringAppendFormat(str, NULL,
			     CFSTR(" Gate " IP_FORMAT),
			     IP_LIST(&r->gateway));
    }
    RouteAddInterfaceToDescription((RouteRef)r, str);
    if (r->ifa.s_addr != 0) {
	CFStringAppendFormat(str, NULL,
			     CFSTR(" Ifa " IP_FORMAT),
			     IP_LIST(&r->ifa));
    }
    RouteAddFlagsToDescription((RouteRef)r, str);
    return;
}

static CFStringRef
IPv4RouteCopyDescription(RouteRef r)
{
    CFMutableStringRef	str;

    str = CFStringCreateMutable(NULL, 0);
    IPv4RouteCopyDescriptionWithString((IPv4RouteRef)r, str);
    return (str);
}

#ifdef TEST_IPV4_ROUTELIST
static CFMutableStringRef
IPv4RouteListCopyDescription(IPv4RouteListRef routes);

static void
IPv4RouteLog(int level, RouteRef route, const char * msg)
{
    CFStringRef	str = IPv4RouteCopyDescription(route);

    if (msg == NULL) {
	SCPrint(TRUE, stdout, CFSTR("%@\n"), str);
    }
    else {
	SCPrint(TRUE, stdout, CFSTR("%s: %@\n"), msg, str);
    }
    CFRelease(str);
    return;
}

static __inline__ void
IPv4RouteListPrint(IPv4RouteListRef routes)
{
    CFStringRef	str = IPv4RouteListCopyDescription(routes);

    SCPrint(TRUE, stdout, CFSTR("%@\n"), str);
    CFRelease(str);
    return;
}

#else /* TEST_IPV4_ROUTELIST */

static __inline__ void
IPv4RouteLog(int level, RouteRef route, const char * msg)
{
    CFStringRef	str = IPv4RouteCopyDescription(route);

    if (msg == NULL) {
	my_log(level, "%@", str);
    }
    else {
	my_log(level, "%s: %@", msg, str);
    }
    CFRelease(str);
    return;
}

#endif /* TEST_IPV4_ROUTELIST */

static boolean_t
IPv4RouteIsEqual(RouteRef r_scan, RouteRef r_route)
{
    IPv4RouteRef 	route = (IPv4RouteRef)r_route;
    IPv4RouteRef 	scan = (IPv4RouteRef)r_scan;

    return ((scan->dest.s_addr == route->dest.s_addr)
	    && (scan->mask.s_addr == route->mask.s_addr)
	    && (scan->ifindex == route->ifindex)
	    && (scan->ifa.s_addr == route->ifa.s_addr)
	    && (scan->gateway.s_addr == route->gateway.s_addr)
	    && (scan->flags == route->flags));
}

static CFMutableStringRef
IPv4RouteListCopyDescription(IPv4RouteListRef routes)
{
    int			i;
    IPv4RouteRef	r;
    CFMutableStringRef	str;

    str = CFStringCreateMutable(NULL, 0);
    CFStringAppendFormat(str, NULL, CFSTR("<IPv4RouteList[%d]> = {"),
			 routes->count);
    for (i = 0, r = routes->list; i < routes->count; i++, r++) {
	CFStringAppendFormat(str, NULL, CFSTR("\n%2d. "), i);
	IPv4RouteCopyDescriptionWithString(r, str);
    }
    CFStringAppend(str, CFSTR("\n}"));
    return (str);
}

static size_t
IPv4RouteListComputeSize(CFIndex n)
{
    return (offsetof(IPv4RouteList, list[n]));
}

static int
count_prefix_bits_set(uint32_t n)
{
    int 	count;
    const static int8_t bits[16] = {
	0,	/* 0000 */
	-1,	/* 0001 */
	-1,	/* 0010 */
	-1,	/* 0011 */
	-1,	/* 0100 */
	-1,	/* 0101 */
	-1,	/* 0110 */
	-1,	/* 0111 */
	1,	/* 1000 */
	-1,	/* 1001 */
	-1,	/* 1010 */
	-1,	/* 1011 */
	2,	/* 1100 */
	-1,	/* 1101 */
	3,	/* 1110 */
	4,	/* 1111 */
    };

    for (count = 0; n != 0; n >>= 4) {
	int	nbits = bits[n & 0x0f];

	if (nbits < 0) {
	    return (-1);
	}
	count += nbits;
    }
    return (count);
}

static uint32_t
prefix_to_mask32(unsigned int prefix_length)
{
    if (prefix_length > 32 || prefix_length == 0) {
	return (0);
    }
    return (0xffffffff << (32 - prefix_length));
}

static int
mask_get_prefix_length(struct in_addr mask)
{
    int 		count;

    count = count_prefix_bits_set(mask.s_addr);
    if (count >= 0) {
	uint32_t	val;

	val = prefix_to_mask32(count);
	if (ntohl(mask.s_addr) != val) {
	    /* expected mask based on prefix length doesn't match */
	    return (-1);
	}
    }
    return (count);
}

static boolean_t
IPv4RouteSetPrefixLength(IPv4RouteRef route)
{
    int		length;

    length = mask_get_prefix_length(route->mask);
    if (length < 0) {
	return (FALSE);
    }
    route->prefix_length = length;
    return (TRUE);
}

static const void *
IPv4RouteGateway(RouteRef r_route)
{
    IPv4RouteRef	route = (IPv4RouteRef)r_route;
    return (&route->gateway);
}

static void
IPv4RouteSetGateway(RouteRef r_route, const void * address)
{
    IPv4RouteRef	route = (IPv4RouteRef)r_route;

    route->gateway = *((struct in_addr *)address);
    return;
}

static const void *
IPv4RouteDestination(RouteRef r_route)
{
    IPv4RouteRef	route = (IPv4RouteRef)r_route;
    return (&route->dest);
}

static boolean_t
IPv4RouteSameSubnet(RouteRef r_route, const void * addr)
{
    const struct in_addr *	address;
    IPv4RouteRef		route = (IPv4RouteRef)r_route;

    address = (const struct in_addr *)addr;
    return ((address->s_addr & route->mask.s_addr) == route->dest.s_addr);
}

/*
 * Define: ROUTE_MSG_ADDRS_SPACE
 * Purpose:
 *   Since sizeof(sockaddr_dl) > sizeof(sockaddr_in), we need space for
 *   3 sockaddr_in's and 2 sockaddr_dl's, but pad it just in case
 *   someone changes the code and doesn't think to modify this.
 */
#define ROUTE_MSG_ADDRS_SPACE	(3 * sizeof(struct sockaddr_in)	\
				 + 2 * sizeof(struct sockaddr_dl) \
				 + 128)
typedef struct {
    struct rt_msghdr	hdr;
    char		addrs[ROUTE_MSG_ADDRS_SPACE];
} route_msg;

/*
 * Function: IPv4RouteApply
 * Purpose:
 *   Add or remove the specified route to/from the kernel routing table.
 */
static int
IPv4RouteApply(RouteRef r_route, int cmd, int sockfd)
{
    int				len;
    int				ret = 0;
    IPv4RouteRef		route = (IPv4RouteRef)r_route;
    route_msg			rtmsg;
    union {
	struct sockaddr_in *	in_p;
	struct sockaddr_dl *	dl_p;
	void *			ptr;
    } rtaddr;

    if (S_netboot && route->dest.s_addr == 0) {
	/* don't touch the default route */
	return (EROUTENOTAPPLIED);
    }
    if ((route->flags & kRouteFlagsIsScoped) != 0
	&& !S_scopedroute) {
	return (EROUTENOTAPPLIED);
    }
    if ((route->flags & kRouteFlagsIsNULL) != 0) {
	return (EROUTENOTAPPLIED);
    }
    if (route->ifindex == 0) {
	my_log(LOG_NOTICE,
	       "IPMonitor IPv4RouteApply: " IP_FORMAT
	       " no interface specified, ignoring",
	       IP_LIST(&route->dest));
	return (ENXIO);
    }
    if (sockfd == -1) {
#ifdef TEST_IPV4_ROUTELIST
	return (0);
#else /* TEST_IPV4_ROUTELIST */
	return (EBADF);
#endif /* TEST_IPV4_ROUTELIST */
    }
    memset(&rtmsg, 0, sizeof(rtmsg));
    rtmsg.hdr.rtm_type = cmd;
    rtmsg.hdr.rtm_version = RTM_VERSION;
    rtmsg.hdr.rtm_seq = ++rtm_seq;
    rtmsg.hdr.rtm_addrs	= RTA_DST | RTA_GATEWAY | RTA_IFP;
    if (route->ifa.s_addr != 0) {
	rtmsg.hdr.rtm_addrs |= RTA_IFA;
    }
    rtmsg.hdr.rtm_flags = RTF_UP | RTF_STATIC;
    if ((route->flags & kRouteFlagsIsHost) != 0) {
	rtmsg.hdr.rtm_flags |= RTF_HOST;
    }
    else {
	rtmsg.hdr.rtm_addrs |= RTA_NETMASK;
	if ((route->flags & kRouteFlagsHasGateway) == 0) {
	    rtmsg.hdr.rtm_flags |= RTF_CLONING;
	}
    }
    if ((route->flags & kRouteFlagsHasGateway) != 0) {
	rtmsg.hdr.rtm_flags |= RTF_GATEWAY;
    }
    if ((route->flags & kRouteFlagsIsScoped) != 0) {
	rtmsg.hdr.rtm_index = route->ifindex;
	rtmsg.hdr.rtm_flags |= RTF_IFSCOPE;
    }

    rtaddr.ptr = rtmsg.addrs;

    /* dest */
    rtaddr.in_p->sin_len = sizeof(*rtaddr.in_p);
    rtaddr.in_p->sin_family = AF_INET;
    rtaddr.in_p->sin_addr = route->dest;
    rtaddr.ptr += sizeof(*rtaddr.in_p);

    /* gateway */
    if ((rtmsg.hdr.rtm_flags & RTF_GATEWAY) != 0) {
	/* gateway is an IP address */
	rtaddr.in_p->sin_len = sizeof(*rtaddr.in_p);
	rtaddr.in_p->sin_family = AF_INET;
	rtaddr.in_p->sin_addr = route->gateway;
	rtaddr.ptr += sizeof(*rtaddr.in_p);
    }
    else {
	/* gateway is the interface itself */
	rtaddr.dl_p->sdl_len = sizeof(*rtaddr.dl_p);
	rtaddr.dl_p->sdl_family = AF_LINK;
	rtaddr.dl_p->sdl_index = route->ifindex;
	rtaddr.ptr += sizeof(*rtaddr.dl_p);
    }

    /* mask */
    if ((rtmsg.hdr.rtm_addrs & RTA_NETMASK) != 0) {
	rtaddr.in_p->sin_len = sizeof(*rtaddr.in_p);
	rtaddr.in_p->sin_family = AF_INET;
	rtaddr.in_p->sin_addr = route->mask;
	rtaddr.ptr += sizeof(*rtaddr.in_p);
    }

    /* interface */
    if ((rtmsg.hdr.rtm_addrs & RTA_IFP) != 0) {
	rtaddr.dl_p->sdl_len = sizeof(*rtaddr.dl_p);
	rtaddr.dl_p->sdl_family = AF_LINK;
	rtaddr.dl_p->sdl_index = route->ifindex;
	rtaddr.ptr += sizeof(*rtaddr.dl_p);
    }
    /* interface address */
    if ((rtmsg.hdr.rtm_addrs & RTA_IFA) != 0) {
	rtaddr.in_p->sin_len = sizeof(*rtaddr.in_p);
	rtaddr.in_p->sin_family = AF_INET;
	rtaddr.in_p->sin_addr = route->ifa;
	rtaddr.ptr += sizeof(*rtaddr.in_p);
    }

    /* apply the route */
    len = (int)(sizeof(rtmsg.hdr) + (rtaddr.ptr - (void *)rtmsg.addrs));
    rtmsg.hdr.rtm_msglen = len;
    if (write(sockfd, &rtmsg, len) == -1) {
	ret = errno;
    }
    return (ret);
}

static const RouteListInfo IPv4RouteListInfo = {
    IPv4RouteListComputeSize,

    IPv4RouteIsEqual,
    IPv4RouteApply,
    IPv4RouteGateway,
    IPv4RouteSetGateway,
    IPv4RouteDestination,
    IPv4RouteSameSubnet,
    IPv4RouteLog,
    IPv4RouteCopyDescription,

    sizeof(IPv4Route),
    sizeof(struct in_addr),
    IPV4_ROUTE_ALL_BITS_SET
};

#if	!TARGET_IPHONE_SIMULATOR
static __inline__ void
IPv4RouteListLog(int level, IPv4RouteListRef routes)
{
    CFStringRef	str = IPv4RouteListCopyDescription(routes);

    my_log(level, "%@", str);
    CFRelease(str);
    return;
}

static void
IPv4RouteListApply(IPv4RouteListRef old_routes, IPv4RouteListRef new_routes,
		   int sockfd)
{
    RouteListApply(&IPv4RouteListInfo,
		   (RouteListRef)old_routes, (RouteListRef)new_routes,
		   sockfd);
    return;
}

static void
IPv4RouteListFinalize(IPv4RouteListRef routes)
{
    RouteListFinalize(&IPv4RouteListInfo, (RouteListRef)routes);
    return;
}
#endif /* !TARGET_IPHONE_SIMULATOR */

#ifdef TEST_IPV4_ROUTELIST
static IPv4RouteListRef
IPv4RouteListAddRouteList(IPv4RouteListRef routes, int init_size,
			  IPv4RouteListRef service_routes, Rank rank)
{
    return ((IPv4RouteListRef)
	    RouteListAddRouteList(&IPv4RouteListInfo,
				  (RouteListRef)routes, init_size,
				  (RouteListRef)service_routes, rank));
}
#endif /* TEST_IPV4_ROUTELIST */

static CFStringRef
plist_get_string(CFDictionaryRef dict, CFStringRef prop_name,
		 char * buf, int buf_size)
{
    CFStringRef	val;

    val = CFDictionaryGetValue(dict, prop_name);
    if (isA_CFString(val) == NULL) {
	return (NULL);
    }
    if (CFStringGetCString(val, buf, buf_size, kCFStringEncodingUTF8)
	== FALSE) {
	return (NULL);
    }
    return (val);
}

typedef struct {
    struct in_addr	addr;
    int *		count_p;
    IFIndex		ifindex;
    IFIndex		exclude_ifindex;
    IPv4RouteRef *	route_p;
    Rank		rank;
    const char *	descr;
} AddIPv4RouteContext, * AddIPv4RouteContextRef;

static void
AddIPv4Route(const void * value, void * context)
{
    AddIPv4RouteContextRef	ctx = (AddIPv4RouteContextRef)context;
    CFDictionaryRef		dict = (CFDictionaryRef)value;
    IPv4RouteRef		r = *ctx->route_p;

    dict = isA_CFDictionary(dict);
    if (dict == NULL
	|| !dict_get_ip(dict, kSCPropNetIPv4RouteDestinationAddress, &r->dest)
	|| !dict_get_ip(dict, kSCPropNetIPv4RouteSubnetMask, &r->mask)) {
	/* one less route than we expected */
	if (dict == NULL) {
	    my_log(LOG_NOTICE, "IPMonitor: %s route is not a dictionary",
		   ctx->descr);
	}
	else {
	    my_log(LOG_NOTICE, "IPMonitor: %s route is invalid, %@",
		   ctx->descr, dict);
	}
	goto skip;
    }
    if (IPv4RouteSetPrefixLength(r) == FALSE) {
	my_log(LOG_NOTICE, "IPMonitor: %s route has invalid subnet mask, %@",
	       ctx->descr, dict);
	goto skip;
    }
    r->rank = ctx->rank;
    r->exclude_ifindex = ctx->exclude_ifindex;
    if (ctx->ifindex != 0) {
	r->ifindex = ctx->ifindex;
	r->ifa = ctx->addr;
	if (ctx->exclude_ifindex == 0
	    && dict_get_ip(dict,
			   kSCPropNetIPv4RouteGatewayAddress,
			   &r->gateway)) {
	    r->flags |= kRouteFlagsHasGateway;
	    if (r->prefix_length == IPV4_ROUTE_ALL_BITS_SET) {
		r->flags |= kRouteFlagsIsHost;
	    }
	}
    }
    else {
	char		ifname[IFNAMSIZ];

	if (plist_get_string(dict, kSCPropNetIPv4RouteInterfaceName,
			     ifname, sizeof(ifname)) != NULL) {
	    IFIndex	ifindex;

	    ifindex = my_if_nametoindex(ifname);
	    if (ifindex == 0) {
		my_log(LOG_NOTICE,
		       "IPMonitor %s: interface %s does not exist, %@",
		       ctx->descr, ifname, dict);
		goto skip;
	    }
	    else if (ifindex == ctx->ifindex) {
		my_log(LOG_NOTICE,
		       "IPMonitor %s: interface %s unexpected, %@",
		       ctx->descr, ifname, dict);
		goto skip;
	    }
	    r->ifindex = ifindex;
	}
    }
    (*ctx->route_p)++;
    return;

 skip:
    (*ctx->count_p)--;
    return;

}

static boolean_t
confirm_interface_name(CFDictionaryRef dict, CFStringRef ifname)
{
    CFStringRef		confirmed_ifname;
    boolean_t		confirmed;

    confirmed_ifname
	= CFDictionaryGetValue(dict, kSCPropConfirmedInterfaceName);
    if (isA_CFString(confirmed_ifname) != NULL) {
	confirmed = CFEqual(confirmed_ifname, ifname);
    }
    else {
	confirmed = TRUE;
    }
    return (confirmed);
}

/*
 * Function: IPv4RouteListCreateWithDictionary
 *
 * Purpose:
 *   Given the service ipv4 entity dictionary, generate the list of routes.
 *   Currently, this includes just the default route and subnet route,
 *   if the service has a subnet mask.
 *
 * Returns:
 *   If the passed in route_list is NULL or too small, this routine
 *   allocates malloc'd memory to hold the routes.
 */
static IPv4RouteListRef
IPv4RouteListCreateWithDictionary(IPv4RouteListRef routes,
				  CFDictionaryRef dict,
				  CFNumberRef rank_assertion)
{
    boolean_t		add_default = FALSE;
    boolean_t		add_router_subnet = FALSE;
    boolean_t		add_subnet = FALSE;
    struct in_addr	addr = { 0 };
    CFArrayRef		additional_routes = NULL;
    CFIndex		additional_routes_count;
    boolean_t		allow_additional_routes = FALSE;
    boolean_t		exclude_from_nwi = FALSE;
    CFArrayRef		excluded_routes = NULL;
    CFIndex		excluded_routes_count;
    RouteFlags		flags = 0;
    IFIndex		ifindex;
    char		ifname[IFNAMSIZ];
    CFStringRef		ifname_cf;
    struct in_addr	mask = { 0 };
    int			n = 0;
    int			prefix_length = 0;
    Rank		primary_rank = kRankAssertionDefault;
    IPv4RouteRef	r;
    Rank		rank = kRankAssertionDefault;
    struct in_addr	router = { 0 };
    struct in_addr	subnet = { 0 };

    if (dict == NULL) {
	return (NULL);
    }
    ifname_cf = plist_get_string(dict, kSCPropInterfaceName,
				 ifname, sizeof(ifname));
    if (ifname_cf == NULL) {
	return (NULL);
    }
    ifindex = my_if_nametoindex(ifname);
    if (ifindex == 0) {
	/* interface doesn't exist */
	return (NULL);
    }
    allow_additional_routes = confirm_interface_name(dict, ifname_cf);
    if (dict_get_ip(dict, kSCPropNetIPv4Router, &router) == FALSE) {
	(void)dict_get_first_ip(dict, kSCPropNetIPv4DestAddresses, &router);
    }
    if (dict_get_first_ip(dict, kSCPropNetIPv4Addresses, &addr)
	&& dict_get_first_ip(dict, kSCPropNetIPv4SubnetMasks, &mask)) {
	/* subnet route */
	subnet = subnet_addr(addr, mask);
	/* ignore link-local subnets, let IPConfiguration handle them for now */
	if (ntohl(subnet.s_addr) != IN_LINKLOCALNETNUM) {
	    prefix_length = mask_get_prefix_length(mask);
	    if (prefix_length < 0) {
		my_log(LOG_NOTICE,
		       "IPMonitor: ignoring bad subnet mask "
		       IP_FORMAT " on %s",
		       IP_LIST(&mask), ifname);
	    }
	    else {
		add_subnet = TRUE;
		n++;
	    }
	}
	else if (router.s_addr == 0) {
	    exclude_from_nwi = TRUE;
	}
    }
    if (addr.s_addr == 0) {
	/* invalid/non-existent address */
	return (NULL);
    }
    if (rank_assertion != NULL) {
	(void)CFNumberGetValue(rank_assertion, kCFNumberSInt32Type,
			       &primary_rank);
    }
    if (router.s_addr == 0) {
	/* if no router is configured, demote the rank if necessary */
	switch (primary_rank) {
	case kRankAssertionLast:
	case kRankAssertionNever:
	case kRankAssertionScoped:
	    /* rank is already demoted */
	    break;
	default:
	    /* demote to RankLast */
	    primary_rank = kRankAssertionLast;
	    break;
	}
    }
    else {
	/*
	 * If the router address is our address and the subnet mask is
	 * not 255.255.255.255, assume all routes are local to the interface.
	 */
	if (addr.s_addr == router.s_addr
	    && mask.s_addr != INADDR_BROADCAST) {
	    ; /* all routes local */
	}
	else {
	    flags |= kRouteFlagsHasGateway;
	}
	if (rank_assertion == NULL && get_override_primary(dict)) {
	    primary_rank = kRankAssertionFirst;
	}
    }

    if (S_dict_get_boolean(dict, kIsNULL, FALSE)) {
	exclude_from_nwi = TRUE;
	flags |= kRouteFlagsIsNULL;
    }

    switch (primary_rank) {
    case kRankAssertionScoped:
	/* Scoped means all routes for the service get scoped */
	primary_rank = rank = kRankAssertionNever;
	flags |= kRouteFlagsIsScoped;
	break;
    case kRankAssertionNever:
	/* Never means just the default route gets scoped */
	rank = kRankAssertionLast;
	flags |= kRouteFlagsIsScoped;
	break;
    default:
	rank = primary_rank;
	break;
    }

    if ((flags & kRouteFlagsHasGateway) != 0) {
	add_router_subnet = TRUE;
	n++;
    }

    if (ifindex != lo0_ifindex()) {
	add_default = TRUE;
	n++;
    }
    if (allow_additional_routes) {
	additional_routes
	    = CFDictionaryGetValue(dict, kSCPropNetIPv4AdditionalRoutes);
	additional_routes = isA_CFArray(additional_routes);
	if (additional_routes != NULL) {
	    additional_routes_count = CFArrayGetCount(additional_routes);
	    n += additional_routes_count;
	}
	excluded_routes
	    = CFDictionaryGetValue(dict, kSCPropNetIPv4ExcludedRoutes);
	excluded_routes = isA_CFArray(excluded_routes);
	if (excluded_routes != NULL) {
	    excluded_routes_count = CFArrayGetCount(excluded_routes);
	    n += excluded_routes_count;
	}
    }
    if (routes == NULL || routes->size < n) {
	routes = (IPv4RouteListRef)malloc(IPv4RouteListComputeSize(n));
	routes->size = n;
    }
    bzero(routes, IPv4RouteListComputeSize(n));
    routes->count = n;
    if (exclude_from_nwi) {
	routes->flags |= kRouteListFlagsExcludeNWI;
    }

    /* start at the beginning */
    r = routes->list;

    if (add_default) {
	/* add the default route */
	routes->flags |= kRouteListFlagsHasDefault;
	r->ifindex = ifindex;
	r->ifa = addr;
	r->flags = flags;
	if ((flags & kRouteFlagsHasGateway) != 0) {
	    r->gateway = router;
	}
	else {
	    r->gateway = addr;
	}
	r->rank = primary_rank;
	r++;
    }

    /* add the subnet route */
    if (add_subnet) {
	if ((flags & kRouteFlagsIsNULL) != 0) {
	    r->flags |= kRouteFlagsIsNULL;
	}
	r->ifindex = ifindex;
	r->gateway = addr;
	r->dest = subnet;
	r->mask = mask;
	r->prefix_length = prefix_length;
	r->ifa = addr;
	r->rank = rank;
	r++;
    }

    /* add the router subnet route */
    if (add_router_subnet) {
	if ((flags & kRouteFlagsIsNULL) != 0) {
	    r->flags |= kRouteFlagsIsNULL;
	}
	r->ifindex = ifindex;
	r->gateway = addr;
	r->dest = router;
	r->mask.s_addr = INADDR_BROADCAST;
	r->prefix_length = IPV4_ROUTE_ALL_BITS_SET;
	r->ifa = addr;
	r->rank = rank;
	r++;
    }

    if (additional_routes != NULL || excluded_routes != NULL) {
	AddIPv4RouteContext		context;

	bzero(&context, sizeof(context));
	context.count_p = &routes->count;
	context.route_p = &r;
	context.rank = rank;

	/* additional routes */
	if (additional_routes != NULL) {
	    context.ifindex = ifindex;
	    context.addr = addr;
	    context.descr = "AdditionalRoutes";
	    CFArrayApplyFunction(additional_routes,
				 CFRangeMake(0, additional_routes_count),
				 AddIPv4Route, &context);
	}
	/* excluded routes */
	if (excluded_routes != NULL) {
	    context.descr = "ExcludedRoutes";
	    /* exclude this interface */
	    context.ifindex = 0;
	    context.exclude_ifindex = ifindex;
	    CFArrayApplyFunction(excluded_routes,
				 CFRangeMake(0, excluded_routes_count),
				 AddIPv4Route, &context);
	}
    }
    return (routes);
}

/**
 ** IPv6Route*
 **/
#define IPV6_ROUTE_ALL_BITS_SET		128

static boolean_t
ipv6_prefix_length_is_valid(int prefix_length)
{
    if (prefix_length < 0 || prefix_length > IPV6_ROUTE_ALL_BITS_SET) {
	return (FALSE);
    }
    return (TRUE);
}

/*
 * from netinet6/in6.c
 */
static void
in6_len2mask(struct in6_addr * mask, int len)
{
    int i;

    bzero(mask, sizeof(*mask));
    for (i = 0; i < len / 8; i++)
	mask->s6_addr[i] = 0xff;
    if (len % 8)
	mask->s6_addr[i] = (0xff00 >> (len % 8)) & 0xff;
}

static void
in6_maskaddr(struct in6_addr * addr, const struct in6_addr * mask)
{
    int i;

    for (i = 0; i < sizeof(addr->s6_addr); i++) {
	addr->s6_addr[i] &= mask->s6_addr[i];
    }
    return;
}

static void
in6_netaddr(struct in6_addr * addr, int len)
{
    struct in6_addr	mask;

    in6_len2mask(&mask, len);
    in6_maskaddr(addr, &mask);
    return;
}

static void
in6_addr_scope_linklocal(struct in6_addr * addr, IFIndex ifindex)
{
    if (IN6_IS_ADDR_LINKLOCAL(addr)) {
	addr->__u6_addr.__u6_addr16[1] = htons(ifindex);
    }
    return;
}

static void
string_append_in6_addr(CFMutableStringRef str, const struct in6_addr * addr)
{
    char 			ntopbuf[INET6_ADDRSTRLEN];

    CFStringAppendCString(str,
			  inet_ntop(AF_INET6, addr, ntopbuf, sizeof(ntopbuf)),
			  kCFStringEncodingASCII);
    return;
}

static void
IPv6RouteCopyDescriptionWithString(IPv6RouteRef r, CFMutableStringRef str)
{
    if ((r->flags & kRouteFlagsIsHost) != 0) {
	CFStringAppend(str, CFSTR("Host "));
	string_append_in6_addr(str, &r->dest);
    }
    else {
	CFStringAppend(str, CFSTR("Net "));
	string_append_in6_addr(str, &r->dest);
	CFStringAppendFormat(str, NULL, CFSTR("/%d"),
			     r->prefix_length);
    }
    if ((r->flags & kRouteFlagsHasGateway) != 0) {
	CFStringAppend(str, CFSTR(" Gate "));
	string_append_in6_addr(str, &r->gateway);
    }
    RouteAddInterfaceToDescription((RouteRef)r, str);
    if (!IN6_ARE_ADDR_EQUAL(&r->ifa, &in6addr_any)) {
	CFStringAppend(str, CFSTR(" Ifa "));
	string_append_in6_addr(str, &r->ifa);
    }
    RouteAddFlagsToDescription((RouteRef)r, str);
    return;
}

static CFStringRef
IPv6RouteCopyDescription(RouteRef r)
{
    CFMutableStringRef	str;

    str = CFStringCreateMutable(NULL, 0);
    IPv6RouteCopyDescriptionWithString((IPv6RouteRef)r, str);
    return (str);
}

static CFMutableStringRef
IPv6RouteListCopyDescription(IPv6RouteListRef routes)
{
    int			i;
    IPv6RouteRef	r;
    CFMutableStringRef	str;

    str = CFStringCreateMutable(NULL, 0);
    CFStringAppendFormat(str, NULL, CFSTR("<IPv6RouteList[%d]> = {"),
			 routes->count);
    for (i = 0, r = routes->list; i < routes->count; i++, r++) {
	CFStringAppendFormat(str, NULL, CFSTR("\n%2d. "), i);
	IPv6RouteCopyDescriptionWithString(r, str);
    }
    CFStringAppend(str, CFSTR("\n}"));
    return (str);
}

#ifdef TEST_IPV6_ROUTELIST

static void
IPv6RouteLog(int level, RouteRef route, const char * msg)
{
    CFStringRef	str = IPv6RouteCopyDescription(route);

    if (msg == NULL) {
	SCPrint(TRUE, stdout, CFSTR("%@\n"), str);
    }
    else {
	SCPrint(TRUE, stdout, CFSTR("%s: %@\n"), msg, str);
    }
    CFRelease(str);
    return;
}

static __inline__ void
IPv6RouteListPrint(IPv6RouteListRef routes)
{
    CFStringRef	str = IPv6RouteListCopyDescription(routes);

    SCPrint(TRUE, stdout, CFSTR("%@\n"), str);
    CFRelease(str);
    return;
}

#else /* TEST_IPV6_ROUTELIST */

static __inline__ void
IPv6RouteLog(int level, RouteRef route, const char * msg)
{
    CFStringRef	str = IPv6RouteCopyDescription(route);

    if (msg == NULL) {
	my_log(level, "%@", str);
    }
    else {
	my_log(level, "%s: %@", msg, str);
    }
    CFRelease(str);
    return;
}

#endif /* TEST_IPV6_ROUTELIST */

static size_t
IPv6RouteListComputeSize(CFIndex n)
{
    return (offsetof(IPv6RouteList, list[n]));
}


typedef struct {
    struct in6_addr *	addr;
    int *		count_p;
    IFIndex		ifindex;
    IFIndex		exclude_ifindex;
    IPv6RouteRef *	route_p;
    Rank		rank;
    const char *	descr;
} AddIPv6RouteContext, * AddIPv6RouteContextRef;

static void
AddIPv6Route(const void * value, void * context)
{
    AddIPv6RouteContextRef	ctx = (AddIPv6RouteContextRef)context;
    CFDictionaryRef		dict = (CFDictionaryRef)value;
    IPv6RouteRef		r = *ctx->route_p;

    dict = isA_CFDictionary(dict);
    if (dict == NULL
	|| !dict_get_ipv6(dict, kSCPropNetIPv6RouteDestinationAddress, &r->dest)
	|| !dict_get_int(dict, kSCPropNetIPv6RoutePrefixLength,
			 &r->prefix_length)
	|| !ipv6_prefix_length_is_valid(r->prefix_length)) {
	/* one less route than we expected */
	if (dict == NULL) {
	    my_log(LOG_NOTICE, "IPMonitor: %s route is not a dictionary",
		   ctx->descr);
	}
	else {
	    my_log(LOG_NOTICE, "IPMonitor: %s route is invalid, %@",
		   ctx->descr, dict);
	}
	goto skip;
    }
    r->rank = ctx->rank;
    r->exclude_ifindex = ctx->exclude_ifindex;
    if (ctx->ifindex != 0) {
	r->ifindex = ctx->ifindex;
	r->ifa = *ctx->addr;
	if (ctx->exclude_ifindex == 0
	    && dict_get_ipv6(dict,
			     kSCPropNetIPv6RouteGatewayAddress,
			     &r->gateway)) {
	    r->flags |= kRouteFlagsHasGateway;
	    if (r->prefix_length == IPV6_ROUTE_ALL_BITS_SET) {
		r->flags |= kRouteFlagsIsHost;
	    }
	}
    }
    else {
	char		ifname[IFNAMSIZ];

	if (plist_get_string(dict, kSCPropNetIPv6RouteInterfaceName,
			     ifname, sizeof(ifname)) != NULL) {
	    IFIndex	ifindex;

	    ifindex = my_if_nametoindex(ifname);
	    if (ifindex == 0) {
		my_log(LOG_NOTICE,
		       "IPMonitor %s: interface %s does not exist, %@",
		       ctx->descr, ifname, dict);
		goto skip;
	    }
	    else if (ifindex == ctx->ifindex) {
		my_log(LOG_NOTICE,
		       "IPMonitor %s: interface %s unexpected, %@",
		       ctx->descr, ifname, dict);
		goto skip;
	    }
	    r->ifindex = ifindex;
	}
    }
    (*ctx->route_p)++;
    return;

 skip:
    (*ctx->count_p)--;
    return;

}

/*
 * Function: IPv6RouteListCreateWithDictionary
 *
 * Purpose:
 *   Given the service IPv6 entity dictionary, generate the list of routes.
 *
 * Returns:
 *   If the passed in route_list is NULL or too small, this routine
 *   allocates malloc'd memory to hold the routes.
 */
static IPv6RouteListRef
IPv6RouteListCreateWithDictionary(IPv6RouteListRef routes,
				  CFDictionaryRef dict,
				  CFNumberRef rank_assertion)
{
    boolean_t		add_default = FALSE;
    boolean_t		add_prefix = FALSE;
    struct in6_addr	addr;
    CFArrayRef		additional_routes = NULL;
    CFIndex		additional_routes_count;
    boolean_t		allow_additional_routes = FALSE;
    boolean_t		exclude_from_nwi = FALSE;
    CFArrayRef		excluded_routes = NULL;
    CFIndex		excluded_routes_count;
    RouteFlags		flags = 0;
    IFIndex		ifindex;
    char		ifname[IFNAMSIZ];
    CFStringRef		ifname_cf;
    int			n = 0;
    int			prefix_length = 0;
    Rank		primary_rank = kRankAssertionDefault;
    IPv6RouteRef	r;
    Rank		rank = kRankAssertionDefault;
    struct in6_addr	router = in6addr_any;

    if (dict == NULL) {
	return (NULL);
    }
    ifname_cf = plist_get_string(dict, kSCPropInterfaceName,
				 ifname, sizeof(ifname));
    if (ifname_cf == NULL) {
	return (NULL);
    }
    ifindex = my_if_nametoindex(ifname);
    if (ifindex == 0) {
	/* interface doesn't exist */
	return (NULL);
    }
    allow_additional_routes = confirm_interface_name(dict, ifname_cf);
    if (dict_get_ipv6(dict, kSCPropNetIPv6Router, &router) == FALSE) {
	(void)dict_get_first_ipv6(dict, kSCPropNetIPv6DestAddresses, &router);
    }
    if (dict_get_first_ipv6(dict, kSCPropNetIPv6Addresses, &addr)) {
	if (IN6_IS_ADDR_UNSPECIFIED(&addr)) {
	    return (NULL);
	}
	if (dict_get_first_int(dict, kSCPropNetIPv6PrefixLength,
			       &prefix_length)
	    && !IN6_IS_ADDR_LINKLOCAL(&addr)
	    && ipv6_prefix_length_is_valid(prefix_length)) {
	    add_prefix = TRUE;
	    n++;
	}
	else {
	    prefix_length = 0;
	}
    }
    else {
	/* no addresses */
	return (NULL);
    }
    if (rank_assertion != NULL) {
	(void)CFNumberGetValue(rank_assertion, kCFNumberSInt32Type,
			       &primary_rank);
    }
    if (!IN6_IS_ADDR_UNSPECIFIED(&router)) {
	if (ifindex != lo0_ifindex()) {
	    add_default = TRUE;
	    n++;
	}
	/*
	 * If the router address is our address and the prefix length is
	 * not 128, assume all routes are local to the interface.
	 */
	if (IN6_ARE_ADDR_EQUAL(&router, &addr)
	    && prefix_length != IPV6_ROUTE_ALL_BITS_SET) {
	    ; /* all routes local */
	}
	else {
	    flags |= kRouteFlagsHasGateway;
	}
	if (rank_assertion == NULL && get_override_primary(dict)) {
	    primary_rank = kRankAssertionFirst;
	}
    }
    if (S_dict_get_boolean(dict, kIsNULL, FALSE)) {
	exclude_from_nwi = TRUE;
	flags |= kRouteFlagsIsNULL;
    }

    switch (primary_rank) {
    case kRankAssertionScoped:
	/* Scoped means all routes for the service get scoped */
	primary_rank = rank = kRankAssertionNever;
	flags |= kRouteFlagsIsScoped;
	break;
    case kRankAssertionNever:
	/* Never means just the default route gets scoped */
	rank = kRankAssertionLast;
	flags |= kRouteFlagsIsScoped;
	break;
    default:
	rank = primary_rank;
	break;
    }

    if (allow_additional_routes) {
	additional_routes
	    = CFDictionaryGetValue(dict, kSCPropNetIPv6AdditionalRoutes);
	additional_routes = isA_CFArray(additional_routes);
	if (additional_routes != NULL) {
	    additional_routes_count = CFArrayGetCount(additional_routes);
	    n += additional_routes_count;
	}
	excluded_routes = CFDictionaryGetValue(dict,
					       kSCPropNetIPv6ExcludedRoutes);
	excluded_routes = isA_CFArray(excluded_routes);
	if (excluded_routes != NULL) {
	    excluded_routes_count = CFArrayGetCount(excluded_routes);
	    n += excluded_routes_count;
	}
    }
    if (n == 0) {
	return (NULL);
    }

    /* need IPv6LL subnet route */
    n++;

    if (routes == NULL || routes->size < n) {
	routes = (IPv6RouteListRef)malloc(IPv6RouteListComputeSize(n));
	routes->size = n;
    }
    bzero(routes, IPv6RouteListComputeSize(n));
    routes->count = n;
    if (exclude_from_nwi) {
	routes->flags |= kRouteListFlagsExcludeNWI;
    }

    /* start at the beginning */
    r = routes->list;
    if (add_default) {
	/* add the default route */
	routes->flags |= kRouteListFlagsHasDefault;
	r->ifindex = ifindex;
	r->ifa = addr;
	r->flags = flags;
	if ((flags & kRouteFlagsHasGateway) != 0) {
	    r->gateway = router;
	}
	else {
	    r->gateway = addr;
	}
	r->rank = primary_rank;
	if (S_scopedroute_v6) {
	    r->flags |= kRouteFlagsKernelManaged;
	}
	r++;
    }


    /* add IPv6LL route */
    r->ifindex = ifindex;
    r->dest.s6_addr[0] = 0xfe;
    r->dest.s6_addr[1] = 0x80;
    r->prefix_length = 64;
    r->rank = rank;
    r->flags |= kRouteFlagsKernelManaged;
    r++;


    /* add the prefix route(s) */
    if (add_prefix) {
	r->flags |= kRouteFlagsKernelManaged;
	if ((flags & kRouteFlagsIsNULL) != 0) {
	    r->flags |= kRouteFlagsIsNULL;
	}
	r->ifindex = ifindex;
	r->gateway = addr;
	r->dest = addr;
	in6_netaddr(&r->dest, prefix_length);
	r->prefix_length = prefix_length;
	r->ifa = addr;
	r->rank = rank;
	r++;
    }

    if (additional_routes != NULL || excluded_routes != NULL) {
	AddIPv6RouteContext		context;

	bzero(&context, sizeof(context));
	context.count_p = &routes->count;
	context.route_p = &r;
	context.rank = rank;

	/* additional routes */
	if (additional_routes != NULL) {
	    context.ifindex = ifindex;
	    context.addr = &addr;
	    context.descr = "AdditionalRoutes";
	    CFArrayApplyFunction(additional_routes,
				 CFRangeMake(0, additional_routes_count),
				 AddIPv6Route, &context);
	}
	/* excluded routes */
	if (excluded_routes != NULL) {
	    context.descr = "ExcludedRoutes";
	    /* exclude this interface */
	    context.ifindex = 0;
	    context.exclude_ifindex = ifindex;
	    context.addr = NULL;
	    CFArrayApplyFunction(excluded_routes,
				 CFRangeMake(0, excluded_routes_count),
				 AddIPv6Route, &context);
	}
    }
    return (routes);
}

static const void *
IPv6RouteGateway(RouteRef r_route)
{
    IPv6RouteRef	route = (IPv6RouteRef)r_route;
    return (&route->gateway);
}

static void
IPv6RouteSetGateway(RouteRef r_route, const void * address)
{
    IPv6RouteRef	route = (IPv6RouteRef)r_route;

    route->gateway = *((struct in6_addr *)address);
    return;
}

static const void *
IPv6RouteDestination(RouteRef r_route)
{
    IPv6RouteRef	route = (IPv6RouteRef)r_route;
    return (&route->dest);
}

static __inline__ int
in6_addr_cmp(const struct in6_addr * a, const struct in6_addr * b)
{
    return (memcmp(a->s6_addr, b->s6_addr, sizeof(struct in6_addr)));
}

static boolean_t
IPv6RouteIsEqual(RouteRef r_route1, RouteRef r_route2)
{
    IPv6RouteRef 	route1 = (IPv6RouteRef)r_route1;
    IPv6RouteRef 	route2 = (IPv6RouteRef)r_route2;

    return (route1->prefix_length == route2->prefix_length
	    && route1->ifindex == route2->ifindex
	    && route1->flags == route2->flags
	    && in6_addr_cmp(&route1->dest, &route2->dest) == 0
	    && in6_addr_cmp(&route1->ifa, &route2->ifa) == 0
	    && in6_addr_cmp(&route1->gateway, &route2->gateway) == 0);
}

static boolean_t
IPv6RouteSameSubnet(RouteRef r_route, const void * addr)
{
    const struct in6_addr *	address = (const struct in6_addr *)addr;
    struct in6_addr		netaddr;
    IPv6RouteRef		route = (IPv6RouteRef)r_route;

    netaddr = *address;
    in6_netaddr(&netaddr, route->prefix_length);
    return (in6_addr_cmp(&netaddr, &route->dest) == 0);
}


#define V6_ROUTE_MSG_ADDRS_SPACE (5 * sizeof(struct sockaddr_dl) + 128)

typedef struct {
    struct rt_msghdr	hdr;
    char		addrs[V6_ROUTE_MSG_ADDRS_SPACE];
} v6_route_msg;

/*
 * Function: IPv6RouteApply
 * Purpose:
 *   Add or remove the specified route to/from the kernel routing table.
 */
static int
IPv6RouteApply(RouteRef r_route, int cmd, int sockfd)
{
    int				len;
    int				ret = 0;
    IPv6RouteRef		route = (IPv6RouteRef)r_route;
    v6_route_msg		rtmsg;
    union {
	struct sockaddr_in6 *	in_p;
	struct sockaddr_dl *	dl_p;
	void *			ptr;
    } rtaddr;

    if ((route->flags & kRouteFlagsIsScoped) != 0
	&& !S_scopedroute_v6) {
	return (EROUTENOTAPPLIED);
    }
    if ((route->flags & kRouteFlagsKernelManaged) != 0) {
	/* the kernel manages this route, don't touch it */
	return (EROUTENOTAPPLIED);
    }
    if ((route->flags & kRouteFlagsIsNULL) != 0) {
	return (EROUTENOTAPPLIED);
    }
    if (route->ifindex == 0) {
	IPv6RouteLog(LOG_NOTICE, (RouteRef)route,
		     "IPMonitor IPv6RouteApply: no interface specified");
	return (ENXIO);
    }
    if (sockfd == -1) {
#ifdef TEST_IPV6_ROUTELIST
	return (0);
#else /* TEST_IPV6_ROUTELIST */
	return (EBADF);
#endif /* TEST_IPV6_ROUTELIST */
    }
    memset(&rtmsg, 0, sizeof(rtmsg));
    rtmsg.hdr.rtm_type = cmd;
    rtmsg.hdr.rtm_version = RTM_VERSION;
    rtmsg.hdr.rtm_seq = ++rtm_seq;
    rtmsg.hdr.rtm_addrs	= RTA_DST | RTA_GATEWAY | RTA_IFP;
    if (!IN6_IS_ADDR_UNSPECIFIED(&route->ifa)) {
	rtmsg.hdr.rtm_addrs |= RTA_IFA;
    }
    rtmsg.hdr.rtm_flags = RTF_UP | RTF_STATIC;
    if ((route->flags & kRouteFlagsIsHost) != 0) {
	rtmsg.hdr.rtm_flags |= RTF_HOST;
    }
    else {
	rtmsg.hdr.rtm_addrs |= RTA_NETMASK;
	if ((route->flags & kRouteFlagsHasGateway) == 0) {
	    rtmsg.hdr.rtm_flags |= RTF_CLONING;
	}
    }
    if ((route->flags & kRouteFlagsHasGateway) != 0) {
	rtmsg.hdr.rtm_flags |= RTF_GATEWAY;
    }
    if ((route->flags & kRouteFlagsIsScoped) != 0) {
	rtmsg.hdr.rtm_index = route->ifindex;
	rtmsg.hdr.rtm_flags |= RTF_IFSCOPE;
    }

    rtaddr.ptr = rtmsg.addrs;

    /* dest */
    rtaddr.in_p->sin6_len = sizeof(*rtaddr.in_p);
    rtaddr.in_p->sin6_family = AF_INET6;
    rtaddr.in_p->sin6_addr = route->dest;
    in6_addr_scope_linklocal(&rtaddr.in_p->sin6_addr, route->ifindex);
    rtaddr.ptr += sizeof(*rtaddr.in_p);

    /* gateway */
    if ((rtmsg.hdr.rtm_flags & RTF_GATEWAY) != 0) {
	/* gateway is an IP address */
	rtaddr.in_p->sin6_len = sizeof(*rtaddr.in_p);
	rtaddr.in_p->sin6_family = AF_INET6;
	rtaddr.in_p->sin6_addr = route->gateway;
	in6_addr_scope_linklocal(&rtaddr.in_p->sin6_addr, route->ifindex);
	rtaddr.ptr += sizeof(*rtaddr.in_p);
    }
    else {
	/* gateway is the interface itself */
	rtaddr.dl_p->sdl_len = sizeof(*rtaddr.dl_p);
	rtaddr.dl_p->sdl_family = AF_LINK;
	rtaddr.dl_p->sdl_index = route->ifindex;
	rtaddr.ptr += sizeof(*rtaddr.dl_p);
    }

    /* mask */
    if ((rtmsg.hdr.rtm_addrs & RTA_NETMASK) != 0) {
	rtaddr.in_p->sin6_len = sizeof(*rtaddr.in_p);
	rtaddr.in_p->sin6_family = AF_INET6;
	in6_len2mask(&rtaddr.in_p->sin6_addr, route->prefix_length);
	rtaddr.ptr += sizeof(*rtaddr.in_p);
    }

    /* interface */
    if ((rtmsg.hdr.rtm_addrs & RTA_IFP) != 0) {
	rtaddr.dl_p->sdl_len = sizeof(*rtaddr.dl_p);
	rtaddr.dl_p->sdl_family = AF_LINK;
	rtaddr.dl_p->sdl_index = route->ifindex;
	rtaddr.ptr += sizeof(*rtaddr.dl_p);
    }
    /* interface address */
    if ((rtmsg.hdr.rtm_addrs & RTA_IFA) != 0) {
	rtaddr.in_p->sin6_len = sizeof(*rtaddr.in_p);
	rtaddr.in_p->sin6_family = AF_INET6;
	rtaddr.in_p->sin6_addr = route->ifa;
	rtaddr.ptr += sizeof(*rtaddr.in_p);
    }

    /* apply the route */
    len = (int)(sizeof(rtmsg.hdr) + (rtaddr.ptr - (void *)rtmsg.addrs));
    rtmsg.hdr.rtm_msglen = len;
    if (write(sockfd, &rtmsg, len) == -1) {
	ret = errno;
    }
    return (ret);
}

static const RouteListInfo IPv6RouteListInfo = {
    IPv6RouteListComputeSize,

    IPv6RouteIsEqual,
    IPv6RouteApply,
    IPv6RouteGateway,
    IPv6RouteSetGateway,
    IPv6RouteDestination,
    IPv6RouteSameSubnet,
    IPv6RouteLog,
    IPv6RouteCopyDescription,

    sizeof(IPv6Route),
    sizeof(struct in6_addr),
    IPV6_ROUTE_ALL_BITS_SET
};

#ifdef TEST_IPV6_ROUTELIST
static IPv6RouteListRef
IPv6RouteListAddRouteList(IPv6RouteListRef routes, int init_size,
			  IPv6RouteListRef service_routes, Rank rank)
{
    return ((IPv6RouteListRef)
	    RouteListAddRouteList(&IPv6RouteListInfo,
				  (RouteListRef)routes, init_size,
				  (RouteListRef)service_routes, rank));
}
#endif /* TEST_IPV6_ROUTELIST */

#if	!TARGET_IPHONE_SIMULATOR
static __inline__ void
IPv6RouteListLog(int level, IPv6RouteListRef routes)
{
    CFStringRef	str = IPv6RouteListCopyDescription(routes);

    my_log(level, "%@", str);
    CFRelease(str);
    return;
}

static void
IPv6RouteListFinalize(IPv6RouteListRef routes)
{
    RouteListFinalize(&IPv6RouteListInfo, (RouteListRef)routes);
    return;
}

static void
IPv6RouteListApply(IPv6RouteListRef old_routes, IPv6RouteListRef new_routes,
		   int sockfd)
{
    RouteListApply(&IPv6RouteListInfo,
		   (RouteListRef)old_routes, (RouteListRef)new_routes,
		   sockfd);
    return;
}
#endif /* !TARGET_IPHONE_SIMULATOR */

/*
 * Function: parse_component
 * Purpose:
 *   Given a string 'key' and a string prefix 'prefix',
 *   return the next component in the slash '/' separated
 *   key.
 *
 * Examples:
 * 1. key = "a/b/c" prefix = "a/"
 *    returns "b"
 * 2. key = "a/b/c" prefix = "a/b/"
 *    returns "c"
 */
static CF_RETURNS_RETAINED CFStringRef
parse_component(CFStringRef key, CFStringRef prefix)
{
    CFMutableStringRef	comp;
    CFRange		range;

    if (CFStringHasPrefix(key, prefix) == FALSE) {
	return (NULL);
    }
    comp = CFStringCreateMutableCopy(NULL, 0, key);
    if (comp == NULL) {
	return (NULL);
    }
    CFStringDelete(comp, CFRangeMake(0, CFStringGetLength(prefix)));
    range = CFStringFind(comp, CFSTR("/"), 0);
    if (range.location == kCFNotFound) {
	return (comp);
    }
    range.length = CFStringGetLength(comp) - range.location;
    CFStringDelete(comp, range);
    return (comp);
}

__private_extern__ boolean_t
service_contains_protocol(CFDictionaryRef service, int af)
{
    boolean_t		contains_protocol = FALSE;
    CFStringRef		entity;
    RouteListRef	routes;
    CFDictionaryRef	dict;

    entity = (af == AF_INET) ? kSCEntNetIPv4 : kSCEntNetIPv6;
    dict = CFDictionaryGetValue(service, entity);
    if (dict == NULL) {
	goto done;
    }
    routes = ipdict_get_routelist(dict);
    if (routes == NULL) {
	goto done;
    }
    if ((routes->flags & kRouteListFlagsExcludeNWI) != 0) {
	goto done;
    }
    contains_protocol = TRUE;

 done:
    return (contains_protocol);
}


static CFMutableDictionaryRef
service_dict_copy(CFStringRef serviceID)
{
    CFDictionaryRef		d = NULL;
    CFMutableDictionaryRef	service_dict;

    /* create a modifyable dictionary, a copy or a new one */
    d = CFDictionaryGetValue(S_service_state_dict, serviceID);
    if (d == NULL) {
	service_dict
	    = CFDictionaryCreateMutable(NULL, 0,
					&kCFTypeDictionaryKeyCallBacks,
					&kCFTypeDictionaryValueCallBacks);
    }
    else {
	service_dict = CFDictionaryCreateMutableCopy(NULL, 0, d);
    }
    return (service_dict);
}

static void
log_service_entity(int level, CFStringRef serviceID, CFStringRef entity,
		   CFStringRef operation, CFTypeRef val)
{
    CFMutableStringRef this_val = NULL;

    if (val != NULL) {
	boolean_t	is_ipv4;
	boolean_t	is_ipv6;

	if ((is_ipv4 = CFEqual(entity, kSCEntNetIPv4))
	    || (is_ipv6 = CFEqual(entity, kSCEntNetIPv6))) {
	    RouteListUnion	routes;

	    routes.ptr = ipdict_get_routelist(val);
	    if (routes.ptr != NULL) {
		CFDictionaryRef	service_dict = NULL;

		if (is_ipv4) {
		    this_val = IPv4RouteListCopyDescription(routes.v4);
		}
		else {
		    this_val = IPv6RouteListCopyDescription(routes.v6);
		}
		service_dict = ipdict_get_service(val);
		if (service_dict != NULL) {
		    CFStringAppendFormat(this_val, NULL,
					 CFSTR("\n<Service> = %@"),
					 service_dict);
		}
		val = this_val;
	    }
	}
    }
    if (val == NULL) {
	val = CFSTR("<none>");
    }
    my_log(level, "IPMonitor: serviceID %@ %@ %@ value = %@",
	   serviceID, operation, entity, val);
    my_CFRelease(&this_val);
    return;
}

static boolean_t
service_dict_set(CFStringRef serviceID, CFStringRef entity,
		 CFTypeRef new_val)
{
    boolean_t			changed = FALSE;
    CFTypeRef			old_val;
    CFMutableDictionaryRef	service_dict;

    service_dict = service_dict_copy(serviceID);
    old_val = CFDictionaryGetValue(service_dict, entity);
    if (new_val == NULL) {
	if (old_val != NULL) {
	    if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
		log_service_entity(LOG_DEBUG, serviceID, entity,
				   CFSTR("Removed:"), old_val);
	    }
	    CFDictionaryRemoveValue(service_dict, entity);
	    changed = TRUE;
	}
    }
    else {
	if (old_val == NULL || CFEqual(new_val, old_val) == FALSE) {
	    if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
		log_service_entity(LOG_DEBUG, serviceID, entity,
				   CFSTR("Changed: old"), old_val);
		log_service_entity(LOG_DEBUG, serviceID, entity,
				   CFSTR("Changed: new"), new_val);
	    }
	    CFDictionarySetValue(service_dict, entity, new_val);
	    changed = TRUE;
	}
    }
    if (CFDictionaryGetCount(service_dict) == 0) {
	CFDictionaryRemoveValue(S_service_state_dict, serviceID);
    }
    else {
	CFDictionarySetValue(S_service_state_dict, serviceID, service_dict);
    }
    my_CFRelease(&service_dict);
    return (changed);
}

static CFDictionaryRef
service_dict_get(CFStringRef serviceID, CFStringRef entity)
{
    CFDictionaryRef	service_dict;

    service_dict = CFDictionaryGetValue(S_service_state_dict, serviceID);
    if (service_dict == NULL) {
	return (NULL);
    }
    return (CFDictionaryGetValue(service_dict, entity));
}

#ifndef kSCPropNetHostname
#define kSCPropNetHostname CFSTR("Hostname")
#endif

__private_extern__
CFStringRef
copy_dhcp_hostname(CFStringRef serviceID)
{
    CFDictionaryRef 	dict = NULL;
    CFStringRef		hostname = NULL;
    CFDictionaryRef 	service_dict = NULL;

    dict = service_dict_get(serviceID, kSCEntNetIPv4);
    if (dict == NULL) {
	return (NULL);
    }
    service_dict = ipdict_get_service(dict);
    if (service_dict == NULL) {
	return (NULL);
    }
    hostname = CFDictionaryGetValue(service_dict, kSCPropNetHostname);
    if (hostname != NULL) {
	CFRetain(hostname);
    }
    return (hostname);
}

#if	!TARGET_IPHONE_SIMULATOR

static struct in6_addr *
ipv6_service_get_router(CFDictionaryRef service,
			IFIndex * ifindex_p, CFStringRef * ifname_p)
{
    IPv6RouteListRef	routes;
    struct in6_addr * 	router = NULL;

    routes = ipdict_get_routelist(service);
    if (routes != NULL
	&& (routes->flags & kRouteListFlagsExcludeNWI) == 0
	&& (routes->flags & kRouteListFlagsHasDefault) != 0) {
	router = &routes->list[0].gateway;
	if (*ifindex_p == 0) {
	    *ifindex_p = routes->list[0].ifindex;
	}
	if (*ifname_p == NULL) {
	    *ifname_p = ipdict_get_ifname(service);
	}
    }
    return (router);
}

static void
ipv6_service_update_router(CFStringRef serviceID, CFDictionaryRef new_service)
{
    IFIndex		ifindex = 0;
    CFStringRef		ifname = NULL;
    char		ntopbuf[INET6_ADDRSTRLEN];
    CFDictionaryRef	old_service;
    struct in6_addr *	old_router;
    struct in6_addr *	new_router;
    int			s = -1;

    old_service = service_dict_get(serviceID, kSCEntNetIPv6);
    old_router = ipv6_service_get_router(old_service, &ifindex, &ifname);
    new_router = ipv6_service_get_router(new_service, &ifindex, &ifname);
    if (ifname == NULL || ifindex == 0) {
	return;
    }
    s = inet6_dgram_socket();
    if (s < 0) {
	my_log(LOG_ERR,
	       "IPMonitor: ipv6_service_update_router: socket failed, %s",
	       strerror(errno));
	goto done;
    }
    /* remove the old router if it was defined */
    if (old_router != NULL
	&& (new_router == NULL
	    || !IN6_ARE_ADDR_EQUAL(old_router, new_router))) {
	if (siocdrdel_in6(s, ifindex, old_router) < 0) {
	    if (errno != EINVAL
		|| (S_IPMonitor_debug & kDebugFlag1) != 0) {
		my_log((errno == EINVAL) ? LOG_DEBUG : LOG_ERR,
		       "IPMonitor: siocdrdel_in6(%@, %s) failed, %s",
		       ifname,
		       inet_ntop(AF_INET6, old_router,
				 ntopbuf, sizeof(ntopbuf)),
		       strerror(errno));
	    }
	}
	else if (S_IPMonitor_debug & kDebugFlag1) {
	    my_log(LOG_DEBUG,
		   "IPMonitor: %@ removed default route %s",
		   ifname,
		   inet_ntop(AF_INET6, old_router, ntopbuf, sizeof(ntopbuf)));
	}
    }
    /* add the new router if it is defined */
    if (new_router != NULL
	&& (old_router == NULL
	    || !IN6_ARE_ADDR_EQUAL(old_router, new_router))) {
	if (siocdradd_in6(s, ifindex, new_router, 0) < 0) {
	    if (errno != EINVAL
		|| (S_IPMonitor_debug & kDebugFlag1) != 0) {
		my_log((errno == EINVAL) ? LOG_DEBUG : LOG_ERR,
		       "IPMonitor: siocdradd_in6(%@, %s) failed, %s",
		       ifname,
		       inet_ntop(AF_INET6, new_router,
				 ntopbuf, sizeof(ntopbuf)),
		       strerror(errno));
	    }
	}
	else if (S_IPMonitor_debug & kDebugFlag1) {
	    my_log(LOG_DEBUG,
		   "IPMonitor: %@ added default route %s",
		   ifname,
		   inet_ntop(AF_INET6, new_router, ntopbuf, sizeof(ntopbuf)));
	}
    }
    close(s);

  done:
    return;
}
#endif	/* !TARGET_IPHONE_SIMULATOR */

#define	ALLOW_EMPTY_STRING	0x1

static CF_RETURNS_RETAINED CFTypeRef
sanitize_prop(CFTypeRef val, uint32_t flags)
{
    if (val != NULL) {
	if (isA_CFString(val)) {
	    CFMutableStringRef	str;

	    str = CFStringCreateMutableCopy(NULL, 0, (CFStringRef)val);
	    CFStringTrimWhitespace(str);
	    if (!(flags & ALLOW_EMPTY_STRING) && (CFStringGetLength(str) == 0)) {
		CFRelease(str);
		str = NULL;
	    }
	    val = str;
	} else {
	    CFRetain(val);
	}
    }

    return val;
}

static void
merge_array_prop(CFMutableDictionaryRef	dict,
		 CFStringRef		key,
		 CFDictionaryRef	state_dict,
		 CFDictionaryRef	setup_dict,
		 uint32_t		flags,
		 Boolean		append)
{
    CFMutableArrayRef	merge_prop;
    CFArrayRef		setup_prop = NULL;
    CFArrayRef		state_prop = NULL;

    if (setup_dict != NULL) {
	setup_prop = isA_CFArray(CFDictionaryGetValue(setup_dict, key));
    }
    if (state_dict != NULL) {
	state_prop = isA_CFArray(CFDictionaryGetValue(state_dict, key));
    }

    if ((setup_prop == NULL) && (state_prop == NULL)) {
	return;
    }

    merge_prop = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    if (setup_prop != NULL) {
	CFIndex	i;
	CFIndex	n;

	n = CFArrayGetCount(setup_prop);
	for (i = 0; i < n; i++) {
	    CFTypeRef   val;

	    val = CFArrayGetValueAtIndex(setup_prop, i);
	    val = sanitize_prop(val, flags);
	    if (val != NULL) {
		CFArrayAppendValue(merge_prop, val);
		CFRelease(val);
	    }
	}
    }
    if (state_prop != NULL
	&& (setup_prop == NULL || S_append_state)) {
	CFIndex	i;
	CFIndex	n;
	CFRange	setup_range	= CFRangeMake(0, CFArrayGetCount(merge_prop));

	n = CFArrayGetCount(state_prop);
	for (i = 0; i < n; i++) {
	    CFTypeRef   val;

	    val = CFArrayGetValueAtIndex(state_prop, i);
	    val = sanitize_prop(val, flags);
	    if (val != NULL) {
		if (append || !CFArrayContainsValue(merge_prop, setup_range, val)) {
		    CFArrayAppendValue(merge_prop, val);
		}
		CFRelease(val);
	    }
	}
    }
    if (CFArrayGetCount(merge_prop) > 0) {
	CFDictionarySetValue(dict, key, merge_prop);
    }
    CFRelease(merge_prop);
    return;
}

static void
pick_prop(CFMutableDictionaryRef	dict,
	  CFStringRef			key,
	  CFDictionaryRef		state_dict,
	  CFDictionaryRef		setup_dict,
	  uint32_t			flags)
{
	CFTypeRef	val	= NULL;

	if (setup_dict != NULL) {
	    val = CFDictionaryGetValue(setup_dict, key);
	    val = sanitize_prop(val, flags);
	}
	if (val == NULL && state_dict != NULL) {
	    val = CFDictionaryGetValue(state_dict, key);
	    val = sanitize_prop(val, flags);
	}
	if (val != NULL) {
	    CFDictionarySetValue(dict, key, val);
	    CFRelease(val);
	}

	return;
}

/**
 ** GetEntityChangesFunc functions
 **/
#define IPV4_ROUTES_N_STATIC		5
#define IPV4_ROUTES_ALIGN_BUF_SIZE_UINT32				\
    (roundup(IPv4RouteListComputeSize(IPV4_ROUTES_N_STATIC),		\
	     sizeof(uint32_t))						\
     / sizeof(uint32_t))

#define IPV4_ROUTES_BUF_DECL(routes)		\
    IPv4RouteListRef	routes;						\
    uint32_t		routes_buf[IPV4_ROUTES_ALIGN_BUF_SIZE_UINT32];	\
									\
    routes = (IPv4RouteListRef)(void *)routes_buf;			\
    routes->size = IPV4_ROUTES_N_STATIC;				\
    routes->count = 0;							\
    routes->flags = 0;

static CFDataRef
IPv4RouteListDataCreate(CFDictionaryRef dict, CFNumberRef rank_assertion)
{
    IPv4RouteListRef	r;
    CFDataRef		routes_data;
    IPV4_ROUTES_BUF_DECL(routes);

    r = IPv4RouteListCreateWithDictionary(routes, dict, rank_assertion);
    if (r != NULL) {
	routes_data = CFDataCreate(NULL,
				   (const void *)r,
				   IPv4RouteListComputeSize(r->count));
	if (r != routes) {
	    free(r);
	}
    }
    else {
	routes_data = NULL;
    }
    return (routes_data);
}
#define IPV6_ROUTES_N_STATIC		3
#define IPV6_ROUTES_ALIGN_BUF_SIZE_UINT32				\
    (roundup(IPv6RouteListComputeSize(IPV6_ROUTES_N_STATIC),		\
	     sizeof(uint32_t))						\
     / sizeof(uint32_t))

#define IPV6_ROUTES_BUF_DECL(routes) \
    IPv6RouteListRef	routes;					       \
    uint32_t		routes_buf[IPV6_ROUTES_ALIGN_BUF_SIZE_UINT32]; \
								       \
    routes = (IPv6RouteListRef)(void *)routes_buf;		       \
    routes->size = IPV6_ROUTES_N_STATIC;			       \
    routes->count = 0;						       \
    routes->flags = 0;

static CFDataRef
IPv6RouteListDataCreate(CFDictionaryRef dict, CFNumberRef rank_assertion)
{
    IPv6RouteListRef	r;
    CFDataRef		routes_data;
    IPV6_ROUTES_BUF_DECL(routes);

    r = IPv6RouteListCreateWithDictionary(routes, dict, rank_assertion);
    if (r != NULL) {
	routes_data = CFDataCreate(NULL,
				   (const void *)r,
				   IPv6RouteListComputeSize(r->count));
	if (r != routes) {
	    free(r);
	}
    }
    else {
	routes_data = NULL;
    }
    return (routes_data);
}

static CFDictionaryRef
IPDictCreate(int af, CFDictionaryRef state_dict, CFDictionaryRef setup_dict,
	     CFNumberRef rank_assertion)
{
    CFDictionaryRef		aggregated_dict = NULL;
    CFDictionaryRef		dict;
    CFMutableDictionaryRef	modified_dict = NULL;
    CFDataRef			routes_data;

    dict = state_dict;
    if (dict != NULL && setup_dict != NULL) {
	/* look for keys in Setup: that override/merge with State: */
	CFArrayRef	additional_routes;
	CFStringRef	router;
	in_addr		router_ip;
	CFStringRef	router_prop;
	CFStringRef	route_list_prop;

	/* Router */
	switch (af) {
	case AF_INET:
	    router_prop = kSCPropNetIPv4Router;
	    route_list_prop = kSCPropNetIPv4AdditionalRoutes;
	    break;
	default:
	case AF_INET6:
	    router_prop = kSCPropNetIPv6Router;
	    route_list_prop = kSCPropNetIPv6AdditionalRoutes;
	    break;
	}
	router = CFDictionaryGetValue(setup_dict, router_prop);
	if (router != NULL
	    && !cfstring_to_ipvx(af, router, &router_ip, sizeof(router_ip))) {
	    router = NULL;
	}

	/* AdditionalRoutes */
	additional_routes
	    = CFDictionaryGetValue(setup_dict, route_list_prop);
	additional_routes = isA_CFArray(additional_routes);

	if (router != NULL || additional_routes != NULL) {
	    modified_dict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
	    if (router != NULL) {
		CFDictionarySetValue(modified_dict,
				     router_prop,
				     router);
	    }
	    if (additional_routes != NULL) {
		CFArrayRef	combined_routes = NULL;
		CFArrayRef	state_routes;

		state_routes
		    = CFDictionaryGetValue(state_dict,
					   route_list_prop);
		if (isA_CFArray(state_routes) != NULL) {
		    combined_routes
			= my_CFArrayCreateCombinedArray(additional_routes,
							state_routes);
		    additional_routes = combined_routes;
		}
		CFDictionarySetValue(modified_dict,
				     route_list_prop,
				     additional_routes);
		if (combined_routes != NULL) {
		    CFRelease(combined_routes);
		}
	    }
	    dict = modified_dict;
	}
    }
    switch (af) {
    case AF_INET:
	routes_data = IPv4RouteListDataCreate(dict, rank_assertion);
	break;
    default:
    case AF_INET6:
	routes_data = IPv6RouteListDataCreate(dict, rank_assertion);
	break;
    }
    if (routes_data != NULL) {
	aggregated_dict = ipdict_create(dict, routes_data);
	CFRelease(routes_data);
    }
    if (modified_dict != NULL) {
	CFRelease(modified_dict);
    }
    return (aggregated_dict);
}

static boolean_t
get_ipv4_changes(CFStringRef serviceID, CFDictionaryRef state_dict,
		 CFDictionaryRef setup_dict, CFDictionaryRef info)
{
    CFDictionaryRef		dict = NULL;
    boolean_t			changed	= FALSE;
    CFNumberRef			rank_assertion = NULL;
    CFDictionaryRef		service_options;

    if (state_dict == NULL) {
	goto done;
    }
    service_options = service_dict_get(serviceID, kSCEntNetService);
    if (service_options != NULL) {
	rank_assertion
	    = CFDictionaryGetValue(service_options,
				   kServiceOptionRankAssertion);
    }
    dict = IPDictCreate(AF_INET, state_dict, setup_dict, rank_assertion);

  done:
    changed = service_dict_set(serviceID, kSCEntNetIPv4, dict);
    if (dict == NULL) {
	/* clean up the rank too */
	CFDictionaryRemoveValue(S_ipv4_service_rank_dict, serviceID);
    }
    my_CFRelease(&dict);
    return (changed);
}

static boolean_t
get_ipv6_changes(CFStringRef serviceID, CFDictionaryRef state_dict,
		 CFDictionaryRef setup_dict, CFDictionaryRef info)
{
    CFDictionaryRef		dict = NULL;
    boolean_t			changed	= FALSE;
    CFNumberRef			rank_assertion = NULL;
    CFDictionaryRef		service_options;

    if (state_dict == NULL) {
	goto done;
    }
    service_options = service_dict_get(serviceID, kSCEntNetService);
    if (service_options != NULL) {
	rank_assertion
	    = CFDictionaryGetValue(service_options,
				   kServiceOptionRankAssertion);
    }
    dict = IPDictCreate(AF_INET6, state_dict, setup_dict, rank_assertion);

  done:
#if	!TARGET_IPHONE_SIMULATOR
    ipv6_service_update_router(serviceID, dict);
#endif /* !TARGET_IPHONE_SIMULATOR */
    changed = service_dict_set(serviceID, kSCEntNetIPv6, dict);
    if (dict == NULL) {
	/* clean up the rank too */
	CFDictionaryRemoveValue(S_ipv6_service_rank_dict, serviceID);
    }
    my_CFRelease(&dict);
    return (changed);
}


#ifdef TEST_DNS
__private_extern__ CFDictionaryRef
ipv4_dict_create(CFDictionaryRef state_dict)
{
    return (IPDictCreate(AF_INET, state_dict, NULL, NULL));
}

__private_extern__ CFDictionaryRef
ipv6_dict_create(CFDictionaryRef state_dict)
{
    return (IPDictCreate(AF_INET6, state_dict, NULL, NULL));
}

#endif /* TEST_DNS */

static void
accumulate_dns_servers(CFArrayRef in_servers, ProtocolFlags active_protos,
		       CFMutableArrayRef out_servers, CFStringRef interface)
{
    CFIndex	count;
    CFIndex	i;

    count = CFArrayGetCount(in_servers);
    for (i = 0; i < count; i++) {
	CFStringRef	addr;
	struct in6_addr	ipv6_addr;
	struct in_addr	ip_addr;

	addr = CFArrayGetValueAtIndex(in_servers, i);
	assert(addr != NULL);

	if (cfstring_to_ip(addr, &ip_addr)) {
	    /* IPv4 address */
	    if ((active_protos & kProtocolFlagsIPv4) == 0
		&& ntohl(ip_addr.s_addr) != INADDR_LOOPBACK) {
		if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
		    my_log(LOG_DEBUG,
			   "IPMonitor: no IPv4 connectivity, "
			   "ignoring DNS server address " IP_FORMAT,
			   IP_LIST(&ip_addr));
		}
		continue;
	    }

	    CFRetain(addr);
	}
	else if (cfstring_to_ip6(addr, &ipv6_addr)) {
	    /* IPv6 address */
	    if ((active_protos & kProtocolFlagsIPv6) == 0
		&& !IN6_IS_ADDR_LOOPBACK(&ipv6_addr)) {
		if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
		    char	ntopbuf[INET6_ADDRSTRLEN];

		    my_log(LOG_DEBUG,
			   "IPMonitor: no IPv6 connectivity, "
			   "ignoring DNS server address %s",
			    inet_ntop(AF_INET6, &ipv6_addr,
				      ntopbuf, sizeof(ntopbuf)));
		}
		continue;
	    }

	    if ((IN6_IS_ADDR_LINKLOCAL(&ipv6_addr) ||
		 IN6_IS_ADDR_MC_LINKLOCAL(&ipv6_addr))
		&& (interface != NULL)
		&& (CFStringFind(addr, CFSTR("%"), 0).location == kCFNotFound)) {
		// append interface name to IPv6 link local address
		addr = CFStringCreateWithFormat(NULL, NULL,
						CFSTR("%@%%%@"),
						addr,
						interface);
	    } else {
		CFRetain(addr);
	    }
	}
	else {
	    /* bad IP address */
	    my_log(LOG_NOTICE,
		   "IPMonitor: ignoring bad DNS server address '%@'",
		   addr);
	    continue;
	}

	/* DNS server is valid and one we want */
	CFArrayAppendValue(out_servers, addr);
	CFRelease(addr);
    }
    return;
}

static void
merge_dns_servers(CFMutableDictionaryRef new_dict,
		  CFArrayRef state_servers,
		  CFArrayRef setup_servers,
		  Boolean have_setup,
		  ProtocolFlags active_protos,
		  CFStringRef interface)
{
    CFMutableArrayRef	dns_servers;
    Boolean		have_dns_setup	= FALSE;

    if (state_servers == NULL && setup_servers == NULL) {
	/* no DNS servers */
	return;
    }
    dns_servers = CFArrayCreateMutable(NULL, 0,
				       &kCFTypeArrayCallBacks);
    if (setup_servers != NULL) {
	accumulate_dns_servers(setup_servers, active_protos,
			       dns_servers, interface);
	if (CFArrayGetCount(dns_servers) > 0) {
	    have_dns_setup = TRUE;
	}
    }
    if ((CFArrayGetCount(dns_servers) == 0 || S_append_state)
	&& state_servers != NULL) {
	accumulate_dns_servers(state_servers, active_protos,
			       dns_servers, NULL);
    }

    /*
     * Here, we determine whether or not we want all queries for this DNS
     * configuration to be bound to the associated network interface.
     *
     * For dynamically derived network configurations (i.e. from State:)
     * this would be the preferred option using the argument "Hey, the
     * server told us to use these servers on this network so let's not
     * argue".
     *
     * But, when a DNS configuration has been provided by the user/admin
     * via the Network pref pane (i.e. from Setup:) we opt to not force
     * binding of the outbound queries.  The simplest example why we take
     * this stance is with a multi-homing configuration.  Consider a system
     * with one network service associated with "en0" and a second service
     * associated with "en1".  The "en0" service has been set higher in
     * the network service order so it would be primary but the user/admin
     * wants the DNS queries to go to a server only accessible via "en1".
     * Without this exception we would take the DNS server addresses from
     * the Network pref pane (for "en0") and have the queries bound to
     * "en0" where they'd never reach their intended destination (via
     * "en1").  So, our exception to the rule is that we will not bind
     * user/admin configurations to any specific network interface.
     *
     * We also add an exception to the "follow the dynamically derived
     * network configuration" path for on-the-fly (no Setup: content)
     * network services.
     */
    if (CFArrayGetCount(dns_servers) != 0) {
	CFDictionarySetValue(new_dict,
			     kSCPropNetDNSServerAddresses, dns_servers);
	if (have_setup && !have_dns_setup) {
	    CFDictionarySetValue(new_dict, DNS_CONFIGURATION_SCOPED_QUERY_KEY, kCFBooleanTrue);
	}
    }

    my_CFRelease(&dns_servers);
    return;
}


static boolean_t
get_dns_changes(CFStringRef serviceID, CFDictionaryRef state_dict,
		CFDictionaryRef setup_dict, CFDictionaryRef info)
{
    ProtocolFlags		active_protos	= kProtocolFlagsNone;
    boolean_t			changed		= FALSE;
    CFStringRef			domain;
    Boolean			have_setup	= FALSE;
    CFStringRef			interface	= NULL;
    CFDictionaryRef		ipv4;
    CFDictionaryRef		ipv6;
    int				i;
    const struct {
	CFStringRef     key;
	uint32_t	flags;
	Boolean		append;
    } merge_list[] = {
	{ kSCPropNetDNSSearchDomains,			0,			FALSE },
	{ kSCPropNetDNSSortList,			0,			FALSE },
	{ kSCPropNetDNSSupplementalMatchDomains,	ALLOW_EMPTY_STRING,	TRUE  },
	{ kSCPropNetDNSSupplementalMatchOrders,		0,			TRUE  },
    };
    CFMutableDictionaryRef      new_dict = NULL;
    const CFStringRef		pick_list[] = {
	kSCPropNetDNSDomainName,
	kSCPropNetDNSOptions,
	kSCPropNetDNSSearchOrder,
	kSCPropNetDNSServerPort,
	kSCPropNetDNSServerTimeout,
	kSCPropNetDNSServiceIdentifier,
	kSCPropNetDNSSupplementalMatchDomainsNoSearch,
    };

    if ((state_dict == NULL) && (setup_dict == NULL)) {
	/* there is no DNS content */
	goto done;
    }

    ipv4 = service_dict_get(serviceID, kSCEntNetIPv4);
    if (ipv4 != NULL) {
	if (get_service_setup_entity(info, serviceID, kSCEntNetIPv4) != NULL) {
	    have_setup = TRUE;
	}
	active_protos |= kProtocolFlagsIPv4;
	interface = ipdict_get_ifname(ipv4);
    }

    ipv6 = service_dict_get(serviceID, kSCEntNetIPv6);
    if (ipv6 != NULL) {
	if (!have_setup
	    && (get_service_setup_entity(info, serviceID, kSCEntNetIPv6)
		!= NULL)) {
	    have_setup = TRUE;
	}
	active_protos |= kProtocolFlagsIPv6;
	if (interface == NULL) {
	    interface = ipdict_get_ifname(ipv6);
	}
    }


    if (active_protos == kProtocolFlagsNone) {
	/* there is no IPv4 nor IPv6 */
	if (state_dict == NULL) {
	    /* ... and no DNS content that we care about */
	    goto done;
	}
	setup_dict = NULL;
    }

    /* merge DNS configuration */
    new_dict = CFDictionaryCreateMutable(NULL, 0,
					 &kCFTypeDictionaryKeyCallBacks,
					 &kCFTypeDictionaryValueCallBacks);

    if (active_protos == kProtocolFlagsNone) {
	merge_dns_servers(new_dict,
			  my_CFDictionaryGetArray(state_dict,
						  kSCPropNetDNSServerAddresses),
			  NULL,
			  FALSE,
			  kProtocolFlagsIPv4 | kProtocolFlagsIPv6,
			  NULL);
    }
    else {
	merge_dns_servers(new_dict,
			  my_CFDictionaryGetArray(state_dict,
						  kSCPropNetDNSServerAddresses),
			  my_CFDictionaryGetArray(setup_dict,
						  kSCPropNetDNSServerAddresses),
			  have_setup,
			  active_protos,
			  interface);
    }

    for (i = 0; i < countof(merge_list); i++) {
	merge_array_prop(new_dict,
			 merge_list[i].key,
			 state_dict,
			 setup_dict,
			 merge_list[i].flags,
			 merge_list[i].append);
    }

    for (i = 0; i < countof(pick_list); i++) {
	pick_prop(new_dict,
		  pick_list[i],
		  state_dict,
		  setup_dict,
		  0);
    }

    if (active_protos == kProtocolFlagsNone) {
	/* there is no IPv4 nor IPv6, only supplemental or service-specific DNS */
	if (CFDictionaryContainsKey(new_dict,
				    kSCPropNetDNSSupplementalMatchDomains)) {
	    /* only keep State: supplemental */
	    CFDictionaryRemoveValue(new_dict, kSCPropNetDNSDomainName);
	    CFDictionaryRemoveValue(new_dict, kSCPropNetDNSSearchDomains);
	    CFDictionaryRemoveValue(new_dict, kSCPropNetDNSSearchOrder);
	    CFDictionaryRemoveValue(new_dict, kSCPropNetDNSSortList);

	    if ((interface == NULL) && (setup_dict == NULL) && (state_dict != NULL)) {
		/*
		 * for supplemental-only configurations, add any scoped (or
		 * wild-card "*") interface
		 */
		interface = CFDictionaryGetValue(state_dict, kSCPropInterfaceName);
	    }
	} else if (CFDictionaryContainsKey(new_dict, kSCPropNetDNSServiceIdentifier) &&
		   (interface == NULL) &&
		   (state_dict != NULL)) {
	    interface = CFDictionaryGetValue(state_dict, kSCPropInterfaceName);
	} else {
	    goto done;
	}
    }

    if (CFDictionaryGetCount(new_dict) == 0) {
	my_CFRelease(&new_dict);
	goto done;
    }

    if (interface != NULL) {
	CFDictionarySetValue(new_dict, kSCPropInterfaceName, interface);
    }

    if (S_append_state) {
	/*
	 * ensure any specified domain name (e.g. the domain returned by
	 * a DHCP server) is in the search list.
	 */
	domain = CFDictionaryGetValue(new_dict, kSCPropNetDNSDomainName);
	if (isA_CFString(domain)) {
	    CFArrayRef      search;

	    search = CFDictionaryGetValue(new_dict, kSCPropNetDNSSearchDomains);
	    if (isA_CFArray(search) &&
		!CFArrayContainsValue(search, CFRangeMake(0, CFArrayGetCount(search)), domain)) {
		CFMutableArrayRef   new_search;

		new_search = CFArrayCreateMutableCopy(NULL, 0, search);
		CFArrayAppendValue(new_search, domain);
		CFDictionarySetValue(new_dict, kSCPropNetDNSSearchDomains, new_search);
		my_CFRelease(&new_search);
	    }
	}
    }

 done:
    changed = service_dict_set(serviceID, kSCEntNetDNS, new_dict);
    my_CFRelease(&new_dict);
    return (changed);
}

static void
merge_dict(const void *key, const void *value, void *context)
{
	CFMutableDictionaryRef	dict	= (CFMutableDictionaryRef)context;

	CFDictionarySetValue(dict, key, value);
	return;
}

#define	PROXY_AUTO_DISCOVERY_URL	252

static CF_RETURNS_RETAINED CFStringRef
wpadURL_dhcp(CFDictionaryRef dhcp_options)
{
    CFStringRef	urlString	= NULL;

    if (dhcp_options != NULL) {
	CFDataRef	data;

	data = DHCPInfoGetOptionData(dhcp_options, PROXY_AUTO_DISCOVERY_URL);
	if (data != NULL) {
	    CFURLRef    url;
	    const UInt8	*urlBytes;
	    CFIndex	urlLen;

	    urlBytes = CFDataGetBytePtr(data);
	    urlLen   = CFDataGetLength(data);
	    while ((urlLen > 0) && (urlBytes[urlLen - 1] == 0)) {
		// remove trailing NUL
		urlLen--;
	    }

	    if (urlLen <= 0) {
		return NULL;
	    }

	    url = CFURLCreateWithBytes(NULL, urlBytes, urlLen, kCFStringEncodingUTF8, NULL);
	    if (url != NULL) {
		urlString = CFURLGetString(url);
		if (urlString != NULL) {
		    CFRetain(urlString);
		}
		CFRelease(url);
	    }
	}
    }

    return urlString;
}

static CF_RETURNS_RETAINED CFStringRef
wpadURL_dns(void)
{
    CFURLRef	url;
    CFStringRef	urlString	= NULL;

    url = CFURLCreateWithString(NULL, CFSTR("http://wpad/wpad.dat"), NULL);
    if (url != NULL) {
	urlString = CFURLGetString(url);
	if (urlString != NULL) {
	    CFRetain(urlString);
	}
	CFRelease(url);
    }

    return urlString;
}

static boolean_t
get_proxies_changes(CFStringRef serviceID, CFDictionaryRef state_dict,
		    CFDictionaryRef setup_dict, CFDictionaryRef info)
{
    ProtocolFlags		active_protos	= kProtocolFlagsNone;
    boolean_t			changed		= FALSE;
    CFStringRef			interface	= NULL;
    CFDictionaryRef		ipv4;
    CFDictionaryRef		ipv6;
    CFMutableDictionaryRef	new_dict	= NULL;
    const struct {
	CFStringRef     key;
	uint32_t	flags;
	Boolean		append;
    } merge_list[] = {
	{ kSCPropNetProxiesSupplementalMatchDomains,	ALLOW_EMPTY_STRING,	TRUE  },
	{ kSCPropNetProxiesSupplementalMatchOrders,	0,			TRUE  },
    };
    const struct {
	    CFStringRef	key1;	/* an "enable" key */
	    CFStringRef	key2;
	    CFStringRef	key3;
    } pick_list[] = {
	    { kSCPropNetProxiesFTPEnable,	kSCPropNetProxiesFTPProxy,	kSCPropNetProxiesFTPPort	},
	    { kSCPropNetProxiesGopherEnable,	kSCPropNetProxiesGopherProxy,	kSCPropNetProxiesGopherPort	},
	    { kSCPropNetProxiesHTTPEnable,	kSCPropNetProxiesHTTPProxy,	kSCPropNetProxiesHTTPPort	},
	    { kSCPropNetProxiesHTTPSEnable,	kSCPropNetProxiesHTTPSProxy,	kSCPropNetProxiesHTTPSPort	},
	    { kSCPropNetProxiesRTSPEnable,	kSCPropNetProxiesRTSPProxy,	kSCPropNetProxiesRTSPPort	},
	    { kSCPropNetProxiesSOCKSEnable,	kSCPropNetProxiesSOCKSProxy,	kSCPropNetProxiesSOCKSPort	},
	    { kSCPropNetProxiesProxyAutoConfigEnable,
	      kSCPropNetProxiesProxyAutoConfigURLString,
	      kSCPropNetProxiesProxyAutoConfigJavaScript, },
	    { kSCPropNetProxiesProxyAutoDiscoveryEnable,
	      NULL,
	      NULL, }
    };

    if ((state_dict == NULL) && (setup_dict == NULL)) {
	/* there is no proxy content */
	goto done;
    }
    ipv4 = service_dict_get(serviceID, kSCEntNetIPv4);
    if (ipdict_get_routelist(ipv4) != NULL) {
	active_protos |= kProtocolFlagsIPv4;
	interface = ipdict_get_ifname(ipv4);
    }
    ipv6 = service_dict_get(serviceID, kSCEntNetIPv6);
    if (ipdict_get_routelist(ipv6) != NULL) {
	active_protos |= kProtocolFlagsIPv6;
	if (interface == NULL) {
	    interface = ipdict_get_ifname(ipv6);
	}
    }
    if (active_protos == kProtocolFlagsNone) {
	/* there is no IPv4 nor IPv6 */
	if (state_dict == NULL) {
	    /* ... and no proxy content that we care about */
	    goto done;
	}
	setup_dict = NULL;
    }

    if ((setup_dict != NULL) && (state_dict != NULL)) {
	CFIndex			i;
	CFMutableDictionaryRef	setup_copy;

	/*
	 * Merge the per-service "Setup:" and "State:" proxy information with
	 * the "Setup:" information always taking precedence.  Additionally,
	 * ensure that if any group of "Setup:" values (e.g. Enabled, Proxy,
	 * Port) is defined than all of the values for that group will be
	 * used.  That is, we don't allow mixing some of the values from
	 * the "Setup:" keys and others from the "State:" keys.
	 */
	new_dict = CFDictionaryCreateMutableCopy(NULL, 0, state_dict);
	for (i = 0; i < countof(merge_list); i++) {
	    merge_array_prop(new_dict,
			     merge_list[i].key,
			     state_dict,
			     setup_dict,
			     merge_list[i].flags,
			     merge_list[i].append);
	}

	setup_copy = CFDictionaryCreateMutableCopy(NULL, 0, setup_dict);
	for (i = 0; i < countof(pick_list); i++) {
	    if (CFDictionaryContainsKey(setup_copy, pick_list[i].key1)) {
		/*
		 * if a "Setup:" enabled key has been provided than we want to
		 * ignore all of the "State:" keys
		 */
		CFDictionaryRemoveValue(new_dict, pick_list[i].key1);
		if (pick_list[i].key2 != NULL) {
		    CFDictionaryRemoveValue(new_dict, pick_list[i].key2);
		}
		if (pick_list[i].key3 != NULL) {
		    CFDictionaryRemoveValue(new_dict, pick_list[i].key3);
		}
	    } else if (CFDictionaryContainsKey(state_dict, pick_list[i].key1) ||
		       ((pick_list[i].key2 != NULL) && CFDictionaryContainsKey(state_dict, pick_list[i].key2)) ||
		       ((pick_list[i].key3 != NULL) && CFDictionaryContainsKey(state_dict, pick_list[i].key3))) {
		/*
		 * if a "Setup:" enabled key has not been provided and we have
		 * some" "State:" keys than we remove all of of "Setup:" keys
		 */
		CFDictionaryRemoveValue(setup_copy, pick_list[i].key1);
		if (pick_list[i].key2 != NULL) {
		    CFDictionaryRemoveValue(setup_copy, pick_list[i].key2);
		}
		if (pick_list[i].key3 != NULL) {
		    CFDictionaryRemoveValue(setup_copy, pick_list[i].key3);
		}
	    }
	}

	/* merge the "Setup:" keys */
	CFDictionaryApplyFunction(setup_copy, merge_dict, new_dict);
	CFRelease(setup_copy);
    }
    else if (setup_dict != NULL) {
	new_dict = CFDictionaryCreateMutableCopy(NULL, 0, setup_dict);
    }
    else if (state_dict != NULL) {
	new_dict = CFDictionaryCreateMutableCopy(NULL, 0, state_dict);
    }

    if ((new_dict != NULL) && (CFDictionaryGetCount(new_dict) == 0)) {
	CFRelease(new_dict);
	new_dict = NULL;
    }

    if ((new_dict != NULL) && (interface != NULL)) {
	CFDictionarySetValue(new_dict, kSCPropInterfaceName, interface);
    }

    /* process WPAD */
    if (new_dict != NULL) {
	CFDictionaryRef	dhcp_options;
	CFNumberRef	num;
	CFNumberRef	wpad	    = NULL;
	int		wpadEnabled = 0;
	CFStringRef	wpadURL	    = NULL;

	if (CFDictionaryGetValueIfPresent(new_dict,
					  kSCPropNetProxiesProxyAutoDiscoveryEnable,
					  (const void **)&num) &&
	    isA_CFNumber(num)) {
	    /* if we have a WPAD key */
	    wpad = num;
	    if (!CFNumberGetValue(num, kCFNumberIntType, &wpadEnabled)) {
		/* if we don't like the enabled key/value */
		wpadEnabled = 0;
	    }
	}

	if (wpadEnabled) {
	    int	pacEnabled  = 0;

	    num = CFDictionaryGetValue(new_dict, kSCPropNetProxiesProxyAutoConfigEnable);
	    if (!isA_CFNumber(num) ||
		!CFNumberGetValue(num, kCFNumberIntType, &pacEnabled)) {
		/* if we don't like the enabled key/value */
		pacEnabled = 0;
	    }

	    if (pacEnabled) {
		CFStringRef	pacURL;

		pacURL = CFDictionaryGetValue(new_dict, kSCPropNetProxiesProxyAutoConfigURLString);
		if (pacURL != NULL) {
		    if (!isA_CFString(pacURL)) {
			/* if we don't like the PAC URL */
			pacEnabled = 0;
		    }
		} else {
		    CFStringRef	pacJS;

		    pacJS = CFDictionaryGetValue(new_dict, kSCPropNetProxiesProxyAutoConfigJavaScript);
		    if (!isA_CFString(pacJS)) {
			/* if we don't have (or like) the PAC JavaScript */
			pacEnabled = 0;
		    }
		}
	    }

	    if (pacEnabled) {
		/*
		 * we already have a PAC URL so disable WPAD.
		 */
		wpadEnabled = 0;
		goto setWPAD;
	    }

	    /*
	     * if WPAD is enabled and we don't already have a PAC URL then
	     * we check for a DHCP provided URL.  If not available, we use
	     * a PAC URL pointing to a well-known file (wpad.dat) on a
	     * well-known host (wpad.<domain>).
	     */
	    dhcp_options = get_service_state_entity(info, serviceID, kSCEntNetDHCP);
	    wpadURL = wpadURL_dhcp(dhcp_options);
	    if (wpadURL == NULL) {
		wpadURL = wpadURL_dns();
	    }
	    if (wpadURL == NULL) {
		wpadEnabled = 0;    /* if we don't have a WPAD URL */
		goto setWPAD;
	    }

	    pacEnabled = 1;
	    num = CFNumberCreate(NULL, kCFNumberIntType, &pacEnabled);
	    CFDictionarySetValue(new_dict,
				 kSCPropNetProxiesProxyAutoConfigEnable,
				 num);
	    CFRelease(num);
	    CFDictionarySetValue(new_dict,
				 kSCPropNetProxiesProxyAutoConfigURLString,
				 wpadURL);
	    CFRelease(wpadURL);
	}

     setWPAD:
	if (wpad != NULL) {
	    num = CFNumberCreate(NULL, kCFNumberIntType, &wpadEnabled);
	    CFDictionarySetValue(new_dict,
				 kSCPropNetProxiesProxyAutoDiscoveryEnable,
				 num);
	    CFRelease(num);
	}
    }

 done:
    changed = service_dict_set(serviceID, kSCEntNetProxies, new_dict);
    my_CFRelease(&new_dict);
    return (changed);
}

#if	!TARGET_OS_IPHONE
static boolean_t
get_smb_changes(CFStringRef serviceID, CFDictionaryRef state_dict,
		CFDictionaryRef setup_dict, CFDictionaryRef info)
{
    boolean_t			changed = FALSE;
    int				i;
    CFMutableDictionaryRef      new_dict = NULL;
    const CFStringRef		pick_list[] = {
	kSCPropNetSMBNetBIOSName,
	kSCPropNetSMBNetBIOSNodeType,
#ifdef	ADD_NETBIOS_SCOPE
	kSCPropNetSMBNetBIOSScope,
#endif	// ADD_NETBIOS_SCOPE
	kSCPropNetSMBWorkgroup,
    };

    if (state_dict == NULL && setup_dict == NULL) {
	/* there is no SMB */
	goto done;
    }
    if (service_dict_get(serviceID, kSCEntNetIPv4) == NULL
	&& service_dict_get(serviceID, kSCEntNetIPv6) == NULL) {
	/* there is no IPv4 or IPv6 */
	goto done;
    }

    /* merge SMB configuration */
    new_dict = CFDictionaryCreateMutable(NULL, 0,
					 &kCFTypeDictionaryKeyCallBacks,
					 &kCFTypeDictionaryValueCallBacks);
    merge_array_prop(new_dict,
		     kSCPropNetSMBWINSAddresses,
		     state_dict,
		     setup_dict,
		     0,
		     FALSE);
    for (i = 0; i < countof(pick_list); i++) {
	pick_prop(new_dict,
		  pick_list[i],
		  state_dict,
		  setup_dict,
		  0);
    }

    if (CFDictionaryGetCount(new_dict) == 0) {
	my_CFRelease(&new_dict);
	goto done;
    }

 done:
    changed = service_dict_set(serviceID, kSCEntNetSMB, new_dict);
    my_CFRelease(&new_dict);
    return (changed);
}
#endif	/* !TARGET_OS_IPHONE */

static CFStringRef
services_info_get_interface(CFDictionaryRef services_info,
			    CFStringRef serviceID)
{
    CFStringRef		interface = NULL;
    CFDictionaryRef	ipv4_dict;

    ipv4_dict = get_service_state_entity(services_info, serviceID,
					 kSCEntNetIPv4);
    if (ipv4_dict != NULL) {
	interface = CFDictionaryGetValue(ipv4_dict, kSCPropInterfaceName);
    }
    else {
	CFDictionaryRef		ipv6_dict;

	ipv6_dict = get_service_state_entity(services_info, serviceID,
					     kSCEntNetIPv6);
	if (ipv6_dict != NULL) {
	    interface = CFDictionaryGetValue(ipv6_dict, kSCPropInterfaceName);
	}
    }
    return (interface);
}


static const struct {
    const CFStringRef *	entityName;
    const CFStringRef *	statusKey;
} transientServiceInfo[] = {
    { &kSCEntNetIPSec,	&kSCPropNetIPSecStatus	},
    { &kSCEntNetPPP,	&kSCPropNetPPPStatus	},
    { &kSCEntNetVPN,	&kSCPropNetVPNStatus	},
};

static Boolean
get_transient_status_changes(CFStringRef serviceID,
			     CFDictionaryRef services_info)
{
    boolean_t	changed = FALSE;
    int		i;

    for (i = 0; i < countof(transientServiceInfo); i++) {
	CFDictionaryRef		dict;
	CFNumberRef		status		= NULL;
	CFMutableDictionaryRef	ts_dict		= NULL;

	dict = get_service_state_entity(services_info, serviceID,
					*transientServiceInfo[i].entityName);

	if (dict != NULL) {
	    status = CFDictionaryGetValue(dict,
					  *transientServiceInfo[i].statusKey);
	}

	if (isA_CFNumber(status) != NULL) {
	    ts_dict = CFDictionaryCreateMutable(NULL,
						 0,
						 &kCFTypeDictionaryKeyCallBacks,
						 &kCFTypeDictionaryValueCallBacks);
	    CFDictionaryAddValue(ts_dict,
				 *transientServiceInfo[i].statusKey,
				 status);
	}

	if (service_dict_set(serviceID, *transientServiceInfo[i].entityName,
			     ts_dict)) {
	    changed = TRUE;
	}

	if (ts_dict != NULL) {
	    CFRelease(ts_dict);
	}
    }
    return (changed);
}

static boolean_t
service_is_expensive(CFStringRef serviceID, CFDictionaryRef services_info)
{
    CFStringRef		ifname;
    boolean_t		is_expensive = FALSE;

    ifname = services_info_get_interface(services_info, serviceID);
    if (ifname != NULL) {
	CFDictionaryRef		if_dict;
	CFStringRef		key;

	key = interface_entity_key_copy(ifname, kSCEntNetLink);
	if_dict = CFDictionaryGetValue(services_info, key);
	CFRelease(key);
	if (isA_CFDictionary(if_dict) != NULL) {
	    CFBooleanRef	expensive;

	    expensive = CFDictionaryGetValue(if_dict, kSCPropNetLinkExpensive);
	    if (isA_CFBoolean(expensive) != NULL
		&& CFBooleanGetValue(expensive)) {
		is_expensive = TRUE;
	    }
	}
    }
    return (is_expensive);
}

static boolean_t
get_rank_changes(CFStringRef serviceID, CFDictionaryRef state_options,
		 CFDictionaryRef setup_options, CFDictionaryRef services_info)
{
    boolean_t			changed		= FALSE;
    boolean_t			ip_is_coupled	= FALSE;
    CFMutableDictionaryRef      new_dict	= NULL;
    Rank			rank_assertion = kRankAssertionDefault;
    Boolean			rank_assertion_is_set = FALSE;
    CFStringRef			setup_rank	= NULL;
    CFStringRef			state_rank	= NULL;


    if (state_options != NULL) {
	CFBooleanRef	coupled;

	state_rank
	    = CFDictionaryGetValue(state_options, kSCPropNetServicePrimaryRank);
	state_rank = isA_CFString(state_rank);
	coupled = CFDictionaryGetValue(state_options, kIPIsCoupled);
	if (isA_CFBoolean(coupled) != NULL && CFBooleanGetValue(coupled)) {
	    ip_is_coupled = TRUE;
	}
    }
    if (setup_options != NULL) {
	CFBooleanRef	coupled;

	setup_rank
	    = CFDictionaryGetValue(setup_options, kSCPropNetServicePrimaryRank);
	setup_rank = isA_CFString(setup_rank);
	coupled = CFDictionaryGetValue(setup_options, kIPIsCoupled);
	if (isA_CFBoolean(coupled) != NULL && CFBooleanGetValue(coupled)) {
	    ip_is_coupled = TRUE;
	}
    }

    if (ip_is_coupled == FALSE) {
	ip_is_coupled = service_is_expensive(serviceID, services_info);
    }
    if (setup_rank != NULL || state_rank != NULL) {
	/* rank assertion is set on the service */
	Rank	setup_assertion;
	Rank	state_assertion;
	Boolean	state_assertion_is_set = FALSE;

	setup_assertion = PrimaryRankGetRankAssertion(setup_rank, NULL);
	state_assertion = PrimaryRankGetRankAssertion(state_rank,
						      &state_assertion_is_set);
	if (setup_assertion > state_assertion) {
	    rank_assertion = setup_assertion;
	    rank_assertion_is_set = TRUE;
	}
	else if (state_assertion_is_set) {
	    rank_assertion = state_assertion;
	    rank_assertion_is_set = TRUE;
	}
    }

    if (rank_assertion_is_set == FALSE) {
	/* check for a rank assertion on the interface */
	CFStringRef interface;

	interface = services_info_get_interface(services_info, serviceID);
	if (interface != NULL) {
	    CFNumberRef	if_rank = NULL;

	    if (S_if_rank_dict != NULL) {
		if_rank = CFDictionaryGetValue(S_if_rank_dict, interface);
	    }
	    rank_assertion
		= InterfaceRankGetRankAssertion(if_rank,
						&rank_assertion_is_set);
	    if (S_IPMonitor_debug & kDebugFlag1) {
		my_log(LOG_DEBUG,
		       "serviceID %@ interface %@ rank = %@",
		       serviceID, interface, if_rank);
	    }
	}
    }


    if (rank_assertion_is_set || ip_is_coupled) {
	new_dict = CFDictionaryCreateMutable(NULL, 0,
					     &kCFTypeDictionaryKeyCallBacks,
					     &kCFTypeDictionaryValueCallBacks);
	if (rank_assertion_is_set) {
	    CFNumberRef		new_rank;

	    new_rank = CFNumberCreate(NULL, kCFNumberSInt32Type,
				      (const void *)&rank_assertion);
	    CFDictionarySetValue(new_dict, kServiceOptionRankAssertion,
				 new_rank);
	    CFRelease(new_rank);
	}
	if (ip_is_coupled) {
	    CFDictionarySetValue(new_dict, kIPIsCoupled, kCFBooleanTrue);
	}
    }
    changed = service_dict_set(serviceID, kSCEntNetService, new_dict);
    my_CFRelease(&new_dict);
    return (changed);
}

static void
add_service_keys(CFStringRef serviceID,
		 CFMutableArrayRef keys, CFMutableArrayRef patterns)
{
    int			i;
    CFStringRef		key;

    if (CFEqual(serviceID, kSCCompAnyRegex)) {
	keys = patterns;
    }

    for (i = 0; i < ENTITY_TYPES_COUNT; i++) {
	key = setup_service_key(serviceID, *entityTypeNames[i]);
	CFArrayAppendValue(keys, key);
	CFRelease(key);
	key = state_service_key(serviceID, *entityTypeNames[i]);
	CFArrayAppendValue(keys, key);
	CFRelease(key);
    }

    key = state_service_key(serviceID, kSCEntNetDHCP);
    CFArrayAppendValue(patterns, key);
    CFRelease(key);

    key = setup_service_key(serviceID, NULL);
    CFArrayAppendValue(patterns, key);
    CFRelease(key);
    key = state_service_key(serviceID, NULL);
    CFArrayAppendValue(patterns, key);
    CFRelease(key);

    return;
}

static void
add_transient_status_keys(CFStringRef service_id, CFMutableArrayRef patterns)
{
    int	    i;

    for (i = 0; i < countof(transientServiceInfo); i++) {
	CFStringRef	pattern;

	pattern = state_service_key(service_id,
				    *transientServiceInfo[i].entityName);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);
    }

    return;
}

static const CFStringRef *reachabilitySetupKeys[] = {
    &kSCEntNetPPP,
    &kSCEntNetInterface,
    &kSCEntNetIPv4,
    &kSCEntNetIPv6,
};


static void
add_reachability_patterns(CFMutableArrayRef patterns)
{
    int		i;

    for (i = 0; i < countof(reachabilitySetupKeys); i++) {
	CFStringRef pattern;
	pattern = setup_service_key(kSCCompAnyRegex, *reachabilitySetupKeys[i]);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);
    }
}


static void
add_vpn_pattern(CFMutableArrayRef patterns)
{
    CFStringRef	pattern;

    pattern = setup_service_key(kSCCompAnyRegex, kSCEntNetVPN);
    CFArrayAppendValue(patterns, pattern);
    CFRelease(pattern);
}

static void
add_interface_link_pattern(CFMutableArrayRef patterns)
{
    CFStringRef	pattern;

    pattern = interface_entity_key_copy(kSCCompAnyRegex, kSCEntNetLink);
    CFArrayAppendValue(patterns, pattern);
    CFRelease(pattern);
}

static CFDictionaryRef
services_info_copy(SCDynamicStoreRef session, CFArrayRef service_list)
{
    CFIndex		count;
    CFMutableArrayRef	get_keys;
    CFMutableArrayRef	get_patterns;
    CFDictionaryRef	info;
    CFIndex		s;

    count = CFArrayGetCount(service_list);
    get_keys = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    get_patterns = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

    CFArrayAppendValue(get_keys, S_setup_global_ipv4);
    CFArrayAppendValue(get_keys, S_multicast_resolvers);
    CFArrayAppendValue(get_keys, S_private_resolvers);

    for (s = 0; s < count; s++) {
	CFStringRef	serviceID = CFArrayGetValueAtIndex(service_list, s);

	add_service_keys(serviceID, get_keys, get_patterns);
	add_transient_status_keys(serviceID, get_keys);
    }

    add_reachability_patterns(get_patterns);

    add_vpn_pattern(get_patterns);

    add_interface_link_pattern(get_patterns);

    info = SCDynamicStoreCopyMultiple(session, get_keys, get_patterns);
    my_CFRelease(&get_keys);
    my_CFRelease(&get_patterns);
    return (info);
}

#if	!TARGET_IPHONE_SIMULATOR

static int
multicast_route(int sockfd, int cmd)
{
    IPv4Route	route;

    bzero(&route, sizeof(route));
    route.dest.s_addr = htonl(INADDR_UNSPEC_GROUP);
    route.mask.s_addr = htonl(IN_CLASSD_NET);
    route.ifindex = lo0_ifindex();
    return (IPv4RouteApply((RouteRef)&route, cmd, sockfd));
}

#endif	/* !TARGET_IPHONE_SIMULATOR */

#if	!TARGET_IPHONE_SIMULATOR

static boolean_t
set_ipv6_default_interface(IFIndex ifindex)
{
    struct in6_ndifreq	ndifreq;
    int			sock;
    boolean_t		success = FALSE;

    bzero((char *)&ndifreq, sizeof(ndifreq));
    strlcpy(ndifreq.ifname, kLoopbackInterface, sizeof(ndifreq.ifname));
    if (ifindex != 0) {
	ndifreq.ifindex = ifindex;
    }
    else {
	ndifreq.ifindex = lo0_ifindex();
    }
    sock = inet6_dgram_socket();
    if (sock == -1) {
	my_log(LOG_ERR,
	       "IPMonitor: set_ipv6_default_interface: socket failed, %s",
	       strerror(errno));
    }
    else {
	if (ioctl(sock, SIOCSDEFIFACE_IN6, (caddr_t)&ndifreq) == -1) {
	    my_log(LOG_ERR,
		   "IPMonitor: ioctl(SIOCSDEFIFACE_IN6) failed, %s",
		   strerror(errno));
	}
	else {
	    success = TRUE;
	}
	close(sock);
    }
    return (success);
}

#endif	/* !TARGET_IPHONE_SIMULATOR */

#if	!TARGET_OS_IPHONE
static __inline__ void
empty_dns()
{
    (void)unlink(VAR_RUN_RESOLV_CONF);
}

static void
set_dns(CFArrayRef val_search_domains,
	CFStringRef val_domain_name,
	CFArrayRef val_servers,
	CFArrayRef val_sortlist)
{
    FILE * f = fopen(VAR_RUN_RESOLV_CONF "-", "w");

    /* publish new resolv.conf */
    if (f) {
	CFIndex	i;
	CFIndex	n;

	SCPrint(TRUE, f, CFSTR("#\n"));
	SCPrint(TRUE, f, CFSTR("# Mac OS X Notice\n"));
	SCPrint(TRUE, f, CFSTR("#\n"));
	SCPrint(TRUE, f, CFSTR("# This file is not used by the host name and address resolution\n"));
	SCPrint(TRUE, f, CFSTR("# or the DNS query routing mechanisms used by most processes on\n"));
	SCPrint(TRUE, f, CFSTR("# this Mac OS X system.\n"));
	SCPrint(TRUE, f, CFSTR("#\n"));
	SCPrint(TRUE, f, CFSTR("# This file is automatically generated.\n"));
	SCPrint(TRUE, f, CFSTR("#\n"));

	if (isA_CFArray(val_search_domains)) {
	    SCPrint(TRUE, f, CFSTR("search"));
	    n = CFArrayGetCount(val_search_domains);
	    for (i = 0; i < n; i++) {
		CFStringRef	domain;

		domain = CFArrayGetValueAtIndex(val_search_domains, i);
		if (isA_CFString(domain)) {
		    SCPrint(TRUE, f, CFSTR(" %@"), domain);
		}
	    }
	    SCPrint(TRUE, f, CFSTR("\n"));
	}
	else if (isA_CFString(val_domain_name)) {
		SCPrint(TRUE, f, CFSTR("domain %@\n"), val_domain_name);
	}

	if (isA_CFArray(val_servers)) {
	    n = CFArrayGetCount(val_servers);
	    for (i = 0; i < n; i++) {
		CFStringRef	nameserver;

		nameserver = CFArrayGetValueAtIndex(val_servers, i);
		if (isA_CFString(nameserver)) {
		    SCPrint(TRUE, f, CFSTR("nameserver %@\n"), nameserver);
		}
	    }
	}

	if (isA_CFArray(val_sortlist)) {
	    SCPrint(TRUE, f, CFSTR("sortlist"));
	    n = CFArrayGetCount(val_sortlist);
	    for (i = 0; i < n; i++) {
		CFStringRef	address;

		address = CFArrayGetValueAtIndex(val_sortlist, i);
		if (isA_CFString(address)) {
		    SCPrint(TRUE, f, CFSTR(" %@"), address);
		}
	    }
	    SCPrint(TRUE, f, CFSTR("\n"));
	}

	fclose(f);
	rename(VAR_RUN_RESOLV_CONF "-", VAR_RUN_RESOLV_CONF);
    }
    return;
}
#endif	/* !TARGET_OS_IPHONE */

static boolean_t
service_get_ip_is_coupled(CFStringRef serviceID)
{
    CFDictionaryRef	dict;
    boolean_t		ip_is_coupled = FALSE;

    dict = service_dict_get(serviceID, kSCEntNetService);
    if (dict != NULL) {
	if (CFDictionaryContainsKey(dict, kIPIsCoupled)) {
	    ip_is_coupled = TRUE;
	}
    }
    return (ip_is_coupled);
}

static CFStringRef
my_CFStringCreateWithInAddr(struct in_addr ip)
{
    CFStringRef	str;

    str = CFStringCreateWithFormat(NULL, NULL, CFSTR(IP_FORMAT), IP_LIST(&ip));
    return (str);
}

static CFStringRef
my_CFStringCreateWithIn6Addr(const struct in6_addr * ip)
{
    char	ntopbuf[INET6_ADDRSTRLEN];

    (void)inet_ntop(AF_INET6, ip, ntopbuf, sizeof(ntopbuf));
    return (CFStringCreateWithFormat(NULL, NULL, CFSTR("%s"), ntopbuf));
}

/*
 * Function: update_ipv4
 * Purpose:
 *   Update the IPv4 configuration based on the latest information.
 *   Publish the State:/Network/Global/IPv4 information, and update the
 *   IPv4 routing table.
 */
static void
update_ipv4(CFStringRef		primary,
	    IPv4RouteListRef	new_routelist,
	    keyChangeListRef	keys)
{
#if	!TARGET_IPHONE_SIMULATOR
    int		sockfd;
#endif	/* !TARGET_IPHONE_SIMULATOR */

    if (keys != NULL) {
	if (new_routelist != NULL && primary != NULL) {
	    const char *		ifn_p = NULL;
	    char			ifname[IFNAMSIZ];
	    IPv4RouteRef		r;
	    CFMutableDictionaryRef	dict = NULL;

	    dict = CFDictionaryCreateMutable(NULL, 0,
					     &kCFTypeDictionaryKeyCallBacks,
					     &kCFTypeDictionaryValueCallBacks);
	    /* the first entry is the default route */
	    r = new_routelist->list;
	    if (r->gateway.s_addr != 0) {
		CFStringRef		str;

		str = my_CFStringCreateWithInAddr(r->gateway);
		CFDictionarySetValue(dict, kSCPropNetIPv4Router, str);
		CFRelease(str);
	    }
	    ifn_p = my_if_indextoname(r->ifindex, ifname);
	    if (ifn_p != NULL) {
		CFStringRef		ifname_cf;

		ifname_cf = CFStringCreateWithCString(NULL,
						      ifn_p,
						      kCFStringEncodingASCII);
		if (ifname_cf != NULL) {
		    CFDictionarySetValue(dict,
					 kSCDynamicStorePropNetPrimaryInterface,
					 ifname_cf);
		    CFRelease(ifname_cf);
		}
	    }
	    CFDictionarySetValue(dict, kSCDynamicStorePropNetPrimaryService,
				 primary);
	    keyChangeListSetValue(keys, S_state_global_ipv4, dict);
	    CFRelease(dict);
	}
	else {
	    keyChangeListRemoveValue(keys, S_state_global_ipv4);
	}
    }

#if	!TARGET_IPHONE_SIMULATOR
    sockfd = open_routing_socket();
    if (sockfd != -1) {
	if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
	    if (S_ipv4_routelist == NULL) {
		my_log(LOG_DEBUG, "Old Routes = <none>");
	    }
	    else {
		my_log(LOG_DEBUG, "Old Routes = ");
		IPv4RouteListLog(LOG_DEBUG, S_ipv4_routelist);
	    }
	    if (new_routelist == NULL) {
		my_log(LOG_DEBUG, "New Routes = <none>");
	    }
	    else {
		my_log(LOG_DEBUG, "New Routes = ");
		IPv4RouteListLog(LOG_DEBUG, new_routelist);
	    }
	}
	/* go through routelist and bind any unbound routes */
	IPv4RouteListFinalize(new_routelist);
	IPv4RouteListApply(S_ipv4_routelist, new_routelist, sockfd);
	if (new_routelist != NULL) {
	    (void)multicast_route(sockfd, RTM_DELETE);
	}
	else {
	    (void)multicast_route(sockfd, RTM_ADD);
	}
	close(sockfd);
    }
    if (S_ipv4_routelist != NULL) {
	free(S_ipv4_routelist);
    }
    S_ipv4_routelist = new_routelist;
#else 	/* !TARGET_IPHONE_SIMULATOR */
    if (new_routelist != NULL) {
	free(new_routelist);
    }
#endif	/* !TARGET_IPHONE_SIMULATOR */

    return;
}

/*
 * Function: update_ipv6
 * Purpose:
 *   Update the IPv6 configuration based on the latest information.
 *   Publish the State:/Network/Global/IPv6 information, and update the
 *   IPv6 routing table.
 */
static void
update_ipv6(CFStringRef		primary,
	    IPv6RouteListRef	new_routelist,
	    keyChangeListRef	keys)
{
#if	!TARGET_IPHONE_SIMULATOR
    int		sockfd;
#endif	/* !TARGET_IPHONE_SIMULATOR */

    if (keys != NULL) {
	if (new_routelist != NULL && primary != NULL) {
	    const char *		ifn_p = NULL;
	    char			ifname[IFNAMSIZ];
	    IPv6RouteRef		r;
	    CFMutableDictionaryRef	dict = NULL;

	    dict = CFDictionaryCreateMutable(NULL, 0,
					     &kCFTypeDictionaryKeyCallBacks,
					     &kCFTypeDictionaryValueCallBacks);
	    /* the first entry is the default route */
	    r = new_routelist->list;
	    if ((r->flags & kRouteFlagsHasGateway) != 0) {
		CFStringRef		router;

		router = my_CFStringCreateWithIn6Addr(&r->gateway);
		CFDictionarySetValue(dict, kSCPropNetIPv6Router, router);
		CFRelease(router);
	    }
	    ifn_p = my_if_indextoname(r->ifindex, ifname);
	    if (ifn_p != NULL) {
		CFStringRef		ifname_cf;

		ifname_cf = CFStringCreateWithCString(NULL,
						      ifn_p,
						      kCFStringEncodingASCII);
		if (ifname_cf != NULL) {
		    CFDictionarySetValue(dict,
					 kSCDynamicStorePropNetPrimaryInterface,
					 ifname_cf);
		    CFRelease(ifname_cf);
		}
	    }
	    CFDictionarySetValue(dict, kSCDynamicStorePropNetPrimaryService,
				 primary);
	    keyChangeListSetValue(keys, S_state_global_ipv6, dict);
	    CFRelease(dict);
#if	!TARGET_IPHONE_SIMULATOR
	    if (S_scopedroute_v6) {
		set_ipv6_default_interface(r->ifindex);
	    }
#endif	/* !TARGET_IPHONE_SIMULATOR */
	}
	else {
#if	!TARGET_IPHONE_SIMULATOR
	    if (S_scopedroute_v6) {
		set_ipv6_default_interface(0);
	    }
#endif	/* !TARGET_IPHONE_SIMULATOR */
	    keyChangeListRemoveValue(keys, S_state_global_ipv6);
	}
    }

#if	!TARGET_IPHONE_SIMULATOR
    sockfd = open_routing_socket();
    if (sockfd != -1) {
	if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
	    if (S_ipv6_routelist == NULL) {
		my_log(LOG_DEBUG, "Old Routes = <none>");
	    }
	    else {
		my_log(LOG_DEBUG, "Old Routes = ");
		IPv6RouteListLog(LOG_DEBUG, S_ipv6_routelist);
	    }
	    if (new_routelist == NULL) {
		my_log(LOG_DEBUG, "New Routes = <none>");
	    }
	    else {
		my_log(LOG_DEBUG, "New Routes = ");
		IPv6RouteListLog(LOG_DEBUG, new_routelist);
	    }
	}
	/* go through routelist and bind any unbound routes */
	IPv6RouteListFinalize(new_routelist);
	IPv6RouteListApply(S_ipv6_routelist, new_routelist, sockfd);
	close(sockfd);
    }
    if (S_ipv6_routelist != NULL) {
	free(S_ipv6_routelist);
    }
    S_ipv6_routelist = new_routelist;
#else 	/* !TARGET_IPHONE_SIMULATOR */
    if (new_routelist != NULL) {
	free(new_routelist);
    }
#endif	/* !TARGET_IPHONE_SIMULATOR */

    return;
}

static Boolean
update_dns(CFDictionaryRef	services_info,
	   CFStringRef		primary,
	   keyChangeListRef	keys)
{
    Boolean		changed	= FALSE;
    CFDictionaryRef	dict	= NULL;

    if (primary != NULL) {
	CFDictionaryRef	service_dict;

	service_dict = CFDictionaryGetValue(S_service_state_dict, primary);
	if (service_dict != NULL) {
	    dict = CFDictionaryGetValue(service_dict, kSCEntNetDNS);
	}
    }

    if (!_SC_CFEqual(S_dns_dict, dict)) {
	if (dict == NULL) {
#if	!TARGET_OS_IPHONE
	    empty_dns();
#endif	/* !TARGET_OS_IPHONE */
	    keyChangeListRemoveValue(keys, S_state_global_dns);
	} else {
	    CFMutableDictionaryRef	new_dict;

#if	!TARGET_OS_IPHONE
	    set_dns(CFDictionaryGetValue(dict, kSCPropNetDNSSearchDomains),
		    CFDictionaryGetValue(dict, kSCPropNetDNSDomainName),
		    CFDictionaryGetValue(dict, kSCPropNetDNSServerAddresses),
		    CFDictionaryGetValue(dict, kSCPropNetDNSSortList));
#endif	/* !TARGET_OS_IPHONE */
	    new_dict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
	    CFDictionaryRemoveValue(new_dict, kSCPropInterfaceName);
	    CFDictionaryRemoveValue(new_dict, kSCPropNetDNSSupplementalMatchDomains);
	    CFDictionaryRemoveValue(new_dict, kSCPropNetDNSSupplementalMatchOrders);
	    CFDictionaryRemoveValue(new_dict, DNS_CONFIGURATION_SCOPED_QUERY_KEY);
	    keyChangeListSetValue(keys, S_state_global_dns, new_dict);
	    CFRelease(new_dict);
	}
	changed = TRUE;
    }

    if (dict != NULL) CFRetain(dict);
    if (S_dns_dict != NULL) CFRelease(S_dns_dict);
    S_dns_dict = dict;

    return changed;
}

static Boolean
update_dnsinfo(CFDictionaryRef	services_info,
	       CFStringRef	primary,
	       keyChangeListRef	keys,
	       CFArrayRef	service_order)
{
    Boolean		changed;
    CFDictionaryRef	dict	= NULL;
    CFArrayRef		multicastResolvers;
    CFArrayRef		privateResolvers;

    multicastResolvers = CFDictionaryGetValue(services_info, S_multicast_resolvers);
    privateResolvers   = CFDictionaryGetValue(services_info, S_private_resolvers);

    if (primary != NULL) {
	CFDictionaryRef	service_dict;

	service_dict = CFDictionaryGetValue(S_service_state_dict, primary);
	if (service_dict != NULL) {
	    dict = CFDictionaryGetValue(service_dict, kSCEntNetDNS);
	}
    }

    changed = dns_configuration_set(dict,
				    S_service_state_dict,
				    service_order,
				    multicastResolvers,
				    privateResolvers);
    if (changed) {
	keyChangeListNotifyKey(keys, S_state_global_dns);
    }
    return changed;
}

static Boolean
update_nwi(nwi_state_t state)
{
    unsigned char		signature[CC_SHA1_DIGEST_LENGTH];
    static unsigned char	signature_last[CC_SHA1_DIGEST_LENGTH];

    _nwi_state_signature(state, signature, sizeof(signature));
    if (bcmp(signature, signature_last, sizeof(signature)) == 0) {
	return FALSE;
    }

    // save [new] signature
    bcopy(signature, signature_last, sizeof(signature));

    // save [new] configuration
    if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
	my_log(LOG_DEBUG, "Updating network information");
	S_nwi_state_dump(state);
    }
    if (_nwi_state_store(state) == FALSE) {
	my_log(LOG_ERR, "Notifying nwi_state_store failed");
    }

    return TRUE;
}

static Boolean
update_proxies(CFDictionaryRef	services_info,
	       CFStringRef	primary,
	       keyChangeListRef	keys,
	       CFArrayRef	service_order)
{
    Boolean	    changed	= FALSE;
    CFDictionaryRef dict	= NULL;
    CFDictionaryRef new_dict;

    if (primary != NULL) {
	CFDictionaryRef	service_dict;

	service_dict = CFDictionaryGetValue(S_service_state_dict, primary);
	if (service_dict != NULL) {
	    dict = CFDictionaryGetValue(service_dict, kSCEntNetProxies);
	}
    }

    new_dict = proxy_configuration_update(dict,
					  S_service_state_dict,
					  service_order,
					  services_info);
    if (!_SC_CFEqual(S_proxies_dict, new_dict)) {
	if (new_dict == NULL) {
	    keyChangeListRemoveValue(keys, S_state_global_proxies);
	} else {
	    keyChangeListSetValue(keys, S_state_global_proxies, new_dict);
	}
	changed = TRUE;
    }

    if (S_proxies_dict != NULL) CFRelease(S_proxies_dict);
    S_proxies_dict = new_dict;

    return changed;
}

#if	!TARGET_OS_IPHONE
static Boolean
update_smb(CFDictionaryRef	services_info,
	   CFStringRef		primary,
	   keyChangeListRef	keys)
{
    Boolean		changed	= FALSE;
    CFDictionaryRef	dict	= NULL;

    if (primary != NULL) {
	CFDictionaryRef	service_dict;

	service_dict = CFDictionaryGetValue(S_service_state_dict, primary);
	if (service_dict != NULL) {
	    dict = CFDictionaryGetValue(service_dict, kSCEntNetSMB);
	}
    }

    if (!_SC_CFEqual(S_smb_dict, dict)) {
	if (dict == NULL) {
	    keyChangeListRemoveValue(keys, S_state_global_smb);
	} else {
	    keyChangeListSetValue(keys, S_state_global_smb, dict);
	}
	changed = TRUE;
    }

    if (dict != NULL) CFRetain(dict);
    if (S_smb_dict != NULL) CFRelease(S_smb_dict);
    S_smb_dict = dict;

    return changed;
}
#endif	/* !TARGET_OS_IPHONE */

static Rank
get_service_rank(CFArrayRef order, int n_order, CFStringRef serviceID)
{
    CFIndex		i;
    Rank		rank = kRankIndexMask;

    if (serviceID != NULL && order != NULL && n_order > 0) {
	for (i = 0; i < n_order; i++) {
	    CFStringRef s = isA_CFString(CFArrayGetValueAtIndex(order, i));

	    if (s == NULL) {
		continue;
	    }
	    if (CFEqual(serviceID, s)) {
		rank = (Rank)i + 1;
		break;
	    }
	}
    }
    return (rank);
}

/**
 ** Service election:
 **/
/*
 * Function: rank_dict_get_service_rank
 * Purpose:
 *   Retrieve the service rank in the given dictionary.
 */
static Rank
rank_dict_get_service_rank(CFDictionaryRef rank_dict, CFStringRef serviceID)
{
    CFNumberRef		rank;
    Rank		rank_val;

    rank_val = RankMake(kRankIndexMask, kRankAssertionDefault);
    rank = CFDictionaryGetValue(rank_dict, serviceID);
    if (rank != NULL) {
	CFNumberGetValue(rank, kCFNumberSInt32Type, &rank_val);
    }
    return (rank_val);
}

/*
 * Function: rank_dict_set_service_rank
 * Purpose:
 *   Save the results of ranking the service so we can look it up later without
 *   repeating all of the ranking code.
 */
static void
rank_dict_set_service_rank(CFMutableDictionaryRef rank_dict,
			   CFStringRef serviceID, Rank rank_val)
{
    CFNumberRef		rank;

    rank = CFNumberCreate(NULL, kCFNumberSInt32Type, (const void *)&rank_val);
    if (rank != NULL) {
	CFDictionarySetValue(rank_dict, serviceID, rank);
	CFRelease(rank);
    }
    return;
}

static const CFStringRef *transientInterfaceEntityNames[] = {
    &kSCEntNetPPP,
};


static void
CollectTransientServices(const void * key,
			 const void * value,
			 void * context)
{
    int			i;
    CFStringRef		service = key;
    CFMutableArrayRef	vif_setup_keys = context;

    /* This service is either a vpn type service or a comm center service */
    if (!CFStringHasPrefix(service, kSCDynamicStoreDomainSetup)) {
	return;
    }

    for (i = 0; i < countof(transientInterfaceEntityNames); i++) {
	if (CFStringHasSuffix(service, *transientInterfaceEntityNames[i])) {
	    CFArrayAppendValue(vif_setup_keys, service);
	    break;
	}
    }

    return;
}


static SCNetworkReachabilityFlags
GetReachabilityFlagsFromVPN(CFDictionaryRef services_info,
			    CFStringRef	    service_id,
			    CFStringRef	    entity,
			    CFStringRef	    vpn_setup_key)
{
    CFStringRef			key;
    CFDictionaryRef		dict;
    SCNetworkReachabilityFlags	flags = 0;


    key = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
						      kSCDynamicStoreDomainSetup,
						      service_id,
						      kSCEntNetInterface);
    dict = CFDictionaryGetValue(services_info, key);
    CFRelease(key);

    if (isA_CFDictionary(dict)
	&& CFDictionaryContainsKey(dict, kSCPropNetInterfaceDeviceName)) {

	flags = (kSCNetworkReachabilityFlagsReachable
		| kSCNetworkReachabilityFlagsTransientConnection
		| kSCNetworkReachabilityFlagsConnectionRequired);

	if (CFEqual(entity, kSCEntNetPPP)) {
	    CFNumberRef	num;
	    CFDictionaryRef p_dict = CFDictionaryGetValue(services_info, vpn_setup_key);

	    if (!isA_CFDictionary(p_dict)) {
		return (flags);
	    }

	    // get PPP dial-on-traffic status
	    num = CFDictionaryGetValue(p_dict, kSCPropNetPPPDialOnDemand);
	    if (isA_CFNumber(num)) {
		int32_t	ppp_demand;

		if (CFNumberGetValue(num, kCFNumberSInt32Type, &ppp_demand)) {
		    if (ppp_demand) {
			flags |= kSCNetworkReachabilityFlagsConnectionOnTraffic;
		    }
		}
	    }
	}
    }
    return (flags);
}

static Boolean
S_dict_get_boolean(CFDictionaryRef dict, CFStringRef key, Boolean def_value)
{
    Boolean		ret = def_value;

    if (dict != NULL) {
	CFBooleanRef	val;

	val = CFDictionaryGetValue(dict, key);
	if (isA_CFBoolean(val) != NULL) {
	    ret = CFBooleanGetValue(val);
	}
    }
    return (ret);
}


static void
GetReachabilityFlagsFromTransientServices(CFDictionaryRef services_info,
					  SCNetworkReachabilityFlags *reach_flags_v4,
					  SCNetworkReachabilityFlags *reach_flags_v6)
{
    CFIndex		i;
    CFIndex		count;
    CFMutableArrayRef	vif_setup_keys;

    vif_setup_keys = CFArrayCreateMutable(NULL,
					  0,
					  &kCFTypeArrayCallBacks);
    CFDictionaryApplyFunction(services_info, CollectTransientServices,
			      vif_setup_keys);
    count = CFArrayGetCount(vif_setup_keys);
    for (i = 0; i < count; i++) {
	CFArrayRef	    components = NULL;
	CFStringRef	    entity;
	CFStringRef	    service_id;
	CFStringRef	    vif_setup_key;

	vif_setup_key = CFArrayGetValueAtIndex(vif_setup_keys, i);

	/*
	 * setup key in the following format:
	 * Setup:/Network/Service/<Service ID>/<Entity>
	 */
	components = CFStringCreateArrayBySeparatingStrings(NULL, vif_setup_key, CFSTR("/"));

	if (CFArrayGetCount(components) != 5) {
	    // invalid Setup key encountered
	    goto skip;
	}

	/* service id is the 3rd element */
	service_id = CFArrayGetValueAtIndex(components, 3);

	/* entity id is the 4th element */
	entity = CFArrayGetValueAtIndex(components, 4);


	if (CFEqual(entity, kSCEntNetPPP)) {
	    SCNetworkReachabilityFlags	flags;
	    CFStringRef			key;

	    flags = GetReachabilityFlagsFromVPN(services_info,
						service_id,
						entity,
						vif_setup_key);

	    /* Check for the v4 reachability flags */
	    key = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainSetup,
							      service_id,
							      kSCEntNetIPv4);

	    if (CFDictionaryContainsKey(services_info, key)) {
		*reach_flags_v4 |= flags;
		my_log(LOG_DEBUG, "Service %@ setting ipv4 reach flags: %d", service_id, *reach_flags_v4);
	    }

	    CFRelease(key);

	    /* Check for the v6 reachability flags */
	    key = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainSetup,
							      service_id,
							      kSCEntNetIPv6);

	    if (CFDictionaryContainsKey(services_info, key)) {
		*reach_flags_v6 |= flags;
		my_log(LOG_DEBUG, "Service %@ setting ipv6 reach flags: %d", service_id, *reach_flags_v6);
	    }
	    CFRelease(key);

	    if (flags != 0) {
		if (components != NULL) {
		    CFRelease(components);
		}
		goto done;
	    }
	}
skip:
	if (components != NULL) {
	    CFRelease(components);
	}
    }
done:
    CFRelease(vif_setup_keys);
    return;
}

static SCNetworkReachabilityFlags
GetReachFlagsFromStatus(CFStringRef entity, int status)
{
    SCNetworkReachabilityFlags flags = 0;

    if (CFEqual(entity, kSCEntNetPPP)) {
	switch (status) {
	    case PPP_RUNNING :
		/* if we're really UP and RUNNING */
		break;
	    case PPP_ONHOLD :
		/* if we're effectively UP and RUNNING */
		break;
	    case PPP_IDLE :
		/* if we're not connected at all */
		flags |= kSCNetworkReachabilityFlagsConnectionRequired;
		break;
	    case PPP_STATERESERVED :
		// if we're not connected at all
		flags |= kSCNetworkReachabilityFlagsConnectionRequired;
		break;
	    default :
		/* if we're in the process of [dis]connecting */
		flags |= kSCNetworkReachabilityFlagsConnectionRequired;
		break;
	}
    }
    else if (CFEqual(entity, kSCEntNetIPSec)) {
	switch (status) {
	    case IPSEC_RUNNING :
		/* if we're really UP and RUNNING */
		break;
	    case IPSEC_IDLE :
		/* if we're not connected at all */
		flags |= kSCNetworkReachabilityFlagsConnectionRequired;
		break;
	    default :
		/* if we're in the process of [dis]connecting */
		flags |= kSCNetworkReachabilityFlagsConnectionRequired;
		break;
	}
    }
    else if  (CFEqual(entity, kSCEntNetVPN)) {
	switch (status) {
	    case VPN_RUNNING :
		/* if we're really UP and RUNNING */
		break;
	    case VPN_IDLE :
	    case VPN_LOADING :
	    case VPN_LOADED :
	    case VPN_UNLOADING :
		/* if we're not connected at all */
		flags |= kSCNetworkReachabilityFlagsConnectionRequired;
		break;
	    default :
		/* if we're in the process of [dis]connecting */
		flags |= kSCNetworkReachabilityFlagsConnectionRequired;
		break;
	}
    }
    return (flags);
}

static void
VPNAttributesGet(CFStringRef		    service_id,
		 CFDictionaryRef	    services_info,
		 SCNetworkReachabilityFlags *flags,
		 CFStringRef		    *server_address,
		 int			    af)
{
    int				i;
    CFDictionaryRef		entity_dict;
    CFNumberRef			num;
    CFDictionaryRef		p_state = NULL;
    int				status = 0;
    CFStringRef  		transient_entity = NULL;

    if (af == AF_INET) {
	entity_dict = service_dict_get(service_id, kSCEntNetIPv4);
    }
    else {
	entity_dict = service_dict_get(service_id, kSCEntNetIPv6);
    }
    entity_dict = ipdict_get_service(entity_dict);
    if (entity_dict == NULL) {
	return;
    }

    for (i = 0; i < countof(transientServiceInfo); i++) {
	CFStringRef	entity = *transientServiceInfo[i].entityName;

	p_state = service_dict_get(service_id, entity);

	/* ensure that this is a VPN Type service */
	if (isA_CFDictionary(p_state)) {
	    transient_entity = entity;
	    break;
	}
    }

    /* Did we find a vpn type service?  If not, we are done.*/
    if (transient_entity == NULL) {
	return;
    }

    *flags |= (kSCNetworkReachabilityFlagsReachable
	       | kSCNetworkReachabilityFlagsTransientConnection);

    /* Get the Server Address */
    if (server_address != NULL) {
	*server_address = CFDictionaryGetValue(entity_dict,
					       CFSTR("ServerAddress"));
	*server_address = isA_CFString(*server_address);
	if (*server_address != NULL) {
	    CFRetain(*server_address);
	}
    }

    /* get status */
    if (!CFDictionaryGetValueIfPresent(p_state,
				       kSCPropNetVPNStatus,		// IPSecStatus, PPPStatus, VPNStatus
				       (const void **)&num) ||
	!isA_CFNumber(num) ||
	!CFNumberGetValue(num, kCFNumberSInt32Type, &status)) {
	return;
    }

    *flags |= GetReachFlagsFromStatus(transient_entity, status);
    if (CFEqual(transient_entity, kSCEntNetPPP)) {
	CFStringRef	key;
	CFDictionaryRef p_setup;
	int		ppp_demand;

	key = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							  kSCDynamicStoreDomainSetup,
							  service_id,
							  kSCEntNetPPP);
	p_setup = CFDictionaryGetValue(services_info, key);
	CFRelease(key);

	/* get dial-on-traffic status */
	if (isA_CFDictionary(p_setup) &&
	    CFDictionaryGetValueIfPresent(p_setup,
					  kSCPropNetPPPDialOnDemand,
					  (const void **)&num) &&
	    isA_CFNumber(num) &&
	    CFNumberGetValue(num, kCFNumberSInt32Type, &ppp_demand) &&
	    (ppp_demand != 0)) {
	    *flags |= kSCNetworkReachabilityFlagsConnectionOnTraffic;
	    if (status == PPP_IDLE) {
		*flags |= kSCNetworkReachabilityFlagsInterventionRequired;
	    }
	}
    }
    return;
}


typedef struct ElectionInfo {
    int				af;
    CFStringRef			entity;
    int				n_services;
    CFArrayRef			order;
    int				n_order;
    ElectionResultsRef		results;
    CFMutableDictionaryRef	rank_dict;
} ElectionInfo, * ElectionInfoRef;

typedef CFDictionaryApplierFunction	ElectionFuncRef;

static void
CandidateRelease(CandidateRef candidate)
{
    my_CFRelease(&candidate->serviceID);
    my_CFRelease(&candidate->if_name);
    my_CFRelease(&candidate->signature);
    return;
}

static void
CandidateCopy(CandidateRef dest, CandidateRef src)
{
    *dest = *src;
    if (dest->serviceID) {
	CFRetain(dest->serviceID);
    }
    if (dest->if_name) {
	CFRetain(dest->if_name);
    }
    if(dest->signature) {
	CFRetain(dest->signature);
    }
    return;
}

static ElectionResultsRef
ElectionResultsAlloc(int af, int size)
{
    ElectionResultsRef results;

    results = (ElectionResultsRef)malloc(ElectionResultsComputeSize(size));
    results->af = af;
    results->count = 0;
    results->size = size;
    return (results);
}

static void
ElectionResultsRelease(ElectionResultsRef results)
{
    int			i;
    CandidateRef	scan;

    for (i = 0, scan = results->candidates;
	 i < results->count;
	 i++, scan++) {
	CandidateRelease(scan);
    }
    free(results);
    return;
}

static void
ElectionResultsLog(int level, ElectionResultsRef results, const char * prefix)
{
    int			i;
    CandidateRef	scan;

    if (results == NULL) {
	my_log(level, "%s: no candidates", prefix);
	return;
    }
    my_log(level, "%s: %d candidates", prefix, results->count);
    for (i = 0, scan = results->candidates;
	 i < results->count;
	 i++, scan++) {
	char	ntopbuf[INET6_ADDRSTRLEN];

	(void)inet_ntop(results->af, &scan->addr, ntopbuf, sizeof(ntopbuf));
	my_log(level, "%d. %@ serviceID=%@ addr=%s rank=0x%x",
	       i, scan->if_name, scan->serviceID, ntopbuf, scan->rank);
    }
    return;
}

/*
 * Function: ElectionResultsAddCandidate
 * Purpose:
 *   Add the candidate into the election results. Find the insertion point
 *   by comparing the rank of the candidate with existing entries.
 */
static void
ElectionResultsAddCandidate(ElectionResultsRef results, CandidateRef candidate)
{
    CFIndex		i;
    CFIndex		where;

    if (results->count == results->size) {
	/* this should not happen */
	my_log(LOG_NOTICE, "can't fit another candidate");
	return;
    }

    /* find the insertion point */
    where = kCFNotFound;
    for (i = 0; i < results->count; i++) {
	CandidateRef	this_candidate = results->candidates + i;

	if (candidate->rank < this_candidate->rank) {
	    where = i;
	    break;
	}
    }
    /* add it to the end */
    if (where == kCFNotFound) {
	CandidateCopy(results->candidates + results->count, candidate);
	results->count++;
	return;
    }
    /* slide existing entries over */
    for (i = results->count; i > where; i--) {
	results->candidates[i] = results->candidates[i - 1];
    }
    /* insert element */
    CandidateCopy(results->candidates + where, candidate);
    results->count++;
    return;
}

static void
elect_ip(const void * key, const void * value, void * context);

/*
 * Function: ElectionResultsCopy
 * Purpose:
 *   Visit all of the services and invoke the protocol-specific election
 *   function.  Return the results of the election.
 */
static ElectionResultsRef
ElectionResultsCopy(int af, CFArrayRef order, int n_order)
{
    int			count;
    ElectionInfo	info;

    count = (int)CFDictionaryGetCount(S_service_state_dict);
    if (count == 0) {
	return (NULL);
    }
    info.af = af;
    if (af == AF_INET) {
	info.entity = kSCEntNetIPv4;
	info.rank_dict = S_ipv4_service_rank_dict;
    }
    else {
	info.entity = kSCEntNetIPv6;
	info.rank_dict = S_ipv6_service_rank_dict;
    }
    info.results = ElectionResultsAlloc(af, count);
    info.n_services = count;
    info.order = order;
    info.n_order = n_order;
    CFDictionaryApplyFunction(S_service_state_dict, elect_ip, (void *)&info);
    if (info.results->count == 0) {
	ElectionResultsRelease(info.results);
	info.results = NULL;
    }
    return (info.results);
}

/*
 * Function: ElectionResultsCandidateNeedsDemotion
 * Purpose:
 *   Check whether the given candidate requires demotion. A candidate
 *   might need to be demoted if its IPv4 and IPv6 services must be coupled
 *   but a higher ranked service has IPv4 or IPv6.
 */
static Boolean
ElectionResultsCandidateNeedsDemotion(ElectionResultsRef other_results,
				      CandidateRef candidate)
{
    CandidateRef	other_candidate;
    Boolean		ret = FALSE;

    if (other_results == NULL
	|| candidate->ip_is_coupled == FALSE
	|| RANK_ASSERTION_MASK(candidate->rank) == kRankAssertionNever) {
	goto done;
    }
    other_candidate = other_results->candidates;
    if (CFEqual(other_candidate->if_name, candidate->if_name)) {
	/* they are over the same interface, no need to demote */
	goto done;
    }
    if (CFStringHasPrefix(other_candidate->if_name, CFSTR("stf"))) {
	/* avoid creating a feedback loop */
	goto done;
    }
    if (RANK_ASSERTION_MASK(other_candidate->rank) == kRankAssertionNever) {
	/* the other candidate isn't eligible to become primary, ignore */
	goto done;
    }
    if (candidate->rank < other_candidate->rank) {
	/* we're higher ranked than the other candidate, ignore */
	goto done;
    }
    ret = TRUE;

 done:
    return (ret);

}


static void
get_signature_sha1(CFStringRef		signature,
		   unsigned char	* sha1)
{
    CC_SHA1_CTX	    ctx;
    CFDataRef	    signature_data;

    signature_data = CFStringCreateExternalRepresentation(NULL,
							  signature,
							  kCFStringEncodingUTF8,
							  0);

    CC_SHA1_Init(&ctx);
    CC_SHA1_Update(&ctx,
		   signature_data,
		   (CC_LONG)CFDataGetLength(signature_data));
    CC_SHA1_Final(sha1, &ctx);

    CFRelease(signature_data);

    return;
}


static void
add_candidate_to_nwi_state(nwi_state_t nwi_state, int af,
			   CandidateRef candidate, Rank rank)
{
    uint64_t		flags = 0;
    char		ifname[IFNAMSIZ];
    nwi_ifstate_t	ifstate;

    if (nwi_state == NULL) {
	/* can't happen */
	return;
    }
    if (RANK_ASSERTION_MASK(rank) == kRankAssertionNever) {
	flags |= NWI_IFSTATE_FLAGS_NOT_IN_LIST;
    }
    if (service_dict_get(candidate->serviceID, kSCEntNetDNS) != NULL) {
	flags |= NWI_IFSTATE_FLAGS_HAS_DNS;
    }
    CFStringGetCString(candidate->if_name, ifname, sizeof(ifname),
		       kCFStringEncodingASCII);
    if ((S_IPMonitor_debug & kDebugFlag2) != 0) {
	char	ntopbuf[INET6_ADDRSTRLEN];

	(void)inet_ntop(af, &candidate->addr, ntopbuf, sizeof(ntopbuf));
	my_log(LOG_DEBUG,
	       "Inserting IPv%c [%s] %s "
	       "with flags 0x%llx rank 0x%x reach_flags 0x%x",
	       ipvx_char(af), ifname, ntopbuf,
	       flags, rank, candidate->reachability_flags);
    }
    ifstate = nwi_insert_ifstate(nwi_state, ifname, af, flags, rank,
				 (void *)&candidate->addr,
				 (void *)&candidate->vpn_server_addr,
				 candidate->reachability_flags);
    if (ifstate != NULL && candidate->signature) {
	uint8_t	    hash[CC_SHA1_DIGEST_LENGTH];

	get_signature_sha1(candidate->signature, hash);
	nwi_ifstate_set_signature(ifstate, hash);
    }
    return;
}


static void
add_reachability_flags_to_candidate(CandidateRef candidate, CFDictionaryRef services_info, int af)
{
    SCNetworkReachabilityFlags	flags = kSCNetworkReachabilityFlagsReachable;
    CFStringRef			vpn_server_address = NULL;

    VPNAttributesGet(candidate->serviceID,
		     services_info,
		     &flags,
		     &vpn_server_address,
		     af);

    candidate->reachability_flags = flags;

    if (vpn_server_address == NULL) {
	bzero(&candidate->vpn_server_addr, sizeof(candidate->vpn_server_addr));
    } else {
	char buf[128];

	CFStringGetCString(vpn_server_address, buf, sizeof(buf),
			   kCFStringEncodingASCII);
	_SC_string_to_sockaddr(buf,
			       AF_UNSPEC,
			       (void *)&candidate->vpn_server_addr,
			       sizeof(candidate->vpn_server_addr));

	CFRelease(vpn_server_address);
    }
    return;
}
/*
 * Function: ElectionResultsCopyPrimary
 * Purpose:
 *   Use the results of the current protocol and the other protocol to
 *   determine which service should become primary.
 *
 *   At the same time, generate the IPv4/IPv6 routing table and
 *   the nwi_state for the protocol.
 */
static CFStringRef
ElectionResultsCopyPrimary(ElectionResultsRef results,
			   ElectionResultsRef other_results,
			   nwi_state_t nwi_state, int af,
			   RouteListRef * ret_routes,
			   CFDictionaryRef services_info)
{
    CFStringRef		primary = NULL;
    Boolean		primary_is_null = FALSE;
    RouteListRef	routes = NULL;

    if (nwi_state != NULL) {
	nwi_state_clear(nwi_state, af);
    }
    if (results != NULL) {
	CandidateRef		deferred[results->count];
	int			deferred_count;
	CFStringRef		entity_name;
	int			i;
	int			initial_size;
	RouteListInfoRef	info;
	CandidateRef		scan;

	switch (af) {
	case AF_INET:
	    entity_name = kSCEntNetIPv4;
	    info = &IPv4RouteListInfo;
	    initial_size = results->count * IPV4_ROUTES_N_STATIC;
	    break;
	default:
	case AF_INET6:
	    entity_name = kSCEntNetIPv6;
	    info = &IPv6RouteListInfo;
	    initial_size = results->count * IPV6_ROUTES_N_STATIC;
	    break;
	}
	deferred_count = 0;
	for (i = 0, scan = results->candidates;
	     i < results->count;
	     i++, scan++) {
	    Boolean		is_primary = FALSE;
	    Rank		rank = scan->rank;
	    CFDictionaryRef	service_dict;
	    RouteListRef	service_routes;
	    Boolean		skip = FALSE;

	    if (primary == NULL
		&& RANK_ASSERTION_MASK(rank) != kRankAssertionNever) {
		if (ElectionResultsCandidateNeedsDemotion(other_results,
							  scan)) {
		    /* demote to RankNever */
		    my_log(LOG_NOTICE,
			   "IPv%c over %@ demoted: not primary for IPv%c",
			   ipvx_char(af), scan->if_name, ipvx_other_char(af));
		    rank = RankMake(rank, kRankAssertionNever);
		    deferred[deferred_count++] = scan;
		    skip = TRUE;
		}
		else {
		    primary = CFRetain(scan->serviceID);
		    is_primary = TRUE;
		}
	    }
	    /* contribute to the routing table */
	    service_dict = service_dict_get(scan->serviceID, entity_name);
	    service_routes = ipdict_get_routelist(service_dict);
	    if (service_routes != NULL) {
		routes = RouteListAddRouteList(info, routes, initial_size,
					       service_routes, rank);
		if ((service_routes->flags & kRouteListFlagsExcludeNWI) != 0) {
		    skip = TRUE;
		}
	    }
	    else {
		skip = TRUE;
	    }
	    if (skip) {
		/* if we're skipping the primary, it's NULL */
		if (is_primary) {
		    primary_is_null = TRUE;
		}
	    }
	    else {
		if (primary_is_null) {
		    /* everything after the primary must be Never */
		    rank = RankMake(rank, kRankAssertionNever);
		}
		add_reachability_flags_to_candidate(scan, services_info, af);
		add_candidate_to_nwi_state(nwi_state, af, scan, rank);
	    }
	}
	for (i = 0; i < deferred_count; i++) {
	    CandidateRef	candidate = deferred[i];
	    Rank		rank;

	    /* demote to RankNever */
	    rank = RankMake(candidate->rank, kRankAssertionNever);
	    add_reachability_flags_to_candidate(candidate, services_info, af);
	    add_candidate_to_nwi_state(nwi_state, af, candidate, rank);
	}
    }
    if (nwi_state != NULL) {
	nwi_state_set_last(nwi_state, af);
    }
    if (ret_routes != NULL) {
	*ret_routes = routes;
    }
    else if (routes != NULL) {
	free(routes);
    }
    if (primary_is_null) {
	my_CFRelease(&primary);
    }
    return (primary);
}


static inline
CFStringRef
service_dict_get_signature(CFDictionaryRef service_dict)
{
    CFStringRef		ifname;

    ifname = CFDictionaryGetValue(service_dict, kSCPropInterfaceName);
    if (isA_CFString(ifname) == NULL
	|| confirm_interface_name(service_dict, ifname) == FALSE) {
	return (NULL);
    }
    return (CFDictionaryGetValue(service_dict, kStoreKeyNetworkSignature));
}

/*
 * Function: elect_ip
 * Purpose:
 *   Evaluate the service and determine what rank the service should have.
 *   If it's a suitable candidate, add it to the election results.
 */
static void
elect_ip(const void * key, const void * value, void * context)
{
    CFDictionaryRef	all_entities_dict = (CFDictionaryRef)value;
    Candidate		candidate;
    Rank		default_rank;
    ElectionInfoRef 	elect_info;
    CFStringRef		if_name;
    CFDictionaryRef	ipdict;
    Rank		primary_rank;
    RouteListUnion	routelist;
    CFDictionaryRef	service_dict;

    elect_info = (ElectionInfoRef)context;
    ipdict = CFDictionaryGetValue(all_entities_dict, elect_info->entity);
    if (ipdict != NULL) {
	routelist.ptr = ipdict_get_routelist(ipdict);
	service_dict = ipdict_get_service(ipdict);
    }
    else {
	routelist.ptr = NULL;
    }
    if (routelist.ptr == NULL || service_dict == NULL) {
	/* no connectivity */
	return;
    }
    if ((routelist.common->flags & kRouteListFlagsHasDefault) == 0) {
	/* no default route, not a candidate for being primary */
	return;
    }
    if_name = CFDictionaryGetValue(service_dict, kSCPropInterfaceName);
    if (if_name == NULL) {
	/* need an interface name */
	return;
    }
    if (CFEqual(if_name, CFSTR(kLoopbackInterface))) {
	/* don't process loopback */
	return;
    }
    bzero(&candidate, sizeof(candidate));
    candidate.serviceID = (CFStringRef)key;
    candidate.rank = get_service_rank(elect_info->order, elect_info->n_order,
				      candidate.serviceID);
    if (elect_info->af == AF_INET) {
	default_rank = routelist.v4->list->rank;
	candidate.addr.v4 = routelist.v4->list->ifa;
    }
    else {
	default_rank = routelist.v6->list->rank;
	candidate.addr.v6 = routelist.v6->list->ifa;
    }
    primary_rank = RANK_ASSERTION_MASK(default_rank);
    if (S_ppp_override_primary) {
	char	ifn[IFNAMSIZ];

	if (CFStringGetCString(if_name, ifn, sizeof(ifn),
			       kCFStringEncodingASCII)
	    && (strncmp(PPP_PREFIX, ifn, sizeof(PPP_PREFIX) - 1) == 0)) {
	    /* PPP override: make ppp* look the best */
	    primary_rank = kRankAssertionFirst;
	}
    }
    candidate.rank = RankMake(candidate.rank, primary_rank);
    candidate.ip_is_coupled = service_get_ip_is_coupled(candidate.serviceID);
    candidate.if_name = if_name;
    rank_dict_set_service_rank(elect_info->rank_dict,
			       candidate.serviceID, candidate.rank);
    candidate.signature = service_dict_get_signature(service_dict);
    ElectionResultsAddCandidate(elect_info->results, &candidate);
    return;
}


static uint32_t
service_changed(CFDictionaryRef services_info, CFStringRef serviceID)
{
    uint32_t		changed = 0;
    int			i;

    /* update service options first (e.g. rank) */
    if (get_rank_changes(serviceID,
			 get_service_state_entity(services_info, serviceID,
						  NULL),
			 get_service_setup_entity(services_info, serviceID,
						  NULL),
			 services_info)) {
	changed |= (1 << kEntityTypeServiceOptions);
    }
    /* update IPv4, IPv6, DNS, Proxies, SMB, ... */
    for (i = 0; i < ENTITY_TYPES_COUNT; i++) {
	GetEntityChangesFuncRef func = entityChangeFunc[i];
	if ((*func)(serviceID,
		    get_service_state_entity(services_info, serviceID,
					     *entityTypeNames[i]),
		    get_service_setup_entity(services_info, serviceID,
					     *entityTypeNames[i]),
		    services_info)) {
	    changed |= (1 << i);
	}
    }

    if (get_transient_status_changes(serviceID, services_info)) {
	changed |= (1 << kEntityTypeTransientStatus);
    }

    return (changed);
}

static CFArrayRef
service_order_get(CFDictionaryRef services_info)
{
    CFArrayRef		order = NULL;
    CFDictionaryRef	ipv4_dict;

    ipv4_dict = my_CFDictionaryGetDictionary(services_info,
					     S_setup_global_ipv4);
    if (ipv4_dict != NULL) {
	CFNumberRef	ppp_override;
	int		ppp_val = 0;

	order = CFDictionaryGetValue(ipv4_dict, kSCPropNetServiceOrder);
	order = isA_CFArray(order);

	/* get ppp override primary */
	ppp_override = CFDictionaryGetValue(ipv4_dict,
					    kSCPropNetPPPOverridePrimary);
	ppp_override = isA_CFNumber(ppp_override);
	if (ppp_override != NULL) {
	    CFNumberGetValue(ppp_override, kCFNumberIntType, &ppp_val);
	}
	S_ppp_override_primary = (ppp_val != 0) ? TRUE : FALSE;
    }
    else {
	S_ppp_override_primary = FALSE;
    }
    return (order);
}

static boolean_t
set_new_primary(CFStringRef * primary_p, CFStringRef new_primary,
		const char * entity)
{
    boolean_t		changed = FALSE;
    CFStringRef		primary = *primary_p;

    if (new_primary != NULL) {
	if (primary != NULL && CFEqual(new_primary, primary)) {
	    if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
		my_log(LOG_DEBUG,
		       "IPMonitor: %@ is still primary %s",
		       new_primary, entity);
	    }
	}
	else {
	    my_CFRelease(primary_p);
	    *primary_p = CFRetain(new_primary);
	    if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
		my_log(LOG_DEBUG,
		       "IPMonitor: %@ is the new primary %s",
		       new_primary, entity);
	    }
	    changed = TRUE;
	}
    }
    else if (primary != NULL) {
	if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
	    my_log(LOG_DEBUG,
		   "IPMonitor: %@ is no longer primary %s",
		   primary, entity);
	}
	my_CFRelease(primary_p);
	changed = TRUE;
    }
    return (changed);
}

static Rank
rank_service_entity(CFDictionaryRef rank_dict, CFStringRef serviceID,
		    CFStringRef entity)
{
    if (service_dict_get(serviceID, entity) == NULL) {
	return (RankMake(kRankIndexMask, kRankAssertionDefault));
    }
    return (rank_dict_get_service_rank(rank_dict, serviceID));
}

static void
append_serviceIDs_for_interface(CFMutableArrayRef services_changed,
				CFStringRef ifname)
{
    CFIndex		count;
    CFIndex		i;
    void * *		keys;
#define N_KEYS_VALUES_STATIC    10
    void *		keys_values_buf[N_KEYS_VALUES_STATIC * 2];
    void * *		values;

    count = CFDictionaryGetCount(S_service_state_dict);
    if (count <= N_KEYS_VALUES_STATIC) {
	keys = keys_values_buf;
    } else {
	keys = (void * *)malloc(sizeof(*keys) * count * 2);
    }
    values = keys + count;
    CFDictionaryGetKeysAndValues(S_service_state_dict,
				 (const void * *)keys,
				 (const void * *)values);

    for (i = 0; i < count; i++) {
	CFDictionaryRef		ipdict = NULL;
	CFStringRef		interface = NULL;
	CFStringRef		serviceID;
	CFDictionaryRef		service_dict;

	serviceID = (CFStringRef)keys[i];
	service_dict = (CFDictionaryRef)values[i];

	/* check whether service has IPv4 or IPv6 */
	ipdict = CFDictionaryGetValue(service_dict, kSCEntNetIPv4);
	if (ipdict == NULL) {
	    ipdict = CFDictionaryGetValue(service_dict, kSCEntNetIPv6);
	    if (ipdict == NULL) {
		continue;
	    }
	}
	interface = ipdict_get_ifname(ipdict);
	if (interface != NULL && CFEqual(interface, ifname)) {
	    if (S_IPMonitor_debug & kDebugFlag1) {
		my_log(LOG_DEBUG,
		       "Found IP service %@ on interface %@.",
		       serviceID, ifname);
	    }
	    my_CFArrayAppendUniqueValue(services_changed, serviceID);
	}
    }
    if (keys != keys_values_buf) {
	free(keys);
    }
    return;
}

static __inline__ const char *
get_changed_str(CFStringRef serviceID, CFStringRef entity,
		CFDictionaryRef old_dict)
{
    CFDictionaryRef new_dict    = NULL;

    if (serviceID != NULL) {
	new_dict = service_dict_get(serviceID, entity);
    }

    if (old_dict == NULL) {
	if (new_dict != NULL) {
	    return "+";
	}
    } else {
	if (new_dict == NULL) {
	    return "-";
	} else if (!CFEqual(old_dict, new_dict)) {
	    return "!";
	}
    }
    return "";
}

static CF_RETURNS_RETAINED CFStringRef
generate_log_changes(nwi_state_t	changes_state,
		     boolean_t		dns_changed,
		     boolean_t		dnsinfo_changed,
		     CFDictionaryRef	old_primary_dns,
		     boolean_t		proxy_changed,
		     CFDictionaryRef	old_primary_proxy,
		     boolean_t		smb_changed,
		     CFDictionaryRef	old_primary_smb
		     )
{
    int idx;
    CFMutableStringRef log_output;
    nwi_ifstate_t scan;

    log_output = CFStringCreateMutable(NULL, 0);

    if (changes_state != NULL) {
	for (idx = 0; idx < countof(nwi_af_list); idx++) {
	    CFMutableStringRef changes = NULL;
	    CFMutableStringRef primary_str = NULL;

	    scan = nwi_state_get_first_ifstate(changes_state, nwi_af_list[idx]);

	    while (scan != NULL) {
		const char * changed_str;

		changed_str = nwi_ifstate_get_diff_str(scan);
		if (changed_str != NULL) {
		    void *		address;
		    const char *	addr_str;
		    char		ntopbuf[INET6_ADDRSTRLEN];

		    address = (void *)nwi_ifstate_get_address(scan);
		    addr_str = inet_ntop(scan->af, address, ntopbuf,
					 sizeof(ntopbuf));
		    if (primary_str ==  NULL) {
			primary_str = CFStringCreateMutable(NULL, 0);
			CFStringAppendFormat(primary_str, NULL,
					     CFSTR("%s%s:%s"),
					     nwi_ifstate_get_ifname(scan),
					     changed_str, addr_str);
		    } else {
			if (changes == NULL) {
			    changes = CFStringCreateMutable(NULL, 0);
			}
			CFStringAppendFormat(changes, NULL, CFSTR(", %s"),
					     nwi_ifstate_get_ifname(scan));
			if (strcmp(changed_str,  "") != 0) {
			    CFStringAppendFormat(changes, NULL, CFSTR("%s:%s"),
						 changed_str, addr_str);
			}
		    }
		}
		scan = nwi_ifstate_get_next(scan, scan->af);
	    }

	    if (primary_str != NULL) {
		CFStringAppendFormat(log_output, NULL, CFSTR(" %s(%@"),
				     nwi_af_list[idx] == AF_INET ? "v4" : "v6",
				     primary_str);

		if (changes != NULL && CFStringGetLength(changes) != 0) {
		    CFStringAppendFormat(log_output, NULL, CFSTR("%@"),
					 changes);
		}
		CFStringAppend(log_output, CFSTR(")"));

		my_CFRelease(&primary_str);
		my_CFRelease(&changes);
	    }
	}
    }

    if (dns_changed || dnsinfo_changed) {
	const char    *str;

	str = get_changed_str(S_primary_dns, kSCEntNetDNS, old_primary_dns);
	if ((strcmp(str, "") == 0) && dnsinfo_changed) {
	    str = "*";	    // dnsinfo change w/no change to primary
	}
	CFStringAppendFormat(log_output, NULL, CFSTR(" DNS%s"), str);
    } else if (S_primary_dns != NULL) {
	CFStringAppend(log_output, CFSTR(" DNS"));
    }

    if (proxy_changed) {
	const char    *str;

	str = get_changed_str(S_primary_proxies, kSCEntNetProxies, old_primary_proxy);
	CFStringAppendFormat(log_output, NULL, CFSTR(" Proxy%s"), str);
    } else if (S_primary_proxies != NULL) {
	CFStringAppend(log_output, CFSTR(" Proxy"));
    }

#if	!TARGET_OS_IPHONE
    if (smb_changed) {
	const char    *str;

	str = get_changed_str(S_primary_smb, kSCEntNetSMB, old_primary_smb);
	CFStringAppendFormat(log_output, NULL, CFSTR(" SMB%s"), str);
    } else if (S_primary_smb != NULL) {
	CFStringAppend(log_output, CFSTR(" SMB"));
    }
#endif	// !TARGET_OS_IPHONE

    return log_output;
}

#pragma mark -
#pragma mark Network changed notification

static dispatch_queue_t
__network_change_queue()
{
    static dispatch_once_t	once;
    static dispatch_queue_t	q;

    dispatch_once(&once, ^{
	q = dispatch_queue_create("network change queue", NULL);
    });

    return q;
}

// Note: must run on __network_change_queue()
static void
post_network_change_when_ready()
{
    int		    status;

    if (S_network_change_needed == 0) {
	return;
    }

    if (!S_network_change_timeout &&
	(!S_dnsinfo_synced || !S_nwi_synced)) {
	// if we [still] need to wait for the DNS configuration
	// or network information changes to be ack'd

	if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
	    my_log(LOG_DEBUG,
		   "Defer \"" _SC_NOTIFY_NETWORK_CHANGE "\" (%s, %s)",
		   S_dnsinfo_synced ? "DNS" : "!DNS",
		   S_nwi_synced     ? "nwi" : "!nwi");
	}
	return;
    }

    // cancel any running timer
    if (S_network_change_timer != NULL) {
	dispatch_source_cancel(S_network_change_timer);
	dispatch_release(S_network_change_timer);
	S_network_change_timer = NULL;
	S_network_change_timeout = FALSE;
    }

    // set (and log?) the post time
    if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
	struct timeval  elapsed;
	struct timeval  end;

	(void) gettimeofday(&end, NULL);
	timersub(&end, &S_network_change_start, &elapsed);

#define	QUERY_TIME__FMT	"%ld.%6.6d"
#define	QUERY_TIME__DIV	1

	my_log(LOG_DEBUG,
	       "Post \"" _SC_NOTIFY_NETWORK_CHANGE "\" (%s: " QUERY_TIME__FMT ": 0x%x)",
	       S_network_change_timeout ? "timeout" : "delayed",
	       elapsed.tv_sec,
	       elapsed.tv_usec / QUERY_TIME__DIV,
	       S_network_change_needed);
    }

    if ((S_network_change_needed & NETWORK_CHANGE_NET) != 0) {
	status = notify_post(_SC_NOTIFY_NETWORK_CHANGE_NWI);
	if (status != NOTIFY_STATUS_OK) {
	    my_log(LOG_ERR,
		   "IPMonitor: notify_post(" _SC_NOTIFY_NETWORK_CHANGE_NWI ") failed: error=%d", status);
	}
    }

    if ((S_network_change_needed & NETWORK_CHANGE_DNS) != 0) {
	status = notify_post(_SC_NOTIFY_NETWORK_CHANGE_DNS);
	if (status != NOTIFY_STATUS_OK) {
	    my_log(LOG_ERR,
		   "IPMonitor: notify_post(" _SC_NOTIFY_NETWORK_CHANGE_DNS ") failed: error=%d", status);
	}
    }

    if ((S_network_change_needed & NETWORK_CHANGE_PROXY) != 0) {
	status = notify_post(_SC_NOTIFY_NETWORK_CHANGE_PROXY);
	if (status != NOTIFY_STATUS_OK) {
	    my_log(LOG_ERR,
		   "IPMonitor: notify_post(" _SC_NOTIFY_NETWORK_CHANGE_PROXY ") failed: error=%d", status);
	}
    }

    status = notify_post(_SC_NOTIFY_NETWORK_CHANGE);
    if (status != NOTIFY_STATUS_OK) {
	my_log(LOG_ERR,
	       "IPMonitor: notify_post(" _SC_NOTIFY_NETWORK_CHANGE ") failed: error=%d", status);
    }

    S_network_change_needed = 0;
    return;
}

#define TRAILING_EDGE_TIMEOUT_NSEC	5 * NSEC_PER_SEC    // 5s

// Note: must run on __network_change_queue()
static void
post_network_change(uint32_t change)
{
    if (S_network_change_needed == 0) {
	// set the start time
	(void) gettimeofday(&S_network_change_start, NULL);
    }

    // indicate that we need to post a change for ...
    S_network_change_needed |= change;

    // cancel any running timer
    if (S_network_change_timer != NULL) {
	dispatch_source_cancel(S_network_change_timer);
	dispatch_release(S_network_change_timer);
	S_network_change_timer = NULL;
	S_network_change_timeout = FALSE;
    }

    // if needed, start new timer
    if (!S_dnsinfo_synced || !S_nwi_synced) {
	S_network_change_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,
							0,
							0,
							__network_change_queue());
	dispatch_source_set_event_handler(S_network_change_timer, ^{
	    S_network_change_timeout = TRUE;
	    post_network_change_when_ready();
	});
	dispatch_source_set_timer(S_network_change_timer,
				  dispatch_time(DISPATCH_TIME_NOW,
						TRAILING_EDGE_TIMEOUT_NSEC),	// start
				  DISPATCH_TIME_FOREVER,			// interval
				  10 * NSEC_PER_MSEC);				// leeway
	dispatch_resume(S_network_change_timer);
    }

    post_network_change_when_ready();

    return;
}

#pragma mark -
#pragma mark Process network (SCDynamicStore) changes

static void
IPMonitorProcessChanges(SCDynamicStoreRef session, CFArrayRef changed_keys,
			CFArrayRef if_rank_changes)
{
    CFIndex		count 			= 0;
    uint32_t		changes			= 0;
    nwi_state_t		changes_state		= NULL;
    boolean_t		dns_changed		= FALSE;
    boolean_t		dnsinfo_changed		= FALSE;
    boolean_t		global_ipv4_changed	= FALSE;
    boolean_t		global_ipv6_changed	= FALSE;
    CFIndex		i;
    keyChangeList	keys;
    CFIndex		n;
    CFStringRef		network_change_msg	= NULL;
    int			n_services;
    int			n_service_order		= 0;
    nwi_state_t		old_nwi_state		= NULL;
    CFDictionaryRef	old_primary_dns		= NULL;
    CFDictionaryRef	old_primary_proxy	= NULL;
#if	!TARGET_OS_IPHONE
    CFDictionaryRef	old_primary_smb		= NULL;
#endif	// !TARGET_OS_IPHONE
    boolean_t		proxies_changed		= FALSE;
    boolean_t		reachability_changed	= FALSE;
    CFArrayRef		service_order;
    CFMutableArrayRef	service_changes		= NULL;
    CFDictionaryRef	services_info		= NULL;
#if	!TARGET_OS_IPHONE
    boolean_t		smb_changed		= FALSE;
#endif	// !TARGET_OS_IPHONE

    /* populate name/index cache */
    my_if_nameindex();

    if (changed_keys != NULL) {
	count = CFArrayGetCount(changed_keys);
	if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
	    my_log(LOG_DEBUG,
		   "IPMonitor: changed keys %@ (%ld)", changed_keys, count);
	}
    }
    if (if_rank_changes == NULL && count == 0) {
	return;
    }

    if (S_primary_dns != NULL) {
	old_primary_dns = service_dict_get(S_primary_dns, kSCEntNetDNS);
	if (old_primary_dns != NULL) {
	    old_primary_dns = CFDictionaryCreateCopy(NULL, old_primary_dns);
	}
    }

    if (S_primary_proxies != NULL) {
	old_primary_proxy
	    = service_dict_get(S_primary_proxies, kSCEntNetProxies);
	if (old_primary_proxy != NULL) {
	    old_primary_proxy = CFDictionaryCreateCopy(NULL, old_primary_proxy);
	}
    }

#if	!TARGET_OS_IPHONE
    if (S_primary_smb != NULL) {
	old_primary_smb = service_dict_get(S_primary_smb, kSCEntNetSMB);
	if (old_primary_smb != NULL) {
	    old_primary_smb = CFDictionaryCreateCopy(NULL, old_primary_smb);
	}
    }
#endif	// !TARGET_OS_IPHONE

    keyChangeListInit(&keys);
    service_changes = CFArrayCreateMutable(NULL, 0,
					   &kCFTypeArrayCallBacks);

    for (i = 0; i < count; i++) {
	CFStringRef	change = CFArrayGetValueAtIndex(changed_keys, i);
	if (CFEqual(change, S_setup_global_ipv4)) {
	    global_ipv4_changed = TRUE;
	    global_ipv6_changed = TRUE;
	}
	else if (CFEqual(change, S_multicast_resolvers)) {
	    dnsinfo_changed = TRUE;
	}
	else if (CFEqual(change, S_private_resolvers)) {
	    dnsinfo_changed = TRUE;
	}
#if	!TARGET_OS_IPHONE
	else if (CFEqual(change, CFSTR(_PATH_RESOLVER_DIR))) {
	    dnsinfo_changed = TRUE;
	}
#endif	/* !TARGET_OS_IPHONE */
	else if (CFStringHasPrefix(change, S_state_service_prefix)) {
	    CFStringRef serviceID;

	    serviceID = parse_component(change, S_state_service_prefix);
	    if (serviceID) {
		my_CFArrayAppendUniqueValue(service_changes, serviceID);
		CFRelease(serviceID);
	    }
	}
	else if (CFStringHasPrefix(change, S_setup_service_prefix)) {
	    int j;

	    CFStringRef serviceID = parse_component(change,
						    S_setup_service_prefix);
	    if (serviceID) {
		my_CFArrayAppendUniqueValue(service_changes, serviceID);
		CFRelease(serviceID);
	    }

	    for (j = 0; j < countof(transientInterfaceEntityNames); j++) {
		if (CFStringHasSuffix(change,
				      *transientInterfaceEntityNames[j])) {
		    reachability_changed = TRUE;
		    break;
		}
	    }

	    if (CFStringHasSuffix(change, kSCEntNetInterface)) {
		 reachability_changed = TRUE;
	    }
	}
    }

    /* determine which serviceIDs are impacted by the interface rank changes */
    if (if_rank_changes != NULL) {
	n = CFArrayGetCount(if_rank_changes);
	for (i = 0; i < n; i++) {
	    CFStringRef ifname = CFArrayGetValueAtIndex(if_rank_changes, i);

	    if (S_IPMonitor_debug & kDebugFlag1) {
		my_log(LOG_DEBUG, "Interface rank changed %@",
		       ifname);
	    }
	    append_serviceIDs_for_interface(service_changes, ifname);
	}
    }

    /* grab a snapshot of everything we need */
    services_info = services_info_copy(session, service_changes);
    service_order = service_order_get(services_info);
    if (service_order != NULL) {
	n_service_order = (int)CFArrayGetCount(service_order);
	if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
	    my_log(LOG_DEBUG,
		   "IPMonitor: service_order %@ ", service_order);
	}
    }

    n = CFArrayGetCount(service_changes);
    for (i = 0; i < n; i++) {
	uint32_t	changes;
	CFStringRef	serviceID;

	serviceID = CFArrayGetValueAtIndex(service_changes, i);
	changes = service_changed(services_info, serviceID);
	if ((changes & (1 << kEntityTypeServiceOptions)) != 0) {
	    /* if __Service__ (e.g. PrimaryRank) changed */
	    global_ipv4_changed = TRUE;
	    global_ipv6_changed = TRUE;
	}
	else {
	    if ((changes & (1 << kEntityTypeIPv4)) != 0) {
		global_ipv4_changed = TRUE;
		dnsinfo_changed = TRUE;
		proxies_changed = TRUE;
	    }
	    if ((changes & (1 << kEntityTypeIPv6)) != 0) {
		global_ipv6_changed = TRUE;
		dnsinfo_changed = TRUE;
		proxies_changed = TRUE;
	    }
	}
	if ((changes & (1 << kEntityTypeDNS)) != 0) {
	    if (S_primary_dns != NULL && CFEqual(S_primary_dns, serviceID)) {
		dns_changed = TRUE;
	    }
	    dnsinfo_changed = TRUE;
	}
	if ((changes & (1 << kEntityTypeProxies)) != 0) {
	    proxies_changed = TRUE;
	}
#if	!TARGET_OS_IPHONE
	if ((changes & (1 << kEntityTypeSMB)) != 0) {
	    if (S_primary_smb != NULL && CFEqual(S_primary_smb, serviceID)) {
		smb_changed = TRUE;
	    }
	}
#endif
	if ((changes & (1 << kEntityTypeTransientStatus)) != 0
	    && (service_dict_get(serviceID, kSCEntNetIPv4) != NULL
		|| service_dict_get(serviceID, kSCEntNetIPv6) != NULL)) {
	    dnsinfo_changed = TRUE;
	}
    }

    /* ensure S_nwi_state can hold as many services as we have currently */
    n_services = (int)CFDictionaryGetCount(S_service_state_dict);
    old_nwi_state = nwi_state_copy_priv(S_nwi_state);
    S_nwi_state = nwi_state_new(S_nwi_state, n_services);

    if (global_ipv4_changed) {
	if (S_ipv4_results != NULL) {
	    ElectionResultsRelease(S_ipv4_results);
	}
	S_ipv4_results
	    = ElectionResultsCopy(AF_INET, service_order, n_service_order);
	if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
	    ElectionResultsLog(LOG_DEBUG, S_ipv4_results, "IPv4");
	}
    }
    if (global_ipv6_changed) {
	if (S_ipv6_results != NULL) {
	    ElectionResultsRelease(S_ipv6_results);
	}
	S_ipv6_results
	    = ElectionResultsCopy(AF_INET6, service_order, n_service_order);
	if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
	    ElectionResultsLog(LOG_DEBUG, S_ipv6_results, "IPv6");
	}
    }
    if (global_ipv4_changed || global_ipv6_changed || dnsinfo_changed) {
	CFStringRef		new_primary;
	RouteListUnion		new_routelist;

	/* IPv4 */
	if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
	    my_log(LOG_DEBUG,
		   "IPMonitor: electing IPv4 primary");
	}
	new_routelist.ptr = NULL;
	new_primary = ElectionResultsCopyPrimary(S_ipv4_results,
						 S_ipv6_results,
						 S_nwi_state, AF_INET,
						 &new_routelist.common,
						 services_info);
	(void)set_new_primary(&S_primary_ipv4, new_primary, "IPv4");
	update_ipv4(S_primary_ipv4, new_routelist.v4, &keys);
	my_CFRelease(&new_primary);

	/* IPv6 */
	if ((S_IPMonitor_debug & kDebugFlag1) != 0) {
	    my_log(LOG_DEBUG,
		   "IPMonitor: electing IPv6 primary");
	}
	new_routelist.ptr = NULL;
	new_primary = ElectionResultsCopyPrimary(S_ipv6_results,
						 S_ipv4_results,
						 S_nwi_state, AF_INET6,
						 &new_routelist.common,
						 services_info);
	(void)set_new_primary(&S_primary_ipv6, new_primary, "IPv6");
	update_ipv6(S_primary_ipv6, new_routelist.v6, &keys);
	my_CFRelease(&new_primary);
    }

    if (global_ipv4_changed || global_ipv6_changed) {
	CFStringRef	new_primary_dns	    = NULL;
	CFStringRef	new_primary_proxies = NULL;
#if	!TARGET_OS_IPHONE
	CFStringRef	new_primary_smb	    = NULL;
#endif	/* !TARGET_OS_IPHONE */

	if (S_primary_ipv4 != NULL && S_primary_ipv6 != NULL) {
	    /* decide between IPv4 and IPv6 */
	    if (rank_service_entity(S_ipv4_service_rank_dict,
				    S_primary_ipv4, kSCEntNetDNS)
		<= rank_service_entity(S_ipv6_service_rank_dict,
				       S_primary_ipv6, kSCEntNetDNS)) {
		new_primary_dns = S_primary_ipv4;
	    }
	    else {
		new_primary_dns = S_primary_ipv6;
	    }
	    if (rank_service_entity(S_ipv4_service_rank_dict,
				    S_primary_ipv4, kSCEntNetProxies)
		<= rank_service_entity(S_ipv6_service_rank_dict,
				       S_primary_ipv6, kSCEntNetProxies)) {
		new_primary_proxies = S_primary_ipv4;
	    }
	    else {
		new_primary_proxies = S_primary_ipv6;
	    }
#if	!TARGET_OS_IPHONE
	    if (rank_service_entity(S_ipv4_service_rank_dict,
				    S_primary_ipv4, kSCEntNetSMB)
		<= rank_service_entity(S_ipv6_service_rank_dict,
				       S_primary_ipv6, kSCEntNetSMB)) {
		new_primary_smb = S_primary_ipv4;
	    }
	    else {
		new_primary_smb = S_primary_ipv6;
	    }
#endif	/* !TARGET_OS_IPHONE */

	}
	else if (S_primary_ipv6 != NULL) {
	    new_primary_dns     = S_primary_ipv6;
	    new_primary_proxies = S_primary_ipv6;
#if	!TARGET_OS_IPHONE
	    new_primary_smb     = S_primary_ipv6;
#endif	/* !TARGET_OS_IPHONE */
	}
	else if (S_primary_ipv4 != NULL) {
	    new_primary_dns     = S_primary_ipv4;
	    new_primary_proxies = S_primary_ipv4;
#if	!TARGET_OS_IPHONE
	    new_primary_smb     = S_primary_ipv4;
#endif	/* !TARGET_OS_IPHONE */
	}

	if (set_new_primary(&S_primary_dns, new_primary_dns, "DNS")) {
	    dns_changed = TRUE;
	    dnsinfo_changed = TRUE;
	}
	if (set_new_primary(&S_primary_proxies, new_primary_proxies,
			    "Proxies")) {
	    proxies_changed = TRUE;
	}
#if	!TARGET_OS_IPHONE
	if (set_new_primary(&S_primary_smb, new_primary_smb, "SMB")) {
	    smb_changed = TRUE;
	}
#endif	/* !TARGET_OS_IPHONE */
    }

    if (!proxies_changed && dnsinfo_changed
	&& ((G_supplemental_proxies_follow_dns != NULL)
	    && CFBooleanGetValue(G_supplemental_proxies_follow_dns))) {
	proxies_changed = TRUE;
    }

    changes_state = nwi_state_diff(old_nwi_state, S_nwi_state);

    if (global_ipv4_changed || global_ipv6_changed
	|| dnsinfo_changed || reachability_changed) {
	if (S_nwi_state != NULL) {
	    S_nwi_state->generation_count = mach_absolute_time();
	    if (global_ipv4_changed || global_ipv6_changed
		|| reachability_changed) {
		SCNetworkReachabilityFlags reach_flags_v4 = 0;
		SCNetworkReachabilityFlags reach_flags_v6 = 0;

		GetReachabilityFlagsFromTransientServices(services_info,
							  &reach_flags_v4,
							  &reach_flags_v6);

		_nwi_state_set_reachability_flags(S_nwi_state, reach_flags_v4,
						  reach_flags_v6);
	    }

	    /* Update the per-interface generation count */
	    _nwi_state_update_interface_generations(old_nwi_state, S_nwi_state,
						    changes_state);
	}

	if (update_nwi(S_nwi_state)) {
	    changes |= NETWORK_CHANGE_NET;

	    /*
	     * the DNS configuration includes per-resolver configuration
	     * reachability flags that are based on the nwi state.  Let's
	     * make sure that we check for changes
	     */
	    dnsinfo_changed = TRUE;
	}
    }
    if (dns_changed) {
	if (update_dns(services_info, S_primary_dns, &keys)) {
	    changes |= NETWORK_CHANGE_DNS;
	    dnsinfo_changed = TRUE;
	} else {
	    dns_changed = FALSE;
	}
    }
    if (dnsinfo_changed) {
	if (update_dnsinfo(services_info, S_primary_dns,
			   &keys, service_order)) {
	    changes |= NETWORK_CHANGE_DNS;
	} else {
	    dnsinfo_changed = FALSE;
	}
    }
    if (proxies_changed) {
	// if proxy change OR supplemental Proxies follow supplemental DNS
	if (update_proxies(services_info, S_primary_proxies,
			   &keys, service_order)) {
	    changes |= NETWORK_CHANGE_PROXY;
	} else {
	    proxies_changed = FALSE;
	}
    }
#if	!TARGET_OS_IPHONE
    if (smb_changed) {
	if (update_smb(services_info, S_primary_smb, &keys)) {
	    changes |= NETWORK_CHANGE_SMB;
	} else {
	    smb_changed = FALSE;
	}
    }
#endif	/* !TARGET_OS_IPHONE */
    my_CFRelease(&service_changes);
    my_CFRelease(&services_info);

    if (changes != 0) {
	network_change_msg =
	    generate_log_changes(changes_state,
				 dns_changed,
				 dnsinfo_changed,
				 old_primary_dns,
				 proxies_changed,
				 old_primary_proxy,
#if	!TARGET_OS_IPHONE
				 smb_changed,
				 old_primary_smb
#else	// !TARGET_OS_IPHONE
				 FALSE,		// smb_changed
				 NULL		// old_primary_smb
#endif	// !TARGET_OS_IPHONE
				 );
    }

    keyChangeListApplyToStore(&keys, session);
    my_CFRelease(&old_primary_dns);
    my_CFRelease(&old_primary_proxy);
#if	!TARGET_OS_IPHONE
    my_CFRelease(&old_primary_smb);
#endif	// !TARGET_OS_IPHONE

    if (changes != 0) {
	dispatch_async(__network_change_queue(), ^{
	    post_network_change(changes);
	});
    }

    if ((network_change_msg != NULL)
	&& (CFStringGetLength(network_change_msg) != 0)) {
	my_log(LOG_NOTICE, "network changed:%@", network_change_msg);
    } else if (keyChangeListActive(&keys)) {
	my_log(LOG_NOTICE, "network changed.");
    } else {
	my_log(LOG_DEBUG, "network event w/no changes");
    }

    my_CFRelease(&network_change_msg);

    if (changes_state != NULL) {
	nwi_state_release(changes_state);
    }
    if (old_nwi_state != NULL) {
	nwi_state_release(old_nwi_state);
    }
    keyChangeListFree(&keys);

    /* release the name/index cache */
    my_if_freenameindex();

    return;
}

static void
IPMonitorNotify(SCDynamicStoreRef session, CFArrayRef changed_keys,
		void * not_used)
{
    IPMonitorProcessChanges(session, changed_keys, NULL);
    return;
}

static void
watch_proxies()
{
#if	!TARGET_OS_IPHONE
    const _scprefs_observer_type type = scprefs_observer_type_mcx;
#else
    const _scprefs_observer_type type = scprefs_observer_type_global;
#endif
    static dispatch_queue_t proxy_cb_queue;

    proxy_cb_queue = dispatch_queue_create("com.apple.SystemConfiguration.IPMonitor.proxy", NULL);
    _scprefs_observer_watch(type,
			     "com.apple.SystemConfiguration.plist",
			     proxy_cb_queue,
			     ^{
				 SCDynamicStoreNotifyValue(NULL, S_state_global_proxies);
				 notify_post(_SC_NOTIFY_NETWORK_CHANGE_PROXY);
				 my_log(LOG_DEBUG, "IPMonitor: Notifying:\n%@",
					S_state_global_proxies);
			     });
    return;
}

#include "IPMonitorControlPrefs.h"

__private_extern__ SCLoggerRef
my_log_get_logger()
{
    return (S_IPMonitor_logger);
}

static void
prefs_changed(__unused SCPreferencesRef prefs)
{
    if (S_bundle_logging_verbose || IPMonitorControlPrefsIsVerbose()) {
	S_IPMonitor_debug = kDebugFlagDefault;
	S_IPMonitor_verbose = TRUE;
	SCLoggerSetFlags(S_IPMonitor_logger, kSCLoggerFlagsFile | kSCLoggerFlagsDefault);
	my_log(LOG_DEBUG, "IPMonitor: Setting logging verbose mode on.");
    } else {
	my_log(LOG_DEBUG, "IPMonitor: Setting logging verbose mode off.");
	S_IPMonitor_debug = 0;
	S_IPMonitor_verbose = FALSE;
	SCLoggerSetFlags(S_IPMonitor_logger, kSCLoggerFlagsDefault);
    }
    return;
}

#define LOGGER_ID CFSTR("com.apple.networking.IPMonitor")
static void
my_log_init()
{
    if (S_IPMonitor_logger != NULL) {
	return;
    }
    S_IPMonitor_logger = SCLoggerCreate(LOGGER_ID);
    return;

}


#if	!TARGET_IPHONE_SIMULATOR
static int
flush_routes(int s)
{
    char *		buf = NULL;
    int			i;
    char *		lim;
#define N_MIB		6
    int 		mib[N_MIB];
    size_t 		needed;
    char *		next;
    struct rt_msghdr *	rtm;
    struct sockaddr_in *sin;

    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_INET;
    mib[4] = NET_RT_FLAGS;
    mib[5] = RTF_STATIC | RTF_DYNAMIC;
    for (i = 0; i < 3; i++) {
	if (sysctl(mib, N_MIB, NULL, &needed, NULL, 0) < 0) {
	    break;
	}
	if ((buf = malloc(needed)) == NULL) {
	    break;
	}
	if (sysctl(mib, N_MIB, buf, &needed, NULL, 0) >= 0) {
	    break;
	}
	free(buf);
	buf = NULL;
    }
    if (buf == NULL) {
	return (-1);
    }
    lim = buf + needed;
    for (next = buf; next < lim; next += rtm->rtm_msglen) {
	uint32_t	addr;

	/* ALIGN: assume kernel provides necessary alignment */
	rtm = (struct rt_msghdr *)(void *)next;
	sin = (struct sockaddr_in *)(rtm + 1);

	addr = ntohl(sin->sin_addr.s_addr);
	if (IN_LOOPBACK(addr)) {
	    my_log(LOG_DEBUG,
		   "IPMonitor: flush_routes: ignoring loopback route");
	    continue;
	}
	if (IN_LOCAL_GROUP(addr)) {
	    my_log(LOG_DEBUG,
		   "IPMonitor: flush_routes: ignoring multicast route");
	    continue;
	}
	rtm->rtm_type = RTM_DELETE;
	rtm->rtm_seq = ++rtm_seq;
	if (write(s, rtm, rtm->rtm_msglen) < 0) {
	    my_log(LOG_DEBUG,
		   "IPMonitor: flush_routes: removing route for "
		   IP_FORMAT " failed, %s",
		   IP_LIST(&sin->sin_addr),
		   strerror(errno));
	}
	else {
	    my_log(LOG_DEBUG,
		   "IPMonitor: flush_routes: removed route for " IP_FORMAT,
		   IP_LIST(&sin->sin_addr));
	}
    }
    free(buf);
    return (0);
}

static void
flush_inet_routes(void)
{
    int	s;

    s = open_routing_socket();
    if (s != -1) {
	flush_routes(s);
	close(s);
    }
}

#else 	/* !TARGET_IPHONE_SIMULATOR */

static void
flush_inet_routes(void)
{
}

#endif	/* !TARGET_IPHONE_SIMULATOR */



static void
ip_plugin_init()
{
    CFMutableArrayRef	keys = NULL;
    CFStringRef		pattern;
    CFMutableArrayRef	patterns = NULL;
    CFRunLoopSourceRef	rls = NULL;

    if (S_is_network_boot() != 0) {
	S_netboot = TRUE;
    }
    else {
	/* flush routes */
	flush_inet_routes();
    }

    if (S_is_scoped_routing_enabled() != 0) {
	S_scopedroute = TRUE;
    }

    if (S_is_scoped_v6_routing_enabled() != 0) {
	S_scopedroute_v6 = TRUE;
    }

    S_session = SCDynamicStoreCreate(NULL, CFSTR("IPMonitor"),
				   IPMonitorNotify, NULL);
    if (S_session == NULL) {
	my_log(LOG_ERR,
	       "IPMonitor ip_plugin_init SCDynamicStoreCreate failed: %s",
	       SCErrorString(SCError()));
	return;
    }
    S_state_global_ipv4
	= SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
						     kSCDynamicStoreDomainState,
						     kSCEntNetIPv4);
    S_state_global_ipv6
	= SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
						     kSCDynamicStoreDomainState,
						     kSCEntNetIPv6);
    S_state_global_dns
	= SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
						     kSCDynamicStoreDomainState,
						     kSCEntNetDNS);
    S_state_global_proxies
	= SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
						     kSCDynamicStoreDomainState,
						     kSCEntNetProxies);
#if	!TARGET_OS_IPHONE
    S_state_global_smb
	= SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
						     kSCDynamicStoreDomainState,
						     kSCEntNetSMB);
#endif	/* !TARGET_OS_IPHONE */
    S_setup_global_ipv4
	= SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
						     kSCDynamicStoreDomainSetup,
						     kSCEntNetIPv4);
    S_state_service_prefix
	= SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
						      kSCDynamicStoreDomainState,
						      CFSTR(""),
						      NULL);
    S_setup_service_prefix
	= SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
						      kSCDynamicStoreDomainSetup,
						      CFSTR(""),
						      NULL);
    S_service_state_dict
	= CFDictionaryCreateMutable(NULL, 0,
				    &kCFTypeDictionaryKeyCallBacks,
				    &kCFTypeDictionaryValueCallBacks);

    S_ipv4_service_rank_dict
	= CFDictionaryCreateMutable(NULL, 0,
				    &kCFTypeDictionaryKeyCallBacks,
				    &kCFTypeDictionaryValueCallBacks);

    S_ipv6_service_rank_dict
	= CFDictionaryCreateMutable(NULL, 0,
				    &kCFTypeDictionaryKeyCallBacks,
				    &kCFTypeDictionaryValueCallBacks);

    keys = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    patterns = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

    /* register for State: and Setup: per-service notifications */
    add_service_keys(kSCCompAnyRegex, keys, patterns);

    pattern = setup_service_key(kSCCompAnyRegex, kSCEntNetPPP);
    CFArrayAppendValue(patterns, pattern);
    CFRelease(pattern);

    pattern = setup_service_key(kSCCompAnyRegex, kSCEntNetVPN);
    CFArrayAppendValue(patterns, pattern);
    CFRelease(pattern);

    pattern = setup_service_key(kSCCompAnyRegex, kSCEntNetInterface);
    CFArrayAppendValue(patterns, pattern);
    CFRelease(pattern);

    /* register for State: per-service PPP/VPN/IPSec status notifications */
    add_transient_status_keys(kSCCompAnyRegex, patterns);

    /* add notifier for ServiceOrder/PPPOverridePrimary changes for IPv4 */
    CFArrayAppendValue(keys, S_setup_global_ipv4);

    /* add notifier for multicast DNS configuration (Bonjour/.local) */
    S_multicast_resolvers = SCDynamicStoreKeyCreate(NULL, CFSTR("%@/%@/%@"),
						    kSCDynamicStoreDomainState,
						    kSCCompNetwork,
						    CFSTR(kDNSServiceCompMulticastDNS));
    CFArrayAppendValue(keys, S_multicast_resolvers);

    /* add notifier for private DNS configuration (Back to My Mac) */
    S_private_resolvers = SCDynamicStoreKeyCreate(NULL, CFSTR("%@/%@/%@"),
						  kSCDynamicStoreDomainState,
						  kSCCompNetwork,
						  CFSTR(kDNSServiceCompPrivateDNS));
    CFArrayAppendValue(keys, S_private_resolvers);

    if (!SCDynamicStoreSetNotificationKeys(S_session, keys, patterns)) {
	my_log(LOG_ERR,
	       "IPMonitor ip_plugin_init "
	       "SCDynamicStoreSetNotificationKeys failed: %s",
	      SCErrorString(SCError()));
	goto done;
    }

    rls = SCDynamicStoreCreateRunLoopSource(NULL, S_session, 0);
    if (rls == NULL) {
	my_log(LOG_ERR,
	       "IPMonitor ip_plugin_init "
	       "SCDynamicStoreCreateRunLoopSource failed: %s",
	       SCErrorString(SCError()));
	goto done;
    }

    CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
    CFRelease(rls);

    /* initialize dns configuration */
    (void)dns_configuration_set(NULL, NULL, NULL, NULL, NULL);
#if	!TARGET_OS_IPHONE
    empty_dns();
#endif	/* !TARGET_OS_IPHONE */
    (void)SCDynamicStoreRemoveValue(S_session, S_state_global_dns);

#if	!TARGET_OS_IPHONE
    /* initialize SMB configuration */
    (void)SCDynamicStoreRemoveValue(S_session, S_state_global_smb);
#endif	/* !TARGET_OS_IPHONE */

    watch_proxies();

  done:
    my_CFRelease(&keys);
    my_CFRelease(&patterns);
    return;
}

__private_extern__
void
prime_IPMonitor()
{
    /* initialize multicast route */
    update_ipv4(NULL, NULL, NULL);
    return;
}

static boolean_t
S_get_plist_boolean(CFDictionaryRef plist, CFStringRef key,
		    boolean_t def)
{
    CFBooleanRef	b;
    boolean_t		ret = def;

    b = isA_CFBoolean(CFDictionaryGetValue(plist, key));
    if (b != NULL) {
	ret = CFBooleanGetValue(b);
    }
    return (ret);
}

#if	!TARGET_IPHONE_SIMULATOR
#include "IPMonitorControlServer.h"

static void
InterfaceRankChanged(void * info)
{
    CFDictionaryRef 	assertions = NULL;
    CFArrayRef		changes;

    changes = IPMonitorControlServerCopyInterfaceRankInformation(&assertions);
    if (S_if_rank_dict != NULL) {
	CFRelease(S_if_rank_dict);
    }
    S_if_rank_dict = assertions;
    if (changes != NULL) {
	IPMonitorProcessChanges(S_session, NULL, changes);
	CFRelease(changes);
    }
    return;
}

static void
StartIPMonitorControlServer(void)
{
    CFRunLoopSourceContext 	context;
    CFRunLoopSourceRef	rls;

    bzero(&context, sizeof(context));
    context.perform = InterfaceRankChanged;
    rls = CFRunLoopSourceCreate(NULL, 0, &context);
    if (IPMonitorControlServerStart(CFRunLoopGetCurrent(), rls,
				    &S_bundle_logging_verbose) == FALSE) {
	my_log(LOG_ERR, "IPMonitorControlServerStart failed");
    }
    else {
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls,
			   kCFRunLoopDefaultMode);
    }
    CFRelease(rls);
    return;
}

#endif	/* !TARGET_IPHONE_SIMULATOR */

__private_extern__
void
load_IPMonitor(CFBundleRef bundle, Boolean bundleVerbose)
{
    CFDictionaryRef	info_dict;

    info_dict = CFBundleGetInfoDictionary(bundle);

    if (info_dict != NULL) {
	S_append_state
	    = S_get_plist_boolean(info_dict,
				  CFSTR("AppendStateArrayToSetupArray"),
				  FALSE);
    }
    if (bundleVerbose) {
	S_IPMonitor_debug = kDebugFlagDefault;
	S_bundle_logging_verbose = bundleVerbose;
	S_IPMonitor_verbose = TRUE;
    }

    my_log_init();

    /* register to receive changes to verbose and read the initial setting  */
    IPMonitorControlPrefsInit(CFRunLoopGetCurrent(), prefs_changed);
    prefs_changed(NULL);


    load_DNSConfiguration(bundle,			// bundle
			  S_IPMonitor_logger,		// SCLogger
			  &S_bundle_logging_verbose,	// bundleVerbose
			  ^(Boolean inSync) {		// syncHandler
			      dispatch_async(__network_change_queue(), ^{
				  S_dnsinfo_synced = inSync;

				  if (inSync &&
				      ((S_network_change_needed & NETWORK_CHANGE_DNS) == 0)) {
				      // all of the mDNSResponder ack's should result
				      // in a [new] network change being posted
				      post_network_change(NETWORK_CHANGE_DNS);
				  } else {
				      post_network_change_when_ready();
				  }
			      });
			  });

    load_NetworkInformation(bundle,			// bundle
			    S_IPMonitor_logger,		// SCLogger
			    &S_bundle_logging_verbose,	// bundleVerbose
			    ^(Boolean inSync) {		// syncHandler
				dispatch_async(__network_change_queue(), ^{
				    S_nwi_synced = inSync;
				    post_network_change_when_ready();
				});
			    });
#if	!TARGET_IPHONE_SIMULATOR
    StartIPMonitorControlServer();
#endif	/* !TARGET_OS_IPHONE */

    dns_configuration_init(bundle);

    proxy_configuration_init(bundle);

    ip_plugin_init();

#if	!TARGET_OS_IPHONE
    if (S_session != NULL) {
	dns_configuration_monitor(S_session, IPMonitorNotify);
    }
#endif	/* !TARGET_OS_IPHONE */

#if	!TARGET_IPHONE_SIMULATOR
    load_hostname((S_IPMonitor_debug & kDebugFlag1) != 0);
#endif	/* !TARGET_IPHONE_SIMULATOR */

#if	!TARGET_OS_IPHONE
    load_smb_configuration((S_IPMonitor_debug & kDebugFlag1) != 0);
#endif	/* !TARGET_OS_IPHONE */

    return;
}


#pragma mark -
#pragma mark Standalone test code


#ifdef TEST_IPMONITOR

#include "dns-configuration.c"

#if	!TARGET_IPHONE_SIMULATOR
#include "set-hostname.c"
#endif	/* !TARGET_IPHONE_SIMULATOR */

int
main(int argc, char **argv)
{
    _sc_log     = FALSE;

    S_IPMonitor_debug = kDebugFlag1;
    if (argc > 1) {
	S_IPMonitor_debug = strtoul(argv[1], NULL, 0);
    }

    load_IPMonitor(CFBundleGetMainBundle(), FALSE);
    prime_IPMonitor();
    S_IPMonitor_debug = kDebugFlag1;
    CFRunLoopRun();
    /* not reached */
    exit(0);
    return 0;
}
#endif /* TEST_IPMONITOR */

#ifdef TEST_ROUTELIST
#include "dns-configuration.c"
#include "set-hostname.c"

struct route {
    const char *	dest;
    int			prefix_length;
    const char *	gateway;
    const char *	ifname;
};

#endif

#ifdef TEST_IPV4_ROUTELIST

typedef struct {
    const char *	addr;
    int			prefix_length;
    const char *	dest;
    const char *	router;
    const char *	ifname;
    Rank		rank;
    const CFStringRef *	primary_rank;
    struct route *	additional_routes;
    int			additional_routes_count;
    struct route *	excluded_routes;
    int			excluded_routes_count;
} IPv4ServiceContents;

typedef const IPv4ServiceContents * IPv4ServiceContentsRef;

struct route loop_routelist[] = {
    { "1.1.1.1", 32, "1.1.1.2", NULL },
    { "1.1.1.2", 32, "1.1.1.3", NULL },
    { "1.1.1.3", 32, "1.1.1.4", NULL },
    { "1.1.1.4", 32, "1.1.1.5", NULL },
    { "1.1.1.5", 32, "1.1.1.6", NULL },
    { "1.1.1.6", 32, "1.1.1.7", NULL },
    { "1.1.1.7", 32, "1.1.1.8", NULL },
    { "1.1.1.8", 32, "1.1.1.9", NULL },
    { "1.1.1.9", 32, "1.1.1.10", NULL },
    { "1.1.1.10", 32, "1.1.1.11", NULL },
    { "1.1.1.11", 32, "1.1.1.1", NULL },
};

struct route vpn_routelist[] = {
    { "10.1.3.0", 24, "17.153.46.24", NULL },
    { "10.1.4.0", 24, "17.153.46.24", NULL },
    { "10.1.5.0", 24, "17.153.46.24", NULL },
    { "10.1.6.0", 24, "17.153.46.24", NULL },
    { "10.1.7.0", 24, "17.153.46.24", NULL },
    { "10.16.0.0", 12, "17.153.46.24", NULL },
    { "10.45.0.0", 16, "17.153.46.24", NULL },
    { "10.53.0.0", 16, "17.153.46.24", NULL },
    { "10.70.0.0", 15, "17.153.46.24", NULL },
    { "10.74.0.0", 15, "17.153.46.24", NULL },
    { "10.90.0.0", 15, "17.153.46.24", NULL },
    { "10.91.0.0", 16, "17.153.46.24", NULL },
    { "10.100.0.0", 16, "17.153.46.24", NULL },
    { "10.113.0.0", 16, "17.153.46.24", NULL },
    { "10.128.0.0", 9, "17.153.46.24", NULL },
    { "17.0.0.0", 9, "17.153.46.24", NULL },
    { "17.34.0.0", 16, "17.153.46.24", NULL },
    { "17.112.156.53", 32, "17.153.46.24", NULL },
    { "17.128.0.0", 10, "17.153.46.24", NULL },
    { "17.149.0.121", 32, "17.153.46.24", NULL },
    { "17.149.7.200", 32, "17.153.46.24", NULL },
    { "17.153.46.24", 32, "17.153.46.24", NULL },
    { "17.192.0.0", 12, "17.153.46.24", NULL },
    { "17.208.0.0", 15, "17.153.46.24", NULL },
    { "17.211.0.0", 16, "17.153.46.24", NULL },
    { "17.212.0.0", 14, "17.153.46.24", NULL },
    { "17.216.0.0", 13, "17.153.46.24", NULL },
    { "17.224.0.0", 12, "17.153.46.24", NULL },
    { "17.240.0.0", 16, "17.153.46.24", NULL },
    { "17.241.0.0", 16, "17.153.46.24", NULL },
    { "17.248.0.0", 14, "17.153.46.24", NULL },
    { "17.251.104.200", 32, "17.153.46.24", NULL },
    { "17.252.0.0", 16, "17.153.46.24", NULL },
    { "17.253.0.0", 16, "17.153.46.24", NULL },
    { "17.254.0.0", 16, "17.153.46.24", NULL },
    { "17.255.0.0", 16, "17.153.46.24", NULL },
    { "151.193.141.0", 27, "17.153.46.24", NULL },
    { "172.16.2.0", 24, "17.153.46.24", NULL },
    { "192.35.50.0", 24, "17.153.46.24", NULL },
    { "204.179.20.0", 24, "17.153.46.24", NULL },
    { "206.112.116.0", 24, "17.153.46.24", NULL },
};

struct route vpn_routelist_ext[] = {
    { "17.151.63.82", 32, "10.0.0.1", "en0" },
    { "17.151.63.81", 32, "17.151.63.81", "en0" },
    { "17.151.63.80", 32, NULL, NULL },
    { "17.1.0.0", 16, NULL, NULL },
    { "17.2.0.0", 24, NULL, NULL },
    { "10.0.0.0", 24, NULL, NULL },
};

/*
 *  addr	prefix	dest	router	    ifname	pri  rank additional-routes+count excluded-routes+count
 */
const IPv4ServiceContents en0_10 = {
    "10.0.0.10", 24, NULL,	"10.0.0.1", "en0", 10, NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en0_15 = {
    "10.0.0.19", 24, NULL,	"10.0.0.1", "en0", 15, NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en0_30 = {
    "10.0.0.11", 24, NULL,	"10.0.0.1", "en0", 30, NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en0_40 = {
    "10.0.0.12", 24, NULL,	"10.0.0.1", "en0", 40, NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en0_50 = {
    "10.0.0.13", 24, NULL,	"10.0.0.1", "en0", 50, NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en0_110 = {
    "192.168.2.10", 24, NULL, "192.168.2.1", "en0",	110, NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en0_1 = {
    "17.202.40.191", 22, NULL, "17.202.20.1", "en0", 1,  NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en1_20 = {
    "10.0.0.20", 24, NULL, "10.0.0.1", "en1", 20, NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en1_2 = {
    "17.202.42.24", 22, NULL, "17.202.20.1", "en1", 2,  NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en1_125 = {
    "192.168.2.20", 24, NULL, "192.168.2.1", "en1", 125, NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents fw0_25 = {
    "192.168.2.30", 24, NULL, "192.168.2.1", "fw0", 25, NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents fw0_21 = {
    "192.168.3.30", 24, NULL, "192.168.3.1", "fw0", 21, NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents ppp0_0_1 = {
    "17.219.156.22", -1, "17.219.156.1", "17.219.156.1", "ppp0", 0,  NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents utun0 = {
    "17.153.46.24", -1, "17.153.46.24", "17.153.46.24", "utun0", 20,  NULL, vpn_routelist, countof(vpn_routelist), vpn_routelist_ext, countof(vpn_routelist_ext)
};

const IPv4ServiceContents en0_test6 = {
    "17.202.42.113", 22, NULL, "17.202.40.1", "en0", 2,  NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en1_test6 = {
    "17.202.42.111", 22, NULL, "17.202.40.1", "en1", 3,  NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en2_test6 = {
    "17.255.98.164", 20, NULL, "17.255.96.1", "en2", 1,  NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en0_test7 = {
    "17.202.42.113", 22, NULL, "17.202.40.1", "en0", 3,  NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en1_test7 = {
    "17.202.42.111", 22, NULL, "17.202.40.1", "en1", 2,  NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en2_test7 = {
    "17.255.98.164", 20, NULL, "17.255.96.1", "en2", 1,  NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents fw0_test6_and_7 = {
    "169.254.11.33", 16, NULL,	NULL,	"fw0", 0x0ffffff, NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en0_10_last = {
    "10.0.0.10", 24, NULL,	"10.0.0.1", "en0", 10, &kSCValNetServicePrimaryRankLast, NULL, 0, NULL, 0
};

const IPv4ServiceContents en0_10_never = {
    "10.0.0.10", 24, NULL,	"10.0.0.1", "en0", 10, &kSCValNetServicePrimaryRankNever, NULL, 0, NULL, 0
};

const IPv4ServiceContents en1_20_first = {
    "10.0.0.20", 24, NULL,	"10.0.0.1", "en1", 20, &kSCValNetServicePrimaryRankFirst, NULL, 0, NULL, 0
};

const IPv4ServiceContents en1_20_never = {
    "10.0.0.20", 24, NULL,	"10.0.0.1", "en1", 20, &kSCValNetServicePrimaryRankNever, NULL, 0, NULL, 0
};

const IPv4ServiceContents en1_20_other_never = {
    "192.168.2.50", 24, NULL, "192.168.2.1", "en1", 20, &kSCValNetServicePrimaryRankNever, NULL, 0, NULL, 0
};

const IPv4ServiceContents en0_linklocal = {
    "169.254.22.44", 16, NULL, NULL,	"en0",	0xfffff, NULL, NULL, 0, NULL, 0
};

const IPv4ServiceContents en0_route_loop = {
    "192.168.130.16", 24, NULL, "192.168.130.1", "en0", 2, NULL, loop_routelist, countof(loop_routelist), NULL, 0
};

typedef struct {
    const char *		name;
    IPv4ServiceContentsRef	test[];
} IPv4RouteTest, * IPv4RouteTestRef;

static IPv4RouteTest test1 = {
    "test1",
    {
	&en0_40,
	&en0_15,
	&fw0_25,
	&en0_30,
	&en1_20,
	&en0_50,
	&en0_10,
	NULL
    }
};

static IPv4RouteTest test2 = {
    "test2",
    {
	&en0_40,
	&fw0_25,
	&en0_30,
	&en1_20,
	&en0_50,
	&en0_10,
	NULL
    }
};

static IPv4RouteTest test3 = {
    "test3",
    {
	&en0_40,
	&en1_20,
	&en0_50,
	&en0_10,
	&en0_110,
	&en1_125,
	&fw0_25,
	&fw0_21,
	&en0_40,
	&en0_30,
	NULL
    }
};

static IPv4RouteTest test4 = {
    "test4",
    {
	&en0_1,
	&en0_40,
	&en0_30,
	&en1_20,
	&en1_2,
	NULL
    }
};

static IPv4RouteTest test5 = {
    "test5",
    {
	&ppp0_0_1,
	&en0_1,
	&en0_40,
	&en0_30,
	&en1_20,
	&en1_2,
	NULL
    }
};

static IPv4RouteTest test6 = {
    "test6",
    {
	&en0_test6,
	&en1_test6,
	&en2_test6,
	&fw0_test6_and_7,
	NULL
    }
};

static IPv4RouteTest test7 = {
    "test7",
    {
	&en0_test7,
	&en1_test7,
	&en2_test7,
	&fw0_test6_and_7,
	NULL
    }
};

static IPv4RouteTest test8 = {
    "test8",
    {
	&en0_10,
	&en1_20,
	NULL
    }
};

static IPv4RouteTest test9 = {
    "test9",
    {
	&en0_10,
	&en1_20_first,
	&fw0_25,
	NULL
    }
};

static IPv4RouteTest test10 = {
    "test10",
    {
	&en0_10_last,
	&en1_20,
	&fw0_25,
	NULL
    }
};

static IPv4RouteTest test11 = {
    "test11",
    {
	&en0_10_never,
	&en1_20,
	&fw0_25,
	NULL
    }
};

static IPv4RouteTest test12 = {
    "test12",
    {
	&en0_10,
	&en1_20,
	NULL
    }
};

static IPv4RouteTest test13 = {
    "test13",
    {
	&en0_10,
	&en1_20_never,
	NULL
    }
};

static IPv4RouteTest test14 = {
    "test14",
    {
	&en1_20_never,
	NULL
    }
};

static IPv4RouteTest test15 = {
    "test15",
    {
	&en0_linklocal,
	NULL
    }
};

static IPv4RouteTest test16 = {
    "test16",
    {
	&en0_10,
	&utun0,
	NULL
    }
};

static IPv4RouteTest test17 = {
    "test17",
    {
	&en0_10,
	&en1_20_other_never,
	NULL
    }
};

static IPv4RouteTest test18 = {
    "test18",
    {
	&en0_route_loop,
	NULL
    }
};

static IPv4RouteTestRef ipv4_tests[] = {
    &test1,
    &test2,
    &test3,
    &test4,
    &test5,
    &test6,
    &test7,
    &test8,
    &test9,
    &test10,
    &test11,
    &test12,
    &test13,
    &test14,
    &test15,
    &test16,
    &test17,
    &test18,
    NULL
};

static boolean_t
ipv4_prefix_length_is_valid(int prefix_length)
{
    if (prefix_length < 0 || prefix_length > IPV4_ROUTE_ALL_BITS_SET) {
	return (FALSE);
    }
    return (TRUE);
}

static void
dict_add_string(CFMutableDictionaryRef dict, CFStringRef prop_name,
		const char * str)
{
    CFStringRef		prop_val;

    if (str == NULL) {
	return;
    }
    prop_val = CFStringCreateWithCString(NULL,
					 str,
					 kCFStringEncodingASCII);
    CFDictionarySetValue(dict, prop_name, prop_val);
    CFRelease(prop_val);
    return;
}

static void
dict_add_string_as_array(CFMutableDictionaryRef dict, CFStringRef prop_name,
			 const char * str)
{
    CFArrayRef		array;
    CFStringRef		prop_val;

    if (str == NULL) {
	return;
    }
    prop_val = CFStringCreateWithCString(NULL,
					 str,
					 kCFStringEncodingASCII);
    array = CFArrayCreate(NULL,
			  (const void **)&prop_val, 1,
			  &kCFTypeArrayCallBacks);
    CFRelease(prop_val);
    CFDictionarySetValue(dict, prop_name, array);
    CFRelease(array);
    return;
}

static void
dict_add_ip(CFMutableDictionaryRef dict, CFStringRef prop_name,
	    struct in_addr ip)
{
    CFStringRef		str;

    str = my_CFStringCreateWithInAddr(ip);
    CFDictionarySetValue(dict, prop_name, str);
    CFRelease(str);
    return;
}

static void
dict_add_ip_as_array(CFMutableDictionaryRef dict, CFStringRef prop_name,
		     struct in_addr ip)
{
    CFArrayRef		array;
    CFStringRef		str;

    str = my_CFStringCreateWithInAddr(ip);
    array = CFArrayCreate(NULL,
			  (const void **)&str, 1,
			  &kCFTypeArrayCallBacks);
    CFRelease(str);
    CFDictionarySetValue(dict, prop_name, array);
    CFRelease(array);
    return;
}

static void
dict_insert_routes(CFMutableDictionaryRef dict, CFStringRef prop_name,
		   struct route * routes, int routes_count)
{
    int			i;
    CFMutableArrayRef	route_list;
    struct route *	scan;

    if (routes == NULL || routes_count == 0) {
	return;
    }
    route_list = CFArrayCreateMutable(NULL, routes_count,
				      &kCFTypeArrayCallBacks);
    for (i = 0, scan = routes; i < routes_count; i++, scan++) {
	struct in_addr			mask;
	CFMutableDictionaryRef		route_dict;

	route_dict
	    = CFDictionaryCreateMutable(NULL, 0,
					&kCFTypeDictionaryKeyCallBacks,
					&kCFTypeDictionaryValueCallBacks);
	dict_add_string(route_dict, kSCPropNetIPv4RouteDestinationAddress,
			scan->dest);
	if (ipv4_prefix_length_is_valid(scan->prefix_length)) {
	    mask.s_addr = htonl(prefix_to_mask32(scan->prefix_length));
	    dict_add_ip(route_dict, kSCPropNetIPv4RouteSubnetMask, mask);
	}
	dict_add_string(route_dict, kSCPropNetIPv4RouteGatewayAddress,
			scan->gateway);
	dict_add_string(route_dict, kSCPropNetIPv4RouteInterfaceName,
			scan->ifname);
	CFArrayAppendValue(route_list, route_dict);
	CFRelease(route_dict);
    }
    CFDictionarySetValue(dict, prop_name, route_list);
    CFRelease(route_list);
    return;
}

static CFDictionaryRef
make_IPv4_dict(IPv4ServiceContentsRef t)
{
    CFMutableDictionaryRef	dict;

    dict = CFDictionaryCreateMutable(NULL, 0,
				     &kCFTypeDictionaryKeyCallBacks,
				     &kCFTypeDictionaryValueCallBacks);
    dict_add_string_as_array(dict, kSCPropNetIPv4Addresses, t->addr);
    if (ipv4_prefix_length_is_valid(t->prefix_length)) {
	struct in_addr		mask;

	mask.s_addr = htonl(prefix_to_mask32(t->prefix_length));
	dict_add_ip_as_array(dict, kSCPropNetIPv4SubnetMasks, mask);
    }
    dict_add_string_as_array(dict, kSCPropNetIPv4DestAddresses, t->dest);
    dict_add_string(dict, kSCPropNetIPv4Router, t->router);
    dict_add_string(dict, kSCPropInterfaceName, t->ifname);
    dict_add_string(dict, kSCPropConfirmedInterfaceName, t->ifname);
    dict_insert_routes(dict, kSCPropNetIPv4AdditionalRoutes,
		       t->additional_routes, t->additional_routes_count);
    dict_insert_routes(dict, kSCPropNetIPv4ExcludedRoutes,
		       t->excluded_routes, t->excluded_routes_count);
    return (dict);
}

typedef enum {
    kDirectionForwards = 0,
    kDirectionBackwards = 1
} Direction;

typedef enum {
    kLogRouteDisabled = 0,
    kLogRouteEnabled = 1
} LogRoute;

static IPv4RouteListRef
make_IPv4RouteList_for_test(IPv4RouteListRef list,
			    IPv4ServiceContentsRef test,
			    LogRoute log_it)
{
    CFDictionaryRef	dict;
    IPv4RouteListRef	r;
    Rank		rank;
    Rank		rank_assertion = kRankAssertionDefault;
    CFNumberRef		rank_assertion_cf = NULL;
    Boolean		rank_assertion_is_set = FALSE;
    IPv4RouteListRef	ret = NULL;
    IPV4_ROUTES_BUF_DECL(routes);

    dict = make_IPv4_dict(test);
    if (dict == NULL) {
	fprintf(stderr, "make_IPv4_dict failed\n");
	exit(1);
    }
    if (test->primary_rank != NULL) {
	rank_assertion
	    = PrimaryRankGetRankAssertion(*test->primary_rank,
					  &rank_assertion_is_set);
	if (rank_assertion_is_set) {
	    rank_assertion_cf
		= CFNumberCreate(NULL, kCFNumberSInt32Type, &rank_assertion);
	}
    }
    r = IPv4RouteListCreateWithDictionary(routes, dict,
					  rank_assertion_cf);
    my_CFRelease(&rank_assertion_cf);
    if (r == NULL) {
	fprintf(stderr, "IPv4RouteListCreateWithDictionary failed\n");
	exit(1);
    }

    if (rank_assertion == kRankAssertionScoped) {
	rank_assertion = kRankAssertionNever;
    }
    rank = RankMake(test->rank, rank_assertion);
    if (log_it == kLogRouteEnabled
	&& (S_IPMonitor_debug & kDebugFlag4) != 0) {
	CFStringRef	descr;

	descr = IPv4RouteListCopyDescription(r);
	SCLog(TRUE, LOG_NOTICE, CFSTR("Adding %@"), descr);
	CFRelease(descr);
    }
    ret = IPv4RouteListAddRouteList(list, 1, r, rank);
    if (r != routes) {
	free(r);
    }
    CFRelease(dict);
    return (ret);
}

static IPv4RouteListRef
make_IPv4RouteList(IPv4ServiceContentsRef * test, Direction direction,
		   LogRoute log_it)
{
    IPv4RouteListRef		ret = NULL;
    IPv4ServiceContentsRef * 	scan;

    switch (direction) {
    case kDirectionBackwards:
	for (scan = test; *scan != NULL; scan++) {
	    /* find the end of the list */
	}
	for (scan--; scan >= test; scan--) {
	    ret = make_IPv4RouteList_for_test(ret, *scan, log_it);
	}
	break;
    default:
    case kDirectionForwards:
	for (scan = test; *scan != NULL; scan++) {
	    ret = make_IPv4RouteList_for_test(ret, *scan, log_it);
	}
	break;
    }
    IPv4RouteListFinalize(ret);
    return (ret);
}

#define EMPHASIS_CHARS		"================="

/*
 * Function: routelist_build_test
 * Purpose:
 *   Runs through the given set of routes first in the forward direction,
 *   then again backwards.  We should end up with exactly the same set of
 *   routes at the end.
 */
static boolean_t
routelist_build_test(IPv4RouteTestRef test)
{
    CFStringRef			descr;
    boolean_t			ret = FALSE;
    IPv4RouteListRef		routes1;
    IPv4RouteListRef		routes2;

    printf("\n" EMPHASIS_CHARS  "> RouteList Build '%s' <"
	   EMPHASIS_CHARS "\n",
	   test->name);

    routes1 = make_IPv4RouteList(test->test, kDirectionForwards,
				 kLogRouteEnabled);
    if ((S_IPMonitor_debug & kDebugFlag4) != 0) {
	if (routes1 != NULL) {
	    descr = IPv4RouteListCopyDescription(routes1);
	    SCPrint(TRUE, stdout, CFSTR("Routes are %@\n"), descr);
	    CFRelease(descr);
	}
    }
    routes2 = make_IPv4RouteList(test->test, kDirectionBackwards,
				 kLogRouteEnabled);
    if ((S_IPMonitor_debug & kDebugFlag4) != 0) {
	if (routes2 != NULL) {
	    descr = IPv4RouteListCopyDescription(routes2);
	    SCPrint(TRUE, stdout, CFSTR("Routes are %@\n"), descr);
	    CFRelease(descr);
	}
    }
    if ((routes1 != NULL && routes2 == NULL)
	|| (routes1 == NULL && routes2 != NULL)) {
	fprintf(stderr, "routes1 is %sNULL but routes2 is %sNULL\n",
	       (routes1 != NULL) ? "not " : "",
	       (routes2 != NULL) ? "not " : "");
    }
    else if (routes1 != NULL && routes2 != NULL) {
	/* check if they are different */
	if (routes1->count != routes2->count) {
	    fprintf(stderr, "routes1 count %d != routes 2 count %d\n",
		    routes1->count, routes2->count);
	}
	else if (bcmp(routes1, routes2,
		      IPv4RouteListComputeSize(routes1->count)) != 0) {
	    fprintf(stderr, "routes1 and routes2 are different\n");
	}
	else {
	    printf("routes1 and routes2 are the same\n");
	    ret = TRUE;
	}
    }
    if (routes1 != NULL) {
	free(routes1);
    }
    if (routes2 != NULL) {
	free(routes2);
    }
    printf(EMPHASIS_CHARS  "> RouteList Build '%s': %s <"
	   EMPHASIS_CHARS "\n",
	   test->name, ret ? "PASSED" : "FAILED");
    return (ret);
}

static void
apply_test(IPv4RouteTestRef old_test, IPv4RouteTestRef new_test)
{
    IPv4RouteListRef	new_routes;
    IPv4RouteListRef	old_routes;

    printf("\n" EMPHASIS_CHARS  "> Apply '%s', '%s' Begin <"
	   EMPHASIS_CHARS "\n",
	   old_test->name, new_test->name);

    old_routes = make_IPv4RouteList(old_test->test, kDirectionForwards,
				    kLogRouteDisabled);
    new_routes = make_IPv4RouteList(new_test->test, kDirectionForwards,
				    kLogRouteDisabled);
    if (old_routes == NULL) {
	printf("No Old Routes\n");
    }
    else {
	printf("Old routes ('%s') = ", old_test->name);
	IPv4RouteListPrint(old_routes);
    }

    /* apply the old routes */
    IPv4RouteListApply(NULL, old_routes, -1);

    if (new_routes == NULL) {
	printf("No New Routes\n");
    }
    else {
	printf("New Routes ('%s') = ", new_test->name);
	IPv4RouteListPrint(new_routes);
    }

    /* apply the new routes */
    IPv4RouteListApply(old_routes, new_routes, -1);

    if (old_routes != NULL) {
	free(old_routes);
    }
    if (new_routes != NULL) {
	free(new_routes);
    }
    printf(EMPHASIS_CHARS  "> Apply '%s', '%s' End <"
	   EMPHASIS_CHARS "\n",
	   old_test->name, new_test->name);
    return;
}

int
main(int argc, char **argv)
{
    IPv4RouteTestRef *	test;

    _sc_log     = FALSE;
    _sc_verbose = (argc > 1) ? TRUE : FALSE;
    S_IPMonitor_debug = kDebugFlag1 | kDebugFlag2 | kDebugFlag4;
    if (argc > 1) {
	S_IPMonitor_debug = strtoul(argv[1], NULL, 0);
    }
    S_scopedroute = (argc < 3);
    for (test = ipv4_tests; *test != NULL; test++) {
	if (routelist_build_test(*test) == FALSE) {
	    fprintf(stderr, "%s failed\n", (*test)->name);
	    exit(1);
	}
    }
    for (test = ipv4_tests; *test != NULL; test++) {
	IPv4RouteTestRef *	test2;

	for (test2 = test + 1; *test2 != NULL; test2++) {
	    apply_test(*test, *test2);
	    apply_test(*test2, *test);
	}
    }

    {
	char    cmd[128];

	printf("\nChecking for leaks\n");
	sprintf(cmd, "leaks %d 2>&1", getpid());
	fflush(stdout);
	(void)system(cmd);
    }
    exit(0);
    return (0);
}

#endif /* TEST_IPV4_ROUTELIST */

#ifdef TEST_IPV6_ROUTELIST

typedef struct {
    const char *	addr;
    int			prefix_length;
    const char *	dest;
} IPv6Address;

typedef const IPv6Address * IPv6AddressRef;

typedef struct {
    IPv6AddressRef	addr;
    int			addr_count;
    const char *	router;
    const char *	ifname;
    Rank		rank;
    const CFStringRef *	primary_rank;
    struct route *	additional_routes;
    int			additional_routes_count;
    struct route *	excluded_routes;
    int			excluded_routes_count;
} IPv6ServiceContents;

typedef const IPv6ServiceContents * IPv6ServiceContentsRef;

struct route loop_routelist[] = {
    { "2620:149:4:f01:225:ff:fecc:89a1", 128,
      "2620:149:4:f01:225:ff:fecc:89a2", NULL },
    { "2620:149:4:f01:225:ff:fecc:89a2", 128,
      "2620:149:4:f01:225:ff:fecc:89a3", NULL },
    { "2620:149:4:f01:225:ff:fecc:89a3", 128,
      "2620:149:4:f01:225:ff:fecc:89a4", NULL },
    { "2620:149:4:f01:225:ff:fecc:89a4", 128,
      "2620:149:4:f01:225:ff:fecc:89a5", NULL },
    { "2620:149:4:f01:225:ff:fecc:89a5", 128,
      "2620:149:4:f01:225:ff:fecc:89a6", NULL },
    { "2620:149:4:f01:225:ff:fecc:89a6", 128,
      "2620:149:4:f01:225:ff:fecc:89a7", NULL },
    { "2620:149:4:f01:225:ff:fecc:89a7", 128,
      "2620:149:4:f01:225:ff:fecc:89a8", NULL },
    { "2620:149:4:f01:225:ff:fecc:89a8", 128,
      "2620:149:4:f01:225:ff:fecc:89a9", NULL },
    { "2620:149:4:f01:225:ff:fecc:89a9", 128,
      "2620:149:4:f01:225:ff:fecc:89aa", NULL },
    { "2620:149:4:f01:225:ff:fecc:89aa", 128,
      "2620:149:4:f01:225:ff:fecc:89ab", NULL },
    { "2620:149:4:f01:225:ff:fecc:89ab", 128,
      "2620:149:4:f01:225:ff:fecc:89a1", NULL },
};

struct route vpn_routelist[] = {
    { "2010:470:1f05:3cb::", 64,
      "fe80::2d0:bcff:fe3d:8c00", NULL },
    { "2010:222:3fa5:acb::", 48,
      "fe80::2d0:bcff:fe3d:8c00", NULL },
    { "2010:222:3fa5:1234::", 40,
      "fe80::2d0:bcff:fe3d:8c00", NULL },
    { "2010:222:3fa5:5678::", 40,
      NULL, NULL },
};

struct route vpn_routelist_ext[] = {
    { "2020:299:a:e02:825:1ed:fecc:abab", 128, NULL, NULL },
};

struct route en1_routelist_ext[] = {
    { "2020:299:abcd:ef12::", 64, NULL, NULL },
};


static const IPv6Address en0_addr1[] = {
    { "2001:470:1f05:3cb:cabc:c8ff:fe96:9601", 64, NULL },
    { "2001:470:1f05:3cb:5c95:58b1:b956:6101", 64, NULL }
};

static const IPv6Address en0_addr2[] = {
    { "2001:470:1f05:3cb:cabc:c8ff:fe96:9602", 64, NULL },
    { "2001:470:1f05:3cb:5c95:58b1:b956:6102", 64, NULL }
};

static const IPv6Address en0_addr3[] = {
    { "2001:470:1f05:3cb:cabc:c8ff:fe96:9603", 64, NULL },
    { "2001:470:1f05:3cb:5c95:58b1:b956:6103", 64, NULL }
};

static const IPv6Address en0_addr4[] = {
    { "2001:470:1f05:3cb:cabc:c8ff:fe96:9604", 64, NULL },
    { "2001:470:1f05:3cb:5c95:58b1:b956:6104", 64, NULL }
};

static const IPv6Address en0_addr5[] = {
    { "2001:470:1f05:3cb:cabc:c8ff:fe96:9605", 64, NULL },
    { "2001:470:1f05:3cb:5c95:58b1:b956:6105", 64, NULL }
};

static const IPv6Address en0_addr6[] = {
    { "2020:299:abcd:ef12:1:2:3:4", 64, NULL },
};

static const IPv6Address en0_lladdr[] = {
    { "fe80::cabc:c8ff:fe96:96af", 64, NULL }
};

static const IPv6Address en1_addr[] = {
    { "2001:470:1f05:3cb:cabc:c8ff:fed9:125a", 64, NULL },
    { "2001:470:1f05:3cb:2d5e:4ec3:304:5b9c", 64, NULL }
};

static const IPv6Address utun0_addr[] = {
    { "2620:149:4:f01:225:ff:fecc:89aa", 64, NULL },
};

static const IPv6Address fw0_addr1[] = {
    { "2011:470:1f05:3cb:cabc:c8ff:fe96:ab01", 64, NULL },
    { "2011:470:1f05:3cb:5c95:58b1:b956:ab01", 64, NULL }
};

/*
 * address+address-count
 * router ifname pri rank additional-routes+count excluded-routes+count
 */

static const IPv6ServiceContents en0_10 = {
    en0_addr1, countof(en0_addr1),
    "fe80::21f:f3ff:fe43:1abf", "en0", 10, NULL, NULL, 0, NULL, 0
};

static const IPv6ServiceContents en0_15 = {
    en0_addr2, countof(en0_addr2),
    "fe80::21f:f3ff:fe43:1abf", "en0", 15, NULL, NULL, 0, NULL, 0
};

static const IPv6ServiceContents en0_30 = {
    en0_addr3, countof(en0_addr3),
    "fe80::21f:f3ff:fe43:1abf", "en0", 30, NULL, NULL, 0, NULL, 0
};

static const IPv6ServiceContents en0_40 = {
    en0_addr4, countof(en0_addr4),
    "fe80::21f:f3ff:fe43:1abf", "en0", 40, NULL, NULL, 0, NULL, 0
};

static const IPv6ServiceContents en0_50 = {
    en0_addr5, countof(en0_addr5),
    "fe80::21f:f3ff:fe43:1abf", "en0", 50, NULL, NULL, 0, NULL, 0
};

static const IPv6ServiceContents en0_10_a = {
    en0_addr6, countof(en0_addr6),
    "fe80::21f:f3ff:fe43:1abf", "en0", 10, NULL, NULL, 0, NULL, 0
};

static const IPv6ServiceContents fw0_25 = {
    fw0_addr1, countof(fw0_addr1),
    "fe80::21f:f3ff:fe43:1abf", "fw0", 25, NULL, NULL, 0, NULL, 0
};

static const IPv6ServiceContents en1_20 = {
    en1_addr, countof(en1_addr),
    "fe80::21f:f3ff:fe43:1abf", "en1", 20, NULL, NULL, 0, NULL, 0
};

static const IPv6ServiceContents en1_10_ext = {
    en1_addr, countof(en1_addr),
    "fe80::21f:f3ff:fe43:1abf", "en1", 10, NULL, NULL, 0,
    en1_routelist_ext, countof(en1_routelist_ext)
};

static const IPv6ServiceContents en0_0_lladdr = {
    en0_lladdr, countof(en0_lladdr),
    "fe80::21f:f3ff:fe43:1abf", "en0", 20, NULL, NULL, 0, NULL, 0
};

static const IPv6ServiceContents en0_loop = {
    en0_addr1, countof(en0_addr1),
    "fe80::21f:f3ff:fe43:1abf", "en0", 10, NULL,
    loop_routelist, countof(loop_routelist), NULL, 0
};

static const IPv6ServiceContents utun0 = {
    utun0_addr, countof(utun0_addr),
    "fe80::2d0:bcff:fe3d:8c00", "utun0", 40, NULL,
    vpn_routelist, countof(vpn_routelist),
    vpn_routelist_ext, countof(vpn_routelist_ext),
};

typedef struct {
    const char *		name;
    IPv6ServiceContentsRef	test[];
} IPv6RouteTest, * IPv6RouteTestRef;

static IPv6RouteTest test1 = {
    "test1",
    {
	&en0_40,
	&en0_15,
	&fw0_25,
	&en0_30,
	&en1_20,
	&en0_50,
	&en0_10,
	NULL
    }
};

static IPv6RouteTest test2 = {
    "test2",
    {
	&en0_40,
	&fw0_25,
	&en0_30,
	&en1_20,
	&en0_50,
	&en0_10,
	NULL
    }
};

static IPv6RouteTest test3 = {
    "test3",
    {
	&en0_10_a,
	&en1_10_ext,
	NULL
    }
};

static IPv6RouteTest test4 = {
    "test4",
    {
	&en0_loop,
	&en1_20,
	NULL
    }
};

static IPv6RouteTest test5 = {
    "test5",
    {
	&en0_10,
	&utun0,
	&en0_0_lladdr,
	&en1_20,
	NULL
    }
};


static IPv6RouteTestRef ipv6_tests[] = {
    &test1,
    &test2,
    &test3,
    &test4,
    &test5,
    NULL
};


static void
dict_add_string(CFMutableDictionaryRef dict, CFStringRef prop_name,
		const char * str)
{
    CFStringRef		prop_val;

    if (str == NULL) {
	return;
    }
    prop_val = CFStringCreateWithCString(NULL,
					 str,
					 kCFStringEncodingASCII);
    CFDictionarySetValue(dict, prop_name, prop_val);
    CFRelease(prop_val);
    return;
}

static void
dict_add_int(CFMutableDictionaryRef dict, CFStringRef prop_name,
	     int int_val)
{
    CFNumberRef		num;

    num = CFNumberCreate(NULL, kCFNumberIntType, &int_val);
    CFDictionarySetValue(dict, prop_name, num);
    CFRelease(num);
    return;
}

static void
dict_insert_v6_routes(CFMutableDictionaryRef dict, CFStringRef prop_name,
		      struct route * routes, int routes_count)
{
    int			i;
    CFMutableArrayRef	route_list;
    struct route *	scan;

    if (routes == NULL || routes_count == 0) {
	return;
    }
    route_list = CFArrayCreateMutable(NULL, routes_count,
				      &kCFTypeArrayCallBacks);
    for (i = 0, scan = routes; i < routes_count; i++, scan++) {
	CFMutableDictionaryRef		route_dict;

	route_dict = CFDictionaryCreateMutable(NULL, 0,
					       &kCFTypeDictionaryKeyCallBacks,
					       &kCFTypeDictionaryValueCallBacks);
	dict_add_string(route_dict, kSCPropNetIPv6RouteDestinationAddress,
			scan->dest);
	dict_add_int(route_dict, kSCPropNetIPv6PrefixLength,
		     scan->prefix_length);
	dict_add_string(route_dict, kSCPropNetIPv6RouteGatewayAddress,
			scan->gateway);
	dict_add_string(route_dict, kSCPropNetIPv6RouteInterfaceName,
			scan->ifname);
	CFArrayAppendValue(route_list, route_dict);
	CFRelease(route_dict);
    }
    CFDictionarySetValue(dict, prop_name, route_list);
    CFRelease(route_list);
    return;
}

static void
array_add_string(CFMutableArrayRef array, const char * c_str)
{
    CFStringRef		str;

    str = CFStringCreateWithCString(NULL,
				    c_str,
				    kCFStringEncodingUTF8);
    CFArrayAppendValue(array, str);
    CFRelease(str);
    return;
}

static void
array_add_int(CFMutableArrayRef array, int int_val)
{
    CFNumberRef		num;

    num = CFNumberCreate(NULL, kCFNumberIntType, &int_val);
    CFArrayAppendValue(array, num);
    CFRelease(num);
    return;
}

static void
dict_add_ipv6_addressing(CFMutableDictionaryRef dict,
			 IPv6AddressRef list, int list_count)
{
    CFMutableArrayRef	addr = NULL;
    CFMutableArrayRef	dest = NULL;
    int			i;
    CFMutableArrayRef	prefix = NULL;
    IPv6AddressRef	scan;

    if (list == NULL || list_count == 0) {
	return;
    }
    for (i = 0, scan = list; i < list_count; i++, scan++) {
	if (scan->addr != NULL) {
	    if (addr == NULL) {
		addr = CFArrayCreateMutable(NULL, list_count,
					    &kCFTypeArrayCallBacks);
	    }
	    array_add_string(addr, scan->addr);
	}
	if (scan->prefix_length >= 0) {
	    if (prefix == NULL) {
		prefix = CFArrayCreateMutable(NULL, list_count,
					      &kCFTypeArrayCallBacks);
	    }
	    array_add_int(prefix, scan->prefix_length);
	}
	if (scan->dest != NULL) {
	    if (dest == NULL) {
		dest = CFArrayCreateMutable(NULL, list_count,
					    &kCFTypeArrayCallBacks);
	    }
	    array_add_string(dest, scan->dest);
	}
    }
    if (addr != NULL) {
	CFDictionarySetValue(dict, kSCPropNetIPv6Addresses, addr);
	CFRelease(addr);
    }
    if (dest != NULL) {
	CFDictionarySetValue(dict, kSCPropNetIPv6DestAddresses, dest);
	CFRelease(dest);
    }
    if (prefix != NULL) {
	CFDictionarySetValue(dict, kSCPropNetIPv6PrefixLength, prefix);
	CFRelease(prefix);
    }
    return;
}

static CFDictionaryRef
make_IPv6_dict(IPv6ServiceContentsRef t)
{
    CFMutableDictionaryRef	dict;

    dict = CFDictionaryCreateMutable(NULL, 0,
				     &kCFTypeDictionaryKeyCallBacks,
				     &kCFTypeDictionaryValueCallBacks);
    dict_add_ipv6_addressing(dict, t->addr, t->addr_count);
    dict_add_string(dict, kSCPropNetIPv6Router, t->router);
    dict_add_string(dict, kSCPropInterfaceName, t->ifname);
    dict_insert_v6_routes(dict, kSCPropNetIPv6AdditionalRoutes,
			  t->additional_routes, t->additional_routes_count);
    dict_insert_v6_routes(dict, kSCPropNetIPv6ExcludedRoutes,
			  t->excluded_routes, t->excluded_routes_count);
    return (dict);
}

typedef enum {
    kDirectionForwards = 0,
    kDirectionBackwards = 1
} Direction;

typedef enum {
    kLogRouteDisabled = 0,
    kLogRouteEnabled = 1
} LogRoute;

static IPv6RouteListRef
make_IPv6RouteList_for_test(IPv6RouteListRef list,
			    IPv6ServiceContentsRef test,
			    LogRoute log_it)
{
    CFDictionaryRef	dict;
    IPv6RouteListRef	r;
    Rank		rank;
    Rank		rank_assertion = kRankAssertionDefault;
    CFNumberRef		rank_assertion_cf = NULL;
    Boolean		rank_assertion_is_set = FALSE;
    IPv6RouteListRef	ret = NULL;
    IPV6_ROUTES_BUF_DECL(routes);

    dict = make_IPv6_dict(test);
    if (dict == NULL) {
	fprintf(stderr, "make_IPv6_dict failed\n");
	exit(1);
    }
    if (test->primary_rank != NULL) {
	rank_assertion
	    = PrimaryRankGetRankAssertion(*test->primary_rank,
					  &rank_assertion_is_set);
	if (rank_assertion_is_set) {
	    rank_assertion_cf
		= CFNumberCreate(NULL, kCFNumberSInt32Type, &rank_assertion);
	}
    }
    r = IPv6RouteListCreateWithDictionary(routes, dict,
					  rank_assertion_cf);
    my_CFRelease(&rank_assertion_cf);
    if (r == NULL) {
	fprintf(stderr, "IPv6RouteListCreateWithDictionary failed\n");
	exit(1);
    }

    if (rank_assertion == kRankAssertionScoped) {
	rank_assertion = kRankAssertionNever;
    }
    rank = RankMake(test->rank, rank_assertion);
    if (log_it == kLogRouteEnabled
	&& (S_IPMonitor_debug & kDebugFlag4) != 0) {
	CFStringRef	descr;

	descr = IPv6RouteListCopyDescription(r);
	SCLog(TRUE, LOG_NOTICE, CFSTR("Adding %@"), descr);
	CFRelease(descr);
    }
    ret = IPv6RouteListAddRouteList(list, 1, r, rank);
    if (r != routes) {
	free(r);
    }
    CFRelease(dict);
    return (ret);
}

static IPv6RouteListRef
make_IPv6RouteList(IPv6ServiceContentsRef * test, Direction direction,
		   LogRoute log_it)
{
    IPv6RouteListRef		ret = NULL;
    IPv6ServiceContentsRef * 	scan;

    switch (direction) {
    case kDirectionBackwards:
	for (scan = test; *scan != NULL; scan++) {
	    /* find the end of the list */
	}
	for (scan--; scan >= test; scan--) {
	    ret = make_IPv6RouteList_for_test(ret, *scan, log_it);
	}
	break;
    default:
    case kDirectionForwards:
	for (scan = test; *scan != NULL; scan++) {
	    ret = make_IPv6RouteList_for_test(ret, *scan, log_it);
	}
	break;
    }
    IPv6RouteListFinalize(ret);
    return (ret);
}

#define EMPHASIS_CHARS	"================="

/*
 * Function: routelist_build_test
 * Purpose:
 *   Runs through the given set of routes first in the forward direction,
 *   then again backwards.  We should end up with exactly the same set of
 *   routes at the end.
 */
static boolean_t
routelist_build_test(IPv6RouteTestRef test)
{
    CFStringRef			descr;
    boolean_t			ret = FALSE;
    IPv6RouteListRef		routes1;
    IPv6RouteListRef		routes2;

    printf("\n" EMPHASIS_CHARS  "> RouteList Build '%s' <"
	   EMPHASIS_CHARS "\n",
	   test->name);

    routes1 = make_IPv6RouteList(test->test, kDirectionForwards,
				 kLogRouteEnabled);
    if ((S_IPMonitor_debug & kDebugFlag4) != 0) {
	if (routes1 != NULL) {
	    descr = IPv6RouteListCopyDescription(routes1);
	    SCPrint(TRUE, stdout, CFSTR("Routes are %@\n"), descr);
	    CFRelease(descr);
	}
    }
    routes2 = make_IPv6RouteList(test->test, kDirectionBackwards,
				 kLogRouteEnabled);
    if ((S_IPMonitor_debug & kDebugFlag4) != 0) {
	if (routes2 != NULL) {
	    descr = IPv6RouteListCopyDescription(routes2);
	    SCPrint(TRUE, stdout, CFSTR("Routes are %@\n"), descr);
	    CFRelease(descr);
	}
    }
    if ((routes1 != NULL && routes2 == NULL)
	|| (routes1 == NULL && routes2 != NULL)) {
	fprintf(stderr, "routes1 is %sNULL but routes2 is %sNULL\n",
	       (routes1 != NULL) ? "not " : "",
	       (routes2 != NULL) ? "not " : "");
    }
    else if (routes1 != NULL && routes2 != NULL) {
	/* check if they are different */
	if (routes1->count != routes2->count) {
	    fprintf(stderr, "routes1 count %d != routes 2 count %d\n",
		    routes1->count, routes2->count);
	}
	else if (bcmp(routes1, routes2,
		      IPv6RouteListComputeSize(routes1->count)) != 0) {
	    fprintf(stderr, "routes1 and routes2 are different\n");
	}
	else {
	    printf("routes1 and routes2 are the same\n");
	    ret = TRUE;
	}
    }
    if (routes1 != NULL) {
	free(routes1);
    }
    if (routes2 != NULL) {
	free(routes2);
    }
    printf(EMPHASIS_CHARS  "> RouteList Build '%s': %s <"
	   EMPHASIS_CHARS "\n",
	   test->name, ret ? "PASSED" : "FAILED");
    return (ret);
}

static void
apply_test(IPv6RouteTestRef old_test, IPv6RouteTestRef new_test)
{
    IPv6RouteListRef	new_routes;
    IPv6RouteListRef	old_routes;

    printf("\n" EMPHASIS_CHARS  "> Apply '%s', '%s' Begin <"
	   EMPHASIS_CHARS "\n",
	   old_test->name, new_test->name);

    old_routes = make_IPv6RouteList(old_test->test, kDirectionForwards,
				    kLogRouteDisabled);
    new_routes = make_IPv6RouteList(new_test->test, kDirectionForwards,
				    kLogRouteDisabled);
    if (old_routes == NULL) {
	printf("No Old Routes\n");
    }
    else {
	printf("Old routes ('%s') = ", old_test->name);
	IPv6RouteListPrint(old_routes);
    }

    /* apply the old routes */
    IPv6RouteListApply(NULL, old_routes, -1);
    if (new_routes == NULL) {
	printf("No New Routes\n");
    }
    else {
	printf("New Routes ('%s') = ", new_test->name);
	IPv6RouteListPrint(new_routes);
    }

    /* apply the new routes */
    IPv6RouteListApply(old_routes, new_routes, -1);
    if (old_routes != NULL) {
	free(old_routes);
    }
    if (new_routes != NULL) {
	free(new_routes);
    }
    printf(EMPHASIS_CHARS  "> Apply '%s', '%s' End <"
	   EMPHASIS_CHARS "\n",
	   old_test->name, new_test->name);
    return;
}

int
main(int argc, char **argv)
{
    IPv6RouteTestRef *	test;

    _sc_log     = FALSE;
    _sc_verbose = (argc > 1) ? TRUE : FALSE;
    S_IPMonitor_debug = kDebugFlag1 | kDebugFlag2 | kDebugFlag4;
    if (argc > 1) {
	S_IPMonitor_debug = strtoul(argv[1], NULL, 0);
    }
    S_scopedroute_v6 = (argc < 3);
    for (test = ipv6_tests; *test != NULL; test++) {
	if (routelist_build_test(*test) == FALSE) {
	    fprintf(stderr, "%s failed\n", (*test)->name);
	    exit(1);
	}
    }
    for (test = ipv6_tests; *test != NULL; test++) {
	IPv6RouteTestRef *	test2;

	for (test2 = test + 1; *test2 != NULL; test2++) {
	    apply_test(*test, *test2);
	    apply_test(*test2, *test);
	}
    }

    {
	char    cmd[128];

	printf("\nChecking for leaks\n");
	sprintf(cmd, "leaks %d 2>&1", getpid());
	fflush(stdout);
	(void)system(cmd);
    }
    exit(0);
    return (0);
}

#endif /* TEST_IPV6_ROUTELIST */


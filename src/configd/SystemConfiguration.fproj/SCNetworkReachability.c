/*
 * Copyright (c) 2003-2015 Apple Inc. All rights reserved.
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
 * Modification History
 *
 * April 12, 2011		Allan Nathanson <ajn@apple.com>
 * - add SCNetworkReachability "server"
 *
 * March 31, 2004		Allan Nathanson <ajn@apple.com>
 * - use [SC] DNS configuration information
 *
 * January 19, 2003		Allan Nathanson <ajn@apple.com>
 * - add advanced reachability APIs
 */

#include <Availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <dispatch/dispatch.h>
#include <dispatch/private.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCValidation.h>
#include <SystemConfiguration/SCPrivate.h>
#include <SystemConfiguration/VPNAppLayerPrivate.h>
#include <pthread.h>
#include <libkern/OSAtomic.h>

#if	!TARGET_OS_IPHONE
#include <IOKit/pwr_mgt/IOPMLibPrivate.h>
#endif	// !TARGET_OS_IPHONE

#include <notify.h>
#include <dnsinfo.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <resolv.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#define	KERNEL_PRIVATE
#include <net/route.h>
#undef	KERNEL_PRIVATE

#ifndef s6_addr16
#define s6_addr16 __u6_addr.__u6_addr16
#endif

#include "SCNetworkConnectionInternal.h"
#include "SCNetworkReachabilityInternal.h"

#include <ppp/ppp_msg.h>
#include <ppp/PPPControllerPriv.h>

#include <network_information.h>







#define	DEBUG_REACHABILITY_TYPE_NAME			"create w/name"
#define	DEBUG_REACHABILITY_TYPE_NAME_CLONE		"      > clone"
#define	DEBUG_REACHABILITY_TYPE_NAME_OPTIONS		"    + options"

#define	DEBUG_REACHABILITY_TYPE_ADDRESS			"create w/address"
#define	DEBUG_REACHABILITY_TYPE_ADDRESS_CLONE		"         > clone"
#define	DEBUG_REACHABILITY_TYPE_ADDRESS_OPTIONS		"       + options"

#define	DEBUG_REACHABILITY_TYPE_ADDRESSPAIR		"create w/address pair"
#define	DEBUG_REACHABILITY_TYPE_ADDRESSPAIR_CLONE	"              > clone"
#define	DEBUG_REACHABILITY_TYPE_ADDRESSPAIR_OPTIONS	"            + options"

#define	DEBUG_REACHABILITY_TYPE_PTR			"create w/ptr"
#define	DEBUG_REACHABILITY_TYPE_PTR_CLONE		"     > clone"
#define	DEBUG_REACHABILITY_TYPE_PTR_OPTIONS		"   + options"

#define	DNS_FLAGS_FORMAT	"[%s%s%s%s%s]"
#define	DNS_FLAGS_VALUES(t)	t->dnsHaveV4      ? "4" : "",	\
				t->dnsHaveV6      ? "6" : "",	\
				t->dnsHavePTR     ? "P" : "",	\
				t->dnsHaveTimeout ? "T" : "",	\
				t->dnsHaveError   ? "E" : ""


static pthread_mutexattr_t	lock_attr;

#define MUTEX_INIT(m) {							\
	int _lock_ = (pthread_mutex_init(m, &lock_attr) == 0);		\
	assert(_lock_);							\
}

#define	MUTEX_LOCK(m) {							\
	int _lock_ = (pthread_mutex_lock(m) == 0);			\
	assert(_lock_);							\
}

#define	MUTEX_UNLOCK(m) {						\
	int _unlock_ = (pthread_mutex_unlock(m) == 0);			\
	assert(_unlock_);						\
}

#define	MUTEX_ASSERT_HELD(m) {						\
	int _locked_ = (pthread_mutex_lock(m) == EDEADLK);		\
	assert(_locked_);						\
}


#define SCNETWORKREACHABILITY_TRIGGER_KEY	CFSTR("com.apple.SCNetworkReachability:FORCE-CHANGE")


#define	N_QUICK	64


static CFStringRef	__SCNetworkReachabilityCopyDescription	(CFTypeRef cf);
static void		__SCNetworkReachabilityDeallocate	(CFTypeRef cf);
static void		reachPerform				(void *info);
static Boolean		reachUpdate				(SCNetworkReachabilityRef target);


static void
__SCNetworkReachabilityHandleChanges		(SCDynamicStoreRef		store,
						 CFArrayRef			changedKeys,
						 void				*info);

static Boolean
__SCNetworkReachabilityScheduleWithRunLoop	(SCNetworkReachabilityRef	target,
						 CFRunLoopRef			runLoop,
						 CFStringRef			runLoopMode,
						 dispatch_queue_t		queue,
						 Boolean			onDemand);

static Boolean
__SCNetworkReachabilityUnscheduleFromRunLoop	(SCNetworkReachabilityRef	target,
						 CFRunLoopRef			runLoop,
						 CFStringRef			runLoopMode,
						 Boolean			onDemand);


static CFTypeID __kSCNetworkReachabilityTypeID	= _kCFRuntimeNotATypeID;


static const CFRuntimeClass __SCNetworkReachabilityClass = {
	0,					// version
	"SCNetworkReachability",		// className
	NULL,					// init
	NULL,					// copy
	__SCNetworkReachabilityDeallocate,	// dealloc
	NULL,					// equal
	NULL,					// hash
	NULL,					// copyFormattingDesc
	__SCNetworkReachabilityCopyDescription	// copyDebugDesc
};


static pthread_once_t		initialized	= PTHREAD_ONCE_INIT;
static const ReachabilityInfo	NOT_REACHABLE	= { 0, 0,		0, { 0 }, FALSE };
static const ReachabilityInfo	NOT_REPORTED	= { 0, 0xFFFFFFFF,	0, { 0 }, FALSE };
static int			rtm_seq		= 0;


static const struct timeval	TIME_ZERO	= { 0, 0 };


static int			dnsCount	= 0;
static int			dnsGeneration	= 0;
static DNSServiceRef		dnsMain		= NULL;
static CFMutableSetRef		dnsUpdated	= NULL;

static Boolean			D_serverBypass	= FALSE;



#if	!TARGET_OS_IPHONE
/*
 * Power capabilities (sleep/wake)
 */
#define POWER_CAPABILITIES_NETWORK	( kIOPMCapabilityCPU		\
					| kIOPMCapabilityNetwork	\
					| kIOPMCapabilityVideo)
static IOPMSystemPowerStateCapabilities	power_capabilities	= POWER_CAPABILITIES_NETWORK;
#endif	// !TARGET_OS_IPHONE


/*
 * host "something has changed" notifications
 */

// Note: protected by _hn_target_queue()
static SCDynamicStoreRef	hn_store	= NULL;
static CFMutableSetRef		hn_targets	= NULL;


static dispatch_queue_t
_hn_changes_queue()
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q = NULL;

	dispatch_once(&once, ^{
		q = dispatch_queue_create("SCNetworkReachability.handleChanges", NULL);
	});

	return q;
}


static dispatch_queue_t
_hn_target_queue()
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create("SCNetworkReachability.targetManagement", NULL);
	});

	return q;
}


/*
 * DNS configuration
 */

typedef struct {
	dns_config_t	*config;
	int		refs;
} dns_configuration_t;


// Note: protected by "dns_lock"
static pthread_mutex_t		dns_lock		= PTHREAD_MUTEX_INITIALIZER;
static dns_configuration_t	*dns_configuration	= NULL;
static int			dns_token;
static Boolean			dns_token_valid		= FALSE;




typedef enum {
	dns_query_async,
	dns_query_mdns,
	dns_query_mdns_timeout,
} query_type;


static void
__mark_operation_start(struct timeval	*queryStart,
		       struct timeval	*queryEnd)
{
	(void) gettimeofday(queryStart, NULL);
	*queryEnd = TIME_ZERO;

	return;
}


static void
__mark_operation_end(SCNetworkReachabilityRef	target,
		     Boolean			found,
		     query_type			query_type,
		     struct timeval		*queryStart,
		     struct timeval		*queryEnd)
{
	struct timeval			queryElapsed;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	(void) gettimeofday(queryEnd, NULL);

	if (!_sc_debug &&
	    (query_type != dns_query_mdns_timeout)) {
		return;
	}

	if (!timerisset(queryStart)) {
		return;
	}

	timersub(queryEnd, queryStart, &queryElapsed);
	switch (query_type) {

		#define	QUERY_TIME__FMT	"%ld.%6.6d"
		#define	QUERY_TIME__DIV	1

		case dns_query_async :
			SCLog(TRUE, LOG_INFO,
			      CFSTR("%sasync DNS complete%s (query time = " QUERY_TIME__FMT ")"),
			      targetPrivate->log_prefix,
			      found ? "" : ", host not found",
			      queryElapsed.tv_sec,
			      queryElapsed.tv_usec / QUERY_TIME__DIV);
			break;
		case dns_query_mdns :
			SCLog(TRUE, LOG_INFO,
			      CFSTR("%s[m]DNS query complete%s (query time = " QUERY_TIME__FMT "), " DNS_FLAGS_FORMAT),
			      targetPrivate->log_prefix,
			      found ? "" : ", host not found",
			      queryElapsed.tv_sec,
			      queryElapsed.tv_usec / QUERY_TIME__DIV,
			      DNS_FLAGS_VALUES(targetPrivate));
			break;
		case dns_query_mdns_timeout :
			SCLog(TRUE, LOG_ERR,
			      CFSTR("%s[m]DNS query timeout (query time = " QUERY_TIME__FMT "), " DNS_FLAGS_FORMAT),
			      targetPrivate->log_prefix,
			      queryElapsed.tv_sec,
			      queryElapsed.tv_usec / QUERY_TIME__DIV,
			      DNS_FLAGS_VALUES(targetPrivate));
			break;
	}

	return;
}


static __inline__ Boolean
__reach_changed(ReachabilityInfo *r1, ReachabilityInfo *r2)
{
	if (r1->flags != r2->flags) {
		// if the reachability flags changed
		return TRUE;
	}

	if (r1->if_index != r2->if_index) {
		// if the target interface changed
		return TRUE;
	}

	if ((r1->sleeping != r2->sleeping) && !r2->sleeping) {
		// if our sleep/wake status changed and if we
		// are no longer sleeping
		return TRUE;
	}

	return FALSE;
}


static __inline__ void
_reach_set(ReachabilityInfo		*dst,
	   const ReachabilityInfo	*src,
	   uint64_t			cycle,
	   unsigned int			requested_if_index,
	   const char			*requested_if_name)
{
	memcpy(dst, src, sizeof(ReachabilityInfo));
	dst->cycle = cycle;

	if (!(dst->flags & kSCNetworkReachabilityFlagsReachable) ||
		(dst->flags & kSCNetworkReachabilityFlagsConnectionRequired)) {
		// if not reachable or connection required, return the
		// requested if_index and if_name.
		dst->if_index = requested_if_index;
		if (requested_if_name != NULL) {
			strlcpy(dst->if_name, requested_if_name, sizeof(dst->if_name));
		} else {
			dst->if_name[0] = '\0';
		}
	}

	return;
}


#pragma mark -
#pragma mark SCDynamicStore info


typedef struct {
	SCDynamicStoreRef	store;
	CFStringRef		entity;
	CFDictionaryRef		dict;
	CFIndex			n;
	const void **		keys;
	const void *		keys_q[N_QUICK];
	const void **		values;
	const void *		values_q[N_QUICK];
} ReachabilityStoreInfo, *ReachabilityStoreInfoRef;


static ReachabilityStoreInfo	S_storeInfo		= { 0 };
static Boolean			S_storeInfoActive	= FALSE;


static dispatch_queue_t
_storeInfo_queue()
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create("SCNetworkReachability.storeInfo", NULL);
	});

	return q;
}


static void
ReachabilityStoreInfo_copy(ReachabilityStoreInfoRef	src,
			   ReachabilityStoreInfoRef	dst)
{
	if (src->dict != NULL) {
		dst->store = src->store;
		CFRetain(dst->store);

		dst->dict = src->dict;
		CFRetain(dst->dict);

		dst->n = src->n;
		if (dst->n > 0) {
			if (dst->n <= (CFIndex)(sizeof(dst->keys_q) / sizeof(CFTypeRef))) {
				dst->keys   = dst->keys_q;
				dst->values = dst->values_q;
			} else {
				dst->keys   = CFAllocatorAllocate(NULL, dst->n * sizeof(CFTypeRef), 0);
				dst->values = CFAllocatorAllocate(NULL, dst->n * sizeof(CFTypeRef), 0);
			}
			memcpy(dst->keys,   src->keys,   dst->n * sizeof(CFTypeRef));
			memcpy(dst->values, src->values, dst->n * sizeof(CFTypeRef));
		}
	}

	return;
}


static void
ReachabilityStoreInfo_enable(Boolean enable)
{
	dispatch_sync(_storeInfo_queue(), ^{
		S_storeInfoActive = enable;
	});

	return;
}


static void
ReachabilityStoreInfo_free(ReachabilityStoreInfoRef store_info)
{
	if ((store_info->n > 0) && (store_info->keys != store_info->keys_q)) {
		CFAllocatorDeallocate(NULL, store_info->keys);
		store_info->keys = NULL;

		CFAllocatorDeallocate(NULL, store_info->values);
		store_info->values = NULL;
	}
	store_info->n = 0;

	if (store_info->dict != NULL) {
		CFRelease(store_info->dict);
		store_info->dict = NULL;
	}

	if (store_info->store != NULL) {
		CFRelease(store_info->store);
		store_info->store = NULL;
	}

	return;
}


static void
ReachabilityStoreInfo_init(ReachabilityStoreInfoRef store_info)
{
	dispatch_sync(_storeInfo_queue(), ^{
		bzero(store_info, sizeof(ReachabilityStoreInfo));

		if (S_storeInfoActive && (S_storeInfo.dict != NULL)) {
			ReachabilityStoreInfo_copy(&S_storeInfo, store_info);
		}
	});

	return;
}


static void
ReachabilityStoreInfo_save(ReachabilityStoreInfoRef store_info)
{
	dispatch_sync(_storeInfo_queue(), ^{
		if ((store_info == NULL) ||
		    !_SC_CFEqual(store_info->dict, S_storeInfo.dict)) {
			// free any old info
			ReachabilityStoreInfo_free(&S_storeInfo);

			// save new info
			if (S_storeInfoActive &&
			    (store_info != NULL) &&
			    (store_info->dict != NULL)) {
				ReachabilityStoreInfo_copy(store_info, &S_storeInfo);
			}
		}
	});

	return;
}


static void
ReachabilityStoreInfo_keys(CFMutableArrayRef *fill_keys, CFMutableArrayRef *fill_patterns)
{
	CFStringRef		key;
	CFMutableArrayRef	keys;
	CFStringRef		pattern;
	CFMutableArrayRef	patterns;

	keys = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	patterns = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	// get info for IPv4 services
	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							 kSCDynamicStoreDomainState,
							 kSCEntNetIPv4);
	CFArrayAppendValue(keys, key);
	CFRelease(key);
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainSetup,
							      kSCCompAnyRegex,
							      kSCEntNetIPv4);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainState,
							      kSCCompAnyRegex,
							      kSCEntNetIPv4);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);

	// get info for IPv6 services
	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							 kSCDynamicStoreDomainState,
							 kSCEntNetIPv6);
	CFArrayAppendValue(keys, key);
	CFRelease(key);
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainSetup,
							      kSCCompAnyRegex,
							      kSCEntNetIPv6);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainState,
							      kSCCompAnyRegex,
							      kSCEntNetIPv6);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);

	// get info for PPP services
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainSetup,
							      kSCCompAnyRegex,
							      kSCEntNetPPP);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainState,
							      kSCCompAnyRegex,
							      kSCEntNetPPP);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);

	// get info for VPN services
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainSetup,
							      kSCCompAnyRegex,
							      kSCEntNetVPN);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainState,
							      kSCCompAnyRegex,
							      kSCEntNetVPN);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);

	// get info for IPSec services
//	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
//							      kSCDynamicStoreDomainSetup,
//							      kSCCompAnyRegex,
//							      kSCEntNetIPSec);
//	CFArrayAppendValue(patterns, pattern);
//	CFRelease(pattern);
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainState,
							      kSCCompAnyRegex,
							      kSCEntNetIPSec);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);

	// get info to identify "available" services
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainSetup,
							      kSCCompAnyRegex,
							      kSCEntNetInterface);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);


	*fill_keys     = keys;
	*fill_patterns = patterns;
	return;
}


static Boolean
ReachabilityStoreInfo_fill(ReachabilityStoreInfoRef store_info)
{
	CFMutableArrayRef	keys;
	CFMutableArrayRef	patterns;

	// get the SCDynamicStore info
	ReachabilityStoreInfo_keys(&keys, &patterns);
	store_info->dict = SCDynamicStoreCopyMultiple(store_info->store, keys, patterns);
	CFRelease(keys);
	CFRelease(patterns);
	if (store_info->dict == NULL) {
		return FALSE;
	}

	// and extract the keys/values for post-processing
	store_info->n = CFDictionaryGetCount(store_info->dict);
	if (store_info->n > 0) {
		if (store_info->n <= (CFIndex)(sizeof(store_info->keys_q) / sizeof(CFTypeRef))) {
			store_info->keys   = store_info->keys_q;
			store_info->values = store_info->values_q;
		} else {
			store_info->keys   = CFAllocatorAllocate(NULL, store_info->n * sizeof(CFTypeRef), 0);
			store_info->values = CFAllocatorAllocate(NULL, store_info->n * sizeof(CFTypeRef), 0);
		}
		CFDictionaryGetKeysAndValues(store_info->dict,
					     store_info->keys,
					     store_info->values);
	}

	return TRUE;
}


static Boolean
ReachabilityStoreInfo_update(ReachabilityStoreInfoRef	store_info,
			     SCDynamicStoreRef		*storeP,
			     sa_family_t		sa_family)
{
	__block Boolean		ok	= TRUE;

	switch (sa_family) {
		case AF_UNSPEC :
			store_info->entity = NULL;
			break;
		case AF_INET :
			store_info->entity = kSCEntNetIPv4;
			break;
		case AF_INET6 :
			store_info->entity = kSCEntNetIPv6;
			break;
		default :
			return FALSE;
	}

	if (store_info->dict != NULL) {
		// if info already available
		return TRUE;
	}

	dispatch_sync(_storeInfo_queue(), ^{
		if (S_storeInfoActive && (S_storeInfo.dict != NULL)) {
			// free any info
			ReachabilityStoreInfo_free(store_info);

			// copy the shared/available info
			ReachabilityStoreInfo_copy(&S_storeInfo, store_info);
		}

		if (store_info->store == NULL) {
			store_info->store = (storeP != NULL) ? *storeP : NULL;
			if (store_info->store != NULL) {
				// keep a reference to the passed in SCDynamicStore
				CFRetain(store_info->store);
			} else {
				store_info->store = SCDynamicStoreCreate(NULL, CFSTR("SCNetworkReachability"), NULL, NULL);
				if (store_info->store == NULL) {
					SCLog(TRUE, LOG_ERR, CFSTR("ReachabilityStoreInfo_update SCDynamicStoreCreate() failed"));
					return;
				}

				if (storeP != NULL) {
					// and pass back a reference
					*storeP = store_info->store;
					CFRetain(*storeP);
				}
			}
		}

		if (sa_family == AF_UNSPEC) {
			// if the address family was not specified than
			// all we wanted, for now, was to establish the
			// SCDynamicStore session
			return;
		}

		if (store_info->dict != NULL) {
			// or we have picked up the shared info
			return;
		}

		ok = ReachabilityStoreInfo_fill(store_info);
		if (!ok) {
			return;
		}

		if (!_SC_CFEqual(store_info->dict, S_storeInfo.dict)) {
			// free any old info
			ReachabilityStoreInfo_free(&S_storeInfo);

			// save new info
			if (S_storeInfoActive &&
			    (store_info->dict != NULL)) {
				ReachabilityStoreInfo_copy(store_info, &S_storeInfo);
			}
		}
	});

	return ok;
}


#pragma mark -
#pragma mark Reachability engine


#define ROUNDUP(a, size) \
	(((a) & ((size)-1)) ? (1 + ((a) | ((size)-1))) : (a))

#define NEXT_SA(ap) (ap) = (struct sockaddr *) \
	((caddr_t)(ap) + ((ap)->sa_len ? ROUNDUP((ap)->sa_len,\
						 sizeof(uint32_t)) :\
						 sizeof(uint32_t)))

static void
get_rtaddrs(int addrs, struct sockaddr *sa, struct sockaddr **rti_info)
{
	int             i;

	for (i = 0; i < RTAX_MAX; i++) {
		if (addrs & (1 << i)) {
			rti_info[i] = sa;
			NEXT_SA(sa);
		} else
			rti_info[i] = NULL;
	}
}


#define BUFLEN (sizeof(struct rt_msghdr) + 512)	/* 8 * sizeof(struct sockaddr_in6) = 192 */


typedef struct {
	union {
		char			bytes[BUFLEN];
		struct rt_msghdr	rtm;
	} buf;
	int			error;
	struct sockaddr         *rti_info[RTAX_MAX];
	struct rt_msghdr	*rtm;
	struct sockaddr_dl	*sdl;
} route_info, *route_info_p;


/*
 * route_get()
 *	returns	zero if route exists and data returned, EHOSTUNREACH
 *	if no route, or errno for any other error.
 */
static int
route_get(const struct sockaddr	*address,
	  unsigned int		if_index,
	  route_info		*info)
{
	int			n;
	int			opt;
	pid_t			pid		= getpid();
	int			rsock;
	struct sockaddr         *sa;
	int32_t			seq		= OSAtomicIncrement32Barrier(&rtm_seq);
#ifndef	RTM_GET_SILENT
#warning Note: Using RTM_GET (and not RTM_GET_SILENT)
	static pthread_mutex_t	lock		= PTHREAD_MUTEX_INITIALIZER;
	int			sosize		= 48 * 1024;
#endif

	bzero(info, sizeof(*info));

	info->rtm = &info->buf.rtm;
	info->rtm->rtm_msglen  = sizeof(struct rt_msghdr);
	info->rtm->rtm_version = RTM_VERSION;
#ifdef	RTM_GET_SILENT
	info->rtm->rtm_type    = RTM_GET_SILENT;
#else
	info->rtm->rtm_type    = RTM_GET;
#endif
	info->rtm->rtm_flags   = RTF_STATIC|RTF_UP|RTF_HOST|RTF_GATEWAY;
	info->rtm->rtm_addrs   = RTA_DST|RTA_IFP; /* Both destination and device */
	info->rtm->rtm_pid     = pid;
	info->rtm->rtm_seq     = seq;

	if (if_index != 0) {
		info->rtm->rtm_flags |= RTF_IFSCOPE;
		info->rtm->rtm_index = if_index;
	}

	switch (address->sa_family) {
		case AF_INET6: {
			struct sockaddr_in6	*sin6;

			/* ALIGN: caller ensures that the address is aligned */
			sin6 = (struct sockaddr_in6 *)(void *)address;
			if ((IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr) ||
			     IN6_IS_ADDR_MC_LINKLOCAL(&sin6->sin6_addr)) &&
			    (sin6->sin6_scope_id != 0)) {
				sin6->sin6_addr.s6_addr16[1] = htons(sin6->sin6_scope_id);
				sin6->sin6_scope_id = 0;
			}
			break;
		}
	}

	sa  = (struct sockaddr *) (info->rtm + 1);
	bcopy(address, sa, address->sa_len);
	n = ROUNDUP(sa->sa_len, sizeof(uint32_t));
	info->rtm->rtm_msglen += n;

	info->sdl = (struct sockaddr_dl *) ((void *)sa + n);
	info->sdl->sdl_family = AF_LINK;
	info->sdl->sdl_len = sizeof (struct sockaddr_dl);
	n = ROUNDUP(info->sdl->sdl_len, sizeof(uint32_t));
	info->rtm->rtm_msglen += n;

#ifndef	RTM_GET_SILENT
	pthread_mutex_lock(&lock);
#endif
	rsock = socket(PF_ROUTE, SOCK_RAW, PF_ROUTE);
	if (rsock == -1) {
		int	error	= errno;

#ifndef	RTM_GET_SILENT
		pthread_mutex_unlock(&lock);
#endif
		SCLog(TRUE, LOG_ERR, CFSTR("socket(PF_ROUTE) failed: %s"), strerror(error));
		return error;
	}
	opt = 1;
	if (ioctl(rsock, FIONBIO, &opt) < 0) {
		int	error	= errno;

		(void)close(rsock);
#ifndef	RTM_GET_SILENT
		pthread_mutex_unlock(&lock);
#endif
		SCLog(TRUE, LOG_ERR, CFSTR("ioctl(FIONBIO) failed: %s"), strerror(error));
		return error;
	}

#ifndef	RTM_GET_SILENT
	if (setsockopt(rsock, SOL_SOCKET, SO_RCVBUF, &sosize, sizeof(sosize)) == -1) {
		int	error	= errno;

		(void)close(rsock);
		pthread_mutex_unlock(&lock);
		SCLog(TRUE, LOG_ERR, CFSTR("setsockopt(SO_RCVBUF) failed: %s"), strerror(error));
		return error;
	}
#endif

	if (write(rsock, &info->buf, info->rtm->rtm_msglen) == -1) {
		int	error	= errno;

		(void)close(rsock);
#ifndef	RTM_GET_SILENT
		pthread_mutex_unlock(&lock);
#endif
		if (error != ESRCH) {
			SCLog(TRUE, LOG_ERR, CFSTR("write() failed: %s"), strerror(error));
			return error;
		}
		return EHOSTUNREACH;
	}

	/*
	 * Type, seq, pid identify our response.
	 * Routing sockets are broadcasters on input.
	 */
	while (TRUE) {
		ssize_t		n;

		n = read(rsock, &info->buf, sizeof(info->buf));
		if (n == -1) {
			int	error	= errno;

			if (error == EINTR) {
				continue;
			}
			(void)close(rsock);
#ifndef	RTM_GET_SILENT
			pthread_mutex_unlock(&lock);
#endif
			SCLog(TRUE, LOG_ERR,
			      CFSTR("SCNetworkReachability: routing socket"
				    " read() failed: %s"), strerror(error));
			return error;
		}
		if ((info->rtm->rtm_type == RTM_GET) 	&&
		    (info->rtm->rtm_seq == seq) 	&&
		    (info->rtm->rtm_pid == pid)) {
		    break;
		}
	}

	(void)close(rsock);
#ifndef	RTM_GET_SILENT
	pthread_mutex_unlock(&lock);
#endif

	get_rtaddrs(info->rtm->rtm_addrs, sa, info->rti_info);

//#define LOG_RTADDRS
#ifdef	LOG_RTADDRS
	{
		int	i;

		SCLog(_sc_debug, LOG_DEBUG, CFSTR("rtm_flags = 0x%8.8x"), info->rtm->rtm_flags);

		if ((info->rti_info[RTAX_NETMASK] != NULL) && (info->rti_info[RTAX_DST] != NULL)) {
			info->rti_info[RTAX_NETMASK]->sa_family = info->rti_info[RTAX_DST]->sa_family;
		}

		for (i = 0; i < RTAX_MAX; i++) {
			if (info->rti_info[i] != NULL) {
				char	addr[128];

				_SC_sockaddr_to_string(info->rti_info[i], addr, sizeof(addr));
				SCLog(_sc_debug, LOG_DEBUG, CFSTR("%d: %s"), i, addr);
			}
		}
	}
#endif	// LOG_RTADDRS

	if ((info->rti_info[RTAX_IFP] == NULL) ||
	    (info->rti_info[RTAX_IFP]->sa_family != AF_LINK)) {
		/* no interface info */
		SCLog(TRUE, LOG_DEBUG, CFSTR("route_get() no interface info"));
		return EINVAL;
	}

	/* ALIGN: accessors are retrieving byte values, cast ok. */
	info->sdl = (struct sockaddr_dl *)(void *) info->rti_info[RTAX_IFP];
	if ((info->sdl->sdl_nlen == 0) || (info->sdl->sdl_nlen > IFNAMSIZ)) {
		/* no interface name */
		return EHOSTUNREACH;
	}

	return 0;
}


static void
log_address(const char			*str,
	    const struct sockaddr	*sa,
	    unsigned int		if_index,
	    const char			*log_prefix)
{
	char	addr[128];
	char	if_name[IFNAMSIZ + 1];

	_SC_sockaddr_to_string(sa, addr, sizeof(addr));

	if ((if_index != 0) &&
	    (if_indextoname(if_index, &if_name[1]) != NULL)) {
		if_name[0] = '%';
	} else {
		if_name[0] = '\0';
	}

	SCLog(TRUE, LOG_INFO, CFSTR("%s%s(%s%s)"),
	      log_prefix,
	      str,
	      addr,
	      if_name);

	return;
}


static int
checkAddress_route(const struct sockaddr	*address,
		   unsigned int			if_index,
		   char				*if_name,
		   struct ifreq			*ifr,
		   ReachabilityInfo		*reach_info,
		   route_info			*info,
		   int				*sc_status,
		   const char			*log_prefix)
{
	int			isock		= -1;
	int			ret		= 0;
	char			*statusMessage	= NULL;
	struct sockaddr_in	v4mapped;

	switch (address->sa_family) {
		case AF_INET :
		case AF_INET6 :
			if (_sc_debug) {
				log_address("checkAddress", address, if_index, log_prefix);
			}
			break;
		default :
			/*
			 * if no code for this address family (yet)
			 */
			SCLog(TRUE, LOG_INFO,
			      CFSTR("checkAddress(): unexpected address family %d"),
			      address->sa_family);
			*sc_status = kSCStatusInvalidArgument;
			ret = EPERM;
			goto done;
	}

	if (address->sa_family == AF_INET6) {
		/* ALIGN: sin6_addr accessed aligned, cast ok. */
		struct sockaddr_in6	*sin6	= (struct sockaddr_in6 *)(void *)address;

		if (IN6_IS_ADDR_V4MAPPED(&sin6->sin6_addr)) {
			bzero(&v4mapped, sizeof(v4mapped));
			v4mapped.sin_len         = sizeof(v4mapped);
			v4mapped.sin_family      = AF_INET;
			v4mapped.sin_port        = sin6->sin6_port;
			v4mapped.sin_addr.s_addr = sin6->sin6_addr.__u6_addr.__u6_addr32[3];
			address = (struct sockaddr *)&v4mapped;
		}
	}

	ret = route_get(address, if_index, info);
	switch (ret) {
		case 0 :
			break;
		case EHOSTUNREACH :
			// if no route
			goto done;
		default :
			// if error
			*sc_status = ret;
			goto done;
	}

	/* get the interface flags */

	isock = socket(AF_INET, SOCK_DGRAM, 0);
	if (isock == -1) {
		ret = errno;
		SCLog(TRUE, LOG_ERR, CFSTR("socket() failed: %s"), strerror(errno));
		goto done;
	}

	bzero(ifr, sizeof(*ifr));
	bcopy(info->sdl->sdl_data, ifr->ifr_name, info->sdl->sdl_nlen);

	if (ioctl(isock, SIOCGIFFLAGS, (char *)ifr) == -1) {
		ret = errno;
		SCLog(TRUE, LOG_ERR, CFSTR("ioctl(SIOCGIFFLAGS) failed: %s"), strerror(errno));
		goto done;
	}

	if (!(ifr->ifr_flags & IFF_UP)) {
		ret = EHOSTUNREACH;
		goto done;
	}

	statusMessage = "isReachable";
	reach_info->flags |= kSCNetworkReachabilityFlagsReachable;

	if (info->rtm->rtm_flags & RTF_LOCAL) {
		statusMessage = "isReachable (is a local address)";
		reach_info->flags |= kSCNetworkReachabilityFlagsIsLocalAddress;
	} else if (ifr->ifr_flags & IFF_LOOPBACK) {
		statusMessage = "isReachable (is loopback network)";
		reach_info->flags |= kSCNetworkReachabilityFlagsIsLocalAddress;
	} else if ((info->rti_info[RTAX_IFA] != NULL) &&
		   (info->rti_info[RTAX_IFA]->sa_family != AF_LINK)) {
		void	*addr1	= (void *)address;
		void	*addr2	= (void *)info->rti_info[RTAX_IFA];
		size_t	len	= address->sa_len;

		if ((address->sa_family != info->rti_info[RTAX_IFA]->sa_family) &&
		    (address->sa_len    != info->rti_info[RTAX_IFA]->sa_len)) {
			SCLog(TRUE, LOG_NOTICE,
			      CFSTR("address family/length mismatch: %d/%d != %d/%d"),
			      address->sa_family,
			      address->sa_len,
			      info->rti_info[RTAX_IFA]->sa_family,
			      info->rti_info[RTAX_IFA]->sa_len);
			goto done;
		}

		switch (address->sa_family) {
			case AF_INET :
				/* ALIGN: cast ok, because only bcmp is used. */
				addr1 = &((struct sockaddr_in *)(void *)address)->sin_addr;
				addr2 = &((struct sockaddr_in *)(void *)info->rti_info[RTAX_IFA])->sin_addr;
				len = sizeof(struct in_addr);

				/*
				 * check if 0.0.0.0
				 */
				/* ALIGN: sin_addr should be aligned, cast ok. */
				if (((struct sockaddr_in *)(void *)address)->sin_addr.s_addr == 0) {
					statusMessage = "isReachable (this host)";
					reach_info->flags |= kSCNetworkReachabilityFlagsIsLocalAddress;
				}
				break;
			case AF_INET6 :
				/* ALIGN: cast ok, because only bcmp is used. */
				addr1 = &((struct sockaddr_in6 *)(void *)address)->sin6_addr;
				addr2 = &((struct sockaddr_in6 *)(void *)info->rti_info[RTAX_IFA])->sin6_addr;
				len = sizeof(struct in6_addr);
				break;
			default :
				break;
		}

		if (bcmp(addr1, addr2, len) == 0) {
			statusMessage = "isReachable (is interface address)";
			reach_info->flags |= kSCNetworkReachabilityFlagsIsLocalAddress;
		}
	}

	if (!(info->rtm->rtm_flags & RTF_GATEWAY) &&
	    (info->rti_info[RTAX_GATEWAY] != NULL) &&
	    (info->rti_info[RTAX_GATEWAY]->sa_family == AF_LINK) &&
	    !(ifr->ifr_flags & IFF_POINTOPOINT)) {
		reach_info->flags |= kSCNetworkReachabilityFlagsIsDirect;
	}

	bzero(if_name, IFNAMSIZ);
	bcopy(info->sdl->sdl_data,
	      if_name,
	      (info->sdl->sdl_nlen <= IFNAMSIZ) ? info->sdl->sdl_nlen : IFNAMSIZ);

	strlcpy(reach_info->if_name, if_name, sizeof(reach_info->if_name));
	reach_info->if_index = info->sdl->sdl_index;

	if (_sc_debug) {
		SCLog(TRUE, LOG_INFO, CFSTR("%s  status    = %s"), log_prefix, statusMessage);
		SCLog(TRUE, LOG_INFO, CFSTR("%s  device    = %s (%hu)"), log_prefix, if_name, info->sdl->sdl_index);
		SCLog(TRUE, LOG_INFO, CFSTR("%s  sdl_type  = 0x%x"), log_prefix, info->sdl->sdl_type);
		SCLog(TRUE, LOG_INFO, CFSTR("%s  ifr_flags = 0x%04hx"), log_prefix, ifr->ifr_flags);
		SCLog(TRUE, LOG_INFO, CFSTR("%s  rtm_flags = 0x%08x"), log_prefix, info->rtm->rtm_flags);
	}

    done :
	if (isock != -1) (void)close(isock);
	return ret;
}


static Boolean
checkAddress(ReachabilityStoreInfoRef	store_info,
	     const struct sockaddr	*address,
	     unsigned int		if_index,
	     ReachabilityInfo		*reach_info,
	     const char			*log_prefix)
{
	route_info		info;
	struct ifreq		ifr;
	char			if_name[IFNAMSIZ];
	nwi_ifstate_t		ifstate;
	nwi_state_t		nwi_state;
	int			ret;
	int			sc_status	= kSCStatusReachabilityUnknown;

	_reach_set(reach_info, &NOT_REACHABLE, reach_info->cycle, if_index, NULL);

	nwi_state = nwi_state_copy();

	if (address != NULL) {
		ret = checkAddress_route(address,
					if_index,
					if_name,
					&ifr,
					reach_info,
					&info,
					&sc_status,
					log_prefix);
	} else {
		/* special case: check only for available paths off the system */
		ret = EHOSTUNREACH;
	}

	if (ret == 0) {
		const struct sockaddr	*vpn_server_address;

		sc_status = kSCStatusOK;

		ifstate = nwi_state_get_ifstate(nwi_state, if_name);
		if (ifstate == NULL) {
			goto done;
		}

		reach_info->flags |= nwi_ifstate_get_reachability_flags(ifstate);


		vpn_server_address = nwi_ifstate_get_vpn_server(ifstate);
		if (vpn_server_address != NULL) {
			char		dst_if_name[IFNAMSIZ];
			route_info	dst_info;

			ret = route_get(vpn_server_address, 0, &dst_info);
			if (ret != 0) {
				goto done;
			}

			bzero(&dst_if_name, sizeof(dst_if_name));
			bcopy(dst_info.sdl->sdl_data,
			      dst_if_name,
			      (dst_info.sdl->sdl_nlen <= IFNAMSIZ) ? dst_info.sdl->sdl_nlen : IFNAMSIZ);
			if (bcmp(if_name, dst_if_name, sizeof(if_name)) != 0) {
				nwi_ifstate_t ifstate;

				ifstate = nwi_state_get_ifstate(nwi_state, dst_if_name);
				if (ifstate != NULL) {
					reach_info->flags |= nwi_ifstate_get_reachability_flags(ifstate);
				}
			}
		}
	} else if (ret == EHOSTUNREACH) {
		if (if_index == 0) {
			int	af;

			// if not "scoped" request
			af = (address != NULL) ? address->sa_family : AF_UNSPEC;
			reach_info->flags |= nwi_state_get_reachability_flags(nwi_state, af);
			sc_status = kSCStatusOK;
		} else {
			// if "scoped" request
			sc_status = kSCStatusNoKey;
		}
	}

    done:

	if (reach_info->flags == 0) {
		SCLog(_sc_debug, LOG_INFO, CFSTR("%s  cannot be reached"), log_prefix);
	}

	if (nwi_state != NULL) {
		nwi_state_release(nwi_state);
	}

	if ((sc_status != kSCStatusOK) && (sc_status != kSCStatusNoKey)) {
		_SCErrorSet(sc_status);
		return FALSE;
	}

	return TRUE;
}


#pragma mark -
#pragma mark SCNetworkReachability APIs


static __inline__ CFTypeRef
isA_SCNetworkReachability(CFTypeRef obj)
{
	return (isA_CFType(obj, SCNetworkReachabilityGetTypeID()));
}


static Boolean
addr_to_PTR_name(const struct sockaddr *sa, char *name, size_t name_len)
{
	int	n;

	switch (sa->sa_family) {
		case AF_INET : {
			union {
				in_addr_t	s_addr;
				unsigned char	b[4];
			} rev;
			/* ALIGN: assuming sa is aligned, then cast ok. */
			struct sockaddr_in	*sin	= (struct sockaddr_in *)(void *)sa;

			/*
			 * build "PTR" query name
			 *   NNN.NNN.NNN.NNN.in-addr.arpa.
			 */
			rev.s_addr = sin->sin_addr.s_addr;
			n = snprintf(name, name_len, "%u.%u.%u.%u.in-addr.arpa.",
				     rev.b[3],
				     rev.b[2],
				     rev.b[1],
				     rev.b[0]);
			if ((n == -1) || (n >= name_len)) {
				return FALSE;
			}

			break;
		}

		case AF_INET6 : {
			int			i;
			int			s	= 0;
			/* ALIGN: assume sa is aligned, cast ok. */
			struct sockaddr_in6	*sin6	= (struct sockaddr_in6 *)(void *)sa;
			size_t			x	= name_len;

			/*
			 * build IPv6 "nibble" PTR query name (RFC 1886, RFC 3152)
			 *   N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.N.ip6.arpa.
			 */
			for (i = sizeof(sin6->sin6_addr) - 1; i >= 0; i--) {
				n = snprintf(&name[s], x, "%x.%x.",
					     ( sin6->sin6_addr.s6_addr[i]       & 0xf),
					     ((sin6->sin6_addr.s6_addr[i] >> 4) & 0xf));
				if ((n == -1) || (n >= x)) {
					return FALSE;
				}

				s += n;
				x -= n;
			}

			n = snprintf(&name[s], x, "ip6.arpa.");
			if ((n == -1) || (n >= x)) {
				return FALSE;
			}

			break;
		}

		default :
			return FALSE;
	}

	return TRUE;
}


CFStringRef
_SCNetworkReachabilityCopyTargetDescription(SCNetworkReachabilityRef target)
{
	CFAllocatorRef			allocator	= CFGetAllocator(target);
	CFMutableStringRef		str;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	str = CFStringCreateMutable(allocator, 0);
	switch (targetPrivate->type) {
		case reachabilityTypeAddress :
		case reachabilityTypeAddressPair : {
			char		buf[64];

			if (targetPrivate->localAddress != NULL) {
				_SC_sockaddr_to_string(targetPrivate->localAddress, buf, sizeof(buf));
				CFStringAppendFormat(str, NULL, CFSTR("local address = %s"),
						     buf);
			}

			if (targetPrivate->remoteAddress != NULL) {
				_SC_sockaddr_to_string(targetPrivate->remoteAddress, buf, sizeof(buf));
				CFStringAppendFormat(str, NULL, CFSTR("%s%saddress = %s"),
						     targetPrivate->localAddress ? ", " : "",
						     (targetPrivate->type == reachabilityTypeAddressPair) ? "remote " : "",
						     buf);
			}
			break;
		}
		case reachabilityTypeName : {
			CFStringAppendFormat(str, NULL, CFSTR("name = %s"), targetPrivate->name);
			break;
		}
		case reachabilityTypePTR : {
			CFStringAppendFormat(str, NULL, CFSTR("ptr = %s"), targetPrivate->name);
			break;
		}
	}

	return str;
}


CFStringRef
_SCNetworkReachabilityCopyTargetFlags(SCNetworkReachabilityRef target)
{
	CFAllocatorRef			allocator	= CFGetAllocator(target);
	CFStringRef			str;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	str = CFStringCreateWithFormat(allocator,
				       NULL,
				       CFSTR("flags = 0x%08x, if_index = %u%s"),
				       targetPrivate->info.flags,
				       targetPrivate->info.if_index,
				       targetPrivate->info.sleeping ? ", z" : "");
	return str;
}


static CFStringRef
__SCNetworkReachabilityCopyDescription(CFTypeRef cf)
{
	CFAllocatorRef			allocator	= CFGetAllocator(cf);
	CFMutableStringRef		result;
	CFStringRef			str;
	SCNetworkReachabilityRef	target		= (SCNetworkReachabilityRef)cf;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	result = CFStringCreateMutable(allocator, 0);
	CFStringAppendFormat(result, NULL, CFSTR("<SCNetworkReachability %p [%p]> {"), cf, allocator);

	// add target description
	str = _SCNetworkReachabilityCopyTargetDescription(target);
	CFStringAppend(result, str);
	CFRelease(str);

	// add additional "name" info
	if (isReachabilityTypeName(targetPrivate->type)) {
		if (targetPrivate->dnsActive) {
			CFStringAppendFormat(result, NULL, CFSTR(" (DNS query active)"));
		} else if (targetPrivate->serverActive &&
			   (targetPrivate->info.flags & kSCNetworkReachabilityFlagsFirstResolvePending)) {
			CFStringAppendFormat(result, NULL, CFSTR(" (server query active)"));
		} else if ((targetPrivate->resolvedAddresses != NULL) || (targetPrivate->resolvedError != NETDB_SUCCESS)) {
			if (targetPrivate->resolvedAddresses != NULL) {
				if (isA_CFArray(targetPrivate->resolvedAddresses)) {
					CFIndex	i;
					CFIndex	n	= CFArrayGetCount(targetPrivate->resolvedAddresses);

					CFStringAppendFormat(result, NULL, CFSTR(" ("));
					for (i = 0; i < n; i++) {
						CFDataRef	address;

						CFStringAppendFormat(result, NULL, CFSTR("%s"),
								     i > 0 ? ", " : "");

						address	= CFArrayGetValueAtIndex(targetPrivate->resolvedAddresses, i);
						if (isA_CFData(address)) {
							char		buf[64];
							struct sockaddr	*sa;

							sa      = (struct sockaddr *)CFDataGetBytePtr(address);
							_SC_sockaddr_to_string(sa, buf, sizeof(buf));
							CFStringAppendFormat(result, NULL, CFSTR("%s"), buf);
						} else {
							CFStringAppendFormat(result, NULL, CFSTR("%@"), address);
						}
					}
					CFStringAppendFormat(result, NULL, CFSTR(")"));
				} else if (CFEqual(targetPrivate->resolvedAddresses, kCFNull)) {
					CFStringAppendFormat(result, NULL, CFSTR(" (%s)"),
							     gai_strerror(targetPrivate->resolvedError));
				} else {
					CFStringAppendFormat(result, NULL, CFSTR(" (no addresses)"));
				}
			} else {
				CFStringAppendFormat(result, NULL, CFSTR(" (%s)"),
						     gai_strerror(targetPrivate->resolvedError));
			}
		}
		if (targetPrivate->dnsFlags != 0) {
			CFStringAppendFormat(result, NULL, CFSTR(", " DNS_FLAGS_FORMAT),
					     DNS_FLAGS_VALUES(targetPrivate));
		}
	}

	if (targetPrivate->onDemandBypass) {
		CFStringAppendFormat(result, NULL, CFSTR(", !ondemand"));
	}


	if (targetPrivate->resolverBypass) {
		CFStringAppendFormat(result, NULL, CFSTR(", !resolve"));
	}


	// add flags
	if (targetPrivate->scheduled) {
		str = _SCNetworkReachabilityCopyTargetFlags(target);
		CFStringAppendFormat(result, NULL, CFSTR(", %@"), str);
		CFRelease(str);
	}

	CFStringAppendFormat(result, NULL, CFSTR("}"));

	return result;
}


static void
__SCNetworkReachabilityDeallocate(CFTypeRef cf)
{
	SCNetworkReachabilityRef	target		= (SCNetworkReachabilityRef)cf;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	SCLog((_sc_debug && (_sc_log > 0)), LOG_INFO, CFSTR("%srelease"),
	      targetPrivate->log_prefix);

	/* disconnect from the reachability server */

	if (targetPrivate->serverActive) {
		__SCNetworkReachabilityServer_targetRemove(target);
	}

	/* release resources */

	pthread_mutex_destroy(&targetPrivate->lock);

	if (targetPrivate->name != NULL)
		CFAllocatorDeallocate(NULL, (void *)targetPrivate->name);

	if (targetPrivate->resolvedAddresses != NULL)
		CFRelease(targetPrivate->resolvedAddresses);

	if (targetPrivate->localAddress != NULL) {
		if (targetPrivate->localAddress == targetPrivate->remoteAddress) {
			targetPrivate->remoteAddress = NULL;
		}
		CFAllocatorDeallocate(NULL, (void *)targetPrivate->localAddress);
	}

	if (targetPrivate->remoteAddress != NULL)
		CFAllocatorDeallocate(NULL, (void *)targetPrivate->remoteAddress);

	if (targetPrivate->rlsContext.release != NULL) {
		(*targetPrivate->rlsContext.release)(targetPrivate->rlsContext.info);
	}

	if (targetPrivate->onDemandName != NULL) {
		CFRelease(targetPrivate->onDemandName);
	}

	if (targetPrivate->onDemandRemoteAddress != NULL) {
		CFRelease(targetPrivate->onDemandRemoteAddress);
	}

	if (targetPrivate->onDemandServer != NULL) {
		CFRelease(targetPrivate->onDemandServer);
	}

	if (targetPrivate->onDemandServiceID != NULL) {
		CFRelease(targetPrivate->onDemandServiceID);
	}

	if (targetPrivate->serverDigest != NULL) {
		CFRelease(targetPrivate->serverDigest);
	}

	if (targetPrivate->serverGroup != NULL) {
		dispatch_release(targetPrivate->serverGroup);
	}

	if (targetPrivate->serverQueue != NULL) {
		dispatch_release(targetPrivate->serverQueue);
	}

	if (targetPrivate->serverWatchers != NULL) {
		CFRelease(targetPrivate->serverWatchers);
	}

	if (targetPrivate->nePolicyResult) {
		free(targetPrivate->nePolicyResult);
	}

	return;
}


static void
__SCNetworkReachabilityInitialize(void)
{
	__kSCNetworkReachabilityTypeID = _CFRuntimeRegisterClass(&__SCNetworkReachabilityClass);

	// provide a way to enable SCNetworkReachability logging without
	// having to set _sc_debug=1.
	if ((getenv("REACH_LOGGING") != NULL) ||
	    (CFPreferencesGetAppBooleanValue(CFSTR("com.apple.SCNetworkReachability.debug"),
					     kCFPreferencesCurrentApplication,
					     NULL))) {
		_sc_debug = TRUE;
	}

	// set per-process "bypass" of the SCNetworkReachability server
	if (getenv("REACH_SERVER_BYPASS") != NULL) {
		D_serverBypass = TRUE;
	}


	pthread_mutexattr_init(&lock_attr);
	pthread_mutexattr_settype(&lock_attr, PTHREAD_MUTEX_ERRORCHECK);

	return;
}


__private_extern__
dispatch_queue_t
__SCNetworkReachability_concurrent_queue()
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create("SCNetworkReachability.concurrent",
					  DISPATCH_QUEUE_CONCURRENT);
	});

	return q;
}


/*
 * __SCNetworkReachabilityUpdateConcurrent
 *
 * Calls reachUpdate()
 * - caller must be holding a reference to the target
 * - caller must *not* be holding the target lock
 * - caller must be running on the __SCNetworkReachability_concurrent_queue()
 */
__private_extern__
void
__SCNetworkReachabilityUpdateConcurrent(SCNetworkReachabilityRef target)
{
	Boolean				changed;
	unsigned int			n;
	dispatch_queue_t		queue;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	changed = reachUpdate((void *)target);
	if (!changed) {
		return;
	}

	n = _SC_ATOMIC_INC(&targetPrivate->pending);
	if (n > 0) {
		// if we already have a notification pending
		return;
	}

	MUTEX_LOCK(&targetPrivate->lock);

	queue = targetPrivate->dispatchQueue;
	if (queue != NULL) {
		dispatch_group_t	group;

		dispatch_retain(queue);

		group = targetPrivate->dispatchGroup;
		dispatch_group_enter(group);

		MUTEX_UNLOCK(&targetPrivate->lock);

		dispatch_sync(queue, ^{
			reachPerform((void *)target);
			dispatch_group_leave(group);
		});

		dispatch_release(queue);
	} else {
		if (targetPrivate->rls != NULL) {
			CFRunLoopSourceSignal(targetPrivate->rls);
			_SC_signalRunLoop(target, targetPrivate->rls, targetPrivate->rlList);
		}

		MUTEX_UNLOCK(&targetPrivate->lock);
	}

	return;
}


/*
 * __SCNetworkReachabilityUpdate
 *
 * Calls reachUpdate() [indirectly]
 * - caller can be holding the target lock
 * - caller can be running on any dispatch queue
 */
__private_extern__
void
__SCNetworkReachabilityUpdate(SCNetworkReachabilityRef target)
{
	CFRetain(target);
	dispatch_async(__SCNetworkReachability_concurrent_queue(), ^{
		__SCNetworkReachabilityUpdateConcurrent(target);
		CFRelease(target);
	});

	return;
}


static SCNetworkReachabilityPrivateRef
__SCNetworkReachabilityCreatePrivate(CFAllocatorRef	allocator)
{
	SCNetworkReachabilityPrivateRef		targetPrivate;
	uint32_t				size;

	/* initialize runtime */
	pthread_once(&initialized, __SCNetworkReachabilityInitialize);

	/* allocate target */
	size          = sizeof(SCNetworkReachabilityPrivate) - sizeof(CFRuntimeBase);
	targetPrivate = (SCNetworkReachabilityPrivateRef)_CFRuntimeCreateInstance(allocator,
										  __kSCNetworkReachabilityTypeID,
										  size,
										  NULL);
	if (targetPrivate == NULL) {
		return NULL;
	}

	bzero((void *)targetPrivate + sizeof(CFRuntimeBase), size);

	MUTEX_INIT(&targetPrivate->lock);

	targetPrivate->cycle				= 1;
	targetPrivate->last_notify			= NOT_REPORTED;
	targetPrivate->serverBypass			= D_serverBypass;



	targetPrivate->log_prefix[0] = '\0';
	if (_sc_log > 0) {
		snprintf(targetPrivate->log_prefix,
			 sizeof(targetPrivate->log_prefix),
			 "[%p] ",
			 targetPrivate);
	}

	return targetPrivate;
}




static const struct sockaddr *
is_valid_address(const struct sockaddr *address)
{
	const struct sockaddr	*valid	= NULL;
	static Boolean	warned	= FALSE;

	if ((address != NULL) &&
	    (address->sa_len <= sizeof(struct sockaddr_storage))) {
		switch (address->sa_family) {
			case AF_INET :
				if (address->sa_len >= sizeof(struct sockaddr_in)) {
					valid = address;
				} else {
					if (!warned) {
						SCLog(TRUE, LOG_ERR,
						      CFSTR("SCNetworkReachabilityCreateWithAddress[Pair] called with \"struct sockaddr *\" len %d < %zu"),
						      address->sa_len,
						      sizeof(struct sockaddr_in));
						warned = TRUE;
					}
				}
				break;
			case AF_INET6 :
				if (address->sa_len >= sizeof(struct sockaddr_in6)) {
					valid = address;
				} else if (!warned) {
					SCLog(TRUE, LOG_ERR,
					      CFSTR("SCNetworkReachabilityCreateWithAddress[Pair] called with \"struct sockaddr *\" len %d < %zu"),
					      address->sa_len,
					      sizeof(struct sockaddr_in6));
					warned = TRUE;
				}
				break;
			default :
				if (!warned) {
					SCLog(TRUE, LOG_ERR,
					      CFSTR("SCNetworkReachabilityCreateWithAddress[Pair] called with invalid address family %d"),
					      address->sa_family);
					warned = TRUE;
				}
		}
	}

	return valid;
}






SCNetworkReachabilityRef
SCNetworkReachabilityCreateWithAddress(CFAllocatorRef		allocator,
				       const struct sockaddr	*address)
{
	SCNetworkReachabilityPrivateRef	targetPrivate;

	address = is_valid_address(address);
	if (address == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	targetPrivate = __SCNetworkReachabilityCreatePrivate(allocator);
	if (targetPrivate == NULL) {
		return NULL;
	}

	targetPrivate->type = reachabilityTypeAddress;
	targetPrivate->remoteAddress = CFAllocatorAllocate(NULL, address->sa_len, 0);
	bcopy(address, targetPrivate->remoteAddress, address->sa_len);



	SCLog((_sc_debug && (_sc_log > 0)), LOG_INFO, CFSTR("%s%s %@"),
	      targetPrivate->log_prefix,
	      DEBUG_REACHABILITY_TYPE_ADDRESS,
	      targetPrivate);

	return (SCNetworkReachabilityRef)targetPrivate;
}


static Boolean
is_same_address(const struct sockaddr *a, const struct sockaddr *b)
{
	const void	*a_addr;
	const void	*b_addr;
	size_t		len;

	if ((a == NULL) ||
	    (b == NULL) ||
	    (a->sa_family != b->sa_family) ||
	    (a->sa_len    != b->sa_len   )) {
		return FALSE;
	}

	switch (a->sa_family) {
		case AF_INET : {
			struct sockaddr_in	*a_sin	= (struct sockaddr_in *)(void *)a;
			struct sockaddr_in	*b_sin	= (struct sockaddr_in *)(void *)b;

			/* ALIGN: assuming a (and b) are aligned, then cast ok. */
			a_addr = &a_sin->sin_addr;
			b_addr = &b_sin->sin_addr;
			len = sizeof(struct in_addr);
			break;
		}

		case AF_INET6 : {
			struct sockaddr_in6	*a_sin6	= (struct sockaddr_in6 *)(void *)a;
			struct sockaddr_in6	*b_sin6	= (struct sockaddr_in6 *)(void *)b;

			if (a_sin6->sin6_scope_id != b_sin6->sin6_scope_id) {
				return FALSE;
			}

			a_addr = &a_sin6->sin6_addr;
			b_addr = &b_sin6->sin6_addr;
			len = sizeof(struct in6_addr);
			break;
		}

		default :
			a_addr = a;
			b_addr = b;
			len = a->sa_len;
			break;
	}

	return (bcmp(a_addr, b_addr, len) == 0);
}


SCNetworkReachabilityRef
SCNetworkReachabilityCreateWithAddressPair(CFAllocatorRef		allocator,
					   const struct sockaddr	*localAddress,
					   const struct sockaddr	*remoteAddress)
{
	SCNetworkReachabilityPrivateRef	targetPrivate;

	if ((localAddress == NULL) && (remoteAddress == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	if (localAddress != NULL) {
		localAddress = is_valid_address(localAddress);
		if (localAddress == NULL) {
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}
	}

	if (remoteAddress != NULL) {
		remoteAddress = is_valid_address(remoteAddress);
		if (remoteAddress == NULL) {
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}
	}

	targetPrivate = __SCNetworkReachabilityCreatePrivate(allocator);
	if (targetPrivate == NULL) {
		return NULL;
	}

	targetPrivate->type = reachabilityTypeAddressPair;

	if (localAddress != NULL) {
		targetPrivate->localAddress = CFAllocatorAllocate(NULL, localAddress->sa_len, 0);
		bcopy(localAddress, targetPrivate->localAddress, localAddress->sa_len);
	}

	if (remoteAddress != NULL) {
		if (is_same_address(localAddress, remoteAddress)) {
			targetPrivate->remoteAddress = targetPrivate->localAddress;
		} else {
			targetPrivate->remoteAddress = CFAllocatorAllocate(NULL, remoteAddress->sa_len, 0);
			bcopy(remoteAddress, targetPrivate->remoteAddress, remoteAddress->sa_len);
		}
	}



	SCLog((_sc_debug && (_sc_log > 0)), LOG_INFO, CFSTR("%s%s %@"),
	      targetPrivate->log_prefix,
	      DEBUG_REACHABILITY_TYPE_ADDRESSPAIR,
	      targetPrivate);

	return (SCNetworkReachabilityRef)targetPrivate;
}


SCNetworkReachabilityRef
SCNetworkReachabilityCreateWithName(CFAllocatorRef	allocator,
				    const char		*nodename)
{
	union {
		struct sockaddr		sa;
		struct sockaddr_in	sin;
		struct sockaddr_in6	sin6;
	} addr;
	size_t				nodenameLen;
	SCNetworkReachabilityPrivateRef	targetPrivate;

	if (nodename == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	nodenameLen = strlen(nodename);
	if (nodenameLen == 0) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	if (nodename[nodenameLen - 1] == '.') {
		int		dots;
		size_t		i;

		// trim trailing "."s
		do {
			--nodenameLen;
		} while ((nodenameLen > 0) && (nodename[nodenameLen - 1] == '.'));

		if (nodenameLen == 0) {
			// if only trailing "."s
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}

		// count the remaining "."s
		dots = 0;
		for (i = 0; i < nodenameLen; i++) {
			if (nodename[i] == '.') dots++;
		}

		if (dots == 0) {
			// if only a single-label, add back the FQDN "."
			nodenameLen++;
		}
	}

	if (_SC_string_to_sockaddr(nodename, AF_UNSPEC, (void *)&addr, sizeof(addr)) != NULL) {
		/* if this "nodename" is really an IP[v6] address in disguise */
		return SCNetworkReachabilityCreateWithAddress(allocator, &addr.sa);
	}

	targetPrivate = __SCNetworkReachabilityCreatePrivate(allocator);
	if (targetPrivate == NULL) {
		return NULL;
	}

	targetPrivate->type = reachabilityTypeName;

	targetPrivate->name = CFAllocatorAllocate(NULL, nodenameLen + 1, 0);
	strlcpy((char *)targetPrivate->name, nodename, nodenameLen + 1);

	targetPrivate->needResolve = TRUE;
	targetPrivate->info.flags |= kSCNetworkReachabilityFlagsFirstResolvePending;
	targetPrivate->serverInfo.flags |= kSCNetworkReachabilityFlagsFirstResolvePending;

	{
		/* make sure AppLayerVPN only is in client mode */
		CFDictionaryRef	appLayerVPNProperties;

		appLayerVPNProperties = VPNAppLayerCopyCurrentAppProperties();
		if (appLayerVPNProperties != NULL) {
			targetPrivate->serverBypassForVPN = TRUE;
			targetPrivate->serverBypass = YES;
			CFRelease(appLayerVPNProperties);
		}
	}

	SCLog((_sc_debug && (_sc_log > 0)), LOG_INFO, CFSTR("%s%s %@"),
	      targetPrivate->log_prefix,
	      DEBUG_REACHABILITY_TYPE_NAME,
	      targetPrivate);

	return (SCNetworkReachabilityRef)targetPrivate;
}


static SCNetworkReachabilityRef
__SCNetworkReachabilityCreateWithPtr(CFAllocatorRef		allocator,
				     const char			*ptrName,
				     const struct sockaddr	*ptrAddress)
{
	SCNetworkReachabilityRef	target;
	SCNetworkReachabilityPrivateRef	targetPrivate;

	target = SCNetworkReachabilityCreateWithName(NULL, ptrName);
	if (target == NULL) {
		return NULL;
	}

	targetPrivate = (SCNetworkReachabilityPrivateRef)target;

	// change type
	targetPrivate->type = reachabilityTypePTR;

	// and keep the address
	targetPrivate->remoteAddress = CFAllocatorAllocate(NULL, ptrAddress->sa_len, 0);
	bcopy(ptrAddress, targetPrivate->remoteAddress, ptrAddress->sa_len);

	return target;
}




SCNetworkReachabilityRef
SCNetworkReachabilityCreateWithOptions(CFAllocatorRef	allocator,
				       CFDictionaryRef	options)
{
	const struct sockaddr		*addr_l		= NULL;
	const struct sockaddr		*addr_p		= NULL;
	const struct sockaddr		*addr_r		= NULL;
	CFDataRef			data;
	CFStringRef			interface	= NULL;
	CFStringRef			nodename;
	CFBooleanRef			onDemandBypass;
	CFBooleanRef			resolverBypass;
	CFBooleanRef			serverBypass;
	SCNetworkReachabilityRef	target;
	SCNetworkReachabilityPrivateRef	targetPrivate;

	if (!isA_CFDictionary(options)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	nodename = CFDictionaryGetValue(options, kSCNetworkReachabilityOptionNodeName);
	if ((nodename != NULL) &&
	    (!isA_CFString(nodename) || (CFStringGetLength(nodename) == 0))) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}
	data = CFDictionaryGetValue(options, kSCNetworkReachabilityOptionLocalAddress);
	if (data != NULL) {
		if (!isA_CFData(data) || (CFDataGetLength(data) < sizeof(struct sockaddr_in))) {
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}
		addr_l = (const struct sockaddr *)CFDataGetBytePtr(data);
	}
	data = CFDictionaryGetValue(options, kSCNetworkReachabilityOptionPTRAddress);
	if (data != NULL) {
		if (!isA_CFData(data) || (CFDataGetLength(data) < sizeof(struct sockaddr_in))) {
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}
		addr_p = (const struct sockaddr *)CFDataGetBytePtr(data);
	}
	data = CFDictionaryGetValue(options, kSCNetworkReachabilityOptionRemoteAddress);
	if (data != NULL) {
		if (!isA_CFData(data) || (CFDataGetLength(data) < sizeof(struct sockaddr_in))) {
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}
		addr_r = (const struct sockaddr *)CFDataGetBytePtr(data);
	}
	interface = CFDictionaryGetValue(options, kSCNetworkReachabilityOptionInterface);
	if ((interface != NULL) &&
	    (!isA_CFString(interface) || (CFStringGetLength(interface) == 0))) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}
	onDemandBypass = CFDictionaryGetValue(options, kSCNetworkReachabilityOptionConnectionOnDemandBypass);
	if ((onDemandBypass != NULL) && !isA_CFBoolean(onDemandBypass)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}
	resolverBypass = CFDictionaryGetValue(options, kSCNetworkReachabilityOptionResolverBypass);
	if ((resolverBypass != NULL) && !isA_CFBoolean(resolverBypass)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}


	serverBypass = CFDictionaryGetValue(options, kSCNetworkReachabilityOptionServerBypass);
	if ((serverBypass != NULL) && !isA_CFBoolean(serverBypass)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}


	if (nodename != NULL) {
		const char	*name;

		if ((addr_l != NULL) || (addr_r != NULL) || (addr_p != NULL)) {
			// can't have both a nodename and an address
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}

		name = _SC_cfstring_to_cstring(nodename, NULL, 0, kCFStringEncodingUTF8);
		target = SCNetworkReachabilityCreateWithName(allocator, name);
		CFAllocatorDeallocate(NULL, (void *)name);
	} else if (addr_p != NULL) {
		char	name[MAXHOSTNAMELEN];

		if ((addr_l != NULL) ||					// can't have PTR and target address
		    (addr_r != NULL) ||					// can't have PTR and target address
		    !addr_to_PTR_name(addr_p, name, sizeof(name))) {	// can't convert PTR
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}

		target = __SCNetworkReachabilityCreateWithPtr(NULL, name, addr_p);
	} else {
		if ((addr_l != NULL) && (addr_r != NULL)) {
			target = SCNetworkReachabilityCreateWithAddressPair(NULL, addr_l, addr_r);
		} else if (addr_r != NULL) {
			target = SCNetworkReachabilityCreateWithAddress(NULL, addr_r);
		} else if (addr_l != NULL) {
			target = SCNetworkReachabilityCreateWithAddressPair(NULL, addr_l, NULL);
		} else {
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}
	}
	if (target == NULL) {
		return NULL;
	}

	targetPrivate = (SCNetworkReachabilityPrivateRef)target;

	if (interface != NULL) {
		if ((_SC_cfstring_to_cstring(interface,
					     targetPrivate->if_name,
					     sizeof(targetPrivate->if_name),
					     kCFStringEncodingASCII) == NULL) ||
		    ((targetPrivate->if_index = if_nametoindex(targetPrivate->if_name)) == 0)) {
			CFRelease(targetPrivate);
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}
	}


	if (onDemandBypass != NULL) {
		targetPrivate->onDemandBypass = CFBooleanGetValue(onDemandBypass);
	}

	if (resolverBypass != NULL) {
		targetPrivate->resolverBypass = CFBooleanGetValue(resolverBypass);
	}

	/* if by name, make sure client-only VPN types stay in client mode */
	if (serverBypass != NULL && targetPrivate->serverBypassForVPN == FALSE) {
		targetPrivate->serverBypass = CFBooleanGetValue(serverBypass);
	}


	if (_sc_debug && (_sc_log > 0)) {
		const char	*opt	= "???";

		switch (targetPrivate->type) {
			case reachabilityTypeAddress :
				opt = DEBUG_REACHABILITY_TYPE_ADDRESS_OPTIONS;
				break;
			case reachabilityTypeAddressPair :
				opt = DEBUG_REACHABILITY_TYPE_ADDRESSPAIR_OPTIONS;
				break;
			case reachabilityTypeName :
				opt = DEBUG_REACHABILITY_TYPE_NAME_OPTIONS;
				break;
			case reachabilityTypePTR :
				opt = DEBUG_REACHABILITY_TYPE_PTR_OPTIONS;
				break;
		}

		SCLog(TRUE, LOG_INFO, CFSTR("%s%s %@"),
		      targetPrivate->log_prefix,
		      opt,
		      targetPrivate);
	}

	return (SCNetworkReachabilityRef)targetPrivate;
}


static SCNetworkReachabilityRef
__SCNetworkReachabilityCreateCopy(SCNetworkReachabilityRef target)
{
	SCNetworkReachabilityRef	clone		= NULL;
	SCNetworkReachabilityPrivateRef	clonePrivate;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	switch (targetPrivate->type) {
		case reachabilityTypeAddress :
			clone = SCNetworkReachabilityCreateWithAddress(NULL,
								       targetPrivate->remoteAddress);
			break;
		case reachabilityTypeAddressPair :
			clone = SCNetworkReachabilityCreateWithAddressPair(NULL,
									   targetPrivate->localAddress,
									   targetPrivate->remoteAddress);
			break;
		case reachabilityTypeName :
			clone = SCNetworkReachabilityCreateWithName(NULL,
								    targetPrivate->name);
			break;
		case reachabilityTypePTR :
			clone = __SCNetworkReachabilityCreateWithPtr(NULL,
								     targetPrivate->name,
								     targetPrivate->remoteAddress);
			break;
	}
	if (clone == NULL) {
		return NULL;
	}

	clonePrivate = (SCNetworkReachabilityPrivateRef)clone;

	clonePrivate->quiet = TRUE;

	clonePrivate->if_index = targetPrivate->if_index;
	bcopy(targetPrivate->if_name, clonePrivate->if_name, sizeof(clonePrivate->if_name));

	clonePrivate->onDemandBypass = targetPrivate->onDemandBypass;


	clonePrivate->serverBypass = targetPrivate->serverBypass;

	clonePrivate->resolverBypass = targetPrivate->resolverBypass;


	if (_sc_debug && (_sc_log > 0)) {
		const char	*opt	= "???";

		switch (clonePrivate->type) {
			case reachabilityTypeAddress :
				opt = DEBUG_REACHABILITY_TYPE_ADDRESS_CLONE;
				break;
			case reachabilityTypeAddressPair :
				opt = DEBUG_REACHABILITY_TYPE_ADDRESSPAIR_CLONE;
				break;
			case reachabilityTypeName :
				opt = DEBUG_REACHABILITY_TYPE_NAME_CLONE;
				break;
			case reachabilityTypePTR :
				opt = DEBUG_REACHABILITY_TYPE_PTR_CLONE;
				break;
		}

		SCLog(TRUE, LOG_INFO, CFSTR("%s%s %p %@"),
		      clonePrivate->log_prefix,
		      opt,
		      targetPrivate,
		      clone);
	}

	return clone;
}


CFTypeID
SCNetworkReachabilityGetTypeID(void)
{
	pthread_once(&initialized, __SCNetworkReachabilityInitialize);	/* initialize runtime */
	return __kSCNetworkReachabilityTypeID;
}


CFArrayRef	/* CFArray[CFData], where each CFData is a (struct sockaddr *) */
SCNetworkReachabilityCopyResolvedAddress(SCNetworkReachabilityRef	target,
					 int				*error_num)
{
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	if (!isA_SCNetworkReachability(target)) {
		_SCErrorSet(kSCStatusInvalidArgument);
	       return NULL;
	}

	if (!isReachabilityTypeName(targetPrivate->type)) {
		_SCErrorSet(kSCStatusInvalidArgument);
	       return NULL;
	}

	if (error_num) {
		*error_num = targetPrivate->resolvedError;
	}

	if (targetPrivate->resolvedAddresses != NULL) {
		if (isA_CFArray(targetPrivate->resolvedAddresses)) {
			return CFRetain(targetPrivate->resolvedAddresses);
		} else {
			/* if status is known but no resolved addresses to return */
			_SCErrorSet(kSCStatusOK);
			return NULL;
		}
	}

	_SCErrorSet(kSCStatusReachabilityUnknown);
	return NULL;
}


static void
__SCNetworkReachabilitySetResolvedError(SCNetworkReachabilityRef	target,
					int32_t				status)
{
	SCNetworkReachabilityPrivateRef		targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	MUTEX_ASSERT_HELD(&targetPrivate->lock);

	__mark_operation_end(target,
			     FALSE,				// if successful query
			     dns_query_async,			// async
			     &targetPrivate->dnsQueryStart,	// start time
			     &targetPrivate->dnsQueryEnd);	// end time

	if (targetPrivate->resolvedAddresses != NULL) {
		CFRelease(targetPrivate->resolvedAddresses);
		targetPrivate->resolvedAddresses = NULL;
	}

	SCLog(_sc_debug, LOG_INFO, CFSTR("%scould not be resolved: %s"),
	      targetPrivate->log_prefix,
	      gai_strerror(status));

	/* save the error associated with the attempt to resolve the name */
	targetPrivate->resolvedAddresses = CFRetain(kCFNull);
	targetPrivate->resolvedError     = status;
	targetPrivate->needResolve       = FALSE;

	return;
}


/*
 * rankReachability()
 *   Not reachable       == 0
 *   Connection Required == 1
 *   Reachable           == 2
 */
static int
rankReachability(SCNetworkReachabilityFlags flags)
{
	int	rank = 0;

	if (flags & kSCNetworkReachabilityFlagsReachable)		rank = 2;
	if (flags & kSCNetworkReachabilityFlagsConnectionRequired)	rank = 1;
	return rank;
}


#pragma mark -
#pragma mark DNS name resolution


static void
update_resolver_reachability(ReachabilityStoreInfoRef	store_info,
			     dns_resolver_t		*resolver,
			     SCNetworkReachabilityFlags	*flags,
			     Boolean			*haveDNS,
			     uint32_t			*resolver_if_index,
			     const char			*log_prefix)
{
	if (resolver_if_index) *resolver_if_index = 0;

	if (resolver->n_nameserver > 0) {
		*flags   = (SCNetworkReachabilityFlags)resolver->reach_flags;
		if (resolver_if_index != NULL) {
			*resolver_if_index = resolver->if_index;
		}
		*haveDNS = TRUE;
	} else {
		*flags   = kSCNetworkReachabilityFlagsReachable;
		*haveDNS = FALSE;
	}

	return;
}


static Boolean
check_matching_resolvers(ReachabilityStoreInfoRef	store_info,
			 dns_config_t			*dns_config,
			 const char			*fqdn,
			 unsigned int			if_index,
			 SCNetworkReachabilityFlags	*flags,
			 Boolean			*haveDNS,
			 uint32_t			*resolver_if_index,
			 int				*dns_config_index,
			 const char			*log_prefix)
{
	int		i;
	Boolean		matched		= FALSE;
	const char	*name		= fqdn;
	int32_t		n_resolvers;
	dns_resolver_t	**resolvers;

	if (if_index == 0) {
		n_resolvers = dns_config->n_resolver;
		resolvers   = dns_config->resolver;
	} else {
		n_resolvers = dns_config->n_scoped_resolver;
		resolvers   = dns_config->scoped_resolver;
	}

	/* In case we couldn't find a match, setting an index of -1
	   and resolver_if_index 0 */
	if (dns_config_index != NULL) *dns_config_index = -1;
	if (resolver_if_index != NULL) *resolver_if_index = 0;

	while (!matched && (name != NULL)) {
		size_t	len;

		/*
		 * check if the provided name (or sub-component)
		 * matches one of our resolver configurations.
		 */
		len = strlen(name);
		for (i = 0; i < n_resolvers; i++) {
			char		*domain;
			dns_resolver_t	*resolver;

			resolver = resolvers[i];
			if ((if_index != 0) && (if_index != resolver->if_index)) {
				continue;
			}

			domain   = resolver->domain;
			if (domain != NULL && (len == strlen(domain))) {
				if (strcasecmp(name, domain) == 0) {
					/*
					 * if name matches domain
					 */
					matched = TRUE;
					update_resolver_reachability(store_info,
								     resolver,
								     flags,
								     haveDNS,
								     resolver_if_index,
								     log_prefix);
					if (dns_config_index != NULL) *dns_config_index = i;
					break;
				}
			}
		}

		if (!matched) {
			/*
			 * we have not found a matching resolver, try
			 * a less qualified domain
			 */
			name = strchr(name, '.');
			if ((name != NULL) && (*name != '\0')) {
				name++;
			} else {
				name = NULL;
			}
		}
	}

	return matched;
}


static dns_resolver_t *
get_default_resolver(dns_config_t *dns_config, unsigned int if_index)
{
	int		i;
	int32_t		n_resolvers;
	dns_resolver_t	*resolver	= NULL;
	dns_resolver_t	**resolvers;

	if (if_index == 0) {
		n_resolvers = dns_config->n_resolver;
		resolvers   = dns_config->resolver;
	} else {
		n_resolvers = dns_config->n_scoped_resolver;
		resolvers   = dns_config->scoped_resolver;
	}

	for (i = 0; i < n_resolvers; i++) {
		if ((if_index != 0) && (if_index != resolvers[i]->if_index)) {
			continue;
		}

		if (((if_index == 0) && (i == 0)) ||
		    ((if_index != 0) && (resolver == NULL))) {
			// if this is the first (aka default) resolver
			resolver = resolvers[i];
		} else if ((resolvers[i]->domain == NULL) &&
			   (resolvers[i]->search_order < resolver->search_order)) {
			// if this is a default resolver with a lower search order
			resolver = resolvers[i];
		}
	}

	return resolver;
}


static dns_configuration_t *
dns_configuration_retain()
{
	dns_configuration_t	*config;

	pthread_mutex_lock(&dns_lock);

	if (dns_configuration != NULL) {
		Boolean		refresh	= TRUE;

		if (dns_token_valid) {
			int		check	= 0;
			uint32_t	status;

			/*
			 * check if the global [DNS] configuration snapshot needs
			 * to be updated
			 */
			status = notify_check(dns_token, &check);
			if (status != NOTIFY_STATUS_OK) {
				SCLog(TRUE, LOG_INFO, CFSTR("notify_check() failed, status=%u"), status);
			} else if (check == 0) {
				// if the snapshot does not need to be refreshed
				refresh = FALSE;
			}
		}

		if (refresh) {
			if (dns_configuration->refs == 0) {
				dns_configuration_free(dns_configuration->config);
				CFAllocatorDeallocate(NULL, dns_configuration);
			}
			dns_configuration = NULL;
		}
	}

	if (dns_configuration == NULL) {
		dns_config_t	*new_config;

		new_config = dns_configuration_copy();
		if (new_config != NULL) {
			dns_configuration = CFAllocatorAllocate(NULL, sizeof(dns_configuration_t), 0);
			dns_configuration->config = new_config;
			dns_configuration->refs   = 0;
		}
	}

	if (dns_configuration != NULL) {
		dns_configuration->refs++;
	}

	config = dns_configuration;
	pthread_mutex_unlock(&dns_lock);
	return config;
}


static void
dns_configuration_release(dns_configuration_t *config)
{
	pthread_mutex_lock(&dns_lock);

	config->refs--;
	if (config->refs == 0) {
		if (!dns_token_valid && (config == dns_configuration)) {
			dns_configuration = NULL;
		}

		if (config != dns_configuration) {
			dns_configuration_free(config->config);
			CFAllocatorDeallocate(NULL, config);
		}
	}

	pthread_mutex_unlock(&dns_lock);
	return;
}


static Boolean
dns_configuration_watch()
{
	int		dns_check	= 0;
	const char	*dns_key;
	Boolean		ok		= FALSE;
	uint32_t	status;

	pthread_mutex_lock(&dns_lock);

	dns_key = dns_configuration_notify_key();
	if (dns_key == NULL) {
		SCLog(TRUE, LOG_INFO, CFSTR("dns_configuration_notify_key() failed"));
		goto done;
	}

	status = notify_register_check(dns_key, &dns_token);
	if (status == NOTIFY_STATUS_OK) {
		dns_token_valid = TRUE;
	} else {
		SCLog(TRUE, LOG_INFO, CFSTR("notify_register_check() failed, status=%u"), status);
		goto done;
	}

	status = notify_check(dns_token, &dns_check);
	if (status != NOTIFY_STATUS_OK) {
		SCLog(TRUE, LOG_INFO, CFSTR("notify_check() failed, status=%u"), status);
		(void)notify_cancel(dns_token);
		dns_token_valid = FALSE;
		goto done;
	}

	ok = TRUE;

    done :

	pthread_mutex_unlock(&dns_lock);
	return ok;
}


static void
dns_configuration_unwatch()
{
	pthread_mutex_lock(&dns_lock);

	(void)notify_cancel(dns_token);
	dns_token_valid = FALSE;

	if ((dns_configuration != NULL) && (dns_configuration->refs == 0)) {
		dns_configuration_free(dns_configuration->config);
		CFAllocatorDeallocate(NULL, dns_configuration);
		dns_configuration = NULL;
	}

	pthread_mutex_unlock(&dns_lock);
	return;
}


static void
_SC_R_updateResolverReachability(ReachabilityStoreInfoRef	store_info,
				 SCNetworkReachabilityFlags	*flags,
				 Boolean			*haveDNS,
				 const char			*nodename,
				 unsigned int			if_index,
				 uint32_t			*resolver_if_index,
				 int				*dns_config_index,
				 const char			*log_prefix
				 )
{
	dns_resolver_t		*default_resolver;
	dns_configuration_t	*dns;
	Boolean			found			= FALSE;
	char			*fqdn			= (char *)nodename;
	int			i;
	Boolean			isFQDN			= FALSE;
	size_t			len;
	const int		ndots			= 1;
	Boolean			useDefault		= FALSE;

	if (resolver_if_index) *resolver_if_index = 0;
	if (dns_config_index) *dns_config_index = -1;

	/*
	 * We first assume that all of the configured DNS servers
	 * are available.  Since we don't know which name server will
	 * be consulted to resolve the specified nodename we need to
	 * check the availability of ALL name servers.  We can only
	 * proceed if we know that our query can be answered.
	 */

	*flags   = kSCNetworkReachabilityFlagsReachable;
	*haveDNS = FALSE;

	len = (nodename != NULL) ? strlen(nodename) : 0;
	if (len == 0) {
		// if no nodename, return not reachable
		*flags = 0;
		return;
	}

	dns = dns_configuration_retain();
	if (dns == NULL) {
		// if error
		SCLog(_sc_debug, LOG_INFO, CFSTR("%sDNS: no configuration"), log_prefix);
		goto done;
	}

	default_resolver = get_default_resolver(dns->config, if_index);
	if (default_resolver == NULL) {
		// if no resolver configuration
		SCLog(_sc_debug, LOG_INFO, CFSTR("%sDNS: no resolvers"), log_prefix);
		goto done;
	}

	if (fqdn[len - 1] == '.') {
		isFQDN = TRUE;

		// trim trailing '.''s
		while ((len > 0) && (fqdn[len-1] == '.')) {
			if (fqdn == nodename) {
				fqdn = strdup(nodename);
				assert(fqdn != nodename);
			}
			fqdn[--len] = '\0';
		}
	}

	/*
	 * check if the provided name matches a supplemental domain
	 */
	found = check_matching_resolvers(store_info, dns->config, fqdn, if_index,
					 flags, haveDNS, resolver_if_index,
					 dns_config_index, log_prefix);

	if (!found && !isFQDN) {
		/*
		 * if we did not match a supplemental domain name and if the
		 * provided name has enough "."s then the first query will be
		 * directed to the default resolver.
		 */
		char	*cp;
		int	dots;

		dots = 0;
		for (cp = fqdn; *cp != '\0'; cp++) {
			if (*cp == '.') dots++;
		}

		/* Per KB: HT4845 */
		if (dots >= ndots) {
			useDefault = TRUE;
		}
	}

	if (!found && !isFQDN && !useDefault && (dns->config->n_resolver > 1)) {
		/*
		 * FQDN not specified, try matching w/search domains
		 */
		if (default_resolver->n_search > 0) {
			for (i = 0; !found && (i < default_resolver->n_search); i++) {
				int	ret;
				char	*search_fqdn	= NULL;

				ret = asprintf(&search_fqdn, "%s.%s", fqdn, default_resolver->search[i]);
				if (ret == -1) {
					continue;
				}

				// try the provided name with the search domain appended
				found = check_matching_resolvers(store_info,
								 dns->config,
								 search_fqdn,
								 if_index,
								 flags,
								 haveDNS,
								 resolver_if_index,
								 dns_config_index,
								 log_prefix);
				free(search_fqdn);
			}
		} else if (default_resolver->domain != NULL) {
			char	*dp;
			int	domain_parts	= 0;

			// count domain parts
			for (dp = default_resolver->domain; *dp != '\0'; dp++) {
				if (*dp == '.') {
					domain_parts++;
				}
			}

			// remove trailing dots
			for (dp--; (dp >= default_resolver->domain) && (*dp == '.'); dp--) {
				*dp = '\0';
				domain_parts--;
			}

			if (dp >= default_resolver->domain) {
				// dots are separators, bump # of components
				domain_parts++;
			}

			dp = default_resolver->domain;
			for (i = LOCALDOMAINPARTS; !found && (i <= (domain_parts - ndots)); i++) {
				int	ret;
				char	*search_fqdn	= NULL;

				ret = asprintf(&search_fqdn, "%s.%s", fqdn, dp);
				if (ret == -1) {
					continue;
				}

				// try the provided name with the [default] domain appended
				found = check_matching_resolvers(store_info,
								 dns->config,
								 search_fqdn,
								 if_index,
								 flags,
								 haveDNS,
								 resolver_if_index,
								 dns_config_index,
								 log_prefix);
				free(search_fqdn);

				// move to the next component of the [default] domain
				dp = strchr(dp, '.') + 1;
			}
		}
	}

	if (!found) {
		// update the reachability of the default resolver
		update_resolver_reachability(store_info,
					     default_resolver,
					     flags,
					     haveDNS,
					     resolver_if_index,
					     log_prefix);
		if (dns_config_index != NULL) *dns_config_index = 0;
	}

    done :

	if (fqdn != nodename)	free(fqdn);

	if (dns != NULL) {
		dns_configuration_release(dns);
	}

	return;
}


Boolean
__SC_checkResolverReachabilityInternal(SCDynamicStoreRef		*storeP,
				       SCNetworkReachabilityFlags	*flags,
				       Boolean				*haveDNS,
				       const char			*nodename,
				       uint32_t				*resolver_if_index,
				       int				*dns_config_index)
{
	Boolean			ok;
	ReachabilityStoreInfo	store_info;

	ReachabilityStoreInfo_init(&store_info);
	ok = ReachabilityStoreInfo_update(&store_info, storeP, AF_UNSPEC);
	if (!ok) {
		goto done;
	}

	_SC_R_updateResolverReachability(&store_info,
					 flags,
					 haveDNS,
					 nodename,
					 0,
					 resolver_if_index,
					 dns_config_index,
					 "");

    done :

	ReachabilityStoreInfo_free(&store_info);
	return ok;
}


/*
 * _SC_checkResolverReachabilityByAddress()
 *
 * Given an IP address, determine whether a reverse DNS query can be issued
 * using the current network configuration.
 */
Boolean
_SC_checkResolverReachabilityByAddress(SCDynamicStoreRef		*storeP,
				       SCNetworkReachabilityFlags	*flags,
				       Boolean				*haveDNS,
				       struct sockaddr			*sa)
{
	Boolean			ok;
	char			ptr_name[128];
	ReachabilityStoreInfo	store_info;

	ReachabilityStoreInfo_init(&store_info);
	ok = ReachabilityStoreInfo_update(&store_info, storeP, AF_UNSPEC);
	if (!ok) {
		goto done;
	}

	/*
	 * Ideally, we would have an API that given a local IP
	 * address would return the DNS server(s) that would field
	 * a given PTR query.  Fortunately, we do have an SPI which
	 * which will provide this information given a "name" so we
	 * take the address, convert it into the inverse query name,
	 * and find out which servers should be consulted.
	 */
	ok = addr_to_PTR_name(sa, ptr_name, sizeof(ptr_name));
	if (!ok) {
		goto done;
	}

	_SC_R_updateResolverReachability(&store_info, flags, haveDNS, ptr_name, 0, NULL, NULL, "");

    done :

	ReachabilityStoreInfo_free(&store_info);
	return ok;
}


#pragma mark -
#pragma mark DNSServiceGetAddrInfo support


/*
 * DNS query handling
 *
 * Notes :
 *
 * 1. We have a "contract" with mDNSResponder that for EVERY network
 *    or DNS configuration change that should warrant our [re-]starting
 *    a query, mDNSResponder will acknowledge the latest DNS configuration.
 *
 * 2. IPMonitor also posts a notification AFTER every network or DNS
 *    configuration change.
 *
 * 3. We use IPMonitor's "trailing edge" as a signal to restart any
 *    by-name queries.
 */


// Note: protected by _hn_target_queue()
static int			dns_refresh_token;
static Boolean			dns_refresh_token_valid	= FALSE;


/*
 * dns_refresh_handler
 *
 * Called to notify/update all SCNetworkReachability by-name targets of
 * a network/DNS change.  The change should [re-]start a DNS query to
 * resolve the name.
 * - should be exec'd on the _hn_target_queue()
 */
static void
dns_refresh_handler()
{
	CFArrayRef			changes;
	CFStringRef			key;
	__block SCDynamicStoreRef	store	= NULL;

	dispatch_sync(_hn_target_queue(), ^{
		if (dns_refresh_token_valid && (hn_store != NULL)) {
			store = CFRetain(hn_store);
		}
	});

	if (store == NULL) {
		return;
	}

	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							 kSCDynamicStoreDomainState,
							 kSCEntNetDNS);
	changes = CFArrayCreate(NULL, (const void **)&key, 1, &kCFTypeArrayCallBacks);
	__SCNetworkReachabilityHandleChanges(store, changes, NULL);
	CFRelease(changes);
	CFRelease(key);

	CFRelease(store);
	return;
}


/*
 * dns_refresh_enable
 *
 * Called to monitor for network/DNS changes that should restart a DNS query.
 * - caller must be running on the _hn_target_queue()
 */
static Boolean
dns_refresh_enable(dispatch_queue_t q)
{
	uint32_t	status;

	status = notify_register_dispatch(_SC_NOTIFY_NETWORK_CHANGE,
					  &dns_refresh_token,
					  q,
					  ^(int token){
						  dns_refresh_handler();
					  });
	if (status != NOTIFY_STATUS_OK) {
		SCLog(TRUE, LOG_INFO, CFSTR("notify_register_dispatch() failed, status=%u"), status);
		return FALSE;
	}

	dns_refresh_token_valid = TRUE;

	return TRUE;
}


/*
 * dns_refresh_disable
 *
 * Called to stop monitoring for network/DNS changes
 * - caller must be running on the _hn_target_queue()
 */
static void
dns_refresh_disable()
{
	(void)notify_cancel(dns_refresh_token);
	dns_refresh_token_valid = FALSE;
	return;
}


#pragma mark -
#pragma mark [m]DNS Queries


static void
dequeueDNSQuery(SCNetworkReachabilityRef target);


static dispatch_queue_t
_dns_queue()
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create("SCNetworkReachability.DNSService", NULL);
	});

	return q;
}


/*
 * _dns_complete
 */
static __inline__ Boolean
_dns_complete(SCNetworkReachabilityRef target)
{
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	if ((targetPrivate->dnsHaveV4 && targetPrivate->dnsHaveV6) ||
	    targetPrivate->dnsHavePTR ||
	    targetPrivate->dnsHaveError ||
	    targetPrivate->dnsHaveTimeout) {
		return TRUE;
	}

	return FALSE;
}


/*
 * _dns_notify
 *
 * Called to push out a target's DNS changes
 * - caller must be running on the _dns_queue()
 */
static void
_dns_notify(const void *value, void *context)
{
	SCNetworkReachabilityRef	target		= (SCNetworkReachabilityRef)value;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	MUTEX_LOCK(&targetPrivate->lock);

	if (_dns_complete(target)) {
		__mark_operation_end(target,
				     (targetPrivate->resolvedError == NETDB_SUCCESS),	// if successful query
				     dns_query_mdns,					// [m]DNS query
				     &targetPrivate->dnsQueryStart,			// start time
				     &targetPrivate->dnsQueryEnd);			// end time

		// update target info
		if (targetPrivate->resolvedAddresses != NULL) {
			CFRelease(targetPrivate->resolvedAddresses);
		}
		targetPrivate->resolvedAddresses = targetPrivate->dnsAddresses;
		targetPrivate->dnsAddresses      = NULL;

		targetPrivate->resolvedError     = targetPrivate->dnsError;
		targetPrivate->dnsError          = NETDB_SUCCESS;

		dequeueDNSQuery(target);

		targetPrivate->needResolve = FALSE;

		if (targetPrivate->scheduled) {
			__SCNetworkReachabilityUpdate(target);
		}
	}

	MUTEX_UNLOCK(&targetPrivate->lock);
	return;
}


typedef enum {
	MARK_NONE,
	MARK_ERROR,
	MARK_TIMEOUT,
	MARK_HAVE_V4,
	MARK_HAVE_V6,
	MARK_HAVE_PTR,
} _dns_mark_t;


/*
 * _dns_mark
 */
static __inline__ void
_dns_mark(SCNetworkReachabilityRef target, _dns_mark_t mark)
{
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	switch (mark) {
		case MARK_NONE :
			break;
		case MARK_ERROR :
			targetPrivate->dnsHaveError = TRUE;
			break;
		case MARK_TIMEOUT :
			targetPrivate->dnsHaveTimeout = TRUE;
			break;
		case MARK_HAVE_V4 :
			targetPrivate->dnsHaveV4 = TRUE;
			break;
		case MARK_HAVE_V6 :
			targetPrivate->dnsHaveV6 = TRUE;
			break;
		case MARK_HAVE_PTR :
			targetPrivate->dnsHavePTR = TRUE;
			break;
	}

	return;
}


/*
 * _dns_callback
 *
 * Called to process [m]DNS query updates
 * - caller must be running on the _dns_queue()
 */
static void
_dns_callback(DNSServiceRef		sdRef,
	      DNSServiceFlags		flags,
	      DNSServiceErrorType	errorCode,
	      _dns_mark_t		dnsMark,
	      CFTypeRef			dnsAddress,	// CFData(struct sockaddr) or CFString(ptr_name)
	      void			*context)
{
	int				failures	= 0;
	Boolean				restart		= FALSE;
	SCNetworkReachabilityRef	target		= (SCNetworkReachabilityRef)context;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	MUTEX_LOCK(&targetPrivate->lock);

	if (sdRef != targetPrivate->dnsTarget) {
		// if this DNSServiceRef is no longer associated with the target
		MUTEX_UNLOCK(&targetPrivate->lock);
		return;
	}

	switch (errorCode) {
		case kDNSServiceErr_NoError :
			if (dnsAddress != NULL) {
				CFMutableArrayRef	addresses;
				CFIndex			i;

				_dns_mark(target, dnsMark);

				if (targetPrivate->dnsAddresses != NULL) {
					if (isA_CFArray(targetPrivate->dnsAddresses)) {
						addresses = CFArrayCreateMutableCopy(NULL, 0, targetPrivate->dnsAddresses);
					} else {
						addresses = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
					}

					CFRelease(targetPrivate->dnsAddresses);
					targetPrivate->dnsAddresses = NULL;
				} else {
					addresses = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
				}

				i = CFArrayGetFirstIndexOfValue(addresses,
								CFRangeMake(0, CFArrayGetCount(addresses)),
								dnsAddress);
				if (flags & kDNSServiceFlagsAdd) {
					// add address
					if (i == kCFNotFound) {
						CFArrayAppendValue(addresses, dnsAddress);
					}
#ifdef	HANDLE_RMV_REQUESTS
				} else {
					// remove address
					if (i != kCFNotFound) {
						CFArrayRemoveValueAtIndex(addresses, i);
					}
#endif	// HANDLE_RMV_REQUESTS
				}

				if (CFArrayGetCount(addresses) > 0) {
					targetPrivate->dnsAddresses = addresses;
					targetPrivate->dnsError     = NETDB_SUCCESS;
				} else {
					// if host not found
					targetPrivate->dnsAddresses = CFRetain(kCFNull);
					targetPrivate->dnsError     = EAI_NONAME;
					CFRelease(addresses);
				}

			}
			break;
		case kDNSServiceErr_BadParam :
			_dns_mark(target, MARK_ERROR);

			if (targetPrivate->dnsAddresses != NULL) {
				CFRelease(targetPrivate->dnsAddresses);
			}
			targetPrivate->dnsAddresses = CFRetain(kCFNull);
			targetPrivate->dnsError     = EAI_NONAME;
			break;
		case kDNSServiceErr_NoSuchRecord :
			_dns_mark(target, dnsMark);

			if (targetPrivate->dnsAddresses == NULL) {
				targetPrivate->dnsAddresses = CFRetain(kCFNull);
				targetPrivate->dnsError     = EAI_NONAME;
			}
			break;
		case kDNSServiceErr_Timeout :
			_dns_mark(target, MARK_TIMEOUT);

			if (targetPrivate->dnsAddresses == NULL) {
				targetPrivate->dnsAddresses = CFRetain(kCFNull);
				targetPrivate->dnsError     = EAI_NONAME;
			}
			break;
		default :
			SCLog(TRUE, LOG_ERR,
			      CFSTR("%sSCNetworkReachability _dns_callback w/error=%d (n=%d)"),
			      targetPrivate->log_prefix,
			      errorCode,
			      targetPrivate->dnsFailures + 1);
			// fall through
		case kDNSServiceErr_ServiceNotRunning :
			_dns_mark(target, MARK_ERROR);

			// bump per-target failure count
			failures = ++targetPrivate->dnsFailures;

			// Check to see if we've seen too many failures for this target
			if (failures > 2) {
				// if so, there's little point in retrying over
				// and over again so let's just return an error
				// and move on.
				if (targetPrivate->dnsAddresses != NULL) {
					CFRelease(targetPrivate->dnsAddresses);
				}
				targetPrivate->dnsAddresses = CFRetain(kCFNull);
				targetPrivate->dnsError     = EAI_NONAME;
			} else if (targetPrivate->dnsGeneration == dnsGeneration) {
				// if not, then "mDNSResponder" crashed or some
				// other/unexpected error occurred.  In this
				// case, we'll try again with a clean slate and
				// restart all requests.
				if (dnsMain != NULL) {
					DNSServiceRefDeallocate(dnsMain);
					dnsMain = NULL;
					dnsCount = 0;
					dnsGeneration++;
					restart = TRUE;
				}
			}
			break;
	}

	// update DNS failure count (and [re-]set to zero if we're OK)
	targetPrivate->dnsFailures = failures;

	MUTEX_UNLOCK(&targetPrivate->lock);

	if (restart) {
		SCLog(TRUE, LOG_DEBUG,
		      CFSTR("%sreconnecting SCNetworkReachability w/\"mDNSResponder\" (%d)"),
		      targetPrivate->log_prefix,
		      dnsGeneration);

		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 500 * NSEC_PER_MSEC),
			       _hn_changes_queue(),
			       ^{
			dns_refresh_handler();
		});

		// and flush the dnsUpdated queue as any DNS results we may have
		// accumulated are no longer valid.
		if (dnsUpdated != NULL) {
			CFRelease(dnsUpdated);
			dnsUpdated = NULL;
		}
		return;
	}

	if (targetPrivate->dnsHaveTimeout) {
	    targetPrivate->dnsNoAddressesSinceLastTimeout = TRUE;
	} else if (targetPrivate->dnsNoAddressesSinceLastTimeout &&
	           isA_CFArray(targetPrivate->dnsAddresses) &&
	           CFArrayGetCount(targetPrivate->dnsAddresses) > 0)
	{
	    targetPrivate->dnsNoAddressesSinceLastTimeout = FALSE;
	}

	// the "more coming" flag applies to DNSService callouts for any/all
	// hosts that are being watched so we need to keep track of the targets
	// we have updated.  When we [finally] have the last callout then we
	// push our notifications for all of the updated targets.

	if (dnsUpdated == NULL) {
		dnsUpdated = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
	}
	CFSetAddValue(dnsUpdated, target);

	if (!(flags & kDNSServiceFlagsMoreComing)) {
		CFSetApplyFunction(dnsUpdated, _dns_notify, NULL);
		CFRelease(dnsUpdated);
		dnsUpdated = NULL;
	}

	return;
}


/*
 * _dns_getaddrinfo_callback
 *
 * Called to process [m]DNS query updates
 * - caller must be running on the _dns_queue()
 */
static void
_dns_getaddrinfo_callback(DNSServiceRef			sdRef,
			  DNSServiceFlags		flags,
			  uint32_t			interfaceIndex,
			  DNSServiceErrorType		errorCode,
			  const char			*hostname,
			  const struct sockaddr		*address,
			  uint32_t			ttl,
			  void				*context)
{
	CFDataRef	dnsAddress	= NULL;
	_dns_mark_t	dnsMark		= MARK_NONE;

	if (address != NULL) {
		switch (errorCode) {
			case kDNSServiceErr_NoError :
				dnsAddress = CFDataCreate(NULL, (void *)address, address->sa_len);
				// ... and fall through
			case kDNSServiceErr_NoSuchRecord :
				switch (address->sa_family) {
					case AF_INET :
						dnsMark = MARK_HAVE_V4;
						break;
					case AF_INET6 :
						dnsMark = MARK_HAVE_V6;
						break;
				}
				break;
			default :
				break;
		}
	}

	_dns_callback(sdRef, flags, errorCode, dnsMark, dnsAddress, context);

	if (dnsAddress != NULL) {
		CFRelease(dnsAddress);
	}

	return;
}


static CFStringRef
_dns_copy_domain_name(const uint8_t *rdata, uint16_t rdlen)
{
	CFMutableStringRef	domain;
	const uint8_t		*label;
	uint8_t			label_len;

	domain = CFStringCreateMutable(NULL, 0);

	label = rdata;
	label_len = *(label++);
	while (label_len != 0) {
		while (label_len-- > 0) {
			uint8_t		byte	= *label++;

			if ((byte == '.') || (byte == '\\')) {
				// if escape needed
				CFStringAppendFormat(domain, NULL, CFSTR("\\%c"), byte);
			} else if (byte <= ' ') {
				CFStringAppendFormat(domain, NULL, CFSTR("\\%c%c%c"),
						     '0' + (byte / 100),
						     '0' + ((byte / 10) % 10),
						     '0' + (byte % 10));
			} else {
				CFStringAppendFormat(domain, NULL, CFSTR("%c"), byte);
			}
		}

		label_len = *(label++);
		if (label_len != 0) {
			CFStringAppendFormat(domain, NULL, CFSTR("."));
		}
	}

	return domain;
}


static void
_dns_queryrecord_callback(DNSServiceRef			sdRef,
			  DNSServiceFlags		flags,
			  uint32_t			interfaceIndex,
			  DNSServiceErrorType		errorCode,
			  const char			*fullname,
			  uint16_t			rrtype,
			  uint16_t			rrclass,
			  uint16_t			rdlen,
			  const void			*rdata,
			  uint32_t			ttl,
			  void				*context)
{
	_dns_mark_t			dnsMark		= MARK_NONE;
	CFStringRef			dnsPTRName	= NULL;
	SCNetworkReachabilityRef	target		= (SCNetworkReachabilityRef)context;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	// for now, we only support using DNSServiceQueryRecord for PTR queries
	assert(targetPrivate->type == reachabilityTypePTR);

	if (rdata != NULL) {
		switch (errorCode) {
			case kDNSServiceErr_NoError :
				if (rrtype == kDNSServiceType_PTR) {
					dnsPTRName = _dns_copy_domain_name(rdata, rdlen);
				}
				// ... and fall through
			case kDNSServiceErr_NoSuchRecord :
				dnsMark = MARK_HAVE_PTR;
				break;
			default :
				break;
		}
	}

	_dns_callback(sdRef, flags, errorCode, dnsMark, dnsPTRName, context);

	if (dnsPTRName != NULL) {
		CFRelease(dnsPTRName);
	}

	return;
}


static Boolean
enqueueDNSQuery(SCNetworkReachabilityRef target)
{
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	MUTEX_ASSERT_HELD(&targetPrivate->lock);

	// clear DNS flags, mark the query active
	targetPrivate->dnsFlags = 0;
	targetPrivate->dnsActive = TRUE;

	// track the DNS resolution time
	__mark_operation_start(&targetPrivate->dnsQueryStart, &targetPrivate->dnsQueryEnd);

	CFRetain(target);
	dispatch_async(_dns_queue(), ^{
		DNSServiceErrorType	err;
		const char		*fcn	= "???";
		DNSServiceRef		sdRef	= NULL;

		if (targetPrivate->dnsTarget != NULL) {
			// if already running
			CFRelease(target);
			return;
		}

		// if needed, start interacting with "mDNSResponder"
		if (dnsMain == NULL) {
			err = DNSServiceCreateConnection(&dnsMain);
			if (err != kDNSServiceErr_NoError) {
				SCLog(TRUE, LOG_ERR,
				      CFSTR("%sDNSServiceCreateConnection(&dnsMain) failed, error = %d"),
				      targetPrivate->log_prefix,
				      err);
				goto done;
			}

			err = DNSServiceSetDispatchQueue(dnsMain, _dns_queue());
			if (err != kDNSServiceErr_NoError) {
				SCLog(TRUE, LOG_ERR,
				      CFSTR("%sDNSServiceSetDispatchQueue() failed, error = %d"),
				      targetPrivate->log_prefix,
				      err);
				DNSServiceRefDeallocate(dnsMain);
				dnsMain = NULL;
				dnsGeneration++;
				goto done;
			}
		}

		// start a query for this target
		sdRef = dnsMain;

		switch (targetPrivate->type) {
			case reachabilityTypeName :
				fcn = "DNSServiceGetAddrInfo";
				err = DNSServiceGetAddrInfo(&sdRef,				// sdRef
							    kDNSServiceFlagsReturnIntermediates // flags
							    | kDNSServiceFlagsShareConnection
							    | kDNSServiceFlagsSuppressUnusable
							    | kDNSServiceFlagsTimeout,
							    targetPrivate->if_index,		// interfaceIndex
							    0,					// protocol
							    targetPrivate->name,		// hostname
							    _dns_getaddrinfo_callback,		// callback
							    (void *)target);			// context
				break;
			case reachabilityTypePTR :
				fcn = "DNSServiceQueryRecord";
				err = DNSServiceQueryRecord(&sdRef,				// sdRef
							    kDNSServiceFlagsReturnIntermediates	// flags
							    | kDNSServiceFlagsShareConnection
							    | kDNSServiceFlagsSuppressUnusable
							    | kDNSServiceFlagsTimeout,
							    targetPrivate->if_index,		// interfaceIndex
							    targetPrivate->name,		// fullname
							    kDNSServiceType_PTR,		// rrtype
							    kDNSServiceClass_IN,		// rrclass
							    _dns_queryrecord_callback,		// callback
							    (void *)target);			// context
				break;
			default :
				err = kDNSServiceErr_Unknown;
				break;
		}

		switch (err) {
			case kDNSServiceErr_NoError :
				dnsCount++;
				break;

			default :
				SCLog(TRUE, LOG_ERR,
				      CFSTR("%s%s() failed, error = %d (%d)"),
				      targetPrivate->log_prefix,
				      fcn,
				      err,
				      dnsCount);
				// fall through

			case kDNSServiceErr_BadParam :
				if (dnsCount == 0) {
					// if this was the first request
					DNSServiceRefDeallocate(dnsMain);
					dnsMain = NULL;
					dnsGeneration++;
				}
				sdRef = NULL;
				break;
		}

	    done :

		MUTEX_LOCK(&targetPrivate->lock);

		if (err == kDNSServiceErr_NoError) {
			targetPrivate->dnsGeneration = dnsGeneration;
			targetPrivate->dnsTarget = sdRef;
		} else {
			targetPrivate->dnsActive = FALSE;

			// queue up the returned error
			dispatch_async(_dns_queue(), ^{
				_dns_callback(NULL,			// sdRef
					      0,			// flags
					      err,			// errorCode
					      MARK_ERROR,		// dnsMark
					      NULL,			// dnsAddress
					      (void *)target);		// context
				CFRelease(target);
			});
		}

		MUTEX_UNLOCK(&targetPrivate->lock);

		return;
	});

	return TRUE;
}


static void
dequeueDNSQuery(SCNetworkReachabilityRef target)
{
	DNSServiceRef			sdRef;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	MUTEX_ASSERT_HELD(&targetPrivate->lock);

	// terminate the [target] [m]DNS query
	sdRef = targetPrivate->dnsTarget;
	targetPrivate->dnsTarget = NULL;

	// mark the query NOT active
	targetPrivate->dnsActive = FALSE;

	// don't do anything if the sdRef is not valid
	if (sdRef != NULL) {
		int			generation;

		generation = targetPrivate->dnsGeneration;
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 3LL * NSEC_PER_SEC),
			       _dns_queue(),
			       ^{
			if (generation == dnsGeneration) {
				// if we're pointing to the same DNSService
				// generation as the main/active session
				// deallocate per-target query
				DNSServiceRefDeallocate(sdRef);
				dnsCount--;
				if (dnsCount == 0) {
					// if no more queries active
					DNSServiceRefDeallocate(dnsMain);
					dnsMain = NULL;
					dnsGeneration++;
				}
			}

			CFRelease(target);
		});
	}

	if (targetPrivate->dnsAddresses != NULL) {
		CFRelease(targetPrivate->dnsAddresses);
		targetPrivate->dnsAddresses = NULL;
	}
	targetPrivate->dnsError = NETDB_SUCCESS;

	return;
}


#pragma mark -
#pragma mark Synchronous DNS query support


#define SYNC_DNS_QUERY_TIMEOUT_NSEC	35 * NSEC_PER_SEC	// 35s


static void
sync_DNS_query_callback(SCNetworkReachabilityRef	clone,
			SCNetworkReachabilityFlags	cloneFlags,
			void				*info)
{
	dispatch_semaphore_t		s	= (dispatch_semaphore_t)info;

	dispatch_semaphore_signal(s);
	return;
}


static void
sync_DNS_query(SCNetworkReachabilityRef target)
{
	SCNetworkReachabilityRef	clone;
	SCNetworkReachabilityPrivateRef	clonePrivate;
	SCNetworkReachabilityContext	context	= { 0, NULL, NULL, NULL, NULL };
	dispatch_queue_t		q;
	long				ret;
	dispatch_semaphore_t		s;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	clone = __SCNetworkReachabilityCreateCopy(target);
	if (clone == NULL) {
		return;
	}
	clonePrivate = (SCNetworkReachabilityPrivateRef)clone;

	q = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
	s = dispatch_semaphore_create(0);

	// start async query
	context.info = (void *)s;
	SCNetworkReachabilitySetCallback(clone, sync_DNS_query_callback, &context);
	SCNetworkReachabilitySetDispatchQueue(clone, q);

	// wait for reply (or timeout)
	ret = dispatch_semaphore_wait(s, dispatch_time(DISPATCH_TIME_NOW,
						       SYNC_DNS_QUERY_TIMEOUT_NSEC));
	if (ret != 0) {
		dispatch_sync(_dns_queue(), ^{
			// mark as both a timeout *and* an error
			_dns_mark(clone, MARK_TIMEOUT);
			_dns_mark(clone, MARK_ERROR);

			__mark_operation_end(clone,
					     FALSE,				// if successful query
					     dns_query_mdns_timeout,		// [m]DNS query
					     &clonePrivate->dnsQueryStart,	// start time
					     &clonePrivate->dnsQueryEnd);	// end time

			MUTEX_LOCK(&clonePrivate->lock);

			// update target info with what's available
			if (clonePrivate->resolvedAddresses != NULL) {
				CFRelease(clonePrivate->resolvedAddresses);
				clonePrivate->resolvedAddresses = NULL;
			}
			if ((clonePrivate->dnsAddresses != NULL) &&
			    isA_CFArray(clonePrivate->dnsAddresses) &&
			    (CFArrayGetCount(clonePrivate->dnsAddresses) > 0)) {
				clonePrivate->resolvedAddresses = CFArrayCreateMutableCopy(NULL,
											   0,
											   clonePrivate->dnsAddresses);
			}
			if (clonePrivate->resolvedAddresses != NULL) {
				// if timeout w/partial results
				clonePrivate->resolvedError     = NETDB_SUCCESS;
			} else {
				// if timeout w/no results
				clonePrivate->resolvedAddresses = CFRetain(kCFNull);
				clonePrivate->resolvedError     = EAI_NONAME;
			}

			MUTEX_UNLOCK(&clonePrivate->lock);
		});
	}

	// cancel request
	SCNetworkReachabilitySetDispatchQueue(clone, NULL);
	SCNetworkReachabilitySetCallback(clone, NULL, NULL);

	// transfer reply
	if (clonePrivate->resolvedAddresses  != NULL) CFRetain(clonePrivate->resolvedAddresses);
	if (targetPrivate->resolvedAddresses != NULL) CFRelease(targetPrivate->resolvedAddresses);
	targetPrivate->resolvedAddresses = clonePrivate->resolvedAddresses;
	targetPrivate->resolvedError     = clonePrivate->resolvedError;
	targetPrivate->resolverFlags     = clonePrivate->resolverFlags;
	targetPrivate->cycle             = clonePrivate->cycle;
	targetPrivate->dnsFlags          = clonePrivate->dnsFlags;
	memcpy(&targetPrivate->info, &clonePrivate->info, sizeof(ReachabilityInfo));
	memcpy(&targetPrivate->last_notify, &clonePrivate->last_notify, sizeof(ReachabilityInfo));

	CFRelease(clone);
	dispatch_release(s);

	return;
}


#pragma mark -
#pragma mark Network Information support


// Note: protected by _hn_target_queue()
static int			network_changed_token;
static Boolean			network_changed_token_valid	= FALSE;


/*
 * nwi_refresh_handler
 *
 * Called to notify/update network changed events
 * - should be exec'd on the _hn_changes_queue()
 */
static void
nwi_refresh_handler()
{
	CFArrayRef			changes;
	CFStringRef			key;
	__block SCDynamicStoreRef	store	= NULL;

	dispatch_sync(_hn_target_queue(), ^{
		if (network_changed_token_valid && (hn_store != NULL)) {
			store = CFRetain(hn_store);
		}
	});

	if (store == NULL) {
		return;
	}

	// Fake a network change.
	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							 kSCDynamicStoreDomainState,
							 kSCEntNetIPv4);
	changes = CFArrayCreate(NULL, (const void **)&key, 1, &kCFTypeArrayCallBacks);
	__SCNetworkReachabilityHandleChanges(store, changes, NULL);
	CFRelease(changes);
	CFRelease(key);

	CFRelease(store);
	return;
}


/*
 * nwi_refresh_enable
 *
 * Called to monitor for network changes.
 * - caller must be running on the _hn_target_queue()
 * - passed in queue should be _hn_changes_queue()
 */
static Boolean
nwi_refresh_enable(dispatch_queue_t q)
{
	uint32_t	status;

	status = notify_register_dispatch(_SC_NOTIFY_NETWORK_CHANGE_NWI,	// trailing nwi_state_get_notify_key()
					  &network_changed_token,
					  q,
					  ^(int token){
						  nwi_refresh_handler();
					  });
	if (status != NOTIFY_STATUS_OK) {
		SCLog(TRUE, LOG_INFO, CFSTR("notify_register_dispatch() failed for network changes, status=%u"), status);
		return FALSE;
	}

	network_changed_token_valid = TRUE;

	return TRUE;
}


/*
 * nwi_refresh_disable
 *
 * Called to stop monitoring for network changes
 * - caller must be running on the _hn_target_queue()
 */
static void
nwi_refresh_disable()
{
	if (network_changed_token_valid) {
		(void)notify_cancel(network_changed_token);
		network_changed_token_valid = FALSE;
	}

	return;
}


#pragma mark -
#pragma mark Sleep/wake support


#if	!TARGET_OS_IPHONE

// Note: protected by _hn_target_queue()
static IOPMConnection		power_changed_connection	= NULL;
static const CFStringRef	power_changed_key		= CFSTR("*** EARLY WAKE ***");


/*
 * power_refresh_handler
 *
 * Called to notify/update power capability changed events
 * - should be exec'd on the _hn_changes_queue()
 */
static void
power_refresh_handler(void				*param,
		      IOPMConnection			connection,
		      IOPMConnectionMessageToken	token,
		      IOPMSystemPowerStateCapabilities	capabilities)
{
	Boolean				change;
	IOReturn			ret;
	__block SCDynamicStoreRef	store	= NULL;

	dispatch_sync(_hn_target_queue(), ^{
		if ((power_changed_connection != NULL) && (hn_store != NULL)) {
			store = CFRetain(hn_store);
		}
	});

	if (store == NULL) {
		return;
	}

	// check for [relevant] changes
	change = ((power_capabilities ^ capabilities) & POWER_CAPABILITIES_NETWORK) != 0;

	// update capabilities
	power_capabilities = capabilities;

	if (change) {
		CFArrayRef	changes;

		// fake a network change.
		changes = CFArrayCreate(NULL, (const void **)&power_changed_key, 1, &kCFTypeArrayCallBacks);
		__SCNetworkReachabilityHandleChanges(store, changes, NULL);
		CFRelease(changes);
	}

	ret = IOPMConnectionAcknowledgeEvent(connection, token);
	if (ret != kIOReturnSuccess) {
		SCLog(TRUE, LOG_ERR, CFSTR("IOPMConnectionAcknowledgeEvent failed, 0x%08x"), ret);
	}

	CFRelease(store);
	return;
}


/*
 * power_refresh_enable
 *
 * Called to monitor power changes.
 * - caller must be running on the _hn_target_queue()
 * - passed in queue should be _hn_changes_queue()
 */
static Boolean
power_refresh_enable(dispatch_queue_t q)
{
	IOPMConnection	connection	= NULL;
	IOReturn	ret;

	ret = IOPMConnectionCreate(CFSTR("com.apple.SCNetworkReachability"),
				   kIOPMEarlyWakeNotification | kIOPMSleepWakeInterest,
				   &connection);
	if (ret != kIOReturnSuccess) {
		SCLog(TRUE, LOG_ERR, CFSTR("IOPMConnectionCreate failed, 0x%08x"), ret);
		goto failed;
	}

	ret = IOPMConnectionSetNotification(connection, NULL, power_refresh_handler);
	if (ret != kIOReturnSuccess) {
		SCLog(TRUE, LOG_ERR, CFSTR("IOPMConnectionSetNotification failed, 0x%08x"), ret);
		goto failed;
	}

	power_changed_connection = connection;
	IOPMConnectionSetDispatchQueue(connection, q);
	power_capabilities = IOPMConnectionGetSystemCapabilities();

	return TRUE;

    failed:

	if (connection != NULL) {
		IOPMConnectionRelease(connection);
	}

	return FALSE;
}


static void
power_refresh_disable()
{
	if (power_changed_connection != NULL) {
		IOPMConnectionSetDispatchQueue(power_changed_connection, NULL);
		IOPMConnectionRelease(power_changed_connection);
		power_changed_connection = NULL;
	}

	return;
}

#endif	// !TARGET_OS_IPHONE






#pragma mark -
#pragma mark OnDemand


SCNetworkServiceRef
SCNetworkReachabilityCopyOnDemandService(SCNetworkReachabilityRef	target,
					 CFDictionaryRef		*userOptions)
{
	SCNetworkServiceRef		service		= NULL;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	if (!isA_SCNetworkReachability(target)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	if (targetPrivate->onDemandServiceID != NULL) {
		service = _SCNetworkServiceCopyActive(NULL, targetPrivate->onDemandServiceID);
	}

	if (userOptions != NULL) {
		if (targetPrivate->onDemandName != NULL) {
			*userOptions = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
			CFDictionarySetValue((CFMutableDictionaryRef)*userOptions, kSCNetworkConnectionSelectionOptionOnDemandHostName, targetPrivate->onDemandName);
		} else {
			*userOptions = NULL;
		}
	}

	return service;
}




static void
__SCNetworkReachabilityOnDemandCheckCallback(SCNetworkReachabilityRef	onDemandServer,
					     SCNetworkReachabilityFlags	onDemandFlags,
					     void			*info)
{
	SCNetworkReachabilityRef	target		= (SCNetworkReachabilityRef)info;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	MUTEX_LOCK(&targetPrivate->lock);

	if (!targetPrivate->scheduled) {
		// if not currently scheduled
		MUTEX_UNLOCK(&targetPrivate->lock);
		return;
	}

	SCLog(_sc_debug, LOG_INFO, CFSTR("%sOnDemand \"server\" status changed (now 0x%08x)"),
	      targetPrivate->log_prefix,
	      onDemandFlags);

	if (targetPrivate->type == reachabilityTypeName) {
		// make sure that we resolve the name again
		targetPrivate->needResolve = TRUE;
	}

	__SCNetworkReachabilityUpdate(target);

	MUTEX_UNLOCK(&targetPrivate->lock);

	return;
}


static Boolean
__SCNetworkReachabilityOnDemandCheck(ReachabilityStoreInfoRef	store_info,
				     SCNetworkReachabilityRef	target,
				     Boolean			onDemandRetry,
				     SCNetworkReachabilityFlags	*flags)
{
	SCNetworkConnectionRef		connection		= NULL;
	SCNetworkConnectionType		connectionType		= kSCNetworkConnectionTypeUnknown;
	Boolean				isAppLayerVPN		= FALSE;
	Boolean				isOnDemandService	= FALSE;
	Boolean				ok			= FALSE;
	CFStringRef			onDemandRemoteAddress	= NULL;
	CFStringRef			onDemandServiceID	= NULL;
	SCNetworkConnectionStatus	onDemandStatus		= kSCNetworkConnectionInvalid;
	CFMutableDictionaryRef		selectOptions		= NULL;
	Boolean				success			= FALSE;
	SCNetworkReachabilityPrivateRef	targetPrivate		= (SCNetworkReachabilityPrivateRef)target;

	MUTEX_ASSERT_HELD(&targetPrivate->lock);

	if (targetPrivate->onDemandName == NULL) {
		targetPrivate->onDemandName = CFStringCreateWithCString(NULL, targetPrivate->name, kCFStringEncodingUTF8);
	}

	/*
	 * check if an OnDemand VPN configuration matches the name.
	 */

	connection = SCNetworkConnectionCreate(kCFAllocatorDefault, NULL, NULL);
	if (connection == NULL) {
		goto done;
	}

	/* set select options */
	selectOptions = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (selectOptions == NULL) {
		goto done;
	}

	CFDictionaryAddValue(selectOptions, kSCNetworkConnectionSelectionOptionOnDemandHostName, targetPrivate->onDemandName);
	CFDictionaryAddValue(selectOptions, kSCNetworkConnectionSelectionOptionOnDemandRetry, onDemandRetry ? kCFBooleanTrue : kCFBooleanFalse);
	CFDictionaryAddValue(selectOptions, kSCNetworkConnectionSelectionOptionNoUserPrefs, kCFBooleanTrue);

	/* select service. May be On Demand or App Layer VPN */
	if (!SCNetworkConnectionSelectServiceWithOptions(connection, selectOptions)) {
		goto done;
	}

	/* get reachability flags (of VPN server) */
	(void) SCNetworkConnectionGetReachabilityInfo(connection, flags, NULL);

	connectionType = SCNetworkConnectionGetType(connection);
	if (connectionType == kSCNetworkConnectionTypeAppLayerVPN) {
		isAppLayerVPN = TRUE;
	}

	/* get on-demand info */
	onDemandServiceID = SCNetworkConnectionCopyServiceID(connection);
	if (SCNetworkConnectionCopyOnDemandInfo(connection, &onDemandRemoteAddress, &onDemandStatus)) {
		if (onDemandRemoteAddress != NULL) {
			isOnDemandService = TRUE;
			ok = TRUE;
		}
	}

	/* handle non-OnDemand App Layer VPN */
	if (isAppLayerVPN && !isOnDemandService) {
		SCLog(_sc_debug, LOG_INFO, CFSTR("%s  status  * = 0x%08x (App Layer VPN)"),
		      targetPrivate->log_prefix,
		      *flags);
		if (*flags & kSCNetworkReachabilityFlagsReachable) {
			// if VPN "server" is reachable

			if (!(*flags & kSCNetworkReachabilityFlagsTransientConnection)) {
				// start w/clean flags if not already layered on a transient network
				*flags = kSCNetworkReachabilityFlagsReachable;
			}

			*flags |= kSCNetworkReachabilityFlagsTransientConnection;
			if (onDemandStatus != kSCNetworkConnectionConnected) {
				*flags |= kSCNetworkReachabilityFlagsConnectionRequired;
			}

			SCLog(_sc_debug, LOG_INFO, CFSTR("%s  status    = isReachable%s"),
			      (onDemandStatus != kSCNetworkConnectionConnected)
					? " (after App Layer connect)" : "",
			      targetPrivate->log_prefix);
		}

		success = TRUE;
		goto done;
	}

	if (!_SC_CFEqual(targetPrivate->onDemandRemoteAddress, onDemandRemoteAddress) ||
	    !_SC_CFEqual(targetPrivate->onDemandServiceID, onDemandServiceID)) {
		if (targetPrivate->onDemandRemoteAddress != NULL) {
			CFRelease(targetPrivate->onDemandRemoteAddress);
			targetPrivate->onDemandRemoteAddress = NULL;
		}

		if (targetPrivate->onDemandServer != NULL) {
			SCNetworkReachabilitySetCallback(targetPrivate->onDemandServer, NULL, NULL);
			if (targetPrivate->dispatchQueue != NULL) {
				// unschedule
				__SCNetworkReachabilityUnscheduleFromRunLoop(targetPrivate->onDemandServer, NULL, NULL, TRUE);
			} else if (targetPrivate->rls != NULL) {
				CFIndex	i;
				CFIndex	n;

				// unschedule
				n = CFArrayGetCount(targetPrivate->rlList);
				for (i = 0; i < n; i += 3) {
					CFRunLoopRef	rl	= (CFRunLoopRef)CFArrayGetValueAtIndex(targetPrivate->rlList, i+1);
					CFStringRef	rlMode	= (CFStringRef) CFArrayGetValueAtIndex(targetPrivate->rlList, i+2);

					__SCNetworkReachabilityUnscheduleFromRunLoop(targetPrivate->onDemandServer, rl, rlMode, TRUE);
				}
			}

			CFRelease(targetPrivate->onDemandServer);
			targetPrivate->onDemandServer = NULL;
		}

		if (targetPrivate->onDemandServiceID != NULL) {
			CFRelease(targetPrivate->onDemandServiceID);
			targetPrivate->onDemandServiceID = NULL;
		}
	}

	if (ok) {
		if (onDemandStatus != kSCNetworkConnectionConnected) {
			/*
			 * if we have a VPN configuration matching the name *and* we need to
			 * bring the VPN up.  Combine our flags with those of the VPN server.
			 */
			if (targetPrivate->onDemandServer == NULL) {
				SCNetworkReachabilityPrivateRef	demandPrivate;
				CFMutableDictionaryRef		options;

				options = CFDictionaryCreateMutable(NULL,
								    0,
								    &kCFTypeDictionaryKeyCallBacks,
								    &kCFTypeDictionaryValueCallBacks);
				CFDictionarySetValue(options, kSCNetworkReachabilityOptionNodeName, onDemandRemoteAddress);
				CFDictionarySetValue(options, kSCNetworkReachabilityOptionConnectionOnDemandBypass, kCFBooleanTrue);
				if (targetPrivate->serverBypass) {
					CFDictionarySetValue(options, kSCNetworkReachabilityOptionServerBypass, kCFBooleanTrue);
				}
				targetPrivate->onDemandServer = SCNetworkReachabilityCreateWithOptions(NULL, options);
				CFRelease(options);

				// indent OnDemand target
				demandPrivate = (SCNetworkReachabilityPrivateRef)targetPrivate->onDemandServer;
				strlcat(demandPrivate->log_prefix, ".... ", sizeof(demandPrivate->log_prefix));

				if (targetPrivate->scheduled) {
					SCNetworkReachabilityContext	context	= { 0, NULL, CFRetain, CFRelease, CFCopyDescription };

					context.info = (void *)target;
					SCNetworkReachabilitySetCallback(targetPrivate->onDemandServer,
									 __SCNetworkReachabilityOnDemandCheckCallback,
									 &context);

					// schedule server reachability to match that of the target
					if (targetPrivate->dispatchQueue != NULL) {
						__SCNetworkReachabilityScheduleWithRunLoop(targetPrivate->onDemandServer, NULL, NULL, targetPrivate->dispatchQueue, TRUE);
					} else {
						CFIndex	i;
						CFIndex	n;

						n = CFArrayGetCount(targetPrivate->rlList);
						for (i = 0; i < n; i += 3) {
							CFRunLoopRef	rl	= (CFRunLoopRef)CFArrayGetValueAtIndex(targetPrivate->rlList, i+1);
							CFStringRef	rlMode	= (CFStringRef) CFArrayGetValueAtIndex(targetPrivate->rlList, i+2);

							__SCNetworkReachabilityScheduleWithRunLoop(targetPrivate->onDemandServer, rl, rlMode, NULL, TRUE);
						}
					}
				}
			}

			SCLog(_sc_debug, LOG_INFO, CFSTR("%s  status  * = 0x%08x"),
			      targetPrivate->log_prefix,
			      *flags);


			if ((*flags & kSCNetworkReachabilityFlagsReachable) && !(*flags & kSCNetworkReachabilityFlagsConnectionRequired)) {
				// if VPN "server" is [still] reachable

				if (!(*flags & kSCNetworkReachabilityFlagsTransientConnection)) {
					// start w/clean flags if not already layered on a transient network
					*flags = kSCNetworkReachabilityFlagsReachable;
				}

				*flags |= kSCNetworkReachabilityFlagsTransientConnection;
				*flags |= kSCNetworkReachabilityFlagsConnectionRequired;
				*flags |= kSCNetworkReachabilityFlagsConnectionOnDemand;

				// set 'InterventionRequired' if the OnDemand connection is paused
				if (SCNetworkConnectionIsOnDemandSuspended(connection)) {
					*flags |= kSCNetworkReachabilityFlagsInterventionRequired;
				}

				if (_sc_debug) {
					SCLog(TRUE, LOG_INFO, CFSTR("%s  service * = %@"),
					      targetPrivate->log_prefix,
					      onDemandServiceID);
					SCLog(TRUE, LOG_INFO, CFSTR("%s  status    = isReachable (after OnDemand connect)"),
					      targetPrivate->log_prefix);
				}

				success = TRUE;
			}
		}

		if (onDemandRemoteAddress != NULL) {
			if (targetPrivate->onDemandRemoteAddress == NULL) {
				targetPrivate->onDemandRemoteAddress = CFRetain(onDemandRemoteAddress);
			}
		}

		if (onDemandServiceID != NULL) {
			if (targetPrivate->onDemandServiceID == NULL) {
				targetPrivate->onDemandServiceID = CFRetain(onDemandServiceID);
			}
		}
	}

    done:

	if (onDemandServiceID != NULL) {
		CFRelease(onDemandServiceID);
	}
	if (onDemandRemoteAddress != NULL) {
		CFRelease(onDemandRemoteAddress);
	}
	if (connection != NULL) {
		CFRelease(connection);
	}
	if (selectOptions != NULL) {
		CFRelease(selectOptions);
	}
	return success;
}


/*
 * OnDemand configuration handling
 *
 * Notes :
 *
 * 1. We have a "contract" with mDNSResponder that for EVERY network
 *    or DNS configuration change that should warrant our [re-]starting
 *    a query, mDNSResponder will acknowledge the latest DNS configuration.
 *
 * 2. IPMonitor also posts a notification AFTER every network or DNS
 *    configuration change.
 *
 * 3. We use IPMonitor's "trailing edge" as a signal to restart any
 *    by-name queries.
 */


// Note: protected by _hn_target_queue()
static int			onDemand_refresh_token;
static Boolean			onDemand_refresh_token_valid	= FALSE;


/*
 * onDemand_refresh_handler
 *
 * Called to notify/update all SCNetworkReachability targets of
 * OnDemand changes.
 * - should be exec'd on the _hn_changes_queue()
 */
static void
onDemand_refresh_handler()
{
	CFArrayRef			changes;
	CFStringRef			key;
	__block SCDynamicStoreRef	store	= NULL;

	dispatch_sync(_hn_target_queue(), ^{
		if (onDemand_refresh_token_valid && (hn_store != NULL)) {
			store = CFRetain(hn_store);
		}
	});

	if (store == NULL) {
		return;
	}

	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							 kSCDynamicStoreDomainState,
							 kSCEntNetOnDemand);
	changes = CFArrayCreate(NULL, (const void **)&key, 1, &kCFTypeArrayCallBacks);
	__SCNetworkReachabilityHandleChanges(store, changes, NULL);
	CFRelease(changes);
	CFRelease(key);

	CFRelease(store);
	return;
}


/*
 * onDemand_refresh_enable
 *
 * Called to monitor for OnDemand changes.
 * - caller must be running on the _hn_target_queue()
 */
static Boolean
onDemand_refresh_enable(dispatch_queue_t q)
{
	uint32_t	status;

	status = notify_register_dispatch(kSCNETWORKCONNECTION_ONDEMAND_NOTIFY_KEY,
					  &onDemand_refresh_token,
					  q,
					  ^(int token){
						  onDemand_refresh_handler();
					  });
	if (status != NOTIFY_STATUS_OK) {
		SCLog(TRUE, LOG_INFO, CFSTR("notify_register_dispatch() failed, status=%u"), status);
		return FALSE;
	}

	onDemand_refresh_token_valid = TRUE;

	return TRUE;
}


/*
 * onDemand_refresh_disable
 *
 * Called to stop monitoring for OnDemand changes
 * - caller must be running on the _hn_target_queue()
 */
static void
onDemand_refresh_disable()
{
	(void)notify_cancel(onDemand_refresh_token);
	onDemand_refresh_token_valid = FALSE;
	return;
}




#pragma mark -
#pragma mark Reachability Flags


static Boolean
__SCNetworkReachabilityGetFlags(ReachabilityStoreInfoRef	store_info,
				SCNetworkReachabilityRef	target,
				ReachabilityInfo		*reach_info,
				Boolean				async)
{
	CFMutableArrayRef		addresses	= NULL;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;
	ReachabilityInfo		my_info		= NOT_REACHABLE;
	Boolean				ok		= TRUE;

	MUTEX_ASSERT_HELD(&targetPrivate->lock);

	_reach_set(reach_info, &NOT_REACHABLE, reach_info->cycle, targetPrivate->if_index, targetPrivate->if_name);

	if (!isA_SCNetworkReachability(target)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

#if	TARGET_OS_IPHONE
	if (isReachabilityTypeName(targetPrivate->type) &&
	    !async &&
	    pthread_is_threaded_np() &&
	    pthread_main_np()) {
		SCLog(TRUE, LOG_WARNING, CFSTR("Warning: sync SCNetworkReachability (by-name) query on main thread"));
	}
#endif	// TARGET_OS_IPHONE

	if (!targetPrivate->serverBypass) {
		if (!targetPrivate->serverActive) {

			ok = __SCNetworkReachabilityServer_targetAdd(target);
			if (!ok) {
				targetPrivate->serverBypass = TRUE;
			}
		}

		if (targetPrivate->serverActive) {
			ok = __SCNetworkReachabilityServer_targetStatus(target);
			if (!ok) {
				SCLog(TRUE, LOG_DEBUG,
				      CFSTR("__SCNetworkReachabilityGetFlags _targetStatus() failed"));
				_SCErrorSet(kSCStatusFailed);
				goto done;
			}

			targetPrivate->cycle = targetPrivate->serverInfo.cycle;
			_reach_set(&my_info,
				   &targetPrivate->serverInfo,
				   targetPrivate->serverInfo.cycle,
				   targetPrivate->if_index,
				   targetPrivate->if_name);
			goto done;
		}
	}


	switch (targetPrivate->type) {
		case reachabilityTypeAddress :
		case reachabilityTypeAddressPair : {
			/*
			 * Check "local" address
			 */
			if (targetPrivate->localAddress != NULL) {
				/*
				 * Check "local" address
				 */
				ok = checkAddress(store_info,
						  targetPrivate->localAddress,
						  targetPrivate->if_index,
						  &my_info,
						  targetPrivate->log_prefix);
				if (!ok) {
					goto done2;	/* not today */
				}

				if (!(my_info.flags & kSCNetworkReachabilityFlagsIsLocalAddress)) {
					goto done2;	/* not reachable, non-"local" address */
				}
			}

			/*
			 * Check "remote" address
			 */
			if ((targetPrivate->remoteAddress != NULL) &&
			    (targetPrivate->localAddress != targetPrivate->remoteAddress)) {
				/*
				 * in cases where we have different "local" and "remote" addresses
				 * we need to re-initialize the to-be-returned flags.
				 */
				my_info = NOT_REACHABLE;

				/*
				 * Check "remote" address
				 */
				ok = checkAddress(store_info,
						  targetPrivate->remoteAddress,
						  targetPrivate->if_index,
						  &my_info,
						  targetPrivate->log_prefix);
				if (!ok) {
					goto done2;	/* not today */
				}
			}

			break;

		}

		case reachabilityTypeName :
		case reachabilityTypePTR  : {
			int				error;
			int				ns_dns_config	= -1;
			SCNetworkReachabilityFlags	ns_flags	= 0;
			uint32_t			ns_if_index	= 0;

			addresses = (CFMutableArrayRef)SCNetworkReachabilityCopyResolvedAddress(target, &error);
			if ((addresses != NULL) || (error != NETDB_SUCCESS)) {
				/* if resolved or an error had been detected */
				if (!async) {
					/* if not an async request */
					goto checkResolvedAddresses;
				} else if (targetPrivate->dnsActive) {
					/* if [m]DNS query active */
					goto checkResolvedAddresses;
				} else if (!targetPrivate->needResolve) {
					/*
					 * if this is an async request (i.e. someone is watching the reachability
					 * of this target), if no query active, and if no query is needed
					 */
					goto checkResolvedAddresses;
				}
			}

			if (!targetPrivate->onDemandBypass) {
				Boolean				onDemand;
				SCNetworkReachabilityFlags	onDemandFlags	= 0;

				/*
				 * before we attempt our initial DNS query, check if there is
				 * an OnDemand configuration that we should be using.
				 */
				onDemand = __SCNetworkReachabilityOnDemandCheck(store_info, target, FALSE, &onDemandFlags);
				if (onDemand) {
					/* if OnDemand connection is needed */
					my_info.flags = onDemandFlags;
					goto done;
				}
			}

			targetPrivate->dnsBlocked = FALSE;

			/* update the reachability of the DNS servers */
			_SC_R_updateResolverReachability(store_info,
							 &ns_flags,
							 &targetPrivate->haveDNS,
							 targetPrivate->name,
							 targetPrivate->if_index,
							 &ns_if_index,
							 &ns_dns_config,
							 targetPrivate->log_prefix);


			// save resolver reachability flags
			targetPrivate->resolverFlags = ns_flags;

			if (rankReachability(ns_flags) < 2) {
				/*
				 * if DNS servers are not (or are no longer) reachable, set
				 * flags based on the availability of configured (but not
				 * active) services.
				 */

				SCLog(_sc_debug, LOG_INFO, CFSTR("%sDNS server(s) not available"),
				      targetPrivate->log_prefix);

				if (!targetPrivate->dnsBlocked) {
					ok = checkAddress(store_info,
							  NULL,
							  targetPrivate->if_index,
							  &my_info,
							  targetPrivate->log_prefix);
					if (!ok) {
						SCLog(_sc_debug, LOG_INFO, CFSTR("%sNo available networks"),
						      targetPrivate->log_prefix);
						goto done2;
					}
				} else {
					// if not checking "available" networks
					my_info.flags = ns_flags;
					my_info.if_index = ns_if_index;
				}

				if (async && targetPrivate->scheduled) {
					/*
					 * return "host not found", set flags appropriately,
					 * and schedule notification.
					 */
					__SCNetworkReachabilitySetResolvedError(target, EAI_NONAME);
					my_info.flags |= (targetPrivate->info.flags & kSCNetworkReachabilityFlagsFirstResolvePending);

					SCLog(_sc_debug, LOG_INFO, CFSTR("%sno DNS servers are reachable"),
					      targetPrivate->log_prefix);
					__SCNetworkReachabilityUpdate(target);
				}

				break;
			}

			if (targetPrivate->resolverBypass) {
				if (targetPrivate->haveDNS) {
					/*
					 * if we are not resolving the name, and if we have
					 * one or more DNS resolvers, then return flags that
					 * reflect the reachability of the resolvers (and
					 * not the actual name).
					 */
					my_info.flags = ns_flags;
					my_info.if_index = ns_if_index;
				}
				break;
			}

			if (async) {
				/* for async requests we return the last known status */
				my_info = targetPrivate->info;

				if (targetPrivate->dnsActive) {
					/* if [m]DNS query active */
					if (_sc_debug && !targetPrivate->quiet) {
						SCLog(TRUE, LOG_INFO,
						      CFSTR("%swaiting for DNS reply"),
						      targetPrivate->log_prefix);
					}
					if ((addresses != NULL) || (error != NETDB_SUCCESS)) {
						/* updated reachability based on the previous reply */
						goto checkResolvedAddresses;
					}
					break;
				}

				SCLog(_sc_debug, LOG_INFO,
				      CFSTR("%sstart DNS query for name = %s"),
				      targetPrivate->log_prefix,
				      targetPrivate->name);

				/*
				 * initiate an DNS query w/DNSServiceGetAddrInfo
				 */
				enqueueDNSQuery(target);
				break;
			}

			SCLog(_sc_debug, LOG_INFO,
			      CFSTR("%scheckName(%s)"),
			      targetPrivate->log_prefix,
			      targetPrivate->name);

			/*
			 * OK, all of the DNS name servers are available.  Let's
			 * resolve the nodename into an address.
			 */
			sync_DNS_query(target);

			if (!(targetPrivate->dnsHaveTimeout && targetPrivate->dnsHaveError)) {
				// if target reach info is valid
				memcpy(reach_info, &targetPrivate->info, sizeof(ReachabilityInfo));
				goto done2;
			}

			if (addresses != NULL)	CFRelease(addresses);
			addresses = (CFMutableArrayRef)SCNetworkReachabilityCopyResolvedAddress(target, &error);

		    checkResolvedAddresses :

			/*
			 * We first assume that the requested host is NOT available.
			 * Then, check each address for accessibility and return the
			 * best status available.
			 */
			my_info = NOT_REACHABLE;

			if ((targetPrivate->type == reachabilityTypeName) && isA_CFArray(addresses)) {
				CFIndex		i;
				CFIndex		n	= CFArrayGetCount(addresses);
				struct sockaddr	*sa;

				for (i = 0; i < n; i++) {
					ReachabilityInfo	ns_info	= NOT_REACHABLE;

					sa = (struct sockaddr *)CFDataGetBytePtr(CFArrayGetValueAtIndex(addresses, i));

					ok = checkAddress(store_info,
							  sa,
							  targetPrivate->if_index,
							  &ns_info,
							  targetPrivate->log_prefix);
					if (!ok) {
						goto done2;	/* not today */
					}

					if (rankReachability(ns_info.flags) > rankReachability(my_info.flags)) {
						/* return the best case result */
						my_info = ns_info;
						if (rankReachability(my_info.flags) == 2) {
							/* can't get any better than REACHABLE */
							break;
						}
					}
				}

				if (_sc_debug) {
					for (i++; i < n; i++) {
						sa = (struct sockaddr *)CFDataGetBytePtr(CFArrayGetValueAtIndex(addresses, i));
						log_address("skipAddress",
							    sa,
							    targetPrivate->if_index,
							    targetPrivate->log_prefix);
					}
				}
			} else if ((targetPrivate->type == reachabilityTypePTR) && isA_CFArray(addresses)) {
				CFIndex		i;
				CFIndex		n	= CFArrayGetCount(addresses);

				my_info = NOT_REACHABLE;

				for (i = 0; i < n; i++) {
					if (i == 0) {
						my_info.flags = kSCNetworkReachabilityFlagsReachable;
					}

					if (_sc_debug) {
						CFStringRef	ptrName;

						ptrName = CFArrayGetValueAtIndex(addresses, i);
						SCLog(TRUE, LOG_INFO, CFSTR("%sPTR name(%@)"),
						      targetPrivate->log_prefix,
						      ptrName);
					}
				}
			} else {
				if ((error == EAI_NONAME)
#if defined(EAI_NODATA) && (EAI_NODATA != EAI_NONAME)
				     || (error == EAI_NODATA)
#endif
				     ) {
					/*
					 * the target host name could not be resolved
					 */
					if (!targetPrivate->onDemandBypass) {
						Boolean				onDemand;
						SCNetworkReachabilityFlags	onDemandFlags	= 0;

						/*
						 * our initial DNS query failed, check again to see if there
						 * there is an OnDemand configuration that we should be using.
						 */
						onDemand = __SCNetworkReachabilityOnDemandCheck(store_info, target, TRUE, &onDemandFlags);
						if (onDemand) {
							/* if OnDemand connection is needed */
							my_info.flags = onDemandFlags;
							goto done;
						}
					}


					if (!targetPrivate->haveDNS) {
						/*
						 * No DNS servers are defined. Set flags based on
						 * the availability of configured (but not active)
						 * services.
						 */
						ok = checkAddress(store_info,
								  NULL,
								  targetPrivate->if_index,
								  &my_info,
								  targetPrivate->log_prefix);
						if (!ok) {
							goto done2;	/* not today */
						}

						if ((my_info.flags & kSCNetworkReachabilityFlagsReachable) &&
							(my_info.flags & kSCNetworkReachabilityFlagsConnectionRequired)) {
							/*
							 * Since we might pick up a set of DNS servers when this connection
							 * is established, don't reply with a "HOST NOT FOUND" error just yet.
							 */
							break;
						}

						/* Host not found, not reachable! */
						my_info = NOT_REACHABLE;
					}
				}
			}

			break;
		}
	}

    done:


	_reach_set(reach_info, &my_info, targetPrivate->cycle, targetPrivate->if_index, targetPrivate->if_name);

    done2 :

	if (addresses != NULL)	CFRelease(addresses);
	return ok;
}

int
SCNetworkReachabilityGetInterfaceIndex(SCNetworkReachabilityRef target)
{
	int				if_index	= -1;
	Boolean				ok		= TRUE;
	ReachabilityStoreInfo		store_info;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	if (!isA_SCNetworkReachability(target)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return if_index;
	}

	ReachabilityStoreInfo_init(&store_info);

	MUTEX_LOCK(&targetPrivate->lock);

	if (targetPrivate->scheduled) {
		// if being watched, return the last known (and what should be current) status
		goto done;
	}


	ok = __SCNetworkReachabilityGetFlags(&store_info, target, &targetPrivate->info, FALSE);

    done :

	/* Only return the if_index if the connection is reachable not for reachable connection
	 * required etc ... */
	if (ok && rankReachability(targetPrivate->info.flags) == 2) {
		if_index = targetPrivate->info.if_index;
	}

	MUTEX_UNLOCK(&targetPrivate->lock);
	ReachabilityStoreInfo_free(&store_info);
	return if_index;
}


Boolean
SCNetworkReachabilityGetFlags(SCNetworkReachabilityRef		target,
			      SCNetworkReachabilityFlags	*flags)
{
	Boolean				ok		= TRUE;
	ReachabilityStoreInfo		store_info;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	if (!isA_SCNetworkReachability(target)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	ReachabilityStoreInfo_init(&store_info);

	MUTEX_LOCK(&targetPrivate->lock);

	if (targetPrivate->scheduled) {
		// if being watched, return the last known (and what should be current) status
		*flags = targetPrivate->info.flags & kSCNetworkReachabilityFlagsMask;

		if (isReachabilityTypeName(targetPrivate->type) && targetPrivate->dnsNoAddressesSinceLastTimeout) {
			targetPrivate->needResolve = TRUE;
			ReachabilityInfo tmp_reach_info	= NOT_REACHABLE;
			__SCNetworkReachabilityGetFlags(&store_info, target, &tmp_reach_info, TRUE);
		}

		goto done;
	}


	ok = __SCNetworkReachabilityGetFlags(&store_info, target, &targetPrivate->info, FALSE);
	if (_sc_debug) {
		SCLog(TRUE, LOG_INFO, CFSTR("%s  flags     = 0x%08x"), targetPrivate->log_prefix, targetPrivate->info.flags);
	}

	*flags = targetPrivate->info.flags & kSCNetworkReachabilityFlagsMask;

    done :

	MUTEX_UNLOCK(&targetPrivate->lock);
	ReachabilityStoreInfo_free(&store_info);
	return ok;
}


#pragma mark -
#pragma mark Notifications


/*
 * __SCNetworkReachabilityHandleChanges
 *
 * Called to process network configuration changes and determine
 * if a reachability notification is warranted.
 * - should be exec'd on the _hn_changes_queue()
 */
static void
__SCNetworkReachabilityHandleChanges(SCDynamicStoreRef	store,
				     CFArrayRef		changedKeys,
				     void		*info)
{
	Boolean			dnsConfigChanged	= FALSE;
	CFIndex			i;
	Boolean			forcedChange		= FALSE;
	CFStringRef		key;
	Boolean			match;
	CFIndex			nChanges;
	CFIndex			nGlobals		= 0;
	CFIndex			nTargets;
	Boolean			neChanged		= FALSE;
	Boolean			networkConfigChanged	= FALSE;
	struct timeval		now;
	Boolean			onDemandConfigChanged	= FALSE;
#if	!TARGET_OS_IPHONE
	Boolean			powerStatusChanged	= FALSE;
#endif	// !TARGET_OS_IPHONE
	ReachabilityStoreInfo	store_info;
	const void *		targets_q[N_QUICK];
	const void **		targets			= targets_q;
	__block CFSetRef	watchers		= NULL;

	nChanges = CFArrayGetCount(changedKeys);
	if (nChanges == 0) {
		/* if no changes */
		return;
	}


	dispatch_sync(_hn_target_queue(), ^{
		/* grab the currently watched targets */
		if (hn_targets != NULL) {
			watchers = CFSetCreateCopy(NULL, hn_targets);
		}
	});

	nTargets = (watchers != NULL) ? CFSetGetCount(watchers) : 0;
	if (nTargets == 0) {
		/* if no addresses being monitored */
		goto done;
	}

	/* grab the current time */
	(void)gettimeofday(&now, NULL);

#if	!TARGET_OS_IPHONE
	match = CFArrayContainsValue(changedKeys, CFRangeMake(0, nChanges), power_changed_key);
	if (match) {
		/* handle "early" wake notification */
		nGlobals++;
		powerStatusChanged = TRUE;
	}
#endif	// !TARGET_OS_IPHONE

	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							 kSCDynamicStoreDomainState,
							 kSCEntNetDNS);
	match = CFArrayContainsValue(changedKeys, CFRangeMake(0, nChanges), key);
	CFRelease(key);
	if (match) {
		nGlobals++;
		dnsConfigChanged = TRUE;	/* the DNS server(s) have changed */
	}

	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							 kSCDynamicStoreDomainState,
							 kSCEntNetOnDemand);
	match = CFArrayContainsValue(changedKeys, CFRangeMake(0, nChanges), key);
	CFRelease(key);
	if (match) {
		nGlobals++;
		onDemandConfigChanged = TRUE;	/* the OnDemand configuration has changed */

		// force OnDemand configuration refresh (if SC notification arrives before BSD notify)
		__SCNetworkConnectionForceOnDemandConfigurationRefresh();
	}



	match = CFArrayContainsValue(changedKeys, CFRangeMake(0, nChanges), SCNETWORKREACHABILITY_TRIGGER_KEY);
	if (match) {
		nGlobals++;
		forcedChange = TRUE;		/* an SCDynamicStore driven "network" change */
	}

	if (nChanges > nGlobals) {
		networkConfigChanged = TRUE;
	}

	if (_sc_debug) {
		unsigned int		changes			= 0;
		static const char	*change_strings[]	= {
			// with no "power" status change
			"",						// 00000
			"network ",					// 00001
			"DNS ",						// 00010
			"network and DNS ",				// 00011
			"OnDemand ",					// 00100
			"network and OnDemand ",			// 00101
			"DNS and OnDemand ",				// 00110
			"network, DNS, and OnDemand ",			// 00111
			"NE ",						// 01000
			"network and NE ",				// 01001
			"DNS and NE ",					// 01010
			"network, DNS, and NE ",			// 01011
			"OnDemand and NE ",				// 01100
			"network, OnDemand, and NE ",			// 01101
			"DNS, OnDemand, and NE ",			// 01110
			"network, DNS, OnDemand, and NE ",		// 01111
#if	!TARGET_OS_IPHONE
			// with "power" status change
			"power",					// 10000
			"network and power ",				// 10001
			"DNS and power ",				// 10010
			"network, DNS, and power ",			// 10011
			"OnDemand and power ",				// 10100
			"network, OnDemand, and power ",		// 10101
			"DNS, OnDemand, and power ",			// 10110
			"network, DNS, OnDemand, and power ",		// 10111
			"NE and power ",				// 11000
			"network, NE, and power ",			// 11001
			"DNS, NE, and power ",				// 11010
			"network, DNS, NE, and power ",			// 11011
			"OnDemand, NE, and power ",			// 11100
			"network, OnDemand, NE, and power ",		// 11101
			"DNS, OnDemand, NE, and power ",		// 11110
			"network, DNS, OnDemand, NE, and power ",	// 11111
#endif	// !TARGET_OS_IPHONE
		};

#if	!TARGET_OS_IPHONE
		#define	PWR	16
		if (powerStatusChanged) {
			changes |= PWR;
		}
#endif	// !TARGET_OS_IPHONE

		#define	NE	8
		if (neChanged) {
			changes |= NE;
		}

		#define	VOD	4
		if (onDemandConfigChanged) {
			changes |= VOD;
		}

		#define	DNS	2
		if (dnsConfigChanged) {
			changes |= DNS;
		}

		#define	NET	1
		if (networkConfigChanged) {
			changes |= NET;
		}

		SCLog(TRUE, LOG_INFO,
		      CFSTR("process %s%s%sconfiguration change"),
		      forcedChange ? "[forced] " : "",
		      change_strings[changes]);
	}

	ReachabilityStoreInfo_init(&store_info);

	if (nTargets > (CFIndex)(sizeof(targets_q) / sizeof(CFTypeRef)))
		targets = CFAllocatorAllocate(NULL, nTargets * sizeof(CFTypeRef), 0);
	CFSetGetValues(watchers, targets);
	for (i = 0; i < nTargets; i++) {
		Boolean				dnsNeedsUpdate	= FALSE;
		SCNetworkReachabilityRef	target		= targets[i];
		SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

		MUTEX_LOCK(&targetPrivate->lock);


		if (dnsConfigChanged) {
			targetPrivate->last_dns = now;
		}

		if (networkConfigChanged) {
			targetPrivate->last_network = now;
		}

#if	!TARGET_OS_IPHONE
		if (powerStatusChanged) {
			targetPrivate->last_power = now;
		}
#endif	// !TARGET_OS_IPHONE

		if (isReachabilityTypeName(targetPrivate->type)) {
			Boolean		dnsChanged	= (dnsConfigChanged	 |
							   dnsNeedsUpdate	 |
							   onDemandConfigChanged |
							   neChanged);

			if (!dnsChanged) {
				/*
				 * if the DNS configuration didn't change we still need to
				 * check that the DNS servers are accessible.
				 */
				Boolean				ns_blocked	= FALSE;
				int				ns_dns_config	= -1;
				SCNetworkReachabilityFlags	ns_flags	= 0;
				uint32_t			ns_if_index	= 0;
				Boolean				ok;

				/* update the reachability of the DNS servers */
				ok = ReachabilityStoreInfo_update(&store_info, &store, AF_UNSPEC);
				if (ok) {
					_SC_R_updateResolverReachability(&store_info,
									 &ns_flags,
									 &targetPrivate->haveDNS,
									 targetPrivate->name,
									 targetPrivate->if_index,
									 &ns_if_index,
									 &ns_dns_config,
									 targetPrivate->log_prefix);
				} else {
					ns_flags = kSCNetworkReachabilityFlagsReachable;
					dnsChanged = TRUE;
				}


				if (rankReachability(ns_flags) < 2) {
					/*
					 * if DNS servers are not (or are no longer) reachable, set
					 * flags based on the availability of configured (but not
					 * active) services.
					 */
					SCLog(_sc_debug, LOG_INFO, CFSTR("%sDNS server(s) not available"),
					      targetPrivate->log_prefix);
					dnsChanged = TRUE;
				}

				if ((targetPrivate->dnsBlocked != ns_blocked) ||
				    (targetPrivate->resolverFlags != ns_flags)) {
					// if the DNS blocked or resolver reachability changed
					targetPrivate->dnsBlocked = ns_blocked;
					targetPrivate->resolverFlags = ns_flags;
					dnsChanged = TRUE;
				}
			}

			if (dnsChanged) {
				if (targetPrivate->dnsActive) {
					// if we have an outstanding [m]DNS query
					SCLog(_sc_debug, LOG_INFO,
					      CFSTR("%scancel [m]DNS query for name = %s"),
					      targetPrivate->log_prefix,
					      targetPrivate->name);
					dequeueDNSQuery(target);
				}

				/* schedule request to resolve the name again */
				targetPrivate->needResolve = TRUE;
			}
		}

		if (forcedChange) {
			targetPrivate->cycle++;
		}

		if (targetPrivate->scheduled) {
			__SCNetworkReachabilityUpdate(target);
		}

		MUTEX_UNLOCK(&targetPrivate->lock);
	}
	if (targets != targets_q)	CFAllocatorDeallocate(NULL, targets);

	ReachabilityStoreInfo_free(&store_info);

    done :

	if (watchers != NULL) CFRelease(watchers);
	return;
}


/*
 * __SCNetworkReachabilityHandleStoreChanges
 *
 * Called to process SCDynamicStore network configuration changes.
 * - should be exec'd on the _hn_changes_queue()
 */
static void
__SCNetworkReachabilityHandleStoreChanges(SCDynamicStoreRef	store,
					  CFArrayRef		changedKeys,
					  void			*info)
{
	nwi_state_t	nwi_state;

	if ((CFArrayGetCount(changedKeys) == 1) &&
	    CFArrayContainsValue(changedKeys, CFRangeMake(0, 1), SCNETWORKREACHABILITY_TRIGGER_KEY)) {
		goto update;
	}

	/* "something" [else] changed, start fresh */
	ReachabilityStoreInfo_save(NULL);

	nwi_state = nwi_state_copy();
	if (nwi_state != NULL) {
		// if we have some networking
		nwi_state_release(nwi_state);
		return;
	}

	// if no networking, use the [SC] changes to add/update
	// the kSCNetworkReachabilityFlagsConnectionRequired flag

    update :

	__SCNetworkReachabilityHandleChanges(store, changedKeys, info);
	return;
}


#if	!TARGET_OS_IPHONE

static Boolean
darkWakeNotify(SCNetworkReachabilityRef target)
{
	return FALSE;
}


static Boolean
systemIsAwake(IOPMSystemPowerStateCapabilities power_capabilities)
{
	if ((power_capabilities & POWER_CAPABILITIES_NETWORK) != POWER_CAPABILITIES_NETWORK) {
		// if we're not fully awake (from a networking point of view).
		return FALSE;
	}

	return TRUE;
}

#endif	// !TARGET_OS_IPHONE


static void
reachPerform(void *info)
{
	void				*context_info;
	void				(*context_release)(const void *);
	unsigned int			n;
	ReachabilityInfo		reach_info;
	SCNetworkReachabilityCallBack	rlsFunction;
	SCNetworkReachabilityRef	target		= (SCNetworkReachabilityRef)info;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	n = _SC_ATOMIC_ZERO(&targetPrivate->pending);
	if (_sc_debug && (n > 1)) {
		SCLog(TRUE, LOG_DEBUG,
		      CFSTR("%sdelivering SCNetworkReachability notifications (%u)"),
		      targetPrivate->log_prefix,
		      n);
	}

	MUTEX_LOCK(&targetPrivate->lock);

	if (!targetPrivate->scheduled) {
		// if no longer scheduled
		SCLog(_sc_debug, LOG_DEBUG,
		      CFSTR("%sskipping SCNetworkReachability callback, no longer scheduled"),
		      targetPrivate->log_prefix);
		MUTEX_UNLOCK(&targetPrivate->lock);
		return;
	}

	// capture current state
	memcpy(&reach_info, &targetPrivate->info, sizeof(ReachabilityInfo));

	// callout
	rlsFunction = targetPrivate->rlsFunction;
	if (targetPrivate->rlsContext.retain != NULL) {
		context_info	= (void *)(*targetPrivate->rlsContext.retain)(targetPrivate->rlsContext.info);
		context_release	= targetPrivate->rlsContext.release;
	} else {
		context_info	= targetPrivate->rlsContext.info;
		context_release	= NULL;
	}

	// update last notification info
	_reach_set(&targetPrivate->last_notify, &reach_info, targetPrivate->cycle, targetPrivate->if_index, targetPrivate->if_name);
	(void)gettimeofday(&targetPrivate->last_push, NULL);

	MUTEX_UNLOCK(&targetPrivate->lock);

	if (rlsFunction != NULL) {
		(*rlsFunction)(target,
			       reach_info.flags & kSCNetworkReachabilityFlagsMask,
			       context_info);
	}

	if (context_release != NULL) {
		(*context_release)(context_info);
	}

	return;
}


/*
 * reachUpdate
 *
 * - caller must *not* be holding the target lock
 * - caller must be running on the __SCNetworkReachability_concurrent_queue()
 */
static Boolean
reachUpdate(SCNetworkReachabilityRef target)
{
	uint64_t			cycle;
	Boolean				defer		= FALSE;
	Boolean				forced;
	Boolean				ok;
	ReachabilityInfo		reach_info	= NOT_REACHABLE;
	ReachabilityStoreInfo		store_info;
	Boolean				target_debug;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	target_debug = (_sc_debug && !targetPrivate->quiet);

	if (target_debug) {
		SCLog(TRUE, LOG_INFO, CFSTR("%schecking target reachability"),
		      targetPrivate->log_prefix);
	}


	MUTEX_LOCK(&targetPrivate->lock);

	if (!targetPrivate->scheduled) {
		// if not currently scheduled
		MUTEX_UNLOCK(&targetPrivate->lock);
		return FALSE;
	}

	/* update reachability, notify if status changed */
	ReachabilityStoreInfo_init(&store_info);
	ok = __SCNetworkReachabilityGetFlags(&store_info, target, &reach_info, TRUE);
	ReachabilityStoreInfo_free(&store_info);
	if (!ok) {
		/* if reachability status not available */
		if (target_debug) {
			SCLog(TRUE, LOG_INFO, CFSTR("%sflags not available"),
			      targetPrivate->log_prefix);
		}
		reach_info = NOT_REACHABLE;
	}

#if	!TARGET_OS_IPHONE
	/*
	 * We want to defer the notification if this is a maintenance wake *and*
	 * the reachability flags that we would be reporting to the application
	 * are better than those that we last reported.
	 */
	if (!systemIsAwake(power_capabilities)) {
		/* if this is a maintenace wake */
		reach_info.sleeping = TRUE;

		if (rankReachability(reach_info.flags) >= rankReachability(targetPrivate->info.flags)) {
			/*
			 * don't report the change if the new reachability flags are
			 * the same or "better"
			 */
			defer = !darkWakeNotify(target);
		} else if (!__reach_changed(&targetPrivate->last_notify, &reach_info)) {
			/*
			 * if we have already posted this change
			 */
			defer = !darkWakeNotify(target);
		}
	}
#endif	// !TARGET_OS_IPHONE

	cycle = targetPrivate->cycle;
	forced = ((cycle != 0) && (targetPrivate->info.cycle != cycle));

	/*
	 *	  (A)	  (B)	  (C)
	 *	 reach		resolve
	 *	changed	forced	pending
	 *	=======	=======	=======
	 *	   N	   N	   N	No change
	 *	   N	   N	   Y	No change
	 *	   N	   Y	   N	Change		(forced && !resolve pending)
	 *	   N	   Y	   Y	No change	(suppress forced w/resolve pending)
	 *	   Y	  N/A	  N/A	Change
	 *	   Y	  N/A	  N/A	Change
	 *	   Y	  N/A	  N/A	Change
	 *	   Y	  N/A	  N/A	Change
	 *
	 *	Change    == A || (B && !C)
	 *	No Change == !(A || (B && !C))
	 *	No Change == !A && !(B && !C)
	 *	No Change == !A && (!B || C)
	 *	No Change == (!B || C) && !A
	 */
	if ((!forced || (reach_info.flags == kSCNetworkReachabilityFlagsFirstResolvePending))
	    && !__reach_changed(&targetPrivate->info, &reach_info)) {
		if (target_debug) {
			if (targetPrivate->info.sleeping == reach_info.sleeping) {
				SCLog(TRUE, LOG_INFO,
				      CFSTR("%sflags/interface match (now 0x%08x/%u%s)%s%s"),
				      targetPrivate->log_prefix,
				      reach_info.flags,
				      reach_info.if_index,
				      reach_info.sleeping ? ", z" : "",
				      defer ? ", deferred" : "",
				      forced ? ", forced" : "");
			} else {
				SCLog(TRUE, LOG_INFO,
				      CFSTR("%sflags/interface equiv (was 0x%08x/%u%s, now 0x%08x/%u%s)%s%s"),
				      targetPrivate->log_prefix,
				      targetPrivate->info.flags,
				      targetPrivate->info.if_index,
				      targetPrivate->info.sleeping ? ", z" : "",
				      reach_info.flags,
				      reach_info.if_index,
				      reach_info.sleeping ? ", z" : "",
				      defer ? ", deferred" : "",
				      forced ? ", forced" : "");
			}
		}
		MUTEX_UNLOCK(&targetPrivate->lock);
		return FALSE;
	}

	if (target_debug) {
		SCLog(TRUE, LOG_INFO,
		      CFSTR("%sflags/interface have changed (was 0x%08x/%u%s, now 0x%08x/%u%s)%s%s"),
		      targetPrivate->log_prefix,
		      targetPrivate->info.flags,
		      targetPrivate->info.if_index,
		      targetPrivate->info.sleeping ? ", z" : "",
		      reach_info.flags,
		      reach_info.if_index,
		      reach_info.sleeping ? ", z" : "",
		      defer ? ", deferred" : "",
		      forced ? ", forced" : "");
	}

	/* update flags / interface */
	_reach_set(&targetPrivate->info, &reach_info, cycle, targetPrivate->if_index, targetPrivate->if_name);

	/* as needed, defer the notification */
	if (defer) {
		MUTEX_UNLOCK(&targetPrivate->lock);
		return FALSE;
	}

	MUTEX_UNLOCK(&targetPrivate->lock);

	return TRUE;
}


Boolean
SCNetworkReachabilitySetCallback(SCNetworkReachabilityRef	target,
				 SCNetworkReachabilityCallBack	callout,
				 SCNetworkReachabilityContext	*context)
{
	SCNetworkReachabilityPrivateRef	targetPrivate = (SCNetworkReachabilityPrivateRef)target;

	MUTEX_LOCK(&targetPrivate->lock);

	if (targetPrivate->rlsContext.release != NULL) {
		/* let go of the current context */
		(*targetPrivate->rlsContext.release)(targetPrivate->rlsContext.info);
	}

	targetPrivate->rlsFunction   			= callout;
	targetPrivate->rlsContext.info			= NULL;
	targetPrivate->rlsContext.retain		= NULL;
	targetPrivate->rlsContext.release		= NULL;
	targetPrivate->rlsContext.copyDescription	= NULL;
	if (context) {
		bcopy(context, &targetPrivate->rlsContext, sizeof(SCNetworkReachabilityContext));
		if (context->retain != NULL) {
			targetPrivate->rlsContext.info = (void *)(*context->retain)(context->info);
		}
	}

	MUTEX_UNLOCK(&targetPrivate->lock);

	return TRUE;
}


static CFStringRef
reachRLSCopyDescription(const void *info)
{
	SCNetworkReachabilityRef		target	= (SCNetworkReachabilityRef)info;

	return CFStringCreateWithFormat(NULL,
					NULL,
					CFSTR("<SCNetworkReachability RLS> {target = %p}"),
					target);
}


static Boolean
__SCNetworkReachabilityScheduleWithRunLoop(SCNetworkReachabilityRef	target,
					   CFRunLoopRef			runLoop,
					   CFStringRef			runLoopMode,
					   dispatch_queue_t		queue,
					   Boolean			onDemand)
{
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;
	Boolean				init		= FALSE;
	__block Boolean			ok		= FALSE;

	MUTEX_LOCK(&targetPrivate->lock);

	if ((targetPrivate->dispatchQueue != NULL) ||		// if we are already scheduled with a dispatch queue
	    ((queue != NULL) && targetPrivate->scheduled)) {	// if we are already scheduled on a CFRunLoop
		_SCErrorSet(kSCStatusInvalidArgument);
		goto done;
	}

	if (!targetPrivate->serverBypass) {
		if (!targetPrivate->serverActive) {

			ok = __SCNetworkReachabilityServer_targetAdd(target);
			if (!ok) {
				targetPrivate->serverBypass = TRUE;
			}
		}

		if (targetPrivate->serverActive) {
			if (targetPrivate->scheduled) {
				// if already scheduled
				goto watch;
			}

			ok = __SCNetworkReachabilityServer_targetSchedule(target);
			if (!ok) {
				SCLog(TRUE, LOG_DEBUG,
				      CFSTR("__SCNetworkReachabilityScheduleWithRunLoop _targetMonitor() failed"));
				_SCErrorSet(kSCStatusFailed);
				goto done;
			}

			goto watch;
		}
	}

	/* schedule the SCNetworkReachability did-something-change handler */

	dispatch_sync(_hn_target_queue(), ^{
		ok = FALSE;

		if (!onDemand && (hn_store == NULL)) {
			CFMutableArrayRef	keys;
			CFMutableArrayRef	patterns;
			Boolean			watch_dns_configuration		= FALSE;
			Boolean			watch_dns_changes		= FALSE;
			Boolean			watch_nwi			= FALSE;
			Boolean			watch_onDemand_networking	= FALSE;
#if	!TARGET_OS_IPHONE
			Boolean			watch_power			= FALSE;
#endif	// !TARGET_OS_IPHONE

			hn_store = SCDynamicStoreCreate(NULL,
							CFSTR("SCNetworkReachability"),
							__SCNetworkReachabilityHandleStoreChanges,
							NULL);
			if (hn_store == NULL) {
				SCLog(TRUE, LOG_ERR, CFSTR("SCDynamicStoreCreate() failed"));
				return;
			}

			ReachabilityStoreInfo_keys(&keys, &patterns);
			CFArrayAppendValue(keys, SCNETWORKREACHABILITY_TRIGGER_KEY);	// force posting reach change
			ok = SCDynamicStoreSetNotificationKeys(hn_store, keys, patterns);
			CFRelease(keys);
			CFRelease(patterns);
			if (!ok) {
				SCLog(TRUE, LOG_ERR, CFSTR("SCDynamicStoreSetNotificationKeys() failed"));
				CFRelease(hn_store);
				hn_store = NULL;
				return;
			}

			ok = SCDynamicStoreSetDispatchQueue(hn_store, _hn_changes_queue());
			if (!ok) {
				SCLog(TRUE, LOG_ERR, CFSTR("SCDynamicStoreSetDispatchQueue() failed"));
				CFRelease(hn_store);
				hn_store = NULL;
				return;
			}

			// watch for network information changes
			watch_nwi = nwi_refresh_enable(_hn_changes_queue());
			if (!watch_nwi) {
				goto fail;
			}

			// watch for DNS configuration (resolver reachability) changes
			watch_dns_configuration = dns_configuration_watch();
			if (!watch_dns_configuration) {
				goto fail;
			}

			// watch for changes affecting DNS queries
			watch_dns_changes = dns_refresh_enable(_hn_changes_queue());
			if (!watch_dns_changes) {
				goto fail;
			}

#if	!TARGET_OS_IPHONE
			// watch for power capabilities (sleep/wake) changes
			watch_power = power_refresh_enable(_hn_changes_queue());
			if (!watch_power) {
				goto fail;
			}
#endif	// !TARGET_OS_IPHONE

			// watch for OnDemand network changes
			watch_onDemand_networking = onDemand_refresh_enable(_hn_changes_queue());
			if (!watch_onDemand_networking) {
				goto fail;
			}



			hn_targets = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
			ReachabilityStoreInfo_enable(TRUE);

			goto scheduled;

		    fail :

			ok = FALSE;



			if (watch_onDemand_networking) {
				onDemand_refresh_disable();
			}

#if	!TARGET_OS_IPHONE
			if (watch_power) {
				power_refresh_disable();
			}
#endif	// !TARGET_OS_IPHONE

			if (watch_dns_changes) {
				dns_refresh_disable();
			}

			if (watch_dns_configuration) {
				dns_configuration_unwatch();
			}

			if (watch_nwi) {
				nwi_refresh_disable();
			}

			SCDynamicStoreSetDispatchQueue(hn_store, NULL);
			CFRelease(hn_store);
			hn_store = NULL;

			_SCErrorSet(kSCStatusFailed);

			return;
		}

	    scheduled :

		CFSetAddValue(hn_targets, target);

		ok = TRUE;
	});

	if (!ok) {
		goto done;
	}

    watch :

	if (!targetPrivate->scheduled) {
		CFRunLoopSourceContext	context = { 0				// version
						  , (void *)target		// info
						  , CFRetain			// retain
						  , CFRelease			// release
						  , reachRLSCopyDescription	// copyDescription
						  , CFEqual			// equal
						  , CFHash			// hash
						  , NULL			// schedule
						  , NULL			// cancel
						  , reachPerform		// perform
						  };

		if (runLoop != NULL) {
			targetPrivate->rls    = CFRunLoopSourceCreate(NULL, 0, &context);
			targetPrivate->rlList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		}

		if (isReachabilityTypeName(targetPrivate->type)) {
			/*
			 * we're now scheduled so let's ensure that we
			 * are starting with a clean slate before we
			 * resolve the name
			 */
			if (targetPrivate->resolvedAddresses != NULL) {
				CFRelease(targetPrivate->resolvedAddresses);
				targetPrivate->resolvedAddresses = NULL;
			}
			targetPrivate->resolvedError = NETDB_SUCCESS;
			targetPrivate->needResolve = TRUE;
			_reach_set(&targetPrivate->info,
				   &NOT_REACHABLE,
				   targetPrivate->info.cycle,
				   targetPrivate->if_index,
				   targetPrivate->if_name);
			targetPrivate->info.flags |= kSCNetworkReachabilityFlagsFirstResolvePending;
			_reach_set(&targetPrivate->serverInfo,
				   &NOT_REACHABLE,
				   targetPrivate->serverInfo.cycle,
				   targetPrivate->if_index,
				   targetPrivate->if_name);
			targetPrivate->serverInfo.flags |= kSCNetworkReachabilityFlagsFirstResolvePending;
		}


		targetPrivate->scheduled = TRUE;

		init = TRUE;
	}

	if (queue != NULL) {
		// retain dispatch queue
		dispatch_retain(queue);
		targetPrivate->dispatchQueue = queue;

		//
		// We've taken a reference to the client's dispatch_queue and we
		// want to hold on to that reference until we've processed any/all
		// notifications.  To facilitate this we create a group, dispatch
		// any notification blocks to via that group, and when the caller
		// has told us to stop the notifications (unschedule) we wait for
		// the group to empty and use the group's finalizer to release
		// our reference to the client's queue.
		//

		// make sure that we have group to track any async requests
		targetPrivate->dispatchGroup = dispatch_group_create();

		// retain the target ... and release it when the group is released
		CFRetain(target);
		dispatch_set_context(targetPrivate->dispatchGroup, (void *)target);
		dispatch_set_finalizer_f(targetPrivate->dispatchGroup, (dispatch_function_t)CFRelease);
	} else {
		if (!_SC_isScheduled(NULL, runLoop, runLoopMode, targetPrivate->rlList)) {
			/*
			 * if we do not already have host notifications scheduled with
			 * this runLoop / runLoopMode
			 */
			CFRunLoopAddSource(runLoop, targetPrivate->rls, runLoopMode);
		}

		_SC_schedule(target, runLoop, runLoopMode, targetPrivate->rlList);
	}

	if (init) {
		ReachabilityInfo	reach_info	= NOT_REACHABLE;
		ReachabilityStoreInfo	store_info;

		/*
		 * if we have yet to schedule SC notifications for this address
		 * - initialize current reachability status
		 */
		ReachabilityStoreInfo_init(&store_info);
		if (__SCNetworkReachabilityGetFlags(&store_info, target, &reach_info, TRUE)) {
			/*
			 * if reachability status available
			 * - set flags
			 * - schedule notification to report status via callback
			 */
			reach_info.flags |= (targetPrivate->info.flags & kSCNetworkReachabilityFlagsFirstResolvePending);
			_reach_set(&targetPrivate->info,
				   &reach_info,
				   targetPrivate->cycle,
				   targetPrivate->if_index,
				   targetPrivate->if_name);
			__SCNetworkReachabilityUpdate(target);
		} else {
			/* if reachability status not available, async lookup started */
			_reach_set(&targetPrivate->info,
				   &NOT_REACHABLE,
				   targetPrivate->cycle,
				   targetPrivate->if_index,
				   targetPrivate->if_name);
			_reach_set(&targetPrivate->serverInfo,
				   &NOT_REACHABLE,
				   targetPrivate->cycle,
				   targetPrivate->if_index,
				   targetPrivate->if_name);
		}
		ReachabilityStoreInfo_free(&store_info);
	}

	if (targetPrivate->onDemandServer != NULL) {
		__SCNetworkReachabilityScheduleWithRunLoop(targetPrivate->onDemandServer, runLoop, runLoopMode, queue, TRUE);
	}

	SCLog((_sc_debug && (_sc_log > 0)), LOG_INFO, CFSTR("%sscheduled"),
	      targetPrivate->log_prefix);

	ok = TRUE;

    done :

	MUTEX_UNLOCK(&targetPrivate->lock);
	return ok;
}


static Boolean
__SCNetworkReachabilityUnscheduleFromRunLoop(SCNetworkReachabilityRef	target,
					     CFRunLoopRef		runLoop,
					     CFStringRef		runLoopMode,
					     Boolean			onDemand)
{
	dispatch_group_t		drainGroup	= NULL;
	dispatch_queue_t		drainQueue	= NULL;
	CFIndex				n		= 0;
	Boolean				ok		= FALSE;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	// hold a reference while we unschedule
	CFRetain(target);

	MUTEX_LOCK(&targetPrivate->lock);

	if (((runLoop == NULL) && (targetPrivate->dispatchQueue == NULL)) ||	// if we should be scheduled on a dispatch queue (but are not)
	    ((runLoop != NULL) && (targetPrivate->dispatchQueue != NULL))) {	// if we should be scheduled on a CFRunLoop (but are not)
		_SCErrorSet(kSCStatusInvalidArgument);
		goto done;
	}

	if (!targetPrivate->scheduled) {
		// if not currently scheduled
		_SCErrorSet(kSCStatusInvalidArgument);
		goto done;
	}

	// unschedule the target specific sources
	if (targetPrivate->dispatchQueue != NULL) {
		if (targetPrivate->onDemandServer != NULL) {
			SCNetworkReachabilitySetCallback(targetPrivate->onDemandServer, NULL, NULL);
			__SCNetworkReachabilityUnscheduleFromRunLoop(targetPrivate->onDemandServer, NULL, NULL, TRUE);
		}

		// save dispatchQueue, release reference when we've queue'd blocks complete, allow re-scheduling
		drainGroup = targetPrivate->dispatchGroup;
		targetPrivate->dispatchGroup = NULL;
		drainQueue = targetPrivate->dispatchQueue;
		targetPrivate->dispatchQueue = NULL;
	} else {
		if (!_SC_unschedule(target, runLoop, runLoopMode, targetPrivate->rlList, FALSE)) {
			// if not currently scheduled
			_SCErrorSet(kSCStatusInvalidArgument);
			goto done;
		}

		if (targetPrivate->onDemandServer != NULL) {
			__SCNetworkReachabilityUnscheduleFromRunLoop(targetPrivate->onDemandServer, runLoop, runLoopMode, TRUE);
		}

		n = CFArrayGetCount(targetPrivate->rlList);
		if ((n == 0) || !_SC_isScheduled(NULL, runLoop, runLoopMode, targetPrivate->rlList)) {
			// if target is no longer scheduled for this runLoop / runLoopMode
			CFRunLoopRemoveSource(runLoop, targetPrivate->rls, runLoopMode);

			if (n == 0) {
				// if *all* notifications have been unscheduled
				if (targetPrivate->onDemandServer != NULL) {
					SCNetworkReachabilitySetCallback(targetPrivate->onDemandServer, NULL, NULL);
				}
				CFRelease(targetPrivate->rlList);
				targetPrivate->rlList = NULL;
				CFRunLoopSourceInvalidate(targetPrivate->rls);
				CFRelease(targetPrivate->rls);
				targetPrivate->rls = NULL;
			}
		}
	}

	if (n == 0) {
		//
		// Cancel our request for server monitoring
		//
		if (targetPrivate->serverActive) {
			ok = __SCNetworkReachabilityServer_targetUnschedule(target);
			if (!ok) {
				SCLog(TRUE, LOG_DEBUG,
				      CFSTR("__SCNetworkReachabilityUnscheduleFromRunLoop _targetMonitor() failed"));
				_SCErrorSet(kSCStatusFailed);
			}
		}

		// if *all* notifications have been unscheduled
		targetPrivate->scheduled = FALSE;
	}

	if (targetPrivate->serverActive) {
		goto unwatch;
	}

	if (n == 0) {
		if (targetPrivate->dnsActive) {
			// if we have an active [m]DNS query
			dequeueDNSQuery(target);
		}

		dispatch_sync(_hn_target_queue(), ^{
			CFSetRemoveValue(hn_targets, target);

			if (onDemand) {
				return;
			}

			if (CFSetGetCount(hn_targets) > 0) {
				return;
			}

			// if we are no longer monitoring any targets
			SCDynamicStoreSetDispatchQueue(hn_store, NULL);
			CFRelease(hn_store);
			hn_store = NULL;
			CFRelease(hn_targets);
			hn_targets = NULL;

			ReachabilityStoreInfo_enable(FALSE);
			ReachabilityStoreInfo_save(NULL);

			/*
			 * until we start monitoring again, ensure that
			 * any resources associated with watching network
			 * and configuration changes have been released.
			 */


			// OnDemand configuration
			onDemand_refresh_disable();

#if	!TARGET_OS_IPHONE
			// sleep/wake & power capabilities
			power_refresh_disable();
#endif	// !TARGET_OS_IPHONE

			// outstanding DNS queries
			dns_refresh_disable();

			// DNS configuration
			dns_configuration_unwatch();

			// nwi
			nwi_refresh_disable();
		});
	}

    unwatch :

	SCLog((_sc_debug && (_sc_log > 0)), LOG_INFO, CFSTR("%sunscheduled"),
	      targetPrivate->log_prefix);

	ok = TRUE;

    done :

	MUTEX_UNLOCK(&targetPrivate->lock);

	if (drainGroup != NULL) {
		dispatch_group_notify(drainGroup, __SCNetworkReachability_concurrent_queue(), ^{
			// release group/queue references
			dispatch_release(drainQueue);
			dispatch_release(drainGroup);	// releases our target reference
		});
	}

	// release our reference
	CFRelease(target);

	return ok;
}

Boolean
SCNetworkReachabilityScheduleWithRunLoop(SCNetworkReachabilityRef	target,
					 CFRunLoopRef			runLoop,
					 CFStringRef			runLoopMode)
{
	if (!isA_SCNetworkReachability(target) || (runLoop == NULL) || (runLoopMode == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	return __SCNetworkReachabilityScheduleWithRunLoop(target, runLoop, runLoopMode, NULL, FALSE);
}

Boolean
SCNetworkReachabilityUnscheduleFromRunLoop(SCNetworkReachabilityRef	target,
					   CFRunLoopRef			runLoop,
					   CFStringRef			runLoopMode)
{
	if (!isA_SCNetworkReachability(target) || (runLoop == NULL) || (runLoopMode == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	return __SCNetworkReachabilityUnscheduleFromRunLoop(target, runLoop, runLoopMode, FALSE);
}

Boolean
SCNetworkReachabilitySetDispatchQueue(SCNetworkReachabilityRef	target,
				      dispatch_queue_t		queue)
{
	Boolean	ok	= FALSE;

	if (!isA_SCNetworkReachability(target)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (queue != NULL) {
		ok = __SCNetworkReachabilityScheduleWithRunLoop(target, NULL, NULL, queue, FALSE);
	} else {
		ok = __SCNetworkReachabilityUnscheduleFromRunLoop(target, NULL, NULL, FALSE);
	}

	return ok;
}

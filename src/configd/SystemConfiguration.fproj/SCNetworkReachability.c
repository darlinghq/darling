/*
 * Copyright (c) 2003-2019 Apple Inc. All rights reserved.
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

#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <dispatch/dispatch.h>
#include <dispatch/private.h>
#include <pthread.h>
#include <libkern/OSAtomic.h>
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
#include <net/network_agent.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>

#define	SC_LOG_HANDLE		__log_SCNetworkReachability
#define SC_LOG_HANDLE_TYPE	static
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCValidation.h>
#include <SystemConfiguration/SCPrivate.h>
#include "SCD.h"
#include "SCNetworkReachabilityInternal.h"



#include <nw/private.h>

#define	DEBUG_REACHABILITY_TYPE_NAME			"create w/name"
#define	DEBUG_REACHABILITY_TYPE_NAME_OPTIONS		"    + options"

#define	DEBUG_REACHABILITY_TYPE_ADDRESS			"create w/address"
#define	DEBUG_REACHABILITY_TYPE_ADDRESS_OPTIONS		"       + options"

#define	DEBUG_REACHABILITY_TYPE_ADDRESSPAIR		"create w/address pair"
#define	DEBUG_REACHABILITY_TYPE_ADDRESSPAIR_OPTIONS	"            + options"

#define	DEBUG_REACHABILITY_TYPE_PTR			"create w/ptr"
#define	DEBUG_REACHABILITY_TYPE_PTR_OPTIONS		"   + options"

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


#define	N_QUICK	64

#define REACHABILITY_NETWORK_EXTENSION_AGENT_DOMAIN "NetworkExtension"
#define REACHABILITY_AGENT_DATA_KEY "data"


static CFStringRef	__SCNetworkReachabilityCopyDescription	(CFTypeRef cf);
static void		__SCNetworkReachabilityDeallocate	(CFTypeRef cf);

static SCNetworkReachabilityFlags
__SCNetworkReachabilityGetFlagsFromPath(nw_path_t			path,
					ReachabilityAddressType		type,
					nw_resolver_status_t		resolverStatus,
					nw_array_t			resolvedEndpoints,
					Boolean				resolvedEndpointUseFlags,
					SCNetworkReachabilityFlags	resolvedEndpointFlags);

static Boolean
__SCNetworkReachabilitySetDispatchQueue(SCNetworkReachabilityPrivateRef	targetPrivate,
					dispatch_queue_t		queue);

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

static dispatch_queue_t
_callback_queue()
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create("SCNetworkReachability.callback", NULL);
	});

	return q;
}

static os_log_t
__log_SCNetworkReachability(void)
{
	static os_log_t	log	= NULL;

	if (log == NULL) {
		log = os_log_create("com.apple.SystemConfiguration", "SCNetworkReachability");
	}

	return log;
}


#pragma mark -
#pragma mark SCNetworkReachability APIs


static __inline__ CFTypeRef
isA_SCNetworkReachability(CFTypeRef obj)
{
	return (isA_CFType(obj, SCNetworkReachabilityGetTypeID()));
}

static CFStringRef
_SCNetworkReachabilityCopyTargetDescription(SCNetworkReachabilityRef target)
{
	CFAllocatorRef			allocator	= CFGetAllocator(target);
	CFMutableStringRef		str;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	str = CFStringCreateMutable(allocator, 0);
	switch (targetPrivate->type) {
		case reachabilityTypeAddress :
		case reachabilityTypeAddressPair : {
			char buf[64];
			if (targetPrivate->localAddressEndpoint != NULL) {
				_SC_sockaddr_to_string(nw_endpoint_get_address(targetPrivate->localAddressEndpoint), buf, sizeof(buf));
				CFStringAppendFormat(str, NULL, CFSTR("local address = %s"),
						     buf);
			}
			if (targetPrivate->remoteAddressEndpoint != NULL) {
				_SC_sockaddr_to_string(nw_endpoint_get_address(targetPrivate->remoteAddressEndpoint), buf, sizeof(buf));
				CFStringAppendFormat(str, NULL, CFSTR("%s%saddress = %s"),
						     targetPrivate->localAddressEndpoint ? ", " : "",
						     (targetPrivate->type == reachabilityTypeAddressPair) ? "remote " : "",
						     buf);
			} else {
				CFStringAppendFormat(str, NULL, CFSTR("default path"));
			}
			break;
		}
		case reachabilityTypeName : {
			CFStringAppendFormat(str, NULL, CFSTR("name = %s"), nw_endpoint_get_hostname(targetPrivate->hostnameEndpoint));
			break;
		}
		case reachabilityTypePTR : {
			char buf[64];
			if (targetPrivate->remoteAddressEndpoint != NULL) {
				_SC_sockaddr_to_string(nw_endpoint_get_address(targetPrivate->remoteAddressEndpoint), buf, sizeof(buf));
				CFStringAppendFormat(str, NULL, CFSTR("ptr = %s"),
						     buf);
			}
			break;
		}
	}

	if (targetPrivate->parameters != NULL) {
		unsigned int	if_index;

		if_index = nw_parameters_get_required_interface_index(targetPrivate->parameters);
		if (if_index != 0) {
			CFStringAppendFormat(str, NULL, CFSTR(", if_index = %u"), if_index);
		}
	}

	return str;
}


static CFStringRef
__SCNetworkReachabilityCopyTargetFlags(SCNetworkReachabilityRef target)
{
	CFAllocatorRef			allocator	= CFGetAllocator(target);
	CFStringRef			str;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	str = CFStringCreateWithFormat(allocator,
				       NULL,
				       CFSTR("flags = 0x%08x, if_index = %u"),
				       __SCNetworkReachabilityGetFlagsFromPath(targetPrivate->lastPath,
									       targetPrivate->type,
									       targetPrivate->lastResolverStatus,
									       targetPrivate->lastResolvedEndpoints,
									       targetPrivate->lastResolvedEndpointHasFlags,
									       targetPrivate->lastResolvedEndpointFlags),
				       targetPrivate->lastResolvedEndpointHasFlags ? targetPrivate->lastResolvedEndpointInterfaceIndex
										   : nw_path_get_interface_index(targetPrivate->lastPath));
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

	MUTEX_LOCK(&targetPrivate->lock);

	result = CFStringCreateMutable(allocator, 0);
	CFStringAppendFormat(result, NULL, CFSTR("<SCNetworkReachability %p [%p]> {"), cf, allocator);

	// add target description
	str = _SCNetworkReachabilityCopyTargetDescription(target);
	CFStringAppend(result, str);
	CFRelease(str);

	// add additional "name" info
	if (isReachabilityTypeName(targetPrivate->type)) {
		if (targetPrivate->resolver && targetPrivate->lastResolverStatus == nw_resolver_status_invalid) {
			CFStringAppendFormat(result, NULL, CFSTR(" (DNS query active)"));
		} else if (targetPrivate->lastResolverStatus != nw_resolver_status_invalid) {
			CFStringAppendFormat(result, NULL, CFSTR(" (%s"), (targetPrivate->lastResolverStatus == nw_resolver_status_complete) ? "complete" : "in progress");
			if (nw_array_get_count(targetPrivate->lastResolvedEndpoints) > 0) {
				nw_array_apply(targetPrivate->lastResolvedEndpoints, ^bool(size_t index, nw_object_t object) {
#pragma unused(index)
					nw_endpoint_t endpoint = (nw_endpoint_t)object;
					nw_endpoint_type_t endpoint_type = nw_endpoint_get_type(endpoint);
					if (endpoint_type == nw_endpoint_type_address) {
						char buf[64];
						const struct sockaddr	*sa = nw_endpoint_get_address(endpoint);
						_SC_sockaddr_to_string(sa, buf, sizeof(buf));
						CFStringAppendFormat(result, NULL, CFSTR(", %s"), buf);
					} else if (endpoint_type == nw_endpoint_type_host) {
						CFStringAppendFormat(result, NULL, CFSTR(", %s"), nw_endpoint_get_hostname(endpoint));
					} else {
						CFStringAppendFormat(result, NULL, CFSTR(", unexpected nw_endpoint type: %d"), endpoint_type);
					}
					return TRUE;
				});
			} else {
				CFStringAppendFormat(result, NULL, CFSTR(", no addresses"));
			}
			CFStringAppendFormat(result, NULL, CFSTR(")"));
		}
	}

	if (targetPrivate->resolverBypass) {
		CFStringAppendFormat(result, NULL, CFSTR(", !resolve"));
	}

	// add flags
	if (targetPrivate->scheduled) {
		str = __SCNetworkReachabilityCopyTargetFlags(target);
		CFStringAppendFormat(result, NULL, CFSTR(", %@"), str);
		CFRelease(str);
	}

	CFStringAppendFormat(result, NULL, CFSTR("}"));

	MUTEX_UNLOCK(&targetPrivate->lock);

	return result;
}


static void
__SCNetworkReachabilityDeallocate(CFTypeRef cf)
{
	SCNetworkReachabilityRef	target		= (SCNetworkReachabilityRef)cf;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	SC_log(LOG_DEBUG, "%srelease", targetPrivate->log_prefix);

	/* release resources */
	MUTEX_LOCK(&targetPrivate->lock);
	targetPrivate->scheduled = FALSE;

	if (targetPrivate->hostnameEndpoint) {
		nw_release(targetPrivate->hostnameEndpoint);
		targetPrivate->hostnameEndpoint = NULL;
	}
	if (targetPrivate->localAddressEndpoint) {
		nw_release(targetPrivate->localAddressEndpoint);
		targetPrivate->localAddressEndpoint = NULL;
	}
	if (targetPrivate->remoteAddressEndpoint) {
		nw_release(targetPrivate->remoteAddressEndpoint);
		targetPrivate->remoteAddressEndpoint = NULL;
	}
	if (targetPrivate->parameters) {
		nw_release(targetPrivate->parameters);
		targetPrivate->parameters = NULL;
	}
	if (targetPrivate->lastPath) {
		nw_release(targetPrivate->lastPath);
		targetPrivate->lastPath = NULL;
	}
	if (targetPrivate->lastPathParameters) {
		nw_release(targetPrivate->lastPathParameters);
		targetPrivate->lastPathParameters = NULL;
	}
	if (targetPrivate->lastResolvedEndpoints) {
		nw_release(targetPrivate->lastResolvedEndpoints);
		targetPrivate->lastResolvedEndpoints = NULL;
	}

	if (targetPrivate->rlsContext.release != NULL) {
		(*targetPrivate->rlsContext.release)(targetPrivate->rlsContext.info);
	}

	MUTEX_UNLOCK(&targetPrivate->lock);

	pthread_mutex_destroy(&targetPrivate->lock);

	return;
}


static void
__SCNetworkReachabilityInitialize(void)
{
	__kSCNetworkReachabilityTypeID = _CFRuntimeRegisterClass(&__SCNetworkReachabilityClass);

	pthread_mutexattr_init(&lock_attr);
	pthread_mutexattr_settype(&lock_attr, PTHREAD_MUTEX_ERRORCHECK);

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

	/* initialize non-zero/NULL members */
	MUTEX_INIT(&targetPrivate->lock);
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
				} else if (!warned) {
					SC_log(LOG_WARNING, "SCNetworkReachabilityCreateWithAddress[Pair] called with \"struct sockaddr *\" len %d < %zu",
					      address->sa_len,
					      sizeof(struct sockaddr_in));
					warned = TRUE;
				}
				break;
			case AF_INET6 :
				if (address->sa_len >= sizeof(struct sockaddr_in6)) {
					valid = address;
				} else if (!warned) {
					SC_log(LOG_WARNING, "SCNetworkReachabilityCreateWithAddress[Pair] called with \"struct sockaddr *\" len %d < %zu",
					      address->sa_len,
					      sizeof(struct sockaddr_in6));
					warned = TRUE;
				}
				break;
			default :
				if (!warned) {
					SC_log(LOG_WARNING, "SCNetworkReachabilityCreateWithAddress[Pair] called with invalid address family %d",
					      address->sa_family);
					warned = TRUE;
				}
		}
	}

	return valid;
}

static bool
__SCNetworkReachabilityAddressIsEmpty(const struct sockaddr *address)
{
	if (address == NULL) {
		return TRUE;
	}

	if (address->sa_family == AF_INET) {
		return (((struct sockaddr_in *)(void *)address)->sin_addr.s_addr == 0);
	} else if (address->sa_family == AF_INET6) {
		return IN6_IS_ADDR_UNSPECIFIED(&((struct sockaddr_in6 *)(void *)address)->sin6_addr);
	} else {
		return FALSE;
	}
}

SCNetworkReachabilityRef
SCNetworkReachabilityCreateWithAddress(CFAllocatorRef		allocator,
				       const struct sockaddr	*address)
{
	const struct sockaddr		*targetAddress;
	SCNetworkReachabilityPrivateRef	targetPrivate;

	targetAddress = is_valid_address(address);
	if (targetAddress == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	targetPrivate = __SCNetworkReachabilityCreatePrivate(allocator);
	if (targetPrivate == NULL) {
		return NULL;
	}

	targetPrivate->type = reachabilityTypeAddress;

	if (!__SCNetworkReachabilityAddressIsEmpty(targetAddress)) {
		targetPrivate->remoteAddressEndpoint = nw_endpoint_create_address(targetAddress);
	}

	SC_log(LOG_DEBUG, "%s%s %@",
	       targetPrivate->log_prefix,
	       DEBUG_REACHABILITY_TYPE_ADDRESS,
	       targetPrivate);

	return (SCNetworkReachabilityRef)targetPrivate;
}


#if	!TARGET_OS_IPHONE
static Boolean
is_ipv4_loopback(const struct sockaddr *sa)
{
	uint32_t		addr;
	struct sockaddr_in	*sin	= (struct sockaddr_in *)(void *)sa;

	if ((sa == NULL) ||
	    (sa->sa_len < sizeof(struct sockaddr_in)) ||
	    (sa->sa_family != AF_INET)) {
		return FALSE;
	}

	addr = ntohl(sin->sin_addr.s_addr);
	return IN_LOOPBACK(addr) ? TRUE : FALSE;
}
#endif	// !TARGET_OS_IPHONE


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

#if	!TARGET_OS_IPHONE
	// Check/fix for loopback IP --> remote IP (rdar://26561383)
	if ((localAddress != NULL) && (remoteAddress != NULL) &&
	    is_ipv4_loopback(localAddress) && !is_ipv4_loopback(remoteAddress)) {
		static Boolean	warned	= FALSE;

		if (!warned) {
			SC_log(LOG_WARNING, "BUG: SCNetworkReachabilityCreateWithAddressPair() called with local <loopback>");
			SC_log(LOG_WARNING, "address and remote <non-loopback> address. To return the expected (but actually");
			SC_log(LOG_WARNING, "incorrect) result, switched to SCNetworkReachabilityCreateWithAddress() with");
			SC_log(LOG_WARNING, "the remote address.");
			warned = TRUE;
		}

		return SCNetworkReachabilityCreateWithAddress(allocator, remoteAddress);
	}
#endif	// !TARGET_OS_IPHONE

	targetPrivate = __SCNetworkReachabilityCreatePrivate(allocator);
	if (targetPrivate == NULL) {
		return NULL;
	}

	targetPrivate->type = reachabilityTypeAddressPair;

	if (localAddress != NULL) {
		targetPrivate->localAddressEndpoint = nw_endpoint_create_address(localAddress);
	}

	if (remoteAddress != NULL) {
		if (is_same_address(localAddress, remoteAddress)) {
			targetPrivate->remoteAddressEndpoint = nw_retain(targetPrivate->localAddressEndpoint);
		} else {
			targetPrivate->remoteAddressEndpoint = nw_endpoint_create_address(remoteAddress);
		}
	}

	targetPrivate->parameters = nw_parameters_create();
	nw_parameters_set_local_address(targetPrivate->parameters, targetPrivate->localAddressEndpoint);

	SC_log(LOG_DEBUG, "%s%s %@",
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

	if (_SC_string_to_sockaddr(nodename, AF_UNSPEC, (void *)&addr, sizeof(addr)) != NULL) {
		/* if this "nodename" is really an IP[v6] address in disguise */
		return SCNetworkReachabilityCreateWithAddress(allocator, &addr.sa);
	}

	targetPrivate = __SCNetworkReachabilityCreatePrivate(allocator);
	if (targetPrivate == NULL) {
		return NULL;
	}

	targetPrivate->type = reachabilityTypeName;

	targetPrivate->hostnameEndpoint = nw_endpoint_create_host(nodename, "0");

	SC_log(LOG_DEBUG, "%s%s %@",
	       targetPrivate->log_prefix,
	       DEBUG_REACHABILITY_TYPE_NAME,
	       targetPrivate);

	return (SCNetworkReachabilityRef)targetPrivate;
}


static SCNetworkReachabilityRef
__SCNetworkReachabilityCreateWithPTR(CFAllocatorRef		allocator,
				     const struct sockaddr	*ptrAddress)
{
	SCNetworkReachabilityPrivateRef	targetPrivate;

	ptrAddress = is_valid_address(ptrAddress);
	if (ptrAddress == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	targetPrivate = __SCNetworkReachabilityCreatePrivate(allocator);
	if (targetPrivate == NULL) {
		return NULL;
	}

	targetPrivate->type = reachabilityTypePTR;

	targetPrivate->remoteAddressEndpoint = nw_endpoint_create_address(ptrAddress);

	targetPrivate->parameters = nw_parameters_create();
	nw_parameters_set_resolve_ptr(targetPrivate->parameters, TRUE);

	SC_log(LOG_DEBUG, "%s%s %@",
	       targetPrivate->log_prefix,
	       DEBUG_REACHABILITY_TYPE_PTR,
	       targetPrivate);

	return (SCNetworkReachabilityRef)targetPrivate;
}

SCNetworkReachabilityRef
SCNetworkReachabilityCreateWithOptions(CFAllocatorRef	allocator,
				       CFDictionaryRef	options)
{
	const struct sockaddr		*addr_l		= NULL;
	const struct sockaddr		*addr_p		= NULL;
	const struct sockaddr		*addr_r		= NULL;
	CFDataRef			data;
	Boolean				haveOpt		= FALSE;
	CFStringRef			interface	= NULL;
	CFStringRef			nodename;
	CFBooleanRef			resolverBypass;
	SCNetworkReachabilityRef	target;
	SCNetworkReachabilityPrivateRef	targetPrivate;
	unsigned int			if_index = 0;
	char				if_name[IFNAMSIZ];
	CFDataRef			sourceAppAuditToken	= NULL;
	CFStringRef			sourceAppBundleID	= NULL;

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
		if (!isA_CFData(data) || ((size_t)CFDataGetLength(data) < sizeof(struct sockaddr_in))) {
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}
		addr_l = (const struct sockaddr *)CFDataGetBytePtr(data);
	}
	data = CFDictionaryGetValue(options, kSCNetworkReachabilityOptionPTRAddress);
	if (data != NULL) {
		if (!isA_CFData(data) || ((size_t)CFDataGetLength(data) < sizeof(struct sockaddr_in))) {
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}
		addr_p = (const struct sockaddr *)CFDataGetBytePtr(data);
	}
	data = CFDictionaryGetValue(options, kSCNetworkReachabilityOptionRemoteAddress);
	if (data != NULL) {
		if (!isA_CFData(data) || ((size_t)CFDataGetLength(data) < sizeof(struct sockaddr_in))) {
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
	resolverBypass = CFDictionaryGetValue(options, kSCNetworkReachabilityOptionResolverBypass);
	if ((resolverBypass != NULL) && !isA_CFBoolean(resolverBypass)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}
	sourceAppAuditToken =
		CFDictionaryGetValue(options, kSCNetworkReachabilityOptionSourceAppAuditToken);
	if ((sourceAppAuditToken != NULL) &&
		(!isA_CFData(sourceAppAuditToken) ||
		(CFDataGetLength(sourceAppAuditToken) != sizeof(audit_token_t)))) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}
	sourceAppBundleID =
		CFDictionaryGetValue(options, kSCNetworkReachabilityOptionSourceAppBundleIdentifier);
	if ((sourceAppBundleID != NULL) &&
		(!isA_CFString(sourceAppBundleID) ||
		(CFStringGetLength(sourceAppBundleID) == 0))) {
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
		if ((addr_l != NULL) ||					// can't have PTR and target address
		    (addr_r != NULL)) {
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}

		target = __SCNetworkReachabilityCreateWithPTR(NULL, addr_p);
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
					     if_name,
					     sizeof(if_name),
					     kCFStringEncodingASCII) == NULL) ||
			((if_index = if_nametoindex(if_name)) == 0)) {
			CFRelease(targetPrivate);
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}
	}

	if (targetPrivate->parameters == NULL) {
		targetPrivate->parameters = nw_parameters_create();
	}

	if (if_index != 0) {
		nw_interface_t interfaceObject = nw_interface_create_with_index(if_index);
		nw_parameters_require_interface(targetPrivate->parameters, interfaceObject);
		nw_release(interfaceObject);
		haveOpt = TRUE;
	}

	if (resolverBypass != NULL) {
		targetPrivate->resolverBypass = CFBooleanGetValue(resolverBypass);
		haveOpt = TRUE;
	}

	if (sourceAppAuditToken != NULL) {
		audit_token_t atoken;
		CFDataGetBytes(sourceAppAuditToken,
				CFRangeMake(0, CFDataGetLength(sourceAppAuditToken)),
				(UInt8 *)&atoken);
		nw_parameters_set_source_application(targetPrivate->parameters, atoken);
		haveOpt = TRUE;
	} else if (sourceAppBundleID != NULL) {
		char *cBundleID = _SC_cfstring_to_cstring(sourceAppBundleID,
								NULL,
								0,
								kCFStringEncodingUTF8);
		if (cBundleID != NULL) {
			nw_parameters_set_source_application_by_bundle_id(targetPrivate->parameters,
									  cBundleID);
			CFAllocatorDeallocate(NULL, (void *)cBundleID);
		} else {
			SC_log(LOG_WARNING, "failed to convert %@ to a C string", sourceAppBundleID);
		}
		haveOpt = TRUE;
	}

	if (haveOpt) {
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

		SC_log(LOG_DEBUG, "%s%s %@",
		       targetPrivate->log_prefix,
		       opt,
		       targetPrivate);
	}

	return (SCNetworkReachabilityRef)targetPrivate;
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
	CFMutableArrayRef		array		= NULL;
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
		*error_num = 0;
	}

	MUTEX_LOCK(&targetPrivate->lock);

	if (nw_array_get_count(targetPrivate->lastResolvedEndpoints) > 0) {
		array = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		nw_array_apply(targetPrivate->lastResolvedEndpoints, ^bool(size_t index, nw_object_t object) {
#pragma unused(index)
			nw_endpoint_type_t endpoint_type = nw_endpoint_get_type((nw_endpoint_t)object);
			if (endpoint_type == nw_endpoint_type_address) {
				const struct sockaddr *address = nw_endpoint_get_address((nw_endpoint_t)object);
				if (address == NULL) {
					SC_log(LOG_ERR, "nw_endpoint_type_address w/no address");
					return TRUE;
				}

				CFDataRef addressData = CFDataCreate(kCFAllocatorDefault, (const uint8_t *)address, address->sa_len);
				CFArrayAppendValue(array, addressData);
				CFRelease(addressData);
			} else if (endpoint_type == nw_endpoint_type_host) {
				const char *host = nw_endpoint_get_hostname((nw_endpoint_t)object);
				if (host == NULL) {
					SC_log(LOG_ERR, "nw_endpoint_type_host w/no host");
					return TRUE;
				}

				CFStringRef string = CFStringCreateWithCString(kCFAllocatorDefault, host, kCFStringEncodingASCII);
				if (string == NULL) {
					SC_log(LOG_ERR, "nw_endpoint_type_host w/non-ASCII host");
					return TRUE;
				}

				if (CFStringHasPrefix(string,  CFSTR(".")) || CFStringHasSuffix(string,  CFSTR("."))) {
					CFMutableStringRef mutableString = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, string);
					CFRelease(string);
					string = mutableString;
					CFStringTrim(mutableString, CFSTR("."));
				}
				CFArrayAppendValue(array, string);
				CFRelease(string);
			} else {
				SC_log(LOG_ERR, "unexpected nw_endpoint type: %d", endpoint_type);
			}
			return TRUE;
		});
	}

	MUTEX_UNLOCK(&targetPrivate->lock);
	_SCErrorSet(kSCStatusOK);
	return array;
}

#pragma mark -
#pragma mark Reachability Flags

static void
__SCNetworkReachabilityGetAgentVPNFlags(xpc_object_t dictionary, Boolean *vpn, Boolean *onDemand)
{
	const struct netagent *agent = NULL;
	size_t length = 0;

	if (dictionary == NULL || vpn == NULL || onDemand == NULL) {
		return;
	}

	*vpn = FALSE;
	*onDemand = FALSE;

	agent = xpc_dictionary_get_data(dictionary, REACHABILITY_AGENT_DATA_KEY, &length);
	if (agent == NULL || length < sizeof(struct netagent) || length != (sizeof(struct netagent) + agent->netagent_data_size)) {
		return;
	}

	if (strncmp(REACHABILITY_NETWORK_EXTENSION_AGENT_DOMAIN, agent->netagent_domain, NETAGENT_DOMAINSIZE) == 0) {
		*vpn = TRUE;
		if ((agent->netagent_flags & NETAGENT_FLAG_VOLUNTARY) &&
		    !(agent->netagent_flags & NETAGENT_FLAG_ACTIVE)) {
			*onDemand = TRUE;
		}
	}
}

static bool
nw_path_is_linklocal_direct(nw_path_t path)
{
	bool is_linklocal_direct = false;

	nw_endpoint_t endpoint = nw_path_copy_endpoint(path);
	if (endpoint == NULL) {
		return false;
	}

	if (nw_endpoint_get_type(endpoint) == nw_endpoint_type_address) {
		const struct sockaddr_in *sin;

		sin = (const struct sockaddr_in *)(void *)nw_endpoint_get_address(endpoint);;
		if ((sin != NULL) &&
		    (sin->sin_family == AF_INET) &&
		    IN_LINKLOCAL(ntohl(sin->sin_addr.s_addr))) {
			nw_interface_t interface = nw_path_copy_interface(path);

			if (interface != NULL) {
				nw_interface_type_t type = nw_interface_get_type(interface);
				if ((type == nw_interface_type_wired) ||
				    ((type == nw_interface_type_wifi) &&
				     (nw_interface_get_subtype(interface) != nw_interface_subtype_wifi_awdl))) {
					is_linklocal_direct = true;
				}

				nw_release(interface);
			}
		}
	}

	nw_release(endpoint);
	return is_linklocal_direct;
}

static SCNetworkReachabilityFlags
__SCNetworkReachabilityGetFlagsFromPath(nw_path_t			path,
					ReachabilityAddressType		type,
					nw_resolver_status_t		resolverStatus,
					nw_array_t			resolvedEndpoints,
					Boolean				resolvedEndpointUseFlags,
					SCNetworkReachabilityFlags	resolvedEndpointFlags)
{
	__block SCNetworkReachabilityFlags flags = kSCNetworkReachabilityFlagsReachable;
	__block const char *why = "???";
	if (path != NULL) {
		nw_path_status_t status = nw_path_get_status(path);
		if (status == nw_path_status_satisfied) {
			__block bool checkDNSFlags = TRUE;
			flags = kSCNetworkReachabilityFlagsReachable;
			why = "nw_path_status_satisfied";
#if	TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
			if (nw_path_uses_interface_type(path, nw_interface_type_cellular)) {
				flags |= (kSCNetworkReachabilityFlagsTransientConnection | kSCNetworkReachabilityFlagsIsWWAN);
				why = "nw_path_status_satisfied, cellular";
			}
#endif	// TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
			xpc_object_t agent_dictionary = nw_path_copy_netagent_dictionary(path);
			if (agent_dictionary != NULL) {
				if (xpc_dictionary_get_count(agent_dictionary) > 0) {
					xpc_dictionary_apply(agent_dictionary, ^bool(const char *key, xpc_object_t value) {
#pragma unused(key)
						Boolean vpn = FALSE;
						Boolean onDemand = FALSE;
						__SCNetworkReachabilityGetAgentVPNFlags(value, &vpn, &onDemand);
						if (vpn) {
							// VPN flows are transient
							flags |= kSCNetworkReachabilityFlagsTransientConnection;
							why = "nw_path_status_satisfied, VPN";
						}
						if (onDemand &&
						    type == reachabilityTypeName &&
						    resolverStatus == nw_resolver_status_complete &&
						    nw_array_get_count(resolvedEndpoints) == 0) {
							// On Demand by hostname, when no address has been resolved
							flags |= (kSCNetworkReachabilityFlagsConnectionRequired | kSCNetworkReachabilityFlagsConnectionOnDemand);
							why = "nw_path_status_satisfied, OnDemand";
							checkDNSFlags = FALSE;
						}
						return TRUE;
					});

				}
				xpc_release(agent_dictionary);
			}
			if (isReachabilityTypeName(type)) {
				if (checkDNSFlags) {
					if (resolverStatus == nw_resolver_status_complete &&
					    nw_array_get_count(resolvedEndpoints) == 0) {
						// DNS didn't resolve, as a final answer for now. Not reachable!
						flags = 0;
						why = "nw_path_status_satisfied, DNS not reachable";
					} else if (resolvedEndpointUseFlags) {
						flags = resolvedEndpointFlags;
						why = "nw_path_status_satisfied, resolved endpoint flags";
					}
				}
			} else {
				if (nw_path_is_direct(path) || nw_path_is_linklocal_direct(path)) {
					flags |= kSCNetworkReachabilityFlagsIsDirect;
					why = "nw_path_status_satisfied, by address, direct";
				}
				if (nw_path_is_local(path)) {
					flags |= kSCNetworkReachabilityFlagsIsLocalAddress;
					why = "nw_path_status_satisfied, by address, local";
				}
			}
		} else if (status == nw_path_status_unsatisfied) {
			flags = 0;
			why = "nw_path_status_unsatisfied";
#if	TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
			if (nw_path_uses_interface_type(path, nw_interface_type_cellular)) {
				flags |= kSCNetworkReachabilityFlagsIsWWAN;
				why = "nw_path_status_unsatisfied, WWAN";
			}
#endif	// TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
		} else if (status == nw_path_status_satisfiable) {
			flags = (kSCNetworkReachabilityFlagsReachable | kSCNetworkReachabilityFlagsConnectionRequired | kSCNetworkReachabilityFlagsTransientConnection);
			why = "nw_path_status_satisfiable";
			uuid_t vpn_uuid;
			if (nw_path_get_vpn_config_id(path, &vpn_uuid)) {
				flags |= kSCNetworkReachabilityFlagsConnectionOnDemand;
				why = "nw_path_status_satisfiable, OnDemand";
			}
#if	TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
			else if (nw_path_uses_interface_type(path, nw_interface_type_cellular)) {
				flags |= kSCNetworkReachabilityFlagsIsWWAN;
				why = "nw_path_status_satisfiable, WWAN";
			}
#endif	// TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
		}
	}
	SC_log(LOG_DEBUG, "__SCNetworkReachabilityGetFlagsFromPath, flags = 0x%08x, %s", flags, why);
	return (flags & kSCNetworkReachabilityFlagsMask);
}

static nw_endpoint_t
__SCNetworkReachabilityGetPrimaryEndpoint(SCNetworkReachabilityPrivateRef targetPrivate)
{
	if (targetPrivate->type == reachabilityTypeName) {
		return targetPrivate->hostnameEndpoint;
	} else if (targetPrivate->type == reachabilityTypeAddress ||
		   targetPrivate->type == reachabilityTypeAddressPair ||
		   targetPrivate->type == reachabilityTypePTR) {
		return targetPrivate->remoteAddressEndpoint;
	}
	return NULL;
}

int
SCNetworkReachabilityGetInterfaceIndex(SCNetworkReachabilityRef target)
{
	int				if_index	= -1;
	Boolean				ok		= TRUE;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;
	SCNetworkReachabilityFlags	flags		= 0;

	if (!isA_SCNetworkReachability(target)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return if_index;
	}

	MUTEX_LOCK(&targetPrivate->lock);

	flags = __SCNetworkReachabilityGetFlagsFromPath(targetPrivate->lastPath,
							targetPrivate->type,
							nw_resolver_status_invalid,
							NULL,
							targetPrivate->lastResolvedEndpointHasFlags,
							targetPrivate->lastResolvedEndpointFlags);

	/* Only return the if_index if the connection is reachable not for reachable connection
	 * required etc ... */
	if (ok && __SCNetworkReachabilityRank(flags) == ReachabilityRankReachable) {
		if (targetPrivate->lastResolvedEndpointHasFlags) {
			if_index = targetPrivate->lastResolvedEndpointInterfaceIndex;
		} else {
			if_index = nw_path_get_interface_index(targetPrivate->lastPath);
		}
	}

	MUTEX_UNLOCK(&targetPrivate->lock);
	return if_index;
}

// CrazyIvan46 is the feature that allows connections to IPv4 literals on IPv6-only (NAT64+DNS64) networks to work
// This function replaces the path when the initial one isn't satisfied and our target is an IPv4 literal
// It tries IPv6 reachability instead in case we could synthesize another address to connect to
static OS_OBJECT_RETURNS_RETAINED nw_path_t
__SCNetworkReachabilityCreateCrazyIvan46Path(nw_path_t path, nw_endpoint_t endpoint,
					     nw_parameters_t parameters, Boolean allow_resolution)
{
	nw_path_t retPath = NULL;
	const struct sockaddr *sa;

	if ((nw_path_get_status(path) != nw_path_status_unsatisfied) ||
	    (NULL == endpoint) || (nw_endpoint_get_type(endpoint) != nw_endpoint_type_address)) {
		return NULL;
	}

	sa = nw_endpoint_get_address(endpoint);

	if (sa->sa_family != AF_INET) {
		return NULL;
	}

	if (allow_resolution) {
		uint32_t ifIndex = 0;
		int32_t numPrefixes;
		nw_nat64_prefix_t *prefixes = NULL;
		struct sockaddr_in sin;

		memcpy(&sin, sa, MIN(sizeof(sin), sa->sa_len));
		if (NULL != parameters) {
			ifIndex = nw_parameters_get_required_interface_index(parameters);
		}
		numPrefixes = nw_nat64_copy_prefixes(&ifIndex, &prefixes);
		if (numPrefixes > 0) {
			struct sockaddr_in6 synthesizedAddress = {
				.sin6_len = sizeof(struct sockaddr_in6),
				.sin6_family = AF_INET6,
				.sin6_port = htons(nw_endpoint_get_port(endpoint)),
				.sin6_flowinfo = 0,
				.sin6_scope_id = 0
			};
			nw_endpoint_t synthesizedEndpoint;
			nw_path_evaluator_t synthesizedEvaluator;
			nw_path_t synthesizedPath;

			nw_nat64_synthesize_v6(&prefixes[0], &sin.sin_addr, &synthesizedAddress.sin6_addr);
			synthesizedEndpoint = nw_endpoint_create_address((const struct sockaddr *)&synthesizedAddress);
			synthesizedEvaluator = nw_path_create_evaluator_for_endpoint(synthesizedEndpoint, parameters);
			synthesizedPath = nw_path_evaluator_copy_path(synthesizedEvaluator);
			if (nw_path_get_status(synthesizedPath) != nw_path_status_unsatisfied) {
				retPath = synthesizedPath;
				SC_log(LOG_INFO, "Using CrazyIvan46 synthesized reachability result");
			} else {
				nw_release(synthesizedPath);
			}
			nw_release(synthesizedEvaluator);
			nw_release(synthesizedEndpoint);
		}
	} else {
		// Don't synthesize in non-scheduled mode to avoid generating DNS traffic
		nw_path_t v6Path;
		nw_path_evaluator_t v6PathEvaluator;
		nw_parameters_t v6Parameters;

		if (NULL != parameters) {
			v6Parameters = nw_parameters_copy(parameters);
		} else {
			v6Parameters = nw_parameters_create();
		}
		nw_parameters_set_required_address_family(v6Parameters, AF_INET6);
		v6PathEvaluator = nw_path_create_evaluator_for_endpoint(NULL, v6Parameters);
		v6Path = nw_path_evaluator_copy_path(v6PathEvaluator);
		if (nw_path_get_status(v6Path) != nw_path_status_unsatisfied) {
			retPath = v6Path;
			SC_log(LOG_INFO, "Using CrazyIvan46 simple reachability result");
		} else {
			nw_release(v6Path);
		}
		nw_release(v6PathEvaluator);
		nw_release(v6Parameters);
	}
	return retPath;
}

Boolean
SCNetworkReachabilityGetFlags(SCNetworkReachabilityRef		target,
			      SCNetworkReachabilityFlags	*flags)
{
	nw_path_t			crazyIvanPath;
	nw_endpoint_t			endpoint;
	Boolean				ok		= TRUE;
	nw_path_t			path;
	nw_path_evaluator_t		pathEvaluator;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	if (!isA_SCNetworkReachability(target)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	MUTEX_LOCK(&targetPrivate->lock);

	if (targetPrivate->scheduled) {
		// if being watched, return the last known (and what should be current) status
		*flags = __SCNetworkReachabilityGetFlagsFromPath(targetPrivate->lastPath,
								 targetPrivate->type,
								 targetPrivate->lastResolverStatus,
								 targetPrivate->lastResolvedEndpoints,
								 targetPrivate->lastResolvedEndpointHasFlags,
								 targetPrivate->lastResolvedEndpointFlags);
		// because we have synchronously captured the current status, we no longer
		// need our by-name required callback
		targetPrivate->sentFirstUpdate = TRUE;
		goto done;
	}

	// Not being watched, so run a one-shot path evaluator
	// We don't care about DNS resolution in this case, since we only need to have the
	// DNS resolution to support clients watching reachability to get updates
	endpoint = __SCNetworkReachabilityGetPrimaryEndpoint(targetPrivate);
	pathEvaluator = nw_path_create_evaluator_for_endpoint(endpoint, targetPrivate->parameters);
	path = nw_path_evaluator_copy_path(pathEvaluator);

	if (isReachabilityTypeAddress(targetPrivate->type)) {
		crazyIvanPath = __SCNetworkReachabilityCreateCrazyIvan46Path(path, endpoint,
									     targetPrivate->parameters, FALSE);
		if (NULL != crazyIvanPath) {
			nw_release(path);
			path = crazyIvanPath;
		}
	}

	*flags = __SCNetworkReachabilityGetFlagsFromPath(path, 0, nw_resolver_status_invalid, NULL, FALSE, 0);
	nw_release(path);
	nw_release(pathEvaluator);

    done :

	MUTEX_UNLOCK(&targetPrivate->lock);
	return ok;
}


#pragma mark -
#pragma mark Notifications

static void
reachPerformAndUnlock(SCNetworkReachabilityPrivateRef targetPrivate)
{
	void				*context_info;
	void				(*context_release)(const void *);
	SCNetworkReachabilityCallBack	rlsFunction;
	SCNetworkReachabilityFlags	flags		= 0;

	if (!targetPrivate->scheduled) {
		// if no longer scheduled
		SC_log(LOG_INFO, "%sskipping SCNetworkReachability callback, no longer scheduled",
		       targetPrivate->log_prefix);
		MUTEX_UNLOCK(&targetPrivate->lock);
		return;
	}

	// callout
	rlsFunction = targetPrivate->rlsFunction;
	if (targetPrivate->rlsContext.retain != NULL) {
		context_info	= (void *)(*targetPrivate->rlsContext.retain)(targetPrivate->rlsContext.info);
		context_release	= targetPrivate->rlsContext.release;
	} else {
		context_info	= targetPrivate->rlsContext.info;
		context_release	= NULL;
	}

	flags = __SCNetworkReachabilityGetFlagsFromPath(targetPrivate->lastPath,
							targetPrivate->type,
							targetPrivate->lastResolverStatus,
							targetPrivate->lastResolvedEndpoints,
							targetPrivate->lastResolvedEndpointHasFlags,
							targetPrivate->lastResolvedEndpointFlags);

	MUTEX_UNLOCK(&targetPrivate->lock);

	if (rlsFunction != NULL) {
		SC_log(LOG_DEBUG, "%sexec SCNetworkReachability callout w/flags = 0x%08x",
		       targetPrivate->log_prefix,
		       flags);
		(*rlsFunction)((SCNetworkReachabilityRef)targetPrivate,
			       flags,
			       context_info);
	}

	if (context_release != NULL) {
		(*context_release)(context_info);
	}

	return;
}

static void
reachPerform(void *info)
{
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)info;

	MUTEX_LOCK(&targetPrivate->lock);
	reachPerformAndUnlock(targetPrivate);

}

static void
reachUpdateAndUnlock(SCNetworkReachabilityPrivateRef targetPrivate)
{
	targetPrivate->sentFirstUpdate = TRUE;
	if (targetPrivate->rls != NULL) {
		if (targetPrivate->rlList != NULL) {
			CFRunLoopSourceSignal(targetPrivate->rls);
			_SC_signalRunLoop(targetPrivate, targetPrivate->rls, targetPrivate->rlList);
		}
		MUTEX_UNLOCK(&targetPrivate->lock);
	} else {
		reachPerformAndUnlock(targetPrivate);
	}
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
		memcpy(&targetPrivate->rlsContext, context, sizeof(SCNetworkReachabilityContext));
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

Boolean
SCNetworkReachabilityScheduleWithRunLoop(SCNetworkReachabilityRef	target,
					 CFRunLoopRef			runLoop,
					 CFStringRef			runLoopMode)
{
	Boolean				success = FALSE;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;
	if (!isA_SCNetworkReachability(target) || (runLoop == NULL) || (runLoopMode == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	MUTEX_LOCK(&targetPrivate->lock);

	if (targetPrivate->scheduled) {
		if (targetPrivate->rls != NULL && targetPrivate->rlList != NULL) {
			if (!_SC_isScheduled(NULL, runLoop, runLoopMode, targetPrivate->rlList)) {
				/*
				 * if we do not already have host notifications scheduled with
				 * this runLoop / runLoopMode
				 */
				CFRunLoopAddSource(runLoop, targetPrivate->rls, runLoopMode);
			}

			_SC_schedule(target, runLoop, runLoopMode, targetPrivate->rlList);

			MUTEX_UNLOCK(&targetPrivate->lock);
			return TRUE;
		} else {
			MUTEX_UNLOCK(&targetPrivate->lock);
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
		}
	}

	CFRunLoopSourceContext	context = {
		0				// version
		, (void *)target		// info
		, CFRetain			// retain
		, CFRelease			// release
		, reachRLSCopyDescription	// copyDescription
		, CFEqual			// equal
		, CFHash			// hash
		, NULL				// schedule
		, NULL				// cancel
		, reachPerform			// perform
	};

	targetPrivate->rls    = CFRunLoopSourceCreate(NULL, 0, &context);
	targetPrivate->rlList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	if (!_SC_isScheduled(NULL, runLoop, runLoopMode, targetPrivate->rlList)) {
		/*
		 * if we do not already have host notifications scheduled with
		 * this runLoop / runLoopMode
		 */
		CFRunLoopAddSource(runLoop, targetPrivate->rls, runLoopMode);
	}

	_SC_schedule(target, runLoop, runLoopMode, targetPrivate->rlList);

	success = __SCNetworkReachabilitySetDispatchQueue(targetPrivate, _callback_queue());
	if (!success) {
		if (_SC_unschedule(target, runLoop, runLoopMode, targetPrivate->rlList, FALSE)) {
			CFIndex n = CFArrayGetCount(targetPrivate->rlList);
			if ((n == 0) || !_SC_isScheduled(NULL, runLoop, runLoopMode, targetPrivate->rlList)) {
				// if target is no longer scheduled for this runLoop / runLoopMode
				CFRunLoopRemoveSource(runLoop, targetPrivate->rls, runLoopMode);
				if (n == 0) {
					// if *all* notifications have been unscheduled
					CFRelease(targetPrivate->rlList);
					targetPrivate->rlList = NULL;
					CFRunLoopSourceInvalidate(targetPrivate->rls);
					CFRelease(targetPrivate->rls);
					targetPrivate->rls = NULL;
				}
			}
		}
	}

	MUTEX_UNLOCK(&targetPrivate->lock);
	return success;
}

Boolean
SCNetworkReachabilityUnscheduleFromRunLoop(SCNetworkReachabilityRef	target,
					   CFRunLoopRef			runLoop,
					   CFStringRef			runLoopMode)
{
	Boolean				success = FALSE;
	Boolean				unscheduleDispatchQueue = FALSE;
	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;

	if (!isA_SCNetworkReachability(target) || (runLoop == NULL) || (runLoopMode == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	MUTEX_LOCK(&targetPrivate->lock);

	if (targetPrivate->rlList == NULL || targetPrivate->rls == NULL || !targetPrivate->scheduled) {
		MUTEX_UNLOCK(&targetPrivate->lock);
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (_SC_unschedule(target, runLoop, runLoopMode, targetPrivate->rlList, FALSE)) {
		CFIndex n = CFArrayGetCount(targetPrivate->rlList);
		if ((n == 0) || !_SC_isScheduled(NULL, runLoop, runLoopMode, targetPrivate->rlList)) {
			// if target is no longer scheduled for this runLoop / runLoopMode
			CFRunLoopRemoveSource(runLoop, targetPrivate->rls, runLoopMode);
			if (n == 0) {
				// if *all* notifications have been unscheduled
				unscheduleDispatchQueue = TRUE;
				CFRelease(targetPrivate->rlList);
				targetPrivate->rlList = NULL;
				CFRunLoopSourceInvalidate(targetPrivate->rls);
				CFRelease(targetPrivate->rls);
				targetPrivate->rls = NULL;
			}
		}
	}

	if (unscheduleDispatchQueue) {
		success = __SCNetworkReachabilitySetDispatchQueue(targetPrivate, NULL);
	} else {
		success = TRUE;
	}
	MUTEX_UNLOCK(&targetPrivate->lock);
	return success;
}

static __inline__ void
__SCNetworkReachabilityCopyPathStatus(SCNetworkReachabilityPrivateRef targetPrivate, SCNetworkReachabilityFlags *flags, uint *ifIndex, size_t *endpointCount)
{
	if (flags) {
		*flags = __SCNetworkReachabilityGetFlagsFromPath(targetPrivate->lastPath,
								 targetPrivate->type,
								 targetPrivate->lastResolverStatus,
								 targetPrivate->lastResolvedEndpoints,
								 targetPrivate->lastResolvedEndpointHasFlags,
								 targetPrivate->lastResolvedEndpointFlags);
	}
	if (ifIndex) {
		*ifIndex = nw_path_get_interface_index(targetPrivate->lastPath);
	}
	if (endpointCount) {
		*endpointCount = nw_array_get_count(targetPrivate->lastResolvedEndpoints);
	}
	return;
}

static __inline__ Boolean
__SCNetworkReachabilityShouldUpdateClient(SCNetworkReachabilityPrivateRef targetPrivate, SCNetworkReachabilityFlags oldFlags, uint oldIFIndex, size_t oldEndpointCount)
{
	SCNetworkReachabilityFlags newFlags = 0;
	uint newIFIndex = 0;
	size_t newEndpointCount = 0;
	__SCNetworkReachabilityCopyPathStatus(targetPrivate, &newFlags, &newIFIndex, &newEndpointCount);
	return (!targetPrivate->sentFirstUpdate ||
		oldFlags != newFlags ||
		oldIFIndex != newIFIndex ||
		oldEndpointCount != newEndpointCount);
}

static void
__SCNetworkReachabilityRestartResolver(SCNetworkReachabilityPrivateRef targetPrivate)
{
	if (targetPrivate &&
	    !targetPrivate->resolverBypass &&
	    isReachabilityTypeName(targetPrivate->type)) {
		nw_resolver_t resolver;
		CFRetain(targetPrivate);
		if (NULL != targetPrivate->resolver) {
			nw_resolver_cancel(targetPrivate->resolver);
		}
		if (targetPrivate->lastPath != NULL) {
			resolver = nw_resolver_create_with_path(targetPrivate->lastPath);
		} else {
			resolver = nw_resolver_create_with_endpoint(__SCNetworkReachabilityGetPrimaryEndpoint(targetPrivate), targetPrivate->lastPathParameters ? targetPrivate->lastPathParameters : targetPrivate->parameters);
		}
		if (resolver == NULL) {
			SC_log(LOG_ERR, "%sfailed to create a nw_resolver", targetPrivate->log_prefix);
			targetPrivate->resolver = NULL;
			CFRelease(targetPrivate);
			return;
		}
		targetPrivate->resolver = resolver;
		nw_resolver_set_cancel_handler(resolver, ^(void) {
			MUTEX_LOCK(&targetPrivate->lock);
			if (resolver == targetPrivate->resolver) {
				targetPrivate->resolver = NULL;
			}
			nw_release(resolver);
			MUTEX_UNLOCK(&targetPrivate->lock);
			CFRelease(targetPrivate);
		});
		if (!nw_resolver_set_update_handler(resolver, targetPrivate->dispatchQueue, ^(nw_resolver_status_t status, nw_array_t resolved_endpoints) {
			MUTEX_LOCK(&targetPrivate->lock);
			if (targetPrivate->scheduled) {
				SCNetworkReachabilityFlags oldFlags = 0;
				uint oldIFIndex = 0;
				size_t oldEndpointCount = 0;
				__SCNetworkReachabilityCopyPathStatus(targetPrivate, &oldFlags, &oldIFIndex, &oldEndpointCount);

				targetPrivate->lastResolverStatus = status;
				nw_release(targetPrivate->lastResolvedEndpoints);
				targetPrivate->lastResolvedEndpoints = nw_retain(resolved_endpoints);

				// Run path evaluation on the resolved endpoints
				__block Boolean hasFlags = FALSE;
				targetPrivate->lastResolvedEndpointHasFlags = FALSE;
				targetPrivate->lastResolvedEndpointFlags = 0;
				targetPrivate->lastResolvedEndpointInterfaceIndex = 0;
				nw_array_apply(targetPrivate->lastResolvedEndpoints, ^bool(size_t index, nw_object_t object) {
#pragma unused(index)
					SCNetworkReachabilityFlags flags = 0;
					uint interfaceIndex = 0;
					ReachabilityRankType rank;
					nw_endpoint_t resolvedEndpoint = (nw_endpoint_t)object;
					nw_path_evaluator_t pathEvaluator = nw_path_create_evaluator_for_endpoint(resolvedEndpoint, targetPrivate->lastPathParameters ? targetPrivate->lastPathParameters : targetPrivate->parameters);
					nw_path_t path = nw_path_evaluator_copy_path(pathEvaluator);
					if (path != NULL) {
						flags = __SCNetworkReachabilityGetFlagsFromPath(path, 0, nw_resolver_status_invalid, NULL, FALSE, 0);
						hasFlags = TRUE;
					}
					interfaceIndex = nw_path_get_interface_index(path);
					nw_release(path);
					nw_release(pathEvaluator);

					rank = __SCNetworkReachabilityRank(flags);
					if (rank > __SCNetworkReachabilityRank(targetPrivate->lastResolvedEndpointFlags)) {
						// Return the best case result
						targetPrivate->lastResolvedEndpointFlags = flags;
						targetPrivate->lastResolvedEndpointInterfaceIndex = interfaceIndex;
						if (rank == ReachabilityRankReachable) {
							// Can't get any better than REACHABLE
							return FALSE;
						}
					}
					return TRUE;
				});
				targetPrivate->lastResolvedEndpointHasFlags = hasFlags;

				if (__SCNetworkReachabilityShouldUpdateClient(targetPrivate, oldFlags, oldIFIndex, oldEndpointCount)) {
					reachUpdateAndUnlock(targetPrivate);
				} else {
					MUTEX_UNLOCK(&targetPrivate->lock);
				}
			} else {
				MUTEX_UNLOCK(&targetPrivate->lock);
			}
		})) {
			nw_release(resolver);
			targetPrivate->resolver = NULL;
			CFRelease(targetPrivate);
		}
	}
}

static Boolean
__SCNetworkReachabilitySetDispatchQueue(SCNetworkReachabilityPrivateRef	targetPrivate,
					dispatch_queue_t		queue)
{
	Boolean	ok	= FALSE;

	if (queue != NULL) {
		nw_path_t		crazyIvanPath;
		nw_endpoint_t		endpoint;
		nw_path_evaluator_t	pathEvaluator;

		if ((targetPrivate->dispatchQueue != NULL) ||		// if we are already scheduled with a dispatch queue
		    ((queue != NULL) && targetPrivate->scheduled)) {	// if we are already scheduled on a CFRunLoop
			_SCErrorSet(kSCStatusInvalidArgument);
			goto done;
		}

		SC_log(LOG_DEBUG, "%sscheduled", targetPrivate->log_prefix);

		// retain dispatch queue
		dispatch_retain(queue);
		nw_path_evaluator_cancel(targetPrivate->pathEvaluator);
		endpoint = __SCNetworkReachabilityGetPrimaryEndpoint(targetPrivate);
		pathEvaluator = nw_path_create_evaluator_for_endpoint(endpoint, targetPrivate->parameters);
		targetPrivate->pathEvaluator = pathEvaluator;
		targetPrivate->dispatchQueue = queue;
		targetPrivate->scheduled = TRUE;
		if (isReachabilityTypeName(targetPrivate->type)) {
			// we must have at least one callback for by-name queries
			targetPrivate->sentFirstUpdate = FALSE;
		} else {
			targetPrivate->sentFirstUpdate = TRUE;
		}

		nw_release(targetPrivate->lastPath);
		targetPrivate->lastPath = nw_path_evaluator_copy_path(pathEvaluator);

		if (isReachabilityTypeAddress(targetPrivate->type)) {
			crazyIvanPath = __SCNetworkReachabilityCreateCrazyIvan46Path(targetPrivate->lastPath, endpoint,
										     targetPrivate->parameters, FALSE);
			if (NULL != crazyIvanPath) {
				nw_release(targetPrivate->lastPath);
				targetPrivate->lastPath = crazyIvanPath;
			}
		}

		nw_release(targetPrivate->lastPathParameters);
		targetPrivate->lastPathParameters = nw_path_copy_derived_parameters(targetPrivate->lastPath);

		targetPrivate->lastResolverStatus = nw_resolver_status_invalid;
		nw_release(targetPrivate->lastResolvedEndpoints);
		targetPrivate->lastResolvedEndpoints = NULL;
		__SCNetworkReachabilityRestartResolver(targetPrivate);

		CFRetain(targetPrivate);
		nw_path_evaluator_set_cancel_handler(pathEvaluator, ^(void) {
			MUTEX_LOCK(&targetPrivate->lock);
			if (pathEvaluator == targetPrivate->pathEvaluator) {
				targetPrivate->pathEvaluator = NULL;
			}
			nw_release(pathEvaluator);
			MUTEX_UNLOCK(&targetPrivate->lock);
			CFRelease(targetPrivate);
		});

		if (!nw_path_evaluator_set_update_handler(pathEvaluator, targetPrivate->dispatchQueue, ^(nw_path_t path) {
			MUTEX_LOCK(&targetPrivate->lock);
			if (targetPrivate->scheduled) {
				nw_path_t crazyIvanPath;
				SCNetworkReachabilityFlags oldFlags = 0;
				uint oldIFIndex = 0;
				size_t oldEndpointCount = 0;
				__SCNetworkReachabilityCopyPathStatus(targetPrivate, &oldFlags, &oldIFIndex, &oldEndpointCount);

				nw_release(targetPrivate->lastPath);
				targetPrivate->lastPath = nw_retain(path);

				if (isReachabilityTypeAddress(targetPrivate->type)) {
					crazyIvanPath =
						__SCNetworkReachabilityCreateCrazyIvan46Path(targetPrivate->lastPath,
											     endpoint,
											     targetPrivate->parameters,
											     TRUE);
					if (NULL != crazyIvanPath) {
						nw_release(targetPrivate->lastPath);
						targetPrivate->lastPath = crazyIvanPath;
					}
				}

				if (targetPrivate->lastResolverStatus == nw_resolver_status_complete) {
					targetPrivate->lastResolverStatus = nw_resolver_status_invalid;
					__SCNetworkReachabilityRestartResolver(targetPrivate);
				}

				if (__SCNetworkReachabilityShouldUpdateClient(targetPrivate, oldFlags, oldIFIndex, oldEndpointCount)) {
					reachUpdateAndUnlock(targetPrivate);
				} else {
					MUTEX_UNLOCK(&targetPrivate->lock);
				}
			} else {
				MUTEX_UNLOCK(&targetPrivate->lock);
			}
		})) {
			targetPrivate->pathEvaluator = NULL;
			nw_release(pathEvaluator);
			CFRelease(targetPrivate);
		}
	} else {
		if (targetPrivate->dispatchQueue == NULL) {	// if we should be scheduled on a dispatch queue (but are not)
			_SCErrorSet(kSCStatusInvalidArgument);
			goto done;
		}

		if (!targetPrivate->scheduled) {
			// if not currently scheduled
			_SCErrorSet(kSCStatusInvalidArgument);
			goto done;
		}

		targetPrivate->scheduled = FALSE;
		targetPrivate->sentFirstUpdate = FALSE;
		nw_path_evaluator_cancel(targetPrivate->pathEvaluator);
		targetPrivate->pathEvaluator = NULL;
		nw_release(targetPrivate->lastPath);
		targetPrivate->lastPath = NULL;
		nw_release(targetPrivate->lastPathParameters);
		targetPrivate->lastPathParameters = NULL;
		nw_release(targetPrivate->lastResolvedEndpoints);
		targetPrivate->lastResolvedEndpoints = NULL;
		if (NULL != targetPrivate->resolver) {
			nw_resolver_cancel(targetPrivate->resolver);
			targetPrivate->resolver = NULL;
		}
		if (targetPrivate->dispatchQueue != NULL) {
			dispatch_release(targetPrivate->dispatchQueue);
			targetPrivate->dispatchQueue = NULL;
		}

		SC_log(LOG_DEBUG, "%sunscheduled", targetPrivate->log_prefix);
	}
	ok = TRUE;
done:
	return ok;
}

Boolean
SCNetworkReachabilitySetDispatchQueue(SCNetworkReachabilityRef	target,
				      dispatch_queue_t		queue)
{
	if (!isA_SCNetworkReachability(target)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	SCNetworkReachabilityPrivateRef	targetPrivate	= (SCNetworkReachabilityPrivateRef)target;
	MUTEX_LOCK(&targetPrivate->lock);
	Boolean success = __SCNetworkReachabilitySetDispatchQueue(targetPrivate, queue);
	MUTEX_UNLOCK(&targetPrivate->lock);
	return success;
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
#pragma unused(storeP)
#pragma unused(sa)
	nw_path_evaluator_t evaluator = nw_path_create_default_evaluator();
	nw_path_t path = nw_path_evaluator_copy_path(evaluator);
	if (nw_path_get_status(path) == nw_path_status_unsatisfied_network) {
		if (flags) {
			*flags = 0;
		}
		if (haveDNS) {
			*haveDNS = FALSE;
		}
	} else {
		if (flags) {
			*flags = kSCNetworkReachabilityFlagsReachable;
		}
		if (haveDNS) {
			*haveDNS = TRUE;
		}
	}
	nw_release(evaluator);
	nw_release(path);

	return TRUE;
}


/*
 * Copyright (c) 2017-2019 Apple Inc. All rights reserved.
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
 * April 17, 2017	Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#include "nat64-configuration.h"

#include <TargetConditionals.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#if TEST_NAT64_CONFIGURATION
static Boolean			G_set_prefixes_force_failure;
#define my_if_nametoindex	if_nametoindex
#else
#include "ip_plugin.h"
#endif

#define	INET6	1

#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <netinet/in.h>
#include <nw/private.h>
#include <sys/queue.h>


/**
 ** Support functions
 **/
static dispatch_queue_t
nat64_dispatch_queue(void)
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create("nat64 prefix request queue", NULL);
	});

	return q;
}

static Boolean
_nat64_prefix_set(const char		*if_name,
		  int32_t		num_prefixes,
		  nw_nat64_prefix_t	*prefixes)
{
	struct if_nat64req	req;
	int			ret;
	int			s;

	// pass NAT64 prefixes to the kernel
	memset(&req, 0, sizeof(req));
	strlcpy(req.ifnat64_name, if_name, sizeof(req.ifnat64_name));

	if (num_prefixes == 0) {
		SC_log(LOG_NOTICE, "%s: nat64 prefix unavailable", if_name);
	}

	for (int32_t i = 0; i < num_prefixes; i++) {
		char	prefix_str[NW_NAT64_PREFIX_STR_LENGTH]	= {0};

		nw_nat64_write_prefix_to_string(&prefixes[i], prefix_str, sizeof(prefix_str));
		SC_log(LOG_NOTICE, "%s: nat64 prefix[%d] = %s", if_name, i, prefix_str);

		if (i < NAT64_MAX_NUM_PREFIXES) {
			req.ifnat64_prefixes[i].prefix_len = prefixes[i].length;
			memcpy(&req.ifnat64_prefixes[i].ipv6_prefix,
			       &prefixes[i].data,
			       MIN(sizeof(req.ifnat64_prefixes[i].ipv6_prefix), sizeof(prefixes[i].data)));	// MIN(16, 12)
		}
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		SC_log(LOG_ERR, "socket() failed: %s", strerror(errno));
		return (FALSE);
	}
	ret = ioctl(s, SIOCSIFNAT64PREFIX, &req);
	close(s);
	if (ret == -1) {
		if ((errno != ENOENT) || (num_prefixes != 0)) {
			SC_log(LOG_ERR, "%s: ioctl(SIOCSIFNAT64PREFIX) failed: %s", if_name, strerror(errno));
		}
		return (FALSE);
	}

	SC_log(LOG_NOTICE, "%s: nat64 prefix%s updated", if_name, (num_prefixes != 1) ? "es" : "");
	return (TRUE);
}


static void
_nat64_prefix_post(CFStringRef		interface,
		   int32_t		num_prefixes,
		   nw_nat64_prefix_t	*prefixes,
		   CFAbsoluteTime	start_time)
{
#if TEST_NAT64_CONFIGURATION
#pragma unused(interface)
#pragma unused(num_prefixes)
#pragma unused(prefixes)
#pragma unused(start_time)
	return;
#else /* TEST_NAT64_CONFIGURATION */

	CFStringRef	key;

	key = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
							    kSCDynamicStoreDomainState,
							    interface,
							    kSCEntNetNAT64);
	if (num_prefixes >= 0) {
		CFDateRef		date;
		CFMutableDictionaryRef	plat_dict;

		plat_dict = CFDictionaryCreateMutable(NULL,
						      0,
						      &kCFTypeDictionaryKeyCallBacks,
						      &kCFTypeDictionaryValueCallBacks);
		/* prefixes (if available) */
		if (num_prefixes > 0) {
			CFMutableArrayRef	prefix_array;

			prefix_array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
			for (int32_t i = 0; i < num_prefixes; i++) {
				char		prefix_str[NW_NAT64_PREFIX_STR_LENGTH]	= {0};
				CFStringRef	str;

				nw_nat64_write_prefix_to_string(&prefixes[i], prefix_str, sizeof(prefix_str));
				str = CFStringCreateWithCString(NULL, prefix_str, kCFStringEncodingASCII);
				CFArrayAppendValue(prefix_array, str);
				CFRelease(str);
			}
			CFDictionarySetValue(plat_dict, kSCPropNetNAT64PrefixList, prefix_array);
			CFRelease(prefix_array);
		}
		/* start time */
		date = CFDateCreate(NULL, start_time);
		CFDictionarySetValue(plat_dict,
				     kSCPropNetNAT64PLATDiscoveryStartTime,
				     date);
		CFRelease(date);

		/* completion time */
		date = CFDateCreate(NULL, CFAbsoluteTimeGetCurrent());
		CFDictionarySetValue(plat_dict,
				     kSCPropNetNAT64PLATDiscoveryCompletionTime,
				     date);
		CFRelease(date);

		(void)SCDynamicStoreSetValue(NULL, key, plat_dict);
		SC_log(LOG_NOTICE, "%@: PLAT discovery complete %@",
		       interface, plat_dict);
		CFRelease(plat_dict);
	} else {
		(void)SCDynamicStoreRemoveValue(NULL, key);
	}
	CFRelease(key);
#endif /* TEST_NAT64_CONFIGURATION */
	return;
}

static nw_nat64_prefixes_resolver_t
_nat64_resolver_create(unsigned int if_index)
{
	nw_interface_t			interface;
	nw_parameters_t			params;
	nw_nat64_prefixes_resolver_t	resolver;

	params = nw_parameters_create();
	interface = nw_interface_create_with_index(if_index);
	if (interface == NULL) {
		SC_log(LOG_NOTICE,
		       "nw_interface_create_with_index(%u) failed",
		       if_index);
		return (NULL);
	}
	nw_parameters_require_interface(params, interface);
	nw_parameters_set_required_address_family(params, AF_INET6);
	nw_release(interface);
	resolver = nw_nat64_prefixes_resolver_create(params);
	nw_release(params);
	return (resolver);
}

/**
 ** NAT64PrefixRequest
 **/
struct NAT64PrefixRequest;
typedef struct NAT64PrefixRequest NAT64PrefixRequest, * NAT64PrefixRequestRef;
#define NAT64PrefixRequest_LIST_ENTRY LIST_ENTRY(NAT64PrefixRequest)
#define NAT64PrefixRequest_LIST_HEAD LIST_HEAD(NAT64PrefixRequestHead,	\
					       NAT64PrefixRequest)
static NAT64PrefixRequest_LIST_HEAD	S_request_head;
static struct NAT64PrefixRequestHead * 	S_request_head_p = &S_request_head;

typedef CF_ENUM(uint16_t, RequestFlags) {
	kRequestFlagsNone = 0x0000,
	kRequestFlagsValid = 0x0001,
};

struct NAT64PrefixRequest {
	NAT64PrefixRequest_LIST_ENTRY	link;
	nw_nat64_prefixes_resolver_t	resolver;
	const char *			if_name;
	CFStringRef			if_name_cf;
	unsigned int			if_index;
	unsigned int			retain_count;
	RequestFlags			flags;
};

static Boolean
NAT64PrefixRequestFlagsIsSet(NAT64PrefixRequestRef request, RequestFlags flags)
{
    return ((request->flags & flags) != 0);
}

static void
NAT64PrefixRequestFlagsSet(NAT64PrefixRequestRef request, RequestFlags flags)
{
    request->flags |= flags;
}

static void
NAT64PrefixRequestFlagsClear(NAT64PrefixRequestRef request, RequestFlags flags)
{
    request->flags &= ~flags;
}

static NAT64PrefixRequestRef
NAT64PrefixRequestFindInterface(CFStringRef if_name_cf)
{
	NAT64PrefixRequestRef		scan;

	LIST_FOREACH(scan, S_request_head_p, link) {
		if (CFEqual(if_name_cf, scan->if_name_cf)) {
			return (scan);
		}
	}
	return (NULL);
}

static void
NAT64PrefixRequestRetain(NAT64PrefixRequestRef request)
{
	request->retain_count++;
	SC_log(LOG_DEBUG, "%s: %s %p %u",
	       request->if_name, __FUNCTION__,
	       request, request->retain_count);
	return;
}

static NAT64PrefixRequestRef
NAT64PrefixRequestCreate(CFStringRef if_name_cf)
{
	unsigned int		if_index;
	char *			if_name;
	NAT64PrefixRequestRef	request;

	if_name = _SC_cfstring_to_cstring(if_name_cf, NULL, 0,
					  kCFStringEncodingASCII);
	if (if_name == NULL) {
		SC_log(LOG_ERR,
		       "%@: could not convert interface name",
		       if_name_cf);
		return (NULL);
	}
	if_index = my_if_nametoindex(if_name);
	if (if_index == 0) {
		SC_log(LOG_NOTICE,
		       "%s: interface does not exist", if_name);
		CFAllocatorDeallocate(NULL, if_name);
		return (NULL);
	}
	request = malloc(sizeof(*request));
	SC_log(LOG_DEBUG, "%@: %s %p", if_name_cf, __FUNCTION__, request);
	bzero(request, sizeof(*request));
	request->if_name_cf = CFRetain(if_name_cf);
	request->if_name = if_name;
	request->if_index = if_index;
	LIST_INSERT_HEAD(S_request_head_p, request, link);
	NAT64PrefixRequestFlagsSet(request, kRequestFlagsValid);
	NAT64PrefixRequestRetain(request);
	return (request);
}

static void
NAT64PrefixRequestStopResolver(NAT64PrefixRequestRef request)
{
	if (request->resolver != NULL) {
		SC_log(LOG_DEBUG, "%s: %s",
		       request->if_name, __FUNCTION__);
		nw_nat64_prefixes_resolver_cancel(request->resolver);
		nw_release(request->resolver);
		request->resolver = NULL;
	}
	return;
}

static void
NAT64PrefixRequestInvalidate(NAT64PrefixRequestRef request)
{
	SC_log(LOG_DEBUG, "%s: %s", request->if_name, __FUNCTION__);
	NAT64PrefixRequestStopResolver(request);
	if (NAT64PrefixRequestFlagsIsSet(request, kRequestFlagsValid)) {
		NAT64PrefixRequestFlagsClear(request, kRequestFlagsValid);
		LIST_REMOVE(request, link);
	}
	return;
}

static void
NAT64PrefixRequestRelease(NAT64PrefixRequestRef request)
{
	if (request->retain_count == 0) {
		SC_log(LOG_ERR, "%s: retain count is zero %p",
		       __FUNCTION__, request);
		return;
	}
	request->retain_count--;
	SC_log(LOG_DEBUG,
	       "%s: %s %p %u",
	       request->if_name, __FUNCTION__, request, request->retain_count);
	if (request->retain_count != 0) {
		return;
	}
	NAT64PrefixRequestInvalidate(request);
	SC_log(LOG_DEBUG, "%s %s: deallocate %p",
	       request->if_name, __FUNCTION__, request);
	if (request->if_name_cf != NULL) {
		CFRelease(request->if_name_cf);
		request->if_name_cf = NULL;
	}
	if (request->if_name != NULL) {
		CFAllocatorDeallocate(NULL, (void *)request->if_name);
		request->if_name = NULL;
	}
	free(request);
	return;
}

static void
NAT64PrefixRequestStart(NAT64PrefixRequestRef request)
{
	dispatch_block_t		cancel_handler;
	nw_nat64_copy_prefixes_block_t	handler;
	nw_nat64_prefixes_resolver_t	resolver;
	CFAbsoluteTime			start_time;

	SC_log(LOG_INFO, "%s: %s",  request->if_name, __FUNCTION__);
	if (request->resolver != NULL) {
		SC_log(LOG_DEBUG, "%s %s: resolver is already active",
		       request->if_name, __FUNCTION__);
		return;
	}
	resolver = _nat64_resolver_create(request->if_index);
	if (resolver == NULL) {
		return;
	}
	NAT64PrefixRequestRetain(request);
	cancel_handler = ^{
		SC_log(LOG_DEBUG, "%s: NAT64 resolver cancelled",
		       request->if_name);
		NAT64PrefixRequestRelease(request);
		return;
	};
	start_time = CFAbsoluteTimeGetCurrent();
	handler = ^(int32_t num_prefixes, nw_nat64_prefix_t *prefixes) {
		Boolean	remove_resolver = FALSE;

		if (!NAT64PrefixRequestFlagsIsSet(request,
						  kRequestFlagsValid)) {
			SC_log(LOG_INFO, "%s: NAT64 request is stale %p",
			       request->if_name, request);
			return;
		}
		if (prefixes != NULL) {
			/* set prefixes on the interface */
			_nat64_prefix_set(request->if_name,
					  num_prefixes, prefixes);
			remove_resolver = TRUE;
		} else {
			SC_log(LOG_ERR, "%s: NAT64 no prefixes",
			       request->if_name);
		}
		_nat64_prefix_post(request->if_name_cf,
				   num_prefixes, prefixes, start_time);
#if TEST_NAT64_CONFIGURATION
		if (G_set_prefixes_force_failure) {
			remove_resolver = TRUE;
		}
#endif /* TEST_NAT64_CONFIGURATION */
		if (remove_resolver) {
			/* remove resolver */
			NAT64PrefixRequestInvalidate(request);
			NAT64PrefixRequestRelease(request);
			return;
		}
	};
	nw_nat64_prefixes_resolver_set_cancel_handler(resolver, cancel_handler);
	nw_nat64_prefixes_resolver_set_update_handler(resolver,
						      nat64_dispatch_queue(),
						      handler);
	nw_nat64_prefixes_resolver_start(resolver);
	request->resolver = resolver;
	return;
}

/**
 ** Set iterators
 **/
static void
_nat64_process_prefix_request(const void *value, void *context)
{
#pragma unused(context)
	CFStringRef		interface = (CFStringRef)value;
	NAT64PrefixRequestRef	request;

	request = NAT64PrefixRequestFindInterface(interface);
	if (request != NULL) {
		return;
	}

	/* start a new request */
	request = NAT64PrefixRequestCreate(interface);
	if (request != NULL) {
		NAT64PrefixRequestStart(request);
	}
	return;
}

static void
_nat64_process_prefix_update(const void *value, void *context)
{
#pragma unused(context)
	CFStringRef		interface = (CFStringRef)value;
	NAT64PrefixRequestRef	request;

	request = NAT64PrefixRequestFindInterface(interface);
	if (request == NULL) {
		SC_log(LOG_DEBUG, "%@ %s: no existing request",
		       interface, __FUNCTION__);
		return;
	}

	/* destroy the old one, start a new one */
	SC_log(LOG_INFO, "%@: %s", interface, __FUNCTION__);
	NAT64PrefixRequestInvalidate(request);
	NAT64PrefixRequestRelease(request);

	/* start a new request */
	request = NAT64PrefixRequestCreate(interface);
	if (request != NULL) {
		NAT64PrefixRequestStart(request);
	}
	return;
}

static void
_nat64_process_cancel_request(const void * value, void * context)
{
#pragma unused(context)
	CFStringRef		interface = (CFStringRef)value;
	NAT64PrefixRequestRef	request;

	/* if there's an in-flight request, remove it */
	request = NAT64PrefixRequestFindInterface(interface);
	if (request == NULL) {
		/* no resolver */
		SC_log(LOG_DEBUG, "%@ %s: no active NAT64 request",
		       interface, __FUNCTION__);
		return;
	}
	SC_log(LOG_DEBUG, "%s %s: removing NAT64 request",
	       request->if_name,  __FUNCTION__);
	_nat64_prefix_set(request->if_name, 0, NULL);
	NAT64PrefixRequestInvalidate(request);
	NAT64PrefixRequestRelease(request);
	return;
}


#pragma mark -
#pragma mark NAT64 prefix functions (for IPMonitor)


__private_extern__
Boolean
is_nat64_prefix_request(CFStringRef change, CFStringRef *interface)
{
	CFArrayRef		components;
	static CFStringRef	prefix	= NULL;
	Boolean			yn	= FALSE;
	static dispatch_once_t	once;

	dispatch_once(&once, ^{
		prefix = SCDynamicStoreKeyCreateNetworkInterface(NULL, kSCDynamicStoreDomainState);
	});

	*interface = NULL;
	if (!CFStringHasPrefix(change, prefix) ||
	    !CFStringHasSuffix(change, kSCEntNetNAT64PrefixRequest)) {
		return FALSE;
	}

	components = CFStringCreateArrayBySeparatingStrings(NULL, change, CFSTR("/"));
	if (CFArrayGetCount(components) == 5) {
		*interface = CFArrayGetValueAtIndex(components, 3);
		CFRetain(*interface);
		yn = TRUE;
	}
	CFRelease(components);

	return yn;
}


__private_extern__ void
nat64_prefix_request_add_pattern(CFMutableArrayRef patterns)
{
	CFStringRef	pattern;

	pattern = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
								kSCDynamicStoreDomainState,
								kSCCompAnyRegex,
								kSCEntNetNAT64PrefixRequest);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);
	return;
}

static void
nat64_configuration_update_locked(CFSetRef requests, CFSetRef updates,
				  CFSetRef cancellations)
{
	if (cancellations != NULL) {
		CFSetApplyFunction(cancellations,
				   _nat64_process_cancel_request,
				   NULL);
	}
	// for any interface that changed, refresh the nat64 prefix
	if (updates != NULL) {
		CFSetApplyFunction(updates, _nat64_process_prefix_update, NULL);
	}

	// for any requested interface, query the nat64 prefix
	if (requests != NULL) {
		CFSetApplyFunction(requests, _nat64_process_prefix_request,
				   NULL);
	}
	return;
}

__private_extern__
void
nat64_configuration_update(CFSetRef requests, CFSetRef updates,
			   CFSetRef cancellations)
{
	dispatch_block_t	update_block;

	if (requests != NULL) {
		CFRetain(requests);
	}
	if (updates != NULL) {
		CFRetain(updates);
	}
	if (cancellations != NULL) {
		CFRetain(cancellations);
	}
	update_block = ^{
		SC_log(LOG_DEBUG,
		       "NAT64 requests %@ updates %@ cancellations %@",
		       requests, updates, cancellations);
		nat64_configuration_update_locked(requests, updates,
						  cancellations);
		if (requests != NULL) {
			CFRelease(requests);
		}
		if (updates != NULL) {
			CFRelease(updates);
		}
		if (cancellations != NULL) {
			CFRelease(cancellations);
		}
	};
	dispatch_async(nat64_dispatch_queue(), update_block);
	return;
}

#if TEST_NAT64_CONFIGURATION
int
main(int argc, char * argv[])
{
	CFStringRef	if_name_cf;
	CFMutableSetRef	set;

	set = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
	for (int i = 1; i < argc; i++) {
		if_name_cf = CFStringCreateWithCString(NULL,
						       argv[i],
						       kCFStringEncodingASCII);
		CFSetAddValue(set, if_name_cf);
		CFRelease(if_name_cf);
	}
	if (CFSetGetCount(set) == 0) {
		fprintf(stderr, "nothing to do\n");
		exit(0);
	}
	SC_log(LOG_NOTICE, "Starting %@", set);
	nat64_configuration_update(set, NULL, NULL);
	sleep(2);

	SC_log(LOG_NOTICE, "Starting 2 %@", set);
	nat64_configuration_update(set, NULL, NULL);
	sleep(2);

	SC_log(LOG_NOTICE, "Updating");
	nat64_configuration_update(NULL, set, NULL);
	sleep(2);

	SC_log(LOG_NOTICE, "Cancelling");
	nat64_configuration_update(NULL, NULL, set);
	sleep(2);

	G_set_prefixes_force_failure = TRUE;
	SC_log(LOG_NOTICE, "Starting (with forced failure) %@", set);
	nat64_configuration_update(set, NULL, NULL);
	sleep(2);

	SC_log(LOG_NOTICE, "Starting (with forced failure 2) %@", set);
	nat64_configuration_update(set, NULL, NULL);

	dispatch_main();
	exit(0);
	return (0);
}
#endif /* TEST_NAT64_CONFIGURATION */

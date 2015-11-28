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

/*
 * SCNetworkSignature.c
 * - implementation of SCNetworkSignatureRef API that allows access to
     network identification information
 *
 */
/*
 * Modification History
 *
 * November 6, 2006	Dieter Siegmund (dieter@apple.com)
 * - initial revision
 */


#include <netinet/in.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFRuntime.h>
#include <SystemConfiguration/SCDynamicStore.h>
#include <SystemConfiguration/SCValidation.h>
#include <SystemConfiguration/SCPrivate.h>
#include "SCNetworkSignature.h"
#include "SCNetworkSignaturePrivate.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <network/conninfo.h>

#pragma mark SCNetworkSignature Supporting APIs

static CFStringRef
create_global_state_v4_key(void)
{
	return SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							  kSCDynamicStoreDomainState,
							  kSCEntNetIPv4);

}

static CFStringRef
create_global_setup_v4_key(void)
{
	return SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							  kSCDynamicStoreDomainSetup,
							  kSCEntNetIPv4);
}

static CFStringRef
create_ipv4_services_pattern(void)
{
	return SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							   kSCDynamicStoreDomainState,
							   kSCCompAnyRegex,
							   kSCEntNetIPv4);
}

static CFStringRef
create_ipv6_services_pattern(void)
{
	return SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							   kSCDynamicStoreDomainState,
							   kSCCompAnyRegex,
							   kSCEntNetIPv6);
}

static CFDictionaryRef
copy_services_for_address_family(CFAllocatorRef alloc,
				 int af)
{
	CFDictionaryRef info;
	CFArrayRef      patterns;
	CFStringRef     pattern;
	CFStringRef     prop;

	prop = (af == AF_INET) ? kSCEntNetIPv4 : kSCEntNetIPv6;
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainState,
							      kSCCompAnyRegex,
							      prop);
	patterns = CFArrayCreate(NULL,
				 (const void * *)&pattern, 1,
				 &kCFTypeArrayCallBacks);
	CFRelease(pattern);
	info = SCDynamicStoreCopyMultiple(NULL, NULL, patterns);
	CFRelease(patterns);

	return (info);
}


static CF_RETURNS_RETAINED CFStringRef
my_IPAddressToCFString(int af, const void * src_p)
{
	char		ntopbuf[INET6_ADDRSTRLEN];

	if (inet_ntop(af, src_p, ntopbuf, sizeof(ntopbuf)) != NULL) {
		return (CFStringCreateWithCString(NULL, ntopbuf,
						  kCFStringEncodingASCII));
	}
	return (NULL);
}

#pragma mark -


#pragma mark SCNetworkSignature APIs

CFStringRef
SCNetworkSignatureCopyActiveIdentifierForAddress(CFAllocatorRef alloc,
						 const struct sockaddr * addr)
{
	CFStringRef		ident	= NULL;
	CFDictionaryRef		info = NULL;
	CFStringRef		global_state_v4_key = NULL;
	CFDictionaryRef		global_v4_state_dict = NULL;
	CFArrayRef		keys = NULL;
	CFArrayRef		patterns = NULL;
	in_addr_t 		s_addr;
	CFStringRef		service = NULL;
	CFDictionaryRef		service_dict = NULL;
	CFStringRef		service_id = NULL;
	struct sockaddr_in *	sin_p;
	CFStringRef		v4_service_pattern = NULL;

	/* only accept 0.0.0.0 (i.e. default) for now */
	if (addr == NULL
	    || addr->sa_family != AF_INET
	    || addr->sa_len != sizeof(struct sockaddr_in)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		goto done;
	}

	/* ALIGN: force alignment */
	sin_p = (struct sockaddr_in *)(void *)addr;
	bcopy(&sin_p->sin_addr.s_addr, &s_addr, sizeof(s_addr));
	if (s_addr != 0) {
		_SCErrorSet(kSCStatusInvalidArgument);
		goto done;
	}

	global_state_v4_key = create_global_state_v4_key();
	keys = CFArrayCreate(NULL, (const void * *)&global_state_v4_key,
			     1, &kCFTypeArrayCallBacks);

	v4_service_pattern = create_ipv4_services_pattern();
	patterns = CFArrayCreate(NULL,  (const void * *)&v4_service_pattern, 1,
				 &kCFTypeArrayCallBacks);

	info = SCDynamicStoreCopyMultiple(NULL, keys, patterns);

	if (info == NULL
	    || CFDictionaryGetCount(info) == 0) {
		goto done;
	}

	global_v4_state_dict = CFDictionaryGetValue(info, global_state_v4_key);

	if (isA_CFDictionary(global_v4_state_dict) == NULL) {
		goto done;
	}

	service_id = CFDictionaryGetValue(global_v4_state_dict,
					   kSCDynamicStorePropNetPrimaryService);

	if (isA_CFString(service_id) == NULL) {
		goto done;
	}

	service = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainState,
							      service_id, kSCEntNetIPv4);

	service_dict = CFDictionaryGetValue(info, service);


	if (isA_CFDictionary(service_dict) == NULL
	    || CFDictionaryGetCount(service_dict) == 0) {
		goto done;
	}

	ident = CFDictionaryGetValue(service_dict, kStoreKeyNetworkSignature);
	ident = isA_CFString(ident);
done:
	if (ident != NULL) {
		CFRetain(ident);
	} else {
		_SCErrorSet(kSCStatusFailed);
	}
	if (info != NULL) {
		CFRelease(info);
	}
	if (global_state_v4_key != NULL) {
		CFRelease(global_state_v4_key);
	}
	if (service != NULL) {
		CFRelease(service);
	}
	if (keys != NULL) {
		CFRelease(keys);
	}
	if (patterns != NULL) {
		CFRelease(patterns);
	}
	if (v4_service_pattern != NULL) {
		CFRelease(v4_service_pattern);
	}
	return (ident);
}

CFArrayRef /* of CFStringRef's */
SCNetworkSignatureCopyActiveIdentifiers(CFAllocatorRef alloc)
{
	CFMutableArrayRef	active = NULL;
	CFIndex			count = 0;
	CFStringRef		global_setup_v4_key = NULL;
	CFDictionaryRef		global_v4_dict;
	int			i;
	CFDictionaryRef		info = NULL;
	CFArrayRef		keys = NULL;
	CFMutableArrayRef	patterns = NULL;
	CFRange			range;
	CFMutableDictionaryRef	services_dict = NULL;
	CFArrayRef		service_order;
	CFStringRef		v4_service_pattern = NULL;
	CFStringRef		v6_service_pattern = NULL;
	const void * *		values = NULL;
#define KEYS_STATIC_COUNT	10
	const void *		values_static[KEYS_STATIC_COUNT];

	patterns = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	global_setup_v4_key = create_global_setup_v4_key();
	keys = CFArrayCreate(NULL, (const void * *)&global_setup_v4_key, 1,
			     &kCFTypeArrayCallBacks);

	v4_service_pattern = create_ipv4_services_pattern();
	CFArrayAppendValue(patterns, v4_service_pattern);

	v6_service_pattern = create_ipv6_services_pattern();
	CFArrayAppendValue(patterns, v6_service_pattern);

	info = SCDynamicStoreCopyMultiple(NULL, keys, patterns);

	if (info == NULL
	    || CFDictionaryGetCount(info) == 0) {
		goto done;
	}

	services_dict = CFDictionaryCreateMutableCopy(NULL, 0, info);
	/*
	 * The service_dict should only contain services and once each
	 * service has been visited, it will be removed from the dictionary.
	 */
	CFDictionaryRemoveValue(services_dict, global_setup_v4_key);

	global_v4_dict = CFDictionaryGetValue(info, global_setup_v4_key);

	if (isA_CFDictionary(global_v4_dict) != NULL) {
		service_order = CFDictionaryGetValue(global_v4_dict,
						     kSCPropNetServiceOrder);
		if (isA_CFArray(service_order) != NULL) {
			count = CFArrayGetCount(service_order);
		}
	}

	active = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	range = CFRangeMake(0, 0);

	for (i = 0; i < count ; i++) {
		int			j;
		CFStringRef		network_sig;
		CFStringRef		service;
		CFStringRef		service_id;
		CFDictionaryRef		service_info;
		CFStringRef		afs[2] = {kSCEntNetIPv4, kSCEntNetIPv6};

		service_id = CFArrayGetValueAtIndex(service_order, i);

		if (isA_CFString(service_id) == NULL) {
			continue;
		}

		for (j = 0; j < 2; j++) {
			service = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
						kSCDynamicStoreDomainState,
						service_id, afs[j]);

			service_info = CFDictionaryGetValue(services_dict, service);

			/* Does this service have a signature? */
			if (isA_CFDictionary(service_info) != NULL) {
				network_sig = CFDictionaryGetValue(service_info, kStoreKeyNetworkSignature);
				if (isA_CFString(network_sig) != NULL
				    && CFArrayContainsValue(active, range, network_sig) == FALSE) {
					CFArrayAppendValue(active, network_sig);
					network_sig = NULL;
					range.length++;
				}
				CFDictionaryRemoveValue(services_dict, service);
			}
			CFRelease(service);
		}
	}

	count = CFDictionaryGetCount(services_dict);
	if (count != 0) {
		if (count > KEYS_STATIC_COUNT) {
			values = (const void * *)malloc(sizeof(*values) * count);
		} else {
			values = values_static;
		}
		CFDictionaryGetKeysAndValues(services_dict, NULL,
					     (const void * *)values);
	}

	for (i = 0; i < count; i++) {
		CFStringRef 		network_sig;
		CFDictionaryRef 	service_dict = (CFDictionaryRef)values[i];

		if (isA_CFDictionary(service_dict) == NULL) {
			continue;
		}

		network_sig = CFDictionaryGetValue(service_dict,
						   kStoreKeyNetworkSignature);
		/* Does this service have a signature? */
		if (isA_CFString(network_sig) != NULL
		    && CFArrayContainsValue(active, range, network_sig) == FALSE) {
			CFArrayAppendValue(active, network_sig);
			range.length++;
			network_sig = NULL;
		}
	}
 done:
	if (info != NULL) {
		CFRelease(info);
	}
	if (services_dict != NULL) {
		CFRelease(services_dict);
	}
	if (global_setup_v4_key != NULL) {
		CFRelease(global_setup_v4_key);
	}
	if (v4_service_pattern != NULL) {
		CFRelease(v4_service_pattern);
	}
	if (v6_service_pattern != NULL) {
		CFRelease(v6_service_pattern);
	}
	if (values != NULL && values != values_static) {
		free(values);
	}
	if (keys != NULL) {
		CFRelease(keys);
	}
	if (patterns != NULL) {
		CFRelease(patterns);
	}
	if (active != NULL && CFArrayGetCount(active) == 0) {
		CFRelease(active);
		active = NULL;
	}
	if (active == NULL) {
		_SCErrorSet(kSCStatusFailed);
	}
	return (active);
}


CFStringRef
SCNetworkSignatureCopyIdentifierForConnectedSocket(CFAllocatorRef alloc,
						   int sock_fd)
{
	CFStringRef		addresses_key;
	int			af;
	CFIndex			count;
	int			i;
	char			if_name[IFNAMSIZ];
	CFStringRef		if_name_cf	= NULL;
	conninfo_t *		info		= NULL;
	const void * *		keys		= NULL;
#define KEYS_STATIC_COUNT	10
	const void *		keys_static[KEYS_STATIC_COUNT];
	const void *		local_ip_p;
	CFStringRef		local_ip_str	= NULL;
	CFStringRef		ret_signature	= NULL;
	CFDictionaryRef		service_info	= NULL;
	int			status		= kSCStatusFailed;

	if (copyconninfo(sock_fd, CONNID_ANY, &info) != 0) {
		status = kSCStatusInvalidArgument;
		goto done;
	}
	if ((info->ci_flags & CIF_CONNECTED) == 0
	    || info->ci_src == NULL) {
	    goto done;
	}
	af = info->ci_src->sa_family;
	switch (af) {
	case AF_INET:
		addresses_key = kSCPropNetIPv4Addresses;
		local_ip_p = &((struct sockaddr_in *)
			       (void *)info->ci_src)->sin_addr;
		break;
	case AF_INET6:
		addresses_key = kSCPropNetIPv6Addresses;
		local_ip_p = &((struct sockaddr_in6 *)
			       (void *)info->ci_src)->sin6_addr;
		break;
	default:
		status = kSCStatusInvalidArgument;
		goto done;
	}

	/* search for service with matching IP address and interface name */
	service_info = copy_services_for_address_family(alloc, af);
	if (service_info == NULL) {
		goto done;
	}
	local_ip_str = my_IPAddressToCFString(af, local_ip_p);
	if (local_ip_str == NULL) {
		goto done;
	}
	if (info->ci_ifindex != 0
	    && if_indextoname(info->ci_ifindex, if_name) != NULL) {
		if_name_cf
			= CFStringCreateWithCString(NULL, if_name,
						    kCFStringEncodingASCII);
	}
	count = CFDictionaryGetCount(service_info);
	if (count > KEYS_STATIC_COUNT) {
		keys = (const void * *)malloc(sizeof(*keys) * count);
	}
	else {
		keys = keys_static;
	}
	CFDictionaryGetKeysAndValues(service_info, keys, NULL);
	for (i = 0; i < count; i++) {
		CFArrayRef		addrs;
		CFRange			range;
		CFStringRef		signature;
		CFDictionaryRef		value;

		value = CFDictionaryGetValue(service_info, keys[i]);
		if (isA_CFDictionary(value) == NULL) {
			continue;
		}
		signature = CFDictionaryGetValue(value,
						 kStoreKeyNetworkSignature);
		if (isA_CFString(signature) == NULL) {
			/* no signature */
			continue;
		}
		if (if_name_cf != NULL) {
			CFStringRef		confirmed_if;
			CFStringRef		this_if;

			this_if = CFDictionaryGetValue(value,
						       kSCPropInterfaceName);
			if (isA_CFString(this_if) == NULL
			    || !CFEqual(this_if, if_name_cf)) {
				/* no interface or it doesn't match */
				continue;
			}
			confirmed_if
				= CFDictionaryGetValue(value,
						       kSCPropConfirmedInterfaceName);
			if (isA_CFString(confirmed_if) != NULL
			    && !CFEqual(confirmed_if, if_name_cf)) {
				/* confirmed interface doesn't match */
				continue;
			}
		}

		addrs = CFDictionaryGetValue(value, addresses_key);
		if (isA_CFArray(addrs) == NULL) {
			continue;
		}
		range = CFRangeMake(0, CFArrayGetCount(addrs));
		if (CFArrayContainsValue(addrs, range, local_ip_str)) {
			ret_signature = CFRetain(signature);
			status = kSCStatusOK;
			break;
		}
	}

 done:
	if (info != NULL) {
		freeconninfo(info);
	}
	if (if_name_cf != NULL) {
		CFRelease(if_name_cf);
	}
	if (local_ip_str != NULL) {
		CFRelease(local_ip_str);
	}
	if (keys != NULL && keys != keys_static) {
		free(keys);
	}
	if (service_info != NULL) {
		CFRelease(service_info);
	}
	if (status != kSCStatusOK) {
		_SCErrorSet(status);
	}
	return (ret_signature);
}

#pragma mark -

/*
 * Copyright (c) 2002-2014 Apple Inc. All rights reserved.
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
 * August 5, 2002	Allan Nathanson <ajn@apple.com>
 * - split code out from eventmon.c
 */

#include "eventmon.h"
#include "cache.h"
#include "ev_ipv4.h"

#ifndef	kSCEntNetIPv4ARPCollision
#define	kSCEntNetIPv4ARPCollision	CFSTR("IPv4ARPCollision")
#endif	/* kSCEntNetIPv4ARPCollision */

#if	!TARGET_OS_IPHONE
#ifndef	kSCEntNetIPv4PortInUse
#define	kSCEntNetIPv4PortInUse		CFSTR("PortInUse")
#endif	/* kSCEntNetIPv4PortInUse */
#endif	/* !TARGET_OS_IPHONE */

#define IP_FORMAT	"%d.%d.%d.%d"
#define IP_CH(ip, i)	(((u_char *)(ip))[i])
#define IP_LIST(ip)	IP_CH(ip,0),IP_CH(ip,1),IP_CH(ip,2),IP_CH(ip,3)


static void
appendAddress(CFMutableDictionaryRef dict, CFStringRef key, struct in_addr *address)
{
	CFStringRef		addr;
	CFArrayRef		addrs;
	CFMutableArrayRef	newAddrs;

	addrs = CFDictionaryGetValue(dict, key);
	if (addrs) {
		newAddrs = CFArrayCreateMutableCopy(NULL, 0, addrs);
	} else {
		newAddrs = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}

	addr = CFStringCreateWithFormat(NULL, NULL, CFSTR(IP_FORMAT), IP_LIST(address));
	CFArrayAppendValue(newAddrs, addr);
	CFRelease(addr);

	CFDictionarySetValue(dict, key, newAddrs);
	CFRelease(newAddrs);
	return;
}


static CFMutableDictionaryRef
copyIF(CFStringRef key, CFMutableDictionaryRef oldIFs, CFMutableDictionaryRef newIFs)
{
	CFDictionaryRef		dict		= NULL;
	CFMutableDictionaryRef	newDict		= NULL;

	if (CFDictionaryGetValueIfPresent(newIFs, key, (const void **)&dict)) {
		newDict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
	} else {
		dict = cache_SCDynamicStoreCopyValue(store, key);
		if (dict) {
			CFDictionarySetValue(oldIFs, key, dict);
			if (isA_CFDictionary(dict)) {
				newDict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
				CFDictionaryRemoveValue(newDict, kSCPropNetIPv4Addresses);
				CFDictionaryRemoveValue(newDict, kSCPropNetIPv4SubnetMasks);
				CFDictionaryRemoveValue(newDict, kSCPropNetIPv4DestAddresses);
				CFDictionaryRemoveValue(newDict, kSCPropNetIPv4BroadcastAddresses);
			}
			CFRelease(dict);
		}
	}

	if (!newDict) {
		newDict = CFDictionaryCreateMutable(NULL,
						    0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);
	}

	return newDict;
}


static void
updateStore(const void *key, const void *value, void *context)
{
	CFDictionaryRef dict;
	CFDictionaryRef newDict = (CFDictionaryRef)value;
	CFDictionaryRef	oldIFs	= (CFDictionaryRef)context;

	dict = CFDictionaryGetValue(oldIFs, key);

	if (!dict || !CFEqual(dict, newDict)) {
		if (CFDictionaryGetCount(newDict) > 0) {
			cache_SCDynamicStoreSetValue(store, key, newDict);
		} else if (dict) {
			cache_SCDynamicStoreRemoveValue(store, key);
		}
		network_changed = TRUE;
	}

	return;
}


__private_extern__
void
ipv4_interface_update(struct ifaddrs *ifap, const char *if_name)
{
	struct ifaddrs		*ifa;
	struct ifaddrs		*ifap_temp	= NULL;
	CFStringRef		interface;
	boolean_t		interfaceFound	= FALSE;
	CFStringRef		key		= NULL;
	CFMutableDictionaryRef	oldIFs;
	CFMutableDictionaryRef	newDict		= NULL;
	CFMutableDictionaryRef	newIFs;

	oldIFs = CFDictionaryCreateMutable(NULL,
					   0,
					   &kCFTypeDictionaryKeyCallBacks,
					   &kCFTypeDictionaryValueCallBacks);
	newIFs = CFDictionaryCreateMutable(NULL,
					   0,
					   &kCFTypeDictionaryKeyCallBacks,
					   &kCFTypeDictionaryValueCallBacks);

	if (!ifap) {
		if (getifaddrs(&ifap_temp) == -1) {
			SCLog(TRUE, LOG_ERR, CFSTR("getifaddrs() failed: %s"), strerror(errno));
			goto error;
		}
		ifap = ifap_temp;
	}

	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		struct sockaddr_in	*sin;

		if (ifa->ifa_addr->sa_family != AF_INET) {
			continue;			/* sorry, not interested */
		}

		/* check if this is the requested interface */
		if (if_name) {
			if (strncmp(if_name, ifa->ifa_name, IFNAMSIZ) == 0) {
				interfaceFound = TRUE;	/* yes, this is the one I want */
			} else {
				continue;		/* sorry, not interested */
			}
		}

		interface = CFStringCreateWithCString(NULL, ifa->ifa_name, kCFStringEncodingMacRoman);
		key       = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
									  kSCDynamicStoreDomainState,
									  interface,
									  kSCEntNetIPv4);
		CFRelease(interface);

		newDict = copyIF(key, oldIFs, newIFs);

		/* ALIGN: cast ok, this should be aligned (getifaddrs). */
		sin = (struct sockaddr_in *)(void *)ifa->ifa_addr;
		appendAddress(newDict, kSCPropNetIPv4Addresses, &sin->sin_addr);

		if (ifa->ifa_flags & IFF_POINTOPOINT) {
			struct sockaddr_in	*dst;

			/* ALIGN: cast ok, this should be aligned (getifaddrs). */
			dst = (struct sockaddr_in *)(void *)ifa->ifa_dstaddr;
			appendAddress(newDict, kSCPropNetIPv4DestAddresses, &dst->sin_addr);
		} else {
			struct sockaddr_in	*brd;
			struct sockaddr_in	*msk;

			/* ALIGN: cast ok, this should be aligned (getifaddrs). */
			brd = (struct sockaddr_in *)(void *)ifa->ifa_broadaddr;
			appendAddress(newDict, kSCPropNetIPv4BroadcastAddresses,&brd->sin_addr);

			/* ALIGN: cast ok, this should be aligned (getifaddrs). */
			msk = (struct sockaddr_in *)(void *)ifa->ifa_netmask;
			appendAddress(newDict, kSCPropNetIPv4SubnetMasks, &msk->sin_addr);
		}

		CFDictionarySetValue(newIFs, key, newDict);
		CFRelease(newDict);
		CFRelease(key);
	}

	/* if the last address[es] were removed from the target interface */
	if (if_name && !interfaceFound) {
		interface = CFStringCreateWithCString(NULL, if_name, kCFStringEncodingMacRoman);
		key       = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
									  kSCDynamicStoreDomainState,
									  interface,
									  kSCEntNetIPv4);
		CFRelease(interface);

		newDict = copyIF(key, oldIFs, newIFs);

		CFDictionarySetValue(newIFs, key, newDict);
		CFRelease(newDict);
		CFRelease(key);
	}

	CFDictionaryApplyFunction(newIFs, updateStore, oldIFs);

    error :

	if (ifap_temp)	freeifaddrs(ifap_temp);
	CFRelease(oldIFs);
	CFRelease(newIFs);

	return;
}

__private_extern__
void
ipv4_arp_collision(const char *if_name, struct in_addr ip_addr, int hw_len, const void * hw_addr)
{
	uint8_t	*		hw_addr_bytes = (uint8_t *)hw_addr;
	int			i;
	CFStringRef		if_name_cf;
	CFMutableStringRef	key;
	CFStringRef		prefix;

	if_name_cf = CFStringCreateWithCString(NULL, if_name,
					       kCFStringEncodingASCII);
	prefix = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
							       kSCDynamicStoreDomainState,
							       if_name_cf,
							       kSCEntNetIPv4ARPCollision);
	key = CFStringCreateMutableCopy(NULL, 0, prefix);
	CFStringAppendFormat(key, NULL, CFSTR("/" IP_FORMAT),
			     IP_LIST(&ip_addr));
	for (i = 0; i < hw_len; i++) {
	    CFStringAppendFormat(key, NULL, CFSTR("%s%02x"),
				 (i == 0) ? "/" : ":", hw_addr_bytes[i]);
	}
	cache_SCDynamicStoreNotifyValue(store, key);
	CFRelease(key);
	CFRelease(prefix);
	CFRelease(if_name_cf);
	return;
}

#if	!TARGET_OS_IPHONE
__private_extern__
void
ipv4_port_in_use(uint16_t port, pid_t req_pid)
{
	CFStringRef		key;

	key = SCDynamicStoreKeyCreate(NULL,
				      CFSTR("%@/%@/Protocol/%@/%@/%d/%d"),
				      kSCDynamicStoreDomainState,
				      kSCCompNetwork,
				      kSCEntNetIPv4,
				      kSCEntNetIPv4PortInUse,
				      port, req_pid);
	cache_SCDynamicStoreNotifyValue(store, key);
	CFRelease(key);
	return;
}
#endif	/* !TARGET_OS_IPHONE */

static void
interface_notify_entity(const char * if_name, CFStringRef entity)
{
	CFStringRef		if_name_cf;
	CFStringRef		key;

	if_name_cf = CFStringCreateWithCString(NULL, if_name,
					       kCFStringEncodingASCII);
	key = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
							    kSCDynamicStoreDomainState,
							    if_name_cf,
							    entity);
	CFRelease(if_name_cf);
	cache_SCDynamicStoreNotifyValue(store, key);
	CFRelease(key);
	return;
}

__private_extern__ void
ipv4_router_arp_failure(const char * if_name)
{
	interface_notify_entity(if_name, kSCEntNetIPv4RouterARPFailure);
	return;
}

__private_extern__ void
ipv4_router_arp_alive(const char * if_name)
{
	interface_notify_entity(if_name, kSCEntNetIPv4RouterARPAlive);
	return;
}

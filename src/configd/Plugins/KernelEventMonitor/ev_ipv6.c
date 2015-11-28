/*
 * Copyright (c) 2002-2007, 2011, 2013 Apple Inc. All rights reserved.
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
 * - initial revision
 */


#include "eventmon.h"
#include "cache.h"
#include "ev_ipv6.h"

#define s6_addr16 __u6_addr.__u6_addr16

#ifdef	NOTYET
#ifndef	kSCPropNetIPv6ScopeID
#define kSCPropNetIPv6ScopeID			SCSTR("ScopeID")
#endif
#endif	/* NOTYET */


static void
appendAddress(CFMutableDictionaryRef dict, CFStringRef key, struct sockaddr_in6 *sin6)
{
	CFStringRef		addr;
	CFArrayRef		addrs;
	CFMutableArrayRef	newAddrs;
	char			str[64];

	addrs = CFDictionaryGetValue(dict, key);
	if (addrs) {
		newAddrs = CFArrayCreateMutableCopy(NULL, 0, addrs);
	} else {
		newAddrs = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}

	if (inet_ntop(AF_INET6, (const void *)&sin6->sin6_addr, str, sizeof(str)) == NULL) {
		SCLog(TRUE, LOG_ERR, CFSTR("inet_ntop() failed: %s"), strerror(errno));
		str[0] = '\0';
	}

	addr = CFStringCreateWithFormat(NULL, NULL, CFSTR("%s"), str);
	CFArrayAppendValue(newAddrs, addr);
	CFRelease(addr);

	CFDictionarySetValue(dict, key, newAddrs);
	CFRelease(newAddrs);
	return;
}


static void
appendFlags(CFMutableDictionaryRef dict, int flags6)
{
	CFArrayRef		flags;
	CFMutableArrayRef	newFlags;
	CFNumberRef		v6Flags;

	flags = CFDictionaryGetValue(dict, kSCPropNetIPv6Flags);
	if (flags) {
		newFlags = CFArrayCreateMutableCopy(NULL, 0, flags);
	} else {
		newFlags = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}

	v6Flags = CFNumberCreate(NULL, kCFNumberIntType, &flags6);
	CFArrayAppendValue(newFlags, v6Flags);
	CFRelease(v6Flags);

	CFDictionarySetValue(dict, kSCPropNetIPv6Flags, newFlags);
	CFRelease(newFlags);
	return;
}


static void
appendPrefixLen(CFMutableDictionaryRef dict, struct sockaddr_in6 *sin6)
{
	register u_int8_t	*name		= &sin6->sin6_addr.s6_addr[0];
	CFNumberRef		prefixLen;
	CFArrayRef		prefixLens;
	CFMutableArrayRef	newPrefixLens;

	register int		byte;
	register int		bit;
	int			plen		= 0;

	for (byte = 0; byte < sizeof(struct in6_addr); byte++, plen += 8) {
		if (name[byte] != 0xff) {
			break;
		}
	}

	if (byte == sizeof(struct in6_addr)) {
		goto append;
	}

	for (bit = 7; bit != 0; bit--, plen++) {
		if (!(name[byte] & (1 << bit))) {
			break;
		}
	}

	for (; bit != 0; bit--) {
		if (name[byte] & (1 << bit)) {
			plen = 0;
			goto append;
		}
	}

	byte++;
	for (; byte < sizeof(struct in6_addr); byte++) {
		if (name[byte]) {
			plen = 0;
			goto append;
		}
	}

    append :

	prefixLens = CFDictionaryGetValue(dict, kSCPropNetIPv6PrefixLength);
	if (prefixLens) {
		newPrefixLens = CFArrayCreateMutableCopy(NULL, 0, prefixLens);
	} else {
		newPrefixLens = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}

	prefixLen = CFNumberCreate(NULL, kCFNumberIntType, &plen);
	CFArrayAppendValue(newPrefixLens, prefixLen);
	CFRelease(prefixLen);

	CFDictionarySetValue(dict, kSCPropNetIPv6PrefixLength, newPrefixLens);
	CFRelease(newPrefixLens);
	return;
}


#ifdef	NOTYET
static void
appendScopeID(CFMutableDictionaryRef dict, struct sockaddr_in6 *sin6)
{
	CFNumberRef		scope;
	CFArrayRef		scopes;
	CFMutableArrayRef	newScopes;

	scopes = CFDictionaryGetValue(dict, kSCPropNetIPv6ScopeID);
	if (scopes) {
		newScopes = CFArrayCreateMutableCopy(NULL, 0, scopes);
	} else {
		newScopes = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}

	scope = CFNumberCreate(NULL, kCFNumberSInt32Type, &sin6->sin6_scope_id);
	CFArrayAppendValue(newScopes, scope);
	CFRelease(scope);

	CFDictionarySetValue(dict, kSCPropNetIPv6ScopeID, newScopes);
	CFRelease(newScopes);
	return;
}
#endif	/* NOTYET */


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
				CFDictionaryRemoveValue(newDict, kSCPropNetIPv6Addresses);
				CFDictionaryRemoveValue(newDict, kSCPropNetIPv6DestAddresses);
				CFDictionaryRemoveValue(newDict, kSCPropNetIPv6Flags);
				CFDictionaryRemoveValue(newDict, kSCPropNetIPv6PrefixLength);
#ifdef	NOTYET
				CFDictionaryRemoveValue(newDict, kSCPropNetIPv6ScopeID);
#endif	/* NOTYET */
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
interface_update_ipv6(struct ifaddrs *ifap, const char *if_name)
{
	struct ifaddrs		*ifa;
	struct ifaddrs		*ifap_temp	= NULL;
	CFStringRef		interface;
	boolean_t		interfaceFound	= FALSE;
	CFStringRef		key		= NULL;
	CFMutableDictionaryRef	oldIFs;
	CFMutableDictionaryRef	newDict		= NULL;
	CFMutableDictionaryRef	newIFs;
	int			sock		= -1;

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
		struct in6_ifreq	ifr6;
#define	flags6	ifr6.ifr_ifru.ifru_flags6
		struct sockaddr_in6	*sin6;

		if (ifa->ifa_addr->sa_family != AF_INET6) {
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

		if (sock == -1) {
			sock = dgram_socket(AF_INET6);
			if (sock == -1) {
				SCLog(TRUE, LOG_NOTICE, CFSTR("interface_update_ipv6: socket open failed, %s"), strerror(errno));
				goto error;
			}
		}

		/* get the current cache information */
		interface = CFStringCreateWithCString(NULL, ifa->ifa_name, kCFStringEncodingMacRoman);
		key       = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
									  kSCDynamicStoreDomainState,
									  interface,
									  kSCEntNetIPv6);
		CFRelease(interface);

		newDict = copyIF(key, oldIFs, newIFs);

		/* ALIGN: ifa->ifa_addr aligned (getifaddrs), cast ok. */
		sin6 = (struct sockaddr_in6 *)(void *)ifa->ifa_addr;

		/* XXX: embedded link local addr check */
		if (IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr) || IN6_IS_ADDR_MC_LINKLOCAL(&sin6->sin6_addr)) {
			u_int16_t	index;

			index = sin6->sin6_addr.s6_addr16[1];
			if (index != 0) {
				sin6->sin6_addr.s6_addr16[1] = 0;
				if (sin6->sin6_scope_id == 0) {
					sin6->sin6_scope_id = ntohs(index);
				}
			}
		}

		bzero((char *)&ifr6, sizeof(ifr6));
		strncpy(ifr6.ifr_name, ifa->ifa_name, sizeof(ifr6.ifr_name));
		ifr6.ifr_addr = *sin6;
		if (ioctl(sock, SIOCGIFAFLAG_IN6, &ifr6) == -1) {
			/* if flags not available for this address */
			SCLog(TRUE,
			      (errno != EADDRNOTAVAIL) ? LOG_NOTICE : LOG_DEBUG,
			      CFSTR("interface_update_ipv6: ioctl failed, %s"),
			      strerror(errno));
		}

		appendAddress  (newDict, kSCPropNetIPv6Addresses, sin6);
#ifdef	NOTYET
		appendScopeID  (newDict, sin6);
#endif	/* NOTYET */
		/* ALIGN: ifa should be aligned (from getifaddrs), cast ok.
		 * appendPrefixLen expect byte alignment */
		appendPrefixLen(newDict, (struct sockaddr_in6 *)(void *)ifa->ifa_netmask);
		appendFlags    (newDict, flags6);


		if (ifa->ifa_flags & IFF_POINTOPOINT
		    && ifa->ifa_dstaddr != NULL) {
			struct sockaddr_in6	*dst6;

			/* ALIGN: ifa should be aligned (from getifaddrs), cast ok. */
			dst6 = (struct sockaddr_in6 *)(void *)ifa->ifa_dstaddr;

			/* XXX: embedded link local addr check */
			if (IN6_IS_ADDR_LINKLOCAL(&dst6->sin6_addr) || IN6_IS_ADDR_MC_LINKLOCAL(&dst6->sin6_addr)) {
				u_int16_t	index;

				index = dst6->sin6_addr.s6_addr16[1];
				if (index != 0) {
					dst6->sin6_addr.s6_addr16[1] = 0;
					if (dst6->sin6_scope_id == 0) {
						dst6->sin6_scope_id = ntohs(index);
					}
				}
			}

			appendAddress(newDict, kSCPropNetIPv6DestAddresses, dst6);
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
									  kSCEntNetIPv6);
		CFRelease(interface);

		newDict = copyIF(key, oldIFs, newIFs);

		CFDictionarySetValue(newIFs, key, newDict);
		CFRelease(newDict);
		CFRelease(key);
	}

	CFDictionaryApplyFunction(newIFs, updateStore, oldIFs);

    error :

	if (ifap_temp)	freeifaddrs(ifap_temp);
	if (sock != -1)	close(sock);
	CFRelease(oldIFs);
	CFRelease(newIFs);

	return;
}

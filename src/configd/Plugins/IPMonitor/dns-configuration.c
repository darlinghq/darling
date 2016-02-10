/*
 * Copyright (c) 2004-2014 Apple Inc. All rights reserved.
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
 * March 22, 2004	Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <TargetConditionals.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#if	!TARGET_OS_IPHONE
#include <notify.h>
extern uint32_t notify_monitor_file(int token, const char *name, int flags);
#endif	// !TARGET_OS_IPHONE
#include <CommonCrypto/CommonDigest.h>

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include <SystemConfiguration/SCValidation.h>
#include "ip_plugin.h"

#include "dns-configuration.h"

#include <dnsinfo.h>
#include "dnsinfo_create.h"
#include "dnsinfo_server.h"

#ifdef	MAIN
#undef	MAIN
#include "dnsinfo_copy.c"
#include "dnsinfo_internal.h"
#define	MAIN
#define	DNS_CONFIGURATION_DEBUG
#endif	// MAIN

#include <dns_sd.h>
#ifndef	kDNSServiceCompMulticastDNS
#define	kDNSServiceCompMulticastDNS	"MulticastDNS"
#endif
#ifndef	kDNSServiceCompPrivateDNS
#define	kDNSServiceCompPrivateDNS	"PrivateDNS"
#endif

#define DNS_CONFIGURATION_FLAGS_KEY	CFSTR("__FLAGS__")
#define DNS_CONFIGURATION_IF_INDEX_KEY	CFSTR("__IF_INDEX__")
#define DNS_CONFIGURATION_ORDER_KEY	CFSTR("__ORDER__")

/* multicast DNS resolver configurations */
static	CFNumberRef	S_mdns_timeout	= NULL;

/* private DNS resolver configurations */
static	CFNumberRef	S_pdns_timeout	= NULL;


#pragma mark -
#pragma mark DNS resolver flags




static void
add_dns_query_flags(const void *key, const void *value, void *context)
{
	CFDictionaryRef	service		= value;
	uint32_t	*query_flags	= context;


	// check if the service has v4 or v6 configured

	if ((*query_flags & DNS_RESOLVER_FLAGS_REQUEST_A_RECORDS) == 0
	    && service_contains_protocol(service, AF_INET)) {
		*query_flags |= DNS_RESOLVER_FLAGS_REQUEST_A_RECORDS;
	}
	if ((*query_flags & DNS_RESOLVER_FLAGS_REQUEST_AAAA_RECORDS) == 0
	    && service_contains_protocol(service, AF_INET6)) {
		*query_flags |= DNS_RESOLVER_FLAGS_REQUEST_AAAA_RECORDS;
	}

	return;
}


#pragma mark -
#pragma mark DNS resolver configuration


static void
add_resolver(CFMutableArrayRef resolvers, CFMutableDictionaryRef resolver)
{
	CFIndex		i;
	CFStringRef	interface;
	CFIndex		n_resolvers;
	CFNumberRef	order;
	uint32_t	order_val	= 0;

	order = CFDictionaryGetValue(resolver, kSCPropNetDNSSearchOrder);
	if (!isA_CFNumber(order) ||
	    !CFNumberGetValue(order, kCFNumberSInt32Type, &order_val)) {
		order     = NULL;
		order_val = 0;
	}

	n_resolvers = CFArrayGetCount(resolvers);
	for (i = 0; i < n_resolvers; i++) {
		CFDictionaryRef		match_resolver;

		match_resolver = CFArrayGetValueAtIndex(resolvers, i);
		if (CFEqual(resolver, match_resolver)) {
			// a real duplicate
			return;
		}

		if (order != NULL) {
			CFMutableDictionaryRef	compare;
			Boolean			match;

			compare = CFDictionaryCreateMutableCopy(NULL, 0, match_resolver);
			CFDictionarySetValue(compare, kSCPropNetDNSSearchOrder, order);
			match = CFEqual(resolver, compare);
			CFRelease(compare);
			if (match) {
				CFNumberRef	match_order;
				uint32_t	match_order_val	= 0;

				// if only the search order's are different
				match_order = CFDictionaryGetValue(match_resolver, kSCPropNetDNSSearchOrder);
				if (!isA_CFNumber(match_order) ||
				    !CFNumberGetValue(match_order, kCFNumberSInt32Type, &match_order_val)) {
					match_order_val = 0;
				}

				if (order_val < match_order_val ) {
					// if we should prefer this match resolver, else just skip it
					CFArraySetValueAtIndex(resolvers, i, resolver);
				}

				return;
			}
		}
	}

	order = CFNumberCreate(NULL, kCFNumberCFIndexType, &n_resolvers);
	CFDictionarySetValue(resolver, DNS_CONFIGURATION_ORDER_KEY, order);
	CFRelease(order);

	interface = CFDictionaryGetValue(resolver, kSCPropInterfaceName);
	if ((interface != NULL) && !CFEqual(interface, CFSTR("*"))) {
		uint32_t	flags;
		unsigned int	if_index		= 0;
		char		if_name[IF_NAMESIZE];
		CFNumberRef	num;
		CFBooleanRef	val;

		if (_SC_cfstring_to_cstring(interface,
					    if_name,
					    sizeof(if_name),
					    kCFStringEncodingASCII) != NULL) {
			if_index = my_if_nametoindex(if_name);
		}

		if ((if_index != 0) &&
		    (
		     // check if this is a "scoped" configuration
		     (CFDictionaryGetValueIfPresent(resolver, DNS_CONFIGURATION_FLAGS_KEY, (const void **)&num) &&
		      isA_CFNumber(num) &&
		      CFNumberGetValue(num, kCFNumberSInt32Type, &flags) &&
		      (flags & DNS_RESOLVER_FLAGS_SCOPED) != 0)
		     ||
		     // check if we should scope all queries with this configuration
		     (CFDictionaryGetValueIfPresent(resolver, DNS_CONFIGURATION_SCOPED_QUERY_KEY, (const void **)&val) &&
		      isA_CFBoolean(val) &&
		      CFBooleanGetValue(val))
		    )
		   ) {
			// if interface index available and it should be used
			num = CFNumberCreate(NULL, kCFNumberIntType, &if_index);
			CFDictionarySetValue(resolver, DNS_CONFIGURATION_IF_INDEX_KEY, num);
			CFRelease(num);
		}
	}

	CFArrayAppendValue(resolvers, resolver);
	return;
}


static void
add_supplemental(CFMutableArrayRef resolvers, CFDictionaryRef dns, uint32_t defaultOrder)
{
	CFArrayRef	domains;
	CFIndex		i;
	CFIndex		n_domains;
	CFArrayRef	orders;

	domains = CFDictionaryGetValue(dns, kSCPropNetDNSSupplementalMatchDomains);
	n_domains = isA_CFArray(domains) ? CFArrayGetCount(domains) : 0;
	if (n_domains == 0) {
		return;
	}

	orders = CFDictionaryGetValue(dns, kSCPropNetDNSSupplementalMatchOrders);
	if (orders != NULL) {
		if (!isA_CFArray(orders) || (n_domains != CFArrayGetCount(orders))) {
			return;
		}
	}

	/*
	 * yes, this is a "supplemental" resolver configuration, expand
	 * the match domains and add each to the resolvers list.
	 */
	for (i = 0; i < n_domains; i++) {
		CFStringRef		match_domain;
		CFNumberRef		match_order;
		CFMutableDictionaryRef	match_resolver;

		match_domain = CFArrayGetValueAtIndex(domains, i);
		if (!isA_CFString(match_domain)) {
			continue;
		}

		match_resolver = CFDictionaryCreateMutableCopy(NULL, 0, dns);

		// set supplemental resolver "domain"
		if (CFStringGetLength(match_domain) > 0) {
			CFDictionarySetValue(match_resolver, kSCPropNetDNSDomainName, match_domain);
		} else {
			CFDictionaryRemoveValue(match_resolver, kSCPropNetDNSDomainName);
		}

		// set supplemental resolver "search_order"
		match_order = (orders != NULL) ? CFArrayGetValueAtIndex(orders, i) : NULL;
		if (isA_CFNumber(match_order)) {
			CFDictionarySetValue(match_resolver, kSCPropNetDNSSearchOrder, match_order);
		} else if (!CFDictionaryContainsKey(match_resolver, kSCPropNetDNSSearchOrder)) {
			CFNumberRef     num;

			num = CFNumberCreate(NULL, kCFNumberIntType, &defaultOrder);
			CFDictionarySetValue(match_resolver, kSCPropNetDNSSearchOrder, num);
			CFRelease(num);

			defaultOrder++;		// if multiple domains, maintain ordering
		}

		// remove keys we don't want in a supplemental resolver
		CFDictionaryRemoveValue(match_resolver, kSCPropNetDNSSupplementalMatchDomains);
		CFDictionaryRemoveValue(match_resolver, kSCPropNetDNSSupplementalMatchOrders);
		CFDictionaryRemoveValue(match_resolver, kSCPropNetDNSSearchDomains);
		CFDictionaryRemoveValue(match_resolver, kSCPropNetDNSSortList);

		add_resolver(resolvers, match_resolver);
		CFRelease(match_resolver);
	}

	return;
}


#define	N_QUICK	32


static void
add_supplemental_resolvers(CFMutableArrayRef	resolvers,
			   CFDictionaryRef	services,
			   CFArrayRef		service_order,
			   CFStringRef		scoped_interface,
			   CFDictionaryRef	scoped_service)
{
	const void *		keys_q[N_QUICK];
	const void **		keys	= keys_q;
	CFIndex			i;
	CFIndex			n_order;
	CFIndex			n_services;
	const void *		vals_q[N_QUICK];
	const void **		vals	= vals_q;

	n_services = isA_CFDictionary(services) ? CFDictionaryGetCount(services) : 0;
	if (n_services == 0) {
		return;		// if no services
	}

	if (n_services > (CFIndex)(sizeof(keys_q) / sizeof(CFTypeRef))) {
		keys = CFAllocatorAllocate(NULL, n_services * sizeof(CFTypeRef), 0);
		vals = CFAllocatorAllocate(NULL, n_services * sizeof(CFTypeRef), 0);
	}

	n_order = isA_CFArray(service_order) ? CFArrayGetCount(service_order) : 0;

	CFDictionaryGetKeysAndValues(services, keys, vals);
	for (i = 0; i < n_services; i++) {
		uint32_t		defaultOrder;
		CFDictionaryRef		dns;
		CFStringRef		interface;
		uint32_t		interface_flags;
		CFMutableDictionaryRef	newDNS		= NULL;
		CFDictionaryRef		service		= (CFDictionaryRef)vals[i];

		if (!isA_CFDictionary(service)) {
			continue;
		}

		dns = CFDictionaryGetValue(service, kSCEntNetDNS);
		dns = isA_CFDictionary(dns);
		if (dns == NULL) {
			continue;
		}

		interface = CFDictionaryGetValue(dns, kSCPropInterfaceName);

		if (scoped_interface != NULL) {
			//
			// we only want to add split/supplemental configurations
			// for queries scoped to an interface if they are NOT
			// associated with a "real" service
			//
			if (CFDictionaryContainsKey(service, kSCEntNetIPv4) ||
			    CFDictionaryContainsKey(service, kSCEntNetIPv6)) {
				continue;
			}

			//
			// in addition, we don't want to add split/supplemental
			// configurations for queries scoped to an interface if
			// the configuration does not apply to all interfaces and
			// the configuration is explicitly NOT for this interface
			//
			if (!_SC_CFEqual(interface, CFSTR("*")) &&
			    !_SC_CFEqual(interface, scoped_interface)) {
				continue;
			}

			//
			// lastly, check if A/AAAA queries should be issued (based
			// on the IP[v6] addresses).  If we would not be issuing a
			// query then don't bother adding the configuration.
			//
			interface_flags = 0;
			add_dns_query_flags(NULL, scoped_service, &interface_flags);
			if (interface_flags == 0) {
				continue;
			}
		}

		defaultOrder = DEFAULT_SEARCH_ORDER
			       - (DEFAULT_SEARCH_ORDER / 2)
			       + ((DEFAULT_SEARCH_ORDER / 1000) * (uint32_t)i);
		if ((n_order > 0) &&
		    !CFArrayContainsValue(service_order, CFRangeMake(0, n_order), keys[i])) {
			// push out services not specified in service order
			defaultOrder += (DEFAULT_SEARCH_ORDER / 1000) * n_services;
		}

		/*
		 * Ensure that we have the correct InterfaceName in the DNS configuration
		 *
		 * scoped_interface	[supplemental] interface	DNS interface
		 * ================	========================	=================
		 * NULL			NULL				NULL (No change)
		 * NULL			en0				NULL
		 * NULL			*				NULL
		 * en0			NULL				"en0"
		 * en0			en0				"en0" (now mutable)
		 * en0			*				"en0"
		 */
		if ((scoped_interface == NULL) && (interface == NULL)) {
			newDNS = (CFMutableDictionaryRef)CFRetain(dns);
		} else {
			newDNS = CFDictionaryCreateMutableCopy(NULL, 0, dns);
			if (scoped_interface != NULL) {
				CFDictionarySetValue(newDNS, kSCPropInterfaceName, scoped_interface);
			} else {
				CFDictionaryRemoveValue(newDNS, kSCPropInterfaceName);
			}
		}

		if (scoped_interface != NULL) {
			uint32_t	flags;
			CFNumberRef	num;

			// set "scoped" configuration flag(s)
			if (!CFDictionaryGetValueIfPresent(newDNS, DNS_CONFIGURATION_FLAGS_KEY, (const void **)&num) ||
			    !isA_CFNumber(num) ||
			    !CFNumberGetValue(num, kCFNumberSInt32Type, &flags)) {
				flags = 0;
			}
			flags |= DNS_RESOLVER_FLAGS_SCOPED;

			// add A/AAAA query flag(s)
			flags |= interface_flags;

			num = CFNumberCreate(NULL, kCFNumberSInt32Type, &flags);
			CFDictionarySetValue(newDNS, DNS_CONFIGURATION_FLAGS_KEY, num);
			CFRelease(num);
		}

		// add [scoped] resolver entry
		add_supplemental(resolvers, newDNS, defaultOrder);
		CFRelease(newDNS);
	}

	if (keys != keys_q) {
		CFAllocatorDeallocate(NULL, keys);
		CFAllocatorDeallocate(NULL, vals);
	}

	return;
}


static void
add_multicast_resolvers(CFMutableArrayRef resolvers, CFArrayRef multicastResolvers)
{
	CFIndex	i;
	CFIndex	n;

	n = isA_CFArray(multicastResolvers) ? CFArrayGetCount(multicastResolvers) : 0;
	for (i = 0; i < n; i++) {
		uint32_t		defaultOrder;
		CFStringRef		domain;
		CFNumberRef		num;
		CFMutableDictionaryRef	resolver;

		domain = CFArrayGetValueAtIndex(multicastResolvers, i);
		domain = _SC_trimDomain(domain);
		if (domain == NULL) {
			continue;
		}

		defaultOrder = DEFAULT_SEARCH_ORDER
		+ (DEFAULT_SEARCH_ORDER / 2)
		+ ((DEFAULT_SEARCH_ORDER / 1000) * (uint32_t)i);

		resolver = CFDictionaryCreateMutable(NULL,
						     0,
						     &kCFTypeDictionaryKeyCallBacks,
						     &kCFTypeDictionaryValueCallBacks);
		CFDictionarySetValue(resolver, kSCPropNetDNSDomainName, domain);
		CFDictionarySetValue(resolver, kSCPropNetDNSOptions, CFSTR("mdns"));
		num = CFNumberCreate(NULL, kCFNumberIntType, &defaultOrder);
		CFDictionarySetValue(resolver, kSCPropNetDNSSearchOrder, num);
		CFRelease(num);
		if (S_mdns_timeout != NULL) {
			CFDictionarySetValue(resolver, kSCPropNetDNSServerTimeout, S_mdns_timeout);
		}
		add_resolver(resolvers, resolver);
		CFRelease(resolver);
		CFRelease(domain);
	}

	return;
}


static void
add_private_resolvers(CFMutableArrayRef resolvers, CFArrayRef privateResolvers)
{
	CFIndex	i;
	CFIndex	n;

	n = isA_CFArray(privateResolvers) ? CFArrayGetCount(privateResolvers) : 0;
	for (i = 0; i < n; i++) {
		uint32_t		defaultOrder;
		CFStringRef		domain;
		CFNumberRef		num;
		CFMutableDictionaryRef	resolver;

		domain = CFArrayGetValueAtIndex(privateResolvers, i);
		domain = _SC_trimDomain(domain);
		if (domain == NULL) {
			continue;
		}

		defaultOrder = DEFAULT_SEARCH_ORDER
			       - (DEFAULT_SEARCH_ORDER / 4)
			       + ((DEFAULT_SEARCH_ORDER / 1000) * (uint32_t)i);

		resolver = CFDictionaryCreateMutable(NULL,
						     0,
						     &kCFTypeDictionaryKeyCallBacks,
						     &kCFTypeDictionaryValueCallBacks);
		CFDictionarySetValue(resolver, kSCPropNetDNSDomainName, domain);
		CFDictionarySetValue(resolver, kSCPropNetDNSOptions, CFSTR("pdns"));
		num = CFNumberCreate(NULL, kCFNumberIntType, &defaultOrder);
		CFDictionarySetValue(resolver, kSCPropNetDNSSearchOrder, num);
		CFRelease(num);
		if (S_pdns_timeout != NULL) {
			CFDictionarySetValue(resolver, kSCPropNetDNSServerTimeout, S_pdns_timeout);
		}
		add_resolver(resolvers, resolver);
		CFRelease(resolver);
		CFRelease(domain);
	}

	return;
}


static CFComparisonResult
compareBySearchOrder(const void *val1, const void *val2, void *context)
{
	CFDictionaryRef	dns1	= (CFDictionaryRef)val1;
	CFDictionaryRef	dns2	= (CFDictionaryRef)val2;
	CFNumberRef	num1;
	CFNumberRef	num2;
	uint32_t	order1	= DEFAULT_SEARCH_ORDER;
	uint32_t	order2	= DEFAULT_SEARCH_ORDER;

	num1 = CFDictionaryGetValue(dns1, kSCPropNetDNSSearchOrder);
	if (!isA_CFNumber(num1) ||
	    !CFNumberGetValue(num1, kCFNumberSInt32Type, &order1)) {
		order1 = DEFAULT_SEARCH_ORDER;
	}

	num2 = CFDictionaryGetValue(dns2, kSCPropNetDNSSearchOrder);
	if (!isA_CFNumber(num2) ||
	    !CFNumberGetValue(num2, kCFNumberSInt32Type, &order2)) {
		order2 = DEFAULT_SEARCH_ORDER;
	}

	if (order1 == order2) {
		// if same "SearchOrder", retain original orderring for configurations
		if (CFDictionaryGetValueIfPresent(dns1, DNS_CONFIGURATION_ORDER_KEY, (const void **)&num1) &&
		    CFDictionaryGetValueIfPresent(dns2, DNS_CONFIGURATION_ORDER_KEY, (const void **)&num2) &&
		    isA_CFNumber(num1) &&
		    isA_CFNumber(num2) &&
		    CFNumberGetValue(num1, kCFNumberSInt32Type, &order1) &&
		    CFNumberGetValue(num2, kCFNumberSInt32Type, &order2)) {
			if (order1 == order2) {
				return kCFCompareEqualTo;
			} else {
				return (order1 < order2) ? kCFCompareLessThan : kCFCompareGreaterThan;
			}
		}

		return kCFCompareEqualTo;
	}

	return (order1 < order2) ? kCFCompareLessThan : kCFCompareGreaterThan;
}


static CF_RETURNS_RETAINED CFArrayRef
extract_search_domains(CFMutableDictionaryRef defaultDomain, CFArrayRef supplemental)
{
	CFStringRef		defaultDomainName	= NULL;
	uint32_t		defaultOrder		= DEFAULT_SEARCH_ORDER;
	CFArrayRef		defaultSearchDomains	= NULL;
	CFIndex			defaultSearchIndex	= 0;
	CFIndex			i;
	CFMutableArrayRef	mySearchDomains;
	CFMutableArrayRef	mySupplemental		= NULL;
	CFIndex			n_supplemental;

	mySearchDomains = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	if (defaultDomain != NULL) {
		CFNumberRef	num;

		num = CFDictionaryGetValue(defaultDomain, kSCPropNetDNSSearchOrder);
		if (!isA_CFNumber(num) ||
		    !CFNumberGetValue(num, kCFNumberSInt32Type, &defaultOrder)) {
			defaultOrder = DEFAULT_SEARCH_ORDER;
		}

		defaultDomainName    = CFDictionaryGetValue(defaultDomain, kSCPropNetDNSDomainName);
		defaultSearchDomains = CFDictionaryGetValue(defaultDomain, kSCPropNetDNSSearchDomains);
	}

	// validate the provided "search" domains or move/expand/promote the "domain" name
	if (isA_CFArray(defaultSearchDomains)) {
		CFIndex	n_search;

		n_search = CFArrayGetCount(defaultSearchDomains);
		for (i = 0; i < n_search; i++) {
			CFStringRef	search;

			search = CFArrayGetValueAtIndex(defaultSearchDomains, i);
			search = _SC_trimDomain(search);
			if (search != NULL) {
				CFArrayAppendValue(mySearchDomains, search);
				CFRelease(search);
			}
		}
	} else {
		defaultDomainName = _SC_trimDomain(defaultDomainName);
		if (defaultDomainName != NULL) {
			char		*domain;
			int		domain_parts	= 1;
			char		*dp;
			const int	ndots		= 1;

			domain = _SC_cfstring_to_cstring(defaultDomainName,
							 NULL,
							 0,
							 kCFStringEncodingUTF8);
			CFRelease(defaultDomainName);

			// count domain parts
			for (dp = domain; *dp != '\0'; dp++) {
				if (*dp == '.') {
					domain_parts++;
				}
			}

			// move "domain" to "search" list (and expand as needed)
			i = LOCALDOMAINPARTS;
			dp = domain;
			do {
				CFStringRef	search;
				CFStringRef	str;

				str = CFStringCreateWithCString(NULL,
								dp,
								kCFStringEncodingUTF8);
				search = _SC_trimDomain(str);
				CFRelease(str);
				if (search != NULL) {
					CFArrayAppendValue(mySearchDomains, search);
					CFRelease(search);
				}

				dp = strchr(dp, '.') + 1;
			} while (++i <= (domain_parts - ndots));
			CFAllocatorDeallocate(NULL, domain);
		}
	}

	// add any supplemental "domain" names to the search list
	n_supplemental = (supplemental != NULL) ? CFArrayGetCount(supplemental) : 0;
	if (n_supplemental > 1) {
		mySupplemental = CFArrayCreateMutableCopy(NULL, 0, supplemental);
		CFArraySortValues(mySupplemental,
				  CFRangeMake(0, n_supplemental),
				  compareBySearchOrder,
				  NULL);
		supplemental = mySupplemental;
	}
	for (i = 0; i < n_supplemental; i++) {
		CFDictionaryRef dns;
		CFIndex		domainIndex;
		int		noSearch;
		CFNumberRef	num;
		CFStringRef	options;
		CFStringRef	supplementalDomain;
		uint32_t	supplementalOrder;

		dns = CFArrayGetValueAtIndex(supplemental, i);

		options = CFDictionaryGetValue(dns, kSCPropNetDNSOptions);
		if (isA_CFString(options)) {
			CFRange	range;

			if (CFEqual(options, CFSTR("pdns"))) {
				// don't add private resolver domains to the search list
				continue;
			}

			range = CFStringFind(options, CFSTR("interface="), 0);
			if (range.location != kCFNotFound) {
				// don't add scoped resolver domains to the search list
				continue;
			}
		}

		supplementalDomain = CFDictionaryGetValue(dns, kSCPropNetDNSDomainName);
		supplementalDomain = _SC_trimDomain(supplementalDomain);
		if (supplementalDomain == NULL) {
			continue;
		}

		num = CFDictionaryGetValue(dns, kSCPropNetDNSSupplementalMatchDomainsNoSearch);
		if (isA_CFNumber(num) &&
		    CFNumberGetValue(num, kCFNumberIntType, &noSearch) &&
		    (noSearch != 0)) {
			CFRelease(supplementalDomain);
			continue;
		}

		if (CFStringHasSuffix(supplementalDomain, CFSTR(".in-addr.arpa")) ||
		    CFStringHasSuffix(supplementalDomain, CFSTR(".ip6.arpa"    ))) {
			CFRelease(supplementalDomain);
			continue;
		}

		domainIndex = CFArrayGetFirstIndexOfValue(mySearchDomains,
							  CFRangeMake(0, CFArrayGetCount(mySearchDomains)),
							  supplementalDomain);

		num = CFDictionaryGetValue(dns, kSCPropNetDNSSearchOrder);
		if (!isA_CFNumber(num) ||
		    !CFNumberGetValue(num, kCFNumberSInt32Type, &supplementalOrder)) {
			supplementalOrder = DEFAULT_SEARCH_ORDER;
		}

		if (supplementalOrder < defaultOrder) {
			if (domainIndex != kCFNotFound) {
				// if supplemental domain is already in the search list
				CFArrayRemoveValueAtIndex(mySearchDomains, domainIndex);
				if (domainIndex < defaultSearchIndex) {
					defaultSearchIndex--;
				}
			}
			CFArrayInsertValueAtIndex(mySearchDomains,
						  defaultSearchIndex,
						  supplementalDomain);
			defaultSearchIndex++;
		} else {
			if (domainIndex == kCFNotFound) {
				// add to the (end of the) search list
				CFArrayAppendValue(mySearchDomains, supplementalDomain);
			}
		}

		CFRelease(supplementalDomain);
	}
	if (mySupplemental != NULL) CFRelease(mySupplemental);

	// update the "search" domains
	if (CFArrayGetCount(mySearchDomains) == 0) {
		CFRelease(mySearchDomains);
		mySearchDomains = NULL;
	}

	// remove the "domain" name and "search" list
	CFDictionaryRemoveValue(defaultDomain, kSCPropNetDNSDomainName);
	CFDictionaryRemoveValue(defaultDomain, kSCPropNetDNSSearchDomains);

	return mySearchDomains;
}


static void
add_scoped_resolvers(CFMutableArrayRef scoped, CFDictionaryRef services, CFArrayRef service_order)
{
	const void *		keys_q[N_QUICK];
	const void **		keys	= keys_q;
	CFIndex			i;
	CFIndex			n_order;
	CFIndex			n_services;
	CFMutableArrayRef	order;
	CFMutableSetRef		seen;

	n_services = isA_CFDictionary(services) ? CFDictionaryGetCount(services) : 0;
	if (n_services == 0) {
		return;		// if no services
	}

	// ensure that we process all services in order

	n_order = isA_CFArray(service_order) ? CFArrayGetCount(service_order) : 0;
	if (n_order > 0) {
		order = CFArrayCreateMutableCopy(NULL, 0, service_order);
	} else {
		order = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}

	if (n_services > (CFIndex)(sizeof(keys_q) / sizeof(CFTypeRef))) {
		keys = CFAllocatorAllocate(NULL, n_services * sizeof(CFTypeRef), 0);
	}
	CFDictionaryGetKeysAndValues(services, keys, NULL);
	for (i = 0; i < n_services; i++) {
		CFStringRef	serviceID = (CFStringRef)keys[i];

		if (!CFArrayContainsValue(order, CFRangeMake(0, n_order), serviceID)) {
			CFArrayAppendValue(order, serviceID);
			n_order++;
		}
	}
	if (keys != keys_q) {
		CFAllocatorDeallocate(NULL, keys);
	}

	// iterate over services

	seen = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
	for (i = 0; i < n_order; i++) {
		CFDictionaryRef		dns;
		uint32_t		flags;
		char			if_name[IF_NAMESIZE];
		CFStringRef		interface;
		CFMutableDictionaryRef	newDNS;
		CFNumberRef		num;
		CFArrayRef		searchDomains;
		CFDictionaryRef		service;
		CFStringRef		serviceID;
		uint32_t		these_flags;

		serviceID = CFArrayGetValueAtIndex(order, i);
		service = CFDictionaryGetValue(services, serviceID);
		if (!isA_CFDictionary(service)) {
			// if no service
			continue;
		}

		dns = CFDictionaryGetValue(service, kSCEntNetDNS);
		if (!isA_CFDictionary(dns)) {
			// if no DNS
			continue;
		}

		interface = CFDictionaryGetValue(dns, kSCPropInterfaceName);
		if ((interface == NULL) || CFEqual(interface, CFSTR("*"))) {
			// if no [scoped] interface or supplemental configuration w/match-all
			continue;
		}

		if (CFDictionaryContainsKey(dns, kSCPropNetDNSServiceIdentifier)) {
			// if this is a service-specific resolver
			continue;
		}

		if (CFSetContainsValue(seen, interface)) {
			// if we've already processed this [scoped] interface
			continue;
		}
		CFSetSetValue(seen, interface);

		if ((_SC_cfstring_to_cstring(interface,
					     if_name,
					     sizeof(if_name),
					     kCFStringEncodingASCII) == NULL) ||
		    (my_if_nametoindex(if_name) == 0)) {
			// if interface index not available
			continue;
		}

		// add [scoped] resolver entry
		newDNS = CFDictionaryCreateMutableCopy(NULL, 0, dns);

		// set search list
		searchDomains = extract_search_domains(newDNS, NULL);
		if (searchDomains != NULL) {
			CFDictionarySetValue(newDNS, kSCPropNetDNSSearchDomains, searchDomains);
			CFRelease(searchDomains);
		}

		// set "scoped" configuration flag(s)
		if (!CFDictionaryGetValueIfPresent(newDNS, DNS_CONFIGURATION_FLAGS_KEY, (const void **)&num) ||
		    !isA_CFNumber(num) ||
		    !CFNumberGetValue(num, kCFNumberSInt32Type, &flags)) {
			flags = 0;
		}
		flags |= DNS_RESOLVER_FLAGS_SCOPED;

		these_flags = 0;
		add_dns_query_flags(serviceID, service, &these_flags);
		if (these_flags == 0) {
		    goto skip;
		}
		flags |= these_flags;

		num = CFNumberCreate(NULL, kCFNumberSInt32Type, &flags);
		CFDictionarySetValue(newDNS, DNS_CONFIGURATION_FLAGS_KEY, num);
		CFRelease(num);

		// remove keys we don't want in a [scoped] resolver
		CFDictionaryRemoveValue(newDNS, kSCPropNetDNSSupplementalMatchDomains);
		CFDictionaryRemoveValue(newDNS, kSCPropNetDNSSupplementalMatchOrders);

		// add the [scoped] resolver
		add_resolver(scoped, newDNS);

		// add any supplemental resolver configurations for this interface
		add_supplemental_resolvers(scoped, services, service_order, interface, service);

	skip:
		CFRelease(newDNS);
	}

	CFRelease(seen);
	CFRelease(order);
	return;
}


static void
add_service_specific_resolvers(CFMutableArrayRef resolvers, CFDictionaryRef services)
{
	CFIndex	services_count	= (isA_CFDictionary(services) ? CFDictionaryGetCount(services) : 0);

	if (services_count > 0) {
		CFIndex			key_idx;
		CFStringRef		keys_q[N_QUICK];
		CFStringRef		*keys	= keys_q;
		CFMutableSetRef		seen	= CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);

		if (services_count > (CFIndex)(sizeof(keys_q) / sizeof(keys_q[0]))) {
			keys = CFAllocatorAllocate(kCFAllocatorDefault, services_count * sizeof(keys[0]), 0);
		}

		CFDictionaryGetKeysAndValues(services, (const void **)keys, NULL);

		for (key_idx = 0; key_idx < services_count; key_idx++) {
			CFDictionaryRef	service	= CFDictionaryGetValue(services, keys[key_idx]);
			CFDictionaryRef	dns	= CFDictionaryGetValue(service, kSCEntNetDNS);

			if (isA_CFDictionary(dns)) {
				CFNumberRef	service_identifier	= CFDictionaryGetValue(dns, kSCPropNetDNSServiceIdentifier);

				if (isA_CFNumber(service_identifier)) {
					if (!CFSetContainsValue(seen, service_identifier)) {
						CFMutableDictionaryRef	new_resolver	= CFDictionaryCreateMutableCopy(NULL, 0, dns);
						CFNumberRef		flags_num;
						int32_t			flags		= 0;

						CFSetSetValue(seen, service_identifier);

						if (!CFDictionaryGetValueIfPresent(new_resolver, DNS_CONFIGURATION_FLAGS_KEY, (const void **)&flags_num) ||
						    !isA_CFNumber(flags_num) ||
						    !CFNumberGetValue(flags_num, kCFNumberSInt32Type, &flags)) {
							flags = 0;
						}

						flags |= DNS_RESOLVER_FLAGS_SERVICE_SPECIFIC | DNS_RESOLVER_FLAGS_REQUEST_ALL_RECORDS;

						flags_num = CFNumberCreate(NULL, kCFNumberSInt32Type, &flags);
						CFDictionarySetValue(new_resolver, DNS_CONFIGURATION_FLAGS_KEY, flags_num);
						CFRelease(flags_num);

						if (CFDictionaryContainsKey(new_resolver, kSCPropInterfaceName)) {
							CFDictionarySetValue(new_resolver, DNS_CONFIGURATION_SCOPED_QUERY_KEY, kCFBooleanTrue);
						}

						CFDictionaryRemoveValue(new_resolver, kSCPropNetDNSSupplementalMatchDomains);
						CFDictionaryRemoveValue(new_resolver, kSCPropNetDNSSupplementalMatchOrders);

						add_resolver(resolvers, new_resolver);
						CFRelease(new_resolver);
					} else {
						my_log(LOG_ERR, "add_service_specific_resolvers: got a resolver with a duplicate service identifier, skipping");
					}
				}
			}
		}

		if (keys != keys_q) {
			CFAllocatorDeallocate(kCFAllocatorDefault, keys);
		}
		CFRelease(seen);
	}
}


static void
add_default_resolver(CFMutableArrayRef	resolvers,
		     CFDictionaryRef	defaultResolver,
		     Boolean		*orderAdded,
		     CFArrayRef		*searchDomains)
{
	CFMutableDictionaryRef	myDefault;
	uint32_t		myOrder	= DEFAULT_SEARCH_ORDER;
	CFNumberRef		order;

	if (defaultResolver == NULL) {
		myDefault = CFDictionaryCreateMutable(NULL,
						      0,
						      &kCFTypeDictionaryKeyCallBacks,
						      &kCFTypeDictionaryValueCallBacks);
	} else {
		myDefault = CFDictionaryCreateMutableCopy(NULL, 0, defaultResolver);
	}
	assert(myDefault != NULL);

	// ensure that the default resolver has a search order

	order = CFDictionaryGetValue(myDefault, kSCPropNetDNSSearchOrder);
	if (!isA_CFNumber(order) ||
	    !CFNumberGetValue(order, kCFNumberSInt32Type, &myOrder)) {
		myOrder = DEFAULT_SEARCH_ORDER;
		order = CFNumberCreate(NULL, kCFNumberIntType, &myOrder);
		CFDictionarySetValue(myDefault, kSCPropNetDNSSearchOrder, order);
		CFRelease(order);
		*orderAdded = TRUE;
	}

	// extract the "search" domain list for the default resolver (and
	// any supplemental resolvers)

	*searchDomains = extract_search_domains(myDefault, resolvers);

	// add the default resolver

	add_resolver(resolvers, myDefault);
	CFRelease(myDefault);
	return;
}


static dns_create_resolver_t
create_resolver(CFDictionaryRef dns)
{
	CFArrayRef		list;
	CFNumberRef		num;
	dns_create_resolver_t	_resolver;
	CFStringRef		str;
	CFStringRef		targetInterface		= NULL;
	unsigned int		targetInterfaceIndex	= 0;

	_resolver = _dns_resolver_create();

	// process domain
	str = CFDictionaryGetValue(dns, kSCPropNetDNSDomainName);
	if (isA_CFString(str) && (CFStringGetLength(str) > 0)) {
		char	domain[NS_MAXDNAME];

		if (_SC_cfstring_to_cstring(str, domain, sizeof(domain), kCFStringEncodingUTF8) != NULL) {
			_dns_resolver_set_domain(&_resolver, domain);
		}
	}

	// process search domains
	list = CFDictionaryGetValue(dns, kSCPropNetDNSSearchDomains);
	if (isA_CFArray(list)) {
		CFIndex	i;
		CFIndex n	= CFArrayGetCount(list);

		// add "search" domains
		for (i = 0; i < n; i++) {
			str = CFArrayGetValueAtIndex(list, i);
			if (isA_CFString(str) && (CFStringGetLength(str) > 0)) {
				char	search[NS_MAXDNAME];

				if (_SC_cfstring_to_cstring(str, search, sizeof(search), kCFStringEncodingUTF8) != NULL) {
					_dns_resolver_add_search(&_resolver, search);
				}
			}
		}
	}

	// process interface index
	num = CFDictionaryGetValue(dns, DNS_CONFIGURATION_IF_INDEX_KEY);
	if (isA_CFNumber(num)) {
		int	if_index;

		if (CFNumberGetValue(num, kCFNumberIntType, &if_index)) {
			char	if_name[IFNAMSIZ];

			_dns_resolver_set_if_index(&_resolver, if_index);

			if ((if_index != 0) &&
			    (my_if_indextoname(if_index, if_name) != NULL)) {
				targetInterface = CFStringCreateWithCString(NULL,
									    if_name,
									    kCFStringEncodingASCII);
				targetInterfaceIndex = if_index;
			}
		}
	}

	// process flags
	num = CFDictionaryGetValue(dns, DNS_CONFIGURATION_FLAGS_KEY);
	if (isA_CFNumber(num)) {
		uint32_t	flags;

		if (CFNumberGetValue(num, kCFNumberSInt32Type, &flags)) {
			_dns_resolver_set_flags(&_resolver, flags);
		}
	}

	// process nameserver addresses
	// Note: the flags may be overwritten if the resolver has LOOPBACK addresses
	list = CFDictionaryGetValue(dns, kSCPropNetDNSServerAddresses);
	if (isA_CFArray(list)) {
		CFIndex	i;
		CFIndex	n	= CFArrayGetCount(list);

		for (i = 0; i < n; i++) {
			union {
				struct sockaddr         sa;
				struct sockaddr_in      sin;
				struct sockaddr_in6     sin6;
			} addr;
			char				buf[64];

			str = CFArrayGetValueAtIndex(list, i);
			if (!isA_CFString(str)) {
				continue;
			}

			if (_SC_cfstring_to_cstring(str, buf, sizeof(buf), kCFStringEncodingASCII) == NULL) {
				continue;
			}

			if (_SC_string_to_sockaddr(buf, AF_UNSPEC, (void *)&addr, sizeof(addr)) == NULL) {
				continue;
			}

			if ((addr.sa.sa_family == AF_INET6) &&
			    IN6_IS_ADDR_LINKLOCAL(&addr.sin6.sin6_addr) &&
			    (addr.sin6.sin6_scope_id == 0) &&
			    (targetInterfaceIndex != 0)) {
				// for link local [IPv6] addresses, if the scope id is not
				// set then we should use the interface associated with the
				// resolver configuration
				addr.sin6.sin6_scope_id = targetInterfaceIndex;
			}

			_dns_resolver_add_nameserver(&_resolver, &addr.sa);
		}
	}

	// process search order
	num = CFDictionaryGetValue(dns, kSCPropNetDNSSearchOrder);
	if (isA_CFNumber(num)) {
		uint32_t	order;

		if (CFNumberGetValue(num, kCFNumberSInt32Type, &order)) {
			_dns_resolver_set_order(&_resolver, order);
		}
	}

	// process sortlist
	list = CFDictionaryGetValue(dns, kSCPropNetDNSSortList);
	if (isA_CFArray(list)) {
		CFIndex	i;
		CFIndex n	= CFArrayGetCount(list);

		for (i = 0; i < n; i++) {
			char		buf[128];
			char		*slash;
			dns_sortaddr_t	sortaddr;

			str = CFArrayGetValueAtIndex(list, i);
			if (!isA_CFString(str)) {
				continue;
			}

			if (_SC_cfstring_to_cstring(str, buf, sizeof(buf), kCFStringEncodingASCII) == NULL) {
				continue;
			}

			slash = strchr(buf, '/');
			if (slash != NULL) {
				*slash = '\0';
			}

			bzero(&sortaddr, sizeof(sortaddr));
			if (inet_aton(buf, &sortaddr.address) != 1) {
				/* if address not valid */
				continue;
			}

			if (slash != NULL) {
				if (inet_aton(slash + 1, &sortaddr.mask) != 1) {
					/* if subnet mask not valid */
					continue;
				}
			} else {
				in_addr_t	a;
				in_addr_t	m;

				a = ntohl(sortaddr.address.s_addr);
				if (IN_CLASSA(a)) {
					m = IN_CLASSA_NET;
				} else if (IN_CLASSB(a)) {
					m = IN_CLASSB_NET;
				} else if (IN_CLASSC(a)) {
					m = IN_CLASSC_NET;
				} else {
					continue;
				}

				sortaddr.mask.s_addr = htonl(m);
			}

			_dns_resolver_add_sortaddr(&_resolver, &sortaddr);
		}
	}

	// process port
	num = CFDictionaryGetValue(dns, kSCPropNetDNSServerPort);
	if (isA_CFNumber(num)) {
		int	port;

		if (CFNumberGetValue(num, kCFNumberIntType, &port)) {
			_dns_resolver_set_port(&_resolver, (uint16_t)port);
		}
	}

	// process timeout
	num = CFDictionaryGetValue(dns, kSCPropNetDNSServerTimeout);
	if (isA_CFNumber(num)) {
		int	timeout;

		if (CFNumberGetValue(num, kCFNumberIntType, &timeout)) {
			_dns_resolver_set_timeout(&_resolver, (uint32_t)timeout);
		}
	}

	// process options
	str = CFDictionaryGetValue(dns, kSCPropNetDNSOptions);
	if (isA_CFString(str)) {
		char	*options;

		options = _SC_cfstring_to_cstring(str, NULL, 0, kCFStringEncodingUTF8);
		if (options != NULL) {
			_dns_resolver_set_options(&_resolver, options);
			CFAllocatorDeallocate(NULL, options);
		}
	}

	num = CFDictionaryGetValue(dns, kSCPropNetDNSServiceIdentifier);
	if (isA_CFNumber(num)) {
		int	service_identifier;

		if (CFNumberGetValue(num, kCFNumberIntType, &service_identifier)) {
			_dns_resolver_set_service_identifier(&_resolver, (uint32_t)service_identifier);
		}
	}

	if (targetInterface != NULL) {
		CFRelease(targetInterface);
	}

	return _resolver;
}


static __inline__ Boolean
isScopedConfiguration(CFDictionaryRef dns)
{
	uint32_t	flags;
	CFNumberRef	num;

	if ((dns != NULL) &&
	    CFDictionaryGetValueIfPresent(dns, DNS_CONFIGURATION_FLAGS_KEY, (const void **)&num) &&
	    (num != NULL) &&
	    CFNumberGetValue(num, kCFNumberSInt32Type, &flags) &&
	    ((flags & DNS_RESOLVER_FLAGS_SCOPED) != 0)) {
		return TRUE;
	}

	return FALSE;
}


static __inline__ Boolean
isServiceSpecificConfiguration(CFDictionaryRef dns)
{
	uint32_t	flags;
	CFNumberRef	num;

	if (dns != NULL &&
	    CFDictionaryGetValueIfPresent(dns, DNS_CONFIGURATION_FLAGS_KEY, (const void **)&num) &&
	    num != NULL &&
	    CFNumberGetValue(num, kCFNumberSInt32Type, &flags) &&
	    (flags & DNS_RESOLVER_FLAGS_SERVICE_SPECIFIC))
	{
		return TRUE;
	}

	return FALSE;
}


static CFComparisonResult
compareDomain(const void *val1, const void *val2, void *context)
{
	CFDictionaryRef		dns1	= (CFDictionaryRef)val1;
	CFDictionaryRef		dns2	= (CFDictionaryRef)val2;
	CFStringRef		domain1;
	CFStringRef		domain2;
	CFArrayRef		labels1	= NULL;
	CFArrayRef		labels2	= NULL;
	CFIndex			n1;
	CFIndex			n2;
	CFComparisonResult	result;
	Boolean			rev1;
	Boolean			rev2;
	Boolean			scoped1;
	Boolean			scoped2;

	// "default" domains sort before "supplemental" domains
	domain1 = CFDictionaryGetValue(dns1, kSCPropNetDNSDomainName);
	domain2 = CFDictionaryGetValue(dns2, kSCPropNetDNSDomainName);
	if (domain1 == NULL) {
		return kCFCompareLessThan;
	} else if (domain2 == NULL) {
		return kCFCompareGreaterThan;
	}

	// sort non-scoped before scoped
	scoped1 = isScopedConfiguration(dns1);
	scoped2 = isScopedConfiguration(dns2);
	if (scoped1 != scoped2) {
		if (!scoped1) {
			return kCFCompareLessThan;
		} else {
			return kCFCompareGreaterThan;
		}
	}

	// must have domain names for any further comparisons
	if ((domain1 == NULL) || (domain2 == NULL)) {
		return kCFCompareEqualTo;
	}

	// forward (A, AAAA) domains sort before reverse (PTR) domains
	rev1 = CFStringHasSuffix(domain1, CFSTR(".arpa"));
	rev2 = CFStringHasSuffix(domain2, CFSTR(".arpa"));
	if (rev1 != rev2) {
		if (rev1) {
			return kCFCompareGreaterThan;
		} else {
			return kCFCompareLessThan;
		}
	}

	labels1 = CFStringCreateArrayBySeparatingStrings(NULL, domain1, CFSTR("."));
	n1 = CFArrayGetCount(labels1);

	labels2 = CFStringCreateArrayBySeparatingStrings(NULL, domain2, CFSTR("."));
	n2 = CFArrayGetCount(labels2);

	while ((n1 > 0) && (n2 > 0)) {
		CFStringRef	label1	= CFArrayGetValueAtIndex(labels1, --n1);
		CFStringRef	label2	= CFArrayGetValueAtIndex(labels2, --n2);

		// compare domain labels
		result = CFStringCompare(label1, label2, kCFCompareCaseInsensitive);
		if (result != kCFCompareEqualTo) {
			goto done;
		}
	}

	// longer labels (corp.apple.com) sort before shorter labels (apple.com)
	if (n1 > n2) {
		result = kCFCompareLessThan;
		goto done;
	} else if (n1 < n2) {
		result = kCFCompareGreaterThan;
		goto done;
	}

	// sort by search order
	result = compareBySearchOrder(val1, val2, context);

    done :

	if (labels1 != NULL) CFRelease(labels1);
	if (labels2 != NULL) CFRelease(labels2);
	return result;
}


__private_extern__
Boolean
dns_configuration_set(CFDictionaryRef   defaultResolver,
		      CFDictionaryRef   services,
		      CFArrayRef	serviceOrder,
		      CFArrayRef	multicastResolvers,
		      CFArrayRef	privateResolvers)
{
	dns_create_config_t	_config;
	Boolean			changed			= FALSE;
	uint32_t		dns_resolver_flags	= 0;
	CFIndex			i;
	CFMutableDictionaryRef	myDefault;
	Boolean			myOrderAdded		= FALSE;
	CFArrayRef		mySearchDomains		= NULL;
	CFIndex			n_resolvers;
	CFMutableArrayRef	resolvers;
	unsigned char		signature[CC_SHA1_DIGEST_LENGTH];
	static unsigned char	signature_last[CC_SHA1_DIGEST_LENGTH];

	// establish list of resolvers

	resolvers = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	assert(resolvers != NULL);

	// collect (and add) any "supplemental" resolver configurations

	add_supplemental_resolvers(resolvers, services, serviceOrder, NULL, NULL);

	// collect (and add) any "private" resolver configurations

	add_private_resolvers(resolvers, privateResolvers);

	// add the "default" resolver

	add_default_resolver(resolvers, defaultResolver, &myOrderAdded, &mySearchDomains);

	// collect (and add) any "multicast" resolver configurations

	add_multicast_resolvers(resolvers, multicastResolvers);

	// collect (and add) any "scoped" resolver configurations

	add_scoped_resolvers(resolvers, services, serviceOrder);

	// collect (and add) any "service-specific" resolver configurations

	add_service_specific_resolvers(resolvers, services);

	// sort resolvers

	n_resolvers = CFArrayGetCount(resolvers);
	if (n_resolvers > 1) {
		CFArraySortValues(resolvers, CFRangeMake(0, n_resolvers), compareDomain, NULL);
	}

	// cleanup

	for (i = n_resolvers; --i > 0; ) {
		CFDictionaryRef	resolver;

		resolver = CFArrayGetValueAtIndex(resolvers, i);
		if (!CFDictionaryContainsKey(resolver, kSCPropNetDNSDomainName) &&
		    !CFDictionaryContainsKey(resolver, kSCPropNetDNSSearchDomains) &&
		    !CFDictionaryContainsKey(resolver, kSCPropNetDNSServerAddresses)) {
			// remove empty resolver
			CFArrayRemoveValueAtIndex(resolvers, i);
			n_resolvers--;
		}
	}

	// update the default resolver

	myDefault = CFDictionaryCreateMutableCopy(NULL,
						  0,
						  CFArrayGetValueAtIndex(resolvers, 0));
	if (mySearchDomains != NULL) {
		// add search domains to the default resolver
		CFDictionarySetValue(myDefault, kSCPropNetDNSSearchDomains, mySearchDomains);
		CFRelease(mySearchDomains);
	}
	if (myOrderAdded && (n_resolvers > 1)) {
		CFDictionaryRef	resolver;

		resolver = CFArrayGetValueAtIndex(resolvers, 1);
		if (CFDictionaryContainsKey(resolver, kSCPropNetDNSDomainName) ||
		    isScopedConfiguration(resolver)) {
			// if not a supplemental "default" resolver (a domain name is
			// present) or if it's a scoped configuration
			CFDictionaryRemoveValue(myDefault, kSCPropNetDNSSearchOrder);
		}
	}
	CFArraySetValueAtIndex(resolvers, 0, myDefault);
	CFRelease(myDefault);

	// establish resolver configuration

	if ((defaultResolver == NULL) && (n_resolvers <= 1)) {
		/*
		 * if no default and no supplemental/scoped resolvers
		 */
		_config = NULL;
	} else {
		/*
		 * if default and/or supplemental/scoped resolvers are defined
		 */
		_config = _dns_configuration_create();

		CFDictionaryApplyFunction(services, add_dns_query_flags , &dns_resolver_flags);

		for (i = 0; i < n_resolvers; i++) {
			boolean_t		is_default_resolver;
			CFDictionaryRef		resolver;
			dns_create_resolver_t	_resolver;

			resolver = CFArrayGetValueAtIndex(resolvers, i);

			is_default_resolver = (!isScopedConfiguration(resolver) && !isServiceSpecificConfiguration(resolver));
			if (is_default_resolver) {
				CFMutableDictionaryRef	new_resolver;
				CFNumberRef		num;

				new_resolver = CFDictionaryCreateMutableCopy(NULL, 0, resolver);

				num = CFNumberCreate(NULL, kCFNumberSInt32Type, &dns_resolver_flags);
				CFDictionarySetValue(new_resolver, DNS_CONFIGURATION_FLAGS_KEY, num);
				CFRelease(num);

				resolver = new_resolver;
			}

			_resolver = create_resolver(resolver);
			_dns_configuration_add_resolver(&_config, _resolver);
			_dns_resolver_free(&_resolver);

			if (is_default_resolver) {
				CFRelease(resolver);
			}
		}

#if	!TARGET_OS_IPHONE
		// add flatfile resolvers

		_dnsinfo_flatfile_set_flags(dns_resolver_flags);
		_dnsinfo_flatfile_add_resolvers(&_config);
#endif	// !TARGET_OS_IPHONE
	}

#ifdef	DNS_CONFIGURATION_DEBUG
	{
		uint8_t			*buf;
		dns_config_t		*config;
		_dns_config_buf_t	*config_buf;
		uint32_t		n_config;
		uint32_t		n_padding;

		config_buf = (_dns_config_buf_t *)_config;
		n_config  = sizeof(_dns_config_buf_t) + ntohl(config_buf->n_attribute);
		n_padding = ntohl(config_buf->n_padding);
		buf = malloc(n_config + n_padding);
		bcopy((void *)config_buf, buf, n_config);
		bzero(&buf[n_config], n_padding);
		config = expand_config((_dns_config_buf_t *)buf);
		_dns_configuration_print(config);
		free(buf);
	}
#endif	// DNS_CONFIGURATION_DEBUG

	// check if the configuration changed
	_dns_configuration_signature(&_config, signature, sizeof(signature));
	if (bcmp(signature, signature_last, sizeof(signature)) != 0) {
		// save [new] signature
		bcopy(signature, signature_last, sizeof(signature));

		// save [new] configuration
		if (!_dns_configuration_store(&_config)) {
			my_log(LOG_ERR, "dns_configuration_set: could not store configuration");
		}

		changed = TRUE;
	}

	if (_config != NULL) _dns_configuration_free(&_config);

	CFRelease(resolvers);
	return changed;
}


#if	!TARGET_OS_IPHONE
static SCDynamicStoreRef	dns_configuration_store;
static SCDynamicStoreCallBack	dns_configuration_callout;

static void
dns_configuration_changed(CFMachPortRef port, void *msg, CFIndex size, void *info)
{
	CFStringRef	key		= CFSTR(_PATH_RESOLVER_DIR);
	CFArrayRef	keys;
	Boolean		resolvers_now;
	static Boolean	resolvers_save	= FALSE;
	struct stat	statbuf;

	resolvers_now = (stat(_PATH_RESOLVER_DIR, &statbuf) == 0);
	if (!resolvers_save && (resolvers_save == resolvers_now)) {
		// if we did not (and still do not) have an "/etc/resolvers"
		// directory than this notification is the result of a change
		// to the "/etc" directory.
		return;
	}
	resolvers_save = resolvers_now;

	my_log(LOG_DEBUG, _PATH_RESOLVER_DIR " changed");

	// fake a "DNS" change
	keys = CFArrayCreate(NULL, (const void **)&key, 1, &kCFTypeArrayCallBacks);
	(*dns_configuration_callout)(dns_configuration_store, keys, NULL);
	CFRelease(keys);
	return;
}


__private_extern__
void
dns_configuration_monitor(SCDynamicStoreRef store, SCDynamicStoreCallBack callout)
{
	CFMachPortRef		mp;
	mach_port_t		notify_port;
	int			notify_token;
	CFRunLoopSourceRef	rls;
	uint32_t		status;

	dns_configuration_store   = store;
	dns_configuration_callout = callout;

	status = notify_register_mach_port(_PATH_RESOLVER_DIR, &notify_port, 0, &notify_token);
	if (status != NOTIFY_STATUS_OK) {
		my_log(LOG_ERR, "notify_register_mach_port() failed");
		return;
	}

	status = notify_monitor_file(notify_token, "/private" _PATH_RESOLVER_DIR, 0);
	if (status != NOTIFY_STATUS_OK) {
		my_log(LOG_ERR, "notify_monitor_file() failed");
		(void)notify_cancel(notify_token);
		return;
	}

	mp = _SC_CFMachPortCreateWithPort("IPMonitor/dns_configuration",
					  notify_port,
					  dns_configuration_changed,
					  NULL);

	rls = CFMachPortCreateRunLoopSource(NULL, mp, -1);
	if (rls == NULL) {
		my_log(LOG_ERR, "SCDynamicStoreCreateRunLoopSource() failed");
		CFRelease(mp);
		(void)notify_cancel(notify_token);
		return;
	}
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	CFRelease(mp);
	return;
}
#endif	// !TARGET_OS_IPHONE


__private_extern__
void
dns_configuration_init(CFBundleRef bundle)
{
	CFDictionaryRef	dict;

	dict = CFBundleGetInfoDictionary(bundle);
	if (isA_CFDictionary(dict)) {
		S_mdns_timeout = CFDictionaryGetValue(dict, CFSTR("mdns_timeout"));
		S_mdns_timeout = isA_CFNumber(S_mdns_timeout);

		S_pdns_timeout = CFDictionaryGetValue(dict, CFSTR("pdns_timeout"));
		S_pdns_timeout = isA_CFNumber(S_pdns_timeout);
	}

	return;
}


#pragma mark -
#pragma mark Standalone test code


#ifdef	MAIN

static void
split(const void * key, const void * value, void * context)
{
	CFArrayRef		components;
	CFStringRef		entity_id;
	CFStringRef		service_id;
	CFMutableDictionaryRef	state_dict;

	components = CFStringCreateArrayBySeparatingStrings(NULL, (CFStringRef)key, CFSTR("/"));
	service_id = CFArrayGetValueAtIndex(components, 3);
	entity_id  = CFArrayGetValueAtIndex(components, 4);
	state_dict = (CFMutableDictionaryRef)CFDictionaryGetValue(context, service_id);
	if (state_dict != NULL) {
		state_dict = CFDictionaryCreateMutableCopy(NULL, 0, state_dict);
	} else {
		state_dict = CFDictionaryCreateMutable(NULL,
						       0,
						       &kCFTypeDictionaryKeyCallBacks,
						       &kCFTypeDictionaryValueCallBacks);
	}

	if (CFEqual(entity_id, kSCEntNetIPv4) ||
	    CFEqual(entity_id, kSCEntNetIPv6)) {
		CFDictionaryRef		dict;
		CFStringRef		interface;

		if (CFEqual(entity_id, kSCEntNetIPv4)) {
			dict = ipv4_dict_create(value);
		}
		else {
			dict = ipv6_dict_create(value);
		}
		if (dict != NULL) {
			CFDictionarySetValue(state_dict, entity_id, dict);
		}

		interface = CFDictionaryGetValue((CFDictionaryRef)value, kSCPropInterfaceName);
		if (interface != NULL) {
			CFDictionaryRef		dns;
			CFMutableDictionaryRef	new_dns;

			dns = CFDictionaryGetValue(state_dict, kSCEntNetDNS);
			if (dns != NULL) {
				new_dns = CFDictionaryCreateMutableCopy(NULL, 0, dns);
			} else {
				new_dns = CFDictionaryCreateMutable(NULL,
								    0,
								    &kCFTypeDictionaryKeyCallBacks,
								    &kCFTypeDictionaryValueCallBacks);
			}
			CFDictionarySetValue(new_dns, kSCPropInterfaceName, interface);
			CFDictionarySetValue(state_dict, kSCEntNetDNS, new_dns);
			CFRelease(new_dns);
		}
	} else if (CFEqual(entity_id, kSCEntNetDNS)) {
		CFDictionaryRef	dns;

		dns = CFDictionaryGetValue(state_dict, kSCEntNetDNS);
		if (dns != NULL) {
			CFStringRef	interface;

			interface = CFDictionaryGetValue(dns, kSCPropInterfaceName);
			if (interface != NULL) {
				CFMutableDictionaryRef	new_dns;

				new_dns = CFDictionaryCreateMutableCopy(NULL, 0, (CFDictionaryRef)value);
				CFDictionarySetValue(new_dns, kSCPropInterfaceName, interface);
				CFDictionarySetValue(state_dict, kSCEntNetDNS, new_dns);
				CFRelease(new_dns);
			} else {
				CFDictionarySetValue(state_dict, kSCEntNetDNS, (CFDictionaryRef)value);
			}
		} else {
			CFDictionarySetValue(state_dict, kSCEntNetDNS, (CFDictionaryRef)value);
		}
	} else {
		CFDictionarySetValue(state_dict, entity_id, (CFDictionaryRef)value);
	}

	CFDictionarySetValue((CFMutableDictionaryRef)context, service_id, state_dict);
	CFRelease(state_dict);
	CFRelease(components);

	return;
}

int
main(int argc, char **argv)
{
	CFDictionaryRef		entities;
	CFStringRef		key;
	CFArrayRef		multicast_resolvers;
	CFStringRef		pattern;
	CFMutableArrayRef	patterns;
	CFStringRef		primary		= NULL;
	CFDictionaryRef		primaryDNS	= NULL;
	CFArrayRef		private_resolvers;
	CFArrayRef		service_order	= NULL;
	CFMutableDictionaryRef	service_state_dict;
	CFDictionaryRef		setup_global_ipv4;
	CFDictionaryRef		state_global_ipv4;
	SCDynamicStoreRef	store;

	_sc_log     = FALSE;
	_sc_verbose = (argc > 1) ? TRUE : FALSE;

	store = SCDynamicStoreCreate(NULL, CFSTR("TEST"), NULL, NULL);

	// get IPv4, IPv6, and DNS entities
	patterns = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainState,
							      kSCCompAnyRegex,
							      kSCEntNetIPv4);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainState,
							      kSCCompAnyRegex,
							      kSCEntNetIPv6);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainState,
							      kSCCompAnyRegex,
							      kSCEntNetDNS);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);
	entities = SCDynamicStoreCopyMultiple(store, NULL, patterns);
	CFRelease(patterns);

	service_state_dict = CFDictionaryCreateMutable(NULL,
						       0,
						       &kCFTypeDictionaryKeyCallBacks,
						       &kCFTypeDictionaryValueCallBacks);
	CFDictionaryApplyFunction(entities, split, service_state_dict);
	CFRelease(entities);

	// get primary service ID
	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							 kSCDynamicStoreDomainState,
							 kSCEntNetIPv4);
	state_global_ipv4 = SCDynamicStoreCopyValue(store, key);
	CFRelease(key);
	if (state_global_ipv4 != NULL) {
		primary = CFDictionaryGetValue(state_global_ipv4, kSCDynamicStorePropNetPrimaryService);
		if (primary != NULL) {
			CFDictionaryRef	service_dict;

			// get DNS configuration for primary service
			service_dict = CFDictionaryGetValue(service_state_dict, primary);
			if (service_dict != NULL) {
				primaryDNS = CFDictionaryGetValue(service_dict, kSCEntNetDNS);
			}
		}
	}

	// get serviceOrder
	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							 kSCDynamicStoreDomainSetup,
							 kSCEntNetIPv4);
	setup_global_ipv4 = SCDynamicStoreCopyValue(store, key);
	CFRelease(key);
	if (setup_global_ipv4 != NULL) {
		service_order = CFDictionaryGetValue(setup_global_ipv4, kSCPropNetServiceOrder);
	}

	// get multicast resolvers
	key = SCDynamicStoreKeyCreate(NULL, CFSTR("%@/%@/%@"),
				      kSCDynamicStoreDomainState,
				      kSCCompNetwork,
				      CFSTR(kDNSServiceCompMulticastDNS));
	multicast_resolvers = SCDynamicStoreCopyValue(store, key);
	CFRelease(key);

	// get private resolvers
	key = SCDynamicStoreKeyCreate(NULL, CFSTR("%@/%@/%@"),
				      kSCDynamicStoreDomainState,
				      kSCCompNetwork,
				      CFSTR(kDNSServiceCompPrivateDNS));
	private_resolvers = SCDynamicStoreCopyValue(store, key);
	CFRelease(key);

	// update DNS configuration
	dns_configuration_init(CFBundleGetMainBundle());
	(void)dns_configuration_set(primaryDNS,
				    service_state_dict,
				    service_order,
				    multicast_resolvers,
				    private_resolvers);

	// cleanup
	if (setup_global_ipv4 != NULL)	CFRelease(setup_global_ipv4);
	if (state_global_ipv4 != NULL)	CFRelease(state_global_ipv4);
	if (multicast_resolvers != NULL) CFRelease(multicast_resolvers);
	if (private_resolvers != NULL)	CFRelease(private_resolvers);
	CFRelease(service_state_dict);
	CFRelease(store);

	/* not reached */
	exit(0);
	return 0;
}
#endif


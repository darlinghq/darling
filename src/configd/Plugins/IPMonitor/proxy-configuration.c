/*
 * Copyright (c) 2011-2014 Apple Inc. All rights reserved.
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
 * January 3, 2011	Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <TargetConditionals.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include <SystemConfiguration/SCValidation.h>
#include "ip_plugin.h"


#define DEFAULT_MATCH_ORDER    200000   /* match order for the "default" proxy configuration */


#define PROXY_MATCH_ORDER_KEY	CFSTR("__MATCH_ORDER__")
#define ORDER_KEY		CFSTR("__ORDER__")


CFBooleanRef	G_supplemental_proxies_follow_dns	= NULL;


static void
add_proxy(CFMutableArrayRef proxies, CFMutableDictionaryRef proxy)
{
	CFIndex		i;
	CFIndex		n_proxies;
	CFNumberRef	order;

	n_proxies = CFArrayGetCount(proxies);
	for (i = 0; i < n_proxies; i++) {
		CFDictionaryRef		match_proxy;

		match_proxy = CFArrayGetValueAtIndex(proxies, i);
		if (CFEqual(proxy, match_proxy)) {
			// a real duplicate
			return;
		}
	}

	order = CFNumberCreate(NULL, kCFNumberCFIndexType, &n_proxies);
	CFDictionarySetValue(proxy, ORDER_KEY, order);
	CFRelease(order);

	CFArrayAppendValue(proxies, proxy);
	return;
}


static void
add_supplemental(CFMutableArrayRef proxies, CFDictionaryRef proxy, uint32_t defaultOrder)
{
	CFArrayRef	domains;
	CFIndex		i;
	CFIndex		n_domains;
	CFArrayRef	orders;

	domains = CFDictionaryGetValue(proxy, kSCPropNetProxiesSupplementalMatchDomains);
	n_domains = isA_CFArray(domains) ? CFArrayGetCount(domains) : 0;
	if (n_domains == 0) {
		return;
	}

	orders = CFDictionaryGetValue(proxy, kSCPropNetProxiesSupplementalMatchOrders);
	if (orders != NULL) {
		if (!isA_CFArray(orders) || (n_domains != CFArrayGetCount(orders))) {
			return;
		}
	}

	/*
	 * yes, this is a "supplemental" proxy configuration, expand
	 * the match domains and add each to the proxies list.
	 */
	for (i = 0; i < n_domains; i++) {
		CFStringRef		match_domain;
		CFNumberRef		match_order;
		CFMutableDictionaryRef	match_proxy;

		match_domain = CFArrayGetValueAtIndex(domains, i);
		if (!isA_CFString(match_domain)) {
			continue;
		}

		match_proxy = CFDictionaryCreateMutableCopy(NULL, 0, proxy);

		// set supplemental proxy match "domain"
		match_domain = _SC_trimDomain(match_domain);
		if (match_domain != NULL) {
			CFDictionarySetValue(match_proxy, kSCPropNetProxiesSupplementalMatchDomain, match_domain);
			CFRelease(match_domain);
		} else {
			CFDictionaryRemoveValue(match_proxy, kSCPropNetProxiesSupplementalMatchDomain);
		}

		// set supplemental proxy match "order"
		match_order = (orders != NULL) ? CFArrayGetValueAtIndex(orders, i) : NULL;
		if (isA_CFNumber(match_order)) {
			CFDictionarySetValue(match_proxy, PROXY_MATCH_ORDER_KEY, match_order);
		} else {
			CFNumberRef     num;

			num = CFNumberCreate(NULL, kCFNumberIntType, &defaultOrder);
			CFDictionarySetValue(match_proxy, PROXY_MATCH_ORDER_KEY, num);
			CFRelease(num);

			defaultOrder++;		// if multiple domains, maintain ordering
		}

		// remove keys we don't want in a supplemental proxy
		CFDictionaryRemoveValue(match_proxy, kSCPropNetProxiesSupplementalMatchDomains);
		CFDictionaryRemoveValue(match_proxy, kSCPropNetProxiesSupplementalMatchOrders);
		CFDictionaryRemoveValue(match_proxy, kSCPropInterfaceName);

		add_proxy(proxies, match_proxy);
		CFRelease(match_proxy);
	}

	return;
}


#define	N_QUICK	32


static void
add_supplemental_proxies(CFMutableArrayRef proxies, CFDictionaryRef services, CFArrayRef service_order)
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
		CFDictionaryRef		proxy;
		CFMutableDictionaryRef	proxyWithDNS	= NULL;
		CFDictionaryRef		service		= (CFDictionaryRef)vals[i];

		if (!isA_CFDictionary(service)) {
			continue;
		}

		proxy = CFDictionaryGetValue(service, kSCEntNetProxies);
		if (!isA_CFDictionary(proxy)) {
			continue;
		}

		if ((G_supplemental_proxies_follow_dns != NULL) && CFBooleanGetValue(G_supplemental_proxies_follow_dns)) {
			CFDictionaryRef	dns;
			CFArrayRef	matchDomains;
			CFArrayRef	matchOrders;

			if (!CFDictionaryContainsKey(proxy, kSCPropNetProxiesSupplementalMatchDomains) &&
			    CFDictionaryGetValueIfPresent(service, kSCEntNetDNS, (const void **)&dns) &&
			    isA_CFDictionary(dns) &&
			    CFDictionaryGetValueIfPresent(dns, kSCPropNetDNSSupplementalMatchDomains, (const void **)&matchDomains) &&
			    isA_CFArray(matchDomains)) {
				proxyWithDNS = CFDictionaryCreateMutableCopy(NULL, 0, proxy);
				CFDictionarySetValue(proxyWithDNS, kSCPropNetProxiesSupplementalMatchDomains, matchDomains);
				if (CFDictionaryGetValueIfPresent(dns, kSCPropNetDNSSupplementalMatchOrders, (const void **)&matchOrders) &&
				    isA_CFArray(matchOrders)) {
					CFDictionarySetValue(proxyWithDNS, kSCPropNetProxiesSupplementalMatchOrders, matchOrders);
				} else {
					CFDictionaryRemoveValue(proxyWithDNS, kSCPropNetProxiesSupplementalMatchOrders);
				}
				proxy = proxyWithDNS;
			}
		}

		defaultOrder = DEFAULT_MATCH_ORDER
			       - (DEFAULT_MATCH_ORDER / 2)
			       + ((DEFAULT_MATCH_ORDER / 1000) * (uint32_t)i);
		if ((n_order > 0) &&
		    !CFArrayContainsValue(service_order, CFRangeMake(0, n_order), keys[i])) {
			// push out services not specified in service order
			defaultOrder += (DEFAULT_MATCH_ORDER / 1000) * n_services;
		}

		add_supplemental(proxies, proxy, defaultOrder);
		if (proxyWithDNS != NULL) CFRelease(proxyWithDNS);
	}

	if (keys != keys_q) {
		CFAllocatorDeallocate(NULL, keys);
		CFAllocatorDeallocate(NULL, vals);
	}

	return;
}


static CFComparisonResult
compareBySearchOrder(const void *val1, const void *val2, void *context)
{
	CFDictionaryRef	proxy1	= (CFDictionaryRef)val1;
	CFDictionaryRef	proxy2	= (CFDictionaryRef)val2;
	CFNumberRef	num1;
	CFNumberRef	num2;
	uint32_t	order1	= DEFAULT_MATCH_ORDER;
	uint32_t	order2	= DEFAULT_MATCH_ORDER;

	num1 = CFDictionaryGetValue(proxy1, PROXY_MATCH_ORDER_KEY);
	if (!isA_CFNumber(num1) ||
	    !CFNumberGetValue(num1, kCFNumberIntType, &order1)) {
		order1 = DEFAULT_MATCH_ORDER;
	}

	num2 = CFDictionaryGetValue(proxy2, PROXY_MATCH_ORDER_KEY);
	if (!isA_CFNumber(num2) ||
	    !CFNumberGetValue(num2, kCFNumberIntType, &order2)) {
		order2 = DEFAULT_MATCH_ORDER;
	}

	if (order1 == order2) {
		// if same match "order", retain original ordering for configurations
		if (CFDictionaryGetValueIfPresent(proxy1, ORDER_KEY, (const void **)&num1) &&
		    CFDictionaryGetValueIfPresent(proxy2, ORDER_KEY, (const void **)&num2) &&
		    isA_CFNumber(num1) &&
		    isA_CFNumber(num2) &&
		    CFNumberGetValue(num1, kCFNumberIntType, &order1) &&
		    CFNumberGetValue(num2, kCFNumberIntType, &order2)) {
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


static __inline__ Boolean
isSupplementalProxy(CFDictionaryRef proxy)
{
	if ((proxy != NULL) &&
	    CFDictionaryContainsKey(proxy, kSCPropNetProxiesSupplementalMatchDomain)) {
		return TRUE;
	}

	return FALSE;
}


static CFArrayRef
copy_supplemental_proxies(CFArrayRef proxies, Boolean skip)
{
	CFIndex			i;
	CFIndex			n_proxies;
	CFMutableArrayRef	supplemental	= NULL;

	// iterate over services

	n_proxies = isA_CFArray(proxies) ? CFArrayGetCount(proxies) : 0;
	for (i = 0; i < n_proxies; i++) {
		CFDictionaryRef		proxy;

		proxy = CFArrayGetValueAtIndex(proxies, i);
		if (!isSupplementalProxy(proxy)) {
			// if not supplemental proxy (i.e. no match domain)
			continue;
		}

		// add [supplemental] proxy entry
		if (supplemental == NULL) {
			supplemental = CFArrayCreateMutable(NULL,
							    0,
							    &kCFTypeArrayCallBacks);
		}
		CFArrayAppendValue(supplemental, proxy);
	}

	return supplemental;
}


static CFArrayRef
service_order_copy_all(CFDictionaryRef services, CFArrayRef service_order)
{
	const void *		keys_q[N_QUICK];
	const void **		keys	= keys_q;
	CFIndex			i;
	CFIndex			n_order;
	CFIndex			n_services;
	CFMutableArrayRef	order;

	// ensure that we process all services in order
	n_services = isA_CFDictionary(services) ? CFDictionaryGetCount(services) : 0;
	if (n_services == 0) {
		// if no services
		return NULL;
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
		CFStringRef	serviceID	= (CFStringRef)keys[i];

		if (!CFArrayContainsValue(order, CFRangeMake(0, n_order), serviceID)) {
			CFArrayAppendValue(order, serviceID);
			n_order++;
		}
	}
	if (keys != keys_q) {
		CFAllocatorDeallocate(NULL, keys);
	}

	return order;
}


static CFDictionaryRef
copy_app_layer_vpn_proxies(CFDictionaryRef services, CFArrayRef order, CFDictionaryRef services_info)
{
	CFMutableDictionaryRef	app_layer_proxies	= NULL;
	CFIndex			i;
	CFIndex			n_order;

	if (!isA_CFDictionary(services_info)) {
		return NULL;
	}

	// iterate over services

	n_order = isA_CFArray(order) ? CFArrayGetCount(order) : 0;
	for (i = 0; i < n_order; i++) {
		CFMutableDictionaryRef	newProxy;
		CFDictionaryRef		proxy;
		CFDictionaryRef		service;
		CFStringRef		serviceID;
		CFDictionaryRef		vpn;
		CFStringRef		vpn_key;

		serviceID = CFArrayGetValueAtIndex(order, i);
		service = CFDictionaryGetValue(services, serviceID);
		if (!isA_CFDictionary(service)) {
			// if no service
			continue;
		}

		proxy = CFDictionaryGetValue(service, kSCEntNetProxies);
		if (!isA_CFDictionary(proxy)) {
			// if no proxy
			continue;
		}

		vpn_key = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
								      kSCDynamicStoreDomainSetup,
								      serviceID,
								      kSCEntNetVPN);
		vpn = CFDictionaryGetValue(services_info, vpn_key);
		CFRelease(vpn_key);

		if (!isA_CFDictionary(vpn) || !CFDictionaryContainsKey(vpn, kSCPropNetVPNAppRules)) {
			// if not App Layer vpn
			continue;
		}

		if ((app_layer_proxies != NULL) &&
		    CFDictionaryContainsKey(app_layer_proxies, serviceID)) {
			// if we've already processed this [app_layer_proxies] interface
			continue;
		}

		// add [app_layer_proxies] proxy entry
		newProxy = CFDictionaryCreateMutableCopy(NULL, 0, proxy);
		CFDictionaryRemoveValue(newProxy, kSCPropNetProxiesSupplementalMatchDomains);
		CFDictionaryRemoveValue(newProxy, kSCPropNetProxiesSupplementalMatchOrders);
		if (app_layer_proxies == NULL) {
			app_layer_proxies = CFDictionaryCreateMutable(NULL,
								      0,
								      &kCFTypeDictionaryKeyCallBacks,
								      &kCFTypeDictionaryValueCallBacks);
		}
		CFDictionarySetValue(app_layer_proxies, serviceID, newProxy);
		CFRelease(newProxy);
	}

	return app_layer_proxies;
}


static CFDictionaryRef
copy_scoped_proxies(CFDictionaryRef services, CFArrayRef order)
{
	CFIndex			i;
	CFIndex			n_order;
	CFMutableDictionaryRef	scoped	= NULL;

	// iterate over services

	n_order = isA_CFArray(order) ? CFArrayGetCount(order) : 0;
	for (i = 0; i < n_order; i++) {
		char			if_name[IF_NAMESIZE];
		CFStringRef		interface;
		CFMutableDictionaryRef	newProxy;
		CFDictionaryRef		proxy;
		CFDictionaryRef		service;
		CFStringRef		serviceID;

		serviceID = CFArrayGetValueAtIndex(order, i);
		service = CFDictionaryGetValue(services, serviceID);
		if (!isA_CFDictionary(service)) {
			// if no service
			continue;
		}

		proxy = CFDictionaryGetValue(service, kSCEntNetProxies);
		if (!isA_CFDictionary(proxy)) {
			// if no proxy
			continue;
		}

		interface = CFDictionaryGetValue(proxy, kSCPropInterfaceName);
		if (interface == NULL) {
			// if no [scoped] interface
			continue;
		}
		if ((scoped != NULL) &&
		    CFDictionaryContainsKey(scoped, interface)) {
			// if we've already processed this [scoped] interface
			continue;
		}

		if ((_SC_cfstring_to_cstring(interface,
					     if_name,
					     sizeof(if_name),
					     kCFStringEncodingASCII) == NULL) ||
		    ((my_if_nametoindex(if_name)) == 0)) {
			// if interface index not available
			continue;
		}

		// add [scoped] proxy entry
		// ... and remove keys we don't want in a [scoped] proxy
		CFRetain(interface);
		newProxy = CFDictionaryCreateMutableCopy(NULL, 0, proxy);
		CFDictionaryRemoveValue(newProxy, kSCPropNetProxiesSupplementalMatchDomains);
		CFDictionaryRemoveValue(newProxy, kSCPropNetProxiesSupplementalMatchOrders);
		CFDictionaryRemoveValue(newProxy, kSCPropInterfaceName);
		if (scoped == NULL) {
			scoped = CFDictionaryCreateMutable(NULL,
							   0,
							   &kCFTypeDictionaryKeyCallBacks,
							   &kCFTypeDictionaryValueCallBacks);
		}
		CFDictionarySetValue(scoped, interface, newProxy);
		CFRelease(newProxy);
		CFRelease(interface);
	}

	return scoped;
}


static void
add_default_proxy(CFMutableArrayRef	proxies,
		  CFDictionaryRef	defaultProxy,
		  Boolean		*orderAdded)
{
	CFMutableDictionaryRef	myDefault;
	uint32_t		myOrder	= DEFAULT_MATCH_ORDER;
	CFNumberRef		order	= NULL;

	if (defaultProxy == NULL) {
		myDefault = CFDictionaryCreateMutable(NULL,
						      0,
						      &kCFTypeDictionaryKeyCallBacks,
						      &kCFTypeDictionaryValueCallBacks);
	} else {
		myDefault = CFDictionaryCreateMutableCopy(NULL, 0, defaultProxy);
		CFDictionaryRemoveValue(myDefault, kSCPropInterfaceName);
		order = CFDictionaryGetValue(myDefault, PROXY_MATCH_ORDER_KEY);
	}

	// ensure that the default proxy has a search order

	if (!isA_CFNumber(order) ||
	    !CFNumberGetValue(order, kCFNumberIntType, &myOrder)) {
		myOrder = DEFAULT_MATCH_ORDER;
		order = CFNumberCreate(NULL, kCFNumberIntType, &myOrder);
		CFDictionarySetValue(myDefault, PROXY_MATCH_ORDER_KEY, order);
		CFRelease(order);
		*orderAdded = TRUE;
	}

	// add the default proxy

	add_proxy(proxies, myDefault);
	CFRelease(myDefault);
	return;
}


static CFComparisonResult
compareDomain(const void *val1, const void *val2, void *context)
{
	CFDictionaryRef		proxy1	= (CFDictionaryRef)val1;
	CFDictionaryRef		proxy2	= (CFDictionaryRef)val2;
	CFStringRef		domain1;
	CFStringRef		domain2;
	CFArrayRef		labels1	= NULL;
	CFArrayRef		labels2	= NULL;
	CFIndex			n1;
	CFIndex			n2;
	CFComparisonResult	result;
	Boolean			rev1;
	Boolean			rev2;

	// "default" domains sort before "supplemental" domains
	domain1 = CFDictionaryGetValue(proxy1, kSCPropNetProxiesSupplementalMatchDomain);
	domain2 = CFDictionaryGetValue(proxy2, kSCPropNetProxiesSupplementalMatchDomain);
	if (domain1 == NULL) {
		if (domain2 == NULL) {
			return kCFCompareEqualTo;
		}
		return kCFCompareLessThan;
	} else if (domain2 == NULL) {
		return kCFCompareGreaterThan;
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
CF_RETURNS_RETAINED CFDictionaryRef
proxy_configuration_update(CFDictionaryRef	defaultProxy,
			   CFDictionaryRef	services,
			   CFArrayRef		serviceOrder,
			   CFDictionaryRef	servicesInfo)
{
	CFIndex			i;
	CFMutableDictionaryRef	myDefault;
	Boolean			myOrderAdded	= FALSE;
	CFMutableDictionaryRef	newProxy	= NULL;
	CFIndex			n_proxies;
	CFDictionaryRef		proxy;
	CFMutableArrayRef	proxies;

	// establish full list of proxies

	proxies = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	// collect (and add) any "supplemental" proxy configurations

	add_supplemental_proxies(proxies, services, serviceOrder);

	// add the "default" proxy

	add_default_proxy(proxies, defaultProxy, &myOrderAdded);

	// sort proxies, cleanup

	n_proxies = CFArrayGetCount(proxies);
	if (n_proxies > 1) {
		CFArraySortValues(proxies, CFRangeMake(0, n_proxies), compareDomain, NULL);
	}

	// cleanup

	for (i = n_proxies - 1; i >= 0; i--) {
		proxy = CFArrayGetValueAtIndex(proxies, i);

		if ((i > 0) &&
		    !CFDictionaryContainsKey(proxy, kSCPropNetProxiesSupplementalMatchDomain)) {
			// remove non-supplemental proxy
			CFArrayRemoveValueAtIndex(proxies, i);
			n_proxies--;
			continue;
		}

		newProxy = CFDictionaryCreateMutableCopy(NULL, 0, proxy);
		CFDictionaryRemoveValue(newProxy, PROXY_MATCH_ORDER_KEY);
		CFDictionaryRemoveValue(newProxy, ORDER_KEY);
		CFArraySetValueAtIndex(proxies, i, newProxy);
		CFRelease(newProxy);
	}

	// update the default proxy

	myDefault = CFDictionaryCreateMutableCopy(NULL,
						  0,
						  CFArrayGetValueAtIndex(proxies, 0));
	if (myOrderAdded && (n_proxies > 1)) {
		CFDictionaryRef	proxy;

		proxy = CFArrayGetValueAtIndex(proxies, 1);
		if (CFDictionaryContainsKey(proxy, kSCPropNetProxiesSupplementalMatchDomain)) {
			// if not a supplemental "default" proxy (a match domain name is
			// present)
			CFDictionaryRemoveValue(myDefault, PROXY_MATCH_ORDER_KEY);
		}
	}
	CFArraySetValueAtIndex(proxies, 0, myDefault);
	CFRelease(myDefault);

	// establish proxy configuration

	if (n_proxies > 0) {
		CFDictionaryRef		app_layer;
		CFDictionaryRef		scoped;
		CFArrayRef		serviceOrderAll;
		Boolean			skip		= FALSE;
		CFArrayRef		supplemental;

		proxy = CFArrayGetValueAtIndex(proxies, 0);
		if (!CFDictionaryContainsKey(proxy, kSCPropNetProxiesSupplementalMatchDomain)) {
			// if we have "a" default (non-supplemental) proxy
			newProxy = CFDictionaryCreateMutableCopy(NULL, 0, proxy);
			CFDictionaryRemoveValue(newProxy, kSCPropNetProxiesSupplementalMatchDomains);
			CFDictionaryRemoveValue(newProxy, kSCPropNetProxiesSupplementalMatchOrders);
			skip = TRUE;
		} else {
			newProxy = CFDictionaryCreateMutable(NULL,
							     0,
							     &kCFTypeDictionaryKeyCallBacks,
							     &kCFTypeDictionaryValueCallBacks);
		}

		serviceOrderAll = service_order_copy_all(services, serviceOrder);

		// collect (and add) any "supplemental" proxy configurations

		supplemental = copy_supplemental_proxies(proxies, skip);
		if (supplemental != NULL) {
			CFDictionarySetValue(newProxy, kSCPropNetProxiesSupplemental, supplemental);
			CFRelease(supplemental);
		}

		// collect (and add) any "scoped" proxy configurations

		scoped = copy_scoped_proxies(services, serviceOrderAll);
		if (scoped != NULL) {
			CFDictionarySetValue(newProxy, kSCPropNetProxiesScoped, scoped);
			CFRelease(scoped);
		}

		// collect (and add) any "services" based proxy configurations

		app_layer = copy_app_layer_vpn_proxies(services, serviceOrderAll, servicesInfo);
		if (app_layer != NULL) {
			CFDictionarySetValue(newProxy, kSCPropNetProxiesServices, app_layer);
			CFRelease(app_layer);
		}

		if (serviceOrderAll != NULL) {
			CFRelease(serviceOrderAll);
		}
	} else {
		newProxy = NULL;
	}

	CFRelease(proxies);
	return newProxy;
}


__private_extern__
void
proxy_configuration_init(CFBundleRef bundle)
{
	CFDictionaryRef	dict;

	dict = CFBundleGetInfoDictionary(bundle);
	if (isA_CFDictionary(dict)) {
		G_supplemental_proxies_follow_dns = CFDictionaryGetValue(dict, CFSTR("SupplementalProxiesFollowSupplementalDNS"));
		G_supplemental_proxies_follow_dns = isA_CFBoolean(G_supplemental_proxies_follow_dns);
	}

	return;
}


#pragma mark -
#pragma mark Standalone test code


#ifdef	MAIN

static void
mergeDict(const void *key, const void *value, void *context)
{
	CFMutableDictionaryRef	newDict	= (CFMutableDictionaryRef)context;

	CFDictionarySetValue(newDict, key, value);
	return;
}


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
		CFStringRef	interface;

		interface = CFDictionaryGetValue((CFDictionaryRef)value, kSCPropInterfaceName);
		if (interface != NULL) {
			CFDictionaryRef		proxy;
			CFMutableDictionaryRef	new_proxy;

			proxy = CFDictionaryGetValue(state_dict, kSCEntNetProxies);
			if (proxy != NULL) {
				new_proxy = CFDictionaryCreateMutableCopy(NULL, 0, proxy);
			} else {
				new_proxy = CFDictionaryCreateMutable(NULL,
								0,
								&kCFTypeDictionaryKeyCallBacks,
								&kCFTypeDictionaryValueCallBacks);
			}
			CFDictionarySetValue(new_proxy, kSCPropInterfaceName, interface);
			CFDictionarySetValue(state_dict, kSCEntNetProxies, new_proxy);
			CFRelease(new_proxy);
		}
	} else if (CFEqual(entity_id, kSCEntNetProxies)) {
		CFDictionaryRef	proxy;

		proxy = CFDictionaryGetValue(state_dict, kSCEntNetProxies);
		if (proxy != NULL) {
			CFStringRef		domain;
			CFMutableDictionaryRef	new_proxy;

			// if we already have some Setup: or State: proxy content
			domain = CFArrayGetValueAtIndex(components, 0);
			if (CFEqual(domain, kSCDynamicStoreDomainState)) {
				// if we've already seen the Setup: key
				new_proxy = CFDictionaryCreateMutableCopy(NULL, 0, (CFDictionaryRef)value);
				CFDictionaryApplyFunction(proxy, mergeDict, new_proxy);
			} else {
				// if we've already seen the State: key
				new_proxy = CFDictionaryCreateMutableCopy(NULL, 0, proxy);
				CFDictionaryApplyFunction((CFDictionaryRef)value, mergeDict, new_proxy);
			}
			CFDictionarySetValue(state_dict, kSCEntNetProxies, new_proxy);
			CFRelease(new_proxy);
		} else {
			CFDictionarySetValue(state_dict, kSCEntNetProxies, (CFDictionaryRef)value);
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
	CFDictionaryRef		newProxy	= NULL;
	CFStringRef		pattern;
	CFMutableArrayRef	patterns;
	CFStringRef		primary		= NULL;
	CFMutableDictionaryRef	primary_proxy	= NULL;
	CFArrayRef		service_order	= NULL;
	CFMutableDictionaryRef	service_state_dict;
	CFDictionaryRef		setup_global_ipv4;
	CFDictionaryRef		state_global_ipv4;
	SCDynamicStoreRef	store;

	_sc_log     = FALSE;
	_sc_verbose = (argc > 1) ? TRUE : FALSE;

	store = SCDynamicStoreCreate(NULL, CFSTR("TEST"), NULL, NULL);

	// get IPv4, IPv6, and Proxies entities
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
							      kSCDynamicStoreDomainSetup,
							      kSCCompAnyRegex,
							      kSCEntNetProxies);
	CFArrayAppendValue(patterns, pattern);
	CFRelease(pattern);
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCDynamicStoreDomainState,
							      kSCCompAnyRegex,
							      kSCEntNetProxies);
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

			// get proxy configuration for primary service
			service_dict = CFDictionaryGetValue(service_state_dict, primary);
			if (service_dict != NULL) {
				CFDictionaryRef	service_proxy;

				service_proxy = CFDictionaryGetValue(service_dict, kSCEntNetProxies);
				if (service_proxy != NULL) {
					primary_proxy = CFDictionaryCreateMutableCopy(NULL, 0, service_proxy);
					CFDictionaryRemoveValue(primary_proxy, kSCPropInterfaceName);
				}
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

	// update proxy configuration
	proxy_configuration_init(CFBundleGetMainBundle());
	newProxy = proxy_configuration_update(primary_proxy,
					      service_state_dict,
					      service_order,
					      NULL);
	if (newProxy != NULL) {
		SCPrint(TRUE, stdout, CFSTR("%@\n"), newProxy);
		CFRelease(newProxy);
	}

	// cleanup
	if (setup_global_ipv4 != NULL)	CFRelease(setup_global_ipv4);
	if (state_global_ipv4 != NULL)	CFRelease(state_global_ipv4);
	CFRelease(service_state_dict);
	CFRelease(store);

	/* not reached */
	exit(0);
	return 0;
}
#endif


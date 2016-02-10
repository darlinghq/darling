/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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
 * March 15, 2013		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include <SystemConfiguration/SCValidation.h>

#include <dns_sd.h>
#ifndef	kDNSServiceCompMulticastDNS
#define	kDNSServiceCompMulticastDNS	"MulticastDNS"
#endif
#ifndef	kDNSServiceCompPrivateDNS
#define	kDNSServiceCompPrivateDNS	"PrivateDNS"
#endif

#include "cache.h"


static Boolean			_verbose	= FALSE;
static CFMutableArrayRef	mirror_keys	= NULL;
static CFMutableArrayRef	mirror_patterns	= NULL;
static SCDynamicStoreRef	store_host	= NULL;
static SCDynamicStoreRef	store_sim	= NULL;


#pragma mark -
#pragma mark iOS Simulator Support


static void
mirror(SCDynamicStoreRef store, CFArrayRef changes, void *info)
{
	CFDictionaryRef	content_host;
	CFDictionaryRef	content_sim;
	CFIndex		i;
	CFIndex		n;

	n = CFArrayGetCount(changes);
	if (n == 0) {
		// if no changes
		return;
	}

	// capture "host" content
	if (info == NULL) {
		content_host = SCDynamicStoreCopyMultiple(store_host, changes, NULL);
	} else {
		content_host = (CFDictionaryRef)info;
	}

	// capture [current] "sim" content
	content_sim = SCDynamicStoreCopyMultiple(store_sim, changes, NULL);

	// update
	cache_open();
	for (i = 0; i < n; i++) {
		CFStringRef		key;
		CFPropertyListRef	val;

		key = CFArrayGetValueAtIndex(changes, i);
		val = (content_host != NULL) ? CFDictionaryGetValue(content_host, key) : NULL;
		if (val != NULL) {
			// if "host" content changed
			cache_SCDynamicStoreSetValue(store_sim, key, val);
		} else {
			// if no "host" content
			val = (content_sim != NULL) ? CFDictionaryGetValue(content_sim, key) : NULL;
			if (val != NULL) {
				// if we need to remove the "sim" content
				cache_SCDynamicStoreRemoveValue(store_sim, key);
			} else {
				// if no "sim" content to remove, just notify
				cache_SCDynamicStoreNotifyValue(store_sim, key);
			}
		}
	}
	cache_write(store_sim);
	cache_close();

	// cleanup
	if ((info == NULL) && (content_host != NULL)) {
		CFRelease(content_host);
	}
	if (content_sim != NULL) {
		CFRelease(content_sim);
	}

	return;
}


static void
mirror_setup()
{
	CFStringRef	key;
	CFStringRef	pattern;

	mirror_keys     = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	mirror_patterns = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	// Plugin:InterfaceNamer
	key = SCDynamicStoreKeyCreate(NULL,
				      CFSTR("%@" "InterfaceNamer"),
				      kSCDynamicStoreDomainPlugin);
	CFArrayAppendValue(mirror_keys, key);
	CFRelease(key);

	// Setup:/System
//	key = SCDynamicStoreKeyCreateComputerName(NULL);
//	CFArrayAppendValue(mirror_keys, key);
//	CFRelease(key);

	// Setup:/Network
//	key = SCDynamicStoreKeyCreateHostNames(NULL);
//	CFArrayAppendValue(mirror_keys, key);
//	CFRelease(key);

	// Setup:/Network/Global/.*
	pattern = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							     kSCDynamicStoreDomainSetup,
							     kSCCompAnyRegex);
	CFArrayAppendValue(mirror_patterns, pattern);
	CFRelease(pattern);

	// Setup:/Network/Service/.*
	// State:/Network/Service/.*
	pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							      kSCCompAnyRegex,
							      CFSTR(".*"),
							      NULL);
	CFArrayAppendValue(mirror_patterns, pattern);
	CFRelease(pattern);

	// Setup:/Network/Interface/.*
	// State:/Network/Interface/.*
	pattern = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
								kSCCompAnyRegex,
								CFSTR(".*"),
								NULL);
	CFArrayAppendValue(mirror_patterns, pattern);
	CFRelease(pattern);

	// State:/Network/MulticastDNS
	key = SCDynamicStoreKeyCreate(NULL, CFSTR("%@/%@/%@"),
				      kSCDynamicStoreDomainState,
				      kSCCompNetwork,
				      CFSTR(kDNSServiceCompMulticastDNS));
	CFArrayAppendValue(mirror_keys, key);
	CFRelease(key);

	// State:/Network/PrivateDNS
	key = SCDynamicStoreKeyCreate(NULL, CFSTR("%@/%@/%@"),
				      kSCDynamicStoreDomainState,
				      kSCCompNetwork,
				      CFSTR(kDNSServiceCompPrivateDNS));
	CFArrayAppendValue(mirror_keys, key);
	CFRelease(key);

	return;
}


#define	N_QUICK	64


__private_extern__
void
prime_SimulatorSupport()
{
	CFDictionaryRef	content_host;
	CFIndex		n;

	SCLog(_verbose, LOG_DEBUG, CFSTR("prime() called"));

	// copy current content from base OS store to _Sim store
	content_host = SCDynamicStoreCopyMultiple(store_host, mirror_keys, mirror_patterns);
	CFRelease(mirror_keys);
	mirror_keys = NULL;
	CFRelease(mirror_patterns);
	mirror_patterns = NULL;

	if (content_host == NULL) {
		return;
	}

	n = CFDictionaryGetCount(content_host);
	if (n > 0) {
		CFArrayRef	changes;
		const void *	keys_host_q[N_QUICK];
		const void **	keys_host	= keys_host_q;

		if (n > (CFIndex)(sizeof(keys_host_q) / sizeof(CFStringRef))) {
			keys_host = CFAllocatorAllocate(NULL, n * sizeof(CFStringRef), 0);
		}

		CFDictionaryGetKeysAndValues(content_host, keys_host, NULL);

		changes = CFArrayCreate(NULL, keys_host, n, &kCFTypeArrayCallBacks);
		mirror(store_host, changes, (void *)content_host);
		CFRelease(changes);

		if (keys_host != keys_host_q) {
			CFAllocatorDeallocate(NULL, keys_host);
		}
	}

	CFRelease(content_host);
	return;
}


__private_extern__
void
load_SimulatorSupport(CFBundleRef bundle, Boolean bundleVerbose)
{
	Boolean			ok;
	CFMutableDictionaryRef	options;
	CFRunLoopSourceRef	rls;

	if (bundleVerbose) {
		_verbose = TRUE;
	}

	SCLog(_verbose, LOG_DEBUG, CFSTR("load() called"));
	SCLog(_verbose, LOG_DEBUG, CFSTR("  bundle ID = %@"), CFBundleGetIdentifier(bundle));

	// setup
	mirror_setup();

	// setup SCDynamicStore mirroring (from "host")
	options = CFDictionaryCreateMutable(NULL,
					    0,
					    &kCFTypeDictionaryKeyCallBacks,
					    &kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(options, kSCDynamicStoreUseHostServer, kCFBooleanTrue);
	store_host = SCDynamicStoreCreateWithOptions(NULL,
						     CFSTR("SimulatorSupport(host)"),
						     options,
						     mirror,
						     NULL);
	CFRelease(options);
	assert(store_host != NULL);

	ok = SCDynamicStoreSetNotificationKeys(store_host, mirror_keys, mirror_patterns);
	assert(ok);

	rls = SCDynamicStoreCreateRunLoopSource(NULL, store_host, 0);
	assert(rls != NULL);

	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	// setup SCDynamicStore mirroring (to "iOS Simulator")
	store_sim = SCDynamicStoreCreate(NULL,
					 CFSTR("SimulatorSupport(sim)"),
					 NULL,
					 NULL);

	return;
}


#ifdef	MAIN


#pragma mark -
#pragma mark Standalone test code


int
main(int argc, char **argv)
{
	_sc_log     = FALSE;
	_sc_verbose = (argc > 1) ? TRUE : FALSE;

	load_SimulatorSupport(CFBundleGetMainBundle(), (argc > 1) ? TRUE : FALSE);
	prime_SimulatorSupport();
	CFRunLoopRun();
	// not reached
	exit(0);
	return 0;
}
#endif

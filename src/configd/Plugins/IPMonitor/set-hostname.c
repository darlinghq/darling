/*
 * Copyright (c) 2004-2018 Apple Inc. All rights reserved.
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
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb_async.h>

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCDynamicStoreCopyDHCPInfo.h>
#include <SystemConfiguration/SCValidation.h>
#include <SystemConfiguration/SCPrivate.h>

#include <notify.h>

#ifdef	MAIN
#define	my_log(__level, __format, ...)	SCPrint(TRUE, stdout, CFSTR(__format "\n"), ## __VA_ARGS__)
#else	// MAIN
#include "ip_plugin.h"
#endif	// MAIN

static SCDynamicStoreRef	store		= NULL;
static CFRunLoopRef		rl		= NULL;
static CFRunLoopSourceRef	rls		= NULL;
static dispatch_queue_t		queue		= NULL;

static int			notify_token	= -1;

static struct timeval		ptrQueryStart;
static SCNetworkReachabilityRef	ptrTarget	= NULL;


#define	HOSTNAME_NOTIFY_KEY	"com.apple.system.hostname"
#define SET_HOSTNAME_QUEUE	"com.apple.config.set-hostname"

CFStringRef copy_dhcp_hostname(CFStringRef serviceID);

static void
set_hostname(CFStringRef hostname)
{
	if (hostname != NULL) {
		char	old_name[MAXHOSTNAMELEN];
		char	new_name[MAXHOSTNAMELEN];

		if (gethostname(old_name, sizeof(old_name)) == -1) {
			my_log(LOG_ERR, "gethostname() failed: %s", strerror(errno));
			old_name[0] = '\0';
		}

		if (_SC_cfstring_to_cstring(hostname,
					    new_name,
					    sizeof(new_name),
					    kCFStringEncodingUTF8) == NULL) {
			my_log(LOG_NOTICE, "could not convert [new] hostname");
			new_name[0] = '\0';
		}

		old_name[sizeof(old_name)-1] = '\0';
		new_name[sizeof(new_name)-1] = '\0';
		if (strcmp(old_name, new_name) != 0) {
			if (sethostname(new_name, (int)strlen(new_name)) == 0) {
				uint32_t	status;

				my_log(LOG_NOTICE,
				       "setting hostname to \"%s\"",
				       new_name);

				status = notify_post(HOSTNAME_NOTIFY_KEY);
				if (status != NOTIFY_STATUS_OK) {
					my_log(LOG_ERR,
					       "notify_post(" HOSTNAME_NOTIFY_KEY ") failed: error=%u",
					       status);
				}
			} else {
				my_log(LOG_ERR,
				       "sethostname(%s, %ld) failed: %s",
				       new_name,
				       strlen(new_name),
				       strerror(errno));
			}
		}
	}

	return;
}


static CFStringRef
copy_prefs_hostname(SCDynamicStoreRef store)
{
	CFDictionaryRef		dict;
	CFStringRef		key;
	CFStringRef		name		= NULL;

	key  = SCDynamicStoreKeyCreateComputerName(NULL);
	dict = SCDynamicStoreCopyValue(store, key);
	CFRelease(key);
	if (dict == NULL) {
		goto done;
	}
	if (!isA_CFDictionary(dict)) {
		goto done;
	}

	name = isA_CFString(CFDictionaryGetValue(dict, kSCPropSystemHostName));
	if (name == NULL) {
		goto done;
	}
	CFRetain(name);

    done :

	if (dict != NULL)	CFRelease(dict);

	return name;
}


static CFStringRef
copy_primary_service(SCDynamicStoreRef store)
{
	CFDictionaryRef	dict;
	CFStringRef	key;
	CFStringRef	serviceID	= NULL;

	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							 kSCDynamicStoreDomainState,
							 kSCEntNetIPv4);
	dict = SCDynamicStoreCopyValue(store, key);
	CFRelease(key);

	if (dict != NULL) {
		if (isA_CFDictionary(dict)) {
			serviceID = CFDictionaryGetValue(dict, kSCDynamicStorePropNetPrimaryService);
			if (isA_CFString(serviceID)) {
				CFRetain(serviceID);
			} else {
				serviceID = NULL;
			}
		}
		CFRelease(dict);
	}

	return serviceID;
}


static CFStringRef
copy_primary_ip(SCDynamicStoreRef store, CFStringRef serviceID)
{
	CFDictionaryRef	dict;
	CFStringRef	key;
	CFStringRef	address	= NULL;

	key = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							  kSCDynamicStoreDomainState,
							  serviceID,
							  kSCEntNetIPv4);
	dict = SCDynamicStoreCopyValue(store, key);
	CFRelease(key);

	if (dict != NULL) {
		if (isA_CFDictionary(dict)) {
			CFArrayRef	addresses;

			addresses = CFDictionaryGetValue(dict, kSCPropNetIPv4Addresses);
			if (isA_CFArray(addresses) && (CFArrayGetCount(addresses) > 0)) {
				address = CFArrayGetValueAtIndex(addresses, 0);
				if (isA_CFString(address)) {
					CFRetain(address);
				} else {
					address = NULL;
				}
			}
		}
		CFRelease(dict);
	}

	return address;
}


static void
ptr_query_stop()
{
	if (ptrTarget == NULL) {
		return;
	}

	my_log(LOG_INFO, "hostname: ptr query stop");

	SCNetworkReachabilitySetCallback(ptrTarget, NULL, NULL);
	SCNetworkReachabilityUnscheduleFromRunLoop(ptrTarget, rl, kCFRunLoopDefaultMode);
	CFRelease(ptrTarget);
	ptrTarget = NULL;

	return;
}


/* Return a ptr record if the sharing pref name is a matching FQDN */
static CFStringRef
hostname_match_full(CFArrayRef hosts, CFIndex count, CFStringRef nameToMatch)
{
	CFIndex		i;
	CFStringRef	matchedHostName	= NULL;

	for (i = 0; i < count; i++) {
		CFStringRef tempHostName;

		tempHostName = CFArrayGetValueAtIndex(hosts, i);
		if (CFStringCompare(tempHostName, nameToMatch, kCFCompareCaseInsensitive) == 0) {
			matchedHostName = tempHostName;
			break;
		}
	}
	return matchedHostName;
}


/* Return a ptr record if the sharing pref name matches DNS record's first label */
static CFStringRef
hostname_match_first_label(CFArrayRef hosts, CFIndex count, CFStringRef nameToMatch)
{
	CFIndex		i;
	CFStringRef	matchedHostName	= NULL;

	for (i = 0; i < count; i++) {
		CFArrayRef	fqdnSeparated;
		CFStringRef	tempHostName;

		tempHostName = CFArrayGetValueAtIndex(hosts, i);
		fqdnSeparated = CFStringCreateArrayBySeparatingStrings(NULL, tempHostName, CFSTR("."));
		if (fqdnSeparated != NULL) {
			CFStringRef	firstLabel;
			Boolean		matchFound;

			firstLabel = CFArrayGetValueAtIndex(fqdnSeparated, 0);
			matchFound = (CFStringCompare(firstLabel, nameToMatch, kCFCompareCaseInsensitive) == 0);
			CFRelease(fqdnSeparated);
			if (matchFound) {
				matchedHostName = tempHostName;
				break;
			}
		}
	}
	return matchedHostName;
}


static void
ptr_query_callback(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void *info)
{
#pragma unused(info)
	CFStringRef		hostname	= NULL;
	struct timeval		ptrQueryComplete;
	struct timeval		ptrQueryElapsed;

	(void) gettimeofday(&ptrQueryComplete, NULL);
	timersub(&ptrQueryComplete, &ptrQueryStart, &ptrQueryElapsed);

	// use reverse DNS name, if available

	if (flags & kSCNetworkReachabilityFlagsReachable) {
		int		error_num;
		CFArrayRef	hosts;

		/*
		 * if [reverse] DNS query was successful
		 */
		hosts = SCNetworkReachabilityCopyResolvedAddress(target, &error_num);
		if (hosts != NULL) {
			CFIndex count = CFArrayGetCount(hosts);
			if (count > 0) {
				CFStringRef	computerName;
				CFStringRef	localHostName;

				my_log(LOG_INFO, "hostname: ptr query complete (query time = %ld.%3.3d)",
				       ptrQueryElapsed.tv_sec,
				       ptrQueryElapsed.tv_usec / 1000);

				// first, check if ComputerName is dns-clean
				computerName = _SCPreferencesCopyComputerName(NULL, NULL);
				if (computerName != NULL) {
					if (_SC_CFStringIsValidDNSName(computerName)) {
						CFRange	dotsCheck;

						dotsCheck = CFStringFind(computerName, CFSTR("."), 0);
						if (dotsCheck.length == 0) {
							hostname = hostname_match_first_label(hosts, count, computerName);
						} else {
							hostname = hostname_match_full(hosts, count, computerName);
						}
					}
					CFRelease(computerName);
				}

				// if no match, check LocalHostName against the first label of FQDN
				localHostName = (hostname == NULL) ? SCDynamicStoreCopyLocalHostName(store) : NULL;
				if (localHostName != NULL) {
					hostname = hostname_match_first_label(hosts, count, localHostName);
					CFRelease(localHostName);
				}

				// if no match, use the first of the returned names
				if (hostname == NULL) {
					hostname = CFArrayGetValueAtIndex(hosts, 0);
				}

				my_log(LOG_INFO, "hostname (reverse DNS query) = %@", hostname);
				set_hostname(hostname);
			} else {
				my_log(LOG_INFO, "hostname: ptr query complete w/no hosts (query time = %ld.%3.3d)",
				       ptrQueryElapsed.tv_sec,
				       ptrQueryElapsed.tv_usec / 1000);
			}
			CFRelease(hosts);

			if (hostname != NULL) {
				goto done;
			}
		} else {
			// if kSCNetworkReachabilityFlagsReachable and hosts == NULL
			// it means the PTR request has not come back yet
			// we must wait for this callback to be called again
			my_log(LOG_INFO, "hostname: ptr query reply w/no hosts (query time = %ld.%3.3d)",
			       ptrQueryElapsed.tv_sec,
			       ptrQueryElapsed.tv_usec / 1000);
			return;
		}
	} else {
		my_log(LOG_INFO, "hostname: ptr query complete, host not found (query time = %ld.%3.3d)",
		       ptrQueryElapsed.tv_sec,
		       ptrQueryElapsed.tv_usec / 1000);
	}

	// get local (multicast DNS) name, if available

	hostname = SCDynamicStoreCopyLocalHostName(store);
	if (hostname != NULL) {
		CFMutableStringRef	localHostName;

		my_log(LOG_INFO, "hostname (multicast DNS) = %@", hostname);
		localHostName = CFStringCreateMutableCopy(NULL, 0, hostname);
		assert(localHostName != NULL);
		CFStringAppend(localHostName, CFSTR(".local"));
		set_hostname(localHostName);
		CFRelease(localHostName);
		CFRelease(hostname);
		goto done;
	}

	// use "localhost" if not other name is available

	my_log(LOG_INFO, "hostname (localhost)");
	set_hostname(CFSTR("localhost"));

    done :

	ptr_query_stop();

#ifdef	MAIN
	CFRunLoopStop(rl);
#endif	// MAIN

	return;
}


static Boolean
ptr_query_start(CFStringRef address)
{
	union {
		struct sockaddr         sa;
		struct sockaddr_in      sin;
		struct sockaddr_in6     sin6;
	} addr;
	char				buf[64];
	CFDataRef			data;
	CFMutableDictionaryRef		options;

	if (_SC_cfstring_to_cstring(address, buf, sizeof(buf), kCFStringEncodingASCII) == NULL) {
		my_log(LOG_ERR, "could not convert [primary] address string");
		return FALSE;
	}

	if (_SC_string_to_sockaddr(buf, AF_UNSPEC, (void *)&addr, sizeof(addr)) == NULL) {
		my_log(LOG_ERR, "could not convert [primary] address");
		return FALSE;
	}

	options = CFDictionaryCreateMutable(NULL,
					    0,
					    &kCFTypeDictionaryKeyCallBacks,
					    &kCFTypeDictionaryValueCallBacks);
	data = CFDataCreate(NULL, (const UInt8 *)&addr.sa, addr.sa.sa_len);
	CFDictionarySetValue(options, kSCNetworkReachabilityOptionPTRAddress, data);
	CFRelease(data);
	ptrTarget = SCNetworkReachabilityCreateWithOptions(NULL, options);
	CFRelease(options);
	if (ptrTarget == NULL) {
		my_log(LOG_ERR, "could not resolve [primary] address");
		return FALSE;
	}

	my_log(LOG_INFO, "hostname: ptr query start");

	(void) gettimeofday(&ptrQueryStart, NULL);
	(void) SCNetworkReachabilitySetCallback(ptrTarget, ptr_query_callback, NULL);
	(void) SCNetworkReachabilityScheduleWithRunLoop(ptrTarget, rl, kCFRunLoopDefaultMode);

	return TRUE;
}


static void
update_hostname(SCDynamicStoreRef store, CFArrayRef changedKeys, void *info)
{
#pragma unused(changedKeys)
#pragma unused(info)
	CFStringRef	address		= NULL;
	CFStringRef	hostname	= NULL;
	CFStringRef	serviceID	= NULL;

	// if active, cancel any in-progress attempt to resolve the primary IP address

	if (ptrTarget != NULL) {
		ptr_query_stop();
	}

	// get [prefs] hostname, if available

	hostname = copy_prefs_hostname(store);
	if (hostname != NULL) {
		my_log(LOG_INFO, "hostname (prefs) = %@", hostname);
		set_hostname(hostname);
		goto done;
	}

	// get primary service ID

	serviceID = copy_primary_service(store);
	if (serviceID == NULL) {
		goto mDNS;
	}

	// get DHCP provided name, if available

	hostname = copy_dhcp_hostname(serviceID);
	if (hostname != NULL) {
		my_log(LOG_INFO, "hostname (DHCP) = %@", hostname);
		set_hostname(hostname);
		goto done;
	}

	// get DNS name associated with primary IP, if available

	address = copy_primary_ip(store, serviceID);
	if (address != NULL) {
		boolean_t	isExpensive;

		// start reverse DNS query using primary IP address
		// if primary service is not expensive
		isExpensive = check_if_service_expensive(serviceID);
		if (!isExpensive) {
			Boolean	ok;

			ok = ptr_query_start(address);
			if (ok) {
				// if query started
				goto done;
			}
		}
	}

    mDNS :

	// get local (multicast DNS) name, if available

	hostname = SCDynamicStoreCopyLocalHostName(store);
	if (hostname != NULL) {
		CFMutableStringRef	localHostName;

		my_log(LOG_INFO, "hostname (multicast DNS) = %@", hostname);
		localHostName = CFStringCreateMutableCopy(NULL, 0, hostname);
		assert(localHostName != NULL);
		CFStringAppend(localHostName, CFSTR(".local"));
		set_hostname(localHostName);
		CFRelease(localHostName);
		goto done;
	}

	// use "localhost" if not other name is available

	set_hostname(CFSTR("localhost"));

    done :

	if (address)	CFRelease(address);
	if (hostname)	CFRelease(hostname);
	if (serviceID)	CFRelease(serviceID);

	return;
}


__private_extern__
void
load_hostname(Boolean verbose)
{
#pragma unused(verbose)
	CFStringRef		key;
	CFMutableArrayRef	keys		= NULL;
	dispatch_block_t	notify_block;
	Boolean			ok;
	CFMutableArrayRef	patterns	= NULL;
	uint32_t		status;

	/* initialize a few globals */

	store = SCDynamicStoreCreate(NULL, CFSTR("set-hostname"), update_hostname, NULL);
	if (store == NULL) {
		my_log(LOG_ERR,
		       "SCDynamicStoreCreate() failed: %s",
		       SCErrorString(SCError()));
		goto error;
	}

	queue = dispatch_queue_create(SET_HOSTNAME_QUEUE, NULL);
	if (queue == NULL) {
		my_log(LOG_ERR,
		       "dispatch_queue_create() failed");
		goto error;
	}

	/* establish notification keys and patterns */

	keys     = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	patterns = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	/* ...watch for (per-service) DHCP option changes */
	key = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
							  kSCDynamicStoreDomainState,
							  kSCCompAnyRegex,
							  kSCEntNetDHCP);
	CFArrayAppendValue(patterns, key);
	CFRelease(key);

	/* ...watch for (BSD) hostname changes */
	key = SCDynamicStoreKeyCreateComputerName(NULL);
	CFArrayAppendValue(keys, key);
	CFRelease(key);

	/* ...watch for local (multicast DNS) hostname changes */
	key = SCDynamicStoreKeyCreateHostNames(NULL);
	CFArrayAppendValue(keys, key);
	CFRelease(key);

	/* register the keys/patterns */
	ok = SCDynamicStoreSetNotificationKeys(store, keys, patterns);
	CFRelease(keys);
	CFRelease(patterns);
	if (!ok) {
		my_log(LOG_ERR,
		       "SCDynamicStoreSetNotificationKeys() failed: %s",
		       SCErrorString(SCError()));
		goto error;
	}

	rl = CFRunLoopGetCurrent();
	rls = SCDynamicStoreCreateRunLoopSource(NULL, store, 0);
	if (rls == NULL) {
		my_log(LOG_ERR,
		       "SCDynamicStoreCreateRunLoopSource() failed: %s",
		       SCErrorString(SCError()));
		goto error;
	}
	CFRunLoopAddSource(rl, rls, kCFRunLoopDefaultMode);

	/* ...watch for primary service/interface and DNS configuration changes */
	notify_block = ^{
		CFArrayRef      changes;
		CFStringRef     key;

		key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
								 kSCDynamicStoreDomainState,
								 kSCEntNetDNS);
		changes = CFArrayCreate(NULL, (const void **)&key, 1, &kCFTypeArrayCallBacks);
		(*update_hostname)(store, changes, NULL);
		CFRelease(changes);
		CFRelease(key);

		return;
	};
	status = notify_register_dispatch(_SC_NOTIFY_NETWORK_CHANGE,
					  &notify_token,
					  queue,
					  ^(int token){
#pragma unused(token)
						  CFRunLoopPerformBlock(rl,
									kCFRunLoopDefaultMode,
									notify_block);
						  CFRunLoopWakeUp(rl);
					  });
	if (status != NOTIFY_STATUS_OK) {
		my_log(LOG_ERR, "notify_register_dispatch() failed: %u", status);
		goto error;
	}

	return;

    error :

	if (rls != NULL) {
		CFRunLoopRemoveSource(rl, rls, kCFRunLoopDefaultMode);
		CFRelease(rls);
		rls = NULL;
	}
	if (store != NULL) {
		CFRelease(store);
		store = NULL;
	}
	if (queue != NULL) {
		dispatch_release(queue);
		queue = NULL;
	}

	return;
}


#ifdef	MAIN
int
main(int argc, char **argv)
{

#ifdef	DEBUG

	_sc_log = FALSE;
	if ((argc > 1) && (strcmp(argv[1], "-d") == 0)) {
		_sc_verbose = TRUE;
		argv++;
		argc--;
	}

	CFStringRef		address;
	CFStringRef		hostname;
	CFStringRef		serviceID;
	SCDynamicStoreRef	store;

	store = SCDynamicStoreCreate(NULL, CFSTR("set-hostname"), NULL, NULL);
	if (store == NULL) {
		SCPrint(TRUE, stdout,
			CFSTR("SCDynamicStoreCreate() failed: %s\n"),
			SCErrorString(SCError()));
		exit(1);
	}

	// get [prefs] hostname, if available
	hostname = copy_prefs_hostname(store);
	if (hostname != NULL) {
		SCPrint(TRUE, stdout, CFSTR("hostname (prefs) = %@\n"), hostname);
		CFRelease(hostname);
	}

	// get local (multicast DNS) name, if available

	hostname = SCDynamicStoreCopyLocalHostName(store);
	if (hostname != NULL) {
		SCPrint(TRUE, stdout, CFSTR("hostname (multicast DNS) = %@\n"), hostname);
		CFRelease(hostname);
	}

	// get primary service
	serviceID = copy_primary_service(store);
	if (serviceID != NULL) {
		SCPrint(TRUE, stdout, CFSTR("primary service ID = %@\n"), serviceID);
	} else {
		SCPrint(TRUE, stdout, CFSTR("No primary service\n"));
	}

	if ((argc == (2+1)) && (argv[1][0] == 's')) {
		if (serviceID != NULL)	CFRelease(serviceID);
		serviceID = CFStringCreateWithCString(NULL, argv[2], kCFStringEncodingUTF8);
		SCPrint(TRUE, stdout, CFSTR("alternate service ID = %@\n"), serviceID);
	}

	if (serviceID != NULL) {
		// get DHCP provided name
		hostname = copy_dhcp_hostname(serviceID);
		if (hostname != NULL) {
			SCPrint(TRUE, stdout, CFSTR("hostname (DHCP) = %@\n"), hostname);
			CFRelease(hostname);
		}

		// get primary IP address
		address = copy_primary_ip(store, serviceID);
		if (address != NULL) {
			SCPrint(TRUE, stdout, CFSTR("primary address = %@\n"), address);

			if ((argc == (2+1)) && (argv[1][0] == 'a')) {
				if (address != NULL)	CFRelease(address);
				address = CFStringCreateWithCString(NULL, argv[2], kCFStringEncodingUTF8);
				SCPrint(TRUE, stdout, CFSTR("alternate primary address = %@\n"), address);
			}

			// start reverse DNS query using primary IP address
			(void) ptr_query_start(address);
			CFRelease(address);
		}

		CFRelease(serviceID);
	}

	CFRelease(store);

	CFRunLoopRun();

#else	/* DEBUG */

	_sc_log     = FALSE;
	_sc_verbose = (argc > 1) ? TRUE : FALSE;

	load_hostname((argc > 1) ? TRUE : FALSE);
	CFRunLoopRun();
	/* not reached */

#endif	/* DEBUG */

	exit(0);
	return 0;
}
#endif	/* MAIN */

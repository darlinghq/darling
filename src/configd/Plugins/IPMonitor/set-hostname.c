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
#define my_log(__level, fmt, ...)	SCPrint(TRUE, stdout, CFSTR(fmt "\n"), ## __VA_ARGS__)
#else	// MAIN
#include "ip_plugin.h"
#endif	// MAIN

static SCDynamicStoreRef	store		= NULL;
static CFRunLoopSourceRef	rls		= NULL;

static struct timeval		ptrQueryStart;
static SCNetworkReachabilityRef	ptrTarget	= NULL;

static Boolean			_verbose	= FALSE;


#define	HOSTNAME_NOTIFY_KEY	"com.apple.system.hostname"

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
			my_log(LOG_ERR, "could not convert [new] hostname");
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

	SCNetworkReachabilitySetCallback(ptrTarget, NULL, NULL);
	SCNetworkReachabilityUnscheduleFromRunLoop(ptrTarget, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
	CFRelease(ptrTarget);
	ptrTarget = NULL;

	return;
}


static void
ptr_query_callback(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void *info)
{
	CFStringRef		hostname	= NULL;
	struct timeval		ptrQueryComplete;
	struct timeval		ptrQueryElapsed;

	(void) gettimeofday(&ptrQueryComplete, NULL);
	timersub(&ptrQueryComplete, &ptrQueryStart, &ptrQueryElapsed);
	if (_verbose) {
		my_log(LOG_DEBUG, "ptr query complete%s (query time = %ld.%3.3d)",
		       (flags & kSCNetworkReachabilityFlagsReachable) ? "" : ", host not found",
		       ptrQueryElapsed.tv_sec,
		       ptrQueryElapsed.tv_usec / 1000);
	}

	// use reverse DNS name, if available

	if (flags & kSCNetworkReachabilityFlagsReachable) {
		int		error_num;
		CFArrayRef	hosts;

		/*
		 * if [reverse] DNS query was successful
		 */
		hosts = SCNetworkReachabilityCopyResolvedAddress(target, &error_num);
		if (hosts != NULL) {
			if (CFArrayGetCount(hosts) > 0) {

				hostname = CFArrayGetValueAtIndex(hosts, 0);
				my_log(LOG_DEBUG, "hostname (reverse DNS query) = %@", hostname);
				set_hostname(hostname);
			}
			CFRelease(hosts);

			if (hostname != NULL) {
				goto done;
			}
		}
	}

	// get local (multicast DNS) name, if available

	hostname = SCDynamicStoreCopyLocalHostName(store);
	if (hostname != NULL) {
		CFMutableStringRef	localName;

		my_log(LOG_DEBUG, "hostname (multicast DNS) = %@", hostname);
		localName = CFStringCreateMutableCopy(NULL, 0, hostname);
		assert(localName != NULL);
		CFStringAppend(localName, CFSTR(".local"));
		set_hostname(localName);
		CFRelease(localName);
		CFRelease(hostname);
		goto done;
	}

	// use "localhost" if not other name is available

	set_hostname(CFSTR("localhost"));

    done :

	ptr_query_stop();

#ifdef	MAIN
	CFRunLoopStop(CFRunLoopGetCurrent());
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

	(void) SCNetworkReachabilitySetCallback(ptrTarget, ptr_query_callback, NULL);
	(void) SCNetworkReachabilityScheduleWithRunLoop(ptrTarget, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

	return TRUE;
}


static void
update_hostname(SCDynamicStoreRef store, CFArrayRef changedKeys, void *info)
{
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
		my_log(LOG_DEBUG, "hostname (prefs) = %@", hostname);
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
		my_log(LOG_DEBUG, "hostname (DHCP) = %@", hostname);
		set_hostname(hostname);
		goto done;
	}

	// get DNS name associated with primary IP, if available

	address = copy_primary_ip(store, serviceID);
	if (address != NULL) {
		Boolean	ok;

		// start reverse DNS query using primary IP address
		ok = ptr_query_start(address);
		if (ok) {
			// if query started
			goto done;
		}
	}

    mDNS :

	// get local (multicast DNS) name, if available

	hostname = SCDynamicStoreCopyLocalHostName(store);
	if (hostname != NULL) {
		CFMutableStringRef	localName;

		my_log(LOG_DEBUG, "hostname (multicast DNS) = %@", hostname);
		localName = CFStringCreateMutableCopy(NULL, 0, hostname);
		assert(localName != NULL);
		CFStringAppend(localName, CFSTR(".local"));
		set_hostname(localName);
		CFRelease(localName);
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
	CFStringRef		key;
	CFMutableArrayRef	keys		= NULL;
	CFMutableArrayRef	patterns	= NULL;

	if (verbose) {
		_verbose = TRUE;
	}

	/* initialize a few globals */

	store = SCDynamicStoreCreate(NULL, CFSTR("set-hostname"), update_hostname, NULL);
	if (store == NULL) {
		my_log(LOG_ERR,
		       "SCDynamicStoreCreate() failed: %s",
		       SCErrorString(SCError()));
		goto error;
	}

	/* establish notification keys and patterns */

	keys     = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	patterns = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	/* ...watch for primary service / interface changes */
	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							 kSCDynamicStoreDomainState,
							 kSCEntNetIPv4);
	CFArrayAppendValue(keys, key);
	CFRelease(key);

	/* ...watch for DNS configuration changes */
	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL,
							 kSCDynamicStoreDomainState,
							 kSCEntNetDNS);
	CFArrayAppendValue(keys, key);
	CFRelease(key);

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
	if (!SCDynamicStoreSetNotificationKeys(store, keys, patterns)) {
		my_log(LOG_ERR,
		       "SCDynamicStoreSetNotificationKeys() failed: %s",
		       SCErrorString(SCError()));
		goto error;
	}

	rls = SCDynamicStoreCreateRunLoopSource(NULL, store, 0);
	if (!rls) {
		my_log(LOG_ERR,
		       "SCDynamicStoreCreateRunLoopSource() failed: %s",
		       SCErrorString(SCError()));
		goto error;
	}
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);

	CFRelease(keys);
	CFRelease(patterns);
	return;

    error :

	if (keys != NULL)	CFRelease(keys);
	if (patterns != NULL)	CFRelease(patterns);
	if (store != NULL)	CFRelease(store);
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
		_verbose = TRUE;
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

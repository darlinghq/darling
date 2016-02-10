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
 * October 21, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <net/if.h>
#include <net/if_media.h>

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include <SystemConfiguration/SCValidation.h>
#include <SystemConfiguration/SCDPlugin.h>		// for _SCDPluginExecCommand

#include "SCNetworkConfigurationInternal.h"


static CFMutableDictionaryRef	baseSettings	= NULL;
static CFStringRef		interfacesKey	= NULL;
static SCDynamicStoreRef	store		= NULL;
static CFRunLoopSourceRef	rls		= NULL;
static CFMutableDictionaryRef	wantSettings	= NULL;

static Boolean			_verbose	= FALSE;


#pragma mark -
#pragma mark Capabilities


#define	CAPABILITIES_KEY	CFSTR("_CAPABILITIES_")


__private_extern__
Boolean
_SCNetworkInterfaceSetCapabilities(SCNetworkInterfaceRef	interface,
				   CFDictionaryRef		options)
{
	CFDictionaryRef	baseOptions;
	int		cap_base;
	int		cap_current;
	int		cap_requested;
	CFStringRef	interfaceName;

#ifdef	SIOCSIFCAP
	struct ifreq	ifr;
	int		ret;
	int		sock;
#endif	// SIOCSIFCAP

	interfaceName = SCNetworkInterfaceGetBSDName(interface);
	if (interfaceName == NULL) {
		/* if no BSD interface name */
		return FALSE;
	}

	cap_current = __SCNetworkInterfaceCreateCapabilities(interface, -1, NULL);
	if (cap_current == -1) {
		/* could not get current capabilities */
		return FALSE;
	}

	// get base capabilities
	cap_base = cap_current;
	baseOptions = CFDictionaryGetValue(baseSettings, interfaceName);
	if (baseOptions != NULL) {
		CFNumberRef	num;

		num = CFDictionaryGetValue(baseOptions, CAPABILITIES_KEY);
		if (num != NULL) {
			CFNumberGetValue(num, kCFNumberIntType, &cap_base);
		}
	}

	cap_requested = __SCNetworkInterfaceCreateCapabilities(interface, cap_base, options);

#ifdef	SIOCSIFCAP
	if (cap_requested == cap_current) {
		/* if current setting is as requested */
		return TRUE;
	}

	bzero((char *)&ifr, sizeof(ifr));
	(void)_SC_cfstring_to_cstring(interfaceName, ifr.ifr_name, sizeof(ifr.ifr_name), kCFStringEncodingASCII);
	ifr.ifr_curcap = cap_current;
	ifr.ifr_reqcap = cap_requested;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		SCLog(TRUE, LOG_ERR, CFSTR("socket() failed: %s"), strerror(errno));
		return FALSE;
	}

	ret = ioctl(sock, SIOCSIFCAP, (caddr_t)&ifr);
	(void)close(sock);
	if (ret == -1) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("ioctl(SIOCSIFCAP) failed: %s"), strerror(errno));
		return FALSE;
	}
#endif	// SIOCSIFCAP

	return TRUE;
}


#pragma mark -
#pragma mark Media options


static CFDictionaryRef
__copyMediaOptions(CFDictionaryRef options)
{
	CFMutableDictionaryRef	requested	= NULL;
	CFTypeRef		val;

	if (!isA_CFDictionary(options)) {
		return NULL;
	}

	val = CFDictionaryGetValue(options, kSCPropNetEthernetMediaSubType);
	if (isA_CFString(val)) {
		requested = CFDictionaryCreateMutable(NULL,
						      0,
						      &kCFTypeDictionaryKeyCallBacks,
						      &kCFTypeDictionaryValueCallBacks);
		CFDictionaryAddValue(requested, kSCPropNetEthernetMediaSubType, val);
	} else {
		/* if garbage */;
		return NULL;
	}

	val = CFDictionaryGetValue(options, kSCPropNetEthernetMediaOptions);
	if (isA_CFArray(val)) {
		CFDictionaryAddValue(requested, kSCPropNetEthernetMediaOptions, val);
	} else {
		/* if garbage */;
		CFRelease(requested);
		return NULL;
	}

	return requested;
}


__private_extern__
Boolean
_SCNetworkInterfaceSetMediaOptions(SCNetworkInterfaceRef	interface,
				   CFDictionaryRef		options)
{
	CFArrayRef		available	= NULL;
	CFDictionaryRef		current		= NULL;
	struct ifmediareq	ifm;
	struct ifreq		ifr;
	CFStringRef		interfaceName;
	Boolean			ok		= FALSE;
	int			newOptions;
	CFDictionaryRef		requested;
	int			sock		= -1;

	if (!isA_SCNetworkInterface(interface)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	interfaceName = SCNetworkInterfaceGetBSDName(interface);
	if (interfaceName == NULL) {
		/* if no BSD interface name */
		SCLog(_verbose, LOG_INFO, CFSTR("no BSD interface name for %@"), interface);
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	/* get current & available options */
	if (!SCNetworkInterfaceCopyMediaOptions(interface, &current, NULL, &available, FALSE)) {
		/* could not get current media options */
		SCLog(_verbose, LOG_INFO, CFSTR("no media options for %@"), interfaceName);
		return FALSE;
	}

	/* extract just the dictionary key/value pairs of interest */
	requested = __copyMediaOptions(options);
	if (requested == NULL) {
		CFDictionaryRef	baseOptions;

		/* get base options */
		baseOptions = CFDictionaryGetValue(baseSettings, interfaceName);
		requested = __copyMediaOptions(baseOptions);
	}
	if (requested == NULL) {
		/* get base options */
		requested = __copyMediaOptions(current);
	}
	if (requested == NULL) {
		/* if no media options to set */
		goto done;
	}

	if ((current != NULL) && CFEqual(current, requested)) {
		/* if current settings are as requested */
		ok = TRUE;
		goto done;
	}

	if (!CFArrayContainsValue(available, CFRangeMake(0, CFArrayGetCount(available)), requested)) {
		/* if requested settings not currently available */
		SCLog(_verbose, LOG_INFO, CFSTR("requested media settings unavailable for %@"), interfaceName);
		goto done;
	}

	newOptions = __SCNetworkInterfaceCreateMediaOptions(interface, requested);
	if (newOptions == -1) {
		/* since we have just validated, this should never happen */
		goto done;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		SCLog(TRUE, LOG_ERR, CFSTR("socket() failed: %s"), strerror(errno));
		goto done;
	}

	bzero((char *)&ifm, sizeof(ifm));
	(void)_SC_cfstring_to_cstring(interfaceName, ifm.ifm_name, sizeof(ifm.ifm_name), kCFStringEncodingASCII);

	if (ioctl(sock, SIOCGIFMEDIA, (caddr_t)&ifm) == -1) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("ioctl(SIOCGIFMEDIA) failed: %s"), strerror(errno));
		goto done;
	}

	bzero((char *)&ifr, sizeof(ifr));
	bcopy(ifm.ifm_name, ifr.ifr_name, sizeof(ifr.ifr_name));
	ifr.ifr_media =  ifm.ifm_current & ~(IFM_NMASK|IFM_TMASK|IFM_OMASK|IFM_GMASK);
	ifr.ifr_media |= newOptions;

	SCLog(_verbose, LOG_INFO, CFSTR("old media settings: 0x%8.8x (0x%8.8x)"), ifm.ifm_current, ifm.ifm_active);
	SCLog(_verbose, LOG_INFO, CFSTR("new media settings: 0x%8.8x"), ifr.ifr_media);

	if (ioctl(sock, SIOCSIFMEDIA, (caddr_t)&ifr) == -1) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("%@: ioctl(SIOCSIFMEDIA) failed: %s"), interfaceName, strerror(errno));
		goto done;
	}

	ok = TRUE;

    done :

	if (available != NULL)	CFRelease(available);
	if (current != NULL)	CFRelease(current);
	if (requested != NULL)	CFRelease(requested);
	if (sock != -1)	(void)close(sock);

	return ok;
}


#pragma mark -
#pragma mark MTU


#ifndef	USE_SIOCSIFMTU
static void
ifconfig_exit(pid_t pid, int status, struct rusage *rusage, void *context)
{
	char	*if_name	= (char *)context;

	if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) != 0) {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("ifconfig %s failed, exit status = %d"),
			      if_name,
			      WEXITSTATUS(status));
		}
	} else if (WIFSIGNALED(status)) {
		SCLog(TRUE, LOG_DEBUG,
		      CFSTR("ifconfig %s: terminated w/signal = %d"),
		      if_name,
		      WTERMSIG(status));
	} else {
		SCLog(TRUE, LOG_DEBUG,
		      CFSTR("ifconfig %s: exit status = %d"),
		      if_name,
		      status);
	}

	CFAllocatorDeallocate(NULL, if_name);
	return;
}
#endif	/* !USE_SIOCSIFMTU */


__private_extern__
Boolean
_SCNetworkInterfaceSetMTU(SCNetworkInterfaceRef	interface,
			  CFDictionaryRef	options)
{
	CFStringRef	interfaceName;
	int		mtu_cur		= -1;
	int		mtu_max		= -1;
	int		mtu_min		= -1;
	int		requested;
	CFNumberRef	val;

	interfaceName = SCNetworkInterfaceGetBSDName(interface);
	if (interfaceName == NULL) {
		/* if no BSD interface name */
		return FALSE;
	}

	if (!SCNetworkInterfaceCopyMTU(interface, &mtu_cur, &mtu_min, &mtu_max)) {
		/* could not get current MTU */
		return FALSE;
	}

	val = NULL;
	if (isA_CFDictionary(options)) {
		val = CFDictionaryGetValue(options, kSCPropNetEthernetMTU);
		val = isA_CFNumber(val);
	}
	if (val == NULL) {
		CFDictionaryRef	baseOptions;

		/* get base MTU */
		baseOptions = CFDictionaryGetValue(baseSettings, interfaceName);
		if (baseOptions != NULL) {
			val = CFDictionaryGetValue(baseOptions, kSCPropNetEthernetMTU);
		}
	}
	if (val != NULL) {
		CFNumberGetValue(val, kCFNumberIntType, &requested);
	} else {
		requested = mtu_cur;
	}

	if (requested == mtu_cur) {
		/* if current setting is as requested */
		return TRUE;
	}

	if (((mtu_min >= 0) && (requested < mtu_min)) ||
	    ((mtu_max >= 0) && (requested > mtu_max))) {
		/* if requested MTU outside of the valid range */
		return FALSE;
	}

#ifdef	USE_SIOCSIFMTU
{
	struct ifreq	ifr;
	int		ret;
	int		sock;

	bzero((char *)&ifr, sizeof(ifr));
	(void)_SC_cfstring_to_cstring(interfaceName, ifr.ifr_name, sizeof(ifr.ifr_name), kCFStringEncodingASCII);
	ifr.ifr_mtu = requested;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		SCLog(TRUE, LOG_ERR, CFSTR("socket() failed: %s"), strerror(errno));
		return FALSE;
	}

	ret = ioctl(sock, SIOCSIFMTU, (caddr_t)&ifr);
	(void)close(sock);
	if (ret == -1) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("ioctl(SIOCSIFMTU) failed: %s"), strerror(errno));
		return FALSE;
	}
}
#else	/* !USE_SIOCSIFMTU */
{
	char	*ifconfig_argv[] = { "ifconfig", NULL, "mtu", NULL, NULL };
	pid_t	pid;

	ifconfig_argv[1] = _SC_cfstring_to_cstring(interfaceName, NULL, 0, kCFStringEncodingASCII);
	(void)asprintf(&ifconfig_argv[3], "%d", requested);

	pid = _SCDPluginExecCommand(ifconfig_exit,	// callout,
				    ifconfig_argv[1],	// context
				    0,			// uid
				    0,			// gid
				    "/sbin/ifconfig",	// path
				    ifconfig_argv	// argv
				   );

//	CFAllocatorDeallocate(NULL, ifconfig_argv[1]);	// released in ifconfig_exit()
	free(ifconfig_argv[3]);

	if (pid <= 0) {
		return FALSE;
	}
}
#endif	/* !USE_SIOCSIFMTU */

	return TRUE;
}


#pragma mark -
#pragma mark Update link configuration


/*
 * Function: parse_component
 * Purpose:
 *   Given a string 'key' and a string prefix 'prefix',
 *   return the next component in the slash '/' separated
 *   key.
 *
 * Examples:
 * 1. key = "a/b/c" prefix = "a/"
 *    returns "b"
 * 2. key = "a/b/c" prefix = "a/b/"
 *    returns "c"
 */
static CF_RETURNS_RETAINED CFStringRef
parse_component(CFStringRef key, CFStringRef prefix)
{
	CFMutableStringRef	comp;
	CFRange			range;

	if (CFStringHasPrefix(key, prefix) == FALSE) {
		return NULL;
	}
	comp = CFStringCreateMutableCopy(NULL, 0, key);
	CFStringDelete(comp, CFRangeMake(0, CFStringGetLength(prefix)));
	range = CFStringFind(comp, CFSTR("/"), 0);
	if (range.location == kCFNotFound) {
		return comp;
	}
	range.length = CFStringGetLength(comp) - range.location;
	CFStringDelete(comp, range);
	return comp;
}


static void updateLink(CFStringRef interfaceName, CFDictionaryRef options);


static void
updateInterfaces(CFArrayRef newInterfaces)
{
	CFIndex			i;
	CFIndex			n_old;
	CFIndex			n_new;
	static CFArrayRef	oldInterfaces	= NULL;

	n_old = (oldInterfaces != NULL) ? CFArrayGetCount(oldInterfaces) : 0;
	n_new = CFArrayGetCount(newInterfaces);

	for (i = 0; i < n_new; i++) {
		CFStringRef	interfaceName;

		interfaceName = CFArrayGetValueAtIndex(newInterfaces, i);

		if ((n_old == 0) ||
		    !CFArrayContainsValue(oldInterfaces,
					  CFRangeMake(0, n_old),
					  interfaceName)) {
			CFDictionaryRef	options;

			// if new interface
			options = CFDictionaryGetValue(wantSettings, interfaceName);
			updateLink(interfaceName, options);
		}
	}

	if (oldInterfaces != NULL) CFRelease(oldInterfaces);
	oldInterfaces = CFRetain(newInterfaces);
}


static void
updateLink(CFStringRef interfaceName, CFDictionaryRef options)
{
	SCNetworkInterfaceRef	interface;

	/* retain requested configuration */
	if (options != NULL) {
		CFDictionarySetValue(wantSettings, interfaceName, options);
	} else {
		CFDictionaryRemoveValue(wantSettings, interfaceName);
	}

	/* apply requested configuration */
	interface = _SCNetworkInterfaceCreateWithBSDName(NULL, interfaceName,
							 kIncludeAllVirtualInterfaces);
	if (interface == NULL) {
		return;
	}

	if (options != NULL) {
		if (!CFDictionaryContainsKey(baseSettings, interfaceName)) {
			int			cur_cap		= -1;
			CFDictionaryRef		cur_media	= NULL;
			CFMutableDictionaryRef	new_media	= NULL;
			int			cur_mtu		= -1;

			/* preserve current media options */
			if (SCNetworkInterfaceCopyMediaOptions(interface, &cur_media, NULL, NULL, FALSE)) {
				if (cur_media != NULL) {
					new_media = CFDictionaryCreateMutableCopy(NULL, 0, cur_media);
					CFRelease(cur_media);
				}
			}

			/* preserve current MTU */
			if (SCNetworkInterfaceCopyMTU(interface, &cur_mtu, NULL, NULL)) {
				if (cur_mtu != -1) {
					CFNumberRef	num;

					if (new_media == NULL) {
						new_media = CFDictionaryCreateMutable(NULL,
										      0,
										      &kCFTypeDictionaryKeyCallBacks,
										      &kCFTypeDictionaryValueCallBacks);
					}

					num = CFNumberCreate(NULL, kCFNumberIntType, &cur_mtu);
					CFDictionaryAddValue(new_media, kSCPropNetEthernetMTU, num);
					CFRelease(num);
				}
			}

			/* preserve capabilities */
			cur_cap = __SCNetworkInterfaceCreateCapabilities(interface, -1, NULL);
			if (cur_cap != -1) {
				CFNumberRef	num;

				if (new_media == NULL) {
					new_media = CFDictionaryCreateMutable(NULL,
									      0,
									      &kCFTypeDictionaryKeyCallBacks,
									      &kCFTypeDictionaryValueCallBacks);
				}

				num = CFNumberCreate(NULL, kCFNumberIntType, &cur_cap);
				CFDictionaryAddValue(new_media, CAPABILITIES_KEY, num);
				CFRelease(num);
			}

			if (new_media != NULL) {
				CFDictionarySetValue(baseSettings, interfaceName, new_media);
				CFRelease(new_media);
			}
		}

		/* establish new settings */
		(void)_SCNetworkInterfaceSetCapabilities(interface, options);
		(void)_SCNetworkInterfaceSetMediaOptions(interface, options);
		(void)_SCNetworkInterfaceSetMTU         (interface, options);
	} else {
		/* no requested settings */
		options = CFDictionaryGetValue(baseSettings, interfaceName);
		if (options != NULL) {
			/* restore original settings */
			(void)_SCNetworkInterfaceSetCapabilities(interface, options);
			(void)_SCNetworkInterfaceSetMediaOptions(interface, options);
			(void)_SCNetworkInterfaceSetMTU         (interface, options);
			CFDictionaryRemoveValue(baseSettings, interfaceName);
		}
	}

	CFRelease(interface);
	return;
}


static void
linkConfigChangedCallback(SCDynamicStoreRef store, CFArrayRef changedKeys, void *arg)
{
	CFDictionaryRef		changes;
	CFIndex			i;
	CFIndex			n;
	static CFStringRef	prefix		= NULL;

	if (prefix == NULL) {
		prefix = SCDynamicStoreKeyCreate(NULL,
						 CFSTR("%@/%@/%@/"),
						 kSCDynamicStoreDomainSetup,
						 kSCCompNetwork,
						 kSCCompInterface);
	}

	changes = SCDynamicStoreCopyMultiple(store, changedKeys, NULL);

	n = CFArrayGetCount(changedKeys);
	for (i = 0; i < n; i++) {
		CFStringRef	key;
		CFDictionaryRef	info;

		key  = CFArrayGetValueAtIndex(changedKeys, i);
		info = CFDictionaryGetValue(changes, key);

		if (CFEqual(key, interfacesKey)) {
			CFArrayRef	interfaces;

			interfaces = CFDictionaryGetValue(info, kSCPropNetInterfaces);
			if (isA_CFArray(interfaces)) {
				updateInterfaces(interfaces);
			}
		} else {
			CFStringRef	interfaceName;

			interfaceName = parse_component(key, prefix);
			if (interfaceName != NULL) {
				updateLink(interfaceName, info);
				CFRelease(interfaceName);
			}
		}
	}

	CFRelease(changes);

	return;
}


__private_extern__
void
load_LinkConfiguration(CFBundleRef bundle, Boolean bundleVerbose)
{
	CFStringRef		key;
	CFMutableArrayRef	keys		= NULL;
	Boolean			ok;
	CFMutableArrayRef	patterns	= NULL;

	if (bundleVerbose) {
		_verbose = TRUE;
	}

	SCLog(_verbose, LOG_DEBUG, CFSTR("load() called"));
	SCLog(_verbose, LOG_DEBUG, CFSTR("  bundle ID = %@"), CFBundleGetIdentifier(bundle));

	/* initialize a few globals */

	baseSettings = CFDictionaryCreateMutable(NULL,
						 0,
						 &kCFTypeDictionaryKeyCallBacks,
						 &kCFTypeDictionaryValueCallBacks);
	wantSettings = CFDictionaryCreateMutable(NULL,
						 0,
						 &kCFTypeDictionaryKeyCallBacks,
						 &kCFTypeDictionaryValueCallBacks);

	/* open a "configd" store to allow cache updates */
	store = SCDynamicStoreCreate(NULL,
				     CFSTR("Link Configuraton plug-in"),
				     linkConfigChangedCallback,
				     NULL);
	if (store == NULL) {
		SCLog(TRUE, LOG_ERR, CFSTR("SCDynamicStoreCreate() failed: %s"), SCErrorString(SCError()));
		goto error;
	}

	/* establish notification keys and patterns */
	keys     = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	patterns = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	/* ...watch for a change in the list of network interfaces */
	interfacesKey = SCDynamicStoreKeyCreateNetworkInterface(NULL,
								kSCDynamicStoreDomainState);
	CFArrayAppendValue(keys, interfacesKey);

	/* ...watch for (per-interface) AirPort configuration changes */
	key = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
							    kSCDynamicStoreDomainSetup,
							    kSCCompAnyRegex,
							    kSCEntNetAirPort);
	CFArrayAppendValue(patterns, key);
	CFRelease(key);

	/* ...watch for (per-interface) Ethernet configuration changes */
	key = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
							    kSCDynamicStoreDomainSetup,
							    kSCCompAnyRegex,
							    kSCEntNetEthernet);
	CFArrayAppendValue(patterns, key);
	CFRelease(key);

	/* ...watch for (per-interface) FireWire configuration changes */
	key = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
							    kSCDynamicStoreDomainSetup,
							    kSCCompAnyRegex,
							    kSCEntNetFireWire);
	CFArrayAppendValue(patterns, key);
	CFRelease(key);

	/* register the keys/patterns */
	ok = SCDynamicStoreSetNotificationKeys(store, keys, patterns);
	CFRelease(keys);
	CFRelease(patterns);
	if (!ok) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("SCDynamicStoreSetNotificationKeys() failed: %s"),
		      SCErrorString(SCError()));
		goto error;
	}

	rls = SCDynamicStoreCreateRunLoopSource(NULL, store, 0);
	if (rls == NULL) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("SCDynamicStoreCreateRunLoopSource() failed: %s"),
		      SCErrorString(SCError()));
		goto error;
	}

	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	return;

    error :

	if (baseSettings != NULL)	CFRelease(baseSettings);
	if (wantSettings != NULL)	CFRelease(wantSettings);
	if (store != NULL) 		CFRelease(store);
	return;
}


#ifdef	MAIN


#pragma mark -
#pragma mark Standalone test code


int
main(int argc, char **argv)
{
	SCPreferencesRef	prefs;

	_sc_log     = FALSE;
	_sc_verbose = (argc > 1) ? TRUE : FALSE;

	prefs = SCPreferencesCreate(NULL, CFSTR("linkconfig"), NULL);
	if (prefs != NULL) {
		SCNetworkSetRef	set;

		set = SCNetworkSetCopyCurrent(prefs);
		if (set != NULL) {
			CFMutableSetRef	seen;
			CFArrayRef	services;

			services = SCNetworkSetCopyServices(set);
			if (services != NULL) {
				CFIndex		i;
				CFIndex		n;

				seen = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);

				n = CFArrayGetCount(services);
				for (i = 0; i < n; i++) {
					SCNetworkInterfaceRef	interface;
					SCNetworkServiceRef	service;

					service = CFArrayGetValueAtIndex(services, i);
					interface = SCNetworkServiceGetInterface(service);
					if ((interface != NULL) &&
					    !CFSetContainsValue(seen, interface)) {
						CFDictionaryRef	capabilities;

						capabilities = SCNetworkInterfaceCopyCapability(interface, NULL);
						if (capabilities != NULL) {
							int		cap_current;
							int		cap_requested;
							CFDictionaryRef	options;

							options = SCNetworkInterfaceGetConfiguration(interface);
							cap_current   = __SCNetworkInterfaceCreateCapabilities(interface, -1, NULL);
							cap_requested = __SCNetworkInterfaceCreateCapabilities(interface, cap_current, options);

							SCPrint(TRUE, stdout,
								CFSTR("%sinterface = %@, current = %p, requested = %p\n%@\n"),
								(i == 0) ? "" : "\n",
								SCNetworkInterfaceGetBSDName(interface),
								(void *)(uintptr_t)cap_current,
								(void *)(uintptr_t)cap_requested,
								capabilities);
							CFRelease(capabilities);
						}

						CFSetAddValue(seen, interface);
					}
				}

				CFRelease(seen);
				CFRelease(services);
			}

			CFRelease(set);
		}

		CFRelease(prefs);
	}

	load_LinkConfiguration(CFBundleGetMainBundle(), (argc > 1) ? TRUE : FALSE);
	CFRunLoopRun();
	/* not reached */
	exit(0);
	return 0;
}
#endif

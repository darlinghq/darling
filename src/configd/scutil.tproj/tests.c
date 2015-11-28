/*
 * Copyright (c) 2000, 2001, 2003-2014 Apple Inc. All rights reserved.
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
 * July 9, 2001			Allan Nathanson <ajn@apple.com>
 * - added "-r" option for checking network reachability
 * - added "-w" option to check/wait for the presence of a
 *   dynamic store key.
 *
 * June 1, 2001			Allan Nathanson <ajn@apple.com>
 * - public API conversion
 *
 * November 9, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include "scutil.h"
#include "prefs.h"
#include "tests.h"

#include <netdb.h>
#include <netdb_async.h>
#include <notify.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <dnsinfo.h>
#include "dnsinfo_internal.h"
#include <network_information.h>
#include "SCNetworkReachabilityInternal.h"
#include <CommonCrypto/CommonDigest.h>


static Boolean	resolver_bypass;


static CF_RETURNS_RETAINED CFMutableDictionaryRef
_setupReachabilityOptions(int argc, char **argv, const char *interface)
{
	int			i;
	CFMutableDictionaryRef	options;

	options = CFDictionaryCreateMutable(NULL,
					    0,
					    &kCFTypeDictionaryKeyCallBacks,
					    &kCFTypeDictionaryValueCallBacks);

	for (i = 0; i < argc; i++) {
		if (strcasecmp(argv[i], "interface") == 0) {
			if (++i >= argc) {
				SCPrint(TRUE, stderr, CFSTR("No interface\n"));
				CFRelease(options);
				exit(1);
			}

			interface = argv[i];
			continue;
		}


		if (strcasecmp(argv[i], "server") == 0) {
			CFDictionarySetValue(options,
					     kSCNetworkReachabilityOptionServerBypass,
					     kCFBooleanFalse);
			continue;
		} else if (strcasecmp(argv[i], "no-server") == 0) {
			CFDictionarySetValue(options,
					     kSCNetworkReachabilityOptionServerBypass,
					     kCFBooleanTrue);
			continue;
		}


		if (strcasecmp(argv[i], "no-connection-on-demand") == 0) {
			CFDictionarySetValue(options,
					     kSCNetworkReachabilityOptionConnectionOnDemandBypass,
					     kCFBooleanTrue);
			continue;
		}

		if (strcasecmp(argv[i], "no-resolve") == 0) {
			CFDictionarySetValue(options,
					     kSCNetworkReachabilityOptionResolverBypass,
					     kCFBooleanTrue);
			resolver_bypass = TRUE;
			continue;
		}

		if (strcasecmp(argv[i], "ptr") == 0) {
			CFDictionarySetValue(options,
					     kSCNetworkReachabilityOptionPTRAddress,
					     kCFBooleanTrue);
			continue;
		}

		if (strlen(argv[i]) == 0) {
			continue;
		}

		SCPrint(TRUE, stderr, CFSTR("Unrecognized option: %s\n"), argv[i]);
		CFRelease(options);
		exit(1);
	}

	if (interface != NULL) {
		CFStringRef	str;

		if (if_nametoindex(interface) == 0) {
			SCPrint(TRUE, stderr, CFSTR("No interface: %s\n"), interface);
			exit(1);
		}

		str  = CFStringCreateWithCString(NULL, interface, kCFStringEncodingASCII);
		CFDictionarySetValue(options, kSCNetworkReachabilityOptionInterface, str);
		CFRelease(str);
	}

	if (CFDictionaryGetCount(options) == 0) {
		CFRelease(options);
		options = NULL;
	}

	return options;
}


static SCNetworkReachabilityRef
_setupReachability(int argc, char **argv, SCNetworkReachabilityContext *context)
{
	const char			*ip_address	= argv[0];
	char				*ip_addressN	= NULL;
	const char			*interface;
	CFMutableDictionaryRef		options		= NULL;
	const char			*remote_address	= NULL;
	char				*remote_addressN= NULL;
	const char			*remote_interface;
	struct sockaddr_in		sin;
	struct sockaddr_in6		sin6;
	SCNetworkReachabilityRef	target		= NULL;

	bzero(&sin, sizeof(sin));
	sin.sin_len    = sizeof(sin);
	sin.sin_family = AF_INET;

	bzero(&sin6, sizeof(sin6));
	sin6.sin6_len    = sizeof(sin6);
	sin6.sin6_family = AF_INET6;

	interface = strchr(ip_address, '%');
	if (interface != NULL) {
		ip_addressN = strdup(ip_address);
		ip_addressN[interface - ip_address] = '\0';
		ip_address = ip_addressN;
		interface++;
	}

	if ((argc > 1) && (strlen(argv[1]) > 0)) {
		remote_address = argv[1];

		remote_interface = strchr(remote_address, '%');
		if (remote_interface != NULL) {
			remote_addressN = strdup(remote_address);
			remote_addressN[remote_interface - remote_address] = '\0';
			remote_address = remote_addressN;
			remote_interface++;
		}
	}

	if (inet_aton(ip_address, &sin.sin_addr) == 1) {
		struct sockaddr_in	r_sin;

		if (argc > 1) {
			bzero(&r_sin, sizeof(r_sin));
			r_sin.sin_len    = sizeof(r_sin);
			r_sin.sin_family = AF_INET;
		}

		if ((argc == 1)
		    || (remote_address == NULL)
		    || (inet_aton(remote_address, &r_sin.sin_addr) == 0)) {
			if (argc > 2) {
				options = _setupReachabilityOptions(argc - 2, argv + 2, interface);
			}
			if (options == NULL) {
				target = SCNetworkReachabilityCreateWithAddress(NULL, (struct sockaddr *)&sin);
				if (context != NULL) {
					context->info = "by address";
				}
			} else {
				CFDataRef	data;

				data = CFDataCreate(NULL, (const UInt8 *)&sin, sizeof(sin));
				CFDictionarySetValue(options, kSCNetworkReachabilityOptionRemoteAddress, data);
				CFRelease(data);

				if (context != NULL) {
					if (CFDictionaryContainsKey(options,
								    kSCNetworkReachabilityOptionInterface)) {
						if (CFDictionaryGetCount(options) == 2) {
							context->info = "by address w/scope";
						} else {
							context->info = "by address w/scope and options";
						}
					} else {
						context->info = "by address w/options";
					}
				}
			}
		} else {
			if (remote_interface != NULL) {
				if ((interface != NULL) && (strcmp(interface, remote_interface) != 0)) {
					SCPrint(TRUE, stderr,
						CFSTR("Interface mismatch \"%s\" != \"%s\"\n"),
						interface,
						remote_interface);
					exit(1);
				}

				interface = remote_interface;
			}

			options = _setupReachabilityOptions(argc - 2, argv + 2, interface);
			if (options == NULL) {
				target = SCNetworkReachabilityCreateWithAddressPair(NULL,
										    (struct sockaddr *)&sin,
										    (struct sockaddr *)&r_sin);
				if (context != NULL) {
					context->info = "by address pair";
				}
			} else {
				CFDataRef	data;

				data = CFDataCreate(NULL, (const UInt8 *)&sin, sizeof(sin));
				CFDictionarySetValue(options, kSCNetworkReachabilityOptionLocalAddress, data);
				CFRelease(data);
				data = CFDataCreate(NULL, (const UInt8 *)&r_sin, sizeof(r_sin));
				CFDictionarySetValue(options, kSCNetworkReachabilityOptionRemoteAddress, data);
				CFRelease(data);

				if (context != NULL) {
					if (CFDictionaryContainsKey(options,
								    kSCNetworkReachabilityOptionInterface)) {
						if (CFDictionaryGetCount(options) == 3) {
							context->info = "by address pair w/scope";
						} else {
							context->info = "by address pair w/scope and options";
						}
					} else {
						context->info = "by address pair w/options";
					}
				}
			}
		}
	} else if (inet_pton(AF_INET6, ip_address, &sin6.sin6_addr) == 1) {
		struct sockaddr_in6	r_sin6;

		if (interface != NULL) {
			sin6.sin6_scope_id = if_nametoindex(interface);
		}

		if (argc > 1) {
			bzero(&r_sin6, sizeof(r_sin6));
			r_sin6.sin6_len    = sizeof(r_sin6);
			r_sin6.sin6_family = AF_INET6;
		}

		if ((argc == 1)
		    || (remote_address == NULL)
		    || (inet_pton(AF_INET6, remote_address, &r_sin6.sin6_addr) == 0)) {
			if (argc > 2) {
				options = _setupReachabilityOptions(argc - 2, argv + 2, NULL);
			}
			if (options == NULL) {
				target = SCNetworkReachabilityCreateWithAddress(NULL, (struct sockaddr *)&sin6);
				if (context != NULL) {
					context->info = "by (v6) address";
				}
			} else {
				CFDataRef	data;

				data = CFDataCreate(NULL, (const UInt8 *)&sin6, sizeof(sin6));
				CFDictionarySetValue(options, kSCNetworkReachabilityOptionRemoteAddress, data);
				CFRelease(data);

				if (context != NULL) {
					context->info = "by (v6) address w/options";
				}
			}
		} else {
			if (remote_interface != NULL) {
				r_sin6.sin6_scope_id = if_nametoindex(remote_interface);

				if ((interface != NULL) && (strcmp(interface, remote_interface) != 0)) {
					SCPrint(TRUE, stderr,
						CFSTR("Interface mismatch \"%s\" != \"%s\"\n"),
						interface,
						remote_interface);
					exit(1);
				}
			}

			options = _setupReachabilityOptions(argc - 2, argv + 2, NULL);
			if (options == NULL) {
				target = SCNetworkReachabilityCreateWithAddressPair(NULL,
										    (struct sockaddr *)&sin6,
										    (struct sockaddr *)&r_sin6);
				if (context != NULL) {
					context->info = "by (v6) address pair";
				}
			} else {
				CFDataRef	data;

				data = CFDataCreate(NULL, (const UInt8 *)&sin6, sizeof(sin6));
				CFDictionarySetValue(options, kSCNetworkReachabilityOptionLocalAddress, data);
				CFRelease(data);
				data = CFDataCreate(NULL, (const UInt8 *)&r_sin6, sizeof(r_sin6));
				CFDictionarySetValue(options, kSCNetworkReachabilityOptionRemoteAddress, data);
				CFRelease(data);

				if (context != NULL) {
					context->info = "by (v6) address pair w/options";
				}
			}
		}
	} else {
		if (argc == 1) {
			target = SCNetworkReachabilityCreateWithName(NULL, argv[0]);
			if (context != NULL) {
				context->info = "by name";
			}
		} else {
			options = _setupReachabilityOptions(argc - 1, argv + 1, NULL);
			if (options == NULL) {
				target = SCNetworkReachabilityCreateWithName(NULL, argv[0]);
				if (context != NULL) {
					context->info = "by name";
				}
			} else {
				CFStringRef	str;

				str  = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
				CFDictionarySetValue(options, kSCNetworkReachabilityOptionNodeName, str);
				CFRelease(str);

				if (context != NULL) {
					context->info = "by name w/options";
				}
			}
		}
	}

	if (ip_addressN != NULL) {
		free(ip_addressN);
	}

	if (remote_addressN != NULL) {
		free(remote_addressN);
	}

	if ((target == NULL) && (options != NULL)) {
		if (CFDictionaryContainsKey(options, kSCNetworkReachabilityOptionPTRAddress)) {
			CFDataRef	address;

			address = CFDictionaryGetValue(options, kSCNetworkReachabilityOptionRemoteAddress);
			if (address == NULL) {
				SCPrint(TRUE, stderr, CFSTR("No address\n"));
				exit(1);
			}
			CFDictionarySetValue(options, kSCNetworkReachabilityOptionPTRAddress, address);
			CFDictionaryRemoveValue(options, kSCNetworkReachabilityOptionRemoteAddress);

			if (context != NULL) {
				CFIndex	n	= CFDictionaryGetCount(options);

				if (n == 1) {
					context->info = "by PTR";
				} else if (CFDictionaryContainsKey(options,
								   kSCNetworkReachabilityOptionInterface)) {
					if (n == 2) {
						context->info = "by PTR w/scope";
					} else {
						context->info = "by PTR w/scope and options";
					}
				} else {
					context->info = "by PTR w/options";
				}
			}
		}

		target = SCNetworkReachabilityCreateWithOptions(NULL, options);
		CFRelease(options);
	}

	return target;
}


static void
_printReachability(SCNetworkReachabilityRef target)
{
	SCNetworkReachabilityFlags	flags;
	Boolean				ok;

	ok = SCNetworkReachabilityGetFlags(target, &flags);
	if (!ok) {
		SCPrint(TRUE, stderr, CFSTR("    could not determine reachability, %s\n"), SCErrorString(SCError()));
		return;
	}

	SCPrint(_sc_debug, stdout, CFSTR("flags = 0x%08x ("), flags);
	__SCNetworkReachabilityPrintFlags(flags);
	SCPrint(_sc_debug, stdout, CFSTR(")"));
	SCPrint(TRUE, stdout, CFSTR("\n"));

	if (resolver_bypass) {
		int	if_index;

		if_index = SCNetworkReachabilityGetInterfaceIndex(target);
		SCPrint(_sc_debug, stdout, CFSTR("interface index = %d\n"), if_index);
	}

	return;
}


__private_extern__
void
do_checkReachability(int argc, char **argv)
{
	SCNetworkReachabilityRef	target;

	target = _setupReachability(argc, argv, NULL);
	if (target == NULL) {
		SCPrint(TRUE, stderr, CFSTR("  Could not determine status: %s\n"), SCErrorString(SCError()));
		exit(1);
	}

	_printReachability(target);
	CFRelease(target);
	exit(0);
}


static void
_printNWIFlags(nwi_ifstate_flags flags)
{
	if (flags == 0) {
		return;
	}

	SCPrint(TRUE, stdout, CFSTR(" ("));
	if (flags & NWI_IFSTATE_FLAGS_HAS_IPV4) {
		SCPrint(TRUE, stdout, CFSTR("IPv4"));
		flags &= ~NWI_IFSTATE_FLAGS_HAS_IPV4;
		SCPrint(flags != 0, stdout, CFSTR(","));
	}
	if (flags & NWI_IFSTATE_FLAGS_HAS_IPV6) {
		SCPrint(TRUE, stdout, CFSTR("IPv6"));
		flags &= ~NWI_IFSTATE_FLAGS_HAS_IPV6;
		SCPrint(flags != 0, stdout, CFSTR(","));
	}
	if (flags & NWI_IFSTATE_FLAGS_HAS_DNS) {
		SCPrint(TRUE, stdout, CFSTR("DNS"));
		flags &= ~NWI_IFSTATE_FLAGS_HAS_DNS;
		SCPrint(flags != 0, stdout, CFSTR(","));
	}
	if (flags != 0) {
		SCPrint(TRUE, stdout, CFSTR("%p"), (void *)flags);
	}
	SCPrint(TRUE, stdout, CFSTR(")"));

	return;
}


static void
_printNWIInfo(nwi_ifstate_t ifstate)
{
	nwi_ifstate_flags		ifstate_flags = nwi_ifstate_get_flags(ifstate);
	SCNetworkReachabilityFlags	reach_flags = nwi_ifstate_get_reachability_flags(ifstate);
	const uint8_t			*signature;
	int				signature_length;
	const struct sockaddr		*vpn_addr = nwi_ifstate_get_vpn_server(ifstate);

	SCPrint(TRUE, stdout,
		CFSTR(" %7s : flags %p"),
		nwi_ifstate_get_ifname(ifstate),
		(void *)ifstate_flags);
	_printNWIFlags(ifstate_flags);

	SCPrint(TRUE, stdout, CFSTR("\n           reach 0x%08x ("), reach_flags);
	__SCNetworkReachabilityPrintFlags(reach_flags);
	SCPrint(TRUE, stdout, CFSTR(")"));

	if (vpn_addr != NULL) {
		char vpn_ntopbuf[INET6_ADDRSTRLEN];

		_SC_sockaddr_to_string(vpn_addr, vpn_ntopbuf, sizeof(vpn_ntopbuf));
		SCPrint(TRUE, stdout, CFSTR("\n           VPN server: %s"), vpn_ntopbuf);
	}

	signature = nwi_ifstate_get_signature(ifstate, AF_UNSPEC, &signature_length);
	if (signature != NULL) {
		CFDataRef	digest	= NULL;

		digest = CFDataCreate(NULL, signature, CC_SHA1_DIGEST_LENGTH);
		SCPrint(TRUE, stdout, CFSTR("\n           Signature Hash: %@"), digest);
		CFRelease(digest);
	} else {
		SCPrint(TRUE, stdout, CFSTR("\n           Signature Hash: <empty>"));
	}

	SCPrint(TRUE, stdout, CFSTR("\n           generation %llu\n"),
		nwi_ifstate_get_generation(ifstate));

	return;
}


static void
_printNWIReachInfo(nwi_state_t state, int af)
{
	uint32_t	reach_flags;

	reach_flags = nwi_state_get_reachability_flags(state, af);
	SCPrint(TRUE, stdout, CFSTR("\n   REACH : flags 0x%08x ("), reach_flags);
	__SCNetworkReachabilityPrintFlags(reach_flags);
	SCPrint(TRUE, stdout, CFSTR(")\n"));

	return;
}


static void
do_printNWI(int argc, char **argv, nwi_state_t state)
{
	nwi_ifstate_t	ifstate;

	if (state == NULL) {
		SCPrint(TRUE, stdout, CFSTR("No network information\n"));
		return;
	}

	if (argc > 0) {
		ifstate = nwi_state_get_ifstate(state, argv[0]);
		if (ifstate != NULL) {
			_printNWIInfo(ifstate);
		} else {
			SCPrint(TRUE, stdout, CFSTR("No network information (for %s)\n"), argv[0]);
		}
		return;
	}

	SCPrint(TRUE, stdout, CFSTR("Network information (generation %llu)"),
		nwi_state_get_generation(state));

	SCPrint(TRUE, stdout, CFSTR("\nIPv4 network interface information\n"));

	ifstate = nwi_state_get_first_ifstate(state, AF_INET);
	if (ifstate == NULL) {
		SCPrint(TRUE, stdout, CFSTR("   No IPv4 states found\n"));
	} else {
		while (ifstate != NULL) {
			_printNWIInfo(ifstate);
			ifstate = nwi_ifstate_get_next(ifstate, AF_INET);
		}
	}
	_printNWIReachInfo(state, AF_INET);

	SCPrint(TRUE, stdout, CFSTR("\nIPv6 network interface information\n"));

	ifstate = nwi_state_get_first_ifstate(state, AF_INET6);
	if (ifstate == NULL) {
		SCPrint(TRUE, stdout, CFSTR("   No IPv6 states found\n"));
	} else {
		while (ifstate != NULL) {
			_printNWIInfo(ifstate);
			ifstate = nwi_ifstate_get_next(ifstate, AF_INET6);
		}
	}
	_printNWIReachInfo(state, AF_INET6);

	return;
}


__private_extern__
void
do_showNWI(int argc, char **argv)
{
	nwi_state_t	state;

	state = nwi_state_copy();
	do_printNWI(argc, argv, state);
	if (state != NULL) {
		nwi_state_release(state);
	} else {
		exit(1);
	}

	exit(0);
}


__private_extern__
void
do_watchNWI(int argc, char **argv)
{
	nwi_state_t	state;
	int		status;
	int		token;

	state = nwi_state_copy();
	do_printNWI(argc, argv, state);
	if (state != NULL) {
		nwi_state_release(state);
	}

	status = notify_register_dispatch(nwi_state_get_notify_key(),
					  &token,
					  dispatch_get_main_queue(),
					  ^(int token){
						  nwi_state_t		state;
						  struct tm		tm_now;
						  struct timeval	tv_now;

						  (void)gettimeofday(&tv_now, NULL);
						  (void)localtime_r(&tv_now.tv_sec, &tm_now);
						  SCPrint(TRUE, stdout, CFSTR("\n*** %2d:%02d:%02d.%03d\n\n"),
							  tm_now.tm_hour,
							  tm_now.tm_min,
							  tm_now.tm_sec,
							  tv_now.tv_usec / 1000);

						  state = nwi_state_copy();
						  do_printNWI(argc, argv, state);
						  if (state != NULL) {
							  nwi_state_release(state);
						  }
					  });
	if (status != NOTIFY_STATUS_OK) {
		SCLog(TRUE, LOG_INFO, CFSTR("notify_register_dispatch() failed for nwi changes, status=%u"), status);
		exit(1);
	}

	CFRunLoopRun();
	exit(0);
}


static void
callout(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void *info)
{
	static int	n = 3;
	struct tm	tm_now;
	struct timeval	tv_now;

	(void)gettimeofday(&tv_now, NULL);
	(void)localtime_r(&tv_now.tv_sec, &tm_now);

	SCPrint(TRUE, stdout, CFSTR("\n*** %2d:%02d:%02d.%03d\n\n"),
		tm_now.tm_hour,
		tm_now.tm_min,
		tm_now.tm_sec,
		tv_now.tv_usec / 1000);
	SCPrint(TRUE, stdout, CFSTR("%2d: callback w/flags=0x%08x (info=\"%s\")\n"), n++, flags, (char *)info);
	SCPrint(TRUE, stdout, CFSTR("    %@\n"), target);
	_printReachability(target);
	SCPrint(TRUE, stdout, CFSTR("\n"));
	return;
}


__private_extern__
void
do_watchReachability(int argc, char **argv)
{
	SCNetworkReachabilityContext	context	= { 0, NULL, NULL, NULL, NULL };
	SCNetworkReachabilityRef	target;
	SCNetworkReachabilityRef	target_async;

	target = _setupReachability(argc, argv, NULL);
	if (target == NULL) {
		SCPrint(TRUE, stderr, CFSTR("  Could not determine status: %s\n"), SCErrorString(SCError()));
		exit(1);
	}

	target_async = _setupReachability(argc, argv, &context);
	if (target_async == NULL) {
		SCPrint(TRUE, stderr, CFSTR("  Could not determine status: %s\n"), SCErrorString(SCError()));
		exit(1);
	}

	// Normally, we don't want to make any calls to SCNetworkReachabilityGetFlags()
	// until after the "target" has been scheduled on a run loop.  Otherwise, we'll
	// end up making a synchronous DNS request and that's not what we want.
	//
	// To test the case were an application first calls SCNetworkReachabilityGetFlags()
	// we provide the "CHECK_REACHABILITY_BEFORE_SCHEDULING" environment variable.
	if (getenv("CHECK_REACHABILITY_BEFORE_SCHEDULING") != NULL) {
		CFRelease(target_async);
		target_async = CFRetain(target);
	}

	// Direct check of reachability
	SCPrint(TRUE, stdout, CFSTR(" 0: direct\n"));
	SCPrint(TRUE, stdout, CFSTR("   %@\n"), target);
	_printReachability(target);
	CFRelease(target);
	SCPrint(TRUE, stdout, CFSTR("\n"));

	// schedule the target
	SCPrint(TRUE, stdout, CFSTR(" 1: start\n"));
	SCPrint(TRUE, stdout, CFSTR("   %@\n"), target_async);
//	_printReachability(target_async);
	SCPrint(TRUE, stdout, CFSTR("\n"));

	if (!SCNetworkReachabilitySetCallback(target_async, callout, &context)) {
		printf("SCNetworkReachabilitySetCallback() failed: %s\n", SCErrorString(SCError()));
		exit(1);
	}

	if (doDispatch) {
		if (!SCNetworkReachabilitySetDispatchQueue(target_async, dispatch_get_main_queue())) {
			printf("SCNetworkReachabilitySetDispatchQueue() failed: %s\n", SCErrorString(SCError()));
			exit(1);
		}
	} else {
		if (!SCNetworkReachabilityScheduleWithRunLoop(target_async, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode)) {
			printf("SCNetworkReachabilityScheduleWithRunLoop() failed: %s\n", SCErrorString(SCError()));
			exit(1);
		}
	}

	// Note: now that we are scheduled on a run loop we can call SCNetworkReachabilityGetFlags()
	//       to get the current status.  For "names", a DNS lookup has already been initiated.
	SCPrint(TRUE, stdout, CFSTR(" 2: on %s\n"), doDispatch ? "dispatch queue" : "runloop");
	SCPrint(TRUE, stdout, CFSTR("   %@\n"), target_async);
	_printReachability(target_async);
	SCPrint(TRUE, stdout, CFSTR("\n"));

	CFRunLoopRun();
	exit(0);
}


static void
do_printDNSConfiguration(int argc, char **argv, dns_config_t *dns_config)
{
	SCNetworkReachabilityRef	target;

	if (dns_config == NULL) {
		SCPrint(TRUE, stdout, CFSTR("No DNS configuration available\n"));
		return;
	}

	if (argc > 1) {
		int				dns_config_index = -1;
		SCNetworkReachabilityFlags	flags = 0;
		Boolean				haveDNS = FALSE;
		Boolean				ok = FALSE;
		dns_resolver_t			*resolver;
		uint32_t			resolver_if_index;
		SCNetworkReachabilityPrivateRef targetPrivate;

		target = _setupReachability(argc, argv, NULL);

		targetPrivate = (SCNetworkReachabilityPrivateRef)target;

		if (targetPrivate->type != reachabilityTypeName) {
			SCPrint(TRUE, stdout, CFSTR("\"%s\" is not a hostname.\n"), argv[0]);
			exit(1);
		}

		ok = __SC_checkResolverReachabilityInternal(&store, &flags,
							    &haveDNS, targetPrivate->name,
							    &resolver_if_index, &dns_config_index);

		if (!ok) {
			SCPrint(TRUE, stdout, CFSTR("No DNS configuration available.\n" ));
			return;
		}

		SCPrint(TRUE, stdout, CFSTR("DNS configuration for %s\n"),
			targetPrivate->name);

		if (targetPrivate->if_index == 0) {
			resolver = dns_config->resolver[dns_config_index];
		} else {
			resolver = dns_config->scoped_resolver[dns_config_index];
		}

		_dns_resolver_print(resolver, dns_config_index + 1);

		if (target != NULL) CFRelease(target);
	} else {
		_dns_configuration_print(dns_config);
	}

	if (_sc_debug) {
		SCPrint(TRUE, stdout, CFSTR("\ngeneration = %llu\n"), dns_config->generation);
	}

	return;
}


__private_extern__
void
do_showDNSConfiguration(int argc, char **argv)
{
	dns_config_t	*dns_config;

	dns_config = dns_configuration_copy();
	do_printDNSConfiguration(argc, argv, dns_config);
	if (dns_config != NULL) {
		dns_configuration_free(dns_config);
	} else {
		exit(1);
	}

	exit(0);
}


__private_extern__
void
do_watchDNSConfiguration(int argc, char **argv)
{
	dns_config_t	*dns_config;
	int		status;
	int		token;

	dns_config = dns_configuration_copy();
	do_printDNSConfiguration(argc, argv, dns_config);
	if (dns_config != NULL) {
		dns_configuration_free(dns_config);
	}

	status = notify_register_dispatch(dns_configuration_notify_key(),
					  &token,
					  dispatch_get_main_queue(),
					  ^(int token){
						  dns_config_t		*dns_config;
						  struct tm		tm_now;
						  struct timeval	tv_now;

						  (void)gettimeofday(&tv_now, NULL);
						  (void)localtime_r(&tv_now.tv_sec, &tm_now);
						  SCPrint(TRUE, stdout, CFSTR("\n*** %2d:%02d:%02d.%03d\n\n"),
							  tm_now.tm_hour,
							  tm_now.tm_min,
							  tm_now.tm_sec,
							  tv_now.tv_usec / 1000);

						  dns_config = dns_configuration_copy();
						  do_printDNSConfiguration(argc, argv, dns_config);
						  if (dns_config != NULL) {
							  dns_configuration_free(dns_config);
						  }
					  });
	if (status != NOTIFY_STATUS_OK) {
		SCLog(TRUE, LOG_INFO, CFSTR("notify_register_dispatch() failed for DNS configuration changes, status=%u"), status);
		exit(1);
	}

	CFRunLoopRun();
	exit(0);
}


static void
showProxy(CFDictionaryRef proxy)
{
	CFMutableDictionaryRef	cleaned	= NULL;

	if (!_sc_debug) {
		cleaned = CFDictionaryCreateMutableCopy(NULL, 0, proxy);
		CFDictionaryRemoveValue(cleaned, kSCPropNetProxiesScoped);
		CFDictionaryRemoveValue(cleaned, kSCPropNetProxiesServices);
		CFDictionaryRemoveValue(cleaned, kSCPropNetProxiesSupplemental);
		proxy = cleaned;
	}

	SCPrint(TRUE, stdout, CFSTR("%@\n"), proxy);
	if (cleaned != NULL) CFRelease(cleaned);
	return;
}


__private_extern__
void
do_showProxyConfiguration(int argc, char **argv)
{
	CFMutableDictionaryRef	options = NULL;
	CFDictionaryRef		proxies;

	if (getenv("BYPASS_GLOBAL_PROXY") != NULL) {
		options = CFDictionaryCreateMutable(NULL, 0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);
		CFDictionaryAddValue(options, kSCProxiesNoGlobal, kCFBooleanTrue);
	}

	proxies = SCDynamicStoreCopyProxiesWithOptions(NULL, options);

	if (options != NULL) {
		CFRelease(options);
	}

	if (proxies != NULL) {
		CFStringRef	interface	= NULL;
		CFStringRef	server		= NULL;

		while (argc > 0) {
			if (strcasecmp(argv[0], "interface") == 0) {
				argv++;
				argc--;

				if (argc < 1) {
					SCPrint(TRUE, stderr, CFSTR("No interface\n"));
					exit(1);
				}

				if (if_nametoindex(argv[0]) == 0) {
					SCPrint(TRUE, stderr, CFSTR("No interface: %s\n"), argv[0]);
					exit(1);
				}

				interface = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
				argv++;
				argc--;
			} else {
				if (server != NULL) {
					CFRelease(server);
				}

				server = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
				argv++;
				argc--;
			}
		}

		if ((server != NULL) || (interface != NULL)) {
			CFArrayRef	matching;

			matching = SCNetworkProxiesCopyMatching(proxies, server, interface);
			if (matching != NULL) {
				CFIndex	i;
				CFIndex	n;

				if (server != NULL) {
					if (interface != NULL) {
						SCPrint(TRUE, stdout,
							CFSTR("server = %@, interface = %@\n"),
							server,
							interface);
					} else {
						SCPrint(TRUE, stdout,
							CFSTR("server = %@\n"),
							server);
					}
				} else {
					SCPrint(TRUE, stdout,
						CFSTR("interface = %@\n"),
						interface);
				}

				n = CFArrayGetCount(matching);
				for (i = 0; i < n; i++) {
					CFDictionaryRef	proxy;

					proxy = CFArrayGetValueAtIndex(matching, i);
					SCPrint(TRUE, stdout, CFSTR("\nproxy #%ld\n"), i + 1);
					showProxy(proxy);
				}

				CFRelease(matching);
			} else {
				SCPrint(TRUE, stdout, CFSTR("No matching proxy configurations\n"));
			}
		} else {
			showProxy(proxies);
		}

		if (interface != NULL) CFRelease(interface);
		if (server != NULL) CFRelease(server);
		CFRelease(proxies);
	} else {
		SCPrint(TRUE, stdout, CFSTR("No proxy configuration available\n"));
	}

	exit(0);
}


__private_extern__
void
do_snapshot(int argc, char **argv)
{
	if (!SCDynamicStoreSnapshot(store)) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
	}

	(void) _SCNetworkReachabilityServer_snapshot();

	return;
}


__private_extern__
void
do_renew(char *if_name)
{
	CFArrayRef	services;
	Boolean		ok	= FALSE;

	if ((if_name == NULL) || (strlen(if_name) == 0)) {
		SCPrint(TRUE, stderr, CFSTR("No interface name\n"));
		exit(1);
	}

	if (getenv("ATTEMPT_DHCP_RENEW_WITH_SCDYNAMICSTORE") != NULL) {
		CFArrayRef	interfaces;

		interfaces = SCNetworkInterfaceCopyAll();
		if (interfaces != NULL) {
			CFIndex		i;
			CFStringRef	match_name;
			CFIndex		n;

			match_name = CFStringCreateWithCString(NULL, if_name, kCFStringEncodingASCII);
			assert(match_name != NULL);

			n = CFArrayGetCount(interfaces);
			for (i = 0; i < n; i++) {
				CFStringRef		bsd_name;
				SCNetworkInterfaceRef	interface;

				interface = CFArrayGetValueAtIndex(interfaces, i);
				bsd_name = SCNetworkInterfaceGetBSDName(interface);
				if (_SC_CFEqual(bsd_name, match_name)) {
					// if match
					ok = SCNetworkInterfaceForceConfigurationRefresh(interface);
					if (!ok) {
						int	status;

						status = SCError();
						if (status != kSCStatusAccessError) {
							SCPrint(TRUE, stderr, CFSTR("%s\n"), SCErrorString(status));
							exit(1);
						}

						// ... and if can't write the SCDynamicStore, try w/prefs
					}

					break;
				}
			}

			CFRelease(match_name);
			CFRelease(interfaces);
		}

		if (ok) {
			exit(0);
		}
	}

	do_prefs_init();	/* initialization */
	do_prefs_open(0, NULL);	/* open default prefs */

	services = SCNetworkServiceCopyAll(prefs);
	if (services != NULL) {
		CFIndex		i;
		CFStringRef	match_name;
		CFIndex		n;

		match_name = CFStringCreateWithCString(NULL, if_name, kCFStringEncodingASCII);
		assert(match_name != NULL);

		n = CFArrayGetCount(services);
		for (i = 0; i < n; i++) {
			CFStringRef		bsd_name;
			SCNetworkInterfaceRef	interface;
			SCNetworkServiceRef	service;

			service = CFArrayGetValueAtIndex(services, i);
			interface = SCNetworkServiceGetInterface(service);
			if (interface == NULL) {
				// if no interface
				continue;
			}

			bsd_name = SCNetworkInterfaceGetBSDName(interface);
			if (_SC_CFEqual(bsd_name, match_name)) {
				// if match
				ok = SCNetworkInterfaceForceConfigurationRefresh(interface);
				if (!ok) {
					SCPrint(TRUE, stderr, CFSTR("%s\n"), SCErrorString(SCError()));
					exit(1);
				}

				break;
			}
		}

		CFRelease(match_name);
		CFRelease(services);
	}

	if (!ok) {
		SCPrint(TRUE, stderr, CFSTR("No interface\n"));
		exit(1);
	}

	_prefs_close();
	exit(0);
}


static void
waitKeyFound()
{
	exit(0);
}


static void
waitTimeout(int sigraised)
{
	exit(1);
}


__private_extern__
void
do_wait(char *waitKey, int timeout)
{
	struct itimerval	itv;
	CFStringRef		key;
	CFMutableArrayRef	keys;
	Boolean			ok;

	store = SCDynamicStoreCreate(NULL, CFSTR("scutil (wait)"), waitKeyFound, NULL);
	if (store == NULL) {
		SCPrint(TRUE, stderr,
			CFSTR("SCDynamicStoreCreate() failed: %s\n"), SCErrorString(SCError()));
		exit(1);
	}

	key  = CFStringCreateWithCString(NULL, waitKey, kCFStringEncodingUTF8);

	keys = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	CFArrayAppendValue(keys, key);
	ok = SCDynamicStoreSetNotificationKeys(store, keys, NULL);
	CFRelease(keys);
	if (!ok) {
		SCPrint(TRUE, stderr,
			CFSTR("SCDynamicStoreSetNotificationKeys() failed: %s\n"), SCErrorString(SCError()));
		exit(1);
	}

	notifyRls = SCDynamicStoreCreateRunLoopSource(NULL, store, 0);
	if (!notifyRls) {
		SCPrint(TRUE, stderr,
			CFSTR("SCDynamicStoreCreateRunLoopSource() failed: %s\n"), SCErrorString(SCError()));
		exit(1);
	}

	CFRunLoopAddSource(CFRunLoopGetCurrent(), notifyRls, kCFRunLoopDefaultMode);

	value = SCDynamicStoreCopyValue(store, key);
	if (value) {
		/* if the key is already present */
		exit(0);
	}
	CFRelease(key);

	if (timeout > 0) {
		signal(SIGALRM, waitTimeout);
		bzero(&itv, sizeof(itv));
		itv.it_value.tv_sec = timeout;
		if (setitimer(ITIMER_REAL, &itv, NULL) == -1) {
			SCPrint(TRUE, stderr,
				CFSTR("setitimer() failed: %s\n"), strerror(errno));
			exit(1);
		}
	}

	CFRunLoopRun();
}

#ifdef	TEST_DNS_CONFIGURATION

Boolean			doDispatch	= FALSE;
CFRunLoopSourceRef	notifyRls	= NULL;
SCDynamicStoreRef	store		= NULL;
CFPropertyListRef	value		= NULL;

int
main(int argc, char **argv)
{
	dns_config_t	*dns_config;

fprintf(stdout, "copy configuration\n");
	dns_config = dns_configuration_copy();
	if (dns_config != NULL) {

fprintf(stdout, "sleeping for 120 seconds\n");
sleep(120);

fprintf(stdout, "sending ack\n");
		_dns_configuration_ack(dns_config, "TEST_DNS_CONFIGURATION");

fprintf(stdout, "sleeping for 120 seconds\n");
sleep(120);

		dns_configuration_free(dns_config);
	}

	  do_showDNSConfiguration(argc, argv);
	exit(0);
}

#endif	// TEST_DNS_CONFIGURATION

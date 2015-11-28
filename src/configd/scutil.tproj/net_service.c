/*
 * Copyright (c) 2004-2010, 2013, 2014 Apple Inc. All rights reserved.
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
 * August 5, 2004			Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#include "scutil.h"
#include "net.h"
#include "net_service.h"
#include "net_interface.h"
#include "net_protocol.h"
#include "prefs.h"


/* -------------------- */


static SCNetworkServiceRef
_find_service(char *match)
{
	Boolean			allowIndex	= TRUE;
	CFIndex			i;
	CFIndex			n;
	CFStringRef		select_name	= NULL;
	SCNetworkServiceRef	selected	= NULL;

	if (services == NULL) {
		if (net_set == NULL) {
			SCPrint(TRUE, stdout, CFSTR("set not selected\n"));
			return NULL;
		}

		services = SCNetworkSetCopyServices(net_set);
		if (services == NULL) {
			SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
			return NULL;
		}

		allowIndex = FALSE;
	}

	// try to select the service by its serviceID

	select_name = CFStringCreateWithCString(NULL, match, kCFStringEncodingUTF8);

	n = CFArrayGetCount(services);
	for (i = 0; i < n; i++) {
		SCNetworkServiceRef	service;
		CFStringRef		serviceID;

		service   = CFArrayGetValueAtIndex(services, i);
		serviceID = SCNetworkServiceGetServiceID(service);
		if (CFEqual(select_name, serviceID)) {
			selected = service;
			goto done;
		}
	}

	// try to select the service by its name

	for (i = 0; i < n; i++) {
		SCNetworkServiceRef	service;
		CFStringRef		serviceName;

		service     = CFArrayGetValueAtIndex(services, i);
		serviceName = SCNetworkServiceGetName(service);
		if ((serviceName != NULL) && CFEqual(select_name, serviceName)) {
			if (selected == NULL) {
				selected = service;
			} else {
				// if multiple services match
				selected = NULL;
				SCPrint(TRUE, stdout, CFSTR("multiple services match\n"));
				goto done;
			}
		}
	}

	if (selected != NULL) {
		goto done;
	}

	// try to select the service by its name (case insensitive)

	for (i = 0; i < n; i++) {
		SCNetworkServiceRef	service;
		CFStringRef		serviceName;

		service     = CFArrayGetValueAtIndex(services, i);
		serviceName = SCNetworkServiceGetName(service);
		if ((serviceName != NULL) &&
		    CFStringCompare(select_name,
				    serviceName,
				    kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
			if (selected == NULL) {
				selected = service;
			} else {
				// if multiple services match
				selected = NULL;
				SCPrint(TRUE, stdout, CFSTR("multiple services match\n"));
				goto done;
			}
		}
	}

	if (selected != NULL) {
		goto done;
	}

	// try to select the service by its [BSD] interface name

	for (i = 0; i < n; i++) {
		SCNetworkInterfaceRef	interface;
		CFStringRef		interfaceName	= NULL;
		SCNetworkServiceRef	service;

		service     = CFArrayGetValueAtIndex(services, i);

		interface = SCNetworkServiceGetInterface(service);
		while ((interface != NULL) && (interfaceName == NULL)) {
			interfaceName = SCNetworkInterfaceGetBSDName(interface);
			if (interfaceName == NULL) {
				interface = SCNetworkInterfaceGetInterface(interface);
			}
		}

		if (interfaceName == NULL) {
			continue;
		}

		if (CFStringCompare(select_name,
				    interfaceName,
				    kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
			if (selected == NULL) {
				selected = service;
			} else {
				// if multiple services match
				selected = NULL;
				SCPrint(TRUE, stdout, CFSTR("multiple services match\n"));
				goto done;
			}
		}
	}

	if (selected != NULL) {
		goto done;
	}

	// try to select the service by its index

	if (allowIndex) {
		char	*end;
		char	*str	= match;
		long	val;

		errno = 0;
		val = strtol(str, &end, 10);
		if ((*str != '\0') && (*end == '\0') && (errno == 0)) {
			if ((val > 0) && (val <= n)) {
				selected = CFArrayGetValueAtIndex(services, val - 1);
			}
		}
	}

	if (selected != NULL) {
		goto done;
	}

	SCPrint(TRUE, stdout, CFSTR("no match, which service?\n"));

    done :

	if (select_name != NULL) CFRelease(select_name);
	return selected;
}


/* -------------------- */


__private_extern__
void
create_service(int argc, char **argv)
{
	SCNetworkInterfaceRef	interface;
	CFStringRef		interfaceName;
	Boolean			ok;
	SCNetworkServiceRef	service		= NULL;
	CFStringRef		serviceName;
	CFStringRef		setName;
	CFArrayRef		supported;

	if (prefs == NULL) {
		SCPrint(TRUE, stdout, CFSTR("network configuration not open\n"));
		return;
	}

	if (net_set == NULL) {
		SCPrint(TRUE, stdout, CFSTR("set not selected\n"));
		return;
	}

	if (argc < 1) {
		if (net_interface == NULL) {
			SCPrint(TRUE, stdout, CFSTR("no network interface selected\n"));
			return;
		}

		interface = net_interface;
	} else {
		int	nArgs;

		interface = _find_interface(argc, argv, &nArgs);
		argv += nArgs;
		argc -= nArgs;
	}

	if (interface == NULL) {
		return;
	}

	supported = SCNetworkInterfaceGetSupportedProtocolTypes(interface);
	if (supported == NULL) {
		SCPrint(TRUE, stdout, CFSTR("no network protocols are supported over this interface\n"));
		return;
	}

	service = SCNetworkServiceCreate(prefs, interface);
	if (service == NULL) {
		SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
		goto done;
	}

	if ((argc > 0) && (strlen(argv[0]) > 0)) {
		Boolean         ok;

		serviceName = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
//		argv++;
//		argc--;

		ok = SCNetworkServiceSetName(service, serviceName);
		CFRelease(serviceName);
		if (!ok) {
			SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
			(void)SCNetworkServiceRemove(service);
			goto done;
		}
	}

	ok = SCNetworkServiceEstablishDefaultConfiguration(service);
	if (!ok) {
		SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
		(void)SCNetworkServiceRemove(service);
		goto done;
	}

	ok = SCNetworkSetAddService(net_set, service);
	if (!ok) {
		SCPrint(TRUE, stdout, CFSTR("service not created: %s\n"), SCErrorString(SCError()));
		(void)SCNetworkServiceRemove(service);
		goto done;
	}

	_prefs_changed = TRUE;

	if (net_service != NULL) CFRelease(net_service);
	net_service = CFRetain(service);

	serviceName = SCNetworkServiceGetName(service);
	if (serviceName != NULL) {
		SCPrint(TRUE, stdout,
			CFSTR("service \"%@\" (%@) created and selected\n"),
			serviceName,
			SCNetworkServiceGetServiceID(service));
	} else {
		SCPrint(TRUE, stdout,
			CFSTR("service ID \"%@\" created and selected\n"),
			SCNetworkServiceGetServiceID(service));
	}

	setName = SCNetworkSetGetName(net_set);
	if (setName != NULL) {
		SCPrint(TRUE, stdout, CFSTR("& added to set \"%@\"\n"), setName);
	} else {
		SCPrint(TRUE, stdout, CFSTR("& added to set ID \"%@\"\n"),
			SCNetworkSetGetSetID(net_set));
	}

	if (net_interface != NULL) CFRelease(net_interface);
	net_interface = SCNetworkServiceGetInterface(net_service);
	if (net_interface != NULL) {
		CFRetain(net_interface);
	}

	interfaceName = SCNetworkInterfaceGetLocalizedDisplayName(interface);
	if (interfaceName == NULL) {
		interfaceName = SCNetworkInterfaceGetBSDName(interface);
	}
	if (interfaceName == NULL) {
		interfaceName = SCNetworkInterfaceGetInterfaceType(interface);
	}

	SCPrint(TRUE, stdout,
		CFSTR("& interface \"%@\" selected\n"),
		interfaceName);

	if (protocols != NULL) {
		CFRelease(protocols);
		protocols = NULL;
	}

	if (net_protocol != NULL) {
		CFRelease(net_protocol);
		net_protocol = NULL;
		SCPrint(TRUE, stdout, CFSTR("& no protocol selected\n"));
	}

	if (services != NULL) {
		CFRelease(services);
		services = NULL;
	}

    done :

	if (service != NULL) CFRelease(service);
	return;
}


__private_extern__
void
disable_service(int argc, char **argv)
{
	SCNetworkServiceRef	service;

	if (argc == 1) {
		service = _find_service(argv[0]);
	} else {
		if (net_service != NULL) {
			service = net_service;
		} else {
			SCPrint(TRUE, stdout, CFSTR("service not selected\n"));
			return;
		}
	}

	if (service == NULL) {
		return;
	}

	if (!SCNetworkServiceSetEnabled(service, FALSE)) {
		SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
		return;
	}

	_prefs_changed = TRUE;
	return;
}


__private_extern__
void
enable_service(int argc, char **argv)
{
	SCNetworkServiceRef	service;

	if (argc == 1) {
		service = _find_service(argv[0]);
	} else {
		if (net_service != NULL) {
			service = net_service;
		} else {
			SCPrint(TRUE, stdout, CFSTR("service not selected\n"));
			return;
		}
	}

	if (service == NULL) {
		return;
	}

	if (!SCNetworkServiceSetEnabled(service, TRUE)) {
		SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
		return;
	}

	_prefs_changed = TRUE;
	return;
}


__private_extern__
void
remove_service(int argc, char **argv)
{
	SCNetworkServiceRef	service		= NULL;
	CFStringRef		serviceName;

	if (argc == 1) {
		service = _find_service(argv[0]);
	} else {
		if (net_service != NULL) {
			service = net_service;
		}
	}

	if (service == NULL) {
		return;
	}

	CFRetain(service);

	if (!SCNetworkServiceRemove(service)) {
		SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
		goto done;
	}

	_prefs_changed = TRUE;

	serviceName = SCNetworkServiceGetName(service);
	if (serviceName != NULL) {
		SCPrint(TRUE, stdout, CFSTR("service \"%@\" removed\n"), serviceName);
	} else {
		SCPrint(TRUE, stdout,
			CFSTR("service ID \"%@\" removed\n"),
			SCNetworkServiceGetServiceID(service));
	}

	if ((net_service != NULL) && CFEqual(service, net_service)) {
		CFRelease(net_service);
		net_service = NULL;
		SCPrint(TRUE, stdout, CFSTR("& no service selected\n"));

		if (protocols != NULL) {
			CFRelease(protocols);
			protocols = NULL;
		}

		if (net_protocol != NULL) {
			CFRelease(net_protocol);
			net_protocol = NULL;
			SCPrint(TRUE, stdout, CFSTR("& no protocol selected\n"));
		}

		if (net_interface != NULL) {
			CFRelease(net_interface);
			net_interface = NULL;
			SCPrint(TRUE, stdout, CFSTR("& no interface selected\n"));
		}
	}

	if (services != NULL) {
		CFRelease(services);
		services = NULL;
	}

    done :

	CFRelease(service);
	return;
}


__private_extern__
void
select_service(int argc, char **argv)
{
	SCNetworkInterfaceRef	interface;
	SCNetworkServiceRef	service;
	CFStringRef		serviceName;

	service = _find_service(argv[0]);

	if (service == NULL) {
		return;
	}

	if (net_service != NULL) CFRelease(net_service);
	net_service = CFRetain(service);

	serviceName = SCNetworkServiceGetName(service);
	if (serviceName != NULL) {
		SCPrint(TRUE, stdout, CFSTR("service \"%@\" selected\n"), serviceName);
	} else {
		SCPrint(TRUE, stdout,
			CFSTR("service ID \"%@\" selected\n"),
			SCNetworkServiceGetServiceID(service));
	}

	interface = SCNetworkServiceGetInterface(service);
	if (interface != NULL) {
		CFStringRef	interfaceName;

		if (net_interface != NULL) CFRelease(net_interface);
		net_interface = CFRetain(interface);

		interfaceName = SCNetworkInterfaceGetLocalizedDisplayName(interface);
		if (interfaceName == NULL) {
			interfaceName = SCNetworkInterfaceGetBSDName(interface);
		}
		if (interfaceName == NULL) {
			interfaceName = SCNetworkInterfaceGetInterfaceType(interface);
		}

		SCPrint(TRUE, stdout,
			CFSTR("& interface \"%@\" selected\n"),
			interfaceName);
	} else {
		if (net_interface != NULL) {
			CFRelease(net_interface);
			net_interface = NULL;
			SCPrint(TRUE, stdout, CFSTR("& no interface selected\n"));
		}
	}

	if (protocols != NULL) {
		CFRelease(protocols);
		protocols = NULL;
	}

	if (net_protocol != NULL) {
		CFRelease(net_protocol);
		net_protocol = NULL;
		SCPrint(TRUE, stdout, CFSTR("& no protocol selected\n"));
	}

	return;
}


__private_extern__
void
set_service(int argc, char **argv)
{
	Boolean	ok;

	if (net_service == NULL) {
		SCPrint(TRUE, stdout, CFSTR("service not selected\n"));
		return;
	}

	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("set what?\n"));
		return;
	}

	while (argc > 0) {
		char	*command;

		command = argv[0];
		argv++;
		argc--;

		if (strcmp(command, "name") == 0) {
			CFStringRef	serviceName;

			if (argc < 1) {
				SCPrint(TRUE, stdout, CFSTR("name not specified\n"));
				return;
			}

			serviceName = (strlen(argv[0]) > 0)
					? CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8)
					: NULL;
			argv++;
			argc--;

			ok = SCNetworkServiceSetName(net_service, serviceName);
			if (serviceName != NULL) CFRelease(serviceName);
			if (!ok) {
				SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
				return;
			}

			_prefs_changed = TRUE;
		} else if (strcmp(command, "order") == 0) {

			char		*end;
			long		newIndex;
			CFIndex		nServices;
			char		*str;
			CFArrayRef	services;

			services = SCNetworkSetCopyServices(net_set);
			nServices = CFArrayGetCount(services);
			CFRelease(services);

			if (argc < 1) {
				SCPrint(TRUE, stdout, CFSTR("order not specified\n"));
				return;
			}

			if (net_set == NULL) {
				SCPrint(TRUE, stdout, CFSTR("set not selected\n"));
				return;
			}

			str = argv[0];
			argv++;
			argc--;

			errno = 0;
			newIndex = strtol(str, &end, 10);
			if ((*str != '\0') && (*end == '\0') && (errno == 0)) {
				if ((newIndex > 0) && (newIndex <= nServices)) {
					CFIndex			curIndex;
					CFMutableArrayRef	newOrder;
					CFArrayRef		order;
					CFStringRef		serviceID	= SCNetworkServiceGetServiceID(net_service);

					order = SCNetworkSetGetServiceOrder(net_set);
					if (order == NULL) {
						newOrder = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
					} else {
						newOrder = CFArrayCreateMutableCopy(NULL, 0, order);
					}

					curIndex = CFArrayGetFirstIndexOfValue(newOrder,
									       CFRangeMake(0, CFArrayGetCount(newOrder)),
									       serviceID);
					if (curIndex != kCFNotFound) {
						CFArrayRemoveValueAtIndex(newOrder, curIndex);
					}

					if (newIndex <= CFArrayGetCount(newOrder)) {
						CFArrayInsertValueAtIndex(newOrder, newIndex - 1, serviceID);
					} else {
						CFArrayAppendValue(newOrder, serviceID);
					}

					ok = SCNetworkSetSetServiceOrder(net_set, newOrder);
					CFRelease(newOrder);
					if (!ok) {
						SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
						return;
					}

					_prefs_changed = TRUE;
				} else {
					SCPrint(TRUE, stdout, CFSTR("set order to what?\n"));
					return;
				}
			} else {
				SCPrint(TRUE, stdout, CFSTR("set what?\n"));
				return;
			}
		} else if (strcmp(command, "rank") == 0) {
			SCNetworkServicePrimaryRank	rank	= kSCNetworkServicePrimaryRankDefault;
			SCNetworkServiceRef		service	= (argc < 2) ? net_service : NULL;

			if (argc < 1) {
				SCPrint(TRUE, stdout, CFSTR("rank not specified\n"));
				return;
			}

			if (strlen(argv[0]) > 0) {
				if (strcasecmp(argv[0], "Never") == 0) {
					rank = kSCNetworkServicePrimaryRankNever;
				} else if ((service != net_service) && (strcasecmp(argv[0], "First") == 0)) {
					rank = kSCNetworkServicePrimaryRankFirst;
				} else if ((service != net_service) && (strcasecmp(argv[0], "Last") == 0)) {
					rank = kSCNetworkServicePrimaryRankLast;
				} else if ((service != net_service) && (strcasecmp(argv[0], "Scoped") == 0)) {
					rank = kSCNetworkServicePrimaryRankScoped;
				} else {
					SCPrint(TRUE, stdout, CFSTR("rank not valid\n"));
					return;
				}
			}
			argv++;
			argc--;

			if (service == NULL) {
				CFStringRef		serviceID;
				SCDynamicStoreRef	store;

				store = SCDynamicStoreCreate(NULL,
							     CFSTR("scutil (set primary rank)"),
							     NULL,
							     NULL);
				serviceID = SCNetworkServiceGetServiceID(net_service);
				service = _SCNetworkServiceCopyActive(store, serviceID);
				CFRelease(store);

				argv++;
				argc--;
			}

			ok = SCNetworkServiceSetPrimaryRank(service, rank);
			if (service != net_service) CFRelease(service);
			if (ok) {
				if (service == net_service) _prefs_changed = TRUE;
			} else {
				SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
				return;
			}
		} else if (strcmp(command, "id") == 0) {
			CFStringRef	serviceID;

			if ((argc < 1) || (strlen(argv[0]) == 0)) {
				SCPrint(TRUE, stdout, CFSTR("set id not specified\n"));
				return;
			}

			serviceID = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
			argv++;
			argc--;

			ok = _SCNetworkServiceSetServiceID(net_service, serviceID);
			CFRelease(serviceID);
			if (!ok) {
				SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
				return;
			}

			_prefs_changed = TRUE;
		} else {
			SCPrint(TRUE, stdout, CFSTR("set what?\n"));
		}
	}

	return;
}


static void
__show_service_interface(SCNetworkServiceRef service, const char *prefix)
{
	CFStringRef		description;
	SCNetworkInterfaceRef	interface;

	interface = SCNetworkServiceGetInterface(service);
	if (interface == NULL) {
		return;
	}

	description = _interface_description(interface);
	SCPrint(TRUE, stdout, CFSTR("%s%@\n"), prefix, description);
	CFRelease(description);

	return;
}

static void
__show_service_protocols(SCNetworkServiceRef service, const char *prefix, Boolean skipEmpty)
{
	CFIndex		i;
	CFIndex		n;
	CFArrayRef	protocols;

	protocols = SCNetworkServiceCopyProtocols(service);
	if (protocols == NULL) {
		return;
	}

	n = CFArrayGetCount(protocols);
	if (n > 1) {
		CFMutableArrayRef	sorted;

		sorted = CFArrayCreateMutableCopy(NULL, 0, protocols);
		CFArraySortValues(sorted,
				  CFRangeMake(0, n),
				  _compare_protocols,
				  NULL);
		CFRelease(protocols);
		protocols = sorted;
	}

	for (i = 0; i < n; i++) {
		CFStringRef		description;
		SCNetworkProtocolRef	protocol;

		protocol = CFArrayGetValueAtIndex(protocols, i);
		description = _protocol_description(protocol, skipEmpty);
		if (description != NULL) {
			CFStringRef	protocolType;

			protocolType = SCNetworkProtocolGetProtocolType(protocol);
			SCPrint(TRUE, stdout,
				CFSTR("%s%@%*s : %@\n"),
				prefix,
				protocolType,
				(int)(sizeof("Interface") - CFStringGetLength(protocolType) - 1),
				"",
				description);
			CFRelease(description);
		}
	}

	CFRelease(protocols);
	return;
}


__private_extern__
void
show_service(int argc, char **argv)
{
	SCNetworkInterfaceRef		interface;
	CFArrayRef			protocols;
	SCNetworkServiceRef		service;
	CFStringRef			serviceName;
	SCNetworkServicePrimaryRank	serviceRank;

	if (argc == 1) {
		service = _find_service(argv[0]);
	} else {
		if (net_service != NULL) {
			service = net_service;
		} else {
			SCPrint(TRUE, stdout, CFSTR("service not selected\n"));
			return;
		}
	}

	if (service == NULL) {
		return;
	}

	SCPrint(TRUE, stdout, CFSTR("service id           = %@\n"), SCNetworkServiceGetServiceID(service));

	serviceName = SCNetworkServiceGetName(service);
	SCPrint(TRUE, stdout, CFSTR("name                 = %@\n"),
		(serviceName != NULL) ? serviceName : CFSTR(""));

	serviceRank = SCNetworkServiceGetPrimaryRank(service);
	switch (serviceRank) {
		case kSCNetworkServicePrimaryRankDefault :
			// nothing to report
			break;
		case kSCNetworkServicePrimaryRankFirst :
			SCPrint(TRUE, stdout, CFSTR("primary rank         = FIRST\n"));
			break;
		case kSCNetworkServicePrimaryRankLast :
			SCPrint(TRUE, stdout, CFSTR("primary rank         = LAST\n"));
			break;
		case kSCNetworkServicePrimaryRankNever :
			SCPrint(TRUE, stdout, CFSTR("primary rank         = NEVER\n"));
			break;
		case kSCNetworkServicePrimaryRankScoped :
			SCPrint(TRUE, stdout, CFSTR("primary rank         = SCOPED\n"));
			break;
		default :
			SCPrint(TRUE, stdout, CFSTR("primary rank         = %d\n"), serviceRank);
			break;
	}

	interface = SCNetworkServiceGetInterface(service);
	if (interface != NULL) {
		CFStringRef	interfaceName;

		interfaceName = SCNetworkInterfaceGetLocalizedDisplayName(interface);
		if (interfaceName != NULL) {
			CFRetain(interfaceName);
		} else {
			interfaceName = _interface_description(interface);
		}
		if (interfaceName != NULL) {
			SCPrint(TRUE, stdout, CFSTR("interface            = %@\n"), interfaceName);
			CFRelease(interfaceName);
		}
	} else {
		SCPrint(TRUE, stdout, CFSTR("\n  No interface!\n\n"));
	}

	protocols = SCNetworkServiceCopyProtocols(service);
	if (protocols != NULL) {
		CFIndex	n;

		n = CFArrayGetCount(protocols);
		if (n > 1) {
			CFMutableArrayRef	sorted;

			sorted = CFArrayCreateMutableCopy(NULL, 0, protocols);
			CFArraySortValues(sorted,
					  CFRangeMake(0, n),
					  _compare_protocols,
					  NULL);
			CFRelease(protocols);
			protocols = sorted;
		}

		if (n > 0) {
			CFIndex	i;

			SCPrint(TRUE, stdout, CFSTR("configured protocols = "));
			for (i = 0; i < n; i++) {
				SCNetworkProtocolRef	protocol;

				protocol = CFArrayGetValueAtIndex(protocols, i);
				SCPrint(TRUE, stdout, CFSTR("%s%@"),
					(i == 0) ? "" : ", ",
					SCNetworkProtocolGetProtocolType(protocol));
			}
			SCPrint(TRUE, stdout, CFSTR("\n"));

			__show_service_protocols(service, "  ", FALSE);
		} else {
			SCPrint(TRUE, stdout, CFSTR("no configured protocols\n"));
		}

		CFRelease(protocols);
	}

	if (_sc_debug) {
		SCPrint(TRUE, stdout, CFSTR("\n%@\n"), service);
	}

	return;
}


__private_extern__
void
show_services(int argc, char **argv)
{
	CFIndex	i;
	CFIndex	n;

	if (prefs == NULL) {
		SCPrint(TRUE, stdout, CFSTR("network configuration not open\n"));
		return;
	}

	if (argc == 1) {
		if (services != NULL) CFRelease(services);
		services = SCNetworkServiceCopyAll(prefs);
	} else {
		if (net_set == NULL) {
			SCPrint(TRUE, stdout, CFSTR("set not selected\n"));
			return;
		}

		if (services != NULL) CFRelease(services);
		services = SCNetworkSetCopyServices(net_set);
		n = (services != NULL) ? CFArrayGetCount(services) : 0;
		if (n > 1) {
			CFArrayRef		order;
			CFMutableArrayRef	sorted;

			order  = SCNetworkSetGetServiceOrder(net_set);
			sorted = CFArrayCreateMutableCopy(NULL, 0, services);
			CFArraySortValues(sorted,
					  CFRangeMake(0, CFArrayGetCount(sorted)),
					  _SCNetworkServiceCompare,
					  (void *)order);
			CFRelease(services);
			services = sorted;
		}
	}

	if (services == NULL) {
		SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
		return;
	}

	n = CFArrayGetCount(services);
	for (i = 0; i < n; i++) {
		SCNetworkServiceRef	service;
		CFStringRef		serviceName;
		CFStringRef		serviceID;

		service     = CFArrayGetValueAtIndex(services, i);
		serviceID   = SCNetworkServiceGetServiceID(service);
		serviceName = SCNetworkServiceGetName(service);
		if (serviceName == NULL) serviceName = CFSTR("");

		SCPrint(TRUE, stdout, CFSTR("%c%2ld: %@%-*s (%@)%s\n"),
			((net_service != NULL) && CFEqual(service, net_service)) ? '>' : ' ',
			i + 1,
			serviceName,
			(int)(30 - CFStringGetLength(serviceName)),
			" ",
			serviceID,
			SCNetworkServiceGetEnabled(service) ? "" : " *DISABLED*");

		__show_service_interface(service, "       Interface : ");
		__show_service_protocols(service, "       ", TRUE);
	}

	return;
}

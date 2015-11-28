/*
 * Copyright (c) 2004-2011, 2013, 2014 Apple Inc. All rights reserved.
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


#include <TargetConditionals.h>
#include "scutil.h"
#include "net.h"
#include "prefs.h"


#if	TARGET_OS_IPHONE
#define	INLINE_PASSWORDS_USE_CFSTRING
#endif	// TARGET_OS_IPHONE


#pragma mark -
#pragma mark Interface management


static CFArrayRef
_copy_interfaces()
{
	CFMutableArrayRef	interfaces;
	CFArrayRef		real_interfaces;

	real_interfaces = _SCNetworkInterfaceCopyAllWithPreferences(prefs);
	if (real_interfaces == NULL) {
		SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
		return NULL;
	}

	interfaces = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	// include real interfaces
	CFArrayAppendArray(interfaces,
			   real_interfaces,
			   CFRangeMake(0, CFArrayGetCount(real_interfaces)));
	CFRelease(real_interfaces);

	// include pseudo interfaces
	CFArrayAppendValue(interfaces, kSCNetworkInterfaceLoopback);
	CFArrayAppendValue(interfaces, kSCNetworkInterfaceIPv4);

	// include interfaces that we have created
	if (new_interfaces != NULL) {
		CFArrayAppendArray(interfaces,
				   new_interfaces,
				   CFRangeMake(0, CFArrayGetCount(new_interfaces)));
	}

	return (CFArrayRef)interfaces;
}


__private_extern__
SCNetworkInterfaceRef
_find_interface(int argc, char **argv, int *nArgs)
{
	Boolean			allowIndex	= TRUE;
	CFIndex			i;
	CFArrayRef		myInterfaces	= interfaces;
	CFIndex			n;
	CFStringRef		select_name	= NULL;
	SCNetworkInterfaceRef	selected	= NULL;

	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("no interface specified\n"));
		return NULL;
	}

	if (nArgs != NULL) *nArgs = 1;

	if (strcasecmp(argv[0], "$child") == 0) {
		if (net_interface == NULL) {
			SCPrint(TRUE, stdout, CFSTR("interface not selected\n"));
			goto done;
		}

		selected = SCNetworkInterfaceGetInterface(net_interface);
		if(selected == NULL) {
			SCPrint(TRUE, stdout, CFSTR("no child interface\n"));
		}

		goto done;
	} else if (strcasecmp(argv[0], "$service") == 0) {
		if (net_service == NULL) {
			SCPrint(TRUE, stdout, CFSTR("service not selected\n"));
			goto done;
		}

		selected = SCNetworkServiceGetInterface(net_service);
		if(selected == NULL) {
			SCPrint(TRUE, stdout, CFSTR("no interface for service\n"));
		}

		goto done;
	}

#if	!TARGET_OS_IPHONE
	else if (strcasecmp(argv[0], "$bond") == 0) {
		CFStringRef	interfaceType;

		if (net_interface == NULL) {
			SCPrint(TRUE, stdout, CFSTR("interface not selected\n"));
			goto done;
		}

		interfaceType = SCNetworkInterfaceGetInterfaceType(net_interface);
		if (!CFEqual(interfaceType, kSCNetworkInterfaceTypeBond)) {
			SCPrint(TRUE, stdout, CFSTR("interface not Bond\n"));
			goto done;
		}

		if (argc < 2) {
			SCPrint(TRUE, stdout, CFSTR("no member interface specified\n"));
			return NULL;
		}
		argv++;
		argc--;
		if (nArgs != NULL) *nArgs += 1;

		myInterfaces = SCBondInterfaceGetMemberInterfaces(net_interface);
		if (myInterfaces == NULL) {
			SCPrint(TRUE, stdout, CFSTR("no member interfaces\n"));
			goto done;
		}
		allowIndex = FALSE;
	}
#endif	// !TARGET_OS_IPHONE

	else if (strcasecmp(argv[0], "$bridge") == 0) {
		CFStringRef	interfaceType;

		if (net_interface == NULL) {
			SCPrint(TRUE, stdout, CFSTR("interface not selected\n"));
			goto done;
		}

		interfaceType = SCNetworkInterfaceGetInterfaceType(net_interface);
		if (!CFEqual(interfaceType, kSCNetworkInterfaceTypeBridge)) {
			SCPrint(TRUE, stdout, CFSTR("interface not Bridge\n"));
			goto done;
		}

		if (argc < 2) {
			SCPrint(TRUE, stdout, CFSTR("no member interface specified\n"));
			return NULL;
		}
		argv++;
		argc--;
		if (nArgs != NULL) *nArgs += 1;

		myInterfaces = SCBridgeInterfaceGetMemberInterfaces(net_interface);
		if (myInterfaces == NULL) {
			SCPrint(TRUE, stdout, CFSTR("no member interfaces\n"));
			goto done;
		}
		allowIndex = FALSE;
	}

	else if (strcasecmp(argv[0], "$vlan") == 0) {
		CFStringRef	interfaceType;

		if (net_interface == NULL) {
			SCPrint(TRUE, stdout, CFSTR("interface not selected\n"));
			goto done;
		}

		interfaceType = SCNetworkInterfaceGetInterfaceType(net_interface);
		if (!CFEqual(interfaceType, kSCNetworkInterfaceTypeVLAN)) {
			SCPrint(TRUE, stdout, CFSTR("interface not VLAN\n"));
			goto done;
		}

		selected = SCVLANInterfaceGetPhysicalInterface(net_interface);
		if(selected == NULL) {
			SCPrint(TRUE, stdout, CFSTR("no physical interface\n"));
		}

		goto done;
	}

	if ((myInterfaces == NULL) && (interfaces == NULL)) {
		interfaces = _copy_interfaces();
		if (interfaces == NULL) {
			return NULL;
		}
		myInterfaces = interfaces;
		allowIndex = FALSE;
	}

	// try to select the interface by its display name

	select_name = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);

	n = (myInterfaces != NULL) ? CFArrayGetCount(myInterfaces) : 0;
	for (i = 0; i < n; i++) {
		SCNetworkInterfaceRef	interface;
		CFStringRef		interfaceName;

		interface = CFArrayGetValueAtIndex(myInterfaces, i);
		interfaceName = SCNetworkInterfaceGetLocalizedDisplayName(interface);
		if ((interfaceName != NULL) && CFEqual(select_name, interfaceName)) {
			if (selected == NULL) {
				selected = interface;
			} else {
				// if multiple interfaces match
				selected = NULL;
				SCPrint(TRUE, stdout, CFSTR("multiple interfaces match\n"));
				goto done;
			}
		}
	}

	if (selected != NULL) {
		goto done;
	}

	// try to select the interface by its BSD name

	for (i = 0; i < n; i++) {
		SCNetworkInterfaceRef	interface;
		CFStringRef		bsd_name	= NULL;

		interface = CFArrayGetValueAtIndex(myInterfaces, i);
		while ((interface != NULL) && (bsd_name == NULL)) {
			bsd_name = SCNetworkInterfaceGetBSDName(interface);
			if (bsd_name == NULL) {
				interface = SCNetworkInterfaceGetInterface(interface);
			}
		}

		if ((bsd_name != NULL) && CFEqual(select_name, bsd_name)) {
			if (selected == NULL) {
				selected = interface;
			} else {
				// if multiple interfaces match
				selected = NULL;
				SCPrint(TRUE, stdout, CFSTR("multiple interfaces match\n"));
				goto done;
			}
		}
	}

	if (selected != NULL) {
		goto done;
	}

	// try to select the interface by its interface type

	for (i = 0; i < n; i++) {
		SCNetworkInterfaceRef	interface;
		CFStringRef		interfaceType;

		interface = CFArrayGetValueAtIndex(myInterfaces, i);
		interfaceType = SCNetworkInterfaceGetInterfaceType(interface);
		if (CFEqual(select_name, interfaceType)) {
			if (selected == NULL) {
				selected = interface;
			} else {
				// if multiple interfaces match
				selected = NULL;
				SCPrint(TRUE, stdout, CFSTR("multiple interfaces match\n"));
				goto done;
			}
		}
	}

	if (selected != NULL) {
		goto done;
	}

	if (allowIndex) {
		char	*end;
		char	*str	= argv[0];
		long	val;

		// try to select the interface by its index

		errno = 0;
		val = strtol(str, &end, 10);
		if ((*str != '\0') &&
		    ((*end == '\0') || (*end == '.')) &&
		    (errno == 0)) {
			if ((val > 0) && (val <= n)) {
				selected = CFArrayGetValueAtIndex(myInterfaces, val - 1);

				if (*end == '.') {
					str = end + 1;
					val = strtol(str, &end, 10);
					if ((*str != '\0') && (*end == '\0') && (errno == 0)) {
						while (val-- > 0) {
							selected = SCNetworkInterfaceGetInterface(selected);
							if (selected == NULL) {
								break;
							}
						}
					}
				}
			}
		}
	}

	if (selected != NULL) {
		goto done;
	}

	SCPrint(TRUE, stdout, CFSTR("no match\n"));

    done :

	if (select_name != NULL) CFRelease(select_name);
	return selected;
}


/* -------------------- */


__private_extern__
void
create_interface(int argc, char **argv)
{
	SCNetworkInterfaceRef	interface;
	CFStringRef		interfaceName;
	CFStringRef		interfaceType;
	SCNetworkInterfaceRef	new_interface;

	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("what interface type?\n"));
		return;
	}

	interfaceType = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	argv++;
	argc--;

	if (CFEqual(interfaceType, kSCNetworkInterfaceTypeBond)) {
		SCPrint(TRUE, stdout, CFSTR("bond creation not yet supported\n"));
		goto done;
	}
	if (CFEqual(interfaceType, kSCNetworkInterfaceTypeBridge)) {
		SCPrint(TRUE, stdout, CFSTR("bridge creation not yet supported\n"));
		goto done;
	}
	if (CFEqual(interfaceType, kSCNetworkInterfaceTypeVLAN)) {
		SCPrint(TRUE, stdout, CFSTR("vlan creation not yet supported\n"));
		goto done;
	}

	if (argc < 1) {
		if (net_interface == NULL) {
			SCPrint(TRUE, stdout, CFSTR("no network interface selected\n"));
			goto done;
		}

		interface = net_interface;
	} else {
		interface = _find_interface(argc, argv, NULL);
	}

	if (interface == NULL) {
		goto done;
	}

	new_interface = SCNetworkInterfaceCreateWithInterface(interface, interfaceType);
	if (new_interface == NULL) {
		SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
		goto done;
	}

	if (new_interfaces == NULL) {
		new_interfaces = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}
	CFArrayAppendValue(new_interfaces, new_interface);

	if (net_interface != NULL) CFRelease(net_interface);
	net_interface = new_interface;

	interfaceName = SCNetworkInterfaceGetLocalizedDisplayName(net_interface);
	if (interfaceName == NULL) {
		interfaceName = SCNetworkInterfaceGetBSDName(net_interface);
	}
	if (interfaceName == NULL) {
		interfaceName = SCNetworkInterfaceGetInterfaceType(net_interface);
	}
	SCPrint(TRUE, stdout, CFSTR("interface \"%@\" created and selected\n"), interfaceName);

    done :

	CFRelease(interfaceType);
	return;
}


/* -------------------- */


__private_extern__
void
select_interface(int argc, char **argv)
{
	SCNetworkInterfaceRef	interface;

	interface = _find_interface(argc, argv, NULL);
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

		SCPrint(TRUE, stdout, CFSTR("interface \"%@\" selected\n"), interfaceName);
	}

	return;
}


/* -------------------- */


__private_extern__
void
_show_interface(SCNetworkInterfaceRef interface, CFStringRef prefix, Boolean showChild)
{
	CFDictionaryRef configuration;
	CFStringRef	if_bsd_name;
	CFStringRef	if_localized_name;
	CFStringRef	if_mac_address;
	CFStringRef	if_type;
	Boolean		isPhysicalEthernet;
	CFArrayRef	supported;

	if_localized_name = SCNetworkInterfaceGetLocalizedDisplayName(interface);
	if (if_localized_name != NULL) {
		SCPrint(TRUE, stdout, CFSTR("%@  name                 = %@\n"), prefix, if_localized_name);
	}

	if_bsd_name = SCNetworkInterfaceGetBSDName(interface);
	if (if_bsd_name != NULL) {
		SCPrint(TRUE, stdout, CFSTR("%@  interface name       = %@\n"), prefix, if_bsd_name);
	}

	if_type = SCNetworkInterfaceGetInterfaceType(interface);
	SCPrint(TRUE, stdout, CFSTR("%@  type                 = %@\n"), prefix, if_type);

	if_mac_address = SCNetworkInterfaceGetHardwareAddressString(interface);
	if (if_mac_address != NULL) {
		SCPrint(TRUE, stdout, CFSTR("%@  address              = %@\n"), prefix, if_mac_address);
	}

	configuration = SCNetworkInterfaceGetConfiguration(interface);
	if ((configuration != NULL) &&
	    CFDictionaryContainsKey(configuration, kSCResvInactive)) {
		configuration = NULL;
	}

	if (if_bsd_name != NULL) {
		CFArrayRef	available;
		CFDictionaryRef	active;
		CFDictionaryRef	cap_current;
		int		mtu_cur;
		int		mtu_min;
		int		mtu_max;

		cap_current = SCNetworkInterfaceCopyCapability(interface, NULL);
		if (cap_current != NULL) {
			CFIndex			i;
			CFArrayRef		cap_names;
			CFMutableArrayRef	cap_sorted;
			const void		**keys;
			CFIndex			n;

			n = CFDictionaryGetCount(cap_current);
			keys = CFAllocatorAllocate(NULL, n * sizeof(CFStringRef), 0);
			CFDictionaryGetKeysAndValues(cap_current, keys, NULL);
			cap_names = CFArrayCreate(NULL, keys, n, &kCFTypeArrayCallBacks);
			CFAllocatorDeallocate(NULL, keys);

			cap_sorted = CFArrayCreateMutableCopy(NULL, 0, cap_names);
			CFRelease(cap_names);

			CFArraySortValues(cap_sorted, CFRangeMake(0, n), (CFComparatorFunction)CFStringCompare, NULL);

			SCPrint(TRUE, stdout, CFSTR("%@  capabilities         = "), prefix);
			for (i = 0; i < n; i++) {
				CFStringRef	cap_name;
				int		cap_val;
				CFNumberRef	val	= NULL;

				cap_name = CFArrayGetValueAtIndex(cap_sorted, i);
				if (configuration != NULL) {
					val = CFDictionaryGetValue(configuration, cap_name);
				}
				if (!isA_CFNumber(val)) {
					val = CFDictionaryGetValue(cap_current, cap_name);
				}

				SCPrint(TRUE, stdout, CFSTR("%s%@%c"),
					(i == 0) ? "" : ",",
					cap_name,
					(CFNumberGetValue(val, kCFNumberIntType, &cap_val) &&
					 (cap_val != 0)) ? '+' : '-');
			}
			SCPrint(TRUE, stdout, CFSTR("\n"));

			CFRelease(cap_sorted);
			CFRelease(cap_current);
		}

		if (SCNetworkInterfaceCopyMTU(interface, &mtu_cur, &mtu_min, &mtu_max)) {
			char	isCurrent	= '*';

			if (configuration != NULL) {
				int		mtu_req;
				CFNumberRef	num;

				num = CFDictionaryGetValue(configuration, kSCPropNetEthernetMTU);
				if (isA_CFNumber(num)) {
					CFNumberGetValue(num, kCFNumberIntType, &mtu_req);
					if (mtu_cur != mtu_req) {
						mtu_cur = mtu_req;
						isCurrent = ' ';
					}
				}
			}

			SCPrint(TRUE, stdout, CFSTR("%@  mtu                %c = %d (%d < n < %d)\n"),
				prefix,
				isCurrent,
				mtu_cur,
				mtu_min,
				mtu_max);
		}

		if (SCNetworkInterfaceCopyMediaOptions(interface, NULL, &active, &available, TRUE)) {
			char		isCurrent	= ' ';
			CFArrayRef	options		= NULL;
			CFArrayRef	options_req	= NULL;
			CFStringRef	subtype		= NULL;
			CFStringRef	subtype_req	= NULL;

			if (configuration != NULL) {
				subtype_req = CFDictionaryGetValue(configuration, kSCPropNetEthernetMediaSubType);
				options_req = CFDictionaryGetValue(configuration, kSCPropNetEthernetMediaOptions);
			}

			if (subtype_req == NULL) {
				subtype_req = CFSTR("autoselect");
			}

			if (active != NULL) {
				subtype = CFDictionaryGetValue(active, kSCPropNetEthernetMediaSubType);
				options = CFDictionaryGetValue(active, kSCPropNetEthernetMediaOptions);
			}

			if (subtype != NULL) {
				if (((subtype_req != NULL) &&
				     CFEqual(subtype, subtype_req)) &&
				    ((options == options_req) ||
				     ((options != NULL) &&
				      (options_req != NULL) &&
				      CFEqual(options, options_req)))
				   ) {
					isCurrent = '*';
				} else if ((subtype_req == NULL) ||
					   ((subtype_req != NULL) &&
					    CFEqual(subtype_req, CFSTR("autoselect")))) {
					// if requested subtype not specified or "autoselect"
					isCurrent = '*';
				}
			}

			if (subtype_req != NULL) {
				SCPrint(TRUE, stdout, CFSTR("%@  media              %c = %@"),
					prefix,
					isCurrent,
					subtype_req);

				if ((options_req != NULL) &&
				    (CFArrayGetCount(options_req) > 0)) {
					CFStringRef	options_str;

					options_str = CFStringCreateByCombiningStrings(NULL, options_req, CFSTR(","));
					SCPrint(TRUE, stdout, CFSTR(" <%@>"), options_str);
					CFRelease(options_str);
				}

				SCPrint(TRUE, stdout, CFSTR("\n"));
			}

			SCPrint(TRUE, stdout, CFSTR("\n"));

			if (available != NULL) {
				CFIndex		i;
				CFIndex		n_subtypes;
				CFArrayRef	subtypes;

				subtypes   = SCNetworkInterfaceCopyMediaSubTypes(available);
				n_subtypes = (subtypes != NULL) ? CFArrayGetCount(subtypes) : 0;
				for (i = 0; i < n_subtypes; i++) {
					CFIndex		j;
					CFIndex		n_subtype_options;
					CFStringRef	subtype;
					CFArrayRef	subtype_options;

					subtype = CFArrayGetValueAtIndex(subtypes, i);
					subtype_options = SCNetworkInterfaceCopyMediaSubTypeOptions(available, subtype);
					n_subtype_options = (subtype_options != NULL) ? CFArrayGetCount(subtype_options) : 0;
					for (j = 0; j < n_subtype_options; j++) {
						char		isCurrent	= ' ';
						CFArrayRef	options;

						options = CFArrayGetValueAtIndex(subtype_options, j);

						if (((subtype_req != NULL) &&
						     CFEqual(subtype, subtype_req)) &&
						    ((options == options_req) ||
						     ((options != NULL) &&
						      (options_req != NULL) &&
						      CFEqual(options, options_req)))
						   ) {
							isCurrent = '*';
						}

						SCPrint(TRUE, stdout, CFSTR("%@  %s    %c = %@"),
							prefix,
							((i == 0) && (j == 0)) ? "supported media" : "               ",
							isCurrent,
							subtype);

						if ((options != NULL) &&
						    (CFArrayGetCount(options) > 0)) {
							CFStringRef	options_str;

							options_str = CFStringCreateByCombiningStrings(NULL, options, CFSTR(","));
							SCPrint(TRUE, stdout, CFSTR(" <%@>"), options_str);
							CFRelease(options_str);
						}

						SCPrint(TRUE, stdout, CFSTR("\n"));
					}
					if (subtype_options != NULL) CFRelease(subtype_options);
				}
				if (subtypes != NULL) CFRelease(subtypes);
			}
		} else {
			SCPrint(TRUE, stdout, CFSTR("\n"));
		}
	}

	supported = SCNetworkInterfaceGetSupportedInterfaceTypes(interface);
	SCPrint(TRUE, stdout, CFSTR("%@  supported interfaces = "), prefix);
	if (supported != NULL) {
		CFIndex i;
		CFIndex n	= CFArrayGetCount(supported);

		for (i = 0; i < n; i++) {
			SCPrint(TRUE, stdout, CFSTR("%s%@"),
				(i == 0) ? "" : ", ",
				CFArrayGetValueAtIndex(supported, i));
		}
	}
	SCPrint(TRUE, stdout, CFSTR("\n"));

	supported = SCNetworkInterfaceGetSupportedProtocolTypes(interface);
	SCPrint(TRUE, stdout, CFSTR("%@  supported protocols  = "), prefix);
	if (supported != NULL) {
		CFIndex i;
		CFIndex n	= CFArrayGetCount(supported);

		for (i = 0; i < n; i++) {
			SCPrint(TRUE, stdout, CFSTR("%s%@"),
				(i == 0) ? "" : ", ",
				CFArrayGetValueAtIndex(supported, i));
		}
	}
	SCPrint(TRUE, stdout, CFSTR("\n"));

	isPhysicalEthernet = _SCNetworkInterfaceIsPhysicalEthernet(interface);
	SCPrint(TRUE, stdout, CFSTR("%@  is physical ethernet = %s \n"),
		prefix, (isPhysicalEthernet == TRUE) ? "YES" : "NO");

	if (configuration != NULL) {
		CFMutableDictionaryRef	effective;

		effective = CFDictionaryCreateMutableCopy(NULL, 0, configuration);

		// remove known (and already reported) interface configuration keys
		if (CFDictionaryContainsKey(effective, kSCResvInactive)) {
			CFDictionaryRemoveAllValues(effective);
		}
		CFDictionaryRemoveValue(effective, kSCPropNetEthernetMTU);
		CFDictionaryRemoveValue(effective, kSCPropNetEthernetMediaSubType);
		CFDictionaryRemoveValue(effective, kSCPropNetEthernetMediaOptions);

		if (CFDictionaryGetCount(effective) > 0) {
			SCPrint(TRUE, stdout, CFSTR("\n%@  per-interface configuration\n"), prefix);
			_show_entity(effective, prefix);
		}

		CFRelease(effective);
	}

	if (CFEqual(if_type, kSCNetworkInterfaceTypePPP)) {
		SCNetworkInterfaceRef	childInterface;

		childInterface = SCNetworkInterfaceGetInterface(interface);
		if (childInterface != NULL) {
			CFStringRef	childInterfaceType;

			childInterfaceType = SCNetworkInterfaceGetInterfaceType(childInterface);
			if (CFEqual(childInterfaceType, kSCNetworkInterfaceTypeL2TP)) {
				CFDictionaryRef		ipsec_configuration;

				ipsec_configuration = SCNetworkInterfaceGetExtendedConfiguration(interface, kSCEntNetIPSec);
				if (isA_CFDictionary(ipsec_configuration) &&
				    (CFDictionaryGetCount(ipsec_configuration) > 0)) {
					SCPrint(TRUE, stdout, CFSTR("\n%@  per-interface IPSec configuration\n"), prefix);
					_show_entity(ipsec_configuration, prefix);
				}
			}
		}
	}

	if (_sc_debug) {
		SCPrint(TRUE, stdout, CFSTR("\n%@\n"), interface);
	}

	interface = SCNetworkInterfaceGetInterface(interface);
	if (interface != NULL) {
		CFStringRef	newPrefix;

		newPrefix = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@  "), prefix);
		SCPrint(TRUE, stdout, CFSTR("\n%@child interface\n"), newPrefix);
		_show_interface(interface, newPrefix, showChild);
		CFRelease(newPrefix);
	}

	return;
}


/* -------------------- */


static Boolean
validateMediaOptions(SCNetworkInterfaceRef interface, CFMutableDictionaryRef newConfiguration)
{
	Boolean		ok	= TRUE;
	CFNumberRef	mtu;
	CFArrayRef	options;
	CFStringRef	subtype;

	mtu = CFDictionaryGetValue(newConfiguration, kSCPropNetEthernetMTU);
	if (isA_CFNumber(mtu)) {
		int	mtu_max;
		int	mtu_min;
		int	mtu_val;

		if (!SCNetworkInterfaceCopyMTU(interface, NULL, &mtu_min, &mtu_max)) {
			SCPrint(TRUE, stdout, CFSTR("cannot set MTU\n"));
			return FALSE;
		}

		if (!CFNumberGetValue(mtu, kCFNumberIntType, &mtu_val) ||
		    (mtu_val < mtu_min) ||
		    (mtu_val > mtu_max)) {
			SCPrint(TRUE, stdout, CFSTR("mtu out of range\n"));
			return FALSE;
		}
	}

	subtype = CFDictionaryGetValue(newConfiguration, kSCPropNetEthernetMediaSubType);
	options = CFDictionaryGetValue(newConfiguration, kSCPropNetEthernetMediaOptions);

	if (subtype != NULL) {
		CFArrayRef	available	= NULL;
		CFArrayRef	config_options	= options;
		CFArrayRef	subtypes	= NULL;
		CFArrayRef	subtype_options	= NULL;

		ok = FALSE;

		if (options == NULL) {
			config_options = CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
		}

		if (!SCNetworkInterfaceCopyMediaOptions(interface, NULL, NULL, &available, FALSE)) {
			SCPrint(TRUE, stdout, CFSTR("media type / options not available\n"));
			goto checked;
		}

		if (available == NULL) {
			goto checked;
		}

		subtypes = SCNetworkInterfaceCopyMediaSubTypes(available);
		if ((subtypes == NULL) ||
		    !CFArrayContainsValue(subtypes,
					 CFRangeMake(0, CFArrayGetCount(subtypes)),
					 subtype)) {
			SCPrint(TRUE, stdout, CFSTR("media type not valid\n"));
			goto checked;
		}

		subtype_options = SCNetworkInterfaceCopyMediaSubTypeOptions(available, subtype);
		if ((subtype_options == NULL) ||
		    !CFArrayContainsValue(subtype_options,
					  CFRangeMake(0, CFArrayGetCount(subtype_options)),
					  config_options)) {
			SCPrint(TRUE, stdout, CFSTR("media options not valid for \"%@\"\n"), subtype);
			goto checked;
		}

		if (options == NULL) {
			CFDictionarySetValue(newConfiguration, kSCPropNetEthernetMediaOptions, config_options);
		}

		ok = TRUE;

	    checked :

		if (available       != NULL)	CFRelease(available);
		if (subtypes        != NULL)	CFRelease(subtypes);
		if (subtype_options != NULL)	CFRelease(subtype_options);
		if (options         == NULL)	CFRelease(config_options);
	} else {
		if (options != NULL) {
			SCPrint(TRUE, stdout, CFSTR("media type and options must both be specified\n"));
			return FALSE;
		}
	}

	return ok;
}


/* -------------------- */


__private_extern__
void
show_interfaces(int argc, char **argv)
{
	CFIndex		i;
	CFIndex		n;

	if (interfaces != NULL) CFRelease(interfaces);
	interfaces = _copy_interfaces();
	if (interfaces == NULL) {
		return;
	}

	n = CFArrayGetCount(interfaces);
	for (i = 0; i < n; i++) {
		CFIndex			childIndex	= 0;
		SCNetworkInterfaceRef	interface;

		interface = CFArrayGetValueAtIndex(interfaces, i);
		do {
			CFStringRef	interfaceName;
			char		isSelected;

			interfaceName = SCNetworkInterfaceGetLocalizedDisplayName(interface);
			if (interfaceName == NULL) {
				interfaceName = SCNetworkInterfaceGetBSDName(interface);
			}
			if (interfaceName == NULL) {
				interfaceName = SCNetworkInterfaceGetInterfaceType(interface);
			}

			isSelected = ' ';
			if ((net_interface != NULL) && CFEqual(interface, net_interface)) {
				isSelected = '>';
			}

			if (childIndex == 0) {
				SCPrint(TRUE, stdout, CFSTR("%c%2ld: %@\n"),
					isSelected,
					i + 1,
					interfaceName);
			} else {
				SCPrint(TRUE, stdout, CFSTR("%c%2ld.%ld: %@\n"),
					isSelected,
					i + 1,
					childIndex,
					interfaceName);
			}

			if (_sc_debug) {
				CFMutableStringRef	desc;
				CFMutableDictionaryRef	formatOptions;

				desc = CFStringCreateMutable(NULL, 0);

				formatOptions = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
				CFDictionarySetValue(formatOptions, CFSTR("PREFIX1"), CFSTR(""));
				CFDictionarySetValue(formatOptions, CFSTR("PREFIX2"), CFSTR("$$"));
				CFStringAppendFormat(desc, formatOptions, CFSTR("%@"), interface);
				CFRelease(formatOptions);

				// cleanup SCNetworkInterface details
				CFStringFindAndReplace(desc,
						       CFSTR("]> {"),
						       CFSTR("]>\n       {\n         "),
						       CFRangeMake(0, CFStringGetLength(desc)),
						       0);
				CFStringFindAndReplace(desc,
						       CFSTR(", "),
						       CFSTR("\n         "),
						       CFRangeMake(0, CFStringGetLength(desc)),
						       0);
				CFStringFindAndReplace(desc,
						       CFSTR("}"),
						       CFSTR("\n       }"),
						       CFRangeMake(CFStringGetLength(desc) - 1, 1),
						       kCFCompareBackwards|kCFCompareAnchored);

				// additional cleanup for Bond, Bridge, VLAN options
				CFStringFindAndReplace(desc,
						       CFSTR("> {\n"),
						       CFSTR(">\n           {\n"),
						       CFRangeMake(0, CFStringGetLength(desc)),
						       0);
				CFStringFindAndReplace(desc,
						       CFSTR("\n$$"),
						       CFSTR("\n           "),
						       CFRangeMake(0, CFStringGetLength(desc)),
						       0);
				CFStringFindAndReplace(desc,
						       CFSTR("$$"),
						       CFSTR(""),
						       CFRangeMake(0, CFStringGetLength(desc)),
						       0);

				SCPrint(TRUE, stdout, CFSTR("\n     %@\n\n"), desc);
				CFRelease(desc);
			}

			interface = SCNetworkInterfaceGetInterface(interface);
			childIndex++;
		} while (interface != NULL);
	}

	return;
}


/* -------------------- */


static int
__doRank(CFStringRef key, const char *description, void *info, int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	SCNetworkInterfaceRef		interface;
	CFStringRef			interfaceName;
	Boolean				ok	= FALSE;
	SCNetworkServicePrimaryRank	rank	= kSCNetworkServicePrimaryRankDefault;
	SCDynamicStoreRef		store;

	if (argc < 1) {
		SCPrint(TRUE, stdout,
			CFSTR("%s not specified\n"),
			description != NULL ? description : "rank");
		return -1;
	}

	if (strlen(argv[0]) == 0) {
		rank = kSCNetworkServicePrimaryRankDefault;
	} else if ((strcasecmp(argv[0], "First") == 0)) {
		rank = kSCNetworkServicePrimaryRankFirst;
	} else if ((strcasecmp(argv[0], "Last") == 0)) {
		rank = kSCNetworkServicePrimaryRankLast;
	} else if ((strcasecmp(argv[0], "Never") == 0)) {
		rank = kSCNetworkServicePrimaryRankNever;
	} else if ((strcasecmp(argv[0], "Scoped") == 0)) {
		rank = kSCNetworkServicePrimaryRankScoped;
	} else {
		SCPrint(TRUE, stdout, CFSTR("invalid rank\n"));
		return -1;
	}

	interfaceName = SCNetworkInterfaceGetBSDName(net_interface);
	if (interfaceName == NULL) {
		SCPrint(TRUE, stdout, CFSTR("no BSD interface\n"));
		return FALSE;
	}

	store = SCDynamicStoreCreate(NULL, CFSTR("scutil --net"), NULL, NULL);
	interface = _SCNetworkInterfaceCopyActive(store, interfaceName);
	CFRelease(store);
	if (interface == NULL) {
		SCPrint(TRUE, stdout, CFSTR("No active interface\n"));
		return -1;
	}

	ok = SCNetworkInterfaceSetPrimaryRank(interface, rank);
	CFRelease(interface);
	if (!ok) {
		SCPrint(TRUE, stdout, CFSTR("could not update per-interface rank\n"));
		return -1;
	}

	return 1;
}


/* -------------------- */


static void
_replaceOne(const void *key, const void *value, void *context)
{
	CFMutableDictionaryRef	newConfiguration	= (CFMutableDictionaryRef)context;

	CFDictionarySetValue(newConfiguration, key, value);
	return;
}


static void
updateInterfaceConfiguration(CFMutableDictionaryRef newConfiguration)
{
	CFDictionaryRef	configuration;

	CFDictionaryRemoveAllValues(newConfiguration);

	configuration = SCNetworkInterfaceGetConfiguration(net_interface);
	if (configuration != NULL) {
		CFDictionaryApplyFunction(configuration, _replaceOne, (void *)newConfiguration);
	}

	return;
}


#pragma mark -
#pragma mark Bond options


static options bondOptions[] = {
	{ "mtu"       , NULL, isNumber     , &kSCPropNetEthernetMTU         , NULL, NULL },
	// xxx  { "+device"   , ... },
	// xxx  { "-device"   , ... },

	{ "?"         , NULL , isHelp     , NULL                            , NULL,
		"\nBond configuration commands\n\n"
		" set interface [mtu n] [media type] [mediaopts opts]\n"
	}
};
#define	N_BOND_OPTIONS	(sizeof(bondOptions) / sizeof(bondOptions[0]))


static Boolean
set_interface_bond(int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	CFStringRef	interfaceName;
	Boolean		ok;

	interfaceName = SCNetworkInterfaceGetBSDName(net_interface);
	if (interfaceName == NULL) {
		SCPrint(TRUE, stdout, CFSTR("no BSD interface\n"));
		return FALSE;
	}

	ok = _process_options(bondOptions, N_BOND_OPTIONS, argc, argv, newConfiguration);
	if (ok) {
		// validate configuration
		if (!validateMediaOptions(net_interface, newConfiguration)) {
			return FALSE;
		}
	}

	return ok;
}


#pragma mark -
#pragma mark Bridge options


static options bridgeOptions[] = {
	{ "mtu"       , NULL, isNumber     , &kSCPropNetEthernetMTU         , NULL, NULL },
// xxx  { "+device"   , ... },
// xxx  { "-device"   , ... },

	{ "?"         , NULL , isHelp     , NULL                            , NULL,
	    "\nBridge configuration commands\n\n"
	    " set interface [mtu n] [media type] [mediaopts opts]\n"
	}
};
#define	N_BRIDGE_OPTIONS	(sizeof(bridgeOptions) / sizeof(bridgeOptions[0]))


static Boolean
set_interface_bridge(int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	CFStringRef	interfaceName;
	Boolean		ok;

	interfaceName = SCNetworkInterfaceGetBSDName(net_interface);
	if (interfaceName == NULL) {
		SCPrint(TRUE, stdout, CFSTR("no BSD interface\n"));
		return FALSE;
	}

	ok = _process_options(bridgeOptions, N_BRIDGE_OPTIONS, argc, argv, newConfiguration);
	if (ok) {
		// validate configuration
		if (!validateMediaOptions(net_interface, newConfiguration)) {
			return FALSE;
		}
	}

	return ok;
}


#pragma mark -
#pragma mark AirPort options


static options airportOptions[] = {
	{ "mtu"       , NULL, isNumber     , &kSCPropNetEthernetMTU         , NULL, NULL },
	{ "media"     , NULL, isString     , &kSCPropNetEthernetMediaSubType, NULL, NULL },
	{ "mediaopt"  , NULL, isStringArray, &kSCPropNetEthernetMediaOptions, NULL, NULL },

	{ "rank"      , NULL, isOther      , NULL                            , __doRank, NULL },

	{ "?"         , NULL, isHelp       , NULL                            , NULL,
	    "\nAirPort configuration commands\n\n"
	    " set interface [mtu n] [media type] [mediaopts opts]\n"
	}
};
#define	N_AIRPORT_OPTIONS	(sizeof(airportOptions) / sizeof(airportOptions[0]))


static Boolean
set_interface_airport(int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	CFStringRef	interfaceName;
	Boolean		ok;

	interfaceName = SCNetworkInterfaceGetBSDName(net_interface);
	if (interfaceName == NULL) {
		SCPrint(TRUE, stdout, CFSTR("no BSD interface\n"));
		return FALSE;
	}

	ok = _process_options(airportOptions, N_AIRPORT_OPTIONS, argc, argv, newConfiguration);
	if (ok) {
		// validate configuration
		if (!validateMediaOptions(net_interface, newConfiguration)) {
			return FALSE;
		}
	}

	return ok;
}


#pragma mark -
#pragma mark Ethernet options


static int
__doCapability(CFStringRef key, const char *description, void *info, int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	Boolean	ok	= FALSE;

	if (argc < 1) {
		SCPrint(TRUE, stdout,
			CFSTR("%s not specified\n"),
			description != NULL ? description : "enable/disable");
		return -1;
	}

	if (strlen(argv[0]) == 0) {
		ok = SCNetworkInterfaceSetCapability(net_interface, key, NULL);
	} else if ((strcasecmp(argv[0], "disable") == 0) ||
		   (strcasecmp(argv[0], "no"     ) == 0) ||
		   (strcasecmp(argv[0], "off"    ) == 0) ||
		   (strcasecmp(argv[0], "0"      ) == 0)) {
		ok = SCNetworkInterfaceSetCapability(net_interface, key, CFNumberRef_0);
	} else if ((strcasecmp(argv[0], "enable") == 0) ||
		   (strcasecmp(argv[0], "yes"   ) == 0) ||
		   (strcasecmp(argv[0], "on"    ) == 0) ||
		   (strcasecmp(argv[0], "1"     ) == 0)) {
		ok = SCNetworkInterfaceSetCapability(net_interface, key, CFNumberRef_1);
	} else {
		SCPrint(TRUE, stdout, CFSTR("invalid value\n"));
		return -1;
	}

	if (ok) {
		updateInterfaceConfiguration(newConfiguration);
	} else {
		SCPrint(TRUE, stdout,
			CFSTR("%@ not updated: %s\n"),
			key,
			SCErrorString(SCError()));
		return -1;
	}

	return 1;
}


static options ethernetOptions[] = {
	{ "mtu"       , NULL, isNumber     , &kSCPropNetEthernetMTU         , NULL, NULL },
	{ "media"     , NULL, isString     , &kSCPropNetEthernetMediaSubType, NULL, NULL },
	{ "mediaopt"  , NULL, isStringArray, &kSCPropNetEthernetMediaOptions, NULL, NULL },

	{ "av"        , NULL, isOther      , &kSCPropNetEthernetCapabilityAV    , __doCapability, NULL },
	{ "lro"       , NULL, isOther      , &kSCPropNetEthernetCapabilityLRO   , __doCapability, NULL },
	{ "rxcsum"    , NULL, isOther      , &kSCPropNetEthernetCapabilityRXCSUM, __doCapability, NULL },
	{ "tso"       , NULL, isOther      , &kSCPropNetEthernetCapabilityTSO   , __doCapability, NULL },
	{ "txcsum"    , NULL, isOther      , &kSCPropNetEthernetCapabilityTXCSUM, __doCapability, NULL },

	{ "rank"      , NULL, isOther      , NULL                            , __doRank, NULL },

	{ "?"         , NULL, isHelp       , NULL                            , NULL,
	    "\nEthernet configuration commands\n\n"
	    " set interface [mtu n] [media type] [mediaopts opts]\n"
	}
};
#define	N_ETHERNET_OPTIONS	(sizeof(ethernetOptions) / sizeof(ethernetOptions[0]))


static Boolean
set_interface_ethernet(int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	CFStringRef	interfaceName;
	Boolean		ok;

	interfaceName = SCNetworkInterfaceGetBSDName(net_interface);
	if (interfaceName == NULL) {
		SCPrint(TRUE, stdout, CFSTR("no BSD interface\n"));
		return FALSE;
	}

	ok = _process_options(ethernetOptions, N_ETHERNET_OPTIONS, argc, argv, newConfiguration);
	if (ok) {
		// validate configuration
		if (!validateMediaOptions(net_interface, newConfiguration)) {
			return FALSE;
		}
	}

	return ok;
}


#pragma mark -
#pragma mark IPSec options


static int
__doIPSecSharedSecret(CFStringRef key, const char *description, void *info, int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	CFStringRef	encryptionType;

	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("IPSec shared secret not specified\n"));
		return -1;
	}

	encryptionType = CFDictionaryGetValue(newConfiguration, kSCPropNetIPSecSharedSecretEncryption);
	if (strlen(argv[0]) > 0) {
		if (encryptionType == NULL) {
#ifdef	INLINE_PASSWORDS_USE_CFSTRING
			CFStringRef		pw;

			pw = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
#else	// INLINE_PASSWORDS_USE_CFSTRING
			CFIndex			n;
			CFMutableDataRef	pw;
			CFStringRef		str;

			str = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
			n = CFStringGetLength(str);
			pw = CFDataCreateMutable(NULL, n * sizeof(UniChar));
			CFDataSetLength(pw, n * sizeof(UniChar));
			/* ALIGN: CF aligns to at least >8 bytes */
			CFStringGetCharacters(str,
					      CFRangeMake(0, n),
					      (UniChar *)(void *)CFDataGetMutableBytePtr(pw));
			CFRelease(str);
#endif	// INLINE_PASSWORDS_USE_CFSTRING

			CFDictionarySetValue(newConfiguration, key, pw);
			CFRelease(pw);
		} else if (CFEqual(encryptionType, kSCValNetIPSecSharedSecretEncryptionKeychain)) {
			Boolean		ok;
			CFDataRef	pw;
			CFStringRef	str;

			str = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
			pw = CFStringCreateExternalRepresentation(NULL, str, kCFStringEncodingUTF8, 0);
			ok = SCNetworkInterfaceSetPassword(net_interface,
							   kSCNetworkInterfacePasswordTypeIPSecSharedSecret,
							   pw,
							   NULL);
			CFRelease(pw);
			CFRelease(str);
			if (ok) {
				updateInterfaceConfiguration(newConfiguration);
			} else {
				return -1;
			}
		} else {
			SCPrint(TRUE, stdout, CFSTR("IPSec shared secret type \"%@\" not supported\n"), encryptionType);
			return -1;
		}
	} else {
		if (encryptionType == NULL) {
			CFDictionaryRemoveValue(newConfiguration, key);
		} else if (CFEqual(encryptionType, kSCValNetIPSecSharedSecretEncryptionKeychain)) {
			Boolean		ok;
			ok = SCNetworkInterfaceRemovePassword(net_interface, kSCNetworkInterfacePasswordTypeIPSecSharedSecret);
			if (ok) {
				updateInterfaceConfiguration(newConfiguration);
			} else {
				return -1;
			}
		} else {
			SCPrint(TRUE, stdout, CFSTR("IPSec shared secret type \"%@\" not supported\n"), encryptionType);
			return -1;
		}
	}

	return 1;
}


static int
__doIPSecSharedSecretType(CFStringRef key, const char *description, void *info, int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("IPSec shared secret type mode not specified\n"));
		return -1;
	}

	if (strlen(argv[0]) > 0) {
		if (strcasecmp(argv[0], "keychain") == 0) {
			CFDictionarySetValue(newConfiguration, key, kSCValNetIPSecSharedSecretEncryptionKeychain);
		} else {
			SCPrint(TRUE, stdout, CFSTR("invalid shared secret type\n"));
			return -1;
		}
	} else {
		CFDictionaryRemoveValue(newConfiguration, key);
	}

	// encryption type changed, reset shared secret
	CFDictionaryRemoveValue(newConfiguration, kSCPropNetIPSecSharedSecret);

	return 1;
}


static int
__doIPSecXAuthPassword(CFStringRef key, const char *description, void *info, int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	CFStringRef	encryptionType;

	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("IPSec XAuth password not specified\n"));
		return -1;
	}

	encryptionType = CFDictionaryGetValue(newConfiguration, kSCPropNetIPSecXAuthPasswordEncryption);
	if (strlen(argv[0]) > 0) {
		if (encryptionType == NULL) {
#ifdef	INLINE_PASSWORDS_USE_CFSTRING
			CFStringRef		pw;

			pw = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
#else	// INLINE_PASSWORDS_USE_CFSTRING
			CFIndex			n;
			CFMutableDataRef	pw;
			CFStringRef		str;

			str = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
			n = CFStringGetLength(str);
			pw = CFDataCreateMutable(NULL, n * sizeof(UniChar));
			CFDataSetLength(pw, n * sizeof(UniChar));
			/* ALIGN: CF aligns to at least >8 byte boundries */
			CFStringGetCharacters(str,
					      CFRangeMake(0, n),
					      (UniChar *)(void *)CFDataGetMutableBytePtr(pw));
			CFRelease(str);
#endif	// INLINE_PASSWORDS_USE_CFSTRING

			CFDictionarySetValue(newConfiguration, key, pw);
			CFRelease(pw);
		} else if (CFEqual(encryptionType, kSCValNetIPSecXAuthPasswordEncryptionKeychain)) {
			Boolean		ok;
			CFDataRef	pw;
			CFStringRef	str;

			str = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
			pw = CFStringCreateExternalRepresentation(NULL, str, kCFStringEncodingUTF8, 0);
			ok = SCNetworkInterfaceSetPassword(net_interface,
							   kSCNetworkInterfacePasswordTypeIPSecXAuth,
							   pw,
							   NULL);
			CFRelease(pw);
			CFRelease(str);
			if (ok) {
				updateInterfaceConfiguration(newConfiguration);
			} else {
				return -1;
			}
		} else {
			SCPrint(TRUE, stdout, CFSTR("IPSec XAuthPassword type \"%@\" not supported\n"), encryptionType);
			return -1;
		}
	} else {
		if (encryptionType == NULL) {
			CFDictionaryRemoveValue(newConfiguration, key);
		} else if (CFEqual(encryptionType, kSCValNetIPSecXAuthPasswordEncryptionKeychain)) {
			Boolean		ok;

			ok = SCNetworkInterfaceRemovePassword(net_interface, kSCNetworkInterfacePasswordTypeIPSecXAuth);
			if (ok) {
				updateInterfaceConfiguration(newConfiguration);
			} else {
				return -1;
			}
		} else {
			SCPrint(TRUE, stdout, CFSTR("IPSec XAuthPassword type \"%@\" not supported\n"), encryptionType);
			return -1;
		}
	}

	return 1;
}


static int
__doIPSecXAuthPasswordType(CFStringRef key, const char *description, void *info, int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("IPSec XAuth password type mode not specified\n"));
		return -1;
	}

	if (strlen(argv[0]) > 0) {
		if (strcasecmp(argv[0], "keychain") == 0) {
			CFDictionarySetValue(newConfiguration, key, kSCValNetIPSecXAuthPasswordEncryptionKeychain);
		} else {
			SCPrint(TRUE, stdout, CFSTR("invalid XAuth password type\n"));
			return -1;
		}
	} else {
		CFDictionaryRemoveValue(newConfiguration, key);
	}

	// encryption type changed, reset XAuthPassword
	CFDictionaryRemoveValue(newConfiguration, kSCPropNetIPSecXAuthPassword);

	return 1;
}


static CF_RETURNS_RETAINED CFStringRef
__cleanupDomainName(CFStringRef domain)
{
	CFMutableStringRef	newDomain;

	newDomain = CFStringCreateMutableCopy(NULL, 0, domain);
	CFStringTrimWhitespace(newDomain);
	CFStringTrim(newDomain, CFSTR("."));
	if (CFStringGetLength(newDomain) == 0) {
		CFRelease(newDomain);
		newDomain = NULL;
	}

	return newDomain;
}


static int
__doOnDemandDomains(CFStringRef key, const char *description, void *info, int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	CFMutableArrayRef	domains;

	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("OnDemand domain name(s) not specified\n"));
		return -1;
	}

	domains = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	if (strlen(argv[0]) > 0) {
		CFArrayRef	array;
		CFStringRef	str;

		str = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
		array = CFStringCreateArrayBySeparatingStrings(NULL, str, CFSTR(","));
		CFRelease(str);

		if (array != NULL) {
			CFIndex	i;
			CFIndex	n	= CFArrayGetCount(array);

			for (i = 0; i < n; i++) {
				CFStringRef	domain;

				domain = __cleanupDomainName(CFArrayGetValueAtIndex(array, i));
				if (domain != NULL) {
					CFArrayAppendValue(domains, domain);
					CFRelease(domain);
				} else {
					CFRelease(array);
					CFRelease(domains);
					SCPrint(TRUE, stdout, CFSTR("invalid OnDemand domain name\n"));
					return -1;
				}
			}
			CFRelease(array);
		}
	}

	if (CFArrayGetCount(domains) > 0) {
		CFDictionarySetValue(newConfiguration, key, domains);
	} else {
		CFDictionaryRemoveValue(newConfiguration, key);
	}

	CFRelease(domains);
	return 1;
}


static options ipsecOnDemandOptions[] = {
	{ "OnDemandMatchDomainsAlways" , "domain", isOther  , &kSCPropNetIPSecOnDemandMatchDomainsAlways , __doOnDemandDomains, NULL },
	{   "always"                   , "domain", isOther  , &kSCPropNetIPSecOnDemandMatchDomainsAlways , __doOnDemandDomains, NULL },
	{ "OnDemandMatchDomainsOnRetry", "domain", isOther  , &kSCPropNetIPSecOnDemandMatchDomainsOnRetry, __doOnDemandDomains, NULL },
	{   "retry"                    , "domain", isOther  , &kSCPropNetIPSecOnDemandMatchDomainsOnRetry, __doOnDemandDomains, NULL },
	{ "OnDemandMatchDomainsNever"  , "domain", isOther  , &kSCPropNetIPSecOnDemandMatchDomainsNever  , __doOnDemandDomains, NULL },
	{   "never"                    , "domain", isOther  , &kSCPropNetIPSecOnDemandMatchDomainsNever  , __doOnDemandDomains, NULL },

	{ "?"                          , NULL    , isHelp , NULL                                       , NULL               ,
	    "\nOnDemandMatch configuration commands\n\n"
	    " set interface OnDemandMatch always domain-name[,domain-name]\n"
	    " set interface OnDemandMatch retry  domain-name[,domain-name]\n"
	    " set interface OnDemandMatch never  domain-name[,domain-name]\n"
	}
};
#define	N_IPSEC_ONDEMAND_OPTIONS	(sizeof(ipsecOnDemandOptions) / sizeof(ipsecOnDemandOptions[0]))


static int
__doIPSecOnDemandMatch(CFStringRef key, const char *description, void *info, int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	Boolean	ok;

	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("set what?\n"));
		return -1;
	}

	ok = _process_options(ipsecOnDemandOptions, N_IPSEC_ONDEMAND_OPTIONS, argc, argv, newConfiguration);
	if (!ok) {
		goto done;
	}

    done :

	return argc;
}


static selections ipsecAuthenticationMethodSelections[] = {
	{ CFSTR("SharedSecret"), &kSCValNetIPSecAuthenticationMethodSharedSecret, 0 },
	{ CFSTR("Certificate") , &kSCValNetIPSecAuthenticationMethodCertificate , 0 },
	{ CFSTR("Hybrid")      , &kSCValNetIPSecAuthenticationMethodHybrid      , 0 },
	{ NULL                 , NULL                                           , 0 }
};


static selections ipsecLocalIdentifierTypeSelections[] = {
	{ CFSTR("KeyID")       , &kSCValNetIPSecLocalIdentifierTypeKeyID        , 0 },
	{ NULL                 , NULL                                           , 0 }
};


static options ipsecOptions[] = {
	{ "AuthenticationMethod"   , NULL, isChooseOne  , &kSCPropNetIPSecAuthenticationMethod   , NULL                      , (void *)ipsecAuthenticationMethodSelections },
	{ "LocalIdentifier"        , NULL, isString     , &kSCPropNetIPSecLocalIdentifier        , NULL                      , NULL                                        },
	{   "group"                , NULL, isString     , &kSCPropNetIPSecLocalIdentifier        , NULL                      , NULL                                        },
	{ "LocalIdentifierType"    , NULL, isChooseOne  , &kSCPropNetIPSecLocalIdentifierType    , NULL                      , (void *)ipsecLocalIdentifierTypeSelections  },
	{ "RemoteAddress"          , NULL, isString     , &kSCPropNetIPSecRemoteAddress          , NULL                      , NULL                                        },
	{ "SharedSecret"           , NULL, isOther      , &kSCPropNetIPSecSharedSecret           , __doIPSecSharedSecret     , NULL                                        },
	{ "SharedSecretEncryption" , NULL, isOther      , &kSCPropNetIPSecSharedSecretEncryption , __doIPSecSharedSecretType , NULL                                        },

	// --- XAuth: ---
	{ "XAuthEnabled"           , NULL, isBoolean    , &kSCPropNetIPSecXAuthEnabled           , NULL                      , NULL                                        },
	{ "XAuthName"              , NULL, isString     , &kSCPropNetIPSecXAuthName              , NULL                      , NULL                                        },
	{ "XAuthPassword"          , NULL, isOther      , &kSCPropNetIPSecXAuthPassword          , __doIPSecXAuthPassword    , NULL                                        },
	{ "XAuthPasswordEncryption", NULL, isOther      , &kSCPropNetIPSecXAuthPasswordEncryption, __doIPSecXAuthPasswordType, NULL                                        },

	// --- OnDemand: ---
	{ "OnDemandEnabled"        , NULL, isBoolean    , &kSCPropNetIPSecOnDemandEnabled        , NULL                      , NULL },
	{ "OnDemandMatch"          , NULL, isOther      , NULL                                   , __doIPSecOnDemandMatch    , NULL },

	{ "?"         , NULL , isHelp     , NULL                            , NULL,
		"\nIPSec configuration commands\n\n"
		" set interface [AuthenticationMethod {SharedSecret|Certificate|Hybrid}]\n"
		" set interface [LocalIdentifier group]\n"
		" set interface [LocalIdentifierType {KeyID}]\n"
		" set interface [RemoteAddress name-or-address]\n"
		" set interface [SharedSecret secret]\n"
		" set interface [SharedSecretEncryption {Keychain}]\n"
		" set interface [XAuthEnabled {enable|disable}]\n"
		" set interface [XAuthPassword password]\n"
		" set interface [XAuthPasswordEncryption {Keychain}]\n"
		" set interface [OnDemandEnabled {enable|disable}]\n"
		" set interface [OnDemandMatch <match-options>]\n"
	}
};
#define	N_IPSEC_OPTIONS	(sizeof(ipsecOptions) / sizeof(ipsecOptions[0]))


static Boolean
set_interface_ipsec(int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	Boolean		ok;

	ok = _process_options(ipsecOptions, N_IPSEC_OPTIONS, argc, argv, newConfiguration);
	return ok;
}


#pragma mark -
#pragma mark FireWire options


static options firewireOptions[] = {
	{ "mtu"       , NULL, isNumber     , &kSCPropNetEthernetMTU         , NULL, NULL },
	{ "media"     , NULL, isString     , &kSCPropNetEthernetMediaSubType, NULL, NULL },
	{ "mediaopt"  , NULL, isStringArray, &kSCPropNetEthernetMediaOptions, NULL, NULL },

	{ "?"         , NULL , isHelp     , NULL                            , NULL,
	    "\nFireWire configuration commands\n\n"
	    " set interface [mtu n] [media type] [mediaopts opts]\n"
	}
};
#define	N_FIREWIRE_OPTIONS	(sizeof(firewireOptions) / sizeof(firewireOptions[0]))


static Boolean
set_interface_firewire(int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	CFStringRef	interfaceName;
	Boolean		ok;

	interfaceName = SCNetworkInterfaceGetBSDName(net_interface);
	if (interfaceName == NULL) {
		SCPrint(TRUE, stdout, CFSTR("no BSD interface\n"));
		return FALSE;
	}

	ok = _process_options(firewireOptions, N_FIREWIRE_OPTIONS, argc, argv, newConfiguration);
	if (ok) {
		// validate configuration
		if (!validateMediaOptions(net_interface, newConfiguration)) {
			return FALSE;
		}
	}

	return ok;
}


#pragma mark -
#pragma mark Modem options


static selections modemDialSelections[] = {
	{ CFSTR("ignore"), &kSCValNetModemDialModeIgnoreDialTone , 0 },
	{ CFSTR("manual"), &kSCValNetModemDialModeManual         , 0 },
	{ CFSTR("wait")  , &kSCValNetModemDialModeWaitForDialTone, 0 },
	{ NULL           , NULL                                  , 0 }
};

static options modemOptions[] = {
	{ "ConnectionPersonality"        , "NULL"  , isString   , &kSCPropNetModemConnectionPersonality      , NULL, NULL                        },
	{ "DeviceModel"                  , "model" , isString   , &kSCPropNetModemDeviceModel                , NULL, NULL                        },
	{ "DeviceVendor"                 , "vendor", isString   , &kSCPropNetModemDeviceVendor               , NULL, NULL                        },
	{ "ConnectionScript"             , "script", isString   , &kSCPropNetModemConnectionScript           , NULL, NULL                        },
	{ "DialMode"                     , "mode"  , isChooseOne, &kSCPropNetModemDialMode                   , NULL, (void *)modemDialSelections },
	{ "CallWaiting"                  , NULL    , isBoolean  , &kSCPropNetModemHoldEnabled                , NULL, NULL                        },
	{ "CallWaitingAlert"             , NULL    , isBoolean  , &kSCPropNetModemHoldCallWaitingAudibleAlert, NULL, NULL                        },
	{ "CallWaitingDisconnectOnAnswer", NULL    , isBoolean  , &kSCPropNetModemHoldDisconnectOnAnswer     , NULL, NULL                        },
	{ "DataCompression"              , NULL    , isBoolean  , &kSCPropNetModemDataCompression            , NULL, NULL                        },
	{ "ErrorCorrection"              , NULL    , isBoolean  , &kSCPropNetModemErrorCorrection            , NULL, NULL                        },
	{ "HoldReminder"                 , NULL    , isBoolean  , &kSCPropNetModemHoldReminder               , NULL, NULL                        },
	{ "HoldReminderTime"             , "time"  , isNumber   , &kSCPropNetModemHoldReminderTime           , NULL, NULL                        },
	{ "PulseDial"                    , NULL    , isBoolean  , &kSCPropNetModemPulseDial                  , NULL, NULL                        },
	{ "Speaker"                      , NULL    , isBoolean  , &kSCPropNetModemSpeaker                    , NULL, NULL                        },

	{ "?"                            , NULL    , isHelp     , NULL                                       , NULL,
	    "\nModem configuration commands\n\n"
	    " set interface [DeviceVendor vendor]\n"
	    " set interface [DeviceModel model]\n"
	    " set interface [ConnectionPersonality personality]\n"
	    "\n"
	    " set interface [ConnectionScript connection-script]\n"
	    "\n"
	    " set interface [CallWaiting {enable|disable}]\n"
	    " set interface [CallWaitingAlert {enable|disable}]\n"
	    " set interface [CallWaitingDisconnectOnAnswer {enable|disable}]\n"
	    " set interface [DialMode {ignore|wait}]\n"
	    " set interface [DataCompression {enable|disable}]\n"
	    " set interface [ErrorCorrection {enable|disable}]\n"
	    " set interface [HoldReminder {enable|disable}]\n"
	    " set interface [HoldReminderTime n]\n"
	    " set interface [PulseDial {enable|disable}]\n"
	    " set interface [Speaker {enable|disable}]\n"
	}
};
#define	N_MODEM_OPTIONS	(sizeof(modemOptions) / sizeof(modemOptions[0]))


static Boolean
set_interface_modem(int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	Boolean	ok;

	ok = _process_options(modemOptions, N_MODEM_OPTIONS, argc, argv, newConfiguration);
	return ok;
}


#pragma mark -
#pragma mark PPP options


static int
__doPPPAuthPW(CFStringRef key, const char *description, void *info, int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	CFStringRef	encryptionType;

	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("PPP password not specified\n"));
		return -1;
	}

	encryptionType = CFDictionaryGetValue(newConfiguration, kSCPropNetPPPAuthPasswordEncryption);
	if (strlen(argv[0]) > 0) {
		if (encryptionType == NULL) {
#ifdef	INLINE_PASSWORDS_USE_CFSTRING
			CFStringRef		pw;

			pw = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
#else	// INLINE_PASSWORDS_USE_CFSTRING
			CFIndex			n;
			CFMutableDataRef	pw;
			CFStringRef		str;

			str = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
			n = CFStringGetLength(str);
			pw = CFDataCreateMutable(NULL, n * sizeof(UniChar));
			CFDataSetLength(pw, n * sizeof(UniChar));
			/* ALIGN: CF aligns to at least >8 byte boundries */
			CFStringGetCharacters(str,
					      CFRangeMake(0, n),
					      (UniChar *)(void *)CFDataGetMutableBytePtr(pw));
			CFRelease(str);
#endif	// INLINE_PASSWORDS_USE_CFSTRING

			CFDictionarySetValue(newConfiguration, key, pw);
			CFRelease(pw);
		} else if (CFEqual(encryptionType, kSCValNetPPPAuthPasswordEncryptionKeychain)) {
			Boolean		ok;
			CFDataRef	pw;
			CFStringRef	str;

			str = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
			pw = CFStringCreateExternalRepresentation(NULL, str, kCFStringEncodingUTF8, 0);
			ok = SCNetworkInterfaceSetPassword(net_interface,
							   kSCNetworkInterfacePasswordTypePPP,
							   pw,
							   NULL);
			CFRelease(pw);
			CFRelease(str);
			if (ok) {
				updateInterfaceConfiguration(newConfiguration);
			} else {
				return -1;
			}
		} else {
			SCPrint(TRUE, stdout, CFSTR("PPP password type \"%@\" not supported\n"), encryptionType);
			return -1;
		}
	} else {
		if (encryptionType == NULL) {
			CFDictionaryRemoveValue(newConfiguration, key);
		} else if (CFEqual(encryptionType, kSCValNetPPPAuthPasswordEncryptionKeychain)) {
			Boolean		ok;

			ok = SCNetworkInterfaceRemovePassword(net_interface, kSCNetworkInterfacePasswordTypePPP);
			if (ok) {
				updateInterfaceConfiguration(newConfiguration);
			} else {
				return -1;
			}
		} else {
			SCPrint(TRUE, stdout, CFSTR("PPP password type \"%@\" not supported\n"), encryptionType);
			return -1;
		}
	}

	return 1;
}


static int
__doPPPAuthPWType(CFStringRef key, const char *description, void *info, int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("PPP password type mode not specified\n"));
		return -1;
	}

	if (strlen(argv[0]) > 0) {
		if (strcasecmp(argv[0], "keychain") == 0) {
			CFDictionarySetValue(newConfiguration, key, kSCValNetPPPAuthPasswordEncryptionKeychain);
		} else {
			SCPrint(TRUE, stdout, CFSTR("invalid password type\n"));
			return -1;
		}
	} else {
		CFDictionaryRemoveValue(newConfiguration, key);
	}

	// encryption type changed, reset password
	CFDictionaryRemoveValue(newConfiguration, kSCPropNetPPPAuthPassword);

	return 1;
}


static options l2tp_ipsecOptions[] = {
	{ "SharedSecret"          , NULL, isOther      , &kSCPropNetIPSecSharedSecret          , __doIPSecSharedSecret    , NULL                                        },
	{ "SharedSecretEncryption", NULL, isOther      , &kSCPropNetIPSecSharedSecretEncryption, __doIPSecSharedSecretType, NULL                                        },

	{ "?"         , NULL , isHelp     , NULL                            , NULL,
		"\nIPSec configuration commands\n\n"
		" set interface ipsec [SharedSecret secret]\n"
		" set interface ipsec [SharedSecretEncryption {Keychain}]\n"
	}
};
#define	N_L2TP_IPSEC_OPTIONS	(sizeof(l2tp_ipsecOptions) / sizeof(l2tp_ipsecOptions[0]))


static int
__doPPPIPSec(CFStringRef key, const char *description, void *info, int argc, char **argv, CFMutableDictionaryRef newPPPConfiguration)
{
	SCNetworkInterfaceRef	childInterface;
	CFStringRef		childInterfaceType;
	CFDictionaryRef		configuration;
	CFMutableDictionaryRef	newConfiguration;
	Boolean			ok;

	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("set what?\n"));
		return -1;
	}

	childInterface = SCNetworkInterfaceGetInterface(net_interface);
	if (childInterface == NULL) {
		SCPrint(TRUE, stdout, CFSTR("this interfaces configuration cannot be changed\n"));
		return -1;
	}

	childInterfaceType = SCNetworkInterfaceGetInterfaceType(childInterface);
	if (!CFEqual(childInterfaceType, kSCNetworkInterfaceTypeL2TP)) {
		SCPrint(TRUE, stdout, CFSTR("this interfaces configuration cannot be changed\n"));
		return -1;
	}

	configuration = SCNetworkInterfaceGetExtendedConfiguration(net_interface, kSCEntNetIPSec);
	if (configuration == NULL) {
		newConfiguration = CFDictionaryCreateMutable(NULL,
							     0,
							     &kCFTypeDictionaryKeyCallBacks,
							     &kCFTypeDictionaryValueCallBacks);
	} else {
		newConfiguration = CFDictionaryCreateMutableCopy(NULL, 0, configuration);
		CFDictionaryRemoveValue(newConfiguration, kSCResvInactive);
	}

	ok = _process_options(l2tp_ipsecOptions, N_L2TP_IPSEC_OPTIONS, argc, argv, newConfiguration);
	if (!ok) {
		goto done;
	}

	if (((configuration == NULL) && (CFDictionaryGetCount(newConfiguration) > 0)) ||
	    ((configuration != NULL) && !CFEqual(configuration, newConfiguration))) {
		if (!SCNetworkInterfaceSetExtendedConfiguration(net_interface, kSCEntNetIPSec, newConfiguration)) {
			if (SCError() == kSCStatusNoKey) {
				SCPrint(TRUE, stdout, CFSTR("could not update per-service interface configuration\n"));
			} else {
				SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
			}
			goto done;
		}

		_prefs_changed = TRUE;
	}

    done :

	if (newConfiguration != NULL) CFRelease(newConfiguration);
	return argc;
}


#ifdef	NOTYET
static options pppOnDemandOptions[] = {
	{ "OnDemandMatchDomainsAlways" , "domain", isOther  , &kSCPropNetPPPOnDemandMatchDomainsAlways , __doOnDemandDomains, NULL },
	{   "always"                   , "domain", isOther  , &kSCPropNetPPPOnDemandMatchDomainsAlways , __doOnDemandDomains, NULL },
	{ "OnDemandMatchDomainsOnRetry", "domain", isOther  , &kSCPropNetPPPOnDemandMatchDomainsOnRetry, __doOnDemandDomains, NULL },
	{   "retry"                    , "domain", isOther  , &kSCPropNetPPPOnDemandMatchDomainsOnRetry, __doOnDemandDomains, NULL },
	{ "OnDemandMatchDomainsNever"  , "domain", isOther  , &kSCPropNetPPPOnDemandMatchDomainsNever  , __doOnDemandDomains, NULL },
	{   "never"                    , "domain", isOther  , &kSCPropNetPPPOnDemandMatchDomainsNever  , __doOnDemandDomains, NULL },

	{ "?"                          , NULL    , isHelp , NULL                                     , NULL               ,
	    "\nOnDemandMatch configuration commands\n\n"
	    " set interface OnDemand always domain-name[,domain-name]\n"
	    " set interface OnDemand retry  domain-name[,domain-name]\n"
	    " set interface OnDemand never  domain-name[,domain-name]\n"
	}
};
#define	N_PPP_ONDEMAND_OPTIONS	(sizeof(pppOnDemandOptions) / sizeof(pppOnDemandOptions[0]))


static int
__doPPPOnDemandMatch(CFStringRef key, const char *description, void *info, int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	Boolean	ok;

	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("set what?\n"));
		return -1;
	}

	ok = _process_options(pppOnDemandOptions, N_PPP_ONDEMAND_OPTIONS, argc, argv, newConfiguration);
	if (!ok) {
		goto done;
	}

    done :

	return argc;
}
#endif	// NOTYET


static selections authPromptSelections[] = {
	{ CFSTR("before"), &kSCValNetPPPAuthPromptBefore, 0 },
	{ CFSTR("after") , &kSCValNetPPPAuthPromptAfter , 0 },
	{ NULL    , NULL                                , 0 }
};


static selections authProtocolSelections[] = {
	{ CFSTR("CHAP")   , &kSCValNetPPPAuthProtocolCHAP   , 0 },
	{ CFSTR("EAP")    , &kSCValNetPPPAuthProtocolEAP    , 0 },
	{ CFSTR("MSCHAP1"), &kSCValNetPPPAuthProtocolMSCHAP1, 0 },
	{ CFSTR("MSCHAP2"), &kSCValNetPPPAuthProtocolMSCHAP2, 0 },
	{ CFSTR("PAP")    , &kSCValNetPPPAuthProtocolPAP    , 0 },
	{ NULL            , NULL                            , 0 }
};


static options pppOptions[] = {
	{ "ACSP"                      , NULL          , isBoolean        , &kSCPropNetPPPACSPEnabled               , NULL                , NULL                           },
	{ "ConnectTime"               , "?time"       , isNumber         , &kSCPropNetPPPConnectTime               , NULL                , NULL                           },
	{ "DialOnDemand"              , NULL          , isBoolean        , &kSCPropNetPPPDialOnDemand              , NULL                , NULL                           },
	{ "DisconnectOnFastUserSwitch", NULL          , isBoolean        , &kSCPropNetPPPDisconnectOnFastUserSwitch, NULL                , NULL                           },
	{ "DisconnectOnIdle"          , NULL          , isBoolean        , &kSCPropNetPPPDisconnectOnIdle          , NULL                , NULL                           },
	{ "DisconnectOnIdleTimer"     , "timeout"     , isNumber         , &kSCPropNetPPPDisconnectOnIdleTimer     , NULL                , NULL                           },
	{ "DisconnectOnLogout"        , NULL          , isBoolean        , &kSCPropNetPPPDisconnectOnLogout        , NULL                , NULL                           },
	{ "DisconnectOnSleep"         , NULL          , isBoolean        , &kSCPropNetPPPDisconnectOnSleep         , NULL                , NULL                           },
	{ "DisconnectTime"            , "?time"       , isNumber         , &kSCPropNetPPPDisconnectTime            , NULL                , NULL                           },
	{ "IdleReminder"              , NULL          , isBoolean        , &kSCPropNetPPPIdleReminder              , NULL                , NULL                           },
	{ "IdleReminderTimer"         , "time"        , isNumber         , &kSCPropNetPPPIdleReminderTimer         , NULL                , NULL                           },
	{ "Logfile"                   , "path"        , isString         , &kSCPropNetPPPLogfile                   , NULL                , NULL                           },
	{ "Plugins"                   , "plugin"      , isStringArray    , &kSCPropNetPPPPlugins                   , NULL                , NULL                           },
	{ "RetryConnectTime"          , "time"        , isNumber         , &kSCPropNetPPPRetryConnectTime          , NULL                , NULL                           },
	{ "SessionTimer"              , "time"        , isNumber         , &kSCPropNetPPPSessionTimer              , NULL                , NULL                           },
	{ "UseSessionTimer"           , NULL          , isBoolean        , &kSCPropNetPPPUseSessionTimer           , NULL                , NULL                           },
	{ "VerboseLogging"            , NULL          , isBoolean        , &kSCPropNetPPPVerboseLogging            , NULL                , NULL                           },

	// --- Auth: ---
	{ "AuthEAPPlugins"            , "plugin"      , isStringArray    , &kSCPropNetPPPAuthEAPPlugins            , NULL                , NULL                           },
	{ "AuthName"                  , "account"     , isString         , &kSCPropNetPPPAuthName                  , NULL                , NULL                           },
	{   "Account"                 , "account"     , isString         , &kSCPropNetPPPAuthName                  , NULL                , NULL                           },
	{ "AuthPassword"              , "password"    , isOther          , &kSCPropNetPPPAuthPassword              , __doPPPAuthPW       , NULL                           },
	{   "Password"                , "password"    , isOther          , &kSCPropNetPPPAuthPassword              , __doPPPAuthPW       , NULL                           },
	{ "AuthPasswordEncryption"    , "type"        , isOther          , &kSCPropNetPPPAuthPasswordEncryption    , __doPPPAuthPWType   , NULL                           },
	{ "AuthPrompt"                , "before/after", isChooseOne      , &kSCPropNetPPPAuthPrompt                , NULL                , (void *)authPromptSelections   },
	{ "AuthProtocol"              , "protocol"    , isChooseMultiple , &kSCPropNetPPPAuthProtocol              , NULL                , (void *)authProtocolSelections },

	// --- Comm: ---
	{ "CommRemoteAddress"         , "phone#"      , isString         , &kSCPropNetPPPCommRemoteAddress         , NULL                , NULL                           },
	{   "Number"                  , "phone#"      , isString         , &kSCPropNetPPPCommRemoteAddress         , NULL                , NULL                           },
	{ "CommAlternateRemoteAddress", "phone#"      , isString         , &kSCPropNetPPPCommAlternateRemoteAddress, NULL                , NULL                           },
	{ "CommConnectDelay"          , "time"        , isNumber         , &kSCPropNetPPPCommConnectDelay          , NULL                , NULL                           },
	{ "CommDisplayTerminalWindow" , NULL          , isBoolean        , &kSCPropNetPPPCommDisplayTerminalWindow , NULL                , NULL                           },
	{ "CommRedialCount"           , "retry count" , isNumber         , &kSCPropNetPPPCommRedialCount           , NULL                , NULL                           },
	{ "CommRedialEnabled"         , NULL          , isBoolean        , &kSCPropNetPPPCommRedialEnabled         , NULL                , NULL                           },
	{ "CommRedialInterval"        , "retry delay" , isNumber         , &kSCPropNetPPPCommRedialInterval        , NULL                , NULL                           },
	{ "CommTerminalScript"        , "script"      , isString         , &kSCPropNetPPPCommTerminalScript        , NULL                , NULL                           },
	{ "CommUseTerminalScript"     , NULL          , isBoolean        , &kSCPropNetPPPCommUseTerminalScript     , NULL                , NULL                           },

	// --- CCP: ---
	{ "CCPEnabled"                , NULL          , isBoolean        , &kSCPropNetPPPCCPEnabled                , NULL                , NULL                           },
	{ "CCPMPPE40Enabled"          , NULL          , isBoolean        , &kSCPropNetPPPCCPMPPE40Enabled          , NULL                , NULL                           },
	{ "CCPMPPE128Enabled"         , NULL          , isBoolean        , &kSCPropNetPPPCCPMPPE128Enabled         , NULL                , NULL                           },

	// --- IPCP: ---
	{ "IPCPCompressionVJ"         , NULL          , isBoolean        , &kSCPropNetPPPIPCPCompressionVJ         , NULL                , NULL                           },
	{ "IPCPUsePeerDNS"            , NULL          , isBoolean        , &kSCPropNetPPPIPCPUsePeerDNS            , NULL                , NULL                           },

	// --- LCP: ---
	{ "LCPEchoEnabled"            , NULL          , isBoolean        , &kSCPropNetPPPLCPEchoEnabled            , NULL                , NULL                           },
	{ "LCPEchoFailure"            , NULL          , isNumber         , &kSCPropNetPPPLCPEchoFailure            , NULL                , NULL                           },
	{ "LCPEchoInterval"           , NULL          , isNumber         , &kSCPropNetPPPLCPEchoInterval           , NULL                , NULL                           },
	{ "LCPCompressionACField"     , NULL          , isBoolean        , &kSCPropNetPPPLCPCompressionACField     , NULL                , NULL                           },
	{ "LCPCompressionPField"      , NULL          , isBoolean        , &kSCPropNetPPPLCPCompressionPField      , NULL                , NULL                           },
	{ "LCPMRU"                    , NULL          , isNumber         , &kSCPropNetPPPLCPMRU                    , NULL                , NULL                           },
	{ "LCPMTU"                    , NULL          , isNumber         , &kSCPropNetPPPLCPMTU                    , NULL                , NULL                           },
	{ "LCPReceiveACCM"            , NULL          , isNumber         , &kSCPropNetPPPLCPReceiveACCM            , NULL                , NULL                           },
	{ "LCPTransmitACCM"           , NULL          , isNumber         , &kSCPropNetPPPLCPTransmitACCM           , NULL                , NULL                           },

	// --- IPSec: ---
	{ "IPSec"                     , NULL          , isOther          , NULL                                    , __doPPPIPSec        , NULL                           },

#ifdef	NOTYET
	// --- OnDemand: ---
	{ "OnDemandEnabled"            , NULL         , isBoolean        , &kSCPropNetPPPOnDemandEnabled           , NULL                , NULL },
	{ "OnDemandMatch"              , NULL         , isOther          , NULL                                    , __doPPPOnDemandMatch, NULL },
#endif	// NOTYET

	// --- Help ---
	{ "?"                         , NULL          , isHelp           , NULL                                    , NULL                ,
	    "\nPPP configuration commands\n\n"
	    " set interface [Account account]\n"
	    " set interface [Password password]\n"
	    " set interface [Number telephone-number]\n"
	    " set interface [AlternateNumber telephone-number]\n"
	    " set interface [IdleReminder {enable|disable}]\n"
	    " set interface [IdleReminderTimer time-in-seconds]\n"
	    " set interface [DisconnectOnIdle {enable|disable}]\n"
	    " set interface [DisconnectOnIdleTimer time-in-seconds]\n"
	    " set interface [DisconnectOnLogout {enable|disable}]\n"
	    " set interface [IPSec <ipsec-options>]\n"
#ifdef	NOTYET
	    " set interface [OnDemandEnabled {enable|disable}]\n"
	    " set interface [OnDemandMatch <match-options>]\n"
#endif	// NOTYET
	}
};
#define	N_PPP_OPTIONS	(sizeof(pppOptions) / sizeof(pppOptions[0]))


static Boolean
set_interface_ppp(int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	Boolean	ok;

	ok = _process_options(pppOptions, N_PPP_OPTIONS, argc, argv, newConfiguration);
	return ok;
}


#pragma mark -
#pragma mark VLAN options


static Boolean
set_interface_vlan(int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
// xxxxx ("device", "tag")
SCPrint(TRUE, stdout, CFSTR("vlan interface management not yet supported\n"));
	return FALSE;
}


#pragma mark -
#pragma mark VPN options


static int
__doVPNAuthPW(CFStringRef key, const char *description, void *info, int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	CFStringRef	encryptionType;

	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("VPN password not specified\n"));
		return -1;
	}

	encryptionType = CFDictionaryGetValue(newConfiguration, kSCPropNetVPNAuthPasswordEncryption);
	if (strlen(argv[0]) > 0) {
		if (encryptionType == NULL) {
#ifdef	INLINE_PASSWORDS_USE_CFSTRING
			CFStringRef		pw;

			pw = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
#else	// INLINE_PASSWORDS_USE_CFSTRING
			CFIndex			n;
			CFMutableDataRef	pw;
			CFStringRef		str;

			str = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
			n = CFStringGetLength(str);
			pw = CFDataCreateMutable(NULL, n * sizeof(UniChar));
			CFDataSetLength(pw, n * sizeof(UniChar));
			CFStringGetCharacters(str,
					      CFRangeMake(0, n),
					      (UniChar *)(void *)CFDataGetMutableBytePtr(pw));
			CFRelease(str);
#endif	// INLINE_PASSWORDS_USE_CFSTRING

			CFDictionarySetValue(newConfiguration, key, pw);
			CFRelease(pw);
		} else if (CFEqual(encryptionType, kSCValNetVPNAuthPasswordEncryptionKeychain)) {
			Boolean		ok;
			CFDataRef	pw;
			CFStringRef	str;

			str = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
			pw = CFStringCreateExternalRepresentation(NULL, str, kCFStringEncodingUTF8, 0);
			ok = SCNetworkInterfaceSetPassword(net_interface,
							   kSCNetworkInterfacePasswordTypeVPN,
							   pw,
							   NULL);
			CFRelease(pw);
			CFRelease(str);
			if (ok) {
				updateInterfaceConfiguration(newConfiguration);
			} else {
				return -1;
			}
		} else {
			SCPrint(TRUE, stdout, CFSTR("VPN password type \"%@\" not supported\n"), encryptionType);
			return -1;
		}
	} else {
		if (encryptionType == NULL) {
			CFDictionaryRemoveValue(newConfiguration, key);
		} else if (CFEqual(encryptionType, kSCValNetVPNAuthPasswordEncryptionKeychain)) {
			Boolean		ok;

			ok = SCNetworkInterfaceRemovePassword(net_interface, kSCNetworkInterfacePasswordTypeVPN);
			if (ok) {
				updateInterfaceConfiguration(newConfiguration);
			} else {
				return -1;
			}
		} else {
			SCPrint(TRUE, stdout, CFSTR("PPP password type \"%@\" not supported\n"), encryptionType);
			return -1;
		}
	}

	return 1;
}


static int
__doVPNAuthPWType(CFStringRef key, const char *description, void *info, int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("VPN password type mode not specified\n"));
		return -1;
	}

	if (strlen(argv[0]) > 0) {
		if (strcasecmp(argv[0], "keychain") == 0) {
			CFDictionarySetValue(newConfiguration, key, kSCValNetVPNAuthPasswordEncryptionKeychain);
		} else if (strcasecmp(argv[0], "prompt") == 0) {
			CFDictionarySetValue(newConfiguration, key, kSCValNetVPNAuthPasswordEncryptionPrompt);
		} else {
			SCPrint(TRUE, stdout, CFSTR("invalid password type\n"));
			return -1;
		}
	} else {
		CFDictionaryRemoveValue(newConfiguration, key);
	}

	// encryption type changed, reset password
	CFDictionaryRemoveValue(newConfiguration, kSCPropNetVPNAuthPassword);

	return 1;
}


static selections vpnAuthenticationMethodSelections[] = {
	{ CFSTR("Password")    , &kSCValNetVPNAuthenticationMethodPassword    , 0 },
	{ CFSTR("Certificate") , &kSCValNetVPNAuthenticationMethodCertificate , 0 },
	{ NULL                 , NULL                                         , 0 }
};


static options vpnOptions[] = {
	{ "AuthName"                  , "account"     , isString     , &kSCPropNetVPNAuthName                  , NULL                , NULL                                      },
	{   "Account"                 , "account"     , isString     , &kSCPropNetVPNAuthName                  , NULL                , NULL                                      },
	{ "AuthPassword"              , "password"    , isOther      , &kSCPropNetVPNAuthPassword              , __doVPNAuthPW       , NULL                                      },
	{   "Password"                , "password"    , isOther      , &kSCPropNetVPNAuthPassword              , __doVPNAuthPW       , NULL                                      },
	{ "AuthPasswordEncryption"    , "type"        , isOther      , &kSCPropNetVPNAuthPasswordEncryption    , __doVPNAuthPWType   , NULL                                      },
	{ "AuthenticationMethod"      , NULL          , isChooseOne  , &kSCPropNetVPNAuthenticationMethod      , NULL                , (void *)vpnAuthenticationMethodSelections },
	{ "ConnectTime"               , "?time"       , isNumber     , &kSCPropNetVPNConnectTime               , NULL                , NULL                                      },
	{ "DisconnectOnFastUserSwitch", NULL          , isBoolean    , &kSCPropNetVPNDisconnectOnFastUserSwitch, NULL                , NULL                                      },
	{ "DisconnectOnIdle"          , NULL          , isBoolean    , &kSCPropNetVPNDisconnectOnIdle          , NULL                , NULL                                      },
	{ "DisconnectOnIdleTimer"     , "timeout"     , isNumber     , &kSCPropNetVPNDisconnectOnIdleTimer     , NULL                , NULL                                      },
	{ "DisconnectOnLogout"        , NULL          , isBoolean    , &kSCPropNetVPNDisconnectOnLogout        , NULL                , NULL                                      },
	{ "DisconnectOnSleep"         , NULL          , isBoolean    , &kSCPropNetVPNDisconnectOnSleep         , NULL                , NULL                                      },
	{ "Logfile"                   , "path"        , isString     , &kSCPropNetVPNLogfile                   , NULL                , NULL                                      },
	{ "MTU"                       , NULL          , isNumber     , &kSCPropNetVPNMTU                       , NULL                , NULL                                      },
	{ "RemoteAddress"             , "server"      , isString     , &kSCPropNetVPNRemoteAddress             , NULL                , NULL                                      },
	{   "Server"                  , "server"      , isString     , &kSCPropNetVPNRemoteAddress             , NULL                , NULL                                      },
	{ "VerboseLogging"            , NULL          , isBoolean    , &kSCPropNetVPNVerboseLogging            , NULL                , NULL                                      },

	// --- Help ---
	{ "?"                         , NULL          , isHelp       , NULL                                    , NULL                ,
	    "\nVPN configuration commands\n\n"
	    " set interface [Server server]\n"
	    " set interface [Account account]\n"
	    " set interface [Password password]\n"
	}
};
#define	N_VPN_OPTIONS	(sizeof(vpnOptions) / sizeof(vpnOptions[0]))


static Boolean
set_interface_vpn(int argc, char **argv, CFMutableDictionaryRef newConfiguration)
{
	Boolean	ok;

	ok = _process_options(vpnOptions, N_VPN_OPTIONS, argc, argv, newConfiguration);
	return ok;
}


#pragma mark -
#pragma mark [more] Interface management


__private_extern__
void
set_interface(int argc, char **argv)
{
	CFDictionaryRef		configuration;
	CFStringRef		interfaceType;
	CFMutableDictionaryRef	newConfiguration	= NULL;
	Boolean			ok			= FALSE;

	if (net_interface == NULL) {
		SCPrint(TRUE, stdout, CFSTR("interface not selected\n"));
		return;
	}

	if (argc < 1) {
		SCPrint(TRUE, stdout, CFSTR("set what?\n"));
		return;
	}

	configuration = SCNetworkInterfaceGetConfiguration(net_interface);
	if (configuration != NULL) {
		configuration = CFDictionaryCreateCopy(NULL, configuration);
		newConfiguration = CFDictionaryCreateMutableCopy(NULL, 0, configuration);
		CFDictionaryRemoveValue(newConfiguration, kSCResvInactive);
	} else {
		newConfiguration = CFDictionaryCreateMutable(NULL,
							     0,
							     &kCFTypeDictionaryKeyCallBacks,
							     &kCFTypeDictionaryValueCallBacks);
	}

	interfaceType = SCNetworkInterfaceGetInterfaceType(net_interface);

	if (CFEqual(interfaceType, kSCNetworkInterfaceTypeEthernet)) {
		ok = set_interface_ethernet(argc, argv, newConfiguration);
	} else if (CFEqual(interfaceType, kSCNetworkInterfaceTypeFireWire)) {
		ok = set_interface_firewire(argc, argv, newConfiguration);
	} else if (CFEqual(interfaceType, kSCNetworkInterfaceTypeIPSec)) {
		ok = set_interface_ipsec(argc, argv, newConfiguration);
	} else if (CFEqual(interfaceType, kSCNetworkInterfaceTypeModem)) {
		ok = set_interface_modem(argc, argv, newConfiguration);
	} else if (CFEqual(interfaceType, kSCNetworkInterfaceTypeIEEE80211)) {
		ok = set_interface_airport(argc, argv, newConfiguration);
	} else if (CFEqual(interfaceType, kSCNetworkInterfaceTypePPP)) {
		ok = set_interface_ppp(argc, argv, newConfiguration);
	} else if (CFEqual(interfaceType, kSCNetworkInterfaceTypeBond)) {
		ok = set_interface_bond(argc, argv, newConfiguration);
	} else if (CFEqual(interfaceType, kSCNetworkInterfaceTypeBridge)) {
		ok = set_interface_bridge(argc, argv, newConfiguration);
	} else if (CFEqual(interfaceType, kSCNetworkInterfaceTypeVLAN)) {
		ok = set_interface_vlan(argc, argv, newConfiguration);
	} else if (CFEqual(interfaceType, kSCNetworkInterfaceTypeVPN)) {
		ok = set_interface_vpn(argc, argv, newConfiguration);
	} else {
		SCPrint(TRUE, stdout, CFSTR("this interfaces configuration cannot be changed\n"));
	}

	if (!ok) {
		goto done;
	}

	if (((configuration == NULL) && (CFDictionaryGetCount(newConfiguration) > 0)) ||
	    ((configuration != NULL) && !CFEqual(configuration, newConfiguration))) {
		if (!SCNetworkInterfaceSetConfiguration(net_interface, newConfiguration)) {
			if (SCError() == kSCStatusNoKey) {
				SCPrint(TRUE, stdout, CFSTR("could not update per-service interface configuration\n"));
			} else {
				SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
			}
			goto done;
		}

		_prefs_changed = TRUE;
	}

    done :

	if (configuration != NULL) CFRelease(configuration);
	if (newConfiguration != NULL) CFRelease(newConfiguration);
	return;
}


/* -------------------- */


__private_extern__
void
show_interface(int argc, char **argv)
{
	SCNetworkInterfaceRef	interface;

	if (argc >= 1) {
		interface = _find_interface(argc, argv, NULL);
	} else {
		if (net_interface != NULL) {
			interface = net_interface;
		} else {
			SCPrint(TRUE, stdout, CFSTR("interface not selected\n"));
			return;
		}
	}

	if (interface != NULL) {
		_show_interface(interface, CFSTR(""), TRUE);
	}

	return;
}


/* -------------------- */


__private_extern__
CF_RETURNS_RETAINED CFStringRef
_interface_description(SCNetworkInterfaceRef interface)
{
	CFMutableStringRef	description;
	CFStringRef		if_bsd_name;
	CFStringRef		if_type;

	description = CFStringCreateMutable(NULL, 0);

	if_type = SCNetworkInterfaceGetInterfaceType(interface);
	CFStringAppend(description, if_type);

	if_bsd_name = SCNetworkInterfaceGetBSDName(interface);
	if (if_bsd_name != NULL) {
		CFStringAppendFormat(description, NULL, CFSTR(" (%@)"), if_bsd_name);
	}

	interface = SCNetworkInterfaceGetInterface(interface);
	while ((interface != NULL) &&
	       !CFEqual(interface, kSCNetworkInterfaceIPv4)) {
		CFStringRef	childDescription;

		childDescription = _interface_description(interface);
		CFStringAppendFormat(description, NULL, CFSTR(" / %@"), childDescription);
		CFRelease(childDescription);

		interface = SCNetworkInterfaceGetInterface(interface);
	}

	return description;
}

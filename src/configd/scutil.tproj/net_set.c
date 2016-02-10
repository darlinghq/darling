/*
 * Copyright (c) 2004, 2005, 2009-2011, 2013, 2014 Apple Inc. All rights reserved.
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
#include "net_set.h"
#include "net_service.h"
#include "prefs.h"


/* -------------------- */


static CFComparisonResult
_compare_sets(const void *val1, const void *val2, void *context)
{
	CFStringRef	id1;
	CFStringRef	id2;
	CFStringRef	name1;
	CFStringRef	name2;
	SCNetworkSetRef	s1	= (SCNetworkSetRef)val1;
	SCNetworkSetRef	s2	= (SCNetworkSetRef)val2;

	name1 = SCNetworkSetGetName(s1);
	name2 = SCNetworkSetGetName(s2);

	if (name1 != NULL) {
		if (name2 != NULL) {
			return CFStringCompare(name1, name2, 0);
		} else {
			return kCFCompareLessThan;
		}
	}

	if (name2 != NULL) {
		return kCFCompareGreaterThan;
	}

	id1 = SCNetworkSetGetSetID(s1);
	id2 = SCNetworkSetGetSetID(s2);
	return CFStringCompare(id1, id2, 0);
}


static CFArrayRef
_copy_sets()
{
	CFArrayRef		sets;
	CFMutableArrayRef	sorted;

	sets = SCNetworkSetCopyAll(prefs);
	if (sets == NULL) {
		SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
		return NULL;
	}

	sorted = CFArrayCreateMutableCopy(NULL, 0, sets);
	CFArraySortValues(sorted,
			  CFRangeMake(0, CFArrayGetCount(sorted)),
			  _compare_sets,
			  NULL);

	CFRelease(sets);
	sets = CFArrayCreateCopy(NULL, sorted);
	CFRelease(sorted);
	return sets;
}


static SCNetworkSetRef
_find_set(char *match)
{
	Boolean			allowIndex	= TRUE;
	CFIndex			i;
	CFIndex			n;
	CFStringRef		select_name	= NULL;
	SCNetworkSetRef		selected	= NULL;

	if (sets == NULL) {
		if (prefs == NULL) {
			SCPrint(TRUE, stdout, CFSTR("network configuration not open\n"));
			return NULL;
		}

		sets = _copy_sets();
		if (sets == NULL) {
			return NULL;
		}

		allowIndex = FALSE;
	}

	// try to select the set by its setID

	select_name = CFStringCreateWithCString(NULL, match, kCFStringEncodingUTF8);

	n = CFArrayGetCount(sets);
	for (i = 0; i < n; i++) {
		SCNetworkSetRef	set;
		CFStringRef	setID;

		set   = CFArrayGetValueAtIndex(sets, i);
		setID = SCNetworkSetGetSetID(set);
		if (CFEqual(select_name, setID)) {
			selected = set;
			goto done;

		}
	}

	// try to select the set by its name

	for (i = 0; i < n; i++) {
		SCNetworkSetRef	set;
		CFStringRef	setName;

		set     = CFArrayGetValueAtIndex(sets, i);
		setName = SCNetworkSetGetName(set);
		if ((setName != NULL) &&
		    CFEqual(select_name, setName)) {
			if (selected == NULL) {
				selected = set;
			} else {
				// if multiple sets match
				selected = NULL;
				SCPrint(TRUE, stdout, CFSTR("multiple sets match\n"));
				goto done;
			}
		}
	}

	if (selected != NULL) {
		goto done;
	}

	// try to select the set by its name (case insensitive)

	for (i = 0; i < n; i++) {
		SCNetworkSetRef	set;
		CFStringRef	setName;

		set     = CFArrayGetValueAtIndex(sets, i);
		setName = SCNetworkSetGetName(set);
		if ((setName != NULL) &&
		    CFStringCompare(select_name,
				    setName,
				    kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
			if (selected == NULL) {
				selected = set;
			} else {
				// if multiple sets match
				selected = NULL;
				SCPrint(TRUE, stdout, CFSTR("multiple sets match\n"));
				goto done;
			}
		}
	}

	if (selected != NULL) {
		goto done;
	}

	// try to select the set by its index

	if (allowIndex) {
		char	*end;
		char	*str	= match;
		long	val;

		errno = 0;
		val = strtol(str, &end, 10);
		if ((*str != '\0') && (*end == '\0') && (errno == 0)) {
			if ((val > 0) && (val <= n)) {
				selected = CFArrayGetValueAtIndex(sets, val - 1);
			}
		}
	}

	if (selected != NULL) {
		goto done;
	}

	SCPrint(TRUE, stdout, CFSTR("no match, which set?\n"));

    done :

	if (select_name != NULL) CFRelease(select_name);
	return selected;
}


/* -------------------- */


__private_extern__
void
create_set(int argc, char **argv)
{
	SCNetworkSetRef	set;
	CFStringRef	setName;

	if (prefs == NULL) {
		SCPrint(TRUE, stdout, CFSTR("network configuration not open\n"));
		return;
	}

	set = SCNetworkSetCreate(prefs);
	if (set == NULL) {
		SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
		return;
	}

	if ((argc > 0) && (strlen(argv[0]) > 0)) {
		Boolean		ok;

		setName = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
//		argv++;
//		argc--;

		ok = SCNetworkSetSetName(set, setName);
		CFRelease(setName);
		if (!ok) {
			SCPrint(TRUE, stdout, CFSTR("set not created: %s\n"), SCErrorString(SCError()));
			(void) SCNetworkSetRemove(set);
			CFRelease(set);
			return;
		}
	}

	_prefs_changed = TRUE;

	if (net_set != NULL) CFRelease(net_set);
	net_set = set;

	setName = SCNetworkSetGetName(set);
	if (setName != NULL) {
		SCPrint(TRUE, stdout,
			CFSTR("set \"%@\" (%@) created and selected\n"),
			setName,
			SCNetworkSetGetSetID(set));
	} else {
		SCPrint(TRUE, stdout,
			CFSTR("set ID \"%@\" created and selected\n"),
			SCNetworkSetGetSetID(set));
	}

	if (net_service != NULL) {
		CFRelease(net_service);
		net_service = NULL;
		SCPrint(TRUE, stdout, CFSTR("& no service selected\n"));
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

	if (sets != NULL) {
		CFRelease(sets);
		sets = NULL;
	}

	return;
}


__private_extern__
void
remove_set(int argc, char **argv)
{
	SCNetworkSetRef	set	= NULL;
	CFStringRef	setName;

	if (argc == 1) {
		set = _find_set(argv[0]);
	} else {
		if (net_set != NULL) {
			set = net_set;
		}
	}

	if (set == NULL) {
		return;
	}

	CFRetain(set);

	if (!SCNetworkSetRemove(set)) {
		SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
		goto done;
	}

	_prefs_changed = TRUE;

	setName = SCNetworkSetGetName(set);
	if (setName != NULL) {
		SCPrint(TRUE, stdout, CFSTR("set \"%@\" removed\n"), setName);
	} else {
		SCPrint(TRUE, stdout,
			CFSTR("set ID \"%@\" removed\n"),
			SCNetworkSetGetSetID(set));
	}

	if (CFEqual(set, net_set)) {
		if (net_service != NULL) {
			CFRelease(net_service);
			net_service = NULL;
			SCPrint(TRUE, stdout, CFSTR("& no service selected\n"));
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

	if (sets != NULL) {
		CFRelease(sets);
		sets = NULL;
	}

    done :

	CFRelease(set);
	return;
}


__private_extern__
void
select_set(int argc, char **argv)
{
	SCNetworkSetRef	set;
	CFStringRef	setName;

	set = _find_set(argv[0]);

	if (set == NULL) {
		return;
	}

	if (net_set != NULL) CFRelease(net_set);
	net_set = CFRetain(set);

	setName = SCNetworkSetGetName(set);
	if (setName != NULL) {
		SCPrint(TRUE, stdout, CFSTR("set \"%@\" selected\n"), setName);
	} else {
		SCPrint(TRUE, stdout,
			CFSTR("set ID \"%@\" selected\n"),
			SCNetworkSetGetSetID(set));
	}

	if (net_service != NULL) {
		CFRelease(net_service);
		net_service = NULL;
		SCPrint(TRUE, stdout, CFSTR("& no service selected\n"));
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

	return;
}


__private_extern__
void
set_set(int argc, char **argv)
{
	Boolean	ok;

	if (net_set == NULL) {
		SCPrint(TRUE, stdout, CFSTR("set not selected\n"));
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
			CFStringRef	setName;

			if (argc < 1) {
				SCPrint(TRUE, stdout, CFSTR("name not specified\n"));
				return;
			}

			setName = (strlen(argv[0]) > 0)
					? CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8)
					: NULL;
			argv++;
			argc--;

			ok = SCNetworkSetSetName(net_set, setName);
			if (setName != NULL) CFRelease(setName);
			if (!ok) {
				SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
				return;
			}

			_prefs_changed = TRUE;

			if (sets != NULL) {
				/*
				 * since the (displayed) ordering may have changed, refresh sets
				 */
				char	*setID;

				setID = _SC_cfstring_to_cstring(SCNetworkSetGetSetID(net_set),
								NULL,
								0,
								kCFStringEncodingUTF8);

				CFRelease(net_set);
				net_set = NULL;

				CFRelease(sets);
				sets = NULL;

				net_set = _find_set(setID);
				if (net_set != NULL) {
					CFRetain(net_set);
				}

				CFAllocatorDeallocate(NULL, setID);
			}
		} else if (strcmp(command, "current") == 0) {
			ok = SCNetworkSetSetCurrent(net_set);
			if (!ok) {
				SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
				return;
			}

			_prefs_changed = TRUE;
		} else if (strcmp(command, "id") == 0) {
			char		*newID;
			CFStringRef	setID;

			if ((argc < 1) || (strlen(argv[0]) == 0)) {
				SCPrint(TRUE, stdout, CFSTR("set id not specified\n"));
				return;
			}

			newID = argv[0];
			setID = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
			argv++;
			argc--;

			ok = _SCNetworkSetSetSetID(net_set, setID);
			CFRelease(setID);
			if (!ok) {
				SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
				return;
			}

			_prefs_changed = TRUE;

			if (sets != NULL) {
				/*
				 * since the (displayed) ordering may have changed, refresh sets
				 */
				CFRelease(net_set);
				net_set = NULL;

				CFRelease(sets);
				sets = NULL;

				net_set = _find_set(newID);
				if (net_set != NULL) {
					CFRetain(net_set);
				}
			}
		} else {
			SCPrint(TRUE, stdout, CFSTR("set what?\n"));
		}
	}

	return;
}


__private_extern__
void
show_set(int argc, char **argv)
{
	CFArrayRef	interfaces;
	CFArrayRef	services;
	SCNetworkSetRef	set	= NULL;
	CFStringRef	setName;

	if (argc == 1) {
		set = _find_set(argv[0]);
	} else {
		if (net_set != NULL) {
			set = net_set;
		} else {
			SCPrint(TRUE, stdout, CFSTR("set not selected\n"));
			return;
		}
	}

	if (set == NULL) {
		return;
	}

	SCPrint(TRUE, stdout, CFSTR("set id               = %@\n"), SCNetworkSetGetSetID(set));

	setName = SCNetworkSetGetName(set);
	SCPrint(TRUE, stdout, CFSTR("name                 = %@\n"),
		(setName != NULL) ? setName : CFSTR(""));

	services = SCNetworkSetCopyServices(set);
	if (services != NULL) {
		CFIndex			i;
		CFIndex			n;
		CFIndex			nOrder	= 0;
		CFArrayRef		order;

		order = SCNetworkSetGetServiceOrder(set);
		if (order != NULL) {
			nOrder = CFArrayGetCount(order);
		}

		n = CFArrayGetCount(services);
		if (n > 1) {
			CFMutableArrayRef	sorted;

			sorted = CFArrayCreateMutableCopy(NULL, 0, services);
			CFArraySortValues(sorted,
					  CFRangeMake(0, CFArrayGetCount(sorted)),
					  _SCNetworkServiceCompare,
					  (void *)order);
			CFRelease(services);
			services = sorted;
		}

		SCPrint(TRUE, stdout, CFSTR("services =\n"));

		for (i = 0; i < n; i++) {
			CFIndex			orderIndex	= kCFNotFound;
			SCNetworkServiceRef	service;
			CFStringRef		serviceName;
			CFStringRef		serviceID;

			service     = CFArrayGetValueAtIndex(services, i);
			serviceID   = SCNetworkServiceGetServiceID(service);
			serviceName = SCNetworkServiceGetName(service);
			if (serviceName == NULL) serviceName = CFSTR("");

			if (order != NULL) {
				orderIndex  = CFArrayGetFirstIndexOfValue(order,
									  CFRangeMake(0, nOrder),
									  serviceID);
			}
			if (orderIndex != kCFNotFound) {
				SCPrint(TRUE, stdout, CFSTR("%c%2ld: %@%-*s (%@)\n"),
					((net_service != NULL) && CFEqual(service, net_service)) ? '>' : ' ',
					orderIndex + 1,
					serviceName,
					(int)(30 - CFStringGetLength(serviceName)),
					" ",
					serviceID);
			} else {
				SCPrint(TRUE, stdout, CFSTR("%c  : %@%-*s (%@)\n"),
					((net_service != NULL) && CFEqual(service, net_service)) ? '>' : ' ',
					serviceName,
					(int)(30 - CFStringGetLength(serviceName)),
					" ",
					serviceID);
			}
		}

		CFRelease(services);
	}

	interfaces = SCNetworkSetCopyAvailableInterfaces(set);
	if (interfaces != NULL) {
		CFIndex			i;
		SCNetworkInterfaceRef	interface;
		CFIndex			n;

		SCPrint(TRUE, stdout, CFSTR("available interfaces =\n"));

		n = CFArrayGetCount(interfaces);
		for (i = 0; i < n; i++) {
			interface = CFArrayGetValueAtIndex(interfaces, i);
			SCPrint(TRUE, stdout, CFSTR(" %2ld: %@ \n"),
				i + 1,
				SCNetworkInterfaceGetLocalizedDisplayName(interface));
		}

		CFRelease(interfaces);
	}

	if (_sc_debug) {
		SCPrint(TRUE, stdout, CFSTR("\n%@\n"), set);
	}

	return;
}


__private_extern__
void
show_sets(int argc, char **argv)
{
	SCNetworkSetRef	current;
	CFIndex		i;
	CFIndex		n;

	if (prefs == NULL) {
		SCPrint(TRUE, stdout, CFSTR("network configuration not open\n"));
		return;
	}

	if (sets == NULL) {
		sets = _copy_sets();
		if (sets == NULL) {
			return;
		}
	}

	current = SCNetworkSetCopyCurrent(prefs);

	n = CFArrayGetCount(sets);
	for (i = 0; i < n; i++) {
		SCNetworkSetRef	set;
		CFStringRef	setID;
		CFStringRef	setName;

		set     = CFArrayGetValueAtIndex(sets, i);
		setID   = SCNetworkSetGetSetID(set);
		setName = SCNetworkSetGetName(set);
		if (setName == NULL) setName = CFSTR("");

		SCPrint(TRUE, stdout, CFSTR(" %c%c%2ld: %@%-*s (%@)\n"),
			((current != NULL) && CFEqual(set, current)) ? '*' : ' ',
			((net_set != NULL) && CFEqual(set, net_set)) ? '>' : ' ',
			i + 1,
			setName,
			(int)(30 - CFStringGetLength(setName)),
			" ",
			setID);
	}

	if (current != NULL)	CFRelease(current);

	return;
}

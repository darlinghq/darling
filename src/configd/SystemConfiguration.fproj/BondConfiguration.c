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
 * November 28, 2005		Allan Nathanson <ajn@apple.com>
 * - public API
 *
 * July 22, 2004		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>

#include <SystemConfiguration/SystemConfiguration.h>
#include "SCNetworkConfigurationInternal.h"
#include "SCPreferencesInternal.h"
#include <SystemConfiguration/SCValidation.h>
#include <SystemConfiguration/SCPrivate.h>

#include <ifaddrs.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/ethernet.h>
#define	KERNEL_PRIVATE
#include <net/if.h>
#include <net/if_var.h>
#undef	KERNEL_PRIVATE
#include <net/if_bond_var.h>
#include <net/if_types.h>
#include <net/if_media.h>
#include <net/route.h>

/* ---------- Bond support ---------- */

static int
inet_dgram_socket()
{
	int	s;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		SCLog(TRUE, LOG_ERR, CFSTR("socket() failed: %s"), strerror(errno));
	}

	return s;
}

static int
siocgifmedia(int s, const char * ifname, int * status, int * active)
{
	struct ifmediareq	ifmr;

	*status = 0;
	*active = 0;
	bzero(&ifmr, sizeof(ifmr));
	strncpy(ifmr.ifm_name, ifname, sizeof(ifmr.ifm_name));
	if (ioctl(s, SIOCGIFMEDIA, &ifmr) == -1) {
		return (-1);
	}
	if (ifmr.ifm_count != 0) {
		*status = ifmr.ifm_status;
		*active = ifmr.ifm_active;
	}
	return (0);
}

static struct if_bond_status_req *
if_bond_status_req_copy(int s, const char * ifname)
{
	void *				buf = NULL;
	struct if_bond_req		ibr;
	struct if_bond_status_req *	ibsr_p;
	struct ifreq			ifr;

	bzero(&ifr, sizeof(ifr));
	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	bzero((char *)&ibr, sizeof(ibr));
	ibr.ibr_op = IF_BOND_OP_GET_STATUS;
	ibsr_p = &ibr.ibr_ibru.ibru_status;
	ibsr_p->ibsr_version = IF_BOND_STATUS_REQ_VERSION;
	ifr.ifr_data = (caddr_t)&ibr;

	/* how many of them are there? */
	if (ioctl(s, SIOCGIFBOND, (caddr_t)&ifr) == -1) {
		goto failed;
	}
	buf = malloc(sizeof(struct if_bond_status) * ibsr_p->ibsr_total + sizeof(*ibsr_p));
	if (buf == NULL) {
		goto failed;
	}
	if (ibsr_p->ibsr_total == 0) {
		goto done;
	}
	ibsr_p->ibsr_count = ibsr_p->ibsr_total;
	ibsr_p->ibsr_buffer = buf + sizeof(*ibsr_p);

	/* get the list */
	if (ioctl(s, SIOCGIFBOND, (caddr_t)&ifr) == -1) {
		goto failed;
	}
 done:
	(*(struct if_bond_status_req *)buf) = *ibsr_p;
	return ((struct if_bond_status_req *)buf);

 failed:
	if (buf != NULL) {
		free(buf);
	}
	return (NULL);
}


static void
add_interface(CFMutableArrayRef *interfaces, CFStringRef if_name, SCPreferencesRef ni_prefs)
{
	SCNetworkInterfaceRef	interface = NULL;

	if (*interfaces == NULL) {
		*interfaces = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}
	if (ni_prefs != NULL) {
		interface = __SCNetworkInterfaceCreateWithNIPreferencesUsingBSDName(NULL, ni_prefs, if_name);
	}
	if (interface == NULL) {
		interface = _SCNetworkInterfaceCreateWithBSDName(NULL, if_name,
								 kIncludeNoVirtualInterfaces);
	}
	
	if (interface != NULL) {
		CFArrayAppendValue(*interfaces, interface);
		CFRelease(interface);
	}
}


static Boolean
_SCBondInterfaceSetMode(SCBondInterfaceRef bond, CFNumberRef mode);


typedef struct {
	CFMutableArrayRef	bonds;
	SCPreferencesRef	ni_prefs;
	SCPreferencesRef	prefs;
} addContext, *addContextRef;


static void
add_configured_interface(const void *key, const void *value, void *context)
{
	SCBondInterfaceRef		bond;
	CFStringRef			bond_if		= (CFStringRef)key;
	CFDictionaryRef			bond_info	= (CFDictionaryRef)value;
	CFDictionaryRef			bond_options;
	CFIndex				i;
	CFArrayRef			interfaces;
	SCNetworkInterfacePrivateRef	interfacePrivate;
	CFMutableArrayRef		members		= NULL;
	CFNumberRef			mode;
	addContextRef			myContext	= (addContextRef)context;
	CFStringRef			name;
	CFIndex				n;

	// create the bond interface
	bond = (SCBondInterfaceRef)_SCBondInterfaceCreatePrivate(NULL, bond_if);

	// add member interfaces
	interfaces = CFDictionaryGetValue(bond_info, kSCPropVirtualNetworkInterfacesBondInterfaces);
	n = isA_CFArray(interfaces) ? CFArrayGetCount(interfaces) : 0;
	for (i = 0; i < n; i++) {
		CFStringRef	member;

		member = CFArrayGetValueAtIndex(interfaces, i);
		if (isA_CFString(member)) {
			add_interface(&members, member, myContext->ni_prefs);
		}
	}
	if (members != NULL) {
		__SCBondInterfaceSetMemberInterfaces(bond, members);
		CFRelease(members);
	}

	// set display name
	name = CFDictionaryGetValue(bond_info, kSCPropUserDefinedName);
	if (isA_CFString(name)) {
		SCBondInterfaceSetLocalizedDisplayName(bond, name);
	}

	// set options
	bond_options = CFDictionaryGetValue(bond_info, kSCPropVirtualNetworkInterfacesBondOptions);
	if (isA_CFDictionary(bond_options)) {
		SCBondInterfaceSetOptions(bond, bond_options);
	}

	// set the mode
	mode = CFDictionaryGetValue(bond_info, kSCPropVirtualNetworkInterfacesBondMode);
	_SCBondInterfaceSetMode(bond, isA_CFNumber(mode));

	// estabish link to the stored configuration
	interfacePrivate = (SCNetworkInterfacePrivateRef)bond;
	interfacePrivate->prefs = CFRetain(myContext->prefs);

	CFArrayAppendValue(myContext->bonds, bond);
	CFRelease(bond);

	return;
}



#pragma mark -
#pragma mark SCBondInterface APIs


static __inline__ void
my_CFDictionaryApplyFunction(CFDictionaryRef			theDict,
			     CFDictionaryApplierFunction	applier,
			     void				*context)
{
	CFAllocatorRef	myAllocator;
	CFDictionaryRef	myDict;

	myAllocator = CFGetAllocator(theDict);
	myDict      = CFDictionaryCreateCopy(myAllocator, theDict);
	CFDictionaryApplyFunction(myDict, applier, context);
	CFRelease(myDict);
	return;
}


CFArrayRef
SCBondInterfaceCopyAll(SCPreferencesRef prefs)
{
	addContext		context;
	CFDictionaryRef		dict;
	SCPreferencesRef	ni_prefs;
	CFStringRef		path;
	
	if ((prefs == NULL) ||
	    (__SCPreferencesUsingDefaultPrefs(prefs) == TRUE)) {
		ni_prefs = NULL;
	}
	else {
		ni_prefs = __SCPreferencesCreateNIPrefsFromPrefs(prefs);
	}
	
	context.bonds = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	context.prefs = prefs;
	context.ni_prefs = ni_prefs;
	
	path = CFStringCreateWithFormat(NULL,
					NULL,
					CFSTR("/%@/%@"),
					kSCPrefVirtualNetworkInterfaces,
					kSCNetworkInterfaceTypeBond);
	dict = SCPreferencesPathGetValue(prefs, path);
	CFRelease(path);
	if (isA_CFDictionary(dict)) {
		my_CFDictionaryApplyFunction(dict, add_configured_interface, &context);
	}

	if (ni_prefs != NULL) {
		CFRelease(ni_prefs);
	}
	return context.bonds;
}


__private_extern__ void
__SCBondInterfaceListCollectMembers(CFArrayRef interfaces, CFMutableSetRef set)
{
	CFIndex	i;
	CFIndex	n;

	n = CFArrayGetCount(interfaces);
	for (i = 0; i < n; i++) {
		SCBondInterfaceRef	bondInterface;
		CFArrayRef		members;

		bondInterface = CFArrayGetValueAtIndex(interfaces, i);
		members = SCBondInterfaceGetMemberInterfaces(bondInterface);
		if (members != NULL) {
			CFIndex	j;
			CFIndex	n_members;

			// exclude the member interfaces of this bond
			n_members = CFArrayGetCount(members);
			for (j = 0; j < n_members; j++) {
				SCNetworkInterfaceRef	member;

				member = CFArrayGetValueAtIndex(members, j);
				CFSetAddValue(set, member);
			}
		}

	}
	return;
}


CFArrayRef /* of SCNetworkInterfaceRef's */
SCBondInterfaceCopyAvailableMemberInterfaces(SCPreferencesRef prefs)
{
	CFMutableArrayRef	available;
	CFMutableSetRef		excluded;
	CFArrayRef		interfaces;

	available = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	excluded  = CFSetCreateMutable  (NULL, 0, &kCFTypeSetCallBacks);

	// exclude Bond [member] interfaces
	interfaces = SCBondInterfaceCopyAll(prefs);
	if (interfaces != NULL) {
		__SCBondInterfaceListCollectMembers(interfaces, excluded);
		CFRelease(interfaces);
	}

	// exclude Bridge [member] interfaces
	interfaces = SCBridgeInterfaceCopyAll(prefs);
	if (interfaces != NULL) {
		__SCBridgeInterfaceListCollectMembers(interfaces, excluded);
		CFRelease(interfaces);
	}

	// exclude VLAN [physical] interfaces
	interfaces = SCVLANInterfaceCopyAll(prefs);
	if (interfaces != NULL) {
		CFIndex	i;
		CFIndex	n;

		n = CFArrayGetCount(interfaces);
		for (i = 0; i < n; i++) {
			SCVLANInterfaceRef	vlanInterface;
			SCNetworkInterfaceRef	physical;

			// exclude the physical interface of this VLAN
			vlanInterface = CFArrayGetValueAtIndex(interfaces, i);
			physical = SCVLANInterfaceGetPhysicalInterface(vlanInterface);
			CFSetAddValue(excluded, physical);
		}
		CFRelease(interfaces);
	}

	// identify available interfaces
	interfaces = __SCNetworkInterfaceCopyAll_IONetworkInterface();
	if (interfaces != NULL) {
		CFIndex	i;
		CFIndex	n;

		n = CFArrayGetCount(interfaces);
		for (i = 0; i < n; i++) {
			SCNetworkInterfaceRef		interface;
			SCNetworkInterfacePrivateRef	interfacePrivate;

			interface = CFArrayGetValueAtIndex(interfaces, i);
			interfacePrivate = (SCNetworkInterfacePrivateRef)interface;

			if (!interfacePrivate->supportsBond) {
				// if this interface is not available
				continue;
			}

			if (CFSetContainsValue(excluded, interface)) {
				// if excluded
				continue;
			}

			CFArrayAppendValue(available, interface);
		}
		CFRelease(interfaces);
	}

	CFRelease(excluded);

	return available;
}


CFArrayRef
_SCBondInterfaceCopyActive(void)
{
	struct ifaddrs		*ifap;
	struct ifaddrs		*ifp;
	int			s;
	CFMutableArrayRef	bonds	= NULL;

	if (getifaddrs(&ifap) == -1) {
		_SCErrorSet(errno);
		SCLog(TRUE, LOG_ERR, CFSTR("getifaddrs() failed: %s"), strerror(errno));
		return NULL;
	}

	s = inet_dgram_socket();
	if (s == -1) {
		_SCErrorSet(errno);
		goto done;
	}

	bonds = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	for (ifp = ifap; ifp != NULL; ifp = ifp->ifa_next) {
		SCBondInterfaceRef		bond;
		CFStringRef			bond_if;
		struct if_bond_status_req	*ibsr_p;
		struct if_data			*if_data;
		int				int_val;
		CFNumberRef			mode;
		CFMutableArrayRef		members		= NULL;

		if_data = (struct if_data *)ifp->ifa_data;
		if (if_data == NULL
		    || ifp->ifa_addr->sa_family != AF_LINK
		    || if_data->ifi_type != IFT_IEEE8023ADLAG) {
			continue;
		}

		ibsr_p = if_bond_status_req_copy(s, ifp->ifa_name);
		if (ibsr_p == NULL) {
			if (errno == EBUSY) {
				continue;
			}
			_SCErrorSet(errno);
			SCLog(TRUE, LOG_ERR,
			      CFSTR("if_bond_status_req_copy(%s) failed: %s"),
			      ifp->ifa_name,
			      strerror(errno));
			CFRelease(bonds);
			bonds = NULL;
			goto done;
		}

		// create the bond interface
		bond_if = CFStringCreateWithCString(NULL, ifp->ifa_name, kCFStringEncodingASCII);
		bond    = (SCBondInterfaceRef)_SCBondInterfaceCreatePrivate(NULL, bond_if);
		CFRelease(bond_if);

		// set the mode
		int_val = ibsr_p->ibsr_mode;
		mode = CFNumberCreate(NULL, kCFNumberIntType, &int_val);
		assert(mode != NULL);
		_SCBondInterfaceSetMode(bond, mode);
		CFRelease(mode);

		// add member interfaces
		if (ibsr_p->ibsr_total > 0) {
			int 			i;
			struct if_bond_status *	ibs_p;

			// iterate over each member interface
			ibs_p = (struct if_bond_status *)ibsr_p->ibsr_buffer;
			for (i = 0; i < ibsr_p->ibsr_total; i++) {
				CFStringRef	member;

				member = CFStringCreateWithCString(NULL, ibs_p[i].ibs_if_name, kCFStringEncodingASCII);
				add_interface(&members, member, NULL);
				CFRelease(member);
			}
		}
		free(ibsr_p);

		if (members != NULL) {
			__SCBondInterfaceSetMemberInterfaces(bond, members);
			CFRelease(members);
		}

		// add bond
		CFArrayAppendValue(bonds, bond);
		CFRelease(bond);
	}

    done :

	if (s != -1) {
		(void) close(s);
	}
	freeifaddrs(ifap);
	return bonds;
}


SCBondInterfaceRef
SCBondInterfaceCreate(SCPreferencesRef prefs)
{
	CFAllocatorRef		allocator;
	SCBondInterfaceRef	bond		= NULL;
	CFIndex			i;

	if (prefs == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	allocator = CFGetAllocator(prefs);

	// create a new bond using an unused interface name
	for (i = 0; bond == NULL; i++) {
		CFDictionaryRef			dict;
		CFStringRef			bond_if;
		SCNetworkInterfacePrivateRef	interfacePrivate;
		CFMutableDictionaryRef		newDict;
		CFArrayRef			newInterfaces;
		Boolean				ok;
		CFStringRef			path;

		bond_if = CFStringCreateWithFormat(allocator, NULL, CFSTR("bond%ld"), i);
		path    = CFStringCreateWithFormat(allocator,
						   NULL,
						   CFSTR("/%@/%@/%@"),
						   kSCPrefVirtualNetworkInterfaces,
						   kSCNetworkInterfaceTypeBond,
						   bond_if);
		dict = SCPreferencesPathGetValue(prefs, path);
		if (dict != NULL) {
			// if bond interface name not available
			CFRelease(path);
			CFRelease(bond_if);
			continue;
		}

		// add the bond to the stored preferences
		newDict = CFDictionaryCreateMutable(allocator,
						    0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);
		newInterfaces = CFArrayCreate(allocator, NULL, 0, &kCFTypeArrayCallBacks);
		CFDictionaryAddValue(newDict, kSCPropVirtualNetworkInterfacesBondInterfaces, newInterfaces);
		CFRelease(newInterfaces);
		ok = SCPreferencesPathSetValue(prefs, path, newDict);
		CFRelease(newDict);
		CFRelease(path);
		if (!ok) {
			// if the bond could not be saved
			CFRelease(bond_if);
			break;
		}

		// create the SCBondInterfaceRef
		bond = (SCBondInterfaceRef)_SCBondInterfaceCreatePrivate(allocator, bond_if);
		CFRelease(bond_if);

		// estabish link to the stored configuration
		interfacePrivate = (SCNetworkInterfacePrivateRef)bond;
		interfacePrivate->prefs = CFRetain(prefs);
	}

	return bond;
}


Boolean
SCBondInterfaceRemove(SCBondInterfaceRef bond)
{
	CFStringRef			bond_if;
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bond;
	Boolean				ok;
	CFStringRef			path;

	if (!isA_SCBondInterface(bond)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (interfacePrivate->prefs == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	bond_if = SCNetworkInterfaceGetBSDName(bond);
	path    = CFStringCreateWithFormat(NULL,
					   NULL,
					   CFSTR("/%@/%@/%@"),
					   kSCPrefVirtualNetworkInterfaces,
					   kSCNetworkInterfaceTypeBond,
					   bond_if);
	ok = SCPreferencesPathRemoveValue(interfacePrivate->prefs, path);
	CFRelease(path);

	return ok;
}


CFArrayRef
SCBondInterfaceGetMemberInterfaces(SCBondInterfaceRef bond)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bond;

	if (!isA_SCBondInterface(bond)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	return interfacePrivate->bond.interfaces;
}


CFDictionaryRef
SCBondInterfaceGetOptions(SCBondInterfaceRef bond)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bond;

	if (!isA_SCBondInterface(bond)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	return interfacePrivate->bond.options;
}


__private_extern__
Boolean
__SCBondInterfaceSetMemberInterfaces(SCBondInterfaceRef bond, CFArrayRef members)
{
	CFIndex				i;
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bond;
	CFIndex				n;
	CFMutableArrayRef		newMembers;
	Boolean				ok			= TRUE;

	n = (members != NULL) ? CFArrayGetCount(members) : 0;

	// set member interfaces in the stored preferences
	if (interfacePrivate->prefs != NULL) {
		CFDictionaryRef		dict;
		CFMutableDictionaryRef	newDict;
		CFStringRef		path;

		path = CFStringCreateWithFormat(NULL,
						NULL,
						CFSTR("/%@/%@/%@"),
						kSCPrefVirtualNetworkInterfaces,
						kSCNetworkInterfaceTypeBond,
						interfacePrivate->entity_device);
		dict = SCPreferencesPathGetValue(interfacePrivate->prefs, path);
		if (!isA_CFDictionary(dict)) {
			// if the prefs are confused
			CFRelease(path);
			_SCErrorSet(kSCStatusFailed);
			return FALSE;
		}

		newMembers = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		for (i = 0; i < n; i++) {
			SCNetworkInterfaceRef	interface;
			CFStringRef		memberName;

			interface = CFArrayGetValueAtIndex(members, i);
			memberName = SCNetworkInterfaceGetBSDName(interface);
			CFArrayAppendValue(newMembers, memberName);
		}

		newDict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
		CFDictionarySetValue(newDict, kSCPropVirtualNetworkInterfacesBondInterfaces, newMembers);
		CFRelease(newMembers);
		if (!CFEqual(dict, newDict)) {
			ok = SCPreferencesPathSetValue(interfacePrivate->prefs, path, newDict);
		}
		CFRelease(newDict);
		CFRelease(path);
	}

	if (ok) {
		newMembers = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		for (i = 0; i < n; i++) {
			SCNetworkInterfaceRef		member;
			SCNetworkInterfacePrivateRef	newMember;

			member = CFArrayGetValueAtIndex(members, i);
			newMember = __SCNetworkInterfaceCreateCopy(NULL,
								   member,
								   interfacePrivate->prefs,
								   interfacePrivate->serviceID);
			CFArrayAppendValue(newMembers, newMember);
			CFRelease(newMember);
		}
		CFRelease(interfacePrivate->bond.interfaces);
		interfacePrivate->bond.interfaces = newMembers;
	}

	return ok;
}


Boolean
SCBondInterfaceSetMemberInterfaces(SCBondInterfaceRef bond, CFArrayRef members)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bond;
	Boolean				ok;
	int				sc_status		= kSCStatusOK;

	if (!isA_SCBondInterface(bond)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if ((members != NULL) && !isA_CFArray(members)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (interfacePrivate->prefs != NULL) {
		CFArrayRef	available;
		CFArrayRef	current;
		CFIndex		i;
		CFIndex		n_available;
		CFIndex		n_current;
		CFIndex		n_members;
		CFArrayRef	services	= NULL;

		current     = SCBondInterfaceGetMemberInterfaces(bond);
		n_current   = (current != NULL) ? CFArrayGetCount(current) : 0;

		available   = SCBondInterfaceCopyAvailableMemberInterfaces(interfacePrivate->prefs);
		n_available = (available != NULL) ? CFArrayGetCount(available) : 0;

		n_members = (members != NULL) ? CFArrayGetCount(members) : 0;
		for (i = 0; i < n_members; i++) {
			SCNetworkInterfaceRef	member;

			member = CFArrayGetValueAtIndex(members, i);

			if ((current != NULL) &&
			    CFArrayContainsValue(current, CFRangeMake(0, n_current), member)) {
				// current members are allowed
				continue;
			}

			if ((available != NULL) &&
			    CFArrayContainsValue(available, CFRangeMake(0, n_available), member)) {
				// available members are allowed but cannot be associated
				// with any other network services.

				if (services == NULL) {
					services = __SCNetworkServiceCopyAllEnabled(interfacePrivate->prefs);
				}
				if ((services != NULL) &&
				    __SCNetworkServiceExistsForInterface(services, member)) {
					sc_status = kSCStatusKeyExists;
					break;
				}

				// if available
				continue;
			}

			// if member not allowed
			sc_status = kSCStatusInvalidArgument;
			break;
		}

		if (available != NULL) CFRelease(available);
		if (services != NULL) CFRelease(services);
	}

	if (sc_status != kSCStatusOK) {
		_SCErrorSet(sc_status);
		return FALSE;
	}

	ok = __SCBondInterfaceSetMemberInterfaces(bond, members);
	return ok;
}


Boolean
SCBondInterfaceSetLocalizedDisplayName(SCBondInterfaceRef bond, CFStringRef newName)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bond;
	Boolean				ok			= TRUE;

	if (!isA_SCBondInterface(bond)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if ((newName != NULL) && !isA_CFString(newName)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	// set name in the stored preferences
	if (interfacePrivate->prefs != NULL) {
		CFDictionaryRef		dict;
		CFMutableDictionaryRef	newDict;
		CFStringRef		path;

		path = CFStringCreateWithFormat(NULL,
						NULL,
						CFSTR("/%@/%@/%@"),
						kSCPrefVirtualNetworkInterfaces,
						kSCNetworkInterfaceTypeBond,
						interfacePrivate->entity_device);
		dict = SCPreferencesPathGetValue(interfacePrivate->prefs, path);
		if (!isA_CFDictionary(dict)) {
			// if the prefs are confused
			CFRelease(path);
			_SCErrorSet(kSCStatusFailed);
			return FALSE;
		}

		newDict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
		if (newName != NULL) {
			CFDictionarySetValue(newDict, kSCPropUserDefinedName, newName);
		} else {
			CFDictionaryRemoveValue(newDict, kSCPropUserDefinedName);
		}
		if (!CFEqual(dict, newDict)) {
			ok = SCPreferencesPathSetValue(interfacePrivate->prefs, path, newDict);
		}
		CFRelease(newDict);
		CFRelease(path);
	}

	// set name in the SCBondInterfaceRef
	if (ok) {
		if (interfacePrivate->localized_name != NULL) {
			CFRelease(interfacePrivate->localized_name);
			interfacePrivate->localized_name = NULL;
		}
		if (newName != NULL) {
			interfacePrivate->localized_name = CFStringCreateCopy(NULL, newName);
		}
	}

	return ok;
}


Boolean
SCBondInterfaceSetOptions(SCBondInterfaceRef bond, CFDictionaryRef newOptions)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bond;
	Boolean				ok			= TRUE;

	if (!isA_SCBondInterface(bond)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if ((newOptions != NULL) && !isA_CFDictionary(newOptions)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	// set options in the stored preferences
	if (interfacePrivate->prefs != NULL) {
		CFDictionaryRef		dict;
		CFMutableDictionaryRef	newDict;
		CFStringRef		path;

		path = CFStringCreateWithFormat(NULL,
						NULL,
						CFSTR("/%@/%@/%@"),
						kSCPrefVirtualNetworkInterfaces,
						kSCNetworkInterfaceTypeBond,
						interfacePrivate->entity_device);
		dict = SCPreferencesPathGetValue(interfacePrivate->prefs, path);
		if (!isA_CFDictionary(dict)) {
			// if the prefs are confused
			CFRelease(path);
			_SCErrorSet(kSCStatusFailed);
			return FALSE;
		}

		newDict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
		if (newOptions != NULL) {
			CFDictionarySetValue(newDict, kSCPropVirtualNetworkInterfacesBondOptions, newOptions);
		} else {
			CFDictionaryRemoveValue(newDict, kSCPropVirtualNetworkInterfacesBondOptions);
		}
		if (!CFEqual(dict, newDict)) {
			ok = SCPreferencesPathSetValue(interfacePrivate->prefs, path, newDict);
		}
		CFRelease(newDict);
		CFRelease(path);
	}

	// set options in the SCBondInterfaceRef
	if (ok) {
		if (interfacePrivate->bond.options != NULL) {
			CFRelease(interfacePrivate->bond.options);
			interfacePrivate->bond.options = NULL;
		}
		if (newOptions != NULL) {
			interfacePrivate->bond.options = CFDictionaryCreateCopy(NULL, newOptions);
		}
	}

	return ok;
}


static Boolean
_SCBondInterfaceSetMode(SCBondInterfaceRef bond, CFNumberRef mode)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bond;
	Boolean				needs_release 		= FALSE;
	Boolean				ok			= TRUE;

	assert(bond != NULL);

	if (mode == NULL) {
		int	mode_num	= IF_BOND_MODE_LACP;

		mode = CFNumberCreate(NULL, kCFNumberIntType, &mode_num);
		needs_release = TRUE;
	}

	// set mode in the stored preferences
	if (interfacePrivate->prefs != NULL) {
		CFDictionaryRef		dict;
		CFMutableDictionaryRef	newDict;
		CFStringRef		path;

		path = CFStringCreateWithFormat(NULL,
						NULL,
						CFSTR("/%@/%@/%@"),
						kSCPrefVirtualNetworkInterfaces,
						kSCNetworkInterfaceTypeBond,
						interfacePrivate->entity_device);
		dict = SCPreferencesPathGetValue(interfacePrivate->prefs, path);
		if (!isA_CFDictionary(dict)) {
			// if the prefs are confused
			CFRelease(path);
			_SCErrorSet(kSCStatusFailed);
			ok = FALSE;
			goto done;
		}
		newDict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
		CFDictionarySetValue(newDict, kSCPropVirtualNetworkInterfacesBondMode, mode);
		if (!CFEqual(dict, newDict)) {
			ok = SCPreferencesPathSetValue(interfacePrivate->prefs, path, newDict);
		}
		CFRelease(newDict);
		CFRelease(path);
	}

	if (ok) {
		CFRetain(mode);
		if (interfacePrivate->bond.mode != NULL) {
			CFRelease(interfacePrivate->bond.mode);
		}
		interfacePrivate->bond.mode = mode;
	}

    done :

	if (needs_release) CFRelease(mode);
	return ok;
}

Boolean
SCBondInterfaceSetMode(SCBondInterfaceRef bond, CFNumberRef mode)
{
	int	mode_num;

	if (!isA_SCBondInterface(bond) || !isA_CFNumber(mode)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (CFNumberGetValue(mode, kCFNumberIntType, &mode_num) == FALSE) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	switch (mode_num) {
		case IF_BOND_MODE_LACP:
		case IF_BOND_MODE_STATIC:
			break;
		default:
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
	}

	return (_SCBondInterfaceSetMode(bond, mode));
}

CFNumberRef
SCBondInterfaceGetMode(SCBondInterfaceRef bond)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bond;

	if (!isA_SCBondInterface(bond)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}
	return (interfacePrivate->bond.mode);
}


#pragma mark -
#pragma mark SCBondStatus APIs


typedef struct {

	// base CFType information
	CFRuntimeBase			cfBase;

	// bond status
	SCBondInterfaceRef		bond;
	CFDictionaryRef			status_bond;

	// member interfaces and status
	CFArrayRef			interfaces;		// of SCNetworkInterfaceRef's
	CFDictionaryRef			status_interfaces;	// key = interface, val = interface status)

} SCBondStatusPrivate, * SCBondStatusPrivateRef;


const CFStringRef kSCBondStatusDeviceAggregationStatus	= CFSTR("AggregationStatus");
const CFStringRef kSCBondStatusDeviceCollecting		= CFSTR("Collecting");
const CFStringRef kSCBondStatusDeviceDistributing	= CFSTR("Distributing");


static CFStringRef	__SCBondStatusCopyDescription	(CFTypeRef cf);
static void		__SCBondStatusDeallocate	(CFTypeRef cf);
static Boolean		__SCBondStatusEqual		(CFTypeRef cf1, CFTypeRef cf2);


static const CFRuntimeClass __SCBondStatusClass = {
	0,				// version
	"BondStatus",			// className
	NULL,				// init
	NULL,				// copy
	__SCBondStatusDeallocate,	// dealloc
	__SCBondStatusEqual,		// equal
	NULL,				// hash
	NULL,				// copyFormattingDesc
	__SCBondStatusCopyDescription	// copyDebugDesc
};


static CFTypeID		__kSCBondStatusTypeID	= _kCFRuntimeNotATypeID;


static pthread_once_t	bondStatus_init		= PTHREAD_ONCE_INIT;


static CFStringRef
__SCBondStatusCopyDescription(CFTypeRef cf)
{
	CFAllocatorRef		allocator	= CFGetAllocator(cf);
	CFMutableStringRef	result;
	SCBondStatusPrivateRef	statusPrivate	= (SCBondStatusPrivateRef)cf;

	result = CFStringCreateMutable(allocator, 0);
	CFStringAppendFormat(result, NULL, CFSTR("<SCBondStatus %p [%p]> {"), cf, allocator);
	CFStringAppendFormat(result, NULL, CFSTR(" bond = %@"), statusPrivate->bond);
	CFStringAppendFormat(result, NULL, CFSTR(", interface = %@"), statusPrivate->status_bond);
	CFStringAppendFormat(result, NULL, CFSTR(", members = %@"),   statusPrivate->status_interfaces);
	CFStringAppendFormat(result, NULL, CFSTR(" }"));

	return result;
}


static void
__SCBondStatusDeallocate(CFTypeRef cf)
{
	SCBondStatusPrivateRef	statusPrivate	= (SCBondStatusPrivateRef)cf;

	/* release resources */

	CFRelease(statusPrivate->bond);
	CFRelease(statusPrivate->status_bond);
	if (statusPrivate->interfaces != NULL) CFRelease(statusPrivate->interfaces);
	CFRelease(statusPrivate->status_interfaces);
	return;
}


static Boolean
__SCBondStatusEqual(CFTypeRef cf1, CFTypeRef cf2)
{
	SCBondStatusPrivateRef	status1	= (SCBondStatusPrivateRef)cf1;
	SCBondStatusPrivateRef	status2	= (SCBondStatusPrivateRef)cf2;

	if (status1 == status2)
		return TRUE;

	if (!CFEqual(status1->bond, status2->bond))
		return FALSE;	// if not the same bond

	if (!CFEqual(status1->status_bond, status2->status_bond))
		return FALSE;	// if not the same interface status

	if (!CFEqual(status1->status_interfaces, status2->status_interfaces))
		return FALSE;	// if not the same status of the member interfaces

	return TRUE;
}


static void
__SCBondStatusInitialize(void)
{
	__kSCBondStatusTypeID = _CFRuntimeRegisterClass(&__SCBondStatusClass);
	return;
}


static SCBondStatusRef
__SCBondStatusCreatePrivate(CFAllocatorRef	allocator,
			    SCBondInterfaceRef	bond,
			    CFDictionaryRef	status_bond,
			    CFDictionaryRef	status_interfaces)
{
	SCBondStatusPrivateRef	statusPrivate;
	uint32_t		size;

	/* initialize runtime */
	pthread_once(&bondStatus_init, __SCBondStatusInitialize);

	/* allocate bond */
	size          = sizeof(SCBondStatusPrivate) - sizeof(CFRuntimeBase);
	statusPrivate = (SCBondStatusPrivateRef)_CFRuntimeCreateInstance(allocator,
									 __kSCBondStatusTypeID,
									 size,
									 NULL);
	if (statusPrivate == NULL) {
		return NULL;
	}

	/* establish the bond status */

	statusPrivate->bond		 = CFRetain(bond);
	statusPrivate->status_bond       = CFDictionaryCreateCopy(NULL, status_bond);

	statusPrivate->interfaces        = NULL;
	statusPrivate->status_interfaces = CFDictionaryCreateCopy(NULL, status_interfaces);

	return (SCBondStatusRef)statusPrivate;
}


static __inline__ CFTypeRef
isA_SCBondStatus(CFTypeRef obj)
{
	return (isA_CFType(obj, SCBondStatusGetTypeID()));
}


CFTypeID
SCBondStatusGetTypeID()
{
	pthread_once(&bondStatus_init, __SCBondStatusInitialize);	/* initialize runtime */
	return __kSCBondStatusTypeID;
}


#define	N_QUICK	16


CFArrayRef /* of SCNetworkInterfaceRef's */
SCBondStatusGetMemberInterfaces(SCBondStatusRef bondStatus)
{
	SCBondStatusPrivateRef	statusPrivate	= (SCBondStatusPrivateRef)bondStatus;

	if (!isA_SCBondStatus(bondStatus)) {
		return NULL;
	}

	if (statusPrivate->interfaces == NULL) {
		const void *	keys_q[N_QUICK];
		const void **	keys	= keys_q;
		CFIndex		n;

		n = CFDictionaryGetCount(statusPrivate->status_interfaces);
		if (n > (CFIndex)(sizeof(keys_q) / sizeof(CFTypeRef))) {
			keys = CFAllocatorAllocate(NULL, n * sizeof(CFTypeRef), 0);
		}
		CFDictionaryGetKeysAndValues(statusPrivate->status_interfaces, keys, NULL);
		statusPrivate->interfaces = CFArrayCreate(NULL, keys, n, &kCFTypeArrayCallBacks);
		if (keys != keys_q) {
			CFAllocatorDeallocate(NULL, keys);
		}
	}

	return statusPrivate->interfaces;
}


CFDictionaryRef
SCBondStatusGetInterfaceStatus(SCBondStatusRef bondStatus, SCNetworkInterfaceRef interface)
{
	CFDictionaryRef		status		= NULL;
	SCBondStatusPrivateRef	statusPrivate	= (SCBondStatusPrivateRef)bondStatus;

	if (!isA_SCBondStatus(bondStatus)) {
		return NULL;
	}

	if (interface == NULL) {
		// return status of the bond
		status = statusPrivate->status_bond;
	} else {
		// return status of the member interface
		status = CFDictionaryGetValue(statusPrivate->status_interfaces, interface);
	}

	return status;
}


SCBondStatusRef
SCBondInterfaceCopyStatus(SCBondInterfaceRef bond)
{
	int				bond_if_active;
	int				bond_if_status;
	CFIndex				i;
	struct if_bond_status_req	*ibsr_p		= NULL;
	char				if_name[IFNAMSIZ];
	CFIndex				n;
	CFNumberRef			num;
	int				s;
	struct if_bond_status		*scan_p;
	SCBondStatusRef			status		= NULL;
	CFMutableDictionaryRef		status_bond;
	CFMutableDictionaryRef		status_interfaces;

	if (!isA_SCBondInterface(bond)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	s = inet_dgram_socket();
	if (s == -1) {
		_SCErrorSet(errno);
		goto done;
	}

	_SC_cfstring_to_cstring(SCNetworkInterfaceGetBSDName(bond),
				if_name,
				sizeof(if_name),
				kCFStringEncodingASCII);
	if (siocgifmedia(s, if_name, &bond_if_status, &bond_if_active) == -1) {
		_SCErrorSet(errno);
		switch (errno) {
			case EBUSY :
			case ENXIO :
				break;
			default :
				SCLog(TRUE, LOG_ERR,
				      CFSTR("siocgifmedia(%s) failed: %s"),
				      if_name,
				      strerror(errno));
		}
		goto done;
	}
	ibsr_p = if_bond_status_req_copy(s, if_name);
	if (ibsr_p == NULL) {
		_SCErrorSet(errno);
		goto done;
	}

	status_bond = CFDictionaryCreateMutable(NULL,
						0,
						&kCFTypeDictionaryKeyCallBacks,
						&kCFTypeDictionaryValueCallBacks);

	status_interfaces = CFDictionaryCreateMutable(NULL,
						      0,
						      &kCFTypeDictionaryKeyCallBacks,
						      &kCFTypeDictionaryValueCallBacks);
	n = ibsr_p->ibsr_total;
	for (i = 0, scan_p = (struct if_bond_status *)ibsr_p->ibsr_buffer; i < n; i++, scan_p++) {
		int				collecting	= 0;
		int				distributing	= 0;
		SCNetworkInterfaceRef		interface;
		CFStringRef			interface_name;
		struct if_bond_partner_state *	ps;
		CFMutableDictionaryRef		status_interface;
		int				status_val;

		ps = &scan_p->ibs_partner_state;

		if (lacp_actor_partner_state_in_sync(scan_p->ibs_state)) {
			/* we're in-sync */
			status_val = kSCBondStatusOK;
			if (lacp_actor_partner_state_in_sync(ps->ibps_state)) {
				/* partner is also in-sync */
				if (lacp_actor_partner_state_collecting(scan_p->ibs_state)
				    && lacp_actor_partner_state_distributing(ps->ibps_state)) {
					/* we're able to collect (receive) frames */
					collecting = 1;
				}
				if (lacp_actor_partner_state_distributing(scan_p->ibs_state)
				    && lacp_actor_partner_state_collecting(ps->ibps_state)) {
					/* we're able to distribute (transmit) frames */
					distributing = 1;
				}
			}
		} else {
			int			active = 0;
			int			status = 0;
			static lacp_system	zeroes = { {0, 0, 0, 0, 0, 0}};

			if (siocgifmedia(s, scan_p->ibs_if_name, &status, &active) == -1) {
				switch (errno) {
					case EBUSY :
					case ENXIO :
						break;
					default :
						SCLog(TRUE, LOG_ERR,
						      CFSTR("siocgifmedia(%s) failed: %s"),
						      if_name,
						      strerror(errno));
						break;
				}
			}
			if (((status & IFM_AVALID) == 0) ||
			    ((status & IFM_ACTIVE) == 0) ||
			    ((active & IFM_FDX   ) == 0)) {
				/* link down or not full-duplex */
				status_val = kSCBondStatusLinkInvalid;
			} else if ((ps->ibps_system_priority == 0) &&
				   (bcmp(&zeroes, &ps->ibps_system, sizeof(zeroes)) == 0)) {
				/* no one on the other end of the link */
				status_val = kSCBondStatusNoPartner;
			} else if (active != bond_if_active) {
				/* the link speed was different */
				status_val = kSCBondStatusLinkInvalid;
			} else {
				/* partner is not in the active group */
				status_val = kSCBondStatusNotInActiveGroup;
			}
		}

		// interface
		strlcpy(if_name, scan_p->ibs_if_name, sizeof(if_name));
		interface_name = CFStringCreateWithCString(NULL, if_name, kCFStringEncodingASCII);
		interface = _SCNetworkInterfaceCreateWithBSDName(NULL, interface_name,
								 kIncludeNoVirtualInterfaces);
		CFRelease(interface_name);

		// interface status
		status_interface = CFDictionaryCreateMutable(NULL,
							     0,
							     &kCFTypeDictionaryKeyCallBacks,
							     &kCFTypeDictionaryValueCallBacks);
		num = CFNumberCreate(NULL, kCFNumberIntType, &status_val);
		CFDictionarySetValue(status_interface, kSCBondStatusDeviceAggregationStatus, num);
		CFRelease(num);
		num = CFNumberCreate(NULL, kCFNumberIntType, &collecting);
		CFDictionarySetValue(status_interface, kSCBondStatusDeviceCollecting, num);
		CFRelease(num);
		num = CFNumberCreate(NULL, kCFNumberIntType, &distributing);
		CFDictionarySetValue(status_interface, kSCBondStatusDeviceDistributing, num);
		CFRelease(num);

		CFDictionarySetValue(status_interfaces, interface, status_interface);
		CFRelease(interface);
		CFRelease(status_interface);
	}

	status = __SCBondStatusCreatePrivate(NULL, bond, status_bond, status_interfaces);
	CFRelease(status_bond);
	CFRelease(status_interfaces);

    done:

	if (s != -1) {
		close(s);
	}
	if (ibsr_p != NULL) {
		free(ibsr_p);
	}
	return (SCBondStatusRef)status;
}


#pragma mark -
#pragma mark SCBondInterface management


static Boolean
__bond_set_mode(int s, CFStringRef bond_if, CFNumberRef mode)
{
	struct if_bond_req	breq;
	struct ifreq		ifr;
	int					mode_num;

	mode_num = IF_BOND_MODE_LACP;
	if (mode != NULL) {
		CFNumberGetValue(mode, kCFNumberIntType, &mode_num);
	}

	// bond interface
	bzero(&ifr, sizeof(ifr));
	(void) _SC_cfstring_to_cstring(bond_if,
				       ifr.ifr_name,
				       sizeof(ifr.ifr_name),
				       kCFStringEncodingASCII);
	ifr.ifr_data = (caddr_t)&breq;
	bzero(&breq, sizeof(breq));
	breq.ibr_op = IF_BOND_OP_SET_MODE;
	breq.ibr_ibru.ibru_int_val = mode_num;
	if (ioctl(s, SIOCSIFBOND, (caddr_t)&ifr) == -1) {
		_SCErrorSet(errno);
		SCLog(TRUE, LOG_ERR,
		      CFSTR("could not set mode to %@ on bond \"%@\": %s"),
		      mode,
		      bond_if,
		      strerror(errno));
		return FALSE;
	}

	return TRUE;
}

static Boolean
__bond_add_interface(int s, CFStringRef bond_if, CFStringRef interface_if)
{
	struct if_bond_req	breq;
	struct ifreq		ifr;

	// bond interface
	bzero(&ifr, sizeof(ifr));
	(void) _SC_cfstring_to_cstring(bond_if,
				       ifr.ifr_name,
				       sizeof(ifr.ifr_name),
				       kCFStringEncodingASCII);
	ifr.ifr_data = (caddr_t)&breq;

	// new bond member
	bzero(&breq, sizeof(breq));
	breq.ibr_op = IF_BOND_OP_ADD_INTERFACE;
	(void) _SC_cfstring_to_cstring(interface_if,
				       breq.ibr_ibru.ibru_if_name,
				       sizeof(breq.ibr_ibru.ibru_if_name),
				       kCFStringEncodingASCII);

	// add new bond member
	if (ioctl(s, SIOCSIFBOND, (caddr_t)&ifr) == -1) {
		_SCErrorSet(errno);
		SCLog(TRUE, LOG_ERR,
		      CFSTR("could not add interface \"%@\" to bond \"%@\": %s"),
		      interface_if,
		      bond_if,
		      strerror(errno));
		return FALSE;
	}

	return TRUE;
}


static Boolean
__bond_remove_interface(int s, CFStringRef bond_if, CFStringRef interface_if)
{
	struct if_bond_req	breq;
	struct ifreq		ifr;

	// bond interface
	bzero(&ifr, sizeof(ifr));
	(void) _SC_cfstring_to_cstring(bond_if,
				       ifr.ifr_name,
				       sizeof(ifr.ifr_name),
				       kCFStringEncodingASCII);
	ifr.ifr_data = (caddr_t)&breq;

	// bond member to remove
	bzero(&breq, sizeof(breq));
	breq.ibr_op = IF_BOND_OP_REMOVE_INTERFACE;
	(void) _SC_cfstring_to_cstring(interface_if,
				       breq.ibr_ibru.ibru_if_name,
				       sizeof(breq.ibr_ibru.ibru_if_name),
				       kCFStringEncodingASCII);

	// remove bond member
	if (ioctl(s, SIOCSIFBOND, (caddr_t)&ifr) == -1) {
		_SCErrorSet(errno);
		SCLog(TRUE, LOG_ERR,
		      CFSTR("could not remove interface \"%@\" from bond \"%@\": %s"),
		      interface_if,
		      bond_if,
		      strerror(errno));
		return FALSE;
	}

	return TRUE;
}


Boolean
_SCBondInterfaceUpdateConfiguration(SCPreferencesRef prefs)
{
	CFArrayRef			active		= NULL;
	CFArrayRef			config		= NULL;
	CFIndex				i;
	CFIndex				nActive;
	CFIndex				nConfig;
	Boolean				ok		= TRUE;
	int				s		= -1;

	if (prefs == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	/* configured Bonds */
	config  = SCBondInterfaceCopyAll(prefs);
	nConfig = (config != NULL) ? CFArrayGetCount(config) : 0;

	/* active Bonds */
	active  = _SCBondInterfaceCopyActive();
	nActive = (active != NULL) ? CFArrayGetCount(active) : 0;

	/*
	 * remove any no-longer-configured bond interfaces and
	 * any devices associated with a bond that are no longer
	 * associated with a bond.
	 */
	for (i = 0; i < nActive; i++) {
		SCBondInterfaceRef	a_bond;
		CFStringRef		a_bond_if;
		CFIndex			j;
		Boolean			found	= FALSE;

		a_bond    = CFArrayGetValueAtIndex(active, i);
		a_bond_if = SCNetworkInterfaceGetBSDName(a_bond);

		for (j = 0; j < nConfig; j++) {
			SCBondInterfaceRef	c_bond;
			CFStringRef		c_bond_if;

			c_bond    = CFArrayGetValueAtIndex(config, j);
			c_bond_if = SCNetworkInterfaceGetBSDName(c_bond);

			if (CFEqual(a_bond_if, c_bond_if)) {
				CFIndex		a;
				CFArrayRef	a_bond_interfaces;
				CFIndex		a_count;
				CFArrayRef	c_bond_interfaces;
				CFIndex		c_count;

				c_bond_interfaces = SCBondInterfaceGetMemberInterfaces(c_bond);
				c_count           = (c_bond_interfaces != NULL) ? CFArrayGetCount(c_bond_interfaces) : 0;

				a_bond_interfaces = SCBondInterfaceGetMemberInterfaces(a_bond);
				a_count           = (a_bond_interfaces != NULL) ? CFArrayGetCount(a_bond_interfaces) : 0;

				for (a = 0; a < a_count; a++) {
					SCNetworkInterfaceRef	a_interface;
					CFStringRef		a_interface_if;

					a_interface = CFArrayGetValueAtIndex(a_bond_interfaces, a);
					if ((c_count == 0) ||
					    !CFArrayContainsValue(c_bond_interfaces,
								  CFRangeMake(0, c_count),
								  a_interface)) {
						/*
						 * if this device is no longer part
						 * of the bond.
						 */
						if (s == -1) {
							s = inet_dgram_socket();
							if (s == -1) {
								_SCErrorSet(errno);
								ok = FALSE;
								goto done;
							}
						}

						a_interface_if = SCNetworkInterfaceGetBSDName(a_interface);
						if (!__bond_remove_interface(s, a_bond_if, a_interface_if)) {
							ok = FALSE;
						}
					}
				}

				found = TRUE;
				break;
			}
		}

		if (!found) {
			/*
			 * if this interface is no longer configured
			 */
			if (s == -1) {
				s = inet_dgram_socket();
				if (s == -1) {
					_SCErrorSet(errno);
					ok = FALSE;
					goto done;
				}
			}

			if (!__destroyInterface(s, a_bond_if)) {
				_SCErrorSet(errno);
				ok = FALSE;
			}
		}
	}

	/*
	 * add any newly-configured bond interfaces and add any
	 * devices that should now be associated with the bond.
	 */
	for (i = 0; i < nConfig; i++) {
		CFNumberRef		c_bond_mode;
		SCBondInterfaceRef	c_bond;
		CFArrayRef		c_bond_interfaces;
		CFStringRef		c_bond_if;
		CFIndex			c_count;
		Boolean			found		= FALSE;
		CFIndex			j;

		c_bond            = CFArrayGetValueAtIndex(config, i);
		c_bond_if         = SCNetworkInterfaceGetBSDName(c_bond);
		c_bond_interfaces = SCBondInterfaceGetMemberInterfaces(c_bond);
		c_bond_mode       = SCBondInterfaceGetMode(c_bond);
		c_count           = (c_bond_interfaces != NULL) ? CFArrayGetCount(c_bond_interfaces) : 0;

		for (j = 0; j < nActive; j++) {
			SCBondInterfaceRef	a_bond;
			CFArrayRef		a_bond_interfaces;
			CFNumberRef		a_bond_mode;
			CFStringRef		a_bond_if;
			CFIndex			a_count;

			a_bond            = CFArrayGetValueAtIndex(active, j);
			a_bond_if         = SCNetworkInterfaceGetBSDName(a_bond);
			a_bond_interfaces = SCBondInterfaceGetMemberInterfaces(a_bond);
			a_bond_mode		  = SCBondInterfaceGetMode(a_bond);
			a_count           = (a_bond_interfaces != NULL) ? CFArrayGetCount(a_bond_interfaces) : 0;

			if (CFEqual(c_bond_if, a_bond_if)) {
				CFIndex	c;
				Boolean	if_list_change = FALSE;
				Boolean mode_change = FALSE;

				found = TRUE;

				if (!_SC_CFEqual(a_bond_mode, c_bond_mode)) {
					mode_change = TRUE;
				}

				if (!_SC_CFEqual(c_bond_interfaces, a_bond_interfaces)) {
					if_list_change = TRUE;
				}
				if (!mode_change && !if_list_change) {
					break;	// if no change
				}
				if (s == -1) {
					s = inet_dgram_socket();
					if (s == -1) {
						_SCErrorSet(errno);
						ok = FALSE;
						goto done;
					}
				}
				if (mode_change) {
					__bond_set_mode(s, a_bond_if, c_bond_mode);
				}
				if (!if_list_change) {
					break; // no if list changes
				}

				/*
				 * ensure that the first device of the bond matches, if
				 * not then we remove all current devices and add them
				 * back in the preferred order.
				 */
				if ((c_count > 0) &&
				    (a_count > 0) &&
				    !CFEqual(CFArrayGetValueAtIndex(c_bond_interfaces, 0),
					     CFArrayGetValueAtIndex(a_bond_interfaces, 0))) {
					CFIndex	a;

					for (a = 0; a < a_count; a++) {
						SCNetworkInterfaceRef	a_interface;
						CFStringRef		a_interface_if;

						a_interface = CFArrayGetValueAtIndex(a_bond_interfaces, a);
						if (!CFArrayContainsValue(c_bond_interfaces,
									 CFRangeMake(0, c_count),
									 a_interface)) {
							continue;	// if already removed
						}

						a_interface_if = SCNetworkInterfaceGetBSDName(a_interface);
						if (!__bond_remove_interface(s, a_bond_if, a_interface_if)) {
							ok = FALSE;
						}
					}

					a_count = 0;	// all active devices have been removed
				}

				/*
				 * add any devices which are not currently associated
				 * with the bond interface.
				 */
				for (c = 0; c < c_count; c++) {
					SCNetworkInterfaceRef		c_interface;
					SCNetworkInterfacePrivateRef	c_interfacePrivate;
					CFStringRef			c_interface_if;

					c_interface = CFArrayGetValueAtIndex(c_bond_interfaces, c);
					if ((a_count == 0) ||
					    !CFArrayContainsValue(a_bond_interfaces,
								  CFRangeMake(0, a_count),
								  c_interface)) {
						/*
						 * check if this member interface can be added to a bond.
						 */
						c_interfacePrivate = (SCNetworkInterfacePrivateRef)c_interface;
						if (!c_interfacePrivate->supportsBond) {
							// if member not supported
							continue;
						}

						/*
						 * if this member interface is not currently part of the bond.
						 */
						c_interface_if = SCNetworkInterfaceGetBSDName(c_interface);
						if (!__bond_add_interface(s, c_bond_if, c_interface_if)) {
							// if member could not be added
							ok = FALSE;
						}
					}
				}

				break;
			}
		}

		if (!found) {
			CFIndex	c;

			if (s == -1) {
				s = inet_dgram_socket();
				if (s == -1) {
					_SCErrorSet(errno);
					ok = FALSE;
					goto done;
				}
			}

			/*
			 * establish the new bond interface.
			 */
			if (!__createInterface(s, c_bond_if)) {
				_SCErrorSet(errno);
				ok = FALSE;
				continue;
			}

			/* set the mode */
			__bond_set_mode(s, c_bond_if, c_bond_mode);

			/*
			 * add the member interfaces
			 */
			for (c = 0; c < c_count; c++) {
				SCNetworkInterfaceRef		c_interface;
				SCNetworkInterfacePrivateRef	c_interfacePrivate;
				CFStringRef			c_interface_if;

				c_interface = CFArrayGetValueAtIndex(c_bond_interfaces, c);
				c_interfacePrivate = (SCNetworkInterfacePrivateRef)c_interface;
				if (!c_interfacePrivate->supportsBond) {
					// if member not supported
					continue;
				}

				c_interface_if = SCNetworkInterfaceGetBSDName(c_interface);
				if (!__bond_add_interface(s, c_bond_if, c_interface_if)) {
					// if member could not be added
					ok = FALSE;
				}
			}
		}

	}

    done :

	if (active != NULL)	CFRelease(active);
	if (config != NULL)	CFRelease(config);
	if (s != -1)		(void) close(s);

	return ok;
}

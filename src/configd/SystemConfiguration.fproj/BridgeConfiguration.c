/*
 * Copyright (c) 2009-2014 Apple Inc. All rights reserved.
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
 * July 27, 2009		Allan Nathanson <ajn@apple.com>
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
#include <net/if_types.h>
#include <net/if_media.h>
#include <net/route.h>

#ifdef	IFT_BRIDGE
#include <net/if_bridgevar.h>
#endif	// IFT_BRIDGE

/* ---------- Bridge support ---------- */

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

#ifdef	IFT_BRIDGE
static struct ifbifconf *
ifbifconf_copy(int s, const char * ifname)
{
	void *			buf;
	size_t			buflen;
	struct ifbifconf *	ibc_p	= NULL;
	struct ifdrv		ifd;
	uint32_t		len	= sizeof(struct ifbreq) * 16;

	bzero(&ifd, sizeof(ifd));
	strncpy(ifd.ifd_name, ifname, sizeof(ifd.ifd_name));
	ifd.ifd_cmd = BRDGGIFS;

	buflen = sizeof(struct ifbifconf) + len;
	buf = malloc(buflen);
	while (buf != NULL) {
		bzero(buf, buflen);
		ibc_p = (struct ifbifconf *)buf;
		ibc_p->ifbic_len = len;
		ibc_p->ifbic_buf = buf + sizeof(*ibc_p);

		ifd.ifd_len = sizeof(*ibc_p);
		ifd.ifd_data = ibc_p;
		if (ioctl(s, SIOCGDRVSPEC, (caddr_t)&ifd) == -1) {
			goto failed;
		}

		if ((ibc_p->ifbic_len + sizeof(struct ifbreq)) < len) {
			// if we have room for all of the member interfaces
			break;
		}

		len *= 2;
		buflen = sizeof(struct ifbifconf) + len;
		buf = reallocf(buf, buflen);
	}

	if (buf == NULL) {
		goto failed;
	}

	return ibc_p;

    failed:
	if (buf != NULL) {
		free(buf);
	}
	return NULL;
}
#endif	// IFT_BRIDGE


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

typedef struct {
	CFMutableArrayRef	bridges;
	SCPreferencesRef	ni_prefs;
	SCPreferencesRef	prefs;
} addContext, *addContextRef;


static void
add_configured_interface(const void *key, const void *value, void *context)
{
	SCBridgeInterfaceRef		bridge;
	CFStringRef			bridge_if	= (CFStringRef)key;
	CFDictionaryRef			bridge_info	= (CFDictionaryRef)value;
	CFDictionaryRef			bridge_options;
	CFIndex				i;
	CFArrayRef			interfaces;
	SCNetworkInterfacePrivateRef	interfacePrivate;
	CFMutableArrayRef		members		= NULL;
	addContextRef			myContext	= (addContextRef)context;
	CFStringRef			name;
	CFStringRef			name_auto	= NULL;
	CFIndex				n;

	// create the bridge interface
	bridge = (SCBridgeInterfaceRef)_SCBridgeInterfaceCreatePrivate(NULL, bridge_if);
	assert(bridge != NULL);

	// estabish link to the stored configuration
	interfacePrivate = (SCNetworkInterfacePrivateRef)bridge;
	interfacePrivate->prefs = CFRetain(myContext->prefs);

	// add member interfaces
	interfaces = CFDictionaryGetValue(bridge_info, kSCPropVirtualNetworkInterfacesBridgeInterfaces);
	n = isA_CFArray(interfaces) ? CFArrayGetCount(interfaces) : 0;
	for (i = 0; i < n; i++) {
		CFStringRef	member;

		member = CFArrayGetValueAtIndex(interfaces, i);
		if (isA_CFString(member)) {
			add_interface(&members, member, myContext->ni_prefs);
		}
	}
	if (members != NULL) {
		__SCBridgeInterfaceSetMemberInterfaces(bridge, members);
		CFRelease(members);
	}

	// set options
	bridge_options = CFDictionaryGetValue(bridge_info, kSCPropVirtualNetworkInterfacesBridgeOptions);
	if (isA_CFDictionary(bridge_options)) {
		SCBridgeInterfaceSetOptions(bridge, bridge_options);
		name_auto = CFDictionaryGetValue(bridge_options, CFSTR("__AUTO__"));
	}

	// set display name
	name = CFDictionaryGetValue(bridge_info, kSCPropUserDefinedName);
	if (isA_CFString(name)) {
		SCBridgeInterfaceSetLocalizedDisplayName(bridge, name);
	} else if (isA_CFString(name_auto)) {
		interfacePrivate->localized_key = name_auto;
		if (interfacePrivate->localized_arg1 != NULL) {
			CFRelease(interfacePrivate->localized_arg1);
			interfacePrivate->localized_arg1 = NULL;
		}
	}

	CFArrayAppendValue(myContext->bridges, bridge);
	CFRelease(bridge);

	return;
}


#pragma mark -
#pragma mark SCBridgeInterface APIs


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
SCBridgeInterfaceCopyAll(SCPreferencesRef prefs)
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
	context.bridges = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	context.prefs = prefs;
	context.ni_prefs = ni_prefs;
	
	path = CFStringCreateWithFormat(NULL,
					NULL,
					CFSTR("/%@/%@"),
					kSCPrefVirtualNetworkInterfaces,
					kSCNetworkInterfaceTypeBridge);
	dict = SCPreferencesPathGetValue(prefs, path);
	if (isA_CFDictionary(dict)) {
		my_CFDictionaryApplyFunction(dict, add_configured_interface, &context);
	}
	CFRelease(path);
	if (ni_prefs != NULL) {
		CFRelease(ni_prefs);
	}
	return context.bridges;
}


__private_extern__ void
__SCBridgeInterfaceListCollectMembers(CFArrayRef interfaces, CFMutableSetRef set)
{
	CFIndex	i;
	CFIndex	n;

	n = CFArrayGetCount(interfaces);
	for (i = 0; i < n; i++) {
		SCBridgeInterfaceRef	bridgeInterface;
		CFArrayRef		members;

		bridgeInterface = CFArrayGetValueAtIndex(interfaces, i);
		members = SCBridgeInterfaceGetMemberInterfaces(bridgeInterface);
		if (members != NULL) {
			CFIndex	j;
			CFIndex	n_members;

			// exclude the member interfaces of this bridge
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
SCBridgeInterfaceCopyAvailableMemberInterfaces(SCPreferencesRef prefs)
{
	CFMutableArrayRef	available;
	CFMutableSetRef		excluded;
	CFArrayRef		interfaces;

	available = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	excluded  = CFSetCreateMutable  (NULL, 0, &kCFTypeSetCallBacks);

#if	!TARGET_OS_IPHONE
	// exclude Bond [member] interfaces
	interfaces = SCBondInterfaceCopyAll(prefs);
	if (interfaces != NULL) {
		__SCBondInterfaceListCollectMembers(interfaces, excluded);
		CFRelease(interfaces);
	}
#endif	// !TARGET_OS_IPHONE

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

			if (!interfacePrivate->supportsBridge) {
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
_SCBridgeInterfaceCopyActive(void)
{
	struct ifaddrs		*ifap;
	struct ifaddrs		*ifp;
	int			s;
	CFMutableArrayRef	bridges	= NULL;

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

	bridges = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	for (ifp = ifap; ifp != NULL; ifp = ifp->ifa_next) {
#ifdef	IFT_BRIDGE
		SCBridgeInterfaceRef		bridge;
		CFStringRef			bridge_if;
		struct ifbifconf		*ibc_p;
		struct if_data			*if_data;
		CFMutableArrayRef		members		= NULL;
		size_t				n;

		if_data = (struct if_data *)ifp->ifa_data;
		if (if_data == NULL
		    || ifp->ifa_addr->sa_family != AF_LINK
		    || if_data->ifi_type != IFT_BRIDGE) {
			continue;
		}

		// make sure that we leave non-SC configured bridge
		// interfaces (those with unit #'s >= 100) alone.
		n = strlen(ifp->ifa_name);
		if ((n > 3) &&
		    isdigit(ifp->ifa_name[n - 1]) &&
		    isdigit(ifp->ifa_name[n - 2]) &&
		    isdigit(ifp->ifa_name[n - 3])) {
			// if not SC managed bridge interface
			continue;
		}

		ibc_p = ifbifconf_copy(s, ifp->ifa_name);
		if (ibc_p == NULL) {
			if (errno == EBUSY) {
				continue;
			}
			_SCErrorSet(errno);
			SCLog(TRUE, LOG_ERR,
			      CFSTR("ifbifconf_copy(%s) failed: %s"),
			      ifp->ifa_name,
			      strerror(errno));
			CFRelease(bridges);
			bridges = NULL;
			goto done;
		}

		// create the bridge interface
		bridge_if = CFStringCreateWithCString(NULL, ifp->ifa_name, kCFStringEncodingASCII);
		bridge    = (SCBridgeInterfaceRef)_SCBridgeInterfaceCreatePrivate(NULL, bridge_if);
		CFRelease(bridge_if);

		// add member interfaces
		if (ibc_p->ifbic_len > 0) {
			int 		i;

			// iterate over each member interface
			for (i = 0; i < ibc_p->ifbic_len / sizeof(struct ifbreq); i++) {
				struct ifbreq	*ibr_p;
				CFStringRef	member;

				ibr_p = ibc_p->ifbic_req + i;
				member = CFStringCreateWithCString(NULL, ibr_p->ifbr_ifsname, kCFStringEncodingASCII);
				add_interface(&members, member, NULL);
				CFRelease(member);
			}
		}
		free(ibc_p);

		if (members != NULL) {
			__SCBridgeInterfaceSetMemberInterfaces(bridge, members);
			CFRelease(members);
		}

		// add bridge
		CFArrayAppendValue(bridges, bridge);
		CFRelease(bridge);
#endif	// IFT_BRIDGE
	}

    done :

	if (s != -1) {
		(void) close(s);
	}
	freeifaddrs(ifap);
	return bridges;
}


SCBridgeInterfaceRef
SCBridgeInterfaceCreate(SCPreferencesRef prefs)
{
	CFAllocatorRef		allocator;
	SCBridgeInterfaceRef	bridge		= NULL;
	CFIndex			i;

	if (prefs == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	allocator = CFGetAllocator(prefs);

	// create a new bridge using an unused interface name
	for (i = 0; bridge == NULL; i++) {
		CFDictionaryRef			dict;
		CFStringRef			bridge_if;
		SCNetworkInterfacePrivateRef	interfacePrivate;
		CFMutableDictionaryRef		newDict;
		CFArrayRef			newInterfaces;
		Boolean				ok;
		CFStringRef			path;

		bridge_if = CFStringCreateWithFormat(allocator, NULL, CFSTR("bridge%ld"), i);
		path    = CFStringCreateWithFormat(allocator,
						   NULL,
						   CFSTR("/%@/%@/%@"),
						   kSCPrefVirtualNetworkInterfaces,
						   kSCNetworkInterfaceTypeBridge,
						   bridge_if);
		dict = SCPreferencesPathGetValue(prefs, path);
		if (dict != NULL) {
			// if bridge interface name not available
			CFRelease(path);
			CFRelease(bridge_if);
			continue;
		}

		// add the bridge to the stored preferences
		newDict = CFDictionaryCreateMutable(allocator,
						    0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);
		newInterfaces = CFArrayCreate(allocator, NULL, 0, &kCFTypeArrayCallBacks);
		CFDictionaryAddValue(newDict, kSCPropVirtualNetworkInterfacesBridgeInterfaces, newInterfaces);
		CFRelease(newInterfaces);
		ok = SCPreferencesPathSetValue(prefs, path, newDict);
		CFRelease(newDict);
		CFRelease(path);
		if (!ok) {
			// if the bridge could not be saved
			CFRelease(bridge_if);
			break;
		}

		// create the SCBridgeInterfaceRef
		bridge = (SCBridgeInterfaceRef)_SCBridgeInterfaceCreatePrivate(allocator, bridge_if);
		CFRelease(bridge_if);

		// estabish link to the stored configuration
		interfacePrivate = (SCNetworkInterfacePrivateRef)bridge;
		interfacePrivate->prefs = CFRetain(prefs);
	}

	return bridge;
}


Boolean
SCBridgeInterfaceRemove(SCBridgeInterfaceRef bridge)
{
	CFStringRef			bridge_if;
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bridge;
	Boolean				ok;
	CFStringRef			path;

	if (!isA_SCBridgeInterface(bridge)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (interfacePrivate->prefs == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	bridge_if = SCNetworkInterfaceGetBSDName(bridge);
	path    = CFStringCreateWithFormat(NULL,
					   NULL,
					   CFSTR("/%@/%@/%@"),
					   kSCPrefVirtualNetworkInterfaces,
					   kSCNetworkInterfaceTypeBridge,
					   bridge_if);
	ok = SCPreferencesPathRemoveValue(interfacePrivate->prefs, path);
	CFRelease(path);

	return ok;
}


CFArrayRef
SCBridgeInterfaceGetMemberInterfaces(SCBridgeInterfaceRef bridge)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bridge;

	if (!isA_SCBridgeInterface(bridge)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	return interfacePrivate->bridge.interfaces;
}


CFDictionaryRef
SCBridgeInterfaceGetOptions(SCBridgeInterfaceRef bridge)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bridge;

	if (!isA_SCBridgeInterface(bridge)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	return interfacePrivate->bridge.options;
}


__private_extern__
Boolean
__SCBridgeInterfaceSetMemberInterfaces(SCBridgeInterfaceRef bridge, CFArrayRef members)
{
	CFIndex				i;
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bridge;
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
						kSCNetworkInterfaceTypeBridge,
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
		CFDictionarySetValue(newDict, kSCPropVirtualNetworkInterfacesBridgeInterfaces, newMembers);
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
		CFRelease(interfacePrivate->bridge.interfaces);
		interfacePrivate->bridge.interfaces = newMembers;
	}

	return ok;
}


Boolean
SCBridgeInterfaceSetMemberInterfaces(SCBridgeInterfaceRef bridge, CFArrayRef members)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bridge;
	Boolean				ok;
	int				sc_status		= kSCStatusOK;

	if (!isA_SCBridgeInterface(bridge)) {
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

		current     = SCBridgeInterfaceGetMemberInterfaces(bridge);
		n_current   = (current != NULL) ? CFArrayGetCount(current) : 0;

		available   = SCBridgeInterfaceCopyAvailableMemberInterfaces(interfacePrivate->prefs);
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

	ok = __SCBridgeInterfaceSetMemberInterfaces(bridge, members);
	return ok;
}


Boolean
SCBridgeInterfaceSetLocalizedDisplayName(SCBridgeInterfaceRef bridge, CFStringRef newName)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bridge;
	Boolean				ok			= TRUE;

	if (!isA_SCBridgeInterface(bridge)) {
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
						kSCNetworkInterfaceTypeBridge,
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

	// set name in the SCBridgeInterfaceRef
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
SCBridgeInterfaceSetOptions(SCBridgeInterfaceRef bridge, CFDictionaryRef newOptions)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)bridge;
	Boolean				ok			= TRUE;

	if (!isA_SCBridgeInterface(bridge)) {
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
						kSCNetworkInterfaceTypeBridge,
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
			CFDictionarySetValue(newDict, kSCPropVirtualNetworkInterfacesBridgeOptions, newOptions);
		} else {
			CFDictionaryRemoveValue(newDict, kSCPropVirtualNetworkInterfacesBridgeOptions);
		}
		if (!CFEqual(dict, newDict)) {
			ok = SCPreferencesPathSetValue(interfacePrivate->prefs, path, newDict);
		}
		CFRelease(newDict);
		CFRelease(path);
	}

	// set options in the SCBridgeInterfaceRef
	if (ok) {
		if (interfacePrivate->bridge.options != NULL) {
			CFRelease(interfacePrivate->bridge.options);
			interfacePrivate->bridge.options = NULL;
		}
		if (newOptions != NULL) {
			CFStringRef	name_auto	= NULL;

			interfacePrivate->bridge.options = CFDictionaryCreateCopy(NULL, newOptions);

			// set [auto] display name from options
			if ((interfacePrivate->localized_name == NULL) &&
			    CFDictionaryGetValueIfPresent(newOptions,
							  CFSTR("__AUTO__"),
							  (const void **)&name_auto) &&
			    isA_CFString(name_auto)) {
				// set display name
				interfacePrivate->localized_key = name_auto;
				if (interfacePrivate->localized_arg1 != NULL) {
					CFRelease(interfacePrivate->localized_arg1);
					interfacePrivate->localized_arg1 = NULL;
				}
			}
		}
	}

	return ok;
}


#pragma mark -
#pragma mark SCBridgeInterface management


#ifdef	IFT_BRIDGE
static Boolean
__bridge_add_interface(int s, CFStringRef bridge_if, CFStringRef interface_if)
{
	struct ifbreq	breq;
	struct ifdrv	ifd;

	// bridge interface
	bzero(&ifd, sizeof(ifd));
	(void) _SC_cfstring_to_cstring(bridge_if,
				       ifd.ifd_name,
				       sizeof(ifd.ifd_name),
				       kCFStringEncodingASCII);
	ifd.ifd_cmd = BRDGADD;
	ifd.ifd_len = sizeof(breq);
	ifd.ifd_data = (caddr_t)&breq;

	// new bridge member
	bzero(&breq, sizeof(breq));
	(void) _SC_cfstring_to_cstring(interface_if,
				       breq.ifbr_ifsname,
				       sizeof(breq.ifbr_ifsname),
				       kCFStringEncodingASCII);

	// add new bridge member
	if (ioctl(s, SIOCSDRVSPEC, (caddr_t)&ifd) == -1) {
		_SCErrorSet(errno);
		SCLog(TRUE, LOG_ERR,
		      CFSTR("could not add interface \"%@\" to bridge \"%@\": %s"),
		      interface_if,
		      bridge_if,
		      strerror(errno));
		return FALSE;
	}

	return TRUE;
}


static Boolean
__bridge_remove_interface(int s, CFStringRef bridge_if, CFStringRef interface_if)
{
	struct ifbreq	breq;
	struct ifdrv	ifd;

	// bridge interface
	bzero(&ifd, sizeof(ifd));
	(void) _SC_cfstring_to_cstring(bridge_if,
				       ifd.ifd_name,
				       sizeof(ifd.ifd_name),
				       kCFStringEncodingASCII);
	ifd.ifd_cmd = BRDGDEL;
	ifd.ifd_len = sizeof(breq);
	ifd.ifd_data = (caddr_t)&breq;

	// bridge member to remove
	bzero(&breq, sizeof(breq));
	(void) _SC_cfstring_to_cstring(interface_if,
				       breq.ifbr_ifsname,
				       sizeof(breq.ifbr_ifsname),
				       kCFStringEncodingASCII);

	// remove bridge member
	if (ioctl(s, SIOCSDRVSPEC, (caddr_t)&ifd) == -1) {
		_SCErrorSet(errno);
		SCLog(TRUE, LOG_ERR,
		      CFSTR("could not add interface \"%@\" to bridge \"%@\": %s"),
		      interface_if,
		      bridge_if,
		      strerror(errno));
		return FALSE;
	}

	return TRUE;
}
#endif	// IFT_BRIDGE


Boolean
_SCBridgeInterfaceUpdateConfiguration(SCPreferencesRef prefs)
{
#ifdef	IFT_BRIDGE
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

	/* configured Bridges */
	config  = SCBridgeInterfaceCopyAll(prefs);
	nConfig = (config != NULL) ? CFArrayGetCount(config) : 0;

	/* active Bridges */
	active  = _SCBridgeInterfaceCopyActive();
	nActive = (active != NULL) ? CFArrayGetCount(active) : 0;

	/*
	 * remove any no-longer-configured bridge interfaces and
	 * any devices associated with a bridge that are no longer
	 * associated with a bridge.
	 */
	for (i = 0; i < nActive; i++) {
		SCBridgeInterfaceRef	a_bridge;
		CFStringRef		a_bridge_if;
		CFIndex			j;
		Boolean			found	= FALSE;

		a_bridge    = CFArrayGetValueAtIndex(active, i);
		a_bridge_if = SCNetworkInterfaceGetBSDName(a_bridge);

		for (j = 0; j < nConfig; j++) {
			SCBridgeInterfaceRef	c_bridge;
			CFStringRef		c_bridge_if;

			c_bridge    = CFArrayGetValueAtIndex(config, j);
			c_bridge_if = SCNetworkInterfaceGetBSDName(c_bridge);

			if (CFEqual(a_bridge_if, c_bridge_if)) {
				CFIndex		a;
				CFArrayRef	a_bridge_interfaces;
				CFIndex		a_count;
				CFArrayRef	c_bridge_interfaces;
				CFIndex		c_count;

				c_bridge_interfaces = SCBridgeInterfaceGetMemberInterfaces(c_bridge);
				c_count             = (c_bridge_interfaces != NULL) ? CFArrayGetCount(c_bridge_interfaces) : 0;

				a_bridge_interfaces = SCBridgeInterfaceGetMemberInterfaces(a_bridge);
				a_count             = (a_bridge_interfaces != NULL) ? CFArrayGetCount(a_bridge_interfaces) : 0;

				for (a = 0; a < a_count; a++) {
					SCNetworkInterfaceRef	a_interface;
					CFStringRef		a_interface_if;

					a_interface = CFArrayGetValueAtIndex(a_bridge_interfaces, a);
					if ((c_count == 0) ||
					    !CFArrayContainsValue(c_bridge_interfaces,
								  CFRangeMake(0, c_count),
								  a_interface)) {
						/*
						 * if this device is no longer part
						 * of the bridge.
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
						if (!__bridge_remove_interface(s, a_bridge_if, a_interface_if)) {
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

			if (!__destroyInterface(s, a_bridge_if)) {
				_SCErrorSet(errno);
				ok = FALSE;
			}
		}
	}

	/*
	 * add any newly-configured bridge interfaces and add any
	 * devices that should now be associated with the bridge.
	 */
	for (i = 0; i < nConfig; i++) {
		SCBridgeInterfaceRef	c_bridge;
		CFArrayRef		c_bridge_interfaces;
		CFStringRef		c_bridge_if;
		CFIndex			c_count;
		Boolean			found		= FALSE;
		CFIndex			j;

		c_bridge            = CFArrayGetValueAtIndex(config, i);
		c_bridge_if         = SCNetworkInterfaceGetBSDName(c_bridge);
		c_bridge_interfaces = SCBridgeInterfaceGetMemberInterfaces(c_bridge);
		c_count             = (c_bridge_interfaces != NULL) ? CFArrayGetCount(c_bridge_interfaces) : 0;

		for (j = 0; j < nActive; j++) {
			SCBridgeInterfaceRef	a_bridge;
			CFArrayRef		a_bridge_interfaces;
			CFStringRef		a_bridge_if;
			CFIndex			a_count;

			a_bridge            = CFArrayGetValueAtIndex(active, j);
			a_bridge_if         = SCNetworkInterfaceGetBSDName(a_bridge);
			a_bridge_interfaces = SCBridgeInterfaceGetMemberInterfaces(a_bridge);
			a_count             = (a_bridge_interfaces != NULL) ? CFArrayGetCount(a_bridge_interfaces) : 0;

			if (CFEqual(c_bridge_if, a_bridge_if)) {
				CFIndex	c;
				Boolean	if_list_change = FALSE;

				found = TRUE;

				if (!_SC_CFEqual(c_bridge_interfaces, a_bridge_interfaces)) {
					if_list_change = TRUE;
				}
				if (!if_list_change) {
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
				if (!if_list_change) {
					break; // no if list changes
				}

				/*
				 * ensure that the first device of the bridge matches, if
				 * not then we remove all current devices and add them
				 * back in the preferred order.
				 */
				if ((c_count > 0) &&
				    (a_count > 0) &&
				    !CFEqual(CFArrayGetValueAtIndex(c_bridge_interfaces, 0),
					     CFArrayGetValueAtIndex(a_bridge_interfaces, 0))) {
					CFIndex	a;

					for (a = 0; a < a_count; a++) {
						SCNetworkInterfaceRef	a_interface;
						CFStringRef		a_interface_if;

						a_interface = CFArrayGetValueAtIndex(a_bridge_interfaces, a);
						if (!CFArrayContainsValue(c_bridge_interfaces,
									 CFRangeMake(0, c_count),
									 a_interface)) {
							continue;	// if already removed
						}

						a_interface_if = SCNetworkInterfaceGetBSDName(a_interface);
						if (!__bridge_remove_interface(s, a_bridge_if, a_interface_if)) {
							ok = FALSE;
						}
					}

					a_count = 0;	// all active devices have been removed
				}

				/*
				 * add any devices which are not currently associated
				 * with the bridge interface.
				 */
				for (c = 0; c < c_count; c++) {
					SCNetworkInterfaceRef		c_interface;
					SCNetworkInterfacePrivateRef	c_interfacePrivate;
					CFStringRef			c_interface_if;

					c_interface = CFArrayGetValueAtIndex(c_bridge_interfaces, c);
					if ((a_count == 0) ||
					    !CFArrayContainsValue(a_bridge_interfaces,
								  CFRangeMake(0, a_count),
								  c_interface)) {
						/*
						 * check if this member interface can be added to a bridge.
						 */
						c_interfacePrivate = (SCNetworkInterfacePrivateRef)c_interface;
						if (!c_interfacePrivate->supportsBridge) {
							// if member not supported
							continue;
						}

						/*
						 * if this member interface is not currently part of the bridge.
						 */
						c_interface_if = SCNetworkInterfaceGetBSDName(c_interface);
						if (!__bridge_add_interface(s, c_bridge_if, c_interface_if)) {
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
			 * establish the new bridge interface.
			 */
			if (!__createInterface(s, c_bridge_if)) {
				_SCErrorSet(errno);
				ok = FALSE;
				continue;
			}

			/*
			 * add the member interfaces
			 */
			for (c = 0; c < c_count; c++) {
				SCNetworkInterfaceRef		c_interface;
				SCNetworkInterfacePrivateRef	c_interfacePrivate;
				CFStringRef			c_interface_if;

				c_interface = CFArrayGetValueAtIndex(c_bridge_interfaces, c);
				c_interfacePrivate = (SCNetworkInterfacePrivateRef)c_interface;
				if (!c_interfacePrivate->supportsBridge) {
					// if member not supported
					continue;
				}

				c_interface_if = SCNetworkInterfaceGetBSDName(c_interface);
				if (!__bridge_add_interface(s, c_bridge_if, c_interface_if)) {
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
#else	// IFT_BRIDGE
	return TRUE;
#endif	// IFT_BRIDGE
}

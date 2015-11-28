/*
 * Copyright (c) 2003-2013 Apple Inc. All rights reserved.
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
 * November 14, 2003		Allan Nathanson <ajn@apple.com>
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
#include <net/ethernet.h>
#define	KERNEL_PRIVATE
#include <net/if.h>
#include <net/if_var.h>
#undef	KERNEL_PRIVATE
#include <net/if_vlan_var.h>
#include <net/if_types.h>

/* ---------- VLAN support ---------- */

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


typedef struct {
	CFMutableArrayRef	vlans;
	SCPreferencesRef	ni_prefs;
	SCPreferencesRef	prefs;
} addContext, *addContextRef;


static void
add_configured_interface(const void *key, const void *value, void *context)
{
	SCNetworkInterfacePrivateRef	interfacePrivate;
	addContextRef			myContext	= (addContextRef)context;
	SCVLANInterfaceRef		vlan;
	CFStringRef			vlan_if		= (CFStringRef)key;
	CFDictionaryRef			vlan_info	= (CFDictionaryRef)value;
	CFStringRef			vlan_name;
	CFDictionaryRef			vlan_options;
	SCNetworkInterfaceRef		vlan_physical = NULL;
	CFStringRef			vlan_physical_if;
	CFNumberRef			vlan_tag;

	vlan_physical_if = CFDictionaryGetValue(vlan_info, kSCPropVirtualNetworkInterfacesVLANInterface);
	if (!isA_CFString(vlan_physical_if)) {
		// if prefs are confused
		return;
	}

	vlan_tag = CFDictionaryGetValue(vlan_info, kSCPropVirtualNetworkInterfacesVLANTag);
	if (!isA_CFNumber(vlan_tag)) {
		// if prefs are confused
		return;
	}

	// create the VLAN interface
	vlan = (SCVLANInterfaceRef)_SCVLANInterfaceCreatePrivate(NULL, vlan_if);
	assert(vlan != NULL);

	// set physical interface and tag
	if (myContext->ni_prefs != NULL) {
		vlan_physical = __SCNetworkInterfaceCreateWithNIPreferencesUsingBSDName(NULL, myContext->ni_prefs,
											vlan_physical_if);
	}
	if (vlan_physical == NULL) {
		vlan_physical = _SCNetworkInterfaceCreateWithBSDName(NULL, vlan_physical_if,
								     kIncludeBondInterfaces);
	}
	assert(vlan_physical != NULL);

	// since we KNOW that the physical interface supported VLANs when
	// it was first established it's OK to force that state here ...
	// and this is needed for the case when the interface (e.g. a
	// dongle) is not currently attached to the system
	interfacePrivate = (SCNetworkInterfacePrivateRef)vlan_physical;
	interfacePrivate->supportsVLAN = TRUE;

	// and now we associate the physical interface and tag
	SCVLANInterfaceSetPhysicalInterfaceAndTag(vlan, vlan_physical, vlan_tag);
	CFRelease(vlan_physical);

	// set display name
	vlan_name = CFDictionaryGetValue(vlan_info, kSCPropUserDefinedName);
	if (isA_CFString(vlan_name)) {
		SCVLANInterfaceSetLocalizedDisplayName(vlan, vlan_name);
	}

	// set options
	vlan_options = CFDictionaryGetValue(vlan_info, kSCPropVirtualNetworkInterfacesVLANOptions);
	if (isA_CFDictionary(vlan_options)) {
		SCVLANInterfaceSetOptions(vlan, vlan_options);
	}

	// estabish link to the stored configuration
	interfacePrivate = (SCNetworkInterfacePrivateRef)vlan;
	interfacePrivate->prefs = CFRetain(myContext->prefs);

	CFArrayAppendValue(myContext->vlans, vlan);
	CFRelease(vlan);

	return;
}


static SCVLANInterfaceRef
findVLANInterfaceAndTag(SCPreferencesRef prefs, SCNetworkInterfaceRef physical, CFNumberRef tag)
{
	CFIndex			i;
	CFIndex			n;
	SCVLANInterfaceRef	vlan	= NULL;
	CFArrayRef		vlans;

	vlans = SCVLANInterfaceCopyAll(prefs);

	n = CFArrayGetCount(vlans);
	for (i = 0; i < n; i++) {
		SCVLANInterfaceRef	config_vlan;
		SCNetworkInterfaceRef	config_physical;
		CFNumberRef		config_tag;

		config_vlan     = CFArrayGetValueAtIndex(vlans, i);
		config_physical = SCVLANInterfaceGetPhysicalInterface(config_vlan);
		config_tag      = SCVLANInterfaceGetTag(config_vlan);

		if ((config_physical != NULL) && (config_tag != NULL)) {
			if (!CFEqual(physical, config_physical)) {
				// if this VLAN has a different physical interface
				continue;
			}

			if (!CFEqual(tag, config_tag)) {
				// if this VLAN has a different tag
				continue;
			}

			vlan = CFRetain(config_vlan);
			break;
		}
	}
	CFRelease(vlans);

	return vlan;
}


#pragma mark -
#pragma mark SCVLANInterface APIs


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
SCVLANInterfaceCopyAll(SCPreferencesRef prefs)
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
	context.vlans = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	context.ni_prefs = ni_prefs;
	context.prefs = prefs;

	path = CFStringCreateWithFormat(NULL,
					NULL,
					CFSTR("/%@/%@"),
					kSCPrefVirtualNetworkInterfaces,
					kSCNetworkInterfaceTypeVLAN);
	dict = SCPreferencesPathGetValue(prefs, path);
	CFRelease(path);
	if (isA_CFDictionary(dict)) {
		my_CFDictionaryApplyFunction(dict, add_configured_interface, &context);
	}
	if (ni_prefs != NULL) {
		CFRelease(ni_prefs);
	}
	return context.vlans;
}


static void
addAvailableInterfaces(CFMutableArrayRef available, CFArrayRef interfaces,
		       CFSetRef excluded)
{
	CFIndex	i;
	CFIndex	n;

	n = CFArrayGetCount(interfaces);
	for (i = 0; i < n; i++) {
		SCNetworkInterfaceRef		interface;
		SCNetworkInterfacePrivateRef	interfacePrivate;

		interface = CFArrayGetValueAtIndex(interfaces, i);
		interfacePrivate = (SCNetworkInterfacePrivateRef)interface;

		if ((excluded != NULL)
		    && CFSetContainsValue(excluded, interface)) {
			// exclude this interface
			continue;
		}
		if (interfacePrivate->supportsVLAN) {
			// if this interface is available
			CFArrayAppendValue(available, interface);
		}
	}

	return;
}


CFArrayRef
SCVLANInterfaceCopyAvailablePhysicalInterfaces()
{
	CFMutableArrayRef	available;
	CFArrayRef		bond_interfaces		= NULL;
	CFArrayRef		bridge_interfaces	= NULL;
	CFMutableSetRef		excluded		= NULL;
	CFArrayRef		interfaces;
	SCPreferencesRef	prefs;

	available = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	prefs = SCPreferencesCreate(NULL, CFSTR("SCVLANInterfaceCopyAvailablePhysicalInterfaces"), NULL);
	if (prefs != NULL) {
#if	!TARGET_OS_IPHONE
		bond_interfaces = SCBondInterfaceCopyAll(prefs);
		if (bond_interfaces != NULL) {
			excluded = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
			__SCBondInterfaceListCollectMembers(bond_interfaces, excluded);
		}
#endif	// !TARGET_OS_IPHONE

		bridge_interfaces = SCBridgeInterfaceCopyAll(prefs);
		if (bridge_interfaces != NULL) {
			if (excluded == NULL) {
				excluded = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
			}
			__SCBridgeInterfaceListCollectMembers(bridge_interfaces, excluded);
		}

		CFRelease(prefs);
	}

	// add real interfaces that aren't part of a bond or bridge
	interfaces = __SCNetworkInterfaceCopyAll_IONetworkInterface();
	if (interfaces != NULL) {
		addAvailableInterfaces(available, interfaces, excluded);
		CFRelease(interfaces);
	}

	// add bond interfaces
	if (bond_interfaces != NULL) {
		addAvailableInterfaces(available, bond_interfaces, NULL);
		CFRelease(bond_interfaces);
	}

	// add bridge interfaces
	if (bridge_interfaces != NULL) {
		addAvailableInterfaces(available, bridge_interfaces, NULL);
		CFRelease(bridge_interfaces);
	}

	if (excluded != NULL) {
		CFRelease(excluded);
	}

	return available;
}


CFArrayRef
_SCVLANInterfaceCopyActive(void)
{
	struct ifaddrs		*ifap;
	struct ifaddrs		*ifp;
	int			s;
	CFMutableArrayRef	vlans	= NULL;

	if (getifaddrs(&ifap) == -1) {
		SCLog(TRUE, LOG_ERR, CFSTR("getifaddrs() failed: %s"), strerror(errno));
		_SCErrorSet(kSCStatusFailed);
		return NULL;
	}

	s = inet_dgram_socket();
	if (s == -1) {
		_SCErrorSet(errno);
		goto done;
	}

	vlans = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	for (ifp = ifap; ifp != NULL; ifp = ifp->ifa_next) {
		struct if_data		*if_data;
		struct ifreq		ifr;
		SCVLANInterfaceRef	vlan;
		CFStringRef		vlan_if;
		SCNetworkInterfaceRef	vlan_physical;
		CFStringRef		vlan_physical_if;
		CFNumberRef		vlan_tag;
		char			vlr_parent[IFNAMSIZ];
		int			vlr_tag;
		struct vlanreq		vreq;

		if_data = (struct if_data *)ifp->ifa_data;
		if (if_data == NULL
		    || ifp->ifa_addr->sa_family != AF_LINK
		    || if_data->ifi_type != IFT_L2VLAN) {
			continue;
		}

		bzero(&ifr, sizeof(ifr));
		bzero(&vreq, sizeof(vreq));
		strlcpy(ifr.ifr_name, ifp->ifa_name, sizeof(ifr.ifr_name));
		ifr.ifr_data = (caddr_t)&vreq;

		if (ioctl(s, SIOCGIFVLAN, (caddr_t)&ifr) == -1) {
			SCLog(TRUE, LOG_ERR, CFSTR("ioctl(SIOCGIFVLAN) failed: %s"), strerror(errno));
			CFRelease(vlans);
			vlans = NULL;
			_SCErrorSet(kSCStatusFailed);
			goto done;
		}

		// create the VLAN interface
		vlan_if = CFStringCreateWithCString(NULL, ifp->ifa_name, kCFStringEncodingASCII);
		vlan    = (SCVLANInterfaceRef)_SCVLANInterfaceCreatePrivate(NULL, vlan_if);
		assert(vlan != NULL);
		CFRelease(vlan_if);

		// set the physical interface and tag
		strlcpy(vlr_parent, vreq.vlr_parent, sizeof(vlr_parent));
		vlan_physical_if = CFStringCreateWithCString(NULL, vlr_parent, kCFStringEncodingASCII);
		vlan_physical = _SCNetworkInterfaceCreateWithBSDName(NULL, vlan_physical_if,
								     kIncludeBondInterfaces);
		assert(vlan_physical != NULL);
		CFRelease(vlan_physical_if);

		vlr_tag  = vreq.vlr_tag;
		vlan_tag = CFNumberCreate(NULL, kCFNumberIntType, &vlr_tag);
		assert(vlan_tag != NULL);

		SCVLANInterfaceSetPhysicalInterfaceAndTag(vlan, vlan_physical, vlan_tag);
		CFRelease(vlan_physical);
		CFRelease(vlan_tag);

		// add VLAN
		CFArrayAppendValue(vlans, vlan);
		CFRelease(vlan);
	}

    done :

	if (s != -1) {
		(void) close(s);
	}
	freeifaddrs(ifap);
	return vlans;
}


SCVLANInterfaceRef
SCVLANInterfaceCreate(SCPreferencesRef prefs, SCNetworkInterfaceRef physical, CFNumberRef tag)
{
	CFAllocatorRef			allocator;
	CFIndex				i;
	SCNetworkInterfacePrivateRef	interfacePrivate;
	SCVLANInterfaceRef		vlan;

	if (prefs == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}
	
	if (!isA_SCNetworkInterface(physical)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	interfacePrivate = (SCNetworkInterfacePrivateRef)physical;
	if (!interfacePrivate->supportsVLAN) {
		if (__SCPreferencesUsingDefaultPrefs(prefs) == FALSE) {
			interfacePrivate->supportsVLAN = TRUE;
		}
		else {
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}
	}

	if (isA_CFNumber(tag)) {
		int	tag_val;

		CFNumberGetValue(tag, kCFNumberIntType, &tag_val);
		if ((tag_val < 1) || (tag_val > 4094)) {
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}
	} else {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	// make sure that physical interface and tag are not used
	vlan = findVLANInterfaceAndTag(prefs, physical, tag);
	if (vlan != NULL) {
		CFRelease(vlan);
		_SCErrorSet(kSCStatusKeyExists);
		return NULL;
	}

	allocator = CFGetAllocator(prefs);

	// create a new VLAN using an unused interface name
	for (i = 0; vlan == NULL; i++) {
		CFDictionaryRef			dict;
		CFStringRef			vlan_if;
		Boolean				ok;
		CFStringRef			path;

		vlan_if = CFStringCreateWithFormat(allocator, NULL, CFSTR("vlan%ld"), i);
		path    = CFStringCreateWithFormat(allocator,
						   NULL,
						   CFSTR("/%@/%@/%@"),
						   kSCPrefVirtualNetworkInterfaces,
						   kSCNetworkInterfaceTypeVLAN,
						   vlan_if);
		dict = SCPreferencesPathGetValue(prefs, path);
		if (dict != NULL) {
			// if VLAN interface name not available
			CFRelease(path);
			CFRelease(vlan_if);
			continue;
		}

		// add the VLAN to the stored preferences
		dict = CFDictionaryCreate(allocator,
					  NULL, NULL, 0,
					  &kCFTypeDictionaryKeyCallBacks,
					  &kCFTypeDictionaryValueCallBacks);
		ok = SCPreferencesPathSetValue(prefs, path, dict);
		CFRelease(dict);
		CFRelease(path);
		if (!ok) {
			// if the VLAN could not be saved
			CFRelease(vlan_if);
			_SCErrorSet(kSCStatusFailed);
			break;
		}

		// create the SCVLANInterfaceRef
		vlan = (SCVLANInterfaceRef)_SCVLANInterfaceCreatePrivate(allocator, vlan_if);
		CFRelease(vlan_if);

		// estabish link to the stored configuration
		interfacePrivate = (SCNetworkInterfacePrivateRef)vlan;
		interfacePrivate->prefs = CFRetain(prefs);

		// set physical interface and tag
		SCVLANInterfaceSetPhysicalInterfaceAndTag(vlan, physical, tag);
	}

	return vlan;
}


Boolean
SCVLANInterfaceRemove(SCVLANInterfaceRef vlan)
{
	CFStringRef			vlan_if;
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)vlan;
	Boolean				ok;
	CFStringRef			path;

	if (!isA_SCVLANInterface(vlan)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (interfacePrivate->prefs == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	vlan_if = SCNetworkInterfaceGetBSDName(vlan);
	path    = CFStringCreateWithFormat(NULL,
					   NULL,
					   CFSTR("/%@/%@/%@"),
					   kSCPrefVirtualNetworkInterfaces,
					   kSCNetworkInterfaceTypeVLAN,
					   vlan_if);
	ok = SCPreferencesPathRemoveValue(interfacePrivate->prefs, path);
	CFRelease(path);

	return ok;
}


SCNetworkInterfaceRef
SCVLANInterfaceGetPhysicalInterface(SCVLANInterfaceRef vlan)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)vlan;

	if (!isA_SCVLANInterface(vlan)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	return interfacePrivate->vlan.interface;
}


CFNumberRef
SCVLANInterfaceGetTag(SCVLANInterfaceRef vlan)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)vlan;

	if (!isA_SCVLANInterface(vlan)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	return interfacePrivate->vlan.tag;
}


CFDictionaryRef
SCVLANInterfaceGetOptions(SCVLANInterfaceRef vlan)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)vlan;

	if (!isA_SCVLANInterface(vlan)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	return interfacePrivate->vlan.options;
}


Boolean
SCVLANInterfaceSetPhysicalInterfaceAndTag(SCVLANInterfaceRef vlan, SCNetworkInterfaceRef physical, CFNumberRef tag)
{
	SCNetworkInterfacePrivateRef	interfacePrivate;
	Boolean				ok			= TRUE;
	SCPreferencesRef		prefs;
	
	if (!isA_SCVLANInterface(vlan)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (!isA_SCNetworkInterface(physical)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	interfacePrivate = (SCNetworkInterfacePrivateRef)physical;
	prefs = interfacePrivate->prefs;
	
	if (!interfacePrivate->supportsVLAN) {
		if (__SCPreferencesUsingDefaultPrefs(prefs) == FALSE) {
			interfacePrivate->supportsVLAN = TRUE;
		}
		else {
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
		}
	}

	if (isA_CFNumber(tag)) {
		int	tag_val;

		CFNumberGetValue(tag, kCFNumberIntType, &tag_val);
		if ((tag_val < 1) || (tag_val > 4094)) {
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
		}
	} else {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	interfacePrivate = (SCNetworkInterfacePrivateRef)vlan;
	if (interfacePrivate->prefs != NULL) {
		SCVLANInterfaceRef	config_vlan;
		CFDictionaryRef		dict;
		CFMutableDictionaryRef	newDict;
		CFStringRef		path;

		// make sure that physical interface and tag are not used
		config_vlan = findVLANInterfaceAndTag(interfacePrivate->prefs, physical, tag);
		if (config_vlan != NULL) {
			if (!CFEqual(vlan, config_vlan)) {
				CFRelease(config_vlan);
				_SCErrorSet(kSCStatusKeyExists);
				return FALSE;
			}
			CFRelease(config_vlan);
		}

		// set interface/tag in the stored preferences
		path = CFStringCreateWithFormat(NULL,
						NULL,
						CFSTR("/%@/%@/%@"),
						kSCPrefVirtualNetworkInterfaces,
						kSCNetworkInterfaceTypeVLAN,
						interfacePrivate->entity_device);
		dict = SCPreferencesPathGetValue(interfacePrivate->prefs, path);
		if (!isA_CFDictionary(dict)) {
			// if the prefs are confused
			CFRelease(path);
			_SCErrorSet(kSCStatusFailed);
			return FALSE;
		}

		newDict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
		CFDictionarySetValue(newDict,
				     kSCPropVirtualNetworkInterfacesVLANInterface,
				     SCNetworkInterfaceGetBSDName(physical));
		CFDictionarySetValue(newDict, kSCPropVirtualNetworkInterfacesVLANTag, tag);
		if (!CFEqual(dict, newDict)) {
			ok = SCPreferencesPathSetValue(interfacePrivate->prefs, path, newDict);
		}
		CFRelease(newDict);
		CFRelease(path);
	}

	if (ok) {
		SCNetworkInterfacePrivateRef	newInterface;
		CFTypeRef			save;

		// set physical interface
		newInterface = __SCNetworkInterfaceCreateCopy(NULL,
							      physical,
							      interfacePrivate->prefs,
							      interfacePrivate->serviceID);
		save = interfacePrivate->vlan.interface;
		interfacePrivate->vlan.interface = (SCNetworkInterfaceRef)newInterface;
		if (save != NULL) CFRelease(save);

		// set tag
		save = interfacePrivate->vlan.tag;
		interfacePrivate->vlan.tag = CFRetain(tag);
		if (save != NULL) CFRelease(save);
	}

	return ok;
}


Boolean
SCVLANInterfaceSetLocalizedDisplayName(SCVLANInterfaceRef vlan, CFStringRef newName)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)vlan;
	Boolean				ok			= TRUE;

	if (!isA_SCVLANInterface(vlan)) {
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
						kSCNetworkInterfaceTypeVLAN,
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

	// set name in the SCVLANInterfaceRef
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
SCVLANInterfaceSetOptions(SCVLANInterfaceRef vlan, CFDictionaryRef newOptions)
{
	SCNetworkInterfacePrivateRef	interfacePrivate	= (SCNetworkInterfacePrivateRef)vlan;
	Boolean				ok			= TRUE;

	if (!isA_SCVLANInterface(vlan)) {
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
						kSCNetworkInterfaceTypeVLAN,
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
			CFDictionarySetValue(newDict, kSCPropVirtualNetworkInterfacesVLANOptions, newOptions);
		} else {
			CFDictionaryRemoveValue(newDict, kSCPropVirtualNetworkInterfacesVLANOptions);
		}
		if (!CFEqual(dict, newDict)) {
			ok = SCPreferencesPathSetValue(interfacePrivate->prefs, path, newDict);
		}
		CFRelease(newDict);
		CFRelease(path);
	}

	// set options in the SCVLANInterfaceRef
	if (ok) {
		if (interfacePrivate->vlan.options != NULL) {
			CFRelease(interfacePrivate->vlan.options);
			interfacePrivate->vlan.options = NULL;
		}
		if (newOptions != NULL) {
			interfacePrivate->vlan.options = CFDictionaryCreateCopy(NULL, newOptions);
		}
	}

	return ok;
}


#pragma mark -
#pragma mark SCVLANInterface management


static Boolean
__vlan_set(int s, CFStringRef interface_if, CFStringRef physical_if, CFNumberRef tag)
{
	struct ifreq	ifr;
	int		tag_val;
	struct vlanreq	vreq;

	bzero(&ifr, sizeof(ifr));
	bzero(&vreq, sizeof(vreq));

	// interface
	(void) _SC_cfstring_to_cstring(interface_if,
				       ifr.ifr_name,
				       sizeof(ifr.ifr_name),
				       kCFStringEncodingASCII);
	ifr.ifr_data = (caddr_t)&vreq;

	// physical interface
	(void) _SC_cfstring_to_cstring(physical_if,
				       vreq.vlr_parent,
				       sizeof(vreq.vlr_parent),
				       kCFStringEncodingASCII);

	// tag
	CFNumberGetValue(tag, kCFNumberIntType, &tag_val);
	vreq.vlr_tag = tag_val;

	// update physical interface and tag
	if (ioctl(s, SIOCSIFVLAN, (caddr_t)&ifr) == -1) {
		SCLog(TRUE, LOG_ERR, CFSTR("ioctl(SIOCSIFVLAN) failed: %s"), strerror(errno));
		_SCErrorSet(kSCStatusFailed);
		return FALSE;
	}

	return TRUE;
}


static Boolean
__vlan_clear(int s, CFStringRef interface_if)
{
	struct ifreq	ifr;
	struct vlanreq	vreq;

	bzero(&ifr, sizeof(ifr));
	bzero(&vreq, sizeof(vreq));

	// interface
	(void) _SC_cfstring_to_cstring(interface_if,
				       ifr.ifr_name,
				       sizeof(ifr.ifr_name),
				       kCFStringEncodingASCII);
	ifr.ifr_data = (caddr_t)&vreq;

	// clear physical interface
	bzero(&vreq.vlr_parent, sizeof(vreq.vlr_parent));

	// clear tag
	vreq.vlr_tag = 0;

	// update physical interface and tag
	if (ioctl(s, SIOCSIFVLAN, (caddr_t)&ifr) == -1) {
		SCLog(TRUE, LOG_ERR, CFSTR("ioctl(SIOCSIFVLAN) failed: %s"), strerror(errno));
		_SCErrorSet(kSCStatusFailed);
		return FALSE;
	}

	return TRUE;
}


Boolean
_SCVLANInterfaceUpdateConfiguration(SCPreferencesRef prefs)
{
	CFArrayRef			active		= NULL;
	CFArrayRef			config		= NULL;
	CFMutableDictionaryRef		devices		= NULL;
	CFIndex				i;
	CFIndex				nActive;
	CFIndex				nConfig;
	Boolean				ok		= TRUE;
	int				s		= -1;

	if (prefs == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	/* configured VLANs */
	config = SCVLANInterfaceCopyAll(prefs);
	nConfig = (config != NULL) ? CFArrayGetCount(config) : 0;

	/* physical interfaces */
	devices = CFDictionaryCreateMutable(NULL,
					    0,
					    &kCFTypeDictionaryKeyCallBacks,
					    &kCFTypeDictionaryValueCallBacks);

	/* active VLANs */
	active  = _SCVLANInterfaceCopyActive();
	nActive = (active != NULL) ? CFArrayGetCount(active) : 0;

	/* remove any no-longer-configured VLAN interfaces */
	for (i = 0; i < nActive; i++) {
		SCVLANInterfaceRef	a_vlan;
		CFStringRef		a_vlan_if;
		CFIndex			j;
		Boolean			found	= FALSE;

		a_vlan    = CFArrayGetValueAtIndex(active, i);
		a_vlan_if = SCNetworkInterfaceGetBSDName(a_vlan);

		for (j = 0; j < nConfig; j++) {
			SCVLANInterfaceRef	c_vlan;
			CFStringRef		c_vlan_if;

			c_vlan    = CFArrayGetValueAtIndex(config, j);
			c_vlan_if = SCNetworkInterfaceGetBSDName(c_vlan);

			if (CFEqual(a_vlan_if, c_vlan_if)) {
				found = TRUE;
				break;
			}
		}

		if (!found) {
			// remove VLAN interface
			if (s == -1) {
				s = inet_dgram_socket();
				if (s == -1) {
					_SCErrorSet(errno);
					ok = FALSE;
					goto done;
				}
			}
			if (!__destroyInterface(s, a_vlan_if)) {
				ok = FALSE;
				_SCErrorSet(errno);
			}
		}
	}

	/* create (and update) configured VLAN interfaces */
	for (i = 0; i < nConfig; i++) {
		SCVLANInterfaceRef	c_vlan;
		CFStringRef		c_vlan_if;
		SCNetworkInterfaceRef	c_vlan_physical;
		Boolean			found		= FALSE;
		CFIndex			j;
		CFBooleanRef		supported;

		c_vlan          = CFArrayGetValueAtIndex(config, i);
		c_vlan_if       = SCNetworkInterfaceGetBSDName(c_vlan);
		c_vlan_physical = SCVLANInterfaceGetPhysicalInterface(c_vlan);

		if (c_vlan_physical == NULL) {
			continue;
		}
		// determine if the physical interface supports VLANs
		supported = CFDictionaryGetValue(devices, c_vlan_physical);
		if (supported == NULL) {
			SCNetworkInterfacePrivateRef	c_vlan_physicalPrivate	= (SCNetworkInterfacePrivateRef)c_vlan_physical;

			supported = c_vlan_physicalPrivate->supportsVLAN ? kCFBooleanTrue
									 : kCFBooleanFalse;
			CFDictionaryAddValue(devices, c_vlan_physical, supported);
		}

		for (j = 0; j < nActive; j++) {
			SCVLANInterfaceRef	a_vlan;
			CFStringRef		a_vlan_if;

			a_vlan    = CFArrayGetValueAtIndex(active, j);
			a_vlan_if = SCNetworkInterfaceGetBSDName(a_vlan);

			if (CFEqual(c_vlan_if, a_vlan_if)) {
				if (!CFEqual(c_vlan, a_vlan)) {
					// update VLAN interface
					if (s == -1) {
						s = inet_dgram_socket();
						if (s == -1) {
							_SCErrorSet(errno);
							ok = FALSE;
							goto done;
						}
					}

					if (!CFBooleanGetValue(supported)
					    || !__vlan_clear(s, c_vlan_if)
					    || !__vlan_set(s, c_vlan_if,
							   SCNetworkInterfaceGetBSDName(c_vlan_physical),
							   SCVLANInterfaceGetTag(c_vlan))) {
						// something went wrong, try to blow the VLAN away
						if (!CFBooleanGetValue(supported)) {
							_SCErrorSet(kSCStatusFailed);
						}
						(void)__destroyInterface(s, c_vlan_if);
						ok = FALSE;
					}
				}

				found = TRUE;
				break;
			}
		}

		if (!found && CFBooleanGetValue(supported)) {
			// if the physical interface supports VLANs, add new interface
			Boolean		created;

			if (s == -1) {
				s = inet_dgram_socket();
				if (s == -1) {
					_SCErrorSet(errno);
					ok = FALSE;
					goto done;
				}
			}

			created = __createInterface(s, c_vlan_if);
			if (!created
			    || !__vlan_set(s,
					   c_vlan_if,
					   SCNetworkInterfaceGetBSDName(c_vlan_physical),
					   SCVLANInterfaceGetTag(c_vlan))) {
				if (created) {
					// something went wrong, try to blow the VLAN away
					(void)__destroyInterface(s, c_vlan_if);
				} else {
					_SCErrorSet(errno);
				}
				ok = FALSE;
			}
		}

	}

    done :

	if (active)	CFRelease(active);
	if (config)	CFRelease(config);
	if (devices)	CFRelease(devices);
	if (s != -1)	(void) close(s);

	return ok;
}

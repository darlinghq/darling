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
 * May 13, 2004		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include "SCNetworkConfigurationInternal.h"
#include <SystemConfiguration/SCValidation.h>
#include <SystemConfiguration/SCPrivate.h>

#include <pthread.h>


static CFStringRef	__SCNetworkSetCopyDescription		(CFTypeRef cf);
static void		__SCNetworkSetDeallocate		(CFTypeRef cf);
static Boolean		__SCNetworkSetEqual			(CFTypeRef cf1, CFTypeRef cf2);
static CFHashCode	__SCNetworkSetHash			(CFTypeRef cf);


static CFTypeID __kSCNetworkSetTypeID	= _kCFRuntimeNotATypeID;


static const CFRuntimeClass __SCNetworkSetClass = {
	0,				// version
	"SCNetworkSet",			// className
	NULL,				// init
	NULL,				// copy
	__SCNetworkSetDeallocate,       // dealloc
	__SCNetworkSetEqual,		// equal
	__SCNetworkSetHash,		// hash
	NULL,				// copyFormattingDesc
	__SCNetworkSetCopyDescription	// copyDebugDesc
};


static pthread_once_t		initialized	= PTHREAD_ONCE_INIT;


static CFStringRef
__SCNetworkSetCopyDescription(CFTypeRef cf)
{
	CFAllocatorRef		allocator	= CFGetAllocator(cf);
	CFMutableStringRef      result;
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)cf;

	result = CFStringCreateMutable(allocator, 0);
	CFStringAppendFormat(result, NULL, CFSTR("<SCNetworkSet %p [%p]> {"), cf, allocator);
	CFStringAppendFormat(result, NULL, CFSTR("id = %@"), setPrivate->setID);
	CFStringAppendFormat(result, NULL, CFSTR(", prefs = %p"), setPrivate->prefs);
	if (setPrivate->name != NULL) {
		CFStringAppendFormat(result, NULL, CFSTR(", name = %@"), setPrivate->name);
	}
	CFStringAppendFormat(result, NULL, CFSTR("}"));

	return result;
}


static void
__SCNetworkSetDeallocate(CFTypeRef cf)
{
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)cf;

	/* release resources */

	CFRelease(setPrivate->setID);
	CFRelease(setPrivate->prefs);
	if (setPrivate->name != NULL)
		CFRelease(setPrivate->name);

	return;
}


static Boolean
__SCNetworkSetEqual(CFTypeRef cf1, CFTypeRef cf2)
{
	SCNetworkSetPrivateRef	s1	= (SCNetworkSetPrivateRef)cf1;
	SCNetworkSetPrivateRef	s2	= (SCNetworkSetPrivateRef)cf2;

	if (s1 == s2)
		return TRUE;

	if (s1->prefs != s2->prefs)
		return FALSE;   // if not the same prefs

	if (!CFEqual(s1->setID, s2->setID))
		return FALSE;	// if not the same set identifier

	return TRUE;
}


static CFHashCode
__SCNetworkSetHash(CFTypeRef cf)
{
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)cf;

	return CFHash(setPrivate->setID);
}


static void
__SCNetworkSetInitialize(void)
{
	__kSCNetworkSetTypeID = _CFRuntimeRegisterClass(&__SCNetworkSetClass);
	return;
}


static SCNetworkSetPrivateRef
__SCNetworkSetCreatePrivate(CFAllocatorRef      allocator,
			    SCPreferencesRef	prefs,
			    CFStringRef		setID)
{
	SCNetworkSetPrivateRef  setPrivate;
	uint32_t		size;

	/* initialize runtime */
	pthread_once(&initialized, __SCNetworkSetInitialize);

	/* allocate target */
	size            = sizeof(SCNetworkSetPrivate) - sizeof(CFRuntimeBase);
	setPrivate = (SCNetworkSetPrivateRef)_CFRuntimeCreateInstance(allocator,
								      __kSCNetworkSetTypeID,
								      size,
								      NULL);
	if (setPrivate == NULL) {
		return NULL;
	}

	setPrivate->setID       = CFStringCreateCopy(NULL, setID);
	setPrivate->prefs       = CFRetain(prefs);
	setPrivate->name	= NULL;
	setPrivate->established	= FALSE;	// "new" (not yet established) set

	return setPrivate;
}


#pragma mark -


static int
_serviceOrder(SCNetworkServiceRef service)
{
	SCNetworkInterfaceRef	interface;

	interface = SCNetworkServiceGetInterface(service);
	if ((interface == NULL) || _SCNetworkServiceIsVPN(service)) {
		return 100000;	// if unknown or VPN interface, sort last
	}

	return __SCNetworkInterfaceOrder(interface);
}


static void
_serviceOrder_add(SCNetworkSetRef set, SCNetworkServiceRef service)
{
	CFIndex			i;
	CFIndex			n;
	CFMutableArrayRef	newOrder;
	CFArrayRef		order;
	CFStringRef		serviceID;
	CFIndex			serviceOrder;
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)set;
	CFIndex			slot;

	order = SCNetworkSetGetServiceOrder(set);
	if (order != NULL) {
		newOrder = CFArrayCreateMutableCopy(NULL, 0, order);
	} else {
		newOrder = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}
	assert(newOrder != NULL);
	n = CFArrayGetCount(newOrder);

	serviceID = SCNetworkServiceGetServiceID(service);
	if (CFArrayContainsValue(newOrder, CFRangeMake(0, n), serviceID)) {
		// if serviceID already present
		goto done;
	}

	serviceOrder = _serviceOrder(service);

	slot = 0;
	for (i = 0; i < n; i++) {
		int			slotOrder;
		SCNetworkServiceRef	slotService;
		CFStringRef		slotServiceID;

		slotServiceID = CFArrayGetValueAtIndex(newOrder, i);
		if (!isA_CFString(slotServiceID)) {
			// if bad prefs
			continue;
		}

		slotService = SCNetworkServiceCopy(setPrivate->prefs, slotServiceID);
		if (slotService == NULL) {
			// if serviceID not valid
			continue;
		}

		slotOrder = _serviceOrder(slotService);
		if (serviceOrder >= slotOrder) {
			// add the service *after* this one
			slot = i + 1;
		}

		CFRelease(slotService);
	}

	CFArrayInsertValueAtIndex(newOrder, slot, serviceID);
	(void) SCNetworkSetSetServiceOrder(set, newOrder);

    done :

	CFRelease(newOrder);

	return;
}


static void
_serviceOrder_remove(SCNetworkSetRef set, SCNetworkServiceRef service)
{
	CFMutableArrayRef	newOrder;
	CFArrayRef		order;
	CFStringRef		serviceID;

	order = SCNetworkSetGetServiceOrder(set);
	if (order == NULL) {
		return;
	}

	serviceID = SCNetworkServiceGetServiceID(service);

	newOrder = CFArrayCreateMutableCopy(NULL, 0, order);
	while (TRUE) {
		CFIndex	i;

		i = CFArrayGetFirstIndexOfValue(newOrder,
						CFRangeMake(0, CFArrayGetCount(newOrder)),
						serviceID);
		if (i == kCFNotFound) {
			break;
		}

		CFArrayRemoveValueAtIndex(newOrder, i);
	}
	(void) SCNetworkSetSetServiceOrder(set, newOrder);
	CFRelease(newOrder);

	return;
}


#pragma mark -
#pragma mark SCNetworkSet APIs


#define	N_QUICK	16


#define PREVENT_DUPLICATE_SERVICE_NAMES
#ifdef  PREVENT_DUPLICATE_SERVICE_NAMES
static CFStringRef
copy_next_name(CFStringRef name)
{
	CFArrayRef		components;
	CFIndex			n;
	CFMutableArrayRef	newComponents;
	SInt32			suffix	= 2;

	if (name == NULL) {
		return NULL;
	}

	components = CFStringCreateArrayBySeparatingStrings(NULL, name, CFSTR(" "));
	if (components != NULL) {
		newComponents = CFArrayCreateMutableCopy(NULL, 0, components);
		CFRelease(components);
	} else {
		newComponents = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		CFArrayAppendValue(newComponents, name);
	}

	n = CFArrayGetCount(newComponents);
	if (n > 1) {
		CFStringRef	str;

		str = CFArrayGetValueAtIndex(newComponents, n - 1);
		suffix = CFStringGetIntValue(str);
		if (suffix++ > 0) {
			CFArrayRemoveValueAtIndex(newComponents, n - 1);
		} else {
			suffix = 2;
		}
	}

	name = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), (int)suffix);
	CFArrayAppendValue(newComponents, name);
	CFRelease(name);

	name = CFStringCreateByCombiningStrings(NULL, newComponents, CFSTR(" "));
	CFRelease(newComponents);

	return name;
}


static Boolean
ensure_unique_service_name(SCNetworkServiceRef service)
{
	SCNetworkInterfaceRef	interface;
	CFStringRef		name;
	Boolean			ok	= TRUE;

	interface = SCNetworkServiceGetInterface(service);

	name = SCNetworkServiceGetName(service);
	if (name != NULL) {
		CFRetain(name);
	}

	while (TRUE) {
		CFStringRef	newName;

		ok = SCNetworkServiceSetName(service, name);
		if (ok) {
			break;
		}

		if (SCError() != kSCStatusKeyExists) {
			SCLog(TRUE, LOG_DEBUG,
			      CFSTR("could not update service name for \"%@\": %s"),
			      SCNetworkInterfaceGetLocalizedDisplayName(interface),
			      SCErrorString(SCError()));
			break;
		}

		newName = copy_next_name(name);
		if (newName == NULL) {
			SCLog(TRUE, LOG_DEBUG,
			      CFSTR("could not create unique name for \"%@\": %s"),
			      SCNetworkInterfaceGetLocalizedDisplayName(interface),
			      SCErrorString(SCError()));
			break;
		}

		// try again with the "new" name
		if (name != NULL) {
			CFRelease(name);
		}
		name = newName;
	}

	if (name != NULL) {
		CFRelease(name);
	}

	return ok;
}
#endif	// PREVENT_DUPLICATE_SERVICE_NAMES


Boolean
SCNetworkSetAddService(SCNetworkSetRef set, SCNetworkServiceRef service)
{
	SCNetworkInterfaceRef		interface;
	CFArrayRef			interface_config	= NULL;
	CFStringRef			link;
	Boolean				ok;
	CFStringRef			path;
	SCNetworkServicePrivateRef	servicePrivate		= (SCNetworkServicePrivateRef)service;
	SCNetworkSetPrivateRef		setPrivate		= (SCNetworkSetPrivateRef)set;

	if (!isA_SCNetworkSet(set)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (!isA_SCNetworkService(service) || (servicePrivate->prefs == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	// make sure that we do not add an orphaned network service if its
	// associated interface is a member of a bond or bridge.
	interface = SCNetworkServiceGetInterface(service);
	if ((interface != NULL) &&
	    __SCNetworkInterfaceIsMember(servicePrivate->prefs, interface)) {
		_SCErrorSet(kSCStatusKeyExists);
		return FALSE;
	}

//#define PREVENT_DUPLICATE_SETS
#ifdef  PREVENT_DUPLICATE_SETS
	CFArrayRef	sets;

	// ensure that each service is only a member of ONE set
	sets = SCNetworkSetCopyAll(setPrivate->prefs);
	if (sets != NULL) {
		CFIndex		i;
		CFIndex		n;

		n = CFArrayGetCount(sets);
		for (i = 0; i < n; i++) {
			Boolean		found;
			CFArrayRef      services;
			SCNetworkSetRef set;

			set = CFArrayGetValueAtIndex(sets, i);
			services = SCNetworkSetCopyServices(set);
			found = CFArrayContainsValue(services,
						     CFRangeMake(0, CFArrayGetCount(services)),
						     service);
			CFRelease(services);

			if (found) {
				CFRelease(sets);
				_SCErrorSet(kSCStatusKeyExists);
				return FALSE;
			}
		}
		CFRelease(sets);
	}
#endif  /* PREVENT_DUPLICATE_SETS */

	// get the [deep] interface configuration settings
	interface = SCNetworkServiceGetInterface(service);
	if (interface != NULL) {
		interface_config = __SCNetworkInterfaceCopyDeepConfiguration(set, interface);
	}

	// create the link between "set" and the "service"
	path = SCPreferencesPathKeyCreateSetNetworkServiceEntity(NULL,				// allocator
								 setPrivate->setID,		// set
								 servicePrivate->serviceID,     // service
								 NULL);				// entity
	link = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
							      servicePrivate->serviceID,	// service
							      NULL);				// entity
	ok = SCPreferencesPathSetLink(setPrivate->prefs, path, link);
#ifdef	PREVENT_DUPLICATE_SERVICE_NAMES
	if (ok) {
		ok = ensure_unique_service_name(service);
		if (!ok) {
			// if we could not ensure a unique name, remove the (just added)
			// link between the "set" and the "service"
			(void) SCPreferencesPathRemoveValue(setPrivate->prefs, path);
		}
	}
#endif	// PREVENT_DUPLICATE_SERVICE_NAMES
	CFRelease(path);
	CFRelease(link);
	if (!ok) {
		goto done;
	}

	// push the [deep] interface configuration into all sets which contain this service.
	if (interface != NULL) {
		__SCNetworkInterfaceSetDeepConfiguration(set, interface, interface_config);
	}

	// add service to ServiceOrder
	_serviceOrder_add(set, service);

	// mark set as no longer "new"
	setPrivate->established	= TRUE;

    done :

	if (interface_config != NULL)	CFRelease(interface_config);
	return ok;
}


SCNetworkSetRef
SCNetworkSetCopy(SCPreferencesRef prefs, CFStringRef setID)
{
	CFDictionaryRef		entity;
	CFStringRef		path;
	SCNetworkSetPrivateRef	setPrivate;

	if (!isA_CFString(setID)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	path = SCPreferencesPathKeyCreateSet(NULL, setID);
	entity = SCPreferencesPathGetValue(prefs, path);
	CFRelease(path);

	if (!isA_CFDictionary(entity)) {
		_SCErrorSet(kSCStatusNoKey);
		return NULL;
	}

	setPrivate = __SCNetworkSetCreatePrivate(NULL, prefs, setID);
	assert(setPrivate != NULL);

	// mark set as "old" (already established)
	setPrivate->established = TRUE;

	return (SCNetworkSetRef)setPrivate;
}


Boolean
SCNetworkSetContainsInterface(SCNetworkSetRef set, SCNetworkInterfaceRef interface)
{
	Boolean		found	= FALSE;
	CFArrayRef	services;

	services = SCNetworkSetCopyServices(set);
	if (services != NULL) {
		found = __SCNetworkServiceExistsForInterface(services, interface);
		CFRelease(services);
	}

	return found;
}


CFArrayRef /* of SCNetworkSetRef's */
SCNetworkSetCopyAll(SCPreferencesRef prefs)
{
	CFMutableArrayRef	array;
	CFIndex			n;
	CFStringRef		path;
	CFDictionaryRef		sets;

	path = SCPreferencesPathKeyCreateSets(NULL);
	sets = SCPreferencesPathGetValue(prefs, path);
	CFRelease(path);

	if ((sets != NULL) && !isA_CFDictionary(sets)) {
		return NULL;
	}

	array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	n = (sets != NULL) ? CFDictionaryGetCount(sets) : 0;
	if (n > 0) {
		CFIndex		i;
		const void *    keys_q[N_QUICK];
		const void **   keys	= keys_q;
		const void *    vals_q[N_QUICK];
		const void **   vals	= vals_q;

		if (n > (CFIndex)(sizeof(keys_q) / sizeof(CFTypeRef))) {
			keys = CFAllocatorAllocate(NULL, n * sizeof(CFTypeRef), 0);
			vals = CFAllocatorAllocate(NULL, n * sizeof(CFPropertyListRef), 0);
		}
		CFDictionaryGetKeysAndValues(sets, keys, vals);
		for (i = 0; i < n; i++) {
			SCNetworkSetPrivateRef	setPrivate;

			if (!isA_CFDictionary(vals[i])) {
				SCLog(TRUE,
				      LOG_INFO,
				      CFSTR("SCNetworkSetCopyAll(): error w/set \"%@\"\n"),
				      keys[i]);
				continue;
			}

			setPrivate = __SCNetworkSetCreatePrivate(NULL, prefs, keys[i]);
			assert(setPrivate != NULL);

			// mark set as "old" (already established)
			setPrivate->established = TRUE;

			CFArrayAppendValue(array, (SCNetworkSetRef)setPrivate);
			CFRelease(setPrivate);
		}
		if (keys != keys_q) {
			CFAllocatorDeallocate(NULL, keys);
			CFAllocatorDeallocate(NULL, vals);
		}
	}

	return array;
}


CFArrayRef /* of SCNetworkInterfaceRef's */
SCNetworkSetCopyAvailableInterfaces(SCNetworkSetRef set)
{
	CFMutableArrayRef	available;
	CFMutableSetRef		excluded	= NULL;
	int			i;
	CFArrayRef		interfaces;
	CFIndex			n_interfaces;
	CFIndex			n_exclusions	= 0;
	SCPreferencesRef	prefs;
	SCNetworkSetPrivateRef	setPrivate;

	setPrivate = (SCNetworkSetPrivateRef)set;
	prefs = setPrivate->prefs;

	interfaces = _SCNetworkInterfaceCopyAllWithPreferences(prefs);
	n_interfaces = CFArrayGetCount(interfaces);
	if (n_interfaces == 0) {
		return interfaces;
	}

	if (prefs != NULL) {
		CFArrayRef	bridges	= NULL;

		excluded = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);

#if	!TARGET_OS_IPHONE
		CFArrayRef	bonds	= NULL;

		bonds = SCBondInterfaceCopyAll(prefs);
		if (bonds != NULL) {
			__SCBondInterfaceListCollectMembers(bonds, excluded);
			CFRelease(bonds);
		}
#endif	/* !TARGET_OS_IPHONE */

		bridges = SCBridgeInterfaceCopyAll(prefs);
		if (bridges != NULL) {
			__SCBridgeInterfaceListCollectMembers(bridges, excluded);
			CFRelease(bridges);
		}

		n_exclusions = CFSetGetCount(excluded);
	}

	if (n_exclusions == 0) {
		if (excluded != NULL) {
			CFRelease(excluded);
		}

		return interfaces;
	}

	available = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	for (i = 0; i < n_interfaces; i++) {
		SCNetworkInterfaceRef	interface;

		interface = CFArrayGetValueAtIndex(interfaces, i);
		if (CFSetContainsValue(excluded, interface)) {
			// if excluded
			continue;
		}

		CFArrayAppendValue(available, interface);
	}

	CFRelease(interfaces);
	CFRelease(excluded);

	return available;
}


SCNetworkSetRef
SCNetworkSetCopyCurrent(SCPreferencesRef prefs)
{
	CFArrayRef		components;
	CFStringRef		currentID;
	SCNetworkSetPrivateRef	setPrivate	= NULL;

	currentID = SCPreferencesGetValue(prefs, kSCPrefCurrentSet);
	if (!isA_CFString(currentID)) {
		return NULL;
	}

	components = CFStringCreateArrayBySeparatingStrings(NULL, currentID, CFSTR("/"));
	if (CFArrayGetCount(components) == 3) {
		CFStringRef	setID;
		CFStringRef	path;

		setID = CFArrayGetValueAtIndex(components, 2);
		path = SCPreferencesPathKeyCreateSet(NULL, setID);
		if (CFEqual(path, currentID)) {
			setPrivate = __SCNetworkSetCreatePrivate(NULL, prefs, setID);
			assert(setPrivate != NULL);

			// mark set as "old" (already established)
			setPrivate->established = TRUE;
		} else {
			SCLog(TRUE, LOG_ERR, CFSTR("SCNetworkSetCopyCurrent(): preferences are non-conformant"));
		}
		CFRelease(path);
	}
	CFRelease(components);

	return (SCNetworkSetRef)setPrivate;
}


CFArrayRef /* of SCNetworkServiceRef's */
SCNetworkSetCopyServices(SCNetworkSetRef set)
{
	CFMutableArrayRef       array;
	CFDictionaryRef		dict;
	CFIndex			n;
	CFStringRef		path;
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)set;

	if (!isA_SCNetworkSet(set)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	path = SCPreferencesPathKeyCreateSetNetworkService(NULL, setPrivate->setID, NULL);
	dict = SCPreferencesPathGetValue(setPrivate->prefs, path);
	CFRelease(path);
	if ((dict != NULL) && !isA_CFDictionary(dict)) {
		return NULL;
	}

	array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	n = (dict != NULL) ? CFDictionaryGetCount(dict) : 0;
	if (n > 0) {
		CFIndex		i;
		const void *    keys_q[N_QUICK];
		const void **   keys	= keys_q;

		if (n > (CFIndex)(sizeof(keys_q) / sizeof(CFTypeRef))) {
			keys = CFAllocatorAllocate(NULL, n * sizeof(CFTypeRef), 0);
		}
		CFDictionaryGetKeysAndValues(dict, keys, NULL);
		for (i = 0; i < n; i++) {
			CFArrayRef	components;
			CFStringRef	link;

			path = SCPreferencesPathKeyCreateSetNetworkServiceEntity(NULL,
										 setPrivate->setID,
										 (CFStringRef)keys[i],
										 NULL);
			link = SCPreferencesPathGetLink(setPrivate->prefs, path);
			CFRelease(path);
			if (link == NULL) {
				SCLog(TRUE,
				      LOG_INFO,
				      CFSTR("SCNetworkSetCopyServices(): service \"%@\" for set \"%@\" is not a link\n"),
				      keys[i],
				      setPrivate->setID);
				continue;	 // if the service is not a link
			}

			components = CFStringCreateArrayBySeparatingStrings(NULL, link, CFSTR("/"));
			if (CFArrayGetCount(components) == 3) {
				CFStringRef serviceID;

				serviceID = CFArrayGetValueAtIndex(components, 2);
				path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,		// allocator
										      serviceID,	// service
										      NULL);		// entity
				if (CFEqual(path, link)) {
					SCNetworkServicePrivateRef	servicePrivate;

					servicePrivate = __SCNetworkServiceCreatePrivate(NULL,
											 setPrivate->prefs,
											 serviceID,
											 NULL);
					CFArrayAppendValue(array, (SCNetworkServiceRef)servicePrivate);
					CFRelease(servicePrivate);
				}
				CFRelease(path);
			}
			CFRelease(components);
		}
		if (keys != keys_q) {
			CFAllocatorDeallocate(NULL, keys);
		}
	}

	return array;
}


SCNetworkSetRef
SCNetworkSetCreate(SCPreferencesRef prefs)
{
	CFArrayRef		components;
	CFDictionaryRef		entity;
	Boolean			ok;
	CFStringRef		path;
	CFStringRef		prefix;
	CFStringRef		setID;
	SCNetworkSetPrivateRef	setPrivate;

	prefix = SCPreferencesPathKeyCreateSets(NULL);
	path = __SCPreferencesPathCreateUniqueChild_WithMoreSCFCompatibility(prefs, prefix);
	if (path == NULL) path = SCPreferencesPathCreateUniqueChild(prefs, prefix);
	CFRelease(prefix);
	if (path == NULL) {
		return NULL;
	}

	components = CFStringCreateArrayBySeparatingStrings(NULL, path, CFSTR("/"));
	setID = CFArrayGetValueAtIndex(components, 2);
	setPrivate = __SCNetworkSetCreatePrivate(NULL, prefs, setID);
	assert(setPrivate != NULL);
	CFRelease(components);

	// mark set as "new" (not yet established)
	setPrivate->established = FALSE;

	// establish the set in the preferences
	entity = CFDictionaryCreate(NULL,
				    NULL, NULL, 0,
				    &kCFTypeDictionaryKeyCallBacks,
				    &kCFTypeDictionaryValueCallBacks);
	ok = SCPreferencesPathSetValue(prefs, path, entity);
	CFRelease(path);
	CFRelease(entity);
	if (!ok) {
		CFRelease(setPrivate);
		setPrivate = NULL;
	}

	return (SCNetworkSetRef)setPrivate;
}


CFStringRef
SCNetworkSetGetSetID(SCNetworkSetRef set)
{
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)set;

	if (!isA_SCNetworkSet(set)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	return setPrivate->setID;
}


CFStringRef
SCNetworkSetGetName(SCNetworkSetRef set)
{
	CFBundleRef		bundle;
	CFDictionaryRef		entity;
	CFStringRef		path;
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)set;

	if (!isA_SCNetworkSet(set)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	if (setPrivate->name != NULL) {
		return setPrivate->name;
	}

	path = SCPreferencesPathKeyCreateSet(NULL, setPrivate->setID);
	entity = SCPreferencesPathGetValue(setPrivate->prefs, path);
	CFRelease(path);

	if (isA_CFDictionary(entity)) {
		CFStringRef	name;

		name = CFDictionaryGetValue(entity, kSCPropUserDefinedName);
		if (isA_CFString(name)) {
			setPrivate->name = CFRetain(name);
		}
	}

	bundle = _SC_CFBundleGet();
	if (bundle != NULL) {
		if (setPrivate->name != NULL) {
			CFStringRef	non_localized;

			non_localized = _SC_CFBundleCopyNonLocalizedString(bundle,
									   CFSTR("DEFAULT_SET_NAME"),
									   CFSTR("Automatic"),
									   NULL);
			if (non_localized != NULL) {
				if (CFEqual(setPrivate->name, non_localized)) {
					CFStringRef	localized;

					// if "Automatic", return localized name
					localized = CFBundleCopyLocalizedString(bundle,
										CFSTR("DEFAULT_SET_NAME"),
										CFSTR("Automatic"),
										NULL);
					if (localized != NULL) {
						CFRelease(setPrivate->name);
						setPrivate->name = localized;
					}
				}

				CFRelease(non_localized);
			}
		}
	}

	return setPrivate->name;
}


CFArrayRef /* of serviceID CFStringRef's */
SCNetworkSetGetServiceOrder(SCNetworkSetRef set)
{
	CFDictionaryRef		dict;
	CFStringRef		path;
	CFArrayRef		serviceOrder;
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)set;

	if (!isA_SCNetworkSet(set)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	path = SCPreferencesPathKeyCreateSetNetworkGlobalEntity(NULL, setPrivate->setID, kSCEntNetIPv4);
	if (path == NULL) {
		return NULL;
	}

	dict = SCPreferencesPathGetValue(setPrivate->prefs, path);
	CFRelease(path);
	if (!isA_CFDictionary(dict)) {
		return NULL;
	}

	serviceOrder = CFDictionaryGetValue(dict, kSCPropNetServiceOrder);
	serviceOrder = isA_CFArray(serviceOrder);

	return serviceOrder;
}


CFTypeID
SCNetworkSetGetTypeID(void)
{
	pthread_once(&initialized, __SCNetworkSetInitialize);	/* initialize runtime */
	return __kSCNetworkSetTypeID;
}


Boolean
SCNetworkSetRemove(SCNetworkSetRef set)
{
	CFStringRef		currentPath;
	Boolean			ok		= FALSE;
	CFStringRef		path;
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)set;

	if (!isA_SCNetworkSet(set)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	currentPath = SCPreferencesGetValue(setPrivate->prefs, kSCPrefCurrentSet);
	path = SCPreferencesPathKeyCreateSet(NULL, setPrivate->setID);
	if (!isA_CFString(currentPath) || !CFEqual(currentPath, path)) {
		ok = SCPreferencesPathRemoveValue(setPrivate->prefs, path);
	} else {
		_SCErrorSet(kSCStatusInvalidArgument);
	}
	CFRelease(path);

	return ok;
}


Boolean
SCNetworkSetRemoveService(SCNetworkSetRef set, SCNetworkServiceRef service)
{
	SCNetworkInterfaceRef		interface;
	CFArrayRef			interface_config	= NULL;
	Boolean				ok;
	CFStringRef			path;
	int				sc_status		= kSCStatusOK;
	SCNetworkServicePrivateRef	servicePrivate		= (SCNetworkServicePrivateRef)service;
	SCNetworkSetPrivateRef		setPrivate		= (SCNetworkSetPrivateRef)set;

	if (!isA_SCNetworkSet(set)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (!isA_SCNetworkService(service) || (servicePrivate->prefs == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	// remove service from ServiceOrder
	_serviceOrder_remove(set, service);

	// get the [deep] interface configuration settings
	interface = SCNetworkServiceGetInterface(service);
	if (interface != NULL) {
		interface_config = __SCNetworkInterfaceCopyDeepConfiguration(set, interface);
		if (interface_config != NULL) {
			// remove the interface configuration from all sets which contain this service.
			__SCNetworkInterfaceSetDeepConfiguration(set, interface, NULL);
		}
	}

	// remove the link between "set" and the "service"
	path = SCPreferencesPathKeyCreateSetNetworkServiceEntity(NULL,
								 setPrivate->setID,
								 servicePrivate->serviceID,
								 NULL);
	ok = SCPreferencesPathRemoveValue(setPrivate->prefs, path);
	if (!ok) {
		sc_status = SCError();	// preserve the error
	}
	CFRelease(path);

	// push the [deep] interface configuration [back] into all sets which contain the service.
	if (interface_config != NULL) {
		__SCNetworkInterfaceSetDeepConfiguration(set, interface, interface_config);
	}

	if (interface_config != NULL)     CFRelease(interface_config);
	if (!ok) {
		_SCErrorSet(sc_status);
	}
	return ok;
}


Boolean
SCNetworkSetSetCurrent(SCNetworkSetRef set)
{
	Boolean			ok;
	CFStringRef		path;
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)set;

	if (!isA_SCNetworkSet(set)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	path = SCPreferencesPathKeyCreateSet(NULL, setPrivate->setID);
	ok = SCPreferencesSetValue(setPrivate->prefs, kSCPrefCurrentSet, path);
	CFRelease(path);
	return ok;
}


Boolean
SCNetworkSetSetName(SCNetworkSetRef set, CFStringRef name)
{
	CFBundleRef		bundle		= NULL;
	CFDictionaryRef		entity;
	CFStringRef		localized	= NULL;
	CFStringRef		non_localized	= NULL;
	Boolean			ok		= FALSE;
	CFStringRef		path;
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)set;

	if (!isA_SCNetworkSet(set)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if ((name != NULL) && !isA_CFString(name)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	// if known, compare against localized name

	if (name != NULL) {
		bundle = _SC_CFBundleGet();
		if (bundle != NULL) {
			non_localized = _SC_CFBundleCopyNonLocalizedString(bundle,
									   CFSTR("DEFAULT_SET_NAME"),
									   CFSTR("Automatic"),
									   NULL);
			if (non_localized != NULL) {
				if (CFEqual(name, non_localized)) {
					localized = CFBundleCopyLocalizedString(bundle,
										CFSTR("DEFAULT_SET_NAME"),
										CFSTR("Automatic"),
										NULL);
					if (localized != NULL) {
						name = localized;
					}
				}
			}
		}
	}

#define PREVENT_DUPLICATE_SET_NAMES
#ifdef  PREVENT_DUPLICATE_SET_NAMES
	if (name != NULL) {
		CFArrayRef      sets;

		// ensure that each set is uniquely named

		sets = SCNetworkSetCopyAll(setPrivate->prefs);
		if (sets != NULL) {
			CFIndex		i;
			CFIndex		n;

			n = CFArrayGetCount(sets);
			for (i = 0; i < n; i++) {
				CFStringRef     otherID;
				CFStringRef     otherName;
				SCNetworkSetRef set		= CFArrayGetValueAtIndex(sets, i);

				otherID = SCNetworkSetGetSetID(set);
				if (CFEqual(setPrivate->setID, otherID)) {
					continue;       // skip current set
				}

				otherName = SCNetworkSetGetName(set);
				if ((otherName != NULL) && CFEqual(name, otherName)) {
					// if "name" not unique
					CFRelease(sets);
					_SCErrorSet(kSCStatusKeyExists);
					goto done;
				}
			}
			CFRelease(sets);
		}
	}
#endif  /* PREVENT_DUPLICATE_SET_NAMES */

	// if known, store non-localized name

	if ((name != NULL) && (bundle != NULL) && (non_localized != NULL)) {
		if (localized == NULL) {
			localized = CFBundleCopyLocalizedString(bundle,
								CFSTR("DEFAULT_SET_NAME"),
								CFSTR("Automatic"),
								NULL);
		}

		if (localized != NULL) {
			if (CFEqual(name, localized)) {
				name = non_localized;
			}
		}
	}

	// update the "name"

	path = SCPreferencesPathKeyCreateSet(NULL, setPrivate->setID);
	entity = SCPreferencesPathGetValue(setPrivate->prefs, path);
	if (isA_CFDictionary(entity) ||
	    ((entity == NULL) && (name != NULL))) {
		CFMutableDictionaryRef	newEntity;

		if (entity != NULL) {
			newEntity = CFDictionaryCreateMutableCopy(NULL, 0, entity);
		} else {
			newEntity = CFDictionaryCreateMutable(NULL,
							      0,
							      &kCFTypeDictionaryKeyCallBacks,
							      &kCFTypeDictionaryValueCallBacks);
		}
		if (name != NULL) {
			CFDictionarySetValue(newEntity, kSCPropUserDefinedName, name);
		} else {
			CFDictionaryRemoveValue(newEntity, kSCPropUserDefinedName);
		}
		ok = SCPreferencesPathSetValue(setPrivate->prefs, path, newEntity);
		CFRelease(newEntity);
	}
	CFRelease(path);

    done :

	if (localized != NULL)		CFRelease(localized);
	if (non_localized != NULL)	CFRelease(non_localized);
	return ok;
}


Boolean
SCNetworkSetSetServiceOrder(SCNetworkSetRef set, CFArrayRef newOrder)
{
	CFDictionaryRef		dict;
	CFMutableDictionaryRef  newDict;
	Boolean			ok;
	CFStringRef		path;
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)set;

	if (!isA_SCNetworkSet(set)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (isA_CFArray(newOrder)) {
		CFIndex	i;
		CFIndex	n	= CFArrayGetCount(newOrder);

		for (i = 0; i < n; i++) {
			CFStringRef	serviceID;

			serviceID = CFArrayGetValueAtIndex(newOrder, i);
			if (!isA_CFString(serviceID)) {
				_SCErrorSet(kSCStatusInvalidArgument);
				return FALSE;
			}
		}
	} else {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	path = SCPreferencesPathKeyCreateSetNetworkGlobalEntity(NULL, setPrivate->setID, kSCEntNetIPv4);
	if (path == NULL) {
		return FALSE;
	}

	dict = SCPreferencesPathGetValue(setPrivate->prefs, path);
	if (dict != NULL) {
		newDict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
	} else {
		newDict = CFDictionaryCreateMutable(NULL,
						    0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);
	}

	CFDictionarySetValue(newDict, kSCPropNetServiceOrder, newOrder);
	ok = SCPreferencesPathSetValue(setPrivate->prefs, path, newDict);
	CFRelease(newDict);
	CFRelease(path);

	return ok;
}


#pragma mark -
#pragma mark SCNetworkSet SPIs


static void
add_supported_interfaces(CFMutableArrayRef interface_list, SCNetworkInterfaceRef interface)
{
	CFIndex		i;
	CFArrayRef	interface_types;
	CFIndex		n;

	interface_types = SCNetworkInterfaceGetSupportedInterfaceTypes(interface);
	n = (interface_types != NULL) ? CFArrayGetCount(interface_types) : 0;
	for (i = 0; i < n; i++) {
		SCNetworkInterfaceRef	parent;
		CFStringRef		interface_type;

		interface_type = CFArrayGetValueAtIndex(interface_types, i);
		parent = SCNetworkInterfaceCreateWithInterface(interface, interface_type);
		if (parent != NULL) {
			CFArrayAppendValue(interface_list, parent);
			CFRelease(parent);
		}
	}

	return;
}


static CFSetRef	/* of SCNetworkInterfaceRef's */
copyExcludedInterfaces(SCPreferencesRef prefs)
{
	CFMutableSetRef	excluded;
	CFArrayRef	interfaces;

	excluded = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);

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

	return excluded;
}


#if	!TARGET_OS_IPHONE
static SCBridgeInterfaceRef
copyAutoBridgeInterface(SCPreferencesRef prefs, CFStringRef bridgeName)
{
	SCBridgeInterfaceRef	bridge		= NULL;
	CFArrayRef		interfaces;

	// exclude Bridge [member] interfaces
	interfaces = SCBridgeInterfaceCopyAll(prefs);
	if (interfaces != NULL) {
		CFIndex		i;
		CFIndex		n;

		n = CFArrayGetCount(interfaces);
		for (i = 0; i < n; i++) {
			SCBridgeInterfaceRef	interface;
			CFStringRef		name	= NULL;
			CFDictionaryRef		options;

			interface = CFArrayGetValueAtIndex(interfaces, i);
			options = SCBridgeInterfaceGetOptions(interface);
			if ((options != NULL) &&
			    CFDictionaryGetValueIfPresent(options,
							  CFSTR("__AUTO__"),
							  (const void **)&name) &&
			    _SC_CFEqual(name, bridgeName)) {
				bridge = interface;
				CFRetain(bridge);
				break;
			}
		}

		CFRelease(interfaces);
	}

	if (bridge == NULL) {
		bridge = SCBridgeInterfaceCreate(prefs);
		if (bridge != NULL) {
			CFMutableDictionaryRef	newOptions;
			Boolean			ok;

			newOptions = CFDictionaryCreateMutable(NULL, 0,
							       &kCFTypeDictionaryKeyCallBacks,
							       &kCFTypeDictionaryValueCallBacks);
			CFDictionarySetValue(newOptions, CFSTR("__AUTO__"), bridgeName);
			ok = SCBridgeInterfaceSetOptions(bridge, newOptions);
			CFRelease(newOptions);
			if (!ok) {
				CFRelease(bridge);
				bridge = NULL;
			}
		}
	}

	return bridge;
}
#endif	// !TARGET_OS_IPHONE


static CFArrayRef
copyServices(SCNetworkSetRef set)
{
	CFArrayRef		services;
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)set;

	// first, assume that we only want to add new services
	// for those interfaces that are not represented in the
	// current set.
	services = SCNetworkSetCopyServices(set);
	if ((services != NULL) && setPrivate->established) {
		// but, if we are given an existing (or "established") set
		// than we only want to add new services for those interfaces
		// that are not represented in *any* set.
		CFRelease(services);
		services = SCNetworkServiceCopyAll(setPrivate->prefs);
	}

	return services;
}


#if	!TARGET_OS_IPHONE
static CFArrayRef
updateServices(CFArrayRef services, SCNetworkInterfaceRef interface)
{
	CFStringRef		bsdName;
	CFIndex			i;
	CFIndex			n;
	CFMutableArrayRef	newServices;

	if (services == NULL) {
		return NULL;
	}

	bsdName = SCNetworkInterfaceGetBSDName(interface);

	newServices = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	n = CFArrayGetCount(services);
	for (i = 0; i < n; i++) {
		SCNetworkInterfaceRef		interface;
		CFStringRef			interfaceName;
		SCNetworkServiceRef		newService;
		SCNetworkServiceRef		service;
		CFStringRef			serviceID;
		SCNetworkServicePrivateRef	servicePrivate;

		service = CFArrayGetValueAtIndex(services, i);
		interface = SCNetworkServiceGetInterface(service);
		interfaceName = SCNetworkInterfaceGetBSDName(interface);
		if (!_SC_CFEqual(interfaceName, bsdName)) {
			// if not a match, retain
			CFArrayAppendValue(newServices, service);
			continue;
		}

		// if a match, update
		serviceID = SCNetworkServiceGetServiceID(service);
		servicePrivate = (SCNetworkServicePrivateRef)service;
		newService = SCNetworkServiceCopy(servicePrivate->prefs, serviceID);
		if (newService != NULL) {
			CFArrayAppendValue(newServices, newService);
			CFRelease(newService);
		}
	}

	return newServices;
}
#endif	// !TARGET_OS_IPHONE


static __inline__ Boolean
skipInterface(SCNetworkInterfaceRef interface)
{
	CFStringRef	action;

	action = _SCNetworkInterfaceGetConfigurationAction(interface);
	if (isA_CFString(action) &&
	    CFEqual(action, kSCNetworkInterfaceConfigurationActionValueNone)) {
		return TRUE;
	}

	return FALSE;
}


static Boolean
__SCNetworkSetEstablishDefaultConfigurationForInterfaces(SCNetworkSetRef set, CFArrayRef interfaces, Boolean excludeHidden)
{
	CFSetRef		excluded	= NULL;
	CFIndex			i;
	CFIndex			n		= 0;
	Boolean			ok		= TRUE;
	CFArrayRef		services;
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)set;
	Boolean			updated		= FALSE;
	Boolean			updatedIFs	= FALSE;

#if	TARGET_OS_IPHONE
	CFArrayRef		orphans		= NULL;
	CFArrayRef		sets;

	sets = SCNetworkSetCopyAll(setPrivate->prefs);
	if (sets != NULL) {
		if (CFArrayGetCount(sets) == 1) {
			services = SCNetworkSetCopyServices(set);
			if (services != NULL) {
				n = CFArrayGetCount(services);
				CFRelease(services);
			}

			if ((n == 0) && CFEqual(set, CFArrayGetValueAtIndex(sets, 0))) {
				// after a "Reset Network Settings" we need to find (and
				// add back) any VPN services that were orphaned.
				orphans = SCNetworkServiceCopyAll(setPrivate->prefs);
			}
		}

		CFRelease(sets);
	}
#endif	// TARGET_OS_IPHONE

	// copy network services
	services = copyServices(set);

	// copy network interfaces to be excluded
	excluded = copyExcludedInterfaces(setPrivate->prefs);

#if	!TARGET_OS_IPHONE
	// look for interfaces that should auto-magically be added
	// to an Ethernet bridge
	n = (interfaces != NULL) ? CFArrayGetCount(interfaces) : 0;
	for (i = 0; i < n; i++) {
		SCBridgeInterfaceRef	bridge		= NULL;
		SCNetworkInterfaceRef	interface;

		interface = CFArrayGetValueAtIndex(interfaces, i);

		if (excludeHidden && skipInterface(interface)) {
			// if not auto-configure
			continue;
		}

		if ((excluded != NULL)
		    && CFSetContainsValue(excluded, interface)) {
			// if this interface is a member of a Bond or Bridge
			continue;
		}

		if (__SCNetworkServiceExistsForInterface(services, interface)) {
			// if this is not a new interface
			continue;
		}

		if (_SCNetworkInterfaceIsBuiltin(interface) &&
		    _SCNetworkInterfaceIsThunderbolt(interface) &&
		    !isA_SCBridgeInterface(interface)) {
			// add built-in Thunderbolt interfaces to bridge
			bridge = copyAutoBridgeInterface(setPrivate->prefs, CFSTR("thunderbolt-bridge"));
		}

		if (bridge != NULL) {
			CFIndex			bridgeIndex;
			CFArrayRef		members;
			CFMutableArrayRef	newMembers;
			CFMutableSetRef		newExcluded;
			CFMutableArrayRef	newInterfaces;
			CFArrayRef		newServices;

			// track the bridge interface (if it's in our list)
			bridgeIndex = CFArrayGetFirstIndexOfValue(interfaces,
								  CFRangeMake(0, CFArrayGetCount(interfaces)),
								  bridge);

			// add new member interface
			members = SCBridgeInterfaceGetMemberInterfaces(bridge);
			if ((members != NULL) && (CFArrayGetCount(members) > 0)) {
				newMembers = CFArrayCreateMutableCopy(NULL, 0, members);
				updated = TRUE;		// if we're updating an existing bridge
			} else {
				newMembers = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
			}
			CFArrayAppendValue(newMembers, interface);
			ok = SCBridgeInterfaceSetMemberInterfaces(bridge, newMembers);
			CFRelease(newMembers);
			if (!ok) {
				SCLog(TRUE, LOG_DEBUG,
				      CFSTR("could not update bridge with \"%@\": %s\n"),
				      SCNetworkInterfaceGetLocalizedDisplayName(interface),
				      SCErrorString(SCError()));
				CFRelease(bridge);
				continue;
			}

			// exclude the new member interface
			newExcluded = CFSetCreateMutableCopy(NULL, 0, excluded);
			CFRelease(excluded);
			CFSetAddValue(newExcluded, interface);
			excluded = newExcluded;

			// update the list of interfaces to include the [new or updated] bridge
			newInterfaces = CFArrayCreateMutableCopy(NULL, 0, interfaces);
			if (bridgeIndex != kCFNotFound) {
				CFArraySetValueAtIndex(newInterfaces, bridgeIndex, bridge);
			} else {
				CFArrayAppendValue(newInterfaces, bridge);
			}
			if (updatedIFs) {
				CFRelease(interfaces);
			}
			interfaces = newInterfaces;
			updatedIFs = TRUE;

			// refresh [existing] services
			newServices = updateServices(services, bridge);
			if (newServices != NULL) {
				CFRelease(services);
				services = newServices;
			}

			CFRelease(bridge);
		}
	}
#endif	// !TARGET_OS_IPHONE

	n = (interfaces != NULL) ? CFArrayGetCount(interfaces) : 0;
	for (i = 0; i < n; i++) {
		SCNetworkInterfaceRef	interface;
		CFMutableArrayRef	interface_list;

		interface = CFArrayGetValueAtIndex(interfaces, i);

		if (excludeHidden && skipInterface(interface)) {
			// if not auto-configure
			continue;
		}

		if ((excluded != NULL)
		    && CFSetContainsValue(excluded, interface)) {
			// if this interface is a member of a Bond or Bridge
			continue;
		}

		if (__SCNetworkServiceExistsForInterface(services, interface)) {
			// if this is not a new interface
			continue;
		}

		interface_list = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		CFArrayAppendValue(interface_list, interface);

		while (ok && (CFArrayGetCount(interface_list) > 0)) {
			CFArrayRef		protocol_types;

			interface = CFArrayGetValueAtIndex(interface_list, 0);

			protocol_types = SCNetworkInterfaceGetSupportedProtocolTypes(interface);
			if ((protocol_types != NULL) && (CFArrayGetCount(protocol_types) > 0)) {
				SCNetworkServiceRef	service;

				service = SCNetworkServiceCreate(setPrivate->prefs, interface);
				if (service == NULL) {
					SCLog(TRUE, LOG_DEBUG,
					      CFSTR("could not create service for \"%@\": %s\n"),
					      SCNetworkInterfaceGetLocalizedDisplayName(interface),
					      SCErrorString(SCError()));
					ok = FALSE;
					goto nextInterface;
				}

				ok = SCNetworkServiceEstablishDefaultConfiguration(service);
				if (!ok) {
					SCLog(TRUE, LOG_DEBUG,
					      CFSTR("could not estabish default configuration for \"%@\": %s\n"),
					      SCNetworkInterfaceGetLocalizedDisplayName(interface),
					      SCErrorString(SCError()));
					SCNetworkServiceRemove(service);
					CFRelease(service);
					goto nextInterface;
				}

				ok = SCNetworkSetAddService(set, service);
				if (!ok) {
					SCLog(TRUE, LOG_DEBUG,
					      CFSTR("could not add service for \"%@\": %s\n"),
					      SCNetworkInterfaceGetLocalizedDisplayName(interface),
					      SCErrorString(SCError()));
					SCNetworkServiceRemove(service);
					CFRelease(service);
					goto nextInterface;
				}

				CFRelease(service);
				updated = TRUE;
			} else {
				add_supported_interfaces(interface_list, interface);
			}

		    nextInterface :

			CFArrayRemoveValueAtIndex(interface_list, 0);
		}
		CFRelease(interface_list);
	}
	if (updatedIFs)		CFRelease(interfaces);
	if (services != NULL)	CFRelease(services);
	if (excluded != NULL)	CFRelease(excluded);

#if	TARGET_OS_IPHONE
	if (orphans != NULL) {
		if (ok && updated) {
			CFIndex	i;
			CFIndex	n	= CFArrayGetCount(orphans);

			for (i = 0; i < n; i++) {
				SCNetworkServiceRef	service;

				service = CFArrayGetValueAtIndex(orphans, i);
				if (_SCNetworkServiceIsVPN(service)) {
					ok = SCNetworkSetAddService(set, service);
					if (!ok) {
						break;
					}
				}
			}
		}

		CFRelease(orphans);
	}
#endif	// TARGET_OS_IPHONE

	if (ok && !updated) {
		// if no changes were made
		_SCErrorSet(kSCStatusOK);
	}

	return updated;
}


Boolean
SCNetworkSetEstablishDefaultConfiguration(SCNetworkSetRef set)
{
	CFArrayRef		interfaces;
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)set;
	Boolean			updated		= FALSE;

	if (!isA_SCNetworkSet(set)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	interfaces = _SCNetworkInterfaceCopyAllWithPreferences(setPrivate->prefs);
	if (interfaces != NULL) {
		updated = __SCNetworkSetEstablishDefaultConfigurationForInterfaces(set, interfaces, TRUE);
		CFRelease(interfaces);
	}

	return updated;
}


Boolean
SCNetworkSetEstablishDefaultInterfaceConfiguration(SCNetworkSetRef set, SCNetworkInterfaceRef interface)
{
	CFArrayRef	interfaces;
	Boolean		updated;

	if (!isA_SCNetworkSet(set)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (!isA_SCNetworkInterface(interface)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	interfaces = CFArrayCreate(NULL, (const void **)&interface, 1, &kCFTypeArrayCallBacks);
	assert(interfaces != NULL);
	updated = __SCNetworkSetEstablishDefaultConfigurationForInterfaces(set, interfaces, FALSE);
	CFRelease(interfaces);

	return updated;
}


SCNetworkServiceRef
SCNetworkSetCopySelectedVPNService(SCNetworkSetRef set)
{
	CFIndex			i;
	CFIndex			n;
	SCNetworkServiceRef	selected	= NULL;
	CFArrayRef		services;
	CFMutableArrayRef	services_vpn	= NULL;

	if (!isA_SCNetworkSet(set)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	services = SCNetworkSetCopyServices(set);
	if (services != NULL) {
		n = CFArrayGetCount(services);
		for (i = 0; i < n; i++) {
			SCNetworkServiceRef	service;

			service = CFArrayGetValueAtIndex(services, i);
			if (!SCNetworkServiceGetEnabled(service)) {
				// if not enabled
				continue;
			}

			if (!_SCNetworkServiceIsVPN(service)) {
				// if not VPN service
				continue;
			}

			if (services_vpn == NULL) {
				services_vpn = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
			}
			CFArrayAppendValue(services_vpn, service);
		}

		CFRelease(services);
	}

	if (services_vpn == NULL) {
		// if no VPN services
		return NULL;
	}

	n = CFArrayGetCount(services_vpn);
	if (n > 1) {
		CFArrayRef		order;
		CFMutableArrayRef	sorted;

		order = SCNetworkSetGetServiceOrder(set);
		sorted = CFArrayCreateMutableCopy(NULL, 0, services_vpn);
		CFArraySortValues(sorted,
				  CFRangeMake(0, CFArrayGetCount(sorted)),
				  _SCNetworkServiceCompare,
				  (void *)order);
		CFRelease(services_vpn);
		services_vpn = sorted;
	}

#if	TARGET_OS_IPHONE
	if (n > 1) {
		CFStringRef	serviceID_prefs;

#define VPN_PREFERENCES	CFSTR("com.apple.mobilevpn")
#define VPN_SERVICE_ID	CFSTR("activeVPNID")

		CFPreferencesAppSynchronize(VPN_PREFERENCES);
		serviceID_prefs = CFPreferencesCopyAppValue(VPN_SERVICE_ID, VPN_PREFERENCES);
		if (serviceID_prefs != NULL) {
			for (i = 0; i < n; i++) {
				SCNetworkServiceRef	service;
				CFStringRef		serviceID;

				service = CFArrayGetValueAtIndex(services_vpn, i);
				serviceID = SCNetworkServiceGetServiceID(service);
				if (CFEqual(serviceID, serviceID_prefs)) {
					selected = service;
					CFRetain(selected);
					break;
				}

			}

			CFRelease(serviceID_prefs);
		}
	}
#endif	// TARGET_OS_IPHONE

	if (selected == NULL) {
		selected = CFArrayGetValueAtIndex(services_vpn, 0);
		CFRetain(selected);
	}

	CFRelease(services_vpn);
	return selected;
}


Boolean
SCNetworkSetSetSelectedVPNService(SCNetworkSetRef set, SCNetworkServiceRef service)
{
	Boolean		ok	= TRUE;
	CFArrayRef	services;

	if (!isA_SCNetworkSet(set)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (!isA_SCNetworkService(service) || !_SCNetworkServiceIsVPN(service)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	services = SCNetworkSetCopyServices(set);
	if (services != NULL) {
		CFIndex	i;
		CFIndex	n	= CFArrayGetCount(services);

		if (!CFArrayContainsValue(services, CFRangeMake(0, n), service)) {
			// if selected service not a member of the current set
			_SCErrorSet(kSCStatusInvalidArgument);
			ok = FALSE;
			goto done;
		}

		for (i = 0; ok && (i < n); i++) {
			SCNetworkServiceRef	vpn;

			vpn = CFArrayGetValueAtIndex(services, i);
			if (!_SCNetworkServiceIsVPN(vpn)) {
				// if not VPN service
				continue;
			}

			ok = SCNetworkServiceSetEnabled(vpn, CFEqual(service, vpn));
		}
	}

    done :

	if (services != NULL) CFRelease(services);
	return ok;
}


Boolean
_SCNetworkSetSetSetID(SCNetworkSetRef set, CFStringRef newSetID)
{
	SCNetworkSetRef		currentSet		= NULL;
	SCNetworkSetPrivateRef	currentSetPrivate	= NULL;
	CFDictionaryRef		entity;
	CFStringRef		newPath;
	Boolean			ok			= FALSE;
	CFStringRef		oldPath			= NULL;
	SCNetworkSetPrivateRef	setPrivate		= (SCNetworkSetPrivateRef)set;
	Boolean			updateCurrentSet	= FALSE;

	if (!isA_SCNetworkSet(set)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (!isA_CFString(newSetID)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	// If newSetID is equal to current setID, our work is done
	if (CFEqual(newSetID, setPrivate->setID)) {
		return TRUE;
	}

	newPath = SCPreferencesPathKeyCreateSet(NULL, newSetID);
	entity = SCPreferencesPathGetValue(setPrivate->prefs, newPath);
	if (isA_CFDictionary(entity)) {
		// if the new set already exists
		_SCErrorSet(kSCStatusKeyExists);
		goto done;
	}

	oldPath = SCPreferencesPathKeyCreateSet(NULL, setPrivate->setID);
	entity = SCPreferencesPathGetValue(setPrivate->prefs, oldPath);
	if (!isA_CFDictionary(entity)) {
		// if the set has already been removed
		_SCErrorSet(kSCStatusNoKey);
		goto done;
	}

	ok = SCPreferencesPathSetValue(setPrivate->prefs, newPath, entity);
	if (!ok) {
		goto done;
	}

	ok = SCPreferencesPathRemoveValue(setPrivate->prefs, oldPath);
	if (!ok) {
		goto done;
	}

	// update current set (if needed)
	currentSet = SCNetworkSetCopyCurrent(setPrivate->prefs);
	if (currentSet != NULL) {
		currentSetPrivate = (SCNetworkSetPrivateRef)currentSet;
		if (CFEqual(currentSetPrivate->setID, setPrivate->setID)) {
			updateCurrentSet = TRUE;
		}
		CFRelease(currentSet);
	}

	CFRetain(newSetID);
	CFRelease(setPrivate->setID);

	setPrivate->setID = newSetID;

	if (updateCurrentSet) {
		SCNetworkSetSetCurrent(set);
	}

    done:

	if (oldPath != NULL) {
		CFRelease(oldPath);
	}
	if (newPath != NULL) {
		CFRelease(newPath);
	}

	return ok;
}

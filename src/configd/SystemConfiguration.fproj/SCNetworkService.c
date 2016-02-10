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
#include "SCPreferencesInternal.h"

#include <pthread.h>

#define EXTERNAL_ID_DOMAIN_PREFIX	"_"

static CFStringRef	__SCNetworkServiceCopyDescription	(CFTypeRef cf);
static void		__SCNetworkServiceDeallocate		(CFTypeRef cf);
static Boolean		__SCNetworkServiceEqual			(CFTypeRef cf1, CFTypeRef cf2);
static CFHashCode	__SCNetworkServiceHash			(CFTypeRef cf);


static CFTypeID __kSCNetworkServiceTypeID		= _kCFRuntimeNotATypeID;


static const CFRuntimeClass __SCNetworkServiceClass = {
	0,					// version
	"SCNetworkService",			// className
	NULL,					// init
	NULL,					// copy
	__SCNetworkServiceDeallocate,		// dealloc
	__SCNetworkServiceEqual,		// equal
	__SCNetworkServiceHash,			// hash
	NULL,					// copyFormattingDesc
	__SCNetworkServiceCopyDescription	// copyDebugDesc
};


static pthread_once_t		initialized		= PTHREAD_ONCE_INIT;


static CFStringRef
__SCNetworkServiceCopyDescription(CFTypeRef cf)
{
	CFAllocatorRef			allocator	= CFGetAllocator(cf);
	CFMutableStringRef		result;
	SCNetworkServicePrivateRef	servicePrivate	= (SCNetworkServicePrivateRef)cf;

	result = CFStringCreateMutable(allocator, 0);
	CFStringAppendFormat(result, NULL, CFSTR("<SCNetworkService %p [%p]> {"), cf, allocator);
	CFStringAppendFormat(result, NULL, CFSTR("id = %@"), servicePrivate->serviceID);
	if (servicePrivate->prefs != NULL) {
		CFStringAppendFormat(result, NULL, CFSTR(", prefs = %p"), servicePrivate->prefs);
	} else if (servicePrivate->store != NULL) {
		CFStringAppendFormat(result, NULL, CFSTR(", store = %p"), servicePrivate->store);
	}
	if (servicePrivate->name != NULL) {
		CFStringAppendFormat(result, NULL, CFSTR(", name = %@"), servicePrivate->name);
	}
	CFStringAppendFormat(result, NULL, CFSTR("}"));

	return result;
}


static void
__SCNetworkServiceDeallocate(CFTypeRef cf)
{
	SCNetworkServicePrivateRef	servicePrivate	= (SCNetworkServicePrivateRef)cf;

	/* release resources */

	CFRelease(servicePrivate->serviceID);
	if (servicePrivate->interface != NULL) CFRelease(servicePrivate->interface);
	if (servicePrivate->prefs != NULL) CFRelease(servicePrivate->prefs);
	if (servicePrivate->store != NULL) CFRelease(servicePrivate->store);
	if (servicePrivate->name != NULL) CFRelease(servicePrivate->name);
	if (servicePrivate->externalIDs != NULL) CFRelease(servicePrivate->externalIDs);

	return;
}


static Boolean
__SCNetworkServiceEqual(CFTypeRef cf1, CFTypeRef cf2)
{
	SCNetworkServicePrivateRef	s1	= (SCNetworkServicePrivateRef)cf1;
	SCNetworkServicePrivateRef	s2	= (SCNetworkServicePrivateRef)cf2;

	if (s1 == s2)
		return TRUE;

	if (s1->prefs != s2->prefs)
		return FALSE;   // if not the same prefs

	if (!CFEqual(s1->serviceID, s2->serviceID))
		return FALSE;	// if not the same service identifier

	return TRUE;
}


static CFHashCode
__SCNetworkServiceHash(CFTypeRef cf)
{
	SCNetworkServicePrivateRef	servicePrivate	= (SCNetworkServicePrivateRef)cf;

	return CFHash(servicePrivate->serviceID);
}


static void
__SCNetworkServiceInitialize(void)
{
	__kSCNetworkServiceTypeID = _CFRuntimeRegisterClass(&__SCNetworkServiceClass);
	return;
}


__private_extern__ SCNetworkServicePrivateRef
__SCNetworkServiceCreatePrivate(CFAllocatorRef		allocator,
				SCPreferencesRef	prefs,
				CFStringRef		serviceID,
				SCNetworkInterfaceRef   interface)
{
	SCNetworkServicePrivateRef		servicePrivate;
	uint32_t				size;

	/* initialize runtime */
	pthread_once(&initialized, __SCNetworkServiceInitialize);

	/* allocate target */
	size           = sizeof(SCNetworkServicePrivate) - sizeof(CFRuntimeBase);
	servicePrivate = (SCNetworkServicePrivateRef)_CFRuntimeCreateInstance(allocator,
									      __kSCNetworkServiceTypeID,
									      size,
									      NULL);
	if (servicePrivate == NULL) {
		return NULL;
	}

	servicePrivate->prefs		= (prefs != NULL) ? CFRetain(prefs): NULL;
	servicePrivate->serviceID	= CFStringCreateCopy(NULL, serviceID);
	servicePrivate->interface       = (interface != NULL) ? CFRetain(interface) : NULL;
	servicePrivate->name		= NULL;

	return servicePrivate;
}


#pragma mark -
#pragma mark Service ordering


CFComparisonResult
_SCNetworkServiceCompare(const void *val1, const void *val2, void *context)
{
	CFStringRef		id1;
	CFStringRef		id2;
	CFArrayRef		order	= (CFArrayRef)context;
	SCNetworkServiceRef	s1	= (SCNetworkServiceRef)val1;
	SCNetworkServiceRef	s2	= (SCNetworkServiceRef)val2;

	id1 = SCNetworkServiceGetServiceID(s1);
	id2 = SCNetworkServiceGetServiceID(s2);

	if (order != NULL) {
		CFIndex	o1;
		CFIndex	o2;
		CFRange	range;

		range = CFRangeMake(0, CFArrayGetCount(order));
		o1 = CFArrayGetFirstIndexOfValue(order, range, id1);
		o2 = CFArrayGetFirstIndexOfValue(order, range, id2);

		if (o1 > o2) {
			return (o2 != kCFNotFound) ? kCFCompareGreaterThan : kCFCompareLessThan;
		} else if (o1 < o2) {
			return (o1 != kCFNotFound) ? kCFCompareLessThan    : kCFCompareGreaterThan;
		}
	}

	return CFStringCompare(id1, id2, 0);
}


#pragma mark -
#pragma mark SCNetworkService APIs


#define	N_QUICK	64


__private_extern__ CFArrayRef /* of SCNetworkServiceRef's */
__SCNetworkServiceCopyAllEnabled(SCPreferencesRef prefs)
{
	CFMutableArrayRef	array	= NULL;
	CFIndex			i_sets;
	CFIndex			n_sets;
	CFArrayRef		sets;

	sets = SCNetworkSetCopyAll(prefs);
	if (sets == NULL) {
		return NULL;
	}

	n_sets = CFArrayGetCount(sets);
	for (i_sets = 0; i_sets < n_sets; i_sets++) {
		CFIndex		i_services;
		CFIndex		n_services;
		CFArrayRef	services;
		SCNetworkSetRef	set;

		set = CFArrayGetValueAtIndex(sets, i_sets);
		services = SCNetworkSetCopyServices(set);
		if (services == NULL) {
			continue;
		}

		n_services = CFArrayGetCount(services);
		for (i_services = 0; i_services < n_services; i_services++) {
			SCNetworkServiceRef service;

			service = CFArrayGetValueAtIndex(services, i_services);
			if (!SCNetworkServiceGetEnabled(service)) {
				// if not enabled
				continue;
			}

			if ((array == NULL) ||
			    !CFArrayContainsValue(array,
						  CFRangeMake(0, CFArrayGetCount(array)),
						  service)) {
				if (array == NULL) {
					array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
				}
				CFArrayAppendValue(array, service);
			}
		}
		CFRelease(services);
	}
	CFRelease(sets);

	return array;
}


__private_extern__ Boolean
__SCNetworkServiceExistsForInterface(CFArrayRef services, SCNetworkInterfaceRef interface)
{
	CFIndex	i;
	CFIndex	n;

	n = isA_CFArray(services) ? CFArrayGetCount(services) : 0;
	for (i = 0; i < n; i++) {
		SCNetworkServiceRef	service;
		SCNetworkInterfaceRef	service_interface;

		service = CFArrayGetValueAtIndex(services, i);

		service_interface = SCNetworkServiceGetInterface(service);
		while (service_interface != NULL) {
			if (CFEqual(interface, service_interface)) {
				return TRUE;
			}

			service_interface = SCNetworkInterfaceGetInterface(service_interface);
		}
	}

	return FALSE;
}


static void
mergeDict(const void *key, const void *value, void *context)
{
	CFMutableDictionaryRef	newDict	= (CFMutableDictionaryRef)context;

	CFDictionarySetValue(newDict, key, value);
	return;
}


static CF_RETURNS_RETAINED CFDictionaryRef
_protocolTemplate(SCNetworkServiceRef service, CFStringRef protocolType)
{
	SCNetworkInterfaceRef		interface;
	SCNetworkServicePrivateRef      servicePrivate  = (SCNetworkServicePrivateRef)service;
	CFDictionaryRef			template	= NULL;

	interface = servicePrivate->interface;
	if (interface != NULL) {
		SCNetworkInterfaceRef   childInterface;
		CFStringRef		childInterfaceType      = NULL;
		CFStringRef		interfaceType;

		// get the template
		interfaceType = SCNetworkInterfaceGetInterfaceType(servicePrivate->interface);
		childInterface = SCNetworkInterfaceGetInterface(servicePrivate->interface);
		if (childInterface != NULL) {
			childInterfaceType = SCNetworkInterfaceGetInterfaceType(childInterface);
		}

		template = __copyProtocolTemplate(interfaceType, childInterfaceType, protocolType);
		if (template != NULL) {
			CFDictionaryRef		overrides;

			// move to the interface at the lowest layer
			while (childInterface != NULL) {
				interface = childInterface;
				childInterface = SCNetworkInterfaceGetInterface(interface);
			}

			overrides = __SCNetworkInterfaceGetTemplateOverrides(interface, protocolType);
			if (overrides != NULL) {
				CFMutableDictionaryRef	newTemplate;

				newTemplate = CFDictionaryCreateMutableCopy(NULL, 0, template);
				CFDictionaryApplyFunction(overrides, mergeDict, newTemplate);
				CFRelease(template);
				template = newTemplate;
			}
		}
	}

	if (template == NULL) {
		template = CFDictionaryCreate(NULL,
					      NULL,
					      NULL,
					      0,
					      &kCFTypeDictionaryKeyCallBacks,
					      &kCFTypeDictionaryValueCallBacks);
	}

	return template;
}


Boolean
SCNetworkServiceAddProtocolType(SCNetworkServiceRef service, CFStringRef protocolType)
{
	CFDictionaryRef			entity;
	CFDictionaryRef			newEntity       = NULL;
	Boolean				ok		= FALSE;
	CFStringRef			path;
	SCNetworkProtocolRef		protocol;
	SCNetworkServicePrivateRef      servicePrivate  = (SCNetworkServicePrivateRef)service;

	if (!isA_SCNetworkService(service) || (servicePrivate->prefs == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (!__SCNetworkProtocolIsValidType(protocolType)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
							      servicePrivate->serviceID,	// service
							      protocolType);			// entity

	entity = SCPreferencesPathGetValue(servicePrivate->prefs, path);
	if (entity != NULL) {
		// if "protocol" already exists
		_SCErrorSet(kSCStatusKeyExists);
		goto done;
	}

	newEntity = CFDictionaryCreate(NULL,
				       NULL,
				       NULL,
				       0,
				       &kCFTypeDictionaryKeyCallBacks,
				       &kCFTypeDictionaryValueCallBacks);
	ok = SCPreferencesPathSetValue(servicePrivate->prefs, path, newEntity);
	CFRelease(newEntity);
	if (!ok) {
		goto done;
	}

	protocol  = SCNetworkServiceCopyProtocol(service, protocolType);
	assert(protocol != NULL);

	newEntity = _protocolTemplate(service, protocolType);
	ok = SCNetworkProtocolSetConfiguration(protocol, newEntity);
	CFRelease(newEntity);
	CFRelease(protocol);

    done :

	CFRelease(path);
	return ok;
}


CFArrayRef /* of SCNetworkServiceRef's */
SCNetworkServiceCopyAll(SCPreferencesRef prefs)
{
	CFMutableArrayRef	array;
	CFIndex			n;
	CFStringRef		path;
	CFDictionaryRef		services;

	path = SCPreferencesPathKeyCreateNetworkServices(NULL);
	services = SCPreferencesPathGetValue(prefs, path);
	CFRelease(path);

	if ((services != NULL) && !isA_CFDictionary(services)) {
		return NULL;
	}

	array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	n = (services != NULL) ? CFDictionaryGetCount(services) : 0;
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
		CFDictionaryGetKeysAndValues(services, keys, vals);
		for (i = 0; i < n; i++) {
			CFDictionaryRef			entity;
			SCNetworkServicePrivateRef	servicePrivate;

			if (!isA_CFDictionary(vals[i])) {
				SCLog(TRUE,
				      LOG_INFO,
				      CFSTR("SCNetworkServiceCopyAll(): error w/service \"%@\"\n"),
				      keys[i]);
				continue;
			}

			entity = CFDictionaryGetValue(vals[i], kSCEntNetInterface);
			if (!isA_CFDictionary(entity)) {
				// if no "interface"
				SCLog(TRUE,
				      LOG_INFO,
				      CFSTR("SCNetworkServiceCopyAll(): no \"%@\" entity for service \"%@\"\n"),
				      kSCEntNetInterface,
				      keys[i]);
				continue;
			}

			servicePrivate = __SCNetworkServiceCreatePrivate(NULL, prefs, keys[i], NULL);
			assert(servicePrivate != NULL);
			CFArrayAppendValue(array, (SCNetworkServiceRef)servicePrivate);
			CFRelease(servicePrivate);
		}
		if (keys != keys_q) {
			CFAllocatorDeallocate(NULL, keys);
			CFAllocatorDeallocate(NULL, vals);
		}
	}

	return array;
}


__private_extern__
CFArrayRef /* of SCNetworkInterfaceRef's */
__SCNetworkServiceCopyAllInterfaces(SCPreferencesRef prefs)
{
	CFMutableArrayRef interfaces = NULL;
	CFArrayRef services = NULL;
	CFIndex servicesCount = 0;
	SCNetworkServiceRef service = NULL;
	SCNetworkInterfaceRef interface = NULL;

	services = SCNetworkServiceCopyAll(prefs);
	if (services == NULL) {
		goto done;
	}

	servicesCount = CFArrayGetCount(services);
	if (servicesCount == 0) {
		goto done;
	}

	interfaces = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	for (CFIndex idx = 0; idx < servicesCount; idx++) {
		service = CFArrayGetValueAtIndex(services, idx);
		interface = SCNetworkServiceGetInterface(service);

		if (isA_SCNetworkInterface(interface) == NULL) {
			continue;
		}
		CFArrayAppendValue(interfaces, interface);
	}

	if (CFArrayGetCount(interfaces) == 0) {
		// Do not return an empty array
		CFRelease(interfaces);
		interfaces = NULL;
	}

    done:

	if (services != NULL) {
		CFRelease(services);
	}
	return  interfaces;
}


/*
 * build a list of all of a services entity types that are associated
 * with the services interface.  The list will include :
 *
 * - entity types associated with the interface type (Ethernet, FireWire, PPP, ...)
 * - entity types associated with the interface sub-type (PPPSerial, PPPoE, L2TP, PPTP, ...)
 * - entity types associated with the hardware device (Ethernet, AirPort, FireWire, Modem, ...)
 */
static CFSetRef
_copyInterfaceEntityTypes(CFDictionaryRef protocols)
{
	CFDictionaryRef interface;
	CFMutableSetRef interface_entity_types;

	interface_entity_types = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);

	interface = CFDictionaryGetValue(protocols, kSCEntNetInterface);
	if (isA_CFDictionary(interface)) {
		CFStringRef	entities[]	= { kSCPropNetInterfaceType,
						    kSCPropNetInterfaceSubType,
						    kSCPropNetInterfaceHardware };
		int		i;

		// include the "Interface" entity itself
		CFSetAddValue(interface_entity_types, kSCEntNetInterface);

		// include the entities associated with the interface
		for (i = 0; i < sizeof(entities)/sizeof(entities[0]); i++) {
			CFStringRef     entity;

			entity = CFDictionaryGetValue(interface, entities[i]);
			if (isA_CFString(entity)) {
				CFSetAddValue(interface_entity_types, entity);
			}
		}

		/*
		 * and, because we've found some misguided network preference code
		 * developers leaving [PPP] entity dictionaries around even though
		 * they are unused and/or unneeded...
		 */
		CFSetAddValue(interface_entity_types, kSCEntNetPPP);
	}

	return interface_entity_types;
}


SCNetworkServiceRef
SCNetworkServiceCopy(SCPreferencesRef prefs, CFStringRef serviceID)
{
	CFDictionaryRef			entity;
	CFStringRef			path;
	SCNetworkServicePrivateRef      servicePrivate;

	if (!isA_CFString(serviceID)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,			// allocator
							      serviceID,		// service
							      kSCEntNetInterface);      // entity
	entity = SCPreferencesPathGetValue(prefs, path);
	CFRelease(path);

	if (!isA_CFDictionary(entity)) {
		// a "service" must have an "interface"
		_SCErrorSet(kSCStatusNoKey);
		return NULL;
	}

	servicePrivate = __SCNetworkServiceCreatePrivate(NULL, prefs, serviceID, NULL);
	return (SCNetworkServiceRef)servicePrivate;
}


SCNetworkServiceRef
_SCNetworkServiceCopyActive(SCDynamicStoreRef store, CFStringRef serviceID)
{
	SCNetworkServicePrivateRef      servicePrivate;

	if (!isA_CFString(serviceID)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	servicePrivate = __SCNetworkServiceCreatePrivate(NULL, NULL, serviceID, NULL);
	assert(servicePrivate != NULL);
	if (store != NULL) {
		servicePrivate->store = CFRetain(store);
	}
	return (SCNetworkServiceRef)servicePrivate;
}


SCNetworkProtocolRef
SCNetworkServiceCopyProtocol(SCNetworkServiceRef service, CFStringRef protocolType)
{
	CFSetRef			non_protocol_entities;
	CFStringRef			path;
	CFDictionaryRef			protocols;
	SCNetworkProtocolPrivateRef	protocolPrivate = NULL;
	SCNetworkServicePrivateRef	servicePrivate  = (SCNetworkServicePrivateRef)service;

	if (!isA_SCNetworkService(service) || (servicePrivate->prefs == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	if (!isA_CFString(protocolType)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
							      servicePrivate->serviceID,	// service
							      NULL);				// entity
	protocols = SCPreferencesPathGetValue(servicePrivate->prefs, path);
	CFRelease(path);

	if (!isA_CFDictionary(protocols)) {
		// if corrupt prefs
		_SCErrorSet(kSCStatusFailed);
		return NULL;
	}

	non_protocol_entities = _copyInterfaceEntityTypes(protocols);
	if (CFSetContainsValue(non_protocol_entities, protocolType)) {
		// if the "protocolType" matches an interface entity type
		_SCErrorSet(kSCStatusInvalidArgument);
		goto done;
	}

	if (!CFDictionaryContainsKey(protocols, protocolType)) {
		// if the "protocolType" entity does not exist
		_SCErrorSet(kSCStatusNoKey);
		goto done;
	}

	protocolPrivate = __SCNetworkProtocolCreatePrivate(NULL, protocolType, service);

    done :

	CFRelease(non_protocol_entities);

	return (SCNetworkProtocolRef)protocolPrivate;
}


CFArrayRef /* of SCNetworkProtocolRef's */
SCNetworkServiceCopyProtocols(SCNetworkServiceRef service)
{
	CFMutableArrayRef		array;
	CFIndex				n;
	CFSetRef			non_protocol_entities;
	CFStringRef			path;
	CFDictionaryRef			protocols;
	SCNetworkServicePrivateRef	servicePrivate  = (SCNetworkServicePrivateRef)service;

	if (!isA_SCNetworkService(service) || (servicePrivate->prefs == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
							      servicePrivate->serviceID,	// service
							      NULL);				// entity
	protocols = SCPreferencesPathGetValue(servicePrivate->prefs, path);
	CFRelease(path);

	if (!isA_CFDictionary(protocols)) {
		// if corrupt prefs
		_SCErrorSet(kSCStatusFailed);
		return NULL;
	}

	non_protocol_entities = _copyInterfaceEntityTypes(protocols);

	array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	n = CFDictionaryGetCount(protocols);
	if (n > 0) {
		CFIndex				i;
		const void *			keys_q[N_QUICK];
		const void **			keys		= keys_q;
		const void *			vals_q[N_QUICK];
		const void **			vals		= vals_q;

		if (n > (CFIndex)(sizeof(keys_q) / sizeof(CFTypeRef))) {
			keys = CFAllocatorAllocate(NULL, n * sizeof(CFTypeRef), 0);
			vals = CFAllocatorAllocate(NULL, n * sizeof(CFPropertyListRef), 0);
		}
		CFDictionaryGetKeysAndValues(protocols, keys, vals);
		for (i = 0; i < n; i++) {
			SCNetworkProtocolPrivateRef	protocolPrivate;

			if (!isA_CFDictionary(vals[i])) {
				// if it's not a dictionary then it can't be a protocol entity
				continue;
			}

			if (CFSetContainsValue(non_protocol_entities, keys[i])) {
				// skip any non-protocol (interface) entities
				continue;
			}

			protocolPrivate = __SCNetworkProtocolCreatePrivate(NULL, keys[i], service);
			CFArrayAppendValue(array, (SCNetworkProtocolRef)protocolPrivate);

			CFRelease(protocolPrivate);
		}
		if (keys != keys_q) {
			CFAllocatorDeallocate(NULL, keys);
			CFAllocatorDeallocate(NULL, vals);
		}
	}

	CFRelease(non_protocol_entities);

	return array;
}


static Boolean
__SCNetworkServiceSetInterfaceEntity(SCNetworkServiceRef     service,
				     SCNetworkInterfaceRef   interface)
{
	CFDictionaryRef			entity;
	Boolean				ok;
	CFStringRef			path;
	SCNetworkServicePrivateRef      servicePrivate		= (SCNetworkServicePrivateRef)service;

	path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
							      servicePrivate->serviceID,	// service
							      kSCEntNetInterface);		// entity
	entity = __SCNetworkInterfaceCopyInterfaceEntity(interface);
	ok = SCPreferencesPathSetValue(servicePrivate->prefs, path, entity);
	CFRelease(entity);
	CFRelease(path);

	return ok;
}


SCNetworkServiceRef
SCNetworkServiceCreate(SCPreferencesRef prefs, SCNetworkInterfaceRef interface)
{
	CFArrayRef			components;
	CFArrayRef			interface_config;
	CFStringRef			interface_name;
	SCNetworkInterfaceRef		newInterface;
	CFStringRef			path;
	CFStringRef			prefix;
	CFStringRef			serviceID;
	SCNetworkServicePrivateRef	servicePrivate;
	CFArrayRef			supported_protocols;

	if (!isA_SCNetworkInterface(interface)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	// only allow network interfaces which support one or more protocols
	// to be added to a service.  The one exception is that we allow
	// third-party interface types to be configured.
	supported_protocols = SCNetworkInterfaceGetSupportedProtocolTypes(interface);
	if (supported_protocols == NULL) {
		CFStringRef	interface_type;

		interface_type = SCNetworkInterfaceGetInterfaceType(interface);
		if (CFStringFind(interface_type, CFSTR("."), 0).location == kCFNotFound) {
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
		}
	}

	// do not allow creation of a network service if the interface is a
	// member of a bond or bridge
	if (__SCNetworkInterfaceIsMember(prefs, interface)) {
		_SCErrorSet(kSCStatusKeyExists);
		return NULL;
	}

	// establish the service
	prefix = SCPreferencesPathKeyCreateNetworkServices(NULL);
	path = __SCPreferencesPathCreateUniqueChild_WithMoreSCFCompatibility(prefs, prefix);
	if (path == NULL) path = SCPreferencesPathCreateUniqueChild(prefs, prefix);
	CFRelease(prefix);
	if (path == NULL) {
		return NULL;
	}

	components = CFStringCreateArrayBySeparatingStrings(NULL, path, CFSTR("/"));
	CFRelease(path);

	serviceID = CFArrayGetValueAtIndex(components, 2);
	servicePrivate = __SCNetworkServiceCreatePrivate(NULL, prefs, serviceID, NULL);
	CFRelease(components);

	// duplicate the interface and associate the copy with the new service
	newInterface = (SCNetworkInterfaceRef)__SCNetworkInterfaceCreateCopy(NULL,
									     interface,
									     prefs,
									     serviceID);
	servicePrivate->interface = newInterface;

	// establish "default" configuration(s) for the interface
	for (interface = newInterface;
	     interface != NULL;
	     interface = SCNetworkInterfaceGetInterface(interface)) {
		SCNetworkInterfaceRef   childInterface;
		CFStringRef		childInterfaceType      = NULL;
		CFDictionaryRef		config;
		CFStringRef		interfaceType;

		interfaceType = SCNetworkInterfaceGetInterfaceType(interface);
		childInterface = SCNetworkInterfaceGetInterface(interface);
		if (childInterface != NULL) {
			childInterfaceType = SCNetworkInterfaceGetInterfaceType(childInterface);
		}

		config = __copyInterfaceTemplate(interfaceType, childInterfaceType);
		if (config != NULL) {
			if (CFEqual(interfaceType, kSCNetworkInterfaceTypeBluetooth) ||
			    CFEqual(interfaceType, kSCNetworkInterfaceTypeIrDA     ) ||
			    CFEqual(interfaceType, kSCNetworkInterfaceTypeModem    ) ||
			    CFEqual(interfaceType, kSCNetworkInterfaceTypeSerial   ) ||
			    CFEqual(interfaceType, kSCNetworkInterfaceTypeWWAN     )) {
				CFDictionaryRef		overrides;

				overrides = __SCNetworkInterfaceGetTemplateOverrides(interface, kSCNetworkInterfaceTypeModem);

				// a ConnectionScript (and related keys) from the interface
				// should trump the settings from the configuration template.
				if (overrides != NULL) {
					CFMutableDictionaryRef	newConfig;

					newConfig = CFDictionaryCreateMutableCopy(NULL, 0, config);
					if (CFDictionaryContainsKey(overrides, kSCPropNetModemConnectionScript)) {
						CFDictionaryRemoveValue(newConfig, kSCPropNetModemConnectionPersonality);
						CFDictionaryRemoveValue(newConfig, kSCPropNetModemConnectionScript);
						CFDictionaryRemoveValue(newConfig, kSCPropNetModemDeviceVendor);
						CFDictionaryRemoveValue(newConfig, kSCPropNetModemDeviceModel);
					}
					CFDictionaryApplyFunction(overrides, mergeDict, newConfig);
					CFRelease(config);
					config = newConfig;
				}
			} else if (CFEqual(interfaceType, kSCNetworkInterfaceTypePPP) ||
				   CFEqual(interfaceType, kSCNetworkInterfaceTypeVPN)) {
				CFDictionaryRef		overrides;

				overrides = __SCNetworkInterfaceGetTemplateOverrides(interface, kSCNetworkInterfaceTypePPP);
				if (overrides != NULL) {
					CFMutableDictionaryRef	newConfig;

					newConfig = CFDictionaryCreateMutableCopy(NULL, 0, config);
					CFDictionaryApplyFunction(overrides, mergeDict, newConfig);
					CFRelease(config);
					config = newConfig;
				}
			}

			if (!__SCNetworkInterfaceSetConfiguration(interface, NULL, config, TRUE)) {
				SCLog(TRUE, LOG_DEBUG,
				      CFSTR("SCNetworkService __SCNetworkInterfaceSetConfiguration failed(), interface=%@, type=NULL"),
				      interface);
			}
			CFRelease(config);
		}
	}

	// add the interface [entity] to the service
	(void) __SCNetworkServiceSetInterfaceEntity((SCNetworkServiceRef)servicePrivate,
						    servicePrivate->interface);

	// push the [deep] interface configuration into the service.
	interface_config = __SCNetworkInterfaceCopyDeepConfiguration(NULL, servicePrivate->interface);
	__SCNetworkInterfaceSetDeepConfiguration(NULL, servicePrivate->interface, interface_config);
	if (interface_config != NULL) CFRelease(interface_config);

	// set the service name to match that of the associated interface
	//
	// Note: It might seem a bit odd to call SCNetworkServiceGetName
	// followed by an immediate call to SCNetworkServiceSetName.  The
	// trick here is that if no name has previously been set, the
	// "get" function will return the name of the associated interface.
	//
	// ... and we "set" a name to ensure that applications that do
	// not use the APIs will still find a UserDefinedName property
	// in the SCDynamicStore.
	//
	interface_name = SCNetworkServiceGetName((SCNetworkServiceRef)servicePrivate);
	if (interface_name != NULL) {
		(void) SCNetworkServiceSetName((SCNetworkServiceRef)servicePrivate,
					       interface_name);
	}

	return (SCNetworkServiceRef)servicePrivate;
}


Boolean
SCNetworkServiceEstablishDefaultConfiguration(SCNetworkServiceRef service)
{
	CFIndex				i;
	SCNetworkInterfaceRef		interface;
	CFIndex				n;
	CFArrayRef			protocolTypes;
	SCNetworkServicePrivateRef	servicePrivate  = (SCNetworkServicePrivateRef)service;

	if (!isA_SCNetworkService(service) || (servicePrivate->prefs == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	interface = SCNetworkServiceGetInterface(service);
	if (interface == NULL) {
		return FALSE;
	}

	protocolTypes = SCNetworkInterfaceGetSupportedProtocolTypes(interface);
	n = (protocolTypes != NULL) ? CFArrayGetCount(protocolTypes) : 0;
	for (i = 0; i < n; i++) {
		Boolean			enabled;
		CFDictionaryRef		newEntity	= NULL;
		Boolean			ok;
		SCNetworkProtocolRef	protocol	= NULL;
		CFStringRef		protocolType;

		protocolType = CFArrayGetValueAtIndex(protocolTypes, i);
		ok = SCNetworkServiceAddProtocolType(service, protocolType);
		if (!ok && (SCError() != kSCStatusKeyExists)) {
			// could not add protocol
			goto nextProtocol;
		}

		protocol = SCNetworkServiceCopyProtocol(service, protocolType);
		if (protocol == NULL) {
			// oops, somethings wrong (should never happen)
			goto nextProtocol;
		}

		newEntity = _protocolTemplate(service, protocolType);
		ok = SCNetworkProtocolSetConfiguration(protocol, newEntity);
		if (!ok) {
			// could not set default configuration
			goto nextProtocol;
		}

		enabled = !CFDictionaryContainsKey(newEntity, kSCResvInactive);
		ok = SCNetworkProtocolSetEnabled(protocol, enabled);
		if (!ok) {
			// could not enable/disable protocol
			goto nextProtocol;
		}

	    nextProtocol :

		if (newEntity != NULL) CFRelease(newEntity);
		if (protocol  != NULL) CFRelease(protocol);
	}

	return TRUE;
}


Boolean
SCNetworkServiceGetEnabled(SCNetworkServiceRef service)
{
	Boolean				enabled;
	CFStringRef			path;
	SCNetworkServicePrivateRef      servicePrivate  = (SCNetworkServicePrivateRef)service;

	if (!isA_SCNetworkService(service) || (servicePrivate->prefs == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
							      servicePrivate->serviceID,	// service
							      NULL);				// entity
	enabled = __getPrefsEnabled(servicePrivate->prefs, path);
	CFRelease(path);

	return enabled;
}


SCNetworkInterfaceRef
SCNetworkServiceGetInterface(SCNetworkServiceRef service)
{
	SCNetworkServicePrivateRef      servicePrivate  = (SCNetworkServicePrivateRef)service;

	if (!isA_SCNetworkService(service) || (servicePrivate->prefs == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	if (servicePrivate->interface == NULL) {
		CFDictionaryRef entity;
		CFStringRef     path;

		path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
								      servicePrivate->serviceID,	// service
								      kSCEntNetInterface);		// entity
		entity = SCPreferencesPathGetValue(servicePrivate->prefs, path);
		CFRelease(path);

		if (isA_CFDictionary(entity)) {
			servicePrivate->interface = _SCNetworkInterfaceCreateWithEntity(NULL, entity, service);
		}
	}

	return servicePrivate->interface;
}


CFStringRef
SCNetworkServiceGetName(SCNetworkServiceRef service)
{
	CFDictionaryRef			entity;
	SCNetworkInterfaceRef		interface;
	CFStringRef			name		= NULL;
	CFStringRef			path;
	SCNetworkServicePrivateRef	servicePrivate	= (SCNetworkServicePrivateRef)service;
	Boolean				useSystemInterfaces = TRUE;

	if (!isA_SCNetworkService(service) || (servicePrivate->prefs == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	if (servicePrivate->name != NULL) {
		return servicePrivate->name;
	}

	path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
							      servicePrivate->serviceID,	// service
							      NULL);				// entity
	entity = SCPreferencesPathGetValue(servicePrivate->prefs, path);
	CFRelease(path);

	useSystemInterfaces = ((__SCPreferencesUsingDefaultPrefs(servicePrivate->prefs)) &&
			       (__SCPreferencesGetLimitSCNetworkConfiguration(servicePrivate->prefs) == FALSE));
	
	if (isA_CFDictionary(entity)) {
		name = CFDictionaryGetValue(entity, kSCPropUserDefinedName);
		if (isA_CFString(name)) {
			servicePrivate->name = CFRetain(name);
			if (useSystemInterfaces == FALSE) {
				return servicePrivate->name;
			}
		}
	}

	interface = SCNetworkServiceGetInterface(service);
	while (interface != NULL) {
		SCNetworkInterfaceRef   childInterface;
		CFStringRef		interfaceType;

		interfaceType = SCNetworkInterfaceGetInterfaceType(interface);
		if (CFEqual(interfaceType, kSCNetworkInterfaceTypeVPN)) {
			break;
		}

		childInterface = SCNetworkInterfaceGetInterface(interface);
		if ((childInterface == NULL) ||
		    CFEqual(childInterface, kSCNetworkInterfaceIPv4)) {
			break;
		}

		interface = childInterface;
	}

	if (interface != NULL) {
		int		i;
		CFStringRef	interface_name	= NULL;
		CFStringRef	suffix		= NULL;

		//
		// check if the [stored] service name matches the non-localized interface
		// name.  If so, return the localized name.
		//
		// Also, the older "Built-in XXX" interface names are too long for the
		// current UI. If we find that the [stored] service name matches the older
		// name, return the newer (and shorter) localized name.
		//
		// Note: the user/admin will no longer be able to set the service name
		//       to "Built-in Ethernet".
		//
		for (i = 0; i < 3; i++) {
			if (servicePrivate->name == NULL) {
				// if no [stored] service name to compare
				break;
			}

			switch (i) {
				case 0 :
					// compare the non-localized interface name
					interface_name = __SCNetworkInterfaceGetNonLocalizedDisplayName(interface);
					if (interface_name != NULL) {
						CFRetain(interface_name);
					}
					break;
#if	!TARGET_OS_IPHONE
				case 1 :
					// compare the older "Built-in XXX" localized name
					interface_name = __SCNetworkInterfaceCopyXLocalizedDisplayName(interface);
					break;
				case 2 :
					// compare the older "Built-in XXX" non-localized name
					interface_name = __SCNetworkInterfaceCopyXNonLocalizedDisplayName(interface);
					break;
#endif	// !TARGET_OS_IPHONE
				default :
					continue;
			}

			if (interface_name != NULL) {
				Boolean	match	= FALSE;

				if (CFEqual(name, interface_name)) {
					// if service name matches the OLD localized
					// interface name
					match = TRUE;
				} else if (CFStringHasPrefix(name, interface_name)) {
					CFIndex	prefixLen	= CFStringGetLength(interface_name);
					CFIndex	suffixLen	= CFStringGetLength(name);

					suffix = CFStringCreateWithSubstring(NULL,
									     name,
									     CFRangeMake(prefixLen, suffixLen - prefixLen));
					match = TRUE;
				}
				CFRelease(interface_name);

				if (match) {
					CFRelease(servicePrivate->name);
					servicePrivate->name = NULL;
					break;
				}
			}
		}

		//
		// if the service name has not been set, use the localized interface name
		//
		if (servicePrivate->name == NULL) {
			interface_name = SCNetworkInterfaceGetLocalizedDisplayName(interface);
			if (interface_name != NULL) {
				if (suffix != NULL) {
					servicePrivate->name = CFStringCreateWithFormat(NULL,
											NULL,
											CFSTR("%@%@"),
											interface_name,
											suffix);
				} else {
					servicePrivate->name = CFRetain(interface_name);
				}
			}
		}
		if (suffix != NULL) CFRelease(suffix);
	}

	return servicePrivate->name;
}


CFStringRef
SCNetworkServiceGetServiceID(SCNetworkServiceRef service)
{
	SCNetworkServicePrivateRef	servicePrivate	= (SCNetworkServicePrivateRef)service;

	if (!isA_SCNetworkService(service)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	return servicePrivate->serviceID;
}


CFTypeID
SCNetworkServiceGetTypeID(void)
{
	pthread_once(&initialized, __SCNetworkServiceInitialize);	/* initialize runtime */
	return __kSCNetworkServiceTypeID;
}


Boolean
SCNetworkServiceRemove(SCNetworkServiceRef service)
{
	Boolean				ok		= FALSE;
	CFStringRef			path;
	SCNetworkServicePrivateRef	servicePrivate	= (SCNetworkServicePrivateRef)service;
	CFArrayRef			sets;

	if (!isA_SCNetworkService(service) || (servicePrivate->prefs == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	// remove service from all sets

	sets = SCNetworkSetCopyAll(servicePrivate->prefs);
	if (sets != NULL) {
		CFIndex	i;
		CFIndex n;

		n = CFArrayGetCount(sets);
		for (i = 0; i < n; i++) {
			SCNetworkSetRef	set;

			set = CFArrayGetValueAtIndex(sets, i);
			ok = SCNetworkSetRemoveService(set, service);
			if (!ok && (SCError() != kSCStatusNoKey)) {
				CFRelease(sets);
				return ok;
			}
		}
		CFRelease(sets);
	}

	// remove service

	path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
							      servicePrivate->serviceID,	// service
							      NULL);				// entity
	ok = SCPreferencesPathRemoveValue(servicePrivate->prefs, path);
	CFRelease(path);

	return ok;
}


Boolean
SCNetworkServiceRemoveProtocolType(SCNetworkServiceRef service, CFStringRef protocolType)
{
	CFDictionaryRef			entity;
	Boolean				ok		= FALSE;
	CFStringRef			path;
	SCNetworkServicePrivateRef      servicePrivate  = (SCNetworkServicePrivateRef)service;

	if (!isA_SCNetworkService(service) || (servicePrivate->prefs == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (!__SCNetworkProtocolIsValidType(protocolType)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
							      servicePrivate->serviceID,	// service
							      protocolType);			// entity

	entity = SCPreferencesPathGetValue(servicePrivate->prefs, path);
	if (entity == NULL) {
		// if "protocol" does not exist
		_SCErrorSet(kSCStatusNoKey);
		goto done;
	}

	ok = SCPreferencesPathRemoveValue(servicePrivate->prefs, path);

    done :

	CFRelease(path);
	return ok;
}


Boolean
SCNetworkServiceSetEnabled(SCNetworkServiceRef service, Boolean enabled)
{
	Boolean				ok;
	CFStringRef			path;
	SCNetworkServicePrivateRef      servicePrivate  = (SCNetworkServicePrivateRef)service;

	if (!isA_SCNetworkService(service) || (servicePrivate->prefs == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	// make sure that we do not enable a network service if the
	// associated interface is a member of a bond or bridge.
	if (enabled) {
		SCNetworkInterfaceRef	interface;

		interface = SCNetworkServiceGetInterface(service);
		if ((interface != NULL) &&
		    __SCNetworkInterfaceIsMember(servicePrivate->prefs, interface)) {
			_SCErrorSet(kSCStatusKeyExists);
			return FALSE;
		}
	}

	path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
							      servicePrivate->serviceID,	// service
							      NULL);				// entity
	ok = __setPrefsEnabled(servicePrivate->prefs, path, enabled);
	CFRelease(path);

	return ok;
}


Boolean
SCNetworkServiceSetName(SCNetworkServiceRef service, CFStringRef name)
{
	CFDictionaryRef			entity;
	Boolean				ok		= FALSE;
	CFStringRef			path;
	CFStringRef			saveName	= NULL;
	SCNetworkServicePrivateRef	servicePrivate	= (SCNetworkServicePrivateRef)service;

	if (!isA_SCNetworkService(service) || (servicePrivate->prefs == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (name != NULL) {
		if (!isA_CFString(name)) {
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
		}
		saveName = CFRetain(name);
	}

	if (name != NULL) {
		SCNetworkInterfaceRef	interface;

		interface = SCNetworkServiceGetInterface(service);
		while (interface != NULL) {
			SCNetworkInterfaceRef	childInterface;

			childInterface = SCNetworkInterfaceGetInterface(interface);
			if (childInterface == NULL) {
				break;
			}

			interface = childInterface;
		}

		if (interface != NULL) {
			CFStringRef	interface_name;

			interface_name = SCNetworkInterfaceGetLocalizedDisplayName(interface);
			if (interface_name != NULL) {
				if (CFEqual(name, interface_name)) {
					// if service name matches the localized interface name
					// then store the non-localized name.
					interface_name = __SCNetworkInterfaceGetNonLocalizedDisplayName(interface);
					if (interface_name != NULL) {
						CFRelease(saveName);
						saveName = CFRetain(interface_name);
					}
				} else if (CFStringHasPrefix(name, interface_name)) {
					CFIndex		prefixLen	= CFStringGetLength(interface_name);
					CFStringRef	suffix;
					CFIndex		suffixLen	= CFStringGetLength(name);

					// if service name matches the localized interface name plus
					// a few extra characters) then store the non-localized name with
					// the same suffix.
					suffix = CFStringCreateWithSubstring(NULL,
									     name,
									     CFRangeMake(prefixLen, suffixLen - prefixLen));
					interface_name = __SCNetworkInterfaceGetNonLocalizedDisplayName(interface);
					if (interface_name != NULL) {
						CFRelease(saveName);
						saveName = CFStringCreateWithFormat(NULL,
										    NULL,
										    CFSTR("%@%@"),
										    interface_name,
										    suffix);
					}
					CFRelease(suffix);
				}
			}
		}
	}

#define PREVENT_DUPLICATE_SERVICE_NAMES
#ifdef  PREVENT_DUPLICATE_SERVICE_NAMES
	if (name != NULL) {
		CFArrayRef      sets;

		// ensure that each service is uniquely named within its sets

		sets = SCNetworkSetCopyAll(servicePrivate->prefs);
		if (sets != NULL) {
			CFIndex		set_index;
			CFIndex		set_count;

			set_count = CFArrayGetCount(sets);
			for (set_index = 0; set_index < set_count; set_index++) {
				CFIndex		service_index;
				Boolean		isDup		= FALSE;
				Boolean		isMember	= FALSE;
				CFIndex		service_count;
				CFArrayRef      services;
				SCNetworkSetRef set		= CFArrayGetValueAtIndex(sets, set_index);

				services = SCNetworkSetCopyServices(set);

				service_count = CFArrayGetCount(services);
				for (service_index = 0; service_index < service_count; service_index++) {
					CFStringRef		otherID;
					CFStringRef		otherName;
					SCNetworkServiceRef     otherService;

					otherService = CFArrayGetValueAtIndex(services, service_index);

					otherID = SCNetworkServiceGetServiceID(otherService);
					if (CFEqual(servicePrivate->serviceID, otherID)) {
						// if the service is a member of this set
						isMember = TRUE;
						continue;
					}

					otherName = SCNetworkServiceGetName(otherService);
					if ((otherName != NULL) && CFEqual(name, otherName)) {
						isDup = TRUE;
						continue;
					}
				}

				CFRelease(services);

				if (isMember && isDup) {
					/*
					 * if this service is a member of the set and
					 * the "name" is not unique.
					 */
					CFRelease(sets);
					if (saveName != NULL) CFRelease(saveName);
					_SCErrorSet(kSCStatusKeyExists);
					return FALSE;
				}
			}

			CFRelease(sets);
		}
	}
#endif  /* PREVENT_DUPLICATE_SERVICE_NAMES */

	path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
							      servicePrivate->serviceID,	// service
							      NULL);				// entity
	entity = SCPreferencesPathGetValue(servicePrivate->prefs, path);
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
		if (saveName != NULL) {
			CFDictionarySetValue(newEntity, kSCPropUserDefinedName, saveName);
		} else {
			CFDictionaryRemoveValue(newEntity, kSCPropUserDefinedName);
		}
		ok = SCPreferencesPathSetValue(servicePrivate->prefs, path, newEntity);
		CFRelease(newEntity);
	}
	CFRelease(path);
	if (saveName != NULL) CFRelease(saveName);

	if (servicePrivate->name != NULL) CFRelease(servicePrivate->name);
	if (name != NULL) CFRetain(name);
	servicePrivate->name = name;

	return ok;
}


#pragma mark -
#pragma mark SCNetworkService SPIs


SCNetworkServicePrimaryRank
SCNetworkServiceGetPrimaryRank(SCNetworkServiceRef service)
{
	CFDictionaryRef			entity;
	Boolean				ok		= TRUE;
	CFStringRef			path;
	SCNetworkServicePrimaryRank	rank		= kSCNetworkServicePrimaryRankDefault;
	CFStringRef			rankStr		= NULL;
	SCNetworkServicePrivateRef      servicePrivate	= (SCNetworkServicePrivateRef)service;

	if (!isA_SCNetworkService(service)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return rank;
	}

	if (servicePrivate->prefs != NULL) {
		path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,
								      servicePrivate->serviceID,
								      NULL);
		entity = SCPreferencesPathGetValue(servicePrivate->prefs, path);
		CFRelease(path);
		if (isA_CFDictionary(entity)) {
			rankStr = CFDictionaryGetValue(entity, kSCPropNetServicePrimaryRank);
			ok = __str_to_rank(rankStr, &rank);
		}
	} else if (servicePrivate->store != NULL) {
		path = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
								   kSCDynamicStoreDomainState,
								   servicePrivate->serviceID,
								   NULL);
		entity = SCDynamicStoreCopyValue(servicePrivate->store, path);
		CFRelease(path);
		if (entity != NULL) {
			if (isA_CFDictionary(entity)) {
				rankStr = CFDictionaryGetValue(entity, kSCPropNetServicePrimaryRank);
				ok = __str_to_rank(rankStr, &rank);
			}
			CFRelease(entity);
		}
	} else {
		_SCErrorSet(kSCStatusInvalidArgument);
		return rank;
	}

	if (!ok) {
		rank = kSCNetworkServicePrimaryRankDefault;
		_SCErrorSet(kSCStatusInvalidArgument);
	} else if (rank == kSCNetworkServicePrimaryRankDefault) {
		_SCErrorSet(kSCStatusOK);
	}

	return rank;
}


Boolean
SCNetworkServiceSetPrimaryRank(SCNetworkServiceRef		service,
			       SCNetworkServicePrimaryRank	newRank)
{
	Boolean				ok;
	CFDictionaryRef			entity;
	CFMutableDictionaryRef		newEntity;
	CFStringRef			path		= NULL;
	CFStringRef			rankStr		= NULL;
	SCNetworkServicePrivateRef      servicePrivate	= (SCNetworkServicePrivateRef)service;

	if (!isA_SCNetworkService(service)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	ok = __rank_to_str(newRank, &rankStr);
	if (!ok) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (servicePrivate->prefs != NULL) {
		switch (newRank) {
		case kSCNetworkServicePrimaryRankDefault:
		case kSCNetworkServicePrimaryRankNever:
		case kSCNetworkServicePrimaryRankScoped:
			path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,
									      servicePrivate->serviceID,
									      NULL);
			entity = SCPreferencesPathGetValue(servicePrivate->prefs, path);
			if (entity != NULL) {
				if (!isA_CFDictionary(entity)) {
					// if corrupt prefs
					_SCErrorSet(kSCStatusFailed);
					goto done;
				}
				newEntity = CFDictionaryCreateMutableCopy(NULL, 0, entity);
			} else {
				newEntity = CFDictionaryCreateMutable(NULL,
								      0,
								      &kCFTypeDictionaryKeyCallBacks,
								      &kCFTypeDictionaryValueCallBacks);
			}
			if (rankStr != NULL) {
				CFDictionarySetValue(newEntity, kSCPropNetServicePrimaryRank, rankStr);
			} else {
				CFDictionaryRemoveValue(newEntity, kSCPropNetServicePrimaryRank);
			}
			if (CFDictionaryGetCount(newEntity) > 0) {
				ok = SCPreferencesPathSetValue(servicePrivate->prefs, path, newEntity);
			} else {
				ok = SCPreferencesPathRemoveValue(servicePrivate->prefs, path);
			}
			CFRelease(newEntity);
			if (!ok) {
				goto done;
			}
			break;
		default:
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
		}
	} else if (servicePrivate->store != NULL) {
		path = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
								   kSCDynamicStoreDomainState,
								   servicePrivate->serviceID,
								   NULL);
		entity = SCDynamicStoreCopyValue(servicePrivate->store, path);
		if (entity != NULL) {
			if (!isA_CFDictionary(entity)) {
				// if corrupt prefs
				CFRelease(entity);
				_SCErrorSet(kSCStatusFailed);
				goto done;
			}
			newEntity = CFDictionaryCreateMutableCopy(NULL, 0, entity);
			CFRelease(entity);
		} else {
			newEntity = CFDictionaryCreateMutable(NULL,
							      0,
							      &kCFTypeDictionaryKeyCallBacks,
							      &kCFTypeDictionaryValueCallBacks);
		}
		if (rankStr != NULL) {
			CFDictionarySetValue(newEntity, kSCPropNetServicePrimaryRank, rankStr);
		} else {
			CFDictionaryRemoveValue(newEntity, kSCPropNetServicePrimaryRank);
		}
		if (CFDictionaryGetCount(newEntity) > 0) {
			ok = SCDynamicStoreSetValue(servicePrivate->store, path, newEntity);
		} else {
			ok = SCDynamicStoreRemoveValue(servicePrivate->store, path);
		}
		CFRelease(newEntity);
		if (!ok) {
			goto done;
		}
	} else {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

    done :

	if (path != NULL)	CFRelease(path);
	return ok;
}


Boolean
_SCNetworkServiceIsVPN(SCNetworkServiceRef service)
{
	SCNetworkInterfaceRef	interface;
	CFStringRef		interfaceType;

	interface = SCNetworkServiceGetInterface(service);
	if (interface == NULL) {
		return FALSE;
	}

	interfaceType = SCNetworkInterfaceGetInterfaceType(interface);
	if (CFEqual(interfaceType, kSCNetworkInterfaceTypePPP)) {
		interface = SCNetworkInterfaceGetInterface(interface);
		if (interface == NULL) {
			return FALSE;
		}

		interfaceType = SCNetworkInterfaceGetInterfaceType(interface);
		if (CFEqual(interfaceType, kSCNetworkInterfaceTypeL2TP)) {
			return TRUE;
		}
		if (CFEqual(interfaceType, kSCNetworkInterfaceTypePPTP)) {
			return TRUE;
		}
	} else if (CFEqual(interfaceType, kSCNetworkInterfaceTypeVPN)) {
		return TRUE;
	} else if (CFEqual(interfaceType, kSCNetworkInterfaceTypeIPSec)) {
		return TRUE;
	}

	return FALSE;
}


Boolean
SCNetworkServiceSetExternalID(SCNetworkServiceRef service, CFStringRef identifierDomain, CFStringRef identifier)
{
	CFStringRef					prefs_path;
	CFDictionaryRef				service_dictionary;
	SCNetworkServicePrivateRef	service_private		= (SCNetworkServicePrivateRef)service;
	Boolean						success				= FALSE;
	CFStringRef					prefixed_domain;

	if (!isA_SCNetworkService(service) || (service_private->prefs == NULL) || !isA_CFString(identifierDomain)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (identifier != NULL && !isA_CFString(identifier)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	prefixed_domain = CFStringCreateWithFormat(NULL, 0, CFSTR("%s%@"), EXTERNAL_ID_DOMAIN_PREFIX, identifierDomain);

	prefs_path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,
								    service_private->serviceID,
								    NULL);

	service_dictionary = SCPreferencesPathGetValue(service_private->prefs, prefs_path);
	if (isA_CFDictionary(service_dictionary) || ((service_dictionary == NULL) && (identifier != NULL))) {
		CFMutableDictionaryRef	new_service_dictionary;

		if (service_dictionary != NULL) {
			new_service_dictionary = CFDictionaryCreateMutableCopy(NULL, 0, service_dictionary);
		} else {
			new_service_dictionary = CFDictionaryCreateMutable(NULL,
									   0,
									   &kCFTypeDictionaryKeyCallBacks,
									   &kCFTypeDictionaryValueCallBacks);
		}

		if (identifier != NULL) {
			CFDictionarySetValue(new_service_dictionary, prefixed_domain, identifier);
		} else {
			CFDictionaryRemoveValue(new_service_dictionary, prefixed_domain);
		}
		success = SCPreferencesPathSetValue(service_private->prefs, prefs_path, new_service_dictionary);
		CFRelease(new_service_dictionary);
	}
	CFRelease(prefs_path);

	if (identifier != NULL) {
	    if (service_private->externalIDs == NULL) {
			service_private->externalIDs = CFDictionaryCreateMutable(NULL,
										 0,
																	 &kCFTypeDictionaryKeyCallBacks,
																	 &kCFTypeDictionaryValueCallBacks);
	    }
	    CFDictionarySetValue(service_private->externalIDs, prefixed_domain, identifier);
	} else {
	    if (service_private->externalIDs != NULL) {
			CFDictionaryRemoveValue(service_private->externalIDs, prefixed_domain);
	    }
	}

	CFRelease(prefixed_domain);

	if (!success) {
		_SCErrorSet(kSCStatusFailed);
	}

	return success;
}


CFStringRef
SCNetworkServiceCopyExternalID(SCNetworkServiceRef service, CFStringRef identifierDomain)
{
	SCNetworkServicePrivateRef	service_private		= (SCNetworkServicePrivateRef)service;
	CFStringRef					identifier			= NULL;
	CFStringRef					prefixed_domain;

	if (!isA_SCNetworkService(service) || (service_private->prefs == NULL) || !isA_CFString(identifierDomain)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	prefixed_domain = CFStringCreateWithFormat(NULL, 0, CFSTR("%s%@"), EXTERNAL_ID_DOMAIN_PREFIX, identifierDomain);

	if (service_private->externalIDs != NULL) {
		identifier = CFDictionaryGetValue(service_private->externalIDs, prefixed_domain);
		if (identifier != NULL) {
			CFRetain(identifier);
		}
	}

	if (identifier == NULL) {
		CFStringRef			prefs_path;
		CFDictionaryRef		service_dictionary;

		prefs_path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,
									    service_private->serviceID,
									    NULL);

		service_dictionary = SCPreferencesPathGetValue(service_private->prefs, prefs_path);
		if (isA_CFDictionary(service_dictionary)) {
			identifier = CFDictionaryGetValue(service_dictionary, prefixed_domain);
			if (identifier != NULL) {
				CFRetain(identifier);
				if (service_private->externalIDs == NULL) {
					service_private->externalIDs = CFDictionaryCreateMutable(NULL,
												 0,
												 &kCFTypeDictionaryKeyCallBacks,
												 &kCFTypeDictionaryValueCallBacks);
				}
				CFDictionarySetValue(service_private->externalIDs, prefixed_domain, identifier);
			}
		}
		CFRelease(prefs_path);
	}

	CFRelease(prefixed_domain);

	if (identifier == NULL) {
		_SCErrorSet(kSCStatusNoKey);
	}

	return identifier;
}


typedef struct {
	CFStringRef	oldServiceID;
	CFStringRef	newServiceID;
} serviceContext, *serviceContextRef;


static void
replaceServiceID(const void *value, void *context)
{
	CFStringRef		link		= NULL;
	CFStringRef		oldLink;
	CFMutableArrayRef	newServiceOrder;
	CFStringRef		path;
	serviceContextRef	service_context	= (serviceContextRef)context;
	CFArrayRef		serviceOrder	= NULL;
	SCNetworkSetRef		set		= (SCNetworkSetRef)value;
	SCNetworkSetPrivateRef	setPrivate	= (SCNetworkSetPrivateRef)set;

	// update service order
	serviceOrder = SCNetworkSetGetServiceOrder(set);
	if ((isA_CFArray(serviceOrder) != NULL) &&
	    (CFArrayContainsValue(serviceOrder,
				  CFRangeMake(0, CFArrayGetCount(serviceOrder)),
				  service_context->oldServiceID) == TRUE)) {
		CFIndex	count;
		CFIndex	serviceOrderIndex;

		// replacing all instances of old service ID with new one
		newServiceOrder = CFArrayCreateMutableCopy(NULL, 0, serviceOrder);
		count = CFArrayGetCount(newServiceOrder);
		for (serviceOrderIndex = 0; serviceOrderIndex < count; serviceOrderIndex++) {
			CFStringRef	serviceID;

			serviceID = CFArrayGetValueAtIndex(newServiceOrder, serviceOrderIndex);
			if (CFEqual(serviceID, service_context->oldServiceID)) {
				CFArraySetValueAtIndex(newServiceOrder, serviceOrderIndex, service_context->newServiceID);
			}
		}
		SCNetworkSetSetServiceOrder(set, newServiceOrder);
		CFRelease(newServiceOrder);
	}

	// check if service with old serviceID is part of the set
	path = SCPreferencesPathKeyCreateSetNetworkServiceEntity(NULL,				// allocator
								 setPrivate->setID,		// set
								 service_context->oldServiceID,	// service
								 NULL);				// entity
	oldLink = SCPreferencesPathGetLink(setPrivate->prefs, path);
	if (oldLink == NULL) {
		// don't make any changes if service with old serviceID is not found
		goto done;
	}

	// remove link between "set" and old "service"
	SCPreferencesPathRemoveValue(setPrivate->prefs, path);
	CFRelease(path);

	// create the link between "set" and the "service"
	path = SCPreferencesPathKeyCreateSetNetworkServiceEntity(NULL,				// allocator
								 setPrivate->setID,		// set
								 service_context->newServiceID,	// service
								 NULL);				// entity
	link = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
							      service_context->newServiceID,	// service
							      NULL);				// entity
	SCPreferencesPathSetLink(setPrivate->prefs, path, link);

    done:

	if (path != NULL) {
		CFRelease(path);
	}
	if (link != NULL) {
		CFRelease(link);
	}

	return;
}


Boolean
_SCNetworkServiceSetServiceID(SCNetworkServiceRef service, CFStringRef newServiceID)
{
	CFArrayRef			allSets		= NULL;
	CFDictionaryRef			entity;
	CFStringRef			newPath;
	Boolean				ok		= FALSE;
	CFStringRef			oldPath		= NULL;
	serviceContext			service_context;
	SCNetworkServicePrivateRef	servicePrivate	= (SCNetworkServicePrivateRef)service;

	if (!isA_SCNetworkService(service) || (servicePrivate->prefs == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (!isA_CFString(newServiceID)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (CFEqual(newServiceID, servicePrivate->serviceID)) {
		// no work needs to be done if new service ID is equal to current service ID
		return TRUE;
	}

	newPath = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
								 newServiceID,			// service
								 NULL);				// entity
	entity = SCPreferencesPathGetValue(servicePrivate->prefs, newPath);
	if (isA_CFDictionary(entity)) {
		// if the new service already exists
		_SCErrorSet(kSCStatusKeyExists);
		goto done;
	}

	oldPath = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
								 servicePrivate->serviceID,	// service
								 NULL);				// entity
	entity = SCPreferencesPathGetValue(servicePrivate->prefs, oldPath);
	if (!isA_CFDictionary(entity)) {
		// if the service has already been removed
		_SCErrorSet(kSCStatusNoKey);
		goto done;
	}

	ok = SCPreferencesPathSetValue(servicePrivate->prefs, newPath, entity);
	if (!ok) goto done;

	ok = SCPreferencesPathRemoveValue(servicePrivate->prefs, oldPath);
	if (!ok) goto done;

	allSets = SCNetworkSetCopyAll(servicePrivate->prefs);

	service_context.newServiceID = newServiceID;
	service_context.oldServiceID = servicePrivate->serviceID;

	// find all sets w/oldServiceID and update
	// ... and update the serviceOrder
	CFArrayApplyFunction(allSets,
			     CFRangeMake(0, CFArrayGetCount(allSets)),
			     replaceServiceID,
			     &service_context);

	if (servicePrivate->interface != NULL) {
		SCNetworkInterfaceRef		newInterface;

		// duplicate the interface and associate the copy with the new service ID
		newInterface = (SCNetworkInterfaceRef)__SCNetworkInterfaceCreateCopy(NULL,
										     servicePrivate->interface,
										     servicePrivate->prefs,
										     newServiceID);
		CFRelease(servicePrivate->interface);
		servicePrivate->interface = newInterface;
	}

	// replace serviceID with new one
	CFRetain(newServiceID);
	CFRelease(servicePrivate->serviceID);
	servicePrivate->serviceID = newServiceID;

    done:

	if (oldPath != NULL) {
		CFRelease(oldPath);
	}
	if (newPath != NULL) {
		CFRelease(newPath);
	}
	if (allSets != NULL) {
		CFRelease(allSets);
	}
	return ok;
}

#define kVPNProtocolPayloadInfo			CFSTR("com.apple.payload")
#define kSCEntNetLoginWindowEAPOL		CFSTR("EAPOL.LoginWindow")

static void
copyInterfaceConfiguration(SCNetworkServiceRef oldService, SCNetworkServiceRef newService)
{
	SCNetworkInterfaceRef	oldInterface;
	SCNetworkInterfaceRef	newInterface;

	oldInterface = SCNetworkServiceGetInterface(oldService);
	newInterface = SCNetworkServiceGetInterface(newService);

	while (oldInterface != NULL) {
		CFDictionaryRef	configuration;
		CFStringRef		interfaceType;

		if (newInterface == NULL) {
			// oops ... interface layering does not match
			return;
		}

		// copy interface configuration
		configuration = SCNetworkInterfaceGetConfiguration(oldInterface);

		if ((configuration != NULL) ||
		    (SCError() == kSCStatusOK)) {
			if (SCNetworkInterfaceSetConfiguration(newInterface, configuration) == FALSE) {
				SCLog(_sc_debug, LOG_DEBUG, CFSTR("problem setting interface configuration"));
			}

		}

		// special case: PPP/L2TP + IPSec
		interfaceType = SCNetworkInterfaceGetInterfaceType(oldInterface);
		if (CFEqual(interfaceType, kSCNetworkInterfaceTypePPP)) {
			SCNetworkInterfaceRef	childInterface;

			childInterface = SCNetworkInterfaceGetInterface(oldInterface);
			if (childInterface != NULL) {
				CFStringRef		childInterfaceType;

				childInterfaceType = SCNetworkInterfaceGetInterfaceType(childInterface);

				if (CFEqual(childInterfaceType, kSCNetworkInterfaceTypeL2TP)) {
					configuration = SCNetworkInterfaceGetExtendedConfiguration(oldInterface, kSCEntNetIPSec);
					if ((configuration != NULL) ||
					    (SCError() == kSCStatusOK)) {
						if (SCNetworkInterfaceSetExtendedConfiguration(newInterface, kSCEntNetIPSec, configuration) == FALSE) {
							SCLog(_sc_debug, LOG_DEBUG, CFSTR("problem setting child interface configuration"));
						}
					}
				}
			}
		}

		// special case: 802.1x
		configuration = SCNetworkInterfaceGetExtendedConfiguration(oldInterface, kSCEntNetEAPOL);
		if ((configuration != NULL) ||
		    (SCError() == kSCStatusOK)) {
			(void) SCNetworkInterfaceSetExtendedConfiguration(newInterface, kSCEntNetEAPOL, configuration);
		}

		// special case: Managed Client
		configuration = SCNetworkInterfaceGetExtendedConfiguration(oldInterface, kVPNProtocolPayloadInfo);
		if ((configuration != NULL) ||
		    (SCError() == kSCStatusOK)) {
			(void) SCNetworkInterfaceSetExtendedConfiguration(newInterface, kVPNProtocolPayloadInfo, configuration);
		}

		// special case: Network Pref
		configuration = SCNetworkInterfaceGetExtendedConfiguration(oldInterface, kSCValNetPPPAuthProtocolEAP);
		if ((configuration != NULL) ||
		    (SCError() == kSCStatusOK)) {
			(void) SCNetworkInterfaceSetExtendedConfiguration(newInterface, kSCValNetPPPAuthProtocolEAP, configuration);
		}

		// special case: Remote Pref
		configuration = SCNetworkInterfaceGetExtendedConfiguration(oldInterface, kSCEntNetLoginWindowEAPOL);
		if ((configuration != NULL) ||
		    (SCError() == kSCStatusOK)) {
			(void) SCNetworkInterfaceSetExtendedConfiguration(newInterface, kSCEntNetLoginWindowEAPOL, configuration);
		}

		// special case: Network Extension
		configuration = SCNetworkInterfaceGetExtendedConfiguration(oldInterface, kSCNetworkInterfaceTypeIPSec);
		if ((configuration != NULL) ||
		    (SCError() == kSCStatusOK)) {
			(void) SCNetworkInterfaceSetExtendedConfiguration(newInterface, kSCNetworkInterfaceTypeIPSec, configuration);
		}

		oldInterface = SCNetworkInterfaceGetInterface(oldInterface);
		newInterface = SCNetworkInterfaceGetInterface(newInterface);
	}

	return;
}

static void
addProtocolToService(SCNetworkServiceRef service, CFStringRef protocolType, CFDictionaryRef configuration, Boolean enabled)
{
	Boolean ok;
	SCNetworkProtocolRef protocol;

	protocol = SCNetworkServiceCopyProtocol(service, protocolType);

	if ((protocol == NULL) &&
	    (SCError() == kSCStatusNoKey)) {
		ok = SCNetworkServiceAddProtocolType(service, protocolType);
		if (ok) {
			protocol = SCNetworkServiceCopyProtocol(service, protocolType);
		}
	}
	if (protocol != NULL) {
		SCNetworkProtocolSetConfiguration(protocol, configuration);
		SCNetworkProtocolSetEnabled(protocol, enabled);
		CFRelease(protocol);
	}
	return;
}



__private_extern__
Boolean
__SCNetworkServiceMigrateNew(SCPreferencesRef		prefs,
			     SCNetworkServiceRef	service,
			     CFDictionaryRef		bsdMapping,
			     CFDictionaryRef		setMapping,
			     CFDictionaryRef		serviceSetMapping)
{
	CFStringRef deviceName = NULL;
	Boolean enabled;
	SCNetworkInterfaceRef interface = NULL;
	CFDictionaryRef interfaceEntity = NULL;
	CFMutableDictionaryRef interfaceEntityMutable = NULL;
	SCNetworkSetRef newSet = NULL;
	SCPreferencesRef ni_prefs = NULL;
	SCNetworkInterfaceRef ni_interface = NULL;
	SCNetworkInterfaceRef oldInterface = NULL;
	SCNetworkSetRef oldSet = NULL;
	SCNetworkServiceRef newService = NULL;
	CFStringRef serviceID = NULL;
	SCNetworkServicePrivateRef servicePrivate = (SCNetworkServicePrivateRef) service;
	CFMutableDictionaryRef servicesMutable = NULL;
	CFArrayRef setList = NULL;
	Boolean success = FALSE;
	CFStringRef targetDeviceName = NULL;
	CFStringRef userDefinedName = NULL;
	CFStringRef userDefinedNameInterface = NULL;
	CFArrayRef protocols = NULL;
	CFStringRef subType;

	if ((isA_SCNetworkService(service) == NULL) ||
	(isA_SCNetworkInterface(servicePrivate->interface) == NULL) ||
	(servicePrivate->prefs == NULL)) {
		goto done;
	}
	serviceID = servicePrivate->serviceID;

	newService = SCNetworkServiceCopy(prefs, serviceID);
	if (newService != NULL) {
		// Cannot add service if it already exists
		SCLog(TRUE, LOG_DEBUG, CFSTR("__SCNetworkServiceMigrateNew: Cannot add service if it already exists."));
		goto done;
	}

	oldInterface = SCNetworkServiceGetInterface(service);
	interfaceEntity = __SCNetworkInterfaceCopyInterfaceEntity(oldInterface);
	if (interfaceEntity == NULL) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("__SCNetworkServiceMigrateNew: interfaceEntity does not exist"));
		goto done;
	}
	interfaceEntityMutable = CFDictionaryCreateMutableCopy(NULL, 0, interfaceEntity);
	CFRelease(interfaceEntity);

	if (isA_CFDictionary(bsdMapping) != NULL) {
		deviceName = CFDictionaryGetValue(interfaceEntityMutable, kSCPropNetInterfaceDeviceName);
		if (isA_CFString(deviceName) != NULL) {
			targetDeviceName = CFDictionaryGetValue(bsdMapping, deviceName);
			if (targetDeviceName != NULL) {
				// update mapping
				CFDictionarySetValue(interfaceEntityMutable, kSCPropNetInterfaceDeviceName, targetDeviceName);
				ni_prefs = __SCPreferencesCreateNIPrefsFromPrefs(prefs);
				ni_interface = __SCNetworkInterfaceCreateWithNIPreferencesUsingBSDName(NULL, ni_prefs, targetDeviceName);
				if (ni_interface != NULL) {
					userDefinedNameInterface = __SCNetworkInterfaceGetUserDefinedName(ni_interface);
				}
			}
		}
		if (userDefinedNameInterface == NULL) {
			userDefinedNameInterface = CFDictionaryGetValue(interfaceEntityMutable, kSCPropUserDefinedName);
		}
	}
	subType = CFDictionaryGetValue(interfaceEntityMutable, kSCPropNetInterfaceSubType);
	interface = _SCNetworkInterfaceCreateWithEntity(NULL, interfaceEntityMutable, NULL);
	if (userDefinedNameInterface != NULL) {
		__SCNetworkInterfaceSetUserDefinedName(interface, userDefinedNameInterface);
	}
	// Supporting PPPoE subtype
	if (subType != NULL &&
	    CFEqual(subType, kSCValNetInterfaceSubTypePPPoE)) {
		SCNetworkInterfaceRef childInterface = SCNetworkInterfaceGetInterface(interface);
		if (childInterface != NULL) {
			__SCNetworkInterfaceSetUserDefinedName(childInterface, userDefinedNameInterface);
		}
	}
	newService = SCNetworkServiceCreate(prefs, interface);

	if (newService == NULL) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("__SCNetworkServiceMigrateNew: Could not create new service"));
		goto done;
	}

	enabled = SCNetworkServiceGetEnabled(service);
	SCNetworkServiceSetEnabled(newService, enabled);
	
	if (SCNetworkServiceEstablishDefaultConfiguration(newService) == FALSE) {
		SCNetworkServiceRemove(newService);
		SCLog(TRUE, LOG_DEBUG, CFSTR("__SCNetworkServiceMigrateNew: SCNetworkServiceEstablishDefaultConfiguration failed"));
		goto done;
	}
	
	// Set service ID
	_SCNetworkServiceSetServiceID(newService, serviceID);

	userDefinedName = SCNetworkServiceGetName(service);
	if (userDefinedName != NULL &&
	    SCNetworkServiceSetName(newService, userDefinedName) == FALSE) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("__SCNetworkServiceMigrateNew: Could not set service name to %@"), userDefinedName);
	}

	// Determine which sets to add service
	if (setMapping != NULL &&
	    serviceSetMapping != NULL) {
		setList = CFDictionaryGetValue(serviceSetMapping, service);
		if (setList != NULL) {
			for (CFIndex idx = 0; idx < CFArrayGetCount(setList); idx++) {
				oldSet = CFArrayGetValueAtIndex(setList, idx);
				newSet = CFDictionaryGetValue(setMapping, oldSet);
				
				if (newSet == NULL) {
					continue;
				}
				if (SCNetworkSetAddService(newSet, newService) == FALSE) {
					SCLog(_sc_debug, LOG_DEBUG, CFSTR("__SCNetworkServiceMigrateNew: Could not add service to set: %@"), newSet);
					continue;
				}
			}
		}
	}
	
	protocols = SCNetworkServiceCopyProtocols(service);
	if (protocols != NULL) {

		for (CFIndex idx = 0; idx < CFArrayGetCount(protocols); idx++) {
			SCNetworkProtocolRef protocol = CFArrayGetValueAtIndex(protocols, idx);
			CFDictionaryRef configuration = SCNetworkProtocolGetConfiguration(protocol);
			CFStringRef protocolType = SCNetworkProtocolGetProtocolType(protocol);
			enabled = SCNetworkProtocolGetEnabled(protocol);
			addProtocolToService(newService, protocolType, configuration, enabled);
		}
		CFRelease(protocols);
	}

	copyInterfaceConfiguration(service, newService);

	success = TRUE;
done:
	if (interface != NULL) {
		CFRelease(interface);
	}
	if (interfaceEntityMutable != NULL) {
		CFRelease(interfaceEntityMutable);
	}
	if (newService != NULL) {
		CFRelease(newService);
	}
	if (servicesMutable != NULL) {
		CFRelease(servicesMutable);
	}
	if (ni_prefs != NULL) {
		CFRelease(ni_prefs);
	}
	if (ni_interface != NULL) {
		CFRelease(ni_interface);
	}
	return success;
}


__private_extern__
Boolean
__SCNetworkServiceCreate(SCPreferencesRef	prefs,
			 SCNetworkInterfaceRef	interface,
			 CFStringRef		userDefinedName)
{
	SCNetworkSetRef currentSet = NULL;
	Boolean ok = FALSE;
	SCNetworkServiceRef service = NULL;

	if (interface == NULL) {
		goto done;
	}

	if (userDefinedName == NULL) {
		userDefinedName = __SCNetworkInterfaceGetUserDefinedName(interface);
		if (userDefinedName == NULL) {
			SCLog(TRUE, LOG_DEBUG, CFSTR("__SCNetworkServiceCreate: userDefinedName is NULL"));
			goto done;
		}
	}
	service = SCNetworkServiceCreate(prefs, interface);
	if (service == NULL) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("__SCNetworkServiceCreate: Failed to create service: %s"), SCErrorString(SCError()));
	} else {
		ok = SCNetworkServiceSetName(service, userDefinedName);
		if (ok == FALSE) {
			SCLog(TRUE, LOG_DEBUG, CFSTR("__SCNetworkServiceCreate: Failed to set name: %s"), SCErrorString(SCError()));
			SCNetworkServiceRemove(service);
			goto done;
		}

		ok = SCNetworkServiceEstablishDefaultConfiguration(service);
		if (ok == FALSE) {
			SCLog(TRUE, LOG_DEBUG, CFSTR("__SCNetworkServiceCreate: Failed to establish default configuration: %s"), SCErrorString(SCError()));
			SCNetworkServiceRemove(service);
			goto done;
		}
	}
	currentSet = SCNetworkSetCopyCurrent(prefs);
	if (currentSet == NULL) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("__SCNetworkServiceCreate: Could not copy current set"));
		if (service != NULL) {
			SCNetworkServiceRemove(service);
		}
		goto done;
	}
	if (service != NULL) {
		ok = SCNetworkSetAddService(currentSet, service);
		if (ok == FALSE) {
			SCLog(TRUE, LOG_DEBUG, CFSTR("__SCNetworkServiceCreate: Could not add service to the current set"));
			SCNetworkServiceRemove(service);
			goto done;
		}
	}

    done:
	if (service != NULL) {
		CFRelease(service);
	}
	if (currentSet != NULL) {
		CFRelease(currentSet);
	}
	return ok;
}

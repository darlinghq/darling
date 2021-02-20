/*
 * Copyright (c) 2004-2008, 2016-2018 Apple Inc. All rights reserved.
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
#include "SCNetworkConfigurationInternal.h"

#include <pthread.h>


static CFStringRef	__SCNetworkProtocolCopyDescription	(CFTypeRef cf);
static void		__SCNetworkProtocolDeallocate		(CFTypeRef cf);
static Boolean		__SCNetworkProtocolEqual		(CFTypeRef cf1, CFTypeRef cf2);
static CFHashCode	__SCNetworkProtocolHash			(CFTypeRef cf);


const CFStringRef kSCNetworkProtocolTypeDNS		= CFSTR("DNS");
const CFStringRef kSCNetworkProtocolTypeIPv4		= CFSTR("IPv4");
const CFStringRef kSCNetworkProtocolTypeIPv6		= CFSTR("IPv6");
const CFStringRef kSCNetworkProtocolTypeProxies		= CFSTR("Proxies");
#if	!TARGET_OS_IPHONE
const CFStringRef kSCNetworkProtocolTypeSMB		= CFSTR("SMB");
#endif	// !TARGET_OS_IPHONE


static CFTypeID __kSCNetworkProtocolTypeID	= _kCFRuntimeNotATypeID;


static const CFRuntimeClass __SCNetworkProtocolClass = {
	0,					// version
	"SCNetworkProtocol",			// className
	NULL,					// init
	NULL,					// copy
	__SCNetworkProtocolDeallocate,		// dealloc
	__SCNetworkProtocolEqual,		// equal
	__SCNetworkProtocolHash,		// hash
	NULL,					// copyFormattingDesc
	__SCNetworkProtocolCopyDescription	// copyDebugDesc
};


static pthread_once_t		initialized	= PTHREAD_ONCE_INIT;


static CFStringRef
__SCNetworkProtocolCopyDescription(CFTypeRef cf)
{
	CFAllocatorRef			allocator	= CFGetAllocator(cf);
	CFMutableStringRef		result;
	SCNetworkProtocolPrivateRef	protocolPrivate	= (SCNetworkProtocolPrivateRef)cf;

	result = CFStringCreateMutable(allocator, 0);
	CFStringAppendFormat(result, NULL, CFSTR("<SCNetworkProtocol %p [%p]> {"), cf, allocator);
	CFStringAppendFormat(result, NULL, CFSTR("id = %@"), protocolPrivate->entityID);
	CFStringAppendFormat(result, NULL, CFSTR(", service = %p"), protocolPrivate->service);
	CFStringAppendFormat(result, NULL,
			     CFSTR(", prefs = %p"),
			     ((SCNetworkServicePrivateRef)protocolPrivate->service)->prefs);
	CFStringAppendFormat(result, NULL, CFSTR("}"));

	return result;
}


static void
__SCNetworkProtocolDeallocate(CFTypeRef cf)
{
	SCNetworkProtocolPrivateRef	protocolPrivate	= (SCNetworkProtocolPrivateRef)cf;

	/* release resources */
	CFRelease(protocolPrivate->entityID);
	CFRelease(protocolPrivate->service);

	return;
}


static Boolean
__SCNetworkProtocolEqual(CFTypeRef cf1, CFTypeRef cf2)
{
	SCNetworkProtocolPrivateRef	p1	= (SCNetworkProtocolPrivateRef)cf1;
	SCNetworkProtocolPrivateRef	p2	= (SCNetworkProtocolPrivateRef)cf2;

	if (p1 == p2)
		return TRUE;

	if (!CFEqual(p1->entityID, p2->entityID))
		return FALSE;	// if not the same protocol type

	if (p1->service == p2->service)
		return TRUE;	// if both point to the same service

	if ((p1->service != NULL) && (p2->service != NULL) && CFEqual(p1->service, p2->service))
		return TRUE;	// if both effectively point to the same service

	return FALSE;
}


static CFHashCode
__SCNetworkProtocolHash(CFTypeRef cf)
{
	SCNetworkProtocolPrivateRef	protocolPrivate	= (SCNetworkProtocolPrivateRef)cf;

	return CFHash(protocolPrivate->entityID);
}


static void
__SCNetworkProtocolInitialize(void)
{
	__kSCNetworkProtocolTypeID = _CFRuntimeRegisterClass(&__SCNetworkProtocolClass);
	return;
}


__private_extern__ SCNetworkProtocolPrivateRef
__SCNetworkProtocolCreatePrivate(CFAllocatorRef		allocator,
				 CFStringRef		entityID,
				 SCNetworkServiceRef	service)
{
	SCNetworkProtocolPrivateRef		protocolPrivate;
	uint32_t				size;

	/* initialize runtime */
	pthread_once(&initialized, __SCNetworkProtocolInitialize);

	/* allocate target */
	size            = sizeof(SCNetworkProtocolPrivate) - sizeof(CFRuntimeBase);
	protocolPrivate = (SCNetworkProtocolPrivateRef)_CFRuntimeCreateInstance(allocator,
										__kSCNetworkProtocolTypeID,
										size,
										NULL);
	if (protocolPrivate == NULL) {
		return NULL;
	}

	/* initialize non-zero/NULL members */
	protocolPrivate->entityID	= CFStringCreateCopy(NULL, entityID);
	protocolPrivate->service	= CFRetain(service);

	return protocolPrivate;
}


__private_extern__ Boolean
__SCNetworkProtocolIsValidType(CFStringRef protocolType)
{
	static const CFStringRef	*valid_types[]	= {
		&kSCNetworkProtocolTypeDNS,
		&kSCNetworkProtocolTypeIPv4,
		&kSCNetworkProtocolTypeIPv6,
		&kSCNetworkProtocolTypeProxies,
#if	!TARGET_OS_IPHONE
		&kSCNetworkProtocolTypeSMB,
#endif	// !TARGET_OS_IPHONE
	};

	for (size_t i = 0; i < sizeof(valid_types)/sizeof(valid_types[0]); i++) {
		if (CFEqual(protocolType, *valid_types[i])) {
			// if known/valid protocol type
			return TRUE;
		}
	}

	if (CFStringFindWithOptions(protocolType,
				    CFSTR("."),
				    CFRangeMake(0, CFStringGetLength(protocolType)),
				    0,
				    NULL)) {
		// if user-defined protocol type (e.g. com.apple.myProtocol)
		return TRUE;
	}

	return FALSE;
}


static CFStringRef
copyProtocolConfigurationPath(SCNetworkProtocolPrivateRef protocolPrivate)
{
	CFStringRef			path;
	SCNetworkServicePrivateRef	servicePrivate;

	servicePrivate = (SCNetworkServicePrivateRef)protocolPrivate->service;
	path = SCPreferencesPathKeyCreateNetworkServiceEntity(NULL,				// allocator
							      servicePrivate->serviceID,	// service
							      protocolPrivate->entityID);	// entity
	return path;
}


#pragma mark -
#pragma mark SCNetworkProtocol APIs


CFComparisonResult
_SCNetworkProtocolCompare(const void *val1, const void *val2, void *context)
{
#pragma unused(context)
	SCNetworkProtocolRef	p1	= (SCNetworkProtocolRef)val1;
	SCNetworkProtocolRef	p2	= (SCNetworkProtocolRef)val2;
	CFStringRef		type1;
	CFStringRef		type2;

	type1 = SCNetworkProtocolGetProtocolType(p1);
	type2 = SCNetworkProtocolGetProtocolType(p2);

	return CFStringCompare(type1, type2, 0);
}


CFTypeID
SCNetworkProtocolGetTypeID()
{
	pthread_once(&initialized, __SCNetworkProtocolInitialize);	/* initialize runtime */
	return __kSCNetworkProtocolTypeID;
}


CFDictionaryRef
SCNetworkProtocolGetConfiguration(SCNetworkProtocolRef protocol)
{
	CFDictionaryRef			config;
	CFStringRef			path;
	SCNetworkProtocolPrivateRef	protocolPrivate	= (SCNetworkProtocolPrivateRef)protocol;
	SCNetworkServicePrivateRef	servicePrivate;

	if (!isA_SCNetworkProtocol(protocol)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	servicePrivate = (SCNetworkServicePrivateRef)protocolPrivate->service;
	path = copyProtocolConfigurationPath(protocolPrivate);
	config = __getPrefsConfiguration(servicePrivate->prefs, path);
	CFRelease(path);

	return config;
}


Boolean
SCNetworkProtocolGetEnabled(SCNetworkProtocolRef protocol)
{
	Boolean				enabled;
	CFStringRef			path;
	SCNetworkProtocolPrivateRef	protocolPrivate	= (SCNetworkProtocolPrivateRef)protocol;
	SCNetworkServicePrivateRef	servicePrivate;

	if (!isA_SCNetworkProtocol(protocol)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	servicePrivate = (SCNetworkServicePrivateRef)protocolPrivate->service;
	path = copyProtocolConfigurationPath(protocolPrivate);
	enabled = __getPrefsEnabled(servicePrivate->prefs, path);
	CFRelease(path);

	return enabled;
}


CFStringRef
SCNetworkProtocolGetProtocolType(SCNetworkProtocolRef protocol)
{
	SCNetworkProtocolPrivateRef	protocolPrivate	= (SCNetworkProtocolPrivateRef)protocol;

	if (!isA_SCNetworkProtocol(protocol)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	return protocolPrivate->entityID;
}


Boolean
SCNetworkProtocolSetConfiguration(SCNetworkProtocolRef protocol, CFDictionaryRef config)
{
	Boolean				ok;
	CFStringRef			path;
	SCNetworkProtocolPrivateRef	protocolPrivate	= (SCNetworkProtocolPrivateRef)protocol;
	SCNetworkServiceRef		service;
	SCNetworkServicePrivateRef	servicePrivate;

	if (!isA_SCNetworkProtocol(protocol)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	service = protocolPrivate->service;
	servicePrivate = (SCNetworkServicePrivateRef)service;
	if (!__SCNetworkServiceExists(service)) {
		SC_log(LOG_ERR, "SCNetworkProtocolSetConfiguration() w/removed service\n  protocol = %@\n  service = %@",
		       protocolPrivate->entityID,
		       servicePrivate);
		_SC_crash_once("SCNetworkProtocolSetConfiguration() w/removed service", NULL, NULL);
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	path = copyProtocolConfigurationPath(protocolPrivate);
	ok = __setPrefsConfiguration(servicePrivate->prefs, path, config, TRUE);
	CFRelease(path);

	if (ok) {
		SC_log(LOG_DEBUG, "SCNetworkProtocolSetConfiguration(): %@ --> %@",
		       protocol,
		       config != NULL ? config : (CFDictionaryRef)CFSTR("NULL"));
	}

	return ok;
}


Boolean
SCNetworkProtocolSetEnabled(SCNetworkProtocolRef protocol, Boolean enabled)
{
	Boolean				ok;
	CFStringRef			path;
	SCNetworkProtocolPrivateRef	protocolPrivate	= (SCNetworkProtocolPrivateRef)protocol;
	SCNetworkServiceRef		service;
	SCNetworkServicePrivateRef	servicePrivate;

	if (!isA_SCNetworkProtocol(protocol)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	service = protocolPrivate->service;
	servicePrivate = (SCNetworkServicePrivateRef)service;
	if (!__SCNetworkServiceExists(service)) {
		SC_log(LOG_ERR, "SCNetworkProtocolSetEnabled() w/removed service\n  protocol = %@\n  service = %@",
		       protocolPrivate->entityID,
		       servicePrivate);
		_SC_crash_once("SCNetworkProtocolSetEnabled() w/removed service", NULL, NULL);
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	path = copyProtocolConfigurationPath(protocolPrivate);
	ok = __setPrefsEnabled(servicePrivate->prefs, path, enabled);
	CFRelease(path);

	if (ok) {
		SC_log(LOG_DEBUG, "SCNetworkProtocolSetEnabled(): %@ -> %s",
		       protocol,
		       enabled ? "Enabled" : "Disabled");
	}

	return ok;
}

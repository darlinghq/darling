/*
 * Copyright (c) 2002-2006, 2009, 2011, 2013 Apple Inc. All rights reserved.
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
 * August 5, 2002	Allan Nathanson <ajn@apple.com>
 * - split code out from eventmon.c
 */

#include "eventmon.h"
#include "cache.h"
#include "ev_dlil.h"
#include "ev_extra.h"

#ifndef kSCEntNetIdleRoute
#define kSCEntNetIdleRoute       CFSTR("IdleRoute")
#endif  /* kSCEntNetIdleRoute */

static CFStringRef
create_interface_key(const char * if_name)
{
	CFStringRef		interface;
	CFStringRef		key;

	interface = CFStringCreateWithCString(NULL, if_name, kCFStringEncodingMacRoman);
	key       = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
								  kSCDynamicStoreDomainState,
								  interface,
								  kSCEntNetLink);
	CFRelease(interface);
	return (key);
}


static CFMutableDictionaryRef
copy_entity(CFStringRef key)
{
	CFDictionaryRef		dict;
	CFMutableDictionaryRef	newDict		= NULL;

	dict = cache_SCDynamicStoreCopyValue(store, key);
	if (dict != NULL) {
		if (isA_CFDictionary(dict) != NULL) {
			newDict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
		}
		CFRelease(dict);
	}
	if (newDict == NULL) {
		newDict = CFDictionaryCreateMutable(NULL,
						    0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);
	}
	return (newDict);
}


static void
interface_update_status(const char *if_name,
			CFBooleanRef active, boolean_t attach,
			CFBooleanRef expensive)
{
	CFStringRef		key		= NULL;
	CFMutableDictionaryRef	newDict		= NULL;

	key = create_interface_key(if_name);
	newDict = copy_entity(key);
	/* if new status available, update cache */
	if (active != NULL) {
		CFDictionarySetValue(newDict, kSCPropNetLinkActive, active);
	} else {
		CFDictionaryRemoveValue(newDict, kSCPropNetLinkActive);
	}

	if (attach == TRUE) {
		/* the interface was attached, remove stale state */
		CFDictionaryRemoveValue(newDict, kSCPropNetLinkDetaching);
	}

	if ((expensive != NULL) && CFBooleanGetValue(expensive)) {
		CFDictionarySetValue(newDict, kSCPropNetLinkExpensive, expensive);
	} else {
		CFDictionaryRemoveValue(newDict, kSCPropNetLinkExpensive);
	}

	/* update status */
	if (CFDictionaryGetCount(newDict) > 0) {
		cache_SCDynamicStoreSetValue(store, key, newDict);
	} else {
		cache_SCDynamicStoreRemoveValue(store, key);
	}

	CFRelease(key);
	CFRelease(newDict);
	return;
}


#ifdef KEV_DL_LINK_QUALITY_METRIC_CHANGED
static CFStringRef
create_linkquality_key(const char * if_name)
{
	CFStringRef		interface;
	CFStringRef		key;

	interface = CFStringCreateWithCString(NULL, if_name, kCFStringEncodingMacRoman);
	key = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
							    kSCDynamicStoreDomainState,
							    interface,
							    kSCEntNetLinkQuality);
	CFRelease(interface);
	return (key);
}


__private_extern__
void
interface_update_quality_metric(const char *if_name,
				int quality)
{
	CFStringRef  		key             = NULL;
	CFMutableDictionaryRef	newDict         = NULL;
	CFNumberRef		linkquality     = NULL;

	key = create_linkquality_key(if_name);
	newDict = copy_entity(key);

	if (quality != IFNET_LQM_THRESH_UNKNOWN) {
		linkquality = CFNumberCreate(NULL, kCFNumberIntType, &quality);
		CFDictionarySetValue(newDict, kSCPropNetLinkQuality, linkquality);
		CFRelease(linkquality);
	} else {
		CFDictionaryRemoveValue(newDict, kSCPropNetLinkQuality);
	}

	/* update status */
	if (CFDictionaryGetCount(newDict) > 0) {
		cache_SCDynamicStoreSetValue(store, key, newDict);
	} else {
		cache_SCDynamicStoreRemoveValue(store, key);
	}

	CFRelease(key);
	CFRelease(newDict);
	return;
}


static
void
link_update_quality_metric(const char *if_name)
{
	struct ifreq	ifr;
	int		quality = IFNET_LQM_THRESH_UNKNOWN;
	int		sock;

	sock = dgram_socket(AF_INET);
	if (sock == -1) {
		SCLog(TRUE, LOG_NOTICE, CFSTR("socket_get_link_quality: socket open failed,  %s"), strerror(errno));
		goto done;
	}

	bzero((char *)&ifr, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", if_name);

	if (ioctl(sock, SIOCGIFLINKQUALITYMETRIC, (caddr_t)&ifr) != -1) {
		quality = ifr.ifr_link_quality_metric;
	}

done:
	interface_update_quality_metric(if_name, quality);
	if (sock != -1)
		close(sock);
	return;

}
#endif /* KEV_DL_LINK_QUALITY_METRIC_CHANGED */


#ifdef	KEV_DL_ISSUES
static CFStringRef
create_link_issues_key(const char * if_name)
{
	CFStringRef	interface;
	CFStringRef	key;

	interface = CFStringCreateWithCString(NULL, if_name, kCFStringEncodingMacRoman);
	key = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
							    kSCDynamicStoreDomainState,
							    interface,
							    kSCEntNetLinkIssues);
	CFRelease(interface);
	return (key);
}


__private_extern__
void
interface_update_link_issues(const char		*if_name,
			     uint64_t		timestamp,
			     uint8_t		*modid,
			     size_t		modid_size,
			     uint8_t		*info,
			     size_t		info_size)
{
	CFDataRef		infoData;
	CFStringRef		key;
	CFDataRef		modidData;
	CFMutableDictionaryRef	newDict;
	CFDateRef		timeStamp;

	key = create_link_issues_key(if_name);

	newDict = copy_entity(key);

	modidData = CFDataCreate(NULL, modid, modid_size);
	CFDictionarySetValue(newDict, kSCPropNetLinkIssuesModuleID, modidData);
	CFRelease(modidData);

	if (info_size != 0) {
		infoData = CFDataCreate(NULL, info, info_size);
		CFDictionarySetValue(newDict, kSCPropNetLinkIssuesInfo, infoData);
		CFRelease(infoData);
	} else {
		CFDictionaryRemoveValue(newDict, kSCPropNetLinkIssuesInfo);
	}

	timeStamp = CFDateCreate(NULL, timestamp);
	CFDictionarySetValue(newDict, kSCPropNetLinkIssuesTimeStamp, timeStamp);
	CFRelease(timeStamp);

	cache_SCDynamicStoreSetValue(store, key, newDict);
	CFRelease(newDict);
	CFRelease(key);
	return;
}
#endif	/* KEV_DL_ISSUES */


__private_extern__
void
interface_detaching(const char *if_name)
{
	CFStringRef		key;
	CFMutableDictionaryRef	newDict;

	key = create_interface_key(if_name);
	newDict = copy_entity(key);
	CFDictionarySetValue(newDict, kSCPropNetLinkDetaching,
			     kCFBooleanTrue);
	cache_SCDynamicStoreSetValue(store, key, newDict);
	CFRelease(newDict);
	CFRelease(key);
	return;
}


static void
interface_remove(const char *if_name)
{
	CFStringRef		key;

	key = create_interface_key(if_name);
	cache_SCDynamicStoreRemoveValue(store, key);
	CFRelease(key);

#ifdef	KEV_DL_LINK_QUALITY_METRIC_CHANGED
	key = create_linkquality_key(if_name);
	cache_SCDynamicStoreRemoveValue(store, key);
	CFRelease(key);
#endif	/* KEV_DL_LINK_QUALITY_METRIC_CHANGED */

#ifdef	KEV_DL_ISSUES
	key = create_link_issues_key(if_name);
	cache_SCDynamicStoreRemoveValue(store, key);
	CFRelease(key);
#endif	/* KEV_DL_ISSUES */

	return;
}


__private_extern__
void
link_update_status(const char *if_name, boolean_t attach)
{
	CFBooleanRef		active		= NULL;
	CFBooleanRef		expensive;
	struct ifmediareq	ifm;
	int			sock;

	sock = dgram_socket(AF_INET);
	if (sock == -1) {
		SCLog(TRUE, LOG_NOTICE, CFSTR("link_update_status: socket open failed,  %s"), strerror(errno));
		return;
	}

	/* get "Link" */
	bzero((char *)&ifm, sizeof(ifm));
	(void) strncpy(ifm.ifm_name, if_name, sizeof(ifm.ifm_name));

	if (ioctl(sock, SIOCGIFMEDIA, (caddr_t)&ifm) == -1) {
		/* if media status not available for this interface */
		goto update;
	}

	if (ifm.ifm_count == 0) {
		/* no media types */
		goto update;
	}

	if (!(ifm.ifm_status & IFM_AVALID)) {
		/* if active bit not valid */
		goto update;
	}

	if (ifm.ifm_status & IFM_ACTIVE) {
		active = kCFBooleanTrue;
	} else {
		active = kCFBooleanFalse;
	}

 update:

	/* get "Expensive" */
	expensive = interface_update_expensive(if_name);

	/* update status */
	interface_update_status(if_name, active, attach, expensive);
	close(sock);
	return;
}

__private_extern__
void
link_add(const char *if_name)
{
	CFStringRef		interface;
	CFStringRef		cacheKey;
	CFDictionaryRef		dict;
	CFMutableDictionaryRef	newDict		= NULL;
	CFArrayRef		ifList;
	CFMutableArrayRef	newIFList	= NULL;

	interface = CFStringCreateWithCString(NULL, if_name, kCFStringEncodingMacRoman);
	cacheKey  = SCDynamicStoreKeyCreateNetworkInterface(NULL,
							    kSCDynamicStoreDomainState);

	dict = cache_SCDynamicStoreCopyValue(store, cacheKey);
	if (dict) {
		if (isA_CFDictionary(dict)) {
			newDict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
			ifList  = CFDictionaryGetValue(newDict, kSCPropNetInterfaces);
			if (isA_CFArray(ifList)) {
				newIFList = CFArrayCreateMutableCopy(NULL, 0, ifList);
			}
		}
		CFRelease(dict);
	}

	if (!newDict) {
		newDict = CFDictionaryCreateMutable(NULL,
						    0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);
	}

	if (!newIFList) {
		newIFList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}

	if (CFArrayContainsValue(newIFList,
				 CFRangeMake(0, CFArrayGetCount(newIFList)),
				 interface) == FALSE) {
		CFArrayAppendValue(newIFList, interface);
		CFDictionarySetValue(newDict, kSCPropNetInterfaces, newIFList);
	}
	cache_SCDynamicStoreSetValue(store, cacheKey, newDict);
	link_update_status(if_name, TRUE);
#ifdef KEV_DL_LINK_QUALITY_METRIC_CHANGED
	link_update_quality_metric(if_name);
#endif /* KEV_DL_LINK_QUALITY_METRIC_CHANGED */
	CFRelease(cacheKey);
	CFRelease(interface);
	if (newDict)	CFRelease(newDict);
	if (newIFList)	CFRelease(newIFList);
	return;
}


__private_extern__
void
link_remove(const char *if_name)
{
	CFStringRef		interface;
	CFStringRef		cacheKey;
	CFDictionaryRef		dict;
	CFMutableDictionaryRef	newDict		= NULL;
	CFArrayRef		ifList;
	CFMutableArrayRef	newIFList	= NULL;
	CFIndex			i;

	interface = CFStringCreateWithCString(NULL, if_name, kCFStringEncodingMacRoman);
	cacheKey  = SCDynamicStoreKeyCreateNetworkInterface(NULL,
							    kSCDynamicStoreDomainState);

	dict = cache_SCDynamicStoreCopyValue(store, cacheKey);
	if (dict) {
		if (isA_CFDictionary(dict)) {
			newDict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
			ifList  = CFDictionaryGetValue(newDict, kSCPropNetInterfaces);
			if (isA_CFArray(ifList)) {
				newIFList = CFArrayCreateMutableCopy(NULL, 0, ifList);
			}
		}
		CFRelease(dict);
	}

	if (!newIFList ||
	    ((i = CFArrayGetFirstIndexOfValue(newIFList,
					     CFRangeMake(0, CFArrayGetCount(newIFList)),
					     interface)) == kCFNotFound)
	   ) {
		/* we're not tracking this interface */
		goto done;
	}

	CFArrayRemoveValueAtIndex(newIFList, i);
	CFDictionarySetValue(newDict, kSCPropNetInterfaces, newIFList);
	cache_SCDynamicStoreSetValue(store, cacheKey, newDict);

	interface_remove(if_name);

    done:

	CFRelease(cacheKey);
	CFRelease(interface);
	if (newDict)	CFRelease(newDict);
	if (newIFList)	CFRelease(newIFList);
	return;
}


#ifdef KEV_DL_IF_IDLE_ROUTE_REFCNT
#define INVALID_SOCKET_REF 	-1
static
int
socket_reference_count(const char* if_name) {
	struct ifreq	ifr;
	int		ref = INVALID_SOCKET_REF;
	int		s;

	s = dgram_socket(AF_INET);
	if (s == -1) {
		return (ref);
	}

	bzero((char *)&ifr, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", if_name);

	if (ioctl(s, SIOCGIFGETRTREFCNT, (caddr_t)&ifr) != -1) {
		ref = ifr.ifr_route_refcnt;
	} else {
		ref = INVALID_SOCKET_REF;
	}
	close(s);
	return (ref);
}


__private_extern__
void
interface_update_idle_state(const char *if_name)
{
	CFStringRef		if_name_cf;
	CFStringRef		key;
	int			ref;

	/* We will only update the SCDynamicStore if the idle ref count
	 * is still 0 */
	ref = socket_reference_count(if_name);
	if (ref != 0) {
		return;
	}

	if_name_cf = CFStringCreateWithCString(NULL, if_name,
					       kCFStringEncodingASCII);

	key = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
							    kSCDynamicStoreDomainState,
							    if_name_cf,
							    kSCEntNetIdleRoute);

	cache_SCDynamicStoreNotifyValue(store, key);
	CFRelease(key);
	CFRelease(if_name_cf);
	return;
}
#endif	// KEV_DL_IF_IDLE_ROUTE_REFCNT

/*
 * Copyright (c) 2003, 2004, 2006, 2011 Apple Inc. All rights reserved.
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
 * May 1, 2003	Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>	// for SCLog()

#include "cache.h"


static	CFMutableDictionaryRef	cached_keys	= NULL;
static	CFMutableDictionaryRef	cached_set	= NULL;
static	CFMutableArrayRef	cached_removals	= NULL;
static	CFMutableArrayRef	cached_notifys	= NULL;


__private_extern__
void
cache_open(void)
{
	cached_keys     = CFDictionaryCreateMutable(NULL,
						    0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);
	cached_set      = CFDictionaryCreateMutable(NULL,
						    0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);
	cached_removals = CFArrayCreateMutable(NULL,
					       0,
					       &kCFTypeArrayCallBacks);
	cached_notifys  = CFArrayCreateMutable(NULL,
					       0,
					       &kCFTypeArrayCallBacks);

	return;
}


__private_extern__
CFPropertyListRef
cache_SCDynamicStoreCopyValue(SCDynamicStoreRef store, CFStringRef key)
{
	CFPropertyListRef	value;

	value = CFDictionaryGetValue(cached_set, key);
	if (value) {
		// if we have "set" a new value
		return (CFRetain(value));
	}

	if (CFArrayContainsValue(cached_removals,
				 CFRangeMake(0, CFArrayGetCount(cached_removals)),
				 key)) {
		// if we have "removed" the key
		_SCErrorSet(kSCStatusNoKey);
		return NULL;
	}

	value = CFDictionaryGetValue(cached_keys, key);
	if (value) {
		// if we have a cached value
		return (CFRetain(value));
	}

	value = SCDynamicStoreCopyValue(store, key);
	if (value) {
		CFDictionarySetValue(cached_keys, key, value);
	}

	return value;
}


__private_extern__
void
cache_SCDynamicStoreSetValue(SCDynamicStoreRef store, CFStringRef key, CFPropertyListRef value)
{
	CFIndex	i;

	i = CFArrayGetFirstIndexOfValue(cached_removals,
					CFRangeMake(0, CFArrayGetCount(cached_removals)),
					key);
	if (i != kCFNotFound) {
		// if previously "removed"
		CFArrayRemoveValueAtIndex(cached_removals, i);
	}

	CFDictionarySetValue(cached_set, key, value);

	return;
}

__private_extern__
void
cache_SCDynamicStoreRemoveValue(SCDynamicStoreRef store, CFStringRef key)
{
	CFDictionaryRemoveValue(cached_set, key);

	if (!CFArrayContainsValue(cached_removals,
				  CFRangeMake(0, CFArrayGetCount(cached_removals)),
				  key)) {
		CFArrayAppendValue(cached_removals, key);
	}

	return;
}


__private_extern__
void
cache_SCDynamicStoreNotifyValue(SCDynamicStoreRef store, CFStringRef key)
{
	if (!CFArrayContainsValue(cached_notifys,
				  CFRangeMake(0, CFArrayGetCount(cached_notifys)),
				  key)) {
		CFArrayAppendValue(cached_notifys, key);
	}

	return;
}


__private_extern__
void
cache_write(SCDynamicStoreRef store)
{
	if ((CFDictionaryGetCount(cached_set) > 0) ||
	    (CFArrayGetCount(cached_removals) > 0) ||
	    (CFArrayGetCount(cached_notifys)  > 0)) {
		if (!SCDynamicStoreSetMultiple(store,
				 cached_set,
				 cached_removals,
				 cached_notifys)) {
			SCLog(TRUE,
			      LOG_ERR,
			      CFSTR("SCDynamicStoreSetMultiple() failed: %s"),
			      SCErrorString(SCError()));
		}
	}

	return;
}


__private_extern__
void
cache_close(void)
{
	CFRelease(cached_keys);
	CFRelease(cached_set);
	CFRelease(cached_removals);
	CFRelease(cached_notifys);

	return;
}

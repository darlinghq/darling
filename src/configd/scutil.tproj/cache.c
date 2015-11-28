/*
 * Copyright (c) 2000-2005, 2011 Apple Inc. All rights reserved.
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
 * June 1, 2001			Allan Nathanson <ajn@apple.com>
 * - public API conversion
 *
 * November 9, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <sys/types.h>

#include "scutil.h"
#include "cache.h"


#pragma mark -
#pragma mark SCDynamicStore "cache"


static	Boolean			use_cache	= FALSE;

static	CFMutableDictionaryRef	cached_keys	= NULL;
static	CFMutableDictionaryRef	cached_set	= NULL;
static	CFMutableArrayRef	cached_removals	= NULL;
static	CFMutableArrayRef	cached_notifys	= NULL;


static void
cache_open(void)
{
	if (use_cache) {
		// if we are already using the cache
		cache_close();
	}

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

	use_cache = TRUE;
	return;
}


static CFPropertyListRef
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


static void
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

static void
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


static void
cache_SCDynamicStoreNotifyValue(SCDynamicStoreRef store, CFStringRef key)
{
	if (!CFArrayContainsValue(cached_notifys,
				  CFRangeMake(0, CFArrayGetCount(cached_notifys)),
				  key)) {
		CFArrayAppendValue(cached_notifys, key);
	}

	return;
}


static void
cache_write(SCDynamicStoreRef store)
{
	if ((CFDictionaryGetCount(cached_set) > 0) ||
	    (CFArrayGetCount(cached_removals) > 0) ||
	    (CFArrayGetCount(cached_notifys)  > 0)) {
		if (!SCDynamicStoreSetMultiple(store,
					       cached_set,
					       cached_removals,
					       cached_notifys)) {
			SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
		}
	}

	return;
}


__private_extern__
void
cache_close(void)
{
	if (!use_cache) {
		return;
	}

	CFRelease(cached_keys);
	CFRelease(cached_set);
	CFRelease(cached_removals);
	CFRelease(cached_notifys);

	use_cache = FALSE;
	return;
}


#pragma mark -
#pragma mark SCDynamicStore operations


__private_extern__
void
do_block(int argc, char **argv)
{
	Boolean		enable	= FALSE;

	if (argc >= 1) {
		if        ((strcasecmp(argv[0], "begin") == 0) ||
			   (strcasecmp(argv[0], "start") == 0) ||
			   (strcasecmp(argv[0], "on"   ) == 0) ||
			   (strcasecmp(argv[0], "1"    ) == 0)) {
			enable = TRUE;
		} else if ((strcasecmp(argv[0], "end"  ) == 0) ||
			   (strcasecmp(argv[0], "stop" ) == 0) ||
			   (strcasecmp(argv[0], "off"  ) == 0) ||
			   (strcasecmp(argv[0], "0"    ) == 0)) {
			enable = FALSE;
		} else {
			SCPrint(TRUE, stdout, CFSTR("invalid value\n"));
			return;
		}
	} else {
		enable = !use_cache;	// toggle
	}

	if (enable) {
		// begin block of SCDynamicStore operations
		if (use_cache) {
			SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(kSCStatusLocked));
			return;
		}

		SCPrint(TRUE, stdout, CFSTR("Begin block of SCDynamicStore operations\n"));

		cache_open();
	} else {
		CFIndex	n;

		// end block of SCDynamicStore operations
		if (!use_cache) {
			SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(kSCStatusNeedLock));
			return;
		}

		n = CFDictionaryGetCount(cached_keys) +
		    CFArrayGetCount(cached_removals)  +
		    CFArrayGetCount(cached_notifys);
		SCPrint(TRUE, stdout,
			CFSTR("End block of SCDynamicStore operations%s\n"),
			(n > 0) ? ", posting changes" : "");
		if (n > 0) {
			cache_write(store);
		}
		cache_close();
	}

	return;
}


static CFComparisonResult
sort_keys(const void *p1, const void *p2, void *context) {
	CFStringRef key1 = (CFStringRef)p1;
	CFStringRef key2 = (CFStringRef)p2;
	return CFStringCompare(key1, key2, 0);
}


#define	N_QUICK	64


__private_extern__
void
do_list(int argc, char **argv)
{
	int			i;
	CFStringRef		pattern;
	CFArrayRef		list;
	CFIndex			listCnt;
	CFMutableArrayRef	sortedList;

	pattern = CFStringCreateWithCString(NULL,
					    (argc >= 1) ? argv[0] : ".*",
					    kCFStringEncodingUTF8);

	list = SCDynamicStoreCopyKeyList(store, pattern);
	CFRelease(pattern);
	if (list == NULL) {
		if (SCError() != kSCStatusOK) {
			SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
			return;
		} else {
		    if (!use_cache) {
			SCPrint(TRUE, stdout, CFSTR("  no keys.\n"));
			return;
		    } else {
			CFIndex	n;

			n = CFDictionaryGetCount(cached_set);
			if (n > 0) {
				const void *	cachedKeys_q[N_QUICK];
				const void **	cachedKeys	= cachedKeys_q;

				if (n > (CFIndex)(sizeof(cachedKeys_q) / sizeof(CFStringRef))) {
					cachedKeys = CFAllocatorAllocate(NULL, n * sizeof(CFStringRef), 0);
				}
				CFDictionaryGetKeysAndValues(cached_set, cachedKeys, NULL);
				list = CFArrayCreate(NULL, cachedKeys, n, &kCFTypeArrayCallBacks);
				if (cachedKeys != cachedKeys_q) {
					CFAllocatorDeallocate(NULL, cachedKeys);
				}
			} else {
				SCPrint(TRUE, stdout, CFSTR("  no keys.\n"));
				return;
			}
		    }
		}
	} else if (use_cache &&
		   ((CFDictionaryGetCount(cached_set) > 0) || (CFArrayGetCount(cached_removals) > 0))) {
		SCPrint(TRUE, stdout,
			CFSTR("  Note: SCDynamicStore transactions in progress, key list (below) may be out of date.\n\n"));
	}

	listCnt = CFArrayGetCount(list);
	sortedList = CFArrayCreateMutableCopy(NULL, listCnt, list);
	CFRelease(list);
	CFArraySortValues(sortedList,
			  CFRangeMake(0, listCnt),
			  sort_keys,
			  NULL);

	if (listCnt > 0) {
		for (i = 0; i < listCnt; i++) {
			SCPrint(TRUE,
				stdout,
				CFSTR("  subKey [%d] = %@\n"),
				i,
				CFArrayGetValueAtIndex(sortedList, i));
		}
	} else {
		SCPrint(TRUE, stdout, CFSTR("  no keys.\n"));
	}
	CFRelease(sortedList);

	return;
}


__private_extern__
void
do_add(int argc, char **argv)
{
	CFStringRef	key;

	key    = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);

	if (argc < 2) {
		if (!use_cache) {
			if (!SCDynamicStoreAddValue(store, key, value)) {
				SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
			}
		} else {
			CFTypeRef	val;

			val = cache_SCDynamicStoreCopyValue(store, key);
			if (val != NULL) {
				CFRelease(val);
				SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(kSCStatusKeyExists));
			} else {
				cache_SCDynamicStoreSetValue(store, key, value);
			}
		}
	} else {
		if (!use_cache) {
			if (!SCDynamicStoreAddTemporaryValue(store, key, value)) {
				SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
			}
		} else {
			CFTypeRef	val;

			val = cache_SCDynamicStoreCopyValue(store, key);
			if (val != NULL) {
				CFRelease(val);
				SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(kSCStatusKeyExists));
			} else {
				if (!SCDynamicStoreAddTemporaryValue(store, key, value)) {
					SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
				} else {
					// and save the temp value in the cache too!
					cache_SCDynamicStoreSetValue(store, key, value);
				}
			}
		}
	}

	CFRelease(key);
	return;
}


__private_extern__
void
do_get(int argc, char **argv)
{
	CFStringRef		key;
	CFPropertyListRef	newValue;

	key      = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	if (!use_cache) {
		newValue = SCDynamicStoreCopyValue(store, key);
	} else {
		newValue = cache_SCDynamicStoreCopyValue(store, key);
	}
	CFRelease(key);
	if (newValue == NULL) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
		return;
	}

	if (value != NULL) {
		CFRelease(value);		/* we have new information, release the old */
	}
	value = newValue;

	return;
}


__private_extern__
void
do_set(int argc, char **argv)
{
	CFStringRef	key;

	key = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	if (!use_cache) {
		if (!SCDynamicStoreSetValue(store, key, value)) {
			SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
		}
	} else {
		cache_SCDynamicStoreSetValue(store, key, value);
	}
	CFRelease(key);
	return;
}


__private_extern__
void
do_show(int argc, char **argv)
{
	CFStringRef		key;
	CFPropertyListRef	newValue;

	key = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);

	if (argc == 1) {
		if (!use_cache) {
			newValue = SCDynamicStoreCopyValue(store, key);
		} else {
			newValue = cache_SCDynamicStoreCopyValue(store, key);
		}
	} else {
		CFArrayRef	patterns;

		patterns = CFArrayCreate(NULL, (const void **)&key, 1, &kCFTypeArrayCallBacks);
		if (!use_cache) {
			newValue = SCDynamicStoreCopyMultiple(store, NULL, patterns);
		} else {
			CFArrayRef		keys;
			CFMutableDictionaryRef	newDict;

			newDict = CFDictionaryCreateMutable(NULL,
							    0,
							    &kCFTypeDictionaryKeyCallBacks,
							    &kCFTypeDictionaryValueCallBacks);
			keys = SCDynamicStoreCopyKeyList(store, key);
			if (keys != NULL) {
				CFIndex		i;
				CFIndex		n;

				n = CFArrayGetCount(keys);
				for (i = 0; i < n; i++) {
					CFStringRef	storeKey;
					CFTypeRef	storeVal;

					storeKey = CFArrayGetValueAtIndex(keys, i);
					storeVal = cache_SCDynamicStoreCopyValue(store, storeKey);
					if (storeVal != NULL) {
						CFDictionarySetValue(newDict, storeKey, storeVal);
						CFRelease(storeVal);
					}
				}
				CFRelease(keys);
			}

			if ((CFDictionaryGetCount(cached_set) > 0) || (CFArrayGetCount(cached_removals) > 0)) {
				SCPrint(TRUE, stdout, CFSTR("  Note: SCDynamicStore locked, keys included (below) may be out of date.\n\n"));
			}

			newValue = newDict;
		}
		CFRelease(patterns);
	}

	CFRelease(key);
	if (newValue == NULL) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
		return;
	}

	SCPrint(TRUE, stdout, CFSTR("%@\n"), newValue);
	CFRelease(newValue);
	return;
}


__private_extern__
void
do_remove(int argc, char **argv)
{
	CFStringRef	key;

	key = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	if (!use_cache) {
		if (!SCDynamicStoreRemoveValue(store, key)) {
			SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
		}
	} else {
		cache_SCDynamicStoreRemoveValue(store, key);
	}
	CFRelease(key);
	return;
}


__private_extern__
void
do_notify(int argc, char **argv)
{
	CFStringRef	key;

	key = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	if (!use_cache) {
		if (!SCDynamicStoreNotifyValue(store, key)) {
			SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
		}
	} else {
		cache_SCDynamicStoreNotifyValue(store, key);
	}
	CFRelease(key);
	return;
}

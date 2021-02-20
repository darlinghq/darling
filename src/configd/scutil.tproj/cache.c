/*
 * Copyright (c) 2000-2005, 2011, 2017, 2018 Apple Inc. All rights reserved.
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
#include "SCDynamicStoreInternal.h"


#pragma mark -
#pragma mark SCDynamicStore "cache"


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
		enable = !_SCDynamicStoreCacheIsActive(store);	// toggle
	}

	if (enable) {
		// begin block of SCDynamicStore operations
		if (_SCDynamicStoreCacheIsActive(store)) {
			SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(kSCStatusLocked));
			return;
		}

		SCPrint(TRUE, stdout, CFSTR("Begin block of SCDynamicStore operations\n"));

		_SCDynamicStoreCacheOpen(store);
	} else {
		CFIndex				n		= 0;
		SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

		// end block of SCDynamicStore operations
		if (!_SCDynamicStoreCacheIsActive(store)) {
			SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(kSCStatusNeedLock));
			return;
		}

		n += (storePrivate->cached_set != NULL)
			? CFDictionaryGetCount(storePrivate->cached_set) : 0;
		n += (storePrivate->cached_removals != NULL)
			? CFArrayGetCount(storePrivate->cached_removals) : 0;
		n += (storePrivate->cached_notifys != NULL)
			? CFArrayGetCount(storePrivate->cached_notifys) : 0;

		SCPrint(TRUE, stdout,
			CFSTR("End block of SCDynamicStore operations%s\n"),
			(n > 0) ? ", posting changes" : "");
		if (n > 0) {
			_SCDynamicStoreCacheCommitChanges(store);
		}
		_SCDynamicStoreCacheClose(store);
	}

	return;
}


static CFComparisonResult
sort_keys(const void *p1, const void *p2, void *context)
{
#pragma unused(context)
	CFStringRef key1 = (CFStringRef)p1;
	CFStringRef key2 = (CFStringRef)p2;
	return CFStringCompare(key1, key2, 0);
}


#define	N_QUICK	64


__private_extern__
void
do_list(int argc, char **argv)
{
	int				i;
	CFStringRef			pattern;
	CFArrayRef			list;
	CFIndex				listCnt;
	CFMutableArrayRef		sortedList;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

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
		    if (!_SCDynamicStoreCacheIsActive(store)) {
			SCPrint(TRUE, stdout, CFSTR("  no keys.\n"));
			return;
		    } else {
			CFIndex	n;

			n = (storePrivate->cached_set != NULL)
				? CFDictionaryGetCount(storePrivate->cached_set) : 0;
			if (n > 0) {
				const void *	cachedKeys_q[N_QUICK];
				const void **	cachedKeys	= cachedKeys_q;

				if (n > (CFIndex)(sizeof(cachedKeys_q) / sizeof(CFStringRef))) {
					cachedKeys = CFAllocatorAllocate(NULL, n * sizeof(CFStringRef), 0);
				}
				CFDictionaryGetKeysAndValues(storePrivate->cached_set, cachedKeys, NULL);
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
	} else if (_SCDynamicStoreCacheIsActive(store) &&
		   (((storePrivate->cached_set != NULL) &&
		     (CFDictionaryGetCount(storePrivate->cached_set) > 0)) ||
		    ((storePrivate->cached_removals != NULL) &&
		     (CFArrayGetCount(storePrivate->cached_removals) > 0)))) {
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
		if (!_SCDynamicStoreCacheIsActive(store)) {
			if (!SCDynamicStoreAddValue(store, key, value)) {
				SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
			}
		} else {
			SCPrint(TRUE, stdout, CFSTR("  Cannot \"add\" with block\n"));
		}
	} else {
		if (!_SCDynamicStoreCacheIsActive(store)) {
			if (!SCDynamicStoreAddTemporaryValue(store, key, value)) {
				SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
			}
		} else {
			SCPrint(TRUE, stdout, CFSTR("  Cannot \"add temporary\" with block\n"));
		}
	}

	CFRelease(key);
	return;
}


__private_extern__
void
do_get(int argc, char **argv)
{
#pragma unused(argc)
	CFStringRef		key;
	CFPropertyListRef	newValue;

	key      = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	newValue = SCDynamicStoreCopyValue(store, key);
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
#pragma unused(argc)
	CFStringRef	key;

	key = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	if (!SCDynamicStoreSetValue(store, key, value)) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
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
		newValue = SCDynamicStoreCopyValue(store, key);
	} else {
		CFArrayRef	patterns;

		patterns = CFArrayCreate(NULL, (const void **)&key, 1, &kCFTypeArrayCallBacks);
		if (!_SCDynamicStoreCacheIsActive(store)) {
			newValue = SCDynamicStoreCopyMultiple(store, NULL, patterns);
		} else {
			CFArrayRef			keys;
			CFMutableDictionaryRef		newDict;
			SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

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
					storeVal = SCDynamicStoreCopyValue(store, storeKey);
					if (storeVal != NULL) {
						CFDictionarySetValue(newDict, storeKey, storeVal);
						CFRelease(storeVal);
					}
				}
				CFRelease(keys);
			}

			if (((storePrivate->cached_set != NULL) &&
			     (CFDictionaryGetCount(storePrivate->cached_set) > 0)) ||
			    ((storePrivate->cached_removals != NULL) &&
			     (CFArrayGetCount(storePrivate->cached_removals) > 0))) {
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
#pragma unused(argc)
	CFStringRef	key;

	key = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	if (!SCDynamicStoreRemoveValue(store, key)) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
	}
	CFRelease(key);
	return;
}


__private_extern__
void
do_notify(int argc, char **argv)
{
#pragma unused(argc)
	CFStringRef	key;

	key = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	if (!SCDynamicStoreNotifyValue(store, key)) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
	}
	CFRelease(key);
	return;
}

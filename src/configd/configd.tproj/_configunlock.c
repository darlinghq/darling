/*
 * Copyright (c) 2000-2004, 2006, 2011 Apple Inc. All rights reserved.
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
 * March 24, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#include "configd.h"
#include "configd_server.h"
#include "session.h"
#include "pattern.h"


#define N_QUICK	32


static void
_notifyWatchers()
{
	CFIndex			keyCnt;
	const void *		keys_q[N_QUICK];
	const void **		keys	= keys_q;

	keyCnt = CFSetGetCount(changedKeys);
	if (keyCnt == 0)
		return;		/* if nothing to do */

	if (keyCnt > (CFIndex)(sizeof(keys_q) / sizeof(CFStringRef)))
		keys = CFAllocatorAllocate(NULL, keyCnt * sizeof(CFStringRef), 0);

	CFSetGetValues(changedKeys, keys);

	while (--keyCnt >= 0) {
		CFArrayRef		changes;
		CFDictionaryRef		dict;
		CFDictionaryRef		info;
		CFMutableDictionaryRef	newInfo;
		CFMutableArrayRef	newChanges;
		CFArrayRef		sessionsWatchingKey;
		CFIndex			watcherCnt;
		const void *		watchers_q[N_QUICK];
		const void **		watchers	= watchers_q;

		dict = CFDictionaryGetValue(storeData, (CFStringRef)keys[keyCnt]);
		if ((dict == NULL) || (CFDictionaryContainsKey(dict, kSCDWatchers) == FALSE)) {
			/* key doesn't exist or nobody cares if it changed */
			continue;
		}

		/*
		 * Add this key to the list of changes for each of the
		 * sessions which is "watching".
		 */
		sessionsWatchingKey = CFDictionaryGetValue(dict, kSCDWatchers);
		watcherCnt = CFArrayGetCount(sessionsWatchingKey);
		if (watcherCnt == 0) {
			/* if no watchers */
			continue;
		}

		if (watcherCnt > (CFIndex)(sizeof(watchers_q) / sizeof(CFNumberRef)))
			watchers = CFAllocatorAllocate(NULL, watcherCnt * sizeof(CFNumberRef), 0);

		CFArrayGetValues(sessionsWatchingKey, CFRangeMake(0, watcherCnt), watchers);

		while (--watcherCnt >= 0) {
			CFStringRef	sessionKey;

			sessionKey = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@"), watchers[watcherCnt]);
			info = CFDictionaryGetValue(sessionData, sessionKey);
			if (info) {
				newInfo = CFDictionaryCreateMutableCopy(NULL, 0, info);
			} else {
				newInfo = CFDictionaryCreateMutable(NULL,
								    0,
								    &kCFTypeDictionaryKeyCallBacks,
								    &kCFTypeDictionaryValueCallBacks);
			}

			changes = CFDictionaryGetValue(newInfo, kSCDChangedKeys);
			if (changes) {
				newChanges = CFArrayCreateMutableCopy(NULL, 0, changes);
			} else {
				newChanges = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
			}

			if (CFArrayContainsValue(newChanges,
						 CFRangeMake(0, CFArrayGetCount(newChanges)),
						 (CFStringRef)keys[keyCnt]) == FALSE) {
				CFArrayAppendValue(newChanges, (CFStringRef)keys[keyCnt]);
			}
			CFDictionarySetValue(newInfo, kSCDChangedKeys, newChanges);
			CFRelease(newChanges);
			CFDictionarySetValue(sessionData, sessionKey, newInfo);
			CFRelease(newInfo);
			CFRelease(sessionKey);

			/*
			 * flag this session as needing a kick
			 */
			if (needsNotification == NULL)
				needsNotification = CFSetCreateMutable(NULL,
								       0,
								       &kCFTypeSetCallBacks);
			CFSetAddValue(needsNotification, watchers[watcherCnt]);
		}

		if (watchers != watchers_q) CFAllocatorDeallocate(NULL, watchers);
	}

	if (keys != keys_q) CFAllocatorDeallocate(NULL, keys);

	/*
	 * The list of changed keys have been updated for any sessions
	 * monitoring changes to the "store". The next step, handled by
	 * the "configd" server, is to push out any needed notifications.
	 */
	CFSetRemoveAllValues(changedKeys);

}


static void
_processDeferredRemovals()
{
	CFIndex			keyCnt;
	const void *		keys_q[N_QUICK];
	const void **		keys	= keys_q;

	keyCnt = CFSetGetCount(deferredRemovals);
	if (keyCnt == 0)
		return;		/* if nothing to do */

	if (keyCnt > (CFIndex)(sizeof(keys_q) / sizeof(CFStringRef)))
		keys = CFAllocatorAllocate(NULL, keyCnt * sizeof(CFStringRef), 0);

	CFSetGetValues(deferredRemovals, keys);

	while (--keyCnt >= 0) {
		patternRemoveKey((CFStringRef)keys[keyCnt]);
	}

	if (keys != keys_q) CFAllocatorDeallocate(NULL, keys);

	/*
	 * All regex keys associated with removed store dictionary keys have
	 * been removed. Start the list fresh again.
	 */
	CFSetRemoveAllValues(deferredRemovals);

	return;
}


static void
_cleanupRemovedSessionKeys(const void *value, void *context)
{
	CFStringRef		removedKey = (CFStringRef)value;
	CFRange			dRange;
	CFStringRef		sessionKey;
	CFStringRef		key;
	CFDictionaryRef		sessionDict;
	CFArrayRef		sessionKeys;
	CFIndex			i;
	CFMutableDictionaryRef	newSessionDict;

	dRange     = CFStringFind(removedKey, CFSTR(":"), 0);
	sessionKey = CFStringCreateWithSubstring(NULL,
						 removedKey,
						 CFRangeMake(0, dRange.location));
	key        = CFStringCreateWithSubstring(NULL,
						 removedKey,
						 CFRangeMake(dRange.location+dRange.length,
							     CFStringGetLength(removedKey)-dRange.location-dRange.length));

	/*
	 * remove the key from the session key list
	 */
	sessionDict = CFDictionaryGetValue(sessionData, sessionKey);
	if (!sessionDict) {
		/* if no session */
		goto done;
	}

	sessionKeys = CFDictionaryGetValue(sessionDict, kSCDSessionKeys);
	if (!sessionKeys) {
		/* if no session keys */
		goto done;
	}

	i = CFArrayGetFirstIndexOfValue(sessionKeys,
					CFRangeMake(0, CFArrayGetCount(sessionKeys)),
					key);
	if (i == kCFNotFound) {
		/* if this session key has already been removed */
		goto done;
	}

	newSessionDict = CFDictionaryCreateMutableCopy(NULL, 0, sessionDict);
	if (CFArrayGetCount(sessionKeys) == 1) {
		/* remove the last (session) key */
		CFDictionaryRemoveValue(newSessionDict, kSCDSessionKeys);
	} else {
		CFMutableArrayRef	newSessionKeys;

		/* remove the (session) key */
		newSessionKeys = CFArrayCreateMutableCopy(NULL, 0, sessionKeys);
		CFArrayRemoveValueAtIndex(newSessionKeys, i);
		CFDictionarySetValue(newSessionDict, kSCDSessionKeys, newSessionKeys);
		CFRelease(newSessionKeys);
	}
	CFDictionarySetValue(sessionData, sessionKey, newSessionDict);
	CFRelease(newSessionDict);

    done:

	CFRelease(sessionKey);
	CFRelease(key);

	return;
}


__private_extern__
int
__SCDynamicStorePush(void)
{
	/*
	 * push notifications to any session watching those keys which
	 * were recently changed.
	 */
	_notifyWatchers();

	/*
	 * process any deferred key deletions.
	 */
	_processDeferredRemovals();

	/*
	 * clean up any removed session keys
	 */
	CFSetApplyFunction(removedSessionKeys, _cleanupRemovedSessionKeys, NULL);
	CFSetRemoveAllValues(removedSessionKeys);

	return kSCStatusOK;
}

/*
 * Copyright (c) 2000-2004, 2006, 2008, 2011, 2012, 2014 Apple Inc. All rights reserved.
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
#include "session.h"
#include "pattern.h"


__private_extern__
int
__SCDynamicStoreSetValue(SCDynamicStoreRef store, CFStringRef key, CFDataRef value, Boolean internal)
{
	CFDictionaryRef			dict;
	CFMutableDictionaryRef		newDict;
	Boolean				newEntry	= FALSE;
	int				sc_status	= kSCStatusOK;
	CFStringRef			sessionKey;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;
	CFStringRef			storeSessionKey;

	if (_configd_trace) {
		SCTrace(TRUE, _configd_trace,
			CFSTR("%s%s : %5d : %@\n"),
			internal ? "*set " : "set  ",
			storePrivate->useSessionKeys ? "t " : "  ",
			storePrivate->server,
			key);
	}

	/*
	 * Grab the current (or establish a new) dictionary for this key.
	 */

	dict = CFDictionaryGetValue(storeData, key);
	if (dict) {
		newDict = CFDictionaryCreateMutableCopy(NULL,
							0,
							dict);
	} else {
		newDict = CFDictionaryCreateMutable(NULL,
						    0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);
	}

	/*
	 * Update the dictionary entry to be saved to the store.
	 */
	newEntry = !CFDictionaryContainsKey(newDict, kSCDData);
	CFDictionarySetValue(newDict, kSCDData, value);

	sessionKey = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), storePrivate->server);

	/*
	 * Manage per-session keys.
	 */
	if (storePrivate->useSessionKeys) {
		if (newEntry) {
			CFArrayRef		keys;
			CFMutableDictionaryRef	newSession;
			CFMutableArrayRef	newKeys;
			CFDictionaryRef		session;

			/*
			 * Add this key to my list of per-session keys
			 */
			session = CFDictionaryGetValue(sessionData, sessionKey);
			keys = CFDictionaryGetValue(session, kSCDSessionKeys);
			if ((keys == NULL) ||
			    (CFArrayGetFirstIndexOfValue(keys,
							 CFRangeMake(0, CFArrayGetCount(keys)),
							 key) == kCFNotFound)) {
				/*
				 * if no session keys defined "or" keys defined but not
				 * this one...
				 */
				if (keys != NULL) {
					/* this is the first session key */
					newKeys = CFArrayCreateMutableCopy(NULL, 0, keys);
				} else {
					/* this is an additional session key */
					newKeys = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
				}
				CFArrayAppendValue(newKeys, key);

				/* update session dictionary */
				newSession = CFDictionaryCreateMutableCopy(NULL, 0, session);
				CFDictionarySetValue(newSession, kSCDSessionKeys, newKeys);
				CFRelease(newKeys);
				CFDictionarySetValue(sessionData, sessionKey, newSession);
				CFRelease(newSession);
			}

			/*
			 * Mark the key as a "session" key and track the creator.
			 */
			CFDictionarySetValue(newDict, kSCDSession, sessionKey);
		} else {
			/*
			 * Since we are using per-session keys and this key already
			 * exists, check if it was created by "our" session
			 */
			dict = CFDictionaryGetValue(storeData, key);
			if (!CFDictionaryGetValueIfPresent(dict, kSCDSession, (void *)&storeSessionKey) ||
			    !CFEqual(sessionKey, storeSessionKey)) {
				/*
				 * if the key exists and is not a session key or
				 * if the key exists it's not "our" session.
				 */
				sc_status = kSCStatusKeyExists;
				CFRelease(sessionKey);
				CFRelease(newDict);
				goto done;
			}
		}
	} else {
		/*
		* Since we are updating this key we need to check and, if
		* necessary, remove the indication that this key is on
		* another session's remove-on-close list.
		*/
		if (!newEntry &&
		    CFDictionaryGetValueIfPresent(newDict, kSCDSession, (void *)&storeSessionKey) &&
		    !CFEqual(sessionKey, storeSessionKey)) {
			CFStringRef	removedKey;

			/* We are no longer a session key! */
			CFDictionaryRemoveValue(newDict, kSCDSession);

			/* add this session key to the (session) removal list */
			removedKey = CFStringCreateWithFormat(NULL, 0, CFSTR("%@:%@"), storeSessionKey, key);
			CFSetAddValue(removedSessionKeys, removedKey);
			CFRelease(removedKey);
		}
	}

	CFRelease(sessionKey);

	/*
	 * Update the dictionary entry in the store.
	 */

	CFDictionarySetValue(storeData, key, newDict);
	CFRelease(newDict);

	/*
	 * For "new" entries to the store, check the deferred cleanup
	 * list. If the key is flagged for removal, remove it from the
	 * list since any defined regex's for this key are still defined
	 * and valid. If the key is not flagged then iterate over the
	 * sessionData dictionary looking for regex keys which match the
	 * updated key. If a match is found then we mark those keys as
	 * being watched.
	 */

	if (newEntry) {
		if (CFSetContainsValue(deferredRemovals, key)) {
			CFSetRemoveValue(deferredRemovals, key);
		} else {
			patternAddKey(key);
		}
	}

	/*
	 * Mark this key as "changed". Any "watchers" will be notified
	 * as soon as the lock is released.
	 */
	CFSetAddValue(changedKeys, key);

    done :

	if (!internal) {
		/* push changes */
		__SCDynamicStorePush();
	}

	return sc_status;
}

__private_extern__
kern_return_t
_configset(mach_port_t			server,
	   xmlData_t			keyRef,		/* raw XML bytes */
	   mach_msg_type_number_t	keyLen,
	   xmlData_t			dataRef,	/* raw XML bytes */
	   mach_msg_type_number_t	dataLen,
	   int				oldInstance,
	   int				*newInstance,
	   int				*sc_status,
	   audit_token_t		audit_token)
{
	CFDataRef		data		= NULL;	/* data (un-serialized) */
	CFStringRef		key		= NULL;	/* key  (un-serialized) */
	serverSessionRef	mySession;

	*sc_status = kSCStatusOK;

	/* un-serialize the key */
	if (!_SCUnserializeString(&key, NULL, (void *)keyRef, keyLen)) {
		*sc_status = kSCStatusFailed;
	}

	/* un-serialize the data */
	if (!_SCUnserializeData(&data, (void *)dataRef, dataLen)) {
		*sc_status = kSCStatusFailed;
	}

	if (*sc_status != kSCStatusOK) {
		goto done;
	}

	if (!isA_CFString(key)) {
		*sc_status = kSCStatusInvalidArgument;
		goto done;
	}

	mySession = getSession(server);
	if (mySession == NULL) {
		mySession = tempSession(server, CFSTR("SCDynamicStoreSetValue"), audit_token);
		if (mySession == NULL) {
			/* you must have an open session to play */
			*sc_status = kSCStatusNoStoreSession;
			goto done;
		}
	}

	if (!hasWriteAccess(mySession, key)) {
		*sc_status = kSCStatusAccessError;
		goto done;
	}

	*sc_status = __SCDynamicStoreSetValue(mySession->store, key, data, FALSE);
	*newInstance = 0;

    done :

	if (key)	CFRelease(key);
	if (data)	CFRelease(data);
	return KERN_SUCCESS;
}

static void
setSpecificKey(const void *key, const void *value, void *context)
{
	CFStringRef		k	= (CFStringRef)key;
	CFDataRef		v	= (CFDataRef)value;
	SCDynamicStoreRef	store	= (SCDynamicStoreRef)context;

	if (!isA_CFString(k)) {
		return;
	}

	if (!isA_CFData(v)) {
		return;
	}

	(void) __SCDynamicStoreSetValue(store, k, v, TRUE);

	return;
}

static void
removeSpecificKey(const void *value, void *context)
{
	CFStringRef		k	= (CFStringRef)value;
	SCDynamicStoreRef	store	= (SCDynamicStoreRef)context;

	if (!isA_CFString(k)) {
		return;
	}

	(void) __SCDynamicStoreRemoveValue(store, k, TRUE);

	return;
}

static void
notifySpecificKey(const void *value, void *context)
{
	CFStringRef		k	= (CFStringRef)value;
	SCDynamicStoreRef	store	= (SCDynamicStoreRef)context;

	if (!isA_CFString(k)) {
		return;
	}

	(void) __SCDynamicStoreNotifyValue(store, k, TRUE);

	return;
}

__private_extern__
int
__SCDynamicStoreSetMultiple(SCDynamicStoreRef store, CFDictionaryRef keysToSet, CFArrayRef keysToRemove, CFArrayRef keysToNotify)
{
	int				sc_status	= kSCStatusOK;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	if (_configd_trace) {
		SCTrace(TRUE, _configd_trace,
			CFSTR("set m   : %5d : %ld set, %ld remove, %ld notify\n"),
			storePrivate->server,
			keysToSet    ? CFDictionaryGetCount(keysToSet)    : 0,
			keysToRemove ? CFArrayGetCount     (keysToRemove) : 0,
			keysToNotify ? CFArrayGetCount     (keysToNotify) : 0);
	}

	/*
	 * Set the new/updated keys
	 */
	if (keysToSet) {
		CFDictionaryApplyFunction(keysToSet,
					  setSpecificKey,
					  (void *)store);
	}

	/*
	 * Remove the specified keys
	 */
	if (keysToRemove) {
		CFArrayApplyFunction(keysToRemove,
				     CFRangeMake(0, CFArrayGetCount(keysToRemove)),
				     removeSpecificKey,
				     (void *)store);
	}

	/*
	 * Notify the specified keys
	 */
	if (keysToNotify) {
		CFArrayApplyFunction(keysToNotify,
				     CFRangeMake(0, CFArrayGetCount(keysToNotify)),
				     notifySpecificKey,
				     (void *)store);
	}

	/* push changes */
	__SCDynamicStorePush();

	return sc_status;
}

#define	N_QUICK	32

__private_extern__
kern_return_t
_configset_m(mach_port_t		server,
	     xmlData_t			dictRef,
	     mach_msg_type_number_t	dictLen,
	     xmlData_t			removeRef,
	     mach_msg_type_number_t	removeLen,
	     xmlData_t			notifyRef,
	     mach_msg_type_number_t	notifyLen,
	     int			*sc_status,
	     audit_token_t		audit_token)
{
	CFDictionaryRef		dict		= NULL;		/* key/value dictionary (un-serialized) */
	serverSessionRef	mySession;
	CFArrayRef		notify		= NULL;		/* keys to notify (un-serialized) */
	CFArrayRef		remove		= NULL;		/* keys to remove (un-serialized) */

	*sc_status = kSCStatusOK;

	if ((dictRef != NULL) && (dictLen > 0)) {
		/* un-serialize the key/value pairs to set */
		if (!_SCUnserialize((CFPropertyListRef *)&dict, NULL, (void *)dictRef, dictLen)) {
			*sc_status = kSCStatusFailed;
		}
	}

	if ((removeRef != NULL) && (removeLen > 0)) {
		/* un-serialize the keys to remove */
		if (!_SCUnserialize((CFPropertyListRef *)&remove, NULL, (void *)removeRef, removeLen)) {
			*sc_status = kSCStatusFailed;
		}
	}

	if ((notifyRef != NULL) && (notifyLen > 0)) {
		/* un-serialize the keys to notify */
		if (!_SCUnserialize((CFPropertyListRef *)&notify, NULL, (void *)notifyRef, notifyLen)) {
			*sc_status = kSCStatusFailed;
		}
	}

	if (*sc_status != kSCStatusOK) {
		goto done;
	}

	if ((dict != NULL) && !isA_CFDictionary(dict)) {
		*sc_status = kSCStatusInvalidArgument;
		goto done;
	}

	if ((remove != NULL) && !isA_CFArray(remove)) {
		*sc_status = kSCStatusInvalidArgument;
		goto done;
	}

	if ((notify != NULL) && !isA_CFArray(notify)) {
		*sc_status = kSCStatusInvalidArgument;
		goto done;
	}

	mySession = getSession(server);
	if (mySession == NULL) {
		mySession = tempSession(server, CFSTR("SCDynamicStoreSetMultiple"), audit_token);
		if (mySession == NULL) {
			/* you must have an open session to play */
			*sc_status = kSCStatusNoStoreSession;
			goto done;
		}
	}

	if (dict != NULL) {
		const void *	keys_q[N_QUICK];
		const void **	keys	= keys_q;
		CFIndex		i;
		CFIndex		n;
		Boolean		writeOK	= TRUE;

		n = CFDictionaryGetCount(dict);
		if (n > (CFIndex)(sizeof(keys_q) / sizeof(CFStringRef))) {
			keys   = CFAllocatorAllocate(NULL, n * sizeof(CFStringRef), 0);
		}
		CFDictionaryGetKeysAndValues(dict, keys, NULL);
		for (i = 0; i < n; i++) {
			CFStringRef	key;

			key = (CFStringRef)keys[i];
			if (!hasWriteAccess(mySession, key)) {
				writeOK = FALSE;
				break;
			}
		}
		if (keys != keys_q) {
			CFAllocatorDeallocate(NULL, keys);
		}

		if (!writeOK) {
			*sc_status = kSCStatusAccessError;
			goto done;
		}
	}

	if (remove != NULL) {
		CFIndex	i;
		CFIndex	n	= CFArrayGetCount(remove);

		for (i = 0; i < n; i++) {
			CFStringRef	key;

			key = CFArrayGetValueAtIndex(remove, i);
			if (!hasWriteAccess(mySession, key)) {
				*sc_status = kSCStatusAccessError;
				goto done;
			}
		}
	}

	if (notify != NULL) {
		CFIndex	i;
		CFIndex	n	= CFArrayGetCount(notify);

		for (i = 0; i < n; i++) {
			CFStringRef	key;

			key = CFArrayGetValueAtIndex(notify, i);
			if (!hasWriteAccess(mySession, key)) {
				*sc_status = kSCStatusAccessError;
				goto done;
			}
		}
	}

	*sc_status = __SCDynamicStoreSetMultiple(mySession->store, dict, remove, notify);

    done :

	if (dict != NULL)	CFRelease(dict);
	if (remove != NULL)	CFRelease(remove);
	if (notify != NULL)	CFRelease(notify);

	return KERN_SUCCESS;
}

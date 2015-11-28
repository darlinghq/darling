/*
 * Copyright (c) 2000-2004, 2006, 2008, 2011, 2012 Apple Inc. All rights reserved.
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

__private_extern__
int
__SCDynamicStoreRemoveValue(SCDynamicStoreRef store, CFStringRef key, Boolean internal)
{
	CFDictionaryRef			dict;
	CFMutableDictionaryRef		newDict;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;
	int				sc_status	= kSCStatusOK;
	CFStringRef			sessionKey;

	if (_configd_trace) {
		SCTrace(TRUE, _configd_trace,
			CFSTR("%s : %5d : %@\n"),
			internal ? "*remove" : "remove ",
			storePrivate->server,
			key);
	}

	/*
	 * Ensure that this key exists.
	 */
	dict = CFDictionaryGetValue(storeData, key);
	if ((dict == NULL) || (CFDictionaryContainsKey(dict, kSCDData) == FALSE)) {
		/* key doesn't exist (or data never defined) */
		sc_status = kSCStatusNoKey;
		goto done;
	}
	newDict = CFDictionaryCreateMutableCopy(NULL, 0, dict);

	/*
	 * Mark this key as "changed". Any "watchers" will be
	 * notified as soon as the lock is released.
	 */
	CFSetAddValue(changedKeys, key);

	/*
	 * Add this key to a deferred cleanup list so that, after
	 * the change notifications are posted, any associated
	 * regex keys can be removed.
	 */
	CFSetAddValue(deferredRemovals, key);

	/*
	 * Check if this is a session key and, if so, add it
	 * to the (session) removal list
	 */
	sessionKey = CFDictionaryGetValue(newDict, kSCDSession);
	if (sessionKey) {
		CFStringRef	removedKey;

		/* We are no longer a session key! */
		CFDictionaryRemoveValue(newDict, kSCDSession);

		/* add this session key to the (session) removal list */
		removedKey = CFStringCreateWithFormat(NULL, 0, CFSTR("%@:%@"), sessionKey, key);
		CFSetAddValue(removedSessionKeys, removedKey);
		CFRelease(removedKey);
	}

	/*
	 * Remove data and update/remove the dictionary store entry.
	 */
	CFDictionaryRemoveValue(newDict, kSCDData);
	if (CFDictionaryGetCount(newDict) > 0) {
		/* this key is still being "watched" */
		CFDictionarySetValue(storeData, key, newDict);
	} else {
		/* no information left, remove the empty dictionary */
		CFDictionaryRemoveValue(storeData, key);
	}
	CFRelease(newDict);

	if (!internal) {
		/* push changes */
		__SCDynamicStorePush();
	}

    done:

	return sc_status;
}


__private_extern__
kern_return_t
_configremove(mach_port_t		server,
	      xmlData_t			keyRef,		/* raw XML bytes */
	      mach_msg_type_number_t	keyLen,
	      int			*sc_status,
	      audit_token_t		audit_token)
{
	CFStringRef		key		= NULL;		/* key  (un-serialized) */
	serverSessionRef	mySession;

	/* un-serialize the key */
	if (!_SCUnserializeString(&key, NULL, (void *)keyRef, keyLen)) {
		*sc_status = kSCStatusFailed;
		goto done;
	}

	if (!isA_CFString(key)) {
		*sc_status = kSCStatusInvalidArgument;
		goto done;
	}

	mySession = getSession(server);
	if (mySession == NULL) {
		mySession = tempSession(server, CFSTR("SCDynamicStoreRemoveValue"), audit_token);
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

	*sc_status = __SCDynamicStoreRemoveValue(mySession->store, key, FALSE);

    done :

	if (key)	CFRelease(key);

	return KERN_SUCCESS;
}


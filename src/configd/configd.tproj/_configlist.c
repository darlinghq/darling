/*
 * Copyright (c) 2000-2008, 2011, 2013 Apple Inc. All rights reserved.
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

#define	N_QUICK	64

__private_extern__
int
__SCDynamicStoreCopyKeyList(SCDynamicStoreRef store, CFStringRef key, Boolean isRegex, CFArrayRef *subKeys)
{
	CFMutableArrayRef		keyArray;
	CFIndex				storeCnt;
	CFStringRef			storeStr;
	CFDictionaryRef			storeValue;

	if (isRegex) {
		*subKeys = patternCopyMatches(key);
		return (*subKeys != NULL) ? kSCStatusOK : kSCStatusFailed;
	}

	storeCnt = CFDictionaryGetCount(storeData);
	keyArray = CFArrayCreateMutable(NULL, storeCnt, &kCFTypeArrayCallBacks);
	if (storeCnt > 0) {
		int		i;
		const void *	storeKeys_q[N_QUICK];
		const void **	storeKeys	= storeKeys_q;
		const void *	storeValues_q[N_QUICK];
		const void **	storeValues	= storeValues_q;

		if (storeCnt > (CFIndex)(sizeof(storeKeys_q) / sizeof(CFStringRef))) {
			storeKeys   = CFAllocatorAllocate(NULL, storeCnt * sizeof(CFStringRef), 0);
			storeValues = CFAllocatorAllocate(NULL, storeCnt * sizeof(CFStringRef), 0);
		}

		CFDictionaryGetKeysAndValues(storeData, storeKeys, storeValues);
		for (i = 0; i < storeCnt; i++) {
			storeStr   = (CFStringRef)storeKeys[i];
			storeValue = (CFDictionaryRef)storeValues[i];
			/*
			 * only return those keys which are prefixed by the
			 * provided key string and have data.
			 */
			if (((CFStringGetLength(key) == 0) || CFStringHasPrefix(storeStr, key)) &&
			    CFDictionaryContainsKey(storeValue, kSCDData)) {
				CFArrayAppendValue(keyArray, storeStr);
			}
		}

		if (storeKeys != storeKeys_q) {
			CFAllocatorDeallocate(NULL, storeKeys);
			CFAllocatorDeallocate(NULL, storeValues);
		}
	}

	*subKeys = CFArrayCreateCopy(NULL, keyArray);
	CFRelease(keyArray);
	return kSCStatusOK;
}


__private_extern__
kern_return_t
_configlist(mach_port_t			server,
	    xmlData_t			keyRef,		/* raw XML bytes */
	    mach_msg_type_number_t	keyLen,
	    int				isRegex,
	    xmlDataOut_t		*listRef,	/* raw XML bytes */
	    mach_msg_type_number_t	*listLen,
	    int				*sc_status,
	    audit_token_t		audit_token)
{
	CFStringRef		key		= NULL;		/* key  (un-serialized) */
	CFIndex			len;
	serverSessionRef	mySession;
	Boolean			ok;
	CFArrayRef		subKeys;			/* array of CFStringRef's */

	*listRef = NULL;
	*listLen = 0;

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
		mySession = tempSession(server, CFSTR("SCDynamicStoreCopyKeyList"), audit_token);
		if (mySession == NULL) {
			/* you must have an open session to play */
			*sc_status = kSCStatusNoStoreSession;
			goto done;
		}
	}

	*sc_status = __SCDynamicStoreCopyKeyList(mySession->store, key, isRegex != 0, &subKeys);
	if (*sc_status != kSCStatusOK) {
		goto done;
	}

	/* serialize the list of keys */
	ok = _SCSerialize(subKeys, NULL, (void **)listRef, &len);
	*listLen = (mach_msg_type_number_t)len;
	CFRelease(subKeys);
	if (!ok) {
		*sc_status = kSCStatusFailed;
		goto done;
	}

    done :

	if (key)	CFRelease(key);
	return KERN_SUCCESS;
}

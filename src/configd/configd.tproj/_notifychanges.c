/*
 * Copyright (c) 2000-2004, 2006, 2011, 2013 Apple Inc. All rights reserved.
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
__SCDynamicStoreCopyNotifiedKeys(SCDynamicStoreRef store, CFArrayRef *notifierKeys)
{
	SCDynamicStorePrivateRef	storePrivate = (SCDynamicStorePrivateRef)store;
	CFStringRef			sessionKey;
	CFDictionaryRef			info;
	CFMutableDictionaryRef		newInfo;

	sessionKey = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), storePrivate->server);
	info = CFDictionaryGetValue(sessionData, sessionKey);
	if ((info == NULL) ||
	    (CFDictionaryContainsKey(info, kSCDChangedKeys) == FALSE)) {
		CFRelease(sessionKey);
		*notifierKeys = CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
		return kSCStatusOK;
	}
	newInfo = CFDictionaryCreateMutableCopy(NULL, 0, info);

	*notifierKeys = CFDictionaryGetValue(newInfo, kSCDChangedKeys);
	CFRetain(*notifierKeys);

	CFDictionaryRemoveValue(newInfo, kSCDChangedKeys);
	if (CFDictionaryGetCount(newInfo) > 0) {
		CFDictionarySetValue(sessionData, sessionKey, newInfo);
	} else {
		CFDictionaryRemoveValue(sessionData, sessionKey);
	}
	CFRelease(newInfo);
	CFRelease(sessionKey);

	return kSCStatusOK;
}


__private_extern__
kern_return_t
_notifychanges(mach_port_t			server,
	       xmlDataOut_t			*listRef,	/* raw XML bytes */
	       mach_msg_type_number_t		*listLen,
	       int				*sc_status
)
{
	CFIndex			len;
	serverSessionRef	mySession = getSession(server);
	CFArrayRef		notifierKeys;	/* array of CFStringRef's */
	Boolean			ok;

	*listRef = NULL;
	*listLen = 0;

	if (mySession == NULL) {
		*sc_status = kSCStatusNoStoreSession;	/* you must have an open session to play */
		return KERN_SUCCESS;
	}

	*sc_status = __SCDynamicStoreCopyNotifiedKeys(mySession->store, &notifierKeys);
	if (*sc_status != kSCStatusOK) {
		return KERN_SUCCESS;
	}

	/* serialize the array of keys */
	ok = _SCSerialize(notifierKeys, NULL, (void **)listRef, &len);
	*listLen = (mach_msg_type_number_t)len;
	CFRelease(notifierKeys);
	if (!ok) {
		*sc_status = kSCStatusFailed;
		return KERN_SUCCESS;
	}

	return KERN_SUCCESS;
}

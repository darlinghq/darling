/*
 * Copyright (c) 2000, 2001, 2003-2005, 2009-2011, 2013 Apple Inc. All rights reserved.
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
 * - initial revision
 */

#include <mach/mach.h>
#include <mach/mach_error.h>

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include "SCDynamicStoreInternal.h"
#include "config.h"		/* MiG generated file */


Boolean
SCDynamicStoreSetNotificationKeys(SCDynamicStoreRef	store,
				  CFArrayRef		keys,
				  CFArrayRef		patterns)
{
	SCDynamicStorePrivateRef	storePrivate = (SCDynamicStorePrivateRef)store;
	kern_return_t			status;
	CFDataRef			xmlKeys		= NULL;	/* keys (XML serialized) */
	xmlData_t			myKeysRef	= NULL;	/* keys (serialized) */
	CFIndex				myKeysLen	= 0;
	CFDataRef			xmlPatterns	= NULL;	/* patterns (XML serialized) */
	xmlData_t			myPatternsRef	= NULL;	/* patterns (serialized) */
	CFIndex				myPatternsLen	= 0;
	int				sc_status;
	CFMutableArrayRef		tmp;

	if (store == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoStoreSession);
		return FALSE;
	}

	if (storePrivate->server == MACH_PORT_NULL) {
		_SCErrorSet(kSCStatusNoStoreServer);
		return FALSE;	/* you must have an open session to play */
	}

	/* serialize the keys */
	if (keys != NULL) {
		if (!_SCSerialize(keys, &xmlKeys, (void **)&myKeysRef, &myKeysLen)) {
			_SCErrorSet(kSCStatusFailed);
			return FALSE;
		}
	}

	/* serialize the patterns */
	if (patterns != NULL) {
		if (!_SCSerialize(patterns, &xmlPatterns, (void **)&myPatternsRef, &myPatternsLen)) {
			if (xmlKeys != NULL) CFRelease(xmlKeys);
			_SCErrorSet(kSCStatusFailed);
			return FALSE;
		}
	}

    retry :

	/* send the keys and patterns, fetch the associated result from the server */
	status = notifyset(storePrivate->server,
			   myKeysRef,
			   (mach_msg_type_number_t)myKeysLen,
			   myPatternsRef,
			   (mach_msg_type_number_t)myPatternsLen,
			   (int *)&sc_status);

	if (__SCDynamicStoreCheckRetryAndHandleError(store,
						     status,
						     &sc_status,
						     "SCDynamicStoreSetNotificationKeys notifyset()")) {
		goto retry;
	}

	/* clean up */
	if (xmlKeys != NULL)		CFRelease(xmlKeys);
	if (xmlPatterns != NULL)	CFRelease(xmlPatterns);

	if (sc_status != kSCStatusOK) {
		_SCErrorSet(sc_status);
		return FALSE;
	}

	/* in case we need to re-connect, save the keys/patterns */
	tmp = (keys != NULL) ? CFArrayCreateMutableCopy(NULL, 0, keys) : NULL;
	if (storePrivate->keys != NULL) CFRelease(storePrivate->keys);
	storePrivate->keys = tmp;

	tmp = (patterns != NULL) ? CFArrayCreateMutableCopy(NULL, 0, patterns) : NULL;
	if (storePrivate->patterns != NULL) CFRelease(storePrivate->patterns);
	storePrivate->patterns = tmp;

	return TRUE;
}

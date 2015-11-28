/*
 * Copyright (c) 2000-2006, 2009-2011, 2013 Apple Inc. All rights reserved.
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

#include <mach/mach.h>
#include <mach/mach_error.h>

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include "SCDynamicStoreInternal.h"
#include "config.h"		/* MiG generated file */


Boolean
SCDynamicStoreSetMultiple(SCDynamicStoreRef	store,
			  CFDictionaryRef	keysToSet,
			  CFArrayRef		keysToRemove,
			  CFArrayRef		keysToNotify)
{
	SCDynamicStorePrivateRef	storePrivate;
	kern_return_t			status;
	CFDataRef			xmlSet		= NULL;	/* key/value pairs to set (XML serialized) */
	xmlData_t			mySetRef	= NULL;	/* key/value pairs to set (serialized) */
	CFIndex				mySetLen	= 0;
	CFDataRef			xmlRemove	= NULL;	/* keys to remove (XML serialized) */
	xmlData_t			myRemoveRef	= NULL;	/* keys to remove (serialized) */
	CFIndex				myRemoveLen	= 0;
	CFDataRef			xmlNotify	= NULL;	/* keys to notify (XML serialized) */
	xmlData_t			myNotifyRef	= NULL;	/* keys to notify (serialized) */
	CFIndex				myNotifyLen	= 0;
	int				sc_status;

	if (store == NULL) {
		store = __SCDynamicStoreNullSession();
		if (store == NULL) {
			/* sorry, you must provide a session */
			_SCErrorSet(kSCStatusNoStoreSession);
			return FALSE;
		}
	}

	storePrivate = (SCDynamicStorePrivateRef)store;
	if (storePrivate->server == MACH_PORT_NULL) {
		_SCErrorSet(kSCStatusNoStoreServer);
		return FALSE;	/* you must have an open session to play */
	}

	/* serialize the key/value pairs to set*/
	if (keysToSet != NULL) {
		CFDictionaryRef	newInfo;
		Boolean		ok;

		newInfo = _SCSerializeMultiple(keysToSet);
		if (newInfo == NULL) {
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
		}

		ok = _SCSerialize(newInfo, &xmlSet, (void **)&mySetRef, &mySetLen);
		CFRelease(newInfo);
		if (!ok) {
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
		}
	}

	/* serialize the keys to remove */
	if (keysToRemove != NULL) {
		if (!_SCSerialize(keysToRemove, &xmlRemove, (void **)&myRemoveRef, &myRemoveLen)) {
			if (xmlSet != NULL)	CFRelease(xmlSet);
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
		}
	}

	/* serialize the keys to notify */
	if (keysToNotify != NULL) {
		if (!_SCSerialize(keysToNotify, &xmlNotify, (void **)&myNotifyRef, &myNotifyLen)) {
			if (xmlSet != NULL)	CFRelease(xmlSet);
			if (xmlRemove != NULL)	CFRelease(xmlRemove);
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
		}
	}

    retry :

	/* send the keys and patterns, fetch the associated result from the server */
	status = configset_m(storePrivate->server,
			     mySetRef,
			     (mach_msg_type_number_t)mySetLen,
			     myRemoveRef,
			     (mach_msg_type_number_t)myRemoveLen,
			     myNotifyRef,
			     (mach_msg_type_number_t)myNotifyLen,
			     (int *)&sc_status);

	if (__SCDynamicStoreCheckRetryAndHandleError(store,
						     status,
						     &sc_status,
						     "SCDynamicStoreSetMultiple configset_m()")) {
		goto retry;
	}

	/* clean up */
	if (xmlSet != NULL)	CFRelease(xmlSet);
	if (xmlRemove != NULL)	CFRelease(xmlRemove);
	if (xmlNotify != NULL)	CFRelease(xmlNotify);

	if (sc_status != kSCStatusOK) {
		_SCErrorSet(sc_status);
		return FALSE;
	}

	return TRUE;
}

Boolean
SCDynamicStoreSetValue(SCDynamicStoreRef store, CFStringRef key, CFPropertyListRef value)
{
	SCDynamicStorePrivateRef	storePrivate;
	kern_return_t			status;
	CFDataRef			utfKey;		/* serialized key */
	xmlData_t			myKeyRef;
	CFIndex				myKeyLen;
	CFDataRef			xmlData;	/* serialized data */
	xmlData_t			myDataRef;
	CFIndex				myDataLen;
	int				sc_status;
	int				newInstance;

	if (store == NULL) {
		store = __SCDynamicStoreNullSession();
		if (store == NULL) {
			/* sorry, you must provide a session */
			_SCErrorSet(kSCStatusNoStoreSession);
			return FALSE;
		}
	}

	storePrivate = (SCDynamicStorePrivateRef)store;
	if (storePrivate->server == MACH_PORT_NULL) {
		/* sorry, you must have an open session to play */
		_SCErrorSet(kSCStatusNoStoreServer);
		return FALSE;
	}

	/* serialize the key */
	if (!_SCSerializeString(key, &utfKey, (void **)&myKeyRef, &myKeyLen)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	/* serialize the data */
	if (!_SCSerialize(value, &xmlData, (void **)&myDataRef, &myDataLen)) {
		CFRelease(utfKey);
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

    retry :

	/* send the key & data to the server, get new instance id */
	status = configset(storePrivate->server,
			   myKeyRef,
			   (mach_msg_type_number_t)myKeyLen,
			   myDataRef,
			   (mach_msg_type_number_t)myDataLen,
			   0,
			   &newInstance,
			   (int *)&sc_status);

	if (__SCDynamicStoreCheckRetryAndHandleError(store,
						     status,
						     &sc_status,
						     "SCDynamicStoreSetValue configset()")) {
		goto retry;
	}

	/* clean up */
	CFRelease(utfKey);
	CFRelease(xmlData);

	if (sc_status != kSCStatusOK) {
		_SCErrorSet(sc_status);
		return FALSE;
	}

	return TRUE;
}

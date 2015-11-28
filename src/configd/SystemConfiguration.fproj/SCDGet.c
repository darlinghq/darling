/*
 * Copyright (c) 2000-2005, 2009-2011, 2013 Apple Inc. All rights reserved.
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


CFDictionaryRef
SCDynamicStoreCopyMultiple(SCDynamicStoreRef	store,
			   CFArrayRef		keys,
			   CFArrayRef		patterns)
{
	SCDynamicStorePrivateRef	storePrivate;
	kern_return_t			status;
	CFDataRef			xmlKeys		= NULL;	/* keys (XML serialized) */
	xmlData_t			myKeysRef	= NULL;	/* keys (serialized) */
	CFIndex				myKeysLen	= 0;
	CFDataRef			xmlPatterns	= NULL;	/* patterns (XML serialized) */
	xmlData_t			myPatternsRef	= NULL;	/* patterns (serialized) */
	CFIndex				myPatternsLen	= 0;
	xmlDataOut_t			xmlDictRef	= NULL;	/* dict (serialized) */
	mach_msg_type_number_t		xmlDictLen	= 0;
	CFDictionaryRef			dict		= NULL;	/* dict (un-serialized) */
	CFDictionaryRef			expDict		= NULL;	/* dict (un-serialized / expanded) */
	int				sc_status;

	if (store == NULL) {
		store = __SCDynamicStoreNullSession();
		if (store == NULL) {
			/* sorry, you must provide a session */
			_SCErrorSet(kSCStatusNoStoreSession);
			return NULL;
		}
	}

	storePrivate = (SCDynamicStorePrivateRef)store;
	if (storePrivate->server == MACH_PORT_NULL) {
		_SCErrorSet(kSCStatusNoStoreServer);
		return NULL;	/* you must have an open session to play */
	}

	/* serialize the keys */
	if (keys != NULL) {
		if (!_SCSerialize(keys, &xmlKeys, (void **)&myKeysRef, &myKeysLen)) {
			_SCErrorSet(kSCStatusFailed);
			return NULL;
		}
	}

	/* serialize the patterns */
	if (patterns != NULL) {
		if (!_SCSerialize(patterns, &xmlPatterns, (void **)&myPatternsRef, &myPatternsLen)) {
			if (xmlKeys != NULL) CFRelease(xmlKeys);
			_SCErrorSet(kSCStatusFailed);
			return NULL;
		}
	}

    retry :

	/* send the keys and patterns, fetch the associated result from the server */
	status = configget_m(storePrivate->server,
			     myKeysRef,
			     (mach_msg_type_number_t)myKeysLen,
			     myPatternsRef,
			     (mach_msg_type_number_t)myPatternsLen,
			     &xmlDictRef,
			     &xmlDictLen,
			     (int *)&sc_status);

	if (__SCDynamicStoreCheckRetryAndHandleError(store,
						     status,
						     &sc_status,
						     "SCDynamicStoreCopyMultiple configget_m()")) {
		goto retry;
	}

	/* clean up */
	if (xmlKeys != NULL)		CFRelease(xmlKeys);
	if (xmlPatterns != NULL)	CFRelease(xmlPatterns);

	if (sc_status != kSCStatusOK) {
		if (xmlDictRef != NULL) {
			(void) vm_deallocate(mach_task_self(), (vm_address_t)xmlDictRef, xmlDictLen);
		}
		_SCErrorSet(sc_status);
		return NULL;
	}

	/* un-serialize the dictionary */
	if (!_SCUnserialize((CFPropertyListRef *)&dict, NULL, xmlDictRef, xmlDictLen)) {
		_SCErrorSet(kSCStatusFailed);
		return NULL;
	}

	expDict = _SCUnserializeMultiple(dict);
	CFRelease(dict);

	return expDict;
}


CFPropertyListRef
SCDynamicStoreCopyValue(SCDynamicStoreRef store, CFStringRef key)
{
	SCDynamicStorePrivateRef	storePrivate;
	kern_return_t			status;
	CFDataRef			utfKey;			/* key (XML serialized) */
	xmlData_t			myKeyRef;		/* key (serialized) */
	CFIndex				myKeyLen;
	xmlDataOut_t			xmlDataRef	= NULL;	/* data (serialized) */
	mach_msg_type_number_t		xmlDataLen	= 0;
	CFPropertyListRef		data;			/* data (un-serialized) */
	int				newInstance;
	int				sc_status;

	if (store == NULL) {
		store = __SCDynamicStoreNullSession();
		if (store == NULL) {
			/* sorry, you must provide a session */
			_SCErrorSet(kSCStatusNoStoreSession);
			return NULL;
		}
	}

	storePrivate = (SCDynamicStorePrivateRef)store;
	if (storePrivate->server == MACH_PORT_NULL) {
		_SCErrorSet(kSCStatusNoStoreServer);
		return NULL;	/* you must have an open session to play */
	}

	/* serialize the key */
	if (!_SCSerializeString(key, &utfKey, (void **)&myKeyRef, &myKeyLen)) {
		_SCErrorSet(kSCStatusFailed);
		return NULL;
	}

    retry :

	/* send the key & fetch the associated data from the server */
	status = configget(storePrivate->server,
			   myKeyRef,
			   (mach_msg_type_number_t)myKeyLen,
			   &xmlDataRef,
			   &xmlDataLen,
			   &newInstance,
			   (int *)&sc_status);

	if (__SCDynamicStoreCheckRetryAndHandleError(store,
						     status,
						     &sc_status,
						     "SCDynamicStoreCopyValue configget()")) {
		goto retry;
	}

	/* clean up */
	CFRelease(utfKey);

	if (sc_status != kSCStatusOK) {
		if (xmlDataRef != NULL) {
			(void) vm_deallocate(mach_task_self(), (vm_address_t)xmlDataRef, xmlDataLen);
		}
		_SCErrorSet(sc_status);
		return NULL;
	}

	/* un-serialize the data */
	if (!_SCUnserialize(&data, NULL, xmlDataRef, xmlDataLen)) {
		_SCErrorSet(kSCStatusFailed);
		return NULL;
	}

	return data;
}

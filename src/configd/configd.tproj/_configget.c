/*
 * Copyright (c) 2000-2006, 2008, 2011, 2013, 2014 Apple Inc. All rights reserved.
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
__SCDynamicStoreCopyValue(SCDynamicStoreRef store, CFStringRef key, CFDataRef *value, Boolean internal)
{
	SCDynamicStorePrivateRef	storePrivate = (SCDynamicStorePrivateRef)store;
	CFDictionaryRef			dict;

	if (_configd_trace) {
		SCTrace(TRUE, _configd_trace,
			CFSTR("%s : %5d : %@\n"),
			internal ? "*copy  " : "copy   ",
			storePrivate->server,
			key);
	}

	dict = CFDictionaryGetValue(storeData, key);
	if ((dict == NULL) || (CFDictionaryContainsKey(dict, kSCDData) == FALSE)) {
		/* key doesn't exist (or data never defined) */
		return kSCStatusNoKey;
	}

	/* Return the data associated with the key */
	*value = CFRetain(CFDictionaryGetValue(dict, kSCDData));

	return kSCStatusOK;
}

__private_extern__
kern_return_t
_configget(mach_port_t			server,
	   xmlData_t			keyRef,		/* raw XML bytes */
	   mach_msg_type_number_t	keyLen,
	   xmlDataOut_t			*dataRef,	/* raw XML bytes */
	   mach_msg_type_number_t	*dataLen,
	   int				*newInstance,
	   int				*sc_status,
	   audit_token_t		audit_token)
{
	CFStringRef		key		= NULL;		/* key  (un-serialized) */
	CFIndex			len;
	serverSessionRef	mySession;
	Boolean			ok;
	CFDataRef		value;

	*dataRef = NULL;
	*dataLen = 0;

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
		mySession = tempSession(server, CFSTR("SCDynamicStoreCopyValue"), audit_token);
		if (mySession == NULL) {
			/* you must have an open session to play */
			*sc_status = kSCStatusNoStoreSession;
			goto done;
		}
	}

	*sc_status = __SCDynamicStoreCopyValue(mySession->store, key, &value, FALSE);
	if (*sc_status != kSCStatusOK) {
		goto done;
	}

	/* serialize the data */
	ok = _SCSerializeData(value, (void **)dataRef, &len);
	*dataLen = (mach_msg_type_number_t)len;
	CFRelease(value);
	if (!ok) {
		*sc_status = kSCStatusFailed;
		goto done;
	}

	/*
	 * return the instance number associated with the returned data.
	 */
	*newInstance = 1;

    done :

	if (key != NULL)	CFRelease(key);
	return KERN_SUCCESS;
}

/*
 * "context" argument for addSpecificKey() and addSpecificPattern()
 */
typedef struct {
	SCDynamicStoreRef	store;
	CFMutableDictionaryRef	dict;
} addSpecific, *addSpecificRef;

static void
addSpecificKey(const void *value, void *context)
{
	CFDataRef		data;
	CFStringRef		key		= (CFStringRef)value;
	addSpecificRef		myContextRef	= (addSpecificRef)context;
	int			sc_status;

	if (!isA_CFString(key)) {
		return;
	}

	sc_status = __SCDynamicStoreCopyValue(myContextRef->store, key, &data, TRUE);
	if (sc_status == kSCStatusOK) {
		CFDictionaryAddValue(myContextRef->dict, key, data);
		CFRelease(data);
	}

	return;
}

static void
addSpecificPattern(const void *value, void *context)
{
	CFStringRef		pattern		= (CFStringRef)value;
	addSpecificRef		myContextRef	= (addSpecificRef)context;
	int			sc_status;
	CFArrayRef		keys;

	if (!isA_CFString(pattern)) {
		return;
	}

	sc_status = __SCDynamicStoreCopyKeyList(myContextRef->store, pattern, TRUE, &keys);
	if (sc_status == kSCStatusOK) {
		CFArrayApplyFunction(keys,
				     CFRangeMake(0, CFArrayGetCount(keys)),
				     addSpecificKey,
				     context);
		CFRelease(keys);
	}

	return;
}

__private_extern__
int
__SCDynamicStoreCopyMultiple(SCDynamicStoreRef store, CFArrayRef keys, CFArrayRef patterns, CFDictionaryRef *values)
{
	SCDynamicStorePrivateRef	storePrivate = (SCDynamicStorePrivateRef)store;
	addSpecific			myContext;

	if (_configd_trace) {
		SCTrace(TRUE, _configd_trace,
			CFSTR("copy m  : %5d : %ld keys, %ld patterns\n"),
			storePrivate->server,
			keys     ? CFArrayGetCount(keys)     : 0,
			patterns ? CFArrayGetCount(patterns) : 0);
	}

	myContext.store = store;
	myContext.dict  = CFDictionaryCreateMutable(NULL,
						    0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);

	if (keys) {
		CFArrayApplyFunction(keys,
				     CFRangeMake(0, CFArrayGetCount(keys)),
				     addSpecificKey,
				     &myContext);
	}

	if (patterns) {
		CFArrayApplyFunction(patterns,
				     CFRangeMake(0, CFArrayGetCount(patterns)),
				     addSpecificPattern,
				     &myContext);
	}

	/* Return the keys/values associated with the key */
	*values = myContext.dict;

	return kSCStatusOK;
}

__private_extern__
kern_return_t
_configget_m(mach_port_t		server,
	     xmlData_t			keysRef,
	     mach_msg_type_number_t	keysLen,
	     xmlData_t			patternsRef,
	     mach_msg_type_number_t	patternsLen,
	     xmlDataOut_t		*dataRef,
	     mach_msg_type_number_t	*dataLen,
	     int			*sc_status,
	     audit_token_t		audit_token)
{
	CFDictionaryRef		dict		= NULL;	/* keys/values (un-serialized) */
	CFArrayRef		keys		= NULL;	/* keys (un-serialized) */
	CFIndex			len;
	serverSessionRef	mySession;
	Boolean			ok;
	CFArrayRef		patterns	= NULL;	/* patterns (un-serialized) */

	*dataRef = NULL;
	*dataLen = 0;

	*sc_status = kSCStatusOK;

	if (keysRef && (keysLen > 0)) {
		/* un-serialize the keys */
		if (!_SCUnserialize((CFPropertyListRef *)&keys, NULL, (void *)keysRef, keysLen)) {
			*sc_status = kSCStatusFailed;
		}
	}

	if (patternsRef && (patternsLen > 0)) {
		/* un-serialize the patterns */
		if (!_SCUnserialize((CFPropertyListRef *)&patterns, NULL, (void *)patternsRef, patternsLen)) {
			*sc_status = kSCStatusFailed;
		}
	}

	if (*sc_status != kSCStatusOK) {
		goto done;
	}

	if ((keys != NULL) && !isA_CFArray(keys)) {
		*sc_status = kSCStatusInvalidArgument;
		goto done;
	}

	if ((patterns != NULL) && !isA_CFArray(patterns)) {
		*sc_status = kSCStatusInvalidArgument;
		goto done;
	}

	mySession = getSession(server);
	if (mySession == NULL) {
		mySession = tempSession(server, CFSTR("SCDynamicStoreCopyMultiple"), audit_token);
		if (mySession == NULL) {
			/* you must have an open session to play */
			*sc_status = kSCStatusNoStoreSession;
			goto done;
		}
	}

	/* fetch the requested information */
	*sc_status = __SCDynamicStoreCopyMultiple(mySession->store, keys, patterns, &dict);

	/* serialize the dictionary of matching keys/patterns */
	ok = _SCSerialize(dict, NULL, (void **)dataRef, &len);
	*dataLen = (mach_msg_type_number_t)len;
	CFRelease(dict);
	if (!ok) {
		*sc_status = kSCStatusFailed;
	}

    done :

	if (keys)	CFRelease(keys);
	if (patterns)	CFRelease(patterns);
	return KERN_SUCCESS;
}

/*
 * Copyright (c) 2003, 2004, 2006-2008, 2011, 2012 Apple Inc. All rights reserved.
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
 * April 23, 2003		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#include "configd.h"
#include "pattern.h"


/*
 * Notes:
 *
 * - pattern matching information is maintained in a dictionary (patternData).
 * - the dictionary "key" is the regular expression being matched
 * - the dictionary "value" is a CFArray with the following contents
 *     [0]   = CFData consisting of the pre-compiled regular expression
 *     [1]   = CFArray[CFNumber] consisting of the sessions watching this pattern
 *     [2-n] = dynamic store keys which match this pattern
 */


typedef struct {
	CFMutableArrayRef	pInfo;
	CFDataRef		pRegex;
} addContext, *addContextRef;


static __inline__ void
my_CFDictionaryApplyFunction(CFDictionaryRef			theDict,
			     CFDictionaryApplierFunction	applier,
			     void				*context)
{
	CFAllocatorRef	myAllocator;
	CFDictionaryRef	myDict;

	myAllocator = CFGetAllocator(theDict);
	myDict      = CFDictionaryCreateCopy(myAllocator, theDict);
	CFDictionaryApplyFunction(myDict, applier, context);
	CFRelease(myDict);
	return;
}


static Boolean
keyMatchesPattern(CFStringRef key, CFDataRef pRegex)
{
	CFIndex			len;
	Boolean			match		= FALSE;
	regex_t			*preg;
	int			reError;
	char			str_q[256];
	char *			str		= str_q;

	/* convert store key to C string */
	len = CFStringGetMaximumSizeForEncoding(CFStringGetLength(key), kCFStringEncodingASCII) + 1;
	if (len > (CFIndex)sizeof(str_q))
		str = CFAllocatorAllocate(NULL, len, 0);
	if (_SC_cfstring_to_cstring(key, str, len, kCFStringEncodingASCII) == NULL) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("keyMatchesPattern(): could not convert store key to C string"));
		goto done;
	}

	/* ALIGN: CF aligns to >8 byte boundries */
	preg = (regex_t *)(void *)CFDataGetBytePtr(pRegex);

	/* compare key to regular expression pattern */
	reError = regexec(preg, str, 0, NULL, 0);
	switch (reError) {
		case 0 :
			match = TRUE;
			break;
		case REG_NOMATCH :
			/* no match */
			break;
		default : {
			char	reErrBuf[256];

			(void)regerror(reError, preg, reErrBuf, sizeof(reErrBuf));
			SCLog(TRUE, LOG_DEBUG, CFSTR("keyMatchesPattern regexec(): %s"), reErrBuf);
			break;
		}
	}

    done :

	if (str != str_q) CFAllocatorDeallocate(NULL, str);
	return match;
}


static void
identifyKeyForPattern(const void *key, void *val, void *context)
{
	CFStringRef		storeKey	= (CFStringRef)key;
	CFDictionaryRef		storeValue	= (CFDictionaryRef)val;
	CFMutableArrayRef	pInfo		= ((addContextRef)context)->pInfo;
	CFDataRef		pRegex		= ((addContextRef)context)->pRegex;

	if (!CFDictionaryContainsKey(storeValue, kSCDData)) {
		/* if no data (yet) */
		return;
	}

	if (keyMatchesPattern(storeKey, pRegex)) {
		/* if we've got a match */
		CFArrayAppendValue(pInfo, storeKey);
	}

	return;
}


static Boolean
patternCompile(CFStringRef pattern, CFDataRef pRegex, CFStringRef *error)
{
	Boolean		append		= FALSE;
	Boolean		insert		= FALSE;
	CFIndex		len		= 0;
	CFIndex		len_c;
	Boolean		ok;
	char		str_q[256];
	char *		str		= str_q;

	if (CFStringGetLength(pattern) == 0) {
		SCLog(TRUE, LOG_ERR, CFSTR("patternCompile(): empty string"));
	}

	if (!CFStringHasPrefix(pattern, CFSTR("^"))) {
		insert = TRUE;
	}

	if (!CFStringHasSuffix(pattern, CFSTR("$")) ||
	     CFStringHasSuffix(pattern, CFSTR("\\$"))) {
		append = TRUE;
	}

	/* if regex pattern is not bounded at both ends */
	if (insert || append) {
	      pattern = CFStringCreateWithFormat(NULL,
						 NULL,
						 CFSTR("%s%@%s"),
						 insert ? "^" : "",
						 pattern,
						 append ? "$" : "");
	}

	len_c = CFStringGetBytes(pattern,
				 CFRangeMake(0, CFStringGetLength(pattern)),
				 kCFStringEncodingASCII,
				 0,
				 FALSE,
				 NULL,
				 0,
				 &len);
	if (len_c <= 0) {
		SCLog(TRUE, LOG_ERR, CFSTR("patternCompile(): could not get buffer length for \"%@\""), pattern);
		len = sizeof(str_q) - 1;
	}
	if (++len > (CFIndex)sizeof(str_q)) {
		str = CFAllocatorAllocate(NULL, len, 0);
	}
	ok = (_SC_cfstring_to_cstring(pattern, str, len, kCFStringEncodingASCII) != NULL);
	if (insert || append) {
		CFRelease(pattern);
	}
	if (ok) {
		regex_t	*preg;
		int	reError;

		/* ALIGN: CF aligns to >8 byte boundries */
		preg = (regex_t *)(void *)CFDataGetBytePtr(pRegex);

		reError = regcomp(preg, str, REG_EXTENDED);
		if (reError != 0) {
			char	reErrBuf[256];

			(void)regerror(reError, preg, reErrBuf, sizeof(reErrBuf));
			*error = CFStringCreateWithCString(NULL, reErrBuf, kCFStringEncodingASCII);
#ifdef	DEBUG
			SCLog(_configd_verbose, LOG_DEBUG, CFSTR("patternCompile regcomp(%s) failed: %s"), str, reErrBuf);
#endif	/* DEBUG */
			ok = FALSE;
		}
	} else {
		*error = CFRetain(CFSTR("could not convert pattern to regex string"));
#ifdef	DEBUG
		SCLog(_configd_verbose, LOG_DEBUG, CFSTR("%@"), *error);
#endif	/* DEBUG */
	}

	if (str != str_q)	CFAllocatorDeallocate(NULL, str);
	return ok;
}


static void
patternRelease(CFDataRef pRegex)
{
	regex_t		*preg;

	/* ALIGN: CF aligns to >8 byte boundries */
	preg = (regex_t *)(void *)CFDataGetBytePtr(pRegex);
	regfree(preg);

	return;
}


static CF_RETURNS_RETAINED CFMutableArrayRef
patternCopy(CFStringRef	pattern)
{
	CFArrayRef	pInfo;

	pInfo = CFDictionaryGetValue(patternData, pattern);
	return (pInfo != NULL) ? CFArrayCreateMutableCopy(NULL, 0, pInfo) : NULL;
}


static CF_RETURNS_RETAINED CFMutableArrayRef
patternNew(CFStringRef pattern)
{
	addContext		context;
	CFStringRef		err	= NULL;
	CFMutableArrayRef	pInfo;
	CFMutableDataRef	pRegex;
	CFArrayRef		pSessions;

	/* create the pattern info */
	pInfo = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	/* compile the regular expression from the pattern string. */
	pRegex = CFDataCreateMutable(NULL, sizeof(regex_t));
	CFDataSetLength(pRegex, sizeof(regex_t));
	if (!patternCompile(pattern, pRegex, &err)) {
		CFRelease(err);
		CFRelease(pRegex);
		CFRelease(pInfo);
		return NULL;
	}

	/* add the compiled regular expression to the pattern info */
	CFArrayAppendValue(pInfo, pRegex);

	/* add the initial (empty) list of sessions watching this pattern */
	pSessions = CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
	CFArrayAppendValue(pInfo, pSessions);
	CFRelease(pSessions);

	/* identify/add all existing keys that match the specified pattern */
	context.pInfo  = pInfo;
	context.pRegex = pRegex;
	my_CFDictionaryApplyFunction(storeData,
				     (CFDictionaryApplierFunction)identifyKeyForPattern,
				     &context);

	CFRelease(pRegex);
	return pInfo;
}


__private_extern__
CFArrayRef
patternCopyMatches(CFStringRef pattern)
{
	Boolean			isNew	= FALSE;
	CFArrayRef		keys;
	CFMutableArrayRef	pInfo;

	/* find (or create new instance of) this pattern */
	pInfo = patternCopy(pattern);
	if (pInfo == NULL) {
		/* if new pattern */
		pInfo = patternNew(pattern);
		if (pInfo == NULL) {
			return NULL;
		}

		isNew = TRUE;
	}

	if (isNew) {
		CFDataRef	pRegex;

		pRegex = CFArrayGetValueAtIndex(pInfo, 0);
		patternRelease(pRegex);
	}

	CFArrayReplaceValues(pInfo, CFRangeMake(0, 2), NULL, 0);
	keys = CFArrayCreateCopy(NULL, pInfo);
	CFRelease(pInfo);

	return keys;
}


__private_extern__
Boolean
patternKeyMatches(CFStringRef pattern, CFStringRef key)
{
	Boolean			isNew	= FALSE;
	Boolean			match	= FALSE;
	CFMutableArrayRef	pInfo;
	CFDataRef		pRegex;

	/* find (or create new instance of) this pattern */
	pInfo = patternCopy(pattern);
	if (pInfo != NULL) {
		CFIndex		n;

		/* if existing pattern, check if known key */
		n = CFArrayGetCount(pInfo);
		match = (n > 2) &&
			CFArrayContainsValue(pInfo, CFRangeMake(2, n - 2), key);
		if (match) {
			goto done;
		}
	} else {
		/* if new pattern */
		pInfo = patternNew(pattern);
		if (pInfo == NULL) {
			return FALSE;
		}

		isNew = TRUE;
	}

	pRegex = CFArrayGetValueAtIndex(pInfo, 0);
	match = keyMatchesPattern(key, pRegex);

	if (isNew) {
		patternRelease(pRegex);
	}

    done :

	CFRelease(pInfo);

	return match;
}


__private_extern__
Boolean
patternAddSession(CFStringRef pattern, CFNumberRef sessionNum)
{
	CFIndex                 i;
	CFIndex                 n;
	CFMutableArrayRef       pInfo;
	CFMutableArrayRef	pSessions;

	/* find (or create new instance of) this pattern */
	pInfo = patternCopy(pattern);
	if (pInfo == NULL) {
		/* if new pattern */
		pInfo = patternNew(pattern);
		if (pInfo == NULL) {
			return FALSE;
		}
	}

	/* add this session as a pattern watcher */
	pSessions = (CFMutableArrayRef)CFArrayGetValueAtIndex(pInfo, 1);
	pSessions = CFArrayCreateMutableCopy(NULL, 0, pSessions);
	CFArrayAppendValue(pSessions, sessionNum);
	CFArraySetValueAtIndex(pInfo, 1, pSessions);
	CFRelease(pSessions);

	/* update pattern watcher info */
	CFDictionarySetValue(patternData, pattern, pInfo);

	/* add this session as a watcher of any existing keys */
	n = CFArrayGetCount(pInfo);
	for (i = 2; i < n; i++) {
		CFStringRef     matchingKey;

		matchingKey = CFArrayGetValueAtIndex(pInfo, i);
		_addWatcher(sessionNum, matchingKey);
	}

	CFRelease(pInfo);
	return TRUE;
}


__private_extern__
void
patternRemoveSession(CFStringRef pattern, CFNumberRef sessionNum)
{
	CFIndex                 i;
	CFIndex                 n;
	CFMutableArrayRef       pInfo;
	CFDataRef		pRegex;
	CFMutableArrayRef	pSessions;

	/* find instance of this pattern */
	pInfo = patternCopy(pattern);
	assert(pInfo != NULL);

	/* remove this session as a watcher from all matching keys */
	n = CFArrayGetCount(pInfo);
	for (i = 2; i < n; i++) {
		CFStringRef     matchingKey;

		matchingKey = CFArrayGetValueAtIndex(pInfo, i);
		_removeWatcher(sessionNum, matchingKey);
	}

	/* remove session from watchers */
	pSessions = (CFMutableArrayRef)CFArrayGetValueAtIndex(pInfo, 1);
	n = CFArrayGetCount(pSessions);
	if (n > 1) {
		/* if other sessions are watching this pattern */

		pSessions = CFArrayCreateMutableCopy(NULL, 0, pSessions);
		i = CFArrayGetFirstIndexOfValue(pSessions, CFRangeMake(0, n), sessionNum);
		CFArrayRemoveValueAtIndex(pSessions, i);
		CFArraySetValueAtIndex(pInfo, 1, pSessions);
		CFRelease(pSessions);

		CFDictionarySetValue(patternData, pattern, pInfo);
	} else {
		/* if no other sessions are watching this pattern */

		pRegex = CFArrayGetValueAtIndex(pInfo, 0);
		patternRelease(pRegex);
		CFDictionaryRemoveValue(patternData, pattern);
	}

	CFRelease(pInfo);
	return;
}


static void
addKeyForPattern(const void *key, void *val, void *context)
{
	CFStringRef		pattern		= (CFStringRef)key;
	CFArrayRef		pInfo		= (CFArrayRef)val;
	CFStringRef		storeKey	= (CFStringRef)context;

	CFIndex			len;
	regex_t			*preg;
	int			reError;
	char			str_q[256];
	char *			str		= str_q;

	/* convert store key to C string */
	len = CFStringGetMaximumSizeForEncoding(CFStringGetLength(storeKey), kCFStringEncodingASCII) + 1;
	if (len > (CFIndex)sizeof(str_q))
		str = CFAllocatorAllocate(NULL, len, 0);
	if (_SC_cfstring_to_cstring(storeKey, str, len, kCFStringEncodingASCII) == NULL) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("addKeyForPattern(): could not convert store key to C string"));
		goto done;
	}

	/* compare new store key to regular expression pattern */
	/* ALIGN: CF aligns to >8 byte boundries */
	preg = (regex_t *)(void *)CFDataGetBytePtr(CFArrayGetValueAtIndex(pInfo, 0));
	reError = regexec(preg, str, 0, NULL, 0);
	switch (reError) {
		case 0 : {
			/*
			 * we've got a match
			 */
			CFIndex			i;
			CFIndex			n;
			CFMutableArrayRef	pInfo_new;
			CFArrayRef		pSessions;

			/* add watchers */
			pSessions = CFArrayGetValueAtIndex(pInfo, 1);
			n = CFArrayGetCount(pSessions);
			for (i = 0; i < n; i++) {
				CFNumberRef	sessionNum	= CFArrayGetValueAtIndex(pSessions, i);

				_addWatcher(sessionNum, storeKey);
			}

			/* add key, update pattern watcher info */
			pInfo_new = CFArrayCreateMutableCopy(NULL, 0, pInfo);
			CFArrayAppendValue(pInfo_new, storeKey);
			CFDictionarySetValue(patternData, pattern, pInfo_new);
			CFRelease(pInfo_new);
			break;
		}
		case REG_NOMATCH :
			/* no match */
			break;
		default : {
			char	reErrBuf[256];

			(void)regerror(reError, preg, reErrBuf, sizeof(reErrBuf));
			SCLog(TRUE, LOG_DEBUG, CFSTR("addKeyForPattern regexec(): %s"), reErrBuf);
			break;
		}
	}

    done :

	if (str != str_q) CFAllocatorDeallocate(NULL, str);
	return;
}


__private_extern__
void
patternAddKey(CFStringRef key)
{
	void	*context	= (void *)key;

	my_CFDictionaryApplyFunction(patternData,
				     (CFDictionaryApplierFunction)addKeyForPattern,
				     context);

	return;
}


static void
removeKeyFromPattern(const void *key, void *val, void *context)
{
	CFStringRef		pattern		= (CFStringRef)key;
	CFArrayRef		pInfo		= (CFArrayRef)val;
	CFStringRef		storeKey	= (CFStringRef)context;

	CFIndex			i;
	CFIndex			n;
	CFMutableArrayRef	pInfo_new;
	CFArrayRef		pSessions;

	n = CFArrayGetCount(pInfo);
	if (n <= 2) {
		/* if no keys match this pattern */
		return;
	}

	i = CFArrayGetFirstIndexOfValue(pInfo, CFRangeMake(2, n-2), storeKey);
	if (i == kCFNotFound) {
		/* if this key wasn't matched by this pattern */
		return;
	}

	/* remove key from pattern info */
	pInfo_new = CFArrayCreateMutableCopy(NULL, 0, pInfo);
	CFArrayRemoveValueAtIndex(pInfo_new, i);

	/* remove watchers */
	pSessions = CFArrayGetValueAtIndex(pInfo_new, 1);
	n = CFArrayGetCount(pSessions);
	for (i = 0; i < n; i++) {
		CFNumberRef	sessionNum	= CFArrayGetValueAtIndex(pSessions, i);

		_removeWatcher(sessionNum, storeKey);
	}

	CFDictionarySetValue(patternData, pattern, pInfo_new);
	CFRelease(pInfo_new);
	return;
}


__private_extern__
void
patternRemoveKey(CFStringRef key)
{
	void	*context	= (void *)key;

	my_CFDictionaryApplyFunction(patternData,
				     (CFDictionaryApplierFunction)removeKeyFromPattern,
				     context);

	return;
}

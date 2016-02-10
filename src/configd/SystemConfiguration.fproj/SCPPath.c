/*
 * Copyright (c) 2000-2006, 2008, 2011, 2012 Apple Inc. All rights reserved.
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
 * November 16, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCValidation.h>
#include <SystemConfiguration/SCPrivate.h>
#include "SCPreferencesInternal.h"

#define	MAXLINKS	8

static CF_RETURNS_RETAINED CFMutableArrayRef
normalizePath(CFStringRef path)
{
	CFMutableArrayRef	elements;
	CFIndex			n;
	CFArrayRef		tmpElements;

	if (!isA_CFString(path)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	if (!CFStringHasPrefix(path, CFSTR("/"))) {
		/* if no root separator */
		return NULL;
	}

	tmpElements = CFStringCreateArrayBySeparatingStrings(NULL, path, CFSTR("/"));
	elements    = CFArrayCreateMutableCopy(NULL, 0, tmpElements);
	CFRelease(tmpElements);

	/* remove empty path components */
	n = CFArrayGetCount(elements);
	while (n-- > 0) {
		CFStringRef	pathElement;

		pathElement = CFArrayGetValueAtIndex(elements, n);
		if (CFStringGetLength(pathElement) == 0) {
			CFArrayRemoveValueAtIndex(elements, n);
		}
	}

	return elements;
}


static Boolean
getPath(SCPreferencesRef prefs, CFStringRef path, CFDictionaryRef *entity)
{
	CFStringRef		element;
	CFMutableArrayRef	elements;
	CFIndex			i;
	CFStringRef		link;
	CFIndex			nElements;
	CFIndex			nLinks		= 0;
	Boolean			ok		= FALSE;
	CFDictionaryRef		value		= NULL;

	elements = normalizePath(path);
	if (elements == NULL) {
		_SCErrorSet(kSCStatusNoKey);
		return FALSE;
	}

    restart :

	nElements = CFArrayGetCount(elements);

	if (nElements < 1) {
		SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;

		__SCPreferencesAccess(prefs);

		*entity = prefsPrivate->prefs;
		ok = TRUE;
		goto done;
	}

	for (i = 0; i < nElements; i++) {
		element = CFArrayGetValueAtIndex(elements, i);
		if (i == 0) {
			value = SCPreferencesGetValue(prefs, CFArrayGetValueAtIndex(elements, 0));
		} else {
			value = CFDictionaryGetValue(value, element);
		}
		if (value == NULL) {
			/* if path component does not exist */
			_SCErrorSet(kSCStatusNoKey);
			goto done;
		}

		if (!isA_CFDictionary(value)) {
			/* if path component not a dictionary */
			_SCErrorSet(kSCStatusNoKey);
			goto done;
		}

		if ((i < nElements - 1) &&
		    CFDictionaryGetValueIfPresent(value, kSCResvLink, (const void **)&link)) {
			/*
			 * if not the last path component and this
			 * element is a link
			 */
			CFMutableArrayRef	linkElements;

			if (++nLinks > MAXLINKS) {
				/* if we are chasing our tail */
				_SCErrorSet(kSCStatusMaxLink);
				goto done;
			}

			linkElements = normalizePath(link);
			if (linkElements == NULL) {
				/* if the link is bad */
				_SCErrorSet(kSCStatusNoKey);
				goto done;
			}

			CFArrayAppendArray(linkElements,
					   elements,
					   CFRangeMake(i + 1, nElements-i - 1));
			CFRelease(elements);
			elements = linkElements;

			goto restart;
		}
	}

	*entity = value;
	ok = TRUE;

    done :

	CFRelease(elements);
	return ok;
}


static Boolean
setPath(SCPreferencesRef prefs, CFStringRef path, CFDictionaryRef entity)
{
	CFStringRef		element;
	CFMutableArrayRef	elements;
	CFIndex			i;
	CFStringRef		link;
	CFIndex			nElements;
	CFIndex			nLinks		= 0;
	CFDictionaryRef		newEntity	= NULL;
	CFDictionaryRef		node		= NULL;
	CFMutableArrayRef	nodes		= NULL;
	Boolean			ok		= FALSE;

	if ((entity != NULL) && !isA_CFDictionary(entity)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	elements = normalizePath(path);
	if (elements == NULL) {
		_SCErrorSet(kSCStatusNoKey);
		return FALSE;
	}

    restart :

	nElements = CFArrayGetCount(elements);

	if (nElements < 1) {
		SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;

		__SCPreferencesAccess(prefs);

		if (prefsPrivate->prefs != NULL) {
			CFRelease(prefsPrivate->prefs);
		}

		if (entity == NULL) {
			prefsPrivate->prefs = CFDictionaryCreateMutable(NULL,
									0,
									&kCFTypeDictionaryKeyCallBacks,
									&kCFTypeDictionaryValueCallBacks);
		} else {
			prefsPrivate->prefs = CFDictionaryCreateMutableCopy(NULL, 0, entity);
		}

		prefsPrivate->changed = TRUE;
		ok = TRUE;
		goto done;
	}

	nodes = CFArrayCreateMutable(NULL, nElements - 1, &kCFTypeArrayCallBacks);
	for (i = 0; i < nElements - 1; i++) {
		element = CFArrayGetValueAtIndex(elements, i);
		if (i == 0) {
			node = SCPreferencesGetValue(prefs, element);
		} else {
			node = CFDictionaryGetValue(node, element);

		}

		if (node) {
			/* if path component exists */
			CFArrayAppendValue(nodes, node);
		} else {
			/* if path component does not exist */
			node = CFDictionaryCreate(NULL,
						  NULL,
						  NULL,
						  0,
						  &kCFTypeDictionaryKeyCallBacks,
						  &kCFTypeDictionaryValueCallBacks);
			CFArrayAppendValue(nodes, node);
			CFRelease(node);
		}

		if (!isA_CFDictionary(node)) {
			_SCErrorSet(kSCStatusNoKey);
			goto done;
		}

		if ((i < nElements - 1) &&
		    CFDictionaryGetValueIfPresent(node, kSCResvLink, (const void **)&link)) {
			/*
			 * if not the last path component and this
			 * element is a link
			 */
			CFMutableArrayRef	linkElements;

			if (++nLinks > MAXLINKS) {
				/* if we are chasing our tail */
				_SCErrorSet(kSCStatusMaxLink);
				goto done;
			}

			linkElements = normalizePath(link);
			if (linkElements == NULL) {
				/* if the link is bad */
				_SCErrorSet(kSCStatusNoKey);
				goto done;
			}

			CFArrayAppendArray(linkElements,
					   elements,
					   CFRangeMake(i + 1, nElements-i - 1));
			CFRelease(elements);
			elements = linkElements;

			CFRelease(nodes);
			nodes = NULL;
			goto restart;
		}
	}

	/*
	 * make sure that the last component doesn't step on top
	 * of a non-dictionary component.
	 */
	element = CFArrayGetValueAtIndex(elements, nElements - 1);
	if (nElements > 1) {
		node = CFArrayGetValueAtIndex(nodes, nElements - 2);
		node = CFDictionaryGetValue(node, element);
	} else {
		node = SCPreferencesGetValue(prefs, element);
	}
	if ((node != NULL) && !isA_CFDictionary(node)) {
		// we won't step on a non-dictionary component
		_SCErrorSet(kSCStatusInvalidArgument);
		goto done;
	}

	if (entity != NULL) {
		newEntity = CFRetain(entity);
	}
	for (i = nElements - 1; i >= 0; i--) {
		element = CFArrayGetValueAtIndex(elements, i);
		if (i == 0) {
			if (newEntity != NULL) {
				ok = SCPreferencesSetValue(prefs, element, newEntity);
			} else {
				ok = SCPreferencesRemoveValue(prefs, element);
			}
		} else {
			CFMutableDictionaryRef	newNode;

			node    = CFArrayGetValueAtIndex(nodes, i - 1);
			newNode = CFDictionaryCreateMutableCopy(NULL, 0, node);
			if (newEntity != NULL) {
				CFDictionarySetValue(newNode, element, newEntity);
				CFRelease(newEntity);
			} else {
				CFDictionaryRemoveValue(newNode, element);
				if (CFDictionaryGetCount(newNode) == 0) {
					// prune the (now empty) parent
					CFRelease(newNode);
					newNode = NULL;
				}
			}
			newEntity = newNode;
		}
	}
	if (newEntity != NULL) {
		CFRelease(newEntity);
	}

    done :

	if (nodes != NULL)	CFRelease(nodes);
	CFRelease(elements);
	return ok;
}


CFStringRef
SCPreferencesPathCreateUniqueChild(SCPreferencesRef	prefs,
				   CFStringRef		prefix)
{
	CFStringRef             child;
	CFStringRef		newPath		= NULL;
	CFDictionaryRef		newDict;
	CFUUIDRef               uuid;
	CFDictionaryRef		entity;

	if (prefs == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoPrefsSession);
		return NULL;
	}

	if (getPath(prefs, prefix, &entity)) {
		// if prefix path exists
		if (CFDictionaryContainsKey(entity, kSCResvLink)) {
			/* the path is a link... */
			_SCErrorSet(kSCStatusFailed);
			return NULL;
		}
	} else if (SCError() != kSCStatusNoKey) {
		// if any error except for a missing prefix path component
		return NULL;
	}

	uuid    = CFUUIDCreate(NULL);
	child   = CFUUIDCreateString(NULL, uuid);
	newPath = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@/%@"), prefix, child);
	CFRelease(child);
	CFRelease(uuid);

	/* save a new/empty dictionary */
	newDict = CFDictionaryCreate(NULL,
				     NULL, NULL, 0,
				     &kCFTypeDictionaryKeyCallBacks,
				     &kCFTypeDictionaryValueCallBacks);
	assert(newDict != NULL);

	if (!setPath(prefs, newPath, newDict)) {
		CFRelease(newPath);
		newPath = NULL;
	}
	CFRelease(newDict);

	return newPath;
}


CFDictionaryRef
SCPreferencesPathGetValue(SCPreferencesRef	prefs,
			  CFStringRef		path)
{
	CFDictionaryRef	entity;
	CFStringRef	entityLink;

	if (prefs == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoPrefsSession);
		return NULL;
	}

	if (!getPath(prefs, path, &entity)) {
		return NULL;
	}

	if (isA_CFDictionary(entity) &&
	    (CFDictionaryGetValueIfPresent(entity, kSCResvLink, (const void **)&entityLink))) {
		/* if this is a dictionary AND it is a link */
		if (!getPath(prefs, entityLink, &entity)) {
			/* if it was a bad link */
			return NULL;
		}
	}

	return entity;
}


CFStringRef
SCPreferencesPathGetLink(SCPreferencesRef	prefs,
			 CFStringRef		path)
{
	CFDictionaryRef	entity;
	CFStringRef	entityLink;

	if (prefs == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoPrefsSession);
		return NULL;
	}

	if (!getPath(prefs, path, &entity)) {
		return NULL;
	}

	if (isA_CFDictionary(entity) &&
	    (CFDictionaryGetValueIfPresent(entity, kSCResvLink, (const void **)&entityLink))) {
		/* if this is a dictionary AND it is a link */
		return entityLink;
	}

	return NULL;
}


Boolean
SCPreferencesPathSetValue(SCPreferencesRef	prefs,
			  CFStringRef		path,
			  CFDictionaryRef	value)
{
	Boolean			ok;

	if (prefs == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoPrefsSession);
		return FALSE;
	}

#define	NETPREF_NEEDS_REPAIR
#ifdef	NETPREF_NEEDS_REPAIR
	if (CFEqual(path, CFSTR("/CurrentSet")) && isA_CFString(value)) {
		static Boolean	warned	= FALSE;
		if (!warned) {
			SCPrint(TRUE, stderr, CFSTR("SCPreferencesPathSetValue(, %@, ) called with non-dictionary value\n"), path);
			warned = TRUE;
		}
		return SCPreferencesSetValue(prefs, CFSTR("CurrentSet"), value);
	}
#endif	// NETPREF_NEEDS_REPAIR

	if (!isA_CFDictionary(value)) {
#ifdef	NETPREF_NEEDS_REPAIR
SCPrint(TRUE, stderr, CFSTR("SCPreferencesPathSetValue(, %@, ) called with non-dictionary value\n"), path);
#endif	// NETPREF_NEEDS_REPAIR
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	ok = setPath(prefs, path, value);
	return ok;
}


Boolean
SCPreferencesPathSetLink(SCPreferencesRef	prefs,
			 CFStringRef		path,
			 CFStringRef		link)
{
	CFMutableDictionaryRef	dict;
	CFDictionaryRef		entity;
	Boolean			ok;

	if (prefs == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoPrefsSession);
		return FALSE;
	}

	if (!isA_CFString(link)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (!getPath(prefs, link, &entity)) {
		// if bad link
		return FALSE;
	}

	dict = CFDictionaryCreateMutable(NULL,
					 0,
					 &kCFTypeDictionaryKeyCallBacks,
					 &kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(dict, kSCResvLink, link);
	ok = setPath(prefs, path, dict);
	CFRelease(dict);

	return ok;
}


Boolean
SCPreferencesPathRemoveValue(SCPreferencesRef	prefs,
			     CFStringRef	path)
{
	CFMutableArrayRef	elements	= NULL;
	Boolean			ok		= FALSE;
	CFDictionaryRef		value;

	if (prefs == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoPrefsSession);
		return FALSE;
	}

	if (!getPath(prefs, path, &value)) {
		// if no such path
		return FALSE;
	}

	elements = normalizePath(path);
	if (elements == NULL) {
		_SCErrorSet(kSCStatusNoKey);
		return FALSE;
	}

	ok = setPath(prefs, path, NULL);

	CFRelease(elements);
	return ok;
}

/*
 * Copyright (c) 2006, 2007, 2010, 2014 Apple Inc. All rights reserved.
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
 * May 24, 2006	Allan Nathanson (ajn@apple.com)
 * - adapted (for SystemConfiguration)
 *
 * May 10, 2006	Dieter Siegmund (dieter@apple.com)
 * - created (for EAP)
 */

#include <Availability.h>
#include <TargetConditionals.h>
#include <sys/param.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFBundlePriv.h>	// for _CFBundleCopyMainBundleExecutableURL
#include <SystemConfiguration/SCPrivate.h>	// for _SCErrorSet
#include "dy_framework.h"

#include "SCPreferencesInternal.h"


#if	!TARGET_OS_IPHONE
static CFDataRef
copyMyExecutablePath(void)
{
	char		fspath[MAXPATHLEN];
	Boolean		isBundle	= FALSE;
	Boolean		ok;
	CFDataRef	path		= NULL;
	CFURLRef	url;

	url = _CFBundleCopyMainBundleExecutableURL(&isBundle);
	if (url == NULL) {
		return NULL;
	}

	ok  = CFURLGetFileSystemRepresentation(url, TRUE, (UInt8 *)fspath, sizeof(fspath));
	CFRelease(url);
	if (!ok) {
		return NULL;
	}
	fspath[sizeof(fspath) - 1] = '\0';

	if (isBundle) {
		const char	*slash;

		slash = strrchr(fspath, '/');
		if (slash != NULL) {
			const char	*contents;

			contents = strstr(fspath, "/Contents/MacOS/");
			if ((contents != NULL) &&
			    ((contents + sizeof("/Contents/MacOS/") - 1) == slash)) {
				path = CFDataCreate(NULL, (UInt8 *)fspath, contents - fspath);
				goto done;
			}
		}
	}

	path = CFDataCreate(NULL, (UInt8 *)fspath, strlen(fspath));

    done :

	return path;
}


#pragma mark -
#pragma mark Keychain helper APIs


#if	(__MAC_OS_X_VERSION_MIN_REQUIRED < 1070)
/*
 * Create a SecAccessRef with a custom form.
 *
 * Both the owner and the ACL set allow free access to root,
 * but nothing to anyone else.
 *
 * NOTE: This is not the easiest way to build up CSSM data structures
 *       but it is a way that does not depend on any outside software
 *       layers (other than CSSM and Security's Sec* layer, of course).
 */
static SecAccessRef
_SCSecAccessCreateForUID(uid_t uid)
{
	SecAccessRef	access	= NULL;
	OSStatus	status;

	// make the "uid/gid" ACL subject
	// this is a CSSM_LIST_ELEMENT chain

	CSSM_ACL_PROCESS_SUBJECT_SELECTOR	selector	= {
		CSSM_ACL_PROCESS_SELECTOR_CURRENT_VERSION,			// version
		CSSM_ACL_MATCH_UID,						// active fields mask: match uids (only)
		uid,								// effective user id to match
		0								// effective group id to match
	};

	CSSM_LIST_ELEMENT			subject2	= {
		NULL,								// NextElement
		0								// WordID
		// rest is defaulted
	};

	subject2.Element.Word.Data   = (UInt8 *)&selector;
	subject2.Element.Word.Length = sizeof(selector);

	CSSM_LIST_ELEMENT			subject1	= {
		&subject2,							// NextElement
		CSSM_ACL_SUBJECT_TYPE_PROCESS,					// WordID
		CSSM_LIST_ELEMENT_WORDID					// ElementType
		// rest is defaulted
	};

	// rights granted (replace with individual list if desired)
	CSSM_ACL_AUTHORIZATION_TAG		rights[]	= {
		CSSM_ACL_AUTHORIZATION_ANY					// everything
	};

	// owner component (right to change ACL)
	CSSM_ACL_OWNER_PROTOTYPE		owner		= {
		{								// TypedSubject
			CSSM_LIST_TYPE_UNKNOWN,					   // type of this list
			&subject1,						   // head of the list
			&subject2						   // tail of the list
		},
		FALSE								// Delegate
	};

	// ACL entries (any number, just one here)
	CSSM_ACL_ENTRY_INFO			acls[]		= {
		{
			{							// EntryPublicInfo
				{						   // TypedSubject
					CSSM_LIST_TYPE_UNKNOWN,			      // type of this list
					&subject1,				      // head of the list
					&subject2				      // tail of the list
				},
				FALSE,						   // Delegate
				{						   // Authorization
					sizeof(rights) / sizeof(rights[0]),	      // NumberOfAuthTags
					rights					      // AuthTags
				},
				{						   // TimeRange
				},
				{						   // EntryTag
				}
			},
			0							// EntryHandle
		}
	};

	status = SecAccessCreateFromOwnerAndACL(&owner,
						sizeof(acls) / sizeof(acls[0]),
						acls,
						&access);
	if (status != noErr) {
		_SCErrorSet(status);
	}

	return access;
}
#endif	// (__MAC_OS_X_VERSION_MIN_REQUIRED < 1070)


// one example would be to pass a URL for "/System/Library/CoreServices/SystemUIServer.app"
static SecAccessRef
_SCSecAccessCreateForExecutables(CFStringRef	label,
				 CFArrayRef	executableURLs)
{
	SecAccessRef			access			= NULL;
	CFArrayRef			aclList			= NULL;
	CFIndex				i;
	CFIndex				n;
	OSStatus			status;
	SecTrustedApplicationRef	trustedApplication;
	CFMutableArrayRef		trustedApplications;

	trustedApplications = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	// Use default access ("confirm access")

	// Next, we make an exception list of applications you want to trust.
	// These applications will be allowed to access the item without requiring
	// user confirmation.

	// Trust the calling application
	status = SecTrustedApplicationCreateFromPath(NULL, &trustedApplication);
	if (status == noErr) {
		CFArrayAppendValue(trustedApplications, trustedApplication);
		CFRelease(trustedApplication);
	}

	n = (executableURLs != NULL) ? CFArrayGetCount(executableURLs) : 0;
	for (i = 0; i < n; i++) {
		Boolean		ok;
		char		path[MAXPATHLEN];
		CFURLRef	url;

		url = CFArrayGetValueAtIndex(executableURLs, i);
		ok = CFURLGetFileSystemRepresentation(url, TRUE, (UInt8 *)path, sizeof(path));
		if (!ok) {
			continue;
		}

		status = SecTrustedApplicationCreateFromPath(path, &trustedApplication);
		if (status == noErr) {
			CFArrayAppendValue(trustedApplications, trustedApplication);
			CFRelease(trustedApplication);
		}
	}

	status = SecAccessCreate(label, trustedApplications, &access);
	if (status != noErr) {
		goto done;
	}

    done :

	if (aclList != NULL)	CFRelease(aclList);
	CFRelease(trustedApplications);

	return	access;
}
#endif	// !TARGET_OS_IPHONE


SecKeychainRef
_SCSecKeychainCopySystemKeychain(void)
{
#if	!TARGET_OS_IPHONE
	SecKeychainRef		keychain	= NULL;
	OSStatus		status;

	status = SecKeychainCopyDomainDefault(kSecPreferencesDomainSystem, &keychain);
	if (status != noErr) {
		_SCErrorSet(status);
		if (keychain != NULL) CFRelease(keychain);
		return NULL;
	}

	return keychain;
#else	// !TARGET_OS_IPHONE
	_SCErrorSet(kSCStatusAccessError);
	return NULL;
#endif	// !TARGET_OS_IPHONE
}


#if	!TARGET_OS_IPHONE
static OSStatus
findKeychainItem(SecKeychainRef		keychain,
		 CFStringRef		unique_id,
		 SecKeychainItemRef	*item)
{
	CFMutableDictionaryRef	query;
	OSStatus		status;

	query = CFDictionaryCreateMutable(NULL,
					  0,
					  &kCFTypeDictionaryKeyCallBacks,
					  &kCFTypeDictionaryValueCallBacks);
	if (keychain != NULL) {
		CFArrayRef	keychains;

		keychains = CFArrayCreate(NULL, (const void **)&keychain, 1, &kCFTypeArrayCallBacks);
		CFDictionarySetValue(query, kSecMatchSearchList, keychains);
		CFRelease(keychains);
	}
	CFDictionarySetValue(query, kSecClass      , kSecClassGenericPassword);
	CFDictionarySetValue(query, kSecAttrService, unique_id);
	CFDictionarySetValue(query, kSecReturnRef  , kCFBooleanTrue);
	status = SecItemCopyMatching(query, (CFTypeRef *)item);
	CFRelease(query);

	return status;
}
#endif	// !TARGET_OS_IPHONE


CFDataRef
_SCSecKeychainPasswordItemCopy(SecKeychainRef	keychain,
			       CFStringRef	unique_id)
{
#if	!TARGET_OS_IPHONE
	SecKeychainItemRef	item			= NULL;
	CFDataRef		keychain_password	= NULL;
	OSStatus		status;

	status = findKeychainItem(keychain, unique_id, &item);
	if (status == noErr) {
		void *		pw	= NULL;
		UInt32 		pw_len	= 0;

		status = SecKeychainItemCopyContent(item, NULL, NULL, &pw_len, &pw);
		if (status == noErr) {
			keychain_password = CFDataCreate(NULL, pw, pw_len);
			status = SecKeychainItemFreeContent(NULL, pw);
		}
	}
	if (item != NULL) CFRelease(item);
	if (status != noErr) {
		_SCErrorSet(status);
	}

	return keychain_password;
#else	// !TARGET_OS_IPHONE
	_SCErrorSet(kSCStatusAccessError);
	return NULL;
#endif	// !TARGET_OS_IPHONE
}


Boolean
_SCSecKeychainPasswordItemExists(SecKeychainRef keychain, CFStringRef unique_id)
{
#if	!TARGET_OS_IPHONE
	SecKeychainItemRef	item;
	OSStatus		status;

	status = findKeychainItem(keychain, unique_id, &item);
	if (status != noErr) {
		_SCErrorSet(status);
		return FALSE;
	}

	CFRelease(item);
	return TRUE;
#else	// !TARGET_OS_IPHONE
	_SCErrorSet(kSCStatusAccessError);
	return FALSE;
#endif	// !TARGET_OS_IPHONE
}


Boolean
_SCSecKeychainPasswordItemRemove(SecKeychainRef keychain, CFStringRef unique_id)
{
#if	!TARGET_OS_IPHONE
	SecKeychainItemRef	item;
	OSStatus		status;

	status = findKeychainItem(keychain, unique_id, &item);
	if (status != noErr) {
		_SCErrorSet(status);
		return FALSE;
	}

	status = SecKeychainItemDelete(item);
	CFRelease(item);
	if (status != noErr) {
		_SCErrorSet(status);
		return FALSE;
	}

	return TRUE;
#else	// !TARGET_OS_IPHONE
	_SCErrorSet(kSCStatusAccessError);
	return FALSE;
#endif	// !TARGET_OS_IPHONE
}


Boolean
_SCSecKeychainPasswordItemSet(SecKeychainRef	keychain,
			      CFStringRef	unique_id,
			      CFStringRef	label,
			      CFStringRef	description,
			      CFStringRef	account,
			      CFDataRef		password,
			      CFDictionaryRef	options)
{
#if	!TARGET_OS_IPHONE
	SecAccessRef			access			= NULL;
	CFBooleanRef			allowRoot		= NULL;
	CFArrayRef			allowedExecutables	= NULL;
	SecKeychainAttribute		attributes[4];
	SecKeychainAttributeList	attributeList		= { 0, attributes };
	CFIndex				i;
	SecKeychainItemRef		item			= NULL;
	CFIndex				n			= 0;
	OSStatus			status;

	if (options != NULL) {
		if (isA_CFDictionary(options)) {
			allowRoot          = CFDictionaryGetValue(options, kSCKeychainOptionsAllowRoot);
			allowedExecutables = CFDictionaryGetValue(options, kSCKeychainOptionsAllowedExecutables);
		} else {
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
		}
	}

	if (!isA_CFString(unique_id) ||
	    ((label              != NULL) && !isA_CFString (label             )) ||
	    ((description        != NULL) && !isA_CFString (description       )) ||
	    ((account            != NULL) && !isA_CFString (account           )) ||
	    ((password           != NULL) && !isA_CFData   (password          )) ||
	    ((allowRoot          != NULL) && !isA_CFBoolean(allowRoot         )) ||
	    ((allowedExecutables != NULL) && !isA_CFArray  (allowedExecutables)) ||
	    ((allowRoot          != NULL) && (allowedExecutables != NULL))) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if ((allowRoot != NULL) && CFBooleanGetValue(allowRoot)) {
#if	(__MAC_OS_X_VERSION_MIN_REQUIRED < 1070)
		access = _SCSecAccessCreateForUID(0);
		if (access == NULL) {
			return FALSE;
		}
#else	// (__MAC_OS_X_VERSION_MIN_REQUIRED < 1070)
		CFErrorRef	error	= NULL;

		access = SecAccessCreateWithOwnerAndACL(0, 0, kSecUseOnlyUID, NULL, &error);
		if (access == NULL) {
			CFIndex	code	= kSCStatusAccessError;

			if (error != NULL) {

				code = CFErrorGetCode(error);
				CFRelease(error);
			}
			_SCErrorSet((int)code);
			return FALSE;
		}
#endif	// (__MAC_OS_X_VERSION_MIN_REQUIRED < 1070)
	} else if (allowedExecutables != NULL) {
		access = _SCSecAccessCreateForExecutables(label, allowedExecutables);
		if (access == NULL) {
			return FALSE;
		}
	}

	for (i = 0; i < 4; i++) {
		CFStringRef		str	= NULL;
		SecKeychainAttrType	tag	= 0;

		switch (i) {
			case 0 :
				str = unique_id;
				tag = kSecServiceItemAttr;
				break;
			case 1 :
				str = label;
				tag = kSecLabelItemAttr;
				break;
			case 2 :
				str = description;
				tag = kSecDescriptionItemAttr;
				break;
			case 3 :
				str = account;
				tag = kSecAccountItemAttr;
				break;
		}

		if (str == NULL) {
			continue;
		}

		attributes[n].tag    = tag;
		attributes[n].data   = _SC_cfstring_to_cstring(str, NULL, 0, kCFStringEncodingUTF8);
		attributes[n].length = (UInt32)strlen(attributes[n].data);
		n++;
	}

	status = findKeychainItem(keychain, unique_id, &item);
	switch (status) {
		case noErr : {
			const void	*pw	= NULL;
			UInt32		pw_len	= 0;

			// keychain item exists
			if (password != NULL) {
				pw     = CFDataGetBytePtr(password);
				pw_len = (UInt32)CFDataGetLength(password);
			}

			attributeList.count = (UInt32)n;
			status = SecKeychainItemModifyContent(item,
							      &attributeList,
							      pw_len,
							      pw);
			break;
		}

		case errSecItemNotFound : {
			// no keychain item
			if (password == NULL) {
				// creating new keychain item and password not specified
				status = kSCStatusInvalidArgument;
				goto done;
			}

			attributeList.count = (UInt32)n;
			status = SecKeychainItemCreateFromContent(kSecGenericPasswordItemClass,
								&attributeList,
								(UInt32)CFDataGetLength(password),
								CFDataGetBytePtr(password),
								keychain,
								access,
								NULL);
			break;
		}

		// some other error
		default :
			break;
	}

    done :

	if (access != NULL)	CFRelease(access);
	if (item != NULL)	CFRelease(item);

	for (i = 0; i < n; i++) {
		CFAllocatorDeallocate(NULL, attributes[i].data);
	}

	if (status != noErr) {
		_SCErrorSet(status);
		return FALSE;
	}

	return TRUE;
#else	// !TARGET_OS_IPHONE
	_SCErrorSet(kSCStatusAccessError);
	return FALSE;
#endif	// !TARGET_OS_IPHONE
}


#pragma mark -
#pragma mark "System" Keychain APIs (w/SCPreferences)


#include "SCHelper_client.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/errno.h>


#if	!TARGET_OS_IPHONE
static CFDataRef
__SCPreferencesSystemKeychainPasswordItemCopy_helper(SCPreferencesRef	prefs,
						     CFStringRef	unique_id)
{
	CFDataRef		data		= NULL;
	Boolean			ok;
	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;
	uint32_t		status		= kSCStatusOK;
	CFDataRef		reply		= NULL;

	if (prefsPrivate->helper_port == MACH_PORT_NULL) {
		ok = __SCPreferencesCreate_helper(prefs);
		if (!ok) {
			return FALSE;
		}
	}

	ok = _SCSerializeString(unique_id, &data, NULL, NULL);
	if (!ok) {
		goto fail;
	}

	// have the helper set the "System" Keychain password
	ok = _SCHelperExec(prefsPrivate->helper_port,
			   SCHELPER_MSG_KEYCHAIN_COPY,
			   data,
			   &status,
			   &reply);
	if (data != NULL) CFRelease(data);
	if (!ok) {
		goto fail;
	}

	if (status != kSCStatusOK) {
		goto error;
	}

	return reply;

    fail :

	// close helper
	if (prefsPrivate->helper_port != MACH_PORT_NULL) {
		_SCHelperClose(&prefsPrivate->helper_port);
	}

	status = kSCStatusAccessError;

    error :

	// return error
	if (reply != NULL) CFRelease(reply);
	_SCErrorSet(status);
	return NULL;
}
#endif	// !TARGET_OS_IPHONE


CFDataRef
_SCPreferencesSystemKeychainPasswordItemCopy(SCPreferencesRef	prefs,
					     CFStringRef	unique_id)
{
#if	!TARGET_OS_IPHONE
	SecKeychainRef		keychain	= NULL;
	CFDataRef		password	= NULL;
	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;

	if (prefs == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoPrefsSession);
		return NULL;
	}

	if (!isA_CFString(unique_id)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	if (prefsPrivate->authorizationData != NULL) {
		password = __SCPreferencesSystemKeychainPasswordItemCopy_helper(prefs, unique_id);
		goto done;
	}

	keychain = _SCSecKeychainCopySystemKeychain();
	if (keychain == NULL) {
		goto done;
	}

	password = _SCSecKeychainPasswordItemCopy(keychain, unique_id);

    done :

	if (keychain != NULL)	CFRelease(keychain);
	return password;
#else	// !TARGET_OS_IPHONE
	_SCErrorSet(kSCStatusAccessError);
	return NULL;
#endif	// !TARGET_OS_IPHONE
}


Boolean
_SCPreferencesSystemKeychainPasswordItemExists(SCPreferencesRef	prefs,
					       CFStringRef	unique_id)
{
#if	!TARGET_OS_IPHONE
	SecKeychainRef		keychain	= NULL;
	Boolean			ok		= FALSE;
//	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;

	if (prefs == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoPrefsSession);
		return FALSE;
	}

	if (!isA_CFString(unique_id)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

//	if (prefsPrivate->authorizationData != NULL) {
//		ok = __SCPreferencesSystemKeychainPasswordItemExists_helper(prefs, unique_id);
//		goto done;
//	}

	keychain = _SCSecKeychainCopySystemKeychain();
	if (keychain == NULL) {
		goto done;
	}

	ok = _SCSecKeychainPasswordItemExists(keychain, unique_id);

    done :

	if (keychain != NULL)	CFRelease(keychain);
	return ok;
#else	// !TARGET_OS_IPHONE
	_SCErrorSet(kSCStatusAccessError);
	return FALSE;
#endif	// !TARGET_OS_IPHONE
}


#if	!TARGET_OS_IPHONE
static Boolean
__SCPreferencesSystemKeychainPasswordItemRemove_helper(SCPreferencesRef	prefs,
						       CFStringRef	unique_id)
{
	CFDataRef		data		= NULL;
	Boolean			ok;
	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;
	uint32_t		status		= kSCStatusOK;
	CFDataRef		reply		= NULL;

	if (prefsPrivate->helper_port == MACH_PORT_NULL) {
		ok = __SCPreferencesCreate_helper(prefs);
		if (!ok) {
			return FALSE;
		}
	}

	ok = _SCSerializeString(unique_id, &data, NULL, NULL);
	if (!ok) {
		goto fail;
	}

	// have the helper set the "System" Keychain password
	ok = _SCHelperExec(prefsPrivate->helper_port,
			   SCHELPER_MSG_KEYCHAIN_REMOVE,
			   data,
			   &status,
			   &reply);
	if (data != NULL) CFRelease(data);
	if (!ok) {
		goto fail;
	}

	if (status != kSCStatusOK) {
		goto error;
	}

	return TRUE;

    fail :

	// close helper
	if (prefsPrivate->helper_port != MACH_PORT_NULL) {
		_SCHelperClose(&prefsPrivate->helper_port);
	}

	status = kSCStatusAccessError;

    error :

	// return error
	if (reply != NULL) CFRelease(reply);
	_SCErrorSet(status);
	return FALSE;
}
#endif	// !TARGET_OS_IPHONE


Boolean
_SCPreferencesSystemKeychainPasswordItemRemove(SCPreferencesRef	prefs,
					       CFStringRef	unique_id)
{
#if	!TARGET_OS_IPHONE
	SecKeychainRef		keychain	= NULL;
	Boolean			ok		= FALSE;
	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;

	if (prefs == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoPrefsSession);
		return FALSE;
	}

	if (!isA_CFString(unique_id)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (prefsPrivate->authorizationData != NULL) {
		ok = __SCPreferencesSystemKeychainPasswordItemRemove_helper(prefs, unique_id);
		goto done;
	}

	keychain = _SCSecKeychainCopySystemKeychain();
	if (keychain == NULL) {
		goto done;
	}

	ok = _SCSecKeychainPasswordItemRemove(keychain, unique_id);

    done :

	if (keychain != NULL)	CFRelease(keychain);
	return ok;
#else	// !TARGET_OS_IPHONE
	_SCErrorSet(kSCStatusAccessError);
	return FALSE;
#endif	// !TARGET_OS_IPHONE
}


#if	!TARGET_OS_IPHONE
static Boolean
__SCPreferencesSystemKeychainPasswordItemSet_helper(SCPreferencesRef	prefs,
						    CFStringRef		unique_id,
						    CFStringRef		label,
						    CFStringRef		description,
						    CFStringRef		account,
						    CFDataRef		password,
						    CFDictionaryRef	options)
{
	CFDataRef		data		= NULL;
	CFMutableDictionaryRef	newOptions	= NULL;
	Boolean			ok;
	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;
	uint32_t		status		= kSCStatusOK;
	CFDataRef		reply		= NULL;

	if (prefsPrivate->helper_port == MACH_PORT_NULL) {
		ok = __SCPreferencesCreate_helper(prefs);
		if (!ok) {
			return FALSE;
		}
	}

	if (isA_CFDictionary(options)) {
		CFArrayRef	executableURLs	= NULL;

		newOptions = CFDictionaryCreateMutableCopy(NULL, 0, options);

		if (CFDictionaryGetValueIfPresent(newOptions,
						  kSCKeychainOptionsAllowedExecutables,
						  (const void **)&executableURLs)) {
			CFMutableArrayRef	executablePaths;
			CFIndex			i;
			CFIndex			n;
			CFDataRef		path;

			executablePaths = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

			path = copyMyExecutablePath();
			if (path != NULL) {
				CFArrayAppendValue(executablePaths, path);
				CFRelease(path);
			}

			n = CFArrayGetCount(executableURLs);
			for (i = 0; i < n; i++) {
				char		fspath[MAXPATHLEN];
				CFURLRef	url;

				url = CFArrayGetValueAtIndex(executableURLs, i);
				ok  = CFURLGetFileSystemRepresentation(url, TRUE, (UInt8 *)fspath, sizeof(fspath));
				if (!ok) {
					continue;
				}
				fspath[sizeof(fspath) - 1] = '\0';
				path = CFDataCreate(NULL, (UInt8 *)fspath, strlen(fspath));
				CFArrayAppendValue(executablePaths, path);
				CFRelease(path);
			}

			CFDictionarySetValue(newOptions, kSCKeychainOptionsAllowedExecutables, executablePaths);
			CFRelease(executablePaths);
		}
	} else {
		newOptions = CFDictionaryCreateMutable(NULL,
						       0,
						       &kCFTypeDictionaryKeyCallBacks,
						       &kCFTypeDictionaryValueCallBacks);
	}

	if (unique_id   != NULL) CFDictionarySetValue(newOptions, kSCKeychainOptionsUniqueID   , unique_id);
	if (label       != NULL) CFDictionarySetValue(newOptions, kSCKeychainOptionsLabel      , label);
	if (description != NULL) CFDictionarySetValue(newOptions, kSCKeychainOptionsDescription, description);
	if (account     != NULL) CFDictionarySetValue(newOptions, kSCKeychainOptionsAccount    , account);
	if (password    != NULL) CFDictionarySetValue(newOptions, kSCKeychainOptionsPassword   , password);

	//
	// if not AllowRoot and a list of executables was not provided than
	// pass the current executable
	//
	if (!CFDictionaryContainsKey(newOptions, kSCKeychainOptionsAllowRoot) &&
	    !CFDictionaryContainsKey(newOptions, kSCKeychainOptionsAllowedExecutables)) {
		CFDataRef	path;

		path = copyMyExecutablePath();
		if (path != NULL) {
			CFArrayRef	executablePaths;

			executablePaths = CFArrayCreate(NULL, (const void **)&path, 1, &kCFTypeArrayCallBacks);
			CFRelease(path);
			CFDictionarySetValue(newOptions, kSCKeychainOptionsAllowedExecutables, executablePaths);
			CFRelease(executablePaths);
		}
	}

	ok = _SCSerialize(newOptions, &data, NULL, NULL);
	CFRelease(newOptions);
	if (!ok) {
		goto fail;
	}

	// have the helper create the "System" Keychain password
	ok = _SCHelperExec(prefsPrivate->helper_port,
			   SCHELPER_MSG_KEYCHAIN_SET,
			   data,
			   &status,
			   &reply);
	if (data != NULL) CFRelease(data);
	if (!ok) {
		goto fail;
	}

	if (status != kSCStatusOK) {
		goto error;
	}

	return TRUE;

    fail :

	// close helper
	if (prefsPrivate->helper_port != MACH_PORT_NULL) {
		_SCHelperClose(&prefsPrivate->helper_port);
	}

	status = kSCStatusAccessError;

    error :

	// return error
	if (reply != NULL) CFRelease(reply);
	_SCErrorSet(status);
	return FALSE;
}
#endif	// !TARGET_OS_IPHONE


Boolean
_SCPreferencesSystemKeychainPasswordItemSet(SCPreferencesRef	prefs,
					    CFStringRef		unique_id,
					    CFStringRef		label,
					    CFStringRef		description,
					    CFStringRef		account,
					    CFDataRef		password,
					    CFDictionaryRef	options)
{
#if	!TARGET_OS_IPHONE
	SecKeychainRef		keychain		= NULL;
	Boolean			ok			= FALSE;
	SCPreferencesPrivateRef	prefsPrivate		= (SCPreferencesPrivateRef)prefs;

	if (prefs == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoPrefsSession);
		return FALSE;
	}

	if (!isA_CFString(unique_id) ||
	    ((label       != NULL) && !isA_CFString    (label      )) ||
	    ((description != NULL) && !isA_CFString    (description)) ||
	    ((account     != NULL) && !isA_CFString    (account    )) ||
	    ((password    != NULL) && !isA_CFData      (password   )) ||
	    ((options     != NULL) && !isA_CFDictionary(options    ))) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (prefsPrivate->authorizationData != NULL) {
		ok = __SCPreferencesSystemKeychainPasswordItemSet_helper(prefs,
									 unique_id,
									 label,
									 description,
									 account,
									 password,
									 options);
		goto done;
	}

	keychain = _SCSecKeychainCopySystemKeychain();
	if (keychain == NULL) {
		goto done;
	}

	ok = _SCSecKeychainPasswordItemSet(keychain,
					   unique_id,
					   label,
					   description,
					   account,
					   password,
					   options);

    done :

	if (keychain != NULL)	CFRelease(keychain);
	return ok;
#else	// !TARGET_OS_IPHONE
	_SCErrorSet(kSCStatusAccessError);
	return FALSE;
#endif	// !TARGET_OS_IPHONE
}

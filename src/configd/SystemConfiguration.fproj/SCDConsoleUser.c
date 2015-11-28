/*
 * Copyright (c) 2000-2005, 2009, 2011 Apple Inc. All rights reserved.
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
 * May 1, 2003			Allan Nathanson <ajn@apple.com>
 * - add console [session] information SPIs
 *
 * June 1, 2001			Allan Nathanson <ajn@apple.com>
 * - public API conversion
 *
 * January 2, 2001		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCValidation.h>
#include <SystemConfiguration/SCPrivate.h>


#undef	kSCPropUsersConsoleUserName
#define	kSCPropUsersConsoleUserName	CFSTR("Name")

#undef	kSCPropUsersConsoleUserUID
#define	kSCPropUsersConsoleUserUID	CFSTR("UID")

#undef	kSCPropUsersConsoleUserGID
#define	kSCPropUsersConsoleUserGID	CFSTR("GID")

#undef	kSCPropUsersConsoleSessionInfo
#define	kSCPropUsersConsoleSessionInfo	CFSTR("SessionInfo")


// from CoreGraphics (CGSession.h)
const CFStringRef kSCConsoleSessionUserName		= CFSTR("kCGSSessionUserNameKey");		/* value is CFString */
const CFStringRef kSCConsoleSessionUID			= CFSTR("kCGSSessionUserIDKey");		/* value is CFNumber (a uid_t) */
const CFStringRef kSCConsoleSessionConsoleSet		= CFSTR("kCGSSessionConsoleSetKey");		/* value is CFNumber */
const CFStringRef kSCConsoleSessionOnConsole		= CFSTR("kCGSSessionOnConsoleKey");		/* value is CFBoolean */
const CFStringRef kSCConsoleSessionLoginDone		= CFSTR("kCGSessionLoginDoneKey");		/* value is CFBoolean */

// from CoreGraphics (CGSSession.h)
const CFStringRef kSCConsoleSessionID			= CFSTR("kCGSSessionIDKey");			/* value is CFNumber */

// for loginwindow
const CFStringRef kSCConsoleSessionSystemSafeBoot	= CFSTR("kCGSSessionSystemSafeBoot");		/* value is CFBoolean */
const CFStringRef kSCConsoleSessionLoginwindowSafeLogin	= CFSTR("kCGSSessionLoginwindowSafeLogin");	/* value is CFBoolean */


CFStringRef
SCDynamicStoreKeyCreateConsoleUser(CFAllocatorRef allocator)
{
	return SCDynamicStoreKeyCreate(allocator,
				       CFSTR("%@/%@/%@"),
				       kSCDynamicStoreDomainState,
				       kSCCompUsers,
				       kSCEntUsersConsoleUser);
}


CFStringRef
SCDynamicStoreCopyConsoleUser(SCDynamicStoreRef	store,
			      uid_t		*uid,
			      gid_t		*gid)
{
	CFStringRef		consoleUser	= NULL;
	CFDictionaryRef		dict		= NULL;
	CFStringRef		key;

	key  = SCDynamicStoreKeyCreateConsoleUser(NULL);
	dict = SCDynamicStoreCopyValue(store, key);
	CFRelease(key);
	if (!isA_CFDictionary(dict)) {
		_SCErrorSet(kSCStatusNoKey);
		goto done;
	}

	consoleUser = CFDictionaryGetValue(dict, kSCPropUsersConsoleUserName);
	consoleUser = isA_CFString(consoleUser);
	if (!consoleUser) {
		_SCErrorSet(kSCStatusNoKey);
		goto done;
	}

	CFRetain(consoleUser);

	if (uid) {
		CFNumberRef	num;
		SInt32		val;

		num = CFDictionaryGetValue(dict, kSCPropUsersConsoleUserUID);
		if (isA_CFNumber(num)) {
			if (CFNumberGetValue(num, kCFNumberSInt32Type, &val)) {
				*uid = (uid_t)val;
			}
		}
	}

	if (gid) {
		CFNumberRef	num;
		SInt32		val;

		num = CFDictionaryGetValue(dict, kSCPropUsersConsoleUserGID);
		if (isA_CFNumber(num)) {
			if (CFNumberGetValue(num, kCFNumberSInt32Type, &val)) {
				*gid = (gid_t)val;
			}
		}
	}

    done :

	if (dict)		CFRelease(dict);
	return consoleUser;
}


CFArrayRef
SCDynamicStoreCopyConsoleInformation(SCDynamicStoreRef store)
{
	CFDictionaryRef		dict		= NULL;
	CFArrayRef		info		= NULL;
	CFStringRef		key;

	key  = SCDynamicStoreKeyCreateConsoleUser(NULL);
	dict = SCDynamicStoreCopyValue(store, key);
	CFRelease(key);
	if (!isA_CFDictionary(dict)) {
		_SCErrorSet(kSCStatusNoKey);
		goto done;
	}

	info = CFDictionaryGetValue(dict, kSCPropUsersConsoleSessionInfo);
	info = isA_CFArray(info);
	if (!info) {
		_SCErrorSet(kSCStatusNoKey);
		goto done;
	}

	CFRetain(info);

    done :

	if (dict)		CFRelease(dict);
	return info;
}


Boolean
SCDynamicStoreSetConsoleInformation(SCDynamicStoreRef	store,
				    const char		*user,
				    uid_t		uid,
				    gid_t		gid,
				    CFArrayRef		sessions)
{
	CFStringRef		consoleUser;
	CFMutableDictionaryRef	dict		= NULL;
	CFStringRef		key		= SCDynamicStoreKeyCreateConsoleUser(NULL);
	Boolean			ok		= FALSE;

	if ((user == NULL) && (sessions == NULL)) {
		ok = SCDynamicStoreRemoveValue(store, key);
		goto done;
	}

	dict = CFDictionaryCreateMutable(NULL,
					 0,
					 &kCFTypeDictionaryKeyCallBacks,
					 &kCFTypeDictionaryValueCallBacks);

	if (user != NULL) {
		CFNumberRef	num;

		consoleUser = CFStringCreateWithCString(NULL, user, kCFStringEncodingMacRoman);
		CFDictionarySetValue(dict, kSCPropUsersConsoleUserName, consoleUser);
		CFRelease(consoleUser);

		num = CFNumberCreate(NULL, kCFNumberSInt32Type, (SInt32 *)&uid);
		CFDictionarySetValue(dict, kSCPropUsersConsoleUserUID, num);
		CFRelease(num);

		num = CFNumberCreate(NULL, kCFNumberSInt32Type, (SInt32 *)&gid);
		CFDictionarySetValue(dict, kSCPropUsersConsoleUserGID, num);
		CFRelease(num);
	}

	if (sessions != NULL) {
		CFDictionarySetValue(dict, kSCPropUsersConsoleSessionInfo, sessions);
	}

	ok = SCDynamicStoreSetValue(store, key, dict);

    done :

	if (dict)		CFRelease(dict);
	if (key)		CFRelease(key);
	return ok;
}


Boolean
SCDynamicStoreSetConsoleUser(SCDynamicStoreRef	store,
			     const char		*user,
			     uid_t		uid,
			     gid_t		gid)
{
	CFStringRef		consoleUser;
	CFMutableDictionaryRef	dict		= NULL;
	CFStringRef		key		= SCDynamicStoreKeyCreateConsoleUser(NULL);
	CFNumberRef		num;
	Boolean			ok		= FALSE;

	if (user == NULL) {
		ok = SCDynamicStoreRemoveValue(store, key);
		goto done;
	}

	dict = CFDictionaryCreateMutable(NULL,
					 0,
					 &kCFTypeDictionaryKeyCallBacks,
					 &kCFTypeDictionaryValueCallBacks);

	consoleUser = CFStringCreateWithCString(NULL, user, kCFStringEncodingMacRoman);
	CFDictionarySetValue(dict, kSCPropUsersConsoleUserName, consoleUser);
	CFRelease(consoleUser);

	num = CFNumberCreate(NULL, kCFNumberSInt32Type, (SInt32 *)&uid);
	CFDictionarySetValue(dict, kSCPropUsersConsoleUserUID, num);
	CFRelease(num);

	num = CFNumberCreate(NULL, kCFNumberSInt32Type, (SInt32 *)&gid);
	CFDictionarySetValue(dict, kSCPropUsersConsoleUserGID, num);
	CFRelease(num);

	ok = SCDynamicStoreSetValue(store, key, dict);

    done :

	if (dict)		CFRelease(dict);
	if (key)		CFRelease(key);
	return ok;
}

/*
 * Copyright (c) 2005-2007, 2010, 2011 Apple Inc. All rights reserved.
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

#ifndef _SCHELPER_CLIENT_H
#define _SCHELPER_CLIENT_H

#include <sys/cdefs.h>
#include <Availability.h>
#include <TargetConditionals.h>
#include <CoreFoundation/CoreFoundation.h>

#define kSCKeychainOptionsAccount		CFSTR("Account")		// CFString
#define kSCKeychainOptionsDescription		CFSTR("Description")		// CFString
#define kSCKeychainOptionsLabel			CFSTR("Label")			// CFString
#define kSCKeychainOptionsPassword		CFSTR("Password")		// CFData
#define kSCKeychainOptionsUniqueID		CFSTR("UniqueID")		// CFString

#define kSCHelperAuthAuthorization		CFSTR("Authorization")		// CFData[AuthorizationExternalForm]
#define kSCHelperAuthCallerInfo			CFSTR("CallerInfo")		// CFString

enum {
	// authorization
	SCHELPER_MSG_AUTH		= 1,

	// SCPreferences
	SCHELPER_MSG_PREFS_OPEN		= 100,
	SCHELPER_MSG_PREFS_ACCESS,
	SCHELPER_MSG_PREFS_LOCK,
	SCHELPER_MSG_PREFS_LOCKWAIT,
	SCHELPER_MSG_PREFS_COMMIT,
	SCHELPER_MSG_PREFS_APPLY,
	SCHELPER_MSG_PREFS_UNLOCK,
	SCHELPER_MSG_PREFS_CLOSE,
	SCHELPER_MSG_PREFS_SYNCHRONIZE,

	// SCNetworkConfiguration
	SCHELPER_MSG_INTERFACE_REFRESH	= 200,

#if	!TARGET_OS_IPHONE
	// "System" Keychain
	SCHELPER_MSG_KEYCHAIN_COPY	= 300,
	SCHELPER_MSG_KEYCHAIN_EXISTS,
	SCHELPER_MSG_KEYCHAIN_REMOVE,
	SCHELPER_MSG_KEYCHAIN_SET,
#endif	// !TARGET_OS_IPHONE

	// miscellaneous
	SCHELPER_MSG_EXIT		= 9999
};

__BEGIN_DECLS

Boolean	_SCHelperOpen	(CFDataRef		authorizationData,
			 mach_port_t		*helper_port);

Boolean	_SCHelperExec	(mach_port_t		helper_port,
			 uint32_t		msgID,
			 CFDataRef		data,
			 uint32_t		*status,
			 CFDataRef		*reply);

void	_SCHelperClose	(mach_port_t		*helper_port);

__END_DECLS

#endif /* _SCHELPER_CLIENT_H */


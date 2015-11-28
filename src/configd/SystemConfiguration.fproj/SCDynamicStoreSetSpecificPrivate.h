/*
 * Copyright (c) 2001, 2003-2005, 2008 Apple Inc. All rights reserved.
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

#ifndef _SCDYNAMICSTORESETSPECIFICPRIVATE_H
#define _SCDYNAMICSTORESETSPECIFICPRIVATE_H

#include <Availability.h>
#include <sys/cdefs.h>
#include <SystemConfiguration/SCDynamicStore.h>


/*!
	@header SCDynamicStoreSetSpecificPrivate
 */

__BEGIN_DECLS

/*!
	@function SCDynamicStoreSetConsoleInformation
	@discussion Returns information about all console users on the system.
	@param store An SCDynamicStoreRef that should be used for communication
		with the server.
		If NULL, a temporary session will be used.
	@param user A pointer to a character buffer containing the name of
		the current/primary "Console" session. If NULL, any current
		"Console" session information will be reset.
	@param uid The user ID of the current/primary "Console" user.
	@param gid The group ID of the current/primary "Console" user.
	@param sessions An array of dictionaries containing information about
		each console session on the system; NULL if no sessions are
		defined.

		The contents of this array should match that of the CoreGraphics
		CGSCopySessionList() SPI.

	@result A boolean indicating the success (or failure) of the call.
 */
Boolean
SCDynamicStoreSetConsoleInformation	(
					SCDynamicStoreRef	store,
					const char		*user,
					uid_t			uid,
					gid_t			gid,
					CFArrayRef		sessions
					)			__OSX_AVAILABLE_STARTING(__MAC_10_3,__IPHONE_NA);

/*!
	@function SCDynamicStoreSetConsoleUser
	@discussion Sets the name, user ID, and group ID of the currently
		logged in user.
	@param store An SCDynamicStoreRef that should be used for communication
		with the server.
		If NULL, a temporary session will be used.
	@param user A pointer to a character buffer containing the name of
		the current "Console" user. If NULL, any current "Console"
		user information will be reset.
	@param uid The user ID of the current "Console" user.
	@param gid The group ID of the current "Console" user.
	@result A boolean indicating the success (or failure) of the call.
 */
Boolean
SCDynamicStoreSetConsoleUser		(
					SCDynamicStoreRef	store,
					const char		*user,
					uid_t			uid,
					gid_t			gid
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_NA);

__END_DECLS

#endif /* _SCDYNAMICSTORESETSPECIFICPRIVATE_H */

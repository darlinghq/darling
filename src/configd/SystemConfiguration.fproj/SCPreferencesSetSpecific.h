/*
 * Copyright (c) 2000-2002, 2004, 2005, 2008 Apple Inc. All rights reserved.
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

#ifndef _SCPREFERENCESSETSPECIFIC_H
#ifdef	USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS
#include <SystemConfiguration/_SCPreferencesSetSpecific.h>
#else	/* USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS */
#define _SCPREFERENCESSETSPECIFIC_H

#include <Availability.h>
#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCPreferences.h>


/*!
	@header SCPreferencesSetSpecific
	@discussion The functions in the SCPreferencesSetSpecific API allow
		an application to set specific configuration information
		about the current system (for example, the computer or
		sharing name).

		To access configuration preferences, you must first establish
		a preferences session using the SCPreferencesCreate function.
 */


__BEGIN_DECLS

/*!
	@function SCPreferencesSetComputerName
	@discussion Updates the computer name preference.

		Note: To commit these changes to permanent storage you must
		call the SCPreferencesCommitChanges function.
		In addition, you must call the SCPreferencesApplyChanges
		function for the new name to become active.
	@param prefs The preferences session.
	@param name The computer name to be set.
	@param nameEncoding The encoding associated with the computer name.
	@result Returns TRUE if successful; FALSE otherwise.
 */
Boolean
SCPreferencesSetComputerName		(
					SCPreferencesRef	prefs,
					CFStringRef		name,
					CFStringEncoding	nameEncoding
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesSetLocalHostName
	@discussion Updates the local host name.

		Note: To commit these changes to permanent storage you must
		call the SCPreferencesCommitChanges function.
		In addition, you must call the SCPreferencesApplyChanges
		function for the new name to become active.
	@param prefs The preferences session.
	@param name The local host name to be set.

	Note: this string must conform to the naming conventions of a DNS host
		name as specified in RFC 1034 (section 3.5).
	@result Returns TRUE if successful; FALSE otherwise.
 */
Boolean
SCPreferencesSetLocalHostName		(
					SCPreferencesRef	prefs,
					CFStringRef		name
					)			__OSX_AVAILABLE_STARTING(__MAC_10_2,__IPHONE_2_0/*SPI*/);

__END_DECLS

#endif	/* USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS */
#endif /* _SCPREFERENCESSETSPECIFIC_H */

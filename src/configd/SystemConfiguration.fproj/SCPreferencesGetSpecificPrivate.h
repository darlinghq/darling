/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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

#ifndef _SCPREFERENCESGETSPECIFICPRIVATE_H
#define _SCPREFERENCESGETSPECIFICPRIVATE_H

#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCPreferences.h>


/*!
	@header SCPreferencesGetSpecificPrivate
	@discussion The functions in the SCPreferencesGetSpecificPrivate API allow
		an application to get specific configuration information
		about the current system (for example, the host name).

		To access configuration preferences, you must first establish
		a preferences session using the SCPreferencesCreate function.
 */


__BEGIN_DECLS

/*!
	@function SCPreferencesGetHostName
	@discussion Gets the host name preference.
	@param prefs The preferences session.
	@result name The host name to be set;
		NULL if the name has not been set or if an error was encountered.
 */
CFStringRef
SCPreferencesGetHostName		(
					SCPreferencesRef	prefs
					);

CFStringRef
_SCPreferencesCopyLocalHostName		(
					SCPreferencesRef	prefs
					);

CFStringRef
_SCPreferencesCopyComputerName		(
					SCPreferencesRef	prefs,
					CFStringEncoding	*nameEncoding
					);
__END_DECLS

#endif /* _SCPREFERENCESGETSPECIFICPRIVATE_H */

/*
 * Copyright (c) 2000, 2001, 2004, 2005, 2008 Apple Inc. All rights reserved.
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

#ifndef _SCPREFERENCESPATH_H
#ifdef	USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS
#include <SystemConfiguration/_SCPreferencesPath.h>
#else	/* USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS */
#define _SCPREFERENCESPATH_H

#include <Availability.h>
#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCPreferences.h>


/*!
	@header SCPreferencesPath
	@discussion The SCPreferencesPath API allows an application to
		load and store XML configuration data in a controlled
		manner and provide the necessary notifications to other
		applications that need to be aware of configuration
		changes.

		The functions in the SCPreferencesPath API make certain
		assumptions about the layout of the preferences data.
		These functions view the data as a collection of dictionaries
		of key-value pairs and an associated path name.
		The root path ("/") identifies the top-level dictionary.
		Additional path components specify the keys for subdictionaries.

		For example, the following dictionary can be accessed via
		two paths.  The root ("/") path would return a dictionary
		with all keys and values.  The path "/path1" would only
		return the dictionary with the "key3" and "key4" properties.

	<pre>
	@textblock
	<dict>
		<key>key1</key>
		<string>val1</string>
		<key>key2</key>
		<string>val2</string>
		<key>path1</key>
		<dict>
			<key>key3</key>
			<string>val3</string>
			<key>key4</key>
			<string>val4</string>
		</dict>
	</dict>
	@/textblock
	</pre>

	Each dictionary can also include the kSCResvLink ("__LINK__") key.
	The value associated with this key is interpreted as a link to
	another path.  If this key is present, a call to the
	SCPreferencesPathGetValue function returns the dictionary
	specified by the link.
 */


__BEGIN_DECLS

/*!
	@function SCPreferencesPathCreateUniqueChild
	@discussion Creates a new path component within the dictionary
		hierarchy.
	@param prefs The preferences session.
	@param prefix A string that represents the parent path.
	@result Returns a string representing the new (unique) child path; NULL
		if the specified path does not exist.
 */
CFStringRef
SCPreferencesPathCreateUniqueChild	(
					SCPreferencesRef	prefs,
					CFStringRef		prefix
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesPathGetValue
	@discussion Returns the dictionary associated with the specified
		path.
	@param prefs The preferences session.
	@param path A string that represents the path to be returned.
	@result Returns the dictionary associated with the specified path; NULL
		if the path does not exist.
 */
CFDictionaryRef
SCPreferencesPathGetValue		(
					SCPreferencesRef	prefs,
					CFStringRef		path
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesPathGetLink
	@discussion Returns the link (if one exists) associated with the
		specified path.
	@param prefs The preferences session.
	@param path A string that represents the path to be returned.
	@result Returns the dictionary associated with the specified path; NULL
		if the path is not a link or does not exist.
 */
CFStringRef
SCPreferencesPathGetLink		(
					SCPreferencesRef	prefs,
					CFStringRef		path
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesPathSetValue
	@discussion Associates a dictionary with the specified path.
	@param prefs The preferences session.
	@param path A string that represents the path to be updated.
	@param value A dictionary that represents the data to be
		stored at the specified path.
	@result Returns TRUE if successful; FALSE otherwise.
 */
Boolean
SCPreferencesPathSetValue		(
					SCPreferencesRef	prefs,
					CFStringRef		path,
					CFDictionaryRef		value
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesPathSetLink
	@discussion Associates a link to a second dictionary at the
		specified path.
	@param prefs The preferences session.
	@param path A string that represents the path to be updated.
	@param link A string that represents the link to be stored
		at the specified path.
	@result Returns TRUE if successful; FALSE otherwise.
 */
Boolean
SCPreferencesPathSetLink		(
					SCPreferencesRef	prefs,
					CFStringRef		path,
					CFStringRef		link
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesPathRemoveValue
	@discussion Removes the data associated with the specified path.
	@param prefs The preferences session.
	@param path A string that represents the path to be returned.
	@result Returns TRUE if successful; FALSE otherwise.
 */
Boolean
SCPreferencesPathRemoveValue		(
					SCPreferencesRef	prefs,
					CFStringRef		path
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

__END_DECLS

#endif	/* USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS */
#endif /* _SCPREFERENCESPATH_H */

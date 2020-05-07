/*
 * Copyright (c) 2000-2004 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef __USERNOTIFICATION_KUNCUSERNOTIFICATIONS_H
#define __USERNOTIFICATION_KUNCUSERNOTIFICATIONS_H

#include <sys/cdefs.h>


#include <mach/message.h>
#include <mach/kern_return.h>
#include <UserNotification/UNDTypes.h>

__BEGIN_DECLS

/*
 * non blocking notice call.
 */
kern_return_t
KUNCUserNotificationDisplayNotice(
	int             noticeTimeout,
	unsigned        flags,
	char            *iconPath,
	char            *soundPath,
	char            *localizationPath,
	char            *alertHeader,
	char            *alertMessage,
	char            *defaultButtonTitle) __attribute__((deprecated));

/*
 * ***BLOCKING*** alert call, returned int value corresponds to the
 * pressed button, spin this off in a thread only, or expect your kext to block.
 */
kern_return_t
KUNCUserNotificationDisplayAlert(
	int             alertTimeout,
	unsigned        flags,
	char            *iconPath,
	char            *soundPath,
	char            *localizationPath,
	char            *alertHeader,
	char            *alertMessage,
	char            *defaultButtonTitle,
	char            *alternateButtonTitle,
	char            *otherButtonTitle,
	unsigned        *responseFlags) __attribute__((deprecated));


/*
 * Execute a userland executable with the given path, user and type
 */

#define kOpenApplicationPath    0       /* essentially executes the path */
#define kOpenPreferencePanel    1       /* runs the preferences with the foo.preference opened.  foo.preference must exist in /System/Library/Preferences */
#define kOpenApplication        2       /* essentially runs /usr/bin/open on the passed in application name */


#define kOpenAppAsRoot          0
#define kOpenAppAsConsoleUser   1

kern_return_t
KUNCExecute(
	char    *executionPath,
	int     openAsUser,
	int     pathExecutionType) __attribute__((deprecated));


/* KUNC User Notification XML Keys
 *
 * These are the keys used in the xml plist file passed in to the
 * KUNCUserNotitificationDisplayFrom* calls
 *
 * KUNC Notifications are completely dependent on CFUserNotifications in
 * user land.  The same restrictions apply, including the number of text fields,
 * types of information displayable, etc.
 *
 *  Key			Type
 * Header			string (header displayed on dialog)
 *                              corresponds to kCFUserNotificationAlertHeaderKey
 *
 * Icon URL			string (url of the icon to display)
 *                              corresponds to kCFUserNotificationIconURLKey
 *
 * Sound URL			string (url of the sound to play on display)
 *                              corresponds to kCFUserNotificationSoundURLKey
 *
 * Localization URL		string (url of bundle to retrieve localization
 *				info from, using Localizable.strings files)
 *                              corresponds to kCFUserNotificationLocalizationURLKey
 *
 * Message			string (text of the message, can contain %@'s
 *				which are filled from tokenString passed in)
 *                              corresponds to kCFUserNotificationAlertMessageKey
 *
 * OK Button Title              string (title of the "main" button)
 *                              corresponds to kCFUserNotificationDefaultButtonTitleKey
 *
 * Alternate Button Title       string (title of the "alternate" button,  usually cancel)
 *                              corresponds to kCFUserNotificationAlternateButtonTitleKey
 *
 * Other Button Title	        string (title of the "other" button)
 *                              corresponds to kCFUserNotificationOtherButtonTitleKey
 *
 * Timeout			string (numeric, int - seconds until the dialog
 *				goes away on it's own)
 *
 * Alert Level			string (Stop, Notice, Alert)
 *
 * Blocking Message		string (numeric, 1 or 0 - if 1, the dialog will
 *				have no buttons)
 *
 * Text Field Strings		array of strings (each becomes a text field)
 *                              corresponds to kCFUserNotificationTextFieldTitlesKey
 *
 * Password Fields		array of strings (numeric - each indicates a
 *				pwd field)
 *
 * Popup Button Strings		array of strings (each entry becomes a popup
 *				button string)
 *
 * Radio Button Strings		array of strings (each becomes a radio button)
 *
 * Check Box Strings		array of strings (each becomes a check box)
 *                              corresponds to kCFUserNotificationCheckBoxTitlesKey
 *
 * Selected Radio		string (numeric - which radio is selected)
 *
 * Checked Boxes		array of strings (numeric - each indicates a
 *				checked field)
 *
 * Selected Popup		string (numeric - which popup entry is selected)
 *
 */

/*
 * Bundle Calls
 *
 *	Arguments
 *
 *	bundleIdentifier
 *		path to the actual bundle (not inside of it)
 *	        (i.e. "/System/Library/Extensions/Foo.kext")
 *		***NOTE***
 *		This WILL change soon to expect the CFBundleIdentifier instead of a bundle path
 *	fileName
 *		filename in bundle to retrive the xml from (i.e. "Messages")
 *	fileExtension
 *		if fileName has an extension, it goes here (i.e., "dict");
 *	messageKey
 *		name of the xml key in the dictionary in the file to retrieve
 *		the info from (i.e., "Error Message")
 *	tokenString
 *		a string in the form of "foo@bar" where each element is
 *		seperated by the @ character.  This string can be used to
 *		replace values of the form %@ in the message key in the provided
 *		dictionary in the xml plist
 *	specialKey
 *		user specified key for notification, use this to match return
 *		values with your requested notification, this value is passed
 *		back to the client in the callback pararmeter contextKey
 */

typedef uintptr_t KUNCUserNotificationID;

/*
 * Reponse value checking & default setting
 *
 * The reponse value returned in the response Flags of the
 * KUNCUserNotificationCallBack can be tested against the following
 * enum and 2 defines to determine the state.
 */

enum {
	kKUNCDefaultResponse        = 0,
	kKUNCAlternateResponse      = 1,
	kKUNCOtherResponse          = 2,
	kKUNCCancelResponse         = 3
};

#define KUNCCheckBoxChecked(i)  (1 << (8 + i))   /* can be used for radio's too */
#define KUNCPopUpSelection(n)   (n << 24)

/*
 * Callback function for KUNCNotifications
 */
typedef void
(*KUNCUserNotificationCallBack)(
	int             contextKey,
	int             responseFlags,
	const void      *xmlData);

/*
 * Get a notification ID
 */
KUNCUserNotificationID KUNCGetNotificationID(void) __attribute__((deprecated));

/* This function currently requires a bundle path, which kexts cannot currently get.  In the future, the CFBundleIdentiofier of the kext will be pass in in place of the bundlePath. */

kern_return_t
KUNCUserNotificationDisplayFromBundle(
	KUNCUserNotificationID          notificationID,
	char                            *bundleIdentifier,
	char                            *fileName,
	char                            *fileExtension,
	char                            *messageKey,
	char                            *tokenString,
	KUNCUserNotificationCallBack    callback,
	int                             contextKey) __attribute__((deprecated));


kern_return_t
KUNCUserNotificationCancel(
	KUNCUserNotificationID  notification) __attribute__((deprecated));


__END_DECLS

#endif  /* __USERNOTIFICATION_KUNCUSERNOTIFICATIONS_H */

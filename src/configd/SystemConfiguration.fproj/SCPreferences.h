/*
 * Copyright (c) 2000, 2001, 2004, 2005, 2007-2010 Apple Inc. All rights reserved.
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

#ifndef _SCPREFERENCES_H
#ifdef	USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS
#include <SystemConfiguration/_SCPreferences.h>
#else	/* USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS */
#define _SCPREFERENCES_H

#include <Availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <dispatch/dispatch.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCDynamicStore.h>

#if	!TARGET_OS_IPHONE
#include <Security/Security.h>
#else	// !TARGET_OS_IPHONE
typedef const struct AuthorizationOpaqueRef *	AuthorizationRef;
#endif	// !TARGET_OS_IPHONE


/*!
	@header SCPreferences
	@discussion The SCPreferences API allows an application to load and
		store XML configuration data in a controlled manner and provide
		the necessary notifications to other applications that need to
		be aware of configuration changes.

		To access configuration preferences, you must first establish a
		preferences session using the SCPreferencesCreate function.
		To identify a specific set of preferences to access, you pass a
		value in the prefsID parameter.
		A NULL value indicates that the default system preferences are
		to be accessed.
		A string that starts with a leading "/" character specifies
		the absolute path to the file containing the preferences to
		be accessed.
		A string that does not start with a leading "/" character
		specifies a file relative to the default system preferences
		directory.

		When you are finished with the preferences session, use
		CFRelease to close it.
 */


/*!
	@typedef SCPreferencesRef
	@discussion This is the handle to an open preferences session for
		accessing system configuration preferences.
 */
typedef const struct __SCPreferences *	SCPreferencesRef;

/*!
	@enum SCPreferencesNotification
	@discussion Used with the SCPreferencesCallBack callback
		to describe the type of notification.
	@constant kSCPreferencesNotificationCommit Indicates when new
		preferences have been saved.
	@constant kSCPreferencesNotificationApply Key Indicates when a
		request has been made to apply the currently saved
		preferences to the active system configuration.
 */
enum {
	kSCPreferencesNotificationCommit	= 1<<0,	// __OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0/*SPI*/)
	kSCPreferencesNotificationApply		= 1<<1	// __OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0/*SPI*/)
};

typedef	uint32_t	SCPreferencesNotification;

/*!
	@typedef SCPreferencesContext
	Structure containing user-specified data and callbacks for SCPreferences.
	@field version The version number of the structure type being passed
		in as a parameter to the SCPreferencesSetCallback function.
		This structure is version 0.
	@field info A C pointer to a user-specified block of data.
	@field retain The callback used to add a retain for the info field.
		If this parameter is not a pointer to a function of the correct
		prototype, the behavior is undefined.
		The value may be NULL.
	@field release The calllback used to remove a retain previously added
		for the info field.
		If this parameter is not a pointer to a function of the
		correct prototype, the behavior is undefined.
		The value may be NULL.
	@field copyDescription The callback used to provide a description of
		the info field.
 */
typedef struct {
	CFIndex		version;
	void *		info;
	const void	*(*retain)(const void *info);
	void		(*release)(const void *info);
	CFStringRef	(*copyDescription)(const void *info);
} SCPreferencesContext;

/*!
	@typedef SCPreferencesCallBack
	@discussion Type of the callback function used when the
		preferences have been updated and/or applied.
	@param prefs The preferences session.
	@param notificationType The type of notification, such as changes
		committed, changes applied, etc.
	@param info A C pointer to a user-specified block of data.
 */
typedef void (*SCPreferencesCallBack)   (
					SCPreferencesRef		prefs,
					SCPreferencesNotification	notificationType,
					void				*info
					);


__BEGIN_DECLS

/*!
	@function SCPreferencesGetTypeID
	@discussion Returns the type identifier of all SCPreferences instances.
 */
CFTypeID
SCPreferencesGetTypeID			(void)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesCreate
	@discussion Initiates access to the per-system set of configuration
		preferences.
	@param allocator The CFAllocator that should be used to allocate
		memory for this preferences session.
		This parameter may be NULL in which case the current
		default CFAllocator is used.
		If this reference is not a valid CFAllocator, the behavior
		is undefined.
	@param name A string that describes the name of the calling
		process.
	@param prefsID A string that identifies the name of the
		group of preferences to be accessed or updated.
	@result Returns a reference to the new SCPreferences.
		You must release the returned value.
 */
SCPreferencesRef
SCPreferencesCreate			(
					CFAllocatorRef		allocator,
					CFStringRef		name,
					CFStringRef		prefsID
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);


/*!
	@function SCPreferencesCreateWithAuthorization
	@discussion Initiates access to the per-system set of configuration
		preferences.
	@param allocator The CFAllocator that should be used to allocate
		memory for this preferences session.
		This parameter may be NULL in which case the current
		default CFAllocator is used.
		If this reference is not a valid CFAllocator, the behavior
		is undefined.
	@param name A string that describes the name of the calling
		process.
	@param prefsID A string that identifies the name of the
		group of preferences to be accessed or updated.
	@param authorization An authorization reference that is used to
		authorize any access to the enhanced privileges needed
		to manage the preferences session.
	@result Returns a reference to the new SCPreferences.
		You must release the returned value.
 */
SCPreferencesRef
SCPreferencesCreateWithAuthorization	(
					CFAllocatorRef		allocator,
					CFStringRef		name,
					CFStringRef		prefsID,
					AuthorizationRef	authorization
					)			__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesLock
	@discussion Locks access to the configuration preferences.

		This function obtains exclusive access to the configuration
		preferences.  Clients attempting to obtain exclusive access
		to the preferences will either receive a kSCStatusPrefsBusy
		error or block waiting for the lock to be released.
	@param prefs The preferences session.
	@param wait A boolean flag indicating whether the calling process
		should block waiting for another process to complete its update
		operation and release its lock.
	@result Returns TRUE if the lock was obtained;
		FALSE if an error occurred.
 */
Boolean
SCPreferencesLock			(
					SCPreferencesRef	prefs,
					Boolean			wait
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesCommitChanges
	@discussion Commits changes made to the configuration preferences to
		persistent storage.

		This function commits any changes to permanent storage.
		Implicit calls to the SCPreferencesLock and SCPreferencesUnlock
		functions will be made if exclusive access has not already been
		established.

		Note: This routine commits changes to persistent storage.
		Call the SCPreferencesApplyChanges function to apply the
		changes to the running system.
	@param prefs The preferences session.
	@result Returns TRUE if the lock was obtained;
		FALSE if an error occurred.
 */
Boolean
SCPreferencesCommitChanges		(
					SCPreferencesRef	prefs
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesApplyChanges
	@discussion Requests that the currently stored configuration
		preferences be applied to the active configuration.
	@param prefs The preferences session.
	@result Returns TRUE if the lock was obtained;
		FALSE if an error occurred.
 */
Boolean
SCPreferencesApplyChanges		(
					SCPreferencesRef	prefs
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesUnlock
	@discussion Releases exclusive access to the configuration preferences.

		This function releases the exclusive access lock to the
		preferences.  Other clients will be now be able to establish
		exclusive access to the preferences.
	@param prefs The preferences session.
	@result Returns TRUE if the lock was obtained;
		FALSE if an error occurred.
 */
Boolean
SCPreferencesUnlock			(
					SCPreferencesRef	prefs
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesGetSignature
	@discussion Returns a sequence of bytes that can be used to determine
		if the saved configuration preferences have changed.
	@param prefs The preferences session.
	@result Returns a CFDataRef that reflects the signature of the configuration
		preferences at the time of the call to the SCPreferencesCreate function.
 */
CFDataRef
SCPreferencesGetSignature		(
					SCPreferencesRef	prefs
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesCopyKeyList
	@discussion Returns an array of currently defined preference keys.
	@param prefs The preferences session.
	@result Returns the list of keys.
		You must release the returned value.
 */
CFArrayRef
SCPreferencesCopyKeyList		(
					SCPreferencesRef	prefs
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesGetValue
	@discussion Returns the data associated with a preference key.

		This function retrieves data associated with the specified
		key.

		Note: To avoid inadvertantly reading stale data, first call
		the SCPreferencesLock function.
	@param prefs The preferences session.
	@param key The preference key to be returned.
	@result Returns the value associated with the specified preference key;
		NULL if no value was located.
 */
CFPropertyListRef
SCPreferencesGetValue			(
					SCPreferencesRef	prefs,
					CFStringRef		key
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesAddValue
	@discussion Adds data for a preference key.

		This function associates new data with the specified key.
		To commit these changes to permanent storage, a call must
		be made to the SCPreferencesCommitChanges function.
	@param prefs The preferences session.
	@param key The preference key to be updated.
	@param value The CFPropertyListRef object containing the
		value to be associated with the specified preference key.
	@result Returns TRUE if the value was added;
		FALSE if the key already exists or
		if an error occurred.
 */
Boolean
SCPreferencesAddValue			(
					SCPreferencesRef	prefs,
					CFStringRef		key,
					CFPropertyListRef	value
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesSetValue
	@discussion Updates the data associated with a preference key.

		This function adds or replaces the value associated with the
		specified key.  To commit these changes to permanent storage
		a call must be made to the SCPreferencesCommitChanges function.
	@param prefs The preferences session.
	@param key The preference key to be updated.
	@param value The CFPropertyListRef object containing the
		data to be associated with the specified preference key.
	@result Returns TRUE if the value was set;
		FALSE if an error occurred.
 */
Boolean
SCPreferencesSetValue			(
					SCPreferencesRef	prefs,
					CFStringRef		key,
					CFPropertyListRef	value
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesRemoveValue
	@discussion Removes the data associated with a preference key.

		This function removes the data associated with the specified
		key.  To commit these changes to permanent storage a call must
		be made to the SCPreferencesCommitChanges function.
	@param prefs The preferences session.
	@param key The preference key to be removed.
	@result Returns TRUE if the value was removed;
		FALSE if the key did not exist or if an error occurred.
 */
Boolean
SCPreferencesRemoveValue		(
					SCPreferencesRef	prefs,
					CFStringRef		key
					)			__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesSetCallback
	@discussion Assigns a callback to a preferences session.  The function
		is called when the changes to the preferences have been
		committed or applied.
	@param prefs The preferences session.
	@param callout The function to be called when the preferences have
		been changed or applied.
		If NULL, the current callback is removed.
	@param context The SCPreferencesContext associated with
		the callout.
	@result Returns TRUE if the notification client was successfully set.
 */
Boolean
SCPreferencesSetCallback		(
					SCPreferencesRef	prefs,
					SCPreferencesCallBack	callout,
					SCPreferencesContext	*context
					)			__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesScheduleWithRunLoop
	@discussion Schedule commit and apply notifications for the specified
		preferences session using the specified run loop and mode.
	@param prefs The preferences session.
	@param runLoop A reference to a run loop on which the notification
		should be scheduled.
		Must be non-NULL.
	@param runLoopMode The mode on which to schedule the notification.
		Must be non-NULL.
	@result Returns TRUE if the notifications are successfully scheduled;
		FALSE otherwise.
 */
Boolean
SCPreferencesScheduleWithRunLoop	(
					SCPreferencesRef	prefs,
					CFRunLoopRef		runLoop,
					CFStringRef		runLoopMode
					)			__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesUnscheduleFromRunLoop
	@discussion Unschedule commit and apply notifications for the specified
		preferences session from the specified run loop and mode.
	@param prefs The preferences session.
	@param runLoop A reference to a run loop from which the notification
		should be unscheduled.
		Must be non-NULL.
	@param runLoopMode The mode on which to unschedule the notification.
		Must be non-NULL.
	@result Returns TRUE if the notifications are successfully unscheduled;
		FALSE otherwise.
 */
Boolean
SCPreferencesUnscheduleFromRunLoop	(
					SCPreferencesRef	prefs,
					CFRunLoopRef		runLoop,
					CFStringRef		runLoopMode
					)			__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0/*SPI*/);

/*!
	@function SCPreferencesSetDispatchQueue
	@discussion Schedule commit and apply notifications for the specified
		preferences session.
	@param prefs The preferences session.
	@param queue The dispatch queue to run the callback function on.
	@result Returns TRUE if the notifications are successfully scheduled;
		FALSE otherwise.
 */
Boolean
SCPreferencesSetDispatchQueue		(
					 SCPreferencesRef	prefs,
					 dispatch_queue_t	queue
					 )			__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0/*SPI*/);

/*!
	@function SCPreferencesSynchronize
	@discussion Synchronizes accessed preferences with committed changes.

		Any references to preference values returned by calls to the
		SCPreferencesGetValue function are no longer valid unless they
		were explicitly retained or copied.  Any preference values
		that were updated (add, set, remove) but not committed will
		be discarded.
	@param prefs The preferences session.
 */
void
SCPreferencesSynchronize		(
					SCPreferencesRef	prefs
					)			__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0/*SPI*/);

__END_DECLS

#endif	/* USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS */
#endif /* _SCPREFERENCES_H */

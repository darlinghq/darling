/*
 * Copyright (c) 2000, 2001, 2003-2005, 2008-2010 Apple Inc. All rights reserved.
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

#ifndef _SCDYNAMICSTORE_H
#ifdef	USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS
#include <SystemConfiguration/_SCDynamicStore.h>
#else	/* USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS */
#define _SCDYNAMICSTORE_H

#include <Availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <dispatch/dispatch.h>
#include <CoreFoundation/CoreFoundation.h>


/*!
	@header SCDynamicStore
	@discussion The SCDynamicStore API provides access to the key-value
		pairs in the dynamic store of a running system.  The dynamic
		store contains, among other items, a copy of the configuration
		settings for the currently active set (which is sometimes
		refered to as the location) and information about the current
		network state.

		The functions in the SCDynamicStore API allow you to find
		key-value pairs, add or remove key-value pairs, add or change
		values, and request notifications.

		To use the functions of the SCDynamicStore API, you must first
		establish a dynamic store session using the SCDynamicStoreCreate
		function.  When you are finished with the session, use CFRelease
		to close it.
 */


/*!
	@typedef SCDynamicStoreRef
	@discussion This is the handle to an open a dynamic store session
		with the system configuration daemon.
 */
typedef const struct __SCDynamicStore *	SCDynamicStoreRef;

/*!
	@typedef SCDynamicStoreContext
	Structure containing user-specified data and callbacks for an
	SCDynamicStore session.
	@field version The version number of the structure type being passed
		in as a parameter to the SCDynamicStore creation function.
		This structure is version 0.
	@field info A C pointer to a user-specified block of data.
	@field retain The callback used to add a retain for the info field.
		If this parameter is not a pointer to a function of the correct
		prototype, the behavior is undefined.  The value may be NULL.
	@field release The calllback used to remove a retain previously added
		for the info field.  If this parameter is not a pointer to a
		function of the correct prototype, the behavior is undefined.
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
} SCDynamicStoreContext;

/*!
	@typedef SCDynamicStoreCallBack
	@discussion Type of callback function used when notification of
		changes to the dynamic store is delivered.
	@param store The dynamic store session.
	@param changedKeys The list of changed keys.

		The list includes any specific SCDynamicStore keys that
		changed (add, update, remove, notify) since the last call
		to SCDynamicStoreSetNotificationKeys or since the last
		notification callback. The list also includes any specific
		keys matching one of the pattern string(s) that changed.

		An empty list indicates that the SCDynamicStore server
		restarted and that any assumptions based on prior content
		of the SCDynamicStore should be disgarded.

	@param info A C pointer to a user-specified block of data.
 */
typedef void (*SCDynamicStoreCallBack)	(
					SCDynamicStoreRef	store,
					CFArrayRef		changedKeys,
					void			*info
					);


__BEGIN_DECLS

/*!
	@function SCDynamicStoreGetTypeID
	@discussion Returns the type identifier of all SCDynamicStore instances.
 */
CFTypeID
SCDynamicStoreGetTypeID			(void)				__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);


/*!
	@function SCDynamicStoreCreate
	@discussion Creates a new session used to interact with the dynamic
		store maintained by the System Configuration server.
	@param allocator The CFAllocator that should be used to allocate
		memory for the local dynamic store object.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@param name A string that describes the name of the calling
		process or plug-in of the caller.
	@param callout The function to be called when a watched value
		in the dynamic store is changed.
		A NULL value can be specified if no callouts are
		desired.
	@param context The SCDynamicStoreContext associated with the callout.
	@result Returns a reference to the new SCDynamicStore session.
		You must release the returned value.
 */
SCDynamicStoreRef
SCDynamicStoreCreate			(
					CFAllocatorRef			allocator,
					CFStringRef			name,
					SCDynamicStoreCallBack		callout,
					SCDynamicStoreContext		*context
					)				__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreCreateWithOptions
	@discussion Creates a new session used to interact with the dynamic
		store maintained by the System Configuration server.
	@param allocator The CFAllocator that should be used to allocate
		memory for the local dynamic store object.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@param name A string that describes the name of the calling
		process or plug-in of the caller.
	@param storeOptions A CFDictionary containing options for the
		dynamic store session (such as whether all keys added or set
		into the dynamic store should be per-session keys).

		Currently available options include:

		<TABLE BORDER>
		<TR>
			<TH>key</TD>
			<TH>value</TD>
		</TR>
		<TR>
			<TD>kSCDynamicStoreUseSessionKeys</TD>
			<TD>CFBooleanRef</TD>
		</TR>
		</TABLE>

		A NULL value can be specified if no options are desired.
	@param callout The function to be called when a watched value
		in the dynamic store is changed.
		A NULL value can be specified if no callouts are
		desired.
	@param context The SCDynamicStoreContext associated with the callout.
	@result Returns a reference to the new SCDynamicStore session.
		You must release the returned value.
 */
SCDynamicStoreRef
SCDynamicStoreCreateWithOptions		(
					CFAllocatorRef			allocator,
					CFStringRef			name,
					CFDictionaryRef			storeOptions,
					SCDynamicStoreCallBack		callout,
					SCDynamicStoreContext		*context
					)				__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0/*SPI*/);

extern const CFStringRef	kSCDynamicStoreUseSessionKeys		__OSX_AVAILABLE_STARTING(__MAC_10_4,__IPHONE_2_0/*SPI*/);	/* CFBoolean */

/*!
	@function SCDynamicStoreCreateRunLoopSource
	@discussion Creates a CFRunLoopSource object that can be added to the
		application's run loop.  All dynamic store notifications are
		delivered using this run loop source.
	@param allocator The CFAllocator that should be used to allocate
		memory for this run loop source.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@param store A reference to the dynamic store session.
	@param order On platforms which support it, for source versions
		which support it, this parameter determines the order in
		which the sources which are ready to be processed are
		handled. A lower order number causes processing before
		higher order number sources. It is inadvisable to depend
		on the order number for any architectural or design aspect
		of code. In the absence of any reason to do otherwise,
		zero should be used.
	@result A reference to the new CFRunLoopSource.
		You must release the returned value.

 */
CFRunLoopSourceRef
SCDynamicStoreCreateRunLoopSource	(
					CFAllocatorRef			allocator,
					SCDynamicStoreRef		store,
					CFIndex				order
					)				__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreSetDispatchQueue
	@discussion Initiates notifications for the Notification
		Keys in store to the callback contained in store.
	@param store A reference to the dynamic store session.
	@param queue The dispatch queue to run the callback function on.
		Pass NULL to disable notifications, and release the queue.
	@result Returns TRUE on success, FALSE on failure.

 */
Boolean
SCDynamicStoreSetDispatchQueue		(
					SCDynamicStoreRef		store,
					dispatch_queue_t		queue
					)				__OSX_AVAILABLE_STARTING(__MAC_10_6,__IPHONE_4_0/*SPI*/);

/*!
	@function SCDynamicStoreCopyKeyList
	@discussion Returns an array of CFString keys representing the
		current dynamic store entries that match a specified pattern.
	@param store The dynamic store session.
	@param pattern A regex(3) regular expression pattern
		used to match the dynamic store keys.
	@result Returns the list of matching keys; NULL if an error was
		encountered.
		You must release the returned value.
 */
CFArrayRef
SCDynamicStoreCopyKeyList		(
					SCDynamicStoreRef		store,
					CFStringRef			pattern
					)				__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreAddValue
	@discussion Adds the key-value pair to the dynamic store if no
		such key already exists.
	@param store The dynamic store session.
	@param key The key of the value to add to the dynamic store.
	@param value The value to add to the dynamic store.
	@result Returns TRUE if the key was added; FALSE if the key was already
		present in the dynamic store or if an error was encountered.
 */
Boolean
SCDynamicStoreAddValue			(
					SCDynamicStoreRef		store,
					CFStringRef			key,
					CFPropertyListRef		value
					)				__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreAddTemporaryValue
	@discussion Temporarily adds the key-value pair to the dynamic store
		if no such key already exists.  Unless the key is updated by another
		session, the key-value pair will be removed automatically when the
		session is closed.
	@param store The dynamic store session.
	@param key The key of the value to add to the dynamic store.
	@param value The value to add to the dynamic store.
	@result Returns TRUE if the key was added; FALSE if the key was already
		present in the dynamic store or if an error was encountered.
 */
Boolean
SCDynamicStoreAddTemporaryValue		(
					SCDynamicStoreRef		store,
					CFStringRef			key,
					CFPropertyListRef		value
					)				__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreCopyValue
	@discussion Gets the value of the specified key from the dynamic store.
	@param store The dynamic store session.
	@param key The key associated with the value you want to get.
	@result Returns the value from the dynamic store that is associated with the given
		key; NULL if no value was located or an error was encountered.
		You must release the returned value.
 */
CFPropertyListRef
SCDynamicStoreCopyValue			(
					SCDynamicStoreRef		store,
					CFStringRef			key
					)				__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreCopyMultiple
	@discussion Gets the values of multiple keys in the dynamic store.
	@param store The dynamic store session.
	@param keys The keys associated with the values you want to get; NULL if no specific
		keys are requested.
	@param patterns An array of regex(3) pattern strings used to match the keys; NULL
		if no key patterns are requested.
	@result Returns a dictionary containing the key-value pairs of specific keys and the
		key-value pairs of keys that matched the specified patterns;
		NULL if an error was encountered.
		You must release the returned value.
 */
CFDictionaryRef
SCDynamicStoreCopyMultiple		(
					SCDynamicStoreRef		store,
					CFArrayRef			keys,
					CFArrayRef			patterns
					)				__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreSetValue
	@discussion Adds or replaces a value in the dynamic store for
		the specified key.
	@param store The dynamic store session.
	@param key The key you want to set.
	@param value The value to add to or replace in the dynamic store.
	@result Returns TRUE if the key was updated; FALSE if an error was encountered.
 */
Boolean
SCDynamicStoreSetValue			(
					SCDynamicStoreRef		store,
					CFStringRef			key,
					CFPropertyListRef		value
					)				__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreSetMultiple
	@discussion Updates multiple values in the dynamic store.
	@param store The dynamic store session.
	@param keysToSet A dictionary of key-value pairs you want to set into the dynamic store.
	@param keysToRemove An array of keys you want to remove from the dynamic store.
	@param keysToNotify An array of keys to flag as changed (without changing their values).
	@result Returns TRUE if the dynamic store updates were successful; FALSE if an error was encountered.
 */
Boolean
SCDynamicStoreSetMultiple		(
					SCDynamicStoreRef		store,
					CFDictionaryRef			keysToSet,
					CFArrayRef			keysToRemove,
					CFArrayRef			keysToNotify
					)				__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreRemoveValue
	@discussion Removes the value of the specified key from the
		dynamic store.
	@param store The dynamic store session.
	@param key The key of the value you want to remove.
	@result Returns TRUE if the key was removed; FALSE if no value was
		located or an error was encountered.
 */
Boolean
SCDynamicStoreRemoveValue		(
					SCDynamicStoreRef		store,
					CFStringRef			key
					)				__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreNotifyValue
	@discussion Triggers a notification to be delivered for the
		specified key in the dynamic store.
	@param store The dynamic store session.
	@param key The key that should be flagged as changed.  Any dynamic store sessions
		that are monitoring this key will received a notification.  Note that the
		key's value is not updated.
	@result Returns TRUE if the notification was processed; FALSE if an error was encountered.
 */
Boolean
SCDynamicStoreNotifyValue		(
					SCDynamicStoreRef		store,
					CFStringRef			key
					)				__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreSetNotificationKeys
	@discussion Specifies a set of specific keys and key patterns
		that should be monitored for changes.
	@param store The dynamic store session being watched.
	@param keys An array of keys to be monitored; NULL if no specific keys
		are to be monitored.
	@param patterns An array of regex(3) pattern strings used to match keys to be monitored;
		NULL if no key patterns are to be monitored.
	@result Returns TRUE if the set of notification keys and patterns was successfully
		updated; FALSE if an error was encountered.
 */
Boolean
SCDynamicStoreSetNotificationKeys	(
					SCDynamicStoreRef		store,
					CFArrayRef			keys,
					CFArrayRef			patterns
					)				__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

/*!
	@function SCDynamicStoreCopyNotifiedKeys
	@discussion Returns an array of CFString keys representing the
		dynamic store entries that have changed since this
		function was last called.  If possible, your application should
		use the notification functions instead of polling for the list
		of changed keys returned by this function.
	@param store The dynamic store session.
	@result Returns the list of changed keys;
		NULL if an error was encountered.
		You must release the returned value.
 */
CFArrayRef
SCDynamicStoreCopyNotifiedKeys		(
					SCDynamicStoreRef		store
					)				__OSX_AVAILABLE_STARTING(__MAC_10_1,__IPHONE_2_0/*SPI*/);

__END_DECLS

#endif	/* USE_SYSTEMCONFIGURATION_PRIVATE_HEADERS */
#endif	/* _SCDYNAMICSTORE_H */

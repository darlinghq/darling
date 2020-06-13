/*
 * Copyright (c) 2017-2019 Apple Inc. All rights reserved.
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
 * InterfaceNamerControlPrefs.c
 * - definitions for accessing InterfaceNamer control preferences
 */

/*
 * Modification History
 *
 * January 12, 2017	Allan Nathanson (ajn@apple.com)
 * - created
 */

#include <TargetConditionals.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include <SystemConfiguration/scprefs_observer.h>
#include "InterfaceNamerControlPrefs.h"

/*
 * kInterfaceNamerControlPrefsID
 * - identifies the InterfaceNamer preferences file that contains 'AllowNewInterfaces'
 */
#define kInterfaceNamerControlPrefsIDStr	"com.apple.InterfaceNamer.control.plist"
#define kInterfaceNamerControlPrefsID		CFSTR(kInterfaceNamerControlPrefsIDStr)

/*
 * kAllowNewInterfaces
 * - indicates whether InterfaceNamer is allowed to create new interfaces
 *   while the screen is locked or not
 */
#define kAllowNewInterfaces			CFSTR("AllowNewInterfaces")

static SCPreferencesRef				S_prefs;
static InterfaceNamerControlPrefsCallBack	S_callback;

static SCPreferencesRef
InterfaceNamerControlPrefsGet(void)
{
	if (S_prefs == NULL) {
		InterfaceNamerControlPrefsInit(NULL, NULL);
	}

	return (S_prefs);
}

static void
prefs_changed(void * arg)
{
#pragma unused(arg)
	/* get the current value */
	if (S_callback != NULL) {
		(*S_callback)(S_prefs);
	}

	return;
}

#if	TARGET_OS_IPHONE
/*
 * kInterfaceNamerControlManangedPrefsID
 * - identifies the location of the managed preferences file
 */
#define kManagedPrefsDirStr			"/Library/Managed Preferences/mobile/"
#define kInterfaceNamerControlManagedPrefsID	CFSTR(kManagedPrefsDirStr	\
							kInterfaceNamerControlPrefsIDStr)
static SCPreferencesRef				S_managed_prefs;

static SCPreferencesRef
InterfaceNamerControlManagedPrefsGet(void)
{
	if (S_managed_prefs == NULL) {
		CFMutableDictionaryRef	options;

		options = CFDictionaryCreateMutable(NULL,
						    0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);
		CFDictionarySetValue(options, kSCPreferencesOptionRemoveWhenEmpty, kCFBooleanTrue);
		S_managed_prefs = SCPreferencesCreateWithOptions(NULL,
								 CFSTR("InterfaceNamerControlPrefs"),
								 kInterfaceNamerControlManagedPrefsID,
								 NULL,
								 options);
		CFRelease(options);
	}
	return (S_managed_prefs);
}

static void
enable_prefs_observer(CFRunLoopRef runloop)
{
	CFRunLoopSourceContext 	context;
	dispatch_queue_t	queue;
	CFRunLoopSourceRef	source;

	memset(&context, 0, sizeof(context));
	context.perform = prefs_changed;
	source = CFRunLoopSourceCreate(kCFAllocatorDefault, 0, &context);
	CFRunLoopAddSource(runloop, source, kCFRunLoopCommonModes);
	queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
	_scprefs_observer_watch(scprefs_observer_type_global,
				kInterfaceNamerControlPrefsIDStr,
				queue,
				^{
		if (source != NULL) {
			CFRunLoopSourceSignal(source);
			if (runloop != NULL) {
				CFRunLoopWakeUp(runloop);
			}
		};
	});
	return;
}

#else	/* TARGET_OS_IPHONE */

static void
enable_prefs_observer(CFRunLoopRef runloop)
{
#pragma unused(runloop)
	return;
}

#endif	/* TARGET_OS_IPHONE */

static void
InterfaceNamerControlPrefsChanged(SCPreferencesRef		prefs,
				  SCPreferencesNotification	type,
				  void				*info)
{
#pragma unused(prefs)
#pragma unused(type)
#pragma unused(info)
    prefs_changed(NULL);
    return;
}

__private_extern__ SCPreferencesRef
InterfaceNamerControlPrefsInit(CFRunLoopRef				runloop,
			       InterfaceNamerControlPrefsCallBack	callback)
{
	CFMutableDictionaryRef	options;

	options = CFDictionaryCreateMutable(NULL,
					    0,
					    &kCFTypeDictionaryKeyCallBacks,
					    &kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(options, kSCPreferencesOptionRemoveWhenEmpty, kCFBooleanTrue);
	S_prefs = SCPreferencesCreateWithOptions(NULL,
						 CFSTR("InterfaceNamerControlPrefs"),
						 kInterfaceNamerControlPrefsID,
						 NULL,
						 options);
	CFRelease(options);

	if ((runloop != NULL) && (callback != NULL)) {
		S_callback = callback;
		if (!SCPreferencesSetCallback(S_prefs, InterfaceNamerControlPrefsChanged, NULL)) {
			SC_log(LOG_NOTICE, "SCPreferencesSetCallBack() failed: %s", SCErrorString(SCError()));
			goto done;
		}

		if (!SCPreferencesScheduleWithRunLoop(S_prefs, runloop, kCFRunLoopCommonModes)) {
			SC_log(LOG_NOTICE, "SCPreferencesScheduleWithRunLoop() failed: %s", SCErrorString(SCError()));
			(void) SCPreferencesSetCallback(S_prefs, NULL, NULL);
		}

		enable_prefs_observer(runloop);
	}

    done :
	return (S_prefs);
}

static Boolean
InterfaceNamerControlPrefsSave(void)
{
	Boolean	saved	= FALSE;

	if (S_prefs != NULL) {
		saved = SCPreferencesCommitChanges(S_prefs);
		SCPreferencesSynchronize(S_prefs);
	}
	return (saved);
}

static CFBooleanRef
prefs_get_boolean(CFStringRef key)
{
	CFBooleanRef		b	= NULL;
	SCPreferencesRef	prefs;

#if	TARGET_OS_IPHONE
	prefs = InterfaceNamerControlManagedPrefsGet();
	if (prefs != NULL) {
		b = SCPreferencesGetValue(prefs, key);
		b = isA_CFBoolean(b);
		if (b != NULL) {
			return (b);
		}
	}
#endif	/* TARGET_OS_IPHONE */

	prefs = InterfaceNamerControlPrefsGet();
	if (prefs != NULL) {
		b = SCPreferencesGetValue(prefs, key);
		b = isA_CFBoolean(b);
	}
	return (b);
}

static void
prefs_set_boolean(CFStringRef key, CFBooleanRef b)
{
	SCPreferencesRef	prefs;

	prefs = InterfaceNamerControlPrefsGet();
	if (prefs != NULL) {
		if (isA_CFBoolean(b) == NULL) {
			SCPreferencesRemoveValue(prefs, key);
		}
		else {
			SCPreferencesSetValue(prefs, key, b);
		}
	}
	return;
}

static void
InterfaceNamerControlPrefsRefresh(void)
{
	if (S_prefs != NULL) {
		SCPreferencesSynchronize(S_prefs);
	}
#if	TARGET_OS_IPHONE
	if (S_managed_prefs != NULL) {
		SCPreferencesSynchronize(S_managed_prefs);
	}
#endif	/* TARGET_OS_IPHONE */
	return;
}

/**
 ** Get
 **/
__private_extern__ Boolean
InterfaceNamerControlPrefsAllowNewInterfaces(void)
{
	CFBooleanRef	b;
	Boolean		allow	= FALSE;

	b = prefs_get_boolean(kAllowNewInterfaces);
	if (b != NULL) {
		allow = CFBooleanGetValue(b);
	}
	/* flush the backing store */
	InterfaceNamerControlPrefsRefresh();
	return (allow);
}

/**
 ** Set
 **/
__private_extern__ Boolean
InterfaceNamerControlPrefsSetAllowNewInterfaces(Boolean allow)
{
	if (allow) {
		prefs_set_boolean(kAllowNewInterfaces, kCFBooleanTrue);
	} else {
		prefs_set_boolean(kAllowNewInterfaces, NULL);
	}
	return (InterfaceNamerControlPrefsSave());
}

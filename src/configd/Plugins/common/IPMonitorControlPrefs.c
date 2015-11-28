/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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
 * IPMonitorControlPrefs.c
 * - definitions for accessing IPMonitor control preferences
 */

/*
 * Modification History
 *
 * January 14, 2013	Dieter Siegmund (dieter@apple)
 * - created
 */

#include <SystemConfiguration/SCPreferences.h>
#include <SystemConfiguration/SCPrivate.h>
#include <SystemConfiguration/scprefs_observer.h>
#include <TargetConditionals.h>
#include "IPMonitorControlPrefs.h"

/*
 * kIPMonitorControlPrefsID
 * - identifies the IPMonitor preferences file that contains 'Verbose'
 */
#define kIPMonitorControlPrefsIDStr	"com.apple.IPMonitor.control.plist"
#define kIPMonitorControlPrefsID	CFSTR(kIPMonitorControlPrefsIDStr)

/*
 * kVerbose
 * - indicates whether IPMonitor is verbose or not
 */
#define kVerbose			CFSTR("Verbose")

static SCPreferencesRef			S_prefs;
static IPMonitorControlPrefsCallBack	S_callback;

static SCPreferencesRef
IPMonitorControlPrefsGet(void)
{
    if (S_prefs == NULL) {
	IPMonitorControlPrefsInit(NULL, NULL);
    }
    return (S_prefs);
}

static void
prefs_changed(__unused void * arg)
{
    /* get the current value */
    if (S_callback != NULL) {
	(*S_callback)(S_prefs);
    }
    return;
}

#if	TARGET_OS_IPHONE
/*
 * kIPMonitorControlManangedPrefsID
 * - identifies the location of the managed preferences file
 */
#define kManagedPrefsDirStr		"/Library/Managed Preferences/mobile/"
#define kIPMonitorControlManagedPrefsID	CFSTR(kManagedPrefsDirStr	\
					      kIPMonitorControlPrefsIDStr)
static SCPreferencesRef		S_managed_prefs;

static SCPreferencesRef
IPMonitorControlManagedPrefsGet(void)
{
    if (S_managed_prefs == NULL) {
	S_managed_prefs
	    = SCPreferencesCreate(NULL, CFSTR("IPMonitorControlPrefs"),
				  kIPMonitorControlManagedPrefsID);
    }
    return (S_managed_prefs);
}

static void
enable_prefs_observer(CFRunLoopRef runloop)
{
    CFRunLoopSourceContext 	context;
    dispatch_block_t		handler;
    dispatch_queue_t		queue;
    CFRunLoopSourceRef		source;

    bzero(&context, sizeof(context));
    context.perform = prefs_changed;
    source = CFRunLoopSourceCreate(kCFAllocatorDefault, 0, &context);
    CFRunLoopAddSource(runloop, source, kCFRunLoopCommonModes);
    queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    handler = ^{
	if (source != NULL) {
	    CFRunLoopSourceSignal(source);
	    if (runloop != NULL) {
		CFRunLoopWakeUp(runloop);
	    }
	};
    };
    _scprefs_observer_watch(scprefs_observer_type_global,
			    kIPMonitorControlPrefsIDStr,
			    queue, handler);
    return;
}

#else	/* TARGET_OS_IPHONE */

static void
enable_prefs_observer(CFRunLoopRef runloop)
{
    return;
}

#endif	/* TARGET_OS_IPHONE */

static void
IPMonitorControlPrefsChanged(SCPreferencesRef prefs,
			     SCPreferencesNotification type,
			     void * info)
{
    prefs_changed(NULL);
    return;
}

__private_extern__ SCPreferencesRef
IPMonitorControlPrefsInit(CFRunLoopRef runloop,
			  IPMonitorControlPrefsCallBack callback)
{
    S_prefs = SCPreferencesCreate(NULL, CFSTR("IPMonitorControlPrefs"),
				  kIPMonitorControlPrefsID);
    if (runloop != NULL && callback != NULL) {
	S_callback = callback;
	SCPreferencesSetCallback(S_prefs, IPMonitorControlPrefsChanged, NULL);
	SCPreferencesScheduleWithRunLoop(S_prefs, runloop,
					 kCFRunLoopCommonModes);
	enable_prefs_observer(runloop);
    }
    return (S_prefs);
}

static Boolean
IPMonitorControlPrefsSave(void)
{
    Boolean		saved = FALSE;

    if (S_prefs != NULL) {
	saved = SCPreferencesCommitChanges(S_prefs);
	SCPreferencesSynchronize(S_prefs);
    }
    return (saved);
}

static CFBooleanRef
prefs_get_boolean(CFStringRef key)
{
    CFBooleanRef	b = NULL;

#if	TARGET_OS_IPHONE
    b = SCPreferencesGetValue(IPMonitorControlManagedPrefsGet(), key);
    b = isA_CFBoolean(b);
#endif	/* TARGET_OS_IPHONE */
    if (b == NULL) {
	b = SCPreferencesGetValue(IPMonitorControlPrefsGet(), key);
	b = isA_CFBoolean(b);
    }
    return (b);
}

static void
prefs_set_boolean(CFStringRef key, CFBooleanRef b)
{
    SCPreferencesRef	prefs = IPMonitorControlPrefsGet();

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
IPMonitorControlPrefsRefresh(void)
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
IPMonitorControlPrefsIsVerbose(void)
{
    CFBooleanRef	b;
    Boolean		verbose = FALSE;

    b = prefs_get_boolean(kVerbose);
    if (b != NULL) {
	verbose = CFBooleanGetValue(b);
    }
    /* flush the backing store */
    IPMonitorControlPrefsRefresh();
    return (verbose);
}

/**
 ** Set
 **/
__private_extern__ Boolean
IPMonitorControlPrefsSetVerbose(Boolean verbose)
{
    if (verbose == FALSE) {
	prefs_set_boolean(kVerbose, NULL);
    }
    else {
	prefs_set_boolean(kVerbose, kCFBooleanTrue);
    }
    return (IPMonitorControlPrefsSave());
}

#ifdef TEST_IPMONITORCONTROLPREFS
int
main(int argc, char * argv[])
{
    Boolean	need_usage = FALSE;
    Boolean	success = FALSE;

    if (argc < 2) {
	need_usage = TRUE;
    }
    else if (strcasecmp(argv[1], "on") == 0) {
	success = IPMonitorControlPrefsSetVerbose(TRUE);
    }
    else if (strcasecmp(argv[1], "off") == 0) {
	success = IPMonitorControlPrefsSetVerbose(FALSE);
    }
    else {
	need_usage = TRUE;
    }
    if (need_usage) {
	fprintf(stderr, "usage: %s ( on | off )\n", argv[0]);
	exit(1);
    }
    if (success == FALSE) {
	fprintf(stderr, "failed to save prefs\n");
	exit(2);
    }
    exit(0);
    return (0);
}

#endif /* TEST_IPMONITORCONTROLPREFS */

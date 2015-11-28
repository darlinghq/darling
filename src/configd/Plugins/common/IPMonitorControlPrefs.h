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

#ifndef _EAP8021X_EAPOLCONTROLPREFS_H
#define _EAP8021X_EAPOLCONTROLPREFS_H

/*
 * IPMonitorControlPrefs.h
 * - definitions for accessing IPMonitor control preferences and being notified
 *   when they change
 */

/*
 * Modification History
 *
 * January 14, 2013	Dieter Siegmund (dieter@apple)
 * - created
 */
#include <CoreFoundation/CFRunLoop.h>
#include <SystemConfiguration/SCPreferences.h>

typedef void (*IPMonitorControlPrefsCallBack)(SCPreferencesRef prefs);

SCPreferencesRef
IPMonitorControlPrefsInit(CFRunLoopRef runloop,
			  IPMonitorControlPrefsCallBack callback);

Boolean
IPMonitorControlPrefsIsVerbose(void);

Boolean
IPMonitorControlPrefsSetVerbose(Boolean verbose);

#endif /* _COMMON_IPMONITORCONTROLPREFS_H */

/*
 * Copyright (c) 2000, 2001, 2003, 2004, 2006, 2009, 2011 Apple Inc. All rights reserved.
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
 * Modification History
 *
 * June 1, 2001			Allan Nathanson <ajn@apple.com>
 * - public API conversion
 *
 * June 2, 2000			Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#ifndef _S_SCD_H
#define _S_SCD_H

#include <sys/cdefs.h>


/*
 * keys in the "storeData" dictionary
 */

/*
 * data associated with a key
 */
#define	kSCDData	CFSTR("data")
/*
 * client session ids watching a key and, since we can possibly have
 * multiple regex keys which reference the key, a count of active
 * references
 */
#define	kSCDWatchers	CFSTR("watchers")
#define	kSCDWatcherRefs	CFSTR("watcherRefs")
/*
 * client session id for per-session keys.
 */
#define	kSCDSession	CFSTR("session")


/*
 * keys in the "sessionData" dictionary
 */

/*
 * the name of the calling application / plug-in
 */
#define	kSCDName	CFSTR("name")
/*
 * keys which have changed since last call to SCDNotifierGetChanges()
 */
#define	kSCDChangedKeys	CFSTR("changedKeys")
/*
 * keys which are to be removed when the session is closed
 */
#define	kSCDSessionKeys	CFSTR("sessionKeys")


extern CFMutableDictionaryRef	storeData;
extern CFMutableDictionaryRef	sessionData;
extern CFMutableDictionaryRef	patternData;
extern CFMutableSetRef		changedKeys;
extern CFMutableSetRef		deferredRemovals;
extern CFMutableSetRef		removedSessionKeys;
extern CFMutableSetRef		needsNotification;


__BEGIN_DECLS

int
__SCDynamicStoreOpen			(SCDynamicStoreRef	*store,
					 CFStringRef		name);
int
__SCDynamicStoreClose			(SCDynamicStoreRef	*store);

int
__SCDynamicStorePush			(void);

int
__SCDynamicStoreCopyKeyList		(SCDynamicStoreRef	store,
					 CFStringRef		prefix,
					 Boolean		isRegex,
					 CFArrayRef		*subKeys);

int
__SCDynamicStoreAddValue		(SCDynamicStoreRef	store,
					 CFStringRef		key,
					 CFDataRef		value);

int
__SCDynamicStoreCopyValue		(SCDynamicStoreRef	store,
					 CFStringRef		key,
					 CFDataRef		*value,
					 Boolean		internal);

int
__SCDynamicStoreCopyMultiple		(SCDynamicStoreRef	store,
					 CFArrayRef		keys,
					 CFArrayRef		patterns,
					 CFDictionaryRef	*values);

int
__SCDynamicStoreSetValue		(SCDynamicStoreRef	store,
					 CFStringRef		key,
					 CFDataRef		value,
					 Boolean		internal);

int
__SCDynamicStoreSetMultiple		(SCDynamicStoreRef	store,
					 CFDictionaryRef	keysToSet,
					 CFArrayRef		keysToRemove,
					 CFArrayRef		keysToNotify);

int
__SCDynamicStoreRemoveValue		(SCDynamicStoreRef	store,
					 CFStringRef		key,
					 Boolean		internal);

int
__SCDynamicStoreNotifyValue		(SCDynamicStoreRef	store,
					 CFStringRef		key,
					 Boolean		internal);

int
__SCDynamicStoreSnapshot		(SCDynamicStoreRef	store);

int
__SCDynamicStoreAddWatchedKey		(SCDynamicStoreRef	store,
					 CFStringRef		key,
					 Boolean		isRegex,
					 Boolean		internal);

int
__SCDynamicStoreRemoveWatchedKey	(SCDynamicStoreRef	store,
					 CFStringRef		key,
					 Boolean		isRegex,
					 Boolean		internal);

int
__SCDynamicStoreSetNotificationKeys	(SCDynamicStoreRef	store,
					 CFArrayRef		keys,
					 CFArrayRef		patterns);

int
__SCDynamicStoreCopyNotifiedKeys	(SCDynamicStoreRef	store,
					 CFArrayRef		*notifierKeys);

int
__SCDynamicStoreNotifyMachPort		(SCDynamicStoreRef	store,
					 mach_msg_id_t		msgid,
					 mach_port_t		port);

int
__SCDynamicStoreNotifyFileDescriptor	(SCDynamicStoreRef	store,
					 int32_t		identifier,
					 int			*fd);

int
__SCDynamicStoreNotifySignal		(SCDynamicStoreRef	store,
					 pid_t			pid,
					 int			sig);

int
__SCDynamicStoreNotifyCancel		(SCDynamicStoreRef	store);

void
_addWatcher				(CFNumberRef		sessionNum,
					 CFStringRef		watchedKey);

void
_removeWatcher				(CFNumberRef		sessionNum,
					 CFStringRef		watchedKey);

void
pushNotifications			(FILE			*_configd_trace);

__END_DECLS

#endif /* !_S_SCD_H */

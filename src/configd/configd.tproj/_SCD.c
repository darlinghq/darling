/*
 * Copyright (c) 2000, 2001, 2003-2005, 2009, 2011, 2012 Apple Inc. All rights reserved.
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


#include <unistd.h>

#include "configd.h"
#include "configd_server.h"
#include "session.h"


__private_extern__ CFMutableDictionaryRef	sessionData		= NULL;

__private_extern__ CFMutableDictionaryRef	storeData		= NULL;

__private_extern__ CFMutableDictionaryRef	patternData		= NULL;

__private_extern__ CFMutableSetRef		changedKeys		= NULL;

__private_extern__ CFMutableSetRef		deferredRemovals	= NULL;

__private_extern__ CFMutableSetRef		removedSessionKeys	= NULL;

__private_extern__ CFMutableSetRef		needsNotification	= NULL;


__private_extern__
void
_addWatcher(CFNumberRef sessionNum, CFStringRef watchedKey)
{
	CFDictionaryRef		dict;
	CFMutableDictionaryRef	newDict;
	CFArrayRef		watchers;
	CFMutableArrayRef	newWatchers;
	CFArrayRef		watcherRefs;
	CFMutableArrayRef	newWatcherRefs;
	CFIndex			i;
	int			refCnt;
	CFNumberRef		refNum;

	/*
	 * Get the dictionary associated with this key out of the store
	 */
	dict = CFDictionaryGetValue(storeData, watchedKey);
	if (dict) {
		newDict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
	} else {
		newDict = CFDictionaryCreateMutable(NULL,
						    0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);
	}

	/*
	 * Get the set of watchers out of the keys dictionary
	 */
	watchers    = CFDictionaryGetValue(newDict, kSCDWatchers);
	watcherRefs = CFDictionaryGetValue(newDict, kSCDWatcherRefs);
	if (watchers) {
		newWatchers    = CFArrayCreateMutableCopy(NULL, 0, watchers);
		newWatcherRefs = CFArrayCreateMutableCopy(NULL, 0, watcherRefs);
	} else {
		newWatchers    = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		newWatcherRefs = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}

	/*
	 * Add my session to the set of watchers
	 */
	i = CFArrayGetFirstIndexOfValue(newWatchers,
					CFRangeMake(0, CFArrayGetCount(newWatchers)),
					sessionNum);
	if (i == kCFNotFound) {
		/* if this is the first instance of this session watching this key */
		CFArrayAppendValue(newWatchers, sessionNum);
		refCnt = 1;
		refNum = CFNumberCreate(NULL, kCFNumberIntType, &refCnt);
		CFArrayAppendValue(newWatcherRefs, refNum);
		CFRelease(refNum);
	} else {
		/* if this is another instance of this session watching this key */
		refNum = CFArrayGetValueAtIndex(newWatcherRefs, i);
		CFNumberGetValue(refNum, kCFNumberIntType, &refCnt);
		refCnt++;
		refNum = CFNumberCreate(NULL, kCFNumberIntType, &refCnt);
		CFArraySetValueAtIndex(newWatcherRefs, i, refNum);
		CFRelease(refNum);
	}

	/*
	 * Update the keys dictionary
	 */
	CFDictionarySetValue(newDict, kSCDWatchers, newWatchers);
	CFRelease(newWatchers);
	CFDictionarySetValue(newDict, kSCDWatcherRefs, newWatcherRefs);
	CFRelease(newWatcherRefs);

	/*
	 * Update the store for this key
	 */
	CFDictionarySetValue(storeData, watchedKey, newDict);
	CFRelease(newDict);

#ifdef	DEBUG
	SCLog(_configd_verbose, LOG_DEBUG, CFSTR("  _addWatcher: %@, %@"), sessionNum, watchedKey);
#endif	/* DEBUG */

	return;
}


__private_extern__
void
_removeWatcher(CFNumberRef sessionNum, CFStringRef watchedKey)
{
	CFDictionaryRef		dict;
	CFMutableDictionaryRef	newDict;
	CFArrayRef		watchers;
	CFMutableArrayRef	newWatchers;
	CFArrayRef		watcherRefs;
	CFMutableArrayRef	newWatcherRefs;
	CFIndex			i;
	int			refCnt;
	CFNumberRef		refNum;

	/*
	 * Get the dictionary associated with this key out of the store
	 */
	dict = CFDictionaryGetValue(storeData, watchedKey);
	if ((dict == NULL) || (CFDictionaryContainsKey(dict, kSCDWatchers) == FALSE)) {
		/* key doesn't exist (isn't this really fatal?) */
#ifdef	DEBUG
		SCLog(_configd_verbose, LOG_DEBUG, CFSTR("  _removeWatcher: %@, %@, key not watched"), sessionNum, watchedKey);
#endif	/* DEBUG */
		return;
	}
	newDict = CFDictionaryCreateMutableCopy(NULL, 0, dict);

	/*
	 * Get the set of watchers out of the keys dictionary and
	 * remove this session from the list.
	 */
	watchers       = CFDictionaryGetValue(newDict, kSCDWatchers);
	newWatchers    = CFArrayCreateMutableCopy(NULL, 0, watchers);

	watcherRefs    = CFDictionaryGetValue(newDict, kSCDWatcherRefs);
	newWatcherRefs = CFArrayCreateMutableCopy(NULL, 0, watcherRefs);

	/* locate the session reference */
	i = CFArrayGetFirstIndexOfValue(newWatchers,
					CFRangeMake(0, CFArrayGetCount(newWatchers)),
					sessionNum);
	if (i == kCFNotFound) {
#ifdef	DEBUG
		SCLog(_configd_verbose, LOG_DEBUG, CFSTR("  _removeWatcher: %@, %@, session not watching"), sessionNum, watchedKey);
#endif	/* DEBUG */
		CFRelease(newDict);
		CFRelease(newWatchers);
		CFRelease(newWatcherRefs);
		return;
	}

	/* remove one session reference */
	refNum = CFArrayGetValueAtIndex(newWatcherRefs, i);
	CFNumberGetValue(refNum, kCFNumberIntType, &refCnt);
	if (--refCnt > 0) {
		refNum = CFNumberCreate(NULL, kCFNumberIntType, &refCnt);
		CFArraySetValueAtIndex(newWatcherRefs, i, refNum);
		CFRelease(refNum);
	} else {
		/* if this was the last reference */
		CFArrayRemoveValueAtIndex(newWatchers, i);
		CFArrayRemoveValueAtIndex(newWatcherRefs, i);
	}

	if (CFArrayGetCount(newWatchers) > 0) {
		/* if this key is still being "watched" */
		CFDictionarySetValue(newDict, kSCDWatchers, newWatchers);
		CFDictionarySetValue(newDict, kSCDWatcherRefs, newWatcherRefs);
	} else {
		/* no watchers left, remove the empty set */
		CFDictionaryRemoveValue(newDict, kSCDWatchers);
		CFDictionaryRemoveValue(newDict, kSCDWatcherRefs);
	}
	CFRelease(newWatchers);
	CFRelease(newWatcherRefs);

	if (CFDictionaryGetCount(newDict) > 0) {
		/* if this key is still active */
		CFDictionarySetValue(storeData, watchedKey, newDict);
	} else {
		/* no information left, remove the empty dictionary */
		CFDictionaryRemoveValue(storeData, watchedKey);
	}
	CFRelease(newDict);

#ifdef	DEBUG
	SCLog(_configd_verbose, LOG_DEBUG, CFSTR("  _removeWatcher: %@, %@"), sessionNum, watchedKey);
#endif	/* DEBUG */

	return;
}


#define N_QUICK	64


__private_extern__
void
pushNotifications(FILE *_configd_trace)
{
	CFIndex				notifyCnt;
	int				server;
	const void *			sessionsToNotify_q[N_QUICK];
	const void **			sessionsToNotify	= sessionsToNotify_q;
	SCDynamicStorePrivateRef	storePrivate;
	serverSessionRef		theSession;

	if (needsNotification == NULL)
		return;		/* if no sessions need to be kicked */

	notifyCnt = CFSetGetCount(needsNotification);
	if (notifyCnt > (CFIndex)(sizeof(sessionsToNotify_q) / sizeof(CFNumberRef)))
		sessionsToNotify = CFAllocatorAllocate(NULL, notifyCnt * sizeof(CFNumberRef), 0);
	CFSetGetValues(needsNotification, sessionsToNotify);
	while (--notifyCnt >= 0) {
		(void) CFNumberGetValue(sessionsToNotify[notifyCnt],
					kCFNumberIntType,
					&server);
		theSession = getSession(server);
		storePrivate = (SCDynamicStorePrivateRef)theSession->store;

		/*
		 * deliver notifications to client sessions
		 */
		if ((storePrivate->notifyStatus == Using_NotifierInformViaMachPort) &&
		    (storePrivate->notifyPort != MACH_PORT_NULL)) {
			/*
			 * Post notification as mach message
			 */
			if (_configd_trace != NULL) {
				SCTrace(TRUE, _configd_trace,
					CFSTR("%s : %5d : port = %d, msgid = %d\n"),
					"-->port",
					storePrivate->server,
					storePrivate->notifyPort,
					storePrivate->notifyPortIdentifier);
			}

			_SC_sendMachMessage(storePrivate->notifyPort, storePrivate->notifyPortIdentifier);
		}

		if ((storePrivate->notifyStatus == Using_NotifierInformViaFD) &&
		    (storePrivate->notifyFile >= 0)) {
			ssize_t		written;

			if (_configd_trace != NULL) {
				SCTrace(TRUE, _configd_trace,
					CFSTR("%s : %5d : fd = %d, msgid = %d\n"),
					"-->fd  ",
					storePrivate->server,
					storePrivate->notifyFile,
					storePrivate->notifyFileIdentifier);
			}

			written = write(storePrivate->notifyFile,
					&storePrivate->notifyFileIdentifier,
					sizeof(storePrivate->notifyFileIdentifier));
			if (written == -1) {
				if (errno == EWOULDBLOCK) {
#ifdef	DEBUG
					SCLog(_configd_verbose, LOG_DEBUG,
					      CFSTR("sorry, only one outstanding notification per session."));
#endif	/* DEBUG */
				} else {
#ifdef	DEBUG
					SCLog(_configd_verbose, LOG_DEBUG,
					      CFSTR("could not send notification, write() failed: %s"),
					      strerror(errno));
#endif	/* DEBUG */
					storePrivate->notifyFile = -1;
				}
			} else if (written != sizeof(storePrivate->notifyFileIdentifier)) {
#ifdef	DEBUG
				SCLog(_configd_verbose, LOG_DEBUG,
				      CFSTR("could not send notification, incomplete write()"));
#endif	/* DEBUG */
				storePrivate->notifyFile = -1;
			}
		}

		if ((storePrivate->notifyStatus == Using_NotifierInformViaSignal) &&
		    (storePrivate->notifySignal > 0)) {
			kern_return_t	status;
			pid_t		pid;
			/*
			 * Post notification as signal
			 */
			status = pid_for_task(storePrivate->notifySignalTask, &pid);
			if (status == KERN_SUCCESS) {
				if (_configd_trace != NULL) {
					SCTrace(TRUE, _configd_trace,
						CFSTR("%s : %5d : pid = %d, signal = sig%s (%d)\n"),
						"-->sig ",
						storePrivate->server,
						pid,
						sys_signame[storePrivate->notifySignal],
						storePrivate->notifySignal);
				}

				if (kill(pid, storePrivate->notifySignal) != 0) {
					if (errno != ESRCH) {
						SCLog(TRUE, LOG_ERR,
						      CFSTR("could not send sig%s to PID %d: %s"),
						      sys_signame[storePrivate->notifySignal],
						      pid,
						      strerror(errno));
					}
				}
			} else {
				mach_port_type_t	pt;

				__MACH_PORT_DEBUG(TRUE, "*** pushNotifications pid_for_task failed: releasing task", storePrivate->notifySignalTask);
				if (mach_port_type(mach_task_self(), storePrivate->notifySignalTask, &pt) == KERN_SUCCESS) {
					if ((pt & MACH_PORT_TYPE_DEAD_NAME) != 0) {
						SCLog(TRUE, LOG_ERR, CFSTR("pushNotifications pid_for_task() failed: %s"), mach_error_string(status));
					}
				} else {
					SCLog(TRUE, LOG_ERR, CFSTR("pushNotifications mach_port_type() failed: %s"), mach_error_string(status));
				}

				/* don't bother with any more attempts */
				(void) mach_port_deallocate(mach_task_self(), storePrivate->notifySignalTask);
				storePrivate->notifySignal     = 0;
				storePrivate->notifySignalTask = TASK_NULL;
			}
	       }
	}
	if (sessionsToNotify != sessionsToNotify_q) CFAllocatorDeallocate(NULL, sessionsToNotify);

	/*
	 * this list of notifications have been posted, wait for some more.
	 */
	CFRelease(needsNotification);
	needsNotification = NULL;

	return;
}

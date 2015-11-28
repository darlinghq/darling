/*
 * Copyright (c) 2000-2004, 2006, 2009, 2011 Apple Inc. All rights reserved.
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
 * March 24, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <unistd.h>
#include <sys/types.h>

#include "configd.h"
#include "configd_server.h"
#include "session.h"

__private_extern__
int
__SCDynamicStoreNotifySignal(SCDynamicStoreRef store, pid_t pid, int sig)
{
	SCDynamicStorePrivateRef	storePrivate = (SCDynamicStorePrivateRef)store;
	CFStringRef			sessionKey;
	CFDictionaryRef			info;

	if (storePrivate->notifyStatus != NotifierNotRegistered) {
		/* sorry, you can only have one notification registered at once */
		return kSCStatusNotifierActive;
	}

	if (pid == getpid()) {
		/* sorry, you can't request that configd be signalled */
		return kSCStatusInvalidArgument;
	}

	if ((sig <= 0) || (sig > NSIG)) {
		/* sorry, you must specify a valid signal */
		return kSCStatusInvalidArgument;
	}

	/* push out a notification if any changes are pending */
	sessionKey = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), storePrivate->server);
	info = CFDictionaryGetValue(sessionData, sessionKey);
	CFRelease(sessionKey);
	if (info && CFDictionaryContainsKey(info, kSCDChangedKeys)) {
		CFNumberRef	sessionNum;

		if (needsNotification == NULL)
			needsNotification = CFSetCreateMutable(NULL,
							       0,
							       &kCFTypeSetCallBacks);

		sessionNum = CFNumberCreate(NULL, kCFNumberIntType, &storePrivate->server);
		CFSetAddValue(needsNotification, sessionNum);
		CFRelease(sessionNum);
	}

	return kSCStatusOK;
}


__private_extern__
kern_return_t
_notifyviasignal(mach_port_t	server,
		 task_t		task,
		 int		sig,
		 int		*sc_status)
{
	serverSessionRef		mySession  = getSession(server);
	pid_t				pid;
	kern_return_t			status;
	SCDynamicStorePrivateRef	storePrivate;

	if (mySession == NULL) {
		/* sorry, you must have an open session to play */
		*sc_status = kSCStatusNoStoreSession;
		if (task != TASK_NULL) {
			(void) mach_port_deallocate(mach_task_self(), task);
		}
		return KERN_SUCCESS;
	}
	storePrivate = (SCDynamicStorePrivateRef)mySession->store;

	if (task != TASK_NULL) {
		status = pid_for_task(task, &pid);
		if (status != KERN_SUCCESS) {
			/* could not determine pid for task */
			*sc_status = kSCStatusFailed;
			(void) mach_port_deallocate(mach_task_self(), task);
			return KERN_SUCCESS;
		}
	} else {
		/* sorry, you must specify a valid task */
		*sc_status = kSCStatusInvalidArgument;
		return KERN_SUCCESS;
	}

	*sc_status = __SCDynamicStoreNotifySignal(mySession->store, pid, sig);
	if (*sc_status != kSCStatusOK) {
		__MACH_PORT_DEBUG(TRUE, "*** _notifyviasignal __SCDynamicStoreNotifySignal failed: releasing task)", task);
		(void) mach_port_deallocate(mach_task_self(), task);
		return KERN_SUCCESS;
	}

	__MACH_PORT_DEBUG(TRUE, "*** _notifyviasignal", task);
	storePrivate->notifyStatus     = Using_NotifierInformViaSignal;
	storePrivate->notifySignal     = sig;
	storePrivate->notifySignalTask = task;

	return KERN_SUCCESS;
}

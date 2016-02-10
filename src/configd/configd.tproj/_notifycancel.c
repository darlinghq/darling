/*
 * Copyright (c) 2000, 2001, 2003-2006, 2009 Apple Inc. All rights reserved.
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
 * March 31, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <unistd.h>

#include "configd.h"
#include "session.h"


__private_extern__
int
__SCDynamicStoreNotifyCancel(SCDynamicStoreRef store)
{
	SCDynamicStorePrivateRef	storePrivate = (SCDynamicStorePrivateRef)store;

	/*
	 * cleanup any mach port based notifications.
	 */
	if (storePrivate->notifyPort != MACH_PORT_NULL) {
		__MACH_PORT_DEBUG(TRUE, "*** __SCDynamicStoreNotifyCancel (notify port)", storePrivate->notifyPort);
		(void) mach_port_deallocate(mach_task_self(), storePrivate->notifyPort);
		storePrivate->notifyPort = MACH_PORT_NULL;
	}

	/*
	 * cleanup any file based notifications.
	 */
	if (storePrivate->notifyFile != -1) {
		// close (notification) fd
		(void) close(storePrivate->notifyFile);
		storePrivate->notifyFile = -1;
	}

	/*
	 * cleanup any signal notifications.
	 */
	if (storePrivate->notifySignal > 0) {
		__MACH_PORT_DEBUG(TRUE, "*** __SCDynamicStoreNotifyCancel (signal)", storePrivate->notifySignalTask);
		(void) mach_port_deallocate(mach_task_self(), storePrivate->notifySignalTask);
		storePrivate->notifySignal     = 0;
		storePrivate->notifySignalTask = TASK_NULL;
	}

	/* remove this session from the to-be-notified list */
	if (needsNotification) {
		CFNumberRef	num;

		num = CFNumberCreate(NULL, kCFNumberIntType, &storePrivate->server);
		CFSetRemoveValue(needsNotification, num);
		CFRelease(num);

		if (CFSetGetCount(needsNotification) == 0) {
			CFRelease(needsNotification);
			needsNotification = NULL;
		}
	}

	/* set notifier inactive */
	storePrivate->notifyStatus = NotifierNotRegistered;

	return kSCStatusOK;
}


__private_extern__
kern_return_t
_notifycancel(mach_port_t	server,
	      int		*sc_status)
{
	serverSessionRef	mySession = getSession(server);

	if (mySession == NULL) {
		*sc_status = kSCStatusNoStoreSession;	/* you must have an open session to play */
		return KERN_SUCCESS;
	}

	__MACH_PORT_DEBUG(((SCDynamicStorePrivateRef)mySession->store)->notifyPort != MACH_PORT_NULL,
			  "*** _notifycancel",
			  ((SCDynamicStorePrivateRef)mySession->store)->notifyPort);
	*sc_status = __SCDynamicStoreNotifyCancel(mySession->store);
	return KERN_SUCCESS;
}

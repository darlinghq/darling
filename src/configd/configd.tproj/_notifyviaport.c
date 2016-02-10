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
 * March 24, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include "configd.h"
#include "session.h"

__private_extern__
int
__SCDynamicStoreNotifyMachPort(SCDynamicStoreRef	store,
			       mach_msg_id_t		identifier,
			       mach_port_t		port)
{
	SCDynamicStorePrivateRef	storePrivate = (SCDynamicStorePrivateRef)store;
	CFStringRef			sessionKey;
	CFDictionaryRef			info;

	if (storePrivate->notifyStatus != NotifierNotRegistered) {
		/* sorry, you can only have one notification registered at once */
		return kSCStatusNotifierActive;
	}

	if (port == MACH_PORT_NULL) {
		/* sorry, you must specify a valid mach port */
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
_notifyviaport(mach_port_t	server,
	       mach_port_t	port,
	       mach_msg_id_t	identifier,
	       int		*sc_status
)
{
	serverSessionRef		mySession	= getSession(server);
	SCDynamicStorePrivateRef	storePrivate;

	if (mySession == NULL) {
		/* sorry, you must have an open session to play */
		*sc_status = kSCStatusNoStoreSession;
		if (port != MACH_PORT_NULL) {
			(void) mach_port_deallocate(mach_task_self(), port);
		}
		return KERN_SUCCESS;
	}
	storePrivate = (SCDynamicStorePrivateRef)mySession->store;

	*sc_status = __SCDynamicStoreNotifyMachPort(mySession->store, identifier, port);
	if (*sc_status != kSCStatusOK) {
		// if we can't enable the notification, release the provided callback port
		if (port != MACH_PORT_NULL) {
			__MACH_PORT_DEBUG(TRUE, "*** _notifyviaport __SCDynamicStoreNotifyMachPort failed: releasing port", port);
			(void) mach_port_deallocate(mach_task_self(), port);
		}
		return KERN_SUCCESS;
	}

	/* save notification port, requested identifier, and set notifier active */
	__MACH_PORT_DEBUG(TRUE, "*** _notifyviaport", port);
	storePrivate->notifyStatus         = Using_NotifierInformViaMachPort;
	storePrivate->notifyPort           = port;
	storePrivate->notifyPortIdentifier = identifier;

	return KERN_SUCCESS;
}

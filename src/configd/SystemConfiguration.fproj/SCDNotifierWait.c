/*
 * Copyright (c) 2000, 2001, 2004, 2006, 2009-2011 Apple Inc. All rights reserved.
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

#include <mach/mach.h>
#include <mach/mach_error.h>

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include "SCDynamicStoreInternal.h"
#include "config.h"		/* MiG generated file */

static mach_msg_id_t
waitForMachMessage(mach_port_t port)
{
	union {
		u_int8_t		buf[sizeof(mach_msg_empty_t) + MAX_TRAILER_SIZE];
		mach_msg_empty_rcv_t	msg;
	} notify_msg;
	kern_return_t 	status;

	status = mach_msg(&notify_msg.msg.header,	/* msg */
			  MACH_RCV_MSG,			/* options */
			  0,				/* send_size */
			  sizeof(notify_msg),		/* rcv_size */
			  port,				/* rcv_name */
			  MACH_MSG_TIMEOUT_NONE,	/* timeout */
			  MACH_PORT_NULL);		/* notify */
	if (status != KERN_SUCCESS) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("waitForMachMessage mach_msg(): %s"), mach_error_string(status));
		return -1;
	}

	return notify_msg.msg.header.msgh_id;
}


Boolean
SCDynamicStoreNotifyWait(SCDynamicStoreRef store)
{
	SCDynamicStorePrivateRef	storePrivate = (SCDynamicStorePrivateRef)store;
	kern_return_t			status;
	mach_port_t			port;
	mach_port_t			oldNotify;
	int				sc_status;
	mach_msg_id_t			msgid;

	if (store == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoStoreSession);
		return FALSE;
	}

	if (storePrivate->server == MACH_PORT_NULL) {
		/* sorry, you must have an open session to play */
		_SCErrorSet(kSCStatusNoStoreServer);
		return FALSE;
	}

	if (storePrivate->notifyStatus != NotifierNotRegistered) {
		/* sorry, you can only have one notification registered at once */
		_SCErrorSet(kSCStatusNotifierActive);
		return FALSE;
	}

	/* Allocating port (for server response) */
	status = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port);
	if (status != KERN_SUCCESS) {
		SCLog(TRUE, LOG_ERR, CFSTR("SCDynamicStoreNotifyWait mach_port_allocate(): %s"), mach_error_string(status));
		_SCErrorSet(status);
		return FALSE;
	}

	status = mach_port_insert_right(mach_task_self(),
					port,
					port,
					MACH_MSG_TYPE_MAKE_SEND);
	if (status != KERN_SUCCESS) {
		/*
		 * We can't insert a send right into our own port!  This should
		 * only happen if someone stomped on OUR port (so let's leave
		 * the port alone).
		 */
		SCLog(TRUE, LOG_ERR, CFSTR("SCDynamicStoreNotifyWait mach_port_insert_right(): %s"), mach_error_string(status));
		_SCErrorSet(status);
		return FALSE;
	}

	/* Request a notification when/if the server dies */
	status = mach_port_request_notification(mach_task_self(),
						port,
						MACH_NOTIFY_NO_SENDERS,
						1,
						port,
						MACH_MSG_TYPE_MAKE_SEND_ONCE,
						&oldNotify);
	if (status != KERN_SUCCESS) {
		/*
		 * We can't request a notification for our own port!  This should
		 * only happen if someone stomped on OUR port (so let's leave
		 * the port alone).
		 */
		SCLog(TRUE, LOG_ERR, CFSTR("SCDynamicStoreNotifyWait mach_port_request_notification(): %s"), mach_error_string(status));
		_SCErrorSet(status);
		return FALSE;
	}

	if (oldNotify != MACH_PORT_NULL) {
		SCLog(TRUE, LOG_ERR, CFSTR("SCDynamicStoreNotifyWait(): oldNotify != MACH_PORT_NULL"));
	}

    retry :

	status = notifyviaport(storePrivate->server,
			       port,
			       0,
			       (int *)&sc_status);

	if (__SCDynamicStoreCheckRetryAndHandleError(store,
						     status,
						     &sc_status,
						     "SCDynamicStoreNotifyWait notifyviaport()")) {
		goto retry;
	}

	if (status != KERN_SUCCESS) {
		if ((status == MACH_SEND_INVALID_DEST) || (status == MIG_SERVER_DIED)) {
			/* remove the send right that we tried (but failed) to pass to the server */
			(void) mach_port_deallocate(mach_task_self(), port);
		}

		/* remove our receive right  */
		(void) mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_RECEIVE, -1);
	}

	if (sc_status != kSCStatusOK) {
		_SCErrorSet(sc_status);
		return FALSE;
	}

	/* set notifier active */
	storePrivate->notifyStatus = Using_NotifierWait;

	msgid = waitForMachMessage(port);

	/* set notifier inactive */
	storePrivate->notifyStatus = NotifierNotRegistered;

	if (msgid == MACH_NOTIFY_NO_SENDERS) {
		/* the server closed the notifier port */
#ifdef	DEBUG
		SCLog(_sc_verbose, LOG_DEBUG, CFSTR("SCDynamicStoreNotifyWait notifier port closed, destroying port %d"), port);
#endif	/* DEBUG */
		_SCErrorSet(kSCStatusNoStoreServer);
		return FALSE;
	}

	if (msgid == -1) {
		/* one of the mach routines returned an error */
#ifdef	DEBUG
		SCLog(_sc_verbose, LOG_DEBUG, CFSTR("SCDynamicStoreNotifyWait communication with server failed, destroying port %d"), port);
#endif	/* DEBUG */
		(void) mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_RECEIVE , -1);
		_SCErrorSet(kSCStatusNoStoreServer);
		return FALSE;
	}

	// something changed, cancelling notification request
	status = notifycancel(storePrivate->server,
			      (int *)&sc_status);

	if (__SCDynamicStoreCheckRetryAndHandleError(store,
						     status,
						     &sc_status,
						     "SCDynamicStoreNotifyWait notifycancel()")) {
		sc_status = kSCStatusOK;
	}

	/* remove our receive right  */
	(void) mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_RECEIVE , -1);

	if (sc_status != kSCStatusOK) {
		_SCErrorSet(sc_status);
		return FALSE;
	}

	return TRUE;
}

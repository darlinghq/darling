/*
 * Copyright (c) 2000-2005, 2008-2019 Apple Inc. All rights reserved.
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

#include "SCDynamicStoreInternal.h"
#include "config.h"		/* MiG generated file */
#include "SCD.h"

#if    !TARGET_OS_SIMULATOR || (defined(IPHONE_SIMULATOR_HOST_MIN_VERSION_REQUIRED) && (IPHONE_SIMULATOR_HOST_MIN_VERSION_REQUIRED >= 1090))
#define HAVE_MACHPORT_GUARDS
#endif


static void
removeReceiveRight(SCDynamicStoreRef store, mach_port_t port)
{
	/* remove our receive right */
#ifdef	HAVE_MACHPORT_GUARDS
	(void) mach_port_destruct(mach_task_self(), port, 0, (mach_port_context_t)store);
#else	// HAVE_MACHPORT_GUARDS
	(void) mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_RECEIVE, -1);
#endif	// HAVE_MACHPORT_GUARDS
	return;
}


#define	MAX_INVALID_RIGHT_RETRY	3

static mach_port_t
addNotifyPort(SCDynamicStoreRef store)
{
	kern_return_t				kr;
	mach_port_t				oldNotify;
#ifdef	HAVE_MACHPORT_GUARDS
	mach_port_options_t			opts;
#endif	// HAVE_MACHPORT_GUARDS
	mach_port_t				port;
	int					sc_status;
	SCDynamicStorePrivateRef		storePrivate	= (SCDynamicStorePrivateRef)store;
	int					try		= 0;

	/* allocate a mach port for the SCDynamicStore notifications */

    retry_allocate :

#ifdef	HAVE_MACHPORT_GUARDS
	memset(&opts, 0, sizeof(opts));
	opts.flags = MPO_CONTEXT_AS_GUARD|MPO_INSERT_SEND_RIGHT;

	kr = mach_port_construct(mach_task_self(), &opts, (mach_port_context_t)store, &port);
#else	// HAVE_MACHPORT_GUARDS
	kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port);
#endif	// HAVE_MACHPORT_GUARDS

	if (kr != KERN_SUCCESS) {
		SC_log(LOG_NOTICE, "could not allocate mach port: %s", mach_error_string(kr));
		if ((kr == KERN_NO_SPACE) || (kr == KERN_RESOURCE_SHORTAGE)) {
			usleep(50 * 1000);	// sleep 50ms between attempts
			goto retry_allocate;
		}
		goto fail;
	}

#ifndef	HAVE_MACHPORT_GUARDS
	kr = mach_port_insert_right(mach_task_self(),
				    port,
				    port,
				    MACH_MSG_TYPE_MAKE_SEND);
	if (kr != KERN_SUCCESS) {
		/*
		 * We can't insert a send right into our own port!  This should
		 * only happen if someone stomped on OUR port (so let's leave
		 * the port alone).
		 */
		SC_log(LOG_NOTICE, "mach_port_insert_right() failed: %s", mach_error_string(kr));
		goto fail;
	}
#endif	// HAVE_MACHPORT_GUARDS

	/* Request a notification when/if the server dies */
	kr = mach_port_request_notification(mach_task_self(),
					    port,
					    MACH_NOTIFY_NO_SENDERS,
					    1,
					    port,
					    MACH_MSG_TYPE_MAKE_SEND_ONCE,
					    &oldNotify);
	if (kr != KERN_SUCCESS) {
		/*
		 * We can't request a notification for our own port!  This should
		 * only happen if someone stomped on OUR port (so let's leave
		 * the port alone).
		 */
		SC_log(LOG_NOTICE, "mach_port_request_notification() failed: %s", mach_error_string(kr));
		goto fail;
	}

	if (oldNotify != MACH_PORT_NULL) {
		SC_log(LOG_NOTICE, "oldNotify != MACH_PORT_NULL");
	}

#ifdef	DEBUG
	SC_log(LOG_DEBUG, "+ establish notification request w/port=0x%x (%d) with SCDynamicStore server (%@)",
	       port, port,
	       (storePrivate->name != NULL) ? storePrivate->name : CFSTR("?"));
#endif	/* DEBUG */

    retry :

	__MACH_PORT_DEBUG(TRUE, "*** addNotifyPort", port);
	kr = notifyviaport(storePrivate->server, port, 0, (int *)&sc_status);

	if (kr == MACH_SEND_INVALID_RIGHT) {
		/*
		 * the notifyviaport() call was not able to pass/process
		 * the [notify port] send right.  This was most likely due
		 * to the [SCDynamicStore] server having restarted.  We
		 * handle this issue by removing the remaining receive
		 * right, reallocating the notify port, and retrying
		 * the operation.
		 */
		SC_log((try++ < MAX_INVALID_RIGHT_RETRY) ? LOG_INFO : LOG_ERR,
		       "SCDynamicStore callback notifyviaport() w/port 0x%x (%d) failed (try %d): %s",
		       port, port,
		       try,
		       mach_error_string(kr));

		removeReceiveRight(store, port);
	}

	if (__SCDynamicStoreCheckRetryAndHandleError(store,
						     kr,
						     &sc_status,
						     "SCDynamicStore callback notifyviaport()")) {
		if (kr == MACH_SEND_INVALID_RIGHT) {
			if (try <= MAX_INVALID_RIGHT_RETRY) {
				goto retry_allocate;
			}
			goto fail;
		}
		goto retry;
	}

	if (try > 0) {
		SC_log(LOG_INFO,
		       "SCDynamicStore callback notifyviaport() succeeded after %d retr%s w/port 0x%x (%d)",
		       try,
		       (try == 1) ? "y" : "ies",
		       port, port);
	}

	if (kr != KERN_SUCCESS) {
		if ((kr == MACH_SEND_INVALID_DEST) || (kr == MIG_SERVER_DIED)) {
			/* remove the send right that we tried (but failed) to pass to the server */
			(void) mach_port_deallocate(mach_task_self(), port);
		}
		removeReceiveRight(store, port);
		goto fail;
	}

	if (sc_status != kSCStatusOK) {
		/* something [else] didn't work  */
		removeReceiveRight(store, port);
		kr = sc_status;
		goto fail;
	}

	return port;

    fail :

	_SCErrorSet(kr);
	return MACH_PORT_NULL;
}


static CFStringRef
notifyMPCopyDescription(const void *info)
{
	SCDynamicStoreRef	store	= (SCDynamicStoreRef)info;

	return CFStringCreateWithFormat(NULL,
					NULL,
					CFSTR("<SCDynamicStore notification MP> {store = %p}"),
					store);
}


static void
rlsCallback(CFMachPortRef port, void *msg, CFIndex size, void *info)
{
#ifndef	DEBUG
#pragma unused(port)
#endif	/* DEBUG */
#pragma unused(size)
	mach_no_senders_notification_t	*buf		= msg;
	mach_msg_id_t			msgid		= buf->not_header.msgh_id;
	SCDynamicStoreRef		store		= (SCDynamicStoreRef)info;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

#ifdef	DEBUG
	SC_log(LOG_DEBUG, "mach port callback, %ssignal RLS(%@)",
	       (msgid == MACH_NOTIFY_NO_SENDERS) ? "reconnect and " : "",
	       (storePrivate->name != NULL) ? storePrivate->name : CFSTR("?"));
#endif	/* DEBUG */

	if (msgid == MACH_NOTIFY_NO_SENDERS) {
		/* the server died, disable additional callbacks */
#ifdef	DEBUG
		SC_log(LOG_DEBUG, "  notifier port closed");
#endif	/* DEBUG */

#ifdef	DEBUG
		if (port != storePrivate->rlsNotifyPort) {
			SC_log(LOG_DEBUG, "why is port != rlsNotifyPort?");
		}
#endif	/* DEBUG */

		/* re-establish notification and inform the client */
		(void)__SCDynamicStoreReconnectNotifications(store);
	}

	/* signal the real runloop source */
	if (storePrivate->rls != NULL) {
		CFRunLoopSourceSignal(storePrivate->rls);
	}

	return;
}


static void
rlsSchedule(void *info, CFRunLoopRef rl, CFStringRef mode)
{
	SCDynamicStoreRef		store		= (SCDynamicStoreRef)info;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

#ifdef	DEBUG
	SC_log(LOG_DEBUG, "schedule notifications for mode %@", mode);
#endif	/* DEBUG */

	if (storePrivate->rlsNotifyPort == NULL) {
		CFMachPortContext	context		= { 0
							  , (void *)store
							  , CFRetain
							  , CFRelease
							  , notifyMPCopyDescription
							  };
		mach_port_t		port;

#ifdef	DEBUG
		SC_log(LOG_DEBUG, "  activate callback runloop source");
#endif	/* DEBUG */

		port = addNotifyPort(store);
		if (port == MACH_PORT_NULL) {
			return;
		}

		__MACH_PORT_DEBUG(TRUE, "*** rlsSchedule (after addNotifyPort)", port);
		storePrivate->rlsNotifyPort = _SC_CFMachPortCreateWithPort("SCDynamicStore",
									   port,
									   rlsCallback,
									   &context);

		if (rl != NULL) {
			storePrivate->rlsNotifyRLS = CFMachPortCreateRunLoopSource(NULL, storePrivate->rlsNotifyPort, 0);
			storePrivate->rlList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		}
	}

	if (storePrivate->rlsNotifyRLS != NULL) {
		/* set notifier active */
		storePrivate->notifyStatus = Using_NotifierInformViaRunLoop;

		if (!_SC_isScheduled(store, rl, mode, storePrivate->rlList)) {
			/*
			 * if we are not already scheduled with this runLoop / runLoopMode
			 */
			CFRunLoopAddSource(rl, storePrivate->rlsNotifyRLS, mode);
			__MACH_PORT_DEBUG(TRUE, "*** rlsSchedule (after CFRunLoopAddSource)", CFMachPortGetPort(storePrivate->rlsNotifyPort));
		}

		_SC_schedule(store, rl, mode, storePrivate->rlList);
	}

	return;
}


static void
rlsCancel(void *info, CFRunLoopRef rl, CFStringRef mode)
{
	CFIndex				n		= 0;
	SCDynamicStoreRef		store		= (SCDynamicStoreRef)info;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

#ifdef	DEBUG
	SC_log(LOG_DEBUG, "cancel notifications for mode %@", mode);
#endif	/* DEBUG */

	if (storePrivate->rlsNotifyRLS != NULL) {
		if (_SC_unschedule(store, rl, mode, storePrivate->rlList, FALSE)) {
			/*
			 * if currently scheduled on this runLoop / runLoopMode
			 */
			n = CFArrayGetCount(storePrivate->rlList);
			if (n == 0 || !_SC_isScheduled(store, rl, mode, storePrivate->rlList)) {
				/*
				 * if we are no longer scheduled to receive notifications for
				 * this runLoop / runLoopMode
				 */
				CFRunLoopRemoveSource(rl, storePrivate->rlsNotifyRLS, mode);
			}
		}
	}

	if (n == 0) {
		int		sc_status;
		kern_return_t	kr;

#ifdef	DEBUG
		SC_log(LOG_DEBUG, "  cancel callback runloop source");
#endif	/* DEBUG */
		__MACH_PORT_DEBUG((storePrivate->rlsNotifyPort != NULL),
				  "*** rlsCancel",
				  CFMachPortGetPort(storePrivate->rlsNotifyPort));

		if (storePrivate->rls != NULL) {
			// Remove the reference we took on the rls. We do not invalidate
			// the runloop source and let the client do it when appropriate.
			CFRelease(storePrivate->rls);
			storePrivate->rls = NULL;
		}

		if (storePrivate->rlList != NULL) {
			CFRelease(storePrivate->rlList);
			storePrivate->rlList = NULL;
		}

		if (storePrivate->rlsNotifyRLS != NULL) {
			/* invalidate & remove the run loop source */
			CFRunLoopSourceInvalidate(storePrivate->rlsNotifyRLS);
			CFRelease(storePrivate->rlsNotifyRLS);
			storePrivate->rlsNotifyRLS = NULL;
		}

		if (storePrivate->rlsNotifyPort != NULL) {
			mach_port_t	mp;

			mp = CFMachPortGetPort(storePrivate->rlsNotifyPort);
			__MACH_PORT_DEBUG((storePrivate->rlsNotifyPort != NULL),
					  "*** rlsCancel (before invalidating/releasing CFMachPort)",
					  mp);

			/* invalidate and release port */
			CFMachPortInvalidate(storePrivate->rlsNotifyPort);
			CFRelease(storePrivate->rlsNotifyPort);
			storePrivate->rlsNotifyPort = NULL;

			/* and, finally, remove our receive right  */
			removeReceiveRight(store, mp);
		}

#ifdef	DEBUG
		SC_log(LOG_DEBUG, "  cancel notification request with SCDynamicStore server");
#endif	/* DEBUG */

		if (storePrivate->server != MACH_PORT_NULL) {
			kr = notifycancel(storePrivate->server, (int *)&sc_status);

			(void) __SCDynamicStoreCheckRetryAndHandleError(store,
									kr,
									&sc_status,
									"rlsCancel notifycancel()");

			if (kr != KERN_SUCCESS) {
				return;
			}
		}

		/* set notifier inactive */
		storePrivate->notifyStatus = NotifierNotRegistered;
	}

	return;
}


static void
rlsPerform(void *info)
{
	CFArrayRef			changedKeys	= NULL;
	void				*context_info;
	void				(*context_release)(const void *);
	SCDynamicStoreCallBack		rlsFunction;
	SCDynamicStoreRef		store		= (SCDynamicStoreRef)info;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

#ifdef	DEBUG
	SC_log(LOG_DEBUG, "handling SCDynamicStore changes (%@)",
	       (storePrivate->name != NULL) ? storePrivate->name : CFSTR("?"));
#endif	/* DEBUG */

	changedKeys = SCDynamicStoreCopyNotifiedKeys(store);
	if (storePrivate->disconnectForceCallBack) {
		storePrivate->disconnectForceCallBack = FALSE;
		if (changedKeys == NULL) {
			changedKeys = CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
		}
	} else if ((changedKeys == NULL) || (CFArrayGetCount(changedKeys) == 0)) {
		/* if no changes or something happened to the server */
		goto done;
	}

	rlsFunction = storePrivate->rlsFunction;

	if (storePrivate->rlsContext.retain != NULL) {
		context_info	= (void *)storePrivate->rlsContext.retain(storePrivate->rlsContext.info);
		context_release	= storePrivate->rlsContext.release;
	} else {
		context_info	= storePrivate->rlsContext.info;
		context_release	= NULL;
	}
	if (rlsFunction != NULL) {
		SC_log(LOG_DEBUG, "+ exec SCDynamicStore callout");
		(*rlsFunction)(store, changedKeys, context_info);
		SC_log(LOG_DEBUG, "+ done");
	}
	if (context_release != NULL) {
		context_release(context_info);
	}

    done :

#ifdef	DEBUG
	SC_log(LOG_DEBUG, "done");
#endif	/* DEBUG */

	if (changedKeys != NULL) {
		CFRelease(changedKeys);
	}

	return;
}


static CFStringRef
rlsCopyDescription(const void *info)
{
	CFMutableStringRef		result;
	SCDynamicStoreRef		store		= (SCDynamicStoreRef)info;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	result = CFStringCreateMutable(NULL, 0);
	CFStringAppendFormat(result, NULL, CFSTR("<SCDynamicStore RLS> {"));
	CFStringAppendFormat(result, NULL, CFSTR("store = %p"), store);
	if (storePrivate->notifyStatus == Using_NotifierInformViaRunLoop) {
		CFStringRef	description	= NULL;

		CFStringAppendFormat(result, NULL, CFSTR(", callout = %p"), storePrivate->rlsFunction);

		if ((storePrivate->rlsContext.info != NULL) && (storePrivate->rlsContext.copyDescription != NULL)) {
			description = (*storePrivate->rlsContext.copyDescription)(storePrivate->rlsContext.info);
		}
		if (description == NULL) {
			description = CFStringCreateWithFormat(NULL, NULL, CFSTR("<SCDynamicStore context %p>"), storePrivate->rlsContext.info);
		}
		if (description == NULL) {
			description = CFRetain(CFSTR("<no description>"));
		}
		CFStringAppendFormat(result, NULL, CFSTR(", context = %@"), description);
		CFRelease(description);
	}
	CFStringAppendFormat(result, NULL, CFSTR("}"));

	return result;
}


CFRunLoopSourceRef
SCDynamicStoreCreateRunLoopSource(CFAllocatorRef	allocator,
				  SCDynamicStoreRef	store,
				  CFIndex		order)
{
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	if (store == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoStoreSession);
		return NULL;
	}

	if (storePrivate->server == MACH_PORT_NULL) {
		/* sorry, you must have an open session to play */
		_SCErrorSet(kSCStatusNoStoreServer);
		return NULL;
	}

	switch (storePrivate->notifyStatus) {
		case NotifierNotRegistered :
		case Using_NotifierInformViaRunLoop :
			/* OK to enable runloop notification */
			break;
		default :
			/* sorry, you can only have one notification registered at once */
			_SCErrorSet(kSCStatusNotifierActive);
			return NULL;
	}

	if (storePrivate->rls == NULL) {
		CFRunLoopSourceContext	context = { 0			// version
						  , (void *)store	// info
						  , CFRetain		// retain
						  , CFRelease		// release
						  , rlsCopyDescription	// copyDescription
						  , CFEqual		// equal
						  , CFHash		// hash
						  , rlsSchedule		// schedule
						  , rlsCancel		// cancel
						  , rlsPerform		// perform
						  };

		storePrivate->rls = CFRunLoopSourceCreate(allocator, order, &context);
		if (storePrivate->rls == NULL) {
			_SCErrorSet(kSCStatusFailed);
		}
	}

	if (storePrivate->rls != NULL) {
		CFRetain(storePrivate->rls);
	}

	return storePrivate->rls;
}


Boolean
SCDynamicStoreSetDispatchQueue(SCDynamicStoreRef store, dispatch_queue_t queue)
{
	dispatch_group_t		drainGroup	= NULL;
	dispatch_queue_t		drainQueue	= NULL;
	dispatch_group_t		group		= NULL;
	mach_port_t			mp;
	Boolean				ok		= FALSE;
	dispatch_source_t		source;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	if (store == NULL) {
		// sorry, you must provide a session
		_SCErrorSet(kSCStatusNoStoreSession);
		return FALSE;
	}

	if (queue == NULL) {
		if (storePrivate->dispatchQueue == NULL) {
			// if not scheduled
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
		}

#ifdef	DEBUG
		SC_log(LOG_DEBUG, "unschedule notifications from dispatch queue (%@)",
		       (storePrivate->name != NULL) ? storePrivate->name : CFSTR("?"));
#endif	/* DEBUG */

		ok = TRUE;
		goto cleanup;
	}

	if (storePrivate->server == MACH_PORT_NULL) {
		// sorry, you must have an open session to play
		_SCErrorSet(kSCStatusNoStoreServer);
		return FALSE;
	}

	if ((storePrivate->dispatchQueue != NULL)	||
	    (storePrivate->rls != NULL)			||
	    (storePrivate->notifyStatus != NotifierNotRegistered)) {
		// if already scheduled
		_SCErrorSet(kSCStatusNotifierActive);
		return FALSE;
	}

#ifdef	DEBUG
	SC_log(LOG_DEBUG, "schedule notifications for dispatch queue (%@)",
	       (storePrivate->name != NULL) ? storePrivate->name : CFSTR("?"));
#endif	/* DEBUG */

	//
	// mark our using of the SCDynamicStore notifications, create and schedule
	// the notification source/port (storePrivate->dispatchSource), and a bunch
	// of other "setup"
	//
	storePrivate->notifyStatus = Using_NotifierInformViaDispatch;

	mp = addNotifyPort(store);
	if (mp == MACH_PORT_NULL) {
		// if we could not schedule the notification
		_SCErrorSet(kSCStatusFailed);
#ifdef	DEBUG
		SC_log(LOG_DEBUG, "addNotifyPort() failed (%@)",
		       (storePrivate->name != NULL) ? storePrivate->name : CFSTR("?"));
#endif	/* DEBUG */
		goto cleanup;
	}

	// retain the dispatch queue
	storePrivate->dispatchQueue = queue;
	dispatch_retain(storePrivate->dispatchQueue);

	//
	// We've taken a reference to the callers dispatch_queue and we
	// want to hold on to that reference until we've processed any/all
	// notifications.  To facilitate this we create a group, dispatch
	// any notification blocks to via that group, and when the caller
	// has told us to stop the notifications (unschedule) we wait for
	// the group to empty and use the group's finalizer to release
	// our reference to the SCDynamicStore.
	//
	group = dispatch_group_create();
	storePrivate->dispatchGroup = group;
	CFRetain(store);
	dispatch_set_context(storePrivate->dispatchGroup, (void *)store);
	dispatch_set_finalizer_f(storePrivate->dispatchGroup, (dispatch_function_t)CFRelease);

	// create a dispatch source for the mach notifications
	source = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, mp, 0, queue);
	if (source == NULL) {
		SC_log(LOG_NOTICE, "dispatch_source_create() failed");

		//  remove our receive right
		removeReceiveRight(store, mp);

		_SCErrorSet(kSCStatusFailed);
		goto cleanup;
	}

	dispatch_source_set_event_handler(source, ^{
		kern_return_t	kr;
		mach_msg_id_t	msgid;
		union {
			u_int8_t			buf1[sizeof(mach_msg_empty_rcv_t) + MAX_TRAILER_SIZE];
			u_int8_t			buf2[sizeof(mach_no_senders_notification_t) + MAX_TRAILER_SIZE];
			mach_msg_empty_rcv_t		msg;
			mach_no_senders_notification_t	no_senders;
		} notify_msg;

		kr = mach_msg(&notify_msg.msg.header,	// msg
			      MACH_RCV_MSG,		// options
			      0,			// send_size
			      sizeof(notify_msg),	// rcv_size
			      mp,			// rcv_name
			      MACH_MSG_TIMEOUT_NONE,	// timeout
			      MACH_PORT_NULL);		// notify
		if (kr != KERN_SUCCESS) {
			SC_log(LOG_NOTICE, "mach_msg() failed, kr=0x%x", kr);
			return;
		}

		msgid = notify_msg.msg.header.msgh_id;
		mach_msg_destroy(&notify_msg.msg.header);

#ifdef	DEBUG
		SC_log(LOG_DEBUG, "dispatch source callback, queue rlsPerform (%@)",
		       (storePrivate->name != NULL) ? storePrivate->name : CFSTR("?"));
#endif	/* DEBUG */

		CFRetain(store);
		dispatch_group_async(group, queue, ^{
			if (msgid == MACH_NOTIFY_NO_SENDERS) {
				// re-establish notification and inform the client
				(void)__SCDynamicStoreReconnectNotifications(store);
			}
			rlsPerform(storePrivate);
			CFRelease(store);
		});
	});

	dispatch_source_set_cancel_handler(source, ^{
		__MACH_PORT_DEBUG(TRUE,
				  "*** SCDynamicStoreSetDispatchQueue (before cancel)",
				  mp);

		// remove our receive right
		removeReceiveRight(store, mp);

		// release source
		dispatch_release(source);
	});

	storePrivate->dispatchSource = source;
	dispatch_resume(source);

	return TRUE;

    cleanup :

	CFRetain(store);

	if (storePrivate->dispatchSource != NULL) {
		dispatch_source_cancel(storePrivate->dispatchSource);
		storePrivate->dispatchSource = NULL;
	}
	drainGroup = storePrivate->dispatchGroup;
	storePrivate->dispatchGroup = NULL;
	drainQueue = storePrivate->dispatchQueue;
	storePrivate->dispatchQueue = NULL;

	if ((drainGroup != NULL) && (drainQueue != NULL)) {
		dispatch_group_notify(drainGroup, drainQueue, ^{
			// release group/queue references
			dispatch_release(drainQueue);
			dispatch_release(drainGroup);	// releases our store reference
		});
	}

	storePrivate->notifyStatus = NotifierNotRegistered;

	CFRelease(store);

	return ok;
}

/*
 * Copyright (c) 2000-2005, 2008-2014 Apple Inc. All rights reserved.
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

#include <Availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <dispatch/dispatch.h>
#include <mach/mach.h>
#include <mach/mach_error.h>

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include "SCDynamicStoreInternal.h"
#include "config.h"		/* MiG generated file */

#if !TARGET_IPHONE_SIMULATOR || (defined(IPHONE_SIMULATOR_HOST_MIN_VERSION_REQUIRED) && (IPHONE_SIMULATOR_HOST_MIN_VERSION_REQUIRED >= 1090))
#define HAVE_MACHPORT_GUARDS
#endif


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
	mach_no_senders_notification_t	*buf		= msg;
	mach_msg_id_t			msgid		= buf->not_header.msgh_id;
	SCDynamicStoreRef		store		= (SCDynamicStoreRef)info;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	if (msgid == MACH_NOTIFY_NO_SENDERS) {
		/* the server died, disable additional callbacks */
#ifdef	DEBUG
		SCLog(_sc_verbose, LOG_INFO, CFSTR("  rlsCallback(), notifier port closed"));
#endif	/* DEBUG */

#ifdef	DEBUG
		if (port != storePrivate->rlsNotifyPort) {
			SCLog(_sc_verbose, LOG_DEBUG, CFSTR("rlsCallback(), why is port != rlsNotifyPort?"));
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
	SCLog(_sc_verbose, LOG_DEBUG,
	      CFSTR("schedule notifications for mode %@"),
	      (rl != NULL) ? mode : CFSTR("libdispatch"));
#endif	/* DEBUG */

	if (storePrivate->rlList == NULL) {
		CFMachPortContext	context		= { 0
							  , (void *)store
							  , CFRetain
							  , CFRelease
							  , notifyMPCopyDescription
							  };
		kern_return_t		kr;
		mach_port_t		oldNotify;
#ifdef	HAVE_MACHPORT_GUARDS
		mach_port_options_t	opts;
#endif	// HAVE_MACHPORT_GUARDS
		mach_port_t		port;
		int			sc_status;

#ifdef	DEBUG
		SCLog(_sc_verbose, LOG_DEBUG, CFSTR("  activate callback runloop source"));
#endif	/* DEBUG */

		/* allocate a mach port for the SCDynamicStore notifications */

	    retry_allocate :

#ifdef	HAVE_MACHPORT_GUARDS
		bzero(&opts, sizeof(opts));
		opts.flags = MPO_CONTEXT_AS_GUARD|MPO_INSERT_SEND_RIGHT;

		kr = mach_port_construct(mach_task_self(), &opts, store, &port);
#else	// HAVE_MACHPORT_GUARDS
		kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port);
#endif	// HAVE_MACHPORT_GUARDS

		if (kr != KERN_SUCCESS) {
			SCLog(TRUE, LOG_ERR, CFSTR("rlsSchedule could not allocate mach port: %s"), mach_error_string(kr));
			if ((kr == KERN_NO_SPACE) || (kr == KERN_RESOURCE_SHORTAGE)) {
				sleep(1);
				goto retry_allocate;
			} else {
				return;
			}
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
			SCLog(TRUE, LOG_ERR, CFSTR("rlsSchedule mach_port_insert_right(): %s"), mach_error_string(kr));
			return;
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
			SCLog(TRUE, LOG_ERR, CFSTR("rlsSchedule mach_port_request_notification(): %s"), mach_error_string(kr));
			return;
		}

		if (oldNotify != MACH_PORT_NULL) {
			SCLog(TRUE, LOG_ERR, CFSTR("rlsSchedule(): oldNotify != MACH_PORT_NULL"));
		}

	    retry :

		__MACH_PORT_DEBUG(TRUE, "*** rlsSchedule", port);
		kr = notifyviaport(storePrivate->server, port, 0, (int *)&sc_status);

		if (__SCDynamicStoreCheckRetryAndHandleError(store,
							     kr,
							     &sc_status,
							     "rlsSchedule notifyviaport()")) {
			goto retry;
		}

		if (kr != KERN_SUCCESS) {
			if ((kr == MACH_SEND_INVALID_DEST) || (kr == MIG_SERVER_DIED)) {
				/* remove the send right that we tried (but failed) to pass to the server */
				(void) mach_port_deallocate(mach_task_self(), port);
			}

			/* remove our receive right  */
#ifdef	HAVE_MACHPORT_GUARDS
			(void) mach_port_destruct(mach_task_self(), port, 0, store);
#else	// HAVE_MACHPORT_GUARDS
			(void) mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_RECEIVE, -1);
#endif	// HAVE_MACHPORT_GUARDS
			return;
		}

		if (sc_status != kSCStatusOK) {
			/* something [else] didn't work, remove our receive right  */
#ifdef	HAVE_MACHPORT_GUARDS
			(void) mach_port_destruct(mach_task_self(), port, 0, store);
#else	// HAVE_MACHPORT_GUARDS
			(void) mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_RECEIVE, -1);
#endif	// HAVE_MACHPORT_GUARDS
			return;
		}

		__MACH_PORT_DEBUG(TRUE, "*** rlsSchedule (after notifyviaport)", port);
		storePrivate->rlsNotifyPort = _SC_CFMachPortCreateWithPort("SCDynamicStore",
									   port,
									   rlsCallback,
									   &context);
		storePrivate->rlsNotifyRLS = CFMachPortCreateRunLoopSource(NULL, storePrivate->rlsNotifyPort, 0);

		storePrivate->rlList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}

	if ((rl != NULL) && (storePrivate->rlsNotifyRLS != NULL)) {
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
	SCLog(_sc_verbose, LOG_DEBUG,
	      CFSTR("cancel notifications for mode %@"),
	      (rl != NULL) ? mode : CFSTR("libdispatch"));
#endif	/* DEBUG */

	if ((rl != NULL) && (storePrivate->rlsNotifyRLS != NULL)) {
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
		SCLog(_sc_verbose, LOG_DEBUG, CFSTR("  cancel callback runloop source"));
#endif	/* DEBUG */
		__MACH_PORT_DEBUG((storePrivate->rlsNotifyPort != NULL),
				  "*** rlsCancel",
				  CFMachPortGetPort(storePrivate->rlsNotifyPort));

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
#ifdef	HAVE_MACHPORT_GUARDS
			(void) mach_port_destruct(mach_task_self(), mp, 0, store);
#else	// HAVE_MACHPORT_GUARDS
			(void) mach_port_mod_refs(mach_task_self(), mp, MACH_PORT_RIGHT_RECEIVE, -1);
#endif	// HAVE_MACHPORT_GUARDS
		}

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
	}

	return;
}


static void
rlsPerform(void *info)
{
	CFArrayRef			changedKeys;
	void				*context_info;
	void				(*context_release)(const void *);
	SCDynamicStoreCallBack		rlsFunction;
	SCDynamicStoreRef		store		= (SCDynamicStoreRef)info;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

#ifdef	DEBUG
	SCLog(_sc_verbose, LOG_DEBUG, CFSTR("  executing notification function"));
#endif	/* DEBUG */

	changedKeys = SCDynamicStoreCopyNotifiedKeys(store);
	if (storePrivate->disconnectForceCallBack) {
		storePrivate->disconnectForceCallBack = FALSE;
		if (changedKeys == NULL) {
			changedKeys = CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
		}
	} else {
		if (changedKeys == NULL) {
			/* if no changes or something happened to the server */
			return;
		} else if (CFArrayGetCount(changedKeys) == 0) {
			goto done;
		}
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
		(*rlsFunction)(store, changedKeys, context_info);
	}
	if (context_release != NULL) {
		context_release(context_info);
	}

    done :

	CFRelease(changedKeys);
	return;
}


static CFTypeRef
rlsRetain(CFTypeRef cf)
{
	SCDynamicStoreRef		store		= (SCDynamicStoreRef)cf;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	switch (storePrivate->notifyStatus) {
		case NotifierNotRegistered :
			/* mark RLS active */
			storePrivate->notifyStatus = Using_NotifierInformViaRunLoop;
			/* keep a reference to the store */
			CFRetain(store);
			break;
		case Using_NotifierInformViaRunLoop :
			break;
		default :
			SCLog(TRUE, LOG_ERR, CFSTR("rlsRetain() error: notifyStatus=%d"), storePrivate->notifyStatus);
			break;
	}

	return cf;
}


static void
rlsRelease(CFTypeRef cf)
{
	SCDynamicStoreRef		store		= (SCDynamicStoreRef)cf;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	switch (storePrivate->notifyStatus) {
		case NotifierNotRegistered :
			break;
		case Using_NotifierInformViaRunLoop :
			/* mark RLS inactive */
			storePrivate->notifyStatus = NotifierNotRegistered;
			storePrivate->rls = NULL;

			/* release our reference to the store */
			CFRelease(store);
			break;
		default :
			SCLog(TRUE, LOG_ERR, CFSTR("rlsRelease() error: notifyStatus=%d"), storePrivate->notifyStatus);
			break;
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

	if (storePrivate->rls != NULL) {
		CFRetain(storePrivate->rls);
	} else {
		CFRunLoopSourceContext	context = { 0			// version
						  , (void *)store	// info
						  , rlsRetain		// retain
						  , rlsRelease		// release
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
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
		}

		ok = TRUE;
		goto cleanup;
	}

	if (storePrivate->server == MACH_PORT_NULL) {
		// sorry, you must have an open session to play
		_SCErrorSet(kSCStatusNoStoreServer);
		return FALSE;
	}

	if ((storePrivate->dispatchQueue != NULL) || (storePrivate->rls != NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (storePrivate->notifyStatus != NotifierNotRegistered) {
		// sorry, you can only have one notification registered at once...
		_SCErrorSet(kSCStatusNotifierActive);
		return FALSE;
	}

	/*
	 * mark our using of the SCDynamicStore notifications, create and schedule
	 * the notification port (storePrivate->rlsNotifyPort), and a bunch of other
	 * "setup"
	 */
	storePrivate->notifyStatus = Using_NotifierInformViaDispatch;
	rlsSchedule((void*)store, NULL, NULL);
	if (storePrivate->rlsNotifyPort == NULL) {
		/* if we could not schedule the notification */
		_SCErrorSet(kSCStatusFailed);
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
	mp = CFMachPortGetPort(storePrivate->rlsNotifyPort);
	source = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, mp, 0, queue);
	if (source == NULL) {
		SCLog(TRUE, LOG_ERR, CFSTR("SCDynamicStore dispatch_source_create() failed"));
		_SCErrorSet(kSCStatusFailed);
		goto cleanup;
	}

	dispatch_source_set_event_handler(source, ^{
		kern_return_t	kr;
		mach_msg_id_t	msgid;
		union {
			u_int8_t			buf[sizeof(mach_msg_empty_t) + MAX_TRAILER_SIZE];
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
			SCLog(TRUE, LOG_ERR,
			      CFSTR("SCDynamicStore notification handler, kr=0x%x"),
			      kr);
			return;
		}

		msgid = notify_msg.msg.header.msgh_id;

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

	rlsCancel((void*)store, NULL, NULL);

	if (drainGroup != NULL) {
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

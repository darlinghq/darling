/*
 * Copyright (c) 2000-2006, 2008-2014 Apple Inc. All rights reserved.
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

#include <TargetConditionals.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <servers/bootstrap.h>
#include <bootstrap_priv.h>

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include "SCD.h"
#include "SCDynamicStoreInternal.h"
#include "config.h"		/* MiG generated file */


static CFStringRef		_sc_bundleID	= NULL;
static pthread_mutex_t		_sc_lock	= PTHREAD_MUTEX_INITIALIZER;
static mach_port_t		_sc_server	= MACH_PORT_NULL;


static const char	*notifyType[] = {
	"",
	"wait",
	"inform w/callback",
	"inform w/mach port",
	"inform w/fd",
	"inform w/signal",
	"inform w/runLoop",
	"inform w/dispatch"
};


static CFStringRef
__SCDynamicStoreCopyDescription(CFTypeRef cf) {
	CFAllocatorRef			allocator	= CFGetAllocator(cf);
	CFMutableStringRef		result;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)cf;

	result = CFStringCreateMutable(allocator, 0);
	CFStringAppendFormat(result, NULL, CFSTR("<SCDynamicStore %p [%p]> {"), cf, allocator);
	if (storePrivate->server != MACH_PORT_NULL) {
		CFStringAppendFormat(result, NULL, CFSTR("server port = 0x%x"), storePrivate->server);
	} else {
		CFStringAppendFormat(result, NULL, CFSTR("server not (no longer) available"));
	}
	if (storePrivate->disconnectFunction != NULL) {
		CFStringAppendFormat(result, NULL, CFSTR(", disconnect = %p"), storePrivate->disconnectFunction);
	}
	switch (storePrivate->notifyStatus) {
		case Using_NotifierWait :
			CFStringAppendFormat(result, NULL, CFSTR(", waiting for a notification"));
			break;
		case Using_NotifierInformViaMachPort :
			CFStringAppendFormat(result, NULL, CFSTR(", mach port notifications"));
			break;
		case Using_NotifierInformViaFD :
			CFStringAppendFormat(result, NULL, CFSTR(", FD notifications"));
			break;
		case Using_NotifierInformViaSignal :
			CFStringAppendFormat(result, NULL, CFSTR(", BSD signal notifications"));
			break;
		case Using_NotifierInformViaRunLoop :
		case Using_NotifierInformViaDispatch :
			if (storePrivate->notifyStatus == Using_NotifierInformViaRunLoop) {
				CFStringAppendFormat(result, NULL, CFSTR(", runloop notifications"));
				CFStringAppendFormat(result, NULL, CFSTR(" {callout = %p"), storePrivate->rlsFunction);
				CFStringAppendFormat(result, NULL, CFSTR(", info = %p"), storePrivate->rlsContext.info);
				CFStringAppendFormat(result, NULL, CFSTR(", rls = %p"), storePrivate->rls);
			} else if (storePrivate->notifyStatus == Using_NotifierInformViaDispatch) {
				CFStringAppendFormat(result, NULL, CFSTR(", dispatch notifications"));
				CFStringAppendFormat(result, NULL, CFSTR(" {callout = %p"), storePrivate->rlsFunction);
				CFStringAppendFormat(result, NULL, CFSTR(", info = %p"), storePrivate->rlsContext.info);
				CFStringAppendFormat(result, NULL, CFSTR(", queue = %p"), storePrivate->dispatchQueue);
				CFStringAppendFormat(result, NULL, CFSTR(", source = %p"), storePrivate->dispatchSource);
			}
			if (storePrivate->rlsNotifyRLS != NULL) {
				CFStringAppendFormat(result, NULL, CFSTR(", notify rls = %@" ), storePrivate->rlsNotifyRLS);
			}
			CFStringAppendFormat(result, NULL, CFSTR("}"));
			break;
		default :
			CFStringAppendFormat(result, NULL, CFSTR(", notification delivery not requested%s"),
					     storePrivate->rlsFunction ? " (yet)" : "");
			break;
	}
	CFStringAppendFormat(result, NULL, CFSTR("}"));

	return result;
}


static void
__SCDynamicStoreDeallocate(CFTypeRef cf)
{
	int				oldThreadState;
	SCDynamicStoreRef		store		= (SCDynamicStoreRef)cf;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	(void) pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldThreadState);

	/* Remove/cancel any outstanding notification requests. */
	(void) SCDynamicStoreNotifyCancel(store);

	if (storePrivate->server != MACH_PORT_NULL) {
		/*
		 * Remove our send right to the SCDynamicStore server.
		 *
		 * In the case of a "real" session this will result in our
		 * session being closed.
		 *
		 * In the case of a "NULL" session, we just remove the
		 * the send right reference we are holding.
		 */
		__MACH_PORT_DEBUG(TRUE, "*** __SCDynamicStoreDeallocate", storePrivate->server);
		(void) mach_port_deallocate(mach_task_self(), storePrivate->server);
		storePrivate->server = MACH_PORT_NULL;
	}

	(void) pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldThreadState);
	pthread_testcancel();

	/* release any callback context info */
	if (storePrivate->rlsContext.release != NULL) {
		(*storePrivate->rlsContext.release)(storePrivate->rlsContext.info);
	}

	/* release any keys being watched */
	if (storePrivate->keys != NULL) CFRelease(storePrivate->keys);
	if (storePrivate->patterns != NULL) CFRelease(storePrivate->patterns);

	/* release any client info */
	if (storePrivate->name != NULL) CFRelease(storePrivate->name);
	if (storePrivate->options != NULL) CFRelease(storePrivate->options);

	return;
}


static CFTypeID __kSCDynamicStoreTypeID = _kCFRuntimeNotATypeID;


static const CFRuntimeClass __SCDynamicStoreClass = {
	0,				// version
	"SCDynamicStore",		// className
	NULL,				// init
	NULL,				// copy
	__SCDynamicStoreDeallocate,	// dealloc
	NULL,				// equal
	NULL,				// hash
	NULL,				// copyFormattingDesc
	__SCDynamicStoreCopyDescription	// copyDebugDesc
};


static void
childForkHandler()
{
	/* the process has forked (and we are the child process) */

	_sc_server = MACH_PORT_NULL;
	return;
}


static pthread_once_t initialized	= PTHREAD_ONCE_INIT;

static void
__SCDynamicStoreInitialize(void)
{
	CFBundleRef	bundle;

	/* register with CoreFoundation */
	__kSCDynamicStoreTypeID = _CFRuntimeRegisterClass(&__SCDynamicStoreClass);

	/* add handler to cleanup after fork() */
	(void) pthread_atfork(NULL, NULL, childForkHandler);

	/* get the application/executable/bundle name */
	bundle = CFBundleGetMainBundle();
	if (bundle != NULL) {
		_sc_bundleID = CFBundleGetIdentifier(bundle);
		if (_sc_bundleID != NULL) {
			CFRetain(_sc_bundleID);
		} else {
			CFURLRef	url;

			url = CFBundleCopyExecutableURL(bundle);
			if (url != NULL) {
				_sc_bundleID = CFURLCopyPath(url);
				CFRelease(url);
			}
		}

		if (_sc_bundleID != NULL) {
			if (CFEqual(_sc_bundleID, CFSTR("/"))) {
				CFRelease(_sc_bundleID);
				_sc_bundleID = CFStringCreateWithFormat(NULL, NULL, CFSTR("(%d)"), getpid());
			}
		}
	}

	return;
}


static mach_port_t
__SCDynamicStoreServerPort(SCDynamicStorePrivateRef storePrivate, kern_return_t *status)
{
	mach_port_t	server	= MACH_PORT_NULL;
	char		*server_name;

	server_name = getenv("SCD_SERVER");


	if (server_name == NULL) {
		server_name = SCD_SERVER;
	}

#if	defined(BOOTSTRAP_PRIVILEGED_SERVER) && !TARGET_IPHONE_SIMULATOR
	*status = bootstrap_look_up2(bootstrap_port,
				     server_name,
				     &server,
				     0,
				     BOOTSTRAP_PRIVILEGED_SERVER);
#else	// defined(BOOTSTRAP_PRIVILEGED_SERVER) && !TARGET_IPHONE_SIMULATOR
	*status = bootstrap_look_up(bootstrap_port, server_name, &server);
#endif	// defined(BOOTSTRAP_PRIVILEGED_SERVER) && !TARGET_IPHONE_SIMULATOR

	switch (*status) {
		case BOOTSTRAP_SUCCESS :
			/* service currently registered, "a good thing" (tm) */
			return server;
		case BOOTSTRAP_NOT_PRIVILEGED :
			/* the service is not privileged */
			break;
		case BOOTSTRAP_UNKNOWN_SERVICE :
			/* service not currently registered, try again later */
			break;
		default :
#ifdef	DEBUG
			SCLog(_sc_verbose, LOG_DEBUG,
			      CFSTR("SCDynamicStoreCreate[WithOptions] bootstrap_look_up() failed: status=%s"),
			      bootstrap_strerror(*status));
#endif	/* DEBUG */
			break;
	}

	return MACH_PORT_NULL;
}


SCDynamicStorePrivateRef
__SCDynamicStoreCreatePrivate(CFAllocatorRef		allocator,
			     const CFStringRef		name,
			     SCDynamicStoreCallBack	callout,
			     SCDynamicStoreContext	*context)
{
	uint32_t			size;
	SCDynamicStorePrivateRef	storePrivate;

	/* initialize runtime */
	pthread_once(&initialized, __SCDynamicStoreInitialize);


	/* allocate session */
	size  = sizeof(SCDynamicStorePrivate) - sizeof(CFRuntimeBase);
	storePrivate = (SCDynamicStorePrivateRef)_CFRuntimeCreateInstance(allocator,
									  __kSCDynamicStoreTypeID,
									  size,
									  NULL);
	if (storePrivate == NULL) {
		_SCErrorSet(kSCStatusFailed);
		return NULL;
	}

	/* client side of the "configd" session */
	storePrivate->name				= (name != NULL) ? CFRetain(name) : NULL;
	storePrivate->options				= NULL;

	/* server side of the "configd" session */
	storePrivate->server				= MACH_PORT_NULL;
	storePrivate->serverNullSession			= FALSE;

	/* flags */
	storePrivate->useSessionKeys			= FALSE;

	/* Notification status */
	storePrivate->notifyStatus			= NotifierNotRegistered;

	/* "client" information associated with SCDynamicStoreCreateRunLoopSource() */
	storePrivate->rlList				= NULL;
	storePrivate->rls				= NULL;
	storePrivate->rlsFunction			= callout;
	storePrivate->rlsContext.info			= NULL;
	storePrivate->rlsContext.retain			= NULL;
	storePrivate->rlsContext.release		= NULL;
	storePrivate->rlsContext.copyDescription	= NULL;
	if (context) {
		bcopy(context, &storePrivate->rlsContext, sizeof(SCDynamicStoreContext));
		if (context->retain != NULL) {
			storePrivate->rlsContext.info = (void *)(*context->retain)(context->info);
		}
	}
	storePrivate->rlsNotifyPort			= NULL;
	storePrivate->rlsNotifyRLS			= NULL;

	/* "client" information associated with SCDynamicStoreSetDispatchQueue() */
	storePrivate->dispatchGroup			= NULL;
	storePrivate->dispatchQueue			= NULL;
	storePrivate->dispatchSource			= NULL;

	/* "client" information associated with SCDynamicStoreSetDisconnectCallBack() */
	storePrivate->disconnectFunction		= NULL;
	storePrivate->disconnectForceCallBack		= FALSE;

	/* "server" information associated with SCDynamicStoreSetNotificationKeys() */
	storePrivate->keys				= NULL;
	storePrivate->patterns				= NULL;

	/* "server" information associated with SCDynamicStoreNotifyMachPort(); */
	storePrivate->notifyPort			= MACH_PORT_NULL;
	storePrivate->notifyPortIdentifier		= 0;

	/* "server" information associated with SCDynamicStoreNotifyFileDescriptor(); */
	storePrivate->notifyFile			= -1;
	storePrivate->notifyFileIdentifier		= 0;

	/* "server" information associated with SCDynamicStoreNotifySignal(); */
	storePrivate->notifySignal			= 0;
	storePrivate->notifySignalTask			= TASK_NULL;

	return storePrivate;
}


static void
updateServerPort(SCDynamicStorePrivateRef storePrivate, mach_port_t *server, int *sc_status_p)
{
	pthread_mutex_lock(&_sc_lock);
	if (_sc_server != MACH_PORT_NULL) {
		if (*server == _sc_server) {
			mach_port_t	old_port;

			// if the server we tried returned the error, save the old port,
			// [re-]lookup the name to the server, and deallocate the original
			// send [or dead name] right

			old_port = _sc_server;
			_sc_server = __SCDynamicStoreServerPort(storePrivate, sc_status_p);
			(void)mach_port_deallocate(mach_task_self(), old_port);
		} else {
			// another thread has refreshed the [main] SCDynamicStore server port
		}
	} else {
		_sc_server = __SCDynamicStoreServerPort(storePrivate, sc_status_p);
	}
	*server = _sc_server;
	pthread_mutex_unlock(&_sc_lock);

	return;
}


static Boolean
__SCDynamicStoreAddSession(SCDynamicStorePrivateRef storePrivate)
{
	kern_return_t		kr		= KERN_SUCCESS;
	CFDataRef		myName;			/* serialized name */
	xmlData_t		myNameRef;
	CFIndex			myNameLen;
	CFDataRef		myOptions	= NULL;	/* serialized options */
	xmlData_t		myOptionsRef	= NULL;
	CFIndex			myOptionsLen	= 0;
	int			sc_status	= kSCStatusFailed;
	mach_port_t		server;

	if (!_SCSerializeString(storePrivate->name, &myName, (void **)&myNameRef, &myNameLen)) {
		goto done;
	}

	/* serialize the options */
	if (storePrivate->options != NULL) {
		if (!_SCSerialize(storePrivate->options, &myOptions, (void **)&myOptionsRef, &myOptionsLen)) {
			CFRelease(myName);
			goto done;
		}
	}

	/* open a new session with the server */
	server = MACH_PORT_NULL;


	updateServerPort(storePrivate, &server, &sc_status);


	while (server != MACH_PORT_NULL) {
		// if SCDynamicStore server available

		if (!storePrivate->serverNullSession) {
			// if SCDynamicStore session
			kr = configopen(server,
					myNameRef,
					(mach_msg_type_number_t)myNameLen,
					myOptionsRef,
					(mach_msg_type_number_t)myOptionsLen,
					&storePrivate->server,
					(int *)&sc_status);
		} else {
			// if NULL session
			if (storePrivate->server == MACH_PORT_NULL) {
				// use the [main] SCDynamicStore server port
				kr = mach_port_mod_refs(mach_task_self(), server, MACH_PORT_RIGHT_SEND, +1);
				if (kr == KERN_SUCCESS) {
					storePrivate->server = server;
					sc_status = kSCStatusOK;
				} else {
					storePrivate->server = MACH_PORT_NULL;
				}
			} else {
				// if the server port we used returned an error
				storePrivate->server = MACH_PORT_NULL;
				kr = MACH_SEND_INVALID_DEST;
			}
		}

		if (kr == KERN_SUCCESS) {
			break;
		}

		// our [cached] server port is not valid
		if ((kr != MACH_SEND_INVALID_DEST) && (kr != MIG_SERVER_DIED)) {
			// if we got an unexpected error, don't retry
			sc_status = kr;
			break;
		}


		updateServerPort(storePrivate, &server, &sc_status);
	}
	__MACH_PORT_DEBUG(TRUE, "*** SCDynamicStoreAddSession", storePrivate->server);

	// clean up
	CFRelease(myName);
	if (myOptions != NULL)	CFRelease(myOptions);

    done :

	switch (sc_status) {
		case kSCStatusOK :
			return TRUE;
		case BOOTSTRAP_UNKNOWN_SERVICE :
			SCLog(TRUE,
			      (kr == KERN_SUCCESS) ? LOG_DEBUG : LOG_ERR,
			      CFSTR("SCDynamicStore server not available"));
			sc_status = kSCStatusNoStoreServer;
			break;
		default :
			SCLog(TRUE,
			      (kr == KERN_SUCCESS) ? LOG_DEBUG : LOG_ERR,
			      CFSTR("SCDynamicStoreAddSession configopen(): %s"),
			      SCErrorString(sc_status));
			break;
	}

	_SCErrorSet(sc_status);
	return FALSE;
}


__private_extern__
SCDynamicStoreRef
__SCDynamicStoreNullSession(void)
{
	SCDynamicStorePrivateRef	storePrivate;
	Boolean				ok	= TRUE;
	__SCThreadSpecificDataRef	tsd;

	tsd = __SCGetThreadSpecificData();
	if (tsd->_sc_store == NULL) {
		storePrivate = __SCDynamicStoreCreatePrivate(NULL,
							     CFSTR("NULL session"),
							     NULL,
							     NULL);
		assert(storePrivate != NULL);
		storePrivate->serverNullSession = TRUE;
		tsd->_sc_store = (SCDynamicStoreRef)storePrivate;
	}

	storePrivate = (SCDynamicStorePrivateRef)tsd->_sc_store;
	if (storePrivate->server == MACH_PORT_NULL) {
		ok = __SCDynamicStoreAddSession(storePrivate);
	}

	return ok ? tsd->_sc_store : NULL;
}


static Boolean
__SCDynamicStoreReconnect(SCDynamicStoreRef store)
{
	Boolean				ok;
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	ok = __SCDynamicStoreAddSession(storePrivate);
	return ok;
}


__private_extern__
Boolean
__SCDynamicStoreCheckRetryAndHandleError(SCDynamicStoreRef	store,
					 kern_return_t		status,
					 int			*sc_status,
					 const char		*log_str)
{
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	if (status == KERN_SUCCESS) {
		/* no error */
		return FALSE;
	}

	if ((status == MACH_SEND_INVALID_DEST) || (status == MIG_SERVER_DIED)) {
		/* the server's gone, remove the session's dead name right */
		(void) mach_port_deallocate(mach_task_self(), storePrivate->server);
		storePrivate->server = MACH_PORT_NULL;

		/* reconnect */
		if (__SCDynamicStoreReconnect(store)) {
			/* retry needed */
			return TRUE;
		}
	} else {
		/* an unexpected error, leave the [session] port alone */
		SCLog(TRUE, LOG_ERR, CFSTR("%s: %s"), log_str, mach_error_string(status));
		storePrivate->server = MACH_PORT_NULL;
	}

	*sc_status = status;
	return FALSE;
}


static void
pushDisconnect(SCDynamicStoreRef store)
{
	void					*context_info;
	void					(*context_release)(const void *);
	SCDynamicStoreDisconnectCallBack	disconnectFunction;
	SCDynamicStorePrivateRef		storePrivate	= (SCDynamicStorePrivateRef)store;

	disconnectFunction = storePrivate->disconnectFunction;
	if (disconnectFunction == NULL) {
		// if no reconnect callout, push empty notification
		storePrivate->disconnectForceCallBack = TRUE;
		return;
	}

	if (storePrivate->rlsContext.retain != NULL) {
		context_info	= (void *)storePrivate->rlsContext.retain(storePrivate->rlsContext.info);
		context_release	= storePrivate->rlsContext.release;
	} else {
		context_info	= storePrivate->rlsContext.info;
		context_release	= NULL;
	}
	(*disconnectFunction)(store, context_info);
	if (context_release) {
		context_release(context_info);
	}

	return;
}


__private_extern__
Boolean
__SCDynamicStoreReconnectNotifications(SCDynamicStoreRef store)
{
	dispatch_queue_t			dispatchQueue	= NULL;
	__SCDynamicStoreNotificationStatus	notifyStatus;
	Boolean					ok		= TRUE;
	CFArrayRef				rlList		= NULL;
	SCDynamicStorePrivateRef		storePrivate	= (SCDynamicStorePrivateRef)store;

	// save old SCDynamicStore [notification] state
	notifyStatus = storePrivate->notifyStatus;

	// before tearing down our [old] notifications, make sure we've
	// retained any information that will be lost when we cancel the
	// current no-longer-valid handler
	switch (notifyStatus) {
		case Using_NotifierInformViaRunLoop :
			if (storePrivate->rlList != NULL) {
				rlList = CFArrayCreateCopy(NULL, storePrivate->rlList);
			}
		case Using_NotifierInformViaDispatch :
			dispatchQueue = storePrivate->dispatchQueue;
			if (dispatchQueue != NULL) dispatch_retain(dispatchQueue);
			break;
		default :
			break;
	}

	// cancel [old] notifications
	if (!SCDynamicStoreNotifyCancel(store)) {
		// if we could not cancel / reconnect
		SCLog(TRUE, LOG_DEBUG,
		      CFSTR("__SCDynamicStoreReconnectNotifications: SCDynamicStoreNotifyCancel() failed: %s"),
		      SCErrorString(SCError()));
	}

	// set notification keys & patterns
	if ((storePrivate->keys != NULL) || (storePrivate->patterns)) {
		ok = SCDynamicStoreSetNotificationKeys(store,
						       storePrivate->keys,
						       storePrivate->patterns);
		if (!ok) {
			SCLog((SCError() != BOOTSTRAP_UNKNOWN_SERVICE),
			      LOG_ERR,
			      CFSTR("__SCDynamicStoreReconnectNotifications: SCDynamicStoreSetNotificationKeys() failed"));
			goto done;
		}
	}

	switch (notifyStatus) {
		case Using_NotifierInformViaRunLoop : {
			CFIndex			i;
			CFIndex			n;
			CFRunLoopSourceRef	rls;

			rls = SCDynamicStoreCreateRunLoopSource(NULL, store, 0);
			if (rls == NULL) {
				SCLog((SCError() != BOOTSTRAP_UNKNOWN_SERVICE),
				      LOG_ERR,
				      CFSTR("__SCDynamicStoreReconnectNotifications: SCDynamicStoreCreateRunLoopSource() failed"));
				ok = FALSE;
				break;
			}

			n = (rlList != NULL) ? CFArrayGetCount(rlList) : 0;
			for (i = 0; i < n; i += 3) {
				CFRunLoopRef	rl	= (CFRunLoopRef)CFArrayGetValueAtIndex(rlList, i+1);
				CFStringRef	rlMode	= (CFStringRef) CFArrayGetValueAtIndex(rlList, i+2);

				CFRunLoopAddSource(rl, rls, rlMode);
			}

			CFRelease(rls);
			break;
		}
		case Using_NotifierInformViaDispatch :
			ok = SCDynamicStoreSetDispatchQueue(store, dispatchQueue);
			if (!ok) {
				SCLog((SCError() != BOOTSTRAP_UNKNOWN_SERVICE),
				      LOG_ERR,
				      CFSTR("__SCDynamicStoreReconnectNotifications: SCDynamicStoreSetDispatchQueue() failed"));
				goto done;
			}
			break;

		default :
			_SCErrorSet(kSCStatusFailed);
			ok = FALSE;
			break;
	}

    done :

	// cleanup
	switch (notifyStatus) {
		case Using_NotifierInformViaRunLoop :
			if (rlList != NULL) CFRelease(rlList);
			break;
		case Using_NotifierInformViaDispatch :
			if (dispatchQueue != NULL) dispatch_release(dispatchQueue);
			break;
		default :
			break;
	}

	if (!ok) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("SCDynamicStore server %s, notification (%s) not restored"),
		      (SCError() == BOOTSTRAP_UNKNOWN_SERVICE) ? "shutdown" : "failed",
		      notifyType[notifyStatus]);
	}

	// inform the client
	pushDisconnect(store);

	return ok;
}


const CFStringRef	kSCDynamicStoreUseSessionKeys	= CFSTR("UseSessionKeys");	/* CFBoolean */



SCDynamicStoreRef
SCDynamicStoreCreateWithOptions(CFAllocatorRef		allocator,
				CFStringRef		name,
				CFDictionaryRef		storeOptions,
				SCDynamicStoreCallBack	callout,
				SCDynamicStoreContext	*context)
{
	Boolean				ok;
	SCDynamicStorePrivateRef	storePrivate;

	// allocate and initialize a new session
	storePrivate = __SCDynamicStoreCreatePrivate(allocator, NULL, callout, context);
	if (storePrivate == NULL) {
		return NULL;
	}

	// set "name"
	if (_sc_bundleID != NULL) {
		storePrivate->name = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@:%@"), _sc_bundleID, name);
	} else {
		storePrivate->name = CFRetain(name);
	}

	// set "options"

	if (storeOptions != NULL) {
		storePrivate->options = CFRetain(storeOptions);
	}

	// establish SCDynamicStore session
	ok = __SCDynamicStoreAddSession(storePrivate);
	if (!ok) {
		CFRelease(storePrivate);
		storePrivate = NULL;
	}

	return (SCDynamicStoreRef)storePrivate;
}


SCDynamicStoreRef
SCDynamicStoreCreate(CFAllocatorRef		allocator,
		     CFStringRef		name,
		     SCDynamicStoreCallBack	callout,
		     SCDynamicStoreContext	*context)
{
	return SCDynamicStoreCreateWithOptions(allocator, name, NULL, callout, context);
}


CFTypeID
SCDynamicStoreGetTypeID(void) {
	pthread_once(&initialized, __SCDynamicStoreInitialize);	/* initialize runtime */
	return __kSCDynamicStoreTypeID;
}

Boolean
SCDynamicStoreSetDisconnectCallBack(SCDynamicStoreRef			store,
				    SCDynamicStoreDisconnectCallBack	callout)
{
	SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)store;

	if (store == NULL) {
		/* sorry, you must provide a session */
		_SCErrorSet(kSCStatusNoStoreSession);
		return FALSE;
	}

	storePrivate->disconnectFunction = callout;
	return TRUE;
}

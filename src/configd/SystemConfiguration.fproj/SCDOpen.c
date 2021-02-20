/*
 * Copyright (c) 2000-2006, 2008-2019 Apple Inc. All rights reserved.
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

#include "SCDynamicStoreInternal.h"
#include "config.h"		/* MiG generated file */
#include "SCD.h"

static CFStringRef		_sc_bundleID	= NULL;
static pthread_mutex_t		_sc_lock	= PTHREAD_MUTEX_INITIALIZER;
static mach_port_t		_sc_server	= MACH_PORT_NULL;
static unsigned int		_sc_store_cnt	= 0;
static unsigned int		_sc_store_max	= 50;	/* complain if SCDynamicStore objects exceeds this [soft-]limit */


static const char	*notifyType[] = {
	"",
	"wait",
	"inform w/callback",
	"inform w/mach port",
	"inform w/fd",
	"inform w/runLoop",
	"inform w/dispatch"
};


void
_SCDynamicStoreSetSessionWatchLimit(unsigned int limit)
{
	_sc_store_max = limit;
	return;
}


__private_extern__ os_log_t
__log_SCDynamicStore(void)
{
	static os_log_t	log	= NULL;

	if (log == NULL) {
		log = os_log_create("com.apple.SystemConfiguration", "SCDynamicStore");
	}

	return log;
}


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
		case Using_NotifierInformViaRunLoop :
		case Using_NotifierInformViaDispatch :
			if (storePrivate->notifyStatus == Using_NotifierInformViaRunLoop) {
				CFStringAppendFormat(result, NULL, CFSTR(", runloop notifications"));
				CFStringAppendFormat(result, NULL, CFSTR(" {callout = %p"), storePrivate->rlsFunction);
				CFStringAppendFormat(result, NULL, CFSTR(", info = %p"), storePrivate->rlsContext.info);
				CFStringAppendFormat(result, NULL, CFSTR(", rls = %p"), storePrivate->rls);
				CFStringAppendFormat(result, NULL, CFSTR(", notify rls = %@" ), storePrivate->rlsNotifyRLS);
			} else if (storePrivate->notifyStatus == Using_NotifierInformViaDispatch) {
				CFStringAppendFormat(result, NULL, CFSTR(", dispatch notifications"));
				CFStringAppendFormat(result, NULL, CFSTR(" {callout = %p"), storePrivate->rlsFunction);
				CFStringAppendFormat(result, NULL, CFSTR(", info = %p"), storePrivate->rlsContext.info);
				CFStringAppendFormat(result, NULL, CFSTR(", queue = %p"), storePrivate->dispatchQueue);
				CFStringAppendFormat(result, NULL, CFSTR(", source = %p"), storePrivate->dispatchSource);
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

	/* release any cached content */
	if (storePrivate->cache_active) {
		_SCDynamicStoreCacheClose(store);
	}

	_SC_ATOMIC_DEC(&_sc_store_cnt);

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
#pragma unused(storePrivate)
	mach_port_t	server	= MACH_PORT_NULL;
	char		*server_name;

	server_name = getenv("SCD_SERVER");

#ifndef	DEBUG
	/*
	 * only allow the SCDynamicStore server bootstrap name to be changed with
	 * DEBUG builds.  For RELEASE builds, assume that no server is available.
	 */
	if (server_name != NULL) {
		*status = BOOTSTRAP_UNKNOWN_SERVICE;
		return MACH_PORT_NULL;
	}
#endif	/* DEBUG */


	if (server_name == NULL) {
		server_name = SCD_SERVER;
	}

#if	defined(BOOTSTRAP_PRIVILEGED_SERVER) && !TARGET_OS_SIMULATOR
	*status = bootstrap_look_up2(bootstrap_port,
				     server_name,
				     &server,
				     0,
				     BOOTSTRAP_PRIVILEGED_SERVER);
#else	// defined(BOOTSTRAP_PRIVILEGED_SERVER) && !TARGET_OS_SIMULATOR
	*status = bootstrap_look_up(bootstrap_port, server_name, &server);
#endif	// defined(BOOTSTRAP_PRIVILEGED_SERVER) && !TARGET_OS_SIMULATOR

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
			SC_log(LOG_INFO, "bootstrap_look_up() failed: status=%s (%d)",
			       bootstrap_strerror(*status),
			       *status);
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
	unsigned int			n;
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

	/* initialize non-zero/NULL members */

	/* client side of the "configd" session */
	storePrivate->name				= (name != NULL) ? CFRetain(name) : NULL;

	/* Notification status */
	storePrivate->notifyStatus			= NotifierNotRegistered;

	/* "client" information associated with SCDynamicStoreCreateRunLoopSource() */
	storePrivate->rlsFunction			= callout;
	if (context != NULL) {
		memcpy(&storePrivate->rlsContext, context, sizeof(SCDynamicStoreContext));
		if (context->retain != NULL) {
			storePrivate->rlsContext.info = (void *)(*context->retain)(context->info);
		}
	}

	/* "server" information associated with SCDynamicStoreNotifyFileDescriptor(); */
	storePrivate->notifyFile			= -1;

	/* watch for excessive SCDynamicStore usage */
	n = _SC_ATOMIC_INC(&_sc_store_cnt);
	if (n > _sc_store_max) {
		if (_sc_store_max > 0) {
			SC_log(LOG_ERR, "SCDynamicStoreCreate(): number of SCDynamicStore objects now exceeds %u", n - 1);
			_sc_store_max = (_sc_store_max < 5000) ? (_sc_store_max * 2) : 0;
			_SC_crash_once("Excessive number of SCDynamicStore objects", NULL, NULL);
		}
	}

	return storePrivate;
}


static void
updateServerPort(SCDynamicStorePrivateRef storePrivate, mach_port_t *server, int *sc_status_p)
{
	mach_port_t	old_port;

	pthread_mutex_lock(&_sc_lock);
	old_port = _sc_server;
	if (_sc_server != MACH_PORT_NULL) {
		if (*server == _sc_server) {
			// if the server we tried returned the error, save the old port,
			// [re-]lookup the name to the server, and deallocate the original
			// send [or dead name] right
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

#ifdef	DEBUG
	SC_log(LOG_DEBUG, "updateServerPort (%@): 0x%x (%d) --> 0x%x (%d)",
	       (storePrivate->name != NULL) ? storePrivate->name : CFSTR("?"),
	       old_port, old_port,
	       *server, *server);
#endif	// DEBUG

	return;
}


static Boolean
__SCDynamicStoreAddSession(SCDynamicStorePrivateRef storePrivate)
{
	kern_return_t				kr		= KERN_SUCCESS;
	CFDataRef				myName;			/* serialized name */
	xmlData_t				myNameRef;
	CFIndex					myNameLen;
	CFDataRef				myOptions	= NULL;	/* serialized options */
	xmlData_t				myOptionsRef	= NULL;
	CFIndex					myOptionsLen	= 0;
	int					sc_status	= kSCStatusFailed;
	mach_port_t				server;

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
					if (kr == KERN_INVALID_RIGHT) {
						// We can get KERN_INVALID_RIGHT if the server dies and we try to
						// add a send right to a stale (now dead) port name
						kr = MACH_SEND_INVALID_DEST;
					}
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
			SC_log((kr == KERN_SUCCESS) ? LOG_INFO : LOG_ERR, "SCDynamicStore server not available");
			sc_status = kSCStatusNoStoreServer;
			break;
		default :
			SC_log((kr == KERN_SUCCESS) ? LOG_INFO : LOG_ERR, "configopen() failed: %d: %s",
			       sc_status,
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

	switch (status) {
		case MACH_SEND_INVALID_DEST :
		case MACH_SEND_INVALID_RIGHT :
		case MIG_SERVER_DIED :
			/*
			 * the server's gone, remove the session's send (or dead name) right
			 */
#ifdef	DEBUG
			SC_log(LOG_DEBUG, "__SCDynamicStoreCheckRetryAndHandleError(%s): %@: 0x%x (%d) --> 0x%x (%d)",
			       log_str,
			       (storePrivate->name != NULL) ? storePrivate->name : CFSTR("?"),
			       storePrivate->server, storePrivate->server,
			       MACH_PORT_NULL, MACH_PORT_NULL);
#endif	// DEBUG
			(void) mach_port_deallocate(mach_task_self(), storePrivate->server);
			storePrivate->server = MACH_PORT_NULL;

			/* reconnect */
			if (__SCDynamicStoreReconnect(store)) {
				/* retry needed */
				return TRUE;
			} else {
				status = SCError();
			}
			;;

		default :
			/*
			 * an unexpected error, leave the [session] port alone
			 */
#ifdef	DEBUG
			SC_log(LOG_DEBUG, "__SCDynamicStoreCheckRetryAndHandleError(%s): %@: unexpected status=%s (0x%x)",
			       log_str,
			       (storePrivate->name != NULL) ? storePrivate->name : CFSTR("?"),
			       mach_error_string(status),
			       status);
#endif	// DEBUG

			SC_log(LOG_NOTICE, "%s: %s", log_str, mach_error_string(status));
			storePrivate->server = MACH_PORT_NULL;
			;;
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
	SC_log(LOG_DEBUG, "exec SCDynamicStore disconnect callout");
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

#ifdef	DEBUG
	SC_log(LOG_DEBUG, "SCDynamicStore: reconnect notifications (%@)",
	       (storePrivate->name != NULL) ? storePrivate->name : CFSTR("?"));
#endif	// DEBUG

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
			break;
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
		SC_log(LOG_NOTICE, "SCDynamicStoreNotifyCancel() failed: %s", SCErrorString(SCError()));
	}

	// set notification keys & patterns
	if ((storePrivate->keys != NULL) || (storePrivate->patterns)) {
		ok = SCDynamicStoreSetNotificationKeys(store,
						       storePrivate->keys,
						       storePrivate->patterns);
		if (!ok) {
			if (SCError() != BOOTSTRAP_UNKNOWN_SERVICE) {
				SC_log(LOG_NOTICE, "SCDynamicStoreSetNotificationKeys() failed");
			}
			goto done;
		}
	}

	switch (notifyStatus) {
		case Using_NotifierInformViaRunLoop : {
			CFIndex			i;
			CFIndex			n;
			CFRunLoopSourceRef	rls;

#ifdef	DEBUG
			SC_log(LOG_DEBUG, "SCDynamicStore: reconnecting w/CFRunLoop (%@)",
			       (storePrivate->name != NULL) ? storePrivate->name : CFSTR("?"));
#endif	// DEBUG

			rls = SCDynamicStoreCreateRunLoopSource(NULL, store, 0);
			if (rls == NULL) {
				if (SCError() != BOOTSTRAP_UNKNOWN_SERVICE) {
					SC_log(LOG_NOTICE, "SCDynamicStoreCreateRunLoopSource() failed");
				}
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

#ifdef	DEBUG
			SC_log(LOG_DEBUG, "SCDynamicStore: reconnecting w/dispatch queue (%@)",
			       (storePrivate->name != NULL) ? storePrivate->name : CFSTR("?"));
#endif	// DEBUG

			ok = SCDynamicStoreSetDispatchQueue(store, dispatchQueue);
			if (!ok) {
				if (SCError() != BOOTSTRAP_UNKNOWN_SERVICE) {
					SC_log(LOG_NOTICE, "SCDynamicStoreSetDispatchQueue() failed");
				}
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
		SC_log(LOG_NOTICE, "SCDynamicStore server %s, notification (%s) not restored",
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

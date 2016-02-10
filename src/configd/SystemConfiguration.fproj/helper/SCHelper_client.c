/*
 * Copyright (c) 2005-2008, 2010, 2011, 2013 Apple Inc. All rights reserved.
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

#include <fcntl.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <servers/bootstrap.h>
#include <bootstrap_priv.h>
#include <pthread.h>

#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SCPrivate.h>

#include "SCHelper_client.h"
#include "helper.h"		// MiG generated file


#define	HELPER					"SCHelper"
#define	HELPER_LEN				(sizeof(HELPER) - 1)

#define	SUFFIX_SYM				"~sym"
#define	SUFFIX_SYM_LEN				(sizeof(SUFFIX_SYM) - 1)


static pthread_mutex_t	_helper_lock	= PTHREAD_MUTEX_INITIALIZER;
static mach_port_t	_helper_server	= MACH_PORT_NULL;


static mach_port_t
__SCHelperServerPort(kern_return_t *status)
{
	mach_port_t	server	= MACH_PORT_NULL;
	char		*server_name;

	server_name = getenv("SCHELPER_SERVER");
	if (!server_name) {
		server_name = SCHELPER_SERVER;
	}

#ifdef	BOOTSTRAP_PRIVILEGED_SERVER
	*status = bootstrap_look_up2(bootstrap_port,
				     server_name,
				     &server,
				     0,
				     BOOTSTRAP_PRIVILEGED_SERVER);
#else	// BOOTSTRAP_PRIVILEGED_SERVER
	*status = bootstrap_look_up(bootstrap_port, server_name, &server);
#endif	// BOOTSTRAP_PRIVILEGED_SERVER

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
			      CFSTR("__SCHelperServerPort bootstrap_look_up() failed: status=%s"),
			      bootstrap_strerror(*status));
#endif	/* DEBUG */
			break;
	}

	return MACH_PORT_NULL;
}


__private_extern__
Boolean
_SCHelperOpen(CFDataRef authorizationData, mach_port_t *helper_port)
{
	kern_return_t		kr;
	Boolean			ok;
	mach_port_t		server;
	uint32_t		status	= 0;

	*helper_port = MACH_PORT_NULL;

	// open a new session with the server
	server = _helper_server;
	while (TRUE) {
		if (server != MACH_PORT_NULL) {
			kr = helperinit(server,
					helper_port,
					&status);
			if (kr == KERN_SUCCESS) {
				break;
			}

			// our [cached] server port is not valid
			if (kr != MACH_SEND_INVALID_DEST) {
				// if we got an unexpected error, don't retry
				status = kr;
				break;
			}
		}

		pthread_mutex_lock(&_helper_lock);
		if (_helper_server != MACH_PORT_NULL) {
			if (server == _helper_server) {
				// if the server we tried returned the error
				(void)mach_port_deallocate(mach_task_self(), _helper_server);
				_helper_server = __SCHelperServerPort(&kr);
				if (_helper_server == MACH_PORT_NULL) {
					status = kr;
				}
			} else {
				// another thread has refreshed the SCHelper server port
			}
		} else {
			_helper_server = __SCHelperServerPort(&kr);
			if (_helper_server == MACH_PORT_NULL) {
				status = kr;
			}
		}
		server = _helper_server;
		pthread_mutex_unlock(&_helper_lock);

		if (server == MACH_PORT_NULL) {
			// if SCHelper server not available
			break;
		}
	}
	__MACH_PORT_DEBUG(TRUE, "*** _SCHelperOpen", *helper_port);

	if (*helper_port == MACH_PORT_NULL) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("_SCHelperOpen: could not contact server: %s"),
		      SCErrorString(status));
		return FALSE;
	}

	ok = _SCHelperExec(*helper_port, SCHELPER_MSG_AUTH, authorizationData, &status, NULL);
	if (!ok) {
		SCLog(TRUE, LOG_INFO, CFSTR("_SCHelperOpen: could not send authorization"));
		goto error;
	}

	ok = (status == 0);
	if (!ok) {
		SCLog(TRUE, LOG_INFO, CFSTR("could not start \"" HELPER "\", status = %u"), status);
		goto error;
	}

	return TRUE;

    error :

	if (*helper_port != MACH_PORT_NULL) {
		(void)mach_port_deallocate(mach_task_self(), *helper_port);
		*helper_port = MACH_PORT_NULL;
	}

	return FALSE;

}


__private_extern__
void
_SCHelperClose(mach_port_t *helper_port)
{
	if (!_SCHelperExec(*helper_port, SCHELPER_MSG_EXIT, NULL, NULL, NULL)) {
		SCLog(TRUE, LOG_INFO, CFSTR("_SCHelperOpen: could not send exit request"));
	}

	if (*helper_port != MACH_PORT_NULL) {
		(void)mach_port_deallocate(mach_task_self(), *helper_port);
		*helper_port = MACH_PORT_NULL;
	}

	return;
}


static CFDataRef
_SCHelperExecCopyBacktrace()
{
	static Boolean		loggingEnabled	= FALSE;
	static dispatch_once_t	once;
	CFDataRef		traceData	= NULL;

	dispatch_once(&once, ^{
		if(getenv("ENABLE_SCHELPER_BACKTRACES")) {
			loggingEnabled = TRUE;
		}
	});

	if (loggingEnabled) {
		CFStringRef	backtrace;

		backtrace = _SC_copyBacktrace();
		if (backtrace != NULL) {
			_SCSerializeString(backtrace, &traceData, NULL, NULL);
			CFRelease(backtrace);
		}
	}

	return traceData;
}


Boolean
_SCHelperExec(mach_port_t port, uint32_t msgID, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	kern_return_t		kr;
	CFDataRef		myData		= NULL;
	xmlDataOut_t		replyRef	= NULL;		/* raw bytes */
	mach_msg_type_number_t	replyLen	= 0;
	uint32_t		replyStatus	= 0;
	CFDataRef		traceData;

	traceData = _SCHelperExecCopyBacktrace();

	kr = helperexec(port,
			msgID,
			(data != NULL) ? (void *)CFDataGetBytePtr(data) : NULL,
			(data != NULL) ? (mach_msg_type_number_t)CFDataGetLength(data) : 0,
			(traceData != NULL) ? (void *)CFDataGetBytePtr(traceData) : NULL,
			(traceData != NULL) ? (mach_msg_type_number_t)CFDataGetLength(traceData) : 0,
			&replyStatus,
			&replyRef,
			&replyLen);

	if (traceData != NULL) {
		CFRelease(traceData);
	}

	if (kr != KERN_SUCCESS) {
		if (replyRef != NULL) {
			(void) vm_deallocate(mach_task_self(), (vm_address_t)replyRef, replyLen);
		}

		if (kr != MACH_SEND_INVALID_DEST) {
			// if we got an unexpected error
			SCLog(TRUE, LOG_ERR, CFSTR("_SCHelperExec() failed: %s"), mach_error_string(kr));
		}
		_SCErrorSet(kr);

		return FALSE;
	}

	// un-serialize the reply
	if (replyRef != NULL) {
		if (!_SCUnserializeData(&myData, replyRef, replyLen)) {
			return FALSE;
		}
	}

	if (status != NULL) {
		*status = replyStatus;
	}

	if (reply != NULL) {
		*reply = myData;
	} else if (myData != NULL) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("_SCHelperExec() data available with no place to go"));
		CFRelease(myData);
	}

	return TRUE;
}

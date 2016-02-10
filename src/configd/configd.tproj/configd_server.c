/*
 * Copyright (c) 2000-2011, 2013 Apple Inc. All rights reserved.
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
 * March 9, 2004		Allan Nathanson <ajn@apple.com>
 * - add DNS configuration server
 *
 * June 1, 2001			Allan Nathanson <ajn@apple.com>
 * - public API conversion
 *
 * March 24, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <TargetConditionals.h>
#include <sysexits.h>
#include <unistd.h>
#include <sys/types.h>
#include <servers/bootstrap.h>

#include "configd.h"
#include "configd_server.h"
#include "notify_server.h"
#include "session.h"

/* MiG generated externals and functions */
extern struct mig_subsystem	_config_subsystem;
extern boolean_t		config_server(mach_msg_header_t *, mach_msg_header_t *);

/* configd server port (for new session requests) */
static CFMachPortRef		configd_port		= NULL;

__private_extern__
boolean_t
config_demux(mach_msg_header_t *request, mach_msg_header_t *reply)
{
	Boolean				processed = FALSE;

	/*
	 * (attempt to) process SCDynamicStore requests.
	 */
	processed = config_server(request, reply);
	if (processed) {
		return TRUE;
	}

	/*
	 * (attempt to) process (NO MORE SENDERS) notification messages.
	 */
	processed = notify_server(request, reply);
	if (processed) {
		return TRUE;
	}

	/*
	 * unknown message ID, log and return an error.
	 */
	SCLog(TRUE, LOG_ERR, CFSTR("config_demux(): unknown message ID (%d) received"), request->msgh_id);
	reply->msgh_bits        = MACH_MSGH_BITS(MACH_MSGH_BITS_REMOTE(request->msgh_bits), 0);
	reply->msgh_remote_port = request->msgh_remote_port;
	reply->msgh_size        = sizeof(mig_reply_error_t);	/* Minimal size */
	reply->msgh_local_port  = MACH_PORT_NULL;
	reply->msgh_id          = request->msgh_id + 100;
	((mig_reply_error_t *)reply)->NDR = NDR_record;
	((mig_reply_error_t *)reply)->RetCode = MIG_BAD_ID;

	return FALSE;
}


#define	MACH_MSG_BUFFER_SIZE	128


__private_extern__
void
configdCallback(CFMachPortRef port, void *msg, CFIndex size, void *info)
{
	mig_reply_error_t *	bufRequest	= msg;
	uint32_t		bufReply_q[MACH_MSG_BUFFER_SIZE/sizeof(uint32_t)];
	mig_reply_error_t *	bufReply	= (mig_reply_error_t *)bufReply_q;
	static CFIndex		bufSize		= 0;
	mach_msg_return_t	mr;
	int			options;

	if (bufSize == 0) {
		// get max size for MiG reply buffers
		bufSize = _config_subsystem.maxsize;

		// check if our on-the-stack reply buffer will be big enough
		if (bufSize > sizeof(bufReply_q)) {
			SCLog(TRUE, LOG_NOTICE,
			      CFSTR("configdCallback(): buffer size should be increased > %d"),
			      _config_subsystem.maxsize);
		}
	}

	if (bufSize > sizeof(bufReply_q)) {
		bufReply = CFAllocatorAllocate(NULL, _config_subsystem.maxsize, 0);
	}
	bufReply->RetCode = 0;

	/* we have a request message */
	(void) config_demux(&bufRequest->Head, &bufReply->Head);

	if (!(bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX)) {
		if (bufReply->RetCode == MIG_NO_REPLY) {
			bufReply->Head.msgh_remote_port = MACH_PORT_NULL;
		} else if ((bufReply->RetCode != KERN_SUCCESS) &&
			   (bufRequest->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX)) {
			/*
			 * destroy the request - but not the reply port
			 */
			bufRequest->Head.msgh_remote_port = MACH_PORT_NULL;
			mach_msg_destroy(&bufRequest->Head);
		}
	}

	if (bufReply->Head.msgh_remote_port != MACH_PORT_NULL) {
		/*
		 * send reply.
		 *
		 * We don't want to block indefinitely because the client
		 * isn't receiving messages from the reply port.
		 * If we have a send-once right for the reply port, then
		 * this isn't a concern because the send won't block.
		 * If we have a send right, we need to use MACH_SEND_TIMEOUT.
		 * To avoid falling off the kernel's fast RPC path unnecessarily,
		 * we only supply MACH_SEND_TIMEOUT when absolutely necessary.
		 */

		options = MACH_SEND_MSG;
		if (MACH_MSGH_BITS_REMOTE(bufReply->Head.msgh_bits) != MACH_MSG_TYPE_MOVE_SEND_ONCE) {
			options |= MACH_SEND_TIMEOUT;
		}
		mr = mach_msg(&bufReply->Head,		/* msg */
			      options,			/* option */
			      bufReply->Head.msgh_size,	/* send_size */
			      0,			/* rcv_size */
			      MACH_PORT_NULL,		/* rcv_name */
			      MACH_MSG_TIMEOUT_NONE,	/* timeout */
			      MACH_PORT_NULL);		/* notify */

		/* Has a message error occurred? */
		switch (mr) {
			case MACH_SEND_INVALID_DEST:
			case MACH_SEND_TIMED_OUT:
				break;
			default :
				/* Includes success case.  */
				goto done;
		}
	}

	if (bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) {
		mach_msg_destroy(&bufReply->Head);
	}

    done :

	if (bufReply != (mig_reply_error_t *)bufReply_q)
		CFAllocatorDeallocate(NULL, bufReply);
	return;
}


static CFStringRef
serverMPCopyDescription(const void *info)
{
	return CFStringCreateWithFormat(NULL, NULL, CFSTR("<main DynamicStore MP>"));
}


__private_extern__
void
server_init()
{
	serverSessionRef	mySession;
	CFRunLoopSourceRef	rls;
	char			*service_name;
	mach_port_t		service_port	= MACH_PORT_NULL;
	kern_return_t 		status;

	service_name = getenv("SCD_SERVER");
	if (!service_name) {
		service_name = SCD_SERVER;
	}

	/* Check "configd" server status */
	status = bootstrap_check_in(bootstrap_port, service_name, &service_port);
	switch (status) {
		case BOOTSTRAP_SUCCESS :
			/* if we are being [re-]started by launchd */
			break;
		case BOOTSTRAP_NOT_PRIVILEGED :
			/* if another instance of the server is starting */
			SCLog(TRUE, LOG_ERR, CFSTR("'%s' server already starting"), service_name);
			exit (EX_UNAVAILABLE);
		case BOOTSTRAP_SERVICE_ACTIVE :
			/* if another instance of the server is active */
			SCLog(TRUE, LOG_ERR, CFSTR("'%s' server already active"), service_name);
			exit (EX_UNAVAILABLE);
		default :
			SCLog(TRUE, LOG_ERR,
			      CFSTR("server_init bootstrap_check_in(..., '%s', ...) failed: %s"),
			      service_name,
			      bootstrap_strerror(status));
			exit (EX_UNAVAILABLE);
	}

	/* Create the primary / new connection port and backing session */
	mySession = addSession(service_port, serverMPCopyDescription);
	configd_port = mySession->serverPort;

	/*
	 * Create and add a run loop source for the port and add this source
	 * to the default run loop mode.
	 */
	rls = CFMachPortCreateRunLoopSource(NULL, configd_port, 0);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	return;
}


__private_extern__
int
server_shutdown()
{
	if (configd_port != NULL) {
		mach_port_t	service_port	= CFMachPortGetPort(configd_port);

		CFMachPortInvalidate(configd_port);
		CFRelease(configd_port);
		configd_port = NULL;

		if (service_port != MACH_PORT_NULL) {
			(void) mach_port_mod_refs(mach_task_self(),
						  service_port,
						  MACH_PORT_RIGHT_RECEIVE,
						  -1);
		}
	}

	return EX_OK;
}


__private_extern__
void
server_loop()
{
	pthread_setname_np("SCDynamicStore");

	while (TRUE) {
		/*
		 * process one run loop event
		 */
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0e10, TRUE);

		/*
		 * check for, and if necessary, push out change notifications
		 * to other processes.
		 */
		pushNotifications(_configd_trace);
	}
}

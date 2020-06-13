/*
 * Copyright (c) 2000-2011, 2013, 2015-2017, 2019 Apple Inc. All rights reserved.
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
#include "session.h"

/* MiG generated externals and functions */
extern struct mig_subsystem	_config_subsystem;


__private_extern__
void
server_mach_channel_handler(void			*context,	// serverSessionRef
			    dispatch_mach_reason_t	reason,
			    dispatch_mach_msg_t		message,
			    mach_error_t		error)
{
#pragma unused(error)
	bool			ok;
	serverSessionRef	session	= (serverSessionRef)context;

	static const struct mig_subsystem * const subsystems[] = {
		(mig_subsystem_t)&_config_subsystem,
	};

	switch (reason) {
		case DISPATCH_MACH_MESSAGE_RECEIVED:
			ok = dispatch_mach_mig_demux(context, subsystems, 1, message);
			if (ok) {
				/*
				 * the dispatch mach message has been consumed as per
				 * usual MIG rules.  Check for, and if necessary, push
				 * out [SCDynamicStore] change notifications to other
				 * processes.
				 */
				pushNotifications();
			} else {
				/*
				 * no subsystem claimed the message, destroy it
				 */
				mach_msg_destroy(dispatch_mach_msg_get_msg(message, NULL));
			}
			break;

		case DISPATCH_MACH_NO_SENDERS:
			__MACH_PORT_DEBUG(TRUE, "*** server_mach_channel_handler DISPATCH_MACH_NO_SENDERS", session->key);
			closeSession(session);
			break;

		case DISPATCH_MACH_CANCELED:
			__MACH_PORT_DEBUG(TRUE, "*** server_mach_channel_handler DISPATCH_MACH_CANCELED", session->key);
			cleanupSession(session);
			break;

		default:
			break;
	}

	return;
}


__private_extern__
dispatch_workloop_t
server_queue(void)
{
	static dispatch_once_t		once;
	static dispatch_workloop_t	workloop;

	dispatch_once(&once, ^{
		workloop = dispatch_workloop_create_inactive("configd/SCDynamicStore");
		dispatch_set_qos_class_fallback(workloop, QOS_CLASS_UTILITY);
		dispatch_activate(workloop);
	});

	return workloop;
}


__private_extern__
void
server_init()
{
	char			*service_name;
	mach_port_t		service_port	= MACH_PORT_NULL;
	kern_return_t 		status;

	service_name = getenv("SCD_SERVER");
	if (service_name == NULL) {
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
			SC_log(LOG_ERR, "'%s' server already starting", service_name);
			exit (EX_UNAVAILABLE);
		case BOOTSTRAP_SERVICE_ACTIVE :
			/* if another instance of the server is active */
			SC_log(LOG_ERR, "'%s' server already active", service_name);
			exit (EX_UNAVAILABLE);
		default :
			SC_log(LOG_ERR, "server_init bootstrap_check_in(..., '%s', ...) failed: %s",
			       service_name,
			       bootstrap_strerror(status));
			exit (EX_UNAVAILABLE);
	}

	/* Create the primary / new connection port and backing session */
	addServer(service_port);

	return;
}

/*
 * Copyright (c) 2000, 2001, 2003-2005, 2008-2011, 2013 Apple Inc. All rights reserved.
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
 * April 5, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <mach/mach.h>
#include <mach/mach_error.h>

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include "SCDynamicStoreInternal.h"
#include "config.h"		/* MiG generated file */

#include <paths.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

Boolean
SCDynamicStoreNotifyFileDescriptor(SCDynamicStoreRef	store,
				   int32_t		identifier,
				   int			*fd)
{
	size_t				n;
	int				sc_status;
	int				sock;
	SCDynamicStorePrivateRef	storePrivate = (SCDynamicStorePrivateRef)store;
	kern_return_t			status;
	char				tmpdir[PATH_MAX];
	struct sockaddr_un		un;

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

	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		_SCErrorSet(errno);
		SCLog(TRUE, LOG_NOTICE, CFSTR("SCDynamicStoreNotifyFileDescriptor socket(): %s"), strerror(errno));
		return FALSE;
	}

	/* establish a UNIX domain socket for server->client notification */

	n = confstr(_CS_DARWIN_USER_TEMP_DIR, tmpdir, sizeof(tmpdir));
	if ((n <= 0) || (n >= sizeof(tmpdir))) {
		(void) strlcpy(tmpdir, _PATH_TMP, sizeof(tmpdir));
	}

	bzero(&un, sizeof(un));
	un.sun_family = AF_UNIX;
	snprintf(un.sun_path,
		 sizeof(un.sun_path)-1,
		 "%s%s-%d-%d",
		 tmpdir,
		 "SCDynamicStoreNotifyFileDescriptor",
		 getpid(),
		 storePrivate->server);

	/* ensure that the path does not already exist */
	(void) unlink(un.sun_path);

	if (bind(sock, (struct sockaddr *)&un, sizeof(un)) == -1) {
		_SCErrorSet(errno);
		SCLog(TRUE, LOG_NOTICE, CFSTR("SCDynamicStoreNotifyFileDescriptor bind(): %s"), strerror(errno));
		(void) unlink(un.sun_path);
		(void) close(sock);
		return FALSE;
	}

	if (listen(sock, 0) == -1) {
		_SCErrorSet(errno);
		SCLog(TRUE, LOG_NOTICE, CFSTR("SCDynamicStoreNotifyFileDescriptor listen(): %s"), strerror(errno));
		(void) unlink(un.sun_path);
		(void) close(sock);
		return FALSE;
	}

    retry :

	status = notifyviafd(storePrivate->server,
			     un.sun_path,
			     (mach_msg_type_number_t)strlen(un.sun_path),
			     identifier,
			     (int *)&sc_status);

	if (__SCDynamicStoreCheckRetryAndHandleError(store,
						     status,
						     &sc_status,
						     "SCDynamicStoreNotifyFileDescriptor notifyviafd()")) {
		goto retry;
	}

	if (status != KERN_SUCCESS) {
		_SCErrorSet(status);
		return FALSE;
	}

	(void) unlink(un.sun_path);

	if (sc_status != kSCStatusOK) {
		_SCErrorSet(sc_status);
		SCLog(TRUE, LOG_NOTICE,
		      CFSTR("SCDynamicStoreNotifyFileDescriptor server error: %s"),
		      SCErrorString(sc_status));
		(void) close(sock);
		return FALSE;
	}

	*fd = accept(sock, 0, 0);
	if (*fd == -1) {
		_SCErrorSet(errno);
		SCLog(TRUE, LOG_NOTICE, CFSTR("SCDynamicStoreNotifyFileDescriptor accept(): %s"), strerror(errno));
		(void) close(sock);
		return FALSE;
	}
	(void) close(sock);

	/* set notifier active */
	storePrivate->notifyStatus = Using_NotifierInformViaFD;

	return TRUE;
}

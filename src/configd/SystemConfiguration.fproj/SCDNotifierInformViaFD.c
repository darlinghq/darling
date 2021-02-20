/*
 * Copyright (c) 2000, 2001, 2003-2005, 2008-2011, 2013, 2015-2017, 2019 Apple Inc. All rights reserved.
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
	int					fildes[2]	= { -1, -1 };
	fileport_t				fileport	= MACH_PORT_NULL;
	int					ret;
	int					sc_status;
	SCDynamicStorePrivateRef		storePrivate = (SCDynamicStorePrivateRef)store;
	kern_return_t				status;

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

	ret = pipe(fildes);
	if (ret == -1) {
		_SCErrorSet(errno);
		SC_log(LOG_ERR, "pipe() failed: %s", strerror(errno));
		goto fail;
	}

	/*
	 * send fildes[1], the sender's fd, to configd using a fileport and
	 * return fildes[0] to the caller.
	 */

	fileport = MACH_PORT_NULL;
	ret = fileport_makeport(fildes[1], &fileport);
	if (ret < 0) {
		_SCErrorSet(errno);
		SC_log(LOG_ERR, "fileport_makeport() failed: %s", strerror(errno));
		goto fail;
	}

    retry :

	status = notifyviafd(storePrivate->server,
			     fileport,
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
		goto fail;
	}

	if (sc_status != kSCStatusOK) {
		_SCErrorSet(sc_status);
		goto fail;
	}

	/* the SCDynamicStore server now has a copy of the write side, close our reference */
	(void) close(fildes[1]);

	/* and keep the read side */
	*fd = fildes[0];

	/* set notifier active */
	storePrivate->notifyStatus = Using_NotifierInformViaFD;

	return TRUE;

    fail :

	if (fildes[0] != -1) close(fildes[0]);
	if (fildes[1] != -1) close(fildes[1]);
	return FALSE;
}

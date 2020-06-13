/*
 * Copyright (c) 2000, 2001, 2003-2006, 2008-2011, 2015, 2019 Apple Inc. All rights reserved.
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

#include <unistd.h>
#include <sys/fileport.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "configd.h"
#include "session.h"


__private_extern__
int
__SCDynamicStoreNotifyFileDescriptor(SCDynamicStoreRef	store)
{
	serverSessionRef		mySession;
	SCDynamicStorePrivateRef	storePrivate = (SCDynamicStorePrivateRef)store;

	if (storePrivate->notifyStatus != NotifierNotRegistered) {
		/* sorry, you can only have one notification registered at once */
		return kSCStatusNotifierActive;
	}

	/* push out a notification if any changes are pending */
	mySession = getSession(storePrivate->server);
	if (mySession->changedKeys != NULL) {
		CFNumberRef	sessionNum;

		if (needsNotification == NULL)
			needsNotification = CFSetCreateMutable(NULL,
							       0,
							       &kCFTypeSetCallBacks);

		sessionNum = CFNumberCreate(NULL, kCFNumberIntType, &storePrivate->server);
		CFSetAddValue(needsNotification, sessionNum);
		CFRelease(sessionNum);
	}

	return kSCStatusOK;
}


__private_extern__
kern_return_t
_notifyviafd(mach_port_t	server,
	     fileport_t		fileport,
	     int		identifier,
	     int		*sc_status
)
{
	int				fd		= -1;
	int				flags;
	serverSessionRef		mySession       = getSession(server);
	SCDynamicStorePrivateRef	storePrivate;

	/* get notification file descriptor */
	fd = fileport_makefd(fileport);
	mach_port_deallocate(mach_task_self(), fileport);
	if (fd < 0) {
		*sc_status = errno;
		return KERN_SUCCESS;
	}

	flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		*sc_status = errno;
		goto fail;
	}

	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) == -1) {
		*sc_status = errno;
		goto fail;
	}

	if (mySession == NULL) {
		*sc_status = kSCStatusNoStoreSession;	/* you must have an open session to play */
		goto fail;
	}
	storePrivate = (SCDynamicStorePrivateRef)mySession->store;

	/* do common sanity checks */
	*sc_status = __SCDynamicStoreNotifyFileDescriptor(mySession->store);

	/* check status of __SCDynamicStoreNotifyFileDescriptor() */
	if (*sc_status != kSCStatusOK) {
		goto fail;
	}

	/* set notifier active */
	storePrivate->notifyStatus         = Using_NotifierInformViaFD;
	storePrivate->notifyFile           = fd;
	storePrivate->notifyFileIdentifier = identifier;

	return KERN_SUCCESS;

    fail :

	if (fd >= 0) close(fd);
	return KERN_SUCCESS;
}

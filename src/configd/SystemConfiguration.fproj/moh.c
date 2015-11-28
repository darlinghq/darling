/*
 * Copyright (c) 2002, 2003, 2005, 2013, 2014 Apple Inc. All rights reserved.
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
 * May 29, 2002		Roger Smith <rsmith@apple.com>
 * - initial revision
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>

#include "moh.h"
#include "moh_msg.h"

// Note: right now we are not currently using the deviceName.  This could be the raw
//       tty name such as "modem" since this is guaranteed to be unique in the /dev.
//       We would use this deviceName to differientate between multiple MOH devices
//       present in the system when we create the socket.


static ssize_t
readn(int ref, void *data, size_t len)
{
	size_t	left	= len;
	ssize_t	n;
	void	*p	= data;

	while (left > 0) {
		if ((n = read(ref, p, left)) == -1) {
			if (errno != EINTR) {
				return -1;
			}
			n = 0;
		} else if (n == 0) {
			break; /* EOF */
		}

		left -= n;
		p += n;
	}
	return (len - left);
}


static ssize_t
writen(int ref, const void *data, size_t len)
{
	size_t		left	= len;
	ssize_t		n;
	const void	*p	= data;

	while (left > 0) {
		if ((n = write(ref, p, left)) == -1) {
			if (errno != EINTR) {
				return -1;
			}
			n = 0;
		}
		left -= n;
		p += n;
	}
	return len;
}


__private_extern__
int
MOHInit(int *ref, CFStringRef deviceName)
{
	int			sock;
	int			status;
	struct sockaddr_un	sun;

	sock = socket(AF_LOCAL, SOCK_STREAM, 0);

	bzero(&sun, sizeof(sun));
	sun.sun_family = AF_LOCAL;
	strncpy(sun.sun_path, MOH_PATH, sizeof(sun.sun_path));

	status = connect(sock, (struct sockaddr *)&sun, sizeof(sun));
	if (status == -1) {
		return errno;
	}

	*ref = sock;
	return 0;
}


__private_extern__
int
MOHDispose(int ref)
{
	if (close(ref) == -1) {
		return errno;
	}
	return 0;
}


__private_extern__
int
MOHExec(int		ref,
	uint32_t	link,
	uint32_t	cmd,
	void		*request,
	size_t		requestLen,
	void		**reply,
	size_t		*replyLen)
{
	struct moh_msg_hdr	msg;
	char			*buf		= NULL;
	ssize_t			n;

	bzero(&msg, sizeof(msg));
	msg.m_type = cmd;
	msg.m_link = link;
	msg.m_len  = ((request != NULL) && (requestLen > 0)) ? (uint32_t)requestLen : 0;

	//  send the command
	n = writen(ref, &msg, sizeof(msg));
	if (n == -1) {
		SCLog(_sc_verbose, LOG_ERR, CFSTR("MOHExec writen() failed: %s"), strerror(errno));
		return errno;
	} else if (n != sizeof(msg)) {
		SCLog(_sc_verbose, LOG_ERR, CFSTR("MOHExec writen() failed: wrote=%ld"), n);
		return -1;
	}

	if ((request != NULL) && (requestLen > 0)) {
		n = writen(ref, request, requestLen);
		if (n == -1) {
			SCLog(_sc_verbose, LOG_ERR, CFSTR("MOHExec writen() failed: %s"), strerror(errno));
			return errno;
		} else if (n != (ssize_t)requestLen) {
			SCLog(_sc_verbose, LOG_ERR, CFSTR("MOHExec writen() failed: wrote=%ld"), n);
			return -1;
		}
	}

	// always expect a reply
	n = readn(ref, &msg, sizeof(msg));
	if (n == -1) {
		SCLog(_sc_verbose, LOG_ERR, CFSTR("MOHExec readn() failed: error=%s"), strerror(errno));
		return errno;
	} else if (n != sizeof(msg)) {
		SCLog(_sc_verbose, LOG_ERR, CFSTR("MOHExec readn() failed: insufficent data, read=%ld"), n);
		return -1;
	}

	if (msg.m_len) {
		buf = CFAllocatorAllocate(NULL, msg.m_len, 0);
		if (buf) {
			// read reply
			n = readn(ref, buf, msg.m_len);
			if (n == -1) {
				SCLog(_sc_verbose, LOG_ERR, CFSTR("MOHExec readn() failed: error=%s"), strerror(errno));
				CFAllocatorDeallocate(NULL, buf);
				return errno;
			} else if (n != (ssize_t)msg.m_len) {
				SCLog(_sc_verbose, LOG_ERR, CFSTR("MOHExec readn() failed: insufficent data, read=%ld"), n);
				CFAllocatorDeallocate(NULL, buf);
				return -1;
			}
		}
	}

	if (reply && replyLen) {
		*reply    = buf;
		*replyLen = msg.m_len;
	} else if (buf) {
		// if additional returned data is unwanted
		CFAllocatorDeallocate(NULL, buf);
	}

	return msg.m_result;
}


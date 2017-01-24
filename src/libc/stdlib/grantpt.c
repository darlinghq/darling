/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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

#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

/*
 * posix_openpt call for cloning pty implementation.
 *
 * Just open /dev/ptmx
 */
int
posix_openpt(int flags)
{
	int fd = open("/dev/ptmx", flags);
	if (fd >= 0)
		return fd;
	return -1;
}

/*
 * grantpt call for cloning pty implementation.
 *
 * Change UID and GID of slave pty associated with master pty whose
 * fd is provided, to the real UID and real GID of the calling thread.
 */
int
grantpt(int fd)
{
	return ioctl(fd, TIOCPTYGRANT);
}

/*
 * ptsname call for cloning pty implementation.
 *
 * NOTE: Returns a pointer to a static buffer, which will be overwritten on
 * subsequent calls.
 */
char *
ptsname(int fd)
{
	static char *ptsnamebuf = NULL;
	int error;
	char *retval = NULL;
	struct stat sbuf;

	if (ptsnamebuf == NULL) {
		ptsnamebuf = malloc(128); // defined by TIOCPTYGNAME
	}
	
	error = ioctl(fd, TIOCPTYGNAME, ptsnamebuf);
	if (!error) {
		/*
		 * XXX TSD
		 *
		 * POSIX: Handle device rename test case, which is expected
		 * to fail if the pty has been renamed.
		 */
		if (stat(ptsnamebuf, &sbuf) == 0) {
			retval = ptsnamebuf;
		}
	}

	return (retval);
}

/*
 * unlockpt call for cloning pty implementation.
 *
 * Unlock the slave pty associated with the master to which fd refers.
 */
int
unlockpt(int fd)
{
	return ioctl(fd, TIOCPTYUNLK);
}

/*-
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)termios.c	8.2 (Berkeley) 2/21/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/termios.c,v 1.16 2009/05/07 13:49:48 ed Exp $");

#if __DARWIN_UNIX03
#ifdef VARIANT_CANCELABLE
#include <pthread.h>
#endif /* VARIANT_CANCELABLE */
#endif /* __DARWIN_UNIX03 */

#include "namespace.h"
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include "un-namespace.h"

#ifndef BUILDING_VARIANT
int
tcgetattr(fd, t)
	int fd;
	struct termios *t;
{

	return (_ioctl(fd, TIOCGETA, t));
}

int
tcsetattr(fd, opt, t)
	int fd, opt;
	const struct termios *t;
{
	struct termios localterm;

	if (opt & TCSASOFT) {
		localterm = *t;
		localterm.c_cflag |= CIGNORE;
		t = &localterm;
	}
	switch (opt & ~TCSASOFT) {
	case TCSANOW:
		return (_ioctl(fd, TIOCSETA, t));
	case TCSADRAIN:
		return (_ioctl(fd, TIOCSETAW, t));
	case TCSAFLUSH:
		return (_ioctl(fd, TIOCSETAF, t));
	default:
		errno = EINVAL;
		return (-1);
	}
}

int
tcsetpgrp(int fd, pid_t pgrp)
{
	int s;

	if (isatty(fd) == 0)
		return (-1);

	s = pgrp;
	return (_ioctl(fd, TIOCSPGRP, &s));
}

pid_t
tcgetpgrp(fd)
	int fd;
{
	int s;

	if (isatty(fd) == 0)
		return ((pid_t)-1);

	if (_ioctl(fd, TIOCGPGRP, &s) < 0)
		return ((pid_t)-1);

	return ((pid_t)s);
}

#if 0 // Needs API review first
pid_t
tcgetsid(int fd)
{
	int s;

	if (_ioctl(fd, TIOCGSID, &s) < 0)
		return ((pid_t)-1);

	return ((pid_t)s);
}

int
tcsetsid(int fd, pid_t pid)
{

	if (pid != getsid(0)) {
		errno = EINVAL;
		return (-1);
	}

	return (_ioctl(fd, TIOCSCTTY, NULL));
}
#endif

speed_t
cfgetospeed(t)
	const struct termios *t;
{

	return (t->c_ospeed);
}

speed_t
cfgetispeed(t)
	const struct termios *t;
{

	return (t->c_ispeed);
}

int
cfsetospeed(t, speed)
	struct termios *t;
	speed_t speed;
{

	t->c_ospeed = speed;
	return (0);
}

int
cfsetispeed(t, speed)
	struct termios *t;
	speed_t speed;
{

	t->c_ispeed = speed;
	return (0);
}

int
cfsetspeed(t, speed)
	struct termios *t;
	speed_t speed;
{

	t->c_ispeed = t->c_ospeed = speed;
	return (0);
}

/*
 * Make a pre-existing termios structure into "raw" mode: character-at-a-time
 * mode with no characters interpreted, 8-bit data path.
 */
void
cfmakeraw(t)
	struct termios *t;
{

	t->c_iflag &= ~(IMAXBEL|IXOFF|INPCK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IGNPAR);
	t->c_iflag |= IGNBRK;
	t->c_oflag &= ~OPOST;
	t->c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL|ICANON|ISIG|IEXTEN|NOFLSH|TOSTOP|PENDIN);
	t->c_cflag &= ~(CSIZE|PARENB);
	t->c_cflag |= CS8|CREAD;
	t->c_cc[VMIN] = 1;
	t->c_cc[VTIME] = 0;
}

int
tcsendbreak(fd, len)
	int fd, len;
{
	struct timeval sleepytime;

	sleepytime.tv_sec = 0;
	sleepytime.tv_usec = 400000;
	if (_ioctl(fd, TIOCSBRK, 0) == -1)
		return (-1);
	(void)_select(0, 0, 0, 0, &sleepytime);
	if (_ioctl(fd, TIOCCBRK, 0) == -1)
		return (-1);
	return (0);
}
#endif /* BUILDING_VARIANT */

int
__tcdrain(fd)
	int fd;
{
#if __DARWIN_UNIX03
#ifdef VARIANT_CANCELABLE
	pthread_testcancel();
#endif /* VARIANT_CANCELABLE */
#endif /* __DARWIN_UNIX03 */
	return (_ioctl(fd, TIOCDRAIN, 0));
}

__weak_reference(__tcdrain, tcdrain);
__weak_reference(__tcdrain, _tcdrain);

#ifndef BUILDING_VARIANT
int
tcflush(fd, which)
	int fd, which;
{
	int com;

	switch (which) {
	case TCIFLUSH:
		com = FREAD;
		break;
	case TCOFLUSH:
		com = FWRITE;
		break;
	case TCIOFLUSH:
		com = FREAD | FWRITE;
		break;
	default:
		errno = EINVAL;
		return (-1);
	}
	return (_ioctl(fd, TIOCFLUSH, &com));
}

int
tcflow(fd, action)
	int fd, action;
{
	switch (action) {
	case TCOOFF:
		return (_ioctl(fd, TIOCSTOP, 0));
	case TCOON:
		return (_ioctl(fd, TIOCSTART, 0));
	case TCION:
		return (_ioctl(fd, TIOCIXON, 0));
	case TCIOFF:
		return (_ioctl(fd, TIOCIXOFF, 0));
	default:
		errno = EINVAL;
		return (-1);
	}
	/* NOTREACHED */
}
#endif /* BUILDING_VARIANT */

/*-
 * Copyright (c) 1993
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
static char sccsid[] = "@(#)sysctl.c	8.2 (Berkeley) 1/4/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/sysctl.c,v 1.6 2007/01/09 00:27:55 imp Exp $");

#include <sys/param.h>
#include <sys/sysctl.h>

#include <errno.h>
#include <limits.h>
#include <paths.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

extern int __sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp,
    void *newp, size_t newlen);

int
sysctl(name, namelen, oldp, oldlenp, newp, newlen)
	int *name;
	u_int namelen;
	void *oldp, *newp;
	size_t *oldlenp, newlen;
{
	if (name[0] != CTL_USER) {
		if (namelen == 2 && name[0] == CTL_KERN && name[1] == KERN_EXEC) {
			/*
			 * 7723306: intercept kern.exec and fake a return of
			 * a dummy string ("/" in this case)
			 */
			if (newp != NULL) {
				errno = EPERM;
				return -1;
			}
			if (oldp == NULL) {
				if (oldlenp != NULL) *oldlenp = 2;
				return 0;
			}
			if (oldlenp == NULL) {
				errno = EFAULT;
				return -1;
			}
			if (*oldlenp < 2) {
				errno = ENOMEM;
				return -1;
			}
			memmove(oldp, "/", 2);
			*oldlenp = 2;
			return 0;
		}
		return (__sysctl(name, namelen, oldp, oldlenp, newp, newlen));
	}

	if (newp != NULL) {
		errno = EPERM;
		return (-1);
	}
	if (namelen != 2) {
		errno = EINVAL;
		return (-1);
	}

	switch (name[1]) {
	case USER_CS_PATH:
		if (oldp && *oldlenp < sizeof(_PATH_STDPATH)) {
			errno = ENOMEM;
			return -1;
		}
		*oldlenp = sizeof(_PATH_STDPATH);
		if (oldp != NULL)
			memmove(oldp, _PATH_STDPATH, sizeof(_PATH_STDPATH));
		return (0);
	}

	if (oldp && *oldlenp < sizeof(int)) {
		errno = ENOMEM;
		return (-1);
	}
	*oldlenp = sizeof(int);
	if (oldp == NULL)
		return (0);

	switch (name[1]) {
	case USER_BC_BASE_MAX:
		*(int *)oldp = BC_BASE_MAX;
		return (0);
	case USER_BC_DIM_MAX:
		*(int *)oldp = BC_DIM_MAX;
		return (0);
	case USER_BC_SCALE_MAX:
		*(int *)oldp = BC_SCALE_MAX;
		return (0);
	case USER_BC_STRING_MAX:
		*(int *)oldp = BC_STRING_MAX;
		return (0);
	case USER_COLL_WEIGHTS_MAX:
		*(int *)oldp = COLL_WEIGHTS_MAX;
		return (0);
	case USER_EXPR_NEST_MAX:
		*(int *)oldp = EXPR_NEST_MAX;
		return (0);
	case USER_LINE_MAX:
		*(int *)oldp = LINE_MAX;
		return (0);
	case USER_RE_DUP_MAX:
		*(int *)oldp = RE_DUP_MAX;
		return (0);
	case USER_POSIX2_VERSION:
		*(int *)oldp = _POSIX2_VERSION;
		return (0);
	case USER_POSIX2_C_BIND:
#ifdef POSIX2_C_BIND
		*(int *)oldp = 1;
#else
		*(int *)oldp = 0;
#endif
		return (0);
	case USER_POSIX2_C_DEV:
#ifdef	POSIX2_C_DEV
		*(int *)oldp = 1;
#else
		*(int *)oldp = 0;
#endif
		return (0);
	case USER_POSIX2_CHAR_TERM:
#ifdef	POSIX2_CHAR_TERM
		*(int *)oldp = 1;
#else
		*(int *)oldp = 0;
#endif
		return (0);
	case USER_POSIX2_FORT_DEV:
#ifdef	POSIX2_FORT_DEV
		*(int *)oldp = 1;
#else
		*(int *)oldp = 0;
#endif
		return (0);
	case USER_POSIX2_FORT_RUN:
#ifdef	POSIX2_FORT_RUN
		*(int *)oldp = 1;
#else
		*(int *)oldp = 0;
#endif
		return (0);
	case USER_POSIX2_LOCALEDEF:
#ifdef	POSIX2_LOCALEDEF
		*(int *)oldp = 1;
#else
		*(int *)oldp = 0;
#endif
		return (0);
	case USER_POSIX2_SW_DEV:
#ifdef	POSIX2_SW_DEV
		*(int *)oldp = 1;
#else
		*(int *)oldp = 0;
#endif
		return (0);
	case USER_POSIX2_UPE:
#ifdef	POSIX2_UPE
		*(int *)oldp = 1;
#else
		*(int *)oldp = 0;
#endif
		return (0);
	case USER_STREAM_MAX:
		*(int *)oldp = FOPEN_MAX;
		return (0);
	case USER_TZNAME_MAX:
		*(int *)oldp = NAME_MAX;
		return (0);
	default:
		errno = EINVAL;
		return (-1);
	}
	/* NOTREACHED */
}

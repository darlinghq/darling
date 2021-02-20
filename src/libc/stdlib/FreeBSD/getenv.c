/*
 * Copyright (c) 1987, 1993
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
static char sccsid[] = "@(#)getenv.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdlib/getenv.c,v 1.8 2007/05/01 16:02:41 ache Exp $");

#include <os/lock_private.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <crt_externs.h>

__private_extern__ char *__findenv_locked(const char *, int *, char **);
__private_extern__ void __environ_lock(void);
__private_extern__ void __environ_unlock(void);

/*
 * __findenv_locked --
 *	Returns pointer to value associated with name, if any, else NULL.
 *	Sets offset to be the offset of the name/value combination in the
 *	environmental array, for use by setenv(3) and unsetenv(3).
 *	Explicitly removes '=' in argument name.
 *
 *	This routine *should* be a static; don't use it.
 */
__private_extern__ char *
__findenv_locked(name, offset, environ)
	const char *name;
	int *offset;
	char **environ;
{
	int len, i;
	const char *np;
	char **p, *cp;

	if (name == NULL || environ == NULL)
		return (NULL);
	for (np = name; *np && *np != '='; ++np)
		continue;
	len = np - name;
	for (p = environ; (cp = *p) != NULL; ++p) {
		for (np = name, i = len; i && *cp; i--)
			if (*cp++ != *np++)
				break;
		if (i == 0 && *cp++ == '=') {
			*offset = p - environ;
			return (cp);
		}
	}
	return (NULL);
}

static os_unfair_lock __environ_lock_obj = OS_UNFAIR_LOCK_INIT;
__private_extern__ void
__environ_lock(void)
{
	os_unfair_lock_lock_with_options(
			&__environ_lock_obj, OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
}
__private_extern__ void
__environ_unlock(void)
{
	os_unfair_lock_unlock(&__environ_lock_obj);
}
__private_extern__ void
__environ_lock_fork_child(void)
{
	__environ_lock_obj = OS_UNFAIR_LOCK_INIT;
}

/*
 * _getenvp -- SPI using an arbitrary pointer to string array (the array must
 * have been created with malloc) and an env state, created by _allocenvstate().
 *	Returns ptr to value associated with name, if any, else NULL.
 */
char *
_getenvp(const char *name, char ***envp, void *state __unused)
{
	// envp is passed as an argument, so the lock is not protecting everything
	int offset;
	__environ_lock();
	char *result = (__findenv_locked(name, &offset, *envp));
	__environ_unlock();
	return result;
}

/*
 * getenv --
 *	Returns ptr to value associated with name, if any, else NULL.
 */
char *
getenv(name)
	const char *name;
{
	int offset;
	__environ_lock();
	char *result = __findenv_locked(name, &offset, *_NSGetEnviron());
	__environ_unlock();
	return result;
}

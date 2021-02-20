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
 * 3. Neither the name of the University nor the names of its contributors
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
static char sccsid[] = "@(#)mktemp.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>

#include "namespace.h"
#include <assert.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "un-namespace.h"

#define ALLOWED_MKOSTEMP_FLAGS (O_APPEND | O_SHLOCK | O_EXLOCK | O_CLOEXEC)

char *_mktemp(char *);

typedef enum {
	FTPP_DONE, FTPP_TRY_NEXT, FTPP_ERROR
} find_temp_path_progress_t;

/* A contract for actions that find_temp_path performs for every path from
 * the template.
 *
 * If the desired path was found, set result and return FTPP_DONE.
 * If an IO/FS error ocurred, set errno and return FTPP_ERROR.
 * Otherwise return FTPP_TRY_NEXT.
 */
typedef find_temp_path_progress_t (*find_temp_path_action_t)(
		int dfd, char *path, void *ctx, void *result);

static int find_temp_path(int dfd, char *path, int slen, bool stat_base_dir,
		find_temp_path_action_t action, void *action_ctx, void *action_result);

static find_temp_path_progress_t _mkostemps_action(
		int dfd, char *path, void *ctx, void *result);
static find_temp_path_progress_t _mktemp_action(
		int dfd, char *path, void *ctx, void *result);
static find_temp_path_progress_t _mkdtemp_action(
		int dfd, char *path, void *ctx, void *result);
static find_temp_path_progress_t _mkstemp_dprotected_np_action(
		int dfd, char *path, void *ctx, void *result);

static const char padchar[] =
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

int
mkostemps(char *path, int slen, int oflags)
{
	int fd;
	if (oflags & ~ALLOWED_MKOSTEMP_FLAGS) {
		errno = EINVAL;
		return -1;
	}
	return (find_temp_path(AT_FDCWD, path, slen, TRUE, _mkostemps_action, &oflags, &fd) ? fd : -1);
}

int
mkostempsat_np(int dfd, char *path, int slen, int oflags)
{
	int fd;
	if (oflags & ~ALLOWED_MKOSTEMP_FLAGS) {
		errno = EINVAL;
		return -1;
	}
	return (find_temp_path(dfd, path, slen, TRUE, _mkostemps_action, &oflags, &fd) ? fd : -1);
}

int
mkstemps(char *path, int slen)
{
	int fd;

	return (find_temp_path(AT_FDCWD, path, slen, TRUE, _mkostemps_action, NULL, &fd) ? fd : -1);
}

int
mkstempsat_np(int dfd, char *path, int slen)
{
	int fd;

	return (find_temp_path(dfd, path, slen, TRUE, _mkostemps_action, NULL, &fd) ? fd : -1);
}

int
mkostemp(char *path, int oflags)
{
	int fd;
	if (oflags & ~ALLOWED_MKOSTEMP_FLAGS) {
		errno = EINVAL;
		return -1;
	}
	return (find_temp_path(AT_FDCWD, path, 0, TRUE, _mkostemps_action, &oflags, &fd) ? fd : -1);
}

int
mkstemp(char *path)
{
	int fd;

	return (find_temp_path(AT_FDCWD, path, 0, TRUE, _mkostemps_action, NULL, &fd) ? fd : -1);
}

char *
mkdtemp(char *path)
{
	return (find_temp_path(AT_FDCWD, path, 0, TRUE, _mkdtemp_action, NULL, NULL) ?
			path : (char *)NULL);
}

char *
mkdtempat_np(int dfd, char *path)
{
	return (find_temp_path(dfd, path, 0, TRUE, _mkdtemp_action, NULL, NULL) ?
			path : (char *)NULL);
}

char *
_mktemp(char *path)
{
	return (find_temp_path(AT_FDCWD, path, 0, FALSE, _mktemp_action, NULL, NULL) ?
			path : (char *)NULL);
}

__warn_references(mktemp,
    "warning: mktemp() possibly used unsafely; consider using mkstemp()");

char *
mktemp(char *path)
{
	return (_mktemp(path));
}

int
mkstemp_dprotected_np(char *path, int class, int dpflags)
{
	int fd;
	int ctx[2] = { class, dpflags };

	return (find_temp_path(AT_FDCWD, path, 0, TRUE, _mkstemp_dprotected_np_action, &ctx, &fd) ? fd : -1);
}

/* For every path matching a given template, invoke an action. Depending on
 * the progress reported by action, stops or tries the next path.
 * Returns 1 if succeeds, 0 and sets errno if fails.
 */
static int
find_temp_path(int dfd, char *path, int slen, bool stat_base_dir,
		find_temp_path_action_t action, void *action_ctx, void *action_result)
{
	char *start, *trv, *suffp, *carryp;
	char *pad;
	struct stat sbuf;
	int rval;
	uint32_t rand;
	char carrybuf[MAXPATHLEN];

	if (slen < 0) {
		errno = EINVAL;
		return (0);
	}

	for (trv = path; *trv != '\0'; ++trv)
		;
	if (trv - path >= MAXPATHLEN) {
		errno = ENAMETOOLONG;
		return (0);
	}
	trv -= slen;
	suffp = trv;
	--trv;
	if (trv < path || NULL != strchr(suffp, '/')) {
		errno = EINVAL;
		return (0);
	}

	/* Fill space with random characters */
	while (trv >= path && *trv == 'X') {
		rand = arc4random_uniform(sizeof(padchar) - 1);
		*trv-- = padchar[rand];
	}
	start = trv + 1;

	/* save first combination of random characters */
	memcpy(carrybuf, start, suffp - start);

	/*
	 * check the target directory.
	 */
	if (stat_base_dir) {
		for (; trv > path; --trv) {
			if (*trv == '/') {
				*trv = '\0';
				rval = fstatat(dfd, path, &sbuf, 0);
				*trv = '/';
				if (rval != 0)
					return (0);
				if (!S_ISDIR(sbuf.st_mode)) {
					errno = ENOTDIR;
					return (0);
				}
				break;
			}
		}
	}

	for (;;) {
		switch (action(dfd, path, action_ctx, action_result)) {
		case FTPP_DONE:
			return (1);
		case FTPP_ERROR:
			return (0); // errno must be set by the action
		default:
			; // FTPP_TRY_NEXT, fall-through
		}

		/* If we have a collision, cycle through the space of filenames */
		for (trv = start, carryp = carrybuf;;) {
			/* have we tried all possible permutations? */
			if (trv == suffp) {
				/* yes - exit with EEXIST */
				errno = EEXIST;
				return (0);
			}
			pad = strchr(padchar, *trv);
			if (pad == NULL) {
				/* this should never happen */
				errno = EIO;
				return (0);
			}
			/* increment character */
			*trv = (*++pad == '\0') ? padchar[0] : *pad;
			/* carry to next position? */
			if (*trv == *carryp) {
				/* increment position and loop */
				++trv;
				++carryp;
			} else {
				/* try with new name */
				break;
			}
		}
	}
	/*NOTREACHED*/
}

static find_temp_path_progress_t
_mkostemps_action(int dfd, char *path, void *ctx, void *result)
{
	int oflags = (ctx != NULL) ? *((int *) ctx) : 0;
	int fd = openat(dfd, path, O_CREAT|O_EXCL|O_RDWR|oflags, 0600);
	if (fd >= 0) {
		*((int *) result) = fd;
		return FTPP_DONE;
	}
	return (errno == EEXIST) ?
			FTPP_TRY_NEXT :
			FTPP_ERROR; // errno is set already
}

static find_temp_path_progress_t
_mktemp_action(int dfd, char *path, void *ctx __unused, void *result __unused)
{
	struct stat sbuf;
	if (fstatat(dfd, path, &sbuf, AT_SYMLINK_NOFOLLOW)) {
		// stat failed
		return (errno == ENOENT) ?
				FTPP_DONE : // path is vacant, done
				FTPP_ERROR; // errno is set already
	}
	return FTPP_TRY_NEXT;
}

static find_temp_path_progress_t
_mkdtemp_action(int dfd, char *path, void *ctx __unused, void *result __unused)
{
	if (mkdirat(dfd, path, 0700) == 0)
		return FTPP_DONE;
	return (errno == EEXIST) ?
			FTPP_TRY_NEXT :
			FTPP_ERROR; // errno is set already
}

static find_temp_path_progress_t
_mkstemp_dprotected_np_action(int dfd, char *path, void *ctx, void *result)
{
	assert(dfd == AT_FDCWD);

	int class = ((int *) ctx)[0];
	int dpflags = ((int *) ctx)[1];
	int fd = open_dprotected_np(path, O_CREAT|O_EXCL|O_RDWR, class, dpflags, 0600);
	if (fd >= 0) {
		*((int *) result) = fd;
		return FTPP_DONE;
	}
	return (errno == EEXIST) ?
			FTPP_TRY_NEXT :
			FTPP_ERROR; // errno is set already
}



/*
 * Copyright (c) 2000, Todd C. Miller.  All rights reserved.
 * Copyright (c) 1996, Jason Downs.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <paths.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "util.h"

/*
 * opendev(3) is an inherently non-thread-safe API, since
 * it returns a buffer to global storage. However we can
 * at least make sure the storage allocation is thread safe
 * and does not leak memory in case of simultaneous
 * initialization
 */
static pthread_once_t opendev_namebuf_once = PTHREAD_ONCE_INIT;
static char *namebuf = NULL;

static void opendev_namebuf_init(void);

int
opendev(path, oflags, dflags, realpath)
	char *path;
	int oflags;
	int dflags;
	char **realpath;
{
	int fd;
	char *slash, *prefix;

	/* Initial state */
	if (realpath)
		*realpath = path;
	fd = -1;
	errno = ENOENT;

	if (pthread_once(&opendev_namebuf_once,
					 opendev_namebuf_init)
		|| !namebuf) {
		errno = ENOMEM;
		return -1;
	}

	if (dflags & OPENDEV_BLCK)
		prefix = "";			/* block device */
	else
		prefix = "r";			/* character device */

	if ((slash = strchr(path, '/')))
		fd = open(path, oflags);
	else if (dflags & OPENDEV_PART) {
		if (snprintf(namebuf, PATH_MAX, "%s%s%s",
		    _PATH_DEV, prefix, path) < PATH_MAX) {
			char *slice;
			while ((slice = strrchr(namebuf, 's')) &&
			    isdigit(*(slice-1))) *slice = '\0';
			fd = open(namebuf, oflags);
			if (realpath)
				*realpath = namebuf;
		} else
			errno = ENAMETOOLONG;
	}
	if (!slash && fd == -1 && errno == ENOENT) {
		if (snprintf(namebuf, PATH_MAX, "%s%s%s",
		    _PATH_DEV, prefix, path) < PATH_MAX) {
			fd = open(namebuf, oflags);
			if (realpath)
				*realpath = namebuf;
		} else
			errno = ENAMETOOLONG;
	}
	return (fd);
}

static void opendev_namebuf_init(void)
{
	namebuf = malloc(PATH_MAX);
}

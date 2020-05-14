/*
 * Copyright (c) 1983, 1993
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
static char sccsid[] = "@(#)scandir.c	8.3 (Berkeley) 1/2/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * Scan the directory dirname calling select to make a list of selected
 * directory entries then sort using qsort and compare routine dcomp.
 * Returns the number of entries and a pointer to a list of pointers to
 * struct dirent (through namelist). Returns -1 if there were any errors.
 */

#include "namespace.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include "un-namespace.h"

/*
 * The DIRSIZ macro is the minimum record length which will hold the directory
 * entry.  This requires the amount of space in struct dirent without the
 * d_name field, plus enough space for the name and a terminating nul byte
 * (dp->d_namlen + 1), rounded up to a 4 byte boundary.
 */
#undef DIRSIZ
#define DIRSIZ(dp)							\
	((sizeof(struct dirent) - sizeof(dp)->d_name) +			\
	    (((dp)->d_namlen + 1 + 3) &~ 3))

int
#ifdef I_AM_SCANDIR_B
scandir_b(const char *dirname, struct dirent ***namelist,
    int (^select)(const struct dirent *),
    int (^_dcomp)(const struct dirent **, const struct dirent **))
#else
scandir(const char *dirname, struct dirent ***namelist,
    int (*select)(const struct dirent *),
    int (*_dcomp)(const struct dirent **, const struct dirent **))
#endif
{
	struct dirent *d, *p, **names = NULL;
	size_t nitems = 0;
	long arraysz;
	DIR *dirp;

	/* see <rdar://problem/10293482> */
#ifdef I_AM_SCANDIR_B
	int (^dcomp)(const void *, const void *) = (void *)_dcomp;
#else
	int (*dcomp)(const void *, const void *) = (void *)_dcomp;
#endif

	if ((dirp = opendir(dirname)) == NULL)
		return(-1);

	arraysz = 32;	/* initial estimate of the array size */
	names = (struct dirent **)malloc(arraysz * sizeof(struct dirent *));
	if (names == NULL)
		goto fail;

	while ((d = readdir(dirp)) != NULL) {
		if (select != NULL && !select(d))
			continue;	/* just selected names */
		/*
		 * Make a minimum size copy of the data
		 */
		p = (struct dirent *)malloc(DIRSIZ(d));
		if (p == NULL)
			goto fail;
		p->d_fileno = d->d_fileno;
		p->d_type = d->d_type;
		p->d_reclen = d->d_reclen;
		p->d_namlen = d->d_namlen;
		bcopy(d->d_name, p->d_name, p->d_namlen + 1);
		/*
		 * Check to make sure the array has space left and
		 * realloc the maximum size.
		 */
		if (nitems >= arraysz) {
			struct dirent **names2;

			names2 = (struct dirent **)realloc((char *)names,
				(arraysz * 2) * sizeof(struct dirent *));
			if (names2 == NULL) {
				free(p);
				goto fail;
			}
			names = names2;
			arraysz *= 2;
		}
		names[nitems++] = p;
	}
	closedir(dirp);
	if (nitems && dcomp != NULL)
#ifdef I_AM_SCANDIR_B
		qsort_b(names, nitems, sizeof(struct dirent *), dcomp);
#else
		qsort(names, nitems, sizeof(struct dirent *), dcomp);
#endif
	*namelist = names;
	return (int) (nitems);

fail:
	while (nitems > 0)
		free(names[--nitems]);
	free(names);
	closedir(dirp);
	return (-1);
}

#ifndef I_AM_SCANDIR_B
/*
 * Alphabetic order comparison routine for those who want it.
 * POSIX 2008 requires that alphasort() uses strcoll().
 */
int
alphasort(const struct dirent **d1, const struct dirent **d2)
{

	return (strcoll((*d1)->d_name, (*d2)->d_name));
}
#endif // I_AM_SCANDIR_B


/*	$NetBSD: getmntopts.c,v 1.3 2003/08/07 16:44:58 agc Exp $	*/

/*-
 * Copyright (c) 1994
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

#include <sys/cdefs.h>
#ifndef lint
#if 0
static char sccsid[] = "@(#)getmntopts.c	8.3 (Berkeley) 3/29/95";
#else
__RCSID("$NetBSD: getmntopts.c,v 1.3 2003/08/07 16:44:58 agc Exp $");
#endif
#endif /* not lint */

#include <sys/param.h>

#include <err.h>
#include <errno.h>
#include <fstab.h>
#include <stdlib.h>
#include <string.h>

#include <mntopts.h>

int getmnt_silent = 0;

static const char errmsg[] = "-o %s: option not supported";

struct mntoptparse {
	const char *options;
	const struct mntopt *mopts;
	char *optbuf;
	char **optarg;
};

const char *
getmntoptstr(mntoptparse_t mp, const char *opt)
{
	const struct mntopt *m;

	for (m = mp->mopts; m->m_option != NULL; m++)
		if (strcasecmp(opt, m->m_option) == 0)
			break;

	if (m->m_option == NULL) {
		if (getmnt_silent == 0)
			errx(1, errmsg, opt);
		else
			return NULL;
	}

	return mp->optarg[m - mp->mopts];
}

long
getmntoptnum(mntoptparse_t mp, const char *opt)
{
	char *ep;
	long rv;
	void (*fun)(int, const char *, ...) = NULL;
	const char *val = getmntoptstr(mp, opt);

	if (val == NULL) {
		if (getmnt_silent == 0)
			errx(1, "Missing %s argument", opt);
		else
			return -1;
	}

	errno = 0;
	rv = strtol(val, &ep, 0);

	if (*ep)
		fun = errx;

	if (errno == ERANGE && (rv == LONG_MAX || rv == LONG_MIN))
		fun = err;

	if (fun) {
		if (getmnt_silent != 0)
			return -1;
		(*fun)(1, "Invalid %s argument `%s'", opt, val);
	}
	return rv;
}

void
freemntopts(mntoptparse_t mp)
{
	free(mp->optbuf);
	free(mp->optarg);
	free(mp);
}

mntoptparse_t
getmntopts(const char *options, const struct mntopt *m0, int *flagp,
    int *altflagp)
{
	const struct mntopt *m;
	int negative;
	char *opt, *p, *ctx = NULL;
	int *thisflagp;
	size_t nopts;
	mntoptparse_t mp;

	for (nopts = 0, m = m0; m->m_option != NULL; ++m, nopts++)
		continue;
	
	if ((mp = malloc(sizeof(struct mntoptparse))) == NULL)
		return NULL;

	/* Copy option string, since it is about to be torn asunder... */
	if ((mp->optbuf = strdup(options)) == NULL) {
		free(mp);
		return NULL;
	}

	if ((mp->optarg = calloc(nopts, sizeof(char *))) == NULL) {
		free(mp->optbuf);
		free(mp);
		return NULL;
	}

	mp->mopts = m0;
	mp->options = options;

	for (opt = mp->optbuf; (opt = strtok_r(opt, ",", &ctx)) != NULL; opt = NULL) {
		/* Check for "no" prefix. */
		if (opt[0] == 'n' && opt[1] == 'o') {
			negative = 1;
			opt += 2;
		} else
			negative = 0;

		/*
		 * for options with assignments in them (ie. quotas)
		 * ignore the assignment as it's handled elsewhere
		 */
		p = strchr(opt, '=');
		if (p) {
			 *p++ = '\0';
		}

		/* Scan option table. */
		for (m = m0; m->m_option != NULL; ++m)
			if (strcasecmp(opt, m->m_option) == 0)
				break;

		/* Save flag, or fail if option is not recognised. */
		if (m->m_option) {
			mp->optarg[m - m0] = p;
			thisflagp = m->m_altloc ? altflagp : flagp;
			if (negative == m->m_inverse)
				*thisflagp |= m->m_flag;
			else
				*thisflagp &= ~m->m_flag;
		} else if (!getmnt_silent) {
			errx(1, errmsg, opt);
		}
	}
	return mp;
}

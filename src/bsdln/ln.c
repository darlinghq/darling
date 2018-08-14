/*-
 * Copyright (c) 1987, 1993, 1994
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

#if 0
#ifndef lint
static char const copyright[] =
"@(#) Copyright (c) 1987, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)ln.c	8.2 (Berkeley) 3/31/94";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
//__FBSDID("$FreeBSD: src/bin/ln/ln.c,v 1.34 2006/02/14 11:08:05 glebius Exp $");

#include <bsd/string.h>

#include <sys/param.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int	fflag;				/* Unlink existing files. */
int	Fflag;				/* Remove empty directories also. */
int	hflag;				/* Check new name for symlink first. */
int	iflag;				/* Interactive mode. */
int	sflag;				/* Symbolic, not hard, link. */
int	vflag;				/* Verbose output. */
					/* System link call. */
int (*linkf)(const char *, const char *);
char	linkch;

int	linkit(const char *, const char *, int);
void	usage(void);

int
main(int argc, char *argv[])
{
	struct stat sb;
	char *p, *sourcedir;
	int ch, exitval;

	if (argc < 1)
		usage();
	/*
	 * Test for the special case where the utility is called as
	 * "link", for which the functionality provided is greatly
	 * simplified.
	 */
	if ((p = rindex(argv[0], '/')) == NULL)
		p = argv[0];
	else
		++p;
	if (strcmp(p, "link") == 0) {
		while (getopt(argc, argv, "") != -1)
			usage();
		argc -= optind;
		argv += optind;
		if (argc != 2)
			usage();
		linkf = link;
		exit(linkit(argv[0], argv[1], 0));
	}

	while ((ch = getopt(argc, argv, "Ffhinsv")) != -1)
		switch (ch) {
		case 'F':
			Fflag = 1;
			break;
		case 'f':
			fflag = 1;
			iflag = 0;
			break;
		case 'h':
		case 'n':
			hflag = 1;
			break;
		case 'i':
			iflag = 1;
			fflag = 0;
			break;
		case 's':
			sflag = 1;
			break;
		case 'v':
			vflag = 1;
			break;
		case '?':
		default:
			usage();
		}

	argv += optind;
	argc -= optind;

	linkf = sflag ? symlink : link;
	linkch = sflag ? '-' : '=';
	if (sflag == 0)
		Fflag = 0;
	if (Fflag == 1 && iflag == 0)
		fflag = 1;

	switch(argc) {
	case 0:
		usage();
		/* NOTREACHED */
	case 1:				/* ln target */
		exit(linkit(argv[0], ".", 1));
	case 2:				/* ln target source */
		exit(linkit(argv[0], argv[1], 0));
	default:
		;
	}
					/* ln target1 target2 directory */
	sourcedir = argv[argc - 1];
	if (hflag && lstat(sourcedir, &sb) == 0 && S_ISLNK(sb.st_mode)) {
		/*
		 * We were asked not to follow symlinks, but found one at
		 * the target--simulate "not a directory" error
		 */
		errno = ENOTDIR;
		err(1, "%s", sourcedir);
	}
	if (stat(sourcedir, &sb))
		err(1, "%s", sourcedir);
	if (!S_ISDIR(sb.st_mode))
		usage();
	for (exitval = 0; *argv != sourcedir; ++argv)
		exitval |= linkit(*argv, sourcedir, 1);
	exit(exitval);
}

int
linkit(const char *target, const char *source, int isdir)
{
	struct stat sb;
	const char *p;
	int ch, exists, first;
	char path[PATH_MAX];
	char bbuf[PATH_MAX];

	if (!sflag) {
		/* If target doesn't exist, quit now. */
		if (stat(target, &sb)) {
			warn("%s", target);
			return (1);
		}
		/* Only symbolic links to directories. */
		if (S_ISDIR(sb.st_mode)) {
			errno = EISDIR;
			warn("%s", target);
			return (1);
		}
	}

	/*
	 * If the source is a directory (and not a symlink if hflag),
	 * append the target's name.
	 */
	if (isdir ||
	    (lstat(source, &sb) == 0 && S_ISDIR(sb.st_mode)) ||
	    (!hflag && stat(source, &sb) == 0 && S_ISDIR(sb.st_mode))) {
		if (strlcpy(bbuf, target, sizeof(bbuf)) >= sizeof(bbuf) ||
		    (p = basename(bbuf)) == NULL ||
		    snprintf(path, sizeof(path), "%s/%s", source, p) >=
		    (ssize_t)sizeof(path)) {
			errno = ENAMETOOLONG;
			warn("%s", target);
			return (1);
		}
		source = path;
	}

	exists = !lstat(source, &sb);
	/*
	 * If the file exists, then unlink it forcibly if -f was specified
	 * and interactively if -i was specified.
	 */
	if (fflag && exists) {
		if (Fflag && S_ISDIR(sb.st_mode)) {
			if (rmdir(source)) {
				warn("%s", source);
				return (1);
			}
		} else if (unlink(source)) {
			warn("%s", source);
			return (1);
		}
	} else if (iflag && exists) {
		fflush(stdout);
		fprintf(stderr, "replace %s? ", source);

		first = ch = getchar();
		while(ch != '\n' && ch != EOF)
			ch = getchar();
		if (first != 'y' && first != 'Y') {
			fprintf(stderr, "not replaced\n");
			return (1);
		}

		if (Fflag && S_ISDIR(sb.st_mode)) {
			if (rmdir(source)) {
				warn("%s", source);
				return (1);
			}
		} else if (unlink(source)) {
			warn("%s", source);
			return (1);
		}
	}

	/* Attempt the link. */
	if ((*linkf)(target, source)) {
		warn("%s", source);
		return (1);
	}
	if (vflag)
		(void)printf("%s %c> %s\n", source, linkch, target);
	return (0);
}

void
usage(void)
{
	(void)fprintf(stderr, "%s\n%s\n%s\n",
	    "usage: ln [-Ffhinsv] source_file [target_file]",
	    "       ln [-Ffhinsv] source_file ... target_dir",
	    "       link source_file target_file");
	exit(1);
}

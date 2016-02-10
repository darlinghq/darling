/*
 * Copyright (c) 1999, 2000, 2003, 2005, 2008 Apple Inc. All rights reserved.
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
 * Copyright (c) 1990, 1993, 1994
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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


#include <sys/param.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __BLOCKS__
#include <Block.h>
#endif /* __BLOCKS__ */

static FTSENT	*fts_alloc(FTS *, char *, int);
static FTSENT	*fts_build(FTS *, int);
static void	 fts_lfree(FTSENT *);
static void	 fts_load(FTS *, FTSENT *);
static size_t	 fts_maxarglen(char * const *);
static void	 fts_padjust(FTS *, void *);
static int	 fts_palloc(FTS *, size_t);
static FTSENT	*fts_sort(FTS *, FTSENT *, int);
static u_short	 fts_stat(FTS *, FTSENT *, int);

#define	ISDOT(a)	(a[0] == '.' && (!a[1] || a[1] == '.' && !a[2]))

#define	ISSET(opt)	(sp->fts_options & opt)
#define	SET(opt)	(sp->fts_options |= opt)

#define	CHDIR(sp, path)	(!ISSET(FTS_NOCHDIR) && chdir(path))
#define	FCHDIR(sp, fd)	(!ISSET(FTS_NOCHDIR) && fchdir(fd))

/* fts_build flags */
#define	BCHILD		1		/* fts_children */
#define	BNAMES		2		/* fts_children, names only */
#define	BREAD		3		/* fts_read */

/* 5653270
 * For directories containing > 64k subdirectories (or HFS+ with > 64k files
 * and subdirectories), struct stat's st_nlink (16 bits) will overflow.  This
 * causes the case with FTS_NOSTAT and FTS_PHYSICAL set to prematurely stop
 * recursing into subdirectories, because of an optimization that expects
 * st_nlink to be the number of subdirectories (once that number has been
 * encountered, no further calls to stat should be needed).
 *
 * However, on Mac OS X, another optimization largely nullifies the st_nlink
 * optimization.  struct dirent contains d_type, which can distinguish
 * directories from files without initially calling stat. So stat is only
 * called on known directories, rather than on other files.  With this
 * optimization, the difference in also using the st_nlink optimization is
 * pretty minimal (tests show an improvement of a percent or two, probably
 * due to additional if statement clauses that need to be evaluated). 
 *
 * So removing the st_nlink optimization code will fix the > 64k subdirectories
 * problem.  And if we replace the multiple if clause logic with a single
 * switch statement, we can recover the minimal performance lose.  We can
 * go even further and for the case of FTS_NOSTAT and FTS_LOGICAL set, we
 * can use d_type to also distinguish symbolic links, and so we only need to
 * call stat on directories and symlinks, not on all files.  This provides
 * a significant performance boost in that special case.
 */
/*
 * The following macros defines values of the dostat variable, which is or-ed
 * with the value of d_type, and the result used in a switch statement to
 * determine whether to call stat or not.  (We order the macros to minimize
 * the size of any jump table that the compiler may generate.)
 */
#define	F_SHIFT		4		/* shift to leave space for d_type */
#define F_NOSTAT	(0 << F_SHIFT)	/* don't do any stat's */
#define F_STATDIRSYM	(1 << F_SHIFT)	/* only stat directories and symlinks (and unknowns) */
#define F_ALWAYSSTAT	(2 << F_SHIFT)	/* always stat */
#define F_STATDIR	(3 << F_SHIFT)	/* only stat directories (and unknowns) */

static FTS *
__fts_open(argv, sp)
	char * const *argv;
	register FTS *sp;
{
	register FTSENT *p, *root;
	register int nitems;
	FTSENT *parent, *tmp = NULL;
	int len;

	/* Logical walks turn on NOCHDIR; symbolic links are too hard. */
	if (ISSET(FTS_LOGICAL))
		SET(FTS_NOCHDIR);

	/*
	 * Start out with 1K of path space, and enough, in any case,
	 * to hold the user's paths.
	 */
	if (fts_palloc(sp, MAX(fts_maxarglen(argv), MAXPATHLEN)))
		goto mem1;

	/* Allocate/initialize root's parent. */
	if ((parent = fts_alloc(sp, "", 0)) == NULL)
		goto mem2;
	parent->fts_level = FTS_ROOTPARENTLEVEL;

	/* Allocate/initialize root(s). */
	for (root = NULL, nitems = 0; *argv; ++argv, ++nitems) {
		/* Don't allow zero-length paths. */
		if ((len = strlen(*argv)) == 0) {
			errno = ENOENT;
			goto mem3;
		}

		p = fts_alloc(sp, *argv, len);
		p->fts_level = FTS_ROOTLEVEL;
		p->fts_parent = parent;
		p->fts_accpath = p->fts_name;
		p->fts_info = fts_stat(sp, p, ISSET(FTS_COMFOLLOWDIR) ? -1 : ISSET(FTS_COMFOLLOW));

		/* Command-line "." and ".." are real directories. */
		if (p->fts_info == FTS_DOT)
			p->fts_info = FTS_D;

		/*
		 * If comparison routine supplied, traverse in sorted
		 * order; otherwise traverse in the order specified.
		 */
		if (sp->fts_compar) {
			p->fts_link = root;
			root = p;
		} else {
			p->fts_link = NULL;
			if (root == NULL)
				tmp = root = p;
			else {
				tmp->fts_link = p;
				tmp = p;
			}
		}
	}
	if (sp->fts_compar && nitems > 1)
		root = fts_sort(sp, root, nitems);

	/*
	 * Allocate a dummy pointer and make fts_read think that we've just
	 * finished the node before the root(s); set p->fts_info to FTS_INIT
	 * so that everything about the "current" node is ignored.
	 */
	if ((sp->fts_cur = fts_alloc(sp, "", 0)) == NULL)
		goto mem3;
	sp->fts_cur->fts_link = root;
	sp->fts_cur->fts_info = FTS_INIT;

	/*
	 * If using chdir(2), grab a file descriptor pointing to dot to insure
	 * that we can get back here; this could be avoided for some paths,
	 * but almost certainly not worth the effort.  Slashes, symbolic links,
	 * and ".." are all fairly nasty problems.  Note, if we can't get the
	 * descriptor we run anyway, just more slowly.
	 */
	if (!ISSET(FTS_NOCHDIR) && (sp->fts_rfd = open(".", O_RDONLY, 0)) < 0)
		SET(FTS_NOCHDIR);

	return (sp);

mem3:	fts_lfree(root);
	free(parent);
mem2:	free(sp->fts_path);
mem1:	free(sp);
	return (NULL);
}

FTS *
fts_open(argv, options, compar)
	char * const *argv;
	int options;
	int (*compar)();
{
	register FTS *sp;

	/* Options check. */
	if (options & ~FTS_OPTIONMASK) {
		errno = EINVAL;
		return (NULL);
	}

	/* Allocate/initialize the stream */
	if ((sp = malloc((u_int)sizeof(FTS))) == NULL)
		return (NULL);
	memset(sp, 0, sizeof(FTS));
	sp->fts_compar = compar;
	sp->fts_options = options;

	return __fts_open(argv, sp);
}

#ifdef __BLOCKS__
FTS *
fts_open_b(argv, options, compar)
	char * const *argv;
	int options;
	int (^compar)(const FTSENT **, const FTSENT **);
{
	register FTS *sp;

	/* Options check. */
	if (options & ~FTS_OPTIONMASK) {
		errno = EINVAL;
		return (NULL);
	}

	/* Allocate/initialize the stream */
	if ((sp = malloc((u_int)sizeof(FTS))) == NULL)
		return (NULL);
	memset(sp, 0, sizeof(FTS));
	sp->fts_compar_b = (int (^)())Block_copy(compar);
	sp->fts_options = options | FTS_BLOCK_COMPAR;

	return __fts_open(argv, sp);
}
#endif /* __BLOCKS__ */

static void
fts_load(sp, p)
	FTS *sp;
	register FTSENT *p;
{
	register int len;
	register char *cp;

	/*
	 * Load the stream structure for the next traversal.  Since we don't
	 * actually enter the directory until after the preorder visit, set
	 * the fts_accpath field specially so the chdir gets done to the right
	 * place and the user can access the first node.  From fts_open it's
	 * known that the path will fit.
	 */
	len = p->fts_pathlen = p->fts_namelen;
	memmove(sp->fts_path, p->fts_name, len + 1);
	if ((cp = strrchr(p->fts_name, '/')) && (cp != p->fts_name || cp[1])) {
		len = strlen(++cp);
		memmove(p->fts_name, cp, len + 1);
		p->fts_namelen = len;
	}
	p->fts_accpath = p->fts_path = sp->fts_path;
	sp->fts_dev = p->fts_dev;
}

int
fts_close(sp)
	FTS *sp;
{
	register FTSENT *freep, *p;
	int saved_errno = 0;

	/*
	 * This still works if we haven't read anything -- the dummy structure
	 * points to the root list, so we step through to the end of the root
	 * list which has a valid parent pointer.
	 */
	if (sp->fts_cur) {
		for (p = sp->fts_cur; p->fts_level >= FTS_ROOTLEVEL;) {
			freep = p;
			p = p->fts_link ? p->fts_link : p->fts_parent;
			free(freep);
		}
		free(p);
	}

	/* Free up child linked list, sort array, path buffer. */
	if (sp->fts_child)
		fts_lfree(sp->fts_child);
	if (sp->fts_array)
		free(sp->fts_array);
	free(sp->fts_path);

	/* Return to original directory, save errno if necessary. */
	if (!ISSET(FTS_NOCHDIR)) {
            if (fchdir(sp->fts_rfd)) {
                saved_errno = errno;
            }
            (void)close(sp->fts_rfd);
	}

#ifdef __BLOCKS__
	/* Free up any block pointer. */
	if (ISSET(FTS_BLOCK_COMPAR) && sp->fts_compar_b != NULL)
		Block_release(sp->fts_compar_b);
#endif /* __BLOCKS__ */

	/* Free up the stream pointer. */
	free(sp);

	/* Set errno and return. */
	if (saved_errno) {
		errno = saved_errno;
		return (-1);
	}
	return (0);
}

/*
 * Special case a root of "/" so that slashes aren't appended which would
 * cause paths to be written as "//foo".
 */
#define	NAPPEND(p)							\
	(p->fts_level == FTS_ROOTLEVEL && p->fts_pathlen == 1 &&	\
	    p->fts_path[0] == '/' ? 0 : p->fts_pathlen)

FTSENT *
fts_read(sp)
	register FTS *sp;
{
	register FTSENT *p, *tmp;
	register int instr;
	register char *t;
	int saved_errno;

	/* If finished or unrecoverable error, return NULL. */
	if (sp->fts_cur == NULL || ISSET(FTS_STOP))
		return (NULL);

	/* Set current node pointer. */
	p = sp->fts_cur;

	/* Save and zero out user instructions. */
	instr = p->fts_instr;
	p->fts_instr = FTS_NOINSTR;

	/* Any type of file may be re-visited; re-stat and re-turn. */
	if (instr == FTS_AGAIN) {
		p->fts_info = fts_stat(sp, p, 0);
		return (p);
	}

	/*
	 * Following a symlink -- SLNONE test allows application to see
	 * SLNONE and recover.  If indirecting through a symlink, have
	 * keep a pointer to current location.  If unable to get that
	 * pointer, follow fails.
	 */
	if (instr == FTS_FOLLOW &&
	    (p->fts_info == FTS_SL || p->fts_info == FTS_SLNONE)) {
		p->fts_info = fts_stat(sp, p, 1);
		if (p->fts_info == FTS_D && !ISSET(FTS_NOCHDIR))
			if ((p->fts_symfd = open(".", O_RDONLY, 0)) < 0) {
				p->fts_errno = errno;
				p->fts_info = FTS_ERR;
			} else
				p->fts_flags |= FTS_SYMFOLLOW;
		return (p);
	}

	/* Directory in pre-order. */
	if (p->fts_info == FTS_D) {
		/* If skipped or crossed mount point, do post-order visit. */
		if (instr == FTS_SKIP ||
		    ISSET(FTS_XDEV) && p->fts_dev != sp->fts_dev) {
			if (p->fts_flags & FTS_SYMFOLLOW)
				(void)close(p->fts_symfd);
			if (sp->fts_child) {
				fts_lfree(sp->fts_child);
				sp->fts_child = NULL;
			}
			p->fts_info = FTS_DP;
			return (p);
		} 

		/* Rebuild if only read the names and now traversing. */
		if (sp->fts_child && sp->fts_options & FTS_NAMEONLY) {
			sp->fts_options &= ~FTS_NAMEONLY;
			fts_lfree(sp->fts_child);
			sp->fts_child = NULL;
		}

		/*
		 * Cd to the subdirectory.
		 *
		 * If have already read and now fail to chdir, whack the list
		 * to make the names come out right, and set the parent errno
		 * so the application will eventually get an error condition.
		 * Set the FTS_DONTCHDIR flag so that when we logically change
		 * directories back to the parent we don't do a chdir.
		 *
		 * If haven't read do so.  If the read fails, fts_build sets
		 * FTS_STOP or the fts_info field of the node.
		 */
		if (sp->fts_child) {
			if (CHDIR(sp, p->fts_accpath)) {
				p->fts_errno = errno;
				p->fts_flags |= FTS_DONTCHDIR;
				for (p = sp->fts_child; p; p = p->fts_link)
					p->fts_accpath =
					    p->fts_parent->fts_accpath;
			}
		} else if ((sp->fts_child = fts_build(sp, BREAD)) == NULL) {
			if (ISSET(FTS_STOP))
				return (NULL);
			return (p);
		}
		p = sp->fts_child;
		sp->fts_child = NULL;
		goto name;
	}

	/* Move to the next node on this level. */
next:	tmp = p;
	if (p = p->fts_link) {
		/*
		 * If reached the top, return to the original directory, and
		 * load the paths for the next root.
		 */
		if (p->fts_level == FTS_ROOTLEVEL) {
			if (!ISSET(FTS_NOCHDIR) && FCHDIR(sp, sp->fts_rfd)) {
				SET(FTS_STOP);
				return (NULL);
			}
			fts_load(sp, p);
			free(tmp);
			return (sp->fts_cur = p);
		}

		/*
		 * User may have called fts_set on the node.  If skipped,
		 * ignore.  If followed, get a file descriptor so we can
		 * get back if necessary.
		 */
		if (p->fts_instr == FTS_SKIP) {
			free(tmp);
			goto next;
		}
		if (p->fts_instr == FTS_FOLLOW) {
			p->fts_info = fts_stat(sp, p, 1);
			if (p->fts_info == FTS_D && !ISSET(FTS_NOCHDIR))
				if ((p->fts_symfd =
				    open(".", O_RDONLY, 0)) < 0) {
					p->fts_errno = errno;
					p->fts_info = FTS_ERR;
				} else
					p->fts_flags |= FTS_SYMFOLLOW;
			p->fts_instr = FTS_NOINSTR;
		}

		free(tmp);
name:		t = sp->fts_path + NAPPEND(p->fts_parent);
		*t++ = '/';
		memmove(t, p->fts_name, p->fts_namelen + 1);
		return (sp->fts_cur = p);
	}

	/* Move up to the parent node. */
	p = tmp->fts_parent;
	free(tmp);

	if (p->fts_level == FTS_ROOTPARENTLEVEL) {
		/*
		 * Done; free everything up and set errno to 0 so the user
		 * can distinguish between error and EOF.
		 */
		free(p);
		errno = 0;
		return (sp->fts_cur = NULL);
	}

	/* Nul terminate the pathname. */
	sp->fts_path[p->fts_pathlen] = '\0';

	/*
	 * Return to the parent directory.  If at a root node or came through
	 * a symlink, go back through the file descriptor.  Otherwise, cd up
	 * one directory.
	 */
	if (p->fts_level == FTS_ROOTLEVEL) {
		if (!ISSET(FTS_NOCHDIR) && FCHDIR(sp, sp->fts_rfd)) {
			SET(FTS_STOP);
			return (NULL);
		}
	} else if (p->fts_flags & FTS_SYMFOLLOW) {
		if (FCHDIR(sp, p->fts_symfd)) {
			saved_errno = errno;
			(void)close(p->fts_symfd);
			errno = saved_errno;
			SET(FTS_STOP);
			return (NULL);
		}
		(void)close(p->fts_symfd);
	} else if (!(p->fts_flags & FTS_DONTCHDIR)) {
		if (CHDIR(sp, "..")) {
			SET(FTS_STOP);
			return (NULL);
		}
	}
	p->fts_info = p->fts_errno ? FTS_ERR : FTS_DP;
	return (sp->fts_cur = p);
}

/*
 * Fts_set takes the stream as an argument although it's not used in this
 * implementation; it would be necessary if anyone wanted to add global
 * semantics to fts using fts_set.  An error return is allowed for similar
 * reasons.
 */
/* ARGSUSED */
int
fts_set(sp, p, instr)
	FTS *sp;
	FTSENT *p;
	int instr;
{
	if (instr && instr != FTS_AGAIN && instr != FTS_FOLLOW &&
	    instr != FTS_NOINSTR && instr != FTS_SKIP) {
		errno = EINVAL;
		return (1);
	}
	p->fts_instr = instr;
	return (0);
}

FTSENT *
fts_children(sp, instr)
	register FTS *sp;
	int instr;
{
	register FTSENT *p;
	int fd;

	if (instr && instr != FTS_NAMEONLY) {
		errno = EINVAL;
		return (NULL);
	}

	/* Set current node pointer. */
	p = sp->fts_cur;

	/*
	 * Errno set to 0 so user can distinguish empty directory from
	 * an error.
	 */
	errno = 0;

	/* Fatal errors stop here. */
	if (ISSET(FTS_STOP))
		return (NULL);

	/* Return logical hierarchy of user's arguments. */
	if (p->fts_info == FTS_INIT)
		return (p->fts_link);

	/*
	 * If not a directory being visited in pre-order, stop here.  Could
	 * allow FTS_DNR, assuming the user has fixed the problem, but the
	 * same effect is available with FTS_AGAIN.
	 */
	if (p->fts_info != FTS_D /* && p->fts_info != FTS_DNR */)
		return (NULL);

	/* Free up any previous child list. */
	if (sp->fts_child)
		fts_lfree(sp->fts_child);

	if (instr == FTS_NAMEONLY) {
		sp->fts_options |= FTS_NAMEONLY;
		instr = BNAMES;
	} else 
		instr = BCHILD;

	/*
	 * If using chdir on a relative path and called BEFORE fts_read does
	 * its chdir to the root of a traversal, we can lose -- we need to
	 * chdir into the subdirectory, and we don't know where the current
	 * directory is, so we can't get back so that the upcoming chdir by
	 * fts_read will work.
	 */
	if (p->fts_level != FTS_ROOTLEVEL || p->fts_accpath[0] == '/' ||
	    ISSET(FTS_NOCHDIR))
		return (sp->fts_child = fts_build(sp, instr));

	if ((fd = open(".", O_RDONLY, 0)) < 0)
		return (NULL);
	sp->fts_child = fts_build(sp, instr);
	if (fchdir(fd))
		return (NULL);
	(void)close(fd);
	return (sp->fts_child);
}

/*
 * This is the tricky part -- do not casually change *anything* in here.  The
 * idea is to build the linked list of entries that are used by fts_children
 * and fts_read.  There are lots of special cases.
 *
 * The real slowdown in walking the tree is the stat calls.  If FTS_NOSTAT is
 * set, we can use d_type to determine if the entry is a directory (or for
 * logical walks, a directory or symlink) and not call stat for other file
 * types.  This cuts the number of stat calls significantly.
 */
static FTSENT *
fts_build(sp, type)
	register FTS *sp;
	int type;
{
	register struct dirent *dp;
	register FTSENT *p, *head;
	register int nitems;
	FTSENT *cur, *tail;
	DIR *dirp;
	void *adjaddr;
	int cderrno, descend, len, level, maxlen, dostat, oflag, saved_errno;
	char *cp = NULL;

	/* Set current node pointer. */
	cur = sp->fts_cur;

	/*
	 * Open the directory for reading.  If this fails, we're done.
	 * If being called from fts_read, set the fts_info field.
	 */
#ifdef FTS_WHITEOUT
	if (ISSET(FTS_WHITEOUT))
		oflag = DTF_NODUP|DTF_REWIND;
	else
		oflag = DTF_HIDEW|DTF_NODUP|DTF_REWIND;
#else
#define __opendir2(path, flag) opendir(path)
#endif
	if ((dirp = __opendir2(cur->fts_accpath, oflag)) == NULL) {
		if (type == BREAD) {
			cur->fts_info = FTS_DNR;
			cur->fts_errno = errno;
		}
		return (NULL);
	}

	if (type == BNAMES)
		dostat = F_NOSTAT;
	else if (ISSET(FTS_NOSTAT))
		dostat = ISSET(FTS_PHYSICAL) ? F_STATDIR : F_STATDIRSYM;
	else
		dostat = F_ALWAYSSTAT;

#ifdef notdef
	(void)printf("dostat == %d\n", dostat);
	(void)printf("NOSTAT %d PHYSICAL %d SEEDOT %d\n",
	    ISSET(FTS_NOSTAT), ISSET(FTS_PHYSICAL), ISSET(FTS_SEEDOT));
#endif
	/*
	 * If we're going to need to stat anything or we want to descend
	 * and stay in the directory, chdir.  If this fails we keep going,
	 * but set a flag so we don't chdir after the post-order visit.
	 * We won't be able to stat anything, but we can still return the
	 * names themselves.  Note, that since fts_read won't be able to
	 * chdir into the directory, it will have to return different path
	 * names than before, i.e. "a/b" instead of "b".  Since the node
	 * has already been visited in pre-order, have to wait until the
	 * post-order visit to return the error.  There is a special case
	 * here, if there was nothing to stat then it's not an error to
	 * not be able to stat.  This is all fairly nasty.  If a program
	 * needed sorted entries or stat information, they had better be
	 * checking FTS_NS on the returned nodes.
	 */
	cderrno = 0;
	if (dostat || type == BREAD)
		if (FCHDIR(sp, dirfd(dirp))) {
			if (dostat && type == BREAD)
				cur->fts_errno = errno;
			cur->fts_flags |= FTS_DONTCHDIR;
			descend = 0;
			cderrno = errno;
		} else
			descend = 1;
	else
		descend = 0;

	/*
	 * Figure out the max file name length that can be stored in the
	 * current path -- the inner loop allocates more path as necessary.
	 * We really wouldn't have to do the maxlen calculations here, we
	 * could do them in fts_read before returning the path, but it's a
	 * lot easier here since the length is part of the dirent structure.
	 *
	 * If not changing directories set a pointer so that can just append
	 * each new name into the path.
	 */
	maxlen = sp->fts_pathlen - cur->fts_pathlen - 1;
	len = NAPPEND(cur);
	if (ISSET(FTS_NOCHDIR)) {
		cp = sp->fts_path + len;
		*cp++ = '/';
	}

	level = cur->fts_level + 1;

	/* Read the directory, attaching each entry to the `link' pointer. */
	adjaddr = NULL;
	for (head = tail = NULL, nitems = 0; dp = readdir(dirp);) {
		if (!ISSET(FTS_SEEDOT) && ISDOT(dp->d_name))
			continue;

		if ((p = fts_alloc(sp, dp->d_name, (int)dp->d_namlen)) == NULL)
			goto mem1;
		if (dp->d_namlen > maxlen) {
			if (fts_palloc(sp, (size_t)dp->d_namlen)) {
				/*
				 * No more memory for path or structures.  Save
				 * errno, free up the current structure and the
				 * structures already allocated.
				 */
mem1:				saved_errno = errno;
				if (p)
					free(p);
				fts_lfree(head);
				(void)closedir(dirp);
				errno = saved_errno;
				cur->fts_info = FTS_ERR;
				SET(FTS_STOP);
				return (NULL);
			}
			adjaddr = sp->fts_path;
			maxlen = sp->fts_pathlen - sp->fts_cur->fts_pathlen - 1;
		}

		p->fts_pathlen = len + dp->d_namlen + 1;
		p->fts_parent = sp->fts_cur;
		p->fts_level = level;

#ifdef FTS_WHITEOUT
		if (dp->d_type == DT_WHT)
			p->fts_flags |= FTS_ISW;
#endif

		if (cderrno) {
			if (dostat) {
				p->fts_info = FTS_NS;
				p->fts_errno = cderrno;
			} else
				p->fts_info = FTS_NSOK;
			p->fts_accpath = cur->fts_accpath;
		} else {
			/*
			 * We need to know all file types values that d_type may
			 * be set to.  So if that changes, the following needs
			 * to be modified appropriately.
			 */
			switch(dostat | dp->d_type) {
			case (F_STATDIR | DT_UNKNOWN):
			case (F_STATDIR | DT_DIR):
			case (F_STATDIRSYM | DT_UNKNOWN):
			case (F_STATDIRSYM | DT_DIR):
			case (F_STATDIRSYM | DT_LNK):
			case (F_ALWAYSSTAT | DT_UNKNOWN):
			case (F_ALWAYSSTAT | DT_FIFO):
			case (F_ALWAYSSTAT | DT_CHR):
			case (F_ALWAYSSTAT | DT_DIR):
			case (F_ALWAYSSTAT | DT_BLK):
			case (F_ALWAYSSTAT | DT_REG):
			case (F_ALWAYSSTAT | DT_LNK):
			case (F_ALWAYSSTAT | DT_SOCK):
			case (F_ALWAYSSTAT | DT_WHT):
				/* Build a file name for fts_stat to stat. */
				if (ISSET(FTS_NOCHDIR)) {
					p->fts_accpath = p->fts_path;
					memmove(cp, p->fts_name, p->fts_namelen + 1);
				} else
					p->fts_accpath = p->fts_name;
				/* Stat it. */
				p->fts_info = fts_stat(sp, p, 0);
				break;
			default:
				/* No stat necessary */
				p->fts_accpath =
				    ISSET(FTS_NOCHDIR) ? p->fts_path : p->fts_name;
				p->fts_info = FTS_NSOK;
				break;
			}
		}

		/* We walk in directory order so "ls -f" doesn't get upset. */
		p->fts_link = NULL;
		if (head == NULL)
			head = tail = p;
		else {
			tail->fts_link = p;
			tail = p;
		}
		++nitems;
	}
	(void)closedir(dirp);

	/*
	 * If had to realloc the path, adjust the addresses for the rest
	 * of the tree.
	 */
	if (adjaddr)
		fts_padjust(sp, adjaddr);

	/*
	 * If not changing directories, reset the path back to original
	 * state.
	 */
	if (ISSET(FTS_NOCHDIR)) {
		if (cp - 1 > sp->fts_path)
			--cp;
		*cp = '\0';
	}

	/*
	 * If descended after called from fts_children or after called from
	 * fts_read and nothing found, get back.  At the root level we use
	 * the saved fd; if one of fts_open()'s arguments is a relative path
	 * to an empty directory, we wind up here with no other way back.  If
	 * can't get back, we're done.
	 */
	if (descend && (type == BCHILD || !nitems) &&
	    (cur->fts_level == FTS_ROOTLEVEL ?
	    FCHDIR(sp, sp->fts_rfd) : CHDIR(sp, ".."))) {
		cur->fts_info = FTS_ERR;
		SET(FTS_STOP);
		return (NULL);
	}

	/* If didn't find anything, return NULL. */
	if (!nitems) {
		if (type == BREAD)
			cur->fts_info = FTS_DP;
		return (NULL);
	}

	/* Sort the entries. */
	if (sp->fts_compar && nitems > 1)
		head = fts_sort(sp, head, nitems);
	return (head);
}

static u_short
fts_stat(sp, p, follow)
	FTS *sp;
	register FTSENT *p;
	int follow;
{
	register FTSENT *t;
	register dev_t dev;
	register ino_t ino;
	struct stat *sbp, sb;
	int saved_errno;

	/* If user needs stat info, stat buffer already allocated. */
	sbp = ISSET(FTS_NOSTAT) ? &sb : p->fts_statp;

#ifdef FTS_WHITEOUT
	/* check for whiteout */
	if (p->fts_flags & FTS_ISW) {
		if (sbp != &sb) {
			memset(sbp, '\0', sizeof (*sbp));
			sbp->st_mode = S_IFWHT;
		}
		return (FTS_W);
	}
#endif
	
	/*
	 * If doing a logical walk, or application requested FTS_FOLLOW, do
	 * a stat(2).  If that fails, check for a non-existent symlink.  If
	 * fail, set the errno from the stat call.
	 */
	if (ISSET(FTS_LOGICAL) || follow) {
		if (stat(p->fts_accpath, sbp)) {
			saved_errno = errno;
			if (!lstat(p->fts_accpath, sbp)) {
				if (saved_errno == ELOOP)
					p->fts_errno = ELOOP;
				errno = 0;
				return (FTS_SLNONE);
			} 
			p->fts_errno = saved_errno;
			goto err;
		}
		/*
		 * For FTS_COMFOLLOWDIR, drop back to lstat unless we have
		 * a directory
		 */
		if (follow == -1 && !S_ISDIR(sbp->st_mode)) {
			if (lstat(p->fts_accpath, sbp)) {
				p->fts_errno = errno;
				goto err;
			}
		}
	} else if (lstat(p->fts_accpath, sbp)) {
		p->fts_errno = errno;
err:		memset(sbp, 0, sizeof(struct stat));
		return (FTS_NS);
	}

	if (S_ISDIR(sbp->st_mode)) {
		/*
		 * Set the device/inode.  Used to find cycles and check for
		 * crossing mount points.  Also remember the link count, used
		 * in fts_build to limit the number of stat calls.  It is
		 * understood that these fields are only referenced if fts_info
		 * is set to FTS_D.
		 */
		dev = p->fts_dev = sbp->st_dev;
		ino = p->fts_ino = sbp->st_ino;
		p->fts_nlink = sbp->st_nlink;

		if (ISDOT(p->fts_name))
			return (FTS_DOT);

		/*
		 * Cycle detection is done by brute force when the directory
		 * is first encountered.  If the tree gets deep enough or the
		 * number of symbolic links to directories is high enough,
		 * something faster might be worthwhile.
		 */
		for (t = p->fts_parent;
		    t->fts_level >= FTS_ROOTLEVEL; t = t->fts_parent)
			if (ino == t->fts_ino && dev == t->fts_dev) {
				p->fts_cycle = t;
				return (FTS_DC);
			}
		return (FTS_D);
	}
	if (S_ISLNK(sbp->st_mode))
		return (FTS_SL);
	if (S_ISREG(sbp->st_mode))
		return (FTS_F);
	return (FTS_DEFAULT);
}

static FTSENT *
fts_sort(sp, head, nitems)
	FTS *sp;
	FTSENT *head;
	register int nitems;
{
	register FTSENT **ap, *p;

	/*
	 * Construct an array of pointers to the structures and call qsort(3).
	 * Reassemble the array in the order returned by qsort.  If unable to
	 * sort for memory reasons, return the directory entries in their
	 * current order.  Allocate enough space for the current needs plus
	 * 40 so don't realloc one entry at a time.
	 */
	if (nitems > sp->fts_nitems) {
		sp->fts_nitems = nitems + 40;
		if ((sp->fts_array = realloc(sp->fts_array,
		    (size_t)(sp->fts_nitems * sizeof(FTSENT *)))) == NULL) {
			sp->fts_nitems = 0;
			return (head);
		}
	}
	for (ap = sp->fts_array, p = head; p; p = p->fts_link)
		*ap++ = p;
#ifdef __BLOCKS__
	if (ISSET(FTS_BLOCK_COMPAR))
		qsort_b((void *)sp->fts_array, nitems, sizeof(FTSENT *), (int (^)(const void *, const void *))sp->fts_compar_b);
	else
#endif /* __BLOCKS__ */
		qsort((void *)sp->fts_array, nitems, sizeof(FTSENT *), sp->fts_compar);
	for (head = *(ap = sp->fts_array); --nitems; ++ap)
		ap[0]->fts_link = ap[1];
	ap[0]->fts_link = NULL;
	return (head);
}

static FTSENT *
fts_alloc(sp, name, namelen)
	FTS *sp;
	char *name;
	register int namelen;
{
	register FTSENT *p;
	size_t len;

	/*
	 * The file name is a variable length array and no stat structure is
	 * necessary if the user has set the nostat bit.  Allocate the FTSENT
	 * structure, the file name and the stat structure in one chunk, but
	 * be careful that the stat structure is reasonably aligned.  Since the
	 * fts_name field is declared to be of size 1, the fts_name pointer is
	 * namelen + 2 before the first possible address of the stat structure.
	 */
	len = sizeof(FTSENT) + namelen;
	if (!ISSET(FTS_NOSTAT))
		len += sizeof(struct stat) + ALIGNBYTES;
	if ((p = malloc(len)) == NULL)
		return (NULL);

	/* Copy the name plus the trailing NULL. */
	memmove(p->fts_name, name, namelen + 1);

	if (!ISSET(FTS_NOSTAT))
		p->fts_statp = (struct stat *)ALIGN(p->fts_name + namelen + 2);
	p->fts_namelen = namelen;
	p->fts_path = sp->fts_path;
	p->fts_errno = 0;
	p->fts_flags = 0;
	p->fts_instr = FTS_NOINSTR;
	p->fts_number = 0;
	p->fts_pointer = NULL;
	return (p);
}

static void
fts_lfree(head)
	register FTSENT *head;
{
	register FTSENT *p;

	/* Free a linked list of structures. */
	while (p = head) {
		head = head->fts_link;
		free(p);
	}
}

/*
 * Allow essentially unlimited paths; find, rm, ls should all work on any tree.
 * Most systems will allow creation of paths much longer than MAXPATHLEN, even
 * though the kernel won't resolve them.  Add the size (not just what's needed)
 * plus 256 bytes so don't realloc the path 2 bytes at a time. 
 */
static int
fts_palloc(sp, more)
	FTS *sp;
	size_t more;
{
	sp->fts_pathlen += more + 256;
	sp->fts_path = realloc(sp->fts_path, (size_t)sp->fts_pathlen);
	return (sp->fts_path == NULL);
}

/*
 * When the path is realloc'd, have to fix all of the pointers in structures
 * already returned.
 */
static void
fts_padjust(sp, addr)
	FTS *sp;
	void *addr;
{
	FTSENT *p;

#define	ADJUST(p) {							\
	(p)->fts_accpath =						\
	    (char *)addr + ((p)->fts_accpath - (p)->fts_path);		\
	(p)->fts_path = addr;						\
}
	/* Adjust the current set of children. */
	for (p = sp->fts_child; p; p = p->fts_link)
		ADJUST(p);

	/* Adjust the rest of the tree. */
	for (p = sp->fts_cur; p->fts_level >= FTS_ROOTLEVEL;) {
		ADJUST(p);
		p = p->fts_link ? p->fts_link : p->fts_parent;
	}
}

static size_t
fts_maxarglen(argv)
	char * const *argv;
{
	size_t len, max;

	for (max = 0; *argv; ++argv)
		if ((len = strlen(*argv)) > max)
			max = len;
	return (max);
}

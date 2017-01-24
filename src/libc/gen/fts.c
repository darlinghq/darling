/*
 * Copyright (c) 1999, 2000, 2003, 2005, 2008, 2012, 2016 Apple Inc. All rights reserved.
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

/*	$OpenBSD: fts.c,v 1.51 2015/09/12 13:32:24 guenther Exp $	*/

#include <sys/param.h>
#include <sys/stat.h>

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/vnode.h>
#include <sys/attr.h>

#ifdef __BLOCKS__
#include <Block.h>
#endif /* __BLOCKS__ */
#include <malloc_private.h>

static FTSENT	*fts_alloc(FTS *, char *, int);
static FTSENT	*fts_build(FTS *, int);
static void	 fts_lfree(FTSENT *);
static void	 fts_load(FTS *, FTSENT *);
static size_t	 fts_maxarglen(char * const *);
static void	 fts_padjust(FTS *, FTSENT *);
static int	 fts_palloc(FTS *, size_t);
static FTSENT	*fts_sort(FTS *, FTSENT *, int);
static u_short	 fts_stat(FTS *, FTSENT *, int, int);
static u_short   fts_stat2(FTS *, FTSENT *, int, int, struct stat *);
static int	 fts_safe_changedir(FTS *, FTSENT *, int, char *);

#define	ISDOT(a)	(a[0] == '.' && (!a[1] || (a[1] == '.' && !a[2])))

#define	CLR(opt)	(sp->fts_options &= ~(opt))
#define	ISSET(opt)	(sp->fts_options & (opt))
#define	SET(opt)	(sp->fts_options |= (opt))

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
#define F_D_TYPE	(4 << F_SHIFT)	/* only stat directories but use d_type */
#define F_D_TYPESYM	(5 << F_SHIFT)	/* only stat directories and symlinks but use d_type */

static FTS *
__fts_open(char * const *argv, FTS *sp)
{
	FTSENT *p, *root;
	int nitems;
	FTSENT *parent, *tmp;
	int len;

	/* Logical walks turn on NOCHDIR; symbolic links are too hard. */
	if (ISSET(FTS_LOGICAL))
		SET(FTS_NOCHDIR);

	/*
	 * Start out with 1K of path space, and enough, in any case,
	 * to hold the user's paths.
	 */
	if (fts_palloc(sp, MAX(fts_maxarglen(argv), PATH_MAX)))
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

		if ((p = fts_alloc(sp, *argv, len)) == NULL)
			goto mem3;
		p->fts_level = FTS_ROOTLEVEL;
		p->fts_parent = parent;
		p->fts_accpath = p->fts_name;
		p->fts_info = fts_stat(sp, p, ISSET(FTS_COMFOLLOWDIR) ? -1 : ISSET(FTS_COMFOLLOW), -1);

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
	 * If using chdir(2), grab a file descriptor pointing to dot to ensure
	 * that we can get back here; this could be avoided for some paths,
	 * but almost certainly not worth the effort.  Slashes, symbolic links,
	 * and ".." are all fairly nasty problems.  Note, if we can't get the
	 * descriptor we run anyway, just more slowly.
	 */
	if (!ISSET(FTS_NOCHDIR) &&
	    (sp->fts_rfd = open(".", O_RDONLY | O_CLOEXEC)) < 0)
		SET(FTS_NOCHDIR);

	if (nitems == 0)
		free(parent);

	return (sp);

mem3:	fts_lfree(root);
	free(parent);
mem2:	free(sp->fts_path);
mem1:	free(sp);
	return (NULL);
}

FTS *
fts_open(char * const *argv, int options, int (*compar)())
{
	FTS *sp;

	/* Options check. */
	if (options & ~FTS_OPTIONMASK) {
		errno = EINVAL;
		return (NULL);
	}
	if (options & FTS_NOSTAT_TYPE) options |= FTS_NOSTAT;

	/* Allocate/initialize the stream */
	if ((sp = calloc(1, sizeof(FTS))) == NULL)
		return (NULL);
	sp->fts_compar = compar;
	sp->fts_options = options;

	return __fts_open(argv, sp);
}

#ifdef __BLOCKS__
FTS *
fts_open_b(char * const *argv, int options, int (^compar)(const FTSENT **, const FTSENT **))
{
	FTS *sp;

	/* Options check. */
	if (options & ~FTS_OPTIONMASK) {
		errno = EINVAL;
		return (NULL);
	}
	if (options & FTS_NOSTAT_TYPE) options |= FTS_NOSTAT;

	/* Allocate/initialize the stream */
	if ((sp = calloc(1, sizeof(FTS))) == NULL)
		return (NULL);
	sp->fts_compar_b = (int (^)())Block_copy(compar);
	sp->fts_options = options | FTS_BLOCK_COMPAR;

	return __fts_open(argv, sp);
}
#endif /* __BLOCKS__ */

static void
fts_load(FTS *sp, FTSENT *p)
{
	int len;
	char *cp;

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
fts_close(FTS *sp)
{
	FTSENT *freep, *p;
	int rfd, error = 0;

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

	/* Stash the original directory fd if needed. */
	rfd = ISSET(FTS_NOCHDIR) ? -1 : sp->fts_rfd;

	/* Free up child linked list, sort array, path buffer, stream ptr.*/
	if (sp->fts_child){
		fts_lfree(sp->fts_child);
	}
	free(sp->fts_array); sp->fts_array = NULL;
	free(sp->fts_path); sp->fts_path = NULL;

#ifdef __BLOCKS__
	/* Free up any block pointer. */
	if (ISSET(FTS_BLOCK_COMPAR) && sp->fts_compar_b != NULL)
		Block_release(sp->fts_compar_b);
#endif /* __BLOCKS__ */

	/* Free up the stream pointer. */
	free(sp);

	/* Return to original directory, checking for error. */
	if (rfd != -1) {
		int saved_errno = errno;
		if (fchdir(rfd) != 0){
			error = -1;
			saved_errno = errno;
		}
		if (close(rfd) != 0){
			error = -1;
			saved_errno = errno;
		}
		errno = saved_errno;
	}

	return (error);
}

/*
 * Special case a root of "/" so that slashes aren't appended which would
 * cause paths to be written as "//foo".
 */
#define	NAPPEND(p)							\
	(p->fts_level == FTS_ROOTLEVEL && p->fts_pathlen == 1 &&	\
	    p->fts_path[0] == '/' ? 0 : p->fts_pathlen)

FTSENT *
fts_read(FTS *sp)
{
	FTSENT *p, *tmp;
	int instr;
	char *t;
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
		p->fts_info = fts_stat(sp, p, 0, -1);
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
		p->fts_info = fts_stat(sp, p, 1, -1);
		if (p->fts_info == FTS_D && !ISSET(FTS_NOCHDIR)) {
			if ((p->fts_symfd =
			    open(".", O_RDONLY | O_CLOEXEC)) < 0) {
				p->fts_errno = errno;
				p->fts_info = FTS_ERR;
			} else {
				p->fts_flags |= FTS_SYMFOLLOW;
			}
		}
		return (p);
	}

	/* Directory in pre-order. */
	if (p->fts_info == FTS_D) {
		/* If skipped or crossed mount point, do post-order visit. */
		if (instr == FTS_SKIP ||
		    (ISSET(FTS_XDEV) && p->fts_dev != sp->fts_dev)) {
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
		if (sp->fts_child && ISSET(FTS_NAMEONLY)) {
			CLR(FTS_NAMEONLY);
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
			if (fts_safe_changedir(sp, p, -1, p->fts_accpath)) {
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
	if ((p = p->fts_link)) {
		free(tmp);

		/*
		 * If reached the top, return to the original directory (or
		 * the root of the tree), and load the paths for the next root.
		 */
		if (p->fts_level == FTS_ROOTLEVEL) {
			if (FCHDIR(sp, sp->fts_rfd)) {
				SET(FTS_STOP);
				return (NULL);
			}
			fts_load(sp, p);
			return (sp->fts_cur = p);
		}

		/*
		 * User may have called fts_set on the node.  If skipped,
		 * ignore.  If followed, get a file descriptor so we can
		 * get back if necessary.
		 */
		if (p->fts_instr == FTS_SKIP) {
			goto next;
		}
		if (p->fts_instr == FTS_FOLLOW) {
			p->fts_info = fts_stat(sp, p, 1, -1);
			if (p->fts_info == FTS_D && !ISSET(FTS_NOCHDIR)) {
				if ((p->fts_symfd =
				    open(".", O_RDONLY | O_CLOEXEC, 0)) < 0) {
					p->fts_errno = errno;
					p->fts_info = FTS_ERR;
				} else {
					p->fts_flags |= FTS_SYMFOLLOW;
				}
			}
			p->fts_instr = FTS_NOINSTR;
		}

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

	/* NUL terminate the pathname. */
	sp->fts_path[p->fts_pathlen] = '\0';

	/*
	 * Return to the parent directory.  If at a root node or came through
	 * a symlink, go back through the file descriptor.  Otherwise, cd up
	 * one directory.
	 */
	if (p->fts_level == FTS_ROOTLEVEL) {
		if (FCHDIR(sp, sp->fts_rfd)) {
			SET(FTS_STOP);
			sp->fts_cur = p;
			return (NULL);
		}
	} else if (p->fts_flags & FTS_SYMFOLLOW) {
		if (FCHDIR(sp, p->fts_symfd)) {
			saved_errno = errno;
			(void)close(p->fts_symfd);
			errno = saved_errno;
			SET(FTS_STOP);
			sp->fts_cur = p;
			return (NULL);
		}
		(void)close(p->fts_symfd);
	} else if (!(p->fts_flags & FTS_DONTCHDIR) &&
	    fts_safe_changedir(sp, p->fts_parent, -1, "..")) {
		SET(FTS_STOP);
		sp->fts_cur = p;
		return (NULL);
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
fts_set(FTS __unused *sp, FTSENT *p, int instr)
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
fts_children(FTS *sp, int instr)
{
	FTSENT *p;
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
		SET(FTS_NAMEONLY);
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

	if ((fd = open(".", O_RDONLY | O_CLOEXEC, 0)) < 0)
		return (NULL);
	sp->fts_child = fts_build(sp, instr);
	if (fchdir(fd)) {
		(void)close(fd);
		return (NULL);
	}
	(void)close(fd);
	return (sp->fts_child);
}

typedef struct __attribute__((packed)) {
	uint32_t length;

	/* common attributes */
	attribute_set_t attrset;
	attrreference_t name;
	dev_t st_dev;
	fsobj_type_t objtype;
	struct timespec st_birthtimespec;
	struct timespec st_mtimespec;
	struct timespec st_ctimespec;
	struct timespec st_atimespec;
	uid_t st_uid;
	gid_t st_gid;
	uint32_t accessmask;
	uint32_t st_flags;
	uint64_t st_ino;

	/* non-directory attributes */
	uint32_t st_nlink;
	off_t allocsize;
	uint32_t st_blksize;
	uint32_t st_rdev;
	off_t st_size;
} attrListAttributes;

typedef struct __attribute__((packed)) {
	uint32_t length;

	/* common attributes */
	attribute_set_t attrset;
	attrreference_t name;
	dev_t st_dev;
	fsobj_type_t objtype;
	uint64_t st_ino;

	/* non-directory attributes */
	uint32_t st_nlink;
	uint32_t st_rdev;
} attrListAttributes_nostat;

#define ATTR_BUF_SIZE (32*1024)

typedef struct {
	DIR *dirp;

	struct attrlist requested_attrs;
	void *attrbuf;
	union {
		attrListAttributes *curattr;
		attrListAttributes_nostat *curattr_nostat;
	};
	int dirfd;
	bool done;
	bool nostat;
	bool needs_dot;
	bool needs_dotdot;

	int entry_count;
	int cur_entry;
} dir_handle;

typedef struct {
	char *d_name;
	size_t d_namlen;
	struct stat sb;
	int d_type;
	bool stat_valid;
} dir_entry;

static bool
advance_directory(dir_handle *handle)
{
	if (handle->done) return true;

	assert(handle->dirfd != -1);
	handle->entry_count = getattrlistbulk(handle->dirfd, &handle->requested_attrs,
			handle->attrbuf, ATTR_BUF_SIZE, FSOPT_PACK_INVAL_ATTRS);
	if (handle->entry_count == -1) {
		goto error;
	} else if (handle->entry_count == 0) {
		/* No more entries. */
		handle->done = true;
	}
	handle->cur_entry = 0;
	handle->curattr = handle->attrbuf;
	return true;

error: {
		int saved_errno = errno;
		close(handle->dirfd);
		handle->dirfd = -1;
		errno = saved_errno;
		return false;
	}
}

static bool
open_directory(FTS *sp, dir_handle *handle, const char *path)
{
	memset(handle, 0, sizeof(*handle));

	handle->nostat = ISSET(FTS_NOSTAT);
	handle->needs_dot = handle->needs_dotdot = ISSET(FTS_SEEDOT);

	handle->dirfd = open(path, O_RDONLY | O_NONBLOCK | O_DIRECTORY | O_CLOEXEC);
	if (handle->dirfd == -1) goto fallback;

	handle->attrbuf = malloc(ATTR_BUF_SIZE);
	if (!handle->attrbuf) goto fallback;

	handle->requested_attrs.bitmapcount = ATTR_BIT_MAP_COUNT;
	if (!handle->nostat) {
		handle->requested_attrs.commonattr =
				ATTR_CMN_RETURNED_ATTRS|
				ATTR_CMN_NAME|ATTR_CMN_DEVID|ATTR_CMN_OBJTYPE|
				ATTR_CMN_CRTIME|ATTR_CMN_MODTIME|ATTR_CMN_CHGTIME|ATTR_CMN_ACCTIME|
				ATTR_CMN_OWNERID|ATTR_CMN_GRPID|ATTR_CMN_ACCESSMASK|ATTR_CMN_FLAGS|
				ATTR_CMN_FILEID;
		handle->requested_attrs.fileattr = ATTR_FILE_LINKCOUNT|ATTR_FILE_ALLOCSIZE|
				ATTR_FILE_IOBLOCKSIZE|ATTR_FILE_DEVTYPE|ATTR_FILE_DATALENGTH;
	} else {
		handle->requested_attrs.commonattr = ATTR_CMN_RETURNED_ATTRS|
				ATTR_CMN_NAME|ATTR_CMN_DEVID|ATTR_CMN_OBJTYPE|
				ATTR_CMN_FILEID;
		handle->requested_attrs.fileattr = ATTR_FILE_LINKCOUNT;
	}

	if (advance_directory(handle)) {
		/*
		 * We successfully read the first attribute buffer,
		 * so we'll use getdirentriesattr/getattrlistbulk.
		 */
		return true;
	}

fallback:
	if (handle->dirfd != -1) close(handle->dirfd);
	handle->dirfd = -1;
	free(handle->attrbuf);
	handle->attrbuf = NULL;
	handle->dirp = opendir(path);
	return (handle->dirp != NULL);
}

static bool
read_dirent(dir_handle *handle, dir_entry *entry)
{
	if (handle->dirp) {
		struct dirent *de = readdir(handle->dirp);
		if (de == NULL) return false;
		entry->d_name = de->d_name;
		entry->d_namlen = de->d_namlen;
		entry->d_type = de->d_type;
		entry->stat_valid = false;
		return true;
	}

	/*
	 * There are three states that our dir_handle can be in:
	 *   - real getattrlistbulk use (entry_count >= 0) - the rest of this function
	 *   - fallback - handled above with dirp
	 *   - closed fallback (dirp == NULL) - handled with this check
	 */
	if (handle->dirfd == -1) {
		return false;
	}

	if (handle->needs_dot) {
		handle->needs_dot = false;
		entry->d_name = ".";
		entry->d_namlen = strlen(".");
		entry->d_type = DT_DIR;
		entry->stat_valid = false;
		return true;
	} else if (handle->needs_dotdot) {
		handle->needs_dotdot = false;
		entry->d_name = "..";
		entry->d_namlen = strlen("..");
		entry->d_type = DT_DIR;
		entry->stat_valid = false;
		return true;
	}

	if (handle->cur_entry == handle->entry_count) {
		if (handle->done) return false;                /* Already done with the directory. */
		if (!advance_directory(handle)) return false;  /* Reading the next buffer failed. */
		if (handle->done) return false;                /* We're now done with the directory. */
	}

	bzero(entry, sizeof(*entry));

	attrListAttributes *curattr_stat = NULL;
	attrListAttributes_nostat *curattr_nostat = NULL;
	if (!handle->nostat) {
		curattr_stat = handle->curattr;
		handle->cur_entry++;
		handle->curattr = (attrListAttributes*)(((char*)curattr_stat) + curattr_stat->length);

		entry->d_name = ((char*)&curattr_stat->name) + curattr_stat->name.attr_dataoffset;
		/* attr_length includes the null terminator, but readdir's d_namlen doesn't. */
		entry->d_namlen = curattr_stat->name.attr_length - 1;
	} else {
		curattr_nostat = handle->curattr_nostat;
		handle->cur_entry++;
		handle->curattr_nostat = (attrListAttributes_nostat*)(((char*)curattr_nostat) + curattr_nostat->length);

		entry->d_name = ((char*)&curattr_nostat->name) + curattr_nostat->name.attr_dataoffset;
		/* attr_length includes the null terminator, but readdir's d_namlen doesn't. */
		entry->d_namlen = curattr_nostat->name.attr_length - 1;
	}

	int stat_type = 0;

	switch (handle->nostat ? curattr_nostat->objtype : curattr_stat->objtype) {
	case VREG:
		entry->d_type = DT_REG;
		stat_type = S_IFREG;
		break;
	case VDIR:
		entry->d_type = DT_DIR;
		/* Force a stat call so we don't have to guess on st_size, st_blocks, etc. */
		// stat_type = S_IFDIR;
		break;
	case VBLK:
		entry->d_type = DT_BLK;
		stat_type = S_IFBLK;
		break;
	case VCHR:
		entry->d_type = DT_CHR;
		stat_type = S_IFCHR;
		break;
	case VLNK:
		entry->d_type = DT_LNK;
		stat_type = S_IFLNK;
		break;
	case VSOCK:
		entry->d_type = DT_SOCK;
		stat_type = S_IFSOCK;
		break;
	case VFIFO:
		entry->d_type = DT_FIFO;
		stat_type = S_IFIFO;
		break;
	default:
		entry->d_type = DT_UNKNOWN;
		break;
	}

	if (!handle->nostat && stat_type) {
		entry->stat_valid = true;

		/*
		 * Make sure we got all the attributes we need to fill out a stat structure.
		 */

		attrgroup_t requiredCommon = handle->requested_attrs.commonattr;
		attrgroup_t requiredFile = handle->requested_attrs.fileattr;

		if ((entry->d_type != DT_BLK) && (entry->d_type != DT_CHR)) {
			/* It's okay for ATTR_FILE_DEVTYPE to be missing if the entry isn't a block or character device. */
			curattr_stat->st_rdev = 0;
			requiredFile &= ~ATTR_FILE_DEVTYPE;
		}

		if ((curattr_stat->attrset.commonattr & requiredCommon) != requiredCommon ||
				(curattr_stat->attrset.fileattr & requiredFile) != requiredFile) {
			/* Some of our required attributes are missing. */
			entry->stat_valid = false;
		}
	} else {
		entry->stat_valid = false;
	}

	if (entry->stat_valid) {

#define COPY_FIELD(fld) entry->sb.fld = curattr_stat->fld
		COPY_FIELD(st_dev);
		/* COPY_FIELD(st_mode);      Handled below. */
		COPY_FIELD(st_nlink);
		COPY_FIELD(st_ino);
		COPY_FIELD(st_uid);
		COPY_FIELD(st_gid);
		COPY_FIELD(st_rdev);
		COPY_FIELD(st_atimespec);
		COPY_FIELD(st_mtimespec);
		COPY_FIELD(st_ctimespec);
#if __DARWIN_64_BIT_INO_T
		COPY_FIELD(st_birthtimespec);
#endif /* __DARWIN_64_BIT_INO_T */
		COPY_FIELD(st_size);
		/* COPY_FIELD(st_blocks);    Handled below. */
		COPY_FIELD(st_blksize);
		COPY_FIELD(st_flags);
#undef COPY_FIELD

		/* We have to handle some fields specially. */
		entry->sb.st_mode = (curattr_stat->accessmask & ~S_IFMT) | stat_type;
		entry->sb.st_blocks = howmany(curattr_stat->allocsize, 512); /* Matches vn_stat implementation in xnu. */
	}
	return true;
}

static int
dir_fd(dir_handle *handle)
{
	return handle->dirp ? dirfd(handle->dirp) : handle->dirfd;
}

static void
close_directory(dir_handle *handle)
{
	if (handle->dirp) {
		closedir(handle->dirp);
		handle->dirp = NULL;
	}
	if (handle->dirfd != -1) {
		close(handle->dirfd);
		handle->dirfd = -1;
	}
	free(handle->attrbuf);
	handle->attrbuf = NULL;
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
fts_build(FTS *sp, int type)
{
	dir_entry de = { 0 };
	dir_entry *dp = &de;
	FTSENT *p, *head;
	int nitems;
	FTSENT *cur, *tail;
	dir_handle dirp;
	void *oldaddr;
	int len, maxlen;
	int cderrno, descend, level, dostat, doadjust;
	int saved_errno;
	char *cp;

	/* Set current node pointer. */
	cur = sp->fts_cur;

	/*
	 * Open the directory for reading.  If this fails, we're done.
	 * If being called from fts_read, set the fts_info field.
	 */
	if (!open_directory(sp, &dirp, cur->fts_accpath)) {
		if (type == BREAD) {
			cur->fts_info = FTS_DNR;
			cur->fts_errno = errno;
		}
		return (NULL);
	}

	if (type == BNAMES)
		dostat = F_NOSTAT;
	else if (ISSET(FTS_NOSTAT_TYPE))
		dostat = ISSET(FTS_PHYSICAL) ? F_D_TYPE : F_D_TYPESYM;
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
	if (dostat || type == BREAD) {
		if (fts_safe_changedir(sp, cur, dir_fd(&dirp), NULL)) {
			if (dostat && type == BREAD) {
				cur->fts_errno = errno;
			}
			cur->fts_flags |= FTS_DONTCHDIR;
			descend = 0;
			cderrno = errno;
			close_directory(&dirp);
		} else {
			descend = 1;
		}
	} else {
		descend = 0;
	}

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
	len = NAPPEND(cur);
	if (ISSET(FTS_NOCHDIR)) {
		cp = sp->fts_path + len;
		*cp++ = '/';
	}
	len++;
	maxlen = sp->fts_pathlen - len;

	/*
	 * fts_level is signed so we must prevent it from wrapping
	 * around to FTS_ROOTLEVEL and FTS_ROOTPARENTLEVEL.
	 */
	level = cur->fts_level;
	if (level < FTS_MAXLEVEL)
	    level++;

	/* Read the directory, attaching each entry to the `link' pointer. */
	doadjust = 0;
	for (head = tail = NULL, nitems = 0; read_dirent(&dirp, dp) ; ) {
		if (!ISSET(FTS_SEEDOT) && ISDOT(dp->d_name))
			continue;

		if ((p = fts_alloc(sp, dp->d_name, (int)dp->d_namlen)) == NULL)
			goto mem1;
		if (dp->d_namlen >= (size_t)maxlen) {	/* include space for NUL */
			oldaddr = sp->fts_path;
			if (fts_palloc(sp, dp->d_namlen +len + 1)) {
				/*
				 * No more memory for path or structures.  Save
				 * errno, free up the current structure and the
				 * structures already allocated.
				 */
mem1:				saved_errno = errno;
				free(p);
				fts_lfree(head);
				close_directory(&dirp);
				cur->fts_info = FTS_ERR;
				SET(FTS_STOP);
				errno = saved_errno;
				return (NULL);
			}
			/* Did realloc() change the pointer? */
			if (oldaddr != sp->fts_path) {
				doadjust = 1;
				if (ISSET(FTS_NOCHDIR))
					cp = sp->fts_path + len;
			}
			maxlen = sp->fts_pathlen - len;
		}

		if (len + dp->d_namlen >= USHRT_MAX) {
			/*
			 * In an FTSENT, fts_pathlen is a u_short so it is
			 * possible to wraparound here.  If we do, free up
			 * the current structure and the structures already
			 * allocated, then error out with ENAMETOOLONG.
			 */
			free(p);
			fts_lfree(head);
			close_directory(&dirp);
			cur->fts_info = FTS_ERR;
			SET(FTS_STOP);
			errno = ENAMETOOLONG;
			return (NULL);
		}
		p->fts_level = level;
		p->fts_parent = sp->fts_cur;
		p->fts_pathlen = len + dp->d_namlen;

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
			case (F_D_TYPE | DT_UNKNOWN):
			case (F_D_TYPE | DT_DIR):
			case (F_D_TYPESYM | DT_UNKNOWN):
			case (F_D_TYPESYM | DT_DIR):
			case (F_D_TYPESYM | DT_LNK):
				/* Build a file name for fts_stat to stat. */
				if (ISSET(FTS_NOCHDIR)) {
					p->fts_accpath = p->fts_path;
					memmove(cp, p->fts_name, p->fts_namelen + 1);
					p->fts_info = fts_stat2(sp, p, 0, dir_fd(&dirp), dp->stat_valid ? &dp->sb : NULL);
				} else {
					p->fts_accpath = p->fts_name;
					p->fts_info = fts_stat2(sp, p, 0, -1, dp->stat_valid ? &dp->sb : NULL);
				}
				break;
			case (F_D_TYPE | DT_FIFO):
			case (F_D_TYPE | DT_CHR):
			case (F_D_TYPE | DT_BLK):
			case (F_D_TYPE | DT_SOCK):
			case (F_D_TYPESYM | DT_FIFO):
			case (F_D_TYPESYM | DT_CHR):
			case (F_D_TYPESYM | DT_BLK):
			case (F_D_TYPESYM | DT_SOCK):
				p->fts_info = FTS_DEFAULT;
				goto common_no_stat;
			case (F_D_TYPE | DT_REG):
			case (F_D_TYPESYM | DT_REG):
				p->fts_info = FTS_F;
				goto common_no_stat;
			case (F_D_TYPE | DT_LNK):
				p->fts_info = FTS_SL;
				goto common_no_stat;
			case (F_D_TYPE | DT_WHT):
			case (F_D_TYPESYM | DT_WHT):
				p->fts_info = FTS_W;
				goto common_no_stat;
			default:
				/* No stat necessary */
				p->fts_info = FTS_NSOK;
common_no_stat:
				p->fts_accpath =
				    ISSET(FTS_NOCHDIR) ? p->fts_path : p->fts_name;
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
	close_directory(&dirp);

	/*
	 * If realloc() changed the address of the path, adjust the
	 * addresses for the rest of the tree and the dir list.
	 */
	if (doadjust)
		fts_padjust(sp, head);

	/*
	 * If not changing directories, reset the path back to original
	 * state.
	 */
	if (ISSET(FTS_NOCHDIR)) {
		if (len == sp->fts_pathlen || nitems == 0)
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
	    (cur->fts_level == FTS_ROOTLEVEL ? FCHDIR(sp, sp->fts_rfd) :
	    fts_safe_changedir(sp, cur->fts_parent, -1, ".."))) {
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
fts_stat(FTS *sp, FTSENT *p, int follow, int dfd)
{
	return fts_stat2(sp, p, follow, dfd, NULL);
}

static u_short
fts_stat2(FTS *sp, FTSENT *p, int follow, int dfd, struct stat *insb)
{
	FTSENT *t;
	dev_t dev;
	ino_t ino;
	struct stat *sbp, sb;
	int saved_errno;
	const char *path;

	if (dfd == -1) {
		path = p->fts_accpath;
		dfd = AT_FDCWD;
	} else {
		path = p->fts_name;
	}

	/* If user needs stat info, stat buffer already allocated. */
	sbp = ISSET(FTS_NOSTAT) ? &sb : p->fts_statp;

	/*
	 * getattrlistbulk always returns information about the link, not its
	 * destination, so if we are doing a logical walk we need to detect links
	 * and do a stat anyway.
	 */
	if (follow) {
		insb = NULL;
	} else if (insb && ISSET(FTS_LOGICAL) && ((insb->st_mode & S_IFLNK) == S_IFLNK)) {
		insb = NULL;
	}

	if (insb) {
		memcpy(sbp, insb, sizeof(*insb));
	} else if (ISSET(FTS_LOGICAL) || follow) {
		/*
		 * If doing a logical walk, or application requested FTS_FOLLOW, do
		 * a stat(2).  If that fails, check for a non-existent symlink.  If
		 * fail, set the errno from the stat call.
		 */
		if (fstatat(dfd, path, sbp, 0)) {
			saved_errno = errno;
			if (!fstatat(dfd, path, sbp, AT_SYMLINK_NOFOLLOW)) {
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
			if (fstatat(dfd, path, sbp, AT_SYMLINK_NOFOLLOW)) {
				p->fts_errno = errno;
				goto err;
			}
		}
	} else if (fstatat(dfd, path, sbp, AT_SYMLINK_NOFOLLOW)) {
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
fts_sort(FTS *sp, FTSENT *head, int nitems)
{
	FTSENT **ap, *p;

	/*
	 * Construct an array of pointers to the structures and call qsort(3).
	 * Reassemble the array in the order returned by qsort.  If unable to
	 * sort for memory reasons, return the directory entries in their
	 * current order.  Allocate enough space for the current needs plus
	 * 40 so don't realloc one entry at a time.
	 */
	if (nitems > sp->fts_nitems) {
		struct _ftsent **a;

		sp->fts_nitems = nitems + 40;
		if ((a = reallocarray(sp->fts_array,
		    sp->fts_nitems, sizeof(FTSENT *))) == NULL) {
			free(sp->fts_array);
			sp->fts_array = NULL;
			sp->fts_nitems = 0;
			return (head);
		}
		sp->fts_array = a;
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
fts_alloc(FTS *sp, char *name, int namelen)
{
	FTSENT *p;
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
	if ((p = calloc(1, len)) == NULL)
		return (NULL);

	p->fts_path = sp->fts_path;
	p->fts_namelen = namelen;
	p->fts_instr = FTS_NOINSTR;
	if (!ISSET(FTS_NOSTAT))
		p->fts_statp = (struct stat *)ALIGN(p->fts_name + namelen + 2);
	memcpy(p->fts_name, name, namelen);

	return (p);
}

static void
fts_lfree(FTSENT *head)
{
	FTSENT *p;

	/* Free a linked list of structures. */
	while ((p = head)) {
		head = head->fts_link;
		free(p);
	}
}

/*
 * Allow essentially unlimited paths; find, rm, ls should all work on any tree.
 * Most systems will allow creation of paths much longer than PATH_MAX, even
 * though the kernel won't resolve them.  Add the size (not just what's needed)
 * plus 256 bytes so don't realloc the path 2 bytes at a time.
 */
static int
fts_palloc(FTS *sp, size_t more)
{
	char *p;

	sp->fts_pathlen += more + 256;
	/*
	 * Check for possible wraparound.  In an FTS, fts_pathlen is
	 * a signed int but in an FTSENT it is an unsigned short.
	 * We limit fts_pathlen to USHRT_MAX to be safe in both cases.
	 */
	if (sp->fts_pathlen < 0 || sp->fts_pathlen >= USHRT_MAX) {
		free(sp->fts_path);
		sp->fts_path = NULL;
		errno = ENAMETOOLONG;
		return (1);
	}
	p = realloc(sp->fts_path, sp->fts_pathlen);
	if (p == NULL) {
		free(sp->fts_path);
		sp->fts_path = NULL;
		return (1);
	}
	sp->fts_path = p;
	return (0);
}

/*
 * When the path is realloc'd, have to fix all of the pointers in structures
 * already returned.
 */
static void
fts_padjust(FTS *sp, FTSENT *head)
{
	FTSENT *p;
	char *addr = sp->fts_path;

#define	ADJUST(p) {							\
	if ((p)->fts_accpath != (p)->fts_name) {			\
		(p)->fts_accpath =					\
		    (char *)addr + ((p)->fts_accpath - (p)->fts_path);	\
	}								\
	(p)->fts_path = addr;						\
}
	/* Adjust the current set of children. */
	for (p = sp->fts_child; p; p = p->fts_link)
		ADJUST(p);

	/* Adjust the rest of the tree, including the current level. */
	for (p = head; p->fts_level >= FTS_ROOTLEVEL;) {
		ADJUST(p);
		p = p->fts_link ? p->fts_link : p->fts_parent;
	}
}

static size_t
fts_maxarglen(char * const *argv)
{
	size_t len, max;

	for (max = 0; *argv; ++argv)
		if ((len = strlen(*argv)) > max)
			max = len;
	return (max + 1);
}

/*
 * Change to dir specified by fd or p->fts_accpath without getting
 * tricked by someone changing the world out from underneath us.
 * Assumes p->fts_dev and p->fts_ino are filled in.
 */
static int
fts_safe_changedir(FTS *sp, FTSENT *p, int fd, char *path)
{
	int ret, oerrno, newfd;
	struct stat sb;

	newfd = fd;
	if (ISSET(FTS_NOCHDIR))
		return (0);
	if (fd < 0 && (newfd = open(path, O_RDONLY|O_DIRECTORY|O_CLOEXEC)) < 0)
		return (-1);
	if (fstat(newfd, &sb)) {
		ret = -1;
		goto bail;
	}
	if (p->fts_dev != sb.st_dev || p->fts_ino != sb.st_ino) {
		errno = ENOENT;		/* disinformation */
		ret = -1;
		goto bail;
	}
	ret = fchdir(newfd);
bail:
	oerrno = errno;
	if (fd < 0)
		(void)close(newfd);
	errno = oerrno;
	return (ret);
}

/*	$OpenBSD: nftw.c,v 1.2 2003/07/21 21:15:32 millert Exp $	*/

/*
 * Copyright (c) 2003 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char rcsid[] = "$OpenBSD: nftw.c,v 1.2 2003/07/21 21:15:32 millert Exp $";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fts.h>
#include <ftw.h>
#include <limits.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int
both_ftw(const char *path,
	int (*ofn)(const char *, const struct stat *, int),
	int (*nfn)(const char *, const struct stat *, int, struct FTW *),
	int nfds, int ftwflags)
{
	const char *paths[2];
	struct FTW ftw;
	FTSENT *cur;
	FTS *ftsp = NULL;
	int ftsflags, fnflag, error, postorder, sverrno;
	int cwd_fd = -1; /* cwd_fd != -1 means call chdir a lot */

#if __DARWIN_UNIX03
	/* Macro to skip the mount point itself in UNiX03 mode, in legcy
	  mode the mount point is returned, but we don't decend into it */
#define SKIP_MOUNT if ((ftwflags & FTW_MOUNT) \
      && cur->fts_statp->st_dev != path_stat.st_dev) { \
	continue; \
    }
#else
#define SKIP_MOUNT
#endif

	/* XXX - nfds is currently unused */
	if (nfds < 1 || nfds > OPEN_MAX) {
		errno = EINVAL;
		error = -1;
		goto done;
	}

	ftsflags = FTS_COMFOLLOW;
	if (!(ftwflags & FTW_CHDIR))
		ftsflags |= FTS_NOCHDIR;
	if (ftwflags & FTW_MOUNT)
		ftsflags |= FTS_XDEV;
	if (ftwflags & FTW_PHYS) {
		ftsflags |= FTS_PHYSICAL;
	} else {
		ftsflags |= FTS_LOGICAL;
	}
	postorder = (ftwflags & FTW_DEPTH) != 0;

	/* We have been requested to change directories, and fts doesn't
	  always do it (never for FTS_LOGICAL, and sometimes not for 
	  FTS_PHYSICAL) */
	if (ftwflags & FTW_CHDIR) {
	    cwd_fd = open(".", O_RDONLY, 0);
	    if (cwd_fd < 0) {
		error = -1;
		goto done;
	    }
	    /* Prevent problems if fts ever starts using chdir when passed
	      FTS_PHYSICAL */
	    ftsflags |= FTS_NOCHDIR;
	}

#if __DARWIN_UNIX03
	struct stat path_stat;

	/* UNIX03 requires us to return -1/errno=ELOOP if path
	  is a looping symlink; fts_open is succesful and fts_read
	  gives us FTS_NS which isn't very useful, in fact we get
	  pretty much the same behaviour for ENAMETOOLONG, ENOENT,
	  ENOTDIR, and EACCES */
	{
	    int rc = stat(path, &path_stat);
	    if (rc < 0 
	      && (errno == ELOOP || errno == ENAMETOOLONG || errno == ENOENT
	      || errno == ENOTDIR || errno == EACCES)) {
		    error = -1;
		    goto done;
	    }
	    if (rc >= 0 && nfn) {
		if (!S_ISDIR(path_stat.st_mode)) {
		    errno = ENOTDIR;
		    error = -1;
		    goto done;
		}
	    }
	}
#endif
	paths[0] = path;
	paths[1] = NULL;
	ftsp = fts_open((char * const *)paths, ftsflags, NULL);
	if (ftsp == NULL) {
	    error = -1;
	    goto done;
	}
	error = 0;
	while ((cur = fts_read(ftsp)) != NULL) {
		switch (cur->fts_info) {
		case FTS_D:
			if (postorder)
				continue;
			SKIP_MOUNT;
			/* we will get FTS_DNR next (this is not an issue for
			  FTS_DP, only FTS_D) */
			if (access(cur->fts_path, R_OK) != 0)
			    continue;
			fnflag = FTW_D;
			break;
		case FTS_DNR:
			fnflag = FTW_DNR;
			break;
		case FTS_DP:
			if (!postorder)
				continue;
			SKIP_MOUNT;
			fnflag = FTW_DP;
			break;
		case FTS_F:
		case FTS_DEFAULT:
			SKIP_MOUNT;
			fnflag = FTW_F;
			break;
		case FTS_NS:
		case FTS_NSOK:
			fnflag = FTW_NS;
			break;
		case FTS_SL:
			fnflag = FTW_SL;
			break;
		case FTS_SLNONE:
			fnflag = nfn ? FTW_SLN : FTW_SL;
#if __DARWIN_UNIX03
			{
			    /* The legacy behaviour did not signal an error 
			      on symlink loops unless they ended up causing
			      a directory cycle, but the Unix2003 standard
			      requires ELOOP to end ftw and nftw walks with
			      an error */
			    struct stat sb;
			    int rc = stat(cur->fts_path, &sb);
			    if (rc < 0 && errno == ELOOP) {
				error = -1;
				goto done;
			    }
			}
#endif
			break;
		case FTS_DC:
#if __DARWIN_UNIX03
			/* Unix03 says nftw should break cycles and not return
			  errors in non-physical mode (which is definitly what it
			  says ftw can't do) */
			if (nfn && !(ftwflags & FTW_PHYS)) {
				/* 4489297 - when FTW_DEPTH is set, skip
				   the link also */
				if (postorder)
				    continue;
				fnflag = FTW_D;
				break;
			}
#endif
			errno = ELOOP;
			/* FALLTHROUGH */
		default:
			error = -1;
			goto done;
		}

		if (cwd_fd >= 0) {
		    char *dir, *free_me = NULL;
		    if (fnflag == FTW_D) {
			dir = cur->fts_path;
		    } else {
			/* we could alloc just enough for the directory,
			  and use memmove -- but that is a little more
			  error prone, and not noticable in with all the
			  extra work... */
			dir = free_me = strdup(cur->fts_path);
			dir[cur->fts_pathlen - cur->fts_namelen] = '\0';
		    }
		    int rc = chdir(dir);
		    if (free_me) {
			free(free_me);
		    }
		    if (rc < 0) {
			if(cur->fts_pathlen == cur->fts_namelen &&
			   fnflag == FTW_DNR) {
			    /* If cwd_fd is our last FD, fts_read will give us FTS_DNR
			     * and fts_path == fts_name == "."
			     * This check results in the correct errno being returned.
			     */
			    errno = EMFILE;
			}
			error = -1;
			goto done;
		    }
		}
		if (nfn) {
		    ftw.base = cur->fts_pathlen - cur->fts_namelen;
		    ftw.level = cur->fts_level;
		    error = nfn(cur->fts_path, cur->fts_statp, fnflag, &ftw);
		} else {
		    error = ofn(cur->fts_path, cur->fts_statp, fnflag);
		}
		if (cwd_fd >= 0) {
		    if (fchdir(cwd_fd) < 0) {
			error = -1;
			goto done;
		    }
		}

		if (error != 0)
			break;
	}
done:
	sverrno = errno;
	if(ftsp != NULL)
		(void) fts_close(ftsp);
	if(cwd_fd >= 0)
		(void) close(cwd_fd);
	errno = sverrno;
	return (error);
}

int
ftw(const char *path, int (*fn)(const char *, const struct stat *, int),
    int nfds)
{
	/* The legacy implmentation didn't follow symlinks, but Unix03 
	  does - this was likely a bug in the legacy implemtation; JKH
	  thinks we ought change the legacy behaviour, and I agree; anyone
	  who doesn't should replace FTW_PHYS with
	  __DARWIN_UNIX03 ? 0 : FTW_PHYS */
	return both_ftw(path, fn, NULL, nfds, FTW_PHYS);
}

int
nftw(const char *path, int (*fn)(const char *, const struct stat *, int,
     struct FTW *), int nfds, int ftwflags)
{
	return both_ftw(path, NULL, fn, nfds, ftwflags);
}

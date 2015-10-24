/*
 * Copyright (c) 2003 Constantin S. Svintsoff <kostik@iclub.nsu.ru>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the authors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)realpath.c	8.1 (Berkeley) 2/16/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdlib/realpath.c,v 1.20 2003/05/28 08:23:01 fjoe Exp $");

#include "namespace.h"
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/mount.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/attr.h>
#include <sys/vnode.h>
#include "un-namespace.h"

struct attrs {
	u_int32_t len;
	attrreference_t name;
	dev_t dev;
	fsobj_type_t type;
	fsobj_id_t id;
	char buf[PATH_MAX];
};

#ifndef BUILDING_VARIANT
__private_extern__ struct attrlist _rp_alist = {
	ATTR_BIT_MAP_COUNT,
	0,
	ATTR_CMN_NAME | ATTR_CMN_DEVID | ATTR_CMN_OBJTYPE | ATTR_CMN_OBJID,
	0,
	0,
	0,
	0,
};
#else /* BUILDING_VARIANT */
__private_extern__ struct attrlist _rp_alist;
#endif /* BUILDING_VARIANT */

extern char * __private_getcwd(char *, size_t, int);

/*
 * char *realpath(const char *path, char resolved[PATH_MAX]);
 *
 * Find the real name of path, by removing all ".", ".." and symlink
 * components.  Returns (resolved) on success, or (NULL) on failure,
 * in which case the path which caused trouble is left in (resolved).
 */
char *
realpath(const char *path, char inresolved[PATH_MAX])
{
	struct attrs attrs;
	struct stat sb;
	char *p, *q, *s;
	size_t left_len, resolved_len, save_resolved_len;
	unsigned symlinks;
	int serrno, slen, useattrs, islink;
	char left[PATH_MAX], next_token[PATH_MAX], symlink[PATH_MAX];
	dev_t dev, lastdev;
	struct statfs sfs;
	static dev_t rootdev;
	static int rootdev_inited = 0;
	ino_t inode;
	char *resolved;

	if (path == NULL) {
		errno = EINVAL;
		return (NULL);
	}
#if __DARWIN_UNIX03
	if (*path == 0) {
		errno = ENOENT;
		return (NULL);
	}
#endif /* __DARWIN_UNIX03 */
	/*
	 * Extension to the standard; if inresolved == NULL, allocate memory
	 */
	if (!inresolved) {
	    if ((resolved = malloc(PATH_MAX)) == NULL) return (NULL);
	} else {
	    resolved = inresolved;
	}
	if (!rootdev_inited) {
		rootdev_inited = 1;
		if (stat("/", &sb) < 0) {
error_return:
			if (!inresolved) {
				int e = errno;
				free(resolved);
				errno = e;
			}
			return (NULL);
		}
		rootdev = sb.st_dev;
	}
	serrno = errno;
	symlinks = 0;
	if (path[0] == '/') {
		resolved[0] = '/';
		resolved[1] = '\0';
		if (path[1] == '\0') {
			return (resolved);
		}
		resolved_len = 1;
		left_len = strlcpy(left, path + 1, sizeof(left));
	} else {
#if !defined(VARIANT_DARWINEXTSN) && __DARWIN_UNIX03
		/* 4447159: don't use GETPATH, so this will fail if */
		/* if parent directories are not readable, as per POSIX */
		if (__private_getcwd(resolved, PATH_MAX, 0) == NULL)
#else /* VARIANT_DARWINEXTSN || !__DARWIN_UNIX03 */
		if (__private_getcwd(resolved, PATH_MAX, 1) == NULL)
#endif /* !VARIANT_DARWINEXTSN && __DARWIN_UNIX03 */
		{
			strlcpy(resolved, ".", PATH_MAX);
			goto error_return;
		}
		resolved_len = strlen(resolved);
		left_len = strlcpy(left, path, sizeof(left));
	}
	if (left_len >= sizeof(left) || resolved_len >= PATH_MAX) {
		errno = ENAMETOOLONG;
		goto error_return;
	}
	if (resolved_len > 1) {
		if (stat(resolved, &sb) < 0) {
			goto error_return;
		}
		lastdev = sb.st_dev;
	} else
		lastdev = rootdev;

	/*
	 * Iterate over path components in `left'.
	 */
	while (left_len != 0) {
		/*
		 * Extract the next path component and adjust `left'
		 * and its length.
		 */
		p = strchr(left, '/');
		s = p ? p : left + left_len;
		if (s - left >= sizeof(next_token)) {
			errno = ENAMETOOLONG;
			goto error_return;
		}
		memcpy(next_token, left, s - left);
		next_token[s - left] = '\0';
		left_len -= s - left;
		if (p != NULL)
			memmove(left, s + 1, left_len + 1);
		if (resolved[resolved_len - 1] != '/') {
			if (resolved_len + 1 >= PATH_MAX) {
				errno = ENAMETOOLONG;
				goto error_return;
			}
			resolved[resolved_len++] = '/';
			resolved[resolved_len] = '\0';
		}
		if (next_token[0] == '\0')
			continue;
		else if (strcmp(next_token, ".") == 0)
			continue;
		else if (strcmp(next_token, "..") == 0) {
			/*
			 * Strip the last path component except when we have
			 * single "/"
			 */
			if (resolved_len > 1) {
				resolved[resolved_len - 1] = '\0';
				q = strrchr(resolved, '/') + 1;
				*q = '\0';
				resolved_len = q - resolved;
			}
			continue;
		}

		/*
		 * Save resolved_len, so that we can later null out
		 * the the appended next_token, and replace with the
		 * real name (matters on case-insensitive filesystems).
		 */
		save_resolved_len = resolved_len;

		/*
		 * Append the next path component and lstat() it. If
		 * lstat() fails we still can return successfully if
		 * there are no more path components left.
		 */
		resolved_len = strlcat(resolved, next_token, PATH_MAX);
		if (resolved_len >= PATH_MAX) {
			errno = ENAMETOOLONG;
			goto error_return;
		}
		if (getattrlist(resolved, &_rp_alist, &attrs, sizeof(attrs), FSOPT_NOFOLLOW) == 0) {
			useattrs = 1;
			islink = (attrs.type == VLNK);
			dev = attrs.dev;
			inode = attrs.id.fid_objno;
		} else if (errno == ENOTSUP || errno == EINVAL) {
			if ((useattrs = lstat(resolved, &sb)) == 0) {
				islink = S_ISLNK(sb.st_mode);
				dev = sb.st_dev;
				inode = sb.st_ino;
			}
		} else
			useattrs = -1;
		if (useattrs < 0) {
#if !__DARWIN_UNIX03
			if (errno == ENOENT && p == NULL) {
				errno = serrno;
				return (resolved);
			}
#endif /* !__DARWIN_UNIX03 */
			goto error_return;
		}
		if (dev != lastdev) {
			/*
			 * We have crossed a mountpoint.  For volumes like UDF
			 * the getattrlist name may not match the actual
			 * mountpoint, so we just copy the mountpoint directly.
			 * (3703138).  However, the mountpoint may not be
			 * accessible, as when chroot-ed, so check first.
			 * There may be a file on the chroot-ed volume with
			 * the same name as the mountpoint, so compare device
			 * and inode numbers.
			 */
			lastdev = dev;
			if (statfs(resolved, &sfs) == 0 && lstat(sfs.f_mntonname, &sb) == 0 && dev == sb.st_dev && inode == sb.st_ino) {
				/*
				 * However, it's possible that the mountpoint
				 * path matches, even though it isn't the real
				 * path in the chroot-ed environment, so check
				 * that each component of the mountpoint
				 * is a directory (and not a symlink)
				 */
				char temp[sizeof(sfs.f_mntonname)];
				char *cp;
				int ok = 1;

				strcpy(temp, sfs.f_mntonname);
				for(;;) {
					if ((cp = strrchr(temp, '/')) == NULL) {
						ok = 0;
						break;
					}
					if (cp <= temp)
						break;
					*cp = 0;
					if (lstat(temp, &sb) < 0 || (sb.st_mode & S_IFMT) != S_IFDIR) {
						ok = 0;
						break;
					}
				}
				if (ok) {
					resolved_len = strlcpy(resolved, sfs.f_mntonname, PATH_MAX);
					continue;
				}
			}
			/* if we fail, use the other methods. */
		}
		if (islink) {
			if (symlinks++ > MAXSYMLINKS) {
				errno = ELOOP;
				goto error_return;
			}
			slen = readlink(resolved, symlink, sizeof(symlink) - 1);
			if (slen < 0) {
				goto error_return;
			}
			symlink[slen] = '\0';
			if (symlink[0] == '/') {
				resolved[1] = 0;
				resolved_len = 1;
				lastdev = rootdev;
			} else if (resolved_len > 1) {
				/* Strip the last path component. */
				resolved[resolved_len - 1] = '\0';
				q = strrchr(resolved, '/') + 1;
				*q = '\0';
				resolved_len = q - resolved;
			}

			/*
			 * If there are any path components left, then
			 * append them to symlink. The result is placed
			 * in `left'.
			 */
			if (p != NULL) {
				if (symlink[slen - 1] != '/') {
					if (slen + 1 >= sizeof(symlink)) {
						errno = ENAMETOOLONG;
						goto error_return;
					}
					symlink[slen] = '/';
					symlink[slen + 1] = 0;
				}
				left_len = strlcat(symlink, left, sizeof(left));
				if (left_len >= sizeof(left)) {
					errno = ENAMETOOLONG;
					goto error_return;
				}
			}
			left_len = strlcpy(left, symlink, sizeof(left));
		} else if (useattrs) {
			/*
			 * attrs already has the real name.
			 */

			resolved[save_resolved_len] = '\0';
			resolved_len = strlcat(resolved, (const char *)&attrs.name + attrs.name.attr_dataoffset, PATH_MAX);
			if (resolved_len >= PATH_MAX) {
				errno = ENAMETOOLONG;
				goto error_return;
			}
		}
		/*
		 * For the case of useattrs == 0, we could scan the directory
		 * and try to match the inode.  There are many problems with
		 * this: (1) the directory may not be readable, (2) for multiple
		 * hard links, we would find the first, but not necessarily
		 * the one specified in the path, (3) we can't try to do
		 * a case-insensitive search to match the right one in (2),
		 * because the underlying filesystem may do things like
		 * decompose composed characters.  For most cases, doing
		 * nothing is the right thing when useattrs == 0, so we punt
		 * for now.
		 */
	}

	/*
	 * Remove trailing slash except when the resolved pathname
	 * is a single "/".
	 */
	if (resolved_len > 1 && resolved[resolved_len - 1] == '/')
		resolved[resolved_len - 1] = '\0';
	return (resolved);
}

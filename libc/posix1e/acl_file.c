/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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

/* XXX temporary implementation using __acl__ file */

#include <sys/appleapiopts.h>
#include <sys/types.h>
#include <sys/acl.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "aclvar.h"

static acl_t	acl_get_file1(const char *path, acl_type_t acl_type, int follow);

int
acl_delete_fd_np(int filedes, acl_type_t type)
{
	errno = ENOTSUP;
	return(-1);
}

int
acl_delete_file_np(const char *path, acl_type_t type)
{
	errno = ENOTSUP;
	return(-1);
}

int
acl_delete_link_np(const char *path, acl_type_t type)
{
	errno = ENOTSUP;
	return(-1);
}

acl_t
acl_get_fd(int fd)
{
	return(acl_get_fd_np(fd, ACL_TYPE_EXTENDED));
}

acl_t
acl_get_fd_np(int fd, acl_type_t type)
{
	filesec_t	fsec;
	acl_t		acl;
	struct stat	sb;

	if (type != ACL_TYPE_EXTENDED) {
		errno = EINVAL;
		return(NULL);
	}
	if ((fsec = filesec_init()) == NULL)
		return(NULL);

	acl = NULL;
	if (fstatx_np(fd, &sb, fsec) == 0)
		filesec_get_property(fsec, FILESEC_ACL, &acl);
	filesec_free(fsec);
	return(acl);
}

static acl_t
acl_get_file1(const char *path, acl_type_t acl_type, int follow)
{
	filesec_t	fsec;
	acl_t		acl;
	struct stat	sb;

	if (acl_type != ACL_TYPE_EXTENDED) {
		errno = EINVAL;
		return(NULL);
	}
	if ((fsec = filesec_init()) == NULL)
		return(NULL);

	acl = NULL;
	if ((follow ? statx_np(path, &sb, fsec) : lstatx_np(path, &sb, fsec)) == 0)
		filesec_get_property(fsec, FILESEC_ACL, &acl);
	filesec_free(fsec);
	return(acl);
}

acl_t
acl_get_file(const char *path, acl_type_t type)
{
	return(acl_get_file1(path, type, 1 /* follow */));
}

acl_t
acl_get_link_np(const char *path, acl_type_t type)
{
	return(acl_get_file1(path, type, 0 /* no follow */));
}

int
acl_set_fd_np(int fd, acl_t acl, acl_type_t type)
{
	filesec_t	fsec;
	int		error;

	if ((fsec = filesec_init()) == NULL)
		return(-1);
	if ((filesec_set_property(fsec, FILESEC_ACL, &acl)) != 0) {
		filesec_free(fsec);
		return(-1);
	}
	error = fchmodx_np(fd, fsec);
	filesec_free(fsec);
	return((error == 0) ? 0 : -1);
}

int
acl_set_fd(int fd, acl_t acl)
{
	return(acl_set_fd_np(fd, acl, ACL_TYPE_EXTENDED));
}

int
acl_set_file(const char *path, acl_type_t acl_type, acl_t acl)
{
	filesec_t	fsec;
	int		error;

	if ((fsec = filesec_init()) == NULL)
		return(-1);
	if (filesec_set_property(fsec, FILESEC_ACL, &acl) != 0) {
		filesec_free(fsec);
		return(-1);
	}
	error = chmodx_np(path, fsec);
	filesec_free(fsec);
	return((error == 0) ? 0 : -1);
}

int
acl_set_link_np(const char *path, acl_type_t acl_type, acl_t acl)
{
	struct stat s;

	if(lstat(path, &s) < 0)
		return(-1);
	if(S_ISLNK(s.st_mode)) {
		errno = ENOTSUP;
		return(-1);
	}
	return(acl_set_file(path, acl_type, acl));
}

/*
 * Not applicable; not supported.
 */
int
acl_delete_def_file(__unused const char *path)
{
	errno = ENOTSUP;
	return(-1);
}

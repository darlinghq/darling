/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
 * sys/statvfs.c
 */
#include <sys/param.h>
#include <sys/mount.h>

#include <sys/statvfs.h>	/* glue header */


/* Internal common conversion function */
static void
cvt_statfs_to_statvfs(struct statfs *from, struct statvfs *to)
{
	to->f_bsize	= from->f_iosize;
	to->f_frsize	= from->f_bsize;
	to->f_blocks	= (fsblkcnt_t)from->f_blocks;
	to->f_bfree	= (fsblkcnt_t)from->f_bfree;
	to->f_bavail	= (fsblkcnt_t)from->f_bavail;
	to->f_files	= (fsfilcnt_t)from->f_files;
	to->f_ffree	= (fsfilcnt_t)from->f_ffree;
	to->f_favail	= (fsfilcnt_t)from->f_ffree;
	to->f_fsid	= from->f_fsid.val[0];	/* XXX bad if non-root */
	to->f_namemax	= NAME_MAX;		/* XXX should be per FS */

	/* Conver FS flags */
	to->f_flag	= 0;
	if( from->f_flags & MNT_RDONLY)
		to->f_flag |= ST_RDONLY;
	if( from->f_flags & MNT_NOSUID)
		to->f_flag |= ST_NOSUID;

	return;
}


int
fstatvfs(int fildes, struct statvfs *buf)
{
	int	rv;
	struct statfs cvt;

	if ((rv = fstatfs(fildes, &cvt)) == 0)
		cvt_statfs_to_statvfs(&cvt, buf);

	return(rv);
}


int
statvfs(const char * __restrict path, struct statvfs * __restrict buf)
{
	int	rv;
	struct statfs cvt;

	if ((rv = statfs(path, &cvt)) == 0)
		cvt_statfs_to_statvfs(&cvt, buf);

	return(rv);
}

/* EOF */

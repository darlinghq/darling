/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
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

#include <errno.h>
#include <unistd.h>
#include <System/sys/fsctl.h>

/*
 * Sync a mounted filesystem, without syncing others.
 * There are currently two flags that can be used:
 *
 * SYNC_VOLUME_FULLSYNC causes it to try to push the
 * data to the platter (otherwise, it just pushes it
 * to the disk drive, where it may remain in cache
 * for a while).
 *
 * SYNC_VOLUME_WAIT causes it to wait until the writes
 * have completed.  Otherwise, it'll return as soon
 * as the requests have been made.
 *
 * The functions are a simple wrapper for fsctl, and
 * return what it does.
 */

int
sync_volume_np(const char *path, int flags) {
	int full_sync = 0;
	int terrno;
	int rv;

	if (flags & SYNC_VOLUME_FULLSYNC)
		full_sync |= FSCTL_SYNC_FULLSYNC;

	if (flags & SYNC_VOLUME_WAIT)
		full_sync |= FSCTL_SYNC_WAIT;

	terrno = errno;
	rv = (fsctl(path, FSCTL_SYNC_VOLUME, &full_sync, 0) == -1) ? errno : 0;
	errno = terrno;
	return rv;
}

int
fsync_volume_np(int fd, int flags) {
	int full_sync = 0;
	int terrno;
	int rv;

	if (flags & SYNC_VOLUME_FULLSYNC)
		full_sync |= FSCTL_SYNC_FULLSYNC;

	if (flags & SYNC_VOLUME_WAIT)
		full_sync |= FSCTL_SYNC_WAIT;

	terrno = errno;
	rv = (ffsctl(fd, FSCTL_SYNC_VOLUME, &full_sync, 0) == -1) ? errno : 0;
	errno = terrno;
	return rv;
}


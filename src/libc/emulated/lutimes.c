/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/attr.h>
#include <sys/time.h>
#include <unistd.h>
#include <strings.h>

int
lutimes(const char *path, const struct timeval *times)
{
	struct stat s;
	struct attrlist a;
	struct {
	    struct timespec mod;
	    struct timespec access;
	} t;

	if(lstat(path, &s) < 0)
		return -1;
	if((s.st_mode & S_IFMT) != S_IFLNK)
		return utimes(path, times);
	bzero(&a, sizeof(a));
	a.bitmapcount = ATTR_BIT_MAP_COUNT;
	a.commonattr = ATTR_CMN_MODTIME | ATTR_CMN_ACCTIME;
	if(times) {
		TIMEVAL_TO_TIMESPEC(&times[0], &t.access);
		TIMEVAL_TO_TIMESPEC(&times[1], &t.mod);
	} else {
		struct timeval now;

		if(gettimeofday(&now, NULL) < 0)
			return -1;
		TIMEVAL_TO_TIMESPEC(&now, &t.access);
		TIMEVAL_TO_TIMESPEC(&now, &t.mod);
	}
	return setattrlist(path, &a, &t, sizeof(t), FSOPT_NOFOLLOW);
}

/*
 * Copyright (c) 2003-2005 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved
 *
 * 	File:	libc/sys/gettimeofday.c
 */

#include <time.h>
#include <tzfile.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>

#define	__APPLE_API_PRIVATE
#include <machine/cpu_capabilities.h>
#undef	__APPLE_API_PRIVATE

extern int __gettimeofday(struct timeval *, struct timezone *);
extern int __commpage_gettimeofday(struct timeval *);

int gettimeofday (struct timeval *tp, void *vtzp)
{
	static int validtz = 0;
	static struct timezone cached_tz = {0};
	struct timezone *tzp = (struct timezone *)vtzp;
	struct timeval atv;

	if (tp == NULL) {
	    if (tzp == NULL)
		return	(0);
	    tp = &atv;
	}

	if (__commpage_gettimeofday(tp)) {		/* first try commpage */
		if (__gettimeofday(tp, NULL) < 0) {	/* if it fails, use syscall */
			return (-1);
		}
	}

	if (tzp) {
	    if (validtz == 0)  {
		struct tm *localtm = localtime ((time_t *)&tp->tv_sec);
		cached_tz.tz_dsttime = localtm->tm_isdst;
		cached_tz.tz_minuteswest =
		    (-localtm->tm_gmtoff / SECSPERMIN) +
		    (localtm->tm_isdst * MINSPERHOUR);
		validtz = 1;
	    }
	    tzp->tz_dsttime = cached_tz.tz_dsttime;
	    tzp->tz_minuteswest = cached_tz.tz_minuteswest;
	}
	return (0);
}

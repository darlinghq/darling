/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
#include <time.h>
#include <tzfile.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <mach/clock_types.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <machine/cpu_capabilities.h>

int __commpage_gettimeofday(struct timeval *);

 
#define TIME_ADD(rsecs, secs, rfrac, frac, unit)	\
{							\
	(rfrac) += (frac);				\
	while ((rfrac) >= (unit)) {			\
		(rfrac) -= (unit);			\
		(rsecs) += 1;				\
	}						\
	(rsecs) += (secs);				\
}


int __commpage_gettimeofday(struct timeval  *tp) {
        commpage_timeofday_data_t *commpage_timeofday_datap;
	uint64_t	tbr;
	uint64_t	t64;
	uint64_t	TimeBase;
	uint32_t	TimeStamp_usec;
	uint32_t	TimeStamp_sec;
	uint32_t	TimeBaseTicks_per_sec;
	uint64_t	TimeBase_magic;
	uint32_t	TimeBase_add;
	uint32_t	TimeBase_shift;
	uint32_t	x, q;


        commpage_timeofday_datap =  (commpage_timeofday_data_t *)_COMM_PAGE_TIMEOFDAY_DATA;

	do {
		TimeBase = commpage_timeofday_datap->TimeBase;
		TimeStamp_sec = commpage_timeofday_datap->TimeStamp_sec;
		TimeStamp_usec = commpage_timeofday_datap->TimeStamp_usec;
		TimeBaseTicks_per_sec = commpage_timeofday_datap->TimeBaseTicks_per_sec;
		TimeBase_magic = commpage_timeofday_datap->TimeBase_magic;
		TimeBase_add = commpage_timeofday_datap->TimeBase_add;
		TimeBase_shift = commpage_timeofday_datap->TimeBase_shift;
	} while (TimeBase != commpage_timeofday_datap->TimeBase);

	if (TimeBase == 0)
		return(1);

	tbr = mach_absolute_time();

	t64 = tbr - TimeBase;

	if (t64 >= (uint64_t)TimeBaseTicks_per_sec)
		return(1);

	x = (uint32_t)t64;
	q = ((uint64_t)x * (uint32_t)(TimeBase_magic)) >> 32;
	tp->tv_usec = TimeBase_add ? (((x - q) >> 1) + q) >> (TimeBase_shift - 1) : q >> TimeBase_shift;
	tp->tv_sec = 0;

	TIME_ADD(tp->tv_sec, TimeStamp_sec, tp->tv_usec, TimeStamp_usec, USEC_PER_SEC);

	return(0);
}

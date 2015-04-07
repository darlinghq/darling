/*
 * Copyright (c) 2009-2010 Apple Inc. All rights reserved.
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
#include <stdint.h>
#include <dispatch/dispatch.h>

#define TIME_EVENT_NONE 0
#define TIME_EVENT_ONESHOT 1
#define TIME_EVENT_CLOCK 2
#define TIME_EVENT_CAL 3

/*
 * Timer Event
 */
typedef struct
{
	uint32_t type;
	int64_t start;
	int64_t end;
	uint32_t freq;
	int32_t day;
	int64_t next;
	void (^deactivation_handler)();
	dispatch_source_t src;
	dispatch_source_t t_src;
	dispatch_queue_t t_queue;
	void *contextp;
	uint32_t context32;
	uint64_t context64;
} timer_t;

timer_t *timer_oneshot(time_t when, dispatch_queue_t queue);
timer_t *timer_clock(time_t first, time_t freq_sec, time_t end, dispatch_queue_t queue);
timer_t *timer_calendar(time_t first, time_t freq_mth, time_t end, int day, dispatch_queue_t queue);
timer_t *timer_calendar_long(uint32_t yf, uint32_t mf, uint32_t df, uint32_t hf, uint32_t nf, uint32_t sf, time_t fm, int d, uint32_t ye, uint32_t me, uint32_t de, uint32_t he, uint32_t ne, uint32_t se, dispatch_queue_t queue);

void timer_set_deactivation_handler(timer_t *t, void(^handler)());
void timer_close(timer_t *t);

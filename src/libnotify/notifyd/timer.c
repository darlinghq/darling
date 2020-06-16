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

#include <stdlib.h>
#include <string.h>
#include <Block.h>
#include "timer.h"

#define MINUTE 60
#define HOUR 3600
#define DAY 86400

static const uint8_t mlen[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*
 * Timed events
 *
 * Supported event types:
 *
 * Oneshot
 * Every n seconds/minutes/hours/days/weeks
 * Specific day of the month, every n months
 * Specific weekday following specific day of the month, every n months
 *
 */
static time_t
timer_next(timer_t *t, time_t now)
{
	uint32_t y, m;
	int32_t d, x, a, b, dd, wd;
	struct tm tmp;
	time_t next, tt, tod;
	
	if (t == NULL) return 0;
	
	switch (t->type)
	{
		case TIME_EVENT_ONESHOT:
		{
			/*
			 * oneshot time event
			 */
			if (t->start < now) return 0;
			return t->start;
		}
		case TIME_EVENT_CLOCK:
		{
			/*
			 * event recurs every t->freq seconds
			 */
			
			/* t->end is the cut-off.  If it's in the past, return 0 */
			if ((t->end != 0) && (t->end < now)) return 0;
			
			/* If the start time is in the future, that's the next occurrence */
			if (t->start >= now) return t->start;
			
			/* shouldn't happen, as TIME_EVENT_CLOCK should always recur */ 
			if (t->freq == 0) return 0;
			
			x = (int32_t)(((t->freq - 1) + now - t->start) / t->freq);
			next = t->start + (x * t->freq);
			return next;
		}
		case TIME_EVENT_CAL:
		{
			/*
			 * event recurs every t->freq months
			 * t->base gives us the starting month and year, and the time of day
			 * t->day specifies the day of the month (negative means relative to last day of the month)
			 * t->day is > 100 or < 100, then it means a weekday
			 * 101 = first monday, 102 = first tuesday, ..., 108 = second monday, and so on
			 * -101 = last monday, -102 = last tuesday, ..., -108 = second last monday, and so on
			 */
			
			/* t->end is the cut-off.  If it's in the past, return 0 */
			if ((t->end != 0) && (t->end < now)) return 0;
	
			/* If the start time is in the future, that's the next occurrence */
			if (t->start >= now) return t->start;
			
			next = t->start;
			
			/* If t->next is set from the last time we ran, and it is in the past, we can start there. */
			if ((t->next > 0) && (t->next < now)) next = t->next;
			
			while (next < now)
			{
				/* determine year, month, and time-of-day (clock time) of the next occurance */
				memset(&tmp, 0, sizeof(struct tm));
				localtime_r((const time_t *)&(next), &tmp);
				y = tmp.tm_year;
				m = tmp.tm_mon;
				tod = tmp.tm_sec + (MINUTE * tmp.tm_min) + (HOUR * tmp.tm_hour);
				
				m += t->freq;
				if (m > 11)
				{
					y += (m / 12);
					m %= 12;
				}
				
				/* we now have a year (y), a month (m), and a time of day (tod) */
				
				if (t->day > 0) 
				{
					if (t->day < 100)
					{
						/* easy case: day is the date of the month */
						
						memset(&tmp, 0, sizeof(struct tm));
						tmp.tm_year = y;
						tmp.tm_mon = m;
						tmp.tm_mday = t->day;
						tmp.tm_isdst = -1;
						next = mktime(&tmp) + tod;
						continue;
					}
					else
					{
						/* t->day is a weekday */
						
						wd = t->day - 100;
						
						/* start by finding out the weekday of the first of the month */
						memset(&tmp, 0, sizeof(struct tm));
						tmp.tm_year = y;
						tmp.tm_mon = m;
						tmp.tm_mday = 1;
						tmp.tm_isdst = -1;
						tt = mktime(&tmp);
						localtime_r((const time_t *)&tt, &tmp);
						
						if (tmp.tm_wday == 0) tmp.tm_wday = 7;
						
						x = 0;
						if (tmp.tm_wday > (wd % 7)) x = (wd + 7) - tmp.tm_wday;
						else x = wd - tmp.tm_wday;
						
						tmp.tm_mday += x;
						tmp.tm_isdst = -1;
						next = mktime(&tmp) + tod;
						continue;
					}
				}
				
				if (t->day > -100)
				{
					/* nth day from the end of the month */
					if (m == 1)
					{
						/* determine weekday of last day of February (== March 0) */
						memset(&tmp, 0, sizeof(struct tm));
						tmp.tm_year = y;
						tmp.tm_mon = 2;
						tmp.tm_mday = 0;
						tmp.tm_isdst = -1;
						tt = mktime(&tmp);
						memset(&tmp, 0, sizeof(struct tm));
						localtime_r((const time_t *)&(tt), &tmp);
						d = tmp.tm_mday + t->day;
					}
					else
					{
						d = mlen[m] + t->day;
					}
					
					memset(&tmp, 0, sizeof(struct tm));
					tmp.tm_year = y;
					tmp.tm_mon = m;
					tmp.tm_mday = d;
					tmp.tm_isdst = -1;
					next = mktime(&tmp) + tod;
					continue;
				}
				
				/* t->day is a weekday relative to the end of the month */
				if (m == 1)
				{
					/* determine weekday of last day of February (== March 0) */
					memset(&tmp, 0, sizeof(struct tm));
					tmp.tm_year = y;
					tmp.tm_mon = 2;
					tmp.tm_mday = 0;
					tmp.tm_isdst = -1;
					tt = mktime(&tmp);
					memset(&tmp, 0, sizeof(struct tm));
					localtime_r((const time_t *)&(tt), &tmp);
					d = tmp.tm_mday;
				}
				else
				{
					d = mlen[m];
				}
				
				memset(&tmp, 0, sizeof(struct tm));
				tmp.tm_year = y;
				tmp.tm_mon = m;
				tmp.tm_mday = d;
				tmp.tm_isdst = -1;
				
				dd = -1 * (t->day + 100);
				a = dd % 7;
				b = (dd + 6) / 7;
				if (a <= tmp.tm_wday) b--;
				tmp.tm_mday = ((a - tmp.tm_wday) + d) - (b * 7);
				next = mktime(&tmp) + tod;
			}
			
			t->next = next;
			return next;
		}
		default:
		{
			return 0;
		}
	}
	
}

/*
 * This does the actual free.
 * It is dispatched on the timer's dispatch source queue to make it safe.
 */
static void
timer_free(timer_t *t)
{
	if (t == NULL) return;
	if (t->contextp != NULL) free(t->contextp);

	dispatch_release(t->t_src);
	dispatch_release(t->t_queue);

	memset(t, 0, sizeof(timer_t));
	free(t);
}

void
timer_close(timer_t *t)
{
	if (t == NULL) return;

	if (t->t_src != NULL) dispatch_source_cancel(t->t_src);

	/*
	 * We need to make sure that the source's event handler isn't currently running
	 * before we free the timer.  We let the source's queue do the actual free.
	 */
	dispatch_async(t->t_queue, ^{ timer_free(t); });
}

timer_t *
timer_oneshot(time_t when, dispatch_queue_t queue)
{
	timer_t *t;
	time_t now;
	dispatch_time_t trigger;

	/* refuse a trigger time in the past */
	now = time(0);
	if (when <= now) return NULL;

	t = calloc(1, sizeof(timer_t));
	if (t == NULL) return NULL;

	dispatch_retain(queue);
	
	t->type = TIME_EVENT_ONESHOT;
	t->start = when;
	t->t_queue = queue;

	t->t_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
	t->src = dispatch_source_create(DISPATCH_SOURCE_TYPE_DATA_ADD, 0, 0, queue);

	trigger = dispatch_walltime(NULL, (t->start - now) * NSEC_PER_SEC);
	dispatch_source_set_timer(t->t_src, trigger, NSEC_PER_SEC, 0);

	dispatch_source_set_event_handler(t->t_src, ^{
		dispatch_source_merge_data(t->src, 1);
		dispatch_source_cancel(t->t_src);
	});
	
	dispatch_activate(t->t_src);
	return t;
}

timer_t *
timer_clock(time_t first, time_t freq_sec, time_t end, dispatch_queue_t queue)
{
	timer_t *t;
	time_t now;
	dispatch_time_t trigger;
	int64_t x;
	
	if (freq_sec == 0) return timer_oneshot(first, queue);
	
	now = time(0);
	
	t = calloc(1, sizeof(timer_t));
	if (t == NULL) return NULL;
	
	t->type = TIME_EVENT_CLOCK;

	if (first < now)
	{
		x = ((freq_sec - 1) + now - first) / freq_sec;
		t->start = first + (x * freq_sec);
	}
	else
	{
		t->start = first;
	}

	t->end = end;
	t->freq = (uint32_t)freq_sec;
	t->t_queue = queue;
	
	t->t_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
	t->src = dispatch_source_create(DISPATCH_SOURCE_TYPE_DATA_ADD, 0, 0, queue);
	
	trigger = dispatch_walltime(NULL, (t->start - now) * NSEC_PER_SEC);
	dispatch_source_set_timer(t->t_src, trigger, freq_sec * NSEC_PER_SEC, 0);
	
	dispatch_source_set_event_handler(t->t_src, ^{
		unsigned long n = dispatch_source_get_data(t->t_src);
		dispatch_source_merge_data(t->src, n);
		
		/* deactivate if this is the last time we want to trigger the client source */
		if ((t->end > 0) && (t->end < (time(0) + freq_sec)))
		{
			dispatch_source_cancel(t->t_src);
		}
	});
	
	dispatch_activate(t->t_src);
	
	return t;
}

timer_t *
timer_calendar(time_t first, time_t freq_mth, time_t end, int day, dispatch_queue_t queue)
{
	timer_t *t;
	time_t next, now;
	dispatch_time_t trigger;

	if (freq_mth == 0) return timer_oneshot(first, queue);

	now = time(0);
	
	t = calloc(1, sizeof(timer_t));
	if (t == NULL) return NULL;
	
	t->type = TIME_EVENT_CAL;
	t->start = first;
	t->day = day;
	t->end = end;
	t->freq = (uint32_t)freq_mth;
	t->t_queue = queue;

	t->t_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
	t->src = dispatch_source_create(DISPATCH_SOURCE_TYPE_DATA_ADD, 0, 0, queue);
	
	next = timer_next(t, now);
	trigger = dispatch_walltime(NULL, (next - now) * NSEC_PER_SEC);
	dispatch_source_set_timer(t->t_src, trigger, NSEC_PER_SEC, 0);
	
	dispatch_source_set_event_handler(t->t_src, ^{
		unsigned long n = dispatch_source_get_data(t->t_src);
		dispatch_source_merge_data(t->src, n);

		time_t now = time(0);
		time_t x = timer_next(t, now);

		/* deactivate when there is no next time */
		if (x == 0) 
		{			
			dispatch_source_cancel(t->t_src);
		}
		else
		{
			dispatch_source_set_timer(t->t_src, dispatch_walltime(NULL, (x - now) * NSEC_PER_SEC), NSEC_PER_SEC, 0);
		}
	});
	
	dispatch_activate(t->t_src);
	
	return t;
}

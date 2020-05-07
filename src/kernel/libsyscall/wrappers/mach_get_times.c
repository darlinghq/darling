/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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
#include <stdbool.h>
#include <sys/types.h>
#include <sys/time.h>
#include <mach/mach_time.h>
#include <sys/time.h>
#include <machine/cpu_capabilities.h>

// From __commpage_gettimeofday.c
extern int __commpage_gettimeofday_internal(struct timeval *tp, uint64_t *tbr_out);
extern kern_return_t _mach_continuous_time(uint64_t* absolute_time, uint64_t* cont_time);
// From mach_continuous_time.c
extern uint64_t _mach_continuous_time_base(void);
// Underlying syscall stub
extern int __gettimeofday_with_mach(struct timeval *, struct timezone *, uint64_t *);

kern_return_t
mach_get_times(uint64_t* absolute_time, uint64_t* cont_time, struct timespec *tp)
{
	if (tp == NULL) {
		return _mach_continuous_time(absolute_time, cont_time);
	}

	uint64_t continuous_time_base_prior = -1, continuous_time_base_post = -1;
	uint64_t tbr;
	struct timeval tv;

	do {
		/*
		 * We need to capture the result of gettimeofday without our continuous
		 * time base changing.  Once we have that, and the value for absolute
		 * time that was used to compute the timespec, we can just add the base
		 * to get the accompanying continuous time.
		 */
		continuous_time_base_prior = _mach_continuous_time_base();

		/*
		 * This call has the necessary memory barriers for this retry loop,
		 * since it is implemented with a retry loop of its own.
		 */
		if (__commpage_gettimeofday_internal(&tv, &tbr)) {
			tbr = 0;
			if (__gettimeofday_with_mach(&tv, NULL, &tbr) < 0) {
				return KERN_FAILURE;
			} else if (tbr == 0) {
#if !(TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
				// On an old kernel, likely chroot'ed. (remove next year)
				tbr = mach_absolute_time();
#else
				__builtin_trap();
#endif
			}
		}

		continuous_time_base_post = _mach_continuous_time_base();
	} while (__builtin_expect(continuous_time_base_prior != continuous_time_base_post, 0));

	if (absolute_time) {
		*absolute_time = tbr;
	}
	if (cont_time) {
		*cont_time = continuous_time_base_prior + tbr;
	}
	tp->tv_sec = tv.tv_sec;
	tp->tv_nsec = tv.tv_usec * NSEC_PER_USEC;

	return KERN_SUCCESS;
}

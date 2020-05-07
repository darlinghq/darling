/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * @APPLE_FREE_COPYRIGHT@
 */

#ifndef _ARM_RTCLOCK_H_
#define _ARM_RTCLOCK_H_

#include <mach/boolean.h>
#include <mach/mach_types.h>
#include <mach/mach_time.h>
#include <arm/machine_routines.h>

#define EndOfAllTime            0xFFFFFFFFFFFFFFFFULL
#define DECREMENTER_MAX         0x7FFFFFFFUL
#define DECREMENTER_MIN         0xAUL

typedef struct _rtclock_data_ {
	uint32_t                                                rtc_sec_divisor;
	uint32_t                                                rtc_usec_divisor;
	mach_timebase_info_data_t               rtc_timebase_const;
	union {
		uint64_t                abstime;
		struct {
			uint32_t        low;
			uint32_t        high;
		} abstime_val;
	}                                                               rtc_base;
	union {
		uint64_t                abstime;
		struct {
			uint32_t        low;
			uint32_t        high;
		} abstime_val;
	}                                                               rtc_adj;
	tbd_ops_data_t                                  rtc_timebase_func;

	/* Only needed for AIC manipulation */
	vm_offset_t                                             rtc_timebase_addr;
	vm_offset_t                                             rtc_timebase_val;
} rtclock_data_t;

extern rtclock_data_t                                   RTClockData;
#define rtclock_sec_divisor                             RTClockData.rtc_sec_divisor
#define rtclock_usec_divisor                    RTClockData.rtc_usec_divisor
#define rtclock_timebase_const                  RTClockData.rtc_timebase_const
#define rtclock_base_abstime                    RTClockData.rtc_base.abstime
#define rtclock_base_abstime_low                RTClockData.rtc_base.abstime_val.low
#define rtclock_base_abstime_high               RTClockData.rtc_base.abstime_val.high
#define rtclock_adj_abstime                             RTClockData.rtc_adj.abstime
#define rtclock_adj_abstime_low                 RTClockData.rtc_adj.abstime_val.low
#define rtclock_adj_abstime_high                RTClockData.rtc_adj.abstime_val.high
#define rtclock_timebase_func                   RTClockData.rtc_timebase_func

/* Only needed for AIC manipulation */
#define rtclock_timebase_addr                   RTClockData.rtc_timebase_addr
#define rtclock_timebase_val                    RTClockData.rtc_timebase_val

extern uint64_t arm_timer_slop_max;

extern void rtclock_intr(unsigned int);
extern boolean_t SetIdlePop(void);

extern void ClearIdlePop(boolean_t);
extern void rtclock_early_init(void);

#endif /* _ARM_RTCLOCK_H_ */

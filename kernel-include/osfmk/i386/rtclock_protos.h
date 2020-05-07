/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
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
/*
 *	File:		rtclock_protos.h
 *	Purpose:	C Routines for handling the machine dependent
 *				real-time clock.
 */

#ifndef _I386_RTCLOCK_PROTOS_H_
#define _I386_RTCLOCK_PROTOS_H_

typedef struct pal_rtc_nanotime pal_rtc_nanotime_t;
extern uint64_t tsc_rebase_abs_time;

extern void     _rtc_nanotime_adjust(
	uint64_t                tsc_base_delta,
	pal_rtc_nanotime_t      *dst);

extern uint64_t _rtc_nanotime_read(
	pal_rtc_nanotime_t      *rntp);

extern uint64_t _rtc_tsc_to_nanoseconds(
	uint64_t    value,
	pal_rtc_nanotime_t      *rntp);

extern void     rtclock_intr(x86_saved_state_t *regs);


/*
 * Timer control.
 */
typedef struct {
	void     (*rtc_config)(void);
	uint64_t (*rtc_set)(uint64_t, uint64_t);
} rtc_timer_t;
extern rtc_timer_t      *rtc_timer;

extern void             rtc_timer_init(void);

extern void             rtclock_early_init(void);
extern void             rtc_nanotime_init(uint64_t);
extern void             rtc_decrementer_configure(void);
#endif /* _I386_RTCLOCK_PROTOS_H_ */

/*
 * Copyright (c) 2000-2008 Apple Inc. All rights reserved.
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
 */

#ifndef _KERN_CLOCK_H_
#define _KERN_CLOCK_H_

#include <stdint.h>
#include <mach/mach_types.h>
#include <mach/clock_types.h>
#include <mach/message.h>
#include <mach/mach_time.h>
#include <mach/boolean.h>

#include <kern/kern_types.h>

#include <sys/cdefs.h>


#ifdef  __LP64__

typedef unsigned long           clock_sec_t;
typedef unsigned int            clock_usec_t, clock_nsec_t;

#else   /* __LP64__ */

typedef uint32_t                        clock_sec_t;
typedef uint32_t                        clock_usec_t, clock_nsec_t;

#endif  /* __LP64__ */

#ifdef  MACH_KERNEL_PRIVATE

#include <kern/queue.h>

/*
 * Clock operations list structure. Contains vectors to machine
 * dependent clock routines.
 */
struct  clock_ops {
	int             (*c_config)(void);              /* configuration */

	int             (*c_init)(void);                /* initialize */

	kern_return_t   (*c_gettime)(   /* get time */
		mach_timespec_t                 *cur_time);

	kern_return_t   (*c_getattr)(   /* get attributes */
		clock_flavor_t                  flavor,
		clock_attr_t                    attr,
		mach_msg_type_number_t  *count);
};
typedef const struct clock_ops  *clock_ops_t;
typedef struct clock_ops        clock_ops_data_t;

/*
 * Actual clock object data structure. Contains the machine
 * dependent operations list and clock operation ports.
 */
struct  clock {
	clock_ops_t             cl_ops;         /* operations list */
	struct ipc_port         *cl_service;    /* service port */
	struct ipc_port         *cl_control;    /* control port */
};
typedef struct clock            clock_data_t;

/*
 * Configure the clock system.
 */
extern void             clock_config(void);
extern void             clock_oldconfig(void);

/*
 * Initialize the clock system.
 */
extern void             clock_init(void);
extern void             clock_oldinit(void);

extern void             clock_timebase_init(void);

/*
 * Initialize the clock ipc service facility.
 */
extern void             clock_service_create(void);

extern void clock_gettimeofday_set_commpage(
	uint64_t                                abstime,
	uint64_t                                sec,
	uint64_t                                frac,
	uint64_t                                scale,
	uint64_t                                tick_per_sec);

extern void                     machine_delay_until(uint64_t interval,
    uint64_t                deadline);

extern uint32_t         hz_tick_interval;

extern void             nanotime_to_absolutetime(
	clock_sec_t             secs,
	clock_nsec_t    nanosecs,
	uint64_t                *result);

#endif /* MACH_KERNEL_PRIVATE */

__BEGIN_DECLS

#ifdef  XNU_KERNEL_PRIVATE

extern void                     clock_adjtime(
	long            *secs,
	int                     *microsecs);

extern void                     clock_initialize_calendar(void);

extern void                     clock_wakeup_calendar(void);

extern void                     clock_update_calendar(void);

extern void                     clock_get_calendar_uptime(clock_sec_t           *secs);

extern void clock_gettimeofday_new(clock_sec_t          *secs,
    clock_usec_t    *microsecs);
extern void                     clock_gettimeofday(
	clock_sec_t                     *secs,
	clock_usec_t            *microsecs);

extern void                     clock_gettimeofday_and_absolute_time(
	clock_sec_t                     *secs,
	clock_usec_t            *microsecs,
	uint64_t                        *absolute_time);


extern void                     clock_set_calendar_microtime(
	clock_sec_t                     secs,
	clock_usec_t            microsecs);

extern void                     clock_get_boottime_nanotime(
	clock_sec_t                     *secs,
	clock_nsec_t            *nanosecs);

extern void                     clock_get_boottime_microtime(
	clock_sec_t                     *secs,
	clock_nsec_t            *microsecs);

extern void                     absolutetime_to_microtime(
	uint64_t                        abstime,
	clock_sec_t                     *secs,
	clock_usec_t            *microsecs);

extern void                     clock_deadline_for_periodic_event(
	uint64_t                        interval,
	uint64_t                        abstime,
	uint64_t                        *deadline);

#if     CONFIG_DTRACE

extern void                     clock_get_calendar_nanotime_nowait(
	clock_sec_t                     *secs,
	clock_nsec_t            *nanosecs);

#endif  /* CONFIG_DTRACE */

boolean_t kdp_clock_is_locked(void);

#endif  /* XNU_KERNEL_PRIVATE */

extern void                     clock_get_calendar_microtime(
	clock_sec_t                     *secs,
	clock_usec_t            *microsecs);

extern void                     clock_get_calendar_absolute_and_microtime(
	clock_sec_t                     *secs,
	clock_usec_t            *microsecs,
	uint64_t                *abstime);

extern void                     clock_get_calendar_nanotime(
	clock_sec_t                     *secs,
	clock_nsec_t            *nanosecs);

extern void                     clock_get_system_microtime(
	clock_sec_t                     *secs,
	clock_usec_t            *microsecs);

extern void                     clock_get_system_nanotime(
	clock_sec_t                     *secs,
	clock_nsec_t            *nanosecs);

extern void                             clock_timebase_info(
	mach_timebase_info_t    info);

extern void                             clock_get_uptime(
	uint64_t                *result);

extern void                             clock_interval_to_deadline(
	uint32_t                interval,
	uint32_t                scale_factor,
	uint64_t                *result);

extern void                             clock_interval_to_absolutetime_interval(
	uint32_t                interval,
	uint32_t                scale_factor,
	uint64_t                *result);

extern void                             clock_absolutetime_interval_to_deadline(
	uint64_t                abstime,
	uint64_t                *result);

extern void                             clock_continuoustime_interval_to_deadline(
	uint64_t                abstime,
	uint64_t                *result);

extern void                             clock_delay_until(
	uint64_t                deadline);

extern void                             absolutetime_to_nanoseconds(
	uint64_t                abstime,
	uint64_t                *result);

extern void                             nanoseconds_to_absolutetime(
	uint64_t                nanoseconds,
	uint64_t                *result);

/*
 * Absolute <-> Continuous Time conversion routines
 *
 * It is the caller's responsibility to ensure that these functions are
 * synchronized with respect to updates to the continuous timebase.  The
 * returned value is only valid until the next update to the continuous
 * timebase.
 *
 * If the value to be returned by continuoustime_to_absolutetime would be
 * negative, zero is returned.  This occurs when the provided continuous time
 * is less the amount of the time the system spent asleep and /must/ be
 * handled.
 */
extern uint64_t                 absolutetime_to_continuoustime(
	uint64_t                abstime);
extern uint64_t                 continuoustime_to_absolutetime(
	uint64_t                conttime);

extern uint64_t mach_absolutetime_asleep;
extern uint64_t mach_absolutetime_last_sleep;

#ifdef  KERNEL_PRIVATE

/*
 * Obsolete interfaces.
 */

#ifndef __LP64__

#define MACH_TIMESPEC_SEC_MAX           (0 - 1)
#define MACH_TIMESPEC_NSEC_MAX          (NSEC_PER_SEC - 1)

#define MACH_TIMESPEC_MAX       ((mach_timespec_t) {                            \
	                                                                MACH_TIMESPEC_SEC_MAX,          \
	                                                                MACH_TIMESPEC_NSEC_MAX } )
#define MACH_TIMESPEC_ZERO      ((mach_timespec_t) { 0, 0 } )

#define ADD_MACH_TIMESPEC_NSEC(t1, nsec)                \
  do {                                                                                  \
	(t1)->tv_nsec += (clock_res_t)(nsec);           \
	if ((clock_res_t)(nsec) > 0 &&                          \
	                (t1)->tv_nsec >= NSEC_PER_SEC) {        \
	        (t1)->tv_nsec -= NSEC_PER_SEC;                  \
	        (t1)->tv_sec += 1;                                              \
	}                                                                                       \
	else if ((clock_res_t)(nsec) < 0 &&                     \
	                         (t1)->tv_nsec < 0) {                   \
	        (t1)->tv_nsec += NSEC_PER_SEC;                  \
	        (t1)->tv_sec -= 1;                                              \
	}                                                                                       \
  } while (0)

#include <Availability.h>

/* Use mach_absolute_time() */
extern mach_timespec_t  clock_get_system_value(void) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_0, __MAC_10_8, __IPHONE_2_0, __IPHONE_6_0);

extern mach_timespec_t  clock_get_calendar_value(void) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_0, __MAC_10_8, __IPHONE_2_0, __IPHONE_6_0);

#else   /* __LP64__ */

#ifdef  XNU_KERNEL_PRIVATE

#define MACH_TIMESPEC_ZERO      ((mach_timespec_t) { 0, 0 } )

#endif  /* XNU_KERNEL_PRIVATE */

#endif  /* __LP64__ */

extern void                             delay_for_interval(
	uint32_t                interval,
	uint32_t                scale_factor);

extern void                             delay_for_interval_with_leeway(
	uint32_t                interval,
	uint32_t                leeway,
	uint32_t                scale_factor);

#ifdef  XNU_KERNEL_PRIVATE
extern void delay(int usec);
#endif  /* XNU_KERNEL_PRIVATE */

#endif  /* KERNEL_PRIVATE */

__END_DECLS

#endif  /* _KERN_CLOCK_H_ */

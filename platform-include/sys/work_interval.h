/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#ifndef _SYS_WORK_INTERVAL_H
#define _SYS_WORK_INTERVAL_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

/*
 * Trusted clients with deadline-sensitive work may report information
 * about the execution of their work using the work interval facility.
 * This is intended to be a higher-level semantic than realtime scheduling,
 * which operates at the level of thread block/unblock. A high level
 * operation may have many blocking points, including IPC to other tasks,
 * and this this metric will capture the overall time to complete a unit of
 * work.
 *
 * A work interval is defined by several timestamps, namely (S)tart,
 * (F)inish, (D)eadline, and (N)ext start.
 *
 *   ... ----+==================+--------+--+==== ...
 *           |                  |        |  |
 *           S                  F        D  N
 *
 *           \__________________/
 *                  Active
 *           \___________________________/
 *                   Work Interval
 *
 *                               \_________/
 *                                    |
 *   report information here ---------+
 *
 * Definitions:
 *
 *   Start: Absolute time when the current deadline-oriented work began. Due
 *          to scheduling latency, preemption, and blocking points, the
 *          thread controlling the work interval may actually begin
 *          executing after this ideal time (which may be the previous work
 *          interval's "next start")
 *   Finish: Absolute time when the current deadline-oriented work finished.
 *          This will typically be a timestamp taken before reporting using
 *          the work interval interface.
 *   Deadline: Absolute time by which the current work was expected to finish.
 *          In cases where the amount of computation (or preemption, or time
 *          spent blocked) causes the active period to take longer than
 *          expected, F may be greater than D.
 *   Next start: Absolute time when the next deadline-oriented work is
 *          expected to begin. This is typically the same as Deadline.
 *   Active: The fraction of the work interval spent completing the work. In
 *          cases where the Finish time exceeded the Deadline, this fraction
 *          will be >1.0.
 *
 * Basic Use:
 *
 *   Clients should report information for a work interval after finishing
 *   work for the current interval but before the next work interval begins.
 *
 *   If Finish far exceeds the previously expected Deadline, the
 *   caller may adjust Next Start to align to a multiple of the period
 *   (and skip over several work intervals that could not be
 *   executed).
 *
 * Caution (!):
 *
 *   Because the information supplied via this facility directly influences power
 *   management decisions, clients should strive to be as accurate as possible.
 *   Failure to do so will adversely impact system power and performance.
 *
 */
#ifndef KERNEL

typedef struct work_interval *work_interval_t;

/* Create a new work interval handle (currently for the current thread only). Flags is unused */
int		work_interval_create(work_interval_t *interval_handle, uint32_t flags);

/* Notify the power management subsystem that the work for a current interval has completed */
int		work_interval_notify(work_interval_t interval_handle, uint64_t start, uint64_t finish, uint64_t deadline, uint64_t next_start, uint32_t flags);

/* Notify, with "finish" implicitly set to the current time */
int		work_interval_notify_simple(work_interval_t interval_handle, uint64_t start, uint64_t deadline, uint64_t next_start);

/* Deallocate work interval (currently for the current thread only) */
int		work_interval_destroy(work_interval_t interval_handle);

#endif /* KERNEL */

#if PRIVATE

/* Private interface between Libsyscall and xnu */
#define WORK_INTERVAL_OPERATION_CREATE	0x00000001	/* arg is a uint64_t * that accepts work interval ID as an OUT param */
#define WORK_INTERVAL_OPERATION_DESTROY	0x00000002
#define WORK_INTERVAL_OPERATION_NOTIFY	0x00000003	/* arg is a work_interval_notification_t */

struct work_interval_notification {
	uint64_t	start;
	uint64_t	finish;
	uint64_t	deadline;
	uint64_t	next_start;
	uint32_t	flags;
	uint32_t	unused1;
};
typedef struct work_interval_notification *work_interval_notification_t;

int		__work_interval_ctl(uint32_t operation, uint64_t work_interval_id, void *arg, size_t len);

#endif /* PRIVATE */

__END_DECLS

#endif /* _SYS_WORK_INTERVAL_H */

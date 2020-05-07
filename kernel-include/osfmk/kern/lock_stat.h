/*
 * Copyright (c) 2018 Apple Computer, Inc. All rights reserved.
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
#ifndef _KERN_LOCKSTAT_H
#define _KERN_LOCKSTAT_H
#include <machine/locks.h>
#include <machine/atomic.h>
#include <kern/lock_group.h>

/*
 * N.B.: On x86, statistics are currently recorded for all indirect mutexes.
 * Also, only the acquire attempt count (GRP_MTX_STAT_UTIL) is maintained
 * as a 64-bit quantity (the new x86 specific statistics are also maintained
 * as 32-bit quantities).
 *
 *
 * Enable this preprocessor define to record the first miss alone
 * By default, we count every miss, hence multiple misses may be
 * recorded for a single lock acquire attempt via lck_mtx_lock
 */
#undef LOG_FIRST_MISS_ALONE

/*
 * This preprocessor define controls whether the R-M-W update of the
 * per-group statistics elements are atomic (LOCK-prefixed)
 * Enabled by default.
 */
#define ATOMIC_STAT_UPDATES 1

/*
 * DTrace lockstat probe definitions
 *
 * Spinlocks
 */
#define LS_LCK_SPIN_LOCK_ACQUIRE        0
#define LS_LCK_SPIN_LOCK_SPIN           1
#define LS_LCK_SPIN_UNLOCK_RELEASE      2

/*
 * Mutexes can also have interlock-spin events, which are
 * unique to our lock implementation.
 */
#define LS_LCK_MTX_LOCK_ACQUIRE                 3
#define LS_LCK_MTX_LOCK_BLOCK                   5
#define LS_LCK_MTX_LOCK_SPIN                    6
#define LS_LCK_MTX_LOCK_ILK_SPIN                7
#define LS_LCK_MTX_TRY_LOCK_ACQUIRE             8
#define LS_LCK_MTX_TRY_SPIN_LOCK_ACQUIRE        9
#define LS_LCK_MTX_UNLOCK_RELEASE               10

#define LS_LCK_MTX_LOCK_SPIN_ACQUIRE            39
/*
 * Provide a parallel set for indirect mutexes
 */
#define LS_LCK_MTX_EXT_LOCK_ACQUIRE                     17
#define LS_LCK_MTX_EXT_LOCK_BLOCK                       18
#define LS_LCK_MTX_EXT_LOCK_SPIN                        19
#define LS_LCK_MTX_EXT_LOCK_ILK_SPIN            20
#define LS_LCK_MTX_TRY_EXT_LOCK_ACQUIRE         21
#define LS_LCK_MTX_EXT_UNLOCK_RELEASE           22

/*
 * Reader-writer locks support a blocking upgrade primitive, as
 * well as the possibility of spinning on the interlock.
 */
#define LS_LCK_RW_LOCK_SHARED_ACQUIRE           23
#define LS_LCK_RW_LOCK_SHARED_BLOCK             24
#define LS_LCK_RW_LOCK_SHARED_SPIN              25

#define LS_LCK_RW_LOCK_EXCL_ACQUIRE             26
#define LS_LCK_RW_LOCK_EXCL_BLOCK               27
#define LS_LCK_RW_LOCK_EXCL_SPIN                28

#define LS_LCK_RW_DONE_RELEASE                  29

#define LS_LCK_RW_TRY_LOCK_SHARED_ACQUIRE       30
#define LS_LCK_RW_TRY_LOCK_SHARED_SPIN          31

#define LS_LCK_RW_TRY_LOCK_EXCL_ACQUIRE         32
#define LS_LCK_RW_TRY_LOCK_EXCL_ILK_SPIN        33

#define LS_LCK_RW_LOCK_SHARED_TO_EXCL_UPGRADE   34
#define LS_LCK_RW_LOCK_SHARED_TO_EXCL_SPIN      35
#define LS_LCK_RW_LOCK_SHARED_TO_EXCL_BLOCK     36

#define LS_LCK_RW_LOCK_EXCL_TO_SHARED_DOWNGRADE 37
#define LS_LCK_RW_LOCK_EXCL_TO_SHARED_ILK_SPIN  38

#define LS_NPROBES                      40
#define LS_LCK_INVALID                  LS_NPROBES

#if CONFIG_DTRACE
extern uint32_t lockstat_probemap[LS_NPROBES];
extern void (*lockstat_probe)(uint32_t, uint64_t, uint64_t,
    uint64_t, uint64_t, uint64_t);
/*
 * Macros to record lockstat probes.
 */
#define LOCKSTAT_RECORD4(probe, lp, arg0, arg1, arg2, arg3)             \
	{                                                               \
	        uint32_t id;                                         \
	        if (__improbable(id = lockstat_probemap[(probe)])) {            \
	                (*lockstat_probe)(id, (uintptr_t)(lp), (arg0),  \
	                    (arg1), (arg2), (arg3));                    \
	        }                                                       \
	}
#define LOCKSTAT_RECORD_(probe, lp, arg0, arg1, arg2, arg3, ...) LOCKSTAT_RECORD4(probe, lp, arg0, arg1, arg2, arg3)
#define LOCKSTAT_RECORD__(probe, lp, arg0, arg1, arg2, arg3, ...) LOCKSTAT_RECORD_(probe, lp, arg0, arg1, arg2, arg3)
#define LOCKSTAT_RECORD(probe, lp, ...) LOCKSTAT_RECORD__(probe, lp, ##__VA_ARGS__, 0, 0, 0, 0)
#else
#define LOCKSTAT_RECORD()
#endif /* CONFIG_DTRACE */

/*
 * Time threshold before dtrace lockstat spin
 * probes are triggered
 */
extern uint64_t dtrace_spin_threshold;

#if CONFIG_DTRACE
void lockprof_invoke(lck_grp_t*, lck_grp_stat_t*, uint64_t);
#endif /* CONFIG_DTRACE */

static inline void
lck_grp_stat_enable(lck_grp_stat_t *stat)
{
	stat->lgs_enablings++;
}

static inline void
lck_grp_stat_disable(lck_grp_stat_t *stat)
{
	stat->lgs_enablings--;
}

#if MACH_KERNEL_PRIVATE
#if LOCK_STATS

static inline void
lck_grp_inc_stats(lck_grp_t *grp, lck_grp_stat_t *stat)
{
	if (__improbable(stat->lgs_enablings)) {
		uint64_t val = os_atomic_inc_orig(&stat->lgs_count, relaxed);
#if CONFIG_DTRACE
		if (__improbable(stat->lgs_limit && (val % (stat->lgs_limit)) == 0)) {
			lockprof_invoke(grp, stat, val);
		}
#else
#pragma unused(val)
#endif /* CONFIG_DTRACE */
	}
}

static inline void
lck_grp_inc_time_stats(lck_grp_t *grp, lck_grp_stat_t *stat, uint64_t time)
{
	if (__improbable(stat->lgs_enablings)) {
		uint64_t val = os_atomic_add_orig(&stat->lgs_count, time, relaxed);
#if CONFIG_DTRACE
		if (__improbable(stat->lgs_limit)) {
			while (__improbable(time > stat->lgs_limit)) {
				time -= stat->lgs_limit;
				lockprof_invoke(grp, stat, val);
			}
			if (__improbable(((val % stat->lgs_limit) + time) > stat->lgs_limit)) {
				lockprof_invoke(grp, stat, val);
			}
		}
#else
#pragma unused(val)
#endif /* CONFIG_DTRACE */
	}
}

#endif /* LOCK_STATS */

static inline void
lck_grp_spin_update_held(void *lock LCK_GRP_ARG(lck_grp_t *grp))
{
#pragma unused(lock)
#if CONFIG_DTRACE
	LOCKSTAT_RECORD(LS_LCK_SPIN_LOCK_ACQUIRE, lock, (uintptr_t)LCK_GRP_PROBEARG(grp));
#endif
#if LOCK_STATS
	if (!grp) {
		return;
	}
	lck_grp_stat_t *stat = &grp->lck_grp_stats.lgss_spin_held;
	lck_grp_inc_stats(grp, stat);
#endif /* LOCK_STATS */
}

static inline void
lck_grp_spin_update_miss(void *lock LCK_GRP_ARG(lck_grp_t *grp))
{
#pragma unused(lock)
#if LOCK_STATS
	if (!grp) {
		return;
	}
	lck_grp_stat_t *stat = &grp->lck_grp_stats.lgss_spin_miss;
	lck_grp_inc_stats(grp, stat);
#endif /* LOCK_STATS */
}

static inline void
lck_grp_spin_update_spin(void *lock LCK_GRP_ARG(lck_grp_t *grp), uint64_t time)
{
#pragma unused(lock, time)
#if CONFIG_DTRACE
	if (time > dtrace_spin_threshold) {
		LOCKSTAT_RECORD(LS_LCK_SPIN_LOCK_SPIN, lock, time LCK_GRP_ARG((uintptr_t)grp));
	}
#endif /* CONFIG_DTRACE */
#if LOCK_STATS
	if (!grp) {
		return;
	}
	lck_grp_stat_t *stat = &grp->lck_grp_stats.lgss_spin_spin;
	lck_grp_inc_time_stats(grp, stat, time);
#endif /* LOCK_STATS */
}

static inline boolean_t
lck_grp_spin_spin_enabled(void *lock LCK_GRP_ARG(lck_grp_t *grp))
{
#pragma unused(lock)
	boolean_t enabled = FALSE;
#if CONFIG_DTRACE
	enabled |= lockstat_probemap[LS_LCK_SPIN_LOCK_SPIN] != 0;
#endif /* CONFIG_DTRACE */
#if LOCK_STATS
	enabled |= (grp && grp->lck_grp_stats.lgss_spin_spin.lgs_enablings);
#endif /* LOCK_STATS */
	return enabled;
}

static void inline
lck_grp_mtx_inc_stats(
	uint64_t* stat)
{
#if ATOMIC_STAT_UPDATES
	os_atomic_inc(stat, relaxed);
#else
	*stat = (*stat)++;
#endif /* ATOMIC_STAT_UPDATES */
}

static void inline
lck_grp_mtx_update_miss(
	struct _lck_mtx_ext_ *lock,
	int *first_miss)
{
#pragma unused(first_miss)
#if LOG_FIRST_MISS_ALONE
	if ((*first_miss & 1) == 0) {
#endif /* LOG_FIRST_MISS_ALONE */
	uint64_t* stat = &lock->lck_mtx_grp->lck_grp_stats.lgss_mtx_miss.lgs_count;
	lck_grp_mtx_inc_stats(stat);

#if LOG_FIRST_MISS_ALONE
	*first_miss |= 1;
}
#endif /* LOG_FIRST_MISS_ALONE */
}

static void inline
lck_grp_mtx_update_direct_wait(
	struct _lck_mtx_ext_ *lock)
{
	uint64_t* stat = &lock->lck_mtx_grp->lck_grp_stats.lgss_mtx_direct_wait.lgs_count;
	lck_grp_mtx_inc_stats(stat);
}

static void inline
lck_grp_mtx_update_wait(
	struct _lck_mtx_ext_ *lock,
	int *first_miss)
{
#pragma unused(first_miss)
#if LOG_FIRST_MISS_ALONE
	if ((*first_miss & 2) == 0) {
#endif /* LOG_FIRST_MISS_ALONE */
	uint64_t* stat = &lock->lck_mtx_grp->lck_grp_stats.lgss_mtx_wait.lgs_count;
	lck_grp_mtx_inc_stats(stat);

#if LOG_FIRST_MISS_ALONE
	*first_miss |= 2;
}
#endif /* LOG_FIRST_MISS_ALONE */
}

static void inline
lck_grp_mtx_update_held(
	struct _lck_mtx_ext_ *lock)
{
	uint64_t* stat = &lock->lck_mtx_grp->lck_grp_stats.lgss_mtx_held.lgs_count;
	lck_grp_mtx_inc_stats(stat);
}
#endif /* MACH_KERNEL_PRIVATE */
#endif /* _KERN_LOCKSTAT_H */

/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

#ifndef _KERN_SCHED_CLUTCH_H_
#define _KERN_SCHED_CLUTCH_H_

#include <kern/sched.h>
#include <machine/atomic.h>
#include <kern/priority_queue.h>
#include <kern/thread_group.h>
#include <kern/bits.h>

#if CONFIG_SCHED_CLUTCH

/*
 * Clutch ordering based on thread group flags (specified
 * by the thread grouping mechanism). These properties
 * define a thread group specific priority boost.
 *
 * The current implementation gives a slight boost to
 * HIGH & MED thread groups which effectively deprioritizes
 * daemon thread groups which are marked "Efficient" on AMP
 * systems.
 */
#define SCHED_CLUTCH_TG_PRI_LOW           0x0
#define SCHED_CLUTCH_TG_PRI_MED           0x1
#define SCHED_CLUTCH_TG_PRI_HIGH          0x2

/*
 * For the current implementation, bound threads are not managed
 * in the clutch hierarchy. This helper macro is used to indicate
 * if the thread should be in the hierarchy.
 */
#define SCHED_CLUTCH_THREAD_ELIGIBLE(thread)    ((thread->bound_processor) == PROCESSOR_NULL)

/*
 *
 * Clutch hierarchy locking protocol
 *
 * The scheduler clutch hierarchy is protected by a combination of
 * atomics and pset lock.
 * - All fields protected by the pset lock are annotated with (P)
 * - All fields updated using atomics are annotated with (A)
 * - All fields that are unprotected and are not updated after
 *   initialization are annotated with (I)
 */

/*
 * struct sched_clutch_root_bucket
 *
 * A clutch_root_bucket represents all threads across all thread groups
 * that are in the same scheduler bucket (FG/IN/...). The clutch_root_bucket
 * is selected for execution by the root level bucket selection algorithm
 * which bases the decision on the clutch_root_bucket's deadline (EDF). The
 * deadline for a root bucket is calculated based on its runnable timestamp
 * and the worst-case-execution-latency values specied in sched_clutch_root_bucket_wcel[]
 */
struct sched_clutch_root_bucket {
	/* (I) sched bucket represented by this root bucket */
	uint8_t                         scrb_bucket;
	/* (P) priority queue for all clutch buckets in this sched bucket */
	struct priority_queue           scrb_clutch_buckets;
	/* (P) priority queue entry to use for enqueueing root bucket into root prioq */
	struct priority_queue_entry     scrb_pqlink;
	/* (P) ageout deadline for this root bucket */
	uint64_t                        scrb_deadline;
	/* (P) warped deadline for root bucket */
	uint64_t                        scrb_warped_deadline;
	/* (P) warp remaining for root bucket */
	uint64_t                        scrb_warp_remaining;
};
typedef struct sched_clutch_root_bucket *sched_clutch_root_bucket_t;

/*
 * struct sched_clutch_root
 *
 * A clutch_root represents the root of the hierarchy. It maintains a
 * priority queue of all runnable root buckets. The clutch_root also
 * maintains the information about the last clutch_root_bucket scheduled
 * in order to implement bucket level quantum. The bucket level quantums
 * allow low priority buckets to get a "fair" chance of using the CPU even
 * if they contain a bunch of short executing threads. The bucket quantums
 * are configured using sched_clutch_root_bucket_quantum[]
 */
struct sched_clutch_root {
	/* (P) root level priority; represents the highest runnable thread in the hierarchy */
	int16_t                         scr_priority;
	/* (P) total number of runnable threads in the hierarchy */
	uint16_t                        scr_thr_count;
	/* (P) root level urgency; represents the urgency of the whole hierarchy for pre-emption purposes */
	int16_t                         scr_urgency;

	/* (I) processor set this hierarchy belongs to */
	processor_set_t                 scr_pset;
	/*
	 * (P) list of all runnable clutch buckets across the system;
	 * allows easy iteration in the sched tick based timesharing code
	 */
	queue_head_t                    scr_clutch_buckets;
	/*
	 * (P) list of all runnable foreign buckets in this hierarchy;
	 * used for tracking thread groups which need to be migrated when
	 * psets are available
	 */
	queue_head_t                    scr_foreign_buckets;

	/* Root level bucket management */

	/* (P) bitmap of all runnable clutch_root_buckets; used for root pri calculation */
	bitmap_t                        scr_runnable_bitmap[BITMAP_LEN(TH_BUCKET_SCHED_MAX)];
	/* (P) bitmap of all runnable root buckets which have warps remaining */
	bitmap_t                        scr_warp_available[BITMAP_LEN(TH_BUCKET_SCHED_MAX)];
	/* (P) priority queue of all runnable clutch_root_buckets */
	struct priority_queue           scr_root_buckets;
	/* (P) storage for all possible clutch_root_buckets */
	struct sched_clutch_root_bucket scr_buckets[TH_BUCKET_SCHED_MAX];
};
typedef struct sched_clutch_root *sched_clutch_root_t;

/* forward declaration for sched_clutch */
struct sched_clutch;

/*
 * sched_clutch_bucket_cpu_data_t
 *
 * Used for maintaining clutch bucket used and blocked time. The
 * values are used for calculating the interactivity score for the
 * clutch bucket.
 *
 * Since the CPU used/blocked calculation uses wide atomics, the data
 * types used are different based on the platform.
 */

#if __arm64__

#define CLUTCH_CPU_DATA_MAX             (UINT64_MAX)
typedef uint64_t                        clutch_cpu_data_t;
typedef unsigned __int128               clutch_cpu_data_wide_t;

#else /* __arm64__ */

#define CLUTCH_CPU_DATA_MAX             (UINT32_MAX)
typedef uint32_t                        clutch_cpu_data_t;
typedef uint64_t                        clutch_cpu_data_wide_t;

#endif /* __arm64__ */

typedef union sched_clutch_bucket_cpu_data {
	struct {
		/* Clutch bucket CPU used across all threads */
		clutch_cpu_data_t       scbcd_cpu_used;
		/* Clutch bucket voluntary blocked time */
		clutch_cpu_data_t       scbcd_cpu_blocked;
	} cpu_data;
	clutch_cpu_data_wide_t          scbcd_cpu_data_packed;
} sched_clutch_bucket_cpu_data_t;

/*
 * struct sched_clutch_bucket
 *
 * A sched_clutch_bucket represents the set of threads for a thread
 * group at a particular scheduling bucket. It maintains information
 * about the CPU usage & blocking behavior of all threads part of
 * the clutch_bucket and maintains the timesharing attributes for
 * threads in its runq. It uses the decay based algorithm to timeshare
 * among threads in the runq.
 */
struct sched_clutch_bucket {
	/* (I) bucket for the clutch_bucket */
	uint8_t                         scb_bucket;
	/* (P) priority of the clutch bucket */
	uint8_t                         scb_priority;
	/* (P) interactivity score of the clutch bucket */
	uint8_t                         scb_interactivity_score;
	/* (P) flag to indicate if the bucket is a foreign bucket */
	bool                            scb_foreign;

	/* Properties used for timesharing threads in this clutch_bucket */

	/* (P) number of threads in this clutch_bucket; should match runq.count */
	uint16_t                        scb_thr_count;
	/* (A) run count (running + runnable) for this clutch_bucket */
	uint16_t _Atomic                scb_run_count;
	/* (A) sched tick when the clutch bucket load/shifts were updated */
	uint32_t _Atomic                scb_timeshare_tick;
	/* (A) priority shifts for threads in the clutch_bucket */
	uint32_t _Atomic                scb_pri_shift;
	/* (P) linkage for all clutch_buckets in a root bucket; used for tick operations */
	queue_chain_t                   scb_listlink;


	/* (P) timestamp for the last time the interactivity score was updated */
	uint64_t                        scb_interactivity_ts;
	/* (P) timestamp for the last time the clutch_bucket blocked */
	uint64_t                        scb_blocked_ts;

	/* (A) CPU usage information for the clutch bucket */
	sched_clutch_bucket_cpu_data_t  scb_cpu_data;

	/* (P) linkage for clutch_bucket in root_bucket priority queue */
	struct priority_queue_entry     scb_pqlink;
	/* (I) clutch to which this clutch bucket belongs */
	struct sched_clutch             *scb_clutch;
	/* (A) pointer to the root of the hierarchy this bucket is in */
	struct sched_clutch_root        *scb_root;
	/* (P) priority queue of threads based on their promoted/base priority */
	struct priority_queue           scb_clutchpri_prioq;
	/* (P) runq of threads in clutch_bucket */
	struct run_queue                scb_runq;
};
typedef struct sched_clutch_bucket *sched_clutch_bucket_t;


/*
 * struct sched_clutch
 *
 * A sched_clutch is a 1:1 mapping to a thread group. It maintains the
 * storage for all clutch buckets for this thread group and some properties
 * of the thread group (such as flags etc.)
 */
struct sched_clutch {
	/*
	 * (A) number of runnable threads in sched_clutch; needs to be atomic
	 * to support cross cluster sched_clutch migrations.
	 */
	uint16_t _Atomic                sc_thr_count;
	/*
	 * Grouping specific parameters. Currently the implementation only
	 * supports thread_group based grouping.
	 */
	union {
		/* (A) priority specified by the thread grouping mechanism */
		uint8_t _Atomic         sc_tg_priority;
	};
	union {
		/* (I) Pointer to thread group */
		struct thread_group     *sc_tg;
	};
	/* (I) storage for all clutch_buckets for this clutch */
	struct sched_clutch_bucket      sc_clutch_buckets[TH_BUCKET_SCHED_MAX];
};
typedef struct sched_clutch *sched_clutch_t;


/* Clutch lifecycle management */
void sched_clutch_init_with_thread_group(sched_clutch_t, struct thread_group *);
void sched_clutch_destroy(sched_clutch_t);

/* Clutch thread membership management */
void sched_clutch_thread_clutch_update(thread_t, sched_clutch_t, sched_clutch_t);

/* Clutch timesharing stats management */
uint32_t sched_clutch_thread_run_bucket_incr(thread_t, sched_bucket_t);
uint32_t sched_clutch_thread_run_bucket_decr(thread_t, sched_bucket_t);
void sched_clutch_cpu_usage_update(thread_t, uint64_t);
uint32_t sched_clutch_thread_pri_shift(thread_t, sched_bucket_t);

/* Clutch properties accessors */
uint32_t sched_clutch_root_count(sched_clutch_root_t);

/* Grouping specific external routines */
extern sched_clutch_t sched_clutch_for_thread(thread_t);

#endif /* CONFIG_SCHED_CLUTCH */

#endif /* _KERN_SCHED_CLUTCH_H_ */

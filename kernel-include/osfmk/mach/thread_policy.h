/*
 * Copyright (c) 2000-2007 Apple Inc. All rights reserved.
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

#ifndef _MACH_THREAD_POLICY_H_
#define _MACH_THREAD_POLICY_H_

#include <mach/mach_types.h>

/*
 * These are the calls for accessing the policy parameters
 * of a particular thread.
 *
 * The extra 'get_default' parameter to the second call is
 * IN/OUT as follows:
 * 1) if asserted on the way in it indicates that the default
 * values should be returned, not the ones currently set, in
 * this case 'get_default' will always be asserted on return;
 * 2) if unasserted on the way in, the current settings are
 * desired and if still unasserted on return, then the info
 * returned reflects the current settings, otherwise if
 * 'get_default' returns asserted, it means that there are no
 * current settings due to other parameters taking precedence,
 * and the default ones are being returned instead.
 */

typedef natural_t       thread_policy_flavor_t;
typedef integer_t       *thread_policy_t;

/*
 *  kern_return_t	thread_policy_set(
 *                                       thread_t					thread,
 *                                       thread_policy_flavor_t		flavor,
 *                                       thread_policy_t				policy_info,
 *                                       mach_msg_type_number_t		count);
 *
 *  kern_return_t	thread_policy_get(
 *                                       thread_t					thread,
 *                                       thread_policy_flavor_t		flavor,
 *                                       thread_policy_t				policy_info,
 *                                       mach_msg_type_number_t		*count,
 *                                       boolean_t					*get_default);
 */

/*
 * Defined flavors.
 */
/*
 * THREAD_STANDARD_POLICY:
 *
 * This is the standard (fair) scheduling mode, assigned to new
 * threads.  The thread will be given processor time in a manner
 * which apportions approximately equal share to long running
 * computations.
 *
 * Parameters:
 *	[none]
 */

#define THREAD_STANDARD_POLICY                  1

struct thread_standard_policy {
	natural_t               no_data;
};

typedef struct thread_standard_policy   thread_standard_policy_data_t;
typedef struct thread_standard_policy   *thread_standard_policy_t;

#define THREAD_STANDARD_POLICY_COUNT    0

/*
 * THREAD_EXTENDED_POLICY:
 *
 * Extended form of THREAD_STANDARD_POLICY, which supplies a
 * hint indicating whether this is a long running computation.
 *
 * Parameters:
 *
 * timeshare: TRUE (the default) results in identical scheduling
 * behavior as THREAD_STANDARD_POLICY.
 */

#define THREAD_EXTENDED_POLICY                  1

struct thread_extended_policy {
	boolean_t               timeshare;
};

typedef struct thread_extended_policy   thread_extended_policy_data_t;
typedef struct thread_extended_policy   *thread_extended_policy_t;

#define THREAD_EXTENDED_POLICY_COUNT    ((mach_msg_type_number_t) \
	(sizeof (thread_extended_policy_data_t) / sizeof (integer_t)))

/*
 * THREAD_TIME_CONSTRAINT_POLICY:
 *
 * This scheduling mode is for threads which have real time
 * constraints on their execution.
 *
 * Parameters:
 *
 * period: This is the nominal amount of time between separate
 * processing arrivals, specified in absolute time units.  A
 * value of 0 indicates that there is no inherent periodicity in
 * the computation.
 *
 * computation: This is the nominal amount of computation
 * time needed during a separate processing arrival, specified
 * in absolute time units.
 *
 * constraint: This is the maximum amount of real time that
 * may elapse from the start of a separate processing arrival
 * to the end of computation for logically correct functioning,
 * specified in absolute time units.  Must be (>= computation).
 * Note that latency = (constraint - computation).
 *
 * preemptible: This indicates that the computation may be
 * interrupted, subject to the constraint specified above.
 */

#define THREAD_TIME_CONSTRAINT_POLICY   2

struct thread_time_constraint_policy {
	uint32_t                period;
	uint32_t                computation;
	uint32_t                constraint;
	boolean_t               preemptible;
};

typedef struct thread_time_constraint_policy    \
        thread_time_constraint_policy_data_t;
typedef struct thread_time_constraint_policy    \
        *thread_time_constraint_policy_t;

#define THREAD_TIME_CONSTRAINT_POLICY_COUNT     ((mach_msg_type_number_t) \
	(sizeof (thread_time_constraint_policy_data_t) / sizeof (integer_t)))

/*
 * THREAD_PRECEDENCE_POLICY:
 *
 * This may be used to indicate the relative value of the
 * computation compared to the other threads in the task.
 *
 * Parameters:
 *
 * importance: The importance is specified as a signed value.
 */

#define THREAD_PRECEDENCE_POLICY                3

struct thread_precedence_policy {
	integer_t               importance;
};

typedef struct thread_precedence_policy         thread_precedence_policy_data_t;
typedef struct thread_precedence_policy         *thread_precedence_policy_t;

#define THREAD_PRECEDENCE_POLICY_COUNT  ((mach_msg_type_number_t) \
	(sizeof (thread_precedence_policy_data_t) / sizeof (integer_t)))

/*
 * THREAD_AFFINITY_POLICY:
 *
 * This policy is experimental.
 * This may be used to express affinity relationships
 * between threads in the task. Threads with the same affinity tag will
 * be scheduled to share an L2 cache if possible. That is, affinity tags
 * are a hint to the scheduler for thread placement.
 *
 * The namespace of affinity tags is generally local to one task. However,
 * a child task created after the assignment of affinity tags by its parent
 * will share that namespace. In particular, a family of forked processes
 * may be created with a shared affinity namespace.
 *
 * Parameters:
 * tag: The affinity set identifier.
 */

#define THREAD_AFFINITY_POLICY          4

struct thread_affinity_policy {
	integer_t       affinity_tag;
};

#define THREAD_AFFINITY_TAG_NULL                0

typedef struct thread_affinity_policy           thread_affinity_policy_data_t;
typedef struct thread_affinity_policy           *thread_affinity_policy_t;

#define THREAD_AFFINITY_POLICY_COUNT    ((mach_msg_type_number_t) \
	(sizeof (thread_affinity_policy_data_t) / sizeof (integer_t)))

/*
 * THREAD_BACKGROUND_POLICY:
 */

#define THREAD_BACKGROUND_POLICY        5

struct thread_background_policy {
	integer_t       priority;
};

#define THREAD_BACKGROUND_POLICY_DARWIN_BG 0x1000

typedef struct thread_background_policy         thread_background_policy_data_t;
typedef struct thread_background_policy         *thread_background_policy_t;

#define THREAD_BACKGROUND_POLICY_COUNT  ((mach_msg_type_number_t) \
	(sizeof (thread_background_policy_data_t) / sizeof (integer_t)))


#define THREAD_LATENCY_QOS_POLICY       7
typedef integer_t       thread_latency_qos_t;

struct thread_latency_qos_policy {
	thread_latency_qos_t thread_latency_qos_tier;
};

typedef struct thread_latency_qos_policy        thread_latency_qos_policy_data_t;
typedef struct thread_latency_qos_policy        *thread_latency_qos_policy_t;

#define THREAD_LATENCY_QOS_POLICY_COUNT ((mach_msg_type_number_t)       \
	    (sizeof (thread_latency_qos_policy_data_t) / sizeof (integer_t)))

#define THREAD_THROUGHPUT_QOS_POLICY    8
typedef integer_t       thread_throughput_qos_t;

struct thread_throughput_qos_policy {
	thread_throughput_qos_t thread_throughput_qos_tier;
};

typedef struct thread_throughput_qos_policy     thread_throughput_qos_policy_data_t;
typedef struct thread_throughput_qos_policy     *thread_throughput_qos_policy_t;

#define THREAD_THROUGHPUT_QOS_POLICY_COUNT      ((mach_msg_type_number_t) \
	    (sizeof (thread_throughput_qos_policy_data_t) / sizeof (integer_t)))

#ifdef PRIVATE

/*
 * THREAD_POLICY_STATE:
 */
#define THREAD_POLICY_STATE             6

#define THREAD_POLICY_STATE_FLAG_STATIC_PARAM   0x1

struct thread_policy_state {
	integer_t requested;
	integer_t effective;
	integer_t pending;
	integer_t flags;
	uint64_t thps_requested_policy;
	uint64_t thps_effective_policy;
	uint32_t thps_user_promotions;
	uint32_t thps_user_promotion_basepri;
	uint32_t thps_ipc_overrides;
	uint32_t reserved32;
	uint64_t reserved[2];
};

typedef struct thread_policy_state              thread_policy_state_data_t;
typedef struct thread_policy_state              *thread_policy_state_t;

#define THREAD_POLICY_STATE_COUNT       ((mach_msg_type_number_t) \
	(sizeof (thread_policy_state_data_t) / sizeof (integer_t)))

/*
 * THREAD_QOS_POLICY:
 */
#define THREAD_QOS_POLICY               9
#define THREAD_QOS_POLICY_OVERRIDE      10

typedef uint8_t thread_qos_t;
#define THREAD_QOS_UNSPECIFIED          0
#define THREAD_QOS_DEFAULT              THREAD_QOS_UNSPECIFIED  /* Temporary rename */
#define THREAD_QOS_MAINTENANCE          1
#define THREAD_QOS_BACKGROUND           2
#define THREAD_QOS_UTILITY              3
#define THREAD_QOS_LEGACY               4       /* i.e. default workq threads */
#define THREAD_QOS_USER_INITIATED       5
#define THREAD_QOS_USER_INTERACTIVE     6

#define THREAD_QOS_LAST                 7

#define THREAD_QOS_MIN_TIER_IMPORTANCE  (-15)

/*
 * Overrides are inputs to the task/thread policy engine that
 * temporarily elevate the effective QoS of a thread without changing
 * its steady-state (and round-trip-able) requested QoS. The
 * interfaces into the kernel allow the caller to associate a resource
 * and type that describe the reason/lifecycle of the override. For
 * instance, a contended pthread_mutex_t held by a UTILITY thread
 * might get an override to USER_INTERACTIVE, with the resource
 * being the userspace address of the pthread_mutex_t. When the
 * owning thread releases that resource, it can call into the
 * task policy subsystem to drop the override because of that resource,
 * although if more contended locks are held by the thread, the
 * effective QoS may remain overridden for longer.
 *
 * THREAD_QOS_OVERRIDE_TYPE_PTHREAD_MUTEX is used for contended
 * pthread_mutex_t's via the pthread kext. The holder gets an override
 * with resource=&mutex and a count of 1 by the initial contender.
 * Subsequent contenders raise the QoS value, until the holder
 * decrements the count to 0 and the override is released.
 *
 * THREAD_QOS_OVERRIDE_TYPE_PTHREAD_RWLOCK is unimplemented and has no
 * specified semantics.
 *
 * THREAD_QOS_OVERRIDE_TYPE_PTHREAD_EXPLICIT_OVERRIDE are explicitly
 * paired start/end overrides on a target thread. The resource can
 * either be a memory allocation in userspace, or the pthread_t of the
 * overrider if no allocation was used.
 *
 * THREAD_QOS_OVERRIDE_TYPE_WILDCARD is a catch-all which will reset every
 * resource matching the resource value.  Passing
 * THREAD_QOS_OVERRIDE_RESOURCE_WILDCARD as well will reset everything.
 */

#define THREAD_QOS_OVERRIDE_TYPE_UNKNOWN                        (0)
#define THREAD_QOS_OVERRIDE_TYPE_PTHREAD_MUTEX                  (1)
#define THREAD_QOS_OVERRIDE_TYPE_PTHREAD_RWLOCK                 (2)
#define THREAD_QOS_OVERRIDE_TYPE_PTHREAD_EXPLICIT_OVERRIDE      (3)
#define THREAD_QOS_OVERRIDE_TYPE_WILDCARD                       (5)

/* A special resource value to indicate a resource wildcard */
#define THREAD_QOS_OVERRIDE_RESOURCE_WILDCARD (~((user_addr_t)0))

struct thread_qos_policy {
	integer_t qos_tier;
	integer_t tier_importance;
};

typedef struct thread_qos_policy       thread_qos_policy_data_t;
typedef struct thread_qos_policy      *thread_qos_policy_t;

#define THREAD_QOS_POLICY_COUNT    ((mach_msg_type_number_t) \
	(sizeof (thread_qos_policy_data_t) / sizeof (integer_t)))

#endif /* PRIVATE */

#ifdef PRIVATE

/*
 * Internal bitfields are privately exported for revlocked tracing tools like msa to decode tracepoints.
 *
 * These struct definitions *will* change in the future.
 * When they do, we will update THREAD_POLICY_INTERNAL_STRUCT_VERSION.
 */

#define THREAD_POLICY_INTERNAL_STRUCT_VERSION 5

// legacy names
#define thrp_qos_ipc_override   thrp_qos_kevent_override

struct thread_requested_policy {
	uint64_t        thrp_int_darwinbg       :1,     /* marked as darwinbg via setpriority */
	    thrp_ext_darwinbg       :1,
	    thrp_int_iotier         :2,                 /* IO throttle tier */
	    thrp_ext_iotier         :2,
	    thrp_int_iopassive      :1,                 /* should IOs cause lower tiers to be throttled */
	    thrp_ext_iopassive      :1,
	    thrp_latency_qos        :3,                 /* Timer latency QoS */
	    thrp_through_qos        :3,                 /* Computation throughput QoS */

	    thrp_pidbind_bg         :1,                 /* task i'm bound to is marked 'watchbg' */
	    thrp_qos                :3,                 /* thread qos class */
	    thrp_qos_relprio        :4,                 /* thread qos relative priority (store as inverse, -10 -> 0xA) */
	    thrp_qos_override       :3,                 /* thread qos class override */
	    thrp_qos_promote        :3,                 /* thread qos class from promotion */
	    thrp_qos_kevent_override:3,                 /* thread qos class from kevent override */
	    thrp_terminated         :1,                 /* heading for termination */
	    thrp_qos_sync_ipc_override:3,               /* now unused */
	    thrp_qos_workq_override :3,                 /* thread qos class override (workq) */
	    thrp_qos_wlsvc_override :3,                 /* workloop servicer qos class override */

	    thrp_reserved           :23;
};

struct thread_effective_policy {
	uint64_t        thep_darwinbg           :1,     /* marked as 'background', and sockets are marked bg when created */
	    thep_io_tier            :2,                 /* effective throttle tier */
	    thep_io_passive         :1,                 /* should IOs cause lower tiers to be throttled */
	    thep_all_sockets_bg     :1,                 /* All existing sockets in process are marked as bg (thread: all created by thread) */
	    thep_new_sockets_bg     :1,                 /* Newly created sockets should be marked as bg */
	    thep_terminated         :1,                 /* all throttles have been removed for quick exit or SIGTERM handling */
	    thep_qos_ui_is_urgent   :1,                 /* bump UI-Interactive QoS up to the urgent preemption band */
	    thep_latency_qos        :3,                 /* Timer latency QoS level */
	    thep_through_qos        :3,                 /* Computation throughput QoS level */

	    thep_qos                :3,                 /* thread qos class */
	    thep_qos_relprio        :4,                 /* thread qos relative priority (store as inverse, -10 -> 0xA) */
	    thep_qos_promote        :3,                 /* thread qos class used for promotion */

	    thep_reserved           :40;
};

#endif /* PRIVATE */


#endif  /* _MACH_THREAD_POLICY_H_ */

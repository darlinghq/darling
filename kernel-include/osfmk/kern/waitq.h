#ifndef _WAITQ_H_
#define _WAITQ_H_
/*
 * Copyright (c) 2014-2015 Apple Computer, Inc. All rights reserved.
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
#ifdef  KERNEL_PRIVATE

#include <mach/mach_types.h>
#include <mach/sync_policy.h>
#include <mach/kern_return.h>           /* for kern_return_t */

#include <kern/kern_types.h>            /* for wait_queue_t */
#include <kern/queue.h>
#include <kern/assert.h>

#include <sys/cdefs.h>

#ifdef XNU_KERNEL_PRIVATE
/* priority queue static asserts fail for __ARM64_ARCH_8_32__ kext builds */
#include <kern/priority_queue.h>
#endif /* XNU_KERNEL_PRIVATE */

/*
 * Constants and types used in the waitq APIs
 */
#define WAITQ_ALL_PRIORITIES   (-1)
#define WAITQ_PROMOTE_PRIORITY (-2)
#define WAITQ_PROMOTE_ON_WAKE  (-3)

typedef enum e_waitq_lock_state {
	WAITQ_KEEP_LOCKED    = 0x01,
	WAITQ_UNLOCK         = 0x02,
	WAITQ_SHOULD_LOCK    = 0x04,
	WAITQ_ALREADY_LOCKED = 0x08,
	WAITQ_DONT_LOCK      = 0x10,
} waitq_lock_state_t;

/* Opaque sizes and alignment used for struct verification */
#if __arm__ || __arm64__
	#define WQ_OPAQUE_ALIGN   __BIGGEST_ALIGNMENT__
	#define WQS_OPAQUE_ALIGN  __BIGGEST_ALIGNMENT__
	#if __arm__
		#define WQ_OPAQUE_SIZE   32
		#define WQS_OPAQUE_SIZE  48
	#else
		#define WQ_OPAQUE_SIZE   40
		#define WQS_OPAQUE_SIZE  56
	#endif
#elif __x86_64__
	#define WQ_OPAQUE_ALIGN   8
	#define WQS_OPAQUE_ALIGN  8
	#define WQ_OPAQUE_SIZE   48
	#define WQS_OPAQUE_SIZE  64
#else
	#error Unknown size requirement
#endif

#ifdef MACH_KERNEL_PRIVATE

#include <kern/spl.h>
#include <kern/simple_lock.h>

#include <machine/cpu_number.h>
#include <machine/machine_routines.h> /* machine_timeout_suspended() */

/*
 * The event mask is of 57 bits on 64 bit architeture and 25 bits on
 * 32 bit architecture and so we calculate its size using sizeof(long).
 * If the bitfield for wq_type and wq_fifo is changed, then value of
 * EVENT_MASK_BITS will also change.
 *
 * New plan: this is an optimization anyway, so I'm stealing 32bits
 * from the mask to shrink the waitq object even further.
 */
#define _EVENT_MASK_BITS   ((sizeof(uint32_t) * 8) - 7)


enum waitq_type {
	WQT_INVALID = 0,
	WQT_TSPROXY = 0x1,
	WQT_QUEUE   = 0x2,
	WQT_SET     = 0x3,
};

#if CONFIG_WAITQ_STATS
#define NWAITQ_BTFRAMES 5
struct wq_stats {
	uint64_t waits;
	uint64_t wakeups;
	uint64_t clears;
	uint64_t failed_wakeups;

	uintptr_t last_wait[NWAITQ_BTFRAMES];
	uintptr_t last_wakeup[NWAITQ_BTFRAMES];
	uintptr_t last_failed_wakeup[NWAITQ_BTFRAMES];
};
#endif

/*
 *	struct waitq
 *
 *	This is the definition of the common event wait queue
 *	that the scheduler APIs understand.  It is used
 *	internally by the gerneralized event waiting mechanism
 *	(assert_wait), and also for items that maintain their
 *	own wait queues (such as ports and semaphores).
 *
 *	It is not published to other kernel components.
 *
 *	NOTE:  Hardware locks are used to protect event wait
 *	queues since interrupt code is free to post events to
 *	them.
 */
struct waitq {
	uint32_t /* flags */
	    waitq_type:2,        /* only public field */
	    waitq_fifo:1,        /* fifo wakeup policy? */
	    waitq_prepost:1,     /* waitq supports prepost? */
	    waitq_irq:1,         /* waitq requires interrupts disabled */
	    waitq_isvalid:1,     /* waitq structure is valid */
	    waitq_turnstile:1,   /* waitq is embedded in a turnstile */
	    waitq_eventmask:_EVENT_MASK_BITS;
	/* the wait queue set (set-of-sets) to which this queue belongs */
#if __arm64__
	hw_lock_bit_t   waitq_interlock;        /* interlock */
#else
	hw_lock_data_t  waitq_interlock;        /* interlock */
#endif /* __arm64__ */

	uint64_t waitq_set_id;
	uint64_t waitq_prepost_id;
	union {
		queue_head_t            waitq_queue;        /* queue of elements */
		struct priority_queue   waitq_prio_queue;   /* priority ordered queue of elements */
		struct {
			struct turnstile   *waitq_ts;           /* turnstile for WQT_TSPROXY */
			void               *waitq_tspriv;       /* private field for clients use */
		};
	};
};

static_assert(sizeof(struct waitq) == WQ_OPAQUE_SIZE, "waitq structure size mismatch");
static_assert(__alignof(struct waitq) == WQ_OPAQUE_ALIGN, "waitq structure alignment mismatch");

/*
 *	struct waitq_set
 *
 *	This is the common definition for a set wait queue.
 */
struct waitq_set {
	struct waitq wqset_q;
	uint64_t     wqset_id;
	union {
		uint64_t     wqset_prepost_id;
		void        *wqset_prepost_hook;
	};
};

#define WQSET_NOT_LINKED       ((uint64_t)(~0))
static_assert(sizeof(struct waitq_set) == WQS_OPAQUE_SIZE, "waitq_set structure size mismatch");
static_assert(__alignof(struct waitq_set) == WQS_OPAQUE_ALIGN, "waitq_set structure alignment mismatch");

extern void waitq_bootstrap(void);

#define waitq_is_queue(wq) \
	((wq)->waitq_type == WQT_QUEUE)

#define waitq_is_turnstile_proxy(wq) \
	((wq)->waitq_type == WQT_TSPROXY)

#define waitq_is_turnstile_queue(wq) \
	(((wq)->waitq_irq) && (wq)->waitq_turnstile)

#define waitq_is_set(wq) \
	((wq)->waitq_type == WQT_SET && ((struct waitq_set *)(wq))->wqset_id != 0)

#define waitqs_is_set(wqs) \
	(((wqs)->wqset_q.waitq_type == WQT_SET) && ((wqs)->wqset_id != 0))

#define waitq_valid(wq) \
	((wq) != NULL && (wq)->waitq_isvalid)

#define waitqs_is_linked(wqs) \
	(((wqs)->wqset_id != WQSET_NOT_LINKED) && ((wqs)->wqset_id != 0))

/*
 * Invalidate a waitq. The only valid waitq functions to call after this are:
 *      waitq_deinit()
 *      waitq_set_deinit()
 */
extern void waitq_invalidate_locked(struct waitq *wq);

extern lck_grp_t waitq_lck_grp;

#if __arm64__

#define waitq_held(wq) \
	(hw_lock_bit_held(&(wq)->waitq_interlock, LCK_ILOCK))

#define waitq_lock_try(wq) \
	(hw_lock_bit_try(&(wq)->waitq_interlock, LCK_ILOCK, &waitq_lck_grp))

#else

#define waitq_held(wq) \
	(hw_lock_held(&(wq)->waitq_interlock))

#define waitq_lock_try(wq) \
	(hw_lock_try(&(wq)->waitq_interlock, &waitq_lck_grp))

#endif /* __arm64__ */

#define waitq_wait_possible(thread) \
	((thread)->waitq == NULL)

extern void waitq_lock(struct waitq *wq);

#define waitq_set_lock(wqs)             waitq_lock(&(wqs)->wqset_q)
#define waitq_set_unlock(wqs)           waitq_unlock(&(wqs)->wqset_q)
#define waitq_set_lock_try(wqs)         waitq_lock_try(&(wqs)->wqset_q)
#define waitq_set_can_prepost(wqs)      (waitqs_is_set(wqs) && \
	                                 (wqs)->wqset_q.waitq_prepost)
#define waitq_set_maybe_preposted(wqs)  ((wqs)->wqset_q.waitq_prepost && \
	                                 (wqs)->wqset_prepost_id > 0)
#define waitq_set_has_prepost_hook(wqs) (waitqs_is_set(wqs) && \
	                                 !((wqs)->wqset_q.waitq_prepost) && \
	                                 (wqs)->wqset_prepost_hook)

/* assert intent to wait on a locked wait queue */
extern wait_result_t waitq_assert_wait64_locked(struct waitq *waitq,
    event64_t wait_event,
    wait_interrupt_t interruptible,
    wait_timeout_urgency_t urgency,
    uint64_t deadline,
    uint64_t leeway,
    thread_t thread);

/* pull a thread from its wait queue */
extern int waitq_pull_thread_locked(struct waitq *waitq, thread_t thread);

/* wakeup all threads waiting for a particular event on locked queue */
extern kern_return_t waitq_wakeup64_all_locked(struct waitq *waitq,
    event64_t wake_event,
    wait_result_t result,
    uint64_t *reserved_preposts,
    int priority,
    waitq_lock_state_t lock_state);

/* wakeup one thread waiting for a particular event on locked queue */
extern kern_return_t waitq_wakeup64_one_locked(struct waitq *waitq,
    event64_t wake_event,
    wait_result_t result,
    uint64_t *reserved_preposts,
    int priority,
    waitq_lock_state_t lock_state);

/* return identity of a thread awakened for a particular <wait_queue,event> */
extern thread_t
waitq_wakeup64_identify_locked(struct waitq     *waitq,
    event64_t        wake_event,
    wait_result_t    result,
    spl_t            *spl,
    uint64_t         *reserved_preposts,
    int              priority,
    waitq_lock_state_t lock_state);

/* wakeup thread iff its still waiting for a particular event on locked queue */
extern kern_return_t waitq_wakeup64_thread_locked(struct waitq *waitq,
    event64_t wake_event,
    thread_t thread,
    wait_result_t result,
    waitq_lock_state_t lock_state);

/* clear all preposts generated by the given waitq */
extern int waitq_clear_prepost_locked(struct waitq *waitq);

/* clear all preposts from the given wait queue set */
extern void waitq_set_clear_preposts_locked(struct waitq_set *wqset);

/* unlink the given waitq from all sets - returns unlocked */
extern kern_return_t waitq_unlink_all_unlock(struct waitq *waitq);

/* unlink the given waitq set from all waitqs and waitq sets - returns unlocked */
extern kern_return_t waitq_set_unlink_all_unlock(struct waitq_set *wqset);



/*
 * clear a thread's boosted priority
 * (given via WAITQ_PROMOTE_PRIORITY in the wakeup function)
 */
extern void waitq_clear_promotion_locked(struct waitq *waitq,
    thread_t thread);

/*
 * waitq iteration
 */

enum waitq_iteration_constant {
	WQ_ITERATE_DROPPED             = -4,
	WQ_ITERATE_INVALID             = -3,
	WQ_ITERATE_ABORTED             = -2,
	WQ_ITERATE_FAILURE             = -1,
	WQ_ITERATE_SUCCESS             =  0,
	WQ_ITERATE_CONTINUE            =  1,
	WQ_ITERATE_BREAK               =  2,
	WQ_ITERATE_BREAK_KEEP_LOCKED   =  3,
	WQ_ITERATE_INVALIDATE_CONTINUE =  4,
	WQ_ITERATE_RESTART             =  5,
	WQ_ITERATE_FOUND               =  6,
	WQ_ITERATE_UNLINKED            =  7,
};

/* callback invoked with both 'waitq' and 'wqset' locked */
typedef int (*waitq_iterator_t)(void *ctx, struct waitq *waitq,
    struct waitq_set *wqset);

/* iterate over all sets to which waitq belongs */
extern int waitq_iterate_sets(struct waitq *waitq, void *ctx,
    waitq_iterator_t it);

/* iterator over all waitqs that have preposted to wqset */
extern int waitq_set_iterate_preposts(struct waitq_set *wqset,
    void *ctx, waitq_iterator_t it);

/*
 * prepost reservation
 */
extern uint64_t waitq_prepost_reserve(struct waitq *waitq, int extra,
    waitq_lock_state_t lock_state);

extern void waitq_prepost_release_reserve(uint64_t id);

#else /* !MACH_KERNEL_PRIVATE */

/*
 * The opaque waitq structure is here mostly for AIO and selinfo,
 * but could potentially be used by other BSD subsystems.
 */
struct waitq { char opaque[WQ_OPAQUE_SIZE]; } __attribute__((aligned(WQ_OPAQUE_ALIGN)));
struct waitq_set { char opaque[WQS_OPAQUE_SIZE]; } __attribute__((aligned(WQS_OPAQUE_ALIGN)));

#endif  /* MACH_KERNEL_PRIVATE */


__BEGIN_DECLS

/*
 * waitq init
 */
extern kern_return_t waitq_init(struct waitq *waitq, int policy);
extern void waitq_deinit(struct waitq *waitq);

/*
 * global waitqs
 */
extern struct waitq *_global_eventq(char *event, size_t event_length);
#define global_eventq(event) _global_eventq((char *)&(event), sizeof(event))

extern struct waitq *global_waitq(int index);

typedef uint16_t waitq_set_prepost_hook_t;

/*
 * set alloc/init/free
 */
extern struct waitq_set *waitq_set_alloc(int policy,
    waitq_set_prepost_hook_t *prepost_hook);

extern kern_return_t waitq_set_init(struct waitq_set *wqset,
    int policy, uint64_t *reserved_link,
    waitq_set_prepost_hook_t *prepost_hook);

extern void waitq_set_deinit(struct waitq_set *wqset);

extern kern_return_t waitq_set_free(struct waitq_set *wqset);

#if DEVELOPMENT || DEBUG
extern int sysctl_helper_waitq_set_nelem(void);
#if CONFIG_WAITQ_DEBUG
extern uint64_t wqset_id(struct waitq_set *wqset);

struct waitq *wqset_waitq(struct waitq_set *wqset);
#endif /* CONFIG_WAITQ_DEBUG */
#endif /* DEVELOPMENT || DEBUG */


/*
 * set membership
 */
extern uint64_t waitq_link_reserve(struct waitq *waitq);
extern void waitq_set_lazy_init_link(struct waitq_set *wqset);
extern boolean_t waitq_set_should_lazy_init_link(struct waitq_set *wqset);

extern void waitq_link_release(uint64_t id);

extern boolean_t waitq_member(struct waitq *waitq, struct waitq_set *wqset);

/* returns true if the waitq is in at least 1 set */
extern boolean_t waitq_in_set(struct waitq *waitq);


/* on success, consumes an reserved_link reference */
extern kern_return_t waitq_link(struct waitq *waitq,
    struct waitq_set *wqset,
    waitq_lock_state_t lock_state,
    uint64_t *reserved_link);

extern kern_return_t waitq_unlink(struct waitq *waitq, struct waitq_set *wqset);

extern kern_return_t waitq_unlink_all(struct waitq *waitq);

extern kern_return_t waitq_set_unlink_all(struct waitq_set *wqset);

/*
 * preposts
 */
extern void waitq_clear_prepost(struct waitq *waitq);

extern void waitq_set_clear_preposts(struct waitq_set *wqset);

/*
 * interfaces used primarily by the select/kqueue subsystems
 */
extern uint64_t waitq_get_prepost_id(struct waitq *waitq);
extern void     waitq_unlink_by_prepost_id(uint64_t wqp_id, struct waitq_set *wqset);
extern struct waitq *waitq_lock_by_prepost_id(uint64_t wqp_id);

/*
 * waitq attributes
 */
extern int waitq_is_valid(struct waitq *waitq);

extern int waitq_set_is_valid(struct waitq_set *wqset);

extern int waitq_is_global(struct waitq *waitq);

extern int waitq_irq_safe(struct waitq *waitq);

#if CONFIG_WAITQ_STATS
/*
 * waitq statistics
 */
#define WAITQ_STATS_VERSION 1
struct wq_table_stats {
	uint32_t version;
	uint32_t table_elements;
	uint32_t table_used_elems;
	uint32_t table_elem_sz;
	uint32_t table_slabs;
	uint32_t table_slab_sz;

	uint64_t table_num_allocs;
	uint64_t table_num_preposts;
	uint64_t table_num_reservations;

	uint64_t table_max_used;
	uint64_t table_avg_used;
	uint64_t table_max_reservations;
	uint64_t table_avg_reservations;
};

extern void waitq_link_stats(struct wq_table_stats *stats);
extern void waitq_prepost_stats(struct wq_table_stats *stats);
#endif /* CONFIG_WAITQ_STATS */

/*
 *
 * higher-level waiting APIs
 *
 */

/* assert intent to wait on <waitq,event64> pair */
extern wait_result_t waitq_assert_wait64(struct waitq *waitq,
    event64_t wait_event,
    wait_interrupt_t interruptible,
    uint64_t deadline);

extern wait_result_t waitq_assert_wait64_leeway(struct waitq *waitq,
    event64_t wait_event,
    wait_interrupt_t interruptible,
    wait_timeout_urgency_t urgency,
    uint64_t deadline,
    uint64_t leeway);

/* wakeup the most appropriate thread waiting on <waitq,event64> pair */
extern kern_return_t waitq_wakeup64_one(struct waitq *waitq,
    event64_t wake_event,
    wait_result_t result,
    int priority);

/* wakeup all the threads waiting on <waitq,event64> pair */
extern kern_return_t waitq_wakeup64_all(struct waitq *waitq,
    event64_t wake_event,
    wait_result_t result,
    int priority);

#ifdef  XNU_KERNEL_PRIVATE

/* wakeup a specified thread iff it's waiting on <waitq,event64> pair */
extern kern_return_t waitq_wakeup64_thread(struct waitq *waitq,
    event64_t wake_event,
    thread_t thread,
    wait_result_t result);

/* return a reference to the thread that was woken up */
extern thread_t
waitq_wakeup64_identify(struct waitq    *waitq,
    event64_t       wake_event,
    wait_result_t   result,
    int             priority);

/* take the waitq lock */
extern void waitq_unlock(struct waitq *wq);

#endif /* XNU_KERNEL_PRIVATE */

__END_DECLS

#endif  /* KERNEL_PRIVATE */
#endif  /* _WAITQ_H_ */

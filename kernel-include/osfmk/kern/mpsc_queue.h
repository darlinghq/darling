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

#ifndef _KERN_MPSC_QUEUE_H_
#define _KERN_MPSC_QUEUE_H_

#ifdef XNU_KERNEL_PRIVATE

#include <machine/atomic.h>
#include <kern/macro_help.h>
#include <kern/thread_call.h>

#endif // XNU_KERNEL_PRIVATE

#include <sys/cdefs.h>

__BEGIN_DECLS

/*!
 * @typedef struct mpsc_queue_chain
 *
 * @brief
 * Type for the intrusive linkage used by MPSC queues.
 */
typedef struct mpsc_queue_chain {
	struct mpsc_queue_chain *_Atomic mpqc_next;
} *mpsc_queue_chain_t;

/*!
 * @typedef struct mpsc_queue_head
 *
 * @brief
 * The type for a multi-producer single-consumer queue.
 *
 * @discussion
 * MPSC queues allow for producers to not be affected by other producers or the
 * consumer. Which means in turn that having producers in interrupt context
 * does not require that other producers disable interrupts like a traditional
 * spinlock based approach would require.
 *
 * These queues shine when data is produced from the entire system and is
 * consumed from a single serial context (logging, tracing, ...).
 * mpsc_daemon_queue_t is provided as a fully ready/easy-to-use pre-packaged
 * solution for these common use cases.
 *
 * - mpsc_queue_append() can be used to append a single item
 * - mpsc_queue_append_list() can be used to append a batch of items at once.
 *
 * Functions for the consumer side assume proper serialization that is not
 * provided by the MPSC queue itself. Dequeuing doesn't require preemption
 * to be disabled.
 *
 * <h2>Algorithm</h2>
 *
 * The base of the enqueue algorithm is a single atomic exchange (first half,
 * called __mpsc_queue_append_update_tail) and a list fixup (2nd half, called
 * __mpsc_queue_append_update_prev).
 *
 * Graphically, enqueuing `X` looks like this, with each step being done
 * atomically (for the empty queue case, `tail` points to `head`):
 *
 *     | orig state          | update_tail         | update_prev         |
 *     +---------------------+---------------------+---------------------+
 *     |                     |                     |                     |
 *     | head -> e1 -> e2 -. | head -> e1 -> e2 -. | head -> e1 -> e2 -. |
 *     |                   | |                   | |                   | |
 *     |         ,- ... <--' |         ,- ... <--' |         ,- ... <--' |
 *     |         |           |         |           |         |           |
 *     |         v           |         v           |         v           |
 *     | tail -> eN -> NULL  | tail    eN -> NULL  | tail    eN          |
 *     |                     |   |                 |   |     |           |
 *     |                     |   |                 |   |     v           |
 *     |         X -> NULL   |   `---> X -> NULL   |   '---> X -> NULL   |
 *     |                     |                     |                     |
 *     +---------------------+---------------------+---------------------+
 *
 *
 * There is a small 1-instruction gap of inconsistency which makes the chosen
 * algorithm non linearizable, and requires enqueuers to disable preemption
 * during the enqueue so as not to starve the consumer forever.
 *
 * As far as memory visibility is concerned, enqueuing uses a release fence in
 * update_tail which pairs with memory fences in mpsc_queue_dequeue_batch().
 *
 * Note: as far as the data structure in memory, its layout is equivalent to
 *       a BSD <sys/queue.h> STAILQ. However because of this inconsistency
 *       window and memory ordering concerns, it is incorrect to use STAILQ
 *       macros on an MPSC queue.
 */
typedef struct mpsc_queue_head {
	struct mpsc_queue_chain mpqh_head;
	struct mpsc_queue_chain *_Atomic mpqh_tail;
} *mpsc_queue_head_t;

/*!
 * @macro MPSC_QUEUE_INITIALIZER
 *
 * @brief
 * Macro to use in static initializers for mpsc queues.
 *
 * @param head
 * The name of the variable to initialize.
 */
#define MPSC_QUEUE_INITIALIZER(head)   { .mpqh_tail = &(head).mpqh_head }

#ifdef XNU_KERNEL_PRIVATE

/*!
 * @function mpsc_queue_init
 *
 * @brief
 * Dynamically initialize an mpsc queue.
 *
 * @discussion
 * This initialization assumes that the object holding the queue head
 * is initialized before it can be made visible to other threads/cores.
 *
 * @param q
 * The queue to initialize.
 */
static inline void
mpsc_queue_init(mpsc_queue_head_t q)
{
	os_atomic_init(&q->mpqh_head.mpqc_next, NULL);
	os_atomic_init(&q->mpqh_tail, &q->mpqh_head);
}

/*!
 * @typedef enum mpsc_queue_options
 */
typedef enum mpsc_queue_options {
	MPSC_QUEUE_NONE                = 0,
	MPSC_QUEUE_DISABLE_PREEMPTION  = 1 << 0,
} mpsc_queue_options_t;

/*!
 * @const MPSC_QUEUE_NOTQUEUED_MARKER
 *
 * @brief
 * Magical marker that implementations can use to poison the chain pointer of
 * elements not on any MPSC queue.
 */
#define MPSC_QUEUE_NOTQUEUED_MARKER ((mpsc_queue_chain_t)~0ul)

/*!
 * @macro mpsc_queue_element
 *
 * @brief
 * Macro to find the pointer of an element back from its MPSC chain linkage.
 */
#define mpsc_queue_element(ptr, type, field) __container_of(ptr, type, field)


#pragma mark Advanced Multi Producer calls

/**
 * @function __mpsc_queue_append_update_tail
 *
 * @brief
 * First half of the enqueue operation onto a multi-producer single-consumer
 * queue.
 *
 * @discussion
 * This function is available for algorithms that need to do things (such as
 * taking a refcount) before calling __mpsc_queue_append_update_prev().
 *
 * Preemption should be disabled before calling
 * __mpsc_queue_append_update_tail(), and until
 * __mpsc_queue_append_update_prev() has returned.
 *
 * @param q
 * The queue to update.
 *
 * @param elm
 * The element to append to `q`.
 *
 * @returns
 * A token to later pass to __mpsc_queue_append_update_prev()
 * to complete the enqueue.
 */
static inline mpsc_queue_chain_t
__mpsc_queue_append_update_tail(mpsc_queue_head_t q, mpsc_queue_chain_t elm)
{
	os_atomic_store(&elm->mpqc_next, NULL, relaxed);
	return os_atomic_xchg(&q->mpqh_tail, elm, release);
}

/**
 * @function __mpsc_queue_append_was_empty
 *
 * @brief
 * Tests whether the queue was empty at the time
 * __mpsc_queue_append_update_tail() was called.
 *
 * @param q
 * The queue to test emptiness for.
 *
 * @param prev
 * The token returned by __mpsc_queue_append_update_tail().
 *
 * @returns
 * Whether the queue was empty (true) or not (false).
 */
static inline bool
__mpsc_queue_append_was_empty(mpsc_queue_head_t q, mpsc_queue_chain_t prev)
{
	return &q->mpqh_head == prev;
}

/**
 * @function __mpsc_queue_append_update_prev
 *
 * @brief
 * Second half of the enqueue operation onto a multi-producer single-consumer
 * queue.
 *
 * @discussion
 * This function is available for algorithms that need to do things (such as
 * taking a refcount) before calling __mpsc_queue_append_update_prev().
 *
 * Preemption should be disabled before calling
 * __mpsc_queue_append_update_tail(), and until
 * __mpsc_queue_append_update_prev() has returned.
 *
 * @param prev
 * The token returned by __mpsc_queue_append_update_tail().
 *
 * @param elm
 * The element to append to the queue.
 */
static inline void
__mpsc_queue_append_update_prev(mpsc_queue_chain_t prev, mpsc_queue_chain_t elm)
{
	os_atomic_store(&prev->mpqc_next, elm, relaxed);
}


#pragma mark Multi Producer calls

/**
 * @function mpsc_queue_append_list
 *
 * @brief
 * Enqueues a list of elements onto a queue.
 *
 * @discussion
 * This enqueues a list that has to be fully formed from `first` to `last`
 * at the end of `q`.
 *
 * Preemption should be disabled when calling mpsc_queue_append_list().
 *
 * @param q
 * The queue to update.
 *
 * @param first
 * The first of the list elements being appended.
 *
 * @param last
 * The last of the list elements being appended.
 */
static inline bool
mpsc_queue_append_list(mpsc_queue_head_t q, mpsc_queue_chain_t first,
    mpsc_queue_chain_t last)
{
	mpsc_queue_chain_t prev = __mpsc_queue_append_update_tail(q, last);
	__mpsc_queue_append_update_prev(prev, first);
	return __mpsc_queue_append_was_empty(q, prev);
}

/**
 * @function __mpsc_queue_append_update_tail
 *
 * @brief
 * Enqueues an element onto a queue.
 *
 * @discussion
 * Preemption should be disabled when calling mpsc_queue_append().
 *
 * @param q    the queue to update
 * @param elm  the element to append
 */
static inline bool
mpsc_queue_append(mpsc_queue_head_t q, mpsc_queue_chain_t elm)
{
	return mpsc_queue_append_list(q, elm, elm);
}


#pragma mark Single Consumer calls

/**
 * @function mpsc_queue_dequeue_batch()
 *
 * @brief
 * Atomically empty a queue at once and return the batch head and tail.
 *
 * @discussion
 * Consumer function, must be called in a serialized way with respect to any
 * other consumer function.
 *
 * @param q
 * The queue
 *
 * @param tail
 * An out pointer filled with the last element captured.
 *
 * @param dependency
 * A dependency token (to rely on consume / hardware dependencies)
 * When not trying to take advantage of hardware dependencies, just pass NULL.
 *
 * @returns
 * The first element of the batch if any, or NULL the queue was empty.
 */
mpsc_queue_chain_t
mpsc_queue_dequeue_batch(mpsc_queue_head_t q, mpsc_queue_chain_t *tail,
    os_atomic_dependency_t dependency);

/**
 * @function mpsc_queue_batch_next()
 *
 * @brief
 * Function used to consume an element from a batch dequeued with
 * mpsc_queue_dequeue_batch().
 *
 * @discussion
 * Once a batch has been dequeued, there is no need to hold the consumer lock
 * anymore to consume it.
 *
 * mpsc_queue_batch_foreach_safe() is the preferred interface to consume
 * the whole batch.
 *
 * @param cur
 * The current inspected element of the batch (must be the batch head or
 * a value returned by mpsc_queue_batch_next()).
 *
 * @param tail
 * The last element of the batch.
 *
 * @returns
 * The next element if any.
 */
mpsc_queue_chain_t
mpsc_queue_batch_next(mpsc_queue_chain_t cur, mpsc_queue_chain_t tail);

/**
 * @macro mpsc_queue_batch_foreach_safe
 *
 * @brief
 * Macro used to enumerate a batch dequeued with mpsc_queue_dequeue_batch().
 *
 * @param item
 * The item being currently visited.
 *
 * @param head
 * The first element of the batch.
 *
 * @param tail
 * The last element of the batch.
 */
#define mpsc_queue_batch_foreach_safe(item, head, tail) \
	        for (mpsc_queue_chain_t __tmp, __item = (head), __tail = (tail); \
	                        __tmp = mpsc_queue_batch_next(__item, __tail), (item) = __item; \
	                        __item = __tmp)

/**
 * @function mpsc_queue_restore_batch()
 *
 * @brief
 * "Restore"s a batch at the head of the queue.
 *
 * @discussion
 * Consumer function, must be called in a serialized way with respect to any
 * other consumer function.
 *
 * @param q
 * The queue
 *
 * @param first
 * The first element to put back.
 *
 * @param last
 * The last element to put back.
 * It is the responsibility of the caller to ensure the linkages from first to
 * last are properly set up before calling this function.
 */
void
mpsc_queue_restore_batch(mpsc_queue_head_t q, mpsc_queue_chain_t first,
    mpsc_queue_chain_t last);


#pragma mark "GCD"-like facilities

/*!
 * @typedef struct mpsc_daemon_queue
 *
 * @brief
 * Daemon queues are a ready-to use packaging of the low level MPSC queue
 * primitive.
 *
 * @discussion
 * mpsc_queue_t requires handling of state transitions of the queue and
 * dequeuing yourself, which is a non trivial task.
 *
 * Daemon queues are a simple packaged solution that allows for mpsc_queue_t to
 * form hierarchies (mostly for layering purposes), and be serviced at the
 * bottom of such a hierarchy by a thread or a thread call.
 *
 * Daemon queues assume homogenous items, and are setup with an `invoke`
 * callback that is called in the dequeuer on every item as they are dequeued.
 */
typedef struct mpsc_daemon_queue *mpsc_daemon_queue_t;

/*!
 * @typedef struct mpsc_daemon_queue
 *
 * @brief
 * The type for MPSC Daemon Queues invoke callbacks.
 */
typedef void (*mpsc_daemon_invoke_fn_t)(mpsc_queue_chain_t elm,
    mpsc_daemon_queue_t dq);

/*!
 * @enum mpsc_daemon_queue_kind
 *
 * @brief
 * Internal type, not to be used by clients.
 */
typedef enum mpsc_daemon_queue_kind {
	MPSC_QUEUE_KIND_UNKNOWN,
	MPSC_QUEUE_KIND_NESTED,
	MPSC_QUEUE_KIND_THREAD,
	MPSC_QUEUE_KIND_THREAD_CRITICAL,
	MPSC_QUEUE_KIND_THREAD_CALL,
} mpsc_daemon_queue_kind_t;

/*!
 * @enum mpsc_daemon_queue_state
 *
 * @brief
 * Internal type, not to be used by clients.
 */
typedef enum mpsc_daemon_queue_state {
	MPSC_QUEUE_STATE_DRAINING = 0x0001,
	MPSC_QUEUE_STATE_WAKEUP   = 0x0002,
	MPSC_QUEUE_STATE_CANCELED = 0x0004,
} mpsc_daemon_queue_state_t;

struct mpsc_daemon_queue {
	mpsc_daemon_queue_kind_t    mpd_kind;
	mpsc_daemon_queue_state_t _Atomic mpd_state;
	mpsc_daemon_invoke_fn_t     mpd_invoke;
	union {
		mpsc_daemon_queue_t     mpd_target;
		struct thread          *mpd_thread;
		struct thread_call     *mpd_call;
	};
	struct mpsc_queue_head      mpd_queue;
	struct mpsc_queue_chain     mpd_chain;
};

/*!
 * @function mpsc_daemon_queue_init_with_thread
 *
 * @brief
 * Sets up a daemon queue to be a base queue drained by a kernel thread.
 *
 * @discussion
 * The function will allocate the thread and start it in assert_wait.
 *
 * @param dq
 * The queue to initialize
 *
 * @param invoke
 * The invoke function called on individual items on the queue during drain.
 *
 * @param pri
 * The scheduler priority for the created thread.
 *
 * @param name
 * The name to give to the created thread.
 *
 * @returns
 * Whether creating the thread was successful.
 */
kern_return_t
mpsc_daemon_queue_init_with_thread(mpsc_daemon_queue_t dq,
    mpsc_daemon_invoke_fn_t invoke, int pri, const char *name);


/*!
 * @function mpsc_daemon_queue_init_with_thread_call
 *
 * @brief
 * Sets up a daemon queue to be a base queue drained by a thread call.
 *
 * @param dq
 * The queue to initialize
 *
 * @param invoke
 * The invoke function called on individual items on the queue during drain.
 *
 * @param pri
 * The priority the thread call will run at.
 */
void
mpsc_daemon_queue_init_with_thread_call(mpsc_daemon_queue_t dq,
    mpsc_daemon_invoke_fn_t invoke, thread_call_priority_t pri);

/*!
 * @function mpsc_daemon_queue_init_with_target
 *
 * @brief
 * Sets up a daemon queue to target another daemon queue.
 *
 * @discussion
 * The targetting relationship is useful for subsystem layering purposes only.
 * Because draining a given queue is atomic with respect to its target, target
 * queue hierarchies are prone to starvation.
 *
 * @param dq
 * The queue to initialize
 *
 * @param invoke
 * The invoke function called on individual items on the queue during drain.
 *
 * @param target
 * The target queue of the initialized queue, which has to be initialized with
 * the mpsc_daemon_queue_nested_invoke invoke handler.
 */
void
mpsc_daemon_queue_init_with_target(mpsc_daemon_queue_t dq,
    mpsc_daemon_invoke_fn_t invoke, mpsc_daemon_queue_t target);

/*!
 * @function mpsc_daemon_queue_nested_invoke
 *
 * @brief
 * The invoke function to pass to mpsc_daemon_queue_init_* when a queue is meant
 * to be targeted by other queues.
 */
void
mpsc_daemon_queue_nested_invoke(mpsc_queue_chain_t elm,
    mpsc_daemon_queue_t dq);

/*!
 * @function mpsc_daemon_queue_cancel_and_wait
 *
 * @brief
 * Cancels the queue so that the object owning it can be destroyed.
 *
 * @discussion
 * This interface will cancel the queue and wait synchronously for the
 * cancelation to have taken effect, possibly waiting on elements currently
 * draining.
 *
 * Sending objects to the daemon queue after cancelation is undefined.
 *
 * Calling this function multiple times is undefined.
 *
 * Tearing down daemon queue hierarchies is the responsibility of the adopter.
 */
void
mpsc_daemon_queue_cancel_and_wait(mpsc_daemon_queue_t dq);

/*!
 * @function mpsc_daemon_enqueue
 *
 * @brief
 * Send ("async") an item to a given daemon on a given queue.
 *
 * @discussion
 * It is the responsibility of the caller to ensure preemption is disabled when
 * this call is made.
 *
 * @param dq
 * The daemon queue to enqueue the element onto.
 *
 * @param elm
 * The item to enqueue.
 *
 * @param options
 * Options applicable to the enqueue. In particupar passing
 * MPSC_QUEUE_DISABLE_PREEMPTION makes sure preemption is properly disabled
 * during the enqueue.
 */
void
mpsc_daemon_enqueue(mpsc_daemon_queue_t dq, mpsc_queue_chain_t elm,
    mpsc_queue_options_t options);


#pragma mark Deferred deallocation daemon

/*!
 * @function thread_deallocate_daemon_init
 *
 * @brief
 * Initializes the deferred deallocation daemon, called by thread_daemon_init().
 *
 * @discussion
 * The deferred deallocation daemon is a kernel thread based daemon queue that
 * is targeted by nested daemon queues.
 *
 * It is used to perform deferred deallocation for objects that can't safely be
 * deallocated from the context where the deallocation should normally occur.
 *
 * Subsystems using it are for example: turnstiles, workqueues, threads.
 *
 * @warning
 * New queues should be added to this daemon with great care,
 * as abusing it can lead to unbounded amount of kernel work.
 */
void
thread_deallocate_daemon_init(void);

/*!
 * @function thread_deallocate_daemon_register_queue
 *
 * @brief
 * Dynamically register a queue for deferred deletion with the deferred
 * deallocation daemon.
 *
 * @param dq
 * The daemon queue to register with the deferred deallocation daemon.
 *
 * @param invoke
 * The callback called on every element of this queue by the deallocation
 * daemon.
 */
void
thread_deallocate_daemon_register_queue(mpsc_daemon_queue_t dq,
    mpsc_daemon_invoke_fn_t invoke);


#pragma mark tests
#if DEBUG || DEVELOPMENT

int
mpsc_test_pingpong(uint64_t count, uint64_t *out);

#endif /* DEBUG || DEVELOPMENT */

#endif /* XNU_KERNEL_PRIVATE */

__END_DECLS

#endif /* _KERN_MPSC_QUEUE_H_ */

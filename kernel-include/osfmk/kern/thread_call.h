/*
 * Copyright (c) 1993-1995, 1999-2008 Apple Inc. All rights reserved.
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

/*!
 *  @header thread_call.h
 *  @discussion Facilities for executing work asynchronously.
 */

#ifndef _KERN_THREAD_CALL_H_
#define _KERN_THREAD_CALL_H_

#include <mach/mach_types.h>

#include <kern/clock.h>

#include <sys/cdefs.h>

struct thread_call;
typedef struct thread_call *thread_call_t;

typedef void *thread_call_param_t;
typedef void (*thread_call_func_t)(
	thread_call_param_t     param0,
	thread_call_param_t     param1);
/*!
 *  @enum thread_call_priority_t
 *  @discussion Thread call priorities should not be assumed to have any specific
 *  numerical value; they should be interpreted as importances or roles for work
 *  items, priorities for which will be reasonably managed by the subsystem.
 *  @constant THREAD_CALL_PRIORITY_HIGH Importance above everything but realtime.
 *  Thread calls allocated with this priority execute at extremely high priority,
 *  above everything but realtime threads.  They are generally executed  in serial.
 *  Though they may execute concurrently under some circumstances, no fan-out is implied.
 *  These work items should do very small amounts of work or risk disrupting system
 *  responsiveness.
 *  @constant THREAD_CALL_PRIORITY_KERNEL Importance similar to that of normal kernel
 *  threads.
 *  @constant THREAD_CALL_PRIORITY_USER Importance similar to that of normal user threads.
 *  @constant THREAD_CALL_PRIORITY_LOW Very low importance.
 *  @constant THREAD_CALL_PRIORITY_KERNEL_HIGH Importance higher than most kernel
 *  threads.
 */
typedef enum {
	THREAD_CALL_PRIORITY_HIGH        = 0,
	THREAD_CALL_PRIORITY_KERNEL      = 1,
	THREAD_CALL_PRIORITY_USER        = 2,
	THREAD_CALL_PRIORITY_LOW         = 3,
	THREAD_CALL_PRIORITY_KERNEL_HIGH = 4
} thread_call_priority_t;

enum {
	/* if call is re-submitted while the call is executing on a call thread, then delay the re-enqueue until it returns */
	THREAD_CALL_OPTIONS_ONCE   = 0x00000001,
#ifdef XNU_KERNEL_PRIVATE
	/* execute call from the timer interrupt instead of from the thread call thread, private interface for IOTES workloop signaling */
	THREAD_CALL_OPTIONS_SIGNAL = 0x00000002,
#endif /* XNU_KERNEL_PRIVATE */
};
typedef uint32_t thread_call_options_t;

__BEGIN_DECLS

/*!
 *  @function thread_call_enter
 *  @abstract Submit a thread call work item for immediate execution.
 *  @discussion If the work item is already scheduled for delayed execution, and it has
 *  not yet begun to run, that delayed invocation will be cancelled.  Note that if a
 *  thread call is rescheduled from its own callback, then multiple invocations of the
 *  callback may be in flight at the same time.
 *  @result TRUE if the call was already pending for either delayed or immediate
 *  execution, FALSE otherwise.
 *  @param call The thread call to execute.
 */
extern boolean_t        thread_call_enter(
	thread_call_t           call);
/*!
 *  @function thread_call_enter1
 *  @abstract Submit a thread call work item for immediate execution, with an extra parameter.
 *  @discussion This routine is identical to thread_call_enter(), except that
 *  the second parameter to the callback is specified.
 *  @result TRUE if the call was already pending for either delayed or immediate
 *  execution, FALSE otherwise.
 *  @param call The thread call to execute.
 *  @param param1 Parameter to pass callback.
 */
extern boolean_t        thread_call_enter1(
	thread_call_t           call,
	thread_call_param_t     param1);

/*!
 *  @function thread_call_enter_delayed
 *  @abstract Submit a thread call to be executed at some point in the future.
 *  @discussion If the work item is already scheduled for delayed or immediate execution,
 *  and it has not yet begun to run, that invocation will be cancelled in favor of execution
 *  at the newly specified time.  Note that if a thread call is rescheduled from its own callback,
 *  then multiple invocations of the callback may be in flight at the same time.
 *  @result TRUE if the call was already pending for either delayed or immediate
 *  execution, FALSE otherwise.
 *  @param call The thread call to execute.
 *  @param deadline Time, in absolute time units, at which to execute callback.
 */
extern boolean_t        thread_call_enter_delayed(
	thread_call_t           call,
	uint64_t                deadline);
/*!
 *  @function thread_call_enter1_delayed
 *  @abstract Submit a thread call to be executed at some point in the future, with an extra parameter.
 *  @discussion This routine is identical to thread_call_enter_delayed(),
 *  except that a second parameter to the callback is specified.
 *  @result TRUE if the call was already pending for either delayed or immediate
 *  execution, FALSE otherwise.
 *  @param call The thread call to execute.
 *  @param param1 Second parameter to callback.
 *  @param deadline Time, in absolute time units, at which to execute callback.
 */
extern boolean_t        thread_call_enter1_delayed(
	thread_call_t           call,
	thread_call_param_t     param1,
	uint64_t                deadline);
#ifdef XNU_KERNEL_PRIVATE

/*
 * Flags to alter the default timer/timeout coalescing behavior
 * on a per-thread_call basis.
 *
 * The SYS urgency classes indicate that the thread_call is not
 * directly related to the current thread at the time the thread_call
 * is entered, so it is ignored in the calculation entirely (only
 * the subclass specified is used).
 *
 * The USER flags indicate that both the current thread scheduling and QoS
 * attributes, in addition to the per-thread_call urgency specification,
 * are used to establish coalescing behavior.
 */
#define THREAD_CALL_DELAY_SYS_NORMAL            TIMEOUT_URGENCY_SYS_NORMAL
#define THREAD_CALL_DELAY_SYS_CRITICAL          TIMEOUT_URGENCY_SYS_CRITICAL
#define THREAD_CALL_DELAY_SYS_BACKGROUND        TIMEOUT_URGENCY_SYS_BACKGROUND

#define THREAD_CALL_DELAY_USER_MASK             TIMEOUT_URGENCY_USER_MASK
#define THREAD_CALL_DELAY_USER_NORMAL           TIMEOUT_URGENCY_USER_NORMAL
#define THREAD_CALL_DELAY_USER_CRITICAL         TIMEOUT_URGENCY_USER_CRITICAL
#define THREAD_CALL_DELAY_USER_BACKGROUND       TIMEOUT_URGENCY_USER_BACKGROUND

#define THREAD_CALL_DELAY_URGENCY_MASK          TIMEOUT_URGENCY_MASK

/*
 * Indicate that a specific leeway value is being provided (otherwise
 * the leeway parameter is ignored).  The supplied value can currently
 * only be used to extend the leeway calculated internally from the
 * urgency class provided.
 */
#define THREAD_CALL_DELAY_LEEWAY                TIMEOUT_URGENCY_LEEWAY

/*
 * Indicates that the time parameters should be interpreted as
 * mach_continuous_time values, rather than mach_absolute_time and the timer
 * be programmed to fire based on continuous time.
 */
#define THREAD_CALL_CONTINUOUS  0x100

/*!
 *  @function thread_call_enter_delayed_with_leeway
 *  @abstract Submit a thread call to be executed at some point in the future.
 *  @discussion If the work item is already scheduled for delayed or immediate execution,
 *  and it has not yet begun to run, that invocation will be cancelled in favor of execution
 *  at the newly specified time.  Note that if a thread call is rescheduled from its own callback,
 *  then multiple invocations of the callback may be in flight at the same time.
 *  @result TRUE if the call was already pending for either delayed or immediate
 *  execution, FALSE otherwise.
 *  @param call The thread call to execute.
 *  @param param1 Second parameter to callback.
 *  @param deadline Time, in absolute time units, at which to execute callback.
 *  @param leeway Time delta, in absolute time units, which sets range of time allowing kernel
 *       to decide appropriate time to run.
 *  @param flags configuration for timers in kernel.
 */
extern boolean_t        thread_call_enter_delayed_with_leeway(
	thread_call_t           call,
	thread_call_param_t     param1,
	uint64_t                deadline,
	uint64_t                leeway,
	uint32_t                flags);

#endif /* XNU_KERNEL_PRIVATE */

/*!
 *  @function thread_call_cancel
 *  @abstract Attempt to cancel a pending invocation of a thread call.
 *  @discussion Attempt to cancel a thread call which has been scheduled
 *  for execution with a thread_call_enter* variant.  If the call has not
 *  yet begun executing, the pending invocation will be cancelled and TRUE
 *  will be returned.  If the work item has already begun executing,
 *  thread_call_cancel will return FALSE immediately; the callback may be
 *  about to run, currently running, or already done executing.
 *  @result TRUE if the call was successfully cancelled, FALSE otherwise.
 */
extern boolean_t        thread_call_cancel(
	thread_call_t           call);
/*!
 *  @function thread_call_cancel_wait
 *  @abstract Attempt to cancel a pending invocation of a thread call.
 *  If unable to cancel, wait for current invocation to finish.
 *  @discussion Attempt to cancel a thread call which has been scheduled
 *  for execution with a thread_call_enter* variant.  If the call has not
 *  yet begun executing, the pending invocation will be cancelled and TRUE
 *  will be returned.  If the work item has already begun executing,
 *  thread_call_cancel_wait waits for the most recent invocation to finish. When
 *  called on a work item which has already finished, it will return FALSE immediately.
 *  Note that this routine can only be used on thread calls set up with either
 *  thread_call_allocate or thread_call_allocate_with_priority, and that invocations
 *  of the thread call <i>after</i> the current invocation may be in flight when
 *  thread_call_cancel_wait returns.
 *  @result TRUE if the call was successfully cancelled, FALSE otherwise.
 */
extern boolean_t        thread_call_cancel_wait(
	thread_call_t           call);

/*!
 *  @function thread_call_allocate
 *  @abstract Allocate a thread call to execute with default (high) priority.
 *  @discussion  Allocates a thread call that will run with properties of
 *  THREAD_CALL_PRIORITY_HIGH, binding the first parameter to the callback.
 *  @param func Callback to invoke when thread call is scheduled.
 *  @param param0 First argument ot pass to callback.
 *  @result Thread call which can be passed to thread_call_enter variants.
 */
extern thread_call_t    thread_call_allocate(
	thread_call_func_t      func,
	thread_call_param_t     param0);

/*!
 *  @function thread_call_allocate_with_priority
 *  @abstract Allocate a thread call to execute with a specified priority.
 *  @discussion Identical to thread_call_allocate, except that priority
 *  is specified by caller.
 *  @param func Callback to invoke when thread call is scheduled.
 *  @param param0 First argument to pass to callback.
 *  @param pri Priority of item.
 *  @result Thread call which can be passed to thread_call_enter variants.
 */
extern thread_call_t    thread_call_allocate_with_priority(
	thread_call_func_t      func,
	thread_call_param_t     param0,
	thread_call_priority_t  pri);

/*!
 *  @function thread_call_allocate_with_options
 *  @abstract Allocate a thread call to execute with a specified priority.
 *  @discussion Identical to thread_call_allocate, except that priority
 *  and options are specified by caller.
 *  @param func Callback to invoke when thread call is scheduled.
 *  @param param0 First argument to pass to callback.
 *  @param pri Priority of item.
 *  @param options Options for item.
 *  @result Thread call which can be passed to thread_call_enter variants.
 */
extern thread_call_t    thread_call_allocate_with_options(
	thread_call_func_t      func,
	thread_call_param_t     param0,
	thread_call_priority_t  pri,
	thread_call_options_t   options);

#ifdef KERNEL_PRIVATE
/*!
 *  @function thread_call_allocate_with_qos
 *  @abstract Allocate a thread call to execute with a specified QoS.
 *  @discussion Identical to thread_call_allocate_with_options, except it uses the QoS namespace.
 *      Private interface for pthread kext.
 *  @param func Callback to invoke when thread call is scheduled.
 *  @param param0 First argument to pass to callback.
 *  @param qos_tier QoS tier to execute callback at (as in THREAD_QOS_POLICY)
 *  @param options flags from thread_call_options_t to influence the thread call behavior
 *  @result Thread call which can be passed to thread_call_enter variants.
 */
extern thread_call_t
thread_call_allocate_with_qos(thread_call_func_t        func,
    thread_call_param_t       param0,
    int                       qos_tier,
    thread_call_options_t     options);

/*!
 *  @function thread_call_wait_once
 *  @abstract Wait for a THREAD_CALL_OPTIONS_ONCE call to finish executing if it is executing
 *  @discussion Only works on THREAD_CALL_OPTIONS_ONCE calls
 *  @param call The thread call to wait for
 *  @result True if it waited, false if it did not wait
 */
extern boolean_t
thread_call_wait_once(thread_call_t call);
#endif /* KERNEL_PRIVATE */

/*!
 *  @function thread_call_free
 *  @abstract Release a thread call.
 *  @discussion Should only be used on thread calls allocated with thread_call_allocate
 *  or thread_call_allocate_with_priority.  Once thread_call_free has been called,
 *  no other operations may be performed on a thread call.  If the thread call is
 *  currently pending, thread_call_free will return FALSE and will have no effect.
 *  Calling thread_call_free from a thread call's own callback is safe; the work
 *  item is not considering "pending" at that point.
 *  @result TRUE if the thread call has been successfully released, else FALSE.
 *  @param call The thread call to release.
 */
extern boolean_t        thread_call_free(
	thread_call_t           call);

/*!
 *  @function thread_call_isactive
 *  @abstract Determine whether a thread call is pending or currently executing.
 *  @param call Thread call to examine.
 *  @result TRUE if the thread call is either scheduled for execution (immediately
 *  or at some point in the future) or is currently executing.
 */
boolean_t               thread_call_isactive(
	thread_call_t call);
__END_DECLS

#ifdef  MACH_KERNEL_PRIVATE

#include <kern/call_entry.h>

typedef enum {
	THREAD_CALL_INDEX_HIGH          = 0,
	THREAD_CALL_INDEX_KERNEL        = 1,
	THREAD_CALL_INDEX_USER          = 2,
	THREAD_CALL_INDEX_LOW           = 3,
	THREAD_CALL_INDEX_KERNEL_HIGH   = 4,
	THREAD_CALL_INDEX_QOS_UI        = 5,
	THREAD_CALL_INDEX_QOS_IN        = 6,
	THREAD_CALL_INDEX_QOS_UT        = 7,
	THREAD_CALL_INDEX_MAX           = 8,    /* count of thread call indexes */
} thread_call_index_t;

struct thread_call {
	struct call_entry               tc_call;                /* Must be first for queue macros */
	uint64_t                        tc_submit_count;
	uint64_t                        tc_finish_count;
	uint64_t                        tc_ttd;                 /* Time to deadline at creation */
	uint64_t                        tc_soft_deadline;
	thread_call_index_t             tc_index;
	uint32_t                        tc_flags;
	int32_t                         tc_refs;
};

#define THREAD_CALL_ALLOC       0x01    /* memory owned by thread_call.c */
#define THREAD_CALL_WAIT        0x02    /* thread waiting for call to finish running */
#define THREAD_CALL_DELAYED     0x04    /* deadline based */
#define THREAD_CALL_RUNNING     0x08    /* currently executing on a thread */
#define THREAD_CALL_SIGNAL      0x10    /* call from timer interrupt instead of thread */
#define THREAD_CALL_ONCE        0x20    /* pend the enqueue if re-armed while running */
#define THREAD_CALL_RESCHEDULE  0x40    /* enqueue is pending due to re-arm while running */
#define THREAD_CALL_RATELIMITED TIMEOUT_URGENCY_RATELIMITED     /* 0x80 */
/*      THREAD_CALL_CONTINUOUS  0x100 */

typedef struct thread_call thread_call_data_t;

extern void             thread_call_initialize(void);

extern void             thread_call_setup(
	thread_call_t                   call,
	thread_call_func_t              func,
	thread_call_param_t             param0);

extern void             thread_call_delayed_timer_rescan_all(void);
#endif  /* MACH_KERNEL_PRIVATE */

#ifdef  XNU_KERNEL_PRIVATE

__BEGIN_DECLS

/*
 * These routines are equivalent to their thread_call_enter_XXX
 * variants, only the thread_call_t is allocated out of a
 * fixed preallocated pool of memory, and will panic if the pool
 * is exhausted.
 */

extern void             thread_call_func_delayed(
	thread_call_func_t              func,
	thread_call_param_t             param,
	uint64_t                        deadline);

extern void             thread_call_func_delayed_with_leeway(
	thread_call_func_t              func,
	thread_call_param_t             param,
	uint64_t                deadline,
	uint64_t                leeway,
	uint32_t                flags);

/*
 * This iterates all of the pending or delayed thread calls in the group,
 * which is really inefficient.
 *
 * This is deprecated, switch to an allocated thread call instead.
 */
extern boolean_t        thread_call_func_cancel(
	thread_call_func_t      func,
	thread_call_param_t     param,
	boolean_t               cancel_all);

/*
 * Called on the wake path to adjust the thread callouts running in mach_continuous_time
 */
void                            adjust_cont_time_thread_calls(void);

__END_DECLS

#endif  /* XNU_KERNEL_PRIVATE */

#endif  /* _KERN_THREAD_CALL_H_ */

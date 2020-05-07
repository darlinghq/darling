/*
 * Copyright (c) 2017 Apple, Inc. All rights reserved.
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

#ifndef _WORKQUEUE_TRACE_H_
#define _WORKQUEUE_TRACE_H_

// General workqueue tracepoints, mostly for debugging
#define WQ_TRACE_WORKQUEUE_SUBCLASS 1
// Workqueue request scheduling tracepoints
#define WQ_TRACE_REQUESTS_SUBCLASS 2
// Generic pthread tracepoints
#define WQ_TRACE_BSDTHREAD_SUBCLASS 16

#define TRACE_wq_pthread_exit \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x01)
#define TRACE_wq_workqueue_exit \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x02)
#define TRACE_wq_runthread \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x03)
#define TRACE_wq_death_call \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x05)
#define TRACE_wq_thread_block \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x09)
#define TRACE_wq_thactive_update \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x0a)
#define TRACE_wq_add_timer \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x0b)
#define TRACE_wq_start_add_timer \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x0c)
#define TRACE_wq_override_dispatch \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x14)
#define TRACE_wq_override_reset \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x15)
#define TRACE_wq_thread_create_failed \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x1d)
#define TRACE_wq_thread_terminate \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x1e)
#define TRACE_wq_thread_create \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x1f)
#define TRACE_wq_select_threadreq \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x20)
#define TRACE_wq_creator_select \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x23)
#define TRACE_wq_creator_yield \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x24)
#define TRACE_wq_constrained_admission \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x25)
#define TRACE_wq_wqops_reqthreads \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_WORKQUEUE_SUBCLASS, 0x26)

#define TRACE_wq_create \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_REQUESTS_SUBCLASS, 0x01)
#define TRACE_wq_destroy \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_REQUESTS_SUBCLASS, 0x02)
#define TRACE_wq_thread_logical_run \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_REQUESTS_SUBCLASS, 0x03)
#define TRACE_wq_thread_request_initiate \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_REQUESTS_SUBCLASS, 0x05)
#define TRACE_wq_thread_request_modify \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_REQUESTS_SUBCLASS, 0x06)
#define TRACE_wq_thread_request_fulfill \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_REQUESTS_SUBCLASS, 0x08)

#define TRACE_bsdthread_set_qos_self \
	        KDBG_CODE(DBG_PTHREAD, WQ_TRACE_BSDTHREAD_SUBCLASS, 0x1)

#define WQ_TRACE(x, a, b, c, d, e) \
	        ({ KERNEL_DEBUG_CONSTANT(x, a, b, c, d, e); })
#define WQ_TRACE_WQ(x, wq, b, c, d, e) \
	        ({ KERNEL_DEBUG_CONSTANT(x, (wq)->wq_proc->p_pid, b, c, d, e); })

#if (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD)
#define __wq_trace_only
#else // (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD)
#define __wq_trace_only __unused
#endif // (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD)

#endif // _WORKQUEUE_TRACE_H_

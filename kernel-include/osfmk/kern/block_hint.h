/*
 * Copyright (c) 2000-2016 Apple Inc. All rights reserved.
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

#ifndef _KERN_BLOCK_HINT_H_
#define _KERN_BLOCK_HINT_H_

typedef enum thread_snapshot_wait_flags {
	kThreadWaitNone                 = 0x00,
	kThreadWaitKernelMutex          = 0x01,
	kThreadWaitPortReceive          = 0x02,
	kThreadWaitPortSetReceive       = 0x03,
	kThreadWaitPortSend             = 0x04,
	kThreadWaitPortSendInTransit    = 0x05,
	kThreadWaitSemaphore            = 0x06,
	kThreadWaitKernelRWLockRead     = 0x07,
	kThreadWaitKernelRWLockWrite    = 0x08,
	kThreadWaitKernelRWLockUpgrade  = 0x09,
	kThreadWaitUserLock             = 0x0a,
	kThreadWaitPThreadMutex         = 0x0b,
	kThreadWaitPThreadRWLockRead    = 0x0c,
	kThreadWaitPThreadRWLockWrite   = 0x0d,
	kThreadWaitPThreadCondVar       = 0x0e,
	kThreadWaitParkedWorkQueue      = 0x0f,
	kThreadWaitWorkloopSyncWait     = 0x10,
	kThreadWaitOnProcess            = 0x11,
	kThreadWaitSleepWithInheritor   = 0x12,
} __attribute__((packed)) block_hint_t;

_Static_assert(sizeof(block_hint_t) <= sizeof(short),
    "block_hint_t must fit within a short");

#ifdef XNU_KERNEL_PRIVATE

struct waitq;
struct stackshot_thread_waitinfo;
typedef struct stackshot_thread_waitinfo thread_waitinfo_t;

/* Used for stackshot_thread_waitinfo_unsafe */
extern void kdp_lck_mtx_find_owner(struct waitq * waitq, event64_t event, thread_waitinfo_t *waitinfo);
extern void kdp_sema_find_owner(struct waitq * waitq, event64_t event, thread_waitinfo_t *waitinfo);
extern void kdp_mqueue_send_find_owner(struct waitq * waitq, event64_t event, thread_waitinfo_t *waitinfo);
extern void kdp_mqueue_recv_find_owner(struct waitq * waitq, event64_t event, thread_waitinfo_t *waitinfo);
extern void kdp_ulock_find_owner(struct waitq * waitq, event64_t event, thread_waitinfo_t *waitinfo);
extern void kdp_rwlck_find_owner(struct waitq * waitq, event64_t event, thread_waitinfo_t *waitinfo);
extern void kdp_pthread_find_owner(thread_t thread, thread_waitinfo_t *waitinfo);
extern void *kdp_pthread_get_thread_kwq(thread_t thread);
extern void kdp_workloop_sync_wait_find_owner(thread_t thread, event64_t event, thread_waitinfo_t *waitinfo);
extern void kdp_wait4_find_process(thread_t thread, event64_t event, thread_waitinfo_t *waitinfo);
extern void kdp_sleep_with_inheritor_find_owner(struct waitq * waitq, __unused event64_t event, thread_waitinfo_t * waitinfo);
extern void kdp_turnstile_fill_tsinfo(struct turnstile *ts, thread_turnstileinfo_t *tsinfo);

#endif /* XNU_KERNEL_PRIVATE */

#endif /* !_KERN_BLOCK_HINT_H_ */

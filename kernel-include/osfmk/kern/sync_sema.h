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
/*
 * @OSF_COPYRIGHT@
 *
 */
/*
 *	File:	kern/sync_sema.h
 *	Author:	Joseph CaraDonna
 *
 *	Contains RT distributed semaphore synchronization service definitions.
 */

#ifndef _KERN_SYNC_SEMA_H_
#define _KERN_SYNC_SEMA_H_

#include <kern/kern_types.h>
#include <mach/sync_policy.h>

#ifdef MACH_KERNEL_PRIVATE

#include <kern/queue.h>
#include <kern/waitq.h>
#include <os/refcnt.h>

typedef struct semaphore {
	queue_chain_t     task_link;  /* chain of semaphores owned by a task */
	struct waitq      waitq;      /* queue of blocked threads & lock     */
	task_t            owner;      /* task that owns semaphore            */
	ipc_port_t        port;       /* semaphore port	                     */
	os_refcnt_t       ref_count;  /* reference count		     */
	int               count;      /* current count value	             */
	boolean_t         active;     /* active status			     */
} Semaphore;

#define semaphore_lock(semaphore)   waitq_lock(&(semaphore)->waitq)
#define semaphore_unlock(semaphore) waitq_unlock(&(semaphore)->waitq)

extern void semaphore_init(void);

extern  void            semaphore_reference(semaphore_t semaphore);
extern  void            semaphore_dereference(semaphore_t semaphore);
extern  void            semaphore_destroy_all(task_t task);

#endif /* MACH_KERNEL_PRIVATE */

#endif /* _KERN_SYNC_SEMA_H_ */

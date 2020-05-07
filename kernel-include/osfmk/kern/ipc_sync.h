/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
#ifndef _KERN_IPC_SYNC_H_
#define _KERN_IPC_SYNC_H_

#include <mach/mach_types.h>
#include <ipc/ipc_types.h>
#include <kern/spl.h>

extern semaphore_t convert_port_to_semaphore(ipc_port_t port);
extern ipc_port_t convert_semaphore_to_port(semaphore_t semaphore);
extern kern_return_t port_name_to_semaphore(
	mach_port_name_t  name,
	semaphore_t       *semaphore);
extern void semaphore_notify(mach_msg_header_t *msg);

lock_set_t  convert_port_to_lock_set(ipc_port_t port);
ipc_port_t  convert_lock_set_to_port(lock_set_t lock_set);

#endif /* _KERN_IPC_SYNC_H_ */

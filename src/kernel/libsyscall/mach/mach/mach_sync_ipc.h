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
/*
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

#ifndef _MACH_SYNC_IPC_H_
#define _MACH_SYNC_IPC_H_

#include <mach/mach.h>

__BEGIN_DECLS

/*!
 * @function mach_sync_ipc_link_monitoring_start
 *
 * @abstract
 * Starts monitoring the sync IPC priority inversion avoidance
 * facility of the current thread.
 * A subsequent call to mach_sync_ipc_link_monitoring_stop() will
 * validate that the facility took effect for all synchronous IPC
 * performed from this thread between the calls to start and stop.
 *
 * @discussion
 * In case of success, a port right is returned, which has to be
 * deallocated by passing it to mach_sync_ipc_link_monitoring_stop().
 *
 * @param port
 * Pointer to a mach_port_t that will be populated in case of success.
 *
 * @result
 * KERN_SUCCESS in case of success, specific error otherwise.
 * If the call is not supported, KERN_NOT_SUPPORTED is returned.
 */
extern kern_return_t mach_sync_ipc_link_monitoring_start(mach_port_t* port);

/*!
 * @function mach_sync_ipc_link_monitoring_stop
 *
 * @abstract
 * Stops monitoring the sync IPC priority inversion avoidance facility
 * of the current thread started by a call to mach_sync_ipc_link_monitoring_start().
 *
 * Returns whether the facility took effect for all synchronous IPC performed
 * from this thread between the calls to start and stop.
 *
 * Reasons for this function to return false include:
 * -remote message event handler did not reply to the message itself
 * -remote message was not received by a workloop (xpc connection or dispatch mach channel)
 *
 * @discussion
 * To be called after mach_sync_ipc_link_monitoring_start(). If
 * mach_sync_ipc_link_monitoring_start() didn't return an error this
 * function must be called to deallocate the port right that was returned.
 *
 * @param port
 * mach_port_t returned by mach_sync_ipc_link_monitoring_start().
 *
 * @param in_effect
 * Pointer to boolean_t value that will be populated in the case of success.
 * Indicates whether the sync IPC priority inversion avoidance facility took
 * effect for all synchronous IPC performed from this thread between the calls
 * to start and stop.
 *
 * @result
 * KERN_SUCCESS in case of no errors, specific error otherwise.
 * If the call is not supported, KERN_NOT_SUPPORTED is returned.
 */
extern kern_return_t mach_sync_ipc_link_monitoring_stop(mach_port_t port, boolean_t* in_effect);

typedef enum thread_destruct_special_reply_port_rights {
	THREAD_SPECIAL_REPLY_PORT_ALL,
	THREAD_SPECIAL_REPLY_PORT_RECEIVE_ONLY,
	THREAD_SPECIAL_REPLY_PORT_SEND_ONLY,
} thread_destruct_special_reply_port_rights_t;

extern kern_return_t thread_destruct_special_reply_port(mach_port_name_t port, thread_destruct_special_reply_port_rights_t rights);

extern mach_port_t mig_get_special_reply_port(void);

extern void mig_dealloc_special_reply_port(mach_port_t migport);


__END_DECLS

#endif  /* _MACH_SYNC_IPC_H_ */

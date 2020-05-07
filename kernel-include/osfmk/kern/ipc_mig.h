/*
 * Copyright (c) 2000-2005 Apple Computer, Inc. All rights reserved.
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
 */

#ifndef _KERN_IPC_MIG_H_
#define _KERN_IPC_MIG_H_

#include <mach/mig.h>
#include <mach/mach_types.h>
#include <mach/message.h>
#include <kern/kern_types.h>

#include <sys/cdefs.h>

#ifdef  XNU_KERNEL_PRIVATE

#include <sys/kdebug.h>

/*
 * Define the trace points for MIG-generated calls.  One traces the input parameters
 * to MIG called things, another traces the outputs, and one traces bad message IDs.
 */
#ifdef _MIG_TRACE_PARAMETERS_

#define __BeforeRcvCallTrace(msgid, arg1, arg2, arg3, arg4)                               \
	KERNEL_DEBUG_CONSTANT_IST(KDEBUG_TRACE,          \
	                      KDBG_MIGCODE(msgid) | DBG_FUNC_START,   \
	                      (unsigned int)(arg1),                                   \
	                      (unsigned int)(arg2),                                   \
	                      (unsigned int)(arg3),                                   \
	                      (unsigned int)(arg4),                                   \
	                      (unsigned int)(0));

#define __AfterRcvCallTrace(msgid, arg1, arg2, arg3, arg4)                                \
	KERNEL_DEBUG_CONSTANT_IST(KDEBUG_TRACE,          \
	                      KDBG_MIGCODE(msgid) | DBG_FUNC_END,     \
	                      (unsigned int)(arg1),                                   \
	                      (unsigned int)(arg2),                                   \
	                      (unsigned int)(arg3),                                   \
	                      (unsigned int)(arg4),                                   \
	                      (unsigned int)(0));

#define __BeforeSimpleCallTrace(msgid, arg1, arg2, arg3, arg4)                            \
	KERNEL_DEBUG_CONSTANT_IST(KDEBUG_TRACE,          \
	                      KDBG_MIGCODE(msgid) | DBG_FUNC_START,   \
	                      (unsigned int)(arg1),                                   \
	                      (unsigned int)(arg2),                                   \
	                      (unsigned int)(arg3),                                   \
	                      (unsigned int)(arg4),                                   \
	                      (unsigned int)(0));

#define __AfterSimpleCallTrace(msgid, arg1, arg2, arg3, arg4)                             \
	KERNEL_DEBUG_CONSTANT_IST(KDEBUG_TRACE,          \
	              KDBG_MIGCODE(msgid) | DBG_FUNC_END,     \
	                      (unsigned int)(arg1),                                   \
	                      (unsigned int)(arg2),                                   \
	                      (unsigned int)(arg3),                                   \
	                      (unsigned int)(arg4),                                   \
	                      (unsigned int)(0));

#else /* !_MIG_TRACE_PARAMETERS_ */

#define __BeforeRcvRpc(msgid, _NAME_)                                                 \
	KERNEL_DEBUG_CONSTANT_IST(KDEBUG_TRACE,          \
	                      KDBG_MIGCODE(msgid) | DBG_FUNC_START,   \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0));

#define __AfterRcvRpc(msgid, _NAME_)                                                  \
	KERNEL_DEBUG_CONSTANT_IST(KDEBUG_TRACE,          \
	              KDBG_MIGCODE(msgid) | DBG_FUNC_END,     \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0));


#define __BeforeRcvSimple(msgid, _NAME_)                                              \
	KERNEL_DEBUG_CONSTANT_IST(KDEBUG_TRACE,          \
	                      KDBG_MIGCODE(msgid) | DBG_FUNC_START,   \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0));

#define __AfterRcvSimple(msgid, _NAME_)                                               \
	KERNEL_DEBUG_CONSTANT_IST(KDEBUG_TRACE,          \
	                      KDBG_MIGCODE(msgid) | DBG_FUNC_END,     \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0));

#endif /* !_MIG_TRACE_PARAMETERS_ */

#define _MIG_MSGID_INVALID(msgid)                                                     \
	KERNEL_DEBUG_CONSTANT_IST(KDEBUG_TRACE,          \
	                      MACHDBG_CODE(DBG_MACH_MSGID_INVALID, (msgid)),  \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0),                                      \
	                      (unsigned int)(0))

#endif  /* XNU_KERNEL_PRIVATE */

__BEGIN_DECLS

/* Send a message from the kernel */

extern mach_msg_return_t mach_msg_send_from_kernel_proper(
	mach_msg_header_t       *msg,
	mach_msg_size_t         send_size);

#define mach_msg_send_from_kernel mach_msg_send_from_kernel_proper

extern mach_msg_return_t
mach_msg_rpc_from_kernel_proper(
	mach_msg_header_t       *msg,
	mach_msg_size_t         send_size,
	mach_msg_size_t         rcv_size);

#define mach_msg_rpc_from_kernel mach_msg_rpc_from_kernel_proper

extern void
mach_msg_destroy_from_kernel_proper(
	mach_msg_header_t       *msg);

#define mach_msg_destroy_from_kernel mach_msg_destroy_from_kernel_proper

#ifdef XNU_KERNEL_PRIVATE
extern mach_msg_return_t mach_msg_send_from_kernel_with_options_legacy(
	mach_msg_header_t       *msg,
	mach_msg_size_t         send_size,
	mach_msg_option_t       option,
	mach_msg_timeout_t      timeout_val);

extern mach_msg_return_t mach_msg_send_from_kernel_with_options(
	mach_msg_header_t       *msg,
	mach_msg_size_t         send_size,
	mach_msg_option_t       option,
	mach_msg_timeout_t      timeout_val)
__XNU_INTERNAL(mach_msg_send_from_kernel_with_options);
#else
extern mach_msg_return_t mach_msg_send_from_kernel_with_options(
	mach_msg_header_t       *msg,
	mach_msg_size_t         send_size,
	mach_msg_option_t       option,
	mach_msg_timeout_t      timeout_val);
#endif /* XNU_KERNEL_PRIVATE */

__END_DECLS

#ifdef  MACH_KERNEL_PRIVATE

extern void mach_msg_receive_continue(void);

/* Initialize kernel server dispatch table */
extern void             mig_init(void);

/*
 * Kernel implementation of the MIG object base class
 *
 * Conforms to the MIGObjectInterface defined in <mach/mig.h>
 * Ports are automatically allocated for the duration of outstanding
 * cross-task references and then released.
 */

typedef struct mig_object {
	const IMIGObjectVtbl    *pVtbl;                 /* our interface def */
	mach_port_t             port;                    /* our port pointer  */
} mig_object_data_t;


/*
 * MIG notify base class definition
 * These are chained off the mig object to which the are registered.
 * When that object triggers a notification delivery, we walk this
 * chain and deliver the appropriate notification.
 */
typedef struct mig_notify_object {
	const IMIGNotifyObjectVtbl *pVtbl;                 /* our interface def */
	mach_port_t             port;                    /* our port pointer  */
} mig_notify_object_data_t;

extern kern_return_t mig_object_init(
	mig_object_t            mig_object,
	const IMIGObject        *interface);

extern void mig_object_destroy(
	mig_object_t            mig_object);

extern void mig_object_reference(
	mig_object_t            mig_object);

extern void mig_object_deallocate(
	mig_object_t            mig_object);

extern ipc_port_t convert_mig_object_to_port(
	mig_object_t            mig_object);

extern mig_object_t convert_port_to_mig_object(
	ipc_port_t              port,
	const MIGIID            *iid);

extern void mig_object_no_senders(
	ipc_port_t              port);

#endif  /* MACH_KERNEL_PRIVATE */

#endif  /* _KERN_IPC_MIG_H_ */

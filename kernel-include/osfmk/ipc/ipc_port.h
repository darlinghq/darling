/*
 * Copyright (c) 2000-2016 Apple Computer, Inc. All rights reserved.
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
/*
 * NOTICE: This file was modified by McAfee Research in 2004 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */
/*
 */
/*
 *	File:	ipc/ipc_port.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Definitions for ports.
 */

#ifndef _IPC_IPC_PORT_H_
#define _IPC_IPC_PORT_H_

#ifdef MACH_KERNEL_PRIVATE

#include <mach_assert.h>
#include <mach_debug.h>

#include <mach/mach_types.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/port.h>

#include <kern/assert.h>
#include <kern/kern_types.h>
#include <kern/turnstile.h>

#include <ipc/ipc_types.h>
#include <ipc/ipc_object.h>
#include <ipc/ipc_mqueue.h>
#include <ipc/ipc_space.h>

#include <security/_label.h>

/*
 *  A receive right (port) can be in four states:
 *	1) dead (not active, ip_timestamp has death time)
 *	2) in a space (ip_receiver_name != 0, ip_receiver points
 *	to the space but doesn't hold a ref for it)
 *	3) in transit (ip_receiver_name == 0, ip_destination points
 *	to the destination port and holds a ref for it)
 *	4) in limbo (ip_receiver_name == 0, ip_destination == IP_NULL)
 *
 *  If the port is active, and ip_receiver points to some space,
 *  then ip_receiver_name != 0, and that space holds receive rights.
 *  If the port is not active, then ip_timestamp contains a timestamp
 *  taken when the port was destroyed.
 */

struct task_watchport_elem;

typedef unsigned int ipc_port_timestamp_t;

struct ipc_port {
	/*
	 * Initial sub-structure in common with ipc_pset
	 * First element is an ipc_object second is a
	 * message queue
	 */
	struct ipc_object ip_object;
	struct ipc_mqueue ip_messages;

	union {
		struct ipc_space *receiver;
		struct ipc_port *destination;
		ipc_port_timestamp_t timestamp;
	} data;

	union {
		ipc_kobject_t kobject;
		ipc_importance_task_t imp_task;
		ipc_port_t sync_inheritor_port;
		struct knote *sync_inheritor_knote;
		struct turnstile *sync_inheritor_ts;
	} kdata;

	struct ipc_port *ip_nsrequest;
	struct ipc_port *ip_pdrequest;
	struct ipc_port_request *ip_requests;
	union {
		struct ipc_kmsg *premsg;
		struct turnstile *send_turnstile;
	} kdata2;

	mach_vm_address_t ip_context;

	natural_t ip_sprequests:1,      /* send-possible requests outstanding */
	    ip_spimportant:1,           /* ... at least one is importance donating */
	    ip_impdonation:1,           /* port supports importance donation */
	    ip_tempowner:1,             /* dont give donations to current receiver */
	    ip_guarded:1,               /* port guarded (use context value as guard) */
	    ip_strict_guard:1,          /* Strict guarding; Prevents user manipulation of context values directly */
	    ip_specialreply:1,          /* port is a special reply port */
	    ip_sync_link_state:3,       /* link the port to destination port/ Workloop */
	    ip_sync_bootstrap_checkin:1,/* port part of sync bootstrap checkin, push on thread doing the checkin */
	    ip_immovable_receive:1,     /* the receive right cannot be moved out of a space, until it is destroyed */
	    ip_no_grant:1,              /* Port wont accept complex messages containing (ool) port descriptors */
	    ip_immovable_send:1,        /* No send(once) rights to this port can be moved out of a space */
	    ip_impcount:18;             /* number of importance donations in nested queue */

	mach_port_mscount_t ip_mscount;
	mach_port_rights_t ip_srights;
	mach_port_rights_t ip_sorights;

#if     MACH_ASSERT
#define IP_NSPARES              4
#define IP_CALLSTACK_MAX        16
/*	queue_chain_t	ip_port_links;*//* all allocated ports */
	thread_t        ip_thread;      /* who made me?  thread context */
	unsigned long   ip_timetrack;   /* give an idea of "when" created */
	uintptr_t       ip_callstack[IP_CALLSTACK_MAX]; /* stack trace */
	unsigned long   ip_spares[IP_NSPARES]; /* for debugging */
#endif  /* MACH_ASSERT */
#if DEVELOPMENT || DEBUG
	uint8_t         ip_srp_lost_link:1,     /* special reply port turnstile link chain broken */
	    ip_srp_msg_sent:1;                  /* special reply port msg sent */
#endif
};


#define ip_references           ip_object.io_references

#define ip_receiver_name        ip_messages.imq_receiver_name
#define ip_in_pset              ip_messages.imq_in_pset
#define ip_reply_context        ip_messages.imq_context

#define ip_receiver             data.receiver
#define ip_destination          data.destination
#define ip_timestamp            data.timestamp

#define ip_kobject              kdata.kobject
#define ip_imp_task             kdata.imp_task
#define ip_sync_inheritor_port  kdata.sync_inheritor_port
#define ip_sync_inheritor_knote kdata.sync_inheritor_knote
#define ip_sync_inheritor_ts    kdata.sync_inheritor_ts

#define ip_premsg               kdata2.premsg
#define ip_send_turnstile       kdata2.send_turnstile

#define port_send_turnstile(port)       (IP_PREALLOC(port) ? (port)->ip_premsg->ikm_turnstile : (port)->ip_send_turnstile)

#define set_port_send_turnstile(port, value)                 \
MACRO_BEGIN                                                  \
if (IP_PREALLOC(port)) {                                     \
	(port)->ip_premsg->ikm_turnstile = (value);          \
} else {                                                     \
	(port)->ip_send_turnstile = (value);                 \
}                                                            \
MACRO_END

#define port_send_turnstile_address(port)                    \
(IP_PREALLOC(port) ? &((port)->ip_premsg->ikm_turnstile) : &((port)->ip_send_turnstile))

#define port_rcv_turnstile_address(port) \
	&(port)->ip_messages.imq_wait_queue.waitq_ts


/*
 * SYNC IPC state flags for special reply port/ rcv right.
 *
 * PORT_SYNC_LINK_ANY
 *    Special reply port is not linked to any other port
 *    or WL and linkage should be allowed.
 *
 * PORT_SYNC_LINK_PORT
 *    Special reply port is linked to the port and
 *    ip_sync_inheritor_port contains the inheritor
 *    port.
 *
 * PORT_SYNC_LINK_WORKLOOP_KNOTE
 *    Special reply port is linked to a WL (via a knote).
 *    ip_sync_inheritor_knote contains a pointer to the knote
 *    the port is stashed on.
 *
 * PORT_SYNC_LINK_WORKLOOP_STASH
 *    Special reply port is linked to a WL (via a knote stash).
 *    ip_sync_inheritor_ts contains a pointer to the turnstile with a +1
 *    the port is stashed on.
 *
 * PORT_SYNC_LINK_NO_LINKAGE
 *    Message sent to special reply port, do
 *    not allow any linkages till receive is
 *    complete.
 *
 * PORT_SYNC_LINK_RCV_THREAD
 *    Receive right copied out as a part of bootstrap check in,
 *    push on the thread which copied out the port.
 */
#define PORT_SYNC_LINK_ANY              (0)
#define PORT_SYNC_LINK_PORT             (0x1)
#define PORT_SYNC_LINK_WORKLOOP_KNOTE   (0x2)
#define PORT_SYNC_LINK_WORKLOOP_STASH   (0x3)
#define PORT_SYNC_LINK_NO_LINKAGE       (0x4)
#define PORT_SYNC_LINK_RCV_THREAD       (0x5)

#define IP_NULL                 IPC_PORT_NULL
#define IP_DEAD                 IPC_PORT_DEAD
#define IP_VALID(port)          IPC_PORT_VALID(port)

#define ip_object_to_port(io)   __container_of(io, struct ipc_port, ip_object)
#define ip_to_object(port)      (&(port)->ip_object)
#define ip_active(port)         io_active(ip_to_object(port))
#define ip_lock_init(port)      io_lock_init(ip_to_object(port))
#define ip_lock_held(port)      io_lock_held(ip_to_object(port))
#define ip_lock(port)           io_lock(ip_to_object(port))
#define ip_lock_try(port)       io_lock_try(ip_to_object(port))
#define ip_lock_held_kdp(port)  io_lock_held_kdp(ip_to_object(port))
#define ip_unlock(port)         io_unlock(ip_to_object(port))

#define ip_reference(port)      io_reference(ip_to_object(port))
#define ip_release(port)        io_release(ip_to_object(port))

/* get an ipc_port pointer from an ipc_mqueue pointer */
#define ip_from_mq(mq) \
	        __container_of(mq, struct ipc_port, ip_messages)

#define ip_reference_mq(mq)     ip_reference(ip_from_mq(mq))
#define ip_release_mq(mq)       ip_release(ip_from_mq(mq))

#define ip_kotype(port)         io_kotype(ip_to_object(port))
#define ip_is_kobject(port)     io_is_kobject(ip_to_object(port))

#define ip_full_kernel(port)    imq_full_kernel(&(port)->ip_messages)
#define ip_full(port)           imq_full(&(port)->ip_messages)

/*
 * JMM - Preallocation flag
 * This flag indicates that there is a message buffer preallocated for this
 * port and we should use that when sending (from the kernel) rather than
 * allocate a new one.  This avoids deadlocks during notification message
 * sends by critical system threads (which may be needed to free memory and
 * therefore cannot be blocked waiting for memory themselves).
 */
#define IP_BIT_PREALLOC         0x00008000      /* preallocated mesg */
#define IP_PREALLOC(port)       ((port)->ip_object.io_bits & IP_BIT_PREALLOC)

#define IP_SET_PREALLOC(port, kmsg)                                     \
MACRO_BEGIN                                                             \
	(port)->ip_object.io_bits |= IP_BIT_PREALLOC;                   \
	(port)->ip_premsg = (kmsg);                                     \
MACRO_END

#define IP_CLEAR_PREALLOC(port, kmsg)                                   \
MACRO_BEGIN                                                             \
	assert((port)->ip_premsg == kmsg);                              \
	(port)->ip_object.io_bits &= ~IP_BIT_PREALLOC;                  \
	(port)->ip_premsg = IKM_NULL;                                   \
MACRO_END

/* JMM - address alignment/packing for LP64 */
struct ipc_port_request {
	union {
		struct ipc_port *port;
		ipc_port_request_index_t index;
	} notify;

	union {
		mach_port_name_t name;
		struct ipc_table_size *size;
	} name;
};

#define ipr_next                notify.index
#define ipr_size                name.size

#define ipr_soright             notify.port
#define ipr_name                name.name

/*
 * Use the low bits in the ipr_soright to specify the request type
 */
#define IPR_SOR_SPARM_MASK      1               /* send-possible armed */
#define IPR_SOR_SPREQ_MASK      2               /* send-possible requested */
#define IPR_SOR_SPBIT_MASK      3               /* combo */
#define IPR_SOR_SPARMED(sor)    (((uintptr_t)(sor) & IPR_SOR_SPARM_MASK) != 0)
#define IPR_SOR_SPREQ(sor)      (((uintptr_t)(sor) & IPR_SOR_SPREQ_MASK) != 0)
#define IPR_SOR_PORT(sor)       ((ipc_port_t)((uintptr_t)(sor) & ~IPR_SOR_SPBIT_MASK))
#define IPR_SOR_MAKE(p, m)       ((ipc_port_t)((uintptr_t)(p) | (m)))

extern lck_grp_t        ipc_lck_grp;
extern lck_attr_t       ipc_lck_attr;

/*
 *	Taking the ipc_port_multiple lock grants the privilege
 *	to lock multiple ports at once.  No ports must locked
 *	when it is taken.
 */

extern lck_spin_t ipc_port_multiple_lock_data;

#define ipc_port_multiple_lock_init()                                   \
	        lck_spin_init(&ipc_port_multiple_lock_data, &ipc_lck_grp, &ipc_lck_attr)

#define ipc_port_multiple_lock()                                        \
	        lck_spin_lock_grp(&ipc_port_multiple_lock_data, &ipc_lck_grp)

#define ipc_port_multiple_unlock()                                      \
	        lck_spin_unlock(&ipc_port_multiple_lock_data)

/*
 *	The port timestamp facility provides timestamps
 *	for port destruction.  It is used to serialize
 *	mach_port_names with port death.
 */

extern ipc_port_timestamp_t ipc_port_timestamp_data;

/* Retrieve a port timestamp value */
extern ipc_port_timestamp_t ipc_port_timestamp(void);

/*
 *	Compares two timestamps, and returns TRUE if one
 *	happened before two.  Note that this formulation
 *	works when the timestamp wraps around at 2^32,
 *	as long as one and two aren't too far apart.
 */

#define IP_TIMESTAMP_ORDER(one, two)    ((int) ((one) - (two)) < 0)

static inline void
require_ip_active(ipc_port_t port)
{
	if (!ip_active(port)) {
		panic("Using inactive port %p", port);
	}
}

static inline kern_return_t
ipc_port_translate(
	ipc_space_t                     space,
	mach_port_name_t                name,
	mach_port_right_t               right,
	ipc_port_t                     *portp)
{
	ipc_object_t object;
	kern_return_t kr;

	kr = ipc_object_translate(space, name, right, &object);
	*portp = (kr == KERN_SUCCESS) ? ip_object_to_port(object) : IP_NULL;
	return kr;
}

#define ipc_port_translate_receive(space, name, portp)                  \
	ipc_port_translate((space), (name), MACH_PORT_RIGHT_RECEIVE, portp)

#define ipc_port_translate_send(space, name, portp)                     \
	ipc_port_translate((space), (name), MACH_PORT_RIGHT_SEND, portp)

/* Allocate a notification request slot */
#if IMPORTANCE_INHERITANCE
extern kern_return_t
ipc_port_request_alloc(
	ipc_port_t                      port,
	mach_port_name_t                name,
	ipc_port_t                      soright,
	boolean_t                       send_possible,
	boolean_t                       immediate,
	ipc_port_request_index_t        *indexp,
	boolean_t                       *importantp);
#else
extern kern_return_t
ipc_port_request_alloc(
	ipc_port_t                      port,
	mach_port_name_t                name,
	ipc_port_t                      soright,
	boolean_t                       send_possible,
	boolean_t                       immediate,
	ipc_port_request_index_t        *indexp);
#endif /* IMPORTANCE_INHERITANCE */

/* Grow one of a port's tables of notifcation requests */
extern kern_return_t ipc_port_request_grow(
	ipc_port_t                      port,
	ipc_table_elems_t               target_size);

/* Return the type(s) of notification requests outstanding */
extern mach_port_type_t ipc_port_request_type(
	ipc_port_t                      port,
	mach_port_name_t                name,
	ipc_port_request_index_t        index);

/* Cancel a notification request and return the send-once right */
extern ipc_port_t ipc_port_request_cancel(
	ipc_port_t                      port,
	mach_port_name_t                name,
	ipc_port_request_index_t        index);

/* Arm any delayed send-possible notification */
extern boolean_t ipc_port_request_sparm(
	ipc_port_t                port,
	mach_port_name_t          name,
	ipc_port_request_index_t  index,
	mach_msg_option_t         option,
	mach_msg_priority_t       override);

/* Make a port-deleted request */
extern void ipc_port_pdrequest(
	ipc_port_t      port,
	ipc_port_t      notify,
	ipc_port_t      *previousp);

/* Make a no-senders request */
extern void ipc_port_nsrequest(
	ipc_port_t              port,
	mach_port_mscount_t     sync,
	ipc_port_t              notify,
	ipc_port_t              *previousp);

/* Prepare a receive right for transmission/destruction */
extern boolean_t ipc_port_clear_receiver(
	ipc_port_t              port,
	boolean_t               should_destroy);

__options_decl(ipc_port_init_flags_t, uint32_t, {
	IPC_PORT_INIT_NONE            = 0x00000000,
	IPC_PORT_INIT_MAKE_SEND_RIGHT = 0x00000001,
	IPC_PORT_INIT_MESSAGE_QUEUE   = 0x00000002,
	IPC_PORT_INIT_SPECIAL_REPLY   = 0x00000004,
});

/* Initialize a newly-allocated port */
extern void ipc_port_init(
	ipc_port_t              port,
	ipc_space_t             space,
	ipc_port_init_flags_t   flags,
	mach_port_name_t        name);

/* Allocate a port */
extern kern_return_t ipc_port_alloc(
	ipc_space_t             space,
	ipc_port_init_flags_t   flags,
	mach_port_name_t        *namep,
	ipc_port_t              *portp);

/* Allocate a port, with a specific name */
extern kern_return_t ipc_port_alloc_name(
	ipc_space_t             space,
	ipc_port_init_flags_t   flags,
	mach_port_name_t        name,
	ipc_port_t              *portp);

/* Generate dead name notifications */
extern void ipc_port_dnnotify(
	ipc_port_t              port);

/* Generate send-possible notifications */
extern void ipc_port_spnotify(
	ipc_port_t              port);

/* Destroy a port */
extern void ipc_port_destroy(
	ipc_port_t      port);

/* Check if queueing "port" in a message for "dest" would create a circular
 *  group of ports and messages */
extern boolean_t
ipc_port_check_circularity(
	ipc_port_t      port,
	ipc_port_t      dest);

#if IMPORTANCE_INHERITANCE

enum {
	IPID_OPTION_NORMAL       = 0, /* normal boost */
	IPID_OPTION_SENDPOSSIBLE = 1, /* send-possible induced boost */
};

/* link the destination port with special reply port */
void
ipc_port_link_special_reply_port(
	ipc_port_t special_reply_port,
	ipc_port_t dest_port,
	boolean_t sync_bootstrap_checkin);

#define IPC_PORT_ADJUST_SR_NONE                      0
#define IPC_PORT_ADJUST_SR_ALLOW_SYNC_LINKAGE        0x1
#define IPC_PORT_ADJUST_SR_LINK_WORKLOOP             0x2
#define IPC_PORT_ADJUST_UNLINK_THREAD                0x4
#define IPC_PORT_ADJUST_SR_RECEIVED_MSG              0x8
#define IPC_PORT_ADJUST_SR_ENABLE_EVENT              0x10
#define IPC_PORT_ADJUST_RESET_BOOSTRAP_CHECKIN       0x20

void
ipc_special_reply_port_bits_reset(ipc_port_t special_reply_port);

void
ipc_special_reply_port_msg_sent(ipc_port_t special_reply_port);

void
ipc_special_reply_port_msg_sent(ipc_port_t special_reply_port);

/* Adjust special reply port linkage */
void
ipc_port_adjust_special_reply_port_locked(
	ipc_port_t special_reply_port,
	struct knote *kn,
	uint8_t flags,
	boolean_t get_turnstile);

void
ipc_port_adjust_sync_link_state_locked(
	ipc_port_t port,
	int sync_link_state,
	turnstile_inheritor_t inheritor);

/* Adjust special reply port linkage */
void
ipc_port_adjust_special_reply_port(
	ipc_port_t special_reply_port,
	uint8_t flags);

void
ipc_port_adjust_port_locked(
	ipc_port_t port,
	struct knote *kn,
	boolean_t sync_bootstrap_checkin);

void
ipc_port_clear_sync_rcv_thread_boost_locked(
	ipc_port_t port);

kern_return_t
ipc_port_add_watchport_elem_locked(
	ipc_port_t                 port,
	struct task_watchport_elem *watchport_elem,
	struct task_watchport_elem **old_elem);

kern_return_t
ipc_port_clear_watchport_elem_internal_conditional_locked(
	ipc_port_t                 port,
	struct task_watchport_elem *watchport_elem);

kern_return_t
ipc_port_replace_watchport_elem_conditional_locked(
	ipc_port_t                 port,
	struct task_watchport_elem *old_watchport_elem,
	struct task_watchport_elem *new_watchport_elem);

struct task_watchport_elem *
ipc_port_clear_watchport_elem_internal(
	ipc_port_t                 port);

void
ipc_port_send_turnstile_prepare(ipc_port_t port);

void
ipc_port_send_turnstile_complete(ipc_port_t port);

struct waitq *
ipc_port_rcv_turnstile_waitq(struct waitq *waitq);

/* apply importance delta to port only */
extern mach_port_delta_t
ipc_port_impcount_delta(
	ipc_port_t              port,
	mach_port_delta_t       delta,
	ipc_port_t              base);

/* apply importance delta to port, and return task importance for update */
extern boolean_t
ipc_port_importance_delta_internal(
	ipc_port_t              port,
	natural_t               options,
	mach_port_delta_t       *deltap,
	ipc_importance_task_t   *imp_task);

/* Apply an importance delta to a port and reflect change in receiver task */
extern boolean_t
ipc_port_importance_delta(
	ipc_port_t              port,
	natural_t               options,
	mach_port_delta_t       delta);
#endif /* IMPORTANCE_INHERITANCE */

/* Make a naked send right from a receive right - port locked and active */
extern ipc_port_t ipc_port_make_send_locked(
	ipc_port_t      port);

/* Make a naked send right from a receive right */
extern ipc_port_t ipc_port_make_send(
	ipc_port_t      port);

/* Make a naked send right from another naked send right - port locked and active */
extern void ipc_port_copy_send_locked(
	ipc_port_t      port);

/* Make a naked send right from another naked send right */
extern ipc_port_t ipc_port_copy_send(
	ipc_port_t      port);

/* Copyout a naked send right */
extern mach_port_name_t ipc_port_copyout_send(
	ipc_port_t      sright,
	ipc_space_t     space);

#endif /* MACH_KERNEL_PRIVATE */

#if KERNEL_PRIVATE

/* Release a (valid) naked send right */
extern void ipc_port_release_send(
	ipc_port_t      port);

extern void ipc_port_reference(
	ipc_port_t port);

extern void ipc_port_release(
	ipc_port_t port);

#endif /* KERNEL_PRIVATE */

#ifdef MACH_KERNEL_PRIVATE

/* Make a naked send-once right from a locked and active receive right */
extern ipc_port_t ipc_port_make_sonce_locked(
	ipc_port_t      port);

/* Make a naked send-once right from a receive right */
extern ipc_port_t ipc_port_make_sonce(
	ipc_port_t      port);

/* Release a naked send-once right */
extern void ipc_port_release_sonce(
	ipc_port_t      port);

/* Release a naked (in limbo or in transit) receive right */
extern void ipc_port_release_receive(
	ipc_port_t      port);

/* finalize the destruction of a port before it gets freed */
extern void ipc_port_finalize(
	ipc_port_t      port);

/* Allocate a port in a special space */
extern ipc_port_t ipc_port_alloc_special(
	ipc_space_t             space,
	ipc_port_init_flags_t   flags);

/* Deallocate a port in a special space */
extern void ipc_port_dealloc_special(
	ipc_port_t      port,
	ipc_space_t     space);

#if     MACH_ASSERT
/* Track low-level port deallocation */
extern void ipc_port_track_dealloc(
	ipc_port_t      port);

/* Initialize general port debugging state */
extern void ipc_port_debug_init(void);
#endif  /* MACH_ASSERT */

extern void ipc_port_recv_update_inheritor(ipc_port_t port,
    struct turnstile *turnstile,
    turnstile_update_flags_t flags);

extern void ipc_port_send_update_inheritor(ipc_port_t port,
    struct turnstile *turnstile,
    turnstile_update_flags_t flags);

#define ipc_port_alloc_kernel()         \
	        ipc_port_alloc_special(ipc_space_kernel, IPC_PORT_INIT_NONE)
#define ipc_port_dealloc_kernel(port)   \
	        ipc_port_dealloc_special((port), ipc_space_kernel)

#define ipc_port_alloc_reply()          \
	        ipc_port_alloc_special(ipc_space_reply, IPC_PORT_INIT_MESSAGE_QUEUE)
#define ipc_port_dealloc_reply(port)    \
	        ipc_port_dealloc_special((port), ipc_space_reply)

#endif /* MACH_KERNEL_PRIVATE */

#endif  /* _IPC_IPC_PORT_H_ */

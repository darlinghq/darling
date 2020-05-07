/*
 * Copyright (c) 2015-2016 Apple Computer, Inc. All rights reserved.
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
 *	File:	ipc/flipc.h
 *	Author:	Dean Reece
 *	Date:	2016
 *
 *	Definitions for fast local ipc (flipc).
 */

#ifndef _IPC_FLIPC_H_
#define _IPC_FLIPC_H_

#if MACH_KERNEL_PRIVATE && MACH_FLIPC

#include <kern/mach_node.h>
#include <ipc/ipc_kmsg.h>
#include <sys/cdefs.h>

__BEGIN_DECLS


/*** FLIPC Port Declarations ***/

/*  A FLIPC port (flipc_port_t) is a companion structure to ipc_port_t.
 *  Any ipc_port object that is known to the flipc layer has one of these
 *  structures to maintain the state of the port with respect to flipc.
 *  When a port reverts to a purely local object (all rights for the port exist
 *  on a single node) the flipc port companion structure will be de-allocated.
 */

typedef struct flipc_port {
	struct mnl_obj          obj;            // Necessary to be in mnl_name_table[]
	ipc_port_t          lport;  // The associated local ipc_port
	mach_node_t                     hostnode;       // Node holding the recieve right
	uint32_t            peek_count; // How many kmsgs in mq have been peeked
	uint32_t            state:3;// See FPORT_STATE_* defines below
} *flipc_port_t;

#define FPORT_NULL                      ((flipc_port_t) 0UL)
#define FPORT_VALID(fport)      ((fport) != FPORT_NULL)

#define FPORT_STATE_INIT            (0) // Port is being initialized
#define FPORT_STATE_PROXY           (1) // Principal is on another node
#define FPORT_STATE_PRINCIPAL       (2) // Principal is on this node
#define FPORT_STATE_PREPRINCIPAL    (3) // Principal moving to this node
#define FPORT_STATE_POSTPRINCIPAL   (4) // Principal moving to other node
#define FPORT_STATE_DEAD            (5) // Port is being destroyed


/*** FLIPC Node Managment Declarations (used by mach node layer) ***/

extern mach_node_id_t   localnode_id;   // This node's FLIPC id.

/*  The mach node layer calls flipc_init() once before it calls any other
 *  flipc entry points.  Returns KERN_SUCCESS on success; otherwise flipc
 *  is not initialized and cannot be used.
 */
kern_return_t flipc_init(void);

/*  flipc_node_prepare() is called by mach node layer when a remote node is
 *  registered by a link driver.  This is the flipc layer's opportunity to
 *  convert it to a flipc port and hook it into any appropriate structures.
 *  Note that the node is not yet in the mach node table.  Returns KERN_SUCCESS
 *  on success; otherwise node is not prepared and cannot be used.
 */
kern_return_t flipc_node_prepare(mach_node_t node);

/*  flipc_node_retire() is called by mach node layer when a remote node is
 *  terminated by a link driver.  This is the flipc layer's opportunity to
 *  convert it back to a local port and unhook it into any structures.
 *  Returns KERN_SUCCESS on success.
 */
kern_return_t flipc_node_retire(mach_node_t node);


/*** FLIPC Message Declarations (used by mach node layer) ***/

/*	Definition for a flipc ack/nak message.  These messages are sent to the
 *  originating node of a message to ack or nak the message.  Ack'd messages
 *  are destroyed by the originating node (to avoid duplicate delivery).  Nak'd
 *  messages are re-sent to the node specified in <resend_to> (used when a
 *  receive right moves to a different node).  These messages are queued onto
 *  the originating node's control_port and sent along with other ipc traffic.
 */

typedef struct flipc_ack_msg {
	struct mnl_msg  mnl;        // Flipc message starts with mnl message
	mach_node_id_t  resend_to;  // Node ID for resends (if NAK)
	uint8_t         msg_count;  // Number of msgs being ackd/nakd
}   __attribute__((__packed__)) * flipc_ack_msg_t;

#define FLIPC_CMD_ID (0x43504952UL) // msgh_id "RIPC" for FLIPC msgs
#define FLIPC_CMD_IPCMESSAGE    (1) // IPC Msg: <node> is sender; <fname> is dest port
#define FLIPC_CMD_ACKMESSAGE    (2) // <fname> is port being ack'd
#define FLIPC_CMD_NAKMESSAGE    (3) // <fname> is port being nak'd


/*  The node layer calls flipc_msg_to_remote_node() to fetch the next message
 *  for <to_node>.  This function will block until a message is available or the
 *  node is terminated, in which case it returns MNL_MSG_NULL.
 */
mnl_msg_t flipc_msg_to_remote_node(mach_node_t  to_node,
    uint32_t     flags);

/*  The node layer calls flipc_msg_to_remote_node() to post the next message
 *  from <from_node>.  This function will block until a message is available
 *  or the node is terminated, in which case it returns MNL_MSG_NULL.
 */
void flipc_msg_from_node(mach_node_t    from_node,
    mnl_msg_t      msg_arg,
    uint32_t       flags);

/*  The node layer calls flipc_msg_free() to dispose of sent messages that
 *  originated in the FLIPC layer.
 */
void flipc_msg_free(mnl_msg_t   msg,
    uint32_t    flags);


/*** FLIPC Message Declarations (used by mach ipc subsystem) ***/

/*	Ack a message sent by <mqueue> to <fport>.  A new kmsg is allocated and
 *  filled in as an ack (or nak if <delivered> is false), then posted to the
 *  node's contol port.  This will wake the link driver (if sleeping) and cause
 *  the ack to be included with normal IPC traffic.
 *
 *  This function immediately returns if <fport> or <node> is invalid, so it
 *  is safe & quick to call speculatively.
 *
 *	Called from mach ipc_mqueue.c when a flipc-originated message is consumed.
 */
void flipc_msg_ack(mach_node_t  node,
    ipc_mqueue_t mqueue,
    boolean_t    delivered);


__END_DECLS

#endif  // MACH_KERNEL_PRIVATE
#endif  // _IPC_FLIPC_H_

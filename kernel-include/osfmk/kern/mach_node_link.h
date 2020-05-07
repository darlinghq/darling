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
 *  File:   kern/mach_node_link.h
 *  Author: Dean Reece
 *  Date:   2016
 *
 *  This header provides definitions required by Mach Node Link (MNL) drivers.
 *  MNL drivers pass messages between nodes within a host.
 *
 *  The constructs available at the node link level are very basic:
 *  Node IDs (mach_node_id_t) uniquely identify nodes within a host.
 *  MNL Info (mnl_node_info) describe the static characteristics of a node.
 *  MNL Names (mnl_name_t) uniquely identify abjects across all nodes.
 *  MNL Messages (mnl_msg) are passed between nodes (kernels) within a host.
 */

#ifndef _KERN_MACH_NODE_LINK_H_
#define _KERN_MACH_NODE_LINK_H_

#if KERNEL_PRIVATE

#include <sys/cdefs.h>

__BEGIN_DECLS


/*** Node Info Section ***/

typedef int             mach_node_id_t; // Used to uniquely identify a node
extern mach_node_id_t   localnode_id;   // This node's unique id.

/*  An mnl_node struct describes static characteristcs of a node.  The link
 *  driver requests this structure from the mach_node layer and fills out
 *  the fields.  All fields must be filled in (non-zero) before both rx and tx
 *  links are brought up.
 */
typedef struct mnl_node_info {
	mach_node_id_t  node_id;    // The node ID of this node
	uint8_t         datamodel;  // 1==ILP32, 2==LP64 (matches dtrace)
	uint8_t         byteorder;  // See libkern/OSByteOrder.h
	uint32_t        proto_vers_min;// Oldest MNL protocol vers node can accept
	uint32_t        proto_vers_max;// Newest MNL protocol vers node can accept
} __attribute__ ((aligned(8))) * mnl_node_info_t;

#define MNL_NODE_NULL       ((mnl_node_info_t) 0UL)
#define MNL_NODE_VALID(n)   ((n) != MNL_NODE_NULL)
#define MNL_PROTOCOL_V1 (1UL)           // Current Node Link Protocol Version

/*** Mach Node Link Name Section
 *
 *  A node link name (mnl_name_t) is an oqaque value guaranteed unique across
 *  kernel instances on all nodes.
 */
typedef uint64_t    mnl_name_t;

/*** Mach Node Link Message Section ***/

/*  This structure is the header for an MNL Message buffer; the actual buffer
 *  is normally larger, and holds this header followed by the body of the
 *  message to be transmitted over the link.
 *
 *  Note: The <node_id> and <size> fields are in host-native byte order when
 *  passed to mnl_msg_from_node() and from mnl_msg_to_node().
 *  The byte order of these fields as sent over the link is left to the link
 *  specification.  The link drivers on both sides must translate these fields
 *  between the link's byte order and host-native byte order.
 *
 *  The body of the message, however, is treated as a byte-stream and passed
 *  to/from the mach_node layer without any introspection or byte reordering.
 */
typedef struct mnl_msg {
	uint8_t     sub;    //  8b subsystem code
	uint8_t     cmd;    //  8b command code
	uint8_t     qos;    //  8b TODO: Doesn't do anything yet
	uint8_t     flags;  //  8b Command-specific flag byte
	uint32_t    node_id;// 32b id of node that originated message
	mnl_name_t  object; // 64b object ref (use is determined by sub & cmd)
	uint32_t    options;// 32b Currently unused
	uint32_t    size;   // 32b Number of bytes that follow mnl_msg header
}  __attribute__((__packed__)) * mnl_msg_t;


/*  Allocate a mnl_msg struct plus additional payload.  Link drivers are not
 *  required to use this to allocate messages; any wired and mapped kernel
 *  memory is acceptable.
 *
 *  Arguments:
 *    payload   Number of additional bytes to allocate for message payload
 *    flags     Currently unused; 0 should be passed
 *
 *  Return values:
 *    MNL_MSG_NULL:     Allocation failed
 *    *:                Pointer to new mnl_msg struct of requested size
 */
mnl_msg_t mnl_msg_alloc(int payload, uint32_t flags);


/*  Free a mnl_msg struct allocated by mnl_msg_alloc().
 *
 *  Arguments:
 *    msg       Pointer to the message buffer to be freed
 *    flags     Currently unused; 0 should be passed
 */
void mnl_msg_free(mnl_msg_t msg, uint32_t flags);

#define MNL_MSG_NULL            ((mnl_msg_t) 0UL)
#define MNL_MSG_VALID(msg)      ((msg) != MNL_MSG_NULL)
#define MNL_MSG_SIZE            ((vm_offset_t)sizeof(struct mnl_msg))
#define MNL_MSG_PAYLOAD(msg)    ((vm_offset_t)(msg) + MNL_MSG_SIZE)


/*** Mach Node Link Driver Interface Section ***/

/*  The link driver calls this to setup a new (or restarted) node, and to get
 *  an mnl_node_info struct for use as a parameter to other mnl functions.
 *  If MNL_NODE_NULL is returned, the operation failed.  Otherwise, a pointer
 *  to a new mnl_node struct is returned.  The caller should set all fields
 *  in the structure, then call mnl_register() to complete node registration.
 *
 *  Arguments:
 *    nid       The id of the node to be instantiated
 *    flags     Currently unused; 0 should be passed
 *
 *  Return values:
 *    MNL_NODE_NULL:    Operation failed
 *    *:                Pointer to a new mnl_node struct
 */
mnl_node_info_t mnl_instantiate(mach_node_id_t  nid,
    uint32_t        flags);


/*  The link driver calls mnl_register() to complete the node registration
 *  process.  KERN_SUCCESS is returned if registration succeeded, otherwise
 *  an error is returned.
 *
 *  Arguments:
 *    node      Pointer to the node's mnl_node structure
 *    flags     Currently unused; 0 should be passed
 *
 *  Return values:
 *    KERN_SUCCESS:           Registration succeeded
 *    KERN_INVALID_ARGUMENT:  Field(s) in <node> contained unacceptable values
 *    KERN_*:                 Values returned from underlying functions
 */
kern_return_t mnl_register(mnl_node_info_t  node,
    uint32_t         flags);


/*  The link driver calls this to report that the link has been raised in one
 *  or both directions.  If the link is two uni-directional channels, each link
 *  driver will independently call this function, each only raising the link
 *  they are responsible for.  The mach_node layer will not communicate with
 *  the remote node until both rx and tx links are up.
 *
 *  Arguments:
 *    node      Pointer to the node's mnl_node structure
 *    link      Indicates which link(s) are up (see MNL_LINK_* defines)
 *    flags     Currently unused; 0 should be passed
 *
 *  Return values:
 *    KERN_SUCCESS:           Link state changed successfully.
 *    KERN_INVALID_ARGUMENT:  An argument value was not allowed.
 *    KERN_*:                 Values returned from underlying functions.
 */
kern_return_t mnl_set_link_state(mnl_node_info_t    node,
    int                link,
    uint32_t           flags);

#define MNL_LINK_DOWN   (0UL)
#define MNL_LINK_RX     (1UL)
#define MNL_LINK_TX     (2UL)
#define MNL_LINK_UP     (MNL_LINK_RX|MNL_LINK_TX)


/*  The link driver calls this to indicate a node has terminated and is no
 *  longer available for messaging.  This may be due to a crash or an orderly
 *  shutdown, but either way the remote node no longer retains any state about
 *  the remaining nodes.  References held on behalf of the terminated node
 *  will be cleaned up.  After this is called, both the rx and tx links are
 *  marked as down.  If the remote node restarts, the link driver can bring
 *  up the link using mnl_instantiate() again.
 *
 *  Arguments:
 *    node      Pointer to the node's mnl_node structure
 *    flags     Currently unused; 0 should be passed
 *
 *  Return values:
 *    KERN_SUCCESS:           Node was terminated.
 *    KERN_INVALID_ARGUMENT:  Node id was invalid or non-existant.
 *    KERN_*:                 Values returned from underlying functions.
 */
kern_return_t mnl_terminate(mnl_node_info_t node,
    uint32_t        flags);


/*  The link driver calls this to deliver an incoming message.  Note that the
 *  link driver must dispose of the memory pointed to by <msg> after the
 *  function call returns.
 *
 *  Arguments:
 *    node      Pointer to the node's mnl_node structure
 *    msg       Pointer to the message buffer
 *    flags     Currently unused; 0 should be passed
 */
void mnl_msg_from_node(mnl_node_info_t  node,
    mnl_msg_t        msg,
    uint32_t         flags);


/*  The link driver calls this to fetch the next message to transmit.
 *  This function will block until a message is available, or will return
 *  FLIPC_MSG_NULL if the link is to be terminated.  After the caller has
 *  completed the transmission and no longer needs the msg buffer, it should
 *  call mnl_msg_complete().
 *
 *  Arguments:
 *    node      Pointer to the node's mnl_node structure
 *    flags     Currently unused; 0 should be passed
 */
mnl_msg_t mnl_msg_to_node(mnl_node_info_t   node,
    uint32_t          flags);


/*  The link driver calls this to indicate that the specified msg buffer has
 *  been sent over the link and can be deallocated.
 *
 *  Arguments:
 *    node      Pointer to the node's mnl_node structure
 *    msg       Pointer to the message buffer
 *    flags     Currently unused; 0 should be passed
 */
void mnl_msg_complete(mnl_node_info_t   node,
    mnl_msg_t         msg,
    uint32_t          flags);

__END_DECLS

#endif  /* KERNEL_PRIVATE */
#endif  /* _KERN_MACH_NODE_LINK_H_ */

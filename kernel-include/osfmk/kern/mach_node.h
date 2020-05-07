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
 *  File:   kern/mach_node.h
 *  Author: Dean Reece
 *  Date:   2016
 *
 *  Definitions for mach internode communication (used by flipc).
 *  This header is intended for use inside the kernel only.
 */

#ifndef _KERN_MACH_NODE_H_
#define _KERN_MACH_NODE_H_

#if defined(MACH_KERNEL_PRIVATE) || defined(__APPLE_API_PRIVATE)

/*** Mach Node Name Server Section
 *  Definitions shared by the mach_node layer in the kernel and the
 *  node's bootstrap server (noded).
 */

/* This structure describes messages sent from the mach_node layer to the
 * node bootstrap server.
 */
#pragma pack(4)
typedef struct mach_node_server_msg {
	mach_msg_header_t   header;
	uint32_t    identifier; // See FLIPC_SM_* defines
	uint32_t    options;    // Currently unused
	uint32_t    node_id;    // Node number
} *mach_node_server_msg_t;
#pragma pack()

/* This structure describes node registration messages sent from the mach_node
 * layer to the node bootstrap server.
 */
typedef struct mach_node_server_register_msg {
	struct mach_node_server_msg node_header;
	uint8_t     datamodel;  // 1==ILP32, 2==LP64; matches dtrace
	uint8_t     byteorder;  // Uses defines from libkern/OSByteOrder.h
} *mach_node_server_register_msg_t;
#pragma pack()

#define MACH_NODE_SERVER_MSG_ID (0x45444f4eUL)  // msgh_id "NODE" for Node msgs
#define MACH_NODE_SM_REG_LOCAL           (0UL)  // Register the local node
#define MACH_NODE_SM_REG_REMOTE          (1UL)  // Register a remote node

#if defined(__LP64__)
#define LOCAL_DATA_MODEL    (2) // Native data model is LP64
#else
#define LOCAL_DATA_MODEL    (1) // Native data model is ILP32
#endif

#endif


#if MACH_FLIPC && defined(MACH_KERNEL_PRIVATE)

#include <kern/mach_node_link.h>
#include <kern/queue.h>

#include <sys/cdefs.h>

__BEGIN_DECLS

#define MACH_NODES_MAX          (2)         // Must be a power-of-2
#define MACH_NODE_ID_VALID(nid) (((nid) >= 0) && ((nid) < MACH_NODES_MAX))

typedef struct  flipc_node  *flipc_node_t;  // Defined in ipc/flipc.h


/*** Mach Node Section
 *
 *  An instance of mach_node is allocated for each node known to mach.
 *  In-kernel interfaces use a pointer to this structure to refer to a node.
 *  External interfaces and protocols refer to node by id (mach_node_id_t).
 */
typedef struct mach_node *mach_node_t;

struct mach_node {
	/* Static node details, provided by the link driver at registration */
	struct mnl_node_info info;

	lck_spin_t      node_lock_data;

	/* Flags and status word */
	uint32_t        link:2;     // See MNL_LINK* defines
	uint32_t        published:1;// True if node server has send-right
	uint32_t        active:1;   // True if node is up and ready
	uint32_t        suspended:1;// True if node is active but sleeping
	uint32_t        dead:1;     // True if node is dead
	uint32_t        _reserved:26;// Fill out the 32b flags field

	/* port/space/set */
	ipc_space_t     proxy_space;// Kernel special space for proxy rights
	ipc_pset_t      proxy_port_set;// All proxy ports are in this set
	ipc_port_t      bootstrap_port;// Port for which "noded" holds rcv right
	ipc_port_t      control_port;// For control & ack/nak messages

	/* Misc */
	int             proto_vers; // Protocol version in use for this node
	mach_node_t     antecedent; // Pointer to prior encarnation of this node id
};

extern mach_node_t  localnode;      // This node's mach_node_t struct

#define MACH_NODE_NULL              ((mach_node_t) 0UL)
#define MACH_NODE_SIZE              ((vm_offset_t)sizeof(struct mach_node))
#define MACH_NODE_VALID(node)       ((node) != MACH_NODE_NULL)
#define MACH_NODE_ALLOC()           ((mach_node_t)kalloc(MACH_NODE_SIZE))
#define MACH_NODE_FREE(node)        kfree(node, MACH_NODE_SIZE)

#define MACH_NODE_LOCK_INIT(np)     lck_spin_init(&(np)->node_lock_data, \
	                                          &ipc_lck_grp, &ipc_lck_attr)
#define MACH_NODE_LOCK_DESTROY(np)  lck_spin_destroy(&(np)->node_lock_data, \
	                                             &ipc_lck_grp)
#define MACH_NODE_LOCK(np)          lck_spin_lock(&(np)->node_lock_data)
#define MACH_NODE_UNLOCK(np)        lck_spin_unlock(&(np)->node_lock_data)

/*  Gets or allocates a locked mach_node struct for the specified <node_id>.
 *  The current node is locked and returned if it is not dead, or if it is dead
 *  and <alloc_if_dead> is false.  A new node struct is allocated, locked and
 *  returned if the node is dead and <alloc_if_dead> is true, or if the node
 *  is absent and <alloc_if_absent> is true.  MACH_NODE_NULL is returned if
 *  the node is absent and <alloc_if_absent> is false.  MACH_NODE_NULL is also
 *  returned if a new node structure was not able to be allocated.
 */
mach_node_t
mach_node_for_id_locked(mach_node_id_t  node_id,
    boolean_t       alloc_if_dead,
    boolean_t       alloc_if_absent);


/*** Mach Node Link Name Section
 *
 *  A node link name (mnl_name_t) is an oqaque value guaranteed unique across
 *  kernel instances on all nodes.  This guarantee requires that node ids not
 *  be recycled.
 *
 *  Names 0..(MACH_NODES_MAX-1) represent null (invalid) names
 *  Names MACH_NODES_MAX..(MACH_NODES_MAX*2-1) represent bootstrap names
 *  Names >=(MACH_NODES_MAX*2) represent normal names.
 */

/*  Allocate a new unique name and return it.
 *  Dispose of this with mnl_name_free().
 *  Returns MNL_NAME_NULL on failure.
 */
extern mnl_name_t   mnl_name_alloc(void);

/*  Deallocate a unique name that was allocated via mnl_name_alloc().
 */
extern void mnl_name_free(mnl_name_t name);

/*  This macro is used to convert a node id to a bootstrap port name.
 */
#define MNL_NAME_BOOTSTRAP(nid) ((mnl_name_t) MACH_NODES_MAX | (nid))
#define MNL_NAME_NULL ((mnl_name_t) 0UL)
#define MNL_NAME_VALID(obj) ((obj) >= MACH_NODES_MAX)


/*  The mnl hash table may optionally be used by clients to associate mnl_names
 *  with objects.  Objects to be stored in the hash table must start with an
 *  instance of struct mnk_obj.  It is up to clients of the hash table to
 *  allocate and free the actual objects being stored.
 */
typedef struct mnl_obj {
	queue_chain_t   links;// List of mnk_name_obj (See kern/queue.h "Method 1")
	mnl_name_t      name;// Unique mnl_name
} *mnl_obj_t;

#define MNL_OBJ_NULL        ((mnl_obj_t) 0UL)
#define MNL_OBJ_VALID(obj)  ((obj) != MNL_OBJ_NULL)


/*  Initialize the data structures in the mnl_obj structure at the head of the
 *  provided object.  This should be called on an object before it is passed to
 *  any other mnl_obj* routine.
 */
void mnl_obj_init(mnl_obj_t obj);

/*  Search the local node's hash table for the object associated with a
 *  mnl_name_t and return it.  Returns MNL_NAME_NULL on failure.
 */
mnl_obj_t mnl_obj_lookup(mnl_name_t name);

/*  Search the local node's hash table for the object associated with a
 *  mnl_name_t and remove it.  The pointer to the removed object is returned so
 *  that the caller can appropriately dispose of the object.
 *  Returns MNL_NAME_NULL on failure.
 */
mnl_obj_t mnl_obj_remove(mnl_name_t name);

/*  Insert an object into the locak node's hash table.  If the name of the
 *  provided object is MNL_NAME_NULL then a new mnl_name is allocated and
 *  assigned to the object.  Returns KERN_SUCCESS, or KERN_NAME_EXISTS if
 *  an object associated with that name is already in the hash table.
 */
kern_return_t mnl_obj_insert(mnl_obj_t obj);


/*** Mach Node Link Message Section ***
 *
 *  Struct mnl_msg is only the header for a mnl_msg buffer;
 *  the actual buffer is normally larger.  The rest of the buffer
 *  holds the body of the message to be transmitted over the link.
 *
 *  Note: A mnl_msg received over a link will be in the byte-order of the
 *  node that send it.  fname and size must be corrected to the hosts' native
 *  byte order by the link driver before it is sent up to the flipc layer.
 *  However, the link driver should not attempt to adjust the data model or
 *  byte order of the payload that follows the mnl_msg header - that will
 *  be done by the flipc layer.
 */


/* Values for mnl_msg.sub
 */
#define MACH_NODE_SUB_INVALID   (0) // Never sent
#define MACH_NODE_SUB_NODE      (1) // MNL msg is for node management
#define MACH_NODE_SUB_FLIPC     (2) // MNL msg is for FLIPC subsystem
#define MACH_NODE_SUB_VMSYS     (3) // MNL msg is for VM subsystem


/* Called whenever the node special port changes
 */
void mach_node_port_changed(void);


__END_DECLS

#endif  // MACH_FLIPC && MACH_KERNEL_PRIVATE
#endif  // _KERN_MACH_NODE_H_

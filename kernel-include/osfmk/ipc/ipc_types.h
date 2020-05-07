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

/*
 * Define Basic IPC types available to callers.
 * These are not intended to be used directly, but
 * are used to define other types available through
 * port.h and mach_types.h for in-kernel entities.
 */

#ifndef _IPC_IPC_TYPES_H_
#define _IPC_IPC_TYPES_H_

#include <mach/port.h>
#include <mach/message.h>
#include <mach/mach_types.h>

#ifdef  MACH_KERNEL_PRIVATE

typedef natural_t ipc_table_index_t;    /* index into tables */
typedef natural_t ipc_table_elems_t;    /* size of tables */
typedef natural_t ipc_entry_bits_t;
typedef ipc_table_elems_t ipc_entry_num_t;      /* number of entries */
typedef ipc_table_index_t ipc_port_request_index_t;

typedef mach_port_name_t mach_port_index_t;             /* index values */
typedef mach_port_name_t mach_port_gen_t;               /* generation numbers */

typedef struct ipc_entry *ipc_entry_t;

typedef struct ipc_table_size *ipc_table_size_t;
typedef struct ipc_port_request *ipc_port_request_t;
typedef struct ipc_pset *ipc_pset_t;
typedef struct ipc_kmsg *ipc_kmsg_t;
typedef uint8_t sync_qos_count_t;

#define IE_NULL ((ipc_entry_t) 0)

#define ITS_NULL        ((ipc_table_size_t) 0)
#define ITS_SIZE_NONE   ((ipc_table_elems_t) -1)
#define IPR_NULL        ((ipc_port_request_t) 0)
#define IPS_NULL        ((ipc_pset_t) 0)
#define IKM_NULL        ((ipc_kmsg_t) 0)

typedef void (*mach_msg_continue_t)(mach_msg_return_t); /* after wakeup */
#define MACH_MSG_CONTINUE_NULL  ((mach_msg_continue_t) 0)

typedef struct ipc_importance_elem *ipc_importance_elem_t;
#define IIE_NULL        ((ipc_importance_elem_t)0)

typedef struct ipc_importance_task *ipc_importance_task_t;
#define IIT_NULL        ((ipc_importance_task_t)0)

typedef struct ipc_importance_inherit *ipc_importance_inherit_t;
#define III_NULL        ((ipc_importance_inherit_t)0)


#else   /* MACH_KERNEL_PRIVATE */

struct ipc_object;

#endif  /* MACH_KERNEL_PRIVATE */

typedef struct ipc_object       *ipc_object_t;

#define IPC_OBJECT_NULL         ((ipc_object_t) 0)
#define IPC_OBJECT_DEAD         ((ipc_object_t)~0)
#define IPC_OBJECT_VALID(io)    (((io) != IPC_OBJECT_NULL) && \
	                         ((io) != IPC_OBJECT_DEAD))

#endif  /* _IPC_IPC_TYPES_H_ */

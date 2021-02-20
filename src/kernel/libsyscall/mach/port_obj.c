/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
 * Define a service to map from a kernel-generated port name
 * to server-defined "type" and "value" data to be associated
 * with the port.
 */
#include <mach/port_obj.h>
#include <mach/mach.h>

#define DEFAULT_TABLE_SIZE      (64 * 1024)

struct port_obj_tentry *port_obj_table;
int port_obj_table_size = DEFAULT_TABLE_SIZE;

void
port_obj_init(int maxsize)
{
	kern_return_t kr;

	kr = vm_allocate(mach_task_self_,
	    (vm_offset_t *)&port_obj_table,
	    (vm_size_t)(maxsize * sizeof(*port_obj_table)),
	    TRUE);
	if (kr != KERN_SUCCESS) {
		panic("port_obj_init: can't vm_allocate");
	}
}

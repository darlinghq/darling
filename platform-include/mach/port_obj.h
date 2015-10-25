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

#ifndef PORT_OBJ_H
#define PORT_OBJ_H

#include <mach/port.h>

struct port_obj_tentry {
	void *pos_value;
	int pos_type;
};

#include <sys/cdefs.h>

__BEGIN_DECLS
extern void port_obj_init(int);
__END_DECLS

extern struct port_obj_tentry *port_obj_table;
extern int port_obj_table_size;

#ifndef PORT_OBJ_ASSERT

#define port_set_obj_value_type(pname, value, type)	\
do {							\
	int ndx;					\
							\
	if (!port_obj_table)				\
		port_obj_init(port_obj_table_size);	\
	ndx = MACH_PORT_INDEX(pname);			\
	port_obj_table[ndx].pos_value = (value);	\
	port_obj_table[ndx].pos_type = (type);		\
} while (0)

#define port_get_obj_value(pname)			\
	(port_obj_table[MACH_PORT_INDEX(pname)].pos_value)

#define port_get_obj_type(pname)			\
	(port_obj_table[MACH_PORT_INDEX(pname)].pos_type)

#else	/* PORT_OBJ_ASSERT */

#define port_set_obj_value_type(pname, value, type)	\
do {							\
	int ndx;					\
							\
	if (!port_obj_table)				\
		port_obj_init(port_obj_table_size);	\
	ndx = MACH_PORT_INDEX(pname);			\
	assert(ndx > 0);				\
	assert(ndx < port_obj_table_size);		\
	port_obj_table[ndx].pos_value = (value);	\
	port_obj_table[ndx].pos_type = (type);		\
} while (0)

#define port_get_obj_value(pname)				\
	((MACH_PORT_INDEX(pname) < (unsigned)port_obj_table_size) ?	\
	port_obj_table[MACH_PORT_INDEX(pname)].pos_value :	\
	(panic("port_get_obj_value: index too big"), (void *)-1))

#define port_get_obj_type(pname)				\
	((MACH_PORT_INDEX(pname) < (unsigned)port_obj_table_size) ?	\
	port_obj_table[MACH_PORT_INDEX(pname)].pos_type :	\
	(panic("port_get_obj_type: index too big"), -1))

#endif	/* PORT_OBJ_ASSERT */

#endif	/* PORT_OBJ_H */

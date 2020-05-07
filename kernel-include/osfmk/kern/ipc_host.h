/*
 * Copyright (c) 2000-2007 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 */

#ifndef _KERN_IPC_HOST_H_
#define _KERN_IPC_HOST_H_

#include <mach/port.h>
#include <kern/kern_types.h>

/* Initialize IPC host services */
extern void ipc_host_init(void);

/* Initialize ipc access to processor by allocating a port */
extern void ipc_processor_init(
	processor_t     processor);

/* Enable ipc control of processor by setting port object */
extern void ipc_processor_enable(
	processor_t     processor);

/* Initialize ipc control of a processor set */
extern void ipc_pset_init(
	processor_set_t         pset);

/* Enable ipc access to a processor set */
extern void ipc_pset_enable(
	processor_set_t         pset);

/* Initialize ipc control of a clock */
extern void ipc_clock_init(
	clock_t         clock);

/* Enable ipc access to a clock */
extern void ipc_clock_enable(
	clock_t         clock);

/* Convert from a port to a clock */
extern clock_t convert_port_to_clock(
	ipc_port_t      port);

/* Convert from a port to a clock control */
extern clock_t convert_port_to_clock_ctrl(
	ipc_port_t      port);

/* Convert from a clock to a port */
extern ipc_port_t convert_clock_to_port(
	clock_t         clock);

/* Convert from a clock control to a port */
extern ipc_port_t convert_clock_ctrl_to_port(
	clock_t         clock);

/* Convert from a clock name to a clock pointer */
extern clock_t port_name_to_clock(
	mach_port_name_t clock_name);

/* Convert from a port to a host */
extern host_t convert_port_to_host(
	ipc_port_t      port);

/* Convert from a port to a host privilege port */
extern host_t convert_port_to_host_priv(
	ipc_port_t      port);

/*  Convert from a port to a host paging port */
extern host_t convert_port_to_host_paging(
	ipc_port_t      port);

/* Convert from a host to a port */
extern ipc_port_t convert_host_to_port(
	host_t          host);

/* Convert from a port to a processor */
extern processor_t convert_port_to_processor(
	ipc_port_t      port);

/* Convert from a processor to a port */
extern ipc_port_t convert_processor_to_port(
	processor_t     processor);

/* Convert from a port to a processor set */
extern processor_set_t convert_port_to_pset(
	ipc_port_t      port);

/* Convert from a port to a processor set name */
extern processor_set_t convert_port_to_pset_name(
	ipc_port_t      port);

/* Convert from a processor set to a port */
extern ipc_port_t convert_pset_to_port(
	processor_set_t         processor);

/* Convert from a processor set name to a port */
extern ipc_port_t convert_pset_name_to_port(
	processor_set_t         processor);

/* Convert from a port to a host security port */
extern host_t convert_port_to_host_security(
	ipc_port_t      port);


#endif  /* _KERN_IPC_HOST_H_ */

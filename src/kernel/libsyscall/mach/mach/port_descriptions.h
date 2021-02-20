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
#ifndef _MACH_PORT_DESCRIPTIONS_
#define _MACH_PORT_DESCRIPTIONS_

#include <sys/cdefs.h>

__BEGIN_DECLS

/*
 * Returns a string describing the host special port offset provided, or NULL if
 * the provided offset is not a host special port offset.
 */
const char *mach_host_special_port_description(int offset);

/*
 * Returns a string describing the task special port offset provided, or NULL if
 * the provided offset is not a task special port offset.
 */
const char *mach_task_special_port_description(int offset);

/*
 * Returns the port for the given identifier of a host special port.  For
 * instance, passing "HOST_PRIV_PORT" would return 1.
 *
 * Returns -1 on error.
 */
int mach_host_special_port_for_id(const char *id);

/*
 * Returns the port for the given identifier of a task special port.
 *
 * Returns -1 on error.
 */
int mach_task_special_port_for_id(const char *id);

__END_DECLS

#endif /* !defined(_MACH_PORT_DESCRIPTIONS_) */

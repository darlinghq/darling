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
 * Copyright (c) 2000 Apple Computer, Inc.  All rights reserved.
 *
 * HISTORY
 *
 * 31 August 2000 (debo)
 *  Created.
 */

#ifndef _MACH_MK_TIMER_H_
#define _MACH_MK_TIMER_H_

#include <mach/mach_time.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

mach_port_name_t        mk_timer_create(void);

kern_return_t   mk_timer_destroy(
	mach_port_name_t        name);

kern_return_t   mk_timer_arm(
	mach_port_name_t        name,
	uint64_t                        expire_time);

kern_return_t   mk_timer_cancel(
	mach_port_name_t        name,
	uint64_t                        *result_time);
#define MK_TIMER_NORMAL (0)
#define MK_TIMER_CRITICAL (1)
kern_return_t   mk_timer_arm_leeway(
	mach_port_name_t        name,
	uint64_t                mk_timer_flags,
	uint64_t                mk_timer_expire_time,
	uint64_t                mk_timer_leeway);

__END_DECLS

#pragma pack(4)
struct mk_timer_expire_msg {
	mach_msg_header_t       header;
	uint64_t                        unused[3];
};
#pragma pack()

typedef struct mk_timer_expire_msg              mk_timer_expire_msg_t;

#endif /* _MACH_MK_TIMER_H_ */

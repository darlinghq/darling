/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
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
#ifndef _KERN_ARCADE_H_
#define _KERN_ARCADE_H_

#include <mach/mach_types.h>
#include <kern/kern_types.h>

#include <libkern/section_keywords.h>


#if XNU_KERNEL_PRIVATE

struct arcade_register;

extern void arcade_init(void);

extern void arcade_ast(thread_t thread);

extern void arcade_prepare(task_t task, thread_t thread);

extern void arcade_register_notify(mach_msg_header_t *msg);

extern void arcade_register_reference(arcade_register_t arcade_reg);

extern void arcade_register_release(arcade_register_t arcade_reg);

extern mach_port_t convert_arcade_register_to_port(arcade_register_t arcade_reg);

extern arcade_register_t convert_port_to_arcade_register(mach_port_t port);

#endif /* XNU_KERNEL_PRIVATE */

#endif /* _KERN_ARCADE_H_ */

/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
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

#ifndef __EXC_CATCHER_H
#define __EXC_CATCHER_H

#include "_libkernel_init.h"

typedef kern_return_t (*_libkernel_exc_raise_func_t)(mach_port_t,
    mach_port_t,
    mach_port_t,
    exception_type_t,
    exception_data_t,
    mach_msg_type_number_t);

typedef kern_return_t (*_libkernel_exc_raise_state_func_t)(mach_port_t,
    exception_type_t,
    exception_data_t,
    mach_msg_type_number_t,
    int *,
    thread_state_t,
    mach_msg_type_number_t,
    thread_state_t,
    mach_msg_type_number_t *);

typedef kern_return_t (*_libkernel_exec_raise_state_identity_t)(mach_port_t,
    mach_port_t, mach_port_t,
    exception_type_t,
    exception_data_t,
    mach_msg_type_number_t,
    int *, thread_state_t,
    mach_msg_type_number_t,
    thread_state_t,
    mach_msg_type_number_t *);

#define RTLD_DEFAULT    ((void *) -2)
extern void* (*_dlsym)(void*, const char*);

#endif // __EXC_CATCHER_H

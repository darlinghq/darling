/*
 * Copyright (c) 2000-2004 Apple Computer, Inc. All rights reserved.
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
/*!
 *       @header init.h
 *       This header defines an API to register a function that will be called when
 *       the network stack is being initialized. This gives a kernel extensions an
 *       opportunity to install filters before sockets are created and network
 *       operations occur.
 */
#ifndef _NET_INIT_H_
#define _NET_INIT_H_
#include <sys/kernel_types.h>

/*!
 *       @typedef net_init_func_ptr
 *       @discussion net_init_func_ptr will be called once the networking stack
 *               initialized and before network operations occur.
 */
typedef void    (*net_init_func_ptr)(void);

/*!
 *       @function net_init_add
 *       @discussion Add a function to be called during network initialization. Your
 *               kext must not unload until the function you  register is called if
 *               net_init_add returns success.
 *       @param init_func A pointer to a function to be called when the stack is
 *               initialized.
 *       @result	EINVAL - the init_func value was NULL.
 *                       EALREADY - the network has already been initialized
 *                       ENOMEM - there was not enough memory to perform this operation
 *                       0 - success
 */
errno_t net_init_add(net_init_func_ptr  init_func);

#ifdef BSD_KERNEL_PRIVATE
/* net_init_run is called from bsd_init */
extern void net_init_run(void);
#endif /* BSD_KERNEL_PRIVATE */

#endif /* _NET_INIT_H_ */

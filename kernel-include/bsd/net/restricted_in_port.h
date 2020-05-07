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

#ifndef _NETINET_IN_RESTRICTED_PORT_H_
#define _NETINET_IN_RESTRICTED_PORT_H_

#ifdef BSD_KERNEL_PRIVATE

#include <kern/bits.h>

#define PORT_FLAGS_LISTENER 0x00
#define PORT_FLAGS_BSD      0x02
#define PORT_FLAGS_PF       0x03
#define PORT_FLAGS_MAX      0x03

/*
 * the port in network byte order
 */
#define IS_RESTRICTED_IN_PORT(x) (bitmap_test(restricted_port_bitmap, ntohs((uint16_t)(x))))

extern bitmap_t *restricted_port_bitmap;

extern void restricted_in_port_init(void);

/*
 * The port must be in network byte order
 */
extern bool current_task_can_use_restricted_in_port(in_port_t port, uint8_t protocol, uint32_t port_flags);

#endif /* BSD_KERNEL_PRIVATE */

#endif /* _NETINET_IN_RESTRICTED_PORT_H_ */

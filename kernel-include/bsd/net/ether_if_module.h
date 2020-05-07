/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#ifndef _NET_ETHER_IF_MODULE_H
#define _NET_ETHER_IF_MODULE_H

#include <net/dlil.h>

extern errno_t ether_attach_inet(ifnet_t ifp, protocol_family_t protocol_family);
extern void ether_detach_inet(ifnet_t ifp, protocol_family_t protocol_family);
extern errno_t ether_attach_inet6(ifnet_t ifp, protocol_family_t protocol_family);
extern void ether_detach_inet6(ifnet_t ifp, protocol_family_t protocol_family);
extern errno_t ether_attach_at(struct ifnet *ifp, protocol_family_t proto_family);
extern void ether_detach_at(struct ifnet *ifp, protocol_family_t proto_family);

#endif /* _NET_ETHER_IF_MODULE_H */

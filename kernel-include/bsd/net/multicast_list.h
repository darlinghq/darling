/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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

#ifndef _NET_MULTICAST_LIST_H
#define _NET_MULTICAST_LIST_H

#include <sys/queue.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/kpi_interface.h>

/*
 * multicast_util.h:
 * - keep track of multicast addresses on one device for programming on
 *   another (VLAN, BOND)
 */
struct multicast_entry {
	SLIST_ENTRY(multicast_entry)    mc_entries;
	ifmultiaddr_t                   mc_ifma;
};
SLIST_HEAD(multicast_list, multicast_entry);

void
multicast_list_init(struct multicast_list * mc_list);

int
multicast_list_program(struct multicast_list * mc_list,
    struct ifnet * source_ifp,
    struct ifnet * target_ifp);
int
multicast_list_remove(struct multicast_list * mc_list);

#endif /* _NET_MULTICAST_LIST_H */

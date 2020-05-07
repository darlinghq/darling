/*
 * Copyright (c) 2017-2018 Apple Inc. All rights reserved.
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


#ifndef _NET_IF_PORT_USED_H_
#define _NET_IF_PORT_USED_H_

#ifdef PRIVATE

#include <sys/types.h>
#include <stdint.h>
#include <sys/proc.h>
#include <sys/queue.h>
#include <sys/_types/_timeval32.h>
#include <netinet/in.h>
#include <uuid/uuid.h>

#define IP_PORTRANGE_SIZE 65536

/*
 * The sysctl "net.link.generic.system.port_used.list" returns:
 *  - one "struct xnpigen" as a preamble
 *  - zero or more "struct net_port_info" according to xng_npi_count
 *
 * The list may contain information several interfaces if several drivers
 * queried the list of port to offload
 *
 * The same local port may have more than one "struct net_port_info" on
 * a given interface, for example when a local server has mutiple clients
 * connections
 */

struct xnpigen {
	uint32_t        xng_len; /* length of this data structure */
	uint32_t        xng_gen; /* how many times the list was built */
	uint32_t        xng_npi_count; /* number of net_port_info following */
	uint32_t        xng_npi_size; /* number of struct net_port_info  */
	uuid_t          xng_wakeuuid; /* WakeUUID when list was built */
};

union in_addr_4_6 {
	struct in_addr  _in_a_4;
	struct in6_addr _in_a_6;
};

#define NPIF_IPV4       0x0001
#define NPIF_IPV6       0x0002
#define NPIF_TCP        0x0004
#define NPIF_UDP        0x0008
#define NPIF_DELEGATED  0x0010
#define NPIF_SOCKET     0x0020
#define NPIF_CHANNEL    0x0040
#define NPIF_LISTEN     0x0080

struct net_port_info {
	uint16_t                npi_if_index;
	uint16_t                npi_flags;
	struct timeval32        npi_timestamp; /* when passed to driver */
	uuid_t                  npi_flow_uuid;
	in_port_t               npi_local_port; /* network byte order */
	in_port_t               npi_foreign_port; /* network byte order */
	union in_addr_4_6       npi_local_addr_;
	union in_addr_4_6       npi_foreign_addr_;
	pid_t                   npi_owner_pid;
	pid_t                   npi_effective_pid;
	char                    npi_owner_pname[MAXCOMLEN + 1];
	char                    npi_effective_pname[MAXCOMLEN + 1];
};

#define npi_local_addr_in npi_local_addr_._in_a_4
#define npi_foreign_addr_in npi_foreign_addr_._in_a_4

#define npi_local_addr_in6 npi_local_addr_._in_a_6
#define npi_foreign_addr_in6 npi_foreign_addr_._in_a_6

#ifdef XNU_KERNEL_PRIVATE

void if_ports_used_init(void);

void if_ports_used_update_wakeuuid(struct ifnet *);

struct inpcb;
void if_ports_used_add_inpcb(const uint32_t ifindex, const struct inpcb *inp);


#endif /* XNU_KERNEL_PRIVATE */
#endif /* PRIVATE */

#endif /* _NET_IF_PORT_USED_H_ */

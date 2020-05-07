/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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

#ifndef __NET_API_STATS__
#define __NET_API_STATS__

#ifdef PRIVATE
#include <stdint.h>

/*
 * net_api_stats counts the usage of the networking APIs
 *
 * Note: we are using signed 64 bit values to detect and prevent wrap around
 */
struct net_api_stats {
	/*
	 * Interface Filters
	 */
	int64_t nas_iflt_attach_count;  // Currently attached
	int64_t nas_iflt_attach_total;  // Total number of attachments
	int64_t nas_iflt_attach_os_total;

	/*
	 * IP Filters
	 */
	int64_t nas_ipf_add_count;      // Currently attached
	int64_t nas_ipf_add_total;      // Total number of attachments
	int64_t nas_ipf_add_os_total;

	/*
	 * Socket Filters
	 */
	int64_t nas_sfltr_register_count;       // Currently attached
	int64_t nas_sfltr_register_total;       // Total number of attachments
	int64_t nas_sfltr_register_os_total;

	/*
	 * Sockets
	 */
	int64_t nas_socket_alloc_total;
	int64_t nas_socket_in_kernel_total;
	int64_t nas_socket_in_kernel_os_total;
	int64_t nas_socket_necp_clientuuid_total;

	/*
	 * Sockets per protocol domains
	 */
	int64_t nas_socket_domain_local_total;
	int64_t nas_socket_domain_route_total;
	int64_t nas_socket_domain_inet_total;
	int64_t nas_socket_domain_inet6_total;
	int64_t nas_socket_domain_system_total;
	int64_t nas_socket_domain_multipath_total;
	int64_t nas_socket_domain_key_total;
	int64_t nas_socket_domain_ndrv_total;
	int64_t nas_socket_domain_other_total;

	/*
	 * Sockets per domain and type
	 */
	int64_t nas_socket_inet_stream_total;
	int64_t nas_socket_inet_dgram_total;
	int64_t nas_socket_inet_dgram_connected;
	int64_t nas_socket_inet_dgram_dns;      // port 53
	int64_t nas_socket_inet_dgram_no_data;  // typically for interface ioctl

	int64_t nas_socket_inet6_stream_total;
	int64_t nas_socket_inet6_dgram_total;
	int64_t nas_socket_inet6_dgram_connected;
	int64_t nas_socket_inet6_dgram_dns;     // port 53
	int64_t nas_socket_inet6_dgram_no_data; // typically for interface ioctl

	/*
	 * Multicast join
	 */
	int64_t nas_socket_mcast_join_total;
	int64_t nas_socket_mcast_join_os_total;

	/*
	 * IPv6 Extension Header Socket API
	 */
	int64_t nas_sock_inet6_stream_exthdr_in;
	int64_t nas_sock_inet6_stream_exthdr_out;
	int64_t nas_sock_inet6_dgram_exthdr_in;
	int64_t nas_sock_inet6_dgram_exthdr_out;

	/*
	 * Nexus flows
	 */
	int64_t nas_nx_flow_inet_stream_total;
	int64_t nas_nx_flow_inet_dgram_total;

	int64_t nas_nx_flow_inet6_stream_total;
	int64_t nas_nx_flow_inet6_dgram_total;

	/*
	 * Interfaces
	 */
	int64_t nas_ifnet_alloc_count;
	int64_t nas_ifnet_alloc_total;
	int64_t nas_ifnet_alloc_os_count;
	int64_t nas_ifnet_alloc_os_total;

	/*
	 * PF
	 */
	int64_t nas_pf_addrule_total;
	int64_t nas_pf_addrule_os;

	/*
	 * vmnet API
	 */
	int64_t nas_vmnet_total;
};

#ifdef XNU_KERNEL_PRIVATE
extern struct net_api_stats net_api_stats;

/*
 * Increment up to the max value of int64_t
 */
#define INC_ATOMIC_INT64_LIM(counter) {                                 \
	int64_t val;                                                    \
	do {                                                            \
	        val = counter;                                          \
	        if (val >= INT64_MAX) {                                 \
	                break;                                          \
	        }                                                       \
	} while (!OSCompareAndSwap64(val, val + 1, &(counter)));        \
}
#endif /* XNU_KERNEL_PRIVATE */

#endif /* PRIVATE */

#endif /* __NET_API_STATS__ */

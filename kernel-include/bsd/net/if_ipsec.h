/*
 * Copyright (c) 2012-2019 Apple Inc. All rights reserved.
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


#ifndef _NET_IF_IPSEC_H_
#define _NET_IF_IPSEC_H_

#ifdef BSD_KERNEL_PRIVATE

#include <sys/kern_control.h>
#include <netinet/ip_var.h>


errno_t ipsec_register_control(void);

/* Helpers */
int ipsec_interface_isvalid(ifnet_t interface);

errno_t ipsec_inject_inbound_packet(ifnet_t     interface, mbuf_t packet);

void ipsec_set_pkthdr_for_interface(ifnet_t interface, mbuf_t packet, int family);

void ipsec_set_ipoa_for_interface(ifnet_t interface, struct ip_out_args *ipoa);

struct ip6_out_args;
void ipsec_set_ip6oa_for_interface(ifnet_t interface, struct ip6_out_args *ip6oa);

#endif

/*
 * Name registered by the ipsec kernel control
 */
#define IPSEC_CONTROL_NAME "com.apple.net.ipsec_control"

/*
 * Socket option names to manage ipsec
 */
#define IPSEC_OPT_FLAGS                                 1
#define IPSEC_OPT_IFNAME                                2
#define IPSEC_OPT_EXT_IFDATA_STATS                      3       /* get|set (type int) */
#define IPSEC_OPT_INC_IFDATA_STATS_IN                   4       /* set to increment stat counters (type struct ipsec_stats_param) */
#define IPSEC_OPT_INC_IFDATA_STATS_OUT                  5       /* set to increment stat counters (type struct ipsec_stats_param) */
#define IPSEC_OPT_SET_DELEGATE_INTERFACE                6       /* set the delegate interface (char[]) */
#define IPSEC_OPT_OUTPUT_TRAFFIC_CLASS                  7       /* set the traffic class for packets leaving the interface, see sys/socket.h */
#define IPSEC_OPT_ENABLE_CHANNEL                        8       /* enable a kernel pipe nexus that allows the owner to open a channel to act as a driver,
	                                                         *  Must be set before connecting */
#define IPSEC_OPT_GET_CHANNEL_UUID                      9       /* get the uuid of the kernel pipe nexus instance */
#define IPSEC_OPT_ENABLE_FLOWSWITCH                     10      /* enable a flowswitch nexus that clients can use */
#define IPSEC_OPT_INPUT_FRAG_SIZE                       11      /* set the maximum size of input packets before fragmenting as a uint32_t */

#define IPSEC_OPT_ENABLE_NETIF                          12      /* Must be set before connecting */
#define IPSEC_OPT_SLOT_SIZE                             13      /* Must be set before connecting */
#define IPSEC_OPT_NETIF_RING_SIZE                       14      /* Must be set before connecting */
#define IPSEC_OPT_TX_FSW_RING_SIZE                      15      /* Must be set before connecting */
#define IPSEC_OPT_RX_FSW_RING_SIZE                      16      /* Must be set before connecting */
#define IPSEC_OPT_CHANNEL_BIND_PID                      17      /* Must be set before connecting */
#define IPSEC_OPT_KPIPE_TX_RING_SIZE                    18      /* Must be set before connecting */
#define IPSEC_OPT_KPIPE_RX_RING_SIZE                    19      /* Must be set before connecting */

/*
 * ipsec stats parameter structure
 */
struct ipsec_stats_param {
	u_int64_t       utsp_packets;
	u_int64_t       utsp_bytes;
	u_int64_t       utsp_errors;
};

#endif

/*
 * Copyright (c) 2009-2013 Apple Inc. All rights reserved.
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

#ifndef _NETINET_IN_ARP_H_
#define _NETINET_IN_ARP_H_
#ifdef KERNEL
#include <sys/kernel_types.h>

struct sockaddr;
struct sockaddr_dl;
struct sockaddr_in;

/*!
 *	@function inet_arp_lookup
 *	@discussion This function will check the routing table for a cached
 *		arp entry or trigger an arp query to resolve the ip address to a
 *		link-layer address.
 *
 *		Arp entries are stored in the routing table. This function will
 *		lookup the ip destination in the routing table. If the
 *		destination requires forwarding to a gateway, the route of the
 *		gateway will be looked up. The route entry is inspected to
 *		determine if the link layer destination address is known. If
 *		unknown, the arp generation function for IP attached to the
 *		interface is called to create an arp request packet.
 *	@param interface The interface the packet is being sent on.
 *	@param ip_dest The ip destination of the packet.
 *	@param ll_dest On output, the link-layer destination.
 *	@param ll_dest_len The length of the buffer for ll_dest.
 *	@param hint Any routing hint passed down from the protocol.
 *	@param packet The packet being transmitted.
 *	@result May return an error such as EHOSTDOWN or ENETUNREACH. If
 *		this function returns EJUSTRETURN, the packet has been queued
 *		and will be sent when an arp response is received. If any other
 *		value is returned, the caller is responsible for disposing of
 *		the packet.
 */
#ifdef BSD_KERNEL_PRIVATE
extern boolean_t arp_is_entry_probing(route_t p_route);
extern errno_t arp_lookup_ip(ifnet_t interface,
    const struct sockaddr_in *ip_dest, struct sockaddr_dl *ll_dest,
    size_t ll_dest_len, route_t hint, mbuf_t packet);
#define inet_arp_lookup arp_lookup_ip
#else
extern errno_t inet_arp_lookup(ifnet_t interface,
    const struct sockaddr_in *ip_dest, struct sockaddr_dl *ll_dest,
    size_t ll_dest_len, route_t hint, mbuf_t packet);
#endif /* !BSD_KERNEL_PRIVATE */

/*!
 *	@function inet_arp_handle_input
 *	@discussion This function should be called by code that handles
 *		inbound arp packets. The caller should parse the ARP packet to
 *		pull out the operation and the relevant addresses. If a response
 *		is required, the proto_media_send_arp function will be called.
 *
 *		This function will lookup the sender in the routing table and
 *		add an arp entry if necessary. Any queued packets waiting for
 *		the arp resolution will also be transmitted.
 *	@param interface The interface the packet was received on.
 *	@param arp_op The arp operation, ARPOP_REQUEST or ARPOP_REPLY
 *	@param sender_hw The sender hardware address from the arp payload.
 *	@param sender_ip The sender IP address from the arp payload.
 *	@param target_ip The target IP address from the arp payload.
 *	@result 0 on success or an errno error value on failure.
 */
#ifdef BSD_KERNEL_PRIVATE
extern errno_t arp_ip_handle_input(ifnet_t ifp, u_int16_t arpop,
    const struct sockaddr_dl *sender_hw, const struct sockaddr_in *sender_ip,
    const struct sockaddr_in *target_ip);
#define inet_arp_handle_input arp_ip_handle_input
#else
extern errno_t inet_arp_handle_input(ifnet_t ifp, u_int16_t arpop,
    const struct sockaddr_dl *sender_hw, const struct sockaddr_in *sender_ip,
    const struct sockaddr_in *target_ip);
#endif /* !BSD_KERNEL_PRIVATE */

/*!
 *	@function inet_arp_init_ifaddr
 *	@discussion This function should be called in two places, when an IP
 *		address is added and when the hardware address changes. This
 *		function will setup the ifaddr_t for use with the IP ARP
 *		functions. This function will also trigger the transmission of a
 *		gratuitous ARP packet.
 *
 *		When the SIOCSIFADDR ioctl is handled, the data parameter will
 *		be an ifaddr_t. If this is an IP address, inet_arp_init_ifaddr
 *		should be called. This is usually performed in the protocol
 *		attachment's ioctl handler.
 *
 *		When the event handler for the protocol attachment receives a
 *		KEV_DL_LINK_ADDRESS_CHANGED event, the event handler should call
 *		inet_arp_init_ifaddr for each interface ip address.
 *
 *		For an example, see bsd/net/ether_inet_pr_module.c in xnu.
 *		Search for inet_arp_init_ifaddr.
 *	@param interface The interface the packet was received on.
 *	@param ipaddr The ip interface address.
 */
#ifdef BSD_KERNEL_PRIVATE
/* inet_arp_init_ifaddr is aliased to arp_ifinit (if_ether.h) */
#define inet_arp_init_ifaddr    arp_ifinit
#else
extern void inet_arp_init_ifaddr(ifnet_t interface, ifaddr_t ipaddr);
#endif /* !BSD_KERNEL_PRIVATE */

#ifdef BSD_KERNEL_PRIVATE
extern void arp_init(void);
extern void in_arpdrain(void *);
extern void arp_llreach_set_reachable(struct ifnet *, void *, unsigned int);
#endif /* BSD_KERNEL_PRIVATE */
#endif /* KERNEL */
#endif /* _NETINET_IN_ARP_H_ */

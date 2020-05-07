/*
 * Copyright (c) 2008-2017 Apple Inc. All rights reserved.
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
 *       @header kpi_ipfilter.h
 *       This header defines an API to attach IP filters. IP filters may be
 *       attached to intercept either IPv4 or IPv6 packets. The filters can
 *       intercept all IP packets in to and out of the host regardless of
 *       interface.
 */

#ifndef __KPI_IPFILTER__
#define __KPI_IPFILTER__

#ifndef PRIVATE
#include <Availability.h>
#define __NKE_API_DEPRECATED __API_DEPRECATED("Network Kernel Extension KPI is deprecated", macos(10.4, 10.15))
#else
#define __NKE_API_DEPRECATED
#endif /* PRIVATE */

/*
 * ipf_pktopts
 *
 * Options for outgoing packets. The options need to be preserved when
 * re-injecting a packet.
 */
struct ipf_pktopts {
	u_int32_t                       ippo_flags;
	ifnet_t                         ippo_mcast_ifnet;
	int                             ippo_mcast_loop;
	u_int8_t                        ippo_mcast_ttl;
};
#define IPPOF_MCAST_OPTS            0x1
#ifdef PRIVATE
#define IPPOF_BOUND_IF              0x2
#define IPPOF_NO_IFT_CELLULAR       0x4
#define IPPOF_SELECT_SRCIF          0x8
#define IPPOF_BOUND_SRCADDR         0x10
#define IPPOF_SHIFT_IFSCOPE         16
#define IPPOF_NO_IFF_EXPENSIVE      0x20
#define IPPOF_NO_IFF_CONSTRAINED    0x40
#endif /* PRIVATE */

typedef struct ipf_pktopts *ipf_pktopts_t;

__BEGIN_DECLS

/*!
 *       @typedef ipf_input_func
 *
 *       @discussion ipf_input_func is used to filter incoming ip packets.
 *               The IP filter is called for packets from all interfaces. The
 *               filter is called between when the general IP processing is
 *               handled and when the packet is passed up to the next layer
 *               protocol such as udp or tcp. In the case of encapsulation, such
 *               as UDP in ESP (IPsec), your filter will be called once for ESP
 *               and then again for UDP. This will give your filter an
 *               opportunity to process the ESP header as well as the decrypted
 *               packet. Offset and protocol are used to determine where in the
 *               packet processing is currently occuring. If you're only
 *               interested in TCP or UDP packets, just return 0 if protocol
 *               doesn't match TCP or UDP.
 *       @param cookie The cookie specified when your filter was attached.
 *       @param data The reassembled ip packet, data will start at the ip
 *               header.
 *       @param offset An offset to the next header
 *               (udp/tcp/icmp/esp/etc...).
 *       @param protocol The protocol type (udp/tcp/icmp/etc...) of the IP packet
 *       @result Return:
 *               0 - The caller will continue with normal processing of the
 *                       packet.
 *               EJUSTRETURN - The caller will stop processing the packet,
 *                       the packet will not be freed.
 *               Anything Else - The caller will free the packet and stop
 *                       processing.
 */
typedef errno_t (*ipf_input_func)(void *cookie, mbuf_t *data, int offset,
    u_int8_t protocol);

/*!
 *       @typedef ipf_output_func
 *
 *       @discussion ipf_output_func is used to filter outbound ip packets.
 *               The IP filter is called for packets to all interfaces. The
 *               filter is called before fragmentation and IPsec processing. If
 *               you need to change the destination IP address, call
 *               ipf_inject_output and return EJUSTRETURN.
 *       @param cookie The cookie specified when your filter was attached.
 *       @param data The ip packet, will contain an IP header followed by the
 *               rest of the IP packet.
 *       @result Return:
 *               0 - The caller will continue with normal processing of the
 *                       packet.
 *               EJUSTRETURN - The caller will stop processing the packet,
 *                       the packet will not be freed.
 *               Anything Else - The caller will free the packet and stop
 *                       processing.
 */
typedef errno_t (*ipf_output_func)(void *cookie, mbuf_t *data,
    ipf_pktopts_t options);

/*!
 *       @typedef ipf_detach_func
 *
 *       @discussion ipf_detach_func is called to notify your filter that it
 *               has been detached.
 *       @param cookie The cookie specified when your filter was attached.
 */
typedef void (*ipf_detach_func)(void *cookie);

/*!
 *       @typedef ipf_filter
 *       @discussion This structure is used to define an IP filter for
 *               use with the ipf_addv4 or ipf_addv6 function.
 *       @field cookie A kext defined cookie that will be passed to all
 *               filter functions.
 *       @field name A filter name used for debugging purposes.
 *       @field ipf_input The filter function to handle inbound packets.
 *       @field ipf_output The filter function to handle outbound packets.
 *       @field ipf_detach The filter function to notify of a detach.
 */
struct ipf_filter {
	void            *cookie;
	const char      *name;
	ipf_input_func  ipf_input;
	ipf_output_func ipf_output;
	ipf_detach_func ipf_detach;
};

struct opaque_ipfilter;
typedef struct opaque_ipfilter *ipfilter_t;

/*!
 *       @function ipf_addv4
 *       @discussion Attaches an IPv4 ip filter.
 *       @param filter A structure defining the filter.
 *       @param filter_ref A reference to the filter used to detach it.
 *       @result 0 on success otherwise the errno error.
 */
#ifdef KERNEL_PRIVATE
extern errno_t ipf_addv4_internal(const struct ipf_filter *filter,
    ipfilter_t *filter_ref);

#define ipf_addv4(filter, filter_ref) \
    ipf_addv4_internal((filter), (filter_ref))
#else
extern errno_t ipf_addv4(const struct ipf_filter *filter,
    ipfilter_t *filter_ref)
__NKE_API_DEPRECATED;
#endif /* KERNEL_PRIVATE */

/*!
 *       @function ipf_addv6
 *       @discussion Attaches an IPv6 ip filter.
 *       @param filter A structure defining the filter.
 *       @param filter_ref A reference to the filter used to detach it.
 *       @result 0 on success otherwise the errno error.
 */
#ifdef KERNEL_PRIVATE
extern errno_t ipf_addv6_internal(const struct ipf_filter *filter,
    ipfilter_t *filter_ref);

#define ipf_addv6(filter, filter_ref) \
    ipf_addv6_internal((filter), (filter_ref))
#else
extern errno_t ipf_addv6(const struct ipf_filter *filter,
    ipfilter_t *filter_ref)
__NKE_API_DEPRECATED;
#endif /* KERNEL_PRIVATE */

/*!
 *       @function ipf_remove
 *       @discussion Detaches an IPv4 or IPv6 filter.
 *       @param filter_ref The reference to the filter returned from ipf_addv4 or
 *               ipf_addv6.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ipf_remove(ipfilter_t filter_ref)
__NKE_API_DEPRECATED;

/*!
 *       @function ipf_inject_input
 *       @discussion Inject an IP packet as though it had just been
 *               reassembled in ip_input. When re-injecting a packet intercepted
 *               by the filter's ipf_input function, an IP filter can pass its
 *               reference to avoid processing the packet twice. This also
 *               prevents ip filters installed before this filter from
 *               getting a chance to process the packet. If the filter modified
 *               the packet, it should not specify the filter ref to give other
 *               filters a chance to process the new packet.
 *
 *               Caller is responsible for freeing mbuf chain in the event that
 *               ipf_inject_input returns an error.
 *       @param data The complete IPv4 or IPv6 packet, receive interface must
 *               be set.
 *       @param filter_ref The reference to the filter injecting the data
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ipf_inject_input(mbuf_t data, ipfilter_t filter_ref)
__NKE_API_DEPRECATED;

/*!
 *       @function ipf_inject_output
 *       @discussion Inject an IP packet as though it had just been sent to
 *               ip_output. When re-injecting a packet intercepted by the
 *               filter's ipf_output function, an IP filter can pass its
 *               reference to avoid processing the packet twice. This also
 *               prevents ip filters installed before this filter from getting a
 *               chance to process the packet. If the filter modified the packet,
 *               it should not specify the filter ref to give other filters a
 *               chance to process the new packet.
 *       @param data The complete IPv4 or IPv6 packet.
 *       @param filter_ref The reference to the filter injecting the data
 *       @param options Output options for the packet
 *       @result 0 on success otherwise the errno error. ipf_inject_output
 *               will always free the mbuf.
 */
extern errno_t ipf_inject_output(mbuf_t data, ipfilter_t filter_ref,
    ipf_pktopts_t options)
__NKE_API_DEPRECATED;

__END_DECLS
#endif /* __KPI_IPFILTER__ */

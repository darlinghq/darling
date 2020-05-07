/*
 * Copyright (c) 2004-2019 Apple Inc. All rights reserved.
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
 *       @header kpi_interface.h
 *       This header defines an API to interact with network interfaces in
 *       the kernel. The network interface KPI may be used to implement
 *       network interfaces or to attach protocols to existing interfaces.
 */

#ifndef __KPI_INTERFACE__
#define __KPI_INTERFACE__

#ifndef XNU_KERNEL_PRIVATE
#include <TargetConditionals.h>
#endif

#include <sys/kernel_types.h>

#ifdef KERNEL_PRIVATE
struct if_interface_state;
struct ifnet_interface_advisory;
#include <sys/kpi_mbuf.h>
#endif /* KERNEL_PRIVATE */

#include <sys/_types/_sa_family_t.h>

#ifdef XNU_KERNEL_PRIVATE
#if CONFIG_EMBEDDED
#define KPI_INTERFACE_EMBEDDED 1
#else
#define KPI_INTERFACE_EMBEDDED 0
#endif
#else
#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
#define KPI_INTERFACE_EMBEDDED 1
#else
#define KPI_INTERFACE_EMBEDDED 0
#endif
#endif

struct timeval;
struct sockaddr;
struct sockaddr_dl;
struct kern_event_msg;
struct kev_msg;
struct ifnet_demux_desc;

/*!
 *       @enum Interface Families
 *       @abstract Constants defining interface families.
 *       @constant IFNET_FAMILY_ANY Match interface of any family type.
 *       @constant IFNET_FAMILY_LOOPBACK A software loopback interface.
 *       @constant IFNET_FAMILY_ETHERNET An Ethernet interface.
 *       @constant IFNET_FAMILY_SLIP A SLIP interface.
 *       @constant IFNET_FAMILY_TUN A tunnel interface.
 *       @constant IFNET_FAMILY_VLAN A virtual LAN interface.
 *       @constant IFNET_FAMILY_PPP A PPP interface.
 *       @constant IFNET_FAMILY_PVC A PVC interface.
 *       @constant IFNET_FAMILY_DISC A DISC interface.
 *       @constant IFNET_FAMILY_MDECAP A MDECAP interface.
 *       @constant IFNET_FAMILY_GIF A generic tunnel interface.
 *       @constant IFNET_FAMILY_FAITH A FAITH [IPv4/IPv6 translation] interface.
 *       @constant IFNET_FAMILY_STF A 6to4 interface.
 *       @constant IFNET_FAMILY_FIREWIRE An IEEE 1394 [Firewire] interface.
 *       @constant IFNET_FAMILY_BOND A virtual bonded interface.
 *       @constant IFNET_FAMILY_CELLULAR A cellular interface.
 *       @constant IFNET_FAMILY_6LOWPAN A 6LoWPAN interface.
 *       @constant IFNET_FAMILY_UTUN A utun interface.
 *       @constant IFNET_FAMILY_IPSEC An IPsec interface.
 */
enum {
	IFNET_FAMILY_ANY                = 0,
	IFNET_FAMILY_LOOPBACK           = 1,
	IFNET_FAMILY_ETHERNET           = 2,
	IFNET_FAMILY_SLIP               = 3,
	IFNET_FAMILY_TUN                = 4,
	IFNET_FAMILY_VLAN               = 5,
	IFNET_FAMILY_PPP                = 6,
	IFNET_FAMILY_PVC                = 7,
	IFNET_FAMILY_DISC               = 8,
	IFNET_FAMILY_MDECAP             = 9,
	IFNET_FAMILY_GIF                = 10,
	IFNET_FAMILY_FAITH              = 11,   /* deprecated */
	IFNET_FAMILY_STF                = 12,
	IFNET_FAMILY_FIREWIRE           = 13,
	IFNET_FAMILY_BOND               = 14,
	IFNET_FAMILY_CELLULAR           = 15,
	IFNET_FAMILY_6LOWPAN            = 16,
	IFNET_FAMILY_UTUN               = 17,
	IFNET_FAMILY_IPSEC              = 18
};

/*!
 *       @typedef ifnet_family_t
 *       @abstract Storage type for the interface family.
 */
typedef u_int32_t ifnet_family_t;

#ifdef KERNEL_PRIVATE
/*
 *       @enum Interface Sub-families
 *       @abstract Constants defining interface sub-families (may also
 *               be viewed as the underlying transport).  Some families
 *               (e.g. IFNET_FAMILY_ETHERNET) are often too generic.
 *               These sub-families allow us to further refine the
 *               interface family, e.g. Ethernet over Wi-Fi/USB, etc.
 */
enum {
	IFNET_SUBFAMILY_ANY             = 0,
	IFNET_SUBFAMILY_USB             = 1,
	IFNET_SUBFAMILY_BLUETOOTH       = 2,
	IFNET_SUBFAMILY_WIFI            = 3,
	IFNET_SUBFAMILY_THUNDERBOLT     = 4,
	IFNET_SUBFAMILY_RESERVED        = 5,
	IFNET_SUBFAMILY_INTCOPROC       = 6,
	IFNET_SUBFAMILY_QUICKRELAY      = 7,
	IFNET_SUBFAMILY_DEFAULT         = 8,
};

/*
 *       @typedef ifnet_sub_family_t
 *       @abstract Storage type for the interface sub-family.
 */
typedef u_int32_t ifnet_subfamily_t;
#endif /* KERNEL_PRIVATE */

#ifndef BPF_TAP_MODE_T
#define BPF_TAP_MODE_T
/*!
 *       @enum BPF tap mode
 *       @abstract Constants defining interface families.
 *       @constant BPF_MODE_DISABLED Disable bpf.
 *       @constant BPF_MODE_INPUT Enable input only.
 *       @constant BPF_MODE_OUTPUT Enable output only.
 *       @constant BPF_MODE_INPUT_OUTPUT Enable input and output.
 */

enum {
	BPF_MODE_DISABLED               = 0,
	BPF_MODE_INPUT                  = 1,
	BPF_MODE_OUTPUT                 = 2,
	BPF_MODE_INPUT_OUTPUT           = 3
};
/*!
 *       @typedef bpf_tap_mode
 *       @abstract Mode for tapping. BPF_MODE_DISABLED/BPF_MODE_INPUT_OUTPUT etc.
 */
typedef u_int32_t bpf_tap_mode;
#endif /* !BPF_TAP_MODE_T */

/*!
 *       @typedef protocol_family_t
 *       @abstract Storage type for the protocol family.
 */
typedef u_int32_t protocol_family_t;

/*!
 *       @enum Interface Abilities
 *       @abstract Constants defining interface offload support.
 *       @constant IFNET_CSUM_IP Hardware will calculate IPv4 checksums.
 *       @constant IFNET_CSUM_TCP Hardware will calculate TCP checksums.
 *       @constant IFNET_CSUM_UDP Hardware will calculate UDP checksums.
 *       @constant IFNET_CSUM_FRAGMENT Hardware will checksum IP fragments.
 *       @constant IFNET_IP_FRAGMENT Hardware will fragment IP packets.
 *       @constant IFNET_CSUM_TCPIPV6 Hardware will calculate TCP IPv6 checksums.
 *       @constant IFNET_CSUM_UDPIPV6 Hardware will calculate UDP IPv6 checksums.
 *       @constant IFNET_IPV6_FRAGMENT Hardware will fragment IPv6 packets.
 *       @constant IFNET_VLAN_TAGGING Hardware will generate VLAN headers.
 *       @constant IFNET_VLAN_MTU Hardware supports VLAN MTU.
 *       @constant IFNET_MULTIPAGES Driver is capable of handling packets
 *               coming down from the network stack that reside in virtually,
 *               but not in physically contiguous span of the external mbuf
 *               clusters.  In this case, the data area of a packet in the
 *               external mbuf cluster might cross one or more physical
 *               pages that are disjoint, depending on the interface MTU
 *               and the packet size.  Such a use of larger than system page
 *               size clusters by the network stack is done for better system
 *               efficiency.  Drivers that utilize the IOMbufNaturalMemoryCursor
 *               with the getPhysicalSegmentsWithCoalesce interfaces and
 *               enumerate the list of vectors should set this flag for
 *               possible gain in performance during bulk data transfer.
 *       @constant IFNET_TSO_IPV4 Hardware supports IPv4 TCP Segment Offloading.
 *               If the Interface driver sets this flag, TCP will send larger frames (up to 64KB) as one
 *               frame to the adapter which will perform the final packetization. The maximum TSO segment
 *               supported by the interface can be set with "ifnet_set_tso_mtu". To retreive the real MTU
 *               for the TCP connection the function "mbuf_get_tso_requested" is used by the driver. Note
 *               that if TSO is active, all the packets will be flagged for TSO, not just large packets.
 *       @constant IFNET_TSO_IPV6 Hardware supports IPv6 TCP Segment Offloading.
 *               If the Interface driver sets this flag, TCP IPv6 will send larger frames (up to 64KB) as one
 *               frame to the adapter which will perform the final packetization. The maximum TSO segment
 *               supported by the interface can be set with "ifnet_set_tso_mtu". To retreive the real MTU
 *               for the TCP IPv6 connection the function "mbuf_get_tso_requested" is used by the driver.
 *               Note that if TSO is active, all the packets will be flagged for TSO, not just large packets.
 *       @constant IFNET_TX_STATUS Driver supports returning a per packet
 *               transmission status (pass, fail or other errors) of whether
 *               the packet was successfully transmitted on the link, or the
 *               transmission was aborted, or transmission failed.
 *
 */

enum {
	IFNET_CSUM_IP           = 0x00000001,
	IFNET_CSUM_TCP          = 0x00000002,
	IFNET_CSUM_UDP          = 0x00000004,
	IFNET_CSUM_FRAGMENT     = 0x00000008,
	IFNET_IP_FRAGMENT       = 0x00000010,
	IFNET_CSUM_TCPIPV6      = 0x00000020,
	IFNET_CSUM_UDPIPV6      = 0x00000040,
	IFNET_IPV6_FRAGMENT     = 0x00000080,
#ifdef KERNEL_PRIVATE
	IFNET_CSUM_PARTIAL      = 0x00001000,
	IFNET_CSUM_SUM16        = IFNET_CSUM_PARTIAL,
	IFNET_CSUM_ZERO_INVERT  = 0x00002000,
#endif /* KERNEL_PRIVATE */
	IFNET_VLAN_TAGGING      = 0x00010000,
	IFNET_VLAN_MTU          = 0x00020000,
	IFNET_MULTIPAGES        = 0x00100000,
	IFNET_TSO_IPV4          = 0x00200000,
	IFNET_TSO_IPV6          = 0x00400000,
	IFNET_TX_STATUS         = 0x00800000,
	IFNET_HW_TIMESTAMP      = 0x01000000,
	IFNET_SW_TIMESTAMP      = 0x02000000
};
/*!
 *       @typedef ifnet_offload_t
 *       @abstract Flags indicating the offload support of the interface.
 */
typedef u_int32_t ifnet_offload_t;

#ifdef KERNEL_PRIVATE
#define IFNET_OFFLOADF_BITS \
	"\020\1CSUM_IP\2CSUM_TCP\3CSUM_UDP\4CSUM_IP_FRAGS\5IP_FRAGMENT" \
	"\6CSUM_TCPIPV6\7CSUM_UDPIPV6\10IPV6_FRAGMENT\15CSUM_PARTIAL"   \
	"\16CSUM_ZERO_INVERT\20VLAN_TAGGING\21VLAN_MTU\25MULTIPAGES"    \
	"\26TSO_IPV4\27TSO_IPV6\30TXSTATUS\31HW_TIMESTAMP\32SW_TIMESTAMP"

#define IFNET_CHECKSUMF                                                 \
	(IFNET_CSUM_IP | IFNET_CSUM_TCP | IFNET_CSUM_UDP |              \
	IFNET_CSUM_FRAGMENT | IFNET_CSUM_TCPIPV6 | IFNET_CSUM_UDPIPV6 | \
	IFNET_CSUM_PARTIAL | IFNET_CSUM_ZERO_INVERT)

#define IFNET_TSOF                                                      \
	(IFNET_TSO_IPV4	| IFNET_TSO_IPV6)
#endif /* KERNEL_PRIVATE */

/*
 * Callbacks
 *
 * These are function pointers you supply to the kernel in the interface.
 */
/*!
 *       @typedef bpf_packet_func
 *
 *       @discussion bpf_packet_func The bpf_packet_func is used to intercept
 *               inbound and outbound packets. The tap function will never free
 *               the mbuf. The tap function will only copy the mbuf in to various
 *               bpf file descriptors tapping this interface.
 *       @param interface The interface being sent or received on.
 *       @param data The packet to be transmitted or received.
 *       @result An errno value or zero upon success.
 */
/* Fast path - do not block or spend excessive amounts of time */
typedef errno_t (*bpf_packet_func)(ifnet_t interface, mbuf_t data);

/*!
 *       @typedef ifnet_output_func
 *
 *       @discussion ifnet_output_func is used to transmit packets. The stack
 *               will pass fully formed packets, including frame header, to the
 *               ifnet_output function for an interface. The driver is
 *               responsible for freeing the mbuf.
 *       @param interface The interface being sent on.
 *       @param data The packet to be sent.
 */
/* Fast path - do not block or spend excessive amounts of time */
typedef errno_t (*ifnet_output_func)(ifnet_t interface, mbuf_t data);

/*!
 *       @typedef ifnet_ioctl_func
 *       @discussion ifnet_ioctl_func is used to communicate ioctls from the
 *               stack to the driver.
 *
 *               All undefined ioctls are reserved for future use by Apple. If
 *               you need to communicate with your kext using an ioctl, please
 *               use SIOCSIFKPI and SIOCGIFKPI.
 *       @param interface The interface the ioctl is being sent to.
 *       @param cmd The ioctl command.
 *       @param data A pointer to any data related to the ioctl.
 */
typedef errno_t (*ifnet_ioctl_func)(ifnet_t interface, unsigned long cmd,
    void *data);

/*!
 *       @typedef ifnet_set_bpf_tap
 *       @discussion Deprecated. Specify NULL. Call bpf_tap_in/bpf_tap_out
 *               for all packets.
 */
typedef errno_t (*ifnet_set_bpf_tap)(ifnet_t interface, bpf_tap_mode mode,
    bpf_packet_func callback);

/*!
 *       @typedef ifnet_detached_func
 *       @discussion ifnet_detached_func is called an interface is detached
 *               from the list of interfaces. When ifnet_detach is called, it may
 *               not detach the interface immediately if protocols are attached.
 *               ifnet_detached_func is used to notify the interface that it has
 *               been detached from the networking stack. This is the last
 *               function that will be called on an interface. Until this
 *               function returns, you must not unload a kext supplying function
 *               pointers to this interface, even if ifnet_detacah has been
 *               called. Your detach function may be called during your call to
 *               ifnet_detach.
 *       @param interface The interface that has been detached.
 *               event.
 */
typedef void (*ifnet_detached_func)(ifnet_t interface);

/*!
 *       @typedef ifnet_demux_func
 *       @discussion ifnet_demux_func is called for each inbound packet to
 *               determine which protocol family the packet belongs to. This
 *               information is then used by the stack to determine which
 *               protocol to pass the packet to.  This function may return
 *               protocol families for protocols that are not attached. If the
 *               protocol family has not been attached to the interface, the
 *               packet will be discarded.
 *       @param interface The interface the packet was received on.
 *       @param packet The mbuf containing the packet.
 *       @param frame_header A pointer to the frame header.
 *       @param protocol_family Upon return, the protocol family matching the
 *               packet should be stored here.
 *       @result
 *               If the result is zero, processing will continue normally.
 *               If the result is EJUSTRETURN, processing will stop but the
 *                       packet will not be freed.
 *               If the result is anything else, the processing will stop and
 *                       the packet will be freed.
 */
typedef errno_t (*ifnet_demux_func)(ifnet_t interface, mbuf_t packet,
    char *frame_header, protocol_family_t *protocol_family);

/*!
 *       @typedef ifnet_event_func
 *       @discussion ifnet_event_func is called when an event occurs on a
 *               specific interface.
 *       @param interface The interface the event occurred on.
 */
typedef void (*ifnet_event_func)(ifnet_t interface, const struct kev_msg *msg);

/*!
 *       @typedef ifnet_framer_func
 *       @discussion ifnet_framer_func is called for each outbound packet to
 *               give the interface an opportunity to prepend interface specific
 *               headers.
 *       @param interface The interface the packet is being sent on.
 *       @param packet Pointer to the mbuf containing the packet, caller may
 *               set this to a different mbuf upon return. This can happen if the
 *               frameout function needs to prepend another mbuf to the chain to
 *               have enough space for the header.
 *       @param dest The higher layer protocol destination (i.e. IP address).
 *       @param dest_linkaddr The link layer address as determined by the
 *               protocol's pre-output function.
 *       @param frame_type The frame type as determined by the protocol's
 *               pre-output function.
 *       @discussion prepend_len The length of prepended bytes to the mbuf.
 *               (ONLY used if KPI_INTERFACE_EMBEDDED is defined to 1)
 *       @discussion postpend_len The length of the postpended bytes to the mbuf.
 *               (ONLY used if KPI_INTERFACE_EMBEDDED is defined to 1)
 *       @result
 *               If the result is zero, processing will continue normally.
 *               If the result is EJUSTRETURN, processing will stop but the
 *                       packet will not be freed.
 *               If the result is anything else, the processing will stop and
 *                       the packet will be freed.
 */
typedef errno_t (*ifnet_framer_func)(ifnet_t interface, mbuf_t *packet,
    const struct sockaddr *dest, const char *dest_linkaddr,
    const char *frame_type
#if KPI_INTERFACE_EMBEDDED
    , u_int32_t *prepend_len, u_int32_t *postpend_len
#endif /* KPI_INTERFACE_EMBEDDED */
    );
#ifdef KERNEL_PRIVATE
typedef errno_t (*ifnet_framer_extended_func)(ifnet_t interface, mbuf_t *packet,
    const struct sockaddr *dest, const char *dest_linkaddr,
    const char *frame_type, u_int32_t *prepend_len,
    u_int32_t *postpend_len);
#endif /* KERNEL_PRIVATE */

/*!
 *       @typedef ifnet_add_proto_func
 *       @discussion if_add_proto_func is called by the stack when a protocol
 *               is attached to an interface. This gives the interface an
 *               opportunity to get a list of protocol description structures
 *               for demuxing packets to this protocol (demux descriptors).
 *       @param interface The interface the protocol will be attached to.
 *       @param protocol_family The family of the protocol being attached.
 *       @param demux_array An array of demux descriptors that describe
 *               the interface specific ways of identifying packets belonging
 *               to this protocol family.
 *       @param demux_count The number of demux descriptors in the array.
 *       @result
 *               If the result is zero, processing will continue normally.
 *               If the result is anything else, the add protocol will be
 *                       aborted.
 */
typedef errno_t (*ifnet_add_proto_func)(ifnet_t interface,
    protocol_family_t protocol_family,
    const struct ifnet_demux_desc *demux_array, u_int32_t demux_count);

/*!
 *       @typedef if_del_proto_func
 *       @discussion if_del_proto_func is called by the stack when a protocol
 *               is being detached from an interface. This gives the interface an
 *               opportunity to free any storage related to this specific
 *               protocol being attached to this interface.
 *       @param interface The interface the protocol will be detached from.
 *       @param protocol_family The family of the protocol being detached.
 *       @result
 *               If the result is zero, processing will continue normally.
 *               If the result is anything else, the detach will continue
 *                       and the error will be returned to the caller.
 */
typedef errno_t (*ifnet_del_proto_func)(ifnet_t interface,
    protocol_family_t protocol_family);

/*!
 *       @typedef ifnet_check_multi
 *       @discussion ifnet_check_multi is called for each multicast address
 *               added to an interface. This gives the interface an opportunity
 *               to reject invalid multicast addresses before they are attached
 *               to the interface.
 *
 *               To prevent an address from being added to your multicast list,
 *               return EADDRNOTAVAIL. If you don't know how to parse/translate
 *               the address, return EOPNOTSUPP.
 *       @param interface The interface.
 *       @param mcast The multicast address.
 *       @result
 *               Zero upon success, EADDRNOTAVAIL on invalid multicast,
 *               EOPNOTSUPP for addresses the interface does not understand.
 */
typedef errno_t (*ifnet_check_multi)(ifnet_t interface,
    const struct sockaddr *mcast);

/*!
 *       @typedef proto_media_input
 *       @discussion proto_media_input is called for all inbound packets for
 *               a specific protocol on a specific interface. This function is
 *               registered on an interface using ifnet_attach_protocol.
 *       @param ifp The interface the packet was received on.
 *       @param protocol The protocol of the packet received.
 *       @param packet The packet being input.
 *       @param header The frame header.
 *       @result
 *               If the result is zero, the caller will assume the packet was
 *                       passed to the protocol.
 *               If the result is non-zero and not EJUSTRETURN, the caller will
 *                       free the packet.
 */
typedef errno_t (*proto_media_input)(ifnet_t ifp, protocol_family_t protocol,
    mbuf_t packet, char *header);

/*!
 *       @typedef proto_media_input_v2
 *       @discussion proto_media_input_v2 is called for all inbound packets for
 *               a specific protocol on a specific interface. This function is
 *               registered on an interface using ifnet_attach_protocolv2.
 *               proto_media_input_v2 differs from proto_media_input in that it
 *               will be called for a list of packets instead of once for each
 *               individual packet. The frame header can be retrieved using
 *               mbuf_pkthdr_header.
 *       @param ifp The interface the packet was received on.
 *       @param protocol The protocol of the packet received.
 *       @param packet The packet being input.
 *       @result
 *               If the result is zero, the caller will assume the packets were
 *                       passed to the protocol.
 *               If the result is non-zero and not EJUSTRETURN, the caller will
 *                       free the packets.
 */
typedef errno_t (*proto_media_input_v2)(ifnet_t ifp, protocol_family_t protocol,
    mbuf_t packet);

/*!
 *       @typedef proto_media_preout
 *       @discussion proto_media_preout is called just before the packet
 *               is transmitted. This gives the proto_media_preout function an
 *               opportunity to specify the media specific frame type and
 *               destination.
 *       @param ifp The interface the packet will be sent on.
 *       @param protocol The protocol of the packet being sent
 *               (PF_INET/etc...).
 *       @param packet The packet being sent.
 *       @param dest The protocol level destination address.
 *       @param route A pointer to the routing structure for the packet.
 *       @param frame_type The media specific frame type.
 *       @param link_layer_dest The media specific destination.
 *       @result
 *               If the result is zero, processing will continue normally. If the
 *               result is non-zero, processing will stop. If the result is
 *               non-zero and not EJUSTRETURN, the packet will be freed by the
 *               caller.
 */
typedef errno_t (*proto_media_preout)(ifnet_t ifp, protocol_family_t protocol,
    mbuf_t *packet, const struct sockaddr *dest, void *route, char *frame_type,
    char *link_layer_dest);

/*!
 *       @typedef proto_media_event
 *       @discussion proto_media_event is called to notify this layer of
 *               interface specific events.
 *       @param ifp The interface.
 *       @param protocol The protocol family.
 *       @param event The event.
 */
typedef void (*proto_media_event)(ifnet_t ifp, protocol_family_t protocol,
    const struct kev_msg *event);

/*!
 *       @typedef proto_media_ioctl
 *       @discussion proto_media_event allows this layer to handle ioctls.
 *               When an ioctl is handled, it is passed to the interface filters,
 *               protocol filters, protocol, and interface. If you do not support
 *               this ioctl, return EOPNOTSUPP. If you successfully handle the
 *               ioctl, return zero. If you return any error other than
 *               EOPNOTSUPP, other parts of the stack may not get an opportunity
 *               to process the ioctl. If you return EJUSTRETURN, processing will
 *               stop and a result of zero will be returned to the caller.
 *
 *               All undefined ioctls are reserved for future use by Apple. If
 *               you need to communicate with your kext using an ioctl, please
 *               use SIOCSIFKPI and SIOCGIFKPI.
 *       @param ifp The interface.
 *       @param protocol The protocol family.
 *       @param command The ioctl command.
 *       @param argument The argument to the ioctl.
 *       @result
 *               See the discussion.
 */
typedef errno_t (*proto_media_ioctl)(ifnet_t ifp, protocol_family_t protocol,
    unsigned long command, void *argument);

/*!
 *       @typedef proto_media_detached
 *       @discussion proto_media_detached notifies you that your protocol
 *               has been detached.
 *       @param ifp The interface.
 *       @param protocol The protocol family.
 *       @result
 *               See the discussion.
 */
typedef errno_t (*proto_media_detached)(ifnet_t ifp, protocol_family_t protocol);

/*!
 *       @typedef proto_media_resolve_multi
 *       @discussion proto_media_resolve_multi is called to resolve a
 *               protocol layer mulitcast address to a link layer multicast
 *               address.
 *       @param ifp The interface.
 *       @param proto_addr The protocol address.
 *       @param out_ll A sockaddr_dl to copy the link layer multicast in to.
 *       @param ll_len The length of data allocated for out_ll.
 *       @result Return zero on success or an errno error value on failure.
 */
typedef errno_t (*proto_media_resolve_multi)(ifnet_t ifp,
    const struct sockaddr *proto_addr, struct sockaddr_dl *out_ll,
    size_t ll_len);

/*!
 *       @typedef proto_media_send_arp
 *       @discussion proto_media_send_arp is called by the stack to generate
 *               an ARP packet. This field is currently only used with IP. This
 *               function should inspect the parameters and transmit an arp
 *               packet using the information passed in.
 *       @param ifp The interface the arp packet should be sent on.
 *       @param arpop The arp operation (usually ARPOP_REQUEST or
 *               ARPOP_REPLY).
 *       @param sender_hw The value to use for the sender hardware
 *               address field. If this is NULL, use the hardware address
 *               of the interface.
 *       @param sender_proto The value to use for the sender protocol
 *               address field. This will not be NULL.
 *       @param target_hw The value to use for the target hardware address.
 *               If this is NULL, the target hardware address in the ARP packet
 *               should be NULL and the link-layer destination for the back
 *               should be a broadcast. If this is not NULL, this value should be
 *               used for both the link-layer destination and the target hardware
 *               address.
 *       @param target_proto The target protocol address. This will not be
 *               NULL.
 *       @result Return zero on success or an errno error value on failure.
 */
typedef errno_t (*proto_media_send_arp)(ifnet_t ifp, u_short arpop,
    const struct sockaddr_dl *sender_hw, const struct sockaddr *sender_proto,
    const struct sockaddr_dl *target_hw, const struct sockaddr *target_proto);

/*!
 *       @struct ifnet_stat_increment_param
 *       @discussion This structure is used increment the counters on a
 *               network interface.
 *       @field packets_in The number of packets received.
 *       @field bytes_in The number of bytes received.
 *       @field errors_in The number of receive errors.
 *       @field packets_out The number of packets transmitted.
 *       @field bytes_out The number of bytes transmitted.
 *       @field errors_out The number of transmission errors.
 *       @field collisions The number of collisions seen by this interface.
 *       @field dropped The number of packets dropped.
 */
struct ifnet_stat_increment_param {
	u_int32_t                               packets_in;
	u_int32_t                               bytes_in;
	u_int32_t                               errors_in;

	u_int32_t                               packets_out;
	u_int32_t                               bytes_out;
	u_int32_t                               errors_out;

	u_int32_t                               collisions;
	u_int32_t                               dropped;
};

/*!
 *       @struct ifnet_init_params
 *       @discussion This structure is used to define various properties of
 *               the interface when calling ifnet_allocate. A copy of these
 *               values will be stored in the ifnet and cannot be modified
 *               while the interface is attached.
 *       @field uniqueid An identifier unique to this instance of the
 *               interface.
 *       @field uniqueid_len The length, in bytes, of the uniqueid.
 *       @field name The interface name (i.e. en).
 *       @field unit The interface unit number (en0's unit number is 0).
 *       @field family The interface family.
 *       @field type The interface type (see sys/if_types.h). Must be less
 *               than 256. For new types, use IFT_OTHER.
 *       @field output The output function for the interface. Every packet the
 *               stack attempts to send through this interface will go out
 *               through this function.
 *       @field demux The function used to determine the protocol family of an
 *               incoming packet.
 *       @field add_proto The function used to attach a protocol to this
 *               interface.
 *       @field del_proto The function used to remove a protocol from this
 *               interface.
 *       @field framer The function used to frame outbound packets, may be NULL.
 *       @field softc Driver specific storage. This value can be retrieved from
 *               the ifnet using the ifnet_softc function.
 *       @field ioctl The function used to handle ioctls.
 *       @field set_bpf_tap The function used to set the bpf_tap function.
 *       @field detach The function called to let the driver know the interface
 *               has been detached.
 *       @field event The function to notify the interface of various interface
 *               specific kernel events.
 *       @field broadcast_addr The link-layer broadcast address for this
 *               interface.
 *       @field broadcast_len The length of the link-layer broadcast address.
 */
struct ifnet_init_params {
	/* used to match recycled interface */
	const void              *uniqueid;              /* optional */
	u_int32_t               uniqueid_len;           /* optional */

	/* used to fill out initial values for interface */
	const char              *name;                  /* required */
	u_int32_t               unit;                   /* required */
	ifnet_family_t          family;                 /* required */
	u_int32_t               type;                   /* required */
	ifnet_output_func       output;                 /* required */
	ifnet_demux_func        demux;                  /* required  */
	ifnet_add_proto_func    add_proto;              /* required  */
	ifnet_del_proto_func    del_proto;              /* required  */
	ifnet_check_multi       check_multi;            /* required for non point-to-point interfaces */
	ifnet_framer_func       framer;                 /* optional */
	void                    *softc;                 /* optional */
	ifnet_ioctl_func        ioctl;                  /* optional */
	ifnet_set_bpf_tap       set_bpf_tap;            /* deprecated */
	ifnet_detached_func     detach;                 /* optional */
	ifnet_event_func        event;                  /* optional */
	const void              *broadcast_addr;        /* required for non point-to-point interfaces */
	u_int32_t               broadcast_len;          /* required for non point-to-point interfaces */
};

#ifdef KERNEL_PRIVATE
/* Valid values for version */
#define IFNET_INIT_VERSION_2            2
#define IFNET_INIT_CURRENT_VERSION      IFNET_INIT_VERSION_2

/* Valid values for flags */
#define IFNET_INIT_LEGACY       0x1     /* legacy network interface model */
#define IFNET_INIT_INPUT_POLL   0x2     /* opportunistic input polling model */
#define IFNET_INIT_NX_NOAUTO    0x4     /* do not auto config nexus */
#define IFNET_INIT_ALLOC_KPI    0x8     /* allocated via the ifnet_alloc() KPI */

/*
 *       @typedef ifnet_pre_enqueue_func
 *       @discussion ifnet_pre_enqueue_func is called for each outgoing packet
 *               for the interface. The driver may perform last-minute changes
 *               on the (fully formed) packet, but it is responsible for calling
 *               ifnet_enqueue() to enqueue the packet upon completion.
 *       @param interface The interface being sent on.
 *       @param data The packet to be sent.
 */
typedef errno_t (*ifnet_pre_enqueue_func)(ifnet_t interface, mbuf_t data);

/*
 *       @typedef ifnet_start_func
 *       @discussion ifnet_start_func is used to indicate to the driver that
 *               one or more packets may be dequeued by calling ifnet_dequeue()
 *               or ifnet_dequeue_multi() or ifnet_dequeue_multi_bytes().
 *               This routine gets invoked when ifnet_start() is called;
 *               the ifnet_start_func callback will be executed within the
 *               context of a dedicated kernel thread, hence it is
 *               guaranteed to be single threaded. The driver must employ
 *               additional serializations if this callback routine is
 *               to be called directly from another context, in order to
 *               prevent race condition related issues (e.g. out-of-order
 *               packets.) The dequeued packets will be fully formed
 *               packets (including frame headers). The packets must be
 *               freed by the driver.
 *       @param interface The interface being sent on.
 */
typedef void (*ifnet_start_func)(ifnet_t interface);

/*
 *       @typedef ifnet_input_poll_func
 *       @discussion ifnet_input_poll_func is called by the network stack to
 *               retrieve one or more packets from the driver which implements
 *               the new driver input model.
 *       @param interface The interface to retrieve the packets from.
 *       @param flags For future use.
 *       @param max_count The maximum number of packets to be dequeued.
 *       @param first_packet Pointer to the first packet being dequeued.
 *       @param last_packet Pointer to the last packet being dequeued.
 *       @param cnt Pointer to a storage for the number of packets dequeued.
 *       @param len Pointer to a storage for the total length (in bytes)
 *               of the dequeued packets.
 */
typedef void (*ifnet_input_poll_func)(ifnet_t interface, u_int32_t flags,
    u_int32_t max_count, mbuf_t *first_packet, mbuf_t *last_packet,
    u_int32_t *cnt, u_int32_t *len);

/*
 *       @enum Interface control commands
 *       @abstract Constants defining control commands.
 *       @discussion
 *       @constant IFNET_CTL_SET_INPUT_MODEL Set input model.
 *       @constant IFNET_CTL_GET_INPUT_MODEL Get input model.
 *       @constant IFNET_CTL_SET_LOG Set logging level.
 *       @constant IFNET_CTL_GET_LOG Get logging level.
 */
enum {
	IFNET_CTL_SET_INPUT_MODEL       = 1,    /* input ctl */
	IFNET_CTL_GET_INPUT_MODEL       = 2,    /* input ctl */
	IFNET_CTL_SET_LOG               = 3,    /* output ctl */
	IFNET_CTL_GET_LOG               = 4,    /* output ctl */
	IFNET_CTL_NOTIFY_ADDRESS        = 5     /* output ctl */
};

/*
 *       @typedef ifnet_ctl_cmd_t
 *       @abstract Storage type for the interface control command.
 */
typedef u_int32_t ifnet_ctl_cmd_t;

/*
 *       @enum Interface model sub-commands
 *       @abstract Constants defining model sub-commands.
 *       @discussion
 *       @constant IFNET_MODEL_INPUT_POLL_OFF Polling is inactive.  When set,
 *               the network stack will no longer invoke the input_poll callback
 *               until the next time polling is turned on; the driver should
 *               proceed to pushing the packets up to the network stack as in
 *               the legacy input model, and if applicable, the driver should
 *               also enable receive interrupt for the hardware.  During get,
 *               this indicates that the driver is currently operating in
 *               the legacy/push input model.
 *       @constant IFNET_MODEL_INPUT_POLL_ON Polling is active.  When set, the
 *               network stack will begin to invoke the input_poll callback to
 *               retrieve packets from the driver until the next time polling
 *               is turned off; the driver should no longer be pushing packets
 *               up to the network stack, and if applicable, the driver should
 *               also disable receive interrupt for the hardware.  During get,
 *               this indicates that the driver is currently operating in
 *               the new/pull input model.
 */
enum {
	IFNET_MODEL_INPUT_POLL_OFF      = 0,
	IFNET_MODEL_INPUT_POLL_ON       = 1,
};

/*
 *       @typedef ifnet_model_t
 *       @abstract Storage type for the interface model sub-command.
 */
typedef u_int32_t ifnet_model_t;

/*
 *       @struct ifnet_model_params
 *       @discussion This structure is used as parameter to the ifnet model
 *               sub-commands.
 *       @field model The interface model.
 */
struct ifnet_model_params {
	ifnet_model_t           model;
	u_int32_t               reserved[3];
};

/*
 *       @enum Interface logging sub-commands.
 *       @abstract Constants defining logging levels/priorities.  A level
 *               includes all other levels below it.  It is expected that
 *               verbosity increases along with the level.
 *       @discussion
 *       @constant IFNET_LOG_DEFAULT Revert to default logging level.
 *       @constant IFNET_LOG_ALERT Log actions that must be taken immediately.
 *       @constant IFNET_LOG_CRITICAL Log critical conditions.
 *       @constant IFNET_LOG_ERROR Log error conditions.
 *       @constant IFNET_LOG_WARNING Log warning conditions.
 *       @constant IFNET_LOG_NOTICE Log normal but significant conditions.
 *       @constant IFNET_LOG_INFORMATIONAL Log informational messages.
 *       @constant IFNET_LOG_DEBUG Log debug-level messages.
 */
enum {
	IFNET_LOG_DEFAULT       = 0,
	IFNET_LOG_ALERT         = 1,
	IFNET_LOG_CRITICAL      = 2,
	IFNET_LOG_ERROR         = 3,
	IFNET_LOG_WARNING       = 4,
	IFNET_LOG_NOTICE        = 5,
	IFNET_LOG_INFORMATIONAL = 6,
	IFNET_LOG_DEBUG         = 7
};

#ifdef BSD_KERNEL_PRIVATE
#define IFNET_LOG_MIN   IFNET_LOG_DEFAULT
#define IFNET_LOG_MAX   IFNET_LOG_DEBUG
#endif /* BSD_KERNEL_PRIVATE */

/*
 *       @typedef ifnet_log_level_t
 *       @abstract Storage type for log level/priority.
 */
typedef int32_t ifnet_log_level_t;

/*
 *       @enum Interface logging facilities
 *       @abstract Constants defining the logging facilities which
 *               are to be configured with the specified logging level.
 *       @discussion
 *       @constant IFNET_LOGF_DLIL The DLIL layer.
 *       @constant IFNET_LOGF_FAMILY The networking family layer.
 *       @constant IFNET_LOGF_DRIVER The device driver layer.
 *       @constant IFNET_LOGF_FIRMWARE The firmware layer.
 */
enum {
	IFNET_LOGF_DLIL         = 0x00000001,
	IFNET_LOGF_FAMILY       = 0x00010000,
	IFNET_LOGF_DRIVER       = 0x01000000,
	IFNET_LOGF_FIRMWARE     = 0x10000000
};

#ifdef BSD_KERNEL_PRIVATE
#define IFNET_LOGF_MASK                                                 \
	(IFNET_LOGF_DLIL | IFNET_LOGF_FAMILY | IFNET_LOGF_DRIVER |      \
	IFNET_LOGF_FIRMWARE)

#define IFNET_LOGF_BITS \
	"\020\1DLIL\21FAMILY\31DRIVER\35FIRMWARE"

#endif /* BSD_KERNEL_PRIVATE */

/*
 *       @typedef ifnet_log_flags_t
 *       @abstract Storage type for log flags/facilities.
 */
typedef u_int32_t ifnet_log_flags_t;

/*
 *       @enum Interface logging category
 *       @abstract Constants defininig categories for issues experienced.
 *       @discussion
 *       @constant IFNET_LOGCAT_CONNECTIVITY Connectivity related issues.
 *       @constant IFNET_LOGCAT_QUALITY Quality/fidelity related issues.
 *       @constant IFNET_LOGCAT_PERFORMANCE Performance related issues.
 */
enum {
	IFNET_LOGCAT_CONNECTIVITY       = 1,
	IFNET_LOGCAT_QUALITY            = 2,
	IFNET_LOGCAT_PERFORMANCE        = 3
};

/*
 *       @typedef ifnet_log_category_t
 *       @abstract Storage type for log category.
 */
typedef int32_t ifnet_log_category_t;

/*
 *       @typedef ifnet_log_subcategory_t
 *       @abstract Storage type for log subcategory.  This is largely opaque
 *               and it can be used for IOReturn values, etc.
 */
typedef int32_t ifnet_log_subcategory_t;

/*
 *       @struct ifnet_log_params
 *       @discussion This structure is used as parameter to the ifnet
 *               logging sub-commands.
 *       @field level The logging level/priority.
 *       @field flags The logging flags/facilities.
 *       @field category The category of issue.
 *       @field subcategory The subcategory of issue.
 */
struct ifnet_log_params {
	ifnet_log_level_t       level;
	ifnet_log_flags_t       flags;
	ifnet_log_category_t    category;
	ifnet_log_subcategory_t subcategory;
};

/*
 *       @struct ifnet_notify_address_params
 *       @discussion This structure is used as parameter to the ifnet
 *               address notification sub-command.  This is used to indicate
 *               to the family/driver that one or more addresses of the given
 *               address family has been added to, or removed from the list
 *               of addresses on the interface.  The driver may query for the
 *               current address list by calling ifnet_get_address_list_family().
 *       @field address_family The address family of the interface address(es).
 */
struct ifnet_notify_address_params {
	sa_family_t             address_family;
	u_int32_t               reserved[3];
};

/*
 *       @typedef ifnet_ctl_func
 *       @discussion ifnet_ctl_func is called by the network stack to inform
 *               about changes in parameters, or retrieve the parameters
 *               related to the output or input processing or capabilities.
 *       @param interface The interface.
 *       @param cmd The ifnet_ctl_cmd_t interface control command.
 *       @param arglen The length of the command argument.
 *       @param arg The command argument.
 *       @result 0 upon success, otherwise errno error.
 */
typedef errno_t (*ifnet_ctl_func)(ifnet_t interface, ifnet_ctl_cmd_t cmd,
    u_int32_t arglen, void *arg);

/*
 *       @struct ifnet_init_eparams
 *       @discussion This structure is used to define various properties of
 *               the interface when calling ifnet_allocate_extended. A copy of
 *               these values will be stored in the ifnet and cannot be modified
 *               while the interface is attached.
 *       @field ver The current structure version (IFNET_INIT_CURRENT_VERSION)
 *       @field len The length of this structure.
 *       @field flags See above values for flags.
 *       @field uniqueid An identifier unique to this instance of the
 *               interface.
 *       @field uniqueid_len The length, in bytes, of the uniqueid.
 *       @field name The interface name (i.e. en).
 *       @field unit The interface unit number (en0's unit number is 0).
 *       @field family The interface family.
 *       @field type The interface type (see sys/if_types.h). Must be less
 *               than 256. For new types, use IFT_OTHER.
 *       @field sndq_maxlen The maximum size of the output queue; valid only
 *               if IFNET_INIT_LEGACY is not set.
 *       @field output The output function for the interface. Every packet the
 *               stack attempts to send through this interface will go out
 *               through this function.
 *       @field pre_enqueue The pre_enqueue function for the interface, valid
 *               only if IFNET_INIT_LEGACY is not set, and optional if it is set.
 *       @field start The start function for the interface, valid and required
 *               only if IFNET_INIT_LEGACY is not set.
 *       @field output_ctl The output control function for the interface, valid
 *               only if IFNET_INIT_LEGACY is not set.
 *       @field output_sched_model The IFNET_SCHED_MODEL value for the output
 *               queue, as defined in net/if.h
 *       @field output_target_qdelay The target queue delay is used for
 *               dynamically sizing the output queue, valid only if
 *               IFNET_INIT_LEGACY is not set.
 *       @field output_bw The effective output bandwidth (in bits per second.)
 *       @field output_bw_max The maximum theoretical output bandwidth
 *               (in bits per second.)
 *       @field output_lt The effective output latency (in nanosecond.)
 *       @field output_lt_max The maximum theoretical output latency
 *               (in nanosecond.)
 *       @field start_delay_qlen The maximum length of output queue for
 *               delaying start callback to the driver. This is an
 *               optimization for coalescing output packets.
 *       @field start_delay_timeout The timeout in microseconds to delay
 *               start callback. If start_delay_qlen number of packets are
 *               not in the output queue when the timer fires, the start
 *               callback will be invoked. Maximum allowed value is
 *               20ms (in microseconds).
 *       @field input_poll The poll function for the interface, valid only if
 *               IFNET_INIT_LEGACY is not set and only if IFNET_INIT_INPUT_POLL
 *               is set.
 *       @field input_ctl The input control function for the interface, valid
 *               only if IFNET_INIT_LEGACY is not set and only if opportunistic
 *               input polling is enabled via IFNET_INIT_INPUT_POLL flag.
 *       @field rcvq_maxlen The size of the driver's receive ring or the total
 *               count of descriptors used in the receive path; valid only if
 *               IFNET_INIT_INPUT_POLL is set.
 *       @field input_bw The effective input bandwidth (in bits per second.)
 *       @field input_bw_max The maximum theoretical input bandwidth
 *               (in bits per second.)
 *       @field input_lt The effective input latency (in nanosecond.)
 *       @field input_lt_max The maximum theoretical input latency
 *               (in nanosecond.)
 *       @field demux The function used to determine the protocol family of an
 *               incoming packet.
 *       @field add_proto The function used to attach a protocol to this
 *               interface.
 *       @field del_proto The function used to remove a protocol from this
 *               interface.
 *       @field framer The function used to frame outbound packets, may be NULL.
 *       @field framer_extended The function used to frame outbound packets,
 *               in the newer form; may be NULL.  If specified, it will override
 *               the value set via framer.
 *       @field softc Driver specific storage. This value can be retrieved from
 *               the ifnet using the ifnet_softc function.
 *       @field ioctl The function used to handle ioctls.
 *       @field set_bpf_tap The function used to set the bpf_tap function.
 *       @field detach The function called to let the driver know the interface
 *               has been detached.
 *       @field event The function to notify the interface of various interface
 *               specific kernel events.
 *       @field broadcast_addr The link-layer broadcast address for this
 *               interface.
 *       @field broadcast_len The length of the link-layer broadcast address.
 *       @field tx_headroom The amount of headroom space to be reserved in the
 *               packet being transmitted on the interface, specified in bytes.
 *               Must be a multiple of 8 bytes.
 *       @field tx_trailer The amount of trailer space to be reserved in the
 *               packet being transmitted on the interface, specified in bytes.
 *       @field rx_mit_ival mitigation interval for the rx mitigation logic,
 *               specified in microseconds.
 */
struct ifnet_init_eparams {
	u_int32_t               ver;                    /* required */
	u_int32_t               len;                    /* required */
	u_int32_t               flags;                  /* optional */

	/* used to match recycled interface */
	const void              *uniqueid;              /* optional */
	u_int32_t               uniqueid_len;           /* optional */

	/* used to fill out initial values for interface */
	const char              *name;                  /* required */
	u_int32_t               unit;                   /* required */
	ifnet_family_t          family;                 /* required */
	u_int32_t               type;                   /* required */
	u_int32_t               sndq_maxlen;            /* optional, only for new model */
	ifnet_output_func       output;                 /* required only for legacy model */
	ifnet_pre_enqueue_func  pre_enqueue;            /* optional, only for new model */
	ifnet_start_func        start;                  /* required only for new model */
	ifnet_ctl_func          output_ctl;             /* optional, only for new model */
	u_int32_t               output_sched_model;     /* optional, only for new model */
	u_int32_t               output_target_qdelay;   /* optional, only for new model, value in ms */
	u_int64_t               output_bw;              /* optional */
	u_int64_t               output_bw_max;          /* optional */
	u_int64_t               output_lt;              /* optional */
	u_int64_t               output_lt_max;          /* optional */
	u_int16_t               start_delay_qlen;       /* optional */
	u_int16_t               start_delay_timeout;    /* optional */
	u_int32_t               _reserved[3];           /* for future use */
	ifnet_input_poll_func   input_poll;             /* optional, ignored for legacy model */
	ifnet_ctl_func          input_ctl;              /* required for opportunistic polling */
	u_int32_t               rcvq_maxlen;            /* optional, only for opportunistic polling */
	u_int32_t               __reserved;             /* for future use */
	u_int64_t               input_bw;               /* optional */
	u_int64_t               input_bw_max;           /* optional */
	u_int64_t               input_lt;               /* optional */
	u_int64_t               input_lt_max;           /* optional */
	u_int64_t               ___reserved[2];         /* for future use */
	ifnet_demux_func        demux;                  /* required  */
	ifnet_add_proto_func    add_proto;              /* required  */
	ifnet_del_proto_func    del_proto;              /* required  */
	ifnet_check_multi       check_multi;            /* required for non point-to-point interfaces */
	ifnet_framer_func       framer;                 /* optional */
	void                    *softc;                 /* optional */
	ifnet_ioctl_func        ioctl;                  /* optional */
	ifnet_set_bpf_tap       set_bpf_tap;            /* deprecated */
	ifnet_detached_func     detach;                 /* optional */
	ifnet_event_func        event;                  /* optional */
	const void              *broadcast_addr;        /* required for non point-to-point interfaces */
	u_int32_t               broadcast_len;          /* required for non point-to-point interfaces */
	ifnet_framer_extended_func framer_extended;     /* optional */
	ifnet_subfamily_t       subfamily;              /* optional */
	u_int16_t               tx_headroom;            /* optional */
	u_int16_t               tx_trailer;             /* optional */
	u_int32_t               rx_mit_ival;            /* optional */
#if !defined(__LP64__)
	u_int64_t               ____reserved[2];        /* for future use */
#else
	u_int32_t               ____reserved;           /* for future use */
	u_int64_t               _____reserved[1];       /* for future use */
#endif /* __LP64__ */
};
#endif /* KERNEL_PRIVATE */

/*!
 *       @struct ifnet_stats_param
 *       @discussion This structure is used get and set the interface
 *               statistics.
 *       @field packets_in The number of packets received.
 *       @field bytes_in The number of bytes received.
 *       @field errors_in The number of receive errors.
 *       @field packets_out The number of packets transmitted.
 *       @field bytes_out The number of bytes transmitted.
 *       @field errors_out The number of transmission errors.
 *       @field collisions The number of collisions seen by this interface.
 *       @field dropped The number of packets dropped.
 */
struct ifnet_stats_param {
	u_int64_t       packets_in;
	u_int64_t       bytes_in;
	u_int64_t       multicasts_in;
	u_int64_t       errors_in;

	u_int64_t       packets_out;
	u_int64_t       bytes_out;
	u_int64_t       multicasts_out;
	u_int64_t       errors_out;

	u_int64_t       collisions;
	u_int64_t       dropped;
	u_int64_t       no_protocol;
};

/*!
 *       @struct ifnet_demux_desc
 *       @discussion This structure is to identify packets that belong to a
 *               specific protocol. The types supported are interface specific.
 *               Ethernet supports ETHER_DESC_ETYPE2, ETHER_DESC_SAP, and
 *               ETHER_DESC_SNAP. The type defines the offset in the packet where
 *               the data will be matched as well as context. For example, if
 *               ETHER_DESC_SNAP is specified, the only valid datalen is 5 and
 *               only in the 5 bytes will only be matched when the packet header
 *               indicates that the packet is a SNAP packet.
 *       @field type The type of identifier data (i.e. ETHER_DESC_ETYPE2)
 *       @field data A pointer to an entry of type (i.e. pointer to 0x0800).
 *       @field datalen The number of bytes of data used to describe the
 *               packet.
 */
struct ifnet_demux_desc {
	u_int32_t       type;
	void            *data;
	u_int32_t       datalen;
};

/*!
 *       @struct ifnet_attach_proto_param
 *       @discussion This structure is used to attach a protocol to an
 *               interface. This structure provides the various functions for
 *               handling operations related to the protocol on the interface as
 *               well as information for how to demux packets for this protocol.
 *       @field demux_array An array of ifnet_demux_desc structures
 *               describing the protocol.
 *       @field demux_count The number of entries in the demux_array array.
 *       @field input The function to be called for inbound packets.
 *       @field pre_output The function to be called for outbound packets.
 *       @field event The function to be called for interface events.
 *       @field ioctl The function to be called for ioctls.
 *       @field detached The function to be called for handling the detach.
 */
#ifdef KERNEL_PRIVATE
#define demux_list demux_array
#endif /* KERNEL_PRIVATE */

struct ifnet_attach_proto_param {
	struct ifnet_demux_desc         *demux_array;   /* interface may/may not require */
	u_int32_t                       demux_count;    /* interface may/may not require */

	proto_media_input               input;          /* required */
	proto_media_preout              pre_output;     /* required */
	proto_media_event               event;          /* optional */
	proto_media_ioctl               ioctl;          /* optional */
	proto_media_detached            detached;       /* optional */
	proto_media_resolve_multi       resolve;        /* optional */
	proto_media_send_arp            send_arp;       /* optional */
};

struct ifnet_attach_proto_param_v2 {
	struct ifnet_demux_desc         *demux_array;   /* interface may/may not require */
	u_int32_t                       demux_count;    /* interface may/may not require */

	proto_media_input_v2            input;          /* required */
	proto_media_preout              pre_output;     /* required */
	proto_media_event               event;          /* optional */
	proto_media_ioctl               ioctl;          /* optional */
	proto_media_detached            detached;       /* optional */
	proto_media_resolve_multi       resolve;        /* optional */
	proto_media_send_arp            send_arp;       /* optional */
};

__BEGIN_DECLS

/*
 * Ifnet creation and reference counting
 */

/*!
 *       @function ifnet_allocate
 *       @discussion Allocate an ifnet_t with an initial refcount of 1. Many
 *               parts of the stack do not properly refcount the ifnet_t. In
 *               order to avoid freeing the ifnet_t while some parts of the stack
 *               may contain a reference to it, the ifnet_ts are only recycled,
 *               never freed. A unique id is used to try and recycle the same
 *               ifnet_t when allocating an interface. For example, for an
 *               ethernet interface, the hardware address of the ethernet card is
 *               usually used for the uniqueid. If a PC Card is removed and
 *               inserted again, if the ethernet address of the PC card is used,
 *               the same ifnet_t will be used for the card the second time it is
 *               inserted. In the future, when the ifnet_t is correctly
 *               refcounted by all of the stack, the interfaces may be freed and
 *               the unique ids ignored.
 *       @param init The initial values for the interface. These values can
 *               not be changed after the interface has been allocated.
 *       @param interface The interface allocated upon success.
 *       @result May return ENOMEM if there is insufficient memory or EEXIST
 *               if an interface with the same uniqueid and family has already
 *               been allocated and is in use.
 */
#ifdef KERNEL_PRIVATE
extern errno_t ifnet_allocate_internal(const struct ifnet_init_params *init,
    ifnet_t *interface);

#define ifnet_allocate(init, interface) \
	ifnet_allocate_internal((init), (interface))
#else
extern errno_t ifnet_allocate(const struct ifnet_init_params *init,
    ifnet_t *interface);
#endif /* KERNEL_PRIVATE */

#ifdef KERNEL_PRIVATE
/*
 *       @function ifnet_allocate_extended
 *       @discussion An extended/newer variant of ifnet_allocate, with additional
 *               support for the new output and input driver models.
 *       @param init The initial values for the interface. These values can
 *               not be changed after the interface has been allocated.
 *       @param interface The interface allocated upon success.
 *       @result May return ENOMEM if there is insufficient memory or EBUSY
 *               if an interface with the same uniqueid/(name + unit) and family has already
 *               been allocated and is in use.
 */
extern errno_t ifnet_allocate_extended(const struct ifnet_init_eparams *init,
    ifnet_t *interface);

/*
 *       @function ifnet_purge
 *       @discussion Purge the output queue of an interface which implements
 *               the new driver output model.
 *       @param interface The interface to purge.
 */
extern void ifnet_purge(ifnet_t interface);

/*
 *       @function ifnet_enqueue
 *       @discussion Enqueue a packet to the output queue of an interface
 *               which implements the new driver output model.
 *       @param interface The interface to enqueue the packet to.
 *       @param packet The packet being enqueued; only one packet is allowed
 *               to be enqueued at a time.
 *       @result May return EINVAL if the parameters are invalid; ENXIO if
 *               the interface doesn't implement the new driver output model;
 *               EQFULL if the output queue is flow-controlled; or EQSUSPENDED
 *               if the output queue is suspended.  This routine either frees
 *               or consumes the packet; the caller must not modify or free
 *               it after calling this routine.  Any attempt to enqueue more
 *               than one packet will cause the entire packet chain to be freed.
 */
extern errno_t ifnet_enqueue(ifnet_t interface, mbuf_t packet);

/*
 *       @function ifnet_dequeue
 *       @discussion Dequeue a packet from the output queue of an interface
 *               which implements the new driver output model, and that the
 *               output scheduling model is set to IFNET_SCHED_MODEL_NORMAL.
 *       @param interface The interface to dequeue the packet from.
 *       @param packet Pointer to the packet being dequeued.
 *       @result May return EINVAL if the parameters are invalid, ENXIO if
 *               the interface doesn't implement the new driver output model
 *               or the output scheduling model isn't IFNET_SCHED_MODEL_NORMAL,
 *               or EAGAIN if there is currently no packet available to
 *               be dequeued.
 */
extern errno_t ifnet_dequeue(ifnet_t interface, mbuf_t *packet);

/*
 *       @function ifnet_dequeue_service_class
 *       @discussion Dequeue a packet of a particular service class from the
 *               appropriate output queue of an interface which implements the
 *               new driver output model, and that the output scheduling model
 *               is set to IFNET_SCHED_MODEL_DRIVER_MANAGED.
 *       @param interface The interface to dequeue the packet from.
 *       @param sc The service class.
 *       @param packet Pointer to the packet being dequeued.
 *       @result May return EINVAL if the parameters are invalid, ENXIO if
 *               the interface doesn't implement the new driver output model
 *               or if the output scheduling model isn't configured to
 *               IFNET_SCHED_MODEL_DRIVER_MANAGED, or EAGAIN if there
 *               is currently no packet available to be dequeued.
 */
extern errno_t ifnet_dequeue_service_class(ifnet_t interface,
    mbuf_svc_class_t sc, mbuf_t *packet);

/*
 *       @function ifnet_dequeue_multi
 *       @discussion Dequeue one or more packets from the output queue of an
 *               interface which implements the new driver output model, and that
 *               the output scheduling model is set to IFNET_SCHED_MODEL_NORMAL.
 *               The returned packet chain is traversable with mbuf_nextpkt().
 *       @param interface The interface to dequeue the packets from.
 *       @param max The maximum number of packets in the packet chain that
 *               may be returned to the caller; this needs to be a non-zero
 *               value for any packet to be returned.
 *       @param first_packet Pointer to the first packet being dequeued.
 *       @param last_packet Pointer to the last packet being dequeued.  Caller
 *               may supply NULL if not interested in value.
 *       @param cnt Pointer to a storage for the number of packets dequeued.
 *               Caller may supply NULL if not interested in value.
 *       @param len Pointer to a storage for the total length (in bytes)
 *               of the dequeued packets.  Caller may supply NULL if not
 *               interested in value.
 *       @result May return EINVAL if the parameters are invalid, ENXIO if
 *               the interface doesn't implement the new driver output model
 *               or the output scheduling model isn't IFNET_SCHED_MODEL_NORMAL,
 *               or EAGAIN if there is currently no packet available to
 *               be dequeued.
 */
extern errno_t ifnet_dequeue_multi(ifnet_t interface, u_int32_t max,
    mbuf_t *first_packet, mbuf_t *last_packet, u_int32_t *cnt, u_int32_t *len);

/*
 *       @function ifnet_dequeue_multi_bytes
 *       @discussion Dequeue one or more packets from the output queue of
 *               an interface which implements the new driver output model,
 *               where the scheduling model is set to
 *               IFNET_SCHED_MODEL_NORMAL. The limit is specified in terms
 *               of maximum number of bytes to return. The number of bytes
 *               returned can be slightly higher than the limit so that
 *               packet boundaries can be preserved.
 *       @param interface The interface to dequeue the packets from
 *       @param max_bytes The maximum number of bytes in the packet chain
 *               that may be returned to the caller; this needs to be a
 *               non-zero value for any packet to be returned.
 *       @param first_packet Pointer to the first packet being dequeued
 *       @param last_packet Pointer to the last packet being dequeued
 *       @param cnt Pointer to a storage for the number of bytes dequeued.
 *               Caller may supply NULL if not interested in this value
 *       @param len Pointer to a storage for the total length (in bytes)
 *               of the dequeued packets. Caller may supply NULL if not
 *               interested in this value.
 *       @result May return EINVAL if the parameters are invalid, ENXIO if
 *               the interface doesn't implement the new driver output
 *               model or the output scheduling model isn't
 *               IFNET_SCHED_MODEL_NORMAL, or EAGAIN if there is currently
 *               no packet available to be dequeued
 */
extern errno_t ifnet_dequeue_multi_bytes(ifnet_t interface,
    u_int32_t max_bytes, mbuf_t *first_packet, mbuf_t *last_packet,
    u_int32_t *cnt, u_int32_t *len);

/*
 *       @function ifnet_dequeue_service_class_multi
 *       @discussion Dequeue one or more packets of a particular service class
 *               from the appropriate output queue of an interface which
 *               implements the new driver output model, and that the output
 *               scheduling model is set to IFNET_SCHED_MODEL_DRIVER_MANAGED.
 *               The returned packet chain is traversable with mbuf_nextpkt().
 *       @param interface The interface to dequeue the packets from.
 *       @param sc The service class.
 *       @param max The maximum number of packets in the packet chain that
 *               may be returned to the caller; this needs to be a non-zero
 *               value for any packet to be returned.
 *       @param first_packet Pointer to the first packet being dequeued.
 *       @param last_packet Pointer to the last packet being dequeued.  Caller
 *               may supply NULL if not interested in value.
 *       @param cnt Pointer to a storage for the number of packets dequeued.
 *               Caller may supply NULL if not interested in value.
 *       @param len Pointer to a storage for the total length (in bytes)
 *               of the dequeued packets.  Caller may supply NULL if not
 *               interested in value.
 *       @result May return EINVAL if the parameters are invalid, ENXIO if
 *               the interface doesn't implement the new driver output model
 *               or if the output scheduling model isn't configured to
 *               IFNET_SCHED_MODEL_DRIVER_MANAGED, or EAGAIN if there
 *               is currently no packet available to be dequeued.
 */
extern errno_t ifnet_dequeue_service_class_multi(ifnet_t interface,
    mbuf_svc_class_t sc, u_int32_t max, mbuf_t *first_packet,
    mbuf_t *last_packet, u_int32_t *cnt, u_int32_t *len);

/*
 *       @function ifnet_set_output_sched_model
 *       @discussion Set the output scheduling model of an interface which
 *               implements the new driver output model.
 *       @param interface The interface to set scheduling model on.
 *       @param model The IFNET_SCHED_MODEL value as defined in net/if.h
 *       @result May return EINVAL if the parameters are invalid or ENXIO if
 *               the interface doesn't implement the new driver output model.
 */
extern errno_t ifnet_set_output_sched_model(ifnet_t interface,
    u_int32_t model);

/*
 *       @function ifnet_set_sndq_maxlen
 *       @discussion Set the maximum length of the output queue of an
 *               interface which implements the new driver output model.
 *               This call may be issued post ifnet_allocate_extended in
 *               order to modify the maximum output queue length previously
 *               set at registration time.
 *       @param interface The interface to set the max queue length on.
 *       @param maxqlen The maximum number of packets in the output queue.
 *       @result May return EINVAL if the parameters are invalid or ENXIO if
 *               the interface doesn't implement the new driver output model.
 */
extern errno_t ifnet_set_sndq_maxlen(ifnet_t interface, u_int32_t maxqlen);

/*
 *       @function ifnet_get_sndq_maxlen
 *       @discussion Get the maximum length of the output queue of an
 *               interface which implements the new driver output model.
 *       @param interface The interface to get the max queue length on.
 *       @param maxqlen Pointer to a storage for the maximum number of packets
 *               in the output queue for all service classes.
 *       @result May return EINVAL if the parameters are invalid or ENXIO if
 *               the interface doesn't implement the new driver output model.
 */
extern errno_t ifnet_get_sndq_maxlen(ifnet_t interface, u_int32_t *maxqlen);

/*
 *       @function ifnet_get_sndq_len
 *       @discussion Get the current length of the output queue of an
 *               interface which implements the new driver output model.
 *       @param interface The interface to get the current queue length on.
 *       @param packets Pointer to a storage for the current number of packets
 *               in the aggregate output queue.  This number represents all
 *               enqueued packets regardless of their service classes.
 *       @result May return EINVAL if the parameters are invalid or ENXIO if
 *               the interface doesn't implement the new driver output model.
 */
extern errno_t ifnet_get_sndq_len(ifnet_t interface, u_int32_t *packets);

/*
 *       @function ifnet_get_service_class_sndq_len
 *       @discussion Get the current length of the output queue for a specific
 *               service class of an interface which implements the new driver
 *               output model.
 *       @param interface The interface to get the current queue length on.
 *       @param sc The service class.
 *       @param packets Pointer to a storage for the current number of packets
 *               of the specific service class in the output queue; may be
 *               NULL if caller is not interested in getting the value.  Note
 *               that multiple service classes may be mapped to an output queue;
 *               this routine reports the packet count of that output queue.
 *       @param bytes Pointer to a storage for the current size (in bytes) of
 *               the output queue specific to the service class; may be NULL if
 *               caller is not interested in getting the value.  Note that
 *               multiple service classes may be mapped to an output queue;
 *               this routine reports the length of that output queue.
 *       @result May return EINVAL if the parameters are invalid or ENXIO if
 *               the interface doesn't implement the new driver output model.
 */
extern errno_t ifnet_get_service_class_sndq_len(ifnet_t interface,
    mbuf_svc_class_t sc, u_int32_t *packets, u_int32_t *bytes);

/*
 *       @function ifnet_set_rcvq_maxlen
 *       @discussion Set the maximum length of the input queue of an
 *               interface which implements the new driver input model.
 *               This call may be issued post ifnet_allocate_extended in
 *               order to modify the maximum input queue length previously
 *               set at registration time.
 *       @param interface The interface to set the max queue length on.
 *       @param maxqlen The maximum number of packets in the input queue.
 *               Drivers typically set this to the size of the receive ring
 *               or the total number of descriptors used for the input path.
 *       @result May return EINVAL if the parameters are invalid or ENXIO if
 *               the interface doesn't implement the new driver input model.
 */
extern errno_t ifnet_set_rcvq_maxlen(ifnet_t interface, u_int32_t maxqlen);

/*
 *       @function ifnet_get_rcvq_maxlen
 *       @discussion Get the maximum length of the input queue of an
 *               interface which implements the new driver input model.
 *       @param interface The interface to get the max queue length on.
 *       @param maxqlen Pointer to a storage for the maximum number of packets
 *               in the input queue.
 *       @result May return EINVAL if the parameters are invalid or ENXIO if
 *               the interface doesn't implement the new driver input model.
 */
extern errno_t ifnet_get_rcvq_maxlen(ifnet_t interface, u_int32_t *maxqlen);

/*
 *       @struct ifnet_poll_params
 *       @discussion This structure is used to define various opportunistic
 *               polling parameters for an interface.
 *       @field flags Currently unused/ignored; must be set to zero.
 *       @field packets_limit The maximum number of packets to be dequeued
 *               each time the driver's input poll callback is invoked while
 *               in polling mode; this corresponds to the max_count parameter
 *               of ifnet_input_poll_func.  A zero value indicates the use of
 *               default maximum packets defined by the system.
 *       @field packets_lowat Low watermark packet threshold.
 *       @field packets_hiwat High watermark packet threshold.
 *       @field bytes_lowat Low watermark packet threshold.
 *       @field bytes_hiwat High watermark packet threshold.
 *               The low and high watermark inbound packet and bytes thresholds;
 *               these values may be link rate dependent.  Polling is enabled
 *               when the average inbound packets or bytes goes above the
 *               corresponding high watermark value; it stays in that mode until
 *               both of the average inbound packets and bytes go below their
 *               corresponding low watermark values.  Zero watermark values
 *               indicates the use of default thresholds as defined by the
 *               system.  Both low and high watermark values must either be
 *               zeroes, or both must be non-zeroes with low watermark value
 *               being less than the high watermark value.
 *       @field interval_time The interval time between each invocation of
 *               the driver's input poll callback, in nanoseconds.  A zero
 *               value indicates the use of default poll interval time as
 *               defined by the system.  If a non-zero value is specified and
 *               is less than the minimum interval time, the latter will be
 *               chosen by the system.
 */
struct ifnet_poll_params {
	u_int32_t       flags;
	u_int32_t       packets_limit;
	u_int32_t       packets_lowat;
	u_int32_t       packets_hiwat;
	u_int32_t       bytes_lowat;
	u_int32_t       bytes_hiwat;
	u_int64_t       interval_time;
	u_int64_t       reserved[4];
};

typedef struct ifnet_poll_params ifnet_poll_params_t;

/*
 *       @function ifnet_set_poll_params
 *       @discussion Configures opportunistic input polling parameters on an
 *               interface.  This call may be issued post ifnet_attach in order
 *               to modify the interface's polling parameters.  The driver may
 *               alter the default values chosen by the system to achieve the
 *               optimal performance for a given link rate or driver dynamics.
 *       @param interface The interface to configure opportunistic polling on.
 *       @param poll_params Pointer to the polling parameters.  If NULL, it
 *               implies that the system should revert the interface's polling
 *               parameter to their default values.
 *       @result May return EINVAL if the parameters are invalid or ENXIO if
 *               the interface doesn't implement the new driver input model.
 */
extern errno_t ifnet_set_poll_params(ifnet_t interface,
    ifnet_poll_params_t *poll_params);

/*
 *       @function ifnet_poll_params
 *       @discussion Retrieves opportunistic input polling parameters on an
 *               interface.  This call may be issued post ifnet_attach in order
 *               to retrieve the interface's polling parameters.
 *       @param interface The interface to configure opportunistic polling on.
 *       @param poll_params Pointer to the polling parameters.
 *       @result May return EINVAL if the parameters are invalid or ENXIO if
 *               the interface doesn't implement the new driver input model.
 */
extern errno_t ifnet_poll_params(ifnet_t interface,
    ifnet_poll_params_t *poll_params);

/*
 *       @function ifnet_start
 *       @discussion Trigger the transmission at the driver layer on an
 *               interface which implements the new driver output model.
 *       @param interface The interface to start the transmission on.
 */
extern void ifnet_start(ifnet_t interface);

/*
 *       @function ifnet_flowid
 *       @discussion Returns the interface flow ID value, which can be used
 *               by a (virtual) network interface for participating in the
 *               FLOWSRC_IFNET flow advisory mechanism.  The flow ID value
 *               is available after the interface is attached.
 *       @param interface The interface to retrieve the flow ID from.
 *       @param flowid Pointer to the flow ID value.
 *       @result May return EINVAL if the parameters are invalid or ENXIO if
 *               the interface doesn't implement the new driver input model.
 */
extern errno_t ifnet_flowid(ifnet_t interface, u_int32_t *flowid);

/*
 *       @function ifnet_enable_output
 *       @discussion Enables output on a (virtual) interface if it has been
 *               previously disabled via ifnet_disable_output().  This call
 *               is used to override the flow advisory mechanism typically
 *               used between a (virtual) network interface and a real network
 *               interface beneath it.  Under normal circumstances, the flow
 *               advisory mechanism will automatically re-enable the (virtual)
 *               interface's output mechanism when the real network interface
 *               is able to transmit more data.  Re-enabling output will cause
 *               the (virtual) interface's start callback to be called again.
 *       @param interface The interface to enable the transmission on.
 *       @result May return EINVAL if the parameters are invalid or ENXIO if
 *               the interface doesn't implement the new driver input model.
 */
extern errno_t ifnet_enable_output(ifnet_t interface);

/*
 *       @function ifnet_disable_output
 *       @discussion Disables output on a (virtual) interface.  Disabling
 *               output will cause the (virtual) interface's start callback
 *               to go idle.  This call is typically used by a (virtual)
 *               interface upon receiving flow control feedbacks from the
 *               real network interface beneath it, in order propagate the
 *               flow control condition to the layers above.  Under normal
 *               circumstances, the flow advisory mechanism will automatically
 *               re-enable the (virtual) interface's output mechanism when
 *               the real network interface is able to transmit more data,
 *               as long as the (virtual) interface participates in the
 *               FLOWSRC_IFNET flow advisory for the data that it emits.
 *       @param interface The interface to disable the transmission on.
 *       @result May return EINVAL if the parameters are invalid or ENXIO if
 *               the interface doesn't implement the new driver input model.
 */
extern errno_t ifnet_disable_output(ifnet_t interface);
#endif /* KERNEL_PRIVATE */

/*!
 *       @function ifnet_reference
 *       @discussion Increment the reference count of the ifnet to assure
 *               that it will not go away. The interface must already have at
 *               least one reference.
 *       @param interface The interface to increment the reference count of.
 *       @result May return EINVAL if the interface is not valid.
 */
extern errno_t ifnet_reference(ifnet_t interface);

/*!
 *       @function ifnet_release
 *       @discussion Release a reference of the ifnet, this may trigger a
 *               free if the reference count reaches 0.
 *       @param interface The interface to decrement the reference count of
 *               and possibly free.
 *       @result May return EINVAL if the interface is not valid.
 */
extern errno_t ifnet_release(ifnet_t interface);

/*!
 *       @function ifnet_attach
 *       @discussion Attaches an interface to the global interface list. The
 *               interface must be setup properly before calling attach. The
 *               stack will take a reference on the interface and hold it until
 *               ifnet_detach is called.
 *
 *               This function is intended to be called by the driver. A kext
 *               must not call this function on an interface the kext does not
 *               own.
 *       @param interface The interface to attach.
 *       @param ll_addr The link layer address of the interface. This is used
 *               to fill out the first ifaddr in the list of addresses for the
 *               interface. This parameter is not required for interfaces such as
 *               PPP that have no link-layer address.
 *       @result Will return an error if there is anything wrong with the
 *               interface.
 */
extern errno_t ifnet_attach(ifnet_t interface,
    const struct sockaddr_dl *ll_addr);

/*!
 *       @function ifnet_detach
 *       @discussion Detaches the interface.
 *
 *               Call this to indicate this interface is no longer valid (i.e. PC
 *               Card was removed). This function will begin the process of
 *               removing knowledge of this interface from the stack.
 *
 *               The function will return before the interface is detached. The
 *               functions you supplied in to the interface may continue to be
 *               called. When the detach has been completed, your detached
 *               function will be called. Your kext must not unload until the
 *               detached function has been called. The interface will be
 *               properly freed when the reference count reaches zero.
 *
 *               An interface may not be attached again. You must call
 *               ifnet_allocate to create a new interface to attach.
 *
 *               This function is intended to be called by the driver. A kext
 *               must not call this function on an interface the kext does not
 *               own.
 *       @param interface The interface to detach.
 *       @result 0 on success, otherwise errno error.
 */
extern errno_t ifnet_detach(ifnet_t interface);

/*!
 *       @function ifnet_interface_family_find
 *       @discussion Look up the interface family identifier for a string.
 *               If there is no interface family identifier assigned for this string
 *               a new interface family identifier is created and assigned.
 *               It is recommended to use the bundle id of the KEXT as the string
 *               to avoid collisions with other KEXTs.
 *               The lookup operation is not optimized so a module should call this
 *               function once during startup and cache the interface family identifier.
 *               The interface family identifier for a string will not be re-assigned until
 *               the system reboots.
 *       @param module_string  A unique string identifying your interface family
 *       @param family_id Upon return, a unique interface family identifier for use with
 *               ifnet_* functions. This identifier is valid until the system
 *               is rebooted.
 *       @result 0 on success, otherwise errno error.
 */
extern errno_t ifnet_interface_family_find(const char *module_string, ifnet_family_t *family_id);

/*
 * Interface manipulation.
 */

/*!
 *       @function ifnet_softc
 *       @discussion Returns the driver's private storage on the interface.
 *       @param interface Interface to retrieve the storage from.
 *       @result Driver's private storage.
 */
extern void *ifnet_softc(ifnet_t interface);

/*!
 *       @function ifnet_name
 *       @discussion Returns a pointer to the name of the interface.
 *       @param interface Interface to retrieve the name from.
 *       @result Pointer to the name.
 */
extern const char *ifnet_name(ifnet_t interface);

/*!
 *       @function ifnet_family
 *       @discussion Returns the family of the interface.
 *       @param interface Interface to retrieve the family from.
 *       @result Interface family type.
 */
extern ifnet_family_t ifnet_family(ifnet_t interface);

#ifdef KERNEL_PRIVATE
/*
 *       @function ifnet_subfamily
 *       @discussion Returns the sub-family of the interface.
 *       @param interface Interface to retrieve the sub-family from.
 *       @result Interface sub-family type.
 */
extern ifnet_subfamily_t ifnet_subfamily(ifnet_t interface);
#endif /* KERNEL_PRIVATE */

/*!
 *       @function ifnet_unit
 *       @discussion Returns the unit number of the interface.
 *       @param interface Interface to retrieve the unit number from.
 *       @result Unit number.
 */
extern u_int32_t ifnet_unit(ifnet_t interface);

/*!
 *       @function ifnet_index
 *       @discussion Returns the index of the interface. This index value
 *               will match the index you would find in a sockaddr_dl or using
 *               if_nametoindex or if_indextoname in user space. The value of the
 *               interface index is undefined for an interface that is not
 *               currently attached.
 *       @param interface Interface to retrieve the index of.
 *       @result Index.
 */
extern u_int32_t ifnet_index(ifnet_t interface);

/*!
 *       @function ifnet_set_flags
 *       @discussion Sets the interface flags to match new_flags.
 *       @discussion Sets the interface flags to new_flags. This function
 *               lets you specify which flags you want to change using the mask.
 *               The kernel will effectively take the lock, then set the
 *               interface's flags to (if_flags & ~mask) | (new_flags & mask).
 *       @param interface Interface to set the flags on.
 *       @param new_flags The new set of flags that should be set. These
 *               flags are defined in net/if.h
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_flags(ifnet_t interface, u_int16_t new_flags,
    u_int16_t mask);

/*!
 *       @function ifnet_flags
 *       @discussion Returns the interface flags that are set.
 *       @param interface Interface to retrieve the flags from.
 *       @result Flags. These flags are defined in net/if.h
 */
extern u_int16_t ifnet_flags(ifnet_t interface);


#ifdef KERNEL_PRIVATE
/*
 *       @function ifnet_set_eflags
 *       @discussion Sets the extended interface flags to new_flags. This
 *               function lets you specify which flags you want to change using
 *               the mask. The kernel will effectively take the lock, then set
 *               the interface's extended flags to (if_eflags & ~mask) |
 *               (new_flags & mask).
 *       @param interface The interface.
 *       @param new_flags The new set of flags that should be set. These
 *               flags are defined in net/if.h
 *       @param mask The mask of flags to be modified.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_eflags(ifnet_t interface, u_int32_t new_flags,
    u_int32_t mask);

/*
 *       @function ifnet_eflags
 *       @discussion Returns the extended interface flags that are set.
 *       @param interface Interface to retrieve the flags from.
 *       @result Extended flags. These flags are defined in net/if.h
 */
extern u_int32_t ifnet_eflags(ifnet_t interface);

/*
 *       @function ifnet_set_idle_flags
 *       @discussion Sets the if_idle_flags to new_flags. This function
 *               lets you specify which flags you want to change using the
 *               mask. The kernel will effectively take the lock, then set
 *               the interface's idle flags to:
 *                       (if_idle_flags & ~mask) | (new_flags & mask).
 *               Setting the flags to any non-zero value will cause the
 *               networking stack to aggressively purge expired objects,
 *               such as route entries, etc.
 *       @param interface The interface.
 *       @param new_flags The new set of flags that should be set. These
 *               flags are defined in net/if.h
 *       @param mask The mask of flags to be modified.
 *       @result 0 on success otherwise the errno error.  ENOTSUP is returned
 *               when this call is made on non-supporting platforms.
 */
extern errno_t ifnet_set_idle_flags(ifnet_t interface, u_int32_t new_flags,
    u_int32_t mask);

/*
 *       @function ifnet_idle_flags
 *       @discussion Returns the value of if_idle_flags.
 *       @param interface Interface to retrieve the flags from.
 *       @result if_idle_flags. These flags are defined in net/if.h
 */
extern u_int32_t ifnet_idle_flags(ifnet_t interface);

/*
 *       @function ifnet_set_link_quality
 *       @discussion Sets the Link Quality Metric for the ifnet.
 *       @param interface Interface for which the Link Quality Metric should
 *               be associated to.
 *       @param quality IFNET_LQM value as defined in net/if.h.
 *       @result 0 on success otherwise the errno error.  EINVAL if quality
 *               is not a valid value.  ENXIO if the interface is not attached.
 */
extern errno_t ifnet_set_link_quality(ifnet_t interface, int quality);

/*
 *       @function ifnet_link_quality
 *       @discussion Returns the Link Quality Metric for the ifnet.
 *       @param interface Interface to retrieve the value from.
 *       @result IFNET_LQM as defined in net/if.h
 */
extern int ifnet_link_quality(ifnet_t interface);

/*
 *       @function ifnet_set_interface_state
 *       @discussion Sets the interface state for the ifnet.
 *       @param interface Interface for which the interface state should
 *               be set to.
 *       @param if_interface_state as defined in net/if_var.h.
 *       @result 0 on success otherwise the errno error.  EINVAL if quality
 *               is not a valid value.  ENXIO if the interface is not attached.
 */
extern errno_t ifnet_set_interface_state(ifnet_t interface,
    struct if_interface_state *if_interface_state);

/*
 *       @function ifnet_get_interface_state
 *       @discussion Returns the interface state for the ifnet.
 *       @param if_interface_state to ret.
 *       @result 0 on success, errno otherwise
 */
extern int ifnet_get_interface_state(ifnet_t interface,
    struct if_interface_state *if_interface_state);

/*
 *       @struct ifnet_llreach_info
 *       @discussion This structure is used to describe the link-layer
 *               reachability information of an on-link node.
 *       @field iflri_refcnt The number of network-layer objects referring
 *               to this link-layer reachability record.
 *       @field iflri_probes The total number of outstanding probes.
 *       @field iflri_snd_expire The send expiration time.  This is calculated
 *               based on the last time the system transmitted a packet to the
 *               node.  A zero value indicates that a packet has not been sent
 *               to the node.  A non-zero value indicates the time before the
 *               record is determined to be invalid.  When the record is no
 *               longer valid, the system will send probe(s) to resolve the
 *               node again.  This value is relative to the current time
 *               specified in iflri_curtime.
 *       @field iflri_rcv_expire The receive expiriation time.  This is
 *               calculated based on the last time the system received a packet
 *               from the node.  A zero value indicates that a packet has not
 *               been received from the node.  A non-zero value indicates the
 *               time before the record is determined to be invalid.  When the
 *               record is no longer valid, the system will send probe(s) to
 *               resolve the node again.  This value is relative to the current
 *               time specified in iflri_curtime.
 *       @field iflri_curtime The current time when this record was retrieved.
 *       @field iflri_netproto The protocol number of the network-layer object.
 *       @field iflri_addr The link-layer address of the node.
 *       @field iflri_rssi The received signal strength indication (RSSI) of the
 *               node in dBm.  The special value IFNET_RSSI_UNKNOWN is used when
 *               the RSSI is either unknown or inapplicable for the interface.
 *       @field iflri_lqm The link quality metric (LQM) to the node.  The
 *               special value IFNET_LQM_UNKNOWN is used when the LQM is not
 *               currently known.  The special value IFNET_LQM_OFF is used when
 *               the link quality metric is inapplicable to nodes at this
 *               attached to the network at this interface.
 *       @field iflri_npm The node proximity metric (NPM) to the node.  The
 *               special value IFNET_NPM_UNKNOWN is used when the NPM is not
 *               currently known.
 */
#define IFNET_LLREACHINFO_ADDRLEN       64      /* max ll addr len */

struct ifnet_llreach_info {
	u_int32_t iflri_refcnt;
	u_int32_t iflri_probes;
	u_int64_t iflri_snd_expire;
	u_int64_t iflri_rcv_expire;
	u_int64_t iflri_curtime;
	u_int32_t iflri_netproto;
	u_int8_t  iflri_addr[IFNET_LLREACHINFO_ADDRLEN];
	int32_t   iflri_rssi;
	int32_t   iflri_lqm;
	int32_t   iflri_npm;
};

/*
 *       @function ifnet_inet_defrouter_llreachinfo
 *       @discussion Retrieve link-layer reachability information of the
 *               default IPv4 router specific to the interface.
 *       @param interface The interface associated with the default IPv4 router.
 *       @param pinfo Pointer to the ifnet_llreach_info structure where the
 *               information will be returned to, upon success.
 *       @result	0 upon success, otherwise errno error.
 */
extern errno_t ifnet_inet_defrouter_llreachinfo(ifnet_t interface,
    struct ifnet_llreach_info *pinfo);

/*
 *       @function ifnet_inet6_defrouter_llreachinfo
 *       @discussion Retrieve link-layer reachability information of the
 *               default IPv6 router specific to the interface.
 *       @param interface The interface associated with the default IPv6 router.
 *       @param pinfo Pointer to the ifnet_llreach_info structure where the
 *               information will be returned to, upon success.
 *       @result	0 upon success, otherwise errno error.
 */
extern errno_t ifnet_inet6_defrouter_llreachinfo(ifnet_t interface,
    struct ifnet_llreach_info *pinfo);
#endif /* KERNEL_PRIVATE */

/*!
 *       @function ifnet_set_capabilities_supported
 *       @discussion Specify the capabilities supported by the interface.
 *       @discussion  This function lets you specify which capabilities are supported
 *               by the interface. Typically this function is called by the driver when
 *               the interface gets attached to the system.
 *               The mask allows to control which capability to set or unset.
 *               The kernel will effectively take the lock, then set the
 *               interface's flags to (if_capabilities & ~mask) | (new_caps & mask).
 *
 *               This function is intended to be called by the driver. A kext
 *               must not call this function on an interface the kext does not
 *               own.
 *       @param interface Interface to set the capabilities on.
 *       @param new_caps The value of the capabilities that should be set or unset. These
 *               flags are defined in net/if.h
 *       @param mask Which capabilities that should be affected. These
 *               flags are defined in net/if.h
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_capabilities_supported(ifnet_t interface, u_int32_t new_caps,
    u_int32_t mask);

/*!
 *       @function ifnet_capabilities_supported
 *       @discussion Retrieve the interface capabilities supported by the interface.
 *       @param interface Interface to retrieve the capabilities from.
 *       @result Flags. Capabilities flags are defined in net/if.h
 */
extern u_int32_t ifnet_capabilities_supported(ifnet_t interface);

/*!
 *       @function ifnet_set_capabilities_enabled
 *       @discussion Enable and/or disable the interface capabilities to match new_caps.
 *       @discussion Sets the interface capabilities to new_caps. This function
 *               lets you specify which capabilities you want to change using the mask.
 *               The kernel will effectively take the lock, then set the
 *               interface's flags to (if_capenable & ~mask) | (new_caps & mask).
 *
 *               This function is intended to be called by the driver. A kext
 *               must not call this function on an interface the kext does not
 *               own.
 *
 *               Typically this function is called by the driver when the interface is
 *               created to specify which of the supported capabilities are enabled by
 *               default. This function is also meant to be called when the driver handles
 *               the interface ioctl SIOCSIFCAP.
 *
 *               The driver should call ifnet_set_offlad() to indicate the corresponding
 *               hardware offload bits that will be used by the networking stack.
 *
 *               It is an error to enable a capability that is not marked as
 *               supported by the interface.
 *       @param interface Interface to set the capabilities on.
 *       @param new_caps The value of the capabilities that should be set or unset. These
 *               flags are defined in net/if.h
 *       @param mask Which capabilities that should be affected. These
 *               flags are defined in net/if.h
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_capabilities_enabled(ifnet_t interface, u_int32_t new_caps,
    u_int32_t mask);

/*!
 *       @function ifnet_capabilities_enabled
 *       @discussion Retrieve the interface capabilities enabled on the interface.
 *       @param interface Interface to retrieve the capabilities from.
 *       @result Flags. Capabilities flags are defined in net/if.h
 */
extern u_int32_t ifnet_capabilities_enabled(ifnet_t interface);


/*!
 *       @function ifnet_set_offload
 *       @discussion Sets a bitfield to indicate special hardware offload
 *               support provided by the interface such as hardware checksums and
 *               VLAN. This replaces the if_hwassist flags field. Any flags
 *               unrecognized by the stack will not be set.
 *
 *               Note the system will automatically set the interface capabilities
 *               that correspond to the offload flags modified -- i.e. the driver
 *               does not have to call ifnet_set_capabilities_enabled() and
 *               ifnet_set_capabilities_supported().
 *       @param interface The interface.
 *       @param offload The new set of flags indicating which offload options
 *               the device supports.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_offload(ifnet_t interface, ifnet_offload_t offload);

/*!
 *       @function ifnet_offload
 *       @discussion Returns flags indicating which operations can be
 *               offloaded to the interface.
 *       @param interface Interface to retrieve the offload from.
 *       @result Abilities flags, see ifnet_offload_t.
 */
extern ifnet_offload_t ifnet_offload(ifnet_t interface);

/*!
 *       @function ifnet_set_tso_mtu
 *       @discussion Sets maximum TCP Segmentation Offload segment size for
 *               the interface
 *       @param interface The interface.
 *       @param family The family for which the offload MTU is provided for
 *               (AF_INET or AF_INET6)
 *       @param mtuLen Maximum segment size supported by the interface
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_tso_mtu(ifnet_t interface, sa_family_t family,
    u_int32_t mtuLen);

/*!
 *       @function ifnet_get_tso_mtu
 *       @discussion Returns maximum TCP Segmentation Offload segment size for
 *               the interface
 *       @param interface The interface.
 *       @param family The family for which the offload MTU is provided for
 *               (AF_INET or AF_INET6)
 *       @param mtuLen Value of the maximum MTU supported for the interface
 *               and family provided.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_get_tso_mtu(ifnet_t interface, sa_family_t family,
    u_int32_t *mtuLen);

/*!
 *       @enum Interface wake properties
 *       @abstract Constants defining Interface wake properties.
 *       @constant IFNET_WAKE_ON_MAGIC_PACKET Wake on Magic Packet.
 */
enum {
	IFNET_WAKE_ON_MAGIC_PACKET = 0x01
};

/*!
 *       @function ifnet_set_wake_flags
 *       @discussion Sets the wake properties of the underlying hardware. These are
 *               typically set by the driver.
 *       @param interface The interface.
 *       @param properties Properties to set or unset.
 *       @param mask Mask of the properties to set of unset.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_wake_flags(ifnet_t interface, u_int32_t properties, u_int32_t mask);

/*!
 *       @function ifnet_get_wake_flags
 *       @discussion Returns the wake properties set on the interface.
 *       @param interface The interface.
 *       @result The wake properties
 */
extern u_int32_t ifnet_get_wake_flags(ifnet_t interface);

/*!
 *       @function ifnet_set_link_mib_data
 *       @discussion Sets the mib link data. The ifnet_t will store the
 *               pointer you supply and copy mibLen bytes from the pointer
 *               whenever the sysctl for getting interface specific MIB data is
 *               used. Since the ifnet_t stores a pointer to your data instead of
 *               a copy, you may update the data at the address at any time.
 *
 *               This function is intended to be called by the driver. A kext
 *               must not call this function on an interface the kext does not
 *               own.
 *       @param interface Interface to set the unit number of.
 *       @param mibData A pointer to the data.
 *       @param mibLen Length of data pointed to.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_link_mib_data(ifnet_t interface, void *mibData,
    u_int32_t mibLen);

/*!
 *       @function ifnet_get_link_mib_data
 *       @discussion Copies the link MIB data in to mibData, up to mibLen
 *               bytes. Returns error if the buffer is too small to hold all of
 *               the MIB data.
 *       @param interface The interface.
 *       @param mibData A pointer to space for the mibData to be copied in
 *               to.
 *       @param mibLen When calling, this should be the size of the buffer
 *               passed in mibData. Upon return, this will be the size of data
 *               copied in to mibData.
 *       @result Returns an error if the buffer size is too small or there is
 *               no data.
 */
extern errno_t ifnet_get_link_mib_data(ifnet_t interface, void *mibData,
    u_int32_t *mibLen);

/*!
 *       @function ifnet_get_link_mib_data_length
 *       @discussion Retrieve the size of the mib data.
 *       @param interface The interface.
 *       @result Returns the number of bytes of mib data associated with the
 *               interface.
 */
extern u_int32_t ifnet_get_link_mib_data_length(ifnet_t interface);

/*!
 *       @function ifnet_attach_protocol
 *       @discussion Attaches a protocol to an interface.
 *       @param interface The interface.
 *       @param protocol_family The protocol family being attached
 *               (PF_INET/PF_INET6/etc...).
 *       @param proto_details Details of the protocol being attached.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_attach_protocol(ifnet_t interface,
    protocol_family_t protocol_family,
    const struct ifnet_attach_proto_param *proto_details);

/*!
 *       @function ifnet_attach_protocol_v2
 *       @discussion Attaches a protocol to an interface using the newer
 *           version 2 style interface. So far the only difference is support
 *           for packet chains which improve performance.
 *       @param interface The interface.
 *       @param protocol_family The protocol family being attached
 *               (PF_INET/PF_INET6/etc...).
 *       @param proto_details Details of the protocol being attached.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_attach_protocol_v2(ifnet_t interface,
    protocol_family_t protocol_family,
    const struct ifnet_attach_proto_param_v2 *proto_details);

/*!
 *       @function ifnet_detach_protocol
 *       @discussion Detaches a protocol from an interface.
 *       @param interface The interface.
 *       @param protocol_family The protocol family of the protocol to
 *               detach.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_detach_protocol(ifnet_t interface,
    protocol_family_t protocol_family);

/*!
 *       @function ifnet_output
 *       @discussion Handles an outbound packet on the interface by calling
 *               any filters, a protocol preoutput function, the interface framer
 *               function, and finally the interface's output function. The
 *               protocol_family will be used to apply protocol filters and
 *               determine which preoutput function to call. The route and dest
 *               parameters will be passed to the preoutput function defined for
 *               the attachment of the specified protocol to the specified
 *               interface. ifnet_output will always free the mbuf chain.
 *       @param interface The interface.
 *       @param protocol_family The family of the protocol generating this
 *               packet (i.e. AF_INET).
 *       @param packet The packet to be transmitted.
 *       @param route A pointer to a routing structure for this packet. The
 *               preoutput function determines whether this value may be NULL or
 *               not.
 *       @param dest The destination address of protocol_family type. This
 *               will be passed to the preoutput function. If the preoutput
 *               function does not require this value, you may pass NULL.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_output(ifnet_t interface,
    protocol_family_t protocol_family, mbuf_t packet, void *route,
    const struct sockaddr *dest);

/*!
 *       @function ifnet_output_raw
 *       @discussion Handles and outbond raw packet on the interface by
 *               calling any filters followed by the interface's output function.
 *               protocol_family may be zero. If the packet is from a specific
 *               protocol the protocol_family will be used to apply protocol
 *               filters. All interface filters will be applied to the outgoing
 *               packet. Processing, such as calling the protocol preoutput and
 *               interface framer functions will be bypassed. The packet will
 *               pass through the filters and be sent on the interface as is.
 *               ifnet_output_raw will always free the packet chain.
 *       @param interface The interface.
 *       @param protocol_family The family of the protocol generating this
 *               packet (i.e. AF_INET).
 *       @param packet The fully formed packet to be transmitted.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_output_raw(ifnet_t interface,
    protocol_family_t protocol_family, mbuf_t packet);

/*!
 *       @function ifnet_input
 *       @discussion Inputs packets from the interface. The interface's demux
 *               will be called to determine the protocol. Once the protocol is
 *               determined, the interface filters and protocol filters will be
 *               called. From there, the packet will be passed to the registered
 *               protocol. If there is an error, the mbuf chain will be freed.
 *       @param interface The interface.
 *       @param first_packet The first packet in a chain of packets.
 *       @param stats Counts to be integrated in to the stats. The interface
 *               statistics will be incremented by the amounts specified in
 *               stats. This parameter may be NULL.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_input(ifnet_t interface, mbuf_t first_packet,
    const struct ifnet_stat_increment_param *stats);

#ifdef KERNEL_PRIVATE
/*
 *       @function ifnet_input_extended
 *       @discussion Inputs packets from the interface. The interface's demux
 *               will be called to determine the protocol. Once the protocol is
 *               determined, the interface filters and protocol filters will be
 *               called. From there, the packet will be passed to the registered
 *               protocol. If there is an error, the mbuf chain will be freed.
 *       @param interface The interface.
 *       @param first_packet The first packet in a chain of packets.
 *       @param last_packet The last packet in a chain of packets.  This may be
 *               set to NULL if the driver does not have the information.
 *       @param stats Counts to be integrated in to the stats. The interface
 *               statistics will be incremented by the amounts specified in
 *               stats. Unlike ifnet_input(), this parameter is required by
 *               this extended variant.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_input_extended(ifnet_t interface, mbuf_t first_packet,
    mbuf_t last_packet, const struct ifnet_stat_increment_param *stats);
#endif /* KERNEL_PRIVATE */

/*!
 *       @function ifnet_ioctl
 *       @discussion Calls the interface's ioctl function with the parameters
 *               passed.
 *
 *               All undefined ioctls are reserved for future use by Apple. If
 *               you need to communicate with your kext using an ioctl, please
 *               use SIOCSIFKPI and SIOCGIFKPI.
 *       @param interface The interface.
 *       @param protocol The protocol family of the protocol to send the
 *               ioctl to (may be zero). Some ioctls apply to a protocol while
 *               other ioctls apply to just an interface.
 *       @param ioctl_code The ioctl to perform.
 *       @param ioctl_arg Any parameters to the ioctl.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_ioctl(ifnet_t interface, protocol_family_t protocol,
    unsigned long ioctl_code, void *ioctl_arg);

/*!
 *       @function ifnet_event
 *       @discussion Calls the interface's event function.
 *       @param interface The interface.
 *       @param event_ptr Pointer to an kern_event structure describing the
 *               event.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_event(ifnet_t interface, struct kern_event_msg *event_ptr);

/*!
 *       @function ifnet_set_mtu
 *       @discussion Sets the value of the MTU in the interface structure.
 *               Calling this function will not notify the driver that the MTU
 *               should be changed. Use the appropriate ioctl.
 *
 *               This function is intended to be called by the driver. A kext
 *               must not call this function on an interface the kext does not
 *               own.
 *       @param interface The interface.
 *       @param mtu The new MTU.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_mtu(ifnet_t interface, u_int32_t mtu);

/*!
 *       @function ifnet_mtu
 *       @param interface The interface.
 *       @result The MTU.
 */
extern u_int32_t ifnet_mtu(ifnet_t interface);

/*!
 *       @function ifnet_type
 *       @param interface The interface.
 *       @result The type. See net/if_types.h.
 */
extern u_int8_t ifnet_type(ifnet_t interface);

/*!
 *       @function ifnet_set_addrlen
 *       @discussion
 *               This function is intended to be called by the driver. A kext
 *               must not call this function on an interface the kext does not
 *               own.
 *       @param interface The interface.
 *       @param addrlen The new address length.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_addrlen(ifnet_t interface, u_int8_t addrlen);

/*!
 *       @function ifnet_addrlen
 *       @param interface The interface.
 *       @result The address length.
 */
extern u_int8_t ifnet_addrlen(ifnet_t interface);

/*!
 *       @function ifnet_set_hdrlen
 *       @discussion
 *               This function is intended to be called by the driver. A kext
 *               must not call this function on an interface the kext does not
 *               own.
 *       @param interface The interface.
 *       @param hdrlen The new header length.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_hdrlen(ifnet_t interface, u_int8_t hdrlen);

/*!
 *       @function ifnet_hdrlen
 *       @param interface The interface.
 *       @result The header length.
 */
extern u_int8_t ifnet_hdrlen(ifnet_t interface);

/*!
 *       @function ifnet_set_metric
 *       @discussion
 *               This function is intended to be called by the driver. A kext
 *               must not call this function on an interface the kext does not
 *               own.
 *       @param interface The interface.
 *       @param metric The new metric.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_metric(ifnet_t interface, u_int32_t metric);

/*!
 *       @function ifnet_metric
 *       @param interface The interface.
 *       @result The metric.
 */
extern u_int32_t ifnet_metric(ifnet_t interface);

/*!
 *       @function ifnet_set_baudrate
 *       @discussion
 *               This function is intended to be called by the driver. A kext
 *               must not call this function on an interface the kext does not
 *               own.
 *       @param interface The interface.
 *       @param baudrate The new baudrate.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_baudrate(ifnet_t interface, u_int64_t baudrate);

/*!
 *       @function ifnet_baudrate
 *       @param interface The interface.
 *       @result The baudrate.
 */
extern u_int64_t ifnet_baudrate(ifnet_t interface);

#ifdef KERNEL_PRIVATE
typedef struct if_bandwidths if_bandwidths_t;

/*
 *       @function ifnet_set_bandwidths
 *       @discussion This function allows a driver to indicate the output
 *               and/or input bandwidth information to the system.  Each set
 *               is comprised of the effective and maximum theoretical values.
 *               Each value must be greater than zero.
 *       @param interface The interface.
 *       @param output_bw The output bandwidth values (in bits per second).
 *               May be set to NULL if the caller does not want to alter the
 *               existing output bandwidth values.
 *       @param input_bw The input bandwidth values (in bits per second).
 *               May be set to NULL if the caller does not want to alter the
 *               existing input bandwidth values.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_bandwidths(ifnet_t interface,
    if_bandwidths_t *output_bw, if_bandwidths_t *input_bw);

/*
 *       @function ifnet_bandwidths
 *       @param interface The interface.
 *       @param output_bw The output bandwidth values (in bits per second).
 *               May be set to NULL if the caller does not want to retrieve the
 *               output bandwidth value.
 *       @param input_bw The input bandwidth values (in bits per second).
 *               May be set to NULL if the caller does not want to retrieve the
 *               input bandwidth value.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_bandwidths(ifnet_t interface, if_bandwidths_t *output_bw,
    if_bandwidths_t *input_bw);

typedef struct if_latencies if_latencies_t;

/*
 *       @function ifnet_set_latencies
 *       @discussion This function allows a driver to indicate the output
 *               and/or input latency information to the system.  Each set
 *               is comprised of the effective and maximum theoretical values.
 *               Each value must be greater than zero.
 *       @param interface The interface.
 *       @param output_lt The output latency values (in nanosecond).
 *               May be set to NULL if the caller does not want to alter the
 *               existing output latency values.
 *       @param input_lt The input latency values (in nanosecond).
 *               May be set to NULL if the caller does not want to alter the
 *               existing input latency values.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_latencies(ifnet_t interface,
    if_latencies_t *output_lt, if_latencies_t *input_lt);

/*
 *       @function ifnet_latencies
 *       @param interface The interface.
 *       @param output_lt The output latency values (in nanosecond).
 *               May be set to NULL if the caller does not want to retrieve the
 *               output latency value.
 *       @param input_lt The input latency values (in nanosecond).
 *               May be set to NULL if the caller does not want to retrieve the
 *               input latency value.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_latencies(ifnet_t interface, if_latencies_t *output_lt,
    if_latencies_t *input_lt);
#endif /* KERNEL_PRIVATE */

/*!
 *       @function ifnet_stat_increment
 *       @discussion
 *               This function is intended to be called by the driver. A kext
 *               must not call this function on an interface the kext does not
 *               own.
 *       @param interface The interface.
 *       @param counts A pointer to a structure containing the amount to
 *               increment each counter by. Any counts not appearing in the
 *               ifnet_counter_increment structure are handled in the stack.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_stat_increment(ifnet_t interface,
    const struct ifnet_stat_increment_param *counts);

/*!
 *       @function ifnet_stat_increment_in
 *       @discussion
 *               This function is intended to be called by the driver. This
 *               function allows a driver to update the inbound interface counts.
 *               The most efficient time to update these counts is when calling
 *               ifnet_input.
 *
 *               A lock protects the counts, this makes the increment functions
 *               expensive. The increment function will update the lastchanged
 *               value.
 *       @param interface The interface.
 *       @param packets_in The number of additional packets received.
 *       @param bytes_in The number of additional bytes received.
 *       @param errors_in The number of additional receive errors.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_stat_increment_in(ifnet_t interface,
    u_int32_t packets_in, u_int32_t bytes_in, u_int32_t errors_in);

/*!
 *       @function ifnet_stat_increment_out
 *       @discussion
 *               This function is intended to be called by the driver. This
 *               function allows a driver to update the outbound interface
 *               counts.
 *
 *               A lock protects the counts, this makes the increment functions
 *               expensive. The increment function will update the lastchanged
 *               value.
 *       @param interface The interface.
 *       @param packets_out The number of additional packets sent.
 *       @param bytes_out The number of additional bytes sent.
 *       @param errors_out The number of additional send errors.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_stat_increment_out(ifnet_t interface,
    u_int32_t packets_out, u_int32_t bytes_out, u_int32_t errors_out);

/*!
 *       @function ifnet_set_stat
 *       @discussion
 *               This function is intended to be called by the driver. A kext
 *               must not call this function on an interface the kext does not
 *               own.
 *
 *               The one exception would be the case where a kext wants to zero
 *               all of the counters.
 *       @param interface The interface.
 *       @param stats The new stats values.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_stat(ifnet_t interface,
    const struct ifnet_stats_param *stats);

/*!
 *       @function ifnet_stat
 *       @param interface The interface.
 *       @param out_stats Storage for the values.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_stat(ifnet_t interface,
    struct ifnet_stats_param *out_stats);

/*!
 *       @function ifnet_set_promiscuous
 *       @discussion Enable or disable promiscuous mode on the interface. The
 *               interface keeps an internal count of the number of times
 *               promiscuous mode has been enabled. Promiscuous mode is only
 *               disabled when this count reaches zero. Be sure to disable
 *               promiscuous mode only once for every time you enable it.
 *       @param interface The interface to toggle promiscuous mode on.
 *       @param on If set, the number of promicuous on requests will be
 *               incremented. If this is the first requrest, promiscuous mode
 *               will be enabled. If this is not set, the number of promiscous
 *               clients will be decremented. If this causes the number to reach
 *               zero, promiscuous mode will be disabled.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_set_promiscuous(ifnet_t interface, int on);

/*!
 *       @function ifnet_touch_lastchange
 *       @discussion Updates the lastchange value to now.
 *       @param interface The interface.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_touch_lastchange(ifnet_t interface);

/*!
 *       @function ifnet_lastchange
 *       @param interface The interface.
 *       @param last_change A timeval struct to copy the last time changed in
 *               to.
 */
extern errno_t ifnet_lastchange(ifnet_t interface, struct timeval *last_change);

/*!
 *       @function ifnet_get_address_list
 *       @discussion Get a list of addresses on the interface. Passing NULL
 *               for the interface will return a list of all addresses. The
 *               addresses will have their reference count bumped so they will
 *               not go away. Calling ifnet_free_address_list will decrement the
 *               refcount and free the array. If you wish to hold on to a
 *               reference to an ifaddr_t, be sure to bump the reference count
 *               before calling ifnet_free_address_list.
 *       @param interface The interface.
 *       @param addresses A pointer to a NULL terminated array of ifaddr_ts.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_get_address_list(ifnet_t interface, ifaddr_t **addresses);

/*!
 *       @function ifnet_get_address_list_family
 *       @discussion Get a list of addresses on the interface. Passing NULL
 *               for the interface will return a list of all addresses. The
 *               addresses will have their reference count bumped so they will
 *               not go away. Calling ifnet_free_address_list will decrement the
 *               refcount and free the array. If you wish to hold on to a
 *               reference to an ifaddr_t, be sure to bump the reference count
 *               before calling ifnet_free_address_list. Unlike
 *               ifnet_get_address_list, this function lets the caller specify
 *               the address family to get a list of only a specific address type.
 *       @param interface The interface.
 *       @param addresses A pointer to a NULL terminated array of ifaddr_ts.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_get_address_list_family(ifnet_t interface,
    ifaddr_t **addresses, sa_family_t family);

#ifdef KERNEL_PRIVATE
/*!
 *       @function ifnet_get_inuse_address_list
 *       @discussion Get a list of addresses on the interface that are in
 *       use by atleast one TCP or UDP socket. The rest of the API is similar
 *       to ifnet_get_address_list. Calling ifnet_free_address_list will
 *       free the array of addresses. Note this only gives a point in time
 *       snapshot of the addresses in use.
 *       @param interface The interface
 *       @param addresses A pointer to a NULL terminated array of ifaddr_ts
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_get_inuse_address_list(ifnet_t interface,
    ifaddr_t **addresses);

__private_extern__ errno_t ifnet_get_address_list_family_internal(ifnet_t,
    ifaddr_t **, sa_family_t, int, int, int);
#endif /* KERNEL_PRIVATE */

/*!
 *       @function ifnet_free_address_list
 *       @discussion Free a list of addresses returned from
 *               ifnet_get_address_list. Decrements the refcounts and frees the
 *               memory used for the array of references.
 *       @param addresses An array of ifaddr_ts.
 */
extern void ifnet_free_address_list(ifaddr_t *addresses);

/*!
 *       @function ifnet_set_lladdr
 *       @discussion Sets the link-layer address for this interface.
 *       @param interface The interface the link layer address is being
 *               changed on.
 *       @param lladdr A pointer to the raw link layer address (pointer to
 *               the 6 byte ethernet address for ethernet).
 *       @param lladdr_len The length, in bytes, of the link layer address.
 */
extern errno_t ifnet_set_lladdr(ifnet_t interface, const void *lladdr,
    size_t lladdr_len);

/*!
 *       @function ifnet_lladdr_copy_bytes
 *       @discussion Copies the bytes of the link-layer address into the
 *               specified buffer.
 *       @param interface The interface to copy the link-layer address from.
 *       @param lladdr The buffer to copy the link-layer address in to.
 *       @param length The length of the buffer. This value must match the
 *               length of the link-layer address.
 */
extern errno_t ifnet_lladdr_copy_bytes(ifnet_t interface, void *lladdr,
    size_t length);

#ifdef KERNEL_PRIVATE
/*!
 *       @function ifnet_guarded_lladdr_copy_bytes
 *       @discussion Copies the bytes of the link-layer address into the
 *               specified buffer unless the current process is a sandboxed
 *               application without the net.link.addr system info privilege.
 *       @param interface The interface to copy the link-layer address from.
 *       @param lladdr The buffer to copy the link-layer address in to.
 *       @param length The length of the buffer. This value must match the
 *               length of the link-layer address.
 */
extern errno_t ifnet_guarded_lladdr_copy_bytes(ifnet_t interface, void *lladdr,
    size_t length);

/*!
 *       @function ifnet_lladdr
 *       @discussion Returns a pointer to the link-layer address.
 *       @param interface The interface the link-layer address is on.
 */
extern void *ifnet_lladdr(ifnet_t interface);
#endif /* KERNEL_PRIVATE */

/*!
 *       @function ifnet_llbroadcast_copy_bytes
 *       @discussion Retrieves the link-layer broadcast address for this
 *               interface.
 *       @param interface The interface.
 *       @param addr A buffer to copy the broadcast address in to.
 *       @param bufferlen The length of the buffer at addr.
 *       @param out_len On return, the length of the broadcast address.
 */
extern errno_t ifnet_llbroadcast_copy_bytes(ifnet_t interface, void *addr,
    size_t bufferlen, size_t *out_len);

#ifdef KERNEL_PRIVATE
/*!
 *       @function ifnet_set_lladdr_and_type
 *       @discussion Sets the link-layer address as well as the type field in
 *               the sockaddr_dl. Support for setting the type was added for vlan
 *               and bond interfaces.
 *       @param interface The interface the link layer address is being
 *               changed on.
 *       @param lladdr A pointer to the raw link layer address (pointer to
 *               the 6 byte ethernet address for ethernet).
 *       @param length The length, in bytes, of the link layer address.
 *       @param type The link-layer address type.
 */
extern errno_t ifnet_set_lladdr_and_type(ifnet_t interface, const void *lladdr,
    size_t length, u_char type);
#endif /* KERNEL_PRIVATE */

/*!
 *       @function ifnet_resolve_multicast
 *       @discussion Resolves a multicast address for an attached protocol to
 *               a link-layer address. If a link-layer address is passed in, the
 *               interface will verify that it is a valid multicast address.
 *       @param ifp The interface.
 *       @param proto_addr A protocol address to be converted to a link-layer
 *               address.
 *       @param ll_addr Storage for the resulting link-layer address.
 *       @param ll_len Length of the storage for the link-layer address.
 *       @result 0 on success. EOPNOTSUPP indicates the multicast address was
 *               not supported or could not be translated. Other errors may
 *               indicate other failures.
 */
extern errno_t ifnet_resolve_multicast(ifnet_t ifp,
    const struct sockaddr *proto_addr, struct sockaddr *ll_addr, size_t ll_len);

/*!
 *       @function ifnet_add_multicast
 *       @discussion Joins a multicast and returns an ifmultiaddr_t with the
 *               reference count incremented for you. You are responsible for
 *               decrementing the reference count after calling
 *               ifnet_remove_multicast and making sure you no longer have any
 *               references to the multicast.
 *       @param interface The interface.
 *       @param maddr The multicast address (AF_UNSPEC/AF_LINK) to join. Either
 *               a physical address or logical address to be translated to a
 *               physical address.
 *       @param multicast The resulting ifmultiaddr_t multicast address.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_add_multicast(ifnet_t interface,
    const struct sockaddr *maddr, ifmultiaddr_t *multicast);

/*!
 *       @function ifnet_remove_multicast
 *       @discussion Causes the interface to leave the multicast group. The
 *               stack keeps track of how many times ifnet_add_multicast has been
 *               called for a given multicast address. The multicast will only be
 *               removed when the number of times ifnet_remove_multicast has been
 *               called matches the number of times ifnet_add_multicast has been
 *               called.
 *
 *               The memory for the multicast address is not actually freed until
 *               the separate reference count has reached zero. Some parts of the
 *               stack may keep a pointer to the multicast even after that
 *               multicast has been removed from the interface.
 *
 *               When an interface is detached, all of the multicasts are
 *               removed. If the interface of the multicast passed in is no
 *               longer attached, this function will gracefully return,
 *               performing no work.
 *
 *               It is the callers responsibility to release the multicast
 *               address after calling this function.
 *       @param multicast The multicast to be removed.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_remove_multicast(ifmultiaddr_t multicast);

/*!
 *       @function ifnet_get_multicast_list
 *       @discussion Retrieves a list of multicast address the interface is
 *               set to receive. This function allocates and returns an array of
 *               references to the various multicast addresses. The multicasts
 *               have their reference counts bumped on your behalf. Calling
 *               ifnet_free_multicast_list will decrement the reference counts
 *               and free the array.
 *       @param interface The interface.
 *       @param addresses A pointer to a NULL terminated array of references
 *               to the multicast addresses.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_get_multicast_list(ifnet_t interface,
    ifmultiaddr_t **addresses);

/*!
 *       @function ifnet_free_multicast_list
 *       @discussion Frees a list of multicasts returned by
 *               ifnet_get_multicast_list. Decrements the refcount on each
 *               multicast address and frees the array.
 *       @param multicasts An array of references to the multicast addresses.
 */
extern void ifnet_free_multicast_list(ifmultiaddr_t *multicasts);

/*!
 *       @function ifnet_find_by_name
 *       @discussion Find an interface by the name including the unit number.
 *               Caller must call ifnet_release on any non-null interface return
 *               value.
 *       @param ifname The name of the interface, including any unit number
 *               (i.e. "en0").
 *       @param interface A pointer to an interface reference. This will be
 *               filled in if a matching interface is found.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_find_by_name(const char *ifname, ifnet_t *interface);

/*!
 *       @function ifnet_list_get
 *       @discussion Get a list of attached interfaces. List will be set to
 *               point to an array allocated by ifnet_list_get. The interfaces
 *               are refcounted and the counts will be incremented before the
 *               function returns. The list of interfaces must be freed using
 *               ifnet_list_free.
 *       @param family The interface family (i.e. IFNET_FAMILY_ETHERNET). To
 *               find interfaces of all families, use IFNET_FAMILY_ANY.
 *       @param interfaces A pointer to an array of interface references.
 *       @param count A pointer that will be filled in with the number of
 *               matching interfaces in the array.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_list_get(ifnet_family_t family, ifnet_t **interfaces,
    u_int32_t *count);

#ifdef KERNEL_PRIVATE
/*!
 *       @function ifnet_list_get_all
 *       @discussion Get a list of attached interfaces. List will be set to
 *               point to an array allocated by ifnet_list_get. The interfaces
 *               are refcounted and the counts will be incremented before the
 *               function returns. The list of interfaces must be freed using
 *               ifnet_list_free.  This is similar to ifnet_list_get, except
 *               that it includes interfaces that are detaching.
 *       @param family The interface family (i.e. IFNET_FAMILY_ETHERNET). To
 *               find interfaces of all families, use IFNET_FAMILY_ANY.
 *       @param interfaces A pointer to an array of interface references.
 *       @param count A pointer that will be filled in with the number of
 *               matching interfaces in the array.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_list_get_all(ifnet_family_t family, ifnet_t **interfaces,
    u_int32_t *count);
#endif /* KERNEL_PRIVATE */

/*!
 *       @function ifnet_list_free
 *       @discussion Free a list of interfaces returned by ifnet_list_get.
 *               Decrements the reference count on each interface and frees the
 *               array of references. If you keep a reference to an interface, be
 *               sure to increment the reference count before calling
 *               ifnet_list_free.
 *       @param interfaces An array of interface references from ifnet_list_get.
 */
extern void ifnet_list_free(ifnet_t *interfaces);

/******************************************************************************/
/* ifaddr_t accessors                                                         */
/******************************************************************************/

/*!
 *       @function ifaddr_reference
 *       @discussion Increment the reference count of an address tied to an
 *               interface.
 *       @param ifaddr The interface address.
 *       @result 0 upon success
 */
extern errno_t ifaddr_reference(ifaddr_t ifaddr);

/*!
 *       @function ifaddr_release
 *       @discussion Decrements the reference count of and possibly frees an
 *               address tied to an interface.
 *       @param ifaddr The interface address.
 *       @result 0 upon success
 */
extern errno_t ifaddr_release(ifaddr_t ifaddr);

/*!
 *       @function ifaddr_address
 *       @discussion Copies the address out of the ifaddr.
 *       @param ifaddr The interface address.
 *       @param out_addr The sockaddr storage for the address.
 *       @param addr_size The size of the storage for the address.
 *       @result 0 upon success
 */
extern errno_t ifaddr_address(ifaddr_t ifaddr, struct sockaddr *out_addr,
    u_int32_t addr_size);

/*!
 *       @function ifaddr_address
 *       @discussion Returns the address family of the address.
 *       @param ifaddr The interface address.
 *       @result 0 on failure, address family on success.
 */
extern sa_family_t ifaddr_address_family(ifaddr_t ifaddr);

/*!
 *       @function ifaddr_dstaddress
 *       @discussion Copies the destination address out of the ifaddr.
 *       @param ifaddr The interface address.
 *       @param out_dstaddr The sockaddr storage for the destination address.
 *       @param dstaddr_size The size of the storage for the destination address.
 *       @result 0 upon success
 */
extern errno_t ifaddr_dstaddress(ifaddr_t ifaddr, struct sockaddr *out_dstaddr,
    u_int32_t dstaddr_size);

/*!
 *       @function ifaddr_netmask
 *       @discussion Copies the netmask out of the ifaddr.
 *       @param ifaddr The interface address.
 *       @param out_netmask The sockaddr storage for the netmask.
 *       @param netmask_size The size of the storage for the netmask.
 *       @result 0 upon success
 */
extern errno_t ifaddr_netmask(ifaddr_t ifaddr, struct sockaddr *out_netmask,
    u_int32_t netmask_size);

/*!
 *       @function ifaddr_ifnet
 *       @discussion Returns the interface the address is attached to. The
 *               reference is only valid until the ifaddr is released. If you
 *               need to hold a reference to the ifnet for longer than you hold a
 *               reference to the ifaddr, increment the reference using
 *               ifnet_reference.
 *       @param ifaddr The interface address.
 *       @result A reference to the interface the address is attached to.
 */
extern ifnet_t ifaddr_ifnet(ifaddr_t ifaddr);

/*!
 *       @function ifaddr_withaddr
 *       @discussion Returns an interface address with the address specified.
 *               Increments the reference count on the ifaddr before returning to
 *               the caller. Caller is responsible for calling ifaddr_release.
 *       @param address The address to search for.
 *       @result A reference to the interface address.
 */
extern ifaddr_t ifaddr_withaddr(const struct sockaddr *address);

/*!
 *       @function ifaddr_withdstaddr
 *       @discussion Returns an interface address for the interface address
 *               that matches the destination when the netmask is applied.
 *               Increments the reference count on the ifaddr before returning to
 *               the caller. Caller is responsible for calling ifaddr_release.
 *       @param destination The destination to search for.
 *       @result A reference to the interface address.
 */
extern ifaddr_t ifaddr_withdstaddr(const struct sockaddr *destination);

/*!
 *       @function ifaddr_withnet
 *       @discussion Returns an interface address for the interface with the
 *               network described by net. Increments the reference count on the
 *               ifaddr before returning to the caller. Caller is responsible for
 *               calling ifaddr_release.
 *       @param net The network to search for.
 *       @result A reference to the interface address.
 */
extern ifaddr_t ifaddr_withnet(const struct sockaddr *net);

/*!
 *       @function ifaddr_withroute
 *       @discussion Returns an interface address given a destination and
 *               gateway. Increments the reference count on the ifaddr before
 *               returning to the caller. Caller is responsible for calling
 *               ifaddr_release.
 *       @param flags Routing flags. See net/route.h, RTF_GATEWAY etc.
 *       @param destination The destination to search for.
 *       @param gateway A gateway to search for.
 *       @result A reference to the interface address.
 */
extern ifaddr_t ifaddr_withroute(int flags, const struct sockaddr *destination,
    const struct sockaddr *gateway);

/*!
 *       @function ifaddr_findbestforaddr
 *       @discussion Finds the best local address assigned to a specific
 *               interface to use when communicating with another address.
 *               Increments the reference count on the ifaddr before returning to
 *               the caller. Caller is responsible for calling ifaddr_release.
 *       @param addr The remote address.
 *       @param interface The local interface.
 *       @result A reference to the interface address.
 */
extern ifaddr_t ifaddr_findbestforaddr(const struct sockaddr *addr,
    ifnet_t interface);

/******************************************************************************/
/* ifmultiaddr_t accessors                                                    */
/******************************************************************************/

/*!
 *       @function ifmaddr_reference
 *       @discussion Increment the reference count of an interface multicast
 *               address.
 *       @param ifmaddr The interface multicast address.
 *       @result 0 on success. Only error will be EINVAL if ifmaddr is not valid.
 */
extern errno_t ifmaddr_reference(ifmultiaddr_t ifmaddr);

/*!
 *       @function ifmaddr_release
 *       @discussion Decrement the reference count of an interface multicast
 *               address. If the reference count reaches zero, the ifmultiaddr
 *               will be removed from the interface and the ifmultiaddr will be
 *               freed.
 *       @param ifmaddr The interface multicast address.
 *       @result 0 on success. Only error will be EINVAL if ifmaddr is not valid.
 */
extern errno_t ifmaddr_release(ifmultiaddr_t ifmaddr);

/*!
 *       @function ifmaddr_address
 *       @discussion Copies the multicast address to out_multicast.
 *       @param out_multicast Storage for a sockaddr.
 *       @param addr_size Size of the storage.
 *       @result 0 on success.
 */
extern errno_t ifmaddr_address(ifmultiaddr_t ifmaddr,
    struct sockaddr *out_multicast, u_int32_t addr_size);

/*!
 *       @function ifmaddr_lladdress
 *       @discussion Copies the link layer multicast address to
 *               out_link_layer_multicast.
 *       @param out_link_layer_multicast Storage for a sockaddr.
 *       @param addr_size Size of the storage.
 *       @result 0 on success.
 */
extern errno_t ifmaddr_lladdress(ifmultiaddr_t ifmaddr,
    struct sockaddr *out_link_layer_multicast, u_int32_t addr_size);

/*!
 *       @function ifmaddr_ifnet
 *       @discussion Returns the interface this multicast address is attached
 *               to. The interface reference count is not bumped by this
 *               function. The interface is only valid as long as you don't
 *               release the refernece to the multiast address. If you need to
 *               maintain your pointer to the ifnet, call ifnet_reference
 *               followed by ifnet_release when you're finished.
 *       @param ifmaddr The interface multicast address.
 *       @result A reference to the interface.
 */
extern ifnet_t ifmaddr_ifnet(ifmultiaddr_t ifmaddr);

#ifdef KERNEL_PRIVATE
/******************************************************************************/
/* interface cloner                                                           */
/******************************************************************************/

/*
 *       @typedef ifnet_clone_create_func
 *       @discussion ifnet_clone_create_func is called to create an interface.
 *       @param ifcloner The interface cloner.
 *       @param unit The interface unit number to create.
 *       @param params Additional information specific to the interface cloner.
 *       @result Return zero on success or an errno error value on failure.
 */
typedef errno_t (*ifnet_clone_create_func)(if_clone_t ifcloner, u_int32_t unit, void *params);

/*
 *       @typedef ifnet_clone_destroy_func
 *       @discussion ifnet_clone_create_func is called to destroy an interface created
 *               by an interface cloner.
 *       @param interface The interface to destroy.
 *       @result Return zero on success or an errno error value on failure.
 */
typedef errno_t (*ifnet_clone_destroy_func)(ifnet_t interface);

/*
 *       @struct ifnet_clone_params
 *       @discussion This structure is used to represent an interface cloner.
 *       @field ifc_name The interface name handled by this interface cloner.
 *       @field ifc_create The function to create an interface.
 *       @field ifc_destroy The function to destroy an interface.
 */
struct ifnet_clone_params {
	const char                      *ifc_name;
	ifnet_clone_create_func         ifc_create;
	ifnet_clone_destroy_func        ifc_destroy;
};

/*
 *       @function ifnet_clone_attach
 *       @discussion Attaches a new interface cloner.
 *       @param cloner_params The structure that defines an interface cloner.
 *       @param interface A pointer to an opaque handle that represent the interface cloner
 *               that is attached upon success.
 *       @result Returns 0 on success.
 *               May return ENOBUFS if there is insufficient memory.
 *               May return EEXIST if an interface cloner with the same name is already attached.
 */
extern errno_t ifnet_clone_attach(struct ifnet_clone_params *cloner_params, if_clone_t *ifcloner);

/*
 *       @function ifnet_clone_detach
 *       @discussion Detaches a previously attached interface cloner.
 *       @param ifcloner The opaque handle returned when the interface cloner was attached.
 *       @result Returns 0 on success.
 */
extern errno_t ifnet_clone_detach(if_clone_t ifcloner);

/******************************************************************************/
/* misc                                                                       */
/******************************************************************************/

/*
 *       @function ifnet_get_local_ports
 *       @discussion Returns a bitfield indicating which ports of PF_INET
 *               and PF_INET6 protocol families have sockets in the usable
 *               state.  An interface that supports waking the host on unicast
 *               traffic may use this information to discard incoming unicast
 *               packets that don't have a corresponding bit set instead of
 *               waking up the host. For port 0x0001, bit 1 of the first byte
 *               would be set. For port n, bit 1 << (n % 8) of the (n / 8)'th
 *               byte would be set.
 *       @param ifp The interface in question.  May be NULL, which means
 *               all interfaces.
 *       @param bitfield A pointer to 8192 bytes.
 *       @result Returns 0 on success.
 */
extern errno_t ifnet_get_local_ports(ifnet_t ifp, u_int8_t *bitfield);

#define IFNET_GET_LOCAL_PORTS_WILDCARDOK        0x01
#define IFNET_GET_LOCAL_PORTS_NOWAKEUPOK        0x02
#define IFNET_GET_LOCAL_PORTS_TCPONLY           0x04
#define IFNET_GET_LOCAL_PORTS_UDPONLY           0x08
#define IFNET_GET_LOCAL_PORTS_RECVANYIFONLY     0x10
#define IFNET_GET_LOCAL_PORTS_EXTBGIDLEONLY     0x20
#define IFNET_GET_LOCAL_PORTS_ACTIVEONLY        0x40
#define IFNET_GET_LOCAL_PORTS_ANYTCPSTATEOK     0x80
/*
 *       @function ifnet_get_local_ports_extended
 *       @discussion Returns a bitfield indicating which local ports of the
 *               specified protocol have sockets in the usable state.  An
 *               interface that supports waking the host on unicast traffic may
 *               use this information to discard incoming unicast packets that
 *               don't have a corresponding bit set instead of waking up the
 *               host.  For port 0x0001, bit 1 of the first byte would be set.
 *               For port n, bit 1 << (n % 8) of the (n / 8)'th byte would be
 *               set.
 *       @param ifp The interface in question.  May be NULL, which means
 *               all interfaces.
 *       @param protocol The protocol family of the sockets.  PF_UNSPEC (0)
 *               means all protocols, otherwise PF_INET or PF_INET6.
 *       @param flags A bitwise of the following flags:
 *               IFNET_GET_LOCAL_PORTS_WILDCARDOK: When bit is set,
 *               the list of local ports should include those that are
 *               used by sockets that aren't bound to any local address.
 *               IFNET_GET_LOCAL_PORTS_NOWAKEUPOK: When bit is
 *               set, the list of local ports should return all sockets
 *               including the ones that do not need a wakeup from sleep.
 *               Sockets that do not want to wake from sleep are marked
 *               with a socket option.
 *               IFNET_GET_LOCAL_PORTS_TCPONLY: When bit is set, the list
 *               of local ports should return the ports used by TCP sockets.
 *               IFNET_GET_LOCAL_PORTS_UDPONLY: When bit is set, the list
 *               of local ports should return the ports used by UDP sockets.
 *               only.
 *               IFNET_GET_LOCAL_PORTS_RECVANYIFONLY: When bit is set, the
 *               port is in the list only if the socket has the option
 *               SO_RECV_ANYIF set
 *               IFNET_GET_LOCAL_PORTS_EXTBGIDLEONLY: When bit is set, the
 *               port is in the list only if the socket has the option
 *               SO_EXTENDED_BK_IDLE set
 *               IFNET_GET_LOCAL_PORTS_ACTIVEONLY: When bit is set, the
 *               port is in the list only if the socket is not in a final TCP
 *               state or the connection is not idle in a final TCP state
 *               IFNET_GET_LOCAL_PORTS_ANYTCPSTATEOK: When bit is set, the
 *               port is in the list for all the TCP states except CLOSED
 *               and TIME_WAIT
 *       @param bitfield A pointer to 8192 bytes.
 *       @result Returns 0 on success.
 */
extern errno_t ifnet_get_local_ports_extended(ifnet_t ifp,
    protocol_family_t protocol, u_int32_t flags, u_int8_t *bitfield);

/******************************************************************************/
/* for reporting issues							      */
/******************************************************************************/

#define IFNET_MODIDLEN  20
#define IFNET_MODARGLEN 12

/*
 *       @function ifnet_report_issues
 *       @discussion Provided for network interface families and drivers to
 *               notify the system of issues detected at their layers.
 *       @param ifp The interface experiencing issues.
 *       @param modid The ID of the module reporting issues.  It may contain
 *               any value that is unique enough to identify the module, such
 *               as the SHA-1 hash of the bundle ID of the module, e.g.
 *               "com.apple.iokit.IONetworkingFamily" or
 *               "com.apple.iokit.IO80211Family".
 *       @param info An optional, fixed-size array of octets containing opaque
 *               information data used specific to the module/layer reporting
 *               the issues.  May be NULL.
 *       @result Returns 0 on success, or EINVAL if arguments are invalid.
 */
extern errno_t ifnet_report_issues(ifnet_t ifp, u_int8_t modid[IFNET_MODIDLEN],
    u_int8_t info[IFNET_MODARGLEN]);

/******************************************************************************/
/* for interfaces that support link level transmit completion status          */
/******************************************************************************/
/*
 *       @enum  Per packet phy level transmit completion status values
 *       @abstract Constants defining possible completion status values
 *       A driver may support all or some of these values
 *       @discussion
 *       @constant IFNET_TX_COMPL_SUCCESS  link transmission succeeded
 *       @constant IFNET_TX_COMPL_FAIL	  link transmission failed
 *       @constant IFNET_TX_COMPL_ABORTED  link transmission aborted, may retry
 *       @constant IFNET_TX_QUEUE_FULL	  link level secondary queue full
 */
enum {
	IFNET_TX_COMPL_SUCCESS          = 0,    /* sent on link */
	IFNET_TX_COMPL_FAIL             = 1,    /* failed to send on link */
	IFNET_TX_COMPL_ABORTED          = 2,    /* aborted send, peer asleep */
	IFNET_TX_COMPL_QFULL            = 3     /* driver level queue full */
};

typedef u_int32_t       tx_compl_val_t;

/*
 *       @function ifnet_tx_compl_status
 *       @discussion Used as an upcall from IONetwork Family to stack that
 *               indicates the link level completion status of a transmitted
 *               packet.
 *       @param ifp The interface to which the mbuf was sent
 *       @param m The mbuf that was transmitted
 *       @param val indicates the status of the transmission
 */
extern errno_t ifnet_tx_compl_status(ifnet_t ifp, mbuf_t m, tx_compl_val_t val);

/*
 *       @function ifnet_tx_compl
 *       @discussion Used to indicates the packet has been transmitted.
 *       @param ifp The interface to which the mbuf was sent
 *       @param m The mbuf that was transmitted
 */
extern errno_t ifnet_tx_compl(ifnet_t ifp, mbuf_t m);

/******************************************************************************/
/* for interfaces that support dynamic node absence/presence events           */
/******************************************************************************/

/*
 *       @function ifnet_notice_node_presence
 *       @discussion Provided for network interface drivers to notify the
 *               system of a change detected in the presence of the specified
 *               node.
 *       @param ifp The interface attached to the link where the specified node
 *               is present.
 *       @param sa The AF_LINK family address of the node whose presence is
 *               changing.
 *       @param rssi The received signal strength indication as measured in
 *               dBm by a radio receiver.
 *       @param lqm A link quality metric associated with the specified node.
 *       @param npm A node proximity metric associated with the specified node.
 *       @param srvinfo A fixed-size array of octets containing opaque service
 *               information data used by the mDNS responder subsystem.
 *       @result Returns 0 on success, or EINVAL if arguments are invalid.
 */
extern errno_t
ifnet_notice_node_presence(ifnet_t ifp, struct sockaddr *sa, int32_t rssi,
    int lqm, int npm, u_int8_t srvinfo[48]);

/*
 *       @function ifnet_notice_node_absence
 *       @discussion Provided for network interface drivers to notify the
 *               system that the absence of the specified node has been detected.
 *       @param ifp The interface attached to the link where the absence of the
 *               specified node has been detected.
 *       @param sa The AF_INET6 or AF_LINK family address of the node whose absence has been
 *               detected. If AF_LINK is specified, AF_INET6 address is derived from the
 *               AF_LINK address.
 *       @result Returns 0 on success, or EINVAL if arguments are invalid.
 */
extern errno_t ifnet_notice_node_absence(ifnet_t ifp, struct sockaddr *sa);

/*
 *       @function ifnet_notice_node_presence_v2
 *       @discussion Provided for network interface drivers to notify the
 *               system of a change detected in the presence of the specified
 *               node.
 *       @param ifp The interface attached to the link where the specified node
 *               is present.
 *       @param sa The AF_INET6 family address of the node whose presence is
 *               changing.
 *       @param sdl The AF_LINK family address of the node whose presence is
 *               changing.
 *       @param rssi The received signal strength indication as measured in
 *               dBm by a radio receiver.
 *       @param lqm A link quality metric associated with the specified node.
 *       @param npm A node proximity metric associated with the specified node.
 *       @param srvinfo A fixed-size array of octets containing opaque service
 *               information data used by the mDNS responder subsystem.
 *       @result Returns 0 on success, or EINVAL if arguments are invalid.
 */
extern errno_t
ifnet_notice_node_presence_v2(ifnet_t ifp, struct sockaddr *sa, struct sockaddr_dl *sdl, int32_t rssi,
    int lqm, int npm, u_int8_t srvinfo[48]);

/*
 *       @function ifnet_notice_master_elected
 *       @discussion Provided for network interface drivers to notify the system
 *               that the nodes with a locally detected presence on the attached
 *               link have elected a new master.
 *       @param ifp The interface attached to the link where the new master has
 *               been elected.
 *       @result Returns 0 on success, or EINVAL if arguments are invalid.
 */
extern errno_t ifnet_notice_master_elected(ifnet_t ifp);

/******************************************************************************/
/* for interface delegation						      */
/******************************************************************************/

/*
 *       @function ifnet_set_delegate
 *       @discussion Indicate that an interface is delegating another interface
 *               for accounting/restriction purposes.  This could be used by a
 *               virtual interface that is going over another interface, where
 *               the virtual interface is to be treated as if it's the underlying
 *               interface for certain operations/checks.
 *       @param ifp The delegating interface.
 *       @param delegated_ifp The delegated interface.  If NULL or equal to
 *               the delegating interface itself, any previously-established
 *               delegation is removed.  If non-NULL, a reference to the
 *               delegated interface is held by the delegating interface;
 *               this reference is released via a subsequent call to remove
 *               the established association, or when the delegating interface
 *               is detached.
 *       @param Returns 0 on success, EINVAL if arguments are invalid, or
 *               ENXIO if the delegating interface isn't currently attached.
 */
extern errno_t
ifnet_set_delegate(ifnet_t ifp, ifnet_t delegated_ifp);

/*
 *       @function ifnet_get_delegate
 *       @discussion Retrieve delegated interface information on an interface.
 *       @param ifp The delegating interface.
 *       @param pdelegated_ifp Pointer to the delegated interface.  Upon
 *               success, this will contain the delegated interface or
 *               NULL if there is no delegation in place.  If non-NULL,
 *               the delegated interface will be returned with a reference
 *               held for caller, and the caller is responsible for releasing
 *               it via ifnet_release();
 *       @param Returns 0 on success, EINVAL if arguments are invalid, or
 *               ENXIO if the delegating interface isn't currently attached.
 */
extern errno_t
ifnet_get_delegate(ifnet_t ifp, ifnet_t *pdelegated_ifp);

/*************************************************************************/
/* for interface keep alive offload support                              */
/*************************************************************************/

/*
 *       @struct ifnet_keepalive_offload_frame
 *       @discussion This structure is used to define various opportunistic
 *               polling parameters for an interface.
 *               For IPsec and AirPlay UDP keep alive only a subset of the
 *               fields are relevant.
 *               An incoming TCP keep alive probe has the sequence number
 *               in the TCP header equal to "remote_seq" and the
 *               acknowledgment number field is equal to "local_seq".
 *               An incoming TCP keep alive probe has the sequence number
 *               equlal to "remote_seq" minus 1 and the acknowledgment number
 *               field is equal to "local_seq".
 *               Note that remote_seq is in network byte order so the value to
 *               match may have to be converted to host byte order when
 *               subtracting 1.
 *               For TCP, the field "interval" corresponds to the socket option
 *               TCP_KEEPALIVE, the field "keep_cnt" to TCP_KEEPINTVL and
 *               the field "keep_cnt" to TCP_KEEPCNT.
 *       @field data Keep alive probe to be sent.
 *       @field type The type of keep alive frame
 *       @field length The length of the frame in the data field
 *       @field interval Keep alive interval between probes in seconds
 *       @field ether_type Tell if it's the protocol is IPv4 or IPv6
 *       @field keep_cnt Maximum number of time to retry probes (TCP only)
 *       @field keep_retry Interval before retrying if previous probe was not answered (TCP only)
 *       @field reply_length The length of the frame in the reply_data field (TCP only)
 *       @field addr_length Length in bytes of local_addr and remote_addr (TCP only)
 *       @field flags Flags (TCP only)
 *       @field reply_data Keep alive reply to be sent to incoming probe (TCP only)
 *       @field local_addr Local address: 4 bytes IPv4 or 16 bytes IPv6 address (TCP only)
 *       @field remote_addr Remote address: 4 bytes IPv4 or 16 bytes IPv6 address (TCP only)
 *       @field local_port Local port (TCP only)
 *       @field remote_port Remote port (TCP only)
 *       @field local_seq Local sequence number for matching incoming replies (TCP only)
 *       @field remote_seq Remote sequence number for matching incoming probes or replies (TCP only)
 */

#define IFNET_KEEPALIVE_OFFLOAD_FRAME_DATA_SIZE 128
#define IFNET_KEEPALIVE_OFFLOAD_MAX_ADDR_SIZE 16

struct ifnet_keepalive_offload_frame {
	u_int8_t data[IFNET_KEEPALIVE_OFFLOAD_FRAME_DATA_SIZE]; /* data bytes */
#define IFNET_KEEPALIVE_OFFLOAD_FRAME_IPSEC 0x0
#define IFNET_KEEPALIVE_OFFLOAD_FRAME_AIRPLAY 0x1
#define IFNET_KEEPALIVE_OFFLOAD_FRAME_TCP 0x2
	u_int8_t type;  /* type of application */
	u_int8_t length; /* Number of valid data bytes including offset */
	u_int16_t interval; /* Keep alive interval in seconds */
#define IFNET_KEEPALIVE_OFFLOAD_FRAME_ETHERTYPE_IPV4    0x0
#define IFNET_KEEPALIVE_OFFLOAD_FRAME_ETHERTYPE_IPV6    0x1
	u_int8_t ether_type; /* Ether type IPv4 or IPv6 */
	u_int8_t keep_cnt; /* max number of time to retry probes */
	u_int16_t keep_retry; /* interval before retrying if previous probe was not answered */
	u_int8_t reply_length; /* Length of valid reply_data bytes including offset */
	u_int8_t addr_length; /* Length of valid bytes in local_addr and remote_addr */
#define  IFNET_KEEPALIVE_OFFLOAD_FLAG_NOWAKEFROMSLEEP   0x01
	u_int8_t flags;
	u_int8_t reserved[1];
	u_int8_t reply_data[IFNET_KEEPALIVE_OFFLOAD_FRAME_DATA_SIZE]; /* Response packet */
	u_int8_t local_addr[IFNET_KEEPALIVE_OFFLOAD_MAX_ADDR_SIZE]; /* in network byte order  */
	u_int8_t remote_addr[IFNET_KEEPALIVE_OFFLOAD_MAX_ADDR_SIZE]; /* in network byte order  */
	u_int16_t local_port; /* in host byte order */
	u_int16_t remote_port; /* in host byte order */
	u_int32_t local_seq; /* in host byte order */
	u_int32_t remote_seq; /* in host byte order */
};

/*
 *       @function ifnet_get_keepalive_offload_frames
 *       @discussion Fills out frames_array with IP packets to send at
 *               periodic intervals as Keep-alive or heartbeat messages.
 *               This can be used to offload keep alives for UDP or TCP.
 *               Note: The frames are returned in this order: first the IPsec
 *               frames, then the AirPlay frames and finally the TCP frames.
 *               If a device does not support one kind of keep alive frames_array
 *               it should provide a frames_array large enough to accomodate
 *               the other frames
 *       @param ifp The interface to send frames out on. This is used to
 *               select which sockets or IPsec SAs should generate the
 *               packets.
 *       @param frames_array An array of ifnet_keepalive_offload_frame
 *               structs. This is allocated by the caller, and has
 *               frames_array_count frames of valid memory.
 *       @param frames_array_count The number of valid frames allocated
 *               by the caller in frames_array
 *       @param frame_data_offset The offset in bytes into each frame data
 *               at which the IPv4/IPv6 packet and payload should be written
 *       @param used_frames_count The returned number of frames that were
 *               filled out with valid information.
 *       @result Returns 0 on success, error number otherwise.
 */
extern errno_t ifnet_get_keepalive_offload_frames(ifnet_t ifp,
    struct ifnet_keepalive_offload_frame *frames_array,
    u_int32_t frames_array_count, size_t frame_data_offset,
    u_int32_t *used_frames_count);


/*
 *       @function ifnet_notify_tcp_keepalive_offload_timeout
 *       @discussion Used by an interface to notify a TCP connection whose
 *               keep alive was offloaded did experience a timeout.
 *       @param ifp The interface for which the TCP keep alive offload timed out
 *       @param frame The ifnet_keepalive_offload_frame structure that identifies
 *               the TCP connection that experienced the timeout.
 *               All the fields must be zeroed by the caller except for:
 *               - type: must be IFNET_KEEPALIVE_OFFLOAD_FRAME_TCP
 *               and for the fields identifying the 5-tup;e of the
 *               TCP connection:
 *               - ether_type
 *               - local_addr
 *               - remote_addr
 *               - local_port
 *               - remote_port
 *       @result Returns 0 on success, error number otherwise.
 */
extern errno_t ifnet_notify_tcp_keepalive_offload_timeout(ifnet_t ifp,
    struct ifnet_keepalive_offload_frame *frame);

/*************************************************************************/
/* Link level notifications                                              */
/*************************************************************************/
/*
 *       @function ifnet_link_status_report
 *       @discussion A KPI to let the driver provide link specific
 *               status information to the protocol stack. The KPI will
 *               copy contents from the buffer based on the version and
 *               length provided by the driver. The contents of the buffer
 *               will be read but will not be modified.
 *       @param ifp The interface that is generating the report
 *       @param buffer Buffer containing the link specific information
 *               for this interface. It is the caller's responsibility
 *               to free this buffer.
 *       @param buffer_len Valid length of the buffer provided by the caller
 *       @result Returns 0 on success, error number otherwise.
 */
extern errno_t ifnet_link_status_report(ifnet_t ifp, const void *buffer,
    size_t buffer_len);

/*************************************************************************/
/* QoS Fastlane                                                          */
/*************************************************************************/
/*!
 *       @function ifnet_set_fastlane_capable
 *       @param interface The interface.
 *       @param capable Set the truth value that the interface is attached to
 *               a network that is capable of Fastlane QoS marking.
 *       @result Returns 0 on success, error number otherwise.
 */
extern errno_t ifnet_set_fastlane_capable(ifnet_t interface, boolean_t capable);

/*!
 *       @function ifnet_get_fastlane_capable
 *       @param interface The interface.
 *       @param capable On output contains the truth value that the interface
 *               is attached ta network that is capable of Fastlane QoS marking.
 *       @result Returns 0 on success, error number otherwise.
 */
extern errno_t ifnet_get_fastlane_capable(ifnet_t interface, boolean_t *capable);

/*!
 *       @function ifnet_get_unsent_bytes
 *       @param interface The interface
 *       @param unsent_bytes An out parameter that contains unsent bytes for
 *               an interface
 *       @result Returns 0 on success, error otherwise.
 */
extern errno_t ifnet_get_unsent_bytes(ifnet_t interface, int64_t *unsent_bytes);

typedef struct {
	int32_t buf_interface; /* data to send at interface */
	int32_t buf_sndbuf; /* data to send at socket buffer */
} ifnet_buffer_status_t;

/*!
 *       @function ifnet_get_buffer_status
 *       @param interface The interface
 *       @param buf_status An out parameter that contains unsent bytes
 *               for an interface
 *       @result Returns 0 on success, EINVAL if any of the arguments is
 *               NULL, ENXIO if the interface pointer is invalid
 */
extern errno_t ifnet_get_buffer_status(const ifnet_t interface,
    ifnet_buffer_status_t *buf_status);

/*!
 *       @function ifnet_normalise_unsent_data
 *       @discussion
 *               Gathers the unsent bytes on all the interfaces.
 *               This data will be reported to NetworkStatistics.
 *
 */
extern void ifnet_normalise_unsent_data(void);

/*************************************************************************/
/* Low Power Mode                                                        */
/*************************************************************************/

/*!
 *       @function ifnet_set_low_power_mode
 *       @param interface The interface.
 *       @param on Set the truth value that the interface is in low power mode.
 *       @result Returns 0 on success, error number otherwise.
 */
extern errno_t ifnet_set_low_power_mode(ifnet_t interface, boolean_t on);

/*!
 *       @function ifnet_get_low_power_mode
 *       @param interface The interface.
 *       @param on On output contains the truth value that the interface
 *               is in low power mode.
 *       @result Returns 0 on success, error number otherwise.
 */
extern errno_t ifnet_get_low_power_mode(ifnet_t interface, boolean_t *on);

/*!
 *  @function ifnet_touch_lastupdown
 *  @discussion Updates the lastupdown value to now.
 *  @param interface The interface.
 *  @result 0 on success otherwise the errno error.
 */
extern errno_t ifnet_touch_lastupdown(ifnet_t interface);

/*!
 *  @function ifnet_updown_delta
 *  @discussion Retrieves the difference between lastupdown and now.
 *  @param interface The interface.
 *  @param updown_delta A timeval struct to copy the delta between lastupdown and now.
 *  to.
 */
extern errno_t ifnet_updown_delta(ifnet_t interface, struct timeval *updown_delta);

/*************************************************************************/
/* Interface advisory notifications                                      */
/*************************************************************************/
/*!
 *       @function ifnet_interface_advisory_report
 *       @discussion KPI to let the driver provide interface advisory
 *       notifications to the user space.
 *       @param ifp The interface that is generating the advisory report.
 *       @param advisory structure containing the advisory notification
 *              information.
 *       @result Returns 0 on success, error number otherwise.
 */
extern errno_t ifnet_interface_advisory_report(ifnet_t ifp,
    const struct ifnet_interface_advisory *advisory);

#endif /* KERNEL_PRIVATE */

__END_DECLS

#endif /* __KPI_INTERFACE__ */

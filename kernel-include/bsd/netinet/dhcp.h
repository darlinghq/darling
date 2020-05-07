#ifndef _NETINET_DHCP_H
#define _NETINET_DHCP_H
#include <sys/appleapiopts.h>

/*
 * Copyright (c) 1999-2007 Apple Inc. All rights reserved.
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
/*
 * dhcp.h
 * - definitions for DHCP (as specified in RFC2132)
 */
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

struct dhcp {
	u_char              dp_op;      /* packet opcode type */
	u_char              dp_htype;   /* hardware addr type */
	u_char              dp_hlen;    /* hardware addr length */
	u_char              dp_hops;    /* gateway hops */
	u_int32_t           dp_xid;     /* transaction ID */
	u_int16_t           dp_secs;    /* seconds since boot began */
	u_int16_t           dp_flags;   /* flags */
	struct in_addr      dp_ciaddr;  /* client IP address */
	struct in_addr      dp_yiaddr;  /* 'your' IP address */
	struct in_addr      dp_siaddr;  /* server IP address */
	struct in_addr      dp_giaddr;  /* gateway IP address */
	u_char              dp_chaddr[16];/* client hardware address */
	u_char              dp_sname[64];/* server host name */
	u_char              dp_file[128];/* boot file name */
	u_char              dp_options[0];/* variable-length options field */
};

struct dhcp_packet {
	struct ip           ip;
	struct udphdr       udp;
	struct dhcp         dhcp;
};

#define DHCP_OPTIONS_MIN        312
#define DHCP_PACKET_MIN         (sizeof(struct dhcp_packet) + DHCP_OPTIONS_MIN)
#define DHCP_PAYLOAD_MIN        (sizeof(struct dhcp) + DHCP_OPTIONS_MIN)

/* dhcp message types */
#define DHCPDISCOVER    1
#define DHCPOFFER       2
#define DHCPREQUEST     3
#define DHCPDECLINE     4
#define DHCPACK         5
#define DHCPNAK         6
#define DHCPRELEASE     7
#define DHCPINFORM      8

enum {
	dhcp_msgtype_none_e         = 0,
	dhcp_msgtype_discover_e     = DHCPDISCOVER,
	dhcp_msgtype_offer_e        = DHCPOFFER,
	dhcp_msgtype_request_e      = DHCPREQUEST,
	dhcp_msgtype_decline_e      = DHCPDECLINE,
	dhcp_msgtype_ack_e          = DHCPACK,
	dhcp_msgtype_nak_e          = DHCPNAK,
	dhcp_msgtype_release_e      = DHCPRELEASE,
	dhcp_msgtype_inform_e       = DHCPINFORM,
};

typedef uint8_t dhcp_msgtype_t;

typedef int32_t                 dhcp_time_secs_t; /* absolute time */
typedef int32_t                 dhcp_lease_t;     /* relative time */
#define dhcp_time_hton          htonl
#define dhcp_time_ntoh          ntohl
#define dhcp_lease_hton         htonl
#define dhcp_lease_ntoh         ntohl

#define DHCP_INFINITE_LEASE     ((dhcp_lease_t)-1)
#define DHCP_INFINITE_TIME      ((dhcp_time_secs_t)-1)

#define DHCP_FLAGS_BROADCAST    ((u_short)0x0001)

#endif /* _NETINET_DHCP_H */

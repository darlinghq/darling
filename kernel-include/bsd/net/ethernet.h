/*
 * Copyright (c) 2000-2017 Apple Inc. All rights reserved.
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
 * Fundamental constants relating to ethernet.
 *
 */

#ifndef _NET_ETHERNET_H_
#define _NET_ETHERNET_H_
#include <sys/appleapiopts.h>
#include <sys/types.h>          /* u_ types */

/*
 * The number of bytes in an ethernet (MAC) address.
 */
#define ETHER_ADDR_LEN          6

/*
 * The number of bytes in the type field.
 */
#define ETHER_TYPE_LEN          2

/*
 * The number of bytes in the trailing CRC field.
 */
#define ETHER_CRC_LEN           4

/*
 * The length of the combined header.
 */
#define ETHER_HDR_LEN           (ETHER_ADDR_LEN*2+ETHER_TYPE_LEN)

/*
 * The minimum packet length.
 */
#define ETHER_MIN_LEN           64

/*
 * The maximum packet length.
 */
#define ETHER_MAX_LEN           1518

/*
 * Mbuf adjust factor to force 32-bit alignment of IP header.
 * Drivers should do m_adj(m, ETHER_ALIGN) when setting up a
 * receive so the upper layers get the IP header properly aligned
 * past the 14-byte Ethernet header.
 */
#define ETHER_ALIGN             2       /* driver adjust for IP hdr alignment */

/*
 * A macro to validate a length with
 */
#define ETHER_IS_VALID_LEN(foo) \
	((foo) >= ETHER_MIN_LEN && (foo) <= ETHER_MAX_LEN)

/*
 * Structure of a 10Mb/s Ethernet header.
 */
typedef struct  ether_header {
	u_char  ether_dhost[ETHER_ADDR_LEN];
	u_char  ether_shost[ETHER_ADDR_LEN];
	u_short ether_type;
} ether_header_t;

/*
 * Structure of a 48-bit Ethernet address.
 */
typedef struct  ether_addr {
	u_char octet[ETHER_ADDR_LEN];
} ether_addr_t;

#define ether_addr_octet octet

#define ETHERTYPE_PUP           0x0200  /* PUP protocol */
#define ETHERTYPE_IP            0x0800  /* IP protocol */
#define ETHERTYPE_ARP           0x0806  /* Addr. resolution protocol */
#define ETHERTYPE_REVARP        0x8035  /* reverse Addr. resolution protocol */
#define ETHERTYPE_VLAN          0x8100  /* IEEE 802.1Q VLAN tagging */
#define ETHERTYPE_IPV6          0x86dd  /* IPv6 */
#define ETHERTYPE_PAE           0x888e  /* EAPOL PAE/802.1x */
#define ETHERTYPE_RSN_PREAUTH   0x88c7  /* 802.11i / RSN Pre-Authentication */
#define ETHERTYPE_PTP           0x88f7  /* IEEE 1588 Precision Time Protocol */
#define ETHERTYPE_LOOPBACK      0x9000  /* used to test interfaces */
/* XXX - add more useful types here */
#define ETHERTYPE_IEEE802154    0x0809  /* 802.15.4 */

/*
 * The ETHERTYPE_NTRAILER packet types starting at ETHERTYPE_TRAIL have
 * (type-ETHERTYPE_TRAIL)*512 bytes of data followed
 * by an ETHER type (as given above) and then the (variable-length) header.
 */
#define ETHERTYPE_TRAIL         0x1000          /* Trailer packet */
#define ETHERTYPE_NTRAILER      16

#define ETHERMTU        (ETHER_MAX_LEN-ETHER_HDR_LEN-ETHER_CRC_LEN)
#define ETHERMIN        (ETHER_MIN_LEN-ETHER_HDR_LEN-ETHER_CRC_LEN)

#ifdef KERNEL_PRIVATE
/*
 * The following are used by ethernet interfaces.
 */

struct  ether_addr *ether_aton(const char *);

#ifdef BSD_KERNEL_PRIVATE
extern u_char   etherbroadcastaddr[ETHER_ADDR_LEN];

#if defined (__arm__)

#include <string.h>

static __inline__ int
_ether_cmp(const void * a, const void * b)
{
	return memcmp(a, b, ETHER_ADDR_LEN);
}

#else /* __arm__ */

static __inline__ int
_ether_cmp(const void * a, const void * b)
{
	const u_int16_t * a_s = (const u_int16_t *)a;
	const u_int16_t * b_s = (const u_int16_t *)b;

	if (a_s[0] != b_s[0]
	    || a_s[1] != b_s[1]
	    || a_s[2] != b_s[2]) {
		return 1;
	}
	return 0;
}

#endif /* __arm__ */
#endif /* BSD_KERNEL_PRIVATE */

#define ETHER_IS_MULTICAST(addr) (*(addr) & 0x01) /* is address mcast/bcast? */

#endif /* KERNEL_PRIVATE */

#ifndef KERNEL
#include <sys/cdefs.h>

/*
 * Ethernet address conversion/parsing routines.
 */
__BEGIN_DECLS

int     ether_hostton(const char *, struct ether_addr *);
int     ether_line(const char *, struct ether_addr *, char *);
char    *ether_ntoa(const struct ether_addr *);
struct  ether_addr *ether_aton(const char *);
int     ether_ntohost(char *, const struct ether_addr *);
__END_DECLS
#endif /* !KERNEL */

#endif /* !_NET_ETHERNET_H_ */

/*
 * Copyright (c) 2000-2016 Apple Inc. All rights reserved.
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
 * Copyright (c) 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)if_arp.h	8.1 (Berkeley) 6/10/93
 * $FreeBSD: src/sys/net/if_arp.h,v 1.14.2.1 2000/07/11 20:46:55 archie Exp $
 */

#ifndef _NET_IF_ARP_H_
#define _NET_IF_ARP_H_
#include <stdint.h>
#include <sys/appleapiopts.h>
#include <netinet/in.h>

/*
 * Address Resolution Protocol.
 *
 * See RFC 826 for protocol description.  ARP packets are variable
 * in size; the arphdr structure defines the fixed-length portion.
 * Protocol type values are the same as those for 10 Mb/s Ethernet.
 * It is followed by the variable-sized fields ar_sha, arp_spa,
 * arp_tha and arp_tpa in that order, according to the lengths
 * specified.  Field names used correspond to RFC 826.
 */
struct  arphdr {
	u_short ar_hrd;         /* format of hardware address */
#define ARPHRD_ETHER    1       /* ethernet hardware format */
#define ARPHRD_IEEE802  6       /* token-ring hardware format */
#define ARPHRD_FRELAY   15      /* frame relay hardware format */
#define ARPHRD_IEEE1394 24      /* IEEE1394 hardware address */
#define ARPHRD_IEEE1394_EUI64 27 /* IEEE1394 EUI-64 */
	u_short ar_pro;         /* format of protocol address */
	u_char  ar_hln;         /* length of hardware address */
	u_char  ar_pln;         /* length of protocol address */
	u_short ar_op;          /* one of: */
#define ARPOP_REQUEST   1       /* request to resolve address */
#define ARPOP_REPLY     2       /* response to previous request */
#define ARPOP_REVREQUEST 3      /* request protocol address given hardware */
#define ARPOP_REVREPLY  4       /* response giving protocol address */
#define ARPOP_INVREQUEST 8      /* request to identify peer */
#define ARPOP_INVREPLY  9       /* response identifying peer */
/*
 * The remaining fields are variable in size,
 * according to the sizes above.
 */
#ifdef COMMENT_ONLY
	u_char  ar_sha[];       /* sender hardware address */
	u_char  ar_spa[];       /* sender protocol address */
	u_char  ar_tha[];       /* target hardware address */
	u_char  ar_tpa[];       /* target protocol address */
#endif
};

/*
 * ARP ioctl request
 */
struct arpreq {
	struct  sockaddr arp_pa;                /* protocol address */
	struct  sockaddr arp_ha;                /* hardware address */
	int     arp_flags;                      /* flags */
};
/*  arp_flags and at_flags field values */
#define ATF_INUSE       0x01    /* entry in use */
#define ATF_COM         0x02    /* completed entry (enaddr valid) */
#define ATF_PERM        0x04    /* permanent entry */
#define ATF_PUBL        0x08    /* publish entry (respond for other host) */
#define ATF_USETRAILERS 0x10    /* has requested trailers */

struct arpstat {
	/* Normal things that happen: */
	uint32_t txrequests;    /* # of ARP requests sent by this host. */
	uint32_t txreplies;     /* # of ARP replies sent by this host. */
	uint32_t txannounces;   /* # of ARP announcements sent by this host. */
	uint32_t rxrequests;    /* # of ARP requests received by this host. */
	uint32_t rxreplies;     /* # of ARP replies received by this host. */
	uint32_t received;      /* # of ARP packets received by this host. */

	/* Abnormal event and error counting: */
	uint32_t txconflicts;   /* # of ARP conflict probes sent */
	uint32_t invalidreqs;   /* # of invalid ARP resolve requests */
	uint32_t reqnobufs;     /* # of failed requests due to no memory */
	uint32_t dropped;       /* # of packets dropped waiting for a reply. */
	uint32_t purged;        /* # of packets purged while removing entries */
	uint32_t timeouts;      /* # of times with entries removed */
	                        /* due to timeout. */
	uint32_t dupips;        /* # of duplicate IPs detected. */

	/* General statistics */
	uint32_t inuse;         /* # of ARP entries in routing table */
	uint32_t txurequests;   /* # of ARP requests sent (unicast) */
	uint32_t held;          /* # of packets held waiting for a reply */
};

#ifdef BSD_KERNEL_PRIVATE
extern struct arpstat arpstat;
#endif /* BSD_KERNEL_PRIVATE */

#endif /* !_NET_IF_ARP_H_ */

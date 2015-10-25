/*
 * Copyright (c) 2000-2014 Apple Inc. All rights reserved.
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
 * Copyright (c) 1982, 1986, 1990, 1993
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
 *	@(#)in_pcb.h	8.1 (Berkeley) 6/10/93
 * $FreeBSD: src/sys/netinet/in_pcb.h,v 1.32.2.4 2001/08/13 16:26:17 ume Exp $
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2007 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

#ifndef _NETINET_IN_PCB_H_
#define	_NETINET_IN_PCB_H_
#include <sys/appleapiopts.h>

#include <sys/types.h>
#include <sys/queue.h>

#include <netinet6/ipsec.h> /* for IPSEC */
#if NECP
#include <net/necp.h>
#endif

#if IPSEC
#include <netinet6/ipsec.h> /* for IPSEC */
#endif

typedef	u_quad_t	inp_gen_t;

/*
 * PCB with AF_INET6 null bind'ed laddr can receive AF_INET input packet.
 * So, AF_INET6 null laddr is also used as AF_INET null laddr, by utilizing
 * the following structure.
 */
struct in_addr_4in6 {
	u_int32_t	ia46_pad32[3];
	struct	in_addr	ia46_addr4;
};


/*
 * Interface exported to userland by various protocols which use
 * inpcbs.  Hack alert -- only define if struct xsocket is in scope.
 */
#pragma pack(4)

#if defined(__LP64__)
struct _inpcb_list_entry {
    u_int32_t	le_next;
    u_int32_t	le_prev;
};
#define	_INPCB_PTR(x)		u_int32_t
#define	_INPCB_LIST_ENTRY(x)	struct _inpcb_list_entry
#else /* !__LP64__ */
#define	_INPCB_PTR(x)		x
#define	_INPCB_LIST_ENTRY(x)	LIST_ENTRY(x)
#endif /* !__LP64__ */

struct inpcbinfo;
struct inpcbport;
struct mbuf;
struct ip6_pktopts;
struct ip6_moptions;
struct icmp6_filter;
struct inpcbpolicy;

struct inpcb {
	_INPCB_LIST_ENTRY(inpcb) inp_hash;	/* hash list */
	struct in_addr reserved1;		/* reserved */
	struct in_addr reserved2;		/* reserved */
	u_short	inp_fport;			/* foreign port */
	u_short	inp_lport;			/* local port */
	_INPCB_LIST_ENTRY(inpcb) inp_list;	/* list for all peer PCBs */
	_INPCB_PTR(caddr_t) inp_ppcb;		/* per-protocol pcb */
	_INPCB_PTR(struct inpcbinfo *) inp_pcbinfo;	/* PCB list info */
	_INPCB_PTR(void *) inp_socket;	/* back pointer to socket */
	u_char nat_owner;		/* Used to NAT TCP/UDP traffic */
	u_int32_t nat_cookie;		/* Cookie stored and returned to NAT */
	_INPCB_LIST_ENTRY(inpcb) inp_portlist;	/* this PCB's local port list */
	_INPCB_PTR(struct inpcbport *) inp_phd; /* head of this list */
	inp_gen_t inp_gencnt;		/* generation count of this instance */
	int inp_flags;			/* generic IP/datagram flags */
	u_int32_t inp_flow;

	u_char inp_vflag;

	u_char inp_ip_ttl;		/* time to live proto */
	u_char inp_ip_p;		/* protocol proto */
	/* protocol dependent part */
	union {
		/* foreign host table entry */
		struct in_addr_4in6 inp46_foreign;
		struct in6_addr inp6_foreign;
	} inp_dependfaddr;
	union {
		/* local host table entry */
		struct in_addr_4in6 inp46_local;
		struct in6_addr inp6_local;
	} inp_dependladdr;
	union {
		/* placeholder for routing entry */
		u_char inp4_route[20];
		u_char inp6_route[32];
	} inp_dependroute;
	struct {
		/* type of service proto */
		u_char inp4_ip_tos;
		/* IP options */
		_INPCB_PTR(struct mbuf *) inp4_options;
		/* IP multicast options */
		_INPCB_PTR(struct ip_moptions *) inp4_moptions;
	} inp_depend4;

	struct {
		/* IP options */
		_INPCB_PTR(struct mbuf *) inp6_options;
		u_int8_t inp6_hlim;
		u_int8_t unused_uint8_1;
		ushort unused_uint16_1;
		/* IP6 options for outgoing packets */
		_INPCB_PTR(struct ip6_pktopts *) inp6_outputopts;
		/* IP multicast options */
		_INPCB_PTR(struct ip6_moptions *) inp6_moptions;
		/* ICMPv6 code type filter */
		_INPCB_PTR(struct icmp6_filter *) inp6_icmp6filt;
		/* IPV6_CHECKSUM setsockopt */
		int	inp6_cksum;
		u_short	inp6_ifindex;
		short	inp6_hops;
	} inp_depend6;

	int hash_element;		/* Array index of pcb's hash list */
	_INPCB_PTR(caddr_t) inp_saved_ppcb; /* pointer while cached */
	_INPCB_PTR(struct inpcbpolicy *) inp_sp;
	u_int32_t	reserved[3];	/* reserved */
};

struct	xinpcb {
	u_int32_t	xi_len;		/* length of this structure */
	struct	inpcb xi_inp;
	struct	xsocket xi_socket;
	u_quad_t	xi_alignment_hack;
};

#if !CONFIG_EMBEDDED
struct inpcb64_list_entry {
    u_int64_t   le_next;
    u_int64_t   le_prev;
};

struct	xinpcb64 {
	u_int64_t	xi_len;		/* length of this structure */
	u_int64_t	xi_inpp;
	u_short		inp_fport;	/* foreign port */
	u_short		inp_lport;	/* local port */
	struct inpcb64_list_entry inp_list; /* list for all PCBs */
	u_int64_t	inp_ppcb;	/* ptr to per-protocol PCB */
	u_int64_t	inp_pcbinfo;	/* PCB list info */
	struct inpcb64_list_entry inp_portlist;	/* this PCB's local port list */
	u_int64_t	inp_phd;	/* head of this list */
	inp_gen_t	inp_gencnt;	/* current generation count */
	int		inp_flags;	/* generic IP/datagram flags */
	u_int32_t	inp_flow;
	u_char		inp_vflag;
	u_char		inp_ip_ttl;	/* time to live */
	u_char		inp_ip_p;	/* protocol */
	union {				/* foreign host table entry */
		struct  in_addr_4in6	inp46_foreign;
		struct  in6_addr	inp6_foreign;
	} inp_dependfaddr;
	union {				/* local host table entry */
		struct  in_addr_4in6	inp46_local;
		struct  in6_addr	inp6_local;
	} inp_dependladdr;
	struct {
		u_char	inp4_ip_tos;	/* type of service */
	} inp_depend4;
	struct {
		u_int8_t inp6_hlim;
		int	inp6_cksum;
		u_short	inp6_ifindex;
		short	inp6_hops;
	} inp_depend6;
	struct  xsocket64 xi_socket;
	u_quad_t	xi_alignment_hack;
};
#endif /* !CONFIG_EMBEDDED */


struct	xinpgen {
	u_int32_t	xig_len;	/* length of this structure */
	u_int		xig_count;	/* number of PCBs at this time */
	inp_gen_t	xig_gen;	/* generation count at this time */
	so_gen_t	xig_sogen;	/* current socket generation count */
};

#pragma pack()

/*
 * These defines are for use with the inpcb.
 */
#define	INP_IPV4	0x1
#define	INP_IPV6	0x2
#define	inp_faddr	inp_dependfaddr.inp46_foreign.ia46_addr4
#define	inp_laddr	inp_dependladdr.inp46_local.ia46_addr4
#define	in6p_faddr	inp_dependfaddr.inp6_foreign
#define	in6p_laddr	inp_dependladdr.inp6_local


/*
 * Flags for inp_flags.
 *
 * Some of these are publicly defined for legacy reasons, as they are
 * (unfortunately) used by certain applications to determine, at compile
 * time, whether or not the OS supports certain features.
 */

#define	INP_ANONPORT		0x00000040 /* port chosen for user */


#define	IN6P_IPV6_V6ONLY	0x00008000 /* restrict AF_INET6 socket for v6 */


#define	IN6P_BINDV6ONLY		0x01000000 /* do not grab IPv4 traffic */

#endif /* !_NETINET_IN_PCB_H_ */

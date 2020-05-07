/*
 * Copyright (c) 2000-2018 Apple Inc. All rights reserved.
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
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Copyright (c) 1985, 1986, 1993
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
 *	@(#)in_var.h	8.1 (Berkeley) 6/10/93
 */

#ifndef _NETINET6_IN6_VAR_H_
#define _NETINET6_IN6_VAR_H_
#include <sys/appleapiopts.h>
#include <net/net_kev.h>

#ifdef BSD_KERNEL_PRIVATE
#include <sys/tree.h>
#include <sys/mcache.h>
#endif /* BSD_KERNEL_PRIVATE */
#include <netinet6/scope6_var.h>
#include <sys/kern_event.h>
#include <net/ethernet.h>

/*
 * pltime/vltime are just for future reference (required to implements 2
 * hour rule for hosts).  they should never be modified by nd6_timeout or
 * anywhere else.
 *	userland -> kernel: accept pltime/vltime
 *	kernel -> userland: throw up everything
 *	in kernel: modify preferred/expire only
 */
struct in6_addrlifetime {
	time_t ia6t_expire;     /* valid lifetime expiration time */
	time_t ia6t_preferred;  /* preferred lifetime expiration time */
	u_int32_t ia6t_vltime;  /* valid lifetime */
	u_int32_t ia6t_pltime;  /* prefix lifetime */
};

#ifdef BSD_KERNEL_PRIVATE
struct in6_addrlifetime_32 {
	u_int32_t ia6t_expire;
	u_int32_t ia6t_preferred;
	u_int32_t ia6t_vltime;
	u_int32_t ia6t_pltime;
};

struct in6_addrlifetime_64 {
	u_int64_t ia6t_expire;
	u_int64_t ia6t_preferred;
	u_int32_t ia6t_vltime;
	u_int32_t ia6t_pltime;
};

/*
 * Internal representation of ia6_lifetime (expiration in uptime unit)
 */
struct in6_addrlifetime_i {
	u_int64_t ia6ti_expire;         /* valid lifetime expiration time */
	u_int64_t ia6ti_preferred;      /* preferred lifetime expiration time */
	u_int32_t ia6ti_vltime;         /* valid lifetime */
	u_int32_t ia6ti_pltime;         /* prefix lifetime */
	u_int64_t ia6ti_base_calendartime; /* calendar time at creation */
	u_int64_t ia6ti_base_uptime;    /* uptime at creation */
};

/*
 * Interface address, IPv6 version.  One of these structures
 * is allocated for each interface with an IPv6 address.
 * The ifaddr structure contains the protocol-independent part
 * of the structure and is assumed to be first.
 */

struct in6_ifaddr {
	struct ifaddr ia_ifa;           /* protocol-independent info */
#define ia_ifp          ia_ifa.ifa_ifp
#define ia_flags        ia_ifa.ifa_flags
	struct sockaddr_in6 ia_addr;    /* interface address */
	struct sockaddr_in6 ia_net;     /* network number of interface */
	struct sockaddr_in6 ia_dstaddr; /* space for destination addr */
	struct sockaddr_in6 ia_prefixmask; /* prefix mask */
	u_int32_t ia_plen;              /* prefix length */
	struct in6_ifaddr *ia_next;     /* next in6 list of IP6 addresses */
	int ia6_flags;

	struct in6_addrlifetime_i ia6_lifetime;
	/*
	 * the creation time of this address, which is
	 * currently used for temporary addresses only.
	 */
	u_int64_t ia6_createtime;
	u_int64_t ia6_updatetime;

	/* back pointer to the ND prefix */
	struct nd_prefix *ia6_ndpr;

	/* multicast addresses joined from the kernel */
	LIST_HEAD(, in6_multi_mship) ia6_memberships;
};

#define ifatoia6(ifa)   ((struct in6_ifaddr *)(void *)(ifa))
#endif /* BSD_KERNEL_PRIVATE */

/* control structure to manage address selection policy */
struct in6_addrpolicy {
	struct sockaddr_in6 addr; /* prefix address */
	struct sockaddr_in6 addrmask; /* prefix mask */
	int preced;             /* precedence */
	int label;              /* matching label */
	u_quad_t use;           /* statistics */
};

/*
 * IPv6 interface statistics, as defined in RFC2465 Ipv6IfStatsEntry (p12).
 */
struct in6_ifstat {
	u_quad_t ifs6_in_receive;       /* # of total input datagram */
	u_quad_t ifs6_in_hdrerr;        /* # of datagrams with invalid hdr */
	u_quad_t ifs6_in_toobig;        /* # of datagrams exceeded MTU */
	u_quad_t ifs6_in_noroute;       /* # of datagrams with no route */
	u_quad_t ifs6_in_addrerr;       /* # of datagrams with invalid dst */
	u_quad_t ifs6_in_protounknown;  /* # of datagrams with unknown proto */
	                                /* NOTE: increment on final dst if */
	u_quad_t ifs6_in_truncated;     /* # of truncated datagrams */
	u_quad_t ifs6_in_discard;       /* # of discarded datagrams */
	                                /* NOTE: fragment timeout is not here */
	u_quad_t ifs6_in_deliver;       /* # of datagrams delivered to ULP */
	                                /* NOTE: increment on final dst if */
	u_quad_t ifs6_out_forward;      /* # of datagrams forwarded */
	                                /* NOTE: increment on outgoing if */
	u_quad_t ifs6_out_request;      /* # of outgoing datagrams from ULP */
	                                /* NOTE: does not include forwrads */
	u_quad_t ifs6_out_discard;      /* # of discarded datagrams */
	u_quad_t ifs6_out_fragok;       /* # of datagrams fragmented */
	u_quad_t ifs6_out_fragfail;     /* # of datagrams failed on fragment */
	u_quad_t ifs6_out_fragcreat;    /* # of fragment datagrams */
	                                /* NOTE: this is # after fragment */
	u_quad_t ifs6_reass_reqd;       /* # of incoming fragmented packets */
	                                /* NOTE: increment on final dst if */
	u_quad_t ifs6_reass_ok;         /* # of reassembled packets */
	                                /* NOTE: this is # after reass */
	                                /* NOTE: increment on final dst if */
	u_quad_t ifs6_atmfrag_rcvd;     /* # of atomic fragments received */
	u_quad_t ifs6_reass_fail;       /* # of reass failures */
	                                /* NOTE: may not be packet count */
	                                /* NOTE: increment on final dst if */
	u_quad_t ifs6_in_mcast;         /* # of inbound multicast datagrams */
	u_quad_t ifs6_out_mcast;        /* # of outbound multicast datagrams */

	u_quad_t ifs6_cantfoward_icmp6; /* # of ICMPv6 packets received for unreachable dest */
	u_quad_t ifs6_addr_expiry_cnt;  /* # of address expiry events (excluding privacy addresses) */
	u_quad_t ifs6_pfx_expiry_cnt;   /* # of prefix expiry events */
	u_quad_t ifs6_defrtr_expiry_cnt;        /* # of default router expiry events */
};

/*
 * ICMPv6 interface statistics, as defined in RFC2466 Ipv6IfIcmpEntry.
 * XXX: I'm not sure if this file is the right place for this structure...
 */
struct icmp6_ifstat {
	/*
	 * Input statistics
	 */
	/* ipv6IfIcmpInMsgs, total # of input messages */
	u_quad_t ifs6_in_msg;
	/* ipv6IfIcmpInErrors, # of input error messages */
	u_quad_t ifs6_in_error;
	/* ipv6IfIcmpInDestUnreachs, # of input dest unreach errors */
	u_quad_t ifs6_in_dstunreach;
	/* ipv6IfIcmpInAdminProhibs, # of input admin. prohibited errs */
	u_quad_t ifs6_in_adminprohib;
	/* ipv6IfIcmpInTimeExcds, # of input time exceeded errors */
	u_quad_t ifs6_in_timeexceed;
	/* ipv6IfIcmpInParmProblems, # of input parameter problem errors */
	u_quad_t ifs6_in_paramprob;
	/* ipv6IfIcmpInPktTooBigs, # of input packet too big errors */
	u_quad_t ifs6_in_pkttoobig;
	/* ipv6IfIcmpInEchos, # of input echo requests */
	u_quad_t ifs6_in_echo;
	/* ipv6IfIcmpInEchoReplies, # of input echo replies */
	u_quad_t ifs6_in_echoreply;
	/* ipv6IfIcmpInRouterSolicits, # of input router solicitations */
	u_quad_t ifs6_in_routersolicit;
	/* ipv6IfIcmpInRouterAdvertisements, # of input router advertisements */
	u_quad_t ifs6_in_routeradvert;
	/* ipv6IfIcmpInNeighborSolicits, # of input neighbor solicitations */
	u_quad_t ifs6_in_neighborsolicit;
	/* ipv6IfIcmpInNeighborAdvertisements, # of input neighbor advs. */
	u_quad_t ifs6_in_neighboradvert;
	/* ipv6IfIcmpInRedirects, # of input redirects */
	u_quad_t ifs6_in_redirect;
	/* ipv6IfIcmpInGroupMembQueries, # of input MLD queries */
	u_quad_t ifs6_in_mldquery;
	/* ipv6IfIcmpInGroupMembResponses, # of input MLD reports */
	u_quad_t ifs6_in_mldreport;
	/* ipv6IfIcmpInGroupMembReductions, # of input MLD done */
	u_quad_t ifs6_in_mlddone;

	/*
	 * Output statistics. We should solve unresolved routing problem...
	 */
	/* ipv6IfIcmpOutMsgs, total # of output messages */
	u_quad_t ifs6_out_msg;
	/* ipv6IfIcmpOutErrors, # of output error messages */
	u_quad_t ifs6_out_error;
	/* ipv6IfIcmpOutDestUnreachs, # of output dest unreach errors */
	u_quad_t ifs6_out_dstunreach;
	/* ipv6IfIcmpOutAdminProhibs, # of output admin. prohibited errs */
	u_quad_t ifs6_out_adminprohib;
	/* ipv6IfIcmpOutTimeExcds, # of output time exceeded errors */
	u_quad_t ifs6_out_timeexceed;
	/* ipv6IfIcmpOutParmProblems, # of output parameter problem errors */
	u_quad_t ifs6_out_paramprob;
	/* ipv6IfIcmpOutPktTooBigs, # of output packet too big errors */
	u_quad_t ifs6_out_pkttoobig;
	/* ipv6IfIcmpOutEchos, # of output echo requests */
	u_quad_t ifs6_out_echo;
	/* ipv6IfIcmpOutEchoReplies, # of output echo replies */
	u_quad_t ifs6_out_echoreply;
	/* ipv6IfIcmpOutRouterSolicits, # of output router solicitations */
	u_quad_t ifs6_out_routersolicit;
	/* ipv6IfIcmpOutRouterAdvertisements, # of output router advs. */
	u_quad_t ifs6_out_routeradvert;
	/* ipv6IfIcmpOutNeighborSolicits, # of output neighbor solicitations */
	u_quad_t ifs6_out_neighborsolicit;
	/* ipv6IfIcmpOutNeighborAdvertisements, # of output neighbor advs. */
	u_quad_t ifs6_out_neighboradvert;
	/* ipv6IfIcmpOutRedirects, # of output redirects */
	u_quad_t ifs6_out_redirect;
	/* ipv6IfIcmpOutGroupMembQueries, # of output MLD queries */
	u_quad_t ifs6_out_mldquery;
	/* ipv6IfIcmpOutGroupMembResponses, # of output MLD reports */
	u_quad_t ifs6_out_mldreport;
	/* ipv6IfIcmpOutGroupMembReductions, # of output MLD done */
	u_quad_t ifs6_out_mlddone;
};

struct in6_ifreq {
	char    ifr_name[IFNAMSIZ];
	union {
		struct  sockaddr_in6 ifru_addr;
		struct  sockaddr_in6 ifru_dstaddr;
		int     ifru_flags;
		int     ifru_flags6;
		int     ifru_metric;
		int     ifru_intval;
		caddr_t ifru_data;
		struct in6_addrlifetime ifru_lifetime;
		struct in6_ifstat ifru_stat;
		struct icmp6_ifstat ifru_icmp6stat;
		u_int32_t ifru_scope_id[SCOPE6_ID_MAX];
	} ifr_ifru;
};

struct in6_aliasreq {
	char    ifra_name[IFNAMSIZ];
	struct  sockaddr_in6 ifra_addr;
	struct  sockaddr_in6 ifra_dstaddr;
	struct  sockaddr_in6 ifra_prefixmask;
	int     ifra_flags;
	struct in6_addrlifetime ifra_lifetime;
};

#ifdef PRIVATE
#define IN6_CGA_MODIFIER_LENGTH 16

struct in6_cga_modifier {
	u_int8_t octets[IN6_CGA_MODIFIER_LENGTH];
};

struct in6_cga_prepare {
	struct in6_cga_modifier cga_modifier;
	u_int8_t cga_security_level;
	u_int8_t reserved_A[15];
};

#define IN6_CGA_KEY_MAXSIZE     2048    /* octets */

struct in6_cga_nodecfg {
	struct iovec cga_privkey;
	struct iovec cga_pubkey;
	struct in6_cga_prepare cga_prepare;
};

/*
 * XXX in6_llstartreq will be removed once
 * configd adopts the more generically named
 * in6_cgareq structure.
 */
struct in6_llstartreq {
	char llsr_name[IFNAMSIZ];
	int llsr_flags;
	struct in6_cga_prepare llsr_cgaprep;
	struct in6_addrlifetime llsr_lifetime;
};

struct in6_cgareq {
	char cgar_name[IFNAMSIZ];
	int cgar_flags;
	struct in6_cga_prepare cgar_cgaprep;
	struct in6_addrlifetime cgar_lifetime;
};

#ifdef BSD_KERNEL_PRIVATE
/*
 * XXX Corresponding versions of in6_llstartreq
 * will be removed after the new in6_cgareq is
 * adopted by configd
 */
struct in6_llstartreq_32 {
	char llsr_name[IFNAMSIZ];
	int llsr_flags;
	struct in6_cga_prepare llsr_cgaprep;
	struct in6_addrlifetime_32 llsr_lifetime;
};

struct in6_llstartreq_64 {
	char llsr_name[IFNAMSIZ];
	int llsr_flags;
	struct in6_cga_prepare llsr_cgaprep;
	struct in6_addrlifetime_64 llsr_lifetime;
};

struct in6_cgareq_32 {
	char cgar_name[IFNAMSIZ];
	int cgar_flags;
	struct in6_cga_prepare cgar_cgaprep;
	struct in6_addrlifetime_32 cgar_lifetime;
};

struct in6_cgareq_64 {
	char cgar_name[IFNAMSIZ];
	int cgar_flags;
	struct in6_cga_prepare cgar_cgaprep;
	struct in6_addrlifetime_64 cgar_lifetime;
};

#endif /* !BSD_KERNEL_PRIVATE */
#endif /* PRIVATE */

#ifdef BSD_KERNEL_PRIVATE
struct in6_aliasreq_32 {
	char    ifra_name[IFNAMSIZ];
	struct  sockaddr_in6 ifra_addr;
	struct  sockaddr_in6 ifra_dstaddr;
	struct  sockaddr_in6 ifra_prefixmask;
	int     ifra_flags;
	struct in6_addrlifetime_32 ifra_lifetime;
};

struct in6_aliasreq_64 {
	char    ifra_name[IFNAMSIZ];
	struct  sockaddr_in6 ifra_addr;
	struct  sockaddr_in6 ifra_dstaddr;
	struct  sockaddr_in6 ifra_prefixmask;
	int     ifra_flags;
	struct in6_addrlifetime_64 ifra_lifetime;
};
#endif /* BSD_KERNEL_PRIVATE */

/* prefix type macro */
#define IN6_PREFIX_ND   1
#define IN6_PREFIX_RR   2

/*
 * prefix related flags passed between kernel(NDP related part) and
 * user land command(ifconfig) and daemon(rtadvd).
 */
struct in6_prflags {
	struct prf_ra {
		u_char onlink : 1;
		u_char autonomous : 1;
		u_char reserved : 6;
	} prf_ra;
	u_char prf_reserved1;
	u_short prf_reserved2;
	/* want to put this on 4byte offset */
	struct prf_rr {
		u_char decrvalid : 1;
		u_char decrprefd : 1;
		u_char reserved : 6;
	} prf_rr;
	u_char prf_reserved3;
	u_short prf_reserved4;
};

struct  in6_prefixreq {
	char    ipr_name[IFNAMSIZ];
	u_char  ipr_origin;
	u_char  ipr_plen;
	u_int32_t ipr_vltime;
	u_int32_t ipr_pltime;
	struct in6_prflags ipr_flags;
	struct  sockaddr_in6 ipr_prefix;
};

#define PR_ORIG_RA      0
#define PR_ORIG_RR      1
#define PR_ORIG_STATIC  2
#define PR_ORIG_KERNEL  3

#define ipr_raf_onlink          ipr_flags.prf_ra.onlink
#define ipr_raf_auto            ipr_flags.prf_ra.autonomous

#define ipr_statef_onlink       ipr_flags.prf_state.onlink

#define ipr_rrf_decrvalid       ipr_flags.prf_rr.decrvalid
#define ipr_rrf_decrprefd       ipr_flags.prf_rr.decrprefd

struct in6_rrenumreq {
	char    irr_name[IFNAMSIZ];
	u_char  irr_origin;
	u_char  irr_m_len;      /* match len for matchprefix */
	u_char  irr_m_minlen;   /* minlen for matching prefix */
	u_char  irr_m_maxlen;   /* maxlen for matching prefix */
	u_char  irr_u_uselen;   /* uselen for adding prefix */
	u_char  irr_u_keeplen;  /* keeplen from matching prefix */
	struct irr_raflagmask {
		u_char onlink : 1;
		u_char autonomous : 1;
		u_char reserved : 6;
	} irr_raflagmask;
	u_int32_t irr_vltime;
	u_int32_t irr_pltime;
	struct in6_prflags irr_flags;
	struct  sockaddr_in6 irr_matchprefix;
	struct  sockaddr_in6 irr_useprefix;
};

#define irr_raf_mask_onlink     irr_raflagmask.onlink
#define irr_raf_mask_auto       irr_raflagmask.autonomous
#define irr_raf_mask_reserved   irr_raflagmask.reserved

#define irr_raf_onlink          irr_flags.prf_ra.onlink
#define irr_raf_auto            irr_flags.prf_ra.autonomous

#define irr_statef_onlink       irr_flags.prf_state.onlink

#define irr_rrf                 irr_flags.prf_rr
#define irr_rrf_decrvalid       irr_flags.prf_rr.decrvalid
#define irr_rrf_decrprefd       irr_flags.prf_rr.decrprefd

/*
 * Event data, inet6 style.
 */
struct kev_in6_addrlifetime {
	u_int32_t ia6t_expire;
	u_int32_t ia6t_preferred;
	u_int32_t ia6t_vltime;
	u_int32_t ia6t_pltime;
};

struct kev_in6_data {
	struct net_event_data link_data;
	struct sockaddr_in6 ia_addr;    /* interface address */
	struct sockaddr_in6 ia_net;     /* network number of interface */
	struct sockaddr_in6 ia_dstaddr; /* space for destination addr */
	struct sockaddr_in6 ia_prefixmask; /* prefix mask */
	u_int32_t ia_plen;              /* prefix length */
	u_int32_t ia6_flags;            /* address flags from in6_ifaddr */
	struct kev_in6_addrlifetime ia_lifetime; /* address life info */
	uint8_t ia_mac[ETHER_ADDR_LEN];
};

#ifdef BSD_KERNEL_PRIVATE
/* Utility function used inside netinet6 kernel code for generating events */
void in6_post_msg(struct ifnet *, u_int32_t, struct in6_ifaddr *, uint8_t *mac);

#define IN6_ARE_MASKED_ADDR_EQUAL(d, a, m)      (       \
	(((d)->s6_addr32[0] ^ (a)->s6_addr32[0]) & (m)->s6_addr32[0]) == 0 && \
	(((d)->s6_addr32[1] ^ (a)->s6_addr32[1]) & (m)->s6_addr32[1]) == 0 && \
	(((d)->s6_addr32[2] ^ (a)->s6_addr32[2]) & (m)->s6_addr32[2]) == 0 && \
	(((d)->s6_addr32[3] ^ (a)->s6_addr32[3]) & (m)->s6_addr32[3]) == 0)

/*
 * Given a pointer to an in6_ifaddr (ifaddr),
 * return a pointer to the addr as a sockaddr_in6
 */
#define IA6_IN6(ia)     (&((ia)->ia_addr.sin6_addr))
#define IA6_DSTIN6(ia)  (&((ia)->ia_dstaddr.sin6_addr))
#define IA6_MASKIN6(ia) (&((ia)->ia_prefixmask.sin6_addr))
#define IA6_SIN6(ia)    (&((ia)->ia_addr))
#define IA6_DSTSIN6(ia) (&((ia)->ia_dstaddr))
#define IFA_IN6(x)      \
	(&((struct sockaddr_in6 *)(void *)((x)->ifa_addr))->sin6_addr)
#define IFA_DSTIN6(x)   \
	(&((struct sockaddr_in6 *)(void *)((x)->ifa_dstaddr))->sin6_addr)
#define IFPR_IN6(x)     \
	(&((struct sockaddr_in6 *)(void *)((x)->ifpr_prefix))->sin6_addr)
#endif /* BSD_KERNEL_PRIVATE */

#define SIOCSIFADDR_IN6          _IOW('i', 12, struct in6_ifreq)
#define SIOCGIFADDR_IN6         _IOWR('i', 33, struct in6_ifreq)

/*
 * SIOCSxxx ioctls should be unused (see comments in in6.c), but
 * we do not shift numbers for binary compatibility.
 */
#define SIOCSIFDSTADDR_IN6      _IOW('i', 14, struct in6_ifreq)
#define SIOCSIFNETMASK_IN6      _IOW('i', 22, struct in6_ifreq)

#define SIOCGIFDSTADDR_IN6      _IOWR('i', 34, struct in6_ifreq)
#define SIOCGIFNETMASK_IN6      _IOWR('i', 37, struct in6_ifreq)

#define SIOCDIFADDR_IN6         _IOW('i', 25, struct in6_ifreq)
#define SIOCAIFADDR_IN6         _IOW('i', 26, struct in6_aliasreq)
#ifdef BSD_KERNEL_PRIVATE
#define SIOCAIFADDR_IN6_32      _IOW('i', 26, struct in6_aliasreq_32)
#define SIOCAIFADDR_IN6_64      _IOW('i', 26, struct in6_aliasreq_64)
#endif /* BSD_KERNEL_PRIVATE */

#define SIOCSIFPHYADDR_IN6      _IOW('i', 62, struct in6_aliasreq)
#ifdef BSD_KERNEL_PRIVATE
#define SIOCSIFPHYADDR_IN6_32   _IOW('i', 62, struct in6_aliasreq_32)
#define SIOCSIFPHYADDR_IN6_64   _IOW('i', 62, struct in6_aliasreq_64)
#endif /* BSD_KERNEL_PRIVATE */
#define SIOCGIFPSRCADDR_IN6     _IOWR('i', 63, struct in6_ifreq)
#define SIOCGIFPDSTADDR_IN6     _IOWR('i', 64, struct in6_ifreq)
#define SIOCGIFAFLAG_IN6        _IOWR('i', 73, struct in6_ifreq)

#define SIOCGDRLST_IN6          _IOWR('i', 74, struct in6_drlist)
#ifdef BSD_KERNEL_PRIVATE
#define SIOCGDRLST_IN6_32       _IOWR('i', 74, struct in6_drlist_32)
#define SIOCGDRLST_IN6_64       _IOWR('i', 74, struct in6_drlist_64)
#endif /* BSD_KERNEL_PRIVATE */

#define SIOCGPRLST_IN6          _IOWR('i', 75, struct in6_prlist)
#ifdef BSD_KERNEL_PRIVATE
#define SIOCGPRLST_IN6_32       _IOWR('i', 75, struct in6_prlist_32)
#define SIOCGPRLST_IN6_64       _IOWR('i', 75, struct in6_prlist_64)
#endif /* BSD_KERNEL_PRIVATE */

#define OSIOCGIFINFO_IN6        _IOWR('i', 108, struct in6_ondireq)
#define SIOCGIFINFO_IN6         _IOWR('i', 76, struct in6_ondireq)
#define SIOCSNDFLUSH_IN6        _IOWR('i', 77, struct in6_ifreq)

#define SIOCGNBRINFO_IN6        _IOWR('i', 78, struct in6_nbrinfo)
#ifdef BSD_KERNEL_PRIVATE
#define SIOCGNBRINFO_IN6_32     _IOWR('i', 78, struct in6_nbrinfo_32)
#define SIOCGNBRINFO_IN6_64     _IOWR('i', 78, struct in6_nbrinfo_64)
#endif /* BSD_KERNEL_PRIVATE */

#define SIOCSPFXFLUSH_IN6       _IOWR('i', 79, struct in6_ifreq)
#define SIOCSRTRFLUSH_IN6       _IOWR('i', 80, struct in6_ifreq)

#define SIOCGIFALIFETIME_IN6    _IOWR('i', 81, struct in6_ifreq)
#define SIOCSIFALIFETIME_IN6    _IOWR('i', 82, struct in6_ifreq)
#define SIOCGIFSTAT_IN6         _IOWR('i', 83, struct in6_ifreq)
#define SIOCGIFSTAT_ICMP6       _IOWR('i', 84, struct in6_ifreq)

#define SIOCSDEFIFACE_IN6       _IOWR('i', 85, struct in6_ndifreq)
#define SIOCGDEFIFACE_IN6       _IOWR('i', 86, struct in6_ndifreq)
#ifdef BSD_KERNEL_PRIVATE
#define SIOCSDEFIFACE_IN6_32    _IOWR('i', 85, struct in6_ndifreq_32)
#define SIOCSDEFIFACE_IN6_64    _IOWR('i', 85, struct in6_ndifreq_64)
#define SIOCGDEFIFACE_IN6_32    _IOWR('i', 86, struct in6_ndifreq_32)
#define SIOCGDEFIFACE_IN6_64    _IOWR('i', 86, struct in6_ndifreq_64)
#endif /* BSD_KERNEL_PRIVATE */

#define SIOCSIFINFO_FLAGS       _IOWR('i', 87, struct in6_ndireq) /* XXX */

/* N.B.: These 3 ioctls are deprecated and won't work */
#define SIOCSSCOPE6             _IOW('i', 88, struct in6_ifreq)
#define SIOCGSCOPE6             _IOWR('i', 89, struct in6_ifreq)
#define SIOCGSCOPE6DEF          _IOWR('i', 90, struct in6_ifreq)

#define SIOCSIFPREFIX_IN6       _IOW('i', 100, struct in6_prefixreq) /* set */
#define SIOCGIFPREFIX_IN6       _IOWR('i', 101, struct in6_prefixreq) /* get */
#define SIOCDIFPREFIX_IN6       _IOW('i', 102, struct in6_prefixreq) /* del */
#define SIOCAIFPREFIX_IN6       _IOW('i', 103, struct in6_rrenumreq) /* add */

/* change */
#define SIOCCIFPREFIX_IN6       _IOW('i', 104, struct in6_rrenumreq)

/* set global */
#define SIOCSGIFPREFIX_IN6      _IOW('i', 105, struct in6_rrenumreq)

/*
 * multicast routing, get s/g pkt cnt, pkt cnt per interface.
 */
#define SIOCGETSGCNT_IN6        _IOWR('u', 28, struct sioc_sg_req6)
#define SIOCGETMIFCNT_IN6       _IOWR('u', 107, struct sioc_mif_req6)
#ifdef BSD_KERNEL_PRIVATE
#define SIOCGETMIFCNT_IN6_32    _IOWR('u', 107, struct sioc_mif_req6_32)
#define SIOCGETMIFCNT_IN6_64    _IOWR('u', 107, struct sioc_mif_req6_64)
#endif /* BSD_KERNEL_PRIVATE */

#define SIOCAADDRCTL_POLICY     _IOW('u', 108, struct in6_addrpolicy)
#define SIOCDADDRCTL_POLICY     _IOW('u', 109, struct in6_addrpolicy)

#ifdef PRIVATE
/*
 * temporary control calls to attach/detach IPv6 to/from an interface.
 */
#define SIOCPROTOATTACH_IN6     _IOWR('i', 110, struct in6_aliasreq)
#ifdef BSD_KERNEL_PRIVATE
#define SIOCPROTOATTACH_IN6_32  _IOWR('i', 110, struct in6_aliasreq_32)
#define SIOCPROTOATTACH_IN6_64  _IOWR('i', 110, struct in6_aliasreq_64)
#endif /* BSD_KERNEL_PRIVATE */
#define SIOCPROTODETACH_IN6     _IOWR('i', 111, struct in6_ifreq)

/*
 * start/stop aquiring linklocal on interface
 */
#define SIOCLL_START            _IOWR('i', 130, struct in6_aliasreq)
#ifdef BSD_KERNEL_PRIVATE
#define SIOCLL_START_32         _IOWR('i', 130, struct in6_aliasreq_32)
#define SIOCLL_START_64         _IOWR('i', 130, struct in6_aliasreq_64)
#endif /* BSD_KERNEL_PRIVATE */
#define SIOCLL_STOP             _IOWR('i', 131, struct in6_ifreq)

/*
 * accept rtadvd (and stop accepting) on this interface.
 */
#define SIOCAUTOCONF_START      _IOWR('i', 132, struct in6_ifreq)
#define SIOCAUTOCONF_STOP       _IOWR('i', 133, struct in6_ifreq)

/*
 * add/remove default IPv6 router.
 */
#define SIOCDRADD_IN6           _IOWR('u', 134, struct in6_defrouter)
#ifdef BSD_KERNEL_PRIVATE
#define SIOCDRADD_IN6_32        _IOWR('u', 134, struct in6_defrouter_32)
#define SIOCDRADD_IN6_64        _IOWR('u', 134, struct in6_defrouter_64)
#endif /* BSD_KERNEL_PRIVATE */
#define SIOCDRDEL_IN6           _IOWR('u', 135, struct in6_defrouter)
#ifdef BSD_KERNEL_PRIVATE
#define SIOCDRDEL_IN6_32        _IOWR('u', 135, struct in6_defrouter_32)
#define SIOCDRDEL_IN6_64        _IOWR('u', 135, struct in6_defrouter_64)
#endif /* BSD_KERNEL_PRIVATE */

/*
 * enable/disable IPv6 router mode on interface.
 */
#define SIOCSETROUTERMODE_IN6   _IOWR('i', 136, struct in6_ifreq)

/*
 * start secure link-local interface addresses
 */
#define SIOCLL_CGASTART         _IOW('i', 160, struct in6_cgareq)
#ifdef BSD_KERNEL_PRIVATE
#define SIOCLL_CGASTART_32      _IOW('i', 160, struct in6_cgareq_32)
#define SIOCLL_CGASTART_64      _IOW('i', 160, struct in6_cgareq_64)
#endif

#define SIOCGIFCGAPREP_IN6      _IOWR('i', 187, struct in6_cgareq)
#define SIOCSIFCGAPREP_IN6      _IOWR('i', 188, struct in6_cgareq)

#define SIOCCLAT46_START        _IOWR('i', 189, struct in6_ifreq)
#define SIOCCLAT46_STOP         _IOWR('i', 190, struct in6_ifreq)
#endif /* PRIVATE */

#ifdef BSD_KERNEL_PRIVATE
/*
 * XXX: Do not extend IN6_IFF values beyond 16-bit.  We currently copy the
 *	values from ia6_flags as is into m_pkthdr.{src,dst}_iff which are
 *	defined as 16-bit variables.  We can relax this once we define a
 *	unified set of interface flags for {ia,ia6}_flags and perform
 *	translation between those and the publicly-defined ones below.
 */
#endif /* BSD_KERNEL_PRIVATE */
#define IN6_IFF_ANYCAST         0x0001  /* anycast address */
#define IN6_IFF_TENTATIVE       0x0002  /* tentative address */
#define IN6_IFF_DUPLICATED      0x0004  /* DAD detected duplicate */
#define IN6_IFF_DETACHED        0x0008  /* may be detached from the link */
#define IN6_IFF_DEPRECATED      0x0010  /* deprecated address */

/* don't perform DAD on this address (used only at first SIOC* call) */
#define IN6_IFF_NODAD           0x0020

#define IN6_IFF_AUTOCONF        0x0040  /* autoconfigurable address. */
#define IN6_IFF_TEMPORARY       0x0080  /* temporary (anonymous) address. */
#define IN6_IFF_DYNAMIC         0x0100  /* assigned by DHCPv6 service */
#define IN6_IFF_OPTIMISTIC      0x0200  /* optimistic DAD, i.e. RFC 4429 */
#define IN6_IFF_SECURED         0x0400  /* cryptographically generated */
#ifdef PRIVATE
#define IN6_IFF_SWIFTDAD        0x0800  /* DAD with no delay */
#endif
#define IN6_IFF_CLAT46          0x1000  /* Address reserved for CLAT46 */
#define IN6_IFF_NOPFX           0x8000  /* Depreciated. Don't use. */

/* Duplicate Address Detection [DAD] in progress. */
#define IN6_IFF_DADPROGRESS     (IN6_IFF_TENTATIVE|IN6_IFF_OPTIMISTIC)

/* do not input/output */
#define IN6_IFF_NOTREADY        (IN6_IFF_TENTATIVE|IN6_IFF_DUPLICATED)

/* SLAAC/DHCPv6 address */
#define IN6_IFF_NOTMANUAL       (IN6_IFF_AUTOCONF|IN6_IFF_DYNAMIC)

#ifdef KERNEL
#define IN6_ARE_SCOPE_CMP(a, b)         ((a) - (b))
#define IN6_ARE_SCOPE_EQUAL(a, b)       ((a) == (b))
#endif /* KERNEL */

#ifdef BSD_KERNEL_PRIVATE
extern struct in6_ifaddr *in6_ifaddrs;

extern struct icmp6stat icmp6stat;
extern lck_rw_t in6_ifaddr_rwlock;
extern lck_mtx_t proxy6_lock;
extern u_char inet6ctlerrmap[];
extern u_int32_t in6_maxmtu;

/* N.B.: if_inet6data is never freed once set, so we don't need to lock */
#define in6_ifstat_add_common(_ifp, _tag, _count, _atomic) do {         \
	if (_ifp != NULL && IN6_IFEXTRA(_ifp) != NULL) {                \
	        if (_atomic)                                            \
	                atomic_add_64(                                  \
	                    &IN6_IFEXTRA(_ifp)->in6_ifstat._tag, _count);\
	        else                                                    \
	                IN6_IFEXTRA(_ifp)->in6_ifstat._tag += _count;   \
	}                                                               \
} while (0)

/* atomic version */
#define in6_ifstat_inc(_ifp, _tag) \
	in6_ifstat_add_common(_ifp, _tag, 1, TRUE)

/* non-atomic version (for fast paths) */
#define in6_ifstat_inc_na(_ifp, _tag) \
	in6_ifstat_add_common(_ifp, _tag, 1, FALSE)

/* atomic add version */
#define in6_ifstat_add(_ifp, _tag, _count) \
	in6_ifstat_add_common(_ifp, _tag, _count, TRUE)

/*
 * Macro for finding the internet address structure (in6_ifaddr) corresponding
 * to a given interface (ifnet structure).
 */

#define IFP_TO_IA6(ifp, ia)                                             \
	/* struct ifnet *ifp; */                                        \
	/* struct in6_ifaddr *ia; */                                    \
do {                                                                    \
	struct ifaddr *_ifa;                                            \
	ifnet_lock_assert(ifp, LCK_RW_ASSERT_HELD);                     \
	for (_ifa = (ifp)->if_addrlist.tqh_first; _ifa != NULL;         \
	    _ifa = _ifa->ifa_list.tqe_next) {                           \
	        IFA_LOCK(_ifa);                                         \
	        if (_ifa->ifa_addr->sa_family == AF_INET6) {            \
	                IFA_ADDREF_LOCKED(_ifa);                        \
	                IFA_UNLOCK(_ifa);                               \
	                break;                                          \
	        }                                                       \
	        IFA_UNLOCK(_ifa);                                       \
	}                                                               \
	(ia) = (struct in6_ifaddr *)_ifa;                               \
} while (0)

/*
 * IPv6 multicast MLD-layer source entry.
 */
struct ip6_msource {
	RB_ENTRY(ip6_msource)   im6s_link;      /* RB tree links */
	struct in6_addr         im6s_addr;
	struct im6s_st {
		uint16_t        ex;             /* # of exclusive members */
		uint16_t        in;             /* # of inclusive members */
	}                       im6s_st[2];     /* state at t0, t1 */
	uint8_t                 im6s_stp;       /* pending query */
};

RB_HEAD(ip6_msource_tree, ip6_msource);

RB_PROTOTYPE_SC_PREV(__private_extern__, ip6_msource_tree, ip6_msource,
    im6s_link, ip6_msource_cmp);

/*
 * IPv6 multicast PCB-layer source entry.
 *
 * NOTE: overlapping use of struct ip6_msource fields at start.
 */
struct in6_msource {
	RB_ENTRY(ip6_msource)   im6s_link;      /* Common field */
	struct in6_addr         im6s_addr;      /* Common field */
	uint8_t                 im6sl_st[2];    /* state before/at commit */
};

/*
 * IPv6 multicast PCB-layer group filter descriptor.
 */
struct in6_mfilter {
	struct ip6_msource_tree im6f_sources; /* source list for (S,G) */
	u_long                  im6f_nsrc;    /* # of source entries */
	uint8_t                 im6f_st[2];   /* state before/at commit */
};

/*
 * Legacy KAME IPv6 multicast membership descriptor.
 */
struct in6_multi_mship {
	struct  in6_multi *i6mm_maddr;  /* Multicast address pointer */
	LIST_ENTRY(in6_multi_mship) i6mm_chain;  /* multicast options chain */
};

#ifdef BSD_KERNEL_PRIVATE
#include <netinet6/nd6_var.h>
#include <net/if_llatbl.h>

/*
 * Per-interface IPv6 structures.
 */
struct in6_ifextra {
	struct scope6_id        scope6_id;
	struct in6_ifstat       in6_ifstat;
	struct icmp6_ifstat     icmp6_ifstat;
	struct nd_ifinfo        nd_ifinfo;
	uint32_t                netsig_len;
	u_int8_t                netsig[IFNET_SIGNATURELEN];
	struct ipv6_prefix      nat64_prefixes[NAT64_MAX_NUM_PREFIXES];
	struct lltable          *ii_llt;        /* NDP state */
};
#define IN6_IFEXTRA(_ifp)       ((struct in6_ifextra *)(_ifp->if_inet6data))
#define LLTABLE6(ifp)           ((IN6_IFEXTRA(ifp) == NULL) ? NULL : IN6_IFEXTRA(ifp)->ii_llt)
#endif /* BSD_KERNEL_PRIVATE */

struct mld_ifinfo;

/*
 * IPv6 group descriptor.
 *
 * For every entry on an ifnet's if_multiaddrs list which represents
 * an IP multicast group, there is one of these structures.
 *
 * If any source filters are present, then a node will exist in the RB-tree
 * to permit fast lookup by source whenever an operation takes place.
 * This permits pre-order traversal when we issue reports.
 * Source filter trees are kept separately from the socket layer to
 * greatly simplify locking.
 *
 * When MLDv2 is active, in6m_timer is the response to group query timer.
 * The state-change timer in6m_sctimer is separate; whenever state changes
 * for the group the state change record is generated and transmitted,
 * and kept if retransmissions are necessary.
 *
 * The request count here is a count of requests for this address, not a
 * count of pointers to this structure.
 *
 * FUTURE: in6m_entry is now only used when groups are being purged
 * on a detaching ifnet.  It could be demoted to a SLIST_ENTRY.
 */
struct in6_multi {
	decl_lck_mtx_data(, in6m_lock);
	u_int32_t in6m_refcount;        /* reference count */
	u_int32_t in6m_reqcnt;          /* request count for this address */
	u_int32_t in6m_debug;           /* see ifa_debug flags */
	LIST_ENTRY(in6_multi) in6m_entry; /* list glue */
	struct  in6_addr in6m_addr;     /* IP6 multicast address */
	struct  ifnet *in6m_ifp;        /* back pointer to ifnet */
	struct  ifmultiaddr *in6m_ifma; /* back pointer to ifmultiaddr */
	u_int   in6m_state;             /* state of the membership */
	u_int   in6m_timer;             /* MLD6 listener report timer */

	/* New fields for MLDv2 follow. */
	struct mld_ifinfo       *in6m_mli;      /* MLD info */
	SLIST_ENTRY(in6_multi)   in6m_dtle;     /* detached waiting for rele  */
	SLIST_ENTRY(in6_multi)   in6m_nrele;    /* to-be-released by MLD */
	u_int32_t                in6m_nrelecnt; /* deferred release count */
	struct ip6_msource_tree  in6m_srcs;     /* tree of sources */
	u_long                   in6m_nsrc;     /* # of tree entries */

	struct ifqueue           in6m_scq;  /* pending state-change packets */
	struct timeval           in6m_lastgsrtv;        /* last G-S-R query */
	uint16_t                 in6m_sctimer;  /* state-change timer */
	uint16_t                 in6m_scrv;     /* state-change rexmit count */

	/*
	 * SSM state counters which track state at T0 (the time the last
	 * state-change report's RV timer went to zero) and T1
	 * (time of pending report, i.e. now).
	 * Used for computing MLDv2 state-change reports. Several refcounts
	 * are maintained here to optimize for common use-cases.
	 */
	struct in6m_st {
		uint16_t        iss_fmode;      /* MLD filter mode */
		uint16_t        iss_asm;        /* # of ASM listeners */
		uint16_t        iss_ex;         /* # of exclusive members */
		uint16_t        iss_in;         /* # of inclusive members */
		uint16_t        iss_rec;        /* # of recorded sources */
	}                       in6m_st[2];     /* state at t0, t1 */

	void (*in6m_trace)              /* callback fn for tracing refs */
	(struct in6_multi *, int);
};

#define IN6M_LOCK_ASSERT_HELD(_in6m)                                    \
	LCK_MTX_ASSERT(&(_in6m)->in6m_lock, LCK_MTX_ASSERT_OWNED)

#define IN6M_LOCK_ASSERT_NOTHELD(_in6m)                                 \
	LCK_MTX_ASSERT(&(_in6m)->in6m_lock, LCK_MTX_ASSERT_NOTOWNED)

#define IN6M_LOCK(_in6m)                                                \
	lck_mtx_lock(&(_in6m)->in6m_lock)

#define IN6M_LOCK_SPIN(_in6m)                                           \
	lck_mtx_lock_spin(&(_in6m)->in6m_lock)

#define IN6M_CONVERT_LOCK(_in6m) do {                                   \
	IN6M_LOCK_ASSERT_HELD(_in6m);                                   \
	lck_mtx_convert_spin(&(_in6m)->in6m_lock);                      \
} while (0)

#define IN6M_UNLOCK(_in6m)                                              \
	lck_mtx_unlock(&(_in6m)->in6m_lock)

#define IN6M_ADDREF(_in6m)                                              \
	in6m_addref(_in6m, 0)

#define IN6M_ADDREF_LOCKED(_in6m)                                       \
	in6m_addref(_in6m, 1)

#define IN6M_REMREF(_in6m)                                              \
	in6m_remref(_in6m, 0)

#define IN6M_REMREF_LOCKED(_in6m)                                       \
	in6m_remref(_in6m, 1)

/* flags to in6_update_ifa */
#define IN6_IFAUPDATE_NOWAIT    0x1 /* don't block allocating memory */
#define IN6_IFAUPDATE_1STADDR   0x2 /* first address on interface */
#define IN6_IFAUPDATE_NEWADDR   0x4 /* new address on interface */
#define IN6_IFAUPDATE_DADDELAY  0x8 /* must delay initial DAD probe */

struct ip6_moptions;
struct sockopt;
struct inpcb;

extern LIST_HEAD(in6_multihead, in6_multi) in6_multihead;

/*
 * Structure used by macros below to remember position when stepping through
 * all of the in6_multi records.
 */
struct in6_multistep {
	struct in6_ifaddr *i_ia;
	struct in6_multi *i_in6m;
};

/*
 * Macros for looking up the in6_multi record for a given IP6 multicast
 * address on a given interface. If no matching record is found, "in6m"
 * returns NULL.
 *
 * We do this differently compared other BSD implementations; instead of
 * walking the if_multiaddrs list at the interface and returning the
 * ifma_protospec value of a matching entry, we search the global list
 * of in6_multi records and find it that way.  Otherwise either the two
 * structures (in6_multi, ifmultiaddr) need to be ref counted both ways,
 * which will make things too complicated, or they need to reside in the
 * same protected domain, which they aren't.
 *
 * Must be called with in6_multihead_lock held.
 */
#define IN6_LOOKUP_MULTI(addr, ifp, in6m)                               \
	/* struct in6_addr *addr; */                                    \
	/* struct ifnet *ifp; */                                        \
	/* struct in6_multi *in6m; */                                   \
do {                                                                    \
	struct in6_multistep _step;                                     \
	IN6_FIRST_MULTI(_step, in6m);                                   \
	while ((in6m) != NULL) {                                        \
	        IN6M_LOCK_SPIN(in6m);                                   \
	        if ((in6m)->in6m_ifp == (ifp) &&                        \
	            IN6_ARE_ADDR_EQUAL(&(in6m)->in6m_addr, (addr))) {   \
	                IN6M_ADDREF_LOCKED(in6m);                       \
	                IN6M_UNLOCK(in6m);                              \
	                break;                                          \
	        }                                                       \
	        IN6M_UNLOCK(in6m);                                      \
	        IN6_NEXT_MULTI(_step, in6m);                            \
	}                                                               \
} while (0)

/*
 * Macro to step through all of the in6_multi records, one at a time.
 * The current position is remembered in "step", which the caller must
 * provide.  IN6_FIRST_MULTI(), below, must be called to initialize "step"
 * and get the first record.  Both macros return a NULL "in6m" when there
 * are no remaining records.
 *
 * Must be called with in6_multihead_lock held.
 */
#define IN6_NEXT_MULTI(step, in6m)                                      \
	/* struct in6_multistep step; */                                \
	/* struct in6_multi *in6m; */                                   \
do {                                                                    \
	in6_multihead_lock_assert(LCK_RW_ASSERT_HELD);                  \
	if (((in6m) = (step).i_in6m) != NULL)                           \
	        (step).i_in6m = (step).i_in6m->in6m_entry.le_next;      \
} while (0)

#define IN6_FIRST_MULTI(step, in6m)                                     \
	/* struct in6_multistep step; */                                \
	/* struct in6_multi *in6m */                                    \
do {                                                                    \
	in6_multihead_lock_assert(LCK_RW_ASSERT_HELD);                  \
	(step).i_in6m = in6_multihead.lh_first;                         \
	        IN6_NEXT_MULTI((step), (in6m));                         \
} while (0)

extern lck_mtx_t *inet6_domain_mutex;
extern struct domain *inet6domain;

struct ip6_pktopts;

/* Multicast private KPIs. */
extern int im6o_mc_filter(const struct ip6_moptions *, const struct ifnet *,
    const struct sockaddr_in6 *, const struct sockaddr_in6 *);
extern int in6_mc_join(struct ifnet *, const struct in6_addr *,
    struct in6_mfilter *, struct in6_multi **, int);
extern int in6_mc_leave(struct in6_multi *, struct in6_mfilter *);
extern void in6m_clear_recorded(struct in6_multi *);
extern void in6m_commit(struct in6_multi *);
extern void in6m_purge(struct in6_multi *);
extern void in6m_print(const struct in6_multi *);
extern int in6m_record_source(struct in6_multi *, const struct in6_addr *);
extern int ip6_getmoptions(struct inpcb *, struct sockopt *);
extern int ip6_setmoptions(struct inpcb *, struct sockopt *);
/* Legacy KAME multicast private KPIs. */
extern struct in6_multi_mship *in6_joingroup(struct ifnet *,
    struct in6_addr *, int *, int);
extern int in6_leavegroup(struct in6_multi_mship *);
extern void in6_multi_init(void);
extern void in6m_addref(struct in6_multi *, int);
extern void in6m_remref(struct in6_multi *, int);
extern int in6_multi_detach(struct in6_multi *);
extern int in6_ifindex2scopeid(int);
extern int in6_mask2len(struct in6_addr *, u_char *);
extern void in6_len2mask(struct in6_addr *, int);
extern int in6_control(struct socket *, u_long, caddr_t, struct ifnet *,
    struct proc *);
extern int in6_update_ifa(struct ifnet *, struct in6_aliasreq *, int,
    struct in6_ifaddr **);
extern void in6_purgeaddr(struct ifaddr *);
extern int in6if_do_dad(struct ifnet *);
extern void in6_purgeif(struct ifnet *);
extern void in6_savemkludge(struct in6_ifaddr *);
extern void in6_setmaxmtu(void);
extern void in6_restoremkludge(struct in6_ifaddr *, struct ifnet *);
extern void in6_purgemkludge(struct ifnet *);
extern struct in6_ifaddr *in6ifa_ifpforlinklocal(struct ifnet *, int);
extern struct in6_ifaddr *in6ifa_ifpwithflag(struct ifnet *, int);
extern struct in6_ifaddr *in6ifa_ifpwithaddr(struct ifnet *, struct in6_addr *);
extern struct in6_ifaddr *in6ifa_prproxyaddr(struct in6_addr *);
extern void in6ifa_getlifetime(struct in6_ifaddr *,
    struct in6_addrlifetime *, int);
extern void in6ifa_setlifetime(struct in6_ifaddr *, struct in6_addrlifetime *);
extern char *ip6_sprintf(const struct in6_addr *);
extern int in6_addr2scopeid(struct ifnet *, struct in6_addr *);
extern int in6_matchlen(struct in6_addr *, struct in6_addr *);
extern int in6_are_prefix_equal(struct in6_addr *p1, struct in6_addr *p2,
    int len);
extern void in6_prefixlen2mask(struct in6_addr *maskp, int len);
extern int in6_prefix_add_ifid(int iilen, struct in6_ifaddr *ia);
extern void in6_prefix_remove_ifid(int iilen, struct in6_ifaddr *ia);
extern void in6_purgeprefix(struct ifnet *);
extern void in6_purgeaddrs(struct ifnet *);
extern uint8_t im6s_get_mode(const struct in6_multi *,
    const struct ip6_msource *, uint8_t);
extern void im6f_leave(struct in6_mfilter *);
extern void im6f_purge(struct in6_mfilter *);
extern int in6_embedscope(struct in6_addr *, const struct sockaddr_in6 *,
    struct inpcb *, struct ifnet **, struct ip6_pktopts *);
extern int in6_recoverscope(struct sockaddr_in6 *, const struct in6_addr *,
    struct ifnet *);
extern void in6_aliasreq_64_to_32(struct in6_aliasreq_64 *,
    struct in6_aliasreq_32 *);
extern void in6_aliasreq_32_to_64(struct in6_aliasreq_32 *,
    struct in6_aliasreq_64 *);
extern void in6_ifaddr_init(void);
extern int in6_inithead(void **, int);
extern void in6_rtqdrain(void);
extern struct radix_node *in6_validate(struct radix_node *);
extern int  in6_if2idlen(struct ifnet *);
extern int in6_src_ioctl(u_long, caddr_t);
extern void in6_multihead_lock_exclusive(void);
extern void in6_multihead_lock_shared(void);
extern void in6_multihead_lock_assert(int);
extern void in6_multihead_lock_done(void);

extern void in6_cga_init(void);
extern void in6_cga_node_lock(void);
extern void in6_cga_node_unlock(void);
extern void in6_cga_query(struct in6_cga_nodecfg *);
extern int in6_cga_start(const struct in6_cga_nodecfg *);
extern int in6_cga_stop(void);
extern ssize_t in6_cga_parameters_prepare(void *, size_t,
    const struct in6_addr *, u_int8_t, const struct in6_cga_modifier *);
extern int in6_cga_generate(struct in6_cga_prepare *, u_int8_t,
    struct in6_addr *);
extern int in6_getconninfo(struct socket *, sae_connid_t, uint32_t *,
    uint32_t *, int32_t *, user_addr_t, socklen_t *,
    user_addr_t, socklen_t *, uint32_t *, user_addr_t, uint32_t *);
extern void in6_ip6_to_sockaddr(const struct in6_addr *ip6, u_int16_t port,
    struct sockaddr_in6 *sin6, u_int32_t maxlen);

#endif /* BSD_KERNEL_PRIVATE */
#endif /* _NETINET6_IN6_VAR_H_ */

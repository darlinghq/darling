/*
 * Copyright (c) 2000-2019 Apple Inc. All rights reserved.
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
#define _NETINET_IN_PCB_H_
#include <sys/appleapiopts.h>

#include <sys/types.h>
#include <sys/queue.h>
#ifdef BSD_KERNEL_PRIVATE
#include <sys/bitstring.h>
#include <sys/tree.h>
#include <kern/locks.h>
#include <kern/zalloc.h>
#include <netinet/in_stat.h>
#endif /* BSD_KERNEL_PRIVATE */

#if IPSEC
#include <netinet6/ipsec.h> /* for IPSEC */
#endif /* IPSEC */

#if NECP
#include <net/necp.h>
#endif


#ifdef BSD_KERNEL_PRIVATE
/*
 * struct inpcb is the common protocol control block structure used in most
 * IP transport protocols.
 *
 * Pointers to local and foreign host table entries, local and foreign socket
 * numbers, and pointers up (to a socket structure) and down (to a
 * protocol-specific control block) are stored here.
 */
LIST_HEAD(inpcbhead, inpcb);
LIST_HEAD(inpcbporthead, inpcbport);
#endif /* BSD_KERNEL_PRIVATE */
typedef u_quad_t        inp_gen_t;

/*
 * PCB with AF_INET6 null bind'ed laddr can receive AF_INET input packet.
 * So, AF_INET6 null laddr is also used as AF_INET null laddr, by utilizing
 * the following structure.
 */
struct in_addr_4in6 {
	u_int32_t       ia46_pad32[3];
	struct  in_addr ia46_addr4;
};

#ifdef BSD_KERNEL_PRIVATE
/*
 * NB: the zone allocator is type-stable EXCEPT FOR THE FIRST TWO LONGS
 * of the structure.  Therefore, it is important that the members in
 * that position not contain any information which is required to be
 * stable.
 */
struct  icmp6_filter;
#if CONFIG_MACF_NET
struct  label;
#endif
struct ifnet;

struct inp_stat {
	u_int64_t       rxpackets;
	u_int64_t       rxbytes;
	u_int64_t       txpackets;
	u_int64_t       txbytes;
};

/*
 * struct inpcb captures the network layer state for TCP, UDP and raw IPv6
 * and IPv6 sockets.  In the case of TCP, further per-connection state is
 * hung off of inp_ppcb most of the time.
 */
struct inpcb {
	decl_lck_mtx_data(, inpcb_mtx); /* inpcb per-socket mutex */
	LIST_ENTRY(inpcb) inp_hash;     /* hash list */
	LIST_ENTRY(inpcb) inp_list;     /* list for all PCBs of this proto */
	void    *inp_ppcb;              /* pointer to per-protocol pcb */
	struct inpcbinfo *inp_pcbinfo;  /* PCB list info */
	struct socket *inp_socket;      /* back pointer to socket */
	LIST_ENTRY(inpcb) inp_portlist; /* list for this PCB's local port */
	RB_ENTRY(inpcb) infc_link;      /* link for flowhash RB tree */
	struct inpcbport *inp_phd;      /* head of this list */
	inp_gen_t inp_gencnt;           /* generation count of this instance */
	int     inp_hash_element;       /* array index of pcb's hash list */
	int     inp_wantcnt;            /* wanted count; atomically updated */
	int     inp_state;              /* state (INUSE/CACHED/DEAD) */
	u_short inp_fport;              /* foreign port */
	u_short inp_lport;              /* local port */
	u_int32_t inp_flags;            /* generic IP/datagram flags */
	u_int32_t inp_flags2;           /* generic IP/datagram flags #2 */
	u_int32_t inp_flow;             /* IPv6 flow information */

	u_char  inp_sndinprog_cnt;      /* outstanding send operations */
	uint32_t inp_sndingprog_waiters;/* waiters for outstanding send */
	u_char  inp_vflag;              /* INP_IPV4 or INP_IPV6 */

	u_char inp_ip_ttl;              /* time to live proto */
	u_char inp_ip_p;                /* protocol proto */

	struct ifnet *inp_boundifp;     /* interface for INP_BOUND_IF */
	struct ifnet *inp_last_outifp;  /* last known outgoing interface */
	u_int32_t inp_flowhash;         /* flow hash */

	/* Protocol-dependent part */
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
		struct route inp4_route;
		struct route_in6 inp6_route;
	} inp_dependroute;
	struct {
		/* type of service proto */
		u_char inp4_ip_tos;
		/* IP options */
		struct mbuf *inp4_options;
		/* IP multicast options */
		struct ip_moptions *inp4_moptions;
	} inp_depend4;
	struct {
		/* IP options */
		struct mbuf *inp6_options;
		/* IP6 options for outgoing packets */
		struct  ip6_pktopts *inp6_outputopts;
		/* IP multicast options */
		struct  ip6_moptions *inp6_moptions;
		/* ICMPv6 code type filter */
		struct  icmp6_filter *inp6_icmp6filt;
		/* IPV6_CHECKSUM setsockopt */
		int     inp6_cksum;
		short   inp6_hops;
	} inp_depend6;

	caddr_t inp_saved_ppcb;         /* place to save pointer while cached */
#if CONFIG_MACF_NET
	struct label *inp_label;        /* MAC label */
#endif
#if IPSEC
	struct inpcbpolicy *inp_sp;     /* for IPsec */
#endif /* IPSEC */
#if NECP
	struct {
		char *inp_domain;
		char *inp_account;
	} inp_necp_attributes;
	struct necp_inpcb_result inp_policyresult;
	uuid_t necp_client_uuid;
	necp_client_flow_cb necp_cb;
#endif
	u_char *inp_keepalive_data;     /* for keepalive offload */
	u_int8_t inp_keepalive_datalen; /* keepalive data length */
	u_int8_t inp_keepalive_type;    /* type of application */
	u_int16_t inp_keepalive_interval; /* keepalive interval */
	uint32_t inp_nstat_refcnt __attribute__((aligned(4)));
	struct inp_stat *inp_stat;
	struct inp_stat *inp_cstat;     /* cellular data */
	struct inp_stat *inp_wstat;     /* Wi-Fi data */
	struct inp_stat *inp_Wstat;     /* Wired data */
	u_int8_t inp_stat_store[sizeof(struct inp_stat) + sizeof(u_int64_t)];
	u_int8_t inp_cstat_store[sizeof(struct inp_stat) + sizeof(u_int64_t)];
	u_int8_t inp_wstat_store[sizeof(struct inp_stat) + sizeof(u_int64_t)];
	u_int8_t inp_Wstat_store[sizeof(struct inp_stat) + sizeof(u_int64_t)];
	activity_bitmap_t inp_nw_activity;
	u_int64_t inp_start_timestamp;

	char inp_last_proc_name[MAXCOMLEN + 1];
	char inp_e_proc_name[MAXCOMLEN + 1];
};

#define INP_ADD_STAT(_inp, _cnt_cellular, _cnt_wifi, _cnt_wired, _a, _n) \
do {                                                                    \
	locked_add_64(&((_inp)->inp_stat->_a), (_n));                   \
	if (_cnt_cellular)                                              \
	        locked_add_64(&((_inp)->inp_cstat->_a), (_n));          \
	if (_cnt_wifi)                                                  \
	        locked_add_64(&((_inp)->inp_wstat->_a), (_n));          \
	if (_cnt_wired)                                                 \
	        locked_add_64(&((_inp)->inp_Wstat->_a), (_n));          \
} while (0);
#endif /* BSD_KERNEL_PRIVATE */

/*
 * Interface exported to userland by various protocols which use
 * inpcbs.  Hack alert -- only define if struct xsocket is in scope.
 */
#pragma pack(4)

#if defined(__LP64__)
struct _inpcb_list_entry {
	u_int32_t   le_next;
	u_int32_t   le_prev;
};
#define _INPCB_PTR(x)           u_int32_t
#define _INPCB_LIST_ENTRY(x)    struct _inpcb_list_entry
#else /* !__LP64__ */
#define _INPCB_PTR(x)           x
#define _INPCB_LIST_ENTRY(x)    LIST_ENTRY(x)
#endif /* !__LP64__ */

#ifdef XNU_KERNEL_PRIVATE
/*
 * This is a copy of the inpcb as it shipped in Panther. This structure
 * is filled out in a copy function. This allows the inpcb to change
 * without breaking userland tools.
 *
 * CAUTION: Many fields may not be filled out. Fewer may be filled out
 * in the future. Code defensively.
 */
struct inpcb_compat {
#else
struct inpcbinfo;
struct inpcbport;
struct mbuf;
struct ip6_pktopts;
struct ip6_moptions;
struct icmp6_filter;
struct inpcbpolicy;

struct inpcb {
#endif /* KERNEL_PRIVATE */
	_INPCB_LIST_ENTRY(inpcb) inp_hash;      /* hash list */
	struct in_addr reserved1;               /* reserved */
	struct in_addr reserved2;               /* reserved */
	u_short inp_fport;                      /* foreign port */
	u_short inp_lport;                      /* local port */
	_INPCB_LIST_ENTRY(inpcb) inp_list;      /* list for all peer PCBs */
	_INPCB_PTR(caddr_t) inp_ppcb;           /* per-protocol pcb */
	_INPCB_PTR(struct inpcbinfo *) inp_pcbinfo;     /* PCB list info */
	_INPCB_PTR(void *) inp_socket;  /* back pointer to socket */
	u_char nat_owner;               /* Used to NAT TCP/UDP traffic */
	u_int32_t nat_cookie;           /* Cookie stored and returned to NAT */
	_INPCB_LIST_ENTRY(inpcb) inp_portlist;  /* this PCB's local port list */
	_INPCB_PTR(struct inpcbport *) inp_phd; /* head of this list */
	inp_gen_t inp_gencnt;           /* generation count of this instance */
	int inp_flags;                  /* generic IP/datagram flags */
	u_int32_t inp_flow;

	u_char inp_vflag;

	u_char inp_ip_ttl;              /* time to live proto */
	u_char inp_ip_p;                /* protocol proto */
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
		int     inp6_cksum;
		u_short inp6_ifindex;
		short   inp6_hops;
	} inp_depend6;

	int hash_element;               /* Array index of pcb's hash list */
	_INPCB_PTR(caddr_t) inp_saved_ppcb; /* pointer while cached */
	_INPCB_PTR(struct inpcbpolicy *) inp_sp;
	u_int32_t       reserved[3];    /* reserved */
};

struct  xinpcb {
	u_int32_t       xi_len;         /* length of this structure */
#ifdef XNU_KERNEL_PRIVATE
	struct  inpcb_compat xi_inp;
#else
	struct  inpcb xi_inp;
#endif
	struct  xsocket xi_socket;
	u_quad_t        xi_alignment_hack;
};

#if !CONFIG_EMBEDDED
struct inpcb64_list_entry {
	u_int64_t   le_next;
	u_int64_t   le_prev;
};

struct  xinpcb64 {
	u_int64_t       xi_len;         /* length of this structure */
	u_int64_t       xi_inpp;
	u_short         inp_fport;      /* foreign port */
	u_short         inp_lport;      /* local port */
	struct inpcb64_list_entry inp_list; /* list for all PCBs */
	u_int64_t       inp_ppcb;       /* ptr to per-protocol PCB */
	u_int64_t       inp_pcbinfo;    /* PCB list info */
	struct inpcb64_list_entry inp_portlist; /* this PCB's local port list */
	u_int64_t       inp_phd;        /* head of this list */
	inp_gen_t       inp_gencnt;     /* current generation count */
	int             inp_flags;      /* generic IP/datagram flags */
	u_int32_t       inp_flow;
	u_char          inp_vflag;
	u_char          inp_ip_ttl;     /* time to live */
	u_char          inp_ip_p;       /* protocol */
	union {                         /* foreign host table entry */
		struct  in_addr_4in6    inp46_foreign;
		struct  in6_addr        inp6_foreign;
	} inp_dependfaddr;
	union {                         /* local host table entry */
		struct  in_addr_4in6    inp46_local;
		struct  in6_addr        inp6_local;
	} inp_dependladdr;
	struct {
		u_char  inp4_ip_tos;    /* type of service */
	} inp_depend4;
	struct {
		u_int8_t inp6_hlim;
		int     inp6_cksum;
		u_short inp6_ifindex;
		short   inp6_hops;
	} inp_depend6;
	struct  xsocket64 xi_socket;
	u_quad_t        xi_alignment_hack;
};
#endif /* !CONFIG_EMBEDDED */

#ifdef PRIVATE
struct xinpcb_list_entry {
	u_int64_t   le_next;
	u_int64_t   le_prev;
};

struct  xinpcb_n {
	u_int32_t       xi_len;         /* length of this structure */
	u_int32_t       xi_kind;        /* XSO_INPCB */
	u_int64_t       xi_inpp;
	u_short         inp_fport;      /* foreign port */
	u_short         inp_lport;      /* local port */
	u_int64_t       inp_ppcb;       /* pointer to per-protocol pcb */
	inp_gen_t       inp_gencnt;     /* generation count of this instance */
	int             inp_flags;      /* generic IP/datagram flags */
	u_int32_t       inp_flow;
	u_char          inp_vflag;
	u_char          inp_ip_ttl;     /* time to live */
	u_char          inp_ip_p;       /* protocol */
	union {                         /* foreign host table entry */
		struct in_addr_4in6     inp46_foreign;
		struct in6_addr         inp6_foreign;
	} inp_dependfaddr;
	union {                         /* local host table entry */
		struct in_addr_4in6     inp46_local;
		struct in6_addr         inp6_local;
	} inp_dependladdr;
	struct {
		u_char  inp4_ip_tos;    /* type of service */
	} inp_depend4;
	struct {
		u_int8_t inp6_hlim;
		int     inp6_cksum;
		u_short inp6_ifindex;
		short   inp6_hops;
	} inp_depend6;
	u_int32_t               inp_flowhash;
	u_int32_t       inp_flags2;
};
#endif /* PRIVATE */

struct  xinpgen {
	u_int32_t       xig_len;        /* length of this structure */
	u_int           xig_count;      /* number of PCBs at this time */
	inp_gen_t       xig_gen;        /* generation count at this time */
	so_gen_t        xig_sogen;      /* current socket generation count */
};

#pragma pack()

/*
 * These defines are for use with the inpcb.
 */
#define INP_IPV4        0x1
#define INP_IPV6        0x2
#define inp_faddr       inp_dependfaddr.inp46_foreign.ia46_addr4
#define inp_laddr       inp_dependladdr.inp46_local.ia46_addr4
#define in6p_faddr      inp_dependfaddr.inp6_foreign
#define in6p_laddr      inp_dependladdr.inp6_local

#ifdef BSD_KERNEL_PRIVATE
#define inp_route       inp_dependroute.inp4_route
#define inp_ip_tos      inp_depend4.inp4_ip_tos
#define inp_options     inp_depend4.inp4_options
#define inp_moptions    inp_depend4.inp4_moptions
#define in6p_route      inp_dependroute.inp6_route
#define in6p_ip6_hlim   inp_depend6.inp6_hlim
#define in6p_hops       inp_depend6.inp6_hops   /* default hop limit */
#define in6p_ip6_nxt    inp_ip_p
#define in6p_vflag      inp_vflag
#define in6p_options    inp_depend6.inp6_options
#define in6p_outputopts inp_depend6.inp6_outputopts
#define in6p_moptions   inp_depend6.inp6_moptions
#define in6p_icmp6filt  inp_depend6.inp6_icmp6filt
#define in6p_cksum      inp_depend6.inp6_cksum
#define in6p_ifindex    inp_depend6.inp6_ifindex
#define in6p_flags      inp_flags
#define in6p_flags2     inp_flags2
#define in6p_socket     inp_socket
#define in6p_lport      inp_lport
#define in6p_fport      inp_fport
#define in6p_ppcb       inp_ppcb
#define in6p_state      inp_state
#define in6p_wantcnt    inp_wantcnt
#define in6p_last_outifp inp_last_outifp
#define in6pcb          inpcb
#if IPSEC
#define in6p_sp         inp_sp
#endif /* IPSEC */
#define INP_INC_IFNET_STAT(_inp_, _stat_) { \
	if ((_inp_)->inp_last_outifp != NULL) { \
	        if ((_inp_)->inp_vflag & INP_IPV6) { \
	                (_inp_)->inp_last_outifp->if_ipv6_stat->_stat_++;\
	        } else { \
	                (_inp_)->inp_last_outifp->if_ipv4_stat->_stat_++;\
	        }\
	}\
}

struct inpcbport {
	LIST_ENTRY(inpcbport) phd_hash;
	struct inpcbhead phd_pcblist;
	u_short phd_port;
};

struct intimercount {
	u_int32_t intimer_lazy; /* lazy requests for timer scheduling */
	u_int32_t intimer_fast; /* fast requests, can be coalesced */
	u_int32_t intimer_nodelay; /* fast requests, never coalesced */
};

typedef void (*inpcb_timer_func_t)(struct inpcbinfo *);

/*
 * Global data structure for each high-level protocol (UDP, TCP, ...) in both
 * IPv4 and IPv6.  Holds inpcb lists and information for managing them.  Each
 * pcbinfo is protected by a RW lock: ipi_lock.
 *
 * All INPCB pcbinfo entries are linked together via ipi_entry.
 */
struct inpcbinfo {
	/*
	 * Glue to all PCB infos, as well as garbage collector and
	 * timer callbacks, protected by inpcb_lock.  Callout request
	 * counts are atomically updated.
	 */
	TAILQ_ENTRY(inpcbinfo)  ipi_entry;
	inpcb_timer_func_t      ipi_gc;
	inpcb_timer_func_t      ipi_timer;
	struct intimercount     ipi_gc_req;
	struct intimercount     ipi_timer_req;

	/*
	 * Per-protocol lock protecting pcb list, pcb count, etc.
	 */
	lck_rw_t                *ipi_lock;

	/*
	 * List and count of pcbs on the protocol.
	 */
	struct inpcbhead        *ipi_listhead;
	uint32_t                ipi_count;

	/*
	 * Count of pcbs marked with INP2_TIMEWAIT flag.
	 */
	uint32_t                ipi_twcount;

	/*
	 * Generation count -- incremented each time a connection is
	 * allocated or freed.
	 */
	uint64_t                ipi_gencnt;

	/*
	 * Fields associated with port lookup and allocation.
	 */
	uint16_t                ipi_lastport;
	uint16_t                ipi_lastlow;
	uint16_t                ipi_lasthi;

	/*
	 * Zone from which inpcbs are allocated for this protocol.
	 */
	struct zone             *ipi_zone;

	/*
	 * Per-protocol hash of pcbs, hashed by local and foreign
	 * addresses and port numbers.
	 */
	struct inpcbhead        *ipi_hashbase;
	u_long                  ipi_hashmask;

	/*
	 * Per-protocol hash of pcbs, hashed by only local port number.
	 */
	struct inpcbporthead    *ipi_porthashbase;
	u_long                  ipi_porthashmask;

	/*
	 * Misc.
	 */
	lck_attr_t              *ipi_lock_attr;
	lck_grp_t               *ipi_lock_grp;
	lck_grp_attr_t          *ipi_lock_grp_attr;

#define INPCBINFO_UPDATE_MSS    0x1
#define INPCBINFO_HANDLE_LQM_ABORT      0x2
	u_int32_t               ipi_flags;
};

#define INP_PCBHASH(faddr, lport, fport, mask) \
	(((faddr) ^ ((faddr) >> 16) ^ ntohs((lport) ^ (fport))) & (mask))
#define INP_PCBPORTHASH(lport, mask) \
	(ntohs((lport)) & (mask))

#define INP_IS_FLOW_CONTROLLED(_inp_) \
	((_inp_)->inp_flags & INP_FLOW_CONTROLLED)
#define INP_IS_FLOW_SUSPENDED(_inp_) \
	(((_inp_)->inp_flags & INP_FLOW_SUSPENDED) ||   \
	((_inp_)->inp_socket->so_flags & SOF_SUSPENDED))
#define INP_WAIT_FOR_IF_FEEDBACK(_inp_) \
	(((_inp_)->inp_flags & (INP_FLOW_CONTROLLED | INP_FLOW_SUSPENDED)) != 0)

#define INP_NO_CELLULAR(_inp) \
	((_inp)->inp_flags & INP_NO_IFT_CELLULAR)
#define INP_NO_EXPENSIVE(_inp) \
	((_inp)->inp_flags2 & INP2_NO_IFF_EXPENSIVE)
#define INP_NO_CONSTRAINED(_inp) \
	((_inp)->inp_flags2 & INP2_NO_IFF_CONSTRAINED)
#define INP_AWDL_UNRESTRICTED(_inp) \
	((_inp)->inp_flags2 & INP2_AWDL_UNRESTRICTED)
#define INP_INTCOPROC_ALLOWED(_inp) \
	((_inp)->inp_flags2 & INP2_INTCOPROC_ALLOWED)

#endif /* BSD_KERNEL_PRIVATE */

/*
 * Flags for inp_flags.
 *
 * Some of these are publicly defined for legacy reasons, as they are
 * (unfortunately) used by certain applications to determine, at compile
 * time, whether or not the OS supports certain features.
 */
#ifdef BSD_KERNEL_PRIVATE
#define INP_RECVOPTS            0x00000001 /* receive incoming IP options */
#define INP_RECVRETOPTS         0x00000002 /* receive IP options for reply */
#define INP_RECVDSTADDR         0x00000004 /* receive IP dst address */
#define INP_HDRINCL             0x00000008 /* user supplies entire IP header */
#define INP_HIGHPORT            0x00000010 /* user wants "high" port binding */
#define INP_LOWPORT             0x00000020 /* user wants "low" port binding */
#endif /* BSD_KERNEL_PRIVATE */

#define INP_ANONPORT            0x00000040 /* port chosen for user */

#ifdef BSD_KERNEL_PRIVATE
#define INP_RECVIF              0x00000080 /* receive incoming interface */
#define INP_MTUDISC             0x00000100 /* unused */
#define INP_STRIPHDR            0x00000200 /* strip hdrs in raw_ip (for OT) */
#define INP_RECV_ANYIF          0x00000400 /* don't restrict inbound iface */
#define INP_INADDR_ANY          0x00000800 /* local address wasn't specified */
#define INP_IN6ADDR_ANY         INP_INADDR_ANY
#define INP_RECVTTL             0x00001000 /* receive incoming IP TTL */
#define INP_UDP_NOCKSUM         0x00002000 /* turn off outbound UDP checksum */
#define INP_BOUND_IF            0x00004000 /* bind socket to an interface */
#endif /* BSD_KERNEL_PRIVATE */

#define IN6P_IPV6_V6ONLY        0x00008000 /* restrict AF_INET6 socket for v6 */

#ifdef BSD_KERNEL_PRIVATE
#define IN6P_PKTINFO            0x00010000 /* receive IP6 dst and I/F */
#define IN6P_HOPLIMIT           0x00020000 /* receive hoplimit */
#define IN6P_HOPOPTS            0x00040000 /* receive hop-by-hop options */
#define IN6P_DSTOPTS            0x00080000 /* receive dst options after rthdr */
#define IN6P_RTHDR              0x00100000 /* receive routing header */
#define IN6P_RTHDRDSTOPTS       0x00200000 /* receive dstoptions before rthdr */
#define IN6P_TCLASS             0x00400000 /* receive traffic class value */
#define INP_RECVTOS             IN6P_TCLASS     /* receive incoming IP TOS */
#define IN6P_AUTOFLOWLABEL      0x00800000 /* attach flowlabel automatically */
#endif /* BSD_KERNEL_PRIVATE */

#define IN6P_BINDV6ONLY         0x01000000 /* do not grab IPv4 traffic */

#ifdef BSD_KERNEL_PRIVATE
#define IN6P_RFC2292            0x02000000 /* used RFC2292 API on the socket */
#define IN6P_MTU                0x04000000 /* receive path MTU */
#define INP_PKTINFO             0x08000000 /* rcv and snd PKTINFO for IPv4 */
#define INP_FLOW_SUSPENDED      0x10000000 /* flow suspended */
#define INP_NO_IFT_CELLULAR     0x20000000 /* do not use cellular interface */
#define INP_FLOW_CONTROLLED     0x40000000 /* flow controlled */
#define INP_FC_FEEDBACK         0x80000000 /* got interface flow adv feedback */

#define INP_CONTROLOPTS \
	(INP_RECVOPTS|INP_RECVRETOPTS|INP_RECVDSTADDR|INP_RECVIF|INP_RECVTTL| \
	INP_PKTINFO|IN6P_PKTINFO|IN6P_HOPLIMIT|IN6P_HOPOPTS|IN6P_DSTOPTS| \
	IN6P_RTHDR|IN6P_RTHDRDSTOPTS|IN6P_TCLASS|IN6P_RFC2292|IN6P_MTU)

#define INP_UNMAPPABLEOPTS \
	(IN6P_HOPOPTS|IN6P_DSTOPTS|IN6P_RTHDR|IN6P_AUTOFLOWLABEL)

/*
 * Flags for inp_flags2.
 *
 * Overflowed INP flags; use INP2 prefix to avoid misuse.
 */
#define INP2_TIMEWAIT           0x00000001 /* in TIMEWAIT */
#define INP2_IN_FCTREE          0x00000002 /* in inp_fc_tree */
#define INP2_WANT_APP_POLICY    0x00000004 /* necp app policy check is desired */
#define INP2_NO_IFF_EXPENSIVE   0x00000008 /* do not use expensive interface */
#define INP2_INHASHLIST         0x00000010 /* pcb is in inp_hash list */
#define INP2_AWDL_UNRESTRICTED  0x00000020 /* AWDL restricted mode allowed */
#define INP2_KEEPALIVE_OFFLOAD  0x00000040 /* Enable UDP or TCP keepalive offload */
#define INP2_INTCOPROC_ALLOWED  0x00000080 /* Allow communication via internal co-processor interfaces */
#define INP2_CONNECT_IN_PROGRESS        0x00000100 /* A connect call is in progress, so binds are intermediate steps */
#define INP2_CLAT46_FLOW        0x00000200 /* The flow is going to use CLAT46 path */
#define INP2_EXTERNAL_PORT      0x00000400 /* The port is registered externally, for NECP listeners */
#define INP2_NO_IFF_CONSTRAINED 0x00000800 /* do not use constrained interface */

/*
 * Flags passed to in_pcblookup*() functions.
 */
#define INPLOOKUP_WILDCARD      1

#define sotoinpcb(so)   ((struct inpcb *)(so)->so_pcb)
#define sotoin6pcb(so)  sotoinpcb(so)

struct sysctl_req;

extern int ipport_lowfirstauto;
extern int ipport_lowlastauto;
extern int ipport_firstauto;
extern int ipport_lastauto;
extern int ipport_hifirstauto;
extern int ipport_hilastauto;

/* freshly allocated PCB, it's in use */
#define INPCB_STATE_INUSE       0x1
/* this pcb is sitting in a a cache */
#define INPCB_STATE_CACHED      0x2
/* should treat as gone, will be garbage collected and freed */
#define INPCB_STATE_DEAD        0x3

/* marked as ready to be garbaged collected, should be treated as not found */
#define WNT_STOPUSING           0xffff
/* that pcb is being acquired, do not recycle this time */
#define WNT_ACQUIRE             0x1
/* release acquired mode, can be garbage collected when wantcnt is null */
#define WNT_RELEASE             0x2

extern void in_pcbinit(void);
extern void in_pcbinfo_attach(struct inpcbinfo *);
extern int in_pcbinfo_detach(struct inpcbinfo *);

/* type of timer to be scheduled by inpcb_gc_sched and inpcb_timer_sched */
enum {
	INPCB_TIMER_LAZY = 0x1,
	INPCB_TIMER_FAST,
	INPCB_TIMER_NODELAY
};
extern void inpcb_gc_sched(struct inpcbinfo *, u_int32_t type);
extern void inpcb_timer_sched(struct inpcbinfo *, u_int32_t type);

extern void in_losing(struct inpcb *);
extern void in_rtchange(struct inpcb *, int);
extern int in_pcballoc(struct socket *, struct inpcbinfo *, struct proc *);
extern int in_pcbbind(struct inpcb *, struct sockaddr *, struct proc *);
extern int in_pcbconnect(struct inpcb *, struct sockaddr *, struct proc *,
    unsigned int, struct ifnet **);
extern void in_pcbdetach(struct inpcb *);
extern void in_pcbdispose(struct inpcb *);
extern void in_pcbdisconnect(struct inpcb *);
extern int in_pcbinshash(struct inpcb *, int);
extern int in_pcbladdr(struct inpcb *, struct sockaddr *, struct in_addr *,
    unsigned int, struct ifnet **, int);
extern struct inpcb *in_pcblookup_local(struct inpcbinfo *, struct in_addr,
    u_int, int);
extern struct inpcb *in_pcblookup_local_and_cleanup(struct inpcbinfo *,
    struct in_addr, u_int, int);
extern struct inpcb *in_pcblookup_hash(struct inpcbinfo *, struct in_addr,
    u_int, struct in_addr, u_int, int, struct ifnet *);
extern int in_pcblookup_hash_exists(struct inpcbinfo *, struct in_addr,
    u_int, struct in_addr, u_int, int, uid_t *, gid_t *, struct ifnet *);
extern void in_pcbnotifyall(struct inpcbinfo *, struct in_addr, int,
    void (*)(struct inpcb *, int));
extern void in_pcbrehash(struct inpcb *);
extern int in_getpeeraddr(struct socket *, struct sockaddr **);
extern int in_getsockaddr(struct socket *, struct sockaddr **);
extern int in_getsockaddr_s(struct socket *, struct sockaddr_in *);
extern int in_pcb_checkstate(struct inpcb *, int, int);
extern void in_pcbremlists(struct inpcb *);
extern void inpcb_to_compat(struct inpcb *, struct inpcb_compat *);
#if !CONFIG_EMBEDDED
extern void inpcb_to_xinpcb64(struct inpcb *, struct xinpcb64 *);
#endif

extern int get_pcblist_n(short, struct sysctl_req *, struct inpcbinfo *);

extern void inpcb_get_ports_used(u_int32_t, int, u_int32_t, bitstr_t *,
    struct inpcbinfo *);
#define INPCB_OPPORTUNISTIC_THROTTLEON  0x0001
#define INPCB_OPPORTUNISTIC_SETCMD      0x0002
extern uint32_t inpcb_count_opportunistic(unsigned int, struct inpcbinfo *,
    u_int32_t);
extern uint32_t inpcb_find_anypcb_byaddr(struct ifaddr *, struct inpcbinfo *);
extern void inp_route_copyout(struct inpcb *, struct route *);
extern void inp_route_copyin(struct inpcb *, struct route *);
extern int inp_bindif(struct inpcb *, unsigned int, struct ifnet **);
extern void inp_set_nocellular(struct inpcb *);
extern void inp_clear_nocellular(struct inpcb *);
extern void inp_set_noexpensive(struct inpcb *);
extern void inp_set_noconstrained(struct inpcb *);
extern void inp_set_awdl_unrestricted(struct inpcb *);
extern boolean_t inp_get_awdl_unrestricted(struct inpcb *);
extern void inp_clear_awdl_unrestricted(struct inpcb *);
extern void inp_set_intcoproc_allowed(struct inpcb *);
extern boolean_t inp_get_intcoproc_allowed(struct inpcb *);
extern void inp_clear_intcoproc_allowed(struct inpcb *);
#if NECP
extern void inp_update_necp_policy(struct inpcb *, struct sockaddr *, struct sockaddr *, u_int);
extern void inp_set_want_app_policy(struct inpcb *);
extern void inp_clear_want_app_policy(struct inpcb *);
#endif /* NECP */
extern u_int32_t inp_calc_flowhash(struct inpcb *);
extern void inp_reset_fc_state(struct inpcb *);
extern int inp_set_fc_state(struct inpcb *, int advcode);
extern void inp_fc_unthrottle_tcp(struct inpcb *);
extern void inp_flowadv(uint32_t);
extern int inp_flush(struct inpcb *, int);
extern int inp_findinpcb_procinfo(struct inpcbinfo *, uint32_t, struct so_procinfo *);
extern void inp_get_soprocinfo(struct inpcb *, struct so_procinfo *);
extern int inp_update_policy(struct inpcb *);
extern boolean_t inp_restricted_recv(struct inpcb *, struct ifnet *);
extern boolean_t inp_restricted_send(struct inpcb *, struct ifnet *);
extern void inp_incr_sndbytes_total(struct socket *, int);
extern void inp_decr_sndbytes_total(struct socket *, int);
extern void inp_count_sndbytes(struct inpcb *, u_int32_t);
extern void inp_incr_sndbytes_unsent(struct socket *, int32_t);
extern void inp_decr_sndbytes_unsent(struct socket *, int32_t);
extern int32_t inp_get_sndbytes_allunsent(struct socket *, u_int32_t);
extern void inp_decr_sndbytes_allunsent(struct socket *, u_int32_t);
extern void inp_set_activity_bitmap(struct inpcb *inp);
extern void inp_get_activity_bitmap(struct inpcb *inp, activity_bitmap_t *b);
extern void inp_update_last_owner(struct socket *so, struct proc *p, struct proc *ep);
extern void inp_copy_last_owner(struct socket *so, struct socket *head);
#endif /* BSD_KERNEL_PRIVATE */
#ifdef KERNEL_PRIVATE
/* exported for PPP */
extern void inp_clear_INP_INADDR_ANY(struct socket *);
#endif /* KERNEL_PRIVATE */
#endif /* !_NETINET_IN_PCB_H_ */

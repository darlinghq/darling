/*
 * Copyright (c) 1999-2019 Apple Inc. All rights reserved.
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
/* Copyright (c) 1998, 1999 Apple Computer, Inc. All Rights Reserved */
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*
 * Mach Operating System
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * Copyright (c) 1994 NeXT Computer, Inc. All rights reserved.
 *
 * Copyright (c) 1982, 1986, 1988 Regents of the University of California.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
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
 *	@(#)mbuf.h	8.3 (Berkeley) 1/21/94
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2005 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

#ifndef _SYS_MBUF_H_
#define _SYS_MBUF_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>
#include <sys/_types/_u_int32_t.h> /* u_int32_t */
#include <sys/_types/_u_int64_t.h> /* u_int64_t */
#include <sys/_types/_u_short.h> /* u_short */

#ifdef KERNEL
#include <sys/kpi_mbuf.h>
#endif

#ifdef XNU_KERNEL_PRIVATE
#include <sys/lock.h>
#include <sys/queue.h>
#include <machine/endian.h>
/*
 * Mbufs are of a single size, MSIZE (machine/param.h), which
 * includes overhead.  An mbuf may add a single "mbuf cluster" of size
 * MCLBYTES/MBIGCLBYTES/M16KCLBYTES (also in machine/param.h), which has
 * no additional overhead and is used instead of the internal data area;
 * this is done when at least MINCLSIZE of data must be stored.
 */

/*
 * The following _MLEN and _MHLEN macros are private to xnu.  Private code
 * that are outside of xnu must use the mbuf_get_{mlen,mhlen} routines since
 * the sizes of the structures are dependent upon specific xnu configs.
 */
#define _MLEN           (MSIZE - sizeof(struct m_hdr))  /* normal data len */
#define _MHLEN          (_MLEN - sizeof(struct pkthdr)) /* data len w/pkthdr */

#define NMBPGSHIFT      (PAGE_SHIFT - MSIZESHIFT)
#define NMBPG           (1 << NMBPGSHIFT)       /* # of mbufs per page */

#define NCLPGSHIFT      (PAGE_SHIFT - MCLSHIFT)
#define NCLPG           (1 << NCLPGSHIFT)       /* # of cl per page */

#define NBCLPGSHIFT     (PAGE_SHIFT - MBIGCLSHIFT)
#define NBCLPG          (1 << NBCLPGSHIFT)      /* # of big cl per page */

#define NMBPCLSHIFT     (MCLSHIFT - MSIZESHIFT)
#define NMBPCL          (1 << NMBPCLSHIFT)      /* # of mbufs per cl */

#define NCLPJCLSHIFT    (M16KCLSHIFT - MCLSHIFT)
#define NCLPJCL         (1 << NCLPJCLSHIFT)     /* # of cl per jumbo cl */

#define NCLPBGSHIFT     (MBIGCLSHIFT - MCLSHIFT)
#define NCLPBG          (1 << NCLPBGSHIFT)      /* # of cl per big cl */

#define NMBPBGSHIFT     (MBIGCLSHIFT - MSIZESHIFT)
#define NMBPBG          (1 << NMBPBGSHIFT)      /* # of mbufs per big cl */

/*
 * Macros for type conversion
 * mtod(m,t) -	convert mbuf pointer to data pointer of correct type
 * mtodo(m, o) -- Same as above but with offset 'o' into data.
 * dtom(x) -	convert data pointer within mbuf to mbuf pointer (XXX)
 */
#define mtod(m, t)      ((t)m_mtod(m))
#define mtodo(m, o)     ((void *)(mtod(m, uint8_t *) + (o)))
#define dtom(x)         m_dtom(x)

/* header at beginning of each mbuf: */
struct m_hdr {
	struct mbuf     *mh_next;       /* next buffer in chain */
	struct mbuf     *mh_nextpkt;    /* next chain in queue/record */
	caddr_t         mh_data;        /* location of data */
	int32_t         mh_len;         /* amount of data in this mbuf */
	u_int16_t       mh_type;        /* type of data in this mbuf */
	u_int16_t       mh_flags;       /* flags; see below */
#if __arm__ && (__BIGGEST_ALIGNMENT__ > 4)
/* This is needed because of how _MLEN is defined and used. Ideally, _MLEN
 * should be defined using the offsetof(struct mbuf, M_dat), since there is
 * no guarantee that mbuf.M_dat will start where mbuf.m_hdr ends. The compiler
 * may (and does in the armv7k case) insert padding between m_hdr and M_dat in
 * mbuf. We cannot easily use offsetof, however, since _MLEN is referenced
 * in the definition of mbuf.
 */
} __attribute__((aligned(8)));
#else
};
#endif

/*
 * Packet tag structure (see below for details).
 */
struct m_tag {
	u_int64_t               m_tag_cookie;   /* Error checking */
#ifndef __LP64__
	u_int32_t               pad;            /* For structure alignment */
#endif /* !__LP64__ */
	SLIST_ENTRY(m_tag)      m_tag_link;     /* List of packet tags */
	u_int16_t               m_tag_type;     /* Module specific type */
	u_int16_t               m_tag_len;      /* Length of data */
	u_int32_t               m_tag_id;       /* Module ID */
};

#define M_TAG_ALIGN(len) \
	(P2ROUNDUP(len, sizeof (u_int64_t)) + sizeof (struct m_tag))

#define M_TAG_VALID_PATTERN     0xfeedfacefeedfaceULL
#define M_TAG_FREE_PATTERN      0xdeadbeefdeadbeefULL

/*
 * Packet tag header structure (at the top of mbuf).  Pointers are
 * 32-bit in ILP32; m_tag needs 64-bit alignment, hence padded.
 */
struct m_taghdr {
#ifndef __LP64__
	u_int32_t               pad;            /* For structure alignment */
#endif /* !__LP64__ */
	u_int64_t               refcnt;         /* Number of tags in this mbuf */
};

/*
 * Driver auxiliary metadata tag (KERNEL_TAG_TYPE_DRVAUX).
 */
struct m_drvaux_tag {
	u_int32_t       da_family;      /* IFNET_FAMILY values */
	u_int32_t       da_subfamily;   /* IFNET_SUBFAMILY values */
	u_int32_t       da_reserved;    /* for future */
	u_int32_t       da_length;      /* length of following data */
};

/* Values for pftag_flags (16-bit wide) */
#define PF_TAG_GENERATED                0x1     /* pkt generated by PF */
#define PF_TAG_FRAGCACHE                0x2
#define PF_TAG_TRANSLATE_LOCALHOST      0x4
#if PF_ECN
#define PF_TAG_HDR_INET                 0x8     /* hdr points to IPv4 */
#define PF_TAG_HDR_INET6                0x10    /* hdr points to IPv6 */
#endif /* PF_ECN */
#define PF_TAG_REASSEMBLED              0x20    /* pkt reassembled by PF */
#define PF_TAG_REFRAGMENTED             0x40    /* pkt refragmented by PF */
/*
 * PF mbuf tag
 */
struct pf_mtag {
	u_int16_t       pftag_flags;    /* PF_TAG flags */
	u_int16_t       pftag_rtableid; /* alternate routing table id */
	u_int16_t       pftag_tag;
	u_int16_t       pftag_routed;
#if PF_ECN
	void            *pftag_hdr;     /* saved hdr pos in mbuf, for ECN */
#endif /* PF_ECN */
};

/*
 * PF fragment tag
 */
struct pf_fragment_tag {
	uint32_t ft_id;     /* fragment id */
	uint16_t ft_hdrlen; /* header length of reassembled pkt */
	uint16_t ft_unfragpartlen; /* length of the per-fragment headers */
	uint16_t ft_extoff; /* last extension header offset or 0 */
	uint16_t ft_maxlen; /* maximum fragment payload length */
};

/*
 * TCP mbuf tag
 */
struct tcp_pktinfo {
	union {
		struct {
			u_int32_t segsz;        /* segment size (actual MSS) */
			u_int32_t start_seq;    /* start seq of this packet */
			pid_t     pid;
			pid_t     e_pid;
		} __tx;
		struct {
			u_int16_t lro_pktlen;   /* max seg size encountered */
			u_int8_t  lro_npkts;    /* # of coalesced TCP pkts */
			u_int8_t  lro_timediff; /* time spent in LRO */
		} __rx;
	} __offload;
	union {
		u_int32_t       pri;            /* send msg priority */
		u_int32_t       seq;            /* recv msg sequence # */
	} __msgattr;
#define tso_segsz       proto_mtag.__pr_u.tcp.tm_tcp.__offload.__tx.segsz
#define tx_start_seq    proto_mtag.__pr_u.tcp.tm_tcp.__offload.__tx.start_seq
#define tx_tcp_pid      proto_mtag.__pr_u.tcp.tm_tcp.__offload.__tx.pid
#define tx_tcp_e_pid    proto_mtag.__pr_u.tcp.tm_tcp.__offload.__tx.e_pid
#define lro_pktlen      proto_mtag.__pr_u.tcp.tm_tcp.__offload.__rx.lro_pktlen
#define lro_npkts       proto_mtag.__pr_u.tcp.tm_tcp.__offload.__rx.lro_npkts
#define lro_elapsed     proto_mtag.__pr_u.tcp.tm_tcp.__offload.__rx.lro_timediff
#define msg_pri         proto_mtag.__pr_u.tcp.tm_tcp.__msgattr.pri
#define msg_seq         proto_mtag.__pr_u.tcp.tm_tcp.__msgattr.seq
};

/*
 * MPTCP mbuf tag
 */
struct mptcp_pktinfo {
	u_int64_t       mtpi_dsn;       /* MPTCP Data Sequence Number */
	u_int32_t       mtpi_rel_seq;   /* Relative Seq Number */
	u_int16_t       mtpi_length;    /* Length of mapping */
	u_int16_t       mtpi_csum;
#define mp_dsn          proto_mtag.__pr_u.tcp.tm_mptcp.mtpi_dsn
#define mp_rseq         proto_mtag.__pr_u.tcp.tm_mptcp.mtpi_rel_seq
#define mp_rlen         proto_mtag.__pr_u.tcp.tm_mptcp.mtpi_length
#define mp_csum         proto_mtag.__pr_u.tcp.tm_mptcp.mtpi_csum
};

/*
 * TCP specific mbuf tag.  Note that the current implementation uses
 * MPTCP metadata strictly between MPTCP and the TCP subflow layers,
 * hence tm_tcp and tm_mptcp are mutually exclusive.  This also means
 * that TCP messages functionality is currently incompatible with MPTCP.
 */
struct tcp_mtag {
	union {
		struct tcp_pktinfo      tm_tcp;         /* TCP and below */
		struct mptcp_pktinfo    tm_mptcp;       /* MPTCP-TCP only */
	};
};

struct udp_mtag {
	pid_t     _pid;
	pid_t     _e_pid;
#define tx_udp_pid      proto_mtag.__pr_u.udp._pid
#define tx_udp_e_pid    proto_mtag.__pr_u.udp._e_pid
};

struct rawip_mtag {
	pid_t     _pid;
	pid_t     _e_pid;
#define tx_rawip_pid    proto_mtag.__pr_u.rawip._pid
#define tx_rawip_e_pid  proto_mtag.__pr_u.rawip._e_pid
};

struct driver_mtag_ {
	uintptr_t               _drv_tx_compl_arg;
	uintptr_t               _drv_tx_compl_data;
	kern_return_t           _drv_tx_status;
	uint16_t                _drv_flowid;
#define drv_tx_compl_arg        builtin_mtag._drv_mtag._drv_tx_compl_arg
#define drv_tx_compl_data       builtin_mtag._drv_mtag._drv_tx_compl_data
#define drv_tx_status           builtin_mtag._drv_mtag._drv_tx_status
#define drv_flowid              builtin_mtag._drv_mtag._drv_flowid
};

/*
 * Protocol specific mbuf tag (at most one protocol metadata per mbuf).
 *
 * Care must be taken to ensure that they are mutually exclusive, e.g.
 * IPsec policy ID implies no TCP segment offload (which is fine given
 * that the former is used on the virtual ipsec interface that does
 * not advertise the TSO capability.)
 */
struct proto_mtag_ {
	union {
		struct tcp_mtag tcp;            /* TCP specific */
		struct udp_mtag         udp;    /* UDP specific */
		struct rawip_mtag       rawip;  /* raw IPv4/IPv6 specific */
	} __pr_u;
};

/*
 * NECP specific mbuf tag.
 */
struct necp_mtag_ {
	u_int32_t       necp_policy_id;
	u_int32_t       necp_skip_policy_id;
	u_int32_t       necp_route_rule_id;
	u_int16_t       necp_last_interface_index;
	u_int16_t       necp_app_id;
};

union builtin_mtag {
	struct {
		struct proto_mtag_ _proto_mtag; /* built-in protocol-specific tag */
		struct pf_mtag  _pf_mtag;       /* built-in PF tag */
		struct necp_mtag_ _necp_mtag; /* built-in NECP tag */
	} _net_mtag;
	struct driver_mtag_ _drv_mtag;
#define necp_mtag builtin_mtag._net_mtag._necp_mtag
#define proto_mtag builtin_mtag._net_mtag._proto_mtag
#define driver_mtag builtin_mtag._drv_mtag
};

/*
 * Record/packet header in first mbuf of chain; valid only if M_PKTHDR set.
 */
struct pkthdr {
	struct ifnet *rcvif;            /* rcv interface */
	/* variables for ip and tcp reassembly */
	void    *pkt_hdr;               /* pointer to packet header */
	int32_t len;                    /* total packet length */
	/* variables for hardware checksum */
	/* Note: csum_flags is used for hardware checksum and VLAN */
	u_int32_t csum_flags;           /* flags regarding checksum */
	union {
		struct {
			u_int16_t val;   /* checksum value */
			u_int16_t start; /* checksum start offset */
		} _csum_rx;
#define csum_rx_val     _csum_rx.val
#define csum_rx_start   _csum_rx.start
		struct {
			u_int16_t start; /* checksum start offset */
			u_int16_t stuff; /* checksum stuff offset */
		} _csum_tx;
#define csum_tx_start   _csum_tx.start
#define csum_tx_stuff   _csum_tx.stuff
		/*
		 * Generic data field used by csum routines.
		 * It gets used differently in different contexts.
		 */
		u_int32_t csum_data;
	};
	u_int16_t vlan_tag;             /* VLAN tag, host byte order */
	/*
	 * Packet classifier info
	 *
	 * PKTF_FLOW_ID set means valid flow ID.  A non-zero flow ID value
	 * means the packet has been classified by one of the flow sources.
	 * It is also a prerequisite for flow control advisory, which is
	 * enabled by additionally setting PKTF_FLOW_ADV.
	 *
	 * The protocol value is a best-effort representation of the payload.
	 * It is opportunistically updated and used only for optimization.
	 * It is not a substitute for parsing the protocol header(s); use it
	 * only as a hint.
	 *
	 * If PKTF_IFAINFO is set, pkt_ifainfo contains one or both of the
	 * indices of interfaces which own the source and/or destination
	 * addresses of the packet.  For the local/loopback case (PKTF_LOOP),
	 * both should be valid, and thus allows for the receiving end to
	 * quickly determine the actual interfaces used by the the addresses;
	 * they may not necessarily be the same or refer to the loopback
	 * interface.  Otherwise, in the non-local/loopback case, the indices
	 * are opportunistically set, and because of that only one may be set
	 * (0 means the index has not been determined.)  In addition, the
	 * interface address flags are also recorded.  This allows us to avoid
	 * storing the corresponding {in,in6}_ifaddr in an mbuf tag.  Ideally
	 * this would be a superset of {ia,ia6}_flags, but the namespaces are
	 * overlapping at present, so we'll need a new set of values in future
	 * to achieve this.  For now, we will just rely on the address family
	 * related code paths examining this mbuf to interpret the flags.
	 */
	u_int8_t pkt_proto;             /* IPPROTO value */
	u_int8_t pkt_flowsrc;           /* FLOWSRC values */
	u_int32_t pkt_flowid;           /* flow ID */
	u_int32_t pkt_flags;            /* PKTF flags (see below) */
	u_int32_t pkt_svc;              /* MBUF_SVC value */

	u_int32_t pkt_compl_context;            /* Packet completion context */

	union {
		struct {
			u_int16_t src;          /* ifindex of src addr i/f */
			u_int16_t src_flags;    /* src PKT_IFAIFF flags */
			u_int16_t dst;          /* ifindex of dst addr i/f */
			u_int16_t dst_flags;    /* dst PKT_IFAIFF flags */
		} _pkt_iaif;
#define src_ifindex     _pkt_iaif.src
#define src_iff         _pkt_iaif.src_flags
#define dst_ifindex     _pkt_iaif.dst
#define dst_iff         _pkt_iaif.dst_flags
		u_int64_t pkt_ifainfo;  /* data field used by ifainfo */
		struct {
			u_int32_t if_data; /* bytes in interface queue */
			u_int32_t sndbuf_data; /* bytes in socket buffer */
		} _pkt_bsr;     /* Buffer status report used by cellular interface */
#define bufstatus_if    _pkt_bsr.if_data
#define bufstatus_sndbuf        _pkt_bsr.sndbuf_data
	};
	u_int64_t pkt_timestamp;        /* TX: enqueue time, RX: receive timestamp */

	/*
	 * Tags (external and built-in)
	 */
	SLIST_HEAD(packet_tags, m_tag) tags; /* list of external tags */
	union builtin_mtag builtin_mtag;
	/*
	 * Module private scratch space (32-bit aligned), currently 16-bytes
	 * large. Anything stored here is not guaranteed to survive across
	 * modules.  The AQM layer (outbound) uses all 16-bytes for both
	 * packet scheduling and flow advisory information.
	 */
	struct {
		union {
			u_int8_t        __mpriv8[16];
			u_int16_t       __mpriv16[8];
			struct {
				union {
					u_int8_t        __val8[4];
					u_int16_t       __val16[2];
					u_int32_t       __val32;
				} __mpriv32_u;
			}               __mpriv32[4];
			u_int64_t       __mpriv64[2];
		} __mpriv_u;
	} pkt_mpriv __attribute__((aligned(4)));
#define pkt_mpriv_hash  pkt_mpriv.__mpriv_u.__mpriv32[0].__mpriv32_u.__val32
#define pkt_mpriv_flags pkt_mpriv.__mpriv_u.__mpriv32[1].__mpriv32_u.__val32
#define pkt_mpriv_srcid pkt_mpriv.__mpriv_u.__mpriv32[2].__mpriv32_u.__val32
#define pkt_mpriv_fidx  pkt_mpriv.__mpriv_u.__mpriv32[3].__mpriv32_u.__val32

	u_int32_t redzone;              /* red zone */
	u_int32_t pkt_compl_callbacks;  /* Packet completion callbacks */
};

/*
 * Flow data source type.  A data source module is responsible for generating
 * a unique flow ID and associating it to each data flow as pkt_flowid.
 * This is required for flow control/advisory, as it allows the output queue
 * to identify the data source object and inform that it can resume its
 * transmission (in the event it was flow controlled.)
 */
#define FLOWSRC_INPCB           1       /* flow ID generated by INPCB */
#define FLOWSRC_IFNET           2       /* flow ID generated by interface */
#define FLOWSRC_PF              3       /* flow ID generated by PF */
#define FLOWSRC_CHANNEL         4       /* flow ID generated by channel */

/*
 * FLOWSRC_MPKL_INPUT is not a true flow data source and is used for
 * multi-layer packet logging. We're usurping the pkt_flowsrc field because
 * the mbuf packet header ran out of space and pkt_flowsrc is normally
 * used on output so we assume we can safely overwrite the normal semantic of
 * the field.
 * This value is meant to be used on incoming packet from a lower level protocol
 * to pass information to some upper level protocol. When FLOWSRC_MPKL_INPUT
 * is set, the following fields are used:
 * - pkt_proto: the IP protocol ID of the lower level protocol
 * - pkt_flowid: the identifier of the packet at the lower protocol.
 * For example ESP would set pkt_proto to IPPROTO_ESP and pkt_flowid to the SPI.
 */

/*
 * Packet flags.  Unlike m_flags, all packet flags are copied along when
 * copying m_pkthdr, i.e. no equivalent of M_COPYFLAGS here.  These flags
 * (and other classifier info) will be cleared during DLIL input.
 *
 * Some notes about M_LOOP and PKTF_LOOP:
 *
 *    - M_LOOP flag is overloaded, and its use is discouraged.  Historically,
 *	that flag was used by the KAME implementation for allowing certain
 *	certain exceptions to be made in the IP6_EXTHDR_CHECK() logic; this
 *	was originally meant to be set as the packet is looped back to the
 *	system, and in some circumstances temporarily set in ip6_output().
 *	Over time, this flag was used by the pre-output routines to indicate
 *	to the DLIL frameout and output routines, that the packet may be
 *	looped back to the system under the right conditions.  In addition,
 *	this is an mbuf flag rather than an mbuf packet header flag.
 *
 *    - PKTF_LOOP is an mbuf packet header flag, which is set if and only
 *	if the packet was looped back to the system.  This flag should be
 *	used instead for newer code.
 */
#define PKTF_FLOW_ID            0x1     /* pkt has valid flowid value */
#define PKTF_FLOW_ADV           0x2     /* pkt triggers local flow advisory */
#define PKTF_FLOW_LOCALSRC      0x4     /* pkt is locally originated  */
#define PKTF_FLOW_RAWSOCK       0x8     /* pkt locally generated by raw sock */
#define PKTF_PRIO_PRIVILEGED    0x10    /* packet priority is privileged */
#define PKTF_PROXY_DST          0x20    /* processed but not locally destined */
#define PKTF_INET_RESOLVE       0x40    /* IPv4 resolver packet */
#define PKTF_INET6_RESOLVE      0x80    /* IPv6 resolver packet */
#define PKTF_RESOLVE_RTR        0x100   /* pkt is for resolving router */
#define PKTF_SW_LRO_PKT         0x200   /* pkt is a large coalesced pkt */
#define PKTF_SW_LRO_DID_CSUM    0x400   /* IP and TCP checksums done by LRO */
#define PKTF_MPTCP              0x800   /* TCP with MPTCP metadata */
#define PKTF_MPSO               0x1000  /* MPTCP socket meta data */
#define PKTF_LOOP               0x2000  /* loopbacked packet */
#define PKTF_IFAINFO            0x4000  /* pkt has valid interface addr info */
#define PKTF_SO_BACKGROUND      0x8000  /* data is from background source */
#define PKTF_FORWARDED          0x10000 /* pkt was forwarded from another i/f */
#define PKTF_PRIV_GUARDED       0x20000 /* pkt_mpriv area guard enabled */
#define PKTF_KEEPALIVE          0x40000 /* pkt is kernel-generated keepalive */
#define PKTF_SO_REALTIME        0x80000 /* data is realtime traffic */
#define PKTF_VALID_UNSENT_DATA  0x100000 /* unsent data is valid */
#define PKTF_TCP_REXMT          0x200000 /* packet is TCP retransmission */
#define PKTF_REASSEMBLED        0x400000 /* Packet was reassembled */
#define PKTF_TX_COMPL_TS_REQ    0x800000 /* tx completion timestamp requested */
#define PKTF_TS_VALID           0x1000000 /* pkt timestamp is valid */
#define PKTF_DRIVER_MTAG        0x2000000 /* driver mbuf tags fields inited */
#define PKTF_NEW_FLOW           0x4000000 /* Data from a new flow */
#define PKTF_START_SEQ          0x8000000 /* valid start sequence */
#define PKTF_LAST_PKT           0x10000000 /* last packet in the flow */
#define PKTF_MPTCP_REINJ        0x20000000 /* Packet has been reinjected for MPTCP */
#define PKTF_MPTCP_DFIN         0x40000000 /* Packet is a data-fin */
#define PKTF_HBH_CHKED          0x80000000 /* HBH option is checked */

/* flags related to flow control/advisory and identification */
#define PKTF_FLOW_MASK  \
	(PKTF_FLOW_ID | PKTF_FLOW_ADV | PKTF_FLOW_LOCALSRC | PKTF_FLOW_RAWSOCK)

/*
 * Description of external storage mapped into mbuf, valid only if M_EXT set.
 */
typedef void (*m_ext_free_func_t)(caddr_t, u_int, caddr_t);
struct m_ext {
	caddr_t ext_buf;                /* start of buffer */
	m_ext_free_func_t ext_free;     /* free routine if not the usual */
	u_int   ext_size;               /* size of buffer, for ext_free */
	caddr_t ext_arg;                /* additional ext_free argument */
	struct ext_ref {
		struct mbuf *paired;
		u_int16_t minref;
		u_int16_t refcnt;
		u_int16_t prefcnt;
		u_int16_t flags;
		u_int32_t priv;
		uintptr_t ext_token;
	} *ext_refflags;
};

/* define m_ext to a type since it gets redefined below */
typedef struct m_ext _m_ext_t;

/*
 * The mbuf object
 */
struct mbuf {
	struct m_hdr m_hdr;
	union {
		struct {
			struct pkthdr MH_pkthdr;        /* M_PKTHDR set */
			union {
				struct m_ext MH_ext;    /* M_EXT set */
				char    MH_databuf[_MHLEN];
			} MH_dat;
		} MH;
		char    M_databuf[_MLEN];               /* !M_PKTHDR, !M_EXT */
	} M_dat;
};

#define m_next          m_hdr.mh_next
#define m_len           m_hdr.mh_len
#define m_data          m_hdr.mh_data
#define m_type          m_hdr.mh_type
#define m_flags         m_hdr.mh_flags
#define m_nextpkt       m_hdr.mh_nextpkt
#define m_act           m_nextpkt
#define m_pkthdr        M_dat.MH.MH_pkthdr
#define m_ext           M_dat.MH.MH_dat.MH_ext
#define m_pktdat        M_dat.MH.MH_dat.MH_databuf
#define m_dat           M_dat.M_databuf
#define m_pktlen(_m)    ((_m)->m_pkthdr.len)
#define m_pftag(_m)     (&(_m)->m_pkthdr.builtin_mtag._net_mtag._pf_mtag)

/* mbuf flags (private) */
#define M_EXT           0x0001  /* has associated external storage */
#define M_PKTHDR        0x0002  /* start of record */
#define M_EOR           0x0004  /* end of record */
#define M_PROTO1        0x0008  /* protocol-specific */
#define M_PROTO2        0x0010  /* protocol-specific */
#define M_PROTO3        0x0020  /* protocol-specific */
#define M_LOOP          0x0040  /* packet is looped back (also see PKTF_LOOP) */
#define M_PROTO5        0x0080  /* protocol-specific */

/* mbuf pkthdr flags, also in m_flags (private) */
#define M_BCAST         0x0100  /* send/received as link-level broadcast */
#define M_MCAST         0x0200  /* send/received as link-level multicast */
#define M_FRAG          0x0400  /* packet is a fragment of a larger packet */
#define M_FIRSTFRAG     0x0800  /* packet is first fragment */
#define M_LASTFRAG      0x1000  /* packet is last fragment */
#define M_PROMISC       0x2000  /* packet is promiscuous (shouldn't go to stack) */
#define M_HASFCS        0x4000  /* packet has FCS */
#define M_TAGHDR        0x8000  /* m_tag hdr structure at top of mbuf data */

/*
 * Flags to purge when crossing layers.
 */
#define M_PROTOFLAGS \
	(M_PROTO1|M_PROTO2|M_PROTO3|M_PROTO5)

/* flags copied when copying m_pkthdr */
#define M_COPYFLAGS                                                     \
	(M_PKTHDR|M_EOR|M_PROTO1|M_PROTO2|M_PROTO3 |                    \
	M_LOOP|M_PROTO5|M_BCAST|M_MCAST|M_FRAG |                        \
	M_FIRSTFRAG|M_LASTFRAG|M_PROMISC|M_HASFCS)

/* flags indicating hw checksum support and sw checksum requirements */
#define CSUM_IP                 0x0001          /* will csum IP */
#define CSUM_TCP                0x0002          /* will csum TCP */
#define CSUM_UDP                0x0004          /* will csum UDP */
#define CSUM_IP_FRAGS           0x0008          /* will csum IP fragments */
#define CSUM_FRAGMENT           0x0010          /* will do IP fragmentation */
#define CSUM_TCPIPV6            0x0020          /* will csum TCP for IPv6 */
#define CSUM_UDPIPV6            0x0040          /* will csum UDP for IPv6 */
#define CSUM_FRAGMENT_IPV6      0x0080          /* will do IPv6 fragmentation */

#define CSUM_IP_CHECKED         0x0100          /* did csum IP */
#define CSUM_IP_VALID           0x0200          /*   ... the csum is valid */
#define CSUM_DATA_VALID         0x0400          /* csum_data field is valid */
#define CSUM_PSEUDO_HDR         0x0800          /* csum_data has pseudo hdr */
#define CSUM_PARTIAL            0x1000          /* simple Sum16 computation */
#define CSUM_ZERO_INVERT        0x2000          /* invert 0 to -0 (0xffff) */

#define CSUM_DELAY_DATA         (CSUM_TCP | CSUM_UDP)
#define CSUM_DELAY_IP           (CSUM_IP)       /* IPv4 only: no IPv6 IP cksum */
#define CSUM_DELAY_IPV6_DATA    (CSUM_TCPIPV6 | CSUM_UDPIPV6)
#define CSUM_DATA_IPV6_VALID    CSUM_DATA_VALID /* csum_data field is valid */

#define CSUM_TX_FLAGS                                                   \
	(CSUM_DELAY_IP | CSUM_DELAY_DATA | CSUM_DELAY_IPV6_DATA |       \
	CSUM_DATA_VALID | CSUM_PARTIAL | CSUM_ZERO_INVERT)

#define CSUM_RX_FLAGS                                                   \
	(CSUM_IP_CHECKED | CSUM_IP_VALID | CSUM_PSEUDO_HDR |            \
	CSUM_DATA_VALID | CSUM_PARTIAL)

/*
 * Note: see also IF_HWASSIST_CSUM defined in <net/if_var.h>
 */

/* VLAN tag present */
#define CSUM_VLAN_TAG_VALID     0x00010000      /* vlan_tag field is valid */

/* checksum start adjustment has been done */
#define CSUM_ADJUST_DONE        0x00020000

/* TCP Segment Offloading requested on this mbuf */
#define CSUM_TSO_IPV4           0x00100000      /* This mbuf needs to be segmented by the NIC */
#define CSUM_TSO_IPV6           0x00200000      /* This mbuf needs to be segmented by the NIC */

#define TSO_IPV4_OK(_ifp, _m)                                           \
    (((_ifp)->if_hwassist & IFNET_TSO_IPV4) &&                          \
    ((_m)->m_pkthdr.csum_flags & CSUM_TSO_IPV4))                        \

#define TSO_IPV4_NOTOK(_ifp, _m)                                        \
    (!((_ifp)->if_hwassist & IFNET_TSO_IPV4) &&                         \
    ((_m)->m_pkthdr.csum_flags & CSUM_TSO_IPV4))                        \

#define TSO_IPV6_OK(_ifp, _m)                                           \
    (((_ifp)->if_hwassist & IFNET_TSO_IPV6) &&                          \
    ((_m)->m_pkthdr.csum_flags & CSUM_TSO_IPV6))                        \

#define TSO_IPV6_NOTOK(_ifp, _m)                                        \
    (!((_ifp)->if_hwassist & IFNET_TSO_IPV6) &&                         \
    ((_m)->m_pkthdr.csum_flags & CSUM_TSO_IPV6))                        \

#endif /* XNU_KERNEL_PRIVATE */

/* mbuf types */
#define MT_FREE         0       /* should be on free list */
#define MT_DATA         1       /* dynamic (data) allocation */
#define MT_HEADER       2       /* packet header */
#define MT_SOCKET       3       /* socket structure */
#define MT_PCB          4       /* protocol control block */
#define MT_RTABLE       5       /* routing tables */
#define MT_HTABLE       6       /* IMP host tables */
#define MT_ATABLE       7       /* address resolution tables */
#define MT_SONAME       8       /* socket name */
#define MT_SOOPTS       10      /* socket options */
#define MT_FTABLE       11      /* fragment reassembly header */
#define MT_RIGHTS       12      /* access rights */
#define MT_IFADDR       13      /* interface address */
#define MT_CONTROL      14      /* extra-data protocol message */
#define MT_OOBDATA      15      /* expedited data  */
#define MT_TAG          16      /* volatile metadata associated to pkts */
#define MT_MAX          32      /* enough? */

#ifdef XNU_KERNEL_PRIVATE
/*
 * mbuf allocation/deallocation macros:
 *
 *	MGET(struct mbuf *m, int how, int type)
 * allocates an mbuf and initializes it to contain internal data.
 *
 *	MGETHDR(struct mbuf *m, int how, int type)
 * allocates an mbuf and initializes it to contain a packet header
 * and internal data.
 */

#if 1
#define MCHECK(m) m_mcheck(m)
#else
#define MCHECK(m)
#endif

#define MGET(m, how, type) ((m) = m_get((how), (type)))

#define MGETHDR(m, how, type)   ((m) = m_gethdr((how), (type)))

/*
 * Mbuf cluster macros.
 * MCLALLOC(caddr_t p, int how) allocates an mbuf cluster.
 * MCLGET adds such clusters to a normal mbuf;
 * the flag M_EXT is set upon success.
 * MCLFREE releases a reference to a cluster allocated by MCLALLOC,
 * freeing the cluster if the reference count has reached 0.
 *
 * Normal mbuf clusters are normally treated as character arrays
 * after allocation, but use the first word of the buffer as a free list
 * pointer while on the free list.
 */
union mcluster {
	union   mcluster *mcl_next;
	char    mcl_buf[MCLBYTES];
};

#define MCLALLOC(p, how)        ((p) = m_mclalloc(how))

#define MCLFREE(p)              m_mclfree(p)

#define MCLGET(m, how)          ((m) = m_mclget(m, how))

/*
 * Mbuf big cluster
 */
union mbigcluster {
	union mbigcluster       *mbc_next;
	char                    mbc_buf[MBIGCLBYTES];
};

/*
 * Mbuf jumbo cluster
 */
union m16kcluster {
	union m16kcluster       *m16kcl_next;
	char                    m16kcl_buf[M16KCLBYTES];
};

#define MCLHASREFERENCE(m)      m_mclhasreference(m)

/*
 * MFREE(struct mbuf *m, struct mbuf *n)
 * Free a single mbuf and associated external storage.
 * Place the successor, if any, in n.
 */

#define MFREE(m, n) ((n) = m_free(m))

/*
 * Copy mbuf pkthdr from from to to.
 * from must have M_PKTHDR set, and to must be empty.
 * aux pointer will be moved to `to'.
 */
#define M_COPY_PKTHDR(to, from)         m_copy_pkthdr(to, from)

#define M_COPY_PFTAG(to, from)          m_copy_pftag(to, from)

#define M_COPY_CLASSIFIER(to, from)     m_copy_classifier(to, from)

/*
 * Evaluate TRUE if it's safe to write to the mbuf m's data region (this can
 * be both the local data payload, or an external buffer area, depending on
 * whether M_EXT is set).
 */
#define M_WRITABLE(m)   (((m)->m_flags & M_EXT) == 0 || !MCLHASREFERENCE(m))

/*
 * These macros are mapped to the appropriate KPIs, so that private code
 * can be simply recompiled in order to be forward-compatible with future
 * changes toward the struture sizes.
 */
#define MLEN            mbuf_get_mlen()         /* normal mbuf data len */
#define MHLEN           mbuf_get_mhlen()        /* data len in an mbuf w/pkthdr */
#define MINCLSIZE       mbuf_get_minclsize()    /* cluster usage threshold */
/*
 * Return the address of the start of the buffer associated with an mbuf,
 * handling external storage, packet-header mbufs, and regular data mbufs.
 */
#define M_START(m)                                                      \
	(((m)->m_flags & M_EXT) ? (m)->m_ext.ext_buf :                  \
	 ((m)->m_flags & M_PKTHDR) ? &(m)->m_pktdat[0] :                \
	 &(m)->m_dat[0])

/*
 * Return the size of the buffer associated with an mbuf, handling external
 * storage, packet-header mbufs, and regular data mbufs.
 */
#define M_SIZE(m)                                                       \
	(((m)->m_flags & M_EXT) ? (m)->m_ext.ext_size :                 \
	 ((m)->m_flags & M_PKTHDR) ? MHLEN :                            \
	 MLEN)

#define M_ALIGN(m, len)         m_align(m, len)
#define MH_ALIGN(m, len)        m_align(m, len)
#define MEXT_ALIGN(m, len)      m_align(m, len)

/*
 * Compute the amount of space available before the current start of data in
 * an mbuf.
 *
 * The M_WRITABLE() is a temporary, conservative safety measure: the burden
 * of checking writability of the mbuf data area rests solely with the caller.
 */
#define M_LEADINGSPACE(m)                                               \
	(M_WRITABLE(m) ? ((m)->m_data - M_START(m)) : 0)

/*
 * Compute the amount of space available after the end of data in an mbuf.
 *
 * The M_WRITABLE() is a temporary, conservative safety measure: the burden
 * of checking writability of the mbuf data area rests solely with the caller.
 */
#define M_TRAILINGSPACE(m)                                              \
	(M_WRITABLE(m) ?                                                \
	    ((M_START(m) + M_SIZE(m)) - ((m)->m_data + (m)->m_len)) : 0)

/*
 * Arrange to prepend space of size plen to mbuf m.
 * If a new mbuf must be allocated, how specifies whether to wait.
 * If how is M_DONTWAIT and allocation fails, the original mbuf chain
 * is freed and m is set to NULL.
 */
#define M_PREPEND(m, plen, how, align)  \
    ((m) = m_prepend_2((m), (plen), (how), (align)))

/* change mbuf to new type */
#define MCHTYPE(m, t)           m_mchtype(m, t)

/* compatiblity with 4.3 */
#define m_copy(m, o, l)         m_copym((m), (o), (l), M_DONTWAIT)

#define MBSHIFT         20                              /* 1MB */
#define MBSIZE          (1 << MBSHIFT)
#define GBSHIFT         30                              /* 1GB */
#define GBSIZE          (1 << GBSHIFT)

/*
 * M_STRUCT_GET ensures that intermediate protocol header (from "off" to
 * "off+len") is located in single mbuf, on contiguous memory region.
 * The pointer to the region will be returned to pointer variable "val",
 * with type "typ".
 *
 * M_STRUCT_GET0 does the same, except that it aligns the structure at
 * very top of mbuf.  GET0 is likely to make memory copy than GET.
 */
#define M_STRUCT_GET(val, typ, m, off, len)                             \
do {                                                                    \
	struct mbuf *t;                                                 \
	int tmp;                                                        \
                                                                        \
	if ((m)->m_len >= (off) + (len)) {                              \
	        (val) = (typ)(mtod((m), caddr_t) + (off));              \
	} else {                                                        \
	        t = m_pulldown((m), (off), (len), &tmp);                \
	        if (t != NULL) {                                        \
	                if (t->m_len < tmp + (len))                     \
	                        panic("m_pulldown malfunction");        \
	                (val) = (typ)(mtod(t, caddr_t) + tmp);          \
	        } else {                                                \
	                (val) = (typ)NULL;                              \
	                (m) = NULL;                                     \
	        }                                                       \
	}                                                               \
} while (0)

#define M_STRUCT_GET0(val, typ, m, off, len)                            \
do {                                                                    \
	struct mbuf *t;                                                 \
                                                                        \
	if ((off) == 0 && ((m)->m_len >= (len))) {                      \
	        (val) = (typ)(void *)mtod(m, caddr_t);                  \
	} else {                                                        \
	        t = m_pulldown((m), (off), (len), NULL);                \
	        if (t != NULL) {                                        \
	                if (t->m_len < (len))                           \
	                        panic("m_pulldown malfunction");        \
	                (val) = (typ)(void *)mtod(t, caddr_t);          \
	        } else {                                                \
	                (val) = (typ)NULL;                              \
	                (m) = NULL;                                     \
	        }                                                       \
	}                                                               \
} while (0)

#define MBUF_INPUT_CHECK(m, rcvif)                                      \
do {                                                                    \
	if (!(m->m_flags & MBUF_PKTHDR) ||                              \
	    m->m_len < 0 ||                                             \
	    m->m_len > ((njcl > 0) ? njclbytes : MBIGCLBYTES) ||        \
	    m->m_type == MT_FREE ||                                     \
	    ((m->m_flags & M_EXT) != 0 && m->m_ext.ext_buf == NULL)) {  \
	        panic_plain("Failed mbuf validity check: mbuf %p len %d "  \
	            "type %d flags 0x%x data %p rcvif %s ifflags 0x%x",  \
	            m, m->m_len, m->m_type, m->m_flags,                    \
	            ((m->m_flags & M_EXT) ? m->m_ext.ext_buf : m->m_data), \
	            if_name(rcvif),                                     \
	            (rcvif->if_flags & 0xffff));                        \
	}                                                               \
} while (0)

/*
 * Simple mbuf queueing system
 *
 * This is basically a SIMPLEQ adapted to mbuf use (i.e. using
 * m_nextpkt instead of field.sqe_next).
 *
 * m_next is ignored, so queueing chains of mbufs is possible
 */
#define MBUFQ_HEAD(name)                                        \
struct name {                                                   \
	struct mbuf *mq_first;  /* first packet */              \
	struct mbuf **mq_last;  /* addr of last next packet */  \
}

#define MBUFQ_INIT(q)           do {                            \
	MBUFQ_FIRST(q) = NULL;                                  \
	(q)->mq_last = &MBUFQ_FIRST(q);                         \
} while (0)

#define MBUFQ_PREPEND(q, m)     do {                            \
	if ((MBUFQ_NEXT(m) = MBUFQ_FIRST(q)) == NULL)           \
	        (q)->mq_last = &MBUFQ_NEXT(m);                  \
	MBUFQ_FIRST(q) = (m);                                   \
} while (0)

#define MBUFQ_ENQUEUE(q, m)     do {                            \
	MBUFQ_NEXT(m) = NULL;                                   \
	*(q)->mq_last = (m);                                    \
	(q)->mq_last = &MBUFQ_NEXT(m);                          \
} while (0)

#define MBUFQ_ENQUEUE_MULTI(q, m, n)    do {                    \
	MBUFQ_NEXT(n) = NULL;                                   \
	*(q)->mq_last = (m);                                    \
	(q)->mq_last = &MBUFQ_NEXT(n);                          \
} while (0)

#define MBUFQ_DEQUEUE(q, m)     do {                            \
	if (((m) = MBUFQ_FIRST(q)) != NULL) {                   \
	        if ((MBUFQ_FIRST(q) = MBUFQ_NEXT(m)) == NULL)   \
	                (q)->mq_last = &MBUFQ_FIRST(q);         \
	        else                                            \
	                MBUFQ_NEXT(m) = NULL;                   \
	}                                                       \
} while (0)

#define MBUFQ_REMOVE(q, m)      do {                            \
	if (MBUFQ_FIRST(q) == (m)) {                            \
	        MBUFQ_DEQUEUE(q, m);                            \
	} else {                                                \
	        struct mbuf *_m = MBUFQ_FIRST(q);               \
	        while (MBUFQ_NEXT(_m) != (m))                   \
	                _m = MBUFQ_NEXT(_m);                    \
	        if ((MBUFQ_NEXT(_m) =                           \
	            MBUFQ_NEXT(MBUFQ_NEXT(_m))) == NULL)        \
	                (q)->mq_last = &MBUFQ_NEXT(_m);         \
	}                                                       \
} while (0)

#define MBUFQ_DRAIN(q)          do {                            \
	struct mbuf *__m0;                                      \
	while ((__m0 = MBUFQ_FIRST(q)) != NULL) {               \
	        MBUFQ_FIRST(q) = MBUFQ_NEXT(__m0);              \
	        MBUFQ_NEXT(__m0) = NULL;                        \
	        m_freem(__m0);                                  \
	}                                                       \
	(q)->mq_last = &MBUFQ_FIRST(q);                         \
} while (0)

#define MBUFQ_FOREACH(m, q)                                     \
	for ((m) = MBUFQ_FIRST(q);                              \
	    (m);                                                \
	    (m) = MBUFQ_NEXT(m))

#define MBUFQ_FOREACH_SAFE(m, q, tvar)                          \
	for ((m) = MBUFQ_FIRST(q);                              \
	    (m) && ((tvar) = MBUFQ_NEXT(m), 1);                 \
	    (m) = (tvar))

#define MBUFQ_EMPTY(q)          ((q)->mq_first == NULL)
#define MBUFQ_FIRST(q)          ((q)->mq_first)
#define MBUFQ_NEXT(m)           ((m)->m_nextpkt)
/*
 * mq_last is initialized to point to mq_first, so check if they're
 * equal and return NULL when the list is empty.  Otherwise, we need
 * to subtract the offset of MBUQ_NEXT (i.e. m_nextpkt field) to get
 * to the base mbuf address to return to caller.
 */
#define MBUFQ_LAST(head)                                        \
	(((head)->mq_last == &MBUFQ_FIRST(head)) ? NULL :       \
	((struct mbuf *)(void *)((char *)(head)->mq_last -      \
	     __builtin_offsetof(struct mbuf, m_nextpkt))))

#define max_linkhdr     P2ROUNDUP(_max_linkhdr, sizeof (u_int32_t))
#define max_protohdr    P2ROUNDUP(_max_protohdr, sizeof (u_int32_t))
#endif /* XNU_KERNEL_PRIVATE */

/*
 * Mbuf statistics (legacy).
 */
struct mbstat {
	u_int32_t       m_mbufs;        /* mbufs obtained from page pool */
	u_int32_t       m_clusters;     /* clusters obtained from page pool */
	u_int32_t       m_spare;        /* spare field */
	u_int32_t       m_clfree;       /* free clusters */
	u_int32_t       m_drops;        /* times failed to find space */
	u_int32_t       m_wait;         /* times waited for space */
	u_int32_t       m_drain;        /* times drained protocols for space */
	u_short         m_mtypes[256];  /* type specific mbuf allocations */
	u_int32_t       m_mcfail;       /* times m_copym failed */
	u_int32_t       m_mpfail;       /* times m_pullup failed */
	u_int32_t       m_msize;        /* length of an mbuf */
	u_int32_t       m_mclbytes;     /* length of an mbuf cluster */
	u_int32_t       m_minclsize;    /* min length of data to allocate a cluster */
	u_int32_t       m_mlen;         /* length of data in an mbuf */
	u_int32_t       m_mhlen;        /* length of data in a header mbuf */
	u_int32_t       m_bigclusters;  /* clusters obtained from page pool */
	u_int32_t       m_bigclfree;    /* free clusters */
	u_int32_t       m_bigmclbytes;  /* length of an mbuf cluster */
};

/* Compatibillity with 10.3 */
struct ombstat {
	u_int32_t       m_mbufs;        /* mbufs obtained from page pool */
	u_int32_t       m_clusters;     /* clusters obtained from page pool */
	u_int32_t       m_spare;        /* spare field */
	u_int32_t       m_clfree;       /* free clusters */
	u_int32_t       m_drops;        /* times failed to find space */
	u_int32_t       m_wait;         /* times waited for space */
	u_int32_t       m_drain;        /* times drained protocols for space */
	u_short         m_mtypes[256];  /* type specific mbuf allocations */
	u_int32_t       m_mcfail;       /* times m_copym failed */
	u_int32_t       m_mpfail;       /* times m_pullup failed */
	u_int32_t       m_msize;        /* length of an mbuf */
	u_int32_t       m_mclbytes;     /* length of an mbuf cluster */
	u_int32_t       m_minclsize;    /* min length of data to allocate a cluster */
	u_int32_t       m_mlen;         /* length of data in an mbuf */
	u_int32_t       m_mhlen;        /* length of data in a header mbuf */
};

/*
 * mbuf class statistics.
 */
#define MAX_MBUF_CNAME  15

#if defined(XNU_KERNEL_PRIVATE)
/* For backwards compatibility with 32-bit userland process */
struct omb_class_stat {
	char            mbcl_cname[MAX_MBUF_CNAME + 1]; /* class name */
	u_int32_t       mbcl_size;      /* buffer size */
	u_int32_t       mbcl_total;     /* # of buffers created */
	u_int32_t       mbcl_active;    /* # of active buffers */
	u_int32_t       mbcl_infree;    /* # of available buffers */
	u_int32_t       mbcl_slab_cnt;  /* # of available slabs */
	u_int64_t       mbcl_alloc_cnt; /* # of times alloc is called */
	u_int64_t       mbcl_free_cnt;  /* # of times free is called */
	u_int64_t       mbcl_notified;  /* # of notified wakeups */
	u_int64_t       mbcl_purge_cnt; /* # of purges so far */
	u_int64_t       mbcl_fail_cnt;  /* # of allocation failures */
	u_int32_t       mbcl_ctotal;    /* total only for this class */
	u_int32_t       mbcl_release_cnt; /* amount of memory returned */
	/*
	 * Cache layer statistics
	 */
	u_int32_t       mbcl_mc_state;  /* cache state (see below) */
	u_int32_t       mbcl_mc_cached; /* # of cached buffers */
	u_int32_t       mbcl_mc_waiter_cnt;  /* # waiters on the cache */
	u_int32_t       mbcl_mc_wretry_cnt;  /* # of wait retries */
	u_int32_t       mbcl_mc_nwretry_cnt; /* # of no-wait retry attempts */
	u_int64_t       mbcl_reserved[4];    /* for future use */
} __attribute__((__packed__));
#endif /* XNU_KERNEL_PRIVATE */

typedef struct mb_class_stat {
	char            mbcl_cname[MAX_MBUF_CNAME + 1]; /* class name */
	u_int32_t       mbcl_size;      /* buffer size */
	u_int32_t       mbcl_total;     /* # of buffers created */
	u_int32_t       mbcl_active;    /* # of active buffers */
	u_int32_t       mbcl_infree;    /* # of available buffers */
	u_int32_t       mbcl_slab_cnt;  /* # of available slabs */
#if defined(KERNEL) || defined(__LP64__)
	u_int32_t       mbcl_pad;       /* padding */
#endif /* KERNEL || __LP64__ */
	u_int64_t       mbcl_alloc_cnt; /* # of times alloc is called */
	u_int64_t       mbcl_free_cnt;  /* # of times free is called */
	u_int64_t       mbcl_notified;  /* # of notified wakeups */
	u_int64_t       mbcl_purge_cnt; /* # of purges so far */
	u_int64_t       mbcl_fail_cnt;  /* # of allocation failures */
	u_int32_t       mbcl_ctotal;    /* total only for this class */
	u_int32_t       mbcl_release_cnt; /* amount of memory returned */
	/*
	 * Cache layer statistics
	 */
	u_int32_t       mbcl_mc_state;  /* cache state (see below) */
	u_int32_t       mbcl_mc_cached; /* # of cached buffers */
	u_int32_t       mbcl_mc_waiter_cnt;  /* # waiters on the cache */
	u_int32_t       mbcl_mc_wretry_cnt;  /* # of wait retries */
	u_int32_t       mbcl_mc_nwretry_cnt; /* # of no-wait retry attempts */
	u_int32_t       mbcl_peak_reported; /* last usage peak reported */
	u_int32_t       mbcl_reserved[7];    /* for future use */
} mb_class_stat_t;

#define MCS_DISABLED    0       /* cache is permanently disabled */
#define MCS_ONLINE      1       /* cache is online */
#define MCS_PURGING     2       /* cache is being purged */
#define MCS_OFFLINE     3       /* cache is offline (resizing) */

#if defined(XNU_KERNEL_PRIVATE)
/* For backwards compatibility with 32-bit userland process */
struct omb_stat {
	u_int32_t               mbs_cnt;        /* number of classes */
	struct omb_class_stat   mbs_class[1];   /* class array */
} __attribute__((__packed__));
#endif /* XNU_KERNEL_PRIVATE */

typedef struct mb_stat {
	u_int32_t       mbs_cnt;        /* number of classes */
#if defined(KERNEL) || defined(__LP64__)
	u_int32_t       mbs_pad;        /* padding */
#endif /* KERNEL || __LP64__ */
	mb_class_stat_t mbs_class[1];   /* class array */
} mb_stat_t;

#ifdef PRIVATE
#define MLEAK_STACK_DEPTH       16      /* Max PC stack depth */

typedef struct mleak_trace_stat {
	u_int64_t       mltr_collisions;
	u_int64_t       mltr_hitcount;
	u_int64_t       mltr_allocs;
	u_int64_t       mltr_depth;
	u_int64_t       mltr_addr[MLEAK_STACK_DEPTH];
} mleak_trace_stat_t;

typedef struct mleak_stat {
	u_int32_t               ml_isaddr64;    /* 64-bit KVA? */
	u_int32_t               ml_cnt;         /* number of traces */
	mleak_trace_stat_t      ml_trace[1];    /* trace array */
} mleak_stat_t;

struct mleak_table {
	u_int32_t mleak_capture;        /* sampling capture counter */
	u_int32_t mleak_sample_factor;  /* sample factor */

	/* Times two active records want to occupy the same spot */
	u_int64_t alloc_collisions;
	u_int64_t trace_collisions;

	/* Times new record lands on spot previously occupied by freed alloc */
	u_int64_t alloc_overwrites;
	u_int64_t trace_overwrites;

	/* Times a new alloc or trace is put into the hash table */
	u_int64_t alloc_recorded;
	u_int64_t trace_recorded;

	/* Total number of outstanding allocs */
	u_int64_t outstanding_allocs;

	/* Times mleak_log returned false because couldn't acquire the lock */
	u_int64_t total_conflicts;
};
#endif /* PRIVATE */

#ifdef KERNEL_PRIVATE
__BEGIN_DECLS

/*
 * Exported (private)
 */

extern struct mbstat mbstat;                    /* statistics */

__END_DECLS
#endif /* KERNEL_PRIVATE */

#ifdef XNU_KERNEL_PRIVATE
__BEGIN_DECLS

/*
 * Not exported (xnu private)
 */

/* flags to m_get/MGET */
/* Need to include malloc.h to get right options for malloc  */
#include        <sys/malloc.h>

struct mbuf;

/* length to m_copy to copy all */
#define M_COPYALL       1000000000

#define M_DONTWAIT      M_NOWAIT
#define M_WAIT          M_WAITOK

/* modes for m_copym and variants */
#define M_COPYM_NOOP_HDR        0       /* don't copy/move pkthdr contents */
#define M_COPYM_COPY_HDR        1       /* copy pkthdr from old to new */
#define M_COPYM_MOVE_HDR        2       /* move pkthdr from old to new */
#define M_COPYM_MUST_COPY_HDR   3       /* MUST copy pkthdr from old to new */
#define M_COPYM_MUST_MOVE_HDR   4       /* MUST move pkthdr from old to new */

extern void m_freem(struct mbuf *) __XNU_INTERNAL(m_freem);
extern u_int64_t mcl_to_paddr(char *);
extern void m_adj(struct mbuf *, int);
extern void m_cat(struct mbuf *, struct mbuf *);
extern void m_copydata(struct mbuf *, int, int, void *);
extern struct mbuf *m_copym(struct mbuf *, int, int, int);
extern struct mbuf *m_copym_mode(struct mbuf *, int, int, int, uint32_t);
extern struct mbuf *m_get(int, int);
extern struct mbuf *m_gethdr(int, int);
extern struct mbuf *m_getpacket(void);
extern struct mbuf *m_getpackets(int, int, int);
extern struct mbuf *m_mclget(struct mbuf *, int);
extern void *m_mtod(struct mbuf *);
extern struct mbuf *m_prepend_2(struct mbuf *, int, int, int);
extern struct mbuf *m_pullup(struct mbuf *, int);
extern struct mbuf *m_split(struct mbuf *, int, int);
extern void m_mclfree(caddr_t p);

/*
 * On platforms which require strict alignment (currently for anything but
 * i386 or x86_64), this macro checks whether the data pointer of an mbuf
 * is 32-bit aligned (this is the expected minimum alignment for protocol
 * headers), and assert otherwise.
 */
#if defined(__i386__) || defined(__x86_64__)
#define MBUF_STRICT_DATA_ALIGNMENT_CHECK_32(_m)
#else /* !__i386__ && !__x86_64__ */
#define MBUF_STRICT_DATA_ALIGNMENT_CHECK_32(_m) do {                    \
	if (!IS_P2ALIGNED((_m)->m_data, sizeof (u_int32_t))) {          \
	        if (((_m)->m_flags & M_PKTHDR) &&                       \
	            (_m)->m_pkthdr.rcvif != NULL) {                     \
	                panic_plain("\n%s: mbuf %p data ptr %p is not " \
	                    "32-bit aligned [%s: alignerrs=%lld]\n",    \
	                    __func__, (_m), (_m)->m_data,               \
	                    if_name((_m)->m_pkthdr.rcvif),              \
	                    (_m)->m_pkthdr.rcvif->if_alignerrs);        \
	        } else {                                                \
	                panic_plain("\n%s: mbuf %p data ptr %p is not " \
	                    "32-bit aligned\n",                         \
	                    __func__, (_m), (_m)->m_data);              \
	        }                                                       \
	}                                                               \
} while (0)
#endif /* !__i386__ && !__x86_64__ */

/* Maximum number of MBUF_SC values (excluding MBUF_SC_UNSPEC) */
#define MBUF_SC_MAX_CLASSES     10

/*
 * These conversion macros rely on the corresponding MBUF_SC and
 * MBUF_TC values in order to establish the following mapping:
 *
 *	MBUF_SC_BK_SYS	] ==>	MBUF_TC_BK
 *	MBUF_SC_BK	]
 *
 *	MBUF_SC_BE	] ==>	MBUF_TC_BE
 *	MBUF_SC_RD	]
 *	MBUF_SC_OAM	]
 *
 *	MBUF_SC_AV	] ==>	MBUF_TC_VI
 *	MBUF_SC_RV	]
 *	MBUF_SC_VI	]
 *	MBUF_SC_SIG	]
 *
 *	MBUF_SC_VO	] ==>	MBUF_TC_VO
 *	MBUF_SC_CTL	]
 *
 * The values assigned to each service class allows for a fast mapping to
 * the corresponding MBUF_TC traffic class values, as well as to retrieve the
 * assigned index; therefore care must be taken when comparing against these
 * values.  Use the corresponding class and index macros to retrieve the
 * corresponding portion, and never assume that a higher class corresponds
 * to a higher index.
 */
#define MBUF_SCVAL(x)           ((x) & 0xffff)
#define MBUF_SCIDX(x)           ((((x) >> 16) & 0xff) >> 3)
#define MBUF_SC2TC(_sc)         (MBUF_SCVAL(_sc) >> 7)
#define MBUF_TC2SCVAL(_tc)      ((_tc) << 7)
#define IS_MBUF_SC_BACKGROUND(_sc) (((_sc) == MBUF_SC_BK_SYS) || \
	((_sc) == MBUF_SC_BK))
#define IS_MBUF_SC_REALTIME(_sc)        ((_sc) >= MBUF_SC_AV && (_sc) <= MBUF_SC_VO)
#define IS_MBUF_SC_BESTEFFORT(_sc)      ((_sc) == MBUF_SC_BE || \
    (_sc) == MBUF_SC_RD || (_sc) == MBUF_SC_OAM)

#define SCIDX_BK_SYS            MBUF_SCIDX(MBUF_SC_BK_SYS)
#define SCIDX_BK                MBUF_SCIDX(MBUF_SC_BK)
#define SCIDX_BE                MBUF_SCIDX(MBUF_SC_BE)
#define SCIDX_RD                MBUF_SCIDX(MBUF_SC_RD)
#define SCIDX_OAM               MBUF_SCIDX(MBUF_SC_OAM)
#define SCIDX_AV                MBUF_SCIDX(MBUF_SC_AV)
#define SCIDX_RV                MBUF_SCIDX(MBUF_SC_RV)
#define SCIDX_VI                MBUF_SCIDX(MBUF_SC_VI)
#define SCIDX_SIG               MBUF_SCIDX(MBUF_SC_SIG)
#define SCIDX_VO                MBUF_SCIDX(MBUF_SC_VO)
#define SCIDX_CTL               MBUF_SCIDX(MBUF_SC_CTL)

#define SCVAL_BK_SYS            MBUF_SCVAL(MBUF_SC_BK_SYS)
#define SCVAL_BK                MBUF_SCVAL(MBUF_SC_BK)
#define SCVAL_BE                MBUF_SCVAL(MBUF_SC_BE)
#define SCVAL_RD                MBUF_SCVAL(MBUF_SC_RD)
#define SCVAL_OAM               MBUF_SCVAL(MBUF_SC_OAM)
#define SCVAL_AV                MBUF_SCVAL(MBUF_SC_AV)
#define SCVAL_RV                MBUF_SCVAL(MBUF_SC_RV)
#define SCVAL_VI                MBUF_SCVAL(MBUF_SC_VI)
#define SCVAL_SIG               MBUF_SCVAL(MBUF_SC_SIG)
#define SCVAL_VO                MBUF_SCVAL(MBUF_SC_VO)
#define SCVAL_CTL               MBUF_SCVAL(MBUF_SC_CTL)

#define MBUF_VALID_SC(c)                                                \
	(c == MBUF_SC_BK_SYS || c == MBUF_SC_BK || c == MBUF_SC_BE ||   \
	c == MBUF_SC_RD || c == MBUF_SC_OAM || c == MBUF_SC_AV ||       \
	c == MBUF_SC_RV || c == MBUF_SC_VI || c == MBUF_SC_SIG ||       \
	c == MBUF_SC_VO || c == MBUF_SC_CTL)

#define MBUF_VALID_SCIDX(c)                                             \
	(c == SCIDX_BK_SYS || c == SCIDX_BK || c == SCIDX_BE ||         \
	c == SCIDX_RD || c == SCIDX_OAM || c == SCIDX_AV ||             \
	c == SCIDX_RV || c == SCIDX_VI || c == SCIDX_SIG ||             \
	c == SCIDX_VO || c == SCIDX_CTL)

#define MBUF_VALID_SCVAL(c)                                             \
	(c == SCVAL_BK_SYS || c == SCVAL_BK || c == SCVAL_BE ||         \
	c == SCVAL_RD || c == SCVAL_OAM || c == SCVAL_AV ||             \
	c == SCVAL_RV || c == SCVAL_VI || c == SCVAL_SIG ||             \
	c == SCVAL_VO || SCVAL_CTL)

extern unsigned char *mbutl;    /* start VA of mbuf pool */
extern unsigned char *embutl;   /* end VA of mbuf pool */
extern unsigned int nmbclusters;        /* number of mapped clusters */
extern int njcl;                /* # of jumbo clusters  */
extern int njclbytes;   /* size of a jumbo cluster */
extern int max_hdr;             /* largest link+protocol header */
extern int max_datalen; /* MHLEN - max_hdr */

/* Use max_linkhdr instead of _max_linkhdr */
extern int _max_linkhdr;        /* largest link-level header */

/* Use max_protohdr instead of _max_protohdr */
extern int _max_protohdr;       /* largest protocol header */

__private_extern__ unsigned int mbuf_default_ncl(int, u_int64_t);
__private_extern__ void mbinit(void);
__private_extern__ struct mbuf *m_clattach(struct mbuf *, int, caddr_t,
    void (*)(caddr_t, u_int, caddr_t), u_int, caddr_t, int, int);
__private_extern__ caddr_t m_bigalloc(int);
__private_extern__ void m_bigfree(caddr_t, u_int, caddr_t);
__private_extern__ struct mbuf *m_mbigget(struct mbuf *, int);
__private_extern__ caddr_t m_16kalloc(int);
__private_extern__ void m_16kfree(caddr_t, u_int, caddr_t);
__private_extern__ struct mbuf *m_m16kget(struct mbuf *, int);
__private_extern__ int m_reinit(struct mbuf *, int);
__private_extern__ struct mbuf *m_free(struct mbuf *) __XNU_INTERNAL(m_free);
__private_extern__ struct mbuf *m_getclr(int, int);
__private_extern__ struct mbuf *m_getptr(struct mbuf *, int, int *);
__private_extern__ unsigned int m_length(struct mbuf *);
__private_extern__ unsigned int m_length2(struct mbuf *, struct mbuf **);
__private_extern__ unsigned int m_fixhdr(struct mbuf *);
__private_extern__ struct mbuf *m_defrag(struct mbuf *, int);
__private_extern__ struct mbuf *m_defrag_offset(struct mbuf *, u_int32_t, int);
__private_extern__ struct mbuf *m_prepend(struct mbuf *, int, int);
__private_extern__ struct mbuf *m_copyup(struct mbuf *, int, int);
__private_extern__ struct mbuf *m_retry(int, int);
__private_extern__ struct mbuf *m_retryhdr(int, int);
__private_extern__ int m_freem_list(struct mbuf *);
__private_extern__ int m_append(struct mbuf *, int, caddr_t);
__private_extern__ struct mbuf *m_last(struct mbuf *);
__private_extern__ struct mbuf *m_devget(char *, int, int, struct ifnet *,
    void (*)(const void *, void *, size_t));
__private_extern__ struct mbuf *m_pulldown(struct mbuf *, int, int, int *);

__private_extern__ struct mbuf *m_getcl(int, int, int);
__private_extern__ caddr_t m_mclalloc(int);
__private_extern__ int m_mclhasreference(struct mbuf *);
__private_extern__ void m_copy_pkthdr(struct mbuf *, struct mbuf *);
__private_extern__ void m_copy_pftag(struct mbuf *, struct mbuf *);
__private_extern__ void m_copy_classifier(struct mbuf *, struct mbuf *);

__private_extern__ struct mbuf *m_dtom(void *);
__private_extern__ int m_mtocl(void *);
__private_extern__ union mcluster *m_cltom(int);

__private_extern__ void m_align(struct mbuf *, int);

__private_extern__ struct mbuf *m_normalize(struct mbuf *m);
__private_extern__ void m_mchtype(struct mbuf *m, int t);
__private_extern__ void m_mcheck(struct mbuf *);

__private_extern__ void m_copyback(struct mbuf *, int, int, const void *);
__private_extern__ struct mbuf *m_copyback_cow(struct mbuf *, int, int,
    const void *, int);
__private_extern__ int m_makewritable(struct mbuf **, int, int, int);
__private_extern__ struct mbuf *m_dup(struct mbuf *m, int how);
__private_extern__ struct mbuf *m_copym_with_hdrs(struct mbuf *, int, int, int,
    struct mbuf **, int *, uint32_t);
__private_extern__ struct mbuf *m_getpackethdrs(int, int);
__private_extern__ struct mbuf *m_getpacket_how(int);
__private_extern__ struct mbuf *m_getpackets_internal(unsigned int *, int,
    int, int, size_t);
__private_extern__ struct mbuf *m_allocpacket_internal(unsigned int *, size_t,
    unsigned int *, int, int, size_t);

__private_extern__ int m_ext_set_prop(struct mbuf *, uint32_t, uint32_t);
__private_extern__ uint32_t m_ext_get_prop(struct mbuf *);
__private_extern__ int m_ext_paired_is_active(struct mbuf *);
__private_extern__ void m_ext_paired_activate(struct mbuf *);

__private_extern__ void mbuf_drain(boolean_t);

/*
 * Packets may have annotations attached by affixing a list of "packet
 * tags" to the pkthdr structure.  Packet tags are dynamically allocated
 * semi-opaque data structures that have a fixed header (struct m_tag)
 * that specifies the size of the memory block and an <id,type> pair that
 * identifies it. The id identifies the module and the type identifies the
 * type of data for that module. The id of zero is reserved for the kernel.
 *
 * Note that the packet tag returned by m_tag_allocate has the default
 * memory alignment implemented by malloc.  To reference private data one
 * can use a construct like:
 *
 *      struct m_tag *mtag = m_tag_allocate(...);
 *      struct foo *p = (struct foo *)(mtag+1);
 *
 * if the alignment of struct m_tag is sufficient for referencing members
 * of struct foo.  Otherwise it is necessary to embed struct m_tag within
 * the private data structure to insure proper alignment; e.g.
 *
 *      struct foo {
 *              struct m_tag    tag;
 *              ...
 *      };
 *      struct foo *p = (struct foo *) m_tag_allocate(...);
 *      struct m_tag *mtag = &p->tag;
 */

#define KERNEL_MODULE_TAG_ID    0

enum {
	KERNEL_TAG_TYPE_NONE                    = 0,
	KERNEL_TAG_TYPE_DUMMYNET                = 1,
	KERNEL_TAG_TYPE_DIVERT                  = 2,
	KERNEL_TAG_TYPE_IPFORWARD               = 3,
	KERNEL_TAG_TYPE_IPFILT                  = 4,
	KERNEL_TAG_TYPE_MACLABEL                = 5,
	KERNEL_TAG_TYPE_MAC_POLICY_LABEL        = 6,
	KERNEL_TAG_TYPE_ENCAP                   = 8,
	KERNEL_TAG_TYPE_INET6                   = 9,
	KERNEL_TAG_TYPE_IPSEC                   = 10,
	KERNEL_TAG_TYPE_DRVAUX                  = 11,
	KERNEL_TAG_TYPE_CFIL_UDP                = 13,
	KERNEL_TAG_TYPE_PF_REASS                = 14,
};

/* Packet tag routines */
__private_extern__ struct  m_tag *m_tag_alloc(u_int32_t, u_int16_t, int, int);
__private_extern__ struct  m_tag *m_tag_create(u_int32_t, u_int16_t, int, int,
    struct mbuf *);
__private_extern__ void m_tag_free(struct m_tag *);
__private_extern__ void m_tag_prepend(struct mbuf *, struct m_tag *);
__private_extern__ void m_tag_unlink(struct mbuf *, struct m_tag *);
__private_extern__ void m_tag_delete(struct mbuf *, struct m_tag *);
__private_extern__ void m_tag_delete_chain(struct mbuf *, struct m_tag *);
__private_extern__ struct m_tag *m_tag_locate(struct mbuf *, u_int32_t,
    u_int16_t, struct m_tag *);
__private_extern__ struct m_tag *m_tag_copy(struct m_tag *, int);
__private_extern__ int m_tag_copy_chain(struct mbuf *, struct mbuf *, int);
__private_extern__ void m_tag_init(struct mbuf *, int);
__private_extern__ struct  m_tag *m_tag_first(struct mbuf *);
__private_extern__ struct  m_tag *m_tag_next(struct mbuf *, struct m_tag *);

__private_extern__ void m_scratch_init(struct mbuf *);
__private_extern__ u_int32_t m_scratch_get(struct mbuf *, u_int8_t **);

__private_extern__ void m_classifier_init(struct mbuf *, uint32_t);

__private_extern__ int m_set_service_class(struct mbuf *, mbuf_svc_class_t);
__private_extern__ mbuf_svc_class_t m_get_service_class(struct mbuf *);
__private_extern__ mbuf_svc_class_t m_service_class_from_idx(u_int32_t);
__private_extern__ mbuf_svc_class_t m_service_class_from_val(u_int32_t);
__private_extern__ int m_set_traffic_class(struct mbuf *, mbuf_traffic_class_t);
__private_extern__ mbuf_traffic_class_t m_get_traffic_class(struct mbuf *);

#define ADDCARRY(_x)  do {                                              \
	while (((_x) >> 16) != 0)                                       \
	        (_x) = ((_x) >> 16) + ((_x) & 0xffff);                  \
} while (0)

__private_extern__ u_int16_t m_adj_sum16(struct mbuf *, u_int32_t,
    u_int32_t, u_int32_t, u_int32_t);
__private_extern__ u_int16_t m_sum16(struct mbuf *, u_int32_t, u_int32_t);

__private_extern__ void m_set_ext(struct mbuf *, struct ext_ref *,
    m_ext_free_func_t, caddr_t);
__private_extern__ struct ext_ref *m_get_rfa(struct mbuf *);
__private_extern__ m_ext_free_func_t m_get_ext_free(struct mbuf *);
__private_extern__ caddr_t m_get_ext_arg(struct mbuf *);

__private_extern__ void m_do_tx_compl_callback(struct mbuf *, struct ifnet *);
__private_extern__ mbuf_tx_compl_func m_get_tx_compl_callback(u_int32_t);

__END_DECLS
#endif /* XNU_KERNEL_PRIVATE */
#endif  /* !_SYS_MBUF_H_ */

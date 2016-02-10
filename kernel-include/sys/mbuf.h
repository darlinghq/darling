/*
 * Copyright (c) 1999-2011 Apple Inc. All rights reserved.
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

#ifndef	_SYS_MBUF_H_
#define	_SYS_MBUF_H_

#include <sys/cdefs.h>
#include <sys/appleapiopts.h>

#ifdef XNU_KERNEL_PRIVATE

#include <sys/lock.h>
#include <sys/queue.h>

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
#define	_MLEN		(MSIZE - sizeof(struct m_hdr))	/* normal data len */
#define	_MHLEN		(_MLEN - sizeof(struct pkthdr))	/* data len w/pkthdr */

#define	NMBPBGSHIFT	(MBIGCLSHIFT - MSIZESHIFT)
#define	NMBPBG		(1 << NMBPBGSHIFT)	/* # of mbufs per big cl */

#define	NCLPBGSHIFT	(MBIGCLSHIFT - MCLSHIFT)
#define	NCLPBG		(1 << NCLPBGSHIFT)	/* # of cl per big cl */

#define	NMBPCLSHIFT	(NMBPBGSHIFT - NCLPBGSHIFT)
#define	NMBPCL		(1 << NMBPCLSHIFT)	/* # of mbufs per cl */

#define	NCLPJCLSHIFT	((M16KCLSHIFT - MBIGCLSHIFT) + NCLPBGSHIFT)
#define	NCLPJCL		(1 << NCLPJCLSHIFT)	/* # of cl per jumbo cl */

/*
 * Macros for type conversion
 * mtod(m,t) -	convert mbuf pointer to data pointer of correct type
 * dtom(x) -	convert data pointer within mbuf to mbuf pointer (XXX)
 */
#define	mtod(m, t)	((t)m_mtod(m))
#define	dtom(x)		m_dtom(x)

/* header at beginning of each mbuf: */
struct m_hdr {
	struct	mbuf *mh_next;		/* next buffer in chain */
	struct	mbuf *mh_nextpkt;	/* next chain in queue/record */
	int32_t mh_len;			/* amount of data in this mbuf */
	caddr_t	mh_data;		/* location of data */
	short	mh_type;		/* type of data in this mbuf */
	short	mh_flags;		/* flags; see below */
};

/*
 * Packet tag structure (see below for details).
 */
struct m_tag {
	u_int64_t		m_tag_cookie;	/* Error checking */
#ifndef __LP64__
	u_int32_t		pad;		/* For structure alignment */
#endif /* !__LP64__ */
	SLIST_ENTRY(m_tag)	m_tag_link;	/* List of packet tags */
	u_int16_t		m_tag_type;	/* Module specific type */
	u_int16_t		m_tag_len;	/* Length of data */
	u_int32_t		m_tag_id;	/* Module ID */
};

#define	M_TAG_ALIGN(len) \
	(P2ROUNDUP(len, sizeof (u_int64_t)) + sizeof (struct m_tag))

#define	M_TAG_VALID_PATTERN	0xfeedfacefeedfaceULL
#define	M_TAG_FREE_PATTERN	0xdeadbeefdeadbeefULL

/*
 * Packet tag header structure (at the top of mbuf).  Pointers are
 * 32-bit in ILP32; m_tag needs 64-bit alignment, hence padded.
 */
struct m_taghdr {
#ifndef __LP64__
	u_int32_t		pad;		/* For structure alignment */
#endif /* !__LP64__ */
	u_int64_t		refcnt;		/* Number of tags in this mbuf */
};

/* Values for pftag_flags */
#define	PF_TAG_GENERATED	0x000001	/* pkt generated by PF */
#define	PF_TAG_FRAGCACHE	0x000002
#define	PF_TAG_TRANSLATE_LOCALHOST 0x000004
#define	PF_TAG_FLOWHASH		0x000100	/* valid flowhash value */
#define	PF_TAG_HDR_INET		0x000200	/* hdr points to IPv4 */
#define	PF_TAG_HDR_INET6	0x000400	/* hdr points to IPv6 */
#define	PF_TAG_TCP		0x000800	/* payload is TCP */
#define	PF_TAG_FLOWADV		0x010000	/* local flow advisory */
#define	PF_TAG_QUEUE1		0x100000	/* queue-specific */

#define IF_PKTSEQ_SHIFT		4

/* PF mbuf tag */
struct pf_mtag {
	void		*pftag_hdr;	/* saved hdr pos in mbuf, for ECN */
	unsigned int	pftag_rtableid;	/* alternate routing table id */
	union {
		struct {
			u_int32_t	qid;
			union {
				u_int8_t	val8[4];
				u_int16_t	val16[2];
				u_int32_t	val32;
			} __qpriv_u;	/* for queue-specific use */
		} __pf_data;
		u_int64_t	pktseq;
	} __pfifseq_u; /* Used for pf or interface bandwidth measurement */
#define	pftag_qid	__pfifseq_u.__pf_data.qid
#define	pftag_qpriv8	__pfifseq_u.__pf_data.__qpriv_u.val8
#define	pftag_qpriv16	__pfifseq_u.__pf_data.__qpriv_u.val16
#define	pftag_qpriv32	__pfifseq_u.__pf_data.__qpriv_u.val32
#define	pftag_pktseq	__pfifseq_u.pktseq
	u_int32_t	pftag_flowhash;
	u_int16_t	pftag_tag;
	u_int16_t	pftag_routed;
	u_int32_t	pftag_flags;	/* PF_TAG flags */
};

/* TCP specific mbuf tag */
struct tcp_mtag {
	u_int		tm_tso_segz;	/* TSO segment size (actual MSS) */
	u_int16_t	tm_pktlen;	/* LRO - max segment size encountered */
	u_int16_t	tm_npkts;	/* LRO - number of coalesced TCP pkts */
};

/* record/packet header in first mbuf of chain; valid if M_PKTHDR set */
struct	pkthdr {
	int	len;			/* total packet length */
	struct	ifnet *rcvif;		/* rcv interface */

	/* variables for ip and tcp reassembly */
	void	*header;		/* pointer to packet header */
	/* variables for hardware checksum */
	/* Note: csum_flags is used for hardware checksum and VLAN */
	int	csum_flags;		/* flags regarding checksum */
	int	csum_data;		/* data field used by csum routines */
	u_short	vlan_tag;		/* VLAN tag, host byte order */
	u_short socket_id;		/* socket id */
	SLIST_HEAD(packet_tags, m_tag) tags; /* list of packet tags */
	struct pf_mtag pf_mtag;		/* built-in PF tag */
#define m_flowhash	pf_mtag.pftag_flowhash
#define m_fhflags	pf_mtag.pftag_flags
	u_int32_t svc;			/* MBUF_SVC value */
	u_int16_t vt_nrecs;		/* # of IGMPv3/MLDv2 records */
	u_int16_t aux_flags;		/* auxiliary packet flags */
	struct tcp_mtag	tcp_mtag;	/* tcp related data */
#define tso_segsz	tcp_mtag.tm_tso_segz	
#define lro_pktlen	tcp_mtag.tm_pktlen	
#define lro_npkts	tcp_mtag.tm_npkts
};

/* description of external storage mapped into mbuf, valid if M_EXT set */
struct m_ext {
	caddr_t	ext_buf;		/* start of buffer */
	void	(*ext_free)(caddr_t, u_int, caddr_t);	/* free routine if not the usual */
	u_int	ext_size;		/* size of buffer, for ext_free */
	caddr_t	ext_arg;		/* additional ext_free argument */
	struct	ext_refsq {		/* references held */
		struct ext_refsq *forward, *backward;
	} ext_refs;
	struct ext_ref {
		u_int32_t refcnt;
		u_int32_t flags;
	} *ext_refflags;
};

/* define m_ext to a type since it gets redefined below */
typedef struct m_ext _m_ext_t;

struct mbuf {
	struct	m_hdr m_hdr;
	union {
		struct {
			struct	pkthdr MH_pkthdr;	/* M_PKTHDR set */
			union {
				struct	m_ext MH_ext;	/* M_EXT set */
				char	MH_databuf[_MHLEN];
			} MH_dat;
		} MH;
		char	M_databuf[_MLEN];		/* !M_PKTHDR, !M_EXT */
	} M_dat;
};

#define	m_next		m_hdr.mh_next
#define	m_len		m_hdr.mh_len
#define	m_data		m_hdr.mh_data
#define	m_type		m_hdr.mh_type
#define	m_flags		m_hdr.mh_flags
#define	m_nextpkt	m_hdr.mh_nextpkt
#define	m_act		m_nextpkt
#define	m_pkthdr	M_dat.MH.MH_pkthdr
#define	m_ext		M_dat.MH.MH_dat.MH_ext
#define	m_pktdat	M_dat.MH.MH_dat.MH_databuf
#define	m_dat		M_dat.M_databuf
#define	m_pktlen(_m)	((_m)->m_pkthdr.len)
#define	m_pftag(_m)	(&(_m)->m_pkthdr.pf_mtag)

/* mbuf flags (private) */
#define	M_EXT		0x0001	/* has associated external storage */
#define	M_PKTHDR	0x0002	/* start of record */
#define	M_EOR		0x0004	/* end of record */
#define	M_PROTO1	0x0008	/* protocol-specific */
#define	M_PROTO2	0x0010	/* protocol-specific */
#define	M_PROTO3	0x0020	/* protocol-specific */
#define	M_LOOP		0x0040	/* packet is looped back */
#define	M_PROTO5	0x0080	/* protocol-specific */

/* mbuf pkthdr flags, also in m_flags (private) */
#define	M_BCAST		0x0100	/* send/received as link-level broadcast */
#define	M_MCAST		0x0200	/* send/received as link-level multicast */
#define	M_FRAG		0x0400	/* packet is a fragment of a larger packet */
#define	M_FIRSTFRAG	0x0800	/* packet is first fragment */
#define	M_LASTFRAG	0x1000	/* packet is last fragment */
#define	M_PROMISC	0x2000	/* packet is promiscuous (shouldn't go to stack) */
#define	M_HASFCS	0x4000	/* packet has FCS */
#define	M_TAGHDR	0x8000	/* m_tag hdr structure at top of mbuf data */

/*
 * Flags to purge when crossing layers.
 */
#define	M_PROTOFLAGS \
	(M_PROTO1|M_PROTO2|M_PROTO3|M_PROTO5)

/* flags copied when copying m_pkthdr */
#define	M_COPYFLAGS							\
	(M_PKTHDR|M_EOR|M_PROTO1|M_PROTO2|M_PROTO3 |			\
	M_LOOP|M_PROTO5|M_BCAST|M_MCAST|M_FRAG |			\
	M_FIRSTFRAG|M_LASTFRAG|M_PROMISC|M_HASFCS)

/* flags indicating hw checksum support and sw checksum requirements */
#define	CSUM_IP			0x0001		/* will csum IP */
#define	CSUM_TCP		0x0002		/* will csum TCP */
#define	CSUM_UDP		0x0004		/* will csum UDP */
#define	CSUM_IP_FRAGS		0x0008		/* will csum IP fragments */
#define	CSUM_FRAGMENT		0x0010		/* will do IP fragmentation */
#define	CSUM_TCPIPV6		0x0020		/* will csum TCP for IPv6 */
#define	CSUM_UDPIPV6		0x0040		/* will csum UDP for IPv6 */
#define	CSUM_FRAGMENT_IPV6	0x0080		/* will do IPv6 fragmentation */

#define	CSUM_IP_CHECKED		0x0100		/* did csum IP */
#define	CSUM_IP_VALID		0x0200		/*   ... the csum is valid */
#define	CSUM_DATA_VALID		0x0400		/* csum_data field is valid */
#define	CSUM_PSEUDO_HDR		0x0800		/* csum_data has pseudo hdr */
#define	CSUM_TCP_SUM16		0x1000		/* simple TCP Sum16 computation */

#define	CSUM_DELAY_DATA		(CSUM_TCP | CSUM_UDP)
#define	CSUM_DELAY_IP		(CSUM_IP)	/* IPv4 only: no IPv6 IP cksum */
#define	CSUM_DELAY_IPV6_DATA	(CSUM_TCPIPV6 | CSUM_UDPIPV6)
#define	CSUM_DATA_IPV6_VALID    CSUM_DATA_VALID	/* csum_data field is valid */
/*
 * Note: see also IF_HWASSIST_CSUM defined in <net/if_var.h>
 */
/* bottom 16 bits reserved for hardware checksum */
#define	CSUM_CHECKSUM_MASK	0xffff

/* VLAN tag present */
#define	CSUM_VLAN_TAG_VALID	0x10000		/* vlan_tag field is valid */

/* TCP Segment Offloading requested on this mbuf */
#define	CSUM_TSO_IPV4		0x100000	/* This mbuf needs to be segmented by the NIC */
#define	CSUM_TSO_IPV6		0x200000	/* This mbuf needs to be segmented by the NIC */

/*
 * Auxiliary packet flags.  Unlike m_flags, all auxiliary flags are copied
 * along when copying m_pkthdr, i.e. no equivalent of M_COPYFLAGS here.
 * Note that this flag is 16-bit wide.
 */
#define	MAUXF_PRIO_PRIVILEGED	0x0001	/* packet priority is privileged */
#define	MAUXF_PROXY_DST		0x0002	/* processed but not locally destined */
#define	MAUXF_INET_RESOLVE_RTR	0x0004	/* pkt is for resolving IPv4 router */
#define	MAUXF_INET6_RESOLVE_RTR	0x0008	/* pkt is for resolving IPv6 router */
#define	MAUXF_SW_LRO_PKT	0x0010	/* pkt is a large coalesced pkt */
#define	MAUXF_SW_LRO_DID_CSUM	0x0020	/* IP and TCP checksums done by LRO*/
#endif /* XNU_KERNEL_PRIVATE */

/* mbuf types */
#define	MT_FREE		0	/* should be on free list */
#define	MT_DATA		1	/* dynamic (data) allocation */
#define	MT_HEADER	2	/* packet header */
#define	MT_SOCKET	3	/* socket structure */
#define	MT_PCB		4	/* protocol control block */
#define	MT_RTABLE	5	/* routing tables */
#define	MT_HTABLE	6	/* IMP host tables */
#define	MT_ATABLE	7	/* address resolution tables */
#define	MT_SONAME	8	/* socket name */
#define	MT_SOOPTS	10	/* socket options */
#define	MT_FTABLE	11	/* fragment reassembly header */
#define	MT_RIGHTS	12	/* access rights */
#define	MT_IFADDR	13	/* interface address */
#define	MT_CONTROL	14	/* extra-data protocol message */
#define	MT_OOBDATA	15	/* expedited data  */
#define	MT_TAG		16	/* volatile metadata associated to pkts */
#define	MT_MAX		32	/* enough? */

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
#define	MCHECK(m) m_mcheck(m)
#else
#define	MCHECK(m)
#endif

#define	MGET(m, how, type) ((m) = m_get((how), (type)))

#define	MGETHDR(m, how, type)	((m) = m_gethdr((how), (type)))

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
	union	mcluster *mcl_next;
	char	mcl_buf[MCLBYTES];
};

#define	MCLALLOC(p, how)	((p) = m_mclalloc(how))

#define	MCLFREE(p)		m_mclfree(p)

#define	MCLGET(m, how)		((m) = m_mclget(m, how))

/*
 * Mbuf big cluster
 */
union mbigcluster {
	union mbigcluster	*mbc_next;
	char			mbc_buf[MBIGCLBYTES];
};

/*
 * Mbuf jumbo cluster
 */
union m16kcluster {
	union m16kcluster	*m16kcl_next;
	char			m16kcl_buf[M16KCLBYTES];
};

#define	MCLHASREFERENCE(m)	m_mclhasreference(m)

/*
 * MFREE(struct mbuf *m, struct mbuf *n)
 * Free a single mbuf and associated external storage.
 * Place the successor, if any, in n.
 */

#define	MFREE(m, n) ((n) = m_free(m))

/*
 * Copy mbuf pkthdr from from to to.
 * from must have M_PKTHDR set, and to must be empty.
 * aux pointer will be moved to `to'.
 */
#define	M_COPY_PKTHDR(to, from)		m_copy_pkthdr(to, from)

#define M_COPY_PFTAG(to, from)		m_copy_pftag(to, from)

/*
 * Set the m_data pointer of a newly-allocated mbuf (m_get/MGET) to place
 * an object of the specified size at the end of the mbuf, longword aligned.
 */
#define	M_ALIGN(m, len)							\
do {									\
	(m)->m_data += (MLEN - (len)) &~ (sizeof (long) - 1);		\
} while (0)

/*
 * As above, for mbufs allocated with m_gethdr/MGETHDR
 * or initialized by M_COPY_PKTHDR.
 */
#define	MH_ALIGN(m, len)						\
do {									\
	(m)->m_data += (MHLEN - (len)) &~ (sizeof (long) - 1);		\
} while (0)

/*
 * Compute the amount of space available
 * before the current start of data in an mbuf.
 * Subroutine - data not available if certain references.
 */
#define	M_LEADINGSPACE(m)	m_leadingspace(m)

/*
 * Compute the amount of space available
 * after the end of data in an mbuf.
 * Subroutine - data not available if certain references.
 */
#define	M_TRAILINGSPACE(m)	m_trailingspace(m)

/*
 * Arrange to prepend space of size plen to mbuf m.
 * If a new mbuf must be allocated, how specifies whether to wait.
 * If how is M_DONTWAIT and allocation fails, the original mbuf chain
 * is freed and m is set to NULL.
 */
#define	M_PREPEND(m, plen, how)	((m) = m_prepend_2((m), (plen), (how)))

/* change mbuf to new type */
#define	MCHTYPE(m, t)		m_mchtype(m, t)

/* compatiblity with 4.3 */
#define	m_copy(m, o, l)		m_copym((m), (o), (l), M_DONTWAIT)

#define	MBSHIFT		20				/* 1MB */
#define	MBSIZE		(1 << MBSHIFT)
#define	GBSHIFT		30				/* 1GB */
#define	GBSIZE		(1 << GBSHIFT)

/*
 * M_STRUCT_GET ensures that intermediate protocol header (from "off" to
 * "off+len") is located in single mbuf, on contiguous memory region.
 * The pointer to the region will be returned to pointer variable "val",
 * with type "typ".
 *
 * M_STRUCT_GET0 does the same, except that it aligns the structure at
 * very top of mbuf.  GET0 is likely to make memory copy than GET.
 */
#define	M_STRUCT_GET(val, typ, m, off, len)				\
do {									\
	struct mbuf *t;							\
	int tmp;							\
									\
	if ((m)->m_len >= (off) + (len)) {				\
		(val) = (typ)(mtod((m), caddr_t) + (off));		\
	} else {							\
		t = m_pulldown((m), (off), (len), &tmp);		\
		if (t != NULL) {					\
			if (t->m_len < tmp + (len))			\
				panic("m_pulldown malfunction");	\
			(val) = (typ)(mtod(t, caddr_t) + tmp);		\
		} else {						\
			(val) = (typ)NULL;				\
			(m) = NULL;					\
		}							\
	}								\
} while (0)

#define	M_STRUCT_GET0(val, typ, m, off, len)				\
do {									\
	struct mbuf *t;							\
									\
	if ((off) == 0 && ((m)->m_len >= (len))) {			\
		(val) = (typ)(void *)mtod(m, caddr_t);			\
	} else {							\
		t = m_pulldown((m), (off), (len), NULL);		\
		if (t != NULL) {					\
			if (t->m_len < (len))				\
				panic("m_pulldown malfunction");	\
			(val) = (typ)(void *)mtod(t, caddr_t);		\
		} else {						\
			(val) = (typ)NULL;				\
			(m) = NULL;					\
		}							\
	}								\
} while (0)

#define	MBUF_INPUT_CHECK(m, rcvif)					\
do {									\
	if (!(m->m_flags & MBUF_PKTHDR) ||				\
	    m->m_len < 0 ||						\
	    m->m_len > ((njcl > 0) ? njclbytes : MBIGCLBYTES) ||	\
	    m->m_type == MT_FREE ||					\
	    ((m->m_flags & M_EXT) != 0 && m->m_ext.ext_buf == NULL)) {	\
		panic_plain("Failed mbuf validity check: mbuf %p len %d "  \
		    "type %d flags 0x%x data %p rcvif %s%d ifflags 0x%x",  \
		    m, m->m_len, m->m_type, m->m_flags,			   \
		    ((m->m_flags & M_EXT) ? m->m_ext.ext_buf : m->m_data), \
		    rcvif->if_name, rcvif->if_unit,			\
		    (rcvif->if_flags & 0xffff));			\
	}								\
} while (0)

/*
 * Simple mbuf queueing system
 *
 * This is basically a SIMPLEQ adapted to mbuf use (i.e. using
 * m_nextpkt instead of field.sqe_next).
 *
 * m_next is ignored, so queueing chains of mbufs is possible
 */
#define MBUFQ_HEAD(name)					\
struct name {							\
	struct mbuf *mq_first;	/* first packet */		\
	struct mbuf **mq_last;	/* addr of last next packet */	\
}

#define MBUFQ_INIT(q)		do {				\
	MBUFQ_FIRST(q) = NULL;					\
	(q)->mq_last = &MBUFQ_FIRST(q);				\
} while (0)

#define MBUFQ_PREPEND(q, m)	do {				\
	if ((MBUFQ_NEXT(m) = MBUFQ_FIRST(q)) == NULL)		\
		(q)->mq_last = &MBUFQ_NEXT(m);			\
	MBUFQ_FIRST(q) = (m);					\
} while (0)

#define MBUFQ_ENQUEUE(q, m)	do {				\
	MBUFQ_NEXT(m) = NULL;					\
	*(q)->mq_last = (m);					\
	(q)->mq_last = &MBUFQ_NEXT(m);				\
} while (0)

#define MBUFQ_ENQUEUE_MULTI(q, m, n)	do {			\
	MBUFQ_NEXT(n) = NULL;					\
	*(q)->mq_last = (m);					\
	(q)->mq_last = &MBUFQ_NEXT(n);				\
} while (0)

#define MBUFQ_DEQUEUE(q, m)	do {				\
	if (((m) = MBUFQ_FIRST(q)) != NULL) {			\
		if ((MBUFQ_FIRST(q) = MBUFQ_NEXT(m)) == NULL)	\
			(q)->mq_last = &MBUFQ_FIRST(q);		\
		else						\
			MBUFQ_NEXT(m) = NULL;			\
	}							\
} while (0)

#define	MBUFQ_REMOVE(q, m)	do {				\
	if (MBUFQ_FIRST(q) == (m)) {				\
		MBUFQ_DEQUEUE(q, m);				\
	} else {						\
		struct mbuf *_m = MBUFQ_FIRST(q);		\
		while (MBUFQ_NEXT(_m) != (m))			\
			_m = MBUFQ_NEXT(_m);			\
		if ((MBUFQ_NEXT(_m) =				\
		    MBUFQ_NEXT(MBUFQ_NEXT(_m))) == NULL)	\
			(q)->mq_last = &MBUFQ_NEXT(_m);		\
	}							\
} while (0)

#define MBUFQ_DRAIN(q)		do {				\
	struct mbuf *__m0;					\
	while ((__m0 = MBUFQ_FIRST(q)) != NULL) {		\
		MBUFQ_FIRST(q) = MBUFQ_NEXT(__m0);		\
		MBUFQ_NEXT(__m0) = NULL;			\
		m_freem(__m0);					\
	}							\
	(q)->mq_last = &MBUFQ_FIRST(q);				\
} while (0)

#define	MBUFQ_FOREACH(m, q)					\
	for ((m) = MBUFQ_FIRST(q);				\
	    (m);						\
	    (m) = MBUFQ_NEXT(m))

#define	MBUFQ_FOREACH_SAFE(m, q, tvar)				\
	for ((m) = MBUFQ_FIRST(q);				\
	    (m) && ((tvar) = MBUFQ_NEXT(m), 1);			\
	    (m) = (tvar))

#define	MBUFQ_EMPTY(q)		((q)->mq_first == NULL)
#define MBUFQ_FIRST(q)		((q)->mq_first)
#define MBUFQ_NEXT(m)		((m)->m_nextpkt)
#define MBUFQ_LAST(q)		(*(q)->mq_last)

#define	max_linkhdr	P2ROUNDUP(_max_linkhdr, sizeof (u_int32_t))
#define	max_protohdr	P2ROUNDUP(_max_protohdr, sizeof (u_int32_t))
#endif /* XNU_KERNEL_PRIVATE */

/*
 * Mbuf statistics (legacy).
 */
struct mbstat {
	u_int32_t	m_mbufs;	/* mbufs obtained from page pool */
	u_int32_t	m_clusters;	/* clusters obtained from page pool */
	u_int32_t	m_spare;	/* spare field */
	u_int32_t	m_clfree;	/* free clusters */
	u_int32_t	m_drops;	/* times failed to find space */
	u_int32_t	m_wait;		/* times waited for space */
	u_int32_t	m_drain;	/* times drained protocols for space */
	u_short		m_mtypes[256];	/* type specific mbuf allocations */
	u_int32_t	m_mcfail;	/* times m_copym failed */
	u_int32_t	m_mpfail;	/* times m_pullup failed */
	u_int32_t	m_msize;	/* length of an mbuf */
	u_int32_t	m_mclbytes;	/* length of an mbuf cluster */
	u_int32_t	m_minclsize;	/* min length of data to allocate a cluster */
	u_int32_t	m_mlen;		/* length of data in an mbuf */
	u_int32_t	m_mhlen;	/* length of data in a header mbuf */
	u_int32_t	m_bigclusters;	/* clusters obtained from page pool */
	u_int32_t	m_bigclfree;	/* free clusters */
	u_int32_t	m_bigmclbytes;	/* length of an mbuf cluster */
};

/* Compatibillity with 10.3 */
struct ombstat {
	u_int32_t	m_mbufs;	/* mbufs obtained from page pool */
	u_int32_t	m_clusters;	/* clusters obtained from page pool */
	u_int32_t	m_spare;	/* spare field */
	u_int32_t	m_clfree;	/* free clusters */
	u_int32_t	m_drops;	/* times failed to find space */
	u_int32_t	m_wait;		/* times waited for space */
	u_int32_t	m_drain;	/* times drained protocols for space */
	u_short		m_mtypes[256];	/* type specific mbuf allocations */
	u_int32_t	m_mcfail;	/* times m_copym failed */
	u_int32_t	m_mpfail;	/* times m_pullup failed */
	u_int32_t	m_msize;	/* length of an mbuf */
	u_int32_t	m_mclbytes;	/* length of an mbuf cluster */
	u_int32_t	m_minclsize;	/* min length of data to allocate a cluster */
	u_int32_t	m_mlen;		/* length of data in an mbuf */
	u_int32_t	m_mhlen;	/* length of data in a header mbuf */
};

/*
 * mbuf class statistics.
 */
#define	MAX_MBUF_CNAME	15

#if defined(XNU_KERNEL_PRIVATE)
/* For backwards compatibility with 32-bit userland process */
struct omb_class_stat {
	char		mbcl_cname[MAX_MBUF_CNAME + 1]; /* class name */
	u_int32_t	mbcl_size;	/* buffer size */
	u_int32_t	mbcl_total;	/* # of buffers created */
	u_int32_t	mbcl_active;	/* # of active buffers */
	u_int32_t	mbcl_infree;	/* # of available buffers */
	u_int32_t	mbcl_slab_cnt;	/* # of available slabs */
	u_int64_t	mbcl_alloc_cnt;	/* # of times alloc is called */
	u_int64_t	mbcl_free_cnt;	/* # of times free is called */
	u_int64_t	mbcl_notified;	/* # of notified wakeups */
	u_int64_t	mbcl_purge_cnt;	/* # of purges so far */
	u_int64_t	mbcl_fail_cnt;	/* # of allocation failures */
	u_int32_t	mbcl_ctotal;	/* total only for this class */
	/*
	 * Cache layer statistics
	 */
	u_int32_t	mbcl_mc_state;	/* cache state (see below) */
	u_int32_t	mbcl_mc_cached;	/* # of cached buffers */
	u_int32_t	mbcl_mc_waiter_cnt;  /* # waiters on the cache */
	u_int32_t	mbcl_mc_wretry_cnt;  /* # of wait retries */
	u_int32_t	mbcl_mc_nwretry_cnt; /* # of no-wait retry attempts */
	u_int64_t	mbcl_reserved[4];    /* for future use */
} __attribute__((__packed__));
#endif /* XNU_KERNEL_PRIVATE */

typedef struct mb_class_stat {
	char		mbcl_cname[MAX_MBUF_CNAME + 1]; /* class name */
	u_int32_t	mbcl_size;	/* buffer size */
	u_int32_t	mbcl_total;	/* # of buffers created */
	u_int32_t	mbcl_active;	/* # of active buffers */
	u_int32_t	mbcl_infree;	/* # of available buffers */
	u_int32_t	mbcl_slab_cnt;	/* # of available slabs */
#if defined(KERNEL) || defined(__LP64__)
	u_int32_t	mbcl_pad;	/* padding */
#endif /* KERNEL || __LP64__ */
	u_int64_t	mbcl_alloc_cnt;	/* # of times alloc is called */
	u_int64_t	mbcl_free_cnt;	/* # of times free is called */
	u_int64_t	mbcl_notified;	/* # of notified wakeups */
	u_int64_t	mbcl_purge_cnt;	/* # of purges so far */
	u_int64_t	mbcl_fail_cnt;	/* # of allocation failures */
	u_int32_t	mbcl_ctotal;	/* total only for this class */
	/*
	 * Cache layer statistics
	 */
	u_int32_t	mbcl_mc_state;	/* cache state (see below) */
	u_int32_t	mbcl_mc_cached;	/* # of cached buffers */
	u_int32_t	mbcl_mc_waiter_cnt;  /* # waiters on the cache */
	u_int32_t	mbcl_mc_wretry_cnt;  /* # of wait retries */
	u_int32_t	mbcl_mc_nwretry_cnt; /* # of no-wait retry attempts */
	u_int64_t	mbcl_reserved[4];    /* for future use */
} mb_class_stat_t;

#define	MCS_DISABLED	0	/* cache is permanently disabled */
#define	MCS_ONLINE	1	/* cache is online */
#define	MCS_PURGING	2	/* cache is being purged */
#define	MCS_OFFLINE	3	/* cache is offline (resizing) */

#if defined(XNU_KERNEL_PRIVATE)
/* For backwards compatibility with 32-bit userland process */
struct omb_stat {
	u_int32_t		mbs_cnt;	/* number of classes */
	struct omb_class_stat	mbs_class[1];	/* class array */
} __attribute__((__packed__));
#endif /* XNU_KERNEL_PRIVATE */

typedef struct mb_stat {
	u_int32_t	mbs_cnt;	/* number of classes */
#if defined(KERNEL) || defined(__LP64__)
	u_int32_t	mbs_pad;	/* padding */
#endif /* KERNEL || __LP64__ */
	mb_class_stat_t	mbs_class[1];	/* class array */
} mb_stat_t;

#ifdef PRIVATE
#define	MLEAK_STACK_DEPTH	16	/* Max PC stack depth */

typedef struct mleak_trace_stat {
	u_int64_t	mltr_collisions;
	u_int64_t	mltr_hitcount;
	u_int64_t	mltr_allocs;
	u_int64_t	mltr_depth;
	u_int64_t	mltr_addr[MLEAK_STACK_DEPTH];
} mleak_trace_stat_t;

typedef struct mleak_stat {
	u_int32_t		ml_isaddr64;	/* 64-bit KVA? */
	u_int32_t		ml_cnt;		/* number of traces */
	mleak_trace_stat_t	ml_trace[1];	/* trace array */
} mleak_stat_t;

struct mleak_table {
	u_int32_t mleak_capture;	/* sampling capture counter */
	u_int32_t mleak_sample_factor;	/* sample factor */

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

extern struct mbstat mbstat;			/* statistics */

__END_DECLS
#endif /* KERNEL_PRIVATE */

#ifdef XNU_KERNEL_PRIVATE
__BEGIN_DECLS

/*
 * Not exported (xnu private)
 */

/* flags to m_get/MGET */
/* Need to include malloc.h to get right options for malloc  */
#include	<sys/malloc.h>

struct mbuf;

/* length to m_copy to copy all */
#define	M_COPYALL	1000000000

#define	M_DONTWAIT	M_NOWAIT
#define	M_WAIT		M_WAITOK

/*
 * These macros are mapped to the appropriate KPIs, so that private code
 * can be simply recompiled in order to be forward-compatible with future
 * changes toward the struture sizes.
 */
#define	MLEN		mbuf_get_mlen()		/* normal data len */
#define	MHLEN		mbuf_get_mhlen()	/* data len w/pkthdr */

#define	MINCLSIZE	mbuf_get_minclsize()	/* cluster usage threshold */

extern void m_freem(struct mbuf *);
extern char *mcl_to_paddr(char *);
extern void m_adj(struct mbuf *, int);
extern void m_cat(struct mbuf *, struct mbuf *);
extern void m_copydata(struct mbuf *, int, int, void *);
extern struct mbuf *m_copym(struct mbuf *, int, int, int);
extern struct mbuf *m_get(int, int);
extern struct mbuf *m_gethdr(int, int);
extern struct mbuf *m_getpacket(void);
extern struct mbuf *m_getpackets(int, int, int);
extern struct mbuf *m_mclget(struct mbuf *, int);
extern void *m_mtod(struct mbuf *);
extern struct mbuf *m_prepend_2(struct mbuf *, int, int);
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
#define	MBUF_STRICT_DATA_ALIGNMENT_CHECK_32(_m)
#else /* !__i386__ && !__x86_64__ */
#define	MBUF_STRICT_DATA_ALIGNMENT_CHECK_32(_m) do {			\
	if (!IS_P2ALIGNED((_m)->m_data, sizeof (u_int32_t))) {		\
		if (((_m)->m_flags & M_PKTHDR) &&			\
		    (_m)->m_pkthdr.rcvif != NULL) {			\
			panic_plain("\n%s: mbuf %p data ptr %p is not "	\
			    "32-bit aligned [%s%d: alignerrs=%lld]\n",	\
			    __func__, (_m), (_m)->m_data,		\
			    (_m)->m_pkthdr.rcvif->if_name,		\
			    (_m)->m_pkthdr.rcvif->if_unit,		\
			    (_m)->m_pkthdr.rcvif->if_alignerrs);	\
		} else {						\
			panic_plain("\n%s: mbuf %p data ptr %p is not "	\
			    "32-bit aligned\n",				\
			    __func__, (_m), (_m)->m_data);		\
		}							\
	}								\
} while (0)
#endif /* !__i386__ && !__x86_64__ */

/* Maximum number of MBUF_SC values (excluding MBUF_SC_UNSPEC) */
#define	MBUF_SC_MAX_CLASSES	10

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
#define	MBUF_SCVAL(x)		((x) & 0xffff)
#define	MBUF_SCIDX(x)		((((x) >> 16) & 0xff) >> 3)
#define	MBUF_SC2TC(_sc)		(MBUF_SCVAL(_sc) >> 7)
#define	MBUF_TC2SCVAL(_tc)	((_tc) << 7)
#define IS_MBUF_SC_BACKGROUND(_sc) (((_sc) == MBUF_SC_BK_SYS) || \
	((_sc) == MBUF_SC_BK))

#define	SCIDX_BK_SYS		MBUF_SCIDX(MBUF_SC_BK_SYS)
#define	SCIDX_BK		MBUF_SCIDX(MBUF_SC_BK)
#define	SCIDX_BE		MBUF_SCIDX(MBUF_SC_BE)
#define	SCIDX_RD		MBUF_SCIDX(MBUF_SC_RD)
#define	SCIDX_OAM		MBUF_SCIDX(MBUF_SC_OAM)
#define	SCIDX_AV		MBUF_SCIDX(MBUF_SC_AV)
#define	SCIDX_RV		MBUF_SCIDX(MBUF_SC_RV)
#define	SCIDX_VI		MBUF_SCIDX(MBUF_SC_VI)
#define	SCIDX_VO		MBUF_SCIDX(MBUF_SC_VO)
#define	SCIDX_CTL		MBUF_SCIDX(MBUF_SC_CTL)

#define	SCVAL_BK_SYS		MBUF_SCVAL(MBUF_SC_BK_SYS)
#define	SCVAL_BK		MBUF_SCVAL(MBUF_SC_BK)
#define	SCVAL_BE		MBUF_SCVAL(MBUF_SC_BE)
#define	SCVAL_RD		MBUF_SCVAL(MBUF_SC_RD)
#define	SCVAL_OAM		MBUF_SCVAL(MBUF_SC_OAM)
#define	SCVAL_AV		MBUF_SCVAL(MBUF_SC_AV)
#define	SCVAL_RV		MBUF_SCVAL(MBUF_SC_RV)
#define	SCVAL_VI		MBUF_SCVAL(MBUF_SC_VI)
#define	SCVAL_VO		MBUF_SCVAL(MBUF_SC_VO)
#define	SCVAL_CTL		MBUF_SCVAL(MBUF_SC_CTL)

#define	MBUF_VALID_SC(c)						\
	(c == MBUF_SC_BK_SYS || c == MBUF_SC_BK || c == MBUF_SC_BE ||	\
	c == MBUF_SC_RD || c == MBUF_SC_OAM || c == MBUF_SC_AV ||	\
	c == MBUF_SC_RV || c == MBUF_SC_VI || c == MBUF_SC_VO ||	\
	c == MBUF_SC_CTL)

#define	MBUF_VALID_SCIDX(c)						\
	(c == SCIDX_BK_SYS || c == SCIDX_BK || c == SCIDX_BE ||		\
	c == SCIDX_RD || c == SCIDX_OAM || c == SCIDX_AV ||		\
	c == SCIDX_RV || c == SCIDX_VI || c == SCIDX_VO ||		\
	c == SCIDX_CTL)

#define	MBUF_VALID_SCVAL(c)						\
	(c == SCVAL_BK_SYS || c == SCVAL_BK || c == SCVAL_BE ||		\
	c == SCVAL_RD || c == SCVAL_OAM || c == SCVAL_AV ||		\
	c == SCVAL_RV || c == SCVAL_VI || c == SCVAL_VO ||		\
	c == SCVAL_CTL)

__private_extern__ union mbigcluster *mbutl;	/* start VA of mbuf pool */
__private_extern__ union mbigcluster *embutl;	/* end VA of mbuf pool */
__private_extern__ unsigned int nmbclusters;	/* number of mapped clusters */
__private_extern__ int njcl;		/* # of jumbo clusters  */
__private_extern__ int njclbytes;	/* size of a jumbo cluster */
__private_extern__ int max_hdr;		/* largest link+protocol header */
__private_extern__ int max_datalen;	/* MHLEN - max_hdr */

/* Use max_linkhdr instead of _max_linkhdr */
__private_extern__ int _max_linkhdr;	/* largest link-level header */

/* Use max_protohdr instead of _max_protohdr */
__private_extern__ int _max_protohdr;	/* largest protocol header */

__private_extern__ unsigned int mbuf_default_ncl(int, u_int64_t);
__private_extern__ void mbinit(void);
__private_extern__ struct mbuf *m_clattach(struct mbuf *, int, caddr_t,
    void (*)(caddr_t, u_int, caddr_t), u_int, caddr_t, int);
__private_extern__ caddr_t m_bigalloc(int);
__private_extern__ void m_bigfree(caddr_t, u_int, caddr_t);
__private_extern__ struct mbuf *m_mbigget(struct mbuf *, int);
__private_extern__ caddr_t m_16kalloc(int);
__private_extern__ void m_16kfree(caddr_t, u_int, caddr_t);
__private_extern__ struct mbuf *m_m16kget(struct mbuf *, int);

__private_extern__ struct mbuf *m_free(struct mbuf *);
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

__private_extern__ struct mbuf *m_dtom(void *);
__private_extern__ int m_mtocl(void *);
__private_extern__ union mcluster *m_cltom(int);

__private_extern__ int m_trailingspace(struct mbuf *);
__private_extern__ int m_leadingspace(struct mbuf *);

__private_extern__ struct mbuf *m_normalize(struct mbuf *m);
__private_extern__ void m_mchtype(struct mbuf *m, int t);
__private_extern__ void m_mcheck(struct mbuf *);

__private_extern__ void m_copyback(struct mbuf *, int, int, const void *);
__private_extern__ struct mbuf *m_copyback_cow(struct mbuf *, int, int,
    const void *, int);
__private_extern__ int m_makewritable(struct mbuf **, int, int, int);
__private_extern__ struct mbuf *m_dup(struct mbuf *m, int how);
__private_extern__ struct mbuf *m_copym_with_hdrs(struct mbuf *, int, int, int,
    struct mbuf **, int *);
__private_extern__ struct mbuf *m_getpackethdrs(int, int);
__private_extern__ struct mbuf *m_getpacket_how(int);
__private_extern__ struct mbuf *m_getpackets_internal(unsigned int *, int,
    int, int, size_t);
__private_extern__ struct mbuf *m_allocpacket_internal(unsigned int *, size_t,
    unsigned int *, int, int, size_t);

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

#define	KERNEL_MODULE_TAG_ID	0

enum {
	KERNEL_TAG_TYPE_NONE			= 0,
	KERNEL_TAG_TYPE_DUMMYNET		= 1,
	KERNEL_TAG_TYPE_DIVERT			= 2,
	KERNEL_TAG_TYPE_IPFORWARD		= 3,
	KERNEL_TAG_TYPE_IPFILT			= 4,
	KERNEL_TAG_TYPE_MACLABEL		= 5,
	KERNEL_TAG_TYPE_MAC_POLICY_LABEL	= 6,
	KERNEL_TAG_TYPE_ENCAP			= 8,
	KERNEL_TAG_TYPE_INET6			= 9,
	KERNEL_TAG_TYPE_IPSEC			= 10,
	KERNEL_TAG_TYPE_PF			= 11
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
__private_extern__ void m_tag_init(struct mbuf *);
__private_extern__ struct  m_tag *m_tag_first(struct mbuf *);
__private_extern__ struct  m_tag *m_tag_next(struct mbuf *, struct m_tag *);

__END_DECLS
#endif /* XNU_KERNEL_PRIVATE */
#ifdef KERNEL
#include <sys/kpi_mbuf.h>
#ifdef XNU_KERNEL_PRIVATE
__BEGIN_DECLS

__private_extern__ void m_service_class_init(struct mbuf *);
__private_extern__ int m_set_service_class(struct mbuf *, mbuf_svc_class_t);
__private_extern__ mbuf_svc_class_t m_get_service_class(struct mbuf *);
__private_extern__ mbuf_svc_class_t m_service_class_from_idx(u_int32_t);
__private_extern__ mbuf_svc_class_t m_service_class_from_val(u_int32_t);
__private_extern__ int m_set_traffic_class(struct mbuf *, mbuf_traffic_class_t);
__private_extern__ mbuf_traffic_class_t m_get_traffic_class(struct mbuf *);

__END_DECLS
#endif /* XNU_KERNEL_PRIVATE */
#endif /* KERNEL */
#endif	/* !_SYS_MBUF_H_ */

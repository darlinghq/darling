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
 * Copyright (c) 1982, 1986, 1993
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
 *	@(#)ip_var.h	8.2 (Berkeley) 1/9/95
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2007 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

#ifndef _NETINET_IP_VAR_H_
#define	_NETINET_IP_VAR_H_
#include <sys/appleapiopts.h>

/*
 * Overlay for ip header used by other protocols (tcp, udp).
 */
struct ipovly {
	u_char	ih_x1[9];		/* (unused) */
	u_char	ih_pr;			/* protocol */
	u_short	ih_len;			/* protocol length */
	struct	in_addr ih_src;		/* source internet address */
	struct	in_addr ih_dst;		/* destination internet address */
};

#define	MAX_IPOPTLEN	40

struct	ipstat {
	u_int32_t ips_total;		/* total packets received */
	u_int32_t ips_badsum;		/* checksum bad */
	u_int32_t ips_tooshort;		/* packet too short */
	u_int32_t ips_toosmall;		/* not enough data */
	u_int32_t ips_badhlen;		/* ip header length < data size */
	u_int32_t ips_badlen;		/* ip length < ip header length */
	u_int32_t ips_fragments;	/* fragments received */
	u_int32_t ips_fragdropped;	/* frags dropped (dups, out of space) */
	u_int32_t ips_fragtimeout;	/* fragments timed out */
	u_int32_t ips_forward;		/* packets forwarded */
	u_int32_t ips_fastforward;	/* packets fast forwarded */
	u_int32_t ips_cantforward;	/* packets rcvd for unreachable dest */
	u_int32_t ips_redirectsent;	/* packets forwarded on same net */
	u_int32_t ips_noproto;		/* unknown or unsupported protocol */
	u_int32_t ips_delivered;	/* datagrams delivered to upper level */
	u_int32_t ips_localout;		/* total ip packets generated here */
	u_int32_t ips_odropped;		/* lost packets due to nobufs, etc. */
	u_int32_t ips_reassembled;	/* total packets reassembled ok */
	u_int32_t ips_fragmented;	/* datagrams successfully fragmented */
	u_int32_t ips_ofragments;	/* output fragments created */
	u_int32_t ips_cantfrag;		/* don't fragment flag was set, etc. */
	u_int32_t ips_badoptions;	/* error in option processing */
	u_int32_t ips_noroute;		/* packets discarded due to no route */
	u_int32_t ips_badvers;		/* ip version != 4 */
	u_int32_t ips_rawout;		/* total raw ip packets generated */
	u_int32_t ips_toolong;		/* ip length > max ip packet size */
	u_int32_t ips_notmember;	/* multicasts for unregistered grps */
	u_int32_t ips_nogif;		/* no match gif found */
	u_int32_t ips_badaddr;		/* invalid address on header */
	u_int32_t ips_pktdropcntrl;	/* pkt dropped, no mbufs for ctl data */
	u_int32_t ips_rcv_swcsum;	/* ip hdr swcksum (inbound), packets */
	u_int32_t ips_rcv_swcsum_bytes;	/* ip hdr swcksum (inbound), bytes */
	u_int32_t ips_snd_swcsum;	/* ip hdr swcksum (outbound), packets */
	u_int32_t ips_snd_swcsum_bytes;	/* ip hdr swcksum (outbound), bytes */
	u_int32_t ips_adj;		/* total packets trimmed/adjusted */
	u_int32_t ips_adj_hwcsum_clr;	/* hwcksum discarded during adj */
};

struct ip_linklocal_stat {
	u_int32_t	iplls_in_total;
	u_int32_t	iplls_in_badttl;
	u_int32_t	iplls_out_total;
	u_int32_t	iplls_out_badttl;
};


#endif /* !_NETINET_IP_VAR_H_ */

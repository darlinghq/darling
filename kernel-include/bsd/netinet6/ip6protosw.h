/*
 * Copyright (c) 2008-2019 Apple Inc. All rights reserved.
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

/*	$KAME: ip6protosw.h,v 1.22 2001/02/08 18:02:08 itojun Exp $	*/

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
 *
 */

/*	BSDI protosw.h,v 2.3 1996/10/11 16:02:40 pjd Exp	*/

/*-
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
 *	@(#)protosw.h	8.1 (Berkeley) 6/2/93
 */

#ifndef _NETINET6_IP6PROTOSW_H_
#define _NETINET6_IP6PROTOSW_H_
#include <sys/appleapiopts.h>

#ifdef BSD_KERNEL_PRIVATE
#include <kern/locks.h>

/*
 * Protocol switch table for IPv6.
 * All other definitions should refer to sys/protosw.h
 */

struct mbuf;
struct sockaddr;
struct socket;
struct domain;
struct ip6_hdr;
struct icmp6_hdr;
struct in6_addr;
struct pr_usrreqs;

/*
 * argument type for the last arg of pr_ctlinput().
 * should be consulted only with AF_INET6 family.
 *
 * IPv6 ICMP IPv6 [exthdrs] finalhdr payload
 * ^    ^    ^              ^
 * |    |    ip6c_ip6       ip6c_off
 * |    ip6c_icmp6
 * ip6c_m
 *
 * ip6c_finaldst usually points to ip6c_ip6->ip6_dst.  if the original
 * (internal) packet carries a routing header, it may point the final
 * dstination address in the routing header.
 *
 * ip6c_src: ip6c_ip6->ip6_src + scope info + flowlabel in ip6c_ip6
 *	(beware of flowlabel, if you try to compare it against others)
 * ip6c_dst: ip6c_finaldst + scope info
 */
struct ip6ctlparam {
	struct mbuf *ip6c_m;            /* start of mbuf chain */
	struct icmp6_hdr *ip6c_icmp6;   /* icmp6 header of target packet */
	struct ip6_hdr *ip6c_ip6;       /* ip6 header of target packet */
	int ip6c_off;                   /* offset of the target proto header */
	struct sockaddr_in6 *ip6c_src;  /* srcaddr w/ additional info */
	struct sockaddr_in6 *ip6c_dst;  /* (final) dstaddr w/ additional info */
	struct in6_addr *ip6c_finaldst; /* final destination address */
	void *ip6c_cmdarg;              /* control command dependent data */
	u_int8_t ip6c_nxt;              /* final next header field */
};

/*
 * TODO:
 *
 * This closesly resembles protosw; we should merge them one day.  Offsets
 * must be exactly equivalent, due to casting.
 */
struct ip6protosw {
	TAILQ_ENTRY(ip6protosw) pr_entry; /* chain for domain */
	struct  domain *pr_domain;      /* domain protocol a member of */
	struct protosw *pr_protosw;     /* pointer to self */
	u_int16_t pr_type;              /* socket type used for */
	u_int16_t pr_protocol;          /* protocol number */
	u_int32_t pr_flags;             /* see below */
	/*
	 * protocol-protocol hooks
	 */
	int     (*pr_input)             /* input to protocol (from below) */
	(struct mbuf **, int *, int);
	int     (*pr_output)            /* output to protocol (from above) */
	(struct mbuf *m, struct socket *so,
	struct sockaddr_in6 *, struct mbuf *);
	void    (*pr_ctlinput)          /* control input (from below) */
	(int, struct sockaddr *, void *, struct ifnet *);
	int     (*pr_ctloutput)         /* control output (from above) */
	(struct socket *, struct sockopt *);
	/*
	 * user-protocol hook
	 */
	struct  pr_usrreqs *pr_usrreqs; /* user request; see list below */
	/*
	 * utility hooks
	 */
	void    (*pr_init)              /* initialization hook */
	(struct ip6protosw *, struct domain *);
	void    (*pr_drain)(void);      /* flush any excess space possible */
	/* for compat. with IPv4 protosw */
	int     (*pr_sysctl)(void);     /* sysctl for protocol */
	int     (*pr_lock)              /* lock function for protocol */
	(struct socket *so, int refcnt, void *debug);
	int     (*pr_unlock)            /* unlock for protocol */
	(struct socket *so, int refcnt, void *debug);
	lck_mtx_t *(*pr_getlock)        /* retrieve protocol lock */
	(struct socket *so, int flags);
	/*
	 * misc
	 */
	TAILQ_HEAD(, socket_filter) pr_filter_head;
	struct protosw_old *pr_old;

	void    (*pr_update_last_owner) /* update last socket owner) */
	(struct socket *so, struct proc *p, struct proc *ep);

	void    (*pr_copy_last_owner) /* copy last socket from listener */
	(struct socket *so, struct socket *head);
};
#endif /* BSD_KERNEL_PRIVATE */
#endif /* _NETINET6_IP6PROTOSW_H_ */

/*
 * Copyright (c) 2008-2018 Apple Inc. All rights reserved.
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
 *	@(#)in.h	8.3 (Berkeley) 1/3/94
 */

#ifndef __KAME_NETINET_IN_H_INCLUDED_
#error "do not include netinet6/in6.h directly, include netinet/in.h. " \
        " see RFC2553"
#endif

#ifndef _NETINET6_IN6_H_
#define _NETINET6_IN6_H_
#include <sys/appleapiopts.h>
#include <sys/_types.h>
#include <sys/_types/_sa_family_t.h>

/*
 * Identification of the network protocol stack
 * for *BSD-current/release: http://www.kame.net/dev/cvsweb.cgi/kame/COVERAGE
 * has the table of implementation/integration differences.
 */
#define __KAME__
#define __KAME_VERSION          "2009/apple-darwin"

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
/*
 * Local port number conventions:
 *
 * Ports < IPPORT_RESERVED are reserved for privileged processes (e.g. root),
 * unless a kernel is compiled with IPNOPRIVPORTS defined.
 *
 * When a user does a bind(2) or connect(2) with a port number of zero,
 * a non-conflicting local port address is chosen.
 *
 * The default range is IPPORT_ANONMIN to IPPORT_ANONMAX, although
 * that is settable by sysctl(3); net.inet.ip.anonportmin and
 * net.inet.ip.anonportmax respectively.
 *
 * A user may set the IPPROTO_IP option IP_PORTRANGE to change this
 * default assignment range.
 *
 * The value IP_PORTRANGE_DEFAULT causes the default behavior.
 *
 * The value IP_PORTRANGE_HIGH is the same as IP_PORTRANGE_DEFAULT,
 * and exists only for FreeBSD compatibility purposes.
 *
 * The value IP_PORTRANGE_LOW changes the range to the "low" are
 * that is (by convention) restricted to privileged processes.
 * This convention is based on "vouchsafe" principles only.
 * It is only secure if you trust the remote host to restrict these ports.
 * The range is IPPORT_RESERVEDMIN to IPPORT_RESERVEDMAX.
 */

#define IPV6PORT_RESERVED       1024
#define IPV6PORT_ANONMIN        49152
#define IPV6PORT_ANONMAX        65535
#define IPV6PORT_RESERVEDMIN    600
#define IPV6PORT_RESERVEDMAX    (IPV6PORT_RESERVED-1)
#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */

/*
 * IPv6 address
 */
typedef struct in6_addr {
	union {
		__uint8_t   __u6_addr8[16];
		__uint16_t  __u6_addr16[8];
		__uint32_t  __u6_addr32[4];
	} __u6_addr;                    /* 128-bit IP6 address */
} in6_addr_t;

#define s6_addr   __u6_addr.__u6_addr8
#ifdef KERNEL   /* XXX nonstandard */
#define s6_addr8  __u6_addr.__u6_addr8
#define s6_addr16 __u6_addr.__u6_addr16
#define s6_addr32 __u6_addr.__u6_addr32
#endif

#define INET6_ADDRSTRLEN        46

/*
 * Socket address for IPv6
 */
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
#define SIN6_LEN
#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */
struct sockaddr_in6 {
	__uint8_t       sin6_len;       /* length of this struct(sa_family_t) */
	sa_family_t     sin6_family;    /* AF_INET6 (sa_family_t) */
	in_port_t       sin6_port;      /* Transport layer port # (in_port_t) */
	__uint32_t      sin6_flowinfo;  /* IP6 flow information */
	struct in6_addr sin6_addr;      /* IP6 address */
	__uint32_t      sin6_scope_id;  /* scope zone index */
};

#ifdef KERNEL   /* XXX nonstandard */
/*
 * Local definition for masks
 */
#define IN6MASK0        {{{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }}}
#define IN6MASK7        {{{ 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}}
#define IN6MASK16       {{{ 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}}
#define IN6MASK32       {{{ 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, \
	                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}}
#define IN6MASK64       {{{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
	                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}}
#define IN6MASK96       {{{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
	                    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 }}}
#define IN6MASK128      {{{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
	                    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }}}
#endif

#ifdef KERNEL_PRIVATE
extern const struct sockaddr_in6 sa6_any;

extern const struct in6_addr in6mask0;
extern const struct in6_addr in6mask7;
extern const struct in6_addr in6mask16;
extern const struct in6_addr in6mask32;
extern const struct in6_addr in6mask64;
extern const struct in6_addr in6mask96;
extern const struct in6_addr in6mask128;

#define SIN6(s)         ((struct sockaddr_in6 *)(void *)s)
#define satosin6(sa)    SIN6(sa)
#define sin6tosa(sin6)  ((struct sockaddr *)(void *)(sin6))
#define SIN6IFSCOPE(s)  SIN6(s)
#endif /* KERNEL_PRIVATE */

#ifdef KERNEL   /* XXX nonstandard */
/*
 * Macros started with IPV6_ADDR is KAME local
 */
#if BYTE_ORDER == BIG_ENDIAN
#define IPV6_ADDR_INT32_ONE     1
#define IPV6_ADDR_INT32_TWO     2
#define IPV6_ADDR_INT32_MNL     0xff010000
#define IPV6_ADDR_INT32_MLL     0xff020000
#define IPV6_ADDR_INT32_SMP     0x0000ffff
#define IPV6_ADDR_INT16_ULL     0xfe80
#define IPV6_ADDR_INT16_USL     0xfec0
#define IPV6_ADDR_INT16_MLL     0xff02
#elif BYTE_ORDER == LITTLE_ENDIAN
#define IPV6_ADDR_INT32_ONE     0x01000000
#define IPV6_ADDR_INT32_TWO     0x02000000
#define IPV6_ADDR_INT32_MNL     0x000001ff
#define IPV6_ADDR_INT32_MLL     0x000002ff
#define IPV6_ADDR_INT32_SMP     0xffff0000
#define IPV6_ADDR_INT16_ULL     0x80fe
#define IPV6_ADDR_INT16_USL     0xc0fe
#define IPV6_ADDR_INT16_MLL     0x02ff
#endif
#endif

/*
 * Definition of some useful macros to handle IP6 addresses
 */
#define IN6ADDR_ANY_INIT \
	{{{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}}
#define IN6ADDR_LOOPBACK_INIT \
	{{{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
#define IN6ADDR_NODELOCAL_ALLNODES_INIT \
	{{{ 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
#define IN6ADDR_INTFACELOCAL_ALLNODES_INIT \
	{{{ 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
#define IN6ADDR_LINKLOCAL_ALLNODES_INIT \
	{{{ 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
#define IN6ADDR_LINKLOCAL_ALLROUTERS_INIT \
	{{{ 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 }}}
#define IN6ADDR_LINKLOCAL_ALLV2ROUTERS_INIT \
	{{{ 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16 }}}
#define IN6ADDR_V4MAPPED_INIT \
	{{{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	    0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 }}}
#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */

extern const struct in6_addr in6addr_any;
extern const struct in6_addr in6addr_loopback;
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
extern const struct in6_addr in6addr_nodelocal_allnodes;
extern const struct in6_addr in6addr_linklocal_allnodes;
extern const struct in6_addr in6addr_linklocal_allrouters;
extern const struct in6_addr in6addr_linklocal_allv2routers;
#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */

/*
 * Equality
 * NOTE: Some of kernel programming environment (for example, openbsd/sparc)
 * does not supply memcmp().  For userland memcmp() is preferred as it is
 * in ANSI standard.
 */
#ifdef KERNEL
#define IN6_ARE_ADDR_EQUAL(a, b) \
	(bcmp(&(a)->s6_addr[0], &(b)->s6_addr[0], \
	sizeof (struct in6_addr)) == 0)
#else
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
#define IN6_ARE_ADDR_EQUAL(a, b) \
	(memcmp(&(a)->s6_addr[0], &(b)->s6_addr[0], sizeof (struct in6_addr)) \
	== 0)
#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */
#endif

#ifdef KERNEL                   /* non standard */
/* see if two addresses are equal in a scope-conscious manner. */
#define SA6_ARE_ADDR_EQUAL(a, b) \
	(((a)->sin6_scope_id == 0 || (b)->sin6_scope_id == 0 || \
	((a)->sin6_scope_id == (b)->sin6_scope_id)) && \
	(bcmp(&(a)->sin6_addr, &(b)->sin6_addr, sizeof (struct in6_addr)) == 0))
#endif

/*
 * Unspecified
 */
#define IN6_IS_ADDR_UNSPECIFIED(a)      \
	((*(const __uint32_t *)(const void *)(&(a)->s6_addr[0]) == 0) && \
	(*(const __uint32_t *)(const void *)(&(a)->s6_addr[4]) == 0) && \
	(*(const __uint32_t *)(const void *)(&(a)->s6_addr[8]) == 0) && \
	(*(const __uint32_t *)(const void *)(&(a)->s6_addr[12]) == 0))

/*
 * Loopback
 */
#define IN6_IS_ADDR_LOOPBACK(a)         \
	((*(const __uint32_t *)(const void *)(&(a)->s6_addr[0]) == 0) && \
	(*(const __uint32_t *)(const void *)(&(a)->s6_addr[4]) == 0) && \
	(*(const __uint32_t *)(const void *)(&(a)->s6_addr[8]) == 0) && \
	(*(const __uint32_t *)(const void *)(&(a)->s6_addr[12]) == ntohl(1)))

/*
 * IPv4 compatible
 */
#define IN6_IS_ADDR_V4COMPAT(a)         \
	((*(const __uint32_t *)(const void *)(&(a)->s6_addr[0]) == 0) && \
	(*(const __uint32_t *)(const void *)(&(a)->s6_addr[4]) == 0) && \
	(*(const __uint32_t *)(const void *)(&(a)->s6_addr[8]) == 0) && \
	(*(const __uint32_t *)(const void *)(&(a)->s6_addr[12]) != 0) && \
	(*(const __uint32_t *)(const void *)(&(a)->s6_addr[12]) != ntohl(1)))

/*
 * Mapped
 */
#define IN6_IS_ADDR_V4MAPPED(a)               \
	((*(const __uint32_t *)(const void *)(&(a)->s6_addr[0]) == 0) && \
	(*(const __uint32_t *)(const void *)(&(a)->s6_addr[4]) == 0) && \
	(*(const __uint32_t *)(const void *)(&(a)->s6_addr[8]) == \
	ntohl(0x0000ffff)))

/*
 * 6to4
 */
#define IN6_IS_ADDR_6TO4(x)     (ntohs((x)->s6_addr16[0]) == 0x2002)

/*
 * KAME Scope Values
 */

#ifdef KERNEL   /* XXX nonstandard */
#define IPV6_ADDR_SCOPE_NODELOCAL       0x01
#define IPV6_ADDR_SCOPE_INTFACELOCAL    0x01
#define IPV6_ADDR_SCOPE_LINKLOCAL       0x02
#define IPV6_ADDR_SCOPE_SITELOCAL       0x05
#define IPV6_ADDR_SCOPE_ORGLOCAL        0x08    /* just used in this file */
#define IPV6_ADDR_SCOPE_GLOBAL          0x0e
#else
#define __IPV6_ADDR_SCOPE_NODELOCAL     0x01
#define __IPV6_ADDR_SCOPE_INTFACELOCAL  0x01
#define __IPV6_ADDR_SCOPE_LINKLOCAL     0x02
#define __IPV6_ADDR_SCOPE_SITELOCAL     0x05
#define __IPV6_ADDR_SCOPE_ORGLOCAL      0x08    /* just used in this file */
#define __IPV6_ADDR_SCOPE_GLOBAL        0x0e
#endif

/*
 * Unicast Scope
 * Note that we must check topmost 10 bits only, not 16 bits (see RFC2373).
 */
#define IN6_IS_ADDR_LINKLOCAL(a)        \
	(((a)->s6_addr[0] == 0xfe) && (((a)->s6_addr[1] & 0xc0) == 0x80))
#define IN6_IS_ADDR_SITELOCAL(a)        \
	(((a)->s6_addr[0] == 0xfe) && (((a)->s6_addr[1] & 0xc0) == 0xc0))

/*
 * Multicast
 */
#define IN6_IS_ADDR_MULTICAST(a)        ((a)->s6_addr[0] == 0xff)

/*
 * Unique Local IPv6 Unicast Addresses (per RFC 4193)
 */
#define IN6_IS_ADDR_UNIQUE_LOCAL(a) \
	(((a)->s6_addr[0] == 0xfc) || ((a)->s6_addr[0] == 0xfd))

#ifdef KERNEL   /* XXX nonstandard */
#define IPV6_ADDR_MC_SCOPE(a)           ((a)->s6_addr[1] & 0x0f)
#else
#define __IPV6_ADDR_MC_SCOPE(a)         ((a)->s6_addr[1] & 0x0f)
#endif

/*
 * Multicast Scope
 */
#ifdef KERNEL   /* refers nonstandard items */
#define IN6_IS_ADDR_MC_NODELOCAL(a)     \
	(IN6_IS_ADDR_MULTICAST(a) &&    \
	(IPV6_ADDR_MC_SCOPE(a) == IPV6_ADDR_SCOPE_NODELOCAL))
#define IN6_IS_ADDR_MC_INTFACELOCAL(a)  \
	(IN6_IS_ADDR_MULTICAST(a) &&    \
	(IPV6_ADDR_MC_SCOPE(a) == IPV6_ADDR_SCOPE_INTFACELOCAL))
#define IN6_IS_ADDR_MC_LINKLOCAL(a)     \
	(IN6_IS_ADDR_MULTICAST(a) &&    \
	(IPV6_ADDR_MC_SCOPE(a) == IPV6_ADDR_SCOPE_LINKLOCAL))
#define IN6_IS_ADDR_MC_SITELOCAL(a)     \
	(IN6_IS_ADDR_MULTICAST(a) &&    \
	(IPV6_ADDR_MC_SCOPE(a) == IPV6_ADDR_SCOPE_SITELOCAL))
#define IN6_IS_ADDR_MC_ORGLOCAL(a)      \
	(IN6_IS_ADDR_MULTICAST(a) &&    \
	(IPV6_ADDR_MC_SCOPE(a) == IPV6_ADDR_SCOPE_ORGLOCAL))
#define IN6_IS_ADDR_MC_GLOBAL(a)        \
	(IN6_IS_ADDR_MULTICAST(a) &&    \
	(IPV6_ADDR_MC_SCOPE(a) == IPV6_ADDR_SCOPE_GLOBAL))
#else
#define IN6_IS_ADDR_MC_NODELOCAL(a)     \
	(IN6_IS_ADDR_MULTICAST(a) &&    \
	(__IPV6_ADDR_MC_SCOPE(a) == __IPV6_ADDR_SCOPE_NODELOCAL))
#define IN6_IS_ADDR_MC_LINKLOCAL(a)     \
	(IN6_IS_ADDR_MULTICAST(a) &&    \
	(__IPV6_ADDR_MC_SCOPE(a) == __IPV6_ADDR_SCOPE_LINKLOCAL))
#define IN6_IS_ADDR_MC_SITELOCAL(a)     \
	(IN6_IS_ADDR_MULTICAST(a) &&    \
	(__IPV6_ADDR_MC_SCOPE(a) == __IPV6_ADDR_SCOPE_SITELOCAL))
#define IN6_IS_ADDR_MC_ORGLOCAL(a)      \
	(IN6_IS_ADDR_MULTICAST(a) &&    \
	(__IPV6_ADDR_MC_SCOPE(a) == __IPV6_ADDR_SCOPE_ORGLOCAL))
#define IN6_IS_ADDR_MC_GLOBAL(a)        \
	(IN6_IS_ADDR_MULTICAST(a) &&    \
	(__IPV6_ADDR_MC_SCOPE(a) == __IPV6_ADDR_SCOPE_GLOBAL))
#endif

#ifdef KERNEL   /* nonstandard */
/*
 * KAME Scope
 */
#define IN6_IS_SCOPE_LINKLOCAL(a)       \
	((IN6_IS_ADDR_LINKLOCAL(a)) ||  \
	(IN6_IS_ADDR_MC_LINKLOCAL(a)))

#define IN6_IS_SCOPE_EMBED(a)                   \
	((IN6_IS_ADDR_LINKLOCAL(a)) ||          \
	(IN6_IS_ADDR_MC_LINKLOCAL(a)) ||        \
	(IN6_IS_ADDR_MC_INTFACELOCAL(a)))

#define IFA6_IS_DEPRECATED(a, t) \
	((a)->ia6_lifetime.ia6ti_preferred != 0 && \
	(a)->ia6_lifetime.ia6ti_preferred < (t))
#define IFA6_IS_INVALID(a, t) \
	((a)->ia6_lifetime.ia6ti_expire != 0 && \
	(a)->ia6_lifetime.ia6ti_expire < (t))
#endif /* KERNEL */

#ifdef PRIVATE
struct route_in6_old {
	void            *ro_rt;
	uint32_t        ro_flags;
	struct sockaddr_in6 ro_dst;
};
#endif /* PRIVATE */

#ifdef BSD_KERNEL_PRIVATE
#include <net/if_llatbl.h>
#include <sys/eventhandler.h>

/*
 * IP6 route structure
 *
 * A route consists of a destination address and a reference
 * to a routing entry.  These are often held by protocols
 * in their control blocks, e.g. inpcb.
 */
struct route_in6 {
	/*
	 * N.B: struct route_in6 must begin with ro_{rt,srcia,flags}
	 * because the code does some casts of a 'struct route_in6 *'
	 * to a 'struct route *'.
	 */
	struct rtentry  *ro_rt;
	struct  llentry *ro_lle;

	struct ifaddr   *ro_srcia;
	uint32_t        ro_flags;       /* route flags */
	struct sockaddr_in6 ro_dst;
};
#endif /* BSD_KERNEL_PRIVATE */

/*
 * Options for use with [gs]etsockopt at the IPV6 level.
 * First word of comment is data type; bool is stored in int.
 */
/* no hdrincl */
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
/*
 * RFC 3542 define the following socket options in a manner incompatible
 * with RFC 2292:
 *   IPV6_PKTINFO
 *   IPV6_HOPLIMIT
 *   IPV6_NEXTHOP
 *   IPV6_HOPOPTS
 *   IPV6_DSTOPTS
 *   IPV6_RTHDR
 *
 * To use the new IPv6 Sockets options introduced by RFC 3542
 * the constant __APPLE_USE_RFC_3542 must be defined before
 * including <netinet/in.h>
 *
 * To use the old IPv6 Sockets options from RFC 2292
 * the constant __APPLE_USE_RFC_2292 must be defined before
 * including <netinet/in.h>
 *
 * Note that eventually RFC 3542 is going to be the
 * default and RFC 2292 will be obsolete.
 */
#ifdef BSD_KERNEL_PRIVATE
#define __APPLE_USE_RFC_3542 1
#endif /* BSD_KERNEL_PRIVATE */

#if defined(__APPLE_USE_RFC_3542) && defined(__APPLE_USE_RFC_2292)
#error "__APPLE_USE_RFC_3542 and __APPLE_USE_RFC_2292 cannot be both defined"
#endif

#if 0 /* the followings are relic in IPv4 and hence are disabled */
#define IPV6_OPTIONS            1  /* buf/ip6_opts; set/get IP6 options */
#define IPV6_RECVOPTS           5  /* bool; receive all IP6 opts w/dgram */
#define IPV6_RECVRETOPTS        6  /* bool; receive IP6 opts for response */
#define IPV6_RECVDSTADDR        7  /* bool; receive IP6 dst addr w/dgram */
#define IPV6_RETOPTS            8  /* ip6_opts; set/get IP6 options */
#endif /* 0 */
#define IPV6_SOCKOPT_RESERVED1  3  /* reserved for future use */
#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */
#define IPV6_UNICAST_HOPS       4  /* int; IP6 hops */
#define IPV6_MULTICAST_IF       9  /* __uint8_t; set/get IP6 multicast i/f  */
#define IPV6_MULTICAST_HOPS     10 /* __uint8_t; set/get IP6 multicast hops */
#define IPV6_MULTICAST_LOOP     11 /* __uint8_t; set/get IP6 mcast loopback */
#define IPV6_JOIN_GROUP         12 /* ip6_mreq; join a group membership */
#define IPV6_LEAVE_GROUP        13 /* ip6_mreq; leave a group membership */

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
#define IPV6_PORTRANGE          14 /* int; range to choose for unspec port */
#define ICMP6_FILTER            18 /* icmp6_filter; icmp6 filter */
#define IPV6_2292PKTINFO        19 /* bool; send/recv if, src/dst addr */
#define IPV6_2292HOPLIMIT       20 /* bool; hop limit */
#define IPV6_2292NEXTHOP        21 /* bool; next hop addr */
#define IPV6_2292HOPOPTS        22 /* bool; hop-by-hop option */
#define IPV6_2292DSTOPTS        23 /* bool; destinaion option */
#define IPV6_2292RTHDR          24 /* ip6_rthdr: routing header */

/* buf/cmsghdr; set/get IPv6 options [obsoleted by RFC3542] */
#define IPV6_2292PKTOPTIONS     25

#ifdef __APPLE_USE_RFC_2292
#define IPV6_PKTINFO    IPV6_2292PKTINFO
#define IPV6_HOPLIMIT   IPV6_2292HOPLIMIT
#define IPV6_NEXTHOP    IPV6_2292NEXTHOP
#define IPV6_HOPOPTS    IPV6_2292HOPOPTS
#define IPV6_DSTOPTS    IPV6_2292DSTOPTS
#define IPV6_RTHDR      IPV6_2292RTHDR
#define IPV6_PKTOPTIONS IPV6_2292PKTOPTIONS
#endif /* __APPLE_USE_RFC_2292 */

#define IPV6_CHECKSUM           26 /* int; checksum offset for raw socket */
#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */
#define IPV6_V6ONLY             27 /* bool; only bind INET6 at wildcard bind */
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
#ifndef KERNEL
#define IPV6_BINDV6ONLY         IPV6_V6ONLY
#endif /* KERNEL */


#if 1 /* IPSEC */
#define IPV6_IPSEC_POLICY       28 /* struct; get/set security policy */
#endif /* 1 */
#define IPV6_FAITH              29 /* deprecated */

#if 1 /* IPV6FIREWALL */
#define IPV6_FW_ADD             30 /* add a firewall rule to chain */
#define IPV6_FW_DEL             31 /* delete a firewall rule from chain */
#define IPV6_FW_FLUSH           32 /* flush firewall rule chain */
#define IPV6_FW_ZERO            33 /* clear single/all firewall counter(s) */
#define IPV6_FW_GET             34 /* get entire firewall rule chain */
#endif /* 1 */

/*
 * APPLE: NOTE the value of those 2 options is kept unchanged from
 *   previous version of darwin/OS X for binary compatibility reasons
 *   and differ from FreeBSD (values 57 and 61). See below.
 */
#define IPV6_RECVTCLASS         35 /* bool; recv traffic class values */
#define IPV6_TCLASS             36 /* int; send traffic class value */

#ifdef __APPLE_USE_RFC_3542
/* new socket options introduced in RFC3542 */
/*
 * ip6_dest; send dst option before rthdr
 * APPLE: Value purposely different than FreeBSD (35) to avoid
 * collision with definition of IPV6_RECVTCLASS in previous
 * darwin implementations
 */
#define IPV6_RTHDRDSTOPTS       57

/*
 * bool; recv if, dst addr
 * APPLE: Value purposely different than FreeBSD(36) to avoid
 * collision with definition of IPV6_TCLASS in previous
 * darwin implementations
 */
#define IPV6_RECVPKTINFO        61

#define IPV6_RECVHOPLIMIT       37 /* bool; recv hop limit */
#define IPV6_RECVRTHDR          38 /* bool; recv routing header */
#define IPV6_RECVHOPOPTS        39 /* bool; recv hop-by-hop option */
#define IPV6_RECVDSTOPTS        40 /* bool; recv dst option after rthdr */
#ifdef KERNEL
#define IPV6_RECVRTHDRDSTOPTS   41 /* bool; recv dst option before rthdr */
#endif

#define IPV6_USE_MIN_MTU        42 /* bool; send packets at the minimum MTU */
#define IPV6_RECVPATHMTU        43 /* bool; notify an according MTU */

/*
 * mtuinfo; get the current path MTU (sopt), 4 bytes int;
 * MTU notification (cmsg)
 */
#define IPV6_PATHMTU            44

#if 0 /* obsoleted during 2292bis -> 3542 */
/* no data; ND reachability confirm (cmsg only/not in of RFC3542) */
#define IPV6_REACHCONF          45
#endif
/* more new socket options introduced in RFC3542 */
#define IPV6_3542PKTINFO        46 /* in6_pktinfo; send if, src addr */
#define IPV6_3542HOPLIMIT       47 /* int; send hop limit */
#define IPV6_3542NEXTHOP        48 /* sockaddr; next hop addr */
#define IPV6_3542HOPOPTS        49 /* ip6_hbh; send hop-by-hop option */
#define IPV6_3542DSTOPTS        50 /* ip6_dest; send dst option befor rthdr */
#define IPV6_3542RTHDR          51 /* ip6_rthdr; send routing header */

#define IPV6_PKTINFO    IPV6_3542PKTINFO
#define IPV6_HOPLIMIT   IPV6_3542HOPLIMIT
#define IPV6_NEXTHOP    IPV6_3542NEXTHOP
#define IPV6_HOPOPTS    IPV6_3542HOPOPTS
#define IPV6_DSTOPTS    IPV6_3542DSTOPTS
#define IPV6_RTHDR      IPV6_3542RTHDR

#define IPV6_AUTOFLOWLABEL      59 /* bool; attach flowlabel automagically */

#define IPV6_DONTFRAG           62 /* bool; disable IPv6 fragmentation */

/* int; prefer temporary addresses as the source address. */
#define IPV6_PREFER_TEMPADDR    63

/*
 * The following option is private; do not use it from user applications.
 * It is deliberately defined to the same value as IP_MSFILTER.
 */
#define IPV6_MSFILTER           74 /* struct __msfilterreq; */
#endif /* __APPLE_USE_RFC_3542 */

#define IPV6_BOUND_IF           125 /* int; set/get bound interface */

#ifdef PRIVATE
#define IPV6_NO_IFT_CELLULAR    6969 /* for internal use only */
#define IPV6_OUT_IF             9696 /* for internal use only */
#endif /* PRIVATE */

/* to define items, should talk with KAME guys first, for *BSD compatibility */

#define IPV6_RTHDR_LOOSE        0 /* this hop need not be a neighbor. */
#define IPV6_RTHDR_STRICT       1 /* this hop must be a neighbor. */
#define IPV6_RTHDR_TYPE_0       0 /* IPv6 routing header type 0 */

/*
 * Defaults and limits for options
 */
#define IPV6_DEFAULT_MULTICAST_HOPS 1   /* normally limit m'casts to 1 hop  */
#define IPV6_DEFAULT_MULTICAST_LOOP 1   /* normally hear sends if a member  */

/*
 * The im6o_membership vector for each socket is now dynamically allocated at
 * run-time, bounded by USHRT_MAX, and is reallocated when needed, sized
 * according to a power-of-two increment.
 */
#define IPV6_MIN_MEMBERSHIPS    31
#define IPV6_MAX_MEMBERSHIPS    4095

/*
 * Default resource limits for IPv6 multicast source filtering.
 * These may be modified by sysctl.
 */
#define IPV6_MAX_GROUP_SRC_FILTER       512     /* sources per group */
#define IPV6_MAX_SOCK_SRC_FILTER        128     /* sources per socket/group */

/*
 * Argument structure for IPV6_JOIN_GROUP and IPV6_LEAVE_GROUP.
 */
struct ipv6_mreq {
	struct in6_addr ipv6mr_multiaddr;
	unsigned int    ipv6mr_interface;
};

/*
 * IPV6_2292PKTINFO: Packet information(RFC2292 sec 5)
 */
struct in6_pktinfo {
	struct in6_addr ipi6_addr;      /* src/dst IPv6 address */
	unsigned int    ipi6_ifindex;   /* send/recv interface index */
};

/*
 * Control structure for IPV6_RECVPATHMTU socket option.
 */
struct ip6_mtuinfo {
	struct sockaddr_in6 ip6m_addr;  /* or sockaddr_storage? */
	uint32_t ip6m_mtu;
};

/*
 * Argument for IPV6_PORTRANGE:
 * - which range to search when port is unspecified at bind() or connect()
 */
#define IPV6_PORTRANGE_DEFAULT  0       /* default range */
#define IPV6_PORTRANGE_HIGH     1       /* "high" - request firewall bypass */
#define IPV6_PORTRANGE_LOW      2       /* "low" - vouchsafe security */

/*
 * Definitions for inet6 sysctl operations.
 *
 * Third level is protocol number.
 * Fourth level is desired variable within that protocol.
 */
#define IPV6PROTO_MAXID (IPPROTO_PIM + 1)  /* don't list to IPV6PROTO_MAX */

/*
 * Names for IP sysctl objects
 */
#define IPV6CTL_FORWARDING      1       /* act as router */
#define IPV6CTL_SENDREDIRECTS   2       /* may send redirects when forwarding */
#define IPV6CTL_DEFHLIM         3       /* default Hop-Limit */
#ifdef notyet
#define IPV6CTL_DEFMTU          4       /* default MTU */
#endif
#define IPV6CTL_FORWSRCRT       5       /* forward source-routed dgrams */
#define IPV6CTL_STATS           6       /* stats */
#define IPV6CTL_MRTSTATS        7       /* multicast forwarding stats */
#define IPV6CTL_MRTPROTO        8       /* multicast routing protocol */
#define IPV6CTL_MAXFRAGPACKETS  9       /* max packets reassembly queue */
#define IPV6CTL_SOURCECHECK     10      /* verify source route and intf */
#define IPV6CTL_SOURCECHECK_LOGINT 11   /* minimume logging interval */
#define IPV6CTL_ACCEPT_RTADV    12
#define IPV6CTL_KEEPFAITH       13      /* deprecated */
#define IPV6CTL_LOG_INTERVAL    14
#define IPV6CTL_HDRNESTLIMIT    15
#define IPV6CTL_DAD_COUNT       16
#define IPV6CTL_AUTO_FLOWLABEL  17
#define IPV6CTL_DEFMCASTHLIM    18
#define IPV6CTL_GIF_HLIM        19      /* default HLIM for gif encap packet */
#define IPV6CTL_KAME_VERSION    20
#define IPV6CTL_USE_DEPRECATED  21      /* use deprec addr (RFC2462 5.5.4) */
#define IPV6CTL_RR_PRUNE        22      /* walk timer for router renumbering */
#if 0   /* obsolete */
#define IPV6CTL_MAPPED_ADDR     23
#endif
#define IPV6CTL_V6ONLY          24
#define IPV6CTL_RTEXPIRE        25      /* cloned route expiration time */
#define IPV6CTL_RTMINEXPIRE     26      /* min value for expiration time */
#define IPV6CTL_RTMAXCACHE      27      /* trigger level for dynamic expire */

#define IPV6CTL_USETEMPADDR     32      /* use temporary addresses [RFC 4941] */
#define IPV6CTL_TEMPPLTIME      33      /* preferred lifetime for tmpaddrs */
#define IPV6CTL_TEMPVLTIME      34      /* valid lifetime for tmpaddrs */
#define IPV6CTL_AUTO_LINKLOCAL  35      /* automatic link-local addr assign */
#define IPV6CTL_RIP6STATS       36      /* raw_ip6 stats */
#define IPV6CTL_PREFER_TEMPADDR 37      /* prefer temporary addr as src */
#define IPV6CTL_ADDRCTLPOLICY   38      /* get/set address selection policy */
#define IPV6CTL_USE_DEFAULTZONE 39      /* use default scope zone */

#define IPV6CTL_MAXFRAGS        41      /* max fragments */
#define IPV6CTL_MCAST_PMTU      44      /* enable pMTU discovery for mcast? */

#define IPV6CTL_NEIGHBORGCTHRESH 46
#define IPV6CTL_MAXIFPREFIXES   47
#define IPV6CTL_MAXIFDEFROUTERS 48
#define IPV6CTL_MAXDYNROUTES    49
#define ICMPV6CTL_ND6_ONLINKNSRFC4861   50

/* New entries should be added here from current IPV6CTL_MAXID value. */
/* to define items, should talk with KAME guys first, for *BSD compatibility */
#define IPV6CTL_MAXID           51

#ifdef BSD_KERNEL_PRIVATE
#define CTL_IPV6PROTO_NAMES { \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, \
	{ "tcp6", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "udp6", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, \
	{ "ip6", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, \
	{ "ipsec6", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "icmp6", CTLTYPE_NODE }, \
	{ 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
}
/*
 * Redefinition of mbuf flags
 */
#define M_AUTHIPHDR     M_PROTO2
#define M_DECRYPTED     M_PROTO3
#define M_AUTHIPDGM     M_PROTO5

struct cmsghdr;
struct mbuf;
struct ifnet;
struct in6_aliasreq;
struct lltable;

extern struct lltable * in6_lltattach(struct ifnet *ifp);
extern uint16_t in6_pseudo(const struct in6_addr *, const struct in6_addr *,
    uint32_t);
extern u_int16_t inet6_cksum(struct mbuf *, uint32_t, uint32_t, uint32_t);
extern u_int16_t inet6_cksum_buffer(const uint8_t *, uint32_t, uint32_t,
    uint32_t);

#define in6_cksum(_m, _n, _o, _l)                       \
	inet6_cksum(_m, _n, _o, _l)
#define in6_cksum_buffer(_b, _n, _o, _l)                \
	inet6_cksum_buffer(_b, _n, _o, _l)

extern int in6_addrscope(struct in6_addr *);
extern struct in6_ifaddr *in6_ifawithscope(struct ifnet *, struct in6_addr *);
extern struct in6_ifaddr *in6_ifawithifp(struct ifnet *, struct in6_addr *);

struct sockaddr;

extern void in6_sin6_2_sin(struct sockaddr_in *sin, struct sockaddr_in6 *sin6);
extern void in6_sin_2_v4mapsin6(struct sockaddr_in *sin,
    struct sockaddr_in6 *sin6);
extern void in6_sin6_2_sin_in_sock(struct sockaddr *nam);
extern int in6_sin_2_v4mapsin6_in_sock(struct sockaddr **nam);

extern uint32_t in6_finalize_cksum(struct mbuf *, uint32_t, int32_t,
    int32_t, uint32_t);

#define in6_delayed_cksum(_m)                   \
	((void) in6_finalize_cksum(_m, 0, 0, -1, CSUM_DELAY_IPV6_DATA))
#define in6_delayed_cksum_offset(_m, _o, _s, _p)        \
	((void) in6_finalize_cksum(_m, _o, _s, _p, CSUM_DELAY_IPV6_DATA))

/* IPv6 protocol events */
extern struct eventhandler_lists_ctxt in6_evhdlr_ctxt;
/*
 * XXX Avoid reordering the enum values below.
 * If the order is changed, please make sure
 * in6_event2kev_array is also changed to reflect the
 * change in order of the enums
 */
typedef enum {
	/* Address events */
	/*
	 * XXX To avoid duplicacy and also for correctness
	 * only report these for link local and stable addresses
	 * NOTE: Link local address can never be marked detached
	 * or duplicated.
	 */
	IN6_ADDR_MARKED_DUPLICATED,
	IN6_ADDR_MARKED_DETACHED,
	IN6_ADDR_MARKED_DEPRECATED,

	/* Expiry events */
	IN6_NDP_RTR_EXPIRY,
	IN6_NDP_PFX_EXPIRY,
	IN6_NDP_ADDR_EXPIRY,

	/* XXX DNS expiry needs to be handled by user-space */
	/* MAX */
	IN6_EVENT_MAX,
} in6_evhdlr_code_t;

struct in6_event2kev {
	in6_evhdlr_code_t       in6_event_code;
	uint32_t                in6_event_kev_subclass;
	uint32_t                in6_event_kev_code;
	const char              *in6_event_str;
};
extern struct in6_event2kev in6_event2kev_array[];
extern void in6_eventhdlr_callback(struct eventhandler_entry_arg, in6_evhdlr_code_t,
    struct ifnet *, struct in6_addr *, uint32_t);
extern void in6_event_enqueue_nwk_wq_entry(in6_evhdlr_code_t,
    struct ifnet *, struct in6_addr *, uint32_t);

typedef void (*in6_event_fn) (struct eventhandler_entry_arg, in6_evhdlr_code_t,
    struct ifnet *, struct in6_addr *, uint32_t);
EVENTHANDLER_DECLARE(in6_event, in6_event_fn);
#endif /* BSD_KERNEL_PRIVATE */

#ifdef PRIVATE
/* CLAT46 events */
typedef enum in6_clat46_evhdlr_code_t {
	IN6_CLAT46_EVENT_V4_FLOW,
	IN6_CLAT46_EVENT_V6_ADDR_CONFFAIL,
} in6_clat46_evhdlr_code_t;

struct kev_netevent_clat46_data {
	in6_clat46_evhdlr_code_t clat46_event_code;
	pid_t epid;
	uuid_t euuid;
};
#endif /* PRIVATE */

#ifdef BSD_KERNEL_PRIVATE
/* CLAT46 events */
extern struct eventhandler_lists_ctxt in6_clat46_evhdlr_ctxt;
extern void in6_clat46_eventhdlr_callback(struct eventhandler_entry_arg,
    in6_clat46_evhdlr_code_t, pid_t, uuid_t);
extern void in6_clat46_event_enqueue_nwk_wq_entry(in6_clat46_evhdlr_code_t,
    pid_t, uuid_t);

typedef void (*in6_clat46_event_fn) (struct eventhandler_entry_arg, in6_clat46_evhdlr_code_t,
    pid_t, uuid_t);
EVENTHANDLER_DECLARE(in6_clat46_event, in6_clat46_event_fn);
#endif /* BSD_KERNEL_PRIVATE */

#ifdef KERNEL_PRIVATE
/* exporte for ApplicationFirewall */
extern int in6_localaddr(struct in6_addr *);
extern int in6addr_local(struct in6_addr *);
#endif /* KERNEL_PRIVATE */

#ifndef KERNEL
__BEGIN_DECLS
struct cmsghdr;

extern int inet6_option_space(int);
extern int inet6_option_init(void *, struct cmsghdr **, int);
extern int inet6_option_append(struct cmsghdr *, const __uint8_t *, int, int);
extern __uint8_t *inet6_option_alloc(struct cmsghdr *, int, int, int);
extern int inet6_option_next(const struct cmsghdr *, __uint8_t **);
extern int inet6_option_find(const struct cmsghdr *, __uint8_t **, int);

extern size_t inet6_rthdr_space(int, int);
extern struct cmsghdr *inet6_rthdr_init(void *, int);
extern int inet6_rthdr_add(struct cmsghdr *, const struct in6_addr *,
    unsigned int);
extern int inet6_rthdr_lasthop(struct cmsghdr *, unsigned int);
#if 0 /* not implemented yet */
extern int inet6_rthdr_reverse(const struct cmsghdr *, struct cmsghdr *);
#endif
extern int inet6_rthdr_segments(const struct cmsghdr *);
extern struct in6_addr *inet6_rthdr_getaddr(struct cmsghdr *, int);
extern int inet6_rthdr_getflags(const struct cmsghdr *, int);

extern int inet6_opt_init(void *, socklen_t);
extern int inet6_opt_append(void *, socklen_t, int, __uint8_t, socklen_t,
    __uint8_t, void **);
extern int inet6_opt_finish(void *, socklen_t, int);
extern int inet6_opt_set_val(void *, int, void *, socklen_t);

extern int inet6_opt_next(void *, socklen_t, int, __uint8_t *, socklen_t *,
    void **);
extern int inet6_opt_find(void *, socklen_t, int, __uint8_t, socklen_t *,
    void **);
extern int inet6_opt_get_val(void *, int, void *, socklen_t);
extern socklen_t inet6_rth_space(int, int);
extern void *inet6_rth_init(void *, socklen_t, int, int);
extern int inet6_rth_add(void *, const struct in6_addr *);
extern int inet6_rth_reverse(const void *, void *);
extern int inet6_rth_segments(const void *);
extern struct in6_addr *inet6_rth_getaddr(const void *, int);

__END_DECLS
#endif /* !KERNEL */
#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */
#endif /* !_NETINET6_IN6_H_ */

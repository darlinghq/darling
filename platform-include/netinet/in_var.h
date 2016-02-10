/*
 * Copyright (c) 2000-2013 Apple Inc. All rights reserved.
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
 *	@(#)in_var.h	8.2 (Berkeley) 1/9/95
 */

#ifndef _NETINET_IN_VAR_H_
#define	_NETINET_IN_VAR_H_
#include <sys/appleapiopts.h>

#include <sys/queue.h>
#include <sys/kern_event.h>


struct in_aliasreq {
	char ifra_name[IFNAMSIZ];	/* if name, e.g. "en0" */
	struct sockaddr_in ifra_addr;
	struct sockaddr_in ifra_broadaddr;
#define	ifra_dstaddr	ifra_broadaddr
	struct sockaddr_in ifra_mask;
};

/*
 * Event data, inet style.
 */
struct kev_in_data {
	struct net_event_data link_data;
	struct in_addr ia_addr;		/* interface address */
	u_int32_t ia_net;		/* network number of interface */
	u_int32_t ia_netmask;		/* mask of net part */
	u_int32_t ia_subnet;		/* subnet number, including net */
	u_int32_t ia_subnetmask;	/* mask of subnet part */
	struct in_addr ia_netbroadcast;	/* to recognize net broadcasts */
	struct in_addr ia_dstaddr;
};

struct kev_in_collision {
	struct net_event_data link_data; /* link where ARP was received on */
	struct in_addr ia_ipaddr;	/* conflicting IP address */
	u_char hw_len;			/* length of hardware address */
	u_char hw_addr[0];		/* variable length hardware address */
};

struct kev_in_arpfailure {
	struct net_event_data link_data; /* link where ARP is being sent */
};

struct kev_in_arpalive {
	struct net_event_data link_data; /* link where ARP was received */
};


#ifdef __APPLE_API_PRIVATE
struct kev_in_portinuse {
	u_int16_t port;		/* conflicting port number in host order */
	u_int32_t req_pid;	/* PID port requestor */
	u_int32_t reserved[2];
};
#endif /* __APPLE_API_PRIVATE */

/*
 * Define inet event subclass and specific inet events.
 */
#define	KEV_INET_SUBCLASS		1 /* inet subclass identifier */

#define	KEV_INET_NEW_ADDR		1 /* Userland configured IP address */
#define	KEV_INET_CHANGED_ADDR		2 /* Address changed event */
#define	KEV_INET_ADDR_DELETED		3 /* IPv6 address was deleted */
#define	KEV_INET_SIFDSTADDR		4 /* Dest. address was set */
#define	KEV_INET_SIFBRDADDR		5 /* Broadcast address was set */
#define	KEV_INET_SIFNETMASK		6 /* Netmask was set */
#define	KEV_INET_ARPCOLLISION		7 /* ARP collision detected */
#ifdef __APPLE_API_PRIVATE
#define	KEV_INET_PORTINUSE		8 /* use ken_in_portinuse */
#endif
#define	KEV_INET_ARPRTRFAILURE		9 /* ARP resolution failed for router */
#define	KEV_INET_ARPRTRALIVE		10 /* ARP resolution succeeded for 
					      router */

/* INET6 stuff */
#include <netinet6/in6_var.h>
#endif /* _NETINET_IN_VAR_H_ */

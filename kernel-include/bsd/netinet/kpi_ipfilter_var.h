/*
 * Copyright (c) 2008 Apple Computer, Inc. All rights reserved.
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

#ifndef __KPI_IPFILTER_VAR__
#define __KPI_IPFILTER_VAR__

#include <sys/appleapiopts.h>
#include <netinet/kpi_ipfilter.h>

#ifdef KERNEL_PRIVATE

/* Private data structure, stripped out by ifdef tool */
/* Implementation specific bits */

#include <sys/queue.h>

struct ipfilter {
	TAILQ_ENTRY(ipfilter)   ipf_link;
	struct ipf_filter       ipf_filter;
	struct ipfilter_list    *ipf_head;
	TAILQ_ENTRY(ipfilter)   ipf_tbr;
};
TAILQ_HEAD(ipfilter_list, ipfilter);


extern struct ipfilter_list     ipv6_filters;
extern struct ipfilter_list     ipv4_filters;

extern ipfilter_t ipf_get_inject_filter(struct mbuf *m);
extern void ipf_ref(void);
extern void ipf_unref(void);
extern int ipf_init(void);
extern void ip_proto_dispatch_in(struct mbuf *m, int hlen, u_int8_t proto,
    ipfilter_t ipfref);

#endif /* KERNEL_PRIVATE */

#endif /*__KPI_IPFILTER_VAR__*/

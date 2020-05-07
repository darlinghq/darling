/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
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
 * limitations under the License
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef LRO_EXT_H_
#define LRO_EXT_H_

#ifdef BSD_KERNEL_PRIVATE

/* All definitions exported from LRO go into this file */

extern int sw_lro;
extern int lrodebug;
extern unsigned int coalesc_sz;

/* flow return values */
#define TCP_LRO_NAN             0x00    /* No flow exists */
#define TCP_LRO_CONSUMED        0x01    /* LRO consumed the packet */
#define TCP_LRO_EJECT_FLOW      0x02    /* LRO ejected the flow */
#define TCP_LRO_COALESCE        0x03    /* LRO to coalesce the packet */
#define TCP_LRO_COLLISION       0x04    /* Two flows map to the same slot */

void tcp_lro_init(void);

/* When doing LRO in IP call this function */
struct mbuf* tcp_lro(struct mbuf *m, unsigned int hlen);

/* TCP calls this to start coalescing a flow */
int tcp_start_coalescing(struct ip *, struct tcphdr *, int tlen);

/* TCP calls this to stop coalescing a flow */
int tcp_lro_remove_state(struct in_addr, struct in_addr, unsigned short,
    unsigned short);

/* TCP calls this to keep the seq number updated */
void tcp_update_lro_seq(__uint32_t, struct in_addr, struct in_addr,
    unsigned short, unsigned short);

#endif

#endif /* LRO_EXT_H_ */

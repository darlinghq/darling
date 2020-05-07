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
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef TCP_LRO_H_
#define TCP_LRO_H_

#ifdef BSD_KERNEL_PRIVATE

#define TCP_LRO_NUM_FLOWS (16)  /* must be <= 255 for char lro_flow_map */
#define TCP_LRO_FLOW_MAP  (1024)

struct lro_flow {
	struct mbuf             *lr_mhead;      /* coalesced mbuf chain head */
	struct mbuf             *lr_mtail;      /* coalesced mbuf chain tail */
	struct tcphdr           *lr_tcphdr;     /* ptr to TCP hdr in frame */
	u_int32_t               *lr_tsval;      /* address of tsval in frame */
	u_int32_t               *lr_tsecr;      /* tsecr field in TCP header */
	tcp_seq                 lr_seq;         /* next expected seq num */
	unsigned int            lr_len;         /* length of LRO frame */
	struct in_addr          lr_faddr;       /* foreign address */
	struct in_addr          lr_laddr;       /* local address */
	unsigned short int      lr_fport;       /* foreign port */
	unsigned short int      lr_lport;       /* local port */
	u_int32_t               lr_timestamp;   /* for ejecting the flow */
	unsigned short int      lr_hash_map;    /* back pointer to hash map */
	unsigned short int      lr_flags;       /* pad */
} __attribute__((aligned(8)));

/* lr_flags - only 16 bits available */
#define LRO_EJECT_REQ   0x1


#define TCP_LRO_FLOW_UNINIT TCP_LRO_NUM_FLOWS+1
#define TCP_LRO_FLOW_NOTFOUND TCP_LRO_FLOW_UNINIT

/* Max packets to be coalesced before pushing to app */
#define LRO_MX_COALESCE_PKTS (8)

/*
 * Min num of bytes in a packet to trigger coalescing
 */
#define LRO_MIN_COALESC_SZ  (1300)

/*
 * Max amount of time to wait before flushing flows in msecs.
 * Units are in msecs.
 * This number has been carefully chosen and should be altered with care.
 */
#define LRO_MX_TIME_TO_BUFFER 10

/* similar to INP_PCBHASH */
#define LRO_HASH(faddr, laddr, fport, lport, mask) \
	(((faddr) ^ ((laddr) >> 16) ^ ntohs((lport) ^ (fport))) & (mask))
#endif

#endif /* TCP_LRO_H_ */

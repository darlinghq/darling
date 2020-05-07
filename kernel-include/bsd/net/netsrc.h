/*
 * Copyright (c) 2011-2013 Apple Inc. All rights reserved.
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

#ifndef __NET_NETSRC_H__

#include <netinet/in.h>

#define NETSRC_CTLNAME  "com.apple.netsrc"

#define NETSRC_VERSION1 1
#define NETSRC_VERSION2 2
#define NETSRC_CURVERS  NETSRC_VERSION2

struct netsrc_req {
	unsigned int nrq_ver;
	unsigned int nrq_ifscope;
	union {
		union sockaddr_in_4_6 nrq_dst;
		union sockaddr_in_4_6 _usa;
	};
};

#define nrq_sin         _usa.sin
#define nrq_sin6        _usa.sin6

struct netsrc_repv1 {
	union {
		union sockaddr_in_4_6 nrp_src;
		union sockaddr_in_4_6 _usa;
	};
#define NETSRC_IP6_FLAG_TENTATIVE       0x0001
#define NETSRC_IP6_FLAG_TEMPORARY       0x0002
#define NETSRC_IP6_FLAG_DEPRECATED      0x0004
#define NETSRC_IP6_FLAG_OPTIMISTIC      0x0008
#define NETSRC_IP6_FLAG_SECURED         0x0010
	uint16_t nrp_flags;
	uint16_t nrp_label;
	uint16_t nrp_precedence;
	uint16_t nrp_dstlabel;
	uint16_t nrp_dstprecedence;
	uint16_t nrp_unused;    // Padding
};

struct netsrc_repv2 {
	union {
		union sockaddr_in_4_6 nrp_src;
		union sockaddr_in_4_6 _usa;
	};
	uint32_t nrp_min_rtt;
	uint32_t nrp_connection_attempts;
	uint32_t nrp_connection_successes;
	// Continues from above, fixes naming
#define NETSRC_FLAG_IP6_TENTATIVE       NETSRC_IP6_FLAG_TENTATIVE
#define NETSRC_FLAG_IP6_TEMPORARY       NETSRC_IP6_FLAG_TEMPORARY
#define NETSRC_FLAG_IP6_DEPRECATED      NETSRC_IP6_FLAG_DEPRECATED
#define NETSRC_FLAG_IP6_OPTIMISTIC      NETSRC_IP6_FLAG_OPTIMISTIC
#define NETSRC_FLAG_IP6_SECURED         NETSRC_IP6_FLAG_SECURED
#define NETSRC_FLAG_ROUTEABLE           0x00000020
#define NETSRC_FLAG_DIRECT                      0x00000040
#define NETSRC_FLAG_AWDL                        0x00000080
#define NETSRC_FLAG_IP6_DYNAMIC         0x00000100
#define NETSRC_FLAG_IP6_AUTOCONF        0x00000200
	uint32_t nrp_flags;
	uint16_t nrp_label;
	uint16_t nrp_precedence;
	uint16_t nrp_dstlabel;
	uint16_t nrp_dstprecedence;
	uint16_t nrp_ifindex;
	uint16_t nrp_unused; // Padding
};

#define netsrc_rep netsrc_repv2

#define nrp_sin         nrp_src.sin
#define nrp_sin6        nrp_src.sin6

#ifdef KERNEL_PRIVATE
__private_extern__ void netsrc_init(void);
#endif

#endif /* __NET_NETSRC_H__ */

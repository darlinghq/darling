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
 * Copyright (c) 1982, 1986, 1993
 *      The Regents of the University of California.  All rights reserved.
 */
#ifndef __KDP_UDP_H
#define __KDP_UDP_H

#include <libsa/types.h>
#include <libkern/OSByteOrder.h>   /* OSSwap functions */
#include <stdint.h>

struct kdp_in_addr {
	uint32_t s_addr;
};

#define ETHER_ADDR_LEN 6

struct kdp_ether_addr {
	u_char ether_addr_octet[ETHER_ADDR_LEN];
};

typedef struct kdp_ether_addr enet_addr_t;

extern struct kdp_ether_addr kdp_get_mac_addr(void);
unsigned int  kdp_get_ip_address(void);

struct  kdp_ether_header {
	u_char  ether_dhost[ETHER_ADDR_LEN];
	u_char  ether_shost[ETHER_ADDR_LEN];
	u_short ether_type;
};

typedef struct kdp_ether_header ether_header_t;

#define ntohs(x)           OSSwapBigToHostInt16(x)
#define ntohl(x)           OSSwapBigToHostInt32(x)
#define htons(x)           OSSwapHostToBigInt16(x)
#define htonl(x)           OSSwapHostToBigInt32(x)

/*
 * IONetworkingFamily only.
 */
typedef uint32_t (*kdp_link_t)(void);
typedef boolean_t (*kdp_mode_t)(boolean_t);
void    kdp_register_link(kdp_link_t link, kdp_mode_t mode);
void    kdp_unregister_link(kdp_link_t link, kdp_mode_t mode);

#endif /* __KDP_UDP_H */

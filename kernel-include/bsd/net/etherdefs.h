/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
 * Mach Operating System
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 * HISTORY
 * 11-Jul-93  Mac Gillon (mgillon) at NeXT
 *	Integrated MULTICAST support
 *
 * 09-Apr-90  Bradley Taylor (btaylor) at NeXT, Inc.
 *	Created. Originally part of <netinet/if_ether.h>.
 */
#ifndef _ETHERDEFS_
#define _ETHERDEFS_
#include <sys/appleapiopts.h>
#if !defined(KERNEL) || defined(__APPLE_API_OBSOLETE)

#include <net/ethernet.h>
#warning net/etherdefs.h is obsolete! Use net/ethernet.h

#include        <netinet/if_ether.h>

/*
 * Ethernet address - 6 octets
 */
#define NUM_EN_ADDR_BYTES       ETHER_ADDR_LEN


typedef struct ether_addr enet_addr_t;

typedef struct ether_header ether_header_t;

#define IFTYPE_ETHERNET "10MB Ethernet"

#define ETHERHDRSIZE    ETHER_HDR_LEN
#define ETHERMAXPACKET  ETHER_MAX_LEN
#define ETHERMINPACKET  ETHER_MIN_LEN
#define ETHERCRC        ETHER_CRC_LEN

/*
 * Byte and bit in an enet_addr_t defining individual/group destination.
 */
#define EA_GROUP_BYTE   0
#define EA_GROUP_BIT    0x01


#endif /* KERNEL && !__APPLE_API_OBSOLETE */
#endif /* _ETHERDEFS_ */

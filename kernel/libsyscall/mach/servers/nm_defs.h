/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
 * Random definitions for the network service that everyone needs!
 */

/*
 * HISTORY:
 * 27-Mar-90  Gregg Kellogg (gk) at NeXT
 *	include <sys/netport.h> rather than <sys/ipc_netport.h>
 *
 * 24-Aug-88  Daniel Julin (dpj) at Carnegie-Mellon University
 *	Replace sys/mach_ipc_netport.h with kern/ipc_netport.h. Sigh.
 *
 * 24-May-88  Daniel Julin (dpj) at Carnegie-Mellon University
 *	Replace mach_ipc_vmtp.h with mach_ipc_netport.h.
 *
 *  4-Sep-87  Daniel Julin (dpj) at Carnegie-Mellon University
 *	Fixed for new kernel include files which declare a lot
 *	of network server stuff internally, because of the NETPORT
 *	option.
 *
 *  5-Nov-86  Robert Sansom (rds) at Carnegie-Mellon University
 *	Started.
 *
 */

#ifndef	_NM_DEFS_
#define	_NM_DEFS_

/*
 * netaddr_t is declared with the kernel files,
 * in <sys/netport.h>.
 */
#include	<sys/netport.h>

#ifdef	notdef
typedef unsigned long	netaddr_t;
#endif	/* notdef */

typedef union {
    struct {
	unsigned char ia_net_owner;
	unsigned char ia_net_node_type;
	unsigned char ia_host_high;
	unsigned char ia_host_low;
    } ia_bytes;
    netaddr_t ia_netaddr;
} ip_addr_t;

#endif	/* _NM_DEFS_ */


/*
 * Copyright (c) 2000-2010 Apple Inc. All rights reserved.
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
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */

#ifndef __NFS_KRPC_H__
#define __NFS_KRPC_H__

#include <sys/appleapiopts.h>

#include <sys/cdefs.h>

#ifdef __APPLE_API_PRIVATE
int krpc_call(struct sockaddr_in *sin, u_int sotype,
    u_int prog, u_int vers, u_int func,
    mbuf_t *data, struct sockaddr_in *from);

int krpc_portmap(struct sockaddr_in *sin,
    u_int prog, u_int vers, u_int proto, u_int16_t *portp);


/*
 * RPC definitions for the portmapper (portmap and rpcbind)
 */
#define PMAPPORT                111
#define PMAPPROG                100000
#define PMAPVERS                2
#define PMAPPROC_NULL           0
#define PMAPPROC_SET            1
#define PMAPPROC_UNSET          2
#define PMAPPROC_GETPORT        3
#define PMAPPROC_DUMP           4
#define PMAPPROC_CALLIT         5

#define RPCBPROG                PMAPPROG
#define RPCBVERS3               3
#define RPCBVERS4               4
#define RPCBPROC_NULL           0
#define RPCBPROC_SET            1
#define RPCBPROC_UNSET          2
#define RPCBPROC_GETADDR        3
#define RPCBPROC_DUMP           4
#define RPCBPROC_CALLIT         5
#define RPCBPROC_BCAST          RPCBPROC_CALLIT
#define RPCBPROC_GETTIME        6
#define RPCBPROC_UADDR2TADDR    7
#define RPCBPROC_TADDR2UADDR    8
#define RPCBPROC_GETVERSADDR    9
#define RPCBPROC_INDIRECT       10
#define RPCBPROC_GETADDRLIST    11
#define RPCBPROC_GETSTAT        12


/*
 * RPC definitions for bootparamd
 */
#define BOOTPARAM_PROG          100026
#define BOOTPARAM_VERS          1
#define BOOTPARAM_WHOAMI        1
#define BOOTPARAM_GETFILE       2

#endif /* __APPLE_API_PRIVATE */
#endif /* __NFS_KRPC_H__ */

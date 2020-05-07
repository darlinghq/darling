/*
 * Copyright (c) 2000-2018 Apple Inc. All rights reserved.
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
/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rick Macklem at The University of Guelph.
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
 *	@(#)rpcv2.h	8.2 (Berkeley) 3/30/95
 * FreeBSD-Id: rpcv2.h,v 1.8 1997/05/11 18:05:39 tegge Exp $
 */


#ifndef _NFS_RPCV2_H_
#define _NFS_RPCV2_H_

#include <sys/appleapiopts.h>

#ifdef __APPLE_API_PRIVATE
/*
 * Definitions for Sun RPC Version 2, from
 * "RPC: Remote Procedure Call Protocol Specification" RFC1057
 */

/* Version # */
#define RPC_VER2        2

/* Authentication */
#define RPCAUTH_NULL    0
#define RPCAUTH_NONE    RPCAUTH_NULL
#define RPCAUTH_UNIX    1
#define RPCAUTH_SYS     RPCAUTH_UNIX
#define RPCAUTH_SHORT   2
#define RPCAUTH_KERB4   4
#define RPCAUTH_KRB5    390003
#define RPCAUTH_KRB5I   390004
#define RPCAUTH_KRB5P   390005
#define RPCAUTH_INVALID ~0U
#define RPCAUTH_UNKNOWN RPCAUTH_INVALID

#define RPCAUTH_MAXSIZ  400
#define RPCAUTH_UNIXGIDS 16

/*
 * Constants associated with authentication flavours.
 */
#define RPCAKN_FULLNAME 0
#define RPCAKN_NICKNAME 1

/* Rpc Constants */
#define RPC_CALL        0
#define RPC_REPLY       1
#define RPC_MSGACCEPTED 0
#define RPC_MSGDENIED   1
#define RPC_SUCCESS     0
#define RPC_PROGUNAVAIL 1
#define RPC_PROGMISMATCH        2
#define RPC_PROCUNAVAIL 3
#define RPC_GARBAGE     4               /* I like this one */
#define RPC_SYSTEM_ERR  5
#define RPC_MISMATCH    0
#define RPC_AUTHERR     1

/* Authentication failures */
#define AUTH_BADCRED            1
#define AUTH_REJECTCRED         2
#define AUTH_BADVERF            3
#define AUTH_REJECTVERF         4
#define AUTH_TOOWEAK            5       /* Give em wheaties */
#define AUTH_INVALIDRESP        6
#define AUTH_FAILED             7
#define AUTH_KERB_GENERIC       8
#define AUTH_TIMEEXPIRE         9
#define AUTH_TKT_FILE           10
#define AUTH_DECODE             11
#define AUTH_NET_ADDR           12
#define RPCSEC_GSS_CREDPROBLEM  13
#define RPCSEC_GSS_CTXPROBLEM   14

/* Sizes of rpc header parts */
#define RPC_SIZ         24
#define RPC_REPLYSIZ    28

/* RPC Prog definitions */
#define RPCPROG_MNT     100005
#define RPCMNT_VER1     1
#define RPCMNT_VER3     3
#define RPCMNT_MOUNT    1
#define RPCMNT_DUMP     2
#define RPCMNT_UMOUNT   3
#define RPCMNT_UMNTALL  4
#define RPCMNT_EXPORT   5
#define RPCMNT_NAMELEN  255
#define RPCMNT_PATHLEN  1024
#define RPCPROG_NFS     100003

#define RPCPROG_RQUOTA  100011
#define RPCRQUOTA_VER           1
#define RPCRQUOTA_EXT_VER       2
#define RPCRQUOTA_GET           1
#define RQUOTA_STAT_OK          1
#define RQUOTA_STAT_NOQUOTA     2
#define RQUOTA_STAT_EPERM       3

/* Local transports for rpcbind */
#define RPCB_TICOTSORD_PATH "/var/run/rpcb.ticotsord"
#define RPCB_TICLTS_PATH "/var/run/rpcb.ticlst"

/* Local transport for nfs */
#define NFS_TICOTSORD_PATH "/var/ran/nfs.ticotsord"
#define NFS_TICLTS_PATH "/var/run/nfs.ticlts"

#endif /* __APPLE_API_PRIVATE */
#endif /* _NFS_RPCV2_H_ */

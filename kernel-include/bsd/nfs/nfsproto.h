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
 *	@(#)nfsproto.h	8.2 (Berkeley) 3/30/95
 * FreeBSD-Id: nfsproto.h,v 1.3 1997/02/22 09:42:50 peter Exp $
 */

#ifndef _NFS_NFSPROTO_H_
#define _NFS_NFSPROTO_H_

#include <sys/appleapiopts.h>

#ifdef __APPLE_API_PRIVATE

/*
 * NFS definitions per the various NFS protocol specs:
 * Version 2 (RFC 1094), Version 3 (RFC 1813), and Version 4 (RFC 3530)
 * and various protocol-related implementation definitions.
 */

/* Only define these if nfs_prot.h hasn't been included */
#ifndef NFS_PROGRAM

#define NFS_PORT        2049
#define NFS_PROG        100003
#define NFS_VER2        2
#define NFS_VER3        3
#define NFS_VER4        4
#define NFS_V2MAXDATA   8192
#define NFS_MAXDGRAMDATA 16384
#define NFS_PREFDGRAMDATA 8192

#ifdef XNU_TARGET_OS_IOS
#define NFS_MAXDATA     (32 * PAGE_SIZE) /* Same as NFS_MAXBSIZE from nfsnode.h */
#else /*  TARGET_OS_IOS */
#define NFS_MAXDATA     (64*1024)
#endif /*  TARGET_OS_IOS */

#define NFSRV_MAXDATA   (64*1024) // XXX not ready for >64K
#define NFS_MAXPATHLEN  1024
#define NFS_MAXNAMLEN   255
#define NFS_MAXPACKET   (16*1024*1024)
#define NFS_UDPSOCKBUF  (224*1024)
#define NFS_FABLKSIZE   512     /* Size in bytes of a block wrt fa_blocks */

#define NFS4_CALLBACK_PROG              0x4E465343 /* "NFSC" */
#define NFS4_CALLBACK_PROG_VERSION      1

/* Stat numbers for NFS RPC returns */
#define NFS_OK                          0
#define NFSERR_PERM                     1
#define NFSERR_NOENT                    2
#define NFSERR_IO                       5
#define NFSERR_NXIO                     6
#define NFSERR_ACCES                    13
#define NFSERR_EXIST                    17
#define NFSERR_XDEV                     18      /* Version 3 only */
#define NFSERR_NODEV                    19
#define NFSERR_NOTDIR                   20
#define NFSERR_ISDIR                    21
#define NFSERR_INVAL                    22      /* Version 3 only */
#define NFSERR_FBIG                     27
#define NFSERR_NOSPC                    28
#define NFSERR_ROFS                     30
#define NFSERR_MLINK                    31      /* Version 3 only */
#define NFSERR_NAMETOL                  63
#define NFSERR_NOTEMPTY                 66
#define NFSERR_DQUOT                    69
#define NFSERR_STALE                    70
#define NFSERR_REMOTE                   71      /* Version 3 only */
#define NFSERR_WFLUSH                   99      /* Version 2 only */
#define NFSERR_BADHANDLE                10001   /* The rest Version 3 only */
#define NFSERR_NOT_SYNC                 10002
#define NFSERR_BAD_COOKIE               10003
#define NFSERR_NOTSUPP                  10004
#define NFSERR_TOOSMALL                 10005
#define NFSERR_SERVERFAULT              10006
#define NFSERR_BADTYPE                  10007
#define NFSERR_JUKEBOX                  10008
#define NFSERR_TRYLATER                 NFSERR_JUKEBOX
#define NFSERR_DELAY                    NFSERR_JUKEBOX
#define NFSERR_SAME                     10009   /* The rest Version 4 only */
#define NFSERR_DENIED                   10010
#define NFSERR_EXPIRED                  10011
#define NFSERR_LOCKED                   10012
#define NFSERR_GRACE                    10013
#define NFSERR_FHEXPIRED                10014
#define NFSERR_SHARE_DENIED             10015
#define NFSERR_WRONGSEC                 10016
#define NFSERR_CLID_INUSE               10017
#define NFSERR_RESOURCE                 10018
#define NFSERR_MOVED                    10019
#define NFSERR_NOFILEHANDLE             10020
#define NFSERR_MINOR_VERS_MISMATCH      10021
#define NFSERR_STALE_CLIENTID           10022
#define NFSERR_STALE_STATEID            10023
#define NFSERR_OLD_STATEID              10024
#define NFSERR_BAD_STATEID              10025
#define NFSERR_BAD_SEQID                10026
#define NFSERR_NOT_SAME                 10027
#define NFSERR_LOCK_RANGE               10028
#define NFSERR_SYMLINK                  10029
#define NFSERR_RESTOREFH                10030
#define NFSERR_LEASE_MOVED              10031
#define NFSERR_ATTRNOTSUPP              10032
#define NFSERR_NO_GRACE                 10033
#define NFSERR_RECLAIM_BAD              10034
#define NFSERR_RECLAIM_CONFLICT         10035
#define NFSERR_BADXDR                   10036
#define NFSERR_LOCKS_HELD               10037
#define NFSERR_OPENMODE                 10038
#define NFSERR_BADOWNER                 10039
#define NFSERR_BADCHAR                  10040
#define NFSERR_BADNAME                  10041
#define NFSERR_BAD_RANGE                10042
#define NFSERR_LOCK_NOTSUPP             10043
#define NFSERR_OP_ILLEGAL               10044
#define NFSERR_DEADLOCK                 10045
#define NFSERR_FILE_OPEN                10046
#define NFSERR_ADMIN_REVOKED            10047
#define NFSERR_CB_PATH_DOWN             10048

#define NFSERR_STALEWRITEVERF           30001   /* Fake return for nfs_commit() */
#define NFSERR_DIRBUFDROPPED            30002   /* Fake return for nfs*_readdir_rpc() */

/*
 * For gss we would like to return EAUTH when we don't have or can't get credentials,
 * but some callers don't know what to do with it, so we define our own version
 * of EAUTH to be EACCES
 */
#define NFSERR_EAUTH    EACCES

#define NFSERR_RETVOID          0x20000000 /* Return void, not error */
#define NFSERR_AUTHERR          0x40000000 /* Mark an authentication error */
#define NFSERR_RETERR           0x80000000 /* Mark an error return for V3 */

#endif /* !NFS_PROGRAM */

/* Sizes in bytes of various nfs rpc components */
#define NFSX_UNSIGNED   4

/* specific to NFS Version 2 */
#define NFSX_V2FH       32
#define NFSX_V2FATTR    68
#define NFSX_V2SATTR    32
#define NFSX_V2COOKIE   4
#define NFSX_V2STATFS   20

/* specific to NFS Version 3 */
#define NFSX_V3FHMAX            64      /* max. allowed by protocol */
#define NFSX_V3FATTR            84
#define NFSX_V3SATTR            60      /* max. all fields filled in */
#define NFSX_V3POSTOPATTR       (NFSX_V3FATTR + NFSX_UNSIGNED)
#define NFSX_V3WCCDATA          (NFSX_V3POSTOPATTR + 8 * NFSX_UNSIGNED)
#define NFSX_V3COOKIEVERF       8
#define NFSX_V3WRITEVERF        8
#define NFSX_V3CREATEVERF       8
#define NFSX_V3STATFS           52
#define NFSX_V3FSINFO           48
#define NFSX_V3PATHCONF         24

/* specific to NFS Version 4 */
#define NFS4_FHSIZE             128
#define NFS4_VERIFIER_SIZE      8
#define NFS4_OPAQUE_LIMIT       1024

/* variants for multiple versions */
#define NFSX_FH(V)              (((V) == NFS_VER2) ? NFSX_V2FH : (NFSX_UNSIGNED + \
	                         (((V) == NFS_VER3) ? NFSX_V3FHMAX : NFS4_FHSIZE)))
#define NFSX_SRVFH(V, FH)        (((V) == NFS_VER2) ? NFSX_V2FH : (FH)->nfh_len)
#define NFSX_FATTR(V)           (((V) == NFS_VER3) ? NFSX_V3FATTR : NFSX_V2FATTR)
#define NFSX_PREOPATTR(V)       (((V) == NFS_VER3) ? (7 * NFSX_UNSIGNED) : 0)
#define NFSX_POSTOPATTR(V)      (((V) == NFS_VER3) ? (NFSX_V3FATTR + NFSX_UNSIGNED) : 0)
#define NFSX_POSTOPORFATTR(V)   (((V) == NFS_VER3) ? (NFSX_V3FATTR + NFSX_UNSIGNED) : NFSX_V2FATTR)
#define NFSX_WCCDATA(V)         (((V) == NFS_VER3) ? NFSX_V3WCCDATA : 0)
#define NFSX_WCCORFATTR(V)      (((V) == NFS_VER3) ? NFSX_V3WCCDATA : NFSX_V2FATTR)
#define NFSX_SATTR(V)           (((V) == NFS_VER3) ? NFSX_V3SATTR : NFSX_V2SATTR)
#define NFSX_COOKIEVERF(V)      (((V) == NFS_VER3) ? NFSX_V3COOKIEVERF : 0)
#define NFSX_WRITEVERF(V)       (((V) == NFS_VER3) ? NFSX_V3WRITEVERF : 0)
#define NFSX_READDIR(V)         (((V) == NFS_VER3) ? (5 * NFSX_UNSIGNED) : \
	                                (2 * NFSX_UNSIGNED))
#define NFSX_STATFS(V)          (((V) == NFS_VER3) ? NFSX_V3STATFS : NFSX_V2STATFS)

/* Only define these if nfs_prot.h hasn't been included */
#ifndef NFS_PROGRAM

/* nfs rpc procedure numbers (before version mapping) */
#define NFSPROC_NULL            0
#define NFSPROC_GETATTR         1
#define NFSPROC_SETATTR         2
#define NFSPROC_LOOKUP          3
#define NFSPROC_ACCESS          4
#define NFSPROC_READLINK        5
#define NFSPROC_READ            6
#define NFSPROC_WRITE           7
#define NFSPROC_CREATE          8
#define NFSPROC_MKDIR           9
#define NFSPROC_SYMLINK         10
#define NFSPROC_MKNOD           11
#define NFSPROC_REMOVE          12
#define NFSPROC_RMDIR           13
#define NFSPROC_RENAME          14
#define NFSPROC_LINK            15
#define NFSPROC_READDIR         16
#define NFSPROC_READDIRPLUS     17
#define NFSPROC_FSSTAT          18
#define NFSPROC_FSINFO          19
#define NFSPROC_PATHCONF        20
#define NFSPROC_COMMIT          21

#endif /* !NFS_PROGRAM */

#define NFSPROC_NOOP            22
#define NFS_NPROCS              23

/* Actual Version 2 procedure numbers */
#define NFSV2PROC_NULL          0
#define NFSV2PROC_GETATTR       1
#define NFSV2PROC_SETATTR       2
#define NFSV2PROC_NOOP          3
#define NFSV2PROC_ROOT          NFSV2PROC_NOOP  /* Obsolete */
#define NFSV2PROC_LOOKUP        4
#define NFSV2PROC_READLINK      5
#define NFSV2PROC_READ          6
#define NFSV2PROC_WRITECACHE    NFSV2PROC_NOOP  /* Obsolete */
#define NFSV2PROC_WRITE         8
#define NFSV2PROC_CREATE        9
#define NFSV2PROC_REMOVE        10
#define NFSV2PROC_RENAME        11
#define NFSV2PROC_LINK          12
#define NFSV2PROC_SYMLINK       13
#define NFSV2PROC_MKDIR         14
#define NFSV2PROC_RMDIR         15
#define NFSV2PROC_READDIR       16
#define NFSV2PROC_STATFS        17

/*
 * Constants used by the Version 3 protocol for various RPCs
 */

#define NFSV3FSINFO_LINK                0x01
#define NFSV3FSINFO_SYMLINK             0x02
#define NFSV3FSINFO_HOMOGENEOUS         0x08
#define NFSV3FSINFO_CANSETTIME          0x10

/* time setting constants */
#define NFS_TIME_DONT_CHANGE            0
#define NFS_TIME_SET_TO_SERVER          1
#define NFS_TIME_SET_TO_CLIENT          2
#define NFS4_TIME_SET_TO_SERVER         0
#define NFS4_TIME_SET_TO_CLIENT         1

/* access() constants */
#define NFS_ACCESS_READ                 0x01
#define NFS_ACCESS_LOOKUP               0x02
#define NFS_ACCESS_MODIFY               0x04
#define NFS_ACCESS_EXTEND               0x08
#define NFS_ACCESS_DELETE               0x10
#define NFS_ACCESS_EXECUTE              0x20
#define NFS_ACCESS_ALL (NFS_ACCESS_READ | NFS_ACCESS_MODIFY             \
	                 | NFS_ACCESS_EXTEND | NFS_ACCESS_EXECUTE       \
	                 | NFS_ACCESS_DELETE | NFS_ACCESS_LOOKUP)

/* NFS WRITE how constants */
#define NFS_WRITE_UNSTABLE              0
#define NFS_WRITE_DATASYNC              1
#define NFS_WRITE_FILESYNC              2

/* NFS CREATE types */
#define NFS_CREATE_UNCHECKED            0
#define NFS_CREATE_GUARDED              1
#define NFS_CREATE_EXCLUSIVE            2

/* Only define these if nfs_prot.h hasn't been included */
#ifndef NFS_PROGRAM
/* NFS object types */
typedef enum { NFNON=0, NFREG=1, NFDIR=2, NFBLK=3, NFCHR=4, NFLNK=5,
	       NFSOCK=6, NFFIFO=7, NFATTRDIR=8, NFNAMEDATTR=9 } nfstype;
#endif /* !NFS_PROGRAM */

/*
 * File Handle (32 bytes for version 2), variable up to 64 for version 3
 * and variable up to 128 bytes for version 4.
 * File Handles of up to NFS_SMALLFH in size are stored directly in the
 * nfs node, whereas larger ones are malloc'd. (This never happens when
 * NFS_SMALLFH is set to the largest size.)
 * NFS_SMALLFH should be in the range of 32 to 64 and be divisible by 4.
 */
#ifndef NFS_SMALLFH
#define NFS_SMALLFH     64
#endif

/*
 * NFS attribute management stuff
 */
#define NFS_ATTR_BITMAP_LEN     2
#define NFS_BITMAP_SET(B, I)    (((uint32_t *)(B))[(I)/32] |= 1U<<((I)%32))
#define NFS_BITMAP_CLR(B, I)    (((uint32_t *)(B))[(I)/32] &= ~(1U<<((I)%32)))
#define NFS_BITMAP_ISSET(B, I)  (((uint32_t *)(B))[(I)/32] & (1U<<((I)%32)))
#define NFS_BITMAP_ZERO(B, L) \
	do { \
	        int __i; \
	        for (__i=0; __i < (L); __i++) \
	                ((uint32_t*)(B))[__i] = 0; \
	} while (0)

extern uint32_t nfs_fs_attr_bitmap[NFS_ATTR_BITMAP_LEN];
extern uint32_t nfs_object_attr_bitmap[NFS_ATTR_BITMAP_LEN];
extern uint32_t nfs_getattr_bitmap[NFS_ATTR_BITMAP_LEN];

#define NFS_CLEAR_ATTRIBUTES(A) NFS_BITMAP_ZERO((A), NFS_ATTR_BITMAP_LEN)
#define NFS_COPY_ATTRIBUTES(SRC, DST) \
	do { \
	int __i; \
	for (__i=0; __i < NFS_ATTR_BITMAP_LEN; __i++) \
	        ((uint32_t*)(DST))[__i] = ((uint32_t*)(SRC))[__i]; \
	} while (0)

/* NFS attributes */
#define NFS_FATTR_SUPPORTED_ATTRS               0
#define NFS_FATTR_TYPE                          1
#define NFS_FATTR_FH_EXPIRE_TYPE                2
#define NFS_FATTR_CHANGE                        3
#define NFS_FATTR_SIZE                          4
#define NFS_FATTR_LINK_SUPPORT                  5
#define NFS_FATTR_SYMLINK_SUPPORT               6
#define NFS_FATTR_NAMED_ATTR                    7
#define NFS_FATTR_FSID                          8
#define NFS_FATTR_UNIQUE_HANDLES                9
#define NFS_FATTR_LEASE_TIME                    10
#define NFS_FATTR_RDATTR_ERROR                  11
#define NFS_FATTR_FILEHANDLE                    19
#define NFS_FATTR_ACL                           12
#define NFS_FATTR_ACLSUPPORT                    13
#define NFS_FATTR_ARCHIVE                       14
#define NFS_FATTR_CANSETTIME                    15
#define NFS_FATTR_CASE_INSENSITIVE              16
#define NFS_FATTR_CASE_PRESERVING               17
#define NFS_FATTR_CHOWN_RESTRICTED              18
#define NFS_FATTR_FILEID                        20
#define NFS_FATTR_FILES_AVAIL                   21
#define NFS_FATTR_FILES_FREE                    22
#define NFS_FATTR_FILES_TOTAL                   23
#define NFS_FATTR_FS_LOCATIONS                  24
#define NFS_FATTR_HIDDEN                        25
#define NFS_FATTR_HOMOGENEOUS                   26
#define NFS_FATTR_MAXFILESIZE                   27
#define NFS_FATTR_MAXLINK                       28
#define NFS_FATTR_MAXNAME                       29
#define NFS_FATTR_MAXREAD                       30
#define NFS_FATTR_MAXWRITE                      31
#define NFS_FATTR_MIMETYPE                      32
#define NFS_FATTR_MODE                          33
#define NFS_FATTR_NO_TRUNC                      34
#define NFS_FATTR_NUMLINKS                      35
#define NFS_FATTR_OWNER                         36
#define NFS_FATTR_OWNER_GROUP                   37
#define NFS_FATTR_QUOTA_AVAIL_HARD              38
#define NFS_FATTR_QUOTA_AVAIL_SOFT              39
#define NFS_FATTR_QUOTA_USED                    40
#define NFS_FATTR_RAWDEV                        41
#define NFS_FATTR_SPACE_AVAIL                   42
#define NFS_FATTR_SPACE_FREE                    43
#define NFS_FATTR_SPACE_TOTAL                   44
#define NFS_FATTR_SPACE_USED                    45
#define NFS_FATTR_SYSTEM                        46
#define NFS_FATTR_TIME_ACCESS                   47
#define NFS_FATTR_TIME_ACCESS_SET               48
#define NFS_FATTR_TIME_BACKUP                   49
#define NFS_FATTR_TIME_CREATE                   50
#define NFS_FATTR_TIME_DELTA                    51
#define NFS_FATTR_TIME_METADATA                 52
#define NFS_FATTR_TIME_MODIFY                   53
#define NFS_FATTR_TIME_MODIFY_SET               54
#define NFS_FATTR_MOUNTED_ON_FILEID             55

#define NFS4_ALL_ATTRIBUTES(A) \
	do { \
	/* required: */ \
	NFS_BITMAP_SET((A), NFS_FATTR_SUPPORTED_ATTRS); \
	NFS_BITMAP_SET((A), NFS_FATTR_TYPE); \
	NFS_BITMAP_SET((A), NFS_FATTR_FH_EXPIRE_TYPE); \
	NFS_BITMAP_SET((A), NFS_FATTR_CHANGE); \
	NFS_BITMAP_SET((A), NFS_FATTR_SIZE); \
	NFS_BITMAP_SET((A), NFS_FATTR_LINK_SUPPORT); \
	NFS_BITMAP_SET((A), NFS_FATTR_SYMLINK_SUPPORT); \
	NFS_BITMAP_SET((A), NFS_FATTR_NAMED_ATTR); \
	NFS_BITMAP_SET((A), NFS_FATTR_FSID); \
	NFS_BITMAP_SET((A), NFS_FATTR_UNIQUE_HANDLES); \
	NFS_BITMAP_SET((A), NFS_FATTR_LEASE_TIME); \
	NFS_BITMAP_SET((A), NFS_FATTR_RDATTR_ERROR); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILEHANDLE); \
	/* optional: */ \
	NFS_BITMAP_SET((A), NFS_FATTR_ACL); \
	NFS_BITMAP_SET((A), NFS_FATTR_ACLSUPPORT); \
	NFS_BITMAP_SET((A), NFS_FATTR_ARCHIVE); \
	NFS_BITMAP_SET((A), NFS_FATTR_CANSETTIME); \
	NFS_BITMAP_SET((A), NFS_FATTR_CASE_INSENSITIVE); \
	NFS_BITMAP_SET((A), NFS_FATTR_CASE_PRESERVING); \
	NFS_BITMAP_SET((A), NFS_FATTR_CHOWN_RESTRICTED); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILEID); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILES_AVAIL); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILES_FREE); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILES_TOTAL); \
	NFS_BITMAP_SET((A), NFS_FATTR_FS_LOCATIONS); \
	NFS_BITMAP_SET((A), NFS_FATTR_HIDDEN); \
	NFS_BITMAP_SET((A), NFS_FATTR_HOMOGENEOUS); \
	NFS_BITMAP_SET((A), NFS_FATTR_MAXFILESIZE); \
	NFS_BITMAP_SET((A), NFS_FATTR_MAXLINK); \
	NFS_BITMAP_SET((A), NFS_FATTR_MAXNAME); \
	NFS_BITMAP_SET((A), NFS_FATTR_MAXREAD); \
	NFS_BITMAP_SET((A), NFS_FATTR_MAXWRITE); \
	NFS_BITMAP_SET((A), NFS_FATTR_MIMETYPE); \
	NFS_BITMAP_SET((A), NFS_FATTR_MODE); \
	NFS_BITMAP_SET((A), NFS_FATTR_NO_TRUNC); \
	NFS_BITMAP_SET((A), NFS_FATTR_NUMLINKS); \
	NFS_BITMAP_SET((A), NFS_FATTR_OWNER); \
	NFS_BITMAP_SET((A), NFS_FATTR_OWNER_GROUP); \
	NFS_BITMAP_SET((A), NFS_FATTR_QUOTA_AVAIL_HARD); \
	NFS_BITMAP_SET((A), NFS_FATTR_QUOTA_AVAIL_SOFT); \
	NFS_BITMAP_SET((A), NFS_FATTR_QUOTA_USED); \
	NFS_BITMAP_SET((A), NFS_FATTR_RAWDEV); \
	NFS_BITMAP_SET((A), NFS_FATTR_SPACE_AVAIL); \
	NFS_BITMAP_SET((A), NFS_FATTR_SPACE_FREE); \
	NFS_BITMAP_SET((A), NFS_FATTR_SPACE_TOTAL); \
	NFS_BITMAP_SET((A), NFS_FATTR_SPACE_USED); \
	NFS_BITMAP_SET((A), NFS_FATTR_SYSTEM); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_ACCESS); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_ACCESS_SET); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_BACKUP); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_CREATE); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_DELTA); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_METADATA); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_MODIFY); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_MODIFY_SET); \
	NFS_BITMAP_SET((A), NFS_FATTR_MOUNTED_ON_FILEID); \
	} while (0)

#define NFS4_PER_OBJECT_ATTRIBUTES(A) \
	do { \
	/* required: */ \
	NFS_BITMAP_SET((A), NFS_FATTR_TYPE); \
	NFS_BITMAP_SET((A), NFS_FATTR_CHANGE); \
	NFS_BITMAP_SET((A), NFS_FATTR_SIZE); \
	NFS_BITMAP_SET((A), NFS_FATTR_NAMED_ATTR); \
	NFS_BITMAP_SET((A), NFS_FATTR_FSID); \
	NFS_BITMAP_SET((A), NFS_FATTR_RDATTR_ERROR); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILEHANDLE); \
	/* optional: */ \
	NFS_BITMAP_SET((A), NFS_FATTR_ACL); \
	NFS_BITMAP_SET((A), NFS_FATTR_ARCHIVE); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILEID); \
	NFS_BITMAP_SET((A), NFS_FATTR_HIDDEN); \
	NFS_BITMAP_SET((A), NFS_FATTR_MAXLINK); \
	NFS_BITMAP_SET((A), NFS_FATTR_MIMETYPE); \
	NFS_BITMAP_SET((A), NFS_FATTR_MODE); \
	NFS_BITMAP_SET((A), NFS_FATTR_NUMLINKS); \
	NFS_BITMAP_SET((A), NFS_FATTR_OWNER); \
	NFS_BITMAP_SET((A), NFS_FATTR_OWNER_GROUP); \
	NFS_BITMAP_SET((A), NFS_FATTR_RAWDEV); \
	NFS_BITMAP_SET((A), NFS_FATTR_SPACE_USED); \
	NFS_BITMAP_SET((A), NFS_FATTR_SYSTEM); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_ACCESS); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_BACKUP); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_CREATE); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_METADATA); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_MODIFY); \
	NFS_BITMAP_SET((A), NFS_FATTR_MOUNTED_ON_FILEID); \
	} while (0)

#define NFS4_PER_FS_ATTRIBUTES(A) \
	do { \
	/* required: */ \
	NFS_BITMAP_SET((A), NFS_FATTR_SUPPORTED_ATTRS); \
	NFS_BITMAP_SET((A), NFS_FATTR_FH_EXPIRE_TYPE); \
	NFS_BITMAP_SET((A), NFS_FATTR_LINK_SUPPORT); \
	NFS_BITMAP_SET((A), NFS_FATTR_SYMLINK_SUPPORT); \
	NFS_BITMAP_SET((A), NFS_FATTR_UNIQUE_HANDLES); \
	NFS_BITMAP_SET((A), NFS_FATTR_LEASE_TIME); \
	/* optional: */ \
	NFS_BITMAP_SET((A), NFS_FATTR_ACLSUPPORT); \
	NFS_BITMAP_SET((A), NFS_FATTR_CANSETTIME); \
	NFS_BITMAP_SET((A), NFS_FATTR_CASE_INSENSITIVE); \
	NFS_BITMAP_SET((A), NFS_FATTR_CASE_PRESERVING); \
	NFS_BITMAP_SET((A), NFS_FATTR_CHOWN_RESTRICTED); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILES_AVAIL); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILES_FREE); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILES_TOTAL); \
	NFS_BITMAP_SET((A), NFS_FATTR_FS_LOCATIONS); \
	NFS_BITMAP_SET((A), NFS_FATTR_HOMOGENEOUS); \
	NFS_BITMAP_SET((A), NFS_FATTR_MAXFILESIZE); \
	NFS_BITMAP_SET((A), NFS_FATTR_MAXNAME); \
	NFS_BITMAP_SET((A), NFS_FATTR_MAXREAD); \
	NFS_BITMAP_SET((A), NFS_FATTR_MAXWRITE); \
	NFS_BITMAP_SET((A), NFS_FATTR_NO_TRUNC); \
	NFS_BITMAP_SET((A), NFS_FATTR_SPACE_AVAIL); \
	NFS_BITMAP_SET((A), NFS_FATTR_SPACE_FREE); \
	NFS_BITMAP_SET((A), NFS_FATTR_SPACE_TOTAL); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_DELTA); \
	} while (0)

#define NFS4_DEFAULT_ATTRIBUTES(A) \
	do { \
	/* required: */ \
	NFS_BITMAP_SET((A), NFS_FATTR_SUPPORTED_ATTRS); \
	NFS_BITMAP_SET((A), NFS_FATTR_TYPE); \
	NFS_BITMAP_SET((A), NFS_FATTR_FH_EXPIRE_TYPE); \
	NFS_BITMAP_SET((A), NFS_FATTR_CHANGE); \
	NFS_BITMAP_SET((A), NFS_FATTR_SIZE); \
	NFS_BITMAP_SET((A), NFS_FATTR_LINK_SUPPORT); \
	NFS_BITMAP_SET((A), NFS_FATTR_SYMLINK_SUPPORT); \
	NFS_BITMAP_SET((A), NFS_FATTR_NAMED_ATTR); \
	NFS_BITMAP_SET((A), NFS_FATTR_FSID); \
	NFS_BITMAP_SET((A), NFS_FATTR_UNIQUE_HANDLES); \
	NFS_BITMAP_SET((A), NFS_FATTR_LEASE_TIME); \
	/* NFS_BITMAP_SET((A), NFS_FATTR_RDATTR_ERROR); */ \
	/* NFS_BITMAP_SET((A), NFS_FATTR_FILEHANDLE); */ \
	/* optional: */ \
	/* NFS_BITMAP_SET((A), NFS_FATTR_ACL); */ \
	NFS_BITMAP_SET((A), NFS_FATTR_ACLSUPPORT); \
	NFS_BITMAP_SET((A), NFS_FATTR_ARCHIVE); \
	/* NFS_BITMAP_SET((A), NFS_FATTR_CANSETTIME); */ \
	NFS_BITMAP_SET((A), NFS_FATTR_CASE_INSENSITIVE); \
	NFS_BITMAP_SET((A), NFS_FATTR_CASE_PRESERVING); \
	NFS_BITMAP_SET((A), NFS_FATTR_CHOWN_RESTRICTED); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILEID); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILES_AVAIL); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILES_FREE); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILES_TOTAL); \
	/* NFS_BITMAP_SET((A), NFS_FATTR_FS_LOCATIONS); */ \
	NFS_BITMAP_SET((A), NFS_FATTR_HIDDEN); \
	NFS_BITMAP_SET((A), NFS_FATTR_HOMOGENEOUS); \
	NFS_BITMAP_SET((A), NFS_FATTR_MAXFILESIZE); \
	NFS_BITMAP_SET((A), NFS_FATTR_MAXLINK); \
	NFS_BITMAP_SET((A), NFS_FATTR_MAXNAME); \
	NFS_BITMAP_SET((A), NFS_FATTR_MAXREAD); \
	NFS_BITMAP_SET((A), NFS_FATTR_MAXWRITE); \
	/* NFS_BITMAP_SET((A), NFS_FATTR_MIMETYPE); */ \
	NFS_BITMAP_SET((A), NFS_FATTR_MODE); \
	NFS_BITMAP_SET((A), NFS_FATTR_NO_TRUNC); \
	NFS_BITMAP_SET((A), NFS_FATTR_NUMLINKS); \
	NFS_BITMAP_SET((A), NFS_FATTR_OWNER); \
	NFS_BITMAP_SET((A), NFS_FATTR_OWNER_GROUP); \
	/* NFS_BITMAP_SET((A), NFS_FATTR_QUOTA_AVAIL_HARD); */ \
	/* NFS_BITMAP_SET((A), NFS_FATTR_QUOTA_AVAIL_SOFT); */ \
	/* NFS_BITMAP_SET((A), NFS_FATTR_QUOTA_USED); */ \
	NFS_BITMAP_SET((A), NFS_FATTR_RAWDEV); \
	NFS_BITMAP_SET((A), NFS_FATTR_SPACE_AVAIL); \
	NFS_BITMAP_SET((A), NFS_FATTR_SPACE_FREE); \
	NFS_BITMAP_SET((A), NFS_FATTR_SPACE_TOTAL); \
	NFS_BITMAP_SET((A), NFS_FATTR_SPACE_USED); \
	/* NFS_BITMAP_SET((A), NFS_FATTR_SYSTEM); */ \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_ACCESS); \
	/* NFS_BITMAP_SET((A), NFS_FATTR_TIME_ACCESS_SET); */ \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_BACKUP); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_CREATE); \
	/* NFS_BITMAP_SET((A), NFS_FATTR_TIME_DELTA); */ \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_METADATA); \
	NFS_BITMAP_SET((A), NFS_FATTR_TIME_MODIFY); \
	/* NFS_BITMAP_SET((A), NFS_FATTR_TIME_MODIFY_SET); */ \
	NFS_BITMAP_SET((A), NFS_FATTR_MOUNTED_ON_FILEID); \
	} while (0)

/* attributes requested when we want to do a "statfs" */
#define NFS4_STATFS_ATTRIBUTES(A) \
	do { \
	/* optional: */ \
	NFS_BITMAP_SET((A), NFS_FATTR_FILES_AVAIL); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILES_FREE); \
	NFS_BITMAP_SET((A), NFS_FATTR_FILES_TOTAL); \
	NFS_BITMAP_SET((A), NFS_FATTR_SPACE_AVAIL); \
	NFS_BITMAP_SET((A), NFS_FATTR_SPACE_FREE); \
	NFS_BITMAP_SET((A), NFS_FATTR_SPACE_TOTAL); \
	} while (0)

/*
 * NFS OPEN constants
 */
/* open type */
#define NFS_OPEN_NOCREATE                       0
#define NFS_OPEN_CREATE                         1
/* delegation space limit */
#define NFS_LIMIT_SIZE                          1
#define NFS_LIMIT_BLOCKS                        2
/* access/deny modes */
#define NFS_OPEN_SHARE_ACCESS_NONE              0x00000000
#define NFS_OPEN_SHARE_ACCESS_READ              0x00000001
#define NFS_OPEN_SHARE_ACCESS_WRITE             0x00000002
#define NFS_OPEN_SHARE_ACCESS_BOTH              0x00000003
#define NFS_OPEN_SHARE_DENY_NONE                0x00000000
#define NFS_OPEN_SHARE_DENY_READ                0x00000001
#define NFS_OPEN_SHARE_DENY_WRITE               0x00000002
#define NFS_OPEN_SHARE_DENY_BOTH                0x00000003
/* delegation types */
#define NFS_OPEN_DELEGATE_NONE                  0
#define NFS_OPEN_DELEGATE_READ                  1
#define NFS_OPEN_DELEGATE_WRITE                 2
/* delegation claim types */
#define NFS_CLAIM_NULL                          0
#define NFS_CLAIM_PREVIOUS                      1
#define NFS_CLAIM_DELEGATE_CUR                  2
#define NFS_CLAIM_DELEGATE_PREV                 3
/* open result flags */
#define NFS_OPEN_RESULT_CONFIRM                 0x00000002
#define NFS_OPEN_RESULT_LOCKTYPE_POSIX          0x00000004
/* NFS lock types */
#define NFS_LOCK_TYPE_READ                      1
#define NFS_LOCK_TYPE_WRITE                     2
#define NFS_LOCK_TYPE_READW                     3 /* "blocking" */
#define NFS_LOCK_TYPE_WRITEW                    4 /* "blocking" */

/* NFSv4 RPC procedures */
#define NFSPROC4_NULL                           0
#define NFSPROC4_COMPOUND                       1
#define NFSPROC4_CB_NULL                        0
#define NFSPROC4_CB_COMPOUND                    1

/* NFSv4 opcodes */
#define NFS_OP_ACCESS                           3
#define NFS_OP_CLOSE                            4
#define NFS_OP_COMMIT                           5
#define NFS_OP_CREATE                           6
#define NFS_OP_DELEGPURGE                       7
#define NFS_OP_DELEGRETURN                      8
#define NFS_OP_GETATTR                          9
#define NFS_OP_GETFH                            10
#define NFS_OP_LINK                             11
#define NFS_OP_LOCK                             12
#define NFS_OP_LOCKT                            13
#define NFS_OP_LOCKU                            14
#define NFS_OP_LOOKUP                           15
#define NFS_OP_LOOKUPP                          16
#define NFS_OP_NVERIFY                          17
#define NFS_OP_OPEN                             18
#define NFS_OP_OPENATTR                         19
#define NFS_OP_OPEN_CONFIRM                     20
#define NFS_OP_OPEN_DOWNGRADE                   21
#define NFS_OP_PUTFH                            22
#define NFS_OP_PUTPUBFH                         23
#define NFS_OP_PUTROOTFH                        24
#define NFS_OP_READ                             25
#define NFS_OP_READDIR                          26
#define NFS_OP_READLINK                         27
#define NFS_OP_REMOVE                           28
#define NFS_OP_RENAME                           29
#define NFS_OP_RENEW                            30
#define NFS_OP_RESTOREFH                        31
#define NFS_OP_SAVEFH                           32
#define NFS_OP_SECINFO                          33
#define NFS_OP_SETATTR                          34
#define NFS_OP_SETCLIENTID                      35
#define NFS_OP_SETCLIENTID_CONFIRM              36
#define NFS_OP_VERIFY                           37
#define NFS_OP_WRITE                            38
#define NFS_OP_RELEASE_LOCKOWNER                39
#define NFS_OP_ILLEGAL                          10044
/* NFSv4 callback opcodes */
#define NFS_OP_CB_GETATTR                       3
#define NFS_OP_CB_RECALL                        4
#define NFS_OP_CB_ILLEGAL                       10044

/* NFSv4 file handle type flags */
#define NFS_FH_PERSISTENT                       0x00000000
#define NFS_FH_NOEXPIRE_WITH_OPEN               0x00000001
#define NFS_FH_VOLATILE_ANY                     0x00000002
#define NFS_FH_VOL_MIGRATION                    0x00000004
#define NFS_FH_VOL_RENAME                       0x00000008

/*
 * NFSv4 ACL constants
 */
/* ACE support mask bits */
#define NFS_ACL_SUPPORT_ALLOW_ACL               0x00000001
#define NFS_ACL_SUPPORT_DENY_ACL                0x00000002
#define NFS_ACL_SUPPORT_AUDIT_ACL               0x00000004
#define NFS_ACL_SUPPORT_ALARM_ACL               0x00000008
/* ACE types */
#define NFS_ACE_ACCESS_ALLOWED_ACE_TYPE         0x00000000
#define NFS_ACE_ACCESS_DENIED_ACE_TYPE          0x00000001
#define NFS_ACE_SYSTEM_AUDIT_ACE_TYPE           0x00000002
#define NFS_ACE_SYSTEM_ALARM_ACE_TYPE           0x00000003
/* ACE flags */
#define NFS_ACE_FILE_INHERIT_ACE                0x00000001
#define NFS_ACE_DIRECTORY_INHERIT_ACE           0x00000002
#define NFS_ACE_NO_PROPAGATE_INHERIT_ACE        0x00000004
#define NFS_ACE_INHERIT_ONLY_ACE                0x00000008
#define NFS_ACE_SUCCESSFUL_ACCESS_ACE_FLAG      0x00000010
#define NFS_ACE_FAILED_ACCESS_ACE_FLAG          0x00000020
#define NFS_ACE_IDENTIFIER_GROUP                0x00000040
#define NFS_ACE_INHERITED_ACE                   0x00000080
/* ACE mask flags */
#define NFS_ACE_READ_DATA                       0x00000001
#define NFS_ACE_LIST_DIRECTORY                  0x00000001
#define NFS_ACE_WRITE_DATA                      0x00000002
#define NFS_ACE_ADD_FILE                        0x00000002
#define NFS_ACE_APPEND_DATA                     0x00000004
#define NFS_ACE_ADD_SUBDIRECTORY                0x00000004
#define NFS_ACE_READ_NAMED_ATTRS                0x00000008
#define NFS_ACE_WRITE_NAMED_ATTRS               0x00000010
#define NFS_ACE_EXECUTE                         0x00000020
#define NFS_ACE_DELETE_CHILD                    0x00000040
#define NFS_ACE_READ_ATTRIBUTES                 0x00000080
#define NFS_ACE_WRITE_ATTRIBUTES                0x00000100
#define NFS_ACE_DELETE                          0x00010000
#define NFS_ACE_READ_ACL                        0x00020000
#define NFS_ACE_WRITE_ACL                       0x00040000
#define NFS_ACE_WRITE_OWNER                     0x00080000
#define NFS_ACE_SYNCHRONIZE                     0x00100000
#define NFS_ACE_GENERIC_READ                    0x00120081
#define NFS_ACE_GENERIC_WRITE                   0x00160106
#define NFS_ACE_GENERIC_EXECUTE                 0x001200A0


/*
 * Quads are defined as arrays of 2 32-bit values to ensure dense packing
 * for the protocol and to facilitate xdr conversion.
 */
struct nfs_uquad {
	u_int32_t       nfsuquad[2];
};
typedef struct nfs_uquad        nfsuint64;

/*
 * Used to convert between two u_int32_ts and a u_quad_t.
 */
union nfs_quadconvert {
	u_int32_t               lval[2];
	u_quad_t        qval;
};
typedef union nfs_quadconvert   nfsquad_t;

/*
 * special data/attribute associated with NFBLK/NFCHR
 */
struct nfs_specdata {
	uint32_t specdata1;     /* major device number */
	uint32_t specdata2;     /* minor device number */
};
typedef struct nfs_specdata nfs_specdata;

/*
 * an "fsid" large enough to hold an NFSv4 fsid.
 */
struct nfs_fsid {
	uint64_t major;
	uint64_t minor;
};
typedef struct nfs_fsid nfs_fsid;

/*
 * NFSv4 stateid structure
 */
struct nfs_stateid {
	uint32_t        seqid;
	uint32_t        other[3];
};
typedef struct nfs_stateid nfs_stateid;

#endif /* __APPLE_API_PRIVATE */
#endif /* _NFS_NFSPROTO_H_ */

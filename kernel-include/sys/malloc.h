/*
 * Copyright (c) 2000-2011 Apple Inc. All rights reserved.
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
/* Copyright (c) 1998, 1999 Apple Computer, Inc. All Rights Reserved */
/* Copyright (c) 1995, 1997 Apple Computer, Inc. All Rights Reserved */
/*
 * Copyright (c) 1987, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	@(#)malloc.h	8.5 (Berkeley) 5/3/95
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2005 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

#ifndef _SYS_MALLOC_H_
#define	_SYS_MALLOC_H_

#include <sys/appleapiopts.h>


#ifdef KERNEL
/*
 * flags to malloc
 */
#define	M_WAITOK	0x0000
#define	M_NOWAIT	0x0001
#define M_ZERO          0x0004          /* bzero the allocation */


#ifdef BSD_KERNEL_PRIVATE

#define KMEMSTATS 0

/*
 * Types of memory to be allocated (not all are used by us)
 */
#define	M_FREE		0	/* should be on free list */
#define	M_MBUF		1	/* mbuf */
#define	M_DEVBUF	2	/* device driver memory */
#define	M_SOCKET	3	/* socket structure */
#define	M_PCB		4	/* protocol control block */
#define	M_RTABLE	5	/* routing tables */
#define	M_HTABLE	6	/* IMP host tables */
#define	M_FTABLE	7	/* fragment reassembly header */
#define	M_ZOMBIE	8	/* zombie proc status */
#define	M_IFADDR	9	/* interface address */
#define	M_SOOPTS	10	/* socket options */
#define	M_SONAME	11	/* socket name */
#define	M_NAMEI		12	/* namei path name buffer */
#define	M_GPROF		13	/* kernel profiling buffer */
#define	M_IOCTLOPS	14	/* ioctl data buffer */
#define	M_MAPMEM	15	/* mapped memory descriptors */
#define	M_CRED		16	/* credentials */
#define	M_PGRP		17	/* process group header */
#define	M_SESSION	18	/* session header */
#define	M_IOV32		19	/* large iov's for 32 bit process */
#define	M_MOUNT		20	/* vfs mount struct */
#define	M_FHANDLE	21	/* network file handle */
#define	M_NFSREQ	22	/* NFS request header */
#define	M_NFSMNT	23	/* NFS mount structure */
#define	M_NFSNODE	24	/* NFS vnode private part */
#define	M_VNODE		25	/* Dynamically allocated vnodes */
#define	M_CACHE		26	/* Dynamically allocated cache entries */
#define	M_DQUOT		27	/* UFS quota entries */
#define	M_UFSMNT	28	/* UFS mount structure */
#define	M_SHM		29	/* SVID compatible shared memory segments */
#define	M_PLIMIT	30	/* plimit  structures */
#define	M_SIGACTS	31	/* sigacts structures */
#define	M_VMOBJ		32	/* VM object structure */
#define	M_VMOBJHASH	33	/* VM object hash structure */
#define	M_VMPMAP	34	/* VM pmap */
#define	M_VMPVENT	35	/* VM phys-virt mapping entry */
#define	M_VMPAGER	36	/* XXX: VM pager struct */
#define	M_VMPGDATA	37	/* XXX: VM pager private data */
#define	M_FILEPROC	38	/* Open file structure */
#define	M_FILEDESC	39	/* Open file descriptor table */
#define	M_LOCKF		40	/* Byte-range locking structures */
#define	M_PROC		41	/* Proc structures */
#define	M_PSTATS	42	/* pstats  proc sub-structures */
#define	M_SEGMENT	43	/* Segment for LFS */
#define	M_LFSNODE	44	/* LFS vnode private part */
#define	M_FFSNODE	45	/* FFS vnode private part */
#define	M_MFSNODE	46	/* MFS vnode private part */
#define	M_NQLEASE	47	/* XXX: Nqnfs lease */
#define	M_NQMHOST	48	/* XXX: Nqnfs host address table */
#define	M_NETADDR	49	/* Export host address structure */
#define	M_NFSSVC	50	/* NFS server structure */
#define	M_NFSUID	51	/* XXX: NFS uid mapping structure */
#define	M_NFSD		52	/* NFS server daemon structure */
#define	M_IPMOPTS	53	/* internet multicast options */
#define	M_IPMADDR	54	/* internet multicast address */
#define	M_IFMADDR	55	/* link-level multicast address */
#define	M_MRTABLE	56	/* multicast routing tables */
#define	M_ISOFSMNT	57	/* ISOFS mount structure */
#define	M_ISOFSNODE	58	/* ISOFS vnode private part */
#define	M_NFSRVDESC	59	/* NFS server socket descriptor */
#define	M_NFSDIROFF	60	/* NFS directory offset data */
#define	M_NFSBIGFH	61	/* NFS version 3 file handle */
#define	M_MSDOSFSMNT	62	/* MSDOS FS mount structure */
#define	M_MSDOSFSFAT	63	/* MSDOS FS fat table */
#define	M_MSDOSFSNODE	64	/* MSDOS FS vnode private part */
#define	M_TTYS		65	/* allocated tty structures */
#define	M_EXEC		66	/* argument lists & other mem used by exec */
#define	M_MISCFSMNT	67	/* miscfs mount structures */
#define	M_MISCFSNODE	68	/* miscfs vnode private part */
#define	M_ADOSFSMNT	69	/* adosfs mount structures */
#define	M_ADOSFSNODE	70	/* adosfs vnode private part */
#define	M_ANODE		71	/* adosfs anode structures and tables. */
#define	M_BUFHDR	72	/* File buffer cache headers */
#define	M_OFILETABL	73	/* Open file descriptor table */
#define	M_MCLUST	74	/* mbuf cluster buffers */
#define	M_HFSMNT	75	/* HFS mount structure */
#define	M_HFSNODE	76	/* HFS catalog node */
#define	M_HFSFORK	77	/* HFS file fork */
#define M_ZFSMNT	78	/* ZFS mount data */
#define	M_ZFSNODE	79	/* ZFS inode */
#define	M_TEMP		80	/* misc temporary data buffers */
#define	M_SECA		81	/* security associations, key management */
#define M_DEVFS		82
#define M_IPFW		83	/* IP Forwarding/NAT */
#define M_UDFNODE	84	/* UDF inodes */
#define M_UDFMNT	85	/* UDF mount structures */
#define M_IP6NDP	86	/* IPv6 Neighbour Discovery*/
#define M_IP6OPT	87	/* IPv6 options management */
#define M_IP6MISC	88	/* IPv6 misc. memory */
#define M_TSEGQ		89	/* TCP segment queue entry, unused */
#define M_IGMP		90
#define M_JNL_JNL   91  /* Journaling: "struct journal" */
#define M_JNL_TR    92  /* Journaling: "struct transaction" */ 
#define	M_SPECINFO	93	/* special file node */
#define M_KQUEUE	94	/* kqueue */
#define	M_HFSDIRHINT	95	/* HFS directory hint */
#define M_CLRDAHEAD	96	/* storage for cluster read-ahead state */
#define M_CLWRBEHIND	97	/* storage for cluster write-behind state */
#define	M_IOV64		98	/* large iov's for 64 bit process */
#define M_FILEGLOB	99	/* fileglobal */
#define M_KAUTH		100	/* kauth subsystem */
#define M_DUMMYNET	101	/* dummynet */
#if CONFIG_VFS_FUNNEL
#define M_UNSAFEFS	102	/* storage for vnode lock state for unsafe FS */
#endif /* CONFIG_VFS_FUNNEL */
#define M_MACPIPELABEL	103	/* MAC pipe labels */
#define M_MACTEMP	104	/* MAC framework */
#define M_SBUF		105	/* string buffers */
#define M_EXTATTR	106	/* extended attribute */
#define M_LCTX		107	/* process login context */
/* M_TRAFFIC_MGT 108 */
#if HFS_COMPRESSION
#define M_DECMPFS_CNODE	109	/* decmpfs cnode structures */
#endif /* HFS_COMPRESSION */
#define M_INMFILTER	110	/* IPv4 multicast PCB-layer source filter */
#define M_IPMSOURCE	111	/* IPv4 multicast IGMP-layer source filter */
#define	M_IN6MFILTER	112	/* IPv6 multicast PCB-layer source filter */
#define	M_IP6MOPTS	113	/* IPv6 multicast options */
#define	M_IP6MSOURCE	114	/* IPv6 multicast MLD-layer source filter */

#define	M_LAST		115	/* Must be last type + 1 */

#else /* BSD_KERNEL_PRIVATE */

#define	M_RTABLE	5	/* routing tables */
#define	M_IFADDR	9	/* interface address (IOFireWireIP)*/
#define	M_LOCKF		40	/* Byte-range locking structures (msdos) */ 
#define	M_TEMP		80	/* misc temporary data buffers */
#define	M_HFSMNT	75	/* HFS mount structure (afpfs) */
#define M_KAUTH		100	/* kauth subsystem (smb) */
#define	M_SONAME	11	/* socket name (smb) */
#define	M_PCB		4	/* protocol control block (smb) */
#define M_UDFNODE	84	/* UDF inodes (udf)*/
#define M_UDFMNT	85	/* UDF mount structures (udf)*/

#endif /* BSD_KERNEL_PRIVATE */

#ifdef BSD_KERNEL_PRIVATE

#if KMEMSTATS
struct kmemstats {
	long	ks_inuse;	/* # of packets of this type currently
				 * in use */
	long	ks_calls;	/* total packets of this type ever allocated */
	long 	ks_memuse;	/* total memory held in bytes */
	u_short	ks_limblocks;	/* number of times blocked for hitting limit */
	u_short	ks_mapblocks;	/* number of times blocked for kernel map */
	long	ks_maxused;	/* maximum number ever used */
	long	ks_limit;	/* most that are allowed to exist */
	long	ks_size;	/* sizes of this thing that are allocated */
	long	ks_spare;
};

extern struct kmemstats kmemstats[];
#endif /* KMEMSTATS */

#endif /* BSD_KERNEL_PRIVATE */

/*
 * The malloc/free primatives used
 * by the BSD kernel code.
 */
#define	MALLOC(space, cast, size, type, flags) \
	(space) = (cast)_MALLOC(size, type, flags)

#define FREE(addr, type) \
	_FREE((void *)addr, type)

#define	REALLOC(space, cast, addr, size, type, flags) \
	(space) = (cast)_REALLOC(addr, size, type, flags)

#define MALLOC_ZONE(space, cast, size, type, flags) \
	(space) = (cast)_MALLOC_ZONE(size, type, flags)

#define FREE_ZONE(addr, size, type) \
	_FREE_ZONE((void *)addr, size, type)

extern void	*_MALLOC(
			size_t		size,
			int		type,
			int		flags);

extern void	_FREE(
			void		*addr,
			int		type);

extern void	*_REALLOC(
			void		*addr,
			size_t		size,
			int		type,
			int		flags);

extern void	*_MALLOC_ZONE(
			size_t		size,
			int		type,
			int		flags);

extern void	_FREE_ZONE(
			void		*elem,
			size_t		size,
			int		type);

#endif	/* KERNEL */

#endif	/* _SYS_MALLOC_H_ */

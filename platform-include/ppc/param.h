/*
 * Copyright (c) 2000-2004 Apple Computer, Inc. All rights reserved.
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
/* Copyright (c) 1993,1995 NeXT Computer, Inc. All Rights Reserved */

#ifndef	_PPC_PARAM_H_
#define	_PPC_PARAM_H_

#include <ppc/_param.h>

/*
 * Round p (pointer or byte index) up to a correctly-aligned value for all
 * data types (int, long, ...).   The result is unsigned int and must be
 * cast to any desired pointer type.
 */
#define	ALIGNBYTES	__DARWIN_ALIGNBYTES
#define	ALIGN(p)	__DARWIN_ALIGN(p)

#define	NBPG		4096		/* bytes/page */
#define	PGOFSET		(NBPG-1)	/* byte offset into page */
#define	PGSHIFT		12		/* LOG2(NBPG) */

#define NBSEG		0x40000000	/* bytes/segment (quadrant) */
#define	SEGOFSET	(NBSEG-1)	/* byte offset into segment */
#define	SEGSHIFT	30		/* LOG2(NBSEG) */

#define	DEV_BSIZE	512
#define	DEV_BSHIFT	9		/* log2(DEV_BSIZE) */
#define BLKDEV_IOSIZE	2048
#define	MAXPHYS		(128 * 1024)	/* max raw I/O transfer size */

#define	STACK_GROWTH_UP	0		/* stack grows to lower addresses */

#define	CLSIZE		1
#define	CLSIZELOG2	0

#define STACKSIZE 4			/* pages in kernel stack */
#define	UPAGES	0	/* total pages in u-area */
					/* red zone is beyond this */

/*
 * Constants related to network buffer management.
 * MCLBYTES must be no larger than CLBYTES (the software page size), and,
 * on machines that exchange pages of input or output buffers with mbuf
 * clusters (MAPPED_MBUFS), MCLBYTES must also be an integral multiple
 * of the hardware page size.
 */
#define	MSIZE		256		/* size of an mbuf */
#define	MCLBYTES	2048		/* large enough for ether MTU */
#define	MCLSHIFT	11
#define	MCLOFSET	(MCLBYTES - 1)
#ifndef NMBCLUSTERS
#if GATEWAY
#define	NMBCLUSTERS	((1024 * 1024) / MCLBYTES)	/* cl map size: 1MB */
#else
#define	NMBCLUSTERS	((1024 * 1024) / MCLBYTES)	
		/* cl map size was  0.5MB when MSIZE was 128, now it's 1MB*/
#endif
#endif

/* pages ("clicks") (NBPG bytes) to disk blocks */
#define	ctod(x)	((x)<<(PGSHIFT-DEV_BSHIFT))
#define	dtoc(x)	((x)>>(PGSHIFT-DEV_BSHIFT))
#define	dtob(x)	((x)<<DEV_BSHIFT)

/* pages to bytes */
#define	ctob(x)	((x)<<PGSHIFT)

/* bytes to pages */
#define	btoc(x)	(((unsigned)(x)+(PGOFSET))>>PGSHIFT)
#ifdef __APPLE__
#define  btodb(bytes, devBlockSize)         \
        ((unsigned)(bytes) / devBlockSize)
#define  dbtob(db, devBlockSize)            \
             ((unsigned)(db) * devBlockSize)
#else
#define	btodb(bytes)	 		/* calculates (bytes / DEV_BSIZE) */ \
	((unsigned)(bytes) >> DEV_BSHIFT)
#define	dbtob(db)			/* calculates (db * DEV_BSIZE) */ \
	((unsigned)(db) << DEV_BSHIFT)
#endif

/*
 * Map a ``block device block'' to a file system block.
 * This should be device dependent, and should use the bsize
 * field from the disk label.
 * For now though just use DEV_BSIZE.
 */
#define	bdbtofsb(bn)	((bn) / (BLKDEV_IOSIZE/DEV_BSIZE))

/* from machdep/ppc/proc_reg.h */
#ifdef __BIG_ENDIAN__
#define ENDIAN_MASK(val,size) (1 << (size-1 - val))
#else
#error code not ported to little endian targets yet
#endif /* __BIG_ENDIAN__ */

#ifndef MASK
#define MASK(PART)	ENDIAN_MASK(PART ## _BIT, 32)
#endif

#define	MSR_EE_BIT	16
#define	MSR_PR_BIT	17
#define USERMODE(msr) (msr & MASK(MSR_PR) ? TRUE : FALSE)
#define BASEPRI(msr) (msr & MASK(MSR_EE) ? TRUE : FALSE)
/* end of from proc_reg.h */

#if	defined(KERNEL) || defined(STANDALONE)
#define	DELAY(n) delay(n)
#else
#define	DELAY(n)	{ register int N = (n); while (--N > 0); }
#endif	/* defined(KERNEL) || defined(STANDALONE) */

#define	NPIDS		16	/* maximum number of PIDs per process */
#define	NIOPIDS		8	/* maximum number of IO space PIDs */

#endif	/* _PPC_PARAM_H_ */

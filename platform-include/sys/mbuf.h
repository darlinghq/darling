/*
 * Copyright (c) 1999-2014 Apple Inc. All rights reserved.
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
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*
 * Mach Operating System
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * Copyright (c) 1994 NeXT Computer, Inc. All rights reserved.
 *
 * Copyright (c) 1982, 1986, 1988 Regents of the University of California.
 * All rights reserved.
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
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
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
 *	@(#)mbuf.h	8.3 (Berkeley) 1/21/94
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2005 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

#ifndef	_SYS_MBUF_H_
#define	_SYS_MBUF_H_

#include <sys/cdefs.h>
#include <sys/appleapiopts.h>


/* mbuf types */
#define	MT_FREE		0	/* should be on free list */
#define	MT_DATA		1	/* dynamic (data) allocation */
#define	MT_HEADER	2	/* packet header */
#define	MT_SOCKET	3	/* socket structure */
#define	MT_PCB		4	/* protocol control block */
#define	MT_RTABLE	5	/* routing tables */
#define	MT_HTABLE	6	/* IMP host tables */
#define	MT_ATABLE	7	/* address resolution tables */
#define	MT_SONAME	8	/* socket name */
#define	MT_SOOPTS	10	/* socket options */
#define	MT_FTABLE	11	/* fragment reassembly header */
#define	MT_RIGHTS	12	/* access rights */
#define	MT_IFADDR	13	/* interface address */
#define	MT_CONTROL	14	/* extra-data protocol message */
#define	MT_OOBDATA	15	/* expedited data  */
#define	MT_TAG		16	/* volatile metadata associated to pkts */
#define	MT_MAX		32	/* enough? */


/*
 * Mbuf statistics (legacy).
 */
struct mbstat {
	u_int32_t	m_mbufs;	/* mbufs obtained from page pool */
	u_int32_t	m_clusters;	/* clusters obtained from page pool */
	u_int32_t	m_spare;	/* spare field */
	u_int32_t	m_clfree;	/* free clusters */
	u_int32_t	m_drops;	/* times failed to find space */
	u_int32_t	m_wait;		/* times waited for space */
	u_int32_t	m_drain;	/* times drained protocols for space */
	u_short		m_mtypes[256];	/* type specific mbuf allocations */
	u_int32_t	m_mcfail;	/* times m_copym failed */
	u_int32_t	m_mpfail;	/* times m_pullup failed */
	u_int32_t	m_msize;	/* length of an mbuf */
	u_int32_t	m_mclbytes;	/* length of an mbuf cluster */
	u_int32_t	m_minclsize;	/* min length of data to allocate a cluster */
	u_int32_t	m_mlen;		/* length of data in an mbuf */
	u_int32_t	m_mhlen;	/* length of data in a header mbuf */
	u_int32_t	m_bigclusters;	/* clusters obtained from page pool */
	u_int32_t	m_bigclfree;	/* free clusters */
	u_int32_t	m_bigmclbytes;	/* length of an mbuf cluster */
};

/* Compatibillity with 10.3 */
struct ombstat {
	u_int32_t	m_mbufs;	/* mbufs obtained from page pool */
	u_int32_t	m_clusters;	/* clusters obtained from page pool */
	u_int32_t	m_spare;	/* spare field */
	u_int32_t	m_clfree;	/* free clusters */
	u_int32_t	m_drops;	/* times failed to find space */
	u_int32_t	m_wait;		/* times waited for space */
	u_int32_t	m_drain;	/* times drained protocols for space */
	u_short		m_mtypes[256];	/* type specific mbuf allocations */
	u_int32_t	m_mcfail;	/* times m_copym failed */
	u_int32_t	m_mpfail;	/* times m_pullup failed */
	u_int32_t	m_msize;	/* length of an mbuf */
	u_int32_t	m_mclbytes;	/* length of an mbuf cluster */
	u_int32_t	m_minclsize;	/* min length of data to allocate a cluster */
	u_int32_t	m_mlen;		/* length of data in an mbuf */
	u_int32_t	m_mhlen;	/* length of data in a header mbuf */
};

/*
 * mbuf class statistics.
 */
#define	MAX_MBUF_CNAME	15


typedef struct mb_class_stat {
	char		mbcl_cname[MAX_MBUF_CNAME + 1]; /* class name */
	u_int32_t	mbcl_size;	/* buffer size */
	u_int32_t	mbcl_total;	/* # of buffers created */
	u_int32_t	mbcl_active;	/* # of active buffers */
	u_int32_t	mbcl_infree;	/* # of available buffers */
	u_int32_t	mbcl_slab_cnt;	/* # of available slabs */
#if defined(KERNEL) || defined(__LP64__)
	u_int32_t	mbcl_pad;	/* padding */
#endif /* KERNEL || __LP64__ */
	u_int64_t	mbcl_alloc_cnt;	/* # of times alloc is called */
	u_int64_t	mbcl_free_cnt;	/* # of times free is called */
	u_int64_t	mbcl_notified;	/* # of notified wakeups */
	u_int64_t	mbcl_purge_cnt;	/* # of purges so far */
	u_int64_t	mbcl_fail_cnt;	/* # of allocation failures */
	u_int32_t	mbcl_ctotal;	/* total only for this class */
	u_int32_t	mbcl_release_cnt; /* amount of memory returned */
	/*
	 * Cache layer statistics
	 */
	u_int32_t	mbcl_mc_state;	/* cache state (see below) */
	u_int32_t	mbcl_mc_cached;	/* # of cached buffers */
	u_int32_t	mbcl_mc_waiter_cnt;  /* # waiters on the cache */
	u_int32_t	mbcl_mc_wretry_cnt;  /* # of wait retries */
	u_int32_t	mbcl_mc_nwretry_cnt; /* # of no-wait retry attempts */
	u_int32_t	mbcl_peak_reported; /* last usage peak reported */
	u_int32_t	mbcl_reserved[7];    /* for future use */
} mb_class_stat_t;

#define	MCS_DISABLED	0	/* cache is permanently disabled */
#define	MCS_ONLINE	1	/* cache is online */
#define	MCS_PURGING	2	/* cache is being purged */
#define	MCS_OFFLINE	3	/* cache is offline (resizing) */


typedef struct mb_stat {
	u_int32_t	mbs_cnt;	/* number of classes */
#if defined(KERNEL) || defined(__LP64__)
	u_int32_t	mbs_pad;	/* padding */
#endif /* KERNEL || __LP64__ */
	mb_class_stat_t	mbs_class[1];	/* class array */
} mb_stat_t;



#endif	/* !_SYS_MBUF_H_ */

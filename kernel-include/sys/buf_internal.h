/*
 * Copyright (c) 2000-2012 Apple Inc. All rights reserved.
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
 * Copyright (c) 1982, 1986, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *	@(#)buf.h	8.9 (Berkeley) 3/30/95
 */

#ifndef _SYS_BUF_INTERNAL_H_
#define	_SYS_BUF_INTERNAL_H_

#include <sys/appleapiopts.h>

#ifdef KERNEL
#include <sys/queue.h>
#include <sys/errno.h>
#include <sys/vm.h>
#include <sys/cdefs.h>
#include <sys/buf.h>
#include <sys/lock.h>

#define NOLIST ((struct buf *)0x87654321)

/*
 * Attributes of an I/O to be used by lower layers
 */
struct bufattr {
#if CONFIG_PROTECT
	struct cprotect *ba_cpentry;	/* address of cp_entry */
	uint64_t ba_cp_file_off;	/* rounded file offset. See buf_setcpoff() for more comments */
#endif
	uint64_t ba_flags;	/* flags. Some are only in-use on embedded devices */
};

/*
 * The buffer header describes an I/O operation in the kernel.
 */
struct buf {
	LIST_ENTRY(buf) b_hash;		/* Hash chain. */
	LIST_ENTRY(buf) b_vnbufs;	/* Buffer's associated vnode. */
	TAILQ_ENTRY(buf) b_freelist;	/* Free list position if not active. */
	int	b_timestamp;		/* timestamp for queuing operation */
	int	b_whichq;		/* the free list the buffer belongs to */
	volatile uint32_t	b_flags;	/* B_* flags. */
	volatile uint32_t	b_lflags;	/* BL_BUSY | BL_WANTED flags... protected by buf_mtx */
	int	b_error;		/* errno value. */
	int	b_bufsize;		/* Allocated buffer size. */
	int	b_bcount;		/* Valid bytes in buffer. */
	int	b_resid;		/* Remaining I/O. */
	dev_t	b_dev;			/* Device associated with buffer. */
	uintptr_t	b_datap;	/* Memory, superblocks, indirect etc.*/
	daddr64_t	b_lblkno;	/* Logical block number. */
	daddr64_t	b_blkno;	/* Underlying physical block number. */
	void	(*b_iodone)(buf_t, void *);	/* Function to call upon completion. */
	vnode_t	b_vp;			/* File vnode for data, device vnode for metadata. */
	kauth_cred_t b_rcred;		/* Read credentials reference. */
	kauth_cred_t b_wcred;		/* Write credentials reference. */
	void *	b_upl;			/* Pointer to UPL */
	buf_t	b_real_bp;		/* used to track bp generated through cluster_bp */
	TAILQ_ENTRY(buf)	b_act;	/* Device driver queue when active */
	void *	b_drvdata;		/* Device driver private use */
	void *  b_fsprivate;		/* filesystem private use */
	void *	b_transaction;		/* journal private use */
	int	b_dirtyoff;		/* Offset in buffer of dirty region. */
	int	b_dirtyend;		/* Offset of end of dirty region. */
	int	b_validoff;		/* Offset in buffer of valid region. */
	int	b_validend;		/* Offset of end of valid region. */

	/* store extra information related to redundancy of data, such as
	 * which redundancy copy to use, etc
	 */
	uint32_t b_redundancy_flags;

	proc_t 	b_proc;			/* Associated proc; NULL if kernel. */
#ifdef BUF_MAKE_PRIVATE
	buf_t   b_data_store;
#endif
	struct bufattr b_attr;
#ifdef JOE_DEBUG
        void *	b_owner;
        int     b_tag;
        void *  b_lastbrelse;
        void *	b_stackbrelse[6];
        void *	b_stackgetblk[6];
#endif
};


/* cluster_io definitions for use with io bufs */
#define b_uploffset  b_bufsize
#define b_orig	     b_freelist.tqe_prev
#define b_shadow     b_freelist.tqe_next
#define	b_shadow_ref b_validoff
#ifdef BUF_MAKE_PRIVATE
#define b_data_ref   b_validend
#endif
#define b_trans_head b_freelist.tqe_prev
#define b_trans_next b_freelist.tqe_next
#define b_iostate    b_rcred
#define b_cliodone   b_wcred

/*
 * These flags are kept in b_lflags...
 * buf_mtxp must be held before examining/updating
 */
#define	BL_BUSY		0x00000001	/* I/O in progress. */
#define	BL_WANTED	0x00000002	/* Process wants this buffer. */
#define BL_IOBUF	0x00000004	/* buffer allocated via 'buf_alloc' */
#define BL_WANTDEALLOC	0x00000010	/* buffer should be put on empty list when clean */
#define BL_SHADOW	0x00000020
#define BL_EXTERNAL	0x00000040
#define BL_WAITSHADOW	0x00000080
#define BL_IOBUF_ALLOC	0x00000100
#define BL_WANTED_REF	0x00000200

/*
 * Parameters for buffer cache garbage collection 
 */
#define BUF_STALE_THRESHHOLD 	30	/* Collect if untouched in the last 30 seconds */
#define BUF_MAX_GC_BATCH_SIZE	64	/* Under a single grab of the lock */

/*
 * mask used by buf_flags... these are the readable external flags
 */
#define BUF_X_RDFLAGS (B_PHYS | B_RAW | B_LOCKED | B_ASYNC | B_READ | B_WRITE | B_PAGEIO |\
		       B_META | B_CLUSTER | B_DELWRI | B_FUA | B_PASSIVE | B_IOSTREAMING | B_THROTTLED_IO |\
		       B_ENCRYPTED_IO | B_STATICCONTENT)
/*
 * mask used by buf_clearflags/buf_setflags... these are the writable external flags
 */
#define BUF_X_WRFLAGS (B_PHYS | B_RAW | B_LOCKED | B_ASYNC | B_READ | B_WRITE | B_PAGEIO |\
		       B_NOCACHE | B_FUA | B_PASSIVE | B_IOSTREAMING)

/*
 * These flags are kept in b_flags... access is lockless
 * External flags are defined in buf.h and cannot overlap
 * the internal flags
 * 
 * these flags are internal... there definition may change
 */
#define	B_CACHE		0x00010000	/* getblk found us in the cache. */
#define	B_DONE		0x00020000	/* I/O completed. */
#define	B_INVAL		0x00040000	/* Does not contain valid info. */
#define	B_ERROR		0x00080000	/* I/O error occurred. */
#define	B_EINTR		0x00100000	/* I/O was interrupted */
#define	B_AGE		0x00200000	/* Move to age queue when I/O done. */
#define B_FILTER	0x00400000	/* call b_iodone from biodone as an in-line filter */
#define	B_CALL		0x00800000	/* Call b_iodone from biodone, assumes b_iodone consumes bp */
#define B_EOT		0x01000000	/* last buffer in a transaction list created by cluster_io */
#define	B_WASDIRTY	0x02000000	/* page was found dirty in the VM cache */
#define	B_HDRALLOC	0x04000000	/* zone allocated buffer header */
#define	B_ZALLOC	0x08000000	/* b_datap is zalloc()ed */
/*
 * private flags used by the journal layer
 */
#define	B_NORELSE	0x10000000	/* don't brelse() in bwrite() */
/*
 * private flags used by by the cluster layer
 */
#define B_TWANTED	0x20000000	/* but_t that is part of a cluster level transaction is wanted */
#define B_COMMIT_UPL    0x40000000	/* commit/abort the UPL on I/O success/failure */
#define B_TDONE		0x80000000	/* buf_t that is part of a cluster level transaction has completed */

/* Flags to low-level allocation routines. */
#define B_CLRBUF	0x01	/* Request allocated buffer be cleared. */
#define B_SYNC		0x02	/* Do all allocations synchronously. */
#define B_NOBUFF	0x04	/* Do not allocate struct buf */

/*
 * ba_flags (Buffer Attribute flags)
 * Some of these may be in-use only on embedded devices.
 */
#define BA_RAW_ENCRYPTED_IO     0x00000001
#define BA_THROTTLED_IO         0x00000002
#define BA_DELAYIDLESLEEP       0x00000004	/* Process is marked to delay idle sleep on disk IO */
#define BA_NOCACHE		0x00000008
#define BA_META			0x00000010


extern int niobuf_headers;		/* The number of IO buffer headers for cluster IO */
extern int nbuf_headers;		/* The number of buffer headers */
extern int max_nbuf_headers;		/* The max number of buffer headers */
extern int nbuf_hashelements;		/* The number of elements in bufhash */
extern struct buf *buf_headers;		/* The buffer headers. */


/*
 * Definitions for the buffer free lists.
 */
#define	BQUEUES		6		/* number of free buffer queues */

#define	BQ_LOCKED	0		/* super-blocks &c */
#define	BQ_LRU		1		/* lru, useful buffers */
#define	BQ_AGE		2		/* rubbish */
#define	BQ_EMPTY	3		/* buffer headers with no memory */
#define BQ_META		4		/* buffer containing metadata */
#define BQ_LAUNDRY	5		/* buffers that need cleaning */


__BEGIN_DECLS

buf_t	buf_create_shadow_priv(buf_t bp, boolean_t force_copy, uintptr_t external_storage, void (*iodone)(buf_t, void *), void *arg);

buf_t	alloc_io_buf(vnode_t, int);
void	free_io_buf(buf_t);

int	allocbuf(struct buf *, int);
void	bufinit(void) __attribute__((section("__TEXT, initcode")));

/*
 * Flags for buf_acquire
 */
#define BAC_NOWAIT		0x01	/* Don't wait if buffer is busy */
#define BAC_REMOVE		0x02	/* Remove from free list once buffer is acquired */
#define BAC_SKIP_NONLOCKED	0x04	/* Don't return LOCKED buffers */
#define BAC_SKIP_LOCKED		0x08	/* Only return LOCKED buffers */

void	buf_list_lock(void);
void	buf_list_unlock(void);

void	cluster_init(void) __attribute__((section("__TEXT, initcode")));
void	buf_drop(buf_t);
errno_t	buf_acquire(buf_t, int, int, int);

int	count_busy_buffers(void);
int	count_lock_queue(void);

int buf_flushdirtyblks_skipinfo (vnode_t, int, int, const char *);
void buf_wait_for_shadow_io (vnode_t, daddr64_t);

#ifdef BUF_MAKE_PRIVATE
errno_t	buf_make_private(buf_t bp);
#endif

#ifdef CONFIG_PROTECT
void buf_setcpaddr(buf_t, struct cprotect *);
void buf_setcpoff (buf_t, uint64_t);
#endif

__END_DECLS


/*
 *	Stats on usefulness of the buffer cache
 */
struct bufstats {
	long	bufs_incore;		/* found incore */
	long	bufs_busyincore;	/* found incore. was busy */
	long	bufs_vmhits;		/* not incore. found in VM */
	long	bufs_miss;			/* not incore. not in VM */
	long	bufs_sleeps;		/* buffer starvation */
	long	bufs_eblk;			/* Calls to geteblk */
	long	bufs_iobufmax;		/* Max. number of IO buffers used */
	long	bufs_iobufinuse;	/* number of IO buffers in use */
	long	bufs_iobufsleeps;	/* IO buffer starvation */
};

#endif /* KERNEL */
#endif /* !_SYS_BUF_H_ */

/*
 * Copyright (c) 2004-2005 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1996 John S. Dyson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice immediately at the beginning of the file, without modification,
 *    this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Absolutely no warranty of function or purpose is made by the author
 *    John S. Dyson.
 * 4. This work was done expressly for inclusion into FreeBSD.  Other use
 *    is allowed if this notation is included.
 * 5. Modifications may be freely made to this file if the above conditions
 *    are met.
 *
 * $FreeBSD: src/sys/sys/pipe.h,v 1.24 2003/08/13 20:01:38 alc Exp $
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2006 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

#ifndef _SYS_PIPE_H_
#define _SYS_PIPE_H_

#ifdef	KERNEL
#include <libkern/locks.h>
#endif
#include <sys/queue.h>			/* for TAILQ macros */
#include <sys/ev.h>
#include <sys/cdefs.h>

/*
 * Pipe buffer size, keep moderate in value, pipes take kva space.
 */
#ifndef PIPE_SIZE
#define PIPE_SIZE	16384
#endif

#define PIPE_KVAMAX	(1024 * 1024 * 16)

#ifndef BIG_PIPE_SIZE
#define BIG_PIPE_SIZE	(64*1024)
#endif

#ifndef SMALL_PIPE_SIZE
#define SMALL_PIPE_SIZE	PAGE_SIZE
#endif

/*
 * PIPE_MINDIRECT MUST be smaller than PIPE_SIZE and MUST be bigger
 * than PIPE_BUF.
 */
#ifndef PIPE_MINDIRECT
#define PIPE_MINDIRECT	8192
#endif

#define PIPENPAGES	(BIG_PIPE_SIZE / PAGE_SIZE + 1)

/*
 * Pipe buffer information.
 * Separate in, out, cnt are used to simplify calculations.
 * Buffered write is active when the buffer.cnt field is set.
 */
struct pipebuf {
	u_int	cnt;		/* number of chars currently in buffer */
	u_int	in;		/* in pointer */
	u_int	out;		/* out pointer */
	u_int	size;		/* size of buffer */
	caddr_t	buffer;		/* kva of buffer */
};


#ifdef PIPE_DIRECT
/*
 * Information to support direct transfers between processes for pipes.
 */
struct pipemapping {
	vm_offset_t	kva;		/* kernel virtual address */
	vm_size_t	cnt;		/* number of chars in buffer */
	vm_size_t	pos;		/* current position of transfer */
	int		npages;		/* number of pages */
	vm_page_t	ms[PIPENPAGES];	/* pages in source process */
};
#endif

/*
 * Bits in pipe_state.
 */
#define PIPE_ASYNC	0x004	/* Async? I/O. */
#define PIPE_WANTR	0x008	/* Reader wants some characters. */
#define PIPE_WANTW	0x010	/* Writer wants space to put characters. */
#define PIPE_WANT	0x020	/* Pipe is wanted to be run-down. */
#define PIPE_SEL	0x040	/* Pipe has a select active. */
#define PIPE_EOF	0x080	/* Pipe is in EOF condition. */
#define PIPE_LOCKFL	0x100	/* Process has exclusive access to pointers/data. */
#define PIPE_LWANT	0x200	/* Process wants exclusive access to pointers/data. */
#define PIPE_DIRECTW	0x400	/* Pipe direct write active. */
#define PIPE_DIRECTOK	0x800	/* Direct mode ok. */
#define PIPE_KNOTE	0x1000   /* Pipe has kernel events activated */
#define PIPE_DRAIN	0x2000  /* Waiting for I/O to drop for a close.  Treated like EOF;
					only separate for easier debugging. */
#define PIPE_WSELECT	0x4000  /* Some thread has done an FWRITE select on the pipe */
#define PIPE_DEAD	0x8000  /* Pipe is dead and needs garbage collection */

#ifdef	KERNEL

struct label;

/*
 * Per-pipe data structure.
 * Two of these are linked together to produce bi-directional pipes.
 */
struct pipe {
	struct	pipebuf pipe_buffer;	/* data storage */
#ifdef PIPE_DIRECT
	struct	pipemapping pipe_map;	/* pipe mapping for direct I/O */
#endif
	struct	selinfo pipe_sel;	/* for compat with select */
	pid_t	pipe_pgid;		/* information for async I/O */
	struct	pipe *pipe_peer;	/* link with other direction */
	u_int	pipe_state;		/* pipe status info */
	int	pipe_busy;		/* busy flag, mostly to handle rundown sanely */
	TAILQ_HEAD(,eventqelt) pipe_evlist;
	lck_mtx_t *pipe_mtxp;		/* shared mutex between both pipes */
	struct	timespec st_atimespec;	/* time of last access */
	struct	timespec st_mtimespec;	/* time of last data modification */
	struct	timespec st_ctimespec;	/* time of last status change */
	struct	label *pipe_label;	/* pipe MAC label - shared */
};

#define PIPE_MTX(pipe)		((pipe)->pipe_mtxp)

#define PIPE_LOCK(pipe)		lck_mtx_lock(PIPE_MTX(pipe))
#define PIPE_UNLOCK(pipe)	lck_mtx_unlock(PIPE_MTX(pipe))
#define PIPE_LOCK_ASSERT(pipe, type)  lck_mtx_assert(PIPE_MTX(pipe), (type))

__BEGIN_DECLS
void pipeinit(void);
extern int pipe_stat(struct pipe *, void *, int);
__END_DECLS

#endif	/* KERNEL */

#endif /* !_SYS_PIPE_H_ */

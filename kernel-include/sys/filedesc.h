/*
 * Copyright (c) 2000-2003 Apple Computer, Inc. All rights reserved.
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
/* Copyright (c) 1995, 1997 Apple Computer, Inc. All Rights Reserved */
/*
 * Copyright (c) 1990, 1993
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
 *	@(#)filedesc.h	8.1 (Berkeley) 6/2/93
 */

#ifndef _SYS_FILEDESC_H_
#define	_SYS_FILEDESC_H_

#include <sys/appleapiopts.h>

/*
 * This structure is used for the management of descriptors.  It may be
 * shared by multiple processes.
 *
 * A process is initially started out with NDFILE descriptors [XXXstored within
 * this structureXXX], selected to be enough for typical applications based on
 * the historical limit of 20 open files (and the usage of descriptors by
 * shells).  If these descriptors are exhausted, a larger descriptor table
 * may be allocated, up to a process' resource limit; [XXXthe internal arrays
 * are then unusedXXX].  The initial expansion is set to NDEXTENT; each time
 * it runs out, it is doubled until the resource limit is reached. NDEXTENT
 * should be selected to be the biggest multiple of OFILESIZE (see below)
 * that will fit in a power-of-two sized piece of memory.
 */
#define NDFILE		25		/* 125 bytes */
#define NDEXTENT	50		/* 250 bytes in 256-byte alloc. */ 

#ifdef BSD_KERNEL_PRIVATE

struct klist;

struct filedesc {
	struct	fileproc **fd_ofiles;	/* file structures for open files */
	char	*fd_ofileflags;		/* per-process open file flags */
	struct	vnode *fd_cdir;		/* current directory */
	struct	vnode *fd_rdir;		/* root directory */
	int	fd_nfiles;		/* number of open files allocated */
	int	fd_lastfile;		/* high-water mark of fd_ofiles */
	int	fd_freefile;		/* approx. next free file */
	u_short	fd_cmask;		/* mask for file creation */
	uint32_t	fd_refcnt;		/* reference count */

	int     fd_knlistsize;          /* size of knlist */
	struct  klist *fd_knlist;       /* list of attached knotes */
	u_long  fd_knhashmask;          /* size of knhash */
	struct  klist *fd_knhash;       /* hash table for attached knotes */
        int	fd_flags;
};

/*
 * definitions for fd_flags;
 */
#define	FD_CHROOT	0x01	/* process was chrooted... keep track even */
                                /* if we're force unmounted and unable to */
                                /* take a vnode_ref on fd_rdir during a fork */

/*
 * Per-process open flags.
 */
#define	UF_EXCLOSE 	0x01		/* auto-close on exec */
#define UF_RESERVED	0x04		/* open pending / in progress */
#define UF_CLOSING	0x08		/* close in progress */

#ifdef KERNEL
#define UF_RESVWAIT	0x10		/* close in progress */
#define	UF_INHERIT	0x20		/* "inherit-on-exec" */
#define UF_VALID_FLAGS	\
	(UF_EXCLOSE | UF_RESERVED | UF_CLOSING | UF_RESVWAIT | UF_INHERIT)
#endif /* KERNEL */

/*
 * Storage required per open file descriptor.
 */
#define OFILESIZE (sizeof(struct file *) + sizeof(char))

#ifdef KERNEL
/*
 * Kernel global variables and routines.
 */
extern int	dupfdopen(struct filedesc *fdp,
				int indx, int dfd, int mode, int error);
extern int	fdalloc(proc_t p, int want, int *result);
extern void	fdrelse(proc_t p, int fd);
extern int	fdavail(proc_t p, int n);
#define		fdfile(p, fd)					\
			(&(p)->p_fd->fd_ofiles[(fd)])
#define		fdflags(p, fd)					\
			(&(p)->p_fd->fd_ofileflags[(fd)])
extern int	falloc(proc_t p, struct fileproc **resultfp, int *resultfd, vfs_context_t ctx);
extern void	ffree(struct file *fp);

#ifdef __APPLE_API_PRIVATE
extern struct	filedesc *fdcopy(proc_t p, struct vnode *uth_cdir);
extern void	fdfree(proc_t p);
extern void	fdexec(proc_t p, short flags);
#endif /* __APPLE_API_PRIVATE */

#endif /* KERNEL */

#endif /* BSD_KERNEL_PRIVATE */

#endif /* !_SYS_FILEDESC_H_ */

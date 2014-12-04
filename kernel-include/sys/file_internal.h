/*
 * Copyright (c) 2000-2007 Apple Inc. All rights reserved.
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
 * Copyright (c) 1982, 1986, 1989, 1993
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
 *	@(#)file.h	8.3 (Berkeley) 1/9/95
 */

#ifndef _SYS_FILE_INTERNAL_H_
#define	_SYS_FILE_INTERNAL_H_

#include <sys/appleapiopts.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>

#ifdef KERNEL
#include <sys/errno.h>
#include <sys/queue.h>
#include <sys/cdefs.h>
#include <sys/lock.h>
#include <sys/file.h>

struct proc;
struct uio;
struct knote;
#ifdef __APPLE_API_UNSTABLE

struct file;


/*
 * Kernel descriptor table.
 * One entry for each open kernel vnode and socket.
 */
struct fileproc {
	unsigned int f_flags;
	int32_t f_iocount;
	struct fileglob * f_fglob;
	void *	f_waddr;
};

#define FILEPROC_NULL (struct fileproc *)0

#define FP_INCREATE 	0x0001
#define FP_INCLOSE 	0x0002
#define FP_INSELECT	0x0004
#define FP_INCHRREAD	0x0000	/* disable FP_INCHRREAD <rdar://6986929> */
#define FP_WRITTEN	0x0010
#define FP_CLOSING	0x0020
#define FP_WAITCLOSE	0x0040
#define FP_AIOISSUED	0x0080
#define FP_WAITEVENT	0x0100
#define FP_SELCONFLICT	0x0200	/* select conflict on an individual fp */

#define FP_VALID_FLAGS (FP_INCREATE | FP_INCLOSE | FP_INSELECT | FP_INCHRREAD | FP_WRITTEN | FP_CLOSING | FP_WAITCLOSE | FP_AIOISSUED | FP_WAITEVENT | FP_SELCONFLICT)


#ifndef _KAUTH_CRED_T
#define	_KAUTH_CRED_T
struct ucred;
typedef struct ucred *kauth_cred_t;
struct posix_cred;
typedef struct posix_cred *posix_cred_t;
#endif	/* !_KAUTH_CRED_T */

/* file types */
typedef enum {
	DTYPE_VNODE 	= 1,	/* file */
	DTYPE_SOCKET,		/* communications endpoint */
	DTYPE_PSXSHM,		/* POSIX Shared memory */
	DTYPE_PSXSEM,		/* POSIX Semaphores */
	DTYPE_KQUEUE,		/* kqueue */
	DTYPE_PIPE,		/* pipe */
	DTYPE_FSEVENTS		/* fsevents */
} file_type_t;

/* defines for fg_lflags */
#define FG_TERM 	0x01	/* the fileglob is terminating .. */
#define FG_INSMSGQ 	0x02	/* insert to msgqueue pending .. */
#define FG_WINSMSGQ	0x04 	/* wait for the fielglob is in msgque */
#define FG_RMMSGQ	0x08 	/* the fileglob is being removed from msgqueue */
#define FG_WRMMSGQ	0x10 	/* wait for the fileglob to  be removed from msgqueue */
#define FG_PORTMADE	0x20	/* a port was at some point created for this fileglob */
#define FG_NOSIGPIPE	0x40	/* don't deliver SIGPIPE with EPIPE return */

struct fileglob {
	LIST_ENTRY(fileglob) f_msglist;/* list of active files */
	int32_t	fg_flag;		/* see fcntl.h */
	file_type_t fg_type;		/* descriptor type */
	int32_t	fg_count;	/* reference count */
	int32_t	fg_msgcount;	/* references from message queue */
	kauth_cred_t fg_cred;	/* credentials associated with descriptor */
	struct	fileops {
		int	(*fo_read)	(struct fileproc *fp, struct uio *uio,
					 int flags, vfs_context_t ctx);
		int	(*fo_write)	(struct fileproc *fp, struct uio *uio,
					 int flags, vfs_context_t ctx);
#define	FOF_OFFSET	0x00000001	/* offset supplied to vn_write */
#define FOF_PCRED	0x00000002	/* cred from proc, not current thread */
		int	(*fo_ioctl)	(struct fileproc *fp, u_long com,
					 caddr_t data, vfs_context_t ctx);
		int	(*fo_select)	(struct fileproc *fp, int which,
					 void *wql, vfs_context_t ctx);
		int	(*fo_close)	(struct fileglob *fg, vfs_context_t ctx);
		int	(*fo_kqfilter)	(struct fileproc *fp, struct knote *kn,
					 vfs_context_t ctx);
		int	(*fo_drain)	(struct fileproc *fp, vfs_context_t ctx);
	} *fg_ops;
	off_t	fg_offset;
	void 	*fg_data;		/* vnode or socket or SHM or semaphore */
	lck_mtx_t fg_lock;
	int32_t fg_lflags;		/* file global flags */
#if CONFIG_MACF
	struct label *fg_label;  /* JMM - use the one in the cred? */
#endif
};

#ifdef __APPLE_API_PRIVATE
LIST_HEAD(fmsglist, fileglob);
extern struct fmsglist fmsghead;	/* head of list of open files */
extern int maxfiles;			/* kernel limit on number of open files */
extern int nfiles;			/* actual number of open files */
extern int maxfilesperproc;
#endif /* __APPLE_API_PRIVATE */


__BEGIN_DECLS
int fo_read(struct fileproc *fp, struct uio *uio, int flags, vfs_context_t ctx);
int fo_write(struct fileproc *fp, struct uio *uio, int flags,
		vfs_context_t ctx);
int fo_ioctl(struct fileproc *fp, u_long com, caddr_t data, vfs_context_t ctx);
int fo_select(struct fileproc *fp, int which, void *wql, vfs_context_t ctx);
int fo_close(struct fileglob *fg, vfs_context_t ctx);
int fo_kqfilter(struct fileproc *fp, struct knote *kn, vfs_context_t ctx);
void fileproc_drain(proc_t, struct fileproc *);
int fp_drop(struct proc *p, int fd, struct fileproc *fp, int locked);
int fp_drop_written(proc_t p, int fd, struct fileproc *fp);
int fp_drop_event(proc_t p, int fd, struct fileproc *fp);
int fp_free(struct proc * p, int fd, struct fileproc * fp);
struct kqueue;
int fp_getfkq(struct proc *p, int fd, struct fileproc **resultfp, struct kqueue  **resultkq);
struct psemnode;
int fp_getfpsem(struct proc *p, int fd, struct fileproc **resultfp, struct psemnode  **resultpsem);
struct pshmnode;
int fp_getfpshm(struct proc *p, int fd, struct fileproc **resultfp, struct pshmnode  **resultpshm);
struct pipe;
int fp_getfpipe(struct proc *p, int fd, struct fileproc **resultfp, struct pipe  **resultpipe);
struct atalk;
int fp_getfatalk(struct proc *p, int fd, struct fileproc **resultfp, struct atalk  **resultatalk);
struct vnode;
int fp_getfvp(struct proc *p, int fd, struct fileproc **resultfp, struct vnode  **resultvp);
int fp_getfvpandvid(struct proc *p, int fd, struct fileproc **resultfp, struct vnode  **resultvp, uint32_t * vidp);
struct socket;
int fp_getfsock(struct proc *p, int fd, struct fileproc **resultfp, struct socket  **results);
int fp_lookup(struct proc *p, int fd, struct fileproc **resultfp, int locked);
int closef_locked(struct fileproc *fp, struct fileglob *fg, struct proc *p);
void fg_insertuipc(struct fileglob * fg);
void fg_removeuipc(struct fileglob * fg);
void unp_gc_wait(void);
void procfdtbl_reservefd(struct proc * p, int fd);
void procfdtbl_markclosefd(struct proc * p, int fd);
void procfdtbl_releasefd(struct proc * p, int fd, struct fileproc * fp);
void procfdtbl_waitfd(struct proc * p, int fd);
void procfdtbl_clearfd(struct proc * p, int fd);
boolean_t filetype_issendable(file_type_t type);
extern int fdgetf_noref(proc_t, int, struct fileproc **);
__END_DECLS

#endif /* __APPLE_API_UNSTABLE */

#endif /* KERNEL */

#endif /* !_SYS_FILE_INTERNAL_H_ */

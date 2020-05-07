/*
 * Copyright (c) 2000-2016 Apple Inc. All rights reserved.
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
#define _SYS_FILE_INTERNAL_H_

#include <sys/appleapiopts.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>

#ifdef KERNEL
#include <sys/errno.h>
#include <sys/queue.h>
#include <sys/cdefs.h>
#include <sys/lock.h>
#include <sys/file.h>
#include <sys/filedesc.h>
#include <sys/guarded.h>
#include <os/refcnt.h>

struct proc;
struct uio;
struct knote;
struct kevent_qos_s;

#ifdef __APPLE_API_UNSTABLE

struct file;

__options_decl(fileproc_vflags_t, unsigned int, {
	FPV_NONE        = 0,
	FPV_DRAIN       = 0x01,
});

/*
 * Kernel descriptor table.
 * One entry for each open kernel vnode and socket.
 */
struct fileproc {
	unsigned int f_flags;
	_Atomic fileproc_vflags_t f_vflags;
	os_refcnt_t f_iocount;
	struct fileglob * f_fglob;
	void *f_wset;
};

#define FILEPROC_NULL (struct fileproc *)0

#define FP_INCREATE     0x0001
#define FP_INCLOSE      0x0002
#define FP_INSELECT     0x0004
#define FP_UNUSED       0x0008  /* unused (was FP_INCHRREAD) */
#define FP_WRITTEN      0x0010
#define FP_CLOSING      0x0020
#define FP_WAITCLOSE    0x0040
#define FP_AIOISSUED    0x0080
#define FP_WAITEVENT    0x0100
#define FP_SELCONFLICT  0x0200  /* select conflict on an individual fp */

/* squeeze a "type" value into the upper flag bits */

#define _FP_TYPESHIFT   24
#define FP_TYPEMASK     (0x7 << _FP_TYPESHIFT)  /* 8 "types" of fileproc */

#define FILEPROC_TYPE(fp)       ((fp)->f_flags & FP_TYPEMASK)

#define FP_ISGUARDED(fp, attribs)  \
	        ((FILEPROC_TYPE(fp) == FTYPE_GUARDED) ? fp_isguarded(fp, attribs) : 0)

typedef enum {
	FTYPE_SIMPLE    = 0,
	FTYPE_GUARDED   = (1 << _FP_TYPESHIFT)
} fileproc_type_t;

#define FP_VALID_FLAGS  (FP_INCREATE | FP_INCLOSE | FP_INSELECT |\
	        FP_WRITTEN | FP_CLOSING | FP_WAITCLOSE |\
	        FP_AIOISSUED | FP_WAITEVENT | FP_SELCONFLICT | _FP_TYPEMASK)

#ifndef _KAUTH_CRED_T
#define _KAUTH_CRED_T
struct ucred;
typedef struct ucred *kauth_cred_t;
struct posix_cred;
typedef struct posix_cred *posix_cred_t;
#endif  /* !_KAUTH_CRED_T */

/* file types */
typedef enum {
	DTYPE_VNODE     = 1,    /* file */
	DTYPE_SOCKET,           /* communications endpoint */
	DTYPE_PSXSHM,           /* POSIX Shared memory */
	DTYPE_PSXSEM,           /* POSIX Semaphores */
	DTYPE_KQUEUE,           /* kqueue */
	DTYPE_PIPE,             /* pipe */
	DTYPE_FSEVENTS,         /* fsevents */
	DTYPE_ATALK,            /* (obsolete) */
	DTYPE_NETPOLICY,        /* networking policy */
} file_type_t;

/* defines for fg_lflags */
#define FG_TERM         0x01    /* the fileglob is terminating .. */
#define FG_INSMSGQ      0x02    /* insert to msgqueue pending .. */
#define FG_WINSMSGQ     0x04    /* wait for the fielglob is in msgque */
#define FG_RMMSGQ       0x08    /* the fileglob is being removed from msgqueue */
#define FG_WRMMSGQ      0x10    /* wait for the fileglob to  be removed from msgqueue */
#define FG_PORTMADE     0x20    /* a port was at some point created for this fileglob */
#define FG_NOSIGPIPE    0x40    /* don't deliver SIGPIPE with EPIPE return */
#define FG_OFF_LOCKED   0x80    /* Used as a mutex for offset changes (for vnodes) */
#define FG_OFF_LOCKWANT 0x100   /* Somebody's wating for the lock */
#define FG_CONFINED     0x200   /* fileglob confined to process, immutably */
#define FG_HAS_OFDLOCK  0x400   /* Has or has had an OFD lock */

struct fileops {
	file_type_t     fo_type;        /* descriptor type */
	int (*fo_read)      (struct fileproc *fp, struct uio *uio,
	    int flags, vfs_context_t ctx);
	int (*fo_write)     (struct fileproc *fp, struct uio *uio,
	    int flags, vfs_context_t ctx);
#define FOF_OFFSET      0x00000001      /* offset supplied to vn_write */
#define FOF_PCRED       0x00000002      /* cred from proc, not current thread */
	int (*fo_ioctl)(struct fileproc *fp, u_long com,
	    caddr_t data, vfs_context_t ctx);
	int (*fo_select)    (struct fileproc *fp, int which,
	    void *wql, vfs_context_t ctx);
	int (*fo_close)     (struct fileglob *fg, vfs_context_t ctx);
	int (*fo_kqfilter)  (struct fileproc *fp, struct knote *, struct kevent_qos_s *);
	int (*fo_drain)     (struct fileproc *fp, vfs_context_t ctx);
};

struct fileglob {
	LIST_ENTRY(fileglob) f_msglist;/* list of active files */
	int32_t fg_flag;        /* see fcntl.h */
	int32_t fg_count;       /* reference count */
	int32_t fg_msgcount;    /* references from message queue */
	int32_t fg_lflags;      /* file global flags */
	kauth_cred_t fg_cred;   /* credentials associated with descriptor */
	const struct fileops *fg_ops;
	off_t   fg_offset;
	void   *fg_data;        /* vnode or socket or SHM or semaphore */
	void   *fg_vn_data;     /* Per fd vnode data, used for directories */
	lck_mtx_t fg_lock;
#if CONFIG_MACF
	struct label *fg_label; /* JMM - use the one in the cred? */
#endif
};

#ifdef __APPLE_API_PRIVATE
LIST_HEAD(fmsglist, fileglob);
extern struct fmsglist fmsghead;        /* head of list of open files */
extern int maxfiles;                    /* kernel limit on number of open files */
extern int nfiles;                      /* actual number of open files */
extern int maxfilesperproc;

#define FILEGLOB_DTYPE(fg)              ((const file_type_t)((fg)->fg_ops->fo_type))
#endif /* __APPLE_API_PRIVATE */


__BEGIN_DECLS

/* wrappers for fp->f_ops->fo_... */
int fo_read(struct fileproc *fp, struct uio *uio, int flags, vfs_context_t ctx);
int fo_write(struct fileproc *fp, struct uio *uio, int flags,
    vfs_context_t ctx);
int fo_ioctl(struct fileproc *fp, u_long com, caddr_t data, vfs_context_t ctx);
int fo_select(struct fileproc *fp, int which, void *wql, vfs_context_t ctx);
int fo_close(struct fileglob *fg, vfs_context_t ctx);
int fo_drain(struct fileproc *fp, vfs_context_t ctx);
int fo_kqfilter(struct fileproc *fp, struct knote *kn, struct kevent_qos_s *kev);

/* Functions to use for unsupported fileops */
int fo_no_read(struct fileproc *fp, struct uio *uio, int flags, vfs_context_t ctx);
int fo_no_write(struct fileproc *fp, struct uio *uio, int flags,
    vfs_context_t ctx);
int fo_no_ioctl(struct fileproc *fp, u_long com, caddr_t data, vfs_context_t ctx);
int fo_no_select(struct fileproc *fp, int which, void *wql, vfs_context_t ctx);
int fo_no_drain(struct fileproc *fp, vfs_context_t ctx);
int fo_no_kqfilter(struct fileproc *, struct knote *, struct kevent_qos_s *kev);

void fileproc_drain(proc_t, struct fileproc *);
int fp_tryswap(proc_t, int fd, struct fileproc *nfp);
int fp_drop(struct proc *p, int fd, struct fileproc *fp, int locked);
int fp_drop_written(proc_t p, int fd, struct fileproc *fp);
int fp_drop_event(proc_t p, int fd, struct fileproc *fp);
void fp_free(struct proc * p, int fd, struct fileproc * fp);
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
int fp_getfvpandvid(struct proc *p, int fd, struct fileproc **resultfp, struct vnode  **resultvp, uint32_t * vidp);
struct socket;
int fp_getfsock(struct proc *p, int fd, struct fileproc **resultfp, struct socket  **results);
int fp_lookup(struct proc *p, int fd, struct fileproc **resultfp, int locked);
int fp_isguarded(struct fileproc *fp, u_int attribs);
int fp_guard_exception(proc_t p, int fd, struct fileproc *fp, u_int attribs);
int closef_locked(struct fileproc *fp, struct fileglob *fg, struct proc *p);
int close_internal_locked(proc_t p, int fd, struct fileproc *fp, int flags);
int fileport_makefd_internal(proc_t p, ipc_port_t port, int uf_flags, int *fd);
struct nameidata;
struct vnode_attr;
int open1(vfs_context_t ctx, struct nameidata *ndp, int uflags,
    struct vnode_attr *vap, fp_allocfn_t fp_zalloc, void *cra,
    int32_t *retval);
int chdir_internal(proc_t p, vfs_context_t ctx, struct nameidata *ndp, int per_thread);
int kqueue_internal(struct proc *p, fp_allocfn_t, void *cra, int32_t *retval);
void fg_insertuipc(struct fileglob * fg);
boolean_t fg_insertuipc_mark(struct fileglob * fg);
void fg_removeuipc(struct fileglob * fg);
boolean_t fg_removeuipc_mark(struct fileglob * fg);
void unp_gc_wait(void);
void procfdtbl_reservefd(struct proc * p, int fd);
void procfdtbl_markclosefd(struct proc * p, int fd);
void procfdtbl_releasefd(struct proc * p, int fd, struct fileproc * fp);
void procfdtbl_waitfd(struct proc * p, int fd);
void procfdtbl_clearfd(struct proc * p, int fd);
boolean_t file_issendable(struct proc * p, struct fileproc *fp);
extern int fdgetf_noref(proc_t, int, struct fileproc **);
extern struct fileproc *fileproc_alloc_init(void *crargs);
extern void fileproc_free(struct fileproc *fp);
extern void guarded_fileproc_free(struct fileproc *fp);
extern void fg_vn_data_free(void *fgvndata);
extern int nameiat(struct nameidata *ndp, int dirfd);
extern int falloc_guarded(struct proc *p, struct fileproc **fp, int *fd,
    vfs_context_t ctx, const guardid_t *guard, u_int attrs);
extern void fileproc_modify_vflags(struct fileproc *fp, fileproc_vflags_t vflags, boolean_t clearflags);
fileproc_vflags_t fileproc_get_vflags(struct fileproc *fp);
__END_DECLS

#endif /* __APPLE_API_UNSTABLE */

#endif /* KERNEL */

#endif /* !_SYS_FILE_INTERNAL_H_ */

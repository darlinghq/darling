/*
 * Copyright (c) 2002-2010 Apple Inc.  All rights reserved.
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
/*-
 * Copyright (c) 1998 Berkeley Software Design, Inc. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Berkeley Software Design Inc's name may not be used to endorse or
 *    promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY BERKELEY SOFTWARE DESIGN INC ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL BERKELEY SOFTWARE DESIGN INC BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      from nfs_lock.h,v 2.2 1998/04/28 19:38:41 don Exp
 * $FreeBSD$
 */

#include <sys/appleapiopts.h>

#ifdef __APPLE_API_PRIVATE

/*
 * lockd uses the nfsclnt system call for the unique kernel services it needs.
 * It passes in a request structure with a version number at the start.
 * This prevents libc from needing to change if the information passed
 * between lockd and the kernel needs to change.
 *
 * If a structure changes, you must bump the version number.
 */

#include <sys/mount.h>

/*
 * The structure that the kernel hands lockd for each lock request.
 */
#define LOCKD_MSG_VERSION       3
typedef struct nfs_lock_msg {
	int                     lm_version;             /* LOCKD_MSG version */
	int                     lm_flags;               /* request flags */
	u_int64_t               lm_xid;                 /* unique message transaction ID */
	struct flock            lm_fl;                  /* The lock request. */
	struct sockaddr_storage lm_addr;                /* The address. */
	int                     lm_fh_len;              /* The file handle length. */
	struct xucred           lm_cred;                /* user cred for lock req */
	u_int8_t                lm_fh[NFSV3_MAX_FH_SIZE]; /* The file handle. */
} LOCKD_MSG;

/* lm_flags */
#define LOCKD_MSG_BLOCK         0x0001  /* a blocking request */
#define LOCKD_MSG_TEST          0x0002  /* just a lock test */
#define LOCKD_MSG_NFSV3         0x0004  /* NFSv3 request */
#define LOCKD_MSG_CANCEL        0x0008  /* cancelling blocked request */
#define LOCKD_MSG_DENIED_GRACE  0x0010  /* lock denied due to grace period */
#define LOCKD_MSG_RECLAIM       0x0020  /* lock reclaim request */
#define LOCKD_MSG_TCP           0x0040  /* (try to) use TCP for request */

/* The structure used to maintain the pending request queue */
typedef struct nfs_lock_msg_request {
	TAILQ_ENTRY(nfs_lock_msg_request) lmr_next;     /* in-kernel pending request list */
	int                     lmr_answered;           /* received an answer? */
	int                     lmr_errno;              /* return status */
	int                     lmr_saved_errno;        /* original return status */
	LOCKD_MSG               lmr_msg;                /* the message */
} LOCKD_MSG_REQUEST;

TAILQ_HEAD(nfs_lock_msg_queue, nfs_lock_msg_request);
typedef struct nfs_lock_msg_queue LOCKD_MSG_QUEUE;


/*
 * The structure that lockd hands the kernel for each lock answer.
 */
#define LOCKD_ANS_VERSION       2
struct lockd_ans {
	int             la_version;             /* lockd_ans version */
	int             la_errno;               /* return status */
	u_int64_t       la_xid;                 /* unique message transaction ID */
	int             la_flags;               /* answer flags */
	pid_t           la_pid;                 /* pid of lock requester/owner */
	off_t           la_start;               /* lock starting offset */
	off_t           la_len;                 /* lock length */
	int             la_fh_len;              /* The file handle length. */
	u_int8_t        la_fh[NFSV3_MAX_FH_SIZE];/* The file handle. */
};

/* la_flags */
#define LOCKD_ANS_GRANTED       0x0001  /* NLM_GRANTED request */
#define LOCKD_ANS_LOCK_INFO     0x0002  /* lock info valid */
#define LOCKD_ANS_LOCK_EXCL     0x0004  /* lock is exclusive */
#define LOCKD_ANS_DENIED_GRACE  0x0008  /* lock denied due to grace period */


/*
 * The structure that lockd hands the kernel for each notify.
 */
#define LOCKD_NOTIFY_VERSION    1
struct lockd_notify {
	int                     ln_version;             /* lockd_notify version */
	int                     ln_flags;               /* notify flags */
	int                     ln_pad;                 /* (for alignment) */
	int                     ln_addrcount;           /* # of addresss */
	struct sockaddr_storage ln_addr[1];             /* List of addresses. */
};


#ifdef KERNEL
void    nfs_lockinit(void);
void    nfs_lockd_mount_register(struct nfsmount *);
void    nfs_lockd_mount_unregister(struct nfsmount *);
int     nfs3_lockd_request(nfsnode_t, int, LOCKD_MSG_REQUEST *, int, thread_t);
int     nfslockdans(proc_t p, struct lockd_ans *ansp);
int     nfslockdnotify(proc_t p, user_addr_t argp);

#endif
#endif /* __APPLE_API_PRIVATE */

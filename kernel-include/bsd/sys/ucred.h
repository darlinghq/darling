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
/* Copyright (c) 1995, 1997 Apple Computer, Inc. All Rights Reserved */
/*
 * Copyright (c) 1989, 1993
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
 *	@(#)ucred.h	8.4 (Berkeley) 1/9/95
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2005 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

#ifndef _SYS_UCRED_H_
#define _SYS_UCRED_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>
#include <sys/param.h>
#include <bsm/audit.h>

struct label;

#ifdef __APPLE_API_UNSTABLE
#ifdef KERNEL
#include <sys/queue.h>

/*
 * In-kernel credential structure.
 *
 * Note that this structure should not be used outside the kernel, nor should
 * it or copies of it be exported outside.
 */
struct ucred {
	LIST_ENTRY(ucred)       cr_link; /* never modify this without KAUTH_CRED_HASH_LOCK */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
	_Atomic u_long          cr_ref;  /* reference count */
#elif defined(__cplusplus) && __cplusplus >= 201103L
	_Atomic u_long          cr_ref;  /* reference count */
#else
	volatile u_long         cr_ref;  /* reference count */
#endif

	struct posix_cred {
		/*
		 * The credential hash depends on everything from this point on
		 * (see kauth_cred_get_hashkey)
		 */
		uid_t   cr_uid;         /* effective user id */
		uid_t   cr_ruid;        /* real user id */
		uid_t   cr_svuid;       /* saved user id */
		short   cr_ngroups;     /* number of groups in advisory list */
		gid_t   cr_groups[NGROUPS];/* advisory group list */
		gid_t   cr_rgid;        /* real group id */
		gid_t   cr_svgid;       /* saved group id */
		uid_t   cr_gmuid;       /* UID for group membership purposes */
		int     cr_flags;       /* flags on credential */
	} cr_posix;
	struct label    *cr_label;      /* MAC label */
	/*
	 * NOTE: If anything else (besides the flags)
	 * added after the label, you must change
	 * kauth_cred_find().
	 */
	struct au_session cr_audit;             /* user auditing data */
};
#else /* KERNEL */
struct ucred;
struct posix_cred;
#endif /* KERNEL */

#ifndef _KAUTH_CRED_T
#define _KAUTH_CRED_T
typedef struct ucred *kauth_cred_t;
typedef struct posix_cred *posix_cred_t;
#endif  /* !_KAUTH_CRED_T */

/*
 * Credential flags that can be set on a credential
 */
#define CRF_NOMEMBERD   0x00000001      /* memberd opt out by setgroups() */
#define CRF_MAC_ENFORCE 0x00000002      /* force entry through MAC Framework */
                                        /* also forces credential cache miss */

/*
 * This is the external representation of struct ucred.
 */
struct xucred {
	u_int   cr_version;             /* structure layout version */
	uid_t   cr_uid;                 /* effective user id */
	short   cr_ngroups;             /* number of advisory groups */
	gid_t   cr_groups[NGROUPS];     /* advisory group list */
};
#define XUCRED_VERSION  0

#define cr_gid cr_groups[0]
#define NOCRED ((kauth_cred_t )0)       /* no credential available */
#define FSCRED ((kauth_cred_t )-1)      /* filesystem credential */

#define IS_VALID_CRED(_cr)      ((_cr) != NOCRED && (_cr) != FSCRED)

#ifdef KERNEL
#ifdef __APPLE_API_OBSOLETE
__BEGIN_DECLS
int             crcmp(kauth_cred_t cr1, kauth_cred_t cr2);
int             suser(kauth_cred_t cred, u_short *acflag);
int             set_security_token(struct proc * p);
int             set_security_token_task_internal(struct proc *p, void *task);
void            cru2x(kauth_cred_t cr, struct xucred *xcr);
__END_DECLS
#endif /* __APPLE_API_OBSOLETE */
#endif /* KERNEL */
#endif /* __APPLE_API_UNSTABLE */

#endif /* !_SYS_UCRED_H_ */

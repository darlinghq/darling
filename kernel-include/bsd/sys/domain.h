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
/* Copyright (c) 1998, 1999 Apple Computer, Inc. All Rights Reserved */
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*
 * Copyright (c) 1982, 1986, 1993
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
 *	@(#)domain.h	8.1 (Berkeley) 6/2/93
 * $FreeBSD: src/sys/sys/domain.h,v 1.14 1999/12/29 04:24:40 peter Exp $
 */

#ifndef _SYS_DOMAIN_H_
#define _SYS_DOMAIN_H_

#ifdef PRIVATE
#include <sys/appleapiopts.h>
#include <sys/cdefs.h>
#ifdef KERNEL_PRIVATE
#include <kern/locks.h>
/*
 * Structure per communications domain.
 */

/*
 * Forward structure declarations for function prototypes [sic].
 */
struct  mbuf;

#pragma pack(4)

#ifdef XNU_KERNEL_PRIVATE
/*
 * Legacy domain.
 *
 * NOTE: Do not modify this structure, as there are modules outside of xnu
 * which rely on the size and layout for binary compatibility.  This structure
 * is simply used by the exported net_{add,del}_domain_old, pffinddomain_old
 * routines.  Internally, domains are stored in the private variant of domain
 * defined down below.
 */
struct domain_old {
#else
struct domain {
#endif /* !XNU_KERNEL_PRIVATE */
	int     dom_family;             /* AF_xxx */
	const char *dom_name;
	void    (*dom_init)(void);      /* initialize domain data structures */
	int     (*dom_externalize)      /* externalize access rights */
	(struct mbuf *);
	void    (*dom_dispose)          /* dispose of internalized rights */
	(struct mbuf *);
#ifdef XNU_KERNEL_PRIVATE
	struct  protosw_old *dom_protosw; /* Chain of protosw's for AF */
	struct  domain_old *dom_next;
#else
	struct  protosw *dom_protosw;   /* Chain of protosw's for AF */
	struct  domain *dom_next;
#endif /* !XNU_KERNEL_PRIVATE */
	int     (*dom_rtattach)         /* initialize routing table */
	(void **, int);
	int     dom_rtoffset;           /* an arg to rtattach, in bits */
	int     dom_maxrtkey;           /* for routing layer */
	int     dom_protohdrlen;        /* len of protocol header */
	int     dom_refs;               /* # socreates outstanding */
	lck_mtx_t *dom_mtx;             /* domain global mutex */
	uint32_t dom_flags;
	uint32_t reserved[2];
};

#pragma pack()

#ifdef XNU_KERNEL_PRIVATE

#include <sys/queue.h>

/*
 * Internal, private and extendable representation of domain.
 */
struct domain {
	int     dom_family;             /* AF_xxx */
	uint32_t dom_flags;             /* domain flags (see below ) */
	uint32_t dom_refs;              /* # socreates outstanding */
	lck_mtx_t *dom_mtx;             /* domain global mutex */
	decl_lck_mtx_data(, dom_mtx_s);
	TAILQ_ENTRY(domain) dom_entry;  /* next domain in list */
	TAILQ_HEAD(, protosw) dom_protosw; /* protosw chain */
	void    (*dom_init)             /* initialize domain data structures */
	(struct domain *);
	int     (*dom_externalize)      /* externalize access rights */
	(struct mbuf *);
	void    (*dom_dispose)          /* dispose of internalized rights */
	(struct mbuf *);
	int     (*dom_rtattach)         /* initialize routing table */
	(void **, int);
	int     dom_rtoffset;           /* an arg to rtattach, in bits */
	int     dom_maxrtkey;           /* for routing layer */
	int     dom_protohdrlen;        /* len of protocol header */
	const char *dom_name;
	struct domain_old *dom_old;     /* domain pointer per net_add_domain */
};

extern TAILQ_HEAD(domains_head, domain) domains;
extern struct domain *localdomain;
#endif /* XNU_KERNEL_PRIVATE */

/*
 * Values for dom_flags
 */
#define DOM_REENTRANT   0x1
#ifdef BSD_KERNEL_PRIVATE
#define DOM_INITIALIZED 0x2             /* domain has been initialized */
#define DOM_ATTACHED    0x4             /* domain is in the global list */
#define DOM_OLD         0x10000000      /* domain added via net_add_domain */

/* pseudo-public domain flags */
#define DOMF_USERFLAGS  (DOM_REENTRANT)
#endif /* BSD_KERNEL_PRIVATE */

__BEGIN_DECLS
#ifdef XNU_KERNEL_PRIVATE
extern void net_add_domain_old(struct domain_old *dp);
extern int net_del_domain_old(struct domain_old *);
extern void net_drain_domains(void);
extern void domain_proto_mtx_lock_assert_held(void);
extern void domain_proto_mtx_lock_assert_notheld(void);
struct domain_guard;
typedef const struct domain_guard *domain_guard_t;
extern domain_guard_t domain_guard_deploy(void);
extern void domain_guard_release(domain_guard_t);
struct domain_unguard;
typedef const struct domain_unguard *domain_unguard_t;
extern domain_unguard_t domain_unguard_deploy(void);
extern void domain_unguard_release(domain_unguard_t);
extern struct domain_old *pffinddomain_old(int);
extern struct domain *pffinddomain(int) __XNU_INTERNAL(pffinddomain);
#else
extern void net_add_domain(struct domain *dp);
extern int net_del_domain(struct domain *);
extern struct domain *pffinddomain(int);
#endif /* XNU_KERNEL_PRIVATE */
__END_DECLS
#endif /* KERNEL_PRIVATE */
#endif /* PRIVATE */
#endif  /* _SYS_DOMAIN_H_ */

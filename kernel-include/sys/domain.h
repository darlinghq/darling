/*
 * Copyright (c) 2000-2006, 2012 Apple Computer, Inc. All rights reserved.
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
#ifdef KERNEL
#include <kern/locks.h>
#endif /* KERNEL */
/*
 * Structure per communications domain.
 */

#include <sys/cdefs.h>

/*
 * Forward structure declarations for function prototypes [sic].
 */
struct	mbuf;
#define DOM_REENTRANT	0x01

#pragma pack(4)

struct	domain {
	int	dom_family;		/* AF_xxx */
	const char	*dom_name;
	void	(*dom_init)(void);	/* initialize domain data structures */
	int	(*dom_externalize)(struct mbuf *);
					/* externalize access rights */
	void	(*dom_dispose)(struct mbuf *);
					/* dispose of internalized rights */
	struct	protosw *dom_protosw;	/* Chain of protosw's for AF */
	struct	domain *dom_next;
	int	(*dom_rtattach)(void **, int);
					/* initialize routing table */
	int	dom_rtoffset;		/* an arg to rtattach, in bits */
	int	dom_maxrtkey;		/* for routing layer */
	int	dom_protohdrlen;	/* Let the protocol tell us */
	int	dom_refs;		/* # socreates outstanding */
#ifdef _KERN_LOCKS_H_
	lck_mtx_t 	*dom_mtx;		/* domain global mutex */
#else
	void 	*dom_mtx;		/* domain global mutex */
#endif
	uint32_t		dom_flags;
	uint32_t		reserved[2];
};

#pragma pack()

#ifdef KERNEL
extern struct	domain *domains;
extern struct	domain localdomain;

__BEGIN_DECLS
extern void	net_add_domain(struct domain *dp);
extern int	net_del_domain(struct domain *);
extern int	domain_proto_mtx_lock(void);
extern void	domain_proto_mtx_unlock(int locked);
__END_DECLS

#define DOMAIN_SET(domain_set) 

#endif /* KERNEL */
#endif /* PRIVATE */
#endif	/* _SYS_DOMAIN_H_ */

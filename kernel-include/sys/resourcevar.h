/*
 * Copyright (c) 2000-2002 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1991, 1993
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
 *	@(#)resourcevar.h	8.4 (Berkeley) 1/9/95
 */

#ifndef	_SYS_RESOURCEVAR_H_
#define	_SYS_RESOURCEVAR_H_

#include <sys/appleapiopts.h>

/*
 * Kernel per-process accounting / statistics
 * (not necessarily resident except when running).
 */
struct pstats {
#define	pstat_startzero	p_ru
	struct	rusage p_ru;		/* stats for this proc */
	struct	rusage p_cru;		/* (PL) sum of stats for reaped children */

	struct uprof {			/* profile arguments */
		struct uprof *pr_next;  /* multiple prof buffers allowed */
		caddr_t	pr_base;	/* buffer base */
		u_int32_t	pr_size;	/* buffer size */
		u_int32_t	pr_off;		/* pc offset */
		u_int32_t	pr_scale;	/* pc scaling */
		u_int32_t	pr_addr;	/* temp storage for addr until AST */
		u_int32_t	pr_ticks;	/* temp storage for ticks until AST */
	} p_prof;
#define	pstat_endzero	p_start

	struct  timeval p_start;	/* starting time ; compat only */
#ifdef KERNEL
	struct user_uprof {			    /* profile arguments */
		struct user_uprof *pr_next;  /* multiple prof buffers allowed */
		user_addr_t	    pr_base;	/* buffer base */
		user_size_t	    pr_size;	/* buffer size */
		user_ulong_t	pr_off;		/* pc offset */
		user_ulong_t	pr_scale;	/* pc scaling */
		user_ulong_t	pr_addr;	/* temp storage for addr until AST */
		user_ulong_t	pr_ticks;	/* temp storage for ticks until AST */
	} user_p_prof;
#endif // KERNEL
};

/*
 * Kernel shareable process resource limits.  Because this structure
 * is moderately large but changes infrequently, it is normally
 * shared copy-on-write after forks.  If a group of processes
 * ("threads") share modifications, the PL_SHAREMOD flag is set,
 * and a copy must be made for the child of a new fork that isn't
 * sharing modifications to the limits.
 */
/* 
 * Modifications are done with the list lock held (p_limit as well)and access indv 
 * limits can be done without limit as we keep the old copy in p_olimit. Which is 
 * dropped in proc_exit. This way all access will have a valid kernel address
 */
struct plimit {
	struct	rlimit pl_rlimit[RLIM_NLIMITS];
	int	pl_refcnt;		/* number of references */
};

#ifdef KERNEL
/* add user profiling from AST */
#define	ADDUPROF(p)							\
    addupc_task(p,							\
                (proc_is64bit((p)) ? (p)->p_stats->user_p_prof.pr_addr \
                                   : CAST_USER_ADDR_T((p)->p_stats->p_prof.pr_addr)), \
                (proc_is64bit((p)) ? (p)->p_stats->user_p_prof.pr_ticks \
                                   : (p)->p_stats->p_prof.pr_ticks))

void	 addupc_intr(struct proc *p, uint32_t pc, u_int ticks);
void	 addupc_task(struct proc *p, user_addr_t pc, u_int ticks);
void	 calcru(struct proc *p, struct timeval *up, struct timeval *sp,
	    struct timeval *ip);
void	 ruadd(struct rusage *ru, struct rusage *ru2);
void proc_limitget(proc_t p, int whichi, struct rlimit * limp);
void proc_limitdrop(proc_t p, int exiting);
void proc_limitfork(proc_t parent, proc_t child);
int proc_limitreplace(proc_t p);
void proc_limitblock(proc_t);
void proc_limitunblock(proc_t);
#endif /* KERNEL */


#endif	/* !_SYS_RESOURCEVAR_H_ */

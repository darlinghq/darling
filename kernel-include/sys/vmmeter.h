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
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*-
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
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
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
 *	@(#)vmmeter.h	8.2 (Berkeley) 7/10/94
 */

#ifndef _SYS_VMMETER_H_
#define	_SYS_VMMETER_H_

#include <sys/appleapiopts.h>

#ifdef __APPLE_API_OBSOLETE
/*
 * System wide statistics counters.
 */
struct vmmeter {
	/*
	 * General system activity.
	 */
	unsigned int v_swtch;		/* context switches */
	unsigned int v_trap;		/* calls to trap */
	unsigned int v_syscall;	/* calls to syscall() */
	unsigned int v_intr;		/* device interrupts */
	unsigned int v_soft;		/* software interrupts */
	unsigned int v_faults;		/* total faults taken */
	/*
	 * Virtual memory activity.
	 */
	unsigned int v_lookups;	/* object cache lookups */
	unsigned int v_hits;		/* object cache hits */
	unsigned int v_vm_faults;	/* number of address memory faults */
	unsigned int v_cow_faults;	/* number of copy-on-writes */
	unsigned int v_swpin;		/* swapins */
	unsigned int v_swpout;		/* swapouts */
	unsigned int v_pswpin;		/* pages swapped in */
	unsigned int v_pswpout;	/* pages swapped out */
	unsigned int v_pageins;	/* number of pageins */
	unsigned int v_pageouts;	/* number of pageouts */
	unsigned int v_pgpgin;		/* pages paged in */
	unsigned int v_pgpgout;	/* pages paged out */
	unsigned int v_intrans;	/* intransit blocking page faults */
	unsigned int v_reactivated;	/* number of pages reactivated from free list */
	unsigned int v_rev;		/* revolutions of the hand */
	unsigned int v_scan;		/* scans in page out daemon */
	unsigned int v_dfree;		/* pages freed by daemon */
	unsigned int v_pfree;		/* pages freed by exiting processes */
	unsigned int v_zfod;		/* pages zero filled on demand */
	unsigned int v_nzfod;		/* number of zfod's created */
	/*
	 * Distribution of page usages.
	 */
	unsigned int v_page_size;	/* page size in bytes */
	unsigned int v_kernel_pages;	/* number of pages in use by kernel */
	unsigned int v_free_target;	/* number of pages desired free */
	unsigned int v_free_min;	/* minimum number of pages desired free */
	unsigned int v_free_count;	/* number of pages free */
	unsigned int v_wire_count;	/* number of pages wired down */
	unsigned int v_active_count;	/* number of pages active */
	unsigned int v_inactive_target; /* number of pages desired inactive */
	unsigned int v_inactive_count;  /* number of pages inactive */
};

/* systemwide totals computed every five seconds */
struct vmtotal
{
	int16_t	t_rq;		/* length of the run queue */
	int16_t	t_dw;		/* jobs in ``disk wait'' (neg priority) */
	int16_t	t_pw;		/* jobs in page wait */
	int16_t	t_sl;		/* jobs sleeping in core */
	int16_t	t_sw;		/* swapped out runnable/short block jobs */
	int32_t	t_vm;		/* total virtual memory */
	int32_t	t_avm;		/* active virtual memory */
	int32_t	t_rm;		/* total real memory in use */
	int32_t	t_arm;		/* active real memory */
	int32_t	t_vmshr;	/* shared virtual memory */
	int32_t	t_avmshr;	/* active shared virtual memory */
	int32_t	t_rmshr;	/* shared real memory */
	int32_t	t_armshr;	/* active shared real memory */
	int32_t	t_free;		/* free memory pages */
};
#ifdef KERNEL
extern struct	vmtotal total;
#endif

#endif /*__APPLE_API_OBSOLETE */

#endif /* !_SYS_VMMETER_H_ */

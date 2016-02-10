/*
 * Copyright (c) 2000-2006 Apple Computer, Inc. All rights reserved.
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
 *	@(#)vm.h	8.5 (Berkeley) 5/11/95
 */
/* HISTORY
 *  05-Jun-95  Mac Gillon (mgillon) at NeXT
 * 	4.4 code uses this file to import MACH API
 */

#ifndef _SYS_VM_H
#define _SYS_VM_H

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>

#ifdef BSD_KERNEL_PRIVATE

/* Machine specific config stuff */
#if	defined(KERNEL) && !defined(MACH_USER_API)	
#include <sys/vmmeter.h>
#include <sys/queue.h>
#include <mach/vm_param.h>
#endif

/*
 * Shareable process virtual address space.
 * May eventually be merged with vm_map.
 * Several fields are temporary (text, data stuff).
 */
struct vmspace {
	int	vm_refcnt;	/* number of references */
	caddr_t	vm_shm;		/* SYS5 shared memory private data XXX */
/* we copy from vm_startcopy to the end of the structure on fork */
#define vm_startcopy vm_rssize
	segsz_t vm_rssize; 	/* current resident set size in pages */
	segsz_t vm_swrss;	/* resident set size before last swap */
	segsz_t vm_tsize;	/* text size (pages) XXX */
	segsz_t vm_dsize;	/* data size (pages) XXX */
	segsz_t vm_ssize;	/* stack size (pages) */
	caddr_t	vm_taddr;	/* user virtual address of text XXX */
	caddr_t	vm_daddr;	/* user virtual address of data XXX */
	caddr_t vm_maxsaddr;	/* user VA at max stack growth */
};

#ifdef KERNEL
/* LP64 version of vmspace.  all pointers 
 * grow when we're dealing with a 64-bit process.
 * WARNING - keep in sync with vmspace
 */

struct user32_vmspace {
	int     	vm_refcnt;      /* number of references */
	uint32_t	vm_shm;			/* SYS5 shared memory private data XXX */
	segsz_t 	vm_rssize;		/* current resident set size in pages */
	segsz_t 	vm_swrss;		/* resident set size before last swap */
	segsz_t 	vm_tsize;		/* text size (pages) XXX */
	segsz_t		vm_dsize;		/* data size (pages) XXX */
	segsz_t 	vm_ssize;		/* stack size (pages) */
	uint32_t	vm_taddr;       /* user virtual address of text XXX */
	uint32_t	vm_daddr;       /* user virtual address of data XXX */
	uint32_t vm_maxsaddr; 	/* user VA at max stack growth */
};
struct user_vmspace {
	int     	vm_refcnt;      /* number of references */
	user_addr_t	vm_shm __attribute((aligned(8)));			/* SYS5 shared memory private data XXX */
	segsz_t 	vm_rssize;		/* current resident set size in pages */
	segsz_t 	vm_swrss;		/* resident set size before last swap */
	segsz_t 	vm_tsize;		/* text size (pages) XXX */
	segsz_t		vm_dsize;		/* data size (pages) XXX */
	segsz_t 	vm_ssize;		/* stack size (pages) */
	user_addr_t	vm_taddr __attribute((aligned(8)));       /* user virtual address of text XXX */
	user_addr_t	vm_daddr;       /* user virtual address of data XXX */
	user_addr_t vm_maxsaddr; 	/* user VA at max stack growth */
};

#endif /* KERNEL */

#include <kern/thread.h>

#else /* BSD_KERNEL_PRIVATE */
/* just to keep kinfo_proc happy */
/* NOTE: Pointer fields are size variant for LP64 */
struct vmspace {
	int32_t	dummy;
	caddr_t	dummy2;
	int32_t	dummy3[5];
	caddr_t	dummy4[3];
};

#endif /* BSD_KERNEL_PRIVATE */

#ifdef KERNEL

__BEGIN_DECLS
struct proc *current_proc(void);
__END_DECLS

#endif /* KERNEL */

#endif /* _SYS_VM_H */

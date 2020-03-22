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
/*
 * Copyright (c) 1992, 1993 NeXT Computer, Inc.
 */

#ifndef	_PPC_SIGNAL_H_
#define	_PPC_SIGNAL_H_ 1

#include <sys/cdefs.h>

#ifndef _ANSI_SOURCE

typedef int sig_atomic_t; 

#include <sys/appleapiopts.h>

#ifdef __APPLE_API_OBSOLETE

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)

#define __need_struct_sigcontext
#define __need_struct_sigcontext32
#define __need_struct_sigcontext64
#include <ppc/_structs.h>

/*
 * Machine-dependant flags used in sigvec call.
 */
#define	SV_SAVE_REGS	0x1000	/* Save all regs in sigcontext */

/*
 * regs_saved_t -- Describes which registers beyond what the kernel cares
 *		   about are saved to and restored from this sigcontext.
 *
 * The default is REGS_SAVED_CALLER, only the caller saved registers
 * are saved.  If the SV_SAVE_REGS flag was set when the signal
 * handler was registered with sigvec() then all the registers will be
 * saved in the sigcontext, and REGS_SAVED_ALL will be set.  The C
 * library uses REGS_SAVED_NONE in order to quickly restore kernel
 * state during a longjmp().
 */
typedef enum {
	REGS_SAVED_NONE,		/* Only kernel managed regs restored */
	REGS_SAVED_CALLER,		/* "Caller saved" regs: rpc, a0-a7,
					   t0-t4, at, lk0-lk1, xt1-xt20,
					   xr0-xr1 */
	REGS_SAVED_ALL			/* All registers */
} regs_saved_t;

#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */

#endif /* __APPLE_API_OBSOLETE */

#endif /* _ANSI_SOURCE */

#endif /* _PPC_SIGNAL_H_ */


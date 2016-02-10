/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/*
 * HISTORY
 * 08-Mar-90  Avadis Tevanian, Jr. (avie) at NeXT
 *	Added errno field to cproc structure.
 *
 */

/*
 * cthread_internals.h - by Eric Cooper
 *
 * Private definitions for the C Threads implementation.
 *
 * The cproc structure is used for different implementations
 * of the basic schedulable units that execute cthreads.
 *
 *	MTHREAD		MACH threads; single address space,
 *			kernel-mode preemptive scheduling
 */
#include <assert.h>
#include <mach/mach.h>
#include <mach/mach_error.h>

/*
 * Low-level thread implementation.
 * This structure must agree with struct ur_cthread in cthreads.h
 */
typedef struct cproc {
	struct cproc *next;	 /* for lock, condition, and ready queues */
	struct cproc *incarnation;	/* for cthread_self() */
	int state;
	mach_port_t reply_port;		/* for mig_get_reply_port() */

	mach_port_t wait_port;

	int id;
	struct cproc *link;		/* so all cprocs can be found
					   after a fork() */
	int flags;

	unsigned int stack_base;
	unsigned int stack_size;
	int	error;

} *cproc_t;

#define	NO_CPROC		((cproc_t) 0)
#define	cproc_self()		((cproc_t) ur_cthread_self())
extern void cthread_set_self(cproc_t p);

/*
 * Possible cproc states.
 */
#define	CPROC_RUNNING		0
#define	CPROC_SPINNING		1
#define	CPROC_BLOCKED		2

/*
 * The cproc flag bits.
 */
#define CPROC_INITIAL_STACK	0x1
#define	CPROC_NOCACHE_THREAD	/* Don't try to cache this cthread on exit */

/*
 * C Threads imports:
 */
#ifdef __STRICT_BSD__
extern char *malloc();
#endif /* __STRICT_BSD__ */

/*
 * Mach imports:
 */
extern void mach_error();

/*
 * C library imports:
 */
#ifdef __STRICT_BSD__
extern exit();
#else
#include <stdlib.h>
#endif /* __STRICT_BSD__ */

/*
 * Macro for MACH kernel calls.
 */
#ifndef MACH_CALL
#define	MACH_CALL(expr, ret) {						\
	if (((ret) = (expr)) != KERN_SUCCESS) {				\
		mach_error(#expr, (ret));				\
		assert(0);						\
	}								\
}
#endif

/*
 * Debugging support.
 */
#ifdef	CTHREADS_DEBUG

#define	private
#define	TRACE(x)	if (cthread_debug) x ; else
extern int cthread_debug;

/*
 * C library imports:
 */
#include <stdio.h>
#include <stdlib.h>
#else	/* CTHREADS_DEBUG */

#define	private static
#define	TRACE(x)

#endif	/* DEBUG */

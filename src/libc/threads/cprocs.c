/*
 * Copyright (c) 1999, 2008 Apple Inc. All rights reserved.
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
 * 22-July-93 Blaine Garst
 *	fixed kernel cache set up of cproc info
 *
 * 05-April-90  Morris Meyer (mmeyer) at NeXT
 * 	Fixed bug in cproc_fork_child() where the first cproc would
 *	try doing a msg_rpc() with an invalid reply port.
 *
 */

/*
 * cprocs.c - by Eric Cooper
 *
 * Implementation of cprocs (lightweight processes)
 * and primitive synchronization operations.
 */
#include "pthread_internals.h"
#include <stdlib.h>
#include "cthreads.h"
#include "cthread_internals.h"
#include <mach/message.h>

/*
 * C Threads imports:
 */
extern void stack_init();
extern void alloc_stack(), _dealloc_stack();

/*
 * Mach imports:
 */
extern mach_port_t mach_thread_self();
extern boolean_t swtch_pri();

#ifdef	CTHREADS_DEBUG
private void
print_cproc(p)
	cproc_t p;
{
	char *s;

	switch (p->state) {
	    case CPROC_RUNNING:
		s = "";
		break;
	    case CPROC_SPINNING:
		s = "+";
		break;
	    case CPROC_BLOCKED:
		s = "*";
		break;
	    default:
		ASSERT(SHOULDNT_HAPPEN);
	}
	printf(" %x(%s)%s",
		p->id,
		cthread_name(p->incarnation), s);
}

private void
print_cproc_queue(name, queue)
	const char * name;
	cthread_queue_t queue;
{
	printf("[%s] %s:", cthread_name(cthread_self()), name);
	cthread_queue_map(queue, cproc_t, print_cproc);
	printf("\n");
}
#endif	/* CTHREADS_DEBUG */

#ifdef	CTHREADS_DEBUG
private cproc_t cprocs = NO_CPROC;	/* linked list of cprocs */

private void
print_all_cprocs()
{
	cproc_t p;

	printf("[%s] cprocs:", cthread_name(cthread_self()));
	for (p = cprocs; p != NO_CPROC; p = p->link)
		print_cproc(p);
	printf("\n");
}
#endif	/* CTHREADS_DEBUG */

/*
 * Routines for supporting fork() of multi-threaded programs.
 */

void
_cproc_fork_child()
/*
 * Called in the child after a fork().  Resets cproc data structures to
 * coincide with the reality that we now have a single cproc and cthread.
 */
{
	pthread_t pself;

        pself = pthread_self();
        pself->reply_port = MACH_PORT_NULL;
}

/*
 *	Support for a per-thread UNIX errno.
 */

#undef errno
extern int errno;
extern int *__error(void);
extern int __pthread_canceled(int);

void
cthread_set_errno_self(int error, int nocancel)
{
	int *ep = __error();
	extern int __unix_conforming;
	pthread_t self = NULL;
	int check_cancel = __unix_conforming && !nocancel;

	if (check_cancel && ((error & 0xff) == EINTR) && (__pthread_canceled(0) == 0)) {
		self = pthread_self();
		if (self != NULL)
			self->cancel_error = error;
		pthread_exit(PTHREAD_CANCELED);
	}

	if (ep != &errno) {
		*ep = error;
	}

	errno = error;
}


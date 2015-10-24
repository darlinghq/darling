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
 * cthreads.c - by Eric Cooper
 *
 * Implementation of cthread_fork, cthread_join, cthread_exit, etc.
 * HISTORY
 * 22-July-93 Blaine Garst
 *	fixed association of read_thread info
 *	fixed kernel cache set up of cproc info
 *
 */
#include "pthread_internals.h"
#include <stdlib.h>
#include <sys/queue.h>
#include "cthreads.h"
#include "cthread_internals.h"
/*
 * C Threads imports:
 */
extern void cproc_init();
extern thread_port_t cproc_create();
extern void mig_init();
extern void _pthread_set_self(pthread_t);

extern void pthread_workqueue_atfork_prepare(void);
extern void pthread_workqueue_atfork_parent(void);
extern void pthread_workqueue_atfork_child(void);
/*
 * Mach imports:
 */
extern void mig_fork_child();

/*
 * C library imports:
 */
extern int _setjmp(jmp_buf env);
extern void _longjmp(jmp_buf env, int val);

/*
 * Thread status bits.
 */
#define	T_MAIN		0x1
#define	T_RETURNED	0x2
#define	T_DETACHED	0x4

#ifdef	CTHREADS_DEBUG
int cthread_debug = FALSE;
#endif	/* CTHREADS_DEBUG */

/*
 * Routines for supporting fork() of multi-threaded programs.
 */


extern void _malloc_fork_prepare(), _malloc_fork_parent();
extern void _malloc_fork_child();
extern void fork_mach_init();
extern void _cproc_fork_child(), _stack_fork_child();
extern void _asl_fork_child(void);
extern void _pthread_fork_child(pthread_t);
extern void _pthread_fork_child_postinit();
extern void _notify_fork_child(void);

static pthread_t psaved_self = 0;
static pthread_lock_t psaved_self_global_lock = LOCK_INITIALIZER;
static pthread_lock_t pthread_atfork_lock = LOCK_INITIALIZER;
struct pthread_atfork_entry {
        TAILQ_ENTRY(pthread_atfork_entry) qentry;
        void (*prepare)(void);
        void (*parent)(void);
        void (*child)(void);
};
static TAILQ_HEAD(pthread_atfork_queue_head, pthread_atfork_entry) pthread_atfork_queue = TAILQ_HEAD_INITIALIZER(pthread_atfork_queue);

int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
	struct pthread_atfork_entry *e;
 
	e = malloc(sizeof(struct pthread_atfork_entry));
	if (e == NULL)
		return (ENOMEM);
 
	e->prepare = prepare;
	e->parent = parent;
	e->child = child;

	_spin_lock(&pthread_atfork_lock);
	TAILQ_INSERT_TAIL(&pthread_atfork_queue, e, qentry);
	_spin_unlock(&pthread_atfork_lock);

	return 0; 
}

void _cthread_fork_prepare()
/*
 * Prepare cthreads library to fork() a multi-threaded program.  All cthread
 * library critical section locks are acquired before a fork() and released
 * afterwards to insure no cthread data structure is left in an inconsistent
 * state in the child, which comes up with only the forking thread running.
 */
{
	struct pthread_atfork_entry *e;

	_spin_lock(&pthread_atfork_lock);
#ifdef TAILQ_FOREACH_REVERSE_LEGACY_ORDER
	TAILQ_FOREACH_REVERSE(e, &pthread_atfork_queue, qentry, pthread_atfork_queue_head)
#else /* !TAILQ_FOREACH_REVERSE_LEGACY_ORDER */
	TAILQ_FOREACH_REVERSE(e, &pthread_atfork_queue, pthread_atfork_queue_head, qentry)
#endif /* TAILQ_FOREACH_REVERSE_LEGACY_ORDER */
	{
		if (e->prepare != NULL)
			e->prepare();
	}

	_spin_lock(&psaved_self_global_lock);
	psaved_self = pthread_self();
	_spin_lock(&psaved_self->lock);
	_malloc_fork_prepare();

	pthread_workqueue_atfork_prepare();
}

void _cthread_fork_parent()
/*
 * Called in the parent process after a fork syscall.
 * Releases locks acquired by cthread_fork_prepare().
 */
{
	struct pthread_atfork_entry *e;

	_malloc_fork_parent();
        _spin_unlock(&psaved_self->lock);
        _spin_unlock(&psaved_self_global_lock);

	TAILQ_FOREACH(e, &pthread_atfork_queue, qentry) {
		if (e->parent != NULL)
			e->parent();
	}
	_spin_unlock(&pthread_atfork_lock);

	pthread_workqueue_atfork_parent();
}

void _cthread_fork_child()
/*
 * Called in the child process after a fork syscall.  Releases locks acquired
 * by cthread_fork_prepare().  Deallocates cthread data structures which
 * described other threads in our parent.  Makes this thread the main thread.
 */
{
	pthread_t p = psaved_self;
        
	_pthread_set_self(p);
        _spin_unlock(&psaved_self_global_lock);   
	mig_fork_child();
	_malloc_fork_child();
	p->kernel_thread = mach_thread_self();
	p->reply_port = mach_reply_port();
	p->__cleanup_stack = NULL;
	p->death = MACH_PORT_NULL;
	p->joiner = NULL;
	p->detached |= _PTHREAD_CREATE_PARENT;
        _spin_unlock(&p->lock);

	_pthread_fork_child(p);
}

void _cthread_fork_child_postinit()
{
	struct pthread_atfork_entry *e;

	__is_threaded = 0;

	_pthread_fork_child_postinit();
	mig_init(1);		/* enable multi-threaded mig interfaces */

	pthread_workqueue_atfork_child();

	TAILQ_FOREACH(e, &pthread_atfork_queue, qentry) {
		if (e->child != NULL)
			e->child();
	}
	LOCK_INIT(pthread_atfork_lock);	
}

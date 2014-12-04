/*
 * Copyright (c) 2000-2008 Apple Inc. All rights reserved.
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
 * Copyright 1996 1995 by Open Software Foundation, Inc. 1997 1996 1995 1994 1993 1992 1991  
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 * 
 */
/*
 * MkLinux
 */

/*
 * POSIX Pthread Library
 */

#include "pthread_internals.h"

#include <assert.h>
#include <stdio.h>	/* For printf(). */
#include <stdlib.h>
#include <errno.h>	/* For __mach_errno_addr() prototype. */
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/queue.h>
#include <machine/vmparam.h>
#include <mach/vm_statistics.h>

extern int __unix_conforming;
extern void __posix_join_cleanup(void *arg);
extern pthread_lock_t _pthread_list_lock;
extern void _pthread_testcancel(pthread_t thread, int isconforming);
extern int _pthread_reap_thread(pthread_t th, mach_port_t kernel_thread, void **value_ptr, int conforming);
extern int _pthread_cond_wait(pthread_cond_t *cond, 
			pthread_mutex_t *mutex,
			const struct timespec *abstime,
			int isRelative,
			int isconforming);
extern int __sigwait(const sigset_t *set, int *sig);

#ifdef VARIANT_CANCELABLE
extern int __semwait_signal(int cond_sem, int mutex_sem, int timeout, int relative, __int64_t tv_sec, __int32_t tv_nsec);
#else
extern int __semwait_signal(int cond_sem, int mutex_sem, int timeout, int relative, __int64_t tv_sec, __int32_t tv_nsec)  __asm__("___semwait_signal_nocancel");
#endif

/*
 * Wait for a thread to terminate and obtain its exit value.
 */
int       
pthread_join(pthread_t thread, 
	     void **value_ptr)
{
	int res = 0;
	pthread_t self = pthread_self();
	mach_port_t kthport;
	int conforming = 0;
#if !__DARWIN_UNIX03
	kern_return_t kern_res;
#endif

#if __DARWIN_UNIX03
	if (__unix_conforming == 0)
		__unix_conforming = 1;

#ifdef VARIANT_CANCELABLE
	_pthread_testcancel(self, 1);
#endif /* VARIANT_CANCELABLE */
#endif /* __DARWIN_UNIX03 */

	if ((res = _pthread_lookup_thread(thread, &kthport, 1)) != 0)
		return(res);

	if (thread->sig == _PTHREAD_SIG)
	{
	if (thread->newstyle == 0) {
		semaphore_t death = new_sem_from_pool(); /* in case we need it */

		LOCK(thread->lock);
		if ((thread->detached & PTHREAD_CREATE_JOINABLE) &&
			thread->death == SEMAPHORE_NULL)
		{

			assert(thread->joiner == NULL);
			if (thread != self && (self == NULL || self->joiner != thread))
			{
				int already_exited = (thread->detached & _PTHREAD_EXITED);

				thread->death = death;
				thread->joiner = self;
				UNLOCK(thread->lock);

				if (!already_exited)
				{
#if __DARWIN_UNIX03
					/* Wait for it to signal... */ 
					pthread_cleanup_push(__posix_join_cleanup, (void *)thread);
					do {
						res = __semwait_signal(death, 0, 0, 0, (int64_t)0, (int32_t)0);
					} while ((res < 0) && (errno == EINTR));
					pthread_cleanup_pop(0);

#else /* __DARWIN_UNIX03 */
					/* Wait for it to signal... */ 
					do {
						PTHREAD_MACH_CALL(semaphore_wait(death), kern_res);
					} while (kern_res != KERN_SUCCESS);
#endif /* __DARWIN_UNIX03 */
				} 

				LOCK(_pthread_list_lock);
				TAILQ_REMOVE(&__pthread_head, thread, plist);
#if WQ_TRACE
				__kdebug_trace(0x9000010, thread, 0, 0, 16, 0);
#endif
				UNLOCK(_pthread_list_lock);
				/* ... and wait for it to really be dead */
				while ((res = _pthread_reap_thread(thread,
							thread->kernel_thread,
							value_ptr, __unix_conforming)) == EAGAIN)
				{
					sched_yield();
				}
			
			} else {
				UNLOCK(thread->lock);
				res = EDEADLK;
			}
		} else {
			UNLOCK(thread->lock);
			res = EINVAL;
		}
		restore_sem_to_pool(death);
		return res;
	} else {
		/* new style */

		semaphore_t death = SEMAPHORE_NULL; /* in case we need it */
		semaphore_t joinsem = SEMAPHORE_NULL;

		if (thread->joiner_notify == MACH_PORT_NULL)
			 death = new_sem_from_pool();

		LOCK(thread->lock);
		if ((thread->detached & PTHREAD_CREATE_JOINABLE) &&
			(thread->joiner == NULL))
		{
			assert(thread->kernel_thread == kthport);
			if (thread != self && (self == NULL || self->joiner != thread))
			{
				if (thread->joiner_notify == MACH_PORT_NULL) {
					if (death == SEMAPHORE_NULL)
						LIBC_ABORT("thread %p: death == SEMAPHORE_NULL", thread);
					thread->joiner_notify = death;
					death = SEMAPHORE_NULL;
				} 
				joinsem = thread->joiner_notify;
				thread->joiner = self;
				UNLOCK(thread->lock);

				if (death != SEMAPHORE_NULL) {
					restore_sem_to_pool(death);
					death = SEMAPHORE_NULL;
				}
#if __DARWIN_UNIX03
				/* Wait for it to signal... */ 
				pthread_cleanup_push(__posix_join_cleanup, (void *)thread);
				do {
					res = __semwait_signal(joinsem, 0, 0, 0, (int64_t)0, (int32_t)0);
				} while ((res < 0) && (errno == EINTR));
				pthread_cleanup_pop(0);
#else /* __DARWIN_UNIX03 */
				/* Wait for it to signal... */ 
				do {
					PTHREAD_MACH_CALL(semaphore_wait(joinsem), kern_res);
				} while (kern_res != KERN_SUCCESS);
#endif /* __DARWIN_UNIX03 */

				restore_sem_to_pool(joinsem);
				res = _pthread_join_cleanup(thread, value_ptr, conforming);
			} else {
				UNLOCK(thread->lock);
				res = EDEADLK;
			}
		} else {
			UNLOCK(thread->lock);
			res = EINVAL;
		}
		if (death != SEMAPHORE_NULL)
			restore_sem_to_pool(death);
		return res;
	}/* end of new style */
	}
	return ESRCH;
}

int       
pthread_cond_wait(pthread_cond_t *cond, 
		  pthread_mutex_t *mutex)
{
	int conforming;
#if __DARWIN_UNIX03

	if (__unix_conforming == 0)
		__unix_conforming = 1;

#ifdef VARIANT_CANCELABLE
	conforming = 1;
#else /* !VARIANT_CANCELABLE */
	conforming = -1;
#endif /* VARIANT_CANCELABLE */
#else /* __DARWIN_UNIX03 */
	conforming = 0;
#endif /* __DARWIN_UNIX03 */
	return (_pthread_cond_wait(cond, mutex, (struct timespec *)NULL, 0, conforming));
}

int       
pthread_cond_timedwait(pthread_cond_t *cond, 
		       pthread_mutex_t *mutex,
		       const struct timespec *abstime)
{
	int conforming;
#if __DARWIN_UNIX03
	if (__unix_conforming == 0)
		__unix_conforming = 1;

#ifdef VARIANT_CANCELABLE
	conforming = 1;
#else /* !VARIANT_CANCELABLE */
	conforming = -1;
#endif /* VARIANT_CANCELABLE */
#else /* __DARWIN_UNIX03 */
        conforming = 0;
#endif /* __DARWIN_UNIX03 */

	return (_pthread_cond_wait(cond, mutex, abstime, 0, conforming));
}

int
sigwait(const sigset_t * set, int * sig)
{
#if __DARWIN_UNIX03
	int err = 0;

	if (__unix_conforming == 0)
		__unix_conforming = 1;

#ifdef VARIANT_CANCELABLE
	_pthread_testcancel(pthread_self(), 1);
#endif /* VARIANT_CANCELABLE */

	if (__sigwait(set, sig) == -1) {
		err = errno;

#ifdef VARIANT_CANCELABLE
		_pthread_testcancel(pthread_self(), 1);
#endif /* VARIANT_CANCELABLE */
		
		/* 
		 * EINTR that isn't a result of pthread_cancel()
		 * is translated to 0.
		 */
		if (err == EINTR) {
			err = 0;
		}
	}
	return(err);
#else /* __DARWIN_UNIX03 */
	if (__sigwait(set, sig) == -1) {
		/* 
		 * EINTR that isn't a result of pthread_cancel()
		 * is translated to 0.
		 */
		if (errno != EINTR) {
			return -1;
		}
	}

	return 0;
#endif /* __DARWIN_UNIX03 */
}

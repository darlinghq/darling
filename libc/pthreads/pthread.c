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
#include "pthread_workqueue.h"

#include <assert.h>
#include <stdio.h>	/* For printf(). */
#include <stdlib.h>
#include <errno.h>	/* For __mach_errno_addr() prototype. */
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <machine/vmparam.h>
#include <mach/vm_statistics.h>
#include <mach/mach_init.h>
#define	__APPLE_API_PRIVATE
#include <machine/cpu_capabilities.h>
#include <libkern/OSAtomic.h>
#if defined(__ppc__)
#include <libkern/OSCrossEndian.h>
#endif
#include <dispatch/private.h> /* for at_fork handlers */


extern int _pthread_setcancelstate_internal(int state, int *oldstate, int conforming);
extern int __pthread_sigmask(int, const sigset_t *, sigset_t *);

#ifndef BUILDING_VARIANT /* [ */

__private_extern__ struct __pthread_list __pthread_head = TAILQ_HEAD_INITIALIZER(__pthread_head);



int32_t workq_targetconc[WORKQ_NUM_PRIOQUEUE];

/* Per-thread kernel support */
extern void _pthread_set_self(pthread_t);
extern void mig_init(int);
static int _pthread_create_pthread_onstack(pthread_attr_t *attrs, void **stack, pthread_t *thread);
static kern_return_t _pthread_free_pthread_onstack(pthread_t t, int freestruct, int termthread);
static void _pthread_struct_init(pthread_t t, const pthread_attr_t *attrs, void * stack, size_t stacksize, int kernalloc, int nozero);
static int  _new_pthread_create_suspended(pthread_t *thread, 
	       const pthread_attr_t *attr,
	       void *(*start_routine)(void *), 
	       void *arg,
		int create_susp);

/* the registered libdispatch worker function */
static void (*__libdispatch_workerfunction)(int, int, void *) = NULL;

/* Get CPU capabilities from the kernel */
__private_extern__ void _init_cpu_capabilities(void);

/* Needed to tell the malloc subsystem we're going multithreaded */
extern void set_malloc_singlethreaded(int);

/* Used when we need to call into the kernel with no reply port */
extern pthread_lock_t reply_port_lock;
int _pthread_find_thread(pthread_t thread);

/* Mach message used to notify that a thread needs to be reaped */

typedef struct _pthread_reap_msg_t {
	mach_msg_header_t header;
	pthread_t thread;
	mach_msg_trailer_t trailer;
} pthread_reap_msg_t;

/* Utilitie */

__private_extern__ uintptr_t commpage_pfz_base=0;

void __pthread_pfz_setup(const char *apple[]) __attribute__ ((visibility ("hidden")));

static uintptr_t __pfz_from_kernel(const char *str)
{
	unsigned long tmpval;
	/* Skip over key to the first value */
	str = strchr(str, '=');
	if (str == NULL)
		return 0;
	str++;
	tmpval = strtoul(str, NULL, 0); /* may err by 0 or ULONG_MAX */
	if (tmpval == ULONG_MAX)
		tmpval = 0;

	return (uintptr_t) tmpval;
}

void
__pthread_pfz_setup(const char *apple[])
{
	const char **p;
	for (p = apple; p && *p; p++) {
		/* checking if matching apple variable is at begining */
		if (strstr(*p, "pfz=") == *p) {
			commpage_pfz_base = __pfz_from_kernel(*p);
			bzero(*p,strlen(*p));
			break;
		}
	}

	if (commpage_pfz_base == 0)
		commpage_pfz_base = _COMM_PAGE_TEXT_START;

	return;
}


/* We'll implement this when the main thread is a pthread */
/* Use the local _pthread struct to avoid malloc before our MiG reply port is set */
static struct _pthread _thread = {0};

/* This global should be used (carefully) by anyone needing to know if a 
** pthread has been created.
*/
int __is_threaded = 0;
/* _pthread_count is protected by _pthread_list_lock */
static int _pthread_count = 1;
int __unix_conforming = 0;
static int __workqueue_newspis = 0;
static int __workqueue_oldspis = 0;
__private_extern__ size_t pthreadsize = 0;

/* under rosetta we will use old style creation of threads */
static int __oldstyle = 0;

__private_extern__ pthread_lock_t _pthread_list_lock = LOCK_INITIALIZER;

/* Same implementation as LOCK, but without the __is_threaded check */
int _spin_tries = 0;
extern kern_return_t syscall_thread_switch(mach_port_name_t, int, mach_msg_timeout_t);
__private_extern__ void _spin_lock_retry(pthread_lock_t *lock)
{ 
	int tries = _spin_tries;
	do {
		if (tries-- > 0)
			continue;
		syscall_thread_switch(THREAD_NULL, SWITCH_OPTION_DEPRESS, 1);
		tries = _spin_tries;
	} while(!_spin_lock_try(lock));
}

static mach_port_t thread_recycle_port = MACH_PORT_NULL;

/* These are used to keep track of a semaphore pool shared by mutexes and condition
** variables.
*/

static semaphore_t *sem_pool = NULL;
static int sem_pool_count = 0;
static int sem_pool_current = 0;
static pthread_lock_t sem_pool_lock = LOCK_INITIALIZER;

static int default_priority;
static int max_priority;
static int min_priority;
static int pthread_concurrency;

static OSSpinLock __workqueue_list_lock = OS_SPINLOCK_INIT;

static void _pthread_exit(pthread_t self, void *value_ptr) __dead2;
static void _pthread_setcancelstate_exit(pthread_t self, void  *value_ptr, int conforming);
static pthread_attr_t _pthread_attr_default = {0};
static void _pthread_workq_init(pthread_workqueue_t wq, const pthread_workqueue_attr_t * attr);
static int kernel_workq_setup = 0;
static volatile int32_t kernel_workq_count = 0;
static volatile unsigned int user_workq_count = 0; 	/* number of outstanding workqueues */
static volatile unsigned int user_workitem_count = 0; 	/* number of outstanding workitems */
#define KERNEL_WORKQ_ELEM_MAX  	64		/* Max number of elements in the kerrel */
static int wqreadyprio = 0;	/* current highest prio queue ready with items */

__private_extern__ struct __pthread_workitem_pool __pthread_workitem_pool_head = TAILQ_HEAD_INITIALIZER(__pthread_workitem_pool_head);
__private_extern__ struct __pthread_workqueue_pool __pthread_workqueue_pool_head = TAILQ_HEAD_INITIALIZER(__pthread_workqueue_pool_head);

static struct _pthread_workitem *  __workqueue_pool_ptr;
static size_t __workqueue_pool_size = 0;
static int __workqueue_nitems = 0;

struct _pthread_workqueue_head __pthread_workq0_head;
struct _pthread_workqueue_head __pthread_workq1_head;
struct _pthread_workqueue_head __pthread_workq2_head;
struct _pthread_workqueue_head __pthread_workq3_head;
pthread_workqueue_head_t __pthread_wq_head_tbl[WORKQ_NUM_PRIOQUEUE] = {&__pthread_workq0_head, &__pthread_workq1_head, &__pthread_workq2_head, &__pthread_workq3_head};

static void workqueue_list_lock(void);
static void workqueue_list_unlock(void);
static int valid_workq(pthread_workqueue_t);
static void pick_nextworkqueue_droplock(void);
static int post_nextworkitem(pthread_workqueue_t workq);
static void _pthread_workq_return(pthread_t self);
static pthread_workqueue_attr_t _pthread_wq_attr_default = {0};
extern void start_wqthread(pthread_t self, mach_port_t kport, void * stackaddr, pthread_workitem_t item, int reuse);
extern void thread_start(pthread_t self, mach_port_t kport, void *(*fun)(void *), void * funarg, size_t stacksize, unsigned int flags);
static pthread_workitem_t alloc_workitem(void);
static void free_workitem(pthread_workitem_t);
static void grow_workitem(void);
static pthread_workqueue_t alloc_workqueue(void);
static void free_workqueue(pthread_workqueue_t);
static int _pthread_work_internal_init(void);
static void workqueue_exit(pthread_t self, pthread_workqueue_t workq, pthread_workitem_t item);
void _pthread_fork_child_postinit();

void pthread_workqueue_atfork_prepare(void);
void pthread_workqueue_atfork_parent(void);
void pthread_workqueue_atfork_child(void);

extern void dispatch_atfork_prepare(void);
extern void dispatch_atfork_parent(void);
extern void dispatch_atfork_child(void);

/* workq_kernreturn commands */
#define WQOPS_QUEUE_ADD 1
#define WQOPS_QUEUE_REMOVE 2
#define WQOPS_THREAD_RETURN 4
#define WQOPS_THREAD_SETCONC  8
#define WQOPS_QUEUE_NEWSPISUPP  0x10    /* this is to check for newer SPI support */
#define WQOPS_QUEUE_REQTHREADS  0x20	/* request number of threads of a prio */

/* flag values for reuse field in the libc side _pthread_wqthread */
#define WQ_FLAG_THREAD_PRIOMASK		0x0000ffff
#define WQ_FLAG_THREAD_OVERCOMMIT	0x00010000      /* thread is with overcommit prio */
#define WQ_FLAG_THREAD_REUSE		0x00020000      /* thread is being reused */
#define WQ_FLAG_THREAD_NEWSPI		0x00040000      /* the call is with new SPIs */


#define WORKQUEUE_OVERCOMMIT 0x10000	/* the work_kernreturn() for overcommit in prio field */

/*
 * Flags filed passed to bsdthread_create and back in pthread_start 
31  <---------------------------------> 0
_________________________________________
| flags(8) | policy(8) | importance(16) |
-----------------------------------------
*/
__private_extern__
void _pthread_start(pthread_t self, mach_port_t kport, void *(*fun)(void *), void * funarg, size_t stacksize, unsigned int flags);

__private_extern__
void _pthread_wqthread(pthread_t self, mach_port_t kport, void * stackaddr, pthread_workitem_t item, int reuse);

#define PTHREAD_START_CUSTOM	0x01000000
#define PTHREAD_START_SETSCHED	0x02000000
#define PTHREAD_START_DETACHED	0x04000000
#define PTHREAD_START_POLICY_BITSHIFT 16
#define PTHREAD_START_POLICY_MASK 0xff
#define PTHREAD_START_IMPORTANCE_MASK 0xffff

static int pthread_setschedparam_internal(pthread_t, mach_port_t, int, const struct sched_param *);
extern pthread_t __bsdthread_create(void *(*func)(void *), void * func_arg, void * stack, pthread_t  thread, unsigned int flags);
extern int __bsdthread_register(void (*)(pthread_t, mach_port_t, void *(*)(void *), void *, size_t, unsigned int), void (*)(pthread_t, mach_port_t, void *, pthread_workitem_t, int), int,void (*)(pthread_t, mach_port_t, void *(*)(void *), void *, size_t, unsigned int), int32_t *,__uint64_t);
extern int __bsdthread_terminate(void * freeaddr, size_t freesize, mach_port_t kport, mach_port_t joinsem);
extern __uint64_t __thread_selfid( void );
extern int __pthread_canceled(int);
extern void _pthread_keys_init(void);
extern int __pthread_kill(mach_port_t, int);
extern int __pthread_markcancel(int);
extern int __workq_open(void);


extern int __workq_kernreturn(int, pthread_workitem_t, int, int);

#if defined(__ppc__) || defined(__ppc64__)
static const vm_address_t PTHREAD_STACK_HINT = 0xF0000000;
#elif defined(__i386__) || defined(__x86_64__)
static const vm_address_t PTHREAD_STACK_HINT = 0xB0000000;
#elif defined(__arm__)  
static const vm_address_t PTHREAD_STACK_HINT = 0x30000000;
#else
#error Need to define a stack address hint for this architecture
#endif

/* Set the base address to use as the stack pointer, before adjusting due to the ABI
 * The guardpages for stackoverflow protection is also allocated here
 * If the stack was already allocated(stackaddr in attr)  then there are no guardpages 
 * set up for the thread
 */

static int
_pthread_allocate_stack(pthread_attr_t *attrs, void **stack)
{
    kern_return_t kr;
    vm_address_t stackaddr;
    size_t guardsize;

    assert(attrs->stacksize >= PTHREAD_STACK_MIN);
    if (attrs->stackaddr != NULL) {
	/* No guard pages setup in this case */
        assert(((uintptr_t)attrs->stackaddr % vm_page_size) == 0);
        *stack = attrs->stackaddr;
         return 0;
    }

    guardsize = attrs->guardsize;
    stackaddr = PTHREAD_STACK_HINT;
    kr = vm_map(mach_task_self(), &stackaddr,
    			attrs->stacksize + guardsize,
    			vm_page_size-1,
    			VM_MAKE_TAG(VM_MEMORY_STACK)| VM_FLAGS_ANYWHERE , MEMORY_OBJECT_NULL,
    			0, FALSE, VM_PROT_DEFAULT, VM_PROT_ALL,
    			VM_INHERIT_DEFAULT);
    if (kr != KERN_SUCCESS)
    	kr = vm_allocate(mach_task_self(),
    					&stackaddr, attrs->stacksize + guardsize,
    					VM_MAKE_TAG(VM_MEMORY_STACK)| VM_FLAGS_ANYWHERE);
    if (kr != KERN_SUCCESS) {
        return EAGAIN;
    }
     /* The guard page is at the lowest address */
     /* The stack base is the highest address */
    if (guardsize)
    	kr = vm_protect(mach_task_self(), stackaddr, guardsize, FALSE, VM_PROT_NONE);
    *stack = (void *)(stackaddr + attrs->stacksize + guardsize);
        return 0;
}

static int
_pthread_create_pthread_onstack(pthread_attr_t *attrs, void **stack, pthread_t *thread)
{
    kern_return_t kr;
	pthread_t t;
    vm_address_t stackaddr;
    size_t guardsize, allocsize;

	assert(attrs->stacksize >= PTHREAD_STACK_MIN);

    if (attrs->stackaddr != NULL) {
	/* No guard pages setup in this case */
		assert(((uintptr_t)attrs->stackaddr % vm_page_size) == 0);
		*stack = attrs->stackaddr;
		t = (pthread_t)malloc(pthreadsize);
		_pthread_struct_init(t, attrs, attrs->stackaddr, 0, 0, 0);
		t->freeStackOnExit = 0;
		t->freeaddr = 0;
		t->freesize = 0;
		*thread = t;
 		return 0;
	}

    guardsize = attrs->guardsize;
	allocsize = attrs->stacksize + guardsize + pthreadsize;
    stackaddr = PTHREAD_STACK_HINT;
    kr = vm_map(mach_task_self(), &stackaddr,
    			allocsize,
    			vm_page_size-1,
    			VM_MAKE_TAG(VM_MEMORY_STACK)| VM_FLAGS_ANYWHERE , MEMORY_OBJECT_NULL,
    			0, FALSE, VM_PROT_DEFAULT, VM_PROT_ALL,
    			VM_INHERIT_DEFAULT);
    if (kr != KERN_SUCCESS)
    	kr = vm_allocate(mach_task_self(),
    					&stackaddr, allocsize,
    					VM_MAKE_TAG(VM_MEMORY_STACK)| VM_FLAGS_ANYWHERE);
    if (kr != KERN_SUCCESS) {
        return EAGAIN;
    }
     /* The guard page is at the lowest address */
     /* The stack base is the highest address */
    if (guardsize)
    	kr = vm_protect(mach_task_self(), stackaddr, guardsize, FALSE, VM_PROT_NONE);


	*stack = (void *)(stackaddr + attrs->stacksize + guardsize);

	t = (pthread_t)(stackaddr + attrs->stacksize + guardsize);
	_pthread_struct_init(t, attrs, *stack, 0, 0, 1);
	t->kernalloc = 0;
	t->freesize = allocsize;
	t->freeaddr = (void *)stackaddr;
	t->freeStackOnExit = 1;
	*thread = t;

    return 0;
}

static kern_return_t
_pthread_free_pthread_onstack(pthread_t t, int freestruct, int termthread)
{
	kern_return_t res = 0;
    	vm_address_t freeaddr;
    	size_t freesize;
	int thread_count;
	mach_port_t kport;
	semaphore_t joinsem = SEMAPHORE_NULL;

#if PTH_TRACE
	__kdebug_trace(0x900001c, freestruct, termthread, 0, 0, 0);
#endif
	kport = t->kernel_thread;
	joinsem = t->joiner_notify;

	if (t->freeStackOnExit) {
		freeaddr = (vm_address_t)t->freeaddr;
		if (freestruct)
			freesize = t->stacksize + t->guardsize + pthreadsize;
		else
			freesize = t->stacksize + t->guardsize;
		if (termthread) {
			mig_dealloc_reply_port(MACH_PORT_NULL);
			LOCK(_pthread_list_lock);
			if (freestruct != 0) {
				TAILQ_REMOVE(&__pthread_head, t, plist);
				/* if parent has not returned from create yet keep pthread_t */
#if PTH_LISTTRACE
				__kdebug_trace(0x9000010, t, 0, 0, 1, 0);
#endif
				if (t->parentcheck == 0)
					freesize -= pthreadsize;
			}
			t->childexit = 1;
			thread_count = --_pthread_count;
			UNLOCK(_pthread_list_lock);

#if PTH_TRACE
		__kdebug_trace(0x9000020, freeaddr, freesize, kport, 1, 0);
#endif
			if (thread_count <=0)
				exit(0);
			else
				__bsdthread_terminate((void *)freeaddr, freesize, kport, joinsem);
			LIBC_ABORT("thread %p didn't terminate", t);
		} else  {
#if PTH_TRACE
			__kdebug_trace(0x9000024, freeaddr, freesize, 0, 1, 0);
#endif
			res = vm_deallocate(mach_task_self(), freeaddr, freesize);
		}
	} else {
		if (termthread) {
			mig_dealloc_reply_port(MACH_PORT_NULL);
			LOCK(_pthread_list_lock);
			if (freestruct != 0) {
				TAILQ_REMOVE(&__pthread_head, t, plist);
#if PTH_LISTTRACE
				__kdebug_trace(0x9000010, t, 0, 0, 2, 0);
#endif
			}
			thread_count = --_pthread_count;
			t->childexit = 1;
			UNLOCK(_pthread_list_lock);

			if (freestruct)  {
#if PTH_TRACE
		__kdebug_trace(0x9000008, t, 0, 0, 2, 0);
#endif
				free(t);
			}

			freeaddr = 0;
			freesize = 0;
#if PTH_TRACE
		__kdebug_trace(0x9000020, 0, 0, kport, 2, 0);
#endif

			if (thread_count <=0)
				exit(0);
			else
				__bsdthread_terminate(NULL, 0, kport, joinsem);
			LIBC_ABORT("thread %p didn't terminate", t);
		} else if (freestruct) {
		       t->sig = _PTHREAD_NO_SIG;
#if PTH_TRACE
			__kdebug_trace(0x9000024, t, 0, 0, 2, 0);
#endif
			free(t);
		}
	}
	return(res);
} 



/*
 * Destroy a thread attribute structure
 */
int       
pthread_attr_destroy(pthread_attr_t *attr)
{
	if (attr->sig == _PTHREAD_ATTR_SIG)
	{
		attr->sig = 0;
		return (0);
	} else
	{
		return (EINVAL); /* Not an attribute structure! */
	}
}

/*
 * Get the 'detach' state from a thread attribute structure.
 * Note: written as a helper function for info hiding
 */
int       
pthread_attr_getdetachstate(const pthread_attr_t *attr, 
			    int *detachstate)
{
	if (attr->sig == _PTHREAD_ATTR_SIG)
	{
		*detachstate = attr->detached;
		return (0);
	} else
	{
		return (EINVAL); /* Not an attribute structure! */
	}
}

/*
 * Get the 'inherit scheduling' info from a thread attribute structure.
 * Note: written as a helper function for info hiding
 */
int       
pthread_attr_getinheritsched(const pthread_attr_t *attr, 
			     int *inheritsched)
{
	if (attr->sig == _PTHREAD_ATTR_SIG)
	{
		*inheritsched = attr->inherit;
		return (0);
	} else
	{
		return (EINVAL); /* Not an attribute structure! */
	}
}

/*
 * Get the scheduling parameters from a thread attribute structure.
 * Note: written as a helper function for info hiding
 */
int       
pthread_attr_getschedparam(const pthread_attr_t *attr, 
			   struct sched_param *param)
{
	if (attr->sig == _PTHREAD_ATTR_SIG)
	{
		*param = attr->param;
		return (0);
	} else
	{
		return (EINVAL); /* Not an attribute structure! */
	}
}

/*
 * Get the scheduling policy from a thread attribute structure.
 * Note: written as a helper function for info hiding
 */
int       
pthread_attr_getschedpolicy(const pthread_attr_t *attr, 
			    int *policy)
{
	if (attr->sig == _PTHREAD_ATTR_SIG)
	{
		*policy = attr->policy;
		return (0);
	} else
	{
		return (EINVAL); /* Not an attribute structure! */
	}
}

/* Retain the existing stack size of 512K and not depend on Main thread default stack size */
static const size_t DEFAULT_STACK_SIZE = (512*1024);
/*
 * Initialize a thread attribute structure to default values.
 */
int       
pthread_attr_init(pthread_attr_t *attr)
{
	attr->stacksize = DEFAULT_STACK_SIZE;
	attr->stackaddr = NULL;
	attr->sig = _PTHREAD_ATTR_SIG;
	attr->param.sched_priority = default_priority;
	attr->param.quantum = 10; /* quantum isn't public yet */
	attr->detached = PTHREAD_CREATE_JOINABLE;
	attr->inherit = _PTHREAD_DEFAULT_INHERITSCHED;
	attr->policy = _PTHREAD_DEFAULT_POLICY;
	attr->freeStackOnExit = 1;
	attr->fastpath = 1;
	attr->schedset = 0;
	attr->guardsize = vm_page_size;
	return (0);
}

/*
 * Set the 'detach' state in a thread attribute structure.
 * Note: written as a helper function for info hiding
 */
int       
pthread_attr_setdetachstate(pthread_attr_t *attr, 
			    int detachstate)
{
	if (attr->sig == _PTHREAD_ATTR_SIG)
	{
		if ((detachstate == PTHREAD_CREATE_JOINABLE) ||
		    (detachstate == PTHREAD_CREATE_DETACHED))
		{
			attr->detached = detachstate;
			return (0);
		} else
		{
			return (EINVAL);
		}
	} else
	{
		return (EINVAL); /* Not an attribute structure! */
	}
}

/*
 * Set the 'inherit scheduling' state in a thread attribute structure.
 * Note: written as a helper function for info hiding
 */
int       
pthread_attr_setinheritsched(pthread_attr_t *attr, 
			     int inheritsched)
{
	if (attr->sig == _PTHREAD_ATTR_SIG)
	{
		if ((inheritsched == PTHREAD_INHERIT_SCHED) ||
		    (inheritsched == PTHREAD_EXPLICIT_SCHED))
		{
			attr->inherit = inheritsched;
			return (0);
		} else
		{
			return (EINVAL);
		}
	} else
	{
		return (EINVAL); /* Not an attribute structure! */
	}
}

/*
 * Set the scheduling paramters in a thread attribute structure.
 * Note: written as a helper function for info hiding
 */
int       
pthread_attr_setschedparam(pthread_attr_t *attr, 
			   const struct sched_param *param)
{
	if (attr->sig == _PTHREAD_ATTR_SIG)
	{
		/* TODO: Validate sched_param fields */
		attr->param = *param;
		attr->schedset = 1;
		return (0);
	} else
	{
		return (EINVAL); /* Not an attribute structure! */
	}
}

/*
 * Set the scheduling policy in a thread attribute structure.
 * Note: written as a helper function for info hiding
 */
int       
pthread_attr_setschedpolicy(pthread_attr_t *attr, 
			    int policy)
{
	if (attr->sig == _PTHREAD_ATTR_SIG)
	{
		if ((policy == SCHED_OTHER) ||
		    (policy == SCHED_RR) ||
		    (policy == SCHED_FIFO))
		{
			attr->policy = policy;
			attr->schedset = 1;
			return (0);
		} else
		{
			return (EINVAL);
		}
	} else
	{
		return (EINVAL); /* Not an attribute structure! */
	}
}

/*
 * Set the scope for the thread.
 * We currently only provide PTHREAD_SCOPE_SYSTEM
 */
int
pthread_attr_setscope(pthread_attr_t *attr,
                            int scope)
{
    if (attr->sig == _PTHREAD_ATTR_SIG) {
        if (scope == PTHREAD_SCOPE_SYSTEM) {
            /* No attribute yet for the scope */
            return (0);
        } else if (scope == PTHREAD_SCOPE_PROCESS) {
            return (ENOTSUP);
        }
    }
    return (EINVAL); /* Not an attribute structure! */
}

/*
 * Get the scope for the thread.
 * We currently only provide PTHREAD_SCOPE_SYSTEM
 */
int
pthread_attr_getscope(const pthread_attr_t *attr,
                            int *scope)
{
    if (attr->sig == _PTHREAD_ATTR_SIG) {
        *scope = PTHREAD_SCOPE_SYSTEM;
        return (0);
    }
    return (EINVAL); /* Not an attribute structure! */
}

/* Get the base stack address of the given thread */
int
pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr)
{
    if (attr->sig == _PTHREAD_ATTR_SIG) {
        *stackaddr = attr->stackaddr;
        return (0);
    } else {
        return (EINVAL); /* Not an attribute structure! */
    }
}

int
pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr)
{
    if ((attr->sig == _PTHREAD_ATTR_SIG) && (((uintptr_t)stackaddr % vm_page_size) == 0)) {
        attr->stackaddr = stackaddr;
        attr->freeStackOnExit = 0;
	attr->fastpath = 0;
        return (0);
    } else {
        return (EINVAL); /* Not an attribute structure! */
    }
}

int
pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
    if (attr->sig == _PTHREAD_ATTR_SIG) {
        *stacksize = attr->stacksize;
        return (0);
    } else {
        return (EINVAL); /* Not an attribute structure! */
    }
}

int
pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
    if ((attr->sig == _PTHREAD_ATTR_SIG) && ((stacksize % vm_page_size) == 0) && (stacksize >= PTHREAD_STACK_MIN)) {
        attr->stacksize = stacksize;
        return (0);
    } else {
        return (EINVAL); /* Not an attribute structure! */
    }
}

int
pthread_attr_getstack(const pthread_attr_t *attr, void **stackaddr, size_t * stacksize)
{
    if (attr->sig == _PTHREAD_ATTR_SIG) {
	*stackaddr = (void *)((uintptr_t)attr->stackaddr - attr->stacksize);
        *stacksize = attr->stacksize;
        return (0);
    } else {
        return (EINVAL); /* Not an attribute structure! */
    }
}

/* By SUSV spec, the stackaddr is the base address, the lowest addressable 
 * byte address. This is not the same as in pthread_attr_setstackaddr.
 */
int
pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize)
{
    if ((attr->sig == _PTHREAD_ATTR_SIG) && 
	(((uintptr_t)stackaddr % vm_page_size) == 0) && 
	 ((stacksize % vm_page_size) == 0) && (stacksize >= PTHREAD_STACK_MIN)) {
		attr->stackaddr = (void *)((uintptr_t)stackaddr + stacksize);
        	attr->stacksize = stacksize;
        	attr->freeStackOnExit = 0;
		attr->fastpath = 0;
        	return (0);
    } else {
        return (EINVAL); /* Not an attribute structure! */
    }
}


/*
 * Set the guardsize attribute in the attr.
 */
int
pthread_attr_setguardsize(pthread_attr_t *attr,
                            size_t guardsize)
{
    if (attr->sig == _PTHREAD_ATTR_SIG) {
	/* Guardsize of 0 is valid, ot means no guard */
        if ((guardsize % vm_page_size) == 0) {
		attr->guardsize = guardsize;
		attr->fastpath = 0;
            return (0);
	} else
		return(EINVAL);
    }
    return (EINVAL); /* Not an attribute structure! */
}

/*
 * Get the guardsize attribute in the attr.
 */
int
pthread_attr_getguardsize(const pthread_attr_t *attr,
                            size_t *guardsize)
{
    if (attr->sig == _PTHREAD_ATTR_SIG) {
        *guardsize = attr->guardsize;
        return (0);
    }
    return (EINVAL); /* Not an attribute structure! */
}


/*
 * Create and start execution of a new thread.
 */

static void
_pthread_body(pthread_t self)
{
    _pthread_set_self(self);
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) 
	if( (self->thread_id = __thread_selfid()) == (__uint64_t)-1)
		printf("Failed to set thread_id in _pthread_body\n");
#endif
    _pthread_exit(self, (self->fun)(self->arg));
}

void
_pthread_start(pthread_t self, mach_port_t kport, void *(*fun)(void *), void * funarg, size_t stacksize, unsigned int pflags)
{
#if WQ_DEBUG
	pthread_t pself;
#endif
	pthread_attr_t *attrs = &_pthread_attr_default;
	char * stackaddr;

	if ((pflags & PTHREAD_START_CUSTOM) == 0) {
		stackaddr = (char *)self;
		_pthread_struct_init(self, attrs, stackaddr,  stacksize, 1, 1);
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) 
		_pthread_set_self(self);
#endif
		LOCK(_pthread_list_lock);
		if (pflags & PTHREAD_START_SETSCHED) {
			self->policy = ((pflags >> PTHREAD_START_POLICY_BITSHIFT) & PTHREAD_START_POLICY_MASK);
			self->param.sched_priority = (pflags & PTHREAD_START_IMPORTANCE_MASK);
		}
		/* These are not joinable threads */
		if ((pflags & PTHREAD_START_DETACHED) == PTHREAD_START_DETACHED)  {
			self->detached &= ~PTHREAD_CREATE_JOINABLE;
			self->detached |= PTHREAD_CREATE_DETACHED;
		}
	}  else { 
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) 
		_pthread_set_self(self);
#endif
		LOCK(_pthread_list_lock);
	}
	self->kernel_thread = kport;
	self->fun = fun;
	self->arg = funarg;
	
	/* Add to the pthread list */
	if (self->parentcheck == 0) {
		TAILQ_INSERT_TAIL(&__pthread_head, self, plist);
#if PTH_LISTTRACE
		__kdebug_trace(0x900000c, self, 0, 0, 3, 0);
#endif
		_pthread_count++;
	}
	self->childrun = 1;
	UNLOCK(_pthread_list_lock);

#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) 
	if( (self->thread_id = __thread_selfid()) == (__uint64_t)-1)
		printf("Failed to set thread_id in pthread_start\n");
#endif

#if WQ_DEBUG
	pself = pthread_self();
	if (self != pself)
		LIBC_ABORT("self %p != pself %p", self, pself);
#endif
#if PTH_TRACE
	__kdebug_trace(0x9000030, self, pflags, 0, 0, 0);
#endif

	_pthread_exit(self, (self->fun)(self->arg));
}

int
_pthread_create(pthread_t t,
		const pthread_attr_t *attrs,
                void *stack,
		const mach_port_t kernel_thread)
{
	int res;
	res = 0;
	
	do
	{
		memset(t, 0, sizeof(*t));
		t->newstyle = 0;
		t->schedset = 0;
		t->kernalloc = 0;
		t->tsd[0] = t;
		t->max_tsd_key = 0;
		t->wqthread = 0;
		t->cur_workq = 0;
		t->cur_workitem = 0;
		t->stacksize = attrs->stacksize;
                t->stackaddr = (void *)stack;
		t->guardsize = attrs->guardsize;
                t->kernel_thread = kernel_thread;
		t->detached = attrs->detached;
		t->inherit = attrs->inherit;
		t->policy = attrs->policy;
		t->param = attrs->param;
		t->freeStackOnExit = attrs->freeStackOnExit;
		t->cancel_error = 0;
		t->sig = _PTHREAD_SIG;
                t->reply_port = MACH_PORT_NULL;
                t->cthread_self = NULL;
		LOCK_INIT(t->lock);
		t->plist.tqe_next = (struct _pthread *)0;
		t->plist.tqe_prev = (struct _pthread **)0;
		t->cancel_state = PTHREAD_CANCEL_ENABLE | PTHREAD_CANCEL_DEFERRED;
		t->__cleanup_stack = (struct __darwin_pthread_handler_rec *)NULL;
		t->death = SEMAPHORE_NULL;

		if (kernel_thread != MACH_PORT_NULL)
			(void)pthread_setschedparam_internal(t, kernel_thread, t->policy, &t->param);
	} while (0);
	return (res);
}

void
_pthread_struct_init(pthread_t t, const pthread_attr_t *attrs, void * stack, size_t stacksize, int kernalloc, int nozero)
{
	mach_vm_offset_t stackaddr = (mach_vm_offset_t)(uintptr_t)stack;

		if (nozero == 0) {
			memset(t, 0, sizeof(*t));
			t->plist.tqe_next = (struct _pthread *)0;
			t->plist.tqe_prev = (struct _pthread **)0;
		}
		t->schedset = attrs->schedset;
		t->tsd[0] = t;
		if (kernalloc != 0) {
			stackaddr = (mach_vm_offset_t)(uintptr_t)t;

			/* if allocated from kernel set values appropriately */
			t->stacksize = stacksize;
       			t->stackaddr = (void *)(uintptr_t)stackaddr;
			t->freeStackOnExit = 1;
			t->freeaddr = (void *)(uintptr_t)(stackaddr - stacksize - vm_page_size);
			t->freesize = pthreadsize + stacksize + vm_page_size;
		} else {
			t->stacksize = attrs->stacksize;
			t->stackaddr = (void *)stack;
		}
		t->guardsize = attrs->guardsize;
		t->detached = attrs->detached;
		t->inherit = attrs->inherit;
		t->policy = attrs->policy;
		t->param = attrs->param;
		t->cancel_error = 0;
		t->sig = _PTHREAD_SIG;
		t->reply_port = MACH_PORT_NULL;
		t->cthread_self = NULL;
		LOCK_INIT(t->lock);
		t->cancel_state = PTHREAD_CANCEL_ENABLE | PTHREAD_CANCEL_DEFERRED;
		t->__cleanup_stack = (struct __darwin_pthread_handler_rec *)NULL;
		t->death = SEMAPHORE_NULL;
		t->newstyle = 1;
		t->kernalloc = kernalloc;
		t->wqthread = 0;
		t->cur_workq = 0;
		t->cur_workitem = 0;
		t->max_tsd_key = 0;
}

/* Need to deprecate this in future */
int
_pthread_is_threaded(void)
{
    return __is_threaded;
}

/* Non portable public api to know whether this process has(had) atleast one thread 
 * apart from main thread. There could be race if there is a thread in the process of
 * creation at the time of call . It does not tell whether there are more than one thread
 * at this point of time.
 */
int
pthread_is_threaded_np(void)
{
    return (__is_threaded);
}

mach_port_t
pthread_mach_thread_np(pthread_t t)
{
	mach_port_t kport = MACH_PORT_NULL;

	if (t == NULL)
		goto out;
	
	/* 
	 * If the call is on self, return the kernel port. We cannot
	 * add this bypass for main thread as it might have exited,
	 * and we should not return stale port info.
	 */
	if (t == pthread_self())
	{
		kport = t->kernel_thread;
		goto out;
	}
	
	if (_pthread_lookup_thread(t, &kport, 0) != 0)
		return((mach_port_t)0);

out:
	return(kport);
}

pthread_t pthread_from_mach_thread_np(mach_port_t kernel_thread) 
{
    struct _pthread * p = NULL;

	/* No need to wait as mach port is already known */
    LOCK(_pthread_list_lock);
    TAILQ_FOREACH(p, &__pthread_head, plist) {
        if (p->kernel_thread == kernel_thread) 
			break;
    }
    UNLOCK(_pthread_list_lock);     
    return p;
}

size_t
pthread_get_stacksize_np(pthread_t t)
{
	int ret;
	size_t size = 0;

	if (t == NULL)
		return(ESRCH);
	
	if ( t == pthread_self() || t == &_thread ) //since the main thread will not get de-allocated from underneath us
	{
		size=t->stacksize;
		return size;
	}


	LOCK(_pthread_list_lock);

	if ((ret = _pthread_find_thread(t)) != 0) {
		UNLOCK(_pthread_list_lock);
		return(ret);
	}

	size=t->stacksize;
	UNLOCK(_pthread_list_lock);
	
	return(size);
}

void *
pthread_get_stackaddr_np(pthread_t t)
{
	int ret;
	void * addr = NULL;

	if (t == NULL)
		return((void *)(uintptr_t)ESRCH);
	
	if(t == pthread_self() || t == &_thread) //since the main thread will not get deallocated from underneath us
		return t->stackaddr;

	LOCK(_pthread_list_lock);

	if ((ret = _pthread_find_thread(t)) != 0) {
		UNLOCK(_pthread_list_lock);
		return((void *)(uintptr_t)ret);
	}
	addr = t->stackaddr;
	UNLOCK(_pthread_list_lock);

	return(addr);
}

mach_port_t
_pthread_reply_port(pthread_t t)
{
    return t->reply_port;
}


/* returns non-zero if the current thread is the main thread */
int
pthread_main_np(void)
{
	pthread_t self = pthread_self();

	return ((self->detached & _PTHREAD_CREATE_PARENT) == _PTHREAD_CREATE_PARENT);
}


#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) 
/* if we are passed in a pthread_t that is NULL, then we return
   the current thread's thread_id. So folks don't have to call
   pthread_self, in addition to us doing it, if they just want 
   their thread_id.
*/
int
pthread_threadid_np(pthread_t thread, __uint64_t *thread_id)
{
	int rval=0;
	pthread_t self = pthread_self();

	if (thread_id == NULL) {
		return(EINVAL);
	} else if (thread == NULL || thread == self) {
		*thread_id = self->thread_id;
		return rval;
	}
	
	LOCK(_pthread_list_lock);
	if ((rval = _pthread_find_thread(thread)) != 0) {
		UNLOCK(_pthread_list_lock);
		return(rval);
	}
	*thread_id = thread->thread_id;
	UNLOCK(_pthread_list_lock);
	return rval;
}
#endif

int
pthread_getname_np(pthread_t thread, char *threadname, size_t len)
{
	int rval;
	rval = 0;

	if (thread == NULL)
		return(ESRCH);
	
	LOCK(_pthread_list_lock);
	if ((rval = _pthread_find_thread(thread)) != 0) {
		UNLOCK(_pthread_list_lock);
		return(rval);
	}
	strlcpy(threadname, thread->pthread_name, len);
	UNLOCK(_pthread_list_lock);
	return rval;
}

int
pthread_setname_np(const char *threadname)
{
	int rval = 0;
	int len = 0;
	pthread_t current_thread = pthread_self();

	if (threadname != NULL)
		len = strlen(threadname);

	/* protytype is in pthread_internals.h */
	rval = proc_setthreadname((void *)threadname, len);
	if (rval == 0) {
		if (threadname != NULL) {
			strlcpy(current_thread->pthread_name, threadname, MAXTHREADNAMESIZE);
		} else {
			memset(current_thread->pthread_name, 0 , MAXTHREADNAMESIZE);
		}

	}
	return rval;

}

static int       
_new_pthread_create_suspended(pthread_t *thread, 
	       const pthread_attr_t *attr,
	       void *(*start_routine)(void *), 
	       void *arg,
		int create_susp)
{
	pthread_attr_t *attrs;
	void *stack;
	int error;
	unsigned int flags;
	pthread_t t,t2;
	kern_return_t kern_res;
	mach_port_t kernel_thread = MACH_PORT_NULL;
	int needresume;
	task_t self = mach_task_self();
	int kernalloc = 0;
	int susp = create_susp;

	if ((attrs = (pthread_attr_t *)attr) == (pthread_attr_t *)NULL)
	{			/* Set up default paramters */
		attrs = &_pthread_attr_default;
	} else if (attrs->sig != _PTHREAD_ATTR_SIG) {
            return EINVAL;
	}
	error = 0;

	if (((attrs->policy != _PTHREAD_DEFAULT_POLICY)  || 
		(attrs->param.sched_priority != default_priority)) && (create_susp == 0)) {
		needresume = 1;	
		susp = 1;	
	} else 
		needresume = 0;

	/* In default policy (ie SCHED_OTHER) only sched_priority is used. Check for
	 * any change in priority or policy is needed here.
	 */
	if ((__oldstyle == 1) || (create_susp != 0)) {
		/* Rosetta or pthread_create_suspended() */
		/* running under rosetta */
		/* Allocate a stack for the thread */
#if PTH_TRACE
		__kdebug_trace(0x9000000, create_susp, 0, 0, 0, 0);
#endif
                if ((error = _pthread_allocate_stack(attrs, &stack)) != 0) {
			return(error);
                }
		t = (pthread_t)malloc(sizeof(struct _pthread));
		*thread = t;
		if (susp) {
			/* Create the Mach thread for this thread */
			PTHREAD_MACH_CALL(thread_create(self, &kernel_thread), kern_res);
			if (kern_res != KERN_SUCCESS)
			{
				printf("Can't create thread: %d\n", kern_res);
				return(EINVAL);
			}
		}
                if ((error = _pthread_create(t, attrs, stack, kernel_thread)) != 0)
		{
			return(error);
		}
		set_malloc_singlethreaded(0);
		__is_threaded = 1;

		/* Send it on it's way */
		t->arg = arg;
		t->fun = start_routine;
		t->newstyle = 0;
		/* Now set it up to execute */
		LOCK(_pthread_list_lock);
		TAILQ_INSERT_TAIL(&__pthread_head, t, plist);
#if PTH_LISTTRACE
		__kdebug_trace(0x900000c, t, 0, 0, 4, 0);
#endif
		_pthread_count++;
		UNLOCK(_pthread_list_lock);
		_pthread_setup(t, _pthread_body, stack, susp, needresume);
		return(0);
	} else {

		flags = 0;
		if (attrs->fastpath == 1)
			kernalloc = 1;

		if (attrs->detached == PTHREAD_CREATE_DETACHED)
				flags |= PTHREAD_START_DETACHED;
		if (attrs->schedset != 0) {
			flags |= PTHREAD_START_SETSCHED;
			flags |= ((attrs->policy & PTHREAD_START_POLICY_MASK) << PTHREAD_START_POLICY_BITSHIFT);
			flags |= (attrs->param.sched_priority & PTHREAD_START_IMPORTANCE_MASK);
		} 

		set_malloc_singlethreaded(0);
		__is_threaded = 1;

		if (kernalloc == 0) {
			/* Allocate a stack for the thread */
			flags |= PTHREAD_START_CUSTOM;
			if ((error = _pthread_create_pthread_onstack(attrs, &stack, &t)) != 0) {
				return(error);
			}
			/* Send it on it's way */
			t->arg = arg;
			t->fun = start_routine;
			t->newstyle = 1;

#if PTH_TRACE
		__kdebug_trace(0x9000004, t, flags, 0, 0, 0);
#endif
			
			if ((t2 = __bsdthread_create(start_routine, arg, stack, t, flags)) == (pthread_t)-1) {
				_pthread_free_pthread_onstack(t, 1, 0);
				return (EAGAIN);
			}
			else t=t2;
			LOCK(_pthread_list_lock);
			t->parentcheck = 1;
			if ((t->childexit != 0) && ((t->detached & PTHREAD_CREATE_DETACHED) == PTHREAD_CREATE_DETACHED)) {
				/* detached child exited, mop up */
				UNLOCK(_pthread_list_lock);
#if PTH_TRACE
			__kdebug_trace(0x9000008, t, 0, 0, 1, 0);
#endif
			if(t->freeStackOnExit)
                                vm_deallocate(self, (mach_vm_address_t)(uintptr_t)t, pthreadsize);
                        else
				free(t);
			} else if (t->childrun == 0) {
				TAILQ_INSERT_TAIL(&__pthread_head, t, plist);
				_pthread_count++;
#if PTH_LISTTRACE
				__kdebug_trace(0x900000c, t, 0, 0, 1, 0);
#endif
				UNLOCK(_pthread_list_lock);
			} else 
				UNLOCK(_pthread_list_lock);

			*thread = t;

#if PTH_TRACE
		__kdebug_trace(0x9000014, t, 0, 0, 1, 0);
#endif
			return (0);

		} else {
			/* kernel allocation */
#if PTH_TRACE
		__kdebug_trace(0x9000018, flags, 0, 0, 0, 0);
#endif
			if ((t = __bsdthread_create(start_routine, arg, (void *)attrs->stacksize, NULL, flags)) == (pthread_t)-1)
				return (EAGAIN);
			/* Now set it up to execute */
			LOCK(_pthread_list_lock);
			t->parentcheck = 1;
			if ((t->childexit != 0) && ((t->detached & PTHREAD_CREATE_DETACHED) == PTHREAD_CREATE_DETACHED)) {
				/* detached child exited, mop up */
				UNLOCK(_pthread_list_lock);
#if PTH_TRACE
			__kdebug_trace(0x9000008, t, pthreadsize, 0, 2, 0);
#endif
				vm_deallocate(self, (mach_vm_address_t)(uintptr_t)t, pthreadsize);
			} else if (t->childrun == 0) {
				TAILQ_INSERT_TAIL(&__pthread_head, t, plist);
				_pthread_count++;
#if PTH_LISTTRACE
				__kdebug_trace(0x900000c, t, 0, 0, 2, 0);
#endif
				UNLOCK(_pthread_list_lock);
			} else 
				UNLOCK(_pthread_list_lock);

			*thread = t;

#if PTH_TRACE
			__kdebug_trace(0x9000014, t, 0, 0, 2, 0);
#endif
			return(0);
		}
	}
}

static int       
_pthread_create_suspended(pthread_t *thread, 
	       const pthread_attr_t *attr,
	       void *(*start_routine)(void *), 
	       void *arg,
           int suspended)
{
	pthread_attr_t *attrs;
	void *stack;
	int res;
	pthread_t t;
	kern_return_t kern_res;
	mach_port_t kernel_thread = MACH_PORT_NULL;
	int needresume;

	if ((attrs = (pthread_attr_t *)attr) == (pthread_attr_t *)NULL)
	{			/* Set up default paramters */
		attrs = &_pthread_attr_default;
	} else if (attrs->sig != _PTHREAD_ATTR_SIG) {
            return EINVAL;
	}
	res = 0;

	/* In default policy (ie SCHED_OTHER) only sched_priority is used. Check for
	 * any change in priority or policy is needed here.
	 */
	if (((attrs->policy != _PTHREAD_DEFAULT_POLICY)  || 
		(attrs->param.sched_priority != default_priority)) && (suspended == 0)) {
		needresume = 1;	
		suspended = 1;	
	} else 
		needresume = 0;

	do
	{
		/* Allocate a stack for the thread */
                if ((res = _pthread_allocate_stack(attrs, &stack)) != 0) {
                    break;
                }
		t = (pthread_t)malloc(sizeof(struct _pthread));
		*thread = t;
		if (suspended) {
			/* Create the Mach thread for this thread */
			PTHREAD_MACH_CALL(thread_create(mach_task_self(), &kernel_thread), kern_res);
			if (kern_res != KERN_SUCCESS)
			{
				printf("Can't create thread: %d\n", kern_res);
				res = EINVAL; /* Need better error here? */
				break;
			}
		}
                if ((res = _pthread_create(t, attrs, stack, kernel_thread)) != 0)
		{
			break;
		}
		set_malloc_singlethreaded(0);
		__is_threaded = 1;

		/* Send it on it's way */
		t->arg = arg;
		t->fun = start_routine;
		/* Now set it up to execute */
		LOCK(_pthread_list_lock);
		TAILQ_INSERT_TAIL(&__pthread_head, t, plist);
#if PTH_LISTTRACE
		__kdebug_trace(0x900000c, t, 0, 0, 5, 0);
#endif
		_pthread_count++;
		UNLOCK(_pthread_list_lock);
		_pthread_setup(t, _pthread_body, stack, suspended, needresume);
	} while (0);
	return (res);
}

int
pthread_create(pthread_t *thread,
           const pthread_attr_t *attr,
           void *(*start_routine)(void *),
           void *arg)
{
    return _new_pthread_create_suspended(thread, attr, start_routine, arg, 0);
}

int
pthread_create_suspended_np(pthread_t *thread,
           const pthread_attr_t *attr,
           void *(*start_routine)(void *),
           void *arg)
{
    return _pthread_create_suspended(thread, attr, start_routine, arg, 1);
}

/*
 * Make a thread 'undetached' - no longer 'joinable' with other threads.
 */
int       
pthread_detach(pthread_t thread)
{
	int newstyle = 0;
	int ret;

	if ((ret = _pthread_lookup_thread(thread, NULL, 1)) != 0) {
		return (ret); /* Not a valid thread */
	}

	LOCK(thread->lock);
	newstyle = thread->newstyle;
	if (thread->detached & PTHREAD_CREATE_JOINABLE)
	{
		if (thread->detached & _PTHREAD_EXITED) {
			UNLOCK(thread->lock);
			pthread_join(thread, NULL);
			return 0;
		} else {
			if (newstyle == 0) {
				semaphore_t death = thread->death;

				thread->detached &= ~PTHREAD_CREATE_JOINABLE;
				thread->detached |= PTHREAD_CREATE_DETACHED;
				UNLOCK(thread->lock);
				if (death)
					(void) semaphore_signal(death);
			} else {
				mach_port_t joinport = thread->joiner_notify;
	
				thread->detached &= ~PTHREAD_CREATE_JOINABLE;
				thread->detached |= PTHREAD_CREATE_DETACHED;
				
				UNLOCK(thread->lock);
				if (joinport) {
					semaphore_signal(joinport);
				}
			}
			return(0);
		}
	} else {
		UNLOCK(thread->lock);
		return (EINVAL);
	}
}


/*
 * pthread_kill call to system call
 */
int   
pthread_kill (
        pthread_t th,
        int sig)
{
	int error = 0;
	mach_port_t kport = MACH_PORT_NULL;
	
	if ((sig < 0) || (sig > NSIG))
		return(EINVAL);

	if (_pthread_lookup_thread(th, &kport, 0) != 0)
		return (ESRCH); /* Not a valid thread */

	/* if the thread is a workqueue thread, just return error */
	if ((th->wqthread != 0) && (th->wqkillset ==0)) {
		return(ENOTSUP);
	}

	error = __pthread_kill(kport, sig);

	if (error == -1)
		error = errno;
	return(error);
}

int 
__pthread_workqueue_setkill(int enable)
{
	pthread_t self = pthread_self();

	LOCK(self->lock);
	if (enable == 0)
		self->wqkillset = 0;
	else
		self->wqkillset = 1;
	UNLOCK(self->lock);

	return(0);

}

/* Announce that there are pthread resources ready to be reclaimed in a */
/* subsequent pthread_exit or reaped by pthread_join. In either case, the Mach */
/* thread underneath is terminated right away. */
static
void _pthread_become_available(pthread_t thread, mach_port_t kernel_thread) {
	pthread_reap_msg_t msg;
	kern_return_t ret;

	msg.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_MAKE_SEND,
					      MACH_MSG_TYPE_MOVE_SEND);
	msg.header.msgh_size = sizeof msg - sizeof msg.trailer;
	msg.header.msgh_remote_port = thread_recycle_port;
	msg.header.msgh_local_port = kernel_thread; 
	msg.header.msgh_id = 0x44454144; /* 'DEAD' */
	msg.thread = thread;
	ret = mach_msg_send(&msg.header);
	assert(ret == MACH_MSG_SUCCESS);
}

/* Reap the resources for available threads */
__private_extern__
int _pthread_reap_thread(pthread_t th, mach_port_t kernel_thread, void **value_ptr, int conforming) {
	mach_port_type_t ptype;
	kern_return_t ret;
	task_t self;

	self = mach_task_self();
	if (kernel_thread != MACH_PORT_DEAD) {
		ret = mach_port_type(self, kernel_thread, &ptype);
		if (ret == KERN_SUCCESS && ptype != MACH_PORT_TYPE_DEAD_NAME) {
			/* not quite dead yet... */
			return EAGAIN;
		}
		ret = mach_port_deallocate(self, kernel_thread);
		if (ret != KERN_SUCCESS) {
			fprintf(stderr,
				  "mach_port_deallocate(kernel_thread) failed: %s\n",
				  mach_error_string(ret));
		}
	}

	if (th->reply_port != MACH_PORT_NULL) {
		ret = mach_port_mod_refs(self, th->reply_port,
						 MACH_PORT_RIGHT_RECEIVE, -1);
		if (ret != KERN_SUCCESS) {
			fprintf(stderr,
				  "mach_port_mod_refs(reply_port) failed: %s\n",
				  mach_error_string(ret));
		}
	}

	if (th->freeStackOnExit) {
		vm_address_t addr = (vm_address_t)th->stackaddr;
		vm_size_t size;

		size = (vm_size_t)th->stacksize + th->guardsize;
		
		addr -= size;
		ret = vm_deallocate(self, addr, size);
		if (ret != KERN_SUCCESS) {
		  fprintf(stderr,
				    "vm_deallocate(stack) failed: %s\n",
				    mach_error_string(ret));
		}
	}


	if (value_ptr) 
		*value_ptr = th->exit_value;
	if (conforming) {
		if ((th->cancel_state & (PTHREAD_CANCEL_ENABLE|_PTHREAD_CANCEL_PENDING)) ==
            (PTHREAD_CANCEL_ENABLE|_PTHREAD_CANCEL_PENDING) && (value_ptr != NULL)) 
			*value_ptr = PTHREAD_CANCELED;
		th->sig = _PTHREAD_NO_SIG;
	}


	if (th != &_thread)
		free(th);

	return 0;
}

static
void _pthread_reap_threads(void)
{
	pthread_reap_msg_t msg;
	kern_return_t ret;

	ret = mach_msg(&msg.header, MACH_RCV_MSG|MACH_RCV_TIMEOUT, 0,
			sizeof msg, thread_recycle_port, 
			MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	while (ret == MACH_MSG_SUCCESS) {
		mach_port_t kernel_thread = msg.header.msgh_remote_port;
		pthread_t thread = msg.thread;

		/* deal with race with thread_create_running() */
		if (kernel_thread == MACH_PORT_NULL &&
		    kernel_thread != thread->kernel_thread) {
			kernel_thread = thread->kernel_thread;
		}
		
		if ( kernel_thread == MACH_PORT_NULL ||
		     _pthread_reap_thread(thread, kernel_thread, (void **)0, 0) == EAGAIN)
		{
			/* not dead yet, put it back for someone else to reap, stop here */
			_pthread_become_available(thread, kernel_thread);
			return;
		}

		ret = mach_msg(&msg.header, MACH_RCV_MSG|MACH_RCV_TIMEOUT, 0,
				sizeof msg, thread_recycle_port, 
				MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	}
}

/* For compatibility... */

pthread_t
_pthread_self() {
	return pthread_self();
}

/*
 * Terminate a thread.
 */
int __disable_threadsignal(int);

static void 
_pthread_exit(pthread_t self, void *value_ptr)
{
	struct __darwin_pthread_handler_rec *handler;
	kern_return_t kern_res;
	int thread_count;
	int newstyle = self->newstyle;

	/* Make this thread not to receive any signals */
	__disable_threadsignal(1);

#if PTH_TRACE
	__kdebug_trace(0x900001c, self, newstyle, 0, 0, 0);
#endif

	/* set cancel state to disable and type to deferred */
	_pthread_setcancelstate_exit(self, value_ptr, __unix_conforming);

	while ((handler = self->__cleanup_stack) != 0)
	{
		(handler->__routine)(handler->__arg);
		self->__cleanup_stack = handler->__next;
	}
	_pthread_tsd_cleanup(self);

	if (newstyle == 0) {
		_pthread_reap_threads();

		LOCK(self->lock);
		self->detached |= _PTHREAD_EXITED;

		if (self->detached & PTHREAD_CREATE_JOINABLE) {
			mach_port_t death = self->death;
			self->exit_value = value_ptr;
			UNLOCK(self->lock);
			/* the joiner will need a kernel thread reference, leave ours for it */
			if (death) {
				PTHREAD_MACH_CALL(semaphore_signal(death), kern_res);
				if (kern_res != KERN_SUCCESS)
					fprintf(stderr,
						"semaphore_signal(death) failed: %s\n",
						mach_error_string(kern_res));
			}
			LOCK(_pthread_list_lock);
			thread_count = --_pthread_count;
			UNLOCK(_pthread_list_lock);
		} else {
			UNLOCK(self->lock);
			LOCK(_pthread_list_lock);
			TAILQ_REMOVE(&__pthread_head, self, plist);
#if PTH_LISTTRACE
			__kdebug_trace(0x9000010, self, 0, 0, 5, 0);
#endif
			thread_count = --_pthread_count;
			UNLOCK(_pthread_list_lock);
			/* with no joiner, we let become available consume our cached ref */
			_pthread_become_available(self, self->kernel_thread);
		}

		if (thread_count <= 0)
			exit(0);

		/* Use a new reference to terminate ourselves. Should never return. */
		PTHREAD_MACH_CALL(thread_terminate(mach_thread_self()), kern_res);
		fprintf(stderr, "thread_terminate(mach_thread_self()) failed: %s\n",
				mach_error_string(kern_res));
	} else {
		semaphore_t joinsem = SEMAPHORE_NULL;

		if ((self->joiner_notify == (mach_port_t)0) && (self->detached & PTHREAD_CREATE_JOINABLE))
			joinsem = new_sem_from_pool();
		LOCK(self->lock);
		self->detached |= _PTHREAD_EXITED;

		self->exit_value = value_ptr;
		if (self->detached & PTHREAD_CREATE_JOINABLE) {
			if (self->joiner_notify == (mach_port_t)0) {
				self->joiner_notify = joinsem;
				joinsem = SEMAPHORE_NULL;
			}
			UNLOCK(self->lock);
			if (joinsem != SEMAPHORE_NULL)
				restore_sem_to_pool(joinsem);
			_pthread_free_pthread_onstack(self, 0, 1);
		} else {
			UNLOCK(self->lock);
			/* with no joiner, we let become available consume our cached ref */
			if (joinsem != SEMAPHORE_NULL)
				restore_sem_to_pool(joinsem);
			_pthread_free_pthread_onstack(self, 1, 1);
		}
	}
	LIBC_ABORT("thread %p didn't exit", self);
}

void
pthread_exit(void *value_ptr)
{
	pthread_t self = pthread_self();
	/* if the current thread is a workqueue thread, just crash the app, as per libdispatch folks */
	if (self->wqthread == 0) {
		_pthread_exit(self, value_ptr);
	} else {
		LIBC_ABORT("pthread_exit() may only be called against threads created via pthread_create()");
	}
}

/*
 * Get the scheduling policy and scheduling paramters for a thread.
 */
int       
pthread_getschedparam(pthread_t thread, 
		      int *policy,
		      struct sched_param *param)
{
	int ret;

	if (thread == NULL)
		return(ESRCH);
	
	LOCK(_pthread_list_lock);

	if ((ret = _pthread_find_thread(thread)) != 0) {
		UNLOCK(_pthread_list_lock);
		return(ret);
	}
	if (policy != 0)
		*policy = thread->policy;
	if (param != 0)
		*param = thread->param;
	UNLOCK(_pthread_list_lock);

	return(0);
}

/*
 * Set the scheduling policy and scheduling paramters for a thread.
 */
static int       
pthread_setschedparam_internal(pthread_t thread, 
		      mach_port_t  kport,
		      int policy,
		      const struct sched_param *param)
{
	policy_base_data_t bases;
	policy_base_t base;
	mach_msg_type_number_t count;
	kern_return_t ret;

	switch (policy)
	{
		case SCHED_OTHER:
			bases.ts.base_priority = param->sched_priority;
			base = (policy_base_t)&bases.ts;
			count = POLICY_TIMESHARE_BASE_COUNT;
			break;
		case SCHED_FIFO:
			bases.fifo.base_priority = param->sched_priority;
			base = (policy_base_t)&bases.fifo;
			count = POLICY_FIFO_BASE_COUNT;
			break;
		case SCHED_RR:
			bases.rr.base_priority = param->sched_priority;
			/* quantum isn't public yet */
			bases.rr.quantum = param->quantum;
			base = (policy_base_t)&bases.rr;
			count = POLICY_RR_BASE_COUNT;
			break;
		default:
			return (EINVAL);
	}
	ret = thread_policy(kport, policy, base, count, TRUE);
	if (ret != KERN_SUCCESS)
			return (EINVAL);
	return (0);
}

int       
pthread_setschedparam(pthread_t t, 
		      int policy,
		      const struct sched_param *param)
{
	mach_port_t kport = MACH_PORT_NULL;
	int error;
	int bypass = 1;

	if (t != pthread_self() && t != &_thread ) { //since the main thread will not get de-allocated from underneath us
		bypass = 0;
		if (_pthread_lookup_thread(t, &kport, 0) != 0)
			return(ESRCH);
	} else
		kport = t->kernel_thread;

	error = pthread_setschedparam_internal(t, kport, policy, param);
	if (error == 0) {
		if (bypass == 0) {
			/* ensure the thread is still valid */
			LOCK(_pthread_list_lock);
			if ((error = _pthread_find_thread(t)) != 0) {
				UNLOCK(_pthread_list_lock);
				return(error);
			}
			t->policy = policy;
			t->param = *param;
			UNLOCK(_pthread_list_lock);
		}  else {
			t->policy = policy;
			t->param = *param;
		}
	}
	return(error);
}

/*
 * Get the minimum priority for the given policy
 */
int
sched_get_priority_min(int policy)
{
    return default_priority - 16;
}

/*
 * Get the maximum priority for the given policy
 */
int
sched_get_priority_max(int policy)
{
    return default_priority + 16;
}

/*
 * Determine if two thread identifiers represent the same thread.
 */
int       
pthread_equal(pthread_t t1, 
	      pthread_t t2)
{
	return (t1 == t2);
}

// Force LLVM not to optimise this to a call to __pthread_set_self, if it does
// then _pthread_set_self won't be bound when secondary threads try and start up.
void __attribute__((noinline))
_pthread_set_self(pthread_t p)
{
	extern void __pthread_set_self(void *);

        if (p == 0) {
                if (_thread.tsd[0] != 0) {
                        bzero(&_thread, sizeof(struct _pthread));
                }
                p = &_thread;
        }
        p->tsd[0] = p;
	__pthread_set_self(&p->tsd[0]);
}

void 
cthread_set_self(void *cself)
{
    pthread_t self = pthread_self();
    if ((self == (pthread_t)NULL) || (self->sig != _PTHREAD_SIG)) {
        _pthread_set_self(cself);
        return;
    }
    self->cthread_self = cself;
}

void *
ur_cthread_self(void) {
    pthread_t self = pthread_self();
    if ((self == (pthread_t)NULL) || (self->sig != _PTHREAD_SIG)) {
        return (void *)self;
    }
    return self->cthread_self;
}

/*
 * cancellation handler for pthread once as the init routine can have a
 * cancellation point. In that case we need to restore the spin unlock 
 */
void
__pthread_once_cancel_handler(pthread_once_t *once_control)
{
		_spin_unlock(&once_control->lock);
}


/*
 * Execute a function exactly one time in a thread-safe fashion.
 */
int       
pthread_once(pthread_once_t *once_control, 
	     void (*init_routine)(void))
{
	_spin_lock(&once_control->lock);
	if (once_control->sig == _PTHREAD_ONCE_SIG_init)
	{
		pthread_cleanup_push((void (*)(void *))__pthread_once_cancel_handler, once_control);
		(*init_routine)();
		pthread_cleanup_pop(0);
		once_control->sig = _PTHREAD_ONCE_SIG;
	}
	_spin_unlock(&once_control->lock);
	return (0);  /* Spec defines no possible errors! */
}

/*
 * Insert a cancellation point in a thread.
 */
__private_extern__ void
_pthread_testcancel(pthread_t thread, int isconforming)
{
	LOCK(thread->lock);
	if ((thread->cancel_state & (PTHREAD_CANCEL_ENABLE|_PTHREAD_CANCEL_PENDING)) == 
	    (PTHREAD_CANCEL_ENABLE|_PTHREAD_CANCEL_PENDING))
	{
		UNLOCK(thread->lock);
		if (isconforming)
			pthread_exit(PTHREAD_CANCELED);
		else
			pthread_exit(0);
	}
	UNLOCK(thread->lock);
}



int
pthread_getconcurrency(void)
{
	return(pthread_concurrency);
}

int
pthread_setconcurrency(int new_level)
{
	if (new_level < 0)
		return EINVAL;
	pthread_concurrency = new_level;
	return(0);
}

/*
 * Perform package initialization - called automatically when application starts
 */
int
pthread_init(void)
{
	pthread_attr_t *attrs;
        pthread_t thread;
	kern_return_t kr;
	host_priority_info_data_t priority_info;
	host_info_t info;
	host_flavor_t flavor;
	host_t host;
	mach_msg_type_number_t count;
	int mib[2];
	int ncpus = 0;
	size_t len;
	void *stackaddr;

	pthreadsize = round_page(sizeof (struct _pthread));
	count = HOST_PRIORITY_INFO_COUNT;
	info = (host_info_t)&priority_info;
	flavor = HOST_PRIORITY_INFO;
	host = mach_host_self();
	kr = host_info(host, flavor, info, &count);
        if (kr != KERN_SUCCESS)
                printf("host_info failed (%d); probably need privilege.\n", kr);
        else {
		default_priority = priority_info.user_priority;
		min_priority = priority_info.minimum_priority;
		max_priority = priority_info.maximum_priority;
	}
	attrs = &_pthread_attr_default;
	pthread_attr_init(attrs);

	TAILQ_INIT(&__pthread_head);
	LOCK_INIT(_pthread_list_lock);
	thread = &_thread;
	TAILQ_INSERT_HEAD(&__pthread_head, thread, plist);
	_pthread_set_self(thread);
#if PTH_LISTTRACE
	__kdebug_trace(0x900000c, thread, 0, 0, 10, 0);
#endif
	
	/* In case of dyld reset the tsd keys from 1 - 10 */
	_pthread_keys_init();

    	mib[0] = CTL_KERN;
    	mib[1] = KERN_USRSTACK;
    	len = sizeof (stackaddr);
    	if (sysctl (mib, 2, &stackaddr, &len, NULL, 0) != 0)
       		stackaddr = (void *)USRSTACK;
	_pthread_create(thread, attrs, stackaddr, mach_thread_self());
	thread->stacksize = DFLSSIZ; //initialize main thread's stacksize based on vmparam.h
	thread->detached = PTHREAD_CREATE_JOINABLE|_PTHREAD_CREATE_PARENT;

	_init_cpu_capabilities();
	if ((ncpus = _NumCPUs()) > 1)
		_spin_tries = MP_SPIN_TRIES;

	workq_targetconc[WORKQ_HIGH_PRIOQUEUE] = ncpus;
	workq_targetconc[WORKQ_DEFAULT_PRIOQUEUE] = ncpus;
	workq_targetconc[WORKQ_LOW_PRIOQUEUE] =  ncpus;
	workq_targetconc[WORKQ_BG_PRIOQUEUE] =  ncpus;

	mach_port_deallocate(mach_task_self(), host);
    
#if defined(__ppc__)
	IF_ROSETTA() {
		__oldstyle = 1;
	}
#endif

#if defined(_OBJC_PAGE_BASE_ADDRESS)
{
        vm_address_t objcRTPage = (vm_address_t)_OBJC_PAGE_BASE_ADDRESS;
        kr = vm_map(mach_task_self(),
                 &objcRTPage, vm_page_size * 4, vm_page_size - 1,
                 VM_FLAGS_FIXED | VM_MAKE_TAG(0), // Which tag to use?
                 MACH_PORT_NULL,
                 (vm_address_t)0, FALSE,
                 (vm_prot_t)0, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE,
                 VM_INHERIT_DEFAULT);
		/* We ignore the return result here. The ObjC runtime will just have to deal. */
}
#endif
	//added so that thread_recycle_port is initialized on new launch.
	_pthread_fork_child_postinit();
	mig_init(1);		/* enable multi-threaded mig interfaces */
	if (__oldstyle == 0) {
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) 
		__bsdthread_register(thread_start, start_wqthread, round_page(sizeof(struct _pthread)), _pthread_start, &workq_targetconc[0], (uintptr_t)(&thread->tsd[__PTK_LIBDISPATCH_KEY0]) - (uintptr_t)(&thread->tsd[0]));
#else
		__bsdthread_register(_pthread_start, _pthread_wqthread, round_page(sizeof(struct _pthread)), NULL, &workq_targetconc[0], (uintptr_t)&thread->tsd[__PTK_LIBDISPATCH_KEY0] - (uintptr_t)thread);
#endif
	}

#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) 
	if( (thread->thread_id = __thread_selfid()) == (__uint64_t)-1)
		printf("Failed to set thread_id in pthread_init\n");
#endif
	return 0;
}

int sched_yield(void)
{
    swtch_pri(0);
    return 0;
}

/* This used to be the "magic" that gets the initialization routine called when the application starts */
/*
 * (These has been moved to setenv.c, so we can use it to fix a less than 10.5
 * crt1.o issue)
 * static int _do_nothing(void) { return 0; }
 * int (*_cthread_init_routine)(void) = _do_nothing;
 */

/* Get a semaphore from the pool, growing it if necessary */

__private_extern__ semaphore_t new_sem_from_pool(void) {
	kern_return_t res;
	semaphore_t sem;
        int i;
        
	LOCK(sem_pool_lock);
	if (sem_pool_current == sem_pool_count) {
		sem_pool_count += 16;
		sem_pool = realloc(sem_pool, sem_pool_count * sizeof(semaphore_t));
		for (i = sem_pool_current; i < sem_pool_count; i++) {
			PTHREAD_MACH_CALL(semaphore_create(mach_task_self(), &sem_pool[i], SYNC_POLICY_FIFO, 0), res);
		}
	}
	sem = sem_pool[sem_pool_current++];
	UNLOCK(sem_pool_lock);
	return sem;
}

/* Put a semaphore back into the pool */
__private_extern__ void restore_sem_to_pool(semaphore_t sem) {
	LOCK(sem_pool_lock);
	sem_pool[--sem_pool_current] = sem;
	UNLOCK(sem_pool_lock);
}

static void sem_pool_reset(void) {
	LOCK(sem_pool_lock);
	sem_pool_count = 0;
	sem_pool_current = 0;
	sem_pool = NULL; 
	UNLOCK(sem_pool_lock);
}

__private_extern__ void _pthread_fork_child(pthread_t p) {
	/* Just in case somebody had it locked... */
	UNLOCK(sem_pool_lock);
	sem_pool_reset();
	/* No need to hold the pthread_list_lock as no one other than this 
	 * thread is present at this time
	 */
	TAILQ_INIT(&__pthread_head);
	LOCK_INIT(_pthread_list_lock);
	TAILQ_INSERT_HEAD(&__pthread_head, p, plist);
#if PTH_LISTTRACE
	__kdebug_trace(0x900000c, p, 0, 0, 10, 0);
#endif
	_pthread_count = 1;
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) 
	if( (p->thread_id = __thread_selfid()) == (__uint64_t)-1)
		printf("Failed to set thread_id in pthread_fork_child\n");
#endif
}

void _pthread_fork_child_postinit() {
       kern_return_t kr;

       kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &thread_recycle_port);
       if (kr != KERN_SUCCESS) {
               abort();
       }
}

/*
 * Query/update the cancelability 'state' of a thread
 */
int
_pthread_setcancelstate_internal(int state, int *oldstate, int conforming)
{
	pthread_t self = pthread_self();


	switch (state) {
		case PTHREAD_CANCEL_ENABLE:
		if (conforming)
			__pthread_canceled(1);
			break;
		case PTHREAD_CANCEL_DISABLE:
		if (conforming)
			__pthread_canceled(2);
			break;
		default:
			return EINVAL;
	}

	self = pthread_self();
	LOCK(self->lock);
	if (oldstate)
		*oldstate = self->cancel_state & _PTHREAD_CANCEL_STATE_MASK;
	self->cancel_state &= ~_PTHREAD_CANCEL_STATE_MASK;
	self->cancel_state |= state;
	UNLOCK(self->lock);
	if (!conforming)
		_pthread_testcancel(self, 0);  /* See if we need to 'die' now... */
	return (0);
}

/* When a thread exits set the cancellation state to DISABLE and DEFERRED */
static void
_pthread_setcancelstate_exit(pthread_t self, void * value_ptr, int conforming)
{
	LOCK(self->lock);
	self->cancel_state &= ~(_PTHREAD_CANCEL_STATE_MASK | _PTHREAD_CANCEL_TYPE_MASK);
	self->cancel_state |= (PTHREAD_CANCEL_DISABLE | PTHREAD_CANCEL_DEFERRED);
	if ((value_ptr == PTHREAD_CANCELED)) {
// 4597450: begin
		self->detached |= _PTHREAD_WASCANCEL;
// 4597450: end
	}
	UNLOCK(self->lock);
}

int
_pthread_join_cleanup(pthread_t thread, void ** value_ptr, int conforming)
{
	kern_return_t res;
	int ret;

#if PTH_TRACE
	__kdebug_trace(0x9000028, thread, 0, 0, 1, 0);
#endif
	/* The scenario where the joiner was waiting for the thread and
	 * the pthread detach happened on that thread. Then the semaphore
	 * will trigger but by the time joiner runs, the target thread could be
	 * freed. So we need to make sure that the thread is still in the list 
	 * and is joinable  before we continue with the join.
	 */
	LOCK(_pthread_list_lock);
	if ((ret = _pthread_find_thread(thread)) != 0) {
		UNLOCK(_pthread_list_lock);
		/* returns ESRCH */
		return(ret);
	}
	if ((thread->detached & PTHREAD_CREATE_JOINABLE) == 0) {
		/* the thread might be a detached thread */
		UNLOCK(_pthread_list_lock);
		return(ESRCH);

	}
	/* It is still a joinable thread and needs to be reaped */
	TAILQ_REMOVE(&__pthread_head, thread, plist);
#if PTH_LISTTRACE
	__kdebug_trace(0x9000010, thread, 0, 0, 3, 0);
#endif
	UNLOCK(_pthread_list_lock);

	if (value_ptr)
		*value_ptr = thread->exit_value;
	if (conforming) {
		if ((thread->cancel_state & (PTHREAD_CANCEL_ENABLE|_PTHREAD_CANCEL_PENDING)) ==
			(PTHREAD_CANCEL_ENABLE|_PTHREAD_CANCEL_PENDING) && (value_ptr != NULL)) {
			*value_ptr = PTHREAD_CANCELED;
		}
	}
	if (thread->reply_port != MACH_PORT_NULL) {
		res = mach_port_mod_refs(mach_task_self(), thread->reply_port, MACH_PORT_RIGHT_RECEIVE, -1);
		if (res != KERN_SUCCESS) 
			fprintf(stderr,"mach_port_mod_refs(reply_port) failed: %s\n",mach_error_string(res));
		thread->reply_port = MACH_PORT_NULL;
	}
	if (thread->freeStackOnExit) {
		thread->sig = _PTHREAD_NO_SIG;
#if PTH_TRACE
	__kdebug_trace(0x9000028, thread, 0, 0, 2, 0);
#endif
		vm_deallocate(mach_task_self(), (mach_vm_address_t)(uintptr_t)thread, pthreadsize);
	} else {
		thread->sig = _PTHREAD_NO_SIG;
#if PTH_TRACE
	__kdebug_trace(0x9000028, thread, 0, 0, 3, 0);
#endif
		free(thread);
	}
	return(0);
}

/* ALWAYS called with list lock and return with list lock */
int
_pthread_find_thread(pthread_t thread)
{
	pthread_t p;

loop:
	TAILQ_FOREACH(p, &__pthread_head, plist) {
		if (p == thread) {
			if (thread->kernel_thread == MACH_PORT_NULL) {
				UNLOCK(_pthread_list_lock);
				sched_yield();
				LOCK(_pthread_list_lock);
				goto loop;
			} 
			return(0);
		}
	}
	return(ESRCH);
}

int
_pthread_lookup_thread(pthread_t thread, mach_port_t * portp, int only_joinable)
{
	mach_port_t kport;
	int ret = 0;

	if (thread == NULL)
		return(ESRCH);
	
	LOCK(_pthread_list_lock);

	if ((ret = _pthread_find_thread(thread)) != 0) {
		UNLOCK(_pthread_list_lock);
		return(ret);
	}
	if ((only_joinable != 0) && ((thread->detached & PTHREAD_CREATE_DETACHED) != 0)) {
		UNLOCK(_pthread_list_lock);
		return(EINVAL);
	} 
	kport = thread->kernel_thread;
	UNLOCK(_pthread_list_lock);
	if (portp != NULL)
		*portp = kport;
	return(0);
}

/* XXXXXXXXXXXXX Pthread Workqueue Attributes XXXXXXXXXXXXXXXXXX */
int 
pthread_workqueue_attr_init_np(pthread_workqueue_attr_t * attrp)
{
	attrp->queueprio = WORKQ_DEFAULT_PRIOQUEUE;
	attrp->sig = PTHREAD_WORKQUEUE_ATTR_SIG;
	attrp->overcommit = 0;
	return(0);
}

int 
pthread_workqueue_attr_destroy_np(pthread_workqueue_attr_t * attr)
{
	if (attr->sig == PTHREAD_WORKQUEUE_ATTR_SIG)
	{
		return (0);
	} else
	{
		return (EINVAL); /* Not an attribute structure! */
	}
}

int 
pthread_workqueue_attr_getqueuepriority_np(const pthread_workqueue_attr_t * attr, int * qpriop)
{
    if (attr->sig == PTHREAD_WORKQUEUE_ATTR_SIG) {
        *qpriop = attr->queueprio;
        return (0);
    } else {
        return (EINVAL); /* Not an attribute structure! */
    }
}


int 
pthread_workqueue_attr_setqueuepriority_np(pthread_workqueue_attr_t * attr, int qprio)
{
int error = 0;

    if (attr->sig == PTHREAD_WORKQUEUE_ATTR_SIG) {
		switch(qprio) {
			case WORKQ_HIGH_PRIOQUEUE:
			case WORKQ_DEFAULT_PRIOQUEUE:
			case WORKQ_LOW_PRIOQUEUE:
			case WORKQ_BG_PRIOQUEUE:
        			attr->queueprio = qprio;
				break;
			default:
				error = EINVAL;
		}
    } else {
	error = EINVAL;
    }
	return (error);
}


int 
pthread_workqueue_attr_getovercommit_np(const pthread_workqueue_attr_t * attr, int * ocommp)
{
    if (attr->sig == PTHREAD_WORKQUEUE_ATTR_SIG) {
        *ocommp = attr->overcommit;
        return (0);
    } else {
        return (EINVAL); /* Not an attribute structure! */
    }
}


int 
pthread_workqueue_attr_setovercommit_np(pthread_workqueue_attr_t * attr, int ocomm)
{
int error = 0;

    if (attr->sig == PTHREAD_WORKQUEUE_ATTR_SIG) {
	attr->overcommit = ocomm;
    } else {
	error = EINVAL;
    }
	return (error);
}
/* XXXXXXXXXXXXX Pthread Workqueue support routines XXXXXXXXXXXXXXXXXX */

static void
workqueue_list_lock()
{
	OSSpinLockLock(&__workqueue_list_lock);
}

static void
workqueue_list_unlock()
{
	OSSpinLockUnlock(&__workqueue_list_lock);
}

int
pthread_workqueue_init_np()
{
	int ret;

	if (__workqueue_newspis != 0)
		return(EPERM);
	__workqueue_oldspis = 1;	

	workqueue_list_lock();
	ret =_pthread_work_internal_init();
	workqueue_list_unlock();

	return(ret);
}

int
pthread_workqueue_requestconcurrency_np(int queue, int request_concurrency)
{
	int error = 0;

	if (__workqueue_newspis != 0)
		return(EPERM);

	if (queue < 0 || queue > WORKQ_NUM_PRIOQUEUE)
		return(EINVAL);

	error =__workq_kernreturn(WQOPS_THREAD_SETCONC, NULL, request_concurrency, queue);

	if (error == -1)
		return(errno);
	return(0);
}

void
pthread_workqueue_atfork_prepare(void)
{
	/* 
	 * NOTE:  Any workq additions here  
	 * should be for i386,x86_64 only 
	 */
	dispatch_atfork_prepare();
}

void
pthread_workqueue_atfork_parent(void)
{
	/* 
	 * NOTE:  Any workq additions here  
	 * should be for i386,x86_64 only 
	 */
	dispatch_atfork_parent();
}

void
pthread_workqueue_atfork_child(void)
{
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) 
	pthread_t self = pthread_self();

	__workqueue_list_lock = OS_SPINLOCK_INIT;

	/* already using new spis? */
	if (__workqueue_newspis != 0) {
		/* prepare the kernel for workq action */
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) 
		__bsdthread_register(thread_start, start_wqthread, round_page(sizeof(struct _pthread)), _pthread_start, &workq_targetconc[0], (uintptr_t)(&self->tsd[__PTK_LIBDISPATCH_KEY0]) - (uintptr_t)(&self->tsd[0]));
#else
		__bsdthread_register(_pthread_start, _pthread_wqthread, round_page(sizeof(struct _pthread)),NULL,NULL,0);
#endif
		(void)__workq_open();
		kernel_workq_setup = 1;
		return;
	}

	/* not using old spis either? */
	if (__workqueue_oldspis == 0)
		return;

	/* 
	 * NOTE:  workq additions here  
	 * are for i386,x86_64 only as
	 * ppc and arm do not support it
	 */
	if (kernel_workq_setup != 0){
	   kernel_workq_setup = 0;
	   _pthread_work_internal_init();
	}
#endif
	dispatch_atfork_child();
}

static int
_pthread_work_internal_init(void)
{
	int i, error;
	pthread_workqueue_head_t headp;
	pthread_workqueue_t wq;
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) 
	pthread_t self = pthread_self();
#endif

	if (kernel_workq_setup == 0) {
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) 
		__bsdthread_register(thread_start, start_wqthread, round_page(sizeof(struct _pthread)), _pthread_start, &workq_targetconc[0], (uintptr_t)(&self->tsd[__PTK_LIBDISPATCH_KEY0]) - (uintptr_t)(&self->tsd[0]));
#else
		__bsdthread_register(_pthread_start, _pthread_wqthread, round_page(sizeof(struct _pthread)),NULL,NULL,0);
#endif

		_pthread_wq_attr_default.queueprio = WORKQ_DEFAULT_PRIOQUEUE;
		_pthread_wq_attr_default.sig = PTHREAD_WORKQUEUE_ATTR_SIG;

		for( i = 0; i< WORKQ_NUM_PRIOQUEUE; i++) {
			headp = __pthread_wq_head_tbl[i];
			TAILQ_INIT(&headp->wqhead);
			headp->next_workq = 0;
		}

		__workqueue_pool_ptr = NULL;
		__workqueue_pool_size = round_page(sizeof(struct _pthread_workitem) * WORKITEM_POOL_SIZE);

		__workqueue_pool_ptr = (struct _pthread_workitem *)mmap(NULL, __workqueue_pool_size,
			PROT_READ|PROT_WRITE,
			MAP_ANON | MAP_PRIVATE,
			0,
			0);

		if (__workqueue_pool_ptr == MAP_FAILED) {
			/* Not expected to fail, if it does, always malloc for work items */
			__workqueue_nitems = WORKITEM_POOL_SIZE;
			__workqueue_pool_ptr = NULL;
		} else 
			__workqueue_nitems = 0;

		/* sets up the workitem pool */
		grow_workitem();

		/* since the size is less than a page, leaving this in malloc pool */
		wq = (struct _pthread_workqueue *)malloc(sizeof(struct _pthread_workqueue) * WORKQUEUE_POOL_SIZE);
		bzero(wq, (sizeof(struct _pthread_workqueue) * WORKQUEUE_POOL_SIZE));
		for (i = 0; i < WORKQUEUE_POOL_SIZE; i++) {
			TAILQ_INSERT_TAIL(&__pthread_workqueue_pool_head, &wq[i], wq_list);
		}

		if (error = __workq_open()) {
			TAILQ_INIT(&__pthread_workitem_pool_head);
			TAILQ_INIT(&__pthread_workqueue_pool_head);
			if (__workqueue_pool_ptr != NULL) {
				munmap((void *)__workqueue_pool_ptr, __workqueue_pool_size);
			}
			free(wq);
			return(ENOMEM);
		}
		kernel_workq_setup = 1;
	}
	return(0);
}


/* This routine is called with list lock held */
static pthread_workitem_t
alloc_workitem(void)
{
	pthread_workitem_t witem;

	if (TAILQ_EMPTY(&__pthread_workitem_pool_head)) {
		/* the chunk size is set so some multiple of it is pool size */
		if (__workqueue_nitems < WORKITEM_POOL_SIZE) {
			grow_workitem();
		} else {
			workqueue_list_unlock();
			witem = malloc(sizeof(struct _pthread_workitem));
			workqueue_list_lock();
			witem->fromcache = 0;
			goto out;
		}
	}
	witem = TAILQ_FIRST(&__pthread_workitem_pool_head);
	TAILQ_REMOVE(&__pthread_workitem_pool_head, witem, item_entry);
	witem->fromcache = 1;
out:
	witem->flags = 0;
	witem->item_entry.tqe_next = 0;
	witem->item_entry.tqe_prev = 0;
	user_workitem_count++;
	return(witem);
}

/* This routine is called with list lock held */
static void
free_workitem(pthread_workitem_t witem) 
{
	user_workitem_count--;
	witem->flags = 0;
	if (witem->fromcache != 0)
		TAILQ_INSERT_TAIL(&__pthread_workitem_pool_head, witem, item_entry);
	else
		free(witem);
}

static void
grow_workitem(void)
{
	pthread_workitem_t witemp;
	int i;

	 witemp = &__workqueue_pool_ptr[__workqueue_nitems];
	bzero(witemp, (sizeof(struct _pthread_workitem) * WORKITEM_CHUNK_SIZE));
	for (i = 0; i < WORKITEM_CHUNK_SIZE; i++) {
		witemp[i].fromcache = 1;
		TAILQ_INSERT_TAIL(&__pthread_workitem_pool_head, &witemp[i], item_entry);
	}
	__workqueue_nitems += WORKITEM_CHUNK_SIZE;
}

/* This routine is called with list lock held */
static pthread_workqueue_t
alloc_workqueue(void)
{
	pthread_workqueue_t wq;

	if (TAILQ_EMPTY(&__pthread_workqueue_pool_head)) {
		workqueue_list_unlock();
		wq = malloc(sizeof(struct _pthread_workqueue));
		workqueue_list_lock();
	} else {
		wq = TAILQ_FIRST(&__pthread_workqueue_pool_head);
		TAILQ_REMOVE(&__pthread_workqueue_pool_head, wq, wq_list);
	}
	user_workq_count++;
	return(wq);
}

/* This routine is called with list lock held */
static void
free_workqueue(pthread_workqueue_t wq) 
{
	user_workq_count--;
	TAILQ_INSERT_TAIL(&__pthread_workqueue_pool_head, wq, wq_list);
}

static void
_pthread_workq_init(pthread_workqueue_t wq, const pthread_workqueue_attr_t * attr)
{
	bzero(wq, sizeof(struct _pthread_workqueue));
	if (attr != NULL) {
		wq->queueprio = attr->queueprio;
		wq->overcommit = attr->overcommit;
	 } else {
		wq->queueprio = WORKQ_DEFAULT_PRIOQUEUE;
		wq->overcommit = 0;
	}
	LOCK_INIT(wq->lock);
	wq->flags = 0;
	TAILQ_INIT(&wq->item_listhead);
	TAILQ_INIT(&wq->item_kernhead);
#if WQ_LISTTRACE
	__kdebug_trace(0x90080ac, wq, &wq->item_listhead, wq->item_listhead.tqh_first,  wq->item_listhead.tqh_last, 0);
#endif
	wq->wq_list.tqe_next = 0;
	wq->wq_list.tqe_prev = 0;
	wq->sig = PTHREAD_WORKQUEUE_SIG;
	wq->headp = __pthread_wq_head_tbl[wq->queueprio];
}

int 
valid_workq(pthread_workqueue_t workq)
{
	if (workq->sig == PTHREAD_WORKQUEUE_SIG) 
		return(1);
	else
		return(0);
}


/* called with list lock */
static void
pick_nextworkqueue_droplock()
{
	int i, curwqprio, val, found;
	pthread_workqueue_head_t headp;
	pthread_workqueue_t workq;
	pthread_workqueue_t nworkq = NULL;

#if WQ_TRACE
	__kdebug_trace(0x9008098, kernel_workq_count, 0, 0,  0, 0);
#endif
loop:
	while (kernel_workq_count < KERNEL_WORKQ_ELEM_MAX) {
		found = 0;
		for (i = 0; i < WORKQ_NUM_PRIOQUEUE; i++)  {
			wqreadyprio = i;	/* because there is nothing else higher to run */
			headp = __pthread_wq_head_tbl[i];

			if (TAILQ_EMPTY(&headp->wqhead))
				continue;
			workq = headp->next_workq;
			if (workq == NULL)
				workq = TAILQ_FIRST(&headp->wqhead);
			curwqprio = workq->queueprio;
			nworkq = workq;	/* starting pt */
			while (kernel_workq_count < KERNEL_WORKQ_ELEM_MAX) {
				headp->next_workq = TAILQ_NEXT(workq, wq_list);
				if (headp->next_workq == NULL)
					headp->next_workq = TAILQ_FIRST(&headp->wqhead);
#if WQ_TRACE
	__kdebug_trace(0x9008098, kernel_workq_count, workq, 0,  1, 0);
#endif
				val = post_nextworkitem(workq);

				if (val != 0) {
					/* things could have changed so reasses */
					/* If kernel queue is full , skip */
					if (kernel_workq_count >= KERNEL_WORKQ_ELEM_MAX)
						break;
					/* If anything with higher prio arrived, then reevaluate */
					if (wqreadyprio < curwqprio)
						goto loop;	/* we need re evaluate again */
					/* we can post some more work items */
					found = 1;
				}

				/* cannot use workq here as it could be freed */
				if (TAILQ_EMPTY(&headp->wqhead))
					break;
				/* if we found nothing to run and only one workqueue in the list, skip */
				if ((val == 0) && (workq == headp->next_workq))
					break;
				workq = headp->next_workq;
				if (workq == NULL)
					workq = TAILQ_FIRST(&headp->wqhead);
				if (val != 0)
					nworkq = workq;
				/* if we found nothing to run and back to workq where we started */
				if ((val == 0) && (workq == nworkq))
					break;
			}
			if (kernel_workq_count >= KERNEL_WORKQ_ELEM_MAX)
				break;
		}
		/* nothing found to run?  */
		if (found == 0)
			break;
	}
	workqueue_list_unlock();
}

static int
post_nextworkitem(pthread_workqueue_t workq)
{
	int error, prio;
	pthread_workitem_t witem;
	pthread_workqueue_head_t headp;
	void (*func)(pthread_workqueue_t, void *);

		if ((workq->flags & PTHREAD_WORKQ_SUSPEND) == PTHREAD_WORKQ_SUSPEND) {
			return(0);
		}
#if WQ_TRACE
		__kdebug_trace(0x900809c, workq, workq->item_listhead.tqh_first, 0, 1, 0);
#endif
		if (TAILQ_EMPTY(&workq->item_listhead)) {
			return(0);
		}
		if ((workq->flags & PTHREAD_WORKQ_BARRIER_ON) ==  PTHREAD_WORKQ_BARRIER_ON)
				return(0);

		witem = TAILQ_FIRST(&workq->item_listhead);
		headp = workq->headp;
#if WQ_TRACE
		__kdebug_trace(0x900809c, workq, witem, 0, 0xee, 0);
#endif
		if ((witem->flags & PTH_WQITEM_BARRIER) == PTH_WQITEM_BARRIER) {
#if WQ_TRACE
			__kdebug_trace(0x9000064, workq, 0, 0, 2, 0);
#endif
			
			if ((witem->flags & PTH_WQITEM_APPLIED) != 0) {
				return(0);
			}
			/* Also barrier when nothing is there needs to be handled */
			/* Nothing to wait for */
			if (workq->kq_count != 0) {
				witem->flags |= PTH_WQITEM_APPLIED;
				workq->flags |= PTHREAD_WORKQ_BARRIER_ON;
				workq->barrier_count = workq->kq_count;
#if WQ_TRACE
			__kdebug_trace(0x9000064, 1, workq->barrier_count, 0, 0, 0);
#endif
				return(1);
			} else {
#if WQ_TRACE
			__kdebug_trace(0x9000064, 2, workq->barrier_count, 0, 0, 0);
#endif
				if (witem->func != NULL) {
				/* since we are going to drop list lock */
				witem->flags |= PTH_WQITEM_APPLIED;
				workq->flags |= PTHREAD_WORKQ_BARRIER_ON;
				        workqueue_list_unlock();
					func = (void (*)(pthread_workqueue_t, void *))witem->func;
					(*func)(workq, witem->func_arg);
#if WQ_TRACE
			__kdebug_trace(0x9000064, 3, workq->barrier_count, 0, 0, 0);
#endif
					workqueue_list_lock();
					workq->flags &= ~PTHREAD_WORKQ_BARRIER_ON;
				}
				TAILQ_REMOVE(&workq->item_listhead, witem, item_entry);
#if WQ_LISTTRACE
	__kdebug_trace(0x90080a8, workq, &workq->item_listhead, workq->item_listhead.tqh_first,  workq->item_listhead.tqh_last, 0);
#endif
				free_workitem(witem);
#if WQ_TRACE
			__kdebug_trace(0x9000064, 4, workq->barrier_count, 0, 0, 0);
#endif
				return(1);
			}
		} else if ((witem->flags & PTH_WQITEM_DESTROY) == PTH_WQITEM_DESTROY) {
#if WQ_TRACE
			__kdebug_trace(0x9000068, 1, workq->kq_count, 0, 0, 0);
#endif
			if ((witem->flags & PTH_WQITEM_APPLIED) != 0) {
				return(0);
			}
			witem->flags |= PTH_WQITEM_APPLIED;
			workq->flags |= (PTHREAD_WORKQ_BARRIER_ON | PTHREAD_WORKQ_TERM_ON);
			workq->barrier_count = workq->kq_count;
			workq->term_callback = (void (*)(struct _pthread_workqueue *,void *))witem->func;
			workq->term_callarg = witem->func_arg;
			TAILQ_REMOVE(&workq->item_listhead, witem, item_entry);
#if WQ_LISTTRACE
	__kdebug_trace(0x90080a8, workq, &workq->item_listhead, workq->item_listhead.tqh_first,  workq->item_listhead.tqh_last, 0);
#endif
			if ((TAILQ_EMPTY(&workq->item_listhead)) && (workq->kq_count == 0)) {
				if (!(TAILQ_EMPTY(&workq->item_kernhead))) {
#if WQ_TRACE
				__kdebug_trace(0x900006c, workq, workq->kq_count, 0, 0xff, 0);
#endif
				}
				free_workitem(witem);
				workq->flags |= PTHREAD_WORKQ_DESTROYED;
#if WQ_TRACE
			__kdebug_trace(0x900006c, workq, workq->kq_count, 0, 1, 0);
#endif
				headp = __pthread_wq_head_tbl[workq->queueprio];
				if (headp->next_workq == workq) {
					headp->next_workq = TAILQ_NEXT(workq, wq_list);
					if (headp->next_workq == NULL) {
						headp->next_workq = TAILQ_FIRST(&headp->wqhead);
						if (headp->next_workq == workq)
							headp->next_workq = NULL;
					}
				}
				workq->sig = 0;
				TAILQ_REMOVE(&headp->wqhead, workq, wq_list);
				if (workq->term_callback != NULL) {
				        workqueue_list_unlock();
					(*workq->term_callback)(workq, workq->term_callarg);
					workqueue_list_lock();
				}
				free_workqueue(workq);
				return(1);
			} else  {
				TAILQ_INSERT_HEAD(&workq->item_listhead, witem, item_entry);
#if WQ_LISTTRACE
	__kdebug_trace(0x90080b0, workq, &workq->item_listhead, workq->item_listhead.tqh_first,  workq->item_listhead.tqh_last, 0);
#endif
			}
#if WQ_TRACE
			__kdebug_trace(0x9000068, 2, workq->barrier_count, 0, 0, 0);
#endif
			return(1);
		}  else {
#if WQ_TRACE
			__kdebug_trace(0x9000060, witem, workq, witem->func_arg, 0xfff, 0);
#endif
			TAILQ_REMOVE(&workq->item_listhead, witem, item_entry);
#if WQ_LISTTRACE
	__kdebug_trace(0x90080a8, workq, &workq->item_listhead, workq->item_listhead.tqh_first,  workq->item_listhead.tqh_last, 0);
#endif
			TAILQ_INSERT_TAIL(&workq->item_kernhead, witem, item_entry);
			if ((witem->flags & PTH_WQITEM_KERN_COUNT) == 0) {
				workq->kq_count++;
				witem->flags |= PTH_WQITEM_KERN_COUNT;
			}
			OSAtomicIncrement32Barrier(&kernel_workq_count);
			workqueue_list_unlock();
			
			prio = workq->queueprio;
			if (workq->overcommit != 0) {
				prio |= WORKQUEUE_OVERCOMMIT;
			}

			if (( error =__workq_kernreturn(WQOPS_QUEUE_ADD, witem, workq->affinity, prio)) == -1) {
				OSAtomicDecrement32Barrier(&kernel_workq_count);
				workqueue_list_lock();
#if WQ_TRACE
			__kdebug_trace(0x900007c, witem, workq, witem->func_arg, workq->kq_count, 0);
#endif
				TAILQ_REMOVE(&workq->item_kernhead, witem, item_entry);
				TAILQ_INSERT_HEAD(&workq->item_listhead, witem, item_entry);
#if WQ_LISTTRACE
	__kdebug_trace(0x90080b0, workq, &workq->item_listhead, workq->item_listhead.tqh_first,  workq->item_listhead.tqh_last, 0);
#endif
				if ((workq->flags & (PTHREAD_WORKQ_BARRIER_ON | PTHREAD_WORKQ_TERM_ON)) != 0)
					workq->flags |= PTHREAD_WORKQ_REQUEUED;
			} else
				workqueue_list_lock();
#if WQ_TRACE
			__kdebug_trace(0x9000060, witem, workq, witem->func_arg, workq->kq_count, 0);
#endif
			return(1);
		}
		/* noone should come here */
#if 1
		printf("error in logic for next workitem\n");
		LIBC_ABORT("error in logic for next workitem");
#endif
		return(0);
}

void
_pthread_wqthread(pthread_t self, mach_port_t kport, void * stackaddr, pthread_workitem_t item, int reuse)
{
	int ret;
	pthread_attr_t *attrs = &_pthread_attr_default;
	pthread_workqueue_t workq;
#if WQ_DEBUG
	pthread_t pself;
#endif
	int thread_reuse = 0;
	int thread_priority = 0;
	int thread_newspi = 0;
	int thread_options = 0;

	if (reuse & WQ_FLAG_THREAD_NEWSPI) {
		thread_reuse = reuse & WQ_FLAG_THREAD_REUSE;
		if ((reuse & WQ_FLAG_THREAD_OVERCOMMIT) != 0)
			thread_options = WORKQ_ADDTHREADS_OPTION_OVERCOMMIT;
		thread_priority = reuse & WQ_FLAG_THREAD_PRIOMASK;
		thread_newspi = 1;
		workq = NULL;
	} else {
		thread_reuse = (reuse == 0)? 0: WQ_FLAG_THREAD_REUSE;
		workq = item->workq;
	}

	
	if (thread_reuse == 0) {
		/* reuse is set to 0, when a thread is newly created to run a workitem */
		_pthread_struct_init(self, attrs, stackaddr,  DEFAULT_STACK_SIZE, 1, 1);
		self->wqthread = 1;
		self->wqkillset = 0;
		self->parentcheck = 1;

		/* These are not joinable threads */
		self->detached &= ~PTHREAD_CREATE_JOINABLE;
		self->detached |= PTHREAD_CREATE_DETACHED;
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) 
		_pthread_set_self(self);
#endif
#if WQ_TRACE
		__kdebug_trace(0x9000050, self, item, item->func_arg, 0, 0);
#endif
		self->kernel_thread = kport;
		if (thread_newspi != 0) {
			self->fun = (void *(*)(void *))__libdispatch_workerfunction;
			self->arg = thread_priority;
		} else {
			self->fun = (void *(*)(void *))item->func;
			self->arg = item->func_arg;
		}
		/* Add to the pthread list */
		LOCK(_pthread_list_lock);
		TAILQ_INSERT_TAIL(&__pthread_head, self, plist);
#if PTH_LISTTRACE
		__kdebug_trace(0x900000c, self, 0, 0, 10, 0);
#endif
		_pthread_count++;
		UNLOCK(_pthread_list_lock);

#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) 
		if( (self->thread_id = __thread_selfid()) == (__uint64_t)-1)
			printf("Failed to set thread_id in pthread_wqthread\n");
#endif

	} else  {
		/* reuse is set to 1, when a thread is resued to run another work item */
#if WQ_TRACE
		__kdebug_trace(0x9000054, self, item, item->func_arg, 0, 0);
#endif
		/* reset all tsd from 1 to KEYS_MAX */
		if (self == NULL)
			LIBC_ABORT("_pthread_wqthread: pthread %p setup to be NULL", self);

		if (thread_newspi != 0) {
			self->fun = (void *(*)(void *))__libdispatch_workerfunction;
			self->arg = NULL;
		} else {
			self->fun = (void *(*)(void *))item->func;
			self->arg = item->func_arg;
		}
	}

#if WQ_DEBUG
	if (reuse == 0) {
		pself = pthread_self();
		if (self != pself) {
#if WQ_TRACE
		__kdebug_trace(0x9000078, self, pself, item->func_arg, 0, 0);
#endif
			printf("pthread_self not set: pself %p, passed in %p\n", pself, self);
    			_pthread_set_self(self);
			pself = pthread_self();
			if (self != pself)
				printf("(2)pthread_self not set: pself %p, passed in %p\n", pself, self);
			pself = self;
		}
	} else {
		pself = pthread_self();
		if (self != pself) {
			printf("(3)pthread_self not set in reuse: pself %p, passed in %p\n", pself, self);
			LIBC_ABORT("(3)pthread_self not set in reuse: pself %p, passed in %p", pself, self);
		}
	}
#endif /* WQ_DEBUG */

	if (thread_newspi != 0) {
		(*__libdispatch_workerfunction)(thread_priority, thread_options, NULL);
		_pthread_workq_return(self);
	 } else {
		self->cur_workq = workq;
		self->cur_workitem = item;
		OSAtomicDecrement32Barrier(&kernel_workq_count);

		ret = (int)(intptr_t)(*self->fun)(self->arg);
		/* If we reach here without going through the above initialization path then don't go through
		* with the teardown code path ( e.g. setjmp/longjmp ). Instead just exit this thread.
		*/
		if (self != pthread_self()) {
			pthread_exit(PTHREAD_CANCELED);
		}

		workqueue_exit(self, workq, item);
	}
}

static void
workqueue_exit(pthread_t self, pthread_workqueue_t workq, pthread_workitem_t item)
{
	pthread_workitem_t baritem;
	pthread_workqueue_head_t headp;
	void (*func)(pthread_workqueue_t, void *);
	
	workqueue_list_lock();

	TAILQ_REMOVE(&workq->item_kernhead, item, item_entry);
	workq->kq_count--;
#if WQ_TRACE
	__kdebug_trace(0x9000070, self, 1, item->func_arg, workq->kq_count, 0);
#endif
	free_workitem(item);

	if ((workq->flags & PTHREAD_WORKQ_BARRIER_ON) == PTHREAD_WORKQ_BARRIER_ON) {
		workq->barrier_count--;
#if WQ_TRACE
		__kdebug_trace(0x9000084, self, workq->barrier_count, workq->kq_count, 1, 0);
#endif
		if (workq->barrier_count <= 0 ) {
			/* Need to remove barrier item from the list */
			baritem = TAILQ_FIRST(&workq->item_listhead);
#if WQ_DEBUG
			if ((baritem->flags & (PTH_WQITEM_BARRIER | PTH_WQITEM_DESTROY| PTH_WQITEM_APPLIED)) == 0)
				printf("Incorect bar item being removed in barrier processing\n");
#endif /* WQ_DEBUG */
			/* if the front item is a barrier and call back is registered, run that */
			 if (((baritem->flags & PTH_WQITEM_BARRIER) == PTH_WQITEM_BARRIER) && (baritem->func != NULL)) {
				workqueue_list_unlock();
				func = (void (*)(pthread_workqueue_t, void *))baritem->func;
				(*func)(workq, baritem->func_arg);
				workqueue_list_lock();
			}
			TAILQ_REMOVE(&workq->item_listhead, baritem, item_entry);
#if WQ_LISTTRACE
	__kdebug_trace(0x90080a8, workq, &workq->item_listhead, workq->item_listhead.tqh_first,  workq->item_listhead.tqh_last, 0);
#endif
			free_workitem(baritem);
			workq->flags &= ~PTHREAD_WORKQ_BARRIER_ON;
#if WQ_TRACE
			__kdebug_trace(0x9000058, self, item, item->func_arg, 0, 0);
#endif
			if ((workq->flags & PTHREAD_WORKQ_TERM_ON) != 0) {
				headp = __pthread_wq_head_tbl[workq->queueprio];
				workq->flags |= PTHREAD_WORKQ_DESTROYED;
#if WQ_TRACE
			__kdebug_trace(0x900006c, workq, workq->kq_count, 0, 2, 0);
#endif
				if (headp->next_workq == workq) {
					headp->next_workq = TAILQ_NEXT(workq, wq_list);
					if (headp->next_workq == NULL) {
						headp->next_workq = TAILQ_FIRST(&headp->wqhead);
						if (headp->next_workq == workq)
							headp->next_workq = NULL;
					}
				}
				TAILQ_REMOVE(&headp->wqhead, workq, wq_list);
				workq->sig = 0;
				if (workq->term_callback != NULL) {
				        workqueue_list_unlock();
					(*workq->term_callback)(workq, workq->term_callarg);
					workqueue_list_lock();
				}
				free_workqueue(workq);
			} else {
			/* if there are higher prio schedulabel item reset to wqreadyprio */
				if ((workq->queueprio < wqreadyprio) && (!(TAILQ_EMPTY(&workq->item_listhead))))
						wqreadyprio = workq->queueprio;
			}
		}
	} 
#if WQ_TRACE
	else {
		__kdebug_trace(0x9000070, self, 2, item->func_arg, workq->barrier_count, 0);
	}
	
	__kdebug_trace(0x900005c, self, item, 0, 0, 0);
#endif
	pick_nextworkqueue_droplock();
	_pthread_workq_return(self);
}

static void 
_pthread_workq_return(pthread_t self)
{
	__workq_kernreturn(WQOPS_THREAD_RETURN, NULL, 0, 0);

	/* This is the way to terminate the thread */
	_pthread_exit(self, NULL);
}


/* XXXXXXXXXXXXX Pthread Workqueue functions XXXXXXXXXXXXXXXXXX */

int
pthread_workqueue_setdispatch_np(void (*worker_func)(int, int, void *))
{
	int error = 0;

	if (__workqueue_oldspis != 0)
		return(EPERM);

	__workqueue_newspis = 1;	

	if (__libdispatch_workerfunction == NULL) {
		__libdispatch_workerfunction = worker_func;
		/* check whether the kernel supports new SPIs */
		error = __workq_kernreturn(WQOPS_QUEUE_NEWSPISUPP, NULL, 0, 0);
		if (error == -1){
			__libdispatch_workerfunction = NULL;
			error = ENOTSUP;
			__workqueue_newspis = 0;
		} else  {
			/* prepare the kernel for workq action */
			(void)__workq_open();
			kernel_workq_setup = 1;
			if (__is_threaded == 0)
				__is_threaded = 1;
			__workqueue_newspis = 1;	
		}
	} else {
		error = EBUSY;
	}

	return(error);
}

int
pthread_workqueue_addthreads_np(int queue_priority, int options, int numthreads)
{
	int priority = queue_priority & WQ_FLAG_THREAD_PRIOMASK;
	int error = 0;

	/* new spi not inited yet?? */
	if (__workqueue_newspis == 0)
		return(EPERM);


	if ((options & WORKQ_ADDTHREADS_OPTION_OVERCOMMIT) != 0)
		priority |= WORKQUEUE_OVERCOMMIT;
		
		error = __workq_kernreturn(WQOPS_QUEUE_REQTHREADS, NULL, numthreads, priority);

		if (error == -1)
			return(errno);
		else
			return(0);
}

int 
pthread_workqueue_create_np(pthread_workqueue_t * workqp, const pthread_workqueue_attr_t * attr)
{
	pthread_workqueue_t wq;
	pthread_workqueue_head_t headp;

#if defined(__ppc__)
	IF_ROSETTA() {
		return(ENOTSUP);
	}
#endif
	if (__workqueue_newspis != 0)
		return(EPERM);

	if (__workqueue_oldspis == 0)
		__workqueue_oldspis = 1;	

	if ((attr != NULL) && (attr->sig != PTHREAD_WORKQUEUE_ATTR_SIG)) {
		return(EINVAL);
	}

	if (__is_threaded == 0)
		__is_threaded = 1;

	workqueue_list_lock();
	if (kernel_workq_setup == 0) {
		int ret = _pthread_work_internal_init();
		if (ret != 0) {
			workqueue_list_unlock();
			return(ret);
		}
	}
	
	wq = alloc_workqueue();
	
	_pthread_workq_init(wq, attr);

	headp = __pthread_wq_head_tbl[wq->queueprio];
	TAILQ_INSERT_TAIL(&headp->wqhead, wq, wq_list);
	if (headp->next_workq == NULL) {
		headp->next_workq = TAILQ_FIRST(&headp->wqhead);
	}

	workqueue_list_unlock();
	
	*workqp = wq;

	return(0);
}

int 
pthread_workqueue_additem_np(pthread_workqueue_t workq, void ( *workitem_func)(void *), void * workitem_arg, pthread_workitem_handle_t * itemhandlep, unsigned int *gencountp)
{
	pthread_workitem_t witem;

	if (__workqueue_newspis != 0)
		return(EPERM);

	if (valid_workq(workq) == 0) {
		return(EINVAL);
	}

	workqueue_list_lock();

	/*
	 * Allocate the workitem here as it can drop the lock.
	 * Also we can evaluate the workqueue state only once.
	 */
	witem = alloc_workitem();
	witem->func = workitem_func;
	witem->func_arg = workitem_arg;
	witem->workq = workq;

	/* alloc workitem can drop the lock, check the state  */
	if ((workq->flags & (PTHREAD_WORKQ_IN_TERMINATE | PTHREAD_WORKQ_DESTROYED)) != 0) {
		free_workitem(witem);
		workqueue_list_unlock();
		*itemhandlep = 0;
		return(ESRCH);
	}

	if (itemhandlep != NULL)
		*itemhandlep = (pthread_workitem_handle_t *)witem;
	if (gencountp != NULL)
		*gencountp = 0;
#if WQ_TRACE
	__kdebug_trace(0x9008090, witem, witem->func, witem->func_arg,  workq, 0);
#endif
	TAILQ_INSERT_TAIL(&workq->item_listhead, witem, item_entry);
#if WQ_LISTTRACE
	__kdebug_trace(0x90080a4, workq, &workq->item_listhead, workq->item_listhead.tqh_first,  workq->item_listhead.tqh_last, 0);
#endif
	
	if (((workq->flags & PTHREAD_WORKQ_BARRIER_ON) == 0) && (workq->queueprio < wqreadyprio))
			wqreadyprio = workq->queueprio;

	pick_nextworkqueue_droplock();

	return(0);
}

int
pthread_workqueue_getovercommit_np(pthread_workqueue_t workq,  unsigned int *ocommp)
{
	if (__workqueue_newspis != 0)
		return(EPERM);

        if (valid_workq(workq) == 0) {
                return(EINVAL);
        }

	if (ocommp != NULL)
		*ocommp = workq->overcommit;
	return(0);
}


#else /* !BUILDING_VARIANT ] [ */
extern int __unix_conforming;
extern int _pthread_count;
extern int __workqueue_newspis;
extern int __workqueue_oldspis;

extern pthread_lock_t _pthread_list_lock;
extern void _pthread_testcancel(pthread_t thread, int isconforming);
extern int _pthread_reap_thread(pthread_t th, mach_port_t kernel_thread, void **value_ptr, int conforming);

#endif /* !BUILDING_VARIANT ] */

#if __DARWIN_UNIX03

__private_extern__ void 
__posix_join_cleanup(void *arg)
{
	pthread_t thread = (pthread_t)arg;
	int already_exited, res;
	void * dummy;
	semaphore_t death;
	int newstyle;

	LOCK(thread->lock);
	already_exited = (thread->detached & _PTHREAD_EXITED);

	newstyle = thread->newstyle;
	
#if WQ_TRACE
	__kdebug_trace(0x900002c, thread, newstyle, 0, 0, 0);
#endif
	if (newstyle == 0) {
		death = thread->death;
		if (!already_exited){
			thread->joiner = (struct _pthread *)NULL;
			UNLOCK(thread->lock);
			restore_sem_to_pool(death);
		} else {
			UNLOCK(thread->lock);
			while ((res = _pthread_reap_thread(thread,
					thread->kernel_thread,
					&dummy, 1)) == EAGAIN)
				{
					sched_yield();
				}
			restore_sem_to_pool(death);

		}

	} else {
		/* leave another thread to join */
		thread->joiner = (struct _pthread *)NULL;
		UNLOCK(thread->lock);
	}
}

#endif /* __DARWIN_UNIX03 */


/*
 * Wait for a thread to terminate and obtain its exit value.
 */
/*
int       
pthread_join(pthread_t thread, 
	     void **value_ptr)

moved to pthread_cancelable.c */

/*
 * Cancel a thread
 */
int
pthread_cancel(pthread_t thread)
{
#if __DARWIN_UNIX03
	if (__unix_conforming == 0)
		__unix_conforming = 1;
#endif /* __DARWIN_UNIX03 */

	if (_pthread_lookup_thread(thread, NULL, 0) != 0)
		return(ESRCH);

	/* if the thread is a workqueue thread, then return error */
	if (thread->wqthread != 0) {
		return(ENOTSUP);
	}
#if __DARWIN_UNIX03
	int state;

	LOCK(thread->lock);
	state = thread->cancel_state |= _PTHREAD_CANCEL_PENDING;
	UNLOCK(thread->lock);
	if (state & PTHREAD_CANCEL_ENABLE)
		__pthread_markcancel(thread->kernel_thread);
#else /* __DARWIN_UNIX03 */
	thread->cancel_state |= _PTHREAD_CANCEL_PENDING;
#endif /* __DARWIN_UNIX03 */
	return (0);
}

void
pthread_testcancel(void)
{
	pthread_t self = pthread_self();

#if __DARWIN_UNIX03
	if (__unix_conforming == 0)
		__unix_conforming = 1;
	_pthread_testcancel(self, 1);
#else /* __DARWIN_UNIX03 */
	_pthread_testcancel(self, 0);
#endif /* __DARWIN_UNIX03 */

}


/*
 * Query/update the cancelability 'state' of a thread
 */
int
pthread_setcancelstate(int state, int *oldstate)
{
#if __DARWIN_UNIX03
	if (__unix_conforming == 0) {
		__unix_conforming = 1;
	}
	return (_pthread_setcancelstate_internal(state, oldstate, 1));
#else /* __DARWIN_UNIX03 */
	return (_pthread_setcancelstate_internal(state, oldstate, 0));
#endif /* __DARWIN_UNIX03 */

}



/*
 * Query/update the cancelability 'type' of a thread
 */
int
pthread_setcanceltype(int type, int *oldtype)
{
	pthread_t self = pthread_self();
	
#if __DARWIN_UNIX03
	if (__unix_conforming == 0)
		__unix_conforming = 1;
#endif /* __DARWIN_UNIX03 */

	if ((type != PTHREAD_CANCEL_DEFERRED) && 
	    (type != PTHREAD_CANCEL_ASYNCHRONOUS))
		return EINVAL;
	self = pthread_self();
	LOCK(self->lock);
	if (oldtype)
		*oldtype = self->cancel_state & _PTHREAD_CANCEL_TYPE_MASK;
	self->cancel_state &= ~_PTHREAD_CANCEL_TYPE_MASK;
	self->cancel_state |= type;
	UNLOCK(self->lock);
#if !__DARWIN_UNIX03
	_pthread_testcancel(self, 0);  /* See if we need to 'die' now... */
#endif /* __DARWIN_UNIX03 */
	return (0);
}

int 
pthread_sigmask(int how, const sigset_t * set, sigset_t * oset)
{
#if __DARWIN_UNIX03
	int err = 0;

	if (__pthread_sigmask(how, set, oset) == -1) {
		err = errno;
	} 
	return(err);
#else /* __DARWIN_UNIX03 */
	return(__pthread_sigmask(how, set, oset));
#endif /* __DARWIN_UNIX03 */
}

/*
int
sigwait(const sigset_t * set, int * sig)

moved to pthread_cancelable.c */

/*
 * Copyright (c) 2000-2003, 2007 Apple Inc. All rights reserved.
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
 * POSIX Threads - IEEE 1003.1c
 */

#ifndef _POSIX_PTHREAD_INTERNALS_H
#define _POSIX_PTHREAD_INTERNALS_H

// suppress pthread_attr_t typedef in sys/signal.h
#define _PTHREAD_ATTR_T
struct _pthread_attr_t; /* forward reference */
typedef struct _pthread_attr_t pthread_attr_t;

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <TargetConditionals.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <libkern/OSAtomic.h>


#ifndef __POSIX_LIB__
#define __POSIX_LIB__
#endif

#include "posix_sched.h"		/* For POSIX scheduling policy & parameter */
#include <sys/queue.h>		/* For POSIX scheduling policy & parameter */
#include "pthread_machdep.h"		/* Machine-dependent definitions. */
#include "pthread_spinlock.h"		/* spinlock definitions. */

TAILQ_HEAD(__pthread_list, _pthread);

extern int __pthread_lock_debug;
extern int __pthread_lock_old;

extern struct __pthread_list __pthread_head;        /* head of list of open files */
extern pthread_lock_t _pthread_list_lock;
extern  size_t pthreadsize;
/*
 * Compiled-in limits
 */
#if TARGET_OS_EMBEDDED
#define _EXTERNAL_POSIX_THREAD_KEYS_MAX 256
#define _INTERNAL_POSIX_THREAD_KEYS_MAX 256
#define _INTERNAL_POSIX_THREAD_KEYS_END 512
#else
#define _EXTERNAL_POSIX_THREAD_KEYS_MAX 512
#define _INTERNAL_POSIX_THREAD_KEYS_MAX 256
#define _INTERNAL_POSIX_THREAD_KEYS_END 768
#endif

/*
 * Threads
 */
#define MAXTHREADNAMESIZE	64
#define _PTHREAD_T
typedef struct _pthread
{
	long	       sig;	      /* Unique signature for this structure */
	struct __darwin_pthread_handler_rec *__cleanup_stack;
	pthread_lock_t lock;	      /* Used for internal mutex on structure */
	uint32_t	detached:8,
			inherit:8,
			policy:8,
			freeStackOnExit:1,
			newstyle:1,
			kernalloc:1,
			schedset:1,
			wqthread:1,
			wqkillset:1,
			pad:2;
	size_t	       guardsize;	/* size in bytes to guard stack overflow */
#if  !defined(__LP64__)
	int	       pad0;		/* for backwards compatibility */
#endif
	struct sched_param param;
	uint32_t	cancel_error;
#if defined(__LP64__)
	uint32_t	cancel_pad;	/* pad value for alignment */
#endif
	struct _pthread *joiner;
#if !defined(__LP64__)
	int		pad1;		/* for backwards compatibility */
#endif
	void           *exit_value;
	semaphore_t    death;		/* pthread_join() uses this to wait for death's call */
	mach_port_t    kernel_thread; /* kernel thread this thread is bound to */
	void	       *(*fun)(void*);/* Thread start routine */
        void	       *arg;	      /* Argment for thread start routine */
	int	       cancel_state;  /* Whether thread can be cancelled */
	int	       err_no;		/* thread-local errno */
	void	       *tsd[_EXTERNAL_POSIX_THREAD_KEYS_MAX + _INTERNAL_POSIX_THREAD_KEYS_MAX];  /* Thread specific data */
        void           *stackaddr;     /* Base of the stack (is aligned on vm_page_size boundary */
        size_t         stacksize;      /* Size of the stack (is a multiple of vm_page_size and >= PTHREAD_STACK_MIN) */
	mach_port_t    reply_port;     /* Cached MiG reply port */
#if defined(__LP64__)
        int		pad2;		/* for natural alignment */
#endif
	void           *cthread_self;  /* cthread_self() if somebody calls cthread_set_self() */
	/* protected by list lock */
	uint32_t 	childrun:1,
			parentcheck:1,
			childexit:1,
			pad3:29;
#if defined(__LP64__)
	int		pad4;		/* for natural alignment */
#endif
	TAILQ_ENTRY(_pthread) plist;
	void *	freeaddr;
	size_t	freesize;
	mach_port_t	joiner_notify;
	char	pthread_name[MAXTHREADNAMESIZE];		/* including nulll the name */
        int	max_tsd_key;
	void *	cur_workq;
	void * cur_workitem;
	uint64_t thread_id;
} *pthread_t;

/*
 * Thread attributes
 */
struct _pthread_attr_t
{
	long	       sig;	      /* Unique signature for this structure */
	pthread_lock_t lock;
	uint32_t	detached:8,
			inherit:8,
			policy:8,
			freeStackOnExit:1,
			fastpath:1,
			schedset:1,
			reserved1:5;
	size_t	       guardsize;	/* size in bytes to guard stack overflow */
	int      reserved2; 	/* Should we free the stack when we exit? */
	struct sched_param param;
        void           *stackaddr;     /* Base of the stack (is aligned on vm_page_size boundary */
        size_t         stacksize;      /* Size of the stack (is a multiple of vm_page_size and >= PTHREAD_STACK_MIN) */
	boolean_t	reserved3;
};

/*
 * Mutex attributes
 */
#define _PTHREAD_MUTEX_POLICY_NONE		0
#define _PTHREAD_MUTEX_POLICY_FAIRSHARE		1
#define _PTHREAD_MUTEX_POLICY_FIRSTFIT		2
#define _PTHREAD_MUTEX_POLICY_REALTIME		3
#define _PTHREAD_MUTEX_POLICY_ADAPTIVE		4
#define _PTHREAD_MUTEX_POLICY_PRIPROTECT	5
#define _PTHREAD_MUTEX_POLICY_PRIINHERIT	6

#define _PTHREAD_MUTEXATTR_T
typedef struct 
{
	long sig;		     /* Unique signature for this structure */
	int prioceiling;
	uint32_t protocol:2,		/* protocol attribute */
		type:2,			/* mutex type */
		pshared:2,
		policy:3,
		rfu:23;
} pthread_mutexattr_t;

/*
 * Mutex variables
 */
struct _pthread_mutex_options {
	uint32_t  protocol:2,		/* protocol */
		type:2,			/* mutex type */
		pshared:2,			/* mutex type */
		policy:3,
		hold:2,
		misalign:1,		/* 8 byte aligned? */
		notify:1,		/* CV notify field for kernel */
		mutex:1,		/* used in clrprepo that it is a mutex */
		rfu:2,
		lock_count:16;
};


#define _PTHREAD_MTX_OPT_PSHARED 0x010
#define _PTHREAD_MTX_OPT_HOLD 0x200	/* current owner of the mutex */
#define _PTHREAD_MTX_OPT_NOMTX 0x400	/* no mutex refs held */

#define _PTHREAD_MTX_OPT_NOTIFY 0x1000	/* notify to drop mutex handling in cvwait */
#define _PTHREAD_MTX_OPT_MUTEX	0x2000 /* this is a mutex type  */


#define _PTHREAD_MUTEX_T
typedef struct _pthread_mutex
{
	long	       sig;	      /* Unique signature for this structure */
	pthread_lock_t lock;	      /* Used for internal mutex on structure */
	union {
		uint32_t value;
		struct _pthread_mutex_options options;
	} mtxopts;
	int16_t       prioceiling;
	int16_t	       priority;      /* Priority to restore when mutex unlocked */
	uint32_t      waiters;       /* Count of threads waiting for this mutex */
	pthread_t      owner;	      /* Which thread has this mutex locked */
	struct _pthread_mutex *next, *prev;  /* List of other mutexes he owns */
	struct _pthread_cond *busy;   /* List of condition variables using this mutex */
	semaphore_t    sem;	      /* Semaphore used for waiting */
	semaphore_t	order;
} pthread_mutex_t;


typedef struct _npthread_mutex
{
/* keep same as pthread_mutex_t from here to .. */
	long	       sig;	      /* Unique signature for this structure */
	pthread_lock_t lock;	      /* Used for static init sequencing */
	union {
		uint32_t value;
		struct _pthread_mutex_options options;
	} mtxopts;
	int16_t       prioceiling;
	int16_t	       priority;      /* Priority to restore when mutex unlocked */
	uint32_t	m_seq[3];
#if  defined(__LP64__)
	uint64_t      m_tid;	      /* Which thread has this mutex locked */
	uint32_t  *	m_lseqaddr;
	uint32_t  *	m_useqaddr;
	uint32_t 	reserved[2];
#else
	uint32_t  *	m_lseqaddr;
	uint64_t      m_tid;	      /* Which thread has this mutex locked */
	uint32_t  *	m_useqaddr;
#endif
} npthread_mutex_t;




/*
 * Condition variable attributes
 */
#define _PTHREAD_CONDATTR_T
typedef struct 
{
	long	       sig;	     /* Unique signature for this structure */
	uint32_t	 pshared:2,		/* pshared */
		unsupported:30;
} pthread_condattr_t;

/*
 * Condition variables
 */
#define _PTHREAD_COND_T
typedef struct _pthread_cond
{
	long	       sig;	     /* Unique signature for this structure */
	pthread_lock_t lock;	     /* Used for internal mutex on structure */
	uint32_t	waiters:15,	/* Number of threads waiting */
		   sigspending:15,	/* Number of outstanding signals */
			pshared:2;
	struct _pthread_cond *next, *prev;  /* List of condition variables using mutex */
	struct _pthread_mutex *busy; /* mutex associated with variable */
	semaphore_t    sem;	     /* Kernel semaphore */
} pthread_cond_t;


typedef struct _npthread_cond
{
	long	       sig;	     /* Unique signature for this structure */
	pthread_lock_t lock;	     /* Used for internal mutex on structure */
	uint32_t	rfu:29,		/* not in use*/
			misalign: 1,	/* structure is not aligned to 8 byte boundary */
			pshared:2;
	struct _npthread_mutex *busy; /* mutex associated with variable */
	uint32_t 	c_seq[3];
#if defined(__LP64__)
	uint32_t	reserved[3];
#endif /* __LP64__ */
} npthread_cond_t;

/*
 * Initialization control (once) variables
 */
#define _PTHREAD_ONCE_T
typedef struct 
{
	long	       sig;	      /* Unique signature for this structure */
	pthread_lock_t lock;	      /* Used for internal mutex on structure */
} pthread_once_t;

#define _PTHREAD_RWLOCKATTR_T
typedef struct {
	long	       sig;	      /* Unique signature for this structure */
	int             pshared;
	int		rfu[2];		/* reserved for future use */
} pthread_rwlockattr_t;

#define _PTHREAD_RWLOCK_T
typedef struct {
	long 		sig;
	pthread_mutex_t lock;   /* monitor lock */
	int             state;
	pthread_cond_t  read_signal;
	pthread_cond_t  write_signal;
	int             blocked_writers;
	int 		reserved;
	pthread_t	owner;
	int	rfu[1];
	int 		pshared;
} pthread_rwlock_t;

#define PTHRW_RFU_64BIT 124 /* 31 * sizeof(uint32_t) */
#define PTHRW_RFU_32BIT 72  /* 18 * sizeof(uint32_t) */

#define _PTHREAD_RWLOCK_T
typedef struct {
	long 		sig;
	pthread_lock_t lock;
#if defined(__LP64__)
	int 		reserv;
	volatile uint32_t	rw_seq[4];
	pthread_t	rw_owner;
#else /* __LP64__ */
	volatile uint32_t	rw_seq[4];
	pthread_t	rw_owner;
	int 		reserv;
#endif /* __LP64__ */
	volatile uint32_t *	rw_lcntaddr;
	volatile uint32_t *	rw_seqaddr;
	volatile uint32_t *	rw_ucntaddr;
	uint32_t        rw_flags;
	int		misalign;
#if defined(__LP64__)
	char	rfu[PTHRW_RFU_64BIT];
#else /* __LP64__ */
	char	rfu[PTHRW_RFU_32BIT];
#endif /* __LP64__ */
	int 		pshared;
} npthread_rwlock_t;

/* flags for rw_flags */
#define PTHRW_KERN_PROCESS_SHARED	0x10
#define PTHRW_KERN_PROCESS_PRIVATE	0x20
#define PTHRW_KERN_PROCESS_FLAGS_MASK	0x30
#define _PTHREAD_RWLOCK_UPGRADE_TRY	0x10000

/* New model bits on Lword */
#define PTH_RWL_KBIT	0x01	/* users cannot acquire in user mode */
#define PTH_RWL_EBIT	0x02	/* exclusive lock in progress */
#define PTH_RWL_WBIT	0x04	/* write waiters pending in kernel */
#define PTH_RWL_PBIT	0x04	/* prepost (cv) pending in kernel */
#define PTH_RWL_YBIT	0x08	/* yielding write waiters pending in kernel */
#define PTH_RWL_RETRYBIT 0x08 	/* mutex retry wait */
#define PTH_RWL_LBIT	0x10	/* long read in progress */
#define PTH_RWL_MTXNONE	0x10	/* indicates the cvwait does not have mutex held */
#define PTH_RWL_UBIT	0x20	/* upgrade request pending */
#define PTH_RWL_MTX_WAIT 0x20 	/* in cvar in mutex wait */
#define PTH_RWL_RBIT	0x40	/* reader pending in kernel(not used) */
#define PTH_RWL_MBIT	0x40	/* overlapping grants from kernel */
#define PTH_RWL_TRYLKBIT 0x40	/* sets try lock attempt */
#define PTH_RWL_IBIT	0x80	/* lock reset, held untill first succeesful unlock */

/* UBIT values for mutex, cvar */
#define PTH_RWU_SBIT	0x01
#define PTH_RWU_BBIT	0x02

#define PTHRW_RWL_INIT       PTH_RWL_IBIT    /* reset on the lock bits (U)*/
#define PTHRW_RWLOCK_INIT    (PTH_RWL_IBIT | PTH_RWL_RBIT)   /* reset on the lock bits (U)*/
#define PTH_RWLOCK_RESET_RBIT	0xffffffbf

#define PTHRW_INC	0x100
#define PTHRW_BIT_MASK	0x000000ff

#define PTHRW_UN_BIT_MASK 0x000000bf	/* remove overlap  bit */


/* New model bits on Sword */
#define PTH_RWS_SBIT	0x01	/* kernel transition seq not set yet*/
#define PTH_RWS_IBIT	0x02	/* Sequence is not set on return from kernel */

#define PTH_RWS_CV_CBIT	PTH_RWS_SBIT	/* kernel has cleared all info w.r.s.t CV */
#define PTH_RWS_CV_PBIT	PTH_RWS_IBIT	/* kernel has prepost/fake structs only,no waiters */
#define PTH_RWS_CV_BITSALL	(PTH_RWS_CV_CBIT | PTH_RWS_CV_PBIT)
#define PTH_RWS_CV_MBIT PTH_RWL_MBIT    /* to indicate prepost return from kernel */
#define PTH_RWS_CV_RESET_PBIT	0xfffffffd

#define PTH_RWS_WSVBIT	0x04	/* save W bit */
#define PTH_RWS_USVBIT	0x08	/* save U bit */
#define PTH_RWS_YSVBIT	0x10	/* save Y bit */
#define PTHRW_RWS_INIT       PTH_RWS_SBIT    /* reset on the lock bits (U)*/
#define PTHRW_RWS_SAVEMASK (PTH_RWS_WSVBIT|PTH_RWS_USVBIT|PTH_RWS_YSVBIT)    /*save bits mask*/
#define PTHRW_SW_Reset_BIT_MASK 0x000000fe	/* remove S bit and get rest of the bits */

#define PTHRW_COUNT_SHIFT	8 
#define PTHRW_COUNT_MASK 	0xffffff00
#define PTHRW_MAX_READERS 	0xffffff00


#define PTHREAD_MTX_TID_SWITCHING (uint64_t)-1

/* new L word defns */
#define can_rwl_readinuser(x) ((((x) & (PTH_RWL_UBIT | PTH_RWL_WBIT | PTH_RWL_KBIT)) == 0)||(((x) & PTH_RWL_LBIT) != 0))
#define can_rwl_longreadinuser(x) (((x) & (PTH_RWL_UBIT | PTH_RWL_WBIT | PTH_RWL_KBIT | PTH_RWL_YBIT)) == 0)
#define is_rwl_ebit_set(x) (((x) & PTH_RWL_EBIT) != 0)
#define is_rwl_eubit_set(x) (((x) & (PTH_RWL_EBIT | PTH_RWL_UBIT)) != 0)
#define is_rwl_wbit_set(x) (((x) & PTH_RWL_WBIT) != 0)
#define is_rwl_lbit_set(x) (((x) & PTH_RWL_LBIT) != 0)
#define is_rwl_ebit_clear(x) (((x) & PTH_RWL_EBIT) == 0)
#define is_rwl_lbit_clear(x) (((x) & PTH_RWL_LBIT) == 0)
#define is_rwl_readoverlap(x) (((x) & PTH_RWL_MBIT) != 0)

/* S word checks */
#define is_rws_setseq(x) (((x) & PTH_RWS_SBIT))
#define is_rws_setunlockinit(x) (((x) & PTH_RWS_IBIT))

/* is x lower than Y */
static inline int is_seqlower(uint32_t x, uint32_t y) {
        if (x < y) {
                if ((y-x) < (PTHRW_MAX_READERS/2))
                        return(1);
        } else {
                if ((x-y) > (PTHRW_MAX_READERS/2))
                        return(1);
        }
        return(0);
}

/* is x lower than or eq Y */
static inline int is_seqlower_eq(uint32_t x, uint32_t y) {
        if (x==y)
                return(1);
        else
                return(is_seqlower(x,y));
}

/* is x greater than Y */
static inline int is_seqhigher(uint32_t x, uint32_t y) {
        if (x > y) {
                if ((x-y) < (PTHRW_MAX_READERS/2))
                        return(1);
        } else {
                if ((y-x) > (PTHRW_MAX_READERS/2))
                        return(1);
        }
        return(0);
}

static inline  int diff_genseq(uint32_t x, uint32_t y) {
	if (x == y) {
		return(0);
        } else if (x > y)  {
                return(x-y);
        } else {
                return((PTHRW_MAX_READERS - y) + x + PTHRW_INC);
        }
}

/* keep the size to 64bytes  for both 64 and 32 */
#define _PTHREAD_WORKQUEUE_ATTR_T
typedef struct {
	uint32_t sig;
	int queueprio;
	int overcommit;
	unsigned int resv2[13];
} pthread_workqueue_attr_t;

#define _PTHREAD_WORKITEM_T
typedef struct _pthread_workitem {
	TAILQ_ENTRY(_pthread_workitem) item_entry;	/* pthread_workitem list in prio */
	void	(*func)(void *);
	void	* func_arg;
	struct _pthread_workqueue *  workq;	
	unsigned int	flags;
	unsigned int	fromcache;		/* padding for 64bit */
}  * pthread_workitem_t;

#define PTH_WQITEM_INKERNEL_QUEUE 	1
#define PTH_WQITEM_RUNNING		2
#define PTH_WQITEM_COMPLETED 		4
#define PTH_WQITEM_REMOVED 		8
#define PTH_WQITEM_BARRIER 		0x10
#define PTH_WQITEM_DESTROY 		0x20
#define PTH_WQITEM_NOTINLIST 		0x40
#define PTH_WQITEM_APPLIED 		0x80
#define PTH_WQITEM_KERN_COUNT 		0x100

/* try to fit these within multiple of pages (8 pages for now) */
#define WORKITEM_POOL_SIZE 680
/* ensure some multiple of the chunk is the pool size */
#define WORKITEM_CHUNK_SIZE 40

#define WORKITEM_STARTPOOL_SIZE WORKITEM_CHUNK_SIZE

TAILQ_HEAD(__pthread_workitem_pool, _pthread_workitem);
extern struct __pthread_workitem_pool __pthread_workitem_pool_head;        /* head list of workitem pool  */

#define _PTHREAD_WORKQUEUE_HEAD_T
typedef struct  _pthread_workqueue_head {
	TAILQ_HEAD(, _pthread_workqueue) wqhead;
	struct _pthread_workqueue * next_workq;	
} * pthread_workqueue_head_t;


/* sized to be 128 bytes both in 32 and 64bit archs */
#define _PTHREAD_WORKQUEUE_T
typedef struct  _pthread_workqueue {
	unsigned int       sig;	      /* Unique signature for this structure */
	pthread_lock_t lock;	      /* Used for internal mutex on structure */
	TAILQ_ENTRY(_pthread_workqueue) wq_list;	/* workqueue list in prio */
	TAILQ_HEAD(, _pthread_workitem) item_listhead;	/* pthread_workitem list in prio */
	TAILQ_HEAD(, _pthread_workitem) item_kernhead;	/* pthread_workitem list in prio */
	unsigned int	flags;
	size_t		stacksize;
	int		istimeshare;
	int 		importance;
	int 		affinity;
	int		queueprio;
	int		barrier_count;
	int		kq_count;
	void		(*term_callback)(struct _pthread_workqueue *,void *);
	void  * term_callarg;
	pthread_workqueue_head_t headp;
	int		overcommit;
#if  !defined(__LP64__)
	unsigned int	rev2[12];
#endif
}  * pthread_workqueue_t;

#define	 PTHREAD_WORKQ_IN_CREATION	1
#define	 PTHREAD_WORKQ_IN_TERMINATE	2
#define	 PTHREAD_WORKQ_BARRIER_ON	4
#define	 PTHREAD_WORKQ_TERM_ON		8
#define	 PTHREAD_WORKQ_DESTROYED	0x10
#define	 PTHREAD_WORKQ_REQUEUED		0x20
#define	 PTHREAD_WORKQ_SUSPEND		0x40

#define WORKQUEUE_POOL_SIZE 16
TAILQ_HEAD(__pthread_workqueue_pool, _pthread_workqueue);
extern struct __pthread_workqueue_pool __pthread_workqueue_pool_head;        /* head list of workqueue pool  */

#include "pthread_spis.h"

#if defined(__i386__) || defined(__ppc64__) || defined(__x86_64__) || (defined(__arm__) && (defined(_ARM_ARCH_7) || !defined(_ARM_ARCH_6) || !defined(__thumb__))) 
/*
 * Inside libSystem, we can use r13 or %gs directly to get access to the
 * thread-specific data area. The current thread is in the first slot.
 */
inline static pthread_t __attribute__((__pure__))
_pthread_self_direct(void)
{
	pthread_t ret;

#if defined(__i386__) || defined(__x86_64__)
	ret = _pthread_getspecific_direct(0);
#elif defined(__ppc64__)
	register const pthread_t __pthread_self asm ("r13");
	ret = __pthread_self;
#elif defined(__arm__) && defined(_ARM_ARCH_6)
	ret = _pthread_getspecific_direct(0);
#elif defined(__arm__) && !defined(_ARM_ARCH_6)
	ret = _pthread_getspecific_direct(0);
#endif
       return ret;
}
#define pthread_self() _pthread_self_direct()
#endif

#define _PTHREAD_DEFAULT_INHERITSCHED	PTHREAD_INHERIT_SCHED
#define _PTHREAD_DEFAULT_PROTOCOL	PTHREAD_PRIO_NONE
#define _PTHREAD_DEFAULT_PRIOCEILING	0
#define _PTHREAD_DEFAULT_POLICY		SCHED_OTHER
#define _PTHREAD_DEFAULT_STACKSIZE	0x80000	  /* 512K */
#define _PTHREAD_DEFAULT_PSHARED	PTHREAD_PROCESS_PRIVATE

#define _PTHREAD_NO_SIG			0x00000000
#define _PTHREAD_MUTEX_ATTR_SIG		0x4D545841  /* 'MTXA' */
#define _PTHREAD_MUTEX_SIG		0x4D555458  /* 'MUTX' */
#define _PTHREAD_MUTEX_SIG_init		0x32AAABA7  /* [almost] ~'MUTX' */
#define _PTHREAD_ERRORCHECK_MUTEX_SIG_init      0x32AAABA1
#define _PTHREAD_RECURSIVE_MUTEX_SIG_init       0x32AAABA2
#define _PTHREAD_FIRSTFIT_MUTEX_SIG_init       0x32AAABA3
#define _PTHREAD_MUTEX_SIG_init_MASK            0xfffffff0
#define _PTHREAD_MUTEX_SIG_CMP                  0x32AAABA0
#define _PTHREAD_COND_ATTR_SIG		0x434E4441  /* 'CNDA' */
#define _PTHREAD_COND_SIG		0x434F4E44  /* 'COND' */
#define _PTHREAD_COND_SIG_init		0x3CB0B1BB  /* [almost] ~'COND' */
#define _PTHREAD_ATTR_SIG		0x54484441  /* 'THDA' */
#define _PTHREAD_ONCE_SIG		0x4F4E4345  /* 'ONCE' */
#define _PTHREAD_ONCE_SIG_init		0x30B1BCBA  /* [almost] ~'ONCE' */
#define _PTHREAD_SIG			0x54485244  /* 'THRD' */
#define _PTHREAD_RWLOCK_ATTR_SIG	0x52574C41  /* 'RWLA' */
#define _PTHREAD_RWLOCK_SIG		0x52574C4B  /* 'RWLK' */
#define _PTHREAD_RWLOCK_SIG_init	0x2DA8B3B4  /* [almost] ~'RWLK' */


#define _PTHREAD_KERN_COND_SIG		0x12345678  /*  */
#define _PTHREAD_KERN_MUTEX_SIG		0x34567812  /*  */
#define _PTHREAD_KERN_RWLOCK_SIG	0x56781234  /*  */

#define _PTHREAD_CREATE_PARENT		4
#define _PTHREAD_EXITED			8
// 4597450: begin
#define _PTHREAD_WASCANCEL		0x10
// 4597450: end

#if defined(DEBUG)
#define _PTHREAD_MUTEX_OWNER_SELF	pthread_self()
#else
#define _PTHREAD_MUTEX_OWNER_SELF	(pthread_t)0x12141968
#endif
#define _PTHREAD_MUTEX_OWNER_SWITCHING	(pthread_t)(~0)

#define _PTHREAD_CANCEL_STATE_MASK   0x01
#define _PTHREAD_CANCEL_TYPE_MASK    0x02
#define _PTHREAD_CANCEL_PENDING	     0x10  /* pthread_cancel() has been called for this thread */

extern boolean_t swtch_pri(int);

#ifndef PTHREAD_MACH_CALL
#define	PTHREAD_MACH_CALL(expr, ret) (ret) = (expr)
#endif

/* Prototypes. */

/* Functions defined in machine-dependent files. */
extern vm_address_t _sp(void);
extern vm_address_t _adjust_sp(vm_address_t sp);
extern void _pthread_setup(pthread_t th, void (*f)(pthread_t), void *sp, int suspended, int needresume);

extern void _pthread_tsd_cleanup(pthread_t self);

__private_extern__ void __mtx_holdlock(npthread_mutex_t *mutex, uint32_t diff, uint32_t * flagp, uint32_t ** pmtxp, uint32_t * mgenp, uint32_t * ugenp, uint64_t *tidp);
__private_extern__ int __mtx_droplock(npthread_mutex_t *mutex, uint32_t diff, uint32_t * flagp, uint32_t ** pmtxp, uint32_t * mgenp, uint32_t * ugenp);
__private_extern__ int __mtx_updatebits(npthread_mutex_t *mutex, uint32_t updateval, int firstfiti, int fromcond, uint64_t selfid);

/* syscall interfaces */
extern uint32_t __psynch_mutexwait(pthread_mutex_t * mutex,  uint32_t mgen, uint32_t  ugen, uint64_t tid, uint32_t flags);
extern uint32_t __psynch_mutexdrop(pthread_mutex_t * mutex,  uint32_t mgen, uint32_t  ugen, uint64_t tid, uint32_t flags);

extern uint32_t __psynch_cvbroad(pthread_cond_t * cv, uint64_t cvlsgen, uint64_t cvudgen, uint32_t flags, pthread_mutex_t * mutex,  uint64_t mugen, uint64_t tid);
extern uint32_t __psynch_cvsignal(pthread_cond_t * cv, uint64_t cvlsgen, uint32_t cvugen, int thread_port, pthread_mutex_t * mutex,  uint64_t mugen, uint64_t tid, uint32_t flags);
extern uint32_t __psynch_cvwait(pthread_cond_t * cv, uint64_t cvlsgen, uint32_t cvugen, pthread_mutex_t * mutex,  uint64_t mugen, uint32_t flags, int64_t sec, uint32_t nsec);
extern uint32_t __psynch_cvclrprepost(void * cv, uint32_t cvgen, uint32_t cvugen, uint32_t cvsgen, uint32_t prepocnt, uint32_t preposeq, uint32_t flags);
extern uint32_t __psynch_rw_longrdlock(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);
extern uint32_t __psynch_rw_yieldwrlock(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);
extern int __psynch_rw_downgrade(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);
extern uint32_t __psynch_rw_upgrade(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);
extern uint32_t __psynch_rw_rdlock(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);
extern uint32_t __psynch_rw_wrlock(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);
extern uint32_t __psynch_rw_unlock(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);
extern uint32_t __psynch_rw_unlock2(pthread_rwlock_t * rwlock, uint32_t lgenval, uint32_t ugenval, uint32_t rw_wc, int flags);

__private_extern__ semaphore_t new_sem_from_pool(void);
__private_extern__ void restore_sem_to_pool(semaphore_t);
__private_extern__ void _pthread_atfork_queue_init(void);
int _pthread_lookup_thread(pthread_t thread, mach_port_t * port, int only_joinable);
int _pthread_join_cleanup(pthread_t thread, void ** value_ptr, int conforming);
__private_extern__ int proc_setthreadname(void * buffer, int buffersize);

#endif /* _POSIX_PTHREAD_INTERNALS_H */

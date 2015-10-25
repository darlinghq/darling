/*
 * Copyright (c) 2000-2003, 2007, 2008 Apple Inc. All rights reserved.
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
 * -- Mutex variable support
 */

#include "pthread_internals.h"

#ifdef PLOCKSTAT
#include "plockstat.h"
#else /* !PLOCKSTAT */
#define	PLOCKSTAT_MUTEX_SPIN(x)
#define	PLOCKSTAT_MUTEX_SPUN(x, y, z)
#define	PLOCKSTAT_MUTEX_ERROR(x, y)
#define	PLOCKSTAT_MUTEX_BLOCK(x)
#define	PLOCKSTAT_MUTEX_BLOCKED(x, y)
#define	PLOCKSTAT_MUTEX_ACQUIRE(x, y, z)
#define	PLOCKSTAT_MUTEX_RELEASE(x, y)
#endif /* PLOCKSTAT */

extern int __unix_conforming;
extern int __unix_conforming;

#ifndef BUILDING_VARIANT
__private_extern__ int usenew_mtximpl = 1;
static void __pthread_mutex_set_signature(npthread_mutex_t * mutex);
int __mtx_markprepost(npthread_mutex_t *mutex, uint32_t oupdateval, int firstfit);
static int _pthread_mutex_destroy_locked(pthread_mutex_t *omutex);
#else /* BUILDING_VARIANT */
extern int usenew_mtximpl;
#endif /* BUILDING_VARIANT */


#ifdef NOTNEEDED 
#define USE_COMPAGE 1
extern int _commpage_pthread_mutex_lock(uint32_t * lvalp, int flags, uint64_t mtid, uint32_t mask, uint64_t * tidp, int *sysret);
#endif

#include <machine/cpu_capabilities.h>

int _pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr, uint32_t static_type);


#if defined(__LP64__)
#define MUTEX_GETSEQ_ADDR(mutex, lseqaddr, useqaddr) \
{ \
		if (mutex->mtxopts.options.misalign != 0) { \
			lseqaddr = &mutex->m_seq[0]; \
			useqaddr = &mutex->m_seq[1]; \
		 } else { \
			lseqaddr = &mutex->m_seq[1]; \
			useqaddr = &mutex->m_seq[2]; \
		} \
}
#else /* __LP64__ */
#define MUTEX_GETSEQ_ADDR(mutex, lseqaddr, useqaddr) \
{ \
		if (mutex->mtxopts.options.misalign != 0) { \
			lseqaddr = &mutex->m_seq[1]; \
			useqaddr = &mutex->m_seq[2]; \
		 }else { \
			lseqaddr = &mutex->m_seq[0]; \
			useqaddr = &mutex->m_seq[1]; \
		} \
}
#endif /* __LP64__ */

#define _KSYN_TRACE_ 0

#if _KSYN_TRACE_
/* The Function qualifiers  */
#define DBG_FUNC_START          1
#define DBG_FUNC_END            2
#define DBG_FUNC_NONE           0

int __kdebug_trace(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

#define _KSYN_TRACE_UM_LOCK     0x9000060
#define _KSYN_TRACE_UM_UNLOCK   0x9000064
#define _KSYN_TRACE_UM_MHOLD    0x9000068
#define _KSYN_TRACE_UM_MDROP    0x900006c
#define _KSYN_TRACE_UM_MUBITS    0x900007c
#define _KSYN_TRACE_UM_MARKPP    0x90000a8

#endif /* _KSYN_TRACE_ */

#ifndef BUILDING_VARIANT /* [ */

#define BLOCK_FAIL_PLOCKSTAT    0
#define BLOCK_SUCCESS_PLOCKSTAT 1

#ifdef PR_5243343
/* 5243343 - temporary hack to detect if we are running the conformance test */
extern int PR_5243343_flag;
#endif /* PR_5243343 */

/* This function is never called and exists to provide never-fired dtrace
 * probes so that user d scripts don't get errors.
 */
__private_extern__ __attribute__((used)) void
_plockstat_never_fired(void) 
{
	PLOCKSTAT_MUTEX_SPIN(NULL);
	PLOCKSTAT_MUTEX_SPUN(NULL, 0, 0);
}


/*
 * Initialize a mutex variable, possibly with additional attributes.
 * Public interface - so don't trust the lock - initialize it first.
 */
int
pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
#if 0
	/* conformance tests depend on not having this behavior */
	/* The test for this behavior is optional */
	if (mutex->sig == _PTHREAD_MUTEX_SIG)
		return EBUSY;
#endif
	LOCK_INIT(mutex->lock);
	return (_pthread_mutex_init(mutex, attr, 0x7));
}

/*
 * Fetch the priority ceiling value from a mutex variable.
 * Note: written as a 'helper' function to hide implementation details.
 */
int
pthread_mutex_getprioceiling(const pthread_mutex_t *mutex,
                             int *prioceiling)
{
	int res;

	LOCK(mutex->lock);
        if (mutex->sig == _PTHREAD_MUTEX_SIG)
        {
                *prioceiling = mutex->prioceiling;
                res = 0;
        } else
                res = EINVAL; /* Not an initialized 'attribute' structure */
	UNLOCK(mutex->lock);
	return (res);
}

/*
 * Set the priority ceiling for a mutex.
 * Note: written as a 'helper' function to hide implementation details.
 */
int
pthread_mutex_setprioceiling(pthread_mutex_t *mutex,
                             int prioceiling,
                             int *old_prioceiling)
{
	int res;

	LOCK(mutex->lock);
        if (mutex->sig == _PTHREAD_MUTEX_SIG)
        {
                if ((prioceiling >= -999) ||
                    (prioceiling <= 999))
                {
                        *old_prioceiling = mutex->prioceiling;
                        mutex->prioceiling = prioceiling;
                        res = 0;
                } else
                        res = EINVAL; /* Invalid parameter */
        } else
                res = EINVAL; /* Not an initialized 'attribute' structure */
	UNLOCK(mutex->lock);
	return (res);
}

/*
 * Get the priority ceiling value from a mutex attribute structure.
 * Note: written as a 'helper' function to hide implementation details.
 */
int
pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr,
                                 int *prioceiling)
{
        if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG)
        {
                *prioceiling = attr->prioceiling;
                return (0);
        } else
        {
                return (EINVAL); /* Not an initialized 'attribute' structure */
        }
}

/*
 * Get the mutex 'protocol' value from a mutex attribute structure.
 * Note: written as a 'helper' function to hide implementation details.
 */
int
pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr,
                              int *protocol)
{
        if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG)
        {
                *protocol = attr->protocol;
                return (0);
        } else
        {
                return (EINVAL); /* Not an initialized 'attribute' structure */
        }
}
/*
 * Get the mutex 'type' value from a mutex attribute structure.
 * Note: written as a 'helper' function to hide implementation details.
 */
int
pthread_mutexattr_gettype(const pthread_mutexattr_t *attr,
                              int *type)
{
        if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG)
        {
                *type = attr->type;
                return (0);
        } else
        {
                return (EINVAL); /* Not an initialized 'attribute' structure */
        }
}

/*
 *
 */
int
pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *pshared)
{
        if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG)
        {
                *pshared = (int)attr->pshared;
                return (0);
        } else
        {
                return (EINVAL); /* Not an initialized 'attribute' structure */
        }
}

/*
 * Initialize a mutex attribute structure to system defaults.
 */
int
pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
        attr->prioceiling = _PTHREAD_DEFAULT_PRIOCEILING;
        attr->protocol = _PTHREAD_DEFAULT_PROTOCOL;
        attr->policy = _PTHREAD_MUTEX_POLICY_FAIRSHARE;
        attr->type = PTHREAD_MUTEX_DEFAULT;
        attr->sig = _PTHREAD_MUTEX_ATTR_SIG;
        attr->pshared = _PTHREAD_DEFAULT_PSHARED;
        return (0);
}

/*
 * Set the priority ceiling value in a mutex attribute structure.
 * Note: written as a 'helper' function to hide implementation details.
 */
int
pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr,
                                 int prioceiling)
{
        if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG)
        {
                if ((prioceiling >= -999) ||
                    (prioceiling <= 999))
                {
                        attr->prioceiling = prioceiling;
                        return (0);
                } else
                {
                        return (EINVAL); /* Invalid parameter */
                }
        } else
        {
                return (EINVAL); /* Not an initialized 'attribute' structure */
        }
}

/*
 * Set the mutex 'protocol' value in a mutex attribute structure.
 * Note: written as a 'helper' function to hide implementation details.
 */
int
pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr,
                              int protocol)
{
        if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG)
        {
                if ((protocol == PTHREAD_PRIO_NONE) ||
                    (protocol == PTHREAD_PRIO_INHERIT) ||
                    (protocol == PTHREAD_PRIO_PROTECT))
                {
                        attr->protocol = protocol;
                        return (0);
                } else
                {
                        return (EINVAL); /* Invalid parameter */
                }
        } else
        {
                return (EINVAL); /* Not an initialized 'attribute' structure */
        }
}

int
pthread_mutexattr_setpolicy_np(pthread_mutexattr_t *attr,
                              int policy)
{
        if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG)
        {
                if (
		    (policy == _PTHREAD_MUTEX_POLICY_FAIRSHARE) ||
                    (policy == _PTHREAD_MUTEX_POLICY_FIRSTFIT)
#if NOTYET
		    ||
                    (policy == _PTHREAD_MUTEX_POLICY_REALTIME) ||
                    (policy == _PTHREAD_MUTEX_POLICY_ADAPTIVE) ||
                    (policy == _PTHREAD_MUTEX_POLICY_PRIPROTECT) ||
                    (policy == _PTHREAD_MUTEX_POLICY_PRIINHERIT)
#endif /* NOTYET */
		)
                {
                        attr->policy = policy;
                        return (0);
                } else
                {
                        return (EINVAL); /* Invalid parameter */
                }
        } else
        {
                return (EINVAL); /* Not an initialized 'attribute' structure */
        }
}

/*
 * Set the mutex 'type' value in a mutex attribute structure.
 * Note: written as a 'helper' function to hide implementation details.
 */
int
pthread_mutexattr_settype(pthread_mutexattr_t *attr,
                              int type)
{
        if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG)
        {
                if ((type == PTHREAD_MUTEX_NORMAL) ||
                    (type == PTHREAD_MUTEX_ERRORCHECK) ||
                    (type == PTHREAD_MUTEX_RECURSIVE) ||
                    (type == PTHREAD_MUTEX_DEFAULT))
                {
                        attr->type = type;
                        return (0);
                } else
                {
                        return (EINVAL); /* Invalid parameter */
                }
        } else
        {
                return (EINVAL); /* Not an initialized 'attribute' structure */
        }
}


int mutex_try_lock(int *x) {
        return _spin_lock_try((pthread_lock_t *)x);
}

void mutex_wait_lock(int *x) {
        for (;;) {
                if( _spin_lock_try((pthread_lock_t *)x)) {
                        return;
                }
                swtch_pri(0);
        }
}

void 
cthread_yield(void) 
{
        sched_yield();
}

void 
pthread_yield_np (void) 
{
        sched_yield();
}


/*
 * Temp: till pshared is fixed correctly
 */
int
pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared)
{
#if __DARWIN_UNIX03
	if (__unix_conforming == 0)
		__unix_conforming = 1;
#endif /* __DARWIN_UNIX03 */

        if (attr->sig == _PTHREAD_MUTEX_ATTR_SIG)
        {
#if __DARWIN_UNIX03
                if (( pshared == PTHREAD_PROCESS_PRIVATE) || (pshared == PTHREAD_PROCESS_SHARED))
#else /* __DARWIN_UNIX03 */
                if ( pshared == PTHREAD_PROCESS_PRIVATE)
#endif /* __DARWIN_UNIX03 */
	  	{
                         attr->pshared = pshared; 
                        return (0);
                } else {
                        return (EINVAL); /* Invalid parameter */
                }
        } else
        {
                return (EINVAL); /* Not an initialized 'attribute' structure */
        }
}

/*
 * Drop the mutex unlock references(from cond wait or mutex_unlock().
 * 
 */
__private_extern__ int
__mtx_droplock(npthread_mutex_t * mutex, uint32_t diffgen, uint32_t * flagsp, uint32_t ** pmtxp, uint32_t * mgenp, uint32_t * ugenp)
{
	pthread_t self;
	uint64_t selfid, resettid;
	int firstfit = (mutex->mtxopts.options.policy == _PTHREAD_MUTEX_POLICY_FIRSTFIT);
	uint32_t lgenval, ugenval, nlval, ulval, morewaiters=0, flags;
	volatile uint32_t * lseqaddr, *useqaddr;
	uint64_t oldval64, newval64;
	int numwaiters=0, clearprepost = 0;

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_MDROP | DBG_FUNC_START, (uint32_t)mutex, diffgen, 0, 0, 0);
#endif
	MUTEX_GETSEQ_ADDR(mutex, lseqaddr, useqaddr);
	

	flags = mutex->mtxopts.value;
	flags &= ~_PTHREAD_MTX_OPT_NOTIFY;	/* no notification by default */


	if (mutex->mtxopts.options.type != PTHREAD_MUTEX_NORMAL)
	{
		self = pthread_self();
		(void) pthread_threadid_np(self, &selfid); 

		if (mutex->m_tid != selfid)
		{
			//LIBC_ABORT("dropping recur or error mutex not owned by the thread\n");
			PLOCKSTAT_MUTEX_ERROR((pthread_mutex_t *)mutex, EPERM);
			return(EPERM);
		} else if (mutex->mtxopts.options.type == PTHREAD_MUTEX_RECURSIVE &&
				   --mutex->mtxopts.options.lock_count)
		{
			PLOCKSTAT_MUTEX_RELEASE((pthread_mutex_t *)mutex, 1);
			goto out;
		}
	}
	
	
retry:
	lgenval = *lseqaddr;
	ugenval = *useqaddr;

	clearprepost = 0;

	numwaiters = diff_genseq((lgenval & PTHRW_COUNT_MASK),(ugenval & PTHRW_COUNT_MASK));	/* pendig waiters */

	if (numwaiters == 0) {
		/* spurious unlocks, do not touch tid */
		oldval64 = (((uint64_t)ugenval) << 32);
		oldval64 |= lgenval;
		if ((firstfit != 0) && ((lgenval & PTH_RWL_PBIT) != 0)) {
			clearprepost = 1;
			lgenval &= ~PTH_RWL_PBIT;
			newval64 = (((uint64_t)ugenval) << 32);
			newval64 |= lgenval;
		} else
			newval64 = oldval64;
		if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lseqaddr) != TRUE) 
			goto retry;
		/* validated L & U to be same, this is spurious unlock */
		flags &= ~_PTHREAD_MTX_OPT_NOTIFY;
		if (clearprepost == 1)
			 __psynch_cvclrprepost(mutex, lgenval, ugenval, 0, 0, lgenval, (flags | _PTHREAD_MTX_OPT_MUTEX));
			
		goto out;
	}

	if (numwaiters < diffgen) {
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_MDROP | DBG_FUNC_NONE, (uint32_t)mutex, numwaiters, lgenval, ugenval, 0);
#endif
		/* cannot drop more than existing number of waiters */
		diffgen = numwaiters;
	}

	oldval64 = (((uint64_t)ugenval) << 32);
	oldval64 |= lgenval;
	ulval = ugenval  + diffgen;
	nlval = lgenval;

	if ((lgenval & PTHRW_COUNT_MASK) == (ulval & PTHRW_COUNT_MASK)) {
		/* do not reset Ibit, just K&E */
		nlval &= ~(PTH_RWL_KBIT | PTH_RWL_EBIT);
		flags &= ~_PTHREAD_MTX_OPT_NOTIFY;
		if ((firstfit != 0) && ((lgenval & PTH_RWL_PBIT) != 0)) {
			clearprepost = 1;
			nlval &= ~PTH_RWL_PBIT;
		}
	} else {
		/* need to signal others waiting for mutex */
		morewaiters = 1;
		flags |= _PTHREAD_MTX_OPT_NOTIFY;
	}

	if (((nlval & PTH_RWL_EBIT) != 0) && (firstfit != 0)) {
		nlval &= ~PTH_RWL_EBIT;		/* reset Ebit so another can acquire meanwhile */
	}

	newval64 = (((uint64_t)ulval) << 32);
	newval64 |= nlval;

	resettid = mutex->m_tid;

	if ((lgenval & PTHRW_COUNT_MASK) == (ulval & PTHRW_COUNT_MASK))
		mutex->m_tid = 0;
	else if (firstfit == 0)
		mutex->m_tid = PTHREAD_MTX_TID_SWITCHING;
	
	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lseqaddr) != TRUE) {
		mutex->m_tid = resettid;
		goto retry;
	}


#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_MDROP | DBG_FUNC_NONE, (uint32_t)mutex, 2, lgenval, ugenval, 0);
	(void)__kdebug_trace(_KSYN_TRACE_UM_MDROP | DBG_FUNC_NONE, (uint32_t)mutex, 2, nlval, ulval, 0);
#endif

	if (clearprepost != 0) {
		 __psynch_cvclrprepost(mutex, nlval, ulval, 0, 0, nlval, (flags | _PTHREAD_MTX_OPT_MUTEX));
	}
		
	if (mgenp != NULL)
		*mgenp = nlval;		
	if (ugenp != NULL)
		*ugenp = ulval;		
#if USE_COMPAGE
	if (pmtxp != NULL)
		*pmtxp = lseqaddr;
#else
	if (pmtxp != NULL)
		*pmtxp = (uint32_t *)mutex;
#endif

out:
	if (flagsp != NULL)
		*flagsp = flags;		

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_MDROP | DBG_FUNC_END, (uint32_t)mutex, flags, 0, 0, 0);
#endif
	return(0);
}

int
__mtx_updatebits(npthread_mutex_t *mutex, uint32_t oupdateval, int firstfit, int fromcond, uint64_t selfid)
{
	uint32_t updateval = oupdateval;
#if !USE_COMPAGE
	pthread_mutex_t * omutex = (pthread_mutex_t *)mutex;
#endif
	int isebit = 0;
	uint32_t lgenval, ugenval, nval, uval, bits;
	volatile uint32_t * lseqaddr, *useqaddr;
	uint64_t oldval64, newval64;

	MUTEX_GETSEQ_ADDR(mutex, lseqaddr, useqaddr);

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_MUBITS | DBG_FUNC_START, (uint32_t)mutex, oupdateval, firstfit, fromcond, 0);
#endif

retry:
        lgenval = *lseqaddr;
        ugenval = *useqaddr;
        bits = updateval & PTHRW_BIT_MASK;

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_MUBITS | DBG_FUNC_NONE, (uint32_t)mutex, 1, lgenval, ugenval, 0);
#endif


	if ((updateval & PTH_RWL_MTX_WAIT) != 0) {
		lgenval = (updateval & PTHRW_COUNT_MASK) | (lgenval & PTHRW_BIT_MASK);
		if (fromcond == 0) {
			/* if from mutex_lock(), it will handle the rewind */
			return(1);
		}
		/* go block in the kernel with same lgenval as returned */
		goto ml1;
	} else {
		/* firsfit might not have EBIT */
		if (firstfit != 0) {
			if ((lgenval & PTH_RWL_EBIT) != 0)
				isebit = 1;
			else
				isebit = 0;
		} else if ((lgenval & (PTH_RWL_KBIT|PTH_RWL_EBIT)) == (PTH_RWL_KBIT|PTH_RWL_EBIT)) {
			/* fairshare mutex and the bits are already set, just update tid */
			goto out;
		}
	}

	/* either firstfist or no E bit set */
	/* update the bits */
	oldval64 = (((uint64_t)ugenval) << 32);
	oldval64 |= lgenval;
	uval = ugenval;
	nval = lgenval | (PTH_RWL_KBIT|PTH_RWL_EBIT);
	newval64 = (((uint64_t)uval) << 32);
	newval64 |= nval;

	/* set s and b bit */
	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lseqaddr) == TRUE) {
#if _KSYN_TRACE_
		(void)__kdebug_trace(_KSYN_TRACE_UM_MUBITS | DBG_FUNC_NONE, (uint32_t)mutex, 2, nval, uval, 0);
#endif
		if ((firstfit != 0) && (isebit != 0))
			goto handleffit;

		goto out;
	} else  {
		if (firstfit == 0)
			goto retry;
		else
			goto handleffit;
	}
	
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_MUBITS | DBG_FUNC_NONE, (uint32_t)mutex, 4, nval, uval, 0);
#endif

out:
	/* succesful bits updation */
	mutex->m_tid = selfid;
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_MUBITS | DBG_FUNC_END, (uint32_t)mutex, 0, 0, 0, 0);
#endif
	return(0);

handleffit:
	/* firstfit failure */
        lgenval = *lseqaddr;
        ugenval = *useqaddr;
	if ((lgenval & PTH_RWL_EBIT) == 0)
		goto retry;

	if (fromcond == 0)
		return(1);
	else {
		/* called from condition variable code  block again */
ml1:
#if  USE_COMPAGE /* [ */
		updateval = __psynch_mutexwait((pthread_mutex_t *)lseqaddr, lgenval | PTH_RWL_RETRYBIT, ugenval, mutex->m_tid,
					   mutex->mtxopts.value);
#else /* USECOMPAGE ][ */
		updateval = __psynch_mutexwait(omutex, lgenval | PTH_RWL_RETRYBIT, ugenval, mutex->m_tid,
					   mutex->mtxopts.value);
#endif /* USE_COMPAGE ] */
		if (updateval == (uint32_t)-1) {
			goto ml1;
		}

		/* now update the bits */
		goto retry;
	}
	/* cannot reach */
	goto retry;
}


int
__mtx_markprepost(npthread_mutex_t *mutex, uint32_t oupdateval, int firstfit)
{
	uint32_t updateval = oupdateval;
	int clearprepost = 0;
	uint32_t lgenval, ugenval,flags;
	volatile uint32_t * lseqaddr, *useqaddr;
	uint64_t oldval64, newval64;

	MUTEX_GETSEQ_ADDR(mutex, lseqaddr, useqaddr);

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_MARKPP | DBG_FUNC_START, (uint32_t)mutex, oupdateval, firstfit, 0, 0);
#endif

retry:

	clearprepost = 0;

	if ((firstfit != 0) && ((updateval & PTH_RWL_PBIT) != 0)) {
		flags = mutex->mtxopts.value;

        	lgenval = *lseqaddr;
        	ugenval = *useqaddr;

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_MARKPP | DBG_FUNC_NONE, (uint32_t)mutex, 1, lgenval, ugenval, 0);
#endif
		/* update the bits */
		oldval64 = (((uint64_t)ugenval) << 32);
		oldval64 |= lgenval;

		if ((lgenval & PTHRW_COUNT_MASK) == (ugenval & PTHRW_COUNT_MASK)) {
			clearprepost = 1;	
			lgenval &= ~PTH_RWL_PBIT;
			
		} else {
			lgenval |= PTH_RWL_PBIT;
		}
		newval64 = (((uint64_t)ugenval) << 32);
		newval64 |= lgenval;

		if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lseqaddr) == TRUE) {
#if _KSYN_TRACE_
			(void)__kdebug_trace(_KSYN_TRACE_UM_MARKPP | DBG_FUNC_NONE, (uint32_t)mutex, 2, lgenval, ugenval, 0);
#endif

			if (clearprepost != 0)
			 	__psynch_cvclrprepost(mutex, lgenval, ugenval, 0, 0, lgenval, (flags | _PTHREAD_MTX_OPT_MUTEX));
				
		} else  {
				goto retry;
		}
	
#if _KSYN_TRACE_
		(void)__kdebug_trace(_KSYN_TRACE_UM_MARKPP | DBG_FUNC_END, (uint32_t)mutex, 0, 0, 0, 0);
#endif
	}
	return(0);
}

/* 
 * For the new style mutex, interlocks are not held all the time.
 * We needed the signature to be set in the end. And we  need
 * to protect against the code getting reorganized by compiler.
 */
static void
__pthread_mutex_set_signature(npthread_mutex_t * mutex)
{
	mutex->sig = _PTHREAD_MUTEX_SIG;
}

int
pthread_mutex_lock(pthread_mutex_t *omutex)
{
	pthread_t self;
	uint64_t selfid;
	npthread_mutex_t * mutex = (npthread_mutex_t *)omutex;
	int sig = mutex->sig; 
#if NEVERINCOMPAGE || !USE_COMPAGE
	//uint32_t oldval, newval;
#endif
	int retval;
	int gotlock = 0, firstfit = 0;
	uint32_t updateval, lgenval, ugenval, nval, uval;
	volatile uint32_t * lseqaddr, *useqaddr;
	uint64_t oldval64, newval64;
#if USE_COMPAGE
	int sysret = 0;
	uint32_t mask;
#else
	int retrybit = 0;
#endif
	
	/* To provide backwards compat for apps using mutex incorrectly */
	if ((sig != _PTHREAD_MUTEX_SIG) && ((sig & _PTHREAD_MUTEX_SIG_init_MASK) != _PTHREAD_MUTEX_SIG_CMP)) {
		PLOCKSTAT_MUTEX_ERROR(omutex, EINVAL);
		return(EINVAL);
	}
	if (mutex->sig != _PTHREAD_MUTEX_SIG) {
		LOCK(mutex->lock);
		if ((mutex->sig & _PTHREAD_MUTEX_SIG_init_MASK) == _PTHREAD_MUTEX_SIG_CMP) {
		/* static initializer, init the mutex */
                	if(retval = _pthread_mutex_init(omutex, NULL, (mutex->sig & 0xf)) != 0){
                		UNLOCK(mutex->lock);
				PLOCKSTAT_MUTEX_ERROR(omutex, retval);
                        	return(retval);
                   	}
		} else if (mutex->sig != _PTHREAD_MUTEX_SIG) {
			UNLOCK(mutex->lock);
			PLOCKSTAT_MUTEX_ERROR(omutex, EINVAL);
			return(EINVAL);
		}
		UNLOCK(mutex->lock);
	}
	
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_LOCK | DBG_FUNC_START, (uint32_t)mutex, 0, 0, 0, 0);
#endif
	MUTEX_GETSEQ_ADDR(mutex, lseqaddr, useqaddr);

	self = pthread_self();
	(void) pthread_threadid_np(self, &selfid); 

	if (mutex->mtxopts.options.type != PTHREAD_MUTEX_NORMAL) {
		if (mutex->m_tid == selfid) {
			if (mutex->mtxopts.options.type == PTHREAD_MUTEX_RECURSIVE)
			{
				if (mutex->mtxopts.options.lock_count < USHRT_MAX)
				{
					mutex->mtxopts.options.lock_count++;
					PLOCKSTAT_MUTEX_ACQUIRE(omutex, 1, 0);
					retval = 0;
				} else {
					retval = EAGAIN;
					PLOCKSTAT_MUTEX_ERROR(omutex, retval);
				}
			} else	{ /* PTHREAD_MUTEX_ERRORCHECK */
				retval = EDEADLK;
				PLOCKSTAT_MUTEX_ERROR(omutex, retval);
			}
			return (retval);
		}
	}

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_LOCK | DBG_FUNC_NONE, (uint32_t)mutex, 1, 0, 0, 0);
#endif

#if  USE_COMPAGE /* [ */

ml0:
	mask = PTH_RWL_EBIT;
	retval = _commpage_pthread_mutex_lock(lseqaddr, mutex->mtxopts.value, selfid, mask, &mutex->m_tid, &sysret);
	if (retval == 0) {
		gotlock = 1;	
	} else if (retval == 1) {
		gotlock = 1;	
		updateval = sysret;
		/* returns 0 on succesful update */
		if (__mtx_updatebits( mutex, updateval, firstfit, 0, selfid) == 1) {
			/* could not acquire, may be locked in ffit case */
#if USE_COMPAGE
			LIBC_ABORT("comapge implementatin looping in libc \n");
#endif
			goto ml0;
		}
	} 
#if NEVERINCOMPAGE
	else if  (retval == 3) {
		cthread_set_errno_self(sysret);
		oldval = *lseqaddr;
		uval = *useqaddr;
		newval = oldval + PTHRW_INC;
		gotlock = 0;
		/* to block in the kerenl again */
	} 
#endif
	else {
		LIBC_ABORT("comapge implementation bombed \n");
	}
		

#else /* USECOMPAGE ][ */
retry:
	lgenval = *lseqaddr;
	ugenval = *useqaddr;
	
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_LOCK | DBG_FUNC_NONE, (uint32_t)mutex, 2, lgenval, ugenval, 0);
#endif /* _KSYN_TRACE_ */
	
	if((lgenval & PTH_RWL_EBIT) == 0) {
		gotlock = 1;
	} else {
		gotlock = 0;
	}

	oldval64 = (((uint64_t)ugenval) << 32);
	oldval64 |= lgenval;
	uval = ugenval;
	nval = (lgenval + PTHRW_INC) | (PTH_RWL_EBIT|PTH_RWL_KBIT);
	newval64 = (((uint64_t)uval) << 32);
	newval64 |= nval;

	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lseqaddr) == TRUE) {
#if _KSYN_TRACE_
		(void)__kdebug_trace(_KSYN_TRACE_UM_LOCK | DBG_FUNC_NONE, (uint32_t)mutex, 2, nval, uval, 0);
#endif
		if (gotlock != 0) {
			mutex->m_tid = selfid;
			goto out;
		}
	} else 
		goto retry;
	

	retrybit = 0;
	if (gotlock == 0) {
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_LOCK | DBG_FUNC_NONE, (uint32_t)mutex, 3, nval, uval, 0);
#endif
		firstfit = (mutex->mtxopts.options.policy == _PTHREAD_MUTEX_POLICY_FIRSTFIT);
ml1:
		updateval = __psynch_mutexwait(omutex, nval | retrybit, uval, mutex->m_tid,
							   mutex->mtxopts.value);
		
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_LOCK | DBG_FUNC_NONE, (uint32_t)mutex, 4, updateval, 0, 0);
#endif
		if (updateval == (uint32_t)-1) {
			goto ml1;
		}

		/* returns 0 on succesful update; in firstfit it may fail with 1 */
		if (__mtx_updatebits( mutex, PTHRW_INC | (PTH_RWL_KBIT | PTH_RWL_EBIT), firstfit, 0, selfid) == 1) {
			/* could not acquire, may be locked in ffit case */
			retrybit = PTH_RWL_RETRYBIT;
#if USE_COMPAGE
		LIBC_ABORT("comapge implementatin looping in libc \n");

#endif
			goto  ml1;
		}
	}
#endif /* USE_COMPAGE ] */
	
out:
	if (mutex->mtxopts.options.type == PTHREAD_MUTEX_RECURSIVE)
		mutex->mtxopts.options.lock_count = 1;

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_LOCK | DBG_FUNC_END, (uint32_t)mutex, 0, 0, 0, 0);
#endif
	return (0);
}

/*
 * Attempt to lock a mutex, but don't block if this isn't possible.
 */
int
pthread_mutex_trylock(pthread_mutex_t *omutex)
{
	npthread_mutex_t * mutex = (npthread_mutex_t *)omutex;
	int sig = mutex->sig;
	int error = 0;
	pthread_t self;
	uint64_t selfid;
	int gotlock = 0;
	uint32_t lgenval, ugenval, nval, uval;
	volatile uint32_t * lseqaddr, *useqaddr;
	uint64_t oldval64, newval64;
	
	/* To provide backwards compat for apps using mutex incorrectly */
	if ((sig != _PTHREAD_MUTEX_SIG) && ((sig & _PTHREAD_MUTEX_SIG_init_MASK) != _PTHREAD_MUTEX_SIG_CMP)) {
		PLOCKSTAT_MUTEX_ERROR(omutex, EINVAL);
		return(EINVAL);
	}
	
	if (mutex->sig != _PTHREAD_MUTEX_SIG) {
		LOCK(mutex->lock);
		if ((mutex->sig & _PTHREAD_MUTEX_SIG_init_MASK) == _PTHREAD_MUTEX_SIG_CMP) {
			/* static initializer, init the mutex */
			if((error = _pthread_mutex_init(omutex, NULL, (mutex->sig & 0xf))) != 0){
                                UNLOCK(mutex->lock);
				PLOCKSTAT_MUTEX_ERROR(omutex, error);
                                return(error);
                        }
		} else if (mutex->sig != _PTHREAD_MUTEX_SIG) {
			UNLOCK(mutex->lock);
			PLOCKSTAT_MUTEX_ERROR(omutex, EINVAL);
			return(EINVAL);
		}
		UNLOCK(mutex->lock);
	}
	
	MUTEX_GETSEQ_ADDR(mutex, lseqaddr, useqaddr);

	self = pthread_self();
	(void) pthread_threadid_np(self, &selfid); 

	if (mutex->mtxopts.options.type != PTHREAD_MUTEX_NORMAL) {
		if (mutex->m_tid == selfid) {
			if (mutex->mtxopts.options.type == PTHREAD_MUTEX_RECURSIVE)
			{
				if (mutex->mtxopts.options.lock_count < USHRT_MAX)
				{
					mutex->mtxopts.options.lock_count++;
					PLOCKSTAT_MUTEX_ACQUIRE(omutex, 1, 0);
					error = 0;
				} else {
					error = EAGAIN;
					PLOCKSTAT_MUTEX_ERROR(omutex, error);
				}
			} else	{ /* PTHREAD_MUTEX_ERRORCHECK */
				error = EDEADLK;
				PLOCKSTAT_MUTEX_ERROR(omutex, error);
			}
			return (error);
		}
	}
retry:
	lgenval = *lseqaddr;
	ugenval = *useqaddr;
	
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_LOCK | DBG_FUNC_NONE, (uint32_t)mutex, 2, lgenval, ugenval, 0);
#endif /* _KSYN_TRACE_ */
	

	oldval64 = (((uint64_t)ugenval) << 32);
	oldval64 |= lgenval;
	uval = ugenval;

	/* if we can acquire go ahead otherwise ensure it is still busy */
	if((lgenval & PTH_RWL_EBIT) == 0) {
		gotlock = 1;
		nval = (lgenval + PTHRW_INC) | (PTH_RWL_EBIT|PTH_RWL_KBIT);
	} else {
		nval = (lgenval | PTH_RWL_TRYLKBIT);
		gotlock = 0;
	}

	newval64 = (((uint64_t)uval) << 32);
	newval64 |= nval;

	/* set s and b bit */
	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lseqaddr) == TRUE) {
#if _KSYN_TRACE_
		(void)__kdebug_trace(_KSYN_TRACE_UM_LOCK | DBG_FUNC_NONE, (uint32_t)mutex, 2, nval, uval, 0);
#endif
		if (gotlock != 0) {
			mutex->m_tid = selfid;
			if (mutex->mtxopts.options.type == PTHREAD_MUTEX_RECURSIVE)
				mutex->mtxopts.options.lock_count = 1;
			PLOCKSTAT_MUTEX_ACQUIRE(omutex, 1, 0);
		} else  {
			error = EBUSY;
			PLOCKSTAT_MUTEX_ERROR(omutex, error);
		}
	} else 
		goto retry;
	

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_LOCK | DBG_FUNC_END, (uint32_t)mutex, 0xfafafafa, 0, error, 0);
#endif
	return (error);
}

/*
 * Unlock a mutex.
 * TODO: Priority inheritance stuff
 */
int
pthread_mutex_unlock(pthread_mutex_t *omutex)
{
	npthread_mutex_t * mutex = (npthread_mutex_t *)omutex;
	int retval;
	uint32_t mtxgen, mtxugen, flags, notify, updateval;
	int sig = mutex->sig; 
	pthread_t self;
	uint64_t selfid;
	volatile uint32_t * lseqaddr, *useqaddr;
	int firstfit = 0;
	
	/* To provide backwards compat for apps using mutex incorrectly */
	
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_UNLOCK | DBG_FUNC_START, (uint32_t)mutex, 0, 0, 0, 0);
#endif
	if ((sig != _PTHREAD_MUTEX_SIG) && ((sig & _PTHREAD_MUTEX_SIG_init_MASK) != _PTHREAD_MUTEX_SIG_CMP)) {
		PLOCKSTAT_MUTEX_ERROR(omutex, EINVAL);
		return(EINVAL);
	}
	
	if (mutex->sig != _PTHREAD_MUTEX_SIG) {
		LOCK(mutex->lock);
		if ((mutex->sig & _PTHREAD_MUTEX_SIG_init_MASK) == _PTHREAD_MUTEX_SIG_CMP) {
			/* static initializer, init the mutex */
                        if((retval = _pthread_mutex_init(omutex, NULL, (mutex->sig & 0xf))) != 0){
                                UNLOCK(mutex->lock);
				PLOCKSTAT_MUTEX_ERROR(omutex, retval);
                                return(retval);
                        }		
		} else if (mutex->sig != _PTHREAD_MUTEX_SIG) {
			UNLOCK(mutex->lock);
			PLOCKSTAT_MUTEX_ERROR(omutex, EINVAL);
			return(EINVAL);
		}
		UNLOCK(mutex->lock);
	}

	MUTEX_GETSEQ_ADDR(mutex, lseqaddr, useqaddr);

	notify = 0;
	retval = __mtx_droplock(mutex, PTHRW_INC, &flags, NULL, &mtxgen, &mtxugen);
	if (retval != 0)
		return(retval);
	
	if ((flags & _PTHREAD_MTX_OPT_NOTIFY) != 0) {
		firstfit = (mutex->mtxopts.options.policy == _PTHREAD_MUTEX_POLICY_FIRSTFIT);

		self = pthread_self();
		(void) pthread_threadid_np(self, &selfid); 

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_UNLOCK | DBG_FUNC_NONE, (uint32_t)mutex, 1, mtxgen, mtxugen, 0);
#endif
#if  USE_COMPAGE /* [ */
		if ((updateval = __psynch_mutexdrop((pthread_mutex_t *)lseqaddr, mtxgen, mtxugen, mutex->m_tid, flags)) == (uint32_t)-1) 
#else /* USECOMPAGE ][ */
		if ((updateval = __psynch_mutexdrop(omutex, mtxgen, mtxugen, mutex->m_tid, flags))== (uint32_t)-1) 
#endif /* USE_COMPAGE ] */
		{
			retval = errno;
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_UNLOCK | DBG_FUNC_END, (uint32_t)mutex, retval, 0, 0, 0);
#endif
			if (retval == 0)
				return(0);
			else if (errno == EINTR)
				return(0);
			else {
				LIBC_ABORT("__p_mutexdrop failed with error %d\n", retval);
				return(retval);
			}
		} else if (firstfit == 1) {
			if ((updateval & PTH_RWL_PBIT) != 0) {
				__mtx_markprepost(mutex, updateval, firstfit);
			}
		}
	}
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_UNLOCK | DBG_FUNC_END, (uint32_t)mutex, 0, 0, 0, 0);
#endif
	return(0);
}


/*
 * Initialize a mutex variable, possibly with additional attributes.
 */
int
_pthread_mutex_init(pthread_mutex_t *omutex, const pthread_mutexattr_t *attr, uint32_t static_type)
{
	npthread_mutex_t * mutex = (npthread_mutex_t *)omutex;
		
	if (attr)
	{
		if (attr->sig != _PTHREAD_MUTEX_ATTR_SIG)
			return (EINVAL);
		mutex->prioceiling = attr->prioceiling;
		mutex->mtxopts.options.protocol = attr->protocol;
		mutex->mtxopts.options.policy = attr->policy;
		mutex->mtxopts.options.type = attr->type;
		mutex->mtxopts.options.pshared = attr->pshared;
	} else {
                switch(static_type) {
                        case 1:
                                mutex->mtxopts.options.type = PTHREAD_MUTEX_ERRORCHECK;
                                break;
                        case 2:
                                mutex->mtxopts.options.type = PTHREAD_MUTEX_RECURSIVE;
                                break;
                        case 3:
				/* firstfit  fall thru */
                        case 7:
                                mutex->mtxopts.options.type = PTHREAD_MUTEX_DEFAULT;
                                break;
                        default:
                                return(EINVAL);
                }

		mutex->prioceiling = _PTHREAD_DEFAULT_PRIOCEILING;
		mutex->mtxopts.options.protocol = _PTHREAD_DEFAULT_PROTOCOL;
		if (static_type != 3)
			mutex->mtxopts.options.policy = _PTHREAD_MUTEX_POLICY_FAIRSHARE;
		else
			mutex->mtxopts.options.policy = _PTHREAD_MUTEX_POLICY_FIRSTFIT;
		mutex->mtxopts.options.pshared = _PTHREAD_DEFAULT_PSHARED;
	}
	
	mutex->mtxopts.options.notify = 0;
	mutex->mtxopts.options.rfu = 0;
	mutex->mtxopts.options.hold = 0;
	mutex->mtxopts.options.mutex = 1;
	mutex->mtxopts.options.lock_count = 0;
	/* address 8byte aligned? */
	if (((uintptr_t)mutex & 0x07) != 0) {
		/* 4byte alinged */
		mutex->mtxopts.options.misalign = 1;
#if defined(__LP64__)
		mutex->m_lseqaddr = &mutex->m_seq[0];
		mutex->m_useqaddr = &mutex->m_seq[1];
#else /* __LP64__ */
		mutex->m_lseqaddr = &mutex->m_seq[1];
		mutex->m_useqaddr = &mutex->m_seq[2];
#endif /* __LP64__ */
	} else {
		/* 8byte alinged */
		mutex->mtxopts.options.misalign = 0;
#if defined(__LP64__)
		mutex->m_lseqaddr = &mutex->m_seq[1];
		mutex->m_useqaddr = &mutex->m_seq[2];
#else /* __LP64__ */
		mutex->m_lseqaddr = &mutex->m_seq[0];
		mutex->m_useqaddr = &mutex->m_seq[1];
#endif /* __LP64__ */
	}
	mutex->m_tid = 0;
	mutex->m_seq[0] = 0;
	mutex->m_seq[1] = 0;
	mutex->m_seq[2] = 0;
	mutex->prioceiling = 0;
	mutex->priority = 0;
	/* 
	 * For the new style mutex, interlocks are not held all the time.
	 * We needed the signature to be set in the end. And we  need
	 * to protect against the code getting reorganized by compiler.
	 * mutex->sig = _PTHREAD_MUTEX_SIG;
	 */
	__pthread_mutex_set_signature(mutex);
	return (0);
}


/*
 * Destroy a mutex variable.
 */
int
pthread_mutex_destroy(pthread_mutex_t *omutex)
{
	int res;
	npthread_mutex_t * mutex = (npthread_mutex_t *)omutex;

	LOCK(mutex->lock);
	res = _pthread_mutex_destroy_locked(omutex);
	UNLOCK(mutex->lock);
	
	return(res);	
}


static int
_pthread_mutex_destroy_locked(pthread_mutex_t *omutex)
{
	int res;
	npthread_mutex_t * mutex = (npthread_mutex_t *)omutex;
	uint32_t lgenval, ugenval;
	volatile uint32_t * lseqaddr, *useqaddr;


	if (mutex->sig == _PTHREAD_MUTEX_SIG)
	{
		MUTEX_GETSEQ_ADDR(mutex, lseqaddr, useqaddr);

		lgenval = *(lseqaddr);
		ugenval = *(useqaddr);
		if ((mutex->m_tid == (uint64_t)0) &&
		    ((lgenval &  PTHRW_COUNT_MASK) == (ugenval &  PTHRW_COUNT_MASK)))
		{
			mutex->sig = _PTHREAD_NO_SIG;
			res = 0;
		}
		else
			res = EBUSY;
	} else if((mutex->sig & _PTHREAD_MUTEX_SIG_init_MASK )== _PTHREAD_MUTEX_SIG_CMP) {
		mutex->sig = _PTHREAD_NO_SIG;
                res = 0;
	} else
		res = EINVAL;

	return (res);
}


#endif /* !BUILDING_VARIANT ] */

/*
 * Destroy a mutex attribute structure.
 */
int
pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
#if __DARWIN_UNIX03
	if (__unix_conforming == 0)
		__unix_conforming = 1;
	if (attr->sig != _PTHREAD_MUTEX_ATTR_SIG)
		return (EINVAL);
#endif /* __DARWIN_UNIX03 */

        attr->sig = _PTHREAD_NO_SIG;  /* Uninitialized */
        return (0);
}



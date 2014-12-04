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
 */
/*
 * MkLinux
 */

/*
 * POSIX Pthread Library
 */

#include "pthread_internals.h"
#include <sys/time.h>              /* For struct timespec and getclock(). */
#include <stdio.h>

#ifdef PLOCKSTAT
#include "plockstat.h"
#else /* !PLOCKSTAT */
#define PLOCKSTAT_MUTEX_RELEASE(x, y)
#endif /* PLOCKSTAT */

extern int _pthread_cond_init(pthread_cond_t *, const pthread_condattr_t *, int);
extern int __unix_conforming;
extern int usenew_mtximpl;

#ifdef PR_5243343
/* 5243343 - temporary hack to detect if we are running the conformance test */
extern int PR_5243343_flag;
#endif /* PR_5243343 */

__private_extern__ int _pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime, int isRelative, int isconforming);
#ifndef BUILDING_VARIANT
static void cond_cleanup(void *arg);
static void cond_dropwait(npthread_cond_t * cond, int error, uint32_t updateval);
static void __pthread_cond_set_signature(npthread_cond_t * cond);
static int _pthread_cond_destroy_locked(pthread_cond_t *cond);
#endif

#if defined(__LP64__)
#define  COND_GETSEQ_ADDR(cond, c_lseqcnt, c_useqcnt, c_sseqcnt) \
{ \
	if (cond->misalign != 0) { \
		c_lseqcnt = &cond->c_seq[1]; \
		c_sseqcnt = &cond->c_seq[2]; \
		c_useqcnt = &cond->c_seq[0]; \
	} else { \
		/* aligned */ \
		c_lseqcnt = &cond->c_seq[0]; \
		c_sseqcnt = &cond->c_seq[1]; \
		c_useqcnt = &cond->c_seq[2]; \
	} \
}
#else /* __LP64__ */
#define  COND_GETSEQ_ADDR(cond, c_lseqcnt, c_useqcnt, c_sseqcnt) \
{ \
	if (cond->misalign != 0) { \
		c_lseqcnt = &cond->c_seq[1]; \
		c_sseqcnt = &cond->c_seq[2]; \
		c_useqcnt = &cond->c_seq[0]; \
	} else { \
		/* aligned */ \
		c_lseqcnt = &cond->c_seq[0]; \
		c_sseqcnt = &cond->c_seq[1]; \
		c_useqcnt = &cond->c_seq[2]; \
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
#define _KSYN_TRACE_UM_CVWAIT   0x9000070
#define _KSYN_TRACE_UM_CVSIG    0x9000074
#define _KSYN_TRACE_UM_CVBRD    0x9000078
#define _KSYN_TRACE_UM_CDROPWT    0x90000a0
#define _KSYN_TRACE_UM_CVCLRPRE    0x90000a4

#endif /* _KSYN_TRACE_ */


#ifndef BUILDING_VARIANT /* [ */

int
pthread_condattr_init(pthread_condattr_t *attr)
{
        attr->sig = _PTHREAD_COND_ATTR_SIG;
        attr->pshared = _PTHREAD_DEFAULT_PSHARED;
        return (0);
}

int       
pthread_condattr_destroy(pthread_condattr_t *attr)
{
        attr->sig = _PTHREAD_NO_SIG;  /* Uninitialized */
        return (0);
}

int
pthread_condattr_getpshared(const pthread_condattr_t *attr,
				int *pshared)
{
        if (attr->sig == _PTHREAD_COND_ATTR_SIG)
        {
                *pshared = (int)attr->pshared;
                return (0);
        } else
        {
                return (EINVAL); /* Not an initialized 'attribute' structure */
        }
}




/* temp home till pshared is fixed correctly */
int
pthread_condattr_setpshared(pthread_condattr_t * attr, int pshared)
{

        if (attr->sig == _PTHREAD_COND_ATTR_SIG)
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

__private_extern__ int       
_pthread_cond_init(pthread_cond_t *ocond,
		  const pthread_condattr_t *attr,
		  int conforming)
{
	npthread_cond_t * cond = (npthread_cond_t *)ocond;

	cond->busy = (npthread_mutex_t *)NULL;
	cond->c_seq[0] = 0;
	cond->c_seq[1] = 0;
	cond->c_seq[2] = 0;
	cond->rfu = 0;

	if (((uintptr_t)cond & 0x07) != 0) {
		cond->misalign = 1;
		cond->c_seq[2] = PTH_RWS_CV_CBIT;
	} else {
		cond->misalign = 0;
		cond->c_seq[1] = PTH_RWS_CV_CBIT;	/* set Sword to 0c */
	}
	if (conforming) {
		if (attr)
			cond->pshared = attr->pshared;
		else
			cond->pshared = _PTHREAD_DEFAULT_PSHARED;
	} else
		cond->pshared = _PTHREAD_DEFAULT_PSHARED;
	/* 
	 * For the new style mutex, interlocks are not held all the time.
	 * We needed the signature to be set in the end. And we  need
	 * to protect against the code getting reorganized by compiler.
	 * cond->sig = _PTHREAD_COND_SIG;
	 */
	__pthread_cond_set_signature(cond);
	return (0);
}

int
pthread_cond_destroy(pthread_cond_t * ocond)
{
	npthread_cond_t *cond = (npthread_cond_t *)ocond;
	int ret;

	/* to provide backwards compat for apps using united condtn vars */
	if((cond->sig != _PTHREAD_COND_SIG) && (cond->sig != _PTHREAD_COND_SIG_init))
		return(EINVAL);

	LOCK(cond->lock);
	ret = _pthread_cond_destroy_locked(ocond);
	UNLOCK(cond->lock);
	
	return(ret);
}

static int       
_pthread_cond_destroy_locked(pthread_cond_t * ocond)
{
	npthread_cond_t *cond = (npthread_cond_t *)ocond;
	int ret;
	volatile uint32_t * c_lseqcnt, *c_useqcnt, *c_sseqcnt;
	uint32_t lcntval , ucntval, scntval;
	uint64_t oldval64, newval64;

retry:
	if (cond->sig == _PTHREAD_COND_SIG)
	{
		COND_GETSEQ_ADDR(cond, c_lseqcnt, c_useqcnt, c_sseqcnt);
		lcntval = *c_lseqcnt;
		ucntval = *c_useqcnt;
		scntval = *c_sseqcnt;

		if ((lcntval & PTHRW_COUNT_MASK) == (scntval & PTHRW_COUNT_MASK)) {
			/* validate it is not busy */
			oldval64 = (((uint64_t)scntval) << 32);
			oldval64 |= lcntval;
			newval64 = oldval64;

			if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)c_lseqcnt) != TRUE)
				goto retry;
			cond->sig = _PTHREAD_NO_SIG;
			ret = 0;
		} else
			ret = EBUSY;
	} else if (cond->sig == _PTHREAD_COND_SIG_init) {
                cond->sig = _PTHREAD_NO_SIG;
                ret = 0;
	} else
		ret = EINVAL; /* Not an initialized condition variable structure */
	return (ret);
}

/*
 * Signal a condition variable, waking up all threads waiting for it.
 */
int       
pthread_cond_broadcast(pthread_cond_t *ocond)
{
	npthread_cond_t * cond = (npthread_cond_t *)ocond;
	int sig = cond->sig;
	uint32_t flags, updateval;
	uint32_t lcntval , ucntval, scntval;
	uint64_t oldval64, newval64, mugen, cvlsgen, cvudgen, mtid=0;
	int diffgen, error = 0;
	volatile uint32_t * c_lseqcnt, *c_useqcnt, *c_sseqcnt;
	uint32_t * pmtx = NULL;
	uint32_t nlval, ulval;
	int needclearpre = 0, retry_count = 0, uretry_count = 0;
	int ucountreset = 0;

	/* to provide backwards compat for apps using united condtn vars */
	if((sig != _PTHREAD_COND_SIG) && (sig != _PTHREAD_COND_SIG_init))
		return(EINVAL);

	if (sig != _PTHREAD_COND_SIG)
	{
		LOCK(cond->lock);
		if (cond->sig == _PTHREAD_COND_SIG_init)
		{
			_pthread_cond_init(ocond, NULL, 0);
			/* just inited nothing to post */
			UNLOCK(cond->lock);
			return (0);
		} else  if (cond->sig != _PTHREAD_COND_SIG) {
			/* Not a condition variable */
			UNLOCK(cond->lock);
			return (EINVAL);
		}
		UNLOCK(cond->lock);
	}

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVBRD | DBG_FUNC_START, (uint32_t)cond, 0, 0, 0, 0);
#endif

	COND_GETSEQ_ADDR(cond, c_lseqcnt, c_useqcnt, c_sseqcnt);
retry:
	lcntval = *c_lseqcnt;
	ucntval = *c_useqcnt;
	scntval = *c_sseqcnt;

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVBRD | DBG_FUNC_NONE, (uint32_t)cond, lcntval, ucntval, scntval, 0);
#endif
 
	if (((lcntval & PTHRW_COUNT_MASK)  == (scntval & PTHRW_COUNT_MASK)) ||
		((lcntval & PTHRW_COUNT_MASK)  == (ucntval & PTHRW_COUNT_MASK))) {
		/* validate it is spurious and return */
		oldval64 = (((uint64_t)scntval) << 32);
		oldval64 |= lcntval;
		newval64 = oldval64;

		if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)c_lseqcnt) != TRUE)
			goto retry;
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVBRD | DBG_FUNC_NONE, (uint32_t)cond, lcntval, ucntval, 0xf1f1f1f1, 0);
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVBRD | DBG_FUNC_END, (uint32_t)cond, scntval, 0, 0xf1f1f1f1, 0);
#endif
		return(0);
	}

	/* validate to eliminate spurious values, race snapshots */
	if (is_seqhigher((scntval & PTHRW_COUNT_MASK), (lcntval & PTHRW_COUNT_MASK))) {
		/* since ucntval may be newer, just redo */
		retry_count++;
		if (retry_count > 8192) {
			return(EAGAIN);
		} else {
			sched_yield();
			goto retry;
		}
	} else if (is_seqhigher((ucntval & PTHRW_COUNT_MASK), (lcntval & PTHRW_COUNT_MASK))) {
		/* since ucntval may be newer, just redo */
		uretry_count++;
		if (uretry_count > 8192) {
			/*
			 * U value if not used for a while can go out of sync
			 * set this to S value and try one more time.
			 */
			if (ucountreset != 0)
				return(EAGAIN);
			else
			if (OSAtomicCompareAndSwap32Barrier(ucntval, (scntval & PTHRW_COUNT_MASK), (volatile int32_t *)c_useqcnt) == TRUE) {
				/* now the U is reset to S value */
				ucountreset = 1;	
				uretry_count = 0;
			} 
		}
		sched_yield();
		goto retry;
	}

	if (is_seqlower(ucntval & PTHRW_COUNT_MASK, scntval & PTHRW_COUNT_MASK) != 0) {
		/* If U < S, set U = S+diff due to intr's TO, etc */
		ulval = (scntval & PTHRW_COUNT_MASK);
	} else {
		/* If U >= S, set U = U+diff due to intr's TO, etc */
		ulval = (ucntval & PTHRW_COUNT_MASK);
	}
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVBRD | DBG_FUNC_NONE, lcntval, ucntval, scntval, diffgen, 0);
#endif
	
	diffgen = diff_genseq((lcntval & PTHRW_COUNT_MASK), (ulval & PTHRW_COUNT_MASK));

	/* set U = L */
	ulval = (lcntval & PTHRW_COUNT_MASK);
	if (OSAtomicCompareAndSwap32Barrier(ucntval, ulval, (volatile int32_t *)c_useqcnt) != TRUE) {
		goto retry;
	}

	flags = 0;
	if (cond->pshared == PTHREAD_PROCESS_SHARED)
		flags |= _PTHREAD_MTX_OPT_PSHARED;
	pmtx = NULL;

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVBRD | DBG_FUNC_NONE, (uint32_t)cond, 3, diffgen, flags, 0);
#endif
	nlval = lcntval;

	/* pass old u val so kernel will know the diffgen */
	mugen = 0;
	cvlsgen = ((uint64_t)scntval << 32) | nlval;
	cvudgen = ((uint64_t)ucntval << 32) | diffgen;

	updateval = __psynch_cvbroad(ocond, cvlsgen, cvudgen, flags, (pthread_mutex_t *)pmtx, mugen, mtid);

	if (updateval != (uint32_t)-1) {

		/* if kernel granted woke some threads, updatwe S for them as they will not access cv on their way out */
		/* Were any threads woken or bits to be set? */
		if (updateval != 0) {
retry2:
			needclearpre = 0;
			lcntval = *c_lseqcnt;
			ucntval = *c_useqcnt;
			scntval = *c_sseqcnt;
			/* update scntval with number of expected returns  and bits */
			nlval = (scntval & PTHRW_COUNT_MASK) + (updateval & PTHRW_COUNT_MASK);
			/* set bits */
			nlval |= ((scntval & PTH_RWS_CV_BITSALL) | (updateval & PTH_RWS_CV_BITSALL));

#if _KSYN_TRACE_
			(void)__kdebug_trace(_KSYN_TRACE_UM_CVBRD | DBG_FUNC_NONE, 0x25, lcntval, scntval, updateval, 0);
#endif
			/* if L==S and c&p bits are set, needs clearpre */
			if (((nlval & PTHRW_COUNT_MASK) == (lcntval & PTHRW_COUNT_MASK)) 
				&& ((nlval & PTH_RWS_CV_BITSALL) == PTH_RWS_CV_BITSALL)) {
				/* reset p bit but retain c bit on the sword */
				nlval &= PTH_RWS_CV_RESET_PBIT;
				needclearpre = 1;
			}

			oldval64 = (((uint64_t)scntval) << 32);
			oldval64 |= lcntval;
			newval64 = (((uint64_t)nlval) << 32);
			newval64 |= lcntval;

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVBRD | DBG_FUNC_NONE, 0x25, nlval, scntval, updateval, 0);
#endif

			if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)c_lseqcnt) != TRUE)
				goto retry2;

			/* if L == S, then reset associated mutex */
			if ((nlval & PTHRW_COUNT_MASK) == (lcntval & PTHRW_COUNT_MASK)) {
				cond->busy = (npthread_mutex_t *)NULL;
			}
		
			if (needclearpre != 0) {
				(void)__psynch_cvclrprepost(ocond, lcntval, ucntval, nlval, 0, lcntval, flags);
			}
		}

	} 
	error = 0;

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVBRD | DBG_FUNC_END, (uint32_t)cond, 0, error, 0, 0);
#endif
	return(error);
}


/*
 * Signal a condition variable, waking a specified thread.
 */

int       
pthread_cond_signal_thread_np(pthread_cond_t *ocond, pthread_t thread)
{
	npthread_cond_t * cond = (npthread_cond_t *)ocond;
	int sig = cond->sig;
	uint32_t flags, updateval;
	uint32_t lcntval , ucntval, scntval;
	uint32_t nlval, ulval=0;
	volatile uint32_t * c_lseqcnt, *c_useqcnt, *c_sseqcnt;
	uint64_t oldval64, newval64, mugen, cvlsgen, mtid = 0;
	int needclearpre = 0, retry_count = 0, uretry_count = 0;
	int error, ucountreset = 0;

	/* to provide backwards compat for apps using united condtn vars */

	if((sig != _PTHREAD_COND_SIG) && (sig != _PTHREAD_COND_SIG_init))
		return(EINVAL);

	if (cond->sig != _PTHREAD_COND_SIG) {
		LOCK(cond->lock);
		if (cond->sig != _PTHREAD_COND_SIG) {
			if  (cond->sig == _PTHREAD_COND_SIG_init) {
				_pthread_cond_init(ocond, NULL, 0);
				/* just inited, nothing to post yet */
				UNLOCK(cond->lock);
				return(0);
			} else   {
				UNLOCK(cond->lock);
				return(EINVAL);
			}
		}
		UNLOCK(cond->lock);
	}

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVSIG | DBG_FUNC_START, (uint32_t)cond, 0, 0, 0, 0);
#endif
	COND_GETSEQ_ADDR(cond, c_lseqcnt, c_useqcnt, c_sseqcnt);
retry:
	lcntval = *c_lseqcnt;
	ucntval = *c_useqcnt;
	scntval = *c_sseqcnt;
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVSIG | DBG_FUNC_NONE, (uint32_t)cond, lcntval, ucntval, scntval, 0);
#endif

	if (((lcntval & PTHRW_COUNT_MASK)  == (scntval & PTHRW_COUNT_MASK)) ||
		((thread == 0) && ((lcntval & PTHRW_COUNT_MASK)  == (ucntval & PTHRW_COUNT_MASK)))) {
		/* If L <= S+U, it is spurious broadcasr */
		/* validate it is spurious and return */
		oldval64 = (((uint64_t)scntval) << 32);
		oldval64 |= lcntval;
		newval64 = oldval64;

		if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)c_lseqcnt) != TRUE)
			goto retry;
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVSIG | DBG_FUNC_NONE, (uint32_t)cond, lcntval, ucntval, 0xf1f1f1f1, 0);
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVSIG | DBG_FUNC_END, (uint32_t)cond, scntval, 0, 0xf1f1f1f1, 0);
#endif
		return(0);
	}

	if (thread == 0) {
		/* validate to eliminate spurious values, race snapshots */
		if (is_seqhigher((scntval & PTHRW_COUNT_MASK), (lcntval & PTHRW_COUNT_MASK))) {
			/* since ucntval may be newer, just redo */
			retry_count++;
			if (retry_count > 8192) {
				return(EAGAIN);
			} else {
				sched_yield();
				goto retry;
			}
        	} else if (is_seqhigher((ucntval & PTHRW_COUNT_MASK), (lcntval & PTHRW_COUNT_MASK))) {
			/* since ucntval may be newer, just redo */
			uretry_count++;
			if (uretry_count > 8192) {
				/*
				 * U value if not used for a while can go out of sync
				 * set this to S value and try one more time.
				 */
				if (ucountreset != 0)
					return(EAGAIN);
				else
				if (OSAtomicCompareAndSwap32Barrier(ucntval, (scntval & PTHRW_COUNT_MASK), (volatile int32_t *)c_useqcnt) == TRUE) {
					/* now the U is reset to S value */
					ucountreset = 1;	
					uretry_count = 0;
				} 
			}
			sched_yield();
			goto retry;
		}
	} /* thread == 0 ) */

	if (thread == 0) {
		/* 
		 * skip manipulating U count as ESRCH from kernel cannot be handled properly.
		 * S count will cover the imbalance and next signal without thread or broadcast
		 * will correct it. But we need to send the right U to kernel so it will use 
		 * that to look for the appropriate sequenc. So the ulval is computed anyway.
		 */

		if (is_seqlower(ucntval & PTHRW_COUNT_MASK, scntval & PTHRW_COUNT_MASK) != 0) {
			/* If U < S, set U = S+1 due to intr's TO, etc */
			ulval = (scntval & PTHRW_COUNT_MASK) + PTHRW_INC;
		} else {
			/* If U >= S, set U = U+1 due to intr's TO, etc */
			ulval = (ucntval & PTHRW_COUNT_MASK) + PTHRW_INC;
		}

		if (OSAtomicCompareAndSwap32Barrier(ucntval, ulval, (volatile int32_t *)c_useqcnt) != TRUE) {
			goto retry;
		}
	} 

	flags = 0;
	if (cond->pshared == PTHREAD_PROCESS_SHARED)
		flags |= _PTHREAD_MTX_OPT_PSHARED;
	
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVSIG | DBG_FUNC_NONE, (uint32_t)cond, 3, nlval, ulval, 0);
#endif
	nlval = lcntval;
	/* pass old u val so kernel will know the diffgen */
	mugen = 0;
	cvlsgen = ((uint64_t)scntval << 32) | nlval;

	updateval = __psynch_cvsignal(ocond, cvlsgen, ucntval, pthread_mach_thread_np(thread), (pthread_mutex_t *)0, mugen, mtid, flags);


	if (updateval != (uint32_t)-1) {

		/* if kernel granted woke some threads, updatwe S for them as they will not access cv on their way out */
		/* Were any threads woken or bits to be set? */
		if (updateval != 0) {
retry2:
			lcntval = *c_lseqcnt;
			ucntval = *c_useqcnt;
			scntval = *c_sseqcnt;
			/* update scntval with number of expected returns  and bits */
			nlval = (scntval & PTHRW_COUNT_MASK) + (updateval & PTHRW_COUNT_MASK);
			/* set bits */
			nlval |= ((scntval & PTH_RWS_CV_BITSALL) | (updateval & PTH_RWS_CV_BITSALL));

#if _KSYN_TRACE_
		(void)__kdebug_trace(_KSYN_TRACE_UM_CVSIG | DBG_FUNC_NONE, 0x25, 0, 0, updateval, 0);
#endif
			/* if L==S and c&p bits are set, needs clearpre */
			if (((nlval & PTHRW_COUNT_MASK) == (lcntval & PTHRW_COUNT_MASK)) 
				&& ((nlval & PTH_RWS_CV_BITSALL) == PTH_RWS_CV_BITSALL)) {
				/* reset p bit but retain c bit on the sword */
				nlval &= PTH_RWS_CV_RESET_PBIT;
				needclearpre = 1;
			} else
				needclearpre = 0;

			oldval64 = (((uint64_t)scntval) << 32);
			oldval64 |= lcntval;
			newval64 = (((uint64_t)nlval) << 32);
			newval64 |= lcntval;

#if _KSYN_TRACE_
(void)__kdebug_trace(_KSYN_TRACE_UM_CVSIG | DBG_FUNC_NONE, 0x25, nlval, ulval, updateval, 0);
#endif

			if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)c_lseqcnt) != TRUE)
				goto retry2;

			/* if L == S, then reset associated mutex */
			if ((nlval & PTHRW_COUNT_MASK) == (lcntval & PTHRW_COUNT_MASK)) {
				cond->busy = (npthread_mutex_t *)NULL;
			}
	
			if (needclearpre != 0) {
				(void)__psynch_cvclrprepost(ocond, lcntval, ucntval, nlval, 0, lcntval, flags);
			}
		}
	}

	error = 0;

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVSIG | DBG_FUNC_END, (uint32_t)cond, 0, 0, 0, 0);
#endif
	return (error);
}

/*
 * Signal a condition variable, waking only one thread.
 */
int
pthread_cond_signal(pthread_cond_t *cond)
{
	return pthread_cond_signal_thread_np(cond, NULL);
}

/*
 * Manage a list of condition variables associated with a mutex
 */


/*
 * Suspend waiting for a condition variable.
 * Note: we have to keep a list of condition variables which are using
 * this same mutex variable so we can detect invalid 'destroy' sequences.
 * If isconforming < 0, we skip the _pthread_testcancel(), but keep the
 * remaining conforming behavior..
 */
__private_extern__ int       
_pthread_cond_wait(pthread_cond_t *ocond, 
		   pthread_mutex_t *omutex,
		   const struct timespec *abstime,
		   int isRelative,
		    int isconforming)
{
	int retval;
	npthread_cond_t * cond = (npthread_cond_t *)ocond;
	npthread_mutex_t * mutex = (npthread_mutex_t * )omutex;
	mach_timespec_t then = {0,0};
	struct timespec cthen = {0,0};
	int sig = cond->sig;
	int msig = mutex->sig;
	npthread_mutex_t * pmtx;
	uint32_t mtxgen, mtxugen, flags=0, updateval;
	uint32_t lcntval , ucntval, scntval;
	uint32_t nlval, ulval, savebits;
	volatile uint32_t * c_lseqcnt, *c_useqcnt, *c_sseqcnt;
	uint64_t oldval64, newval64, mugen, cvlsgen;
	uint32_t * npmtx = NULL;
	int error, local_error;

extern void _pthread_testcancel(pthread_t thread, int isconforming);

	/* to provide backwards compat for apps using united condtn vars */
	if((sig != _PTHREAD_COND_SIG) && (sig != _PTHREAD_COND_SIG_init))
		return(EINVAL);

	if (isconforming) {
		if((msig != _PTHREAD_MUTEX_SIG) && ((msig & _PTHREAD_MUTEX_SIG_init_MASK) != _PTHREAD_MUTEX_SIG_CMP))		
			return(EINVAL);
		if (isconforming > 0)
			_pthread_testcancel(pthread_self(), 1);
	}

	if (cond->sig != _PTHREAD_COND_SIG)
	{
		LOCK(cond->lock);
		if (cond->sig != _PTHREAD_COND_SIG) {
			if  (cond->sig == _PTHREAD_COND_SIG_init) {
				_pthread_cond_init(ocond, NULL, 0);
			} else   {
				UNLOCK(cond->lock);
				return(EINVAL);
			}
		}
		UNLOCK(cond->lock);
	}

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVWAIT | DBG_FUNC_START, (uint32_t)cond, isRelative, 0, (uint32_t)abstime, 0);
#endif
	COND_GETSEQ_ADDR(cond, c_lseqcnt, c_useqcnt, c_sseqcnt);

	/* send relative time to kernel */
	if (abstime) {
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVWAIT | DBG_FUNC_START, 0x11111111, abstime->tv_nsec, abstime->tv_sec, 0, 0);
#endif
		if (isRelative == 0) {
			struct timespec now;
			struct timeval tv;
			gettimeofday(&tv, NULL);
			TIMEVAL_TO_TIMESPEC(&tv, &now);

			/* Compute relative time to sleep */
			then.tv_nsec = abstime->tv_nsec - now.tv_nsec;
			then.tv_sec = abstime->tv_sec - now.tv_sec;
			if (then.tv_nsec < 0)
			{
				then.tv_nsec += NSEC_PER_SEC;
				then.tv_sec--;
			}
			if (((int)then.tv_sec < 0) ||
				((then.tv_sec == 0) && (then.tv_nsec == 0)))
			{
				return ETIMEDOUT;
			}
			if (isconforming != 0) {
				cthen.tv_sec = abstime->tv_sec;
            			cthen.tv_nsec = abstime->tv_nsec;
            			if ((cthen.tv_sec < 0) || (cthen.tv_nsec < 0)) {
                			return EINVAL;
            			}
            			if (cthen.tv_nsec >= NSEC_PER_SEC) {
                			return EINVAL;
            			}
			}
		} else {
			then.tv_sec = abstime->tv_sec;
			then.tv_nsec = abstime->tv_nsec;
			if ((then.tv_sec == 0) && (then.tv_nsec == 0)) {
				return ETIMEDOUT;
			}
		}
		if(isconforming && ((then.tv_sec < 0) || (then.tv_nsec < 0))) {
			return EINVAL;
		}
		if (then.tv_nsec >= NSEC_PER_SEC) {
			return EINVAL;
		}
	}

	if ((cond->busy != (npthread_mutex_t *)NULL) &&  (cond->busy !=  mutex))
		return (EINVAL);

	pmtx = mutex; 
retry:
	lcntval = *c_lseqcnt;
	ucntval = *c_useqcnt;
	scntval = *c_sseqcnt;

	oldval64 = (((uint64_t)scntval) << 32);
	oldval64 |= lcntval;

	/* remove c and p bits on S word */
	savebits = scntval & PTH_RWS_CV_BITSALL;
	ulval = (scntval & PTHRW_COUNT_MASK);
	nlval = lcntval + PTHRW_INC;
	newval64 = (((uint64_t)ulval) << 32);
	newval64 |= nlval;

	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)c_lseqcnt) != TRUE)
		goto retry;

	cond->busy = mutex;

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVWAIT | DBG_FUNC_NONE, (uint32_t)cond, lcntval, ucntval, scntval, 0);
#endif
	retval = __mtx_droplock(pmtx, PTHRW_INC, &flags, &npmtx, &mtxgen, &mtxugen);

	/* TBD:  cases are for normal (non owner for recursive mutex; error checking)*/
	if (retval != 0)
		return(EINVAL);
	if ((flags & _PTHREAD_MTX_OPT_NOTIFY) == 0) {
		npmtx = NULL;
		mugen = 0;
	} else 
		mugen = ((uint64_t)mtxugen << 32) | mtxgen;
	flags &= ~_PTHREAD_MTX_OPT_MUTEX;	/* reset the mutex bit as this is cvar */
	
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVWAIT | DBG_FUNC_NONE, (uint32_t)cond, 3, (uint32_t)mutex, flags, 0);
#endif
	

	cvlsgen = ((uint64_t)(ulval | savebits)<< 32) | nlval;

	if (isconforming) {
		pthread_cleanup_push(cond_cleanup, (void *)cond);
		updateval = __psynch_cvwait(ocond, cvlsgen, ucntval, (pthread_mutex_t *)npmtx, mugen, flags, (int64_t)then.tv_sec, (int32_t)then.tv_nsec);
		_pthread_testcancel(pthread_self(), isconforming);
		pthread_cleanup_pop(0);
	} else {
		updateval = __psynch_cvwait(ocond, cvlsgen, ucntval, (pthread_mutex_t *)npmtx, mugen, flags, (int64_t)then.tv_sec, (int32_t)then.tv_nsec);

	}

	retval = 0;

	if (updateval == (uint32_t)-1) {
		local_error = errno;
		error = local_error & 0xff;
		if (error == ETIMEDOUT) {
			retval = ETIMEDOUT;
		} else if (error == EINTR) {
			/*
			**  EINTR can be treated as a spurious wakeup unless we were canceled.
			*/
			retval = 0;
		} else 
			retval =  EINVAL;
//#if _KSYN_TRACE_
//	(void)__kdebug_trace(0x9000070 | 0, (uint32_t)cond, 0xf1f1f2f2, local_error, error, 0);
//#endif

		/* add unlock ref to show one less waiter */
		cond_dropwait(cond, local_error, 0);
	} else {
//#if _KSYN_TRACE_
//	(void)__kdebug_trace(0x9000070 | 0, (uint32_t)cond, 0xf3f3f4f4, updateval, 0, 0);
//#endif
		/* succesful wait */
		if (updateval != 0) {
			/* the return due to prepost and might have bit states */
			/* update S and return for prepo if needed */
			cond_dropwait(cond, 0, updateval);
		}
		retval = 0;
	}
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVWAIT | DBG_FUNC_NONE, (uint32_t)cond, 4, retval, 0, 0);
#endif
	pthread_mutex_lock(omutex);

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CVWAIT | DBG_FUNC_END, (uint32_t)cond, 0, 0, retval, 0);
#endif
	return(retval);
}

/* 
 * For the new style mutex, interlocks are not held all the time.
 * We needed the signature to be set in the end. And we  need
 * to protect against the code getting reorganized by compiler.
 */
static void 
__pthread_cond_set_signature(npthread_cond_t  * cond)
{
	cond->sig = _PTHREAD_COND_SIG;
}


static void 
cond_cleanup(void *arg)
{
	npthread_cond_t *cond = (npthread_cond_t *)arg;
	pthread_mutex_t *mutex;

// 4597450: begin
	pthread_t thread = pthread_self();
	int thcanceled = 0;

	LOCK(thread->lock);
	thcanceled = (thread->detached & _PTHREAD_WASCANCEL);
	UNLOCK(thread->lock);

	if (thcanceled == 0)
		return;

// 4597450: end
    	mutex = (pthread_mutex_t *) cond->busy;
	
	/* add unlock ref to show one less waiter */
	cond_dropwait(cond, thread->cancel_error, 0);

	/*
	** Can't do anything if this fails -- we're on the way out
	*/
	if (mutex != NULL)
    		(void)pthread_mutex_lock(mutex);
}

#define ECVCERORR       256
#define ECVPERORR       512

void
cond_dropwait(npthread_cond_t * cond, int error, uint32_t updateval)
{
	int sig = cond->sig;
	pthread_cond_t * ocond = (pthread_cond_t *)cond;
	int needclearpre = 0;
	uint32_t diffgen, nlval, ulval, flags;
	uint32_t lcntval , ucntval, scntval, lval;
	volatile uint32_t * c_lseqcnt, *c_useqcnt, *c_sseqcnt;
	uint64_t oldval64, newval64;

	/* to provide backwards compat for apps using united condtn vars */

	if (sig != _PTHREAD_COND_SIG) 
		return;

	COND_GETSEQ_ADDR(cond, c_lseqcnt, c_useqcnt, c_sseqcnt);

	if (error != 0) {
		lval = PTHRW_INC;
		if ((error & ECVCERORR) != 0)
			lval |= PTH_RWS_CV_CBIT;
		if ((error & ECVPERORR) != 0)
			lval |= PTH_RWS_CV_PBIT;
	} else {
		lval = updateval;
	}
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CDROPWT | DBG_FUNC_START, (uint32_t)cond, error, updateval, 0xee, 0);
#endif
retry:
	lcntval = *c_lseqcnt;
	ucntval = *c_useqcnt;
	scntval = *c_sseqcnt;

	diffgen = diff_genseq((lcntval & PTHRW_COUNT_MASK),  (scntval & PTHRW_COUNT_MASK));	/* pendig waiters */
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CDROPWT | DBG_FUNC_NONE, (uint32_t)cond, lcntval, scntval, diffgen, 0);
#endif
	if (diffgen <= 0) {
		/* TBD: Assert, should not be the case */
		/* validate it is spurious and return */
		oldval64 = (((uint64_t)scntval) << 32);
		oldval64 |= lcntval;
		newval64 = oldval64;
		if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)c_lseqcnt) != TRUE)
			goto retry;
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CDROPWT | DBG_FUNC_END, (uint32_t)cond, 0, 0, 0, 0);
#endif
		return;
	}

	/* update S by one */
	oldval64 = (((uint64_t)scntval) << 32);
	oldval64 |= lcntval;

	/* update scntval with number of expected returns  and bits */
	ulval = (scntval & PTHRW_COUNT_MASK) + (lval & PTHRW_COUNT_MASK);
	/* set bits */
	ulval |= ((scntval & PTH_RWS_CV_BITSALL) | (lval & PTH_RWS_CV_BITSALL));

	nlval = lcntval;
		
	needclearpre = 0;

	/* If L==S, need to return to kernel */
	if ((nlval & PTHRW_COUNT_MASK) == (ulval & PTHRW_COUNT_MASK)) {
		if ((ulval & PTH_RWS_CV_BITSALL) == PTH_RWS_CV_BITSALL) {
			/* reset p bit but retain c bit on the sword */
			needclearpre = 1;
			ulval &= PTH_RWS_CV_RESET_PBIT;
		} 
	}

	newval64 = (((uint64_t)ulval) << 32);
	newval64 |= nlval;

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CDROPWT | DBG_FUNC_NONE, (uint32_t)cond, 0xffff, nlval, ulval, 0);
#endif
	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)c_lseqcnt) != TRUE)
		goto retry;

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CDROPWT | DBG_FUNC_NONE, (uint32_t)cond, 2, 0, 0xee, 0);
#endif
	if ((nlval & PTHRW_COUNT_MASK) == (ulval & PTHRW_COUNT_MASK)) {
		/* last usage remove the mutex */
		cond->busy = NULL;
	}

#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CDROPWT | DBG_FUNC_NONE, nlval, ucntval, ulval, PTHRW_INC, 0);
#endif
	if (needclearpre != 0) {
		flags = 0;
		if (cond->pshared == PTHREAD_PROCESS_SHARED)
			flags |= _PTHREAD_MTX_OPT_PSHARED;
		/* reset prepost */
		(void)__psynch_cvclrprepost(ocond, nlval, ucntval, ulval, 0, nlval, flags);
	}
#if _KSYN_TRACE_
	(void)__kdebug_trace(_KSYN_TRACE_UM_CDROPWT | DBG_FUNC_END, nlval, ucntval, ulval, PTHRW_INC, 0);
#endif
	return;
}


int       
pthread_cond_timedwait_relative_np(pthread_cond_t *cond, 
		       pthread_mutex_t *mutex,
		       const struct timespec *abstime)
{
	return (_pthread_cond_wait(cond, mutex, abstime, 1, 0));
}



#else /* !BUILDING_VARIANT */

extern int _pthread_cond_wait(pthread_cond_t *cond, 
			pthread_mutex_t *mutex,
			const struct timespec *abstime,
			int isRelative,
			int isconforming);

#endif /* !BUILDING_VARIANT ] */
/*
 * Initialize a condition variable.  Note: 'attr' is ignored.
 */

/*
 * Initialize a condition variable.  This is the public interface.
 * We can't trust the lock, so initialize it first before taking
 * it.
 */
int       
pthread_cond_init(pthread_cond_t *cond,
		  const pthread_condattr_t *attr)
{
	int conforming;

#if __DARWIN_UNIX03
        conforming = 1;
#else /* __DARWIN_UNIX03 */
        conforming = 0;
#endif /* __DARWIN_UNIX03 */

	/* lock is same offset in both structures */
	LOCK_INIT(cond->lock);

	return (_pthread_cond_init(cond, attr, conforming));
}

/*
int       
pthread_cond_wait(pthread_cond_t *cond, 
		  pthread_mutex_t *mutex)

int       
pthread_cond_timedwait(pthread_cond_t *cond, 
		       pthread_mutex_t *mutex,
		       const struct timespec *abstime)

moved to pthread_cancelable.c */

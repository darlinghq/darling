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
/*-
 * Copyright (c) 1998 Alex Nash
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/lib/libc_r/uthread/uthread_rwlock.c,v 1.6 2001/04/10 04:19:20 deischen Exp $
 */

/* 
 * POSIX Pthread Library 
 * -- Read Write Lock support
 * 4/24/02: A. Ramesh
 *	   Ported from FreeBSD
 */

#include "pthread_internals.h"
#include <stdio.h>      /* For printf(). */

extern int __unix_conforming;

#ifdef PLOCKSTAT
#include "plockstat.h"
#else /* !PLOCKSTAT */
#define PLOCKSTAT_RW_ERROR(x, y, z)
#define PLOCKSTAT_RW_BLOCK(x, y)
#define PLOCKSTAT_RW_BLOCKED(x, y, z)
#define PLOCKSTAT_RW_ACQUIRE(x, y)    
#define PLOCKSTAT_RW_RELEASE(x, y)
#endif /* PLOCKSTAT */

#define READ_LOCK_PLOCKSTAT  0
#define WRITE_LOCK_PLOCKSTAT 1

#define BLOCK_FAIL_PLOCKSTAT    0
#define BLOCK_SUCCESS_PLOCKSTAT 1

/* maximum number of times a read lock may be obtained */
#define	MAX_READ_LOCKS		(INT_MAX - 1) 


#ifndef BUILDING_VARIANT /* [ */
__private_extern__ int usenew_impl = 1;
#else /* BUILDING_VARIANT */
extern int usenew_impl;
#endif /* BUILDING_VARIANT */

extern int PR_5243343_flag;

#if defined(__LP64__)
#define RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr) \
{ \
                if (rwlock->misalign != 0) { \
                        lcntaddr = &rwlock->rw_seq[1]; \
			seqaddr = &rwlock->rw_seq[2]; \
                        ucntaddr = &rwlock->rw_seq[3]; \
                 } else { \
                        lcntaddr = &rwlock->rw_seq[0]; \
			seqaddr = &rwlock->rw_seq[1]; \
                        ucntaddr = &rwlock->rw_seq[2]; \
                } \
}
#else /* __LP64__ */
#define RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr) \
{ \
                if (rwlock->misalign != 0) { \
                        lcntaddr = &rwlock->rw_seq[1]; \
			seqaddr = &rwlock->rw_seq[2]; \
                        ucntaddr = &rwlock->rw_seq[3]; \
                 } else { \
                        lcntaddr = &rwlock->rw_seq[0]; \
			seqaddr = &rwlock->rw_seq[1]; \
                        ucntaddr = &rwlock->rw_seq[2]; \
                } \
}
#endif /* __LP64__ */

__private_extern__ int __pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);


#define _KSYN_TRACE_ 0

#if _KSYN_TRACE_
#include <sys/sysctl.h>
#ifndef BUILDING_VARIANT /* [ */
static void set_enable(int);
#endif /* !BUILDING_VARIANT ] */

/* The Function qualifiers  */
#define DBG_FUNC_START          1
#define DBG_FUNC_END            2
#define DBG_FUNC_NONE           0

int __kdebug_trace(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

#define _KSYN_TRACE_RW_RDLOCK     0x9000080
#define _KSYN_TRACE_RW_WRLOCK	 0x9000084
#define _KSYN_TRACE_RW_UNLOCK    0x9000088
#define _KSYN_TRACE_RW_UNACT1    0x900808c
#define _KSYN_TRACE_RW_UNACT2    0x9008090
#define _KSYN_TRACE_RW_UNACTK    0x9008094
#define _KSYN_TRACE_RW_UNACTE    0x9008098
#define _KSYN_TRACE_RW_UNACTR    0x900809c
#define _KSYN_TRACE_RW_TOOMANY    0x90080a0
#define _KSYN_TRACE_RW_TRYWRLOCK    0x90080a4
#define _KSYN_TRACE_RW_TRYRDLOCK    0x90080a8
#endif /* _KSYN_TRACE_ */

__private_extern__ void rwlock_action_onreturn(pthread_rwlock_t * rwlock, uint32_t updateval);
__private_extern__ int rw_diffgenseq(uint32_t x, uint32_t y);

#ifndef BUILDING_VARIANT /* [ */
static uint32_t modbits(uint32_t lgenval, uint32_t updateval, uint32_t savebits);

int
pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
{
        attr->sig = _PTHREAD_RWLOCK_ATTR_SIG;
	attr->pshared = _PTHREAD_DEFAULT_PSHARED;
        return (0);
}

int       
pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
{
        attr->sig = _PTHREAD_NO_SIG;  /* Uninitialized */
	attr->pshared = 0;
        return (0);
}

int
pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr,
				int *pshared)
{
        if (attr->sig == _PTHREAD_RWLOCK_ATTR_SIG)
        {
		*pshared = (int)attr->pshared; 
                return (0);
        } else
        {
                return (EINVAL); /* Not an initialized 'attribute' structure */
        }
}


int
pthread_rwlockattr_setpshared(pthread_rwlockattr_t * attr, int pshared)
{
        if (attr->sig == _PTHREAD_RWLOCK_ATTR_SIG)
        {
#if __DARWIN_UNIX03
                if (( pshared == PTHREAD_PROCESS_PRIVATE) || (pshared == PTHREAD_PROCESS_SHARED))
#else /* __DARWIN_UNIX03 */
                if ( pshared == PTHREAD_PROCESS_PRIVATE)
#endif /* __DARWIN_UNIX03 */
                {
						attr->pshared = pshared ;
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

__private_extern__ int
__pthread_rwlock_init(pthread_rwlock_t * orwlock, const pthread_rwlockattr_t *attr)
{
	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;

	if ((attr != NULL) && (attr->pshared == PTHREAD_PROCESS_SHARED)) {
		rwlock->pshared = PTHREAD_PROCESS_SHARED;
		rwlock->rw_flags = PTHRW_KERN_PROCESS_SHARED;
	 } else {
		rwlock->pshared = _PTHREAD_DEFAULT_PSHARED;
		rwlock->rw_flags = PTHRW_KERN_PROCESS_PRIVATE;
	}
	
	if (((uintptr_t)rwlock & 0x07) != 0) {
		rwlock->misalign = 1;
#if defined(__LP64__)
		rwlock->rw_lcntaddr = &rwlock->rw_seq[1];
		rwlock->rw_seqaddr = &rwlock->rw_seq[2];
		rwlock->rw_ucntaddr = &rwlock->rw_seq[3];
		rwlock->rw_seq[1]= PTHRW_RWLOCK_INIT;
		rwlock->rw_seq[2]= PTHRW_RWS_INIT;
		rwlock->rw_seq[3]= 0;
#else /* __LP64__ */
		rwlock->rw_lcntaddr = &rwlock->rw_seq[1];
		rwlock->rw_seqaddr = &rwlock->rw_seq[2];
		rwlock->rw_ucntaddr = &rwlock->rw_seq[3];
		rwlock->rw_seq[1]= PTHRW_RWLOCK_INIT;
		rwlock->rw_seq[2]= PTHRW_RWS_INIT;
		rwlock->rw_seq[3]= 0;
#endif /* __LP64__ */
		
	} else {
		rwlock->misalign = 0;
#if defined(__LP64__)
		rwlock->rw_lcntaddr = &rwlock->rw_seq[0];
		rwlock->rw_seqaddr = &rwlock->rw_seq[1];
		rwlock->rw_ucntaddr = &rwlock->rw_seq[2];
		rwlock->rw_seq[0]= PTHRW_RWLOCK_INIT;
		rwlock->rw_seq[1]= PTHRW_RWS_INIT;
		rwlock->rw_seq[2]= 0;
#else /* __LP64__ */
		rwlock->rw_lcntaddr = &rwlock->rw_seq[0];
		rwlock->rw_seqaddr = &rwlock->rw_seq[1];
		rwlock->rw_ucntaddr = &rwlock->rw_seq[2];
		rwlock->rw_seq[0]= PTHRW_RWLOCK_INIT;
		rwlock->rw_seq[1]= PTHRW_RWS_INIT;
		rwlock->rw_seq[2]= 0;
#endif /* __LP64__ */
		
	}

	rwlock->reserv = 0;
	rwlock->rw_owner = NULL;
#if defined(__LP64__)
	memset(rwlock->rfu, 0, PTHRW_RFU_64BIT);
#else
	memset(rwlock->rfu, 0, PTHRW_RFU_32BIT);
#endif 

	rwlock->sig = _PTHREAD_RWLOCK_SIG;
	
	return(0);
}

#if _KSYN_TRACE_
static void
set_enable(int val)
{
	int mib[6];
	size_t needed = 0;

    mib[0] = CTL_KERN;
	mib[1] = KERN_KDEBUG;
	mib[2] = KERN_KDENABLE;
	mib[3] = val;
	mib[4] = 0;
	mib[5] = 0;
	/* best effort to stop the trace */
	(void)sysctl(mib, 4, NULL, &needed, NULL, 0);
}
#endif

static uint32_t
modbits(uint32_t lgenval, uint32_t updateval, uint32_t savebits)
{
	uint32_t lval = lgenval & PTHRW_BIT_MASK;
	uint32_t uval = updateval & PTHRW_BIT_MASK;
	uint32_t rval, nlval;

	nlval = (lval | uval) & ~(PTH_RWL_MBIT);
	
	/* reconcile bits on the lock with what kernel needs to set */
	if ((uval & PTH_RWL_LBIT) != 0)
		nlval &= ~PTH_RWL_KBIT;
	else if (((uval & PTH_RWL_KBIT) == 0) && ((lval & PTH_RWL_WBIT) == 0))
		nlval &= ~PTH_RWL_KBIT;

	if (savebits !=0 ) {
		if (((savebits & PTH_RWS_WSVBIT) != 0) && ((nlval & PTH_RWL_WBIT) == 0) && 
				((nlval & PTH_RWL_EBIT) == 0)) {
			if ((nlval & PTH_RWL_LBIT) == 0)
				nlval |= (PTH_RWL_WBIT | PTH_RWL_KBIT);
			else
				nlval |= PTH_RWL_WBIT;
		}
		if (((savebits & PTH_RWS_YSVBIT) != 0) && ((nlval & PTH_RWL_YBIT) == 0) && 
				((nlval & PTH_RWL_EBIT) == 0)) {
				nlval |= PTH_RWL_YBIT;
		}
		if (((savebits & PTH_RWS_USVBIT) != 0) && ((nlval & PTH_RWL_EBIT) == 0)) {
			if ((nlval & PTH_RWL_LBIT) == 0)
				nlval |= (PTH_RWL_UBIT | PTH_RWL_KBIT);
			else
				nlval |= PTH_RWL_UBIT;
		}
	}
	rval = (lgenval & PTHRW_COUNT_MASK) | nlval;
	return(rval);
}


__private_extern__ void
rwlock_action_onreturn(pthread_rwlock_t * orwlock, uint32_t updateval)
{

	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;
	uint32_t lcntval, rw_seq, newval = 0, newsval, lval, uval;
	volatile uint32_t * lcntaddr, *ucntaddr, *seqaddr;
	uint64_t oldval64, newval64;
	int setbits = 0;
	int overlap = 0;
	uint32_t savebits = 0;
	int isoverlap = 0;
	
	/* TBD: restore U bit */
	if (rwlock->pshared == PTHREAD_PROCESS_SHARED) {
		RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr);
	} else {
		lcntaddr = rwlock->rw_lcntaddr;
		seqaddr = rwlock->rw_seqaddr;
	}

#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNACT1 | DBG_FUNC_START, updateval, 0, 0, 0, 0);
#endif

	isoverlap = updateval & PTH_RWL_MBIT;

loop:
	setbits = 0;
	lcntval = *lcntaddr;
	rw_seq = *seqaddr;
	savebits = 0;
	
	if (isoverlap != 0) {
		/* overlap return, just increment and inspect bits */
		setbits = 1;
		overlap = 1;
		/* set s word, increment by specified value  */
		newsval = rw_seq + (updateval & PTHRW_COUNT_MASK);
		if ((newsval &  PTHRW_RWS_SAVEMASK) != 0) {
			savebits = newsval & PTHRW_RWS_SAVEMASK;
			newsval &= ~PTHRW_RWS_SAVEMASK;
		}
	} else {
		/* normal return */
		if (is_rws_setunlockinit(rw_seq) != 0) {
			setbits = 1;
			/* set s word to passed in value */
			newsval = (rw_seq & PTHRW_COUNT_MASK) + (updateval & PTHRW_COUNT_MASK);
			if ((rw_seq &  PTHRW_RWS_SAVEMASK) != 0) {
				savebits = rw_seq & PTHRW_RWS_SAVEMASK;
				newsval &= ~PTHRW_RWS_SAVEMASK;
			}
		} else {
			newval = lcntval;
			newsval = rw_seq;
		}
	}
	if (setbits != 0) {
		newval = modbits(lcntval, updateval, savebits);

#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNACT1 | DBG_FUNC_NONE, rw_seq, newsval, 0xeeeeeeee, updateval, 0);
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNACT1 | DBG_FUNC_NONE, lcntval, newval, 0xeeeeeeee, updateval, 0);
#endif
		oldval64 = (((uint64_t)rw_seq) << 32);
		oldval64 |= lcntval;
		newval64 = (((uint64_t)newsval) << 32);
		newval64 |= newval;
	
		if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) != TRUE)
			goto loop;
		/* Check for consistency */
		lval = lcntval & PTHRW_BIT_MASK;
		uval = updateval & PTHRW_BIT_MASK;
	}

#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNACT1 | DBG_FUNC_END, rw_seq, newsval, 0xffffffff, 0, 0);
#endif
	return;
}

/* returns are not bit shifted */
__private_extern__ int
rw_diffgenseq(uint32_t x, uint32_t y)
{
	uint32_t lx = (x & PTHRW_COUNT_MASK);
	uint32_t ly = (y &PTHRW_COUNT_MASK);

	if (lx  > ly) {
		return(lx-ly);
	} else {
		return((PTHRW_MAX_READERS - y) + lx + PTHRW_INC);
	}

}

#ifdef NOTYET
/********************************************************** */
static int pthread_rwlock_upgrade_internal(pthread_rwlock_t * orwlock, int trylock);

int
pthread_rwlock_longrdlock_np(pthread_rwlock_t * orwlock)
{
	pthread_t self;
	uint32_t lcntval, ucntval, rw_seq, newval, newsval, updateval;
	int error = 0, retry_count = 0;
	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;
	uint64_t oldval64, newval64;
	volatile uint32_t * lcntaddr, *ucntaddr, *seqaddr;
	uint64_t myid = 0;
	
	if (rwlock->sig != _PTHREAD_RWLOCK_SIG) {
		LOCK(rwlock->lock);
		if (rwlock->sig == _PTHREAD_RWLOCK_SIG_init) {
		 	if ((error = __pthread_rwlock_init(orwlock, NULL)) != 0)  {
				UNLOCK(rwlock->lock);
				PLOCKSTAT_RW_ERROR(orwlock, READ_LOCK_PLOCKSTAT, error);
				return(error);
			}
		} else if (rwlock->sig != _PTHREAD_RWLOCK_SIG){
			UNLOCK(rwlock->lock);
			PLOCKSTAT_RW_ERROR(orwlock, READ_LOCK_PLOCKSTAT, EINVAL);
			return(EINVAL);
		}
		UNLOCK(rwlock->lock);
	}
	
	if (rwlock->pshared == PTHREAD_PROCESS_SHARED) {
		RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr);
	} else {
		lcntaddr = rwlock->rw_lcntaddr;
		ucntaddr = rwlock->rw_ucntaddr;
		seqaddr = rwlock->rw_seqaddr;
	}

loop:
	lcntval = *lcntaddr;
	ucntval = *ucntaddr;
	rw_seq = *seqaddr;
	
	if (can_rwl_longreadinuser(lcntval))
		goto gotlock;

#if __DARWIN_UNIX03
	if (is_rwl_ebit_set(lcntval)) {
		self = pthread_self();
		if(rwlock->rw_owner == self) {
			error = EDEADLK;
			goto out;
		}
	}
#endif /* __DARWIN_UNIX03 */
	
	/* need to block in kernel */
	newval  = (lcntval + PTHRW_INC);

	newsval = rw_seq;
	if (is_rws_setseq(rw_seq)) {
		newsval &= PTHRW_SW_Reset_BIT_MASK;
		newsval |= (newval & PTHRW_COUNT_MASK);
	}

	/* update lock seq and  block in kernel */
	
	oldval64 = (((uint64_t)rw_seq) << 32);
	oldval64 |= lcntval;
	
	newval64 = (((uint64_t)(newsval)) << 32);
	newval64 |= newval;
	
	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) != TRUE)
		goto loop;
kblock:
	updateval = __psynch_rw_longrdlock(orwlock, newval, ucntval, newsval, rwlock->rw_flags);
	if (updateval == (uint32_t)-1) {
		error = errno;
	} else
		error = 0;
	
	if (error == EINTR)
		goto kblock;
	
	if (error == 0) {
		rwlock_action_onreturn(orwlock, updateval);
		if ( is_rwl_lbit_clear(updateval)) {
#if _KSYN_TRACE_
			set_enable(2);
#endif /* _KSYN_TRACE_ */
			(void)pthread_threadid_np(pthread_self(), &myid);
			LIBC_ABORT("yieldwrlock from kernel without EBit  %x: tid %x\n", updateval, (uint32_t)myid);
			/* kernel cannot wakeup without granting E bit */
		}
		goto successout;
	} else {
#if _KSYN_TRACE_
		set_enable(2);
#endif /* _KSYN_TRACE_ */
		(void)pthread_threadid_np(pthread_self(), &myid);
		LIBC_ABORT("yieldwrlock from kernel with unknown error %x: tid %x\n", updateval, (uint32_t)myid);
		goto out;
	}
	
gotlock:
	if (rw_diffgenseq(lcntval, ucntval) >= PTHRW_MAX_READERS) {
		/* since ucntval may be newer, just redo */
		retry_count++;
		if (retry_count > 1024) {
		
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_TOOMANY | DBG_FUNC_NONE, (uint32_t)rwlock, 0XEEEEEEEE, lcntval, ucntval, 0);
#endif
			error = EAGAIN;
			goto out;
		} else {
			sched_yield();
			goto loop;
		}
	}
	
	/* Need to update L and S word */
	newval  = (lcntval + PTHRW_INC) | PTH_RWL_LBIT;
	newsval = (rw_seq + PTHRW_INC);

	oldval64 = (((uint64_t)rw_seq) << 32);
	oldval64 |= lcntval;
	newval64 = (((uint64_t)newsval) << 32);
	newval64 |= newval;
	
	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) != TRUE)
		goto loop;

successout:
	PLOCKSTAT_RW_ACQUIRE(orwlock, READ_LOCK_PLOCKSTAT);
	return(0);
out:
	PLOCKSTAT_RW_ERROR(orwlock, READ_LOCK_PLOCKSTAT, error);    
	return(error);
}

int
pthread_rwlock_yieldwrlock_np(pthread_rwlock_t * orwlock)
{
	uint32_t lcntval, ucntval, rw_seq, newval, newsval, updateval;
	int error = 0;
#if __DARWIN_UNIX03
	pthread_t self = pthread_self();
#endif /* __DARWIN_UNIX03 */
	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;
	uint64_t oldval64, newval64;
	volatile uint32_t * lcntaddr, *ucntaddr, *seqaddr;
	uint64_t myid = 0;
	
	if (rwlock->sig != _PTHREAD_RWLOCK_SIG) {
		LOCK(rwlock->lock);
		if (rwlock->sig == _PTHREAD_RWLOCK_SIG_init) {
		 	if ((error = __pthread_rwlock_init(orwlock, NULL)) != 0)  {
				UNLOCK(rwlock->lock);
				PLOCKSTAT_RW_ERROR(orwlock, WRITE_LOCK_PLOCKSTAT, error);
				return(error);
			}
		} else if (rwlock->sig != _PTHREAD_RWLOCK_SIG){
			UNLOCK(rwlock->lock);
			PLOCKSTAT_RW_ERROR(orwlock, WRITE_LOCK_PLOCKSTAT, EINVAL);
			return(EINVAL);
		}
		UNLOCK(rwlock->lock);
	}

	if (rwlock->pshared == PTHREAD_PROCESS_SHARED) {
		RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr);
	} else {
		lcntaddr = rwlock->rw_lcntaddr;
		ucntaddr = rwlock->rw_ucntaddr;
		seqaddr = rwlock->rw_seqaddr;
	}

loop:
	lcntval = *lcntaddr;
	ucntval = *ucntaddr;
	rw_seq = *seqaddr;

#if __DARWIN_UNIX03
	if (is_rwl_ebit_set(lcntval)) {
		if (rwlock->rw_owner == self) {
			error = EDEADLK;
			goto out;
		}
	}
#endif /* __DARWIN_UNIX03 */
	
	if (lcntval == PTHRW_RWL_INIT) {
		/* if we can acquire set L and S word */
		lcntval = PTHRW_RWL_INIT;
		newval  = PTHRW_RWL_INIT | PTHRW_INC | PTH_RWL_KBIT| PTH_RWL_EBIT;
		newsval = rw_seq + PTHRW_INC;

		oldval64 = (((uint64_t)rw_seq) << 32);
		oldval64 |= lcntval;
	
		newval64 = (((uint64_t)newsval) << 32);
		newval64 |= newval;
	
		if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) == TRUE) {
			goto gotit;
		} else
			goto loop;
	}
	
	newval = (lcntval + PTHRW_INC)| PTH_RWL_YBIT;	

	newsval = rw_seq;
	if (is_rws_setseq(rw_seq)) {
		newsval &= PTHRW_SW_Reset_BIT_MASK;
		newsval |= (newval & PTHRW_COUNT_MASK);
	}
	
	oldval64 = (((uint64_t)rw_seq) << 32);
	oldval64 |= lcntval;
	
	newval64 = (((uint64_t)(newsval)) << 32);
	newval64 |= newval;
	
	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) != TRUE)
		goto loop;

	PLOCKSTAT_RW_BLOCK(orwlock, WRITE_LOCK_PLOCKSTAT);
retry:
	updateval = __psynch_rw_yieldwrlock(orwlock, newval, ucntval, newsval, rwlock->rw_flags);
	if (updateval == (uint32_t)-1) {
		error = errno;
	} else
		error = 0;
	
	if (error == EINTR)
		goto retry;
	
	
	PLOCKSTAT_RW_BLOCKED(orwlock, WRITE_LOCK_PLOCKSTAT, BLOCK_SUCCESS_PLOCKSTAT);
	if (error != 0) {
#if _KSYN_TRACE_
		set_enable(2);
#endif /* _KSYN_TRACE_ */
		(void)pthread_threadid_np(pthread_self(), &myid);
		LIBC_ABORT("yieldwrlock from kernel with unknown error %x: tid %x\n", updateval, (uint32_t)myid);
	}
	
	
out:	
	if (error == 0) {
gotit:
		rwlock_action_onreturn(orwlock, updateval);
		if ( is_rwl_ebit_clear(updateval)) {
#if _KSYN_TRACE_
			set_enable(2);
#endif /* _KSYN_TRACE_ */
			(void)pthread_threadid_np(pthread_self(), &myid);
			LIBC_ABORT("yieldwrlock from kernel without EBit  %x: tid %x\n", updateval, (uint32_t)myid);
		}
#if __DARWIN_UNIX03
		rwlock->rw_owner = self;
#endif /* __DARWIN_UNIX03 */
		PLOCKSTAT_RW_ACQUIRE(orwlock, WRITE_LOCK_PLOCKSTAT);
		return(0);
	} else {
		PLOCKSTAT_RW_ERROR(orwlock, WRITE_LOCK_PLOCKSTAT, error);    
		return(error);
	}
}
int
pthread_rwlock_downgrade_np(pthread_rwlock_t * orwlock)
{
	uint32_t lcntval, ucntval, rw_seq, newval, newsval, updateval;
	int error = 0, haswbit = 0, hasubit = 0, hasybit = 0;
#if __DARWIN_UNIX03
	pthread_t self = pthread_self();
#endif /* __DARWIN_UNIX03 */
	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;
	uint64_t oldval64, newval64;
	volatile uint32_t * lcntaddr, *ucntaddr, *seqaddr;
	uint64_t myid = 0;
	
	if (rwlock->sig != _PTHREAD_RWLOCK_SIG) {
		LOCK(rwlock->lock);
		if (rwlock->sig == _PTHREAD_RWLOCK_SIG_init) {
		 	if ((error = __pthread_rwlock_init(orwlock, NULL)) != 0)  {
				UNLOCK(rwlock->lock);
				PLOCKSTAT_RW_ERROR(orwlock, READ_LOCK_PLOCKSTAT, error);
				return(error);
			}
		} else if (rwlock->sig != _PTHREAD_RWLOCK_SIG){
			UNLOCK(rwlock->lock);
			PLOCKSTAT_RW_ERROR(orwlock, READ_LOCK_PLOCKSTAT, EINVAL);
			return(EINVAL);
		}
		UNLOCK(rwlock->lock);
	}
	if (rwlock->pshared == PTHREAD_PROCESS_SHARED) {
		RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr);
	} else {
		lcntaddr = rwlock->rw_lcntaddr;
		ucntaddr = rwlock->rw_ucntaddr;
		seqaddr = rwlock->rw_seqaddr;
	}

loop:
	lcntval = *lcntaddr;
	ucntval = *ucntaddr;
	rw_seq = *seqaddr;

	
	/* if not holding exclusive lock, return */
	if ((is_rwl_ebit_set(lcntval )== 0) || (rwlock->rw_owner != self)) {
		return(EINVAL);
	}
	
	/* no other waiters and be granted in user space? ? */
	if ((lcntval & PTHRW_COUNT_MASK) == (ucntval + PTHRW_INC)) {
#if 0
		/* should have no write waiters pending */
		if (is_rwl_wbit_set(lcntval) != 0) {
#if _KSYN_TRACE_
			set_enable(2);
#endif /* _KSYN_TRACE_ */
			(void)pthread_threadid_np(pthread_self(), &myid);
			LIBC_ABORT("downgrade in user mode but W bit set  %x: tid %x\n", lcntval, (uint32_t)myid);
		}
#endif
		/* preserve count and remove ke bits */
		newval = lcntval & ~(PTH_RWL_EBIT | PTH_RWL_KBIT);
		/* if we can acquire set L and S word */
		newsval = rw_seq;

		oldval64 = (((uint64_t)rw_seq) << 32);
		oldval64 |= lcntval;
	
		newval64 = (((uint64_t)newsval) << 32);
		newval64 |= newval;
	
		if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) == TRUE) {
#if __DARWIN_UNIX03
			rwlock->rw_owner = (pthread_t)0;
#endif /* __DARWIN_UNIX03 */
			return(0);
		} else
			goto loop;
	} else {
		
		haswbit = lcntval & PTH_RWL_WBIT;
		hasubit = lcntval & PTH_RWL_UBIT;
		hasybit = lcntval & PTH_RWL_YBIT;

		/* reset all bits and set k */
		newval = (lcntval & PTHRW_COUNT_MASK) | PTH_RWL_KBIT;
		/* set I bit on S word */	
		newsval = rw_seq | PTH_RWS_IBIT;
		if (haswbit != 0)
			newsval |= PTH_RWS_WSVBIT;
		if (hasubit != 0)
			newsval |= PTH_RWS_USVBIT;
		if (hasybit != 0)
			newsval |= PTH_RWS_YSVBIT;

		oldval64 = (((uint64_t)rw_seq) << 32);
		oldval64 |= lcntval;
	
		newval64 = (((uint64_t)newsval) << 32);
		newval64 |= newval;
	
		if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) != TRUE)
			goto loop;

#if __DARWIN_UNIX03
		rwlock->rw_owner = 0;
#endif /* __DARWIN_UNIX03 */

retry:
		updateval = __psynch_rw_downgrade(orwlock, newval, ucntval, newsval, rwlock->rw_flags);
		if (updateval == (uint32_t)-1) {
			error = errno;
		} else
			error = 0;
	
		/* TBD: what to do with the error, EINTR ?? */
		if (error == EINTR)
			goto retry;

		if (error == 0)  {
			rwlock_action_onreturn(orwlock, updateval);
			return(0);
		} else {
#if _KSYN_TRACE_
			set_enable(1);
#endif /* _KSYN_TRACE_ */
			(void)pthread_threadid_np(pthread_self(), &myid);
			LIBC_ABORT("downgrade from kernel with unknown error %x with tid %x\n", updateval, (uint32_t)myid);
		}
		/* Not reached */	
	}
	return(EINVAL);	
}

int
pthread_rwlock_upgrade_np(pthread_rwlock_t * orwlock)
{
	return(pthread_rwlock_upgrade_internal(orwlock, 0));
}

int
pthread_rwlock_tryupgrade_np(pthread_rwlock_t *orwlock)
{
	return(pthread_rwlock_upgrade_internal(orwlock, 1));
}

static int
pthread_rwlock_upgrade_internal(pthread_rwlock_t * orwlock, int trylock)
{
	uint32_t lcntval, ucntval, rw_seq, newval, newsval, updateval;
	int error = 0, flags ;
#if __DARWIN_UNIX03
	pthread_t self = pthread_self();
#endif /* __DARWIN_UNIX03 */
	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;
	uint64_t oldval64, newval64;
	volatile uint32_t * lcntaddr, *ucntaddr, *seqaddr;
	uint64_t myid = 0;
	
	if (rwlock->sig != _PTHREAD_RWLOCK_SIG) {
		/* check for static initialization */
		LOCK(rwlock->lock);
		if (rwlock->sig == _PTHREAD_RWLOCK_SIG_init) {
		 	if ((error = __pthread_rwlock_init(orwlock, NULL)) != 0)  {
				UNLOCK(rwlock->lock);
				return(error);
			}
		} else if (rwlock->sig != _PTHREAD_RWLOCK_SIG){
			UNLOCK(rwlock->lock);
			return(EINVAL);
		}
		UNLOCK(rwlock->lock);
	}
	if (rwlock->pshared == PTHREAD_PROCESS_SHARED) {
		RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr);
	} else {
		lcntaddr = rwlock->rw_lcntaddr;
		ucntaddr = rwlock->rw_ucntaddr;
		seqaddr = rwlock->rw_seqaddr;
	}

loop:
	lcntval = *lcntaddr;
	ucntval = *ucntaddr;
	rw_seq = *seqaddr;
	
	if (is_rwl_eubit_set(lcntval) !=0) {
		return(EBUSY);
	}
	
	/* set U and K bit and go to kernel */
	newval  = (lcntval  | (PTH_RWL_UBIT | PTH_RWL_KBIT));
	newsval = rw_seq;
#if 0
	if (is_rws_setseq(rw_seq)) {
		newsval &= PTHRW_SW_Reset_BIT_MASK;
		newsval |= (newval & PTHRW_COUNT_MASK);
	}
#endif

	/* update lock seq and  block in kernel */
	
	oldval64 = (((uint64_t)rw_seq) << 32);
	oldval64 |= lcntval;
	
	newval64 = (((uint64_t)(newsval)) << 32);
	newval64 |= newval;
	
	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) != TRUE)
		goto loop;
	flags = rwlock->rw_flags;
	if (trylock != 0) {
		flags |= _PTHREAD_RWLOCK_UPGRADE_TRY;
	}
retry:
	updateval = __psynch_rw_upgrade(orwlock, newval, ucntval, newsval, rwlock->rw_flags);
	if (updateval == (uint32_t)-1) {
		error = errno;
	} else
		error = 0;
		
	if (error == EINTR)
		goto retry;
		
		
	if (error == 0) {
		rwlock_action_onreturn(orwlock, updateval);
		if ( is_rwl_ebit_clear(updateval)) {
#if _KSYN_TRACE_
			set_enable(2);
#endif /* _KSYN_TRACE_ */
			(void)pthread_threadid_np(pthread_self(), &myid);
			LIBC_ABORT("upgrade from kernel without EBit  %x: tid %x\n", updateval, (uint32_t)myid);
		}
#if __DARWIN_UNIX03
		rwlock->rw_owner = self;
#endif /* __DARWIN_UNIX03 */
		return(0);
	} else {
		if (trylock != 0) {
			return (EBUSY);
		}
	}

	return(error);	
}

/* Returns true if the rwlock is held for reading by any thread or held for writing by the current thread */
int 
pthread_rwlock_held_np(pthread_rwlock_t * orwlock)
{
	uint32_t lcntval, ucntval, rw_seq;
	int error = 0;
	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;
	volatile uint32_t * lcntaddr, *ucntaddr, *seqaddr;

	if (rwlock->sig != _PTHREAD_RWLOCK_SIG) {
		LOCK(rwlock->lock);
		if (rwlock->sig == _PTHREAD_RWLOCK_SIG_init) {
		 	if ((error = __pthread_rwlock_init(orwlock, NULL)) != 0)  {
				UNLOCK(rwlock->lock);
				return(0);
			}
		} else if (rwlock->sig != _PTHREAD_RWLOCK_SIG){
			UNLOCK(rwlock->lock);
			return(-1);
		}
		UNLOCK(rwlock->lock);
	}
	
	if (rwlock->pshared == PTHREAD_PROCESS_SHARED) {
		RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr);
	} else {
		lcntaddr = rwlock->rw_lcntaddr;
		ucntaddr = rwlock->rw_ucntaddr;
		seqaddr = rwlock->rw_seqaddr;
	}

	lcntval = *lcntaddr;
	ucntval = *ucntaddr;
	rw_seq = *seqaddr;
	
	if ((lcntval & PTHRW_COUNT_MASK) == (ucntval & PTHRW_COUNT_MASK))
		return(0);
	
	return(1);
}

/* Returns true if the rwlock is held for reading by any thread */
int 
pthread_rwlock_rdheld_np(pthread_rwlock_t * orwlock)
{
	uint32_t lcntval, ucntval, rw_seq;
	int error = 0;
	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;
	volatile uint32_t * lcntaddr, *ucntaddr, *seqaddr;
	
	if (rwlock->sig != _PTHREAD_RWLOCK_SIG) {
		LOCK(rwlock->lock);
		if (rwlock->sig == _PTHREAD_RWLOCK_SIG_init) {
		 	if ((error = __pthread_rwlock_init(orwlock, NULL)) != 0)  {
				UNLOCK(rwlock->lock);
				return(0);
			}
		} else if (rwlock->sig != _PTHREAD_RWLOCK_SIG){
			UNLOCK(rwlock->lock);
			return(-1);
		}
		UNLOCK(rwlock->lock);
	}
	

	if (rwlock->pshared == PTHREAD_PROCESS_SHARED) {
		RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr);
	} else {
		lcntaddr = rwlock->rw_lcntaddr;
		ucntaddr = rwlock->rw_ucntaddr;
		seqaddr = rwlock->rw_seqaddr;
	}

	lcntval = *lcntaddr;
	ucntval = *ucntaddr;
	rw_seq = *seqaddr;

	if ((lcntval & PTHRW_COUNT_MASK) == (ucntval & PTHRW_COUNT_MASK))
		return(0);

	if (is_rwl_ebit_set(lcntval) !=0) {
		return(0);
	}
	return(1);
}

/* Returns true if the rwlock is held for writing by the current thread */
int 
pthread_rwlock_wrheld_np(pthread_rwlock_t * orwlock)
{
	uint32_t lcntval, ucntval, rw_seq;
	pthread_t self = pthread_self();
	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;
	volatile uint32_t * lcntaddr, *ucntaddr, *seqaddr;
	int error = 0;
	
	if (rwlock->sig != _PTHREAD_RWLOCK_SIG) {
		LOCK(rwlock->lock);
		if (rwlock->sig == _PTHREAD_RWLOCK_SIG_init) {
		 	if ((error = __pthread_rwlock_init(orwlock, NULL)) != 0)  {
				UNLOCK(rwlock->lock);
				return(0);
			}
		} else if (rwlock->sig != _PTHREAD_RWLOCK_SIG){
			UNLOCK(rwlock->lock);
			return(-1);
		}
		UNLOCK(rwlock->lock);
	}
	
	if (rwlock->pshared == PTHREAD_PROCESS_SHARED) {
		RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr);
	} else {
		lcntaddr = rwlock->rw_lcntaddr;
		ucntaddr = rwlock->rw_ucntaddr;
		seqaddr = rwlock->rw_seqaddr;
	}

	lcntval = *lcntaddr;
	ucntval = *ucntaddr;
	rw_seq = *seqaddr;

	if ((is_rwl_ebit_set(lcntval)) && (rwlock->rw_owner == self)) {
		return(1);
	}
	return(0);
}
/******************************************************/
#endif /* NOTYET */


#endif /* !BUILDING_VARIANT ] */

int
pthread_rwlock_destroy(pthread_rwlock_t *orwlock)
{
	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;
#if __DARWIN_UNIX03
	uint32_t rw_lcnt, rw_ucnt;
	volatile uint32_t * lcntaddr, *ucntaddr, *seqaddr;
#endif /* __DARWIN_UNIX03 */
	
	if (rwlock->sig != _PTHREAD_RWLOCK_SIG && rwlock->sig != _PTHREAD_RWLOCK_SIG_init)
		return(EINVAL);
	if (rwlock->sig == _PTHREAD_RWLOCK_SIG) {
#if __DARWIN_UNIX03
		if (rwlock->pshared == PTHREAD_PROCESS_SHARED) {
			RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr);
		} else {
			lcntaddr = rwlock->rw_lcntaddr;
			ucntaddr = rwlock->rw_ucntaddr;
			seqaddr = rwlock->rw_seqaddr;
		}

		rw_lcnt = *lcntaddr;
		rw_ucnt = *ucntaddr;
		
		if((rw_lcnt & PTHRW_COUNT_MASK) != rw_ucnt)
			return(EBUSY);
		
#endif /* __DARWIN_UNIX03 */
		//bzero(rwlock, sizeof(npthread_rwlock_t));
		rwlock->sig = _PTHREAD_NO_SIG;
		return(0);
	} else if (rwlock->sig == _PTHREAD_RWLOCK_SIG_init) {
		rwlock->sig = _PTHREAD_NO_SIG;
                return(0);
	} else 
		return(EINVAL);
}


int
pthread_rwlock_init(pthread_rwlock_t * orwlock, const pthread_rwlockattr_t *attr)
{
	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;
#if __DARWIN_UNIX03
	uint32_t rw_lcnt, rw_ucnt;
	volatile uint32_t * lcntaddr, *ucntaddr, *seqaddr;
#endif /* __DARWIN_UNIX03 */
	
#if __DARWIN_UNIX03
	if (attr && (attr->sig != _PTHREAD_RWLOCK_ATTR_SIG)) {
		return(EINVAL);
	}
	
	if (rwlock->sig == _PTHREAD_RWLOCK_SIG) {
		if (rwlock->pshared == PTHREAD_PROCESS_SHARED) {
			RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr);
		} else {
			lcntaddr = rwlock->rw_lcntaddr;
			ucntaddr = rwlock->rw_ucntaddr;
			seqaddr = rwlock->rw_seqaddr;
		}
		
		rw_lcnt = *lcntaddr;
		rw_ucnt = *ucntaddr;
		
		if ((rw_lcnt & PTHRW_COUNT_MASK) != rw_ucnt)
			return(EBUSY);
		
	}
#endif
	LOCK_INIT(rwlock->lock);
	return(__pthread_rwlock_init(orwlock, attr));
	
}

int
pthread_rwlock_rdlock(pthread_rwlock_t * orwlock)
{
#if __DARWIN_UNIX03
	pthread_t self;
#endif /* __DARWIN_UNIX03 */
	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;
	uint32_t lcntval, ucntval, rw_seq, newval, newsval, updateval;
	int error = 0, retry_count = 0;
	uint64_t oldval64, newval64;
	volatile uint32_t * lcntaddr, *ucntaddr, *seqaddr;
	uint64_t myid = 0;
	
	if (rwlock->sig != _PTHREAD_RWLOCK_SIG) {
		LOCK(rwlock->lock);
		if (rwlock->sig == _PTHREAD_RWLOCK_SIG_init) {
		 	if ((error = __pthread_rwlock_init(orwlock, NULL)) != 0)  {
				UNLOCK(rwlock->lock);
				PLOCKSTAT_RW_ERROR(orwlock, READ_LOCK_PLOCKSTAT, error);
				return(error);
			}
		} else if (rwlock->sig != _PTHREAD_RWLOCK_SIG){
			UNLOCK(rwlock->lock);
			PLOCKSTAT_RW_ERROR(orwlock, READ_LOCK_PLOCKSTAT, EINVAL);
			return(EINVAL);
		}
		UNLOCK(rwlock->lock);
	}
	
	if (rwlock->pshared == PTHREAD_PROCESS_SHARED) {
		RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr);
	} else {
		lcntaddr = rwlock->rw_lcntaddr;
		ucntaddr = rwlock->rw_ucntaddr;
		seqaddr = rwlock->rw_seqaddr;
	}

#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_RDLOCK | DBG_FUNC_START, (uint32_t)rwlock, 0, 0, 0, 0);
#endif
loop:
	lcntval = *lcntaddr;
	ucntval = *ucntaddr;
	rw_seq = *seqaddr;
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_RDLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, lcntval, (ucntval | 0xee), rw_seq, 0);
#endif
	
	/* if l bit is on or u and k bit is clear, acquire lock in userland */
	if (can_rwl_readinuser(lcntval))
		goto gotlock;
	
#if __DARWIN_UNIX03
	if (is_rwl_ebit_set(lcntval)) {
		self = pthread_self();
		if(rwlock->rw_owner == self) {
			error = EDEADLK;
			goto out;
		}
	}
#endif /* __DARWIN_UNIX03 */
	

	/* Need to block in kernel , remove Rbit */
	newval  = (lcntval + PTHRW_INC) & PTH_RWLOCK_RESET_RBIT;
	
	newsval = rw_seq;
	if (is_rws_setseq(rw_seq)) {
		newsval &= PTHRW_SW_Reset_BIT_MASK;
		newsval |= (newval & PTHRW_COUNT_MASK);
	}

	oldval64 = (((uint64_t)rw_seq) << 32);
	oldval64 |= lcntval;
	
	newval64 = (((uint64_t)newsval) << 32);
	newval64 |= newval;
	
	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) != TRUE)
		goto loop;

	/* give writers priority over readers */
	PLOCKSTAT_RW_BLOCK(orwlock, READ_LOCK_PLOCKSTAT);

#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_RDLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, lcntval, newval, newsval, 0);
#endif

retry:
	updateval = __psynch_rw_rdlock(orwlock, newval, ucntval, newsval, rwlock->rw_flags);
	
	if (updateval == (uint32_t)-1) {
		error = errno;
	} else
		error = 0;
	
	if (error == EINTR)
		goto retry;

	if (error == 0)  {
		rwlock_action_onreturn(orwlock, updateval);
		PLOCKSTAT_RW_BLOCKED(orwlock, READ_LOCK_PLOCKSTAT, BLOCK_SUCCESS_PLOCKSTAT);
		PLOCKSTAT_RW_ACQUIRE(orwlock, READ_LOCK_PLOCKSTAT);    
		return(0);
	} else {
		PLOCKSTAT_RW_BLOCKED(orwlock, READ_LOCK_PLOCKSTAT, BLOCK_FAIL_PLOCKSTAT);
#if _KSYN_TRACE_
		set_enable(1);
#endif /* _KSYN_TRACE_ */
		(void)pthread_threadid_np(pthread_self(), &myid);
		LIBC_ABORT("rdlock from kernel with unknown error %x with tid %x\n", updateval, (uint32_t)myid);
		goto out;
	}
	/* Not reached */	
	
gotlock:
	if (rw_diffgenseq(lcntval, ucntval) >= PTHRW_MAX_READERS) {
		/* since ucntval may be newer, just redo */
		retry_count++;
		if (retry_count > 1024) {
		
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_TOOMANY | DBG_FUNC_NONE, (uint32_t)rwlock, 0XEEEEEEEE, lcntval, ucntval, 0);
#endif
			error = EAGAIN;
			goto out;
		} else {
			sched_yield();
			goto loop;
		}
	}
	
	/* Need to update L (remove R bit) and S word */
	newval  = (lcntval + PTHRW_INC)  & PTH_RWLOCK_RESET_RBIT;
	newsval = (rw_seq + PTHRW_INC);

	oldval64 = (((uint64_t)rw_seq) << 32);
	oldval64 |= lcntval;
	newval64 = (((uint64_t)newsval) << 32);
	newval64 |= newval;
	
	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) != TRUE)
		goto loop;

#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_RDLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, 0x55555555, lcntval, newval, 0);
#endif
	
	PLOCKSTAT_RW_ACQUIRE(orwlock, READ_LOCK_PLOCKSTAT);
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_RDLOCK | DBG_FUNC_END, (uint32_t)rwlock, 0xAAAAAAAA, 0, 0, 0);
#endif
	return(0);
out:
	PLOCKSTAT_RW_ERROR(orwlock, READ_LOCK_PLOCKSTAT, error);    
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_RDLOCK | DBG_FUNC_END, (uint32_t)rwlock, 0xAAAAAAAA, error, 0, 0);
#endif
	return(error);
}

int
pthread_rwlock_tryrdlock(pthread_rwlock_t * orwlock)
{
	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;
	uint32_t lcntval, ucntval, rw_seq, newval, newsval;
	int error = 0, retry_count = 0;
	uint64_t oldval64, newval64;
	volatile uint32_t * lcntaddr, *ucntaddr, *seqaddr;
	
	if (rwlock->sig != _PTHREAD_RWLOCK_SIG) {
		LOCK(rwlock->lock);
		if (rwlock->sig == _PTHREAD_RWLOCK_SIG_init) {
		 	if ((error = __pthread_rwlock_init(orwlock, NULL)) != 0)  {
				UNLOCK(rwlock->lock);
				PLOCKSTAT_RW_ERROR(orwlock, READ_LOCK_PLOCKSTAT, error);
				return(error);
			}
		} else if (rwlock->sig != _PTHREAD_RWLOCK_SIG){
			UNLOCK(rwlock->lock);
			PLOCKSTAT_RW_ERROR(orwlock, READ_LOCK_PLOCKSTAT, EINVAL);
			return(EINVAL);
		}
		UNLOCK(rwlock->lock);
	}
	
	if (rwlock->pshared == PTHREAD_PROCESS_SHARED) {
		RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr);
	} else {
		lcntaddr = rwlock->rw_lcntaddr;
		ucntaddr = rwlock->rw_ucntaddr;
		seqaddr = rwlock->rw_seqaddr;
	}

loop:
	lcntval = *lcntaddr;
	ucntval = *ucntaddr;
	rw_seq = *seqaddr;
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_TRYRDLOCK | DBG_FUNC_START, (uint32_t)rwlock, lcntval, ucntval, rw_seq, 0);
#endif

	/* if l bit is on or u and k bit is clear, acquire lock in userland */
	if (can_rwl_readinuser(lcntval))
		goto gotlock;
	
	error = EBUSY;
	goto out;
	
gotlock:
	if (rw_diffgenseq(lcntval, ucntval) >= PTHRW_MAX_READERS) {
		/* since ucntval may be newer, just redo */
		retry_count++;
		if (retry_count > 1024) {
		
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_TOOMANY | DBG_FUNC_NONE, (uint32_t)rwlock, 0XEEEEEEEE, lcntval, ucntval, 0);
#endif
			error = EAGAIN;
			goto out;
		} else {
			sched_yield();
			goto loop;
		}
	}
	
	/* Need to update L(remove Rbit ) and S word */
	newval  = (lcntval + PTHRW_INC) & PTH_RWLOCK_RESET_RBIT;
	newsval = (rw_seq + PTHRW_INC);

	oldval64 = (((uint64_t)rw_seq) << 32);
	oldval64 |= lcntval;
	newval64 = (((uint64_t)newsval) << 32);
	newval64 |= newval;
	
	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) != TRUE)
		goto loop;

#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_TRYRDLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, 0x55555555, lcntval, newval, 0);
#endif
	
	PLOCKSTAT_RW_ACQUIRE(orwlock, READ_LOCK_PLOCKSTAT);
	return(0);

out:
	PLOCKSTAT_RW_ERROR(orwlock, READ_LOCK_PLOCKSTAT, error);    
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_TRYRDLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, 0x55555555, error, 0, 0);
#endif
	return(error);
}

int
pthread_rwlock_trywrlock(pthread_rwlock_t * orwlock)
{
#if __DARWIN_UNIX03
	pthread_t self = pthread_self();
#endif /* __DARWIN_UNIX03 */
	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;
	uint32_t lcntval, rw_seq, newval, newsval;
	int error = 0, gotlock = 0;
	uint64_t oldval64, newval64;
	volatile uint32_t * lcntaddr, *ucntaddr, *seqaddr;
	
	if (rwlock->sig != _PTHREAD_RWLOCK_SIG) {
		LOCK(rwlock->lock);
		if (rwlock->sig == _PTHREAD_RWLOCK_SIG_init) {
		 	if ((error = __pthread_rwlock_init(orwlock, NULL)) != 0)  {
				UNLOCK(rwlock->lock);
				PLOCKSTAT_RW_ERROR(orwlock, WRITE_LOCK_PLOCKSTAT, error);
				return(error);
			}
		} else if (rwlock->sig != _PTHREAD_RWLOCK_SIG){
			UNLOCK(rwlock->lock);
			PLOCKSTAT_RW_ERROR(orwlock, WRITE_LOCK_PLOCKSTAT, EINVAL);
			return(EINVAL);
		}
		UNLOCK(rwlock->lock);
	}
	
	if (rwlock->pshared == PTHREAD_PROCESS_SHARED) {
		RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr);
	} else {
		lcntaddr = rwlock->rw_lcntaddr;
		ucntaddr = rwlock->rw_ucntaddr;
		seqaddr = rwlock->rw_seqaddr;
	}

#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_TRYWRLOCK | DBG_FUNC_START, (uint32_t)rwlock, 0, 0, 0, 0);
#endif
loop:
	lcntval = *lcntaddr;
	rw_seq = *seqaddr;

	/* can we acquire in userland? */
	if ((lcntval & PTH_RWL_RBIT) != 0) {
		newval  = ((lcntval + PTHRW_INC) & PTHRW_COUNT_MASK) | PTH_RWL_IBIT | PTH_RWL_KBIT| PTH_RWL_EBIT;
		newsval = rw_seq + PTHRW_INC;
		gotlock = 1;
	} else
		gotlock = 0;


	oldval64 = (((uint64_t)rw_seq) << 32);
	oldval64 |= lcntval;
	
	if (gotlock != 0) {
		newval64 = (((uint64_t)newsval) << 32);
		newval64 |= newval;
	} else 
		newval64 = oldval64;
	
	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) != TRUE) {
		goto loop;
	}
	if (gotlock == 1) {
#if __DARWIN_UNIX03
		rwlock->rw_owner = self;
#endif /* __DARWIN_UNIX03 */
		PLOCKSTAT_RW_ACQUIRE(orwlock, WRITE_LOCK_PLOCKSTAT);
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_TRYWRLOCK | DBG_FUNC_END, (uint32_t)rwlock, 0, 0, 0, 0);
#endif
		return(0);
	} else {
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_TRYWRLOCK | DBG_FUNC_END, (uint32_t)rwlock, 0xAAAAAAAA, EBUSY, 0, 0);
#endif

		PLOCKSTAT_RW_ERROR(orwlock, WRITE_LOCK_PLOCKSTAT, EBUSY);    
		return(EBUSY);
	}
}

int
pthread_rwlock_wrlock(pthread_rwlock_t * orwlock)
{
#if __DARWIN_UNIX03
	pthread_t self = pthread_self();
#endif /* __DARWIN_UNIX03 */
	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;
	uint32_t lcntval, ucntval, rw_seq, newval, newsval, updateval;
	int error = 0, gotlock = 0;
	uint64_t oldval64, newval64;
	volatile uint32_t * lcntaddr, *ucntaddr, *seqaddr;
	uint64_t myid = 0;
	
	if (rwlock->sig != _PTHREAD_RWLOCK_SIG) {
		LOCK(rwlock->lock);
		if (rwlock->sig == _PTHREAD_RWLOCK_SIG_init) {
		 	if ((error = __pthread_rwlock_init(orwlock, NULL)) != 0)  {
				UNLOCK(rwlock->lock);
				PLOCKSTAT_RW_ERROR(orwlock, WRITE_LOCK_PLOCKSTAT, error);
				return(error);
			}
		} else if (rwlock->sig != _PTHREAD_RWLOCK_SIG){
			UNLOCK(rwlock->lock);
			PLOCKSTAT_RW_ERROR(orwlock, WRITE_LOCK_PLOCKSTAT, EINVAL);
			return(EINVAL);
		}
		UNLOCK(rwlock->lock);
	}
	
	if (rwlock->pshared == PTHREAD_PROCESS_SHARED) {
		RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr);
	} else {
		lcntaddr = rwlock->rw_lcntaddr;
		ucntaddr = rwlock->rw_ucntaddr;
		seqaddr = rwlock->rw_seqaddr;
	}
	
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_WRLOCK | DBG_FUNC_START, (uint32_t)rwlock, 0, 0, 0, 0);
#endif
loop:
	lcntval = *lcntaddr;
	ucntval = *ucntaddr;
	rw_seq = *seqaddr;
	
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_WRLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, lcntval, ucntval, rw_seq, 0);
#endif

#if __DARWIN_UNIX03
	if (is_rwl_ebit_set(lcntval)) {
		if(rwlock->rw_owner == self) {
			error = EDEADLK;
			goto out;
		}
	}
#endif /* __DARWIN_UNIX03 */

	
	if ((lcntval & PTH_RWL_RBIT) != 0) {
		/* lock is restart state, writer can acquire the lock */
		newval  = ((lcntval + PTHRW_INC) & PTHRW_COUNT_MASK) | PTH_RWL_IBIT | PTH_RWL_KBIT| PTH_RWL_EBIT;

		newsval = rw_seq + PTHRW_INC;
		gotlock = 1;

	} else {
		if (is_rwl_lbit_set(lcntval))
			newval = (lcntval + PTHRW_INC)| PTH_RWL_WBIT;	
		else
			newval = (lcntval + PTHRW_INC) | PTH_RWL_KBIT| PTH_RWL_WBIT;	

		newsval = rw_seq;
		if (is_rws_setseq(rw_seq)) {
			newsval &= PTHRW_SW_Reset_BIT_MASK;
			newsval |= (newval & PTHRW_COUNT_MASK);
		}
		gotlock = 0;
	}

	/* update lock seq */
	oldval64 = (((uint64_t)rw_seq) << 32);
	oldval64 |= lcntval;
	
	newval64 = (((uint64_t)newsval) << 32);
	newval64 |= newval;
	
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_WRLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, 0x55555555, lcntval, newval, 0);
#endif
	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) != TRUE)
		goto loop;
		
	/* lock acquired in userland itself? */
	if (gotlock != 0)
		goto gotit;

	/* unable to acquire in userland, transition to kernel */

	PLOCKSTAT_RW_BLOCK(orwlock, WRITE_LOCK_PLOCKSTAT);
retry:
	updateval = __psynch_rw_wrlock(orwlock, newval, ucntval, newsval, rwlock->rw_flags);
	if (updateval == (uint32_t)-1) {
		error = errno;
	} else
		error = 0;
	
	if (error == EINTR) {
		goto retry;
	} 

	if (error != 0) {
#if _KSYN_TRACE_
		set_enable(2);
#endif /* _KSYN_TRACE_ */
		(void)pthread_threadid_np(pthread_self(), &myid);
		LIBC_ABORT("wrlock from kernel with unknown error %x: tid %x\n", updateval, (uint32_t)myid);
	}
	
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_WRLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, 0x33333333, newval, updateval, 0);
#endif
	PLOCKSTAT_RW_BLOCKED(orwlock, WRITE_LOCK_PLOCKSTAT, BLOCK_SUCCESS_PLOCKSTAT);
	if (error == 0) {
		rwlock_action_onreturn(orwlock, updateval);
gotit:
#if __DARWIN_UNIX03
		rwlock->rw_owner = self;
#endif /* __DARWIN_UNIX03 */
		PLOCKSTAT_RW_ACQUIRE(orwlock, WRITE_LOCK_PLOCKSTAT);
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_WRLOCK | DBG_FUNC_END, (uint32_t)rwlock, 0xAAAAAAAA, error, 0, 0);
#endif
		return(0);
	} 
#if __DARWIN_UNIX03
out:
#endif /* __DARWIN_UNIX03 */
	PLOCKSTAT_RW_ERROR(orwlock, WRITE_LOCK_PLOCKSTAT, error);    
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_WRLOCK | DBG_FUNC_END, (uint32_t)rwlock, 0xAAAAAAAA, error, 0, 0);
#endif
	return(error);
}


int
pthread_rwlock_unlock(pthread_rwlock_t * orwlock)
{
	npthread_rwlock_t * rwlock = (npthread_rwlock_t *)orwlock;
	uint32_t lcntval, ucntval, rw_seq, newval, newsval, updateval, ulval;
	int error = 0, wrlock = 0, haswbit = 0, hasubit = 0, hasybit = 0;
	uint64_t oldval64, newval64;
	volatile uint32_t * lcntaddr, *ucntaddr, *seqaddr;
	uint64_t myid = 0;
	
	if (rwlock->sig != _PTHREAD_RWLOCK_SIG) {
		LOCK(rwlock->lock);
		if (rwlock->sig == _PTHREAD_RWLOCK_SIG_init) {
		 	if ((error = __pthread_rwlock_init(orwlock, NULL)) != 0)  {
				UNLOCK(rwlock->lock);
				PLOCKSTAT_RW_ERROR(orwlock, wrlock, error);
				return(error);
			}
		} else if (rwlock->sig != _PTHREAD_RWLOCK_SIG){
			UNLOCK(rwlock->lock);
			PLOCKSTAT_RW_ERROR(orwlock, wrlock, EINVAL);
			return(EINVAL);
		}
		UNLOCK(rwlock->lock);
	}
	
	if (rwlock->pshared == PTHREAD_PROCESS_SHARED) {
		RWLOCK_GETSEQ_ADDR(rwlock, lcntaddr, ucntaddr, seqaddr);
	} else {
		lcntaddr = rwlock->rw_lcntaddr;
		ucntaddr = rwlock->rw_ucntaddr;
		seqaddr = rwlock->rw_seqaddr;
	}
	
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNLOCK | DBG_FUNC_START, (uint32_t)rwlock, 0, 0, 0, 0);
#endif
loop:
	lcntval = *lcntaddr;
	ucntval = *ucntaddr;
	rw_seq = *seqaddr;
	

	
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, 0x51515151, lcntval, ucntval, 0);
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, 0x51515151, rw_seq, 0, 0);
#endif
	/* check for spurious unlocks */
	if ((lcntval & PTH_RWL_RBIT) != 0) {
		newval  = lcntval ;
		newsval = rw_seq;

		oldval64 = (((uint64_t)rw_seq) << 32);
		oldval64 |= lcntval;
	
		newval64 = (((uint64_t)newsval) << 32);
		newval64 |= newval;
	
		if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) == TRUE) {
			/* spurious unlock, return */
			error = EINVAL;
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, 0x1a1b1c1d, lcntval, ucntval, 0);
#endif
			goto succout;
		} else
			goto loop;
	}

	if (is_rwl_ebit_set(lcntval)) {
		wrlock = 1;
#if __DARWIN_UNIX03
		rwlock->rw_owner = (pthread_t)0;
#endif /* __DARWIN_UNIX03 */
	}
	
	/* update U */
	
	ulval = (ucntval + PTHRW_INC);

	if (OSAtomicCompareAndSwap32Barrier(ucntval, ulval, (volatile int32_t *)ucntaddr) != TRUE)
		goto loop;
	
lp11:
	/* just validate the l and S values */
	oldval64 = (((uint64_t)rw_seq) << 32);
	oldval64 |= lcntval;

	newval64 = oldval64;

	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) != TRUE)  {
		lcntval = *lcntaddr;
		rw_seq = *seqaddr;
		goto lp11;
	}

#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, 0xd1d2d3d4, lcntval, rw_seq, 0);
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, 0xd1d2d3d4, ulval, 0, 0);
#endif

	/* last unlock, note U is already updated ? */
	if((lcntval & PTHRW_COUNT_MASK) == (ulval & PTHRW_COUNT_MASK)) {

#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, 0xbbbbbbbb, lcntval, ucntval, 0);
#endif
		/* Set L with R and init bits and set S to L */
		newval  = (lcntval & PTHRW_COUNT_MASK)| PTHRW_RWLOCK_INIT;
		newsval = (lcntval & PTHRW_COUNT_MASK)| PTHRW_RWS_INIT;

		oldval64 = (((uint64_t)rw_seq) << 32);
		oldval64 |= lcntval;

		newval64 = (((uint64_t)newsval) << 32);
		newval64 |= newval;

		if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) != TRUE)  {
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, 0xcccccccc, 0, 0, 0);
#endif
			lcntval = *lcntaddr;
			rw_seq = *seqaddr;
			goto lp11;
		}
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, 0xdddddddd, lcntval, ucntval, 0);
#endif
		goto succout;
	}

	/* if it is not exclusive or no Writer/yield pending, skip */
	if ((lcntval & (PTH_RWL_EBIT | PTH_RWL_WBIT | PTH_RWL_YBIT | PTH_RWL_KBIT)) == 0) {
		goto succout;
	}

	/* kernel transition needed? */
	/* U+1 == S? */
	if ((ulval + PTHRW_INC) != (rw_seq & PTHRW_COUNT_MASK)) {
		if ((lcntval & PTH_RWL_UBIT) != 0) {
			/* if U bit is set U + 2 == S ? */
			if ((ulval + PTHRW_INC + PTHRW_INC) != (rw_seq & PTHRW_COUNT_MASK))
				goto succout;
		} else 
			goto succout;
	}

	haswbit = lcntval & PTH_RWL_WBIT;
	hasubit = lcntval & PTH_RWL_UBIT;
	hasybit = lcntval & PTH_RWL_YBIT;

	/* reset all bits and set k */
	newval = (lcntval & PTHRW_COUNT_MASK) | PTH_RWL_KBIT;
	/* set I bit on S word */	
	newsval = rw_seq | PTH_RWS_IBIT;
	if (haswbit != 0)
		newsval |= PTH_RWS_WSVBIT;
	if (hasubit != 0)
		newsval |= PTH_RWS_USVBIT;
	if (hasybit != 0)
		newsval |= PTH_RWS_YSVBIT;

	oldval64 = (((uint64_t)rw_seq) << 32);
	oldval64 |= lcntval;
	
	newval64 = (((uint64_t)newsval) << 32);
	newval64 |= newval;
	
	if (OSAtomicCompareAndSwap64Barrier(oldval64, newval64, (volatile int64_t *)lcntaddr) != TRUE)
		goto lp11;

#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, 0x55555511, 1, ulval, 0);
#endif
	updateval = __psynch_rw_unlock(orwlock, lcntval, ulval, newsval, rwlock->rw_flags);
	if (updateval == (uint32_t)-1) {
		error = errno;
	} else
		error = 0;
	
	if(error != 0) {

		/* not sure what is the scenario */
		if(error != EINTR) {
#if _KSYN_TRACE_
		set_enable(4);
#endif /* _KSYN_TRACE_ */
			(void)pthread_threadid_np(pthread_self(), &myid);
			LIBC_ABORT("rwunlock from kernel with unknown error %x: tid %x\n", error, (uint32_t)myid);
			goto succout;
		}
		error = 0;
	}
	
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNLOCK | DBG_FUNC_NONE, (uint32_t)rwlock, 0x55555522, 3, lcntval, 0);
#endif

succout:
	PLOCKSTAT_RW_RELEASE(orwlock, wrlock);
#if _KSYN_TRACE_
	if (__pthread_lock_debug != 0)
	(void)__kdebug_trace(_KSYN_TRACE_RW_UNLOCK | DBG_FUNC_END, (uint32_t)rwlock, 0xAAAAAAAA, error, 0, 0);
#endif
	return(0);
}


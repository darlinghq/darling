/*-
 * Copyright (c) 2008-2009, Apple Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SECURITY_AUDIT_AUDIT_BSD_H
#define _SECURITY_AUDIT_AUDIT_BSD_H

#include <sys/cdefs.h>
#include <machine/endian.h>

#if defined(_KERNEL) || defined(KERNEL)

#if DIAGNOSTIC
#ifdef KASSERT
#undef KASSERT
#endif
#ifdef AUDIT_KASSERT_DEBUG
#define KASSERT(exp, msg) do {                                          \
	if (__builtin_expect(!(exp), 0)) {                              \
	        printf("%s:%d KASSERT failed: ", __FILE__, __LINE__);   \
	        printf msg;                                             \
	        printf("\n");                                           \
	}                                                               \
} while (0)
#else
#define KASSERT(exp, msg) do {                                          \
	if (__builtin_expect(!(exp), 0))                                \
	        panic msg;                                              \
} while (0)
#endif
#endif  /* DIAGNOSTIC */

#define AU_MAX_LCK_NAME 32

#if __DARWIN_BYTE_ORDER == __DARWIN_BIG_ENDIAN
#define be16enc(p, d)   *(p) = (d)
#define be32enc(p, d)   *(p) = (d)
#define be64enc(p, d)   *(p) = (d)

#else /* __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN */

#include <libkern/OSByteOrder.h>

#define be16enc(p, d)   OSWriteSwapInt16(p, 0, d)
#define be32enc(p, d)   OSWriteSwapInt32(p, 0, d)
#define be64enc(p, d)   OSWriteSwapInt64(p, 0, d)
#endif /* __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN */

/*
 * BSD kernel memory allocation.
 */
#define AUDIT_MALLOC_DEBUG       0      /* Change to 1 for malloc debugging. */

#define M_AUDITUNKNOWN           0
#define M_AUDITDATA              1
#define M_AUDITPATH              2
#define M_AUDITTEXT              3
#define M_AUDITBSM               4
#define M_AUDITEVCLASS           5
#define M_AUDIT_PIPE             6
#define M_AUDIT_PIPE_ENTRY       7
#define M_AUDIT_PIPE_PRESELECT   8
#define M_AU_SESSION             9
#define M_AU_EV_PLIST           10

#define NUM_MALLOC_TYPES        11

#ifdef  M_WAITOK
#undef  M_WAITOK
#define M_WAITOK        0x0000  /* ok to block */
#endif
#ifdef  M_NOWAIT
#undef  M_NOWAIT
#endif
#define M_NOWAIT        0x0001  /* do not block */
#ifdef  M_ZERO
#undef  M_ZERO
#endif
#define M_ZERO          0x0004  /* bzero the allocation */

#ifdef  M_MAGIC
#undef  M_MAGIC
#endif
#define M_MAGIC         877983977

#ifdef  MALLOC_DEFINE
#undef  MALLOC_DEFINE
#endif
#if AUDIT_MALLOC_DEBUG
struct  au_malloc_type {
	SInt64           mt_size;
	SInt64           mt_maxsize;
	SInt32           mt_inuse;
	SInt32           mt_maxused;
	unsigned         mt_type;
	unsigned         mt_magic;
	const char      *mt_shortdesc;
	const char      *mt_lastcaller;
};
typedef struct au_malloc_type   au_malloc_type_t;

#define MALLOC_DEFINE(type, shortdesc, longdesc)                \
	au_malloc_type_t  audit_##type[1] = {                   \
	        { 0, 0, 0, 0, (type < NUM_MALLOC_TYPES) ? type :\
	          M_AUDITUNKNOWN, M_MAGIC, shortdesc, NULL }    \
	}

extern au_malloc_type_t *audit_malloc_types[];

#else

struct  au_malloc_type {
	uint32_t         mt_magic;
	const char      *mt_shortdesc;
};
typedef struct au_malloc_type   au_malloc_type_t;

#define MALLOC_DEFINE(type, shortdesc, longdesc)                \
	au_malloc_type_t  audit_##type[1] = {                   \
	        {M_MAGIC, shortdesc }                           \
	}

#endif /* AUDIT_MALLOC_DEBUG */

#ifdef  MALLOC_DECLARE
#undef  MALLOC_DECLARE
#endif
#define MALLOC_DECLARE(type) \
	extern au_malloc_type_t audit_##type[]

#if AUDIT_MALLOC_DEBUG
#define malloc(sz, tp, fl)      _audit_malloc(sz, audit_##tp, fl, __FUNCTION__)
void *_audit_malloc(size_t size, au_malloc_type_t *type, int flags,
    const char *fn);
#else
#define malloc(sz, tp, fl)      _audit_malloc(sz, audit_##tp, fl)
void *_audit_malloc(size_t size, au_malloc_type_t *type, int flags);
#endif

#define free(ad, tp)            _audit_free(ad, audit_##tp)
void _audit_free(void *addr, au_malloc_type_t *type);

/*
 * BSD condition variable.
 */
struct cv {
	const char      *cv_description;
	int              cv_waiters;
};

/*
 * BSD mutex.
 */
struct mtx {
	lck_mtx_t       *mtx_lock;
#if DIAGNOSTIC
	char             mtx_name[AU_MAX_LCK_NAME];
#endif
};

/*
 * BSD rw lock.
 */
struct rwlock {
	lck_rw_t        *rw_lock;
#if DIAGNOSTIC
	char             rw_name[AU_MAX_LCK_NAME];
#endif
};

/*
 * Sleep lock.
 */
struct slck {
	lck_mtx_t       *sl_mtx;
	int              sl_locked;
	int              sl_waiting;
#if DIAGNOSTIC
	char             sl_name[AU_MAX_LCK_NAME];
#endif
};

/*
 * Recursive lock.
 */
struct rlck {
	lck_mtx_t       *rl_mtx;
	uint32_t         rl_recurse;
	thread_t         rl_thread;
#if DIAGNOSTIC
	char             rl_name[AU_MAX_LCK_NAME];
#endif
};

/*
 * BSD condition variables functions.
 */
void    _audit_cv_init(struct cv *cvp, const char *desc);
void    _audit_cv_destroy(struct cv *cvp);
void    _audit_cv_signal(struct cv *cvp);
void    _audit_cv_broadcast(struct cv *cvp);
void    _audit_cv_wait(struct cv *cvp, lck_mtx_t *mp, const char *desc);
int     _audit_cv_wait_sig(struct cv *cvp, lck_mtx_t *mp, const char *desc);
int     _audit_cv_wait_continuation(struct cv *cvp, lck_mtx_t *mp,
    thread_continue_t function);
#define cv_init(cvp, desc)        _audit_cv_init(cvp, desc)
#define cv_destroy(cvp)           _audit_cv_destroy(cvp)
#define cv_signal(cvp)            _audit_cv_signal(cvp)
#define cv_broadcast(cvp)         _audit_cv_broadcast(cvp)
#define cv_broadcastpri(cvp, pri) _audit_cv_broadcast(cvp)
#define cv_wait(cvp, mp)          _audit_cv_wait(cvp, (mp)->mtx_lock, #cvp)
#define cv_wait_sig(cvp, mp)      _audit_cv_wait_sig(cvp, (mp)->mtx_lock, #cvp)
#define cv_wait_continuation(cvp, mp, f) \
    _audit_cv_wait_continuation(cvp, (mp)->mtx_lock, f)

/*
 * BSD Mutexes.
 */
void    _audit_mtx_init(struct mtx *mp, const char *name);
void    _audit_mtx_destroy(struct mtx *mp);
#define mtx_init(mp, name, type, opts) \
	                        _audit_mtx_init(mp, name)
#define mtx_lock(mp)            lck_mtx_lock((mp)->mtx_lock)
#define mtx_unlock(mp)          lck_mtx_unlock((mp)->mtx_lock)
#define mtx_destroy(mp)         _audit_mtx_destroy(mp)
#define mtx_yield(mp)           lck_mtx_yield((mp)->mtx_lock)

/*
 * Sleep lock functions.
 */
void            _audit_slck_init(struct slck *lp, const char *grpname);
wait_result_t   _audit_slck_lock(struct slck *lp, int intr);
void            _audit_slck_unlock(struct slck *lp);
int             _audit_slck_trylock(struct slck *lp);
void            _audit_slck_assert(struct slck *lp, u_int assert);
void            _audit_slck_destroy(struct slck *lp);
#define slck_init(lp, name)     _audit_slck_init((lp), (name))
#define slck_lock(lp)           _audit_slck_lock((lp), 0)
#define slck_lock_sig(lp)       (_audit_slck_lock((lp), 1) != THREAD_AWAKENED)
#define slck_unlock(lp)         _audit_slck_unlock((lp))
#define slck_destroy(lp)        _audit_slck_destroy((lp))

/*
 * Recursive lock functions.
 */
void            _audit_rlck_init(struct rlck *lp, const char *grpname);
void            _audit_rlck_lock(struct rlck *lp);
void            _audit_rlck_unlock(struct rlck *lp);
void            _audit_rlck_assert(struct rlck *lp, u_int assert);
void            _audit_rlck_destroy(struct rlck *lp);
#define rlck_init(lp, name)     _audit_rlck_init((lp), (name))
#define rlck_lock(lp)           _audit_rlck_lock((lp))
#define rlck_unlock(lp)         _audit_rlck_unlock((lp))
#define rlck_destroy(lp)        _audit_rlck_destroy((lp))

/*
 * BSD rw locks.
 */
void    _audit_rw_init(struct rwlock *lp, const char *name);
void    _audit_rw_destroy(struct rwlock *lp);
#define rw_init(lp, name)       _audit_rw_init(lp, name)
#define rw_rlock(lp)            lck_rw_lock_shared((lp)->rw_lock)
#define rw_runlock(lp)          lck_rw_unlock_shared((lp)->rw_lock)
#define rw_wlock(lp)            lck_rw_lock_exclusive((lp)->rw_lock)
#define rw_wunlock(lp)          lck_rw_unlock_exclusive((lp)->rw_lock)
#define rw_destroy(lp)          _audit_rw_destroy(lp)

#define MA_OWNED                LCK_MTX_ASSERT_OWNED
#define RA_LOCKED               LCK_RW_ASSERT_HELD
#define RA_RLOCKED              LCK_RW_ASSERT_SHARED
#define RA_WLOCKED              LCK_RW_ASSERT_EXCLUSIVE
#define SA_LOCKED               LCK_RW_ASSERT_HELD
#define SA_XLOCKED              LCK_RW_ASSERT_EXCLUSIVE
#define SL_OWNED                LCK_MTX_ASSERT_OWNED
#define SL_NOTOWNED             LCK_MTX_ASSERT_NOTOWNED
#if DIAGNOSTIC
#define mtx_assert(mp, wht)     lck_mtx_assert((mp)->mtx_lock, wht)
#define rw_assert(lp, wht)      lck_rw_assert((lp)->rw_lock, wht)
#define sx_assert(lp, wht)      lck_rw_assert((lp)->sx_lock, wht)
#define rlck_assert(lp, wht)    _audit_rlck_assert((lp), wht)
#define slck_assert(lp, wht)    _audit_slck_assert((lp), wht)
#else
#define mtx_assert(mp, wht)
#define rw_assert(lp, wht)
#define sx_assert(lp, wht)
#define rlck_assert(lp, wht)
#define slck_assert(lp, wht)
#endif /* DIAGNOSTIC */

/*
 * Synchronization initialization.
 */
void    _audit_lck_grp_init(void);

/*
 * BSD (IPv6) event rate limiter.
 */
int _audit_ppsratecheck(struct timeval *lasttime, int *curpps, int maxpps);
#define ppsratecheck(tv, cr, mr)        _audit_ppsratecheck(tv, cr, mr)

#endif /* defined(_KERNEL) || defined(KERNEL) */
#endif /* _SECURITY_AUDIT_AUDIT_BSD_H */

/****************************************************************************/
/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)qsort.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdlib/qsort.c,v 1.15 2008/01/14 09:21:34 das Exp $");

#include <stdlib.h>
#include <pthread.h>
#include <dispatch/dispatch.h>
#include <stddef.h>
#include <string.h>
#include <libkern/OSAtomic.h>
#include <sys/mman.h>
#include <errno.h>
#include <os/lock.h>
#define __APPLE_API_PRIVATE
#include <machine/cpu_capabilities.h>

#ifdef I_AM_PSORT_R
typedef int		 cmp_t(void *, const void *, const void *);
#else
typedef int		 cmp_t(const void *, const void *);
#endif
#ifdef I_AM_PSORT_B
static inline char	*med3(char *, char *, char *, cmp_t ^, void *) __attribute__((always_inline));
#else
static inline char	*med3(char *, char *, char *, cmp_t *, void *) __attribute__((always_inline));
#endif
static inline void	 swapfunc(char *, char *, int, int) __attribute__((always_inline));

#define min(a, b)	(a) < (b) ? a : b

#define NARGS			((PAGESIZE - offsetof(struct page, args)) / sizeof(union args))
#define PAGESIZE		4096
#define PARALLEL_MIN_SIZE	2000	/* determine heuristically */

struct shared; /* forward reference */
union args {
    union args *next;
    struct {
	struct shared *shared;
	void *a;
	size_t n;
	int depth_limit;
    } /* anonymous */;
};

struct page {
    struct page *next;
    union args args[0];
};

struct shared {
    char *who;
    union args *freelist;
    struct page *pagelist;
#ifdef I_AM_PSORT_R
    void *thunk;
#endif
#ifdef I_AM_PSORT_B
    cmp_t ^cmp;
#else
    cmp_t *cmp;
#endif
    size_t es;
    size_t turnoff;
    dispatch_queue_t queue;
    dispatch_group_t group;
    os_unfair_lock sharedlock;
};

static union args *
getargs(struct shared *shared)
{
    union args *args;

    os_unfair_lock_lock(&shared->sharedlock);
    if(!shared->freelist) {
	struct page *page;
	union args *prev;
	int i;
	if((page = (struct page *)mmap(NULL, PAGESIZE, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0)) == NULL)
	    return NULL;
	page->next = shared->pagelist;
	shared->pagelist = page;
	prev = NULL;
	for(args = page->args, i = NARGS; i > 0; args++, i--) {
	    args->next = prev;
	    prev = args;
	}
	shared->freelist = prev;
    }
    args = shared->freelist;
    shared->freelist = args->next;
    os_unfair_lock_unlock(&shared->sharedlock);
    return args;
}

static void
returnargs(struct shared *shared, union args *args)
{
    os_unfair_lock_lock(&shared->sharedlock);
    args->next = shared->freelist;
    shared->freelist = args;
    os_unfair_lock_unlock(&shared->sharedlock);
}

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */
#define swapcode(TYPE, parmi, parmj, n) { 		\
	long i = (n) / sizeof (TYPE); 			\
	TYPE *pi = (TYPE *) (parmi); 		\
	TYPE *pj = (TYPE *) (parmj); 		\
	do { 						\
		TYPE	t = *pi;		\
		*pi++ = *pj;				\
		*pj++ = t;				\
        } while (--i > 0);				\
}

#define SWAPINIT(a, es) swaptype = ((char *)a - (char *)0) % sizeof(long) || \
	es % sizeof(long) ? 2 : es == sizeof(long)? 0 : 1;

static inline void
swapfunc(a, b, n, swaptype)
	char *a, *b;
	int n, swaptype;
{
	if(swaptype <= 1)
		swapcode(long, a, b, n)
	else
		swapcode(char, a, b, n)
}

#define swap(a, b)					\
	if (swaptype == 0) {				\
		long t = *(long *)(a);			\
		*(long *)(a) = *(long *)(b);		\
		*(long *)(b) = t;			\
	} else						\
		swapfunc(a, b, es, swaptype)

#define vecswap(a, b, n) 	if ((n) > 0) swapfunc(a, b, n, swaptype)

#ifdef I_AM_PSORT_R
#define	CMP(t, x, y) (cmp((t), (x), (y)))
#else
#define	CMP(t, x, y) (cmp((x), (y)))
#endif

static inline char *
med3(char *a, char *b, char *c,
#ifdef I_AM_PSORT_B
cmp_t ^cmp,
#else
cmp_t *cmp,
#endif
void *thunk
#ifndef I_AM_PSORT_R
__unused
#endif
)
{
	return CMP(thunk, a, b) < 0 ?
	       (CMP(thunk, b, c) < 0 ? b : (CMP(thunk, a, c) < 0 ? c : a ))
              :(CMP(thunk, b, c) > 0 ? b : (CMP(thunk, a, c) < 0 ? a : c ));
}

#ifdef __LP64__
#define DEPTH(x)	(2 * (flsl((long)(x)) - 1))
#else /* !__LP64__ */
#define DEPTH(x)	(2 * (fls((int)(x)) - 1))
#endif /* __LP64__ */

#ifdef I_AM_PSORT_R
int __heapsort_r(void *, size_t, size_t, void *, int (*)(void *, const void *, const void *));
#endif

static void _psort_parallel(void *x);

static void
_psort(void *a, size_t n, size_t es,
#ifdef I_AM_PSORT_R
void *thunk,
#else
#define thunk	NULL
#endif
#ifdef I_AM_PSORT_B
cmp_t ^cmp,
#else
cmp_t *cmp,
#endif
int depth_limit, struct shared *shared)
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	size_t d, r;
	int cmp_result;
	int swaptype, swap_cnt;

loop:
	if (depth_limit-- <= 0) {
#ifdef I_AM_PSORT_B
		heapsort_b(a, n, es, cmp);
#elif defined(I_AM_PSORT_R)
		__heapsort_r(a, n, es, thunk, cmp);
#else
		heapsort(a, n, es, cmp);
#endif
		return;
	}
	SWAPINIT(a, es);
	swap_cnt = 0;
	if (n < 7) {
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm; 
			     pl > (char *)a && CMP(thunk, pl - es, pl) > 0;
			     pl -= es)
				swap(pl, pl - es);
		return;
	}
	pm = (char *)a + (n / 2) * es;
	if (n > 7) {
		pl = a;
		pn = (char *)a + (n - 1) * es;
		if (n > 40) {
			d = (n / 8) * es;
			pl = med3(pl, pl + d, pl + 2 * d, cmp, thunk);
			pm = med3(pm - d, pm, pm + d, cmp, thunk);
			pn = med3(pn - 2 * d, pn - d, pn, cmp, thunk);
		}
		pm = med3(pl, pm, pn, cmp, thunk);
	}
	swap(a, pm);
	pa = pb = (char *)a + es;

	pc = pd = (char *)a + (n - 1) * es;
	for (;;) {
		while (pb <= pc && (cmp_result = CMP(thunk, pb, a)) <= 0) {
			if (cmp_result == 0) {
				swap_cnt = 1;
				swap(pa, pb);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (cmp_result = CMP(thunk, pc, a)) >= 0) {
			if (cmp_result == 0) {
				swap_cnt = 1;
				swap(pc, pd);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swap(pb, pc);
		swap_cnt = 1;
		pb += es;
		pc -= es;
	}

	pn = (char *)a + n * es;
	r = min(pa - (char *)a, pb - pa);
	vecswap(a, pb - r, r);
	r = min(pd - pc, pn - pd - es);
	vecswap(pb, pn - r, r);

	if (swap_cnt == 0) {  /* Switch to insertion sort */
		r = 1 + n / 4; /* n >= 7, so r >= 2 */
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm; 
			     pl > (char *)a && CMP(thunk, pl - es, pl) > 0;
			     pl -= es) {
				swap(pl, pl - es);
				if (++swap_cnt > r) goto nevermind;
			}
		return;
	}

nevermind:
	if ((r = pb - pa) > es) {
		r /= es;
		if (shared && r > shared->turnoff) {
			union args *args = getargs(shared);

			if (args == NULL)
				LIBC_ABORT("%s: getargs: %s", shared->who, strerror(errno));
			args->shared = shared;
			args->a = a;
			args->n = r;
			args->depth_limit = depth_limit;
			dispatch_group_async_f(shared->group, shared->queue, args,
					_psort_parallel);
		} else {
#ifdef I_AM_PSORT_R
			_psort(a, r, es, thunk, cmp, depth_limit, NULL);
#else
			_psort(a, r, es, cmp, depth_limit, NULL);
#endif
		}
	}
	if ((r = pd - pc) > es) {
		/* Iterate rather than recurse to save stack space */
		a = pn - r;
		n = r / es;
		goto loop;
	}
/*		psort(pn - r, r / es, es, cmp);*/
}

static void
_psort_parallel(void *x)
{
	union args *args = (union args *)x;
	struct shared *shared = args->shared;

	_psort(args->a, args->n, shared->es,
#ifdef I_AM_PSORT_R
		shared->thunk,
#endif
		shared->cmp, args->depth_limit, shared);
	returnargs(shared, args);
}

/* fast, approximate integer square root */
static size_t
isqrt(size_t x)
{
    size_t s = 1L << (flsl(x) / 2);
    return (s + x / s) / 2;
}

void
#ifdef I_AM_PSORT_R
psort_r(void *a, size_t n, size_t es, void *thunk, cmp_t *cmp)
#elif defined(I_AM_PSORT_B)
psort_b(void *a, size_t n, size_t es, cmp_t ^cmp)
#else
psort(void *a, size_t n, size_t es, cmp_t *cmp)
#endif
{
	if (n >= PARALLEL_MIN_SIZE && _NumCPUs() > 1) {
		struct shared shared;
		union args *args;

		bzero(&shared, sizeof(shared));
		shared.sharedlock = OS_UNFAIR_LOCK_INIT;
		if ((args = getargs(&shared)) != NULL) {
			struct page *p, *pp;
#ifdef I_AM_PSORT_R
			shared.who = "psort_r";
			shared.thunk = thunk;
#elif defined(I_AM_PSORT_B)
			shared.who = "psort_b";
#else
			shared.who = "psort";
#endif
			shared.cmp = cmp;
			shared.es = es;
			shared.queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
			shared.group = dispatch_group_create();
			args->a = a;
			args->n = n;
			args->depth_limit = DEPTH(n);
			args->shared = &shared;
			/*
			 * The turnoff value is the size of a partition that,
			 * below which, we stop doing in parallel, and just do
			 * in the current thread.  The value of sqrt(n) was
			 * determined heuristically.  There is a smaller
			 * dependence on the slowness of the comparison
			 * function, and there might be a dependence on the
			 * number of processors, but the algorithm has not been
			 * determined.  Because the sensitivity to the turnoff
			 * value is relatively low, we use a fast, approximate
			 * integer square root routine that is good enough for
			 * this purpose.
			 */
			shared.turnoff = isqrt(n);
			_psort_parallel(args);

			/* wait for queue to drain */
			dispatch_group_wait(shared.group, DISPATCH_TIME_FOREVER);
			dispatch_release(shared.group);
			for(p = shared.pagelist; p; p = pp) {
				pp = p->next;
				munmap(p, PAGESIZE);
			}
			return;
		}
	}
	/* Just call qsort */
#ifdef I_AM_PSORT_R
	qsort_r(a, n, es, thunk, cmp);
#elif defined(I_AM_PSORT_B)
	qsort_b(a, n, es, cmp);
#else
	qsort(a, n, es, cmp);
#endif
}

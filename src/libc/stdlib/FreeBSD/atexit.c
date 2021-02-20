/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-function-declaration"
#pragma clang diagnostic ignored "-Wstrict-prototypes"
#pragma clang diagnostic ignored "-Winvalid-pp-token"
#pragma clang diagnostic ignored "-Wint-conversion"

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)atexit.c	8.2 (Berkeley) 7/3/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdlib/atexit.c,v 1.8 2007/01/09 00:28:09 imp Exp $");
#include <TargetConditionals.h>

#include "namespace.h"
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#if (defined(__DYNAMIC__) || defined (__BLOCKS__)) && !TARGET_OS_DRIVERKIT
#include <dlfcn.h>
#endif /* defined(__DYNAMIC__) */
#include "atexit.h"
#include "un-namespace.h"

#ifdef __BLOCKS__
#include <Block.h>
#include <Block_private.h>
#endif /* __BLOCKS__ */
#include "libc_private.h"
#include <os/alloc_once_private.h>

#define	ATEXIT_FN_EMPTY	0
#define	ATEXIT_FN_STD	1
#define	ATEXIT_FN_CXA	2
#ifdef __BLOCKS__
#define	ATEXIT_FN_BLK	3
#endif /* __BLOCKS__ */

static pthread_mutex_t atexit_mutex = PTHREAD_MUTEX_INITIALIZER;

#define _MUTEX_LOCK(x)		if (__isthreaded) _pthread_mutex_lock(x)
#define _MUTEX_UNLOCK(x)	if (__isthreaded) _pthread_mutex_unlock(x)

struct atexit {
	struct atexit *next;			/* next in list */
	int ind;				/* next index in this table */
	struct atexit_fn {
		int fn_type;			/* ATEXIT_? from above */
		union {
			void (*std_func)(void);
			void (*cxa_func)(void *);
#ifdef __BLOCKS__
			void (^block)(void);
#endif /* __BLOCKS__ */
		} fn_ptr;			/* function pointer */
		void *fn_arg;			/* argument for CXA callback */
		void *fn_dso;			/* shared module handle */
	} fns[ATEXIT_SIZE];			/* the table itself */
};

static struct atexit *__atexit;		/* points to head of LIFO stack */
static int __atexit_new_registration;

__attribute__ ((visibility ("hidden")))
void
__atexit_init(void)
{
	__atexit = os_alloc_once(OS_ALLOC_ONCE_KEY_LIBSYSTEM_C,
			sizeof(struct atexit), NULL);
}

/*
 * Register the function described by 'fptr' to be called at application
 * exit or owning shared object unload time. This is a helper function
 * for atexit and __cxa_atexit.
 */
static int
atexit_register(struct atexit_fn *fptr)
{
	struct atexit *p = __atexit;
	assert(p);
	_MUTEX_LOCK(&atexit_mutex);
	while (p->ind >= ATEXIT_SIZE) {
		struct atexit *old__atexit;
		old__atexit = __atexit;
	        _MUTEX_UNLOCK(&atexit_mutex);
		if ((p = (struct atexit *)malloc(sizeof(*p))) == NULL)
			return (-1);
		_MUTEX_LOCK(&atexit_mutex);
		if (old__atexit != __atexit) {
			/* Lost race, retry operation */
			_MUTEX_UNLOCK(&atexit_mutex);
			free(p);
			_MUTEX_LOCK(&atexit_mutex);
			p = __atexit;
			continue;
		}
		p->ind = 0;
		p->next = __atexit;
		__atexit = p;
	}
	p->fns[p->ind++] = *fptr;
	__atexit_new_registration = 1;
	_MUTEX_UNLOCK(&atexit_mutex);
	return 0;
}

/*
 * Register a function to be performed at exit.
 */
int
atexit(void (*func)(void))
{
	struct atexit_fn fn;
	int error;

	fn.fn_type = ATEXIT_FN_STD;
	fn.fn_ptr.std_func = func;
	fn.fn_arg = NULL;
	fn.fn_dso = NULL;

#if defined(__DYNAMIC__) && !TARGET_OS_IPHONE && !TARGET_OS_DRIVERKIT
	// <rdar://problem/14596032&15173956>
	struct dl_info info;
	if (dladdr(func, &info)) {
		fn.fn_dso = info.dli_fbase;
	}
#endif

 	error = atexit_register(&fn);	
	return (error);
}

#ifdef __BLOCKS__
int
atexit_b(void (^block)(void))
{
	struct atexit_fn fn;
	int error;

	fn.fn_type = ATEXIT_FN_BLK;
	fn.fn_ptr.block = Block_copy(block);
	fn.fn_arg = NULL;
	fn.fn_dso = NULL;

 	error = atexit_register(&fn);	
	return (error);
}
#endif /* __BLOCKS__ */

/*
 * Register a function to be performed at exit or when an shared object
 * with given dso handle is unloaded dynamically.
 */
int
__cxa_atexit(void (*func)(void *), void *arg, void *dso)
{
	struct atexit_fn fn;
	int error;

	fn.fn_type = ATEXIT_FN_CXA;
	fn.fn_ptr.cxa_func = func;;
	fn.fn_arg = arg;
	fn.fn_dso = dso;

 	error = atexit_register(&fn);	
	return (error);
}

static bool
__cxa_in_range(const struct __cxa_range_t ranges[],
			   unsigned int count,
			   const void* fn)
{
	uintptr_t addr = (uintptr_t)fn;

	unsigned int i;
	for (i = 0; i < count; ++i) {
		const struct __cxa_range_t *r = &ranges[i];
		if (addr < (uintptr_t)r->addr) {
			continue;
		}
		if (addr < ((uintptr_t)r->addr + r->length)) {
			return true;
		}
	}
	return false;
}

/*
 * Call handlers registered via __cxa_atexit/atexit that are in a
 * a range specified.
 * Note: rangeCount==0, means call all handlers.
 */
void
__cxa_finalize_ranges(const struct __cxa_range_t ranges[], unsigned int count)
{
	struct atexit *p;
	struct atexit_fn *fn;
	int n;
	_MUTEX_LOCK(&atexit_mutex);

restart:
	for (p = __atexit; p; p = p->next) {
		for (n = p->ind; --n >= 0;) {
			fn = &p->fns[n];

			if (fn->fn_type == ATEXIT_FN_EMPTY) {
				continue; // already been called
			}

			// Verify that the entry is within the range being unloaded.
			if (count > 0) {
				if (fn->fn_type == ATEXIT_FN_CXA) {
					// for __cxa_atexit(), call if *dso* is in range be unloaded
					if (!__cxa_in_range(ranges, count, fn->fn_dso)) {
						continue; // not being unloaded yet
					}
				} else if (fn->fn_type == ATEXIT_FN_STD) {
					// for atexit, call if *function* is in range be unloaded
					if (!__cxa_in_range(ranges, count,  fn->fn_ptr.std_func)) {
						continue; // not being unloaded yet
					}
#ifdef __BLOCKS__
				} else if (fn->fn_type == ATEXIT_FN_BLK) {
					// for atexit_b, call if block code is in range be unloaded
					void *a = ((struct Block_layout *)fn->fn_ptr.block)->invoke;
					if (!__cxa_in_range(ranges, count, a)) {
						continue; // not being unloaded yet
					}
#endif // __BLOCKS__
				}
			}

			// Clear the entry to indicate that this handler has been called.
			int fn_type = fn->fn_type;
			fn->fn_type = ATEXIT_FN_EMPTY;

			// Detect recursive registrations.
			__atexit_new_registration = 0;
			_MUTEX_UNLOCK(&atexit_mutex);

			// Call the handler.
			if (fn_type == ATEXIT_FN_CXA) {
				fn->fn_ptr.cxa_func(fn->fn_arg);
			} else if (fn_type == ATEXIT_FN_STD) {
				fn->fn_ptr.std_func();
#ifdef __BLOCKS__
			} else if (fn_type == ATEXIT_FN_BLK) {
				fn->fn_ptr.block();
#endif // __BLOCKS__
			}

			// Call any recursively registered handlers.
			_MUTEX_LOCK(&atexit_mutex);
			if (__atexit_new_registration) {
			    goto restart;
			}
		}
	}
	_MUTEX_UNLOCK(&atexit_mutex);
}


/*
 * Call all handlers registered with __cxa_atexit for the shared
 * object owning 'dso'.  Note: if 'dso' is NULL, then all remaining
 * handlers are called.
 */
void
__cxa_finalize(const void *dso)
{
	if (dso != NULL) {
		// Note: this should not happen as only dyld should be calling
		// this and dyld has switched to call __cxa_finalize_ranges directly.
		struct __cxa_range_t range;
		range.addr = dso;
		range.length = 1;
		__cxa_finalize_ranges(&range, 1);
	} else {
		__cxa_finalize_ranges(NULL, 0);
	}
}

#if !TARGET_IPHONE_SIMULATOR && (__i386__ || __x86_64__)
/*
 * Support for thread_local in C++, using existing _tlv_atexit() in libdyld
 */

void _tlv_atexit(void(*f)(void*), void* arg); /* in libdyld */

void
__cxa_thread_atexit(void(*f)(void*), void* arg)
{
    _tlv_atexit(f, arg);
}
#endif
#pragma clang diagnostic pop

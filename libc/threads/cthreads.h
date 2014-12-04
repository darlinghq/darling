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
 * Cthreads.
 */

#ifndef	_CTHREADS_
#define	_CTHREADS_ 1

#if defined(__cplusplus)
#define __DECLBEGIN extern "C" {
#define __DECLEND }
#else
#define __DECLBEGIN
#define __DECLEND
#endif

typedef void *any_t;

/*
 * Spin locks.
 */
__DECLBEGIN
extern void spin_unlock(int *p);
extern void spin_lock(int *p);
__DECLEND

#include <mach/mach_types.h>
#include <mach/boolean.h>

#ifndef MACRO_BEGIN

#define	NEVER FALSE

#define	MACRO_BEGIN	do {
#define	MACRO_END	} while (NEVER)

#endif	/* MACRO_BEGIN */

/*
 * C Threads package initialization.
 */
__DECLBEGIN
extern void cthread_init();
__DECLEND

#include <stdlib.h>

/*
 * Queues.
 */
typedef struct cthread_queue {
	struct cthread_queue_item *head;
	struct cthread_queue_item *tail;
} *cthread_queue_t;

typedef struct cthread_queue_item {
	struct cthread_queue_item *next;
} *cthread_queue_item_t;

#define	NO_QUEUE_ITEM	((cthread_queue_item_t) 0)

#define	QUEUE_INITIALIZER	{ NO_QUEUE_ITEM, NO_QUEUE_ITEM }

#define	cthread_queue_alloc()	((cthread_queue_t) calloc(1, sizeof(struct cthread_queue)))
#define	cthread_queue_init(q)	((q)->head = (q)->tail = 0)
#define	cthread_queue_free(q)	free((any_t) (q))

#define	cthread_queue_enq(q, x) \
	MACRO_BEGIN \
		(x)->next = 0; \
		if ((q)->tail == 0) \
			(q)->head = (cthread_queue_item_t) (x); \
		else \
			(q)->tail->next = (cthread_queue_item_t) (x); \
		(q)->tail = (cthread_queue_item_t) (x); \
	MACRO_END

#define	cthread_queue_preq(q, x) \
	MACRO_BEGIN \
		if ((q)->tail == 0) \
			(q)->tail = (cthread_queue_item_t) (x); \
		((cthread_queue_item_t) (x))->next = (q)->head; \
		(q)->head = (cthread_queue_item_t) (x); \
	MACRO_END

#define	cthread_queue_head(q, t)	((t) ((q)->head))

#define	cthread_queue_deq(q, t, x) \
	MACRO_BEGIN \
		if (((x) = (t) ((q)->head)) != 0 && \
		    ((q)->head = (cthread_queue_item_t) ((x)->next)) == 0) \
			(q)->tail = 0; \
	MACRO_END

#define	cthread_queue_map(q, t, f) \
	MACRO_BEGIN \
		register cthread_queue_item_t x, next; \
		for (x = (cthread_queue_item_t) ((q)->head); x != 0; x = next) { \
			next = x->next; \
			(*(f))((t) x); \
		} \
	MACRO_END


/*
 * Mutex objects.
 */
typedef struct mutex {
	int lock;
	char *name;
} *mutex_t;

#define	MUTEX_INITIALIZER	{ 0, 0 }

#define	mutex_alloc()		((mutex_t) calloc(1, sizeof(struct mutex)))
#define	mutex_init(m)		((m)->lock = 0)
#define	mutex_set_name(m, x)	((m)->name = (x))
#define	mutex_name(m)		((m)->name != 0 ? (m)->name : "?")
#define	mutex_clear(m)		/* nop */
#define	mutex_free(m)		free((any_t) (m))

#define	mutex_lock(m) \
	MACRO_BEGIN \
		if (! mutex_try_lock(m)) mutex_wait_lock(m); \
	MACRO_END

__DECLBEGIN
extern int mutex_try_lock(mutex_t m);	/* nonblocking */
extern void mutex_wait_lock(mutex_t m);	/* blocking */
extern void mutex_unlock(mutex_t m);
__DECLEND

/*
 * Condition variables.
 */
typedef struct condition {
	int lock;
	struct cthread_queue queue;
	char *name;
} *condition_t;

#define	CONDITION_INITIALIZER		{ 0, QUEUE_INITIALIZER, 0 }

#define	condition_alloc()		((condition_t) calloc(1, sizeof(struct condition)))
#define	condition_init(c)		MACRO_BEGIN (c)->lock = 0; cthread_queue_init(&(c)->queue); MACRO_END
#define	condition_set_name(c, x)	((c)->name = (x))
#define	condition_name(c)		((c)->name != 0 ? (c)->name : "?")
#define	condition_clear(c)		MACRO_BEGIN condition_broadcast(c); spin_lock(&(c)->lock); MACRO_END
#define	condition_free(c)		MACRO_BEGIN condition_clear(c); free((any_t) (c)); MACRO_END

#define	condition_signal(c) \
	MACRO_BEGIN \
		if ((c)->queue.head) cond_signal(c); \
	MACRO_END

#define	condition_broadcast(c) \
	MACRO_BEGIN \
		if ((c)->queue.head) cond_broadcast(c); \
	MACRO_END

__DECLBEGIN
extern void cond_signal(condition_t c);
extern void cond_broadcast(condition_t c);
extern void condition_wait(condition_t c, mutex_t m);
__DECLEND

/*
 * Threads.
 */

typedef any_t (*cthread_fn_t)(any_t arg);

#import <setjmp.h>

typedef struct cthread {
	struct cthread *next;
	thread_port_t real_thread;
	struct mutex lock;
	struct condition done;
	int state;
#if defined(__cplusplus)
	jmp_buf catchBuf;
#else
	jmp_buf catch;
#endif
	cthread_fn_t func;
	any_t arg;
	any_t result;
	const char *name;
	any_t data;
} *cthread_t;

#define	NO_CTHREAD	((cthread_t) 0)

__DECLBEGIN
extern cthread_t cthread_fork(cthread_fn_t func, any_t arg);
extern void cthread_detach(cthread_t t);
extern any_t cthread_join(cthread_t t);
extern void cthread_yield(void);
extern void cthread_exit(any_t result);
extern kern_return_t cthread_priority(
        cthread_t	t,
        int		priority,
        boolean_t	set_max);
extern kern_return_t cthread_max_priority(
        cthread_t	t,
        mach_port_t	pset,
        int		max_priority);
extern kern_return_t cthread_abort(cthread_t t);
__DECLEND

/*
 * This structure must agree with struct cproc in cthread_internals.h
 */
typedef struct ur_cthread {
	struct ur_cthread *next;
	cthread_t incarnation;
} *ur_cthread_t;

__DECLBEGIN
extern int cthread_sp(void);

extern int cthread_stack_mask;

extern ur_cthread_t ur_cthread_self(void);
#define cthread_thread(c)	(c->real_thread)
extern void cthread_set_errno_self(int e, int nocancel);
extern int cthread_errno(void);
#define	cthread_assoc(id, t)	(((ur_cthread_t) (id))->incarnation = (t))
#define	cthread_self()		(ur_cthread_self()->incarnation)

extern void cthread_set_name(cthread_t t, const char *name);
extern const char *cthread_name(cthread_t t);
extern int cthread_count(void);
extern void cthread_set_limit(int n);
extern int cthread_limit(void);
__DECLEND

#define	cthread_set_data(t, x)	((t)->data = (x))
#define	cthread_data(t)		((t)->data)

/*
 * Spin locks.
 */
#define	spin_unlock(p)		(*(p) = 0)

/*
 * Mutex locks.
 */
#define	mutex_unlock(m)		((m)->lock = 0)


/*
 * Debugging support.
 */
#ifdef	CTHREADS_DEBUG

#ifndef	ASSERT
/*
 * Assertion macro, similar to <assert.h>
 */
#import <stdio.h>
#define	ASSERT(p) \
	MACRO_BEGIN \
		if (!(p)) { \
			fprintf(stderr, \
				"File %s, line %d: assertion p failed.\n", \
				__FILE__, __LINE__); \
			abort(); \
		} \
	MACRO_END

#endif	/* ASSERT */

#define	SHOULDNT_HAPPEN	0

extern int cthread_debug;

#else	/* CTHREADS_DEBUG */

#ifndef	ASSERT
#define	ASSERT(p)
#endif	/* ASSERT */

#endif	/* CTHREADS_DEBUG */

#endif	/* _CTHREADS_ */

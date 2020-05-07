/*
 * Copyright (c) 2000-2009 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon rights
 * to redistribute these changes.
 */
/*
 */
/*
 *	File:	queue.h
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	1985
 *
 *	Type definitions for generic queues.
 *
 */

#ifndef _KERN_QUEUE_H_
#define _KERN_QUEUE_H_

#include <mach/mach_types.h>
#include <kern/macro_help.h>

#include <sys/cdefs.h>
#include <string.h>

__BEGIN_DECLS

/*
 * Queue Management APIs
 *
 * There are currently two subtly different methods of maintining
 * a queue of objects. Both APIs are contained in this file, and
 * unfortunately overlap.
 * (there is also a third way maintained in bsd/sys/queue.h)
 *
 * Both methods use a common queue head and linkage pattern:
 *      The head of a queue is declared as:
 *              queue_head_t q_head;
 *
 *      Elements in this queue are chained together using
 *      struct queue_entry objects embedded within a structure:
 *              struct some_data {
 *                      int field1;
 *                      int field2;
 *                      ...
 *                      queue_chain_t link;
 *                      ...
 *                      int last_field;
 *              };
 *      struct some_data is referred to as the queue "element."
 *      (note that queue_chain_t is typedef'd to struct queue_entry)
 *
 * IMPORTANT: The two queue iteration methods described below are not
 *            compatible with one another. You must choose one and be careful
 *            to use only the supported APIs for that method.
 *
 * Method 1: chaining of queue_chain_t (linkage chains)
 *      This method uses the next and prev pointers of the struct queue_entry
 *      linkage object embedded in a queue element to point to the next or
 *      previous queue_entry structure in the chain. The head of the queue
 *      (the queue_head_t object) will point to the first and last
 *      struct queue_entry object, and both the next and prev pointer will
 *      point back to the head if the queue is empty.
 *
 *      This method is the most flexible method of chaining objects together
 *      as it allows multiple chains through a given object, by embedding
 *      multiple queue_chain_t objects in the structure, while simultaneously
 *      providing fast removal and insertion into the queue using only
 *      struct queue_entry object pointers.
 *
 *      ++ Valid APIs for this style queue ++
 *      -------------------------------------
 *              [C] queue_init
 *              [C] queue_first
 *              [C] queue_next
 *              [C] queue_last
 *              [C] queue_prev
 *              [C] queue_end
 *              [C] queue_empty
 *
 *              [1] enqueue
 *              [1] dequeue
 *              [1] enqueue_head
 *              [1] enqueue_tail
 *              [1] dequeue_head
 *              [1] dequeue_tail
 *              [1] remqueue
 *              [1] insque
 *              [1] remque
 *              [1] re_queue_head
 *              [1] re_queue_tail
 *              [1] movqueue
 *              [1] qe_element
 *              [1] qe_foreach
 *              [1] qe_foreach_safe
 *              [1] qe_foreach_element
 *              [1] qe_foreach_element_safe
 *
 * Method 2: chaining of elements (element chains)
 *      This method uses the next and prev pointers of the struct queue_entry
 *      linkage object embedded in a queue element to point to the next or
 *      previous queue element (not another queue_entry). The head of the
 *      queue will point to the first and last queue element (struct some_data
 *      from the above example) NOT the embedded queue_entry structure. The
 *      first queue element will have a prev pointer that points to the
 *      queue_head_t, and the last queue element will have a next pointer
 *      that points to the queue_head_t.
 *
 *      This method requires knowledge of the queue_head_t of the queue on
 *      which an element resides in order to remove the element. Iterating
 *      through the elements of the queue is also more cumbersome because
 *      a check against the head pointer plus a cast then offset operation
 *      must be performed at each step of the iteration.
 *
 *      ++ Valid APIs for this style queue ++
 *      -------------------------------------
 *              [C] queue_init
 *              [C] queue_first
 *              [C] queue_next
 *              [C] queue_last
 *              [C] queue_prev
 *              [C] queue_end
 *              [C] queue_empty
 *
 *              [2] queue_enter
 *              [2] queue_enter_first
 *              [2] queue_insert_before
 *              [2] queue_insert_after
 *              [2] queue_field
 *              [2] queue_remove
 *              [2] queue_remove_first
 *              [2] queue_remove_last
 *              [2] queue_assign
 *              [2] queue_new_head
 *              [2] queue_iterate
 *
 * Legend:
 *      [C] -> API common to both methods
 *      [1] -> API used only in method 1 (linkage chains)
 *      [2] -> API used only in method 2 (element chains)
 */

/*
 *	A generic doubly-linked list (queue).
 */

struct queue_entry {
	struct queue_entry      *next;          /* next element */
	struct queue_entry      *prev;          /* previous element */

#if __arm__ && (__BIGGEST_ALIGNMENT__ > 4)
/* For the newer ARMv7k ABI where 64-bit types are 64-bit aligned, but pointers
 * are 32-bit:
 * Since this type is so often cast to various 64-bit aligned types
 * aligning it to 64-bits will avoid -wcast-align without needing
 * to disable it entirely. The impact on memory footprint should be
 * negligible.
 */
} __attribute__ ((aligned(8)));
#else
};
#endif

typedef struct queue_entry      *queue_t;
typedef struct queue_entry      queue_head_t;
typedef struct queue_entry      queue_chain_t;
typedef struct queue_entry      *queue_entry_t;

/*
 *	enqueue puts "elt" on the "queue".
 *	dequeue returns the first element in the "queue".
 *	remqueue removes the specified "elt" from its queue.
 */

#define enqueue(queue, elt)      enqueue_tail(queue, elt)
#define dequeue(queue)          dequeue_head(queue)

#ifdef XNU_KERNEL_PRIVATE
#include <kern/debug.h>
static inline void
__QUEUE_ELT_VALIDATE(queue_entry_t elt)
{
	queue_entry_t   elt_next, elt_prev;

	if (__improbable(elt == (queue_entry_t)NULL)) {
		panic("Invalid queue element %p", elt);
	}

	elt_next = elt->next;
	elt_prev = elt->prev;

	if (__improbable(elt_next == (queue_entry_t)NULL || elt_prev == (queue_entry_t)NULL)) {
		panic("Invalid queue element pointers for %p: next %p prev %p", elt, elt_next, elt_prev);
	}
	if (__improbable(elt_next->prev != elt || elt_prev->next != elt)) {
		panic("Invalid queue element linkage for %p: next %p next->prev %p prev %p prev->next %p",
		    elt, elt_next, elt_next->prev, elt_prev, elt_prev->next);
	}
}

static inline void
__DEQUEUE_ELT_CLEANUP(queue_entry_t elt)
{
	(elt)->next = (queue_entry_t)NULL;
	(elt)->prev = (queue_entry_t)NULL;
}
#else
#define __QUEUE_ELT_VALIDATE(elt) do { } while (0)
#define __DEQUEUE_ELT_CLEANUP(elt) do { } while(0)
#endif /* !XNU_KERNEL_PRIVATE */

static __inline__ void
enqueue_head(
	queue_t         que,
	queue_entry_t   elt)
{
	queue_entry_t   old_head;

	__QUEUE_ELT_VALIDATE((queue_entry_t)que);
	old_head = que->next;
	elt->next = old_head;
	elt->prev = que;
	old_head->prev = elt;
	que->next = elt;
}

static __inline__ void
enqueue_tail(
	queue_t         que,
	queue_entry_t   elt)
{
	queue_entry_t   old_tail;

	__QUEUE_ELT_VALIDATE((queue_entry_t)que);
	old_tail = que->prev;
	elt->next = que;
	elt->prev = old_tail;
	old_tail->next = elt;
	que->prev = elt;
}

static __inline__ queue_entry_t
dequeue_head(
	queue_t que)
{
	queue_entry_t   elt = (queue_entry_t)NULL;
	queue_entry_t   new_head;

	if (que->next != que) {
		elt = que->next;
		__QUEUE_ELT_VALIDATE(elt);
		new_head = elt->next; /* new_head may point to que if elt was the only element */
		new_head->prev = que;
		que->next = new_head;
		__DEQUEUE_ELT_CLEANUP(elt);
	}

	return elt;
}

static __inline__ queue_entry_t
dequeue_tail(
	queue_t que)
{
	queue_entry_t   elt = (queue_entry_t)NULL;
	queue_entry_t   new_tail;

	if (que->prev != que) {
		elt = que->prev;
		__QUEUE_ELT_VALIDATE(elt);
		new_tail = elt->prev; /* new_tail may point to queue if elt was the only element */
		new_tail->next = que;
		que->prev = new_tail;
		__DEQUEUE_ELT_CLEANUP(elt);
	}

	return elt;
}

static __inline__ void
remqueue(
	queue_entry_t   elt)
{
	queue_entry_t   next_elt, prev_elt;

	__QUEUE_ELT_VALIDATE(elt);
	next_elt = elt->next;
	prev_elt = elt->prev; /* next_elt may equal prev_elt (and the queue head) if elt was the only element */
	next_elt->prev = prev_elt;
	prev_elt->next = next_elt;
	__DEQUEUE_ELT_CLEANUP(elt);
}

static __inline__ void
insque(
	queue_entry_t   entry,
	queue_entry_t   pred)
{
	queue_entry_t   successor;

	__QUEUE_ELT_VALIDATE(pred);
	successor = pred->next;
	entry->next = successor;
	entry->prev = pred;
	successor->prev = entry;
	pred->next = entry;
}

static __inline__ void
remque(
	queue_entry_t elt)
{
	queue_entry_t   next_elt, prev_elt;

	__QUEUE_ELT_VALIDATE(elt);
	next_elt = elt->next;
	prev_elt = elt->prev; /* next_elt may equal prev_elt (and the queue head) if elt was the only element */
	next_elt->prev = prev_elt;
	prev_elt->next = next_elt;
	__DEQUEUE_ELT_CLEANUP(elt);
}

/*
 *	Function:	re_queue_head
 *	Parameters:
 *		queue_t que       : queue onto which elt will be pre-pended
 *		queue_entry_t elt : element to re-queue
 *	Description:
 *		Remove elt from its current queue and put it onto the
 *		head of a new queue
 *	Note:
 *		This should only be used with Method 1 queue iteration (linkage chains)
 */
static __inline__ void
re_queue_head(queue_t que, queue_entry_t elt)
{
	queue_entry_t   n_elt, p_elt;

	__QUEUE_ELT_VALIDATE(elt);
	__QUEUE_ELT_VALIDATE((queue_entry_t)que);

	/* remqueue */
	n_elt = elt->next;
	p_elt = elt->prev; /* next_elt may equal prev_elt (and the queue head) if elt was the only element */
	n_elt->prev = p_elt;
	p_elt->next = n_elt;

	/* enqueue_head */
	n_elt = que->next;
	elt->next = n_elt;
	elt->prev = que;
	n_elt->prev = elt;
	que->next = elt;
}

/*
 *	Function:	re_queue_tail
 *	Parameters:
 *		queue_t que       : queue onto which elt will be appended
 *		queue_entry_t elt : element to re-queue
 *	Description:
 *		Remove elt from its current queue and put it onto the
 *		end of a new queue
 *	Note:
 *		This should only be used with Method 1 queue iteration (linkage chains)
 */
static __inline__ void
re_queue_tail(queue_t que, queue_entry_t elt)
{
	queue_entry_t   n_elt, p_elt;

	__QUEUE_ELT_VALIDATE(elt);
	__QUEUE_ELT_VALIDATE((queue_entry_t)que);

	/* remqueue */
	n_elt = elt->next;
	p_elt = elt->prev; /* next_elt may equal prev_elt (and the queue head) if elt was the only element */
	n_elt->prev = p_elt;
	p_elt->next = n_elt;

	/* enqueue_tail */
	p_elt = que->prev;
	elt->next = que;
	elt->prev = p_elt;
	p_elt->next = elt;
	que->prev = elt;
}

/*
 *	Macro:		qe_element
 *	Function:
 *		Convert a queue_entry_t to a queue element pointer.
 *		Get a pointer to the user-defined element containing
 *		a given queue_entry_t
 *	Header:
 *		<type> * qe_element(queue_entry_t qe, <type>, field)
 *			qe      - queue entry to convert
 *			<type>  - what's in the queue (e.g., struct some_data)
 *			<field> - is the chain field in <type>
 *	Note:
 *		Do not use pointer types for <type>
 */
#define qe_element(qe, type, field) __container_of(qe, type, field)

/*
 *	Macro:		qe_foreach
 *	Function:
 *		Iterate over each queue_entry_t structure.
 *		Generates a 'for' loop, setting 'qe' to
 *		each queue_entry_t in the queue.
 *	Header:
 *		qe_foreach(queue_entry_t qe, queue_t head)
 *			qe   - iteration variable
 *			head - pointer to queue_head_t (head of queue)
 *	Note:
 *		This should only be used with Method 1 queue iteration (linkage chains)
 */
#define qe_foreach(qe, head) \
	for (qe = (head)->next; qe != (head); qe = (qe)->next)

/*
 *	Macro:		qe_foreach_safe
 *	Function:
 *		Safely iterate over each queue_entry_t structure.
 *
 *		Use this iterator macro if you plan to remove the
 *		queue_entry_t, qe, from the queue during the
 *		iteration.
 *	Header:
 *		qe_foreach_safe(queue_entry_t qe, queue_t head)
 *			qe   - iteration variable
 *			head - pointer to queue_head_t (head of queue)
 *	Note:
 *		This should only be used with Method 1 queue iteration (linkage chains)
 */
#define qe_foreach_safe(qe, head) \
	for (queue_entry_t _ne = ((head)->next)->next, \
	         __ ## qe ## _unused_shadow __unused = (qe = (head)->next); \
	     qe != (head); \
	     qe = _ne, _ne = (qe)->next)

/*
 *	Macro:		qe_foreach_element
 *	Function:
 *		Iterate over each _element_ in a queue
 *		where each queue_entry_t points to another
 *		queue_entry_t, i.e., managed by the [de|en]queue_head/
 *		[de|en]queue_tail / remqueue / etc. function.
 *	Header:
 *		qe_foreach_element(<type> *elt, queue_t head, <field>)
 *			elt     - iteration variable
 *			<type>  - what's in the queue (e.g., struct some_data)
 *			<field> - is the chain field in <type>
 *	Note:
 *		This should only be used with Method 1 queue iteration (linkage chains)
 */
#define qe_foreach_element(elt, head, field) \
	for (elt = qe_element((head)->next, typeof(*(elt)), field); \
	     &((elt)->field) != (head); \
	     elt = qe_element((elt)->field.next, typeof(*(elt)), field))

/*
 *	Macro:		qe_foreach_element_safe
 *	Function:
 *		Safely iterate over each _element_ in a queue
 *		where each queue_entry_t points to another
 *		queue_entry_t, i.e., managed by the [de|en]queue_head/
 *		[de|en]queue_tail / remqueue / etc. function.
 *
 *		Use this iterator macro if you plan to remove the
 *		element, elt, from the queue during the iteration.
 *	Header:
 *		qe_foreach_element_safe(<type> *elt, queue_t head, <field>)
 *			elt     - iteration variable
 *			<type>  - what's in the queue (e.g., struct some_data)
 *			<field> - is the chain field in <type>
 *	Note:
 *		This should only be used with Method 1 queue iteration (linkage chains)
 */
#define qe_foreach_element_safe(elt, head, field) \
	for (typeof(*(elt)) *_nelt = qe_element(((head)->next)->next, typeof(*(elt)), field), \
	     *__ ## elt ## _unused_shadow __unused = \
	         (elt = qe_element((head)->next, typeof(*(elt)), field)); \
	     &((elt)->field) != (head); \
	     elt = _nelt, _nelt = qe_element((elt)->field.next, typeof(*(elt)), field)) \

#ifdef XNU_KERNEL_PRIVATE

/* Dequeue an element from head, or return NULL if the queue is empty */
#define qe_dequeue_head(head, type, field) ({ \
	queue_entry_t _tmp_entry = dequeue_head((head)); \
	type *_tmp_element = (type*) NULL; \
	if (_tmp_entry != (queue_entry_t) NULL) \
	        _tmp_element = qe_element(_tmp_entry, type, field); \
	_tmp_element; \
})

/* Dequeue an element from tail, or return NULL if the queue is empty */
#define qe_dequeue_tail(head, type, field) ({ \
	queue_entry_t _tmp_entry = dequeue_tail((head)); \
	type *_tmp_element = (type*) NULL; \
	if (_tmp_entry != (queue_entry_t) NULL) \
	        _tmp_element = qe_element(_tmp_entry, type, field); \
	_tmp_element; \
})

/* Peek at the first element, or return NULL if the queue is empty */
#define qe_queue_first(head, type, field) ({ \
	queue_entry_t _tmp_entry = queue_first((head)); \
	type *_tmp_element = (type*) NULL; \
	if (_tmp_entry != (queue_entry_t) head) \
	        _tmp_element = qe_element(_tmp_entry, type, field); \
	_tmp_element; \
})

/* Peek at the last element, or return NULL if the queue is empty */
#define qe_queue_last(head, type, field) ({ \
	queue_entry_t _tmp_entry = queue_last((head)); \
	type *_tmp_element = (type*) NULL; \
	if (_tmp_entry != (queue_entry_t) head) \
	        _tmp_element = qe_element(_tmp_entry, type, field); \
	_tmp_element; \
})

#endif /* XNU_KERNEL_PRIVATE */

/*
 *	Macro:		queue_init
 *	Function:
 *		Initialize the given queue.
 *	Header:
 *		void queue_init(q)
 *			queue_t		q;	\* MODIFIED *\
 */
#define queue_init(q)   \
MACRO_BEGIN             \
	(q)->next = (q);\
	(q)->prev = (q);\
MACRO_END

/*
 *	Macro:		queue_head_init
 *	Function:
 *		Initialize the given queue head
 *	Header:
 *		void queue_head_init(q)
 *			queue_head_t	q;	\* MODIFIED *\
 */
#define queue_head_init(q) \
	queue_init(&(q))

/*
 *	Macro:		queue_chain_init
 *	Function:
 *		Initialize the given queue chain element
 *	Header:
 *		void queue_chain_init(q)
 *			queue_chain_t	q;	\* MODIFIED *\
 */
#define queue_chain_init(q) \
	queue_init(&(q))

/*
 *	Macro:		queue_first
 *	Function:
 *		Returns the first entry in the queue,
 *	Header:
 *		queue_entry_t queue_first(q)
 *			queue_t	q;		\* IN *\
 */
#define queue_first(q)  ((q)->next)

/*
 *	Macro:		queue_next
 *	Function:
 *		Returns the entry after an item in the queue.
 *	Header:
 *		queue_entry_t queue_next(qc)
 *			queue_t qc;
 */
#define queue_next(qc)  ((qc)->next)

/*
 *	Macro:		queue_last
 *	Function:
 *		Returns the last entry in the queue.
 *	Header:
 *		queue_entry_t queue_last(q)
 *			queue_t	q;		\* IN *\
 */
#define queue_last(q)   ((q)->prev)

/*
 *	Macro:		queue_prev
 *	Function:
 *		Returns the entry before an item in the queue.
 *	Header:
 *		queue_entry_t queue_prev(qc)
 *			queue_t qc;
 */
#define queue_prev(qc)  ((qc)->prev)

/*
 *	Macro:		queue_end
 *	Function:
 *		Tests whether a new entry is really the end of
 *		the queue.
 *	Header:
 *		boolean_t queue_end(q, qe)
 *			queue_t q;
 *			queue_entry_t qe;
 */
#define queue_end(q, qe)        ((q) == (qe))

/*
 *	Macro:		queue_empty
 *	Function:
 *		Tests whether a queue is empty.
 *	Header:
 *		boolean_t queue_empty(q)
 *			queue_t q;
 */
#define queue_empty(q)          queue_end((q), queue_first(q))

/*
 *	Function:	movqueue
 *	Parameters:
 *		queue_t _old : head of a queue whose items will be moved
 *		queue_t _new : new queue head onto which items will be moved
 *	Description:
 *		Rebase queue items in _old onto _new then re-initialize
 *		the _old object to an empty queue.
 *		Equivalent to the queue_new_head Method 2 macro
 *	Note:
 *		Similar to the queue_new_head macro, this macros is intented
 *		to function as an initializer method for '_new' and thus may
 *		leak any list items that happen to be on the '_new' list.
 *		This should only be used with Method 1 queue iteration (linkage chains)
 */
static __inline__ void
movqueue(queue_t _old, queue_t _new)
{
	queue_entry_t   next_elt, prev_elt;

	__QUEUE_ELT_VALIDATE((queue_entry_t)_old);

	if (queue_empty(_old)) {
		queue_init(_new);
		return;
	}

	/*
	 * move the queue at _old to _new
	 * and re-initialize _old
	 */
	next_elt = _old->next;
	prev_elt = _old->prev;

	_new->next = next_elt;
	_new->prev = prev_elt;
	next_elt->prev = _new;
	prev_elt->next = _new;

	queue_init(_old);
}

/*----------------------------------------------------------------*/
/*
 * Macros that operate on generic structures.  The queue
 * chain may be at any location within the structure, and there
 * may be more than one chain.
 */

/*
 *	Macro:		queue_enter
 *	Function:
 *		Insert a new element at the tail of the queue.
 *	Header:
 *		void queue_enter(q, elt, type, field)
 *			queue_t q;
 *			<type> elt;
 *			<type> is what's in our queue
 *			<field> is the chain field in (*<type>)
 *	Note:
 *		This should only be used with Method 2 queue iteration (element chains)
 *
 *		We insert a compiler barrier after setting the fields in the element
 *		to ensure that the element is updated before being added to the queue,
 *		which is especially important because stackshot, which operates from
 *		debugger context, iterates several queues that use this macro (the tasks
 *		lists and threads lists) without locks. Without this barrier, the
 *		compiler may re-order the instructions for this macro in a way that
 *		could cause stackshot to trip over an inconsistent queue during
 *		iteration.
 */
#define queue_enter(head, elt, type, field)                     \
MACRO_BEGIN                                                     \
	queue_entry_t __prev;                                   \
                                                                \
	__prev = (head)->prev;                                  \
	(elt)->field.prev = __prev;                             \
	(elt)->field.next = head;                               \
	__compiler_barrier();                                   \
	if ((head) == __prev) {                                 \
	        (head)->next = (queue_entry_t) (elt);           \
	}                                                       \
	else {                                                  \
	        ((type)(void *)__prev)->field.next =            \
	                (queue_entry_t)(elt);                   \
	}                                                       \
	(head)->prev = (queue_entry_t) elt;                     \
MACRO_END

/*
 *	Macro:		queue_enter_first
 *	Function:
 *		Insert a new element at the head of the queue.
 *	Header:
 *		void queue_enter_first(q, elt, type, field)
 *			queue_t q;
 *			<type> elt;
 *			<type> is what's in our queue
 *			<field> is the chain field in (*<type>)
 *	Note:
 *		This should only be used with Method 2 queue iteration (element chains)
 */
#define queue_enter_first(head, elt, type, field)               \
MACRO_BEGIN                                                     \
	queue_entry_t __next;                                   \
                                                                \
	__next = (head)->next;                                  \
	if ((head) == __next) {                                 \
	        (head)->prev = (queue_entry_t) (elt);           \
	}                                                       \
	else {                                                  \
	        ((type)(void *)__next)->field.prev =            \
	                (queue_entry_t)(elt);                   \
	}                                                       \
	(elt)->field.next = __next;                             \
	(elt)->field.prev = head;                               \
	(head)->next = (queue_entry_t) elt;                     \
MACRO_END

/*
 *	Macro:		queue_insert_before
 *	Function:
 *		Insert a new element before a given element.
 *	Header:
 *		void queue_insert_before(q, elt, cur, type, field)
 *			queue_t q;
 *			<type> elt;
 *			<type> cur;
 *			<type> is what's in our queue
 *			<field> is the chain field in (*<type>)
 *	Note:
 *		This should only be used with Method 2 queue iteration (element chains)
 */
#define queue_insert_before(head, elt, cur, type, field)                \
MACRO_BEGIN                                                             \
	queue_entry_t __prev;                                           \
                                                                        \
	if ((head) == (queue_entry_t)(cur)) {                           \
	        (elt)->field.next = (head);                             \
	        if ((head)->next == (head)) {   /* only element */      \
	                (elt)->field.prev = (head);                     \
	                (head)->next = (queue_entry_t)(elt);            \
	        } else {                        /* last element */      \
	                __prev = (elt)->field.prev = (head)->prev;      \
	                ((type)(void *)__prev)->field.next =            \
	                        (queue_entry_t)(elt);                   \
	        }                                                       \
	        (head)->prev = (queue_entry_t)(elt);                    \
	} else {                                                        \
	        (elt)->field.next = (queue_entry_t)(cur);               \
	        if ((head)->next == (queue_entry_t)(cur)) {             \
	/* first element */     \
	                (elt)->field.prev = (head);                     \
	                (head)->next = (queue_entry_t)(elt);            \
	        } else {                        /* middle element */    \
	                __prev = (elt)->field.prev = (cur)->field.prev; \
	                ((type)(void *)__prev)->field.next =            \
	                        (queue_entry_t)(elt);                   \
	        }                                                       \
	        (cur)->field.prev = (queue_entry_t)(elt);               \
	}                                                               \
MACRO_END

/*
 *	Macro:		queue_insert_after
 *	Function:
 *		Insert a new element after a given element.
 *	Header:
 *		void queue_insert_after(q, elt, cur, type, field)
 *			queue_t q;
 *			<type> elt;
 *			<type> cur;
 *			<type> is what's in our queue
 *			<field> is the chain field in (*<type>)
 *	Note:
 *		This should only be used with Method 2 queue iteration (element chains)
 */
#define queue_insert_after(head, elt, cur, type, field)                 \
MACRO_BEGIN                                                             \
	queue_entry_t __next;                                           \
                                                                        \
	if ((head) == (queue_entry_t)(cur)) {                           \
	        (elt)->field.prev = (head);                             \
	        if ((head)->next == (head)) {   /* only element */      \
	                (elt)->field.next = (head);                     \
	                (head)->prev = (queue_entry_t)(elt);            \
	        } else {                        /* first element */     \
	                __next = (elt)->field.next = (head)->next;      \
	                ((type)(void *)__next)->field.prev =            \
	                        (queue_entry_t)(elt);                   \
	        }                                                       \
	        (head)->next = (queue_entry_t)(elt);                    \
	} else {                                                        \
	        (elt)->field.prev = (queue_entry_t)(cur);               \
	        if ((head)->prev == (queue_entry_t)(cur)) {             \
	/* last element */      \
	                (elt)->field.next = (head);                     \
	                (head)->prev = (queue_entry_t)(elt);            \
	        } else {                        /* middle element */    \
	                __next = (elt)->field.next = (cur)->field.next; \
	                ((type)(void *)__next)->field.prev =            \
	                        (queue_entry_t)(elt);                   \
	        }                                                       \
	        (cur)->field.next = (queue_entry_t)(elt);               \
	}                                                               \
MACRO_END

/*
 *	Macro:		queue_field [internal use only]
 *	Function:
 *		Find the queue_chain_t (or queue_t) for the
 *		given element (thing) in the given queue (head)
 *	Note:
 *		This should only be used with Method 2 queue iteration (element chains)
 */
#define queue_field(head, thing, type, field)                   \
	        (((head) == (thing)) ? (head) : &((type)(void *)(thing))->field)

/*
 *	Macro:		queue_remove
 *	Function:
 *		Remove an arbitrary item from the queue.
 *	Header:
 *		void queue_remove(q, qe, type, field)
 *			arguments as in queue_enter
 *	Note:
 *		This should only be used with Method 2 queue iteration (element chains)
 */
#define queue_remove(head, elt, type, field)                    \
MACRO_BEGIN                                                     \
	queue_entry_t	__next, __prev;                         \
                                                                \
	__next = (elt)->field.next;                             \
	__prev = (elt)->field.prev;                             \
                                                                \
	if ((head) == __next)                                   \
	        (head)->prev = __prev;                          \
	else                                                    \
	        ((type)(void *)__next)->field.prev = __prev;    \
                                                                \
	if ((head) == __prev)                                   \
	        (head)->next = __next;                          \
	else                                                    \
	        ((type)(void *)__prev)->field.next = __next;    \
                                                                \
	(elt)->field.next = NULL;                               \
	(elt)->field.prev = NULL;                               \
MACRO_END

/*
 *	Macro:		queue_remove_first
 *	Function:
 *		Remove and return the entry at the head of
 *		the queue.
 *	Header:
 *		queue_remove_first(head, entry, type, field)
 *		entry is returned by reference
 *	Note:
 *		This should only be used with Method 2 queue iteration (element chains)
 */
#define queue_remove_first(head, entry, type, field)            \
MACRO_BEGIN                                                     \
	queue_entry_t	__next;                                 \
                                                                \
	(entry) = (type)(void *) ((head)->next);                \
	__next = (entry)->field.next;                           \
                                                                \
	if ((head) == __next)                                   \
	        (head)->prev = (head);                          \
	else                                                    \
	        ((type)(void *)(__next))->field.prev = (head);  \
	(head)->next = __next;                                  \
                                                                \
	(entry)->field.next = NULL;                             \
	(entry)->field.prev = NULL;                             \
MACRO_END

/*
 *	Macro:		queue_remove_last
 *	Function:
 *		Remove and return the entry at the tail of
 *		the queue.
 *	Header:
 *		queue_remove_last(head, entry, type, field)
 *		entry is returned by reference
 *	Note:
 *		This should only be used with Method 2 queue iteration (element chains)
 */
#define queue_remove_last(head, entry, type, field)             \
MACRO_BEGIN                                                     \
	queue_entry_t	__prev;                                 \
                                                                \
	(entry) = (type)(void *) ((head)->prev);                \
	__prev = (entry)->field.prev;                           \
                                                                \
	if ((head) == __prev)                                   \
	        (head)->next = (head);                          \
	else                                                    \
	        ((type)(void *)(__prev))->field.next = (head);  \
	(head)->prev = __prev;                                  \
                                                                \
	(entry)->field.next = NULL;                             \
	(entry)->field.prev = NULL;                             \
MACRO_END

/*
 *	Macro:		queue_assign
 *	Note:
 *		This should only be used with Method 2 queue iteration (element chains)
 */
#define queue_assign(to, from, type, field)                     \
MACRO_BEGIN                                                     \
	((type)(void *)((from)->prev))->field.next = (to);      \
	((type)(void *)((from)->next))->field.prev = (to);      \
	*to = *from;                                            \
MACRO_END

/*
 *	Macro:		queue_new_head
 *	Function:
 *		rebase old queue to new queue head
 *	Header:
 *		queue_new_head(old, new, type, field)
 *			queue_t old;
 *			queue_t new;
 *			<type> is what's in our queue
 *                      <field> is the chain field in (*<type>)
 *	Note:
 *		This should only be used with Method 2 queue iteration (element chains)
 */
#define queue_new_head(old, new, type, field)                   \
MACRO_BEGIN                                                     \
	if (!queue_empty(old)) {                                \
	        *(new) = *(old);                                \
	        ((type)(void *)((new)->next))->field.prev =     \
	                (new);                                  \
	        ((type)(void *)((new)->prev))->field.next =     \
	                (new);                                  \
	} else {                                                \
	        queue_init(new);                                \
	}                                                       \
MACRO_END

/*
 *	Macro:		queue_iterate
 *	Function:
 *		iterate over each item in the queue.
 *		Generates a 'for' loop, setting elt to
 *		each item in turn (by reference).
 *	Header:
 *		queue_iterate(q, elt, type, field)
 *			queue_t q;
 *			<type> elt;
 *			<type> is what's in our queue
 *			<field> is the chain field in (*<type>)
 *	Note:
 *		This should only be used with Method 2 queue iteration (element chains)
 */
#define queue_iterate(head, elt, type, field)                   \
	for ((elt) = (type)(void *) queue_first(head);          \
	     !queue_end((head), (queue_entry_t)(elt));          \
	     (elt) = (type)(void *) queue_next(&(elt)->field))


__END_DECLS

#endif  /* _KERN_QUEUE_H_ */

/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

#ifndef _KERN_PRIORITY_QUEUE_H_
#define _KERN_PRIORITY_QUEUE_H_

#include <mach/mach_types.h>
#include <kern/macro_help.h>
#include <kern/assert.h>

#include <sys/cdefs.h>

__BEGIN_DECLS

/*
 * A generic priorty ordered queue implementation based on pairing heaps.
 *
 * Reference Papers:
 * - A Back-to-Basics Empirical Study of Priority Queues (https://arxiv.org/abs/1403.0252)
 * - The Pairing Heap: A New Form of Self-Adjusting Heap (https://www.cs.cmu.edu/~sleator/papers/pairing-heaps.pdf)
 *
 * The XNU implementation is a basic version of the pairing heap. It allows for O(1) insertion and amortized O(log n)
 * deletion. It is not a stable data structure since adding stability would need more pointers and hence more memory.
 *
 * The implementation supports two types of key storage:
 *
 *     Type 1: PRIORITY_QUEUE_GENERIC_KEY
 *
 *         This flag is useful when the priorities are either larger than 8-bits or the node comparision needs
 *         extra information other than the priority. The nodes do not store the priorities themselves and on
 *         comparision, callout to the comparator (of type priority_queue_compare_fn_t) provided as part of
 *         initialization.
 *
 *         Sample Initialization:
 *
 *         {
 *             static struct priority_queue pq;
 *             priority_queue_init(pq, PRIORITY_QUEUE_MAX_HEAP | PRIORITY_QUEUE_GENERIC_KEY);
 *         }
 *
 *         For this type, all insertions, priority_increase, priority_decrease must pass PRIORITY_QUEUE_KEY_NONE
 *         as the priority key field.
 *
 *     Type 2: PRIORITY_QUEUE_BUILTIN_KEY
 *
 *         This type is useful when the priorities need to be stored within the data structure itself.
 *         Each node in the priority queue maintains a 8-bit priority key.
 *
 *         Sample Initialization:
 *         {
 *             static struct priority_queue pq;
 *             priority_queue_init(pq, PRIORITY_QUEUE_MAX_HEAP | PRIORITY_QUEUE_BUILTIN_KEY);
 *         }
 *
 *
 * Min / Max Heap:
 *
 *     The semantics of Min/Max heap are not used by the implementation, it assumes that the comparison block
 *     that is passed to the insertion / removal / ... macros provides the right ordering.
 *
 *     However for human readability purposes, whether this heap is a MIN or MAX heap is passed
 *     at initialization time, and will influence whether accessors like priority_queue_min
 *     or priority_queue_max can be used.
 *
 *
 * Element Linkage:
 *
 *         Both types use a common queue head and linkage pattern.
 *         The head of a priority queue is declared as:
 *
 *              struct priority_queue pq_head;
 *
 *         Elements in this queue are linked together using struct priority_queue_entry objects embedded within a structure:
 *              struct some_data {
 *                      int field1;
 *                      int field2;
 *                      ...
 *                      struct priority_queue_entry link;
 *                      ...
 *                      int last_field;
 *              };
 *         struct some_data is referred to as the queue "element"
 *
 *         This method uses the next, prev and child pointers of the struct priority_queue_entry linkage object embedded in a
 *         queue element to point to other elements in the queue. The head of the priority queue (the priority_queue
 *         object) will point to the root of the pairing heap (NULL if heap is empty). This method allows multiple chains
 *         through a given object, by embedding multiple priority_queue_entry objects in the structure, while simultaneously
 *         providing fast removal and insertion into the heap using only priority_queue_entry object pointers.
 */


/*
 * Priority keys maintained by the data structure.
 * Since the priority is packed in the node itself, it restricts keys to be 8-bits only.
 */
#define PRIORITY_QUEUE_KEY_NONE             0
typedef uint8_t priority_queue_key_t;

/*
 * Flags passed to priority_queue_init()
 *
 * One key type must be picked (default is BUILTIN_KEY)
 * Min or Max heap must be picked (default is MAX_HEAP)
 */
typedef enum priority_queue_flags {
	PRIORITY_QUEUE_BUILTIN_KEY    = 0x0,
	PRIORITY_QUEUE_GENERIC_KEY    = 0x1,
	PRIORITY_QUEUE_MAX_HEAP       = 0x0,
	PRIORITY_QUEUE_MIN_HEAP       = 0x2,
#define PRIORITY_QUEUE_BUILTIN_MAX_HEAP (PRIORITY_QUEUE_MAX_HEAP | PRIORITY_QUEUE_BUILTIN_KEY)
} priority_queue_flags_t;

#ifdef __LP64__

/*
 * For 64-bit platforms, pack the priority key into the child pointer
 * The packing/unpacking is done using a compiler trick to sign extend long.
 * This avoids additional NULL checks which are needed in typical packing
 * implementation. The idea is to define the packed location as a long and
 * for unpacking simply cast it to a full pointer which sign extends it.
 */
#define PRIORITY_QUEUE_ENTRY_CHILD_BITS     56
#define PRIORITY_QUEUE_ENTRY_KEY_BITS       8

typedef struct priority_queue_entry {
	struct priority_queue_entry         *next;
	struct priority_queue_entry         *prev;
	long                                key: PRIORITY_QUEUE_ENTRY_KEY_BITS;
	long                                child: PRIORITY_QUEUE_ENTRY_CHILD_BITS;
} *priority_queue_entry_t;

#else /* __LP64__ */

/*
 * For 32-bit platforms, use an extra field to store the key since child pointer packing
 * is not an option. The child is maintained as a long to use the same packing/unpacking
 * routines that work for 64-bit platforms.
 */
typedef struct priority_queue_entry {
	struct priority_queue_entry         *next;
	struct priority_queue_entry         *prev;
	long                                child;
	priority_queue_key_t                key;
} *priority_queue_entry_t;

#endif /* __LP64__ */

/*
 * Comparator block prototype
 * Args:
 *      - elements to compare
 * Return:
 * comparision result to indicate relative ordering of elements according to the heap type
 */
typedef int (^priority_queue_compare_fn_t)(struct priority_queue_entry *e1,
    struct priority_queue_entry *e2);

/*
 * Standard comparision routines for max and min heap.
 * Must be used with PRIORITY_QUEUE_BUILTIN_KEY only.
 */
static inline int
priority_queue_element_builtin_key_compare(priority_queue_entry_t e1, priority_queue_entry_t e2)
{
	return (int)e2->key - (int)e1->key;
}

#define priority_heap_make_comparator(name1, name2, type, field, ...) \
	(^int(priority_queue_entry_t __e1, priority_queue_entry_t __e2){                                        \
	    type *name1 = pqe_element_fast(__e1, type, field);                                                  \
	    type *name2 = pqe_element_fast(__e2, type, field);                                                  \
	    __VA_ARGS__;                                                                                        \
	})

#define PRIORITY_QUEUE_SCHED_PRI_MAX_HEAP_COMPARE                                                               \
	(^int(priority_queue_entry_t e1, priority_queue_entry_t e2){                                            \
	    return -priority_queue_element_builtin_key_compare(e1, e2);                                         \
	})

#define PRIORITY_QUEUE_SCHED_PRI_MIN_HEAP_COMPARE                                                               \
	(^int(priority_queue_entry_t e1, priority_queue_entry_t e2){                                            \
	    return priority_queue_element_builtin_key_compare(e1, e2);                                          \
	})

/*
 * Helper routines for packing/unpacking the child pointer in heap nodes.
 * On 64-bit platforms, these routines rely on the fact that the sign extension
 * for the lower 56-bits of a kernel pointer results in the real pointer. The trick
 * works for NULL pointers as well.
 * */
#define pqueue_entry_pack_child(qe, child_ptr)      ((qe)->child = (long)(child_ptr))
#define pqueue_entry_unpack_child(qe)               ((struct priority_queue_entry *)((qe)->child))

/*
 * Priority queue head structure.
 * Stores the comparision function using pointer packing. The remaining bit is used
 * for type of the queue.
 */
struct priority_queue {
/*
 * we pack priority_queue_flags_t in the least significant two bits
 * of the root pointer.
 */
#define PRIORITY_QUEUE_ROOT_FLAGS_MASK    (3ul)
#define PRIORITY_QUEUE_ROOT_POINTER_MASK  (~PRIORITY_QUEUE_ROOT_FLAGS_MASK)
	unsigned long                       pq_root_packed;
};

/*
 *      Macro:          pqe_element_fast
 *      Function:
 *              Convert a priority_queue_entry_t to a queue element pointer.
 *              Get a pointer to the user-defined element containing
 *              a given priority_queue_entry_t
 *
 *              The fast variant assumes that `qe` is not NULL
 *      Header:
 *              pqe_element_fast(qe, type, field)
 *                      <priority_queue_entry_t> qe
 *                      <type> type of element in priority queue
 *                      <field> chain field in (*<type>)
 *      Returns:
 *              <type *> containing qe
 */
#define pqe_element_fast(qe, type, field)  __container_of(qe, type, field)

/*
 *      Macro:          pqe_element
 *      Function:
 *              Convert a priority_queue_entry_t to a queue element pointer.
 *              Get a pointer to the user-defined element containing
 *              a given priority_queue_entry_t
 *
 *              The non fast variant handles NULL `qe`
 *      Header:
 *              pqe_element(qe, type, field)
 *                      <priority_queue_entry_t> qe
 *                      <type> type of element in priority queue
 *                      <field> chain field in (*<type>)
 *      Returns:
 *              <type *> containing qe
 */
#define pqe_element(qe, type, field)  ({                                                                        \
	priority_queue_entry_t _tmp_entry = (qe);                                                               \
	_tmp_entry ? pqe_element_fast(_tmp_entry, type, field) : ((type *)NULL);                                \
})

#define pqueue_has_generic_keys(p) \
	(((p)->pq_root_packed & PRIORITY_QUEUE_GENERIC_KEY) != 0)

#define pqueue_has_builtin_keys(p) \
	(((p)->pq_root_packed & PRIORITY_QUEUE_GENERIC_KEY) == 0)

#define pqueue_is_min_heap(p) \
	(((p)->pq_root_packed & PRIORITY_QUEUE_MIN_HEAP) != 0)

#define pqueue_is_max_heap(p) \
	(((p)->pq_root_packed & PRIORITY_QUEUE_MIN_HEAP) == 0)

/*
 *      Macro:          pqueue_pack_root
 *      Function:
 *              Pack the root pointer of the head.
 *      Header:
 *              pqueue_pack_root(q, root_ptr)
 *                      <struct priority_queue *> q
 *                      <priority_queue_entry_t> root_ptr
 */
#define pqueue_pack_root(q, root_ptr)                                                                           \
MACRO_BEGIN                                                                                                     \
	uintptr_t __flags = (q)->pq_root_packed & PRIORITY_QUEUE_ROOT_FLAGS_MASK;                               \
	(q)->pq_root_packed = (uintptr_t)(root_ptr) | __flags;                                                  \
MACRO_END

/*
 *      Macro:          pqueue_unpack_root
 *      Function:
 *              Unpack the root pointer from the head of the priority queue.
 *      Header:
 *              pqueue_unpack_root(q)
 *                      <struct priority_queue *> q
 *      Returns:
 *              <priority_queue_entry_t>
 */
#define pqueue_unpack_root(q) \
	((priority_queue_entry_t)((q)->pq_root_packed & PRIORITY_QUEUE_ROOT_POINTER_MASK))

/*
 *      Macro:          pqueue_list_remove
 *      Function:
 *              Helper routine to remove an element from the list at its level
 *      Header:
 *              pqueue_list_remove(elt)
 *                      <priority_queue_entry_t> elt
 *      Returns:
 *              None
 */
static inline void
pqueue_list_remove(priority_queue_entry_t elt)
{
	assert(elt->prev != NULL);
	/* Check if elt is head of list at its level;        */
	/* If yes, make the next node the head at that level */
	/* Else, remove elt from the list at that level      */
	if (pqueue_entry_unpack_child(elt->prev) == elt) {
		pqueue_entry_pack_child(elt->prev, elt->next);
	} else {
		elt->prev->next = elt->next;
	}
	/* Update prev for next element in list */
	if (elt->next != NULL) {
		elt->next->prev = elt->prev;
	}
}

/*
 *      Macro:          pqueue_merge
 *      Function:
 *              Helper routine to merge two subtrees of the heap to form a single tree and
 *              maintain the parent > child invariant. If the two keys are equal, the current
 *              implementation makes the first subtree the parent and the second one the child.
 *      Header:
 *              pqueue_merge(subtree_a, subtree_b, cmp_fn)
 *                      <priority_queue_entry_t> subtree_a
 *                      <priority_queue_entry_t> subtree_b
 *                      <cmp_fn> comparator function
 *      Returns:
 *              <priority_queue_entry_t> pointing to root of the merged tree
 */
static inline priority_queue_entry_t
pqueue_merge(priority_queue_entry_t subtree_a, priority_queue_entry_t subtree_b,
    priority_queue_compare_fn_t cmp_fn)
{
	priority_queue_entry_t merge_result = NULL;
	if (subtree_a == NULL) {
		merge_result = subtree_b;
	} else if (subtree_b == NULL || (subtree_a == subtree_b)) {
		merge_result = subtree_a;
	} else {
		priority_queue_entry_t parent = subtree_a;
		priority_queue_entry_t child = subtree_b;
		if (cmp_fn(subtree_a, subtree_b) < 0) {
			parent = subtree_b;
			child = subtree_a;
		}
		/* Insert the child as the first element in the parent's child list */
		child->next = pqueue_entry_unpack_child(parent);
		child->prev = parent;
		if (pqueue_entry_unpack_child(parent) != NULL) {
			pqueue_entry_unpack_child(parent)->prev = child;
		}
		/* Create the parent child relationship */
		pqueue_entry_pack_child(parent, child);
		parent->next = NULL;
		parent->prev = NULL;
		merge_result = parent;
	}
	return merge_result;
}

/*
 *      Macro:          pqueue_pair_meld
 *      Function:
 *              Helper routine to splitwise pair a set of subtrees on a list at a given level and then
 *              meld them together to form a new tree while maintaining the invariant parent > child.
 *
 *              The caller must check the element is non NULL.
 *
 *      Header:
 *              pqueue_pair_meld(elt, cmp_fn)
 *                      <priority_queue_entry_t> elt
 *                      <cmp_fn> comparator function
 *      Returns:
 *              <priority_queue_entry_t> pointing to root of the melded tree
 */
priority_queue_entry_t
pqueue_pair_meld(priority_queue_entry_t e, priority_queue_compare_fn_t cmp_fn);

/*
 *      Macro:          pqueue_update_key
 *      Function:
 *              Helper routine to update the key for a node in the heap. Note that the priority keys are only
 *              maintained for the PRIORITY_QUEUE_BUILTIN_KEY type of priority queue. For PRIORITY_QUEUE_GENERIC_KEY,
 *              this routine does nothing.
 *      Header:
 *              pqueue_update_key(que, elt, new_key)
 *                      <struct priority_queue *> que
 *                      <priority_queue_entry_t> elt
 *                      <priority_queue_key_t> new_key
 *      Returns:
 *              None
 */
static inline void
pqueue_update_key(struct priority_queue *que, priority_queue_entry_t elt,
    priority_queue_key_t new_key)
{
	if (pqueue_has_builtin_keys(que)) {
		assert(new_key <= UINT8_MAX);
		elt->key = new_key;
	} else {
		assert(new_key == PRIORITY_QUEUE_KEY_NONE);
	}
}

/*
 *      Macro:          pqueue_remove_root
 *      Function:
 *              Helper routine to remove the root element in a priority queue.
 *      Header:
 *              pqueue_remove_root(que, cmp_fn)
 *                      <struct priority_queue *> que
 *                      <priority_queue_entry_t> old_root
 *                      <cmp_fn> comparator function
 *      Returns:
 *              old_root
 */
static inline priority_queue_entry_t
pqueue_remove_root(struct priority_queue *que, priority_queue_entry_t old_root,
    priority_queue_compare_fn_t cmp_fn)
{
	priority_queue_entry_t new_root = pqueue_entry_unpack_child(old_root);
	if (new_root) {
		new_root = pqueue_pair_meld(new_root, cmp_fn);
	}
	pqueue_pack_root(que, new_root);
	return old_root;
}

/*
 *      Macro:          pqueue_remove_non_root
 *      Function:
 *              Helper routine to remove a non root element in a priority queue.
 *      Header:
 *              pqueue_remove_non_root(que, cmp_fn)
 *                      <struct priority_queue *> que
 *                      <priority_queue_entry_t> elt
 *                      <cmp_fn> comparator function
 *      Returns:
 *              elt
 */
static inline priority_queue_entry_t
pqueue_remove_non_root(struct priority_queue *que, priority_queue_entry_t elt,
    priority_queue_compare_fn_t cmp_fn)
{
	priority_queue_entry_t child, new_root;

	/* To remove a non-root element with children levels, */
	/* - Remove element from its current level iist */
	/* - Pairwise split all the elements in the child level list */
	/* - Meld all these splits (right-to-left) to form new subtree */
	/* - Merge the root subtree with the newly formed subtree */
	pqueue_list_remove(elt);

	child = pqueue_entry_unpack_child(elt);
	if (child) {
		child = pqueue_pair_meld(child, cmp_fn);
		new_root = pqueue_merge(pqueue_unpack_root(que), child, cmp_fn);
		pqueue_pack_root(que, new_root);
	}

	return elt;
}

/*
 *      Macro:          pqueue_destroy
 *      Function:
 *              Destroy a priority queue safely. This routine accepts a callback
 *              to handle any cleanup for elements in the priority queue. The queue does
 *              not maintain its invariants while getting destroyed. The priority queue and
 *              the linkage nodes need to be re-initialized before re-using them.
 *
 *              Note: the offset is the offset to the linkage inside the elements
 *              That are linked inside the priority heap, because pqueue_destroy
 *              can't use pqe_element.
 *      Header:
 *              pqueue_destroy(q, offset, callback)
 *                      <struct priority_queue *> q
 *                      <size_t> offset
 *                      <callback> callback for each element
 *
 *      Returns:
 *              None
 */
void
    pqueue_destroy(struct priority_queue *q, size_t offset,
    void (^callback)(void *e));

/*
 * Priority Queue functionality routines
 */

/*
 *      Macro:          priority_queue_empty
 *      Function:
 *              Tests whether a priority queue is empty.
 *      Header:
 *              boolean_t priority_queue_empty(q)
 *                      <struct priority_queue *> q
 */
#define priority_queue_empty(q)         (pqueue_unpack_root((q)) == NULL)

/*
 *      Macro:          priority_queue_entry_key
 *      Function:
 *              Returns the priority queue entry key for an element on a PRIORITY_QUEUE_BUILTIN_KEY
 *              queue. It should not be called for an element on a PRIORITY_QUEUE_GENERIC_KEY queue.
 *      Header:
 *              priority_queue_key_t priority_queue_entry_key(q, elt)
 *                      <struct priority_queue *> q
 *                      <struct priority_queue_entry *> elt
 */
#define priority_queue_entry_key(q, elt) ({                                                                     \
	assert(pqueue_has_builtin_keys(q));                                                                     \
	(priority_queue_key_t)((elt)->key);                                                                     \
})

/*
 *      Macro:          priority_queue_init
 *      Function:
 *              Initialze a <struct priority_queue *> by setting the flags
 *              Valid flags are:
 *              - PRIORITY_QUEUE_BUILTIN_KEY or PRIORITY_QUEUE_GENERIC_KEY
 *              - PRIORITY_QUEUE_MAX_HEAP or PRIORITY_QUEUE_MIN_HEAP
 *      Header:
 *              priority_queue_init(q, cmp_fn, queue_type)
 *                      <struct priority_queue *> q
 *                      <priority_queue_flags_t> queue_flags
 *      Returns:
 *              None
 */
#define priority_queue_init(q, flags)                                                                           \
MACRO_BEGIN                                                                                                     \
	pqueue_pack_root((q), NULL);                                                                            \
	(q)->pq_root_packed = (flags);                                                                          \
MACRO_END

/*
 *      Macro:          priority_queue_entry_init
 *      Function:
 *              Initialze a priority_queue_entry_t
 *      Header:
 *              priority_queue_entry_init(qe)
 *                      <priority_queue_entry_t> qe
 *      Returns:
 *              None
 */
#define priority_queue_entry_init(qe)                                                                           \
MACRO_BEGIN                                                                                                     \
	(qe)->next = NULL;                                                                                      \
	(qe)->prev = NULL;                                                                                      \
	pqueue_entry_pack_child((qe), NULL);                                                                    \
	(qe)->key = PRIORITY_QUEUE_KEY_NONE;                                                                    \
MACRO_END

/*
 *      Macro:          priority_queue_insert
 *      Function:
 *              Insert an element into the priority queue
 *      Header:
 *              priority_queue_insert(que, elt, new_key, cmp_fn)
 *                      <struct priority_queue *> que
 *                      <priority_queue_entry_t> elt
 *                      <priority_queue_key_t> new_key
 *                      <cmp_fn> comparator function
 *      Returns:
 *              Whether the inserted element became the new root
 */
static inline boolean_t
priority_queue_insert(struct priority_queue *que, priority_queue_entry_t elt,
    priority_queue_key_t new_key, priority_queue_compare_fn_t cmp_fn)
{
	priority_queue_entry_t new_root;

	pqueue_update_key(que, elt, new_key);
	new_root = pqueue_merge(pqueue_unpack_root(que), elt, cmp_fn);
	pqueue_pack_root(que, new_root);
	return new_root == elt;
}

/*
 *      Macro:          priority_queue_remove
 *      Function:
 *              Removes an element from the priority queue
 *      Header:
 *              priority_queue_remove(que, elt, cmp_fn)
 *                      <struct priority_queue *> que
 *                      <priority_queue_entry_t> elt
 *                      <cmp_fn> comparator function
 *      Returns:
 *              Whether the removed element was the root
 */
static inline boolean_t
priority_queue_remove(struct priority_queue *que, priority_queue_entry_t elt,
    priority_queue_compare_fn_t cmp_fn)
{
	if (elt == pqueue_unpack_root(que)) {
		pqueue_remove_root(que, elt, cmp_fn);
		priority_queue_entry_init(elt);
		return TRUE;
	} else {
		pqueue_remove_non_root(que, elt, cmp_fn);
		priority_queue_entry_init(elt);
		return FALSE;
	}
}

/*
 *      Macro:          priority_queue_entry_decrease
 *
 *      WARNING:
 *              This function is badly named for a min-heap, as it means the element
 *              moves toward the root, which happens if the key value became smaller.
 *
 *      Function:
 *              Decrease the priority of an element in the priority queue. Since the heap invariant is to always
 *              have the maximum element at the root, the most efficient way to implement this is to remove
 *              the element and re-insert it into the heap.
 *
 *              For PRIORITY_QUEUE_BUILTIN_KEY, the new_key is passed into this routine since the priority is
 *              maintained by the data structure. For PRIORITY_QUEUE_GENERIC_KEY, the caller must update the priority
 *              in the element and then call this routine. For the new_key field, it must pass PRIORITY_QUEUE_KEY_NONE.
 *      Header:
 *              priority_queue_entry_decrease(que, elt, new_key, cmp_fn)
 *                      <struct priority_queue *> que
 *                      <priority_queue_entry_t> elt
 *                      <priority_queue_key_t> new_key
 *                      <cmp_fn> comparator function
 *      Returns:
 *              Whether the update caused the root or its key to change.
 */
static inline boolean_t
priority_queue_entry_decrease(struct priority_queue *que, priority_queue_entry_t elt,
    priority_queue_key_t new_key, priority_queue_compare_fn_t cmp_fn)
{
	boolean_t was_root = priority_queue_remove(que, elt, cmp_fn);
	/* Insert it back in the heap; insertion also causes the priority update in the element */
	priority_queue_insert(que, elt, new_key, cmp_fn);
	return was_root;
}

/*
 *      Macro:          priority_queue_entry_increase
 *
 *      WARNING:
 *              This function is badly named for a min-heap, as it means the element
 *              moves away from the root, which happens if the key value became larger.
 *
 *      Function:
 *              Increase the priority of an element in the priority queue. If the root is being increased, no change
 *              to the data structure is needed. For elements at any other level, unhook it from that level and
 *              re-merge it.
 *
 *              For PRIORITY_QUEUE_BUILTIN_KEY, the new_key is passed into this routine since the priority is
 *              maintained by the data structure. For PRIORITY_QUEUE_GENERIC_KEY, the caller must update the priority
 *              in the element and then call this routine. For the new_key field, it must pass PRIORITY_QUEUE_KEY_NONE.
 *      Header:
 *              priority_queue_entry_increase(que, elt, new_key, cmp_fn)
 *                      <struct priority_queue *> que
 *                      <priority_queue_entry_t> elt
 *                      <priority_queue_key_t> new_key
 *                      <cmp_fn> comparator function
 *      Returns:
 *              Whether the update caused the root or its key to change.
 */
static inline boolean_t
priority_queue_entry_increase(struct priority_queue *que, priority_queue_entry_t elt,
    priority_queue_key_t new_key, priority_queue_compare_fn_t cmp_fn)
{
	if (elt == pqueue_unpack_root(que)) {
		pqueue_update_key(que, elt, new_key);
		return TRUE;
	}

	/* Remove the element from its current level list */
	pqueue_list_remove(elt);
	/* Re-insert the element into the heap with a merge */
	return priority_queue_insert(que, elt, new_key, cmp_fn);
}

/*
 * Min/Max nodes lookup and removal routines
 * Since the data structure is unaware of the type of heap being constructed, it provides both the min
 * and max variants of the lookup and removal routines. Both variants do the exact same operation and
 * it is up to the callers to call the right variant which makes semantic sense for the type of heap.
 */

/*
 *      Macro:          priority_queue_max
 *      Function:
 *              Lookup the max element in a priority queue. It simply returns the root of the
 *              priority queue.
 *      Header:
 *              priority_queue_max(q, type, field)
 *                      <struct priority_queue *> q
 *                      <type> type of element in priority queue
 *                      <field> chain field in (*<type>)
 *      Returns:
 *              <type *> max element
 */
#define priority_queue_max(q, type, field) ({                                                                   \
	assert(pqueue_is_max_heap(q));                                                                          \
	pqe_element(pqueue_unpack_root(q), type, field);                                                        \
})

/*
 *      Macro:          priority_queue_min
 *      Function:
 *              Lookup the min element in a priority queue. It simply returns the root of the
 *              priority queue.
 *      Header:
 *              priority_queue_min(q, type, field)
 *                      <struct priority_queue *> q
 *                      <type> type of element in priority queue
 *                      <field> chain field in (*<type>)
 *      Returns:
 *              <type *> min element
 */
#define priority_queue_min(q, type, field) ({                                                                   \
	assert(pqueue_is_min_heap(q));                                                                          \
	pqe_element(pqueue_unpack_root(q), type, field);                                                        \
})

/*
 *      Macro:          priority_queue_max_key
 *      Function:
 *              Lookup the max key in a priority queue.
 *      Header:
 *              priority_queue_max_key(q)
 *                      <struct priority_queue *> q
 *      Returns:
 *              <type *> max key
 */
#define priority_queue_max_key(q) ({                                                                            \
	assert(pqueue_is_max_heap(q));                                                                          \
	priority_queue_entry_key(q, pqueue_unpack_root(q));                                                     \
})

/*
 *      Macro:          priority_queue_min_key
 *      Function:
 *              Lookup the min key in a priority queue.
 *      Header:
 *              priority_queue_min_key(q)
 *                      <struct priority_queue *> q
 *      Returns:
 *              <type *> min key
 */
#define priority_queue_min_key(q) ({                                                                            \
	assert(pqueue_is_min_heap(q));                                                                          \
	priority_queue_entry_key(pqueue_unpack_root(q));                                                        \
})

/*
 *      Macro:          priority_queue_remove_max
 *      Function:
 *              Remove the max element in a priority queue.
 *              Uses the priority_queue_remove() routine to actually do the removal.
 *      Header:
 *              priority_queue_remove_max(q, type, field)
 *                      <struct priority_queue *> q
 *                      <type> type of element in priority queue
 *                      <field> chain field in (*<type>)
 *      Returns:
 *              <type *> max element
 */
#define priority_queue_remove_max(q, type, field, cmp_fn) ({                                                    \
	assert(pqueue_is_max_heap(q));                                                                          \
	pqe_element(pqueue_remove_root(q, pqueue_unpack_root(q), cmp_fn), type, field);                         \
})

/*
 *      Macro:          priority_queue_remove_min
 *      Function:
 *              Remove the min element in a priority queue.
 *              Uses the priority_queue_remove() routine to actually do the removal.
 *      Header:
 *              priority_queue_remove_min(q, type, field)
 *                      <struct priority_queue *> q
 *                      <type> type of element in priority queue
 *                      <field> chain field in (*<type>)
 *      Returns:
 *              <type *> min element
 */
#define priority_queue_remove_min(q, type, field, cmp_fn) ({                                                    \
	assert(pqueue_is_min_heap(q));                                                                         \
	pqe_element(pqueue_remove_root(q, pqueue_unpack_root(q), cmp_fn), type, field);                         \
})

/*
 *      Macro:          priority_queue_destroy
 *      Function:
 *              Destroy a priority queue safely. This routine accepts a callback
 *              to handle any cleanup for elements in the priority queue. The queue does
 *              not maintain its invariants while getting destroyed. The priority queue and
 *              the linkage nodes need to be re-initialized before re-using them.
 *      Header:
 *              priority_queue_destroy(q, type, field, callback)
 *                      <struct priority_queue *> q
 *                      <type> type of element in priority queue
 *                      <field> chain field in (*<type>)
 *                      <callback> callback for each element
 *
 *      Returns:
 *              None
 */
#define priority_queue_destroy(q, type, field, callback, ...) \
	pqueue_destroy(q, offsetof(type, field), callback, ##__VA_ARGS__)

__END_DECLS

#endif /* _KERN_PRIORITY_QUEUE_H_ */

/*
 * Copyright (c) 2009-2019 Apple Inc. All rights reserved.
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

/*	$NetBSD: tree.h,v 1.13 2006/08/27 22:32:38 christos Exp $	*/
/*	$OpenBSD: tree.h,v 1.7 2002/10/17 21:51:54 art Exp $	*/
/*
 * Copyright 2002 Niels Provos <provos@citi.umich.edu>
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LIBKERN_TREE_H_
#define _LIBKERN_TREE_H_

/*
 * This file defines data structures for different types of trees:
 * splay trees and red-black trees.
 *
 * A splay tree is a self-organizing data structure.  Every operation
 * on the tree causes a splay to happen.  The splay moves the requested
 * node to the root of the tree and partly rebalances it.
 *
 * This has the benefit that request locality causes faster lookups as
 * the requested nodes move to the top of the tree.  On the other hand,
 * every lookup causes memory writes.
 *
 * The Balance Theorem bounds the total access time for m operations
 * and n inserts on an initially empty tree as O((m + n)lg n).  The
 * amortized cost for a sequence of m accesses to a splay tree is O(lg n);
 *
 * A red-black tree is a binary search tree with the node color as an
 * extra attribute.  It fulfills a set of conditions:
 *	- every search path from the root to a leaf consists of the
 *	  same number of black nodes,
 *	- each red node (except for the root) has a black parent,
 *	- each leaf node is black.
 *
 * Every operation on a red-black tree is bounded as O(lg n).
 * The maximum height of a red-black tree is 2lg (n+1).
 */

#define SPLAY_HEAD(name, type)                                          \
struct name {                                                           \
	struct type *sph_root; /* root of the tree */                   \
}

#define SPLAY_INITIALIZER(root)                                         \
	{ NULL }

#define SPLAY_INIT(root) do {                                           \
	(root)->sph_root = NULL;                                        \
} while ( /*CONSTCOND*/ 0)

#define SPLAY_ENTRY(type)                                               \
struct {                                                                \
	struct type *spe_left; /* left element */                       \
	struct type *spe_right; /* right element */                     \
}

#define SPLAY_LEFT(elm, field)          (elm)->field.spe_left
#define SPLAY_RIGHT(elm, field)         (elm)->field.spe_right
#define SPLAY_ROOT(head)                (head)->sph_root
#define SPLAY_EMPTY(head)               (SPLAY_ROOT(head) == NULL)

/* SPLAY_ROTATE_{LEFT,RIGHT} expect that tmp hold SPLAY_{RIGHT,LEFT} */
#define SPLAY_ROTATE_RIGHT(head, tmp, field) do {                       \
	SPLAY_LEFT((head)->sph_root, field) = SPLAY_RIGHT(tmp, field);  \
	SPLAY_RIGHT(tmp, field) = (head)->sph_root;                     \
	(head)->sph_root = tmp;                                         \
} while ( /*CONSTCOND*/ 0)

#define SPLAY_ROTATE_LEFT(head, tmp, field) do {                        \
	SPLAY_RIGHT((head)->sph_root, field) = SPLAY_LEFT(tmp, field);  \
	SPLAY_LEFT(tmp, field) = (head)->sph_root;                      \
	(head)->sph_root = tmp;                                         \
} while ( /*CONSTCOND*/ 0)

#define SPLAY_LINKLEFT(head, tmp, field) do {                           \
	SPLAY_LEFT(tmp, field) = (head)->sph_root;                      \
	tmp = (head)->sph_root;                                         \
	(head)->sph_root = SPLAY_LEFT((head)->sph_root, field);         \
} while ( /*CONSTCOND*/ 0)

#define SPLAY_LINKRIGHT(head, tmp, field) do {                          \
	SPLAY_RIGHT(tmp, field) = (head)->sph_root;                     \
	tmp = (head)->sph_root;                                         \
	(head)->sph_root = SPLAY_RIGHT((head)->sph_root, field);        \
} while ( /*CONSTCOND*/ 0)

#define SPLAY_ASSEMBLE(head, node, left, right, field) do {             \
	SPLAY_RIGHT(left, field) = SPLAY_LEFT((head)->sph_root, field); \
	SPLAY_LEFT(right, field) = SPLAY_RIGHT((head)->sph_root, field);\
	SPLAY_LEFT((head)->sph_root, field) = SPLAY_RIGHT(node, field); \
	SPLAY_RIGHT((head)->sph_root, field) = SPLAY_LEFT(node, field); \
} while ( /*CONSTCOND*/ 0)

/* Generates prototypes and inline functions */

#define SPLAY_PROTOTYPE(name, type, field, cmp)                         \
void name##_SPLAY(struct name *, struct type *);                        \
void name##_SPLAY_MINMAX(struct name *, int);                           \
struct type *name##_SPLAY_INSERT(struct name *, struct type *);         \
struct type *name##_SPLAY_REMOVE(struct name *, struct type *);         \
                                                                        \
/* Finds the node with the same key as elm */                           \
static __inline struct type *                                           \
name##_SPLAY_FIND(struct name *head, struct type *elm)                  \
{                                                                       \
	if (SPLAY_EMPTY(head))                                          \
	        return(NULL);                                           \
	name##_SPLAY(head, elm);                                        \
	if ((cmp)(elm, (head)->sph_root) == 0)                          \
	        return (head->sph_root);                                \
	return (NULL);                                                  \
}                                                                       \
                                                                        \
static __inline struct type *                                           \
name##_SPLAY_NEXT(struct name *head, struct type *elm)                  \
{                                                                       \
	name##_SPLAY(head, elm);                                        \
	if (SPLAY_RIGHT(elm, field) != NULL) {                          \
	        elm = SPLAY_RIGHT(elm, field);                          \
	        while (SPLAY_LEFT(elm, field) != NULL) {                \
	                elm = SPLAY_LEFT(elm, field);                   \
	        }                                                       \
	} else                                                          \
	        elm = NULL;                                             \
	return (elm);                                                   \
}                                                                       \
                                                                        \
static __inline struct type *                                           \
name##_SPLAY_MIN_MAX(struct name *head, int val)                        \
{                                                                       \
	name##_SPLAY_MINMAX(head, val);                                 \
	return (SPLAY_ROOT(head));                                      \
}

/* Main splay operation.
 * Moves node close to the key of elm to top
 */
#define SPLAY_GENERATE(name, type, field, cmp)                          \
struct type *                                                           \
name##_SPLAY_INSERT(struct name *head, struct type *elm)                \
{                                                                       \
    if (SPLAY_EMPTY(head)) {                                            \
	    SPLAY_LEFT(elm, field) = SPLAY_RIGHT(elm, field) = NULL;    \
    } else {                                                            \
	    int __comp;                                                 \
	    name##_SPLAY(head, elm);                                    \
	    __comp = (cmp)(elm, (head)->sph_root);                      \
	    if(__comp < 0) {                                            \
	            SPLAY_LEFT(elm, field) = SPLAY_LEFT((head)->sph_root, field);\
	            SPLAY_RIGHT(elm, field) = (head)->sph_root;         \
	            SPLAY_LEFT((head)->sph_root, field) = NULL;         \
	    } else if (__comp > 0) {                                    \
	            SPLAY_RIGHT(elm, field) = SPLAY_RIGHT((head)->sph_root, field);\
	            SPLAY_LEFT(elm, field) = (head)->sph_root;          \
	            SPLAY_RIGHT((head)->sph_root, field) = NULL;        \
	    } else                                                      \
	            return ((head)->sph_root);                          \
    }                                                                   \
    (head)->sph_root = (elm);                                           \
    return (NULL);                                                      \
}                                                                       \
                                                                        \
struct type *                                                           \
name##_SPLAY_REMOVE(struct name *head, struct type *elm)                \
{                                                                       \
	struct type *__tmp;                                             \
	if (SPLAY_EMPTY(head))                                          \
	        return (NULL);                                          \
	name##_SPLAY(head, elm);                                        \
	if ((cmp)(elm, (head)->sph_root) == 0) {                        \
	        if (SPLAY_LEFT((head)->sph_root, field) == NULL) {      \
	                (head)->sph_root = SPLAY_RIGHT((head)->sph_root, field);\
	        } else {                                                \
	                __tmp = SPLAY_RIGHT((head)->sph_root, field);   \
	                (head)->sph_root = SPLAY_LEFT((head)->sph_root, field);\
	                name##_SPLAY(head, elm);                        \
	                SPLAY_RIGHT((head)->sph_root, field) = __tmp;   \
	        }                                                       \
	        return (elm);                                           \
	}                                                               \
	return (NULL);                                                  \
}                                                                       \
                                                                        \
void                                                                    \
name##_SPLAY(struct name *head, struct type *elm)                       \
{                                                                       \
	struct type __node, *__left, *__right, *__tmp;                  \
	int __comp;                                                     \
\
	SPLAY_LEFT(&__node, field) = SPLAY_RIGHT(&__node, field) = NULL;\
	__left = __right = &__node;                                     \
\
	while ((__comp = (cmp)(elm, (head)->sph_root)) != 0) {          \
	        if (__comp < 0) {                                       \
	                __tmp = SPLAY_LEFT((head)->sph_root, field);    \
	                if (__tmp == NULL)                              \
	                        break;                                  \
	                if ((cmp)(elm, __tmp) < 0){                     \
	                        SPLAY_ROTATE_RIGHT(head, __tmp, field); \
	                        if (SPLAY_LEFT((head)->sph_root, field) == NULL)\
	                                break;                          \
	                }                                               \
	                SPLAY_LINKLEFT(head, __right, field);           \
	        } else if (__comp > 0) {                                \
	                __tmp = SPLAY_RIGHT((head)->sph_root, field);   \
	                if (__tmp == NULL)                              \
	                        break;                                  \
	                if ((cmp)(elm, __tmp) > 0){                     \
	                        SPLAY_ROTATE_LEFT(head, __tmp, field);  \
	                        if (SPLAY_RIGHT((head)->sph_root, field) == NULL)\
	                                break;                          \
	                }                                               \
	                SPLAY_LINKRIGHT(head, __left, field);           \
	        }                                                       \
	}                                                               \
	SPLAY_ASSEMBLE(head, &__node, __left, __right, field);          \
}                                                                       \
                                                                        \
/* Splay with either the minimum or the maximum element \
 * Used to find minimum or maximum element in tree. \
 */                                                                     \
void name##_SPLAY_MINMAX(struct name *head, int __comp) \
{                                                                       \
	struct type __node, *__left, *__right, *__tmp;                  \
\
	SPLAY_LEFT(&__node, field) = SPLAY_RIGHT(&__node, field) = NULL;\
	__left = __right = &__node;                                     \
\
	while (1) {                                                     \
	        if (__comp < 0) {                                       \
	                __tmp = SPLAY_LEFT((head)->sph_root, field);    \
	                if (__tmp == NULL)                              \
	                        break;                                  \
	                if (__comp < 0){                                \
	                        SPLAY_ROTATE_RIGHT(head, __tmp, field); \
	                        if (SPLAY_LEFT((head)->sph_root, field) == NULL)\
	                                break;                          \
	                }                                               \
	                SPLAY_LINKLEFT(head, __right, field);           \
	        } else if (__comp > 0) {                                \
	                __tmp = SPLAY_RIGHT((head)->sph_root, field);   \
	                if (__tmp == NULL)                              \
	                        break;                                  \
	                if (__comp > 0) {                               \
	                        SPLAY_ROTATE_LEFT(head, __tmp, field);  \
	                        if (SPLAY_RIGHT((head)->sph_root, field) == NULL)\
	                                break;                          \
	                }                                               \
	                SPLAY_LINKRIGHT(head, __left, field);           \
	        }                                                       \
	}                                                               \
	SPLAY_ASSEMBLE(head, &__node, __left, __right, field);          \
}

#define SPLAY_NEGINF    -1
#define SPLAY_INF       1

#define SPLAY_INSERT(name, x, y)        name##_SPLAY_INSERT(x, y)
#define SPLAY_REMOVE(name, x, y)        name##_SPLAY_REMOVE(x, y)
#define SPLAY_FIND(name, x, y)          name##_SPLAY_FIND(x, y)
#define SPLAY_NEXT(name, x, y)          name##_SPLAY_NEXT(x, y)
#define SPLAY_MIN(name, x)              (SPLAY_EMPTY(x) ? NULL  \
	                                : name##_SPLAY_MIN_MAX(x, SPLAY_NEGINF))
#define SPLAY_MAX(name, x)              (SPLAY_EMPTY(x) ? NULL  \
	                                : name##_SPLAY_MIN_MAX(x, SPLAY_INF))

#define SPLAY_FOREACH(x, name, head)                                    \
	for ((x) = SPLAY_MIN(name, head);                               \
	     (x) != NULL;                                               \
	     (x) = SPLAY_NEXT(name, head, x))

/* Macros that define a red-black tree */
#define RB_HEAD(name, type)                                             \
struct name {                                                           \
	struct type *rbh_root; /* root of the tree */                   \
}

#define RB_INITIALIZER(root)                                            \
	{ NULL }

#define RB_INIT(root) do {                                              \
	(root)->rbh_root = NULL;                                        \
} while ( /*CONSTCOND*/ 0)

#define RB_BLACK        0
#define RB_RED          1
#define RB_PLACEHOLDER  NULL
#define RB_ENTRY(type)                                                  \
struct {                                                                \
	struct type *rbe_left;          /* left element */              \
	struct type *rbe_right;         /* right element */             \
	struct type *rbe_parent;        /* parent element */            \
}

#define RB_COLOR_MASK                   (uintptr_t)0x1
#define RB_LEFT(elm, field)             (elm)->field.rbe_left
#define RB_RIGHT(elm, field)            (elm)->field.rbe_right
#define _RB_PARENT(elm, field)          (elm)->field.rbe_parent
#define RB_ROOT(head)                   (head)->rbh_root
#define RB_EMPTY(head)                  (RB_ROOT(head) == NULL)

#define RB_SET(name, elm, parent, field) do {                                   \
	name##_RB_SETPARENT(elm, parent);                                       \
	RB_LEFT(elm, field) = RB_RIGHT(elm, field) = NULL;              \
	name##_RB_SETCOLOR(elm, RB_RED);                                \
} while ( /*CONSTCOND*/ 0)

#define RB_SET_BLACKRED(name, black, red, field) do {                           \
	name##_RB_SETCOLOR(black,  RB_BLACK);                           \
	name##_RB_SETCOLOR(red, RB_RED);                                        \
} while ( /*CONSTCOND*/ 0)

#ifndef RB_AUGMENT
#define RB_AUGMENT(x) (void)(x)
#endif

#define RB_ROTATE_LEFT(name, head, elm, tmp, field) do {                        \
	(tmp) = RB_RIGHT(elm, field);                                   \
	if ((RB_RIGHT(elm, field) = RB_LEFT(tmp, field)) != NULL) {     \
	        name##_RB_SETPARENT(RB_LEFT(tmp, field),(elm));         \
	}                                                               \
	RB_AUGMENT(elm);                                                \
	if (name##_RB_SETPARENT(tmp, name##_RB_GETPARENT(elm)) != NULL) {       \
	        if ((elm) == RB_LEFT(name##_RB_GETPARENT(elm), field))  \
	                RB_LEFT(name##_RB_GETPARENT(elm), field) = (tmp);       \
	        else                                                    \
	                RB_RIGHT(name##_RB_GETPARENT(elm), field) = (tmp);      \
	} else                                                          \
	        (head)->rbh_root = (tmp);                               \
	RB_LEFT(tmp, field) = (elm);                                    \
	name##_RB_SETPARENT(elm, (tmp));                                        \
	RB_AUGMENT(tmp);                                                \
	if ((name##_RB_GETPARENT(tmp)))                                 \
	        RB_AUGMENT(name##_RB_GETPARENT(tmp));                   \
} while ( /*CONSTCOND*/ 0)

#define RB_ROTATE_RIGHT(name, head, elm, tmp, field) do {                       \
	(tmp) = RB_LEFT(elm, field);                                    \
	if ((RB_LEFT(elm, field) = RB_RIGHT(tmp, field)) != NULL) {     \
	        name##_RB_SETPARENT(RB_RIGHT(tmp, field), (elm));               \
	}                                                               \
	RB_AUGMENT(elm);                                                \
	if (name##_RB_SETPARENT(tmp, name##_RB_GETPARENT(elm)) != NULL) {       \
	        if ((elm) == RB_LEFT(name##_RB_GETPARENT(elm), field))  \
	                RB_LEFT(name##_RB_GETPARENT(elm), field) = (tmp);       \
	        else                                                    \
	                RB_RIGHT(name##_RB_GETPARENT(elm), field) = (tmp);      \
	} else                                                          \
	        (head)->rbh_root = (tmp);                               \
	RB_RIGHT(tmp, field) = (elm);                                   \
	name##_RB_SETPARENT(elm, tmp);                                  \
	RB_AUGMENT(tmp);                                                \
	if ((name##_RB_GETPARENT(tmp)))                                 \
	        RB_AUGMENT(name##_RB_GETPARENT(tmp));                   \
} while ( /*CONSTCOND*/ 0)

/* Generates prototypes and inline functions */
#define RB_PROTOTYPE(name, type, field, cmp)                            \
void name##_RB_INSERT_COLOR(struct name *, struct type *);      \
void name##_RB_REMOVE_COLOR(struct name *, struct type *, struct type *);\
struct type *name##_RB_REMOVE(struct name *, struct type *);            \
struct type *name##_RB_INSERT(struct name *, struct type *);            \
struct type *name##_RB_FIND(struct name *, struct type *);              \
struct type *name##_RB_NFIND(struct name *, struct type *);             \
struct type *name##_RB_NEXT(struct type *);                             \
struct type *name##_RB_MINMAX(struct name *, int);                      \
struct type *name##_RB_GETPARENT(struct type*);                         \
struct type *name##_RB_SETPARENT(struct type*, struct type*);           \
int name##_RB_GETCOLOR(struct type*);                                   \
void name##_RB_SETCOLOR(struct type*,int);

/* Generates prototypes (with storage class) and inline functions */
#define RB_PROTOTYPE_SC(_sc_, name, type, field, cmp)                   \
_sc_ void name##_RB_INSERT_COLOR(struct name *, struct type *);         \
_sc_ void name##_RB_REMOVE_COLOR(struct name *, struct type *, struct type *); \
_sc_ struct type *name##_RB_REMOVE(struct name *, struct type *);       \
_sc_ struct type *name##_RB_INSERT(struct name *, struct type *);       \
_sc_ struct type *name##_RB_FIND(struct name *, struct type *);         \
_sc_ struct type *name##_RB_NFIND(struct name *, struct type *);        \
_sc_ struct type *name##_RB_NEXT(struct type *);                        \
_sc_ struct type *name##_RB_MINMAX(struct name *, int);                 \
_sc_ struct type *name##_RB_GETPARENT(struct type*);                    \
_sc_ struct type *name##_RB_SETPARENT(struct type*, struct type*);                      \
_sc_ int name##_RB_GETCOLOR(struct type*);                      \
_sc_ void name##_RB_SETCOLOR(struct type*,int)


/* Main rb operation.
 * Moves node close to the key of elm to top
 */
#define RB_GENERATE(name, type, field, cmp)                             \
struct type *name##_RB_GETPARENT(struct type *elm) {                            \
	struct type *parent = _RB_PARENT(elm, field);                   \
	if( parent != NULL) {                                           \
	        parent = (struct type*)((uintptr_t)parent & ~RB_COLOR_MASK);\
	        return( (struct type*) ( (parent == (struct type*) RB_PLACEHOLDER) ? NULL: parent));\
	}                                                               \
	return((struct type*)NULL);                                     \
}                                                                       \
int name##_RB_GETCOLOR(struct type *elm) {                                      \
	int color = 0;                                                  \
	color = (int)((uintptr_t)_RB_PARENT(elm,field) & RB_COLOR_MASK);\
	return(color);                                                  \
}                                                                       \
void name##_RB_SETCOLOR(struct type *elm,int color) {                           \
	struct type *parent = name##_RB_GETPARENT(elm);                 \
	if(parent == (struct type*)NULL)                                \
	        parent = (struct type*) RB_PLACEHOLDER;                 \
	_RB_PARENT(elm, field) = (struct type*)((uintptr_t)parent | (unsigned int)color);\
}                                                                       \
struct type *name##_RB_SETPARENT(struct type *elm, struct type *parent) {       \
	int color = name##_RB_GETCOLOR(elm);                                    \
	_RB_PARENT(elm, field) = parent;                                \
	if(color) name##_RB_SETCOLOR(elm, color);                               \
	return(name##_RB_GETPARENT(elm));                                       \
}                                                                       \
                                                                        \
void                                                                    \
name##_RB_INSERT_COLOR(struct name *head, struct type *elm)             \
{                                                                       \
	struct type *parent, *gparent, *tmp;                            \
	while ((parent = name##_RB_GETPARENT(elm)) != NULL &&           \
	    name##_RB_GETCOLOR(parent) == RB_RED) {                     \
	        gparent = name##_RB_GETPARENT(parent);                  \
	        if (parent == RB_LEFT(gparent, field)) {                \
	                tmp = RB_RIGHT(gparent, field);                 \
	                if (tmp && name##_RB_GETCOLOR(tmp) == RB_RED) { \
	                        name##_RB_SETCOLOR(tmp,  RB_BLACK);     \
	                        RB_SET_BLACKRED(name, parent, gparent, field);\
	                        elm = gparent;                          \
	                        continue;                               \
	                }                                               \
	                if (RB_RIGHT(parent, field) == elm) {           \
	                        RB_ROTATE_LEFT(name, head, parent, tmp, field);\
	                        tmp = parent;                           \
	                        parent = elm;                           \
	                        elm = tmp;                              \
	                }                                               \
	                RB_SET_BLACKRED(name, parent, gparent, field);  \
	                RB_ROTATE_RIGHT(name,head, gparent, tmp, field);        \
	        } else {                                                \
	                tmp = RB_LEFT(gparent, field);                  \
	                if (tmp && name##_RB_GETCOLOR(tmp) == RB_RED) { \
	                        name##_RB_SETCOLOR(tmp,  RB_BLACK);     \
	                        RB_SET_BLACKRED(name, parent, gparent, field);\
	                        elm = gparent;                          \
	                        continue;                               \
	                }                                               \
	                if (RB_LEFT(parent, field) == elm) {            \
	                        RB_ROTATE_RIGHT(name, head, parent, tmp, field);\
	                        tmp = parent;                           \
	                        parent = elm;                           \
	                        elm = tmp;                              \
	                }                                               \
	                RB_SET_BLACKRED(name, parent, gparent, field);  \
	                RB_ROTATE_LEFT(name, head, gparent, tmp, field);        \
	        }                                                       \
	}                                                               \
	name##_RB_SETCOLOR(head->rbh_root,  RB_BLACK);                  \
}                                                                       \
                                                                        \
void                                                                    \
name##_RB_REMOVE_COLOR(struct name *head, struct type *parent, struct type *elm) \
{                                                                       \
	struct type *tmp;                                               \
	while ((elm == NULL || name##_RB_GETCOLOR(elm) == RB_BLACK) &&  \
	    elm != RB_ROOT(head)) {                                     \
	        if (RB_LEFT(parent, field) == elm) {                    \
	                tmp = RB_RIGHT(parent, field);                  \
	                if (name##_RB_GETCOLOR(tmp) == RB_RED) {                \
	                        RB_SET_BLACKRED(name, tmp, parent, field);      \
	                        RB_ROTATE_LEFT(name, head, parent, tmp, field);\
	                        tmp = RB_RIGHT(parent, field);          \
	                }                                               \
	                if ((RB_LEFT(tmp, field) == NULL ||             \
	                    name##_RB_GETCOLOR(RB_LEFT(tmp, field)) == RB_BLACK) &&\
	                    (RB_RIGHT(tmp, field) == NULL ||            \
	                    name##_RB_GETCOLOR(RB_RIGHT(tmp, field)) == RB_BLACK)) {\
	                        name##_RB_SETCOLOR(tmp,  RB_RED);               \
	                        elm = parent;                           \
	                        parent = name##_RB_GETPARENT(elm);              \
	                } else {                                        \
	                        if (RB_RIGHT(tmp, field) == NULL ||     \
	                            name##_RB_GETCOLOR(RB_RIGHT(tmp, field)) == RB_BLACK) {\
	                                struct type *oleft;             \
	                                if ((oleft = RB_LEFT(tmp, field)) \
	                                    != NULL)                    \
	                                        name##_RB_SETCOLOR(oleft,  RB_BLACK);\
	                                name##_RB_SETCOLOR(tmp, RB_RED);        \
	                                RB_ROTATE_RIGHT(name, head, tmp, oleft, field);\
	                                tmp = RB_RIGHT(parent, field);  \
	                        }                                       \
	                        name##_RB_SETCOLOR(tmp, (name##_RB_GETCOLOR(parent)));\
	                        name##_RB_SETCOLOR(parent, RB_BLACK);   \
	                        if (RB_RIGHT(tmp, field))               \
	                                name##_RB_SETCOLOR(RB_RIGHT(tmp, field),RB_BLACK);\
	                        RB_ROTATE_LEFT(name, head, parent, tmp, field);\
	                        elm = RB_ROOT(head);                    \
	                        break;                                  \
	                }                                               \
	        } else {                                                \
	                tmp = RB_LEFT(parent, field);                   \
	                if (name##_RB_GETCOLOR(tmp) == RB_RED) {                \
	                        RB_SET_BLACKRED(name, tmp, parent, field);      \
	                        RB_ROTATE_RIGHT(name, head, parent, tmp, field);\
	                        tmp = RB_LEFT(parent, field);           \
	                }                                               \
	                if ((RB_LEFT(tmp, field) == NULL ||             \
	                    name##_RB_GETCOLOR(RB_LEFT(tmp, field)) == RB_BLACK) &&\
	                    (RB_RIGHT(tmp, field) == NULL ||            \
	                    name##_RB_GETCOLOR(RB_RIGHT(tmp, field)) == RB_BLACK)) {\
	                        name##_RB_SETCOLOR(tmp, RB_RED);                \
	                        elm = parent;                           \
	                        parent = name##_RB_GETPARENT(elm);              \
	                } else {                                        \
	                        if (RB_LEFT(tmp, field) == NULL ||      \
	                            name##_RB_GETCOLOR(RB_LEFT(tmp, field)) == RB_BLACK) {\
	                                struct type *oright;            \
	                                if ((oright = RB_RIGHT(tmp, field)) \
	                                    != NULL)                    \
	                                        name##_RB_SETCOLOR(oright,  RB_BLACK);\
	                                name##_RB_SETCOLOR(tmp,  RB_RED);       \
	                                RB_ROTATE_LEFT(name, head, tmp, oright, field);\
	                                tmp = RB_LEFT(parent, field);   \
	                        }                                       \
	                        name##_RB_SETCOLOR(tmp,(name##_RB_GETCOLOR(parent)));\
	                        name##_RB_SETCOLOR(parent, RB_BLACK);   \
	                        if (RB_LEFT(tmp, field))                \
	                                name##_RB_SETCOLOR(RB_LEFT(tmp, field), RB_BLACK);\
	                        RB_ROTATE_RIGHT(name, head, parent, tmp, field);\
	                        elm = RB_ROOT(head);                    \
	                        break;                                  \
	                }                                               \
	        }                                                       \
	}                                                               \
	if (elm)                                                        \
	        name##_RB_SETCOLOR(elm,  RB_BLACK);                     \
}                                                                       \
                                                                        \
struct type *                                                           \
name##_RB_REMOVE(struct name *head, struct type *elm)                   \
{                                                                       \
	struct type *child, *parent, *old = elm;                        \
	int color;                                                      \
	if (RB_LEFT(elm, field) == NULL)                                \
	        child = RB_RIGHT(elm, field);                           \
	else if (RB_RIGHT(elm, field) == NULL)                          \
	        child = RB_LEFT(elm, field);                            \
	else {                                                          \
	        struct type *left;                                      \
	        elm = RB_RIGHT(elm, field);                             \
	        while ((left = RB_LEFT(elm, field)) != NULL)            \
	                elm = left;                                     \
	        child = RB_RIGHT(elm, field);                           \
	        parent = name##_RB_GETPARENT(elm);                              \
	        color = name##_RB_GETCOLOR(elm);                                \
	        if (child)                                              \
	                name##_RB_SETPARENT(child, parent);             \
	        if (parent) {                                           \
	                if (RB_LEFT(parent, field) == elm)              \
	                        RB_LEFT(parent, field) = child;         \
	                else                                            \
	                        RB_RIGHT(parent, field) = child;        \
	                RB_AUGMENT(parent);                             \
	        } else                                                  \
	                RB_ROOT(head) = child;                          \
	        if (name##_RB_GETPARENT(elm) == old)                    \
	                parent = elm;                                   \
	        (elm)->field = (old)->field;                            \
	        if (name##_RB_GETPARENT(old)) {                         \
	                if (RB_LEFT(name##_RB_GETPARENT(old), field) == old)\
	                        RB_LEFT(name##_RB_GETPARENT(old), field) = elm;\
	                else                                            \
	                        RB_RIGHT(name##_RB_GETPARENT(old), field) = elm;\
	                RB_AUGMENT(name##_RB_GETPARENT(old));           \
	        } else                                                  \
	                RB_ROOT(head) = elm;                            \
	        name##_RB_SETPARENT(RB_LEFT(old, field), elm);          \
	        if (RB_RIGHT(old, field))                               \
	                name##_RB_SETPARENT(RB_RIGHT(old, field), elm); \
	        if (parent) {                                           \
	                left = parent;                                  \
	                do {                                            \
	                        RB_AUGMENT(left);                       \
	                } while ((left = name##_RB_GETPARENT(left)) != NULL); \
	        }                                                       \
	        goto color;                                             \
	}                                                               \
	parent = name##_RB_GETPARENT(elm);                                      \
	color = name##_RB_GETCOLOR(elm);                                        \
	if (child)                                                      \
	        name##_RB_SETPARENT(child, parent);                     \
	if (parent) {                                                   \
	        if (RB_LEFT(parent, field) == elm)                      \
	                RB_LEFT(parent, field) = child;                 \
	        else                                                    \
	                RB_RIGHT(parent, field) = child;                \
	        RB_AUGMENT(parent);                                     \
	} else                                                          \
	        RB_ROOT(head) = child;                                  \
color:                                                                  \
	if (color == RB_BLACK)                                          \
	        name##_RB_REMOVE_COLOR(head, parent, child);            \
	return (old);                                                   \
}                                                                       \
                                                                        \
/* Inserts a node into the RB tree */                                   \
struct type *                                                           \
name##_RB_INSERT(struct name *head, struct type *elm)                   \
{                                                                       \
	struct type *tmp;                                               \
	struct type *parent = NULL;                                     \
	int comp = 0;                                                   \
	tmp = RB_ROOT(head);                                            \
	while (tmp) {                                                   \
	        parent = tmp;                                           \
	        comp = (cmp)(elm, parent);                              \
	        if (comp < 0)                                           \
	                tmp = RB_LEFT(tmp, field);                      \
	        else if (comp > 0)                                      \
	                tmp = RB_RIGHT(tmp, field);                     \
	        else                                                    \
	                return (tmp);                                   \
	}                                                               \
	RB_SET(name, elm, parent, field);                                       \
	if (parent != NULL) {                                           \
	        if (comp < 0)                                           \
	                RB_LEFT(parent, field) = elm;                   \
	        else                                                    \
	                RB_RIGHT(parent, field) = elm;                  \
	        RB_AUGMENT(parent);                                     \
	} else                                                          \
	        RB_ROOT(head) = elm;                                    \
	name##_RB_INSERT_COLOR(head, elm);                              \
	return (NULL);                                                  \
}                                                                       \
                                                                        \
/* Finds the node with the same key as elm */                           \
struct type *                                                           \
name##_RB_FIND(struct name *head, struct type *elm)                     \
{                                                                       \
	struct type *tmp = RB_ROOT(head);                               \
	int comp;                                                       \
	while (tmp) {                                                   \
	        comp = cmp(elm, tmp);                                   \
	        if (comp < 0)                                           \
	                tmp = RB_LEFT(tmp, field);                      \
	        else if (comp > 0)                                      \
	                tmp = RB_RIGHT(tmp, field);                     \
	        else                                                    \
	                return (tmp);                                   \
	}                                                               \
	return (NULL);                                                  \
}                                                                       \
                                                                        \
/* Finds the first node greater than or equal to the search key */      \
__attribute__((unused))                                                 \
struct type *                                                           \
name##_RB_NFIND(struct name *head, struct type *elm)                    \
{                                                                       \
	struct type *tmp = RB_ROOT(head);                               \
	struct type *res = NULL;                                        \
	int comp;                                                       \
	while (tmp) {                                                   \
	        comp = cmp(elm, tmp);                                   \
	        if (comp < 0) {                                         \
	                res = tmp;                                      \
	                tmp = RB_LEFT(tmp, field);                      \
	        }                                                       \
	        else if (comp > 0)                                      \
	                tmp = RB_RIGHT(tmp, field);                     \
	        else                                                    \
	                return (tmp);                                   \
	}                                                               \
	return (res);                                                   \
}                                                                       \
                                                                        \
/* ARGSUSED */                                                          \
struct type *                                                           \
name##_RB_NEXT(struct type *elm)                                        \
{                                                                       \
	if (RB_RIGHT(elm, field)) {                                     \
	        elm = RB_RIGHT(elm, field);                             \
	        while (RB_LEFT(elm, field))                             \
	                elm = RB_LEFT(elm, field);                      \
	} else {                                                        \
	        if (name##_RB_GETPARENT(elm) &&                         \
	            (elm == RB_LEFT(name##_RB_GETPARENT(elm), field)))  \
	                elm = name##_RB_GETPARENT(elm);                 \
	        else {                                                  \
	                while (name##_RB_GETPARENT(elm) &&                      \
	                    (elm == RB_RIGHT(name##_RB_GETPARENT(elm), field)))\
	                        elm = name##_RB_GETPARENT(elm);         \
	                elm = name##_RB_GETPARENT(elm);                 \
	        }                                                       \
	}                                                               \
	return (elm);                                                   \
}                                                                       \
                                                                        \
struct type *                                                           \
name##_RB_MINMAX(struct name *head, int val)                            \
{                                                                       \
	struct type *tmp = RB_ROOT(head);                               \
	struct type *parent = NULL;                                     \
	while (tmp) {                                                   \
	        parent = tmp;                                           \
	        if (val < 0)                                            \
	                tmp = RB_LEFT(tmp, field);                      \
	        else                                                    \
	                tmp = RB_RIGHT(tmp, field);                     \
	}                                                               \
	return (parent);                                                \
}


#define RB_PROTOTYPE_PREV(name, type, field, cmp)                       \
	RB_PROTOTYPE(name, type, field, cmp)                            \
struct type *name##_RB_PREV(struct type *);


#define RB_PROTOTYPE_SC_PREV(_sc_, name, type, field, cmp)              \
	RB_PROTOTYPE_SC(_sc_, name, type, field, cmp);                  \
_sc_ struct type *name##_RB_PREV(struct type *)

#define RB_GENERATE_PREV(name, type, field, cmp)                        \
	RB_GENERATE(name, type, field, cmp);                            \
struct type *                                                           \
name##_RB_PREV(struct type *elm)                                        \
{                                                                       \
	if (RB_LEFT(elm, field)) {                                      \
	        elm = RB_LEFT(elm, field);                              \
	        while (RB_RIGHT(elm, field))                            \
	                elm = RB_RIGHT(elm, field);                     \
	} else {                                                        \
	        if (name##_RB_GETPARENT(elm) &&                         \
	            (elm == RB_RIGHT(name##_RB_GETPARENT(elm), field))) \
	                elm = name##_RB_GETPARENT(elm);                 \
	        else {                                                  \
	                while (name##_RB_GETPARENT(elm) &&              \
	                    (elm == RB_LEFT(name##_RB_GETPARENT(elm), field)))\
	                        elm = name##_RB_GETPARENT(elm);         \
	                elm = name##_RB_GETPARENT(elm);                 \
	        }                                                       \
	}                                                               \
	return (elm);                                                   \
}                                                                       \

#define RB_NEGINF       -1
#define RB_INF  1

#define RB_INSERT(name, x, y)   name##_RB_INSERT(x, y)
#define RB_REMOVE(name, x, y)   name##_RB_REMOVE(x, y)
#define RB_FIND(name, x, y)     name##_RB_FIND(x, y)
#define RB_NFIND(name, x, y)    name##_RB_NFIND(x, y)
#define RB_NEXT(name, x, y)     name##_RB_NEXT(y)
#define RB_PREV(name, x, y)     name##_RB_PREV(y)
#define RB_MIN(name, x)         name##_RB_MINMAX(x, RB_NEGINF)
#define RB_MAX(name, x)         name##_RB_MINMAX(x, RB_INF)

#define RB_FOREACH(x, name, head)                                       \
	for ((x) = RB_MIN(name, head);                                  \
	     (x) != NULL;                                               \
	     (x) = name##_RB_NEXT(x))

#define RB_FOREACH_FROM(x, name, y)                                     \
	for ((x) = (y);                                                 \
	    ((x) != NULL) && ((y) = name##_RB_NEXT(x), (x) != NULL);    \
	    (x) = (y))

#define RB_FOREACH_REVERSE_FROM(x, name, y)                             \
	for ((x) = (y);                                                 \
	    ((x) != NULL) && ((y) = name##_RB_PREV(x), (x) != NULL);    \
	     (x) = (y))

#define RB_FOREACH_SAFE(x, name, head, y)                               \
	for ((x) = RB_MIN(name, head);                                  \
	    ((x) != NULL) && ((y) = name##_RB_NEXT(x), (x) != NULL);    \
	     (x) = (y))

#endif  /* _LIBKERN_TREE_H_ */

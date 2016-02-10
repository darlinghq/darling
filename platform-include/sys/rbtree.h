/*	$NetBSD: rbtree.h,v 1.2 2012/02/17 08:20:55 yamt Exp $	*/

/*-
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * Portions Copyright (c) 2012 Apple Inc. All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Matt Thomas <matt@3am-software.com>.
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SYS_RBTREE_H_
#define	_SYS_RBTREE_H_

#include <Availability.h>

#include <sys/types.h>
#include <stdbool.h>
#include <inttypes.h>
#include <sys/queue.h>

__BEGIN_DECLS


#define	RB_DIR_LEFT		0
#define	RB_DIR_RIGHT		1

#define RB_TREE_MIN(T) rb_tree_iterate((T), NULL, RB_DIR_RIGHT)
#define RB_TREE_MAX(T) rb_tree_iterate((T), NULL, RB_DIR_LEFT)
#define RB_TREE_FOREACH(N, T) \
    for ((N) = RB_TREE_MIN(T); (N); \
	(N) = rb_tree_iterate((T), (N), RB_DIR_RIGHT))
#define RB_TREE_FOREACH_REVERSE(N, T) \
    for ((N) = RB_TREE_MAX(T); (N); \
	(N) = rb_tree_iterate((T), (N), RB_DIR_LEFT))

/*
 * rbto_compare_nodes_fn:
 *	return a positive value if the first node > the second node.
 *	return a negative value if the first node < the second node.
 *	return 0 if they are considered same.
 *
 * rbto_compare_key_fn:
 *	return a positive value if the node > the key.
 *	return a negative value if the node < the key.
 *	return 0 if they are considered same.
 */

typedef signed int (*rbto_compare_nodes_fn)(void *, const void *, const void *);
typedef signed int (*rbto_compare_key_fn)(void *, const void *, const void *);

typedef struct {
	rbto_compare_nodes_fn rbto_compare_nodes;
	rbto_compare_key_fn rbto_compare_key;
	size_t rbto_node_offset;
	void *rbto_context;
} rb_tree_ops_t;

typedef struct rb_node { void * opaque[3]; } rb_node_t;
typedef struct rb_tree { void *opaque[8]; } rb_tree_t;

#define _rb_availability __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
void	rb_tree_init(rb_tree_t *, const rb_tree_ops_t *) _rb_availability;
void *	rb_tree_insert_node(rb_tree_t *, void *) _rb_availability;
void *	rb_tree_find_node(rb_tree_t *, const void *) _rb_availability;
void *	rb_tree_find_node_geq(rb_tree_t *, const void *) _rb_availability;
void *	rb_tree_find_node_leq(rb_tree_t *, const void *) _rb_availability;
void	rb_tree_remove_node(rb_tree_t *, void *) _rb_availability;
void *	rb_tree_iterate(rb_tree_t *, void *, const unsigned int) _rb_availability;
size_t  rb_tree_count(rb_tree_t *) _rb_availability;
#undef _rb_availability

__END_DECLS

#endif	/* _SYS_RBTREE_H_*/

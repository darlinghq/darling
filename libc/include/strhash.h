#ifndef _STRHASH_H_INCLUDE
#define _STRHASH_H_INCLUDE

/* $FreeBSD: /repoman/r/ncvs/src/include/strhash.h,v 1.3 1999/08/28 04:59:30 peter Exp $ */

/*
 *
 *                      Copyright 1990
 *               Terry Jones & Jordan Hubbard
 *
 *		  PCS Computer Systeme, GmbH.
 *	             Munich, West Germany
 *
 *
 *  All rights reserved.
 *
 *  This is unsupported software and is subject to change without notice.
 *  the author makes no representations about the suitability of this software
 *  for any purpose. It is supplied "as is" without express or implied
 *  warranty.
 *
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted, provided
 *  that the above copyright notice appear in all copies and that both that
 *  copyright notice and this permission notice appear in supporting
 *  documentation, and that the name of the author not be used in
 *  advertising or publicity pertaining to distribution of the software
 *  without specific, written prior permission.
 *
 */

/*
 * This is the definition file for hash.c. The plunderer from down-under
 * did the code, I just helped define the spec. That's why his name gets
 * to go first.
 */

#define HASH_SZ 97

typedef struct _node {
    char *key;
    void *data;
    struct _node *next;
} hash_node;

typedef struct {
    int size;
    hash_node **buckets;
} hash_table;

#include <sys/cdefs.h>

__BEGIN_DECLS
hash_table *hash_create(int size);
void	hash_destroy(hash_table *table, char *key,
		     void (*nukefunc)(char *k, void *d));
void	*hash_search(hash_table *table, char *key, void *datum,
		     void (*replace_func)(void *d));
void	hash_traverse(hash_table *table,
		      int (*func)(char *k, void *d, void *arg), void *arg);
void	hash_purge(hash_table *table, void (*purge_func)(char *k, void *d));

#ifdef HASH_STATS
extern void hash_stats();
#endif
__END_DECLS

#endif /* _STRHASH_H_INCLUDE */

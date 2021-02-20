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
 * This is a fairly simple open addressing hash scheme.
 * Terry did all the code, I just did the spec.
 * Thanks again, you crazy Aussie..
 *
 */

/*
 * $Log: strhash.c,v $
 * Revision 2.0  90/03/26  01:44:26  jkh
 * pre-beta check-in
 *
 * Revision 1.8  90/03/09  19:22:35  jkh
 * Fixed bogus comment.
 *
 * Revision 1.7  90/03/09  19:01:08  jkh
 * Added comments, GPL.
 *
 * Revision 1.6  90/03/08  17:55:58  terry
 * Rearranged hash_purge to be a tiny bit more efficient.
 * Added verbose option to hash_stats.
 *
 * Revision 1.5  90/03/08  17:19:54  terry
 * Added hash_purge. Added arg to hash_traverse. Changed all
 * void * to Generic.
 *
 * Revision 1.4  90/03/08  12:02:35  terry
 * Fixed problems with allocation that I screwed up last night.
 * Changed bucket lists to be singly linked. Thanks to JKH, my hero.
 *
 * Revision 1.3  90/03/07  21:33:33  terry
 * Cleaned up a few decls to keep gcc -Wall quiet.
 *
 * Revision 1.2  90/03/07  21:14:53  terry
 * Comments. Added HASH_STATS define. Removed hash_find()
 * and new_node().
 *
 * Revision 1.1  90/03/07  20:49:45  terry
 * Initial revision
 *
 *
 */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstrict-prototypes"

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdlib/strhash.c,v 1.10 2002/03/22 21:53:10 obrien Exp $");

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <strhash.h>

#define HASH_NULL    (hash_table *)0
#define NODE_NULL    (hash_node *)0
#define GENERIC_NULL (void *)0

#define HASH_SZ 97


static int _hash(int size, char *key);
static hash_node *list_find(caddr_t key, hash_node *head);
static int assign_key(char *key, hash_node *node);


/*
 * hash_create()
 *
 * Malloc room for a new hash table and then room for its
 * bucket pointers. Then set all the buckets to
 * point to 0. Return the address of the new table.
 */
hash_table *
hash_create(int size)
{
    int i;
    hash_table *new = (hash_table *)malloc(sizeof(hash_table));

    if (!new || size < 0){
	return HASH_NULL;
    }

    if (size == 0){
	size = HASH_SZ;
    }

    if (!(new->buckets = (hash_node **)malloc(size * sizeof(hash_node *)))){
	return HASH_NULL;
    }

    for (i = 0; i < size; i++){
	new->buckets[i] = NODE_NULL;
    }
    new->size = size;

    return new;
}


/*
 * list_find()
 *
 * Find the key in the linked list pointed to by head.
 */
static hash_node *
list_find(caddr_t key, hash_node *head)
{
    while (head){
	if (!strcmp(head->key, key)){
	    return head;
	}
	head = head->next;
    }
    return NODE_NULL;
}


/*
 * _hash()
 *
 * Compute the hash value for the given key.
 */
static int
_hash(int size, char *key)
{
    unsigned int h = 0x0;

    while (*key){
	h = (h << 1) ^ (h ^ (unsigned char) *key++);
    }

    h %= size;
    return h;
}

/*
 * hash_destroy()
 *
 * Find the key and (if it's there) remove it entirely.
 * The function (*nukefunc)() is in charge of disposing
 * of the storage help by the data associated with the node.
 */
void
hash_destroy(hash_table *table, char *key, void (*nukefunc)())
{
    int bucket = _hash(table->size, key);
    hash_node *found = table->buckets[bucket];
    hash_node *to_free = NODE_NULL;

    if (!found) {
	return;
    }

    if (!strcmp(found->key, key)) {
	/*
	 * It was the head of the list.
	 */
	table->buckets[bucket] = found->next;
	to_free = found;
    }
    else {
	/*
	 * Walk the list, looking one ahead.
	 */
	while (found->next) {
	    if (!strcmp(found->next->key, key)) {
		to_free = found->next;
		found->next = found->next->next;
		break;
	    }
	    found = found->next;
	}

	if (!to_free){
	    return;
	}
    }

    if (nukefunc)
	(*nukefunc)(to_free->key, to_free->data);
    free(to_free);
    return;
}


/*
 * hash_search()
 *
 * Search the table for the given key. Then:
 *
 * 1) If you find it and there is no replacement function, just
 *    return what you found. (This is a simple search).
 * 2) If you find it and there is a replacement function, run
 *    the function on the data you found, and replace the old
 *    data with whatever is passed in datum. Return 0.
 * 3) If you don't find it and there is some datum, insert a
 *    new item into the table. Insertions go at the front of
 *    the bucket. Return 0.
 * 4) Otherwise just return 0.
 *
 */
void *
hash_search(hash_table *table, caddr_t key, void *datum,
	    void (*replace_func)())
{
    int bucket = _hash(table->size, key);
    hash_node *found = list_find(key, table->buckets[bucket]);

    if (found){
	if (!replace_func){
	    return found->data;
	}
	else{
	    (*replace_func)(found->data);
	    found->data = datum;
	}
    }
    else{
	if (datum){

	    hash_node *new = (hash_node *)malloc(sizeof(hash_node));

	    if (!new || !assign_key(key, new)){
		return GENERIC_NULL;
	    }
	    new->data = datum;
	    new->next = table->buckets[bucket];
	    table->buckets[bucket] = new;
	    return new;
	}
    }
    return GENERIC_NULL;
}


/*
 * assign_key()
 *
 * Set the key value of a node to be 'key'. Get some space from
 * malloc and copy it in etc. Return 1 if all is well, 0 otherwise.
 */
static int
assign_key(char *key, hash_node *node)
{
    if (!node || !key){
	return 0;
    }

    if (!(node->key = (char *)malloc(strlen(key) + 1))){
	return 0;
    }

    node->key[0] = '\0';
    strcat(node->key, key);
    return 1;
}

/*
 * hash_traverse()
 *
 * Traverse the hash table and run the function func on the
 * data found at each node and the argument we're passed for it.
 */
void
hash_traverse(hash_table *table, int (*func)(), void *arg)
{
    int i;
    int size = table->size;

    if (!func)
	return;

    for (i = 0; i < size; i++) {
	hash_node *n = table->buckets[i];
	while (n) {
	    if ((*func)(n->key, n->data, arg) == 0)
		return;
	    n = n->next;
	}
    }
    return;
}

/*
 * hash_purge()
 *
 * Run through the entire hash table. Call purge_func
 * on the data found at each node, and then free the node.
 * Set all the bucket pointers to 0.
 */
void
hash_purge(hash_table *table, void (*purge_func)(char *p1, void *p2))
{
    int i;
    int size = table->size;

    for (i = 0; i < size; i++) {
	hash_node *n = table->buckets[i];
	if (n) {
	    do {
		hash_node *to_free = n;
		if (purge_func) {
		    (*purge_func)(n->key, n->data);
		}
		n = n->next;
		free(to_free);
	    } while (n);
	    table->buckets[i] = NODE_NULL;
	}
    }
}

#undef min
#define min(a, b) (a) < (b) ? (a) : (b)

/*
 * hash_stats()
 *
 * Print statistics about the current table allocation to stdout.
 */
void
hash_stats(hash_table *table, int verbose)
{
    int i;
    int total_elements = 0;
    int non_empty_buckets = 0;
    int max_count = 0;
    int max_repeats = 0;
    int *counts;
    int size = table->size;

    if (!(counts = (int *)malloc(size * sizeof(int)))){
	fprintf(stderr, "malloc returns 0\n");
	exit(1);
    }

    for (i = 0; i < size; i++){
	int x = 0;
	hash_node *n = table->buckets[i];
	counts[i] = 0;
	while (n){
	    if (!x){
		x = 1;
		non_empty_buckets++;
		if (verbose){
		    printf("bucket %2d: ", i);
		}
	    }
	    if (verbose){
		printf(" %s", n->key);
	    }
	    counts[i]++;
	    n = n->next;
	}

	total_elements += counts[i];
	if (counts[i] > max_count){
	    max_count = counts[i];
	    max_repeats = 1;
	}
	else if (counts[i] == max_count){
	    max_repeats++;
	}

	if (counts[i] && verbose){
	    printf(" (%d)\n", counts[i]);
	}
    }

    printf("\n");
    printf("%d element%s in storage.\n", total_elements, total_elements == 1 ? "" : "s");

    if (total_elements){
	printf("%d of %d (%.2f%%) buckets are in use\n", non_empty_buckets, size,
	       (double)100 * (double)non_empty_buckets / (double)(size));
	printf("the maximum number of elements in a bucket is %d (%d times)\n", max_count, max_repeats);
	printf("average per bucket is %f\n", (double)total_elements / (double)non_empty_buckets);
	printf("optimal would be %f\n", (double)total_elements / (double)(min(size, total_elements)));
    }
    return;
}
#pragma clang diagnostic pop

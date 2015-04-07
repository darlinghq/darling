/*
 * Copyright (c) 2003-2011 Apple Inc. All rights reserved.
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

#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <os/assumes.h>
#include "table.h"

#define KEY_UNKNOWN    0
#define KEY_INT        1
#define KEY_STR_MINE   2
#define KEY_STR_SHARED 3

#define DEFAULT_SIZE 256

typedef struct table_node_s
{
	union
	{
		char *string;
		const char *const_string;
		uint64_t uint64;
	} key;
	void *datum;
	struct table_node_s *next;
} table_node_t;

typedef struct __table_private
{
	uint32_t type;
	uint32_t bucket_count;
	table_node_t **bucket;
} table_private_t;

typedef struct
{
	uint32_t bucket_index;
	table_node_t *node;
} table_traverse_t;

typedef struct __list_private
{
	struct __list_private *prev;
	struct __list_private *next;
	uint32_t refcount;
	void *data;
} list_private_t;

table_t *
_nc_table_new(uint32_t n)
{
	table_private_t *t;

	t = (table_t *)malloc(sizeof(table_t));
	if (t == NULL) return NULL;

	if (n == 0) n = DEFAULT_SIZE;

	t->type = KEY_UNKNOWN;
	t->bucket_count = n;
	t->bucket = (table_node_t **)calloc(t->bucket_count, sizeof(table_node_t *));
	if (t->bucket == NULL)
	{
		free(t);
		return NULL;
	}

	return (table_t *)t;
}

static uint32_t
hash_key(int size, const char *key)
{
	uint32_t v;
	char *p;

	if (key == NULL) return 0;

	v = 0;
	for (p = (char *)key; *p != '\0'; p++)
	{
		v = (v << 1) ^ (v ^ *p);
	}

	v %= size;
	return v;
}

static uint32_t
hash_nkey(uint32_t size, uint64_t key)
{
	uint32_t x = key;
	uint32_t y = key >> 32;
	return ((x ^ y) % size);
}

void *
_nc_table_find_get_key(table_t *tin, const char *key, const char **shared_key)
{
	table_private_t *t;
	table_node_t *n;
	uint32_t b;

	if (tin == NULL) return NULL;
	if (key == NULL) return NULL;

	if (shared_key != NULL) *shared_key = NULL;

	t = (table_private_t *)tin;
	b = hash_key(t->bucket_count, key);

	for (n = t->bucket[b]; n != NULL; n = n->next)
	{
		if ((n->key.string != NULL) && (!strcmp(key, n->key.string)))
		{
			if (shared_key != NULL) *shared_key = n->key.const_string;
			return n->datum;
		}
	}

	return NULL;
}

void *
_nc_table_find(table_t *tin, const char *key)
{
	return _nc_table_find_get_key(tin, key, NULL);
}

void *
_nc_table_find_64(table_t *tin, uint64_t key)
{
	table_private_t *t;
	table_node_t *n;
	uint32_t b;

	if (tin == NULL) return NULL;

	t = (table_private_t *)tin;
	b = hash_nkey(t->bucket_count, key);

	for (n = t->bucket[b]; n != NULL; n = n->next)
	{
		if ((n->key.uint64 != (uint64_t)-1) && (key == n->key.uint64)) return n->datum;
	}

	return NULL;
}

void *
_nc_table_find_n(table_t *tin, uint32_t key)
{
	uint64_t n64 = key;
	return _nc_table_find_64(tin, n64);
}

static void
_nc_table_insert_type(table_t *tin, int type, char *key, const char *ckey, void *datum)
{
	table_private_t *t;
	table_node_t *n;
	uint32_t b;

	if (tin == NULL) return;
	if ((key == NULL) && (ckey == NULL)) return;
	if (datum == NULL) return;

	t = (table_private_t *)tin;
	if (t->type == KEY_UNKNOWN) t->type = type;
	else os_assumes(t->type == type);

	n = (table_node_t *)malloc(sizeof(table_node_t));

	if (key != NULL)
	{
		b = hash_key(t->bucket_count, key);
		n->key.string = key;
	}
	else
	{
		b = hash_key(t->bucket_count, ckey);
		n->key.const_string = ckey;
	}

	n->datum = datum;
	n->next = t->bucket[b];
	t->bucket[b] = n;
}

void
_nc_table_insert(table_t *tin, const char *key, void *datum)
{
	char *dup;

	if (tin == NULL) return;
	if (key == NULL) return;
	if (datum == NULL) return;

	dup = strdup(key);
	if (dup == NULL) return;

	_nc_table_insert_type(tin, KEY_STR_MINE, dup, NULL, datum);
}

void
_nc_table_insert_no_copy(table_t *tin, const char *key, void *datum)
{
	if (tin == NULL) return;
	if (key == NULL) return;
	if (datum == NULL) return;

	_nc_table_insert_type(tin, KEY_STR_SHARED, NULL, key, datum);
}

void
_nc_table_insert_pass(table_t *tin, char *key, void *datum)
{
	if (tin == NULL) return;
	if (key == NULL) return;
	if (datum == NULL) return;

	_nc_table_insert_type(tin, KEY_STR_MINE, key, NULL, datum);
}

void
_nc_table_insert_64(table_t *tin, uint64_t key, void *datum)
{
	table_private_t *t;
	table_node_t *n;
	uint32_t b;

	if (tin == NULL) return;
	if (datum == NULL) return;

	t = (table_private_t *)tin;
	if (t->type == KEY_UNKNOWN) t->type = KEY_INT;
	else os_assumes(t->type == KEY_INT);

	b = hash_nkey(t->bucket_count, key);
	n = (table_node_t *)malloc(sizeof(table_node_t));
	n->key.uint64 = key;
	n->datum = datum;
	n->next = t->bucket[b];
	t->bucket[b] = n;
}

void
_nc_table_insert_n(table_t *tin, uint32_t key, void *datum)
{
	uint64_t n64 = key;
	_nc_table_insert_64(tin, n64, datum);
}

void
_nc_table_delete(table_t *tin, const char *key)
{
	table_private_t *t;
	table_node_t *n, *p;
	uint32_t b;

	if (tin == NULL) return;
	if (key == NULL) return;

	t = (table_private_t *)tin;
	os_assumes((t->type == KEY_STR_MINE) || (t->type == KEY_STR_SHARED));

	b = hash_key(t->bucket_count, key);

	p = NULL;
	for (n = t->bucket[b]; n != NULL; n = n->next)
	{
		if ((n->key.string != NULL) && (!strcmp(key, n->key.string)))
		{
			if (p == NULL) t->bucket[b] = n->next;
			else p->next = n->next;
			if (t->type == KEY_STR_MINE) free(n->key.string);
			free(n);
			return;
		}
		p = n;
	}
}

void
_nc_table_delete_64(table_t *tin, uint64_t key)
{
	table_private_t *t;
	table_node_t *n, *p;
	uint32_t b;

	if (tin == NULL) return;

	t = (table_private_t *)tin;
	os_assumes(t->type == KEY_INT);

	b = hash_nkey(t->bucket_count, key);

	p = NULL;
	for (n = t->bucket[b]; n != NULL; n = n->next)
	{
		if ((n->key.uint64 != (uint64_t)-1) && (key == n->key.uint64))
		{
			if (p == NULL) t->bucket[b] = n->next;
			else p->next = n->next;
			free(n);
			return;
		}
		p = n;
	}
}

void
_nc_table_delete_n(table_t *tin, uint32_t key)
{
	uint64_t n64 = key;
	_nc_table_delete_64(tin, n64);
}

void *
_nc_table_traverse_start(table_t *tin)
{
	table_traverse_t *tt;
	table_private_t *t;
	uint32_t b;

	if (tin == NULL) return NULL;

	t = (table_private_t *)tin;
	if (t->bucket_count == 0) return NULL;

	for (b = 0; b < t->bucket_count; b++)
	{
		if (t->bucket[b] != NULL)
		{
			tt = (table_traverse_t *)malloc(sizeof(table_traverse_t));
			if (tt == NULL) return NULL;
			tt->bucket_index = b;
			tt->node = t->bucket[b];
			return (void *)tt;
		}
	}

	return NULL;
}

void *
_nc_table_traverse(table_t *tin, void *ttin)
{
	table_private_t *t;
	table_traverse_t *tt;
	void *datum;
	uint32_t b;

	if (tin == NULL) return NULL;
	if (ttin == NULL) return NULL;

	t = (table_private_t *)tin;
	tt = (table_traverse_t *)ttin;

	if (tt->node == NULL) return NULL;

	datum = tt->node->datum;
	tt->node = tt->node->next;
	if (tt->node != NULL) return datum;

	for (b = tt->bucket_index + 1; b < t->bucket_count; b++)
	{
		if (t->bucket[b] != NULL)
		{
			tt->bucket_index = b;
			tt->node = t->bucket[b];
			return datum;
		}
	}

	tt->bucket_index = b;
	tt->node = NULL;

	return datum;
}

void
_nc_table_traverse_end(table_t *tin, void *ttin)
{
	if (ttin == NULL) return;
	free(ttin);
}

void
_nc_table_free(table_t *tin)
{
	table_private_t *t;
	table_node_t *n, *x;
	uint32_t b;

	if (tin == NULL) return;

	t = (table_private_t *)tin;

	for (b = 0; b < t->bucket_count; b++)
	{
		x = NULL;
		for (n = t->bucket[b]; n != NULL; n = x)
		{
			x = n->next;
			if (t->type == KEY_STR_MINE) free(n->key.string);
			free(n);
		}
	}

	free(t->bucket);
	free(t);
}

/* Linked List */

/*
 * Make a new node
 */
list_t *
_nc_list_new(void *d)
{
	list_t *n;

	n = (list_t *)calloc(1, sizeof(list_t));
	if (n == NULL) return NULL;

	n->refcount = 1;
	n->data = d;
	return n;
}

/*
 * Release a node
 */
void
_nc_list_release(list_t *l)
{
	if (l == NULL) return;
	l->refcount--;
	if (l->refcount > 0) return;

	free(l);
}

/*
 * Retain a node
 */
list_t *
_nc_list_retain(list_t *l)
{
	if (l == NULL) return NULL;
	l->refcount++;
	return l;
}

/*
 * Retain a list
 */
list_t *
_nc_list_retain_list(list_t *l)
{
	list_t *n;

	for (n = l; n != NULL; n = n->next) n->refcount++;
	return l;
}

/*
 * Get previous node
 */
list_t *
_nc_list_prev(list_t *l)
{
	if (l == NULL) return NULL;
	return l->prev;
}

/*
 * Get next node
 */
list_t *
_nc_list_next(list_t *l)
{
	if (l == NULL) return NULL;
	return l->next;
}

/*
 * Get head (first node) of list
 */
list_t *
_nc_list_head(list_t *l)
{
	list_t *p;

	if (l == NULL) return NULL;

	for (p = l; p->prev != NULL; p = p->prev);

	return p;
}

/*
 * Get tail (last node) of list
 */
list_t *
_nc_list_tail(list_t *l)
{
	list_t *p;

	if (l == NULL) return NULL;

	for (p = l; p->next != NULL; p = p->next);

	return p;
}

/*
 * Insert a node in front of another node.
 * Cuts list if n is NULL.
 */
list_t *
_nc_list_prepend(list_t *l, list_t *n)
{
	if (l == NULL) return n;

	if (n != NULL)
	{
		n->next = l;
		n->prev = l->prev;
	}

	if (l->prev != NULL) l->prev->next = n;
	l->prev = n;

	return n;
}

/*
 * Append a node after another node.
 * Cuts list if n is NULL.
 */
list_t *
_nc_list_append(list_t *l, list_t *n)
{
	if (l == NULL) return n;

	if (n != NULL)
	{
		n->prev = l;
		n->next = l->next;
	}

	if (l->next != NULL) n->next->prev = n;
	l->next = n;

	return n;
}

/*
 * Set next pointer - use with care.
 */
void
_nc_list_set_next(list_t *l, list_t *n)
{
	if (l == NULL) return;
	l->next = n;
}

/*
 * Set prev pointer - use with care.
 */
void
_nc_list_set_prev(list_t *l, list_t *p)
{
	if (l == NULL) return;
	l->prev = p;
}

/*
 * Concatenate two lists.
 * Returns new head.
 */
list_t *
_nc_list_concat(list_t *a, list_t *b)
{
	list_t *p;

	if (a == NULL) return b;
	if (b == NULL) return a;

	for (p = a; p->next != NULL; p = p->next);

	p->next = b;
	b->prev = p;

	for (p = a; p->prev != NULL; p = p->prev);

	return p;
}

uint32_t
_nc_list_count(list_t *l)
{
	uint32_t n;
	list_t *p;

	n = 0;
	for (p = l; p != NULL; p = p->next) n++;
	return n;
}

void *
_nc_list_data(list_t *l)
{
	if (l == NULL) return NULL;
	return l->data;
}

void
_nc_list_set_data(list_t *l, void *d)
{
	if (l != NULL) l->data = d;
}

list_t *
_nc_list_find(list_t *l, void *d)
{
	list_t *p;

	if (l == NULL) return NULL;

	for (p = l; p != NULL; p = p->next)
	{
		if (p->data == d) return p;
	}

	return NULL;
}

list_t *
_nc_list_find_release(list_t *l, void *d)
{
	list_t *p;

	if (l == NULL) return NULL;

	if (l->data == d)
	{
		p = l->next;
		if (p != NULL) p->prev = NULL;
		_nc_list_release(l);
		return p;
	}

	for (p = l->next; p != NULL; p = p->next)
	{
		if (p->data == d)
		{
			p->prev->next = p->next;
			if (p->next != NULL) p->next->prev = p->prev;
			_nc_list_release(p);
			return l;
		}
	}

	return l;
}

list_t *
_nc_list_reverse(list_t *l)
{
	list_t *x, *s, *r;

	if (l == NULL) return NULL;

	x = l->prev;
	r = l;
	s = l->next;

	while (s != NULL)
	{
		s = r->next;
		r->next = r->prev;
		r->prev = s;
		if (s != NULL) r = s;
	}

	if (x != NULL)
	{
		x->next = r;
		r->prev = x;
	}

	return r;
}

list_t *
_nc_list_extract(list_t *n)
{
	if (n == NULL) return NULL;

	if (n->prev != NULL) n->prev->next = n->next;
	if (n->next != NULL) n->next->prev = n->prev;

	n->prev = NULL;
	n->next = NULL;

	return n;
}

list_t *
_nc_list_chop(list_t *l)
{
	list_t *p;

	if (l == NULL) return NULL;
	p = l->next;
	if (p != NULL) p->prev = NULL;

	_nc_list_release(l);
	return p;
}

void
_nc_list_release_list(list_t *l)
{
	list_t *p, *n;

	if (l == NULL) return;
	if (l->prev != NULL) l->prev->next = NULL;

	p = l;
	while (p != NULL)
	{
		n = p->next;
		_nc_list_release(p);
		p = n;
	}
}

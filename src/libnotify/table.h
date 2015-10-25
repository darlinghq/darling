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

#ifndef _NOTIFY_TABLE_H_
#define _NOTIFY_TABLE_H_

#include <stdint.h>

typedef struct __table_private table_t;
typedef struct __list_private list_t;

extern table_t *_nc_table_new(uint32_t n);

extern void _nc_table_insert(table_t *t, const char *key, void *datum);
extern void _nc_table_insert_no_copy(table_t *t, const char *key, void *datum);
extern void _nc_table_insert_pass(table_t *t, char *key, void *datum);
extern void _nc_table_insert_n(table_t *t, uint32_t key, void *datum);
extern void _nc_table_insert_64(table_t *t, uint64_t key, void *datum);

extern void *_nc_table_find(table_t *t, const char *key);
extern void *_nc_table_find_get_key(table_t *tin, const char *key, const char **shared_key);
extern void *_nc_table_find_n(table_t *t, uint32_t key);
extern void *_nc_table_find_64(table_t *t, uint64_t key);

extern void _nc_table_delete(table_t *t, const char *key);
extern void _nc_table_delete_n(table_t *t, uint32_t key);
extern void _nc_table_delete_64(table_t *t, uint64_t key);

extern void *_nc_table_traverse_start(table_t *tin);
extern void *_nc_table_traverse(table_t *tin, void *ttin);
extern void _nc_table_traverse_end(table_t *tin, void *ttin);

extern void _nc_table_free(table_t *tin);

extern list_t *_nc_list_new(void *d);

extern list_t *_nc_list_retain(list_t *l);
extern list_t *_nc_list_retain_list(list_t *l);

extern void _nc_list_release(list_t *l);
extern void _nc_list_release_list(list_t *l);

extern list_t *_nc_list_prev(list_t *l);
extern list_t *_nc_list_next(list_t *l);

extern void _nc_list_set_next(list_t *l, list_t *n);
extern void _nc_list_set_prev(list_t *l, list_t *p);

extern list_t *_nc_list_head(list_t *l);
extern list_t *_nc_list_tail(list_t *l);

extern list_t *_nc_list_prepend(list_t *l, list_t *n);
extern list_t *_nc_list_append(list_t *l, list_t *n);

extern list_t *_nc_list_concat(list_t *a, list_t *b);

extern void *_nc_list_data(list_t *l);
extern void _nc_list_set_data(list_t *l, void *d);

extern list_t *_nc_list_find(list_t *l, void *d);
extern list_t *_nc_list_find_release(list_t *l, void *d);

extern list_t * _nc_list_reverse(list_t *l);
extern uint32_t _nc_list_count(list_t *l);
extern list_t *_nc_list_extract(list_t *n);
extern list_t *_nc_list_chop(list_t *l);

#endif /* _NOTIFY_TABLE_H_ */

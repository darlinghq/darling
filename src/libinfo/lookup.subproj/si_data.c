/*
 * Copyright (c) 2008-2009 Apple Inc.  All rights reserved.
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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <libkern/OSAtomic.h>
#include "si_data.h"
#include "si_module.h"

si_list_t *
si_list_add(si_list_t *l, si_item_t *e)
{
	size_t size;

	if (e == NULL) return l;

	if (l == NULL)
	{
		l = (si_list_t *)calloc(1, sizeof(si_list_t));
		l->refcount = 1;
	}

	if (l != NULL)
	{
		size = (l->count + 1) * sizeof(si_item_t *);

		l->entry = (si_item_t **)reallocf(l->entry, size);
		if (l->entry != NULL)
		{
			l->entry[l->count++] = si_item_retain(e);
		}
	}

	if ((l == NULL) || (l->entry == NULL))
	{
		free(l);
		l = NULL;
		errno = ENOMEM;
	}

	return l;
}

si_list_t *
si_list_concat(si_list_t *l, si_list_t *x)
{
	si_item_t *item;
	uint32_t newcount;
	size_t size;
	int i;

	if ((x == NULL) || (x->count == 0)) return l;

	if (l == NULL)
	{
		l = (si_list_t *)calloc(1, sizeof(si_list_t));
		l->refcount = 1;
	}

	if (l != NULL)
	{
		newcount = l->count + x->count;
		size = newcount * sizeof(si_item_t *);

		l->entry = (si_item_t **)reallocf(l->entry, size);
		if (l->entry)
		{
			for (i = 0; i < x->count; ++i)
			{
				item = x->entry[i];
				si_item_retain(item);
				l->entry[l->count + i] = item;
			}

			l->count += x->count;
		}
		else
		{
			l->count = 0;
			l = NULL;
		}
	}

	if (l == NULL) errno = ENOMEM;

	return l;
}

si_item_t *
si_list_next(si_list_t *list)
{
	if (list == NULL) return NULL;
	if (list->curr >= list->count) return NULL;

	return list->entry[list->curr++];
}

void
si_list_reset(si_list_t *list)
{
	if (list != NULL) list->curr = 0;
}

si_list_t *
si_list_retain(si_list_t *list)
{
	int32_t rc;

	if (list == NULL) return NULL;

	rc = OSAtomicIncrement32Barrier(&list->refcount);
	assert(rc >= 1);

	return list;
}

void
si_list_release(si_list_t *list)
{
	int32_t rc, i;

	if (list == NULL) return;

	rc = OSAtomicDecrement32Barrier(&list->refcount);
	assert(rc >= 0);

	if (rc == 0)
	{
		for (i = 0; i < list->count; i++)
		{
			si_item_release(list->entry[i]);
		}

		free(list->entry);
		free(list);
	}
}

si_item_t *
si_item_retain(si_item_t *item)
{
	int32_t rc;

	if (item == NULL) return NULL;

	rc = OSAtomicIncrement32Barrier(&item->refcount);
	assert(rc >= 1);

	return item;
}

void
si_item_release(si_item_t *item)
{
	int32_t rc;

	if (item == NULL) return;

	rc = OSAtomicDecrement32Barrier(&item->refcount);
	assert(rc >= 0);

	if (rc == 0) free(item);
}

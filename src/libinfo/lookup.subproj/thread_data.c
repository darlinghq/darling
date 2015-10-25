/*
 * Copyright (c) 2008 Apple Inc.  All rights reserved.
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

#include <thread_data.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

static pthread_key_t _info_key = 0;
static int _info_key_ok = 0;
static pthread_once_t _info_key_initialized = PTHREAD_ONCE_INIT;

struct _li_data_s
{
	uint32_t icount;
	uint32_t *ikey;
	void **idata;
};

typedef struct
{
	si_item_t *thread_item;
	si_list_t *thread_list;
} li_thread_data_t;

static void
_LI_thread_info_free(void *x)
{
	li_thread_data_t *tdata;

	if (x == NULL) return;

	tdata = (li_thread_data_t *)x;
	si_item_release(tdata->thread_item);
	si_list_release(tdata->thread_list);

	free(tdata);
}

static void
_LI_data_free(void *x)
{
	struct _li_data_s *t;
	int i;

	if (x == NULL) return;

	t = (struct _li_data_s *)x;

	for (i = 0; i < t->icount; i++)
	{
		_LI_thread_info_free(t->idata[i]);
		t->idata[i] = NULL;
	}

	if (t->ikey != NULL) free(t->ikey);
	t->ikey = NULL;

	if (t->idata != NULL) free(t->idata);
	t->idata = NULL;

	free(t);
}

static void
_LI_data_init()
{
	/* _info_key_ok is set to 1 if pthread_key_create succeeded */
	if (pthread_key_create(&_info_key, _LI_data_free) == 0) _info_key_ok = 1;
	return;
}

static struct _li_data_s *
_LI_data_get()
{
	struct _li_data_s *libinfo_data;

	/* only one thread should create the _info_key */
	pthread_once(&_info_key_initialized, _LI_data_init);

	/* no thread-specific data if pthread_key_create failed */
	if (_info_key_ok == 0) return NULL;

	/* Check if this thread already created libinfo_data */
	libinfo_data = pthread_getspecific(_info_key);
	if (libinfo_data != NULL) return libinfo_data;

	libinfo_data = (struct _li_data_s *)calloc(1, sizeof(struct _li_data_s));
	if (libinfo_data == NULL) return NULL;

	pthread_setspecific(_info_key, libinfo_data);
	return libinfo_data;
}

static li_thread_data_t *
LI_get_thread_info(uint32_t key)
{
	struct _li_data_s *libinfo_data;
	li_thread_data_t *tdata;
	uint32_t i, n;

	libinfo_data = _LI_data_get();
	if (libinfo_data == NULL) return NULL;

	for (i = 0; i < libinfo_data->icount; i++)
	{
		if (libinfo_data->ikey[i] == key) return libinfo_data->idata[i];
	}

	i = libinfo_data->icount;
	n = i + 1;

	if (i == 0)
	{
		libinfo_data->ikey = (uint32_t *)malloc(sizeof(uint32_t));
		libinfo_data->idata = (void **)malloc(sizeof(void *));
	}
	else
	{
		libinfo_data->ikey = (uint32_t *)reallocf(libinfo_data->ikey, n * sizeof(uint32_t));
		libinfo_data->idata = (void **)reallocf(libinfo_data->idata, n * sizeof(void *));
	}

	if ((libinfo_data->ikey == NULL) || (libinfo_data->idata == NULL))
	{
		if (libinfo_data->ikey != NULL) free(libinfo_data->ikey);
		libinfo_data->ikey = NULL;

		if (libinfo_data->idata != NULL) free(libinfo_data->idata);
		libinfo_data->idata = NULL;

		return NULL;
	}

	tdata = (li_thread_data_t *)calloc(1, sizeof(li_thread_data_t));
	if (tdata == NULL) return NULL;

	libinfo_data->ikey[i] = key;
	libinfo_data->idata[i] = tdata;
	libinfo_data->icount++;

	return tdata;
}

si_item_t *
LI_get_thread_item(uint32_t key)
{
	li_thread_data_t *tdata;

	tdata = LI_get_thread_info(key);
	if (tdata == NULL) return NULL;

	return tdata->thread_item;
}

si_list_t *
LI_get_thread_list(uint32_t key)
{
	li_thread_data_t *tdata;

	tdata = LI_get_thread_info(key);
	if (tdata == NULL) return NULL;

	return tdata->thread_list;
}

void
LI_set_thread_item(uint32_t key, si_item_t *item)
{
	li_thread_data_t *tdata;

	tdata = LI_get_thread_info(key);
	if (tdata == NULL) return;

	si_item_release(tdata->thread_item);
	tdata->thread_item = item;
}

void
LI_set_thread_list(uint32_t key, si_list_t *list)
{
	li_thread_data_t *tdata;

	tdata = LI_get_thread_info(key);
	if (tdata == NULL) return;

	si_list_release(tdata->thread_list);

	si_list_reset(list);
	tdata->thread_list = list;
}

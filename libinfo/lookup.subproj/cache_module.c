/*
 * Copyright (c) 2008-2011 Apple Inc.  All rights reserved.
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

#include <si_module.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <ils.h>
#include <pthread.h>
#include <libkern/OSAtomic.h>
#include <dispatch/dispatch.h>

/* GLOBAL */
uint32_t gL1CacheEnabled = 1;

#define CACHE_COUNT CATEGORY_COUNT
#define CACHE_MAX 20

typedef struct
{
	pthread_mutex_t mutex;
	int head;
	si_item_t *item[CACHE_MAX];
	si_list_t *list;
} cache_store_t;

typedef struct
{
	cache_store_t cache_store[CACHE_COUNT];
} cache_si_private_t;

static void *
cache_validate_item(cache_si_private_t *pp, int cat, int where)
{
	si_item_t *item;

	item = pp->cache_store[cat].item[where];
	if (item == NULL) return NULL;

	if (si_item_is_valid(item)) return si_item_retain(item);

	si_item_release(item);
	pp->cache_store[cat].item[where] = NULL;

	return NULL;
}

static void *
cache_validate_list(cache_si_private_t *pp, int cat)
{
	uint32_t i, valid;
	si_item_t *item, *last;
	si_list_t *list;

	list = pp->cache_store[cat].list;

	if (list == NULL) return NULL;
	if (list->count == 0) return NULL;

	last = list->entry[0];
	valid = si_item_is_valid(last);

	for (i = 1; (i < list->count) && (valid == 1); i++)
	{
		item = list->entry[i];
		if ((item->src == last->src) && (item->type == last->type) && (item->validation_a == last->validation_a) && (item->validation_b == last->validation_b)) continue;

		last = item;
		valid = si_item_is_valid(last);
	}

	if (valid) return si_list_retain(list);

	si_list_release(list);
	pp->cache_store[cat].list = NULL;

	return NULL;
}

static si_item_t *
cache_fetch_item(si_mod_t *si, int cat, const char *name, uint32_t num, int which)
{
	int i;
	cache_si_private_t *pp;
	si_item_t *item;

	if (si == NULL) return NULL;
	if (gL1CacheEnabled == 0) return NULL;

	pp = (cache_si_private_t *)si->private;
	if (pp == NULL) return NULL;

	pthread_mutex_lock(&pp->cache_store[cat].mutex);

	for (i = 0; i < CACHE_MAX; i++)
	{
		item = cache_validate_item(pp, cat, i);
		if (item && si_item_match(item, cat, name, num, which))
		{
			break;
		}
		else
		{
			si_item_release(item);
			item = NULL;
		}
	}

	pthread_mutex_unlock(&(pp->cache_store[cat].mutex));

	return item;
}

static si_list_t *
cache_fetch_list(si_mod_t *si, int cat)
{
	cache_si_private_t *pp;
	si_list_t *list;

	if (si == NULL) return NULL;
	if (gL1CacheEnabled == 0) return NULL;

	pp = (cache_si_private_t *)si->private;
	if (pp == NULL) return NULL;

	pthread_mutex_lock(&(pp->cache_store[cat].mutex));
	list = cache_validate_list(pp, cat);
	pthread_mutex_unlock(&(pp->cache_store[cat].mutex));

	return list;
}

static si_item_t *
cache_user_byname(si_mod_t *si, const char *name)
{
	return cache_fetch_item(si, CATEGORY_USER, name, 0, SEL_NAME);
}

static si_item_t *
cache_user_byuid(si_mod_t *si, uid_t uid)
{
	return cache_fetch_item(si, CATEGORY_USER, NULL, uid, SEL_NUMBER);
}

static si_list_t *
cache_user_all(si_mod_t *si)
{
	return cache_fetch_list(si, CATEGORY_USER);
}

static si_item_t *
cache_group_byname(si_mod_t *si, const char *name)
{
	return cache_fetch_item(si, CATEGORY_GROUP, name, 0, SEL_NAME);
}

static si_item_t *
cache_group_bygid(si_mod_t *si, gid_t gid)
{
	return cache_fetch_item(si, CATEGORY_GROUP, NULL, gid, SEL_NUMBER);
}

static si_list_t *
cache_group_all(si_mod_t *si)
{
	return cache_fetch_list(si, CATEGORY_GROUP);
}

static si_item_t *
cache_grouplist(si_mod_t *si, const char *name, uint32_t count)
{
	return cache_fetch_item(si, CATEGORY_GROUPLIST, name, 0, SEL_NAME);
}

static si_item_t *
cache_alias_byname(si_mod_t *si, const char *name)
{
	return cache_fetch_item(si, CATEGORY_ALIAS, name, 0, SEL_NAME);
}

static si_list_t *
cache_alias_all(si_mod_t *si)
{
	return cache_fetch_list(si, CATEGORY_ALIAS);
}

static si_item_t *
cache_host_byname(si_mod_t *si, const char *name, int af, const char *ignored, uint32_t *err)
{
	si_item_t *item;

	if (err != NULL) *err = SI_STATUS_NO_ERROR;
	item = NULL;

	if (af == AF_INET) item = cache_fetch_item(si, CATEGORY_HOST_IPV4, name, af, SEL_NAME);
	else item = cache_fetch_item(si, CATEGORY_HOST_IPV6, name, af, SEL_NAME);

	if ((item == NULL) && (err != NULL) && (*err == 0)) *err = SI_STATUS_H_ERRNO_HOST_NOT_FOUND;

	return item;
}

static si_item_t *
cache_host_byaddr(si_mod_t *si, const void *addr, int af, const char *ignored, uint32_t *err)
{
	si_item_t *item;

	if (err != NULL) *err = SI_STATUS_NO_ERROR;
	item = NULL;

	if (af == AF_INET) item = cache_fetch_item(si, CATEGORY_HOST_IPV4, addr, af, SEL_NUMBER);
	else item = cache_fetch_item(si, CATEGORY_HOST_IPV6, addr, af, SEL_NUMBER);

	if ((item == NULL) && (err != NULL) && (*err == 0)) *err = SI_STATUS_H_ERRNO_HOST_NOT_FOUND;

	return item;
}

static si_list_t *
cache_host_all(si_mod_t *si)
{
	return cache_fetch_list(si, CATEGORY_HOST);
}

static si_item_t *
cache_network_byname(si_mod_t *si, const char *name)
{
	return cache_fetch_item(si, CATEGORY_NETWORK, name, 0, SEL_NAME);
}

static si_item_t *
cache_network_byaddr(si_mod_t *si, uint32_t addr)
{
	return cache_fetch_item(si, CATEGORY_NETWORK, NULL, addr, SEL_NUMBER);
}

static si_list_t *
cache_network_all(si_mod_t *si)
{
	return cache_fetch_list(si, CATEGORY_NETWORK);
}

static si_item_t *
cache_service_byname(si_mod_t *si, const char *name, const char *proto)
{
	uint32_t pn;

	if (name == NULL) return NULL;
	if (proto == NULL) return cache_fetch_item(si, CATEGORY_SERVICE, name, 0, SEL_NAME);

	pn = 1;
	if (string_equal(proto, "tcp")) pn = 2;

	return cache_fetch_item(si, CATEGORY_SERVICE, name, pn, SEL_NAME);
}

static si_item_t *
cache_service_byport(si_mod_t *si, int port, const char *proto)
{
	return cache_fetch_item(si, CATEGORY_SERVICE, proto, port, SEL_NUMBER);
}

static si_list_t *
cache_service_all(si_mod_t *si)
{
	return cache_fetch_list(si, CATEGORY_SERVICE);
}

static si_item_t *
cache_protocol_byname(si_mod_t *si, const char *name)
{
	return cache_fetch_item(si, CATEGORY_PROTOCOL, name, 0, SEL_NAME);
}

static si_item_t *
cache_protocol_bynumber(si_mod_t *si, int number)
{
	return cache_fetch_item(si, CATEGORY_PROTOCOL, NULL, number, SEL_NUMBER);
}

static si_list_t *
cache_protocol_all(si_mod_t *si)
{
	return cache_fetch_list(si, CATEGORY_PROTOCOL);
}

static si_item_t *
cache_rpc_byname(si_mod_t *si, const char *name)
{
	return cache_fetch_item(si, CATEGORY_RPC, name, 0, SEL_NAME);
}

static si_item_t *
cache_rpc_bynumber(si_mod_t *si, int number)
{
	return cache_fetch_item(si, CATEGORY_RPC, NULL, number, SEL_NUMBER);
}

static si_list_t *
cache_rpc_all(si_mod_t *si)
{
	return cache_fetch_list(si, CATEGORY_RPC);
}

static si_item_t *
cache_fs_byspec(si_mod_t *si, const char *name)
{
	return cache_fetch_item(si, CATEGORY_FS, name, 0, SEL_NAME);
}

static si_item_t *
cache_fs_byfile(si_mod_t *si, const char *name)
{
	return cache_fetch_item(si, CATEGORY_FS, name, 0, SEL_NUMBER);
}

static si_list_t *
cache_fs_all(si_mod_t *si)
{
	return cache_fetch_list(si, CATEGORY_FS);
}

static si_item_t *
cache_mac_byname(si_mod_t *si, const char *name)
{
	return cache_fetch_item(si, CATEGORY_MAC, name, 0, SEL_NAME);
}

static si_item_t *
cache_mac_bymac(si_mod_t *si, const char *mac)
{
	return cache_fetch_item(si, CATEGORY_MAC, mac, 0, SEL_NUMBER);
}

static si_list_t *
cache_mac_all(si_mod_t *si)
{
	return cache_fetch_list(si, CATEGORY_MAC);
}

static si_item_t *
cache_nameinfo(si_mod_t *si, const struct sockaddr *sa, int flags, const char *ignored, uint32_t *err)
{
	/*
	 * Caching of getnameinfo(3) is not supported.
	 * Only the individual host_byaddr and serv_byaddr responses will be cached.
	 * This is because getnameinfo(3) returns numeric responses instead of
	 * failing, which would poison the cache.
	 */
	if (err) *err = SI_STATUS_H_ERRNO_HOST_NOT_FOUND;
	return NULL;
}

static void
cache_close(si_mod_t *si)
{
	cache_si_private_t *pp;
	int i, j;

	if (si == NULL) return;

	pp = (cache_si_private_t *)si->private;
	if (pp == NULL) return;

	for (i = 0; i < CACHE_COUNT; i++)
	{
		si_list_release(pp->cache_store[i].list);

		for (j = 0; j < CACHE_MAX; j++)
		{
			si_item_release(pp->cache_store[i].item[j]);
			pp->cache_store[i].item[j] = NULL;
		}
		
		pthread_mutex_destroy(&(pp->cache_store[i].mutex));
	}

	free(pp);
}

si_mod_t *
si_module_static_cache(void)
{
	static const struct si_mod_vtable_s cache_vtable =
	{
		.sim_close = &cache_close,

		.sim_user_byname = &cache_user_byname,
		.sim_user_byuid = &cache_user_byuid,
		.sim_user_byuuid = NULL,
		.sim_user_all = &cache_user_all,

		.sim_group_byname = &cache_group_byname,
		.sim_group_bygid = &cache_group_bygid,
		.sim_group_byuuid = NULL,
		.sim_group_all = &cache_group_all,

		.sim_grouplist = &cache_grouplist,

		/* no netgroup support */
		.sim_netgroup_byname = NULL,
		.sim_in_netgroup = NULL,

		.sim_alias_byname = &cache_alias_byname,
		.sim_alias_all = &cache_alias_all,

		.sim_host_byname = &cache_host_byname,
		.sim_host_byaddr = &cache_host_byaddr,
		.sim_host_all = &cache_host_all,

		.sim_network_byname = &cache_network_byname,
		.sim_network_byaddr = &cache_network_byaddr,
		.sim_network_all = &cache_network_all,

		.sim_service_byname = &cache_service_byname,
		.sim_service_byport = &cache_service_byport,
		.sim_service_all = &cache_service_all,

		.sim_protocol_byname = &cache_protocol_byname,
		.sim_protocol_bynumber = &cache_protocol_bynumber,
		.sim_protocol_all = &cache_protocol_all,

		.sim_rpc_byname = &cache_rpc_byname,
		.sim_rpc_bynumber = &cache_rpc_bynumber,
		.sim_rpc_all = &cache_rpc_all,

		.sim_fs_byspec = &cache_fs_byspec,
		.sim_fs_byfile = &cache_fs_byfile,
		.sim_fs_all = &cache_fs_all,

		.sim_mac_byname = &cache_mac_byname,
		.sim_mac_bymac = &cache_mac_bymac,
		.sim_mac_all = &cache_mac_all,

		/* no addrinfo support */
		.sim_wants_addrinfo = NULL,
		.sim_addrinfo = NULL,

		.sim_nameinfo = &cache_nameinfo,
	};

	static si_mod_t si =
	{
		.vers = 1,
		.refcount = 1,
		.flags = SI_MOD_FLAG_STATIC,

		.private = NULL,
		.vtable = &cache_vtable,
	};

	static dispatch_once_t once;

	dispatch_once(&once, ^{
		cache_si_private_t *cache;
		int i, j;
		
		cache = calloc(1, sizeof(cache_si_private_t));
		si.name = strdup("cache");
		si.private = cache;

		for (i = 0; i < CACHE_COUNT; i++) {
			for (j = 0; j < CACHE_MAX; j++) {
				pthread_mutex_init(&(cache->cache_store[i].mutex), NULL);
			}
		}
	});

	return &si;
}

void
si_cache_add_item(si_mod_t *si, si_mod_t *src, si_item_t *item)
{
	cache_si_private_t *pp;
	int head, cat;

	if (si == NULL) return;
	if (src == NULL) return;
	if (item == NULL) return;

	if (si == src) return;

	if (src->name == NULL) return;
	if (string_equal(src->name, "cache")) return;

	cat = item->type;
	if ((cat < 0) || (cat >= CACHE_COUNT)) return;

	pp = (cache_si_private_t *)si->private;
	if (pp == NULL) return;

	pthread_mutex_lock(&(pp->cache_store[cat].mutex));

	head = pp->cache_store[item->type].head;

	si_item_release(pp->cache_store[item->type].item[head]);
	pp->cache_store[item->type].item[head] = si_item_retain(item);

	head++;
	if (head >= CACHE_MAX) head = 0;
	pp->cache_store[item->type].head = head;

	pthread_mutex_unlock(&(pp->cache_store[cat].mutex));
}

void
si_cache_add_list(si_mod_t *si, si_mod_t *src, si_list_t *list)
{
	cache_si_private_t *pp;
	si_item_t *item;
	int cat;

	if (si == NULL) return;
	if (src == NULL) return;
	if (list == NULL) return;
	if (list->count == 0) return;

	if (si == src) return;

	if (src->name == NULL) return;
	if (string_equal(src->name, "cache")) return;

	item = list->entry[0];
	if (item == NULL) return;

	cat = item->type;
	if ((cat < 0) || (cat >= CACHE_COUNT)) return;

	pp = (cache_si_private_t *)si->private;
	if (pp == NULL) return;

	pthread_mutex_lock(&(pp->cache_store[cat].mutex));

	si_list_release(pp->cache_store[item->type].list);
	pp->cache_store[item->type].list = si_list_retain(list);

	pthread_mutex_unlock(&(pp->cache_store[cat].mutex));
}

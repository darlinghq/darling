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

#include <stdlib.h>
#include <paths.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <dispatch/dispatch.h>
#include "si_module.h"

#define _PATH_SI_CONF "/etc/sysinfo.conf"

#define SEARCH_FLAG_CACHE_ENABLED 0x00000001
#define SEARCH_MODULE_FLAG_DISABLED 0x00000001

typedef struct
{
	si_mod_t **module;
	uint32_t *module_flags;
	uint32_t count;
	uint32_t flags;
} search_list_t;

typedef struct
{
	search_list_t search_list[CATEGORY_COUNT];
	si_mod_t *cache;
} search_si_private_t;

extern void si_cache_add_item(si_mod_t *si, si_mod_t *src, si_item_t *item);
extern void si_cache_add_list(si_mod_t *si, si_mod_t *src, si_list_t *list);

extern char **_fsi_tokenize(char *data, const char *sep, int trailing_empty, int *ntokens);
extern char *_fsi_get_line(FILE *fp);

static void si_module_config_parse_line(search_si_private_t *pp, char *line);
static void si_module_config_modules_for_category(search_si_private_t *pp, int cat, int ntokens, const char * const *tokens);

static si_mod_t *
search_get_module(search_si_private_t *pp, int cat, int *n)
{
	int x;

	if ((pp == NULL) || (n == NULL)) return NULL;

	x = *n;
	*n = x + 1;

	/* Use custom search list if available */
	if (pp->search_list[cat].count > 0 && x < pp->search_list[cat].count)
	{
		return pp->search_list[cat].module[x];
	}

	/* Otherwise use the default search list */
	while (x < pp->search_list[CATEGORY_DEFAULT].count)
	{
		if (pp->search_list[CATEGORY_DEFAULT].module_flags[x] & SEARCH_MODULE_FLAG_DISABLED)
		{
			x++;
			*n = x + 1;
		}
		else
		{
			return pp->search_list[CATEGORY_DEFAULT].module[x];
		}
	}

	return NULL;
}

__private_extern__ void
search_set_flags(si_mod_t *si, const char *name, uint32_t flag)
{
	search_si_private_t *pp;
	uint32_t i;

	if (si == NULL) return;
	if (si->private == NULL) return;

	pp = (search_si_private_t *)si->private;

	for (i = 0; i < pp->search_list[CATEGORY_DEFAULT].count; i++)
	{
		si_mod_t *mod = pp->search_list[CATEGORY_DEFAULT].module[i];
		if ((mod == NULL) || (mod->name == NULL)) continue;

		if (string_equal(name, mod->name))
		{
			pp->search_list[CATEGORY_DEFAULT].module_flags[i] = flag;
			break;
		}
	}
}

static si_mod_t *
search_cat_cache(search_si_private_t *pp, int cat)
{
	if (pp == NULL) return NULL;
	if (cat < 0 || cat > CATEGORY_COUNT) return NULL;

	if (pp->search_list[cat].count == 0)
	{
		cat = CATEGORY_DEFAULT;
	}

	if ((pp->search_list[cat].flags & SEARCH_FLAG_CACHE_ENABLED) != 0)
	{
		return pp->cache;
	}

	return NULL;
}

static void
search_close(si_mod_t *si)
{
	int i;
	search_si_private_t *pp;

	if (si == NULL) return;
	if (si->private == NULL) return;

	pp = (search_si_private_t *)si->private;

	for (i = 0; i < CATEGORY_COUNT; i++)
	{
		if (pp->search_list[i].module != NULL)
		{
			free(pp->search_list[i].module);
			pp->search_list[i].module = NULL;
			pp->search_list[i].count = 0;
			pp->search_list[i].flags = 0;
		}
	}

	free(pp);
}

static si_item_t *
search_item_byname(si_mod_t *si, const char *name, int cat, si_item_t *(*call)(si_mod_t *, const char *))
{
	int i;
	search_si_private_t *pp;
	si_item_t *item;
	si_mod_t *src;

	if (si == NULL) return NULL;
	if (call == NULL) return NULL;

	pp = (search_si_private_t *)si->private;
	if (pp == NULL) return NULL;

	i = 0;

	while (NULL != (src = search_get_module(pp, cat, &i)))
	{
		item = call(src, name);
		if (item != NULL)
		{
			si_cache_add_item(search_cat_cache(pp, cat), src, item);
			return item;
		}
	}

	return NULL;
}

static si_item_t *
search_item_bynumber(si_mod_t *si, uint32_t number, int cat, si_item_t *(*call)(si_mod_t *, uint32_t))
{
	int i;
	search_si_private_t *pp;
	si_item_t *item;
	si_mod_t *src;

	if (si == NULL) return NULL;
	if (call == NULL) return NULL;

	pp = (search_si_private_t *)si->private;
	if (pp == NULL) return NULL;

	i = 0;

	while (NULL != (src = search_get_module(pp, cat, &i)))
	{
		item = call(src, number);
		if (item != NULL)
		{
			si_cache_add_item(search_cat_cache(pp, cat), src, item);
			return item;
		}
	}

	return NULL;
}

static si_item_t *
search_item_byuuid(si_mod_t *si, uuid_t uuid, int cat, si_item_t *(*call)(si_mod_t *, uuid_t))
{
	int i;
	search_si_private_t *pp;
	si_item_t *item;
	si_mod_t *src;

	if (si == NULL) return NULL;
	if (call == NULL) return NULL;

	pp = (search_si_private_t *)si->private;
	if (pp == NULL) return NULL;

	i = 0;

	while (NULL != (src = search_get_module(pp, cat, &i)))
	{
		item = call(src, uuid);
		if (item != NULL)
		{
			/*
			 * N.B. item not added to cache, since the data does not
			 * contain the uuid that was used to find it.
			 */
			return item;
		}
	}

	return NULL;
}
static si_list_t *
search_list(si_mod_t *si, int cat, si_list_t *(*call)(si_mod_t *))
{
	int i, null_res;
	search_si_private_t *pp;
	si_list_t *list, *all;
	si_mod_t *cache, *src;

	if (si == NULL) return NULL;
	if (call == NULL) return NULL;

	pp = (search_si_private_t *)si->private;
	if (pp == NULL) return NULL;

	cache = search_cat_cache(pp, cat);
	if (cache != NULL)
	{
		list = call(cache);
		if (list != NULL) return list;
	}

	i = 0;

	all = NULL;
	null_res = 0;

	while (NULL != (src = search_get_module(pp, cat, &i)))
	{
		if (src == pp->cache) continue;

		list = call(src);
		if (list == NULL)
		{
			null_res = 1;
			continue;
		}

		all = si_list_concat(all, list);
		si_list_release(list);
	}

	if ((all != NULL) && (null_res == 0)) si_cache_add_list(cache, si, all);
	return all;
}

static si_item_t *
search_user_byname(si_mod_t *si, const char *name)
{
	return search_item_byname(si, name, CATEGORY_USER, si_user_byname);
}

static si_item_t *
search_user_byuid(si_mod_t *si, uid_t uid)
{
	return search_item_bynumber(si, (uint32_t)uid, CATEGORY_USER, si_user_byuid);
}

static si_item_t *
search_user_byuuid(si_mod_t *si, uuid_t uuid)
{
	return search_item_byuuid(si, uuid, CATEGORY_USER, si_user_byuuid);
}

static si_list_t *
search_user_all(si_mod_t *si)
{
	return search_list(si, CATEGORY_USER, si_user_all);
}

static si_item_t *
search_group_byname(si_mod_t *si, const char *name)
{
	return search_item_byname(si, name, CATEGORY_GROUP, si_group_byname);
}

static si_item_t *
search_group_bygid(si_mod_t *si, gid_t gid)
{
	return search_item_bynumber(si, (uint32_t)gid, CATEGORY_GROUP, si_group_bygid);
}

static si_item_t *
search_group_byuuid(si_mod_t *si, uuid_t uuid)
{
	return search_item_byuuid(si, uuid, CATEGORY_GROUP, si_group_byuuid);
}

static si_list_t *
search_group_all(si_mod_t *si)
{
	return search_list(si, CATEGORY_GROUP, si_group_all);
}

static si_item_t *
search_groupist(si_mod_t *si, const char *name, uint32_t count)
{
	int i;
	search_si_private_t *pp;
	si_item_t *item = NULL;
	si_mod_t *src;
	
	if (si == NULL) return NULL;
	
	pp = (search_si_private_t *)si->private;
	if (pp == NULL) return NULL;
	
	i = 0;
	
	while (NULL != (src = search_get_module(pp, CATEGORY_GROUPLIST, &i))) {
		if (src == pp->cache) continue;
		
		if (src->vtable->sim_grouplist != NULL) {
			item = src->vtable->sim_grouplist(si, name, count);
			if (item != NULL) break;
		}
	}
	
	return item;
}

static si_list_t *
search_netgroup_byname(si_mod_t *si, const char *name)
{
	int i, cat, null_res;
	search_si_private_t *pp;
	si_list_t *list, *all;
	si_mod_t *cache, *src;

	if (si == NULL) return NULL;

	pp = (search_si_private_t *)si->private;
	if (pp == NULL) return NULL;

	cat = CATEGORY_NETGROUP;

	cache = search_cat_cache(pp, cat);
	if (cache != NULL)
	{
		list = si_netgroup_byname(cache, name);
		if (list != NULL) return list;
	}

	i = 0;
	null_res = 0;

	all = NULL;
	while (NULL != (src = search_get_module(pp, cat, &i)))
	{
		if (src == pp->cache) continue;

		list = si_netgroup_byname(src, name);
		if (list == NULL)
		{
			null_res = 1;
			continue;
		}

		all = si_list_concat(all, list);
		si_list_release(list);
	}

	if ((all != NULL) && (null_res == 0)) si_cache_add_list(cache, si, all);
	return all;
}

static int
search_in_netgroup(si_mod_t *si, const char *group, const char *host, const char *user, const char *domain)
{
	int i, cat, innetgr;
	search_si_private_t *pp;
	si_mod_t *src;

	if (si == NULL) return 0;

	pp = (search_si_private_t *)si->private;
	if (pp == NULL) return 0;

	cat = CATEGORY_NETGROUP;
	i = 0;

	while (NULL != (src = search_get_module(pp, cat, &i)))
	{
		innetgr = si_in_netgroup(src, group, host, user, domain);
		if (innetgr != 0) return 1;
	}

	return 0;
}

static si_item_t *
search_alias_byname(si_mod_t *si, const char *name)
{
	return search_item_byname(si, name, CATEGORY_ALIAS, si_alias_byname);
}

static si_list_t *
search_alias_all(si_mod_t *si)
{
	return search_list(si, CATEGORY_ALIAS, si_alias_all);
}

static si_item_t *
search_host_byname(si_mod_t *si, const char *name, int af, const char *interface, uint32_t *err)
{
	int i, cat;
	search_si_private_t *pp;
	si_item_t *item;
	si_mod_t *src;

	if (err != NULL) *err = SI_STATUS_NO_ERROR;

	if ((si == NULL) || (name == NULL))
	{
		if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
		return NULL;
	}

	pp = (search_si_private_t *)si->private;
	if (pp == NULL)
	{
		if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
		return NULL;
	}

	cat = CATEGORY_HOST_IPV4;
	if (af == AF_INET6) cat = CATEGORY_HOST_IPV6;

	i = 0;

	while (NULL != (src = search_get_module(pp, cat, &i)))
	{
		item = si_host_byname(src, name, af, interface, err);
		if (item != NULL)
		{
			si_cache_add_item(search_cat_cache(pp, cat), src, item);
			return item;
		}
	}

	if (err != NULL) *err = SI_STATUS_H_ERRNO_HOST_NOT_FOUND;
	return NULL;
}

static si_item_t *
search_host_byaddr(si_mod_t *si, const void *addr, int af, const char *interface, uint32_t *err)
{
	int i, cat;
	search_si_private_t *pp;
	si_item_t *item;
	si_mod_t *src;

	if (err != NULL) *err = SI_STATUS_NO_ERROR;

	if ((si == NULL) || (addr == NULL))
	{
		if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
		return NULL;
	}

	pp = (search_si_private_t *)si->private;
	if (pp == NULL)
	{
		if (err != NULL) *err = SI_STATUS_H_ERRNO_NO_RECOVERY;
		return NULL;
	}

	cat = CATEGORY_HOST_IPV4;
	if (af == AF_INET6) cat = CATEGORY_HOST_IPV6;

	i = 0;

	while (NULL != (src = search_get_module(pp, cat, &i)))
	{
		item = si_host_byaddr(src, addr, af, interface, err);
		if (item != NULL)
		{
			si_cache_add_item(search_cat_cache(pp, cat), src, item);
			return item;
		}
	}

	if (err != NULL) *err = SI_STATUS_H_ERRNO_HOST_NOT_FOUND;
	return NULL;
}

static si_list_t *
search_host_all(si_mod_t *si)
{
	return search_list(si, CATEGORY_HOST, si_host_all);
}

static si_item_t *
search_network_byname(si_mod_t *si, const char *name)
{
	return search_item_byname(si, name, CATEGORY_NETWORK, si_network_byname);
}

static si_item_t *
search_network_byaddr(si_mod_t *si, uint32_t addr)
{
	return search_item_bynumber(si, addr, CATEGORY_NETWORK, si_network_byaddr);
}

static si_list_t *
search_network_all(si_mod_t *si)
{
	return search_list(si, CATEGORY_NETWORK, si_network_all);
}

static si_item_t *
search_service_byname(si_mod_t *si, const char *name, const char *proto)
{
	int i, cat;
	si_item_t *item;
	search_si_private_t *pp;
	si_mod_t *src;

	if (si == NULL) return NULL;
	if (name == NULL) return NULL;

	pp = (search_si_private_t *)si->private;
	if (pp == NULL) return NULL;

	cat = CATEGORY_SERVICE;
	i = 0;

	while (NULL != (src = search_get_module(pp, cat, &i)))
	{
		item = si_service_byname(src, name, proto);
		if (item != NULL)
		{
			si_cache_add_item(search_cat_cache(pp, cat), src, item);
			return item;
		}
	}

	return NULL;
}

static si_item_t *
search_service_byport(si_mod_t *si, int port, const char *proto)
{
	int i, cat;
	search_si_private_t *pp;
	si_item_t *item;
	si_mod_t *src;

	if (si == NULL) return NULL;

	pp = (search_si_private_t *)si->private;
	if (pp == NULL) return NULL;

	cat = CATEGORY_SERVICE;
	i = 0;

	while (NULL != (src = search_get_module(pp, cat, &i)))
	{
		item = si_service_byport(src, port, proto);
		if (item != NULL)
		{
			si_cache_add_item(search_cat_cache(pp, cat), src, item);
			return item;
		}
	}

	return NULL;
}

static si_list_t *
search_service_all(si_mod_t *si)
{
	return search_list(si, CATEGORY_SERVICE, si_service_all);
}

static si_item_t *
search_protocol_byname(si_mod_t *si, const char *name)
{
	return search_item_byname(si, name, CATEGORY_PROTOCOL, si_protocol_byname);
}

static si_item_t *
search_protocol_bynumber(si_mod_t *si, int number)
{
	return search_item_bynumber(si, (uint32_t)number, CATEGORY_PROTOCOL, si_protocol_bynumber);
}

static si_list_t *
search_protocol_all(si_mod_t *si)
{
	return search_list(si, CATEGORY_PROTOCOL, si_protocol_all);
}

static si_item_t *
search_rpc_byname(si_mod_t *si, const char *name)
{
	return search_item_byname(si, name, CATEGORY_RPC, si_rpc_byname);
}

static si_item_t *
search_rpc_bynumber(si_mod_t *si, int number)
{
	int i, cat;
	search_si_private_t *pp;
	si_item_t *item;
	si_mod_t *src;

	if (si == NULL) return NULL;

	pp = (search_si_private_t *)si->private;
	if (pp == NULL) return NULL;

	cat = CATEGORY_RPC;
	i = 0;

	while (NULL != (src = search_get_module(pp, cat, &i)))
	{
		item = si_rpc_bynumber(src, number);
		if (item != NULL)
		{
			si_cache_add_item(search_cat_cache(pp, cat), src, item);
			return item;
		}
	}

	return NULL;
}

static si_list_t *
search_rpc_all(si_mod_t *si)
{
	return search_list(si, CATEGORY_RPC, si_rpc_all);
}

static si_item_t *
search_fs_byspec(si_mod_t *si, const char *name)
{
	return search_item_byname(si, name, CATEGORY_FS, si_fs_byspec);
}

static si_item_t *
search_fs_byfile(si_mod_t *si, const char *name)
{
	return search_item_byname(si, name, CATEGORY_FS, si_fs_byfile);
}

static si_list_t *
search_fs_all(si_mod_t *si)
{
	return search_list(si, CATEGORY_FS, si_fs_all);
}

static si_item_t *
search_mac_byname(si_mod_t *si, const char *name)
{
	return search_item_byname(si, name, CATEGORY_MAC, si_mac_byname);
}

static si_item_t *
search_mac_bymac(si_mod_t *si, const char *mac)
{
	return search_item_byname(si, mac, CATEGORY_MAC, si_mac_bymac);
}

static si_list_t *
search_mac_all(si_mod_t *si)
{
	return search_list(si, CATEGORY_MAC, si_mac_all);
}

static si_list_t *
search_srv_byname(si_mod_t *si, const char* qname, const char *interface, uint32_t *err)
{
	int i, cat;
	si_list_t *list = NULL;
	si_mod_t *src;
	search_si_private_t *pp;

	if (si == NULL) return NULL;

	pp = (search_si_private_t *)si->private;
	if (pp == NULL) return NULL;

	cat = CATEGORY_SRV;
	i = 0;

	while (NULL != (src = search_get_module(pp, cat, &i)))
	{
		if (src == pp->cache) continue;

		if (src->vtable->sim_srv_byname != NULL)
		{
			list = src->vtable->sim_srv_byname(src, qname, interface, err);
			if (list != NULL) return list;
		}
	}

	if ((i > 0) && (err != NULL)) *err = SI_STATUS_EAI_NONAME;
	return NULL;
}

static int
search_wants_addrinfo(si_mod_t *si)
{
	int i, cat;
	si_mod_t *src;
	search_si_private_t *pp;

	if (si == NULL) return 0;

	pp = (search_si_private_t *)si->private;
	if (pp == NULL) return 0;

	cat = CATEGORY_ADDRINFO;
	i = 0;

	while (NULL != (src = search_get_module(pp, cat, &i)))
	{
		if (src == pp->cache) continue;
		if (src->vtable->sim_addrinfo != NULL) return 1;
	}

	return 0;
}

static si_list_t *
search_addrinfo(si_mod_t *si, const void *node, const void *serv, uint32_t family, uint32_t socktype, uint32_t protocol, uint32_t flags, const char *interface, uint32_t *err)
{
	int i, cat;
	search_si_private_t *pp;
	si_list_t *list = NULL;
	si_mod_t *src;

	if (err != NULL) *err = SI_STATUS_EAI_FAIL;

	if (si == NULL) return NULL;

	pp = (search_si_private_t *)si->private;
	if (pp == NULL) return NULL;

	cat = CATEGORY_ADDRINFO;
	i = 0;

	while (NULL != (src = search_get_module(pp, cat, &i)))
	{
		if (src == pp->cache) continue;

		if (src->vtable->sim_addrinfo != NULL)
		{
			list = src->vtable->sim_addrinfo(src, node, serv, family, socktype, protocol, flags, interface, err);
			if (list != NULL) return list;
		}
	}

	if ((i > 0) && (err != NULL)) *err = SI_STATUS_EAI_NONAME;
	return NULL;
}

static si_item_t *
search_nameinfo(si_mod_t *si, const struct sockaddr *sa, int flags, const char *interface, uint32_t *err)
{
	int i, cat;
	search_si_private_t *pp;
	si_item_t *item;
	si_mod_t *src;

	if (err != NULL) *err = SI_STATUS_EAI_FAIL;

	if (si == NULL) return NULL;

	pp = (search_si_private_t *)si->private;
	if (pp == NULL) return NULL;

	cat = CATEGORY_NAMEINFO;
	i = 0;

	while (NULL != (src = search_get_module(pp, cat, &i)))
	{
		item = si_nameinfo(src, sa, flags, interface, err);
		if (item != NULL)
		{
			si_cache_add_item(search_cat_cache(pp, cat), src, item);
			if (err != NULL) *err = SI_STATUS_NO_ERROR;
			return item;
		}
	}

	if ((i > 0) && (err != NULL)) *err = SI_STATUS_EAI_NONAME;
	return NULL;
}

static int
search_is_valid(si_mod_t *si, si_item_t *item)
{
	si_mod_t *src;

	if (si == NULL) return 0;
	if (item == NULL) return 0;
	if (si->name == NULL) return 0;
	if (item->src == NULL) return 0;

	src = (si_mod_t *)item->src;

	if (src->name == NULL) return 0;
	if (string_not_equal(si->name, src->name)) return 0;
	return 0;
}

si_mod_t *
si_module_static_search(void)
{
	static const struct si_mod_vtable_s search_vtable =
	{
		.sim_close = &search_close,

		.sim_is_valid = &search_is_valid,

		.sim_user_byname = &search_user_byname,
		.sim_user_byuid = &search_user_byuid,
		.sim_user_byuuid = &search_user_byuuid,
		.sim_user_all = &search_user_all,

		.sim_group_byname = &search_group_byname,
		.sim_group_bygid = &search_group_bygid,
		.sim_group_byuuid = &search_group_byuuid,
		.sim_group_all = &search_group_all,

		.sim_grouplist = &search_groupist,

		.sim_netgroup_byname = &search_netgroup_byname,
		.sim_in_netgroup = &search_in_netgroup,

		.sim_alias_byname = &search_alias_byname,
		.sim_alias_all = &search_alias_all,

		.sim_host_byname = &search_host_byname,
		.sim_host_byaddr = &search_host_byaddr,
		.sim_host_all = &search_host_all,

		.sim_network_byname = &search_network_byname,
		.sim_network_byaddr = &search_network_byaddr,
		.sim_network_all = &search_network_all,

		.sim_service_byname = &search_service_byname,
		.sim_service_byport = &search_service_byport,
		.sim_service_all = &search_service_all,

		.sim_protocol_byname = &search_protocol_byname,
		.sim_protocol_bynumber = &search_protocol_bynumber,
		.sim_protocol_all = &search_protocol_all,

		.sim_rpc_byname = &search_rpc_byname,
		.sim_rpc_bynumber = &search_rpc_bynumber,
		.sim_rpc_all = &search_rpc_all,

		.sim_fs_byspec = &search_fs_byspec,
		.sim_fs_byfile = &search_fs_byfile,
		.sim_fs_all = &search_fs_all,

		.sim_mac_byname = &search_mac_byname,
		.sim_mac_bymac = &search_mac_bymac,
		.sim_mac_all = &search_mac_all,

		.sim_addrinfo = &search_addrinfo,
		.sim_wants_addrinfo = &search_wants_addrinfo,
		.sim_nameinfo = &search_nameinfo,

		.sim_srv_byname = &search_srv_byname,
	};

	static si_mod_t si =
	{
		.vers = 1,
		.refcount = 1,
		.flags = SI_MOD_FLAG_STATIC,

		.private = NULL,
		.vtable = &search_vtable,
	};

	static dispatch_once_t once;

	dispatch_once(&once, ^{
		si.name = strdup("search");
		search_si_private_t *pp = calloc(1, sizeof(search_si_private_t));
		si.private = pp;

		/*
		 * Default search order:
		 * 1) cache
		 * 2) DirectoryService/OpenDirectory (where available)
		 * 3) flat file
		 * 4) mDNSResponder
		 */

		const char * const modules[] =
		{
			"default", // CATEGORY_DEFAULT
			"cache",
#ifdef DS_AVAILABLE
			"ds",
#endif
			"mdns",
			"file",
		};

		int count = sizeof(modules) / sizeof(char *);
		si_module_config_modules_for_category(pp, CATEGORY_DEFAULT, count, modules);
		pp->cache = pp->search_list[CATEGORY_DEFAULT].module[0];

		char *check = getenv("SYSINFO_CONF_ENABLE");
		if ((check != NULL) && (!strcmp(check, "1")))
		{
			FILE *conf = fopen(_PATH_SI_CONF, "r");
			errno = 0;
			if (conf != NULL)
			{
				forever
				{
					char *line = _fsi_get_line(conf);
					if (line == NULL) break;

					si_module_config_parse_line(pp, line);
					free(line);
				}

				fclose(conf);
			}
		}
	});

	return &si;
}

static void
si_module_config_parse_line(search_si_private_t *pp, char *line)
{
	if (line == NULL || line[0] == '#') {
		return;
	}

	int ntokens = 0;
	char **tokens = _fsi_tokenize(line, "	: ", 0, &ntokens);

	int cat = CATEGORY_INVALID;

	if (string_equal(tokens[0], "default")) cat = CATEGORY_DEFAULT;
	else if (string_equal(tokens[0], "user")) cat = CATEGORY_USER;
	else if (string_equal(tokens[0], "group")) cat = CATEGORY_GROUP;
	else if (string_equal(tokens[0], "grouplist")) cat = CATEGORY_GROUPLIST;
	else if (string_equal(tokens[0], "netgroup")) cat = CATEGORY_NETGROUP;
	else if (string_equal(tokens[0], "alias")) cat = CATEGORY_ALIAS;
	else if (string_equal(tokens[0], "host")) cat = CATEGORY_HOST_IPV4;
	else if (string_equal(tokens[0], "network")) cat = CATEGORY_NETWORK;
	else if (string_equal(tokens[0], "service")) cat = CATEGORY_SERVICE;
	else if (string_equal(tokens[0], "protocol")) cat = CATEGORY_PROTOCOL;
	else if (string_equal(tokens[0], "rpc")) cat = CATEGORY_RPC;
	else if (string_equal(tokens[0], "fs")) cat = CATEGORY_FS;
	else if (string_equal(tokens[0], "mac")) cat = CATEGORY_MAC;
	else if (string_equal(tokens[0], "addrinfo")) cat = CATEGORY_ADDRINFO;
	else if (string_equal(tokens[0], "nameinfo")) cat = CATEGORY_NAMEINFO;

	if (cat != CATEGORY_INVALID)
	{
		si_module_config_modules_for_category(pp, cat, ntokens, (const char * const *)tokens);
	}

	free(tokens);
}

static void
si_module_config_modules_for_category(search_si_private_t *pp, int cat, int ntokens, const char * const *tokens)
{
	int count = ntokens - 1;
	pp->search_list[cat].count = count;
	if (count == 0)
	{
		return;
	}

	pp->search_list[cat].module = (si_mod_t **)calloc(pp->search_list[cat].count, sizeof(si_mod_t *));
	pp->search_list[cat].module_flags = (uint32_t *)calloc(pp->search_list[cat].count, sizeof(uint32_t));
	if ((pp->search_list[cat].module == NULL) || (pp->search_list[cat].module_flags == NULL))
	{
		free(pp->search_list[cat].module);
		free(pp->search_list[cat].module_flags);
		return;
	}

	int i, j;
	for (i = 1, j = 0; i < ntokens; i++)
	{
		si_mod_t *mod = si_module_with_name(tokens[i]);
		if (mod != NULL)
		{
			pp->search_list[cat].module[j] = mod;
			j++;

			if (string_equal(tokens[i], "cache"))
			{
				pp->search_list[cat].flags |= SEARCH_FLAG_CACHE_ENABLED;
			}
		}
	}
}

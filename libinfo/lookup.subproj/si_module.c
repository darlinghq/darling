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

#include <sys/types.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <libkern/OSAtomic.h>
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <mach/mach.h>
#include <dispatch/dispatch.h>
#include <dispatch/private.h>
#include "si_module.h"

#define PLUGIN_DIR_PATH "/usr/lib/info"
#define PLUGIN_BUNDLE_SUFFIX "so"

#define WORKUNIT_CANCELLED     0x00000001
#define WORKUNIT_RETURNS_LIST  0x00000002

#ifdef __BIG_ENDIAN__
#define WORKUNIT_CANCELLED_BIT_ADDRESS 31
#else
#define WORKUNIT_CANCELLED_BIT_ADDRESS 7
#endif

typedef struct si_async_workunit_s
{
	si_mod_t *si;
	uint32_t call;
	char *str1;
	char *str2;
	char *str3;
	uint32_t num1;
	uint32_t num2;
	uint32_t num3;
	uint32_t num4;
	uint32_t err;
	/* async support below */
	uint32_t flags;
	int32_t refcount;
	void *callback;
	void *context;
	mach_port_t port;
	mach_port_t send;
	si_item_t *resitem;
	si_list_t *reslist;
	struct si_async_workunit_s *next;
} si_async_workunit_t;

static si_mod_t **module_list = NULL;
static uint32_t module_count = 0;
static pthread_mutex_t module_mutex = PTHREAD_MUTEX_INITIALIZER;
static si_async_workunit_t *si_async_worklist = NULL;

si_mod_t *si_module_static_search(void);
si_mod_t *si_module_static_cache(void);
si_mod_t *si_module_static_file(void);
#ifdef DS_AVAILABLE
si_mod_t *si_module_static_ds(void);
#endif
si_mod_t *si_module_static_mdns(void);

static void *
si_mod_dlsym(void *so, const char *name, const char *sym)
{
	char *str;
	void *out;

	if ((so == NULL) || (name == NULL) || (sym == NULL)) return NULL;

	str = NULL;
	asprintf(&str, "%s_%s", name, sym);
	if (str == NULL) return NULL;

	out = dlsym(so, str);
	free(str);
	return out;
}

si_mod_t *
si_module_with_path(const char *path, const char *name)
{
	void *so;
	int (*si_sym_init)(si_mod_t *);
	void (*si_sym_close)(si_mod_t *);
	int status;
	si_mod_t *out;
	char *outname;

	if ((path == NULL) || (name == NULL))
	{
		errno = EINVAL;
		return NULL;
	}

	so = dlopen(path, RTLD_LOCAL);
	if (so == NULL) return NULL;

	si_sym_init = si_mod_dlsym(so, name, "init");
	if (si_sym_init == NULL)
	{
		dlclose(so);
		errno = ECONNREFUSED;
		return NULL;
	}

	si_sym_close = si_mod_dlsym(so, name, "close");
	if (si_sym_close == NULL)
	{
		dlclose(so);
		errno = ECONNREFUSED;
		return NULL;
	}

	out = (si_mod_t *)calloc(1, sizeof(si_mod_t));
	outname = strdup(name);

	if ((out == NULL) || (outname == NULL))
	{
		free(out);
		free(outname);
		dlclose(so);
		errno = ENOMEM;
		return NULL;
	}

	out->name = outname;
	out->refcount = 1;
	out->flags = 0;
	out->bundle = so;

	status = si_sym_init(out);
	if (status != 0)
	{
		dlclose(so);
		free(out);
		free(outname);
		errno = ECONNREFUSED;
		return NULL;
	}

	return out;
}

si_mod_t *
si_module_with_name(const char *name)
{
	static struct
	{
		const char *name;
		si_mod_t *(*init)(void);
		si_mod_t *module;
	} modules[] =
	{
		{ "search", si_module_static_search, NULL },
		{ "cache", si_module_static_cache, NULL },
		{ "file", si_module_static_file, NULL },
#ifdef DS_AVAILABLE
		{ "ds", si_module_static_ds, NULL },
#endif
		{ "mdns", si_module_static_mdns, NULL },
		{ NULL, NULL },
	};

	si_mod_t *si = NULL;
	int i;

	/*
	 * We don't need to worry about locking during initialization
	 * because all modules init routine returns static storage.
	 */

	/* Find the module by name */
	for (i = 0; modules[i].name != NULL; ++i)
	{
		if (string_equal(name, modules[i].name))
		{
			si = modules[i].module;
			if (si == NULL)
			{
				si = modules[i].init();
				modules[i].module = si;
			}

			break;
		}
	}

	if (si != NULL) return si;

	pthread_mutex_lock(&module_mutex);
	char *path = NULL;

	asprintf(&path, "%s/%s.%s", PLUGIN_DIR_PATH, name, PLUGIN_BUNDLE_SUFFIX);

	if (path == NULL)
	{
		errno = ENOMEM;
		pthread_mutex_unlock(&module_mutex);
		return NULL;
	}

	si = si_module_with_path(path, name);
	free(path);

	if (si == NULL)
	{
		pthread_mutex_unlock(&module_mutex);
		return NULL;
	}

	/* add out to module_list */
	module_list = (si_mod_t **)reallocf(module_list, (module_count + 1) * sizeof(si_mod_t *));
	if (module_list == NULL)
	{
		pthread_mutex_unlock(&module_mutex);
		return si;
	}

	module_list[module_count] = si;
	module_count++;

	pthread_mutex_unlock(&module_mutex);

	return si;
}

si_mod_t *
si_module_retain(si_mod_t *si)
{
	if (si == NULL) return NULL;
	if (si->flags & SI_MOD_FLAG_STATIC) return si;

	OSAtomicIncrement32Barrier(&si->refcount);

	return si;
}

void
si_module_release(si_mod_t *si)
{
	int32_t i;

	if (si == NULL) return;
	if (si->flags & SI_MOD_FLAG_STATIC) return;

	i = OSAtomicDecrement32Barrier(&si->refcount);
	if (i > 0) return;

	pthread_mutex_lock(&module_mutex);

	for (i = 0; (i < module_count) && (module_list[i] != si); i++);
	if (i >= module_count)
	{
		pthread_mutex_unlock(&module_mutex);
		return;
	}

	if (module_count == 1)
	{
		free(module_list);
		module_list = NULL;
		module_count = 0;
		pthread_mutex_unlock(&module_mutex);
		return;
	}

	for (i++; i < module_count; i++) module_list[i - 1] = module_list[i];
	module_count--;
	module_list = (si_mod_t **)reallocf(module_list, module_count * sizeof(si_mod_t *));
	if (module_list == NULL) module_count = 0;

	pthread_mutex_unlock(&module_mutex);

	if (si->vtable->sim_close != NULL) si->vtable->sim_close(si);
	if (si->bundle != NULL) dlclose(si->bundle);
	free(si->name);
	free(si);
}

const char *
si_module_name(si_mod_t *si)
{
	if (si == NULL) return NULL;

	return (const char *)si->name;
}

int
si_module_vers(si_mod_t *si)
{
	if (si == NULL) return 0;

	return si->vers;
}

int
si_item_match(si_item_t *item, int cat, const void *name, uint32_t num, int which)
{
	int i;
	union
	{
		char *x;
		struct passwd *u;
		struct group *g;
		struct grouplist_s *l;
		struct aliasent *a;
		struct hostent *h;
		struct netent *n;
		struct servent *s;
		struct protoent *p;
		struct rpcent *r;
		struct fstab *f;
		struct mac_s *m;
	} ent;

	if (item == NULL) return 0;
	if (which == SEL_ALL) return 1;
	if ((which == SEL_NAME) && (name == NULL)) return 0;

	ent.x = (char *)((uintptr_t)item + sizeof(si_item_t));

	switch (cat)
	{
		case CATEGORY_USER:
		{
			if ((which == SEL_NAME) && (string_equal(name, ent.u->pw_name))) return 1;
			else if ((which == SEL_NUMBER) && (num == (uint32_t)ent.u->pw_uid)) return 1;
			return 0;
		}
		case CATEGORY_GROUP:
		{
			if ((which == SEL_NAME) && (string_equal(name, ent.g->gr_name))) return 1;
			else if ((which == SEL_NUMBER) && (num == (uint32_t)ent.g->gr_gid)) return 1;
			return 0;
		}
		case CATEGORY_GROUPLIST:
		{
			if ((which == SEL_NAME) && (string_equal(name, ent.l->gl_user))) return 1;
			return 0;
		}
		case CATEGORY_ALIAS:
		{
			if ((which == SEL_NAME) && (string_equal(name, ent.a->alias_name))) return 1;
			return 0;
		}
		case CATEGORY_HOST_IPV4:
		case CATEGORY_HOST_IPV6:
		{
			/* N.B. address family is passed in num variable */
			if (ent.h->h_addrtype != num) return 0;
			if (which == SEL_NAME)
			{
				if (string_equal(name, ent.h->h_name)) return 1;
				if (ent.h->h_aliases != NULL)
				{
					for (i = 0; ent.h->h_aliases[i] != NULL; i++)
					{
						if (string_equal(name, ent.h->h_aliases[i])) return 1;
					}
				}
			}
			else if (which == SEL_NUMBER)
			{
				if (memcmp(name, ent.h->h_addr_list[0], ent.h->h_length) == 0) return 1;
			}
			return 0;
		}
		case CATEGORY_NETWORK:
		{
			if (which == SEL_NAME)
			{
				if (string_equal(name, ent.n->n_name)) return 1;
				if (ent.n->n_aliases != NULL)
				{
					for (i = 0; ent.n->n_aliases[i] != NULL; i++)
					{
						if (string_equal(name, ent.n->n_aliases[i])) return 1;
					}
				}
			}
			else if (which == SEL_NUMBER)
			{
				if (num == ent.n->n_net) return 1;
			}
			return 0;
		}
		case CATEGORY_SERVICE:
		{
			if (which == SEL_NAME)
			{
				/* N.B. for SEL_NAME, num is 0 (don't care), 1 (udp) or 2 (tcp) */
				if ((num == 1) && (string_not_equal("udp", ent.s->s_proto))) return 0;
				if ((num == 2) && (string_not_equal("tcp", ent.s->s_proto))) return 0;

				if (string_equal(name, ent.s->s_name)) return 1;
				if (ent.s->s_aliases != NULL)
				{
					for (i = 0; ent.s->s_aliases[i] != NULL; i++)
					{
						if (string_equal(name, ent.s->s_aliases[i])) return 1;
					}
				}
			}
			else if (which == SEL_NUMBER)
			{
				/* N.B. for SEL_NUMBER, protocol is sent in name variable */
				if ((name != NULL) && (string_not_equal(name, ent.s->s_proto))) return 0;
				if (num == ent.s->s_port) return 1;
			}
			return 0;
		}
		case CATEGORY_PROTOCOL:
		{
			if (which == SEL_NAME)
			{
				if (string_equal(name, ent.p->p_name)) return 1;
				if (ent.p->p_aliases != NULL)
				{
					for (i = 0; ent.p->p_aliases[i] != NULL; i++)
					{
						if (string_equal(name, ent.p->p_aliases[i])) return 1;
					}
				}
			}
			else if (which == SEL_NUMBER)
			{
				if (num == ent.p->p_proto) return 1;
			}
			return 0;
		}
		case CATEGORY_RPC:
		{
			if (which == SEL_NAME)
			{
				if (string_equal(name, ent.r->r_name)) return 1;
				if (ent.r->r_aliases != NULL)
				{
					for (i = 0; ent.r->r_aliases[i] != NULL; i++)
					{
						if (string_equal(name, ent.r->r_aliases[i])) return 1;
					}
				}
			}
			else if (which == SEL_NUMBER)
			{
				if (num == ent.r->r_number) return 1;
			}
			return 0;
		}
		case CATEGORY_FS:
		{
			if ((which == SEL_NAME) && (string_equal(name, ent.f->fs_spec))) return 1;
			if ((which == SEL_NUMBER) && (string_equal(name, ent.f->fs_file))) return 1;
			return 0;
		}
		case CATEGORY_MAC:
		{
			if ((which == SEL_NAME) && (string_equal(name, ent.m->host))) return 1;
			if ((which == SEL_NUMBER) && (string_equal(name, ent.m->mac))) return 1;
			return 0;
		}
		default: return 0;
	}

	return 0;
}

int
si_item_is_valid(si_item_t *item)
{
	si_mod_t *si;

	if (item == NULL) return 0;

	si = item->src;

	if (si == NULL) return 0;
	if (si->vtable->sim_is_valid == NULL) return 0;

	return si->vtable->sim_is_valid(si, item);
}

si_item_t *
si_user_byname(si_mod_t *si, const char *name)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_user_byname == NULL) return NULL;
	return si->vtable->sim_user_byname(si, name);
}

si_item_t *
si_user_byuid(si_mod_t *si, uid_t uid)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_user_byuid == NULL) return NULL;
	return si->vtable->sim_user_byuid(si, uid);
}

si_item_t *
si_user_byuuid(si_mod_t *si, uuid_t uuid)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_user_byuuid == NULL) return NULL;
	return si->vtable->sim_user_byuuid(si, uuid);
}

si_list_t *
si_user_all(si_mod_t *si)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_user_all == NULL) return NULL;
	return si->vtable->sim_user_all(si);
}

si_item_t *
si_group_byname(si_mod_t *si, const char *name)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_group_byname == NULL) return NULL;
	return si->vtable->sim_group_byname(si, name);
}

si_item_t *
si_group_bygid(si_mod_t *si, gid_t gid)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_group_bygid == NULL) return NULL;
	return si->vtable->sim_group_bygid(si, gid);
}

si_item_t *
si_group_byuuid(si_mod_t *si, uuid_t uuid)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_group_byuuid == NULL) return NULL;
	return si->vtable->sim_group_byuuid(si, uuid);
}

si_list_t *
si_group_all(si_mod_t *si)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_group_all == NULL) return NULL;
	return si->vtable->sim_group_all(si);
}

si_item_t *
si_grouplist(si_mod_t *si, const char *name, uint32_t count)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_grouplist == NULL) return NULL;
	return si->vtable->sim_grouplist(si, name, count);
}

si_list_t *
si_netgroup_byname(struct si_mod_s *si, const char *name)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_netgroup_byname == NULL) return NULL;
	return si->vtable->sim_netgroup_byname(si, name);
}

int
si_in_netgroup(struct si_mod_s *si, const char *name, const char *host, const char *user, const char *domain)
{
	if (si == NULL) return 0;
	if (si->vtable->sim_in_netgroup == NULL) return 0;
	return si->vtable->sim_in_netgroup(si, name, host, user, domain);
}

si_item_t *
si_alias_byname(si_mod_t *si, const char *name)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_alias_byname == NULL) return NULL;
	return si->vtable->sim_alias_byname(si, name);
}

si_list_t *
si_alias_all(si_mod_t *si)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_alias_all == NULL) return NULL;
	return si->vtable->sim_alias_all(si);
}

si_item_t *
si_host_byname(si_mod_t *si, const char *name, int af, const char *interface, uint32_t *err)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_host_byname == NULL) return NULL;
	return si->vtable->sim_host_byname(si, name, af, interface, err);
}

si_item_t *
si_host_byaddr(si_mod_t *si, const void *addr, int af, const char *interface, uint32_t *err)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_host_byaddr == NULL) return NULL;
	return si->vtable->sim_host_byaddr(si, addr, af, interface, err);
}

si_list_t *
si_host_all(si_mod_t *si)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_host_all == NULL) return NULL;
	return si->vtable->sim_host_all(si);
}

si_item_t *
si_mac_byname(struct si_mod_s *si, const char *name)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_mac_byname == NULL) return NULL;
	return si->vtable->sim_mac_byname(si, name);
}

si_item_t *
si_mac_bymac(struct si_mod_s *si, const char *mac)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_mac_bymac == NULL) return NULL;
	return si->vtable->sim_mac_bymac(si, mac);
}

si_list_t *
si_mac_all(si_mod_t *si)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_mac_all == NULL) return NULL;
	return si->vtable->sim_mac_all(si);
}

si_item_t *
si_network_byname(si_mod_t *si, const char *name)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_network_byname == NULL) return NULL;
	return si->vtable->sim_network_byname(si, name);
}

si_item_t *
si_network_byaddr(si_mod_t *si, uint32_t addr)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_network_byaddr == NULL) return NULL;
	return si->vtable->sim_network_byaddr(si, addr);
}

si_list_t *
si_network_all(si_mod_t *si)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_network_all == NULL) return NULL;
	return si->vtable->sim_network_all(si);
}

si_item_t *
si_service_byname(si_mod_t *si, const char *name, const char *proto)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_service_byname == NULL) return NULL;
	return si->vtable->sim_service_byname(si, name, proto);
}

si_item_t *
si_service_byport(si_mod_t *si, int port, const char *proto)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_service_byport == NULL) return NULL;
	return si->vtable->sim_service_byport(si, port, proto);
}

si_list_t *
si_service_all(si_mod_t *si)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_service_all == NULL) return NULL;
	return si->vtable->sim_service_all(si);
}

si_item_t *
si_protocol_byname(si_mod_t *si, const char *name)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_protocol_byname == NULL) return NULL;
	return si->vtable->sim_protocol_byname(si, name);
}

si_item_t *
si_protocol_bynumber(si_mod_t *si, uint32_t number)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_protocol_bynumber == NULL) return NULL;
	return si->vtable->sim_protocol_bynumber(si, number);
}

si_list_t *
si_protocol_all(si_mod_t *si)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_protocol_all == NULL) return NULL;
	return si->vtable->sim_protocol_all(si);
}

si_item_t *
si_rpc_byname(si_mod_t *si, const char *name)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_rpc_byname == NULL) return NULL;
	return si->vtable->sim_rpc_byname(si, name);
}

si_item_t *
si_rpc_bynumber(si_mod_t *si, int number)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_rpc_bynumber == NULL) return NULL;
	return si->vtable->sim_rpc_bynumber(si, number);
}

si_list_t *
si_rpc_all(si_mod_t *si)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_rpc_all == NULL) return NULL;
	return si->vtable->sim_rpc_all(si);
}

si_item_t *
si_fs_byspec(si_mod_t *si, const char *spec)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_fs_byspec == NULL) return NULL;
	return si->vtable->sim_fs_byspec(si, spec);
}

si_item_t *
si_fs_byfile(si_mod_t *si, const char *file)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_fs_byfile == NULL) return NULL;
	return si->vtable->sim_fs_byfile(si, file);
}

si_list_t *
si_fs_all(si_mod_t *si)
{
	if (si == NULL) return NULL;
	if (si->vtable->sim_fs_all == NULL) return NULL;
	return si->vtable->sim_fs_all(si);
}

si_item_t *
si_item_call(struct si_mod_s *si, int call, const char *str1, const char *str2, const char *str3, uint32_t num1, uint32_t num2, uint32_t *err)
{
	if (si == NULL) return NULL;

	switch (call)
	{
		case SI_CALL_USER_BYNAME: return si_user_byname(si, str1);
		case SI_CALL_USER_BYUID: return si_user_byuid(si, (uid_t)num1);
		case SI_CALL_GROUP_BYNAME: return si_group_byname(si, str1);
		case SI_CALL_GROUP_BYGID: return si_group_bygid(si, (gid_t)num1);
		case SI_CALL_GROUPLIST: return si_grouplist(si, str1, (int) num1);
		case SI_CALL_ALIAS_BYNAME: return si_alias_byname(si, str1);
		case SI_CALL_HOST_BYNAME: return si_host_byname(si, str1, num1, str3, err);
		case SI_CALL_HOST_BYADDR: return si_host_byaddr(si, (void *)str1, num1, str3, err);
		case SI_CALL_NETWORK_BYNAME: return si_network_byname(si, str1);
		case SI_CALL_NETWORK_BYADDR: return si_network_byaddr(si, num1);
		case SI_CALL_SERVICE_BYNAME: return si_service_byname(si, str1, str2);
		case SI_CALL_SERVICE_BYPORT: return si_service_byport(si, num1, str2);
		case SI_CALL_PROTOCOL_BYNAME: return si_protocol_byname(si, str1);
		case SI_CALL_PROTOCOL_BYNUMBER: return si_protocol_bynumber(si, num1);
		case SI_CALL_RPC_BYNAME: return si_network_byname(si, str1);
		case SI_CALL_RPC_BYNUMBER: return si_rpc_bynumber(si, num1);
		case SI_CALL_FS_BYSPEC: return si_fs_byspec(si, str1);
		case SI_CALL_FS_BYFILE: return si_fs_byfile(si, str1);
		case SI_CALL_NAMEINFO: return si_nameinfo(si, (const struct sockaddr *)str1, num1, str3, err);
		case SI_CALL_IPNODE_BYNAME: return si_ipnode_byname(si, (const char *)str1, num1, num2, str3, err);
		case SI_CALL_MAC_BYNAME: return si_mac_byname(si, (const char *)str1);
		case SI_CALL_MAC_BYMAC: return si_mac_bymac(si, (const char *)str1);

		/* Support for DNS async calls */
		case SI_CALL_DNS_QUERY:
		case SI_CALL_DNS_SEARCH:
		{
			if (si->vtable->sim_item_call == NULL) return NULL;
			return si->vtable->sim_item_call(si, call, str1, str2, str3, num1, num2, err);
		}

		default: return NULL;
	}

	return NULL;
}

si_list_t *
si_list_call(struct si_mod_s *si, int call, const char *str1, const char *str2, const char *str3, uint32_t num1, uint32_t num2, uint32_t num3, uint32_t num4, uint32_t *err)
{
	if (si == NULL) return NULL;

	switch (call)
	{
		case SI_CALL_USER_ALL: return si_user_all(si);
		case SI_CALL_GROUP_ALL: return si_group_all(si);
		case SI_CALL_ALIAS_ALL: return si_alias_all(si);
		case SI_CALL_HOST_ALL: return si_host_all(si);
		case SI_CALL_NETWORK_ALL: return si_network_all(si);
		case SI_CALL_SERVICE_ALL: return si_service_all(si);
		case SI_CALL_PROTOCOL_ALL: return si_protocol_all(si);
		case SI_CALL_RPC_ALL: return si_rpc_all(si);
		case SI_CALL_FS_ALL: return si_fs_all(si);
		case SI_CALL_MAC_ALL: return si_mac_all(si);
		case SI_CALL_ADDRINFO: return si_addrinfo(si, str1, str2, num1, num2, num3, num4, str3, err);
		default: return NULL;
	}

	return NULL;
}

static void
si_async_worklist_add_unit(si_async_workunit_t *r)
{
	pthread_mutex_lock(&module_mutex);
	r->next = si_async_worklist;
	si_async_worklist = r;
	pthread_mutex_unlock(&module_mutex);
}

static void
si_async_worklist_remove_unit(si_async_workunit_t *r)
{
	si_async_workunit_t *x;

	pthread_mutex_lock(&module_mutex);
	if (si_async_worklist == r)
	{
		si_async_worklist = r->next;
	}
	else
	{
		for (x = si_async_worklist; (x != NULL) && (x->next != r); x = x->next) {;}
		if (x != NULL) x->next = r->next;
	}
	pthread_mutex_unlock(&module_mutex);
}

static si_async_workunit_t*
si_async_worklist_find_unit(mach_port_t p)
{
	si_async_workunit_t *r;

	pthread_mutex_lock(&module_mutex);
	for (r = si_async_worklist; (r != NULL) && (r->port != p); r = r->next) {;}
	pthread_mutex_unlock(&module_mutex);

	return r;
}

static si_async_workunit_t *
si_async_workunit_create(si_mod_t *si, int call, const char *str1, const char *str2, const char *str3, uint32_t num1, uint32_t num2, uint32_t num3, uint32_t num4, void *callback, void *context)
{
	si_async_workunit_t *r;
	kern_return_t status;
	mach_port_t reply, send;
	mach_msg_type_name_t type;
	char *s1, *s2, *s3;

	s1 = NULL;
	s2 = NULL;
	s3 = NULL;

	if (si_call_str1_is_buffer(call))
	{
		if (num3 > 0)
		{
			s1 = calloc(1, num3);
			if (s1 == NULL) return NULL;
			memcpy(s1, str1, num3);
		}
	}
	else if (str1 != NULL)
	{
		s1 = strdup(str1);
		if (s1 == NULL) return NULL;
	}

	if (str2 != NULL)
	{
		s2 = strdup(str2);
		if (s2 == NULL)
		{
			if (s1 != NULL) free(s1);
			return NULL;
		}
	}

	if (str3 != NULL)
	{
		s3 = strdup(str3);
		if (s3 == NULL)
		{
			if (s1 != NULL) free(s1);
			if (s2 != NULL) free(s2);
			return NULL;
		}
	}

	r = (si_async_workunit_t *)calloc(1, sizeof(si_async_workunit_t));
	if (r == NULL)
	{
		if (s1 != NULL) free(s1);
		if (s2 != NULL) free(s2);
		if (s3 != NULL) free(s3);
		return NULL;
	}

	reply = MACH_PORT_NULL;
	send = MACH_PORT_NULL;
	type = 0;

	status = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &reply);
	if (status == KERN_SUCCESS) status = mach_port_extract_right(mach_task_self(), reply, MACH_MSG_TYPE_MAKE_SEND_ONCE, &send, &type);
	if (status != KERN_SUCCESS)
	{
		if (reply != MACH_PORT_NULL) mach_port_mod_refs(mach_task_self(), reply, MACH_PORT_RIGHT_RECEIVE, -1);
		if (s1 != NULL) free(s1);
		if (s2 != NULL) free(s2);
		if (s3 != NULL) free(s3);
		free(r);
		return NULL;
	}

	r->si = si;
	r->call = call;
	r->str1 = s1;
	r->str2 = s2;
	r->str3 = s3;
	r->num1 = num1;
	r->num2 = num2;
	r->num3 = num3;
	r->num4 = num4;

	r->refcount = 2;
	r->flags = 0;
	if (si_call_returns_list(call)) r->flags |= WORKUNIT_RETURNS_LIST;

	r->callback = callback;
	r->context = context;
	r->port = reply;
	r->send = send;

	si_async_worklist_add_unit(r);

	return r;
}

static void
si_async_workunit_release(si_async_workunit_t *r)
{
	if (r == NULL) return;

	if (OSAtomicDecrement32Barrier(&(r->refcount)) != 0) return;

#ifdef CALL_TRACE
	fprintf(stderr, "** %s freeing worklist item %p\n", __func__, r);
#endif

	si_async_worklist_remove_unit(r);

	if (r->resitem != NULL) si_item_release(r->resitem);
	if (r->reslist != NULL) si_list_release(r->reslist);

	if (r->str1 != NULL) free(r->str1);
	if (r->str2 != NULL) free(r->str2);
	if (r->str3 != NULL) free(r->str3);

	/* release send-once right if it has not been used */
	if (r->send != MACH_PORT_NULL) mach_port_deallocate(mach_task_self(), r->send);

	/* release receive right */
	mach_port_mod_refs(mach_task_self(), r->port, MACH_PORT_RIGHT_RECEIVE, -1);

	free(r);
}

static void
si_async_launchpad(si_async_workunit_t *r)
{
	kern_return_t status;
	mach_msg_empty_send_t msg;

#ifdef CALL_TRACE
	fprintf(stderr, "** %s starting worklist item %p\n", __func__, r);
#endif

	if (r->flags & WORKUNIT_CANCELLED)
	{
		si_async_workunit_release(r);
#ifdef CALL_TRACE
		fprintf(stderr, "** %s worklist item %p was cancelled early\n", __func__, r);
#endif
		return;
	}

	if (r->flags & WORKUNIT_RETURNS_LIST) r->reslist = si_list_call(r->si, r->call, r->str1, r->str2, r->str3, r->num1, r->num2, r->num3, r->num4, &(r->err));
	else r->resitem = si_item_call(r->si, r->call, r->str1, r->str2, r->str3, r->num1, r->num2, &(r->err));

	/*
	 * Test and set the cancelled flag.
	 * If it was set, then this work item was cancelled.
	 * Otherwise, setting it here prevents si_async_cancel from cancelling:
	 * too late to cancel now!
	 */
	if (OSAtomicTestAndSetBarrier(WORKUNIT_CANCELLED_BIT_ADDRESS, &(r->flags)) == 1)
	{
		si_async_workunit_release(r);
#ifdef CALL_TRACE
		fprintf(stderr, "** %s worklist item %p was cancelled in flight\n", __func__, r);
#endif
		return;
	}
#ifdef CALL_TRACE
	else fprintf(stderr, "** %s worklist item %p flags are now 0x%08x\n", __func__, r, r->flags);
#endif

	memset(&msg, 0, sizeof(mach_msg_empty_send_t));

	msg.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_MOVE_SEND_ONCE, MACH_MSGH_BITS_ZERO);
	msg.header.msgh_remote_port = r->send;
	r->send = MACH_PORT_NULL;
	msg.header.msgh_local_port = MACH_PORT_NULL;
	msg.header.msgh_size = sizeof(mach_msg_empty_send_t);
	msg.header.msgh_id = r->call;

	status = mach_msg(&(msg.header), MACH_SEND_MSG, msg.header.msgh_size, 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
	if (status != MACH_MSG_SUCCESS)
	{
		/* receiver failed - clean up to avoid a port leak */
		mach_msg_destroy(&(msg.header));
#ifdef CALL_TRACE
		fprintf(stderr, "** %s mach message send failed for worklist item %p\n", __func__, r);
#endif
	}

	si_async_workunit_release(r);

	/*
	 * The client is now responsible for calling si_async_handle_reply,
	 * which will invoke the client's callback and then release the workunit.
	 */

#ifdef CALL_TRACE
	fprintf(stderr, "** %s completed async worklist item %p\n", __func__, r);
#endif
}

mach_port_t
si_async_call(struct si_mod_s *si, int call, const char *str1, const char *str2, const char *str3, uint32_t num1, uint32_t num2, uint32_t num3, uint32_t num4, void *callback, void *context)
{
	si_async_workunit_t *req;

	if (si == NULL) return MACH_PORT_NULL;
	if (callback == NULL) return MACH_PORT_NULL;

	/* if module does async on it's own, hand off the call */
	if (si->vtable->sim_async_call != NULL)
	{
		return si->vtable->sim_async_call(si, call, str1, str2, str3, num1, num2, num3, num4, callback, context);
	}

	req = si_async_workunit_create(si, call, str1, str2, str3, num1, num2, num3, num4, callback, context);
	if (req == NULL) return MACH_PORT_NULL;

	/* queue the work on the global low-priority dispatch queue */
#ifdef CALL_TRACE
	fprintf(stderr, "** %s dispatching worklist item %p\n", __func__, req);
#endif
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, DISPATCH_QUEUE_OVERCOMMIT), ^{ si_async_launchpad(req); });

	return req->port;
}

void
si_async_cancel(mach_port_t p)
{
	si_async_workunit_t *r;

	r = si_async_worklist_find_unit(p);
	if (r == NULL)
	{
#ifdef CALL_TRACE
		fprintf(stderr, "** %s can't find worklist item\n", __func__);
#endif
		return;
	}

	/*
	 * Test and set the WORKUNIT_CANCELLED flag.
	 * If it was already set, this work item has been executed - too late to really cancel.
	 */
	if (OSAtomicTestAndSetBarrier(WORKUNIT_CANCELLED_BIT_ADDRESS, &(r->flags)) == 1)
	{
		/* already executed */
#ifdef CALL_TRACE
		fprintf(stderr, "** %s worklist item %p has executed\n", __func__, r);
#endif
	}

#ifdef CALL_TRACE
	fprintf(stderr, "** %s calling worklist item %p callback SI_STATUS_CALL_CANCELLED\n", __func__, r);
#endif

	if (r->callback != NULL)
	{
		if (r->flags & WORKUNIT_RETURNS_LIST) ((list_async_callback)(r->callback))(NULL, SI_STATUS_CALL_CANCELLED, r->context);
		else ((item_async_callback)(r->callback))(NULL, SI_STATUS_CALL_CANCELLED, r->context);
	}

	si_async_workunit_release(r);
}

void
si_async_handle_reply(mach_msg_header_t *msg)
{
	si_async_workunit_t *r;
	mach_port_t reply = msg->msgh_local_port;

	r = si_async_worklist_find_unit(reply);
	if (r == NULL)
	{
#ifdef CALL_TRACE
		fprintf(stderr, "** %s can't find worklist item\n", __func__);
#endif
		return;
	}

#ifdef CALL_TRACE
	fprintf(stderr, "** %s worklist item %p flags are 0x%08x\n", __func__, r, r->flags);
#endif
	if ((r->flags & WORKUNIT_CANCELLED) == 0)
	{
#ifdef CALL_TRACE
		fprintf(stderr, "** %s workunit thread is still active\n", __func__);
#endif
		return;
	}

	if (r->callback != NULL)
	{
		if (r->flags & WORKUNIT_RETURNS_LIST) ((list_async_callback)(r->callback))(r->reslist, r->err, r->context);
		else ((item_async_callback)(r->callback))(r->resitem, r->err, r->context);

		r->reslist = NULL;
		r->resitem = NULL;
	}
	else
	{
#ifdef CALL_TRACE
		fprintf(stderr, "** %s workunit has no callback\n", __func__);
#endif
	}

	si_async_workunit_release(r);
}

char *
si_standardize_mac_address(const char *addr)
{
	char e[6][3];
	char *out;
	struct ether_addr *ether;
	int i;

	if (addr == NULL) return NULL;

	/* ether_aton isn't thread-safe */
	pthread_mutex_lock(&module_mutex);

	ether = ether_aton(addr);
	if (ether == NULL)
	{
		pthread_mutex_unlock(&module_mutex);
		return NULL;
	}

	for (i = 0; i < 6; i++)
	{
		if (ether->ether_addr_octet[i] <= 15)
		{
			sprintf(e[i], "0%x", ether->ether_addr_octet[i]);
		}
		else
		{
			sprintf(e[i], "%x", ether->ether_addr_octet[i]);
		}
	}

	pthread_mutex_unlock(&module_mutex);

	out = NULL;
	asprintf(&out, "%s:%s:%s:%s:%s:%s", e[0], e[1], e[2], e[3], e[4], e[5]);
	return out;
}

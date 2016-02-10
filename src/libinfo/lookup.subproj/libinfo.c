/*
 * Copyright (c) 2008-2013 Apple Inc.  All rights reserved.
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
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <asl.h>
#include <printerdb.h>
#include <sys/param.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include "si_module.h"
#include "libinfo.h"
#include <thread_data.h>
#include <sys/kauth.h>
#include "netdb_async.h"
#include <dispatch/dispatch.h>
#include <mach-o/dyld_priv.h>

#define SOCK_UNSPEC 0
#define IPPROTO_UNSPEC 0
#define IPV6_ADDR_LEN 16
#define IPV4_ADDR_LEN 4

/* kernel syscalls */
extern int __initgroups(u_int gidsetsize, gid_t *gidset, int gmuid);

/* SPI from long ago */
int _proto_stayopen;

extern struct addrinfo *si_list_to_addrinfo(si_list_t *list);
extern int getnameinfo_link(const struct sockaddr *sa, socklen_t salen, char *host, size_t hostlen, char *serv, size_t servlen, int flags);
__private_extern__ void search_set_flags(si_mod_t *si, const char *name, uint32_t flag);

/*
 * Impedence matching for async calls.
 *
 * This layer holds on to the caller's callback and context in this
 * structure, which gets passed to the si_module async routines along
 * with a callbac in this layer.  When this layer gets a callback,
 * it can save the item or list in thread-specific memory and then
 * invoke the caller's callback with the appropriate data type.
 */

typedef struct
{
	void *orig_callback;
	void *orig_context;
	uint32_t cat;
	int32_t key_offset;
} si_context_t;

si_mod_t *
si_search(void)
{
	static si_mod_t *search = NULL;

	if (search == NULL) search = si_module_with_name("search");

	return search;
}

void
si_search_module_set_flags(const char *name, uint32_t flag)
{
	search_set_flags(si_search(), name, flag);
}

static void
si_libinfo_general_callback(si_item_t *item, uint32_t status, void *ctx)
{
	si_context_t *sictx;
	union
	{
		char *x;
		struct passwd *u;
		struct group *g;
		struct grouplist_s *l;
		struct hostent *h;
		struct netent *n;
		struct servent *s;
		struct protoent *p;
		struct rpcent *r;
		struct fstab *f;
	} res;

	if (ctx == NULL) return;

	sictx = (si_context_t *)ctx;

	if ((sictx->orig_callback == NULL) || (status == SI_STATUS_CALL_CANCELLED))
	{
		free(sictx);
		si_item_release(item);
		return;
	}

	if (sictx->key_offset >= 0)
	{
		LI_set_thread_item(sictx->cat + sictx->key_offset, item);
	}

	res.x = NULL;
	if (item != NULL) res.x = (char*)((uintptr_t)item + sizeof(si_item_t));

	switch (sictx->cat)
	{
		case CATEGORY_USER:
		{
			((si_user_async_callback)(sictx->orig_callback))(res.u, sictx->orig_context);
			break;
		}
		case CATEGORY_GROUP:
		{
			((si_group_async_callback)(sictx->orig_callback))(res.g, sictx->orig_context);
			break;
		}
		case CATEGORY_GROUPLIST:
		{
			((si_grouplist_async_callback)(sictx->orig_callback))(res.l, sictx->orig_context);
			break;
		}
		case CATEGORY_HOST_IPV4:
		case CATEGORY_HOST_IPV6:
		{
			((si_host_async_callback)(sictx->orig_callback))(res.h, sictx->orig_context);
			break;
		}
		case CATEGORY_NETWORK:
		{
			((si_network_async_callback)(sictx->orig_callback))(res.n, sictx->orig_context);
			break;
		}
		case CATEGORY_SERVICE:
		{
			((si_service_async_callback)(sictx->orig_callback))(res.s, sictx->orig_context);
			break;
		}
		case CATEGORY_PROTOCOL:
		{
			((si_protocol_async_callback)(sictx->orig_callback))(res.p, sictx->orig_context);
			break;
		}
		case CATEGORY_RPC:
		{
			((si_rpc_async_callback)(sictx->orig_callback))(res.r, sictx->orig_context);
			break;
		}
		case CATEGORY_FS:
		{
			((si_fs_async_callback)(sictx->orig_callback))(res.f, sictx->orig_context);
			break;
		}
	}

	free(sictx);
}

/* USER */

struct passwd *
getpwnam(const char *name)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, name);
#endif

	item = si_user_byname(si_search(), name);
	LI_set_thread_item(CATEGORY_USER + 100, item);

	if (item == NULL) return NULL;
	return (struct passwd *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
getpwnam_async_call(const char *name, si_user_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %s\n", __func__, name);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_USER;
	sictx->key_offset = 100;

	return si_async_call(si_search(), SI_CALL_USER_BYNAME, name, NULL, NULL, 0, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
getpwnam_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

struct passwd *
getpwuid(uid_t uid)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %d\n", __func__, uid);
#endif

	item = si_user_byuid(si_search(), uid);
	LI_set_thread_item(CATEGORY_USER + 200, item);

	if (item == NULL) return NULL;
	return (struct passwd *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
getpwuid_async_call(uid_t uid, si_user_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %d\n", __func__, uid);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_USER;
	sictx->key_offset = 200;

	return si_async_call(si_search(), SI_CALL_USER_BYUID, NULL, NULL, NULL, (uint32_t)uid, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
getpwuid_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

struct passwd *
getpwuuid(uuid_t uuid)
{
	si_item_t *item;

#ifdef CALL_TRACE
	uuid_string_t uuidstr;
	uuid_unparse_upper(uuid, uuidstr);
	fprintf(stderr, "-> %s %s\n", __func__, uuidstr);
#endif

	item = si_user_byuuid(si_search(), uuid);
	LI_set_thread_item(CATEGORY_USER + 300, item);

	if (item == NULL) return NULL;
	return (struct passwd *)((uintptr_t)item + sizeof(si_item_t));
}

void
setpwent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_USER, NULL);
}

struct passwd *
getpwent(void)
{
	si_list_t *list;
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s\n", __func__);
#endif

	list = LI_get_thread_list(CATEGORY_USER);
	if (list == NULL)
	{
		list = si_user_all(si_search());
		LI_set_thread_list(CATEGORY_USER, list);
	}

	item = si_list_next(list);
	if (item == NULL) return NULL;

	return (struct passwd *)((uintptr_t)item + sizeof(si_item_t));
}

void
endpwent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_USER, NULL);
}

int
setpassent(int ignored)
{
	si_list_t *list;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s\n", __func__);
#endif

	list = LI_get_thread_list(CATEGORY_USER);
	si_list_reset(list);

	if (list == NULL) return 0;
	return 1;
}

/* GROUP */

struct group *
getgrnam(const char *name)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, name);
#endif

	item = si_group_byname(si_search(), name);
	LI_set_thread_item(CATEGORY_GROUP + 100, item);

	if (item == NULL) return NULL;
	return (struct group *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
getgrnam_async_call(const char *name, si_group_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %s\n", __func__, name);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_GROUP;
	sictx->key_offset = 100;

	return si_async_call(si_search(), SI_CALL_GROUP_BYNAME, name, NULL, NULL, 0, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
getgrnam_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

struct group *
getgrgid(gid_t gid)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %d\n", __func__, gid);
#endif

	item = si_group_bygid(si_search(), gid);
	LI_set_thread_item(CATEGORY_GROUP + 200, item);

	if (item == NULL) return NULL;
	return (struct group *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
getgrgid_async_call(gid_t gid, si_group_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %d\n", __func__, gid);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_GROUP;
	sictx->key_offset = 200;

	return si_async_call(si_search(), SI_CALL_GROUP_BYGID, NULL, NULL, NULL, (uint32_t)gid, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
getgruid_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

struct group *
getgruuid(uuid_t uuid)
{
	si_item_t *item;

#ifdef CALL_TRACE
	uuid_string_t uuidstr;
	uuid_unparse_upper(uuid, uuidstr);
	fprintf(stderr, "-> %s %s\n", __func__, uuidstr);
#endif

	item = si_group_byuuid(si_search(), uuid);
	LI_set_thread_item(CATEGORY_GROUP + 300, item);

	if (item == NULL) return NULL;
	return (struct group *)((uintptr_t)item + sizeof(si_item_t));
}

void
setgrent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_GROUP, NULL);
}

struct group *
getgrent(void)
{
	si_list_t *list;
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s\n", __func__);
#endif

	list = LI_get_thread_list(CATEGORY_GROUP);
	if (list == NULL)
	{
		list = si_group_all(si_search());
		LI_set_thread_list(CATEGORY_GROUP, list);
	}

	item = si_list_next(list);
	if (item == NULL) return NULL;

	return (struct group *)((uintptr_t)item + sizeof(si_item_t));
}

void
endgrent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_GROUP, NULL);
}

int
setgroupent(int ignored)
{
	si_list_t *list;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s\n", __func__);
#endif

	list = LI_get_thread_list(CATEGORY_GROUP);
	si_list_reset(list);

	if (list == NULL) return 0;
	return 1;
}

/* NETGROUP */
int 
innetgr(const char *group, const char *host, const char *user, const char *domain)
{
	int res;
#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s %s %s %s\n", __func__, group, host, user, domain);
#endif

	res = si_in_netgroup(si_search(), group, host, user, domain);

#ifdef CALL_TRACE
	fprintf(stderr, "<- %s %d\n", __func__, res);
#endif

	return res;
}

/* N.B. there is no async innetgr */

/*
 * setnetgrent is really more like a getXXXbyname routine than a
 * setXXXent routine, since we are looking up a netgroup by name.
 */
void
setnetgrent(const char *name)
{
	si_list_t *list;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, name);
#endif

	list = si_netgroup_byname(si_search(), name);
	LI_set_thread_list(CATEGORY_NETGROUP, list);
}

/* N.B. there is no async getnetgrent */

int
getnetgrent(char **host, char **user, char **domain)
{
	si_list_t *list;
	si_item_t *item;
	struct netgrent_s *ng;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s\n", __func__);
#endif

	list = LI_get_thread_list(CATEGORY_NETGROUP);
	item = si_list_next(list);
	if (item == NULL) return 0;

	ng = (struct netgrent_s *)((uintptr_t)item + sizeof(si_item_t));

	*host = ng->ng_host;
	*user = ng->ng_user;
	*domain = ng->ng_domain;

	return 1;
}

void
endnetgrent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_NETGROUP, NULL);
}

#if DS_AVAILABLE
static void
_check_groups(const char *function, int32_t ngroups)
{
	static dispatch_once_t once;

	if (ngroups > 0 && ngroups < NGROUPS_MAX) {
		return;
	}

	/* only log once per process */
	dispatch_once(&once, ^(void) {
		const char *proc_name = getprogname();
		if (strcmp(proc_name, "id") != 0 && strcmp(proc_name, "smbd") != 0 && strcmp(proc_name, "rpcsvchost") != 0) {
			aslmsg msg = asl_new(ASL_TYPE_MSG);
			char buffer[256];

			snprintf(buffer, sizeof(buffer), "%d", (ngroups == 0 ? INT_MAX : ngroups));
			asl_set(msg, "com.apple.message.value", buffer);

			asl_set(msg, "com.apple.message.domain", "com.apple.system.libinfo");
			asl_set(msg, "com.apple.message.result", "noop");
			asl_set(msg, "com.apple.message.signature", function);

			asl_log(NULL, msg, ASL_LEVEL_NOTICE, "%s called triggering group enumeration", function);

			asl_free(msg);
		}
	});
}
#endif

/* GROUPLIST */

static int
getgrouplist_internal(const char *name, int basegid, gid_t *groups, uint32_t *ngroups)
{
	int i, j, x, g, add, max;
	si_item_t *item;
	si_grouplist_t *gl;

	/*
	 * On input, ngroups specifies the size of the groups array.
	 * On output, it is set to the number of groups that are being returned.
	 * Returns -1 if the size is too small to fit all the groups that were found.
	 */

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s %d\n", __func__, name, basegid);
#endif

	if (name == NULL) return 0;
	if (groups == NULL) return 0;
	if (ngroups == NULL) return 0;

	max = (*ngroups);
	*ngroups = 0;
	if (max <= 0) return 0;

	groups[0] = basegid;
	*ngroups = 1;

	item = si_grouplist(si_search(), name, max);
	LI_set_thread_item(CATEGORY_GROUPLIST, item);
	if (item == NULL) return 0;

	gl = (si_grouplist_t *)((uintptr_t)item + sizeof(si_item_t));

	x = 1;

	for (i = 0; i < gl->gl_count; i++)
	{
		g = gl->gl_gid[i];
		add = 1;
		for (j = 0; j < x; j++) {
			if (groups[j] == g) {
				add = 0;
				break;
			}
		}
		if (add == 0) continue;

		if (x >= max) return -1;
		groups[x] = g;
		x++;
		*ngroups = x;
	}

	return 0;
}

int
getgrouplist(const char *name, int basegid, int *groups, int *ngroups)
{
#if DS_AVAILABLE
	_check_groups("getgrouplist", *ngroups);
#endif

	return getgrouplist_internal(name, basegid, (gid_t *)groups, (uint32_t *)ngroups);
}

static void
merge_gid(gid_t *list, gid_t g, int32_t *count)
{
	int32_t cnt;
	int i;

	cnt = (*count);
	for (i = 0; i < cnt; i++) {
		if (list[i] == g) return;
	}

	list[cnt] = g;
	(*count)++;
}

static int32_t
_getgrouplist_2_internal(const char *name, gid_t basegid, gid_t **groups)
{
	int32_t i, count;
	si_item_t *item;
	gid_t *gids;
	si_grouplist_t *gl;

	item = si_grouplist(si_search(), name, INT_MAX);
	LI_set_thread_item(CATEGORY_GROUPLIST, item);
	if (item == NULL) return -1;

	gl = (si_grouplist_t *) ((uintptr_t) item + sizeof(si_item_t));

	/*
	 * we can allocate enough up-front, we'll only use what we need
	 * we add one to the count that was found in case the basegid is not there
	 */
	gids = calloc(gl->gl_count + 1, sizeof(gid_t));

	count = 0;
	merge_gid(gids, basegid, &count);
	if (gl->gl_gid != NULL) {
		for (i = 0; i < gl->gl_count; i++) {
			merge_gid(gids, gl->gl_gid[i], &count);
		}
	}

	(*groups) = gids;

	return count;
}

int32_t
getgrouplist_2(const char *name, gid_t basegid, gid_t **groups)
{
	/*
	 * Passes back a gid_t list containing all the users groups (and basegid).
	 * Caller must free the list.
	 * Returns the number of gids in the list or -1 on failure.
	 */

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s %d\n", __func__, name, basegid);
#endif

	if (name == NULL) return 0;
	if (groups == NULL) return 0;

#if DS_AVAILABLE
	_check_groups("getgrouplist_2", INT_MAX);
#endif

	return _getgrouplist_2_internal(name, basegid, groups);
}

int32_t
getgroupcount(const char *name, gid_t basegid)
{
	int32_t count;
	gid_t *groups;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s %d\n", __func__, name, basegid);
#endif

#if DS_AVAILABLE
	_check_groups("getgroupcount", INT_MAX);
#endif

	groups = NULL;
	count = _getgrouplist_2_internal(name, basegid, &groups);
	if (groups != NULL) free(groups);

	return count;
}

/* XXX to do: async getgrouplist_2 */

int
initgroups(const char *name, int basegid)
{
	int status;
	uint32_t ngroups;
	gid_t groups[NGROUPS];
	uid_t uid;
#ifdef DS_AVAILABLE
	si_item_t *item;
	struct passwd *p;
#endif

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s %d\n", __func__, name, basegid);
#endif

	/* KAUTH_UID_NONE tells the kernel not to fetch supplementary groups from DirectoryService */
	uid = KAUTH_UID_NONE;

#ifdef DS_AVAILABLE
	/* get the UID for this user */
	item = si_user_byname(si_search(), name);
	if (item != NULL)
	{
		p = (struct passwd *)((uintptr_t)item + sizeof(si_item_t));
		uid = p->pw_uid;
		si_item_release(item);
	}
#endif

	ngroups = NGROUPS;

	/*
	 * Ignore status.
	 * A failure either means that user belongs to more than NGROUPS groups
	 * or no groups at all.
	 */

	(void) getgrouplist_internal(name, basegid, groups, &ngroups);

	status = __initgroups(ngroups, groups, uid);
	if (status < 0) return -1;

	return 0;
}

/* ALIAS */

struct aliasent *
alias_getbyname(const char *name)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, name);
#endif

	item = si_alias_byname(si_search(), name);
	LI_set_thread_item(CATEGORY_ALIAS + 100, item);
	if (item == NULL) return NULL;

	return (struct aliasent *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
alias_getbyname_async_call(const char *name, si_alias_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %s\n", __func__, name);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_ALIAS;
	sictx->key_offset = 100;

	return si_async_call(si_search(), SI_CALL_ALIAS_BYNAME, name, NULL, NULL, 0, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
alias_getbyname_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

void
alias_setent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-> %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_ALIAS, NULL);
}

struct aliasent *
alias_getent(void)
{
	si_list_t *list;
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s\n", __func__);
#endif

	list = LI_get_thread_list(CATEGORY_ALIAS);
	if (list == NULL)
	{
		list = si_alias_all(si_search());
		LI_set_thread_list(CATEGORY_ALIAS, list);
	}

	item = si_list_next(list);
	if (item == NULL) return NULL;

	return (struct aliasent *)((uintptr_t)item + sizeof(si_item_t));
}

void
alias_endent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_ALIAS, NULL);
}

/* HOST */

void
freehostent(struct hostent *h)
{
	if (h == NULL) return;

	si_item_t *item = (si_item_t *)((uintptr_t)h - sizeof(si_item_t));
	si_item_release(item);
}

struct hostent *
gethostbynameerrno(const char *name, int *err)
{
	si_item_t *item;
	uint32_t status;
	struct in_addr addr4;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, name);
#endif

	memset(&addr4, 0, sizeof(struct in_addr));
	status = SI_STATUS_NO_ERROR;
	item = NULL;

	if (inet_aton(name, &addr4) == 1) item = si_ipnode_byname(si_search(), name, AF_INET, 0, NULL, &status);
	else item = si_host_byname(si_search(), name, AF_INET, NULL, &status);

	if (status >= SI_STATUS_INTERNAL) status = NO_RECOVERY;
	if (err != NULL) *err = status;

	LI_set_thread_item(CATEGORY_HOST + 100, item);
	if (item == NULL) return NULL;

	return (struct hostent *)((uintptr_t)item + sizeof(si_item_t));
}

struct hostent *
gethostbyname(const char *name)
{
	si_item_t *item;
	uint32_t status;
	struct in_addr addr4;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, name);
#endif

	memset(&addr4, 0, sizeof(struct in_addr));
	status = SI_STATUS_NO_ERROR;
	item = NULL;

	if (inet_aton(name, &addr4) == 1) item = si_ipnode_byname(si_search(), name, AF_INET, 0, NULL, &status);
	else item = si_host_byname(si_search(), name, AF_INET, NULL, &status);

	if (status >= SI_STATUS_INTERNAL) status = NO_RECOVERY;
	h_errno = status;

	LI_set_thread_item(CATEGORY_HOST + 100, item);
	if (item == NULL) return NULL;

	return (struct hostent *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
gethostbyname_async_call(const char *name, si_host_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %s\n", __func__, name);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_HOST;
	sictx->key_offset = 100;

	return si_async_call(si_search(), SI_CALL_HOST_BYNAME, name, NULL, NULL, AF_INET, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

mach_port_t
gethostbyname_async_start(const char *name, si_host_async_callback callback, void *context)
{
	return gethostbyname_async_call(name, callback, context);
}

void
gethostbyname_async_cancel(mach_port_t p)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	si_async_cancel(p);
}

#if 0
void
gethostbyname_async_handle_reply(void *param)
{
	mach_msg_header_t *msg;

#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	msg = (mach_msg_header_t *)param;
	si_async_handle_reply(msg);
}
#endif

void
gethostbyname_async_handleReply(void *param)
{
	mach_msg_header_t *msg;

#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	msg = (mach_msg_header_t *)param;
	si_async_handle_reply(msg);
}

struct hostent *
gethostbyname2(const char *name, int af)
{
	si_item_t *item;
	uint32_t status;
	struct in_addr addr4;
	struct in6_addr addr6;
	si_mod_t *search = si_search();

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s %d\n", __func__, name, af);
#endif

	memset(&addr4, 0, sizeof(struct in_addr));
	memset(&addr6, 0, sizeof(struct in6_addr));
	status = SI_STATUS_NO_ERROR;
	item = NULL;

	if (((af == AF_INET) && (inet_aton(name, &addr4) == 1)) || ((af == AF_INET6) && (inet_pton(af, name, &addr6) == 1)))
	{
		item = si_ipnode_byname(search, name, (uint32_t)af, 0, NULL, &status);
	}
	else
	{
		item = si_host_byname(search, name, (uint32_t)af, NULL, &status);
	}

	if (status >= SI_STATUS_INTERNAL) status = NO_RECOVERY;
	h_errno = status;

	LI_set_thread_item(CATEGORY_HOST + 100, item);
	if (item == NULL) return NULL;

	return (struct hostent *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
gethostbyname2_async_call(const char *name, int af, si_group_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %s %d\n", __func__, name, af);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_HOST;
	sictx->key_offset = 100;

	return si_async_call(si_search(), SI_CALL_HOST_BYNAME, name, NULL, NULL, (uint32_t)af, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
gethostbyname2_async_cancel(mach_port_t p)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	si_async_cancel(p);
}

void
gethostbyname2_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

struct hostent *
gethostbyaddr(const void *addr, socklen_t len, int type)
{
	si_item_t *item;
	uint32_t status;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, (type == AF_INET) ? inet_ntoa(*(struct in_addr *)addr) : "-IPv6-");
#endif

	status = SI_STATUS_NO_ERROR;

	item = si_host_byaddr(si_search(), addr, (uint32_t)type, NULL, &status);
	if (status >= SI_STATUS_INTERNAL) status = NO_RECOVERY;
	h_errno = status;

	LI_set_thread_item(CATEGORY_HOST + 200, item);
	if (item == NULL) return NULL;

	return (struct hostent *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
gethostbyaddr_async_call(const void *addr, socklen_t len, int type, si_host_async_callback callback, void *context)
{
	si_context_t *sictx;
	uint32_t addrlen;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %s\n", __func__, (type == AF_INET) ? inet_ntoa(*(struct in_addr *)addr) : "-IPv6-");
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_HOST;
	sictx->key_offset = 200;

	/* addr is not a C string - pass length in num3 */
	addrlen = len;
	return si_async_call(si_search(), SI_CALL_HOST_BYADDR, addr, NULL, NULL, (uint32_t)type, 0, addrlen, 0, (void *)si_libinfo_general_callback, sictx);
}

mach_port_t
gethostbyaddr_async_start(const char *addr, int len, int family, si_host_async_callback callback, void *context)
{
	socklen_t slen = len;

	return gethostbyaddr_async_call(addr, slen, family, callback, context);
}

void
gethostbyaddr_async_cancel(mach_port_t p)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	si_async_cancel(p);
}

#if 0
void
gethostbyaddr_async_handle_reply(void *param)
{

	mach_msg_header_t *msg;

#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	msg = (mach_msg_header_t *)param;
	si_async_handle_reply(msg);
}
#endif

void
gethostbyaddr_async_handleReply(void *param)
{
	mach_msg_header_t *msg;

#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	msg = (mach_msg_header_t *)param;
	si_async_handle_reply(msg);
}

struct hostent *
getipnodebyname(const char *name, int family, int flags, int *err)
{
	si_item_t *item;
	uint32_t status;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s %d 0x%08x\n", __func__, name, family, flags);
#endif

	status = SI_STATUS_NO_ERROR;

	item = si_ipnode_byname(si_search(), name, family, flags, NULL, &status);
	if (status >= SI_STATUS_INTERNAL) status = NO_RECOVERY;
	if (err != NULL) *err = status;

	if (item == NULL) return NULL;

	return (struct hostent *)((uintptr_t)item + sizeof(si_item_t));
}

#if 0
mach_port_t
getipnodebyname_async_call(const char *name, int family, int flags, int *err, si_host_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %s %d 0x%08x\n", __func__, name, family, flags);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_HOST;
	sictx->key_offset = -1;

	return si_async_call(si_search(), SI_CALL_IPNODE_BYNAME, name, NULL, NULL, (uint32_t)family, (uint32_t)flags, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

mach_port_t
getipnodebyname_async_start(const char *name, int family, int flags, int *err, si_host_async_callback callback, void *context)
{
	return getipnodebyname_async_call(name, family, flags, err, callback, context);
}

void
getipnodebyname_async_cancel(mach_port_t p)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	si_async_cancel(p);
}

void
getipnodebyname_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

void
getipnodebyname_async_handleReply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}
#endif

static int
is_a4_mapped(const char *s)
{
	int i;
	u_int8_t c;

	if (s == NULL) return 0;

	for (i = 0; i < 10; i++)
	{
		c = s[i];
		if (c != 0x0) return 0;
	}

	for (i = 10; i < 12; i++)
	{
		c = s[i];
		if (c != 0xff) return 0;
	}

	return 1;
}

static int
is_a4_compat(const char *s)
{
	int i;
	u_int8_t c;

	if (s == NULL) return 0;

	for (i = 0; i < 12; i++)
	{
		c = s[i];
		if (c != 0x0) return 0;
	}

	/* Check for :: and ::1 */
	for (i = 13; i < 15; i++)
	{
		/* anything non-zero in these 3 bytes means it's a V4 address */
		c = s[i];
		if (c != 0x0) return 1;
	}

	/* Leading 15 bytes are all zero */
	c = s[15];
	if (c == 0x0) return 0;
	if (c == 0x1) return 0;

	return 1;
}

struct hostent *
getipnodebyaddr(const void *src, size_t len, int family, int *err)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, (family == AF_INET) ? inet_ntoa(*(struct in_addr *)src) : "-IPv6-");
#endif

	if ((family == AF_INET6) && (len == IPV6_ADDR_LEN) && (is_a4_mapped((const char *)src) || is_a4_compat((const char *)src)))
	{
		src += 12;
		len = 4;
		family = AF_INET;
	}

	item = si_host_byaddr(si_search(), src, family, NULL, (uint32_t *)err);
	if (item == NULL) return NULL;

	return (struct hostent *)((uintptr_t)item + sizeof(si_item_t));
}

#if 0
static void
si_libinfo_ipnode_callback(si_item_t *item, uint32_t status, void *ctx)
{
	si_context_t *sictx;
	struct hostent *h;

	if (ctx == NULL) return;

	sictx = (si_context_t *)ctx;

	if ((sictx->orig_callback == NULL) || (status == SI_STATUS_CALL_CANCELLED))
	{
		free(sictx);
		si_item_release(item);
		return;
	}

	if (status >= SI_STATUS_INTERNAL) status = NO_RECOVERY;

	if (item == NULL)
	{
		((si_ipnode_async_callback)(sictx->orig_callback))(NULL, status, sictx->orig_context);
		return;
	}

	h = (struct hostent *)((uintptr_t)item + sizeof(si_item_t));
	((si_ipnode_async_callback)(sictx->orig_callback))(h, status, sictx->orig_context);

	free(sictx);
}

mach_port_t
getipnodebyaddr_async_call(const void *src, socklen_t len, int family, int *err, si_ipnode_async_callback callback, void *context)
{
	si_context_t *sictx;
	uint32_t srclen;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %s\n", __func__, (family == AF_INET) ? inet_ntoa(*(struct in_addr *)src) : "-IPv6-");
#endif

	if ((family == AF_INET6) && (len == IPV6_ADDR_LEN) && (is_a4_mapped((const char *)src) || is_a4_compat((const char *)src)))
	{
		src += 12;
		len = 4;
		family = AF_INET;
	}

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_HOST;
	sictx->key_offset = -1;

	/* src is not a C string - pass length in num3 */
	srclen = len;
	return si_async_call(si_search(), SI_CALL_HOST_BYADDR, src, NULL, NULL, (uint32_t)family, 0, srclen, 0, (void *)si_libinfo_ipnode_callback, sictx);
}

mach_port_t
getipnodebyaddr_async_start(const void *addr, size_t len, int family, int *error, si_ipnode_async_callback callback, void *context)
{
	socklen_t slen = len;

	return getipnodebyaddr_async_call(addr, slen, family, error, callback, context);
}

void
getipnodebyaddr_async_cancel(mach_port_t p)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	si_async_cancel(p);
}

void
getipnodebyaddr_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

void
getipnodebyaddr_async_handleReply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}
#endif

void
sethostent(int ignored)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_HOST, NULL);
}

struct hostent *
gethostent(void)
{
	si_list_t *list;
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s\n", __func__);
#endif

	list = LI_get_thread_list(CATEGORY_HOST);
	if (list == NULL)
	{
		list = si_host_all(si_search());
		LI_set_thread_list(CATEGORY_HOST, list);
	}

	item = si_list_next(list);
	if (item == NULL) return NULL;

	return (struct hostent *)((uintptr_t)item + sizeof(si_item_t));
}

void
endhostent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_HOST, NULL);
}

/* MAC ADDRESS */

int
ether_hostton(const char *name, struct ether_addr *e)
{
	si_item_t *item;
	si_mac_t *mac;
	uint32_t t[6];
	int i;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, name);
#endif

	if (name == NULL) return -1;
	if (e == NULL) return -1;

	item = si_mac_byname(si_search(), name);
	LI_set_thread_item(CATEGORY_MAC + 100, item);
	if (item == NULL) return -1;

	mac = (si_mac_t *)((uintptr_t)item + sizeof(si_item_t));

	i = sscanf(mac->mac, " %x:%x:%x:%x:%x:%x", &t[0], &t[1], &t[2], &t[3], &t[4], &t[5]);
	if (i != 6) return -1;

	for (i = 0; i < 6; i++) e->ether_addr_octet[i] = t[i];
	return 0;
}

/* XXX to do? async ether_hostton */

int
ether_ntohost(char *name, const struct ether_addr *e)
{
	si_item_t *item;
	si_mac_t *mac;
	uint32_t i, x[6];
	char str[256];

	if (name == NULL) return -1;
	if (e == NULL) return -1;

	for (i = 0; i < 6; i++) x[i] = e->ether_addr_octet[i];
	snprintf(str, sizeof(str), "%x:%x:%x:%x:%x:%x", x[0], x[1], x[2], x[3], x[4], x[5]);

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, str);
#endif

	item = si_mac_bymac(si_search(), str);
	LI_set_thread_item(CATEGORY_MAC + 200, item);
	if (item == NULL) return -1;

	mac = (si_mac_t *)((uintptr_t)item + sizeof(si_item_t));

	memcpy(name, mac->host, strlen(mac->host) + 1);
	return 0;
}

/* XXX to do? async ether_ntohost */

/* NETWORK */

struct netent *
getnetbyname(const char *name)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, name);
#endif

	item = si_network_byname(si_search(), name);
	LI_set_thread_item(CATEGORY_NETWORK + 100, item);
	if (item == NULL) return NULL;

	return (struct netent *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
getnetbyname_async_call(const char *name, si_network_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %s\n", __func__, name);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_NETWORK;
	sictx->key_offset = 100;

	return si_async_call(si_search(), SI_CALL_NETWORK_BYNAME, name, NULL, NULL, 0, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
getnetbyname_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

struct netent *
getnetbyaddr(uint32_t net, int type)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s 0x%08x\n", __func__, net);
#endif

	if (type != AF_INET) return NULL;

	item = si_network_byaddr(si_search(), net);
	LI_set_thread_item(CATEGORY_NETWORK + 200, item);
	if (item == NULL) return NULL;

	return (struct netent *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
getnetbyaddr_async_call(uint32_t net, int type, si_group_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s 0x%08x\n", __func__, net);
#endif

	if (type != AF_INET) return MACH_PORT_NULL;

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_NETWORK;
	sictx->key_offset = 200;

	return si_async_call(si_search(), SI_CALL_NETWORK_BYADDR, NULL, NULL, NULL, net, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
getnetbyaddr_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

void
setnetent(int ignored)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_NETWORK, NULL);
}

struct netent *
getnetent(void)
{
	si_list_t *list;
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s\n", __func__);
#endif

	list = LI_get_thread_list(CATEGORY_NETWORK);
	if (list == NULL)
	{
		list = si_network_all(si_search());
		LI_set_thread_list(CATEGORY_NETWORK, list);
	}

	item = si_list_next(list);
	if (item == NULL) return NULL;

	return (struct netent *)((uintptr_t)item + sizeof(si_item_t));
}

void
endnetent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_NETWORK, NULL);
}

/* SERVICE */

struct servent *
getservbyname(const char *name, const char *proto)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s %s\n", __func__, name, proto);
#endif

	item = si_service_byname(si_search(), name, proto);
	LI_set_thread_item(CATEGORY_SERVICE + 100, item);
	if (item == NULL) return NULL;

	return (struct servent *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
getservbyname_async_call(const char *name, const char *proto, si_service_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %s %s\n", __func__, name, proto);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_SERVICE;
	sictx->key_offset = 100;

	return si_async_call(si_search(), SI_CALL_SERVICE_BYNAME, name, proto, NULL, 0, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
getservbyname_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

struct servent *
getservbyport(int port, const char *proto)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %d %s\n", __func__, ntohs((uint16_t)port), proto);
#endif

	item = si_service_byport(si_search(), port, proto);
	LI_set_thread_item(CATEGORY_SERVICE + 200, item);
	if (item == NULL) return NULL;

	return (struct servent *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
getservbyport_async_call(int port, const char *proto, si_group_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %d %s\n", __func__, port, proto);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_SERVICE;
	sictx->key_offset = 200;

	return si_async_call(si_search(), SI_CALL_SERVICE_BYPORT, NULL, proto, NULL, port, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
getservbyport_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

void
setservent(int ignored)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_SERVICE, NULL);
}

struct servent *
getservent(void)
{
	si_list_t *list;
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s\n", __func__);
#endif

	list = LI_get_thread_list(CATEGORY_SERVICE);
	if (list == NULL)
	{
		list = si_service_all(si_search());
		LI_set_thread_list(CATEGORY_SERVICE, list);
	}

	item = si_list_next(list);
	if (item == NULL) return NULL;

	return (struct servent *)((uintptr_t)item + sizeof(si_item_t));
}

void
endservent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_SERVICE, NULL);
}

/* PROTOCOL */

struct protoent *
getprotobyname(const char *name)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, name);
#endif

	item = si_protocol_byname(si_search(), name);
	LI_set_thread_item(CATEGORY_PROTOCOL + 100, item);
	if (item == NULL) return NULL;

	return (struct protoent *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
getprotobyname_async_call(const char *name, si_protocol_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %s\n", __func__, name);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_PROTOCOL;
	sictx->key_offset = 100;

	return si_async_call(si_search(), SI_CALL_PROTOCOL_BYNAME, name, NULL, NULL, 0, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
getprotobyname_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

struct protoent *
getprotobynumber(int number)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %d\n", __func__, number);
#endif

	item = si_protocol_bynumber(si_search(), number);
	LI_set_thread_item(CATEGORY_PROTOCOL + 200, item);
	if (item == NULL) return NULL;

	return (struct protoent *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
getprotobynumber_async_call(int number, si_group_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %d\n", __func__, number);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_PROTOCOL;
	sictx->key_offset = 200;

	return si_async_call(si_search(), SI_CALL_PROTOCOL_BYNUMBER, NULL, NULL, NULL, number, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
getprotobynumber_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

void
setprotoent(int ignored)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_PROTOCOL, NULL);
}

struct protoent *
getprotoent(void)
{
	si_list_t *list;
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s\n", __func__);
#endif

	list = LI_get_thread_list(CATEGORY_PROTOCOL);
	if (list == NULL)
	{
		list = si_protocol_all(si_search());
		LI_set_thread_list(CATEGORY_PROTOCOL, list);
	}

	item = si_list_next(list);
	if (item == NULL) return NULL;

	return (struct protoent *)((uintptr_t)item + sizeof(si_item_t));
}

void
endprotoent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_PROTOCOL, NULL);
}

/* RPC */

struct rpcent *
getrpcbyname(const char *name)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, name);
#endif

	item = si_rpc_byname(si_search(), name);
	LI_set_thread_item(CATEGORY_RPC + 100, item);
	if (item == NULL) return NULL;

	return (struct rpcent *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
getrpcbyname_async_call(const char *name, si_rpc_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %s\n", __func__, name);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_RPC;
	sictx->key_offset = 100;

	return si_async_call(si_search(), SI_CALL_RPC_BYNAME, name, NULL, NULL, 0, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
getrpcbyname_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

struct rpcent *
getrpcbynumber
(
#ifdef __LP64__
	int number
#else
	long number
#endif
)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %ld\n", __func__, (long int)number);
#endif

	item = si_rpc_bynumber(si_search(), number);
	LI_set_thread_item(CATEGORY_RPC + 200, item);
	if (item == NULL) return NULL;

	return (struct rpcent *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
getrpcbynumber_async_call(int number, si_group_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %d\n", __func__, number);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_RPC;
	sictx->key_offset = 200;

	return si_async_call(si_search(), SI_CALL_RPC_BYNUMBER, NULL, NULL, NULL, number, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
getrpcbynumber_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

void
setrpcent(int ignored)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_RPC, NULL);
}

struct rpcent *
getrpcent(void)
{
	si_list_t *list;
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s\n", __func__);
#endif

	list = LI_get_thread_list(CATEGORY_RPC);
	if (list == NULL)
	{
		list = si_rpc_all(si_search());
		LI_set_thread_list(CATEGORY_RPC, list);
	}

	item = si_list_next(list);
	if (item == NULL) return NULL;

	return (struct rpcent *)((uintptr_t)item + sizeof(si_item_t));
}

void
endrpcent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_RPC, NULL);
}

/* FS */

struct fstab *
getfsspec(const char *spec)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, spec);
#endif

	item = si_fs_byspec(si_search(), spec);
	LI_set_thread_item(CATEGORY_FS + 100, item);
	if (item == NULL) return NULL;

	return (struct fstab *)((uintptr_t)item + sizeof(si_item_t));
}

struct fstab *
getfsbyname(const char *name)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, name);
#endif

	return getfsspec(name);
}

mach_port_t
getfsspec_async_call(const char *spec, si_fs_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %s\n", __func__, spec);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_FS;
	sictx->key_offset = 100;

	return si_async_call(si_search(), SI_CALL_FS_BYSPEC, spec, NULL, NULL, 0, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
getfsspec_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

struct fstab *
getfsfile(const char *file)
{
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, file);
#endif

	item = si_fs_byfile(si_search(), file);
	LI_set_thread_item(CATEGORY_FS + 200, item);
	if (item == NULL) return NULL;

	return (struct fstab *)((uintptr_t)item + sizeof(si_item_t));
}

mach_port_t
getfsfile_async_call(const char *file, si_fs_async_callback callback, void *context)
{
	si_context_t *sictx;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %s\n", __func__, file);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_FS;
	sictx->key_offset = 200;

	return si_async_call(si_search(), SI_CALL_FS_BYFILE, file, NULL, NULL, 0, 0, 0, 0, (void *)si_libinfo_general_callback, sictx);
}

void
getfsfile_async_handle_reply(mach_msg_header_t *msg)
{
#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	si_async_handle_reply(msg);
}

int
setfsent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-> %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_FS, NULL);
	return 1;
}

struct fstab *
getfsent(void)
{
	si_list_t *list;
	si_item_t *item;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s\n", __func__);
#endif

	list = LI_get_thread_list(CATEGORY_FS);
	if (list == NULL)
	{
		list = si_fs_all(si_search());
		LI_set_thread_list(CATEGORY_FS, list);
	}

	item = si_list_next(list);
	if (item == NULL) return NULL;

	return (struct fstab *)((uintptr_t)item + sizeof(si_item_t));
}

void
endfsent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	LI_set_thread_list(CATEGORY_FS, NULL);
}

/* GETADDRINFO */

static int
_getaddrinfo_internal(const char *nodename, const char *servname, const struct addrinfo *hints, const char *interface, struct addrinfo **res)
{
	si_list_t *list;
	uint32_t family, socktype, protocol, flags, status;
	struct addrinfo *ai;

	family = AF_UNSPEC;
	socktype = SOCK_UNSPEC;
	protocol = IPPROTO_UNSPEC;
	flags = 0;
	status = SI_STATUS_NO_ERROR;

	if (res == NULL) return 0;
	*res = NULL;

	if (hints != NULL)
	{
		family = hints->ai_family;
		socktype = hints->ai_socktype;
		protocol = hints->ai_protocol;
		flags = hints->ai_flags;
	}

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s %s %u %u %u 0x%08x %s\n", __func__, nodename, servname, family, socktype, protocol, flags, (interface == NULL) ? "" : interface);
#endif

	list = si_addrinfo(si_search(), nodename, servname, family, socktype, protocol, flags, interface, &status);
	if ((status != SI_STATUS_NO_ERROR) || (list == NULL) || (list->count == 0))
	{
		si_list_release(list);

		if (status == SI_STATUS_NO_ERROR) return EAI_NONAME;
		else if (status <= SI_STATUS_EAI_PLUS_100) status = EAI_FAIL;
		else if (status >= SI_STATUS_ERRNO_PLUS_200) status = EAI_FAIL;
		else status = status - SI_STATUS_EAI_PLUS_100;
		return status;
	}

	*res = si_list_to_addrinfo(list);
	si_list_release(list);
	if (*res == NULL) status = EAI_MEMORY;

	/* don't return the canonical name unless asked */
	if ((flags & AI_CANONNAME) == 0)
	{
		for (ai = *res; ai != NULL; ai = ai->ai_next)
		{
			free(ai->ai_canonname);
			ai->ai_canonname = NULL;
		}
	}

	return status;
}

int
getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res)
{
	return _getaddrinfo_internal(nodename, servname, hints, NULL, res);
}

#ifdef CALL_TRACE

static char *
socket_name(int sock)
{
	static char str[16];

	switch(sock)
	{
		case SOCK_UNSPEC: return "SOCK_UNSPEC";
		case SOCK_STREAM: return "SOCK_STREAM";
		case SOCK_DGRAM: return "SOCK_DGRAM";
	}

	sprintf(str, "%d", sock);
	return str;
}

static char *
family_name(int pf)
{
	static char str[16];

	switch(pf)
	{
		case PF_UNSPEC: return "PF_UNSPEC";
		case PF_INET: return "PF_INET";
		case PF_INET6: return "PF_INET6";
	};

	sprintf(str, "%d", pf);
	return str;
}

static char *
protocol_name(int p)
{
	static char str[16];

	switch(p)
	{
		case IPPROTO_UNSPEC: return "IPPROTO_UNSPEC";
		case IPPROTO_TCP: return "IPPROTO_TCP";
		case IPPROTO_UDP: return "IPPROTO_UDP";
	}

	sprintf(str, "%d", p);
	return str;
}

static char *
_gai_inet_ntop(struct in6_addr a)
{
	static char buf[128];
	char t[32];
	unsigned short x;
	char *p;
	int i;

	memset(buf, 0, 128);

	p = (char *)&a.__u6_addr.__u6_addr32;
	for (i = 0; i < 8; i++, x += 1)
	{
		memmove(&x, p, 2);
		p += 2;
		sprintf(t, "%hx", x);
		strcat(buf, t);
		if (i < 7) strcat(buf, ":");
	}

	return buf;
}

static void
fprint_addrinfo(FILE *f, struct addrinfo *a)
{
	int i;
	unsigned char v;
	struct sockaddr_in *s4;
	struct sockaddr_in6 *s6;

	if (a == NULL) return;

	if (a->ai_flags != 0)
	{
		fprintf(f, "flags =");
		if (a->ai_flags & AI_PASSIVE) fprintf(f, " AI_PASSIVE");
		if (a->ai_flags & AI_CANONNAME) fprintf(f, " AI_CANONNAME");
		if (a->ai_flags & AI_NUMERICHOST) fprintf(f, " AI_NUMERICHOST");
		if (a->ai_flags & AI_NUMERICSERV) fprintf(f, " AI_NUMERICSERV");
		fprintf(f, "\n");
	}

	fprintf(f, "family = %s\n", family_name(a->ai_family));
	fprintf(f, "socktype = %s\n", socket_name(a->ai_socktype));
	fprintf(f, "protocol = %s\n", protocol_name(a->ai_protocol));

	fprintf(f, "canonical name = ");
	if (a->ai_canonname == NULL) fprintf(f, "NULL\n");
	else fprintf(f, "\"%s\"\n", a->ai_canonname);

	fprintf(f, "addrlen = %ld\n", (long int)a->ai_addrlen);

	if (a->ai_addr == NULL) fprintf(f, "sockaddr = NULL\n");
	else
	{
		if (a->ai_family == PF_INET)
		{
			s4 = (struct sockaddr_in *)a->ai_addr;

			fprintf(f, "sockaddr_in len = %d\n", s4->sin_len);
			fprintf(f, "sockaddr_in family = %s\n", family_name(s4->sin_family));
			fprintf(f, "sockaddr_in port = %hu\n", ntohs(s4->sin_port));
			fprintf(f, "sockaddr_in address = %s\n", inet_ntoa(s4->sin_addr));
		}
		else if (a->ai_family == PF_INET6)
		{
			s6 = (struct sockaddr_in6 *)a->ai_addr;

			fprintf(f, "sockaddr_in6 len = %d\n", s6->sin6_len);
			fprintf(f, "sockaddr_in6 family = %s\n", family_name(s6->sin6_family));
			fprintf(f, "sockaddr_in6 port = %hu\n", ntohs(s6->sin6_port));
			fprintf(f, "sockaddr_in6 flowinfo = %d\n", s6->sin6_flowinfo);
			fprintf(f, "sockaddr_in6 address = %s\n", _gai_inet_ntop(s6->sin6_addr));
			fprintf(f, "sockaddr_in6 scope_id = %d\n", s6->sin6_scope_id);
		}
		else
		{
			fprintf(f, "sockaddr len = %d\n", a->ai_addr->sa_len);
			fprintf(f, "sockaddr family = %s\n", family_name(a->ai_addr->sa_family));
			fprintf(f, "sockaddr data = ");
			for (i = 0; i < a->ai_addr->sa_len - 2; i++)
			{
				v = a->ai_addr->sa_data[i];
				fprintf(f, "%02x", v);
			}
			fprintf(f, "\n");
		}
	}

	if (a->ai_next != NULL) 
	{
		fprintf(f, "NEXT --->\n");
		fprint_addrinfo(f, a->ai_next);
	}
}

#endif

static void
si_libinfo_addrinfo_callback(si_list_t *list, uint32_t status, void *ctx)
{
	si_context_t *sictx;
	struct addrinfo *out;

	if (ctx == NULL)
	{
#ifdef CALL_TRACE
		fprintf(stderr, "   %s error no context\n", __func__);
#endif
		si_list_release(list);
		return;
	}

	sictx = (si_context_t *)ctx;

	if ((sictx->orig_callback == NULL) || (status == SI_STATUS_CALL_CANCELLED))
	{
#ifdef CALL_TRACE
		fprintf(stderr, "   %s error no callback\n", __func__);
#endif
		si_list_release(list);
		free(sictx);
		return;
	}

	if (status != SI_STATUS_NO_ERROR)
	{
#ifdef CALL_TRACE
		fprintf(stderr, "   %s original status %d\n", __func__, status);
#endif
		if (status <= SI_STATUS_EAI_PLUS_100) status = EAI_FAIL;
		else if (status >= SI_STATUS_ERRNO_PLUS_200) status = EAI_FAIL;
		else status = status - SI_STATUS_EAI_PLUS_100;
	}

	if (list == NULL)
	{
#ifdef CALL_TRACE
		fprintf(stderr, "   %s result NULL status %d (returning EAI_NONAME)\n", __func__, status);
#endif
		((si_addrinfo_async_callback)(sictx->orig_callback))(EAI_NONAME, NULL, sictx->orig_context);
		free(sictx);
		return;
	}

	out = si_list_to_addrinfo(list);
	si_list_release(list);
	if (out == NULL)
	{
#ifdef CALL_TRACE
		fprintf(stderr, "   %s result conversion failed returning NULL status %d (returning EAI_MEMORY)\n", __func__, status);
#endif
		((si_addrinfo_async_callback)(sictx->orig_callback))(EAI_MEMORY, NULL, sictx->orig_context);
		free(sictx);
		return;
	}

#ifdef CALL_TRACE
	fprintf(stderr, "   %s %d\n", __func__, status);
	fprint_addrinfo(stderr, out);
#endif
	((si_addrinfo_async_callback)(sictx->orig_callback))(status, out, sictx->orig_context);

	free(sictx);
}

/* SPI */
mach_port_t
_getaddrinfo_interface_async_call(const char *nodename, const char *servname, const struct addrinfo *hints, const char *interface, si_addrinfo_async_callback callback, void *context)
{
	si_context_t *sictx;
	uint32_t family, socktype, protocol, flags;

	family = AF_UNSPEC;
	socktype = SOCK_UNSPEC;
	protocol = IPPROTO_UNSPEC;
	flags = 0;

	if (hints != NULL)
	{
		family = hints->ai_family;
		socktype = hints->ai_socktype;
		protocol = hints->ai_protocol;
		flags = hints->ai_flags;
	}

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s %s %s %u %u %u 0x%08x\n", __func__, nodename, servname, family, socktype, protocol, flags);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_ADDRINFO;
	sictx->key_offset = 0;

	return si_async_call(si_search(), SI_CALL_ADDRINFO, nodename, servname, interface, family, socktype, protocol, flags, (void *)si_libinfo_addrinfo_callback, sictx);
}

mach_port_t
getaddrinfo_async_call(const char *nodename, const char *servname, const struct addrinfo *hints, si_addrinfo_async_callback callback, void *context)
{
	return _getaddrinfo_interface_async_call(nodename, servname, hints, NULL, callback, context);
}

int32_t
getaddrinfo_async_start(mach_port_t *p, const char *nodename, const char *servname, const struct addrinfo *hints, si_addrinfo_async_callback callback, void *context)
{
	if (p == NULL) return EAI_SYSTEM;

	*p = getaddrinfo_async_call(nodename, servname, hints, callback, context);

	if (*p == MACH_PORT_NULL) return EAI_SYSTEM;
	return 0;
}

int32_t
getaddrinfo_async_send(mach_port_t *p, const char *nodename, const char *servname, const struct addrinfo *hints)
{
	return getaddrinfo_async_start(p, nodename, servname, hints, NULL, NULL);
}

int32_t
getaddrinfo_async_receive(mach_port_t p, struct addrinfo **res)
{
	/* unsupported Leopard SPI */
	return EAI_SYSTEM;
}

void
getaddrinfo_async_cancel(mach_port_t p)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	si_async_cancel(p);
}

int32_t
getaddrinfo_async_handle_reply(void *param)
{
	mach_msg_header_t *msg;

#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	msg = (mach_msg_header_t *)param;
	si_async_handle_reply(msg);

	return 0;
}

/* GETNAMEINFO */

static int
_getnameinfo_interface_internal(const struct sockaddr *sa, socklen_t salen, char *node, socklen_t nodelen, char *service, socklen_t servicelen, int flags, const char *interface)
{
	si_item_t *item;
	si_nameinfo_t *ni;
	uint32_t status, len, wantn, wants;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s\n", __func__);
#endif

	status = SI_STATUS_NO_ERROR;

	wantn = 0;
	if ((node != NULL) && (nodelen > 0)) wantn = 1;

	wants = 0;
	if ((service != NULL) && (servicelen > 0)) wants = 1;

	if ((wantn == 0) && (wants == 0)) return status;

	if (wantn == 0) flags |= NI_NUMERICHOST;
	if (wants == 0) flags |= NI_NUMERICSERV;

	item = si_nameinfo(si_search(), sa, flags, interface, &status);
	if ((status != SI_STATUS_NO_ERROR) || (item == NULL))
	{
		si_item_release(item);

		if (status == SI_STATUS_NO_ERROR) status = EAI_NONAME;
		else if (status <= SI_STATUS_EAI_PLUS_100) status = EAI_FAIL;
		else if (status >= SI_STATUS_ERRNO_PLUS_200) status = EAI_FAIL;
		else status = status - SI_STATUS_EAI_PLUS_100;
		return status;
	}

	ni = (si_nameinfo_t *)((uintptr_t)item + sizeof(si_item_t));

	len = 0;
	if (ni->ni_node != NULL) len = strlen(ni->ni_node) + 1;
	if ((wantn == 1) && (len > 0))
	{
		if (len > nodelen)
		{
			si_item_release(item);
			return EAI_OVERFLOW;
		}

		memset(node, 0, nodelen);
		memcpy(node, ni->ni_node, len);
	}

	len = 0;
	if (ni->ni_serv != NULL) len = strlen(ni->ni_serv) + 1;
	if ((wants == 1) && (len > 0))
	{
		if (len > servicelen)
		{
			si_item_release(item);
			return EAI_OVERFLOW;
		}

		memset(service, 0, servicelen);
		memcpy(service, ni->ni_serv, len);
	}

	si_item_release(item);
	return 0;
}

int
getnameinfo(const struct sockaddr *sa, socklen_t salen, char *node, socklen_t nodelen, char *service, socklen_t servicelen, int flags)
{
	if (sa == NULL) return EAI_FAIL;

	if (sa->sa_family == AF_LINK) return getnameinfo_link(sa, salen, node, nodelen, service, servicelen, flags);
	return _getnameinfo_interface_internal(sa, salen, node, nodelen, service, servicelen, flags, NULL);
}

static void
si_libinfo_nameinfo_callback(si_item_t *item, uint32_t status, void *ctx)
{
	si_context_t *sictx;
	si_nameinfo_t *ni;
	char *node, *serv;

	if (ctx == NULL) return;

	sictx = (si_context_t *)ctx;

	if ((sictx->orig_callback == NULL) || (status == SI_STATUS_CALL_CANCELLED))
	{
		si_item_release(item);
		free(sictx);
		return;
	}

	if (status != SI_STATUS_NO_ERROR)
	{
		if (status <= SI_STATUS_EAI_PLUS_100) status = EAI_FAIL;
		else if (status >= SI_STATUS_ERRNO_PLUS_200) status = EAI_FAIL;
		else status = status - SI_STATUS_EAI_PLUS_100;
	}

	if (item == NULL)
	{
		((si_nameinfo_async_callback)(sictx->orig_callback))(status, NULL, NULL, sictx->orig_context);
		free(sictx);
		return;
	}

	LI_set_thread_item(CATEGORY_NAMEINFO, item);

	node = NULL;
	serv = NULL;

	ni = (si_nameinfo_t *)((uintptr_t)item + sizeof(si_item_t));
	if (ni->ni_node != NULL) node = strdup(ni->ni_node);
	if (ni->ni_serv != NULL) serv = strdup(ni->ni_serv);

	((si_nameinfo_async_callback)(sictx->orig_callback))(status, node, serv, sictx->orig_context);
	free(sictx);
}

/* SPI */
mach_port_t
_getnameinfo_interface_async_call(const struct sockaddr *sa, size_t len, int flags, const char *interface, si_nameinfo_async_callback callback, void *context)
{
	si_context_t *sictx;
	uint32_t salen;

#ifdef CALL_TRACE
	fprintf(stderr, ">> %s\n", __func__);
#endif

	sictx = (si_context_t *)calloc(1, sizeof(si_context_t));
	if (sictx == NULL) return MACH_PORT_NULL;

	sictx->orig_callback = callback;
	sictx->orig_context = context;
	sictx->cat = CATEGORY_ADDRINFO;
	sictx->key_offset = 0;

	/* sa is not a C string - pass length in num3 */
	salen = len;
	return si_async_call(si_search(), SI_CALL_NAMEINFO, (const char *)sa, NULL, interface, flags, 0, salen, 0, (void *)si_libinfo_nameinfo_callback, sictx);
}

mach_port_t
getnameinfo_async_call(const struct sockaddr *sa, size_t len, int flags, si_nameinfo_async_callback callback, void *context)
{
	return _getnameinfo_interface_async_call(sa, len, flags, NULL, callback, context);
}

int32_t
getnameinfo_async_start(mach_port_t *p, const struct sockaddr *sa, size_t salen, int flags, si_nameinfo_async_callback callback, void *context)
{
	if (p == NULL) return EAI_SYSTEM;
	*p = getnameinfo_async_call(sa, salen, flags, callback, context);

	if (*p == MACH_PORT_NULL) return EAI_SYSTEM;
	return 0;
}

int32_t
getnameinfo_async_send(mach_port_t *p, const struct sockaddr *sa, size_t salen, int flags)
{
	return getnameinfo_async_start(p, sa, salen, flags, NULL, NULL);
}

void
getnameinfo_async_cancel(mach_port_t p)
{
#ifdef CALL_TRACE
	fprintf(stderr, "-- %s\n", __func__);
#endif

	si_async_cancel(p);
}

int32_t
getnameinfo_async_handle_reply(void *param)
{
	mach_msg_header_t *msg;

#ifdef CALL_TRACE
	fprintf(stderr, "<< %s\n", __func__);
#endif

	msg = (mach_msg_header_t *)param;
	si_async_handle_reply(msg);

	return 0;
}

/* getpwXXX_r and getgrXXX_r */

static int
copy_user_r(struct passwd *in, struct passwd *out, char *buffer, int buflen)
{
	int hsize;
	char *bp;

	if (in == NULL) return -1;
	if (out == NULL) return -1;

	if (buffer == NULL) buflen = 0;

	/* Calculate size of input */
	hsize = 0;
	if (in->pw_name != NULL) hsize += (strlen(in->pw_name) + 1);
	if (in->pw_passwd != NULL) hsize += (strlen(in->pw_passwd) + 1);
	if (in->pw_class != NULL) hsize += (strlen(in->pw_class) + 1);
	if (in->pw_gecos != NULL) hsize += (strlen(in->pw_gecos) + 1);
	if (in->pw_dir != NULL) hsize += (strlen(in->pw_dir) + 1);
	if (in->pw_shell != NULL) hsize += (strlen(in->pw_shell) + 1);

	/* Check buffer space */
	if (hsize > buflen) return -1;

	/* Copy result into caller's struct passwd, using buffer for memory */
	bp = buffer;

	out->pw_name = NULL;
	if (in->pw_name != NULL)
	{
		out->pw_name = bp;
		hsize = strlen(in->pw_name) + 1;
		memmove(bp, in->pw_name, hsize);
		bp += hsize;
	}

	out->pw_passwd = NULL;
	if (in->pw_passwd != NULL)
	{
		out->pw_passwd = bp;
		hsize = strlen(in->pw_passwd) + 1;
		memmove(bp, in->pw_passwd, hsize);
		bp += hsize;
	}

	out->pw_uid = in->pw_uid;

	out->pw_gid = in->pw_gid;

	out->pw_change = in->pw_change;

	out->pw_class = NULL;
	if (in->pw_class != NULL)
	{
		out->pw_class = bp;
		hsize = strlen(in->pw_class) + 1;
		memmove(bp, in->pw_class, hsize);
		bp += hsize;
	}

	out->pw_gecos = NULL;
	if (in->pw_gecos != NULL)
	{
		out->pw_gecos = bp;
		hsize = strlen(in->pw_gecos) + 1;
		memmove(bp, in->pw_gecos, hsize);
		bp += hsize;
	}

	out->pw_dir = NULL;
	if (in->pw_dir != NULL)
	{
		out->pw_dir = bp;
		hsize = strlen(in->pw_dir) + 1;
		memmove(bp, in->pw_dir, hsize);
		bp += hsize;
	}

	out->pw_shell = NULL;
	if (in->pw_shell != NULL)
	{
		out->pw_shell = bp;
		hsize = strlen(in->pw_shell) + 1;
		memmove(bp, in->pw_shell, hsize);
		bp += hsize;
	}

	out->pw_expire = in->pw_expire;

	return 0;
}

static int
copy_group_r(struct group *in, struct group *out, char *buffer, int buflen)
{
	int i, len, hsize;
	unsigned long addr;
	char *bp, *ap;

	if (in == NULL) return -1;
	if (out == NULL) return -1;

	if (buffer == NULL) buflen = 0;

	/* Calculate size of input */
	hsize = 0;
	if (in->gr_name != NULL) hsize += (strlen(in->gr_name) + 1);
	if (in->gr_passwd != NULL) hsize += (strlen(in->gr_passwd) + 1);

	/* NULL pointer at end of list */
	hsize += sizeof(char *);

	len = 0;
	if (in->gr_mem != NULL)
	{
		for (len = 0; in->gr_mem[len] != NULL; len++)
		{
			hsize += sizeof(char *);
			hsize += (strlen(in->gr_mem[len]) + 1);
		}
	}

	/* Check buffer space */
	if (hsize > buflen) return -1;

	/* Copy result into caller's struct group, using buffer for memory */
	bp = buffer;

	out->gr_name = NULL;
	if (in->gr_name != NULL)
	{
		out->gr_name = bp;
		hsize = strlen(in->gr_name) + 1;
		memmove(bp, in->gr_name, hsize);
		bp += hsize;
	}

	out->gr_passwd = NULL;
	if (in->gr_passwd != NULL)
	{
		out->gr_passwd = bp;
		hsize = strlen(in->gr_passwd) + 1;
		memmove(bp, in->gr_passwd, hsize);
		bp += hsize;
	}

	out->gr_gid = in->gr_gid;

	out->gr_mem = NULL;
	ap = bp + ((len + 1) * sizeof(char *));

	if (in->gr_mem != NULL)
	{
		out->gr_mem = (char **)bp;
		for (i = 0; i < len; i++)
		{
			addr = (unsigned long)ap;
			memmove(bp, &addr, sizeof(unsigned long));
			bp += sizeof(unsigned long);

			hsize = strlen(in->gr_mem[i]) + 1;
			memmove(ap, in->gr_mem[i], hsize);
			ap += hsize;
		}
	}

	memset(bp, 0, sizeof(unsigned long));
	bp = ap;

	return 0;
}

int
getgrnam_r(const char *name, struct group *grp, char *buffer, size_t bufsize, struct group **result)
{
	si_item_t *item;
	struct group *g;
	int status;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, name);
#endif

	if (result != NULL) *result = NULL;

	if ((grp == NULL) || (buffer == NULL) || (result == NULL) || (bufsize == 0)) return ERANGE;

	item = si_group_byname(si_search(), name);
	if (item == NULL) return 0;

	g = (struct group *)((uintptr_t)item + sizeof(si_item_t));

	status = copy_group_r(g, grp, buffer, bufsize);
	si_item_release(item);

	if (status != 0) return ERANGE;

	*result = grp;
	return 0;
}

int
getgrgid_r(gid_t gid, struct group *grp, char *buffer, size_t bufsize, struct group **result)
{
	si_item_t *item;
	struct group *g;
	int status;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %d\n", __func__, gid);
#endif

	if (result != NULL) *result = NULL;

	if ((grp == NULL) || (buffer == NULL) || (result == NULL) || (bufsize == 0)) return ERANGE;

	item = si_group_bygid(si_search(), gid);
	if (item == NULL) return 0;

	g = (struct group *)((uintptr_t)item + sizeof(si_item_t));

	status = copy_group_r(g, grp, buffer, bufsize);
	si_item_release(item);

	if (status != 0) return ERANGE;

	*result = grp;
	return 0;
}

int
getgruuid_r(uuid_t uuid, struct group *grp, char *buffer, size_t bufsize, struct group **result)
{
	si_item_t *item;
	struct group *g;
	int status;

#ifdef CALL_TRACE
	uuid_string_t uuidstr;
	uuid_unparse_upper(uuid, uuidstr);
	fprintf(stderr, "-> %s %s\n", __func__, uuidstr);
#endif

	if (result != NULL) *result = NULL;

	if ((grp == NULL) || (buffer == NULL) || (result == NULL) || (bufsize == 0)) return ERANGE;

	item = si_group_byuuid(si_search(), uuid);
	if (item == NULL) return 0;

	g = (struct group *)((uintptr_t)item + sizeof(si_item_t));

	status = copy_group_r(g, grp, buffer, bufsize);
	si_item_release(item);

	if (status != 0) return ERANGE;

	*result = grp;
	return 0;
}

int
getpwnam_r(const char *name, struct passwd *pw, char *buffer, size_t bufsize, struct passwd **result)
{
	si_item_t *item;
	struct passwd *p;
	int status;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %s\n", __func__, name);
#endif

	if (result != NULL) *result = NULL;

	if ((pw == NULL) || (buffer == NULL) || (result == NULL) || (bufsize == 0)) return ERANGE;

	item = si_user_byname(si_search(), name);
	if (item == NULL) return 0;

	p = (struct passwd *)((uintptr_t)item + sizeof(si_item_t));

	status = copy_user_r(p, pw, buffer, bufsize);
	si_item_release(item);

	if (status != 0) return ERANGE;

	*result = pw;
	return 0;
}

int
getpwuid_r(uid_t uid, struct passwd *pw, char *buffer, size_t bufsize, struct passwd **result)
{
	si_item_t *item;
	struct passwd *p;
	int status;

#ifdef CALL_TRACE
	fprintf(stderr, "-> %s %d\n", __func__, uid);
#endif

	if (result != NULL) *result = NULL;

	if ((pw == NULL) || (buffer == NULL) || (result == NULL) || (bufsize == 0)) return ERANGE;

	item = si_user_byuid(si_search(), uid);
	if (item == NULL) return 0;

	p = (struct passwd *)((uintptr_t)item + sizeof(si_item_t));

	status = copy_user_r(p, pw, buffer, bufsize);
	si_item_release(item);

	if (status != 0) return ERANGE;

	*result = pw;
	return 0;
}

int
getpwuuid_r(uuid_t uuid, struct passwd *pw, char *buffer, size_t bufsize, struct passwd **result)
{
	si_item_t *item;
	struct passwd *p;
	int status;

#ifdef CALL_TRACE
	uuid_string_t uuidstr;
	uuid_unparse_upper(uuid, uuidstr);
	fprintf(stderr, "-> %s %s\n", __func__, uuidstr);
#endif

	if (result != NULL) *result = NULL;

	if ((pw == NULL) || (buffer == NULL) || (result == NULL) || (bufsize == 0)) return ERANGE;

	item = si_user_byuuid(si_search(), uuid);
	if (item == NULL) return 0;

	p = (struct passwd *)((uintptr_t)item + sizeof(si_item_t));

	status = copy_user_r(p, pw, buffer, bufsize);
	si_item_release(item);

	if (status != 0) return ERANGE;

	*result = pw;
	return 0;
}

/* misc */

char *
user_from_uid(uid_t uid, int nouser)
{
	struct passwd *pw;
	static char buf[16];

	pw = getpwuid(uid);
	if (pw != NULL) return pw->pw_name;

	if (nouser) return NULL;

	snprintf(buf, sizeof(buf), "%u", uid);
	return buf;
}

char *
group_from_gid(gid_t gid, int nogroup)
{
	struct group *gr;
	static char buf[16];

	gr = getgrgid(gid);
	if (gr != NULL) return gr->gr_name;

	if (nogroup) return NULL;

	snprintf(buf, sizeof(buf), "%u", gid);
	return buf;
}

/* no longer supported */

const prdb_ent *
prdb_getbyname(const char *name)
{
#ifdef CALL_TRACE
	fprintf(stderr, "~~ %s\n", __func__);
#endif
	return NULL;
}

const prdb_ent *
prdb_get(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "~~ %s\n", __func__);
#endif
	return NULL;
}

void
prdb_set(const char *name)
{
#ifdef CALL_TRACE
	fprintf(stderr, "~~ %s\n", __func__);
#endif
}

void
prdb_end(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "~~ %s\n", __func__);
#endif
}

struct bootparamsent *
bootparams_getbyname(const char *name)
{
#ifdef CALL_TRACE
	fprintf(stderr, "~~ %s\n", __func__);
#endif
	return NULL;
}

struct bootparamsent *
bootparams_getent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "~~ %s\n", __func__);
#endif
	return NULL;
}

void
bootparams_setent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "~~ %s\n", __func__);
#endif
}

void
bootparams_endent(void)
{
#ifdef CALL_TRACE
	fprintf(stderr, "~~ %s\n", __func__);
#endif
}

int
bootp_getbyether(struct ether_addr *enaddr, char **name,struct in_addr *ipaddr, char **bootfile)
{
#ifdef CALL_TRACE
	fprintf(stderr, "~~ %s\n", __func__);
#endif
	return 0;
}

int
bootp_getbyip(struct ether_addr *enaddr, char **name, struct in_addr *ipaddr, char **bootfile)
{
#ifdef CALL_TRACE
	fprintf(stderr, "~~ %s\n", __func__);
#endif
	return 0;
}

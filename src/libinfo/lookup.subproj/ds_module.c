/*
 * Copyright (c) 2008-2011 Apple Inc.  All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This ds contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this ds except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * ds.
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

#ifdef DS_AVAILABLE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <pthread.h>
#include <ils.h>
#include <pwd.h>
#include <grp.h>
#include <fstab.h>
#include <netdb.h>
#include <notify.h>
#include <notify_keys.h>
#include <si_data.h>
#include <si_module.h>
#include <netdb_async.h>
#include <net/if.h>
#include <xpc/xpc.h>
#include <xpc/private.h>
#include <opendirectory/odipc.h>
#include <servers/bootstrap.h>
#include <bootstrap_priv.h>
#include <opendirectory/DSlibinfoMIG_types.h>
#ifdef DEBUG
#include <asl.h>
#endif

#define IPV6_ADDR_LEN 16
#define IPV4_ADDR_LEN 4

typedef si_item_t *(*od_extract_t)(si_mod_t *si, xpc_object_t reply, const void *extra, uint64_t valid_global, uint64_t valid_cat);

/* notify SPI */
uint32_t notify_peek(int token, uint32_t *val);

typedef struct
{
	int notify_token_global;
	int notify_token_user;
	int notify_token_group;
	int notify_token_service;
} ds_si_private_t;

extern uint32_t gL1CacheEnabled;
extern int _si_opendirectory_disabled;

static pthread_key_t _ds_serv_cache_key = 0;
static xpc_pipe_t __od_pipe;	/* use accessor only */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

mach_port_t _ds_port;

static void
_od_fork_child(void)
{
	// re-enable opendirectory interaction since we forked
	_si_opendirectory_disabled = 0;

	if (__od_pipe != NULL) {
		xpc_pipe_invalidate(__od_pipe);
		/* disable release due to 10649340, it will cause a minor leak for each fork without exec */
		// xpc_release(__od_pipe);
		__od_pipe = NULL;
	}
	_ds_port = MACH_PORT_NULL;
	pthread_mutex_unlock(&mutex);
}

static void
_od_fork_prepare(void)
{
	pthread_mutex_lock(&mutex);
}

static void
_od_fork_parent(void)
{
	pthread_mutex_unlock(&mutex);
}

static void
_ds_serv_cache_free(void *x)
{
	if (x != NULL) si_item_release(x);
}

void
_si_disable_opendirectory(void)
{
	_si_opendirectory_disabled = 1;
	_ds_port = MACH_PORT_NULL;
}

XPC_RETURNS_RETAINED
static xpc_pipe_t
_od_xpc_pipe(bool resetPipe)
{
	static dispatch_once_t once;
	xpc_pipe_t result = NULL;
	
	dispatch_once(&once, ^(void) {
		char *xbs_disable;

		/* if this is a build environment we ignore opendirectoryd */
		xbs_disable = getenv("XBS_DISABLE_LIBINFO");
		if ((issetugid() == 0) && (xbs_disable != NULL) && (strcmp(xbs_disable, "YES") == 0)) {
			_si_opendirectory_disabled = 1;
			return;
		}

		pthread_atfork(_od_fork_prepare, _od_fork_parent, _od_fork_child);
	});

	if (_si_opendirectory_disabled == 1) {
		return NULL;
	}

	pthread_mutex_lock(&mutex);
	if (resetPipe) {
		xpc_release(__od_pipe);
		__od_pipe = NULL;
	}

	if (__od_pipe == NULL) {
		if (!issetugid() && getenv("OD_DEBUG_MODE") != NULL) {
			__od_pipe = xpc_pipe_create(kODMachLibinfoPortNameDebug, 0);
		} else {
			__od_pipe = xpc_pipe_create(kODMachLibinfoPortName, XPC_PIPE_FLAG_PRIVILEGED);
		}
	}

	if (__od_pipe != NULL) result = xpc_retain(__od_pipe);
	pthread_mutex_unlock(&mutex);

	return result;
}

static bool
_od_running(void)
{
	xpc_pipe_t pipe;

	pipe = _od_xpc_pipe(false);
	if (pipe != NULL) {
		xpc_release(pipe);
	}

	if (_si_opendirectory_disabled) {
		return 0;
	}

	return (pipe != NULL);
}

static void
_ds_child(void)
{
	_ds_port = MACH_PORT_NULL;
}

int
_ds_running(void)
{
	kern_return_t status;
	char *od_debug_mode = NULL;

	if (_ds_port != MACH_PORT_NULL) return 1;

	if (_si_opendirectory_disabled) return 0;
	pthread_atfork(NULL, NULL, _ds_child);

	if (!issetugid()) {
		od_debug_mode = getenv("OD_DEBUG_MODE");
	}

	if (od_debug_mode) {
		status = bootstrap_look_up(bootstrap_port, kDSStdMachDSLookupPortName "_debug", &_ds_port);
	} else {
		status = bootstrap_look_up2(bootstrap_port, kDSStdMachDSLookupPortName, &_ds_port, 0, BOOTSTRAP_PRIVILEGED_SERVER);
	}
	if ((status != BOOTSTRAP_SUCCESS) && (status != BOOTSTRAP_UNKNOWN_SERVICE)) _ds_port = MACH_PORT_NULL;

	return (_ds_port != MACH_PORT_NULL);
}

static bool
_valid_token(xpc_object_t reply)
{
	audit_token_t token;

	/*
	 * This should really call audit_token_to_au32,
	 * but that's in libbsm, not in a Libsystem library.
	 */
	xpc_dictionary_get_audit_token(reply, &token);

	return ((uid_t) token.val[1] == 0);
}

static void
_ds_get_validation(si_mod_t *si, uint64_t *a, uint64_t *b, int cat)
{
	ds_si_private_t *pp;
	uint32_t peek;
	int status;

	if (si == NULL) return;

	pp = (ds_si_private_t *)si->private;
	if (pp == NULL) return;

	if (a != NULL)
	{
		*a = 0;
		status = notify_peek(pp->notify_token_global, &peek);
		if (status == NOTIFY_STATUS_OK) *a = ntohl(peek);
	}

	if (b != NULL)
	{
		*b = 0;
		peek = 0;
		status = NOTIFY_STATUS_FAILED;

		if (cat == CATEGORY_USER) status = notify_peek(pp->notify_token_user, &peek);
		else if (cat == CATEGORY_GROUP) status = notify_peek(pp->notify_token_group, &peek);
		else if (cat == CATEGORY_GROUPLIST) status = notify_peek(pp->notify_token_group, &peek);
		else if (cat == CATEGORY_SERVICE) status = notify_peek(pp->notify_token_service, &peek);

		if (status == NOTIFY_STATUS_OK) *b = ntohl(peek);
	}
}

XPC_RETURNS_RETAINED
__private_extern__ xpc_object_t
_od_rpc_call(const char *procname, xpc_object_t payload, xpc_pipe_t (*get_pipe)(bool))
{
	xpc_object_t result = NULL;
	xpc_object_t reply;
	xpc_pipe_t od_pipe;
	int retries, rc;

	od_pipe = get_pipe(false);
	if (od_pipe == NULL) return NULL;

	if (payload == NULL) {
		payload = xpc_dictionary_create(NULL, NULL, 0);
	}

	// we nest it for backward compatibility so we can do independent submissions
	xpc_dictionary_set_string(payload, OD_RPC_NAME, procname);
	xpc_dictionary_set_int64(payload, OD_RPC_VERSION, 2);

	for (retries = 0; od_pipe != NULL && retries < 2; retries++) {
		rc = xpc_pipe_routine(od_pipe, payload, &reply);
		switch (rc) {
			case EPIPE:
				xpc_release(od_pipe);
				od_pipe = get_pipe(true);
				break;

			case EAGAIN:
				/* just loop and try to send again */
				break;

			case 0:
				if (_valid_token(reply) == true) {
					result = reply;
				}
				/* fall through since we got a valid response */

			default:
				/* release and NULL the pipe it'll break the loop */
				xpc_release(od_pipe);
				od_pipe = NULL;
				break;
		}
	}

	if (od_pipe != NULL) {
		xpc_release(od_pipe);
	}

	return result;
}

static si_list_t *
_ds_list(si_mod_t *si, int cat, const char *procname, const void *extra, od_extract_t extract)
{
	__block si_list_t *list;
	uint64_t va, vb;
	xpc_object_t reply, result;

	if (procname == NULL) return NULL;

	_ds_get_validation(si, &va, &vb, cat);

	list = NULL;
	reply = _od_rpc_call(procname, NULL, _od_xpc_pipe);
	if (reply != NULL) {
		result = xpc_dictionary_get_value(reply, OD_RPC_RESULT);
		if (result != NULL && xpc_get_type(result) == XPC_TYPE_ARRAY) {
			xpc_array_apply(result, ^bool(size_t index, xpc_object_t value) {
				si_item_t *item = extract(si, value, extra, va, vb);
				list = si_list_add(list, item);
				si_item_release(item);

				return true;
			});
		}

		xpc_release(reply);
	}

	return list;
}

static si_item_t *
_ds_item(si_mod_t *si, int cat, const char *procname, const void *extra, od_extract_t extract, xpc_object_t payload)
{
	xpc_object_t result;
	uint64_t va, vb;
	si_item_t *item = NULL;

	if (procname == NULL) return NULL;

	result = _od_rpc_call(procname, payload, _od_xpc_pipe);
	if (result != NULL) {
		_ds_get_validation(si, &va, &vb, cat);
		if (xpc_dictionary_get_int64(result, OD_RPC_ERROR) == 0) {
			item = extract(si, result, extra, va, vb);
		}

		xpc_release(result);
	}

	return item;
}

static int
_ds_is_valid(si_mod_t *si, si_item_t *item)
{
	si_mod_t *src;
	ds_si_private_t *pp;
	int status;
	uint32_t oldval, newval;

	if (si == NULL) return 0;
	if (item == NULL) return 0;
	if (si->name == NULL) return 0;
	if (item->src == NULL) return 0;

	pp = (ds_si_private_t *)si->private;
	if (pp == NULL) return 0;

	src = (si_mod_t *)item->src;

	if (src->name == NULL) return 0;
	if (string_not_equal(si->name, src->name)) return 0;

	/* check global invalidation */
	oldval = item->validation_a;
	newval = -1;
	status = notify_peek(pp->notify_token_global, &newval);
	if (status != NOTIFY_STATUS_OK) return 0;

	newval = ntohl(newval);
	if (oldval != newval) return 0;

	oldval = item->validation_b;
	newval = -1;
	if (item->type == CATEGORY_USER) status = notify_peek(pp->notify_token_user, &newval);
	else if (item->type == CATEGORY_GROUP) status = notify_peek(pp->notify_token_group, &newval);
	else if (item->type == CATEGORY_SERVICE) status = notify_peek(pp->notify_token_service, &newval);
	else return 0;

	if (status != NOTIFY_STATUS_OK) return 0;

	newval = ntohl(newval);
	if (oldval != newval) return 0;

	return 1;
}

static void
_free_addr_list(char **l)
{
	int i;

	if (l == NULL) return;
	for (i = 0; l[i] != NULL; i++) free(l[i]);
	free(l);
}

/* map ipv4 addresses and append to v6 list */
static int
_map_v4(char ***v6, uint32_t n6, char **v4, uint32_t n4)
{
	struct in6_addr a6;
	uint32_t i;

	a6.__u6_addr.__u6_addr32[0] = 0x00000000;
	a6.__u6_addr.__u6_addr32[1] = 0x00000000;
	a6.__u6_addr.__u6_addr32[2] = htonl(0x0000ffff);

	if (*v6 == NULL)
	{
		*v6 = (char **)calloc(n4 + 1, sizeof(char *));
	}
	else
	{
		*v6 = (char **)reallocf(*v6, (n6 + n4 + 1) * sizeof(char *));
	}

	if (*v6 == NULL) return -1;

	for (i = 0; i < n4; i++)
	{
		(*v6)[n6] = (char *)calloc(1, IPV6_ADDR_LEN);
		if ((*v6)[n6] == NULL) return -1;

		memcpy(&(a6.__u6_addr.__u6_addr32[3]), v4[i], IPV4_ADDR_LEN);
		memcpy((*v6)[n6], &(a6.__u6_addr.__u6_addr32[0]), IPV6_ADDR_LEN);

		n6++;
	}

	return 0;
}

static xpc_object_t
_xpc_query_key_string(const char *key, const char *value)
{
	xpc_object_t payload;

	if (value == NULL) return NULL;

	payload = xpc_dictionary_create(NULL, NULL, 0);
	if (payload == NULL) return NULL;

	xpc_dictionary_set_string(payload, key, value);

	return payload;
}

static xpc_object_t
_xpc_query_key_id(const char *key, id_t idValue)
{
	xpc_object_t payload;

	payload = xpc_dictionary_create(NULL, NULL, 0);
	if (payload == NULL) return NULL;

	xpc_dictionary_set_int64(payload, key, idValue);

	return payload;
}

static xpc_object_t
_xpc_query_key_uuid(const char *key, uuid_t uu)
{
	xpc_object_t payload;

	payload = xpc_dictionary_create(NULL, NULL, 0);
	if (payload == NULL) return NULL;

	xpc_dictionary_set_uuid(payload, key, uu);

	return payload;
}

static xpc_object_t
_xpc_query_key_int(const char *key, int64_t intValue)
{
	xpc_object_t payload;

	payload = xpc_dictionary_create(NULL, NULL, 0);
	if (payload == NULL) return NULL;

	xpc_dictionary_set_int64(payload, key, intValue);

	return payload;
}

#pragma mark -

static int
_extract_string_from_xpc_array_index(xpc_object_t reply, int index, const char **str)
{
	xpc_object_t value;

	if (xpc_array_get_count(reply) < index) return -1;

	value = xpc_array_get_value(reply, index);
	if (xpc_get_type(value) != XPC_TYPE_STRING) return -1;

	*str = xpc_string_get_string_ptr(value);
	return 0;
}

static int
_extract_string_from_xpc_object(xpc_object_t value, const char **str)
{
	if (value == NULL) return -1;
	else if (xpc_get_type(value) == XPC_TYPE_STRING)
	{
		*str = xpc_string_get_string_ptr(value);
		return 0;
	}
	else if (xpc_get_type(value) == XPC_TYPE_ARRAY)
	{
		return _extract_string_from_xpc_array_index(value, 0, str);
	}

	return -1;
}

static int
_extract_uint32_from_xpc_object(xpc_object_t value, uint32_t *val32)
{
	xpc_type_t type;

	if (value == NULL) return -1;
	type = xpc_get_type(value);

	if (type == XPC_TYPE_STRING)
	{
		*val32 = atoi(xpc_string_get_string_ptr(value));
		return 0;
	}
	else if (type == XPC_TYPE_INT64)
	{
		*val32 = (uint32_t)xpc_int64_get_value(value);
		return 0;
	}
	else if (type == XPC_TYPE_BOOL)
	{
		*val32 = (uint32_t)xpc_bool_get_value(value);
		return 0;
	}
	else if (type == XPC_TYPE_ARRAY)
	{
		if (xpc_array_get_count(value) == 0) return -1;
		return _extract_uint32_from_xpc_object(xpc_array_get_value(value, 0), val32);
	}

	return -1;
}

static int
_extract_string_list_from_xpc_array_index(xpc_object_t reply, int index, unsigned int *len, char ***list)
{
	char **result;
	xpc_object_t xpc_array = xpc_array_get_value(reply, index);

	if ((xpc_array == NULL) || (xpc_get_type(xpc_array) != XPC_TYPE_ARRAY)) return -1;

		result = calloc(xpc_array_get_count(xpc_array) + 1, sizeof(*result));
	if (result == NULL) return -1;

	/* include trailing NULL */
	if (len != NULL) (*len) = xpc_array_get_count(xpc_array) + 1;

	xpc_array_apply(xpc_array, ^bool(size_t idx, xpc_object_t value) {
		result[idx] = (char *)xpc_string_get_string_ptr(value);
		return true;
	});

	*list = result;
	return 0;
}

static int
_extract_uint32_from_xpc_array_index(xpc_object_t reply, int index, uint32_t *val32)
{
	xpc_object_t value = xpc_array_get_value(reply, index);
	return _extract_uint32_from_xpc_object(value, val32);
}

static int
_extract_string_list_from_xpc_array(xpc_object_t xpc_array, unsigned int *len, char ***list)
{
	char **result;

	if ((xpc_array == NULL) || (xpc_get_type(xpc_array) != XPC_TYPE_ARRAY)) return -1;

		result = calloc(xpc_array_get_count(xpc_array) + 1, sizeof(*result));
	if (result == NULL) return -1;

	/* include trailing NULL */
	if (len != NULL) (*len) = xpc_array_get_count(xpc_array) + 1;

	xpc_array_apply(xpc_array, ^bool(size_t idx, xpc_object_t value) {
		result[idx] = (char *)xpc_string_get_string_ptr(value);
		return true;
	});

	*list = result;
	return 0;
}

static int
_extract_string_from_xpc_dict(xpc_object_t reply, const char *key, const char **str)
{
	xpc_object_t value = xpc_dictionary_get_value(reply, key);
	const char *result;

	if (value == NULL) return -1;

	if (xpc_get_type(value) != XPC_TYPE_STRING) return -1;

	result = xpc_string_get_string_ptr(value);
	if (result == NULL) return -1;

	*str = result;
	return 0;
}

static int
_extract_uint32_from_xpc_dict(xpc_object_t reply, const char *key, uint32_t *val32)
{
	xpc_object_t value = xpc_dictionary_get_value(reply, key);
	return _extract_uint32_from_xpc_object(value, val32);
}

#pragma mark -

/*
 * user schema
 *
 *				name	: string
 *				passwd	: string
 *				uid		: uint32
 *				gid		: uint32
 *				gecos	: string
 *				dir		: string
 *				shell	: string
 */

static si_item_t *
_extract_user_array(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	struct passwd tmp;
	int i = 0;

	if (xpc_array_get_count(reply) < 7) return NULL;

	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.pw_name)) return NULL;
	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.pw_passwd)) return NULL;
	if (0 != _extract_uint32_from_xpc_array_index(reply, i++, (uint32_t *)&tmp.pw_uid)) return NULL;
	if (0 != _extract_uint32_from_xpc_array_index(reply, i++, (uint32_t *)&tmp.pw_gid)) return NULL;
	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.pw_gecos)) return NULL;
	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.pw_dir)) return NULL;
	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.pw_shell)) return NULL;

	/* default values */
	tmp.pw_change = (time_t)0;
	tmp.pw_expire = (time_t)0;
	tmp.pw_class = (char *)"";

	return (si_item_t *)LI_ils_create("L4488ss44LssssL", (unsigned long)si, CATEGORY_USER, 1, valid_global, valid_cat, tmp.pw_name, tmp.pw_passwd, tmp.pw_uid, tmp.pw_gid, tmp.pw_change, tmp.pw_class, tmp.pw_gecos, tmp.pw_dir, tmp.pw_shell, tmp.pw_expire);
}

static si_item_t *
_extract_user_dict(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	__block struct passwd tmp;
	__block int status = 0;
	__block int parts = 3;

	tmp.pw_name = (char *)"";
	tmp.pw_passwd = (char *)"*";
	tmp.pw_uid = (uid_t)0;
	tmp.pw_gid = (gid_t)0;
	tmp.pw_change = (time_t)0;
	tmp.pw_expire = (time_t)0;
	tmp.pw_class = (char *)"";
	tmp.pw_gecos = (char *)"";
	tmp.pw_dir = (char *)"/var/empty";
	tmp.pw_shell = (char *)"/usr/bin/false";

	xpc_dictionary_apply(reply, ^bool(const char *key, xpc_object_t value) {
		if (key == NULL) return true;
		else if (!strcmp(key, "pw_name"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.pw_name);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "pw_passwd"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.pw_passwd);
			/* no parts check - this value is optional */
		}
		else if (!strcmp(key, "pw_uid"))
		{
			status |= _extract_uint32_from_xpc_object(value, (uint32_t *)&tmp.pw_uid);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "pw_gid"))
		{
			status |= _extract_uint32_from_xpc_object(value, (uint32_t *)&tmp.pw_gid);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "pw_change"))
		{
			status |= _extract_uint32_from_xpc_object(value, (uint32_t *)&tmp.pw_change);
			/* no parts check - this value is optional */
		}
		else if (!strcmp(key, "pw_expire"))
		{
			status |= _extract_uint32_from_xpc_object(value, (uint32_t *)&tmp.pw_expire);
			/* no parts check - this value is optional */
		}
		else if (!strcmp(key, "pw_class"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.pw_class);
			/* no parts check - this value is optional */
		}
		else if (!strcmp(key, "pw_gecos"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.pw_gecos);
			/* no parts check - this value is optional */
		}
		else if (!strcmp(key, "pw_dir"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.pw_dir);
			/* no parts check - this value is optional */
		}
		else if (!strcmp(key, "pw_shell"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.pw_shell);
			/* no parts check - this value is optional */
		}
		return true;
	});

	if ((status != 0) || (parts != 0)) return NULL;

	return (si_item_t *)LI_ils_create("L4488ss44LssssL", (unsigned long)si, CATEGORY_USER, 1, valid_global, valid_cat, tmp.pw_name, tmp.pw_passwd, tmp.pw_uid, tmp.pw_gid, tmp.pw_change, tmp.pw_class, tmp.pw_gecos, tmp.pw_dir, tmp.pw_shell, tmp.pw_expire);
}

static si_item_t *
_extract_user(si_mod_t *si, xpc_object_t reply, __unused const void *ignored, uint64_t valid_global, uint64_t valid_cat)
{
	xpc_type_t type;

	if (si == NULL) return NULL;
	if (reply == NULL) return NULL;

	type = xpc_get_type(reply);

	if (type == XPC_TYPE_ARRAY) return _extract_user_array(si, reply, valid_global, valid_cat);
	else if (type == XPC_TYPE_DICTIONARY) return _extract_user_dict(si, reply, valid_global, valid_cat);

	return NULL;
}

/*
 * group schema
 *
 *				name	: string
 *				gid		: uint32
 * optional		members	: array of string
 *
 */

static si_item_t *
_extract_group_array(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	si_item_t *item;
	struct group tmp;
	int i = 0;
	int arraycount = xpc_array_get_count(reply);

	if ((arraycount < 2) || (arraycount > 3)) return NULL;

	memset(&tmp, 0, sizeof(tmp));

	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.gr_name)) return NULL;
	if (0 != _extract_uint32_from_xpc_array_index(reply, i++, (uint32_t *)&tmp.gr_gid)) return NULL;

	if (arraycount == 3)
	{
		if (0 != _extract_string_list_from_xpc_array_index(reply, i++, NULL, (char ***)&tmp.gr_mem)) return NULL;
	}

	/* default value */
	tmp.gr_passwd = (char *)"*";

	item = (si_item_t *) LI_ils_create("L4488ss4*", (unsigned long)si, CATEGORY_GROUP, 1, valid_global, valid_cat, tmp.gr_name, tmp.gr_passwd, tmp.gr_gid, tmp.gr_mem);

	free(tmp.gr_mem);

	return item;
}

static si_item_t *
_extract_group_dict(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	si_item_t *item;
	__block struct group tmp;
	__block int status = 0;
	__block int parts = 2;

	tmp.gr_name = (char *)"";
	tmp.gr_passwd = (char *)"*";
	tmp.gr_gid = (gid_t)0;
	tmp.gr_mem = NULL;

	xpc_dictionary_apply(reply, ^bool(const char *key, xpc_object_t value) {
		if (key == NULL) return true;
		else if (!strcmp(key, "gr_name"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.gr_name);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "gr_passwd"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.gr_passwd);
			/* no parts check - this value is optional */
		}
		else if (!strcmp(key, "gr_gid"))
		{
			status |= _extract_uint32_from_xpc_object(value, (uint32_t *)&tmp.gr_gid);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "gr_mem"))
		{
			status |= _extract_string_list_from_xpc_array(value, NULL, (char ***)&tmp.gr_mem);
			/* no parts check - this value is optional */
		}
		return true;
	});

	if ((status != 0) || (parts != 0))
	{
		free(tmp.gr_mem);
		return NULL;
	}

	item = (si_item_t *) LI_ils_create("L4488ss4*", (unsigned long)si, CATEGORY_GROUP, 1, valid_global, valid_cat, tmp.gr_name, tmp.gr_passwd, tmp.gr_gid, tmp.gr_mem);

	free(tmp.gr_mem);

	return item;
}

static si_item_t *
_extract_group(si_mod_t *si, xpc_object_t reply, __unused const void *ignored, uint64_t valid_global, uint64_t valid_cat)
{
	xpc_type_t type;

	if (si == NULL) return NULL;
	if (reply == NULL) return NULL;

	type = xpc_get_type(reply);
	if (type == XPC_TYPE_ARRAY) return _extract_group_array(si, reply, valid_global, valid_cat);
	else if (type == XPC_TYPE_DICTIONARY) return _extract_group_dict(si, reply, valid_global, valid_cat);

	return NULL;
}

/*
 * netgroup schema
 *
 *				host	: string
 *				user	: string
 *				domain	: string
 *
 */
static si_item_t *
_extract_netgroup_array(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	const char *host, *user, *domain;
	int i = 0;

	if (xpc_array_get_count(reply) != 3) return NULL;

	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&host)) return NULL;
	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&user)) return NULL;
	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&domain)) return NULL;

	return (si_item_t *)LI_ils_create("L4488sss", (unsigned long)si, CATEGORY_NETGROUP, 1, valid_global, valid_cat, host, user, domain);
}

static si_item_t *
_extract_netgroup_dict(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	__block const char *host = "";
	__block const char *user = "";
	__block const char *domain = "";
	__block int status = 0;
	__block int parts = 3;

	xpc_dictionary_apply(reply, ^bool(const char *key, xpc_object_t value) {
		if (key == NULL) return true;
		else if (!strcmp(key, "host"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&host);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "user"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&user);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "domain"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&domain);
			if (status == 0) parts--;
		}
		return true;
	});

	if ((status != 0) || (parts != 0)) return NULL;

	return (si_item_t *)LI_ils_create("L4488sss", (unsigned long)si, CATEGORY_NETGROUP, 1, valid_global, valid_cat, host, user, domain);
}

static si_item_t *
_extract_netgroup(si_mod_t *si, xpc_object_t reply, const void *ignored, uint64_t valid_global, uint64_t valid_cat)
{
	xpc_type_t type;

	if (si == NULL) return NULL;
	if (reply == NULL) return NULL;

	type = xpc_get_type(reply);
	if (type == XPC_TYPE_ARRAY) return _extract_netgroup_array(si, reply, valid_global, valid_cat);
	else if (type == XPC_TYPE_DICTIONARY) return _extract_netgroup_dict(si, reply, valid_global, valid_cat);

	return NULL;
}

/*
 * alias schema
 *
 *				name	: string
 *				local	: uint32
 * optional		members	: array of string
 *
 */

static si_item_t *
_extract_alias_array(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	si_item_t *item;
	struct aliasent tmp;
	int i = 0;
	int arraycount = xpc_array_get_count(reply);

	if ((arraycount < 2) || (arraycount > 3)) return NULL;

	memset(&tmp, 0, sizeof(tmp));

	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.alias_name)) return NULL;
	if (0 != _extract_uint32_from_xpc_array_index(reply, i++, (uint32_t *)&tmp.alias_local)) return NULL;

	if (arraycount == 3)
	{
		if (0 != _extract_string_list_from_xpc_array_index(reply, i++, NULL, (char ***)&tmp.alias_members)) return NULL;
	}

	item = (si_item_t *)LI_ils_create("L4488s4*4", (unsigned long)si, CATEGORY_ALIAS, 1, valid_global, valid_cat, tmp.alias_name, tmp.alias_members_len, tmp.alias_members, tmp.alias_local);

	free(tmp.alias_members);

	return item;
}

static si_item_t *
_extract_alias_dict(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	si_item_t *item;
	__block struct aliasent tmp;
	__block int status = 0;
	__block int parts = 2;

	tmp.alias_name = (char *)"";
	tmp.alias_local = 0;
	tmp.alias_members = NULL;
	tmp.alias_members_len = 0;

	xpc_dictionary_apply(reply, ^bool(const char *key, xpc_object_t value) {
		if (key == NULL) return true;
		else if (!strcmp(key, "alias_name"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.alias_name);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "alias_local"))
		{
			status |= _extract_uint32_from_xpc_object(value, (uint32_t *)&tmp.alias_local);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "alias_members"))
		{
			status |= _extract_string_list_from_xpc_array(value, &tmp.alias_members_len, (char ***)&tmp.alias_members);
			/* no parts check - this value is optional */
		}
		return true;
	});

	if ((status != 0) || (parts != 0))
	{
		free(tmp.alias_members);
		return NULL;
	}

	item = (si_item_t *)LI_ils_create("L4488s4*4", (unsigned long)si, CATEGORY_ALIAS, 1, valid_global, valid_cat, tmp.alias_name, tmp.alias_members_len, tmp.alias_members, tmp.alias_local);

	free(tmp.alias_members);

	return item;
}

static si_item_t *
_extract_alias(si_mod_t *si, xpc_object_t reply, __unused const void *ignored, uint64_t valid_global, uint64_t valid_cat)
{
	xpc_type_t type;

	if (si == NULL) return NULL;
	if (reply == NULL) return NULL;

	type = xpc_get_type(reply);
	if (type == XPC_TYPE_ARRAY) return _extract_alias_array(si, reply, valid_global, valid_cat);
	else if (type == XPC_TYPE_DICTIONARY) return _extract_alias_dict(si, reply, valid_global, valid_cat);

	return NULL;
}

/*
 * network schema
 *
 *				name	: string
 *				net		: uint32
 * optional		aliases	: array of string
 *
 */

static si_item_t *
_extract_network_array(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	si_item_t *item;
	struct netent tmp;
	int i = 0;
	int arraycount = xpc_array_get_count(reply);

	if ((arraycount < 2) || (arraycount > 3)) return NULL;

	memset(&tmp, 0, sizeof(tmp));

	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.n_name)) return NULL;
	if (0 != _extract_uint32_from_xpc_array_index(reply, i++, (uint32_t *)&tmp.n_net)) return NULL;

	if (arraycount == 3)
	{
		if (0 != _extract_string_list_from_xpc_array_index(reply, i++, NULL, (char ***)&tmp.n_aliases)) return NULL;
	}

	/* default value */
	tmp.n_addrtype = AF_INET;

	item = (si_item_t *)LI_ils_create("L4488s*44", (unsigned long)si, CATEGORY_NETWORK, 1, valid_global, valid_cat, tmp.n_name, tmp.n_aliases, tmp.n_addrtype, tmp.n_net);

	free(tmp.n_aliases);

	return item;
}

static si_item_t *
_extract_network_dict(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	si_item_t *item;
	__block struct netent tmp;
	__block int status = 0;
	__block int parts = 2;

	if (si == NULL) return NULL;
	if (reply == NULL) return NULL;

	tmp.n_name = (char *)"";
	tmp.n_aliases = NULL;
	tmp.n_net = 0;

	/* default value */
	tmp.n_addrtype = AF_INET;

	xpc_dictionary_apply(reply, ^bool(const char *key, xpc_object_t value) {
		if (key == NULL) return true;
		else if (!strcmp(key, "n_name"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.n_name);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "n_aliases"))
		{
			status |= _extract_string_list_from_xpc_array(value, NULL, (char ***)&tmp.n_aliases);
			/* no parts check - this value is optional */
		}
		else if (!strcmp(key, "n_net"))
		{
			status |= _extract_uint32_from_xpc_object(value, (uint32_t *)&tmp.n_net);
			if (status == 0) parts--;
		}
		return true;
	});

	if ((status != 0) || (parts != 0))
	{
		free(tmp.n_aliases);
		return NULL;
	}

	item = (si_item_t *)LI_ils_create("L4488s*44", (unsigned long)si, CATEGORY_NETWORK, 1, valid_global, valid_cat, tmp.n_name, tmp.n_aliases, tmp.n_addrtype, tmp.n_net);

	free(tmp.n_aliases);

	return item;
}

static si_item_t *
_extract_network(si_mod_t *si, xpc_object_t reply, __unused const void *ignored, uint64_t valid_global, uint64_t valid_cat)
{
	xpc_type_t type;

	if (si == NULL) return NULL;
	if (reply == NULL) return NULL;

	type = xpc_get_type(reply);
	if (type == XPC_TYPE_ARRAY) return _extract_network_array(si, reply, valid_global, valid_cat);
	else if (type == XPC_TYPE_DICTIONARY) return _extract_network_dict(si, reply, valid_global, valid_cat);

	return NULL;
}

/*
 * service schema
 *
 *				name	: string
 *				port	: uint32
 *				proto	: string
 * optional		aliases	: array of string
 *
 */

static si_item_t *
_extract_service_array(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	si_item_t *item;
	struct servent tmp;
	int i = 0;
	int arraycount = xpc_array_get_count(reply);

	if ((arraycount < 3) || (arraycount > 4)) return NULL;

	memset(&tmp, 0, sizeof(tmp));

	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.s_name)) return NULL;
	if (0 != _extract_uint32_from_xpc_array_index(reply, i++, (uint32_t *)&tmp.s_port)) return NULL;
	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.s_proto)) return NULL;

	if (arraycount == 4)
	{
		if (0 != _extract_string_list_from_xpc_array_index(reply, i++, NULL, (char ***)&tmp.s_aliases)) return NULL;
	}

	item = (si_item_t *)LI_ils_create("L4488s*4s", (unsigned long)si, CATEGORY_SERVICE, 1, valid_global, valid_cat, tmp.s_name, tmp.s_aliases, tmp.s_port, tmp.s_proto);

	free(tmp.s_aliases);

	return item;
}

static si_item_t *
_extract_service_dict(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	si_item_t *item;
	__block struct servent tmp;
	__block int status = 0;
	__block int parts = 3;

	if (si == NULL) return NULL;
	if (reply == NULL) return NULL;

	tmp.s_name = (char *)"";
	tmp.s_aliases = NULL;
	tmp.s_port = 0;
	tmp.s_proto = (char *)"";

	xpc_dictionary_apply(reply, ^bool(const char *key, xpc_object_t value) {
		if (key == NULL) return true;
		else if (!strcmp(key, "s_name"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.s_name);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "s_aliases"))
		{
			status |= _extract_string_list_from_xpc_array(value, NULL, (char ***)&tmp.s_aliases);
			/* no parts check - this value is optional */
		}
		else if (!strcmp(key, "s_port"))
		{
			uint32_t v32;
			status |= _extract_uint32_from_xpc_object(value, &v32);
			if (status == 0)
			{
				tmp.s_port = (unsigned int)htons(v32); // ugh
				parts--;
			}
		}
		else if (!strcmp(key, "s_proto"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.s_proto);
			if (status == 0) parts--;
		}
		return true;
	});

	if ((status != 0) || (parts != 0))
	{
		free(tmp.s_aliases);
		return NULL;
	}

	item = (si_item_t *)LI_ils_create("L4488s*4s", (unsigned long)si, CATEGORY_SERVICE, 1, valid_global, valid_cat, tmp.s_name, tmp.s_aliases, tmp.s_port, tmp.s_proto);

	free(tmp.s_aliases);

	return item;
}

static si_item_t *
_extract_service(si_mod_t *si, xpc_object_t reply, __unused const void *ignored, uint64_t valid_global, uint64_t valid_cat)
{
	xpc_type_t type;

	if (si == NULL) return NULL;
	if (reply == NULL) return NULL;

	type = xpc_get_type(reply);
	if (type == XPC_TYPE_ARRAY) return _extract_service_array(si, reply, valid_global, valid_cat);
	else if (type == XPC_TYPE_DICTIONARY) return _extract_service_dict(si, reply, valid_global, valid_cat);

	return NULL;
}

/*
 * protocol schema
 *
 *				name	: string
 *				proto	: uint32
 * optional		aliases	: array of string
 *
 */
static si_item_t *
_extract_protocol_array(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	si_item_t *item;
	struct protoent tmp;
	int i = 0;
	int arraycount = xpc_array_get_count(reply);

	if ((arraycount < 2) || (arraycount > 3)) return NULL;

	memset(&tmp, 0, sizeof(tmp));

	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.p_name)) return NULL;
	if (0 != _extract_uint32_from_xpc_array_index(reply, i++, (uint32_t *)&tmp.p_proto)) return NULL;

	if (arraycount == 3)
	{
		if (0 != _extract_string_list_from_xpc_array_index(reply, i++, NULL, (char ***)&tmp.p_aliases)) return NULL;
	}

	item = (si_item_t *)LI_ils_create("L4488s*4", (unsigned long)si, CATEGORY_PROTOCOL, 1, valid_global, valid_cat, tmp.p_name, tmp.p_aliases, tmp.p_proto);

	free(tmp.p_aliases);

	return item;
}

static si_item_t *
_extract_protocol_dict(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	si_item_t *item;
	__block struct protoent tmp;
	__block int status = 0;
	__block int parts = 2;

	tmp.p_name = (char *)"";
	tmp.p_proto = 0;
	tmp.p_aliases = NULL;

	xpc_dictionary_apply(reply, ^bool(const char *key, xpc_object_t value) {
		if (key == NULL) return true;
		else if (!strcmp(key, "p_name"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.p_name);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "p_proto"))
		{
			status |= _extract_uint32_from_xpc_object(value, (uint32_t *)&tmp.p_proto);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "p_aliases"))
		{
			status |= _extract_string_list_from_xpc_array(value, NULL, (char ***)&tmp.p_aliases);
			/* no parts check - this value is optional */
		}
		return true;
	});

	if ((status != 0) || (parts != 0))
	{
		free(tmp.p_aliases);
		return NULL;
	}

	item = (si_item_t *)LI_ils_create("L4488s*4", (unsigned long)si, CATEGORY_PROTOCOL, 1, valid_global, valid_cat, tmp.p_name, tmp.p_aliases, tmp.p_proto);

	free(tmp.p_aliases);

	return item;
}

static si_item_t *
_extract_protocol(si_mod_t *si, xpc_object_t reply, __unused const void *ignored, uint64_t valid_global, uint64_t valid_cat)
{
	xpc_type_t type;

	if (si == NULL) return NULL;
	if (reply == NULL) return NULL;

	type = xpc_get_type(reply);
	if (type == XPC_TYPE_ARRAY) return _extract_protocol_array(si, reply, valid_global, valid_cat);
	else if (type == XPC_TYPE_DICTIONARY) return _extract_protocol_dict(si, reply, valid_global, valid_cat);

	return NULL;
}

/*
 * rpc schema
 *
 *				name	: string
 *				number	: uint32
 * optional		aliases	: array of string
 *
 */

static si_item_t *
_extract_rpc_array(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	si_item_t *item;
	struct rpcent tmp;
	int i = 0;
	int arraycount = xpc_array_get_count(reply);

	if ((arraycount < 2) || (arraycount > 3)) return NULL;

	memset(&tmp, 0, sizeof(tmp));

	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.r_name)) return NULL;
	if (0 != _extract_uint32_from_xpc_array_index(reply, i++, (uint32_t *)&tmp.r_number)) return NULL;

	if (arraycount == 3)
	{
		if (0 != _extract_string_list_from_xpc_array_index(reply, i++, NULL, (char ***)&tmp.r_aliases)) return NULL;
	}

	item = (si_item_t *)LI_ils_create("L4488s*4", (unsigned long)si, CATEGORY_RPC, 1, valid_global, valid_cat, tmp.r_name, tmp.r_aliases, tmp.r_number);

	free(tmp.r_aliases);

	return item;
}

static si_item_t *
_extract_rpc_dict(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	si_item_t *item;
	__block struct rpcent tmp;
	__block int status = 0;
	__block int parts = 2;

	tmp.r_name = (char *)"";
	tmp.r_number = 0;
	tmp.r_aliases = NULL;

	xpc_dictionary_apply(reply, ^bool(const char *key, xpc_object_t value) {
		if (key == NULL) return true;
		else if (!strcmp(key, "r_name"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.r_name);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "r_number"))
		{
			status |= _extract_uint32_from_xpc_object(value, (uint32_t *)&tmp.r_number);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "r_aliases"))
		{
			status |= _extract_string_list_from_xpc_array(value, NULL, (char ***)&tmp.r_aliases);
			/* no parts check - this value is optional */
		}
		return true;
	});

	if ((status != 0) || (parts != 0))
	{
		free(tmp.r_aliases);
		return NULL;
	}

	item = (si_item_t *)LI_ils_create("L4488s*4", (unsigned long)si, CATEGORY_RPC, 1, valid_global, valid_cat, tmp.r_name, tmp.r_aliases, tmp.r_number);

	free(tmp.r_aliases);

	return item;
}

static si_item_t *
_extract_rpc(si_mod_t *si, xpc_object_t reply, __unused const void *ignored, uint64_t valid_global, uint64_t valid_cat)
{
	xpc_type_t type;

	if (si == NULL) return NULL;
	if (reply == NULL) return NULL;

	type = xpc_get_type(reply);
	if (type == XPC_TYPE_ARRAY) return _extract_rpc_array(si, reply, valid_global, valid_cat);
	else if (type == XPC_TYPE_DICTIONARY) return _extract_rpc_dict(si, reply, valid_global, valid_cat);

	return NULL;
}

/*
 * fstab schema
 *
 *				file	: string
 *				spec	: string
 *				freq	: uint32
 *				passno	: uint32
 *				mntopts	: string
 *				type	: string
 *				vfstype	: string
 */

static si_item_t *
_extract_fstab_array(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	__block struct fstab tmp;
	int i = 0;

	if (xpc_array_get_count(reply) != 7) return NULL;

	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.fs_file)) return NULL;
	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.fs_spec)) return NULL;
	if (0 != _extract_uint32_from_xpc_array_index(reply, i++, (uint32_t *)&tmp.fs_freq)) return NULL;
	if (0 != _extract_uint32_from_xpc_array_index(reply, i++, (uint32_t *)&tmp.fs_passno)) return NULL;
	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.fs_mntops)) return NULL;
	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.fs_type)) return NULL;
	if (0 != _extract_string_from_xpc_array_index(reply, i++, (const char **)&tmp.fs_vfstype)) return NULL;

	return (si_item_t *)LI_ils_create("L4488sssss44", (unsigned long)si, CATEGORY_FS, 1, valid_global, valid_cat, tmp.fs_spec, tmp.fs_file, tmp.fs_vfstype, tmp.fs_mntops, tmp.fs_type, tmp.fs_freq, tmp.fs_passno);
}

static si_item_t *
_extract_fstab_dict(si_mod_t *si, xpc_object_t reply, uint64_t valid_global, uint64_t valid_cat)
{
	__block struct fstab tmp;
	__block int status = 0;
	__block int parts = 7;

	tmp.fs_file = NULL;
	tmp.fs_spec = (char *)"";
	tmp.fs_freq = 0;
	tmp.fs_passno = 0;
	tmp.fs_mntops = (char *)"";
	tmp.fs_type = (char *)"";
	tmp.fs_vfstype = (char *)"";

	xpc_dictionary_apply(reply, ^bool(const char *key, xpc_object_t value) {
		if (key == NULL) return true;
		else if (!strcmp(key, "fs_file"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.fs_file);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "fs_spec"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.fs_spec);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "fs_freq"))
		{
			status |= _extract_uint32_from_xpc_object(value, (uint32_t *)&tmp.fs_freq);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "fs_passno"))
		{
			status |= _extract_uint32_from_xpc_object(value, (uint32_t *)&tmp.fs_passno);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "fs_mntops"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.fs_mntops);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "fs_type"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.fs_type);
			if (status == 0) parts--;
		}
		else if (!strcmp(key, "fs_vfstype"))
		{
			status |= _extract_string_from_xpc_object(value, (const char **)&tmp.fs_vfstype);
			if (status == 0) parts--;
		}
		return true;
	});

	if ((status != 0) || (parts != 0)) return NULL;

	return (si_item_t *)LI_ils_create("L4488sssss44", (unsigned long)si, CATEGORY_FS, 1, valid_global, valid_cat, tmp.fs_spec, tmp.fs_file, tmp.fs_vfstype, tmp.fs_mntops, tmp.fs_type, tmp.fs_freq, tmp.fs_passno);
}

static si_item_t *
_extract_fstab(si_mod_t *si, xpc_object_t reply, __unused const void *ignored, uint64_t valid_global, uint64_t valid_cat)
{
	xpc_type_t type;

	if (si == NULL) return NULL;
	if (reply == NULL) return NULL;

	type = xpc_get_type(reply);
	if (type == XPC_TYPE_ARRAY) return _extract_fstab_array(si, reply, valid_global, valid_cat);
	else if (type == XPC_TYPE_DICTIONARY) return _extract_fstab_dict(si, reply, valid_global, valid_cat);

	return NULL;
}

static si_item_t *
_extract_mac_mac(si_mod_t *si, xpc_object_t reply, const void *extra, uint64_t valid_global, uint64_t valid_cat)
{
	xpc_type_t type;
	char *cmac;
	const char *value = NULL;
	si_item_t *out;

	if (si == NULL) return NULL;
	if (reply == NULL) return NULL;
	if (extra == NULL) return NULL;

	type = xpc_get_type(reply);
	if (type == XPC_TYPE_ARRAY)
	{
		if (xpc_array_get_count(reply) >= 1)
		{
			if (0 != _extract_string_from_xpc_array_index(reply, 0, (const char **)&value)) return NULL;
		}
	}
	else if (type == XPC_TYPE_DICTIONARY)
	{
		if (0 != _extract_string_from_xpc_dict(reply, "mac", &value)) return NULL;
	}

	if (value == NULL || value[0] == '\0') return NULL;

	cmac = si_standardize_mac_address(value);
	if (cmac == NULL) return NULL;

	out = (si_item_t *)LI_ils_create("L4488ss", (unsigned long)si, CATEGORY_MAC, 1, valid_global, valid_cat, extra, cmac);

	free(cmac);

	return out;
}

static si_item_t *
_extract_mac_name(si_mod_t *si, xpc_object_t reply, const void *extra, uint64_t valid_global, uint64_t valid_cat)
{
	xpc_type_t type;
	const char *name = NULL;
	si_item_t *out;

	if (si == NULL) return NULL;
	if (reply == NULL) return NULL;
	if (extra == NULL) return NULL;

	type = xpc_get_type(reply);
	if (type == XPC_TYPE_ARRAY)
	{
		if (xpc_array_get_count(reply) >= 1)
		{
			if (0 != _extract_string_from_xpc_array_index(reply, 0, (const char **)&name )) return NULL;
		}
	}
	else if (type == XPC_TYPE_DICTIONARY)
	{
		if (0 != _extract_string_from_xpc_dict(reply, "name", &name)) return NULL;
	}

	if (name == NULL) return NULL;

	out = (si_item_t *)LI_ils_create("L4488ss", (unsigned long)si, CATEGORY_MAC, 1, valid_global, valid_cat, name, extra);

	return out;
}

#pragma mark -

static si_item_t *
ds_user_byname(si_mod_t *si, const char *name)
{
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = _xpc_query_key_string("name", name);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_USER, "getpwnam", NULL, _extract_user, payload);

	xpc_release(payload);
	return item;
}

static si_item_t *
ds_user_byuid(si_mod_t *si, uid_t uid)
{
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = _xpc_query_key_id("uid", uid);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_USER, "getpwuid", NULL, _extract_user, payload);

	xpc_release(payload);
	return item;
}

static si_item_t *
ds_user_byuuid(si_mod_t *si, uuid_t uuid)
{
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = _xpc_query_key_uuid("uuid", uuid);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_USER, "getpwuuid", NULL, _extract_user, payload);

	xpc_release(payload);
	return item;
}

static si_list_t *
ds_user_all(si_mod_t *si)
{
	return _ds_list(si, CATEGORY_USER, "getpwent", NULL, _extract_user);
}

static si_item_t *
ds_group_byname(si_mod_t *si, const char *name)
{
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = _xpc_query_key_string("name", name);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_GROUP, "getgrnam", NULL, _extract_group, payload);

	xpc_release(payload);
	return item;
}

static si_item_t *
ds_group_bygid(si_mod_t *si, gid_t gid)
{
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = _xpc_query_key_id("gid", gid);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_GROUP, "getgrgid", NULL, _extract_group, payload);

	xpc_release(payload);
	return item;
}

static si_item_t *
ds_group_byuuid(si_mod_t *si, uuid_t uuid)
{
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = _xpc_query_key_uuid("uuid", uuid);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_GROUP, "getgruuid", NULL, _extract_group, payload);

	xpc_release(payload);
	return item;
}

static si_list_t *
ds_group_all(si_mod_t *si)
{
	if (!_od_running()) return NULL;
	return _ds_list(si, CATEGORY_GROUP, "getgrent", NULL, _extract_group);
}

static si_item_t *
ds_grouplist(si_mod_t *si, const char *name, uint32_t ngroups)
{
	xpc_object_t payload, reply;
	si_item_t *item = NULL;

	if (!_od_running()) return NULL;
	if (name == NULL) return NULL;

	payload = xpc_dictionary_create(NULL, NULL, 0);
	if (payload == NULL) return NULL;

	xpc_dictionary_set_string(payload, "name", name);
	xpc_dictionary_set_int64(payload, "ngroups", ngroups);

	reply = _od_rpc_call("getgrouplist", payload, _od_xpc_pipe);
	if (reply != NULL) {
		size_t gidptrsz;
		const gid_t *gidptr = xpc_dictionary_get_data(reply, "groups", &gidptrsz);
		uint32_t count = 0;
		uint64_t va, vb;

		_ds_get_validation(si, &va, &vb, CATEGORY_GROUPLIST);

		/* see what we were sent */
		if (0 == _extract_uint32_from_xpc_dict(reply, "count", &count))
		{
			if (count != 0)
			{
				item = (si_item_t *)LI_ils_create("L4488s4@", (unsigned long)si, CATEGORY_GROUPLIST, 1, va, vb, name, count,
												  gidptrsz, gidptr);
			}
		}

		xpc_release(reply);
	}

	xpc_release(payload);

	return item;
}

static si_list_t *
ds_netgroup_byname(si_mod_t *si, const char *name)
{
	xpc_object_t payload;
	si_list_t *list = NULL;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = _xpc_query_key_string("netgroup", name);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_NETGROUP, "getnetgrent", NULL, _extract_netgroup, payload);
	if (item != NULL) {
		list = si_list_add(list, item);
		si_item_release(item);
	}

	xpc_release(payload);

	return list;
}

static int
ds_in_netgroup(si_mod_t *si, const char *group, const char *host, const char *user, const char *domain)
{
	xpc_object_t payload, reply;
	int is_innetgr;

	if (!_od_running()) return 0;

	payload = xpc_dictionary_create(NULL, NULL, 0);
	if (payload == NULL) return 0;

	xpc_dictionary_set_string(payload, "netgroup", (group ? group : ""));
	xpc_dictionary_set_string(payload, "host", (host ? host : ""));
	xpc_dictionary_set_string(payload, "user", (user ? user : ""));
	xpc_dictionary_set_string(payload, "domain", (domain ? domain : ""));

	reply = _od_rpc_call("innetgr", payload, _od_xpc_pipe);
	if (reply != NULL) {
		is_innetgr = xpc_dictionary_get_bool(reply, OD_RPC_RESULT);
		xpc_release(reply);
	} else {
		is_innetgr = 0;
	}

	xpc_release(payload);

	return is_innetgr;
}

static si_item_t *
ds_alias_byname(si_mod_t *si, const char *name)
{
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = _xpc_query_key_string("name", name);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_ALIAS, "alias_getbyname", NULL, _extract_alias, payload);

	xpc_release(payload);
	return item;
}

static si_list_t *
ds_alias_all(si_mod_t *si)
{
	if (!_od_running()) return NULL;
	return _ds_list(si, CATEGORY_ALIAS, "alias_getent", NULL, _extract_alias);
}

static si_item_t *
ds_network_byname(si_mod_t *si, const char *name)
{
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = _xpc_query_key_string("name", name);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_NETWORK, "getnetbyname", NULL, _extract_network, payload);

	xpc_release(payload);
	return item;
}

static si_item_t *
ds_network_byaddr(si_mod_t *si, uint32_t addr)
{
	unsigned char f1, f2, f3;
	char val[64];
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	f1 = addr & 0xff;
	addr >>= 8;
	f2 = addr & 0xff;
	addr >>= 8;
	f3 = addr & 0xff;

	if (f3 != 0) snprintf(val, sizeof(val), "%u.%u.%u", f3, f2, f1);
	else if (f2 != 0) snprintf(val, sizeof(val), "%u.%u", f2, f1);
	else snprintf(val, sizeof(val), "%u", f1);

	payload = _xpc_query_key_string("net", val);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_NETWORK, "getnetbyaddr", NULL, _extract_network, payload);

	xpc_release(payload);
	return item;
}

static si_list_t *
ds_network_all(si_mod_t *si)
{
	if (!_od_running()) return NULL;
	return _ds_list(si, CATEGORY_NETWORK, "getnetent", NULL, _extract_network);
}

static si_item_t *
ds_service_byname(si_mod_t *si, const char *name, const char *proto)
{
	xpc_object_t payload;
	si_item_t *item;
	struct servent *s;

	if (!_od_running()) return NULL;
	if (name == NULL) name = "";
	if (proto == NULL) proto = "";

	/* Check our local service cache (see ds_addrinfo). */
	item = pthread_getspecific(_ds_serv_cache_key);
	if (item != NULL)
	{
		s = (struct servent *)((uintptr_t)item + sizeof(si_item_t));
		if (string_equal(name, s->s_name)) return si_item_retain(item);
	}

	payload = xpc_dictionary_create(NULL, NULL, 0);
	if (payload == NULL) return NULL;

	xpc_dictionary_set_string(payload, "name", name);
	xpc_dictionary_set_string(payload, "proto", proto);

	item = _ds_item(si, CATEGORY_SERVICE, "getservbyname", NULL, _extract_service, payload);

	xpc_release(payload);

	return item;
}

static si_item_t *
ds_service_byport(si_mod_t *si, int port, const char *proto)
{
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = xpc_dictionary_create(NULL, NULL, 0);
	if (payload == NULL) return NULL;

	/* swap to native order, API passes network order */
	xpc_dictionary_set_int64(payload, "port", ntohs(port));
	xpc_dictionary_set_string(payload, "proto", (proto ? proto : ""));

	item = _ds_item(si, CATEGORY_SERVICE, "getservbyport", NULL, _extract_service, payload);

	xpc_release(payload);

	return item;
}

static si_list_t *
ds_service_all(si_mod_t *si)
{
	if (!_od_running()) return NULL;
	return _ds_list(si, CATEGORY_SERVICE, "getservent", NULL, _extract_service);
}

static si_item_t *
ds_protocol_byname(si_mod_t *si, const char *name)
{
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = _xpc_query_key_string("name", name);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_PROTOCOL, "getprotobyname", NULL, _extract_protocol, payload);

	xpc_release(payload);
	return item;
}

static si_item_t *
ds_protocol_bynumber(si_mod_t *si, int number)
{
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = _xpc_query_key_int("number", number);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_PROTOCOL, "getprotobynumber", NULL, _extract_protocol, payload);

	xpc_release(payload);
	return item;
}

static si_list_t *
ds_protocol_all(si_mod_t *si)
{
	return _ds_list(si, CATEGORY_PROTOCOL, "getprotoent", NULL, _extract_protocol);
}

static si_item_t *
ds_rpc_byname(si_mod_t *si, const char *name)
{
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = _xpc_query_key_string("name", name);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_RPC, "getrpcbyname", NULL, _extract_rpc, payload);

	xpc_release(payload);
	return item;
}

static si_item_t *
ds_rpc_bynumber(si_mod_t *si, int number)
{
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = _xpc_query_key_int("number", number);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_RPC, "getrpcbynumber", NULL, _extract_rpc, payload);

	xpc_release(payload);
	return item;
}

static si_list_t *
ds_rpc_all(si_mod_t *si)
{
	return _ds_list(si, CATEGORY_RPC, "getrpcent", NULL, _extract_rpc);
}

static si_item_t *
ds_fs_byspec(si_mod_t *si, const char *name)
{
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = _xpc_query_key_string("name", name);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_FS, "getfsbyname", NULL, _extract_fstab, payload);

	xpc_release(payload);
	return item;
}

static si_list_t *
ds_fs_all(si_mod_t *si)
{
	return _ds_list(si, CATEGORY_FS, "getfsent", NULL, _extract_fstab);
}

static si_item_t *
ds_fs_byfile(si_mod_t *si, const char *name)
{
	si_item_t *item;
	si_list_t *list;
	uint32_t i;
	struct fstab *f;

	if (!_od_running()) return NULL;
	if (name == NULL) return NULL;

	list = ds_fs_all(si);
	if (list == NULL) return NULL;

	item = NULL;
	for (i = 0; (i < list->count) && (item == NULL); i++)
	{
		f = (struct fstab *)((uintptr_t)(list->entry[i]) + sizeof(si_item_t));
		if (string_equal(name, f->fs_file)) item = si_item_retain(list->entry[i]);
	}

	si_list_release(list);
	return item;
}

static si_item_t *
ds_mac_byname(si_mod_t *si, const char *name)
{
	xpc_object_t payload;
	si_item_t *item;

	if (!_od_running()) return NULL;

	payload = _xpc_query_key_string("name", name);
	if (payload == NULL) return NULL;

	item = _ds_item(si, CATEGORY_MAC, "getmacbyname", name, _extract_mac_mac, payload);

	xpc_release(payload);
	return item;
}

static si_item_t *
ds_mac_bymac(si_mod_t *si, const char *mac)
{
	xpc_object_t payload;
	si_item_t *item;
	char *cmac;

	if (!_od_running()) return NULL;

	cmac = si_standardize_mac_address(mac);
	if (cmac == NULL) return NULL;

	payload = xpc_dictionary_create(NULL, NULL, 0);
	if (payload == NULL) return NULL;

	payload = _xpc_query_key_string("mac", cmac);
	item = _ds_item(si, CATEGORY_MAC, "gethostbymac", cmac, _extract_mac_name, payload);

	free(cmac);
	xpc_release(payload);

	return item;
}

#pragma mark -

si_mod_t *
si_module_static_ds(void)
{
	static const struct si_mod_vtable_s ds_vtable =
	{
		.sim_is_valid = &_ds_is_valid,

		.sim_user_byname = &ds_user_byname,
		.sim_user_byuid = &ds_user_byuid,
		.sim_user_byuuid = &ds_user_byuuid,
		.sim_user_all = &ds_user_all,

		.sim_group_byname = &ds_group_byname,
		.sim_group_bygid = &ds_group_bygid,
		.sim_group_byuuid = &ds_group_byuuid,
		.sim_group_all = &ds_group_all,

		.sim_grouplist = &ds_grouplist,

		.sim_netgroup_byname = &ds_netgroup_byname,
		.sim_in_netgroup = &ds_in_netgroup,

		.sim_alias_byname = &ds_alias_byname,
		.sim_alias_all = &ds_alias_all,

		/* host lookups not supported */
		.sim_host_byname = NULL,
		.sim_host_byaddr = NULL,
		.sim_host_all = NULL,

		.sim_network_byname = &ds_network_byname,
		.sim_network_byaddr = &ds_network_byaddr,
		.sim_network_all = &ds_network_all,

		.sim_service_byname = &ds_service_byname,
		.sim_service_byport = &ds_service_byport,
		.sim_service_all = &ds_service_all,

		.sim_protocol_byname = &ds_protocol_byname,
		.sim_protocol_bynumber = &ds_protocol_bynumber,
		.sim_protocol_all = &ds_protocol_all,

		.sim_rpc_byname = &ds_rpc_byname,
		.sim_rpc_bynumber = &ds_rpc_bynumber,
		.sim_rpc_all = &ds_rpc_all,

		.sim_fs_byspec = &ds_fs_byspec,
		.sim_fs_byfile = &ds_fs_byfile,
		.sim_fs_all = &ds_fs_all,

		.sim_mac_byname = &ds_mac_byname,
		.sim_mac_bymac = &ds_mac_bymac,

		/* si_mac_all not supported */
		.sim_mac_all = NULL,

		/* si_addrinfo not supported */
		.sim_wants_addrinfo = NULL,
		.sim_addrinfo = NULL,
	};

	static si_mod_t si =
	{
		.vers = 1,
		.refcount = 1,
		.flags = SI_MOD_FLAG_STATIC,

		.private = NULL,
		.vtable = &ds_vtable,
	};

	static dispatch_once_t once;
	dispatch_once(&once, ^{
		pthread_key_create(&_ds_serv_cache_key, _ds_serv_cache_free);

		si.name = strdup("ds");
		ds_si_private_t *pp = calloc(1, sizeof(ds_si_private_t));

		if (pp != NULL)
		{
			pp->notify_token_global = -1;
			pp->notify_token_user = -1;
			pp->notify_token_group = -1;
			pp->notify_token_service = -1;
		}

		/*
		 * Don't register for notifications if the cache is disabled.
		 * notifyd (notably) disables the cache to prevent deadlocks.
		 */
		if (gL1CacheEnabled != 0)
		{
			/*
			 * Errors in registering for cache invalidation notifications are ignored.
			 * If there are failures, the tokens remain set to -1 which just causes
			 * cached items to be invalidated.
			 */
			notify_register_check(kNotifyDSCacheInvalidation, &(pp->notify_token_global));
			notify_register_check(kNotifyDSCacheInvalidationUser, &(pp->notify_token_user));
			notify_register_check(kNotifyDSCacheInvalidationGroup, &(pp->notify_token_group));
			notify_register_check(kNotifyDSCacheInvalidationService, &(pp->notify_token_service));
		}

		si.private = pp;
	});

	return &si;
}

#endif /* DS_AVAILABLE */

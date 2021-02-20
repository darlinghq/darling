/*
 * Copyright (c) 2003-2012 Apple Inc. All rights reserved.
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
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <signal.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <errno.h>
#include <os/lock_private.h>
#include <dispatch/dispatch.h>
#include <string.h>

#include "libnotify.h"
#include "notify.h"
#include "notify_internal.h"


#pragma mark -
const char *
_notify_shm_id(void)
{
#if TARGET_OS_SIMULATOR
    static dispatch_once_t once;
    static char *shm_id;
    
    dispatch_once(&once, ^{
        /*
         * According to documentation, our shm_id must be no more than 31 characters long
         * but in practice, even 31 characters is too long (<rdar://problem/16860882>),
         * so we jump through some hoops to make a smaller string based on our UDID.
         */
        const char *udid = getenv("SIMULATOR_UDID");
        if (udid && strlen(udid) == 36) {
            char scratch[34]; // 32 characters, 2 NUL
            
            /* 01234567890123456789012345678901234567890 */
            /* UUUUUUUU-UUUU-UUUU-LLLL-LLLLLLLLLLLL */
            memcpy(scratch, udid, 8);
            memcpy(scratch+8, udid+9, 4);
            memcpy(scratch+12, udid+14, 4);
            scratch[16] = '\0';
            
            memcpy(scratch+17, udid+19, 4);
            memcpy(scratch+21, udid+24, 12);
            scratch[33] = '\0';
            
            /*
             * If the input is invalid, these will end up being undefined
             * values, but they'll still be values we can use.
             */
            uint64_t upper = strtoull(scratch, NULL, 16);
            uint64_t lower = strtoull(scratch + 17, NULL, 16);
            
            const char *c64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
            scratch[0]  = c64[(upper >> 57) & 0xf];
            scratch[1]  = c64[(upper >> 50) & 0xf];
            scratch[2]  = c64[(upper >> 43) & 0xf];
            scratch[3]  = c64[(upper >> 36) & 0xf];
            scratch[4]  = c64[(upper >> 29) & 0xf];
            scratch[5]  = c64[(upper >> 22) & 0xf];
            scratch[6]  = c64[(upper >> 15) & 0xf];
            scratch[7]  = c64[(upper >>  8) & 0xf];
            scratch[8]  = c64[(upper >>  1) & 0xf];
            /* Drop a bit on the floor, but that probably doesn't matter.  It does not need to be reversible */
            
            scratch[10] = c64[(lower >> 57) & 0xf];
            scratch[11] = c64[(lower >> 50) & 0xf];
            scratch[12] = c64[(lower >> 43) & 0xf];
            scratch[13] = c64[(lower >> 36) & 0xf];
            scratch[14] = c64[(lower >> 29) & 0xf];
            scratch[15] = c64[(lower >> 22) & 0xf];
            scratch[16] = c64[(lower >> 15) & 0xf];
            scratch[17] = c64[(lower >>  8) & 0xf];
            scratch[18] = c64[(lower >>  1) & 0xf];
            /* Drop a bit on the floor, but that probably doesn't matter.  It does not need to be reversible */
            
            scratch[19] = '\0';
            
            asprintf(&shm_id, "sim.not.%s", scratch);
            assert(shm_id);
        }
        
        if (shm_id == NULL)
        {
            shm_id = "apple.shm.notification_center";
        }
    });
    
    return shm_id;
#else
    return "apple.shm.notification_center";
#endif
}

inline uint64_t
make_client_id(pid_t pid, int token)
{
	union client_id cid = {
		.pid = pid,
		.token = token,
	};
	return cid.hash_key;
}

void
_notify_lib_notify_state_init(notify_state_t * ns, uint32_t flags)
{
#ifdef SINGLE_THREADED_NOTIFY_STATE
	assert((flags & NOTIFY_STATE_USE_LOCKS) == 0);
#endif
	ns->flags = flags;
	ns->sock = -1;
	ns->lock = OS_UNFAIR_LOCK_INIT;
	_nc_table_init(&ns->name_table, offsetof(name_info_t, name));
	_nc_table_init_64(&ns->name_id_table, offsetof(name_info_t, name_id));
	_nc_table_init_64(&ns->client_table, offsetof(client_t, cid.hash_key));
	_nc_table_init_n(&ns->port_table, offsetof(port_data_t, port));
	_nc_table_init_n(&ns->proc_table, offsetof(proc_data_t, pid));
}

// We only need to lock in the client
inline static void
_notify_state_lock(os_unfair_lock_t lock)
{
#ifndef SINGLE_THREADED_NOTIFY_STATE
	os_unfair_lock_lock_with_options(lock, OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
#endif
}

inline static void
_notify_state_unlock(os_unfair_lock_t lock)
{
#ifndef SINGLE_THREADED_NOTIFY_STATE
	os_unfair_lock_unlock(lock);
#endif
}

static client_t *
_internal_client_new(notify_state_t *ns, pid_t pid, int token, name_info_t *n)
{
	client_t *c;
	uint64_t cid = make_client_id(pid, token);

	/* detect duplicates - should never happen, but it would be bad */
	c = _nc_table_find_64(&ns->client_table, cid);
	if (c != NULL) return NULL;

	c = calloc(1, sizeof(client_t));
	if (c == NULL) return NULL;

	ns->stat_client_alloc++;
	c->cid.hash_key = cid;
	c->name_info = n;

	LIST_INSERT_HEAD(&n->subscriptions, c, client_subscription_entry);
	n->refcount++;

	_nc_table_insert_64(&ns->client_table, &c->cid.hash_key);
	return c;
}

static void
_internal_client_release(notify_state_t *ns, client_t *c)
{
	_nc_table_delete_64(&ns->client_table, c->cid.hash_key);

	if (c->state_and_type & NOTIFY_TYPE_FILE) {
		if (c->deliver.fd >= 0) close(c->deliver.fd);
	} else if (c->state_and_type & NOTIFY_TYPE_PORT) {
		/* release my send right to the port */
		mach_port_deallocate(mach_task_self(), c->deliver.port);
	}

	free(c);
	ns->stat_client_free++;
}

static name_info_t *
_internal_new_name(notify_state_t *ns, const char *name)
{
	name_info_t *n;
	size_t namelen;

	if (name == NULL) return NULL;

	namelen = strlen(name) + 1;

	n = (name_info_t *)calloc(1, sizeof(name_info_t) + namelen);
	if (n == NULL) return NULL;

	ns->stat_name_alloc++;

	n->name = (char *)n + sizeof(name_info_t);
	memcpy(n->name, name, namelen);

	n->name_id = ns->name_id++;
	n->access = NOTIFY_ACCESS_DEFAULT;
	n->slot = (uint32_t)-1;
	n->val = 1;

	LIST_INIT(&n->subscriptions);

	_nc_table_insert(&ns->name_table, &n->name);
	_nc_table_insert_64(&ns->name_id_table, &n->name_id);

	return n;
}

static void
_internal_insert_controlled_name(notify_state_t *ns, name_info_t *n)
{
	int i, j;

	if (n == NULL) return;

	if (ns->controlled_name == NULL) ns->controlled_name_count = 0;

	for (i = 0; i < ns->controlled_name_count; i++)
	{
		if (ns->controlled_name[i] == n) return;
	}

	ns->controlled_name = (name_info_t **)reallocf(ns->controlled_name, (ns->controlled_name_count + 1) * sizeof(name_info_t *));

	/*
	 * Insert name in reverse sorted order (longer names preceed shorter names).
	 * this means that in _internal_check_access, we check subspaces from the bottom up
	 * i.e. we check access for the "deepest" controlled subspace.
	 */

	for (i = 0; i < ns->controlled_name_count; i++)
	{
		if (strcmp(n->name, ns->controlled_name[i]->name) > 0) break;
	}

	for (j = ns->controlled_name_count; j > i; j--)
	{
		ns->controlled_name[j] = ns->controlled_name[j-1];
	}

	ns->controlled_name[i] = n;
	ns->controlled_name_count++;
}

static void
_internal_remove_controlled_name(notify_state_t *ns, name_info_t *n)
{
	uint32_t i, j;

	for (i = 0; i < ns->controlled_name_count; i++)
	{
		if (ns->controlled_name[i] == n)
		{
			for (j = i + 1; j < ns->controlled_name_count; j++)
			{
				ns->controlled_name[j-1] = ns->controlled_name[j];
			}

			ns->controlled_name_count--;
			if (ns->controlled_name_count == 0)
			{
				free(ns->controlled_name);
				ns->controlled_name = NULL;
			}
			else
			{
				ns->controlled_name = (name_info_t **)reallocf(ns->controlled_name, ns->controlled_name_count * sizeof(name_info_t *));
			}

			return;
		}
	}
}

static uint32_t
_internal_check_access(notify_state_t *ns, const char *name, uid_t uid, gid_t gid, int req)
{
    uint32_t i;
    size_t len, plen;
	name_info_t *p;
	char str[64];

	if (name == NULL) return NOTIFY_STATUS_NULL_INPUT;

	/* root may do anything */
	if (uid == 0) return NOTIFY_STATUS_OK;

	/* if name has "user.uid." as a prefix, it is a user-protected namespace */
	if (!strncmp(name, USER_PROTECTED_UID_PREFIX, USER_PROTECTED_UID_PREFIX_LEN))
    {
        snprintf(str, sizeof(str) - 1, "%s%d", USER_PROTECTED_UID_PREFIX, uid);
        len = strlen(str);

        /* user <uid> may access user.uid.<uid> or a subtree name */
        if ((!strncmp(name, str, len)) && ((name[len] == '\0') || (name[len] == '.'))) return NOTIFY_STATUS_OK;
        return NOTIFY_STATUS_NOT_AUTHORIZED;
    }

    len = strlen(name);

	if (ns->controlled_name == NULL) ns->controlled_name_count = 0;
	for (i = 0; i < ns->controlled_name_count; i++)
	{
		p = ns->controlled_name[i];
		if (p == NULL) break;
		if (p->name == NULL) continue;

		plen = strlen(p->name);
		if (plen > len) continue;
		if (strncmp(p->name, name, plen)) continue;

		/* Found a match or a prefix, check if restrictions apply to this uid/gid */
		if ((p->uid == uid) && (p->access & (req << NOTIFY_ACCESS_USER_SHIFT))) break;
		if ((p->gid == gid) && (p->access & (req << NOTIFY_ACCESS_GROUP_SHIFT))) break;
		if (p->access & (req << NOTIFY_ACCESS_OTHER_SHIFT)) break;

		return NOTIFY_STATUS_NOT_AUTHORIZED;
	}

	return NOTIFY_STATUS_OK;
}

uint32_t
_notify_lib_check_controlled_access(notify_state_t *ns, char *name, uid_t uid, gid_t gid, int req)
{
	uint32_t status;

	_notify_state_lock(&ns->lock);
	status = _internal_check_access(ns, name, uid, gid, req);
	_notify_state_unlock(&ns->lock);

	return status;
}

/*
 * Send notification to a subscriber
 */
static uint32_t
_internal_send(notify_state_t *ns, client_t *c,
		proc_data_t *proc_data, port_data_t *port_data)
{
	uint32_t send;

	if (c->state_and_type & NOTIFY_CLIENT_STATE_SUSPENDED)
	{
		c->state_and_type |= NOTIFY_CLIENT_STATE_PENDING;
		return NOTIFY_STATUS_OK;
	}

	if (proc_data == NULL)
	{
		proc_data = _nc_table_find_n(&ns->proc_table, c->cid.pid);
	}
	if ((proc_data != NULL) && (proc_data->flags & NOTIFY_PORT_PROC_STATE_SUSPENDED))
	{
		c->suspend_count++;
		c->state_and_type |= NOTIFY_CLIENT_STATE_SUSPENDED;
		c->state_and_type |= NOTIFY_CLIENT_STATE_PENDING;
		return NOTIFY_STATUS_OK;
	}

	send = c->cid.token;

	switch (c->state_and_type & NOTIFY_TYPE_MASK)
	{
		case NOTIFY_TYPE_SIGNAL:
		{
			int rc = 0;

			if (c->cid.pid == NOTIFY_CLIENT_SELF) rc = kill(getpid(), c->deliver.sig);
			else rc = kill(c->cid.pid, c->deliver.sig);

			if (rc != 0) return NOTIFY_STATUS_KILL_FAILED;

			c->state_and_type &= ~NOTIFY_CLIENT_STATE_PENDING;
			c->state_and_type &= ~NOTIFY_CLIENT_STATE_TIMEOUT;

			return NOTIFY_STATUS_OK;
		}

		case NOTIFY_TYPE_FILE:
		{
			ssize_t len;

			if (c->deliver.fd >= 0)
			{
				send = htonl(send);
				len = write(c->deliver.fd, &send, sizeof(uint32_t));
				if (len != sizeof(uint32_t))
				{
					close(c->deliver.fd);
					c->deliver.fd = -1;
					return NOTIFY_STATUS_WRITE_FAILED;
				}
			}

			c->state_and_type &= ~NOTIFY_CLIENT_STATE_PENDING;
			c->state_and_type &= ~NOTIFY_CLIENT_STATE_TIMEOUT;

			return NOTIFY_STATUS_OK;
		}

		case NOTIFY_TYPE_PORT:
		{
			kern_return_t kstatus;
			mach_msg_empty_send_t msg;
			mach_msg_option_t opts = MACH_SEND_MSG | MACH_SEND_TIMEOUT;

			if (port_data == NULL)
			{
				port_data = _nc_table_find_n(&ns->port_table, c->deliver.port);
			}
			if ((port_data != NULL) && (port_data->flags & NOTIFY_PORT_PROC_STATE_SUSPENDED))
			{
				c->suspend_count++;
				c->state_and_type |= NOTIFY_CLIENT_STATE_SUSPENDED;
				c->state_and_type |= NOTIFY_CLIENT_STATE_PENDING;
				return NOTIFY_STATUS_OK;
			}

			if (ns->flags & NOTIFY_STATE_ENABLE_RESEND) opts |= MACH_SEND_NOTIFY;

			memset(&msg, 0, sizeof(mach_msg_empty_send_t));
			msg.header.msgh_size = sizeof(mach_msg_empty_send_t);
			msg.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSGH_BITS_ZERO);
			msg.header.msgh_local_port = MACH_PORT_NULL;
			msg.header.msgh_remote_port = c->deliver.port;
			msg.header.msgh_id = (mach_msg_id_t)send;

			kstatus = mach_msg(&msg.header, opts, msg.header.msgh_size, 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);

			if (kstatus == MACH_SEND_TIMED_OUT)
			{
				/* deallocate port rights obtained via pseudo-receive after failed mach_msg() send */
				mach_msg_destroy(&msg.header);
				if (ns->flags & NOTIFY_STATE_ENABLE_RESEND)
				{
					/*
					 * Suspend on timeout.
					 * notifyd will get a MACH_NOTIFY_SEND_POSSIBLE and trigger a retry.
					 * c->suspend_count must be zero, or we would not be trying to send.
					 */
					c->suspend_count++;
					c->state_and_type |= NOTIFY_CLIENT_STATE_SUSPENDED;
					c->state_and_type |= NOTIFY_CLIENT_STATE_PENDING;
					c->state_and_type |= NOTIFY_CLIENT_STATE_TIMEOUT;

					if (port_data) {
						/*
						 * If we failed to send, stop trying on this port
						 * and just wait for the send-possible notification
						 */
						port_data->flags |= NOTIFY_PORT_PROC_STATE_SUSPENDED;
					}
					return NOTIFY_STATUS_OK;
				}

				return NOTIFY_STATUS_MACH_MSG_TIMEOUT;
			}
			else if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_MACH_MSG_FAILED;

			c->state_and_type &= ~NOTIFY_CLIENT_STATE_PENDING;
			c->state_and_type &= ~NOTIFY_CLIENT_STATE_TIMEOUT;

			return NOTIFY_STATUS_OK;
		}

		default:
		{
			break;
		}
	}

	c->state_and_type &= ~NOTIFY_CLIENT_STATE_PENDING;
	c->state_and_type &= ~NOTIFY_CLIENT_STATE_TIMEOUT;

	return NOTIFY_STATUS_OK;
}

uint32_t
_notify_lib_post_client(notify_state_t *ns, client_t *c)
{
	uint32_t status;

	if (c == NULL) return NOTIFY_STATUS_NULL_INPUT;

	_notify_state_lock(&ns->lock);
	status = _internal_send(ns, c, NULL, NULL);
	_notify_state_unlock(&ns->lock);

	return status;
}

static uint32_t
_internal_post_name(notify_state_t *ns, name_info_t *n, uid_t uid, gid_t gid)
{
	int auth;
	client_t *c;

	if (n == NULL) return NOTIFY_STATUS_INVALID_NAME;

	auth = _internal_check_access(ns, n->name, uid, gid, NOTIFY_ACCESS_WRITE);
	if (auth != 0) return NOTIFY_STATUS_NOT_AUTHORIZED;

	n->val++;

	LIST_FOREACH(c, &n->subscriptions, client_subscription_entry) {
		_internal_send(ns, c, NULL, NULL);
	}

	return NOTIFY_STATUS_OK;
}

/*
 * Notify subscribers of this name.
 */
uint32_t
_notify_lib_post(notify_state_t *ns, const char *name, uid_t uid, gid_t gid)
{
	name_info_t *n;
	uint32_t status;

	_notify_state_lock(&ns->lock);

	n = _nc_table_find(&ns->name_table, name);
	if (n == NULL)
	{
		_notify_state_unlock(&ns->lock);
		return NOTIFY_STATUS_INVALID_NAME;
	}

	status = _internal_post_name(ns, n, uid, gid);

	_notify_state_unlock(&ns->lock);
	return status;
}

uint32_t
_notify_lib_post_nid(notify_state_t *ns, uint64_t nid, uid_t uid, gid_t gid)
{
	name_info_t *n;
	uint32_t status;

	_notify_state_lock(&ns->lock);

	n = _nc_table_find_64(&ns->name_id_table, nid);
	if (n == NULL)
	{
		_notify_state_unlock(&ns->lock);
		return NOTIFY_STATUS_INVALID_NAME;
	}

	status = _internal_post_name(ns, n, uid, gid);

	_notify_state_unlock(&ns->lock);
	return status;
}

static void
_internal_release_name_info(notify_state_t *ns, name_info_t *n)
{
	if (n == NULL) return;

	if (n->refcount > 0) n->refcount--;
	if (n->refcount == 0)
	{
		_internal_remove_controlled_name(ns, n);
		_nc_table_delete(&ns->name_table, n->name);
		_nc_table_delete_64(&ns->name_id_table, n->name_id);
		free(n);
		ns->stat_name_free++;
	}
}

/*
 * Cancel (delete) a client
 */
static void
_internal_cancel(notify_state_t *ns, client_t *c)
{
	name_info_t *n = c->name_info;

	LIST_REMOVE(c, client_subscription_entry);
	_internal_client_release(ns, c);
	_internal_release_name_info(ns, n);
}

void
_notify_lib_cancel_client(notify_state_t *ns, client_t *c)
{
	_notify_state_lock(&ns->lock);
	_internal_cancel(ns, c);
	_notify_state_unlock(&ns->lock);
}

void
_notify_lib_cancel(notify_state_t *ns, pid_t pid, int token)
{
	uint64_t cid = make_client_id(pid, token);
	client_t *c;

	_notify_state_lock(&ns->lock);
	c = _nc_table_find_64(&ns->client_table, cid);
	if (c) {
		_internal_cancel(ns, c);
	}
	_notify_state_unlock(&ns->lock);
}

uint32_t
_notify_lib_suspend(notify_state_t *ns, pid_t pid, int token)
{
	uint64_t cid = make_client_id(pid, token);
	uint32_t result = NOTIFY_STATUS_OK;
	client_t *c;

	_notify_state_lock(&ns->lock);

	c = _nc_table_find_64(&ns->client_table, cid);
	if (c != NULL) {
		c->state_and_type |= NOTIFY_CLIENT_STATE_SUSPENDED;
		if (c->suspend_count < UINT8_MAX) c->suspend_count++;
	} else {
		result = NOTIFY_STATUS_CLIENT_NOT_FOUND;
	}

	_notify_state_unlock(&ns->lock);

	return result;
}

static void
_internal_resume(notify_state_t *ns, client_t *c,
		proc_data_t *proc_data, port_data_t *port_data)
{
	if (c->suspend_count > 0) c->suspend_count--;
	if (c->suspend_count == 0) {
		c->state_and_type &= ~NOTIFY_CLIENT_STATE_SUSPENDED;
		c->state_and_type &= ~NOTIFY_CLIENT_STATE_TIMEOUT;

		if (c->state_and_type & NOTIFY_CLIENT_STATE_PENDING) {
			_internal_send(ns, c, proc_data, port_data);
		}
	}
}

void
_notify_lib_resume_client(notify_state_t *ns, client_t *c,
		proc_data_t *proc_data, port_data_t *port_data)
{
	_notify_state_lock(&ns->lock);
	_internal_resume(ns, c, proc_data, port_data);
	_notify_state_unlock(&ns->lock);
}

uint32_t
_notify_lib_resume(notify_state_t *ns, pid_t pid, int token)
{
	uint32_t status = NOTIFY_STATUS_OK;
	uint64_t cid = make_client_id(pid, token);
	client_t *c;

	_notify_state_lock(&ns->lock);

	c = _nc_table_find_64(&ns->client_table, cid);
	if (c != NULL) {
		_internal_resume(ns, c, NULL, NULL);
	} else {
		status = NOTIFY_STATUS_CLIENT_NOT_FOUND;
	}

	_notify_state_unlock(&ns->lock);

	return status;
}

/*
 * Check if a name has changed since the last time this client checked.
 * Returns true, false, or error.
 */
uint32_t
_notify_lib_check(notify_state_t *ns, pid_t pid, int token, int *check)
{
	client_t *c;
	uint64_t cid;

	if (check == NULL) return NOTIFY_STATUS_NULL_INPUT;

	cid = make_client_id(pid, token);

	_notify_state_lock(&ns->lock);

	c = _nc_table_find_64(&ns->client_table, cid);

	if (c == NULL)
	{
		_notify_state_unlock(&ns->lock);
		return NOTIFY_STATUS_INVALID_TOKEN;
	}

	if (c->name_info->val == c->lastval)
	{
		*check = 0;
		_notify_state_unlock(&ns->lock);
		return NOTIFY_STATUS_OK;
	}

	c->lastval = c->name_info->val;
	*check = 1;

	_notify_state_unlock(&ns->lock);
	return NOTIFY_STATUS_OK;
}

/*
 * SPI: get value for a name.
 */
__private_extern__ uint32_t
_notify_lib_peek(notify_state_t *ns, pid_t pid, int token, int *val)
{
    client_t *c;
    uint64_t cid;

    if (val == NULL) return NOTIFY_STATUS_NULL_INPUT;
    
    cid = make_client_id(pid, token);
    
    _notify_state_lock(&ns->lock);
    
    c = _nc_table_find_64(&ns->client_table, cid);
    
    if (c == NULL)
    {
        _notify_state_unlock(&ns->lock);
        return NOTIFY_STATUS_INVALID_TOKEN;
    }
    
    *val = c->name_info->val;
    
    _notify_state_unlock(&ns->lock);
    return NOTIFY_STATUS_OK;
}

/*
 * Get state value for a name.
 */
uint32_t
_notify_lib_get_state(notify_state_t *ns, uint64_t nid, uint64_t *state, uid_t uid, gid_t gid)
{
	name_info_t *n;

	if (state == NULL) return NOTIFY_STATUS_NULL_INPUT;

	*state = 0;

	_notify_state_lock(&ns->lock);

	n = _nc_table_find_64(&ns->name_id_table, nid);

	if (n == NULL)
	{
		_notify_state_unlock(&ns->lock);
		return NOTIFY_STATUS_INVALID_NAME;
	}

#ifdef GET_STATE_AUTH_CHECK
	int auth = _internal_check_access(ns, n->name, uid, gid, NOTIFY_ACCESS_READ);
	if (auth != 0)
	{
		_notify_state_unlock(&ns->lock);
		return NOTIFY_STATUS_NOT_AUTHORIZED;
	}
#endif

	*state = n->state;

	_notify_state_unlock(&ns->lock);
	return NOTIFY_STATUS_OK;
}

/*
 * Set state value for a name.
 */
uint32_t
_notify_lib_set_state(notify_state_t *ns, uint64_t nid, uint64_t state, uid_t uid, gid_t gid)
{
	name_info_t *n;
	int auth;

	_notify_state_lock(&ns->lock);

	n = _nc_table_find_64(&ns->name_id_table, nid);

	if (n == NULL)
	{
		_notify_state_unlock(&ns->lock);
		return NOTIFY_STATUS_INVALID_NAME;
	}

	auth = _internal_check_access(ns, n->name, uid, gid, NOTIFY_ACCESS_WRITE);
	if (auth != 0)
	{
		_notify_state_unlock(&ns->lock);
		return NOTIFY_STATUS_NOT_AUTHORIZED;
	}

	n->state = state;
	n->state_time = mach_absolute_time();

	_notify_state_unlock(&ns->lock);
	return NOTIFY_STATUS_OK;
}

static uint32_t
_internal_register_common(notify_state_t *ns, const char *name, pid_t pid, int token, uid_t uid, gid_t gid, client_t **outc)
{
	client_t *c;
	name_info_t *n;
	int is_new_name;
	uint32_t status;

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;
	if (outc == NULL) return NOTIFY_STATUS_OK;

	status = _internal_check_access(ns, name, uid, gid, NOTIFY_ACCESS_READ);
	if (status != NOTIFY_STATUS_OK) return NOTIFY_STATUS_NOT_AUTHORIZED;

	*outc = NULL;
	is_new_name = 0;

	n = _nc_table_find(&ns->name_table, name);
	if (n == NULL)
	{
		is_new_name = 1;

		n = _internal_new_name(ns, name);
		if (n == NULL) return NOTIFY_STATUS_NEW_NAME_FAILED;
	}

	c = _internal_client_new(ns, pid, token, n);
	if (c == NULL)
	{
		if (is_new_name == 1)
		{
			_nc_table_delete(&ns->name_table, n->name);
			free(n);
			ns->stat_name_free++;
		}

		return NOTIFY_STATUS_NEW_CLIENT_FAILED;
	}

	*outc = c;

	return NOTIFY_STATUS_OK;
}

/*
 * Register for signal.
 * Returns the client_id;
 */
uint32_t
_notify_lib_register_signal(notify_state_t *ns, const char *name, pid_t pid, int token, uint32_t sig, uid_t uid, gid_t gid, uint64_t *out_nid)
{
	client_t *c;
	uint32_t status;

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	c = NULL;

	_notify_state_lock(&ns->lock);

	status = _internal_register_common(ns, name, pid, token, uid, gid, &c);
	if (status != NOTIFY_STATUS_OK)
	{
		_notify_state_unlock(&ns->lock);
		return status;
	}

	c->state_and_type &= ~NOTIFY_TYPE_MASK;
	c->state_and_type |= NOTIFY_TYPE_SIGNAL;
	c->cid.pid = pid;
	c->deliver.sig = sig;
	*out_nid = c->name_info->name_id;

	_notify_state_unlock(&ns->lock);
	return NOTIFY_STATUS_OK;
}

/*
 * Register for notification on a file descriptor.
 * Returns the client_id;
 */
uint32_t
_notify_lib_register_file_descriptor(notify_state_t *ns, const char *name, pid_t pid, int token, int fd, uid_t uid, gid_t gid, uint64_t *out_nid)
{
	client_t *c;
	uint32_t status;

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	c = NULL;

	_notify_state_lock(&ns->lock);

	status = _internal_register_common(ns, name, pid, token, uid, gid, &c);
	if (status != NOTIFY_STATUS_OK)
	{
		_notify_state_unlock(&ns->lock);
		return status;
	}

	c->state_and_type &= ~NOTIFY_TYPE_MASK;
	c->state_and_type |= NOTIFY_TYPE_FILE;

	c->deliver.fd = fd;
	*out_nid = c->name_info->name_id;

	_notify_state_unlock(&ns->lock);
	return NOTIFY_STATUS_OK;
}

/*
 * Register for notification on a mach port.
 * Returns the client_id;
 */
uint32_t
_notify_lib_register_mach_port(notify_state_t *ns, const char *name, pid_t pid, int token, mach_port_t port, uid_t uid, gid_t gid, uint64_t *out_nid)
{
	client_t *c;
	uint32_t status;

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;
	if (!MACH_PORT_VALID(port)) return NOTIFY_STATUS_INVALID_PORT;

	c = NULL;

	_notify_state_lock(&ns->lock);

	status = _internal_register_common(ns, name, pid, token, uid, gid, &c);
	if (status != NOTIFY_STATUS_OK)
	{
		_notify_state_unlock(&ns->lock);
		return status;
	}

	c->state_and_type &= ~NOTIFY_TYPE_MASK;
	c->state_and_type |= NOTIFY_TYPE_PORT;
	c->deliver.port = port;
	*out_nid = c->name_info->name_id;

	_notify_state_unlock(&ns->lock);
	return NOTIFY_STATUS_OK;
}

/*
 * Plain registration - only for notify_check()
 * Returns the client_id.
 */
uint32_t
_notify_lib_register_plain(notify_state_t *ns, const char *name, pid_t pid, int token, uint32_t slot, uint32_t uid, uint32_t gid, uint64_t *out_nid)
{
	client_t *c;
	uint32_t status;

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	c = NULL;

	_notify_state_lock(&ns->lock);

	status = _internal_register_common(ns, name, pid, token, uid, gid, &c);
	if (status != NOTIFY_STATUS_OK)
	{
		_notify_state_unlock(&ns->lock);
		return status;
	}

	if (slot == SLOT_NONE)
	{
		c->state_and_type &= ~NOTIFY_TYPE_MASK;
		c->state_and_type |= NOTIFY_TYPE_PLAIN;
	}
	else
	{
		c->state_and_type &= ~NOTIFY_TYPE_MASK;
		c->state_and_type |= NOTIFY_TYPE_MEMORY;
		c->name_info->slot = slot;
	}

	*out_nid = c->name_info->name_id;

	_notify_state_unlock(&ns->lock);
	return NOTIFY_STATUS_OK;
}

uint32_t
_notify_lib_set_owner(notify_state_t *ns, const char *name, uid_t uid, gid_t gid)
{
	name_info_t *n;

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	_notify_state_lock(&ns->lock);

	n = _nc_table_find(&ns->name_table, name);
	if (n == NULL)
	{
		/* create new name */
		n = _internal_new_name(ns, name);
		if (n == NULL)
		{
			_notify_state_unlock(&ns->lock);
			return NOTIFY_STATUS_NEW_NAME_FAILED;
		}

		/* 
		 * Setting the refcount here allows the namespace to be "pre-populated"
		 * with controlled names.  notifyd does this for reserved names in 
		 * its configuration file.
		 */
		n->refcount++;
	}

	n->uid = uid;
	n->gid = gid;

	_internal_insert_controlled_name(ns, n);

	_notify_state_unlock(&ns->lock);
	return NOTIFY_STATUS_OK;
}

uint32_t
_notify_lib_set_access(notify_state_t *ns, const char *name, uint32_t mode)
{
	name_info_t *n;

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	_notify_state_lock(&ns->lock);

	n = _nc_table_find(&ns->name_table, name);
	if (n == NULL)
	{
		/* create new name */
		n = _internal_new_name(ns, name);
		if (n == NULL)
		{
			_notify_state_unlock(&ns->lock);
			return NOTIFY_STATUS_NEW_NAME_FAILED;
		}

		/* 
		 * Setting the refcount here allows the namespace to be "pre-populated"
		 * with controlled names.  notifyd does this for reserved names in 
		 * its configuration file.
		 */
		n->refcount++;
	}

	n->access = mode;

	_internal_insert_controlled_name(ns, n);

	_notify_state_unlock(&ns->lock);
	return NOTIFY_STATUS_OK;
}

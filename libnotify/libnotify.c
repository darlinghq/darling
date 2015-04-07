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
#include <pthread.h>

#include "libnotify.h"
#include "notify.h"
#include "notify_internal.h"

#define USER_PROTECTED_UID_PREFIX "user.uid."
#define USER_PROTECTED_UID_PREFIX_LEN 9

uint64_t
make_client_id(pid_t pid, int token)
{
	uint64_t cid;

	cid = pid;
	cid <<= 32;
	cid |= token;

	return cid;
}

notify_state_t *
_notify_lib_notify_state_new(uint32_t flags, uint32_t table_size)
{
	notify_state_t *ns;

	ns = (notify_state_t *)calloc(1, sizeof(notify_state_t));
	if (ns == NULL) return NULL;

	ns->flags = flags;
	ns->sock = -1;

	if (ns->flags & NOTIFY_STATE_USE_LOCKS) 
	{
		ns->lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
		if (ns->lock == NULL)
		{
			free(ns);
			return NULL;
		}

		pthread_mutex_init(ns->lock, NULL);
	}

	ns->name_table = _nc_table_new(table_size);
	ns->name_id_table = _nc_table_new(table_size);
	ns->client_table = _nc_table_new(table_size);
	ns->port_table = _nc_table_new(table_size);
	ns->proc_table = _nc_table_new(table_size);

	if ((ns->name_table == NULL) || (ns->name_id_table == NULL) || (ns->client_table == NULL) || (ns->port_table == NULL) || (ns->proc_table == NULL))
	{
		free(ns->lock);
		_nc_table_free(ns->name_table);
		_nc_table_free(ns->name_id_table);
		_nc_table_free(ns->client_table);
		_nc_table_free(ns->port_table);
		_nc_table_free(ns->proc_table);
		free(ns);
		return NULL;
	}

	return ns;
}

void
_notify_lib_notify_state_free(notify_state_t *ns)
{
	if (ns == NULL) return;

	_nc_table_free(ns->name_table);
	_nc_table_free(ns->name_id_table);
	_nc_table_free(ns->client_table);
	_nc_table_free(ns->port_table);
	_nc_table_free(ns->proc_table);

	if (ns->lock != NULL)
	{
		pthread_mutex_destroy(ns->lock);
		free(ns->lock);
	}

	if (ns->sock != -1)
	{
		shutdown(ns->sock, 2);
		close(ns->sock);
	}

	if (ns->controlled_name != NULL) free(ns->controlled_name);
}

static client_t *
_internal_client_new(notify_state_t *ns, pid_t pid, int token)
{
	client_t *c;
	uint64_t cid = make_client_id(pid, token);

	if (ns == NULL) return NULL;

	/* detect duplicates - should never happen, but it would be bad */
	c = _nc_table_find_64(ns->client_table, cid);
	if (c != NULL) return NULL;

	c = calloc(1, sizeof(client_t));
	if (c == NULL) return NULL;

	ns->stat_client_alloc++;

	c->client_id = cid;
	c->pid = pid;
	c->send_val = token;

	_nc_table_insert_64(ns->client_table, cid, c);

	return c;
}

static void
_internal_client_release(notify_state_t *ns, client_t *c)
{
	uint64_t cid;

	if (ns == NULL) return;
	if (c == NULL) return;

	cid = c->client_id;
	_nc_table_delete_64(ns->client_table, cid);

	switch (c->notify_type)
	{
		case NOTIFY_TYPE_SIGNAL:
		{
			break;
		}
		case NOTIFY_TYPE_FILE:
		{
			if (c->fd > 0) close(c->fd);
			c->fd = -1;
			break;
		}
		case NOTIFY_TYPE_PORT:
		{
			if (c->port != MACH_PORT_NULL)
			{
				/* release my send right to the port */
				mach_port_deallocate(mach_task_self(), c->port);
			}
			break;
		}
		default:
		{
			break;
		}
	}

	free(c);
	ns->stat_client_free++;
}

static name_info_t *
_internal_new_name(notify_state_t *ns, const char *name)
{
	name_info_t *n;
	size_t namelen;

	if (ns == NULL) return NULL;
	if (name == NULL) return NULL;

	namelen = strlen(name) + 1;

	n = (name_info_t *)calloc(1, sizeof(name_info_t) + namelen);
	if (n == NULL) return NULL;

	ns->stat_name_alloc++;

	n->name = (char *)n + sizeof(name_info_t);
	memcpy(n->name, name, namelen);

	notify_globals_t globals = _notify_globals();
	n->name_id = globals->name_id++;

	n->access = NOTIFY_ACCESS_DEFAULT;
	n->slot = (uint32_t)-1;
	n->val = 1;

	_nc_table_insert_no_copy(ns->name_table, n->name, n);
	_nc_table_insert_64(ns->name_id_table, n->name_id, n);

	return n;
}

static void
_internal_insert_controlled_name(notify_state_t *ns, name_info_t *n)
{
	int i, j;

	if (ns == NULL) return;
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
	uint32_t i, len, plen;
	name_info_t *p;
	char str[64];

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

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

	if (ns == NULL) return NOTIFY_STATUS_FAILED;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);
	status = _internal_check_access(ns, name, uid, gid, req);
	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);

	return status;
}

uint32_t
_notify_lib_port_proc_new(notify_state_t *ns, mach_port_t port, pid_t proc, uint32_t state, dispatch_source_t src)
{
	portproc_data_t *pdata;

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if ((proc == 0) && (port == MACH_PORT_NULL)) return NOTIFY_STATUS_FAILED;

	pdata = (portproc_data_t *)calloc(1, sizeof(portproc_data_t));
	if (pdata == NULL) return NOTIFY_STATUS_FAILED;

	ns->stat_portproc_alloc++;

	pdata->refcount = 1;
	pdata->flags = state;
	pdata->src = src;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);
	if (proc == 0) _nc_table_insert_n(ns->port_table, port, pdata);
	else _nc_table_insert_n(ns->proc_table, proc, pdata);
	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);

	return NOTIFY_STATUS_OK;
}

portproc_data_t *
_notify_lib_port_proc_find(notify_state_t *ns, mach_port_t port, pid_t proc)
{
	portproc_data_t *pdata = NULL;

	if (ns == NULL) return NULL;
	if ((proc == 0) && (port == MACH_PORT_NULL)) return NULL;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	if (proc == 0) pdata = _nc_table_find_n(ns->port_table, port);
	else pdata = _nc_table_find_n(ns->proc_table, proc);

	if (pdata != NULL) pdata->refcount++;

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);

	return pdata;
}

void
_notify_lib_port_proc_release(notify_state_t *ns, mach_port_t port, pid_t proc)
{
	portproc_data_t *pdata = NULL;

	if (ns == NULL) return;
	if ((proc == 0) && (port == MACH_PORT_NULL)) return;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	if (proc == 0) pdata = _nc_table_find_n(ns->port_table, port);
	else pdata = _nc_table_find_n(ns->proc_table, proc);

	if (pdata != NULL)
	{
		if (pdata->refcount > 0) pdata->refcount--;
		if (pdata->refcount == 0)
		{
			if (proc == 0) _nc_table_delete_n(ns->port_table, port);
			else _nc_table_delete_n(ns->proc_table, proc);

			dispatch_source_cancel(pdata->src);
			dispatch_release(pdata->src);

			free(pdata);
			ns->stat_portproc_free++;
		}
	}

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
}

/*
 * Send notification to a subscriber
 */
static uint32_t
_internal_send(notify_state_t *ns, client_t *c)
{
	uint32_t send;
	portproc_data_t *pdata;

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (c == NULL) return NOTIFY_STATUS_FAILED;

	if (c->state & NOTIFY_CLIENT_STATE_SUSPENDED)
	{
		c->state |= NOTIFY_CLIENT_STATE_PENDING;
		return NOTIFY_STATUS_OK;
	}

	pdata = _nc_table_find_n(ns->proc_table, c->pid);
	if ((pdata != NULL) && (pdata->flags & NOTIFY_PORT_PROC_STATE_SUSPENDED))
	{
		c->suspend_count++;
		c->state |= NOTIFY_CLIENT_STATE_SUSPENDED;
		c->state |= NOTIFY_CLIENT_STATE_PENDING;
		return NOTIFY_STATUS_OK;
	}

	send = c->send_val;

	switch (c->notify_type)
	{
		case NOTIFY_TYPE_SIGNAL:
		{
			int rc = 0;

			if (c->pid == NOTIFY_CLIENT_SELF) rc = kill(getpid(), c->sig);
			else rc = kill(c->pid, c->sig);

			if (rc != 0) return NOTIFY_STATUS_FAILED;

			c->state &= ~NOTIFY_CLIENT_STATE_PENDING;
			c->state &= ~NOTIFY_CLIENT_STATE_TIMEOUT;

			return NOTIFY_STATUS_OK;
		}

		case NOTIFY_TYPE_FILE:
		{
			ssize_t len;

			if (c->fd >= 0)
			{
				send = htonl(send);
				len = write(c->fd, &send, sizeof(uint32_t));
				if (len != sizeof(uint32_t))
				{
					close(c->fd);
					c->fd = -1;
					return NOTIFY_STATUS_FAILED;
				}
			}

			c->state &= ~NOTIFY_CLIENT_STATE_PENDING;
			c->state &= ~NOTIFY_CLIENT_STATE_TIMEOUT;

			return NOTIFY_STATUS_OK;
		}

		case NOTIFY_TYPE_PORT:
		{
			kern_return_t kstatus;
			mach_msg_empty_send_t msg;
			mach_msg_option_t opts = MACH_SEND_MSG | MACH_SEND_TIMEOUT;

			pdata = _nc_table_find_n(ns->port_table, c->port);
			if ((pdata != NULL) && (pdata->flags & NOTIFY_PORT_PROC_STATE_SUSPENDED))
			{
				c->suspend_count++;
				c->state |= NOTIFY_CLIENT_STATE_SUSPENDED;
				c->state |= NOTIFY_CLIENT_STATE_PENDING;
				return NOTIFY_STATUS_OK;
			}

			if (ns->flags & NOTIFY_STATE_ENABLE_RESEND) opts |= MACH_SEND_NOTIFY;

			memset(&msg, 0, sizeof(mach_msg_empty_send_t));
			msg.header.msgh_size = sizeof(mach_msg_empty_send_t);
			msg.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSGH_BITS_ZERO);
			msg.header.msgh_local_port = MACH_PORT_NULL;
			msg.header.msgh_remote_port = c->port;
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
					c->state |= NOTIFY_CLIENT_STATE_SUSPENDED;
					c->state |= NOTIFY_CLIENT_STATE_PENDING;
					c->state |= NOTIFY_CLIENT_STATE_TIMEOUT;

					return NOTIFY_STATUS_OK;
				}

				return NOTIFY_STATUS_FAILED;
			}
			else if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;

			c->state &= ~NOTIFY_CLIENT_STATE_PENDING;
			c->state &= ~NOTIFY_CLIENT_STATE_TIMEOUT;

			return NOTIFY_STATUS_OK;
		}

		default:
		{
			break;
		}
	}

	c->state &= ~NOTIFY_CLIENT_STATE_PENDING;
	c->state &= ~NOTIFY_CLIENT_STATE_TIMEOUT;

	return NOTIFY_STATUS_OK;
}

uint32_t
_notify_lib_post_client(notify_state_t *ns, client_t *c)
{
	uint32_t status;

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (c == NULL) return NOTIFY_STATUS_FAILED;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);
	status = _internal_send(ns, c);
	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);

	return status;
}

static uint32_t
_internal_post_name(notify_state_t *ns, name_info_t *n, uid_t uid, gid_t gid)
{
	int auth;
	list_t *l;
	client_t *c;

	if (n == NULL) return NOTIFY_STATUS_INVALID_NAME;

	auth = _internal_check_access(ns, n->name, uid, gid, NOTIFY_ACCESS_WRITE);
	if (auth != 0) return NOTIFY_STATUS_NOT_AUTHORIZED;

	n->val++;

	for (l = n->subscriptions; l != NULL; l = _nc_list_next(l))
	{
		c = _nc_list_data(l);
		if (c != NULL) _internal_send(ns, c);
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

	if (ns == NULL) return NOTIFY_STATUS_FAILED;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	n = (name_info_t *)_nc_table_find(ns->name_table, name);
	if (n == NULL)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_INVALID_NAME;
	}

	status = _internal_post_name(ns, n, uid, gid);

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
	return status;
}

uint32_t
_notify_lib_post_nid(notify_state_t *ns, uint64_t nid, uid_t uid, gid_t gid)
{
	name_info_t *n;
	uint32_t status;

	if (ns == NULL) return NOTIFY_STATUS_FAILED;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	n = (name_info_t *)_nc_table_find_64(ns->name_id_table, nid);
	if (n == NULL)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_INVALID_NAME;
	}

	status = _internal_post_name(ns, n, uid, gid);

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
	return status;
}

static void
_internal_release_name_info(notify_state_t *ns, name_info_t *n)
{
	if (ns == NULL) return;
	if (n == NULL) return;

	if (n->refcount > 0) n->refcount--;
	if (n->refcount == 0)
	{
		_internal_remove_controlled_name(ns, n);
		_nc_table_delete(ns->name_table, n->name);
		_nc_table_delete_64(ns->name_id_table, n->name_id);
		_nc_list_release_list(n->subscriptions);
		free(n);
		ns->stat_name_free++;
	}
}

/*
 * Cancel (delete) a client
 */
static void
_internal_cancel(notify_state_t *ns, uint64_t cid)
{
	client_t *c;
	name_info_t *n;

	if (ns == NULL) return;

	c = NULL;
	n = NULL;

	c = _nc_table_find_64(ns->client_table, cid);
	if (c == NULL) return;

	n = c->name_info;
	if (n == NULL) return;

	n->subscriptions =_nc_list_find_release(n->subscriptions, c);
	_internal_client_release(ns, c);
	_internal_release_name_info(ns, n);
}

void
_notify_lib_cancel(notify_state_t *ns, pid_t pid, int token)
{
	uint64_t cid;

	if (ns == NULL) return;

	cid = make_client_id(pid, token);

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);
	_internal_cancel(ns, cid);
	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
}

void
_notify_lib_suspend(notify_state_t *ns, pid_t pid, int token)
{
	client_t *c;
	uint64_t cid;

	if (ns == NULL) return;

	cid = make_client_id(pid, token);

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	c = _nc_table_find_64(ns->client_table, cid);
	if (c != NULL)
	{
		c->state |= NOTIFY_CLIENT_STATE_SUSPENDED;
		if (c->suspend_count < UINT32_MAX) c->suspend_count++;
	}

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
}

uint32_t
_notify_lib_resume(notify_state_t *ns, pid_t pid, int token)
{
	client_t *c;
	uint64_t cid;
	uint32_t status = NOTIFY_STATUS_OK;

	if (ns == NULL) return NOTIFY_STATUS_FAILED;

	cid = make_client_id(pid, token);

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	c = _nc_table_find_64(ns->client_table, cid);
	if (c != NULL)
	{
		if (c->suspend_count > 0) c->suspend_count--;
		if (c->suspend_count == 0)
		{
			c->state &= ~NOTIFY_CLIENT_STATE_SUSPENDED;
			c->state &= ~NOTIFY_CLIENT_STATE_TIMEOUT;

			if (c->state & NOTIFY_CLIENT_STATE_PENDING)
			{
				status = _internal_send(ns, c);
			}
		}
	}

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);

	return status;
}

void
_notify_lib_suspend_proc(notify_state_t *ns, pid_t pid)
{
	portproc_data_t *pdata;

	if (ns == NULL) return;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	pdata = _nc_table_find_n(ns->proc_table, pid);
	if (pdata != NULL) pdata->flags |= NOTIFY_PORT_PROC_STATE_SUSPENDED;

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
}

void
_notify_lib_resume_proc(notify_state_t *ns, pid_t pid)
{
	client_t *c;
	void *tt;
	portproc_data_t *pdata;

	if (ns == NULL) return;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	/* Resume all subscriptions for this process */
	pdata = _nc_table_find_n(ns->proc_table, pid);
	if (pdata != NULL) pdata->flags &= ~NOTIFY_PORT_PROC_STATE_SUSPENDED;

	tt = _nc_table_traverse_start(ns->client_table);
	while (tt != NULL)
	{
		c = _nc_table_traverse(ns->client_table, tt);
		if (c == NULL) break;

		if (c->pid == pid)
		{
			if (c->suspend_count > 0) c->suspend_count--;
			if (c->suspend_count == 0)
			{
				c->state &= ~NOTIFY_CLIENT_STATE_SUSPENDED;
				c->state &= ~NOTIFY_CLIENT_STATE_TIMEOUT;

				if (c->state & NOTIFY_CLIENT_STATE_PENDING)
				{
					_internal_send(ns, c);
				}
			}
		}
	}
	_nc_table_traverse_end(ns->client_table, tt);

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
}

void
_notify_lib_suspend_port(notify_state_t *ns, mach_port_t port)
{
	portproc_data_t *pdata;

	if (ns == NULL) return;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	pdata = _nc_table_find_n(ns->port_table, port);
	if (pdata != NULL) pdata->flags |= NOTIFY_PORT_PROC_STATE_SUSPENDED;

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
}

void
_notify_lib_resume_port(notify_state_t *ns, mach_port_t port)
{
	client_t *c;
	void *tt;
	portproc_data_t *pdata;

	if (ns == NULL) return;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	/* Resume all subscriptions with this port */
	pdata = _nc_table_find_n(ns->port_table, port);
	if (pdata != NULL) pdata->flags &= ~NOTIFY_PORT_PROC_STATE_SUSPENDED;

	tt = _nc_table_traverse_start(ns->client_table);
	while (tt != NULL)
	{
		c = _nc_table_traverse(ns->client_table, tt);
		if (c == NULL) break;

		if (c->port == port)
		{
			if (c->suspend_count > 0) c->suspend_count--;
			if (c->suspend_count == 0)
			{
				c->state &= ~NOTIFY_CLIENT_STATE_SUSPENDED;
				c->state &= ~NOTIFY_CLIENT_STATE_TIMEOUT;

				if (c->state & NOTIFY_CLIENT_STATE_PENDING)
				{
					_internal_send(ns, c);
				}
			}
		}
	}
	_nc_table_traverse_end(ns->client_table, tt);

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
}

/*
 * Delete all clients for a process
 * N.B. notifyd does not use this routine.
 */
void
_notify_lib_cancel_proc(notify_state_t *ns, pid_t pid)
{
	client_t *c;
	void *tt;
	list_t *l, *x;

	if (ns == NULL) return;

	x = NULL;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	tt = _nc_table_traverse_start(ns->client_table);
	while (tt != NULL)
	{
		c = _nc_table_traverse(ns->client_table, tt);
		if (c == NULL) break;

		if (c->pid == pid) x = _nc_list_prepend(x, _nc_list_new(c));
	}
	_nc_table_traverse_end(ns->client_table, tt);

	for (l = x; l != NULL; l = _nc_list_next(l))
	{
		c = _nc_list_data(l);
		_internal_cancel(ns, c->client_id);
	}

	_nc_list_release_list(x);

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
}

/*
 * Delete all clients for a port
 * N.B. notifyd does not use this routine.
 */
void
_notify_lib_cancel_port(notify_state_t *ns, mach_port_t port)
{
	client_t *c;
	void *tt;
	list_t *l, *x;

	if (ns == NULL) return;

	x = NULL;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	tt = _nc_table_traverse_start(ns->client_table);
	while (tt != NULL)
	{
		c = _nc_table_traverse(ns->client_table, tt);
		if (c == NULL) break;

		if (c->port == port) x = _nc_list_prepend(x, _nc_list_new(c));
	}
	_nc_table_traverse_end(ns->client_table, tt);

	for (l = x; l != NULL; l = _nc_list_next(l))
	{
		c = _nc_list_data(l);
		_internal_cancel(ns, c->client_id);
	}

	_nc_list_release_list(x);

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
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

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (check == NULL) return NOTIFY_STATUS_FAILED;

	cid = make_client_id(pid, token);

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	c = _nc_table_find_64(ns->client_table, cid);

	if (c == NULL)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_INVALID_TOKEN;
	}

	if (c->name_info == NULL)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_INVALID_TOKEN;
	}

	if (c->name_info->val == c->lastval)
	{
		*check = 0;
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_OK;
	}

	c->lastval = c->name_info->val;
	*check = 1;

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
	return NOTIFY_STATUS_OK;
}

/*
 * SPI: get value for a name.
 */
uint32_t
_notify_lib_peek(notify_state_t *ns, pid_t pid, int token, int *val)
{
	client_t *c;
	uint64_t cid;

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (val == NULL) return NOTIFY_STATUS_FAILED;

	cid = make_client_id(pid, token);

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	c = _nc_table_find_64(ns->client_table, cid);

	if (c == NULL)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_INVALID_TOKEN;
	}

	if (c->name_info == NULL)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_INVALID_TOKEN;
	}

	*val = c->name_info->val;

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
	return NOTIFY_STATUS_OK;
}

int *
_notify_lib_check_addr(notify_state_t *ns, pid_t pid, int token)
{
	client_t *c;
	int *addr;
	uint64_t cid;

	if (ns == NULL) return NULL;

	cid = make_client_id(pid, token);

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	c = _nc_table_find_64(ns->client_table, cid);

	if (c == NULL)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NULL;
	}

	if (c->name_info == NULL)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NULL;
	}

	addr = (int *)&(c->name_info->val);

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
	return addr;
}

/*
 * Get state value for a name.
 */
uint32_t
_notify_lib_get_state(notify_state_t *ns, uint64_t nid, uint64_t *state, uid_t uid, gid_t gid)
{
	name_info_t *n;

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (state == NULL) return NOTIFY_STATUS_FAILED;

	*state = 0;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	n = (name_info_t *)_nc_table_find_64(ns->name_id_table, nid);

	if (n == NULL)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_INVALID_NAME;
	}

#ifdef GET_STATE_AUTH_CHECK
	int auth = _internal_check_access(ns, n->name, uid, gid, NOTIFY_ACCESS_READ);
	if (auth != 0)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_NOT_AUTHORIZED;
	}
#endif

	*state = n->state;

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
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

	if (ns == NULL) return NOTIFY_STATUS_FAILED;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	n = (name_info_t *)_nc_table_find_64(ns->name_id_table, nid);

	if (n == NULL)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_INVALID_NAME;
	}

	auth = _internal_check_access(ns, n->name, uid, gid, NOTIFY_ACCESS_WRITE);
	if (auth != 0)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_NOT_AUTHORIZED;
	}

	n->state = state;
	n->state_time = mach_absolute_time();

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
	return NOTIFY_STATUS_OK;
}

static uint32_t
_internal_register_common(notify_state_t *ns, const char *name, pid_t pid, int token, uid_t uid, gid_t gid, client_t **outc)
{
	client_t *c;
	name_info_t *n;
	int is_new_name;
	uint32_t status;

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;
	if (outc == NULL) return NOTIFY_STATUS_OK;

	status = _internal_check_access(ns, name, uid, gid, NOTIFY_ACCESS_READ);
	if (status != NOTIFY_STATUS_OK) return NOTIFY_STATUS_NOT_AUTHORIZED;

	*outc = NULL;
	is_new_name = 0;

	n = (name_info_t *)_nc_table_find(ns->name_table, name);
	if (n == NULL)
	{
		is_new_name = 1;

		n = _internal_new_name(ns, name);
		if (n == NULL) return NOTIFY_STATUS_FAILED;
	}

	c = _internal_client_new(ns, pid, token);
	if (c == NULL)
	{
		if (is_new_name == 1)
		{
			_nc_table_delete(ns->name_table, n->name);
			_nc_list_release_list(n->subscriptions);
			free(n);
			ns->stat_name_free++;
		}

		return NOTIFY_STATUS_FAILED;
	}

	n->refcount++;

	c->name_info = n;
	n->subscriptions = _nc_list_prepend(n->subscriptions, _nc_list_new(c));

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

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	c = NULL;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	status = _internal_register_common(ns, name, pid, token, uid, gid, &c);
	if (status != NOTIFY_STATUS_OK)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return status;
	}

	c->notify_type = NOTIFY_TYPE_SIGNAL;
	c->pid = pid;
	c->sig = sig;
	*out_nid = c->name_info->name_id;

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
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

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	c = NULL;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	status = _internal_register_common(ns, name, pid, token, uid, gid, &c);
	if (status != NOTIFY_STATUS_OK)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return status;
	}

	c->notify_type = NOTIFY_TYPE_FILE;
	c->fd = fd;
	*out_nid = c->name_info->name_id;

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
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

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	c = NULL;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	status = _internal_register_common(ns, name, pid, token, uid, gid, &c);
	if (status != NOTIFY_STATUS_OK)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return status;
	}

	c->notify_type = NOTIFY_TYPE_PORT;
	c->port = port;
	*out_nid = c->name_info->name_id;

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
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

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	c = NULL;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	status = _internal_register_common(ns, name, pid, token, uid, gid, &c);
	if (status != NOTIFY_STATUS_OK)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return status;
	}

	if (slot == SLOT_NONE)
	{
		c->notify_type = NOTIFY_TYPE_PLAIN;
	}
	else
	{
		c->notify_type = NOTIFY_TYPE_MEMORY;
		c->name_info->slot = slot;
	}

	*out_nid = c->name_info->name_id;

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
	return NOTIFY_STATUS_OK;
}

uint32_t
_notify_lib_set_owner(notify_state_t *ns, const char *name, uid_t uid, gid_t gid)
{
	name_info_t *n;

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	n = (name_info_t *)_nc_table_find(ns->name_table, name);
	if (n == NULL)
	{
		/* create new name */
		n = _internal_new_name(ns, name);
		if (n == NULL)
		{
			if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
			return NOTIFY_STATUS_FAILED;
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

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
	return NOTIFY_STATUS_OK;
}

uint32_t
_notify_lib_get_owner(notify_state_t *ns, const char *name, uint32_t *uid, uint32_t *gid)
{
	name_info_t *n;
	int i, nlen, len;

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	n = (name_info_t *)_nc_table_find(ns->name_table, name);
	if (n != NULL)
	{
		*uid = n->uid;
		*gid = n->gid;
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_OK;
	}

	len = strlen(name);

	for (i = 0; i < ns->controlled_name_count; i++)
	{
		n = ns->controlled_name[i];
		if (n == NULL) break;

		nlen = strlen(n->name);

		if (!strcmp(n->name, name))
		{
			*uid = n->uid;
			*gid = n->gid;
			if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
			return NOTIFY_STATUS_OK;
		}

		/* check if this key is a prefix */
		if (nlen >= len) continue;
		if (strncmp(n->name, name, nlen)) continue;

		*uid = n->uid;
		*gid = n->gid;

		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_OK;
	}

	*uid = 0;
	*gid = 0;

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
	return NOTIFY_STATUS_OK;
}

uint32_t
_notify_lib_set_access(notify_state_t *ns, const char *name, uint32_t mode)
{
	name_info_t *n;

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	n = (name_info_t *)_nc_table_find(ns->name_table, name);
	if (n == NULL)
	{
		/* create new name */
		n = _internal_new_name(ns, name);
		if (n == NULL)
		{
			if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
			return NOTIFY_STATUS_FAILED;
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

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
	return NOTIFY_STATUS_OK;
}

uint32_t
_notify_lib_get_access(notify_state_t *ns, const char *name, uint32_t *mode)
{
	name_info_t *n;
	int i, nlen, len;

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	n = (name_info_t *)_nc_table_find(ns->name_table, name);
	if (n != NULL)
	{
		*mode = n->access;

		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_OK;
	}

	len = strlen(name);

	for (i = 0; i < ns->controlled_name_count; i++)
	{
		n = ns->controlled_name[i];
		if (n == NULL) break;
		if (n->name == NULL) continue;

		nlen = strlen(n->name);

		if (!strcmp(n->name, name))
		{
			*mode = n->access;

			if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
			return NOTIFY_STATUS_OK;
		}

		/* check if this key is a prefix */
		if (nlen >= len) continue;
		if (strncmp(n->name, name, nlen)) continue;

		*mode = n->access;

		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_OK;
	}

	*mode = NOTIFY_ACCESS_DEFAULT;

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
	return NOTIFY_STATUS_OK;
}

uint32_t
_notify_lib_release_name(notify_state_t *ns, const char *name, uid_t uid, gid_t gid)
{
	name_info_t *n;

	if (ns == NULL) return NOTIFY_STATUS_FAILED;
	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	if (ns->lock != NULL) pthread_mutex_lock(ns->lock);

	n = (name_info_t *)_nc_table_find(ns->name_table, name);
	if (n == NULL)
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_INVALID_NAME;
	}

	/* Owner and root may release */
	if ((n->uid != uid) && (uid != 0))
	{
		if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
		return NOTIFY_STATUS_NOT_AUTHORIZED;
	}

	_internal_release_name_info(ns, n);

	if (ns->lock != NULL) pthread_mutex_unlock(ns->lock);
	return NOTIFY_STATUS_OK;
}

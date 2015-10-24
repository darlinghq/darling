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
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/signal.h>
#include <sys/syslimits.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <bootstrap_priv.h>
#include <errno.h>
#include <pthread.h>
#include <TargetConditionals.h>
#include <AvailabilityMacros.h>
#include <libkern/OSAtomic.h>
#include <Block.h>
#include <dispatch/dispatch.h>
#include <dispatch/private.h>
#include <_simple.h>

#include "libnotify.h"

#include "notify.h"
#include "notify_internal.h"
#include "notify_ipc.h"
#include "notify_private.h"

#define INITIAL_TOKEN_ID 0

// <rdar://problem/10385540>
WEAK_IMPORT_ATTRIBUTE bool _dispatch_is_multithreaded(void);

#define EVENT_INIT       0
#define EVENT_REGEN      1

#define SELF_PREFIX "self."
#define SELF_PREFIX_LEN 5

#define COMMON_SELF_PORT_KEY "self.com.apple.system.notify.common"

#define MULTIPLE_REGISTRATION_WARNING_TRIGGER 20

extern uint32_t _notify_lib_peek(notify_state_t *ns, pid_t pid, int token, int *val);
extern int *_notify_lib_check_addr(notify_state_t *ns, pid_t pid, int token);

#define CLIENT_TOKEN_TABLE_SIZE 256

#define NID_UNSET 0xffffffffffffffffL
#define NID_CALLED_ONCE 0xfffffffffffffffeL

#define NO_LOCK 1

typedef struct
{
	uint32_t refcount;
	uint64_t name_id;
} name_table_node_t;

typedef struct
{
	uint32_t refcount;
	const char *name;
	size_t namelen;
	name_table_node_t *name_node;
	uint32_t token;
	uint32_t slot;
	uint32_t val;
	uint32_t flags;
	int fd;
	int signal;
	mach_port_t mp;
	uint32_t client_id;
	uint64_t set_state_time;
	uint64_t set_state_val;
	char * path;
	int path_flags;
	dispatch_queue_t queue;
	notify_handler_t block;
} token_table_node_t;

/* FORWARD */
static void _notify_lib_regenerate(int src);
static void notify_retain_mach_port(mach_port_t mp, int mine);
static void _notify_dispatch_handle(mach_port_t port);
static notify_state_t *_notify_lib_self_state();

#if TARGET_IPHONE_SIMULATOR
const char *
_notify_shm_id()
{
	static dispatch_once_t once;
	static char *shm_id;

	dispatch_once(&once, ^{
		// According to documentation, our shm_id must be no more than 31 characters long
		// but in practice, even 31 characters is too long (<rdar://problem/16860882>),
		// so we jump through some hoops to make a smaller string based on our UDID.
		const char *udid = getenv("SIMULATOR_UDID");
		if (udid && strlen(udid) == 36) {
			char scratch[34]; // 32 characters, 2 NUL

			// 01234567890123456789012345678901234567890
			// UUUUUUUU-UUUU-UUUU-LLLL-LLLLLLLLLLLL
			memcpy(scratch, udid, 8);
			memcpy(scratch+8, udid+9, 4);
			memcpy(scratch+12, udid+14, 4);
			scratch[16] = '\0';

			memcpy(scratch+17, udid+19, 4);
			memcpy(scratch+21, udid+24, 12);
			scratch[33] = '\0';

			// If the input is invalid, these will end up being undefined
			// values, but they'll still be values we can use.
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
			// Drop a bit on the floor, but that probably doesn't matter.  It does not need to be reversible

			scratch[10] = c64[(lower >> 57) & 0xf];
			scratch[11] = c64[(lower >> 50) & 0xf];
			scratch[12] = c64[(lower >> 43) & 0xf];
			scratch[13] = c64[(lower >> 36) & 0xf];
			scratch[14] = c64[(lower >> 29) & 0xf];
			scratch[15] = c64[(lower >> 22) & 0xf];
			scratch[16] = c64[(lower >> 15) & 0xf];
			scratch[17] = c64[(lower >>  8) & 0xf];
			scratch[18] = c64[(lower >>  1) & 0xf];
			// Drop a bit on the floor, but that probably doesn't matter.  It does not need to be reversible

			scratch[19] = '\0';

			asprintf(&shm_id, "sim.not.%s", scratch);
			assert(shm_id);
		}

		if (!shm_id) {
			shm_id = "apple.shm.notification_center";
		}
	});

	return shm_id;
}
#endif

static int
shm_attach(uint32_t size)
{
	int32_t shmfd;
	notify_globals_t globals = _notify_globals();

	shmfd = shm_open(SHM_ID, O_RDONLY, 0);
	if (shmfd == -1) return -1;

	globals->shm_base = mmap(NULL, size, PROT_READ, MAP_SHARED, shmfd, 0);
	close(shmfd);

	if (globals->shm_base == (uint32_t *)-1) globals->shm_base = NULL;
	if (globals->shm_base == NULL) return -1;

	return 0;
}

#ifdef NOTDEF
static void
shm_detach(void)
{
	if (shm_base != NULL)
	{
		shmdt(shm_base);
		shm_base = NULL;
	}
}
#endif

/*
 * Initialization of global variables. Called once per process.
 */
void
_notify_init_globals(void * /* notify_globals_t */ _globals)
{
	notify_globals_t globals = _globals;

	pthread_mutex_init(&globals->notify_lock, NULL);
	globals->token_id = INITIAL_TOKEN_ID;
	globals->notify_common_token = -1;
}

#if !_NOTIFY_HAS_ALLOC_ONCE
notify_globals_t
_notify_globals_impl(void)
{
	static dispatch_once_t once;
	static notify_globals_t globals;
	dispatch_once(&once, ^{
		globals = calloc(1, sizeof(struct notify_globals_s));
		_notify_init_globals(globals);
	});
	return globals;
}
#endif

/*
 * _notify_lib_init is called for each new registration (event = EVENT_INIT).
 * It is also called to re-initialize when the library has detected that
 * notifyd has restarted (event = EVENT_REGEN).
 */
static uint32_t
_notify_lib_init(uint32_t event)
{
	__block kern_return_t kstatus;
	__block bool first = false;
	int status, cid;
	uint64_t state;

	notify_globals_t globals = _notify_globals();

	/* notifyd sets NOTIFY_OPT_DISABLE to avoid re-entrancy issues */
	if (globals->client_opts & NOTIFY_OPT_DISABLE) return NOTIFY_STATUS_FAILED;

	/* Look up the notifyd server port just once. */
	kstatus = KERN_SUCCESS;
	dispatch_once(&globals->notify_server_port_once, ^{
		first = true;
		kstatus = bootstrap_look_up2(bootstrap_port, NOTIFY_SERVICE_NAME, &globals->notify_server_port, 0, BOOTSTRAP_PRIVILEGED_SERVER);
	});

	if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;

	pthread_mutex_lock(&globals->notify_lock);

	/*
	 * _dispatch_is_multithreaded() tells us if it is safe to use dispatch queues for
	 * a shared port for all registratios, and to watch for notifyd exiting / restarting.
	 *
	 * Note that _dispatch_is_multithreaded is weak imported, <rdar://problem/10385540>
	 */
	if (_dispatch_is_multithreaded)
	{
		if (_dispatch_is_multithreaded()) globals->client_opts |= (NOTIFY_OPT_DEMUX | NOTIFY_OPT_REGEN);
	}

	/*
	 * Look up the server's PID and supported IPC version on the first call,
	 * and on a regeneration event (when the server has restarted).
	 */
	if (first || (event == EVENT_REGEN))
	{
		pid_t last_pid = globals->notify_server_pid;

		globals->notify_ipc_version = 0;
		globals->notify_server_pid = 0;

		kstatus = _notify_server_register_plain(globals->notify_server_port, NOTIFY_IPC_VERSION_NAME, NOTIFY_IPC_VERSION_NAME_LEN, &cid, &status);
		if ((kstatus == KERN_SUCCESS) && (status == NOTIFY_STATUS_OK))
		{
			kstatus = _notify_server_get_state(globals->notify_server_port, cid, &state, &status);
			if ((kstatus == KERN_SUCCESS) && (status == NOTIFY_STATUS_OK))
			{
				globals->notify_ipc_version = state;
				state >>= 32;
				globals->notify_server_pid = state;
			}

			_notify_server_cancel(globals->notify_server_port, cid, &status);

			if ((last_pid == globals->notify_server_pid) && (event == EVENT_REGEN))
			{
				pthread_mutex_unlock(&globals->notify_lock);
				return NOTIFY_STATUS_INVALID_REQUEST;
			}
		}

		if (globals->server_proc_source != NULL)
		{
			dispatch_source_cancel(globals->server_proc_source);
			dispatch_release(globals->server_proc_source);
			globals->server_proc_source = NULL;
		}
	}

	if (globals->notify_ipc_version < 2)
	{
		/* regen is not supported below version 2 */
		globals->client_opts &= ~NOTIFY_OPT_REGEN;
	}

	/*
	 * Create a source (DISPATCH_SOURCE_TYPE_PROC) to invoke _notify_lib_regenerate if notifyd restarts.
	 * Available in IPC version 2.
	 */
	if ((globals->server_proc_source == NULL) && (globals->client_opts & NOTIFY_OPT_REGEN) && (globals->notify_server_pid != 0))
	{
		globals->server_proc_source = dispatch_source_create(DISPATCH_SOURCE_TYPE_PROC, (uintptr_t)globals->notify_server_pid, DISPATCH_PROC_EXIT, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0));
		dispatch_source_set_event_handler(globals->server_proc_source, ^{ _notify_lib_regenerate(1); });
		dispatch_resume(globals->server_proc_source);
	}

	/*
	 * Create the shared multiplex ports if NOTIFY_OPT_DEMUX is set.
	 */
	if ((globals->client_opts & NOTIFY_OPT_DEMUX) && (globals->notify_common_port == MACH_PORT_NULL))
	{
		kern_return_t kr;
		task_t task = mach_task_self();

		kr = mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE, &globals->notify_common_port);
		if (kr == KERN_SUCCESS)
		{
			globals->notify_dispatch_source = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, globals->notify_common_port, 0, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0));
			dispatch_source_set_event_handler(globals->notify_dispatch_source, ^{
				notify_globals_t globals = _notify_globals();
				_notify_dispatch_handle(globals->notify_common_port);
			});
			dispatch_source_set_cancel_handler(globals->notify_dispatch_source, ^{
				task_t task = mach_task_self();
				notify_globals_t globals = _notify_globals();
				mach_port_mod_refs(task, globals->notify_common_port, MACH_PORT_RIGHT_RECEIVE, -1);
			});
			dispatch_resume(globals->notify_dispatch_source);
		}
	}

	pthread_mutex_unlock(&globals->notify_lock);

	if (globals->notify_common_port != MACH_PORT_NULL && (first || event == EVENT_REGEN))
	{
		/* register the common port with notifyd */
		status = notify_register_mach_port(COMMON_PORT_KEY, &globals->notify_common_port, NOTIFY_REUSE, &globals->notify_common_token);
	}

	return NOTIFY_STATUS_OK;
}

/* Reset all internal state at fork */
void
_notify_fork_child(void)
{
	notify_globals_t globals = _notify_globals();

	_notify_init_globals(globals);

	/*
	 * Expressly disable notify in the child side of a fork if it had
	 * been initialized in the parent. Using notify in the child process
	 * can lead to deadlock (see <rdar://problem/11498014>).
	 *
	 * Also disable notify in the forked child of a multi-threaded parent that
	 * used dispatch, since notify will use dispatch, and that will blow up.
	 * It's OK to make that check here by calling _dispatch_is_multithreaded(),
	 * since we will actually be looking at the parent's state.
	 */
	if (globals->notify_server_port != MACH_PORT_NULL) globals->client_opts = NOTIFY_OPT_DISABLE;
	if (_dispatch_is_multithreaded) // weak imported symbol
	{
		if (_dispatch_is_multithreaded()) globals->client_opts = NOTIFY_OPT_DISABLE;
	}

	globals->self_state = NULL;
	globals->notify_server_port = MACH_PORT_NULL;
	globals->notify_ipc_version = 0;
	globals->notify_server_pid = 0;

	globals->token_table = NULL;
	globals->token_name_table = NULL;

	globals->fd_count = 0;
	globals->fd_clnt = NULL;
	globals->fd_srv = NULL;
	globals->fd_refcount = NULL;

	globals->mp_count = 0;
	globals->mp_list = NULL;
	globals->mp_refcount = NULL;
	globals->mp_mine = NULL;

	globals->shm_base = NULL;
}

static uint32_t
token_table_add(const char *name, size_t namelen, uint64_t nid, uint32_t token, uint32_t cid, uint32_t slot, uint32_t flags, int sig, int fd, mach_port_t mp, int lock)
{
	token_table_node_t *t;
	name_table_node_t *n;
	uint32_t warn_count = 0;
	notify_globals_t globals = _notify_globals();

	dispatch_once(&globals->token_table_once, ^{
		globals->token_table = _nc_table_new(CLIENT_TOKEN_TABLE_SIZE);
		globals->token_name_table = _nc_table_new(CLIENT_TOKEN_TABLE_SIZE);
	});

	if (globals->token_table == NULL) return -1;
	if (globals->token_name_table == NULL) return -1;
	if (name == NULL) return -1;

	t = (token_table_node_t *)calloc(1, sizeof(token_table_node_t));
	if (t == NULL) return -1;

	t->refcount = 1;

	/* we will get t->name from the token_name_table */
	t->name = NULL;

	t->namelen = namelen;
	t->token = token;
	t->slot = slot;
	t->val = 0;
	t->flags = flags;
	t->fd = fd;
	t->mp = mp;
	t->client_id = cid;

	if (lock != NO_LOCK) pthread_mutex_lock(&globals->notify_lock);
	_nc_table_insert_n(globals->token_table, t->token, t);

	/* check if we have this name in the name table */
	n = _nc_table_find_get_key(globals->token_name_table, name, &(t->name));
	if (n == NULL)
	{
		char *copy_name = strdup(name);
		if (copy_name == NULL)
		{
			free(t);
			if (lock != NO_LOCK) pthread_mutex_unlock(&globals->notify_lock);
			return -1;
		}

		t->name = (const char *)copy_name;

		/* create a new name table node */
		n = (name_table_node_t *)calloc(1, sizeof(name_table_node_t));
		if (n != NULL)
		{
			n->refcount = 1;
			n->name_id = nid;

			/* the name table node "owns" the name */
			_nc_table_insert_pass(globals->token_name_table, copy_name, n);
			t->name_node = n;
		}
	}
	else
	{
		/* this token retains the name table node */
		t->name_node = n;
		n->refcount++;

		if ((n->refcount % MULTIPLE_REGISTRATION_WARNING_TRIGGER) == 0)
		{
			warn_count = n->refcount;
		}
	}

	if (lock != NO_LOCK) pthread_mutex_unlock(&globals->notify_lock);

	if (warn_count > 0)
	{
		char *msg;
		asprintf(&msg, "notify name \"%s\" has been registered %d times - this may be a leak", name, warn_count);
		if (msg)
			_simple_asl_log(ASL_LEVEL_WARNING, "com.apple.notify", msg);
		free(msg);
	}

	return 0;
}

static token_table_node_t *
token_table_find_retain(uint32_t token)
{
	token_table_node_t *t;
	notify_globals_t globals = _notify_globals();

	pthread_mutex_lock(&globals->notify_lock);

	t = (token_table_node_t *)_nc_table_find_n(globals->token_table, token);
	if (t != NULL) t->refcount++;

	pthread_mutex_unlock(&globals->notify_lock);

	return t;
}

static token_table_node_t *
token_table_find_no_lock(uint32_t token)
{
	notify_globals_t globals = _notify_globals();
	return (token_table_node_t *)_nc_table_find_n(globals->token_table, token);
}

static name_table_node_t *
name_table_find_retain_no_lock(const char *name)
{
	name_table_node_t *n;
	notify_globals_t globals = _notify_globals();

	n = (name_table_node_t *)_nc_table_find(globals->token_name_table, name);
	if (n != NULL) n->refcount++;

	return n;
}

static void
name_table_release_no_lock(const char *name)
{
	name_table_node_t *n;
	notify_globals_t globals = _notify_globals();

	n = (name_table_node_t *)_nc_table_find(globals->token_name_table, name);
	if (n != NULL)
	{
		if (n->refcount > 0) n->refcount--;
		if (n->refcount == 0)
		{
			_nc_table_delete(globals->token_name_table, name);
			free(n);
		}
	}
}

static void
name_table_set_nid(const char *name, uint64_t nid)
{
	name_table_node_t *n;
	notify_globals_t globals = _notify_globals();

	pthread_mutex_lock(&globals->notify_lock);

	n = (name_table_node_t *)_nc_table_find(globals->token_name_table, name);
	if (n != NULL) n->name_id = nid;

	pthread_mutex_unlock(&globals->notify_lock);
}

static void
_notify_lib_regenerate_token(token_table_node_t *t)
{
	uint32_t type;
	int status, new_slot;
	kern_return_t kstatus;
	mach_port_t port;
	uint64_t new_nid;
	size_t pathlen;

	if (t == NULL) return;
	if (t->name == NULL) return;
	if (t->flags & NOTIFY_FLAG_SELF) return;
	if ((t->flags & NOTIFY_FLAG_REGEN) == 0) return;
	if (!strcmp(t->name, COMMON_PORT_KEY)) return;

	notify_globals_t globals = _notify_globals();

	port = MACH_PORT_NULL;
	if (t->flags & NOTIFY_TYPE_PORT)
	{
		port = globals->notify_common_port;
	}

	pathlen = 0;
	if (t->path != NULL) pathlen = strlen(t->path);
	type = t->flags & 0x000000ff;

	kstatus = _notify_server_regenerate(globals->notify_server_port, (caddr_t)t->name, t->namelen, t->token, type, port, t->signal, t->slot, t->set_state_val, t->set_state_time, t->path, pathlen, t->path_flags, &new_slot, &new_nid, &status);

	if (kstatus != KERN_SUCCESS) status = NOTIFY_STATUS_FAILED;
	if (status != NOTIFY_STATUS_OK) return;

	t->slot = new_slot;

	/* reset the name_id in the name table node */
	if (t->name_node != NULL) t->name_node->name_id = new_nid;
}

/*
 * Invoked when server has died.
 * Regenerates all registrations and state.
 */
static void
_notify_lib_regenerate(int src)
{
	void *tt;
	token_table_node_t *t;
	notify_globals_t globals = _notify_globals();

	if ((globals->client_opts & NOTIFY_OPT_REGEN) == 0) return;

	/* _notify_lib_init returns an error if regeneration is unnecessary */
	if (_notify_lib_init(EVENT_REGEN) == NOTIFY_STATUS_OK)
	{
		pthread_mutex_lock(&globals->notify_lock);

		tt = _nc_table_traverse_start(globals->token_table);
		while (tt != NULL)
		{
			t = _nc_table_traverse(globals->token_table, tt);
			if (t == NULL) break;
			_notify_lib_regenerate_token(t);
		}

		_nc_table_traverse_end(globals->token_table, tt);

		pthread_mutex_unlock(&globals->notify_lock);
	}
}

/*
 * Regenerate if the server PID (shared memory slot 0) has changed.
 */
static inline void
regenerate_check()
{
	notify_globals_t globals = _notify_globals();

	if ((globals->client_opts & NOTIFY_OPT_REGEN) == 0) return;

	if ((globals->shm_base != NULL) && (globals->shm_base[0] != globals->notify_server_pid)) _notify_lib_regenerate(0);
}

/* notify_lock is required in notify_retain_file_descriptor */
static void
notify_retain_file_descriptor(int clnt, int srv)
{
	int x, i;
	notify_globals_t globals = _notify_globals();

	if (clnt < 0) return;
	if (srv < 0) return;

	pthread_mutex_lock(&globals->notify_lock);

	x = -1;
	for (i = 0; (i < globals->fd_count) && (x < 0); i++)
	{
		if (globals->fd_clnt[i] == clnt) x = i;
	}

	if (x >= 0)
	{
		globals->fd_refcount[x]++;
		pthread_mutex_unlock(&globals->notify_lock);
		return;
	}

	x = globals->fd_count;
	globals->fd_count++;

	if (x == 0)
	{
		globals->fd_clnt = (int *)calloc(1, sizeof(int));
		globals->fd_srv = (int *)calloc(1, sizeof(int));
		globals->fd_refcount = (int *)calloc(1, sizeof(int));
	}
	else
	{
		globals->fd_clnt = (int *)reallocf(globals->fd_clnt, globals->fd_count * sizeof(int));
		globals->fd_srv = (int *)reallocf(globals->fd_srv, globals->fd_count * sizeof(int));
		globals->fd_refcount = (int *)reallocf(globals->fd_refcount, globals->fd_count * sizeof(int));
	}

	if ((globals->fd_clnt == NULL) || (globals->fd_srv == NULL) || (globals->fd_refcount == NULL))
	{
		free(globals->fd_clnt);
		free(globals->fd_srv);
		free(globals->fd_refcount);
		globals->fd_count = 0;
	}
	else
	{
		globals->fd_clnt[x] = clnt;
		globals->fd_srv[x] = srv;
		globals->fd_refcount[x] = 1;
	}

	pthread_mutex_unlock(&globals->notify_lock);
}

/* notify_lock is NOT required in notify_release_file_descriptor */
static void
notify_release_file_descriptor(int fd)
{
	int x, i, j;
	notify_globals_t globals = _notify_globals();

	if (fd < 0) return;

	x = -1;
	for (i = 0; (i < globals->fd_count) && (x < 0); i++)
	{
		if (globals->fd_clnt[i] == fd) x = i;
	}

	if (x < 0) return;

	if (globals->fd_refcount[x] > 0) globals->fd_refcount[x]--;
	if (globals->fd_refcount[x] > 0) return;

	close(globals->fd_clnt[x]);
	close(globals->fd_srv[x]);

	if (globals->fd_count == 1)
	{
		free(globals->fd_clnt);
		free(globals->fd_srv);
		free(globals->fd_refcount);
		globals->fd_count = 0;
		return;
	}

	for (i = x + 1, j = x; i < globals->fd_count; i++, j++)
	{
		globals->fd_clnt[j] = globals->fd_clnt[i];
		globals->fd_srv[j] = globals->fd_srv[i];
		globals->fd_refcount[j] = globals->fd_refcount[i];
	}

	globals->fd_count--;

	globals->fd_clnt = (int *)reallocf(globals->fd_clnt, globals->fd_count * sizeof(int));
	globals->fd_srv = (int *)reallocf(globals->fd_srv, globals->fd_count * sizeof(int));
	globals->fd_refcount = (int *)reallocf(globals->fd_refcount, globals->fd_count * sizeof(int));

	if ((globals->fd_clnt == NULL) || (globals->fd_srv == NULL) || (globals->fd_refcount == NULL))
	{
		free(globals->fd_clnt);
		free(globals->fd_srv);
		free(globals->fd_refcount);
		globals->fd_count = 0;
	}
}

/* notify_lock is required in notify_retain_mach_port */
static void
notify_retain_mach_port(mach_port_t mp, int mine)
{
	int x, i;
	notify_globals_t globals = _notify_globals();

	if (mp == MACH_PORT_NULL) return;

	pthread_mutex_lock(&globals->notify_lock);

	x = -1;
	for (i = 0; (i < globals->mp_count) && (x < 0); i++)
	{
		if (globals->mp_list[i] == mp) x = i;
	}

	if (x >= 0)
	{
		globals->mp_refcount[x]++;
		pthread_mutex_unlock(&globals->notify_lock);
		return;
	}

	x = globals->mp_count;
	globals->mp_count++;

	if (x == 0)
	{
		globals->mp_list = (mach_port_t *)calloc(1, sizeof(mach_port_t));
		globals->mp_refcount = (int *)calloc(1, sizeof(int));
		globals->mp_mine = (int *)calloc(1, sizeof(int));
	}
	else
	{
		globals->mp_list = (mach_port_t *)reallocf(globals->mp_list, globals->mp_count * sizeof(mach_port_t));
		globals->mp_refcount = (int *)reallocf(globals->mp_refcount, globals->mp_count * sizeof(int));
		globals->mp_mine = (int *)reallocf(globals->mp_mine, globals->mp_count * sizeof(int));
	}

	if ((globals->mp_list == NULL) || (globals->mp_refcount == NULL) || (globals->mp_mine == NULL))
	{
		if (globals->mp_list != NULL) free(globals->mp_list);
		if (globals->mp_refcount != NULL) free(globals->mp_refcount);
		if (globals->mp_mine != NULL) free(globals->mp_mine);
		globals->mp_count = 0;
	}
	else
	{
		globals->mp_list[x] = mp;
		globals->mp_refcount[x] = 1;
		globals->mp_mine[x] = mine;
	}

	pthread_mutex_unlock(&globals->notify_lock);
}

/* notify_lock is NOT required in notify_release_mach_port */
static void
notify_release_mach_port(mach_port_t mp, uint32_t flags)
{
	int x, i;
	notify_globals_t globals = _notify_globals();

	if (mp == MACH_PORT_NULL) return;

	x = -1;
	for (i = 0; (i < globals->mp_count) && (x < 0); i++)
	{
		if (globals->mp_list[i] == mp) x = i;
	}

	if (x < 0) return;

	if (globals->mp_refcount[x] > 0) globals->mp_refcount[x]--;
	if (globals->mp_refcount[x] > 0) return;

	if (globals->mp_mine[x] == 1)
	{
		mach_port_mod_refs(mach_task_self(), mp, MACH_PORT_RIGHT_RECEIVE, -1);

		/* release send right if this is a self notification */
		if (flags & NOTIFY_FLAG_SELF) mach_port_deallocate(mach_task_self(), mp);
	}

	if (flags & NOTIFY_FLAG_RELEASE_SEND)
	{
		/* multiplexed registration holds a send right in Libnotify */
		mach_port_deallocate(mach_task_self(), mp);
	}

	if (globals->mp_count == 1)
	{
		if (globals->mp_list != NULL) free(globals->mp_list);
		if (globals->mp_refcount != NULL) free(globals->mp_refcount);
		if (globals->mp_mine != NULL) free(globals->mp_mine);
		globals->mp_count = 0;
		return;
	}

	for (i = x + 1; i < globals->mp_count; i++)
	{
		globals->mp_list[i - 1] = globals->mp_list[i];
		globals->mp_refcount[i - 1] = globals->mp_refcount[i];
		globals->mp_mine[i - 1] = globals->mp_mine[i];
	}

	globals->mp_count--;

	globals->mp_list = (mach_port_t *)reallocf(globals->mp_list, globals->mp_count * sizeof(mach_port_t));
	globals->mp_refcount = (int *)reallocf(globals->mp_refcount, globals->mp_count * sizeof(int));
	globals->mp_mine = (int *)reallocf(globals->mp_mine, globals->mp_count * sizeof(int));

	if ((globals->mp_list == NULL) || (globals->mp_refcount == NULL) || (globals->mp_mine == NULL))
	{
		if (globals->mp_list != NULL) free(globals->mp_list);
		if (globals->mp_refcount != NULL) free(globals->mp_refcount);
		if (globals->mp_mine != NULL) free(globals->mp_mine);
		globals->mp_count = 0;
	}
}

static void
token_table_release_no_lock(token_table_node_t *t)
{
	notify_globals_t globals = _notify_globals();

	if (t == NULL) return;

	if (t->refcount > 0) t->refcount--;
	if (t->refcount > 0) return;

	notify_release_file_descriptor(t->fd);
	notify_release_mach_port(t->mp, t->flags);

	if (t->block != NULL)
	{
		dispatch_async_f(t->queue, t->block, (dispatch_function_t)_Block_release);
	}

	t->block = NULL;

	if (t->queue != NULL) dispatch_release(t->queue);
	t->queue = NULL;

	_nc_table_delete_n(globals->token_table, t->token);
	name_table_release_no_lock(t->name);

	free(t->path);
	free(t);
}

static void
token_table_release(token_table_node_t *t)
{
	notify_globals_t globals = _notify_globals();

	pthread_mutex_lock(&globals->notify_lock);
	token_table_release_no_lock(t);
	pthread_mutex_unlock(&globals->notify_lock);
}

static notify_state_t *
_notify_lib_self_state()
{
	notify_globals_t globals = _notify_globals();

	dispatch_once(&globals->self_state_once, ^{
		globals->self_state = _notify_lib_notify_state_new(NOTIFY_STATE_USE_LOCKS, 0);
	});

	return globals->self_state;
}

/* SPI */
void
notify_set_options(uint32_t opts)
{
	notify_globals_t globals = _notify_globals();

	/* NOTIFY_OPT_DISABLE can be unset with NOTIFY_OPT_ENABLE */
	if (globals->client_opts & NOTIFY_OPT_DISABLE)
	{
		if ((opts & NOTIFY_OPT_ENABLE) == 0) return;

		/* re-enable by swapping in the saved server port and saved opts*/
		pthread_mutex_lock(&globals->notify_lock);

		globals->client_opts = globals->saved_opts;
		globals->notify_server_port = globals->saved_server_port;

		pthread_mutex_unlock(&globals->notify_lock);
		return;
	}

	/*
	 * A client can disable the library even if the server port has already been fetched.
	 * Note that this could race with another thread making a Libnotify call.
	 */
	if (opts & NOTIFY_OPT_DISABLE)
	{
		pthread_mutex_lock(&globals->notify_lock);

		globals->saved_opts = globals->client_opts;
		globals->client_opts = NOTIFY_OPT_DISABLE;

		globals->saved_server_port = globals->notify_server_port;
		globals->notify_server_port = MACH_PORT_NULL;

		pthread_mutex_unlock(&globals->notify_lock);
		return;
	}

	globals->client_opts = opts;

	/* call _notify_lib_init to create ports / dispatch sources as required */
	_notify_lib_init(EVENT_INIT);
}

/*
 * PUBLIC API
 */

/*
 * notify_post is a very simple API, but the implementation is
 * more complex to try to optimize the time it takes.
 *
 * The server - notifyd - keeps a unique ID number for each key
 * in the namespace.  Although it's reasonably fast to call
 * _notify_server_post_4 (a MIG simpleroutine), the MIG call
 * allocates VM and copies the name string.  It's much faster to
 * call using the ID number.  The problem is mapping from name to
 * ID number.  The token table keeps track of all registered names
 * (in the client), but the registration calls are simpleroutines,
 * except for notify_register_check.  notify_register_check saves
 * the name ID in the token table, but the other routines set it
 * to NID_UNSET.
 *
 * In notify_post, we check if the name is known.  If it is not,
 * then the client is doing a "cold call".  There may be no
 * clients for this name anywhere on the system.  In this case
 * we simply send the name.  We take the allocate/copy cost, but
 * the latency is still not too bad since we use a simpleroutine.
 *
 * If the name in registered and the ID number is known, we send
 * the ID using a simpleroutine.  This is very fast.
 *
 * If the name is registered but the ID number is NID_UNSET, we
 * send the name (as in a "cold call".  It *might* just be that
 * this client process just posts once, and we don't want to incur
 * any addition cost.  The ID number is reset to NID_CALLED_ONCE.
 *
 * If the client posts the same name again (the ID number is
 * NID_CALLED_ONCE, we do a synchronous call to notifyd, sending
 * the name string and getting back the name ID, whcih we save
 * in the token table.  This is simply a zero/one/many heuristic:
 * If the client posts the same name more than once, we make the
 * guess that it's going to do it more frequently, and it's worth
 * the time it takes to fetch the ID from notifyd.
 */
uint32_t
notify_post(const char *name)
{
	notify_state_t *ns_self;
	kern_return_t kstatus;
	uint32_t status;
	size_t namelen = 0;
	name_table_node_t *n;
	uint64_t nid = UINT64_MAX;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	if (!strncmp(name, SELF_PREFIX, SELF_PREFIX_LEN))
	{
		ns_self = _notify_lib_self_state();
		if (ns_self == NULL) return NOTIFY_STATUS_FAILED;
		_notify_lib_post(ns_self, name, 0, 0);
		return NOTIFY_STATUS_OK;
	}

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0) return NOTIFY_STATUS_FAILED;
	}

	if (globals->notify_ipc_version == 0)
	{
		namelen = strlen(name);
		kstatus = _notify_server_post(globals->notify_server_port, (caddr_t)name, namelen, (int32_t *)&status);
		if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
		return status;
	}

	namelen = strlen(name);

	/* Lock to prevent a race with notify cancel over the use of name IDs */
	pthread_mutex_lock(&globals->notify_lock);

	/* See if we have a name ID for this name. */
	n = name_table_find_retain_no_lock(name);
	if (n != NULL)
	{
		if (n->name_id == NID_UNSET)
		{
			/* First post goes using the name string. */
			kstatus = _notify_server_post_4(globals->notify_server_port, (caddr_t)name, namelen);
			if (kstatus != KERN_SUCCESS)
			{
				name_table_release_no_lock(name);
				pthread_mutex_unlock(&globals->notify_lock);
				return NOTIFY_STATUS_FAILED;
			}

			n->name_id = NID_CALLED_ONCE;
			name_table_release_no_lock(name);
			pthread_mutex_unlock(&globals->notify_lock);
			return NOTIFY_STATUS_OK;
		}
		else if (n->name_id == NID_CALLED_ONCE)
		{
			/* Post and fetch the name ID.  Slow, but subsequent posts will be very fast. */
			kstatus = _notify_server_post_2(globals->notify_server_port, (caddr_t)name, namelen, &nid, (int32_t *)&status);
			if (kstatus != KERN_SUCCESS)
			{
				name_table_release_no_lock(name);
				pthread_mutex_unlock(&globals->notify_lock);
				return NOTIFY_STATUS_FAILED;
			}

			if (status == NOTIFY_STATUS_OK) n->name_id = nid;
			name_table_release_no_lock(name);
			pthread_mutex_unlock(&globals->notify_lock);
			return status;
		}
		else
		{
			/* We have the name ID.  Do an async post using the name ID.  Very fast. */
			kstatus = _notify_server_post_3(globals->notify_server_port, n->name_id);
			name_table_release_no_lock(name);
			pthread_mutex_unlock(&globals->notify_lock);
			if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
			return NOTIFY_STATUS_OK;
		}
	}

	pthread_mutex_unlock(&globals->notify_lock);

	/* Do an async post using the name string. Fast (but not as fast as using name ID). */
	kstatus = _notify_server_post_4(globals->notify_server_port, (caddr_t)name, namelen);
	if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
	return NOTIFY_STATUS_OK;
}

uint32_t
notify_set_owner(const char *name, uint32_t uid, uint32_t gid)
{
	notify_state_t *ns_self;
	kern_return_t kstatus;
	uint32_t status;
	notify_globals_t globals = _notify_globals();

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	if (!strncmp(name, SELF_PREFIX, SELF_PREFIX_LEN))
	{
		ns_self = _notify_lib_self_state();
		if (ns_self == NULL) return NOTIFY_STATUS_FAILED;
		status = _notify_lib_set_owner(ns_self, name, uid, gid);
		return status;
	}

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0) return NOTIFY_STATUS_FAILED;
	}

	kstatus = _notify_server_set_owner(globals->notify_server_port, (caddr_t)name, strlen(name), uid, gid, (int32_t *)&status);
	if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
	return status;
}

uint32_t
notify_get_owner(const char *name, uint32_t *uid, uint32_t *gid)
{
	notify_state_t *ns_self;
	kern_return_t kstatus;
	uint32_t status;
	notify_globals_t globals = _notify_globals();

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	if (!strncmp(name, SELF_PREFIX, SELF_PREFIX_LEN))
	{
		ns_self = _notify_lib_self_state();
		if (ns_self == NULL) return NOTIFY_STATUS_FAILED;
		status = _notify_lib_get_owner(ns_self, name, uid, gid);
		return status;
	}

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0) return NOTIFY_STATUS_FAILED;
	}

	kstatus = _notify_server_get_owner(globals->notify_server_port, (caddr_t)name, strlen(name), (int32_t *)uid, (int32_t *)gid, (int32_t *)&status);
	if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
	return status;
}

uint32_t
notify_set_access(const char *name, uint32_t access)
{
	notify_state_t *ns_self;
	kern_return_t kstatus;
	uint32_t status;
	notify_globals_t globals = _notify_globals();

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	if (!strncmp(name, SELF_PREFIX, SELF_PREFIX_LEN))
	{
		ns_self = _notify_lib_self_state();
		if (ns_self == NULL) return NOTIFY_STATUS_FAILED;
		status = _notify_lib_set_access(ns_self, name, access);
		return status;
	}

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0) return NOTIFY_STATUS_FAILED;
	}

	kstatus = _notify_server_set_access(globals->notify_server_port, (caddr_t)name, strlen(name), access, (int32_t *)&status);
	if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
	return status;
}

uint32_t
notify_get_access(const char *name, uint32_t *access)
{
	notify_state_t *ns_self;
	kern_return_t kstatus;
	uint32_t status;
	notify_globals_t globals = _notify_globals();

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	if (!strncmp(name, SELF_PREFIX, SELF_PREFIX_LEN))
	{
		ns_self = _notify_lib_self_state();
		if (ns_self == NULL) return NOTIFY_STATUS_FAILED;
		status = _notify_lib_get_access(ns_self, name, access);
		return status;
	}

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0) return NOTIFY_STATUS_FAILED;
	}

	kstatus = _notify_server_get_access(globals->notify_server_port, (caddr_t)name, strlen(name), (int32_t *)access, (int32_t *)&status);
	if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
	return status;
}

/* notifyd retains and releases a name when clients register and cancel. */
uint32_t
notify_release_name(const char *name)
{
	return NOTIFY_STATUS_OK;
}

static void
_notify_dispatch_handle(mach_port_t port)
{
	token_table_node_t *t;
	int token;
	mach_msg_empty_rcv_t msg;
	kern_return_t status;

	if (port == MACH_PORT_NULL) return;

	memset(&msg, 0, sizeof(msg));

	status = mach_msg(&msg.header, MACH_RCV_MSG, 0, sizeof(msg), port, 0, MACH_PORT_NULL);
	if (status != KERN_SUCCESS) return;

	token = msg.header.msgh_id;

	t = token_table_find_retain(token);

	if (t != NULL)
	{
		if ((t->queue != NULL) && (t->block != NULL))
		{
			/*
			 * Don't reference into the token table node after token_table_release().
			 * If the block calls notify_cancel, the node can get trashed, so
			 * we keep anything we need from the block (properly retained and released)
			 * in local variables.  Concurrent notify_cancel() calls in the block are safe.
			 */
			notify_handler_t theblock = Block_copy(t->block);
			dispatch_queue_t thequeue = t->queue;
			dispatch_retain(thequeue);

			dispatch_async(thequeue, ^{
				token_table_node_t *t = token_table_find_no_lock(token);
				if (t != NULL) theblock(token);
			});

			_Block_release(theblock);
			dispatch_release(thequeue);
		}

		token_table_release(t);
	}
}

uint32_t
notify_register_dispatch(const char *name, int *out_token, dispatch_queue_t queue, notify_handler_t handler)
{
	__block uint32_t status;
	token_table_node_t *t;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	if (queue == NULL) return NOTIFY_STATUS_FAILED;
	if (handler == NULL) return NOTIFY_STATUS_FAILED;

	/* client is using dispatch: enable local demux and regeneration */
	notify_set_options(NOTIFY_OPT_DEMUX | NOTIFY_OPT_REGEN);

	status = notify_register_mach_port(name, &globals->notify_common_port, NOTIFY_REUSE, out_token);
	if (status != NOTIFY_STATUS_OK) return status;

	t = token_table_find_retain(*out_token);
	if (t == NULL) return NOTIFY_STATUS_FAILED;

	t->queue = queue;
	dispatch_retain(t->queue);
	t->block = Block_copy(handler);
	token_table_release(t);

	return NOTIFY_STATUS_OK;
}

/* note this does not get self names */
static uint32_t
notify_register_mux_fd(const char *name, int *out_token, int rfd, int wfd)
{
	__block uint32_t status;
	token_table_node_t *t;
	int val;
	notify_globals_t globals = _notify_globals();

	status = NOTIFY_STATUS_OK;

	if (globals->notify_common_port == MACH_PORT_NULL) return NOTIFY_STATUS_FAILED;

	status = notify_register_mach_port(name, &globals->notify_common_port, NOTIFY_REUSE, out_token);

	t = token_table_find_retain(*out_token);
	if (t == NULL) return NOTIFY_STATUS_FAILED;

	t->token = *out_token;
	t->fd = rfd;
	t->queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);
	dispatch_retain(t->queue);
	val = htonl(t->token);
	t->block = (notify_handler_t)Block_copy(^(int unused){ write(wfd, &val, sizeof(val)); });

	token_table_release(t);

	return NOTIFY_STATUS_OK;
}

uint32_t
notify_register_check(const char *name, int *out_token)
{
	notify_state_t *ns_self;
	kern_return_t kstatus;
	uint32_t status, token;
	uint64_t nid;
	int32_t slot, shmsize;
	size_t namelen;
	uint32_t cid;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;
	if (out_token == NULL) return NOTIFY_STATUS_FAILED;

	*out_token = -1;
	namelen = strlen(name);

	if (!strncmp(name, SELF_PREFIX, SELF_PREFIX_LEN))
	{
		ns_self = _notify_lib_self_state();
		if (ns_self == NULL) return NOTIFY_STATUS_FAILED;

		token = OSAtomicIncrement32((int32_t *)&globals->token_id);
		status = _notify_lib_register_plain(ns_self, name, NOTIFY_CLIENT_SELF, token, SLOT_NONE, 0, 0, &nid);
		if (status != NOTIFY_STATUS_OK) return status;

		cid = token;
		token_table_add(name, namelen, nid, token, cid, SLOT_NONE, NOTIFY_FLAG_SELF | NOTIFY_TYPE_PLAIN, SIGNAL_NONE, FD_NONE, MACH_PORT_NULL, 0);

		*out_token = token;
		return NOTIFY_STATUS_OK;
	}

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0) return NOTIFY_STATUS_FAILED;
	}

	token = OSAtomicIncrement32((int32_t *)&globals->token_id);
	kstatus = KERN_SUCCESS;

	if (globals->notify_ipc_version == 0)
	{
		nid = NID_UNSET;
		kstatus = _notify_server_register_check(globals->notify_server_port, (caddr_t)name, namelen, &shmsize, &slot, (int32_t *)&cid, (int32_t *)&status);
	}
	else
	{
		cid = token;
		kstatus = _notify_server_register_check_2(globals->notify_server_port, (caddr_t)name, namelen, token, &shmsize, &slot, &nid, (int32_t *)&status);
	}

	if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
	if (status != NOTIFY_STATUS_OK) return status;

	if (shmsize != -1)
	{
		if (globals->shm_base == NULL)
		{
			if (shm_attach(shmsize) != 0) return NOTIFY_STATUS_FAILED;
			if (globals->shm_base == NULL) return NOTIFY_STATUS_FAILED;
		}

		token_table_add(name, namelen, nid, token, cid, slot, NOTIFY_TYPE_MEMORY | NOTIFY_FLAG_REGEN, SIGNAL_NONE, FD_NONE, MACH_PORT_NULL, 0);
	}
	else
	{
		token_table_add(name, namelen, nid, token, cid, SLOT_NONE, NOTIFY_TYPE_PLAIN | NOTIFY_FLAG_REGEN, SIGNAL_NONE, FD_NONE, MACH_PORT_NULL, 0);
	}

	*out_token = token;
	return status;
}

uint32_t
notify_register_plain(const char *name, int *out_token)
{
	notify_state_t *ns_self;
	kern_return_t kstatus;
	uint32_t status;
	uint64_t nid;
	size_t namelen;
	int token;
	uint32_t cid;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	namelen = strlen(name);

	if (!strncmp(name, SELF_PREFIX, SELF_PREFIX_LEN))
	{
		ns_self = _notify_lib_self_state();
		if (ns_self == NULL) return NOTIFY_STATUS_FAILED;

		token = OSAtomicIncrement32((int32_t *)&globals->token_id);
		status = _notify_lib_register_plain(ns_self, name, NOTIFY_CLIENT_SELF, token, SLOT_NONE, 0, 0, &nid);
		if (status != NOTIFY_STATUS_OK) return status;

		cid = token;
		token_table_add(name, namelen, nid, token, cid, SLOT_NONE, NOTIFY_FLAG_SELF | NOTIFY_TYPE_PLAIN, SIGNAL_NONE, FD_NONE, MACH_PORT_NULL, 0);

		*out_token = token;
		return NOTIFY_STATUS_OK;
	}

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0) return NOTIFY_STATUS_FAILED;
	}

	token = OSAtomicIncrement32((int32_t *)&globals->token_id);

	if (globals->notify_ipc_version == 0)
	{
		kstatus = _notify_server_register_plain(globals->notify_server_port, (caddr_t)name, namelen, (int32_t *)&cid, (int32_t *)&status);
		if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
		if (status != NOTIFY_STATUS_OK) return status;
	}
	else
	{
		cid = token;
		kstatus = _notify_server_register_plain_2(globals->notify_server_port, (caddr_t)name, namelen, token);
		if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
	}

	token_table_add(name, namelen, NID_UNSET, token, cid, SLOT_NONE, NOTIFY_TYPE_PLAIN | NOTIFY_FLAG_REGEN, SIGNAL_NONE, FD_NONE, MACH_PORT_NULL, 0);

	*out_token = token;
	return NOTIFY_STATUS_OK;
}

uint32_t
notify_register_signal(const char *name, int sig, int *out_token)
{
	notify_state_t *ns_self;
	kern_return_t kstatus;
	uint32_t status;
	uint64_t nid;
	size_t namelen;
	int token;
	uint32_t cid;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;

	namelen = strlen(name);

	if (!strncmp(name, SELF_PREFIX, SELF_PREFIX_LEN))
	{
		ns_self = _notify_lib_self_state();
		if (ns_self == NULL) return NOTIFY_STATUS_FAILED;

		token = OSAtomicIncrement32((int32_t *)&globals->token_id);
		status = _notify_lib_register_signal(ns_self, name, NOTIFY_CLIENT_SELF, token, sig, 0, 0, &nid);
		if (status != NOTIFY_STATUS_OK) return status;

		cid = token;
		token_table_add(name, namelen, nid, token, cid, SLOT_NONE, NOTIFY_FLAG_SELF | NOTIFY_TYPE_SIGNAL, sig, FD_NONE, MACH_PORT_NULL, 0);

		*out_token = token;
		return NOTIFY_STATUS_OK;
	}

	if (globals->client_opts & NOTIFY_OPT_DEMUX)
	{
		return notify_register_dispatch(name, out_token, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^(int unused){ kill(getpid(), sig); });
	}

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0) return NOTIFY_STATUS_FAILED;
	}

	token = OSAtomicIncrement32((int32_t *)&globals->token_id);

	if (globals->notify_ipc_version == 0)
	{
		kstatus = _notify_server_register_signal(globals->notify_server_port, (caddr_t)name, namelen, sig, (int32_t *)&cid, (int32_t *)&status);
		if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
		if (status != NOTIFY_STATUS_OK) return status;
	}
	else
	{
		cid = token;
		kstatus = _notify_server_register_signal_2(globals->notify_server_port, (caddr_t)name, namelen, token, sig);
		if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
	}

	token_table_add(name, namelen, NID_UNSET, token, cid, SLOT_NONE, NOTIFY_TYPE_SIGNAL | NOTIFY_FLAG_REGEN, sig, FD_NONE, MACH_PORT_NULL, 0);

	*out_token = token;
	return NOTIFY_STATUS_OK;
}

uint32_t
notify_register_mach_port(const char *name, mach_port_name_t *notify_port, int flags, int *out_token)
{
	notify_state_t *ns_self;
	kern_return_t kstatus;
	uint32_t status;
	uint64_t nid;
	task_t task;
	int token, mine;
	size_t namelen;
	uint32_t cid, tflags;
	token_table_node_t *t;
	mach_port_name_t port;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;
	if (notify_port == NULL) return NOTIFY_STATUS_INVALID_PORT;

	mine = 0;
	namelen = strlen(name);

	task = mach_task_self();

	if ((flags & NOTIFY_REUSE) == 0)
	{
		mine = 1;
		kstatus = mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE, notify_port);
		if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
	}

	kstatus = mach_port_insert_right(task, *notify_port, *notify_port, MACH_MSG_TYPE_MAKE_SEND);
	if (kstatus != KERN_SUCCESS)
	{
		if (mine == 1) mach_port_mod_refs(task, *notify_port, MACH_PORT_RIGHT_RECEIVE, -1);
		return NOTIFY_STATUS_FAILED;
	}

	if (!strncmp(name, SELF_PREFIX, SELF_PREFIX_LEN))
	{
		ns_self = _notify_lib_self_state();
		if (ns_self == NULL)
		{
			if (mine == 1)
			{
				mach_port_mod_refs(task, *notify_port, MACH_PORT_RIGHT_RECEIVE, -1);
			}

			mach_port_deallocate(task, *notify_port);
			return NOTIFY_STATUS_FAILED;
		}

		token = OSAtomicIncrement32((int32_t *)&globals->token_id);
		status = _notify_lib_register_mach_port(ns_self, name, NOTIFY_CLIENT_SELF, token, *notify_port, 0, 0, &nid);
		if (status != NOTIFY_STATUS_OK)
		{
			if (mine == 1)
			{
				mach_port_mod_refs(task, *notify_port, MACH_PORT_RIGHT_RECEIVE, -1);
			}

			mach_port_deallocate(task, *notify_port);
			return status;
		}

		cid = token;
		token_table_add(name, namelen, nid, token, cid, SLOT_NONE, NOTIFY_FLAG_SELF | NOTIFY_TYPE_PORT, SIGNAL_NONE, FD_NONE, *notify_port, 0);

		*out_token = token;
		notify_retain_mach_port(*notify_port, mine);

		return NOTIFY_STATUS_OK;
	}

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0)
		{
			if (mine == 1)
			{
				mach_port_mod_refs(task, *notify_port, MACH_PORT_RIGHT_RECEIVE, -1);
			}

			mach_port_deallocate(task, *notify_port);
			return NOTIFY_STATUS_FAILED;
		}
	}

	if ((globals->client_opts & NOTIFY_OPT_DEMUX) && (*notify_port != globals->notify_common_port))
	{
		port = globals->notify_common_port;
		kstatus = mach_port_insert_right(task, globals->notify_common_port, globals->notify_common_port, MACH_MSG_TYPE_MAKE_SEND);
	}
	else
	{
		port = *notify_port;
		kstatus = KERN_SUCCESS;
	}

	if (kstatus == KERN_SUCCESS)
	{
		token = OSAtomicIncrement32((int32_t *)&globals->token_id);

		if (globals->notify_ipc_version == 0)
		{
			kstatus = _notify_server_register_mach_port(globals->notify_server_port, (caddr_t)name, namelen, port, token, (int32_t *)&cid, (int32_t *)&status);
			if ((kstatus == KERN_SUCCESS) && (status != NOTIFY_STATUS_OK)) kstatus = KERN_FAILURE;
		}
		else
		{
			cid = token;
			kstatus = _notify_server_register_mach_port_2(globals->notify_server_port, (caddr_t)name, namelen, token, port);
		}
	}

	if (kstatus != KERN_SUCCESS)
	{
		if (mine == 1)
		{
			mach_port_mod_refs(task, *notify_port, MACH_PORT_RIGHT_RECEIVE, -1);
		}

		mach_port_deallocate(task, *notify_port);
		return NOTIFY_STATUS_FAILED;
	}

	tflags = NOTIFY_TYPE_PORT;
	if (port == globals->notify_common_port) tflags |= NOTIFY_FLAG_REGEN;
	token_table_add(name, namelen, NID_UNSET, token, cid, SLOT_NONE, tflags, SIGNAL_NONE, FD_NONE, *notify_port, 0);

	if ((globals->client_opts & NOTIFY_OPT_DEMUX) && (*notify_port != globals->notify_common_port))
	{
		t = token_table_find_retain(token);
		if (t == NULL) return NOTIFY_STATUS_FAILED;

		/* remember to release the send right when this gets cancelled */
		t->flags |= NOTIFY_FLAG_RELEASE_SEND;

		port = *notify_port;
		t->queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);
		dispatch_retain(t->queue);
		t->block = (notify_handler_t)Block_copy(^(int unused){
			mach_msg_empty_send_t msg;
			kern_return_t kstatus;

			/* send empty message to the port with msgh_id = token; */
			memset(&msg, 0, sizeof(msg));
			msg.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSGH_BITS_ZERO);
			msg.header.msgh_remote_port = port;
			msg.header.msgh_local_port = MACH_PORT_NULL;
			msg.header.msgh_size = sizeof(mach_msg_empty_send_t);
			msg.header.msgh_id = token;

			kstatus = mach_msg(&(msg.header), MACH_SEND_MSG | MACH_SEND_TIMEOUT, msg.header.msgh_size, 0, MACH_PORT_NULL, 0, MACH_PORT_NULL);
		});

		token_table_release(t);
	}

	*out_token = token;
	notify_retain_mach_port(*notify_port, mine);

	return NOTIFY_STATUS_OK;
}

static char *
_notify_mk_tmp_path(int tid)
{
#if TARGET_OS_EMBEDDED
	int freetmp = 0;
	char *path, *tmp = getenv("TMPDIR");

	if (tmp == NULL)
	{
		asprintf(&tmp, "/tmp/com.apple.notify.%d", geteuid());
		mkdir(tmp, 0755);
		freetmp = 1;
	}

	if (tmp == NULL) return NULL;

	asprintf(&path, "%s/com.apple.notify.%d.%d", tmp, getpid(), tid);
	if (freetmp) free(tmp);
	return path;
#else
	char tmp[PATH_MAX], *path;

	if (confstr(_CS_DARWIN_USER_TEMP_DIR, tmp, sizeof(tmp)) <= 0) return NULL;
#endif

	path = NULL;
	asprintf(&path, "%s/com.apple.notify.%d.%d", tmp, getpid(), tid);
	return path;
}

uint32_t
notify_register_file_descriptor(const char *name, int *notify_fd, int flags, int *out_token)
{
	notify_state_t *ns_self;
	uint32_t i, status;
	uint64_t nid;
	int token, mine, fdpair[2];
	size_t namelen;
	fileport_t fileport;
	kern_return_t kstatus;
	uint32_t cid;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	mine = 0;

	if (name == NULL) return NOTIFY_STATUS_INVALID_NAME;
	if (notify_fd == NULL) return NOTIFY_STATUS_INVALID_FILE;

	namelen = strlen(name);

	if ((flags & NOTIFY_REUSE) == 0)
	{
		if (pipe(fdpair) < 0) return NOTIFY_STATUS_FAILED;

		mine = 1;
		*notify_fd = fdpair[0];
	}
	else
	{
		/* check the file descriptor - it must be one of "ours" */
		for (i = 0; i < globals->fd_count; i++)
		{
			if (globals->fd_clnt[i] == *notify_fd) break;
		}

		if (i >= globals->fd_count) return NOTIFY_STATUS_INVALID_FILE;

		fdpair[0] = globals->fd_clnt[i];
		fdpair[1] = globals->fd_srv[i];
	}

	if (!strncmp(name, SELF_PREFIX, SELF_PREFIX_LEN))
	{
		ns_self = _notify_lib_self_state();
		if (ns_self == NULL)
		{
			if (mine == 1)
			{
				close(fdpair[0]);
				close(fdpair[1]);
			}

			return NOTIFY_STATUS_FAILED;
		}

		token = OSAtomicIncrement32((int32_t *)&globals->token_id);
		status = _notify_lib_register_file_descriptor(ns_self, name, NOTIFY_CLIENT_SELF, token, fdpair[1], 0, 0, &nid);
		if (status != NOTIFY_STATUS_OK)
		{
			if (mine == 1)
			{
				close(fdpair[0]);
				close(fdpair[1]);
			}

			return status;
		}

		cid = token;
		token_table_add(name, namelen, nid, token, cid, SLOT_NONE, NOTIFY_FLAG_SELF | NOTIFY_TYPE_FILE, SIGNAL_NONE, *notify_fd, MACH_PORT_NULL, 0);

		*out_token = token;
		notify_retain_file_descriptor(fdpair[0], fdpair[1]);

		return NOTIFY_STATUS_OK;
	}

	if (globals->client_opts & NOTIFY_OPT_DEMUX)
	{
		/*
		 * Use dispatch to do a write() on fdpair[1] when notified.
		 */
		status = notify_register_mux_fd(name, out_token, fdpair[0], fdpair[1]);
		if (status != NOTIFY_STATUS_OK)
		{
			if (mine == 1)
			{
				close(fdpair[0]);
				close(fdpair[1]);
			}

			return status;
		}

		notify_retain_file_descriptor(fdpair[0], fdpair[1]);
		return NOTIFY_STATUS_OK;
	}

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0)
		{
			if (mine == 1)
			{
				close(fdpair[0]);
				close(fdpair[1]);
			}

			return NOTIFY_STATUS_FAILED;
		}
	}

	/* send fdpair[1] (the sender's fd) to notifyd using a fileport */
	fileport = MACH_PORT_NULL;
	if (fileport_makeport(fdpair[1], (fileport_t *)&fileport) < 0)
	{
		if (mine == 1)
		{
			close(fdpair[0]);
			close(fdpair[1]);
		}

		return NOTIFY_STATUS_FAILED;
	}

	token = OSAtomicIncrement32((int32_t *)&globals->token_id);

	if (globals->notify_ipc_version == 0)
	{
		kstatus = _notify_server_register_file_descriptor(globals->notify_server_port, (caddr_t)name, namelen, (mach_port_t)fileport, token, (int32_t *)&cid, (int32_t *)&status);
		if ((kstatus == KERN_SUCCESS) && (status != NOTIFY_STATUS_OK)) kstatus = KERN_FAILURE;
	}
	else
	{
		kstatus = _notify_server_register_file_descriptor_2(globals->notify_server_port, (caddr_t)name, namelen, token, (mach_port_t)fileport);
	}

	if (kstatus != KERN_SUCCESS)
	{
		if (mine == 1)
		{
			close(fdpair[0]);
			close(fdpair[1]);
		}

		return NOTIFY_STATUS_FAILED;
	}

	token_table_add(name, namelen, NID_UNSET, token, cid, SLOT_NONE, NOTIFY_TYPE_FILE, SIGNAL_NONE, *notify_fd, MACH_PORT_NULL, 0);

	*out_token = token;
	notify_retain_file_descriptor(fdpair[0], fdpair[1]);

	return NOTIFY_STATUS_OK;
}

uint32_t
notify_check(int token, int *check)
{
	kern_return_t kstatus;
	uint32_t status, val;
	token_table_node_t *t;
	uint32_t tid;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	pthread_mutex_lock(&globals->notify_lock);

	t = token_table_find_no_lock(token);
	if (t == NULL)
	{
		pthread_mutex_unlock(&globals->notify_lock);
		return NOTIFY_STATUS_INVALID_TOKEN;
	}

	if (t->flags & NOTIFY_FLAG_SELF)
	{
		/* _notify_lib_check returns NOTIFY_STATUS_FAILED if self_state is NULL */
		status = _notify_lib_check(globals->self_state, NOTIFY_CLIENT_SELF, token, check);
		pthread_mutex_unlock(&globals->notify_lock);
		return status;
	}

	if (t->flags & NOTIFY_TYPE_MEMORY)
	{
		if (globals->shm_base == NULL)
		{
			pthread_mutex_unlock(&globals->notify_lock);
			return NOTIFY_STATUS_FAILED;
		}

		*check = 0;
		val = globals->shm_base[t->slot];
		if (t->val != val)
		{
			*check = 1;
			t->val = val;
		}

		pthread_mutex_unlock(&globals->notify_lock);
		return NOTIFY_STATUS_OK;
	}

	tid = token;
	if (globals->notify_ipc_version == 0) tid = t->client_id;

	pthread_mutex_unlock(&globals->notify_lock);

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0) return NOTIFY_STATUS_FAILED;
	}

	kstatus = _notify_server_check(globals->notify_server_port, tid, check, (int32_t *)&status);

	if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
	return status;
}

uint32_t
notify_peek(int token, uint32_t *val)
{
	token_table_node_t *t;
	uint32_t status;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	t = token_table_find_retain(token);
	if (t == NULL) return NOTIFY_STATUS_INVALID_TOKEN;

	if (t->flags & NOTIFY_FLAG_SELF)
	{
		/* _notify_lib_peek returns NOTIFY_STATUS_FAILED if self_state is NULL */
		status = _notify_lib_peek(globals->self_state, NOTIFY_CLIENT_SELF, token, (int *)val);
		token_table_release(t);
		return status;
	}

	if (t->flags & NOTIFY_TYPE_MEMORY)
	{
		if (globals->shm_base == NULL)
		{
			token_table_release(t);
			return NOTIFY_STATUS_FAILED;
		}

		*val = globals->shm_base[t->slot];
		token_table_release(t);
		return NOTIFY_STATUS_OK;
	}

	token_table_release(t);
	return NOTIFY_STATUS_INVALID_REQUEST;
}

int *
notify_check_addr(int token)
{
	token_table_node_t *t;
	uint32_t slot;
	int *val;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	t = token_table_find_retain(token);
	if (t == NULL) return NULL;

	if (t->flags & NOTIFY_FLAG_SELF)
	{
		/* _notify_lib_check_addr returns NOTIFY_STATUS_FAILED if self_state is NULL */
		val = _notify_lib_check_addr(globals->self_state, NOTIFY_CLIENT_SELF, token);
		token_table_release(t);
		return val;
	}

	if (t->flags & NOTIFY_TYPE_MEMORY)
	{
		slot = t->slot;
		token_table_release(t);

		if (globals->shm_base == NULL) return NULL;
		return (int *)&(globals->shm_base[slot]);
	}

	token_table_release(t);
	return NULL;
}

uint32_t
notify_monitor_file(int token, char *path, int flags)
{
	kern_return_t kstatus;
	uint32_t status, len;
	token_table_node_t *t;
	char *dup;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	if (path == NULL) return NOTIFY_STATUS_INVALID_REQUEST;

	t = token_table_find_retain(token);
	if (t == NULL) return NOTIFY_STATUS_INVALID_TOKEN;

	if (t->flags & NOTIFY_FLAG_SELF)
	{
		token_table_release(t);
		return NOTIFY_STATUS_INVALID_REQUEST;
	}

	/* can only monitor one path with a token */
	if (t->path != NULL)
	{
		token_table_release(t);
		return NOTIFY_STATUS_INVALID_REQUEST;
	}

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0)
		{
			token_table_release(t);
			return NOTIFY_STATUS_FAILED;
		}
	}

	len = strlen(path);
	dup = strdup(path);
	if (dup == NULL) return NOTIFY_STATUS_FAILED;

	if (globals->notify_ipc_version == 0)
	{
		kstatus = _notify_server_monitor_file(globals->notify_server_port, t->client_id, path, len, flags, (int32_t *)&status);
		if ((kstatus == KERN_SUCCESS) && (status != NOTIFY_STATUS_OK)) kstatus = KERN_FAILURE;
	}
	else
	{
		kstatus = _notify_server_monitor_file_2(globals->notify_server_port, token, path, len, flags);
	}

	t->path = dup;
	t->path_flags = flags;

	token_table_release(t);
	if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
	return NOTIFY_STATUS_OK;
}

uint32_t
notify_get_event(int token, int *ev, char *buf, int *len)
{
	if (ev != NULL) *ev = 0;
	if (len != NULL) *len = 0;

	return NOTIFY_STATUS_OK;
}

uint32_t
notify_get_state(int token, uint64_t *state)
{
	kern_return_t kstatus;
	uint32_t status;
	token_table_node_t *t;
	uint64_t nid;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	t = token_table_find_retain(token);
	if (t == NULL) return NOTIFY_STATUS_INVALID_TOKEN;
	if (t->name_node == NULL)
	{
		token_table_release(t);
		return NOTIFY_STATUS_INVALID_TOKEN;
	}

	if (t->flags & NOTIFY_FLAG_SELF)
	{
		/* _notify_lib_get_state returns NOTIFY_STATUS_FAILED if self_state is NULL */
		status = _notify_lib_get_state(globals->self_state, t->name_node->name_id, state, 0, 0);
		token_table_release(t);
		return status;
	}

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0)
		{
			token_table_release(t);
			return NOTIFY_STATUS_FAILED;
		}
	}

	if (globals->notify_ipc_version == 0)
	{
		kstatus = _notify_server_get_state(globals->notify_server_port, t->client_id, state, (int32_t *)&status);
		if ((kstatus == KERN_SUCCESS) && (status != NOTIFY_STATUS_OK)) kstatus = KERN_FAILURE;
	}
	else
	{
		if (t->name_node->name_id >= NID_CALLED_ONCE)
		{
			kstatus = _notify_server_get_state_3(globals->notify_server_port, t->token, state, (uint64_t *)&nid, (int32_t *)&status);
			if ((kstatus == KERN_SUCCESS) && (status == NOTIFY_STATUS_OK)) name_table_set_nid(t->name, nid);
		}
		else
		{
			kstatus = _notify_server_get_state_2(globals->notify_server_port, t->name_node->name_id, state, (int32_t *)&status);
		}
	}

	token_table_release(t);
	if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
	return status;
}

uint32_t
notify_set_state(int token, uint64_t state)
{
	kern_return_t kstatus;
	uint32_t status;
	token_table_node_t *t;
	uint64_t nid;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	t = token_table_find_retain(token);
	if (t == NULL) return NOTIFY_STATUS_INVALID_TOKEN;
	if (t->name_node == NULL)
	{
		token_table_release(t);
		return NOTIFY_STATUS_INVALID_TOKEN;
	}

	if (t->flags & NOTIFY_FLAG_SELF)
	{
		/* _notify_lib_set_state returns NOTIFY_STATUS_FAILED if self_state is NULL */
		status = _notify_lib_set_state(globals->self_state, t->name_node->name_id, state, 0, 0);
		token_table_release(t);
		return status;
	}

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0)
		{
			token_table_release(t);
			return NOTIFY_STATUS_FAILED;
		}
	}

	status = NOTIFY_STATUS_OK;

	if (globals->notify_ipc_version == 0)
	{
		kstatus = _notify_server_set_state(globals->notify_server_port, t->client_id, state, (int32_t *)&status);
		if ((kstatus == KERN_SUCCESS) && (status != NOTIFY_STATUS_OK)) kstatus = KERN_FAILURE;
	}
	else
	{
		if (t->name_node->name_id >= NID_CALLED_ONCE)
		{
			kstatus = _notify_server_set_state_3(globals->notify_server_port, t->token, state, (uint64_t *)&nid, (int32_t *)&status);
			if ((kstatus == KERN_SUCCESS) && (status == NOTIFY_STATUS_OK)) name_table_set_nid(t->name, nid);
		}
		else
		{
			status = NOTIFY_STATUS_OK;
			kstatus = _notify_server_set_state_2(globals->notify_server_port, t->name_node->name_id, state);
		}
	}

	if ((kstatus == KERN_SUCCESS) && (status == NOTIFY_STATUS_OK))
	{
		t->set_state_time = mach_absolute_time();
		t->set_state_val = state;
	}

	token_table_release(t);
	if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;
	return NOTIFY_STATUS_OK;
}

uint32_t
notify_cancel(int token)
{
	token_table_node_t *t;
	uint32_t status;
	kern_return_t kstatus;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	/*
	 * Lock to prevent a race with notify_post, which uses the name ID.
	 * If we are cancelling the last registration for this name, then we need
	 * to block those routines from getting the name ID from the name table.
	 * Once notifyd gets the cancellation, the name may vanish, and the name ID
	 * held in the name table would go stale.
	 *
	 * Uses token_table_find_no_lock() which does not retain, and
	 * token_table_release_no_lock() which releases the token.
	 */
	pthread_mutex_lock(&globals->notify_lock);

	t = token_table_find_no_lock(token);
	if (t == NULL)
	{
		pthread_mutex_unlock(&globals->notify_lock);
		return NOTIFY_STATUS_INVALID_TOKEN;
	}

	if (t->flags & NOTIFY_FLAG_SELF)
	{
		/*
		 * _notify_lib_cancel returns NOTIFY_STATUS_FAILED if self_state is NULL
		 * We let it fail quietly.
		 */
		_notify_lib_cancel(globals->self_state, NOTIFY_CLIENT_SELF, t->token);

		token_table_release_no_lock(t);
		pthread_mutex_unlock(&globals->notify_lock);
		return NOTIFY_STATUS_OK;
	}

	if (globals->notify_ipc_version == 0)
	{
		kstatus = _notify_server_cancel(globals->notify_server_port, t->client_id, (int32_t *)&status);
		if ((kstatus == KERN_SUCCESS) && (status != NOTIFY_STATUS_OK)) kstatus = KERN_FAILURE;
	}
	else
	{
		kstatus = _notify_server_cancel_2(globals->notify_server_port, token);
	}

	token_table_release_no_lock(t);
	pthread_mutex_unlock(&globals->notify_lock);

	if ((kstatus == MIG_SERVER_DIED) || (kstatus == MACH_SEND_INVALID_DEST)) return NOTIFY_STATUS_OK;
	else if (kstatus != KERN_SUCCESS) return NOTIFY_STATUS_FAILED;

	return NOTIFY_STATUS_OK;
}

bool
notify_is_valid_token(int val)
{
	token_table_node_t *t;
	bool valid = false;

	if (val < 0) return false;

	notify_globals_t globals = _notify_globals();

	pthread_mutex_lock(&globals->notify_lock);

	t = (token_table_node_t *)_nc_table_find_n(globals->token_table, val);
	if (t != NULL) valid = true;

	pthread_mutex_unlock(&globals->notify_lock);

	return valid;
}

uint32_t
notify_suspend(int token)
{
	token_table_node_t *t;
	uint32_t status, tid;
	kern_return_t kstatus;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	t = token_table_find_retain(token);
	if (t == NULL) return NOTIFY_STATUS_INVALID_TOKEN;

	if (t->flags & NOTIFY_FLAG_SELF)
	{
		_notify_lib_suspend(globals->self_state, NOTIFY_CLIENT_SELF, t->token);
		token_table_release(t);
		return NOTIFY_STATUS_OK;
	}

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0)
		{
			token_table_release(t);
			return NOTIFY_STATUS_FAILED;
		}
	}

	tid = token;
	if (globals->notify_ipc_version == 0) tid = t->client_id;

	kstatus = _notify_server_suspend(globals->notify_server_port, tid, (int32_t *)&status);

	token_table_release(t);
	if (kstatus != KERN_SUCCESS) status = NOTIFY_STATUS_FAILED;
	return status;
}

uint32_t
notify_resume(int token)
{
	token_table_node_t *t;
	uint32_t status, tid;
	kern_return_t kstatus;
	notify_globals_t globals = _notify_globals();

	regenerate_check();

	t = token_table_find_retain(token);
	if (t == NULL) return NOTIFY_STATUS_INVALID_TOKEN;

	if (t->flags & NOTIFY_FLAG_SELF)
	{
		_notify_lib_resume(globals->self_state, NOTIFY_CLIENT_SELF, t->token);
		token_table_release(t);
		return NOTIFY_STATUS_OK;
	}

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0)
		{
			token_table_release(t);
			return NOTIFY_STATUS_FAILED;
		}
	}

	tid = token;
	if (globals->notify_ipc_version == 0) tid = t->client_id;

	kstatus = _notify_server_resume(globals->notify_server_port, tid, (int32_t *)&status);

	token_table_release(t);
	if (kstatus != KERN_SUCCESS) status = NOTIFY_STATUS_FAILED;
	return status;
}

uint32_t
notify_suspend_pid(pid_t pid)
{
	uint32_t status;
	kern_return_t kstatus;
	notify_globals_t globals = _notify_globals();

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0)
		{
			return NOTIFY_STATUS_FAILED;
		}
	}

	kstatus = _notify_server_suspend_pid(globals->notify_server_port, pid, (int32_t *)&status);

	if (kstatus != KERN_SUCCESS) status = NOTIFY_STATUS_FAILED;
	return status;
}

uint32_t
notify_resume_pid(pid_t pid)
{
	uint32_t status;
	kern_return_t kstatus;
	notify_globals_t globals = _notify_globals();

	if (globals->notify_server_port == MACH_PORT_NULL)
	{
		status = _notify_lib_init(EVENT_INIT);
		if (status != 0)
		{
			return NOTIFY_STATUS_FAILED;
		}
	}

	kstatus = _notify_server_resume_pid(globals->notify_server_port, pid, (int32_t *)&status);

	if (kstatus != KERN_SUCCESS) status = NOTIFY_STATUS_FAILED;
	return status;
}

/* Deprecated SPI */
uint32_t
notify_simple_post(const char *name)
{
	return notify_post(name);
}

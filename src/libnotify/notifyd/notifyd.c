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
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/syslimits.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <asl.h>
#include <assert.h>
#include <inttypes.h>
#include <CrashReporterClient.h>
#include <TargetConditionals.h>
#include "pathwatch.h"
#include "notifyd.h"
#include "service.h"
#include "pathwatch.h"
#include "timer.h"

#include "notify_ipc.h"
#include "notify_private.h"

#define forever for(;;)
#define IndexNull -1

/* Compile flags */
#define RUN_TIME_CHECKS

#if TARGET_IPHONE_SIMULATOR
static const char *_config_file_path;
#define CONFIG_FILE_PATH _config_file_path

static const char *_debug_log_path;
#define DEBUG_LOG_PATH _debug_log_path
#else
#define CONFIG_FILE_PATH "/etc/notify.conf"
#define DEBUG_LOG_PATH "/var/log/notifyd.log"
#endif

#define STATUS_REQUEST_SHORT 0
#define STATUS_REQUEST_LONG 1

#define N_NOTIFY_TYPES 6

static int notifyd_token;

static char *status_file = NULL;

typedef union
{
	mach_msg_header_t head;
	union __RequestUnion__notify_ipc_subsystem request;
} notify_request_msg;

typedef union
{
	mach_msg_header_t head;
	union __ReplyUnion__notify_ipc_subsystem reply;
} notify_reply_msg;

extern boolean_t notify_ipc_server(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);

static const char *
notify_type_name(uint32_t t)
{
	switch (t)
	{
		case NOTIFY_TYPE_NONE:   return "none  ";
		case NOTIFY_TYPE_MEMORY: return "memory";
		case NOTIFY_TYPE_PLAIN:  return "plain ";
		case NOTIFY_TYPE_PORT:   return "port  ";
		case NOTIFY_TYPE_FILE:   return "file  ";
		case NOTIFY_TYPE_SIGNAL: return "signal";
		default: return "unknown";
	}

	return "unknown";
}

static void
fprint_client(FILE *f, client_t *c)
{
	int token;

	if (c == NULL) 
	{
		fprintf(f, "NULL client\n");
		return;
	}

	token = c->client_id;

	fprintf(f, "client_id: %llu\n", c->client_id);
	fprintf(f, "pid: %d\n", c->pid);
	fprintf(f, "token: %d\n", token);
	fprintf(f, "lastval: %u\n", c->lastval);
	fprintf(f, "suspend_count: %u\n", c->suspend_count);
	fprintf(f, "type: %s\n", notify_type_name(c->notify_type));
	switch(c->notify_type)
	{
		case NOTIFY_TYPE_NONE:
			break;

		case NOTIFY_TYPE_PLAIN:
			break;

		case NOTIFY_TYPE_MEMORY:
			break;

		case NOTIFY_TYPE_PORT:
			fprintf(f, "mach port: 0x%08x\n", c->port);
			break;

		case NOTIFY_TYPE_FILE:
			fprintf(f, "fd: %d\n", c->fd);
			break;

		case NOTIFY_TYPE_SIGNAL:
			fprintf(f, "signal: %d\n", c->sig);
			break;

		default: break;
	}
}

static void
fprint_quick_client(FILE *f, client_t *c, int pname)
{
	int token;
	if (c == NULL) return;

	token = c->client_id;

	if (pname == 1) fprintf(f, " [%s]", c->name_info->name);
	fprintf(f, " %u %u", c->pid, token);
	if (c->suspend_count > 0) fprintf(f, " suspend %d", c->suspend_count);
	if (c->state & NOTIFY_CLIENT_STATE_PENDING) fprintf(f, " pending");
	if (c->state & NOTIFY_CLIENT_STATE_TIMEOUT) fprintf(f, " timeout");
	fprintf(f, " %s", notify_type_name(c->notify_type));
	if (c->notify_type == NOTIFY_TYPE_SIGNAL) fprintf(f, " %d", c->sig);
	fprintf(f, "\n");
}

static void
fprint_quick_name_info(FILE *f, name_info_t *n)
{
	list_t *sl;
	client_t *c;

	if (n == NULL) return;

	fprintf(f, "\"%s\" uid=%u gid=%u %03x", n->name, n->uid, n->gid, n->access);
	if (n->slot != -1) 
	{
		fprintf(f, " slot %u", n->slot);
		if (global.shared_memory_refcount[n->slot] != -1) fprintf(f, " = %u", global.shared_memory_base[n->slot]);
	}

	fprintf(f, "\n");

	for (sl = n->subscriptions; sl != NULL; sl = _nc_list_next(sl))
	{
		c = _nc_list_data(sl);
		if (c == NULL) break;

		fprint_quick_client(f, c, 0);
	}

	fprintf(f, "\n");
}

static void
fprint_name_info(FILE *f, const char *name, name_info_t *n, table_t *pid_table, pid_t *max_pid)
{
	list_t *sl;
	client_t *c;
	uint32_t i, reg[N_NOTIFY_TYPES];

	if (n == NULL)
	{
		fprintf(f, "%s unknown\n", name);
		return;
	}

	fprintf(f, "name: %s\n", n->name);
	fprintf(f, "id: %llu\n", n->name_id);
	fprintf(f, "uid: %u\n", n->uid);
	fprintf(f, "gid: %u\n", n->gid);
	fprintf(f, "access: %03x\n", n->access);
	fprintf(f, "refcount: %u\n", n->refcount);
	if (n->slot == -1) fprintf(f, "slot: -unassigned-");
	else
	{
		fprintf(f, "slot: %u", n->slot);
		if (global.shared_memory_refcount[n->slot] != -1)
			fprintf(f, " = %u (%u)", global.shared_memory_base[n->slot], global.shared_memory_refcount[n->slot]);
	}
	fprintf(f, "\n");
	fprintf(f, "val: %u\n", n->val);
	fprintf(f, "state: %llu\n", n->state);

	for (i = 0; i < N_NOTIFY_TYPES; i++) reg[i] = 0;

	for (sl = n->subscriptions; sl != NULL; sl = _nc_list_next(sl))
	{
		list_t *l;

		c = _nc_list_data(sl);
		if (c == NULL) break;

		if ((c->pid != (pid_t)-1) && (c->pid > *max_pid)) *max_pid = c->pid;

		l = _nc_table_find_n(pid_table, c->pid);
		if (l == NULL)
		{
			_nc_table_insert_n(pid_table, (uint32_t)c->pid, _nc_list_new(c));
		}
		else
		{
			_nc_list_concat(l, _nc_list_new(c));
		}

		switch (c->notify_type)
		{
			case NOTIFY_TYPE_MEMORY: reg[1]++; break;
			case NOTIFY_TYPE_PLAIN:  reg[2]++; break;
			case NOTIFY_TYPE_PORT:   reg[3]++; break;
			case NOTIFY_TYPE_FILE:   reg[4]++; break;
			case NOTIFY_TYPE_SIGNAL: reg[5]++; break;
			default: reg[0]++;
		}
	}

	fprintf(f, "types: none %u   memory %u   plain %u   port %u   file %u   signal %u\n", reg[0], reg[1], reg[2], reg[3], reg[4], reg[5]);

	for (sl = n->subscriptions; sl != NULL; sl = _nc_list_next(sl))
	{
		c = _nc_list_data(sl);
		if (c == NULL) break;

		fprintf(f, "\n");
		fprint_client(f, c);
	}
}

static void
fprint_quick_status(FILE *f)
{
	void *tt;
	name_info_t *n;

	tt = _nc_table_traverse_start(global.notify_state->name_table);

	while (tt != NULL)
	{
		n = _nc_table_traverse(global.notify_state->name_table, tt);
		if (n == NULL) break;
		fprint_quick_name_info(f, n);
	}

	_nc_table_traverse_end(global.notify_state->name_table, tt);
	fprintf(f, "\n");
}

static void
fprint_status(FILE *f)
{
	void *tt;
	name_info_t *n;
	int32_t i;
	client_t *c;
	svc_info_t *info;
	path_node_t *node;
	timer_t *timer;
	table_t *pid_table;
	pid_t pid, max_pid;
	uint32_t count;
	portproc_data_t *pdata;

	pid_table = _nc_table_new(0);
	max_pid = 0;

	fprintf(f, "--- GLOBALS ---\n");
	fprintf(f, "%u slots (current id %u)\n", global.nslots, global.slot_id);
	fprintf(f, "%u log_cutoff (default %u)\n", global.log_cutoff, global.log_default);
	fprintf(f, "\n");

	fprintf(f, "--- STATISTICS ---\n");
	fprintf(f, "post         %llu\n", call_statistics.post);
	fprintf(f, "    id       %llu\n", call_statistics.post_by_id);
	fprintf(f, "    name     %llu\n", call_statistics.post_by_name);
	fprintf(f, "    fetch    %llu\n", call_statistics.post_by_name_and_fetch_id);
	fprintf(f, "    no_op    %llu\n", call_statistics.post_no_op);
	fprintf(f, "\n");
	fprintf(f, "register     %llu\n", call_statistics.reg);
	fprintf(f, "    plain    %llu\n", call_statistics.reg_plain);
	fprintf(f, "    check    %llu\n", call_statistics.reg_check);
	fprintf(f, "    signal   %llu\n", call_statistics.reg_signal);
	fprintf(f, "    file     %llu\n", call_statistics.reg_file);
	fprintf(f, "    port     %llu\n", call_statistics.reg_port);
	fprintf(f, "\n");
	fprintf(f, "check        %llu\n", call_statistics.check);
	fprintf(f, "cancel       %llu\n", call_statistics.cancel);
	fprintf(f, "cleanup      %llu\n", call_statistics.cleanup);
	fprintf(f, "regenerate   %llu\n", call_statistics.regenerate);
	fprintf(f, "\n");
	fprintf(f, "suspend      %llu\n", call_statistics.suspend);
	fprintf(f, "resume       %llu\n", call_statistics.resume);
	fprintf(f, "suspend_pid  %llu\n", call_statistics.suspend_pid);
	fprintf(f, "resume_pid   %llu\n", call_statistics.resume_pid);
	fprintf(f, "\n");
	fprintf(f, "get_state    %llu\n", call_statistics.get_state);
	fprintf(f, "    id       %llu\n", call_statistics.get_state_by_id);
	fprintf(f, "    client   %llu\n", call_statistics.get_state_by_client);
	fprintf(f, "    fetch    %llu\n", call_statistics.get_state_by_client_and_fetch_id);
	fprintf(f, "\n");
	fprintf(f, "set_state    %llu\n", call_statistics.set_state);
	fprintf(f, "    id       %llu\n", call_statistics.set_state_by_id);
	fprintf(f, "    client   %llu\n", call_statistics.set_state_by_client);
	fprintf(f, "    fetch    %llu\n", call_statistics.set_state_by_client_and_fetch_id);
	fprintf(f, "\n");
	fprintf(f, "get_owner    %llu\n", call_statistics.get_owner);
	fprintf(f, "set_owner    %llu\n", call_statistics.set_owner);
	fprintf(f, "\n");
	fprintf(f, "get_access   %llu\n", call_statistics.get_access);
	fprintf(f, "set_access   %llu\n", call_statistics.set_access);
	fprintf(f, "\n");
	fprintf(f, "monitor      %llu\n", call_statistics.monitor_file);
	fprintf(f, "svc_path     %llu\n", call_statistics.service_path);
	fprintf(f, "svc_timer    %llu\n", call_statistics.service_timer);

	fprintf(f, "\n");
	fprintf(f, "name         alloc %9u   free %9u   extant %9u\n", global.notify_state->stat_name_alloc , global.notify_state->stat_name_free, global.notify_state->stat_name_alloc - global.notify_state->stat_name_free);
	fprintf(f, "subscription alloc %9u   free %9u   extant %9u\n", global.notify_state->stat_client_alloc , global.notify_state->stat_client_free, global.notify_state->stat_client_alloc - global.notify_state->stat_client_free);
	fprintf(f, "portproc     alloc %9u   free %9u   extant %9u\n", global.notify_state->stat_portproc_alloc , global.notify_state->stat_portproc_free, global.notify_state->stat_portproc_alloc - global.notify_state->stat_portproc_free);
	fprintf(f, "\n");

	count = 0;
	tt = _nc_table_traverse_start(global.notify_state->port_table);
	while (tt != NULL)
	{
		pdata = _nc_table_traverse(global.notify_state->port_table, tt);
		if (pdata == NULL) break;
		count++;
	}
	_nc_table_traverse_end(global.notify_state->port_table, tt);
	fprintf(f, "port count   %u\n", count);

	count = 0;
	tt = _nc_table_traverse_start(global.notify_state->proc_table);
	while (tt != NULL)
	{
		pdata = _nc_table_traverse(global.notify_state->proc_table, tt);
		if (pdata == NULL) break;
		count++;
	}
	_nc_table_traverse_end(global.notify_state->proc_table, tt);
	fprintf(f, "proc count   %u\n", count);
	fprintf(f, "\n");

	fprintf(f, "--- NAME TABLE ---\n");
	count = 0;
	tt = _nc_table_traverse_start(global.notify_state->name_table);

	while (tt != NULL)
	{
		n = _nc_table_traverse(global.notify_state->name_table, tt);
		if (n == NULL) break;
		fprint_name_info(f, n->name, n, pid_table, &max_pid);
		fprintf(f, "\n");
		count++;
	}

	fprintf(f, "--- NAME COUNT %u ---\n", count);
	_nc_table_traverse_end(global.notify_state->name_table, tt);
	fprintf(f, "\n");

	fprintf(f, "--- SUBSCRIPTION TABLE ---\n");
	count = 0;
	tt = _nc_table_traverse_start(global.notify_state->client_table);

	while (tt != NULL)
	{
		c = _nc_table_traverse(global.notify_state->client_table, tt);
		if (c == NULL) break;
		fprint_quick_client(f, c, 1);
		count++;
	}

	fprintf(f, "--- SUBSCRIPTION COUNT %u ---\n", count);
	_nc_table_traverse_end(global.notify_state->client_table, tt);
	fprintf(f, "\n");

	fprintf(f, "--- CONTROLLED NAME ---\n");
	for (i = 0; i < global.notify_state->controlled_name_count; i++)
	{
		fprintf(f, "%s %u %u %03x\n", global.notify_state->controlled_name[i]->name, global.notify_state->controlled_name[i]->uid, global.notify_state->controlled_name[i]->gid, global.notify_state->controlled_name[i]->access);
	}
	fprintf(f, "--- CONTROLLED NAME COUNT %u ---\n", global.notify_state->controlled_name_count);
	fprintf(f, "\n");

	fprintf(f, "--- PUBLIC SERVICE ---\n");
	count = 0;
	tt = _nc_table_traverse_start(global.notify_state->name_table);
	while (tt != NULL)
	{
		n = _nc_table_traverse(global.notify_state->name_table, tt);
		if (n == NULL) break;
		if (n->private == NULL) continue;

		count++;
		info = (svc_info_t *)n->private;

		if (info->type == 0)
		{
			fprintf(f, "Null service: %s\n", n->name);
		}
		if (info->type == SERVICE_TYPE_PATH_PUBLIC)
		{
			node = (path_node_t *)info->private;
			fprintf(f, "Path Service: %s <- %s\n", n->name, node->path);
		}
		else if (info->type == SERVICE_TYPE_TIMER_PUBLIC)
		{
			timer = (timer_t *)info->private;
			switch (timer->type)
			{
				case TIME_EVENT_ONESHOT:
				{
					fprintf(f, "Time Service: %s <- Oneshot %llu\n", n->name, timer->start);
					break;
				}
				case TIME_EVENT_CLOCK:
				{
					fprintf(f, "Time Service: %s <- Clock start %lld freq %u end %lld\n", n->name, timer->start, timer->freq, timer->end);
					break;
				}
				case TIME_EVENT_CAL:
				{
					fprintf(f, "Time Service: %s <- Calendar start %lld freq %u end %lld day %d\n", n->name, timer->start, timer->freq, timer->end, timer->day);
					break;
				}
			}
		}
		else
		{
			fprintf(f, "Unknown service: %s (%u)\n", n->name, info->type);
		}
	}

	fprintf(f, "--- PUBLIC SERVICE COUNT %u ---\n", count);
	_nc_table_traverse_end(global.notify_state->name_table, tt);
	fprintf(f, "\n");

	fprintf(f, "--- PRIVATE SERVICE ---\n");
	count = 0;
	tt = _nc_table_traverse_start(global.notify_state->client_table);
	while (tt != NULL)
	{
		c = _nc_table_traverse(global.notify_state->client_table, tt);
		if (c == NULL) break;
		if (c->private == NULL) continue;

		count++;
		info = (svc_info_t *)c->private;
		n = c->name_info;

		if (info->type == 0)
		{
			fprintf(f, "PID %u Null service: %s\n", c->pid, n->name);
		}
		if (info->type == SERVICE_TYPE_PATH_PRIVATE)
		{
			node = (path_node_t *)info->private;
			fprintf(f, "PID %u Path Service: %s <- %s (UID %d GID %d)\n", c->pid, n->name, node->path, node->uid, node->gid);
		}
		else if (info->type == SERVICE_TYPE_TIMER_PRIVATE)
		{
			timer = (timer_t *)info->private;
			switch (timer->type)
			{
				case TIME_EVENT_ONESHOT:
				{
					fprintf(f, "PID %u Time Service: %s <- Oneshot %"PRId64"\n", c->pid, n->name, timer->start);
					break;
				}
				case TIME_EVENT_CLOCK:
				{
					fprintf(f, "PID %u Time Service: %s <- Clock start %"PRId64" freq %"PRIu32" end %"PRId64"\n", c->pid, n->name, timer->start, timer->freq, timer->end);
					break;
				}
				case TIME_EVENT_CAL:
				{
					fprintf(f, "PID %u Time Service: %s <- Calendar start %"PRId64" freq %"PRIu32" end %"PRId64" day %"PRId32"\n", c->pid, n->name, timer->start, timer->freq, timer->end, timer->day);
					break;
				}
			}
		}
	}

	fprintf(f, "--- PRIVATE SERVICE COUNT %u ---\n", count);
	_nc_table_traverse_end(global.notify_state->client_table, tt);
	fprintf(f, "\n");

	fprintf(f, "--- PROCESSES ---\n");
	for (pid = 0; pid <= max_pid; pid++)
	{
		int mem_count, plain_count, file_count, port_count, sig_count, com_port_count;
		mach_port_t common_port = MACH_PORT_NULL;

		list_t *x;
		list_t *l = _nc_table_find_n(pid_table, pid);
		if (l == NULL) continue;

		mem_count = 0;
		plain_count = 0;
		file_count = 0;
		port_count = 0;
		sig_count = 0;
		com_port_count = 0;

		for (x = l; x != NULL; x = _nc_list_next(x))
		{
			c = _nc_list_data(x);
			if (c != NULL)
			{
				if ((c->notify_type == NOTIFY_TYPE_PORT) && (!strcmp(c->name_info->name, COMMON_PORT_KEY))) common_port = c->port;
			}
		}

		for (x = l; x != NULL; x = _nc_list_next(x))
		{
			c = _nc_list_data(x);
			if (c != NULL)
			{
				switch(c->notify_type)
				{
					case NOTIFY_TYPE_NONE:
						break;

					case NOTIFY_TYPE_PLAIN:
						plain_count++;
						break;

					case NOTIFY_TYPE_MEMORY:
						mem_count++;
						break;

					case NOTIFY_TYPE_PORT:
						port_count++;
						if (c->port == common_port) com_port_count++;
						break;

					case NOTIFY_TYPE_FILE:
						file_count++;
						break;

					case NOTIFY_TYPE_SIGNAL:
						sig_count++;
						break;

					default: break;
				}
			}
		}

		fprintf(f, "pid: %u   ", pid);
		if (file_count + sig_count == 0)
		{
			if (port_count == 0) fprintf(f, "regenerable / polling\n");
			else if (port_count == com_port_count) fprintf(f, "regenerable\n");
			else if (com_port_count > 0) fprintf(f, "partially regenerable\n");
			else fprintf(f, "non-regenerable\n");
		}
		else
		{
			if (com_port_count == 0) fprintf(f, "non-regenerable\n");
			else fprintf(f, "partially regenerable\n");
		}

		fprintf(f, "memory %u   plain %u   port %u   file %u   signal %u   common port %u\n", mem_count, plain_count, port_count, file_count, sig_count, com_port_count);
		for (x = l; x != NULL; x = _nc_list_next(x))
		{
			c = _nc_list_data(x);
			if (c != NULL)
			{
				fprintf(f, "  %s: %s\n", notify_type_name(c->notify_type), c->name_info->name);
			}
		}

		fprintf(f, "\n");
		_nc_list_release_list(l);
	}
	fprintf(f, "\n");
	_nc_table_free(pid_table);
}

void
dump_status(uint32_t level)
{
	FILE *f;

	if (status_file == NULL)
	{
		asprintf(&status_file, "/var/run/notifyd_%u.status", getpid());
		if (status_file == NULL) return;
	}

	unlink(status_file);
	f = fopen(status_file, "w");
	if (f == NULL) return;

	if (level == STATUS_REQUEST_SHORT) fprint_quick_status(f);
	else if (level == STATUS_REQUEST_LONG) fprint_status(f);

	fclose(f);
}

void
log_message(int priority, const char *str, ...)
{
	time_t t;
	char now[32];
	va_list ap;
	FILE *lfp;

	if (priority > global.log_cutoff) return;
	if (global.log_path == NULL) return;

	lfp = fopen(global.log_path, "a");
	if (lfp == NULL) return;

	va_start(ap, str);

	t = time(NULL);
	memset(now, 0, 32);
	strftime(now, 32, "%b %e %T", localtime(&t));

	fprintf(lfp, "%s: ", now);
	vfprintf(lfp, str, ap);
	fflush(lfp);

	va_end(ap);
	fclose(lfp);
}

uint32_t
daemon_post(const char *name, uint32_t u, uint32_t g)
{
	name_info_t *n;
	uint32_t status;

	if (name == NULL) return 0;

	n = (name_info_t *)_nc_table_find(global.notify_state->name_table, name);
	if (n == NULL) return 0;

	if (n->slot != (uint32_t)-1) global.shared_memory_base[n->slot]++;

	status = _notify_lib_post(global.notify_state, name, u, g);
	return status;
}

uint32_t
daemon_post_nid(uint64_t nid, uint32_t u, uint32_t g)
{
	name_info_t *n;
	uint32_t status;

	n = (name_info_t *)_nc_table_find_64(global.notify_state->name_id_table, nid);
	if (n == NULL) return 0;

	if (n->slot != (uint32_t)-1) global.shared_memory_base[n->slot]++;

	status = _notify_lib_post_nid(global.notify_state, nid, u, g);
	return status;
}

void
daemon_post_client(uint64_t cid)
{
	client_t *c;

	c = _nc_table_find_64(global.notify_state->client_table, cid);
	if (c == NULL) return;

	if ((c->notify_type == NOTIFY_TYPE_MEMORY) && (c->name_info != NULL) && (c->name_info->slot != (uint32_t)-1))
	{
		global.shared_memory_base[c->name_info->slot]++;
	}

	_notify_lib_post_client(global.notify_state, c);
}

void
daemon_set_state(const char *name, uint64_t val)
{
	name_info_t *n;

	if (name == NULL) return;

	n = (name_info_t *)_nc_table_find(global.notify_state->name_table, name);
	if (n == NULL) return;

	n->state = val;
}

static void
init_launch_config(const char *name)
{
	launch_data_t tmp, pdict;

	tmp = launch_data_new_string(LAUNCH_KEY_CHECKIN);
	global.launch_dict = launch_msg(tmp);
	launch_data_free(tmp);

	if (global.launch_dict == NULL)
	{
		fprintf(stderr, "%d launchd checkin failed\n", getpid());
		exit(1);
	}

	tmp = launch_data_dict_lookup(global.launch_dict, LAUNCH_JOBKEY_MACHSERVICES);
	if (tmp == NULL)
	{
		fprintf(stderr, "%d launchd lookup of LAUNCH_JOBKEY_MACHSERVICES failed\n", getpid());
		exit(1);
	}

	pdict = launch_data_dict_lookup(tmp, name);
	if (pdict == NULL)
	{
		fprintf(stderr, "%d launchd lookup of name %s failed\n", getpid(), name);
		exit(1);
	}

	global.server_port = launch_data_get_machport(pdict);
	if (global.server_port == MACH_PORT_NULL)
	{
		fprintf(stderr, "%d launchd lookup of server port for name %s failed\n", getpid(), name);
		exit(1);
	}
}

static void
string_list_free(char **l)
{
	int i;

	if (l == NULL) return;
	for (i = 0; l[i] != NULL; i++)
	{
		if (l[i] != NULL) free(l[i]);
		l[i] = NULL;
	}
	free(l);
}

static char **
string_insert(char *s, char **l, unsigned int x)
{
	int i, len;

	if (s == NULL) return l;
	if (l == NULL) 
	{
		l = (char **)malloc(2 * sizeof(char *));
		l[0] = strdup(s);
		l[1] = NULL;
		return l;
	}

	for (i = 0; l[i] != NULL; i++);
	len = i + 1; /* count the NULL on the end of the list too! */

	l = (char **)realloc(l, (len + 1) * sizeof(char *));

	if ((x >= (len - 1)) || (x == IndexNull))
	{
		l[len - 1] = strdup(s);
		l[len] = NULL;
		return l;
	}

	for (i = len; i > x; i--) l[i] = l[i - 1];
	l[x] = strdup(s);
	return l;
}

static char **
string_append(char *s, char **l)
{
	return string_insert(s, l, IndexNull);
}

static unsigned int
string_list_length(char **l)
{
	int i;

	if (l == NULL) return 0;
	for (i = 0; l[i] != NULL; i++);
	return i;
}

static unsigned int
string_index(char c, char *s)
{
	int i;
	char *p;

	if (s == NULL) return IndexNull;

	for (i = 0, p = s; p[0] != '\0'; p++, i++)
	{
		if (p[0] == c) return i;
	}

	return IndexNull;
}

static char **
explode(char *s, char *delim)
{
	char **l = NULL;
	char *p, *t;
	int i, n;

	if (s == NULL) return NULL;

	p = s;
	while (p[0] != '\0')
	{
		for (i = 0; ((p[i] != '\0') && (string_index(p[i], delim) == IndexNull)); i++);
		n = i;
		t = malloc(n + 1);
		for (i = 0; i < n; i++) t[i] = p[i];
		t[n] = '\0';
		l = string_append(t, l);
		free(t);
		t = NULL;
		if (p[i] == '\0') return l;
		if (p[i + 1] == '\0') l = string_append("", l);
		p = p + i + 1;
	}
	return l;
}

static uint32_t
atoaccess(char *s)
{
	uint32_t a;

	if (s == NULL) return 0;
	if (strlen(s) != 6) return 0;

	a = 0;
	if (s[0] == 'r') a |= (NOTIFY_ACCESS_READ << NOTIFY_ACCESS_USER_SHIFT);
	if (s[1] == 'w') a |= (NOTIFY_ACCESS_WRITE << NOTIFY_ACCESS_USER_SHIFT);

	if (s[2] == 'r') a |= (NOTIFY_ACCESS_READ << NOTIFY_ACCESS_GROUP_SHIFT);
	if (s[3] == 'w') a |= (NOTIFY_ACCESS_WRITE << NOTIFY_ACCESS_GROUP_SHIFT);

	if (s[4] == 'r') a |= (NOTIFY_ACCESS_READ << NOTIFY_ACCESS_OTHER_SHIFT);
	if (s[5] == 'w') a |= (NOTIFY_ACCESS_WRITE << NOTIFY_ACCESS_OTHER_SHIFT);

	return a;
}

static void
init_config()
{
	FILE *f;
	struct stat sb;
	char line[1024];
	char **args;
	uint32_t status, argslen;
	uint32_t uid, gid, access;
	uint64_t nid, val64;

	/*
	 * Set IPC Version Number & PID
	 */
	val64 = getpid();
	val64 <<= 32;
	val64 |= NOTIFY_IPC_VERSION;

	_notify_lib_register_plain(global.notify_state, NOTIFY_IPC_VERSION_NAME, -1, notifyd_token++, -1, 0, 0, &nid);
	_notify_lib_set_state(global.notify_state, nid, val64, 0, 0);

	/* Check config file */
	if (stat(CONFIG_FILE_PATH, &sb) != 0) return;

	if (sb.st_uid != 0)
	{
		log_message(ASL_LEVEL_ERR, "config file %s not owned by root: ignored\n", CONFIG_FILE_PATH);
		return;
	}

	if (sb.st_mode & 02)
	{
		log_message(ASL_LEVEL_ERR, "config file %s is world-writable: ignored\n", CONFIG_FILE_PATH);
		return;
	}

	/* Read config file */
	f = fopen(CONFIG_FILE_PATH, "r");
	if (f == NULL) return;

	forever
	{
		if (fgets(line, 1024, f) == NULL) break;
		if (line[0] == '\0') continue;
		if (line[0] == '#') continue;

		line[strlen(line) - 1] = '\0';
		args = explode(line, "\t ");
		argslen = string_list_length(args);
		if (argslen == 0) continue;

		if (!strcasecmp(args[0], "monitor"))
		{
			if (argslen < 3)
			{
				string_list_free(args);
				continue;
			}
			_notify_lib_register_plain(global.notify_state, args[1], -1, notifyd_token++, -1, 0, 0, &nid);
			service_open_path(args[1], args[2], 0, 0);
		}

		if (!strcasecmp(args[0], "timer"))
		{
			if (argslen < 3)
			{
				string_list_free(args);
				continue;
			}
			_notify_lib_register_plain(global.notify_state, args[1], -1, notifyd_token++, -1, 0, 0, &nid);
			status = service_open_timer(args[1], args[2]);
		}

		else if (!strcasecmp(args[0], "set"))
		{
			if (argslen < 3)
			{
				string_list_free(args);
				continue;
			}

			val64 = atoll(args[2]);

			_notify_lib_register_plain(global.notify_state, args[1], -1, notifyd_token++, -1, 0, 0, &nid);
			_notify_lib_set_state(global.notify_state, nid, val64, 0, 0);
		}

		else if (!strcasecmp(args[0], "reserve"))
		{
			if (argslen == 1)
			{
				string_list_free(args);
				continue;
			}

			uid = 0;
			gid = 0;
			access = NOTIFY_ACCESS_DEFAULT;

			if (argslen > 2) uid = atoi(args[2]);
			if (argslen > 3) gid = atoi(args[3]);
			if (argslen > 4) access = atoaccess(args[4]);

			if ((uid != 0) || (gid != 0)) _notify_lib_set_owner(global.notify_state, args[1], uid, gid);
			if (access != NOTIFY_ACCESS_DEFAULT) _notify_lib_set_access(global.notify_state, args[1], access);
		}
		else if (!strcasecmp(args[0], "quit"))
		{
			string_list_free(args);
			break;
		}

		string_list_free(args);
	}

	fclose(f);
}

static void
service_mach_message(bool blocking)
{
	__block kern_return_t status;
	uint32_t rbits, sbits;
	notify_request_msg *request;
	notify_reply_msg *reply;
	char rbuf[sizeof(notify_request_msg) + MAX_TRAILER_SIZE];
	char sbuf[sizeof(notify_reply_msg) + MAX_TRAILER_SIZE];

	forever
	{
		memset(rbuf, 0, sizeof(rbuf));
		memset(sbuf, 0, sizeof(sbuf));

		request = (notify_request_msg *)rbuf;
		reply = (notify_reply_msg *)sbuf;

		request->head.msgh_local_port = global.server_port;
		request->head.msgh_size = global.request_size;

		rbits = MACH_RCV_MSG | (blocking ? 0 : MACH_RCV_TIMEOUT) | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AUDIT) | MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0) | MACH_RCV_VOUCHER;
		sbits = MACH_SEND_MSG;

		status = mach_msg(&(request->head), rbits, 0, global.request_size, global.server_port, 0, MACH_PORT_NULL);
		if (status != KERN_SUCCESS) return;

		voucher_mach_msg_state_t voucher = voucher_mach_msg_adopt(&(request->head));

#if TARGET_OS_EMBEDDED
		/* Synchronize with work_q since on embedded main() calls this
		 * from the global concurrent queue. */
		dispatch_sync(global.work_q, ^{
			status = notify_ipc_server(&(request->head), &(reply->head));
		});
#else
		status = notify_ipc_server(&(request->head), &(reply->head));
#endif

		if (!status && (request->head.msgh_bits & MACH_MSGH_BITS_COMPLEX))
		{
			/* destroy the request - but not the reply port */
			request->head.msgh_remote_port = MACH_PORT_NULL;
			mach_msg_destroy(&(request->head));
		}

		if (reply->head.msgh_remote_port)
		{
			status = mach_msg(&(reply->head), sbits, reply->head.msgh_size, 0, MACH_PORT_NULL, 0, MACH_PORT_NULL);
			if (status == MACH_SEND_INVALID_DEST || status == MACH_SEND_TIMED_OUT)
			{
				/* deallocate reply port rights not consumed by failed mach_msg() send */
				mach_msg_destroy(&(reply->head));
			}
		}

		voucher_mach_msg_revert(voucher);
	}
}

static int32_t
open_shared_memory(const char *name)
{
	int32_t shmfd, isnew;
	uint32_t size;

	size = global.nslots * sizeof(uint32_t);

	isnew = 1;
	shmfd = shm_open(name, O_RDWR, 0644);
	if (shmfd != -1)
	{
		isnew = 0;
	}
	else
	{
		shmfd = shm_open(name, O_RDWR | O_CREAT, 0644);
	}

	if (shmfd == -1)
	{
		char error_message[1024];
		snprintf(error_message, sizeof(error_message), "shm_open %s failed: %s\n", name, strerror(errno));

		CRSetCrashLogMessage(error_message);
		log_message(ASL_LEVEL_NOTICE, "%s", error_message);
		return -1;
	}

	ftruncate(shmfd, size);
	global.shared_memory_base = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
	close(shmfd);

	if (isnew == 0)
	{
		global.last_shm_base = malloc(size);
		if (global.last_shm_base != NULL) memcpy(global.last_shm_base, global.shared_memory_base, size);
	}

	memset(global.shared_memory_base, 0, size);
	global.shared_memory_refcount = (uint32_t *)malloc(size);
	if (global.shared_memory_refcount == NULL) return -1;

	memset(global.shared_memory_refcount, 0, size);

	/* slot 0 is notifyd's pid */
	global.shared_memory_base[0] = getpid();
	global.shared_memory_refcount[0] = 1;
	global.slot_id = 0;

	return 0;
}

int
main(int argc, const char *argv[])
{
	dispatch_queue_t main_q;
	const char *service_name;
	const char *shm_name;
	uint32_t i, status;
	struct rlimit rlim;

#if TARGET_IPHONE_SIMULATOR
	asprintf(&_config_file_path, "%s/private/etc/notify.conf", getenv("SIMULATOR_ROOT"));
 	asprintf(&_debug_log_path, "%s/var/log/notifyd.log", getenv("SIMULATOR_LOG_ROOT"));
#endif

	service_name = NOTIFY_SERVICE_NAME;
	shm_name = SHM_ID;

	notify_set_options(NOTIFY_OPT_DISABLE);

	/* remove limit of number of file descriptors */
	rlim.rlim_max = RLIM_INFINITY;
	rlim.rlim_cur = MIN(OPEN_MAX, rlim.rlim_max);
	setrlimit(RLIMIT_NOFILE, &rlim);

	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGWINCH, SIG_IGN);

	memset(&call_statistics, 0, sizeof(struct call_statistics_s));

	global.request_size = sizeof(notify_request_msg) + MAX_TRAILER_SIZE;
	global.reply_size = sizeof(notify_reply_msg) + MAX_TRAILER_SIZE;
	global.nslots = getpagesize() / sizeof(uint32_t);
	global.notify_state = _notify_lib_notify_state_new(NOTIFY_STATE_ENABLE_RESEND, 1024);
	global.log_cutoff = ASL_LEVEL_ERR;
	global.log_path = strdup(DEBUG_LOG_PATH);
	global.slot_id = (uint32_t)-1;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-d"))
		{
			global.log_cutoff = ASL_LEVEL_DEBUG;
		}
		else if (!strcmp(argv[i], "-log_cutoff"))
		{
			global.log_cutoff = atoi(argv[++i]);
		}
		else if (!strcmp(argv[i], "-log_file"))
		{
			free(global.log_path);
			global.log_path = strdup(argv[++i]);
		}
		else if (!strcmp(argv[i], "-service"))
		{
			service_name = argv[++i];
		}
		else if (!strcmp(argv[i], "-shm"))
		{
			shm_name = argv[++i];
		}
		else if (!strcmp(argv[i], "-shm_pages"))
		{
			global.nslots = atoi(argv[++i]) * (getpagesize() / sizeof(uint32_t));
		}
	}

	global.log_default = global.log_cutoff;

	log_message(ASL_LEVEL_DEBUG, "--------------------\nnotifyd start PID %u\n", getpid());

	init_launch_config(service_name);

	if (global.nslots > 0)
	{
		status = open_shared_memory(shm_name);
		assert(status == 0);
	}

	/* init from config file before starting the work queue */
	init_config();

	main_q = dispatch_get_main_queue();
	assert(main_q != NULL);

	global.work_q = dispatch_queue_create("WorkQ", NULL);
	assert(global.work_q != NULL);

#if TARGET_OS_EMBEDDED
	/* Block a thread in mach_msg() to avoid the syscall overhead of frequent
	 * dispatch source wakeup, and synchronize with work_q after message
	 * reception in service_mach_message(). <rdar://problem/8785140> */
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^{
		forever service_mach_message(true);
	});
#else
	global.mach_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, global.server_port, 0, global.work_q);
	assert(global.mach_src != NULL);

	dispatch_source_set_event_handler(global.mach_src, ^{
		service_mach_message(false);
	});
	dispatch_resume(global.mach_src);
#endif

	/* Set up SIGUSR1 */
	global.sig_usr1_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, (uintptr_t)SIGUSR1, 0, main_q);
	assert(global.sig_usr1_src != NULL);
	dispatch_source_set_event_handler(global.sig_usr1_src, ^{
		dispatch_async(global.work_q, ^{ dump_status(STATUS_REQUEST_SHORT); });
	});
	dispatch_resume(global.sig_usr1_src);

	/* Set up SIGUSR2 */
	global.sig_usr2_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, (uintptr_t)SIGUSR2, 0, main_q);
	assert(global.sig_usr2_src != NULL);
	dispatch_source_set_event_handler(global.sig_usr2_src, ^{
		dispatch_async(global.work_q, ^{ dump_status(STATUS_REQUEST_LONG); });
	});
	dispatch_resume(global.sig_usr2_src);

	/* Set up SIGWINCH */
	global.sig_winch_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, (uintptr_t)SIGWINCH, 0, main_q);
	assert(global.sig_winch_src != NULL);
	dispatch_source_set_event_handler(global.sig_winch_src, ^{
		if (global.log_cutoff == ASL_LEVEL_DEBUG) global.log_cutoff = global.log_default;
		else global.log_cutoff = ASL_LEVEL_DEBUG;
	});
	dispatch_resume(global.sig_winch_src);

	dispatch_main();

	/* NOTREACHED */
	return 0;
}

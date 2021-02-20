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

#include "notifyd.h"
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
#include <xpc/xpc.h>
#include <xpc/private.h>
#include <asl.h>
#include <assert.h>
#include <inttypes.h>
#include <TargetConditionals.h>
#include <bsm/libbsm.h>
#include <servers/bootstrap.h>

#include "pathwatch.h"
#include "service.h"
#include "pathwatch.h"
#include "timer.h"

#include "notify_internal.h"
#include "notify.h"
#include "notify_ipc.h"
#include "notify_ipcServer.h"
#include "notify_private.h"
#include "notifyServer.h"

#define CRSetCrashLogMessage(msg) /**/

#define forever for(;;)
#define IndexNull -1

/* Compile flags */
#define RUN_TIME_CHECKS

#if TARGET_OS_SIMULATOR
static char *_config_file_path;
#define CONFIG_FILE_PATH _config_file_path

static char *_debug_log_path;
#define DEBUG_LOG_PATH _debug_log_path
#else
#define CONFIG_FILE_PATH "/etc/notify.conf"
#define DEBUG_LOG_PATH "/var/log/notifyd.log"
#endif



#define N_NOTIFY_TYPES 6

static int notifyd_token;

static char *status_file = NULL;

struct global_s global;
struct call_statistics_s call_statistics;

static void
notify_reset_stats(void)
{
	_nc_table_foreach(&global.notify_state.name_table, ^bool (void *_n){
		name_info_t *n = _n;

		n->last_hour_postcount = n->postcount;
		n->postcount = 0;
		return true;
	});

	global.last_reset_time = time(NULL);
}

dispatch_queue_t
get_notifyd_workloop(void)
{
	return global.workloop;
}

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

}

static void
fprint_client(FILE *f, client_t *c)
{
	if (c == NULL)
	{
		fprintf(f, "NULL client\n");
		return;
	}

	fprintf(f, "client_id: %llu\n", c->cid.hash_key);
	fprintf(f, "pid: %d\n", c->cid.pid);
	fprintf(f, "token: %d\n", c->cid.token);
	fprintf(f, "lastval: %u\n", c->lastval);
	fprintf(f, "suspend_count: %u\n", c->suspend_count);
	fprintf(f, "type: %s\n", notify_type_name(c->state_and_type & NOTIFY_TYPE_MASK));
	switch(c->state_and_type & NOTIFY_TYPE_MASK)
	{
		case NOTIFY_TYPE_NONE:
			break;

		case NOTIFY_TYPE_PLAIN:
			break;

		case NOTIFY_TYPE_MEMORY:
			break;

		case NOTIFY_TYPE_PORT:
			fprintf(f, "mach port: 0x%08x\n", c->deliver.port);
			break;

		case NOTIFY_TYPE_FILE:
			fprintf(f, "fd: %d\n", c->deliver.fd);
			break;

		case NOTIFY_TYPE_SIGNAL:
			fprintf(f, "signal: %d\n", c->deliver.sig);
			break;

		default: break;
	}
}

static void
fprint_quick_client(FILE *f, client_t *c)
{
	if (c == NULL) return;

	//client_id,pid,token,lastval,suspend_count,0,type,type-info
	fprintf(f, "%llu,%d,%d,%u,%u,%u,", c->cid.hash_key, c->cid.pid, c->cid.token,
			c->lastval, c->suspend_count, 0);

	switch(c->state_and_type & NOTIFY_TYPE_MASK)
	{
	case NOTIFY_TYPE_PORT:
		fprintf(f, "port,0x%08x\n", c->deliver.port);
		break;

	case NOTIFY_TYPE_FILE:
		fprintf(f, "fd,%d\n", c->deliver.fd);
		break;

	case NOTIFY_TYPE_SIGNAL:
		fprintf(f, "signal,%d\n", c->deliver.sig);
		break;

	case NOTIFY_TYPE_MEMORY:
		fprintf(f, "check,0\n");
		break;

	case NOTIFY_TYPE_NONE:
	case NOTIFY_TYPE_PLAIN:
	default:
		fprintf(f, "other,0\n");
		break;
	}
}

static void
fprint_quick_name_info(FILE *f, name_info_t *n)
{
	client_t *c;

	if (n == NULL) return;

	// name:name\n
	// info:id,uid,gid,access,refcount,postcount,lh_postcount,slot,val,state\n
	// clients:\n
	// <client info>
	// <client info>
	// ...
	fprintf(f, "name:%s\n", n->name);
	fprintf(f, "info:%llu,%u,%u,%03x,%u,%u,%u,", n->name_id, n->uid, n->gid, n->access, n->refcount, n->postcount,
		n->last_hour_postcount);
	if (n->slot == -1)
	{
		fprintf(f, "-1,");
	}
	else
	{
		fprintf(f, "%u,", n->slot);
	}

	fprintf(f, "%u,%llu\n", n->val, n->state);

	fprintf(f, "clients:\n");

	LIST_FOREACH(c, &n->subscriptions, client_subscription_entry)
	{
		fprint_quick_client(f, c);
	}

	fprintf(f, "\n");

}

static void
fprint_name_info(FILE *f, const char *name, name_info_t *n, pid_t *max_pid)
{
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
	fprintf(f, "postcount: %u\n", n->postcount);
	fprintf(f, "last hour postcount: %u\n", n->last_hour_postcount);
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

	LIST_FOREACH(c, &n->subscriptions, client_subscription_entry)
	{
		if ((c->cid.pid != (pid_t)-1) && (c->cid.pid > *max_pid)) *max_pid = c->cid.pid;

		switch (c->state_and_type & NOTIFY_TYPE_MASK)
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

	LIST_FOREACH(c, &n->subscriptions, client_subscription_entry)
	{
		fprintf(f, "\n");
		fprint_client(f, c);
	}
}

static void
fprint_quick_status(FILE *f)
{
	int32_t i;

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
	fprintf(f, "checkin      %llu\n", call_statistics.checkin);
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
	fprintf(f, "set_owner    %llu\n", call_statistics.set_owner);
	fprintf(f, "\n");
	fprintf(f, "set_access   %llu\n", call_statistics.set_access);
	fprintf(f, "\n");
	fprintf(f, "monitor      %llu\n", call_statistics.monitor_file);
	fprintf(f, "svc_path     %llu\n", call_statistics.service_path);
	fprintf(f, "svc_timer    %llu\n", call_statistics.service_timer);

	{
		char buf[128];
		strftime(buf, 128, "%a, %d %b %Y %T %z", localtime(&global.last_reset_time));
		fprintf(f, "last reset time was %s\n", buf);
	}

	fprintf(f, "\n");
	fprintf(f, "name         alloc %9u   free %9u   extant %9u\n", global.notify_state.stat_name_alloc , global.notify_state.stat_name_free, global.notify_state.stat_name_alloc - global.notify_state.stat_name_free);
	fprintf(f, "subscription alloc %9u   free %9u   extant %9u\n", global.notify_state.stat_client_alloc , global.notify_state.stat_client_free, global.notify_state.stat_client_alloc - global.notify_state.stat_client_free);
	fprintf(f, "portproc     alloc %9u   free %9u   extant %9u\n", global.notify_state.stat_portproc_alloc , global.notify_state.stat_portproc_free, global.notify_state.stat_portproc_alloc - global.notify_state.stat_portproc_free);
	fprintf(f, "\n");

	fprintf(f, "port count   %u\n", global.notify_state.port_table.count);
	fprintf(f, "proc count   %u\n", global.notify_state.proc_table.count);
	fprintf(f, "\n");

	fprintf(f, "--- NAME TABLE ---\n");
	fprintf(f, "Name Info: id, uid, gid, access, refcount, postcount, last hour postcount, slot, val, state\n");
	fprintf(f, "Client Info: client_id, pid,token, lastval, suspend_count, 0, 0, type, type-info\n\n\n");

	_nc_table_foreach(&global.notify_state.name_table, ^bool(void *n) {
		fprint_quick_name_info(f, n);
		fprintf(f, "\n");
		return true;
	});

	fprintf(f, "--- NAME COUNT %u ---\n", global.notify_state.name_table.count);
	fprintf(f, "\n");

	fprintf(f, "--- CONTROLLED NAME ---\n");
	for (i = 0; i < global.notify_state.controlled_name_count; i++)
	{
		fprintf(f, "%s %u %u %03x\n", global.notify_state.controlled_name[i]->name, global.notify_state.controlled_name[i]->uid, global.notify_state.controlled_name[i]->gid, global.notify_state.controlled_name[i]->access);
	}
	fprintf(f, "--- CONTROLLED NAME COUNT %u ---\n", global.notify_state.controlled_name_count);
	fprintf(f, "\n");

	fprintf(f, "--- PUBLIC SERVICE ---\n");

	_nc_table_foreach(&global.notify_state.name_table, ^bool(void *_n) {
		name_info_t *n = _n;
		svc_info_t *info;
		path_node_t *node;
		timer_t *timer;

		if (n->private == NULL) return true;

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
		return true;
	});

	fprintf(f, "--- PUBLIC SERVICE COUNT %u ---\n", global.notify_state.name_table.count);
	fprintf(f, "\n");

	fprintf(f, "--- PRIVATE SERVICE ---\n");
	_nc_table_foreach_64(&global.notify_state.client_table, ^bool(void *_c) {
		client_t *c = _c;
		svc_info_t *info;
		name_info_t *n;
		path_node_t *node;
		timer_t *timer;

		if (c->service_index == 0) return true;

		info = (svc_info_t *)service_info_get(c->service_index);
		n = c->name_info;

		if (info->type == 0)
		{
			fprintf(f, "PID %u Null service: %s\n", c->cid.pid, n->name);
		}
		if (info->type == SERVICE_TYPE_PATH_PRIVATE)
		{
			node = (path_node_t *)info->private;
			fprintf(f, "PID %u Path Service: %s <- %s\n", c->cid.pid, n->name, node->path);
		}
		else if (info->type == SERVICE_TYPE_TIMER_PRIVATE)
		{
			timer = (timer_t *)info->private;
			switch (timer->type)
			{
				case TIME_EVENT_ONESHOT:
				{
					fprintf(f, "PID %u Time Service: %s <- Oneshot %"PRId64"\n", c->cid.pid, n->name, timer->start);
					break;
				}
				case TIME_EVENT_CLOCK:
				{
					fprintf(f, "PID %u Time Service: %s <- Clock start %"PRId64" freq %"PRIu32" end %"PRId64"\n", c->cid.pid, n->name, timer->start, timer->freq, timer->end);
					break;
				}
				case TIME_EVENT_CAL:
				{
					fprintf(f, "PID %u Time Service: %s <- Calendar start %"PRId64" freq %"PRIu32" end %"PRId64" day %"PRId32"\n", c->cid.pid, n->name, timer->start, timer->freq, timer->end, timer->day);
					break;
				}
			}
		}
		return true;
	});

	fprintf(f, "--- PRIVATE SERVICE COUNT %u ---\n", global.notify_state.client_table.count);
	fprintf(f, "\n");

}

static void
fprint_status(FILE *f)
{
	__block pid_t pid, max_pid;
	int32_t i;

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
	fprintf(f, "checkin      %llu\n", call_statistics.checkin);
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
	fprintf(f, "set_owner    %llu\n", call_statistics.set_owner);
	fprintf(f, "\n");
	fprintf(f, "set_access   %llu\n", call_statistics.set_access);
	fprintf(f, "\n");
	fprintf(f, "monitor      %llu\n", call_statistics.monitor_file);
	fprintf(f, "svc_path     %llu\n", call_statistics.service_path);
	fprintf(f, "svc_timer    %llu\n", call_statistics.service_timer);


	{
		char buf[128];
		strftime(buf, 128, "%a, %d %b %Y %T %z", localtime(&global.last_reset_time));
		fprintf(f, "last reset time was %s\n", buf);
	}

	fprintf(f, "\n");
	fprintf(f, "name         alloc %9u   free %9u   extant %9u\n", global.notify_state.stat_name_alloc , global.notify_state.stat_name_free, global.notify_state.stat_name_alloc - global.notify_state.stat_name_free);
	fprintf(f, "subscription alloc %9u   free %9u   extant %9u\n", global.notify_state.stat_client_alloc , global.notify_state.stat_client_free, global.notify_state.stat_client_alloc - global.notify_state.stat_client_free);
	fprintf(f, "portproc     alloc %9u   free %9u   extant %9u\n", global.notify_state.stat_portproc_alloc , global.notify_state.stat_portproc_free, global.notify_state.stat_portproc_alloc - global.notify_state.stat_portproc_free);
	fprintf(f, "\n");

	fprintf(f, "port count   %u\n", global.notify_state.port_table.count);
	fprintf(f, "proc count   %u\n", global.notify_state.proc_table.count);
	fprintf(f, "\n");

	fprintf(f, "--- NAME TABLE ---\n");

	_nc_table_foreach(&global.notify_state.name_table, ^bool(void *_n) {
		name_info_t *n = _n;
		fprint_name_info(f, n->name, n, &max_pid);
		fprintf(f, "\n");
		return true;
	});

	fprintf(f, "--- NAME COUNT %u ---\n", global.notify_state.name_table.count);
	fprintf(f, "\n");

	fprintf(f, "--- SUBSCRIPTION TABLE ---\n");

	_nc_table_foreach_64(&global.notify_state.client_table, ^bool (void *_c) {
		client_t *c = _c;
		fprintf(f, "%d\n", c->cid.pid);
		return true;
	});

	fprintf(f, "--- SUBSCRIPTION COUNT %u ---\n", global.notify_state.client_table.count);
	fprintf(f, "\n");

	fprintf(f, "--- CONTROLLED NAME ---\n");
	for (i = 0; i < global.notify_state.controlled_name_count; i++)
	{
		fprintf(f, "%s %u %u %03x\n", global.notify_state.controlled_name[i]->name, global.notify_state.controlled_name[i]->uid, global.notify_state.controlled_name[i]->gid, global.notify_state.controlled_name[i]->access);
	}
	fprintf(f, "--- CONTROLLED NAME COUNT %u ---\n", global.notify_state.controlled_name_count);
	fprintf(f, "\n");

	fprintf(f, "--- PUBLIC SERVICE ---\n");

	_nc_table_foreach(&global.notify_state.name_table, ^bool(void *_n) {
		name_info_t *n = _n;
		svc_info_t *info;
		path_node_t *node;
		timer_t *timer;

		if (n->private == NULL) return true;

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
		return true;
	});

	fprintf(f, "--- PUBLIC SERVICE COUNT %u ---\n", global.notify_state.name_table.count);
	fprintf(f, "\n");

	fprintf(f, "--- PRIVATE SERVICE ---\n");

	_nc_table_foreach_64(&global.notify_state.client_table, ^bool (void *_c) {
		client_t *c = _c;
		name_info_t *n;
		svc_info_t *info;
		path_node_t *node;
		timer_t *timer;

        if (c->service_index == 0) return true;

		info = (svc_info_t *)service_info_get(c->service_index);
		n = c->name_info;

		if (info->type == 0)
		{
			fprintf(f, "PID %u Null service: %s\n", c->cid.pid, n->name);
		}
		if (info->type == SERVICE_TYPE_PATH_PRIVATE)
		{
			node = (path_node_t *)info->private;
			fprintf(f, "PID %u Path Service: %s <- %s\n", c->cid.pid, n->name, node->path);
		}
		else if (info->type == SERVICE_TYPE_TIMER_PRIVATE)
		{
			timer = (timer_t *)info->private;
			switch (timer->type)
			{
				case TIME_EVENT_ONESHOT:
				{
					fprintf(f, "PID %u Time Service: %s <- Oneshot %"PRId64"\n", c->cid.pid, n->name, timer->start);
					break;
				}
				case TIME_EVENT_CLOCK:
				{
					fprintf(f, "PID %u Time Service: %s <- Clock start %"PRId64" freq %"PRIu32" end %"PRId64"\n", c->cid.pid, n->name, timer->start, timer->freq, timer->end);
					break;
				}
				case TIME_EVENT_CAL:
				{
					fprintf(f, "PID %u Time Service: %s <- Calendar start %"PRId64" freq %"PRIu32" end %"PRId64" day %"PRId32"\n", c->cid.pid, n->name, timer->start, timer->freq, timer->end, timer->day);
					break;
				}
			}
		}
		return true;
	});

	fprintf(f, "--- PRIVATE SERVICE COUNT %u ---\n", global.notify_state.client_table.count);
	fprintf(f, "\n");

	fprintf(f, "--- PROCESSES ---\n");
	for (pid = 0; pid <= max_pid; pid++)
	{
		int mem_count, plain_count, file_count, port_count, sig_count;
		proc_data_t *pdata;
		client_t *c;

		pdata = _nc_table_find_n(&global.notify_state.proc_table, pid);
		if (pdata == NULL) continue;

		mem_count = 0;
		plain_count = 0;
		file_count = 0;
		port_count = 0;
		sig_count = 0;

		LIST_FOREACH(c, &pdata->clients, client_pid_entry) {
			switch(c->state_and_type & NOTIFY_TYPE_MASK) {
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
				break;

			case NOTIFY_TYPE_FILE:
				file_count++;
				break;

			case NOTIFY_TYPE_SIGNAL:
				sig_count++;
				break;

			default:
				break;
			}
		}

		fprintf(f, "pid: %u   ", pid);

		fprintf(f, "memory %u   plain %u   port %u   file %u   signal %u\n",
				mem_count, plain_count, port_count, file_count, sig_count);
		LIST_FOREACH(c, &pdata->clients, client_pid_entry) {
			fprintf(f, "  %s: %s\n", notify_type_name(c->state_and_type & NOTIFY_TYPE_MASK), c->name_info->name);
		}

		fprintf(f, "\n");
	}
	fprintf(f, "\n");
}

void
dump_status(uint32_t level, int fd)
{
	FILE *f;

	if(fd < 0)
	{
		if (status_file == NULL)
		{
			asprintf(&status_file, "/var/run/notifyd_%u.status", getpid());
			if (status_file == NULL) return;
		}

		unlink(status_file);
		f = fopen(status_file, "w");
	}
	else
	{
		f = fdopen(fd, "w");
	}

	if (f == NULL) return;

	if (level == STATUS_REQUEST_SHORT) fprint_quick_status(f);
	else if (level == STATUS_REQUEST_LONG) fprint_status(f);

	fclose(f);
}

static void
dump_status_handler(void *level)
{
	dump_status((uint32_t)level, -1);
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

bool
has_entitlement(audit_token_t audit, const char *entitlement)
{
	xpc_object_t edata;
	bool result = false;

	pid_t pid = audit_token_to_pid(audit);

	log_message(ASL_LEVEL_NOTICE, "-> has_root_entitlement (PID %d)\n", pid);

	edata = xpc_copy_entitlement_for_token(entitlement, &audit);

	if(edata != NULL){
		result = xpc_bool_get_value(edata);
		xpc_release(edata);
	}

	log_message(ASL_LEVEL_NOTICE, "<- has_root_entitlement (PID %d) = %s\n", pid, result ? "OK" : "FAIL");
	return result;
}

inline bool
has_root_entitlement(audit_token_t audit)
{
	return has_entitlement(audit, ROOT_ENTITLEMENT_KEY);
}

uint32_t
daemon_post(const char *name, uint32_t u, uint32_t g)
{
	name_info_t *n;
	uint32_t status;

	if (name == NULL) return NOTIFY_STATUS_NULL_INPUT;

	n = _nc_table_find(&global.notify_state.name_table, name);
	if (n == NULL) return NOTIFY_STATUS_OK;

	if (n->slot != (uint32_t)-1) global.shared_memory_base[n->slot]++;

	status = _notify_lib_post(&global.notify_state, name, u, g);
	return status;
}

uint32_t
daemon_post_nid(uint64_t nid, uint32_t u, uint32_t g)
{
	name_info_t *n;
	uint32_t status;

	n = _nc_table_find_64(&global.notify_state.name_id_table, nid);
	if (n == NULL) return NOTIFY_STATUS_OK;

	if (n->slot != (uint32_t)-1) global.shared_memory_base[n->slot]++;

	status = _notify_lib_post_nid(&global.notify_state, nid, u, g);
	return status;
}

void
daemon_post_client(uint64_t cid)
{
	client_t *c;

	c = _nc_table_find_64(&global.notify_state.client_table, cid);
	if (c == NULL) return;

	if ((c->state_and_type & NOTIFY_TYPE_MEMORY) && (c->name_info != NULL) && (c->name_info->slot != (uint32_t)-1))
	{
		global.shared_memory_base[c->name_info->slot]++;
	}

	_notify_lib_post_client(&global.notify_state, c);
}

void
daemon_set_state(const char *name, uint64_t val)
{
	name_info_t *n;

	if (name == NULL) return;

	n = _nc_table_find(&global.notify_state.name_table, name);
	if (n == NULL) return;

	n->state = val;
}

static void
init_launch_config(const char *name)
{
	kern_return_t kr;

	kr = bootstrap_check_in(bootstrap_port, name, &global.server_port);

	if(kr != KERN_SUCCESS)
	{
		{
			fprintf(stderr, "%d bootstrap_check_in failed (%d)\n", getpid(), kr);
			exit(1);
		}
	}

	/*
	 * Notifyd uses its port for communication, system wide.
	 * Up our queue length as most of our traffic is made of simpleroutines
	 */
	struct mach_port_limits limits = {
		.mpl_qlimit = 32,
	};

	mach_port_set_attributes(mach_task_self(), global.server_port,
			MACH_PORT_LIMITS_INFO, (mach_port_info_t)&limits, sizeof(limits));
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
init_config(void)
{
	FILE *f;
	struct stat sb;
	char line[1024];
	char **args;
	uint32_t argslen;
	uint32_t uid, gid, access;
	uint64_t nid, val64;

	/*
	 * Set IPC Version Number & PID
	 */
	val64 = getpid();
	val64 <<= 32;
	val64 |= NOTIFY_IPC_VERSION;

	_notify_lib_register_plain(&global.notify_state, NOTIFY_IPC_VERSION_NAME, -1, notifyd_token++, -1, 0, 0, &nid);
	_notify_lib_set_state(&global.notify_state, nid, val64, 0, 0);

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

		if (!strcasecmp(args[0], "monitor")) {
			if (argslen < 3)
			{
				string_list_free(args);
				continue;
			}
			_notify_lib_register_plain(&global.notify_state, args[1], -1, notifyd_token++, -1, 0, 0, &nid);

			dispatch_async(global.workloop, ^{
				service_open_path(args[1], args[2], 0, 0);
				string_list_free(args);
			});
		} else if (!strcasecmp(args[0], "timer")) {
			if (argslen < 3)
			{
				string_list_free(args);
				continue;
			}
			_notify_lib_register_plain(&global.notify_state, args[1], -1, notifyd_token++, -1, 0, 0, &nid);

			dispatch_async(global.workloop, ^{
				service_open_timer(args[1], args[2]);
				string_list_free(args);
			});
		} else if (!strcasecmp(args[0], "set")) {
			if (argslen == 1 || argslen > 3)
			{
				string_list_free(args);
				continue;
			}

			_notify_lib_register_plain(&global.notify_state, args[1], -1, notifyd_token++, -1, 0, 0, &nid);
			if (argslen == 3)
			{
				val64 = atoll(args[2]);
				_notify_lib_set_state(&global.notify_state, nid, val64, 0, 0);
			}

			string_list_free(args);
		} else if (!strcasecmp(args[0], "reserve")) {
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

			if ((uid != 0) || (gid != 0)) _notify_lib_set_owner(&global.notify_state, args[1], uid, gid);
			if (access != NOTIFY_ACCESS_DEFAULT) _notify_lib_set_access(&global.notify_state, args[1], access);

			string_list_free(args);
		} else if (!strcasecmp(args[0], "quit")) {
			string_list_free(args);
			break;
		}

	}

	fclose(f);
}

static void
notifyd_mach_channel_handler(void *context, dispatch_mach_reason_t reason,
		dispatch_mach_msg_t message, mach_error_t error)
{
	static const struct mig_subsystem *subsystems[] = {
		(mig_subsystem_t)&_notify_ipc_subsystem,
	};
	if (reason == DISPATCH_MACH_MESSAGE_RECEIVED) {
		if (!dispatch_mach_mig_demux(context, subsystems, 1, message)) {
			mach_msg_destroy(dispatch_mach_msg_get_msg(message, NULL));
		}
	}
}

kern_return_t
do_mach_notify_port_deleted(mach_port_t notify, mach_port_name_t name)
{
	NOTIFY_INTERNAL_CRASH(0, "notifyd never requests port-deleted");
}

kern_return_t
do_mach_notify_port_destroyed(mach_port_t notify, mach_port_t rights)
{
	NOTIFY_INTERNAL_CRASH(0, "notifyd never requests port-destroyed");
}

kern_return_t
do_mach_notify_no_senders(mach_port_t notify, mach_port_mscount_t mscount)
{
	NOTIFY_INTERNAL_CRASH(0, "notifyd never requests no-senders");
}

kern_return_t
do_mach_notify_send_once(mach_port_t notify)
{
	NOTIFY_INTERNAL_CRASH(0, "someone destroyed our send-possible notification");
}

static void
mach_notifs_handle(void *context, dispatch_mach_reason_t reason,
				   dispatch_mach_msg_t msg, mach_error_t error)
{
	static const struct mig_subsystem *const subsystems[] = {
		(mig_subsystem_t)&do_notify_subsystem,
	};

	if (reason == DISPATCH_MACH_MESSAGE_RECEIVED) {
		if (!dispatch_mach_mig_demux(context, subsystems, 1, msg)) {
			mach_msg_destroy(dispatch_mach_msg_get_msg(msg, NULL));
		}
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
	const char *service_name;
	const char *shm_name;
	uint32_t i, status;
	struct rlimit rlim;
	kern_return_t kr;

#if TARGET_OS_SIMULATOR
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

	global.nslots = getpagesize() / sizeof(uint32_t);
	_notify_lib_notify_state_init(&global.notify_state, NOTIFY_STATE_ENABLE_RESEND);

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

	global.workloop = dispatch_workloop_create_inactive("com.apple.notifyd.main");
	dispatch_set_qos_class_fallback(global.workloop, QOS_CLASS_UTILITY);
	dispatch_activate(global.workloop);

	/* init from config file before starting the listener */
	init_config();

	mach_port_options_t opts = {
		.flags = MPO_STRICT | MPO_CONTEXT_AS_GUARD,
	};
	kr = mach_port_construct(current_task(), &opts, &global, &global.mach_notify_port);
	if (kr != KERN_SUCCESS) {
		NOTIFY_INTERNAL_CRASH(kr, "Unable to allocate Mach notification port");
	}
	global.mach_notifs_channel = dispatch_mach_create_f("com.apple.notifyd.mach-notifs",
			global.workloop, NULL, mach_notifs_handle);
	dispatch_set_qos_class_fallback(global.mach_notifs_channel, QOS_CLASS_USER_INITIATED);
	dispatch_mach_connect(global.mach_notifs_channel, global.mach_notify_port,
			MACH_PORT_NULL, NULL);

	global.mach_channel = dispatch_mach_create_f("com.apple.notifyd.channel",
			global.workloop, NULL, notifyd_mach_channel_handler);
#if TARGET_OS_SIMULATOR
	// simulators don't support MiG QoS propagation yet
	dispatch_set_qos_class_fallback(global.mach_channel, QOS_CLASS_USER_INITIATED);
#else
	dispatch_set_qos_class_fallback(global.mach_channel, QOS_CLASS_BACKGROUND);
#endif
	dispatch_mach_connect(global.mach_channel, global.server_port, MACH_PORT_NULL, NULL);

	/* Set up SIGUSR1 */
	global.sig_usr1_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL,
			(uintptr_t)SIGUSR1, 0, global.workloop);
	assert(global.sig_usr1_src != NULL);
	dispatch_set_context(global.sig_usr1_src, (void *)STATUS_REQUEST_SHORT);
	dispatch_source_set_event_handler_f(global.sig_usr1_src, dump_status_handler);
	dispatch_activate(global.sig_usr1_src);

	/* Set up SIGUSR2 */
	global.sig_usr2_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL,
			(uintptr_t)SIGUSR2, 0, global.workloop);
	assert(global.sig_usr2_src != NULL);
	dispatch_set_context(global.sig_usr2_src, (void *)STATUS_REQUEST_LONG);
	dispatch_source_set_event_handler_f(global.sig_usr2_src, dump_status_handler);
	dispatch_activate(global.sig_usr2_src);

	/* Set up SIGWINCH */
	global.sig_winch_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL,
			(uintptr_t)SIGWINCH, 0, global.workloop);
	assert(global.sig_winch_src != NULL);
	dispatch_source_set_event_handler(global.sig_winch_src, ^{
		if (global.log_cutoff == ASL_LEVEL_DEBUG) global.log_cutoff = global.log_default;
		else global.log_cutoff = ASL_LEVEL_DEBUG;
	});
	dispatch_activate(global.sig_winch_src);

	global.stat_reset_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, global.workloop);
	{
		dispatch_time_t trigger = dispatch_walltime(NULL, 0);
		dispatch_source_set_timer(global.stat_reset_src, trigger, 60 * 60 * NSEC_PER_SEC, 0);
	}
	dispatch_source_set_event_handler(global.stat_reset_src, ^{
		notify_reset_stats();
	});
	dispatch_activate(global.stat_reset_src);
	notify_reset_stats();

	dispatch_main();
}

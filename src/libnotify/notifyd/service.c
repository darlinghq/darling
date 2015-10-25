/*
 * Copyright (c) 2003-2010 Apple Inc. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <asl.h>
#include "notify.h"
#include "notifyd.h"
#include "service.h"
#include "pathwatch.h"
#include "timer.h"

#define NOTIFY_PATH_SERVICE "path:"
#define NOTIFY_PATH_SERVICE_LEN 5
#define NOTIFY_TIMER_SERVICE "timer:"
#define NOTIFY_TIMER_SERVICE_LEN 6

/* Libinfo global */
extern uint32_t gL1CacheEnabled;

static uint32_t
service_type(const char *name)
{
	uint32_t len;

	len = SERVICE_PREFIX_LEN;
	if (strncmp(name, SERVICE_PREFIX, len)) return SERVICE_TYPE_NONE;
	else if (!strncmp(name + len, NOTIFY_PATH_SERVICE, NOTIFY_PATH_SERVICE_LEN)) return SERVICE_TYPE_PATH_PRIVATE;
	else if (!strncmp(name + len, NOTIFY_TIMER_SERVICE, NOTIFY_TIMER_SERVICE_LEN)) return SERVICE_TYPE_TIMER_PRIVATE;
	
	return SERVICE_TYPE_NONE;
}

/*
 * Request notifications for changes on a filesystem path.
 * This creates a new pathwatch node and sets it to post notifications for
 * the specified name.
 *
 * If the notify name already has a pathwatch node for this path, this routine
 * does nothing and allows the client to piggypack on the existing path watcher.
 *
 * Note that this routine is only called for path monitoring as directed by
 * a "monitor" command in /etc/notify.conf, so only an admin can set up a path
 * that gets public notifications.  A client being serviced by the server-side
 * routines in notify_proc.c will only be able to register for a private 
 * (per-client) notification for a path.  This prevents a client from 
 * piggybacking on another client's notifications, and thus prevents the client
 * from getting notifications for a path to which they don't have access. 
 */
int
service_open_path(const char *name, const char *path, uid_t uid, gid_t gid)
{
	name_info_t *n;
	svc_info_t *info;
	path_node_t *node;

	call_statistics.service_path++;

	if (path == NULL) return NOTIFY_STATUS_INVALID_REQUEST;

	n = (name_info_t *)_nc_table_find(global.notify_state->name_table, name);
	if (n == NULL) return NOTIFY_STATUS_INVALID_NAME;

	if (n->private != NULL)
	{
		/* a notify key may only have one service associated with it */
		info = (svc_info_t *)n->private;
		if (info->type != SERVICE_TYPE_PATH_PUBLIC) return NOTIFY_STATUS_INVALID_REQUEST;

		/* the client must be asking for the same path that is being monitored */
		node = (path_node_t *)info->private;
		if (strcmp(path, node->path)) return NOTIFY_STATUS_INVALID_REQUEST;

		/* the name is already getting notifications for this path */
		return NOTIFY_STATUS_OK;
	}

	node = path_node_create(path, uid, gid, PATH_NODE_ALL, dispatch_get_main_queue());
	if (node == NULL) return NOTIFY_STATUS_FAILED;
	
	node->contextp = strdup(name);

	info = (svc_info_t *)calloc(1, sizeof(svc_info_t));
	assert(info != NULL);

	info->type = SERVICE_TYPE_PATH_PUBLIC;
	info->private = node;
	n->private = info;

	dispatch_source_set_event_handler(node->src, ^{
		dispatch_async(global.work_q, ^{
			if (0 == dispatch_source_testcancel(node->src))
			{
				daemon_post((const char *)node->contextp, uid, gid);
			}
		});
	});
	
	dispatch_resume(node->src);

	return NOTIFY_STATUS_OK;
}

/*
 * The private (per-client) path watch service.
 */
int
service_open_path_private(const char *name, client_t *c, const char *path, uid_t uid, gid_t gid, uint32_t flags)
{
	name_info_t *n;
	svc_info_t *info;
	path_node_t *node;

	call_statistics.service_path++;

	if (path == NULL) return NOTIFY_STATUS_INVALID_REQUEST;
	
	n = (name_info_t *)_nc_table_find(global.notify_state->name_table, name);
	if (n == NULL) return NOTIFY_STATUS_INVALID_NAME;
	if (c == NULL) return NOTIFY_STATUS_FAILED;

	if (c->private != NULL)
	{
		/* a client may only have one service */
		info = (svc_info_t *)c->private;
		if (info->type != SERVICE_TYPE_PATH_PRIVATE) return NOTIFY_STATUS_INVALID_REQUEST;
		
		/* the client must be asking for the same path that is being monitored */
		node = (path_node_t *)info->private;
		if (strcmp(path, node->path)) return NOTIFY_STATUS_INVALID_REQUEST;
		
		/* the client is already getting notifications for this path */
		return NOTIFY_STATUS_OK;
	}

	if (flags == 0) flags = PATH_NODE_ALL;

	node = path_node_create(path, uid, gid, flags, dispatch_get_main_queue());
	if (node == NULL) return NOTIFY_STATUS_FAILED;
	
	node->context64 = c->client_id;
	
	info = (svc_info_t *)calloc(1, sizeof(svc_info_t));
	assert(info != NULL);
	
	info->type = SERVICE_TYPE_PATH_PRIVATE;
	info->private = node;
	c->private = info;
	
	dispatch_source_set_event_handler(node->src, ^{
		dispatch_async(global.work_q, ^{
			if (0 == dispatch_source_testcancel(node->src))
			{
				daemon_post_client(node->context64);
			}
		});
	});
	
	dispatch_resume(node->src);
	
	return NOTIFY_STATUS_OK;
}

/* format: [+]nnnn[s|m|h|d] */
static int
parse_single_arg(const char *arg, int relative_ok, time_t *t)
{
	const char *p, *q;
	time_t now, val;

	if (arg == NULL) return -1;
	p = arg;

	now = 0;

	if ((relative_ok != 0) && ((*p == '+') || (*p == '-')))
	{
		p++;
		now = time(NULL);
	}

	if ((*p < '0') || (*p > '9')) return -1;

	q = strchr(p, '.');
	if (q != NULL) q--;
	else q = arg + strlen(arg) - 1;

#ifdef __LP64__
	val = (time_t)atoll(p);
#else
	val = (time_t)atoi(p);
#endif

	if ((*q >= '0') && (*q <= '9'))
	{}
	else if (*q == 's')
	{}
	else if (*q == 'm')
	{
		val *= 60;
	}
	else if (*q == 'h')
	{
		val *= 3600;
	}
	else if (*q == 'd')
	{
		val *= 86400;
	}
	else
	{
		return -1;
	}

	if (*arg == '-') *t = now - val;
	else *t = now + val;

	return 0;
}

static uint32_t
parse_timer_args(const char *args, time_t *s, time_t *f, time_t *e, int32_t *d)
{
	char *p;
	uint32_t t;

	if (args == NULL) return TIME_EVENT_NONE;

	/* first arg is start time */
	if (parse_single_arg(args, 1, s) != 0) return TIME_EVENT_NONE;
	t = TIME_EVENT_ONESHOT;
	
	p = strchr(args, '.');
	if (p != NULL)
	{
		/* second arg is frequency */
		p++;
		if (parse_single_arg(p, 0, f) != 0) return TIME_EVENT_NONE;
		t = TIME_EVENT_CLOCK;
		
		p = strchr(p, '.');
		if (p != NULL)
		{
			/* third arg is end time */
			p++;
			if (parse_single_arg(args, 1, e) != 0) return TIME_EVENT_NONE;
			
			p = strchr(p, '.');
			if (p != NULL)
			{
				/* fourth arg is day number */
				p++;
				*d = atoi(p);
				t = TIME_EVENT_CAL;
			}
		}
	}
	
	if (f == 0) t = TIME_EVENT_ONESHOT;

	return t;
}

int
service_open_timer(const char *name, const char *args)
{
	uint32_t t;
	time_t s, f, e;
	int32_t d;
	name_info_t *n;
	svc_info_t *info;
	timer_t *timer;

	call_statistics.service_timer++;

	n = (name_info_t *)_nc_table_find(global.notify_state->name_table, name);
	if (n == NULL) return NOTIFY_STATUS_INVALID_NAME;

	s = f = e = 0;
	d = 0;

	t = parse_timer_args(args, &s, &f, &e, &d);
	if (t == TIME_EVENT_NONE) return NOTIFY_STATUS_INVALID_REQUEST;

	if (n->private != NULL)
	{
		/* a notify key may only have one service associated with it */
		info = (svc_info_t *)n->private;
		if (info->type != SERVICE_TYPE_TIMER_PUBLIC) return NOTIFY_STATUS_INVALID_REQUEST;
		
		/* the client must be asking for the same timer that is active */
		timer = (timer_t *)info->private;
		if ((timer->type != t) || (timer->start != s) || (timer->freq != f) || (timer->end != e) || (timer->day != d)) return NOTIFY_STATUS_INVALID_REQUEST;
		
		/* the name is already getting notifications for this timer */
		return NOTIFY_STATUS_OK;
	}

	switch (t)
	{
		case TIME_EVENT_ONESHOT:
		{
			timer = timer_oneshot(s, dispatch_get_main_queue());
			break;
		}
		case TIME_EVENT_CLOCK:
		{
			timer = timer_clock(s, f, e, dispatch_get_main_queue());
			break;
		}
		case TIME_EVENT_CAL:
		{
			timer = timer_calendar(s, f, d, e, dispatch_get_main_queue());
			break;
		}
		default:
		{
			return NOTIFY_STATUS_FAILED;
		}
	}

	if (timer == NULL) return NOTIFY_STATUS_FAILED;
	timer->contextp = strdup(name);

	info = (svc_info_t *)calloc(1, sizeof(svc_info_t));
	assert(info != NULL);

	info->type = SERVICE_TYPE_TIMER_PUBLIC;
	info->private = timer;
	n->private = info;

	dispatch_source_set_event_handler(timer->src, ^{
		dispatch_async(global.work_q, ^{
			if (0 == dispatch_source_testcancel(timer->src))
			{
				daemon_post((const char *)timer->contextp, 0, 0);
			}
		});
	});
	
	dispatch_resume(timer->src);

	return NOTIFY_STATUS_OK;
}

int
service_open_timer_private(const char *name, client_t *c, const char *args)
{
	uint32_t t;
	time_t s, f, e;
	int32_t d;
	name_info_t *n;
	svc_info_t *info;
	timer_t *timer;

	call_statistics.service_timer++;

	n = (name_info_t *)_nc_table_find(global.notify_state->name_table, name);
	if (n == NULL) return NOTIFY_STATUS_INVALID_NAME;
	if (c == NULL) return NOTIFY_STATUS_FAILED;

	s = f = e = 0;
	d = 0;
	
	t = parse_timer_args(args, &s, &f, &e, &d);
	if (t == TIME_EVENT_NONE) return NOTIFY_STATUS_INVALID_REQUEST;
	
	if (c->private != NULL)
	{
		/* a client may only have one service */
		info = (svc_info_t *)c->private;
		if (info->type != SERVICE_TYPE_TIMER_PRIVATE) return NOTIFY_STATUS_INVALID_REQUEST;
		
		/* the client must be asking for the same timer that is active */
		timer = (timer_t *)info->private;
		if ((timer->type != t) || (timer->start != s) || (timer->freq != f) || (timer->end != e) || (timer->day != d)) return NOTIFY_STATUS_INVALID_REQUEST;
		
		/* the client is already getting notifications for this timer */
		return NOTIFY_STATUS_OK;
	}
	
	switch (t)
	{
		case TIME_EVENT_ONESHOT:
		{
			timer = timer_oneshot(s, dispatch_get_main_queue());
			break;
		}
		case TIME_EVENT_CLOCK:
		{
			timer = timer_clock(s, f, e, dispatch_get_main_queue());
			break;
		}
		case TIME_EVENT_CAL:
		{
			timer = timer_calendar(s, f, d, e, dispatch_get_main_queue());
			break;
		}
		default:
		{
			return NOTIFY_STATUS_FAILED;
		}
	}
	
	if (timer == NULL) return NOTIFY_STATUS_FAILED;
	timer->context64 = c->client_id;
	
	info = (svc_info_t *)calloc(1, sizeof(svc_info_t));
	assert(info != NULL);
	
	info->type = SERVICE_TYPE_TIMER_PRIVATE;
	info->private = timer;
	c->private = info;
	
	dispatch_source_set_event_handler(timer->src, ^{
		dispatch_async(global.work_q, ^{
			if (0 == dispatch_source_testcancel(timer->src))
			{
				daemon_post_client(timer->context64);
			}
		});
	});
	
	dispatch_resume(timer->src);
	
	return NOTIFY_STATUS_OK;
}

/* called from server-side routines in notify_proc - services are private to the client */
int
service_open(const char *name, client_t *client, uint32_t uid, uint32_t gid)
{
	uint32_t t, flags;
    char *p, *q;
 
	t = service_type(name);

	switch (t)
	{
		case SERVICE_TYPE_NONE:
		{
			return NOTIFY_STATUS_OK;
		}
		case SERVICE_TYPE_PATH_PRIVATE:
		{
			p = strchr(name, ':');
			if (p != NULL) p++;

			flags = 0;

			q = strchr(p, ':');
			if (q != NULL) 
			{
				flags = strtol(p, NULL, 0);
				p = q + 1;
			}

			return service_open_path_private(name, client, p, uid, gid, flags);
		}
		case SERVICE_TYPE_TIMER_PRIVATE:
		{
			p = strchr(name, ':');
			if (p != NULL) p++;
			return service_open_timer_private(name, client, p);
		}
		default: 
		{
			return NOTIFY_STATUS_INVALID_REQUEST;
		}
	}

	return NOTIFY_STATUS_INVALID_REQUEST;
}

void
service_close(svc_info_t *info)
{
	if (info == NULL) return;

	switch (info->type)
	{
		case SERVICE_TYPE_PATH_PUBLIC:
		case SERVICE_TYPE_PATH_PRIVATE:
		{
			path_node_close((path_node_t *)info->private);
			break;
		}
		case SERVICE_TYPE_TIMER_PUBLIC:
		case SERVICE_TYPE_TIMER_PRIVATE:
		{
			timer_close((timer_t *)info->private);
			break;
		}
		default:
		{
		}
	}

	free(info);
}

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

#include "notifyd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <asl.h>
#include <bsm/libbsm.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/mach_traps.h>
#include <sys/sysctl.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include <dispatch/private.h>
#include <assert.h>
#include "notify.h"
#include "service.h"
#include "notify_ipc.h"
#include "notify_ipcServer.h"
#include "notify_internal.h"
#include "notifyServer.h"
#include <sandbox.h>

static void
port_arm_mach_notifications(mach_port_t port)
{
	kern_return_t kr;
	mach_port_t previous;

	/* arm both DEAD_NAME & SEND_POSSIBLE */
	kr = mach_port_request_notification(mach_task_self(), port,
			MACH_NOTIFY_SEND_POSSIBLE, 0, global.mach_notify_port,
			MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous);
	if (kr != KERN_SUCCESS) {
		NOTIFY_INTERNAL_CRASH(kr, "Unable to arm send-possible");
	}
	if (previous != MACH_PORT_NULL) {
		NOTIFY_INTERNAL_CRASH(kr, "send-possible already armed??");
	}
}

static void
port_free(void *_pp)
{
	notify_state_t *ns = &global.notify_state;
	port_data_t *pdata = _pp;

	if (!LIST_EMPTY(&pdata->clients)) {
		NOTIFY_INTERNAL_CRASH(0, "port_proc still had clients");
	}

	_nc_table_delete_n(&ns->port_table, pdata->port);
	mach_port_deallocate(mach_task_self(), pdata->port);

	ns->stat_portproc_free++;
	free(pdata);
}

static void
proc_free(void *_pp)
{
	notify_state_t *ns = &global.notify_state;
	proc_data_t *pdata = _pp;

	if (!LIST_EMPTY(&pdata->clients)) {
		NOTIFY_INTERNAL_CRASH(0, "port_proc still had clients");
	}

	_nc_table_delete_n(&ns->proc_table, pdata->pid);

	ns->stat_portproc_free++;

	dispatch_release(pdata->src);
	free(pdata);
}

static void
proc_create(notify_state_t *ns, client_t *c, pid_t proc, dispatch_source_t src)
{
	proc_data_t *pdata = malloc(sizeof(proc_data_t));

	if (pdata == NULL) {
		// failing here is not an option, and our malloc will never fail
		// for such a small allocation
		NOTIFY_INTERNAL_CRASH(0, "Unable to allocate portproc");
	}

	ns->stat_portproc_alloc++;

	LIST_INIT(&pdata->clients);
	pdata->src = src;
	pdata->flags = PORT_PROC_FLAGS_NONE;
	pdata->pid = proc;
	_nc_table_insert_n(&ns->proc_table, &pdata->pid);
	LIST_INSERT_HEAD(&pdata->clients, c, client_pid_entry);

	dispatch_set_context(src, pdata);
	dispatch_source_set_cancel_handler_f(src, proc_free);
	dispatch_activate(src);
}

static void
port_create(notify_state_t *ns, client_t *c, mach_port_t port)
{
	port_data_t *pdata = calloc(1, sizeof(port_data_t));

	if (pdata == NULL) {
		// failing here is not an option, and our malloc will never fail
		// for such a small allocation
		NOTIFY_INTERNAL_CRASH(0, "Unable to allocate portproc");
	}

	ns->stat_portproc_alloc++;

	LIST_INIT(&pdata->clients);
	pdata->port = port;
	_nc_table_insert_n(&ns->port_table, &pdata->port);
	LIST_INSERT_HEAD(&pdata->clients, c, client_port_entry);

	mach_port_insert_right(mach_task_self(), port, port,
			MACH_MSG_TYPE_COPY_SEND);
	/* arming SEND_POSSIBLE must be done before we attempt any send */
	port_arm_mach_notifications(port);
}

static bool
proc_register(notify_state_t *ns, client_t *c, pid_t pid)
{
	proc_data_t *pdata = _nc_table_find_n(&ns->proc_table, pid);
	if (pdata) {
		LIST_INSERT_HEAD(&pdata->clients, c, client_pid_entry);
	}
	return pdata != NULL;
}

static bool
port_register(notify_state_t *ns, client_t *c, mach_port_t port)
{
	port_data_t *pdata = _nc_table_find_n(&ns->port_table, port);
	if (pdata) {
		LIST_INSERT_HEAD(&pdata->clients, c, client_port_entry);
	}
	return pdata != NULL;
}

static void
proc_resume(void *_pp)
{
	proc_data_t *pdata = _pp;
	client_t *c;

	pdata->flags &= ~NOTIFY_PORT_PROC_STATE_SUSPENDED;

	LIST_FOREACH(c, &pdata->clients, client_pid_entry) {
		_notify_lib_resume_client(&global.notify_state, c, pdata, NULL);
	}
}

/*
 * a.k.a. port_resume
 *
 * receiving a send-possible on a port
 * causes us to resume sending to it.
 */
kern_return_t
do_mach_notify_send_possible(mach_port_t notify, mach_port_name_t port)
{
	port_data_t *pdata;
	client_t *c;

	/* the notification was consumed, re-arm it */
	port_arm_mach_notifications(port);

	pdata = _nc_table_find_n(&global.notify_state.port_table, port);
	pdata->flags &= ~NOTIFY_PORT_PROC_STATE_SUSPENDED;

	LIST_FOREACH(c, &pdata->clients, client_port_entry) {
		_notify_lib_resume_client(&global.notify_state, c, NULL, pdata);
		/*
		 * If a send failed again, there's not point to keep trying,
		 * just wait for the send-possible to be issued.
		 */
		if (pdata->flags & NOTIFY_PORT_PROC_STATE_SUSPENDED) {
			break;
		}
	}

	return KERN_SUCCESS;
}


static void
port_proc_cancel_client(client_t *c)
{
	name_info_t *n;

	call_statistics.cleanup++;

	if (c->service_index != 0)
	{
		service_close(c->service_index);
		c->service_index = 0;
	}

	n = c->name_info;
	assert(n != NULL);
	if ((n->refcount == 1) && (n->private != NULL))
	{
		service_close(n->private);
		n->private = NULL;
	}

	if (c->state_and_type & NOTIFY_TYPE_MEMORY)
	{
		global.shared_memory_refcount[n->slot]--;
	}
	else if (c->state_and_type & NOTIFY_TYPE_PORT)
	{
		LIST_REMOVE(c, client_port_entry);
	}
	LIST_REMOVE(c, client_pid_entry);

	_notify_lib_cancel_client(&global.notify_state, c);
}

static inline void
proc_cancel(proc_data_t *pdata)
{
	client_t *c, *tmp;

	LIST_FOREACH_SAFE(c, &pdata->clients, client_pid_entry, tmp) {
		port_proc_cancel_client(c);
	}

	dispatch_source_cancel(pdata->src);
}

/*
 * a.k.a. port_cancel
 *
 * receiving a dead-name for a port causes us canceling it
 */
kern_return_t
do_mach_notify_dead_name(mach_port_t notify, mach_port_name_t port)
{
	port_data_t *pdata;
	client_t *c, *tmp;

	pdata = _nc_table_find_n(&global.notify_state.port_table, port);
	LIST_FOREACH_SAFE(c, &pdata->clients, client_port_entry, tmp) {
		port_proc_cancel_client(c);
	}
	port_free(pdata);

	// the act of receiving a dead name notification allocates a dead-name
	// right that must be deallocated
	mach_port_deallocate(mach_task_self(), port);
	return KERN_SUCCESS;
}


static void
proc_event(void *_pp)
{
	proc_data_t *pdata = _pp;

	proc_cancel(pdata);
}

static void
register_proc(client_t *c, pid_t pid)
{
	dispatch_source_t src;

	if (pid <= 0) return;

	if (proc_register(&global.notify_state, c, pid)) {
		return;
	}

	src = dispatch_source_create(DISPATCH_SOURCE_TYPE_PROC, pid,
			DISPATCH_PROC_EXIT, global.workloop);
	dispatch_source_set_event_handler_f(src, proc_event);

	proc_create(&global.notify_state, c, pid, src);
}

static void
register_port(client_t *c, mach_port_t port)
{
	if (port_register(&global.notify_state, c, port)) {
		return;
	}

	port_create(&global.notify_state, c, port);
}


static uint32_t
string_validate(caddr_t path, mach_msg_type_number_t pathCnt)
{
	if (path == NULL && pathCnt != 0) {
		return NOTIFY_STATUS_INVALID_REQUEST;
	}
	if (path && (pathCnt == 0 || path[pathCnt - 1] != '\0')) {
		return NOTIFY_STATUS_INVALID_REQUEST;
	}
	return NOTIFY_STATUS_OK;
}

static void
server_preflight(audit_token_t audit, int token, uid_t *uid, gid_t *gid, pid_t *pid, uint64_t *cid)
{
	pid_t xpid;

	if(uid != NULL)
	{
		*uid = audit_token_to_euid(audit);
	}

	if(gid != NULL)
	{
		*gid = audit_token_to_egid(audit);
	}

	xpid = audit_token_to_pid(audit);
	if (pid != NULL)
	{
		*pid = xpid;
	}

	if (token > 0)
	{
		client_t *c;
		uint64_t xcid = make_client_id(xpid, token);
		if (cid != NULL) *cid = xcid;

		c = _nc_table_find_64(&global.notify_state.client_table, xcid);
		if (c != NULL)
		{
			/* duplicate tokens can occur if a process exec()s */
			log_message(ASL_LEVEL_DEBUG, "duplicate token %d sent from PID %d\n", token, xpid);
			port_proc_cancel_client(c);
		}
	}
}

kern_return_t __notify_server_post_3
(
	mach_port_t server,
	uint64_t name_id,
	boolean_t claim_root_access,
	audit_token_t audit
)
{
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	pid_t pid = (pid_t)-1;
	int status;
	name_info_t *n;

	n = _nc_table_find_64(&global.notify_state.name_id_table, name_id);
	if (n == NULL)
	{
		return KERN_SUCCESS; // No one is registered for the name
	}

	n->postcount++;

	server_preflight(audit, -1, &uid, &gid, &pid, NULL);
	
	if ((uid != 0) && claim_root_access && has_root_entitlement(audit))
	{
		uid = 0;
	}

	status = _notify_lib_check_controlled_access(&global.notify_state, n->name, uid, gid, NOTIFY_ACCESS_WRITE);
	assert(status != NOTIFY_STATUS_NULL_INPUT);
	if (status != NOTIFY_STATUS_OK){
		return KERN_SUCCESS; // The poster does not have permission to post
	}

	call_statistics.post++;
	call_statistics.post_by_id++;

	log_message(ASL_LEVEL_DEBUG, "__notify_server_post %s %d by nameid: %llu \n", n->name, pid, name_id);

	status = daemon_post_nid(name_id, uid, gid);
	assert(status == NOTIFY_STATUS_OK);

	return KERN_SUCCESS;
}

kern_return_t __notify_server_post_2
(
	mach_port_t server,
	caddr_t name,
	uint64_t *name_id,
	int *status,
	boolean_t claim_root_access,
	audit_token_t audit
)
{
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	pid_t pid = (pid_t)-1;
	name_info_t *n;

	*name_id = 0;

	server_preflight(audit, -1, &uid, &gid, &pid, NULL);

	if ((uid != 0) && claim_root_access && has_root_entitlement(audit))
	{
		uid = 0;
	}

	*status = _notify_lib_check_controlled_access(&global.notify_state, name, uid, gid, NOTIFY_ACCESS_WRITE);
	if (*status != NOTIFY_STATUS_OK)
	{
		return KERN_SUCCESS;
	}

	call_statistics.post++;
	call_statistics.post_by_name_and_fetch_id++;
	
	n = NULL;
	*status = daemon_post(name, uid, gid);
	assert(*status != NOTIFY_STATUS_NULL_INPUT);

	n = _nc_table_find(&global.notify_state.name_table, name);

	if (n == NULL)
	{
		*status = NOTIFY_STATUS_INVALID_NAME;
		*name_id = UINT64_MAX;
		call_statistics.post_no_op++;
	}
	else
	{
		n->postcount++;
		*name_id = n->name_id;
	}

	if (*name_id == UINT64_MAX) log_message(ASL_LEVEL_DEBUG, "__notify_server_post %s %d\n", name, pid);
	else log_message(ASL_LEVEL_DEBUG, "__notify_server_post %s %d [%llu]\n", name, pid, *name_id);

	return KERN_SUCCESS;
}

kern_return_t __notify_server_post_4
(
	mach_port_t server,
	caddr_t name,
	boolean_t claim_root_access,
	audit_token_t audit
)
{
	uint64_t ignored_name_id;
	int ignored_status;
	kern_return_t kstatus;

	kstatus = __notify_server_post_2(server, name, &ignored_name_id, &ignored_status, claim_root_access, audit);

	call_statistics.post_by_name_and_fetch_id--;
	call_statistics.post_by_name++;

	return kstatus;
}

kern_return_t __notify_server_register_plain_2
(
	mach_port_t server,
	caddr_t name,
	int token,
	audit_token_t audit
)
{
	client_t *c;
	uint64_t nid, cid;
	uint32_t status;
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	pid_t pid = (pid_t)-1;

	call_statistics.reg++;
	call_statistics.reg_plain++;

	server_preflight(audit, token, &uid, &gid, &pid, &cid);

	log_message(ASL_LEVEL_DEBUG, "__notify_server_register_plain %s %d %d\n", name, pid, token);

	status = _notify_lib_register_plain(&global.notify_state, name, pid, token, -1, uid, gid, &nid);
	if (status != NOTIFY_STATUS_OK)
	{
		return KERN_SUCCESS;
	}

	c = _nc_table_find_64(&global.notify_state.client_table, cid);

	if (!strncmp(name, SERVICE_PREFIX, SERVICE_PREFIX_LEN)) service_open(name, c, audit);

	register_proc(c, pid);

	return KERN_SUCCESS;
}

kern_return_t __notify_server_register_check_2
(
	mach_port_t server,
	caddr_t name,
	int token,
	int *size,
	int *slot,
	uint64_t *name_id,
	int *status,
	audit_token_t audit
)
{
	name_info_t *n;
	uint32_t i, j, x, new_slot;
	uint64_t cid;
	client_t *c;
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	pid_t pid = (pid_t)-1;

	*size = 0;
	*slot = 0;
	*name_id = 0;
	*status = NOTIFY_STATUS_OK;

	server_preflight(audit, token, &uid, &gid, &pid, &cid);

	call_statistics.reg++;
	call_statistics.reg_check++;

	if (global.nslots == 0)
	{
		*size = -1;
		*slot = -1;
		return __notify_server_register_plain_2(server, name, token, audit);
	}

	x = (uint32_t)-1;

	n = _nc_table_find(&global.notify_state.name_table, name);
	if (n != NULL) x = n->slot;

	new_slot = 0;
	if (x == (uint32_t)-1) 
	{
		/* find a slot */
		new_slot = 1;

		/*
		 * Check slots beginning at the current slot_id + 1, since it's likely that the
		 * next slot will be available.  Keep looking until we have examined all the
		 * slots (skipping slot 0, which is reserved for notifyd). Stop if we find
		 * an unused (refcount == 0) slot.
		 */
		for (i = 1, j = global.slot_id + 1; i < global.nslots; i++, j++)
		{
			if (j >= global.nslots) j = 1;
			if (global.shared_memory_refcount[j] == 0)
			{
				x = j;
				break;
			}
		}

		if (x == (uint32_t)-1)
		{
			/*
			 * We did not find an unused slot.  At this point, the shared
			 * memory table is full, so we start re-using slots, beginning at 
			 * global.slot_id + 1.
			 */
			global.slot_id++;

			/* wrap around to slot 1 (slot 0 is reserved for notifyd) */
			if (global.slot_id >= global.nslots) global.slot_id = 1;
			log_message(ASL_LEVEL_DEBUG, "reused shared memory slot %u\n", global.slot_id);
			x = global.slot_id;
		}
		else
		{
			/* found a free slot */
			global.slot_id = x;
		}
	}

	if (new_slot == 1) global.shared_memory_base[x] = 1;
	global.shared_memory_refcount[x]++;

	log_message(ASL_LEVEL_DEBUG, "__notify_server_register_check %s %d %d\n", name, pid, token);

	*size = global.nslots * sizeof(uint32_t);
	*slot = x;
	*status = _notify_lib_register_plain(&global.notify_state, name, pid, token, x, uid, gid, name_id);
	if (*status != NOTIFY_STATUS_OK)
	{
		return KERN_SUCCESS;
	}

	c = _nc_table_find_64(&global.notify_state.client_table, cid);

	if (!strncmp(name, SERVICE_PREFIX, SERVICE_PREFIX_LEN)) service_open(name, c, audit);

	register_proc(c, pid);

	return KERN_SUCCESS;
}

kern_return_t __notify_server_register_signal_2
(
	mach_port_t server,
	caddr_t name,
	int token,
	int sig,
	audit_token_t audit
)
{
	client_t *c;
	uint64_t name_id, cid;
	uint32_t status;
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	pid_t pid = (pid_t)-1;

	server_preflight(audit, token, &uid, &gid, &pid, &cid);

	call_statistics.reg++;
	call_statistics.reg_signal++;

	log_message(ASL_LEVEL_DEBUG, "__notify_server_register_signal %s %d %d %d\n", name, pid, token, sig);

	status = _notify_lib_register_signal(&global.notify_state, name, pid, token, sig, uid, gid, &name_id);
	if (status != NOTIFY_STATUS_OK)
	{
		return KERN_SUCCESS;
	}

	c = _nc_table_find_64(&global.notify_state.client_table, cid);

	if (!strncmp(name, SERVICE_PREFIX, SERVICE_PREFIX_LEN)) service_open(name, c, audit);

	register_proc(c, pid);

	return KERN_SUCCESS;
}

kern_return_t __notify_server_register_file_descriptor_2
(
	mach_port_t server,
	caddr_t name,
	int token,
	fileport_t fileport,
	audit_token_t audit
)
{
	client_t *c;
	int fd, flags;
	uint32_t status;
	uint64_t name_id, cid;
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	pid_t pid = (pid_t)-1;

	server_preflight(audit, token, &uid, &gid, &pid, &cid);

	call_statistics.reg++;
	call_statistics.reg_file++;

	log_message(ASL_LEVEL_DEBUG, "__notify_server_register_file_descriptor %s %d %d\n", name, pid, token);

	fd = fileport_makefd(fileport);
	mach_port_deallocate(mach_task_self(), fileport);
	if (fd < 0)
	{
		return KERN_SUCCESS;
	}

	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
	{
		return KERN_SUCCESS;
	}

	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0)
	{
		return KERN_SUCCESS;
	}

	status = _notify_lib_register_file_descriptor(&global.notify_state, name, pid, token, fd, uid, gid, &name_id);
	if (status != NOTIFY_STATUS_OK)
	{
		return KERN_SUCCESS;
	}

	c = _nc_table_find_64(&global.notify_state.client_table, cid);

	if (!strncmp(name, SERVICE_PREFIX, SERVICE_PREFIX_LEN)) service_open(name, c, audit);

	register_proc(c, pid);

	return KERN_SUCCESS;
}

kern_return_t __notify_server_register_mach_port_2
(
	mach_port_t server,
	caddr_t name,
	int token,
	mach_port_t port,
	audit_token_t audit
)
{
	client_t *c;
	uint64_t name_id, cid;
	uint32_t status;
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	pid_t pid = (pid_t)-1;

	if (port == MACH_PORT_DEAD || port == MACH_PORT_NULL)
	{
		return KERN_SUCCESS;
	}

	server_preflight(audit, token, &uid, &gid, &pid, &cid);

	call_statistics.reg++;
	call_statistics.reg_port++;

	log_message(ASL_LEVEL_DEBUG, "__notify_server_register_mach_port %s %d %d\n", name, pid, token);

	status = _notify_lib_register_mach_port(&global.notify_state, name, pid, token, port, uid, gid, &name_id);
	if (status != NOTIFY_STATUS_OK)
	{
		mach_port_deallocate(mach_task_self(), port);
		return KERN_SUCCESS;
	}

	c = _nc_table_find_64(&global.notify_state.client_table,cid);

	if (!strncmp(name, SERVICE_PREFIX, SERVICE_PREFIX_LEN)) service_open(name, c, audit);

	register_proc(c, pid);
	register_port(c, port);

	return KERN_SUCCESS;
}

kern_return_t __notify_server_cancel_2
(
	mach_port_t server,
	int token,
	audit_token_t audit
)
{
	client_t *c;
	uid_t uid = (uid_t)-1;
	pid_t pid = (pid_t)-1;

	server_preflight(audit, -1, &uid, NULL, &pid, NULL);

	call_statistics.cancel++;

	log_message(ASL_LEVEL_DEBUG, "__notify_server_cancel %d %d\n", pid, token);

	c = _nc_table_find_64(&global.notify_state.client_table, make_client_id(pid, token));
	if (c != NULL) {
		port_proc_cancel_client(c);
	}

	return KERN_SUCCESS;
}

kern_return_t __notify_server_suspend
(
	mach_port_t server,
	int token,
	int *status,
	audit_token_t audit
)
{
	pid_t pid = (pid_t)-1;

	server_preflight(audit, -1, NULL, NULL, &pid, NULL);

	call_statistics.suspend++;

	log_message(ASL_LEVEL_DEBUG, "__notify_server_suspend %d %d\n", pid, token);

	*status = _notify_lib_suspend(&global.notify_state, pid, token);
	assert(*status != NOTIFY_STATUS_NULL_INPUT);

	return KERN_SUCCESS;
}

kern_return_t __notify_server_resume
(
	mach_port_t server,
	int token,
	int *status,
	audit_token_t audit
)
{
	pid_t pid = (pid_t)-1;

	server_preflight(audit, -1, NULL, NULL, &pid, NULL);

	call_statistics.resume++;

	log_message(ASL_LEVEL_DEBUG, "__notify_server_resume %d %d\n", pid, token);

	*status = _notify_lib_resume(&global.notify_state, pid, token);
	assert(*status != NOTIFY_STATUS_NULL_INPUT);

	return KERN_SUCCESS;
}

kern_return_t __notify_server_suspend_pid
(
	mach_port_t server,
	int pid,
	audit_token_t audit
)
{
	uid_t uid;
	proc_data_t *pdata;

	uid = audit_token_to_euid(audit);

	call_statistics.suspend_pid++;

	log_message(ASL_LEVEL_DEBUG, "__notify_server_suspend_pid %d\n", pid);

	if (uid != 0)
	{
		log_message(ASL_LEVEL_WARNING, "__notify_server_suspend_pid %d permission denied for caller %d \n", pid, audit_token_to_pid(audit));
		return KERN_SUCCESS;
	}

	pdata = _nc_table_find_n(&global.notify_state.proc_table, pid);
	if (pdata) {
		pdata->flags |= NOTIFY_PORT_PROC_STATE_SUSPENDED;
	}
	return KERN_SUCCESS;
}

kern_return_t __notify_server_resume_pid
(
	mach_port_t server,
	int pid,
	audit_token_t audit
)
{
	uid_t uid;
	proc_data_t *pdata;

	uid = audit_token_to_euid(audit);

	call_statistics.resume_pid++;

	log_message(ASL_LEVEL_DEBUG, "__notify_server_resume_pid %d\n", pid);

	if (uid != 0)
	{
		log_message(ASL_LEVEL_WARNING, "__notify_server_resume_pid %d permission denied for caller %d \n", pid, audit_token_to_pid(audit));
		return KERN_SUCCESS;
	}

	pdata = _nc_table_find_n(&global.notify_state.proc_table, pid);
	if (pdata) {
		proc_resume(pdata);
	}
	return KERN_SUCCESS;
}

kern_return_t __notify_server_check
(
	mach_port_t server,
	int token,
	int *check,
	int *status,
	audit_token_t audit
)
{
	pid_t pid = (gid_t)-1;

	*check = 0;

	call_statistics.check++;

	audit_token_to_au32(audit, NULL, NULL, NULL, NULL, NULL, &pid, NULL, NULL);

	log_message(ASL_LEVEL_DEBUG, "__notify_server_check %d %d\n", pid, token);

	*status =  _notify_lib_check(&global.notify_state, pid, token, check);
	assert(*status != NOTIFY_STATUS_NULL_INPUT);

	return KERN_SUCCESS;
}

kern_return_t __notify_server_get_state
(
	mach_port_t server,
	int token,
	uint64_t *state,
	int *status,
	audit_token_t audit
)
{
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	pid_t pid = (pid_t)-1;
	client_t *c;

	*state = 0;

	server_preflight(audit, -1, &uid, &gid, &pid, NULL);

	call_statistics.get_state++;
	call_statistics.get_state_by_client++;

	log_message(ASL_LEVEL_DEBUG, "__notify_server_get_state %d %d\n", pid, token);

	c = _nc_table_find_64(&global.notify_state.client_table, make_client_id(pid, token));
	if (c == NULL)
	{
		*status = NOTIFY_STATUS_CLIENT_NOT_FOUND;
	}
	else
	{
		*status = _notify_lib_get_state(&global.notify_state, c->name_info->name_id, state, uid, gid);
		assert(*status == NOTIFY_STATUS_OK || *status == NOTIFY_STATUS_NOT_AUTHORIZED);
	}

	return KERN_SUCCESS;
}

kern_return_t __notify_server_get_state_2
(
	mach_port_t server,
	uint64_t name_id,
	uint64_t *state,
	int *status,
	audit_token_t audit
)
{
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;

	*state = 0;

	server_preflight(audit, -1, &uid, &gid, NULL, NULL);

	log_message(ASL_LEVEL_DEBUG, "__notify_server_get_state_2 %llu\n", name_id);

	*status = _notify_lib_get_state(&global.notify_state, name_id, state, uid, gid);

	return KERN_SUCCESS;
}

kern_return_t __notify_server_get_state_3
(
	mach_port_t server,
	int token,
	uint64_t *state,
	uint64_t *name_id,
	int *status,
	audit_token_t audit
)
{
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	pid_t pid = (pid_t)-1;
	client_t *c;

	*state = 0;
	*name_id = 0;

	server_preflight(audit, -1, &uid, &gid, &pid, NULL);

	call_statistics.get_state++;
	call_statistics.get_state_by_client_and_fetch_id++;

	c = _nc_table_find_64(&global.notify_state.client_table, make_client_id(pid, token));
	if (c == NULL)
	{
		*status = NOTIFY_STATUS_CLIENT_NOT_FOUND;
		*name_id = UINT64_MAX;
	}
	else
	{
		*status = _notify_lib_get_state(&global.notify_state, c->name_info->name_id, state, uid, gid);
		assert(*status == NOTIFY_STATUS_OK || *status == NOTIFY_STATUS_NOT_AUTHORIZED);

		*name_id = c->name_info->name_id;
	}

	if (*name_id == UINT64_MAX) log_message(ASL_LEVEL_DEBUG, "__notify_server_get_state_3 %d %d\n", pid, token);
	else log_message(ASL_LEVEL_DEBUG, "__notify_server_get_state_3 %d %d [%llu]\n", pid, token, *name_id);

	return KERN_SUCCESS;
}

kern_return_t __notify_server_set_state_3
(
	mach_port_t server,
	int token,
	uint64_t state,
	uint64_t *name_id,
	int *status,
	boolean_t claim_root_access,
	audit_token_t audit
)
{
	client_t *c;
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	pid_t pid = (pid_t)-1;

	*name_id = 0;

	server_preflight(audit, -1, &uid, &gid, &pid, NULL);

	bool root_entitlement = uid != 0 && claim_root_access && has_root_entitlement(audit);
	if (root_entitlement) uid = 0;

	call_statistics.set_state++;
	call_statistics.set_state_by_client_and_fetch_id++;

	c = _nc_table_find_64(&global.notify_state.client_table, make_client_id(pid, token));
	if (c == NULL)
	{
		*status = NOTIFY_STATUS_CLIENT_NOT_FOUND;
		*name_id = UINT64_MAX;
	}
	else
	{
		assert(c->name_info != NULL);
		*status = _notify_lib_set_state(&global.notify_state, c->name_info->name_id, state, uid, gid);
		assert(*status == NOTIFY_STATUS_OK || *status == NOTIFY_STATUS_NOT_AUTHORIZED);

		*name_id = c->name_info->name_id; 
	}

	if (*name_id == UINT64_MAX) log_message(ASL_LEVEL_DEBUG, "__notify_server_set_state_3 %d %d %llu [uid %d%s gid %d]\n", pid, token, state, uid, root_entitlement ? " (entitlement)" : "", gid);
	else log_message(ASL_LEVEL_DEBUG, "__notify_server_set_state_3 %d %d %llu [%llu] [uid %d%s gid %d]\n", pid, token, state, *name_id, uid, root_entitlement ? " (entitlement)" : "", gid);

	return KERN_SUCCESS;
}

kern_return_t __notify_server_set_state_2
(
	mach_port_t server,
	uint64_t name_id,
	uint64_t state,
	boolean_t claim_root_access,
	audit_token_t audit
)
{
	uint32_t status;
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	pid_t pid = (pid_t)-1;

	call_statistics.set_state++;
	call_statistics.set_state_by_id++;

	audit_token_to_au32(audit, NULL, &uid, &gid, NULL, NULL, &pid, NULL, NULL);

	bool root_entitlement = (uid != 0) && claim_root_access && has_root_entitlement(audit);
	if (root_entitlement) uid = 0;

    status = _notify_lib_set_state(&global.notify_state, name_id, state, uid, gid);

	if(status == NOTIFY_STATUS_OK){
		log_message(ASL_LEVEL_DEBUG, "__notify_server_set_state_2 %d %llu %llu [uid %d%s gid %d]\n", pid, name_id, state, uid, root_entitlement ? " (entitlement)" : "", gid);
	}

	assert(status == NOTIFY_STATUS_OK || status == NOTIFY_STATUS_NOT_AUTHORIZED ||
	       status == NOTIFY_STATUS_INVALID_NAME);

	return KERN_SUCCESS;
}

kern_return_t __notify_server_monitor_file_2
(
	mach_port_t server,
	int token,
	caddr_t path,
	mach_msg_type_number_t pathCnt,
	int flags,
	audit_token_t audit
)
{
	client_t *c;
	name_info_t *n;
	uid_t uid = (uid_t)-1;
	gid_t gid = (gid_t)-1;
	pid_t pid = (pid_t)-1;
	uint32_t ubits = (uint32_t)flags;
	int status;

	status = string_validate(path, pathCnt);
	if (status != NOTIFY_STATUS_OK) return KERN_SUCCESS;

	server_preflight(audit, -1, &uid, &gid, &pid, NULL);

	call_statistics.monitor_file++;

	log_message(ASL_LEVEL_DEBUG, "__notify_server_monitor_file %d %d %s 0x%08x\n", pid, token, path, ubits);

	c = _nc_table_find_64(&global.notify_state.client_table, make_client_id(pid, token));
	if (c == NULL)
	{
		return KERN_SUCCESS;
	}

	n = c->name_info;
	assert(n != NULL);

	service_open_path_private(n->name, c, path, audit, ubits);

	return KERN_SUCCESS;
}

kern_return_t __notify_server_regenerate
(
	mach_port_t server,
	caddr_t name,
	int token,
	uint32_t reg_type,
	mach_port_t port,
	int sig,
	int prev_slot,
	uint64_t prev_state,
	uint64_t prev_time,
	caddr_t path,
	mach_msg_type_number_t pathCnt,
	int path_flags,
	int *new_slot,
	uint64_t *new_nid,
	int *status,
	audit_token_t audit
)
{
	kern_return_t kstatus;
	pid_t pid = (pid_t)-1;
	int size;
	name_info_t *n;
	client_t *c;
	uint64_t cid;

	*new_slot = 0;
	*new_nid = 0;
	*status = NOTIFY_STATUS_OK;

	*status = string_validate(path, pathCnt);
	if (*status != NOTIFY_STATUS_OK) return KERN_SUCCESS;

	call_statistics.regenerate++;

	audit_token_to_au32(audit, NULL, NULL, NULL, NULL, NULL, &pid, NULL, NULL);

	log_message(ASL_LEVEL_DEBUG, "__notify_server_regenerate %s %d %d %d %u %d %d %llu %s %d\n", name, pid, token, reg_type, port, sig, prev_slot, prev_state, path, path_flags);

	cid = make_client_id(pid, token);
	c = _nc_table_find_64(&global.notify_state.client_table, cid);
	if (c != NULL)
	{
		/* duplicate client - this means that a registration interleaved with regeneration; no need to regen */
		*status = NOTIFY_STATUS_DUP_CLIENT;
		return KERN_SUCCESS;
	}

	switch (reg_type)
	{
		case NOTIFY_TYPE_MEMORY:
		{
			/* prev_slot must be between 0 and global.nslots */
			if ((prev_slot < 0) || (prev_slot >= global.nslots))
			{
				*status = NOTIFY_STATUS_INVALID_REQUEST;
				return KERN_SUCCESS;
			}

			kstatus = __notify_server_register_check_2(server, name, token, &size, new_slot, new_nid, status, audit);
			if (*status == NOTIFY_STATUS_OK)
			{
				if ((*new_slot != UINT32_MAX) && (global.last_shm_base != NULL))
				{
					global.shared_memory_base[*new_slot] = global.shared_memory_base[*new_slot] + global.last_shm_base[prev_slot] - 1;
					global.last_shm_base[prev_slot] = 0;
				}
			}
			break;
		}
		case NOTIFY_TYPE_PLAIN:
		{
			kstatus = __notify_server_register_plain_2(server, name, token, audit);
			break;
		}
		case NOTIFY_TYPE_PORT:
		{
			kstatus = __notify_server_register_mach_port_2(server, name, token, port, audit);
			break;
		}
		case NOTIFY_TYPE_SIGNAL:
		{
			kstatus = __notify_server_register_signal_2(server, name, token, sig, audit);
			break;
		}
		case NOTIFY_TYPE_FILE: /* fall through */
		default:
		{
			/* can not regenerate this type */
			*status = NOTIFY_STATUS_TYPE_ISSUE;
			return KERN_SUCCESS;
		}
	}

	if (path != NULL)
	{
		__notify_server_monitor_file_2(server, token, path, pathCnt, path_flags, audit);
	}

	c = _nc_table_find_64(&global.notify_state.client_table, cid);
	if (c == NULL)
	{
		*status = NOTIFY_STATUS_CLIENT_NOT_FOUND;
	}
	else
	{
		*status = NOTIFY_STATUS_OK;
		n = c->name_info;
		*new_nid = n->name_id;
		if (prev_time > n->state_time) n->state = prev_state;
	}

	return KERN_SUCCESS;
}

kern_return_t __notify_server_checkin
(
	mach_port_t server,
	uint32_t *version,
	uint32_t *server_pid,
	int *status,
	audit_token_t audit
)
{
	pid_t pid = (pid_t)-1;

	call_statistics.checkin++;

	audit_token_to_au32(audit, NULL, NULL, NULL, NULL, NULL, &pid, NULL, NULL);

	log_message(ASL_LEVEL_DEBUG, "__notify_server_checkin %d\n", pid);
	*version = NOTIFY_IPC_VERSION;
	*server_pid = getpid();
	*status = NOTIFY_STATUS_OK;
	return KERN_SUCCESS;
}


kern_return_t __notify_server_dump
(
	mach_port_t server,
	fileport_t fileport,
	audit_token_t audit
)
{
	int fd;
	int flags;

	// Only processes with euid 0 can use this spi
	if (audit_token_to_euid(audit) != 0)
	{
		mach_port_deallocate(mach_task_self(), fileport);
		return KERN_SUCCESS;
	}

	// Things starting with "forbidden-" will be automatically denied for any
	// sandboxed process, so if this returns non-zero, then that means the
	// process is in a sandbox and thus can't use this api
	if (sandbox_check_by_audit_token(audit, "forbidden-remote-device-admin", SANDBOX_FILTER_NONE)) {
		mach_port_deallocate(mach_task_self(), fileport);
		return KERN_SUCCESS;
	}

	if (!has_entitlement(audit, NOTIFY_STATE_ENTITLEMENT)) {
		mach_port_deallocate(mach_task_self(), fileport);
		return KERN_SUCCESS;
	}

	fd = fileport_makefd(fileport);
	mach_port_deallocate(mach_task_self(), fileport);
	if (fd < 0)
	{
		return KERN_SUCCESS;
	}

	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
	{
		close(fd);
		return KERN_SUCCESS;
	}

	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0)
	{
		close(fd);
		return KERN_SUCCESS;
	}

	dump_status(STATUS_REQUEST_SHORT, fd);

	close(fd);

	return KERN_SUCCESS;
}

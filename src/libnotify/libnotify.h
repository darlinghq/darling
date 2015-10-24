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

#ifndef _LIBNOTIFY_H_
#define _LIBNOTIFY_H_

#include <pthread.h>
#include <mach/mach.h>
#include <dispatch/dispatch.h>
#include "table.h"

#include <TargetConditionals.h>

#if TARGET_IPHONE_SIMULATOR
extern const char *_notify_shm_id();
#define SHM_ID _notify_shm_id()
#else
#define SHM_ID "apple.shm.notification_center"
#endif

#define NOTIFY_IPC_VERSION_NAME "com.apple.system.notify.ipc_version"
#define NOTIFY_IPC_VERSION_NAME_LEN 35
#define NOTIFY_SERVICE_NAME "com.apple.system.notification_center"
#define NOTIFY_SERVICE_NAME_LEN 36

#define COMMON_PORT_KEY "com.apple.system.notify.common"

/* Notification types */
#define NOTIFY_TYPE_NONE   0x00000000
#define NOTIFY_TYPE_MEMORY 0x00000001
#define NOTIFY_TYPE_PLAIN  0x00000002
#define NOTIFY_TYPE_PORT   0x00000004
#define NOTIFY_TYPE_FILE   0x00000008
#define NOTIFY_TYPE_SIGNAL 0x00000010
#define NOTIFY_TYPE_MASK   0x000000ff
#define NOTIFY_FLAG_SELF   0x80000000
#define NOTIFY_FLAG_REGEN  0x40000000
#define NOTIFY_FLAG_RELEASE_SEND 0x20000000

/* Return values for notify_check() */
#define NOTIFY_CHECK_FALSE 0
#define NOTIFY_CHECK_TRUE 1
#define NOTIFY_CHECK_ERROR 2

/* Access control */
#define NOTIFY_ACCESS_READ   1
#define NOTIFY_ACCESS_WRITE  2

#define NOTIFY_ACCESS_OTHER_SHIFT 8
#define NOTIFY_ACCESS_GROUP_SHIFT 4
#define NOTIFY_ACCESS_USER_SHIFT  0

#define NOTIFY_ACCESS_DEFAULT 0x00000333
#define NOTIFY_ACCESS_USER_RW 0x00000003

/* Filesystem Services */
#define NOTIFY_SERVICE_FILE_STATUS_QUO 0x00
#define NOTIFY_SERVICE_FILE_ADD        0x01
#define NOTIFY_SERVICE_FILE_DELETE     0x02
#define NOTIFY_SERVICE_FILE_MODIFY     0x04
#define NOTIFY_SERVICE_FILE_ATTR       0x08

#define NOTIFY_SERVICE_DIR_FILE_ADD    0x10
#define NOTIFY_SERVICE_DIR_FILE_DELETE 0x20

#define NOTIFY_CLIENT_STATE_SUSPENDED 0x00000001
#define NOTIFY_CLIENT_STATE_PENDING   0x00000002
#define NOTIFY_CLIENT_STATE_TIMEOUT   0x00000004

#define NOTIFY_PORT_PROC_TYPE_PORT			0x00000010
#define NOTIFY_PORT_PROC_TYPE_PROC			0x00000020
#define NOTIFY_PORT_PROC_TYPE_MASK			0x000000f0
#define NOTIFY_PORT_PROC_STATE_INVALID		0x00000001
#define NOTIFY_PORT_PROC_STATE_SUSPENDED	0x00000002
#define NOTIFY_PORT_PROC_STATE_MASK			0x0000000f

/* notify state flags */
#define NOTIFY_STATE_USE_LOCKS 0x00000001
#define NOTIFY_STATE_ENABLE_RESEND 0x00000002

#define NOTIFY_CLIENT_SELF 0
#define SIGNAL_NONE -1
#define FD_NONE -1
#define SLOT_NONE -1

#define _notify_lib_port_new(A,B,C,D) _notify_lib_port_proc_new(A,B,0,C,D)
#define _notify_lib_proc_new(A,B,C,D) _notify_lib_port_proc_new(A,MACH_PORT_NULL,B,C,D)
#define _notify_lib_port_find(A,B) _notify_lib_port_proc_find(A,B,0)
#define _notify_lib_proc_find(A,B) _notify_lib_port_proc_find(A,MACH_PORT_NULL,B)
#define _notify_lib_port_release(A,B) _notify_lib_port_proc_release(A,B,0)
#define _notify_lib_proc_release(A,B) _notify_lib_port_proc_release(A,MACH_PORT_NULL,B)

typedef struct
{
	char *name;
	uint64_t name_id;
	uint32_t uid;
	uint32_t gid;
	uint32_t access;
	uint32_t slot;
	uint32_t refcount;
	uint32_t val;
	uint64_t state;
	uint64_t state_time;
	void *private;
	list_t *subscriptions;
} name_info_t;

typedef struct
{
	uint64_t client_id;
	uint32_t state;
	name_info_t *name_info;
	uint32_t suspend_count;
	uint32_t notify_type;
	uint32_t lastval;
	mach_port_t port;
	int fd;
	uint32_t send_val;
	uint32_t pid;
	uint32_t sig;
	void *private;
} client_t;

typedef struct
{
	uint32_t refcount;
	uint32_t flags;
	dispatch_source_t src;
} portproc_data_t;

typedef struct
{
	uint32_t flags;
	table_t *name_table;
	table_t *name_id_table;
	table_t *client_table;
	table_t *port_table;
	table_t *proc_table;
	name_info_t **controlled_name;
	uint32_t controlled_name_count;
	pthread_mutex_t *lock;
	int sock;
	uint32_t stat_name_alloc;
	uint32_t stat_name_free;
	uint32_t stat_client_alloc;
	uint32_t stat_client_free;
	uint32_t stat_portproc_alloc;
	uint32_t stat_portproc_free;
} notify_state_t;

notify_state_t *_notify_lib_notify_state_new(uint32_t flags, uint32_t table_size);
void _notify_lib_notify_state_free(notify_state_t *ns);

uint32_t _notify_lib_post(notify_state_t *ns, const char *name, uint32_t uid, uint32_t gid);
uint32_t _notify_lib_post_nid(notify_state_t *ns, uint64_t nid, uid_t uid, gid_t gid);
uint32_t _notify_lib_post_client(notify_state_t *ns, client_t *c);

uint32_t _notify_lib_check(notify_state_t *ns, pid_t pid, int token, int *check);
uint32_t _notify_lib_get_state(notify_state_t *ns, uint64_t nid, uint64_t *state, uint32_t uid, uint32_t gid);
uint32_t _notify_lib_set_state(notify_state_t *ns, uint64_t nid, uint64_t state, uint32_t uid, uint32_t gid);

uint32_t _notify_lib_register_plain(notify_state_t *ns, const char *name, pid_t pid, int token, uint32_t slot, uint32_t uid, uint32_t gid, uint64_t *out_nid);
uint32_t _notify_lib_register_signal(notify_state_t *ns, const char *name, pid_t pid, int token, uint32_t sig, uint32_t uid, uint32_t gid, uint64_t *out_nid);
uint32_t _notify_lib_register_mach_port(notify_state_t *ns, const char *name, pid_t pid, int token, mach_port_t port, uint32_t uid, uint32_t gid, uint64_t *out_nid);
uint32_t _notify_lib_register_file_descriptor(notify_state_t *ns, const char *name, pid_t pid, int token, int fd, uint32_t uid, uint32_t gid, uint64_t *out_nid);

uint32_t _notify_lib_get_owner(notify_state_t *ns, const char *name, uint32_t *uid, uint32_t *gid);
uint32_t _notify_lib_get_access(notify_state_t *ns, const char *name, uint32_t *access);

uint32_t _notify_lib_set_owner(notify_state_t *ns, const char *name, uint32_t uid, uint32_t gid);
uint32_t _notify_lib_set_access(notify_state_t *ns, const char *name, uint32_t access);

uint32_t _notify_lib_release_name(notify_state_t *ns, const char *name, uint32_t uid, uint32_t gid);

void _notify_lib_cancel(notify_state_t *ns, pid_t pid, int token);
void _notify_lib_suspend(notify_state_t *ns, pid_t pid, int token);
uint32_t _notify_lib_resume(notify_state_t *ns, pid_t pid, int token);

void _notify_lib_cancel_proc(notify_state_t *ns, pid_t pid);
void _notify_lib_suspend_proc(notify_state_t *ns, pid_t pid);
void _notify_lib_resume_proc(notify_state_t *ns, pid_t pid);

void _notify_lib_suspend_port(notify_state_t *ns, mach_port_t port);
void _notify_lib_resume_port(notify_state_t *ns, mach_port_t port);

uint32_t _notify_lib_check_controlled_access(notify_state_t *ns, char *name, uid_t uid, gid_t gid, int req);

uint64_t make_client_id(pid_t pid, int token);

uint32_t _notify_lib_port_proc_new(notify_state_t *ns, mach_port_t port, pid_t proc, uint32_t state, dispatch_source_t src);
portproc_data_t *_notify_lib_port_proc_find(notify_state_t *ns, mach_port_t port, pid_t proc);
void _notify_lib_port_proc_release(notify_state_t *ns, mach_port_t port, pid_t proc);


#endif /* _LIBNOTIFY_H_ */

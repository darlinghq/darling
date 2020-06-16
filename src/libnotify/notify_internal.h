/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#define __OS_EXPOSE_INTERNALS__ 1
#include <os/internal/internal_shared.h>

#include <dispatch/dispatch.h>
#include <mach/mach.h>
#include <os/lock.h>
#include <stdatomic.h>
#include <stdint.h>
#include <TargetConditionals.h>

#include "libnotify.h"

#define NOTIFY_INTERNAL_CRASH(c, x) __extension__({ \
		_os_set_crash_log_cause_and_message(c, "BUG IN LIBNOTIFY: " x); \
		__builtin_trap(); \
	})

#define NOTIFY_CLIENT_CRASH(c, x) __extension__({ \
		_os_set_crash_log_cause_and_message(c, \
				"BUG IN CLIENT OF LIBNOTIFY: " x); \
		__builtin_trap(); \
	})

#define NOTIFY_STATUS_SERVER_CHECKIN_FAILED 11
// was  NOTIFY_STATUS_LIB_SELF_STATE_FAILED 12
#define NOTIFY_STATUS_SERVER_REGEN_FAILED 13
#define NOTIFY_STATUS_CLIENT_REG_FAILED 14
#define NOTIFY_STATUS_SERVER_POST_4_FAILED 15
#define NOTIFY_STATUS_SERVER_POST_2_FAILED 16
#define NOTIFY_STATUS_SERVER_POST_3_FAILED 17
#define NOTIFY_STATUS_TOKEN_NOT_FOUND 18
#define NOTIFY_STATUS_COMMON_PORT_NULL 19
#define NOTIFY_STATUS_SERVER_PORT_NULL 20
#define NOTIFY_STATUS_REG_CHECK_2_FAILED 21
#define NOTIFY_STATUS_SHM_ATTACH_FAILED 22
#define NOTIFY_STATUS_SHM_BASE_REMAINS_NULL 23
#define NOTIFY_STATUS_REG_PLAIN_2_FAILED 24
#define NOTIFY_STATUS_REG_SIGNAL_2_FAILED 25
#define NOTIFY_STATUS_MACH_PORT_ALLOC_FAILED 26
#define NOTIFY_STATUS_MACH_PORT_INSERT_RIGHT_FAILED 27
#define NOTIFY_STATUS_REG_MACH_PORT_2_FAILED 28
#define NOTIFY_STATUS_PIPE_FAILED 29
#define NOTIFY_STATUS_FILEPORT_MAKEPORT_FAILED 30
#define NOTIFY_STATUS_REG_FD_2_FAILED 31
#define NOTIFY_STATUS_SHM_BASE_NULL 32
#define NOTIFY_STATUS_SERVER_CHECK_FAILED 33
#define NOTIFY_STATUS_STRDUP_FAILED 34
#define NOTIFY_STATUS_SERVER_MONITOR_FILE_2_FAILED 35
#define NOTIFY_STATUS_SERVER_GET_STATE_2_FAILED 36
#define NOTIFY_STATUS_SERVER_SET_STATE_2_FAILED 37
#define NOTIFY_STATUS_SERVER_SUSPEND_FAILED 38
#define NOTIFY_STATUS_SERVER_RESUME_FAILED 39
#define NOTIFY_STATUS_SERVER_SUSPEND_PID_FAILED 40
#define NOTIFY_STATUS_SERVER_RESUME_PID_FAILED 41
#define NOTIFY_STATUS_ALLOC_FAILED 42
#define NOTIFY_STATUS_KILL_FAILED 43
#define NOTIFY_STATUS_WRITE_FAILED 44
#define NOTIFY_STATUS_MACH_MSG_TIMEOUT 45
#define NOTIFY_STATUS_MACH_MSG_FAILED 46
#define NOTIFY_STATUS_NEW_NAME_FAILED 47
#define NOTIFY_STATUS_NEW_CLIENT_FAILED 48
#define NOTIFY_STATUS_STATE_NULL 49
#define NOTIFY_STATUS_CLIENT_NOT_FOUND 50
#define NOTIFY_STATUS_DUP_CLIENT 51
#define NOTIFY_STATUS_TYPE_ISSUE 52
#define NOTIFY_STATUS_PATH_NODE_CREATE_FAILED 53
#define NOTIFY_STATUS_INVALID_TIME_EVENT 54
#define NOTIFY_STATUS_TIMER_FAILED 55
#define NOTIFY_STATUS_DOUBLE_REG 56
#define NOTIFY_STATUS_NO_REGEN_NEEDED 57

#define IS_INTERNAL_ERROR(X) (X >= 11)

#define USER_PROTECTED_UID_PREFIX "user.uid."
#define USER_PROTECTED_UID_PREFIX_LEN 9

struct notify_globals_s
{
	/* global lock */
	os_unfair_lock notify_lock;

	/* notify_check() lock */
	os_unfair_lock check_lock;
	pid_t notify_server_pid;

	atomic_uint_fast32_t client_opts;
	uint32_t saved_opts;

	notify_state_t self_state;

	dispatch_once_t notify_server_port_once;
	mach_port_t notify_server_port;
	mach_port_t saved_server_port;

	mach_port_t notify_common_port;
	int notify_common_token;
	dispatch_source_t notify_dispatch_source;
	dispatch_source_t server_proc_source;

	dispatch_once_t internal_once;
	table_n_t registration_table;
	table_t name_node_table;
	atomic_uint_fast32_t token_id;

	dispatch_once_t make_background_send_queue_once;
	dispatch_queue_t background_send_queue;

	/* file descriptor list */
	uint32_t fd_count;
	int *fd_clnt;
	int *fd_srv;
	int *fd_refcount;

	/* mach port list */
	uint32_t mp_count;
	uint32_t mp_size;
	struct mp_entry {
		mach_port_t mpl_port_name;
		int         mpl_refs;
		bool        mpl_mine;
	} *mp_list;

	/* shared memory base address */
	uint32_t *shm_base;
};

typedef struct notify_globals_s *notify_globals_t;

__private_extern__ uint32_t _notify_lib_peek(notify_state_t *ns, pid_t pid, int token, int *val);


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

#include <dispatch/dispatch.h>
#include <mach/mach.h>
#include <pthread.h>
#include <stdint.h>
#include <TargetConditionals.h>

struct notify_globals_s {
	// Global lock.
	pthread_mutex_t notify_lock;

	int32_t notify_ipc_version;
	pid_t notify_server_pid;

	uint32_t client_opts;
	uint32_t saved_opts;

	// Last allocated name id.
	uint64_t name_id;

	dispatch_once_t self_state_once;
	notify_state_t *self_state;

	dispatch_once_t notify_server_port_once;
	mach_port_t notify_server_port;
	mach_port_t saved_server_port;
	
	mach_port_t notify_common_port;
	int notify_common_token;
	dispatch_source_t notify_dispatch_source;
	dispatch_source_t server_proc_source;
	
	dispatch_once_t token_table_once;
	table_t *token_table;
	table_t *token_name_table;
	uint32_t token_id;
	
	// File descriptor list.
	uint32_t fd_count;
	int *fd_clnt;
	int *fd_srv;
	int *fd_refcount;
	
	// Mach port list.
	uint32_t mp_count;
	mach_port_t *mp_list;
	int *mp_refcount;
	int *mp_mine;
	
	// Shared memory base address.
	uint32_t *shm_base;
};
typedef struct notify_globals_s *notify_globals_t;

#if __has_include(<os/alloc_once_private.h>)
#include <os/alloc_once_private.h>
#if defined(OS_ALLOC_ONCE_KEY_LIBSYSTEM_NOTIFY)
#define _NOTIFY_HAS_ALLOC_ONCE 1
#endif
#endif

__attribute__((visibility("hidden")))
void _notify_init_globals(void * /* notify_globals_t */ globals);

__attribute__((visibility("hidden")))
notify_globals_t _notify_globals_impl(void);

__attribute__((__pure__))
static inline notify_globals_t
_notify_globals(void) {
#if _NOTIFY_HAS_ALLOC_ONCE
	return (notify_globals_t)os_alloc_once(OS_ALLOC_ONCE_KEY_LIBSYSTEM_NOTIFY,
		sizeof(struct notify_globals_s), &_notify_init_globals);
#else
	return _notify_globals_impl();
#endif
}

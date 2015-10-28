/*
 * Copyright (c) 2007-2012 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __LAUNCH_INTERNAL_H__
#define __LAUNCH_INTERNAL_H__

#include "vproc_priv.h"

#include <paths.h>
#include <dispatch/dispatch.h>
#include <pthread.h>

#if __has_include(<os/alloc_once_private.h>)
#include <os/alloc_once_private.h>
#if defined(OS_ALLOC_ONCE_KEY_LIBLAUNCH)
#define _LIBLAUNCH_HAS_ALLOC_ONCE 1
#endif
#endif

typedef struct _launch *launch_t;

struct launch_globals_s {
	// liblaunch.c
	pthread_once_t lc_once;
	pthread_mutex_t lc_mtx;
	launch_t l;
	launch_data_t async_resp;
	
	launch_t in_flight_msg_recv_client;
	
	int64_t s_am_embedded_god;
	
	// libvproc.c
	dispatch_queue_t _vproc_gone2zero_queue;
	_vproc_transaction_callout _vproc_gone2zero_callout;
	void *_vproc_gone2zero_ctx;
	
	dispatch_once_t _vproc_transaction_once;
	uint64_t _vproc_transaction_enabled;
	dispatch_queue_t _vproc_transaction_queue;
	int64_t _vproc_transaction_cnt;
};
typedef struct launch_globals_s *launch_globals_t;

void _launch_init_globals(launch_globals_t globals);

#if !_LIBLAUNCH_HAS_ALLOC_ONCE
launch_globals_t _launch_globals_impl(void);
#endif

__attribute__((__pure__))
static inline launch_globals_t
_launch_globals(void) {
#if _LIBLAUNCH_HAS_ALLOC_ONCE
	return (launch_globals_t)os_alloc_once(OS_ALLOC_ONCE_KEY_LIBLAUNCH,
					       sizeof(struct launch_globals_s),
					       (void*)&_launch_init_globals);
#else
	return _launch_globals_impl();
#endif
}

#pragma GCC visibility push(default)

#define LAUNCHD_DB_PREFIX "/private/var/db/launchd.db"
#define LAUNCHD_LOG_PREFIX "/private/var/log"

#define LAUNCHD_JOB_DEFAULTS "Defaults"
#define LAUNCHD_JOB_DEFAULTS_CACHED "CachedDefaults"

launch_t launchd_fdopen(int, int);
int launchd_getfd(launch_t);
void launchd_close(launch_t, __typeof__(close) closefunc);

launch_data_t launch_data_new_errno(int);
bool launch_data_set_errno(launch_data_t, int);

int launchd_msg_send(launch_t, launch_data_t);
int launchd_msg_recv(launch_t, void (*)(launch_data_t, void *), void *);

size_t launch_data_pack(launch_data_t d, void *where, size_t len, int *fd_where, size_t *fdslotsleft);
launch_data_t launch_data_unpack(void *data, size_t data_size, int *fds, size_t fd_cnt, size_t *data_offset, size_t *fdoffset);

#pragma GCC visibility pop

#endif /*  __LAUNCH_INTERNAL_H__*/

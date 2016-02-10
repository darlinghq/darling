/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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

#ifndef __LAUNCHD_IPC_H__
#define __LAUNCHD_IPC_H__

#include <sys/queue.h>
#include "runtime.h"
#include "core.h"
#include "launch_priv.h"
#include "launch_internal.h"

struct conncb {
	kq_callback kqconn_callback;
	LIST_ENTRY(conncb) sle;
	launch_t conn;
	job_t j;
};

extern char *sockpath;

void ipc_open(int fd, job_t j);
void ipc_close_all_with_job(job_t j);
void ipc_close(struct conncb *c);
void ipc_callback(void *, struct kevent *);
void ipc_revoke_fds(launch_data_t o);
void ipc_close_fds(launch_data_t o);
void ipc_server_init(void);

#endif /* __LAUNCHD_IPC_H__ */

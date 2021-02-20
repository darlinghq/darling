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

#ifndef __LAUNCHD_CORE_LOGIC__
#define __LAUNCHD_CORE_LOGIC__

#include "runtime.h"
#include "bootstrap.h"
#include "launch.h"
#include <xpc/private.h>

typedef struct job_s *job_t;
typedef struct jobmgr_s *jobmgr_t;

extern jobmgr_t root_jobmgr;
extern mach_port_t launchd_audit_port;
extern au_asid_t launchd_audit_session;
extern bool launchd_flat_mach_namespace;
extern bool launchd_embedded_handofgod;

void jobmgr_init(bool);
jobmgr_t jobmgr_shutdown(jobmgr_t jm);
void jobmgr_dispatch_all_semaphores(jobmgr_t jm);
void jobmgr_dispatch_all_interested(jobmgr_t jm, job_t j);
jobmgr_t jobmgr_delete_anything_with_port(jobmgr_t jm, mach_port_t port);

launch_data_t job_export_all(void);

job_t job_dispatch(job_t j, bool kickstart); /* returns j on success, NULL on job removal */
job_t job_find(jobmgr_t jm, const char *label);
job_t job_find_by_service_port(mach_port_t p);
bool job_ack_port_destruction(mach_port_t p);
bool job_is_anonymous(job_t j);
launch_data_t job_export(job_t j);
void job_stop(job_t j);
void job_checkin(job_t j);
void job_remove(job_t j);
bool job_is_god(job_t j);
job_t job_import(launch_data_t pload);
launch_data_t job_import_bulk(launch_data_t pload);
job_t job_mig_intran(mach_port_t mp);
void job_mig_destructor(job_t j);
void job_ack_no_senders(job_t j);
void job_log(job_t j, int pri, const char *msg, ...) __attribute__((format(printf, 3, 4)));
void job_set_pid_crashed(pid_t p);

bool xpc_event_demux(mach_port_t p, xpc_object_t request, xpc_object_t *reply);
bool xpc_process_demux(mach_port_t p, xpc_object_t request, xpc_object_t *reply);

#endif /* __LAUNCHD_CORE_LOGIC__ */

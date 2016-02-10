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

#ifndef __LAUNCHD_RUNTIME_H__
#define __LAUNCHD_RUNTIME_H__

#include <xpc/xpc.h>
#include <mach/mach.h>
#include <sys/types.h>
#include <bsm/libbsm.h>
#include <stdbool.h>
#include <stdint.h>
#include <float.h>
#include <syslog.h>

#include "kill2.h"
#include "ktrace.h"
#include "log.h"

#define	likely(x)	__builtin_expect((bool)(x), true)
#define	unlikely(x)	__builtin_expect((bool)(x), false)

struct ldcred {
	uid_t euid;
	uid_t uid;
	gid_t egid;
	gid_t gid;
	pid_t pid;
	au_asid_t asid;
	mach_port_t asport;
};

typedef void (*kq_callback)(void *, struct kevent *);
typedef boolean_t (*mig_callback)(mach_msg_header_t *, mach_msg_header_t *);
typedef void (*timeout_callback)(void);

extern bool launchd_verbose_boot;
/* Configuration knobs set in do_file_init(). */
extern bool launchd_shutdown_debugging;
extern bool launchd_use_gmalloc;
extern bool launchd_malloc_log_stacks;
extern bool launchd_log_shutdown;
extern bool launchd_log_debug;
extern bool launchd_log_perf;
extern bool launchd_trap_sigkill_bugs;
extern bool launchd_no_jetsam_perm_check;
extern bool launchd_osinstaller;
extern bool launchd_allow_global_dyld_envvars;
#if TARGET_OS_EMBEDDED
extern bool launchd_appletv;
#endif

extern bool launchd_runtime_busy_time;
extern mach_port_t inherited_bootstrap_port;
extern size_t runtime_busy_cnt;
extern int32_t launchd_sync_frequency;
extern pid_t launchd_wsp;

mach_port_t runtime_get_kernel_port(void);
extern boolean_t launchd_internal_demux(mach_msg_header_t *Request, mach_msg_header_t *Reply);

void runtime_add_ref(void);
void runtime_del_ref(void);
void runtime_add_weak_ref(void);
void runtime_del_weak_ref(void);
void runtime_install_timer(void);
void runtime_remove_timer(void);

void launchd_runtime_init(void);
void launchd_runtime_init2(void);
void launchd_runtime(void) __attribute__((noreturn));

void launchd_log_vm_stats(void);

int runtime_close(int fd);
int runtime_fsync(int fd);

#define RUNTIME_ADVISABLE_IDLE_TIMEOUT 30

void runtime_set_timeout(timeout_callback to_cb, unsigned int sec);
kern_return_t runtime_add_mport(mach_port_t name, mig_callback demux);
kern_return_t runtime_remove_mport(mach_port_t name);
void runtime_record_caller_creds(audit_token_t *token);
struct ldcred *runtime_get_caller_creds(void);
audit_token_t *runtime_get_caller_token(void);

const char *signal_to_C_name(unsigned int sig);
const char *reboot_flags_to_C_names(unsigned int flags);

int kevent_bulk_mod(struct kevent *kev, size_t kev_cnt);
int kevent_mod(uintptr_t ident, short filter, u_short flags, u_int fflags, intptr_t data, void *udata);
void log_kevent_struct(int level, struct kevent *kev_base, int indx);

pid_t runtime_fork(mach_port_t bsport);

mach_msg_return_t launchd_exc_runtime_once(mach_port_t port, mach_msg_size_t rcv_msg_size, mach_msg_size_t send_msg_size, mig_reply_error_t *bufRequest, mig_reply_error_t *bufReply, mach_msg_timeout_t to);

int64_t runtime_get_wall_time(void) __attribute__((warn_unused_result));
uint64_t runtime_get_opaque_time(void) __attribute__((warn_unused_result));
uint64_t runtime_get_opaque_time_of_event(void) __attribute__((pure, warn_unused_result));
uint64_t runtime_opaque_time_to_nano(uint64_t o) __attribute__((const, warn_unused_result));
uint64_t runtime_get_nanoseconds_since(uint64_t o) __attribute__((pure, warn_unused_result));

kern_return_t launchd_set_bport(mach_port_t name);
kern_return_t launchd_get_bport(mach_port_t *name);
kern_return_t launchd_mport_notify_req(mach_port_t name, mach_msg_id_t which);
kern_return_t launchd_mport_create_recv(mach_port_t *name);
kern_return_t launchd_mport_deallocate(mach_port_t name);
kern_return_t launchd_mport_make_send(mach_port_t name);
kern_return_t launchd_mport_copy_send(mach_port_t name);
kern_return_t launchd_mport_make_send_once(mach_port_t name, mach_port_t *so);
kern_return_t launchd_mport_close_recv(mach_port_t name);

uint64_t runtime_get_uniqueid(void);

#endif /* __LAUNCHD_RUNTIME_H__ */

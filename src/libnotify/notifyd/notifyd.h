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

#ifndef _NOTIFY_DAEMON_H_
#define _NOTIFY_DAEMON_H_

#define DISPATCH_MACH_SPI 1

#include <libnotify.h>
#include <mach/mach.h>
#include <launch.h>
#include <dispatch/dispatch.h>
#include <dispatch/private.h>


#define STATUS_REQUEST_SHORT 0
#define STATUS_REQUEST_LONG 1

#define NOTIFY_STATE_ENTITLEMENT "com.apple.private.libnotify.statecapture"


struct global_s
{
	notify_state_t notify_state;
	dispatch_mach_t mach_notifs_channel;
	mach_port_t mach_notify_port;
	mach_port_t server_port;
	void **service_info_list;
	dispatch_workloop_t workloop;
	dispatch_mach_t mach_channel;
	dispatch_source_t sig_usr1_src;
	dispatch_source_t sig_usr2_src;
	dispatch_source_t sig_winch_src;
	dispatch_source_t stat_reset_src;
	time_t last_reset_time;
	uint32_t nslots;
	uint32_t slot_id;
	uint32_t *shared_memory_base;
	uint32_t *shared_memory_refcount;
	uint32_t *last_shm_base;
	uint32_t log_cutoff;
	uint32_t log_default;
	uint16_t service_info_count;
	char *log_path;
};

extern struct global_s global;

struct call_statistics_s
{
	uint64_t post;
	uint64_t post_no_op;
	uint64_t post_by_id;
	uint64_t post_by_name;
	uint64_t post_by_name_and_fetch_id;
	uint64_t reg;
	uint64_t reg_plain;
	uint64_t reg_check;
	uint64_t reg_signal;
	uint64_t reg_file;
	uint64_t reg_port;
	uint64_t cancel;
	uint64_t suspend;
	uint64_t resume;
	uint64_t suspend_pid;
	uint64_t resume_pid;
	uint64_t check;
	uint64_t get_state;
	uint64_t get_state_by_client;
	uint64_t get_state_by_id;
	uint64_t get_state_by_client_and_fetch_id;
	uint64_t set_state;
	uint64_t set_state_by_client;
	uint64_t set_state_by_id;
	uint64_t set_state_by_client_and_fetch_id;
	uint64_t set_owner;
	uint64_t set_access;
	uint64_t monitor_file;
	uint64_t service_timer;
	uint64_t service_path;
	uint64_t cleanup;
	uint64_t regenerate;
	uint64_t checkin;
};

extern struct call_statistics_s call_statistics;

extern void log_message(int priority, const char *str, ...) __printflike(2, 3);
extern uint32_t daemon_post(const char *name, uint32_t u, uint32_t g);
extern uint32_t daemon_post_nid(uint64_t nid, uint32_t u, uint32_t g);
extern void daemon_post_client(uint64_t cid);
extern void daemon_set_state(const char *name, uint64_t val);
extern void dump_status(uint32_t level, int fd);
extern bool has_entitlement(audit_token_t audit, const char *entitlement);
extern bool has_root_entitlement(audit_token_t audit);

dispatch_queue_t get_notifyd_workloop(void);

#endif /* _NOTIFY_DAEMON_H_ */

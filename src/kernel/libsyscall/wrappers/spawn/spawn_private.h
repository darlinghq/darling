/*
 * Copyright (c) 2006, 2008 Apple,Inc. All rights reserved.
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

#ifndef _SPAWN_PRIVATE_H_
#define _SPAWN_PRIVATE_H_

#include <spawn.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <Availability.h>
#include <TargetConditionals.h>

__BEGIN_DECLS

int     posix_spawnattr_getpcontrol_np(const posix_spawnattr_t * __restrict, int * __restrict) __API_AVAILABLE(macos(10.6), ios(3.2));
int     posix_spawnattr_setpcontrol_np(posix_spawnattr_t *, const int) __API_AVAILABLE(macos(10.6), ios(3.2));

int     posix_spawnattr_getprocesstype_np(const posix_spawnattr_t * __restrict, int * __restrict) __API_AVAILABLE(macos(10.8), ios(6.0));
int     posix_spawnattr_setprocesstype_np(posix_spawnattr_t *, const int) __API_AVAILABLE(macos(10.8), ios(6.0));

int     posix_spawnattr_setcpumonitor(posix_spawnattr_t * __restrict, uint64_t, uint64_t) __API_AVAILABLE(macos(10.8), ios(6.0));
int     posix_spawnattr_getcpumonitor(posix_spawnattr_t * __restrict, uint64_t *, uint64_t *) __API_AVAILABLE(macos(10.8), ios(6.0));
int     posix_spawnattr_setcpumonitor_default(posix_spawnattr_t * __restrict) __API_AVAILABLE(macos(10.9), ios(6.0));

#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
int     posix_spawnattr_setjetsam(posix_spawnattr_t * __restrict attr,
    short flags, int priority, int memlimit) __API_UNAVAILABLE(macos) __API_AVAILABLE(ios(5.0));
#endif /* (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR) */

int     posix_spawnattr_setjetsam_ext(posix_spawnattr_t * __restrict attr,
    short flags, int priority, int memlimit_active, int memlimit_inactive) __API_AVAILABLE(macos(10.11), ios(9.0));

// time-to-relaunch after jetsam, set by launchd
int     posix_spawnattr_set_jetsam_ttr_np(const posix_spawnattr_t * __restrict attr, uint32_t count, uint32_t *ttrs_millis) __OSX_AVAILABLE_STARTING(__MAC_10_15, __IPHONE_13_0);

int     posix_spawnattr_set_threadlimit_ext(posix_spawnattr_t * __restrict attr,
    int thread_limit)  __API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0));

#define POSIX_SPAWN_IMPORTANCE_PORT_COUNT 32
int     posix_spawnattr_set_importancewatch_port_np(posix_spawnattr_t * __restrict attr,
    int count, mach_port_t portarray[])  __API_AVAILABLE(macos(10.9), ios(6.0));

int     posix_spawnattr_set_registered_ports_np(posix_spawnattr_t * __restrict attr, mach_port_t portarray[], uint32_t count) __API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0));

int
posix_spawnattr_set_ptrauth_task_port_np(posix_spawnattr_t * __restrict attr,
    mach_port_t port) __API_AVAILABLE(macos(10.16), ios(14.0), tvos(14.0), watchos(7.0));

#define POSIX_SPAWN_MACPOLICYINFO_WITHSIZE 1
int     posix_spawnattr_getmacpolicyinfo_np(const posix_spawnattr_t * __restrict, const char *, void **, size_t *) __API_AVAILABLE(macos(10.9), ios(7.0));
int     posix_spawnattr_setmacpolicyinfo_np(posix_spawnattr_t * __restrict, const char *, void *, size_t) __API_AVAILABLE(macos(10.9), ios(7.0));

int     posix_spawnattr_setcoalition_np(const posix_spawnattr_t * __restrict, uint64_t, int, int) __API_AVAILABLE(macos(10.10), ios(8.0));

int     posix_spawnattr_set_qos_clamp_np(const posix_spawnattr_t * __restrict, uint64_t) __API_AVAILABLE(macos(10.10), ios(8.0));
int     posix_spawnattr_get_qos_clamp_np(const posix_spawnattr_t * __restrict, uint64_t * __restrict) __API_AVAILABLE(macos(10.10), ios(8.0));

int     posix_spawnattr_set_darwin_role_np(const posix_spawnattr_t * __restrict, uint64_t) __API_AVAILABLE(macos(10.11), ios(9.0));
int     posix_spawnattr_get_darwin_role_np(const posix_spawnattr_t * __restrict, uint64_t * __restrict) __API_AVAILABLE(macos(10.11), ios(9.0));

int     posix_spawnattr_set_persona_np(const posix_spawnattr_t * __restrict, uid_t, uint32_t) __API_AVAILABLE(macos(10.11), ios(9.0));
int     posix_spawnattr_set_persona_uid_np(const posix_spawnattr_t * __restrict, uid_t) __API_AVAILABLE(macos(10.11), ios(9.0));
int     posix_spawnattr_set_persona_gid_np(const posix_spawnattr_t * __restrict, gid_t) __API_AVAILABLE(macos(10.11), ios(9.0));
int     posix_spawnattr_set_persona_groups_np(const posix_spawnattr_t * __restrict, int, gid_t * __restrict, uid_t) __API_AVAILABLE(macos(10.11), ios(9.0));

int     posix_spawnattr_set_max_addr_np(const posix_spawnattr_t * __restrict attr, uint64_t max_addr) __API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0));

int     posix_spawnattr_set_uid_np(const posix_spawnattr_t * __restrict, uid_t) __API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0));
int     posix_spawnattr_set_gid_np(const posix_spawnattr_t * __restrict, gid_t) __API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0));
int     posix_spawnattr_set_groups_np(const posix_spawnattr_t * __restrict, int, gid_t * __restrict, uid_t) __API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0));
int     posix_spawnattr_set_login_np(const posix_spawnattr_t * __restrict, const char * __restrict) __API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0));

int     posix_spawnattr_set_subsystem_root_path_np(posix_spawnattr_t *attr, char *path); __API_AVAILABLE(macos(11.0), ios(14.0), tvos(14.0), watchos(7.0));

int     posix_spawnattr_set_platform_np(posix_spawnattr_t *attr, int platform, uint32_t flags); __API_AVAILABLE(macos(11.0), ios(14.0), tvos(14.0), watchos(7.0));

int     posix_spawnattr_disable_ptr_auth_a_keys_np(posix_spawnattr_t *attr, uint32_t flags); __API_AVAILABLE(macos(11.0), ios(14.0), tvos(14.0), watchos(7.0));

int     posix_spawn_file_actions_add_fileportdup2_np(posix_spawn_file_actions_t * __restrict, mach_port_t, int) __API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0));

__END_DECLS

#endif /* !defined _SPAWN_PRIVATE_H_*/

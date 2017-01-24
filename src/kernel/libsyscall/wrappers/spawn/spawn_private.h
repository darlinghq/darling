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

int     posix_spawnattr_getpcontrol_np(const posix_spawnattr_t * __restrict, int * __restrict) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
int     posix_spawnattr_setpcontrol_np(posix_spawnattr_t *, const int) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);

int     posix_spawnattr_getprocesstype_np(const posix_spawnattr_t * __restrict, int * __restrict) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
int     posix_spawnattr_setprocesstype_np(posix_spawnattr_t *, const int) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

int	posix_spawnattr_setcpumonitor(posix_spawnattr_t * __restrict, uint64_t, uint64_t) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
int	posix_spawnattr_getcpumonitor(posix_spawnattr_t * __restrict, uint64_t *, uint64_t *) __OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
int	posix_spawnattr_setcpumonitor_default(posix_spawnattr_t * __restrict) __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_6_0);


int     posix_spawnattr_setjetsam_ext(posix_spawnattr_t * __restrict attr,
		   short flags, int priority, int memlimit_active, int memlimit_inactive) __OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0);

#define POSIX_SPAWN_IMPORTANCE_PORT_COUNT 32
int	posix_spawnattr_set_importancewatch_port_np(posix_spawnattr_t * __restrict attr,
                 int count, mach_port_t portarray[])  __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_6_0);

#define POSIX_SPAWN_MACPOLICYINFO_WITHSIZE 1
int	posix_spawnattr_getmacpolicyinfo_np(const posix_spawnattr_t * __restrict, const char *, void **, size_t *) __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_7_0);
int	posix_spawnattr_setmacpolicyinfo_np(posix_spawnattr_t * __restrict, const char *, void *, size_t) __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_7_0);

int	posix_spawnattr_setcoalition_np(const posix_spawnattr_t * __restrict, uint64_t, int, int) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_8_0);

int     posix_spawnattr_set_qos_clamp_np(const posix_spawnattr_t * __restrict, uint64_t) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_8_0);
int     posix_spawnattr_get_qos_clamp_np(const posix_spawnattr_t * __restrict, uint64_t * __restrict) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_8_0);

int     posix_spawnattr_set_darwin_role_np(const posix_spawnattr_t * __restrict, uint64_t) __OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0);
int     posix_spawnattr_get_darwin_role_np(const posix_spawnattr_t * __restrict, uint64_t * __restrict) __OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0);

int     posix_spawnattr_set_persona_np(const posix_spawnattr_t * __restrict, uid_t, uint32_t) __OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0);
int     posix_spawnattr_set_persona_uid_np(const posix_spawnattr_t * __restrict, uid_t) __OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0);
int     posix_spawnattr_set_persona_gid_np(const posix_spawnattr_t * __restrict, gid_t) __OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0);
int     posix_spawnattr_set_persona_groups_np(const posix_spawnattr_t * __restrict, int, gid_t *, uid_t) __OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0);

#endif /* !defined _SPAWN_PRIVATE_H_*/

/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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

#ifndef __BOOTSTRAP_PRIVATE_H__
#define __BOOTSTRAP_PRIVATE_H__

#include <servers/bootstrap.h>
#include <sys/types.h>
#include <uuid/uuid.h>

__BEGIN_DECLS

#pragma GCC visibility push(default)

#define BOOTSTRAP_PER_PID_SERVICE			(1 << 0)
#define BOOTSTRAP_ALLOW_LOOKUP				(1 << 1)
#define BOOTSTRAP_DENY_JOB_CREATION			(1 << 2)
#define BOOTSTRAP_PRIVILEGED_SERVER			(1 << 3)
#define BOOTSTRAP_FORCE_LOCAL				(1 << 4)
#define BOOTSTRAP_SPECIFIC_INSTANCE			(1 << 5)
#define BOOTSTRAP_STRICT_CHECKIN			(1 << 6)
#define BOOTSTRAP_STRICT_LOOKUP				(1 << 7)

#define BOOTSTRAP_PROPERTY_EXPLICITSUBSET	(1 << 0) /* Created via bootstrap_subset(). */
#define BOOTSTRAP_PROPERTY_IMPLICITSUBSET	(1 << 1) /* Created via _vprocmgr_switch_to_session(). */
#define BOOTSTRAP_PROPERTY_MOVEDSUBSET		(1 << 2) /* Created via _vprocmgr_move_subset_to_user(). */
#define BOOTSTRAP_PROPERTY_PERUSER			(1 << 3) /* A per-user launchd's root bootstrap. */
#define BOOTSTRAP_PROPERTY_XPC_DOMAIN		(1 << 4) /* An XPC domain. Duh. */
#define BOOTSTRAP_PROPERTY_XPC_SINGLETON	(1 << 5) /* A singleton XPC domain. */

void bootstrap_init(void);

kern_return_t bootstrap_register2(mach_port_t bp, name_t service_name, mach_port_t sp, uint64_t flags);

kern_return_t bootstrap_look_up2(mach_port_t bp, const name_t service_name, mach_port_t *sp, pid_t target_pid, uint64_t flags);

kern_return_t bootstrap_check_in2(mach_port_t bp, const name_t service_name, mach_port_t *sp, uint64_t flags);

kern_return_t bootstrap_look_up_per_user(mach_port_t bp, const name_t service_name, uid_t target_user, mach_port_t *sp);

kern_return_t bootstrap_lookup_children(mach_port_t bp, mach_port_array_t *children, name_array_t *names, bootstrap_property_array_t *properties, mach_msg_type_number_t *n_children);

kern_return_t bootstrap_look_up3(mach_port_t bp, const name_t service_name, mach_port_t *sp, pid_t target_pid, const uuid_t instance_id, uint64_t flags);

kern_return_t bootstrap_check_in3(mach_port_t bp, const name_t service_name, mach_port_t *sp, uuid_t instance_id, uint64_t flags);

kern_return_t bootstrap_get_root(mach_port_t bp, mach_port_t *root);

#pragma GCC visibility pop

__END_DECLS

#endif /* __BOOTSTRAP_PRIVATE_H__ */

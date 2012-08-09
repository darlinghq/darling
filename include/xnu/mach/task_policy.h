/*
 * Copyright (c) 2000-2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _MACH_TASK_POLICY_H_
#define _MACH_TASK_POLICY_H_

#include <mach/mach_types.h>

/*
 * These are the calls for accessing the policy parameters
 * of a particular task.
 *
 * The extra 'get_default' parameter to the second call is
 * IN/OUT as follows:
 * 1) if asserted on the way in it indicates that the default
 * values should be returned, not the ones currently set, in
 * this case 'get_default' will always be asserted on return;
 * 2) if unasserted on the way in, the current settings are
 * desired and if still unasserted on return, then the info
 * returned reflects the current settings, otherwise if
 * 'get_default' returns asserted, it means that there are no
 * current settings due to other parameters taking precedence,
 * and the default ones are being returned instead.
 */

typedef natural_t	task_policy_flavor_t;
typedef integer_t	*task_policy_t;

/*
kern_return_t	task_policy_set(
					task_t					task,
					task_policy_flavor_t	flavor,
					task_policy_t			policy_info,
					mach_msg_type_number_t	count);

kern_return_t	task_policy_get(
					task_t					task,
					task_policy_flavor_t	flavor,
					task_policy_t			policy_info,
					mach_msg_type_number_t	*count,
					boolean_t				*get_default);
*/

/*
 * Defined flavors.
 */
/*
 * TASK_CATEGORY_POLICY:
 *
 * This provides information to the kernel about the role
 * of the task in the system.
 *
 * Parameters:
 *
 * role: Enumerated as follows:
 *
 * TASK_UNSPECIFIED is the default, since the role is not
 * inherited from the parent.
 *
 * TASK_FOREGROUND_APPLICATION should be assigned when the
 * task is a normal UI application in the foreground from
 * the HI point of view.
 * **N.B. There may be more than one of these at a given time.
 *
 * TASK_BACKGROUND_APPLICATION should be assigned when the
 * task is a normal UI application in the background from
 * the HI point of view.
 *
 * TASK_CONTROL_APPLICATION should be assigned to the unique
 * UI application which implements the pop-up application dialog.
 * There can only be one task at a time with this designation,
 * which is assigned FCFS.
 *
 * TASK_GRAPHICS_SERVER should be assigned to the graphics
 * management (window) server.  There can only be one task at
 * a time with this designation, which is assigned FCFS.
 */

#define TASK_CATEGORY_POLICY		1

enum task_role {
	TASK_RENICED = -1,
	TASK_UNSPECIFIED = 0,
	TASK_FOREGROUND_APPLICATION,
	TASK_BACKGROUND_APPLICATION,
	TASK_CONTROL_APPLICATION,
	TASK_GRAPHICS_SERVER,
	TASK_THROTTLE_APPLICATION,
	TASK_NONUI_APPLICATION,
	TASK_DEFAULT_APPLICATION
};

typedef enum task_role		task_role_t;

struct task_category_policy {
	task_role_t		role;
};

typedef struct task_category_policy		task_category_policy_data_t;
typedef struct task_category_policy		*task_category_policy_t;

#define TASK_CATEGORY_POLICY_COUNT	((mach_msg_type_number_t) \
	(sizeof (task_category_policy_data_t) / sizeof (integer_t)))

#endif	/* _MACH_TASK_POLICY_H_ */

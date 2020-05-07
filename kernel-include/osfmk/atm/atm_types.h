/*
 * Copyright (c) 2012-2013 Apple Inc. All rights reserved.
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

#ifndef _ATM_ATM_TYPES_H_
#define _ATM_ATM_TYPES_H_

#include <stdint.h>
#include <mach/mach_types.h>

#define MACH_VOUCHER_ATTR_ATM_NULL              ((mach_voucher_attr_recipe_command_t)501)
#define MACH_VOUCHER_ATTR_ATM_CREATE            ((mach_voucher_attr_recipe_command_t)510)
#define MACH_VOUCHER_ATTR_ATM_REGISTER          ((mach_voucher_attr_recipe_command_t)511)

typedef uint32_t atm_action_t;
#define ATM_ACTION_DISCARD      0x1
#define ATM_ACTION_COLLECT      0x2
#define ATM_ACTION_LOGFAIL      0x3
#define ATM_FIND_MIN_SUB_AID    0x4
#define ATM_ACTION_UNREGISTER   0x5
#define ATM_ACTION_REGISTER     0x6
#define ATM_ACTION_GETSUBAID    0x7

typedef uint64_t atm_guard_t;
typedef uint64_t aid_t;

/* Deprecated. will be removed soon */
typedef uint64_t subaid_t;
typedef uint64_t mailbox_offset_t;
#define SUB_AID_MAX (UINT64_MAX)

typedef uint64_t atm_aid_t;
typedef uint32_t atm_subaid32_t;
typedef uint64_t mach_atm_subaid_t;             /* Used for mach based apis. */
typedef uint64_t atm_mailbox_offset_t;


typedef mach_port_t atm_memory_descriptor_t;
typedef atm_memory_descriptor_t *atm_memory_descriptor_array_t;
typedef uint64_t *atm_memory_size_array_t;

#define ATM_SUBAID32_MAX        (UINT32_MAX)
#define ATM_TRACE_DISABLE               (0x0100) /* OS_TRACE_MODE_DISABLE - Do not initialize the new logging*/
#define ATM_TRACE_OFF                   (0x0400) /* OS_TRACE_MODE_OFF - Don't drop log messages to new log buffers */
#define ATM_ENABLE_LEGACY_LOGGING       (0x20000000) /* OS_TRACE_SYSTEMMODE_LEGACY_LOGGING - Enable legacy logging  */

#endif /* _ATM_ATM_TYPES_H_ */

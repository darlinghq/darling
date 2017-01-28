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

#ifndef _BANK_BANK_TYPES_H_
#define _BANK_BANK_TYPES_H_

#include <stdint.h>
#include <mach/mach_types.h>

#define MACH_VOUCHER_ATTR_BANK_NULL 		((mach_voucher_attr_recipe_command_t)601)
#define MACH_VOUCHER_ATTR_BANK_CREATE 		((mach_voucher_attr_recipe_command_t)610)

#define MACH_VOUCHER_BANK_CONTENT_SIZE (500)

typedef uint32_t bank_action_t;
#define BANK_ORIGINATOR_PID     0x1
#define BANK_PERSONA_TOKEN      0x2

struct proc_persona_info {
	uint64_t unique_pid;
	int32_t  pid;
	uint32_t flags;
	uint32_t pidversion;
	uint32_t persona_id;
	uint32_t uid;
	uint32_t gid;
	uint8_t  macho_uuid[16];
};

struct persona_token {
	struct proc_persona_info originator;
	struct proc_persona_info proximate;
};

#ifdef PRIVATE
#define ENTITLEMENT_PERSONA_PROPAGATE "com.apple.private.personas.propagate"
#endif /* PRIVATE */

#endif /* _BANK_BANK_TYPES_H_ */

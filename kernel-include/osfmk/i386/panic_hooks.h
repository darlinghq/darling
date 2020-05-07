/*
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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

#ifndef PANICHOOKS_H_
#define PANICHOOKS_H_

#if XNU_KERNEL_PRIVATE

#include <stdint.h>
#include <mach/i386/boolean.h>

typedef struct {
	uint64_t        opaque[6];
} panic_hook_t;

typedef void (*panic_hook_fn_t)(panic_hook_t *);

void panic_hooks_init(void);
void panic_check_hook(void);

void panic_hook(panic_hook_t *hook, panic_hook_fn_t hook_fn);
void panic_unhook(panic_hook_t *hook);
void panic_dump_mem(const void *addr, int len);

typedef struct panic_phys_range {
	uint32_t type;
	uint64_t phys_start;
	uint64_t len;
} panic_phys_range_t;

boolean_t panic_phys_range_before(const void *addr, uint64_t *pphys,
    panic_phys_range_t *range);

#endif // XNU_KERNEL_PRIVATE

#endif // PANICHOOKS_H_

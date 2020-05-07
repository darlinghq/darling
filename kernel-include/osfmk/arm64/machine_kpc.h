/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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
#ifndef _MACHINE_ARM64_KPC_H
#define _MACHINE_ARM64_KPC_H

#include <pexpert/arm64/board_config.h>

typedef uint64_t kpc_config_t;

#define KPC_ARM64_FIXED_COUNT        (2)
#define KPC_ARM64_CONFIGURABLE_COUNT (CORE_NCTRS - KPC_ARM64_FIXED_COUNT)

#define KPC_ARM64_COUNTER_WIDTH    (47)
#define KPC_ARM64_COUNTER_MASK     ((UINT64_C(1) << KPC_ARM64_COUNTER_WIDTH) - 1)
#define KPC_ARM64_COUNTER_OVF_BIT  (47)
#define KPC_ARM64_COUNTER_OVF_MASK (UINT64_C(1) << KPC_ARM64_COUNTER_OVF_BIT)

/* arm64 uses fixed counter shadows */
#define FIXED_COUNTER_SHADOW (1)

#define KPC_ARM64_PMC_COUNT (KPC_ARM64_FIXED_COUNT + KPC_ARM64_CONFIGURABLE_COUNT)

/* Size to the maximum number of counters we could read from every class in one go */
#define KPC_MAX_COUNTERS (KPC_ARM64_FIXED_COUNT + KPC_ARM64_CONFIGURABLE_COUNT + 1)

#endif /* _MACHINE_ARM64_KPC_H */

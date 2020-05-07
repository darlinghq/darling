/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#include <TargetConditionals.h>
#include <machine/cpu_capabilities.h>
#include <mach/kern_return.h>
#include <mach/mach_host.h>
#include <mach/host_priv.h>
#include <sys/types.h>

kern_return_t
host_get_atm_diagnostic_flag(host_t host __unused,
    uint32_t *diagnostic_flag)
{
	volatile uint32_t *diagnostic_flag_address = (volatile uint32_t *)(uintptr_t)(_COMM_PAGE_ATM_DIAGNOSTIC_CONFIG);
	*diagnostic_flag = *diagnostic_flag_address;
	return KERN_SUCCESS;
}

kern_return_t
host_get_multiuser_config_flags(host_t host __unused,
    uint32_t *multiuser_flags)
{
#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
	volatile uint32_t *multiuser_flag_address = (volatile uint32_t *)(uintptr_t)(_COMM_PAGE_MULTIUSER_CONFIG);
	*multiuser_flags = *multiuser_flag_address;
	return KERN_SUCCESS;
#else
	(void)multiuser_flags;
	return KERN_NOT_SUPPORTED;
#endif
}

kern_return_t
host_check_multiuser_mode(host_t host __unused,
    uint32_t *multiuser_mode)
{
#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
	uint32_t multiuser_flags;
	kern_return_t kr;

	kr = host_get_multiuser_config_flags(host, &multiuser_flags);
	if (kr != KERN_SUCCESS) {
		return kr;
	}
	*multiuser_mode = (multiuser_flags & kIsMultiUserDevice) == kIsMultiUserDevice;
	return KERN_SUCCESS;
#else
	(void)multiuser_mode;
	return KERN_NOT_SUPPORTED;
#endif
}

extern kern_return_t
_kernelrpc_host_create_mach_voucher(mach_port_name_t host,
    mach_voucher_attr_raw_recipe_array_t recipes,
    mach_voucher_attr_recipe_size_t recipesCnt,
    mach_port_name_t *voucher);

kern_return_t
host_create_mach_voucher(mach_port_name_t host,
    mach_voucher_attr_raw_recipe_array_t recipes,
    mach_voucher_attr_recipe_size_t recipesCnt,
    mach_port_name_t *voucher)
{
	kern_return_t rv;

	rv = host_create_mach_voucher_trap(host, recipes, recipesCnt, voucher);

#ifdef __x86_64__
	/* REMOVE once XBS kernel has new trap */
	if (rv == ((1 << 24) | 70)) { /* see mach/i386/syscall_sw.h */
		rv = MACH_SEND_INVALID_DEST;
	}
#elif defined(__i386__)
	/* REMOVE once XBS kernel has new trap */
	if (rv == (kern_return_t)(-70)) {
		rv = MACH_SEND_INVALID_DEST;
	}
#endif

	if (rv == MACH_SEND_INVALID_DEST) {
		rv = _kernelrpc_host_create_mach_voucher(host, recipes, recipesCnt, voucher);
	}

	return rv;
}

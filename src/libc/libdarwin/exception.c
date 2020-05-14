/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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
#include "internal.h"

#pragma mark Assertions
static_assert(_OS_CRASH_LAST < OS_CRASH_PORT_ARRAY_COUNT,
		"array not large enough");

#pragma mark Private Utilities
static thread_state_flavor_t
_exception_thread_state_flavor(void)
{
	thread_state_flavor_t flavor = 0;
#if defined(__i386__) || defined(__x86_64__)
	flavor = x86_THREAD_STATE;
#elif defined(__arm__) || defined(__arm64__)
	flavor = ARM_THREAD_STATE;
#else // defined(__i386__) || defined(__x86_64__)
#error "unsupported architecture"
#endif // defined(__i386__) || defined(__x86_64__)
	return flavor;
}

static exception_mask_t
_exception_mask(os_crash_flags_t flags)
{
	exception_mask_t mask = 0;
	if (flags & OS_CRASH_FLAG_CRASH) {
		mask |= EXC_MASK_CRASH;
	}
	if (flags & OS_CRASH_FLAG_GUARD) {
		mask |= EXC_MASK_GUARD;
	}
	if (flags & OS_CRASH_FLAG_RESOURCE) {
		mask |= EXC_MASK_RESOURCE;
	}
	if (flags & OS_CRASH_FLAG_CORPSE) {
		mask |= EXC_MASK_CORPSE_NOTIFY;
		mask &= (exception_mask_t)(~EXC_MASK_CRASH);
	}
	return mask;
}

static exception_behavior_t
_exception_behavior(void)
{
	return (EXCEPTION_STATE_IDENTITY|MACH_EXCEPTION_CODES);
}

static void
_os_exception_port_set(os_crash_port_t *ep,
		os_crash_type_t type, mach_port_t p)
{
	kern_return_t kr = KERN_FAILURE;

	ep->oep_type = type;
	ep->oep_port = p;

	kr = mach_port_mod_refs(mach_task_self(), p, MACH_PORT_RIGHT_SEND, 1);
	if (kr == KERN_SUCCESS) {
		ep->oep_port = p;
	} else {
		ep->oep_port = MACH_PORT_DEAD;
	}
}

#pragma mark API
kern_return_t
os_crash_set_reporter_port(mach_port_t where,
		os_crash_flags_t flags,  mach_port_t p)
{
	kern_return_t kr = KERN_FAILURE;
	exception_mask_t mask = _exception_mask(flags);
	exception_behavior_t bhvr = _exception_behavior();
	thread_state_flavor_t flvr = _exception_thread_state_flavor();
	mach_port_t host_priv = MACH_PORT_NULL;

	// If we're not privleged to get the host-privileged port, no big deal, then
	// the comparison below will fail.
	(void)host_get_host_priv_port(mach_host_self(), &host_priv);

	if (flags == OS_CRASH_FLAG_INIT) {
		return KERN_SUCCESS;
	}

	if (where == mach_host_self() || where == host_priv) {
		kr = host_set_exception_ports(where, mask, p, bhvr, flvr);
	} else {
		kr = task_set_exception_ports(where, mask, p, bhvr, flvr);
	}

	return kr;
}

kern_return_t
os_crash_get_reporter_port_array(mach_port_t where, os_crash_port_array_t array)
{
	kern_return_t kr = KERN_FAILURE;
	exception_mask_t masks[EXC_TYPES_COUNT];
	mach_msg_type_number_t nmasks = 0;
	exception_port_t ports[EXC_TYPES_COUNT];
	exception_behavior_t behaviors[EXC_TYPES_COUNT];
	thread_state_flavor_t flavors[EXC_TYPES_COUNT];
	exception_mask_t mask = EXC_MASK_CRASH
		| EXC_MASK_GUARD
		| EXC_MASK_RESOURCE
		| EXC_MASK_CORPSE_NOTIFY;
	size_t i = 0;
	size_t j = 0;
	mach_port_t host_priv = MACH_PORT_NULL;

	(void)host_get_host_priv_port(mach_host_self(), &host_priv);

	if (where == mach_host_self() || where == host_priv) {
		kr = host_get_exception_ports(mach_host_self(), mask,
				masks, &nmasks, ports, behaviors, flavors);
	} else {
		kr = task_get_exception_ports(where, mask,
				masks, &nmasks, ports, behaviors, flavors);
	}

	if (kr) {
		goto __out;
	}

	bzero(array, sizeof(array[0]) * OS_CRASH_PORT_ARRAY_COUNT);
	for (i = 0; i < nmasks; i++) {
		if (masks[i] & EXC_MASK_CRASH) {
			_os_exception_port_set(&array[j], OS_CRASH_CRASH, ports[i]);
			j++;
		}
		if (masks[i] & EXC_MASK_GUARD) {
			_os_exception_port_set(&array[j], OS_CRASH_GUARD, ports[i]);
			j++;
		}
		if (masks[i] & EXC_MASK_RESOURCE) {
			_os_exception_port_set(&array[j], OS_CRASH_RESOURCE, ports[i]);
			j++;
		}
		if (masks[i] & EXC_MASK_CORPSE_NOTIFY) {
			_os_exception_port_set(&array[j], OS_CRASH_CORPSE, ports[i]);
			j++;
		}
		kr = mach_port_deallocate(mach_task_self(), ports[i]);
		os_assert_mach("deallocate port", kr);
	}

__out:
	return kr;
}

void
os_crash_port_array_deallocate(os_crash_port_array_t array)
{
	size_t i = 0;

	for (i = 0; i < 16; i++) {
		if (MACH_PORT_VALID(array[i].oep_port)) {
			kern_return_t kr = KERN_FAILURE;
			kr = mach_port_deallocate(mach_task_self(), array[i].oep_port);
			os_assert_mach("deallocate port", kr);
		}
	}
}

kern_return_t
os_crash_spawnattr_set_reporter_port(posix_spawnattr_t *psattr,
		os_crash_flags_t flags,  mach_port_t p)
{
	int error = -1;
	exception_mask_t mask = _exception_mask(flags);
	exception_behavior_t bhvr = _exception_behavior();
	thread_state_flavor_t flvr = _exception_thread_state_flavor();

	if (!MACH_PORT_VALID(p)) {
		return KERN_INVALID_NAME;
	}

	if (flags == OS_CRASH_FLAG_INIT) {
		return KERN_SUCCESS;
	}

	error = posix_spawnattr_setexceptionports_np(psattr, mask, p, bhvr, flvr);
	os_assert_zero(error);

	return KERN_SUCCESS;
}

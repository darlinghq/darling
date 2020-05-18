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

// Code that is common to Nano V1 and Nano V2. When Nano V1 is removed,
// most of this file will move to nanov2_malloc.c.

#if CONFIG_NANOZONE

// Possible enablement modes for Nano V2
typedef enum {
	NANO_INACTIVE,	// Inactive, but can be selected with MallocNanoZone=V2
	NANO_ENABLED,	// Available and default if Nano is turned on.
	NANO_FORCED,	// Force use of Nano V2 for all processes.
} nanov2_mode_t;

// Which version of Nano is engaged. By default, none.
nano_version_t _malloc_engaged_nano = NANO_NONE;

// Nano mode selection boot argument
static const char mode_boot_arg[] = "nanov2_mode";
static const char inactive_mode[] = "inactive"; // Use Nano V1 for Nano
static const char enabled_mode[] = "enabled";	// Use Nano V2 for Nano
static const char forced_mode[] = "forced";		// Force Nano V2 everywhere

// The maximum number of per-CPU allocation regions to use for Nano.
unsigned int nano_common_max_magazines;
boolean_t nano_common_max_magazines_is_ncpu = true;

// Boot argument for nano_common_max_magazines
static const char nano_max_magazines_boot_arg[] = "malloc_nano_max_magazines";

#pragma mark -
#pragma mark Initialization

// Shared initialization code. Determines which version of Nano should be used,
// if any, and sets _malloc_engaged_nano. The Nano version is determined as
// follows:
// 1. If the nanov2_mode boot arg has value "forced", Nano V2 is used
//		unconditionally in every process, except in processes that have
//		the MallocNanoZone variable set to V1.
// 2. If the nanov2_mode boot arg has value "enabled", Nano V2 is used if
//		the process wants to use Nano (i.e. the kernel opts the process in, or
//		the environment variable MallocNanoZone is 1).
// 3. If the nanov2_mode boot arg is not present or has any other value,
//		Nano V1 is used if the process wants to use Nano (i.e. the kernel opts
//		the process in, or the environment variable MallocNanoZone is 1).
//
// In cases (2) and (3), the selection can be explicitly overridden by setting
// the environment variable MallocNanoZone to V1 or V2.
void
nano_common_init(const char *envp[], const char *apple[], const char *bootargs)
{
	// Use the nanov2_mode boot argument and MallocNanoZone to determine
	// which version of Nano to use, if any.
	nanov2_mode_t nanov2_mode = NANO_ENABLED;
	const char *p = malloc_common_value_for_key(bootargs, mode_boot_arg);
	if (p) {
		if (!strncmp(p, inactive_mode, sizeof(inactive_mode) - 1)) {
			nanov2_mode = NANO_INACTIVE;
		} else if (!strncmp(p, enabled_mode, sizeof(enabled_mode) - 1)) {
			nanov2_mode = NANO_ENABLED;
		} else if (!strncmp(p, forced_mode, sizeof(forced_mode) - 1)) {
			nanov2_mode = NANO_FORCED;
		}
	}

	if (nanov2_mode == NANO_FORCED) {
		// We will use Nano V2 unless MallocNanoZone is "V1".
		const char *flag = _simple_getenv(envp, "MallocNanoZone");
		if (flag && (flag[0] == 'V' || flag[0] == 'v') && flag[1] == '1') {
			_malloc_engaged_nano = NANO_V1;
		} else {
			_malloc_engaged_nano = NANO_V2;
		}
	} else {
		const char *flag = _simple_getenv(apple, "MallocNanoZone");
		if (flag && flag[0] == '1') {
			_malloc_engaged_nano = nanov2_mode == NANO_ENABLED ? NANO_V2 : NANO_V1;
		}
		/* Explicit overrides from the environment */
		flag = _simple_getenv(envp, "MallocNanoZone");
		if (flag) {
			if (flag[0] == '1') {
				_malloc_engaged_nano = nanov2_mode == NANO_ENABLED ? NANO_V2 : NANO_V1;
			} else if (flag[0] == '0') {
				_malloc_engaged_nano = NANO_NONE;
			} else if (flag[0] == 'V' || flag[0] == 'v') {
				if (flag[1] == '1') {
					_malloc_engaged_nano = NANO_V1;
				} else if (flag[1] == '2') {
					_malloc_engaged_nano = NANO_V2;
				}
			}
		}
	}

	if (_malloc_engaged_nano) {
		// The maximum number of nano magazines can be set either via a
		// boot argument or from the environment. Get the boot argument value
		// here and store it. We can't bounds check it until we have phys_ncpus,
		// which happens later in nano_common_configure(), along with handling
		// of the environment value setting.
		char value_buf[256];
		const char *flag = malloc_common_value_for_key_copy(bootargs,
				nano_max_magazines_boot_arg, value_buf, sizeof(value_buf));
		if (flag) {
			const char *endp;
			long value = malloc_common_convert_to_long(flag, &endp);
			if (!*endp && value >= 0) {
				nano_common_max_magazines = (unsigned int)value;
			} else {
				malloc_report(ASL_LEVEL_ERR,
					"malloc_nano_max_magazines must be positive - ignored.\n");
			}
		}
	}

	switch (_malloc_engaged_nano) {
	case NANO_V1:
		nano_init(envp, apple, bootargs);
		break;
	case NANO_V2:
		nanov2_init(envp, apple, bootargs);
		break;
	default:
		break;
	}
}

// Second phase of initialization, called from _malloc_initialize(). Used for
// code that depends on state set in _malloc_initialize(), such as the
// number of physical CPUs.
void
nano_common_configure(void)
{
	// Set nano_common_max_magazines. An initial (unvalidated) value may have
	// been set from the boot args.
	unsigned int magazines = nano_common_max_magazines > 0 ?
			nano_common_max_magazines : phys_ncpus;

	// Environment variable overrides boot arg, unless it's not valid.
	const char *flag = getenv("MallocNanoMaxMagazines");
#if RDAR_48993662
	if (!flag) {
		flag = getenv("_MallocNanoMaxMagazines");
	}
#endif // RDAR_48993662
	if (flag) {
		int value = (int)strtol(flag, NULL, 0);
		if (value < 0) {
			malloc_report(ASL_LEVEL_ERR,
					"MallocNanoMaxMagazines must be positive - ignored.\n");
		} else {
			magazines = value;
		}
	}

	if (magazines == 0) {
		magazines = phys_ncpus;
	} else if (magazines > phys_ncpus) {
		magazines = phys_ncpus;
		malloc_report(ASL_LEVEL_ERR,
				"Nano maximum magazines limited to number of physical "
				"CPUs [%d]\n", phys_ncpus);
	}
	nano_common_max_magazines = magazines;
	if (flag) {
		malloc_report(ASL_LEVEL_INFO, "Nano maximum magazines set to %d\n",
					   nano_common_max_magazines);
	}
	nano_common_cpu_number_override_set();

	switch (_malloc_engaged_nano) {
	case NANO_V1:
		nano_configure();
		break;
	case NANO_V2:
		nanov2_configure();
		break;
	default:
		break;
	}
}

#pragma mark -
#pragma mark VM Helper Functions

void *
nano_common_allocate_based_pages(size_t size, unsigned char align,
		unsigned debug_flags, int vm_page_label, void *base_addr)
{
	mach_vm_address_t vm_addr;
	uintptr_t addr;
	mach_vm_size_t allocation_size = round_page(size);
	mach_vm_offset_t allocation_mask = ((mach_vm_offset_t)1 << align) - 1;
	int alloc_flags = VM_FLAGS_ANYWHERE | VM_MAKE_TAG(vm_page_label);
	kern_return_t kr;

	if (!allocation_size) {
		allocation_size = vm_page_size;
	}
	if (allocation_size < size) { // size_t arithmetic wrapped!
		return NULL;
	}

	vm_addr = round_page((mach_vm_address_t)base_addr);
	if (!vm_addr) {
		vm_addr = vm_page_size;
	}
	kr = mach_vm_map(mach_task_self(), &vm_addr, allocation_size,
			allocation_mask, alloc_flags, MEMORY_OBJECT_NULL, 0, FALSE,
			VM_PROT_DEFAULT, VM_PROT_ALL, VM_INHERIT_DEFAULT);
	if (kr) {
		malloc_zone_error(debug_flags, false, "*** can't allocate pages: "
				"mach_vm_map(size=%lu) failed (error code=%d)\n", size, kr);
		return NULL;
	}
	addr = (uintptr_t)vm_addr;

	return (void *)addr;
}

// Allocates virtual address from a given address for a given size. Succeeds
// (and returns TRUE) only if we get exactly the range of addresses that we
// asked for.
boolean_t
nano_common_allocate_vm_space(mach_vm_address_t base, mach_vm_size_t size)
{
	mach_vm_address_t vm_addr = base;
	kern_return_t kr = mach_vm_map(mach_task_self(), &vm_addr, size, 0,
		VM_MAKE_TAG(VM_MEMORY_MALLOC_NANO), MEMORY_OBJECT_NULL, 0, FALSE,
		VM_PROT_DEFAULT, VM_PROT_ALL, VM_INHERIT_DEFAULT);
	
	if (kr != KERN_SUCCESS || vm_addr != base) {
		// Failed or we got allocated somewhere else.
		if (!kr) {
			mach_vm_deallocate(mach_task_self(), vm_addr, size);
		}
		return FALSE;
	}
	return TRUE;
}

void
nano_common_deallocate_pages(void *addr, size_t size, unsigned debug_flags)
{
	mach_vm_address_t vm_addr = (mach_vm_address_t)addr;
	mach_vm_size_t allocation_size = size;
	kern_return_t kr;

	kr = mach_vm_deallocate(mach_task_self(), vm_addr, allocation_size);
	if (kr) {
		malloc_zone_error(debug_flags, false, "Can't deallocate_pages at %p\n",
				addr);
	}
}

#pragma mark -
#pragma mark Introspection Helper Functions

kern_return_t
nano_common_default_reader(task_t task, vm_address_t address, vm_size_t size,
		void **ptr)
{
	*ptr = (void *)address;
	return 0;
}

#pragma mark -
#pragma mark Utility functions

void
nano_common_cpu_number_override_set()
{
	boolean_t is_ncpu = _os_cpu_number_override == -1 && nano_common_max_magazines == phys_ncpus;
	
	// This facilitates a shortcut in nanov2_get_allocation_block_index() --
	// if nano_common_max_magazines_is_ncpu is true, we can also assume that
	// _os_cpu_number_override == -1 (i.e. we are not in malloc_replay).
	//
	// We check here for false, because we don't want to write "true" to a __DATA page because
	// that would make it dirty: <rdar://problem/46994833>
	if (!is_ncpu) {
		nano_common_max_magazines_is_ncpu = is_ncpu;
	}
}

#endif // CONFIG_NANOZONE


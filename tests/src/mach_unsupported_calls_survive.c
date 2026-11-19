/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// Mach kernel-interface routines that a guest may call but that the
// emulated kernel does not implement must fail the call, not terminate
// the emulation of the Mach kernel for every process.  Each routine below is
// called with plain arguments; any return code is accepted (the routines are
// implemented on macOS, so the result differs), but afterwards the Mach
// interface must still work.

#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/vm_purgable.h>
#include <mach/vm_statistics.h>
#include <mach/host_info.h>
#include <mach/processor_set.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

static int failures;

static void check(const char *name, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
	failures += !ok;
}

static int server_alive(void)
{
	mach_port_t p = MACH_PORT_NULL;
	if (mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &p) != KERN_SUCCESS) {
		return 0;
	}
	return mach_port_mod_refs(mach_task_self(), p, MACH_PORT_RIGHT_RECEIVE, -1) == KERN_SUCCESS;
}

#define CALL(label, expr) do { \
		kern_return_t kr_ = (expr); \
		char n_[96]; \
		snprintf(n_, sizeof n_, "%s returned (kr=%d) and the server is alive", label, kr_); \
		check(n_, server_alive()); \
	} while (0)

int main(void)
{
	mach_vm_address_t addr = 0;
	mach_vm_allocate(mach_task_self(), &addr, 4096, VM_FLAGS_ANYWHERE);
	host_t host = mach_host_self();

	{
		lockgroup_info_array_t lg = NULL;
		mach_msg_type_number_t lgc = 0;
		CALL("host_lockgroup_info", host_lockgroup_info(host, &lg, &lgc));
	}
	{
		integer_t buf[32];
		mach_msg_type_number_t c = 32;
		CALL("host_info(HOST_SCHED_INFO)", host_info(host, HOST_SCHED_INFO, buf, &c));
		c = 32;
		CALL("host_info(HOST_RESOURCE_SIZES)", host_info(host, HOST_RESOURCE_SIZES, buf, &c));
		c = 32;
		CALL("host_statistics(unknown flavour)", host_statistics(host, 12345, buf, &c));
		c = 32;
		CALL("host_info(unknown flavour)", host_info(host, 12345, buf, &c));
	}
	{
		int state = 0;
		CALL("mach_vm_purgable_control", mach_vm_purgable_control(mach_task_self(), addr, VM_PURGABLE_GET_STATE, &state));
	}
	CALL("mach_vm_behavior_set", mach_vm_behavior_set(mach_task_self(), addr, 4096, VM_BEHAVIOR_DEFAULT));
	CALL("mach_vm_inherit", mach_vm_inherit(mach_task_self(), addr, 4096, VM_INHERIT_COPY));
	{
		vm_machine_attribute_val_t v = MATTR_VAL_CACHE_FLUSH;
		CALL("mach_vm_machine_attribute", mach_vm_machine_attribute(mach_task_self(), addr, 4096, MATTR_CACHE, &v));
	}
	{
		task_purgable_info_t pi;
		CALL("task_purgable_info", task_purgable_info(mach_task_self(), &pi));
	}
	{
		processor_set_name_t pset = MACH_PORT_NULL;
		if (processor_set_default(host, &pset) == KERN_SUCCESS) {
			unsigned int a = 0;
			vm_size_t b = 0, c2 = 0, d = 0;
			vm_offset_t e = 0;
			CALL("processor_set_stack_usage", processor_set_stack_usage(pset, &a, &b, &c2, &d, &e));
			CALL("processor_set_policy_enable", processor_set_policy_enable(pset, POLICY_TIMESHARE));
			CALL("processor_set_policy_disable", processor_set_policy_disable(pset, POLICY_TIMESHARE, FALSE));
			CALL("processor_set_max_priority", processor_set_max_priority(pset, 10, FALSE));
		}
	}
	{
		processor_set_name_t pset = MACH_PORT_NULL;
		CALL("task_get_assignment", task_get_assignment(mach_task_self(), &pset));
	}
	{
		mach_vm_size_t sz = 4096;
		mach_vm_address_t where = 0;
		vm_prot_t cur, max;
		// a remap of a range that is already shared is outside what the emulation supports
		CALL("mach_vm_remap", mach_vm_remap(mach_task_self(), &where, sz, 0, VM_FLAGS_ANYWHERE, mach_task_self(), addr, FALSE, &cur, &max, VM_INHERIT_NONE));
		where = 0;
		CALL("mach_vm_remap again", mach_vm_remap(mach_task_self(), &where, sz, 0, VM_FLAGS_ANYWHERE, mach_task_self(), addr, FALSE, &cur, &max, VM_INHERIT_NONE));
	}

	printf("%s\n", failures ? "FAILED" : "PASSED");
	return failures ? 1 : 0;
}
